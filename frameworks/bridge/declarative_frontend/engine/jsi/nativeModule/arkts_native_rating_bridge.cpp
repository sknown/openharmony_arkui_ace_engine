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
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_rating_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/rating/rating_model_ng.h"

namespace OHOS::Ace::NG {
constexpr double STEPS_DEFAULT = 0.5;
constexpr double STEPS_MIN_SIZE = 0.1;
constexpr int32_t STARS_DEFAULT = 5;
constexpr int NUM_0 = 0;
constexpr int NUM_1 = 1;
constexpr int NUM_2 = 2;
constexpr int NUM_3 = 3;
const char* NODEPTR_OF_UINODE = "nodePtr_";
panda::Local<panda::JSValueRef> JsRatingChangeCallback(panda::JsiRuntimeCallInfo* runtimeCallInfo)
{
    auto vm = runtimeCallInfo->GetVM();
    int32_t argc = static_cast<int32_t>(runtimeCallInfo->GetArgsNumber());
    if (argc != 1) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto firstArg = runtimeCallInfo->GetCallArgRef(0);
    if (!firstArg->IsNumber()) {
        return panda::JSValueRef::Undefined(vm);
    }
    double value = firstArg->ToNumber(vm)->Value();
    auto ref = runtimeCallInfo->GetThisRef();
    auto obj = ref->ToObject(vm);
    if (obj->GetNativePointerFieldCount() < 1) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto frameNode = static_cast<FrameNode*>(obj->GetNativePointerField(0));
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    RatingModelNG::SetChangeValue(frameNode, value);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RatingBridge::SetStars(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    int32_t stars = secondArg->Int32Value(vm);
    if (stars <= 0) {
        stars = STARS_DEFAULT;
    }
    GetArkUINodeModifiers()->getRatingModifier()->setStars(nativeNode, stars);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RatingBridge::ResetStars(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getRatingModifier()->resetStars(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue RatingBridge::SetRatingStepSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    if (secondArg->IsNull() || !secondArg->IsNumber()) {
        GetArkUINodeModifiers()->getRatingModifier()->resetRatingStepSize(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    auto steps = secondArg->ToNumber(vm)->Value();
    if (LessNotEqual(steps, STEPS_MIN_SIZE)) {
        steps = STEPS_DEFAULT;
    }
    GetArkUINodeModifiers()->getRatingModifier()->setRatingStepSize(nativeNode, steps);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RatingBridge::ResetRatingStepSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getRatingModifier()->resetRatingStepSize(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RatingBridge::SetStarStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> backgroundUriArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> foregroundUriArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> secondaryUriArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());

    std::string backgroundUri;
    if (backgroundUriArg->IsString(vm)) {
        backgroundUri = backgroundUriArg->ToString(vm)->ToString();
    }

    std::string foregroundUri;
    if (foregroundUriArg->IsString(vm)) {
        foregroundUri = foregroundUriArg->ToString(vm)->ToString();
    }

    std::string secondaryUri;
    if (secondaryUriArg->IsString(vm)) {
        secondaryUri = secondaryUriArg->ToString(vm)->ToString();
    }

    if (secondaryUri.empty() && !backgroundUri.empty()) {
        secondaryUri = backgroundUri;
    }

    GetArkUINodeModifiers()->getRatingModifier()->setStarStyle(
        nativeNode, backgroundUri.c_str(), foregroundUri.c_str(), secondaryUri.c_str());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RatingBridge::ResetStarStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getRatingModifier()->resetStarStyle(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RatingBridge::SetContentModifierBuilder(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto* frameNode = reinterpret_cast<FrameNode*>(firstArg->ToNativePointer(vm)->Value());
    if (!secondArg->IsObject(vm)) {
        RatingModelNG::SetBuilderFunc(frameNode, nullptr);
        return panda::JSValueRef::Undefined(vm);
    }
    panda::CopyableGlobal<panda::ObjectRef> obj(vm, secondArg);
    auto containerId = Container::CurrentId();
    RatingModelNG::SetBuilderFunc(frameNode,
        [vm, frameNode, obj = std::move(obj), containerId](
            RatingConfiguration config) -> RefPtr<FrameNode> {
            ContainerScope scope(containerId);
            auto context = ArkTSUtils::GetContext(vm);
            CHECK_EQUAL_RETURN(context->IsUndefined(), true, nullptr);
            const char* keys[] = { "stars", "indicator", "rating", "stepSize", "enabled", "triggerChange" };
            Local<JSValueRef> values[] = { panda::NumberRef::New(vm, config.starNum_),
                panda::BooleanRef::New(vm, config.isIndicator_),
                panda::NumberRef::New(vm, config.rating_),
                panda::NumberRef::New(vm, config.stepSize_),
                panda::BooleanRef::New(vm, config.enabled_),
                panda::FunctionRef::New(vm, JsRatingChangeCallback) };
            auto rating = panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, values);
            rating->SetNativePointerFieldCount(vm, 1);
            rating->SetNativePointerField(vm, 0, static_cast<void*>(frameNode));
            panda::Local<panda::JSValueRef> params[NUM_2] = { context, rating };
            LocalScope pandaScope(vm);
            panda::TryCatch trycatch(vm);
            auto makeFunc = obj.ToLocal()->Get(vm, panda::StringRef::NewFromUtf8(vm, "makeContentModifierNode"));
            CHECK_EQUAL_RETURN(makeFunc->IsFunction(vm), false, nullptr);
            panda::Local<panda::FunctionRef> func = makeFunc;
            auto result = func->Call(vm, obj.ToLocal(), params, 2);
            JSNApi::ExecutePendingJob(vm);
            CHECK_EQUAL_RETURN(result.IsEmpty() || trycatch.HasCaught() || !result->IsObject(vm), true, nullptr);
            panda::Local<panda::JSValueRef> nodeptr =
                result->ToObject(vm)->Get(vm, panda::StringRef::NewFromUtf8(vm, NODEPTR_OF_UINODE));
            CHECK_EQUAL_RETURN(nodeptr.IsEmpty() || nodeptr->IsUndefined() || nodeptr->IsNull(), true, nullptr);
            auto* frameNode = reinterpret_cast<FrameNode*>(nodeptr->ToNativePointer(vm)->Value());
            CHECK_NULL_RETURN(frameNode, nullptr);
            return AceType::Claim(frameNode);
        });
    return panda::JSValueRef::Undefined(vm);
}
} // namespace OHOS::Ace::NG