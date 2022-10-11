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

#include "grid_row_view.h"

#include "base/memory/referenced.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "grid_row_layout_pattern.h"

namespace OHOS::Ace::NG {
void GridRowView::Create()
{
    auto col = Referenced::MakeRefPtr<V2::GridContainerSize>();
    auto gutter = Referenced::MakeRefPtr<V2::Gutter>();
    auto breakpoints = Referenced::MakeRefPtr<V2::BreakPoints>();
    auto direction = V2::GridRowDirection::Row;
    Create(col, gutter, breakpoints, direction);
}

void GridRowView::Create(const RefPtr<V2::GridContainerSize>& col, const RefPtr<V2::Gutter>& gutter,
    const RefPtr<V2::BreakPoints>& breakpoints, V2::GridRowDirection direction)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::GRID_ROW_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<GridRowLayoutPattern>(); });
    stack->Push(frameNode);

    ACE_UPDATE_LAYOUT_PROPERTY(GridRowLayoutProperty, Columns, *col);
    ACE_UPDATE_LAYOUT_PROPERTY(GridRowLayoutProperty, Gutter, *gutter);
    ACE_UPDATE_LAYOUT_PROPERTY(GridRowLayoutProperty, BreakPoints, *breakpoints);
    ACE_UPDATE_LAYOUT_PROPERTY(GridRowLayoutProperty, Direction, direction);
}
} // namespace OHOS::Ace::NG