/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core/components_v2/list/render_list.h"

#include "base/log/ace_trace.h"
#include "base/log/log.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "core/animation/bilateral_spring_node.h"
#include "core/components/scroll/render_scroll.h"
#include "core/components/scroll/render_single_child_scroll.h"
#include "core/components/scroll/scroll_spring_effect.h"
#include "core/components/stack/stack_element.h"
#include "core/components_v2/list/list_component.h"
#include "core/components_v2/list/list_scroll_bar_controller.h"
#include "core/event/axis_event.h"
#include "core/gestures/long_press_recognizer.h"
#include "core/gestures/pan_recognizer.h"
#include "core/gestures/sequenced_recognizer.h"

namespace OHOS::Ace::V2 {
namespace {

constexpr double VIEW_PORT_SCALE = 1.2;
constexpr int32_t CHAIN_ANIMATION_NODE_COUNT = 30;
constexpr int32_t DEFAULT_SOURCE = 3;
constexpr int32_t SCROLL_STATE_IDLE = 0;
constexpr int32_t SCROLL_STATE_SCROLL = 1;
constexpr int32_t SCROLL_STATE_FLING = 2;
constexpr float SCROLL_MAX_TIME = 300.0f; // Scroll Animate max time 0.3 second
constexpr int32_t SCROLL_FROM_JUMP = 3;
constexpr int32_t DEFAULT_FINGERS = 1;
constexpr int32_t DEFAULT_DURATION = 200;
constexpr int32_t DEFAULT_DISTANCE = 0;

constexpr bool DIR_HORIZONTAL = false;
constexpr bool DIR_VERTICAL = true;
constexpr bool DIR_FORWARD = false;
constexpr bool DIR_REVERSE = true;
constexpr int32_t STEP_FORWARD = 1;
constexpr int32_t STEP_BACK = -1;
constexpr int32_t STEP_INVALID = 10;
constexpr double MIN_LANE_LENGTH = 1.0;
constexpr int32_t CENTER_ALIGN_DIVIDER = 2;

// IsRightToLeft | IsListVertical | IsDirectionVertical | IsDirectionReverse
const std::map<bool, std::map<bool, std::map<bool, std::map<bool, int32_t>>>> DIRECTION_MAP = {
    { false, // RTL is false
        {
            { false,
                {
                    { DIR_HORIZONTAL, { { DIR_FORWARD, STEP_FORWARD }, { DIR_REVERSE, STEP_BACK } } },
                    { DIR_VERTICAL, { { DIR_FORWARD, STEP_INVALID }, { DIR_REVERSE, STEP_INVALID } } }
                }
            },
            { true,
                {
                    { DIR_HORIZONTAL, { { DIR_FORWARD, STEP_INVALID }, { DIR_REVERSE, STEP_INVALID } } },
                    { DIR_VERTICAL, { { DIR_FORWARD, STEP_FORWARD }, { DIR_REVERSE, STEP_BACK } } }
                }
            }
        }
    },
    { true, // RTL is true
        {
            { false,
                {
                    { DIR_HORIZONTAL, { { DIR_FORWARD, STEP_BACK }, { DIR_REVERSE, STEP_FORWARD } } },
                    { DIR_VERTICAL, { { DIR_FORWARD, STEP_INVALID }, { DIR_REVERSE, STEP_INVALID } } }
                }
            },
            { true,
                {
                    { DIR_HORIZONTAL, { { DIR_FORWARD, STEP_INVALID }, { DIR_REVERSE, STEP_INVALID } } },
                    { DIR_VERTICAL, { { DIR_FORWARD, STEP_BACK }, { DIR_REVERSE, STEP_FORWARD } } }
                }
            }
        }
    }
};
} // namespace

RenderList::~RenderList()
{
    if (scrollBarProxy_) {
        scrollBarProxy_->UnRegisterScrollableNode(AceType::WeakClaim(this));
    }
}

void RenderList::Update(const RefPtr<Component>& component)
{
    component_ = AceType::DynamicCast<ListComponent>(component);
    ACE_DCHECK(component_);
    InitScrollBar();

    RemoveAllItems();

    auto axis = component_->GetDirection();
    vertical_ = axis == Axis::VERTICAL;

    // Start index should be updated only for the first time
    if (startIndex_ == INITIAL_CHILD_INDEX) {
        initialIndex_ = static_cast<size_t>(component_->GetInitialIndex());
        startIndex_ = initialIndex_ > 0 ? initialIndex_ : 0;
    }
    ApplyRestoreInfo();

    const auto& divider = component_->GetItemDivider();
    listSpace_ = component_->GetSpace();
    cachedCount_ = static_cast<size_t>(component_->GetCachedCount());

    LOGI("cached count: %{public}zu", cachedCount_);
    spaceWidth_ = std::max(NormalizePercentToPx(component_->GetSpace(), vertical_),
        divider ? NormalizePercentToPx(divider->strokeWidth, vertical_) : 0.0);

    if (scrollable_) {
        scrollable_->SetAxis(axis);
    } else {
        auto callback = [weak = AceType::WeakClaim(this)](double offset, int32_t source) {
            auto renderList = weak.Upgrade();

            if (!renderList) {
                return false;
            }

            if (source == SCROLL_FROM_START) {
                renderList->ProcessDragStart(offset);
                return true;
            }

            Offset delta;
            if (renderList->vertical_) {
                delta.SetX(0.0);
                delta.SetY(offset);
            } else {
                delta.SetX(offset);
                delta.SetY(0.0);
            }
            renderList->AdjustOffset(delta, source);
            if ((source == SCROLL_FROM_UPDATE || source == SCROLL_FROM_ANIMATION_SPRING) &&
                renderList->currentOffset_ >= 0.0) {
                if (renderList->scrollable_->RelatedScrollEventDoing(Offset(0.0, -offset))) {
                    return false;
                }
            }
            renderList->ProcessDragUpdate(renderList->GetMainAxis(delta));

            // Stop animator of scroll bar.
            auto scrollBarProxy = renderList->scrollBarProxy_;
            if (scrollBarProxy) {
                scrollBarProxy->StopScrollBarAnimator();
            }
            return renderList->UpdateScrollPosition(renderList->GetMainAxis(delta), source);
        };
        scrollable_ = AceType::MakeRefPtr<Scrollable>(callback, axis);
        scrollable_->SetNotifyScrollOverCallBack([weak = AceType::WeakClaim(this)](double velocity) {
            auto list = weak.Upgrade();
            if (!list) {
                return;
            }
            list->ProcessScrollOverCallback(velocity);
        });
        scrollable_->SetScrollEndCallback([weak = AceType::WeakClaim(this)]() {
            auto list = weak.Upgrade();
            if (!list) {
                LOGW("render list Upgrade fail in scroll end callback");
                return;
            }
            auto proxy = list->scrollBarProxy_;
            if (proxy) {
                proxy->StartScrollBarAnimator();
            }
            list->listEventFlags_[ListEvents::SCROLL_STOP] = true;
            list->HandleListEvent();
        });
        if (vertical_) {
            scrollable_->InitRelatedParent(GetParent());
        }
        scrollable_->Initialize(context_);
    }
    // now only support spring
    if (component_->GetEdgeEffect() == EdgeEffect::SPRING) {
        if (!scrollEffect_ || scrollEffect_->GetEdgeEffect() != EdgeEffect::SPRING) {
            scrollEffect_ = AceType::MakeRefPtr<ScrollSpringEffect>();
            ResetEdgeEffect();
        }
    } else {
        scrollEffect_ = nullptr;
    }

    auto controller = component_->GetScrollController();
    if (controller) {
        controller->SetScrollNode(AceType::WeakClaim(this));
    }
    if (!animator_) {
        animator_ = AceType::MakeRefPtr<Animator>(GetContext());
    }

    // chainAnimation
    if (chainAnimation_ != component_->GetChainAnimation()) {
        chainAnimation_ = component_->GetChainAnimation();
        if (chainAnimation_) {
            InitChainAnimation(CHAIN_ANIMATION_NODE_COUNT);
            overSpringProperty_ = SpringChainProperty::GetDefaultOverSpringProperty();
        } else {
            overSpringProperty_ = nullptr;
            chain_ = nullptr;
            chainAdapter_ = nullptr;
        }
    }

    if (chainAnimation_) {
        // add chain interval length
        spaceWidth_ += NormalizeToPx(chainProperty_.Interval());
    }

    scrollBarProxy_ = component_->GetScrollBarProxy();
    InitScrollBarProxy();

    onItemDragStart_ = component_->GetOnItemDragStartId();
    onItemDragEnter_ = component_->GetOnItemDragEnterId();
    onItemDragMove_ = component_->GetOnItemDragMoveId();
    onItemDragLeave_ = component_->GetOnItemDragLeaveId();
    onItemDrop_ = component_->GetOnItemDropId();

    if (onItemDragStart_) {
        CreateDragDropRecognizer();
    }

    isMultiSelectable_ = component_->GetMultiSelectable();
    hasHeight_ = component_->GetHasHeight();
    hasWidth_ = component_->GetHasWidth();
    isLaneList_ = (component_->GetLanes() != -1) || (component_->GetLaneConstrain() != std::nullopt);
    MarkNeedLayout();
}

void RenderList::InitScrollBarProxy()
{
    if (!scrollBarProxy_) {
        return;
    }
    auto callback = [weak = AceType::WeakClaim(this)](double value, int32_t source) {
        auto renderList = weak.Upgrade();
        if (!renderList) {
            LOGE("render list is released");
            return false;
        }
        return renderList->UpdateScrollPosition(value, source);
    };
    scrollBarProxy_->UnRegisterScrollableNode(AceType::WeakClaim(this));
    scrollBarProxy_->RegisterScrollableNode({ AceType::WeakClaim(this), callback });
}

bool RenderList::IsReachStart()
{
    bool scrollUpToReachStart = GreatNotEqual(prevOffset_, 0.0) && LessOrEqual(currentOffset_, 0.0);
    bool scrollDownToReachStart = LessNotEqual(prevOffset_, 0.0) && GreatOrEqual(currentOffset_, 0.0);
    return scrollUpToReachStart || scrollDownToReachStart;
}

void RenderList::InitScrollBar()
{
    if (scrollBar_) {
        scrollBar_->Reset();
        return;
    }
    if (!component_) {
        return;
    }
    const RefPtr<ScrollBarTheme> theme = GetTheme<ScrollBarTheme>();
    if (!theme) {
        return;
    }

    scrollBar_ = AceType::MakeRefPtr<ScrollBar>(component_->GetScrollBar(), theme->GetShapeMode());
    RefPtr<ListScrollBarController> controller = AceType::MakeRefPtr<ListScrollBarController>();
    scrollBar_->SetScrollBarController(controller);

    // set the scroll bar style
    scrollBar_->SetReservedHeight(theme->GetReservedHeight());
    scrollBar_->SetMinHeight(theme->GetMinHeight());
    scrollBar_->SetMinDynamicHeight(theme->GetMinDynamicHeight());
    scrollBar_->SetForegroundColor(theme->GetForegroundColor());
    scrollBar_->SetBackgroundColor(theme->GetBackgroundColor());
    scrollBar_->SetPadding(theme->GetPadding());
    scrollBar_->SetScrollable(true);
    scrollBar_->SetInactiveWidth(theme->GetNormalWidth());
    scrollBar_->SetNormalWidth(theme->GetNormalWidth());
    scrollBar_->SetActiveWidth(theme->GetActiveWidth());
    scrollBar_->SetTouchWidth(theme->GetTouchWidth());
    scrollBar_->InitScrollBar(AceType::WeakClaim(this), GetContext());
    SetScrollBarCallback();
}

void RenderList::SetScrollBarCallback()
{
    if (!scrollBar_ || !scrollBar_->NeedScrollBar()) {
        return;
    }
    auto&& scrollCallback = [weakList = AceType::WeakClaim(this)](double value, int32_t source) {
        auto list = weakList.Upgrade();
        if (!list) {
            LOGE("render list is released");
            return false;
        }
        return list->UpdateScrollPosition(value, source);
    };
    auto&& barEndCallback = [weakList = AceType::WeakClaim(this)](int32_t value) {
        auto list = weakList.Upgrade();
        if (!list) {
            LOGE("render list is released.");
            return;
        }
        list->scrollBarOpacity_ = value;
        list->MarkNeedRender();
    };
    auto&& scrollEndCallback = []() {
        // nothing to do
    };
    scrollBar_->SetCallBack(scrollCallback, barEndCallback, scrollEndCallback);
}

double RenderList::GetLaneLengthInPx(const Dimension& length)
{
    if (length.Unit() == DimensionUnit::PERCENT) {
        return NormalizePercentToPx(length, !vertical_, true);
    }
    return NormalizeToPx(length);
}

void RenderList::CalculateLanes()
{
    auto lanes = component_->GetLanes();
    do {
        auto laneConstrain = component_->GetLaneConstrain();
        // Case 1: lane length constrain is not set
        //      1.1: use [lanes_] set by user if [lanes_] is set
        //      1.2: set [lanes_] to 1 if [lanes_] is not set
        if (!laneConstrain) {
            if (lanes == -1 || lanes == 0) {
                lanes = 1;
            }
            maxLaneLength_ = GetCrossSize(GetLayoutParam().GetMaxSize()) / lanes;
            minLaneLength_ = GetCrossSize(GetLayoutParam().GetMinSize()) / lanes;
            break;
        }
        // Case 2: lane length constrain is set --> need to calculate [lanes_] according to contrain.
        // We agreed on such rules (assuming we have a vertical list here):
        // rule 1: [minLaneLength_] has a higher priority than [maxLaneLength_] when decide [lanes_], for e.g.,
        //         if [minLaneLength_] is 40, [maxLaneLength_] is 60, list's width is 120,
        //         the [lanes_] is 3 rather than 2.
        // rule 2: after [lanes_] is determined by rule 1, the width of lane will be as large as it can be, for e.g.,
        //         if [minLaneLength_] is 40, [maxLaneLength_] is 60, list's width is 132, the [lanes_] is 3
        //         according to rule 1, then the width of lane will be 132 / 3 = 44 rather than 40,
        //         its [minLaneLength_].

        // set layout size temporarily to calculate percent unit of constrain
        SetLayoutSize(GetLayoutParam().GetMaxSize());
        minLaneLength_ = GetLaneLengthInPx(laneConstrain.value().first);
        maxLaneLength_ = GetLaneLengthInPx(laneConstrain.value().second);
        if (LessOrEqual(minLaneLength_, 0.0)) {
            minLaneLength_ = MIN_LANE_LENGTH;
        }
        if (GreatNotEqual(minLaneLength_, maxLaneLength_)) {
            LOGI("minLaneLength: %{public}f is greater than maxLaneLength: %{public}f, set minLaneLength to"
                " maxLaneLength", minLaneLength_, maxLaneLength_);
            maxLaneLength_ = minLaneLength_;
        }

        // if minLaneLength is 40, maxLaneLength is 60
        // when list's width is 120, lanes_ = 3
        // when list's width is 80, lanes_ = 2
        // when list's width is 70, lanes_ = 1
        auto maxCrossSize = GetCrossSize(GetLayoutSize());
        double maxLanes = maxCrossSize / minLaneLength_;
        double minLanes = maxCrossSize / maxLaneLength_;
        // let's considerate senarios when maxCrossSize > 0
        // now it's guaranteed that [minLaneLength_] <= [maxLaneLength_], i.e., maxLanes >= minLanes > 0
        // there are 3 senarios:
        // 1. 1 > maxLanes >= minLanes > 0
        // 2. maxLanes >= 1 >= minLanes > 0
        // 3. maxLanes >= minLanes > 1

        // 1. 1 > maxLanes >= minLanes > 0 ---> maxCrossSize < minLaneLength_ =< maxLaneLength
        if (GreatNotEqual(1, maxLanes) && GreatOrEqual(maxLanes, minLanes)) {
            lanes = 1;
            minLaneLength_ = maxCrossSize;
            maxLaneLength_ = maxCrossSize;
            break;
        }
        // 2. maxLanes >= 1 >= minLanes > 0 ---> minLaneLength_ = maxCrossSize < maxLaneLength
        if (GreatOrEqual(maxLanes, 1) && LessOrEqual(minLanes, 1)) {
            lanes = std::floor(maxLanes);
            maxLaneLength_ = maxCrossSize;
            break;
        }
        // 3. maxLanes >= minLanes > 1 ---> minLaneLength_ <= maxLaneLength < maxCrossSize
        if (GreatOrEqual(maxLanes, minLanes) && GreatNotEqual(minLanes, 1)) {
            lanes = std::floor(maxLanes);
            break;
        }
        lanes = 1;
        LOGE("unexpected situation, set lanes to 1, maxLanes: %{public}f, minLanes: %{public}f, minLaneLength_: "
            "%{public}f, maxLaneLength_: %{public}f", maxLanes, minLanes, minLaneLength_, maxLaneLength_);
    } while (0);
    if (lanes_ != lanes) { // if lanes changes, layout from item 0
        lanes_ = lanes;
        currentOffset_ = 0.0;
        startIndex_ = 0;
    }
}

void RenderList::RequestNewItemsAtEndForLaneList(double& curMainPos, double mainSize, const LayoutParam& innerLayout)
{
    int newItemCntInLine = 0;
    double lineMainSize = 0;
    for (size_t newIndex = startIndex_ + items_.size();; ++newIndex) {
        bool doneRequestNewItem = false;
        do {
            if (GreatOrEqual(curMainPos, endMainPos_)) {
                doneRequestNewItem = true;
                break;
            }
            auto child = RequestAndLayoutNewItem(newIndex, innerLayout);
            if (!child) {
                startIndex_ = std::min(startIndex_, TotalCount());
                doneRequestNewItem = true;
                break;
            }
            if (GreatOrEqual(curMainPos, mainSize)) {
                ++endCachedCount_;
            }
            lineMainSize = std::max(lineMainSize, GetMainSize(child->GetLayoutSize()));
        } while (0);
        if (++newItemCntInLine == lanes_ || doneRequestNewItem) {
            curMainPos += lineMainSize + spaceWidth_;
            newItemCntInLine = 0;
            lineMainSize = 0;
            if (doneRequestNewItem) {
                break;
            }
        }
    }
}

void RenderList::RequestNewItemsAtEnd(double& curMainPos, double mainSize, const LayoutParam& innerLayout)
{
    for (size_t newIndex = startIndex_ + items_.size();; ++newIndex) {
        if (cachedCount_ != 0) {
            if (endCachedCount_ >= cachedCount_) {
                break;
            }
        } else {
            if (GreatOrEqual(curMainPos, endMainPos_)) {
                break;
            }
        }
        auto child = RequestAndLayoutNewItem(newIndex, innerLayout);
        if (!child) {
            startIndex_ = std::min(startIndex_, TotalCount());
            break;
        }
        if (GreatOrEqual(curMainPos, mainSize)) {
            ++endCachedCount_;
        }
        curMainPos += GetMainSize(child->GetLayoutSize()) + spaceWidth_;
    }

    if (selectedItem_ && selectedItemIndex_ < startIndex_) {
        curMainPos += GetMainSize(selectedItem_->GetLayoutSize()) + spaceWidth_;
    }
}

void RenderList::RequestNewItemsAtStartForLaneList(const LayoutParam& innerLayout)
{
    int newItemCntInLine = 0;
    double lineMainSize = 0;
    for (; startIndex_ > 0; --startIndex_) {
        bool doneRequestNewItem = false;
        do {
            if (LessOrEqual(currentOffset_, startMainPos_)) {
                doneRequestNewItem = true;
                break;
            }
            auto child = RequestAndLayoutNewItem(startIndex_ - 1, innerLayout);
            if (!child) {
                doneRequestNewItem = true;
                break;
            }
            if (selectedItemIndex_ == startIndex_) {
                continue;
            }
            if (LessOrEqual(currentOffset_, 0.0)) {
                ++startCachedCount_;
            }
            lineMainSize = std::max(lineMainSize, GetMainSize(child->GetLayoutSize()));
        } while (0);
        if (++newItemCntInLine == lanes_ || (doneRequestNewItem)) {
            currentOffset_ = currentOffset_ - lineMainSize;
            newItemCntInLine = 0;
            lineMainSize = 0;
            if (doneRequestNewItem) {
                break;
            }
        }
    }
}

void RenderList::RequestNewItemsAtStart(const LayoutParam& innerLayout)
{
    for (; startIndex_ > 0; --startIndex_) {
        if (cachedCount_ != 0) {
            if (startCachedCount_ >= cachedCount_) {
                break;
            }
        } else {
            if (LessOrEqual(currentOffset_, startMainPos_)) {
                break;
            }
        }
        auto child = RequestAndLayoutNewItem(startIndex_ - 1, innerLayout);
        if (!child) {
            break;
        }
        if (selectedItemIndex_ == startIndex_) {
            continue;
        }
        if (LessOrEqual(currentOffset_, 0.0)) {
            ++startCachedCount_;
        }
        currentOffset_ -= GetMainSize(child->GetLayoutSize()) + spaceWidth_;
    }
}

void RenderList::PerformLayout()
{
    UpdateAccessibilityAttr();
    // Check validation of layout size
    const double mainSize = ApplyLayoutParam();
    if (NearZero(mainSize)) {
        LOGW("Cannot layout using invalid view port");
        return;
    }
    if (isLaneList_) {
        CalculateLanes();
    }
    const auto innerLayout = MakeInnerLayout();
    double curMainPos = 0.0;
    if (isLaneList_) {
        curMainPos = LayoutOrRecycleCurrentItemsForLaneList(innerLayout, mainSize);
    } else {
        curMainPos = LayoutOrRecycleCurrentItems(innerLayout, mainSize);
    }

    // Try to request new items at end if needed
    if (isLaneList_) {
        RequestNewItemsAtEndForLaneList(curMainPos, mainSize, innerLayout);
    } else {
        RequestNewItemsAtEnd(curMainPos, mainSize, innerLayout);
    }

    if (selectedItem_ && selectedItemIndex_ < startIndex_) {
        curMainPos += GetMainSize(selectedItem_->GetLayoutSize()) + spaceWidth_;
    }

    curMainPos -= spaceWidth_;

    // Check if reach the end of list
    reachEnd_ = LessOrEqual(curMainPos, mainSize);
    bool noEdgeEffect =
        (scrollable_ && scrollable_->IsAnimationNotRunning()) || !scrollEffect_ || autoScrollingForItemMove_;
    if (noEdgeEffect && reachEnd_) {
        // Adjust end of list to match the end of layout
        currentOffset_ += mainSize - curMainPos;
        curMainPos = mainSize;
    }

    // Try to request new items at start if needed
    if (isLaneList_) {
        RequestNewItemsAtStartForLaneList(innerLayout);
    } else {
        RequestNewItemsAtStart(innerLayout);
    }

    if (IsReachStart()) {
        listEventFlags_[ListEvents::REACH_START] = true;
    }
    // Check if reach the start of list
    reachStart_ = GreatOrEqual(currentOffset_, 0.0);
    if (noEdgeEffect && reachStart_) {
        curMainPos -= currentOffset_;
        currentOffset_ = 0;
    }

    bool scrollDownToReachEnd = LessNotEqual(prevMainPos_, mainSize) && GreatOrEqual(curMainPos, mainSize);
    bool scrollUpToReachEnd = GreatNotEqual(prevMainPos_, mainSize) && LessOrEqual(curMainPos, mainSize);
    // verify layout size to avoid trigger reach_end event at first [PerformLayout] when layout size is zero
    if ((scrollDownToReachEnd || scrollUpToReachEnd) && GetLayoutSize().IsValid()) {
        listEventFlags_[ListEvents::REACH_END] = true;
    }
    if (!fixedMainSize_) {
        fixedMainSize_ = !(reachStart_ && reachEnd_);
    }
    // Check if disable or enable scrollable
    CalculateMainScrollExtent(curMainPos, mainSize);

    // Set position for each child
    Size layoutSize;
    if (isLaneList_) {
        layoutSize = SetItemsPositionForLaneList(mainSize, innerLayout);
    } else {
        layoutSize = SetItemsPosition(mainSize, innerLayout);
    }

    // Set layout size of list component itself
    if ((hasHeight_ && vertical_) || (hasWidth_ && !vertical_)) {
        SetLayoutSize(GetLayoutParam().GetMaxSize());
    } else {
        SetLayoutSize(GetLayoutParam().Constrain(layoutSize));
    }

    // Clear auto scrolling flags
    autoScrollingForItemMove_ = false;
    if (scrollable_ && scrollable_->Idle()) {
        ResumeEventCallback(component_, &ListComponent::GetOnScroll, Dimension(offset_ / dipScale_, DimensionUnit::VP),
            ScrollState(SCROLL_STATE_IDLE));
    }

    realMainSize_ = curMainPos - currentOffset_;
    HandleListEvent();
    prevOffset_ = currentOffset_;
    prevMainPos_ = curMainPos;
}

#define CASE_OF_LIST_EVENT_WITH_NO_PARAM(eventNumber, callback)        \
    case ListEvents::eventNumber:                                      \
        if (event.second) {                                            \
            ResumeEventCallback(component_, &ListComponent::callback); \
            LOGD("list event %{public}s triggered.", #eventNumber);    \
            event.second = false;                                      \
        }                                                              \
        break;

void RenderList::HandleListEvent()
{
    for (auto& event : listEventFlags_) {
        switch (event.first) {
            CASE_OF_LIST_EVENT_WITH_NO_PARAM(SCROLL_STOP, GetOnScrollStop);
            CASE_OF_LIST_EVENT_WITH_NO_PARAM(REACH_START, GetOnReachStart);
            CASE_OF_LIST_EVENT_WITH_NO_PARAM(REACH_END, GetOnReachEnd);
            default:
                LOGW("This event does not handle in here, please check. event number: %{public}d", event.first);
                break;
        }
    }
}

double RenderList::CalculateLaneCrossOffset(double crossSize, double childCrossSize)
{
    double delta = crossSize - childCrossSize;
    // TODO: modify in rtl senario
    switch (component_->GetAlignListItemAlign()) {
        case ListItemAlign::START:
            return 0.0;
        case ListItemAlign::CENTER:
            return delta / CENTER_ALIGN_DIVIDER;
        case ListItemAlign::END:
            return delta;
        default:
            LOGW("Invalid ListItemAlign: %{public}d", component_->GetAlignListItemAlign());
            return 0.0;
    }
}

Size RenderList::SetItemsPositionForLaneList(double mainSize, const LayoutParam& layoutParam)
{
    double crossSize = fixedCrossSize_ ? GetCrossSize(GetLayoutParam().GetMaxSize()) : 0.0;
    if (items_.empty()) {
        return MakeValue<Size>(fixedMainSize_ ? mainSize : 0.0, crossSize);
    }

    double curMainPos = currentOffset_;
    size_t index = startIndex_;
    size_t newStickyIndex = 0;
    RefPtr<RenderListItem> newStickyItem;
    RefPtr<RenderListItem> nextStickyItem;
    double nextStickyMainAxis = Size::INFINITE_SIZE;
    size_t firstIdx = INITIAL_CHILD_INDEX;
    size_t lastIdx = 0;
    double selectedItemMainSize = selectedItem_ ? GetMainSize(selectedItem_->GetLayoutSize()) : 0.0;

    double totalLaneCrossSize = 0;
    for (auto iter = items_.begin(); iter != items_.end();) {
        RefPtr<RenderListItem> child;
        double childMainSize = 0.0;
        double childCrossSize = 0.0;
        std::vector<RefPtr<RenderListItem>> itemSet;
        // start set child position in a row
        for (int32_t i = 0; i < lanes_; i++) {
            child = *iter;
            double singleChildSize = GetMainSize(child->GetLayoutSize());
            childCrossSize += GetCrossSize(child->GetLayoutSize());
            childMainSize = std::max(childMainSize, singleChildSize); // get max item height in a row as row height
            // store items in a row, set position of each item after done getting [childMainSize]
            itemSet.emplace_back(child);
            if ((++iter) == items_.end()) {
                break;
            }
        }
        totalLaneCrossSize = std::max(childCrossSize, totalLaneCrossSize);
        auto offsetCross = CalculateLaneCrossOffset(crossSize, totalLaneCrossSize);
        auto offset = MakeValue<Offset>(curMainPos, offsetCross);
        if (chainAnimation_) {
            offset += MakeValue<Offset>(-GetChainDelta(index), 0.0);
        }
        // set item position for one row
        for (size_t i = 0; i < itemSet.size(); i++) {
            auto position = offset + MakeValue<Offset>(0.0, childCrossSize / itemSet.size() * i);
            itemSet[i]->SetPosition(position);
        }

        if (lanes_ == 1) {
            if (selectedItem_) {
                double range = std::min(selectedItemMainSize, childMainSize) / 2.0;
                bool beforeSelectedItem = index <= selectedItemIndex_;
                if (beforeSelectedItem && targetIndex_ == index) {
                    targetMainAxis_ = curMainPos;
                    curMainPos += selectedItemMainSize + spaceWidth_;
                }

                if (movingForward_) {
                    double axis = selectedItemMainAxis_;
                    if (GreatOrEqual(axis, curMainPos) && LessNotEqual(axis, curMainPos + range)) {
                        targetIndex_ = beforeSelectedItem ? index : index - 1;
                        targetMainAxis_ = curMainPos;
                        curMainPos += selectedItemMainSize + spaceWidth_;
                    }
                } else {
                    double axis = selectedItemMainAxis_ + selectedItemMainSize;
                    double limit = curMainPos + childMainSize;
                    if (GreatNotEqual(axis, limit - range) && LessOrEqual(axis, limit)) {
                        targetIndex_ = beforeSelectedItem ? index + 1 : index;
                        targetMainAxis_ = curMainPos;
                        curMainPos -= selectedItemMainSize + spaceWidth_;
                    }
                }
            }

            // Disable sticky mode while expand all items
            if (fixedMainSize_ && itemSet[0]->GetSticky() != StickyMode::NONE) {
                if (LessOrEqual(curMainPos, 0.0)) {
                    newStickyItem = itemSet[0];
                    newStickyIndex = index;
                } else if (!nextStickyItem) {
                    nextStickyItem = itemSet[0];
                    nextStickyMainAxis = curMainPos;
                }
            }
        }
        itemSet.clear();

        childMainSize += spaceWidth_;
        if (LessNotEqual(curMainPos, mainSize) && GreatNotEqual(curMainPos + childMainSize, 0.0)) {
            if (!fixedCrossSize_) {
                crossSize = std::max(crossSize, childCrossSize);
            }
            firstIdx = std::min(firstIdx, index);
            lastIdx = std::max(lastIdx, index);
        }

        if (child != selectedItem_) {
            curMainPos += childMainSize;
        }

        if (selectedItem_ && index > selectedItemIndex_ && targetIndex_ == index) {
            targetMainAxis_ = curMainPos;
            curMainPos += selectedItemMainSize + spaceWidth_;
        }

        ++index;
    }
    if (firstIdx != firstDisplayIndex_ || lastIdx != lastDisplayIndex_) {
        firstDisplayIndex_ = firstIdx;
        lastDisplayIndex_ = lastIdx;
        ResumeEventCallback(component_, &ListComponent::GetOnScrollIndex, static_cast<int32_t>(firstDisplayIndex_),
            static_cast<int32_t>(lastDisplayIndex_));
    }

    // Disable sticky mode while expand all items
    if (!fixedMainSize_) {
        return MakeValue<Size>(curMainPos - spaceWidth_, crossSize);
    }

    if (lanes_ == 1) {
        UpdateStickyListItem(newStickyItem, newStickyIndex, nextStickyItem, layoutParam);
        if (currentStickyItem_) {
            const auto& stickyItemLayoutSize = currentStickyItem_->GetLayoutSize();
            const double mainStickySize = GetMainSize(stickyItemLayoutSize) + spaceWidth_;
            if (nextStickyItem && LessNotEqual(nextStickyMainAxis, mainStickySize)) {
                currentStickyItem_->SetPosition(MakeValue<Offset>(nextStickyMainAxis - mainStickySize, 0.0));
            } else {
                currentStickyItem_->SetPosition(MakeValue<Offset>(0.0, 0.0));
            }

            if (!fixedCrossSize_) {
                crossSize = std::max(crossSize, GetCrossSize(stickyItemLayoutSize));
            }
        }
    }

    return MakeValue<Size>(mainSize, crossSize);
}

Size RenderList::SetItemsPosition(double mainSize, const LayoutParam& layoutParam)
{
    double crossSize = fixedCrossSize_ ? GetCrossSize(GetLayoutParam().GetMaxSize()) : 0.0;
    if (items_.empty()) {
        return MakeValue<Size>(fixedMainSize_ ? mainSize : 0.0, crossSize);
    }

    double curMainPos = currentOffset_;
    size_t index = startIndex_;
    size_t newStickyIndex = 0;
    RefPtr<RenderListItem> newStickyItem;
    RefPtr<RenderListItem> nextStickyItem;
    double nextStickyMainAxis = Size::INFINITE_SIZE;
    size_t firstIdx = INITIAL_CHILD_INDEX;
    size_t lastIdx = 0;
    double selectedItemMainSize = selectedItem_ ? GetMainSize(selectedItem_->GetLayoutSize()) : 0.0;

    for (const auto& child : items_) {
        const auto& childLayoutSize = child->GetLayoutSize();
        double childMainSize = GetMainSize(childLayoutSize);

        if (selectedItem_) {
            double range = std::min(selectedItemMainSize, childMainSize) / 2.0;
            bool beforeSelectedItem = index <= selectedItemIndex_;
            if (beforeSelectedItem && targetIndex_ == index) {
                targetMainAxis_ = curMainPos;
                curMainPos += selectedItemMainSize + spaceWidth_;
            }

            if (movingForward_) {
                double axis = selectedItemMainAxis_;
                if (GreatOrEqual(axis, curMainPos) && LessNotEqual(axis, curMainPos + range)) {
                    targetIndex_ = beforeSelectedItem ? index : index - 1;
                    targetMainAxis_ = curMainPos;
                    curMainPos += selectedItemMainSize + spaceWidth_;
                }
            } else {
                double axis = selectedItemMainAxis_ + selectedItemMainSize;
                double limit = curMainPos + childMainSize;
                if (GreatNotEqual(axis, limit - range) && LessOrEqual(axis, limit)) {
                    targetIndex_ = beforeSelectedItem ? index + 1 : index;
                    targetMainAxis_ = curMainPos;
                    curMainPos -= selectedItemMainSize + spaceWidth_;
                }
            }
        }

        auto offset = MakeValue<Offset>(curMainPos, 0.0);
        if (chainAnimation_) {
            offset += MakeValue<Offset>(-GetChainDelta(index), 0.0);
        }

        child->SetPosition(offset);
        // Disable sticky mode while expand all items
        if (fixedMainSize_ && child->GetSticky() != StickyMode::NONE) {
            if (LessOrEqual(curMainPos, 0.0)) {
                newStickyItem = child;
                newStickyIndex = index;
            } else if (!nextStickyItem) {
                nextStickyItem = child;
                nextStickyMainAxis = curMainPos;
            }
        }

        childMainSize += spaceWidth_;
        if (LessNotEqual(curMainPos, mainSize) && GreatNotEqual(curMainPos + childMainSize, 0.0)) {
            if (!fixedCrossSize_) {
                crossSize = std::max(crossSize, GetCrossSize(childLayoutSize));
            }
            firstIdx = std::min(firstIdx, index);
            lastIdx = std::max(lastIdx, index);
        }

        if (child != selectedItem_) {
            curMainPos += childMainSize;
        }

        if (selectedItem_ && index > selectedItemIndex_ && targetIndex_ == index) {
            targetMainAxis_ = curMainPos;
            curMainPos += selectedItemMainSize + spaceWidth_;
        }

        ++index;
    }
    if (firstIdx != firstDisplayIndex_ || lastIdx != lastDisplayIndex_) {
        firstDisplayIndex_ = firstIdx;
        lastDisplayIndex_ = lastIdx;
        ResumeEventCallback(component_, &ListComponent::GetOnScrollIndex, static_cast<int32_t>(firstDisplayIndex_),
            static_cast<int32_t>(lastDisplayIndex_));
    }

    // Disable sticky mode while expand all items
    if (!fixedMainSize_) {
        return MakeValue<Size>(curMainPos - spaceWidth_, crossSize);
    }

    UpdateStickyListItem(newStickyItem, newStickyIndex, nextStickyItem, layoutParam);
    if (currentStickyItem_) {
        const auto& stickyItemLayoutSize = currentStickyItem_->GetLayoutSize();
        const double mainStickySize = GetMainSize(stickyItemLayoutSize) + spaceWidth_;
        if (nextStickyItem && LessNotEqual(nextStickyMainAxis, mainStickySize)) {
            currentStickyItem_->SetPosition(MakeValue<Offset>(nextStickyMainAxis - mainStickySize, 0.0));
        } else {
            currentStickyItem_->SetPosition(MakeValue<Offset>(0.0, 0.0));
        }

        if (!fixedCrossSize_) {
            crossSize = std::max(crossSize, GetCrossSize(stickyItemLayoutSize));
        }
    }

    return MakeValue<Size>(mainSize, crossSize);
}

void RenderList::UpdateStickyListItem(const RefPtr<RenderListItem>& newStickyItem, size_t newStickyItemIndex,
    const RefPtr<RenderListItem>& nextStickyItem, const LayoutParam& layoutParam)
{
    if (newStickyItem) {
        if (newStickyItem == currentStickyItem_) {
            return;
        }

        if (currentStickyItem_ && currentStickyIndex_ < startIndex_) {
            RecycleListItem(currentStickyIndex_);
        }

        currentStickyItem_ = newStickyItem;
        currentStickyIndex_ = newStickyItemIndex;
        return;
    }

    if (nextStickyItem && nextStickyItem == currentStickyItem_) {
        ApplyPreviousStickyListItem(currentStickyIndex_ - 1, true, layoutParam);
        return;
    }

    if (currentStickyIndex_ == INITIAL_CHILD_INDEX && startIndex_ > 0) {
        ApplyPreviousStickyListItem(startIndex_ - 1, true, layoutParam);
    }
}

LayoutParam RenderList::MakeInnerLayout()
{
    Size maxSize;
    Size minSize;
    if (isLaneList_) {
        if (vertical_) {
            maxSize =
                Size(std::min(GetLayoutParam().GetMaxSize().Width() / lanes_, maxLaneLength_), Size::INFINITE_SIZE);
            minSize = Size(GetLayoutParam().GetMinSize().Width(), 0.0);
        } else {
            maxSize =
                Size(Size::INFINITE_SIZE, std::min(GetLayoutParam().GetMaxSize().Height() / lanes_, maxLaneLength_));
            minSize = Size(0.0, GetLayoutParam().GetMinSize().Height());
        }
        return LayoutParam(maxSize, minSize);
    }
    if (vertical_) {
        maxSize = Size(GetLayoutParam().GetMaxSize().Width(), Size::INFINITE_SIZE);
        minSize = Size(GetLayoutParam().GetMinSize().Width(), 0.0);
    } else {
        maxSize = Size(Size::INFINITE_SIZE, GetLayoutParam().GetMaxSize().Height());
        minSize = Size(0.0, GetLayoutParam().GetMinSize().Height());
    }
    return LayoutParam(maxSize, minSize);
}

bool RenderList::GetCurMainPosAndMainSize(double& curMainPos, double& mainSize)
{
    // Check validation of layout size
    mainSize = ApplyLayoutParam();
    if (NearZero(mainSize)) {
        LOGW("Cannot layout using invalid view port");
        return false;
    }
    const auto innerLayout = MakeInnerLayout();
    if (isLaneList_) {
        curMainPos = LayoutOrRecycleCurrentItemsForLaneList(innerLayout, mainSize);
    } else {
        curMainPos = LayoutOrRecycleCurrentItems(innerLayout, mainSize);
    }
    // Try to request new items at end if needed
    for (size_t newIndex = startIndex_ + items_.size();; ++newIndex) {
        if (cachedCount_ != 0) {
            if (endCachedCount_ >= cachedCount_) {
                break;
            }
        } else {
            if (GreatOrEqual(curMainPos, endMainPos_)) {
                break;
            }
        }
        auto child = RequestAndLayoutNewItem(newIndex, innerLayout);
        if (!child) {
            startIndex_ = std::min(startIndex_, TotalCount());
            break;
        }
        if (GreatOrEqual(curMainPos, mainSize)) {
            ++endCachedCount_;
        }
        curMainPos += GetMainSize(child->GetLayoutSize()) + spaceWidth_;
    }
    if (selectedItem_ && selectedItemIndex_ < startIndex_) {
        curMainPos += GetMainSize(selectedItem_->GetLayoutSize()) + spaceWidth_;
    }
    curMainPos -= spaceWidth_;
    return true;
}

bool RenderList::UpdateScrollPosition(double offset, int32_t source)
{
    prevOffset_ = currentOffset_;
    if (source == SCROLL_FROM_START) {
        return true;
    }

    if (NearZero(offset)) {
        return true;
    }

    if (scrollBar_ && scrollBar_->NeedScrollBar()) {
        scrollBar_->SetActive(SCROLL_FROM_CHILD != source);
    }

    if (reachStart_ && reachEnd_) {
        return false;
    }

    if (offset > 0.0) {
        if (reachStart_ && (!scrollEffect_ || source == SCROLL_FROM_AXIS)) {
            return false;
        }
        reachEnd_ = false;
    } else {
        if (reachEnd_ && (!scrollEffect_ || source == SCROLL_FROM_AXIS)) {
            return false;
        }
        reachStart_ = false;
    }

    auto context = context_.Upgrade();
    if (context) {
        dipScale_ = context->GetDipScale();
    }
    offset_ = offset;
    if (source == SCROLL_FROM_UPDATE) {
        ResumeEventCallback(component_, &ListComponent::GetOnScroll, Dimension(offset_ / dipScale_, DimensionUnit::VP),
            ScrollState(SCROLL_STATE_SCROLL));
    } else if (source == SCROLL_FROM_ANIMATION) {
        ResumeEventCallback(component_, &ListComponent::GetOnScroll, Dimension(offset_ / dipScale_, DimensionUnit::VP),
            ScrollState(SCROLL_STATE_FLING));
    }
    currentOffset_ += offset;
    if (source == SCROLL_FROM_AXIS) {
        double curMainPos = 0.0;
        double mainSize = 0.0;
        GetCurMainPosAndMainSize(curMainPos, mainSize);
        if (currentOffset_ < 0 && curMainPos < mainSize) {
            currentOffset_ += mainSize - curMainPos;
        } else if (currentOffset_ > 0) {
            currentOffset_ = 0;
        }
    }
    MarkNeedLayout(true);
    return true;
}

bool RenderList::TouchTest(const Point& globalPoint, const Point& parentLocalPoint, const TouchRestrict& touchRestrict,
    TouchTestResult& result)
{
    if (GetVisible() && fixedMainSize_ && scrollable_ && scrollable_->Available()) {
        result.emplace_back(scrollable_);
    }
    // when click point is in sticky item, consume the touch event to avoid clicking on the list item underneath.
    if (currentStickyItem_ && currentStickyItem_->GetPaintRect().IsInRegion(parentLocalPoint)) {
        currentStickyItem_->TouchTest(globalPoint, parentLocalPoint, touchRestrict, result);
        return true;
    }

    return RenderNode::TouchTest(globalPoint, parentLocalPoint, touchRestrict, result);
}

void RenderList::OnTouchTestHit(
    const Offset& coordinateOffset, const TouchRestrict& touchRestrict, TouchTestResult& result)
{
    if (!GetVisible()) {
        return;
    }

    if (component_->GetEditMode() && dragDropGesture_) {
        dragDropGesture_->SetCoordinateOffset(coordinateOffset);
        result.emplace_back(dragDropGesture_);
    }

    // Disable scroll while expand all items
    if (!fixedMainSize_) {
        return;
    }
    if (!scrollable_ || !scrollable_->Available()) {
        return;
    }
    if (scrollBar_ && scrollBar_->InBarRegion(globalPoint_ - coordinateOffset)) {
        scrollBar_->AddScrollBarController(coordinateOffset, result);
    } else {
        scrollable_->SetCoordinateOffset(coordinateOffset);
    }
    result.emplace_back(scrollable_);
}

double RenderList::ApplyLayoutParam()
{
    auto maxLayoutSize = GetLayoutParam().GetMaxSize();
    if (!maxLayoutSize.IsValid() || maxLayoutSize.IsEmpty()) {
        return 0.0;
    }

    auto maxMainSize = GetMainSize(maxLayoutSize);

    // Update layout info for list weather layout param is changed
    if (IsLayoutParamChanged()) {
        // Minimum layout param MUST NOT be INFINITE
        ACE_DCHECK(!GetLayoutParam().GetMinSize().IsInfinite());

        if (NearEqual(maxMainSize, Size::INFINITE_SIZE)) {
            // Clear all child items
            RemoveAllItems();
            startIndex_ = 0;
            currentOffset_ = 0.0;

            startMainPos_ = 0.0;
            endMainPos_ = std::numeric_limits<decltype(endMainPos_)>::max();
            fixedMainSizeByLayoutParam_ = false;
        } else {
            startMainPos_ = (1.0 - VIEW_PORT_SCALE) / 2 * maxMainSize;
            endMainPos_ = startMainPos_ + (maxMainSize * VIEW_PORT_SCALE);
            fixedMainSizeByLayoutParam_ = NearEqual(maxMainSize, GetMainSize(GetLayoutParam().GetMinSize()));
        }

        fixedCrossSize_ = !NearEqual(GetCrossSize(maxLayoutSize), Size::INFINITE_SIZE);
        TakeBoundary(fixedMainSizeByLayoutParam_ && fixedCrossSize_);
    }

    fixedMainSize_ = fixedMainSizeByLayoutParam_;
    return maxMainSize;
}

ItemPositionState RenderList::GetItemPositionState(double curMainPos, double lastItemMainSize)
{
    // curMainPos <= startMainPos_
    if (LessOrEqual(curMainPos, startMainPos_)) {
        return ItemPositionState::AHEAD_OF_VIEWPORT;
    }
    // (curMainPos > startMainPos_) and ((curMainPos <= endMainPos_) or (curMainPos - lastItemMainSize <= endMainPos_))
    if (LessOrEqual(curMainPos, endMainPos_) || (curMainPos - lastItemMainSize <= endMainPos_)) {
        return ItemPositionState::IN_VIEWPORT;
    }
    // curMainPos_ - lastItemMainSize > endMainPos_
    if (GreatNotEqual(curMainPos, endMainPos_)) {
        return ItemPositionState::BEHIND_VIEWPORT;
    }
    LOGE("invalid place of list item, curMainPos: %{public}f", curMainPos);
    return ItemPositionState::IN_VIEWPORT;
}

#define RECYCLE_AND_ERASE_ITEMS_OUT_OF_VIEWPORT()                                            \
    do                                                                                       \
    {                                                                                        \
        for (size_t i = 0; i < itemsInOneRow.size(); i++)                                    \
        {                                                                                    \
            if (currentStickyItem_ != itemsInOneRow[i] && selectedItem_ != itemsInOneRow[i]) \
            {                                                                                \
                /* Recycle list items out of view port */                                    \
                RecycleListItem(curIndex - i);                                               \
            }                                                                                \
            it = items_.erase(--it);                                                         \
        }                                                                                    \
    } while (0);

double RenderList::LayoutOrRecycleCurrentItemsForLaneList(const LayoutParam& layoutParam, double mainSize)
{
    if (currentStickyItem_) {
        currentStickyItem_->Layout(layoutParam);
    }

    double curMainPos = currentOffset_;
    size_t curIndex = startIndex_ - 1;
    std::vector<RefPtr<RenderListItem>> itemsInOneRow;
    int32_t lackItemCount = 0;
    for (auto it = items_.begin(); it != items_.end();) {
        // 1. layout children in a row
        double mainSize;
        itemsInOneRow.clear();
        for (int32_t i = 0; i < lanes_; i++) {
            RefPtr<RenderListItem> child = *(it);
            child->Layout(layoutParam);
            double childMainSize = GetMainSize(child->GetLayoutSize());
            mainSize = std::max(mainSize, childMainSize);
            itemsInOneRow.emplace_back(child);
            ++curIndex;
            if (++it == items_.end()) {
                // reach end of [items_]
                lackItemCount = lanes_ - i;
                break;
            }
        }
        // 2. calculate [curMainPos] after layout current row, deciding whether or not to request new item to fill
        // current row or to erase and recycle items in current row
        curMainPos += mainSize + spaceWidth_;

        // 3. do different processing according to item position state
        auto itemPositionState = GetItemPositionState(curMainPos, mainSize + spaceWidth_);
        switch (itemPositionState) {
            // when items are ahead of viewport, do the following things:
            // 1. update [startIndex_] and [currentOffset_]
            // 2. recycle items and erase them from [items_]
            case ItemPositionState::AHEAD_OF_VIEWPORT: {
                startIndex_ = curIndex + 1;
                currentOffset_ = curMainPos;
                RECYCLE_AND_ERASE_ITEMS_OUT_OF_VIEWPORT();
                break;
            }
            // when items are in viewport, continue to layout next row
            // if current item is the last one in [items_], request new items to supply current row
            case ItemPositionState::IN_VIEWPORT: {
                if (lanes_ == 1) {
                    continue; // if list only has one lane, do not need to do suppliment for current row
                }
                size_t target = lackItemCount + items_.size() + startIndex_;
                for (size_t newIndex = startIndex_ + items_.size(); newIndex < target; newIndex++) {
                    auto child = RequestAndLayoutNewItem(newIndex, layoutParam);
                    if (!child) {
                        startIndex_ = std::min(startIndex_, TotalCount());
                        break;
                    }
                }
                break;
            }
            // when items are behind viewport, recycle items and erase them from [items_]
            case ItemPositionState::BEHIND_VIEWPORT: {
                RECYCLE_AND_ERASE_ITEMS_OUT_OF_VIEWPORT();
                curMainPos -= mainSize + spaceWidth_;
                break;
            }
            default:
                LOGW("unexpected item position state: %{public}d", itemPositionState);
                break;
        }
    }
    return curMainPos;
}

double RenderList::LayoutOrRecycleCurrentItems(const LayoutParam& layoutParam, double mainSize)
{
    if (currentStickyItem_) {
        currentStickyItem_->Layout(layoutParam);
    }

    double curMainPos = currentOffset_;
    size_t curIndex = startIndex_;
    if (cachedCount_ != 0) {
        startCachedCount_ = 0;
        endCachedCount_ = 0;
        bool recycleAll = false;
        if (GreatOrEqual(curMainPos, mainSize)) {
            recycleAll = true;
        }
        for (auto it = items_.begin(); it != items_.end(); ++curIndex) {
            const auto& child = *(it);
            if (recycleAll || endCachedCount_ >= cachedCount_) {
                if (currentStickyItem_ != child && selectedItem_ != child) {
                    // Recycle list items out of view port
                    RecycleListItem(curIndex);
                }
                it = items_.erase(it);
                continue;
            }

            if (GreatOrEqual(curMainPos, mainSize)) {
                ++endCachedCount_;
            }

            child->Layout(layoutParam);
            double childMainSize = GetMainSize(child->GetLayoutSize());
            curMainPos += childMainSize + spaceWidth_;

            if (LessOrEqual(curMainPos, 0.0)) {
                ++startCachedCount_;
            }
            ++it;
        }

        double curMainPosForRecycle = currentOffset_;
        curIndex = startIndex_;
        for (auto it = items_.begin(); it != items_.end(); ++curIndex) {
            if (startCachedCount_ > cachedCount_) {
                const auto& child = *(it);
                double childSize = GetMainSize(child->GetLayoutSize());
                curMainPosForRecycle += childSize + spaceWidth_;
                currentOffset_ = curMainPosForRecycle;
                startIndex_ = curIndex + 1;

                if (currentStickyItem_ != child && selectedItem_ != child) {
                    // Recycle list items out of view port
                    RecycleListItem(curIndex);
                }
                it = items_.erase(it);
                --startCachedCount_;
            } else {
                break;
            }
        }
    } else {
        for (auto it = items_.begin(); it != items_.end(); ++curIndex) {
            const auto& child = *(it);
            if (LessOrEqual(curMainPos, endMainPos_)) {
                child->Layout(layoutParam);
                double childSize = GetMainSize(child->GetLayoutSize());
                curMainPos += childSize + spaceWidth_;
                if (GreatOrEqual(curMainPos, startMainPos_)) {
                    ++it;
                    continue;
                }
                currentOffset_ = curMainPos;
                startIndex_ = curIndex + 1;
            }

            if (currentStickyItem_ != child && selectedItem_ != child) {
                // Recycle list items out of view port
                RecycleListItem(curIndex);
            }
            it = items_.erase(it);
        }
    }

    return curMainPos;
}

RefPtr<RenderListItem> RenderList::RequestAndLayoutNewItem(size_t index, const LayoutParam& layoutParam)
{
    ACE_FUNCTION_TRACE();
    RefPtr<RenderListItem> newChild;
    if (index == currentStickyIndex_ && currentStickyItem_) {
        newChild = currentStickyItem_;
    } else {
        newChild = RequestListItem(index);
        if (newChild) {
            AddChild(newChild);
            newChild->Layout(layoutParam);
        }
    }

    if (newChild) {
        if (index < startIndex_) {
            items_.emplace_front(newChild);
        } else {
            items_.emplace_back(newChild);
        }
    }
    return newChild;
}

RefPtr<RenderListItem> RenderList::RequestListItem(size_t index)
{
    auto generator = itemGenerator_.Upgrade();
    auto newItem = generator ? generator->RequestListItem(index) : RefPtr<RenderListItem>();
    if (!newItem) {
        return newItem;
    }

    if (component_->GetEditMode()) {
        newItem->SetEditMode(true);
        newItem->SetOnDeleteClick([weak = AceType::WeakClaim(this)](RefPtr<RenderListItem> item) {
            auto spThis = weak.Upgrade();
            if (!spThis) {
                return;
            }
            spThis->OnItemDelete(item);
        });

        newItem->SetOnSelect([weak = AceType::WeakClaim(this)](RefPtr<RenderListItem> item) {
            auto spThis = weak.Upgrade();
            if (!spThis) {
                return;
            }
            spThis->OnItemSelect(item);
        });
    }

    if (!newItem->GetVisible()) {
        newItem->SetVisible(true);
    }

    return newItem;
}

void RenderList::RecycleListItem(size_t index)
{
    auto generator = itemGenerator_.Upgrade();
    if (generator) {
        generator->RecycleListItem(index);
    }
}

size_t RenderList::TotalCount()
{
    auto generator = itemGenerator_.Upgrade();
    return generator ? generator->TotalCount() : 0;
}

size_t RenderList::FindPreviousStickyListItem(size_t index)
{
    auto generator = itemGenerator_.Upgrade();
    return generator ? generator->FindPreviousStickyListItem(index) : ListItemGenerator::INVALID_INDEX;
}

void RenderList::OnItemDelete(const RefPtr<RenderListItem>& item)
{
    size_t index = GetIndexByListItem(item);
    if (!ResumeEventCallback(component_, &ListComponent::GetOnItemDelete, false, static_cast<int32_t>(index))) {
        LOGI("User canceled, stop deleting item");
        return;
    }

    if (index < startIndex_) {
        --startIndex_;
    }
}

void RenderList::OnItemSelect(const RefPtr<RenderListItem>& item)
{
    targetIndex_ = GetIndexByListItem(item);
    selectedItemIndex_ = targetIndex_;
    selectedItem_ = item;
    selectedItemMainAxis_ = GetMainAxis(item->GetPosition());
    LOGI("Select list item %{private}zu to move", selectedItemIndex_);
}

size_t RenderList::GetIndexByListItem(const RefPtr<RenderListItem>& item) const
{
    ACE_DCHECK(item);

    auto it = std::find(items_.begin(), items_.end(), item);
    if (it != items_.end()) {
        int32_t offset = std::distance(items_.begin(), it);
        ACE_DCHECK(offset >= 0);
        return startIndex_ + offset;
    }

    ACE_DCHECK(fixedMainSize_);
    ACE_DCHECK(item == currentStickyItem_);
    return currentStickyIndex_;
}

void RenderList::RemoveAllItems()
{
    items_.clear();
    ClearChildren();
    currentStickyItem_.Reset();
    currentStickyIndex_ = INITIAL_CHILD_INDEX;
    isActionByScroll_ = false;
}

void RenderList::ApplyPreviousStickyListItem(size_t index, bool needLayout, const LayoutParam& layoutParam)
{
    size_t newIndex = FindPreviousStickyListItem(index);
    if (newIndex == ListItemGenerator::INVALID_INDEX) {
        currentStickyItem_.Reset();
        currentStickyIndex_ = INVALID_CHILD_INDEX;
        return;
    }

    currentStickyIndex_ = newIndex;
    currentStickyItem_ = RequestListItem(currentStickyIndex_);
    if (currentStickyIndex_ < startIndex_) {
        AddChild(currentStickyItem_);
        if (needLayout) {
            currentStickyItem_->Layout(layoutParam);
        }
    }
}

void RenderList::JumpToIndex(int32_t idx, int32_t source)
{
    RemoveAllItems();
    startIndex_ = static_cast<size_t>(idx);
    currentOffset_ = 0.0;
    MarkNeedLayout(true);
}

void RenderList::AnimateTo(const Dimension& position, float duration, const RefPtr<Curve>& curve)
{
    if (!animator_->IsStopped()) {
        animator_->Stop();
    }
    animator_->ClearInterpolators();
    auto pos = NormalizePercentToPx(position, GetDirection());
    auto animation =
        AceType::MakeRefPtr<CurveAnimation<double>>(GetCurrentPosition(), GetCurrentPosition() + pos, curve);
    animation->AddListener([weak = AceType::WeakClaim(this)](double pos) {
        auto renderList = weak.Upgrade();
        if (!renderList) {
            return;
        }
        if (renderList->scrollable_ && !renderList->scrollable_->IsSpringMotionRunning()) {
            Offset delta;
            if (renderList->vertical_) {
                delta.SetX(0.0);
                delta.SetY(pos - renderList->currentOffset_);
            } else {
                delta.SetX(pos - renderList->currentOffset_);
                delta.SetY(0.0);
            }
            renderList->UpdateScrollPosition(renderList->GetMainAxis(delta), SCROLL_FROM_JUMP);
        }
    });
    animator_->AddInterpolator(animation);
    animator_->SetDuration(std::min(duration, SCROLL_MAX_TIME));
    animator_->ClearStopListeners();
    animator_->Play();
}

void RenderList::AdjustOffset(Offset& delta, int32_t source)
{
    // when scrollEffect equal to none, no need to adjust offset
    if (!scrollEffect_) {
        return;
    }

    if (delta.IsZero() || source == SCROLL_FROM_ANIMATION || source == SCROLL_FROM_ANIMATION_SPRING) {
        return;
    }

    double viewPortSize = GetMainSize(GetPaintRect().GetSize());
    double offset = GetMainAxis(delta);
    if (NearZero(viewPortSize) || NearZero(offset)) {
        return;
    }

    double maxScrollExtent = mainScrollExtent_ - viewPortSize;
    double overscrollPastStart = 0.0;
    double overscrollPastEnd = 0.0;
    double overscrollPast = 0.0;
    bool easing = false;

    overscrollPastStart = std::max(GetCurrentPosition(), 0.0);
    overscrollPastEnd = std::max(-GetCurrentPosition() - maxScrollExtent, 0.0);
    // do not adjust offset if direction oppsite from the overScroll direction when out of boundary
    if ((overscrollPastStart > 0.0 && offset < 0.0) || (overscrollPastEnd > 0.0 && offset > 0.0)) {
        return;
    }
    easing = (overscrollPastStart > 0.0 && offset > 0.0) || (overscrollPastEnd > 0.0 && offset < 0.0);

    overscrollPast = std::max(overscrollPastStart, overscrollPastEnd);
    double friction = easing ? RenderScroll::CalculateFriction((overscrollPast - std::abs(offset)) / viewPortSize)
                             : RenderScroll::CalculateFriction(overscrollPast / viewPortSize);
    double direction = offset / std::abs(offset);
    offset = direction * RenderScroll::CalculateOffsetByFriction(overscrollPast, std::abs(offset), friction);
    vertical_ ? delta.SetY(offset) : delta.SetX(offset);
}

double RenderList::GetCurrentPosition() const
{
    return currentOffset_;
}

bool RenderList::IsOutOfBoundary() const
{
    return isOutOfBoundary_;
}

void RenderList::ResetEdgeEffect()
{
    if (!scrollEffect_) {
        LOGE("ResetEdgeEffect failed, scrollEffect_ is nullptr");
        return;
    }

    scrollEffect_->SetCurrentPositionCallback([weak = AceType::WeakClaim(this)]() {
        auto list = weak.Upgrade();
        if (list) {
            return list->GetCurrentPosition();
        }
        return 0.0;
    });
    scrollEffect_->SetLeadingCallback([weak = AceType::WeakClaim(this)]() {
        auto list = weak.Upgrade();
        if (list) {
            return list->GetMainSize(list->GetLayoutSize()) - list->mainScrollExtent_;
        }
        return 0.0;
    });

    scrollEffect_->SetTrailingCallback([weak = AceType::WeakClaim(this)]() { return 0.0; });
    scrollEffect_->SetInitLeadingCallback([weak = AceType::WeakClaim(this)]() {
        auto list = weak.Upgrade();
        if (list) {
            return list->GetMainSize(list->GetLayoutSize()) - list->mainScrollExtent_;
        }
        return 0.0;
    });
    scrollEffect_->SetInitTrailingCallback([weak = AceType::WeakClaim(this)]() { return 0.0; });
    scrollEffect_->SetScrollNode(AceType::WeakClaim(this));
    SetEdgeEffectAttribute();
    scrollEffect_->InitialEdgeEffect();
}

void RenderList::SetEdgeEffectAttribute()
{
    if (scrollEffect_ && scrollable_) {
        scrollEffect_->SetScrollable(scrollable_);
        scrollEffect_->RegisterSpringCallback();
        if (scrollEffect_->IsSpringEffect()) {
            scrollable_->SetOutBoundaryCallback([weakScroll = AceType::WeakClaim(this)]() {
                auto scroll = weakScroll.Upgrade();
                if (scroll) {
                    return scroll->IsOutOfBoundary();
                }
                return false;
            });
        }
    }
}

void RenderList::CalculateMainScrollExtent(double curMainPos, double mainSize)
{
    // check current is out of boundary
    isOutOfBoundary_ = LessNotEqual(curMainPos, mainSize) || GreatNotEqual(currentOffset_, 0.0);
    // content length
    mainScrollExtent_ = curMainPos - currentOffset_;
    if (GetChildren().empty()) {
        return;
    }
    Size itemSize; // Calculate all children layout size.
    for (const auto& child : GetChildren()) {
        itemSize += child->GetLayoutSize();
    }
    auto averageItemHeight = GetMainSize(itemSize) / GetChildren().size() + spaceWidth_;
    estimatedHeight_ = averageItemHeight * TotalCount();
    lastOffset_ = startIndex_ * averageItemHeight - currentOffset_;
    if (estimatedHeight_ <= GetMainSize(GetLayoutSize()) && scrollBar_) {
        LOGD("SetScrollable false, do not show scroll bar.");
        scrollBar_->SetScrollable(false);
    } else {
        scrollBar_->SetScrollable(true);
    }
}

void RenderList::ProcessDragStart(double startPosition)
{
    auto globalMainOffset = GetMainAxis(GetGlobalOffset());
    auto localOffset = startPosition - globalMainOffset;
    auto index = GetNearChildByPosition(localOffset);
    if (index == INVALID_CHILD_INDEX) {
        LOGE("GetNearChildByPosition failed, localOffset = %lf not in item index [ %zu, %zu )", localOffset,
            startIndex_, startIndex_ + items_.size());
        return;
    }
    dragStartIndexPending_ = index;
}

void RenderList::ProcessDragUpdate(double dragOffset)
{
    if (!chainAnimation_) {
        return;
    }

    if (NearZero(dragOffset)) {
        return;
    }

    currentDelta_ = dragOffset;
    double delta = FlushChainAnimation();
    currentOffset_ += delta;
    if (!NearZero(delta)) {
        LOGE("ProcessDragUpdate delta = %lf currentOffset_ = %lf", delta, currentOffset_);
    }
}

void RenderList::ProcessScrollOverCallback(double velocity)
{
    if (!chainAnimation_) {
        LOGD("chain animation is null, no need to handle it.");
        return;
    }

    if (NearZero(velocity)) {
        LOGD("velocity is zero, no need to handle in chain animation.");
        return;
    }

    if (reachStart_) {
        dragStartIndexPending_ = startIndex_;
    } else if (reachEnd_) {
        dragStartIndexPending_ = startIndex_;
        if (!items_.empty()) {
            dragStartIndexPending_ += items_.size() - 1;
        }
    }

    double delta = FlushChainAnimation();
    currentOffset_ += delta;
    if (!NearZero(delta)) {
        LOGE("ProcessScrollOverCallback delta = %lf currentOffset_ = %lf", delta, currentOffset_);
    }
}

void RenderList::InitChainAnimation(int32_t nodeCount)
{
    auto context = GetContext().Upgrade();
    if (!context) {
        LOGE("Init chain animation failed. context is null");
        return;
    }

    if (chainAdapter_ && chain_) {
        return;
    }
    chainAdapter_ = AceType::MakeRefPtr<BilateralSpringAdapter>();
    chain_ = AceType::MakeRefPtr<SimpleSpringChain>(chainAdapter_);
    const auto& property = GetChainProperty();
    chain_->SetFrameDelta(property.FrameDelay());
    if (property.StiffnessTransfer()) {
        chain_->SetStiffnessTransfer(AceType::MakeRefPtr<ExpParamTransfer>(property.StiffnessCoefficient()));
    } else {
        chain_->SetStiffnessTransfer(AceType::MakeRefPtr<LinearParamTransfer>(property.StiffnessCoefficient()));
    }
    if (property.DampingTransfer()) {
        chain_->SetDampingTransfer(AceType::MakeRefPtr<ExpParamTransfer>(property.DampingCoefficient()));
    } else {
        chain_->SetDampingTransfer(AceType::MakeRefPtr<LinearParamTransfer>(property.DampingCoefficient()));
    }
    chain_->SetControlDamping(property.ControlDamping());
    chain_->SetControlStiffness(property.ControlStiffness());
    chain_->SetDecoration(context->NormalizeToPx(property.Interval()));
    chain_->SetMinDecoration(context->NormalizeToPx(property.MinInterval()));
    chain_->SetMaxDecoration(context->NormalizeToPx(property.MaxInterval()));
    for (int32_t index = 0; index < nodeCount; index++) {
        auto node = AceType::MakeRefPtr<BilateralSpringNode>(GetContext(), index, 0.0);
        WeakPtr<BilateralSpringNode> nodeWeak(node);
        WeakPtr<SimpleSpringAdapter> adapterWeak(chainAdapter_);
        node->AddUpdateListener(
            [weak = AceType::WeakClaim(this), nodeWeak, adapterWeak](double value, double velocity) {
                auto renderList = weak.Upgrade();
                auto node = nodeWeak.Upgrade();
                auto adapter = adapterWeak.Upgrade();
                if (!renderList || !node || !adapter) {
                    return;
                }
                renderList->MarkNeedLayout();
            });
        chainAdapter_->AddNode(node);
    }
    chainAdapter_->NotifyControlIndexChange();
}

double RenderList::GetChainDelta(int32_t index) const
{
    if (!chainAdapter_) {
        return 0.0;
    }
    double value = 0.0;
    RefPtr<BilateralSpringNode> node;
    int32_t controlIndex = dragStartIndex_;
    int32_t baseIndex = controlIndex - chainAdapter_->GetControlIndex();
    auto targetIndex = std::clamp(index - baseIndex, 0, CHAIN_ANIMATION_NODE_COUNT - 1);
    node = AceType::DynamicCast<BilateralSpringNode>(chainAdapter_->GetNode(targetIndex));
    if (node) {
        value = node->GetValue();
    }
    LOGD("ChainDelta. controlIndex: %{public}d, index: %{public}d, value: %{public}.3lf", controlIndex, index, value);
    return value;
}

size_t RenderList::GetNearChildByPosition(double mainOffset) const
{
    size_t index = startIndex_;
    size_t prevIndex = INVALID_CHILD_INDEX;

    for (auto& child : items_) {
        auto childMainOffset = GetMainAxis(child->GetPosition());

        if (childMainOffset > mainOffset) {
            return prevIndex;
        }
        prevIndex = index++;
    }
    return prevIndex;
}

double RenderList::FlushChainAnimation()
{
    if (!chainAnimation_ || !chain_ || !chainAdapter_) {
        return 0.0;
    }
    double deltaDistance = 0.0;
    bool needSetValue = false;
    bool overScroll = scrollable_ && scrollable_->IsSpringMotionRunning();
    if (chainOverScroll_ != overScroll) {
        if (overScroll) {
            const auto& springProperty = GetOverSpringProperty();
            if (springProperty && springProperty->IsValid()) {
                chain_->SetControlStiffness(springProperty->Stiffness());
                chain_->SetControlDamping(springProperty->Damping());
            }
        } else {
            chain_->SetControlStiffness(GetChainProperty().ControlStiffness());
            chain_->SetControlDamping(GetChainProperty().ControlDamping());
        }
        chain_->OnControlNodeChange();
        chainOverScroll_ = overScroll;
    }
    chain_->FlushAnimation();
    if (dragStartIndexPending_ != dragStartIndex_) {
        deltaDistance = chainAdapter_->ResetControl(dragStartIndexPending_ - dragStartIndex_);
        LOGD("Switch chain control node. %{public}zu -> %{public}zu, deltaDistance: %{public}.1f", dragStartIndex_,
            dragStartIndexPending_, deltaDistance);
        dragStartIndex_ = dragStartIndexPending_;
        chainAdapter_->SetDeltaValue(-deltaDistance);
        needSetValue = true;
    }
    if (!NearZero(currentDelta_)) {
        LOGD("Set delta chain value. delta: %{public}.1f", currentDelta_);
        chainAdapter_->SetDeltaValue(currentDelta_);
        currentDelta_ = 0.0;
        needSetValue = true;
    }
    if (needSetValue) {
        chain_->SetValue(0.0);
        LOGD("FlushChainAnimation: %{public}s", chainAdapter_->DumpNodes().c_str());
    }
    return deltaDistance;
}

void RenderList::UpdateAccessibilityAttr()
{
    if (!component_) {
        LOGE("RenderList: component is null.");
        return;
    }

    auto accessibilityNode = GetAccessibilityNode().Upgrade();
    if (!accessibilityNode) {
        LOGD("RenderList: current accessibilityNode is null.");
        return;
    }

    auto collectionInfo = accessibilityNode->GetCollectionInfo();
    size_t count = TotalCount() > 0 ? TotalCount() : 1;
    if (vertical_) {
        collectionInfo.rows = static_cast<int32_t>(count);
        collectionInfo.columns = 1;
    } else {
        collectionInfo.rows = 1;
        collectionInfo.columns = static_cast<int32_t>(count);
    }
    accessibilityNode->SetCollectionInfo(collectionInfo);
    accessibilityNode->SetScrollableState(true);
    accessibilityNode->SetActionScrollForward([weakList = AceType::WeakClaim(this)]() {
        auto list = weakList.Upgrade();
        if (list) {
            LOGI("Trigger ScrollForward by Accessibility.");
            return list->HandleActionScroll(true);
        }
        return false;
    });
    accessibilityNode->SetActionScrollBackward([weakList = AceType::WeakClaim(this)]() {
        auto list = weakList.Upgrade();
        if (list) {
            LOGI("Trigger ScrollBackward by Accessibility.");
            return list->HandleActionScroll(false);
        }
        return false;
    });

    accessibilityNode->AddSupportAction(AceAction::ACTION_SCROLL_FORWARD);
    accessibilityNode->AddSupportAction(AceAction::ACTION_SCROLL_BACKWARD);

    scrollFinishEventBack_ = [weakList = AceType::WeakClaim(this)] {
        auto list = weakList.Upgrade();
        if (list) {
            list->ModifyActionScroll();
        }
    };
}

bool RenderList::ActionByScroll(bool forward, ScrollEventBack scrollEventBack)
{
    LOGI("Handle action by Scroll.");
    auto node = GetParent().Upgrade();
    while (node) {
        auto scroll = AceType::DynamicCast<RenderSingleChildScroll>(node);
        if (!scroll) {
            node = node->GetParent().Upgrade();
            continue;
        }

        scroll->ScrollPage(!forward, true, scrollEventBack);
        return true;
    }
    return false;
}

bool RenderList::HandleActionScroll(bool forward)
{
    if (isActionByScroll_) {
        return ActionByScroll(forward, scrollFinishEventBack_);
    }

    if (forward) {
        JumpToIndex(lastDisplayIndex_, DEFAULT_SOURCE);
    } else {
        JumpToIndex(startIndex_, DEFAULT_SOURCE);
    }
    if (scrollFinishEventBack_) {
        scrollFinishEventBack_();
    }
    return true;
}

void RenderList::ModifyActionScroll()
{
    hasActionScroll_ = true;
}

void RenderList::OnPaintFinish()
{
    if (!hasActionScroll_) {
        return;
    }

    hasActionScroll_ = false;
    auto context = context_.Upgrade();
    if (!context) {
        LOGE("RenderList: context is null.");
        return;
    }

    AccessibilityEvent scrollEvent;
    scrollEvent.nodeId = GetAccessibilityNodeId();
    scrollEvent.eventType = "scrollend";
    context->SendEventToAccessibility(scrollEvent);
}

bool RenderList::IsUseOnly()
{
    return true;
}

bool RenderList::PrepareRawRecognizer()
{
    if (rawRecognizer_) {
        return true;
    }

    rawRecognizer_ = AceType::MakeRefPtr<RawRecognizer>();
    auto weak = AceType::WeakClaim(this);
    rawRecognizer_->SetOnTouchDown([weak](const TouchEventInfo& info) {
        if (info.GetTouches().empty()) {
            return;
        }
        auto spThis = weak.Upgrade();
        if (spThis) {
            spThis->lastPos_ = spThis->GetMainAxis(info.GetTouches().front().GetLocalLocation());
        }
    });
    rawRecognizer_->SetOnTouchMove([weak](const TouchEventInfo& info) {
        if (info.GetTouches().empty()) {
            return;
        }

        auto spThis = weak.Upgrade();
        if (!spThis || !spThis->selectedItem_) {
            return;
        }
        double currentPos = spThis->GetMainAxis(info.GetTouches().front().GetLocalLocation());
        spThis->OnSelectedItemMove(currentPos);
    });
    rawRecognizer_->SetOnTouchUp([weak](const TouchEventInfo& info) {
        auto spThis = weak.Upgrade();
        if (spThis) {
            spThis->OnSelectedItemStopMoving(false);
        }
    });
    rawRecognizer_->SetOnTouchCancel([weak](const TouchEventInfo& info) {
        auto spThis = weak.Upgrade();
        if (spThis) {
            spThis->OnSelectedItemStopMoving(true);
        }
    });

    return true;
}

void RenderList::OnSelectedItemMove(double position)
{
    double deltaPos = position - lastPos_;

    movingForward_ = LessOrEqual(deltaPos, 0.0);
    selectedItemMainAxis_ += deltaPos;
    deltaPos = -deltaPos;
    if (LessOrEqual(selectedItemMainAxis_, 0.0)) {
        selectedItemMainAxis_ = 0.0;
    } else {
        double maxMainSize = GetMainSize(GetLayoutSize());
        double mainSize = GetMainSize(selectedItem_->GetLayoutSize());
        if (GreatOrEqual(selectedItemMainAxis_ + mainSize, maxMainSize)) {
            selectedItemMainAxis_ = maxMainSize - mainSize;
        } else {
            deltaPos = 0.0;
            lastPos_ = position;
        }
    }

    if (!NearZero(deltaPos)) {
        currentOffset_ += deltaPos;
        autoScrollingForItemMove_ = true;
    }

    MarkNeedLayout();
}

void RenderList::OnSelectedItemStopMoving(bool canceled)
{
    if (!canceled && targetIndex_ != selectedItemIndex_) {
        auto from = static_cast<int32_t>(selectedItemIndex_);
        auto to = static_cast<int32_t>(targetIndex_);
        LOGI("Moving item from %{private}d to %{private}d", from, to);
        if (!ResumeEventCallback(component_, &ListComponent::GetOnItemMove, false, from, to)) {
            LOGI("User canceled, stop moving item");
        }
    }

    if (selectedItemIndex_ < startIndex_ || selectedItemIndex_ >= startIndex_ + items_.size()) {
        RecycleListItem(selectedItemIndex_);
    }

    targetIndex_ = INITIAL_CHILD_INDEX;
    selectedItemIndex_ = INITIAL_CHILD_INDEX;
    selectedItem_ = nullptr;
    MarkNeedLayout();
}

void RenderList::CreateDragDropRecognizer()
{
    if (dragDropGesture_) {
        return;
    }

    auto longPressRecognizer =
        AceType::MakeRefPtr<OHOS::Ace::LongPressRecognizer>(context_, DEFAULT_DURATION, DEFAULT_FINGERS, false);
    longPressRecognizer->SetOnAction([weakRenderList = AceType::WeakClaim(this)](const GestureEvent& info) {
        auto renderList = weakRenderList.Upgrade();
        if (!renderList) {
            LOGE("LongPress action RenderList is null.");
            return;
        }
        renderList->scrollable_->MarkAvailable(false);
    });
    PanDirection panDirection;
    auto panRecognizer =
        AceType::MakeRefPtr<OHOS::Ace::PanRecognizer>(context_, DEFAULT_FINGERS, panDirection, DEFAULT_DISTANCE);
    panRecognizer->SetOnActionStart([weakRenderList = AceType::WeakClaim(this), context = context_,
                                        onItemDragStart = onItemDragStart_](const GestureEvent& info) {
        if (onItemDragStart) {
            auto pipelineContext = context.Upgrade();
            if (!pipelineContext) {
                LOGE("Context is null.");
                return;
            }

            auto renderList = weakRenderList.Upgrade();
            if (!renderList) {
                LOGE("RenderList is null.");
                return;
            }

            ItemDragInfo dragInfo;
            dragInfo.SetX(info.GetGlobalPoint().GetX());
            dragInfo.SetY(info.GetGlobalPoint().GetY());

            Point point = info.GetGlobalPoint() - renderList->GetGlobalOffset();
            auto listItem = renderList->FindCurrentListItem(point);
            if (!listItem) {
                LOGW("There is no listitem at the point.");
                return;
            }

            if (!listItem->IsMovable()) {
                LOGI("This list item is not movable.");
                return;
            }
            renderList->selectedDragItem_ = listItem;
            renderList->selectedItemIndex_ = renderList->GetIndexByListItem(listItem);
            renderList->selectedDragItem_->SetHidden(true);
            renderList->MarkNeedLayout();

            auto customComponent = onItemDragStart(dragInfo, int32_t(renderList->selectedItemIndex_));
            if (!customComponent) {
                LOGE("Custom component is null.");
                return;
            }
            auto stackElement = pipelineContext->GetLastStack();
            auto positionedComponent = AceType::MakeRefPtr<PositionedComponent>(customComponent);
            positionedComponent->SetTop(Dimension(listItem->GetGlobalOffset().GetY()));
            positionedComponent->SetLeft(Dimension(listItem->GetGlobalOffset().GetX()));
            renderList->SetBetweenItemAndBuilder(
                Offset(info.GetGlobalPoint().GetX() - listItem->GetGlobalOffset().GetX(),
                    info.GetGlobalPoint().GetY() - listItem->GetGlobalOffset().GetY()));

            auto updatePosition = [weak = weakRenderList](
                                      const std::function<void(const Dimension&, const Dimension&)>& func) {
                auto renderList = weak.Upgrade();
                if (!renderList) {
                    return;
                }
                renderList->SetUpdateBuilderFuncId(func);
            };

            positionedComponent->SetUpdatePositionFuncId(updatePosition);
            stackElement->PushComponent(positionedComponent);
            renderList->hasDragItem_ = true;
        }
    });
    panRecognizer->SetOnActionUpdate(
        [weakRenderList = AceType::WeakClaim(this), context = context_](const GestureEvent& info) {
            auto pipelineContext = context.Upgrade();
            if (!pipelineContext) {
                LOGE("Context is null.");
                return;
            }

            auto renderList = weakRenderList.Upgrade();
            if (!renderList) {
                LOGE("RenderList is null.");
                return;
            }

            ItemDragInfo dragInfo;
            dragInfo.SetX(info.GetGlobalPoint().GetX());
            dragInfo.SetY(info.GetGlobalPoint().GetY());

            Point point = info.GetGlobalPoint() - renderList->GetBetweenItemAndBuilder();
            if (renderList->GetUpdateBuilderFuncId()) {
                renderList->GetUpdateBuilderFuncId()(Dimension(point.GetX()), Dimension(point.GetY()));
            }

            auto targetRenderlist = renderList->FindTargetRenderNode<V2::RenderList>(pipelineContext, info);
            auto preTargetRenderlist = renderList->GetPreTargetRenderList();

            if (preTargetRenderlist == targetRenderlist) {
                if (targetRenderlist && targetRenderlist->GetOnItemDragMove()) {
                    Point point = info.GetGlobalPoint() - targetRenderlist->GetGlobalOffset();
                    auto newListItem = targetRenderlist->FindCurrentListItem(point);
                    if (static_cast<int32_t>(targetRenderlist->GetIndexByListItem(newListItem)) > -1) {
                        renderList->insertItemIndex_ = targetRenderlist->GetIndexByListItem(newListItem);
                    }
                    if (targetRenderlist == renderList) {
                        (targetRenderlist->GetOnItemDragMove())(dragInfo,
                            static_cast<int32_t>(renderList->selectedItemIndex_), renderList->insertItemIndex_);
                    } else {
                        (targetRenderlist->GetOnItemDragMove())(dragInfo, -1, renderList->insertItemIndex_);
                    }
                }
                return;
            }
            if (preTargetRenderlist) {
                if (preTargetRenderlist->GetOnItemDragLeave()) {
                    (preTargetRenderlist->GetOnItemDragLeave())(
                        dragInfo, static_cast<int32_t>(renderList->selectedItemIndex_));
                }
            }
            if (targetRenderlist) {
                if (targetRenderlist->GetOnItemDragEnter()) {
                    (targetRenderlist->GetOnItemDragEnter())(dragInfo);
                }
            }
            renderList->SetPreTargetRenderList(targetRenderlist);
        });
    panRecognizer->SetOnActionEnd([weakRenderList = AceType::WeakClaim(this), context = context_](
                                      const GestureEvent& info) {
        auto pipelineContext = context.Upgrade();
        if (!pipelineContext) {
            LOGE("Context is null.");
            return;
        }

        auto renderList = weakRenderList.Upgrade();
        if (!renderList) {
            LOGE("RenderList is null.");
            return;
        }
        if (!renderList->selectedDragItem_ || !renderList->selectedDragItem_->IsMovable()) {
            return;
        }

        ItemDragInfo dragInfo;
        dragInfo.SetX(info.GetGlobalPoint().GetX());
        dragInfo.SetY(info.GetGlobalPoint().GetY());
        if (renderList->hasDragItem_) {
            auto stackElement = pipelineContext->GetLastStack();
            stackElement->PopComponent();
            renderList->hasDragItem_ = false;
        }

        ACE_DCHECK(renderList->GetPreTargetRenderList() ==
                   renderList->FindTargetRenderNode<V2::RenderList>(pipelineContext, info));
        auto targetRenderlist = renderList->GetPreTargetRenderList();

        if (!targetRenderlist) {
            (renderList->GetOnItemDrop())(dragInfo, static_cast<int32_t>(renderList->selectedItemIndex_), -1, true);
            renderList->SetPreTargetRenderList(nullptr);
            renderList->selectedDragItem_->SetHidden(false);
            renderList->MarkNeedLayout();
            return;
        }

        renderList->selectedDragItem_->SetHidden(false);
        if (targetRenderlist->GetOnItemDrop()) {
            Point point = info.GetGlobalPoint() - targetRenderlist->GetGlobalOffset();
            auto newListItem = targetRenderlist->FindCurrentListItem(point);
            if (static_cast<int32_t>(targetRenderlist->GetIndexByListItem(newListItem)) > -1) {
                renderList->insertItemIndex_ = static_cast<size_t>(targetRenderlist->GetIndexByListItem(newListItem));
            }
            if (targetRenderlist == renderList) {
                int32_t from = static_cast<int32_t>(renderList->selectedItemIndex_);
                int32_t to = static_cast<int32_t>(renderList->insertItemIndex_);
                auto moveRes = ResumeEventCallback(
                    renderList->component_, &ListComponent::GetOnItemMove, true, from, to);
                (targetRenderlist->GetOnItemDrop())(dragInfo, from, to, moveRes);
                renderList->MarkNeedLayout();
            } else {
                (targetRenderlist->GetOnItemDrop())(dragInfo, -1, renderList->insertItemIndex_, true);
                targetRenderlist->MarkNeedLayout();
            }
        }
        renderList->SetPreTargetRenderList(nullptr);
        renderList->scrollable_->MarkAvailable(true);
    });
    panRecognizer->SetOnActionCancel([weakRenderList = AceType::WeakClaim(this), context = context_]() {
        auto pipelineContext = context.Upgrade();
        if (!pipelineContext) {
            LOGE("Context is null.");
            return;
        }

        auto renderList = weakRenderList.Upgrade();
        if (!renderList) {
            LOGE("RenderList is null.");
            return;
        }
        if (!renderList->selectedDragItem_ || !renderList->selectedDragItem_->IsMovable()) {
            return;
        }

        if (renderList->hasDragItem_) {
            auto stackElement = pipelineContext->GetLastStack();
            stackElement->PopComponent();
            renderList->hasDragItem_ = false;
        }

        renderList->SetPreTargetRenderList(nullptr);
        renderList->selectedDragItem_->SetHidden(false);
        renderList->scrollable_->MarkAvailable(true);
        renderList->MarkNeedLayout();
    });
    std::vector<RefPtr<GestureRecognizer>> recognizers { longPressRecognizer, panRecognizer };
    dragDropGesture_ = AceType::MakeRefPtr<OHOS::Ace::SequencedRecognizer>(GetContext(), recognizers);
}

RefPtr<RenderListItem> RenderList::FindCurrentListItem(const Point& point)
{
    const auto& children = GetChildren();
    for (auto iter = children.rbegin(); iter != children.rend(); ++iter) {
        auto& child = *iter;
        for (auto& rect : child->GetTouchRectList()) {
            if (rect.IsInRegion(point)) {
                return AceType::DynamicCast<RenderListItem>(child);
            }
        }
    }
    return nullptr;
}

size_t RenderList::CalculateSelectedIndex(
    const RefPtr<RenderList> targetRenderlist, const GestureEvent& info, Size& selectedItemSize)
{
    auto listItem = targetRenderlist->FindTargetRenderNode<RenderListItem>(context_.Upgrade(), info);
    if (listItem) {
        selectedItemSize = listItem->GetLayoutSize();
        return targetRenderlist->GetIndexByListItem(listItem);
    }

    return DEFAULT_INDEX;
}

size_t RenderList::CalculateInsertIndex(
    const RefPtr<RenderList> targetRenderlist, const GestureEvent& info, Size selectedItemSize)
{
    if (targetRenderlist->TotalCount() == 0) {
        return 0;
    }

    auto listItem = targetRenderlist->FindTargetRenderNode<RenderListItem>(context_.Upgrade(), info);
    if (!listItem) {
        GestureEvent newEvent = info;
        while (!listItem) {
            if (FindTargetRenderNode<V2::RenderList>(context_.Upgrade(), newEvent) != targetRenderlist) {
                break;
            }
            double newX = vertical_ ? newEvent.GetGlobalPoint().GetX()
                                    : newEvent.GetGlobalPoint().GetX() - selectedItemSize.Width();
            double newY = vertical_ ? newEvent.GetGlobalPoint().GetY() - selectedItemSize.Height()
                                    : newEvent.GetGlobalPoint().GetY();
            newEvent.SetGlobalPoint(Point(newX, newY));
            listItem = targetRenderlist->FindTargetRenderNode<RenderListItem>(context_.Upgrade(), newEvent);
        }
        if (!listItem) {
            return 0;
        }
        if (static_cast<int32_t>(targetRenderlist->GetIndexByListItem(listItem)) > -1) {
            return targetRenderlist->GetIndexByListItem(listItem) + 1;
        }
        return DEFAULT_INDEX;
    }

    if (static_cast<int32_t>(targetRenderlist->GetIndexByListItem(listItem)) > -1) {
        return targetRenderlist->GetIndexByListItem(listItem);
    }

    return DEFAULT_INDEX;
}

bool RenderList::IsAxisScrollable(AxisDirection direction)
{
    return (((AxisEvent::IsDirectionUp(direction) || AxisEvent::IsDirectionLeft(direction)) && !reachStart_) ||
            ((AxisEvent::IsDirectionDown(direction) || AxisEvent::IsDirectionRight(direction)) && !reachEnd_));
}

void RenderList::HandleAxisEvent(const AxisEvent& event) {}

bool RenderList::HandleMouseEvent(const MouseEvent& event)
{
    if (!isMultiSelectable_) {
        return false;
    }

    if (event.action == MouseAction::HOVER_EXIT) {
        mouseIsHover_ = false;
    } else {
        mouseIsHover_ = true;
    }

    auto context = context_.Upgrade();
    if (context) {
        context->SubscribeCtrlA([wp = AceType::WeakClaim(this)]() {
            auto sp = wp.Upgrade();
            if (sp) {
                if (sp->mouseIsHover_ == true) {
                    sp->MultiSelectAllWhenCtrlA();
                } else {
                    sp->ClearMultiSelect();
                    sp->MarkNeedRender();
                }
            }
        });
    } else {
        LOGE("context is null");
        return false;
    }

    if (context->IsCtrlDown()) {
        if (context->IsKeyboardA()) {
            MultiSelectAllWhenCtrlA();
            return true;
        }
        HandleMouseEventWhenCtrlDown(event);
        return true;
    }
    selectedItemsWithCtrl_.clear();

    if (context->IsShiftDown()) {
        HandleMouseEventWhenShiftDown(event);
        return true;
    }
    firstItemWithShift_ = nullptr;

    HandleMouseEventWithoutKeyboard(event);
    return true;
}

void RenderList::ClearMultiSelect()
{
    for (const auto& listItem : items_) {
        if (!listItem) {
            continue;
        }
        listItem->MarkIsSelected(false);
    }
}

void RenderList::MultiSelectWithoutKeyboard(const Rect& selectedZone)
{
    if (!selectedZone.IsValid()) {
        Point mousePoint(selectedZone.GetOffset().GetX(), selectedZone.GetOffset().GetY());
        auto listItem = FindChildNodeOfClass<RenderListItem>(mousePoint, mousePoint);
        if (!listItem) {
            return;
        }
        if (!listItem->GetSelectable()) {
            return;
        }
        listItem->MarkIsSelected(true);
        if (listItem->GetOnSelectId()) {
            (listItem->GetOnSelectId())(listItem->IsSelected());
        }
        return;
    }

    for (const auto& listItem : items_) {
        if (!listItem) {
            continue;
        }
        if (!listItem->GetSelectable()) {
            continue;
        }
        if (!selectedZone.IsIntersectWith(listItem->GetPaintRect())) {
            listItem->MarkIsSelected(false);
            if (listItem->GetOnSelectId()) {
                (listItem->GetOnSelectId())(listItem->IsSelected());
            }
            continue;
        }
        listItem->MarkIsSelected(true);
        if (listItem->GetOnSelectId()) {
            (listItem->GetOnSelectId())(listItem->IsSelected());
        }
    }
}

void RenderList::HandleMouseEventWithoutKeyboard(const MouseEvent& event)
{
    if (event.button == MouseButton::LEFT_BUTTON) {
        if (event.action == MouseAction::PRESS) {
            ClearMultiSelect();
            mouseStartOffset_ = event.GetOffset() - GetPaintRect().GetOffset();
            mouseEndOffset_ = event.GetOffset() - GetPaintRect().GetOffset();
            auto selectedZone = ComputeSelectedZone(mouseStartOffset_, mouseEndOffset_);
            MultiSelectWithoutKeyboard(selectedZone);
            MarkNeedRender();
        } else if (event.action == MouseAction::MOVE) {
            mouseEndOffset_ = event.GetOffset() - GetPaintRect().GetOffset();
            auto selectedZone = ComputeSelectedZone(mouseStartOffset_, mouseEndOffset_);
            MultiSelectWithoutKeyboard(selectedZone);
            MarkNeedRender();
        } else if (event.action == MouseAction::RELEASE) {
            mouseStartOffset_ = Offset(0.0, 0.0);
            mouseEndOffset_ = Offset(0.0, 0.0);
            MarkNeedRender();
        }
    }
}

RefPtr<RenderListItem> RenderList::GetPressItemWhenShiftDown(const Rect& selectedZone)
{
    if (!selectedZone.IsValid()) {
        Point mousePoint(selectedZone.GetOffset().GetX(), selectedZone.GetOffset().GetY());
        auto listItem = FindChildNodeOfClass<RenderListItem>(mousePoint, mousePoint);
        if (!listItem) {
            return nullptr;
        }
        if (!listItem->GetSelectable()) {
            return nullptr;
        }
        return listItem;
    }
    return nullptr;
}

void RenderList::MultiSelectWhenShiftDown(const Rect& selectedZone)
{
    for (const auto& listItem : items_) {
        if (!listItem) {
            continue;
        }
        if (!listItem->GetSelectable()) {
            continue;
        }
        if (!selectedZone.IsIntersectWith(listItem->GetPaintRect())) {
            continue;
        }
        listItem->MarkIsSelected(true);
        if (listItem->GetOnSelectId()) {
            (listItem->GetOnSelectId())(listItem->IsSelected());
        }
    }
}

void RenderList::HandleMouseEventWhenShiftDown(const MouseEvent& event)
{
    if (event.button == MouseButton::LEFT_BUTTON) {
        if (event.action == MouseAction::PRESS) {
            mouseStartOffset_ = event.GetOffset() - GetPaintRect().GetOffset();
            mouseEndOffset_ = event.GetOffset() - GetPaintRect().GetOffset();
            auto selectedZone = ComputeSelectedZone(mouseStartOffset_, mouseEndOffset_);
            if (firstItemWithShift_ == nullptr) {
                firstItemWithShift_ = GetPressItemWhenShiftDown(selectedZone);
            }
            secondItemWithShift_ = GetPressItemWhenShiftDown(selectedZone);
            MultiSelectAllInRange(firstItemWithShift_, secondItemWithShift_);
            MarkNeedRender();
        } else if (event.action == MouseAction::MOVE) {
            mouseEndOffset_ = event.GetOffset() - GetPaintRect().GetOffset();
            auto selectedZone = ComputeSelectedZone(mouseStartOffset_, mouseEndOffset_);
            MultiSelectWhenShiftDown(selectedZone);
            MarkNeedRender();
        } else if (event.action == MouseAction::RELEASE) {
            mouseStartOffset_ = Offset(0.0, 0.0);
            mouseEndOffset_ = Offset(0.0, 0.0);
            MarkNeedRender();
        }
    }
}

void RenderList::MultiSelectAllInRange(
    const RefPtr<RenderListItem>& firstItem, const RefPtr<RenderListItem>& secondItem)
{
    ClearMultiSelect();
    if (!firstItem) {
        return;
    }

    if (!secondItem) {
        firstItem->MarkIsSelected(true);
        if (firstItem->GetOnSelectId()) {
            (firstItem->GetOnSelectId())(firstItem->IsSelected());
        }
        return;
    }

    auto fromItemIndex = std::min(GetIndexByListItem(firstItem), GetIndexByListItem(secondItem));
    auto toItemIndex = std::max(GetIndexByListItem(firstItem), GetIndexByListItem(secondItem));

    for (const auto& listItem : items_) {
        if (!listItem) {
            continue;
        }
        if (!listItem->GetSelectable()) {
            continue;
        }

        auto nowIndex = GetIndexByListItem(listItem);
        if (nowIndex <= toItemIndex && nowIndex >= fromItemIndex) {
            listItem->MarkIsSelected(true);
            if (listItem->GetOnSelectId()) {
                (listItem->GetOnSelectId())(listItem->IsSelected());
            }
        }
    }
}

void RenderList::MultiSelectWhenCtrlDown(const Rect& selectedZone)
{
    if (!selectedZone.IsValid()) {
        Point mousePoint(selectedZone.GetOffset().GetX(), selectedZone.GetOffset().GetY());
        auto listItem = FindChildNodeOfClass<RenderListItem>(mousePoint, mousePoint);
        if (!listItem) {
            return;
        }
        if (!listItem->GetSelectable()) {
            return;
        }

        if (selectedItemsWithCtrl_.find(listItem) != selectedItemsWithCtrl_.end()) {
            listItem->MarkIsSelected(false);
        } else {
            listItem->MarkIsSelected(true);
        }

        if (listItem->GetOnSelectId()) {
            (listItem->GetOnSelectId())(listItem->IsSelected());
        }
        return;
    }

    for (const auto& listItem : items_) {
        if (!listItem) {
            continue;
        }
        if (!listItem->GetSelectable()) {
            continue;
        }
        if (!selectedZone.IsIntersectWith(listItem->GetPaintRect())) {
            if (selectedItemsWithCtrl_.find(listItem) != selectedItemsWithCtrl_.end()) {
                listItem->MarkIsSelected(true);
            } else {
                listItem->MarkIsSelected(false);
            }
            if (listItem->GetOnSelectId()) {
                (listItem->GetOnSelectId())(listItem->IsSelected());
            }
            continue;
        }

        if (selectedItemsWithCtrl_.find(listItem) != selectedItemsWithCtrl_.end()) {
            listItem->MarkIsSelected(false);
        } else {
            listItem->MarkIsSelected(true);
        }

        if (listItem->GetOnSelectId()) {
            (listItem->GetOnSelectId())(listItem->IsSelected());
        }
    }
}

void RenderList::HandleMouseEventWhenCtrlDown(const MouseEvent& event)
{
    if (event.button == MouseButton::LEFT_BUTTON) {
        if (event.action == MouseAction::PRESS) {
            mouseStartOffset_ = event.GetOffset() - GetPaintRect().GetOffset();
            mouseEndOffset_ = event.GetOffset() - GetPaintRect().GetOffset();
            auto selectedZone = ComputeSelectedZone(mouseStartOffset_, mouseEndOffset_);
            MultiSelectWhenCtrlDown(selectedZone);
            MarkNeedRender();
        } else if (event.action == MouseAction::MOVE) {
            mouseEndOffset_ = event.GetOffset() - GetPaintRect().GetOffset();
            auto selectedZone = ComputeSelectedZone(mouseStartOffset_, mouseEndOffset_);
            MultiSelectWhenCtrlDown(selectedZone);
            MarkNeedRender();
        } else if (event.action == MouseAction::RELEASE) {
            mouseStartOffset_ = Offset(0.0, 0.0);
            mouseEndOffset_ = Offset(0.0, 0.0);
            MarkNeedRender();
            CollectSelectedItems();
        }
    }
}

void RenderList::CollectSelectedItems()
{
    selectedItemsWithCtrl_.clear();
    for (const auto& listItem : items_) {
        if (!listItem) {
            continue;
        }
        if (!listItem->GetSelectable()) {
            continue;
        }
        if (listItem->IsSelected()) {
            selectedItemsWithCtrl_.insert(listItem);
        }
    }
}

void RenderList::MultiSelectAllWhenCtrlA()
{
    for (const auto& listItem : items_) {
        if (!listItem) {
            continue;
        }
        if (!listItem->GetSelectable()) {
            continue;
        }
        listItem->MarkIsSelected(true);
        if (listItem->GetOnSelectId()) {
            (listItem->GetOnSelectId())(listItem->IsSelected());
        }
    }
    MarkNeedRender();
}

int32_t RenderList::RequestNextFocus(bool vertical, bool reverse)
{
    bool rightToLeft_ = false;
    int32_t moveStep = DIRECTION_MAP.at(rightToLeft_).at(vertical_).at(vertical).at(reverse);
    if (moveStep == STEP_INVALID) {
        return -1;
    }
    focusIndex_ += moveStep;
    return focusIndex_;
}

std::string RenderList::ProvideRestoreInfo()
{
    return std::to_string(firstDisplayIndex_);
}

void RenderList::ApplyRestoreInfo()
{
    if (GetRestoreInfo().empty()) {
        return;
    }
    JumpToIndex(StringUtils::StringToInt(GetRestoreInfo()), DEFAULT_SOURCE);
    SetRestoreInfo("");
}

} // namespace OHOS::Ace::V2
