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
#include "core/interfaces/native/node/column_modifier.h"

#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/alignment.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/linear_layout/column_model_ng.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace::NG {
constexpr FlexAlign DEFAULT_JUSTIFY_CONTENT = FlexAlign::FLEX_START;
const int32_t ERROR_INT_CODE = -1;

void SetColumnJustifyContent(ArkUINodeHandle node, int32_t flexAlign)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ColumnModelNG::SetJustifyContent(frameNode, static_cast<FlexAlign>(flexAlign));
}

void ResetColumnJustifyContent(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ColumnModelNG::SetJustifyContent(frameNode, DEFAULT_JUSTIFY_CONTENT);
}

void SetColumnAlignItems(ArkUINodeHandle node, int32_t value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if ((value == static_cast<int32_t>(FlexAlign::FLEX_START)) ||
        (value == static_cast<int32_t>(FlexAlign::FLEX_END)) || (value == static_cast<int32_t>(FlexAlign::CENTER)) ||
        (value == static_cast<int32_t>(FlexAlign::STRETCH))) {
        ColumnModelNG::SetAlignItems(frameNode, static_cast<FlexAlign>(value));
    } else if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
        ColumnModelNG::SetAlignItems(frameNode, FlexAlign::CENTER);
    }
    FlexAlign value_flexAlign = static_cast<FlexAlign>(value);
    ColumnModelNG::SetAlignItems(frameNode, value_flexAlign);
}

void ResetColumnAlignItems(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ColumnModelNG::SetAlignItems(frameNode, FlexAlign::CENTER);
}

ArkUI_Int32 GetColumnJustifyContent(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return static_cast<ArkUI_Int32>(ColumnModelNG::GetJustifyContent(frameNode));
}

ArkUI_Int32 GetColumnAlignItems(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return static_cast<ArkUI_Int32>(ColumnModelNG::GetAlignItems(frameNode));
}

void SetColumnSpace(ArkUINodeHandle node, ArkUI_Float32 value, ArkUI_Int32 unit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    const auto space = CalcDimension(value, static_cast<OHOS::Ace::DimensionUnit>(unit));
    ColumnModelNG::SetSpace(frameNode, space);
}

void ResetColumnSpace(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    const auto space = CalcDimension(0.0, DimensionUnit::PX);
    ColumnModelNG::SetSpace(frameNode, space);
}

namespace NodeModifier {
const ArkUIColumnModifier* GetColumnModifier()
{
    static const ArkUIColumnModifier modifier = {
        SetColumnJustifyContent,
        ResetColumnJustifyContent,
        SetColumnAlignItems,
        ResetColumnAlignItems,
        GetColumnJustifyContent,
        GetColumnAlignItems,
        SetColumnSpace,
        ResetColumnSpace,
    };
    return &modifier;
}
}
}
