/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SHAPE_SHAPE_MODEL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SHAPE_SHAPE_MODEL_H

#include <memory>
#include <mutex>

#include "base/geometry/dimension.h"
#include "base/image/pixel_map.h"
#include "base/memory/referenced.h"
#include "core/components/common/properties/color.h"

namespace OHOS::Ace {
class ACE_FORCE_EXPORT ShapeModel {
public:
    static ShapeModel* GetInstance();
    virtual ~ShapeModel() = default;

    virtual void Create();
    virtual void SetBitmapMesh(std::vector<double>& mesh, int32_t column, int32_t row);
    virtual void SetViewPort(
        const Dimension& dimLeft, const Dimension& dimTop, const Dimension& dimWidth, const Dimension& dimHeight);
    virtual void InitBox(RefPtr<PixelMap>& pixMap);
    virtual void SetStroke(const Color& color);
    virtual void SetFill(const Color& color);
    virtual void SetStrokeDashOffset(const Ace::Dimension& dashOffset);
    virtual void SetStrokeLineCap(int lineCapStyle);
    virtual void SetStrokeLineJoin(int lineJoinStyle);
    virtual void SetStrokeMiterLimit(double miterLimit);
    virtual void SetStrokeOpacity(double opacity);
    virtual void SetFillOpacity(double opacity);
    virtual void SetStrokeWidth(const Ace::Dimension& lineWidth);
    virtual void SetStrokeDashArray(const std::vector<Ace::Dimension>& dashArray);
    virtual void SetAntiAlias(bool antiAlias);
    virtual void SetWidth();
    virtual void SetHeight();

private:
    static std::unique_ptr<ShapeModel> instance_;
    static std::mutex mutex_;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SHAPE_SHAPE_MODEL_H
