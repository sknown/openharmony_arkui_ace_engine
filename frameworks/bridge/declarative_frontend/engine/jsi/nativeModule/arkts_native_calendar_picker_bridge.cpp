/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_calendar_picker_bridge.h"
#include "core/components/calendar/calendar_theme.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_common_bridge.h"

namespace OHOS::Ace::NG {
constexpr int NUM_0 = 0;
constexpr int NUM_1 = 1;
constexpr int NUM_2 = 2;
constexpr int NUM_3 = 3;
constexpr int SIZE_OF_TWO = 2;
constexpr Dimension DEFAULT_TEXTSTYLE_FONTSIZE = 16.0_fp;

void ParseCalendarPickerPadding(
    const EcmaVM* vm, const Local<JSValueRef>& value, CalcDimension& dim, ArkUISizeType& result)
{
    if (value->IsNull() || value->IsUndefined()) {
        dim.SetValue(-1);
        dim.SetUnit(DimensionUnit::VP);
        result.unit = static_cast<int8_t>(dim.Unit());
        result.value = dim.Value();
    }
    if (ArkTSUtils::ParseJsDimensionVp(vm, value, dim)) {
        if (LessOrEqual(dim.Value(), 0.0)) {
            dim.SetValue(-1);
            dim.SetUnit(DimensionUnit::VP);
        }
        result.unit = static_cast<int8_t>(dim.Unit());
        if (dim.CalcValue() != "") {
            result.string = dim.CalcValue().c_str();
        } else {
            result.value = dim.Value();
        }
    }
}

ArkUINativeModuleValue CalendarPickerBridge::SetTextStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, panda::NativePointerRef::New(vm, nullptr));
    RefPtr<CalendarTheme> calendarTheme = pipeline->GetTheme<CalendarTheme>();
    CHECK_NULL_RETURN(calendarTheme, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> colorArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> fontSizeArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> fontWeightArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Color textColor = calendarTheme->GetEntryFontColor();
    if (!colorArg->IsUndefined()) {
        ArkTSUtils::ParseJsColorAlpha(vm, colorArg, textColor);
    }
    CalcDimension fontSizeData(DEFAULT_TEXTSTYLE_FONTSIZE);
    std::string fontSize = fontSizeData.ToString();
    if (ArkTSUtils::ParseJsDimensionFp(vm, fontSizeArg, fontSizeData) && !fontSizeData.IsNegative() &&
        fontSizeData.Unit() != DimensionUnit::PERCENT) {
        fontSize = fontSizeData.ToString();
    }
    std::string fontWeight = "regular";
    if (fontWeightArg->IsString(vm) || fontWeightArg->IsNumber()) {
        fontWeight = fontWeightArg->ToString(vm)->ToString();
    }
    GetArkUINodeModifiers()->getCalendarPickerModifier()->setTextStyle(
        nativeNode, textColor.GetValue(), fontSize.c_str(), fontWeight.c_str());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CalendarPickerBridge::ResetTextStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCalendarPickerModifier()->resetTextStyle(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CalendarPickerBridge::SetEdgeAlign(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> alignTypeArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> dxArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> dyArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (!alignTypeArg->IsNull() && !alignTypeArg->IsUndefined() && alignTypeArg->IsNumber()) {
        int alignType = alignTypeArg->ToNumber(vm)->Value();
        CalcDimension dx;
        CalcDimension dy;
        if (!dxArg->IsNull() && !dxArg->IsUndefined()) {
            ArkTSUtils::ParseJsDimensionVp(vm, dxArg, dx);
        }
        if (!dyArg->IsNull() && !dyArg->IsUndefined()) {
            ArkTSUtils::ParseJsDimensionVp(vm, dyArg, dy);
        }

        ArkUI_Float32 values[SIZE_OF_TWO];
        int units[SIZE_OF_TWO];

        values[NUM_0] = dx.Value();
        units[NUM_0] = static_cast<int>(dx.Unit());
        values[NUM_1] = dy.Value();
        units[NUM_1] = static_cast<int>(dy.Unit());

        GetArkUINodeModifiers()->getCalendarPickerModifier()->setEdgeAlign(
            nativeNode, values, units, SIZE_OF_TWO, alignType);
    } else {
        GetArkUINodeModifiers()->getCalendarPickerModifier()->resetEdgeAlign(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CalendarPickerBridge::ResetEdgeAlign(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCalendarPickerModifier()->resetEdgeAlign(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CalendarPickerBridge::SetCalendarPickerPadding(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> topArg = runtimeCallInfo->GetCallArgRef(1);    // 1: index of parameter top
    Local<JSValueRef> rightArg = runtimeCallInfo->GetCallArgRef(2);  // 2: index of parameter right
    Local<JSValueRef> bottomArg = runtimeCallInfo->GetCallArgRef(3); // 3: index of parameter bottom
    Local<JSValueRef> leftArg = runtimeCallInfo->GetCallArgRef(4);   // 4: index of parameter left
    if (leftArg->IsUndefined() && rightArg->IsUndefined() && topArg->IsUndefined() && bottomArg->IsUndefined()) {
        GetArkUINodeModifiers()->getCalendarPickerModifier()->resetCalendarPickerPadding(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    struct ArkUISizeType top = { 0.0, static_cast<int8_t>(DimensionUnit::VP) };
    struct ArkUISizeType right = { 0.0, static_cast<int8_t>(DimensionUnit::VP) };
    struct ArkUISizeType bottom = { 0.0, static_cast<int8_t>(DimensionUnit::VP) };
    struct ArkUISizeType left = { 0.0, static_cast<int8_t>(DimensionUnit::VP) };

    CalcDimension topDim(0, DimensionUnit::VP);
    CalcDimension rightDim(0, DimensionUnit::VP);
    CalcDimension bottomDim(0, DimensionUnit::VP);
    CalcDimension leftDim(0, DimensionUnit::VP);
    ParseCalendarPickerPadding(vm, topArg, topDim, top);
    ParseCalendarPickerPadding(vm, rightArg, rightDim, right);
    ParseCalendarPickerPadding(vm, bottomArg, bottomDim, bottom);
    ParseCalendarPickerPadding(vm, leftArg, leftDim, left);
    GetArkUINodeModifiers()->getCalendarPickerModifier()->setCalendarPickerPadding(
        nativeNode, &top, &right, &bottom, &left);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CalendarPickerBridge::ResetCalendarPickerPadding(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCalendarPickerModifier()->resetCalendarPickerPadding(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CalendarPickerBridge::SetCalendarPickerBorder(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    Local<JSValueRef> leftArg = runtimeCallInfo->GetCallArgRef(5);   // 5: index of parameter left color
    Local<JSValueRef> rightArg = runtimeCallInfo->GetCallArgRef(6);  // 6: index of parameter right color
    Local<JSValueRef> topArg = runtimeCallInfo->GetCallArgRef(7);    // 7: index of parameter top color
    Local<JSValueRef> bottomArg = runtimeCallInfo->GetCallArgRef(8); // 8: index of parameter bottom color

    CommonBridge::SetBorder(runtimeCallInfo);

    if (leftArg->IsUndefined() && rightArg->IsUndefined() && topArg->IsUndefined() && bottomArg->IsUndefined()) {
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_RETURN(pipeline, panda::NativePointerRef::New(vm, nullptr));
        RefPtr<CalendarTheme> calendarTheme = pipeline->GetTheme<CalendarTheme>();
        CHECK_NULL_RETURN(calendarTheme, panda::NativePointerRef::New(vm, nullptr));
        GetArkUINodeModifiers()->getCalendarPickerModifier()->setCalendarPickerBorder(
            nativeNode, calendarTheme->GetEntryBorderColor().GetValue());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CalendarPickerBridge::ResetCalendarPickerBorder(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCalendarPickerModifier()->resetCalendarPickerBorder(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
} // namespace OHOS::Ace::NG
