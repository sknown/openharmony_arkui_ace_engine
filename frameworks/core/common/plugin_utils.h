/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_PLUGIN_UTILS_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_PLUGIN_UTILS_H

#include <string>

namespace OHOS::Ace {
class PluginUtils {
public:
    PluginUtils() = default;
    virtual ~PluginUtils() = default;

    virtual int32_t StartAbility(
        const std::string& bundleName, const std::string& abilityName, const std::string& params) = 0;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_PLUGIN_UTILS_H