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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_IMAGE_IMAGE_PAINT_METHOD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_IMAGE_IMAGE_PAINT_METHOD_H

#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/macros.h"
#include "base/utils/utils.h"
#include "core/components_ng/render/image_painter.h"
#include "core/components_ng/render/node_paint_method.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT ImagePaintMethod : public NodePaintMethod {
    DECLARE_ACE_TYPE(ImagePaintMethod, NodePaintMethod)
public:
    ImagePaintMethod(const RefPtr<CanvasImage>& canvasImage, const ImagePaintConfig& ImagePaintConfig)
        : canvasImage_(canvasImage), imagePaintConfig_(ImagePaintConfig)
    {}
    ~ImagePaintMethod() override = default;

    CanvasDrawFunction GetContentDrawFunction(PaintWrapper* paintWrapper) override
    {
        CHECK_NULL_RETURN(canvasImage_, nullptr);
        auto offset = paintWrapper->GetContentOffset();
        auto contentSize = paintWrapper->GetContentSize();
        ImagePainter imagePainter(canvasImage_);
        return [imagePainter, offset, ImagePaintConfig = imagePaintConfig_, contentSize](RSCanvas& canvas) {
            if (ImagePaintConfig.isSvg) {
                imagePainter.DrawSVGImage(canvas, offset, contentSize, ImagePaintConfig);
                return;
            }
            if (ImagePaintConfig.imageRepeat_ == ImageRepeat::NOREPEAT) {
                imagePainter.DrawImage(canvas, offset, ImagePaintConfig);
                return;
            }
            imagePainter.DrawImageWithRepeat(canvas, ImagePaintConfig,
                RectF(offset.GetX(), offset.GetY(), contentSize.Width(), contentSize.Height()));
        };
    }

private:
    RefPtr<CanvasImage> canvasImage_;
    ImagePaintConfig imagePaintConfig_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_IMAGE_IMAGE_PAINT_METHOD_H