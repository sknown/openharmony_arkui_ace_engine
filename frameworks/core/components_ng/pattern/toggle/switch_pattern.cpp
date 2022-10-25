/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/toggle/switch_pattern.h"

#include <cmath>
#include <cstdint>

#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/animation/curve.h"
#include "core/animation/curves.h"
#include "core/common/container.h"
#include "core/components/checkable/checkable_theme.h"
#include "core/components_ng/pattern/toggle/switch_layout_algorithm.h"
#include "core/components_ng/pattern/toggle/switch_paint_property.h"
#include "core/components_ng/property/property.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
void SwitchPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToFrame(true);
}

bool SwitchPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, bool skipMeasure, bool skipLayout)
{
    if (skipMeasure || dirty->SkipMeasureContent()) {
        return false;
    }
    if (isOn_.value()) {
        currentOffset_ = GetSwitchWidth();
    }
    return true;
}

void SwitchPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    auto pipeline = host->GetContext();
    CHECK_NULL_VOID(pipeline);
    auto switchTheme = pipeline->GetTheme<SwitchTheme>();
    CHECK_NULL_VOID(switchTheme);
    auto layoutProperty = host->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);

    if (!layoutProperty->GetMarginProperty()) {
        MarginProperty margin;
        margin.left = CalcLength(switchTheme->GetHotZoneHorizontalPadding().Value());
        margin.right = CalcLength(switchTheme->GetHotZoneHorizontalPadding().Value());
        margin.top = CalcLength(switchTheme->GetHotZoneVerticalPadding().Value());
        margin.bottom = CalcLength(switchTheme->GetHotZoneVerticalPadding().Value());
        layoutProperty->UpdateMargin(margin);
    }
    if (layoutProperty->GetPositionProperty()) {
        layoutProperty->UpdateAlignment(
            layoutProperty->GetPositionProperty()->GetAlignment().value_or(Alignment::CENTER));
    } else {
        layoutProperty->UpdateAlignment(Alignment::CENTER);
    }
    auto switchPaintProperty = host->GetPaintProperty<SwitchPaintProperty>();
    CHECK_NULL_VOID(switchPaintProperty);
    if (!isOn_.has_value()) {
        isOn_ = switchPaintProperty->GetIsOnValue();
    }
    auto isOn = switchPaintProperty->GetIsOnValue();
    if (isOn != isOn_.value()) {
        OnChange();
    }
    if (clickListener_) {
        return;
    }
    auto gesture = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gesture);
    auto clickCallback = [weak = WeakClaim(this)](GestureEvent& info) {
        auto switchPattern = weak.Upgrade();
        CHECK_NULL_VOID(switchPattern);
        switchPattern->OnClick();
    };

    clickListener_ = MakeRefPtr<ClickEvent>(std::move(clickCallback));
    gesture->AddClickEvent(clickListener_);
    InitPanEvent(gestureHub);
}

void SwitchPattern::UpdateCurrentOffset(float offset)
{
    currentOffset_ = currentOffset_ + offset;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SwitchPattern::PlayTranslateAnimation(float startPos, float endPos)
{
    LOGI("Play translate animation startPos: %{public}lf, endPos: %{public}lf", startPos, endPos);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto curve = GetCurve();
    if (!curve) {
        curve = Curves::LINEAR;
    }

    // If animation is still running, stop it before play new animation.
    StopTranslateAnimation();

    auto translate = AceType::MakeRefPtr<CurveAnimation<double>>(startPos, endPos, curve);
    auto weak = AceType::WeakClaim(this);
    translate->AddListener(Animation<double>::ValueCallback([weak, startPos, endPos](double value) {
        auto switch_ = weak.Upgrade();
        CHECK_NULL_VOID(switch_);
        if (!NearEqual(value, startPos) && !NearEqual(value, endPos) && !NearEqual(startPos, endPos)) {
            float moveRate =
                Curves::EASE_OUT->MoveInternal(static_cast<float>((value - startPos) / (endPos - startPos)));
            value = startPos + (endPos - startPos) * moveRate;
        }
        switch_->UpdateCurrentOffset(static_cast<float>(value - switch_->currentOffset_));
    }));

    if (!controller_) {
        controller_ = AceType::MakeRefPtr<Animator>(host->GetContext());
    }
    controller_->ClearStopListeners();
    controller_->ClearInterpolators();
    controller_->AddStopListener([weak, this]() {
        auto switch_ = weak.Upgrade();
        CHECK_NULL_VOID(switch_);
        if (!isOn_.value()) {
            if (NearEqual(switch_->currentOffset_, GetSwitchWidth()) && changeFlag_) {
                switch_->isOn_ = true;
                switch_->UpdateChangeEvent();
            }
        } else {
            if (NearEqual(switch_->currentOffset_, 0) && changeFlag_) {
                switch_->isOn_ = false;
                switch_->UpdateChangeEvent();
            }
        }
    });
    controller_->SetDuration(GetDuration());
    controller_->AddInterpolator(translate);
    controller_->Play();
}

RefPtr<Curve> SwitchPattern::GetCurve() const
{
    auto switchPaintProperty = GetPaintProperty<SwitchPaintProperty>();
    CHECK_NULL_RETURN(switchPaintProperty, nullptr);
    return switchPaintProperty->GetCurve().value_or(nullptr);
}

int32_t SwitchPattern::GetDuration() const
{
    const int32_t DEFAULT_DURATION = 400;
    auto switchPaintProperty = GetPaintProperty<SwitchPaintProperty>();
    CHECK_NULL_RETURN(switchPaintProperty, DEFAULT_DURATION);
    return switchPaintProperty->GetDuration().value_or(DEFAULT_DURATION);
}

void SwitchPattern::StopTranslateAnimation()
{
    if (controller_ && !controller_->IsStopped()) {
        controller_->Stop();
    }
}

void SwitchPattern::OnChange()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto translateOffset = GetSwitchWidth();
    StopTranslateAnimation();
    changeFlag_ = true;
    if (!isOn_.value()) {
        PlayTranslateAnimation(0, translateOffset);
    } else {
        PlayTranslateAnimation(translateOffset, 0);
    }
}

float SwitchPattern::GetSwitchWidth() const
{
    const float switchGap = 2.0f;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, false);
    auto switchWidth = geometryNode->GetContentSize().Width() - geometryNode->GetContentSize().Height() + switchGap;
    return switchWidth;
}

void SwitchPattern::UpdateChangeEvent() const
{
    auto switchEventHub = GetEventHub<SwitchEventHub>();
    CHECK_NULL_VOID(switchEventHub);
    switchEventHub->UpdateChangeEvent(isOn_.value());
}

void SwitchPattern::OnClick()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (controller_ && !controller_->IsStopped()) {
        // Clear stop listener before stop, otherwise the previous swipe will be considered complete.
        controller_->ClearStopListeners();
        controller_->Stop();
    }
    OnChange();
}

void SwitchPattern::InitPanEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (panEvent_) {
        return;
    }

    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        LOGD("Pan event start");
        auto pattern = weak.Upgrade();
        if (pattern) {
            if (info.GetInputEventType() == InputEventType::AXIS) {
                return;
            }
        }
    };

    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleDragUpdate(info);
        }
    };

    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        LOGD("Pan event end mainVelocity: %{public}lf", info.GetMainVelocity());
        auto pattern = weak.Upgrade();
        if (pattern) {
            if (info.GetInputEventType() == InputEventType::AXIS) {
                return;
            }
            pattern->HandleDragEnd();
        }
    };

    auto actionCancelTask = [weak = WeakClaim(this)]() {
        LOGD("Pan event cancel");
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleDragEnd();
        }
    };

    PanDirection panDirection;
    panDirection.type = PanDirection::HORIZONTAL;

    float distance = static_cast<float>(Dimension(DEFAULT_PAN_DISTANCE, DimensionUnit::VP).ConvertToPx());
    panEvent_ = MakeRefPtr<PanEvent>(
        std::move(actionStartTask), std::move(actionUpdateTask), std::move(actionEndTask), std::move(actionCancelTask));
    gestureHub->AddPanEvent(panEvent_, panDirection, 1, distance);
}

void SwitchPattern::HandleDragUpdate(const GestureEvent& info)
{
    auto mainDelta = static_cast<float>(info.GetMainDelta());
    auto isOutOfBoundary = IsOutOfBoundary(mainDelta + currentOffset_);
    if (isOutOfBoundary) {
        LOGD("Switch has reached boundary, can't drag any more.");
        return;
    }
    UpdateCurrentOffset(static_cast<float>(mainDelta));
}

void SwitchPattern::HandleDragEnd()
{
    LOGD("Drag end currentOffset: %{public}lf", currentOffset_);
    // Play translate animation.
    auto mainSize = GetSwitchWidth();
    if (std::abs(currentOffset_) >= mainSize / 2) {
        if (!isOn_.value()) {
            changeFlag_ = true;
            PlayTranslateAnimation(mainSize, mainSize);
        } else {
            changeFlag_ = false;
            PlayTranslateAnimation(currentOffset_, mainSize);
        }
    } else if (std::abs(currentOffset_) < mainSize / 2) {
        if (isOn_.value()) {
            changeFlag_ = true;
            PlayTranslateAnimation(0.0f, 0.0f);
        } else {
            changeFlag_ = false;
            PlayTranslateAnimation(currentOffset_, 0.0f);
        }
    }
}

bool SwitchPattern::IsOutOfBoundary(double mainOffset) const
{
    return mainOffset < 0 || mainOffset > GetSwitchWidth();
}

} // namespace OHOS::Ace::NG