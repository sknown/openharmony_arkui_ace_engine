/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core/components_ng/pattern/grid_col/grid_col_model_ng.h"

#include "base/memory/referenced.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/grid_col/grid_col_layout_pattern.h"
#include "core/components_ng/property/property.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
void GridColModelNG::Create()
{
    RefPtr<V2::GridContainerSize> span = AceType::MakeRefPtr<V2::GridContainerSize>(1);
    RefPtr<V2::GridContainerSize> offset = AceType::MakeRefPtr<V2::GridContainerSize>(0);
    RefPtr<V2::GridContainerSize> order = AceType::MakeRefPtr<V2::GridContainerSize>(0);
    Create(span, offset, order);
}

void GridColModelNG::Create(const RefPtr<V2::GridContainerSize> &span, const RefPtr<V2::GridContainerSize> &offset,
    const RefPtr<V2::GridContainerSize> &order)
{
    auto *stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::GRID_COL_ETS_TAG, nodeId);
    auto frameNode = FrameNode::GetOrCreateFrameNode(V2::GRID_COL_ETS_TAG, nodeId,
        []() { return AceType::MakeRefPtr<GridColLayoutPattern>(); });
    stack->Push(frameNode);

    ACE_UPDATE_LAYOUT_PROPERTY(GridColLayoutProperty, Span, *span);
    ACE_UPDATE_LAYOUT_PROPERTY(GridColLayoutProperty, Offset, *offset);
    ACE_UPDATE_LAYOUT_PROPERTY(GridColLayoutProperty, Order, *order);
}

RefPtr<FrameNode> GridColModelNG::CreateFrameNode(int32_t nodeId)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::GRID_COL_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<GridColLayoutPattern>(); });
    return frameNode;
}

void GridColModelNG::SetSpan(const RefPtr<V2::GridContainerSize> &span)
{
    ACE_UPDATE_LAYOUT_PROPERTY(GridColLayoutProperty, Span, *span);
}

void GridColModelNG::SetOffset(const RefPtr<V2::GridContainerSize> &offset)
{
    ACE_UPDATE_LAYOUT_PROPERTY(GridColLayoutProperty, Offset, *offset);
}

void GridColModelNG::SetOrder(const RefPtr<V2::GridContainerSize> &order)
{
    ACE_UPDATE_LAYOUT_PROPERTY(GridColLayoutProperty, Order, *order);
}

void GridColModelNG::SetSpan(FrameNode *frameNode, const RefPtr<V2::GridContainerSize> &span)
{
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(GridColLayoutProperty, Span, *span, frameNode);
}

void GridColModelNG::SetOffset(FrameNode *frameNode, const RefPtr<V2::GridContainerSize> &offset)
{
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(GridColLayoutProperty, Offset, *offset, frameNode);
}

void GridColModelNG::SetOrder(FrameNode *frameNode, const RefPtr<V2::GridContainerSize> &order)
{
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(GridColLayoutProperty, Order, *order, frameNode);
}
} // namespace OHOS::Ace::NG
