/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "core/components/focus_animation/rosen_render_focus_animation.h"

#include <cmath>

#ifndef USE_ROSEN_DRAWING
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradientShader.h"
#endif

#include "base/utils/system_properties.h"
#include "core/pipeline/base/rosen_render_context.h"

namespace OHOS::Ace {
namespace {

constexpr int32_t BOUNDARY_WIDTH = 1;
constexpr int32_t PHONE_BOUNDARY_WIDTH = 2;
constexpr int32_t MAX_ALPHA = 255;
constexpr int32_t FOCUS_ANIMATION_OFFSET = 3;
constexpr int32_t PHONE_FOCUS_OFFSET = 3;
constexpr int32_t PHONE_INDENTED_FOCUS_OFFSET = 1;
#ifndef USE_ROSEN_DRAWING
constexpr int32_t ARRAY_LENGTH = 6;
#endif
constexpr float RIGHT_ANGLE = 90.0f;
constexpr float MAX_TRANSPARENCY = 153.0f;
constexpr float MIN_TRANSPARENCY = 26.0f;
constexpr float MULTIPLE_FACTOR = 2.0f;
constexpr float LEFTEDGE_START_PERCENT = 0.0f;
constexpr float LEFTGLOWEDGE_START_PERCENT = 0.25f;
constexpr float MIDGLOWEDGE_START_PERCENT = 0.41f;
constexpr float RIGHTGLOWEDGE_START_PERCENT = 0.58f;
constexpr float RIGHTEDGE_START_PERCENT = 0.75f;
constexpr float LEFTGLOWEDGE_START_PERCENT_FORLARGEASPECT = 0.33f;
constexpr float MIDGLOWEDGE_START_PERCENT_FORLARGEASPECT = 0.44f;
constexpr float RIGHTGLOWEDGE_START_PERCENT_FORLARGEASPECT = 0.56f;
constexpr float RIGHTEDGE_START_PERCENT_FORLARGEASPECT = 0.67f;
constexpr float RIGHTEDGE_END_PERCENT = 1.0f;
constexpr double DOUBLE_FACTOR = 2.0;
constexpr double BLUR_SIGMA_FACTOR = 0.33;

} // namespace

void RosenRenderFocusAnimation::Paint(RenderContext& context, const Offset& offset)
{
    if (!isDisplay_ || NearZero(rrect_.Width()) || NearZero(rrect_.Height())) {
        return;
    }
    auto pipelineContext = context_.Upgrade();
    if (!pipelineContext) {
        LOGE("pipeline context is nullptr");
        return;
    }
    if (SystemProperties::GetDeviceType() == DeviceType::PHONE && !pipelineContext->IsKeyEvent()) {
        return;
    }
    auto canvas = static_cast<RosenRenderContext*>(&context)->GetCanvas();
    if (!canvas) {
        LOGE("canvas fetch failed");
        return;
    }
    if (SystemProperties::GetDeviceType() == DeviceType::TV) {
        PaintTVFocus(canvas);
    } else if (SystemProperties::GetDeviceType() == DeviceType::PHONE) {
        PaintPhoneFocus(canvas);
    }
}

#ifndef USE_ROSEN_DRAWING
void RosenRenderFocusAnimation::PaintGlow(SkCanvas* skCanvas, SkPaint& paint, int32_t padding) const
{
    int32_t maxHeight = sqrt(pow(width_ + MULTIPLE_FACTOR * padding, MULTIPLE_FACTOR) +
                             pow(height_ + MULTIPLE_FACTOR * padding, MULTIPLE_FACTOR));

    paint.setBlendMode(SkBlendMode::kSrcIn);
    paint.setStyle(SkPaint::kFill_Style);

    SkPoint points[2] = { SkPoint::Make(0 - width_ / MULTIPLE_FACTOR, 0.0f),
        SkPoint::Make(width_ / MULTIPLE_FACTOR, 0.0f) };
    skCanvas->translate(width_ / MULTIPLE_FACTOR, height_ / MULTIPLE_FACTOR);

    // Calculate the angle that each frame moves based on the total number of frames
    skCanvas->rotate(progress_);
    SkScalar pos[ARRAY_LENGTH] = { LEFTEDGE_START_PERCENT, LEFTGLOWEDGE_START_PERCENT, MIDGLOWEDGE_START_PERCENT,
        RIGHTGLOWEDGE_START_PERCENT, RIGHTEDGE_START_PERCENT, RIGHTEDGE_END_PERCENT };

    if (!NearZero(height_)) {
        if (width_ / height_ > DOUBLE_FACTOR) {
            pos[1] = LEFTGLOWEDGE_START_PERCENT_FORLARGEASPECT;
            pos[2] = MIDGLOWEDGE_START_PERCENT_FORLARGEASPECT;
            pos[3] = RIGHTGLOWEDGE_START_PERCENT_FORLARGEASPECT;
            pos[4] = RIGHTEDGE_START_PERCENT_FORLARGEASPECT;
        }
    }

    // Computing transparency
    uint8_t red = pathColor_.GetRed() & 0xFF;
    uint8_t green = pathColor_.GetGreen() & 0xFF;
    uint8_t blue = pathColor_.GetBlue() & 0xFF;
    uint8_t mAlpha = 0;
    if (progress_ > RIGHT_ANGLE) {
        mAlpha = static_cast<uint8_t>(
            MAX_TRANSPARENCY - (progress_ - RIGHT_ANGLE) * ((MAX_TRANSPARENCY - MIN_TRANSPARENCY) / RIGHT_ANGLE));
    } else {
        mAlpha =
            static_cast<uint8_t>(MIN_TRANSPARENCY + progress_ * ((MAX_TRANSPARENCY - MIN_TRANSPARENCY) / RIGHT_ANGLE));
    }
    SkColor boundaryColor = SkColorSetARGB(mAlpha, red, green, blue);
    SkColor glowColor = SkColorSetARGB(MAX_ALPHA, red, green, blue);
    SkColor colors[ARRAY_LENGTH] = { boundaryColor, boundaryColor, glowColor, glowColor, boundaryColor, boundaryColor };

    paint.setShader(SkGradientShader::MakeLinear(points, colors, pos, ARRAY_LENGTH, SkTileMode::kClamp));

    skCanvas->drawRect(
        SkRect::MakeXYWH(0 - (maxHeight / MULTIPLE_FACTOR + padding), 0 - (maxHeight / MULTIPLE_FACTOR + padding),
            maxHeight + MULTIPLE_FACTOR * padding, maxHeight + MULTIPLE_FACTOR * padding),
        paint);
}
#else
void RosenRenderFocusAnimation::PaintGlow(RSCanvas* canvas, RSBrush& brush, int32_t padding) const
{
    int32_t maxHeight = sqrt(pow(width_ + MULTIPLE_FACTOR * padding, MULTIPLE_FACTOR) +
                             pow(height_ + MULTIPLE_FACTOR * padding, MULTIPLE_FACTOR));

    brush.SetBlendMode(RSBlendMode::SRC_IN);

    RSPoint points[2] = { RSPoint(0 - width_ / MULTIPLE_FACTOR, 0.0f), RSPoint(width_ / MULTIPLE_FACTOR, 0.0f) };
    canvas->Translate(width_ / MULTIPLE_FACTOR, height_ / MULTIPLE_FACTOR);

    // Calculate the angle that each frame moves based on the total number of frames
    canvas->Rotate(progress_);

    std::vector<RSScalar> pos = { LEFTEDGE_START_PERCENT, LEFTGLOWEDGE_START_PERCENT, MIDGLOWEDGE_START_PERCENT,
        RIGHTGLOWEDGE_START_PERCENT, RIGHTEDGE_START_PERCENT, RIGHTEDGE_END_PERCENT };

    if (!NearZero(height_)) {
        if (width_ / height_ > DOUBLE_FACTOR) {
            pos.at(1) = LEFTGLOWEDGE_START_PERCENT_FORLARGEASPECT;
            pos.at(2) = MIDGLOWEDGE_START_PERCENT_FORLARGEASPECT;
            pos.at(3) = RIGHTGLOWEDGE_START_PERCENT_FORLARGEASPECT;
            pos.at(4) = RIGHTEDGE_START_PERCENT_FORLARGEASPECT;
        }
    }

    // Computing transparency
    uint8_t red = pathColor_.GetRed() & 0xFF;
    uint8_t green = pathColor_.GetGreen() & 0xFF;
    uint8_t blue = pathColor_.GetBlue() & 0xFF;
    uint8_t mAlpha = 0;
    if (progress_ > RIGHT_ANGLE) {
        mAlpha = static_cast<uint8_t>(
            MAX_TRANSPARENCY - (progress_ - RIGHT_ANGLE) * ((MAX_TRANSPARENCY - MIN_TRANSPARENCY) / RIGHT_ANGLE));
    } else {
        mAlpha =
            static_cast<uint8_t>(MIN_TRANSPARENCY + progress_ * ((MAX_TRANSPARENCY - MIN_TRANSPARENCY) / RIGHT_ANGLE));
    }
    RSColorQuad boundaryColor = RSColor::ColorQuadSetARGB(mAlpha, red, green, blue);
    RSColorQuad glowColor = RSColor::ColorQuadSetARGB(MAX_ALPHA, red, green, blue);
    std::vector<RSColorQuad> colors = { boundaryColor, boundaryColor, glowColor, glowColor, boundaryColor,
        boundaryColor };

    brush.SetShaderEffect(RSShaderEffect::CreateLinearGradient(points[0], points[1], colors, pos, RSTileMode::CLAMP));
    canvas->AttachBrush(brush);
    canvas->DrawRect(RSRect(0 - (maxHeight / MULTIPLE_FACTOR + padding), 0 - (maxHeight / MULTIPLE_FACTOR + padding),
        (maxHeight + MULTIPLE_FACTOR * padding) - (maxHeight / MULTIPLE_FACTOR + padding),
        (maxHeight + MULTIPLE_FACTOR * padding) - (maxHeight / MULTIPLE_FACTOR + padding)));
    canvas->DetachBrush();
}
#endif

#ifndef USE_ROSEN_DRAWING
void RosenRenderFocusAnimation::PaintClipRect(SkCanvas* skCanvas, double offset) const
{
    offset += (NormalizeToPx(blurMaskRadius_) * DOUBLE_FACTOR);
    SkRRect skRRect = SkRRect::MakeRect(SkRect::MakeXYWH(clipRect_.GetOffset().GetX() - offset / DOUBLE_FACTOR,
        clipRect_.GetOffset().GetY() - offset / DOUBLE_FACTOR, clipRect_.Width() + offset,
        clipRect_.Height() + offset));
    skCanvas->clipRRect(skRRect, true);
}
#else
void RosenRenderFocusAnimation::PaintClipRect(RSCanvas* canvas, double offset) const
{
    offset += (NormalizeToPx(blurMaskRadius_) * DOUBLE_FACTOR);
    RSRoundRect rrect;
    rrect.SetRect(RSRect(clipRect_.GetOffset().GetX() - offset / DOUBLE_FACTOR,
        clipRect_.GetOffset().GetY() - offset / DOUBLE_FACTOR,
        clipRect_.Width() + offset + clipRect_.GetOffset().GetX() - offset / DOUBLE_FACTOR,
        clipRect_.Height() + offset + clipRect_.GetOffset().GetY() - offset / DOUBLE_FACTOR));
    canvas->ClipRoundRect(rrect, RSClipOp::INTERSECT, true);
}
#endif

#ifndef USE_ROSEN_DRAWING
void RosenRenderFocusAnimation::PaintTVFocus(SkCanvas* skCanvas)
#else
void RosenRenderFocusAnimation::PaintTVFocus(RSCanvas* canvas)
#endif
{
    double offsetValue = NormalizeToPx(Dimension(FOCUS_ANIMATION_OFFSET, DimensionUnit::VP)) * MULTIPLE_FACTOR;
    double radiusX = NormalizeToPx(rrect_.GetCorner().bottomLeftRadius.GetX());
    double radiusY = NormalizeToPx(rrect_.GetCorner().bottomLeftRadius.GetY());
    radiusX = NearZero(radiusX) ? 0.0 : radiusX + NormalizeToPx(Dimension(FOCUS_ANIMATION_OFFSET, DimensionUnit::VP));
    radiusY = NearZero(radiusX) ? 0.0 : radiusY + NormalizeToPx(Dimension(FOCUS_ANIMATION_OFFSET, DimensionUnit::VP));
    width_ = rrect_.Width() + offsetValue;
    height_ = rrect_.Height() + offsetValue;
    int32_t padding = NormalizeToPx(blurMaskRadius_);

#ifndef USE_ROSEN_DRAWING
    if (isNeedClip_) {
        PaintClipRect(skCanvas, offsetValue);
    }

    std::unique_ptr<SkRect> rect = std::make_unique<SkRect>(SkRect::MakeXYWH(
        0 - padding, 0 - padding, width_ + MULTIPLE_FACTOR * padding, height_ + MULTIPLE_FACTOR * padding));
    skCanvas->translate(offset_.GetX() - offsetValue / MULTIPLE_FACTOR, offset_.GetY() - offsetValue / MULTIPLE_FACTOR);
    int32_t depthOfSavedStack = skCanvas->saveLayerAlpha(rect.get(), MAX_ALPHA);

    SkPath skPath;
    skPath.addRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(0, 0, width_, height_), radiusX, radiusY));

    SkPaint paint;
    paint.setStrokeJoin(SkPaint::Join::kRound_Join);
    paint.setStrokeCap(SkPaint::Cap::kRound_Cap);
    paint.setStrokeWidth(NormalizeToPx(Dimension(BOUNDARY_WIDTH, DimensionUnit::VP)));
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kSolid_SkBlurStyle, NormalizeToPx(blurMaskRadius_) * BLUR_SIGMA_FACTOR));
    paint.setColor(
        SkColorSetARGB(pathColor_.GetAlpha(), pathColor_.GetRed(), pathColor_.GetGreen(), pathColor_.GetBlue()));
    paint.setAlpha(MAX_ALPHA);
    paint.setAntiAlias(true);

    skCanvas->drawPath(skPath, paint);
    PaintGlow(skCanvas, paint, padding);
    skCanvas->restoreToCount(depthOfSavedStack);
#else
    if (isNeedClip_) {
        PaintClipRect(canvas, offsetValue);
    }

    std::unique_ptr<RSRect> rect = std::make_unique<RSRect>(RSRect(0 - padding, 0 - padding,
        width_ + MULTIPLE_FACTOR * padding - padding, height_ + MULTIPLE_FACTOR * padding - padding));
    canvas->Translate(offset_.GetX() - offsetValue / MULTIPLE_FACTOR, offset_.GetY() - offsetValue / MULTIPLE_FACTOR);

    int32_t depthOfSavedStack = static_cast<int32_t>(canvas->GetSaveCount());
    RSBrush layerBrush;
    layerBrush.SetAlpha(MAX_ALPHA);
    RSSaveLayerOps slo(rect.get(), &layerBrush);
    canvas->SaveLayer(slo);

    RSRecordingPath path;
    path.AddRoundRect(RSRect(0, 0, width_, height_), radiusX, radiusY);

    RSPen pen;
    pen.SetJoinStyle(RSPen::JoinStyle::ROUND_JOIN);
    pen.SetCapStyle(RSPen::CapStyle::ROUND_CAP);
    pen.SetWidth(NormalizeToPx(Dimension(BOUNDARY_WIDTH, DimensionUnit::VP)));

    RSFilter filter;
    filter.SetMaskFilter(
        RSMaskFilter::CreateBlurMaskFilter(RSBlurType::SOLID, NormalizeToPx(blurMaskRadius_) * BLUR_SIGMA_FACTOR));
    pen.SetFilter(filter);
    pen.SetColor(RSColor(pathColor_.GetRed(), pathColor_.GetGreen(), pathColor_.GetBlue(), pathColor_.GetAlpha()));
    pen.SetAlpha(MAX_ALPHA);
    pen.SetAntiAlias(true);

    canvas->AttachPen(pen);
    canvas->DrawPath(path);
    canvas->DetachPen();

    RSBrush brush;
    brush.SetFilter(filter);
    brush.SetColor(RSColor(pathColor_.GetRed(), pathColor_.GetGreen(), pathColor_.GetBlue(), pathColor_.GetAlpha()));
    brush.SetAlpha(MAX_ALPHA);
    brush.SetAntiAlias(true);
    PaintGlow(canvas, brush, padding);

    canvas->RestoreToCount(depthOfSavedStack);
#endif
}

#ifndef USE_ROSEN_DRAWING
void RosenRenderFocusAnimation::PaintPhoneFocus(SkCanvas* skCanvas)
{
    double radiusX = 0.0;
    double radiusY = 0.0;
    if (!isIndented_) {
        double offsetValue = NormalizeToPx(Dimension(PHONE_FOCUS_OFFSET, DimensionUnit::VP)) * MULTIPLE_FACTOR;
        radiusX = NormalizeToPx(rrect_.GetCorner().bottomLeftRadius.GetX());
        radiusY = NormalizeToPx(rrect_.GetCorner().bottomLeftRadius.GetY());
        radiusX = NearZero(radiusX) ? 0.0 : radiusX + NormalizeToPx(Dimension(PHONE_FOCUS_OFFSET, DimensionUnit::VP));
        radiusY = NearZero(radiusX) ? 0.0 : radiusY + NormalizeToPx(Dimension(PHONE_FOCUS_OFFSET, DimensionUnit::VP));
        width_ = rrect_.Width() + offsetValue;
        height_ = rrect_.Height() + offsetValue;
        skCanvas->translate(
            offset_.GetX() - offsetValue / MULTIPLE_FACTOR, offset_.GetY() - offsetValue / MULTIPLE_FACTOR);
    } else {
        double offsetValue = NormalizeToPx(Dimension(PHONE_INDENTED_FOCUS_OFFSET, DimensionUnit::VP)) * MULTIPLE_FACTOR;
        radiusX = NormalizeToPx(rrect_.GetCorner().bottomLeftRadius.GetX());
        radiusY = NormalizeToPx(rrect_.GetCorner().bottomLeftRadius.GetY());
        radiusX = NearZero(radiusX)
                      ? 0.0
                      : radiusX - NormalizeToPx(Dimension(PHONE_INDENTED_FOCUS_OFFSET, DimensionUnit::VP));
        radiusY = NearZero(radiusX)
                      ? 0.0
                      : radiusY - NormalizeToPx(Dimension(PHONE_INDENTED_FOCUS_OFFSET, DimensionUnit::VP));
        width_ = rrect_.Width() - offsetValue;
        height_ = rrect_.Height() - offsetValue;
        skCanvas->translate(
            offset_.GetX() + offsetValue / MULTIPLE_FACTOR, offset_.GetY() + offsetValue / MULTIPLE_FACTOR);
    }
    SkPath skPath;
    skPath.addRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(0, 0, width_, height_), radiusX, radiusY));
    SkPaint paint;
    paint.setStrokeJoin(SkPaint::Join::kRound_Join);
    paint.setStrokeCap(SkPaint::Cap::kRound_Cap);
    paint.setStrokeWidth(NormalizeToPx(Dimension(PHONE_BOUNDARY_WIDTH, DimensionUnit::VP)));
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SkColorSetRGB(pathColor_.GetRed(), pathColor_.GetGreen(), pathColor_.GetBlue()));
    paint.setAntiAlias(true);
    skCanvas->drawPath(skPath, paint);
}
#else
void RosenRenderFocusAnimation::PaintPhoneFocus(RSCanvas* canvas)
{
    double radiusX = 0.0;
    double radiusY = 0.0;
    if (!isIndented_) {
        double offsetValue = NormalizeToPx(Dimension(PHONE_FOCUS_OFFSET, DimensionUnit::VP)) * MULTIPLE_FACTOR;
        radiusX = NormalizeToPx(rrect_.GetCorner().bottomLeftRadius.GetX());
        radiusY = NormalizeToPx(rrect_.GetCorner().bottomLeftRadius.GetY());
        radiusX = NearZero(radiusX) ? 0.0 : radiusX + NormalizeToPx(Dimension(PHONE_FOCUS_OFFSET, DimensionUnit::VP));
        radiusY = NearZero(radiusX) ? 0.0 : radiusY + NormalizeToPx(Dimension(PHONE_FOCUS_OFFSET, DimensionUnit::VP));
        width_ = rrect_.Width() + offsetValue;
        height_ = rrect_.Height() + offsetValue;
        canvas->Translate(
            offset_.GetX() - offsetValue / MULTIPLE_FACTOR, offset_.GetY() - offsetValue / MULTIPLE_FACTOR);
    } else {
        double offsetValue = NormalizeToPx(Dimension(PHONE_INDENTED_FOCUS_OFFSET, DimensionUnit::VP)) * MULTIPLE_FACTOR;
        radiusX = NormalizeToPx(rrect_.GetCorner().bottomLeftRadius.GetX());
        radiusY = NormalizeToPx(rrect_.GetCorner().bottomLeftRadius.GetY());
        radiusX = NearZero(radiusX)
                      ? 0.0
                      : radiusX - NormalizeToPx(Dimension(PHONE_INDENTED_FOCUS_OFFSET, DimensionUnit::VP));
        radiusY = NearZero(radiusX)
                      ? 0.0
                      : radiusY - NormalizeToPx(Dimension(PHONE_INDENTED_FOCUS_OFFSET, DimensionUnit::VP));
        width_ = rrect_.Width() - offsetValue;
        height_ = rrect_.Height() - offsetValue;
        canvas->Translate(
            offset_.GetX() + offsetValue / MULTIPLE_FACTOR, offset_.GetY() + offsetValue / MULTIPLE_FACTOR);
    }
    RSRecordingPath path;
    path.AddRoundRect(RSRect(0, 0, width_, height_), radiusX, radiusY);
    RSPen pen;
    pen.SetJoinStyle(RSPen::JoinStyle::ROUND_JOIN);
    pen.SetCapStyle(RSPen::CapStyle::ROUND_CAP);
    pen.SetWidth(NormalizeToPx(Dimension(PHONE_BOUNDARY_WIDTH, DimensionUnit::VP)));
    pen.SetColor(RSColor(pathColor_.GetRed(), pathColor_.GetGreen(), pathColor_.GetBlue(), 255));
    pen.SetAntiAlias(true);
    canvas->AttachPen(pen);
    canvas->DrawPath(path);
    canvas->DetachPen();
}
#endif

} // namespace OHOS::Ace
