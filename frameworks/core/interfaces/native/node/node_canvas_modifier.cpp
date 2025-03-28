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
#include "core/interfaces/native/node/node_canvas_modifier.h"

#include "base/log/log_wrapper.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/custom_paint/canvas_model_ng.h"

namespace OHOS::Ace::NG::NodeModifier {

void SetCanvasOnReady(ArkUINodeHandle node, void* extraParam)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto onChange = []() {
        ArkUINodeEvent event;
        event.kind = COMPONENT_ASYNC_EVENT;
        event.componentAsyncEvent.subKind = ON_CANVAS_READY;
        SendArkUIAsyncEvent(&event);
    };
    CanvasModelNG::SetOnReady(frameNode, std::move(onChange));
}
} // namespace OHOS::Ace::NG::NodeModifier