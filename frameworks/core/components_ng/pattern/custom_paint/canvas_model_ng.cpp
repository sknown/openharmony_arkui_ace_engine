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

#include "core/components_ng/pattern/custom_paint/canvas_model_ng.h"

#include <mutex>

#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/components_ng/pattern/custom_paint/canvas_paint_method.h"
#include "core/components_ng/pattern/custom_paint/custom_paint_pattern.h"

namespace OHOS::Ace {
CanvasModel* CanvasModel::GetInstanceNG()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
            instance_.reset(new NG::CanvasModelNG());
        }
    }
    return instance_.get();
}
} // namespace OHOS::Ace

namespace OHOS::Ace::NG {
RefPtr<AceType> CanvasModelNG::Create()
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::CANVAS_ETS_TAG, nodeId);
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::CANVAS_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<CustomPaintPattern>(); });
    stack->Push(frameNode);
    return frameNode->GetPattern<CustomPaintPattern>();
}

RefPtr<AceType> CanvasModelNG::GetTaskPool(RefPtr<AceType>& pattern)
{
    return pattern;
}

void CanvasModelNG::SetOnReady(std::function<void()>&& onReady)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<CustomPaintEventHub>();
    CHECK_NULL_VOID(eventHub);

    auto func = onReady;
    auto onReadyEvent = [func]() { func(); };
    eventHub->SetOnReady(std::move(onReadyEvent));
}

void CanvasModelNG::EnableAnalyzer(bool enable)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<CustomPaintPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->EnableAnalyzer(enable);
}

void CanvasModelNG::SetOnReady(FrameNode* frameNode, std::function<void()>&& onReady)
{
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<CustomPaintEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto func = onReady;
    auto onReadyEvent = [func]() { func(); };
    eventHub->SetOnReady(std::move(onReadyEvent));
}

RefPtr<AceType> CanvasModelNG::GetCanvasPattern(FrameNode* node)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    RefPtr<AceType> pattern = frameNode->GetPattern<CustomPaintPattern>();
    return CanvasModel::GetInstanceNG()->GetTaskPool(pattern);
}

RefPtr<FrameNode> CanvasModelNG::CreateFrameNode(int32_t nodeId)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::CANVAS_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<CustomPaintPattern>(); });
    return frameNode;
}
} // namespace OHOS::Ace::NG
