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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SHAPE_ABSTRACT_MODEL_NG_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SHAPE_ABSTRACT_MODEL_NG_H

#include "base/geometry/dimension.h"
#include "base/utils/macros.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/shape/shape_abstract_model.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT ShapeAbstractModelNG : public ShapeAbstractModel {
public:
    void SetStroke(const Color& color) override;
    void SetFill(const Color& color) override;
    void SetStrokeDashOffset(const Ace::Dimension& dashOffset) override;
    void SetStrokeLineCap(int lineCapStyle) override;
    void SetStrokeLineJoin(int lineJoinStyle) override;
    void SetStrokeMiterLimit(double miterLimit) override;
    void SetStrokeOpacity(double opacity) override;
    void SetFillOpacity(double opacity) override;
    void SetStrokeWidth(const Ace::Dimension& lineWidth) override;
    void SetStrokeDashArray(const std::vector<Ace::Dimension>& segments) override;
    void SetAntiAlias(bool antiAlias) override;
    void SetWidth(Dimension& width) override;
    void SetHeight(Dimension& height) override;
    static void SetWidth(FrameNode* frameNode, Dimension& width);
    static void ResetWidth(FrameNode* frameNode);
    static void SetHeight(FrameNode* frameNode, Dimension& height);
    static void ResetHeight(FrameNode* frameNode);
    static void SetStroke(FrameNode* frameNode, const Color& color);
    static void SetFill(FrameNode* frameNode, const Color& color);
    static void SetStrokeWidth(FrameNode* frameNode, const Ace::Dimension& lineWidth);
};

} // namespace OHOS::Ace::NG

#endif