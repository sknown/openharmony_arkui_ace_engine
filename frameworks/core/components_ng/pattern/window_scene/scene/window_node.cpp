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

#include "core/components_ng/pattern/window_scene/scene/window_node.h"

#include "core/components_ng/pattern/window_scene/scene/window_pattern.h"

namespace OHOS::Ace::NG {
RefPtr<WindowNode> WindowNode::GetOrCreateWindowNode(
    const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator)
{
    auto windowNode = ElementRegister::GetInstance()->GetSpecificItemById<WindowNode>(nodeId);
    if (windowNode) {
        if (windowNode->GetTag() == tag) {
            return windowNode;
        }
        ElementRegister::GetInstance()->RemoveItemSilently(nodeId);
        auto parent = windowNode->GetParent();
        if (parent) {
            parent->RemoveChild(windowNode);
        }
    }

    auto pattern = patternCreator ? patternCreator() : AceType::MakeRefPtr<Pattern>();
    windowNode = AceType::MakeRefPtr<WindowNode>(tag, nodeId, pattern, false);
    windowNode->InitializePatternAndContext();
    ElementRegister::GetInstance()->AddUINode(windowNode);
    return windowNode;
}
} // namespace OHOS::Ace::NG
