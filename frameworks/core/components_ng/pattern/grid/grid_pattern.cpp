/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/grid/grid_pattern.h"

#include "base/geometry/axis.h"
#include "base/log/dump_log.h"
#include "base/perfmonitor/perf_constants.h"
#include "base/perfmonitor/perf_monitor.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components/scroll/scroll_controller_base.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/base/observer_handler.h"
#include "core/components_ng/pattern/grid/grid_adaptive/grid_adaptive_layout_algorithm.h"
#include "core/components_ng/pattern/grid/grid_item_layout_property.h"
#include "core/components_ng/pattern/grid/grid_item_pattern.h"
#include "core/components_ng/pattern/grid/grid_layout/grid_layout_algorithm.h"
#include "core/components_ng/pattern/grid/grid_layout_property.h"
#include "core/components_ng/pattern/grid/grid_scroll/grid_scroll_layout_algorithm.h"
#include "core/components_ng/pattern/grid/grid_scroll/grid_scroll_with_options_layout_algorithm.h"
#include "core/components_ng/pattern/grid/grid_utils.h"
#include "core/components_ng/pattern/grid/irregular/grid_irregular_layout_algorithm.h"
#include "core/components_ng/pattern/grid/irregular/grid_layout_utils.h"
#include "core/components_ng/pattern/scroll_bar/proxy/scroll_bar_proxy.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

namespace {
const Color ITEM_FILL_COLOR = Color::TRANSPARENT;

double CalcCoordinatesDistance(double curFocusMain, double curFocusCross, double childMain, double childCross)
{
    return std::sqrt(std::pow((curFocusMain - childMain), 2) + std::pow((curFocusCross - childCross), 2));
}
} // namespace

RefPtr<LayoutAlgorithm> GridPattern::CreateLayoutAlgorithm()
{
    auto gridLayoutProperty = GetLayoutProperty<GridLayoutProperty>();
    CHECK_NULL_RETURN(gridLayoutProperty, nullptr);
    std::vector<std::string> cols;
    StringUtils::StringSplitter(gridLayoutProperty->GetColumnsTemplate().value_or(""), ' ', cols);
    std::vector<std::string> rows;
    StringUtils::StringSplitter(gridLayoutProperty->GetRowsTemplate().value_or(""), ' ', rows);
    auto crossCount = cols.empty() ? Infinity<int32_t>() : static_cast<int32_t>(cols.size());
    auto mainCount = rows.empty() ? Infinity<int32_t>() : static_cast<int32_t>(rows.size());
    if (!gridLayoutProperty->IsVertical()) {
        std::swap(crossCount, mainCount);
    }
    gridLayoutInfo_.crossCount_ = crossCount;
    if (targetIndex_.has_value()) {
        gridLayoutInfo_.targetIndex_ = targetIndex_;
    }
    // When rowsTemplate and columnsTemplate is both setting, use static layout algorithm.
    if (!rows.empty() && !cols.empty()) {
        return MakeRefPtr<GridLayoutAlgorithm>(gridLayoutInfo_, crossCount, mainCount);
    }

    // When rowsTemplate and columnsTemplate is both not setting, use adaptive layout algorithm.
    if (rows.empty() && cols.empty()) {
        return MakeRefPtr<GridAdaptiveLayoutAlgorithm>(gridLayoutInfo_);
    }

    // If only set one of rowTemplate and columnsTemplate, use scrollable layout algorithm.
    bool disableSkip = IsOutOfBoundary() || ScrollablePattern::AnimateRunning();
    if (UseIrregularLayout()) {
        auto algo = MakeRefPtr<GridIrregularLayoutAlgorithm>(gridLayoutInfo_, CanOverScroll(GetScrollSource()));
        algo->SetEnableSkip(!disableSkip);
        return algo;
    }
    RefPtr<GridScrollLayoutAlgorithm> result;
    if (!gridLayoutProperty->GetLayoutOptions().has_value()) {
        result = MakeRefPtr<GridScrollLayoutAlgorithm>(gridLayoutInfo_, crossCount, mainCount);
    } else {
        result = MakeRefPtr<GridScrollWithOptionsLayoutAlgorithm>(gridLayoutInfo_, crossCount, mainCount);
    }
    result->SetCanOverScroll(CanOverScroll(GetScrollSource()));
    result->SetScrollSource(GetScrollSource());
    if (ScrollablePattern::AnimateRunning()) {
        result->SetLineSkipping(!disableSkip);
    }
    return result;
}

void GridPattern::BeforeCreateLayoutWrapper()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    gridLayoutInfo_.childrenCount_ = host->GetTotalChildCount();
}

RefPtr<NodePaintMethod> GridPattern::CreateNodePaintMethod()
{
    auto paint = MakeRefPtr<GridPaintMethod>(GetScrollBar());
    CHECK_NULL_RETURN(paint, nullptr);
    CreateScrollBarOverlayModifier();
    paint->SetScrollBarOverlayModifier(GetScrollBarOverlayModifier());
    auto scrollEffect = GetScrollEdgeEffect();
    if (scrollEffect && scrollEffect->IsFadeEffect()) {
        paint->SetEdgeEffect(scrollEffect);
    }
    return paint;
}

void GridPattern::OnModifyDone()
{
    auto gridLayoutProperty = GetLayoutProperty<GridLayoutProperty>();
    CHECK_NULL_VOID(gridLayoutProperty);

    if (multiSelectable_ && !isMouseEventInit_) {
        InitMouseEvent();
    }

    if (!multiSelectable_ && isMouseEventInit_) {
        UninitMouseEvent();
    }

    gridLayoutInfo_.axis_ = gridLayoutProperty->IsVertical() ? Axis::VERTICAL : Axis::HORIZONTAL;
    isConfigScrollable_ = gridLayoutProperty->IsConfiguredScrollable();
    if (!isConfigScrollable_) {
        return;
    }
    SetAxis(gridLayoutInfo_.axis_);
    if (!GetScrollableEvent()) {
        AddScrollEvent();
    }

    SetEdgeEffect();

    auto paintProperty = GetPaintProperty<ScrollablePaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    if (paintProperty->GetScrollBarProperty()) {
        SetScrollBar(paintProperty->GetScrollBarProperty());
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto focusHub = host->GetFocusHub();
    if (focusHub) {
        InitOnKeyEvent(focusHub);
    }
    SetAccessibilityAction();
    Register2DragDropManager();
    if (IsNeedInitClickEventRecorder()) {
        Pattern::InitClickEventRecorder();
    }
}

void GridPattern::MultiSelectWithoutKeyboard(const RectF& selectedZone)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::list<RefPtr<FrameNode>> children;
    host->GenerateOneDepthVisibleFrame(children);
    for (const auto& itemFrameNode : children) {
        auto itemEvent = itemFrameNode->GetEventHub<EventHub>();
        CHECK_NULL_VOID(itemEvent);
        if (!itemEvent->IsEnabled()) {
            continue;
        }

        auto itemPattern = itemFrameNode->GetPattern<GridItemPattern>();
        CHECK_NULL_VOID(itemPattern);
        if (!itemPattern->Selectable()) {
            continue;
        }
        auto itemGeometry = itemFrameNode->GetGeometryNode();
        CHECK_NULL_VOID(itemGeometry);
        auto context = itemFrameNode->GetRenderContext();
        CHECK_NULL_VOID(context);

        auto itemRect = itemGeometry->GetFrameRect();
        auto iter = itemToBeSelected_.find(itemFrameNode->GetId());
        if (iter == itemToBeSelected_.end()) {
            auto result = itemToBeSelected_.emplace(itemFrameNode->GetId(), ItemSelectedStatus());
            iter = result.first;
            iter->second.onSelected = itemPattern->GetEventHub<GridItemEventHub>()->GetOnSelect();
            iter->second.selectChangeEvent = itemPattern->GetEventHub<GridItemEventHub>()->GetSelectChangeEvent();
        }
        auto startMainOffset = mouseStartOffset_.GetMainOffset(gridLayoutInfo_.axis_);
        if (gridLayoutInfo_.axis_ == Axis::VERTICAL) {
            iter->second.rect = itemRect + OffsetF(0, totalOffsetOfMousePressed_ - startMainOffset);
        } else {
            iter->second.rect = itemRect + OffsetF(totalOffsetOfMousePressed_ - startMainOffset, 0);
        }

        if (!selectedZone.IsIntersectWith(itemRect)) {
            itemPattern->MarkIsSelected(false);
            iter->second.selected = false;
            context->OnMouseSelectUpdate(false, ITEM_FILL_COLOR, ITEM_FILL_COLOR);
        } else {
            itemPattern->MarkIsSelected(true);
            iter->second.selected = true;
            context->OnMouseSelectUpdate(true, ITEM_FILL_COLOR, ITEM_FILL_COLOR);
        }
    }

    DrawSelectedZone(selectedZone);
}

void GridPattern::ClearMultiSelect()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::list<RefPtr<FrameNode>> children;
    host->GenerateOneDepthAllFrame(children);
    for (const auto& item : children) {
        if (!AceType::InstanceOf<FrameNode>(item)) {
            continue;
        }

        auto itemFrameNode = AceType::DynamicCast<FrameNode>(item);
        auto itemPattern = itemFrameNode->GetPattern<GridItemPattern>();
        CHECK_NULL_VOID(itemPattern);
        auto selectedStatus = itemToBeSelected_.find(itemFrameNode->GetId());
        if (selectedStatus != itemToBeSelected_.end()) {
            selectedStatus->second.selected = false;
        }
        itemPattern->MarkIsSelected(false);
        auto renderContext = itemFrameNode->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        renderContext->OnMouseSelectUpdate(false, ITEM_FILL_COLOR, ITEM_FILL_COLOR);
    }

    ClearSelectedZone();
}

bool GridPattern::IsItemSelected(const GestureEvent& info)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto node = host->FindChildByPosition(info.GetGlobalLocation().GetX(), info.GetGlobalLocation().GetY());
    CHECK_NULL_RETURN(node, false);
    auto itemPattern = node->GetPattern<GridItemPattern>();
    CHECK_NULL_RETURN(itemPattern, false);
    return itemPattern->IsSelected();
}

void GridPattern::FireOnScrollStart()
{
    UIObserverHandler::GetInstance().NotifyScrollEventStateChange(
        AceType::WeakClaim(this), ScrollEventType::SCROLL_START);
    SuggestOpIncGroup(true);
    PerfMonitor::GetPerfMonitor()->Start(PerfConstants::APP_LIST_FLING, PerfActionType::FIRST_MOVE, "");
    if (GetScrollAbort()) {
        return;
    }
    if (scrollStop_) {
        // onScrollStart triggers immediately on gesture dragStart, but onScrollStop marks scrollStop_ to true on
        // gesture dragEnd, and consumes it/fires onScrollStop after layout. When the user quickly swipes twice, the
        // second onScrollStart can trigger before the first onScrollEnd. In this case, we let the two events annihilate
        // each other and fire neither.
        scrollStop_ = false;
        return;
    }
    auto scrollBar = GetScrollBar();
    if (scrollBar) {
        scrollBar->PlayScrollBarAppearAnimation();
    }
    StopScrollBarAnimatorByProxy();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<GridEventHub>();
    CHECK_NULL_VOID(hub);
    auto onScrollStart = hub->GetOnScrollStart();
    CHECK_NULL_VOID(onScrollStart);
    onScrollStart();
}

SizeF GridPattern::GetContentSize() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, SizeF());
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, SizeF());
    return geometryNode->GetPaddingSize();
}

float GridPattern::GetMainGap() const
{
    float mainGap = 0.0;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 0.0);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0);
    auto viewScopeSize = geometryNode->GetPaddingSize();
    auto layoutProperty = host->GetLayoutProperty<GridLayoutProperty>();
    mainGap = GridUtils::GetMainGap(layoutProperty, viewScopeSize, gridLayoutInfo_.axis_);
    return mainGap;
}

bool GridPattern::UpdateCurrentOffset(float offset, int32_t source)
{
    if (!isConfigScrollable_ || !scrollable_) {
        return true;
    }

    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);

    // check edgeEffect is not springEffect
    if (!HandleEdgeEffect(offset, source, GetContentSize())) {
        if (IsOutOfBoundary()) {
            host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        }
        return false;
    }
    SetScrollSource(source);
    FireAndCleanScrollingListener();
    if (gridLayoutInfo_.synced_) {
        gridLayoutInfo_.prevOffset_ = gridLayoutInfo_.currentOffset_;
        gridLayoutInfo_.synced_ = false;
    }
    // When finger moves down, offset is positive.
    // When finger moves up, offset is negative.
    bool regular = !UseIrregularLayout();
    float mainGap = GetMainGap();
    auto itemsHeight = gridLayoutInfo_.GetTotalHeightOfItemsInView(mainGap, regular);
    if (gridLayoutInfo_.offsetEnd_) {
        if (source == SCROLL_FROM_UPDATE) {
            float overScroll = 0.0f;
            if (!regular) {
                overScroll = gridLayoutInfo_.GetDistanceToBottom(GetMainContentSize(), itemsHeight, mainGap);
            } else {
                overScroll = gridLayoutInfo_.currentOffset_ - (GetMainContentSize() - itemsHeight);
            }
            auto friction = ScrollablePattern::CalculateFriction(std::abs(overScroll) / GetMainContentSize());
            offset *= friction;
        }
        auto userOffset = FireOnWillScroll(-offset);
        gridLayoutInfo_.currentOffset_ -= userOffset;

        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);

        if (GreatNotEqual(gridLayoutInfo_.currentOffset_, GetMainContentSize() - itemsHeight)) {
            gridLayoutInfo_.offsetEnd_ = false;
            gridLayoutInfo_.reachEnd_ = false;
        }
        return true;
    }
    if (gridLayoutInfo_.reachStart_) {
        if (source == SCROLL_FROM_UPDATE) {
            auto friction =
                ScrollablePattern::CalculateFriction(std::abs(gridLayoutInfo_.currentOffset_) / GetMainContentSize());
            offset *= friction;
        }
        auto userOffset = FireOnWillScroll(-offset);
        gridLayoutInfo_.currentOffset_ -= userOffset;

        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);

        if (LessNotEqual(gridLayoutInfo_.currentOffset_, 0.0)) {
            gridLayoutInfo_.reachStart_ = false;
        }
        return true;
    }
    auto userOffset = FireOnWillScroll(-offset);
    gridLayoutInfo_.currentOffset_ -= userOffset;
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    return true;
}

bool GridPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto gridLayoutAlgorithm = DynamicCast<GridLayoutBaseAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(gridLayoutAlgorithm, false);
    const auto& gridLayoutInfo = gridLayoutAlgorithm->GetGridLayoutInfo();
    auto eventhub = GetEventHub<GridEventHub>();
    CHECK_NULL_RETURN(eventhub, false);
    Dimension offset(0, DimensionUnit::VP);
    Dimension offsetPx(gridLayoutInfo.currentOffset_, DimensionUnit::PX);
    auto offsetVpValue = offsetPx.ConvertToVp();
    offset.SetValue(offsetVpValue);
    scrollbarInfo_ = eventhub->FireOnScrollBarUpdate(gridLayoutInfo.startIndex_, offset);
    if (!isInitialized_ || gridLayoutInfo_.startIndex_ != gridLayoutInfo.startIndex_) {
        eventhub->FireOnScrollToIndex(gridLayoutInfo.startIndex_);
    }

    bool indexChanged = (gridLayoutInfo.startIndex_ != gridLayoutInfo_.startIndex_) ||
                        (gridLayoutInfo.endIndex_ != gridLayoutInfo_.endIndex_);
    bool offsetEnd = gridLayoutInfo_.offsetEnd_;
    gridLayoutInfo_ = gridLayoutInfo;
    gridLayoutInfo_.synced_ = true;
    AnimateToTarget(scrollAlign_, layoutAlgorithmWrapper);

    gridLayoutInfo_.reachStart_ =
        gridLayoutInfo_.startIndex_ == 0 && GreatOrEqual(gridLayoutInfo_.currentOffset_, 0.0f);

    gridLayoutInfo_.currentHeight_ = EstimateHeight();
    if (!offsetEnd && gridLayoutInfo_.offsetEnd_) {
        endHeight_ = gridLayoutInfo_.currentHeight_;
    }
    ProcessEvent(indexChanged, gridLayoutInfo_.currentHeight_ - gridLayoutInfo_.prevHeight_);
    gridLayoutInfo_.prevHeight_ = gridLayoutInfo_.currentHeight_;
    gridLayoutInfo_.extraOffset_.reset();
    SetScrollSource(SCROLL_FROM_NONE);
    UpdateScrollBarOffset();
    if (config.frameSizeChange) {
        if (GetScrollBar() != nullptr) {
            GetScrollBar()->ScheduleDisappearDelayTask();
        }
    }
    CheckRestartSpring(false);
    CheckScrollable();
    MarkSelectedItems();
    isInitialized_ = true;
    return false;
}

void GridPattern::CheckScrollable()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gridLayoutProperty = host->GetLayoutProperty<GridLayoutProperty>();
    CHECK_NULL_VOID(gridLayoutProperty);
    if (((gridLayoutInfo_.endIndex_ - gridLayoutInfo_.startIndex_ + 1) < gridLayoutInfo_.childrenCount_) ||
        (gridLayoutInfo_.GetTotalHeightOfItemsInView(GetMainGap(), !UseIrregularLayout()) > GetMainContentSize())) {
        scrollable_ = true;
    } else {
        if (gridLayoutInfo_.startMainLineIndex_ != 0 || GetAlwaysEnabled()) {
            scrollable_ = true;
        } else {
            scrollable_ = false;
        }
    }

    SetScrollEnabled(scrollable_);

    if (!gridLayoutProperty->GetScrollEnabled().value_or(scrollable_)) {
        SetScrollEnabled(false);
    }
}

void GridPattern::ProcessEvent(bool indexChanged, float finalOffset)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gridEventHub = host->GetEventHub<GridEventHub>();
    CHECK_NULL_VOID(gridEventHub);

    auto onScroll = gridEventHub->GetOnScroll();
    PrintOffsetLog(AceLogTag::ACE_GRID, host->GetId(), finalOffset);
    if (onScroll) {
        FireOnScroll(finalOffset, onScroll);
    }
    auto onDidScroll = gridEventHub->GetOnDidScroll();
    if (onDidScroll) {
        FireOnScroll(finalOffset, onDidScroll);
    }

    if (indexChanged) {
        auto onScrollIndex = gridEventHub->GetOnScrollIndex();
        if (onScrollIndex) {
            onScrollIndex(gridLayoutInfo_.startIndex_, gridLayoutInfo_.endIndex_);
        }
    }

    auto onReachStart = gridEventHub->GetOnReachStart();
    if (onReachStart && gridLayoutInfo_.startIndex_ == 0) {
        if (!isInitialized_) {
            onReachStart();
            AddEventsFiredInfo(ScrollableEventType::ON_REACH_START);
        }

        if (!NearZero(finalOffset)) {
            bool scrollUpToStart =
                GreatOrEqual(gridLayoutInfo_.prevHeight_, 0.0) && LessOrEqual(gridLayoutInfo_.currentHeight_, 0.0);
            bool scrollDownToStart =
                LessNotEqual(gridLayoutInfo_.prevHeight_, 0.0) && GreatOrEqual(gridLayoutInfo_.currentHeight_, 0.0);
            if (scrollUpToStart || scrollDownToStart) {
                onReachStart();
                AddEventsFiredInfo(ScrollableEventType::ON_REACH_START);
            }
        }
    }

    auto onReachEnd = gridEventHub->GetOnReachEnd();
    if (onReachEnd && gridLayoutInfo_.endIndex_ == (gridLayoutInfo_.childrenCount_ - 1)) {
        if (!NearZero(finalOffset)) {
            bool scrollDownToEnd = LessNotEqual(gridLayoutInfo_.prevHeight_, endHeight_) &&
                                   GreatOrEqual(gridLayoutInfo_.currentHeight_, endHeight_);
            bool scrollUpToEnd = GreatNotEqual(gridLayoutInfo_.prevHeight_, endHeight_) &&
                                 LessOrEqual(gridLayoutInfo_.currentHeight_, endHeight_);
            if (scrollDownToEnd || scrollUpToEnd) {
                onReachEnd();
                AddEventsFiredInfo(ScrollableEventType::ON_REACH_END);
            }
        }
    }

    OnScrollStop(gridEventHub->GetOnScrollStop());
}

void GridPattern::MarkDirtyNodeSelf()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void GridPattern::OnScrollEndCallback()
{
    isSmoothScrolling_ = false;
    scrollStop_ = true;
    MarkDirtyNodeSelf();
}

std::pair<bool, bool> GridPattern::IsFirstOrLastFocusableChild(int32_t curMainIndex, int32_t curCrossIndex)
{
    std::unordered_set<int32_t> crossIndexSet;
    size_t maxSize = 0;
    for (int32_t index = curMainIndex - curFocusIndexInfo_.mainSpan + 1; index <= curMainIndex; index++) {
        auto tempIndexSet = GetFocusableChildCrossIndexesAt(index);
        if (tempIndexSet.size() > maxSize) {
            maxSize = tempIndexSet.size();
            crossIndexSet = tempIndexSet;
        }
    }
    auto findLesser = std::find_if(crossIndexSet.begin(), crossIndexSet.end(),
        [curCrossIndex](int32_t crossIndex) { return curCrossIndex > crossIndex; });
    auto findGreater = std::find_if(crossIndexSet.begin(), crossIndexSet.end(),
        [curCrossIndex](int32_t crossIndex) { return curCrossIndex < crossIndex; });
    return { curCrossIndex == 0 || findLesser == crossIndexSet.end(),
        curCrossIndex == gridLayoutInfo_.crossCount_ - 1 || findGreater == crossIndexSet.end() };
}

std::pair<FocusStep, FocusStep> GridPattern::GetFocusSteps(int32_t curMainIndex, int32_t curCrossIndex, FocusStep step)
{
    auto firstStep = FocusStep::NONE;
    auto secondStep = FocusStep::NONE;
    auto isFirstOrLastFocusable = IsFirstOrLastFocusableChild(curMainIndex, curCrossIndex);
    auto isFirstFocusable = isFirstOrLastFocusable.first;
    auto isLastFocusable = isFirstOrLastFocusable.second;
    if (gridLayoutInfo_.axis_ == Axis::VERTICAL) {
        if (isFirstFocusable && step == FocusStep::SHIFT_TAB) {
            firstStep = FocusStep::UP;
            secondStep = FocusStep::RIGHT_END;
        } else if (isLastFocusable && step == FocusStep::TAB) {
            firstStep = FocusStep::DOWN;
            secondStep = FocusStep::LEFT_END;
        }
    } else if (gridLayoutInfo_.axis_ == Axis::HORIZONTAL) {
        if (isFirstFocusable && step == FocusStep::SHIFT_TAB) {
            firstStep = FocusStep::LEFT;
            secondStep = FocusStep::DOWN_END;
        } else if (isLastFocusable && step == FocusStep::TAB) {
            firstStep = FocusStep::RIGHT;
            secondStep = FocusStep::UP_END;
        }
    }
    TAG_LOGI(AceLogTag::ACE_GRID, "Get focus steps. First step is %{public}d. Second step is %{public}d", firstStep,
        secondStep);
    return { firstStep, secondStep };
}

WeakPtr<FocusHub> GridPattern::GetNextFocusNode(FocusStep step, const WeakPtr<FocusHub>& currentFocusNode)
{
    auto curFocus = currentFocusNode.Upgrade();
    CHECK_NULL_RETURN(curFocus, nullptr);
    auto curFrame = curFocus->GetFrameNode();
    CHECK_NULL_RETURN(curFrame, nullptr);
    auto curPattern = curFrame->GetPattern();
    CHECK_NULL_RETURN(curPattern, nullptr);
    auto curItemPattern = AceType::DynamicCast<GridItemPattern>(curPattern);
    CHECK_NULL_RETURN(curItemPattern, nullptr);
    auto curItemProperty = curItemPattern->GetLayoutProperty<GridItemLayoutProperty>();
    CHECK_NULL_RETURN(curItemProperty, nullptr);
    auto irregularInfo = curItemPattern->GetIrregularItemInfo();
    bool hasIrregularItemInfo = irregularInfo.has_value();

    auto curMainIndex = curItemProperty->GetMainIndex().value_or(-1);
    auto curCrossIndex = curItemProperty->GetCrossIndex().value_or(-1);
    auto curMainSpan =
        hasIrregularItemInfo ? irregularInfo.value().mainSpan : curItemProperty->GetMainSpan(gridLayoutInfo_.axis_);
    auto curCrossSpan =
        hasIrregularItemInfo ? irregularInfo.value().crossSpan : curItemProperty->GetCrossSpan(gridLayoutInfo_.axis_);
    auto curMainStart =
        hasIrregularItemInfo ? irregularInfo.value().mainStart : curItemProperty->GetMainStart(gridLayoutInfo_.axis_);
    auto curCrossStart =
        hasIrregularItemInfo ? irregularInfo.value().crossStart : curItemProperty->GetCrossStart(gridLayoutInfo_.axis_);
    auto curMainEnd =
        hasIrregularItemInfo ? irregularInfo.value().mainEnd : curItemProperty->GetMainEnd(gridLayoutInfo_.axis_);
    auto curCrossEnd =
        hasIrregularItemInfo ? irregularInfo.value().crossEnd : curItemProperty->GetCrossEnd(gridLayoutInfo_.axis_);

    curFocusIndexInfo_.mainIndex = curMainIndex;
    curFocusIndexInfo_.crossIndex = curCrossIndex;
    curFocusIndexInfo_.mainSpan = curMainSpan;
    curFocusIndexInfo_.crossSpan = curCrossSpan;
    curFocusIndexInfo_.mainStart = curMainStart;
    curFocusIndexInfo_.mainEnd = curMainEnd;
    curFocusIndexInfo_.crossStart = curCrossStart;
    curFocusIndexInfo_.crossEnd = curCrossEnd;

    if (curMainIndex < 0 || curCrossIndex < 0) {
        TAG_LOGW(AceLogTag::ACE_GRID, "can't find focused child.");
        return nullptr;
    }
    if (gridLayoutInfo_.gridMatrix_.find(curMainIndex) == gridLayoutInfo_.gridMatrix_.end()) {
        TAG_LOGW(AceLogTag::ACE_GRID, "Can not find current main index: %{public}d", curMainIndex);
        return nullptr;
    }
    LOGI("GetNextFocusNode: Current focused item is (%{public}d,%{public}d)-[%{public}d,%{public}d]. Focus step is "
         "%{public}d",
        curMainIndex, curCrossIndex, curMainSpan, curCrossSpan, step);
    auto focusSteps = GetFocusSteps(curMainIndex, curCrossIndex, step);
    if (focusSteps.first != FocusStep::NONE && focusSteps.second != FocusStep::NONE) {
        auto firstStepRes = GetNextFocusNode(focusSteps.first, currentFocusNode);
        if (!firstStepRes.Upgrade()) {
            return nullptr;
        }
        auto secondStepRes = GetNextFocusNode(focusSteps.second, firstStepRes);
        if (!secondStepRes.Upgrade()) {
            return firstStepRes;
        }
        return secondStepRes;
    }
    auto indexes = GetNextIndexByStep(curMainIndex, curCrossIndex, curMainSpan, curCrossSpan, step);
    auto nextMainIndex = indexes.first;
    auto nextCrossIndex = indexes.second;
    while (nextMainIndex >= 0 && nextCrossIndex >= 0) {
        if (gridLayoutInfo_.gridMatrix_.find(nextMainIndex) == gridLayoutInfo_.gridMatrix_.end()) {
            TAG_LOGW(AceLogTag::ACE_GRID, "Can not find next main index: %{public}d", nextMainIndex);
            return nullptr;
        }
        auto nextMaxCrossCount = GetCrossCount();
        auto flag = (step == FocusStep::LEFT_END) || (step == FocusStep::RIGHT_END);
        auto weakChild = gridLayoutInfo_.hasBigItem_ ? SearchIrregularFocusableChild(nextMainIndex, nextCrossIndex)
                                                     : SearchFocusableChildInCross(nextMainIndex, nextCrossIndex,
                                                           nextMaxCrossCount, flag ? -1 : curMainIndex, curCrossIndex);
        auto child = weakChild.Upgrade();
        if (child && child->IsFocusable()) {
            ScrollToFocusNode(weakChild);
            return weakChild;
        }
        auto indexes = GetNextIndexByStep(nextMainIndex, nextCrossIndex, 1, 1, step);
        nextMainIndex = indexes.first;
        nextCrossIndex = indexes.second;
    }
    return nullptr;
}

std::pair<int32_t, int32_t> GridPattern::GetNextIndexByStep(
    int32_t curMainIndex, int32_t curCrossIndex, int32_t curMainSpan, int32_t curCrossSpan, FocusStep step)
{
    LOGI("Current item: (%{public}d,%{public}d)-[%{public}d,%{public}d]. Grid axis: %{public}d, step: %{public}d",
        curMainIndex, curCrossIndex, curMainSpan, curCrossSpan, gridLayoutInfo_.axis_, step);
    auto curMainStart = gridLayoutInfo_.startMainLineIndex_;
    auto curMainEnd = gridLayoutInfo_.endMainLineIndex_;
    auto curChildStartIndex = gridLayoutInfo_.startIndex_;
    auto curChildEndIndex = gridLayoutInfo_.endIndex_;
    auto childrenCount = gridLayoutInfo_.childrenCount_;
    auto hasIrregularItems = gridLayoutInfo_.hasBigItem_;
    if (gridLayoutInfo_.gridMatrix_.find(curMainIndex) == gridLayoutInfo_.gridMatrix_.end()) {
        TAG_LOGW(AceLogTag::ACE_GRID, "Can not find current main index: %{public}d", curMainIndex);
        return { -1, -1 };
    }
    auto curMaxCrossCount = GetCrossCount();
    auto nextMainIndex = curMainIndex;
    auto nextCrossIndex = curCrossIndex;
    if ((step == FocusStep::UP_END && gridLayoutInfo_.axis_ == Axis::HORIZONTAL) ||
        (step == FocusStep::LEFT_END && gridLayoutInfo_.axis_ == Axis::VERTICAL)) {
        nextMainIndex = curMainIndex;
        nextCrossIndex = 0;
        isLeftEndStep_ = hasIrregularItems ? true : false;
    } else if ((step == FocusStep::DOWN_END && gridLayoutInfo_.axis_ == Axis::HORIZONTAL) ||
               (step == FocusStep::RIGHT_END && gridLayoutInfo_.axis_ == Axis::VERTICAL)) {
        nextMainIndex = curMainIndex;
        nextCrossIndex = curMaxCrossCount - 1;
        isRightEndStep_ = hasIrregularItems ? true : false;
    } else if (((step == FocusStep::UP || step == FocusStep::SHIFT_TAB) && gridLayoutInfo_.axis_ == Axis::HORIZONTAL) ||
               ((step == FocusStep::LEFT || step == FocusStep::SHIFT_TAB) && gridLayoutInfo_.axis_ == Axis::VERTICAL)) {
        nextMainIndex = curMainIndex;
        nextCrossIndex = curCrossIndex - 1;
        isLeftStep_ = hasIrregularItems ? true : false;
    } else if ((step == FocusStep::UP && gridLayoutInfo_.axis_ == Axis::VERTICAL) ||
               (step == FocusStep::LEFT && gridLayoutInfo_.axis_ == Axis::HORIZONTAL)) {
        nextMainIndex = hasIrregularItems ? curMainIndex - curMainSpan : curMainIndex - 1;
        nextCrossIndex = curCrossIndex + static_cast<int32_t>((curCrossSpan - 1) / 2);
        isUpStep_ = hasIrregularItems ? true : false;
    } else if (((step == FocusStep::DOWN || step == FocusStep::TAB) && gridLayoutInfo_.axis_ == Axis::HORIZONTAL) ||
               ((step == FocusStep::RIGHT || step == FocusStep::TAB) && gridLayoutInfo_.axis_ == Axis::VERTICAL)) {
        nextMainIndex = curMainIndex;
        nextCrossIndex = curCrossIndex + curCrossSpan;
        isRightStep_ = hasIrregularItems ? true : false;
    } else if ((step == FocusStep::DOWN && gridLayoutInfo_.axis_ == Axis::VERTICAL) ||
               (step == FocusStep::RIGHT && gridLayoutInfo_.axis_ == Axis::HORIZONTAL)) {
        nextMainIndex = hasIrregularItems ? curMainIndex + 1 : curMainIndex + curMainSpan;
        nextCrossIndex = curCrossIndex + static_cast<int32_t>((curCrossSpan - 1) / 2);
        isDownStep_ = hasIrregularItems ? true : false;
    } else {
        TAG_LOGW(AceLogTag::ACE_GRID, "Next index return: Invalid step: %{public}d and axis: %{public}d", step,
            gridLayoutInfo_.axis_);
        return { -1, -1 };
    }
    if (curChildStartIndex == 0 && curMainIndex == 0 && nextMainIndex < curMainIndex) {
        nextMainIndex = curMainIndex;
    }
    if (curChildEndIndex == childrenCount - 1 && curMainIndex == curMainEnd && nextMainIndex > curMainIndex) {
        nextMainIndex = curMainIndex;
    }
    if (nextMainIndex == curMainIndex && nextCrossIndex == curCrossIndex) {
        TAG_LOGI(AceLogTag::ACE_GRID,
            "Next index return: Move stoped. Next index: (%{public}d,%{public}d) is same as current.", nextMainIndex,
            nextCrossIndex);
        ResetAllDirectionsStep();
        return { -1, -1 };
    }
    if (curChildStartIndex != 0 && curMainIndex == curMainStart && nextMainIndex < curMainIndex) {
        // Scroll item up.
        UpdateStartIndex(curChildStartIndex - 1);
        auto pipeline = PipelineContext::GetCurrentContext();
        if (pipeline) {
            pipeline->FlushUITasks();
        }
    } else if (curChildEndIndex != childrenCount - 1 && curMainIndex == curMainEnd && nextMainIndex > curMainIndex) {
        // Scroll item down.
        UpdateStartIndex(curChildEndIndex + 1);
        auto pipeline = PipelineContext::GetCurrentContext();
        if (pipeline) {
            pipeline->FlushUITasks();
        }
    }
    curMainStart = gridLayoutInfo_.startMainLineIndex_;
    curMainEnd = gridLayoutInfo_.endMainLineIndex_;
    if (nextMainIndex < curMainStart || nextMainIndex > curMainEnd) {
        ResetAllDirectionsStep();
        return { -1, -1 };
    }
    if (nextCrossIndex < 0) {
        ResetAllDirectionsStep();
        return { -1, -1 };
    }
    if (gridLayoutInfo_.gridMatrix_.find(nextMainIndex) == gridLayoutInfo_.gridMatrix_.end()) {
        ResetAllDirectionsStep();
        return { -1, -1 };
    }
    auto nextMaxCrossCount = GetCrossCount();
    if (nextCrossIndex >= nextMaxCrossCount) {
        TAG_LOGI(AceLogTag::ACE_GRID,
            "Next index: { %{public}d,%{public}d }. Next cross index is greater than max cross count: %{public}d.",
            nextMainIndex, nextCrossIndex, nextMaxCrossCount - 1);
        if (nextMaxCrossCount - 1 != (curCrossIndex + curCrossSpan - 1)) {
            TAG_LOGI(AceLogTag::ACE_GRID,
                "Current cross index: %{public}d is not the tail item. Return to the tail: { %{public}d,%{public}d }",
                curCrossIndex, nextMainIndex, nextMaxCrossCount - 1);
            return { nextMainIndex, nextMaxCrossCount - 1 };
        }
        ResetAllDirectionsStep();
        TAG_LOGI(AceLogTag::ACE_GRID, "Current cross index: %{public}d is the tail item. No next item can be found!",
            curCrossIndex);
        return { -1, -1 };
    }
    TAG_LOGI(AceLogTag::ACE_GRID, "Next index return: { %{public}d,%{public}d }.", nextMainIndex, nextCrossIndex);
    return { nextMainIndex, nextCrossIndex };
}

WeakPtr<FocusHub> GridPattern::SearchFocusableChildInCross(
    int32_t tarMainIndex, int32_t tarCrossIndex, int32_t maxCrossCount, int32_t curMainIndex, int32_t curCrossIndex)
{
    bool isDirectionLeft = true;
    auto indexLeft = tarCrossIndex;
    auto indexRight = tarCrossIndex;
    if (curMainIndex == tarMainIndex) {
        // Search on the same main index. Do not need search on both left and right side.
        if (tarCrossIndex > curCrossIndex) {
            // Only search on the right side.
            indexLeft = -1;
        } else if (tarCrossIndex < curCrossIndex) {
            // Only search on the left side.
            indexRight = maxCrossCount;
        } else {
            TAG_LOGW(AceLogTag::ACE_GRID, "Invalid search index: (%{public}d,%{public}d). It's same as current.",
                tarMainIndex, tarCrossIndex);
            return nullptr;
        }
    }
    while (indexLeft >= 0 || indexRight < maxCrossCount) {
        int32_t curIndex = indexLeft;
        if (indexLeft < 0) {
            curIndex = indexRight++;
        } else if (indexRight >= maxCrossCount) {
            curIndex = indexLeft--;
        } else {
            curIndex = isDirectionLeft ? indexLeft-- : indexRight++;
            isDirectionLeft = !isDirectionLeft;
        }
        auto weakChild = GetChildFocusNodeByIndex(tarMainIndex, curIndex);
        auto child = weakChild.Upgrade();
        if (child && child->IsFocusable()) {
            TAG_LOGI(AceLogTag::ACE_GRID, "Found child. Index: %{public}d,%{public}d", tarMainIndex, curIndex);
            return weakChild;
        }
    }
    return nullptr;
}

WeakPtr<FocusHub> GridPattern::SearchIrregularFocusableChild(int32_t tarMainIndex, int32_t tarCrossIndex)
{
    double minDistance = std::numeric_limits<double>::max();
    int32_t minMainIndex = std::numeric_limits<int32_t>::max();
    int32_t minCrossIndex = std::numeric_limits<int32_t>::max();
    int32_t maxAreaInMainShadow = -1;
    int32_t maxAreaInCrossShadow = -1;
    WeakPtr<FocusHub> targetFocusHubWeak;

    auto gridFrame = GetHost();
    CHECK_NULL_RETURN(gridFrame, nullptr);
    auto gridFocus = gridFrame->GetFocusHub();
    CHECK_NULL_RETURN(gridFocus, nullptr);
    auto childFocusList = gridFocus->GetChildren();
    for (const auto& childFocus : childFocusList) {
        if (!childFocus->IsFocusable()) {
            continue;
        }
        auto childFrame = childFocus->GetFrameNode();
        if (!childFrame) {
            continue;
        }
        auto childPattern = childFrame->GetPattern<GridItemPattern>();
        if (!childPattern) {
            continue;
        }
        auto childItemProperty = childFrame->GetLayoutProperty<GridItemLayoutProperty>();
        if (!childItemProperty) {
            continue;
        }
        auto irregularInfo = childPattern->GetIrregularItemInfo();
        bool hasIrregularItemInfo = irregularInfo.has_value();

        auto childMainIndex = childItemProperty->GetMainIndex().value_or(-1);
        auto childCrossIndex = childItemProperty->GetCrossIndex().value_or(-1);
        auto childMainStart = hasIrregularItemInfo ? irregularInfo.value().mainStart
                                                   : childItemProperty->GetMainStart(gridLayoutInfo_.axis_);
        auto childMainEnd =
            hasIrregularItemInfo ? irregularInfo.value().mainEnd : childItemProperty->GetMainEnd(gridLayoutInfo_.axis_);
        auto chidCrossStart = hasIrregularItemInfo ? irregularInfo.value().crossStart
                                                   : childItemProperty->GetCrossStart(gridLayoutInfo_.axis_);
        auto chidCrossEnd = hasIrregularItemInfo ? irregularInfo.value().crossEnd
                                                 : childItemProperty->GetCrossEnd(gridLayoutInfo_.axis_);
        auto childCrossSpan = hasIrregularItemInfo ? irregularInfo.value().crossSpan
                                                   : childItemProperty->GetCrossSpan(gridLayoutInfo_.axis_);
        auto childMainSpan = hasIrregularItemInfo ? irregularInfo.value().mainSpan
                                                  : childItemProperty->GetMainSpan(gridLayoutInfo_.axis_);

        GridItemIndexInfo childInfo;
        childInfo.mainIndex = childMainIndex;
        childInfo.crossIndex = childCrossIndex;
        childInfo.mainStart = childMainStart;
        childInfo.mainEnd = childMainEnd;
        childInfo.crossStart = chidCrossStart;
        childInfo.crossEnd = chidCrossEnd;

        if (childMainIndex < 0 || childCrossIndex < 0) {
            continue;
        }

        if ((isLeftStep_ && ((childCrossIndex == tarCrossIndex && childCrossSpan == 1) ||
                                (chidCrossEnd >= 0 && chidCrossEnd == tarCrossIndex))) ||
            (isRightStep_ && childCrossIndex == tarCrossIndex)) {
            double nearestDistance = GetNearestDistanceFromChildToCurFocusItemInMainAxis(tarCrossIndex, childInfo);
            int32_t intersectAreaSize = CalcIntersectAreaInTargetDirectionShadow(childInfo, true);
            if (LessNotEqual(nearestDistance, minDistance) ||
                (NearEqual(nearestDistance, minDistance) && intersectAreaSize > maxAreaInCrossShadow) ||
                (NearEqual(nearestDistance, minDistance) && intersectAreaSize == maxAreaInCrossShadow &&
                    childMainIndex < minMainIndex)) {
                minDistance = nearestDistance;
                maxAreaInCrossShadow = intersectAreaSize;
                minMainIndex = childMainIndex;
                targetFocusHubWeak = AceType::WeakClaim(AceType::RawPtr(childFocus));
            }
        } else if ((isUpStep_ && childMainIndex == tarMainIndex) ||
                   (isDownStep_ && ((childMainIndex == tarMainIndex && childMainSpan == 1) ||
                                       (childMainStart >= 0 && childMainStart == tarMainIndex)))) {
            double nearestDistance = GetNearestDistanceFromChildToCurFocusItemInCrossAxis(tarMainIndex, childInfo);
            int32_t intersectAreaSize = CalcIntersectAreaInTargetDirectionShadow(childInfo, false);
            if (LessNotEqual(nearestDistance, minDistance) ||
                (NearEqual(nearestDistance, minDistance) && intersectAreaSize > maxAreaInMainShadow) ||
                (NearEqual(nearestDistance, minDistance) && intersectAreaSize == maxAreaInMainShadow &&
                    childCrossIndex < minCrossIndex)) {
                minDistance = nearestDistance;
                minCrossIndex = childCrossIndex;
                maxAreaInMainShadow = intersectAreaSize;
                targetFocusHubWeak = AceType::WeakClaim(AceType::RawPtr(childFocus));
            }
        } else if ((isLeftEndStep_ || isRightEndStep_) &&
                   ((tarMainIndex == childMainIndex && tarCrossIndex == childCrossIndex) ||
                       (childMainStart >= 0 && childMainStart <= tarMainIndex && tarMainIndex <= childMainIndex &&
                           tarCrossIndex == childCrossIndex))) {
            targetFocusHubWeak = AceType::WeakClaim(AceType::RawPtr(childFocus));
        }
    }
    ResetAllDirectionsStep();
    return targetFocusHubWeak;
}

int32_t GridPattern::CalcIntersectAreaInTargetDirectionShadow(GridItemIndexInfo itemIndexInfo, bool isFindInMainAxis)
{
    int32_t curFocusLeftTopX = -1;
    int32_t curFocusLeftTopY = -1;
    int32_t curFocusRightBottonX = -1;
    int32_t curFocusRightBottonY = -1;

    if (isFindInMainAxis) {
        curFocusLeftTopX =
            curFocusIndexInfo_.mainStart == -1 ? curFocusIndexInfo_.mainIndex : curFocusIndexInfo_.mainStart;
        curFocusLeftTopY = 0;
        curFocusRightBottonX =
            curFocusIndexInfo_.mainEnd == -1 ? curFocusIndexInfo_.mainIndex : curFocusIndexInfo_.mainEnd;
        curFocusRightBottonY = GetCrossCount();
    } else {
        curFocusLeftTopX = gridLayoutInfo_.startMainLineIndex_;
        curFocusLeftTopY =
            curFocusIndexInfo_.crossStart == -1 ? curFocusIndexInfo_.crossIndex : curFocusIndexInfo_.crossStart;
        curFocusRightBottonX = gridLayoutInfo_.endMainLineIndex_;
        curFocusRightBottonY =
            curFocusIndexInfo_.crossEnd == -1 ? curFocusIndexInfo_.crossIndex : curFocusIndexInfo_.crossEnd;
    }
    int32_t childLeftTopX = itemIndexInfo.mainStart == -1 ? itemIndexInfo.mainIndex : itemIndexInfo.mainStart;
    int32_t childLeftTopY = itemIndexInfo.crossStart == -1 ? itemIndexInfo.crossIndex : itemIndexInfo.crossStart;
    int32_t childRightBottonX = itemIndexInfo.mainEnd == -1 ? itemIndexInfo.mainIndex : itemIndexInfo.mainEnd;
    int32_t childRightBottonY = itemIndexInfo.crossEnd == -1 ? itemIndexInfo.crossIndex : itemIndexInfo.crossEnd;

    int32_t intersectAreaLeftTopX = std::max(curFocusLeftTopX, childLeftTopX);
    int32_t intersectAreaLeftTopY = std::max(curFocusLeftTopY, childLeftTopY);
    int32_t intersectAreaRightBottonX = std::min(curFocusRightBottonX, childRightBottonX);
    int32_t intersectAreaRightBottonY = std::min(curFocusRightBottonY, childRightBottonY);

    int32_t intersectWidth = intersectAreaRightBottonX - intersectAreaLeftTopX + 1;
    int32_t intersectHeight = intersectAreaRightBottonY - intersectAreaLeftTopY + 1;

    return (intersectWidth < 0 || intersectHeight < 0) ? -1 : intersectWidth * intersectHeight;
}

double GridPattern::GetNearestDistanceFromChildToCurFocusItemInMainAxis(
    int32_t targetIndex, GridItemIndexInfo itemIndexInfo)
{
    double minDistance = std::numeric_limits<double>::max();
    auto mainAxisIndex =
        curFocusIndexInfo_.mainStart == -1 ? curFocusIndexInfo_.mainIndex : curFocusIndexInfo_.mainStart;
    auto mainAxisEndIndex =
        curFocusIndexInfo_.mainEnd == -1 ? curFocusIndexInfo_.mainIndex : curFocusIndexInfo_.mainEnd;
    for (int32_t i = mainAxisIndex; i <= mainAxisEndIndex; i++) {
        double childMainIndexDistance =
            CalcCoordinatesDistance(i, curFocusIndexInfo_.crossIndex, itemIndexInfo.mainIndex, targetIndex);
        double childMainStartDistance =
            itemIndexInfo.mainStart == -1
                ? std::numeric_limits<double>::max()
                : CalcCoordinatesDistance(i, curFocusIndexInfo_.crossIndex, itemIndexInfo.mainStart, targetIndex);
        double distance = std::min(childMainIndexDistance, childMainStartDistance);
        if (LessNotEqual(distance, minDistance)) {
            minDistance = distance;
        }
    }
    return minDistance;
}

double GridPattern::GetNearestDistanceFromChildToCurFocusItemInCrossAxis(
    int32_t targetIndex, GridItemIndexInfo itemIndexInfo)
{
    double minDistance = std::numeric_limits<double>::max();
    auto crossAxisIndex =
        curFocusIndexInfo_.crossStart == -1 ? curFocusIndexInfo_.crossIndex : curFocusIndexInfo_.crossStart;
    auto crossAxisEndIndex =
        curFocusIndexInfo_.crossEnd == -1 ? curFocusIndexInfo_.crossIndex : curFocusIndexInfo_.crossEnd;
    for (int32_t i = crossAxisIndex; i <= crossAxisEndIndex; i++) {
        double childCrossIndexDistance =
            CalcCoordinatesDistance(curFocusIndexInfo_.mainIndex, i, targetIndex, itemIndexInfo.crossIndex);
        double childCrossEndDistance =
            itemIndexInfo.crossEnd == -1
                ? std::numeric_limits<double>::max()
                : CalcCoordinatesDistance(curFocusIndexInfo_.mainIndex, i, targetIndex, itemIndexInfo.crossEnd);
        double distance = std::min(childCrossIndexDistance, childCrossEndDistance);
        if (LessNotEqual(distance, minDistance)) {
            minDistance = distance;
        }
    }
    return minDistance;
}

void GridPattern::ResetAllDirectionsStep()
{
    isLeftStep_ = false;
    isRightStep_ = false;
    isUpStep_ = false;
    isDownStep_ = false;
    isLeftEndStep_ = false;
    isRightEndStep_ = false;
}

WeakPtr<FocusHub> GridPattern::GetChildFocusNodeByIndex(int32_t tarMainIndex, int32_t tarCrossIndex, int32_t tarIndex)
{
    auto gridFrame = GetHost();
    CHECK_NULL_RETURN(gridFrame, nullptr);
    auto gridFocus = gridFrame->GetFocusHub();
    CHECK_NULL_RETURN(gridFocus, nullptr);
    auto childFocusList = gridFocus->GetChildren();
    for (const auto& childFocus : childFocusList) {
        auto childFrame = childFocus->GetFrameNode();
        if (!childFrame) {
            continue;
        }
        auto childPattern = childFrame->GetPattern();
        if (!childPattern) {
            continue;
        }
        auto childItemPattern = AceType::DynamicCast<GridItemPattern>(childPattern);
        if (!childItemPattern) {
            continue;
        }
        auto childItemProperty = childItemPattern->GetLayoutProperty<GridItemLayoutProperty>();
        if (!childItemProperty) {
            continue;
        }
        auto curMainIndex = childItemProperty->GetMainIndex().value_or(-1);
        auto curCrossIndex = childItemProperty->GetCrossIndex().value_or(-1);
        if (tarIndex < 0) {
            auto curMainSpan = childItemProperty->GetMainSpan(gridLayoutInfo_.axis_);
            auto curCrossSpan = childItemProperty->GetCrossSpan(gridLayoutInfo_.axis_);
            if (curMainIndex <= tarMainIndex && curMainIndex + curMainSpan > tarMainIndex &&
                curCrossIndex <= tarCrossIndex && curCrossIndex + curCrossSpan > tarCrossIndex) {
                return AceType::WeakClaim(AceType::RawPtr(childFocus));
            }
        } else {
            if (gridLayoutInfo_.gridMatrix_.find(curMainIndex) == gridLayoutInfo_.gridMatrix_.end()) {
                TAG_LOGW(AceLogTag::ACE_GRID, "Can not find target main index: %{public}d", curMainIndex);
                continue;
            }
            if (gridLayoutInfo_.gridMatrix_[curMainIndex].find(curCrossIndex) ==
                gridLayoutInfo_.gridMatrix_[curMainIndex].end()) {
                TAG_LOGW(AceLogTag::ACE_GRID, "Can not find target cross index: %{public}d", curCrossIndex);
                continue;
            }
            if (gridLayoutInfo_.gridMatrix_[curMainIndex][curCrossIndex] == tarIndex) {
                return AceType::WeakClaim(AceType::RawPtr(childFocus));
            }
        }
    }
    return nullptr;
}

std::unordered_set<int32_t> GridPattern::GetFocusableChildCrossIndexesAt(int32_t tarMainIndex)
{
    std::unordered_set<int32_t> result;
    auto gridFrame = GetHost();
    CHECK_NULL_RETURN(gridFrame, result);
    auto gridFocus = gridFrame->GetFocusHub();
    CHECK_NULL_RETURN(gridFocus, result);
    auto childFocusList = gridFocus->GetChildren();
    for (const auto& childFocus : childFocusList) {
        if (!childFocus->IsFocusable()) {
            continue;
        }
        auto childFrame = childFocus->GetFrameNode();
        if (!childFrame) {
            continue;
        }
        auto childPattern = childFrame->GetPattern();
        if (!childPattern) {
            continue;
        }
        auto childItemPattern = AceType::DynamicCast<GridItemPattern>(childPattern);
        if (!childItemPattern) {
            continue;
        }
        auto childItemProperty = childItemPattern->GetLayoutProperty<GridItemLayoutProperty>();
        if (!childItemProperty) {
            continue;
        }
        auto irregularInfo = childItemPattern->GetIrregularItemInfo();
        bool hasIrregularItemInfo = irregularInfo.has_value();
        auto curMainIndex = childItemProperty->GetMainIndex().value_or(-1);
        auto curCrossIndex = childItemProperty->GetCrossIndex().value_or(-1);
        auto curMainStart = hasIrregularItemInfo ? irregularInfo.value().mainStart
                                                 : childItemProperty->GetMainStart(gridLayoutInfo_.axis_);
        auto curMainEnd =
            hasIrregularItemInfo ? irregularInfo.value().mainEnd : childItemProperty->GetMainEnd(gridLayoutInfo_.axis_);
        if ((curMainIndex == tarMainIndex) ||
            (curMainStart >= 0 && curMainStart <= tarMainIndex && tarMainIndex <= curMainEnd)) {
            result.emplace(curCrossIndex);
        }
    }
    std::string output;
    for (const auto& index : result) {
        output += std::to_string(index);
    }
    return result;
}

void GridPattern::ScrollToFocusNode(const WeakPtr<FocusHub>& focusNode)
{
    auto nextFocus = focusNode.Upgrade();
    CHECK_NULL_VOID(nextFocus);
    UpdateStartIndex(GetFocusNodeIndex(nextFocus));
}

int32_t GridPattern::GetFocusNodeIndex(const RefPtr<FocusHub>& focusNode)
{
    auto tarFrame = focusNode->GetFrameNode();
    CHECK_NULL_RETURN(tarFrame, -1);
    auto tarPattern = tarFrame->GetPattern();
    CHECK_NULL_RETURN(tarPattern, -1);
    auto tarItemPattern = AceType::DynamicCast<GridItemPattern>(tarPattern);
    CHECK_NULL_RETURN(tarItemPattern, -1);
    auto tarItemProperty = tarItemPattern->GetLayoutProperty<GridItemLayoutProperty>();
    CHECK_NULL_RETURN(tarItemProperty, -1);
    auto tarMainIndex = tarItemProperty->GetMainIndex().value_or(-1);
    auto tarCrossIndex = tarItemProperty->GetCrossIndex().value_or(-1);
    if (gridLayoutInfo_.gridMatrix_.find(tarMainIndex) == gridLayoutInfo_.gridMatrix_.end()) {
        TAG_LOGW(AceLogTag::ACE_GRID, "Can not find target main index: %{public}d", tarMainIndex);
        if (tarMainIndex == 0) {
            return 0;
        }
        return gridLayoutInfo_.childrenCount_ - 1;
    }
    if (gridLayoutInfo_.gridMatrix_[tarMainIndex].find(tarCrossIndex) ==
        gridLayoutInfo_.gridMatrix_[tarMainIndex].end()) {
        TAG_LOGW(AceLogTag::ACE_GRID, "Can not find target cross index: %{public}d", tarCrossIndex);
        if (tarMainIndex == 0) {
            return 0;
        }
        return gridLayoutInfo_.childrenCount_ - 1;
    }
    return gridLayoutInfo_.gridMatrix_[tarMainIndex][tarCrossIndex];
}

void GridPattern::ScrollToFocusNodeIndex(int32_t index)
{
    UpdateStartIndex(index);
    auto pipeline = PipelineContext::GetCurrentContext();
    if (pipeline) {
        pipeline->FlushUITasks();
    }
    auto tarFocusNodeWeak = GetChildFocusNodeByIndex(-1, -1, index);
    auto tarFocusNode = tarFocusNodeWeak.Upgrade();
    if (tarFocusNode) {
        tarFocusNode->RequestFocusImmediately();
    }
}

bool GridPattern::ScrollToNode(const RefPtr<FrameNode>& focusFrameNode)
{
    CHECK_NULL_RETURN(focusFrameNode, false);
    auto focusHub = focusFrameNode->GetFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    auto scrollToIndex = GetFocusNodeIndex(focusHub);
    if (scrollToIndex < 0) {
        return false;
    }
    auto ret = UpdateStartIndex(scrollToIndex);
    auto pipeline = PipelineContext::GetCurrentContext();
    if (pipeline) {
        pipeline->FlushUITasks();
    }
    return ret;
}

std::pair<std::function<bool(float)>, Axis> GridPattern::GetScrollOffsetAbility()
{
    return { [wp = WeakClaim(this)](float moveOffset) -> bool {
                auto pattern = wp.Upgrade();
                CHECK_NULL_RETURN(pattern, false);
                pattern->ScrollBy(-moveOffset);
                return true;
            },
        GetAxis() };
}

std::function<bool(int32_t)> GridPattern::GetScrollIndexAbility()
{
    return [wp = WeakClaim(this)](int32_t index) -> bool {
        auto pattern = wp.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        if (index == FocusHub::SCROLL_TO_HEAD) {
            pattern->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, false);
        } else if (index == FocusHub::SCROLL_TO_TAIL) {
            pattern->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
        } else {
            pattern->UpdateStartIndex(index);
        }
        return true;
    };
}

void GridPattern::ScrollBy(float offset)
{
    StopAnimate();
    UpdateCurrentOffset(-offset, SCROLL_FROM_JUMP);
    // AccessibilityEventType::SCROLL_END
}

void GridPattern::ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    ScrollablePattern::ToJsonValue(json, filter);
    /* no fixed attr below, just return */
    if (filter.IsFastFilter()) {
        return;
    }
    json->PutExtAttr("multiSelectable", multiSelectable_ ? "true" : "false", filter);
    json->PutExtAttr("supportAnimation", supportAnimation_ ? "true" : "false", filter);
}

void GridPattern::InitOnKeyEvent(const RefPtr<FocusHub>& focusHub)
{
    auto onKeyEvent = [wp = WeakClaim(this)](const KeyEvent& event) -> bool {
        auto pattern = wp.Upgrade();
        if (pattern) {
            return pattern->OnKeyEvent(event);
        }
        return false;
    };
    focusHub->SetOnKeyEventInternal(std::move(onKeyEvent));
}

bool GridPattern::OnKeyEvent(const KeyEvent& event)
{
    if (event.action != KeyAction::DOWN) {
        return false;
    }
    if ((event.code == KeyCode::KEY_PAGE_DOWN) || (event.code == KeyCode::KEY_PAGE_UP)) {
        ScrollPage(event.code == KeyCode::KEY_PAGE_UP);
    }
    return false;
}

bool GridPattern::HandleDirectionKey(KeyCode code)
{
    if (code == KeyCode::KEY_DPAD_UP) {
        // Need to update: current selection
        return true;
    }
    if (code == KeyCode::KEY_DPAD_DOWN) {
        // Need to update: current selection
        return true;
    }
    return false;
}

void GridPattern::ScrollPage(bool reverse, bool smooth)
{
    float distance = reverse ? GetMainContentSize() : -GetMainContentSize();
    if (smooth) {
        float position = -gridLayoutInfo_.currentHeight_ + distance;
        ScrollablePattern::AnimateTo(-position, -1, nullptr, true, false, false);
        return;
    } else {
        if (!isConfigScrollable_) {
            return;
        }
        StopAnimate();
        UpdateCurrentOffset(distance, SCROLL_FROM_JUMP);
    }
    // AccessibilityEventType::SCROLL_END
}

bool GridPattern::UpdateStartIndex(int32_t index)
{
    if (!isConfigScrollable_) {
        return false;
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    gridLayoutInfo_.jumpIndex_ = index;
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    // AccessibilityEventType::SCROLL_END
    SetScrollSource(SCROLL_FROM_JUMP);
    return true;
}

bool GridPattern::UpdateStartIndex(int32_t index, ScrollAlign align)
{
    gridLayoutInfo_.scrollAlign_ = align;
    return UpdateStartIndex(index);
}

void GridPattern::OnAnimateStop()
{
    scrollStop_ = true;
    MarkDirtyNodeSelf();
    // AccessibilityEventType::SCROLL_END
}

void GridPattern::AnimateTo(float position, float duration, const RefPtr<Curve> &curve, bool smooth,
                            bool canOverScroll, bool useTotalOffset)
{
    if (!isConfigScrollable_) {
        return;
    }
    ScrollablePattern::AnimateTo(position, duration, curve, smooth, canOverScroll);
}

void GridPattern::ScrollTo(float position)
{
    if (!isConfigScrollable_) {
        return;
    }
    TAG_LOGI(AceLogTag::ACE_GRID, "ScrollTo:%{public}f", position);
    StopAnimate();
    UpdateCurrentOffset(GetTotalOffset() - position, SCROLL_FROM_JUMP);
    // AccessibilityEventType::SCROLL_END
}

float GridPattern::EstimateHeight() const
{
    if (!isConfigScrollable_) {
        return 0.0f;
    }
    // During the scrolling animation, the exact current position is used. Other times use the estimated location
    if (isSmoothScrolling_) {
        const auto* infoPtr = UseIrregularLayout() ? &gridLayoutInfo_ : &scrollGridLayoutInfo_;
        int32_t lineIndex = 0;
        infoPtr->GetLineIndexByIndex(gridLayoutInfo_.startIndex_, lineIndex);
        return infoPtr->GetTotalHeightFromZeroIndex(lineIndex, GetMainGap()) - gridLayoutInfo_.currentOffset_;
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 0.0);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0);
    const auto& info = gridLayoutInfo_;
    auto viewScopeSize = geometryNode->GetPaddingSize();
    auto layoutProperty = host->GetLayoutProperty<GridLayoutProperty>();
    auto mainGap = GridUtils::GetMainGap(layoutProperty, viewScopeSize, info.axis_);
    if (UseIrregularLayout()) {
        return info.GetIrregularOffset(mainGap);
    }
    if (!layoutProperty->GetLayoutOptions().has_value()) {
        return info.GetContentOffset(mainGap);
    }

    return info.GetContentOffset(layoutProperty->GetLayoutOptions().value(), mainGap);
}

float GridPattern::GetAverageHeight() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 0.0);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0);
    const auto& info = gridLayoutInfo_;
    auto viewScopeSize = geometryNode->GetPaddingSize();
    auto layoutProperty = host->GetLayoutProperty<GridLayoutProperty>();

    float heightSum = 0;
    int32_t itemCount = 0;
    auto mainGap = GridUtils::GetMainGap(layoutProperty, viewScopeSize, info.axis_);
    for (const auto& item : info.lineHeightMap_) {
        auto line = info.gridMatrix_.find(item.first);
        if (line == info.gridMatrix_.end()) {
            continue;
        }
        if (line->second.empty()) {
            continue;
        }
        auto lineStart = line->second.begin()->second;
        auto lineEnd = line->second.rbegin()->second;
        itemCount += (lineEnd - lineStart + 1);
        heightSum += item.second + mainGap;
    }
    if (itemCount == 0) {
        return 0;
    }
    return heightSum / itemCount;
}

float GridPattern::GetTotalHeight() const
{
    if (scrollbarInfo_.first.has_value() && scrollbarInfo_.second.has_value()) {
        return scrollbarInfo_.second.value();
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 0.0f);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    auto viewScopeSize = geometryNode->GetPaddingSize();
    auto layoutProperty = host->GetLayoutProperty<GridLayoutProperty>();
    auto mainGap = GridUtils::GetMainGap(layoutProperty, viewScopeSize, gridLayoutInfo_.axis_);
    if (UseIrregularLayout()) {
        return gridLayoutInfo_.GetIrregularHeight(mainGap);
    }
    return gridLayoutInfo_.GetContentHeight(mainGap);
}

void GridPattern::UpdateScrollBarOffset()
{
    if (!GetScrollBar() && !GetScrollBarProxy()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    const auto& info = gridLayoutInfo_;
    float offset = 0;
    float estimatedHeight = 0.f;
    if (scrollbarInfo_.first.has_value() && scrollbarInfo_.second.has_value()) {
        offset = scrollbarInfo_.first.value();
        estimatedHeight = scrollbarInfo_.second.value();
    } else {
        auto viewScopeSize = geometryNode->GetPaddingSize();
        auto layoutProperty = host->GetLayoutProperty<GridLayoutProperty>();
        auto mainGap = GridUtils::GetMainGap(layoutProperty, viewScopeSize, info.axis_);
        if (UseIrregularLayout()) {
            offset = info.GetIrregularOffset(mainGap);
            estimatedHeight = info.GetIrregularHeight(mainGap);
        } else if (!layoutProperty->GetLayoutOptions().has_value()) {
            offset = info.GetContentOffset(mainGap);
            estimatedHeight = info.GetContentHeight(mainGap);
        } else {
            offset = info.GetContentOffset(layoutProperty->GetLayoutOptions().value(), mainGap);
            estimatedHeight =
                info.GetContentHeight(layoutProperty->GetLayoutOptions().value(), info.childrenCount_, mainGap);
        }
    }
    if (info.startMainLineIndex_ != 0 && info.startIndex_ == 0) {
        for (int32_t lineIndex = info.startMainLineIndex_ - 1; lineIndex >= 0; lineIndex--) {
            offset += info.lineHeightMap_.find(lineIndex)->second;
        }
    }
    auto viewSize = geometryNode->GetFrameSize();
    auto overScroll = 0.0f;
    if (gridLayoutInfo_.reachStart_ && Positive(gridLayoutInfo_.currentOffset_)) {
        overScroll = gridLayoutInfo_.currentOffset_;
    } else {
        overScroll = gridLayoutInfo_.lastMainSize_ - estimatedHeight + offset;
        overScroll = Positive(overScroll) ? overScroll : 0.0f;
    }
    HandleScrollBarOutBoundary(overScroll);
    UpdateScrollBarRegion(offset, estimatedHeight, Size(viewSize.Width(), viewSize.Height()), Offset(0.0f, 0.0f));
}

DisplayMode GridPattern::GetDefaultScrollBarDisplayMode() const
{
    auto defaultDisplayMode = DisplayMode::OFF;
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
        defaultDisplayMode = DisplayMode::AUTO;
    }
    return defaultDisplayMode;
}

int32_t GridPattern::GetOriginalIndex() const
{
    return gridLayoutInfo_.GetOriginalIndex();
}

int32_t GridPattern::GetCrossCount() const
{
    return gridLayoutInfo_.crossCount_;
}

int32_t GridPattern::GetChildrenCount() const
{
    return gridLayoutInfo_.childrenCount_;
}

void GridPattern::ClearDragState()
{
    gridLayoutInfo_.ClearDragState();
    MarkDirtyNodeSelf();
}

void GridPattern::UpdateRectOfDraggedInItem(int32_t insertIndex)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::list<RefPtr<FrameNode>> children;
    host->GenerateOneDepthAllFrame(children);
    for (const auto& item : children) {
        auto itemPattern = item->GetPattern<GridItemPattern>();
        CHECK_NULL_VOID(itemPattern);
        auto itemProperty = itemPattern->GetLayoutProperty<GridItemLayoutProperty>();
        CHECK_NULL_VOID(itemProperty);
        auto mainIndex = itemProperty->GetMainIndex().value_or(-1);
        auto crossIndex = itemProperty->GetCrossIndex().value_or(-1);
        if (mainIndex * gridLayoutInfo_.crossCount_ + crossIndex == insertIndex) {
            auto size = item->GetRenderContext()->GetPaintRectWithTransform();
            size.SetOffset(item->GetTransformRelativeOffset());
            gridLayoutInfo_.currentRect_ = size;
            break;
        }
    }
}

void GridPattern::MoveItems(int32_t itemIndex, int32_t insertIndex)
{
    if (insertIndex < 0 ||
        insertIndex >= ((itemIndex == -1) ? (gridLayoutInfo_.childrenCount_ + 1) : gridLayoutInfo_.childrenCount_)) {
        return;
    }

    if (itemIndex == -1) {
        UpdateRectOfDraggedInItem(insertIndex);
    }

    gridLayoutInfo_.SwapItems(itemIndex, insertIndex);

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    auto pipeline = PipelineContext::GetCurrentContext();
    if (pipeline) {
        pipeline->FlushUITasks();
    }
}

bool GridPattern::IsOutOfBoundary(bool useCurrentDelta)
{
    auto scrollable = GetAlwaysEnabled() || (gridLayoutInfo_.startIndex_ > 0) ||
                      (gridLayoutInfo_.endIndex_ < gridLayoutInfo_.childrenCount_ - 1) ||
                      GreatNotEqual(gridLayoutInfo_.totalHeightOfItemsInView_, gridLayoutInfo_.lastMainSize_);

    bool outOfEnd = false;
    if (UseIrregularLayout()) {
        outOfEnd = gridLayoutInfo_.offsetEnd_;
    } else {
        outOfEnd = gridLayoutInfo_.IsOutOfEnd();
    }

    return scrollable && (gridLayoutInfo_.IsOutOfStart() || outOfEnd);
}

float GridPattern::GetEndOffset()
{
    auto& info = gridLayoutInfo_;
    float contentHeight = info.lastMainSize_ - info.contentEndPadding_;
    float mainGap = GetMainGap();
    bool regular = !UseIrregularLayout();
    float heightInView = info.GetTotalHeightOfItemsInView(mainGap, regular);

    if (GetAlwaysEnabled() && info.HeightSumSmaller(contentHeight, mainGap)) {
        // overScroll with contentHeight < viewport
        if (!regular) {
            return info.GetHeightInRange(0, info.startMainLineIndex_, mainGap);
        }
        float totalHeight = info.GetTotalLineHeight(mainGap);
        return totalHeight - heightInView;
    }

    if (regular) {
        return contentHeight - heightInView;
    }
    float disToBot = gridLayoutInfo_.GetDistanceToBottom(contentHeight, heightInView, mainGap);
    return gridLayoutInfo_.currentOffset_ - disToBot;
}

void GridPattern::SetEdgeEffectCallback(const RefPtr<ScrollEdgeEffect>& scrollEffect)
{
    scrollEffect->SetCurrentPositionCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto grid = weak.Upgrade();
        CHECK_NULL_RETURN(grid, 0.0);
        if (!grid->gridLayoutInfo_.synced_) {
            grid->SyncLayoutBeforeSpring();
        }
        return grid->gridLayoutInfo_.currentOffset_;
    });
    scrollEffect->SetLeadingCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto grid = weak.Upgrade();
        CHECK_NULL_RETURN(grid, 0.0);
        return grid->GetEndOffset();
    });
    scrollEffect->SetTrailingCallback([]() -> double { return 0.0; });
    scrollEffect->SetInitLeadingCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto grid = weak.Upgrade();
        CHECK_NULL_RETURN(grid, 0.0);
        return grid->GetEndOffset();
    });
    scrollEffect->SetInitTrailingCallback([]() -> double { return 0.0; });
}

void GridPattern::SyncLayoutBeforeSpring()
{
    auto& info = gridLayoutInfo_;
    if (info.synced_) {
        return;
    }
    if (!UseIrregularLayout()) {
        float delta = info.currentOffset_ - info.prevOffset_;
        if (!info.lineHeightMap_.empty() && LessOrEqual(delta, -info.lineHeightMap_.rbegin()->second)) {
            // old layout can't handle large overScroll offset. Avoid by skipping this layout.
            // Spring animation plays immediately afterwards, so losing this frame's offset is fine
            info.currentOffset_ = info.prevOffset_;
            info.synced_ = true;
            return;
        }
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->SetActive();
    host->CreateLayoutTask();
}

void GridPattern::GetEndOverScrollIrregular(OverScrollOffset& offset, float delta) const
{
    const auto& info = gridLayoutInfo_;
    float disToBot = info.GetDistanceToBottom(
        info.lastMainSize_ - info.contentEndPadding_, info.totalHeightOfItemsInView_, GetMainGap());
    if (!info.offsetEnd_) {
        offset.end = std::min(0.0f, disToBot + static_cast<float>(delta));
    } else if (Negative(delta)) {
        offset.end = delta;
    } else {
        offset.end = std::min(static_cast<float>(delta), -disToBot);
    }
}

OverScrollOffset GridPattern::GetOverScrollOffset(double delta) const
{
    OverScrollOffset offset = { 0, 0 };
    if (gridLayoutInfo_.startIndex_ == 0 && gridLayoutInfo_.startMainLineIndex_ == 0) {
        auto startPos = gridLayoutInfo_.currentOffset_;
        auto newStartPos = startPos + delta;
        if (startPos > 0 && newStartPos > 0) {
            offset.start = delta;
        }
        if (startPos > 0 && newStartPos <= 0) {
            offset.start = -startPos;
        }
        if (startPos <= 0 && newStartPos > 0) {
            offset.start = newStartPos;
        }
    }
    if (UseIrregularLayout()) {
        GetEndOverScrollIrregular(offset, static_cast<float>(delta));
        return offset;
    }
    if (gridLayoutInfo_.endIndex_ == gridLayoutInfo_.childrenCount_ - 1) {
        float endPos = gridLayoutInfo_.currentOffset_ + gridLayoutInfo_.totalHeightOfItemsInView_;
        float mainSize = gridLayoutInfo_.lastMainSize_ - gridLayoutInfo_.contentEndPadding_;
        if (GreatNotEqual(
                GetMainContentSize(), gridLayoutInfo_.currentOffset_ + gridLayoutInfo_.totalHeightOfItemsInView_)) {
            endPos = gridLayoutInfo_.currentOffset_ + GetMainContentSize();
        }
        float newEndPos = endPos + delta;
        if (endPos < mainSize && newEndPos < mainSize) {
            offset.end = delta;
        }
        if (endPos < mainSize && newEndPos >= mainSize) {
            offset.end = mainSize - endPos;
        }
        if (endPos >= mainSize && newEndPos < mainSize) {
            offset.end = newEndPos - mainSize;
        }
    }
    return offset;
}

void GridPattern::SetAccessibilityAction()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto accessibilityProperty = host->GetAccessibilityProperty<AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    accessibilityProperty->SetActionScrollForward([weakPtr = WeakClaim(this)]() {
        const auto& pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (!pattern->IsScrollable()) {
            return;
        }
        pattern->ScrollPage(false);
    });

    accessibilityProperty->SetActionScrollBackward([weakPtr = WeakClaim(this)]() {
        const auto& pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (!pattern->IsScrollable()) {
            return;
        }
        pattern->ScrollPage(true);
    });
}

void GridPattern::DumpAdvanceInfo()
{
    auto property = GetLayoutProperty<GridLayoutProperty>();
    CHECK_NULL_VOID(property);
    ScrollablePattern::DumpAdvanceInfo();
    supportAnimation_ ? DumpLog::GetInstance().AddDesc("supportAnimation:true")
                      : DumpLog::GetInstance().AddDesc("supportAnimation:false");
    isConfigScrollable_ ? DumpLog::GetInstance().AddDesc("isConfigScrollable:true")
                        : DumpLog::GetInstance().AddDesc("isConfigScrollable:false");
    gridLayoutInfo_.lastCrossCount_.has_value()
        ? DumpLog::GetInstance().AddDesc("lastCrossCount:" + std::to_string(gridLayoutInfo_.lastCrossCount_.value()))
        : DumpLog::GetInstance().AddDesc("lastCrossCount:null");
    gridLayoutInfo_.reachEnd_ ? DumpLog::GetInstance().AddDesc("reachEnd:true")
                              : DumpLog::GetInstance().AddDesc("reachEnd:false");
    gridLayoutInfo_.reachStart_ ? DumpLog::GetInstance().AddDesc("reachStart:true")
                                : DumpLog::GetInstance().AddDesc("reachStart:false");
    gridLayoutInfo_.offsetEnd_ ? DumpLog::GetInstance().AddDesc("offsetEnd:true")
                               : DumpLog::GetInstance().AddDesc("offsetEnd:false");
    gridLayoutInfo_.hasBigItem_ ? DumpLog::GetInstance().AddDesc("hasBigItem:true")
                                : DumpLog::GetInstance().AddDesc("hasBigItem:false");
    gridLayoutInfo_.synced_ ? DumpLog::GetInstance().AddDesc("synced:true")
                            : DumpLog::GetInstance().AddDesc("synced:false");
    DumpLog::GetInstance().AddDesc("scrollStop:" + std::to_string(scrollStop_));
    DumpLog::GetInstance().AddDesc("prevHeight:" + std::to_string(gridLayoutInfo_.prevHeight_));
    DumpLog::GetInstance().AddDesc("currentHeight:" + std::to_string(gridLayoutInfo_.currentHeight_));
    DumpLog::GetInstance().AddDesc("endHeight:" + std::to_string(endHeight_));
    DumpLog::GetInstance().AddDesc("currentOffset:" + std::to_string(gridLayoutInfo_.currentOffset_));
    DumpLog::GetInstance().AddDesc("prevOffset:" + std::to_string(gridLayoutInfo_.prevOffset_));
    DumpLog::GetInstance().AddDesc("lastMainSize:" + std::to_string(gridLayoutInfo_.lastMainSize_));
    DumpLog::GetInstance().AddDesc(
        "totalHeightOfItemsInView:" + std::to_string(gridLayoutInfo_.totalHeightOfItemsInView_));
    DumpLog::GetInstance().AddDesc("startIndex:" + std::to_string(gridLayoutInfo_.startIndex_));
    DumpLog::GetInstance().AddDesc("endIndex:" + std::to_string(gridLayoutInfo_.endIndex_));
    DumpLog::GetInstance().AddDesc("jumpIndex:" + std::to_string(gridLayoutInfo_.jumpIndex_));
    DumpLog::GetInstance().AddDesc("crossCount:" + std::to_string(gridLayoutInfo_.crossCount_));
    DumpLog::GetInstance().AddDesc("childrenCount:" + std::to_string(gridLayoutInfo_.childrenCount_));
    DumpLog::GetInstance().AddDesc("RowsTemplate:", property->GetRowsTemplate()->c_str());
    DumpLog::GetInstance().AddDesc("ColumnsTemplate:", property->GetColumnsTemplate()->c_str());
    property->GetCachedCount().has_value()
        ? DumpLog::GetInstance().AddDesc("CachedCount:" + std::to_string(property->GetCachedCount().value()))
        : DumpLog::GetInstance().AddDesc("CachedCount:null");
    property->GetMaxCount().has_value()
        ? DumpLog::GetInstance().AddDesc("MaxCount:" + std::to_string(property->GetMaxCount().value()))
        : DumpLog::GetInstance().AddDesc("MaxCount:null");
    property->GetMinCount().has_value()
        ? DumpLog::GetInstance().AddDesc("MinCount:" + std::to_string(property->GetMinCount().value()))
        : DumpLog::GetInstance().AddDesc("MinCount:null");
    property->GetCellLength().has_value()
        ? DumpLog::GetInstance().AddDesc("CellLength:" + std::to_string(property->GetCellLength().value()))
        : DumpLog::GetInstance().AddDesc("CellLength:null");
    property->GetEditable().has_value()
        ? DumpLog::GetInstance().AddDesc("Editable:" + std::to_string(property->GetEditable().value()))
        : DumpLog::GetInstance().AddDesc("Editable:null");
    property->GetScrollEnabled().has_value()
        ? DumpLog::GetInstance().AddDesc("ScrollEnabled:" + std::to_string(property->GetScrollEnabled().value()))
        : DumpLog::GetInstance().AddDesc("ScrollEnabled:null");
    switch (gridLayoutInfo_.scrollAlign_) {
        case ScrollAlign::NONE: {
            DumpLog::GetInstance().AddDesc("ScrollAlign:NONE");
            break;
        }
        case ScrollAlign::CENTER: {
            DumpLog::GetInstance().AddDesc("ScrollAlign:CENTER");
            break;
        }
        case ScrollAlign::END: {
            DumpLog::GetInstance().AddDesc("ScrollAlign:END");
            break;
        }
        case ScrollAlign::START: {
            DumpLog::GetInstance().AddDesc("ScrollAlign:START");
            break;
        }
        case ScrollAlign::AUTO: {
            DumpLog::GetInstance().AddDesc("ScrollAlign:AUTO");
            break;
        }
        default: {
            break;
        }
    }
    if (!gridLayoutInfo_.gridMatrix_.empty()) {
        DumpLog::GetInstance().AddDesc("-----------start print gridMatrix------------");
        std::string res = std::string("");
        for (auto item : gridLayoutInfo_.gridMatrix_) {
            res.append(std::to_string(item.first));
            res.append(": ");
            for (auto index : item.second) {
                res.append("[")
                    .append(std::to_string(index.first))
                    .append(",")
                    .append(std::to_string(index.second))
                    .append("] ");
            }
            DumpLog::GetInstance().AddDesc(res);
            res.clear();
        }
        DumpLog::GetInstance().AddDesc("-----------end print gridMatrix------------");
    }
    if (!gridLayoutInfo_.lineHeightMap_.empty()) {
        DumpLog::GetInstance().AddDesc("-----------start print lineHeightMap------------");
        for (auto item : gridLayoutInfo_.lineHeightMap_) {
            DumpLog::GetInstance().AddDesc(std::to_string(item.first).append(" :").append(std::to_string(item.second)));
        }
        DumpLog::GetInstance().AddDesc("-----------end print lineHeightMap------------");
    }
    if (!gridLayoutInfo_.irregularItemsPosition_.empty()) {
        DumpLog::GetInstance().AddDesc("-----------start print irregularItemsPosition_------------");
        for (auto item : gridLayoutInfo_.irregularItemsPosition_) {
            DumpLog::GetInstance().AddDesc(std::to_string(item.first).append(" :").append(std::to_string(item.second)));
        }
        DumpLog::GetInstance().AddDesc("-----------end print irregularItemsPosition_------------");
    }
}

std::string GridPattern::ProvideRestoreInfo()
{
    return std::to_string(gridLayoutInfo_.startIndex_);
}

void GridPattern::OnRestoreInfo(const std::string& restoreInfo)
{
    gridLayoutInfo_.jumpIndex_ = StringUtils::StringToInt(restoreInfo);
    gridLayoutInfo_.scrollAlign_ = ScrollAlign::START;
}

Rect GridPattern::GetItemRect(int32_t index) const
{
    if (index < 0 || index < gridLayoutInfo_.startIndex_ || index > gridLayoutInfo_.endIndex_) {
        return Rect();
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, Rect());
    auto item = host->GetChildByIndex(index);
    CHECK_NULL_RETURN(item, Rect());
    auto itemGeometry = item->GetGeometryNode();
    CHECK_NULL_RETURN(itemGeometry, Rect());
    return Rect(itemGeometry->GetFrameRect().GetX(), itemGeometry->GetFrameRect().GetY(),
        itemGeometry->GetFrameRect().Width(), itemGeometry->GetFrameRect().Height());
}

void GridPattern::ScrollToIndex(int32_t index, bool smooth, ScrollAlign align, std::optional<float> extraOffset)
{
    SetScrollSource(SCROLL_FROM_JUMP);
    StopAnimate();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    int32_t totalChildCount = host->TotalChildCount();
    if (((index >= 0) && (index < totalChildCount)) || (index == LAST_ITEM)) {
        if (smooth) {
            SetExtraOffset(extraOffset);
            targetIndex_ = index;
            scrollAlign_ = align;
            host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        } else {
            UpdateStartIndex(index, align);
            if (extraOffset.has_value()) {
                gridLayoutInfo_.extraOffset_ = -extraOffset.value();
            }
        }
    }
    FireAndCleanScrollingListener();
}

void GridPattern::ScrollToEdge(ScrollEdgeType scrollEdgeType, bool smooth)
{
    if (UseIrregularLayout() && scrollEdgeType == ScrollEdgeType::SCROLL_BOTTOM) {
        // for irregular layout, last item might not be at bottom
        gridLayoutInfo_.jumpIndex_ = JUMP_TO_BOTTOM_EDGE;
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        return;
    }
    ScrollablePattern::ScrollToEdge(scrollEdgeType, smooth);
}

// Turn on the scrolling animation
void GridPattern::AnimateToTarget(ScrollAlign align, RefPtr<LayoutAlgorithmWrapper>& layoutAlgorithmWrapper)
{
    if (targetIndex_.has_value()) {
        AnimateToTargetImp(align, layoutAlgorithmWrapper);
        targetIndex_.reset();
    }
}

// scroll to the item where the index is located
bool GridPattern::AnimateToTargetImp(ScrollAlign align, RefPtr<LayoutAlgorithmWrapper>& layoutAlgorithmWrapper)
{
    float mainGap = GetMainGap();
    float targetPos = 0.0f;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto extraOffset = GetExtraOffset();
    auto success = true;
    if (UseIrregularLayout()) {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, false);
        auto size = GridLayoutUtils::GetItemSize(&gridLayoutInfo_, RawPtr(host), *targetIndex_);
        targetPos = gridLayoutInfo_.GetAnimatePosIrregular(*targetIndex_, size.rows, align, mainGap);
        if (Negative(targetPos)) {
            return false;
        }
    } else {
        auto gridScrollLayoutAlgorithm =
            DynamicCast<GridScrollLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
        scrollGridLayoutInfo_ = gridScrollLayoutAlgorithm->GetScrollGridLayoutInfo();
        // Based on the index, align gets the position to scroll to
        success = scrollGridLayoutInfo_.GetGridItemAnimatePos(
            gridLayoutInfo_, targetIndex_.value(), align, mainGap, targetPos);
        if (!success) {
            if (extraOffset.has_value()) {
                targetPos = GetTotalOffset();
            } else {
                return false;
            }
        }
    }

    isSmoothScrolling_ = true;
    if (extraOffset.has_value()) {
        auto initPos = targetPos;
        targetPos += extraOffset.value();
        ACE_SCOPED_TRACE("AnimateToTargetImpl, success:%u, initPos:%f, extraOffset:%f, targetPos:%f", success, initPos,
            extraOffset.value(), targetPos);
        ResetExtraOffset();
    } else {
        ACE_SCOPED_TRACE("AnimateToTargetImpl, extraOffset is null, targetPos:%f", targetPos);
    }
    AnimateTo(targetPos, -1, nullptr, true);
    return true;
}

std::vector<RefPtr<FrameNode>> GridPattern::GetVisibleSelectedItems()
{
    std::vector<RefPtr<FrameNode>> children;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, children);
    for (int32_t index = gridLayoutInfo_.startIndex_; index <= gridLayoutInfo_.endIndex_; ++index) {
        auto item = host->GetChildByIndex(index);
        if (!AceType::InstanceOf<FrameNode>(item)) {
            continue;
        }
        auto itemFrameNode = AceType::DynamicCast<FrameNode>(item);
        auto itemPattern = itemFrameNode->GetPattern<GridItemPattern>();
        if (!itemPattern) {
            continue;
        }
        if (!itemPattern->IsSelected()) {
            continue;
        }
        children.emplace_back(itemFrameNode);
    }
    return children;
}

void GridPattern::StopAnimate()
{
    ScrollablePattern::StopAnimate();
    isSmoothScrolling_ = false;
}

bool GridPattern::IsPredictOutOfRange(int32_t index) const
{
    CHECK_NULL_RETURN(gridLayoutInfo_.reachEnd_, false);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, true);
    auto gridLayoutProperty = host->GetLayoutProperty<GridLayoutProperty>();
    CHECK_NULL_RETURN(gridLayoutProperty, true);
    auto cacheCount = gridLayoutProperty->GetCachedCountValue(0) * gridLayoutInfo_.crossCount_;
    return index < gridLayoutInfo_.startIndex_ - cacheCount || index > gridLayoutInfo_.endIndex_ + cacheCount;
}

inline bool GridPattern::UseIrregularLayout() const
{
    return irregular_ || (SystemProperties::GetGridIrregularLayoutEnabled() &&
                             GetLayoutProperty<GridLayoutProperty>()->HasLayoutOptions());
}

bool GridPattern::IsReverse() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto gridLayoutProperty = host->GetLayoutProperty<GridLayoutProperty>();
    CHECK_NULL_RETURN(gridLayoutProperty, false);
    return gridLayoutProperty->IsReverse();
}
} // namespace OHOS::Ace::NG
