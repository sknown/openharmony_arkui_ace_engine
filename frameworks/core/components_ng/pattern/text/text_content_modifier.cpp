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

#include "core/components_ng/pattern/text/text_content_modifier.h"

#include "base/log/ace_trace.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/render/animation_utils.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/components_ng/render/image_painter.h"
#include "core/components_v2/inspector/utils.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/core/components_ng/render/adapter/pixelmap_image.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float RACE_DURATION_RATIO = 85.0f;
constexpr float RACE_MOVE_PERCENT_MIN = 0.0f;
constexpr float RACE_MOVE_PERCENT_MAX = 100.0f;
constexpr float RACE_SPACE_WIDTH = 48.0f;
constexpr float ROUND_VALUE = 0.5f;
constexpr float RACE_MIN_GRADIENTPERCENT = 0.5f;
constexpr uint32_t POINT_COUNT = 4;
constexpr float OBSCURED_ALPHA = 0.2f;
const FontWeight FONT_WEIGHT_CONVERT_MAP[] = {
    FontWeight::W100,
    FontWeight::W200,
    FontWeight::W300,
    FontWeight::W400,
    FontWeight::W500,
    FontWeight::W600,
    FontWeight::W700,
    FontWeight::W800,
    FontWeight::W900,
    FontWeight::W700,       // FontWeight::BOLD
    FontWeight::W400,       // FontWeight::NORMAL
    FontWeight::W900,       // FontWeight::BOLDER,
    FontWeight::W100,       // FontWeight::LIGHTER
    FontWeight::W500,       // FontWeight::MEDIUM
    FontWeight::W400,       // FontWeight::REGULAR
};

inline FontWeight ConvertFontWeight(FontWeight fontWeight)
{
    return FONT_WEIGHT_CONVERT_MAP[(int)fontWeight];
}
} // namespace

TextContentModifier::TextContentModifier(const std::optional<TextStyle>& textStyle, const WeakPtr<Pattern>& pattern)
    : pattern_(pattern)
{
    contentChange_ = MakeRefPtr<PropertyInt>(0);
    AttachProperty(contentChange_);
    contentOffset_ = MakeRefPtr<PropertyOffsetF>(OffsetF());
    contentSize_ = MakeRefPtr<PropertySizeF>(SizeF());
    AttachProperty(contentOffset_);
    AttachProperty(contentSize_);
    dragStatus_ = MakeRefPtr<PropertyBool>(false);
    AttachProperty(dragStatus_);
    if (textStyle.has_value()) {
        SetDefaultAnimatablePropertyValue(textStyle.value());
    }

    racePercentFloat_ = MakeRefPtr<AnimatablePropertyFloat>(0.0f);
    AttachProperty(racePercentFloat_);
    if (Container::LessThanAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
        clip_ = MakeRefPtr<PropertyBool>(true);
    } else {
        clip_ = MakeRefPtr<PropertyBool>(false);
    }
    AttachProperty(clip_);
    fontFamilyString_ = MakeRefPtr<PropertyString>("");
    AttachProperty(fontFamilyString_);
    fontReady_ = MakeRefPtr<PropertyBool>(false);
    AttachProperty(fontReady_);
}

void TextContentModifier::ChangeDragStatus()
{
    dragStatus_->Set(!dragStatus_->Get());
}

void TextContentModifier::SetDefaultAnimatablePropertyValue(const TextStyle& textStyle)
{
    SetDefaultFontSize(textStyle);
    SetDefaultAdaptMinFontSize(textStyle);
    SetDefaultAdaptMaxFontSize(textStyle);
    SetDefaultFontWeight(textStyle);
    SetDefaultTextColor(textStyle);
    SetDefaultTextShadow(textStyle);
    SetDefaultTextDecoration(textStyle);
    SetDefaultBaselineOffset(textStyle);
}

void TextContentModifier::SetDefaultFontSize(const TextStyle& textStyle)
{
    float fontSizeValue = textStyle.GetFontSize().Value();
    auto pipelineContext = PipelineContext::GetCurrentContext();
    if (pipelineContext) {
        fontSizeValue = pipelineContext->NormalizeToPx(textStyle.GetFontSize());
        if (textStyle.IsAllowScale() && textStyle.GetFontSize().Unit() == DimensionUnit::FP) {
            fontSizeValue = pipelineContext->NormalizeToPx(textStyle.GetFontSize() * pipelineContext->GetFontScale());
        }
    }

    fontSizeFloat_ = MakeRefPtr<AnimatablePropertyFloat>(fontSizeValue);
    AttachProperty(fontSizeFloat_);
}

void TextContentModifier::SetDefaultAdaptMinFontSize(const TextStyle& textStyle)
{
    float fontSizeValue = textStyle.GetFontSize().Value();
    auto pipelineContext = PipelineContext::GetCurrentContext();
    if (pipelineContext) {
        fontSizeValue = textStyle.GetAdaptMinFontSize().ConvertToPx();
        if (textStyle.IsAllowScale() && textStyle.GetAdaptMinFontSize().Unit() == DimensionUnit::FP) {
            fontSizeValue = (textStyle.GetAdaptMinFontSize() * pipelineContext->GetFontScale()).ConvertToPx();
        }
    }

    adaptMinFontSizeFloat_ = MakeRefPtr<AnimatablePropertyFloat>(fontSizeValue);
    AttachProperty(adaptMinFontSizeFloat_);
}

void TextContentModifier::SetDefaultAdaptMaxFontSize(const TextStyle& textStyle)
{
    float fontSizeValue = textStyle.GetFontSize().Value();
    auto pipelineContext = PipelineContext::GetCurrentContext();
    if (pipelineContext) {
        fontSizeValue = textStyle.GetAdaptMaxFontSize().ConvertToPx();
        if (textStyle.IsAllowScale() && textStyle.GetAdaptMaxFontSize().Unit() == DimensionUnit::FP) {
            fontSizeValue = (textStyle.GetAdaptMaxFontSize() * pipelineContext->GetFontScale()).ConvertToPx();
        }
    }

    adaptMaxFontSizeFloat_ = MakeRefPtr<AnimatablePropertyFloat>(fontSizeValue);
    AttachProperty(adaptMaxFontSizeFloat_);
}

void TextContentModifier::SetDefaultFontWeight(const TextStyle& textStyle)
{
    fontWeightFloat_ =
        MakeRefPtr<AnimatablePropertyFloat>(static_cast<float>(ConvertFontWeight(textStyle.GetFontWeight())));
    AttachProperty(fontWeightFloat_);
}

void TextContentModifier::SetDefaultTextColor(const TextStyle& textStyle)
{
    animatableTextColor_ = MakeRefPtr<AnimatablePropertyColor>(LinearColor(textStyle.GetTextColor()));
    AttachProperty(animatableTextColor_);
}

void TextContentModifier::SetDefaultTextShadow(const TextStyle& textStyle)
{
    auto&& textShadows = textStyle.GetTextShadows();
    if (textShadows.empty()) {
        AddDefaultShadow();
        return;
    }
    shadows_.clear();
    shadows_.reserve(textShadows.size());
    for (auto&& textShadow : textShadows) {
        AddShadow(textShadow);
    }
}

void TextContentModifier::AddShadow(const Shadow& shadow)
{
    auto shadowBlurRadiusFloat = MakeRefPtr<AnimatablePropertyFloat>(shadow.GetBlurRadius());
    auto shadowOffsetXFloat = MakeRefPtr<AnimatablePropertyFloat>(shadow.GetOffset().GetX());
    auto shadowOffsetYFloat = MakeRefPtr<AnimatablePropertyFloat>(shadow.GetOffset().GetY());
    auto shadowColor = MakeRefPtr<AnimatablePropertyColor>(LinearColor(shadow.GetColor()));
    Shadow textShadow;
    textShadow.SetBlurRadius(shadow.GetBlurRadius());
    textShadow.SetOffset(shadow.GetOffset());
    textShadow.SetColor(shadow.GetColor());
    shadows_.emplace_back(ShadowProp { .shadow = textShadow,
        .blurRadius = shadowBlurRadiusFloat,
        .offsetX = shadowOffsetXFloat,
        .offsetY = shadowOffsetYFloat,
        .color = shadowColor });
    AttachProperty(shadowBlurRadiusFloat);
    AttachProperty(shadowOffsetXFloat);
    AttachProperty(shadowOffsetYFloat);
    AttachProperty(shadowColor);
}

void TextContentModifier::SetDefaultTextDecoration(const TextStyle& textStyle)
{
    textDecoration_ = textStyle.GetTextDecoration();
    textDecorationStyle_ = textStyle.GetTextDecorationStyle();
    textDecorationColor_ = textStyle.GetTextDecorationColor();
    textDecorationColorAlpha_ = MakeRefPtr<AnimatablePropertyFloat>(
        textDecoration_ == TextDecoration::NONE ? 0.0f : textDecorationColor_->GetAlpha());
    AttachProperty(textDecorationColorAlpha_);
}
void TextContentModifier::SetDefaultBaselineOffset(const TextStyle& textStyle)
{
    float baselineOffset = textStyle.GetBaselineOffset().Value();
    auto pipelineContext = PipelineContext::GetCurrentContext();
    if (pipelineContext) {
        baselineOffset = pipelineContext->NormalizeToPx(textStyle.GetBaselineOffset());
    }

    baselineOffsetFloat_ = MakeRefPtr<AnimatablePropertyFloat>(baselineOffset);
    AttachProperty(baselineOffsetFloat_);
}

void TextContentModifier::SetClip(bool clip)
{
    if (clip_) {
        clip_->Set(clip);
    }
}

void TextContentModifier::SetFontReady(bool value)
{
    if (fontReady_) {
        fontReady_->Set(value);
    }
}

void TextContentModifier::UpdateImageNodeVisible(const VisibleType visible)
{
    for (const auto& imageWeak : imageNodeList_) {
        auto imageNode = imageWeak.Upgrade();
        if (!imageNode) {
            continue;
        }
        auto layoutProperty = imageNode->GetLayoutProperty();
        if (!layoutProperty) {
            continue;
        }
        layoutProperty->UpdateVisibility(visible, true);
    }
}

void TextContentModifier::PaintImage(RSCanvas& canvas, float x, float y)
{
    if (!AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        return;
    }
    auto pattern = DynamicCast<TextPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(pattern);
    size_t index = 0;
    auto rectsForPlaceholders = pattern->GetRectsForPlaceholders();
    auto placeHolderIndexVector = pattern->GetPlaceHolderIndex();
    for (const auto& imageWeak : imageNodeList_) {
        auto imageChild = imageWeak.Upgrade();
        if (!imageChild) {
            continue;
        }
        auto rect = rectsForPlaceholders.at(placeHolderIndexVector.at(index));
        if (!DrawImage(imageChild, canvas, x, y, rect)) {
            continue;
        }
        ++index;
    }
}

bool TextContentModifier::DrawImage(const RefPtr<FrameNode>& imageNode, RSCanvas& canvas, float x, float y,
    const RectF& rect)
{
    CHECK_NULL_RETURN(imageNode, false);
    auto imagePattern = DynamicCast<ImagePattern>(imageNode->GetPattern());
    if (!imagePattern) {
        return false;
    }
    auto layoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    if (!layoutProperty) {
        return false;
    }
    const auto& marginProperty = layoutProperty->GetMarginProperty();
    float marginTop = 0.0f;
    float marginLeft = 0.0f;
    if (marginProperty) {
        marginLeft = marginProperty->left.has_value() ? marginProperty->left->GetDimension().ConvertToPx() : 0.0f;
        marginTop = marginProperty->top.has_value() ? marginProperty->top->GetDimension().ConvertToPx() : 0.0f;
    }
    auto canvasImage = imagePattern->GetCanvasImage();
    if (!canvasImage) {
        canvasImage = imagePattern->GetAltCanvasImage();
    }
    auto geometryNode = imageNode->GetGeometryNode();
    if (!canvasImage || !geometryNode) {
        return false;
    }
    RectF imageRect(rect.Left() + x + marginLeft, rect.Top() + y + marginTop,
        geometryNode->GetFrameSize().Width(), geometryNode->GetFrameSize().Height());
    const auto config = canvasImage->GetPaintConfig();
    if (config.isSvg_) {
        ImagePainter imagePainter(canvasImage);
        OffsetF offset(0, 0);
        SizeF contentSize(geometryNode->GetFrameSize().Width(), geometryNode->GetFrameSize().Height());
        auto clipRect = RSRect(0, 0, geometryNode->GetFrameSize().Width(), geometryNode->GetFrameSize().Height());
        canvas.Save();
        canvas.Translate(rect.Left() + x + marginLeft, rect.Top() + y + marginTop);
        canvas.ClipRect(clipRect, RSClipOp::INTERSECT);
        imagePainter.DrawSVGImage(canvas, offset, contentSize);
        canvas.Restore();
    } else {
        auto pixelMapImage = DynamicCast<PixelMapImage>(canvasImage);
        if (!pixelMapImage) {
            return false;
        }
        pixelMapImage->DrawRect(canvas, ToRSRect(imageRect));
    }
    return true;
}

void TextContentModifier::PaintCustomSpan(DrawingContext& drawingContext)
{
    auto pattern = DynamicCast<TextPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(pattern);
    auto pManager = pattern->GetParagraphManager();
    CHECK_NULL_VOID(pManager);
    auto rectsForPlaceholders = pattern->GetRectsForPlaceholders();
    auto customSpanPlaceholderInfo = pattern->GetCustomSpanPlaceholderInfo();
    auto x = paintOffset_.GetX();
    auto y = paintOffset_.GetY();
    auto rectsForPlaceholderSize = rectsForPlaceholders.size();
    for (auto customSpanPlaceholder : customSpanPlaceholderInfo) {
        if (!customSpanPlaceholder.onDraw) {
            continue;
        }
        auto index = customSpanPlaceholder.customSpanIndex;
        if (index >= rectsForPlaceholderSize) {
            return;
        }
        auto rect = rectsForPlaceholders.at(index);
        auto lineMetrics = pManager->GetLineMetricsByRectF(rect, customSpanPlaceholder.paragraphIndex);
        CustomSpanOptions customSpanOptions;
        customSpanOptions.x = static_cast<double>(rect.Left()) + x;
        customSpanOptions.lineTop = lineMetrics.y + y;
        customSpanOptions.lineBottom = lineMetrics.y + lineMetrics.height + y;
        customSpanOptions.baseline = lineMetrics.y + lineMetrics.ascender + y;
        customSpanPlaceholder.onDraw(drawingContext, customSpanOptions);
    }
}

void TextContentModifier::onDraw(DrawingContext& drawingContext)
{
    auto textPattern = DynamicCast<TextPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textPattern);
    ACE_SCOPED_TRACE("Text paint offset=[%f,%f]", paintOffset_.GetX(), paintOffset_.GetY());
    bool ifPaintObscuration = std::any_of(obscuredReasons_.begin(), obscuredReasons_.end(),
        [](const auto& reason) { return reason == ObscuredReasons::PLACEHOLDER; });
    auto pManager = textPattern->GetParagraphManager();
    CHECK_NULL_VOID(pManager);
    CHECK_NULL_VOID(!pManager->GetParagraphs().empty());

    auto info = GetFadeoutInfo(drawingContext);
    bool isDrawNormal = !ifPaintObscuration || ifHaveSpanItemChildren_;
    if (!info.IsFadeount()) {
        if (isDrawNormal) {
            DrawNormal(drawingContext);
        } else {
            DrawObscuration(drawingContext);
        }
        PaintCustomSpan(drawingContext);
    } else {
        DrawFadeout(drawingContext, info, isDrawNormal);
    }
}

FadeoutInfo TextContentModifier::GetFadeoutInfo(DrawingContext& drawingContext)
{
    FadeoutInfo info;

    auto textPattern = DynamicCast<TextPattern>(pattern_.Upgrade());
    CHECK_NULL_RETURN(textPattern, info);
    auto pManager = textPattern->GetParagraphManager();
    CHECK_NULL_RETURN(pManager, info);

    if (!marqueeSet_ || !marqueeOption_.fadeout) {
        return info;
    }

    if (marqueeGradientPercent_ > RACE_MIN_GRADIENTPERCENT) {
        return info;
    }

    bool isOverlength = pManager->GetTextWidth() > drawingContext.width;

    if (!marqueeOption_.start) {
        info.isLeftFadeout = false;
        info.isRightFadeout = isOverlength;
        info.fadeoutPercent = isOverlength ? marqueeGradientPercent_ : 0;
        return info;
    }

    float raceLength = pManager->GetTextWidth() + textRaceSpaceWidth_;
    float spacePercent = 0;
    float textPercent = 0;
    float drawPercent = 0;
    if (raceLength > 0) {
        spacePercent = textRaceSpaceWidth_ / raceLength * RACE_MOVE_PERCENT_MAX;
        textPercent = pManager->GetTextWidth() / raceLength * RACE_MOVE_PERCENT_MAX;
        drawPercent = drawingContext.width / raceLength * RACE_MOVE_PERCENT_MAX;
    }

    float racePercent = GetTextRacePercent();

    info.fadeoutPercent = marqueeGradientPercent_;
    if (!textRacing_) {
        info.isLeftFadeout = false;
        info.isRightFadeout = isOverlength;
    } else {
        if (marqueeOption_.direction == MarqueeDirection::RIGHT) {
            info.isLeftFadeout = (racePercent > spacePercent) && !NearEqual(racePercent, RACE_MOVE_PERCENT_MAX);
            info.isRightFadeout = !((racePercent > (drawPercent - spacePercent)) && (racePercent < drawPercent));
        } else {
            info.isLeftFadeout = !NearEqual(racePercent, 0.0) && (racePercent < (RACE_MOVE_PERCENT_MAX - spacePercent));
            info.isRightFadeout =
                !((racePercent > (textPercent - drawPercent)) && (racePercent < (RACE_MOVE_PERCENT_MAX - drawPercent)));
        }
    }

    return info;
}

void TextContentModifier::DrawNormal(DrawingContext& drawingContext)
{
    auto textPattern = DynamicCast<TextPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textPattern);
    auto pManager = textPattern->GetParagraphManager();
    CHECK_NULL_VOID(pManager);
    CHECK_NULL_VOID(!pManager->GetParagraphs().empty());

    auto& canvas = drawingContext.canvas;
    auto contentSize = contentSize_->Get();
    auto contentOffset = contentOffset_->Get();
    canvas.Save();
    if (clip_ && clip_->Get() &&
        (!fontSize_.has_value() || !fontSizeFloat_ ||
            NearEqual(fontSize_.value().Value(), fontSizeFloat_->Get()))) {
        RSRect clipInnerRect = RSRect(contentOffset.GetX(), contentOffset.GetY(),
            contentSize.Width() + contentOffset.GetX(), contentSize.Height() + contentOffset.GetY());
        canvas.ClipRect(clipInnerRect, RSClipOp::INTERSECT);
    }
    if (!textRacing_) {
        auto paintOffsetY = paintOffset_.GetY();
        auto paragraphs = pManager->GetParagraphs();
        for (auto && info : paragraphs) {
            auto paragraph = info.paragraph;
            CHECK_NULL_VOID(paragraph);
            paragraph->Paint(canvas, paintOffset_.GetX(), paintOffsetY);
            paintOffsetY += paragraph->GetHeight();
        }
        if (marqueeSet_) {
            PaintImage(drawingContext.canvas, paintOffset_.GetX(), paintOffset_.GetY());
        }
    } else {
        // Racing
        float textRacePercent = marqueeOption_.direction == MarqueeDirection::LEFT
                                    ? GetTextRacePercent()
                                    : RACE_MOVE_PERCENT_MAX - GetTextRacePercent();
        auto paragraph = pManager->GetParagraphs().front().paragraph;
        float paragraph1Offset =
            (paragraph->GetTextWidth() + textRaceSpaceWidth_) * textRacePercent / RACE_MOVE_PERCENT_MAX * -1;
        if ((paintOffset_.GetX() + paragraph1Offset + paragraph->GetTextWidth()) > 0) {
            paragraph->Paint(drawingContext.canvas, paintOffset_.GetX() + paragraph1Offset, paintOffset_.GetY());
            PaintImage(drawingContext.canvas, paintOffset_.GetX() + paragraph1Offset, paintOffset_.GetY());
        }
        float paragraph2Offset = paragraph1Offset + paragraph->GetTextWidth() + textRaceSpaceWidth_;
        if ((paintOffset_.GetX() + paragraph2Offset) < drawingContext.width) {
            paragraph->Paint(drawingContext.canvas, paintOffset_.GetX() + paragraph2Offset, paintOffset_.GetY());
            PaintImage(drawingContext.canvas, paintOffset_.GetX() + paragraph2Offset, paintOffset_.GetY());
        }
    }
    canvas.Restore();
}

void TextContentModifier::DrawFadeout(DrawingContext& drawingContext, const FadeoutInfo& info, const bool& isDrawNormal)
{
    auto textPattern = DynamicCast<TextPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textPattern);

    RSCanvas& canvas = drawingContext.canvas;
    auto contentRect = textPattern->GetTextContentRect();
    RSRect clipInnerRect = RSRect(0, 0, contentRect.Width(), contentRect.Height());

    RSSaveLayerOps slo(&clipInnerRect, nullptr);
    canvas.SaveLayer(slo);

    if (isDrawNormal) {
        DrawNormal(drawingContext);
    } else {
        DrawObscuration(drawingContext);
    }
    PaintCustomSpan(drawingContext);

    RSBrush brush;
    std::vector<RSPoint> points = { RSPoint(0, 0.0f), RSPoint(contentRect.Width(), 0.0f) };
    std::vector<RSColorQuad> colors = { Color::TRANSPARENT.GetValue(), Color::WHITE.GetValue(), Color::WHITE.GetValue(),
        Color::TRANSPARENT.GetValue() };
    std::vector<RSScalar> pos = { 0.0f, info.isLeftFadeout ? info.fadeoutPercent : 0.0f,
        info.isRightFadeout ? (1 - info.fadeoutPercent) : 1.0f, 1.0f };
    brush.SetShaderEffect(
        RSShaderEffect::CreateLinearGradient(points.at(0), points.at(1), colors, pos, RSTileMode::CLAMP));
    brush.SetBlendMode(RSBlendMode::DST_IN);
    canvas.AttachBrush(brush);
    canvas.DrawRect(clipInnerRect);
    canvas.Restore();
}

void TextContentModifier::DrawObscuration(DrawingContext& drawingContext)
{
    RSCanvas& canvas = drawingContext.canvas;
    RSBrush brush;
    std::vector<RSPoint> radiusXY(POINT_COUNT);
    Dimension borderRadius = Dimension(2.0, DimensionUnit::VP);
    for (auto& radius : radiusXY) {
        radius.SetX(static_cast<float>(borderRadius.ConvertToPx()));
        radius.SetY(static_cast<float>(borderRadius.ConvertToPx()));
    }
    CHECK_NULL_VOID(animatableTextColor_);
    Color fillColor = Color(animatableTextColor_->Get().GetValue());
    RSColor rrSColor(fillColor.GetRed(), fillColor.GetGreen(), fillColor.GetBlue(),
        (uint32_t)(fillColor.GetAlpha() * OBSCURED_ALPHA));
    brush.SetColor(rrSColor);
    brush.SetAntiAlias(true);
    canvas.AttachBrush(brush);
    CHECK_NULL_VOID(fontSizeFloat_);
    float fontSize = fontSizeFloat_->Get();
    std::vector<float> textLineWidth;
    float currentLineWidth = 0.0f;
    int32_t maxLineCount = 0;
    CHECK_NULL_VOID(contentSize_);
    CHECK_NULL_VOID(contentOffset_);
    for (auto i = 0U; i < drawObscuredRects_.size(); i++) {
        if (!NearEqual(drawObscuredRects_[i].Width(), 0.0f) && !NearEqual(drawObscuredRects_[i].Height(), 0.0f)) {
            currentLineWidth += drawObscuredRects_[i].Width();
            if (i == (drawObscuredRects_.size() ? drawObscuredRects_.size() - 1 : 0)) {
                textLineWidth.push_back(currentLineWidth);
                maxLineCount += LessOrEqual(drawObscuredRects_[i].Bottom(), contentSize_->Get().Height()) ? 1 : 0;
            } else if (!NearEqual(drawObscuredRects_[i].Bottom(), drawObscuredRects_[i + 1].Bottom())) {
                textLineWidth.push_back(currentLineWidth);
                maxLineCount += LessOrEqual(drawObscuredRects_[i].Bottom(), contentSize_->Get().Height()) ? 1 : 0;
                currentLineWidth = 0;
            } else {
                /** nothing to do **/
            }
        }
    }
    auto baselineOffset = baselineOffsetFloat_->Get();
    int32_t obscuredLineCount = std::min(maxLineCount, static_cast<int32_t>(textLineWidth.size()));
    float offsetY = (contentSize_->Get().Height() - std::fabs(baselineOffset) - (obscuredLineCount * fontSize)) /
                    (obscuredLineCount + 1);
    for (auto i = 0; i < obscuredLineCount; i++) {
        RSRoundRect rSRoundRect(
            RSRect(contentOffset_->Get().GetX(),
                contentOffset_->Get().GetY() - std::min(baselineOffset, 0.0f) +
                    std::max(offsetY + ((offsetY + fontSize) * i), 0.0f),
                contentOffset_->Get().GetX() + std::min(textLineWidth[i], contentSize_->Get().Width()),
                contentOffset_->Get().GetY() - std::min(baselineOffset, 0.0f) +
                    std::min(offsetY + ((offsetY + fontSize) * i) + fontSize, contentSize_->Get().Height())),
            radiusXY);
        canvas.DrawRoundRect(rSRoundRect);
    }
    canvas.DetachBrush();
}

void TextContentModifier::ModifyFontSizeInTextStyle(TextStyle& textStyle)
{
    if (fontSize_.has_value() && fontSizeFloat_) {
        textStyle.SetFontSize(Dimension(fontSizeFloat_->Get(), DimensionUnit::PX));
    }
}

void TextContentModifier::ModifyAdaptMinFontSizeInTextStyle(TextStyle& textStyle)
{
    if (adaptMinFontSize_.has_value() && adaptMinFontSizeFloat_) {
        textStyle.SetAdaptMinFontSize(Dimension(adaptMinFontSizeFloat_->Get(), DimensionUnit::PX));
    }
}

void TextContentModifier::ModifyAdaptMaxFontSizeInTextStyle(TextStyle& textStyle)
{
    if (adaptMaxFontSize_.has_value() && adaptMaxFontSizeFloat_) {
        textStyle.SetAdaptMaxFontSize(Dimension(adaptMaxFontSizeFloat_->Get(), DimensionUnit::PX));
    }
}

void TextContentModifier::ModifyFontWeightInTextStyle(TextStyle& textStyle)
{
    if (fontWeight_.has_value() && fontWeightFloat_) {
        textStyle.SetFontWeight(static_cast<FontWeight>(std::floor(fontWeightFloat_->Get() + ROUND_VALUE)));
    }
}

void TextContentModifier::ModifyTextColorInTextStyle(TextStyle& textStyle)
{
    if (textColor_.has_value() && animatableTextColor_) {
        textStyle.SetTextColor(Color(animatableTextColor_->Get().GetValue()));
    }
}

void TextContentModifier::ModifyTextShadowsInTextStyle(TextStyle& textStyle)
{
    std::vector<Shadow> shadows;
    shadows.reserve(shadows_.size());
    for (auto&& shadow : shadows_) {
        auto blurRadius = shadow.blurRadius->Get();
        auto offsetX = shadow.offsetX->Get();
        auto offsetY = shadow.offsetY->Get();
        auto color = shadow.color->Get();
        shadows.emplace_back(blurRadius, 0, Offset(offsetX, offsetY), Color(color.GetValue()));
    }
    textStyle.SetTextShadows(shadows);
}

void TextContentModifier::ModifyDecorationInTextStyle(TextStyle& textStyle)
{
    if (textDecoration_.has_value() && textDecorationColor_.has_value() && textDecorationColorAlpha_) {
        if (textDecorationAnimatable_) {
            uint8_t alpha = static_cast<int>(std::floor(textDecorationColorAlpha_->Get() + ROUND_VALUE));
            if (alpha == 0) {
                textStyle.SetTextDecoration(TextDecoration::NONE);
                textStyle.SetTextDecorationColor(textDecorationColor_.value());
            } else {
                textStyle.SetTextDecoration(TextDecoration::UNDERLINE);
                textStyle.SetTextDecorationColor(Color(textDecorationColor_.value()).ChangeAlpha(alpha));
            }
        } else {
            textStyle.SetTextDecoration(textDecoration_.value());
            textStyle.SetTextDecorationColor(textDecorationColor_.value());
        }
    }
    if (textDecorationStyle_.has_value()) {
        textStyle.SetTextDecorationStyle(textDecorationStyle_.value());
    }
}

void TextContentModifier::ModifyBaselineOffsetInTextStyle(TextStyle& textStyle)
{
    if (baselineOffset_.has_value() && baselineOffsetFloat_) {
        textStyle.SetBaselineOffset(Dimension(baselineOffsetFloat_->Get(), DimensionUnit::PX));
    }
}

void TextContentModifier::ModifyTextStyle(TextStyle& textStyle)
{
    ModifyFontSizeInTextStyle(textStyle);
    ModifyAdaptMinFontSizeInTextStyle(textStyle);
    ModifyAdaptMaxFontSizeInTextStyle(textStyle);
    ModifyFontWeightInTextStyle(textStyle);
    ModifyTextColorInTextStyle(textStyle);
    ModifyTextShadowsInTextStyle(textStyle);
    ModifyDecorationInTextStyle(textStyle);
    ModifyBaselineOffsetInTextStyle(textStyle);
}

void TextContentModifier::UpdateFontSizeMeasureFlag(PropertyChangeFlag& flag)
{
    if (fontSize_.has_value() && fontSizeFloat_ && !NearEqual(fontSize_.value().Value(), fontSizeFloat_->Get())) {
        flag |= PROPERTY_UPDATE_MEASURE;
    }
}

void TextContentModifier::UpdateAdaptMinFontSizeMeasureFlag(PropertyChangeFlag& flag)
{
    if (adaptMinFontSize_.has_value() && adaptMinFontSizeFloat_ &&
        !NearEqual(adaptMinFontSize_.value().Value(), adaptMinFontSizeFloat_->Get())) {
        flag |= PROPERTY_UPDATE_MEASURE;
    }
}

void TextContentModifier::UpdateAdaptMaxFontSizeMeasureFlag(PropertyChangeFlag& flag)
{
    if (adaptMaxFontSize_.has_value() && adaptMaxFontSizeFloat_ &&
        !NearEqual(adaptMaxFontSize_.value().Value(), adaptMaxFontSizeFloat_->Get())) {
        flag |= PROPERTY_UPDATE_MEASURE;
    }
}

void TextContentModifier::UpdateFontWeightMeasureFlag(PropertyChangeFlag& flag)
{
    if (fontWeight_.has_value() && fontWeightFloat_ &&
        !NearEqual(static_cast<float>(static_cast<int>(fontWeight_.value())), fontWeightFloat_->Get())) {
        flag |= PROPERTY_UPDATE_MEASURE;
    }
}

void TextContentModifier::UpdateTextColorMeasureFlag(PropertyChangeFlag& flag)
{
    if (textColor_.has_value() && animatableTextColor_ &&
        textColor_->GetValue() != animatableTextColor_->Get().GetValue()) {
        flag |= PROPERTY_UPDATE_MEASURE_SELF;
    }
}

void TextContentModifier::UpdateTextShadowMeasureFlag(PropertyChangeFlag& flag)
{
    for (auto&& shadow : shadows_) {
        auto blurRadius = shadow.blurRadius->Get();
        auto offsetX = shadow.offsetX->Get();
        auto offsetY = shadow.offsetY->Get();
        auto color = shadow.color->Get();
        auto compareShadow = Shadow(blurRadius, 0, Offset(offsetX, offsetY), Color(color.GetValue()));
        compareShadow.SetShadowType(shadow.shadow.GetShadowType());
        if (shadow.shadow != compareShadow) {
            flag |= PROPERTY_UPDATE_MEASURE;
            return;
        }
    }
}

void TextContentModifier::UpdateTextDecorationMeasureFlag(PropertyChangeFlag& flag)
{
    if (textDecoration_.has_value() && textDecorationColor_.has_value() && textDecorationColorAlpha_) {
        uint8_t alpha = static_cast<int>(std::floor(textDecorationColorAlpha_->Get() + ROUND_VALUE));
        if (textDecoration_.value() == TextDecoration::UNDERLINE && alpha != textDecorationColor_.value().GetAlpha()) {
            flag |= PROPERTY_UPDATE_MEASURE;
        } else if (textDecoration_.value() == TextDecoration::NONE && alpha != 0.0) {
            flag |= PROPERTY_UPDATE_MEASURE;
        }
    }
}

void TextContentModifier::UpdateBaselineOffsetMeasureFlag(PropertyChangeFlag& flag)
{
    if (baselineOffset_.has_value() && baselineOffsetFloat_ &&
        !NearEqual(baselineOffset_.value().Value(), baselineOffsetFloat_->Get())) {
        flag |= PROPERTY_UPDATE_MEASURE;
    }
}

bool TextContentModifier::NeedMeasureUpdate(PropertyChangeFlag& flag)
{
    flag = 0;
    UpdateFontSizeMeasureFlag(flag);
    UpdateAdaptMinFontSizeMeasureFlag(flag);
    UpdateAdaptMaxFontSizeMeasureFlag(flag);
    UpdateFontWeightMeasureFlag(flag);
    UpdateTextColorMeasureFlag(flag);
    UpdateTextShadowMeasureFlag(flag);
    UpdateTextDecorationMeasureFlag(flag);
    UpdateBaselineOffsetMeasureFlag(flag);
    flag &= (PROPERTY_UPDATE_MEASURE | PROPERTY_UPDATE_MEASURE_SELF | PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);
    return flag;
}

void TextContentModifier::SetFontFamilies(const std::vector<std::string>& value)
{
    CHECK_NULL_VOID(fontFamilyString_);
    fontFamilyString_->Set(V2::ConvertFontFamily(value));
}

void TextContentModifier::SetFontSize(const Dimension& value, TextStyle& textStyle)
{
    float fontSizeValue;
    auto pipelineContext = PipelineContext::GetCurrentContext();
    if (pipelineContext) {
        float fontScaleValue = std::clamp(pipelineContext->GetFontScale(), textStyle.GetMinFontScale(),
            textStyle.GetMaxFontScale());
        if (value.Unit() == DimensionUnit::FP) {
            fontSizeValue = value.ConvertToPx() / pipelineContext->GetFontScale() * fontScaleValue;
        } else {
            fontSizeValue = value.ConvertToPx();
        }
    } else {
        fontSizeValue = value.Value();
    }
    fontSize_ = Dimension(fontSizeValue);
    CHECK_NULL_VOID(fontSizeFloat_);
    fontSizeFloat_->Set(fontSizeValue);
}

void TextContentModifier::SetAdaptMinFontSize(const Dimension& value, TextStyle& textStyle)
{
    float fontSizeValue;
    auto pipelineContext = PipelineContext::GetCurrentContext();
    if (pipelineContext) {
        float fontScaleValue = std::clamp(pipelineContext->GetFontScale(), textStyle.GetMinFontScale(),
            textStyle.GetMaxFontScale());
        if (value.Unit() == DimensionUnit::FP) {
            fontSizeValue = value.ConvertToPx() / pipelineContext->GetFontScale() * fontScaleValue;
        } else {
            fontSizeValue = value.ConvertToPx();
        }
    } else {
        fontSizeValue = value.Value();
    }
    adaptMinFontSize_ = Dimension(fontSizeValue);
    CHECK_NULL_VOID(adaptMinFontSizeFloat_);
    adaptMinFontSizeFloat_->Set(fontSizeValue);
}

void TextContentModifier::SetAdaptMaxFontSize(const Dimension& value, TextStyle& textStyle)
{
    float fontSizeValue;
    auto pipelineContext = PipelineContext::GetCurrentContext();
    if (pipelineContext) {
        float fontScaleValue = std::clamp(pipelineContext->GetFontScale(), textStyle.GetMinFontScale(),
            textStyle.GetMaxFontScale());
        if (value.Unit() == DimensionUnit::FP) {
            fontSizeValue = value.ConvertToPx() / pipelineContext->GetFontScale() * fontScaleValue;
        } else {
            fontSizeValue = value.ConvertToPx();
        }
    } else {
        fontSizeValue = value.Value();
    }
    adaptMaxFontSize_ = Dimension(fontSizeValue);
    CHECK_NULL_VOID(adaptMaxFontSizeFloat_);
    adaptMaxFontSizeFloat_->Set(fontSizeValue);
}

void TextContentModifier::SetFontWeight(const FontWeight& value)
{
    fontWeight_ = ConvertFontWeight(value);
    CHECK_NULL_VOID(fontWeightFloat_);
    fontWeightFloat_->Set(static_cast<int>(ConvertFontWeight(value)));
}

void TextContentModifier::SetTextColor(const Color& value)
{
    textColor_ = value;
    CHECK_NULL_VOID(animatableTextColor_);
    animatableTextColor_->Set(LinearColor(value));
}

void TextContentModifier::SetTextShadow(const std::vector<Shadow>& value)
{
    while (value.size() > shadows_.size()) {
        AddDefaultShadow();
    }
    // else
    while (value.size() < shadows_.size()) {
        shadows_.pop_back();
    }

    for (size_t i = 0; i < shadows_.size(); ++i) {
        auto&& newShadow = value[i];
        Shadow textShadow;
        textShadow.SetBlurRadius(newShadow.GetBlurRadius());
        textShadow.SetOffset(newShadow.GetOffset());
        textShadow.SetColor(newShadow.GetColor());
        shadows_[i].shadow = textShadow;
        shadows_[i].blurRadius->Set(newShadow.GetBlurRadius());
        shadows_[i].offsetX->Set(newShadow.GetOffset().GetX());
        shadows_[i].offsetY->Set(newShadow.GetOffset().GetY());
        shadows_[i].color->Set(LinearColor(newShadow.GetColor()));
    }
}

void TextContentModifier::SetTextDecoration(const TextDecoration& type)
{
    auto oldTextDecoration = textDecoration_.value_or(TextDecoration::NONE);
    if (oldTextDecoration == type) {
        return;
    }

    textDecorationAnimatable_ = (oldTextDecoration == TextDecoration::NONE && type == TextDecoration::UNDERLINE) ||
                                (oldTextDecoration == TextDecoration::UNDERLINE && type == TextDecoration::NONE);

    textDecoration_ = type;
    CHECK_NULL_VOID(textDecorationColorAlpha_);

    oldColorAlpha_ = textDecorationColorAlpha_->Get();
    if (textDecoration_ == TextDecoration::NONE) {
        textDecorationColorAlpha_->Set(0.0f);
    } else {
        textDecorationColorAlpha_->Set(static_cast<float>(textDecorationColor_.value().GetAlpha()));
    }
}

void TextContentModifier::SetTextDecorationStyle(const TextDecorationStyle textDecorationStyle)
{
    textDecorationStyle_ = textDecorationStyle;
}

void TextContentModifier::SetTextDecorationColor(const Color& color)
{
    textDecorationColor_ = color;
}

void TextContentModifier::SetBaselineOffset(const Dimension& value)
{
    float baselineOffsetValue;
    auto pipelineContext = PipelineContext::GetCurrentContext();
    if (pipelineContext) {
        baselineOffsetValue = pipelineContext->NormalizeToPx(value);
    } else {
        baselineOffsetValue = value.Value();
    }
    baselineOffset_ = Dimension(baselineOffsetValue);
    CHECK_NULL_VOID(baselineOffsetFloat_);
    baselineOffsetFloat_->Set(baselineOffsetValue);
}

void TextContentModifier::SetContentOffset(OffsetF& value)
{
    CHECK_NULL_VOID(contentOffset_);
    contentOffset_->Set(value);
}

void TextContentModifier::SetContentSize(SizeF& value)
{
    CHECK_NULL_VOID(contentSize_);
    contentSize_->Set(value);
}

void TextContentModifier::SetIsFocused(const bool& isFocused)
{
    marqueeFocused_ = isFocused;

    DetermineTextRace();
}

void TextContentModifier::SetIsHovered(const bool& isHovered)
{
    marqueeHovered_ = isHovered;

    DetermineTextRace();
}

bool TextContentModifier::SetTextRace(const MarqueeOption& option)
{
    auto textPattern = DynamicCast<TextPattern>(pattern_.Upgrade());
    CHECK_NULL_RETURN(textPattern, false);
    auto pManager = textPattern->GetParagraphManager();
    CHECK_NULL_RETURN(pManager, false);
    textRaceSpaceWidth_ = RACE_SPACE_WIDTH;
    auto pipeline = PipelineContext::GetCurrentContext();
    if (pipeline) {
        textRaceSpaceWidth_ *= pipeline->GetDipScale();
    }

    auto duration =
        static_cast<int32_t>(std::abs(pManager->GetTextWidth() + textRaceSpaceWidth_) * RACE_DURATION_RATIO);
    if (option.step > 0) {
        duration = static_cast<int32_t>(duration / option.step);
    }
    if (duration <= 0) {
        return false;
    }

    if (textRacing_ && marqueeOption_ == option && duration == marqueeDuration_) {
        return false;
    }

    marqueeDuration_ = duration;
    marqueeOption_ = option;
    marqueeGradientPercent_ = 0;

    auto contentWidth = contentSize_->Get().Width();
    if (contentWidth > 0) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_RETURN(pipeline, true);
        auto theme = pipeline->GetTheme<TextTheme>();
        CHECK_NULL_RETURN(theme, true);
        auto fadeoutWidth = theme->GetFadeoutWidth();
        marqueeGradientPercent_ = fadeoutWidth.ConvertToPx() / contentWidth;
    }

    return true;
}

void TextContentModifier::StartTextRace(const MarqueeOption& option)
{
    if (!SetTextRace(option)) {
        return;
    }

    marqueeSet_ = true;
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        UpdateImageNodeVisible(VisibleType::INVISIBLE);
    }
    if (textRacing_) {
        PauseTextRace();
    }
    ResumeTextRace(false);
}

void TextContentModifier::StopTextRace()
{
    marqueeSet_ = false;
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        UpdateImageNodeVisible(VisibleType::VISIBLE);
    }
    PauseTextRace();
}

void TextContentModifier::ResumeAnimation()
{
    CHECK_NULL_VOID(raceAnimation_);
    if (textRacing_) {
        return;
    }
    AnimationUtils::ResumeAnimation(raceAnimation_);
    textRacing_ = true;
}

void TextContentModifier::PauseAnimation()
{
    CHECK_NULL_VOID(raceAnimation_);
    if (!textRacing_) {
        return;
    }
    AnimationUtils::PauseAnimation(raceAnimation_);
    textRacing_ = false;
}

void TextContentModifier::ResumeTextRace(bool bounce)
{
    if (!AllowTextRace()) {
        return;
    }

    if (!bounce) {
        auto textPattern = DynamicCast<TextPattern>(pattern_.Upgrade());
        CHECK_NULL_VOID(textPattern);
        textPattern->FireOnMarqueeStateChange(TextMarqueeState::START);
    }

    AnimationOption option = AnimationOption();
    RefPtr<Curve> curve = MakeRefPtr<LinearCurve>();
    option.SetDuration(marqueeDuration_);
    option.SetDelay(bounce ? marqueeOption_.delay : 0);
    option.SetCurve(curve);
    option.SetIteration(1);
    SetTextRaceAnimation(option);
}

void TextContentModifier::SetTextRaceAnimation(const AnimationOption& option)
{
    marqueeAnimationId_++;
    racePercentFloat_->Set(RACE_MOVE_PERCENT_MIN);
    textRacing_ = true;
    raceAnimation_ = AnimationUtils::StartAnimation(
        option, [&]() { racePercentFloat_->Set(RACE_MOVE_PERCENT_MAX); },
        [weak = AceType::WeakClaim(this), marqueeAnimationId = marqueeAnimationId_, id = Container::CurrentId()]() {
            auto modifier = weak.Upgrade();
            CHECK_NULL_VOID(modifier);

            ContainerScope scope(id);
            auto taskExecutor = Container::CurrentTaskExecutor();
            CHECK_NULL_VOID(taskExecutor);

            auto onFinish = [weak, marqueeAnimationId]() {
                auto modifier = weak.Upgrade();
                CHECK_NULL_VOID(modifier);

                if (marqueeAnimationId != modifier->marqueeAnimationId_) {
                    return;
                }
                auto textPattern = DynamicCast<TextPattern>(modifier->pattern_.Upgrade());
                CHECK_NULL_VOID(textPattern);

                if (NearEqual(modifier->GetTextRacePercent(), RACE_MOVE_PERCENT_MAX)) {
                    textPattern->FireOnMarqueeStateChange(TextMarqueeState::BOUNCE);
                    modifier->marqueeCount_++;
                }
                if (!modifier->AllowTextRace()) {
                    textPattern->FireOnMarqueeStateChange(TextMarqueeState::FINISH);
                } else {
                    auto frameNode = textPattern->GetHost();
                    CHECK_NULL_VOID(frameNode);
                    frameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
                    modifier->ResumeTextRace(true);
                }
            };

            if (taskExecutor->WillRunOnCurrentThread(TaskExecutor::TaskType::UI)) {
                onFinish();
            } else {
                taskExecutor->PostTask(
                    [onFinish]() { onFinish(); }, TaskExecutor::TaskType::UI, "ArkUITextStartTextRace");
            }
        });
}

void TextContentModifier::PauseTextRace()
{
    if (!textRacing_) {
        return;
    }
    if (raceAnimation_) {
        AnimationUtils::StopAnimation(raceAnimation_);
    }

    textRacing_ = false;
    racePercentFloat_->Set(RACE_MOVE_PERCENT_MIN);
}

bool TextContentModifier::AllowTextRace()
{
    if (!marqueeSet_ || !marqueeOption_.start) {
        return false;
    }

    if (marqueeOption_.loop > 0 && marqueeCount_ >= marqueeOption_.loop) {
        return false;
    }

    if (marqueeOption_.startPolicy == MarqueeStartPolicy::ON_FOCUS && !(marqueeFocused_ || marqueeHovered_)) {
        return false;
    }

    return true;
}

void TextContentModifier::DetermineTextRace()
{
    if (!marqueeSet_ || !marqueeOption_.start || marqueeOption_.startPolicy != MarqueeStartPolicy::ON_FOCUS) {
        return;
    }

    if (textRacing_ && !marqueeFocused_ && !marqueeHovered_) {
        PauseTextRace();
        return;
    }

    if (!textRacing_ && (marqueeFocused_ || marqueeHovered_)) {
        ResumeTextRace(false);
    }
}

float TextContentModifier::GetTextRacePercent()
{
    float percent = 0;
    if (racePercentFloat_) {
        percent = racePercentFloat_->Get();
    }
    return percent;
}

void TextContentModifier::ContentChange()
{
    CHECK_NULL_VOID(contentChange_);
    contentChange_->Set(contentChange_->Get() + 1);
}

void TextContentModifier::AddDefaultShadow()
{
    Shadow emptyShadow;
    auto blurRadius = MakeRefPtr<AnimatablePropertyFloat>(emptyShadow.GetBlurRadius());
    auto offsetX = MakeRefPtr<AnimatablePropertyFloat>(emptyShadow.GetOffset().GetX());
    auto offsetY = MakeRefPtr<AnimatablePropertyFloat>(emptyShadow.GetOffset().GetY());
    auto color = MakeRefPtr<AnimatablePropertyColor>(LinearColor(emptyShadow.GetColor()));
    shadows_.emplace_back(
        ShadowProp { .blurRadius = blurRadius, .offsetX = offsetX, .offsetY = offsetY, .color = color });
    AttachProperty(blurRadius);
    AttachProperty(offsetX);
    AttachProperty(offsetY);
    AttachProperty(color);
}
} // namespace OHOS::Ace::NG
