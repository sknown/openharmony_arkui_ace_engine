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

#include "core/components_ng/pattern/linear_layout/row_model_ng.h"

#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

void RowModelNG::Create(const std::optional<Dimension>& space, AlignDeclaration*, const std::string& tag)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::ROW_ETS_TAG, nodeId);
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::ROW_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<LinearLayoutPattern>(false); });
    stack->Push(frameNode);
    ACE_UPDATE_LAYOUT_PROPERTY(LinearLayoutProperty, FlexDirection, FlexDirection::ROW);

    CHECK_NULL_VOID(space);
    if (GreatOrEqual(space->Value(), 0.0)) {
        ACE_UPDATE_LAYOUT_PROPERTY(LinearLayoutProperty, Space, space.value());
    }
}

void RowModelNG::SetSpace(FrameNode* frameNode, const std::optional<Dimension>& space)
{
    CHECK_NULL_VOID(space);
    if (GreatOrEqual(space->Value(), 0.0)) {
        ACE_UPDATE_NODE_LAYOUT_PROPERTY(LinearLayoutProperty, Space, space.value(), frameNode);
    } else {
        LOGE("Column: the space value is illegal due to space is less than zero");
    }
}

RefPtr<FrameNode> RowModelNG::CreateFrameNode(int32_t nodeId)
{
    auto frameNode = FrameNode::CreateFrameNode(
        V2::ROW_ETS_TAG, nodeId, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    auto castLinearLayoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
    if (castLinearLayoutProperty) {
        castLinearLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    }
    return frameNode;
}

void RowModelNG::SetAlignItems(FlexAlign flexAlign)
{
    ACE_UPDATE_LAYOUT_PROPERTY(LinearLayoutProperty, CrossAxisAlign, flexAlign);
}

void RowModelNG::SetJustifyContent(FlexAlign flexAlign)
{
    ACE_UPDATE_LAYOUT_PROPERTY(LinearLayoutProperty, MainAxisAlign, flexAlign);
}

void RowModelNG::SetJustifyContent(FrameNode* frameNode, FlexAlign flexAlign)
{
    CHECK_NULL_VOID(frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(LinearLayoutProperty, MainAxisAlign, flexAlign, frameNode);
}

void RowModelNG::SetAlignItems(FrameNode* frameNode, FlexAlign flexAlign)
{
    CHECK_NULL_VOID(frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(LinearLayoutProperty, CrossAxisAlign, flexAlign, frameNode);
}

FlexAlign RowModelNG::GetJustifyContent(FrameNode* frameNode)
{
    FlexAlign value = FlexAlign::AUTO;
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(LinearLayoutProperty, MainAxisAlign, value, frameNode, value);
    return value;
}

FlexAlign RowModelNG::GetAlignItems(FrameNode* frameNode)
{
    FlexAlign value = FlexAlign::CENTER;
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(LinearLayoutProperty, CrossAxisAlign, value, frameNode, value);
    return value;
}
} // namespace OHOS::Ace::NG
