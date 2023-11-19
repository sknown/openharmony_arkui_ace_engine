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

#include "interfaces/napi/kits/utils/napi_utils.h"
#include "js_native_api.h"
#include "js_native_api_types.h"

#include "core/components_ng/base/observer.h"

namespace OHOS::Ace::Napi {
#define GET_PARAMS(env, info, max) \
    size_t argc = max;             \
    napi_value argv[max] = { 0 };  \
    napi_value thisVar = nullptr;  \
    void* data;                    \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)

bool MatchValueType(napi_env env, napi_value value, napi_valuetype targetType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    return valueType == targetType;
}

static void ParseNavigation(napi_env env, napi_value navigation, std::string& navigationStr)
{
    if (navigation != nullptr) {
        size_t navLen = 0;
        napi_get_value_string_utf8(env, navigation, nullptr, 0, &navLen);
        std::unique_ptr<char[]> nav = std::make_unique<char[]>(navLen + 1);
        napi_get_value_string_utf8(env, navigation, nav.get(), navLen + 1, &navLen);
        navigationStr = nav.get();
    }
}

napi_value ObserverOn(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 3);
    NAPI_ASSERT(env, (argc >= 2 && thisVar != nullptr), "Invalid arguments");

    if (argc == 2 && MatchValueType(env, argv[0], napi_string) && MatchValueType(env, argv[1], napi_function)) {
        auto listener = std::make_shared<NG::UIObserverListener>(env, argv[1]);
        NG::UIObserver::RegisterNavigationCallback(listener);
    }

    if (argc == 3 && MatchValueType(env, argv[0], napi_string) && MatchValueType(env, argv[1], napi_object) &&
        MatchValueType(env, argv[2], napi_function)) {
        napi_value navigationId = nullptr;
        napi_get_named_property(env, argv[1], "navigationId", &navigationId);
        if (MatchValueType(env, navigationId, napi_string)) {
            std::string id;
            ParseNavigation(env, navigationId, id);
            auto listener = std::make_shared<NG::UIObserverListener>(env, argv[2]);
            NG::UIObserver::RegisterNavigationCallback(id, listener);
        }
    }

    napi_value result = nullptr;
    return result;
}

napi_value ObserverOff(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 3);
    NAPI_ASSERT(env, (argc >= 2 && thisVar != nullptr), "Invalid arguments");

    if (argc == 2 && MatchValueType(env, argv[0], napi_string) && MatchValueType(env, argv[1], napi_function)) {
        auto listener = std::make_shared<NG::UIObserverListener>(env, argv[1]);
        NG::UIObserver::UnRegisterNavigationCallback(listener);
    }

    if (argc == 3 && MatchValueType(env, argv[0], napi_string) && MatchValueType(env, argv[1], napi_object) &&
        MatchValueType(env, argv[2], napi_function)) {
        napi_value navigationId = nullptr;
        napi_get_named_property(env, argv[1], "navigationId", &navigationId);
        if (MatchValueType(env, navigationId, napi_string)) {
            std::string id;
            ParseNavigation(env, navigationId, id);
            auto listener = std::make_shared<NG::UIObserverListener>(env, argv[2]);
            NG::UIObserver::UnRegisterNavigationCallback(id, listener);
        }
    }

    napi_value result = nullptr;
    return result;
}

static napi_value UIObserverExport(napi_env env, napi_value exports)
{
    napi_property_descriptor uiObserverDesc[] = {
        DECLARE_NAPI_FUNCTION("on", ObserverOn),
        DECLARE_NAPI_FUNCTION("off", ObserverOff),
    };
    NAPI_CALL(
        env, napi_define_properties(env, exports, sizeof(uiObserverDesc) / sizeof(uiObserverDesc[0]), uiObserverDesc));
    return exports;
}

static napi_module uiObserverModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = UIObserverExport,
    .nm_modname = "arkui.observer",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void ObserverRegister()
{
    napi_module_register(&uiObserverModule);
}
} // namespace OHOS::Ace::Napi
