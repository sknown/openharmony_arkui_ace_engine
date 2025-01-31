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
#include "core/interfaces/native/node/node_list_item_group_modifier.h"

#include "interfaces/native/node/list_option.h"

#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/list/list_item_group_model_ng.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace::NG {
constexpr int CALL_ARG_0 = 0;
constexpr int CALL_ARG_1 = 1;
constexpr int CALL_ARG_2 = 2;
constexpr int32_t DEFAULT_GROUP_DIVIDER_VALUES_COUNT = 3;

void ListItemGroupSetDivider(
    ArkUINodeHandle node, ArkUI_Uint32 color, const ArkUI_Float32* values, const ArkUI_Int32* units, ArkUI_Int32 length)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);

    if (length != DEFAULT_GROUP_DIVIDER_VALUES_COUNT) {
        return;
    }

    V2::ItemDivider divider;
    divider.color = Color(color);
    divider.strokeWidth = Dimension(values[CALL_ARG_0], static_cast<OHOS::Ace::DimensionUnit>(units[CALL_ARG_0]));
    divider.startMargin = Dimension(values[CALL_ARG_1], static_cast<OHOS::Ace::DimensionUnit>(units[CALL_ARG_1]));
    divider.endMargin = Dimension(values[CALL_ARG_2], static_cast<OHOS::Ace::DimensionUnit>(units[CALL_ARG_2]));

    ListItemGroupModelNG::SetDivider(frameNode, divider);
}

void ListItemGroupResetDivider(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    const V2::ItemDivider divider;
    ListItemGroupModelNG::SetDivider(frameNode, divider);
}

void ListItemGroupSetHeader(ArkUINodeHandle node, ArkUINodeHandle header)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto headerNode = reinterpret_cast<FrameNode*>(header);
    CHECK_NULL_VOID(headerNode);
    ListItemGroupModelNG::SetHeader(frameNode, headerNode);
}

void ListItemGroupSetFooter(ArkUINodeHandle node, ArkUINodeHandle footer)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto footerNode = reinterpret_cast<FrameNode*>(footer);
    CHECK_NULL_VOID(footerNode);
    ListItemGroupModelNG::SetFooter(frameNode, footerNode);
}

void SetListItemGroupChildrenMainSize(ArkUINodeHandle node, ArkUIListChildrenMainSize option, int32_t unit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    for (uint32_t i = 0; i < option->mainSize.size(); i++) {
        if (option->mainSize[i] > 0) {
            option->mainSize[i] =
                Dimension(option->mainSize[i], static_cast<OHOS::Ace::DimensionUnit>(unit)).ConvertToPx();
        }
    }
    if (option->defaultMainSize > 0) {
        option->defaultMainSize =
            Dimension(option->defaultMainSize, static_cast<OHOS::Ace::DimensionUnit>(unit)).ConvertToPx();
    }
    ListItemGroupModelNG::SetListChildrenMainSize(frameNode, option->defaultMainSize, option->mainSize);
}

void ResetListItemGroupChildrenMainSize(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ListItemGroupModelNG::ResetListChildrenMainSize(frameNode);
}

void GetlistDivider(ArkUINodeHandle node, ArkUIdividerOptions* option, ArkUI_Int32 unit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto divider = ListItemGroupModelNG::GetDivider(frameNode);
    option->color = divider.color.GetValue();
    option->strokeWidth = divider.strokeWidth.GetNativeValue(static_cast<DimensionUnit>(unit));
    option->startMargin = divider.startMargin.GetNativeValue(static_cast<DimensionUnit>(unit));
    option->endMargin = divider.endMargin.GetNativeValue(static_cast<DimensionUnit>(unit));
}

namespace NodeModifier {
const ArkUIListItemGroupModifier* GetListItemGroupModifier()
{
    static const ArkUIListItemGroupModifier modifier = { ListItemGroupSetDivider, ListItemGroupResetDivider,
        ListItemGroupSetHeader, ListItemGroupSetFooter, SetListItemGroupChildrenMainSize,
        ResetListItemGroupChildrenMainSize };
    return &modifier;
}
} // namespace NodeModifier
} // namespace OHOS::Ace::NG