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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_DECLARATION_TEXT_MENU_TEXT_MENU_DECLARATION_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_DECLARATION_TEXT_MENU_TEXT_MENU_DECLARATION_H

#include <optional>
#include <string>

namespace OHOS::Ace::NG {
struct MenuOptionsParam {
    std::optional<std::string> content;
    std::optional<std::string> icon;
    std::function<void(const std::string&)> action;
    std::function<void(int32_t, int32_t)> actionRange;
    std::string ToString() const
    {
        std::string result;
        result.append("content: ");
        result.append(content.value_or("na"));
        result.append(", icon: ");
        result.append(icon.value_or("na"));
        return result;
    }
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_DECLARATION_NAVIGATION_NAVIGATION_DECLARATION_H