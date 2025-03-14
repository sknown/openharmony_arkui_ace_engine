/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "core/interfaces/native/node/column_split_modifier.h"

#include <optional>

#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/linear_split/linear_split_model.h"
#include "core/components_ng/pattern/linear_split/linear_split_model_ng.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace::NG {
constexpr bool DEFAULT_COLUMN_SPLIT_RESIZABLE = false;
constexpr Dimension DEFAULT_DIVIDER_START = Dimension(0.0, DimensionUnit::VP);
constexpr Dimension DEFAULT_DIVIDER_END = Dimension(0.0, DimensionUnit::VP);
void SetColumnSplitResizable(ArkUINodeHandle node, ArkUI_Bool resizable)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    LinearSplitModelNG::SetResizable(frameNode, NG::SplitType::COLUMN_SPLIT, resizable);
}

void ResetColumnSplitResizable(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    LinearSplitModelNG::SetResizable(frameNode, NG::SplitType::COLUMN_SPLIT, DEFAULT_COLUMN_SPLIT_RESIZABLE);
}

void SetColumnSplitDivider(ArkUINodeHandle node, ArkUI_Float32 stVal, int32_t stUnit,
    ArkUI_Float32 endVal, int32_t endUnit)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Dimension startMarginDimension(stVal, static_cast<DimensionUnit>(stUnit));
    Dimension endMarginDimension(endVal, static_cast<DimensionUnit>(endUnit));
    ItemDivider divider = { startMarginDimension, endMarginDimension };
    LinearSplitModelNG::SetDivider(frameNode, SplitType::COLUMN_SPLIT, divider);
}

void ResetColumnSplitDivider(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    LinearSplitModelNG::SetDivider(frameNode, SplitType::COLUMN_SPLIT, { DEFAULT_DIVIDER_START, DEFAULT_DIVIDER_END });
}

namespace NodeModifier {
const ArkUIColumnSplitModifier* GetColumnSplitModifier()
{
    static const ArkUIColumnSplitModifier modifier = { SetColumnSplitDivider, ResetColumnSplitDivider,
                                                       SetColumnSplitResizable, ResetColumnSplitResizable };
    return &modifier;
}
}
} // namespace OHOS::Ace::NG
