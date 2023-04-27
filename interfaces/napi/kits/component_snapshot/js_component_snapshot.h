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

#ifndef OHOS_NAPI_ACE_COMPONENT_SNAPSHOT_H
#define OHOS_NAPI_ACE_COMPONENT_SNAPSHOT_H

#include <functional>
#include <memory>

#include "js_native_api.h"
#include "js_native_api_types.h"

namespace OHOS::Media {
class PixelMap;
} // namespace OHOS::Media

namespace OHOS::Ace::Napi {
class JsComponentSnapshot {
public:
    JsComponentSnapshot(napi_env env, napi_callback_info info);

    bool CheckArgs(napi_valuetype firstArgType);

    std::function<void(std::shared_ptr<Media::PixelMap>, int32_t)> CreateCallback(napi_value* result);

    napi_value GetArgv(int32_t idx);

    static constexpr int32_t ARGC_MAX = 2;

private:
    napi_env env_ = nullptr;
    napi_value argv_[ARGC_MAX] { nullptr };
    size_t argc_ = ARGC_MAX;
};
} // namespace OHOS::Ace::Napi

#endif