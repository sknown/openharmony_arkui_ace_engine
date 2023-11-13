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

#include "core/components_ng/pattern/ui_extension/ui_extension_manager.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_pattern.h"
namespace OHOS::Ace::NG {
namespace {
constexpr int32_t UI_EXTENSION_OFFSET_MIN = 1000000;
};

void UIExtensionManager::RegisterUIExtensionInFocus(const WeakPtr<UIExtensionPattern>& uiExtensionFocused)
{
    uiExtensionFocused_ = uiExtensionFocused;
}

bool UIExtensionManager::OnBackPressed()
{
    auto uiExtensionFocused = uiExtensionFocused_.Upgrade();
    CHECK_NULL_RETURN(uiExtensionFocused, false);
    return uiExtensionFocused->OnBackPressed();
}

bool UIExtensionManager::IsWrapExtensionAbilityId(int32_t elementId)
{
    return elementId > UI_EXTENSION_OFFSET_MIN;
}

std::pair<int32_t, int32_t> UIExtensionManager::UnWrapExtensionAbilityId(
    int32_t extensionOffset, int32_t elementId)
{
    if (extensionOffset == 0) {
        return std::pair<int32_t, int32_t>(0, 0);
    }
    int32_t index = elementId / extensionOffset;
    int32_t abilityId = elementId % extensionOffset;
    return std::pair<int32_t, int32_t>(index, abilityId);
}

const RefPtr<FrameNode> UIExtensionManager::GetFocusUiExtensionNode()
{
    auto uiExtensionFocused = uiExtensionFocused_.Upgrade();
    CHECK_NULL_RETURN(uiExtensionFocused, nullptr);
    return uiExtensionFocused->GetUiExtensionNode();
}
} // namespace OHOS::Ace::NG
