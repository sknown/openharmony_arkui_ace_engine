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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_CANVAS_IMAGE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_CANVAS_IMAGE_H

#include <memory>
#include "base/geometry/ng/rect_t.h"
#include "base/memory/ace_type.h"
#include "base/utils/noncopyable.h"
#include "core/components/common/properties/decoration.h"
#include "core/components_ng/render/drawing_forward.h"

namespace OHOS::Ace::NG {

struct ImagePaintConfig {
    RectF srcRect_;
    RectF dstRect_;
    ImageRenderMode renderMode_ = ImageRenderMode::ORIGINAL;
    ImageInterpolation imageInterpolation_ = ImageInterpolation::NONE;
    ImageRepeat imageRepeat_ = ImageRepeat::NO_REPEAT;
    ImageFit imageFit_ = ImageFit::COVER;
    std::shared_ptr<std::vector<float>> colorFilter_ = nullptr;
    bool needFlipCanvasHorizontally_ = false;
    std::shared_ptr<std::array<PointF, 4>> borderRadiusXY_ = nullptr;
    bool isSvg_ = false;
};

struct RenderTaskHolder;
// CanvasImage is interface for drawing image.
class CanvasImage : public virtual AceType {
    DECLARE_ACE_TYPE(CanvasImage, AceType)

public:
    CanvasImage() = default;
    ~CanvasImage() override = default;
    virtual void DrawToRSCanvas(
        RSCanvas& canvas, const RSRect& srcRect, const RSRect& dstRect, const std::array<PointF, 4>& radiusXY) = 0;

    static RefPtr<CanvasImage> Create(void* rawImage);
    static RefPtr<CanvasImage> Create();
    // TODO: use [PixelMap] as data source when rs provides interface like
    // DrawBitmapRect(Media::PixelMap* pixelMap, const Rect& dstRect, const Rect& srcRect, ...)
    // now we make [SkImage] from [PixelMap] and use [drawImageRect] to draw image
    static RefPtr<CanvasImage> Create(
        const RefPtr<PixelMap>& pixelMap, const RefPtr<RenderTaskHolder>& renderTaskHolder);
    virtual int32_t GetWidth() const = 0;
    virtual int32_t GetHeight() const = 0;

    void SetPaintConfig(const ImagePaintConfig& config)
    {
        paintConfig_ = std::make_unique<ImagePaintConfig>(config);
    }

    ImagePaintConfig& GetPaintConfig()
    {
        return *paintConfig_;
    }

private:
    std::unique_ptr<ImagePaintConfig> paintConfig_;

    ACE_DISALLOW_COPY_AND_MOVE(CanvasImage);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_CANVAS_IMAGE_H
