/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "bridge/declarative_frontend/jsview/js_datepicker.h"

#include <utility>

#include "base/log/ace_scoring_log.h"
#include "base/utils/utils.h"
#include "bridge/common/utils/engine_helper.h"
#include "bridge/declarative_frontend/engine/functions/js_function.h"
#include "bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "bridge/declarative_frontend/jsview/js_utils.h"
#include "bridge/declarative_frontend/jsview/js_view_common_def.h"
#include "bridge/declarative_frontend/jsview/models/picker_model_impl.h"
#include "bridge/declarative_frontend/jsview/models/timepicker_model_impl.h"
#include "bridge/declarative_frontend/view_stack_processor.h"
#include "core/components/picker/picker_data.h"
#include "core/components/picker/picker_date_component.h"
#include "core/components/picker/picker_theme.h"
#include "core/components/picker/picker_time_component.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/picker/datepicker_model_ng.h"
#include "core/components_ng/pattern/time_picker/timepicker_model.h"
#include "core/components_ng/pattern/time_picker/timepicker_model_ng.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/ace_event_helper.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/bridge/declarative_frontend/ark_theme/theme_apply/js_date_picker_theme.h"
#include "frameworks/bridge/declarative_frontend/ark_theme/theme_apply/js_time_picker_theme.h"

namespace OHOS::Ace {
namespace {
const DimensionOffset DATEPICKER_OFFSET_DEFAULT_TOP = DimensionOffset(0.0_vp, 40.0_vp);
const std::vector<DialogAlignment> DIALOG_ALIGNMENT = { DialogAlignment::TOP, DialogAlignment::CENTER,
    DialogAlignment::BOTTOM, DialogAlignment::DEFAULT, DialogAlignment::TOP_START, DialogAlignment::TOP_END,
    DialogAlignment::CENTER_START, DialogAlignment::CENTER_END, DialogAlignment::BOTTOM_START,
    DialogAlignment::BOTTOM_END };
const char TIMEPICKER_OPTIONS_HOUR[] = "hour";
const char TIMEPICKER_OPTIONS_MINUTE[] = "minute";
const char TIMEPICKER_OPTIONS_SECOND[] = "second";
const std::string TIMEPICKER_OPTIONS_NUMERIC_VAL = "numeric";
const std::string TIMEPICKER_OPTIONS_TWO_DIGIT_VAL = "2-digit";
// difference in shanghai time zone changes
const int32_t TZDB_CHANGE_MILLISECOND = 343000;
} // namespace

std::unique_ptr<DatePickerModel> DatePickerModel::datePickerInstance_ = nullptr;
std::unique_ptr<DatePickerDialogModel> DatePickerDialogModel::datePickerDialogInstance_ = nullptr;
std::unique_ptr<TimePickerModel> TimePickerModel::timePickerInstance_ = nullptr;
std::unique_ptr<TimePickerDialogModel> TimePickerDialogModel::timePickerDialogInstance_ = nullptr;
std::mutex DatePickerModel::mutex_;
std::mutex DatePickerDialogModel::mutex_;
std::mutex TimePickerModel::mutex_;
std::mutex TimePickerDialogModel::mutex_;

DatePickerModel* DatePickerModel::GetInstance()
{
    if (!datePickerInstance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!datePickerInstance_) {
#ifdef NG_BUILD
            datePickerInstance_.reset(new NG::DatePickerModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                datePickerInstance_.reset(new NG::DatePickerModelNG());
            } else {
                datePickerInstance_.reset(new Framework::DatePickerModelImpl());
            }
#endif
        }
    }
    return datePickerInstance_.get();
}

DatePickerDialogModel* DatePickerDialogModel::GetInstance()
{
    if (!datePickerDialogInstance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!datePickerDialogInstance_) {
#ifdef NG_BUILD
            datePickerDialogInstance_.reset(new NG::DatePickerDialogModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                datePickerDialogInstance_.reset(new NG::DatePickerDialogModelNG());
            } else {
                datePickerDialogInstance_.reset(new Framework::DatePickerDialogModelImpl());
            }
#endif
        }
    }
    return datePickerDialogInstance_.get();
}

TimePickerModel* TimePickerModel::GetInstance()
{
    if (!timePickerInstance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!timePickerInstance_) {
#ifdef NG_BUILD
            timePickerInstance_.reset(new NG::TimePickerModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                timePickerInstance_.reset(new NG::TimePickerModelNG());
            } else {
                timePickerInstance_.reset(new Framework::TimePickerModelImpl());
            }
#endif
        }
    }
    return timePickerInstance_.get();
}

TimePickerDialogModel* TimePickerDialogModel::GetInstance()
{
    if (!timePickerDialogInstance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!timePickerDialogInstance_) {
#ifdef NG_BUILD
            timePickerDialogInstance_.reset(new NG::TimePickerDialogModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                timePickerDialogInstance_.reset(new NG::TimePickerDialogModelNG());
            } else {
                timePickerDialogInstance_.reset(new Framework::TimePickerDialogModelImpl());
            }
#endif
        }
    }
    return timePickerDialogInstance_.get();
}
} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {
namespace {
JSRef<JSVal> DatePickerChangeEventToJSValue(const DatePickerChangeEvent& eventInfo)
{
    JSRef<JSObject> obj = JSRef<JSObject>::New();
    std::unique_ptr<JsonValue> argsPtr = JsonUtil::ParseJsonString(eventInfo.GetSelectedStr());
    if (!argsPtr) {
        return JSRef<JSVal>::Cast(obj);
    }
    std::vector<std::string> keys = { "year", "month", "day", "hour", "minute", "second" };
    for (auto iter = keys.begin(); iter != keys.end(); iter++) {
        const std::string key = *iter;
        const auto value = argsPtr->GetValue(key);
        if (!value || value->ToString().empty()) {
            continue;
        }
        obj->SetProperty<int32_t>(key.c_str(), value->GetInt());
    }
    return JSRef<JSVal>::Cast(obj);
}

JSRef<JSVal> DatePickerDateChangeEventToJSValue(const DatePickerChangeEvent& eventInfo)
{
    JSRef<JSObject> obj = JSRef<JSObject>::New();
    std::unique_ptr<JsonValue> argsPtr = JsonUtil::ParseJsonString(eventInfo.GetSelectedStr());
    if (!argsPtr) {
        return JSRef<JSVal>::Cast(obj);
    }
    auto dateObj = JSDatePickerDialog::GetDateObj(argsPtr);
    return JSRef<JSVal>::Cast(dateObj);
}

void ParseFontOfButtonStyle(const JSRef<JSObject>& pickerButtonParamObject, ButtonInfo& buttonInfo)
{
    CalcDimension fontSize;
    JSRef<JSVal> sizeProperty = pickerButtonParamObject->GetProperty("fontSize");
    if (JSViewAbstract::ParseJsDimensionVpNG(sizeProperty, fontSize) && fontSize.Unit() != DimensionUnit::PERCENT &&
        GreatOrEqual(fontSize.Value(), 0.0)) {
        if (JSViewAbstract::ParseJsDimensionFp(sizeProperty, fontSize)) {
            buttonInfo.fontSize = fontSize;
        }
    }
    Color fontColor;
    if (JSViewAbstract::ParseJsColor(pickerButtonParamObject->GetProperty("fontColor"), fontColor)) {
        buttonInfo.fontColor = fontColor;
    }
    auto fontWeight = pickerButtonParamObject->GetProperty("fontWeight");
    if (fontWeight->IsString() || fontWeight->IsNumber()) {
        buttonInfo.fontWeight = ConvertStrToFontWeight(fontWeight->ToString(), FontWeight::MEDIUM);
    }
    JSRef<JSVal> style = pickerButtonParamObject->GetProperty("fontStyle");
    if (style->IsNumber()) {
        auto value = style->ToNumber<int32_t>();
        if (value >= 0 && value < static_cast<int32_t>(FontStyle::NONE)) {
            buttonInfo.fontStyle = static_cast<FontStyle>(value);
        }
    }
    JSRef<JSVal> family = pickerButtonParamObject->GetProperty("fontFamily");
    std::vector<std::string> fontFamilies;
    if (JSViewAbstract::ParseJsFontFamilies(family, fontFamilies)) {
        buttonInfo.fontFamily = fontFamilies;
    }
}

ButtonInfo ParseButtonStyle(const JSRef<JSObject>& pickerButtonParamObject)
{
    ButtonInfo buttonInfo;
    if (pickerButtonParamObject->GetProperty("type")->IsNumber()) {
        buttonInfo.type =
            static_cast<ButtonType>(pickerButtonParamObject->GetProperty("type")->ToNumber<int32_t>());
    }
    if (pickerButtonParamObject->GetProperty("style")->IsNumber()) {
        auto styleModeIntValue = pickerButtonParamObject->GetProperty("style")->ToNumber<int32_t>();
        if (styleModeIntValue >= static_cast<int32_t>(ButtonStyleMode::NORMAL) &&
            styleModeIntValue <= static_cast<int32_t>(ButtonStyleMode::TEXT)) {
            buttonInfo.buttonStyle = static_cast<ButtonStyleMode>(styleModeIntValue);
        }
    }
    if (pickerButtonParamObject->GetProperty("role")->IsNumber()) {
        auto buttonRoleIntValue = pickerButtonParamObject->GetProperty("role")->ToNumber<int32_t>();
        if (buttonRoleIntValue >= static_cast<int32_t>(ButtonRole::NORMAL) &&
            buttonRoleIntValue <= static_cast<int32_t>(ButtonRole::ERROR)) {
            buttonInfo.role = static_cast<ButtonRole>(buttonRoleIntValue);
        }
    }
    ParseFontOfButtonStyle(pickerButtonParamObject, buttonInfo);
    Color backgroundColor;
    if (JSViewAbstract::ParseJsColor(pickerButtonParamObject->GetProperty("backgroundColor"), backgroundColor)) {
        buttonInfo.backgroundColor = backgroundColor;
    }
    auto radius = ParseBorderRadiusAttr(pickerButtonParamObject->GetProperty("borderRadius"));
    if (radius.has_value()) {
        buttonInfo.borderRadius = radius.value();
    }

    auto primaryValue = pickerButtonParamObject->GetProperty("primary");
    if (primaryValue->IsBoolean()) {
        buttonInfo.isPrimary = primaryValue->ToBoolean();
    }

    return buttonInfo;
}

std::vector<ButtonInfo> ParseButtonStyles(const JSRef<JSObject>& paramObject)
{
    std::vector<ButtonInfo> buttonInfos;
    auto acceptButtonStyle = paramObject->GetProperty("acceptButtonStyle");
    if (acceptButtonStyle->IsObject()) {
        auto acceptButtonStyleParamObject = JSRef<JSObject>::Cast(acceptButtonStyle);
        buttonInfos.emplace_back(ParseButtonStyle(acceptButtonStyleParamObject));
        buttonInfos[0].isAcceptButton = true;
    } else {
        ButtonInfo buttonInfo;
        buttonInfos.emplace_back(buttonInfo);
    }
    auto cancelButtonStyle = paramObject->GetProperty("cancelButtonStyle");
    if (cancelButtonStyle->IsObject()) {
        auto cancelButtonStyleParamObject = JSRef<JSObject>::Cast(cancelButtonStyle);
        buttonInfos.emplace_back(ParseButtonStyle(cancelButtonStyleParamObject));
    }

    return buttonInfos;
}
} // namespace

void JSDatePicker::JSBind(BindingTarget globalObj)
{
    JSClass<JSDatePicker>::Declare("DatePicker");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSDatePicker>::StaticMethod("create", &JSDatePicker::Create, opt);
    JSClass<JSDatePicker>::StaticMethod("lunar", &JSDatePicker::SetLunar);
    JSClass<JSDatePicker>::StaticMethod("onChange", &JSDatePicker::OnChange);
    JSClass<JSDatePicker>::StaticMethod("onDateChange", &JSDatePicker::OnDateChange);
    JSClass<JSDatePicker>::StaticMethod("backgroundColor", &JSDatePicker::PickerBackgroundColor);
    // keep compatible, need remove after
    JSClass<JSDatePicker>::StaticMethod("useMilitaryTime", &JSDatePicker::UseMilitaryTime);
    JSClass<JSDatePicker>::StaticMethod("onClick", &JSInteractableView::JsOnClick);
    JSClass<JSDatePicker>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSDatePicker>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSDatePicker>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSDatePicker>::StaticMethod("onAttach", &JSInteractableView::JsOnAttach);
    JSClass<JSDatePicker>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSDatePicker>::StaticMethod("onDetach", &JSInteractableView::JsOnDetach);
    JSClass<JSDatePicker>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSDatePicker>::StaticMethod("disappearTextStyle", &JSDatePicker::SetDisappearTextStyle);
    JSClass<JSDatePicker>::StaticMethod("textStyle", &JSDatePicker::SetTextStyle);
    JSClass<JSDatePicker>::StaticMethod("selectedTextStyle", &JSDatePicker::SetSelectedTextStyle);
    JSClass<JSDatePicker>::InheritAndBind<JSViewAbstract>(globalObj);
}

void JSDatePicker::Create(const JSCallbackInfo& info)
{
    DatePickerType pickerType = DatePickerType::DATE;
    JSRef<JSObject> paramObject;
    if (info.Length() >= 1 && info[0]->IsObject()) {
        paramObject = JSRef<JSObject>::Cast(info[0]);
        auto type = paramObject->GetProperty("type");
        if (type->IsNumber()) {
            pickerType = static_cast<DatePickerType>(type->ToNumber<int32_t>());
        }
    }
    switch (pickerType) {
        case DatePickerType::TIME: {
            CreateTimePicker(info, paramObject);
            break;
        }
        case DatePickerType::DATE: {
            CreateDatePicker(info, paramObject);
            break;
        }
        default: {
            break;
        }
    }
}

void JSDatePicker::SetLunar(bool isLunar)
{
    DatePickerModel::GetInstance()->SetShowLunar(isLunar);
}

void JSDatePicker::UseMilitaryTime(bool isUseMilitaryTime)
{
    DatePickerModel::GetInstance()->SetHour24(isUseMilitaryTime);
}

void JSDatePicker::ParseTextProperties(const JSRef<JSObject>& paramObj, NG::PickerTextProperties& result)
{
    auto disappearProperty = paramObj->GetProperty("disappearTextStyle");
    auto normalProperty = paramObj->GetProperty("textStyle");
    auto selectedProperty = paramObj->GetProperty("selectedTextStyle");

    if (!disappearProperty->IsNull() && disappearProperty->IsObject()) {
        JSRef<JSObject> disappearObj = JSRef<JSObject>::Cast(disappearProperty);
        JSDatePicker::ParseTextStyle(disappearObj, result.disappearTextStyle_, "disappearTextStyle");
    }

    if (!normalProperty->IsNull() && normalProperty->IsObject()) {
        JSRef<JSObject> noramlObj = JSRef<JSObject>::Cast(normalProperty);
        JSDatePicker::ParseTextStyle(noramlObj, result.normalTextStyle_, "textStyle");
    }

    if (!selectedProperty->IsNull() && selectedProperty->IsObject()) {
        JSRef<JSObject> selectedObj = JSRef<JSObject>::Cast(selectedProperty);
        JSDatePicker::ParseTextStyle(selectedObj, result.selectedTextStyle_, "selectedTextStyle");
    }
}

void JSDatePicker::IsUserDefinedFontFamily(const std::string& pos)
{
    if (pos == "disappearTextStyle") {
        DatePickerModel::GetInstance()->HasUserDefinedDisappearFontFamily(true);
    } else if (pos == "textStyle") {
        DatePickerModel::GetInstance()->HasUserDefinedNormalFontFamily(true);
    } else if (pos == "selectedTextStyle") {
        DatePickerModel::GetInstance()->HasUserDefinedSelectedFontFamily(true);
    } else if (pos == "disappearTextStyleTime") {
        TimePickerModel::GetInstance()->HasUserDefinedDisappearFontFamily(true);
    } else if (pos == "textStyleTime") {
        TimePickerModel::GetInstance()->HasUserDefinedNormalFontFamily(true);
    } else if (pos == "selectedTextStyleTime") {
        TimePickerModel::GetInstance()->HasUserDefinedSelectedFontFamily(true);
    }
}

void JSDatePicker::ParseTextStyle(
    const JSRef<JSObject>& paramObj, NG::PickerTextStyle& textStyle, const std::string& pos)
{
    auto fontColor = paramObj->GetProperty("color");
    auto fontOptions = paramObj->GetProperty("font");

    Color textColor;
    if (JSViewAbstract::ParseJsColor(fontColor, textColor)) {
        textStyle.textColor = textColor;
    }

    if (!fontOptions->IsObject()) {
        return;
    }
    JSRef<JSObject> fontObj = JSRef<JSObject>::Cast(fontOptions);
    auto fontSize = fontObj->GetProperty("size");
    auto fontWeight = fontObj->GetProperty("weight");
    auto fontFamily = fontObj->GetProperty("family");
    auto fontStyle = fontObj->GetProperty("style");
    if (fontSize->IsNull() || fontSize->IsUndefined()) {
        textStyle.fontSize = Dimension(-1);
    } else {
        CalcDimension size;
        if (!ParseJsDimensionFp(fontSize, size) || size.Unit() == DimensionUnit::PERCENT) {
            textStyle.fontSize = Dimension(-1);
        } else {
            textStyle.fontSize = size;
        }
    }

    if (!fontWeight->IsNull() && !fontWeight->IsUndefined()) {
        std::string weight;
        if (fontWeight->IsNumber()) {
            weight = std::to_string(fontWeight->ToNumber<int32_t>());
        } else {
            ParseJsString(fontWeight, weight);
        }
        textStyle.fontWeight = ConvertStrToFontWeight(weight);
    }

    if (!fontFamily->IsNull() && !fontFamily->IsUndefined()) {
        std::vector<std::string> families;
        if (ParseJsFontFamilies(fontFamily, families)) {
            textStyle.fontFamily = families;
            IsUserDefinedFontFamily(pos);
        }
    }

    if (fontStyle->IsNumber()) {
        auto style = fontStyle->ToNumber<int32_t>();
        if (style < 0 || style > 1) {
            return;
        }
        textStyle.fontStyle = static_cast<FontStyle>(style);
    }
}

void JSDatePicker::SetDisappearTextStyle(const JSCallbackInfo& info)
{
    auto theme = GetTheme<PickerTheme>();
    CHECK_NULL_VOID(theme);
    NG::PickerTextStyle textStyle;
    JSDatePickerTheme::ObtainTextStyle(textStyle);
    if (info[0]->IsObject()) {
        JSDatePicker::ParseTextStyle(info[0], textStyle, "disappearTextStyle");
    }
    DatePickerModel::GetInstance()->SetDisappearTextStyle(theme, textStyle);
}

void JSDatePicker::SetTextStyle(const JSCallbackInfo& info)
{
    auto theme = GetTheme<PickerTheme>();
    CHECK_NULL_VOID(theme);
    NG::PickerTextStyle textStyle;
    JSDatePickerTheme::ObtainTextStyle(textStyle);
    if (info[0]->IsObject()) {
        JSDatePicker::ParseTextStyle(info[0], textStyle, "textStyle");
    }
    DatePickerModel::GetInstance()->SetNormalTextStyle(theme, textStyle);
}

void JSDatePicker::SetSelectedTextStyle(const JSCallbackInfo& info)
{
    auto theme = GetTheme<PickerTheme>();
    CHECK_NULL_VOID(theme);
    NG::PickerTextStyle textStyle;
    JSDatePickerTheme::ObtainSelectedTextStyle(textStyle);
    if (info[0]->IsObject()) {
        JSDatePicker::ParseTextStyle(info[0], textStyle, "selectedTextStyle");
    }
    DatePickerModel::GetInstance()->SetSelectedTextStyle(theme, textStyle);
}

void JSDatePicker::OnChange(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }

    auto jsFunc = AceType::MakeRefPtr<JsEventFunction<DatePickerChangeEvent, 1>>(
        JSRef<JSFunc>::Cast(info[0]), DatePickerChangeEventToJSValue);
    WeakPtr<NG::FrameNode> targetNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto onChange = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = targetNode](
                        const BaseEventInfo* info) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("datePicker.onChange");
        PipelineContext::SetCallBackNode(node);
        const auto* eventInfo = TypeInfoHelper::DynamicCast<DatePickerChangeEvent>(info);
        func->Execute(*eventInfo);
    };
    DatePickerModel::GetInstance()->SetOnChange(std::move(onChange));
}

void JSDatePicker::OnDateChange(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }
    auto jsFunc = AceType::MakeRefPtr<JsEventFunction<DatePickerChangeEvent, 1>>(
        JSRef<JSFunc>::Cast(info[0]), DatePickerDateChangeEventToJSValue);
    WeakPtr<NG::FrameNode> targetNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto onDateChange = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = targetNode](
                            const BaseEventInfo* info) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("datePicker.onDateChange");
        PipelineContext::SetCallBackNode(node);
        const auto* eventInfo = TypeInfoHelper::DynamicCast<DatePickerChangeEvent>(info);
        func->Execute(*eventInfo);
    };
    DatePickerModel::GetInstance()->SetOnDateChange(std::move(onDateChange));
}

void JSTimePicker::OnChange(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }

    auto jsFunc = AceType::MakeRefPtr<JsEventFunction<DatePickerChangeEvent, 1>>(
        JSRef<JSFunc>::Cast(info[0]), DatePickerChangeEventToJSValue);
    WeakPtr<NG::FrameNode> targetNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto onChange = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = targetNode](
                        const BaseEventInfo* index) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("datePicker.onChange");
        PipelineContext::SetCallBackNode(node);
        const auto* eventInfo = TypeInfoHelper::DynamicCast<DatePickerChangeEvent>(index);
        func->Execute(*eventInfo);
    };
    TimePickerModel::GetInstance()->SetOnChange(std::move(onChange));
}

void JSDatePicker::PickerBackgroundColor(const JSCallbackInfo& info)
{
    JSViewAbstract::JsBackgroundColor(info);

    if (info.Length() < 1) {
        return;
    }
    Color backgroundColor;
    if (!ParseJsColor(info[0], backgroundColor)) {
        return;
    }
    DatePickerModel::GetInstance()->SetBackgroundColor(backgroundColor);
}

PickerDate JSDatePicker::ParseDate(const JSRef<JSVal>& dateVal)
{
    auto pickerDate = PickerDate();
    if (!dateVal->IsObject()) {
        return pickerDate;
    }
    auto dateObj = JSRef<JSObject>::Cast(dateVal);
    auto yearFuncJsVal = dateObj->GetProperty("getFullYear");
    auto monthFuncJsVal = dateObj->GetProperty("getMonth");
    auto dateFuncJsVal = dateObj->GetProperty("getDate");
    if (!(yearFuncJsVal->IsFunction() && monthFuncJsVal->IsFunction() && dateFuncJsVal->IsFunction())) {
        return pickerDate;
    }
    auto yearFunc = JSRef<JSFunc>::Cast(yearFuncJsVal);
    auto monthFunc = JSRef<JSFunc>::Cast(monthFuncJsVal);
    auto dateFunc = JSRef<JSFunc>::Cast(dateFuncJsVal);
    JSRef<JSVal> year = yearFunc->Call(dateObj);
    JSRef<JSVal> month = monthFunc->Call(dateObj);
    JSRef<JSVal> date = dateFunc->Call(dateObj);

    if (year->IsNumber() && month->IsNumber() && date->IsNumber()) {
        pickerDate.SetYear(year->ToNumber<int32_t>());
        pickerDate.SetMonth(month->ToNumber<int32_t>() + 1); // 0-11 means 1 to 12 months
        pickerDate.SetDay(date->ToNumber<int32_t>());
    }
    return pickerDate;
}

PickerTime JSDatePicker::ParseTime(const JSRef<JSVal>& timeVal)
{
    auto pickerTime = PickerTime();
    if (!timeVal->IsObject()) {
        return pickerTime;
    }
    auto timeObj = JSRef<JSObject>::Cast(timeVal);
    auto hourFuncJsVal = timeObj->GetProperty("getHours");
    auto minuteFuncJsVal = timeObj->GetProperty("getMinutes");
    auto secondFuncJsVal = timeObj->GetProperty("getSeconds");
    if (!(hourFuncJsVal->IsFunction() && minuteFuncJsVal->IsFunction() && secondFuncJsVal->IsFunction())) {
        return pickerTime;
    }
    auto hourFunc = JSRef<JSFunc>::Cast(hourFuncJsVal);
    auto minuteFunc = JSRef<JSFunc>::Cast(minuteFuncJsVal);
    auto secondFunc = JSRef<JSFunc>::Cast(secondFuncJsVal);
    JSRef<JSVal> hour = hourFunc->Call(timeObj);
    JSRef<JSVal> minute = minuteFunc->Call(timeObj);
    JSRef<JSVal> second = secondFunc->Call(timeObj);

    if (hour->IsNumber() && minute->IsNumber() && second->IsNumber()) {
        pickerTime.SetHour(hour->ToNumber<int32_t>());
        pickerTime.SetMinute(minute->ToNumber<int32_t>());
        pickerTime.SetSecond(second->ToNumber<int32_t>());
    }
    return pickerTime;
}

void ParseSelectedDateTimeObject(const JSCallbackInfo& info, const JSRef<JSObject>& selectedObject, bool isDatePicker)
{
    JSRef<JSVal> changeEventVal = selectedObject->GetProperty("changeEvent");
    if (changeEventVal->IsUndefined() || !changeEventVal->IsFunction()) {
        return;
    }
    auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(changeEventVal));
    WeakPtr<NG::FrameNode> targetNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto changeEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = targetNode](
                           const BaseEventInfo* info) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("DatePicker.SelectedDateTimeChangeEvent");
        const auto* eventInfo = TypeInfoHelper::DynamicCast<DatePickerChangeEvent>(info);
        CHECK_NULL_VOID(eventInfo);
        auto selectedStr = eventInfo->GetSelectedStr();
        auto sourceJson = JsonUtil::ParseJsonString(selectedStr);
        if (!sourceJson || sourceJson->IsNull()) {
            return;
        }

        auto dateObj = JSDatePickerDialog::GetDateObj(sourceJson);
        PipelineContext::SetCallBackNode(node);
        func->ExecuteJS(1, &dateObj);
    };
    if (isDatePicker) {
        DatePickerModel::GetInstance()->SetChangeEvent(std::move(changeEvent));
    } else {
        TimePickerModel::GetInstance()->SetChangeEvent(std::move(changeEvent));
    }
}

void JSDatePicker::CreateDatePicker(const JSCallbackInfo& info, const JSRef<JSObject>& paramObj)
{
    auto theme = GetTheme<PickerTheme>();
    CHECK_NULL_VOID(theme);
    JSRef<JSVal> startDate;
    JSRef<JSVal> endDate;
    JSRef<JSVal> selectedDate;
    if (!paramObj->IsUndefined()) {
        startDate = paramObj->GetProperty("start");
        endDate = paramObj->GetProperty("end");
        selectedDate = paramObj->GetProperty("selected");
    }
    auto parseStartDate = ParseDate(startDate);
    auto parseEndDate = ParseDate(endDate);
    if (parseStartDate.GetYear() <= 0) {
        parseStartDate = theme->GetDefaultStartDate();
    }
    if (parseEndDate.GetYear() <= 0) {
        parseEndDate = theme->GetDefaultEndDate();
    }
    auto startDays = parseStartDate.ToDays();
    auto endDays = parseEndDate.ToDays();
    if (startDays > endDays) {
        parseStartDate = theme->GetDefaultStartDate();
        parseEndDate = theme->GetDefaultEndDate();
    }
    DatePickerModel::GetInstance()->CreateDatePicker(theme);
    if (startDate->IsObject()) {
        DatePickerModel::GetInstance()->SetStartDate(parseStartDate);
    }
    if (endDate->IsObject()) {
        DatePickerModel::GetInstance()->SetEndDate(parseEndDate);
    }
    if (selectedDate->IsObject()) {
        JSRef<JSObject> selectedDateObj = JSRef<JSObject>::Cast(selectedDate);
        JSRef<JSVal> changeEventVal = selectedDateObj->GetProperty("changeEvent");
        PickerDate parseSelectedDate;
        if (!changeEventVal->IsUndefined() && changeEventVal->IsFunction()) {
            ParseSelectedDateTimeObject(info, selectedDateObj, true);
            parseSelectedDate = ParseDate(selectedDateObj->GetProperty("value"));
        } else {
            parseSelectedDate = ParseDate(selectedDate);
        }
        DatePickerModel::GetInstance()->SetSelectedDate(parseSelectedDate);
    }
    if (!JSDatePickerTheme::ApplyTheme()) {
        SetDefaultAttributes();
    }
}

void JSDatePicker::SetDefaultAttributes()
{
    auto theme = GetTheme<PickerTheme>();
    CHECK_NULL_VOID(theme);
    NG::PickerTextStyle textStyle;
    auto selectedStyle = theme->GetOptionStyle(true, false);
    textStyle.textColor = selectedStyle.GetTextColor();
    textStyle.fontSize = selectedStyle.GetFontSize();
    textStyle.fontWeight = selectedStyle.GetFontWeight();
    DatePickerModel::GetInstance()->SetSelectedTextStyle(theme, textStyle);

    auto disappearStyle = theme->GetDisappearOptionStyle();
    textStyle.textColor = disappearStyle.GetTextColor();
    textStyle.fontSize = disappearStyle.GetFontSize();
    textStyle.fontWeight = disappearStyle.GetFontWeight();
    DatePickerModel::GetInstance()->SetDisappearTextStyle(theme, textStyle);

    auto normalStyle = theme->GetOptionStyle(false, false);
    textStyle.textColor = normalStyle.GetTextColor();
    textStyle.fontSize = normalStyle.GetFontSize();
    textStyle.fontWeight = normalStyle.GetFontWeight();
    DatePickerModel::GetInstance()->SetNormalTextStyle(theme, textStyle);
}

void JSDatePicker::CreateTimePicker(const JSCallbackInfo& info, const JSRef<JSObject>& paramObj)
{
    auto theme = GetTheme<PickerTheme>();
    CHECK_NULL_VOID(theme);
    DatePickerModel::GetInstance()->CreateTimePicker(theme);
    auto selectedTime = paramObj->GetProperty("selected");
    if (selectedTime->IsObject()) {
        JSRef<JSObject> selectedTimeObj = JSRef<JSObject>::Cast(selectedTime);
        JSRef<JSVal> changeEventVal = selectedTimeObj->GetProperty("changeEvent");
        if (!changeEventVal->IsUndefined() && changeEventVal->IsFunction()) {
            ParseSelectedDateTimeObject(info, selectedTimeObj, true);
            auto parseSelectedTime = ParseTime(selectedTimeObj->GetProperty("value"));
            DatePickerModel::GetInstance()->SetSelectedTime(parseSelectedTime);
        } else {
            DatePickerModel::GetInstance()->SetSelectedTime(ParseTime(selectedTime));
        }
    }
}

void JSDatePickerDialog::JSBind(BindingTarget globalObj)
{
    JSClass<JSDatePickerDialog>::Declare("DatePickerDialog");
    JSClass<JSDatePickerDialog>::StaticMethod("show", &JSDatePickerDialog::Show);

    JSClass<JSDatePickerDialog>::Bind<>(globalObj);
}

void DatePickerDialogAppearEvent(const JSCallbackInfo& info, PickerDialogEvent& pickerDialogEvent)
{
    std::function<void()> didAppearEvent;
    std::function<void()> willAppearEvent;
    if (info.Length() == 0 || !info[0]->IsObject()) {
        return;
    }
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    WeakPtr<NG::FrameNode> frameNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto onDidAppear = paramObject->GetProperty("onDidAppear");
    if (!onDidAppear->IsUndefined() && onDidAppear->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onDidAppear));
        didAppearEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = frameNode]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("DatePickerDialog.onDidAppear");
            PipelineContext::SetCallBackNode(node);
            func->Execute();
        };
    }
    auto onWillAppear = paramObject->GetProperty("onWillAppear");
    if (!onWillAppear->IsUndefined() && onWillAppear->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onWillAppear));
        willAppearEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = frameNode]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("DatePickerDialog.onWillAppear");
            PipelineContext::SetCallBackNode(node);
            func->Execute();
        };
    }
    pickerDialogEvent.onDidAppear = std::move(didAppearEvent);
    pickerDialogEvent.onWillAppear = std::move(willAppearEvent);
}

void DatePickerDialogDisappearEvent(const JSCallbackInfo& info, PickerDialogEvent& pickerDialogEvent)
{
    std::function<void()> didDisappearEvent;
    std::function<void()> willDisappearEvent;
    if (info.Length() == 0 || !info[0]->IsObject()) {
        return;
    }
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    WeakPtr<NG::FrameNode> frameNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto onDidDisappear = paramObject->GetProperty("onDidDisappear");
    if (!onDidDisappear->IsUndefined() && onDidDisappear->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onDidDisappear));
        didDisappearEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = frameNode]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("DatePickerDialog.onDidDisappear");
            PipelineContext::SetCallBackNode(node);
            func->Execute();
        };
    }
    auto onWillDisappear = paramObject->GetProperty("onWillDisappear");
    if (!onWillDisappear->IsUndefined() && onWillDisappear->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onWillDisappear));
        willDisappearEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = frameNode]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("DatePickerDialog.onWillDisappear");
            PipelineContext::SetCallBackNode(node);
            func->Execute();
        };
    }
    pickerDialogEvent.onDidDisappear = std::move(didDisappearEvent);
    pickerDialogEvent.onWillDisappear = std::move(willDisappearEvent);
}

std::function<void(const std::string&)> JSDatePickerDialog::GetDateChangeEvent(const JSRef<JSObject>& paramObject,
    const JSCallbackInfo& info, const DatePickerType& pickerType, const WeakPtr<NG::FrameNode>& frameNode)
{
    std::function<void(const std::string&)> dateChangeEvent;
    auto onDateChange = paramObject->GetProperty("onDateChange");
    if (!onDateChange->IsUndefined() && onDateChange->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onDateChange));
        dateChangeEvent = [execCtx = info.GetExecutionContext(), type = pickerType, func = std::move(jsFunc),
                              node = frameNode](const std::string& info) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("DatePickerDialog.onDateChange");
            auto selectedJson = JsonUtil::ParseJsonString(info);
            if (!selectedJson || selectedJson->IsNull()) {
                return;
            }
            auto dateObj = GetDateObj(selectedJson);
            PipelineContext::SetCallBackNode(node);
            func->ExecuteJS(1, &dateObj);
        };
    }
    return dateChangeEvent;
}

std::function<void(const std::string&)> JSDatePickerDialog::GetDateAcceptEvent(const JSRef<JSObject>& paramObject,
    const JSCallbackInfo& info, const DatePickerType& pickerType, const WeakPtr<NG::FrameNode>& frameNode)
{
    std::function<void(const std::string&)> dateAcceptEvent;
    auto onDateAccept = paramObject->GetProperty("onDateAccept");
    if (!onDateAccept->IsUndefined() && onDateAccept->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onDateAccept));
        dateAcceptEvent = [execCtx = info.GetExecutionContext(), type = pickerType, func = std::move(jsFunc),
                              node = frameNode](const std::string& info) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("DatePickerDialog.onDateAccept");
            auto selectedJson = JsonUtil::ParseJsonString(info);
            if (!selectedJson || selectedJson->IsNull()) {
                return;
            }
            auto dateObj = GetDateObj(selectedJson);
            PipelineContext::SetCallBackNode(node);
            func->ExecuteJS(1, &dateObj);
        };
    }
    return dateAcceptEvent;
}

JsiRef<JsiValue> JSDatePickerDialog::GetDateObj(const std::unique_ptr<JsonValue>& selectedJson)
{
    std::tm dateTime = { 0 };
    auto year = selectedJson->GetValue("year");
    if (year && year->IsNumber()) {
        dateTime.tm_year = year->GetInt() - 1900; // local date start from 1900
    }
    auto month = selectedJson->GetValue("month");
    if (month && month->IsNumber()) {
        dateTime.tm_mon = month->GetInt();
    }
    auto day = selectedJson->GetValue("day");
    if (day && day->IsNumber()) {
        dateTime.tm_mday = day->GetInt();
    }
    auto hour = selectedJson->GetValue("hour");
    if (hour && hour->IsNumber()) {
        dateTime.tm_hour = hour->GetInt();
    }
    auto minute = selectedJson->GetValue("minute");
    if (minute && minute->IsNumber()) {
        dateTime.tm_min = minute->GetInt();
    }
    auto timestamp = std::chrono::system_clock::from_time_t(std::mktime(&dateTime));
    auto duration = timestamp.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    if (dateTime.tm_year <= 0) {
        milliseconds += TZDB_CHANGE_MILLISECOND;
    }
    auto dateObj = JSDate::New(milliseconds);
    return dateObj;
}

std::function<void(const std::string&)> JSDatePickerDialog::GetChangeEvent(const JSRef<JSObject>& paramObject,
    const JSCallbackInfo& info, const DatePickerType& pickerType, const WeakPtr<NG::FrameNode>& frameNode)
{
    std::function<void(const std::string&)> changeEvent;
    auto onChange = paramObject->GetProperty("onChange");
    if (!onChange->IsUndefined() && onChange->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onChange));
        changeEvent = [execCtx = info.GetExecutionContext(), type = pickerType, func = std::move(jsFunc),
                          node = frameNode](const std::string& info) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            std::vector<std::string> keys;
            keys = { "year", "month", "day" };
            ACE_SCORING_EVENT("DatePickerDialog.onChange");
            PipelineContext::SetCallBackNode(node);
            func->Execute(keys, info);
        };
    }
    return changeEvent;
}

std::function<void(const std::string&)> JSDatePickerDialog::GetAcceptEvent(
    const JSRef<JSObject>& paramObject, const JSCallbackInfo& info, const WeakPtr<NG::FrameNode>& frameNode)
{
    std::function<void(const std::string&)> acceptEvent;
    auto onAccept = paramObject->GetProperty("onAccept");
    if (!onAccept->IsUndefined() && onAccept->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onAccept));
        acceptEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = frameNode](
                          const std::string& info) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            std::vector<std::string> keys = { "year", "month", "day", "hour", "minute", "second" };
            ACE_SCORING_EVENT("DatePickerDialog.onAccept");
            PipelineContext::SetCallBackNode(node);
            func->Execute(keys, info);
        };
    }
    return acceptEvent;
}

std::function<void()> JSDatePickerDialog::GetCancelEvent(
    const JSRef<JSObject>& paramObject, const JSCallbackInfo& info, const WeakPtr<NG::FrameNode>& frameNode)
{
    std::function<void()> cancelEvent;
    auto onCancel = paramObject->GetProperty("onCancel");
    if (!onCancel->IsUndefined() && onCancel->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onCancel));
        cancelEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = frameNode]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("DatePickerDialog.onCancel");
            PipelineContext::SetCallBackNode(node);
            func->Execute();
        };
    }
    return cancelEvent;
}

void JSDatePickerDialog::UpdateDatePickerSettingData(
    const JSRef<JSObject>& paramObject, NG::DatePickerSettingData& settingData)
{
    auto lunar = paramObject->GetProperty("lunar");
    auto lunarSwitch = paramObject->GetProperty("lunarSwitch");
    auto sTime = paramObject->GetProperty("showTime");
    auto useMilitary = paramObject->GetProperty("useMilitaryTime");
    settingData.isLunar = lunar->ToBoolean();
    settingData.lunarswitch = lunarSwitch->ToBoolean();
    settingData.showTime = sTime->ToBoolean();
    settingData.useMilitary = useMilitary->ToBoolean();
    auto dateTimeOptionsValue = paramObject->GetProperty("dateTimeOptions");
    if (dateTimeOptionsValue->IsObject()) {
        auto dateTimeOptionsObj = JSRef<JSObject>::Cast(dateTimeOptionsValue);
        JSDatePickerDialog::ParseDateTimeOptions(dateTimeOptionsObj, settingData.dateTimeOptions);
    }
    JSDatePicker::ParseTextProperties(paramObject, settingData.properties);
}

void JSDatePickerDialog::UpdatePickerDialogTimeInfo(const JSRef<JSObject>& paramObject, PickerDialogInfo& pickerDialog)
{
    auto theme = GetTheme<PickerTheme>();
    CHECK_NULL_VOID(theme);

    auto startDate = paramObject->GetProperty("start");
    if (startDate->IsObject()) {
        pickerDialog.isStartDate = true;
    }
    auto endDate = paramObject->GetProperty("end");
    if (endDate->IsObject()) {
        pickerDialog.isEndDate = true;
    }
    auto selectedDate = paramObject->GetProperty("selected");
    if (selectedDate->IsObject()) {
        pickerDialog.isSelectedDate = true;
    }
    auto parseStartDate = ParseDate(startDate);
    auto parseEndDate = ParseDate(endDate);
    if (parseStartDate.GetYear() <= 0) {
        parseStartDate = theme->GetDefaultStartDate();
    }
    if (parseEndDate.GetYear() <= 0) {
        parseEndDate = theme->GetDefaultEndDate();
    }
    auto startDays = parseStartDate.ToDays();
    auto endDays = parseEndDate.ToDays();
    if (startDays > endDays) {
        parseStartDate = theme->GetDefaultStartDate();
        parseEndDate = theme->GetDefaultEndDate();
    }
    pickerDialog.parseStartDate = parseStartDate;
    pickerDialog.parseEndDate = parseEndDate;
    pickerDialog.parseSelectedDate = ParseDate(selectedDate);
    pickerDialog.pickerTime = ParseTime(selectedDate);
}

void JSDatePickerDialog::UpdatePickerDialogPositionInfo(
    const JSRef<JSObject>& paramObject, PickerDialogInfo& pickerDialog)
{
    // Parse alignment
    auto alignmentValue = paramObject->GetProperty("alignment");
    if (alignmentValue->IsNumber()) {
        auto alignment = alignmentValue->ToNumber<int32_t>();
        if (alignment >= 0 && alignment <= static_cast<int32_t>(DIALOG_ALIGNMENT.size())) {
            pickerDialog.alignment = DIALOG_ALIGNMENT[alignment];
        }
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
            if (alignment == static_cast<int32_t>(DialogAlignment::TOP) ||
                alignment == static_cast<int32_t>(DialogAlignment::TOP_START) ||
                alignment == static_cast<int32_t>(DialogAlignment::TOP_END)) {
                pickerDialog.offset = DATEPICKER_OFFSET_DEFAULT_TOP;
            }
        }
    }

    // Parse offset
    auto offsetValue = paramObject->GetProperty("offset");
    if (offsetValue->IsObject()) {
        auto offsetObj = JSRef<JSObject>::Cast(offsetValue);
        CalcDimension dx;
        auto dxValue = offsetObj->GetProperty("dx");
        ParseJsDimensionVp(dxValue, dx);
        CalcDimension dy;
        auto dyValue = offsetObj->GetProperty("dy");
        ParseJsDimensionVp(dyValue, dy);
        pickerDialog.offset = DimensionOffset(dx, dy);
    }
}

void JSDatePickerDialog::UpdatePickerDialogInfo(const JSRef<JSObject>& paramObject, PickerDialogInfo& pickerDialog)
{
    UpdatePickerDialogTimeInfo(paramObject, pickerDialog);
    UpdatePickerDialogPositionInfo(paramObject, pickerDialog);
    // Parse maskRect.
    auto maskRectValue = paramObject->GetProperty("maskRect");
    DimensionRect maskRect;
    if (JSViewAbstract::ParseJsDimensionRect(maskRectValue, maskRect)) {
        pickerDialog.maskRect = maskRect;
    }

    auto backgroundColorValue = paramObject->GetProperty("backgroundColor");
    Color backgroundColor;
    if (JSViewAbstract::ParseJsColor(backgroundColorValue, backgroundColor)) {
        pickerDialog.backgroundColor = backgroundColor;
    }

    auto backgroundBlurStyle = paramObject->GetProperty("backgroundBlurStyle");
    BlurStyleOption styleOption;
    if (backgroundBlurStyle->IsNumber()) {
        auto blurStyle = backgroundBlurStyle->ToNumber<int32_t>();
        if (blurStyle >= static_cast<int>(BlurStyle::NO_MATERIAL) &&
            blurStyle <= static_cast<int>(BlurStyle::COMPONENT_ULTRA_THICK)) {
            pickerDialog.backgroundBlurStyle = blurStyle;
        }
    }

    auto shadowValue = paramObject->GetProperty("shadow");
    Shadow shadow;
    if ((shadowValue->IsObject() || shadowValue->IsNumber()) && JSViewAbstract::ParseShadowProps(shadowValue, shadow)) {
        pickerDialog.shadow = shadow;
    }
}

void JSDatePickerDialog::Show(const JSCallbackInfo& info)
{
    auto scopedDelegate = EngineHelper::GetCurrentDelegateSafely();
    CHECK_NULL_VOID(scopedDelegate);
    if (!info[0]->IsObject()) {
        return;
    }

    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    DatePickerType pickerType = DatePickerType::DATE;
    auto type = paramObject->GetProperty("type");
    if (type->IsNumber()) {
        pickerType = static_cast<DatePickerType>(type->ToNumber<int32_t>());
    }
    std::function<void()> cancelEvent;
    std::function<void(const std::string&)> acceptEvent;
    std::function<void(const std::string&)> changeEvent;
    std::function<void(const std::string&)> dateChangeEvent;
    std::function<void(const std::string&)> dateAcceptEvent;
    WeakPtr<NG::FrameNode> frameNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    changeEvent = GetChangeEvent(paramObject, info, pickerType, frameNode);
    acceptEvent = GetAcceptEvent(paramObject, info, frameNode);
    cancelEvent = GetCancelEvent(paramObject, info, frameNode);
    dateChangeEvent = GetDateChangeEvent(paramObject, info, pickerType, frameNode);
    dateAcceptEvent = GetDateAcceptEvent(paramObject, info, pickerType, frameNode);
    NG::DatePickerSettingData settingData;
    UpdateDatePickerSettingData(paramObject, settingData);
    PickerDialogInfo pickerDialog;
    UpdatePickerDialogInfo(paramObject, pickerDialog);

    auto buttonInfos = ParseButtonStyles(paramObject);
    PickerDialogEvent pickerDialogEvent { nullptr, nullptr, nullptr, nullptr };
    DatePickerDialogAppearEvent(info, pickerDialogEvent);
    DatePickerDialogDisappearEvent(info, pickerDialogEvent);
    DatePickerDialogModel::GetInstance()->SetDatePickerDialogShow(pickerDialog, settingData, std::move(cancelEvent),
        std::move(acceptEvent), std::move(changeEvent), std::move(dateAcceptEvent), std::move(dateChangeEvent),
        pickerType, pickerDialogEvent, buttonInfos);
}

void JSDatePickerDialog::DatePickerDialogShow(const JSRef<JSObject>& paramObj,
    const std::map<std::string, NG::DialogEvent>& dialogEvent,
    const std::map<std::string, NG::DialogGestureEvent>& dialogCancelEvent)
{
    auto container = Container::CurrentSafely();
    if (!container) {
        return;
    }
    auto pipelineContext = AccessibilityManager::DynamicCast<NG::PipelineContext>(container->GetPipelineContext());
    if (!pipelineContext) {
        return;
    }

    auto executor = pipelineContext->GetTaskExecutor();
    if (!executor) {
        return;
    }

    NG::DatePickerSettingData settingData;
    auto startDate = paramObj->GetProperty("start");
    auto endDate = paramObj->GetProperty("end");
    auto selectedDate = paramObj->GetProperty("selected");
    auto lunar = paramObj->GetProperty("lunar");
    auto sTime = paramObj->GetProperty("showTime");
    auto useMilitary = paramObj->GetProperty("useMilitaryTime");
    settingData.isLunar = lunar->ToBoolean();
    settingData.showTime = sTime->ToBoolean();
    settingData.useMilitary = useMilitary->ToBoolean();
    auto parseStartDate = ParseDate(startDate);
    auto parseEndDate = ParseDate(endDate);
    auto parseSelectedDate = ParseDate(selectedDate);

    auto theme = GetTheme<DialogTheme>();
    CHECK_NULL_VOID(theme);

    DialogProperties properties;
    properties.alignment = theme->GetAlignment();
    if (properties.alignment == DialogAlignment::BOTTOM &&
        Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_ELEVEN)) {
        properties.offset = DimensionOffset(Offset(0, -theme->GetMarginBottom().ConvertToPx()));
    }
    properties.customStyle = false;
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        properties.offset = DimensionOffset(Offset(0, -theme->GetMarginBottom().ConvertToPx()));
    }

    std::map<std::string, PickerDate> datePickerProperty;
    std::map<std::string, PickerTime> timePickerProperty;
    if (startDate->IsObject()) {
        settingData.datePickerProperty["start"] = parseStartDate;
    }
    if (endDate->IsObject()) {
        settingData.datePickerProperty["end"] = parseEndDate;
    }
    if (selectedDate->IsObject()) {
        settingData.datePickerProperty["selected"] = parseSelectedDate;
        settingData.timePickerProperty["selected"] = ParseTime(selectedDate);
    }

    JSDatePicker::ParseTextProperties(paramObj, settingData.properties);
    auto context = AccessibilityManager::DynamicCast<NG::PipelineContext>(pipelineContext);
    auto overlayManager = context ? context->GetOverlayManager() : nullptr;
    executor->PostTask(
        [properties, settingData, dialogEvent, dialogCancelEvent, weak = WeakPtr<NG::OverlayManager>(overlayManager)] {
            auto overlayManager = weak.Upgrade();
            CHECK_NULL_VOID(overlayManager);
            overlayManager->ShowDateDialog(properties, settingData, dialogEvent, dialogCancelEvent);
        },
        TaskExecutor::TaskType::UI, "ArkUIDialogShowDatePicker");
}

void JSDatePickerDialog::CreateDatePicker(RefPtr<Component>& component, const JSRef<JSObject>& paramObj)
{
    auto datePicker = AceType::MakeRefPtr<PickerDateComponent>();
    auto startDate = paramObj->GetProperty("start");
    auto endDate = paramObj->GetProperty("end");
    auto selectedDate = paramObj->GetProperty("selected");
    auto lunar = paramObj->GetProperty("lunar");
    bool isLunar = lunar->ToBoolean();
    auto parseStartDate = ParseDate(startDate);
    auto parseEndDate = ParseDate(endDate);
    auto parseSelectedDate = ParseDate(selectedDate);
    auto startDays = parseStartDate.ToDays();
    auto endDays = parseEndDate.ToDays();
    if (startDays > endDays) {
        parseStartDate.SetYear(0);
        parseEndDate.SetYear(0);
    }
    if (startDate->IsObject()) {
        datePicker->SetStartDate(parseStartDate);
    }
    if (endDate->IsObject()) {
        datePicker->SetEndDate(parseEndDate);
    }
    if (selectedDate->IsObject()) {
        datePicker->SetSelectedDate(parseSelectedDate);
    }
    datePicker->SetIsDialog(true);
    datePicker->SetIsCreateDialogComponent(true);
    datePicker->SetShowLunar(isLunar);

    component = datePicker;
}

void JSDatePickerDialog::CreateTimePicker(RefPtr<Component>& component, const JSRef<JSObject>& paramObj)
{
    auto timePicker = AceType::MakeRefPtr<PickerTimeComponent>();
    auto selectedTime = paramObj->GetProperty("selected");
    auto useMilitaryTime = paramObj->GetProperty("useMilitaryTime");
    bool isUseMilitaryTime = useMilitaryTime->ToBoolean();
    if (selectedTime->IsObject()) {
        timePicker->SetSelectedTime(ParseTime(selectedTime));
    }
    timePicker->SetIsDialog(true);
    timePicker->SetIsCreateDialogComponent(true);
    timePicker->SetHour24(isUseMilitaryTime);
    component = timePicker;
}

PickerDate JSDatePickerDialog::ParseDate(const JSRef<JSVal>& dateVal)
{
    auto pickerDate = PickerDate();
    if (!dateVal->IsObject()) {
        return pickerDate;
    }
    auto dateObj = JSRef<JSObject>::Cast(dateVal);
    auto yearFuncJsVal = dateObj->GetProperty("getFullYear");
    auto monthFuncJsVal = dateObj->GetProperty("getMonth");
    auto dateFuncJsVal = dateObj->GetProperty("getDate");
    if (!(yearFuncJsVal->IsFunction() && monthFuncJsVal->IsFunction() && dateFuncJsVal->IsFunction())) {
        return pickerDate;
    }
    auto yearFunc = JSRef<JSFunc>::Cast(yearFuncJsVal);
    auto monthFunc = JSRef<JSFunc>::Cast(monthFuncJsVal);
    auto dateFunc = JSRef<JSFunc>::Cast(dateFuncJsVal);
    JSRef<JSVal> year = yearFunc->Call(dateObj);
    JSRef<JSVal> month = monthFunc->Call(dateObj);
    JSRef<JSVal> date = dateFunc->Call(dateObj);

    if (year->IsNumber() && month->IsNumber() && date->IsNumber()) {
        pickerDate.SetYear(year->ToNumber<int32_t>());
        pickerDate.SetMonth(month->ToNumber<int32_t>() + 1); // 0-11 means 1 to 12 months
        pickerDate.SetDay(date->ToNumber<int32_t>());
    }
    return pickerDate;
}

PickerTime JSDatePickerDialog::ParseTime(const JSRef<JSVal>& timeVal)
{
    auto pickerTime = PickerTime();
    if (!timeVal->IsObject()) {
        return pickerTime;
    }
    auto timeObj = JSRef<JSObject>::Cast(timeVal);
    auto hourFuncJsVal = timeObj->GetProperty("getHours");
    auto minuteFuncJsVal = timeObj->GetProperty("getMinutes");
    auto secondFuncJsVal = timeObj->GetProperty("getSeconds");
    if (!(hourFuncJsVal->IsFunction() && minuteFuncJsVal->IsFunction() && secondFuncJsVal->IsFunction())) {
        return pickerTime;
    }
    auto hourFunc = JSRef<JSFunc>::Cast(hourFuncJsVal);
    auto minuteFunc = JSRef<JSFunc>::Cast(minuteFuncJsVal);
    auto secondFunc = JSRef<JSFunc>::Cast(secondFuncJsVal);
    JSRef<JSVal> hour = hourFunc->Call(timeObj);
    JSRef<JSVal> minute = minuteFunc->Call(timeObj);
    JSRef<JSVal> second = secondFunc->Call(timeObj);

    if (hour->IsNumber() && minute->IsNumber() && second->IsNumber()) {
        pickerTime.SetHour(hour->ToNumber<int32_t>());
        pickerTime.SetMinute(minute->ToNumber<int32_t>());
        pickerTime.SetSecond(second->ToNumber<int32_t>());
    }
    return pickerTime;
}

void JSDatePickerDialog::ParseDateTimeOptions(const JSRef<JSObject>& paramObj, DateTimeType& dateTimeOptions)
{
    dateTimeOptions.hourType = ZeroPrefixType::AUTO;
    dateTimeOptions.minuteType = ZeroPrefixType::AUTO;
    dateTimeOptions.secondType = ZeroPrefixType::AUTO;

    auto hourValue = paramObj->GetProperty(TIMEPICKER_OPTIONS_HOUR);
    if (hourValue->IsString()) {
        std::string hour = hourValue->ToString();
        if (hour == TIMEPICKER_OPTIONS_TWO_DIGIT_VAL) {
            dateTimeOptions.hourType = ZeroPrefixType::SHOW;
        } else if (hour == TIMEPICKER_OPTIONS_NUMERIC_VAL) {
            dateTimeOptions.hourType = ZeroPrefixType::HIDE;
        }
    }
    auto minuteValue = paramObj->GetProperty(TIMEPICKER_OPTIONS_MINUTE);
    if (minuteValue->IsString()) {
        dateTimeOptions.minuteType = ZeroPrefixType::SHOW;
        std::string minute = minuteValue->ToString();
        if (minute == TIMEPICKER_OPTIONS_NUMERIC_VAL) {
            dateTimeOptions.minuteType = ZeroPrefixType::HIDE;
        }
    }
}

void JSTimePicker::JSBind(BindingTarget globalObj)
{
    JSClass<JSTimePicker>::Declare("TimePicker");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSTimePicker>::StaticMethod("create", &JSTimePicker::Create, opt);
    JSClass<JSTimePicker>::StaticMethod("onChange", &JSTimePicker::OnChange);
    JSClass<JSTimePicker>::StaticMethod("backgroundColor", &JSTimePicker::PickerBackgroundColor);
    JSClass<JSTimePicker>::StaticMethod("loop", &JSTimePicker::Loop);
    JSClass<JSTimePicker>::StaticMethod("useMilitaryTime", &JSTimePicker::UseMilitaryTime);
    JSClass<JSTimePicker>::StaticMethod("onClick", &JSInteractableView::JsOnClick);
    JSClass<JSTimePicker>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSTimePicker>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSTimePicker>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSTimePicker>::StaticMethod("onAttach", &JSInteractableView::JsOnAttach);
    JSClass<JSTimePicker>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSTimePicker>::StaticMethod("onDetach", &JSInteractableView::JsOnDetach);
    JSClass<JSTimePicker>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSTimePicker>::StaticMethod("disappearTextStyle", &JSTimePicker::SetDisappearTextStyle);
    JSClass<JSTimePicker>::StaticMethod("textStyle", &JSTimePicker::SetTextStyle);
    JSClass<JSTimePicker>::StaticMethod("selectedTextStyle", &JSTimePicker::SetSelectedTextStyle);
    JSClass<JSTimePicker>::StaticMethod("dateTimeOptions", &JSTimePicker::DateTimeOptions);
    JSClass<JSTimePicker>::InheritAndBind<JSViewAbstract>(globalObj);
}

void JSTimePicker::Create(const JSCallbackInfo& info)
{
    JSRef<JSObject> paramObject = JSRef<JSObject>::New();
    if (info.Length() >= 1 && info[0]->IsObject()) {
        paramObject = JSRef<JSObject>::Cast(info[0]);
    }
    CreateTimePicker(info, paramObject);
}

void JSTimePicker::Loop(const JSCallbackInfo& info)
{
    bool isLoop = true;
    if (info[0]->IsBoolean()) {
        isLoop = info[0]->ToBoolean();
    }
    TimePickerModel::GetInstance()->SetWheelModeEnabled(isLoop);
}

void JSTimePicker::UseMilitaryTime(bool isUseMilitaryTime)
{
    TimePickerModel::GetInstance()->SetHour24(isUseMilitaryTime);
}

void JSTimePicker::DateTimeOptions(const JSCallbackInfo& info)
{
    JSRef<JSObject> paramObject;
    ZeroPrefixType hourType = ZeroPrefixType::AUTO;
    ZeroPrefixType minuteType = ZeroPrefixType::AUTO;
    ZeroPrefixType secondType = ZeroPrefixType::AUTO;
    if (info.Length() >= 1 && info[0]->IsObject()) {
        paramObject = JSRef<JSObject>::Cast(info[0]);
        auto hourValue = paramObject->GetProperty(TIMEPICKER_OPTIONS_HOUR);
        if (hourValue->IsString()) {
            std::string hour = hourValue->ToString();
            if (hour == TIMEPICKER_OPTIONS_TWO_DIGIT_VAL) {
                hourType = ZeroPrefixType::SHOW;
            } else if (hour == TIMEPICKER_OPTIONS_NUMERIC_VAL) {
                hourType = ZeroPrefixType::HIDE;
            }
        }
        auto minuteValue = paramObject->GetProperty(TIMEPICKER_OPTIONS_MINUTE);
        if (minuteValue->IsString()) {
            minuteType = ZeroPrefixType::SHOW;
            std::string minute = minuteValue->ToString();
            if (minute == TIMEPICKER_OPTIONS_NUMERIC_VAL) {
                minuteType = ZeroPrefixType::HIDE;
            }
        }
        auto secondValue = paramObject->GetProperty(TIMEPICKER_OPTIONS_SECOND);
        if (secondValue->IsString()) {
            secondType = ZeroPrefixType::SHOW;
            std::string second = secondValue->ToString();
            if (second == TIMEPICKER_OPTIONS_NUMERIC_VAL) {
                secondType = ZeroPrefixType::HIDE;
            }
        }
    }
    TimePickerModel::GetInstance()->SetDateTimeOptions(hourType, minuteType, secondType);
}

void JSTimePicker::PickerBackgroundColor(const JSCallbackInfo& info)
{
    JSViewAbstract::JsBackgroundColor(info);

    if (info.Length() < 1) {
        return;
    }
    Color backgroundColor;
    if (!ParseJsColor(info[0], backgroundColor)) {
        return;
    }
    TimePickerModel::GetInstance()->SetBackgroundColor(backgroundColor);
}

void JSTimePicker::SetDisappearTextStyle(const JSCallbackInfo& info)
{
    auto theme = GetTheme<PickerTheme>();
    CHECK_NULL_VOID(theme);
    NG::PickerTextStyle textStyle;
    JSTimePickerTheme::ObtainTextStyle(textStyle);
    if (info[0]->IsObject()) {
        JSDatePicker::ParseTextStyle(info[0], textStyle, "disappearTextStyleTime");
    }
    TimePickerModel::GetInstance()->SetDisappearTextStyle(theme, textStyle);
}

void JSTimePicker::SetTextStyle(const JSCallbackInfo& info)
{
    auto theme = GetTheme<PickerTheme>();
    CHECK_NULL_VOID(theme);
    NG::PickerTextStyle textStyle;
    JSTimePickerTheme::ObtainTextStyle(textStyle);
    if (info[0]->IsObject()) {
        JSDatePicker::ParseTextStyle(info[0], textStyle, "textStyleTime");
    }
    TimePickerModel::GetInstance()->SetNormalTextStyle(theme, textStyle);
}

void JSTimePicker::SetSelectedTextStyle(const JSCallbackInfo& info)
{
    auto theme = GetTheme<PickerTheme>();
    CHECK_NULL_VOID(theme);
    NG::PickerTextStyle textStyle;
    JSTimePickerTheme::ObtainSelectedTextStyle(textStyle);
    if (info[0]->IsObject()) {
        JSDatePicker::ParseTextStyle(info[0], textStyle, "selectedTextStyleTime");
    }
    TimePickerModel::GetInstance()->SetSelectedTextStyle(theme, textStyle);
}

void JSTimePicker::CreateTimePicker(const JSCallbackInfo& info, const JSRef<JSObject>& paramObj)
{
    auto selectedTime = paramObj->GetProperty("selected");
    auto theme = GetTheme<PickerTheme>();
    CHECK_NULL_VOID(theme);
    auto formatValue = paramObj->GetProperty("format");
    bool showSecond = false;
    if (formatValue->IsNumber()) {
        auto displayedFormat = static_cast<TimePickerFormat>(formatValue->ToNumber<int32_t>());
        if (displayedFormat == TimePickerFormat::HOUR_MINUTE_SECOND) {
            showSecond = true;
        }
    }
    TimePickerModel::GetInstance()->CreateTimePicker(theme, showSecond);
    if (selectedTime->IsObject()) {
        JSRef<JSObject> selectedTimeObj = JSRef<JSObject>::Cast(selectedTime);
        JSRef<JSVal> changeEventVal = selectedTimeObj->GetProperty("changeEvent");
        if (!changeEventVal->IsUndefined() && changeEventVal->IsFunction()) {
            ParseSelectedDateTimeObject(info, selectedTimeObj, false);
            auto parseSelectedTime = ParseTime(selectedTimeObj->GetProperty("value"));
            TimePickerModel::GetInstance()->SetSelectedTime(parseSelectedTime);
        } else {
            TimePickerModel::GetInstance()->SetSelectedTime(ParseTime(selectedTime));
        }
    }
    if (!JSTimePickerTheme::ApplyTheme()) {
        SetDefaultAttributes();
    }
}

void JSTimePicker::SetDefaultAttributes()
{
    auto theme = GetTheme<PickerTheme>();
    CHECK_NULL_VOID(theme);
    NG::PickerTextStyle textStyle;
    auto selectedStyle = theme->GetOptionStyle(true, false);
    textStyle.textColor = selectedStyle.GetTextColor();
    textStyle.fontSize = selectedStyle.GetFontSize();
    textStyle.fontWeight = selectedStyle.GetFontWeight();
    TimePickerModel::GetInstance()->SetSelectedTextStyle(theme, textStyle);

    auto disappearStyle = theme->GetDisappearOptionStyle();
    textStyle.textColor = disappearStyle.GetTextColor();
    textStyle.fontSize = disappearStyle.GetFontSize();
    textStyle.fontWeight = disappearStyle.GetFontWeight();
    TimePickerModel::GetInstance()->SetDisappearTextStyle(theme, textStyle);

    auto normalStyle = theme->GetOptionStyle(false, false);
    textStyle.textColor = normalStyle.GetTextColor();
    textStyle.fontSize = normalStyle.GetFontSize();
    textStyle.fontWeight = normalStyle.GetFontWeight();
    TimePickerModel::GetInstance()->SetNormalTextStyle(theme, textStyle);
}

PickerTime JSTimePicker::ParseTime(const JSRef<JSVal>& timeVal)
{
    auto pickerTime = PickerTime::Current();
    if (!timeVal->IsObject()) {
        return pickerTime;
    }
    auto timeObj = JSRef<JSObject>::Cast(timeVal);
    auto yearFuncJsVal = timeObj->GetProperty("getFullYear");
    if (yearFuncJsVal->IsFunction()) {
        auto yearFunc = JSRef<JSFunc>::Cast(yearFuncJsVal);
        JSRef<JSVal> year = yearFunc->Call(timeObj);
        if (year->IsNumber() && LessOrEqual(year->ToNumber<int32_t>(), 0)) {
            return pickerTime;
        }
    }

    auto hourFuncJsVal = timeObj->GetProperty("getHours");
    auto minuteFuncJsVal = timeObj->GetProperty("getMinutes");
    auto secondFuncJsVal = timeObj->GetProperty("getSeconds");
    if (!(hourFuncJsVal->IsFunction() && minuteFuncJsVal->IsFunction() && secondFuncJsVal->IsFunction())) {
        return pickerTime;
    }
    auto hourFunc = JSRef<JSFunc>::Cast(hourFuncJsVal);
    auto minuteFunc = JSRef<JSFunc>::Cast(minuteFuncJsVal);
    auto secondFunc = JSRef<JSFunc>::Cast(secondFuncJsVal);
    JSRef<JSVal> hour = hourFunc->Call(timeObj);
    JSRef<JSVal> minute = minuteFunc->Call(timeObj);
    JSRef<JSVal> second = secondFunc->Call(timeObj);

    if (hour->IsNumber() && minute->IsNumber() && second->IsNumber()) {
        pickerTime.SetHour(hour->ToNumber<int32_t>());
        pickerTime.SetMinute(minute->ToNumber<int32_t>());
        pickerTime.SetSecond(second->ToNumber<int32_t>());
    }
    return pickerTime;
}

void JSTimePickerDialog::JSBind(BindingTarget globalObj)
{
    JSClass<JSTimePickerDialog>::Declare("TimePickerDialog");
    JSClass<JSTimePickerDialog>::StaticMethod("show", &JSTimePickerDialog::Show);

    JSClass<JSTimePickerDialog>::Bind<>(globalObj);
}

void TimePickerDialogAppearEvent(const JSCallbackInfo& info, TimePickerDialogEvent& timePickerDialogEvent)
{
    std::function<void()> didAppearEvent;
    std::function<void()> willAppearEvent;
    if (info.Length() == 0 || !info[0]->IsObject()) {
        return;
    }
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    WeakPtr<NG::FrameNode> targetNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto onDidAppear = paramObject->GetProperty("onDidAppear");
    if (!onDidAppear->IsUndefined() && onDidAppear->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onDidAppear));
        didAppearEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = targetNode]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("DatePickerDialog.onDidAppear");
            PipelineContext::SetCallBackNode(node);
            func->Execute();
        };
    }
    auto onWillAppear = paramObject->GetProperty("onWillAppear");
    if (!onWillAppear->IsUndefined() && onWillAppear->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onWillAppear));
        willAppearEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = targetNode]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("DatePickerDialog.onWillAppear");
            PipelineContext::SetCallBackNode(node);
            func->Execute();
        };
    }
    timePickerDialogEvent.onDidAppear = std::move(didAppearEvent);
    timePickerDialogEvent.onWillAppear = std::move(willAppearEvent);
}

void TimePickerDialogDisappearEvent(const JSCallbackInfo& info, TimePickerDialogEvent& timePickerDialogEvent)
{
    std::function<void()> didDisappearEvent;
    std::function<void()> willDisappearEvent;
    if (info.Length() == 0 || !info[0]->IsObject()) {
        return;
    }
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    WeakPtr<NG::FrameNode> targetNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto onDidDisappear = paramObject->GetProperty("onDidDisappear");
    if (!onDidDisappear->IsUndefined() && onDidDisappear->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onDidDisappear));
        didDisappearEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = targetNode]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("DatePickerDialog.onDidDisappear");
            PipelineContext::SetCallBackNode(node);
            func->Execute();
        };
    }
    auto onWillDisappear = paramObject->GetProperty("onWillDisappear");
    if (!onWillDisappear->IsUndefined() && onWillDisappear->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onWillDisappear));
        willDisappearEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = targetNode]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("DatePickerDialog.onWillDisappear");
            PipelineContext::SetCallBackNode(node);
            func->Execute();
        };
    }
    timePickerDialogEvent.onDidDisappear = std::move(didDisappearEvent);
    timePickerDialogEvent.onWillDisappear = std::move(willDisappearEvent);
}

void JSTimePickerDialog::Show(const JSCallbackInfo& info)
{
    auto scopedDelegate = EngineHelper::GetCurrentDelegateSafely();
    CHECK_NULL_VOID(scopedDelegate);
    if (!info[0]->IsObject()) {
        return;
    }
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    std::function<void()> cancelEvent;
    std::function<void(const std::string&)> acceptEvent;
    std::function<void(const std::string&)> changeEvent;
    WeakPtr<NG::FrameNode> targetNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto onChange = paramObject->GetProperty("onChange");
    if (!onChange->IsUndefined() && onChange->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onChange));
        changeEvent = [execCtx = info.GetExecutionContext(), type = DatePickerType::TIME, func = std::move(jsFunc),
                          node = targetNode](const std::string& info) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            std::vector<std::string> keys;
            keys = { "hour", "minute" };
            ACE_SCORING_EVENT("DatePickerDialog.onChange");
            PipelineContext::SetCallBackNode(node);
            func->Execute(keys, info);
        };
    }
    auto onAccept = paramObject->GetProperty("onAccept");
    if (!onAccept->IsUndefined() && onAccept->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onAccept));
        acceptEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = targetNode](
                          const std::string& info) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            std::vector<std::string> keys = { "year", "month", "day", "hour", "minute", "second" };
            ACE_SCORING_EVENT("DatePickerDialog.onAccept");
            PipelineContext::SetCallBackNode(node);
            func->Execute(keys, info);
        };
    }
    auto onCancel = paramObject->GetProperty("onCancel");
    if (!onCancel->IsUndefined() && onCancel->IsFunction()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onCancel));
        cancelEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), node = targetNode]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("DatePickerDialog.onCancel");
            PipelineContext::SetCallBackNode(node);
            func->Execute();
        };
    }
    auto selectedTime = paramObject->GetProperty("selected");
    auto useMilitaryTime = paramObject->GetProperty("useMilitaryTime");
    NG::TimePickerSettingData settingData;
    PickerDialogInfo pickerDialog;
    settingData.isUseMilitaryTime = useMilitaryTime->ToBoolean();
    pickerDialog.isUseMilitaryTime = useMilitaryTime->ToBoolean();
    if (selectedTime->IsObject()) {
        PickerDate dialogTitleDate = ParseDate(selectedTime);
        if (dialogTitleDate.GetYear() != 0) {
            settingData.dialogTitleDate = dialogTitleDate;
            pickerDialog.isSelectedTime = true;
            pickerDialog.pickerTime = ParseTime(selectedTime);
        }
    }
    JSDatePicker::ParseTextProperties(paramObject, settingData.properties);
    auto dateTimeOptionsValue = paramObject->GetProperty("dateTimeOptions");
    if (dateTimeOptionsValue->IsObject()) {
        auto dateTimeOptionsObj = JSRef<JSObject>::Cast(dateTimeOptionsValue);
        JSDatePickerDialog::ParseDateTimeOptions(dateTimeOptionsObj, settingData.dateTimeOptions);
    }

    // Parse alignment
    auto alignmentValue = paramObject->GetProperty("alignment");
    if (alignmentValue->IsNumber()) {
        auto alignment = alignmentValue->ToNumber<int32_t>();
        if (alignment >= 0 && alignment <= static_cast<int32_t>(DIALOG_ALIGNMENT.size())) {
            pickerDialog.alignment = DIALOG_ALIGNMENT[alignment];
        }
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
            if (alignment == static_cast<int32_t>(DialogAlignment::TOP) ||
                alignment == static_cast<int32_t>(DialogAlignment::TOP_START) ||
                alignment == static_cast<int32_t>(DialogAlignment::TOP_END)) {
                pickerDialog.offset = DATEPICKER_OFFSET_DEFAULT_TOP;
            }
        }
    }

    // Parse offset
    auto offsetValue = paramObject->GetProperty("offset");
    if (offsetValue->IsObject()) {
        auto offsetObj = JSRef<JSObject>::Cast(offsetValue);
        CalcDimension dx;
        auto dxValue = offsetObj->GetProperty("dx");
        JSAlertDialog::ParseJsDimensionVp(dxValue, dx);
        CalcDimension dy;
        auto dyValue = offsetObj->GetProperty("dy");
        JSAlertDialog::ParseJsDimensionVp(dyValue, dy);
        pickerDialog.offset = DimensionOffset(dx, dy);
    }

    // Parse maskRect.
    auto maskRectValue = paramObject->GetProperty("maskRect");
    DimensionRect maskRect;
    if (JSViewAbstract::ParseJsDimensionRect(maskRectValue, maskRect)) {
        pickerDialog.maskRect = maskRect;
    }

    auto backgroundColorValue = paramObject->GetProperty("backgroundColor");
    Color backgroundColor;
    if (JSViewAbstract::ParseJsColor(backgroundColorValue, backgroundColor)) {
        pickerDialog.backgroundColor = backgroundColor;
    }

    auto backgroundBlurStyle = paramObject->GetProperty("backgroundBlurStyle");
    BlurStyleOption styleOption;
    if (backgroundBlurStyle->IsNumber()) {
        auto blurStyle = backgroundBlurStyle->ToNumber<int32_t>();
        if (blurStyle >= static_cast<int>(BlurStyle::NO_MATERIAL) &&
            blurStyle <= static_cast<int>(BlurStyle::COMPONENT_ULTRA_THICK)) {
            pickerDialog.backgroundBlurStyle = blurStyle;
        }
    }

    auto buttonInfos = ParseButtonStyles(paramObject);

    auto shadowValue = paramObject->GetProperty("shadow");
    Shadow shadow;
    if ((shadowValue->IsObject() || shadowValue->IsNumber()) && JSViewAbstract::ParseShadowProps(shadowValue, shadow)) {
        pickerDialog.shadow = shadow;
    }
    TimePickerDialogEvent timePickerDialogEvent { nullptr, nullptr, nullptr, nullptr };
    TimePickerDialogAppearEvent(info, timePickerDialogEvent);
    TimePickerDialogDisappearEvent(info, timePickerDialogEvent);
    TimePickerDialogModel::GetInstance()->SetTimePickerDialogShow(pickerDialog, settingData, std::move(cancelEvent),
        std::move(acceptEvent), std::move(changeEvent), timePickerDialogEvent, buttonInfos);
}

void JSTimePickerDialog::TimePickerDialogShow(const JSRef<JSObject>& paramObj,
    const std::map<std::string, NG::DialogEvent>& dialogEvent,
    const std::map<std::string, NG::DialogGestureEvent>& dialogCancelEvent)
{
    auto container = Container::CurrentSafely();
    CHECK_NULL_VOID(container);

    auto pipelineContext = AccessibilityManager::DynamicCast<NG::PipelineContext>(container->GetPipelineContext());
    CHECK_NULL_VOID(pipelineContext);

    auto executor = pipelineContext->GetTaskExecutor();
    CHECK_NULL_VOID(executor);

    auto theme = JSAlertDialog::GetTheme<DialogTheme>();
    CHECK_NULL_VOID(theme);

    auto selectedTime = paramObj->GetProperty("selected");
    auto useMilitaryTime = paramObj->GetProperty("useMilitaryTime");
    NG::TimePickerSettingData settingData;
    settingData.isUseMilitaryTime = useMilitaryTime->ToBoolean();

    DialogProperties properties;
    properties.alignment = theme->GetAlignment();
    if (properties.alignment == DialogAlignment::BOTTOM &&
        Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        properties.offset = DimensionOffset(Offset(0, -theme->GetMarginBottom().ConvertToPx()));
    }

    properties.customStyle = false;
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        properties.offset = DimensionOffset(Offset(0, -theme->GetMarginBottom().ConvertToPx()));
    }

    std::map<std::string, PickerTime> timePickerProperty;
    if (selectedTime->IsObject()) {
        settingData.dialogTitleDate = ParseDate(selectedTime);
        timePickerProperty["selected"] = ParseTime(selectedTime);
    }
    JSDatePicker::ParseTextProperties(paramObj, settingData.properties);

    auto context = AccessibilityManager::DynamicCast<NG::PipelineContext>(pipelineContext);
    auto overlayManager = context ? context->GetOverlayManager() : nullptr;
    executor->PostTask(
        [properties, settingData, timePickerProperty, dialogEvent, dialogCancelEvent,
            weak = WeakPtr<NG::OverlayManager>(overlayManager)] {
            auto overlayManager = weak.Upgrade();
            CHECK_NULL_VOID(overlayManager);
            overlayManager->ShowTimeDialog(properties, settingData, timePickerProperty, dialogEvent, dialogCancelEvent);
        },
        TaskExecutor::TaskType::UI, "ArkUIDialogShowTimePicker");
}

void JSTimePickerDialog::CreateTimePicker(RefPtr<Component>& component, const JSRef<JSObject>& paramObj)
{
    auto timePicker = AceType::MakeRefPtr<PickerTimeComponent>();
    auto selectedTime = paramObj->GetProperty("selected");
    auto useMilitaryTime = paramObj->GetProperty("useMilitaryTime");
    bool isUseMilitaryTime = useMilitaryTime->ToBoolean();
    if (selectedTime->IsObject()) {
        timePicker->SetSelectedTime(ParseTime(selectedTime));
    }
    timePicker->SetIsDialog(true);
    timePicker->SetIsCreateDialogComponent(true);
    timePicker->SetHour24(isUseMilitaryTime);
    component = timePicker;
}

PickerTime JSTimePickerDialog::ParseTime(const JSRef<JSVal>& timeVal)
{
    auto pickerTime = PickerTime();
    if (!timeVal->IsObject()) {
        return pickerTime;
    }
    auto timeObj = JSRef<JSObject>::Cast(timeVal);
    auto hourFuncJsVal = timeObj->GetProperty("getHours");
    auto minuteFuncJsVal = timeObj->GetProperty("getMinutes");
    auto secondFuncJsVal = timeObj->GetProperty("getSeconds");
    if (!(hourFuncJsVal->IsFunction() && minuteFuncJsVal->IsFunction() && secondFuncJsVal->IsFunction())) {
        return pickerTime;
    }
    auto hourFunc = JSRef<JSFunc>::Cast(hourFuncJsVal);
    auto minuteFunc = JSRef<JSFunc>::Cast(minuteFuncJsVal);
    auto secondFunc = JSRef<JSFunc>::Cast(secondFuncJsVal);
    JSRef<JSVal> hour = hourFunc->Call(timeObj);
    JSRef<JSVal> minute = minuteFunc->Call(timeObj);
    JSRef<JSVal> second = secondFunc->Call(timeObj);

    if (hour->IsNumber() && minute->IsNumber() && second->IsNumber()) {
        pickerTime.SetHour(hour->ToNumber<int32_t>());
        pickerTime.SetMinute(minute->ToNumber<int32_t>());
        pickerTime.SetSecond(second->ToNumber<int32_t>());
    }
    return pickerTime;
}

PickerDate JSTimePickerDialog::ParseDate(const JSRef<JSVal>& dateVal)
{
    auto pickerDate = PickerDate();
    if (!dateVal->IsObject()) {
        return pickerDate;
    }
    auto dateObj = JSRef<JSObject>::Cast(dateVal);
    auto yearFuncJsVal = dateObj->GetProperty("getFullYear");
    auto monthFuncJsVal = dateObj->GetProperty("getMonth");
    auto dateFuncJsVal = dateObj->GetProperty("getDate");
    if (!(yearFuncJsVal->IsFunction() && monthFuncJsVal->IsFunction() && dateFuncJsVal->IsFunction())) {
        return pickerDate;
    }
    auto yearFunc = JSRef<JSFunc>::Cast(yearFuncJsVal);
    auto monthFunc = JSRef<JSFunc>::Cast(monthFuncJsVal);
    auto dateFunc = JSRef<JSFunc>::Cast(dateFuncJsVal);
    JSRef<JSVal> year = yearFunc->Call(dateObj);
    JSRef<JSVal> month = monthFunc->Call(dateObj);
    JSRef<JSVal> date = dateFunc->Call(dateObj);

    if (year->IsNumber() && month->IsNumber() && date->IsNumber()) {
        pickerDate.SetYear(year->ToNumber<int32_t>());
        pickerDate.SetMonth(month->ToNumber<int32_t>() + 1); // 0-11 means 1 to 12 months
        pickerDate.SetDay(date->ToNumber<int32_t>());
    }
    return pickerDate;
}
} // namespace OHOS::Ace::Framework
