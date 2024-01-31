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
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_radio_bridge.h"

#include "core/interfaces/native/node/api.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"
#include "core/components/checkable/checkable_theme.h"

namespace OHOS::Ace::NG {
constexpr int NUM_0 = 0;
constexpr int NUM_1 = 1;
constexpr int NUM_2 = 2;
constexpr int NUM_3 = 3;
ArkUINativeModuleValue RadioBridge::SetRadioChecked(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    bool isCheck = secondArg->ToBoolean(vm)->Value();
    GetArkUIInternalNodeAPI()->GetRadioModifier().SetRadioChecked(nativeNode, isCheck);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::ResetRadioChecked(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetRadioModifier().ResetRadioChecked(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::SetRadioStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> checkedBackgroundColor = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> uncheckedBorderColor = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> indicatorColor = runtimeCallInfo->GetCallArgRef(NUM_3);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, panda::NativePointerRef::New(vm, nullptr));
    auto radioTheme = pipeline->GetTheme<RadioTheme>();
    CHECK_NULL_RETURN(radioTheme, panda::NativePointerRef::New(vm, nullptr));

    Color checkedBackgroundColorVal;
    if (checkedBackgroundColor->IsNull() || checkedBackgroundColor->IsUndefined() ||
        !ArkTSUtils::ParseJsColorAlpha(vm, checkedBackgroundColor, checkedBackgroundColorVal)) {
        checkedBackgroundColorVal = radioTheme->GetActiveColor();
    }
    Color uncheckedBorderColorVal;
    if (uncheckedBorderColor->IsNull() || uncheckedBorderColor->IsUndefined() ||
        !ArkTSUtils::ParseJsColorAlpha(vm, uncheckedBorderColor, uncheckedBorderColorVal)) {
        uncheckedBorderColorVal = radioTheme->GetInactiveColor();
    }
    Color indicatorColorVal;
    if (indicatorColor->IsNull() || indicatorColor->IsUndefined() ||
        !ArkTSUtils::ParseJsColorAlpha(vm, indicatorColor, indicatorColorVal)) {
        indicatorColorVal = radioTheme->GetPointColor();
    }

    GetArkUIInternalNodeAPI()->GetRadioModifier().SetRadioStyle(nativeNode,
        checkedBackgroundColorVal.GetValue(), uncheckedBorderColorVal.GetValue(), indicatorColorVal.GetValue());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::ResetRadioStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetRadioModifier().ResetRadioStyle(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::SetRadioWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0); //0 is node arguments
    Local<JSValueRef> jsValue = runtimeCallInfo->GetCallArgRef(1); //1 is JsValue
    void* nativeNode = nodeArg->ToNativePointer(vm)->Value();

    CalcDimension width;
    double valueResult = jsValue->ToNumber(vm)->Value();
    if (!ArkTSUtils::ParseJsDimensionVp(vm, jsValue, width) || LessNotEqual(valueResult, 0.0)) {
        GetArkUIInternalNodeAPI()->GetRadioModifier().ResetRadioWidth(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    GetArkUIInternalNodeAPI()->GetRadioModifier().SetRadioWidth(
        nativeNode, width.Value(), static_cast<int>(width.Unit()));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::ResetRadioWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0); //0 is node arguments
    void* nativeNode = nodeArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetRadioModifier().ResetRadioWidth(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::SetRadioHeight(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0); //0 is node arguments
    Local<JSValueRef> jsValue = runtimeCallInfo->GetCallArgRef(1); //1 is Jsvalue
    void* nativeNode = nodeArg->ToNativePointer(vm)->Value();

    CalcDimension height;
    if (!ArkTSUtils::ParseJsDimensionVpNG(vm, jsValue, height)) {
        GetArkUIInternalNodeAPI()->GetRadioModifier().ResetRadioHeight(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    GetArkUIInternalNodeAPI()->GetRadioModifier().SetRadioHeight(
        nativeNode, height.Value(), static_cast<int>(height.Unit()));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::ResetRadioHeight(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0); //0 is node arguments
    void* nativeNode = nodeArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetRadioModifier().ResetRadioHeight(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::SetRadioSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0);     //0 is node arguments
    Local<JSValueRef> widthValue = runtimeCallInfo->GetCallArgRef(1);  //1 is width value
    Local<JSValueRef> heightValue = runtimeCallInfo->GetCallArgRef(2); //2 is height value
    void* nativeNode = nodeArg->ToNativePointer(vm)->Value();

    CalcDimension width = 0.0_vp;
    CalcDimension height = 0.0_vp;
    bool hasWidth = (!widthValue->IsNull() && !widthValue->IsUndefined() &&
        ArkTSUtils::ParseJsDimensionVp(vm, widthValue, width));
    bool hasHeight = (!heightValue->IsNull() && !heightValue->IsUndefined() &&
        ArkTSUtils::ParseJsDimensionVp(vm, heightValue, height));
    if (!hasWidth && !hasHeight) {
        GetArkUIInternalNodeAPI()->GetRadioModifier().ResetRadioSize(nativeNode);
    } else {
        GetArkUIInternalNodeAPI()->GetRadioModifier().SetRadioSize(nativeNode,
            width.Value(), static_cast<int>(width.Unit()), height.Value(), static_cast<int>(height.Unit()));
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::ResetRadioSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0); //0 is node arguments
    void* nativeNode = nodeArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetRadioModifier().ResetRadioSize(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::SetRadioHoverEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0);  //0 is node arguments
    Local<JSValueRef> valueArg = runtimeCallInfo->GetCallArgRef(1); //1 is Jsvalue
    void* nativeNode = nodeArg->ToNativePointer(vm)->Value();

    if (valueArg->IsUndefined() || !valueArg->IsNumber()) {
        GetArkUIInternalNodeAPI()->GetRadioModifier().ResetRadioHoverEffect(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    int32_t intValue = valueArg->Int32Value(vm);
    GetArkUIInternalNodeAPI()->GetRadioModifier().SetRadioHoverEffect(nativeNode, intValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::ResetRadioHoverEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0); //0 is node arguments
    void* nativeNode = nodeArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetRadioModifier().ResetRadioHoverEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::SetRadioPadding(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0);   //0 is node arguments
    Local<JSValueRef> topArg = runtimeCallInfo->GetCallArgRef(1);    //1 is top arguments
    Local<JSValueRef> rightArg = runtimeCallInfo->GetCallArgRef(2);  //2 is right arguments
    Local<JSValueRef> bottomArg = runtimeCallInfo->GetCallArgRef(3); //3 is bottom arguments
    Local<JSValueRef> leftArg = runtimeCallInfo->GetCallArgRef(4);   //4 is left arguments
    void *nativeNode = nodeArg->ToNativePointer(vm)->Value();

    struct ArkUISizeType top = { 0.0, static_cast<int8_t>(DimensionUnit::VP) };
    struct ArkUISizeType right = { 0.0, static_cast<int8_t>(DimensionUnit::VP) };
    struct ArkUISizeType bottom = { 0.0, static_cast<int8_t>(DimensionUnit::VP) };
    struct ArkUISizeType left = { 0.0, static_cast<int8_t>(DimensionUnit::VP) };

    CalcDimension topDimen(0, DimensionUnit::VP);
    CalcDimension rightDimen(0, DimensionUnit::VP);
    CalcDimension bottomDimen(0, DimensionUnit::VP);
    CalcDimension leftDimen(0, DimensionUnit::VP);
    ArkTSUtils::ParsePadding(vm, topArg, topDimen, top);
    ArkTSUtils::ParsePadding(vm, rightArg, rightDimen, right);
    ArkTSUtils::ParsePadding(vm, bottomArg, bottomDimen, bottom);
    ArkTSUtils::ParsePadding(vm, leftArg, leftDimen, left);
    GetArkUIInternalNodeAPI()->GetRadioModifier().SetRadioPadding(nativeNode, &top, &right, &bottom, &left);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::ResetRadioPadding(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0); //0 is node arguments
    void *nativeNode = nodeArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetRadioModifier().ResetRadioPadding(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::SetRadioResponseRegion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0);   //0 is node arguments
    Local<JSValueRef> valueArg = runtimeCallInfo->GetCallArgRef(1);  //1 is Jsvalue
    Local<JSValueRef> lengthArg = runtimeCallInfo->GetCallArgRef(2); //2 is length arguments
    void* nativeNode = nodeArg->ToNativePointer(vm)->Value();
    uint32_t length = static_cast<uint32_t>(lengthArg->Int32Value(vm));
    double regionArray[length];
    int32_t regionUnits[length];
    if (!ArkTSUtils::ParseResponseRegion(vm, valueArg, regionArray, regionUnits, length)) {
        GetArkUIInternalNodeAPI()->GetRadioModifier().ResetRadioResponseRegion(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    GetArkUIInternalNodeAPI()->GetRadioModifier().SetRadioResponseRegion(
        nativeNode, regionArray, regionUnits, length);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RadioBridge::ResetRadioResponseRegion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0); //0 is node arguments
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetRadioModifier().ResetRadioResponseRegion(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
}
