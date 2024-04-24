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

#include "core/components_ng/manager/drag_drop/drag_drop_func_wrapper.h"
#include "core/components_ng/pattern/image/image_pattern.h"

namespace OHOS::Ace::NG {

constexpr float DEFAULT_OPACITY = 0.95f;

void DragDropFuncWrapper::SetDraggingPointerAndPressedState(int32_t currentPointerId, int32_t containerId)
{
    auto pipelineContext = PipelineContext::GetContextByContainerId(containerId);
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetDragDropManager();
    CHECK_NULL_VOID(manager);
    manager->SetDraggingPointer(currentPointerId);
    manager->SetDraggingPressedState(true);
}

void DragDropFuncWrapper::DecideWhetherToStopDragging(
    const PointerEvent& pointerEvent, const std::string& extraParams, int32_t currentPointerId, int32_t containerId)
{
    auto pipelineContext = PipelineContext::GetContextByContainerId(containerId);
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetDragDropManager();
    CHECK_NULL_VOID(manager);
    if (!manager->IsDraggingPressed(currentPointerId)) {
        manager->OnDragEnd(pointerEvent, extraParams);
    }
}


void DragDropFuncWrapper::UpdateDragPreviewOptionsFromModifier(
    std::function<void(WeakPtr<FrameNode>)> applyOnNodeSync, DragPreviewOption& option)
{
    // create one temporary frame node for receiving the value from the modifier
    auto imageNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<ImagePattern>(); });
    CHECK_NULL_VOID(imageNode);

    // execute the modifier
    CHECK_NULL_VOID(applyOnNodeSync);
    applyOnNodeSync(AceType::WeakClaim(AceType::RawPtr(imageNode)));

    // get values from the temporary frame node
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    auto opacity = imageContext->GetOpacity();
    if (opacity.has_value()) {
        option.options.opacity = opacity.value();
    } else {
        option.options.opacity = DEFAULT_OPACITY;
    }
}

} // namespace OHOS::Ace
