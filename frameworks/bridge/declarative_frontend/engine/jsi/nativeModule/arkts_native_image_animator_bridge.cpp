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
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_image_animator_bridge.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/memory/referenced.h"
#include "bridge/declarative_frontend/engine/jsi/components/arkts_native_api.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"
#include "bridge/declarative_frontend/jsview/js_image_animator.h"
#include "core/components/declaration/image/image_animator_declaration.h"

namespace OHOS::Ace::NG {
constexpr int32_t DEFAULT_DURATION = 1000; // ms

constexpr int NUM_0 = 0;
constexpr int NUM_1 = 1;
constexpr int NUM_2 = 2;
constexpr int NUM_3 = 3;
constexpr int NUM_4 = 4;
constexpr int NUM_5 = 5;
constexpr int NUM_6 = 6;
constexpr int NUM_7 = 7;
constexpr int32_t IMAGESIZE = 4;
ArkUINativeModuleValue ImageAnimatorBridge::SetState(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsNumber()) {
        int32_t state = secondArg->Int32Value(vm);
        GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().SetState(nativeNode, state);
    } else {
        GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().ResetState(nativeNode);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ImageAnimatorBridge::ResetState(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().ResetState(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ImageAnimatorBridge::SetDuration(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t duration = DEFAULT_DURATION;
    if (secondArg->IsNumber()) {
        duration = secondArg->Int32Value(vm);
        if (duration < 0) {
            duration = DEFAULT_DURATION;
        }
    }
    secondArg->Int32Value(vm);
    GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().SetDuration(nativeNode, duration);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ImageAnimatorBridge::SetFixedSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    uint32_t fixedSize = 1;
    if (secondArg->IsBoolean()) {
        fixedSize = static_cast<uint32_t>(secondArg->ToBoolean(vm)->Value());
        GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().SetFixedSize(nativeNode, fixedSize);
    } else {
        GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().ResetFixedSize(nativeNode);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ImageAnimatorBridge::ResetFixedSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().ResetFixedSize(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ImageAnimatorBridge::SetFillMode(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    if (secondArg->IsNumber()) {
        int32_t fillMode = secondArg->Int32Value(vm);
        GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().SetFillMode(nativeNode, fillMode);
    } else {
        GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().ResetFillMode(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ImageAnimatorBridge::ResetFillMode(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().ResetFillMode(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ImageAnimatorBridge::SetReverse(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    if (secondArg->IsBoolean()) {
        uint32_t value = static_cast<uint32_t>(secondArg->ToBoolean(vm)->Value());
        GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().SetReverse(nativeNode, value);
    } else {
        GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().ResetReverse(nativeNode);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ImageAnimatorBridge::ResetReverse(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().ResetReverse(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ImageAnimatorBridge::SetImages(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> fourthArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> fifthArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    Local<JSValueRef> sixthArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    Local<JSValueRef> seventhArg = runtimeCallInfo->GetCallArgRef(NUM_6);
    Local<JSValueRef> eighthArg = runtimeCallInfo->GetCallArgRef(NUM_7);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t arrayLength = eighthArg->Int32Value(vm);
    if (arrayLength <= 0) {
        return panda::JSValueRef::Undefined(vm);
    }
    if (secondArg->IsNull() || !secondArg->IsArray(vm) || thirdArg->IsNull() || !thirdArg->IsArray(vm) ||
        fourthArg->IsNull() || !fourthArg->IsArray(vm) || fifthArg->IsNull() || !fifthArg->IsArray(vm) ||
        sixthArg->IsNull() || !sixthArg->IsArray(vm) || seventhArg->IsNull() || !seventhArg->IsArray(vm)) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto parseDimensionStruct = [](const EcmaVM* vm, const Local<JSValueRef>& arg) {
        CalcDimension val(0.0, DimensionUnit::VP);
        ArkTSUtils::ParseJsDimensionVp(vm, arg, val);
        return val;
    };
    auto parseInt32 = [](const EcmaVM* vm, const Local<JSValueRef>& arg) {
        if (arg->IsNumber()) {
            return arg->Int32Value(vm);
        }
        return 0;
    };
    auto srcArray = std::make_unique<std::string[]>(arrayLength);
    auto calcDimension = std::make_unique<CalcDimension[]>(arrayLength * 4);
    auto durationArray = std::make_unique<int32_t[]>(arrayLength);
    if (!ArkTSUtils::ParseStringArray(vm, secondArg, srcArray.get(), arrayLength) ||
        !ArkTSUtils::ParseArray<CalcDimension>(
            vm, thirdArg, calcDimension.get() + arrayLength * NUM_0, arrayLength, parseDimensionStruct) ||
        !ArkTSUtils::ParseArray<CalcDimension>(
            vm, fourthArg, calcDimension.get() + arrayLength * NUM_1, arrayLength, parseDimensionStruct) ||
        !ArkTSUtils::ParseArray<CalcDimension>(
            vm, fifthArg, calcDimension.get() + arrayLength * NUM_2, arrayLength, parseDimensionStruct) ||
        !ArkTSUtils::ParseArray<CalcDimension>(
            vm, sixthArg, calcDimension.get() + arrayLength * NUM_3, arrayLength, parseDimensionStruct) ||
        !ArkTSUtils::ParseArray<int32_t>(vm, seventhArg, durationArray.get(), arrayLength, parseInt32)) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto images = std::make_unique<ImagePropertiesStruct[]>(arrayLength);
    for (int32_t i = 0; i < arrayLength; i++) {
        images[i].src = srcArray[i].c_str();
        for (int32_t j = 0; j < IMAGESIZE; j++) {
            images[i].number[j] = calcDimension[arrayLength * i + j].Value();
            images[i].unit[j] = static_cast<int8_t>(calcDimension[arrayLength * i + j].Unit());
            images[i].calc[j] = const_cast<char*>(calcDimension[arrayLength * i + j].CalcValue().c_str());
        }
        images[i].duration = *(durationArray.get() + i);
    }
    GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().SetImages(nativeNode, images.get(), arrayLength);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ImageAnimatorBridge::ResetImages(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().ResetImages(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ImageAnimatorBridge::SetIteration(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t value;
    if (secondArg->IsNumber()) {
        value = secondArg->Int32Value(vm);
    } else {
        return panda::JSValueRef::Undefined(vm);
    }

    GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().SetImageAnimatorIteration(nativeNode, value);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ImageAnimatorBridge::ResetIteration(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetImageAnimatorModifier().ResetImageAnimatorIteration(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
} // namespace OHOS::Ace::NG
