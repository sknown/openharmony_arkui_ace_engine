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

#include "core/components_ng/pattern/image/image_paint_method.h"

namespace OHOS::Ace::NG {
void ImagePaintMethod::UpdatePaintConfig(const RefPtr<ImageRenderProperty>& renderProps, PaintWrapper* paintWrapper)
{
    auto&& config = canvasImage_->GetPaintConfig();
    config.renderMode_ = renderProps->GetImageRenderMode().value_or(ImageRenderMode::ORIGINAL);
    config.imageInterpolation_ = renderProps->GetImageInterpolation().value_or(ImageInterpolation::NONE);
    config.imageRepeat_ = renderProps->GetImageRepeat().value_or(ImageRepeat::NO_REPEAT);
    auto pipelineCtx = PipelineBase::GetCurrentContext();
    bool isRightToLeft = pipelineCtx && pipelineCtx->IsRightToLeft();
    config.needFlipCanvasHorizontally_ = isRightToLeft && renderProps->GetMatchTextDirection().value_or(false);
    auto colorFilterMatrix = renderProps->GetColorFilter();
    if (colorFilterMatrix.has_value()) {
        config.colorFilter_ = std::make_shared<std::vector<float>>(colorFilterMatrix.value());
    }
    // scale for recordingCanvas: take padding into account
    auto frameSize = paintWrapper->GetGeometryNode()->GetFrameSize();
    auto contentSize = paintWrapper->GetContentSize();
    config.scaleX_ = contentSize.Width() / frameSize.Width();
    config.scaleY_ = contentSize.Height() / frameSize.Height();

    // update border radius
    if (renderProps->GetNeedBorderRadiusValue(false)) {
        auto renderCtx = paintWrapper->GetRenderContext();
        CHECK_NULL_VOID(renderCtx);
        auto borderRadius = renderCtx->GetBorderRadius();
        BorderRadiusArray radiusXY = { PointF(borderRadius->radiusTopLeft->ConvertToPx(),
                                              borderRadius->radiusTopLeft->ConvertToPx()),
            PointF(borderRadius->radiusTopRight->ConvertToPx(), borderRadius->radiusTopRight->ConvertToPx()),
            PointF(borderRadius->radiusBottomLeft->ConvertToPx(), borderRadius->radiusBottomLeft->ConvertToPx()),
            PointF(borderRadius->radiusBottomRight->ConvertToPx(), borderRadius->radiusBottomRight->ConvertToPx()) };
        config.borderRadiusXY_ = std::make_shared<BorderRadiusArray>(std::move(radiusXY));
    }
}

CanvasDrawFunction ImagePaintMethod::GetContentDrawFunction(PaintWrapper* paintWrapper)
{
    CHECK_NULL_RETURN(canvasImage_, nullptr);
    auto offset = paintWrapper->GetContentOffset();
    auto contentSize = paintWrapper->GetContentSize();

    // update render props to ImagePaintConfig
    auto props = DynamicCast<ImageRenderProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_RETURN(props, nullptr);
    UpdatePaintConfig(props, paintWrapper);
    ImagePainter imagePainter(canvasImage_);
    return
        [imagePainter, offset, contentSize](RSCanvas& canvas) { imagePainter.DrawImage(canvas, offset, contentSize); };
}
} // namespace OHOS::Ace::NG