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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CUSTOM_PAINT_CUSTOM_PAINT_PAINT_METHOD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CUSTOM_PAINT_CUSTOM_PAINT_PAINT_METHOD_H

#include "base/geometry/ng/offset_t.h"
#include "base/memory/ace_type.h"
#include "base/utils/macros.h"
#include "core/components/common/properties/paint_state.h"
#include "core/components_ng/image_provider/svg_dom_base.h"
#include "core/components_ng/pattern/custom_paint/canvas_modifier.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/node_paint_method.h"
#ifndef ACE_UNITTEST
#include "core/image/image_loader.h"
#include "core/image/image_object.h"
#include "core/image/image_source_info.h"
#endif
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
enum class FilterType {
    NONE,
    GRAYSCALE,
    SEPIA,
    INVERT,
    OPACITY,
    BRIGHTNESS,
    CONTRAST,
    BLUR,
    DROP_SHADOW,
    SATURATE,
    HUE_ROTATE
};

struct FilterProperty {
    FilterType filterType_;
    std::string filterParam_;
};

class CustomPaintPaintMethod : public NodePaintMethod {
    DECLARE_ACE_TYPE(CustomPaintPaintMethod, NodePaintMethod)
public:
    ~CustomPaintPaintMethod() override = default;

    RefPtr<Modifier> GetContentModifier(PaintWrapper* paintWrapper) override
    {
        return contentModifier_;
    }

    void SetFillRuleForPath(const CanvasFillRule& rule);
    void SetFillRuleForPath2D(const CanvasFillRule& rule);

    void FillRect(const Rect& rect);
    void StrokeRect(const Rect& rect);
    void ClearRect(const Rect& rect);
    void Fill();
    void Fill(const RefPtr<CanvasPath2D>& path);
    void Stroke();
    void Stroke(const RefPtr<CanvasPath2D>& path);
    void Clip();
    void Clip(const RefPtr<CanvasPath2D>& path);
    void BeginPath();
    void ClosePath();
    void MoveTo(double x, double y);
    void LineTo(double x, double y);
    void Arc(const ArcParam& param);
    void ArcTo(const ArcToParam& param);
    void AddRect(const Rect& rect);
    void Ellipse(const EllipseParam& param);
    void BezierCurveTo(const BezierCurveParam& param);
    void QuadraticCurveTo(const QuadraticCurveParam& param);
    void PutImageData(const Ace::ImageData& imageData);

    void Save();
    void Restore();
    void Scale(double x, double y);
    void Rotate(double angle);
    virtual void SetTransform(const TransformParam& param) = 0;
    virtual TransformParam GetTransform() const;
    void ResetTransform();
    void Transform(const TransformParam& param);
    void Translate(double x, double y);
    void SaveLayer();
    void RestoreLayer();

    void SetFilterParam(const std::string& filterStr)
    {
        filterParam_ = filterStr;
    }

    void SetAntiAlias(bool isEnabled)
    {
        antiAlias_ = isEnabled;
    }

    void SetFillColor(const Color& color)
    {
        state_.fillState.SetColor(color);
        state_.fillState.SetTextColor(color);
    }

    void SetFillPattern(const Ace::Pattern& pattern)
    {
        state_.fillState.SetPattern(pattern);
    }

    void SetFillPatternNG(const std::weak_ptr<Ace::Pattern>& pattern)
    {
        state_.fillState.SetPatternNG(pattern);
    }

    void SetFillGradient(const Ace::Gradient& gradient)
    {
        state_.fillState.SetGradient(gradient);
    }

    void SetAlpha(double alpha)
    {
        state_.globalState.SetAlpha(alpha);
    }

    void SetCompositeType(CompositeOperation operation)
    {
        state_.globalState.SetType(operation);
    }

    // direction is also available in strokeText
    void SetTextDirection(TextDirection direction)
    {
        state_.fillState.SetOffTextDirection(direction);
    }

    void SetStrokeColor(const Color& color)
    {
        state_.strokeState.SetColor(color);
    }

    void SetStrokePatternNG(const std::weak_ptr<Ace::Pattern>& pattern)
    {
        state_.strokeState.SetPatternNG(pattern);
    }

    void SetStrokePattern(const Ace::Pattern& pattern)
    {
        state_.strokeState.SetPattern(pattern);
    }

    void SetStrokeGradient(const Ace::Gradient& gradient)
    {
        state_.strokeState.SetGradient(gradient);
    }

    void SetLineCap(LineCapStyle style)
    {
        state_.strokeState.SetLineCap(style);
    }

    void SetLineDashOffset(double offset)
    {
        state_.strokeState.SetLineDashOffset(offset);
    }

    void SetLineJoin(LineJoinStyle style)
    {
        state_.strokeState.SetLineJoin(style);
    }

    void SetLineWidth(double width)
    {
        state_.strokeState.SetLineWidth(width);
    }

    void SetMiterLimit(double limit)
    {
        state_.strokeState.SetMiterLimit(limit);
    }

    LineDashParam GetLineDash() const
    {
        return state_.strokeState.GetLineDash();
    }

    void SetLineDash(const std::vector<double>& segments)
    {
        state_.strokeState.SetLineDash(segments);
    }

    void SetTextAlign(TextAlign align)
    {
        state_.fillState.SetTextAlign(align);
        state_.strokeState.SetTextAlign(align);
    }

    void SetTextBaseline(TextBaseline baseline)
    {
        state_.fillState.SetTextBaseline(baseline);
        state_.strokeState.SetTextBaseline(baseline);
    }

    void SetShadowColor(const Color& color)
    {
        state_.shadow.SetColor(color);
    }

    void SetShadowBlur(double blur)
    {
        state_.shadow.SetBlurRadius(blur);
    }

    void SetShadowOffsetX(double x)
    {
        state_.shadow.SetOffsetX(x);
    }

    void SetShadowOffsetY(double y)
    {
        state_.shadow.SetOffsetY(y);
    }

    void SetSmoothingEnabled(bool enabled)
    {
        smoothingEnabled_ = enabled;
    }

    void SetSmoothingQuality(const std::string& quality)
    {
        smoothingQuality_ = quality;
    }

    void SetFontSize(const Dimension& size)
    {
        state_.fillState.SetFontSize(size);
        state_.strokeState.SetFontSize(size);
    }

    void SetFontStyle(OHOS::Ace::FontStyle style)
    {
        state_.fillState.SetFontStyle(style);
        state_.strokeState.SetFontStyle(style);
    }

    void SetFontWeight(FontWeight weight)
    {
        state_.fillState.SetFontWeight(weight);
        state_.strokeState.SetFontWeight(weight);
    }

    void SetFontFamilies(const std::vector<std::string>& fontFamilies)
    {
        state_.fillState.SetFontFamilies(fontFamilies);
        state_.strokeState.SetFontFamilies(fontFamilies);
    }

    void FlushPipelineImmediately()
    {
        auto context = context_.Upgrade();
        if (context) {
            context->FlushPipelineImmediately();
        }
    }

    void FlushUITasks()
    {
        auto context = context_.Upgrade();
        if (context) {
            context->FlushUITasks();
        }
    }

    void SaveMatrix();
    void RestoreMatrix();
    void ResetTransformMatrix();
    void RotateMatrix(double angle);
    void ScaleMatrix(double x, double y);
    void SetTransformMatrix(const TransformParam& param);
    void TransformMatrix(const TransformParam& param);
    void TranslateMatrix(double tx, double ty);
    void DrawSvgImage(RefPtr<SvgDomBase> svgDom, const Ace::CanvasImage& canvasImage,
        const ImageFit& imageFit);
    void DrawImage(const Ace::CanvasImage& canvasImage, double width, double height);

    void UpdateRecordingCanvas(float width, float height);
    void SetRSCanvasCallback(std::function<void(RSCanvas*, double, double)>& callback);

protected:
    std::optional<double> CalcTextScale(double maxIntrinsicWidth, std::optional<double> maxWidth);
    bool HasShadow() const;
    void UpdateLineDash(RSPen& pen);
    void UpdatePaintShader(RSPen* pen, RSBrush* brush, const Ace::Gradient& gradient);
    void UpdatePaintShader(const Ace::Pattern& pattern, RSPen* pen, RSBrush* brush);
    void InitPaintBlend(RSBrush& brush);
    void InitPaintBlend(RSPen& pen);
    std::shared_ptr<RSShaderEffect> MakeConicGradient(RSBrush* brush, const Ace::Gradient& gradient);

    void Path2DFill();
    void Path2DStroke();
    void Path2DClip();
    void ParsePath2D(const RefPtr<CanvasPath2D>& path);
    void Path2DAddPath(const PathArgs& args);
    void Path2DClosePath();
    void Path2DMoveTo(const PathArgs& args);
    void Path2DLineTo(const PathArgs& args);
    void Path2DArc(const PathArgs& args);
    void Path2DArcTo(const PathArgs& args);
    virtual void Path2DRect(const PathArgs& args) = 0;
    void Path2DEllipse(const PathArgs& args);
    void Path2DBezierCurveTo(const PathArgs& args);
    void Path2DQuadraticCurveTo(const PathArgs& args);
    void Path2DSetTransform(const PathArgs& args);
    RSMatrix GetMatrixFromPattern(const Ace::Pattern& pattern);

    void SetGrayFilter(const std::string& percent, RSPen* pen, RSBrush* brush);
    void SetSepiaFilter(const std::string& percent, RSPen* pen, RSBrush* brush);
    void SetSaturateFilter(const std::string& percent, RSPen* pen, RSBrush* brush);
    void SetHueRotateFilter(const std::string& percent, RSPen* pen, RSBrush* brush);
    void SetInvertFilter(const std::string& percent, RSPen* pen, RSBrush* brush);
    void SetOpacityFilter(const std::string& percent, RSPen* pen, RSBrush* brush);
    void SetBrightnessFilter(const std::string& percent, RSPen* pen, RSBrush* brush);
    void SetContrastFilter(const std::string& percent, RSPen* pen, RSBrush* brush);
    void SetBlurFilter(const std::string& percent, RSPen* pen, RSBrush* brush);

    void SetColorFilter(float matrix[20], RSPen* pen, RSBrush* brush);

    bool GetFilterType(std::vector<FilterProperty>& filters);
    bool IsPercentStr(std::string& percentStr);
    double PxStrToDouble(const std::string& str);
    double BlurStrToDouble(const std::string& str);
    bool CheckNumberAndPercentage(const std::string& param, bool isClamped, float& result);
    void InitImagePaint(RSPen* pen, RSBrush* brush, RSSamplingOptions& options);
    void GetStrokePaint(RSPen& pen, RSSamplingOptions& options);
    void InitImageCallbacks();

    void SetPaintImage(RSPen* pen, RSBrush* brush);
    void ClearPaintImage(RSPen* pen, RSBrush* brush);
    float PercentStrToFloat(const std::string& percentStr);
    bool CheckFilterProperty(FilterType filterType, const std::string& filterParam);
    bool ParseFilter(std::string& filter, std::vector<FilterProperty>& filters);
    FilterType FilterStrToFilterType(const std::string& filterStr);
    bool HasImageShadow() const;

    virtual void ImageObjReady(const RefPtr<Ace::ImageObject>& imageObj) = 0;
    virtual void ImageObjFailed() = 0;
    std::shared_ptr<RSImage> GetImage(const std::string& src);
#ifndef ACE_UNITTEST
    void GetSvgRect(const sk_sp<SkSVGDOM>& skiaDom, const Ace::CanvasImage& canvasImage,
        RSRect* srcRect, RSRect* dstRect);
#endif
    void DrawSvgImage(const Ace::CanvasImage& canvasImage);
    virtual RSCanvas* GetRawPtrOfRSCanvas() = 0;
    virtual void PaintShadow(const RSPath& path, const Shadow& shadow, RSCanvas* canvas,
        const RSBrush* brush = nullptr, const RSPen* pen = nullptr, RSSaveLayerOps* slo = nullptr) = 0;
    virtual void PaintImageShadow(const RSPath& path, const Shadow& shadow, RSCanvas* canvas,
        const RSBrush* brush = nullptr, const RSPen* pen = nullptr, RSSaveLayerOps* slo = nullptr) = 0;
    void PaintText(const float width, double x, double y,
        std::optional<double> maxWidth, bool isStroke, bool hasShadow = false);
    double GetAlignOffset(TextAlign align, double width);
    double GetBaselineOffset(TextBaseline baseline, std::unique_ptr<RSParagraph>& paragraph);
    RSTextAlign GetEffectiveAlign(RSTextAlign align, RSTextDirection direction) const;
#ifndef ACE_UNITTEST
    double GetFontBaseline(const Rosen::Drawing::FontMetrics& fontMetrics, TextBaseline baseline) const;
    double GetFontAlign(TextAlign align, std::unique_ptr<RSParagraph>& paragraph) const;
#endif
    void ResetStates();
    void DrawImageInternal(const Ace::CanvasImage& canvasImage, const std::shared_ptr<RSImage>& image);

    // PaintHolder includes fillState, strokeState, globalState and shadow for save
    PaintHolder state_;
    std::vector<PaintHolder> saveStates_;
    RSMatrix matrix_;
    std::vector<RSMatrix> matrixStates_;

    bool smoothingEnabled_ = true;
    std::string smoothingQuality_ = "low";
    bool antiAlias_ = false;
    std::function<void(RSCanvas*, double, double)> canvasCallback_ = nullptr;
    std::unique_ptr<RSParagraph> paragraph_;

    WeakPtr<PipelineBase> context_;

    RSPath rsPath_;
    RSPath rsPath2d_;
    RSBrush imageBrush_;
    RSSamplingOptions sampleOptions_;
    std::shared_ptr<RSCanvas> rsCanvas_;

    Ace::CanvasImage canvasImage_;
    std::string filterParam_ = "";
    std::unique_ptr<Shadow> imageShadow_;

#ifndef ACE_UNITTEST
    sk_sp<SkSVGDOM> skiaDom_ = nullptr;
    ImageSourceInfo currentSource_;
    ImageSourceInfo loadingSource_;
    ImageObjSuccessCallback imageObjSuccessCallback_;
    UploadSuccessCallback uploadSuccessCallback_;
    OnPostBackgroundTask onPostBackgroundTask_;
    FailedCallback failedCallback_;
#endif

    RefPtr<CanvasModifier> contentModifier_;
    std::shared_ptr<RSRecordingCanvas> rsRecordingCanvas_;

    SizeF lastLayoutSize_;
    RefPtr<ImageCache> imageCache_;
    enum DrawImageType {
        THREE_PARAMS,
        FIVE_PARAMS,
        NINE_PARAMS,
    };
    static const LinearMapNode<void (*)(std::shared_ptr<RSImage>&, std::shared_ptr<RSShaderEffect>&, RSMatrix&)>
        staticPattern[];
    std::vector<FilterProperty> lastFilters_;
    const float defaultOpacity = 1.0f;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CUSTOM_PAINT_CUSTOM_PAINT_PAINT_METHOD_H
