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
#include "core/interfaces/native/node/path_modifier.h"

#include "core/pipeline/base/element_register.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/shape/path_model_ng.h"
#include "core/components/common/layout/constants.h"


namespace OHOS::Ace::NG {

void SetPathCommands(ArkUINodeHandle node, ArkUI_CharPtr commands)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PathModelNG::SetCommands(frameNode, std::string(commands));
}

void ResetPathCommands(ArkUINodeHandle node)
{
    std::string outCommands = "";
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PathModelNG::SetCommands(frameNode, outCommands);
}

namespace NodeModifier {
const ArkUIPathModifier* GetPathModifier()
{
    static const ArkUIPathModifier modifier = {SetPathCommands, ResetPathCommands};

    return &modifier;
}
}
}