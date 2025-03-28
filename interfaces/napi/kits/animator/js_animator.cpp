/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <memory>
#include <string>

#include "animator_option.h"
#include "interfaces/napi/kits/utils/napi_utils.h"
#include "napi/native_api.h"
#include "napi/native_engine/native_value.h"
#include "napi/native_node_api.h"

#include "base/log/ace_trace.h"
#include "base/log/log.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/thread/frame_trace_adapter.h"
#include "bridge/common/utils/utils.h"
#include "core/animation/animator.h"
#include "core/animation/curve.h"
#include "core/animation/curve_animation.h"
#include "core/animation/spring_motion.h"

namespace OHOS::Ace::Napi {

namespace {
constexpr size_t INTERPOLATING_SPRING_PARAMS_SIZE = 4;
constexpr char INTERPOLATING_SPRING[] = "interpolating-spring";
} // namespace

static void ParseString(napi_env env, napi_value propertyNapi, std::string& property)
{
    if (propertyNapi != nullptr) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, propertyNapi, &valueType);
        if (valueType == napi_undefined) {
            NapiThrow(env, "Required input parameters are missing.", ERROR_CODE_PARAM_INVALID);
            return;
        } else if (valueType != napi_string) {
            NapiThrow(env, "The type of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
            return;
        }

        size_t buffSize = 0;
        napi_status status = napi_get_value_string_utf8(env, propertyNapi, nullptr, 0, &buffSize);
        if (status != napi_ok || buffSize == 0) {
            return;
        }
        std::unique_ptr<char[]> propertyString = std::make_unique<char[]>(buffSize + 1);
        size_t retLen = 0;
        napi_get_value_string_utf8(env, propertyNapi, propertyString.get(), buffSize + 1, &retLen);
        property = propertyString.get();
    }
}

static void ParseInt(napi_env env, napi_value propertyNapi, int32_t& property)
{
    if (propertyNapi != nullptr) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, propertyNapi, &valueType);
        if (valueType == napi_undefined) {
            NapiThrow(env, "Required input parameters are missing.", ERROR_CODE_PARAM_INVALID);
            return;
        } else if (valueType != napi_number) {
            NapiThrow(env, "The type of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
            return;
        }
        napi_get_value_int32(env, propertyNapi, &property);
    }
}

static void ParseDouble(napi_env env, napi_value propertyNapi, double& property)
{
    if (propertyNapi != nullptr) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, propertyNapi, &valueType);
        if (valueType == napi_undefined) {
            NapiThrow(env, "Required input parameters are missing.", ERROR_CODE_PARAM_INVALID);
            return;
        } else if (valueType != napi_number) {
            NapiThrow(env, "The type of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
            return;
        }
        napi_get_value_double(env, propertyNapi, &property);
    }
}

static FillMode StringToFillMode(const std::string& fillMode)
{
    if (fillMode.compare("forwards") == 0) {
        return FillMode::FORWARDS;
    } else if (fillMode.compare("backwards") == 0) {
        return FillMode::BACKWARDS;
    } else if (fillMode.compare("both") == 0) {
        return FillMode::BOTH;
    } else {
        return FillMode::NONE;
    }
}

static AnimationDirection StringToAnimationDirection(const std::string& direction)
{
    if (direction.compare("alternate") == 0) {
        return AnimationDirection::ALTERNATE;
    } else if (direction.compare("reverse") == 0) {
        return AnimationDirection::REVERSE;
    } else if (direction.compare("alternate-reverse") == 0) {
        return AnimationDirection::ALTERNATE_REVERSE;
    } else {
        return AnimationDirection::NORMAL;
    }
}

static RefPtr<Motion> ParseOptionToMotion(const std::shared_ptr<AnimatorOption>& option)
{
    const auto& curveStr = option->easing;
    if (curveStr.back() != ')') {
        return nullptr;
    }
    std::string::size_type leftEmbracePosition = curveStr.find_last_of('(');
    if (leftEmbracePosition == std::string::npos) {
        return nullptr;
    }
    auto aniTimFuncName = curveStr.substr(0, leftEmbracePosition);
    if (aniTimFuncName.compare(INTERPOLATING_SPRING)) {
        return nullptr;
    }
    auto params = curveStr.substr(leftEmbracePosition + 1, curveStr.length() - leftEmbracePosition - 2);
    std::vector<std::string> paramsVector;
    StringUtils::StringSplitter(params, ',', paramsVector);
    if (paramsVector.size() != INTERPOLATING_SPRING_PARAMS_SIZE) {
        return nullptr;
    }
    for (auto& param : paramsVector) {
        Framework::RemoveHeadTailSpace(param);
    }
    float velocity = StringUtils::StringToFloat(paramsVector[0]);
    float mass = StringUtils::StringToFloat(paramsVector[1]);
    float stiffness = StringUtils::StringToFloat(paramsVector[2]);
    float damping = StringUtils::StringToFloat(paramsVector[3]);
    // input velocity is normalized velocity, while the velocity of arkui's springMotion is absolute velocity.
    velocity = velocity * (option->end - option->begin);
    if (LessOrEqual(mass, 0)) {
        mass = 1.0f;
    }
    if (LessOrEqual(stiffness, 0)) {
        stiffness = 1.0f;
    }
    if (LessOrEqual(damping, 0)) {
        damping = 1.0f;
    }
    return AceType::MakeRefPtr<SpringMotion>(
        option->begin, option->end, velocity, AceType::MakeRefPtr<SpringProperty>(mass, stiffness, damping));
}

static void ParseAnimatorOption(napi_env env, napi_callback_info info, std::shared_ptr<AnimatorOption>& option)
{
    size_t argc = 1;
    napi_value argv;
    napi_get_cb_info(env, info, &argc, &argv, NULL, NULL);
    if (argc != 1) {
        NapiThrow(env, "The number of parameters must be equal to 1.", ERROR_CODE_PARAM_INVALID);
        return;
    }
    napi_value durationNapi = nullptr;
    napi_value easingNapi = nullptr;
    napi_value delayNapi = nullptr;
    napi_value fillNapi = nullptr;
    napi_value directionNapi = nullptr;
    napi_value iterationsNapi = nullptr;
    napi_value beginNapi = nullptr;
    napi_value endNapi = nullptr;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv, &valueType);
    if (valueType == napi_object) {
        napi_get_named_property(env, argv, "duration", &durationNapi);
        napi_get_named_property(env, argv, "easing", &easingNapi);
        napi_get_named_property(env, argv, "delay", &delayNapi);
        napi_get_named_property(env, argv, "fill", &fillNapi);
        napi_get_named_property(env, argv, "direction", &directionNapi);
        napi_get_named_property(env, argv, "iterations", &iterationsNapi);
        napi_get_named_property(env, argv, "begin", &beginNapi);
        napi_get_named_property(env, argv, "end", &endNapi);
    } else {
        NapiThrow(env, "The type of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
        return;
    }

    int32_t duration = 0;
    int32_t delay = 0;
    int32_t iterations = 0;
    double begin = 0.0;
    double end = 0.0;
    std::string easing = "ease";
    std::string fill = "none";
    std::string direction = "normal";
    ParseString(env, easingNapi, easing);
    ParseString(env, fillNapi, fill);
    ParseString(env, directionNapi, direction);
    ParseInt(env, durationNapi, duration);
    ParseInt(env, delayNapi, delay);
    ParseInt(env, iterationsNapi, iterations);
    ParseDouble(env, beginNapi, begin);
    ParseDouble(env, endNapi, end);
    option->duration = std::max(duration, 0);
    if (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
        option->delay = delay;
    } else {
        option->delay = std::max(delay, 0);
    }
    option->iterations = iterations >= -1 ? iterations : 1;
    option->begin = begin;
    option->end = end;
    option->easing = easing;
    option->fill = fill;
    option->direction = direction;
}

static AnimatorResult* GetAnimatorResult(napi_env env, napi_callback_info info)
{
    AnimatorResult* animatorResult = nullptr;
    napi_value thisVar;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    napi_unwrap(env, thisVar, reinterpret_cast<void**>(&animatorResult));
    return animatorResult;
}

static RefPtr<Animator> GetAnimatorInResult(napi_env env, napi_callback_info info)
{
    AnimatorResult* animatorResult = GetAnimatorResult(env, info);
    if (!animatorResult) {
        return nullptr;
    }
    return animatorResult->GetAnimator();
}

static napi_value JSReset(napi_env env, napi_callback_info info)
{
    AnimatorResult* animatorResult = nullptr;
    napi_value thisVar;
    napi_get_cb_info(env, info, NULL, NULL, &thisVar, NULL);
    napi_unwrap(env, thisVar, (void**)&animatorResult);
    if (!animatorResult) {
        NapiThrow(env, "Internal error. Unwrap animator result is failed.", ERROR_CODE_INTERNAL_ERROR);
        return nullptr;
    }
    auto option = animatorResult->GetAnimatorOption();
    if (!option) {
        NapiThrow(env, "Internal error. Option is null in AnimatorResult.", ERROR_CODE_INTERNAL_ERROR);
        return nullptr;
    }
    ParseAnimatorOption(env, info, option);
    auto animator = animatorResult->GetAnimator();
    if (!animator) {
        NapiThrow(env, "Internal error. Animator is null in AnimatorResult.", ERROR_CODE_INTERNAL_ERROR);
        return nullptr;
    }
    TAG_LOGI(AceLogTag::ACE_ANIMATION, "ohos.animator reset, id:%{public}d", animator->GetId());
    animator->ClearInterpolators();
    animator->ResetIsReverse();
    animatorResult->ApplyOption();
    napi_ref onframeRef = animatorResult->GetOnframeRef();
    if (onframeRef) {
        auto onFrameCallback = [env, onframeRef, id = animator->GetId(),
                                   weakOption = std::weak_ptr<AnimatorOption>(animatorResult->GetAnimatorOption())](
                                   double value) {
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(env, &scope);
            if (scope == nullptr) {
                TAG_LOGW(
                    AceLogTag::ACE_ANIMATION, "ohos.animator call onFrame failed, scope is null, id:%{public}d", id);
                return;
            }
            napi_value ret = nullptr;
            napi_value valueNapi = nullptr;
            napi_value onframe = nullptr;
            auto result = napi_get_reference_value(env, onframeRef, &onframe);
            auto option = weakOption.lock();
            if (!(result == napi_ok && onframe && option)) {
                TAG_LOGW(AceLogTag::ACE_ANIMATION,
                    "ohos.animator call onFrame failed, get reference result:%{public}d, id:%{public}d",
                    result == napi_ok, id);
                napi_close_handle_scope(env, scope);
                return;
            }
            ACE_SCOPED_TRACE(
                "ohos.animator onframe. duration:%d, curve:%s, id:%d", option->duration, option->easing.c_str(), id);
            napi_create_double(env, value, &valueNapi);
            napi_call_function(env, nullptr, onframe, 1, &valueNapi, &ret);
            napi_close_handle_scope(env, scope);
        };
        RefPtr<Animation<double>> animation;
        RefPtr<Motion> motion = ParseOptionToMotion(option);
        if (motion) {
            motion->AddListener(onFrameCallback);
            animatorResult->SetMotion(motion);
        } else {
            auto curve = Framework::CreateCurve(option->easing);
            animation = AceType::MakeRefPtr<CurveAnimation<double>>(option->begin, option->end, curve);
            animation->AddListener(onFrameCallback);
            animator->AddInterpolator(animation);
            animatorResult->SetMotion(nullptr);
        }
    }
    napi_value result;
    napi_get_null(env, &result);
    return result;
}

// since API 9 deprecated
static napi_value JSUpdate(napi_env env, napi_callback_info info)
{
    return JSReset(env, info);
}

static napi_value JSPlay(napi_env env, napi_callback_info info)
{
    FrameTraceAdapter* ft = FrameTraceAdapter::GetInstance();
    if (ft != nullptr) {
        ft->SetFrameTraceLimit();
    }
    auto animatorResult = GetAnimatorResult(env, info);
    if (!animatorResult) {
        TAG_LOGW(AceLogTag::ACE_ANIMATION, "ohos.animator: cannot find animator result when call play");
        return nullptr;
    }
    auto animator = animatorResult->GetAnimator();
    if (!animator) {
        TAG_LOGW(AceLogTag::ACE_ANIMATION, "ohos.animator: no animator is created when call play");
        return nullptr;
    }
    if (!animator->HasScheduler()) {
        auto result = animator->AttachSchedulerOnContainer();
        if (!result) {
            TAG_LOGW(AceLogTag::ACE_ANIMATION,
                "ohos.animator: play failed, animator is not bound to specific context, use uiContext.createAnimator "
                "instead, id:%{public}d",
                animator->GetId());
            return nullptr;
        }
    }
    TAG_LOGI(AceLogTag::ACE_ANIMATION, "ohos.animator play, id:%{public}d, %{public}s",
        animator->GetId(), animatorResult->GetAnimatorOption()->ToString().c_str());
    if (animatorResult->GetMotion()) {
        animator->PlayMotion(animatorResult->GetMotion());
    } else {
        animator->Play();
    }
    animator->PrintVsyncInfoIfNeed();
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

static napi_value JSFinish(napi_env env, napi_callback_info info)
{
    auto animator = GetAnimatorInResult(env, info);
    if (!animator) {
        return nullptr;
    }
    TAG_LOGI(AceLogTag::ACE_ANIMATION, "ohos.animator finish, id:%{public}d", animator->GetId());
    animator->Finish();
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

static napi_value JSPause(napi_env env, napi_callback_info info)
{
    auto animator = GetAnimatorInResult(env, info);
    if (!animator) {
        return nullptr;
    }
    TAG_LOGI(AceLogTag::ACE_ANIMATION, "ohos.animator pause, id:%{public}d", animator->GetId());
    animator->Pause();
    napi_value result;
    napi_get_null(env, &result);
    return result;
}

static napi_value JSCancel(napi_env env, napi_callback_info info)
{
    auto animator = GetAnimatorInResult(env, info);
    if (!animator) {
        return nullptr;
    }
    TAG_LOGI(AceLogTag::ACE_ANIMATION, "ohos.animator cancel, id:%{public}d", animator->GetId());
    animator->Cancel();
    napi_value result;
    napi_get_null(env, &result);
    return result;
}

static napi_value JSReverse(napi_env env, napi_callback_info info)
{
    auto animatorResult = GetAnimatorResult(env, info);
    if (!animatorResult) {
        TAG_LOGW(AceLogTag::ACE_ANIMATION, "ohos.animator: cannot find animator result when call reverse");
        return nullptr;
    }
    if (animatorResult->GetMotion()) {
        TAG_LOGW(AceLogTag::ACE_ANIMATION, "ohos.animator: interpolatingSpringCurve, cannot reverse");
        return nullptr;
    }
    auto animator = animatorResult->GetAnimator();
    if (!animator) {
        TAG_LOGW(AceLogTag::ACE_ANIMATION, "ohos.animator: no animator is created when call reverse");
        return nullptr;
    }
    if (!animator->HasScheduler()) {
        auto result = animator->AttachSchedulerOnContainer();
        if (!result) {
            TAG_LOGW(AceLogTag::ACE_ANIMATION,
                "ohos.animator: reverse failed, animator is not bound to specific context, use "
                "uiContext.createAnimator instead, id:%{public}d",
                animator->GetId());
            return nullptr;
        }
    }
    TAG_LOGI(AceLogTag::ACE_ANIMATION, "ohos.animator reverse, id:%{public}d", animator->GetId());
    animator->Reverse();
    napi_value result;
    napi_get_null(env, &result);
    return result;
}

static bool ParseJsValue(napi_env env, napi_value jsObject, const std::string& name, int32_t& data)
{
    napi_value value = nullptr;
    napi_get_named_property(env, jsObject, name.c_str(), &value);
    napi_valuetype type = napi_undefined;
    napi_typeof(env, value, &type);
    if (type == napi_number) {
        napi_get_value_int32(env, value, &data);
        return true;
    } else {
        return false;
    }
    return true;
}

static napi_value ParseExpectedFrameRateRange(napi_env env, napi_callback_info info, FrameRateRange& frameRateRange)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != 1) {
        NapiThrow(env, "The number of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
        return nullptr;
    }

    napi_value& nativeObj = argv[0];
    if (nativeObj == nullptr) {
        NapiThrow(env, "The nativeObj is nullptr.", ERROR_CODE_PARAM_INVALID);
        return nullptr;
    }

    int32_t minFPS = 0;
    int32_t maxFPS = 0;
    int32_t expectedFPS = 0;
    ParseJsValue(env, nativeObj, "min", minFPS);
    ParseJsValue(env, nativeObj, "max", maxFPS);
    ParseJsValue(env, nativeObj, "expected", expectedFPS);

    frameRateRange.Set(minFPS, maxFPS, expectedFPS);
    if (!frameRateRange.IsValid()) {
        NapiThrow(env, "ExpectedFrameRateRange Error", ERROR_CODE_PARAM_INVALID);
        return nullptr;
    }
    return nullptr;
}

static napi_value JSSetExpectedFrameRateRange(napi_env env, napi_callback_info info)
{
    auto animatorResult = GetAnimatorResult(env, info);
    if (!animatorResult) {
        TAG_LOGW(AceLogTag::ACE_ANIMATION, "ohos.animator: cannot find animator when call SetExpectedFrameRateRange");
        return nullptr;
    }
    auto animator = animatorResult->GetAnimator();
    if (!animator) {
        TAG_LOGW(AceLogTag::ACE_ANIMATION, "ohos.animator: no animator is created when call SetExpectedFrameRateRange");
        return nullptr;
    }
    if (!animator->HasScheduler()) {
        auto result = animator->AttachSchedulerOnContainer();
        if (!result) {
            TAG_LOGW(AceLogTag::ACE_ANIMATION,
                "ohos.animator: SetExpectedFrameRateRange failed because animator is not bound to specific context, "
                "use uiContext.createAnimator instead, id:%{public}d",
                animator->GetId());
            return nullptr;
        }
    }
    TAG_LOGI(AceLogTag::ACE_ANIMATION, "ohos.animator setExpectedFrameRateRange, id:%{public}d", animator->GetId());
    FrameRateRange frameRateRange;
    ParseExpectedFrameRateRange(env, info, frameRateRange);
    animator->SetExpectedFrameRateRange(frameRateRange);
    return nullptr;
}

static napi_value SetOnframe(napi_env env, napi_callback_info info)
{
    AnimatorResult* animatorResult = nullptr;
    size_t argc = 1;
    napi_value thisVar = nullptr;
    napi_value onframe = nullptr;
    napi_get_cb_info(env, info, &argc, &onframe, &thisVar, NULL);
    napi_unwrap(env, thisVar, (void**)&animatorResult);
    if (!animatorResult) {
        return nullptr;
    }
    auto option = animatorResult->GetAnimatorOption();
    if (!option) {
        return nullptr;
    }
    auto animator = animatorResult->GetAnimator();
    if (!animator) {
        return nullptr;
    }
    animator->ClearInterpolators();
    // convert onframe function to reference
    napi_ref onframeRef = animatorResult->GetOnframeRef();
    if (onframeRef) {
        uint32_t count = 0;
        napi_reference_unref(env, onframeRef, &count);
    }
    napi_create_reference(env, onframe, 1, &onframeRef);
    animatorResult->SetOnframeRef(onframeRef);
    auto onFrameCallback = [env, onframeRef, id = animator->GetId(),
                               weakOption = std::weak_ptr<AnimatorOption>(animatorResult->GetAnimatorOption())](
                               double value) {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        if (scope == nullptr) {
            TAG_LOGW(AceLogTag::ACE_ANIMATION, "ohos.animator call onFrame failed, scope is null, id:%{public}d", id);
            return;
        }
        napi_value ret = nullptr;
        napi_value valueNapi = nullptr;
        napi_value onframe = nullptr;
        auto result = napi_get_reference_value(env, onframeRef, &onframe);
        auto option = weakOption.lock();
        if (!(result == napi_ok && onframe && option)) {
            TAG_LOGW(AceLogTag::ACE_ANIMATION,
                "ohos.animator call onFrame failed, get reference result:%{public}d, id:%{public}d", result == napi_ok,
                id);
            napi_close_handle_scope(env, scope);
            return;
        }
        ACE_SCOPED_TRACE(
            "ohos.animator onframe. duration:%d, curve:%s, id:%d", option->duration, option->easing.c_str(), id);
        napi_create_double(env, value, &valueNapi);
        napi_call_function(env, nullptr, onframe, 1, &valueNapi, &ret);
        napi_close_handle_scope(env, scope);
    };
    RefPtr<Animation<double>> animation;
    RefPtr<Motion> motion = ParseOptionToMotion(option);
    if (motion) {
        motion->AddListener(onFrameCallback);
        animatorResult->SetMotion(motion);
    } else {
        auto curve = Framework::CreateCurve(option->easing);
        animation = AceType::MakeRefPtr<CurveAnimation<double>>(option->begin, option->end, curve);
        animation->AddListener(onFrameCallback);
        animator->AddInterpolator(animation);
        animatorResult->SetMotion(nullptr);
    }
    if (!animator->HasScheduler()) {
        animator->AttachSchedulerOnContainer();
    }
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

static napi_value SetOnFrame(napi_env env, napi_callback_info info)
{
    return SetOnframe(env, info);
}

static napi_value SetOnfinish(napi_env env, napi_callback_info info)
{
    AnimatorResult* animatorResult = nullptr;
    size_t argc = 1;
    napi_value thisVar = nullptr;
    napi_value onfinish = nullptr;
    napi_get_cb_info(env, info, &argc, &onfinish, &thisVar, NULL);
    napi_unwrap(env, thisVar, (void**)&animatorResult);
    if (!animatorResult) {
        return nullptr;
    }
    auto option = animatorResult->GetAnimatorOption();
    if (!option) {
        return nullptr;
    }
    auto animator = animatorResult->GetAnimator();
    if (!animator) {
        return nullptr;
    }
    // convert onfinish function to reference
    napi_ref onfinishRef = animatorResult->GetOnfinishRef();
    if (onfinishRef) {
        uint32_t count = 0;
        napi_reference_unref(env, onfinishRef, &count);
    }
    napi_create_reference(env, onfinish, 1, &onfinishRef);
    animatorResult->SetOnfinishRef(onfinishRef);
    animator->ClearStopListeners();
    TAG_LOGI(AceLogTag::ACE_ANIMATION, "ohos.animator set finish callback, id:%{public}d", animator->GetId());
    animator->AddStopListener([env, onfinishRef, id = animator->GetId()] {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        if (scope == nullptr) {
            TAG_LOGW(AceLogTag::ACE_ANIMATION,
                "ohos.animator call finish callback failed, scope is null, id:%{public}d", id);
            return;
        }
        napi_value ret = nullptr;
        napi_value onfinish = nullptr;
        auto result = napi_get_reference_value(env, onfinishRef, &onfinish);
        if (result != napi_ok || onfinish == nullptr) {
            napi_close_handle_scope(env, scope);
            TAG_LOGW(AceLogTag::ACE_ANIMATION,
                "ohos.animator call finish callback failed, get onFinish failed, id:%{public}d", id);
            return;
        }
        ACE_SCOPED_TRACE("ohos.animator finishCallback, id:%d", id);
        result = napi_call_function(env, NULL, onfinish, 0, NULL, &ret);
        TAG_LOGI(AceLogTag::ACE_ANIMATION, "ohos.animator call finish callback done, id:%{public}d, succeed:%{public}d",
            id, result == napi_ok);
        napi_close_handle_scope(env, scope);
    });
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

static napi_value SetOnFinish(napi_env env, napi_callback_info info)
{
    return SetOnfinish(env, info);
}

static napi_value SetOncancel(napi_env env, napi_callback_info info)
{
    AnimatorResult* animatorResult = nullptr;
    size_t argc = 1;
    napi_value thisVar = nullptr;
    napi_value oncancel = nullptr;
    napi_get_cb_info(env, info, &argc, &oncancel, &thisVar, NULL);
    napi_unwrap(env, thisVar, (void**)&animatorResult);
    if (!animatorResult) {
        return nullptr;
    }
    auto option = animatorResult->GetAnimatorOption();
    if (!option) {
        return nullptr;
    }
    auto animator = animatorResult->GetAnimator();
    if (!animator) {
        return nullptr;
    }
    // convert oncancel function to reference
    napi_ref oncancelRef = animatorResult->GetOncancelRef();
    if (oncancelRef) {
        uint32_t count = 0;
        napi_reference_unref(env, oncancelRef, &count);
    }
    napi_create_reference(env, oncancel, 1, &oncancelRef);
    animatorResult->SetOncancelRef(oncancelRef);
    animator->ClearIdleListeners();
    animator->AddIdleListener([env, oncancelRef] {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        if (scope == nullptr) {
            return;
        }
        napi_value ret = nullptr;
        napi_value oncancel = nullptr;
        auto result = napi_get_reference_value(env, oncancelRef, &oncancel);
        if (result != napi_ok || oncancel == nullptr) {
            napi_close_handle_scope(env, scope);
            return;
        }
        napi_call_function(env, NULL, oncancel, 0, NULL, &ret);
        napi_close_handle_scope(env, scope);
    });
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

static napi_value SetOnCancel(napi_env env, napi_callback_info info)
{
    return SetOncancel(env, info);
}

static napi_value SetOnrepeat(napi_env env, napi_callback_info info)
{
    AnimatorResult* animatorResult = nullptr;
    size_t argc = 1;
    napi_value thisVar = nullptr;
    napi_value onrepeat = nullptr;
    napi_get_cb_info(env, info, &argc, &onrepeat, &thisVar, NULL);
    napi_unwrap(env, thisVar, (void**)&animatorResult);
    if (!animatorResult) {
        return nullptr;
    }
    auto option = animatorResult->GetAnimatorOption();
    if (!option) {
        return nullptr;
    }
    auto animator = animatorResult->GetAnimator();
    if (!animator) {
        return nullptr;
    }
    // convert onrepeat function to reference
    napi_ref onrepeatRef = animatorResult->GetOnrepeatRef();
    if (onrepeatRef) {
        uint32_t count = 0;
        napi_reference_unref(env, onrepeatRef, &count);
    }
    napi_create_reference(env, onrepeat, 1, &onrepeatRef);
    animatorResult->SetOnrepeatRef(onrepeatRef);
    animator->ClearRepeatListeners();
    animator->AddRepeatListener([env, onrepeatRef] {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        if (scope == nullptr) {
            return;
        }
        napi_value ret = nullptr;
        napi_value onrepeat = nullptr;
        auto result = napi_get_reference_value(env, onrepeatRef, &onrepeat);
        if (result != napi_ok || onrepeat == nullptr) {
            napi_close_handle_scope(env, scope);
            return;
        }
        napi_call_function(env, NULL, onrepeat, 0, NULL, &ret);
        napi_close_handle_scope(env, scope);
    });
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

static napi_value SetOnRepeat(napi_env env, napi_callback_info info)
{
    return SetOnrepeat(env, info);
}

static napi_value JSCreate(napi_env env, napi_callback_info info)
{
    auto option = std::make_shared<AnimatorOption>();
    ParseAnimatorOption(env, info, option);
    auto animator = CREATE_ANIMATOR("ohos.animator");
    animator->AttachSchedulerOnContainer();
    AnimatorResult* animatorResult = new AnimatorResult(animator, option);
    napi_value jsAnimator = nullptr;
    napi_create_object(env, &jsAnimator);
    napi_wrap(
        env, jsAnimator, animatorResult,
        [](napi_env env, void* data, void* hint) {
            AnimatorResult* animatorResult = (AnimatorResult*)data;
            // release four references(onFunc) before releasing animatorResult
            animatorResult->Destroy(env);
            delete animatorResult;
        },
        nullptr, nullptr);
    napi_property_descriptor resultFuncs[] = {
        DECLARE_NAPI_FUNCTION("update", JSUpdate),
        DECLARE_NAPI_FUNCTION("reset", JSReset),
        DECLARE_NAPI_FUNCTION("play", JSPlay),
        DECLARE_NAPI_FUNCTION("finish", JSFinish),
        DECLARE_NAPI_FUNCTION("pause", JSPause),
        DECLARE_NAPI_FUNCTION("cancel", JSCancel),
        DECLARE_NAPI_FUNCTION("reverse", JSReverse),
        DECLARE_NAPI_FUNCTION("setExpectedFrameRateRange", JSSetExpectedFrameRateRange),
        DECLARE_NAPI_SETTER("onframe", SetOnframe),
        DECLARE_NAPI_SETTER("onfinish", SetOnfinish),
        DECLARE_NAPI_SETTER("oncancel", SetOncancel),
        DECLARE_NAPI_SETTER("onrepeat", SetOnrepeat),
        DECLARE_NAPI_SETTER("onFrame", SetOnFrame),
        DECLARE_NAPI_SETTER("onFinish", SetOnFinish),
        DECLARE_NAPI_SETTER("onCancel", SetOnCancel),
        DECLARE_NAPI_SETTER("onRepeat", SetOnRepeat),
    };

    NAPI_CALL(env, napi_define_properties(env, jsAnimator, sizeof(resultFuncs) / sizeof(resultFuncs[0]), resultFuncs));
    return jsAnimator;
}

// since API 9 deprecated
static napi_value JSCreateAnimator(napi_env env, napi_callback_info info)
{
    return JSCreate(env, info);
}

static napi_value AnimatorExport(napi_env env, napi_value exports)
{
    napi_property_descriptor animatorDesc[] = {
        DECLARE_NAPI_FUNCTION("create", JSCreate),
        DECLARE_NAPI_FUNCTION("createAnimator", JSCreateAnimator),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(animatorDesc) / sizeof(animatorDesc[0]), animatorDesc));
    return exports;
}

static napi_module animatorModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = AnimatorExport,
    .nm_modname = "animator",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void AnimatorRegister()
{
    napi_module_register(&animatorModule);
}

void AnimatorResult::ApplyOption()
{
    CHECK_NULL_VOID(animator_);
    CHECK_NULL_VOID(option_);
    if (motion_) {
        // duration not works. Iteration can only be 1. Direction can only be normal.
        animator_->SetIteration(1);
        animator_->SetAnimationDirection(AnimationDirection::NORMAL);
    } else {
        animator_->SetDuration(option_->duration);
        animator_->SetIteration(option_->iterations);
        animator_->SetAnimationDirection(StringToAnimationDirection(option_->direction));
    }
    animator_->SetStartDelay(option_->delay);
    // FillMode not works for motion in animator implementation.
    animator_->SetFillMode(StringToFillMode(option_->fill));
}

void AnimatorResult::Destroy(napi_env env)
{
    if (animator_) {
        if (!animator_->IsStopped()) {
            animator_->Stop();
            TAG_LOGI(AceLogTag::ACE_ANIMATION, "ohos.animator force stopping done when destroying, id:%{public}d",
                animator_->GetId());
        }
    }
    if (onframe_ != nullptr) {
        napi_delete_reference(env, onframe_);
    }
    if (onfinish_ != nullptr) {
        napi_delete_reference(env, onfinish_);
    }
    if (oncancel_ != nullptr) {
        napi_delete_reference(env, oncancel_);
    }
    if (onrepeat_ != nullptr) {
        napi_delete_reference(env, onrepeat_);
    }
}

} // namespace OHOS::Ace::Napi
