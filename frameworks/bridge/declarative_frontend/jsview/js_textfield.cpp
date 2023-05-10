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

#include "frameworks/bridge/declarative_frontend/jsview/js_textfield.h"

#include <cstdint>
#include <vector>

#include "base/geometry/dimension.h"
#include "base/utils/utils.h"
#include "bridge/common/utils/utils.h"
#include "bridge/declarative_frontend/engine/functions/js_click_function.h"
#include "bridge/declarative_frontend/engine/functions/js_clipboard_function.h"
#include "bridge/declarative_frontend/engine/functions/js_function.h"
#include "bridge/declarative_frontend/jsview/js_container_base.h"
#include "bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "bridge/declarative_frontend/jsview/js_textarea.h"
#include "bridge/declarative_frontend/jsview/js_textinput.h"
#include "bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "bridge/declarative_frontend/jsview/js_view_common_def.h"
#include "bridge/declarative_frontend/jsview/models/text_field_model_impl.h"
#include "bridge/declarative_frontend/jsview/models/view_abstract_model_impl.h"
#include "bridge/declarative_frontend/view_stack_processor.h"
#include "core/common/container.h"
#include "core/common/ime/text_input_action.h"
#include "core/common/ime/text_input_type.h"
#include "core/components/text_field/text_field_component.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"
#include "core/components_ng/pattern/text_field/text_field_model_ng.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace {

std::unique_ptr<TextFieldModel> TextFieldModel::instance_ = nullptr;
std::mutex TextFieldModel::mutex_;

TextFieldModel* TextFieldModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
#ifdef NG_BUILD
            instance_.reset(new NG::TextFieldModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                instance_.reset(new NG::TextFieldModelNG());
            } else {
                instance_.reset(new Framework::TextFieldModelImpl());
            }
#endif
        }
    }
    return instance_.get();
}

} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {

namespace {

const std::vector<TextAlign> TEXT_ALIGNS = { TextAlign::START, TextAlign::CENTER, TextAlign::END };
const std::vector<FontStyle> FONT_STYLES = { FontStyle::NORMAL, FontStyle::ITALIC };
const std::vector<std::string> INPUT_FONT_FAMILY_VALUE = { "sans-serif" };
constexpr uint32_t TEXTAREA_MAXLENGTH_VALUE_DEFAULT = std::numeric_limits<uint32_t>::max();

void InitTextAreaDefaultStyle()
{
    auto boxComponent = ViewStackProcessor::GetInstance()->GetBoxComponent();
    auto* stack = ViewStackProcessor::GetInstance();
    auto textAreaComponent = AceType::DynamicCast<OHOS::Ace::TextFieldComponent>(stack->GetMainComponent());
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetThemeManager()->GetTheme<TextFieldTheme>();
    if (!boxComponent || !textAreaComponent || !theme) {
        LOGI("boxComponent or textAreaComponent or theme is null");
        return;
    }

    textAreaComponent->SetTextMaxLines(TEXTAREA_MAXLENGTH_VALUE_DEFAULT);
    textAreaComponent->SetCursorColor(theme->GetCursorColor());
    textAreaComponent->SetPlaceholderColor(theme->GetPlaceholderColor());
    textAreaComponent->SetFocusBgColor(theme->GetFocusBgColor());
    textAreaComponent->SetFocusPlaceholderColor(theme->GetFocusPlaceholderColor());
    textAreaComponent->SetFocusTextColor(theme->GetFocusTextColor());
    textAreaComponent->SetBgColor(theme->GetBgColor());
    textAreaComponent->SetTextColor(theme->GetTextColor());
    textAreaComponent->SetSelectedColor(theme->GetSelectedColor());
    textAreaComponent->SetHoverColor(theme->GetHoverColor());
    textAreaComponent->SetPressColor(theme->GetPressColor());

    TextStyle textStyle = textAreaComponent->GetTextStyle();
    textStyle.SetTextColor(theme->GetTextColor());
    textStyle.SetFontSize(theme->GetFontSize());
    textStyle.SetFontWeight(theme->GetFontWeight());
    textStyle.SetFontFamilies(INPUT_FONT_FAMILY_VALUE);
    textAreaComponent->SetTextStyle(textStyle);
    textAreaComponent->SetEditingStyle(textStyle);
    textAreaComponent->SetPlaceHoldStyle(textStyle);

    textAreaComponent->SetCountTextStyle(theme->GetCountTextStyle());
    textAreaComponent->SetOverCountStyle(theme->GetOverCountStyle());
    textAreaComponent->SetCountTextStyleOuter(theme->GetCountTextStyleOuter());
    textAreaComponent->SetOverCountStyleOuter(theme->GetOverCountStyleOuter());
    textAreaComponent->SetErrorBorderWidth(theme->GetErrorBorderWidth());
    textAreaComponent->SetErrorBorderColor(theme->GetErrorBorderColor());

    RefPtr<Decoration> backDecoration = AceType::MakeRefPtr<Decoration>();
    backDecoration->SetPadding(theme->GetPadding());
    backDecoration->SetBackgroundColor(theme->GetBgColor());
    backDecoration->SetBorderRadius(theme->GetBorderRadius());
    const auto& boxDecoration = boxComponent->GetBackDecoration();
    if (boxDecoration) {
        backDecoration->SetImage(boxDecoration->GetImage());
        backDecoration->SetGradient(boxDecoration->GetGradient());
    }
    textAreaComponent->SetOriginBorder(backDecoration->GetBorder());
    textAreaComponent->SetDecoration(backDecoration);
    textAreaComponent->SetIconSize(theme->GetIconSize());
    textAreaComponent->SetIconHotZoneSize(theme->GetIconHotZoneSize());
    textAreaComponent->SetHeight(theme->GetHeight());

    // text area need to extend height.
    textAreaComponent->SetExtend(true);
    boxComponent->SetHeight(-1.0, DimensionUnit::VP);
}

} // namespace

void ParseTextFieldTextObject(const JSCallbackInfo& info, const JSRef<JSVal>& changeEventVal)
{
    CHECK_NULL_VOID(changeEventVal->IsFunction());

    JsEventCallback<void(const std::string&)> onChangeEvent(
        info.GetExecutionContext(), JSRef<JSFunc>::Cast(changeEventVal));
    TextFieldModel::GetInstance()->SetOnChangeEvent(std::move(onChangeEvent));
}

void JSTextField::CreateTextInput(const JSCallbackInfo& info)
{
    std::optional<std::string> placeholderSrc;
    std::optional<std::string> value;
    JSTextInputController* jsController = nullptr;
    JSRef<JSVal> changeEventVal = JSRef<JSVal>::Make();
    if (info[0]->IsObject()) {
        auto paramObject = JSRef<JSObject>::Cast(info[0]);
        std::string placeholder;
        if (ParseJsString(paramObject->GetProperty("placeholder"), placeholder)) {
            placeholderSrc = placeholder;
        }
        std::string text;
        if (paramObject->GetProperty("text")->IsObject()) {
            JSRef<JSObject> valueObj = JSRef<JSObject>::Cast(paramObject->GetProperty("text"));
            changeEventVal = valueObj->GetProperty("changeEvent");
            auto valueProperty = valueObj->GetProperty("value");
            if (ParseJsString(valueProperty, text)) {
                value = text;
            }
        } else {
            if (ParseJsString(paramObject->GetProperty("text"), text)) {
                value = text;
            }
        }
        auto controllerObj = paramObject->GetProperty("controller");
        if (!controllerObj->IsUndefined() && !controllerObj->IsNull()) {
            jsController = JSRef<JSObject>::Cast(controllerObj)->Unwrap<JSTextInputController>();
        }
    }

    auto controller = TextFieldModel::GetInstance()->CreateTextInput(placeholderSrc, value);
    if (jsController) {
        jsController->SetController(controller);
    }
    if (!changeEventVal->IsUndefined() && changeEventVal->IsFunction()) {
        ParseTextFieldTextObject(info, changeEventVal);
    }

    if (!Container::IsCurrentUseNewPipeline()) {
        JSInteractableView::SetFocusable(true);
        JSInteractableView::SetFocusNode(true);
    }
}

void JSTextField::CreateTextArea(const JSCallbackInfo& info)
{
    std::optional<std::string> placeholderSrc;
    std::optional<std::string> value;
    JSTextAreaController* jsController = nullptr;
    JSRef<JSVal> changeEventVal;
    if (info[0]->IsObject()) {
        auto paramObject = JSRef<JSObject>::Cast(info[0]);
        std::string placeholder;
        if (ParseJsString(paramObject->GetProperty("placeholder"), placeholder)) {
            placeholderSrc = placeholder;
        }
        std::string text;
        if (paramObject->GetProperty("text")->IsObject()) {
            JSRef<JSObject> valueObj = JSRef<JSObject>::Cast(paramObject->GetProperty("text"));
            changeEventVal = valueObj->GetProperty("changeEvent");
            auto valueProperty = valueObj->GetProperty("value");
            if (ParseJsString(valueProperty, text)) {
                value = text;
            }
        } else {
            if (ParseJsString(paramObject->GetProperty("text"), text)) {
                value = text;
            }
        }
        auto controllerObj = paramObject->GetProperty("controller");
        if (!controllerObj->IsUndefined() && !controllerObj->IsNull()) {
            jsController = JSRef<JSObject>::Cast(controllerObj)->Unwrap<JSTextAreaController>();
        }
    }

    if (Container::IsCurrentUseNewPipeline()) {
        auto controller = TextFieldModel::GetInstance()->CreateTextArea(placeholderSrc, value);
        if (jsController) {
            jsController->SetController(controller);
        }
        if (!changeEventVal->IsUndefined() && changeEventVal->IsFunction()) {
            ParseTextFieldTextObject(info, changeEventVal);
        }
        return;
    }

    RefPtr<TextFieldComponent> textAreaComponent = AceType::MakeRefPtr<TextFieldComponent>();
    textAreaComponent->SetTextFieldController(AceType::MakeRefPtr<TextFieldController>());
    textAreaComponent->SetTextInputType(TextInputType::MULTILINE);
    textAreaComponent->SetHoverAnimationType(HoverAnimationType::BOARD);
    auto paramObject = JSRef<JSObject>::Cast(info[0]);

    ViewStackProcessor::GetInstance()->ClaimElementId(textAreaComponent);
    ViewStackProcessor::GetInstance()->Push(textAreaComponent);
    InitTextAreaDefaultStyle();
    Border boxBorder;
    auto boxComponent = ViewStackProcessor::GetInstance()->GetBoxComponent();
    auto theme = GetTheme<TextFieldTheme>();
    if (boxComponent->GetBackDecoration()) {
        boxBorder = boxComponent->GetBackDecoration()->GetBorder();
    }
    if (value) {
        textAreaComponent->SetValue(value.value());
    }
    if (placeholderSrc) {
        textAreaComponent->SetPlaceholder(placeholderSrc.value());
    }
    JSTextField::UpdateDecoration(boxComponent, textAreaComponent, boxBorder, theme);
    if (jsController) {
        jsController->SetController(textAreaComponent->GetTextFieldController());
    }

    JSInteractableView::SetFocusable(true);
    JSInteractableView::SetFocusNode(true);
}

void JSTextField::SetType(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGI("SetType create error, info is non-valid");
        return;
    }
    if (!info[0]->IsNumber()) {
        LOGI("The inputType is not number");
        return;
    }
    TextInputType textInputType = static_cast<TextInputType>(info[0]->ToNumber<int32_t>());
    TextFieldModel::GetInstance()->SetType(textInputType);
}

void JSTextField::SetPlaceholderColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGI("The arg(SetPlaceholderColor) is wrong, it is supposed to have atleast 1 argument");
        return;
    }

    Color color;
    if (!ParseJsColor(info[0], color)) {
        LOGI("the info[0] is null");
        return;
    }
    TextFieldModel::GetInstance()->SetPlaceholderColor(color);
}

void JSTextField::SetPlaceholderFont(const JSCallbackInfo& info)
{
    if (info.Length() < 1 || !info[0]->IsObject()) {
        LOGI("PlaceholderFont create error, info is non-valid");
        return;
    }
    Font font;
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    auto fontSize = paramObject->GetProperty("size");
    if (fontSize->IsNull() || fontSize->IsUndefined()) {
        font.fontSize = Dimension(-1);
    } else {
        CalcDimension size;
        if (fontSize->IsString()) {
            auto result = StringUtils::StringToDimensionWithThemeValue(fontSize->ToString(), true, Dimension(-1));
            font.fontSize = result;
        } else if (!ParseJsDimensionFp(fontSize, size) || size.Unit() == DimensionUnit::PERCENT) {
            font.fontSize = Dimension(-1);
            LOGW("Parse to dimension FP failed.");
        } else {
            font.fontSize = size;
        }
    }

    std::string weight;
    auto fontWeight = paramObject->GetProperty("weight");
    if (!fontWeight->IsNull()) {
        if (fontWeight->IsNumber()) {
            weight = std::to_string(fontWeight->ToNumber<int32_t>());
        } else {
            ParseJsString(fontWeight, weight);
        }
        font.fontWeight = ConvertStrToFontWeight(weight);
    }

    auto fontFamily = paramObject->GetProperty("family");
    if (!fontFamily->IsNull()) {
        std::vector<std::string> fontFamilies;
        if (ParseJsFontFamilies(fontFamily, fontFamilies)) {
            font.fontFamilies = fontFamilies;
        }
    }

    auto style = paramObject->GetProperty("style");
    if (!style->IsNull()) {
        font.fontStyle = static_cast<FontStyle>(style->ToNumber<int32_t>());
    }
    TextFieldModel::GetInstance()->SetPlaceholderFont(font);
}

void JSTextField::SetEnterKeyType(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    if (!info[0]->IsNumber()) {
        LOGI("Info(SetEnterKeyType) is not number");
        return;
    }
    TextInputAction textInputAction = static_cast<TextInputAction>(info[0]->ToNumber<int32_t>());
    TextFieldModel::GetInstance()->SetEnterKeyType(textInputAction);
}

void JSTextField::SetTextAlign(int32_t value)
{
    if (value >= 0 && value < static_cast<int32_t>(TEXT_ALIGNS.size())) {
        TextFieldModel::GetInstance()->SetTextAlign(TEXT_ALIGNS[value]);
    } else {
        LOGI("the value is error");
    }
}

void JSTextField::SetInputStyle(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGI("The arg(SetInputStyle) is wrong, it is supposed to have at least 1 argument");
        return;
    }
    auto styleString = info[0]->ToString();
    if (styleString == "Inline") {
        TextFieldModel::GetInstance()->SetInputStyle(InputStyle::INLINE);
    } else {
        TextFieldModel::GetInstance()->SetInputStyle(InputStyle::DEFAULT);
    }
}

void JSTextField::SetCaretColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGI("The arg(SetCareColor) is wrong, it is supposed to have atleast 1 argument");
        return;
    }

    Color color;
    if (!ParseJsColor(info[0], color)) {
        LOGI("info[0] is null");
        return;
    }

    TextFieldModel::GetInstance()->SetCaretColor(color);
}

void JSTextField::SetCaretStyle(const JSCallbackInfo& info)
{
    if (info.Length() < 1 || !info[0]->IsObject()) {
        LOGW("CaretStyle create error, info is non-valid");
        return;
    }
    CaretStyle caretStyle;
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    auto caretWidth = paramObject->GetProperty("width");

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetThemeManager()->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID_NOLOG(theme);
    if (caretWidth->IsNull() || caretWidth->IsUndefined()) {
        caretStyle.caretWidth = theme->GetCursorWidth();
    } else {
        CalcDimension width;
        if (!ParseJsDimensionVp(caretWidth, width)) {
            caretStyle.caretWidth = theme->GetCursorWidth();
        }
        if (LessNotEqual(width.Value(), 0.0)) {
            return;
        }
        caretStyle.caretWidth = width;
    }

    if (Container::IsCurrentUseNewPipeline()) {
        TextFieldModel::GetInstance()->SetCaretStyle(caretStyle);
        return;
    }
}

void JSTextField::SetCaretPosition(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGW("The arg(SetCaretPosition) is wrong, it is supposed to have at least 1 arguments");
        return;
    }

    int32_t caretPosition = 0;
    if (!ParseJsInt32(info[0], caretPosition)) {
        return;
    }
    if (caretPosition < 0) {
        return;
    }
    if (Container::IsCurrentUseNewPipeline()) {
        TextFieldModel::GetInstance()->SetCaretPosition(caretPosition);
        return;
    }
}

void JSTextField::SetSelectedBackgroundColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGW("The arg(SetSelectedBackgroundColor) is wrong, it is supposed to have atleast 1 argument");
        return;
    }

    Color selectedColor;
    if (!ParseJsColor(info[0], selectedColor)) {
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto theme = pipeline->GetThemeManager()->GetTheme<TextFieldTheme>();
        CHECK_NULL_VOID_NOLOG(theme);
        selectedColor = theme->GetSelectedColor();
    }
    if (Container::IsCurrentUseNewPipeline()) {
        TextFieldModel::GetInstance()->SetSelectedBackgroundColor(selectedColor);
        return;
    }
}

void JSTextField::SetMaxLength(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGI("The arg(SetMaxLength) is wrong, it is supposed to have atleast 1 argument");
        return;
    }
    int32_t maxLength = 0;
    if (info[0]->IsUndefined()) {
        TextFieldModel::GetInstance()->ResetMaxLength();
        return;
    } else if (!info[0]->IsNumber()) {
        LOGI("Max length should be number");
        TextFieldModel::GetInstance()->ResetMaxLength();
        return;
    }
    maxLength = info[0]->ToNumber<int32_t>();
    if (GreatOrEqual(maxLength, 0)) {
        TextFieldModel::GetInstance()->SetMaxLength(maxLength);
    } else {
        TextFieldModel::GetInstance()->ResetMaxLength();
    }
}

void JSTextField::SetFontSize(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGI("JSTextField::SetFontSize The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    CalcDimension fontSize;
    if (!ParseJsDimensionFp(info[0], fontSize)) {
        LOGI("Parse to dimension FP failed!");
        return;
    }
    TextFieldModel::GetInstance()->SetFontSize(fontSize);
}

void JSTextField::RequestKeyboardOnFocus(bool needToRequest)
{
    TextFieldModel::GetInstance()->RequestKeyboardOnFocus(needToRequest);
}

void JSTextField::SetFontWeight(const std::string& value)
{
    TextFieldModel::GetInstance()->SetFontWeight(ConvertStrToFontWeight(value));
}

void JSTextField::SetTextColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGI("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Color textColor;
    if (!ParseJsColor(info[0], textColor)) {
        LOGI("Parse to dimension FP failed!");
        return;
    }
    TextFieldModel::GetInstance()->SetTextColor(textColor);
}

void JSTextField::SetForegroundColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    ForegroundColorStrategy strategy;
    if (ParseJsColorStrategy(info[0], strategy)) {
        ViewAbstractModel::GetInstance()->SetForegroundColorStrategy(strategy);
        TextFieldModel::GetInstance()->SetForegroundColor(Color::FOREGROUND);
        return;
    }
    Color foregroundColor;
    if (!ParseJsColor(info[0], foregroundColor)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetForegroundColor(foregroundColor);
    TextFieldModel::GetInstance()->SetForegroundColor(foregroundColor);
}

void JSTextField::SetFontStyle(int32_t value)
{
    if (value >= 0 && value < static_cast<int32_t>(FONT_STYLES.size())) {
        TextFieldModel::GetInstance()->SetFontStyle(FONT_STYLES[value]);
    } else {
        LOGI("TextInput fontStyle(%{public}d) illegal value", value);
    }
}

void JSTextField::SetFontFamily(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGI("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    std::vector<std::string> fontFamilies;
    if (!ParseJsFontFamilies(info[0], fontFamilies)) {
        LOGI("Parse FontFamilies failed");
        return;
    }
    TextFieldModel::GetInstance()->SetFontFamily(fontFamilies);
}

void JSTextField::SetInputFilter(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGI("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    std::string inputFilter;
    if (!ParseJsString(info[0], inputFilter)) {
        LOGI("Parse inputFilter failed");
        return;
    }
    if (info.Length() > 1 && info[1]->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsClipboardFunction>(JSRef<JSFunc>::Cast(info[1]));
        auto resultId = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc)](const std::string& info) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            func->Execute(info);
        };
        TextFieldModel::GetInstance()->SetInputFilter(inputFilter, resultId);
        return;
    }
    TextFieldModel::GetInstance()->SetInputFilter(inputFilter, nullptr);
}

void JSTextField::SetShowPasswordIcon(const JSCallbackInfo& info)
{
    if (!info[0]->IsBoolean()) {
        LOGI("The info is wrong, it is supposed to be an boolean");
        return;
    }

    bool isShowPasswordIcon = info[0]->ToBoolean();
    TextFieldModel::GetInstance()->SetShowPasswordIcon(isShowPasswordIcon);
}

void JSTextField::SetBackgroundColor(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        if (info.Length() < 1) {
            LOGI("The argv is wrong, it is supposed to have at least 1 argument");
            return;
        }
        Color backgroundColor;
        if (!ParseJsColor(info[0], backgroundColor)) {
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID_NOLOG(pipeline);
            auto themeManager = pipeline->GetThemeManager();
            CHECK_NULL_VOID_NOLOG(themeManager);
            auto theme = themeManager->GetTheme<TextFieldTheme>();
            CHECK_NULL_VOID_NOLOG(theme);
            backgroundColor = theme->GetBgColor();
        }
        ViewAbstractModel::GetInstance()->SetBackgroundColor(backgroundColor);
        return;
    }
    if (info.Length() < 1) {
        LOGI("The arg(SetBackgroundColor) is wrong, it is supposed to have atleast 1 argument");
        return;
    }

    Color backgroundColor;
    if (!ParseJsColor(info[0], backgroundColor)) {
        LOGI("the info[0] is null");
        return;
    }

    auto* stack = ViewStackProcessor::GetInstance();
    auto component = AceType::DynamicCast<OHOS::Ace::TextFieldComponent>(stack->GetMainComponent());
    if (component) {
        component->SetBgColor(backgroundColor);
    } else {
        LOGI("The component(SetPlaceholderColor) is null");
    }
}

void JSTextField::JsHeight(const JSCallbackInfo& info)
{
    JSViewAbstract::JsHeight(info);
    if (Container::IsCurrentUseNewPipeline()) {
        return;
    }
    if (info.Length() < 1) {
        LOGI("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }
    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        LOGI("Parse to dimension VP failed!");
        return;
    }
    if (LessNotEqual(value.Value(), 0.0)) {
        LOGI("dimension value: %{public}f is invalid!", value.Value());
        return;
    }
    auto* stack = ViewStackProcessor::GetInstance();
    auto textInputComponent = AceType::DynamicCast<TextFieldComponent>(stack->GetMainComponent());
    if (!textInputComponent) {
        LOGI("JSTextInput set height failed, textInputComponent is null.");
        return;
    }
    textInputComponent->SetHeight(value);
}

void JSTextField::JsWidth(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGW("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    if (info[0]->IsString() && info[0]->ToString() == "auto") {
        ViewAbstractModel::GetInstance()->ClearWidthOrHeight(true);
        TextFieldModel::GetInstance()->SetWidthAuto(true);
        return;
    }

    TextFieldModel::GetInstance()->SetWidthAuto(false);
    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        LOGW("Parse width fail");
        return;
    }
    if (LessNotEqual(value.Value(), 0.0)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetWidth(value);
}

bool CheckIsIllegalString(const std::string& value)
{
    if (value.empty()) {
        return true;
    }
    errno = 0;
    char* pEnd = nullptr;
    std::strtod(value.c_str(), &pEnd);
    return (pEnd == value.c_str() || errno == ERANGE);
}

void JSTextField::JsPadding(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        if (info[0]->IsUndefined() || (info[0]->IsString() && CheckIsIllegalString(info[0]->ToString()))) {
            return;
        };
        CalcDimension length;
        ParseJsDimensionVp(info[0], length);
        if (length.IsNegative()) {
            return;
        }
        JSViewAbstract::JsPadding(info);
        return;
    }
    if (!info[0]->IsString() && !info[0]->IsNumber() && !info[0]->IsObject()) {
        LOGI("arg is not a string, number or object.");
        return;
    }
    Edge padding;
    if (info[0]->IsNumber() || info[0]->IsString()) {
        CalcDimension edgeValue;
        if (ParseJsDimensionVp(info[0], edgeValue)) {
            padding = Edge(edgeValue);
        }
    }
    if (info[0]->IsObject()) {
        JSRef<JSObject> object = JSRef<JSObject>::Cast(info[0]);
        CalcDimension left = CalcDimension(0.0, DimensionUnit::VP);
        CalcDimension top = CalcDimension(0.0, DimensionUnit::VP);
        CalcDimension right = CalcDimension(0.0, DimensionUnit::VP);
        CalcDimension bottom = CalcDimension(0.0, DimensionUnit::VP);
        ParseJsDimensionVp(object->GetProperty("left"), left);
        ParseJsDimensionVp(object->GetProperty("top"), top);
        ParseJsDimensionVp(object->GetProperty("right"), right);
        ParseJsDimensionVp(object->GetProperty("bottom"), bottom);
        padding = Edge(left, top, right, bottom);
    }
    auto* stack = ViewStackProcessor::GetInstance();
    auto component = AceType::DynamicCast<TextFieldComponent>(stack->GetMainComponent());
    if (component) {
        auto decoration = component->GetDecoration();
        decoration->SetPadding(padding);
    }
}

void JSTextField::JsBorder(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSViewAbstract::JsBorder(info);
        return;
    }
    if (!info[0]->IsObject()) {
        LOGI("args is not a object. %s", info[0]->ToString().c_str());
        return;
    }
    RefPtr<Decoration> decoration = nullptr;
    auto component = AceType::DynamicCast<TextFieldComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (component) {
        decoration = component->GetDecoration();
    }
    JSRef<JSObject> object = JSRef<JSObject>::Cast(info[0]);
    auto valueWidth = object->GetProperty("width");
    if (!valueWidth->IsUndefined()) {
        ParseBorderWidth(valueWidth);
    }
    auto valueColor = object->GetProperty("color");
    if (!valueColor->IsUndefined()) {
        ParseBorderColor(valueColor);
    }
    auto valueRadius = object->GetProperty("radius");
    if (!valueRadius->IsUndefined()) {
        ParseBorderRadius(valueRadius);
    }
    auto valueStyle = object->GetProperty("style");
    if (!valueStyle->IsUndefined()) {
        ParseBorderStyle(valueStyle);
    }
    ViewAbstractModelImpl::SwapBackBorder(decoration);
    if (component) {
        component->SetOriginBorder(decoration->GetBorder());
    }
    info.ReturnSelf();
}

void JSTextField::JsBorderWidth(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSViewAbstract::JsBorderWidth(info);
        return;
    }
    if (!info[0]->IsObject() && !info[0]->IsString() && !info[0]->IsNumber()) {
        LOGI("args need a string or number or object");
        return;
    }
    RefPtr<Decoration> decoration = nullptr;
    auto component = AceType::DynamicCast<TextFieldComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (component) {
        decoration = component->GetDecoration();
    }
    JSViewAbstract::ParseBorderWidth(info[0]);
    ViewAbstractModelImpl::SwapBackBorder(decoration);
    if (component) {
        component->SetOriginBorder(decoration->GetBorder());
    }
}

void JSTextField::JsBorderColor(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSViewAbstract::JsBorderColor(info);
        return;
    }
    if (!info[0]->IsObject() && !info[0]->IsString() && !info[0]->IsNumber()) {
        LOGI("args need a string or number or object");
        return;
    }
    RefPtr<Decoration> decoration = nullptr;
    auto component = AceType::DynamicCast<TextFieldComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (component) {
        decoration = component->GetDecoration();
    }
    JSViewAbstract::ParseBorderColor(info[0]);
    ViewAbstractModelImpl::SwapBackBorder(decoration);
    if (component) {
        component->SetOriginBorder(decoration->GetBorder());
    }
}

void JSTextField::JsBorderStyle(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSViewAbstract::JsBorderStyle(info);
        return;
    }
    if (!info[0]->IsObject() && !info[0]->IsNumber()) {
        LOGI("args need a string or number or object");
        return;
    }
    RefPtr<Decoration> decoration = nullptr;
    auto component = AceType::DynamicCast<TextFieldComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (component) {
        decoration = component->GetDecoration();
    }
    JSViewAbstract::ParseBorderStyle(info[0]);
    ViewAbstractModelImpl::SwapBackBorder(decoration);
    if (component) {
        component->SetOriginBorder(decoration->GetBorder());
    }
}

void JSTextField::JsBorderRadius(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSViewAbstract::JsBorderRadius(info);
        return;
    }
    if (!info[0]->IsObject() && !info[0]->IsString() && !info[0]->IsNumber()) {
        LOGI("args need a string or number or object");
        return;
    }
    RefPtr<Decoration> decoration = nullptr;
    auto component = AceType::DynamicCast<TextFieldComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (component) {
        decoration = component->GetDecoration();
    }
    JSViewAbstract::ParseBorderRadius(info[0]);
    ViewAbstractModelImpl::SwapBackBorder(decoration);
    if (component) {
        component->SetOriginBorder(decoration->GetBorder());
    }
}

void JSTextField::JsHoverEffect(const JSCallbackInfo& info)
{
    if (!info[0]->IsNumber()) {
        LOGI("info[0] is not a number");
        return;
    }
    if (Container::IsCurrentUseNewPipeline()) {
        NG::ViewAbstract::SetHoverEffect(static_cast<HoverEffectType>(info[0]->ToNumber<int32_t>()));
        return;
    }
    auto component = AceType::DynamicCast<TextFieldComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (component) {
        component->SetHoverAnimationType(static_cast<HoverAnimationType>(info[0]->ToNumber<int32_t>()));
    }
}

void JSTextField::SetOnEditChanged(const JSCallbackInfo& info)
{
    CHECK_NULL_VOID(info[0]->IsFunction());
    JsEventCallback<void(bool)> callback(info.GetExecutionContext(), JSRef<JSFunc>::Cast(info[0]));
    TextFieldModel::GetInstance()->SetOnEditChanged(std::move(callback));
}

void JSTextField::SetOnSubmit(const JSCallbackInfo& info)
{
    CHECK_NULL_VOID(info[0]->IsFunction());
    JsEventCallback<void(int32_t)> callback(info.GetExecutionContext(), JSRef<JSFunc>::Cast(info[0]));
    TextFieldModel::GetInstance()->SetOnSubmit(std::move(callback));
}

void JSTextField::SetOnChange(const JSCallbackInfo& info)
{
    CHECK_NULL_VOID(info[0]->IsFunction());
    JsEventCallback<void(const std::string&)> callback(info.GetExecutionContext(), JSRef<JSFunc>::Cast(info[0]));
    TextFieldModel::GetInstance()->SetOnChange(std::move(callback));
}

void JSTextField::SetOnCopy(const JSCallbackInfo& info)
{
    CHECK_NULL_VOID(info[0]->IsFunction());
    JsEventCallback<void(const std::string&)> callback(info.GetExecutionContext(), JSRef<JSFunc>::Cast(info[0]));
    TextFieldModel::GetInstance()->SetOnCopy(std::move(callback));
}

void JSTextField::SetOnCut(const JSCallbackInfo& info)
{
    CHECK_NULL_VOID(info[0]->IsFunction());
    JsEventCallback<void(const std::string&)> callback(info.GetExecutionContext(), JSRef<JSFunc>::Cast(info[0]));
    TextFieldModel::GetInstance()->SetOnCut(std::move(callback));
}

void JSTextField::SetOnPaste(const JSCallbackInfo& info)
{
    CHECK_NULL_VOID(info[0]->IsFunction());
    JsEventCallback<void(const std::string&)> callback(info.GetExecutionContext(), JSRef<JSFunc>::Cast(info[0]));
    TextFieldModel::GetInstance()->SetOnPaste(std::move(callback));
}

void JSTextField::SetOnClick(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSInteractableView::JsOnClick(info);
        return;
    }
    if (!JSViewBindEvent(&TextFieldComponent::SetOnClick, info)) {
        LOGW("Failed(OnPaste) to bind event");
    }
    info.ReturnSelf();
}

void JSTextField::SetCopyOption(const JSCallbackInfo& info)
{
    if (info.Length() == 0) {
        return;
    }
    auto copyOptions = CopyOptions::None;
    if (info[0]->IsNumber()) {
        auto emunNumber = info[0]->ToNumber<int>();
        copyOptions = static_cast<CopyOptions>(emunNumber);
    }
    TextFieldModel::GetInstance()->SetCopyOption(copyOptions);
}

void JSTextField::JsMenuOptionsExtension(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        if (info[0]->IsArray()) {
            std::vector<NG::MenuOptionsParam> menuOptionsItems;
            JSViewAbstract::ParseMenuOptions(info, JSRef<JSArray>::Cast(info[0]), menuOptionsItems);
            TextFieldModel::GetInstance()->SetMenuOptionItems(std::move(menuOptionsItems));
        }
    } else {
        LOGI("only newPipeline supply");
    }
}

void JSTextField::SetShowUnderline(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        if (!info[0]->IsBoolean()) {
            LOGI("The info is wrong, it is supposed to be an boolean");
            return;
        }
        TextFieldModel::GetInstance()->SetShowUnderline(info[0]->ToBoolean());
    }
}

void JSTextField::UpdateDecoration(const RefPtr<BoxComponent>& boxComponent,
    const RefPtr<TextFieldComponent>& component, const Border& boxBorder,
    const OHOS::Ace::RefPtr<OHOS::Ace::TextFieldTheme>& textFieldTheme)
{
    if (!textFieldTheme) {
        LOGI("UpdateDecoration: textFieldTheme is null.");
        return;
    }

    RefPtr<Decoration> decoration = component->GetDecoration();
    RefPtr<Decoration> boxDecoration = boxComponent->GetBackDecoration();
    if (!decoration) {
        decoration = AceType::MakeRefPtr<Decoration>();
    }
    if (boxDecoration) {
        Border border = decoration->GetBorder();
        border.SetLeftEdge(boxBorder.Left());
        border.SetRightEdge(boxBorder.Right());
        border.SetTopEdge(boxBorder.Top());
        border.SetBottomEdge(boxBorder.Bottom());
        border.SetBorderRadius(textFieldTheme->GetBorderRadius());
        decoration->SetBorder(border);
        component->SetOriginBorder(decoration->GetBorder());

        if (boxDecoration->GetImage() || boxDecoration->GetGradient().IsValid()) {
            // clear box properties except background image and radius.
            boxDecoration->SetBackgroundColor(Color::TRANSPARENT);
            Border border;
            border.SetBorderRadius(textFieldTheme->GetBorderRadius());
            boxDecoration->SetBorder(border);
        }
    } else {
        boxDecoration = AceType::MakeRefPtr<Decoration>();
        boxDecoration->SetBorderRadius(textFieldTheme->GetBorderRadius());
        boxComponent->SetBackDecoration(boxDecoration);
    }
}

void JSTextField::SetShowUnit(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        if (!info[0]->IsFunction()) {
            LOGI("fail to bind SetShowUnit event due to info is not object");
            return;
        }

        auto builderFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSFunc>::Cast(info[0]));
        auto unitFunc = [builderFunc]() { builderFunc->Execute(); };
        TextFieldModel::GetInstance()->SetShowUnit(std::move(unitFunc));
    }
}

void JSTextField::SetShowCounter(const JSCallbackInfo& info)
{
    if (!info[0]->IsBoolean()) {
        LOGI("The info is wrong, it is supposed to be an boolean");
        return;
    }

    TextFieldModel::GetInstance()->SetShowCounter(info[0]->ToBoolean());
}

} // namespace OHOS::Ace::Framework
