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

#include "core/components/custom_paint/rosen_render_custom_paint.h"

#include <cmath>

#include "txt/paragraph_builder.h"
#include "txt/paragraph_style.h"
#include "txt/paragraph_txt.h"
#include "render_service_client/core/ui/rs_node.h"

#include "securec.h"

#ifndef USE_ROSEN_DRAWING
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/utils/SkBase64.h"
#include "include/utils/SkParsePath.h"
#endif

#include "base/geometry/dimension.h"
#include "base/i18n/localization.h"
#include "base/image/pixel_map.h"
#include "base/json/json_util.h"
#include "base/log/ace_trace.h"
#include "base/memory/ace_type.h"
#include "base/utils/linear_map.h"
#include "base/utils/measure_util.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "bridge/common/utils/utils.h"
#include "core/components/calendar/rosen_render_calendar.h"
#include "core/components/common/painter/rosen_decoration_painter.h"
#include "core/components/font/constants_converter.h"
#include "core/components/font/rosen_font_collection.h"
#include "core/components/text/text_theme.h"
#include "core/components_ng/render/adapter/skia_image.h"
#include "core/image/flutter_image_cache.h"
#include "core/image/image_cache.h"
#include "core/image/image_provider.h"
#include "core/pipeline/base/rosen_render_context.h"

namespace OHOS::Ace {
namespace {
constexpr double HANGING_PERCENT = 0.8;
constexpr double HALF_CIRCLE_ANGLE = 180.0;
constexpr double FULL_CIRCLE_ANGLE = 360.0;
constexpr int32_t IMAGE_CACHE_COUNT = 50;
constexpr double DEFAULT_QUALITY = 0.92;
constexpr int32_t MAX_LENGTH = 2048 * 2048;
const std::string UNSUPPORTED = "data:image/png";
const std::string URL_PREFIX = "data:";
const std::string URL_SYMBOL = ";base64,";
const std::string IMAGE_PNG = "image/png";
const std::string IMAGE_JPEG = "image/jpeg";
const std::string IMAGE_WEBP = "image/webp";
const std::u16string ELLIPSIS = u"\u2026";

// If args is empty or invalid format, use default: image/png
std::string GetMimeType(const std::string& args)
{
    // Args example: ["image/png"]
    std::vector<std::string> values;
    StringUtils::StringSplitter(args, '"', values);
    if (values.size() < 3) {
        return IMAGE_PNG;
    } else {
        // Convert to lowercase string.
        for (size_t i = 0; i < values[1].size(); ++i) {
            values[1][i] = static_cast<uint8_t>(tolower(values[1][i]));
        }
        return values[1];
    }
}

// Quality need between 0.0 and 1.0 for MimeType jpeg and webp
double GetQuality(const std::string& args)
{
    // Args example: ["image/jpeg", 0.8]
    std::vector<std::string> values;
    StringUtils::StringSplitter(args, ',', values);
    if (values.size() < 2) {
        return DEFAULT_QUALITY;
    }
    auto mimeType = GetMimeType(args);
    if (mimeType != IMAGE_JPEG && mimeType != IMAGE_WEBP) {
        return DEFAULT_QUALITY;
    }
    double quality = StringUtils::StringToDouble(values[1]);
    if (quality < 0.0 || quality > 1.0) {
        return DEFAULT_QUALITY;
    }
    return quality;
}

#ifndef USE_ROSEN_DRAWING
template<typename T, typename N>
N ConvertEnumToSkEnum(T key, const LinearEnumMapNode<T, N>* map, size_t length, N defaultValue)
{
    int64_t index = BinarySearchFindIndex(map, length, key);
    return index != -1 ? map[index].value : defaultValue;
}
#else
template<typename T, typename N>
N ConvertEnumToDrawingEnum(T key, const LinearEnumMapNode<T, N>* map, size_t length, N defaultValue)
{
    int64_t index = BinarySearchFindIndex(map, length, key);
    return index != -1 ? map[index].value : defaultValue;
}
#endif

#ifndef USE_ROSEN_DRAWING
const LinearEnumMapNode<CompositeOperation, SkBlendMode> SK_BLEND_MODE_TABLE[] = {
    { CompositeOperation::SOURCE_OVER, SkBlendMode::kSrcOver },
    { CompositeOperation::SOURCE_ATOP, SkBlendMode::kSrcATop },
    { CompositeOperation::SOURCE_IN, SkBlendMode::kSrcIn },
    { CompositeOperation::SOURCE_OUT, SkBlendMode::kSrcOut },
    { CompositeOperation::DESTINATION_OVER, SkBlendMode::kDstOver },
    { CompositeOperation::DESTINATION_ATOP, SkBlendMode::kDstATop },
    { CompositeOperation::DESTINATION_IN, SkBlendMode::kDstIn },
    { CompositeOperation::DESTINATION_OUT, SkBlendMode::kDstOut },
    { CompositeOperation::LIGHTER, SkBlendMode::kLighten },
    { CompositeOperation::COPY, SkBlendMode::kSrc },
    { CompositeOperation::XOR, SkBlendMode::kXor },
};
constexpr size_t BLEND_MODE_SIZE = ArraySize(SK_BLEND_MODE_TABLE);
#else
const LinearEnumMapNode<CompositeOperation, RSBlendMode> DRAWING_BLEND_MODE_TABLE[] = {
    { CompositeOperation::SOURCE_OVER, RSBlendMode::SRC_OVER },
    { CompositeOperation::SOURCE_ATOP, RSBlendMode::SRC_ATOP },
    { CompositeOperation::SOURCE_IN, RSBlendMode::SRC_IN },
    { CompositeOperation::SOURCE_OUT, RSBlendMode::SRC_OUT },
    { CompositeOperation::DESTINATION_OVER, RSBlendMode::DST_OVER },
    { CompositeOperation::DESTINATION_ATOP, RSBlendMode::DST_ATOP },
    { CompositeOperation::DESTINATION_IN, RSBlendMode::DST_IN },
    { CompositeOperation::DESTINATION_OUT, RSBlendMode::DST_OUT },
    { CompositeOperation::LIGHTER, RSBlendMode::LIGHTEN },
    { CompositeOperation::COPY, RSBlendMode::SRC },
    { CompositeOperation::XOR, RSBlendMode::XOR },
};
constexpr size_t BLEND_MODE_SIZE = ArraySize(DRAWING_BLEND_MODE_TABLE);
#endif

} // namespace

RosenRenderCustomPaint::RosenRenderCustomPaint()
{
    InitImageCallbacks();
}

RosenRenderCustomPaint::~RosenRenderCustomPaint() {}

void RosenRenderCustomPaint::CreateBitmap(double viewScale)
{
#ifndef USE_ROSEN_DRAWING
    auto imageInfo = SkImageInfo::Make(GetLayoutSize().Width() * viewScale, GetLayoutSize().Height() * viewScale,
        SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kUnpremul_SkAlphaType);
    canvasCache_.reset();
    cacheBitmap_.reset();
    canvasCache_.allocPixels(imageInfo);
    cacheBitmap_.allocPixels(imageInfo);
    canvasCache_.eraseColor(SK_ColorTRANSPARENT);
    cacheBitmap_.eraseColor(SK_ColorTRANSPARENT);
    skCanvas_ = std::make_unique<SkCanvas>(canvasCache_);
    cacheCanvas_ = std::make_unique<SkCanvas>(cacheBitmap_);
#else
    RSBitmapFormat format { RSCOLORTYPE_RGBA_8888,
        RSAlphaType::ALPHATYPE_UNPREMUL };
    canvasCache_.Reset();
    cacheBitmap_.Reset();
    canvasCache_.Build(GetLayoutSize().Width() * viewScale, GetLayoutSize().Height() * viewScale, format);
    cacheBitmap_.Build(GetLayoutSize().Width() * viewScale, GetLayoutSize().Height() * viewScale, format);
    canvasCache_.ClearWithColor(RSColor::COLOR_TRANSPARENT);
    cacheBitmap_.ClearWithColor(RSColor::COLOR_TRANSPARENT);
    drawingCanvas_->Bind(canvasCache_);
    cacheCanvas_->Bind(cacheBitmap_);
#endif
}

void RosenRenderCustomPaint::Paint(RenderContext& context, const Offset& offset)
{
    ACE_SCOPED_TRACE("RosenRenderCustomPaint::Paint");
    auto canvas = static_cast<RosenRenderContext*>(&context)->GetCanvas();
    if (auto rsNode = static_cast<RosenRenderContext*>(&context)->GetRSNode()) {
        rsNode->SetClipToFrame(true);
    }
    if (!canvas) {
        return;
    }
    auto pipeline = context_.Upgrade();
    if (!pipeline) {
        return;
    }
    // use physical pixel to store bitmap
    double viewScale = pipeline->GetViewScale();
    if (lastLayoutSize_ != GetLayoutSize()) {
        if (GetLayoutSize().IsInfinite()) {
            return;
        }
        CreateBitmap(viewScale);
        lastLayoutSize_ = GetLayoutSize();
    }
#ifndef USE_ROSEN_DRAWING
    if (!skCanvas_) {
        LOGE("skCanvas_ is null");
        return;
    }
    skCanvas_->scale(viewScale, viewScale);
    TriggerOnReadyEvent();

    for (const auto& task : tasks_) {
        task(*this, offset);
    }
    skCanvas_->scale(1.0 / viewScale, 1.0 / viewScale);
    tasks_.clear();

    canvas->save();
    canvas->scale(1.0 / viewScale, 1.0 / viewScale);
#ifndef NEW_SKIA
    canvas->drawBitmap(canvasCache_, 0.0f, 0.0f);
#else
    canvas->drawImage(canvasCache_.asImage(), 0.0f, 0.0f, SkSamplingOptions());
#endif
    canvas->restore();
#else
    if (!drawingCanvas_) {
        LOGE("drawingCanvas_ is null");
        return;
    }
    drawingCanvas_->Scale(viewScale, viewScale);
    TriggerOnReadyEvent();

    for (const auto& task : tasks_) {
        task(*this, offset);
    }
    drawingCanvas_->Scale(1.0 / viewScale, 1.0 / viewScale);
    tasks_.clear();

    canvas->Save();
    canvas->Scale(1.0 / viewScale, 1.0 / viewScale);
    canvas->DrawBitmap(canvasCache_, 0.0f, 0.0f);
    canvas->Restore();
#endif
}

#ifndef USE_ROSEN_DRAWING
SkPaint RosenRenderCustomPaint::GetStrokePaint()
{
    static const LinearEnumMapNode<LineJoinStyle, SkPaint::Join> skLineJoinTable[] = {
        { LineJoinStyle::MITER, SkPaint::Join::kMiter_Join },
        { LineJoinStyle::ROUND, SkPaint::Join::kRound_Join },
        { LineJoinStyle::BEVEL, SkPaint::Join::kBevel_Join },
    };
    static const LinearEnumMapNode<LineCapStyle, SkPaint::Cap> skLineCapTable[] = {
        { LineCapStyle::BUTT, SkPaint::Cap::kButt_Cap },
        { LineCapStyle::ROUND, SkPaint::Cap::kRound_Cap },
        { LineCapStyle::SQUARE, SkPaint::Cap::kSquare_Cap },
    };
    SkPaint paint;
    paint.setColor(strokeState_.GetColor().GetValue());
    paint.setStyle(SkPaint::Style::kStroke_Style);
    paint.setStrokeJoin(ConvertEnumToSkEnum(
        strokeState_.GetLineJoin(), skLineJoinTable, ArraySize(skLineJoinTable), SkPaint::Join::kMiter_Join));
    paint.setStrokeCap(ConvertEnumToSkEnum(
        strokeState_.GetLineCap(), skLineCapTable, ArraySize(skLineCapTable), SkPaint::Cap::kButt_Cap));
    paint.setStrokeWidth(static_cast<SkScalar>(strokeState_.GetLineWidth()));
    paint.setStrokeMiter(static_cast<SkScalar>(strokeState_.GetMiterLimit()));

    // set line Dash
    UpdateLineDash(paint);

    // set global alpha
    if (globalState_.HasGlobalAlpha()) {
        paint.setAlphaf(globalState_.GetAlpha());
    }
    return paint;
}
#else
RSPen RosenRenderCustomPaint::GetStrokePaint()
{
    static const LinearEnumMapNode<LineJoinStyle, RSPen::JoinStyle> skLineJoinTable[] = {
        { LineJoinStyle::MITER, RSPen::JoinStyle::MITER_JOIN },
        { LineJoinStyle::ROUND, RSPen::JoinStyle::ROUND_JOIN },
        { LineJoinStyle::BEVEL, RSPen::JoinStyle::BEVEL_JOIN },
    };
    static const LinearEnumMapNode<LineCapStyle, RSPen::CapStyle> skLineCapTable[] = {
        { LineCapStyle::BUTT, RSPen::CapStyle::BUTT_CAP },
        { LineCapStyle::ROUND, RSPen::CapStyle::ROUND_CAP },
        { LineCapStyle::SQUARE, RSPen::CapStyle::SQUARE_CAP },
    };
    RSPen pen;
    pen.SetColor(strokeState_.GetColor().GetValue());
    pen.SetJoinStyle(ConvertEnumToDrawingEnum(
        strokeState_.GetLineJoin(), skLineJoinTable,
        ArraySize(skLineJoinTable), RSPen::JoinStyle::MITER_JOIN));
    pen.SetCapStyle(ConvertEnumToDrawingEnum(
        strokeState_.GetLineCap(), skLineCapTable, ArraySize(skLineCapTable), RSPen::CapStyle::BUTT_CAP));
    pen.SetWidth(static_cast<RSScalar>(strokeState_.GetLineWidth()));
    pen.SetMiterLimit(static_cast<RSScalar>(strokeState_.GetMiterLimit()));

    // set line Dash
    UpdateLineDash(pen);

    // set global alpha
    if (globalState_.HasGlobalAlpha()) {
        pen.SetAlphaF(globalState_.GetAlpha());
    }
    return pen;
}
#endif

std::string RosenRenderCustomPaint::ToDataURL(const std::string& args)
{
    std::string mimeType = GetMimeType(args);
    double quality = GetQuality(args);
    double width = GetLayoutSize().Width();
    double height = GetLayoutSize().Height();
#ifndef USE_ROSEN_DRAWING
    SkBitmap tempCache;
    tempCache.allocPixels(SkImageInfo::Make(width, height, SkColorType::kBGRA_8888_SkColorType,
        (mimeType == IMAGE_JPEG) ? SkAlphaType::kOpaque_SkAlphaType : SkAlphaType::kUnpremul_SkAlphaType));
    bool isGpuEnable = false;
    bool success = false;
    if (!isGpuEnable) {
        if (canvasCache_.empty()) {
            LOGE("Bitmap is empty");
            return UNSUPPORTED;
        }
#ifndef NEW_SKIA
        success = canvasCache_.pixmap().scalePixels(tempCache.pixmap(), SkFilterQuality::kHigh_SkFilterQuality);
#else
        success = canvasCache_.pixmap().scalePixels(tempCache.pixmap(), SkSamplingOptions(SkCubicResampler { 1 / 3.0f, 1 / 3.0f }));
#endif
        if (!success) {
            LOGE("scalePixels failed when ToDataURL.");
            return UNSUPPORTED;
        }
    }
    SkPixmap src = tempCache.pixmap();
    SkDynamicMemoryWStream dst;
    if (mimeType == IMAGE_JPEG) {
        SkJpegEncoder::Options options;
        options.fQuality = quality * 100;
        success = SkJpegEncoder::Encode(&dst, src, options);
    } else if (mimeType == IMAGE_WEBP) {
        SkWebpEncoder::Options options;
        options.fQuality = quality * 100.0;
        success = SkWebpEncoder::Encode(&dst, src, options);
    } else {
        mimeType = IMAGE_PNG;
        SkPngEncoder::Options options;
        success = SkPngEncoder::Encode(&dst, src, options);
    }
    if (!success) {
        LOGE("Encode failed when ToDataURL.");
        return UNSUPPORTED;
    }
    auto result = dst.detachAsData();
    if (result == nullptr) {
        LOGE("DetachAsData failed when ToDataURL.");
        return UNSUPPORTED;
    }
    size_t len = SkBase64::Encode(result->data(), result->size(), nullptr);
    if (len > MAX_LENGTH) {
        LOGE("ToDataURL failed, image too large.");
        return UNSUPPORTED;
    }
    SkString info(len);
    SkBase64::Encode(result->data(), result->size(), info.writable_str());
#else
    // TODO Drawing : SkDynamicMemoryWStream SkWebpEncoder SkBase64
#endif
    return std::string(URL_PREFIX).append(mimeType).append(URL_SYMBOL).append(info.c_str());
}

void RosenRenderCustomPaint::SetAntiAlias(bool isEnabled)
{
    antiAlias_ = isEnabled;
}

void RosenRenderCustomPaint::TransferFromImageBitmap(const RefPtr<OffscreenCanvas>& offscreenCanvas)
{
    std::unique_ptr<ImageData> imageData =
        offscreenCanvas->GetImageData(0, 0, offscreenCanvas->GetWidth(), offscreenCanvas->GetHeight());
    ImageData* imageDataPtr = imageData.get();
    if (imageData != nullptr) {
        PutImageData(Offset(0, 0), *imageDataPtr);
    }
}

void RosenRenderCustomPaint::FillRect(const Offset& offset, const Rect& rect)
{
#ifndef USE_ROSEN_DRAWING
    SkPaint paint;
    paint.setAntiAlias(antiAlias_);
    paint.setColor(fillState_.GetColor().GetValue());
    paint.setStyle(SkPaint::Style::kFill_Style);
    SkRect skRect = SkRect::MakeLTRB(rect.Left() + offset.GetX(), rect.Top() + offset.GetY(),
        rect.Right() + offset.GetX(), offset.GetY() + rect.Bottom());
    if (HasShadow()) {
        SkPath path;
        path.addRect(skRect);
        RosenDecorationPainter::PaintShadow(path, shadow_, skCanvas_.get());
    }
    if (fillState_.GetGradient().IsValid()) {
        UpdatePaintShader(offset, paint, fillState_.GetGradient());
    }
    if (fillState_.GetPattern().IsValid()) {
        UpdatePaintShader(fillState_.GetPattern(), paint);
    }
    if (globalState_.HasGlobalAlpha()) {
        paint.setAlphaf(globalState_.GetAlpha()); // update the global alpha after setting the color
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        skCanvas_->drawRect(skRect, paint);
    } else {
        InitPaintBlend(cachePaint_);
        cacheCanvas_->drawRect(skRect, paint);
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, SkSamplingOptions(), &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
#else
    RSBrush brush;
    brush.SetAntiAlias(antiAlias_);
    brush.SetColor(fillState_.GetColor().GetValue());
    RSRect drawingRect(rect.Left() + offset.GetX(), rect.Top() + offset.GetY(),
        rect.Right() + offset.GetX(), offset.GetY() + rect.Bottom());
    if (HasShadow()) {
        RSRecordingPath path;
        path.AddRect(drawingRect);
        RosenDecorationPainter::PaintShadow(path, shadow_, drawingCanvas_.get());
    }
    if (fillState_.GetGradient().IsValid()) {
        UpdatePaintShader(offset, brush, fillState_.GetGradient());
    }
    if (fillState_.GetPattern().IsValid()) {
        UpdatePaintShader(fillState_.GetPattern(), brush);
    }
    if (globalState_.HasGlobalAlpha()) {
        paint.SetAlphaF(globalState_.GetAlpha()); // update the global alpha after setting the color
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        drawingCanvas_->AttachBrush(brush);
        drawingCanvas_->DrawRect(drawingRect);
        drawingCanvas_->DetachBrush();
    } else {
        InitPaintBlend(cacheBrush_);
        cacheCanvas_->AttachBrush(brush);
        cacheCanvas_->DrawRect(drawingRect);
        cacheCanvas_->DetachBrush();
        drawingCanvas_->DrawBitmap(cacheBitmap_, 0, 0);
        cacheBitmap_.ClearWithColor(RSColor::COLOR_TRANSPARENT);
    }
#endif
}

void RosenRenderCustomPaint::StrokeRect(const Offset& offset, const Rect& rect)
{
#ifndef USE_ROSEN_DRAWING
    SkPaint paint = GetStrokePaint();
    paint.setAntiAlias(antiAlias_);
    SkRect skRect = SkRect::MakeLTRB(rect.Left() + offset.GetX(), rect.Top() + offset.GetY(),
        rect.Right() + offset.GetX(), offset.GetY() + rect.Bottom());
    if (HasShadow()) {
        SkPath path;
        path.addRect(skRect);
        RosenDecorationPainter::PaintShadow(path, shadow_, skCanvas_.get());
    }
    if (strokeState_.GetGradient().IsValid()) {
        UpdatePaintShader(offset, paint, strokeState_.GetGradient());
    }
    if (strokeState_.GetPattern().IsValid()) {
        UpdatePaintShader(strokeState_.GetPattern(), paint);
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        skCanvas_->drawRect(skRect, paint);
    } else {
        InitPaintBlend(cachePaint_);
        cacheCanvas_->drawRect(skRect, paint);
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, SkSamplingOptions(), &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
#else
    RSPen pen = GetStrokePaint();
    pen.SetAntiAlias(antiAlias_);
    RSRect drawingRect(rect.Left() + offset.GetX(), rect.Top() + offset.GetY(),
        rect.Right() + offset.GetX(), offset.GetY() + rect.Bottom());
    if (HasShadow()) {
        RSRecordingPath path;
        path.AddRect(drawingRect);
        RosenDecorationPainter::PaintShadow(path, shadow_, drawingCanvas_);
    }
    if (strokeState_.GetGradient().IsValid()) {
        UpdatePaintShader(offset, pen, strokeState_.GetGradient());
    }
    if (strokeState_.GetPattern().IsValid()) {
        UpdatePaintShader(strokeState_.GetPattern(), pen);
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        drawingCanvas_->AttachPen(pen);
        drawingCanvas_->DrawRect(drawingRect);
        drawingCanvas_->DetachPen();
    } else {
        InitPaintBlend(cacheBrush_);
        cacheCanvas_->AttachPen(pen);
        cacheCanvas_->DrawRect(drawingRect);
        cacheCanvas_->DetachPen();
        drawingCanvas_->DrawBitmap(cacheBitmap_, 0, 0);
        cacheBitmap_.ClearWithColor(RSColor::COLOR_TRANSPARENT);
    }
#endif
}

void RosenRenderCustomPaint::ClearRect(const Offset& offset, const Rect& rect)
{
#ifndef USE_ROSEN_DRAWING
    SkPaint paint;
    paint.setAntiAlias(antiAlias_);
    paint.setBlendMode(SkBlendMode::kClear);
    auto skRect = SkRect::MakeLTRB(rect.Left() + offset.GetX(), rect.Top() + offset.GetY(),
        rect.Right() + offset.GetX(), rect.Bottom() + offset.GetY());
    skCanvas_->drawRect(skRect, paint);
#else
    RSBrush brush;
    brush.SetAntiAlias(antiAlias_);
    brush.SetBlendMode(RSBlendMode::CLEAR);
    RSRect drawingRect(rect.Left() + offset.GetX(), rect.Top() + offset.GetY(),
        rect.Right() + offset.GetX(), rect.Bottom() + offset.GetY());
    drawingCanvas_->Attachbrush(brush);
    drawingCanvas_->DrawRect(drawingRect);
    drawingCanvas_->DetachBrush();
#endif
}

void RosenRenderCustomPaint::FillText(const Offset& offset, const std::string& text, double x, double y)
{
    if (!UpdateParagraph(offset, text, false, HasShadow())) {
        return;
    }
    PaintText(offset, x, y, false, HasShadow());
}

void RosenRenderCustomPaint::StrokeText(const Offset& offset, const std::string& text, double x, double y)
{
    if (HasShadow()) {
        if (!UpdateParagraph(offset, text, true, true)) {
            return;
        }
        PaintText(offset, x, y, true, true);
    }

    if (!UpdateParagraph(offset, text, true)) {
        return;
    }
    PaintText(offset, x, y, true);
}

double RosenRenderCustomPaint::MeasureTextInner(const MeasureContext& context)
{
    using namespace Constants;
    txt::ParagraphStyle style;
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    if (!fontCollection) {
        LOGW("fontCollection is null");
        return 0.0;
    }
    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
    std::vector<std::string> fontFamilies;
    if (context.fontSize) {
        txtStyle.font_size = context.fontSize.value().ConvertToPx();
    } else {
        auto context = PipelineBase::GetCurrentContext();
        auto textTheme = context->GetTheme<TextTheme>();
        txtStyle.font_size = textTheme->GetTextStyle().GetFontSize().ConvertToPx();
    }
    txtStyle.font_style = ConvertTxtFontStyle(context.fontStyle);
    FontWeight fontWeightStr = StringUtils::StringToFontWeight(context.fontWeight);
    txtStyle.font_weight = ConvertTxtFontWeight(fontWeightStr);
    StringUtils::StringSplitter(context.fontFamily, ',', fontFamilies);
    txtStyle.font_families = fontFamilies;
    if (context.letterSpacing.has_value()) {
        txtStyle.letter_spacing = context.letterSpacing.value().ConvertToPx();
    }

    builder->PushStyle(txtStyle);
    builder->AddText(StringUtils::Str8ToStr16(context.textContent));
    auto paragraph = builder->Build();
    if (!paragraph) {
        return 0.0;
    }
    paragraph->Layout(Size::INFINITE_SIZE);
    return std::ceil(paragraph->GetLongestLine());
}

Size RosenRenderCustomPaint::MeasureTextSizeInner(const MeasureContext& context)
{
    using namespace Constants;
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    if (!fontCollection) {
        LOGW("fontCollection is null");
        return Size(0.0, 0.0);
    }
    txt::ParagraphStyle style;
    style.text_align = ConvertTxtTextAlign(context.textAlign);
    if (context.textOverlayFlow == TextOverflow::ELLIPSIS) {
        style.ellipsis = ELLIPSIS;
    }
    if (GreatNotEqual(context.maxlines, 0.0)) {
        style.max_lines = context.maxlines;
    }

    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
    std::vector<std::string> fontFamilies;
    if (context.fontSize.has_value()) {
        txtStyle.font_size = context.fontSize.value().ConvertToPx();
    } else {
        auto context = PipelineBase::GetCurrentContext();
        auto textTheme = context->GetTheme<TextTheme>();
        txtStyle.font_size = textTheme->GetTextStyle().GetFontSize().ConvertToPx();
    }
    txtStyle.font_style = ConvertTxtFontStyle(context.fontStyle);
    FontWeight fontWeightStr = StringUtils::StringToFontWeight(context.fontWeight);
    txtStyle.font_weight = ConvertTxtFontWeight(fontWeightStr);
    StringUtils::StringSplitter(context.fontFamily, ',', fontFamilies);
    txtStyle.font_families = fontFamilies;
    if (context.letterSpacing.has_value()) {
        txtStyle.letter_spacing = context.letterSpacing.value().ConvertToPx();
    }
    if (context.lineHeight.has_value()) {
        if (context.lineHeight->Unit() == DimensionUnit::PERCENT) {
            txtStyle.has_height_override = true;
            txtStyle.height = context.lineHeight->Value();
        } else {
            auto lineHeight = context.lineHeight.value().ConvertToPx();
            if (!NearEqual(lineHeight, txtStyle.font_size) && (lineHeight > 0.0) && (!NearZero(txtStyle.font_size))) {
                txtStyle.height = lineHeight / txtStyle.font_size;
                txtStyle.has_height_override = true;
            }
        }
    }
    builder->PushStyle(txtStyle);
    std::string content = context.textContent;
    StringUtils::TransformStrCase(content, static_cast<int32_t>(context.textCase));
    builder->AddText(StringUtils::Str8ToStr16(content));
    auto paragraph = builder->Build();
    if (!paragraph) {
        return Size(0.0, 0.0);
    }
    if (context.constraintWidth.has_value()) {
        paragraph->Layout(context.constraintWidth.value().ConvertToPx());
    } else {
        paragraph->Layout(Size::INFINITE_SIZE);
    }
    double textWidth = 0.0;
    auto* paragraphTxt = static_cast<txt::ParagraphTxt*>(paragraph.get());
    if (paragraphTxt->GetLineCount() == 1) {
        textWidth = std::max(paragraph->GetLongestLine(), paragraph->GetMaxIntrinsicWidth());
    } else {
        textWidth = paragraph->GetLongestLine();
    }
    auto sizeWidth = std::min(paragraph->GetMaxWidth(), textWidth);
    sizeWidth =
        context.constraintWidth.has_value() ? context.constraintWidth.value().ConvertToPx() : std::ceil(sizeWidth);

    float baselineOffset = 0.0;
    if (context.baselineOffset.has_value()) {
        baselineOffset = static_cast<float>(context.baselineOffset.value().ConvertToPx());
    }
    float heightFinal = static_cast<float>(paragraph->GetHeight()) + std::fabs(baselineOffset);

    return Size(sizeWidth, heightFinal);
}

double RosenRenderCustomPaint::MeasureText(const std::string& text, const PaintState& state)
{
    using namespace Constants;
    txt::ParagraphStyle style;
    style.text_align = ConvertTxtTextAlign(state.GetTextAlign());
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    if (!fontCollection) {
        LOGW("MeasureText: fontCollection is null");
        return 0.0;
    }
    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
    ConvertTxtStyle(state.GetTextStyle(), context_, txtStyle);
    txtStyle.font_size = state.GetTextStyle().GetFontSize().Value();
    builder->PushStyle(txtStyle);
    builder->AddText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->Build();
    paragraph->Layout(Size::INFINITE_SIZE);
    return paragraph->GetMaxIntrinsicWidth();
}

double RosenRenderCustomPaint::MeasureTextHeight(const std::string& text, const PaintState& state)
{
    using namespace Constants;
    txt::ParagraphStyle style;
    style.text_align = ConvertTxtTextAlign(state.GetTextAlign());
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    if (!fontCollection) {
        LOGW("MeasureText: fontCollection is null");
        return 0.0;
    }
    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
    ConvertTxtStyle(state.GetTextStyle(), context_, txtStyle);
    txtStyle.font_size = state.GetTextStyle().GetFontSize().Value();
    builder->PushStyle(txtStyle);
    builder->AddText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->Build();
    paragraph->Layout(Size::INFINITE_SIZE);
    return paragraph->GetHeight();
}

TextMetrics RosenRenderCustomPaint::MeasureTextMetrics(const std::string& text, const PaintState& state)
{
    using namespace Constants;
    txt::ParagraphStyle style;
    style.text_align = ConvertTxtTextAlign(state.GetTextAlign());
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    if (!fontCollection) {
        LOGW("MeasureText: fontCollection is null");
        return { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    }
    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
    ConvertTxtStyle(state.GetTextStyle(), context_, txtStyle);
    txtStyle.font_size = state.GetTextStyle().GetFontSize().Value();
    builder->PushStyle(txtStyle);
    builder->AddText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->Build();
    paragraph->Layout(Size::INFINITE_SIZE);

    auto textAlign = state.GetTextAlign();
    auto textBaseLine = state.GetTextStyle().GetTextBaseline();

    auto width = paragraph->GetMaxIntrinsicWidth();
    auto height = paragraph->GetHeight();

    auto actualBoundingBoxLeft = -GetAlignOffset(textAlign, paragraph);
    auto actualBoundingBoxRight = width - actualBoundingBoxLeft;
    auto actualBoundingBoxAscent = -GetBaselineOffset(textBaseLine, paragraph);
    auto actualBoundingBoxDescent = height - actualBoundingBoxAscent;

    return { width, height, actualBoundingBoxLeft, actualBoundingBoxRight, actualBoundingBoxAscent,
        actualBoundingBoxDescent };
}

void RosenRenderCustomPaint::PaintText(const Offset& offset, double x, double y, bool isStroke, bool hasShadow)
{
    paragraph_->Layout(GetLayoutSize().Width());
    if (GetLayoutSize().Width() > paragraph_->GetMaxIntrinsicWidth()) {
        paragraph_->Layout(std::ceil(paragraph_->GetMaxIntrinsicWidth()));
    }
    auto align = isStroke ? strokeState_.GetTextAlign() : fillState_.GetTextAlign();
    double dx = offset.GetX() + x + GetAlignOffset(align, paragraph_);
    auto baseline =
        isStroke ? strokeState_.GetTextStyle().GetTextBaseline() : fillState_.GetTextStyle().GetTextBaseline();
    double dy = offset.GetY() + y + GetBaselineOffset(baseline, paragraph_);

#ifndef USE_ROSEN_DRAWING
    if (hasShadow) {
        skCanvas_->save();
        auto shadowOffsetX = shadow_.GetOffset().GetX();
        auto shadowOffsetY = shadow_.GetOffset().GetY();
        paragraph_->Paint(skCanvas_.get(), dx + shadowOffsetX, dy + shadowOffsetY);
        skCanvas_->restore();
        return;
    }

    paragraph_->Paint(skCanvas_.get(), dx, dy);
#else
    if (hasShadow) {
        drawingCanvas_->Save();
        auto shadowOffsetX = shadow_.GetOffset().GetX();
        auto shadowOffsetY = shadow_.GetOffset().GetY();
        paragraph_->Paint(drawingCanvas_->GetCanvasData()->ExportSkCanvas(), dx + shadowOffsetX, dy + shadowOffsetY);
        drawingCanvas_->Restore();
        return;
    }

    paragraph_->Paint(drawingCanvas_->GetCanvasData()->ExportSkCanvas(), dx, dy);
#endif
}

double RosenRenderCustomPaint::GetAlignOffset(TextAlign align, std::unique_ptr<txt::Paragraph>& paragraph)
{
    double x = 0.0;
    switch (align) {
        case TextAlign::LEFT:
            x = 0.0;
            break;
        case TextAlign::START:
            x = (GetTextDirection() == TextDirection::LTR) ? 0.0 : -paragraph->GetMaxIntrinsicWidth();
            break;
        case TextAlign::RIGHT:
            x = -paragraph->GetMaxIntrinsicWidth();
            break;
        case TextAlign::END:
            x = (GetTextDirection() == TextDirection::LTR) ? -paragraph->GetMaxIntrinsicWidth() : 0.0;
            break;
        case TextAlign::CENTER:
            x = -paragraph->GetMaxIntrinsicWidth() / 2;
            break;
        default:
            x = 0.0;
            break;
    }
    return x;
}

double RosenRenderCustomPaint::GetBaselineOffset(TextBaseline baseline, std::unique_ptr<txt::Paragraph>& paragraph)
{
    double y = 0.0;
    switch (baseline) {
        case TextBaseline::ALPHABETIC:
            y = -paragraph->GetAlphabeticBaseline();
            break;
        case TextBaseline::IDEOGRAPHIC:
            y = -paragraph->GetIdeographicBaseline();
            break;
        case TextBaseline::BOTTOM:
            y = -paragraph->GetHeight();
            break;
        case TextBaseline::TOP:
            y = 0.0;
            break;
        case TextBaseline::MIDDLE:
            y = -paragraph->GetHeight() / 2;
            break;
        case TextBaseline::HANGING:
            y = -HANGING_PERCENT * (paragraph->GetHeight() - paragraph->GetAlphabeticBaseline());
            break;
        default:
            y = -paragraph->GetAlphabeticBaseline();
            break;
    }
    return y;
}

void RosenRenderCustomPaint::MoveTo(const Offset& offset, double x, double y)
{
#ifndef USE_ROSEN_DRAWING
    skPath_.moveTo(SkDoubleToScalar(x + offset.GetX()), SkDoubleToScalar(y + offset.GetY()));
#else
    drawingPath_.MoveTo(
        static_cast<RSScalar>(x + offset.GetX()), static_cast<RSScalar>(y + offset.GetY()));
#endif
}

void RosenRenderCustomPaint::LineTo(const Offset& offset, double x, double y)
{
#ifndef USE_ROSEN_DRAWING
    skPath_.lineTo(SkDoubleToScalar(x + offset.GetX()), SkDoubleToScalar(y + offset.GetY()));
#else
    drawingPath_.LineTo(
        static_cast<RSScalar>(x + offset.GetX()), static_cast<RSScalar>(y + offset.GetY()));
#endif
}

void RosenRenderCustomPaint::BezierCurveTo(const Offset& offset, const BezierCurveParam& param)
{
#ifndef USE_ROSEN_DRAWING
    skPath_.cubicTo(SkDoubleToScalar(param.cp1x + offset.GetX()), SkDoubleToScalar(param.cp1y + offset.GetY()),
        SkDoubleToScalar(param.cp2x + offset.GetX()), SkDoubleToScalar(param.cp2y + offset.GetY()),
        SkDoubleToScalar(param.x + offset.GetX()), SkDoubleToScalar(param.y + offset.GetY()));
#else
    drawingPath_.CubicTo(static_cast<RSScalar>(param.cp1x + offset.GetX()),
        static_cast<RSScalar>(param.cp1y + offset.GetY()),
        static_cast<RSScalar>(param.cp2x + offset.GetX()),
        static_cast<RSScalar>(param.cp2y + offset.GetY()),
        static_cast<RSScalar>(param.x + offset.GetX()),
        static_cast<RSScalar>(param.y + offset.GetY()));
#endif
}

void RosenRenderCustomPaint::QuadraticCurveTo(const Offset& offset, const QuadraticCurveParam& param)
{
#ifndef USE_ROSEN_DRAWING
    skPath_.quadTo(SkDoubleToScalar(param.cpx + offset.GetX()), SkDoubleToScalar(param.cpy + offset.GetY()),
        SkDoubleToScalar(param.x + offset.GetX()), SkDoubleToScalar(param.y + offset.GetY()));
#else
    drawingPath_.QuadTo(static_cast<RSScalar>(param.cpx + offset.GetX()),
        static_cast<RSScalar>(param.cpy + offset.GetY()),
        static_cast<RSScalar>(param.x + offset.GetX()),
        static_cast<RSScalar>(param.y + offset.GetY()));
#endif
}

void RosenRenderCustomPaint::Arc(const Offset& offset, const ArcParam& param)
{
    double left = param.x - param.radius + offset.GetX();
    double top = param.y - param.radius + offset.GetY();
    double right = param.x + param.radius + offset.GetX();
    double bottom = param.y + param.radius + offset.GetY();
    double startAngle = param.startAngle * HALF_CIRCLE_ANGLE / M_PI;
    double endAngle = param.endAngle * HALF_CIRCLE_ANGLE / M_PI;
    double sweepAngle = endAngle - startAngle;
    if (param.anticlockwise) {
        sweepAngle =
            endAngle > startAngle ? (std::fmod(sweepAngle, FULL_CIRCLE_ANGLE) - FULL_CIRCLE_ANGLE) : sweepAngle;
    } else {
        sweepAngle =
            endAngle > startAngle ? sweepAngle : (std::fmod(sweepAngle, FULL_CIRCLE_ANGLE) + FULL_CIRCLE_ANGLE);
    }
#ifndef USE_ROSEN_DRAWING
    auto rect = SkRect::MakeLTRB(left, top, right, bottom);
    if (NearEqual(std::fmod(sweepAngle, FULL_CIRCLE_ANGLE), 0.0) && !NearEqual(startAngle, endAngle)) {
        // draw circle
        double half = GreatNotEqual(sweepAngle, 0.0) ? HALF_CIRCLE_ANGLE : -HALF_CIRCLE_ANGLE;
        skPath_.arcTo(rect, SkDoubleToScalar(startAngle), SkDoubleToScalar(half), false);
        skPath_.arcTo(rect, SkDoubleToScalar(half + startAngle), SkDoubleToScalar(half), false);
    } else if (!NearEqual(std::fmod(sweepAngle, FULL_CIRCLE_ANGLE), 0.0) && abs(sweepAngle) > FULL_CIRCLE_ANGLE) {
        double half = GreatNotEqual(sweepAngle, 0.0) ? HALF_CIRCLE_ANGLE : -HALF_CIRCLE_ANGLE;
        skPath_.arcTo(rect, SkDoubleToScalar(startAngle), SkDoubleToScalar(half), false);
        skPath_.arcTo(rect, SkDoubleToScalar(half + startAngle), SkDoubleToScalar(half), false);
        skPath_.arcTo(rect, SkDoubleToScalar(half + half + startAngle), SkDoubleToScalar(sweepAngle), false);
    } else {
        skPath_.arcTo(rect, SkDoubleToScalar(startAngle), SkDoubleToScalar(sweepAngle), false);
    }
#else
    RSRect rect(left, top, right, bottom);
    if (NearEqual(std::fmod(sweepAngle, FULL_CIRCLE_ANGLE), 0.0) && !NearEqual(startAngle, endAngle)) {
        // draw circle
        double half = GreatNotEqual(sweepAngle, 0.0) ? HALF_CIRCLE_ANGLE : -HALF_CIRCLE_ANGLE;
        drawingPath_.ArcTo(
            rect, static_cast<RSScalar>(startAngle), static_cast<RSScalar>(half), false);
        drawingPath_.ArcTo(rect, static_cast<RSScalar>(half + startAngle),
            static_cast<RSScalar>(half), false);
    } else if (!NearEqual(std::fmod(sweepAngle, FULL_CIRCLE_ANGLE), 0.0) && abs(sweepAngle) > FULL_CIRCLE_ANGLE) {
        double half = GreatNotEqual(sweepAngle, 0.0) ? HALF_CIRCLE_ANGLE : -HALF_CIRCLE_ANGLE;
        drawingPath_.ArcTo(
            rect, static_cast<RSScalar>(startAngle), static_cast<RSScalar>(half), false);
        drawingPath_.ArcTo(rect, static_cast<RSScalar>(half + startAngle),
            static_cast<RSScalar>(half), false);
        drawingPath_.ArcTo(rect, static_cast<RSScalar>(half + half + startAngle),
            static_cast<RSScalar>(sweepAngle), false);
    } else {
        drawingPath_.ArcTo(rect, static_cast<RSScalar>(startAngle),
            static_cast<RSScalar>(sweepAngle), false);
    }
#endif
}

void RosenRenderCustomPaint::ArcTo(const Offset& offset, const ArcToParam& param)
{
    double x1 = param.x1 + offset.GetX();
    double y1 = param.y1 + offset.GetY();
    double x2 = param.x2 + offset.GetX();
    double y2 = param.y2 + offset.GetY();
    double radius = param.radius;
#ifndef USE_ROSEN_DRAWING
    skPath_.arcTo(SkDoubleToScalar(x1), SkDoubleToScalar(y1), SkDoubleToScalar(x2), SkDoubleToScalar(y2),
        SkDoubleToScalar(radius));
#else
    drawingPath_.ArcTo(static_cast<RSScalar>(x1), static_cast<RSScalar>(y1),
        static_cast<RSScalar>(x2), static_cast<RSScalar>(y2),
        static_cast<RSScalar>(radius));
#endif
}

void RosenRenderCustomPaint::Ellipse(const Offset& offset, const EllipseParam& param)
{
    // Init the start and end angle, then calculated the sweepAngle.
    double startAngle = std::fmod(param.startAngle, M_PI * 2.0);
    double endAngle = std::fmod(param.endAngle, M_PI * 2.0);
    startAngle = (startAngle < 0.0 ? startAngle + M_PI * 2.0 : startAngle) * HALF_CIRCLE_ANGLE / M_PI;
    endAngle = (endAngle < 0.0 ? endAngle + M_PI * 2.0 : endAngle) * HALF_CIRCLE_ANGLE / M_PI;
    if (NearEqual(param.startAngle, param.endAngle)) {
        return; // Just return when startAngle is same as endAngle.
    }
    double rotation = param.rotation * HALF_CIRCLE_ANGLE / M_PI;
    double sweepAngle = endAngle - startAngle;
    if (param.anticlockwise) {
        if (sweepAngle > 0.0) { // Make sure the sweepAngle is negative when anticlockwise.
            sweepAngle -= FULL_CIRCLE_ANGLE;
        }
    } else {
        if (sweepAngle < 0.0) { // Make sure the sweepAngle is positive when clockwise.
            sweepAngle += FULL_CIRCLE_ANGLE;
        }
    }

    // Init the oval Rect(left, top, right, bottom).
    double left = param.x - param.radiusX + offset.GetX();
    double top = param.y - param.radiusY + offset.GetY();
    double right = param.x + param.radiusX + offset.GetX();
    double bottom = param.y + param.radiusY + offset.GetY();
#ifndef USE_ROSEN_DRAWING
    auto rect = SkRect::MakeLTRB(left, top, right, bottom);
    if (!NearZero(rotation)) {
        SkMatrix matrix;
        matrix.setRotate(-rotation, param.x + offset.GetX(), param.y + offset.GetY());
        skPath_.transform(matrix);
    }
    if (NearZero(sweepAngle) && !NearZero(param.endAngle - param.startAngle)) {
        // The entire ellipse needs to be drawn with two arcTo.
        skPath_.arcTo(rect, startAngle, HALF_CIRCLE_ANGLE, false);
        skPath_.arcTo(rect, startAngle + HALF_CIRCLE_ANGLE, HALF_CIRCLE_ANGLE, false);
    } else {
        skPath_.arcTo(rect, startAngle, sweepAngle, false);
    }
    if (!NearZero(rotation)) {
        SkMatrix matrix;
        matrix.setRotate(rotation, param.x + offset.GetX(), param.y + offset.GetY());
        skPath_.transform(matrix);
    }
#else
    auto rect = RSRect(left, top, right, bottom);
    if (!NearZero(rotation)) {
        RSMatrix matrix;
        matrix.Rotate(-rotation, param.x + offset.GetX(), param.y + offset.GetY());
        drawingPath_.Transform(matrix);
    }
    if (NearZero(sweepAngle) && !NearZero(param.endAngle - param.startAngle)) {
        // The entire ellipse needs to be drawn with two arcTo.
        drawingPath_.ArcTo(rect, startAngle, HALF_CIRCLE_ANGLE, false);
        drawingPath_.ArcTo(rect, startAngle + HALF_CIRCLE_ANGLE, HALF_CIRCLE_ANGLE, false);
    } else {
        drawingPath_.ArcTo(rect, startAngle, sweepAngle, false);
    }
    if (!NearZero(rotation)) {
        RSMatrix matrix;
        matrix.Rotate(rotation, param.x + offset.GetX(), param.y + offset.GetY());
        drawingPath_.Transform(matrix);
    }
#endif
}

void RosenRenderCustomPaint::AddRect(const Offset& offset, const Rect& rect)
{
#ifndef USE_ROSEN_DRAWING
    SkRect skRect = SkRect::MakeLTRB(rect.Left() + offset.GetX(), rect.Top() + offset.GetY(),
        rect.Right() + offset.GetX(), offset.GetY() + rect.Bottom());
    skPath_.addRect(skRect);
#else
    RSRect drawingRect(rect.Left() + offset.GetX(), rect.Top() + offset.GetY(),
        rect.Right() + offset.GetX(), offset.GetY() + rect.Bottom());
    drawingPath_.AddRect(drawingRect);
#endif
}

void RosenRenderCustomPaint::SetFillRuleForPath(const CanvasFillRule& rule)
{
#ifndef USE_ROSEN_DRAWING
    if (rule == CanvasFillRule::NONZERO) {
#ifndef NEW_SKIA
        skPath_.setFillType(SkPath::FillType::kWinding_FillType);
#else
        skPath_.setFillType(SkPathFillType::kWinding);
#endif
    } else if (rule == CanvasFillRule::EVENODD) {
#ifndef NEW_SKIA
        skPath_.setFillType(SkPath::FillType::kEvenOdd_FillType);
#else
        skPath_.setFillType(SkPathFillType::kEvenOdd);
#endif
    }
#else
    if (rule == CanvasFillRule::NONZERO) {
        drawingPath_.SetFillStyle(RSPathFillType::WINDING);
    } else if (rule == CanvasFillRule::EVENODD) {
        drawingPath_.SetFillStyle(RSPathFillType::EVEN_ODD);
    }
#endif
}

void RosenRenderCustomPaint::SetFillRuleForPath2D(const CanvasFillRule& rule)
{
#ifndef USE_ROSEN_DRAWING
    if (rule == CanvasFillRule::NONZERO) {
#ifndef NEW_SKIA
        skPath2d_.setFillType(SkPath::FillType::kWinding_FillType);
#else
        skPath_.setFillType(SkPathFillType::kWinding);
#endif
    } else if (rule == CanvasFillRule::EVENODD) {
#ifndef NEW_SKIA
        skPath2d_.setFillType(SkPath::FillType::kEvenOdd_FillType);
#else
        skPath_.setFillType(SkPathFillType::kEvenOdd);
#endif
    }
#else
    if (rule == CanvasFillRule::NONZERO) {
        drawingPath2d_.SetFillStyle(RSPathFillType::WINDING);
    } else if (rule == CanvasFillRule::EVENODD) {
        drawingPath2d_.SetFillStyle(RSPathFillType::EVEN_ODD);
    }
#endif
}

void RosenRenderCustomPaint::ParsePath2D(const Offset& offset, const RefPtr<CanvasPath2D>& path)
{
    for (const auto& [cmd, args] : path->GetCaches()) {
        switch (cmd) {
            case PathCmd::CMDS: {
                Path2DAddPath(offset, args);
                break;
            }
            case PathCmd::TRANSFORM: {
                Path2DSetTransform(offset, args);
                break;
            }
            case PathCmd::MOVE_TO: {
                Path2DMoveTo(offset, args);
                break;
            }
            case PathCmd::LINE_TO: {
                Path2DLineTo(offset, args);
                break;
            }
            case PathCmd::ARC: {
                Path2DArc(offset, args);
                break;
            }
            case PathCmd::ARC_TO: {
                Path2DArcTo(offset, args);
                break;
            }
            case PathCmd::QUADRATIC_CURVE_TO: {
                Path2DQuadraticCurveTo(offset, args);
                break;
            }
            case PathCmd::BEZIER_CURVE_TO: {
                Path2DBezierCurveTo(offset, args);
                break;
            }
            case PathCmd::ELLIPSE: {
                Path2DEllipse(offset, args);
                break;
            }
            case PathCmd::RECT: {
                Path2DRect(offset, args);
                break;
            }
            case PathCmd::CLOSE_PATH: {
                Path2DClosePath(offset, args);
                break;
            }
            default: {
                break;
            }
        }
    }
}

void RosenRenderCustomPaint::Fill(const Offset& offset)
{
#ifndef USE_ROSEN_DRAWING
    SkPaint paint;
    paint.setAntiAlias(antiAlias_);
    paint.setColor(fillState_.GetColor().GetValue());
    paint.setStyle(SkPaint::Style::kFill_Style);
    if (HasShadow()) {
        RosenDecorationPainter::PaintShadow(skPath_, shadow_, skCanvas_.get());
    }
    if (fillState_.GetGradient().IsValid()) {
        UpdatePaintShader(offset, paint, fillState_.GetGradient());
    }
    if (fillState_.GetPattern().IsValid()) {
        UpdatePaintShader(fillState_.GetPattern(), paint);
    }
    if (globalState_.HasGlobalAlpha()) {
        paint.setAlphaf(globalState_.GetAlpha());
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        skCanvas_->drawPath(skPath_, paint);
    } else {
        InitPaintBlend(cachePaint_);
        cacheCanvas_->drawPath(skPath_, paint);
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, SkSamplingOptions(), &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
#else
    RSBrush brush;
    brush.SetAntiAlias(antiAlias_);
    brush.SetColor(fillState_.GetColor().GetValue());
    if (HasShadow()) {
        RosenDecorationPainter::PaintShadow(drawingPath_, shadow_, drawingCanvas_);
    }
    if (fillState_.GetGradient().IsValid()) {
        UpdatePaintShader(offset, brush, fillState_.GetGradient());
    }
    if (fillState_.GetPattern().IsValid()) {
        UpdatePaintShader(fillState_.GetPattern(), brush);
    }
    if (globalState_.HasGlobalAlpha()) {
        brush.SetAlphaF(globalState_.GetAlpha());
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        drawingCanvas_->AttachBrush(brush);
        drawingCanvas_->DrawPath(drawingPath_);
        drawingCanvas_->DetachBrush();
    } else {
        InitPaintBlend(cacheBrush_);
        cacheCanvas_->AttachBrush(brush);
        cacheCanvas_->DrawPath(drawingPath_);
        cacheCanvas_->DetachBrush();
        drawingCanvas_->DrawBitmap(cacheBitmap_, 0, 0);
        cacheBitmap_.ClearWithColor(RSColor::COLOR_TRANSPARENT);
    }
#endif
}

void RosenRenderCustomPaint::Fill(const Offset& offset, const RefPtr<CanvasPath2D>& path)
{
    if (path == nullptr) {
        LOGE("Fill failed, target path is null.");
        return;
    }
    ParsePath2D(offset, path);
    Path2DFill(offset);
#ifndef USE_ROSEN_DRAWING
    skPath2d_.reset();
#else
    drawingPath2d_.Reset();
#endif
}

void RosenRenderCustomPaint::Stroke(const Offset& offset)
{
#ifndef USE_ROSEN_DRAWING
    SkPaint paint = GetStrokePaint();
    paint.setAntiAlias(antiAlias_);
    if (HasShadow()) {
        RosenDecorationPainter::PaintShadow(skPath_, shadow_, skCanvas_.get());
    }
    if (strokeState_.GetGradient().IsValid()) {
        UpdatePaintShader(offset, paint, strokeState_.GetGradient());
    }
    if (strokeState_.GetPattern().IsValid()) {
        UpdatePaintShader(strokeState_.GetPattern(), paint);
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        skCanvas_->drawPath(skPath_, paint);
    } else {
        InitPaintBlend(cachePaint_);
        cacheCanvas_->drawPath(skPath_, paint);
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, SkSamplingOptions(), &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
#else
    RSPen pen = GetStrokePaint();
    pen.SetAntiAlias(antiAlias_);
    if (HasShadow()) {
        RosenDecorationPainter::PaintShadow(drawingPath_, shadow_, drawingCanvas_);
    }
    if (strokeState_.GetGradient().IsValid()) {
        UpdatePaintShader(offset, pen, strokeState_.GetGradient());
    }
    if (strokeState_.GetPattern().IsValid()) {
        UpdatePaintShader(strokeState_.GetPattern(), pen);
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        drawingCanvas_->AttachPen(pen);
        drawingCanvas_->DrawPath(drawingPath_);
        drawingCanvas_->DetachPen();
    } else {
        InitPaintBlend(cacheBrush_);
        cacheCanvas_->AttachPen(pen);
        cacheCanvas_->DrawPath(drawingPath_);
        cacheCanvas_->DetachPen();
        drawingCanvas_->DrawBitmap(cacheBitmap_, 0, 0);
        cacheBitmap_.ClearWithColor(RSColor::COLOR_TRANSPARENT);
    }
#endif
}

void RosenRenderCustomPaint::Stroke(const Offset& offset, const RefPtr<CanvasPath2D>& path)
{
    if (path == nullptr) {
        LOGE("Stroke failed, target path is null.");
        return;
    }
    ParsePath2D(offset, path);
    Path2DStroke(offset);
#ifndef USE_ROSEN_DRAWING
    skPath2d_.reset();
#else
    drawingPath2d_.Reset();
#endif
}

void RosenRenderCustomPaint::Path2DAddPath(const Offset& offset, const PathArgs& args)
{
#ifndef USE_ROSEN_DRAWING
    SkPath out;
    SkParsePath::FromSVGString(args.cmds.c_str(), &out);
    skPath2d_.addPath(out);
#else
    RSRecordingPath out;
    out.FromSVGString(args.cmds.c_str());
    drawingPath2d_.AddPath(out);
#endif
}

void RosenRenderCustomPaint::Path2DSetTransform(const Offset& offset, const PathArgs& args)
{
#ifndef USE_ROSEN_DRAWING
    SkMatrix skMatrix;
    double scaleX = args.para1;
    double skewX = args.para2;
    double skewY = args.para3;
    double scaleY = args.para4;
    double translateX = args.para5;
    double translateY = args.para6;
    skMatrix.setAll(scaleX, skewY, translateX, skewX, scaleY, translateY, 0, 0, 1);
    skPath2d_.transform(skMatrix);
#else
    RSMatrix matrix;
    double scaleX = args.para1;
    double skewX = args.para2;
    double skewY = args.para3;
    double scaleY = args.para4;
    double translateX = args.para5;
    double translateY = args.para6;
    matrix.SetMatrix(scaleX, skewY, translateX, skewX, scaleY, translateY, 0, 0, 1);
    drawingPath2d_.Transform(matrix);
#endif
}

void RosenRenderCustomPaint::Path2DMoveTo(const Offset& offset, const PathArgs& args)
{
    double x = args.para1 + offset.GetX();
    double y = args.para2 + offset.GetY();
#ifndef USE_ROSEN_DRAWING
    skPath2d_.moveTo(x, y);
#else
    drawingPath2d_.MoveTo(x, y);
#endif
}

void RosenRenderCustomPaint::Path2DLineTo(const Offset& offset, const PathArgs& args)
{
    double x = args.para1 + offset.GetX();
    double y = args.para2 + offset.GetY();
#ifndef USE_ROSEN_DRAWING
    skPath2d_.lineTo(x, y);
#else
    drawingPath2d_.LineTo(x, y);
#endif
}

void RosenRenderCustomPaint::Path2DArc(const Offset& offset, const PathArgs& args)
{
    double x = args.para1;
    double y = args.para2;
    double r = args.para3;
#ifndef USE_ROSEN_DRAWING
    auto rect =
        SkRect::MakeLTRB(x - r + offset.GetX(), y - r + offset.GetY(), x + r + offset.GetX(), y + r + offset.GetY());
#else
    RSRect rect(
        x - r + offset.GetX(), y - r + offset.GetY(), x + r + offset.GetX(), y + r + offset.GetY());
#endif
    double startAngle = args.para4 * HALF_CIRCLE_ANGLE / M_PI;
    double endAngle = args.para5 * HALF_CIRCLE_ANGLE / M_PI;
    double sweepAngle = endAngle - startAngle;
    if (!NearZero(args.para6)) {
        sweepAngle =
            endAngle > startAngle ? (std::fmod(sweepAngle, FULL_CIRCLE_ANGLE) - FULL_CIRCLE_ANGLE) : sweepAngle;
    } else {
        sweepAngle =
            endAngle > startAngle ? sweepAngle : (std::fmod(sweepAngle, FULL_CIRCLE_ANGLE) + FULL_CIRCLE_ANGLE);
    }
#ifndef USE_ROSEN_DRAWING
    if (NearEqual(std::fmod(sweepAngle, FULL_CIRCLE_ANGLE), 0.0) && !NearEqual(startAngle, endAngle)) {
        skPath2d_.arcTo(rect, startAngle, HALF_CIRCLE_ANGLE, false);
        skPath2d_.arcTo(rect, startAngle + HALF_CIRCLE_ANGLE, HALF_CIRCLE_ANGLE, false);
    } else if (!NearEqual(std::fmod(sweepAngle, FULL_CIRCLE_ANGLE), 0.0) && abs(sweepAngle) > FULL_CIRCLE_ANGLE) {
        skPath2d_.arcTo(rect, startAngle, HALF_CIRCLE_ANGLE, false);
        skPath2d_.arcTo(rect, startAngle + HALF_CIRCLE_ANGLE, HALF_CIRCLE_ANGLE, false);
        skPath2d_.arcTo(rect, startAngle + HALF_CIRCLE_ANGLE + HALF_CIRCLE_ANGLE, sweepAngle, false);
    } else {
        skPath2d_.arcTo(rect, startAngle, sweepAngle, false);
    }
#else
    if (NearEqual(std::fmod(sweepAngle, FULL_CIRCLE_ANGLE), 0.0) && !NearEqual(startAngle, endAngle)) {
        drawingPath2d_.ArcTo(rect, startAngle, HALF_CIRCLE_ANGLE, false);
        drawingPath2d_.ArcTo(rect, startAngle + HALF_CIRCLE_ANGLE, HALF_CIRCLE_ANGLE, false);
    } else if (!NearEqual(std::fmod(sweepAngle, FULL_CIRCLE_ANGLE), 0.0) && abs(sweepAngle) > FULL_CIRCLE_ANGLE) {
        drawingPath2d_.ArcTo(rect, startAngle, HALF_CIRCLE_ANGLE, false);
        drawingPath2d_.ArcTo(rect, startAngle + HALF_CIRCLE_ANGLE, HALF_CIRCLE_ANGLE, false);
        drawingPath2d_.ArcTo(rect, startAngle + HALF_CIRCLE_ANGLE + HALF_CIRCLE_ANGLE, sweepAngle, false);
    } else {
        drawingPath2d_.ArcTo(rect, startAngle, sweepAngle, false);
    }
#endif
}

void RosenRenderCustomPaint::Path2DArcTo(const Offset& offset, const PathArgs& args)
{
    double x1 = args.para1 + offset.GetX();
    double y1 = args.para2 + offset.GetY();
    double x2 = args.para3 + offset.GetX();
    double y2 = args.para4 + offset.GetY();
    double r = args.para5;
#ifndef USE_ROSEN_DRAWING
    skPath2d_.arcTo(x1, y1, x2, y2, r);
#else
    drawingPath2d_.ArcTo(x1, y1, x2, y2, r);
#endif
}

void RosenRenderCustomPaint::Path2DQuadraticCurveTo(const Offset& offset, const PathArgs& args)
{
    double cpx = args.para1 + offset.GetX();
    double cpy = args.para2 + offset.GetY();
    double x = args.para3 + offset.GetX();
    double y = args.para4 + offset.GetY();
#ifndef USE_ROSEN_DRAWING
    skPath2d_.quadTo(cpx, cpy, x, y);
#else
    drawingPath2d_.QuadTo(cpx, cpy, x, y);
#endif
}

void RosenRenderCustomPaint::Path2DBezierCurveTo(const Offset& offset, const PathArgs& args)
{
    double cp1x = args.para1 + offset.GetX();
    double cp1y = args.para2 + offset.GetY();
    double cp2x = args.para3 + offset.GetX();
    double cp2y = args.para4 + offset.GetY();
    double x = args.para5 + offset.GetX();
    double y = args.para6 + offset.GetY();
#ifndef USE_ROSEN_DRAWING
    skPath2d_.cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
#else
    drawingPath2d_.CubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
#endif
}

void RosenRenderCustomPaint::Path2DEllipse(const Offset& offset, const PathArgs& args)
{
    if (NearEqual(args.para6, args.para7)) {
        return; // Just return when startAngle is same as endAngle.
    }

    double x = args.para1;
    double y = args.para2;
    double rx = args.para3;
    double ry = args.para4;
    double rotation = args.para5 * HALF_CIRCLE_ANGLE / M_PI;
    double startAngle = std::fmod(args.para6, M_PI * 2.0);
    double endAngle = std::fmod(args.para7, M_PI * 2.0);
    bool anticlockwise = NearZero(args.para8) ? false : true;
    startAngle = (startAngle < 0.0 ? startAngle + M_PI * 2.0 : startAngle) * HALF_CIRCLE_ANGLE / M_PI;
    endAngle = (endAngle < 0.0 ? endAngle + M_PI * 2.0 : endAngle) * HALF_CIRCLE_ANGLE / M_PI;
    double sweepAngle = endAngle - startAngle;
    if (anticlockwise) {
        if (sweepAngle > 0.0) { // Make sure the sweepAngle is negative when anticlockwise.
            sweepAngle -= FULL_CIRCLE_ANGLE;
        }
    } else {
        if (sweepAngle < 0.0) { // Make sure the sweepAngle is positive when clockwise.
            sweepAngle += FULL_CIRCLE_ANGLE;
        }
    }
#ifndef USE_ROSEN_DRAWING
    auto rect = SkRect::MakeLTRB(
        x - rx + offset.GetX(), y - ry + offset.GetY(), x + rx + offset.GetX(), y + ry + offset.GetY());

    if (!NearZero(rotation)) {
        SkMatrix matrix;
        matrix.setRotate(-rotation, x + offset.GetX(), y + offset.GetY());
        skPath2d_.transform(matrix);
    }
    if (NearZero(sweepAngle) && !NearZero(args.para6 - args.para7)) {
        // The entire ellipse needs to be drawn with two arcTo.
        skPath2d_.arcTo(rect, startAngle, HALF_CIRCLE_ANGLE, false);
        skPath2d_.arcTo(rect, startAngle + HALF_CIRCLE_ANGLE, HALF_CIRCLE_ANGLE, false);
    } else {
        skPath2d_.arcTo(rect, startAngle, sweepAngle, false);
    }
    if (!NearZero(rotation)) {
        SkMatrix matrix;
        matrix.setRotate(rotation, x + offset.GetX(), y + offset.GetY());
        skPath2d_.transform(matrix);
    }
#else
    RSRect rect(
        x - rx + offset.GetX(), y - ry + offset.GetY(), x + rx + offset.GetX(), y + ry + offset.GetY());

    if (!NearZero(rotation)) {
        RSMatrix matrix;
        matrix.Rotate(-rotation, x + offset.GetX(), y + offset.GetY());
        drawingPath2d_.Transform(matrix);
    }
    if (NearZero(sweepAngle) && !NearZero(args.para6 - args.para7)) {
        // The entire ellipse needs to be drawn with two arcTo.
        drawingPath2d_.ArcTo(rect, startAngle, HALF_CIRCLE_ANGLE, false);
        drawingPath2d_.ArcTo(rect, startAngle + HALF_CIRCLE_ANGLE, HALF_CIRCLE_ANGLE, false);
    } else {
        drawingPath2d_.ArcTo(rect, startAngle, sweepAngle, false);
    }
    if (!NearZero(rotation)) {
        RSMatrix matrix;
        matrix.Rotate(rotation, x + offset.GetX(), y + offset.GetY());
        drawingPath2d_.Transform(matrix);
    }
#endif
}

void RosenRenderCustomPaint::Path2DRect(const Offset& offset, const PathArgs& args)
{
    double left = args.para1 + offset.GetX();
    double top = args.para2 + offset.GetY();
    double right = args.para3 + args.para1 + offset.GetX();
    double bottom = args.para4 + args.para2 + offset.GetY();
#ifndef USE_ROSEN_DRAWING
    skPath2d_.addRect(SkRect::MakeLTRB(left, top, right, bottom));
#else
    drawingPath2d_.AddRect(RSRect(left, top, right, bottom));
#endif
}

void RosenRenderCustomPaint::Path2DClosePath(const Offset& offset, const PathArgs& args)
{
#ifndef USE_ROSEN_DRAWING
    skPath2d_.close();
#else
    drawingPath2d_.Close();
#endif
}

void RosenRenderCustomPaint::Path2DStroke(const Offset& offset)
{
#ifndef USE_ROSEN_DRAWING
    SkPaint paint = GetStrokePaint();
    paint.setAntiAlias(antiAlias_);
    if (HasShadow()) {
        RosenDecorationPainter::PaintShadow(skPath2d_, shadow_, skCanvas_.get());
    }
    if (strokeState_.GetGradient().IsValid()) {
        UpdatePaintShader(offset, paint, strokeState_.GetGradient());
    }
    if (strokeState_.GetPattern().IsValid()) {
        UpdatePaintShader(strokeState_.GetPattern(), paint);
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        skCanvas_->drawPath(skPath2d_, paint);
    } else {
        InitPaintBlend(cachePaint_);
        cacheCanvas_->drawPath(skPath2d_, paint);
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, SkSamplingOptions(), &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
#else
    RSPen pen = GetStrokePaint();
    pen.SetAntiAlias(antiAlias_);
    if (HasShadow()) {
        RosenDecorationPainter::PaintShadow(drawingPath2d_, shadow_, drawingCanvas_);
    }
    if (strokeState_.GetGradient().IsValid()) {
        UpdatePaintShader(offset, pen, strokeState_.GetGradient());
    }
    if (strokeState_.GetPattern().IsValid()) {
        UpdatePaintShader(strokeState_.GetPattern(), pen);
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        drawingCanvas_->drawPath(skPath2d_, paint);
    } else {
        InitPaintBlend(cacheBrush_);
        cacheCanvas_->AttachPen(pen);
        cacheCanvas_->DrawPath(drawingPath2d_);
        cacheCanvas_->DetachPen();
        drawingCanvas_->AttachBrush(cacheBrush_);
        drawingCanvas_->DrawBitmap(cacheBitmap_, 0, 0);
        drawingCanvas_->DetachBrush();
        cacheBitmap_.ClearWithColor(RSColor::COLOR_TRANSPARENT);
    }
#endif
}

void RosenRenderCustomPaint::Path2DFill(const Offset& offset)
{
#ifndef USE_ROSEN_DRAWING
    SkPaint paint;
    paint.setAntiAlias(antiAlias_);
    paint.setColor(fillState_.GetColor().GetValue());
    paint.setStyle(SkPaint::Style::kFill_Style);
    if (HasShadow()) {
        RosenDecorationPainter::PaintShadow(skPath2d_, shadow_, skCanvas_.get());
    }
    if (fillState_.GetGradient().IsValid()) {
        UpdatePaintShader(offset, paint, fillState_.GetGradient());
    }
    if (fillState_.GetPattern().IsValid()) {
        UpdatePaintShader(fillState_.GetPattern(), paint);
    }
    if (globalState_.HasGlobalAlpha()) {
        paint.setAlphaf(globalState_.GetAlpha());
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        skCanvas_->drawPath(skPath2d_, paint);
    } else {
        InitPaintBlend(cachePaint_);
        cacheCanvas_->drawPath(skPath2d_, paint);
#ifndef NEW_SKIA
        skCanvas_->drawBitmap(cacheBitmap_, 0, 0, &cachePaint_);
#else
        skCanvas_->drawImage(cacheBitmap_.asImage(), 0, 0, SkSamplingOptions(), &cachePaint_);
#endif
        cacheBitmap_.eraseColor(0);
    }
#else
    RSBrush brush;
    brush.SetAntiAlias(antiAlias_);
    brush.SetColor(fillState_.GetColor().GetValue());
    if (HasShadow()) {
        RosenDecorationPainter::PaintShadow(drawingPath2d_, shadow_, drawingCanvas_);
    }
    if (fillState_.GetGradient().IsValid()) {
        UpdatePaintShader(offset, brush, fillState_.GetGradient());
    }
    if (fillState_.GetPattern().IsValid()) {
        UpdatePaintShader(fillState_.GetPattern(), brush);
    }
    if (globalState_.HasGlobalAlpha()) {
        paint.SetAlphaF(globalState_.GetAlpha());
    }
    if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
        drawingCanvas_->AttachBrush(brush);
        drawingCanvas_->DrawPath(drawingPath2d_);
        drawingCanvas_->DetachBrush();
    } else {
        InitPaintBlend(cacheBrush_);
        cacheCanvas_->AttachBrush(brush);
        cacheCanvas_->DrawPath(drawingPath2d_);
        cacheCanvas_->DetachBrush();
        drawingCanvas_->DrawBitmap(cacheBitmap_, 0, 0);
        cacheBitmap_.ClearWithColor(RSColor::COLOR_TRANSPARENT);
    }
#endif
}

#ifndef USE_ROSEN_DRAWING
void RosenRenderCustomPaint::Path2DClip()
{
    skCanvas_->clipPath(skPath2d_);
}

void RosenRenderCustomPaint::Clip()
{
    skCanvas_->clipPath(skPath_);
}
#else
void RosenRenderCustomPaint::Path2DClip()
{
    drawingCanvas_->ClipPath(drawingPath2d_);
}

void RosenRenderCustomPaint::Clip()
{
    drawingCanvas_->ClipPath(drawingPath_, RSClipOp::INTERSECT);
}
#endif

void RosenRenderCustomPaint::Clip(const RefPtr<CanvasPath2D>& path)
{
    if (path == nullptr) {
        LOGE("Fill failed, target path is null.");
        return;
    }
    auto offset = Offset(0, 0);
    ParsePath2D(offset, path);
    Path2DClip();
#ifndef USE_ROSEN_DRAWING
    skPath2d_.reset();
#else
    drawingPath2d_.Reset();
#endif
}

void RosenRenderCustomPaint::BeginPath()
{
#ifndef USE_ROSEN_DRAWING
    skPath_.reset();
#else
    drawingPath_.Reset();
#endif
}

void RosenRenderCustomPaint::ResetTransform()
{
#ifndef USE_ROSEN_DRAWING
    skCanvas_->resetMatrix();
#else
    drawingCanvas_->ResetMatrix();
#endif
}

void RosenRenderCustomPaint::ClosePath()
{
#ifndef USE_ROSEN_DRAWING
    skPath_.close();
#else
    drawingPath_.Close();
#endif
}

void RosenRenderCustomPaint::Save()
{
    SaveStates();
#ifndef USE_ROSEN_DRAWING
    skCanvas_->save();
#else
    drawingCanvas_->Save();
#endif
}

void RosenRenderCustomPaint::Restore()
{
    RestoreStates();
#ifndef USE_ROSEN_DRAWING
    skCanvas_->restore();
#else
    drawingCanvas_->Restore();
#endif
}

void RosenRenderCustomPaint::InitImagePaint()
{
#ifndef USE_ROSEN_DRAWING
#ifndef NEW_SKIA
    if (smoothingEnabled_) {
        if (smoothingQuality_ == "low") {
            imagePaint_.setFilterQuality(SkFilterQuality::kLow_SkFilterQuality);
        } else if (smoothingQuality_ == "medium") {
            imagePaint_.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
        } else if (smoothingQuality_ == "high") {
            imagePaint_.setFilterQuality(SkFilterQuality::kHigh_SkFilterQuality);
        } else {
            LOGE("Unsupported Quality type:%{public}s", smoothingQuality_.c_str());
        }
    } else {
        imagePaint_.setFilterQuality(SkFilterQuality::kNone_SkFilterQuality);
    }
#else
    if (smoothingEnabled_) {
        if (smoothingQuality_ == "low") {
            options_ = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
        } else if (smoothingQuality_ == "medium") {
            options_ = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear);
        } else if (smoothingQuality_ == "high") {
            options_ = SkSamplingOptions(SkCubicResampler { 1 / 3.0f, 1 / 3.0f });
        } else {
            LOGE("Unsupported Quality type:%{public}s", smoothingQuality_.c_str());
        }
    } else {
        options_ = SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone);
    }
#endif
#else
    RSFilter filter;
    if (smoothingEnabled_) {
        if (smoothingQuality_ == "low") {
            filter.SetFilterQuality(RSFilter::FilterQuality::LOW);
            imageBrush_.SetFilter(filter);
        } else if (smoothingQuality_ == "medium") {
            filter.SetFilterQuality(RSFilter::FilterQuality::MEDIUM);
            imageBrush_.SetFilter(filter);
        } else if (smoothingQuality_ == "high") {
            filter.SetFilterQuality(RSFilter::FilterQuality::HIGH);
            imageBrush_.SetFilter(filter);
        } else {
            LOGE("Unsupported Quality type:%{public}s", smoothingQuality_.c_str());
        }
    } else {
        filter.SetFilterQuality(RSFilter::FilterQuality::NONE);
        imageBrush_.SetFilter(filter);
    }
#endif
}

#ifndef USE_ROSEN_DRAWING
void RosenRenderCustomPaint::InitPaintBlend(SkPaint& paint)
{
    paint.setBlendMode(
        ConvertEnumToSkEnum(globalState_.GetType(), SK_BLEND_MODE_TABLE, BLEND_MODE_SIZE, SkBlendMode::kSrcOver));
}
#else
void RosenRenderCustomPaint::InitPaintBlend(RSPen& pen)
{
    pen.SetBlendMode(ConvertEnumToDrawingEnum(
        globalState_.GetType(), DRAWING_BLEND_MODE_TABLE, BLEND_MODE_SIZE, RSBlendMode::SRC_OVER));
}
#endif

bool RosenRenderCustomPaint::UpdateParagraph(
    const Offset& offset, const std::string& text, bool isStroke, bool hasShadow)
{
    using namespace Constants;
    txt::ParagraphStyle style;
    if (isStroke) {
        style.text_align = ConvertTxtTextAlign(strokeState_.GetTextAlign());
    } else {
        style.text_align = ConvertTxtTextAlign(fillState_.GetTextAlign());
    }
    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    if (!fontCollection) {
        LOGW("UpdateParagraph: fontCollection is null");
        return false;
    }
    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
    if (!isStroke && hasShadow) {
        txt::TextShadow txtShadow;
        txtShadow.color = shadow_.GetColor().GetValue();
        txtShadow.offset.fX = shadow_.GetOffset().GetX();
        txtShadow.offset.fY = shadow_.GetOffset().GetY();
#ifndef NEW_SKIA
        txtShadow.blur_radius = shadow_.GetBlurRadius();
#else
        txtShadow.blur_sigma = shadow_.GetBlurRadius();
#endif
        txtStyle.text_shadows.emplace_back(txtShadow);
    }
    txtStyle.locale = Localization::GetInstance()->GetFontLocale();
    UpdateTextStyleForeground(offset, isStroke, txtStyle, hasShadow);
    builder->PushStyle(txtStyle);
    builder->AddText(StringUtils::Str8ToStr16(text));
    paragraph_ = builder->Build();
    return true;
}

void RosenRenderCustomPaint::UpdateTextStyleForeground(
    const Offset& offset, bool isStroke, txt::TextStyle& txtStyle, bool hasShadow)
{
    using namespace Constants;
#ifndef USE_ROSEN_DRAWING
    if (!isStroke) {
        txtStyle.color = ConvertSkColor(fillState_.GetColor());
        txtStyle.font_size = fillState_.GetTextStyle().GetFontSize().Value();
        ConvertTxtStyle(fillState_.GetTextStyle(), context_, txtStyle);
        if (fillState_.GetGradient().IsValid()) {
            SkPaint paint;
            paint.setStyle(SkPaint::Style::kFill_Style);
            UpdatePaintShader(offset, paint, fillState_.GetGradient());
            txtStyle.foreground = paint;
            txtStyle.has_foreground = true;
        }
        if (globalState_.HasGlobalAlpha()) {
            if (txtStyle.has_foreground) {
                txtStyle.foreground.setColor(fillState_.GetColor().GetValue());
                txtStyle.foreground.setAlphaf(globalState_.GetAlpha()); // set alpha after color
            } else {
                SkPaint paint;
                paint.setColor(fillState_.GetColor().GetValue());
                paint.setAlphaf(globalState_.GetAlpha()); // set alpha after color
                InitPaintBlend(paint);
                txtStyle.foreground = paint;
                txtStyle.has_foreground = true;
            }
        }
    } else {
        // use foreground to draw stroke
        SkPaint paint = GetStrokePaint();
        InitPaintBlend(paint);
        ConvertTxtStyle(strokeState_.GetTextStyle(), context_, txtStyle);
        txtStyle.font_size = strokeState_.GetTextStyle().GetFontSize().Value();
        if (strokeState_.GetGradient().IsValid()) {
            UpdatePaintShader(offset, paint, strokeState_.GetGradient());
        }
        if (hasShadow) {
            paint.setColor(shadow_.GetColor().GetValue());
            paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle,
                RosenDecorationPainter::ConvertRadiusToSigma(shadow_.GetBlurRadius())));
        }
        txtStyle.foreground = paint;
        txtStyle.has_foreground = true;
    }
#else
    // TODO Drawing : about txt
#endif
}

bool RosenRenderCustomPaint::HasShadow() const
{
    return !(NearZero(shadow_.GetOffset().GetX()) && NearZero(shadow_.GetOffset().GetY()) &&
             NearZero(shadow_.GetBlurRadius()));
}

#ifndef USE_ROSEN_DRAWING
void RosenRenderCustomPaint::UpdatePaintShader(const Offset& offset, SkPaint& paint, const Gradient& gradient)
{
    SkPoint beginPoint = SkPoint::Make(SkDoubleToScalar(gradient.GetBeginOffset().GetX() + offset.GetX()),
        SkDoubleToScalar(gradient.GetBeginOffset().GetY() + offset.GetY()));
    SkPoint endPoint = SkPoint::Make(SkDoubleToScalar(gradient.GetEndOffset().GetX() + offset.GetX()),
        SkDoubleToScalar(gradient.GetEndOffset().GetY() + offset.GetY()));
    SkPoint pts[2] = { beginPoint, endPoint };
    std::vector<GradientColor> gradientColors = gradient.GetColors();
    std::stable_sort(gradientColors.begin(), gradientColors.end(),
        [](auto& colorA, auto& colorB) { return colorA.GetDimension() < colorB.GetDimension(); });
    uint32_t colorsSize = gradientColors.size();
    SkColor colors[gradientColors.size()];
    float pos[gradientColors.size()];
    for (uint32_t i = 0; i < colorsSize; ++i) {
        const auto& gradientColor = gradientColors[i];
        colors[i] = gradientColor.GetColor().GetValue();
        pos[i] = gradientColor.GetDimension().Value();
    }

#ifdef USE_SYSTEM_SKIA
    auto mode = SkShader::kClamp_TileMode;
#else
    auto mode = SkTileMode::kClamp;
#endif

    sk_sp<SkShader> skShader = nullptr;
    if (gradient.GetType() == GradientType::LINEAR) {
        skShader = SkGradientShader::MakeLinear(pts, colors, pos, gradientColors.size(), mode);
    } else {
        if (gradient.GetInnerRadius() <= 0.0 && beginPoint == endPoint) {
            skShader = SkGradientShader::MakeRadial(
                endPoint, gradient.GetOuterRadius(), colors, pos, gradientColors.size(), mode);
        } else {
            skShader = SkGradientShader::MakeTwoPointConical(beginPoint, gradient.GetInnerRadius(), endPoint,
                gradient.GetOuterRadius(), colors, pos, gradientColors.size(), mode);
        }
    }
    paint.setShader(skShader);
}
#else
void RosenRenderCustomPaint::UpdatePaintShader(
    const Offset& offset, RSBrush& brush, const Gradient& gradient)
{
    RSPoint beginPoint =
        RSPoint(static_cast<RSScalar>(gradient.GetBeginOffset().GetX() + offset.GetX()),
            static_cast<RSScalar>(gradient.GetBeginOffset().GetY() + offset.GetY()));
    RSPoint endPoint =
        RSPoint(static_cast<RSScalar>(gradient.GetEndOffset().GetX() + offset.GetX()),
            static_cast<RSScalar>(gradient.GetEndOffset().GetY() + offset.GetY()));
    std::vector<RSPoint> pts = { beginPoint, endPoint };
    auto gradientColors = gradient.GetColors();
    std::stable_sort(gradientColors.begin(), gradientColors.end(),
        [](auto& colorA, auto& colorB) { return colorA.GetDimension() < colorB.GetDimension(); });
    uint32_t colorsSize = gradientColors.size();
    std::vector<RSColorQuad> colors(gradientColors.size(), 0);
    std::vector<RSScalar> pos(gradientColors.size(), 0);
    for (uint32_t i = 0; i < colorsSize; ++i) {
        const auto& gradientColor = gradientColors[i];
        colors.at(i) = gradientColor.GetColor().GetValue();
        pos.at(i) = gradientColor.GetDimension().Value();
    }

    auto mode = RSTileMode::CLAMP;

    std::shared_ptr<RSShaderEffect> shaderEffect = nullptr;
    if (gradient.GetType() == GradientType::LINEAR) {
        shaderEffect = RSShaderEffect::CreateLinearGradient(pts.at(0), pts.at(1), colors, pos, mode);
    } else {
        if (gradient.GetInnerRadius() <= 0.0 && beginPoint == endPoint) {
            shaderEffect = RSShaderEffect::CreateRadialGradient(
                pts.at(1), gradient.GetOuterRadius(), colors, pos, mode);
        } else {
            shaderEffect = RSShaderEffect::CreateTwoPointConical(
                pts.at(0), gradient.GetInnerRadius(), pts.at(1), gradient.GetOuterRadius(), colors, pos, mode);
        }
    }
    brush.SetShaderEffect(shaderEffect);
}
#endif

void RosenRenderCustomPaint::Rotate(double angle)
{
#ifndef USE_ROSEN_DRAWING
    skCanvas_->rotate(angle * 180 / M_PI);
#else
    drawingCanvas_->Rotate(angle * 180 / M_PI);
#endif
}

void RosenRenderCustomPaint::Scale(double x, double y)
{
#ifndef USE_ROSEN_DRAWING
    skCanvas_->scale(x, y);
#else
    drawingCanvas_->Scale(x, y);
#endif
}

void RosenRenderCustomPaint::SetTransform(const TransformParam& param)
{
    auto pipeline = context_.Upgrade();
    if (!pipeline) {
        return;
    }
    // use physical pixel to store bitmap
    double viewScale = pipeline->GetViewScale();
#ifndef USE_ROSEN_DRAWING
    SkMatrix skMatrix;
    skMatrix.setAll(param.scaleX * viewScale, param.skewY * viewScale, param.translateX * viewScale,
        param.skewX * viewScale, param.scaleY * viewScale, param.translateY * viewScale, 0, 0, 1);
    skCanvas_->setMatrix(skMatrix);
#else
    RSMatrix matrix;
    matrix.SetMatrix(param.scaleX * viewScale, param.skewY * viewScale, param.translateX * viewScale,
        param.skewX * viewScale, param.scaleY * viewScale, param.translateY * viewScale, 0, 0, 1);
    drawingCanvas_->SetMatrix(matrix);
#endif
}

void RosenRenderCustomPaint::Transform(const TransformParam& param)
{
#ifndef USE_ROSEN_DRAWING
    SkMatrix skMatrix;
    skMatrix.setAll(param.scaleX, param.skewY, param.translateX, param.skewX, param.scaleY, param.translateY, 0, 0, 1);
    skCanvas_->concat(skMatrix);
#else
    RSMatrix matrix;
    matrix.SetMatrix(param.scaleX, param.skewY, param.translateX, param.skewX, param.scaleY, param.translateY, 0, 0, 1);
    drawingCanvas_->ConcatMatrix(matrix);
#endif
}

void RosenRenderCustomPaint::Translate(double x, double y)
{
#ifndef USE_ROSEN_DRAWING
    skCanvas_->translate(x, y);
#else
    drawingCanvas_->Translate(x, y);
#endif
}

#ifndef USE_ROSEN_DRAWING
void RosenRenderCustomPaint::UpdateLineDash(SkPaint& paint)
{
    if (!strokeState_.GetLineDash().lineDash.empty()) {
        auto lineDashState = strokeState_.GetLineDash().lineDash;
        SkScalar intervals[lineDashState.size()];
        for (size_t i = 0; i < lineDashState.size(); ++i) {
            intervals[i] = SkDoubleToScalar(lineDashState[i]);
        }
        SkScalar phase = SkDoubleToScalar(strokeState_.GetLineDash().dashOffset);
        paint.setPathEffect(SkDashPathEffect::Make(intervals, lineDashState.size(), phase));
    }
}
#else
void RosenRenderCustomPaint::UpdateLineDash(RSPen& pen)
{
    if (!strokeState_.GetLineDash().lineDash.empty()) {
        auto lineDashState = strokeState_.GetLineDash().lineDash;
        RSScalar intervals[lineDashState.size()];
        for (size_t i = 0; i < lineDashState.size(); ++i) {
            intervals[i] = static_cast<RSScalar>(lineDashState[i]);
        }
        RSScalar phase = static_cast<RSScalar>(strokeState_.GetLineDash().dashOffset);
        pen.SetPathEffect(RSPathEffect::CreateDashPathEffect(intervals, lineDashState.size(), phase));
    }
}
#endif

void RosenRenderCustomPaint::InitImageCallbacks()
{
    imageObjSuccessCallback_ = [weak = AceType::WeakClaim(this)](
                                   ImageSourceInfo info, const RefPtr<ImageObject>& imageObj) {
        auto render = weak.Upgrade();
        if (render->loadingSource_ == info) {
            render->ImageObjReady(imageObj);
            return;
        } else {
            LOGE("image sourceInfo_ check error, : %{public}s vs %{public}s", render->loadingSource_.ToString().c_str(),
                info.ToString().c_str());
        }
    };

    failedCallback_ = [weak = AceType::WeakClaim(this)](ImageSourceInfo info, const std::string& errorMsg = "") {
        auto render = weak.Upgrade();
        LOGE("tkh failedCallback_");
        render->ImageObjFailed();
    };

    uploadSuccessCallback_ = [weak = AceType::WeakClaim(this)](
                                 ImageSourceInfo sourceInfo, const RefPtr<NG::CanvasImage>& image) {};

    onPostBackgroundTask_ = [weak = AceType::WeakClaim(this)](CancelableTask task) {};
}

void RosenRenderCustomPaint::ImageObjReady(const RefPtr<ImageObject>& imageObj)
{
    imageObj_ = imageObj;
    if (imageObj_->IsSvg()) {
#ifndef USE_ROSEN_DRAWING
        skiaDom_ = AceType::DynamicCast<SvgSkiaImageObject>(imageObj_)->GetSkiaDom();
#else
        drawingDom_ = AceType::DynamicCast<SvgDrawingImageObject>(imageObj_)->GetSvgDom();
#endif
        currentSource_ = loadingSource_;
        CanvasImage canvasImage = canvasImage_;
        TaskFunc func = [canvasImage](RenderCustomPaint& iface, const Offset& offset) {
            iface.DrawImage(offset, canvasImage, 0, 0);
        };
        tasks_.emplace_back(func);
        MarkNeedRender();
    } else {
        LOGE("image is not svg");
    }
}

void RosenRenderCustomPaint::ImageObjFailed()
{
    imageObj_ = nullptr;
    skiaDom_ = nullptr;
}

void RosenRenderCustomPaint::DrawSvgImage(const Offset& offset, const CanvasImage& canvasImage)
{
#ifndef USE_ROSEN_DRAWING
    const auto skCanvas = skCanvas_.get();
    // Make the ImageSourceInfo
    canvasImage_ = canvasImage;
    loadingSource_ = ImageSourceInfo(canvasImage.src);
    // get the ImageObject
    if (currentSource_ != loadingSource_) {
        ImageProvider::FetchImageObject(loadingSource_, imageObjSuccessCallback_, uploadSuccessCallback_,
            failedCallback_, GetContext(), true, true, true, onPostBackgroundTask_);
    }

    // draw the svg
    if (skiaDom_) {
        SkRect srcRect;
        SkRect dstRect;
        Offset startPoint;
        double scaleX = 1.0f;
        double scaleY = 1.0f;
        switch (canvasImage.flag) {
            case 0:
                srcRect = SkRect::MakeXYWH(0, 0, skiaDom_->containerSize().width(), skiaDom_->containerSize().height());
                dstRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, skiaDom_->containerSize().width(),
                    skiaDom_->containerSize().height());
                break;
            case 1: {
                srcRect = SkRect::MakeXYWH(0, 0, skiaDom_->containerSize().width(), skiaDom_->containerSize().height());
                dstRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
                break;
            }
            case 2: {
                srcRect = SkRect::MakeXYWH(canvasImage.sx, canvasImage.sy, canvasImage.sWidth, canvasImage.sHeight);
                dstRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
                break;
            }
            default:
                break;
        }
        scaleX = dstRect.width() / srcRect.width();
        scaleY = dstRect.height() / srcRect.height();
        startPoint =
            offset + Offset(dstRect.left(), dstRect.top()) - Offset(srcRect.left() * scaleX, srcRect.top() * scaleY);

        skCanvas->save();
        skCanvas->clipRect(dstRect);
        skCanvas->translate(startPoint.GetX(), startPoint.GetY());
        skCanvas->scale(scaleX, scaleY);
        skiaDom_->render(skCanvas);
        skCanvas->restore();
    }
#else
    const auto drawingCanvas = drawingCanvas_;
    // Make the ImageSourceInfo
    canvasImage_ = canvasImage;
    loadingSource_ = ImageSourceInfo(canvasImage.src);
    // get the ImageObject
    if (currentSource_ != loadingSource_) {
        ImageProvider::FetchImageObject(loadingSource_, imageObjSuccessCallback_, uploadSuccessCallback_,
            failedCallback_, GetContext(), true, true, true, renderTaskHolder_, onPostBackgroundTask_);
    }

    // draw the svg
    if (skiaDom_) {
        RSRect srcRect;
        RSRect dstRect;
        Offset startPoint;
        double scaleX = 1.0f;
        double scaleY = 1.0f;
        switch (canvasImage.flag) {
            case 0:
                srcRect = RSRect(0, 0,
                    drawingDom_->GetContainerSize().Width(), drawingDom_->GetContainerSize().Height());
                dstRect = RSRect(canvasImage.dx, canvasImage.dy,
                    drawingDom_->GetContainerSize().Width() + canvasImage.dx,
                    drawingDom_->GetContainerSize().Height() + canvasImage.dy);
                break;
            case 1: {
                srcRect = RSRect(0, 0,
                    drawingDom_->GetContainerSize().Width(), drawingDom_->GetContainerSize().Height());
                dstRect = RSRects(canvasImage.dx, canvasImage.dy,
                    canvasImage.dWidth + canvasImage.dx, canvasImage.dHeight + canvasImage.dy);
                break;
            }
            case 2: {
                srcRect = RSRect(canvasImage.sx, canvasImage.sy,
                    canvasImage.sWidth + canvasImage.sx, canvasImage.sHeight + canvasImage.sy);
                dstRect = RSRect(canvasImage.dx, canvasImage.dy,
                    canvasImage.dWidth + canvasImage.dx, canvasImage.dHeight + canvasImage.dy);
                break;
            }
            default:
                break;
        }
        scaleX = dstRect.GetWidth() / srcRect.GetWidth();
        scaleY = dstRect.GetHeight() / srcRect.GetHeight();
        startPoint = offset  + Offset(dstRect.GetLeft(), dstRect.GetTop())
            - Offset(srcRect.GetLeft() * scaleX, srcRect.GetTop() * scaleY);

        drawingCanvas->Save();
        drawingCanvas->ClipRect(dstRect, false);
        drawingCanvas->Translate(startPoint.GetX(), startPoint.GetY());
        drawingCanvas->Scale(scaleX, scaleY);
        skiaDom_->Render(*drawingCanvas);
        drawingCanvas->Restore();
    }
#endif
}

void RosenRenderCustomPaint::DrawImage(
    const Offset& offset, const CanvasImage& canvasImage, double width, double height)
{
    std::string::size_type tmp = canvasImage.src.find(".svg");
    if (tmp != std::string::npos) {
        DrawSvgImage(offset, canvasImage);
        return;
    }

    auto image = GetImage(canvasImage.src);

    if (!image) {
        LOGE("image is null");
        return;
    }
    InitImagePaint();
#ifndef USE_ROSEN_DRAWING
    InitPaintBlend(imagePaint_);

    switch (canvasImage.flag) {
        case 0:
            skCanvas_->drawImage(image, canvasImage.dx, canvasImage.dy);
            break;
        case 1: {
            SkRect rect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
#ifndef NEW_SKIA
            skCanvas_->drawImageRect(image, rect, &imagePaint_);
#else
            skCanvas_->drawImageRect(image, rect, options_, &imagePaint_);
#endif
            break;
        }
        case 2: {
            SkRect dstRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
            SkRect srcRect = SkRect::MakeXYWH(canvasImage.sx, canvasImage.sy, canvasImage.sWidth, canvasImage.sHeight);
#ifndef NEW_SKIA
            skCanvas_->drawImageRect(image, srcRect, dstRect, &imagePaint_);
#else
            skCanvas_->drawImageRect(image.get(), srcRect, dstRect, options_, &imagePaint_, SkCanvas::kFast_SrcRectConstraint);
#endif
            break;
        }
        default:
            break;
    }
#else
    InitPaintBlend(imagePen_);

    RSSamplingOptions sampling =
        RSSamplingOptions(RSFilterMode::NEAREST, RSMipmapMode::NEAREST);

    switch (canvasImage.flag) {
        case 0:
            drawingCanvas_->DrawImage(*image, canvasImage.dx, canvasImage.dy, sampling);
            break;
        case 1: {
            RSRect rect = RSRect(canvasImage.dx, canvasImage.dy,
                canvasImage.dWidth + canvasImage.dx, canvasImage.dHeight + canvasImage.dy);
            drawingCanvas_->AttachPen(imagePen_);
            drawingCanvas_->DrawImageRect(*image, rect, sampling);
            drawingCanvas_->DetachPen();
            break;
        }
        case 2: {
            RSRect dstRect =
                RSRect(canvasImage.dx, canvasImage.dy,
                    canvasImage.dWidth + canvasImage.dx, canvasImage.dHeight + canvasImage.dy);
            RSRect srcRect =
                RSRect(canvasImage.sx, canvasImage.sy,
                    canvasImage.sWidth + canvasImage.sx, canvasImage.sHeight + canvasImage.sy);
            drawingCanvas_->AttachPen(imagePen_);
            drawingCanvas_->DrawImageRect(*image, srcRect, dstRect, sampling);
            drawingCanvas_->DetachPen();
            break;
        }
        default:
            break;
    }
#endif
}

void RosenRenderCustomPaint::DrawPixelMap(RefPtr<PixelMap> pixelMap, const CanvasImage& canvasImage)
{
    auto context = GetContext().Upgrade();
    if (!context) {
        return;
    }

#ifndef USE_ROSEN_DRAWING
    // get skImage form pixelMap
    auto imageInfo = ImageProvider::MakeSkImageInfoFromPixelMap(pixelMap);
    SkPixmap imagePixmap(imageInfo, reinterpret_cast<const void*>(pixelMap->GetPixels()), pixelMap->GetRowBytes());

    // Step2: Create SkImage and draw it, using gpu or cpu
    sk_sp<SkImage> image =
        SkImage::MakeFromRaster(imagePixmap, &PixelMap::ReleaseProc, PixelMap::GetReleaseContext(pixelMap));
    if (!image) {
        LOGE("image is null");
        return;
    }
    InitImagePaint();
    InitPaintBlend(imagePaint_);
    switch (canvasImage.flag) {
        case 0:
            skCanvas_->drawImage(image, canvasImage.dx, canvasImage.dy);
            break;
        case 1: {
            SkRect rect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
#ifndef NEW_SKIA
            skCanvas_->drawImageRect(image, rect, &imagePaint_);
#else
            skCanvas_->drawImageRect(image, rect, options_, &imagePaint_);
#endif
            break;
        }
        case 2: {
            SkRect dstRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
            SkRect srcRect = SkRect::MakeXYWH(canvasImage.sx, canvasImage.sy, canvasImage.sWidth, canvasImage.sHeight);
#ifndef NEW_SKIA
            skCanvas_->drawImageRect(image, srcRect, dstRect, &imagePaint_);
#else
            skCanvas_->drawImageRect(image, srcRect, dstRect, options_, &imagePaint_, SkCanvas::kFast_SrcRectConstraint);
#endif
            break;
        }
        default:
            break;
    }
#else
    // TODO Drawing : Pixmap
#endif
}

#ifndef USE_ROSEN_DRAWING
void RosenRenderCustomPaint::UpdatePaintShader(const Pattern& pattern, SkPaint& paint)
{
    auto context = GetContext().Upgrade();
    if (!context) {
        return;
    }

    auto width = pattern.GetImageWidth();
    auto height = pattern.GetImageHeight();
    auto image = GreatOrEqual(width, 0) && GreatOrEqual(height, 0)
                     ? ImageProvider::GetSkImage(pattern.GetImgSrc(), context, Size(width, height))
                     : ImageProvider::GetSkImage(pattern.GetImgSrc(), context);
    if (!image) {
        LOGE("image is null");
        return;
    }
    static const LinearMapNode<void (*)(sk_sp<SkImage>, SkPaint&)> staticPattern[] = {
        { "no-repeat",
            [](sk_sp<SkImage> image, SkPaint& paint) {
#ifdef USE_SYSTEM_SKIA
                paint.setShader(image->makeShader(SkShader::kDecal_TileMode, SkShader::kDecal_TileMode, nullptr));
#elif defined(NEW_SKIA)
                paint.setShader(image->makeShader(SkTileMode::kDecal, SkTileMode::kDecal, SkSamplingOptions(), nullptr));
#else
                paint.setShader(image->makeShader(SkTileMode::kDecal, SkTileMode::kDecal, nullptr));
#endif
            } },
        { "repeat",
            [](sk_sp<SkImage> image, SkPaint& paint) {
#ifdef USE_SYSTEM_SKIA
                paint.setShader(image->makeShader(SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode, nullptr));
#elif defined(NEW_SKIA)
                paint.setShader(image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions(), nullptr));
#else
                paint.setShader(image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, nullptr));
#endif
            } },
        { "repeat-x",
            [](sk_sp<SkImage> image, SkPaint& paint) {
#ifdef USE_SYSTEM_SKIA
                paint.setShader(image->makeShader(SkShader::kRepeat_TileMode, SkShader::kDecal_TileMode, nullptr));
#elif defined(NEW_SKIA)
                paint.setShader(image->makeShader(SkTileMode::kRepeat, SkTileMode::kDecal, SkSamplingOptions(), nullptr));
#else
                paint.setShader(image->makeShader(SkTileMode::kRepeat, SkTileMode::kDecal, nullptr));
#endif
            } },
        { "repeat-y",
            [](sk_sp<SkImage> image, SkPaint& paint) {
#ifdef USE_SYSTEM_SKIA
                paint.setShader(image->makeShader(SkShader::kDecal_TileMode, SkShader::kRepeat_TileMode, nullptr));
#elif defined(NEW_SKIA)
                paint.setShader(image->makeShader(SkTileMode::kDecal, SkTileMode::kRepeat, SkSamplingOptions(), nullptr));
#else
                paint.setShader(image->makeShader(SkTileMode::kDecal, SkTileMode::kRepeat, nullptr));
#endif
            } },
    };
    auto operatorIter = BinarySearchFindIndex(staticPattern, ArraySize(staticPattern), pattern.GetRepetition().c_str());
    if (operatorIter != -1) {
        staticPattern[operatorIter].value(image, paint);
    }
}
#else
void RosenRenderCustomPaint::UpdatePaintShader(const Pattern& pattern, RSBrush& brush)
{
    auto context = GetContext().Upgrade();
    if (!context) {
        return;
    }

    auto width = pattern.GetImageWidth();
    auto height = pattern.GetImageHeight();
    auto image = GreatOrEqual(width, 0) && GreatOrEqual(height, 0)
                     ? ImageProvider::GetDrawingImage(pattern.GetImgSrc(), context, Size(width, height))
                     : ImageProvider::GetDrawingImage(pattern.GetImgSrc(), context);
    if (!image) {
        LOGE("image is null");
        return;
    }
    static const LinearMapNode<void (*)(std::shared_ptr<RSImage>, RSBrush&)>
        staticPattern[] = {
            { "no-repeat",
                [](std::shared_ptr<RSImage> image, RSBrush& brush) {
                    brush.SetShaderEffect(RSShaderEffect::CreateImageShader(*image,
                        RSTileMode::DECAL, RSTileMode::DECAL,
                        RSSamplingOptions(
                            RSFilterMode::NEAREST, RSMipmapMode::NEAREST),
                        RSMatrix()));
                } },
            { "repeat",
                [](std::shared_ptr<RSImage> image, RSBrush& brush) {
                    brush.SetShaderEffect(RSShaderEffect::CreateImageShader(*image,
                        RSTileMode::REPEAT, RSTileMode::REPEAT,
                        RSSamplingOptions(
                            RSFilterMode::NEAREST, RSMipmapMode::NEAREST),
                        RSMatrix()));
                } },
            { "repeat-x",
                [](std::shared_ptr<RSImage> image, RSBrush& brush) {
                    brush.SetShaderEffect(RSShaderEffect::CreateImageShader(*image,
                        RSTileMode::REPEAT, RSTileMode::DECAL,
                        RSSamplingOptions(
                            RSFilterMode::NEAREST, RSMipmapMode::NEAREST),
                        RSMatrix()));
                } },
            { "repeat-y",
                [](std::shared_ptr<RSImage> image, RSBrush& brush) {
                    brush.SetShaderEffect(RSShaderEffect::CreateImageShader(*image,
                        RSTileMode::DECAL, RSTileMode::REPEAT,
                        RSSamplingOptions(
                            RSFilterMode::NEAREST, RSMipmapMode::NEAREST),
                        RSMatrix()));
                } },
        };
    auto operatorIter = BinarySearchFindIndex(staticPattern, ArraySize(staticPattern), pattern.GetRepetition().c_str());
    if (operatorIter != -1) {
        staticPattern[operatorIter].value(image, brush);
    }
}
#endif

void RosenRenderCustomPaint::PutImageData(const Offset& offset, const ImageData& imageData)
{
    if (imageData.data.empty()) {
        LOGE("PutImageData failed, image data is empty.");
        return;
    }
    uint32_t* data = new (std::nothrow) uint32_t[imageData.data.size()];
    if (data == nullptr) {
        LOGE("PutImageData failed, new data is null.");
        return;
    }

    for (uint32_t i = 0; i < imageData.data.size(); ++i) {
        data[i] = imageData.data[i].GetValue();
    }
    SkBitmap skBitmap;
    auto imageInfo = SkImageInfo::Make(imageData.dirtyWidth, imageData.dirtyHeight, SkColorType::kBGRA_8888_SkColorType,
        SkAlphaType::kOpaque_SkAlphaType);
    skBitmap.allocPixels(imageInfo);
    skBitmap.setPixels(data);
#ifndef NEW_SKIA
    skCanvas_->drawBitmap(skBitmap, imageData.x, imageData.y);
#else
    skCanvas_->drawImage(skBitmap.asImage(), imageData.x, imageData.y, SkSamplingOptions());
#endif
    delete[] data;
}

std::unique_ptr<ImageData> RosenRenderCustomPaint::GetImageData(double left, double top, double width, double height)
{
    double viewScale = 1.0;
    auto pipeline = context_.Upgrade();
    if (pipeline) {
        viewScale = pipeline->GetViewScale();
    }
    // copy the bitmap to tempCanvas
    auto imageInfo =
        SkImageInfo::Make(width, height, SkColorType::kBGRA_8888_SkColorType, SkAlphaType::kOpaque_SkAlphaType);
    SkBitmap tempCache;
    double scaledLeft = left * viewScale;
    double scaledTop = top * viewScale;
    double dirtyWidth = width >= 0 ? width : 0;
    double dirtyHeight = height >= 0 ? height : 0;
    tempCache.allocPixels(imageInfo);
    int32_t size = dirtyWidth * dirtyHeight;
    bool isGpuEnable = false;
    const uint8_t* pixels = nullptr;
    SkCanvas tempCanvas(tempCache);
    auto srcRect = SkRect::MakeXYWH(scaledLeft, scaledTop, width * viewScale, height * viewScale);
    auto dstRect = SkRect::MakeXYWH(0.0, 0.0, dirtyWidth, dirtyHeight);
    if (!isGpuEnable) {
#ifndef NEW_SKIA
        tempCanvas.drawBitmapRect(canvasCache_, srcRect, dstRect, nullptr);
#else
        tempCanvas.drawImageRect(
            canvasCache_.asImage(), srcRect, dstRect, options_, nullptr, SkCanvas::kFast_SrcRectConstraint);
#endif
    }
    pixels = tempCache.pixmap().addr8();
    if (pixels == nullptr) {
        return nullptr;
    }
    std::unique_ptr<ImageData> imageData = std::make_unique<ImageData>();
    imageData->dirtyWidth = dirtyWidth;
    imageData->dirtyHeight = dirtyHeight;
    // a pixel include 4 data(blue, green, red, alpha)
    for (int i = 0; i < size * 4; i += 4) {
        auto blue = pixels[i];
        auto green = pixels[i + 1];
        auto red = pixels[i + 2];
        auto alpha = pixels[i + 3];
        imageData->data.emplace_back(Color::FromARGB(alpha, red, green, blue));
    }
    return imageData;
}

std::string RosenRenderCustomPaint::GetJsonData(const std::string& path)
{
    AssetImageLoader imageLoader;
    return imageLoader.LoadJsonData(path, context_.Upgrade());
}

void RosenRenderCustomPaint::WebGLInit(CanvasRenderContextBase* context)
{
    webGLContext_ = context;
    if (webGLContext_) {
        auto pipeline = context_.Upgrade();
        if (!pipeline) {
            return;
        }
        double viewScale = pipeline->GetViewScale();
        if (!webglBitmap_.readyToDraw()) {
            auto imageInfo =
                SkImageInfo::Make(GetLayoutSize().Width() * viewScale, GetLayoutSize().Height() * viewScale,
                    SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kOpaque_SkAlphaType);
            webglBitmap_.allocPixels(imageInfo);
#ifdef USE_SYSTEM_SKIA
            webglBitmap_.eraseColor(SK_ColorTRANSPARENT);
#endif
        }
        webGLContext_->SetBitMapPtr(
            reinterpret_cast<char*>(webglBitmap_.getPixels()), webglBitmap_.width(), webglBitmap_.height());
    }
}

void RosenRenderCustomPaint::WebGLUpdate()
{
    LOGD("RosenRenderCustomPaint::WebGLUpdate");
    if (skCanvas_ && webglBitmap_.readyToDraw()) {
        skCanvas_->save();
        /* Do mirror flip */

#ifndef NEW_SKIA
        skCanvas_->setMatrix(SkMatrix::MakeScale(1.0, -1.0));
        skCanvas_->drawBitmap(webglBitmap_, 0, -webglBitmap_.height());
#else
        skCanvas_->setMatrix(SkMatrix::Scale(1.0, -1.0));
        skCanvas_->drawImage(webglBitmap_.asImage(), 0, -webglBitmap_.height(), SkSamplingOptions());
#endif
        skCanvas_->restore();
    }
}

void RosenRenderCustomPaint::DrawBitmapMesh(
    const RefPtr<OffscreenCanvas>& offscreenCanvas, const std::vector<double>& mesh, int32_t column, int32_t row)
{
    std::unique_ptr<ImageData> imageData =
        offscreenCanvas->GetImageData(0, 0, offscreenCanvas->GetWidth(), offscreenCanvas->GetHeight());
    if (imageData != nullptr) {
        if (imageData->data.empty()) {
            LOGE("PutImageData failed, image data is empty.");
            return;
        }
        uint32_t* data = new (std::nothrow) uint32_t[imageData->data.size()];
        if (data == nullptr) {
            LOGE("PutImageData failed, new data is null.");
            return;
        }

        for (uint32_t i = 0; i < imageData->data.size(); ++i) {
            data[i] = imageData->data[i].GetValue();
        }
        SkBitmap skBitmap;
        auto imageInfo = SkImageInfo::Make(imageData->dirtyWidth, imageData->dirtyHeight,
            SkColorType::kBGRA_8888_SkColorType, SkAlphaType::kOpaque_SkAlphaType);
        skBitmap.allocPixels(imageInfo);
        skBitmap.setPixels(data);
        uint32_t size = mesh.size();
        float verts[size];
        for (uint32_t i = 0; i < size; i++) {
            verts[i] = mesh[i];
        }
        Mesh(skBitmap, column, row, verts, 0, nullptr);
        delete[] data;
    }
    LOGD("RosenRenderCustomPaint::DrawBitmapMesh");
}

void RosenRenderCustomPaint::Mesh(
    SkBitmap& bitmap, int column, int row, const float* vertices, const int* colors, const SkPaint* paint)
{
    const int vertCounts = (column + 1) * (row + 1);
    int32_t size = 6;
    const int indexCount = column * row * size;
    uint32_t flags = SkVertices::kHasTexCoords_BuilderFlag;
    if (colors) {
        flags |= SkVertices::kHasColors_BuilderFlag;
    }
    SkVertices::Builder builder(SkVertices::kTriangles_VertexMode, vertCounts, indexCount, flags);
    if (memcpy_s(builder.positions(), vertCounts * sizeof(SkPoint), vertices, vertCounts * sizeof(SkPoint)) != EOK) {
        return;
    }
    if (colors) {
        if (memcpy_s(builder.colors(), vertCounts * sizeof(SkColor), colors, vertCounts * sizeof(SkColor)) != EOK) {
            return;
        }
    }
    SkPoint* texsPoint = builder.texCoords();
    uint16_t* indices = builder.indices();
    const SkScalar height = SkIntToScalar(bitmap.height());
    const SkScalar width = SkIntToScalar(bitmap.width());
    if (row == 0) {
        LOGE("row is zero");
        return;
    }
    if (column == 0) {
        LOGE("column is zero");
        return;
    }
    const SkScalar dy = height / row;
    const SkScalar dx = width / column;

    SkPoint* texsPit = texsPoint;
    SkScalar y = 0;
    for (int i = 0; i <= row; i++) {
        if (i == row) {
            y = height; // to ensure numerically we hit h exactly
        }
        SkScalar x = 0;
        for (int j = 0; j < column; j++) {
            texsPit->set(x, y);
            texsPit += 1;
            x += dx;
        }
        texsPit->set(width, y);
        texsPit += 1;
        y += dy;
    }

    uint16_t* dexs = indices;
    int index = 0;
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < column; j++) {
            *dexs++ = index;
            *dexs++ = index + column + 1;
            *dexs++ = index + column + 2;

            *dexs++ = index;
            *dexs++ = index + column + 2;
            *dexs++ = index + 1;

            index += 1;
        }
        index += 1;
    }

    SkPaint tempPaint;
    if (paint) {
        tempPaint = *paint;
    }
    sk_sp<SkColorFilter> colorFter;
    sk_sp<SkShader> shader;
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
#ifdef USE_SYSTEM_SKIA
    shader = image->makeShader(SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
#elif defined(NEW_SKIA)
    shader = image->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, SkSamplingOptions());
#else
    shader = image->makeShader(SkTileMode::kClamp, SkTileMode::kClamp);
#endif
    if (colorFter) {
        shader = shader->makeWithColorFilter(colorFter);
    }
    tempPaint.setShader(shader);
    skCanvas_->drawVertices(builder.detach(), SkBlendMode::kModulate, tempPaint);
}

sk_sp<SkImage> RosenRenderCustomPaint::GetImage(const std::string& src)
{
    if (!imageCache_) {
        imageCache_ = ImageCache::Create();
        imageCache_->SetCapacity(IMAGE_CACHE_COUNT);
    }
    auto cacheImage = imageCache_->GetCacheImage(src);
    if (cacheImage && cacheImage->imagePtr) {
        return cacheImage->imagePtr;
    }

    auto context = GetContext().Upgrade();
    if (!context) {
        return nullptr;
    }

    auto image = ImageProvider::GetSkImage(src, context);
    if (image) {
        auto rasterizedImage = image->makeRasterImage();
        imageCache_->CacheImage(src, std::make_shared<CachedImage>(rasterizedImage));
        return rasterizedImage;
    }

    return image;
}
} // namespace OHOS::Ace
