/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/checkbox/checkbox_model_ng.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/checkbox/checkbox_pattern.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
void CheckBoxModelNG::Create(
    const std::optional<std::string>& name, const std::optional<std::string>& groupName, const std::string& tagName)
{
    auto* stack = ViewStackProcessor::GetInstance();
    int32_t nodeId = stack->ClaimNodeId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", tagName.c_str(), nodeId);
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(tagName, nodeId, []() { return AceType::MakeRefPtr<CheckBoxPattern>(); });
    ViewStackProcessor::GetInstance()->Push(frameNode);

    auto eventHub = frameNode->GetEventHub<NG::CheckBoxEventHub>();
    if (name.has_value()) {
        eventHub->SetName(name.value());
    }
    if (groupName.has_value()) {
        eventHub->SetGroupName(groupName.value());
    }
}

RefPtr<FrameNode> CheckBoxModelNG::CreateFrameNode(int32_t nodeId)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::CHECK_BOX_ETS_TAG, nodeId, AceType::MakeRefPtr<CheckBoxPattern>());
    CHECK_NULL_RETURN(frameNode, nullptr);
    return frameNode;
}

void CheckBoxModelNG::SetSelect(bool isSelected)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<CheckBoxEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetCurrentUIState(UI_STATE_SELECTED, isSelected);

    ACE_UPDATE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxSelect, isSelected);
}

void CheckBoxModelNG::SetSelectedColor(const Color& color)
{
    ACE_UPDATE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxSelectedColor, color);
}

void CheckBoxModelNG::SetUnSelectedColor(const Color& color)
{
    ACE_UPDATE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxUnSelectedColor, color);
}
void CheckBoxModelNG::SetCheckboxStyle(CheckBoxStyle checkboxStyle)
{
    ACE_UPDATE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxSelectedStyle, checkboxStyle);
}


void CheckBoxModelNG::SetCheckMarkColor(const Color& color)
{
    ACE_UPDATE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxCheckMarkColor, color);
}

void CheckBoxModelNG::SetCheckMarkSize(const Dimension& size)
{
    ACE_UPDATE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxCheckMarkSize, size);
}

void CheckBoxModelNG::SetCheckMarkWidth(const Dimension& width)
{
    ACE_UPDATE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxCheckMarkWidth, width);
}

void CheckBoxModelNG::SetOnChange(ChangeEvent&& onChange)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<CheckBoxEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnChange(std::move(onChange));
}

void CheckBoxModelNG::SetWidth(const Dimension& width)
{
    NG::ViewAbstract::SetWidth(NG::CalcLength(width));
}

void CheckBoxModelNG::SetHeight(const Dimension& height)
{
    NG::ViewAbstract::SetHeight(NG::CalcLength(height));
}

void CheckBoxModelNG::SetPadding(const NG::PaddingPropertyF& args, const NG::PaddingProperty& newArgs, bool flag)
{
    NG::ViewAbstract::SetPadding(newArgs);
}

void CheckBoxModelNG::SetChangeEvent(ChangeEvent&& changeEvent)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<CheckBoxEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetChangeEvent(std::move(changeEvent));
}

void CheckBoxModelNG::SetResponseRegion(const std::vector<DimensionRect>& responseRegion)
{
    NG::ViewAbstract::SetResponseRegion(responseRegion);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<CheckBoxPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetIsUserSetResponseRegion(true);
}

void CheckBoxModelNG::SetSelect(FrameNode* frameNode, bool isSelected)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxSelect, isSelected, frameNode);
}

void CheckBoxModelNG::SetSelectedColor(FrameNode* frameNode, const Color& color)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxSelectedColor, color, frameNode);
}

void CheckBoxModelNG::SetUnSelectedColor(FrameNode* frameNode, const Color& color)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxUnSelectedColor, color, frameNode);
}

void CheckBoxModelNG::SetCheckMarkColor(FrameNode* frameNode, const Color& color)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxCheckMarkColor, color, frameNode);
}

void CheckBoxModelNG::SetCheckMarkSize(FrameNode* frameNode, const Dimension& size)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxCheckMarkSize, size, frameNode);
}

void CheckBoxModelNG::SetCheckMarkWidth(FrameNode* frameNode, const Dimension& width)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxCheckMarkWidth, width, frameNode);
}

void CheckBoxModelNG::SetHeight(FrameNode* frameNode, const Dimension& height)
{
    NG::ViewAbstract::SetHeight(frameNode, NG::CalcLength(height));
}

void CheckBoxModelNG::SetWidth(FrameNode* frameNode, const Dimension& width)
{
    NG::ViewAbstract::SetWidth(frameNode, NG::CalcLength(width));
}

void CheckBoxModelNG::SetPadding(FrameNode* frameNode, const NG::PaddingProperty& padding)
{
    NG::ViewAbstract::SetPadding(padding);
}

void CheckBoxModelNG::SetResponseRegion(FrameNode* frameNode, const std::vector<DimensionRect>& responseRegion)
{
    NG::ViewAbstract::SetResponseRegion(responseRegion);
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<CheckBoxPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetIsUserSetResponseRegion(true);
}

void CheckBoxModelNG::SetCheckboxStyle(FrameNode* frameNode, CheckBoxStyle checkboxStyle)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxSelectedStyle, checkboxStyle, frameNode);
}

bool CheckBoxModelNG::GetSelect(FrameNode* frameNode)
{
    bool value = false;
    ACE_GET_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxSelect, value, frameNode);
    return value;
}

Color CheckBoxModelNG::GetSelectedColor(FrameNode* frameNode)
{
    Color value;
    ACE_GET_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxSelectedColor, value, frameNode);
    return value;
}

Color CheckBoxModelNG::GetUnSelectedColor(FrameNode* frameNode)
{
    Color value;
    ACE_GET_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxUnSelectedColor, value, frameNode);
    return value;
}

Color CheckBoxModelNG::GetCheckMarkColor(FrameNode* frameNode)
{
    Color value;
    ACE_GET_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxCheckMarkColor, value, frameNode);
    return value;
}

Dimension CheckBoxModelNG::GetCheckMarkSize(FrameNode* frameNode)
{
    Dimension value;
    ACE_GET_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxCheckMarkSize, value, frameNode);
    return value;
}

Dimension CheckBoxModelNG::GetCheckMarkWidth(FrameNode* frameNode)
{
    Dimension value;
    ACE_GET_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxCheckMarkWidth, value, frameNode);
    return value;
}

CheckBoxStyle CheckBoxModelNG::GetCheckboxStyle(FrameNode* frameNode)
{
    CheckBoxStyle checkboxStyle = CheckBoxStyle::CIRCULAR_STYLE;
    ACE_GET_NODE_PAINT_PROPERTY(CheckBoxPaintProperty, CheckBoxSelectedStyle, checkboxStyle, frameNode);
    return checkboxStyle;
}

void CheckBoxModelNG::SetOnChange(FrameNode* frameNode, ChangeEvent&& onChange)
{
    auto eventHub = frameNode->GetEventHub<CheckBoxEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnChange(std::move(onChange));
}

} // namespace OHOS::Ace::NG
