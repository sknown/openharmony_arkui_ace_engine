/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWITCH_SWITCH_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWITCH_SWITCH_PATTERN_H

#include "base/geometry/axis.h"
#include "base/geometry/size.h"
#include "base/memory/referenced.h"
#include "core/components/checkable/checkable_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/toggle/switch_accessibility_property.h"
#include "core/components_ng/pattern/toggle/switch_event_hub.h"
#include "core/components_ng/pattern/toggle/switch_layout_algorithm.h"
#include "core/components_ng/pattern/toggle/switch_paint_method.h"
#include "core/components_ng/pattern/toggle/switch_paint_property.h"
#include "core/components_ng/pattern/toggle/toggle_model_ng.h"

namespace OHOS::Ace::NG {

class SwitchPattern : public Pattern {
    DECLARE_ACE_TYPE(SwitchPattern, Pattern);

public:
    SwitchPattern() = default;

    ~SwitchPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<SwitchEventHub>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<SwitchLayoutAlgorithm>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<SwitchPaintProperty>();
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        if (UseContentModifier()) {
            return nullptr;
        }
        auto host = GetHost();
        CHECK_NULL_RETURN(host, nullptr);
        if (!switchModifier_) {
            auto pipeline = PipelineBase::GetCurrentContext();
            auto switchTheme = pipeline->GetTheme<SwitchTheme>();
            auto paintProperty = host->GetPaintProperty<SwitchPaintProperty>();
            auto isSelect = paintProperty->GetIsOnValue(false);
            auto boardColor = isSelect ? paintProperty->GetSelectedColorValue(switchTheme->GetActiveColor())
                                       : switchTheme->GetInactivePointColor();
            switchModifier_ = AceType::MakeRefPtr<SwitchModifier>(isSelect, boardColor, dragOffsetX_);
        }
        auto paintMethod = MakeRefPtr<SwitchPaintMethod>(switchModifier_);
        paintMethod->SetDirection(direction_);
        paintMethod->SetIsSelect(isOn_.value_or(false));
        paintMethod->SetEnabled(enabled_);
        paintMethod->SetDragOffsetX(dragOffsetX_);
        paintMethod->SetTouchHoverAnimationType(touchHoverType_);
        paintMethod->SetIsDragEvent(isDragEvent_);
        paintMethod->SetShowHoverEffect(showHoverEffect_);
        return paintMethod;
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<SwitchAccessibilityProperty>();
    }

    FocusPattern GetFocusPattern() const override
    {
        FocusPaintParam focusPaintParams;

        auto pipelineContext = PipelineBase::GetCurrentContext();
        CHECK_NULL_RETURN(pipelineContext, FocusPattern());
        auto switchTheme = pipelineContext->GetTheme<SwitchTheme>();
        CHECK_NULL_RETURN(switchTheme, FocusPattern());
        auto focusPaintcolor = switchTheme->GetActiveColor();
        focusPaintParams.SetPaintColor(focusPaintcolor);
        focusPaintParams.SetFocusPadding(Dimension(2.0_vp));

        return { FocusType::NODE, true, FocusStyleType::CUSTOM_REGION, focusPaintParams };
    }

    bool IsChecked()
    {
        return isOn_.value_or(false);
    }

    void MarkIsSelected(bool isSelected);

    void SetIsUserSetResponseRegion(bool isUserSetResponseRegion)
    {
        isUserSetResponseRegion_ = isUserSetResponseRegion;
    }

    void SetShowHoverEffect(bool showHoverEffect)
    {
        showHoverEffect_ = showHoverEffect;
    }

    std::string ProvideRestoreInfo() override;

    void OnRestoreInfo(const std::string& restoreInfo) override;
    void OnColorConfigurationUpdate() override;
    void SetBuilderFunc(SwitchMakeCallback&& makeFunc)
    {
        makeFunc_ = std::move(makeFunc);
    }

    bool UseContentModifier()
    {
        return contentModifierNode_ != nullptr;
    }

    void SetSwitchIsOn(bool value);

private:
    void OnModifyDone() override;
    void SetAccessibilityAction();
    void UpdateSelectStatus(bool isSelected);
    void OnAfterModifyDone() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, bool skipMeasure, bool skipLayout) override;
    RefPtr<Curve> GetCurve() const;
    int32_t GetDuration() const;
    void UpdateChangeEvent() const;
    void OnChange();
    void OnTouchDown();
    void OnTouchUp();
    void HandleMouseEvent(bool isHover);
    void HandleFocusEvent(const RefPtr<SwitchPaintProperty>& switchPaintProperty,
        const RefPtr<SwitchTheme>& switchTheme);
    void HandleBlurEvent(const RefPtr<SwitchPaintProperty>& switchPaintProperty,
        const RefPtr<SwitchTheme>& switchTheme);
    float GetSwitchWidth() const;
    float GetSwitchContentOffsetX() const;

    // Init pan recognizer to move items when drag update, play translate animation when drag end.
    void InitPanEvent(const RefPtr<GestureEventHub>& gestureHub);
    void InitClickEvent();
    void InitTouchEvent();
    void InitMouseEvent();
    void InitFocusEvent();

    // Init key event
    void InitOnKeyEvent(const RefPtr<FocusHub>& focusHub);
    void GetInnerFocusPaintRect(RoundRect& paintRect);

    void HandleDragStart();
    void HandleDragUpdate(const GestureEvent& info);
    void HandleDragEnd();

    bool IsOutOfBoundary(double mainOffset) const;
    void OnClick();
    void AddHotZoneRect();
    void RemoveLastHotZoneRect() const;
    void UpdateSwitchPaintProperty();
    void UpdateSwitchLayoutProperty();
    void FireBuilder();

    RefPtr<FrameNode> BuildContentModifierNode();
    std::optional<SwitchMakeCallback> makeFunc_;
    RefPtr<FrameNode> contentModifierNode_;

    RefPtr<PanEvent> panEvent_;

    RefPtr<ClickEvent> clickListener_;
    std::optional<bool> isOn_;
    float currentOffset_ = 0.0f;
    float dragOffsetX_ = 0.0f;

    RefPtr<TouchEventImpl> touchListener_;
    RefPtr<InputEvent> mouseEvent_;
    bool isTouch_ = false;
    bool isHover_ = false;
    bool isUserSetResponseRegion_ = false;
    bool showHoverEffect_ = true;
    bool enabled_ = true;
    bool isBgColorUnselectFocus_ = false;
    bool isPointColorUnselectFocus_ = false;

    float width_ = 0.0f;
    float height_ = 0.0f;
    Dimension hotZoneHorizontalPadding_;
    Dimension hotZoneVerticalPadding_;
    OffsetF offset_;
    SizeF size_;
    OffsetF hotZoneOffset_;
    SizeF hotZoneSize_;
    TouchHoverAnimationType touchHoverType_ = TouchHoverAnimationType::NONE;
    TextDirection direction_ = TextDirection::AUTO;
    bool isDragEvent_ = false;
    RefPtr<SwitchModifier> switchModifier_;
    ACE_DISALLOW_COPY_AND_MOVE(SwitchPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWITCH_SWITCH_PATTERN_H
