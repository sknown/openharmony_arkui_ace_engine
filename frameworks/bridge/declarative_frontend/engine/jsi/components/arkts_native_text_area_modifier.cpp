/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "bridge/declarative_frontend/engine/jsi/components/arkts_native_text_area_modifier.h"
#include "bridge/common/utils/utils.h"
#include "bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/common/properties/text_style.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"
#include "core/components_ng/pattern/text_field/text_field_model_ng.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace::NG {
constexpr InputStyle DEFAULT_TEXT_AREA_STYLE = InputStyle::DEFAULT;
constexpr bool DEFAULT_SELECTION_MENU_HIDDEN = false;
constexpr uint32_t DEFAULT_MAX_VIEW_LINE = 3;
constexpr CopyOptions DEFAULT_COPY_OPTIONS_VALUE = CopyOptions::None;
constexpr Dimension DEFAULT_FONT_SIZE = 16.0_fp;
constexpr FontWeight DEFAULT_FONT_WEIGHT = FontWeight::NORMAL;
constexpr Ace::FontStyle DEFAULT_FONT_STYLE = Ace::FontStyle::NORMAL;
constexpr DisplayMode DEFAULT_BAR_STATE_VALUE = DisplayMode::AUTO;
constexpr bool DEFAULT_KEY_BOARD_VALUE = true;
constexpr char DEFAULT_FONT_FAMILY[] = "HarmonyOS Sans";
constexpr const char DEFAULT_CATET_COLOR[] = "#007DFF";

void SetTextAreaStyle(NodeHandle node, int32_t style)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetInputStyle(frameNode, static_cast<InputStyle>(style));
}

void ResetTextAreaStyle(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetInputStyle(frameNode, DEFAULT_TEXT_AREA_STYLE);
}

void SetTextAreaSelectionMenuHidden(NodeHandle node, uint32_t contextMenuHidden)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetSelectionMenuHidden(frameNode, static_cast<bool>(contextMenuHidden));
}

void ResetTextAreaSelectionMenuHidden(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetSelectionMenuHidden(frameNode, DEFAULT_SELECTION_MENU_HIDDEN);
}

void SetTextAreaMaxLines(NodeHandle node, uint32_t maxLine)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetMaxViewLines(frameNode, maxLine);
}

void ResetTextAreaMaxLines(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetMaxViewLines(frameNode, DEFAULT_MAX_VIEW_LINE);
}

void SetTextAreaCopyOption(NodeHandle node, int32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCopyOption(frameNode, static_cast<CopyOptions>(value));
}

void ResetTextAreaCopyOption(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCopyOption(frameNode, DEFAULT_COPY_OPTIONS_VALUE);
}

void SetTextAreaPlaceholderColor(NodeHandle node, const struct ArkUIResourceColorType *colorType)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Color color;
    if (colorType->string != nullptr) {
        if (!Color::ParseColorString(std::string(colorType->string), color)) {
            return;
        }
    } else {
        color = Color(colorType->number);
    }
    TextFieldModelNG::SetPlaceholderColor(frameNode, color);
}

void ResetTextAreaPlaceholderColor(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto theme = Framework::JSViewAbstract::GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    uint32_t color = theme->GetPlaceholderColor().GetValue();
    TextFieldModelNG::SetPlaceholderColor(frameNode, Color(color));
}

void SetTextAreaTextAlign(NodeHandle node, int32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextAlign value_textAlign = static_cast<TextAlign>(value);
    TextFieldModelNG::SetTextAlign(frameNode, value_textAlign);
}

void ResetTextAreaTextAlign(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetTextAlign(frameNode, TextAlign::START);
}

void SetTextAreaPlaceholderFont(NodeHandle node, const struct StringAndDouble *size,
    const struct ArkUIFontWeight *weight, const char *family, int32_t style)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Font font;

    if (size->valueStr != nullptr) {
        font.fontSize = StringUtils::StringToCalcDimension(size->valueStr, true, DimensionUnit::FP);
    } else {
        font.fontSize = CalcDimension(size->value, DimensionUnit::FP);
    }

    if (weight->valueStr != nullptr) {
        font.fontWeight = StringUtils::StringToFontWeight(std::string(weight->valueStr), FontWeight::NORMAL);
    } else if (weight->value >= 0) {
        font.fontWeight = static_cast<FontWeight>(weight->value);
    }

    if (family != nullptr) {
        font.fontFamilies = Framework::ConvertStrToFontFamilies(std::string(family));
    }

    if (style >= 0) {
        font.fontStyle = static_cast<Ace::FontStyle>(style);
    }
    TextFieldModelNG::SetPlaceholderFont(frameNode, font);
}


void ResetTextAreaPlaceholderFont(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Font font;
    font.fontSize = DEFAULT_FONT_SIZE;
    font.fontWeight = DEFAULT_FONT_WEIGHT;
    font.fontStyle = DEFAULT_FONT_STYLE;
    TextFieldModelNG::SetPlaceholderFont(frameNode, font);
}

void SetTextAreaBarState(NodeHandle node, uint32_t barStateValue)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    DisplayMode displayMode = static_cast<DisplayMode>(barStateValue);
    TextFieldModelNG::SetBarState(frameNode, displayMode);
}

void ResetTextAreaBarState(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetBarState(frameNode, DEFAULT_BAR_STATE_VALUE);
}

void SetTextAreaEnableKeyboardOnFocus(NodeHandle node, uint32_t keyboardOnFocusValue)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::RequestKeyboardOnFocus(frameNode, static_cast<bool>(keyboardOnFocusValue));
}

void ResetTextAreaEnableKeyboardOnFocus(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::RequestKeyboardOnFocus(frameNode, DEFAULT_KEY_BOARD_VALUE);
}

void SetTextAreaFontFamily(NodeHandle node, const char *fontFamily)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    std::vector<std::string> fontFamilies;
    fontFamilies = Framework::ConvertStrToFontFamilies(std::string(fontFamily));
    TextFieldModelNG::SetFontFamily(frameNode, fontFamilies);
}

void ResetTextAreaFontFamily(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    std::vector<std::string> fontFamilies;
    fontFamilies.emplace_back(std::string(DEFAULT_FONT_FAMILY));
    TextFieldModelNG::SetFontFamily(frameNode, fontFamilies);
}

void SetTextAreaShowCounter(NodeHandle node, uint32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowCounter(frameNode, static_cast<bool>(value));
}

void ResetTextAreaShowCounter(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowCounter(frameNode, false);
}

void SetTextAreaCaretColor(NodeHandle node, const struct ArkUIResourceColorType *colorType)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Color color;
    if (colorType->string != nullptr) {
        if (!Color::ParseColorString(std::string(colorType->string), color)) {
            return;
        }
    } else {
        color = Color(colorType->number);
    }
    TextFieldModelNG::SetCaretColor(frameNode, color);
}

void ResetTextAreaCaretColor(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCaretColor(frameNode, Color::FromString(std::string(DEFAULT_CATET_COLOR)));
}

void SetTextAreaMaxLength(NodeHandle node, int32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetMaxLength(frameNode, value);
}

void ResetTextAreaMaxLength(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::ResetMaxLength(frameNode);
}

void SetTextAreaFontColor(NodeHandle node, const struct ArkUIResourceColorType *colorType)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Color color;
    if (colorType->string != nullptr) {
        if (!Color::ParseColorString(std::string(colorType->string), color)) {
            return;
        }
    } else {
        color = Color(colorType->number);
    }
    TextFieldModelNG::SetTextColor(frameNode, color);
}

void ResetTextAreaFontColor(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    int32_t textColor = 0;
    auto theme = Framework::JSViewAbstract::GetTheme<TextFieldTheme>();
    textColor = theme->GetTextColor().GetValue();
    TextFieldModelNG::SetTextColor(frameNode, Color(textColor));
}

void SetTextAreaFontStyle(NodeHandle node, uint32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontStyle(frameNode, static_cast<Ace::FontStyle>(value));
}

void ResetTextAreaFontStyle(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontStyle(frameNode, OHOS::Ace::FontStyle::NORMAL);
}

void SetTextAreaFontWeight(NodeHandle node, uint32_t fontWeight)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontWeight(frameNode, static_cast<FontWeight>(fontWeight));
}
void ResetTextAreaFontWeight(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontWeight(frameNode, OHOS::Ace::FontWeight::NORMAL);
}

void SetTextAreaFontSize(NodeHandle node, const struct StringAndDouble *size)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension result;
    if (size->valueStr != nullptr) {
        result = StringUtils::StringToCalcDimension(size->valueStr, false, DimensionUnit::FP);
    } else if (size->value >= 0) {
        result = CalcDimension(size->value, DimensionUnit::FP);
    } else {
        result = DEFAULT_FONT_SIZE;
    }
    if (result.Unit() == DimensionUnit::PERCENT) {
        result = DEFAULT_FONT_SIZE;
    }
    TextFieldModelNG::SetFontSize(frameNode, result);
}

void ResetTextAreaFontSize(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontSize(frameNode, DEFAULT_FONT_SIZE);
}

ArkUITextAreaModifierAPI GetTextAreaModifier()
{
    static const ArkUITextAreaModifierAPI modifier = { SetTextAreaStyle,
                                                       ResetTextAreaStyle,
                                                       SetTextAreaSelectionMenuHidden,
                                                       ResetTextAreaSelectionMenuHidden,
                                                       SetTextAreaMaxLines,
                                                       ResetTextAreaMaxLines,
                                                       SetTextAreaCopyOption,
                                                       ResetTextAreaCopyOption,
                                                       SetTextAreaPlaceholderColor,
                                                       ResetTextAreaPlaceholderColor,
                                                       SetTextAreaTextAlign,
                                                       ResetTextAreaTextAlign,
                                                       SetTextAreaPlaceholderFont,
                                                       ResetTextAreaPlaceholderFont,
                                                       SetTextAreaBarState,
                                                       ResetTextAreaBarState,
                                                       SetTextAreaEnableKeyboardOnFocus,
                                                       ResetTextAreaEnableKeyboardOnFocus,
                                                       SetTextAreaFontFamily,
                                                       ResetTextAreaFontFamily,
                                                       SetTextAreaShowCounter,
                                                       ResetTextAreaShowCounter,
                                                       SetTextAreaCaretColor,
                                                       ResetTextAreaCaretColor,
                                                       SetTextAreaMaxLength,
                                                       ResetTextAreaMaxLength,
                                                       SetTextAreaFontColor,
                                                       ResetTextAreaFontColor,
                                                       SetTextAreaFontStyle,
                                                       ResetTextAreaFontStyle,
                                                       SetTextAreaFontWeight,
                                                       ResetTextAreaFontWeight,
                                                       SetTextAreaFontSize,
                                                       ResetTextAreaFontSize };

    return modifier;
}
} // namespace OHOS::Ace::NG