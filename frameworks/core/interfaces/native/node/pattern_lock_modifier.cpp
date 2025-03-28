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

#include "core/interfaces/native/node/pattern_lock_modifier.h"
#include "core/components_v2/pattern_lock/pattern_lock_theme.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/alignment.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/patternlock/patternlock_model_ng.h"
#include "core/pipeline/base/element_register.h"
namespace OHOS::Ace::NG {
void SetPatternLockActiveColor(ArkUINodeHandle node, uint32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetActiveColor(frameNode, Color(value));
}

void ResetPatternLockActiveColor(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto patternLockTheme = GetTheme<V2::PatternLockTheme>();
    CHECK_NULL_VOID(patternLockTheme);
    Color activeColor = patternLockTheme->GetActiveColor();
    PatternLockModelNG::SetActiveColor(frameNode, activeColor);
}

void ResetPatternLockCircleRadius(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto patternLockTheme = GetTheme<V2::PatternLockTheme>();
    CHECK_NULL_VOID(patternLockTheme);
    CalcDimension radius = patternLockTheme->GetCircleRadius();
    PatternLockModelNG::SetCircleRadius(frameNode, Dimension(radius.Value(), radius.Unit()));
}

void SetPatternLockCircleRadius(ArkUINodeHandle node, ArkUI_Float32 number, ArkUI_Int32 unit)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetCircleRadius(frameNode, Dimension(number, static_cast<DimensionUnit>(unit)));
}

void SetPatternLockSelectedColor(ArkUINodeHandle node, uint32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetSelectedColor(frameNode, Color(value));
}

void ResetPatternLockSelectedColor(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto patternLockTheme = GetTheme<V2::PatternLockTheme>();
    CHECK_NULL_VOID(patternLockTheme);
    Color selectedColor = patternLockTheme->GetSelectedColor();
    PatternLockModelNG::SetSelectedColor(frameNode, selectedColor);
}

void SetPatternLockSideLength(ArkUINodeHandle node, ArkUI_Float32 number, ArkUI_Int32 unit)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetSideLength(frameNode, Dimension(number, static_cast<DimensionUnit>(unit)));
}

void ResetPatternLockSideLength(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto patternLockTheme = GetTheme<V2::PatternLockTheme>();
    CHECK_NULL_VOID(patternLockTheme);
    CalcDimension sideLength = patternLockTheme->GetSideLength();
    PatternLockModelNG::SetSideLength(frameNode, Dimension(sideLength.Value(), sideLength.Unit()));
}

void SetPatternLockAutoReset(ArkUINodeHandle node, uint32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetAutoReset(frameNode, static_cast<bool>(value));
}

void ResetPatternLockAutoReset(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetAutoReset(frameNode, true);
}

void SetPatternLockPathStrokeWidth(ArkUINodeHandle node, ArkUI_Float32 number, ArkUI_Int32 unit)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetStrokeWidth(frameNode, Dimension(number, static_cast<DimensionUnit>(unit)));
}

void ResetPatternLockPathStrokeWidth(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto patternLockTheme = GetTheme<V2::PatternLockTheme>();
    CHECK_NULL_VOID(patternLockTheme);
    CalcDimension lineWidth = patternLockTheme->GetPathStrokeWidth();
    PatternLockModelNG::SetStrokeWidth(frameNode, lineWidth);
}

void SetPatternLockRegularColor(ArkUINodeHandle node, uint32_t color)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetRegularColor(frameNode, Color(color));
}

void ResetPatternLockRegularColor(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto patternLockTheme = GetTheme<V2::PatternLockTheme>();
    CHECK_NULL_VOID(patternLockTheme);
    Color regularColor = patternLockTheme->GetRegularColor();
    PatternLockModelNG::SetRegularColor(frameNode, regularColor);
}

void SetPatternLockPathColor(ArkUINodeHandle node, uint32_t color)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetPathColor(frameNode, Color(color));
}

void ResetPatternLockPathColor(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto patternLockTheme = GetTheme<V2::PatternLockTheme>();
    CHECK_NULL_VOID(patternLockTheme);
    Color pathColor = patternLockTheme->GetPathColor();
    PatternLockModelNG::SetPathColor(frameNode, pathColor);
}

void SetPatternLockActiveCircleColor(ArkUINodeHandle node, uint32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetActiveCircleColor(frameNode, Color(value));
}

void ResetPatternLockActiveCircleColor(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetActiveCircleColor(frameNode, Color::TRANSPARENT);
}

void SetPatternLockActiveCircleRadius(ArkUINodeHandle node, ArkUI_Float32 number, ArkUI_Int32 unit)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetActiveCircleRadius(frameNode, Dimension(number, static_cast<DimensionUnit>(unit)));
}

void ResetPatternLockActiveCircleRadius(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto patternLockTheme = GetTheme<V2::PatternLockTheme>();
    CHECK_NULL_VOID(patternLockTheme);
    CalcDimension radius = patternLockTheme->GetCircleRadius();
    PatternLockModelNG::SetActiveCircleRadius(frameNode, Dimension(0.0f, DimensionUnit::VP));
}

void SetPatternLockEnableWaveEffect(ArkUINodeHandle node, uint32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetEnableWaveEffect(frameNode, static_cast<bool>(value));
}

void ResetPatternLockEnableWaveEffect(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PatternLockModelNG::SetEnableWaveEffect(frameNode, true);
}

namespace NodeModifier {
const ArkUIPatternLockModifier* GetPatternLockModifier()
{
    static const ArkUIPatternLockModifier modifier = {
        SetPatternLockActiveColor, ResetPatternLockActiveColor, SetPatternLockCircleRadius,
        ResetPatternLockCircleRadius, SetPatternLockSelectedColor, ResetPatternLockSelectedColor,
        SetPatternLockSideLength, ResetPatternLockSideLength, SetPatternLockAutoReset,
        ResetPatternLockAutoReset, SetPatternLockPathStrokeWidth, ResetPatternLockPathStrokeWidth,
        SetPatternLockRegularColor, ResetPatternLockRegularColor, SetPatternLockPathColor,
        ResetPatternLockPathColor, SetPatternLockActiveCircleColor, ResetPatternLockActiveCircleColor,
        SetPatternLockActiveCircleRadius, ResetPatternLockActiveCircleRadius, SetPatternLockEnableWaveEffect,
        ResetPatternLockEnableWaveEffect
    };
    return &modifier;
}
}
} // namespace OHOS::Ace::NG