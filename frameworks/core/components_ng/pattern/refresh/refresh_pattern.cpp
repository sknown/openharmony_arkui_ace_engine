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

#include "core/components_ng/pattern/refresh/refresh_pattern.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/log/dump_log.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/animation/spring_curve.h"
#include "core/common/container.h"
#include "core/components/common/properties/animation_option.h"
#include "core/components/refresh/refresh_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/loading_progress/loading_progress_layout_property.h"
#include "core/components_ng/pattern/loading_progress/loading_progress_paint_property.h"
#include "core/components_ng/pattern/refresh/refresh_animation_state.h"
#include "core/components_ng/pattern/refresh/refresh_layout_property.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/animation_utils.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/base/i18n/localization.h"
#include "frameworks/base/utils/time_util.h"
#include "frameworks/base/utils/utils.h"
#include "frameworks/core/components/common/layout/constants.h"
#include "frameworks/core/components_ng/pattern/loading_progress/loading_progress_pattern.h"
#include "frameworks/core/components_ng/pattern/text/text_pattern.h"

namespace OHOS::Ace::NG {

namespace {
constexpr float PERCENT = 0.01f; // Percent
constexpr float FOLLOW_TO_RECYCLE_DURATION = 600.0f;
constexpr float CUSTOM_BUILDER_ANIMATION_DURATION = 100.0f;
constexpr float LOADING_ANIMATION_DURATION = 350.0f;
constexpr float MAX_OFFSET = 100000.0f;
constexpr float HALF = 0.5f;
constexpr float BASE_SCALE = 0.707f; // std::sqrt(2)/2
constexpr Dimension TRIGGER_LOADING_DISTANCE = 16.0_vp;
constexpr Dimension TRIGGER_REFRESH_WITH_TEXT_DISTANCE = 96.0_vp;
constexpr Dimension TRIGGER_REFRESH_DISTANCE = 64.0_vp;
constexpr Dimension MAX_SCROLL_DISTANCE = 128.0_vp;
constexpr Dimension LOADING_PROGRESS_SIZE = 32.0_vp;
constexpr float DEFAULT_FRICTION = 62.0f;
const RefPtr<Curve> DEFAULT_CURVE = AceType::MakeRefPtr<CubicCurve>(0.2f, 0.0f, 0.1f, 1.0f);
const std::string REFRESH_DRAG_SCENE = "refresh_drag_scene";
constexpr Dimension LOADING_TEXT_TOP_MARGIN = 16.0_vp;
constexpr Dimension LOADING_TEXT_DISPLAY_DISTANCE = 80.0_vp;
} // namespace


Dimension RefreshPattern::GetTriggerRefreshDisTance()
{
    if (hasLoadingText_) {
        return TRIGGER_REFRESH_WITH_TEXT_DISTANCE;
    } else {
        return TRIGGER_REFRESH_DISTANCE;
    }
}

void RefreshPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToBounds(true);
    host->GetRenderContext()->UpdateClipEdge(true);
}

void RefreshPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    auto layoutProperty = GetLayoutProperty<RefreshLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    refreshOffset_ = layoutProperty->GetRefreshOffset().value_or(GetTriggerRefreshDisTance()).Value() > 0 ?
        layoutProperty->GetRefreshOffset().value_or(GetTriggerRefreshDisTance()) : GetTriggerRefreshDisTance();
    pullToRefresh_ = layoutProperty->GetPullToRefresh().value_or(true);
    hasLoadingText_ = layoutProperty->HasLoadingText();
    InitPanEvent(gestureHub);
    InitOnKeyEvent();
    InitChildNode();
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        InitOffsetProperty();
    } else {
        triggerLoadingDistance_ = static_cast<float>(
            std::clamp(layoutProperty->GetIndicatorOffset().value_or(TRIGGER_LOADING_DISTANCE).ConvertToPx(),
                -1.0f * TRIGGER_LOADING_DISTANCE.ConvertToPx(), GetTriggerRefreshDisTance().ConvertToPx()));
        InitLowVersionOffset();
    }
    RefreshStatusChangeEffect();
    SetAccessibilityAction();
}

RefPtr<LayoutAlgorithm> RefreshPattern::CreateLayoutAlgorithm()
{
    auto refreshLayoutAlgorithm = MakeRefPtr<RefreshLayoutAlgorithm>();
    if (isCustomBuilderExist_) {
        refreshLayoutAlgorithm->SetCustomBuilderIndex(0);
        if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
            refreshLayoutAlgorithm->SetBuilderMeasureBaseHeight(builderMeasureBaseHeight_);
        } else {
            refreshLayoutAlgorithm->SetCustomBuilderOffset(customBuilderOffset_);
            refreshLayoutAlgorithm->SetScrollOffset(scrollOffset_);
        }
    }
    return refreshLayoutAlgorithm;
}

void RefreshPattern::InitPanEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (panEvent_) {
        return;
    }
    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto speed = static_cast<float>(info.GetMainVelocity());
        pattern->UpdateDragFRCSceneInfo(REFRESH_DRAG_SCENE, speed, SceneStatus::START);
        pattern->HandleDragStart(true, speed);
    };
    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragUpdate(static_cast<float>(info.GetMainDelta()), static_cast<float>(info.GetMainVelocity()));
    };
    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto speed = static_cast<float>(info.GetMainVelocity());
        pattern->UpdateDragFRCSceneInfo(REFRESH_DRAG_SCENE, speed, SceneStatus::END);
        pattern->HandleDragEnd(speed);
    };
    auto actionCancelTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragCancel();
    };
    PanDirection panDirection;
    panDirection.type = PanDirection::VERTICAL;
    if (panEvent_) {
        gestureHub->RemovePanEvent(panEvent_);
    }

    panEvent_ = MakeRefPtr<PanEvent>(
        std::move(actionStartTask), std::move(actionUpdateTask), std::move(actionEndTask), std::move(actionCancelTask));
    gestureHub->AddPanEvent(panEvent_, panDirection, 1, DEFAULT_PAN_DISTANCE);
}

void RefreshPattern::InitOnKeyEvent()
{
    if (isKeyEventRegisted_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto focusHub = host->GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    auto onKeyEvent = [wp = WeakClaim(this)](const KeyEvent& event) -> bool {
        auto pattern = wp.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->OnKeyEvent(event);
    };
    isKeyEventRegisted_ = true;
    focusHub->SetOnKeyEventInternal(std::move(onKeyEvent));
}

void RefreshPattern::InitProgressNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    progressChild_ = FrameNode::CreateFrameNode(V2::LOADING_PROGRESS_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LoadingProgressPattern>());
    CHECK_NULL_VOID(progressChild_);
    auto gestureHub = progressChild_->GetEventHub<EventHub>();
    if (gestureHub) {
        gestureHub->SetEnabled(false);
    }
    auto progressLayoutProperty = progressChild_->GetLayoutProperty<LoadingProgressLayoutProperty>();
    CHECK_NULL_VOID(progressLayoutProperty);
    progressLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(LOADING_PROGRESS_SIZE.ConvertToPx()), CalcLength(LOADING_PROGRESS_SIZE.ConvertToPx())));
    auto layoutProperty = GetLayoutProperty<RefreshLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto progressPaintProperty = progressChild_->GetPaintProperty<LoadingProgressPaintProperty>();
    CHECK_NULL_VOID(progressPaintProperty);
    progressPaintProperty->UpdateLoadingProgressOwner(LoadingProgressOwner::REFRESH);
    if (layoutProperty->HasProgressColor()) {
        progressPaintProperty->UpdateColor(layoutProperty->GetProgressColorValue());
    }
    layoutProperty->UpdateAlignment(Alignment::TOP_CENTER);
    host->AddChild(progressChild_, -1);
    if (hasLoadingText_) {
        InitProgressColumn();
    }
    progressChild_->MarkDirtyNode();
}

void RefreshPattern::UpdateLoadingTextOpacity(float opacity)
{
    CHECK_NULL_VOID(loadingTextNode_);
    auto loadingTextRenderContext = loadingTextNode_->GetRenderContext();
    CHECK_NULL_VOID(loadingTextRenderContext);
    if (opacity > 0.0f) {
        opacity = std::clamp(scrollOffset_ - static_cast<float>(LOADING_TEXT_DISPLAY_DISTANCE.ConvertToPx()), 0.0f,
            static_cast<float>(TRIGGER_REFRESH_WITH_TEXT_DISTANCE.ConvertToPx() -
                                         LOADING_TEXT_DISPLAY_DISTANCE.ConvertToPx())) /
                  static_cast<float>(
                      TRIGGER_REFRESH_WITH_TEXT_DISTANCE.ConvertToPx() - LOADING_TEXT_DISPLAY_DISTANCE.ConvertToPx());
    }
    loadingTextRenderContext->UpdateOpacity(opacity);
}

void RefreshPattern::InitProgressColumn()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    columnNode_ = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    loadingTextNode_ = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    auto loadingTextLayoutProperty = loadingTextNode_->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(loadingTextLayoutProperty);
    auto layoutProperty = host->GetLayoutProperty<RefreshLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    loadingTextLayoutProperty->UpdateContent(layoutProperty->GetLoadingTextValue());
    loadingTextLayoutProperty->UpdateMaxLines(1);
    loadingTextLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    auto context = PipelineContext::GetCurrentContextSafely();
    CHECK_NULL_VOID(context);
    auto theme = context->GetTheme<RefreshTheme>();
    CHECK_NULL_VOID(theme);
    loadingTextLayoutProperty->UpdateTextColor(theme->GetTextStyle().GetTextColor());
    loadingTextLayoutProperty->UpdateFontSize(theme->GetTextStyle().GetFontSize());
    
    PaddingProperty textpadding;
    textpadding.top = CalcLength(TRIGGER_LOADING_DISTANCE.ConvertToPx() + LOADING_TEXT_TOP_MARGIN.ConvertToPx());
    auto prop = columnNode_->GetLayoutProperty<LinearLayoutProperty>();
    prop->UpdatePadding(textpadding);
    UpdateLoadingTextOpacity(0.0f);

    columnNode_->AddChild(loadingTextNode_, -1);
    host->AddChild(columnNode_);
}

void RefreshPattern::OnColorConfigurationUpdate()
{
    if (isCustomBuilderExist_) {
        return;
    }
    CHECK_NULL_VOID(progressChild_);
    auto pipeline = PipelineContext::GetCurrentContextSafely();
    CHECK_NULL_VOID(pipeline);
    auto themeManager = pipeline->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto theme = themeManager->GetTheme<RefreshTheme>();
    CHECK_NULL_VOID(theme);
    auto layoutProperty = GetLayoutProperty<RefreshLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto progressPaintProperty = progressChild_->GetPaintProperty<LoadingProgressPaintProperty>();
    CHECK_NULL_VOID(progressPaintProperty);
    if (layoutProperty->HasProgressColor()) {
        progressPaintProperty->UpdateColor(layoutProperty->GetProgressColorValue());
    } else {
        progressPaintProperty->UpdateColor(theme->GetProgressColor());
    }
    if (hasLoadingText_) {
        CHECK_NULL_VOID(loadingTextNode_);
        auto textLayoutProperty = loadingTextNode_->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_VOID(textLayoutProperty);
        textLayoutProperty->UpdateFontSize(theme->GetTextStyle().GetFontSize());
        textLayoutProperty->UpdateTextColor(theme->GetTextStyle().GetTextColor());
    }
}

// the child need to add to be added to the first position in customBuilder mode,
// the child need to add to be added to the last position in loadingProgress mode.
void RefreshPattern::InitChildNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto accessibilityProperty = host->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    auto accessibilityLevel = accessibilityProperty->GetAccessibilityLevel();
    if (isCustomBuilderExist_) {
        if (progressChild_) {
            if (hasLoadingText_) {
                CHECK_NULL_VOID(columnNode_);
                host->RemoveChild(columnNode_);
                columnNode_ = nullptr;
                loadingTextNode_ = nullptr;
            }
            host->RemoveChild(progressChild_);
            progressChild_ = nullptr;
        }
    } else {
        if (!progressChild_) {
            InitProgressNode();
            if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
                auto progressContext = progressChild_->GetRenderContext();
                CHECK_NULL_VOID(progressContext);
                progressContext->UpdateOpacity(0.0);
                UpdateLoadingTextOpacity(0.0f);
            } else {
                UpdateLoadingProgress();
            }
        }
        auto progressAccessibilityProperty = progressChild_->GetAccessibilityProperty<AccessibilityProperty>();
        CHECK_NULL_VOID(progressAccessibilityProperty);
        progressAccessibilityProperty->SetAccessibilityLevel(accessibilityLevel);
    }

    if (hasLoadingText_ && loadingTextNode_) {
        auto loadingTextLayoutProperty = loadingTextNode_->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_VOID(loadingTextLayoutProperty);
        auto layoutProperty = host->GetLayoutProperty<RefreshLayoutProperty>();
        CHECK_NULL_VOID(layoutProperty);
        loadingTextLayoutProperty->UpdateContent(layoutProperty->GetLoadingTextValue());
        loadingTextNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        auto textAccessibilityProperty = loadingTextNode_->GetAccessibilityProperty<AccessibilityProperty>();
        CHECK_NULL_VOID(textAccessibilityProperty);
        textAccessibilityProperty->SetAccessibilityLevel(accessibilityLevel);
    }

    OnColorConfigurationUpdate();
}

void RefreshPattern::RefreshStatusChangeEffect()
{
    auto layoutProperty = GetLayoutProperty<RefreshLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto refreshingProp = layoutProperty->GetIsRefreshing().value_or(false);
    if (isRefreshing_ != refreshingProp) {
        if (refreshingProp) {
            QuickStartFresh();
        } else {
            QuickEndFresh();
        }
    }
}

void RefreshPattern::QuickStartFresh()
{
    UpdateRefreshStatus(RefreshStatus::REFRESH);
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        QuickFirstChildAppear();
        return;
    }

    if (isCustomBuilderExist_) {
        CustomBuilderRefreshingAnimation(false);
    } else {
        LoadingProgressRefreshingAnimation(false);
    }
}

void RefreshPattern::QuickEndFresh()
{
    SwitchToFinish();
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        QuickFirstChildDisappear();
        return;
    }

    if (isCustomBuilderExist_) {
        CustomBuilderExit();
    } else {
        LoadingProgressExit();
    }
}

bool RefreshPattern::OnKeyEvent(const KeyEvent& event)
{
    if (event.code == KeyCode::KEY_F5 || (event.IsCombinationKey() && event.IsCtrlWith(KeyCode::KEY_R))) {
        if (!isRefreshing_) {
            QuickStartFresh();
        }
        return true;
    }
    return false;
}

void RefreshPattern::HandleDragStart(bool isDrag, float mainSpeed)
{
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        isSourceFromAnimation_ = !isDrag;
        ResetAnimation();
    } else {
        HandleDragStartLowVersion();
    }
    // AccessibilityEventType::SCROLL_START
}

ScrollResult RefreshPattern::HandleDragUpdate(float delta, float mainSpeed)
{
    UpdateDragFRCSceneInfo(REFRESH_DRAG_SCENE, mainSpeed, SceneStatus::RUNNING);
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        // If dragging does not expand the refresh, there is no need to continue executing the code
        if (NearZero(scrollOffset_) && NonPositive(delta)) {
            return { delta, true };
        }
        auto pullDownRatio = CalculatePullDownRatio();
        scrollOffset_ = std::clamp(scrollOffset_ + delta * pullDownRatio, 0.0f, MAX_OFFSET);
        if (!isSourceFromAnimation_) {
            FireOnOffsetChange(scrollOffset_);
            if (isRefreshing_) {
                UpdateLoadingProgressStatus(RefreshAnimationState::RECYCLE, GetFollowRatio());
                UpdateFirstChildPlacement();
                return { 0.f, true };
            }
            UpdateLoadingProgressStatus(RefreshAnimationState::FOLLOW_HAND, GetFollowRatio());
            if (LessNotEqual(scrollOffset_, static_cast<float>(refreshOffset_.ConvertToPx())) || !pullToRefresh_) {
                UpdateRefreshStatus(RefreshStatus::DRAG);
            } else {
                UpdateRefreshStatus(RefreshStatus::OVER_DRAG);
            }
        }
        UpdateFirstChildPlacement();
    } else {
        HandleDragUpdateLowVersion(delta);
    }
    return { 0.f, true };
}

void RefreshPattern::HandleDragEnd(float speed)
{
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        SpeedTriggerAnimation(speed);
    } else {
        HandleDragEndLowVersion();
    }
}

void RefreshPattern::HandleDragCancel()
{
    HandleDragEnd(0.0f);
}

float RefreshPattern::CalculatePullDownRatio()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 1.0f);
    auto layoutProperty = GetLayoutProperty<RefreshLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, 1.f);
    if (layoutProperty->GetPullDownRatio().has_value()) {
        return layoutProperty->GetPullDownRatio().value();
    }
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 1.0f);
    auto contentHeight = geometryNode->GetPaddingSize().Height();
    return NearZero(contentHeight) ? 1.0f : ScrollablePattern::CalculateFriction(scrollOffset_ / contentHeight);
}

float RefreshPattern::GetFollowRatio()
{
    auto loadingVisibleHeight = GetLoadingVisibleHeight();
    auto ratio = 0.0f;
    if (!NearEqual(static_cast<float>(refreshOffset_.ConvertToPx()), loadingVisibleHeight)) {
        ratio = static_cast<float>(
            (scrollOffset_ - loadingVisibleHeight) / (refreshOffset_.ConvertToPx() - loadingVisibleHeight));
    }
    return std::clamp(ratio, 0.0f, 1.0f);
}

void RefreshPattern::FireStateChange(int32_t value)
{
    auto refreshEventHub = GetEventHub<RefreshEventHub>();
    CHECK_NULL_VOID(refreshEventHub);
    refreshEventHub->FireOnStateChange(value);
    if (refreshStatus_ == RefreshStatus::REFRESH && Recorder::EventRecorder::Get().IsComponentRecordEnable()) {
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto inspectorId = host->GetInspectorId().value_or("");
        Recorder::EventParamsBuilder builder;
        builder.SetId(inspectorId)
            .SetType(host->GetTag())
            .SetEventType(Recorder::EventType::REFRESH)
            .SetDescription(host->GetAutoEventParamValue(""));
        Recorder::EventRecorder::Get().OnEvent(std::move(builder));
    }
}

void RefreshPattern::FireRefreshing()
{
    auto refreshEventHub = GetEventHub<RefreshEventHub>();
    CHECK_NULL_VOID(refreshEventHub);
    refreshEventHub->FireOnRefreshing();
}

void RefreshPattern::FireChangeEvent(const std::string& value)
{
    auto refreshEventHub = GetEventHub<RefreshEventHub>();
    CHECK_NULL_VOID(refreshEventHub);
    refreshEventHub->FireChangeEvent(value);
}

void RefreshPattern::FireOnOffsetChange(float value)
{
    if (NearZero(value)) {
        value = 0.0f;
    }
    if (!NearEqual(lastScrollOffset_, value)) {
        auto refreshEventHub = GetEventHub<RefreshEventHub>();
        CHECK_NULL_VOID(refreshEventHub);
        refreshEventHub->FireOnOffsetChange(Dimension(value).ConvertToVp());
        lastScrollOffset_ = value;
    }
}

void RefreshPattern::AddCustomBuilderNode(const RefPtr<NG::UINode>& builder)
{
    if (!builder) {
        isCustomBuilderExist_ = false;
        customBuilder_ = nullptr;
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    if (!isCustomBuilderExist_) {
        host->AddChild(builder, 0);
    } else {
        auto customNodeChild = host->GetFirstChild();
        CHECK_NULL_VOID(customNodeChild);
        host->ReplaceChild(customNodeChild, builder);
    }
    customBuilder_ = AceType::DynamicCast<FrameNode>(builder);
    isCustomBuilderExist_ = true;
}

void RefreshPattern::SetAccessibilityAction()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto accessibilityProperty = host->GetAccessibilityProperty<AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    accessibilityProperty->SetActionScrollForward([weakPtr = WeakClaim(this)]() {
        const auto& pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (pattern->IsRefreshing()) {
            return;
        }
        pattern->HandleDragStart(true, 0.0f);
        for (float delta = 0.0f; LessNotEqual(delta, static_cast<float>(MAX_SCROLL_DISTANCE.ConvertToPx()));
             delta += TRIGGER_LOADING_DISTANCE.ConvertToPx()) {
            pattern->HandleDragUpdate(delta, 0.0f);
        }
        pattern->HandleDragEnd(0.0f);
    });
}

void RefreshPattern::InitCoordinationEvent(RefPtr<ScrollableCoordinationEvent>& coordinationEvent)
{
    auto onScrollEvent = [weak = WeakClaim(this)](float offset, float mainSpeed) -> bool {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        pattern->HandleDragUpdate(offset, mainSpeed);
        return Positive(pattern->scrollOffset_) || NonNegative(offset);
    };
    coordinationEvent->SetOnScrollEvent(onScrollEvent);
    auto onScrollStartEvent = [weak = WeakClaim(this)](bool isDrag, float mainSpeed) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragStart(isDrag, mainSpeed);
    };
    coordinationEvent->SetOnScrollStartEvent(onScrollStartEvent);
    auto onScrollEndEvent = [weak = WeakClaim(this)](float speed) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragEnd(speed);
    };
    coordinationEvent->SetOnScrollEndEvent(onScrollEndEvent);
}

void RefreshPattern::UpdateRefreshStatus(RefreshStatus newStatus)
{
    if (refreshStatus_ == newStatus) {
        return;
    }
    refreshStatus_ = newStatus;
    if (refreshStatus_ == RefreshStatus::REFRESH) {
        isRefreshing_ = true;
        // the two-way binding of 'refreshing' variable need to be changed before 'onRefreshing' function is triggered
        FireChangeEvent("true");
        FireRefreshing();
    } else {
        isRefreshing_ = false;
        FireChangeEvent("false");
    }
    FireStateChange(static_cast<int>(refreshStatus_));
    TAG_LOGD(AceLogTag::ACE_REFRESH, "refresh status changed %{public}d", static_cast<int32_t>(refreshStatus_));
}

void RefreshPattern::SwitchToFinish()
{
    if (refreshStatus_ != RefreshStatus::REFRESH && refreshStatus_ != RefreshStatus::DONE) {
        UpdateRefreshStatus(RefreshStatus::INACTIVE);
    } else {
        UpdateRefreshStatus(RefreshStatus::DONE);
    }
}

void RefreshPattern::UpdateLoadingProgressStatus(RefreshAnimationState state, float ratio)
{
    // Need to check loadingProgress mode
    CHECK_NULL_VOID(progressChild_);
    auto progressPaintProperty = progressChild_->GetPaintProperty<LoadingProgressPaintProperty>();
    CHECK_NULL_VOID(progressPaintProperty);
    progressPaintProperty->UpdateRefreshAnimationState(state);
    switch (state) {
        case RefreshAnimationState::FOLLOW_HAND:
        case RefreshAnimationState::RECYCLE:
            progressPaintProperty->UpdateRefreshSizeScaleRatio(ratio);
            break;
        default:
            break;
    }
    if (CheckNeedRender(progressPaintProperty->GetPropertyChangeFlag())) {
        progressChild_->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

void RefreshPattern::InitOffsetProperty()
{
    if (!offsetProperty_) {
        auto propertyCallback = [weak = AceType::WeakClaim(this)](float scrollOffset) {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->scrollOffset_ = scrollOffset;
            pattern->UpdateFirstChildPlacement();
            pattern->FireOnOffsetChange(scrollOffset);
        };
        offsetProperty_ = AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(0.0, std::move(propertyCallback));
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto renderContext = host->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        renderContext->AttachNodeAnimatableProperty(offsetProperty_);
        offsetProperty_->SetPropertyUnit(PropertyUnit::PIXEL_POSITION);
    }
}

void RefreshPattern::UpdateFirstChildPlacement()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto refreshHeight = geometryNode->GetFrameSize().Height();
    auto scrollOffset = std::clamp(scrollOffset_, 0.0f, refreshHeight);
    if (progressChild_) {
        if (isSourceFromAnimation_) {
            UpdateLoadingProgressTranslate(0.0f);
            UpdateScrollTransition(scrollOffset);
        } else {
            UpdateLoadingProgressTranslate(scrollOffset);
            UpdateScrollTransition(scrollOffset);
            UpdateLoadingProgressStatus(GetLoadingProgressStatus(), GetFollowRatio());
        }
    } else {
        UpdateBuilderHeight(scrollOffset);
    }
}

void RefreshPattern::UpdateScrollTransition(float scrollOffset)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    // If the refresh has no children without loadingProgress, it does not need to be offset.
    if (host->TotalChildCount() <= 1) {
        return;
    }
    // Need to search for frameNode and skip ComponentNode
    auto childNode = host->GetFirstChild();
    while (!AceType::InstanceOf<FrameNode>(childNode) && !childNode->GetChildren().empty()) {
        childNode = childNode->GetFirstChild();
    }
    auto scrollableNode = AceType::DynamicCast<FrameNode>(childNode);
    CHECK_NULL_VOID(scrollableNode);
    auto scrollableRenderContext = scrollableNode->GetRenderContext();
    CHECK_NULL_VOID(scrollableRenderContext);
    scrollableRenderContext->UpdateTransformTranslate({ 0.0f, scrollOffset, 0.0f });
}

void RefreshPattern::UpdateBuilderHeight(float builderHeight)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = GetLayoutProperty<RefreshLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    builderMeasureBaseHeight_ = builderHeight;
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void RefreshPattern::UpdateLoadingProgressTranslate(float scrollOffset)
{
    CHECK_NULL_VOID(progressChild_);
    auto renderContext = progressChild_->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto loadingVisibleHeight = GetLoadingVisibleHeight();
    if (GreatOrEqual(scrollOffset, loadingVisibleHeight) &&
        !NearEqual(loadingVisibleHeight, static_cast<float>(GetTriggerRefreshDisTance().ConvertToPx()))) {
        auto ratio = static_cast<float>(
            (scrollOffset - loadingVisibleHeight) / (GetTriggerRefreshDisTance().ConvertToPx() - loadingVisibleHeight));
        renderContext->UpdateOpacity(std::clamp(ratio, 0.0f, 1.0f));
        renderContext->UpdateTransformTranslate({ 0.0f, (scrollOffset - loadingVisibleHeight) * HALF, 0.0f });
        if (loadingTextNode_) {
            UpdateLoadingTextOpacity(std::clamp(ratio, 0.0f, 1.0f));
            auto loadingTextRenderContext = loadingTextNode_->GetRenderContext();
            CHECK_NULL_VOID(loadingTextRenderContext);
            loadingTextRenderContext->UpdateTransformTranslate({ 0.0f,
                scrollOffset_ - TRIGGER_LOADING_DISTANCE.ConvertToPx() - LOADING_PROGRESS_SIZE.ConvertToPx() -
                    LOADING_TEXT_TOP_MARGIN.ConvertToPx(),
                0.0f });
        }
    } else {
        renderContext->UpdateOpacity(0.0f);
        UpdateLoadingTextOpacity(0.0f);
    }
}

float RefreshPattern::GetLoadingVisibleHeight()
{
    float loadingHeight = 0.0f;
    CHECK_NULL_RETURN(progressChild_, 0.0f);
    auto renderContext = progressChild_->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, 0.0f);
    auto geometryNode = progressChild_->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    if (hasLoadingText_) {
        auto loadingTextGeometryNode = loadingTextNode_->GetGeometryNode();
        CHECK_NULL_RETURN(loadingTextGeometryNode, 0.0f);
        loadingHeight = geometryNode->GetFrameSize().Height() + loadingTextGeometryNode->GetFrameSize().Height() +
                        LOADING_TEXT_TOP_MARGIN.ConvertToPx();
    } else {
        loadingHeight = geometryNode->GetFrameSize().Height();
    }
    return (HALF + BASE_SCALE * HALF) * loadingHeight;
}

void RefreshPattern::SpeedTriggerAnimation(float speed)
{
    auto targetOffset = (isSourceFromAnimation_ ||
                            LessNotEqual(scrollOffset_, refreshOffset_.ConvertToPx()) || !pullToRefresh_)
                            ? 0.0f
                            : refreshOffset_.ConvertToPx();
    auto dealSpeed = 0.0f;
    if (!NearEqual(scrollOffset_, targetOffset)) {
        dealSpeed = speed / (targetOffset - scrollOffset_);
    }
    bool recycle = true;
    if (!isSourceFromAnimation_ && refreshStatus_ == RefreshStatus::OVER_DRAG) {
        UpdateRefreshStatus(RefreshStatus::REFRESH);
        UpdateLoadingProgressStatus(RefreshAnimationState::FOLLOW_TO_RECYCLE, GetFollowRatio());
    } else if (NearZero(targetOffset)) {
        recycle = false;
        SwitchToFinish();
    }
    ResetAnimation();
    AnimationOption option;
    auto curve = AceType::MakeRefPtr<InterpolatingSpring>(dealSpeed, 1.0f, 228.0f, 30.0f);
    option.SetCurve(curve);
    animation_ = AnimationUtils::StartAnimation(
        option,
        [&, weak = AceType::WeakClaim(this)]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->offsetProperty_->Set(targetOffset);
        },
        [weak = AceType::WeakClaim(this), recycle]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            if (recycle) {
                pattern->UpdateLoadingProgressStatus(RefreshAnimationState::RECYCLE, pattern->GetFollowRatio());
            } else {
                pattern->UpdateLoadingProgressStatus(RefreshAnimationState::FOLLOW_HAND, pattern->GetFollowRatio());
            }
        });
    auto context = PipelineContext::GetCurrentContextSafely();
    CHECK_NULL_VOID(context);
    context->RequestFrame();
}

float RefreshPattern::GetTargetOffset()
{
    if (isSourceFromAnimation_) {
        return 0.0f;
    }
    auto targetOffset = 0.0f;
    switch (refreshStatus_) {
        case RefreshStatus::OVER_DRAG:
        case RefreshStatus::REFRESH:
            targetOffset = refreshOffset_.ConvertToPx();
            break;
        default:
            targetOffset = 0.0f;
            break;
    }
    return targetOffset;
}

void RefreshPattern::SpeedAnimationFinish()
{
    if (isRefreshing_) {
        UpdateLoadingProgressStatus(RefreshAnimationState::RECYCLE, GetFollowRatio());
    } else {
        UpdateLoadingProgressStatus(RefreshAnimationState::FOLLOW_HAND, GetFollowRatio());
    }
}

void RefreshPattern::QuickFirstChildAppear()
{
    isSourceFromAnimation_ = false;
    UpdateLoadingProgressStatus(RefreshAnimationState::RECYCLE, GetFollowRatio());
    ResetAnimation();
    AnimationOption option;
    option.SetCurve(DEFAULT_CURVE);
    option.SetDuration(LOADING_ANIMATION_DURATION);
    animation_ = AnimationUtils::StartAnimation(
        option, [&]() { offsetProperty_->Set(static_cast<float>(refreshOffset_.ConvertToPx())); });
}

void RefreshPattern::QuickFirstChildDisappear()
{
    ResetAnimation();
    AnimationOption option;
    option.SetCurve(DEFAULT_CURVE);
    option.SetDuration(LOADING_ANIMATION_DURATION);
    animation_ = AnimationUtils::StartAnimation(
        option, [&]() { offsetProperty_->Set(0.0f); },
        [weak = AceType::WeakClaim(this)]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->SpeedAnimationFinish();
        });
}

RefreshAnimationState RefreshPattern::GetLoadingProgressStatus()
{
    auto defaultValue = RefreshAnimationState::FOLLOW_HAND;
    CHECK_NULL_RETURN(progressChild_, defaultValue);
    auto progressPaintProperty = progressChild_->GetPaintProperty<LoadingProgressPaintProperty>();
    CHECK_NULL_RETURN(progressPaintProperty, defaultValue);
    return progressPaintProperty->GetRefreshAnimationState().value_or(defaultValue);
}

void RefreshPattern::ResetAnimation()
{
    float currentOffset = scrollOffset_;
    AnimationUtils::StopAnimation(animation_);
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        CHECK_NULL_VOID(offsetProperty_);
        offsetProperty_->Set(currentOffset);
    } else {
        lowVersionOffset_->Set(currentOffset);
    }
}

void RefreshPattern::UpdateDragFRCSceneInfo(const std::string& scene, float speed, SceneStatus sceneStatus)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->AddFRCSceneInfo(scene, std::abs(speed), sceneStatus);
}

void RefreshPattern::InitLowVersionOffset()
{
    if (!lowVersionOffset_) {
        auto propertyCallback = [weak = AceType::WeakClaim(this)](float scrollOffset) {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->scrollOffset_ = scrollOffset;
            pattern->UpdateChild();
        };
        lowVersionOffset_ = AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(0.0, std::move(propertyCallback));
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto renderContext = host->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        renderContext->AttachNodeAnimatableProperty(lowVersionOffset_);
    }
}

void RefreshPattern::UpdateChild()
{
    if (customBuilder_) {
        UpdateCustomBuilderProperty();
    } else {
        UpdateLoadingProgress();
    }
}

void RefreshPattern::HandleDragStartLowVersion()
{
    if (isRefreshing_) {
        return;
    }
    scrollOffset_ = 0.0f;
    UpdateLoadingProgressStatus(RefreshAnimationState::FOLLOW_HAND, 0.0f);
}

void RefreshPattern::HandleDragUpdateLowVersion(float delta)
{
    if (isRefreshing_) {
        return;
    }
    scrollOffset_ = GetScrollOffset(delta);
    if (LessNotEqual(scrollOffset_, static_cast<float>(GetTriggerRefreshDisTance().ConvertToPx()))) {
        UpdateRefreshStatus(RefreshStatus::DRAG);
    } else {
        UpdateRefreshStatus(RefreshStatus::OVER_DRAG);
    }
    if (customBuilder_) {
        HandleCustomBuilderDragUpdateStage();
        return;
    }
    UpdateLoadingProgress();
    if (GreatNotEqual(scrollOffset_, triggerLoadingDistance_)) {
        CHECK_NULL_VOID(progressChild_);
        auto progressPaintProperty = progressChild_->GetPaintProperty<LoadingProgressPaintProperty>();
        CHECK_NULL_VOID(progressPaintProperty);
        float triggerRefreshDistance = GetTriggerRefreshDisTance().ConvertToPx();
        float ratio =
            NearEqual(triggerRefreshDistance, triggerLoadingDistance_)
                ? 1.0f
                : (scrollOffset_ - triggerLoadingDistance_) / (triggerRefreshDistance - triggerLoadingDistance_);
        progressPaintProperty->UpdateRefreshSizeScaleRatio(std::clamp(ratio, 0.0f, 1.0f));
    }
}

void RefreshPattern::HandleDragEndLowVersion()
{
    if (isRefreshing_) {
        return;
    }
    if (customBuilder_) {
        HandleCustomBuilderDragEndStage();
        return;
    }
    if (refreshStatus_ == RefreshStatus::OVER_DRAG) {
        UpdateRefreshStatus(RefreshStatus::REFRESH);
        LoadingProgressRefreshingAnimation(true);
    } else {
        SwitchToFinish();
        LoadingProgressExit();
    }
    // AccessibilityEventType::SCROLL_END
}

void RefreshPattern::LoadingProgressRefreshingAnimation(bool isDrag)
{
    UpdateLoadingProgressStatus(RefreshAnimationState::RECYCLE, 1.0f);
    ResetAnimation();
    AnimationOption option;
    if (isDrag) {
        option.SetCurve(AceType::MakeRefPtr<SpringCurve>(0.0f, 1.0f, 228.0f, 30.0f));
        option.SetDuration(FOLLOW_TO_RECYCLE_DURATION);
    } else {
        option.SetCurve(DEFAULT_CURVE);
        option.SetDuration(LOADING_ANIMATION_DURATION);
    }
    animation_ = AnimationUtils::StartAnimation(
        option, [&]() { lowVersionOffset_->Set(GetTriggerRefreshDisTance().ConvertToPx()); });
}

void RefreshPattern::LoadingProgressExit()
{
    ResetAnimation();
    AnimationOption option;
    option.SetCurve(DEFAULT_CURVE);
    option.SetDuration(LOADING_ANIMATION_DURATION);
    animation_ = AnimationUtils::StartAnimation(
        option, [&]() { lowVersionOffset_->Set(0.0f); },
        [weak = AceType::WeakClaim(this)]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->UpdateLoadingProgressStatus(RefreshAnimationState::FOLLOW_HAND, 0.0f);
        });
}

void RefreshPattern::UpdateLoadingProgress()
{
    float loadingProgressOffset =
        std::clamp(scrollOffset_, triggerLoadingDistance_, static_cast<float>(MAX_SCROLL_DISTANCE.ConvertToPx()));
    UpdateLoadingMarginTop(loadingProgressOffset);
    float triggerRefreshDistance = GetTriggerRefreshDisTance().ConvertToPx();
    float ratio = NearEqual(triggerRefreshDistance, triggerLoadingDistance_)
                      ? 1.0f
                      : (loadingProgressOffset - triggerLoadingDistance_) /
                            (GetTriggerRefreshDisTance().ConvertToPx() - triggerLoadingDistance_);
    auto progressPaintProperty = progressChild_->GetPaintProperty<LoadingProgressPaintProperty>();
    CHECK_NULL_VOID(progressPaintProperty);
    progressPaintProperty->UpdateRefreshSizeScaleRatio(ratio);
    auto progressContext = progressChild_->GetRenderContext();
    CHECK_NULL_VOID(progressContext);
    progressContext->UpdateOpacity(std::clamp(ratio, 0.0f, 1.0f));
    UpdateLoadingTextOpacity(std::clamp(ratio, 0.0f, 1.0f));
    progressChild_->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
}

void RefreshPattern::CustomBuilderRefreshingAnimation(bool isDrag)
{
    ResetAnimation();
    AnimationOption option;
    if (isDrag) {
        option.SetCurve(AceType::MakeRefPtr<SpringCurve>(0.0f, 1.0f, 228.0f, 30.0f));
        option.SetDuration(FOLLOW_TO_RECYCLE_DURATION);
    } else {
        option.SetCurve(DEFAULT_CURVE);
        option.SetDuration(CUSTOM_BUILDER_ANIMATION_DURATION);
    }
    animation_ = AnimationUtils::StartAnimation(
        option, [&]() { lowVersionOffset_->Set(GetTriggerRefreshDisTance().ConvertToPx()); });
}

void RefreshPattern::CustomBuilderExit()
{
    ResetAnimation();
    AnimationOption option;
    option.SetDuration(CUSTOM_BUILDER_ANIMATION_DURATION);
    option.SetCurve(DEFAULT_CURVE);
    animation_ = AnimationUtils::StartAnimation(option, [&]() { lowVersionOffset_->Set(0.0f); });
}

void RefreshPattern::UpdateCustomBuilderProperty()
{
    auto customBuilderSize = customBuilder_->GetGeometryNode()->GetFrameSize();
    auto maxScroll = static_cast<float>(MAX_SCROLL_DISTANCE.ConvertToPx());
    customBuilderOffset_ = std::clamp(scrollOffset_, triggerLoadingDistance_, maxScroll - customBuilderSize.Height());
    float triggerRefreshDistance = GetTriggerRefreshDisTance().ConvertToPx();
    float ratio = NearEqual(triggerRefreshDistance, triggerLoadingDistance_)
                      ? 1.0f
                      : (customBuilderOffset_ - triggerLoadingDistance_) /
                            (GetTriggerRefreshDisTance().ConvertToPx() - triggerLoadingDistance_);
    auto customBuilderContext = customBuilder_->GetRenderContext();
    CHECK_NULL_VOID(customBuilderContext);
    customBuilderContext->UpdateOpacity(std::clamp(ratio, 0.0f, 1.0f));
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
}

void RefreshPattern::HandleCustomBuilderDragUpdateStage()
{
    auto customBuilderSize = customBuilder_->GetGeometryNode()->GetMarginFrameSize();
    auto maxScroll = MAX_SCROLL_DISTANCE.ConvertToPx();
    if (NearZero(static_cast<double>(customBuilder_->GetGeometryNode()->GetMarginFrameSize().Height()))) {
        return;
    }
    if (LessNotEqual(static_cast<double>(maxScroll - customBuilderSize.Height()),
            static_cast<double>(triggerLoadingDistance_))) {
        return;
    }
    UpdateCustomBuilderProperty();
}

void RefreshPattern::HandleCustomBuilderDragEndStage()
{
    if (refreshStatus_ == RefreshStatus::OVER_DRAG) {
        UpdateRefreshStatus(RefreshStatus::REFRESH);
        CustomBuilderRefreshingAnimation(true);
    } else {
        SwitchToFinish();
        CustomBuilderExit();
    }
}

void RefreshPattern::UpdateLoadingMarginTop(float top)
{
    CHECK_NULL_VOID(progressChild_);
    auto progressLayoutProperty = progressChild_->GetLayoutProperty<LoadingProgressLayoutProperty>();
    CHECK_NULL_VOID(progressLayoutProperty);
    MarginProperty marginProperty;
    marginProperty.left = CalcLength(0.0f);
    marginProperty.right = CalcLength(0.0f);
    marginProperty.bottom = CalcLength(0.0f);
    marginProperty.top = CalcLength(top);
    progressLayoutProperty->UpdateMargin(marginProperty);
}

float RefreshPattern::GetScrollOffset(float delta)
{
    auto layoutProperty = GetLayoutProperty<RefreshLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, 0.0f);
    auto frictionRatio = static_cast<float>(layoutProperty->GetFriction().value_or(DEFAULT_FRICTION)) * PERCENT;
    auto scrollY = delta * frictionRatio;
    return std::clamp(scrollOffset_ + scrollY, 0.0f, static_cast<float>(MAX_SCROLL_DISTANCE.ConvertToPx()));
}

ScrollResult RefreshPattern::HandleScroll(float offset, int32_t source, NestedState state, float velocity)
{
    ScrollResult result = { offset, true };
    auto nestedScroll = GetNestedScroll();
    if (NearZero(offset)) {
        return result;
    }
    auto parent = GetNestedScrollParent();
    if (state == NestedState::CHILD_SCROLL) {
        if (Negative(offset) && Positive(scrollOffset_)) {
            if (parent && nestedScroll.forward == NestedScrollMode::PARENT_FIRST) {
                result = parent->HandleScroll(offset, source, NestedState::CHILD_SCROLL, velocity);
                result = HandleDragUpdate(result.remain, velocity);
            } else if (parent && nestedScroll.forward == NestedScrollMode::SELF_FIRST) {
                result = HandleDragUpdate(offset, velocity);
                result = parent->HandleScroll(result.remain, source, NestedState::CHILD_SCROLL, velocity);
            } else {
                result = HandleDragUpdate(offset, velocity);
            }
        } else {
            if (!parent || ((Negative(offset) && (nestedScroll.forward == NestedScrollMode::SELF_ONLY ||
                                                     nestedScroll.forward == NestedScrollMode::PARALLEL)) ||
                               (Positive(offset) && (nestedScroll.backward == NestedScrollMode::SELF_ONLY ||
                                                        nestedScroll.backward == NestedScrollMode::PARALLEL)))) {
                return result;
            } else {
                result = parent->HandleScroll(offset, source, NestedState::CHILD_SCROLL, velocity);
            }
        }
        return result;
    } else if (state == NestedState::CHILD_OVER_SCROLL) {
        if (parent && ((Negative(offset) && nestedScroll.forward == NestedScrollMode::SELF_FIRST) ||
                          (Positive(offset) && nestedScroll.backward == NestedScrollMode::SELF_FIRST))) {
            result = parent->HandleScroll(offset, source, NestedState::CHILD_OVER_SCROLL, velocity);
            if (!NearZero(result.remain)) {
                result = HandleDragUpdate(result.remain, velocity);
            }
            return { 0.f, true };
        } else {
            result = HandleDragUpdate(offset, velocity);
        }
    }
    return result;
}

void RefreshPattern::OnScrollStartRecursive(float position, float velocity)
{
    SetIsNestedInterrupt(false);
    if (!GetIsFixedNestedScrollMode()) {
        SetParentScrollable();
    }
    auto nestedScroll = GetNestedScroll();
    HandleDragStart(true, velocity);
    auto parent = GetNestedScrollParent();
    if (parent && nestedScroll.NeedParent() &&
        (nestedScroll.forward != NestedScrollMode::PARALLEL || nestedScroll.backward != NestedScrollMode::PARALLEL)) {
        parent->OnScrollStartRecursive(position, velocity);
    }
}

bool RefreshPattern::HandleScrollVelocity(float velocity)
{
    auto parent = GetNestedScrollParent();
    auto nestedScroll = GetNestedScroll();
    bool result = false;
    if (parent && ((Negative(velocity) && nestedScroll.forward == NestedScrollMode::PARENT_FIRST) ||
                      (Positive(velocity) && nestedScroll.backward == NestedScrollMode::PARENT_FIRST))) {
        result = parent->HandleScrollVelocity(velocity);
        if (result) {
            return true;
        }
    }
    if (Positive(scrollOffset_) || Positive(velocity)) {
        HandleDragEnd(velocity);
        result = true;
    } else if (parent && ((Negative(velocity) && nestedScroll.forward == NestedScrollMode::SELF_FIRST) ||
                             (Positive(velocity) && nestedScroll.backward == NestedScrollMode::SELF_FIRST))) {
        result = parent->HandleScrollVelocity(velocity);
    }
    return result;
}

void RefreshPattern::OnScrollEndRecursive(const std::optional<float>& velocity)
{
    HandleDragEnd(velocity.value_or(0.f));
    auto parent = GetNestedScrollParent();
    auto nestedScroll = GetNestedScroll();
    if (parent && (nestedScroll.NeedParent() || GetIsNestedInterrupt())) {
        parent->OnScrollEndRecursive(velocity);
    }
    SetIsNestedInterrupt(false);
}

void RefreshPattern::DumpInfo()
{
    DumpLog::GetInstance().AddDesc(
        std::string("RefreshStatus: ").append(std::to_string(static_cast<int32_t>(refreshStatus_))));
}
} // namespace OHOS::Ace::NG
