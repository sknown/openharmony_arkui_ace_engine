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
#include "core/interfaces/native/node/line_modifier.h"

#include "core/pipeline/base/element_register.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/shape/line_model_ng.h"
#include "core/components/common/layout/constants.h"
#include "base/geometry/shape.h"

namespace OHOS::Ace::NG {
void SetStartPoint(ArkUINodeHandle node, const ArkUI_Float32* pointValues, const ArkUI_Int32* pointUnits,
    const char* pointStr[])
{
    ShapePoint point;
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    point.first = Dimension(pointValues[0], static_cast<OHOS::Ace::DimensionUnit>(pointUnits[0]));
    point.second = Dimension(pointValues[1], static_cast<OHOS::Ace::DimensionUnit>(pointUnits[1]));
    LineModelNG::StartPoint(frameNode, point);
}

void ResetStartPoint(ArkUINodeHandle node)
{
    ShapePoint point;
    point.first = 0.0_vp;
    point.second = 0.0_vp;
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    LineModelNG::StartPoint(frameNode, point);
}

void SetEndPoint(ArkUINodeHandle node, const ArkUI_Float32* pointValues, const ArkUI_Int32* pointUnits,
    const char* pointStr[])
{
    ShapePoint point;
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    point.first = Dimension(pointValues[0], static_cast<OHOS::Ace::DimensionUnit>(pointUnits[0]));
    point.second = Dimension(pointValues[1], static_cast<OHOS::Ace::DimensionUnit>(pointUnits[1]));
    LineModelNG::EndPoint(frameNode, point);
}

void ResetEndPoint(ArkUINodeHandle node)
{
    ShapePoint point;
    point.first = 0.0_vp;
    point.second = 0.0_vp;
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    LineModelNG::EndPoint(frameNode, point);
}

namespace NodeModifier {
const ArkUILineModifier* GetLineModifier()
{
    static const ArkUILineModifier modifier = {SetStartPoint, ResetStartPoint, SetEndPoint, ResetEndPoint};
    return &modifier;
}
}
}