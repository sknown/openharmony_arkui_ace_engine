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

#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_toggle_bridge.h"

#include "frameworks/bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"

namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t INDEX_FRAME_NODE_0 = 0;
constexpr uint32_t INDEX_ARGUMENT_1 = 1;
constexpr uint32_t INDEX_ARGUMENT_2 = 2;
constexpr uint32_t INDEX_ARGUMENT_3 = 3;
constexpr uint32_t INDEX_ARGUMENT_4 = 4;

void SetHeightInner(const EcmaVM* vm, const Local<JSValueRef>& nodeArg, const Local<JSValueRef>& heightArg)
{
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    CalcDimension height;
    if (!ArkTSUtils::ParseJsDimensionVpNG(vm, heightArg, height)) {
        GetArkUINodeModifiers()->getToggleModifier()->resetToggleHeight(nativeNode);
    } else {
        if (height.IsNegative()) {
            GetArkUINodeModifiers()->getToggleModifier()->resetToggleHeight(nativeNode);
        } else {
            GetArkUINodeModifiers()->getToggleModifier()->setToggleHeight(
                nativeNode, height.Value(), static_cast<int32_t>(height.Unit()));
        }
    }
}

void PushDimensionVector(const std::optional<Dimension>& valueDimen, std::vector<ArkUI_Float32>& dimensions)
{
    dimensions.push_back(static_cast<double>(valueDimen.has_value()));
    if (valueDimen.has_value()) {
        dimensions.push_back(static_cast<double>(valueDimen.value().Value()));
        dimensions.push_back(static_cast<double>(valueDimen.value().Unit()));
    } else {
        dimensions.push_back(0);
        dimensions.push_back(0);
    }
}

void PushToggleDimension(
    ArkUIRuntimeCallInfo* runtimeCallInfo, EcmaVM* vm, std::vector<ArkUI_Float32>& fontSizesVector, int32_t argIndex)
{
    Local<JSValueRef> arg = runtimeCallInfo->GetCallArgRef(argIndex);
    std::optional<CalcDimension> dimensionOptional = std::nullopt;
    CalcDimension parsedDimension;
    if (ArkTSUtils::ParseJsDimensionVp(vm, arg, parsedDimension)) {
        dimensionOptional = parsedDimension;
    }
    PushDimensionVector(dimensionOptional, fontSizesVector);
}

void PutToggleDimensionParameters(
    ArkUIRuntimeCallInfo* runtimeCallInfo, EcmaVM* vm, std::vector<ArkUI_Float32>& lengthVector)
{
    std::vector<int32_t> indexVector = { INDEX_ARGUMENT_1, INDEX_ARGUMENT_2, INDEX_ARGUMENT_3, INDEX_ARGUMENT_4 };
    for (size_t index = 0; index < indexVector.size(); index++) {
        PushToggleDimension(runtimeCallInfo, vm, lengthVector, indexVector[index]);
    }
}
} // namespace

ArkUINativeModuleValue ToggleBridge::SetSelectedColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> colorArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());

    Color color;
    if (colorArg->IsNull() || colorArg->IsUndefined() || !ArkTSUtils::ParseJsColorAlpha(vm, colorArg, color)) {
        GetArkUINodeModifiers()->getToggleModifier()->resetToggleSelectedColor(nativeNode);
    } else {
        GetArkUINodeModifiers()->getToggleModifier()->setToggleSelectedColor(nativeNode, color.GetValue());
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::ResetSelectedColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getToggleModifier()->resetToggleSelectedColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::SetSwitchPointColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> colorArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());

    Color color;
    if (colorArg->IsNull() || colorArg->IsUndefined() || !ArkTSUtils::ParseJsColorAlpha(vm, colorArg, color)) {
        GetArkUINodeModifiers()->getToggleModifier()->resetToggleSwitchPointColor(nativeNode);
    } else {
        GetArkUINodeModifiers()->getToggleModifier()->setToggleSwitchPointColor(nativeNode, color.GetValue());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::ResetSwitchPointColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getToggleModifier()->resetToggleSwitchPointColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::SetSwitchStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> pointRadiusArg = runtimeCallInfo->GetCallArgRef(INDEX_ARGUMENT_1);
    Local<JSValueRef> unselectedColorArg = runtimeCallInfo->GetCallArgRef(INDEX_ARGUMENT_2);
    Local<JSValueRef> pointColorArg = runtimeCallInfo->GetCallArgRef(INDEX_ARGUMENT_3);
    Local<JSValueRef> trackRadiusArg = runtimeCallInfo->GetCallArgRef(INDEX_ARGUMENT_4);
    CalcDimension pointRadius;
    if (!pointRadiusArg->IsUndefined() &&
        ArkTSUtils::ParseJsDimensionNG(vm, pointRadiusArg, pointRadius, DimensionUnit::VP) &&
        !pointRadius.IsNegative()) {
        GetArkUINodeModifiers()->getToggleModifier()->setTogglePointRadius(
            nativeNode, pointRadius.Value(), static_cast<int>(pointRadius.Unit()));
    } else {
        GetArkUINodeModifiers()->getToggleModifier()->resetTogglePointRadius(nativeNode);
    }
    Color unselectedColor;
    if (unselectedColorArg->IsNull() || unselectedColorArg->IsUndefined() ||
        !ArkTSUtils::ParseJsColorAlpha(vm, unselectedColorArg, unselectedColor)) {
        GetArkUINodeModifiers()->getToggleModifier()->resetToggleUnselectedColor(nativeNode);
    } else {
        GetArkUINodeModifiers()->getToggleModifier()->setToggleUnselectedColor(nativeNode, unselectedColor.GetValue());
    }
    Color pointColor;
    if (pointColorArg->IsNull() || pointColorArg->IsUndefined() ||
        !ArkTSUtils::ParseJsColorAlpha(vm, pointColorArg, pointColor)) {
        GetArkUINodeModifiers()->getToggleModifier()->resetToggleSwitchPointColor(nativeNode);
    } else {
        GetArkUINodeModifiers()->getToggleModifier()->setToggleSwitchPointColor(nativeNode, pointColor.GetValue());
    }
    CalcDimension trackRadius;
    if (!trackRadiusArg->IsUndefined() &&
        ArkTSUtils::ParseJsDimensionNG(vm, trackRadiusArg, trackRadius, DimensionUnit::VP) &&
        !trackRadius.IsNegative()) {
        GetArkUINodeModifiers()->getToggleModifier()->setToggleTrackBorderRadius(
            nativeNode, trackRadius.Value(), static_cast<int>(trackRadius.Unit()));
    } else {
        GetArkUINodeModifiers()->getToggleModifier()->resetToggleTrackBorderRadius(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::ResetSwitchStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getToggleModifier()->resetTogglePointRadius(nativeNode);
    GetArkUINodeModifiers()->getToggleModifier()->resetToggleUnselectedColor(nativeNode);
    GetArkUINodeModifiers()->getToggleModifier()->resetToggleSwitchPointColor(nativeNode);
    GetArkUINodeModifiers()->getToggleModifier()->resetToggleTrackBorderRadius(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::SetHeight(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(INDEX_FRAME_NODE_0);
    Local<JSValueRef> heightArg = runtimeCallInfo->GetCallArgRef(INDEX_ARGUMENT_1);
    SetHeightInner(vm, nodeArg, heightArg);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::ResetHeight(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(INDEX_FRAME_NODE_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getToggleModifier()->resetToggleHeight(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::SetResponseRegion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(INDEX_FRAME_NODE_0);
    Local<JSValueRef> regionArrayArg = runtimeCallInfo->GetCallArgRef(INDEX_ARGUMENT_1);
    Local<JSValueRef> lengthArg = runtimeCallInfo->GetCallArgRef(INDEX_ARGUMENT_2);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    uint32_t length = lengthArg->Uint32Value(vm);
    ArkUI_Float32 regionValueArray[length];
    int32_t regionUnitsArray[length];
    if (!ArkTSUtils::ParseResponseRegion(vm, regionArrayArg, regionValueArray, regionUnitsArray, length)) {
        GetArkUINodeModifiers()->getToggleModifier()->resetToggleResponseRegion(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    GetArkUINodeModifiers()->getToggleModifier()->setToggleResponseRegion(
        nativeNode, regionValueArray, regionUnitsArray, length);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::ResetResponseRegion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(INDEX_FRAME_NODE_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getToggleModifier()->resetToggleResponseRegion(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::SetPadding(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(INDEX_FRAME_NODE_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    std::vector<ArkUI_Float32> lengthVector;
    PutToggleDimensionParameters(runtimeCallInfo, vm, lengthVector);
    GetArkUINodeModifiers()->getToggleModifier()->setTogglePadding(
        nativeNode, lengthVector.data(), lengthVector.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::ResetPadding(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(INDEX_FRAME_NODE_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getToggleModifier()->resetTogglePadding(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::SetBackgroundColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(INDEX_FRAME_NODE_0);
    Local<JSValueRef> colorArg = runtimeCallInfo->GetCallArgRef(INDEX_ARGUMENT_1);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    Color color;
    if (!ArkTSUtils::ParseJsColorAlpha(vm, colorArg, color)) {
        GetArkUINodeModifiers()->getToggleModifier()->resetToggleBackgroundColor(nativeNode);
    } else {
        GetArkUINodeModifiers()->getToggleModifier()->setToggleBackgroundColor(nativeNode, color.GetValue());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::ResetBackgroundColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(INDEX_FRAME_NODE_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getToggleModifier()->resetToggleBackgroundColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::SetHoverEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(INDEX_FRAME_NODE_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> hoverEffectArg = runtimeCallInfo->GetCallArgRef(INDEX_ARGUMENT_1);

    if (hoverEffectArg->IsUndefined() || !hoverEffectArg->IsNumber()) {
        GetArkUINodeModifiers()->getToggleModifier()->resetToggleHoverEffect(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    int32_t hoverEffectValue = hoverEffectArg->Int32Value(vm);
    GetArkUINodeModifiers()->getToggleModifier()->setToggleHoverEffect(nativeNode, hoverEffectValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ToggleBridge::ResetHoverEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(INDEX_FRAME_NODE_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getToggleModifier()->resetToggleHoverEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
} // namespace OHOS::Ace::NG
