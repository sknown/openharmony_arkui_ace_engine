/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"

#include "base/geometry/axis.h"
#include "base/geometry/point.h"
#include "base/log/dump_log.h"
#include "base/perfmonitor/perf_constants.h"
#include "base/perfmonitor/perf_monitor.h"
#include "base/ressched/ressched_report.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/base/observer_handler.h"
#include "core/components_ng/manager/select_overlay/select_overlay_scroll_notifier.h"
#include "core/components_ng/pattern/scroll/effect/scroll_fade_effect.h"
#include "core/components_ng/pattern/scroll/scroll_event_hub.h"
#include "core/components_ng/pattern/scroll/scroll_spring_effect.h"
#include "core/components_ng/pattern/scrollable/scrollable.h"
#include "core/components_ng/pattern/scrollable/scrollable_event_hub.h"
#include "core/components_ng/pattern/scrollable/scrollable_properties.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr Color SELECT_FILL_COLOR = Color(0x1A000000);
constexpr Color SELECT_STROKE_COLOR = Color(0x33FFFFFF);
constexpr float CUSTOM_ANIMATION_DURATION = 1000.0;
constexpr uint32_t MILLOS_PER_NANO_SECONDS = 1000 * 1000 * 1000;
constexpr uint64_t MIN_DIFF_VSYNC = 1000 * 1000; // min is 1ms
constexpr uint32_t MAX_VSYNC_DIFF_TIME = 100 * 1000 * 1000; //max 100ms
constexpr uint32_t EVENTS_FIRED_INFO_COUNT = 50;
constexpr uint32_t SCROLLABLE_FRAME_INFO_COUNT = 50;
constexpr float SPRING_ACCURACY = 0.1;
const std::string SCROLLABLE_DRAG_SCENE = "scrollable_drag_scene";
const std::string SCROLL_BAR_DRAG_SCENE = "scrollBar_drag_scene";
const std::string SCROLLABLE_MOTION_SCENE = "scrollable_motion_scene";
const std::string SCROLLABLE_MULTI_TASK_SCENE = "scrollable_multi_task_scene";
const std::string SCROLL_IN_HOTZONE_SCENE = "scroll_in_hotzone_scene";
const std::string CUSTOM_SCROLL_BAR_SCENE = "custom_scroll_bar_scene";
} // namespace
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

ScrollablePattern::ScrollablePattern()
{
    friction_ = Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_ELEVEN) ? API11_FRICTION : FRICTION;
    friction_ = Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE) ? API12_FRICTION : friction_;
}

ScrollablePattern::ScrollablePattern(EdgeEffect edgeEffect, bool alwaysEnabled)
    : edgeEffect_(edgeEffect), edgeEffectAlwaysEnabled_(alwaysEnabled)
{
    friction_ = Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_ELEVEN) ? API11_FRICTION : FRICTION;
    friction_ = Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE) ? API12_FRICTION : friction_;
}

RefPtr<PaintProperty> ScrollablePattern::CreatePaintProperty()
{
    auto defaultDisplayMode = GetDefaultScrollBarDisplayMode();
    auto property = MakeRefPtr<ScrollablePaintProperty>();
    property->UpdateScrollBarMode(defaultDisplayMode);
    return property;
}

void ScrollablePattern::ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    /* no fixed attr below, just return */
    if (filter.IsFastFilter()) {
        return;
    }
    json->PutExtAttr("friction", GetFriction(), filter);
    if (edgeEffect_ == EdgeEffect::SPRING) {
        json->PutExtAttr("edgeEffect", "EdgeEffect.Spring", filter);
    } else if (edgeEffect_ == EdgeEffect::FADE) {
        json->PutExtAttr("edgeEffect", "EdgeEffect.Fade", filter);
    } else {
        json->PutExtAttr("edgeEffect", "EdgeEffect.None", filter);
    }
    json->PutExtAttr("flingSpeedLimit",
        Dimension(maxFlingVelocity_, DimensionUnit::VP).ToString().c_str(), filter);
    auto JsonEdgeEffectOptions = JsonUtil::Create(true);
    JsonEdgeEffectOptions->Put("alwaysEnabled", GetAlwaysEnabled());
    json->PutExtAttr("edgeEffectOptions", JsonEdgeEffectOptions, filter);
}

void ScrollablePattern::SetAxis(Axis axis)
{
    if (axis_ == axis) {
        return;
    }
    axis_ = axis;
    SetParentScrollable();
    if (scrollBar_) {
        auto positionMode = GetPositionMode();
        scrollBar_->SetPositionMode(positionMode);
        if (scrollBarOverlayModifier_) {
            scrollBarOverlayModifier_->SetPositionMode(positionMode);
        }
    }
    auto gestureHub = GetGestureHub();
    CHECK_NULL_VOID(gestureHub);
    if (scrollableEvent_) {
        gestureHub->RemoveScrollableEvent(scrollableEvent_);
        scrollableEvent_->SetAxis(axis);
        gestureHub->AddScrollableEvent(scrollableEvent_);
    }
    if (scrollEffect_) {
        gestureHub->RemoveScrollEdgeEffect(scrollEffect_);
        gestureHub->AddScrollEdgeEffect(GetAxis(), scrollEffect_);
    }
}

RefPtr<GestureEventHub> ScrollablePattern::GetGestureHub()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(hub, nullptr);
    return hub->GetOrCreateGestureEventHub();
}

RefPtr<InputEventHub> ScrollablePattern::GetInputHub()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(host, nullptr);
    return hub->GetOrCreateInputEventHub();
}

bool ScrollablePattern::OnScrollCallback(float offset, int32_t source)
{
    if (source == SCROLL_FROM_START) {
        FireOnScrollStart();
        return true;
    }
    SuggestOpIncGroup(true);
    return UpdateCurrentOffset(offset, source);
}

void ScrollablePattern::ProcessNavBarReactOnStart()
{
    CHECK_NULL_VOID(navBarPattern_);
    navBarPattern_->OnCoordScrollStart();
}

float ScrollablePattern::ProcessNavBarReactOnUpdate(float offset)
{
    CHECK_NULL_RETURN(navBarPattern_, false);
    return navBarPattern_->OnCoordScrollUpdate(offset);
}

void ScrollablePattern::ProcessNavBarReactOnEnd()
{
    CHECK_NULL_VOID(navBarPattern_);
    navBarPattern_->OnCoordScrollEnd();
}

bool ScrollablePattern::OnScrollPosition(double& offset, int32_t source)
{
    auto isSearchRefresh = GetIsSearchRefresh();
    if (needLinked_) {
        auto isAtTop = (IsAtTop() && Positive(offset));
        auto refreshCoordinateMode = RefreshCoordinationMode::UNKNOWN;
        if (!AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE) ||
            !isSearchRefresh) {
            refreshCoordinateMode = CoordinateWithRefresh(offset, source, isAtTop);
        }
        auto navigationInCoordination = CoordinateWithNavigation(offset, source, isAtTop);
        if ((refreshCoordinateMode == RefreshCoordinationMode::REFRESH_SCROLL) || navigationInCoordination) {
            return false;
        }
    }

    if (source == SCROLL_FROM_START) {
        SetParentScrollable();
        StopScrollBarAnimatorByProxy();
        AbortScrollAnimator();
    } else if (!AnimateStoped()) {
        return false;
    }
    return true;
}

namespace {
inline bool FromDrag(int32_t source)
{
    return source == SCROLL_FROM_UPDATE || source == SCROLL_FROM_AXIS;
}
} // namespace

bool ScrollablePattern::NeedSplitScroll(OverScrollOffset& overOffsets, int32_t source)
{
    return GreatNotEqual(overOffsets.start, 0.0) && refreshCoordination_ && refreshCoordination_->InCoordination() &&
           !isRefreshInReactive_ &&
           (FromDrag(source) || source == SCROLL_FROM_ANIMATION_SPRING ||
               source == SCROLL_FROM_ANIMATION) &&
           axis_ == Axis::VERTICAL;
}

RefreshCoordinationMode ScrollablePattern::CoordinateWithRefresh(double& offset, int32_t source, bool isAtTop)
{
    // use first scroll update to trigger scrollStart. Ignore SCROLL_FROM_START.
    if (source == SCROLL_FROM_START) {
        return RefreshCoordinationMode::UNKNOWN;
    }
    if (!refreshCoordination_) {
        CreateRefreshCoordination();
    }
    auto overOffsets = GetOverScrollOffset(offset);
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN) && !IsAtTop() && Positive(offset) &&
        NeedSplitScroll(overOffsets, source)) {
        offset = offset - overOffsets.start;
        OnScrollCallback(offset, source);
        isRefreshInReactive_ = true;
        refreshCoordination_->OnScrollStart(FromDrag(source), GetVelocity());
    }
    bool hasScrollSpace = Positive(offset) || (Negative(offset) && refreshCoordination_->IsRefreshInScroll());
    if (IsAtTop() && hasScrollSpace &&
        (FromDrag(source) || source == SCROLL_FROM_ANIMATION) &&
        !isRefreshInReactive_ && (axis_ == Axis::VERTICAL)) {
        isRefreshInReactive_ = true;
        refreshCoordination_->OnScrollStart(FromDrag(source), GetVelocity());
    }
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN) &&
        refreshCoordination_->InCoordination() && source != SCROLL_FROM_UPDATE &&
        source != SCROLL_FROM_AXIS && isRefreshInReactive_) {
        isRefreshInReactive_ = false;
        refreshCoordination_->OnScrollEnd(GetVelocity());
    }
    auto mode = RefreshCoordinationMode::UNKNOWN;
    if (refreshCoordination_->InCoordination() && isRefreshInReactive_) {
        if (!refreshCoordination_->OnScroll(
                GreatNotEqual(overOffsets.start, 0.0) ? overOffsets.start : offset, GetVelocity())) {
            isRefreshInReactive_ = false;
        }
        if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
            mode = RefreshCoordinationMode::REFRESH_SCROLL;
        } else {
            if (scrollEffect_ && scrollEffect_->IsSpringEffect()) {
                mode = RefreshCoordinationMode::SCROLLABLE_SCROLL;
            } else {
                mode = RefreshCoordinationMode::REFRESH_SCROLL;
            }
        }
    }
    return mode;
}

bool ScrollablePattern::CoordinateWithNavigation(double& offset, int32_t source, bool isAtTop)
{
    if (source == SCROLL_FROM_START) {
        GetParentNavigation();
        CHECK_NULL_RETURN(navBarPattern_, false);
        if (isAtTop) {
            // Starting coordinating scroll at the beginning of scrolling.
            isReactInParentMovement_ = true;
            ProcessNavBarReactOnStart();
        }
        return false;
    }

    CHECK_NULL_RETURN(navBarPattern_ && navBarPattern_->NeedCoordWithScroll(), false);

    auto overOffsets = GetOverScrollOffset(offset);
    float offsetRemain = 0.0f;
    float offsetCoordinate = offset;

    if (!isReactInParentMovement_ && NeedCoordinateScrollWithNavigation(offset, source, overOffsets)) {
        // Starting coordinating scroll during sliding or flipping.
        isReactInParentMovement_ = true;
        ProcessNavBarReactOnStart();
        offsetRemain = offset - overOffsets.start;
        offsetCoordinate = overOffsets.start;
    }

    if (isReactInParentMovement_) {
        float handledByNav = ProcessNavBarReactOnUpdate(offsetCoordinate);
        if (NearEqual(handledByNav, offsetCoordinate) && !NearZero(offset)) {
            // All offsets are handled by Navigation, list cannot scroll over.
            SetCanOverScroll(false);
            offset = offsetRemain;
        } else {
            // Not all offsets are handled by Navigation, list still needs to scroll.
            if (Positive(offset)) {
                // When scrolling down, allow list to scroll over.
                SetCanOverScroll(true);
            }
            offset = offsetRemain + (offsetCoordinate - handledByNav);
        }

        if (Negative(offset) && source == SCROLL_FROM_ANIMATION_SPRING) {
            // When rebounding form scrolling over, trigger the ProcessNavBarReactOnEnd callback.
            isReactInParentMovement_ = false;
            ProcessNavBarReactOnEnd();
        }
    }

    return false;
}

void ScrollablePattern::SetUiDvsyncSwitch(bool on)
{
    auto context = OHOS::Ace::NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    if (on && inScrollingStatus_) {
        inScrollingStatus_ = false;
        context->SetUiDvsyncSwitch(true);
        switchOnStatus_ = true;
    } else if (!on && switchOnStatus_) {
        context->SetUiDvsyncSwitch(false);
        switchOnStatus_ = false;
    }
}

void ScrollablePattern::OnScrollEnd()
{
    // Previous: Sets ScrollablePattern::OnScrollEnd to Scrollable->scrollEndCallback_
    // Scrollable calls scrollEndCallback_ in HandleOverScroll

    // Now: HandleOverScroll moved to ScrollablePattern and renamed HandleScrollVelocity, directly
    // calls OnScrollEnd in ScrollablePattern
    if (refreshCoordination_) {
        isRefreshInReactive_ = false;
        refreshCoordination_->OnScrollEnd(GetVelocity());
    }
    if (isReactInParentMovement_) {
        isReactInParentMovement_ = false;
        ProcessNavBarReactOnEnd();
    }
    if (isAnimationStop_) {
        SetUiDvsyncSwitch(false);
    }

    OnScrollEndCallback();
    SelectOverlayScrollNotifier::NotifyOnScrollEnd(WeakClaim(this));
}

void ScrollablePattern::AttachAnimatableProperty(RefPtr<Scrollable> scrollable)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto property = scrollable->GetFrictionProperty();
    renderContext->AttachNodeAnimatableProperty(property);

    property = scrollable->GetSpringProperty();
    renderContext->AttachNodeAnimatableProperty(property);
    property = scrollable->GetSnapProperty();
    renderContext->AttachNodeAnimatableProperty(property);
}

void ScrollablePattern::AddScrollEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureHub = GetGestureHub();
    CHECK_NULL_VOID(gestureHub);
    if (scrollableEvent_) {
        gestureHub->RemoveScrollableEvent(scrollableEvent_);
    }
    auto scrollCallback = [weak = WeakClaim(this)](double offset, int32_t source) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->HandleScrollImpl(static_cast<float>(offset), source);
    };
    auto scrollable = MakeRefPtr<Scrollable>(std::move(scrollCallback), GetAxis());
    scrollable->SetNodeId(host->GetAccessibilityId());
    scrollable->SetNodeTag(host->GetTag());
    scrollable->Initialize(host->GetContextRefPtr());
    AttachAnimatableProperty(scrollable);

    // move HandleScroll and HandleOverScroll to ScrollablePattern by setting callbacks to scrollable
    auto handleScroll = [weak = AceType::WeakClaim(this)](
                            float offset, int32_t source, NestedState state) -> ScrollResult {
        auto pattern = weak.Upgrade();
        if (pattern) {
            return pattern->HandleScroll(offset, source, state, pattern->GetVelocity());
        }
        return {};
    };
    scrollable->SetHandleScrollCallback(std::move(handleScroll));

    scrollable->SetOverScrollCallback([weak = WeakClaim(this)](float velocity) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->HandleOverScroll(velocity);
    });

    scrollable->SetIsReverseCallback([weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->IsReverse();
    });

    auto scrollStart = [weak = WeakClaim(this)](float position) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->FireAndCleanScrollingListener();
        pattern->OnScrollStartRecursive(position, pattern->GetVelocity());
    };
    scrollable->SetOnScrollStartRec(std::move(scrollStart));

    auto scrollEndRec = [weak = WeakClaim(this)](const std::optional<float>& velocity) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnScrollEndRecursive(velocity);
    };
    scrollable->SetOnScrollEndRec(std::move(scrollEndRec));

    auto scrollEnd = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnScrollEnd();
    };
    scrollable->SetScrollEndCallback(std::move(scrollEnd));
    scrollable->SetUnstaticFriction(friction_);
    scrollable->SetMaxFlingVelocity(maxFlingVelocity_);

    auto scrollSnap = [weak = WeakClaim(this)](double targetOffset, double velocity) -> bool {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->OnScrollSnapCallback(targetOffset, velocity);
    };
    scrollable->SetOnScrollSnapCallback(scrollSnap);

    auto calePredictSnapOffsetCallback =
            [weak = WeakClaim(this)](float delta, float dragDistance, float velocity) -> std::optional<float> {
        auto pattern = weak.Upgrade();
        std::optional<float> predictSnapOffset;
        CHECK_NULL_RETURN(pattern, predictSnapOffset);
        return pattern->CalePredictSnapOffset(delta, dragDistance, velocity);
    };
    scrollable->SetCalePredictSnapOffsetCallback(std::move(calePredictSnapOffsetCallback));

    auto needScrollSnapToSideCallback = [weak = WeakClaim(this)](float delta) -> bool {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->NeedScrollSnapToSide(delta);
    };
    scrollable->SetNeedScrollSnapToSideCallback(std::move(needScrollSnapToSideCallback));

    auto dragFRCSceneCallback = [weak = WeakClaim(this)](double velocity, SceneStatus sceneStatus) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (sceneStatus == NG::SceneStatus::START) {
            pattern->inScrollingStatus_ = true;
            pattern->SetUiDvsyncSwitch(false);
        } else if (sceneStatus == NG::SceneStatus::END) {
            pattern->SetUiDvsyncSwitch(true);
        }
        return pattern->NotifyFRCSceneInfo(SCROLLABLE_DRAG_SCENE, velocity, sceneStatus);
    };
    scrollable->SetDragFRCSceneCallback(std::move(dragFRCSceneCallback));

    scrollable->SetOnContinuousSliding([weak = WeakClaim(this)]() -> double {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, 0.0);
        return pattern->GetMainContentSize();
    });

    scrollableEvent_ = MakeRefPtr<ScrollableEvent>(GetAxis());
    scrollableEvent_->SetScrollable(scrollable);
    gestureHub->AddScrollableEvent(scrollableEvent_);
    InitTouchEvent(gestureHub);
    RegisterWindowStateChangedCallback();
}

void ScrollablePattern::InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    // use TouchEvent to receive next touch down event to stop animation.
    if (touchEvent_) {
        return;
    }
    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern && pattern->scrollableEvent_);
        auto scrollable = pattern->scrollableEvent_->GetScrollable();
        CHECK_NULL_VOID(scrollable);
        switch (info.GetTouches().front().GetTouchType()) {
            case TouchType::DOWN:
                scrollable->HandleTouchDown();
                break;
            case TouchType::UP:
                scrollable->HandleTouchUp();
                break;
            case TouchType::CANCEL:
                scrollable->HandleTouchCancel();
                break;
            default:
                break;
        }
    };
    if (touchEvent_) {
        gestureHub->RemoveTouchEvent(touchEvent_);
    }
    touchEvent_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gestureHub->AddTouchEvent(touchEvent_);
}

void ScrollablePattern::RegisterWindowStateChangedCallback()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    context->AddWindowStateChangedCallback(host->GetId());
}

void ScrollablePattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    context->RemoveWindowStateChangedCallback(frameNode->GetId());
}

void ScrollablePattern::OnWindowHide()
{
    CHECK_NULL_VOID(scrollableEvent_);
    auto scrollable = scrollableEvent_->GetScrollable();
    CHECK_NULL_VOID(scrollable);
    scrollable->StopFrictionAnimation();
}

void ScrollablePattern::SetEdgeEffect(EdgeEffect edgeEffect)
{
    auto gestureHub = GetGestureHub();
    CHECK_NULL_VOID(gestureHub);
    if (scrollEffect_ && (edgeEffect != scrollEffect_->GetEdgeEffect())) {
        gestureHub->RemoveScrollEdgeEffect(scrollEffect_);
        scrollEffect_.Reset();
    }
    if (edgeEffect == EdgeEffect::SPRING && !scrollEffect_) {
        auto springEffect = AceType::MakeRefPtr<ScrollSpringEffect>();
        CHECK_NULL_VOID(springEffect);
        springEffect->SetOutBoundaryCallback([weak = AceType::WeakClaim(this)]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_RETURN(pattern, false);
            return pattern->OutBoundaryCallback();
        });
        // add callback to springEdgeEffect
        SetEdgeEffectCallback(springEffect);
        scrollEffect_ = springEffect;
        gestureHub->AddScrollEdgeEffect(GetAxis(), scrollEffect_);
    }
    CHECK_NULL_VOID(scrollableEvent_);
    auto scrollable = scrollableEvent_->GetScrollable();
    CHECK_NULL_VOID(scrollable);
    scrollable->StopSpringAnimation();
    if (edgeEffect == EdgeEffect::FADE && !scrollEffect_) {
        auto fadeEdgeEffect = AceType::MakeRefPtr<ScrollFadeEffect>(Color::GRAY);
        CHECK_NULL_VOID(fadeEdgeEffect);
        fadeEdgeEffect->SetHandleOverScrollCallback([weakScroll = AceType::WeakClaim(this)]() -> void {
            auto pattern = weakScroll.Upgrade();
            CHECK_NULL_VOID(pattern);
            auto host = pattern->GetHost();
            CHECK_NULL_VOID(host);
            host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        });
        SetEdgeEffectCallback(fadeEdgeEffect);
        fadeEdgeEffect->InitialEdgeEffect();
        scrollEffect_ = fadeEdgeEffect;
        gestureHub->AddScrollEdgeEffect(GetAxis(), scrollEffect_);
    }
    scrollable->SetEdgeEffect(edgeEffect);
}

void ScrollablePattern::HandleFadeEffect(float offset, int32_t source, const SizeF& size,
    bool isNotPositiveScrollableDistance)
{
    auto isScrollFromUpdate = source == SCROLL_FROM_UPDATE;
    scrollEffect_->HandleOverScroll(GetAxis(), IsReverse() ? offset : -offset,
        size, isScrollFromUpdate, isNotPositiveScrollableDistance);
}

bool ScrollablePattern::HandleEdgeEffect(float offset, int32_t source, const SizeF& size)
{
    bool isAtTop = IsAtTop();
    bool isAtBottom = IsAtBottom();
    bool isNotPositiveScrollableDistance = isAtTop && isAtBottom;
    // check edgeEffect is not springEffect
    if (scrollEffect_ && scrollEffect_->IsFadeEffect() &&
        (source == SCROLL_FROM_UPDATE || source == SCROLL_FROM_ANIMATION)) { // handle edge effect
        if ((isAtTop && Positive(offset)) || (isAtBottom && Negative(offset))) {
            HandleFadeEffect(offset, source, size, isNotPositiveScrollableDistance);
        }
    }
    if (!(scrollEffect_ && scrollEffect_->IsSpringEffect() &&
            (source == SCROLL_FROM_UPDATE || source == SCROLL_FROM_ANIMATION ||
                source == SCROLL_FROM_ANIMATION_SPRING ||
                (source == SCROLL_FROM_ANIMATION_CONTROLLER && animateCanOverScroll_)))) {
        if (isAtTop && Positive(offset)) {
            animateOverScroll_ = false;
            return false;
        }
        if (isAtBottom && Negative(offset)) {
            animateOverScroll_ = false;
            return false;
        }
    }
    animateOverScroll_ = (source == SCROLL_FROM_ANIMATION_CONTROLLER) && (isAtTop || isAtBottom);
    isAnimateOverScroll_ = (source == SCROLL_FROM_ANIMATION_CONTROLLER) && animateCanOverScroll_ &&
                            ((isAtTop && Positive(offset)) || (isAtBottom && Negative(offset)));
    return true;
}

void ScrollablePattern::RegisterScrollBarEventTask()
{
    CHECK_NULL_VOID(scrollBar_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    scrollBar_->SetAxis(axis_);
    scrollBar_->SetMarkNeedRenderFunc([weak = AceType::WeakClaim(AceType::RawPtr(host))]() {
        auto host = weak.Upgrade();
        CHECK_NULL_VOID(host);
        host->MarkNeedRenderOnly();
    });
    auto scrollCallback = [weak = WeakClaim(this)](double offset, int32_t source) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->OnScrollCallback(static_cast<float>(offset), source);
    };
    scrollBar_->SetScrollPositionCallback(std::move(scrollCallback));
    auto scrollEnd = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnScrollEnd();
    };
    scrollBar_->SetScrollEndCallback(std::move(scrollEnd));
    auto calePredictSnapOffsetCallback =
            [weak = WeakClaim(this)](float delta, float dragDistance, float velocity) -> std::optional<float> {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, std::optional<float>());
        return pattern->CalePredictSnapOffset(delta, dragDistance, velocity);
    };
    scrollBar_->SetCalePredictSnapOffsetCallback(std::move(calePredictSnapOffsetCallback));
    auto startScrollSnapMotionCallback = [weak = WeakClaim(this)](float scrollSnapDelta, float scrollSnapVelocity) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->StartScrollSnapMotion(scrollSnapDelta, scrollSnapVelocity);
    };
    scrollBar_->SetStartScrollSnapMotionCallback(std::move(startScrollSnapMotionCallback));
    auto scrollPageCallback = [weak = WeakClaim(this)](bool reverse, bool smooth) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->ScrollPage(reverse, smooth);
    };
    scrollBar_->SetScrollPageCallback(std::move(scrollPageCallback));

    auto dragFRCSceneCallback = [weak = WeakClaim(this)](double velocity, SceneStatus sceneStatus) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        return pattern->NotifyFRCSceneInfo(SCROLL_BAR_DRAG_SCENE, velocity, sceneStatus);
    };
    scrollBar_->SetDragFRCSceneCallback(std::move(dragFRCSceneCallback));
    InitScrollBarGestureEvent();
    InitScrollBarMouseEvent();
}

void ScrollablePattern::InitScrollBarGestureEvent()
{
    auto gestureHub = GetGestureHub();
    CHECK_NULL_VOID(gestureHub);
    auto inputHub = GetInputHub();
    CHECK_NULL_VOID(inputHub);
    scrollBar_->SetGestureEvent();
    scrollBar_->SetMouseEvent();
    scrollBar_->SetHoverEvent();
    gestureHub->AddTouchEvent(scrollBar_->GetTouchEvent());
    inputHub->AddOnMouseEvent(scrollBar_->GetMouseEvent());
    inputHub->AddOnHoverEvent(scrollBar_->GetHoverEvent());
    CHECK_NULL_VOID(scrollableEvent_);
    scrollableEvent_->SetInBarRegionCallback(
        [weak = AceType::WeakClaim(AceType::RawPtr(scrollBar_))](const PointF& point, SourceType source) {
            auto scrollBar = weak.Upgrade();
            CHECK_NULL_RETURN(scrollBar, false);
            if (source == SourceType::MOUSE) {
                return scrollBar->InBarHoverRegion(Point(point.GetX(), point.GetY()));
            }
            return scrollBar->InBarTouchRegion(Point(point.GetX(), point.GetY()));
        });
    scrollableEvent_->SetBarCollectTouchTargetCallback(
        [weak = AceType::WeakClaim(AceType::RawPtr(scrollBar_))](const OffsetF& coordinateOffset,
            const GetEventTargetImpl& getEventTargetImpl, TouchTestResult& result, const RefPtr<FrameNode>& frameNode,
            const RefPtr<TargetComponent>& targetComponent, TouchTestResult& responseLinkResult) {
            auto scrollBar = weak.Upgrade();
            CHECK_NULL_VOID(scrollBar);
            scrollBar->OnCollectTouchTarget(
                coordinateOffset, getEventTargetImpl, result, frameNode, targetComponent, responseLinkResult);
        });
    scrollableEvent_->SetBarCollectLongPressTargetCallback(
        [weak = AceType::WeakClaim(AceType::RawPtr(scrollBar_))](const OffsetF& coordinateOffset,
            const GetEventTargetImpl& getEventTargetImpl, TouchTestResult& result, const RefPtr<FrameNode>& frameNode,
            const RefPtr<TargetComponent>& targetComponent, TouchTestResult& responseLinkResult) {
            auto scrollBar = weak.Upgrade();
            CHECK_NULL_VOID(scrollBar);
            scrollBar->OnCollectLongPressTarget(
                coordinateOffset, getEventTargetImpl, result, frameNode, targetComponent, responseLinkResult);
        });
    scrollableEvent_->SetInBarRectRegionCallback(
        [weak = AceType::WeakClaim(AceType::RawPtr(scrollBar_))](const PointF& point, SourceType source) {
            auto scrollBar = weak.Upgrade();
            CHECK_NULL_RETURN(scrollBar, false);
            return scrollBar->InBarRectRegion(Point(point.GetX(), point.GetY()));
        });
}

void ScrollablePattern::SetScrollBar(DisplayMode displayMode)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (displayMode == DisplayMode::OFF) {
        if (scrollBar_) {
            auto gestureHub = GetGestureHub();
            if (gestureHub) {
                gestureHub->RemoveTouchEvent(scrollBar_->GetTouchEvent());
            }
            scrollBar_.Reset();
            if (scrollBarOverlayModifier_) {
                scrollBarOverlayModifier_->SetOpacity(0);
            }
        }
        return;
    }
    DisplayMode oldDisplayMode = DisplayMode::OFF;
    if (!scrollBar_) {
        scrollBar_ = AceType::MakeRefPtr<ScrollBar>();
        RegisterScrollBarEventTask();
    } else {
        oldDisplayMode = scrollBar_->GetDisplayMode();
    }
    // set the scroll bar style
    auto positionMode = GetPositionMode();
    scrollBar_->SetPositionMode(positionMode);
    if (scrollBarOverlayModifier_) {
        scrollBarOverlayModifier_->SetPositionMode(positionMode);
    }

    if (oldDisplayMode != displayMode) {
        scrollBar_->SetDisplayMode(displayMode);
        if (scrollBarOverlayModifier_ && scrollBar_->IsScrollable()) {
            scrollBarOverlayModifier_->SetOpacity(UINT8_MAX);
        }
        scrollBar_->ScheduleDisappearDelayTask();
    }
    UpdateBorderRadius();
}

void ScrollablePattern::UpdateBorderRadius()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    if (renderContext->HasBorderRadius()) {
        auto borderRadius = renderContext->GetBorderRadius().value();
        if (!(borderRadius == scrollBar_->GetHostBorderRadius())) {
            scrollBar_->SetHostBorderRadius(borderRadius);
            scrollBar_->CalcReservedHeight();
        }
    }
}

void ScrollablePattern::SetScrollBar(const std::unique_ptr<ScrollBarProperty>& property)
{
    if (!property) {
        SetScrollBar(DisplayMode::AUTO);
        return;
    }
    auto displayMode = property->GetScrollBarMode().value_or(DisplayMode::AUTO);
    SetScrollBar(displayMode);
    if (scrollBar_) {
        auto barWidth = property->GetScrollBarWidth();
        if (barWidth) {
            scrollBar_->SetInactiveWidth(barWidth.value());
            scrollBar_->SetNormalWidth(barWidth.value());
            scrollBar_->SetActiveWidth(barWidth.value());
            scrollBar_->SetTouchWidth(barWidth.value());
            scrollBar_->SetIsUserNormalWidth(true);
        } else {
            scrollBar_->SetIsUserNormalWidth(false);
        }
        auto barColor = property->GetScrollBarColor();
        if (barColor) {
            scrollBar_->SetForegroundColor(barColor.value());
        } else {
            auto pipelineContext = GetContext();
            CHECK_NULL_VOID(pipelineContext);
            auto theme = pipelineContext->GetTheme<ScrollBarTheme>();
            CHECK_NULL_VOID(theme);
            scrollBar_->SetForegroundColor(theme->GetForegroundColor());
            scrollBar_->SetBackgroundColor(theme->GetBackgroundColor());
        }
    }
}

void ScrollablePattern::UpdateScrollBarRegion(float offset, float estimatedHeight, Size viewPort, Offset viewOffset)
{
    // inner scrollbar, viewOffset is padding offset
    if (scrollBar_) {
        auto mainSize = axis_ == Axis::VERTICAL ? viewPort.Height() : viewPort.Width();
        bool scrollable = GreatNotEqual(estimatedHeight, mainSize) && IsScrollable();
        if (scrollBar_->IsScrollable() != scrollable) {
            scrollBar_->SetScrollable(scrollable);
            if (scrollBarOverlayModifier_) {
                scrollBarOverlayModifier_->SetOpacity(scrollable ? UINT8_MAX : 0);
                scrollBarOverlayModifier_->SetScrollable(scrollable);
            }
            if (scrollable) {
                scrollBar_->ScheduleDisappearDelayTask();
            }
        }
        Offset scrollOffset = { offset, offset }; // fit for w/h switched.
        UpdateBorderRadius();
        scrollBar_->SetReverse(IsReverse());
        scrollBar_->SetIsOutOfBoundary(IsOutOfBoundary());
        scrollBar_->UpdateScrollBarRegion(viewOffset, viewPort, scrollOffset, estimatedHeight);
        scrollBar_->MarkNeedRender();
    }

    // outer scrollbar
    if (scrollBarProxy_) {
        estimatedHeight_ = estimatedHeight - (GetAxis() == Axis::VERTICAL ? viewPort.Height() : viewPort.Width());
        barOffset_ = -offset;
        scrollBarProxy_->NotifyScrollBar(AceType::WeakClaim(this));
    }
}

void ScrollablePattern::SetScrollBarProxy(const RefPtr<ScrollBarProxy>& scrollBarProxy)
{
    CHECK_NULL_VOID(scrollBarProxy);
    auto scrollFunction = [weak = WeakClaim(this)](double offset, int32_t source) {
        if (source != SCROLL_FROM_START) {
            auto pattern = weak.Upgrade();
            if (!pattern || pattern->GetAxis() == Axis::NONE) {
                return false;
            }
            return pattern->UpdateCurrentOffset(offset, source);
        }
        return true;
    };
    auto scrollStartCallback = [weak = WeakClaim(this)](double offset, int32_t source) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        // no source == SCROLL_FROM_START for ScrollBar
        pattern->OnScrollStartCallback();
        return pattern->OnScrollCallback(static_cast<float>(offset), source);
    };
    auto scrollEndCallback = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnScrollEnd();
    };
    auto calePredictSnapOffsetCallback =
            [weak = WeakClaim(this)](float delta, float dragDistance, float velocity) -> std::optional<float> {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, std::optional<float>());
        return pattern->CalePredictSnapOffset(delta, dragDistance, velocity);
    };
    auto startScrollSnapMotionCallback = [weak = WeakClaim(this)](float scrollSnapDelta, float scrollSnapVelocity) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->StartScrollSnapMotion(scrollSnapDelta, scrollSnapVelocity);
    };

    auto scrollbarFRcallback = [weak = WeakClaim(this)](double velocity, SceneStatus sceneStatus) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        return pattern->NotifyFRCSceneInfo(CUSTOM_SCROLL_BAR_SCENE, velocity, sceneStatus);
    };

    auto scrollPageCallback = [weak = WeakClaim(this)](bool reverse, bool smooth) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        return pattern->ScrollPage(reverse, smooth);
    };
    ScrollableNodeInfo nodeInfo = { AceType::WeakClaim(this), std::move(scrollFunction), std::move(scrollStartCallback),
        std::move(scrollEndCallback), std::move(calePredictSnapOffsetCallback),
        std::move(startScrollSnapMotionCallback), std::move(scrollbarFRcallback), std::move(scrollPageCallback) };
    scrollBarProxy->RegisterScrollableNode(nodeInfo);
    scrollBarProxy_ = scrollBarProxy;
}

void ScrollablePattern::CreateScrollBarOverlayModifier()
{
    CHECK_NULL_VOID(scrollBar_ && scrollBar_->NeedPaint());
    CHECK_NULL_VOID(!scrollBarOverlayModifier_);
    scrollBarOverlayModifier_ = AceType::MakeRefPtr<ScrollBarOverlayModifier>();
    scrollBarOverlayModifier_->SetRect(scrollBar_->GetActiveRect());
    scrollBarOverlayModifier_->SetPositionMode(scrollBar_->GetPositionMode());
}

void ScrollablePattern::HandleScrollBarOutBoundary(float scrollBarOutBoundaryExtent)
{
    scrollBarOutBoundaryExtent_ = scrollBarOutBoundaryExtent;
    CHECK_NULL_VOID(scrollBar_ && scrollBar_->NeedScrollBar());
    scrollBar_->SetOutBoundary(std::abs(scrollBarOutBoundaryExtent_));
}

void ScrollablePattern::SetFriction(double friction)
{
    if (LessOrEqual(friction, 0.0)) {
        friction =
            Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_ELEVEN) ? API11_FRICTION : FRICTION;
        friction =
            Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE) ? API12_FRICTION : friction;
    }
    friction_ = friction;
    CHECK_NULL_VOID(scrollableEvent_);
    auto scrollable = scrollableEvent_->GetScrollable();
    scrollable->SetUnstaticFriction(friction_);
}

void ScrollablePattern::SetMaxFlingVelocity(double max)
{
    if (LessOrEqual(max, 0.0f)) {
        max = MAX_VELOCITY;
    }
    maxFlingVelocity_ = max;
    CHECK_NULL_VOID(scrollableEvent_);
    auto scrollable = scrollableEvent_->GetScrollable();
    scrollable->SetMaxFlingVelocity(max);
}

void ScrollablePattern::GetParentNavigation()
{
    if (navBarPattern_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if ((host->GetTag() != V2::LIST_ETS_TAG) && (host->GetTag() != V2::GRID_ETS_TAG) &&
        (host->GetTag() != V2::SCROLL_ETS_TAG)) {
        return;
    }
    for (auto parent = host->GetParent(); parent != nullptr; parent = parent->GetParent()) {
        RefPtr<FrameNode> frameNode = AceType::DynamicCast<FrameNode>(parent);
        if (!frameNode) {
            continue;
        }
        if ((frameNode->GetTag() == V2::LIST_ETS_TAG) || (frameNode->GetTag() == V2::GRID_ETS_TAG) ||
            (frameNode->GetTag() == V2::SCROLL_ETS_TAG)) {
            break;
        }
        navBarPattern_ = frameNode->GetPattern<NavBarPattern>();
        if (!navBarPattern_) {
            continue;
        }
        return;
    }
    navBarPattern_ = nullptr;
    return;
}

void ScrollablePattern::StopAnimate()
{
    if (!IsScrollableStopped()) {
        StopScrollable();
    }
    if (animator_ && !animator_->IsStopped()) {
        animator_->Stop();
    }
    if (!isAnimationStop_) {
        StopAnimation(springAnimation_);
        StopAnimation(curveAnimation_);
    }
}

void ScrollablePattern::ScrollTo(float position)
{
    StopAnimate();
    UpdateCurrentOffset(GetTotalOffset() - position, SCROLL_FROM_JUMP);
}

void ScrollablePattern::AnimateTo(
    float position, float duration, const RefPtr<Curve>& curve, bool smooth, bool canOverScroll, bool useTotalOffset)
{
    float currVelocity = 0.0f;
    if (!IsScrollableStopped()) {
        CHECK_NULL_VOID(scrollableEvent_);
        auto scrollable = scrollableEvent_->GetScrollable();
        CHECK_NULL_VOID(scrollable);
        currVelocity = -scrollable->GetCurrentVelocity();
        scrollAbort_ = true;
        StopScrollable();
    }
    if (!isAnimationStop_) {
        currVelocity = GetCurrentVelocity();
        scrollAbort_ = true;
        StopAnimation(springAnimation_);
        StopAnimation(curveAnimation_);
    }
    if (animator_ && !animator_->IsStopped()) {
        scrollAbort_ = true;
        animator_->Stop();
    }
    if (NearEqual(position, GetTotalOffset())) {
        return;
    }
    finalPosition_ = position;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    AceAsyncTraceBegin(host->GetId(), (SCROLLER_ANIMATION + std::to_string(host->GetAccessibilityId())).c_str());
    if (smooth) {
        if (!useTotalOffset) {
            lastPosition_ = GetTotalOffset();
        }
        PlaySpringAnimation(position, DEFAULT_SCROLL_TO_VELOCITY, DEFAULT_SCROLL_TO_MASS, DEFAULT_SCROLL_TO_STIFFNESS,
            DEFAULT_SCROLL_TO_DAMPING, useTotalOffset);
    } else {
        PlayCurveAnimation(position, duration, curve, canOverScroll);
    }
    FireOnScrollStart();
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->RequestFrame();
}

void ScrollablePattern::PlaySpringAnimation(float position, float velocity, float mass, float stiffness, float damping,
                                            bool useTotalOffset)
{
    if (!springOffsetProperty_) {
        InitSpringOffsetProperty(useTotalOffset);
        CHECK_NULL_VOID(springOffsetProperty_);
    }
    scrollableEvent_->SetAnimateVelocityCallback([weakScroll = AceType::WeakClaim(this)]() -> double {
        auto pattern = weakScroll.Upgrade();
        CHECK_NULL_RETURN(pattern, 0.0f);
        return pattern->GetCurrentVelocity();
    });

    AnimationOption option;
    auto curve = AceType::MakeRefPtr<InterpolatingSpring>(velocity, mass, stiffness, damping);
    InitOption(option, CUSTOM_ANIMATION_DURATION, curve);
    isAnimationStop_ = false;
    AnimationUtils::ExecuteWithoutAnimation([this]() { springOffsetProperty_->Set(GetTotalOffset()); });
    springAnimation_ = AnimationUtils::StartAnimation(
        option,
        [weak = AceType::WeakClaim(this), position]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->SetUiDvsyncSwitch(true);
            pattern->springOffsetProperty_->Set(position);
        },
        [weak = AceType::WeakClaim(this), id = Container::CurrentId(), &useTotalOffset]() {
            ContainerScope scope(id);
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->SetUiDvsyncSwitch(false);
            useTotalOffset = true;
            auto host = pattern->GetHost();
            CHECK_NULL_VOID(host);
            AceAsyncTraceEnd(host->GetId(), (SCROLLER_ANIMATION + std::to_string(host->GetAccessibilityId())).c_str());
            pattern->NotifyFRCSceneInfo(SCROLLABLE_MULTI_TASK_SCENE, pattern->GetCurrentVelocity(),
                SceneStatus::END);
            pattern->SetScrollEdgeType(ScrollEdgeType::SCROLL_NONE);
    });
    NotifyFRCSceneInfo(SCROLLABLE_MULTI_TASK_SCENE, GetCurrentVelocity(), SceneStatus::START);
}

void ScrollablePattern::PlayCurveAnimation(
    float position, float duration, const RefPtr<Curve>& curve, bool canOverScroll)
{
    AnimationOption option;
    InitOption(option, duration, curve);
    if (!curveOffsetProperty_) {
        InitCurveOffsetProperty();
        CHECK_NULL_VOID(curveOffsetProperty_);
    }
    scrollableEvent_->SetAnimateVelocityCallback([weakScroll = AceType::WeakClaim(this)]() -> double {
        auto pattern = weakScroll.Upgrade();
        CHECK_NULL_RETURN(pattern, 0.0f);
        return pattern->GetCurrentVelocity();
    });
    isAnimationStop_ = false;
    SetAnimateCanOverScroll(canOverScroll);
    curveOffsetProperty_->Set(GetTotalOffset());
    curveAnimation_ = AnimationUtils::StartAnimation(
        option,
        [weak = AceType::WeakClaim(this), position]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->SetUiDvsyncSwitch(true);
            pattern->curveOffsetProperty_->Set(position);
        },
        [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            auto host = pattern->GetHost();
            CHECK_NULL_VOID(host);
            pattern->SetUiDvsyncSwitch(false);
            AceAsyncTraceEnd(host->GetId(), (SCROLLER_ANIMATION + std::to_string(host->GetAccessibilityId())).c_str());
            pattern->NotifyFRCSceneInfo(SCROLLABLE_MULTI_TASK_SCENE, pattern->GetCurrentVelocity(), SceneStatus::END);
        });
    NotifyFRCSceneInfo(SCROLLABLE_MULTI_TASK_SCENE, GetCurrentVelocity(), SceneStatus::START);
}

void ScrollablePattern::InitSpringOffsetProperty(bool useTotalOffset)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto propertyCallback = [weak = AceType::WeakClaim(this), useTotalOffset](float offset) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (pattern->isAnimationStop_) {
            return;
        }
        auto context = OHOS::Ace::PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        uint64_t currentVsync = context->GetVsyncTime();
        uint64_t diff = currentVsync - pattern->lastVsyncTime_;
        if (diff < MAX_VSYNC_DIFF_TIME && diff > MIN_DIFF_VSYNC) {
            pattern->currentVelocity_ = (offset - pattern->lastPosition_) / diff * MILLOS_PER_NANO_SECONDS;
            pattern->NotifyFRCSceneInfo(SCROLLABLE_MULTI_TASK_SCENE, pattern->currentVelocity_,
                SceneStatus::RUNNING);
        }
        auto stopAnimation = NearEqual(pattern->finalPosition_, offset, SPRING_ACCURACY);
        if (stopAnimation) {
            offset = pattern->finalPosition_;
        }
        auto delta = useTotalOffset ? pattern->GetTotalOffset() - offset : pattern->lastPosition_ - offset;
        pattern->lastVsyncTime_ = currentVsync;
        pattern->lastPosition_ = offset;
        if (!pattern->UpdateCurrentOffset(delta,
            SCROLL_FROM_ANIMATION_CONTROLLER) || stopAnimation) {
            pattern->StopAnimation(pattern->springAnimation_);
        }
    };
    springOffsetProperty_ = AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(0.0, std::move(propertyCallback));
    renderContext->AttachNodeAnimatableProperty(springOffsetProperty_);
}

void ScrollablePattern::InitCurveOffsetProperty()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto propertyCallback = [weak = AceType::WeakClaim(this)](float offset) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (pattern->isAnimationStop_) {
            return;
        }
        auto context = OHOS::Ace::PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        uint64_t currentVsync = context->GetVsyncTime();
        uint64_t diff = currentVsync - pattern->lastVsyncTime_;
        if (diff < MAX_VSYNC_DIFF_TIME && diff > MIN_DIFF_VSYNC) {
            pattern->currentVelocity_ = (offset - pattern->lastPosition_) / diff * MILLOS_PER_NANO_SECONDS;
            pattern->NotifyFRCSceneInfo(SCROLLABLE_MULTI_TASK_SCENE, pattern->currentVelocity_,
                SceneStatus::RUNNING);
        }
        auto stopAnimation = NearEqual(pattern->finalPosition_, offset, SPRING_ACCURACY);
        if (stopAnimation) {
            offset = pattern->finalPosition_;
        }
        pattern->lastVsyncTime_ = currentVsync;
        pattern->lastPosition_ = offset;
        pattern->NotifyFRCSceneInfo(SCROLLABLE_MULTI_TASK_SCENE, pattern->GetCurrentVelocity(),
            SceneStatus::RUNNING);
        if (!pattern->UpdateCurrentOffset(pattern->GetTotalOffset() - offset, SCROLL_FROM_ANIMATION_CONTROLLER) ||
            stopAnimation || pattern->isAnimateOverScroll_) {
            if (pattern->isAnimateOverScroll_) {
                pattern->isAnimateOverScroll_ = false;
                auto pauseVelocity = -pattern->currentVelocity_;
                auto context = OHOS::Ace::NG::PipelineContext::GetCurrentContext();
                CHECK_NULL_VOID(context);
                context->MarkNeedFlushAnimationStartTime();
                pattern->PauseAnimation(pattern->curveAnimation_);
                pattern->HandleOverScroll(pauseVelocity);
            } else if (stopAnimation ||
                       (pattern->IsAtTop() && LessOrEqual(pattern->finalPosition_, pattern->GetTotalOffset())) ||
                       (pattern->IsAtBottom() && GreatOrEqual(pattern->finalPosition_, pattern->GetTotalOffset()))) {
                pattern->StopAnimation(pattern->curveAnimation_);
            }
        }
    };
    curveOffsetProperty_ = AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(0.0, std::move(propertyCallback));
    renderContext->AttachNodeAnimatableProperty(curveOffsetProperty_);
}

void ScrollablePattern::InitOption(AnimationOption &option, float duration, const RefPtr<Curve>& curve)
{
    if (!curve) {
        option.SetCurve(Curves::EASE); // default curve
    } else {
        option.SetCurve(curve);
    }
    if (duration > 0) {
        option.SetDuration(duration);
    } else {
        option.SetDuration(CUSTOM_ANIMATION_DURATION);
    }
}

void ScrollablePattern::StopAnimation(std::shared_ptr<AnimationUtils::Animation> animation)
{
    SetAnimateCanOverScroll(false);
    isAnimationStop_ = true;
    currentVelocity_ = 0.0;
    if (!animation) {
        return;
    }
    AnimationUtils::StopAnimation(animation);
    OnAnimateStop();
}

void ScrollablePattern::PauseAnimation(std::shared_ptr<AnimationUtils::Animation> animation)
{
    SetAnimateCanOverScroll(false);
    isAnimationStop_ = true;
    currentVelocity_ = 0.0;
    if (!animation) {
        return;
    }
    AnimationUtils::StopAnimation(animation);
}

void ScrollablePattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToBounds(true);
    host->GetRenderContext()->UpdateClipEdge(true);
}

void ScrollablePattern::UninitMouseEvent()
{
    if (!boxSelectPanEvent_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureHub = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->RemovePanEvent(boxSelectPanEvent_);
    boxSelectPanEvent_.Reset();
    ClearMultiSelect();
    ClearInvisibleItemsSelectedStatus();
    isMouseEventInit_ = false;
}

void ScrollablePattern::InitMouseEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureHub = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    if (!boxSelectPanEvent_) {
        auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& info) {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->HandleDragStart(info);
        };

        auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->HandleDragUpdate(info);
        };

        auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& info) {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->HandleDragEnd(info);
        };
        GestureEventNoParameter actionCancelTask;
        boxSelectPanEvent_ = MakeRefPtr<PanEvent>(std::move(actionStartTask), std::move(actionUpdateTask),
            std::move(actionEndTask), std::move(actionCancelTask));
    }
    PanDirection panDirection = { .type = PanDirection::ALL };
    gestureHub->AddPanEvent(boxSelectPanEvent_, panDirection, 1, DEFAULT_PAN_DISTANCE);
    gestureHub->SetPanEventType(GestureTypeName::BOXSELECT);
    gestureHub->SetOnGestureJudgeNativeBegin([](const RefPtr<NG::GestureInfo>& gestureInfo,
                                                 const std::shared_ptr<BaseGestureEvent>& info) -> GestureJudgeResult {
        if (gestureInfo->GetType() == GestureTypeName::BOXSELECT &&
            gestureInfo->GetInputEventType() != InputEventType::MOUSE_BUTTON) {
            return GestureJudgeResult::REJECT;
        }
        return GestureJudgeResult::CONTINUE;
    });
    isMouseEventInit_ = true;
}

void ScrollablePattern::HandleDragStart(const GestureEvent& info)
{
    TAG_LOGI(AceLogTag::ACE_SCROLLABLE, "Box select start");
    auto mouseOffsetX = static_cast<float>(info.GetRawGlobalLocation().GetX());
    auto mouseOffsetY = static_cast<float>(info.GetRawGlobalLocation().GetY());
    SuggestOpIncGroup(true);
    if (!IsItemSelected(info)) {
        ClearMultiSelect();
        ClearInvisibleItemsSelectedStatus();
        mouseStartOffset_ = OffsetF(mouseOffsetX, mouseOffsetY);
        lastMouseStart_ = mouseStartOffset_;
        mouseEndOffset_ = OffsetF(mouseOffsetX, mouseOffsetY);
        mousePressOffset_ = OffsetF(mouseOffsetX, mouseOffsetY);
        totalOffsetOfMousePressed_ = mousePressOffset_.GetMainOffset(axis_) + GetTotalOffset();
        canMultiSelect_ = true;
    }
    mousePressed_ = true;
}

void ScrollablePattern::HandleDragUpdate(const GestureEvent& info)
{
    auto mouseOffsetX = static_cast<float>(info.GetRawGlobalLocation().GetX());
    auto mouseOffsetY = static_cast<float>(info.GetRawGlobalLocation().GetY());
    if (!mousePressed_ || !canMultiSelect_) {
        return;
    }
    if (info.GetInputEventType() != InputEventType::MOUSE_BUTTON) {
        HandleDragEnd(info);
        return;
    }
    lastMouseMove_ = info;
    auto delta = OffsetF(mouseOffsetX, mouseOffsetY) - mousePressOffset_;
    if (Offset(delta.GetX(), delta.GetY()).GetDistance() > DEFAULT_PAN_DISTANCE.ConvertToPx()) {
        mouseEndOffset_ = OffsetF(mouseOffsetX, mouseOffsetY);
        // avoid large select zone
        LimitMouseEndOffset();
        auto selectedZone = ComputeSelectedZone(mouseStartOffset_, mouseEndOffset_);
        MultiSelectWithoutKeyboard(selectedZone);
        HandleInvisibleItemsSelectedStatus(selectedZone);
    }
    SelectWithScroll();
}

void ScrollablePattern::HandleDragEnd(const GestureEvent& info)
{
    TAG_LOGI(AceLogTag::ACE_SCROLLABLE, "Box select end");
    mouseStartOffset_.Reset();
    lastMouseStart_.Reset();
    mouseEndOffset_.Reset();
    mousePressed_ = false;
    canMultiSelect_ = false;
    ClearSelectedZone();
    itemToBeSelected_.clear();
    lastMouseMove_.SetLocalLocation(Offset::Zero());
}
void ScrollablePattern::ClearInvisibleItemsSelectedStatus()
{
    for (auto& item : itemToBeSelected_) {
        item.second.FireSelectChangeEvent(false);
    }
    itemToBeSelected_.clear();
}

void ScrollablePattern::HandleInvisibleItemsSelectedStatus(const RectF& selectedZone)
{
    auto newRect = selectedZone;
    auto startMainOffset = mouseStartOffset_.GetMainOffset(axis_);
    auto endMainOffset = mouseEndOffset_.GetMainOffset(axis_);
    SelectDirection oldDirection = selectDirection_;
    if (LessNotEqual(startMainOffset, endMainOffset)) {
        selectDirection_ = SELECT_DOWN;
        if (axis_ == Axis::VERTICAL) {
            newRect.SetOffset(OffsetF(selectedZone.Left(), totalOffsetOfMousePressed_));
        } else {
            newRect.SetOffset(OffsetF(totalOffsetOfMousePressed_, selectedZone.Top()));
        }
    } else {
        selectDirection_ = SELECT_UP;
        if (axis_ == Axis::VERTICAL) {
            newRect.SetOffset(
                OffsetF(selectedZone.Left(), totalOffsetOfMousePressed_ - (startMainOffset - endMainOffset)));
        } else {
            newRect.SetOffset(
                OffsetF(totalOffsetOfMousePressed_ - (startMainOffset - endMainOffset), selectedZone.Top()));
        }
    }
    oldDirection = oldDirection == SELECT_NONE ? selectDirection_ : oldDirection;

    for (auto& item : itemToBeSelected_) {
        item.second.FireSelectChangeEvent(newRect.IsIntersectWith(item.second.rect));
    }

    if (oldDirection != selectDirection_) {
        itemToBeSelected_.clear();
    }
}

void ScrollablePattern::SelectWithScroll()
{
    if (!IsScrollable()) {
        return;
    }
    auto offset = GetOutOfScrollableOffset();
    if (NearZero(offset)) {
        return;
    }

    if (AnimateRunning()) {
        return;
    }

    if (!isAnimationStop_) {
        StopAnimation(springAnimation_);
        StopAnimation(curveAnimation_);
    }

    if (!animator_) {
        animator_ = CREATE_ANIMATOR(PipelineBase::GetCurrentContext());
        animator_->AddStopListener([weak = AceType::WeakClaim(this)]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->OnAnimateStop();
        });
    } else if (!animator_->IsStopped()) {
        scrollAbort_ = true;
        animator_->Stop();
    }

    if (!selectMotion_) {
        selectMotion_ = AceType::MakeRefPtr<SelectMotion>(offset, [weak = WeakClaim(this)]() -> bool {
            auto pattern = weak.Upgrade();
            CHECK_NULL_RETURN(pattern, true);
            return pattern->ShouldSelectScrollBeStopped();
        });
        selectMotion_->AddListener([weakScroll = AceType::WeakClaim(this)](double offset) {
            auto pattern = weakScroll.Upgrade();
            CHECK_NULL_VOID(pattern);
            offset = pattern->GetOffsetWithLimit(offset);
            pattern->UpdateCurrentOffset(offset, SCROLL_FROM_AXIS);
            pattern->UpdateMouseStart(offset);
        });
    } else {
        selectMotion_->Reset(offset);
    }

    animator_->PlayMotion(selectMotion_);

    FireOnScrollStart();
}

void ScrollablePattern::ClearSelectedZone()
{
    DrawSelectedZone(RectF());
}

RectF ScrollablePattern::ComputeSelectedZone(const OffsetF& startOffset, const OffsetF& endOffset)
{
    RectF selectedZone;
    if (startOffset.GetX() <= endOffset.GetX()) {
        if (startOffset.GetY() <= endOffset.GetY()) {
            // bottom right
            selectedZone = RectF(startOffset.GetX(), startOffset.GetY(), endOffset.GetX() - startOffset.GetX(),
                endOffset.GetY() - startOffset.GetY());
        } else {
            // top right
            selectedZone = RectF(startOffset.GetX(), endOffset.GetY(), endOffset.GetX() - startOffset.GetX(),
                startOffset.GetY() - endOffset.GetY());
        }
    } else {
        if (startOffset.GetY() <= endOffset.GetY()) {
            // bottom left
            selectedZone = RectF(endOffset.GetX(), startOffset.GetY(), startOffset.GetX() - endOffset.GetX(),
                endOffset.GetY() - startOffset.GetY());
        } else {
            // top left
            selectedZone = RectF(endOffset.GetX(), endOffset.GetY(), startOffset.GetX() - endOffset.GetX(),
                startOffset.GetY() - endOffset.GetY());
        }
    }

    return selectedZone;
}

void ScrollablePattern::DrawSelectedZone(const RectF& selectedZone)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hostContext = host->GetRenderContext();
    CHECK_NULL_VOID(hostContext);
    hostContext->UpdateMouseSelectWithRect(selectedZone, SELECT_FILL_COLOR, SELECT_STROKE_COLOR);
}

void ScrollablePattern::MarkSelectedItems()
{
    if (multiSelectable_ && mousePressed_) {
        auto selectedZone = ComputeSelectedZone(mouseStartOffset_, mouseEndOffset_);
        if (!selectedZone.IsEmpty()) {
            MultiSelectWithoutKeyboard(selectedZone);
            HandleInvisibleItemsSelectedStatus(selectedZone);
        }
    }
}

bool ScrollablePattern::ShouldSelectScrollBeStopped()
{
    if (!mousePressed_) {
        return true;
    }
    auto offset = GetOutOfScrollableOffset();
    if (NearZero(offset)) {
        return true;
    }

    if (selectMotion_) {
        selectMotion_->Reset(offset);
    }
    return false;
};

void ScrollablePattern::UpdateMouseStart(float offset)
{
    if (axis_ == Axis::VERTICAL) {
        mouseStartOffset_.AddY(offset);
    } else {
        mouseStartOffset_.AddX(offset);
    }
}

float ScrollablePattern::GetOutOfScrollableOffset() const
{
    auto offset = 0.0f;
    auto mouseMainOffset = static_cast<float>(
        axis_ == Axis::VERTICAL ? lastMouseMove_.GetLocalLocation().GetY() : lastMouseMove_.GetLocalLocation().GetX());
    auto hostSize = GetHostFrameSize();
    CHECK_NULL_RETURN(hostSize.has_value(), offset);
    auto mainTop = 0.0f;
    auto mainBottom = hostSize->MainSize(axis_);
    if (GreatOrEqual(mouseMainOffset, mainTop) && LessOrEqual(mouseMainOffset, mainBottom)) {
        return offset;
    }
    if (GreatNotEqual(mouseMainOffset, mainBottom)) {
        if (IsAtBottom()) {
            return offset;
        }
        offset = mainBottom - mouseMainOffset;
    }
    if (LessNotEqual(mouseMainOffset, mainTop)) {
        if (IsAtTop()) {
            return offset;
        }
        offset = mainTop - mouseMainOffset;
    }
    return offset;
}

// avoid start position move when offset is bigger then item height
float ScrollablePattern::GetOffsetWithLimit(float offset) const
{
    if (Positive(offset)) {
        auto totalOffset = GetTotalOffset();
        return std::min(totalOffset, offset);
    } else if (Negative(offset)) {
        auto frameNode = GetHost();
        CHECK_NULL_RETURN(frameNode, true);
        auto hostSize = frameNode->GetGeometryNode()->GetFrameSize();
        auto remainHeight = GetTotalHeight() - GetTotalOffset() - hostSize.MainSize(axis_);
        return std::max(offset, -remainHeight);
    }
    return 0;
}

void ScrollablePattern::LimitMouseEndOffset()
{
    float limitedMainOffset = -1.0f;
    float limitedCrossOffset = -1.0f;
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto hostSize = frameNode->GetGeometryNode()->GetFrameSize();
    auto mainSize = hostSize.MainSize(axis_);
    auto crossSize = hostSize.CrossSize(axis_);
    auto mainOffset = mouseEndOffset_.GetMainOffset(axis_);
    auto crossOffset = mouseEndOffset_.GetCrossOffset(axis_);
    if (LessNotEqual(mainOffset, 0.0f)) {
        limitedMainOffset = 0.0f;
    }
    if (GreatNotEqual(mainOffset, mainSize)) {
        limitedMainOffset = mainSize;
    }
    if (LessNotEqual(crossOffset, 0.0f)) {
        limitedCrossOffset = 0.0f;
    }
    if (GreatNotEqual(crossOffset, crossSize)) {
        limitedCrossOffset = crossSize;
    }

    if (axis_ == Axis::VERTICAL) {
        mouseEndOffset_.SetX(LessNotEqual(limitedCrossOffset, 0.0f) ? mouseEndOffset_.GetX() : limitedCrossOffset);
        mouseEndOffset_.SetY(LessNotEqual(limitedMainOffset, 0.0f) ? mouseEndOffset_.GetY() : limitedMainOffset);
    } else {
        mouseEndOffset_.SetX(LessNotEqual(limitedMainOffset, 0.0f) ? mouseEndOffset_.GetX() : limitedMainOffset);
        mouseEndOffset_.SetY(LessNotEqual(limitedCrossOffset, 0.0f) ? mouseEndOffset_.GetY() : limitedCrossOffset);
    }
}

bool ScrollablePattern::HandleScrollImpl(float offset, int32_t source)
{
    // Previous: Set HandleScrollImpl to Scrollable->callback_
    // Scrollable::HandleScroll calls callback_ through UpdateScrollPosition

    // Now: HandleScroll moved to ScrollablePattern, directly call HandleScrollImpl in
    // ScrollablePattern::HandleScroll
    double overOffset = offset;
    if (!OnScrollPosition(overOffset, source)) {
        return false;
    }
    auto result = OnScrollCallback(overOffset, source);
    SelectOverlayScrollNotifier::NotifyOnScrollCallback(WeakClaim(this), overOffset, source);
    return result;
}

void ScrollablePattern::NotifyMoved(bool value)
{
    CHECK_NULL_VOID(scrollableEvent_);
    auto&& scroll = scrollableEvent_->GetScrollable();
    if (scroll) {
        scroll->SetMoved(value);
    }
}

void ScrollablePattern::ProcessSpringEffect(float velocity)
{
    CHECK_NULL_VOID(InstanceOf<ScrollSpringEffect>(scrollEffect_));
    if (!OutBoundaryCallback() && !GetCanOverScroll()) {
        OnScrollEnd();
        return;
    }
    CHECK_NULL_VOID(scrollableEvent_);
    auto scrollable = scrollableEvent_->GetScrollable();
    // HandleTouchUp may be triggered before HandleDragEnd when scrollable nested scrollable,
    // so need to update spring motion.
    if (scrollable && scrollable->IsSpringMotionRunning()) {
        scrollEffect_->ProcessSpringUpdate();
    } else {
        scrollEffect_->ProcessScrollOver(velocity);
    }
}

void ScrollablePattern::SetCanOverScroll(bool val)
{
    CHECK_NULL_VOID(scrollableEvent_);
    auto&& scrollable = scrollableEvent_->GetScrollable();
    if (scrollable) {
        scrollable->SetCanOverScroll(val);
    }
}

bool ScrollablePattern::GetCanOverScroll() const
{
    CHECK_NULL_RETURN(scrollableEvent_, true);
    auto&& scrollable = scrollableEvent_->GetScrollable();
    if (scrollable) {
        return scrollable->CanOverScroll();
    }
    return true;
}

EdgeEffect ScrollablePattern::GetEdgeEffect() const
{
    return edgeEffect_;
}

ScrollState ScrollablePattern::GetScrollState() const
{
    return ScrollablePattern::GetScrollState(scrollSource_);
}

ScrollState ScrollablePattern::GetScrollState(int32_t scrollSource)
{
    // with event
    if (scrollSource == SCROLL_FROM_UPDATE || scrollSource == SCROLL_FROM_AXIS || scrollSource == SCROLL_FROM_BAR) {
        return ScrollState::SCROLL;
    }
    // without event
    if (scrollSource == SCROLL_FROM_ANIMATION || scrollSource == SCROLL_FROM_ANIMATION_SPRING ||
        scrollSource == SCROLL_FROM_ANIMATION_CONTROLLER || scrollSource == SCROLL_FROM_BAR_FLING) {
        return ScrollState::FLING;
    }
    // SCROLL_FROM_NONE, SCROLL_FROM_JUMP, SCROLL_FROM_CHILD, SCROLL_FROM_FOCUS_JUMP, SCROLL_FROM_ROTATE,
    // SCROLL_FROM_INDEXER, SCROLL_FROM_START
    return ScrollState::IDLE;
}

ScrollSource ScrollablePattern::ConvertScrollSource(int32_t source)
{
    // static linear map must be sorted by key.
    static const LinearEnumMapNode<int32_t, ScrollSource> scrollSourceMap[] = {
        { SCROLL_FROM_UPDATE, ScrollSource::DRAG },
        { SCROLL_FROM_ANIMATION, ScrollSource::FLING },
        { SCROLL_FROM_JUMP, ScrollSource::SCROLLER },
        { SCROLL_FROM_ANIMATION_SPRING, ScrollSource::EDGE_EFFECT },
        { SCROLL_FROM_BAR, ScrollSource::SCROLL_BAR },
        { SCROLL_FROM_FOCUS_JUMP, ScrollSource::OTHER_USER_INPUT },
        { SCROLL_FROM_AXIS, ScrollSource::OTHER_USER_INPUT },
        { SCROLL_FROM_ANIMATION_CONTROLLER, ScrollSource::SCROLLER_ANIMATION },
        { SCROLL_FROM_BAR_FLING, ScrollSource::SCROLL_BAR_FLING },
    };
    ScrollSource sourceType = ScrollSource::OTHER_USER_INPUT;
    int64_t idx = BinarySearchFindIndex(scrollSourceMap, ArraySize(scrollSourceMap), source);
    if (idx >= 0) {
        sourceType = scrollSourceMap[idx].value;
    }
    return sourceType;
}

ScrollResult ScrollablePattern::HandleScrollParentFirst(float& offset, int32_t source, NestedState state)
{
    auto parent = GetNestedScrollParent();
    ScrollState scrollState = source == SCROLL_FROM_ANIMATION ? ScrollState::FLING : ScrollState::SCROLL;
    if (state == NestedState::CHILD_OVER_SCROLL) {
        if (GetEdgeEffect() == EdgeEffect::NONE) {
            return parent->HandleScroll(offset, source, NestedState::CHILD_OVER_SCROLL, GetVelocity());
        }
        ExecuteScrollFrameBegin(offset, scrollState);
        return { 0, true };
    }
    auto result = parent->HandleScroll(offset, source, NestedState::CHILD_SCROLL, GetVelocity());
    offset = result.remain;
    if (NearZero(offset)) {
        SetCanOverScroll(!InstanceOf<ScrollablePattern>(parent));
        return { 0, false };
    }
    float allOffset = offset;
    ExecuteScrollFrameBegin(offset, scrollState);
    auto remainOffset = std::abs(offset) < std::abs(allOffset) ? allOffset - offset : 0;
    auto overOffsets = GetOverScrollOffset(offset);
    auto overOffset = offset > 0 ? overOffsets.start : overOffsets.end;
    remainOffset += overOffset;
    if (NearZero(remainOffset)) {
        SetCanOverScroll(false);
        return { 0, false };
    }
    if (state == NestedState::CHILD_SCROLL) {
        offset -= overOffset;
        SetCanOverScroll(false);
        return { remainOffset, !NearZero(overOffset) };
    }
    if (GetEdgeEffect() == EdgeEffect::NONE) {
        result = parent->HandleScroll(remainOffset, source, NestedState::CHILD_OVER_SCROLL, GetVelocity());
        if (NearZero(result.remain)) {
            offset -= overOffset;
        }
    }
    SetCanOverScroll(!NearZero(overOffset) || (NearZero(offset) && result.reachEdge));
    return { 0, GetCanOverScroll() };
}

ScrollResult ScrollablePattern::HandleScrollSelfFirst(float& offset, int32_t source, NestedState state)
{
    auto parent = GetNestedScrollParent();
    ScrollState scrollState = source == SCROLL_FROM_ANIMATION ? ScrollState::FLING : ScrollState::SCROLL;
    if (state == NestedState::CHILD_OVER_SCROLL) {
        auto result = parent->HandleScroll(offset, source, NestedState::CHILD_OVER_SCROLL, GetVelocity());
        if (NearZero(result.remain)) {
            offset = 0;
            return result;
        }
        ExecuteScrollFrameBegin(offset, scrollState);
        if (GetEdgeEffect() == EdgeEffect::NONE) {
            return result;
        }
        return { 0, true };
    }
    float allOffset = offset;
    ExecuteScrollFrameBegin(offset, scrollState);
    auto remainOffset = std::abs(offset) < std::abs(allOffset) ? allOffset - offset : 0;
    auto overOffsets = GetOverScrollOffset(offset);
    auto overOffset = offset > 0 ? overOffsets.start : overOffsets.end;
    if (NearZero(overOffset) && NearZero(remainOffset)) {
        SetCanOverScroll(false);
        return { 0, false };
    }
    offset -= overOffset;
    auto result = parent->HandleScroll(overOffset + remainOffset, source, NestedState::CHILD_SCROLL, GetVelocity());
    if (NearZero(result.remain)) {
        SetCanOverScroll(!InstanceOf<ScrollablePattern>(parent));
        return { 0, false };
    }
    if (state == NestedState::CHILD_SCROLL) {
        SetCanOverScroll(false);
        return result;
    }
    // triggering overScroll, parent always handle it first
    auto overRes = parent->HandleScroll(result.remain, source, NestedState::CHILD_OVER_SCROLL, GetVelocity());
    offset += LessNotEqual(std::abs(overOffset), std::abs(result.remain)) ? overOffset : overRes.remain;
    SetCanOverScroll((!NearZero(overOffset) || NearZero(offset)) && overRes.reachEdge);
    return { 0, GetCanOverScroll() };
}

ScrollResult ScrollablePattern::HandleScrollSelfOnly(float& offset, int32_t source, NestedState state)
{
    float allOffset = offset;
    ScrollState scrollState = source == SCROLL_FROM_ANIMATION ? ScrollState::FLING : ScrollState::SCROLL;
    ExecuteScrollFrameBegin(offset, scrollState);
    auto remainOffset = allOffset - offset;
    auto overOffsets = GetOverScrollOffset(offset);
    auto overOffset = (offset > 0) ? overOffsets.start : overOffsets.end;
    remainOffset += overOffset;
    if (NearZero(remainOffset)) {
        SetCanOverScroll(false);
        return { 0, false };
    }
    bool canOverScroll = false;
    if (state == NestedState::CHILD_SCROLL) {
        offset -= overOffset;
    } else if (state == NestedState::GESTURE) {
        canOverScroll = !NearZero(overOffset) && GetEdgeEffect() != EdgeEffect::NONE;
    } else if (GetEdgeEffect() != EdgeEffect::NONE) {
        remainOffset = 0;
    }
    SetCanOverScroll(canOverScroll);
    return { remainOffset, !NearZero(overOffset) };
}

ScrollResult ScrollablePattern::HandleScrollParallel(float& offset, int32_t source, NestedState state)
{
    auto remainOffset = 0.0;
    auto parent = GetNestedScrollParent();
    ScrollState scrollState = source == SCROLL_FROM_ANIMATION ? ScrollState::FLING : ScrollState::SCROLL;
    if (state == NestedState::CHILD_OVER_SCROLL) {
        if (GetEdgeEffect() == EdgeEffect::NONE) {
            auto result = parent->HandleScroll(offset, source, NestedState::CHILD_OVER_SCROLL, GetVelocity());
            remainOffset = result.remain;
            offset = 0;
        } else {
            ExecuteScrollFrameBegin(offset, scrollState);
        }
        return { remainOffset, true };
    }

    bool canOverScroll = false;
    float parentOffset = offset;
    ExecuteScrollFrameBegin(offset, scrollState);
    auto result = parent->HandleScroll(parentOffset, source, NestedState::CHILD_SCROLL, GetVelocity());

    auto overOffsets = GetOverScrollOffset(offset);
    auto overOffset = offset > 0 ? overOffsets.start : overOffsets.end;
    if (!NearZero(overOffset) && result.reachEdge) {
        if (state == NestedState::CHILD_SCROLL) {
            remainOffset = overOffset;
            offset = offset - overOffset;
        } else if (GetEdgeEffect() == EdgeEffect::NONE) {
            parent->HandleScroll(result.remain, source, NestedState::CHILD_OVER_SCROLL, GetVelocity());
            canOverScroll = true;
            offset = offset - overOffset;
        } else {
            canOverScroll = true;
        }
    } else if (!NearZero(overOffset)) {
        offset = offset - overOffset;
    }
    SetCanOverScroll(canOverScroll);
    return { remainOffset, !NearZero(overOffset) && result.reachEdge };
}

ScrollResult ScrollablePattern::HandleScroll(float offset, int32_t source, NestedState state, float velocity)
{
    ScrollResult result = { 0, false };
    auto host = GetHost();
    CHECK_NULL_RETURN(host, result);
    auto nestedScroll = GetNestedScroll();
    auto parent = GetNestedScrollParent();
    auto overOffsets = GetOverScrollOffset(offset);
    auto initOffset = offset;
    float backOverOffset = (offset > 0) ? overOffsets.end : overOffsets.start;
    if (NearZero(offset) || !NearZero(backOverOffset)) {
        if (!NearEqual(offset, backOverOffset)) {
            SetCanOverScroll(false);
        }
        ScrollState scrollState = source == SCROLL_FROM_ANIMATION ? ScrollState::FLING : ScrollState::SCROLL;
        ExecuteScrollFrameBegin(offset, scrollState);
    } else if (parent && !IsScrollSnap() &&
               ((offset < 0 && nestedScroll.forward == NestedScrollMode::PARENT_FIRST) ||
                   (offset > 0 && nestedScroll.backward == NestedScrollMode::PARENT_FIRST))) {
        result = HandleScrollParentFirst(offset, source, state);
    } else if (parent && ((offset < 0 && nestedScroll.forward == NestedScrollMode::SELF_FIRST) ||
                             (offset > 0 && nestedScroll.backward == NestedScrollMode::SELF_FIRST))) {
        result = HandleScrollSelfFirst(offset, source, state);
    } else if (parent && ((offset < 0 && nestedScroll.forward == NestedScrollMode::PARALLEL) ||
                             (offset > 0 && nestedScroll.backward == NestedScrollMode::PARALLEL))) {
        result = HandleScrollParallel(offset, source, state);
    } else {
        result = HandleScrollSelfOnly(offset, source, state);
    }
    ACE_SCOPED_TRACE("HandleScroll, initOffset:%f, processedOffset:%f, overOffsets.start:%f, overOffsets.end:%f, "
                     "source:%d, nestedState:%d, canOverScroll:%u, id:%d, tag:%s",
        initOffset, offset, overOffsets.start, overOffsets.end, source, state, GetCanOverScroll(),
        static_cast<int32_t>(host->GetAccessibilityId()), host->GetTag().c_str());
    bool moved = HandleScrollImpl(offset, source);
    NotifyMoved(moved);
    return result;
}

bool ScrollablePattern::HandleScrollVelocity(float velocity)
{
    // if scrollable try to over scroll when it is at the boundary,
    // scrollable does not start fling animation.
    auto edgeEffect = GetEdgeEffect();
    auto needFlingAtEdge = !(((IsAtTop() && Positive(velocity)) || (IsAtBottom() && Negative(velocity))));
    auto isOutOfBoundary = OutBoundaryCallback();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    ACE_SCOPED_TRACE("HandleScrollVelocity, IsOutOfBoundary:%u, needFlingAtEdge:%u, edgeEffect:%d, IsAtTop:%u, "
                     "IsAtBottom:%u, velocity:%f, id:%d, tag:%s",
        isOutOfBoundary, needFlingAtEdge, edgeEffect, IsAtTop(), IsAtBottom(), velocity,
        static_cast<int32_t>(host->GetAccessibilityId()), host->GetTag().c_str());
    if (!isOutOfBoundary && needFlingAtEdge) {
        // trigger scroll animation if edge not reached
        if (scrollableEvent_ && scrollableEvent_->GetScrollable()) {
            scrollableEvent_->GetScrollable()->StartScrollAnimation(0.0f, velocity);
            return true;
        }
        return false;
    }
    return HandleOverScroll(velocity) || GetEdgeEffect() == EdgeEffect::FADE;
}

bool ScrollablePattern::HandleOverScroll(float velocity)
{
    auto parent = GetNestedScrollParent();
    auto nestedScroll = GetNestedScroll();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto isOutOfBoundary = IsOutOfBoundary();
    ACE_SCOPED_TRACE("HandleOverScroll, IsOutOfBoundary:%u, id:%d, tag:%s", isOutOfBoundary,
        static_cast<int32_t>(host->GetAccessibilityId()), host->GetTag().c_str());
    if (!parent || !nestedScroll.NeedParent(velocity < 0) || isOutOfBoundary) {
        if (GetEdgeEffect() == EdgeEffect::SPRING && AnimateStoped()) {
            // trigger onScrollEnd later, when spring animation finishes
            ProcessSpringEffect(velocity);
            return true;
        }
        OnScrollEnd();
        return false;
    }
    // parent handle over scroll first
    if ((velocity < 0 && (nestedScroll.forward == NestedScrollMode::SELF_FIRST)) ||
        (velocity > 0 && (nestedScroll.backward == NestedScrollMode::SELF_FIRST)) ||
        (!InstanceOf<ScrollablePattern>(parent) && !isOutOfBoundary)) {
        if (parent->HandleScrollVelocity(velocity)) {
            OnScrollEnd();
            return true;
        }
        if (GetEdgeEffect() == EdgeEffect::SPRING) {
            ProcessSpringEffect(velocity);
            return true;
        }
    }

    // self handle over scroll first
    if (GetEdgeEffect() == EdgeEffect::SPRING) {
        ProcessSpringEffect(velocity);
        return true;
    }
    OnScrollEnd();
    return parent->HandleScrollVelocity(velocity);
}

void ScrollablePattern::ExecuteScrollFrameBegin(float& mainDelta, ScrollState state)
{
    auto context = PipelineContext::GetCurrentContextSafely();
    auto eventHub = GetEventHub<ScrollableEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto scrollFrameBeginCallback = eventHub->GetOnScrollFrameBegin();
    if (!context || !scrollFrameBeginCallback) {
        return;
    }

    auto offset = Dimension(mainDelta / context->GetDipScale(), DimensionUnit::VP);
    auto scrollRes = scrollFrameBeginCallback(-offset, state);
    mainDelta = -context->NormalizeToPx(scrollRes.offset);
}

void ScrollablePattern::OnScrollStartRecursive(float position, float velocity)
{
    SetIsNestedInterrupt(false);
    HandleScrollImpl(position, SCROLL_FROM_START);
    auto parent = GetNestedScrollParent();
    auto nestedScroll = GetNestedScroll();
    if (parent && nestedScroll.NeedParent()) {
        parent->OnScrollStartRecursive(position, GetVelocity());
    }
}

void ScrollablePattern::OnScrollEndRecursive(const std::optional<float>& velocity)
{
    OnScrollEnd();
    auto parent = GetNestedScrollParent();
    auto nestedScroll = GetNestedScroll();
    if (parent && (nestedScroll.NeedParent() || GetIsNestedInterrupt())) {
        parent->OnScrollEndRecursive(velocity);
    }
    SetIsNestedInterrupt(false);
}

float ScrollablePattern::GetVelocity() const
{
    float velocity = 0.0f;
    CHECK_NULL_RETURN(scrollableEvent_, velocity);
    auto scrollable = scrollableEvent_->GetScrollable();
    CHECK_NULL_RETURN(scrollable, velocity);
    velocity = scrollable->GetCurrentVelocity();
    return velocity;
}

void ScrollablePattern::RegisterScrollingListener(const RefPtr<ScrollingListener> listener)
{
    CHECK_NULL_VOID(listener);
    scrollingListener_.emplace_back(listener);
}

void ScrollablePattern::FireAndCleanScrollingListener()
{
    for (auto listener : scrollingListener_) {
        CHECK_NULL_VOID(listener);
        listener->NotifyScrollingEvent();
    }
    scrollingListener_.clear();
}

void ScrollablePattern::CleanScrollingListener()
{
    scrollingListener_.clear();
}

float ScrollablePattern::GetMainContentSize() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 0.0);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0);
    return geometryNode->GetPaddingSize().MainSize(axis_);
}

void ScrollablePattern::ScrollToEdge(ScrollEdgeType scrollEdgeType, bool smooth)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    ACE_SCOPED_TRACE("ScrollToEdge scrollEdgeType:%zu, id:%d, tag:%s", scrollEdgeType,
        static_cast<int32_t>(host->GetAccessibilityId()), host->GetTag().c_str());
    if (scrollEdgeType == ScrollEdgeType::SCROLL_TOP) {
        ScrollToIndex(0, false, ScrollAlign::START);
    } else if (scrollEdgeType == ScrollEdgeType::SCROLL_BOTTOM) {
        // use LAST_ITEM for children count changed after scrollEdge(Edge.Bottom) and before layout
        ScrollToIndex(LAST_ITEM, false, ScrollAlign::END);
    }
}

void ScrollablePattern::Fling(double flingVelocity)
{
    if (!IsScrollableStopped()) {
        scrollAbort_ = true;
        StopScrollable();
    }
    if (!isAnimationStop_) {
        scrollAbort_ = true;
        StopAnimation(springAnimation_);
        StopAnimation(curveAnimation_);
    }
    if (animator_ && !animator_->IsStopped()) {
        scrollAbort_ = true;
        animator_->Stop();
    }
    CHECK_NULL_VOID(scrollableEvent_);
    auto scrollable = scrollableEvent_->GetScrollable();
    CHECK_NULL_VOID(scrollable);
    if (IsOutOfBoundary()) {
        scrollable->HandleOverScroll(flingVelocity);
    } else {
        scrollable->StartScrollAnimation(0.0f, flingVelocity);
    }
    auto pipeline = GetContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->RequestFrame();
}

void ScrollablePattern::NotifyFRCSceneInfo(const std::string& scene, double velocity, SceneStatus sceneStatus)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->AddFRCSceneInfo(scene, velocity, sceneStatus);
}

void ScrollablePattern::FireOnScrollStart()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<ScrollableEventHub>();
    CHECK_NULL_VOID(hub);
    SuggestOpIncGroup(true);
    if (scrollStop_ && !GetScrollAbort()) {
        OnScrollStop(hub->GetOnScrollStop());
    }
    UIObserverHandler::GetInstance().NotifyScrollEventStateChange(
        AceType::WeakClaim(this), ScrollEventType::SCROLL_START);
    PerfMonitor::GetPerfMonitor()->Start(PerfConstants::APP_LIST_FLING, PerfActionType::FIRST_MOVE, "");
    if (GetScrollAbort()) {
        ACE_SCOPED_TRACE("ScrollAbort, no OnScrollStart, id:%d, tag:%s",
            static_cast<int32_t>(host->GetAccessibilityId()), host->GetTag().c_str());
        return;
    }
    auto scrollBar = GetScrollBar();
    if (scrollBar) {
        scrollBar->PlayScrollBarAppearAnimation();
    }
    StopScrollBarAnimatorByProxy();
    host->OnAccessibilityEvent(AccessibilityEventType::SCROLL_START);
    auto onScrollStart = hub->GetOnScrollStart();
    CHECK_NULL_VOID(onScrollStart);
    ACE_SCOPED_TRACE(
        "OnScrollStart, id:%d, tag:%s", static_cast<int32_t>(host->GetAccessibilityId()), host->GetTag().c_str());
    onScrollStart();
    AddEventsFiredInfo(ScrollableEventType::ON_SCROLL_START);
}

void ScrollablePattern::OnScrollStartCallback()
{
    FireOnScrollStart();
};

void ScrollablePattern::FireOnScroll(float finalOffset, OnScrollEvent& onScroll) const
{
    auto offsetPX = Dimension(finalOffset);
    auto offsetVP = Dimension(offsetPX.ConvertToVp(), DimensionUnit::VP);
    auto scrollState = GetScrollState();
    bool isTriggered = false;
    if (!NearZero(finalOffset)) {
        onScroll(offsetVP, scrollState);
        isTriggered = true;
    }
    if (scrollStop_ && !GetScrollAbort()) {
        if (scrollState != ScrollState::IDLE || !isTriggered) {
            onScroll(0.0_vp, ScrollState::IDLE);
        }
    }
}

void ScrollablePattern::SuggestOpIncGroup(bool flag)
{
    if (!SystemProperties::IsOpIncEnable()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (host->GetSuggestOpIncActivatedOnce()) {
        return;
    }
    flag = flag && isVertical();
    if (flag) {
        ACE_SCOPED_TRACE("SuggestOpIncGroup %s", host->GetHostTag().c_str());
        auto parent = host->GetAncestorNodeOfFrame();
        CHECK_NULL_VOID(parent);
        parent->SetSuggestOpIncActivatedOnce();
        host->SetSuggestOpIncActivatedOnce();
        // get 1st layer
        for (auto child : host->GetAllChildren()) {
            if (!child) {
                continue;
            }
            auto frameNode = AceType::DynamicCast<FrameNode>(child);
            if (!frameNode || frameNode->GetSuggestOpIncActivatedOnce()) {
                continue;
            }
            std::string path(host->GetHostTag());
            frameNode->FindSuggestOpIncNode(path, host->GetGeometryNode()->GetFrameSize(), 1);
        }
    }
}

void ScrollablePattern::OnScrollStop(const OnScrollStopEvent& onScrollStop)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (scrollStop_) {
        UIObserverHandler::GetInstance().NotifyScrollEventStateChange(
            AceType::WeakClaim(this), ScrollEventType::SCROLL_STOP);
        if (!GetScrollAbort()) {
            if (host != nullptr) {
                host->OnAccessibilityEvent(AccessibilityEventType::SCROLL_END);
            }
            if (onScrollStop) {
                ACE_SCOPED_TRACE("OnScrollStop, id:%d, tag:%s", static_cast<int32_t>(host->GetAccessibilityId()),
                    host->GetTag().c_str());
                onScrollStop();
                AddEventsFiredInfo(ScrollableEventType::ON_SCROLL_STOP);
                SetScrollSource(SCROLL_FROM_NONE);
            }
            auto scrollBar = GetScrollBar();
            if (scrollBar) {
                scrollBar->ScheduleDisappearDelayTask();
            }
            StartScrollBarAnimatorByProxy();
        } else {
            ACE_SCOPED_TRACE("ScrollAbort, no OnScrollStop, id:%d, tag:%s",
                static_cast<int32_t>(host->GetAccessibilityId()), host->GetTag().c_str());
        }
        PerfMonitor::GetPerfMonitor()->End(PerfConstants::APP_LIST_FLING, false);
        AceAsyncTraceEnd(
            0, (TRAILING_ANIMATION + std::to_string(host->GetAccessibilityId()) + std::string(" ") + host->GetTag())
                .c_str());
        scrollStop_ = false;
        SetScrollAbort(false);
    }
}

float ScrollablePattern::FireOnWillScroll(float offset) const
{
    auto eventHub = GetEventHub<ScrollableEventHub>();
    CHECK_NULL_RETURN(eventHub, offset);
    auto onScroll = eventHub->GetOnWillScroll();
    CHECK_NULL_RETURN(onScroll, offset);
    auto offsetPX = Dimension(offset);
    auto offsetVP = Dimension(offsetPX.ConvertToVp(), DimensionUnit::VP);
    auto scrollRes = onScroll(offsetVP, GetScrollState(), ConvertScrollSource(scrollSource_));
    auto context = PipelineContext::GetCurrentContextSafely();
    CHECK_NULL_RETURN(context, offset);
    return context->NormalizeToPx(scrollRes.offset);
}

/**
 * @description: Register with the drag drop manager
 * @return None
 */
void ScrollablePattern::Register2DragDropManager()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto dragDropManager = pipeline->GetDragDropManager();
    CHECK_NULL_VOID(dragDropManager);
    dragDropManager->RegisterDragStatusListener(host->GetId(), AceType::WeakClaim(AceType::RawPtr(host)));
}

/**
 * @description: Determine whether it is in the hot zone, then
 * 1.Gives the rolling direction according to the location of the hot zone
 * 2.Gives the distance from the edge of the hot zone from the drag point
 * @param {PointF&} point The drag point
 * @return The distance from the edge of the hot zone from the drag point.scroll up:Offset percent is positive, scroll
 * down:Offset percent is  negative
 */
float ScrollablePattern::IsInHotZone(const PointF& point)
{
    auto host = GetHost();
    auto offset = 0.f;
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.f);

    auto wholeRect = geometryNode->GetFrameRect();
    wholeRect.SetOffset(host->GetTransformRelativeOffset());
    auto hotZoneHeightPX = HOT_ZONE_HEIGHT_VP_DIM.ConvertToPx();
    auto hotZoneWidthPX = HOT_ZONE_WIDTH_VP_DIM.ConvertToPx();
    if (isVertical()) {
        // create top hot zone,it is a rectangle
        auto topHotzone = wholeRect;
        topHotzone.SetHeight(hotZoneHeightPX);

        // create bottom hot zone,it is a rectangle
        auto bottomHotzone = wholeRect;
        auto bottomZoneEdgeY = wholeRect.GetY() + wholeRect.Height();
        bottomHotzone.SetTop(bottomZoneEdgeY - hotZoneHeightPX);
        bottomHotzone.SetHeight(hotZoneHeightPX);

        // Determines whether the drag point is within the hot zone,
        // then gives the scroll component movement direction according to which hot zone the point is in
        // top or bottom hot zone
        if (topHotzone.IsInRegion(point)) {
            offset = hotZoneHeightPX - point.GetY() + topHotzone.GetY();
            if (!NearZero(hotZoneHeightPX)) {
                return offset / hotZoneHeightPX;
            }
        } else if (bottomHotzone.IsInRegion(point)) {
            offset = bottomZoneEdgeY - point.GetY() - hotZoneHeightPX;
            if (!NearZero(hotZoneHeightPX)) {
                return offset / hotZoneHeightPX;
            }
        }
    } else {
        auto leftHotzone = wholeRect;

        // create left hot zone,it is a rectangle
        leftHotzone.SetWidth(hotZoneWidthPX);

        // create right hot zone,it is a rectangle
        auto rightHotzone = wholeRect;
        rightHotzone.SetWidth(hotZoneWidthPX);
        auto rightZoneEdgeX = wholeRect.GetX() + wholeRect.Width();
        rightHotzone.SetLeft(rightZoneEdgeX - hotZoneWidthPX);

        // Determines whether the drag point is within the hot zone,
        // gives the scroll component movement direction according to which hot zone the point is in
        // left or right hot zone
        if (leftHotzone.IsInRegion(point)) {
            offset = hotZoneWidthPX - point.GetX() + wholeRect.GetX();
            if (!NearZero(hotZoneWidthPX)) {
                return offset / hotZoneWidthPX;
            }
        } else if (rightHotzone.IsInRegion(point)) {
            offset = rightZoneEdgeX - point.GetX() - hotZoneWidthPX;
            if (!NearZero(hotZoneWidthPX)) {
                return offset / hotZoneWidthPX;
            }
        }
    }

    return 0.f;
}

/**
 * @description: Determines whether the scroll component is in the vertical direction
 * @return True,If the scrolling component is vertical
 */
bool ScrollablePattern::isVertical() const
{
    return axis_ == Axis::VERTICAL;
}

/**
 * @description: scroll up or down
 * @param {float} offsetPct offset percent.When scrolling in the vertical or horizontal direction, there is a distance
 * between the drag point and the outer edge of the hot zone, and the percentage represents the proportion of this
 * distance to the height or width of the hot zone
 * @return None
 */
void ScrollablePattern::HotZoneScroll(const float offsetPct)
{
    auto host = GetHost();
    CHECK_NULL_VOID(IsScrollable());
    CHECK_NULL_VOID(!NearZero(offsetPct));

    // There are three types of situations to consider.
    // 1. Enter the hot zone for the first time.
    // 2. When the drag point leaves the hot zone, it enters the hot zone again
    // 3. When the drag point moves within the hot zone, the hot zone offset changes
    CHECK_NULL_VOID(!NearEqual(lastHonezoneOffsetPct_, offsetPct));

    if (AnimateRunning()) {
        // Variable speed rolling
        // When the drag point is in the hot zone, and the hot zone offset changes.
        // Then need to modify the offset percent
        if (velocityMotion_) {
            velocityMotion_->Reset(offsetPct);
        }
        return;
    }

    if (!animator_) {
        animator_ = CREATE_ANIMATOR(PipelineBase::GetCurrentContext());
        animator_->AddStopListener([weak = AceType::WeakClaim(this)]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->AddHotZoneSenceInterface(SceneStatus::END);
            pattern->OnAnimateStop();
        });
        animator_->AddStartListener([weak = AceType::WeakClaim(this)]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->AddHotZoneSenceInterface(SceneStatus::START);
        });
    }

    if (!velocityMotion_) {
        // Enter the hot zone for the first time.
        velocityMotion_ = AceType::MakeRefPtr<BezierVariableVelocityMotion>(
            offsetPct, [weak = WeakClaim(this)](float offset) -> bool {
                auto pattern = weak.Upgrade();
                CHECK_NULL_RETURN(pattern, true);
                // Stop scrolling when reach the bottom or top
                return ((LessNotEqual(offset, 0) && pattern->IsAtBottom()) ||
                    (GreatNotEqual(offset, 0) && pattern->IsAtTop()));
            });
        velocityMotion_->AddListener([weakScroll = AceType::WeakClaim(this)](double offset) {
            // Get the distance component need to roll from BezierVariableVelocityMotion
            // Roll up: negative value, Roll up: positive value
            auto pattern = weakScroll.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->UpdateCurrentOffset(offset, SCROLL_FROM_AXIS);
            pattern->UpdateMouseStart(offset);
            pattern->AddHotZoneSenceInterface(SceneStatus::RUNNING);
            if (pattern->hotZoneScrollCallback_) {
                pattern->hotZoneScrollCallback_();
            }
        });
        velocityMotion_->ReInit(offsetPct);
    } else {
        // When the drag point leaves the hot zone, it enters the hot zone again.Then need to reset offset percent.
        velocityMotion_->ReInit(offsetPct);
    }
    // Save the last offset percent
    lastHonezoneOffsetPct_ = offsetPct;
    animator_->PlayMotion(velocityMotion_);
    FireOnScrollStart();
}

/**
 * @description: When the drag point leaves the hot zone, stop the animation.
 * @return None
 */
void ScrollablePattern::StopHotzoneScroll()
{
    if (!AnimateStoped()) {
        animator_->Stop();
    }
}

/**
 * @description: Handle drag and drop events
 * When a drag point enters or moves over a component, determine whether it is within the hot zone.
 * When leave the component, stop scrolling
 * @param {DragEventType&} dragEventType Drag the event type
 * @param {NotifyDragEvent&} notifyDragEvent Drag event
 * @return None
 */
void ScrollablePattern::HandleHotZone(
    const DragEventType& dragEventType, const RefPtr<NotifyDragEvent>& notifyDragEvent)
{
    // The starting version of the auto-scroll feature is 11
    CHECK_NULL_VOID(Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN));
    PointF point(static_cast<float>(notifyDragEvent->GetX()), static_cast<float>(notifyDragEvent->GetY()));
    switch (dragEventType) {
        case DragEventType::ENTER: {
            HandleMoveEventInComp(point);
            break;
        }
        case DragEventType::MOVE: {
            HandleMoveEventInComp(point);
            break;
        }
        case DragEventType::DROP:
        case DragEventType::LEAVE: {
            HandleLeaveHotzoneEvent();
            break;
        }
        default:
            break;
    }
}

/**
 * @description:When a drag point is inside the scroll component, it is necessary to handle the events of each moving
 * point
 * @param {PointF&} point the drag point
 * @return None
 */
void ScrollablePattern::HandleMoveEventInComp(const PointF& point)
{
    float offsetPct = IsInHotZone(point);
    if ((Positive(offsetPct) && !IsAtTop()) || (Negative(offsetPct) && !IsAtBottom())) {
        // The drag point enters the hot zone
        HotZoneScroll(offsetPct);
    } else {
        // Although it entered the rolling component, it is not in the rolling component hot zone.Then stop
        // scrolling
        HandleLeaveHotzoneEvent();
    }
}

/**
 * @description:When the drag point is not in the hot zone, need to stop scrolling, if it exists.
 * This function is executed multiple times
 * @return None
 */
void ScrollablePattern::HandleLeaveHotzoneEvent()
{
    // Stop scrolling up and down
    StopHotzoneScroll();
}

/**
 * @description: This is the entry point for handling drag events
 * @return None
 */
void ScrollablePattern::HandleOnDragStatusCallback(
    const DragEventType& dragEventType, const RefPtr<NotifyDragEvent>& notifyDragEvent)
{
    HandleHotZone(dragEventType, notifyDragEvent);
}

/**
 * @description: Cancel registration with the drag drop manager
 * @return None
 */
void ScrollablePattern::UnRegister2DragDropManager()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto dragDropManager = pipeline->GetDragDropManager();
    CHECK_NULL_VOID(dragDropManager);
    dragDropManager->UnRegisterDragStatusListener(host->GetId());
}

bool ScrollablePattern::NeedCoordinateScrollWithNavigation(
    double offset, int32_t source, const OverScrollOffset& overOffsets)
{
    if (!navBarPattern_) {
        return false;
    }
    return (GreatNotEqual(overOffsets.start, 0.0) || navBarPattern_->CanCoordScrollUp(offset)) &&
           (axis_ == Axis::VERTICAL) && (source != SCROLL_FROM_ANIMATION_SPRING);
}

void ScrollablePattern::AddHotZoneSenceInterface(SceneStatus scene)
{
    CHECK_NULL_VOID(velocityMotion_);
    auto velocity = velocityMotion_->GetCurrentVelocity();
    NotifyFRCSceneInfo(SCROLL_IN_HOTZONE_SCENE, velocity, scene);
}

void ScrollablePattern::InitScrollBarClickEvent()
{
    CHECK_NULL_VOID(scrollBar_);
    auto gesture = GetHost()->GetOrCreateGestureEventHub();
    auto clickCallback = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleClickEvent(info);
    };
    auto scrollBarClickEvent = scrollBar_->GetClickEvent();
    scrollBarClickEvent = MakeRefPtr<ClickEvent>(std::move(clickCallback));
    gesture->AddClickEvent(scrollBarClickEvent);
}

void ScrollablePattern::HandleClickEvent(GestureEvent& info)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto localLocation = info.GetLocalLocation();
    CHECK_NULL_VOID(GetScrollBar());
    Point point(localLocation.GetX(), localLocation.GetY());
    bool reverse = false;
    if (scrollBar_->AnalysisUpOrDown(point, reverse) && isMousePressed_) {
        ScrollPage(reverse, true);
    }
}

void ScrollablePattern::ScrollPage(bool reverse, bool smooth)
{
    float distance = reverse ? GetMainContentSize() : -GetMainContentSize();
    float position = -GetTotalOffset() + distance;
    AnimateTo(-position, -1, nullptr, true, false, false);
    return;
}

void ScrollablePattern::InitScrollBarMouseEvent()
{
    CHECK_NULL_VOID(!mouseEvent_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto inputHub = GetInputHub();
    CHECK_NULL_VOID(inputHub);
    auto mouseTask = [weak = WeakClaim(this)](MouseInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (info.GetButton() == MouseButton::LEFT_BUTTON && info.GetAction() == MouseAction::PRESS) {
            pattern->isMousePressed_ = true;
            pattern->InitScrollBarClickEvent();
        } else {
            pattern->isMousePressed_ = false;
        }
    };
    mouseEvent_ = MakeRefPtr<InputEvent>(std::move(mouseTask));
    inputHub->AddOnMouseEvent(mouseEvent_);
}

void ScrollablePattern::PrintOffsetLog(AceLogTag tag, int32_t id, double finalOffset)
{
    if (SystemProperties::GetDebugOffsetLogEnabled() && !NearZero(finalOffset)) {
        TAG_LOGD(tag, "Scrollable id:%{public}d, scrollSource:%{public}d, scrollOffset:%{public}f",
            id, scrollSource_, finalOffset);
    }
}

PositionMode ScrollablePattern::GetPositionMode()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, PositionMode::RIGHT);
    auto positionMode = PositionMode::RIGHT;
    if (axis_ == Axis::HORIZONTAL) {
        positionMode = PositionMode::BOTTOM;
    } else {
        auto isRtl = host->GetLayoutProperty()->GetNonAutoLayoutDirection() == TextDirection::RTL;
        if (isRtl) {
            positionMode = PositionMode::LEFT;
        }
    }
    return positionMode;
}

void ScrollablePattern::ScrollAtFixedVelocity(float velocity)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (AnimateRunning()) {
        StopAnimate();
    }

    if (!animator_) {
        animator_ = CREATE_ANIMATOR(PipelineBase::GetCurrentContext());
        animator_->AddStopListener([weak = AceType::WeakClaim(this)]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->OnAnimateStop();
            auto host = pattern->GetHost();
            CHECK_NULL_VOID(host);
            AceAsyncTraceEnd(
                host->GetId(), (SCROLLER_FIX_VELOCITY_ANIMATION + std::to_string(host->GetAccessibilityId()) +
                                   std::string(" ") + host->GetTag()).c_str());
        });
    }

    if (!fixedVelocityMotion_) {
        fixedVelocityMotion_ = AceType::MakeRefPtr<VelocityMotion>([weak = WeakClaim(this)](float offset) -> bool {
            auto pattern = weak.Upgrade();
            CHECK_NULL_RETURN(pattern, true);
            if (LessNotEqual(offset, 0) && pattern->IsAtBottom()) {
                // Stop scrolling when reach the bottom
                pattern->fixedVelocityMotion_->Init();
                return true;
            } else if (GreatNotEqual(offset, 0) && pattern->IsAtTop()) {
                // Stop scrolling when reach the top
                pattern->fixedVelocityMotion_->Init();
                return true;
            }
            return false;
        });
        fixedVelocityMotion_->AddListener([weakScroll = AceType::WeakClaim(this)](double offset) {
            auto pattern = weakScroll.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->UpdateCurrentOffset(offset, SCROLL_FROM_ANIMATION_CONTROLLER);
        });
        fixedVelocityMotion_->SetVelocity(velocity);
    } else {
        fixedVelocityMotion_->Init();
        fixedVelocityMotion_->SetVelocity(velocity);
    }
    AceAsyncTraceBegin(host->GetId(), (SCROLLER_FIX_VELOCITY_ANIMATION + std::to_string(host->GetAccessibilityId()) +
        std::string(" ") + host->GetTag()).c_str());
    animator_->PlayMotion(fixedVelocityMotion_);
    FireOnScrollStart();
}

void ScrollablePattern::CheckRestartSpring(bool sizeDiminished)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto edgeEffect = GetScrollEdgeEffect();
    if (!edgeEffect || !edgeEffect->IsSpringEffect()) {
        return;
    }
    // Check if need update Spring when itemTotalSize diminishes.
    if (IsScrollableSpringMotionRunning() && sizeDiminished) {
        ACE_SCOPED_TRACE("CheckRestartSpring, do ProcessSpringUpdate, id:%d, tag:%s",
            static_cast<int32_t>(host->GetAccessibilityId()), host->GetTag().c_str());
        edgeEffect->ProcessSpringUpdate();
        return;
    }
    if (!ScrollableIdle() || !IsOutOfBoundary()) {
        return;
    }
    if (AnimateRunning()) {
        return;
    }
    FireOnScrollStart();
    ACE_SCOPED_TRACE("CheckRestartSpring, do ProcessScrollOver, id:%d, tag:%s",
        static_cast<int32_t>(host->GetAccessibilityId()), host->GetTag().c_str());
    edgeEffect->ProcessScrollOver(0);
}

void ScrollablePattern::AddEventsFiredInfo(ScrollableEventType eventType)
{
    if (eventsFiredInfos_.size() >= EVENTS_FIRED_INFO_COUNT) {
        eventsFiredInfos_.pop_front();
    }
    eventsFiredInfos_.push_back(ScrollableEventsFiredInfo({
        .eventFiredTime_ = GetSysTimestamp(),
        .eventType_ = eventType,
        .scrollSource_ = scrollSource_,
    }));
}

void ScrollablePattern::AddScrollableFrameInfo(int32_t scrollSource)
{
    if (scrollableFrameInfos_.size() >= SCROLLABLE_FRAME_INFO_COUNT) {
        scrollableFrameInfos_.pop_front();
    }
    uint32_t canOverScrollInfo = IsScrollableSpringEffect();
    canOverScrollInfo = (canOverScrollInfo << 1) | IsScrollable();
    canOverScrollInfo = (canOverScrollInfo << 1) | ScrollableIdle();
    canOverScrollInfo = (canOverScrollInfo << 1) | animateOverScroll_;
    canOverScrollInfo = (canOverScrollInfo << 1) | animateCanOverScroll_;
    scrollableFrameInfos_.push_back(ScrollableFrameInfo({
        .scrollStateTime_ = GetSysTimestamp(),
        .scrollState_ = scrollSource,
        .canOverScroll_ = lastCanOverScroll_,
        .canOverScrollInfo_ = canOverScrollInfo,
    }));
}

void ScrollablePattern::GetEdgeEffectDumpInfo()
{
    switch (edgeEffect_) {
        case EdgeEffect::NONE: {
            DumpLog::GetInstance().AddDesc("edgeEffect: NONE");
            break;
        }
        case EdgeEffect::SPRING: {
            DumpLog::GetInstance().AddDesc("edgeEffect: SPRING");
            break;
        }
        case EdgeEffect::FADE: {
            DumpLog::GetInstance().AddDesc("edgeEffect: FADE");
            break;
        }
        default: {
            break;
        }
    }
}

void ScrollablePattern::GetAxisDumpInfo()
{
    switch (axis_) {
        case Axis::NONE: {
            DumpLog::GetInstance().AddDesc("Axis: NONE");
            break;
        }
        case Axis::VERTICAL: {
            DumpLog::GetInstance().AddDesc("Axis: VERTICAL");
            break;
        }
        case Axis::HORIZONTAL: {
            DumpLog::GetInstance().AddDesc("Axis: HORIZONTAL");
            break;
        }
        case Axis::FREE: {
            DumpLog::GetInstance().AddDesc("Axis: FREE");
            break;
        }
        default: {
            break;
        }
    }
}

void ScrollablePattern::GetPanDirectionDumpInfo()
{
    switch (GetScrollablePanDirection()) {
        case Axis::NONE: {
            DumpLog::GetInstance().AddDesc("ScrollablePanDirection:NONE");
            break;
        }
        case Axis::VERTICAL: {
            DumpLog::GetInstance().AddDesc("ScrollablePanDirection:VERTICAL");
            break;
        }
        case Axis::HORIZONTAL: {
            DumpLog::GetInstance().AddDesc("ScrollablePanDirection:HORIZONTAL");
            break;
        }
        case Axis::FREE: {
            DumpLog::GetInstance().AddDesc("ScrollablePanDirection:FREE");
            break;
        }
        default: {
            DumpLog::GetInstance().AddDesc("ScrollablePanDirection is null");
            break;
        }
    }
}

void ScrollablePattern::GetPaintPropertyDumpInfo()
{
    auto paintProperty = GetPaintProperty<ScrollablePaintProperty>();
    if (paintProperty) {
        switch (paintProperty->GetScrollBarMode().value_or(DisplayMode::OFF)) {
            case DisplayMode::OFF: {
                DumpLog::GetInstance().AddDesc("innerScrollBarState: OFF");
                break;
            }
            case DisplayMode::AUTO: {
                DumpLog::GetInstance().AddDesc("innerScrollBarState: AUTO");
                break;
            }
            case DisplayMode::ON: {
                DumpLog::GetInstance().AddDesc("innerScrollBarState: ON");
                break;
            }
            default: {
                break;
            }
        }
        auto scrollBarWidth = paintProperty->GetScrollBarWidth();
        scrollBarWidth.has_value() ? DumpLog::GetInstance().AddDesc(std::string("scrollBarWidth: ")
            .append(paintProperty->GetScrollBarWidth().value().ToString()))
            : DumpLog::GetInstance().AddDesc("scrollBarWidth: None");
    }
}

void ScrollablePattern::GetEventDumpInfo()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<ScrollableEventHub>();
    CHECK_NULL_VOID(hub);
    auto onScrollStart = hub->GetOnScrollStart();
    onScrollStart ? DumpLog::GetInstance().AddDesc("hasOnScrollStart: true")
                  : DumpLog::GetInstance().AddDesc("hasOnScrollStart: false");
    auto onScrollStop = hub->GetOnScrollStop();
    onScrollStop ? DumpLog::GetInstance().AddDesc("hasOnScrollStop: true")
                 : DumpLog::GetInstance().AddDesc("hasOnScrollStop: false");
    auto scrollHub = host->GetEventHub<ScrollEventHub>();
    if (scrollHub) {
        auto onWillScroll = scrollHub->GetOnWillScrollEvent();
        onWillScroll ? DumpLog::GetInstance().AddDesc("hasOnWillScroll: true")
                     : DumpLog::GetInstance().AddDesc("hasOnWillScroll: false");
        auto onDidScroll = scrollHub->GetOnDidScrollEvent();
        onDidScroll ? DumpLog::GetInstance().AddDesc("hasOnDidScroll: true")
                    : DumpLog::GetInstance().AddDesc("hasOnDidScroll: false");
    } else {
        auto onWillScroll = hub->GetOnWillScroll();
        onWillScroll ? DumpLog::GetInstance().AddDesc("hasOnWillScroll: true")
                     : DumpLog::GetInstance().AddDesc("hasOnWillScroll: false");
        auto onDidScroll = hub->GetOnDidScroll();
        onDidScroll ? DumpLog::GetInstance().AddDesc("hasOnDidScroll: true")
                    : DumpLog::GetInstance().AddDesc("hasOnDidScroll: false");
    }
    auto onScrollFrameBegin = hub->GetOnScrollFrameBegin();
    onScrollFrameBegin ? DumpLog::GetInstance().AddDesc("hasOnScrollFrameBegin: true")
                       : DumpLog::GetInstance().AddDesc("hasOnScrollFrameBegin: false");
    auto onReachStart = hub->GetOnReachStart();
    onReachStart ? DumpLog::GetInstance().AddDesc("hasOnReachStart: true")
                 : DumpLog::GetInstance().AddDesc("hasOnReachStart: false");
    auto onReachEnd = hub->GetOnReachEnd();
    onReachEnd ? DumpLog::GetInstance().AddDesc("hasOnReachEnd: true")
               : DumpLog::GetInstance().AddDesc("hasOnReachEnd: false");
}

void ScrollablePattern::DumpAdvanceInfo()
{
    GetEdgeEffectDumpInfo();
    edgeEffectAlwaysEnabled_ ? DumpLog::GetInstance().AddDesc("edgeEffectAlwaysEnabled: true")
                             : DumpLog::GetInstance().AddDesc("edgeEffectAlwaysEnabled: false");
    IsScrollable() ? DumpLog::GetInstance().AddDesc("isScrollable: true")
                   : DumpLog::GetInstance().AddDesc("isScrollable: false");
    GetEventDumpInfo();
    DumpLog::GetInstance().AddDesc(GetNestedScroll().ToString().c_str());
    GetIsSearchRefresh() ? DumpLog::GetInstance().AddDesc(std::string("isSearchRefresh: true"))
                         : DumpLog::GetInstance().AddDesc(std::string("isSearchRefresh: false"));
    GetIsFixedNestedScrollMode() ? DumpLog::GetInstance().AddDesc(std::string("isFixedNestedScrollMode: true"))
                                 : DumpLog::GetInstance().AddDesc(std::string("isFixedNestedScrollMode: false"));
    auto parent = GetNestedScrollParent();
    parent && parent->GetHost() ? DumpLog::GetInstance().AddDesc(std::string("nestedScrollParent id: ")
                                                                     .append(std::to_string(parent->GetHost()->GetId()))
                                                                     .append(" tag: ")
                                                                     .append(parent->GetHost()->GetTag()))
                                : DumpLog::GetInstance().AddDesc("nestedScrollParent is null");
    GetAxisDumpInfo();
    GetPanDirectionDumpInfo();
    GetPaintPropertyDumpInfo();
    GetScrollEnabled() ? DumpLog::GetInstance().AddDesc("enableScrollInteraction: true")
                       : DumpLog::GetInstance().AddDesc("enableScrollInteraction: false");
    DumpLog::GetInstance().AddDesc(std::string("friction: ").append(std::to_string(friction_)));
    DumpLog::GetInstance().AddDesc(std::string("flingSpeedLimit: ").append(std::to_string(GetMaxFlingVelocity())));
    DumpLog::GetInstance().AddDesc("==========================eventsFiredInfos==============================");
    for (const auto& info : eventsFiredInfos_) {
        DumpLog::GetInstance().AddDesc(info.ToString());
    }
    DumpLog::GetInstance().AddDesc("==========================eventsFiredInfos==============================");
    DumpLog::GetInstance().AddDesc("==========================scrollableFrameInfos==========================");
    for (const auto& info : scrollableFrameInfos_) {
        DumpLog::GetInstance().AddDesc(info.ToString());
    }
    DumpLog::GetInstance().AddDesc("==========================scrollableFrameInfos==========================");
    DumpLog::GetInstance().AddDesc("==========================inner ScrollBar===============================");
    if (scrollBar_) {
        scrollBar_->DumpAdvanceInfo();
    } else {
        DumpLog::GetInstance().AddDesc("inner ScrollBar is null");
    }
    DumpLog::GetInstance().AddDesc("==========================inner ScrollBar===============================");
}
} // namespace OHOS::Ace::NG
