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
#include "core/interfaces/native/node/nav_destination_modifier.h"

#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/pattern/navrouter/navdestination_model_ng.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace::NG {
void SetHideTitleBar(ArkUINodeHandle node, ArkUI_Bool hideTitle)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavDestinationModelNG::SetHideTitleBar(frameNode, hideTitle);
}

void ResetHideTitleBar(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavDestinationModelNG::SetHideTitleBar(frameNode, false);
}

void SetNavDestinationMode(ArkUINodeHandle node, int32_t value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavDestinationModelNG::SetNavDestinationMode(frameNode, static_cast<NG::NavDestinationMode>(value));
}

void ResetNavDestinationMode(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavDestinationModelNG::SetNavDestinationMode(frameNode, NG::NavDestinationMode::STANDARD);
}
namespace NodeModifier {
const ArkUINavDestinationModifier* GetNavDestinationModifier()
{
    static const ArkUINavDestinationModifier modifier = {
        SetHideTitleBar,
        ResetHideTitleBar,
        SetNavDestinationMode,
        ResetNavDestinationMode,
    };

    return &modifier;
}
}
} // namespace OHOS::Ace::NG
