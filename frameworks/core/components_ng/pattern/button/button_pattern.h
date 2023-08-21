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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_BUTTON_BUTTON_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_BUTTON_BUTTON_PATTERN_H

#include <optional>

#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components/button/button_theme.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/pattern/button/button_event_hub.h"
#include "core/components_ng/pattern/button/button_layout_algorithm.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
namespace OHOS::Ace::NG {
enum class ComponentButtonType { POPUP, BUTTON, STEPPER, NAVIGATION };
class ButtonPattern : public Pattern {
    DECLARE_ACE_TYPE(ButtonPattern, Pattern);

public:
    ButtonPattern() = default;

    ~ButtonPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<ButtonEventHub>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<ButtonLayoutAlgorithm>();
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<ButtonLayoutProperty>();
    }

    FocusPattern GetFocusPattern() const override
    {
        if (buttonType_ == ComponentButtonType::POPUP || buttonType_ == ComponentButtonType::STEPPER) {
            FocusPaintParam focusPaintParam;
            focusPaintParam.SetPaintColor(focusBorderColor_);
            return { FocusType::NODE, true, FocusStyleType::INNER_BORDER, focusPaintParam };
        }
        if (buttonType_ == ComponentButtonType::NAVIGATION) {
            FocusPaintParam focusPaintParam;
            focusPaintParam.SetPaintColor(focusBorderColor_);
            focusPaintParam.SetPaintWidth(focusBorderWidth_);
            return { FocusType::NODE, true, FocusStyleType::INNER_BORDER, focusPaintParam };
        }
        return { FocusType::NODE, true, FocusStyleType::OUTER_BORDER };
    }

    bool IsNeedAdjustByAspectRatio() override
    {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, false);
        auto layoutProperty = host->GetLayoutProperty<ButtonLayoutProperty>();
        CHECK_NULL_RETURN(host, false);
        return layoutProperty->HasAspectRatio() &&
               layoutProperty->GetType().value_or(ButtonType::CAPSULE) != ButtonType::CIRCLE;
    }

    void SetClickedColor(const Color& color)
    {
        clickedColor_ = color;
        isSetClickedColor_ = true;
    }

    void SetFocusBorderColor(const Color& color)
    {
        focusBorderColor_ = color;
    }

    void SetFocusBorderWidth(const Dimension& width)
    {
        focusBorderWidth_ = width;
    }

    void setComponentButtonType(const ComponentButtonType& buttonType)
    {
        buttonType_ = buttonType;
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override
    {
        Pattern::ToJsonValue(json);
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto layoutProperty = host->GetLayoutProperty<ButtonLayoutProperty>();
        CHECK_NULL_VOID(layoutProperty);
        auto context = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(context);
        auto buttonTheme = context->GetTheme<ButtonTheme>();
        CHECK_NULL_VOID(buttonTheme);
        auto textStyle = buttonTheme->GetTextStyle();
        json->Put(
            "type", host->GetTag() == "Toggle"
                        ? "ToggleType.Button"
                        : ConvertButtonTypeToString(layoutProperty->GetType().value_or(ButtonType::CAPSULE)).c_str());
        json->Put("fontSize",
            layoutProperty->GetFontSizeValue(layoutProperty->HasLabel() ? textStyle.GetFontSize() : Dimension(0))
                .ToString()
                .c_str());
        json->Put("fontWeight",
            V2::ConvertWrapFontWeightToStirng(layoutProperty->GetFontWeight().value_or(FontWeight::NORMAL)).c_str());
        json->Put("fontColor", layoutProperty->GetFontColor()
                                   .value_or(layoutProperty->HasLabel() ? textStyle.GetTextColor() : Color::BLACK)
                                   .ColorToString()
                                   .c_str());
        auto fontFamilyVector =
            layoutProperty->GetFontFamily().value_or<std::vector<std::string>>({ "HarmonyOS Sans" });
        std::string fontFamily = fontFamilyVector.at(0);
        for (uint32_t i = 1; i < fontFamilyVector.size(); ++i) {
            fontFamily += ',' + fontFamilyVector.at(i);
        }
        json->Put("fontFamily", fontFamily.c_str());
        json->Put("fontStyle", layoutProperty->GetFontStyle().value_or(Ace::FontStyle::NORMAL) == Ace::FontStyle::NORMAL
                                   ? "FontStyle.Normal"
                                   : "FontStyle.Italic");
        json->Put("label", layoutProperty->GetLabelValue("").c_str());
        auto eventHub = host->GetEventHub<ButtonEventHub>();
        CHECK_NULL_VOID(eventHub);
        json->Put("stateEffect", eventHub->GetStateEffect() ? "true" : "false");
        auto optionJson = JsonUtil::Create(true);
        optionJson->Put(
            "type", ConvertButtonTypeToString(layoutProperty->GetType().value_or(ButtonType::CAPSULE)).c_str());
        optionJson->Put("stateEffect", eventHub->GetStateEffect() ? "true" : "false");
        json->Put("options", optionJson->ToString().c_str());
        auto fontJsValue = JsonUtil::Create(true);
        fontJsValue->Put("size", layoutProperty->GetFontSizeValue(Dimension(0)).ToString().c_str());
        fontJsValue->Put("weight",
            V2::ConvertWrapFontWeightToStirng(layoutProperty->GetFontWeight().value_or(FontWeight::NORMAL)).c_str());
        fontJsValue->Put("family", fontFamily.c_str());
        fontJsValue->Put(
            "style", layoutProperty->GetFontStyle().value_or(Ace::FontStyle::NORMAL) == Ace::FontStyle::NORMAL
                         ? "FontStyle.Normal"
                         : "FontStyle.Italic");
        auto labelJsValue = JsonUtil::Create(true);
        labelJsValue->Put("overflow",
            V2::ConvertWrapTextOverflowToString(layoutProperty->GetTextOverflow().value_or(TextOverflow::CLIP))
                .c_str());
        labelJsValue->Put("maxLines", std::to_string(layoutProperty->GetMaxLines().value_or(DEFAULT_MAXLINES)).c_str());
        labelJsValue->Put("minFontSize", layoutProperty->GetMinFontSizeValue(Dimension(0)).ToString().c_str());
        labelJsValue->Put("maxFontSize", layoutProperty->GetMaxFontSizeValue(Dimension(0)).ToString().c_str());
        labelJsValue->Put("heightAdaptivePolicy",
            V2::ConvertWrapTextHeightAdaptivePolicyToString(
                layoutProperty->GetHeightAdaptivePolicy().value_or(TextHeightAdaptivePolicy::MAX_LINES_FIRST))
                .c_str());
        labelJsValue->Put("font", fontJsValue->ToString().c_str());
        json->Put("labelStyle", labelJsValue->ToString().c_str());
    }

    static std::string ConvertButtonTypeToString(ButtonType buttonType)
    {
        std::string result;
        switch (buttonType) {
            case ButtonType::NORMAL:
                result = "ButtonType.Normal";
                break;
            case ButtonType::CAPSULE:
                result = "ButtonType.Capsule";
                break;
            case ButtonType::CIRCLE:
                result = "ButtonType.Circle";
                break;
            default:
                LOGD("The input does not match any ButtonType");
        }
        return result;
    }

    void SetLocalLocation(const Offset& localLocation)
    {
        localLocation_ = localLocation;
    }

    const Offset& GetLocalLocation() const
    {
        return localLocation_;
    }

    void SetInHover(bool inHover)
    {
        isInHover_ = inHover;
    }

    bool GetIsInHover() const
    {
        return isInHover_;
    }

    RefPtr<InputEvent>& GetHoverListener()
    {
        return hoverListener_;
    }

    RefPtr<TouchEventImpl>& GetTouchListener()
    {
        return touchListener_;
    }

    void OnColorConfigurationUpdate() override;

    void SetSkipColorConfigurationUpdate()
    {
        isColorUpdateFlag_ = true;
    }

protected:
    void OnModifyDone() override;
    void OnAttachToFrameNode() override;
    void InitTouchEvent();
    void InitHoverEvent();
    void OnTouchDown();
    void OnTouchUp();
    void HandleHoverEvent(bool isHover);
    void HandleBackgroundColor();
    void HandleEnabled();
    void InitButtonLabel();
    void AnimateTouchAndHover(RefPtr<RenderContext>& renderContext, float startOpacity, float endOpacity,
        int32_t duration, const RefPtr<Curve>& curve);
    Color clickedColor_;

private:
    static void UpdateTextLayoutProperty(
        RefPtr<ButtonLayoutProperty>& layoutProperty, RefPtr<TextLayoutProperty>& textLayoutProperty);
    Color backgroundColor_;
    Color focusBorderColor_;
    bool isSetClickedColor_ = false;
    ComponentButtonType buttonType_ = ComponentButtonType::BUTTON;

    RefPtr<TouchEventImpl> touchListener_;
    RefPtr<InputEvent> hoverListener_;
    bool isHover_ = false;

    bool isInHover_ = false;
    Offset localLocation_;
    Dimension focusBorderWidth_;

    bool isColorUpdateFlag_ = false;
    ACE_DISALLOW_COPY_AND_MOVE(ButtonPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_BUTTON_BUTTON_PATTERN_H
