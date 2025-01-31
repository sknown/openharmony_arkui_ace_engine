/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "movingphoto_napi.h"

#include "ext_napi_utils.h"
#include "movingphoto_controller.h"
#include "movingphoto_model_ng.h"

#include "base/utils/utils.h"
#include "core/pipeline/pipeline_base.h"
#include "base/resource/data_provider_manager.h"

extern const char _binary_multimedia_movingphotoview_js_start[];
extern const char _binary_multimedia_movingphotoview_abc_start[];
#if !defined(IOS_PLATFORM)
extern const char _binary_multimedia_movingphotoview_js_end[];
extern const char _binary_multimedia_movingphotoview_abc_end[];
#else
extern const char* _binary_multimedia_movingphotoview_js_end;
extern const char* _binary_multimedia_movingphotoview_abc_end;
#endif

namespace OHOS::Ace {
namespace {
static constexpr const size_t MAX_ARG_NUM = 10;
static constexpr const int32_t ARG_NUM_1 = 1;
} // namespace

std::unique_ptr<NG::MovingPhotoModelNG> NG::MovingPhotoModelNG::instance_ = nullptr;
std::mutex NG::MovingPhotoModelNG::mutex_;

NG::MovingPhotoModelNG* NG::MovingPhotoModelNG::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
            instance_.reset(new NG::MovingPhotoModelNG());
        }
    }
    return instance_.get();
}

napi_value JsCreate(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARG_NUM;
    napi_value argv[MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ARG_NUM_1, "Wrong number of arguments");

    if (!ExtNapiUtils::CheckTypeForNapiValue(env, argv[0], napi_object)) {
        return ExtNapiUtils::CreateNull(env);
    }

    napi_value jsController = nullptr;
    NG::MovingPhotoController* controller = nullptr;
    napi_get_named_property(env, argv[0], "controller", &jsController);
    if (ExtNapiUtils::CheckTypeForNapiValue(env, jsController, napi_object)) {
        napi_unwrap(env, jsController, (void**)&controller);
    }
    NG::MovingPhotoModelNG::GetInstance()->Create(Referenced::Claim(controller));

    napi_value jsData = nullptr;
    napi_get_named_property(env, argv[0], "movingPhoto", &jsData);
    if (!ExtNapiUtils::CheckTypeForNapiValue(env, jsData, napi_object)) {
        return ExtNapiUtils::CreateNull(env);
    }

    napi_value getUri = nullptr;
    napi_get_named_property(env, jsData, "getUri", &getUri);
    if (!ExtNapiUtils::CheckTypeForNapiValue(env, getUri, napi_function)) {
        return ExtNapiUtils::CreateNull(env);
    }
    napi_value imageUri;
    napi_call_function(env, jsData, getUri, 0, nullptr, &imageUri);
    std::string imageUriStr = ExtNapiUtils::GetStringFromValueUtf8(env, imageUri);
    NG::MovingPhotoModelNG::GetInstance()->SetImageSrc(imageUriStr);

    return ExtNapiUtils::CreateNull(env);
}

napi_value JsMuted(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARG_NUM;
    napi_value argv[MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ARG_NUM_1, "Wrong number of arguments");

    bool muted = false;
    if (ExtNapiUtils::CheckTypeForNapiValue(env, argv[0], napi_boolean)) {
        muted = ExtNapiUtils::GetBool(env, argv[0]);
    }
    NG::MovingPhotoModelNG::GetInstance()->SetMuted(muted);

    return ExtNapiUtils::CreateNull(env);
}

napi_value JsObjectFit(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARG_NUM;
    napi_value argv[MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ARG_NUM_1, "Wrong number of arguments");

    auto objectFit = ImageFit::COVER;
    if (ExtNapiUtils::CheckTypeForNapiValue(env, argv[0], napi_number)) {
        objectFit = static_cast<ImageFit>(ExtNapiUtils::GetCInt32(env, argv[0]));
    }
    NG::MovingPhotoModelNG::GetInstance()->SetObjectFit(objectFit);

    return ExtNapiUtils::CreateNull(env);
}

napi_value JsOnStart(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARG_NUM;
    napi_value thisVal = nullptr;
    napi_value argv[MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
    if (!ExtNapiUtils::CheckTypeForNapiValue(env, argv[0], napi_function)) {
        return ExtNapiUtils::CreateNull(env);
    }
    auto asyncEvent = std::make_shared<NapiAsyncEvent>(env, argv[0]);
    auto onStart = [asyncEvent]() {
        asyncEvent->Call(0, nullptr);
    };
    NG::MovingPhotoModelNG::GetInstance()->SetOnStart(std::move(onStart));
    return ExtNapiUtils::CreateNull(env);
}

napi_value JsOnStop(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARG_NUM;
    napi_value thisVal = nullptr;
    napi_value argv[MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
    if (!ExtNapiUtils::CheckTypeForNapiValue(env, argv[0], napi_function)) {
        return ExtNapiUtils::CreateNull(env);
    }
    auto asyncEvent = std::make_shared<NapiAsyncEvent>(env, argv[0]);
    auto onStop = [asyncEvent]() {
        asyncEvent->Call(0, nullptr);
    };
    NG::MovingPhotoModelNG::GetInstance()->SetOnStop(std::move(onStop));
    return ExtNapiUtils::CreateNull(env);
}

napi_value JsOnPause(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARG_NUM;
    napi_value thisVal = nullptr;
    napi_value argv[MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
    if (!ExtNapiUtils::CheckTypeForNapiValue(env, argv[0], napi_function)) {
        return ExtNapiUtils::CreateNull(env);
    }
    auto asyncEvent = std::make_shared<NapiAsyncEvent>(env, argv[0]);
    auto onPause = [asyncEvent]() {
        asyncEvent->Call(0, nullptr);
    };
    NG::MovingPhotoModelNG::GetInstance()->SetOnPause(std::move(onPause));
    return ExtNapiUtils::CreateNull(env);
}

napi_value JsOnFinish(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARG_NUM;
    napi_value thisVal = nullptr;
    napi_value argv[MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
    if (!ExtNapiUtils::CheckTypeForNapiValue(env, argv[0], napi_function)) {
        return ExtNapiUtils::CreateNull(env);
    }
    auto asyncEvent = std::make_shared<NapiAsyncEvent>(env, argv[0]);
    auto onFinish = [asyncEvent]() {
        asyncEvent->Call(0, nullptr);
    };
    NG::MovingPhotoModelNG::GetInstance()->SetOnFinish(std::move(onFinish));
    return ExtNapiUtils::CreateNull(env);
}

napi_value JsOnError(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARG_NUM;
    napi_value thisVal = nullptr;
    napi_value argv[MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
    if (!ExtNapiUtils::CheckTypeForNapiValue(env, argv[0], napi_function)) {
        return ExtNapiUtils::CreateNull(env);
    }
    auto asyncEvent = std::make_shared<NapiAsyncEvent>(env, argv[0]);
    auto onError = [asyncEvent]() {
        asyncEvent->Call(0, nullptr);
    };
    NG::MovingPhotoModelNG::GetInstance()->SetOnError(std::move(onError));
    return ExtNapiUtils::CreateNull(env);
}

napi_value InitView(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("create", JsCreate),
        DECLARE_NAPI_FUNCTION("muted", JsMuted),
        DECLARE_NAPI_FUNCTION("objectFit", JsObjectFit),
        DECLARE_NAPI_FUNCTION("onStart", JsOnStart),
        DECLARE_NAPI_FUNCTION("onStop", JsOnStop),
        DECLARE_NAPI_FUNCTION("onPause", JsOnPause),
        DECLARE_NAPI_FUNCTION("onFinish", JsOnFinish),
        DECLARE_NAPI_FUNCTION("onError", JsOnError),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value StartPlayback(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, NULL));
    NG::MovingPhotoController* controller = nullptr;
    napi_unwrap(env, thisVar, (void**)&controller);
    if (controller == nullptr) {
        return ExtNapiUtils::CreateNull(env);
    }
    controller->StartPlayback();
    return ExtNapiUtils::CreateNull(env);
}

napi_value StopPlayback(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, NULL));
    NG::MovingPhotoController* controller = nullptr;
    napi_unwrap(env, thisVar, (void**)&controller);
    if (controller == nullptr) {
        return ExtNapiUtils::CreateNull(env);
    }
    controller->StopPlayback();
    return ExtNapiUtils::CreateNull(env);
}

napi_value MovingPhotoControllerConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    auto controller = AceType::MakeRefPtr<NG::MovingPhotoController>();
    if (controller == nullptr) {
        return ExtNapiUtils::CreateNull(env);
    }
    controller->IncRefCount();
    napi_wrap(
        env, thisVar, AceType::RawPtr(controller),
        [](napi_env env, void* data, void* hint) {
            auto* controller = reinterpret_cast<NG::MovingPhotoController*>(data);
            controller->DecRefCount();
        },
        nullptr, nullptr);
    return thisVar;
}

napi_value InitController(napi_env env, napi_value exports)
{
    napi_value movingphotoControllerClass = nullptr;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("startPlayback", StartPlayback),
        DECLARE_NAPI_FUNCTION("stopPlayback", StopPlayback),
    };
    NAPI_CALL(env, napi_define_class(env, "MovingPhotoViewController", NAPI_AUTO_LENGTH,
        MovingPhotoControllerConstructor, nullptr, sizeof(properties) / sizeof(*properties), properties,
        &movingphotoControllerClass));
    NAPI_CALL(env, napi_set_named_property(env, exports, "MovingPhotoViewController", movingphotoControllerClass));
    return exports;
}

napi_value ExportMovingPhoto(napi_env env, napi_value exports)
{
    InitView(env, exports);
    InitController(env, exports);
    return exports;
}

} // namespace OHOS::Ace

extern "C" __attribute__((visibility("default")))
void NAPI_multimedia_movingphotoview_GetJSCode(const char** buf, int* bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_multimedia_movingphotoview_js_start;
    }

    if (bufLen != nullptr) {
        *bufLen = _binary_multimedia_movingphotoview_js_end - _binary_multimedia_movingphotoview_js_start;
    }
}

// multimedia_movingphotoview JS register
extern "C" __attribute__((visibility("default")))
void NAPI_multimedia_movingphotoview_GetABCCode(const char** buf, int* buflen)
{
    if (buf != nullptr) {
        *buf = _binary_multimedia_movingphotoview_abc_start;
    }
    if (buflen != nullptr) {
        *buflen = _binary_multimedia_movingphotoview_abc_end - _binary_multimedia_movingphotoview_abc_start;
    }
}

static napi_module movingphotoModule  = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = OHOS::Ace::ExportMovingPhoto,
    .nm_modname = "multimedia.movingphotoview",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterModuleMovingPhoto()
{
    napi_module_register(&movingphotoModule);
}
