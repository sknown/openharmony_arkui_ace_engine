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

#include "core/components_ng/pattern/text_field/text_field_layout_algorithm.h"

#include "unicode/uchar.h"

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/i18n/localization.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/common/font_manager.h"
#include "core/components/common/layout/constants.h"
#include "core/components/scroll/scroll_bar_theme.h"
#include "core/components/text/text_theme.h"
#include "core/components/theme/theme_manager.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_content_modifier.h"
#include "core/components_ng/pattern/text_field/text_field_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/pattern/text_field/text_selector.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t COUNTER_TEXT_MAXLINE = 1;
constexpr float PARAGRAPH_SAVE_BOUNDARY = 1.0f;
} // namespace

void TextFieldLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    const auto& layoutConstraint = layoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
    OptionalSizeF frameSize =
        CreateIdealSize(layoutConstraint.value(), Axis::HORIZONTAL, MeasureType::MATCH_PARENT_MAIN_AXIS);
    const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
    const auto& calcLayoutConstraint = layoutWrapper->GetLayoutProperty()->GetCalcLayoutConstraint();
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    float contentWidth = 0.0f;
    float contentHeight = 0.0f;
    if (content) {
        auto contentSize = content->GetRect().GetSize();
        contentWidth = contentSize.Width();
        contentHeight = contentSize.Height();
    }
    if (pattern->IsTextArea()) {
        if (!frameSize.Width().has_value()) {
            // If width is not set, select the maximum value of minWidth and maxWidth to layoutConstraint
            if (calcLayoutConstraint && calcLayoutConstraint->maxSize.has_value() &&
                calcLayoutConstraint->maxSize.value().Width().has_value()) {
                frameSize.SetHeight(std::min(layoutConstraint->maxSize.Width(),
                    contentWidth + pattern->GetHorizontalPaddingSum()));
            } else if (!calcLayoutConstraint) {
                // If calcLayoutConstraint has not set, use the LayoutConstraint initial value
                frameSize.SetWidth(contentWidth + pattern->GetHorizontalPaddingSum());
            } else {
                // If maxWidth is not set and calcLayoutConstraint is set, set minWidth to layoutConstraint
                frameSize.SetWidth(layoutConstraint->minSize.Width());
            }
        }
        if (pattern->IsNormalInlineState()) {
            frameSize.SetWidth(contentWidth + pattern->GetHorizontalPaddingSum() + PARAGRAPH_SAVE_BOUNDARY);
        }
        if (!frameSize.Height().has_value()) {
            // Like width
            if (calcLayoutConstraint && calcLayoutConstraint->maxSize.has_value() &&
                calcLayoutConstraint->maxSize.value().Height().has_value()) {
                frameSize.SetHeight(std::min(layoutConstraint->maxSize.Height(),
                    contentHeight + pattern->GetVerticalPaddingSum()));
            } else if (!calcLayoutConstraint || NearZero(layoutConstraint->minSize.Height())) {
                // calcLayoutConstraint initialized once when setting width, set minHeight=0,
                // so add "minHeight=0" to the constraint.
                frameSize.SetHeight(
                    std::min(layoutConstraint->maxSize.Height(), contentHeight + pattern->GetVerticalPaddingSum()));
            } else {
                frameSize.SetHeight(
                    std::max(layoutConstraint->minSize.Height(), contentHeight + pattern->GetVerticalPaddingSum()));
            }
        }

        // Here's what happens when the height or width is set at list one
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            frameSize.Constrain(layoutConstraint->minSize, layoutConstraint->maxSize);
        } else {
            auto finalSize = UpdateOptionSizeByCalcLayoutConstraint(frameSize,
                layoutWrapper->GetLayoutProperty()->GetCalcLayoutConstraint(),
                layoutWrapper->GetLayoutProperty()->GetLayoutConstraint()->percentReference);
            frameSize.SetWidth(finalSize.Width());
            frameSize.SetHeight(finalSize.Height());
        }
        if (layoutConstraint->maxSize.Height() < layoutConstraint->minSize.Height()) {
            frameSize.SetHeight(layoutConstraint->minSize.Height());
        }
        if (layoutConstraint->maxSize.Width() < layoutConstraint->minSize.Width()) {
            frameSize.SetWidth(layoutConstraint->minSize.Width());
        }
        layoutWrapper->GetGeometryNode()->SetFrameSize(frameSize.ConvertToSizeT());

        frameRect_ =
            RectF(layoutWrapper->GetGeometryNode()->GetFrameOffset(), layoutWrapper->GetGeometryNode()->GetFrameSize());
        return;
    }
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto textFieldTheme = pipeline->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(textFieldTheme);
    auto defaultHeight = textFieldTheme->GetHeight().ConvertToPx();
    if (!frameSize.Height().has_value()) {
        if (calcLayoutConstraint && calcLayoutConstraint->maxSize.has_value() &&
            calcLayoutConstraint->maxSize.value().Height().has_value()) {
            frameSize.SetHeight(std::max(layoutConstraint->maxSize.Height(), layoutConstraint->minSize.Height()));
        } else if (!calcLayoutConstraint || NearZero(layoutConstraint->minSize.Height())) {
            auto height = contentHeight + pattern->GetVerticalPaddingSum() < defaultHeight
                              ? defaultHeight
                              : contentHeight + pattern->GetVerticalPaddingSum();
            frameSize.SetHeight(
                std::min(layoutConstraint->maxSize.Height(), static_cast<float>(height)));
        } else {
            frameSize.SetHeight(layoutConstraint->minSize.Height());
        }
    }
    auto textfieldLayoutProperty = AceType::DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(textfieldLayoutProperty);

    if (textfieldLayoutProperty->GetWidthAutoValue(false)) {
        auto width =
            std::max(std::min(layoutConstraint->maxSize.Width(), contentWidth + pattern->GetHorizontalPaddingSum()),
                layoutConstraint->minSize.Width());
        frameSize.SetWidth(width);
    }
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
        frameSize.Constrain(layoutConstraint->minSize, layoutConstraint->maxSize);
    } else {
        auto finalSize = UpdateOptionSizeByCalcLayoutConstraint(frameSize,
            layoutWrapper->GetLayoutProperty()->GetCalcLayoutConstraint(),
            layoutWrapper->GetLayoutProperty()->GetLayoutConstraint()->percentReference);
        frameSize.SetWidth(finalSize.Width());
        frameSize.SetHeight(finalSize.Height());
    }
    if (layoutConstraint->maxSize.Height() < layoutConstraint->minSize.Height()) {
        frameSize.SetHeight(layoutConstraint->minSize.Height());
    }
    if (layoutConstraint->maxSize.Width() < layoutConstraint->minSize.Width()) {
        frameSize.SetWidth(layoutConstraint->minSize.Width());
    }
    layoutWrapper->GetGeometryNode()->SetFrameSize(frameSize.ConvertToSizeT());
    frameRect_ =
        RectF(layoutWrapper->GetGeometryNode()->GetFrameOffset(), layoutWrapper->GetGeometryNode()->GetFrameSize());

    auto children = frameNode->GetChildren();
    auto layoutProperty = DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    if (!children.empty() && layoutProperty->GetShowUnderlineValue(false) &&
        layoutProperty->GetTextInputTypeValue(TextInputType::UNSPECIFIED) == TextInputType::UNSPECIFIED) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
        auto childLayoutConstraint = textfieldLayoutProperty->CreateChildConstraint();
        CHECK_NULL_VOID(childWrapper);
        childWrapper->Measure(childLayoutConstraint);
    }
}

std::optional<SizeF> TextFieldLayoutAlgorithm::MeasureContent(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, std::nullopt);
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_RETURN(pipeline, std::nullopt);
    auto textFieldLayoutProperty = DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(textFieldLayoutProperty, std::nullopt);
    auto textFieldTheme = pipeline->GetTheme<TextFieldTheme>();
    CHECK_NULL_RETURN(textFieldTheme, std::nullopt);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_RETURN(pattern, std::nullopt);

    // Construct a textstyle.
    TextStyle textStyle;
    std::string textContent;
    bool showPlaceHolder = false;
    auto idealWidth = contentConstraint.selfIdealSize.Width().value_or(contentConstraint.maxSize.Width());
    auto idealHeight = contentConstraint.selfIdealSize.Height().value_or(contentConstraint.maxSize.Height());
    auto idealSize = SizeF { idealWidth, idealHeight };
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
        idealSize.UpdateSizeWhenSmaller(contentConstraint.maxSize);
    } else {
        auto finalSize = UpdateOptionSizeByCalcLayoutConstraint(static_cast<OptionalSize<float>>(idealSize),
            layoutWrapper->GetLayoutProperty()->GetCalcLayoutConstraint(),
            layoutWrapper->GetLayoutProperty()->GetLayoutConstraint()->percentReference);
        idealSize.UpdateSizeWhenSmaller(finalSize.ConvertToSizeT());
    }
    idealWidth = idealSize.Width();
    idealHeight = idealSize.Height();
    auto isInlineStyle = pattern->IsNormalInlineState();
    if (!textFieldLayoutProperty->GetValueValue("").empty()) {
        UpdateTextStyle(frameNode, textFieldLayoutProperty, textFieldTheme, textStyle, pattern->IsDisabled());
        textContent = textFieldLayoutProperty->GetValueValue("");
        if (!pattern->IsTextArea() && isInlineStyle) {
            textStyle.SetTextOverflow(TextOverflow::ELLIPSIS);
        }
    } else {
        UpdatePlaceholderTextStyle(frameNode, textFieldLayoutProperty, textFieldTheme,
            textStyle, pattern->IsDisabled());
        textContent = textFieldLayoutProperty->GetPlaceholderValue("");
        showPlaceHolder = true;
    }

    // use for modifier.
    auto contentModifier = pattern->GetContentModifier();
    if (contentModifier) {
        SetPropertyToModifier(textStyle, contentModifier);
        contentModifier->ModifyTextStyle(textStyle);
        contentModifier->SetFontReady(false);
    }
    auto isPasswordType =
        textFieldLayoutProperty->GetTextInputTypeValue(TextInputType::UNSPECIFIED) == TextInputType::VISIBLE_PASSWORD;
    auto disableTextAlign = !pattern->IsTextArea() && !showPlaceHolder && !isInlineStyle;
    // Create paragraph.
    if (pattern->IsDragging() && !showPlaceHolder) {
        TextStyle dragTextStyle = textStyle;
        Color color = textStyle.GetTextColor().ChangeAlpha(DRAGGED_TEXT_OPACITY);
        dragTextStyle.SetTextColor(color);
        std::vector<TextStyle> textStyles { textStyle, dragTextStyle, textStyle };
        CreateParagraph(textStyles, pattern->GetDragContents(), textContent,
            isPasswordType && pattern->GetTextObscured() && !showPlaceHolder, disableTextAlign);
    } else {
        CreateParagraph(textStyle, textContent, isPasswordType && pattern->GetTextObscured() && !showPlaceHolder,
            pattern->GetNakedCharPosition(), disableTextAlign);
    }

    // paragraph  Layout.
    float imageSize = 0.0f;
    auto showPasswordIcon = textFieldLayoutProperty->GetShowPasswordIcon().value_or(true);
    imageSize = showPasswordIcon ? pattern->GetIconSize() : 0.0f;
    auto imageHotZoneWidth = showPasswordIcon ? imageSize + pattern->GetIconRightOffset() : 0.0f;
    auto scrollBarTheme = pipeline->GetTheme<ScrollBarTheme>();
    CHECK_NULL_RETURN(scrollBarTheme, std::nullopt);
    const auto& layoutConstraint = textFieldLayoutProperty->GetLayoutConstraint();
    if (isInlineStyle) {
        // for InlineStyle, max width is content width with safe boundary.
        float inlineBoxWidth = 0.0f;
        if (pattern->IsFocus()) {
            auto safeBoundary = textFieldTheme->GetInlineBorderWidth().ConvertToPx() * 2;
            paragraph_->Layout(pattern->GetPreviewWidth() == 0 ? idealWidth : pattern->GetPreviewWidth());
            if (pattern->GetPreviewWidth() != 0 && pattern->GetPreviewWidth() > layoutConstraint->maxSize.Width()) {
                paragraph_->Layout(layoutConstraint->maxSize.Width() - safeBoundary - PARAGRAPH_SAVE_BOUNDARY);
            }
#ifndef USE_GRAPHIC_TEXT_GINE
            inlineBoxWidth = paragraph_->GetLongestLine() + PARAGRAPH_SAVE_BOUNDARY;
#else
            inlineBoxWidth = paragraph_->GetActualWidth() + PARAGRAPH_SAVE_BOUNDARY;
#endif
            if (inlineBoxWidth > layoutConstraint->maxSize.Width()) {
                inlineBoxWidth = layoutConstraint->maxSize.Width() - safeBoundary;
            }
            paragraph_->Layout(pattern->GetPreviewWidth() == 0 ? idealWidth : inlineBoxWidth);
        } else {
            inlineBoxWidth = idealWidth;
            paragraph_->Layout(inlineBoxWidth);
        }
        if (pattern->IsTextArea()) {
            paragraph_->Layout(pattern->GetPreviewWidth() == 0 ? idealWidth : inlineBoxWidth);
        }
    } else if (showPlaceHolder) {
        // for placeholder.
        if (isPasswordType) {
            paragraph_->Layout(idealWidth - imageHotZoneWidth);
        } else {
            paragraph_->Layout(idealWidth);
        }
    } else if (textStyle.GetMaxLines() == 1) {
        // for text input case, need to measure in one line without constraint.
        paragraph_->Layout(std::numeric_limits<double>::infinity());
    } else {
        // for text area, max width is content width without scroll bar.
        paragraph_->Layout(
            idealWidth - scrollBarTheme->GetActiveWidth().ConvertToPx() - SCROLL_BAR_LEFT_WIDTH.ConvertToPx());
    }

    // counterParagraph Layout.
    if (textFieldLayoutProperty->GetShowCounterValue(false) && textFieldLayoutProperty->HasMaxLength() &&
        !isInlineStyle) {
        auto textLength = showPlaceHolder ? 0 : StringUtils::ToWstring(textContent).length();
        auto maxLength = textFieldLayoutProperty->GetMaxLength().value();
        CreateCounterParagraph(textLength, maxLength, textFieldTheme);
        if (counterParagraph_) {
            counterParagraph_->Layout(
                idealWidth - scrollBarTheme->GetActiveWidth().ConvertToPx() - SCROLL_BAR_LEFT_WIDTH.ConvertToPx());
        }
    }
    // errorParagraph  Layout.
    if (textFieldLayoutProperty->GetShowErrorTextValue(false)) {
        CreateErrorParagraph(textFieldLayoutProperty->GetErrorTextValue(""), textFieldTheme);
        if (errorParagraph_) {
            errorParagraph_->Layout(std::numeric_limits<double>::infinity());
        }
    }
    auto paragraphNewWidth = static_cast<float>(paragraph_->GetMaxIntrinsicWidth());
    if (!NearEqual(paragraphNewWidth, paragraph_->GetMaxWidth()) && !pattern->IsTextArea() && !showPlaceHolder &&
        !isInlineStyle) {
        paragraph_->Layout(std::ceil(paragraphNewWidth));
        if (counterParagraph_) {
            counterParagraph_->Layout(std::ceil(paragraphNewWidth));
        }
    }
    auto preferredHeight = static_cast<float>(paragraph_->GetHeight());
    if (textContent.empty() || showPlaceHolder) {
        preferredHeight = pattern->PreferredLineHeight();
    }
    if (isInlineStyle && showPlaceHolder && !textContent.empty()) {
        preferredHeight = static_cast<float>(paragraph_->GetHeight());
    }
#ifndef USE_GRAPHIC_TEXT_GINE // support sigleline
    if (isInlineStyle && pattern->IsFocus()) {
        pattern->SetSingleLineHeight(preferredHeight);
    }
#else // support multi-line
    if (isInlineStyle && pattern->IsFocus() && paragraph_->GetLineCount() != 0) {
        pattern->SetSingleLineHeight(preferredHeight / paragraph_->GetLineCount());
    }
#endif
#ifndef USE_GRAPHIC_TEXT_GINE
    paragraphWidth_ = paragraph_->GetLongestLine();
#else
    paragraphWidth_ = paragraph_->GetActualWidth();
#endif
    // textarea size.
    if (pattern->IsTextArea()) {
        auto paragraphHeight =
            (textContent.empty() || !showPlaceHolder) ? preferredHeight : static_cast<float>(paragraph_->GetHeight());
        auto useHeight =
            static_cast<float>(paragraphHeight + (counterParagraph_ ? counterParagraph_->GetHeight() : 0.0f));
        if (isInlineStyle && pattern->IsFocus()) {
            idealHeight = pattern->GetSingleLineHeight() *
                          textFieldLayoutProperty->GetMaxViewLinesValue(INLINE_DEFAULT_VIEW_MAXLINE);
#ifndef USE_GRAPHIC_TEXT_GINE
            idealWidth = paragraph_->GetLongestLine();
#else
            idealWidth = paragraph_->GetActualWidth();
#endif
        }
        if (counterParagraph_ && idealHeight < useHeight) {
            pattern->SetISCounterIdealHeight(true);
            idealHeight = idealHeight - counterParagraph_->GetHeight();
        }
        textRect_.SetSize(
            SizeF(idealWidth - scrollBarTheme->GetActiveWidth().ConvertToPx() - SCROLL_BAR_LEFT_WIDTH.ConvertToPx(),
                paragraph_->GetHeight()));
        return SizeF(idealWidth, std::min(idealHeight, useHeight));
    }
    // check password image size.
    if (!showPasswordIcon || !isPasswordType) {
#ifndef USE_GRAPHIC_TEXT_GINE
        textRect_.SetSize(SizeF(static_cast<float>(paragraph_->GetLongestLine()), preferredHeight));
#else
        textRect_.SetSize(SizeF(static_cast<float>(paragraph_->GetActualWidth()), preferredHeight));
#endif
        imageRect_.Reset();
        if (textFieldLayoutProperty->GetWidthAutoValue(false)) {
            if (LessOrEqual(contentConstraint.minSize.Width(), 0.0f)) {
                idealWidth = std::clamp(textRect_.GetSize().Width(), 0.0f, contentConstraint.maxSize.Width());
            } else if (LessOrEqual(textRect_.Width(), 0.0f)) {
                idealWidth = contentConstraint.minSize.Width();
            } else {
                idealWidth =
                    std::clamp(textRect_.Width(), contentConstraint.minSize.Width(), contentConstraint.maxSize.Width());
            }
        }
        return SizeF(idealWidth, std::min(preferredHeight, idealHeight));
    }

    // password type size.
    imageRect_.SetSize(SizeF(imageSize, imageSize));
    if (pattern->GetTextObscured() && pattern->GetHidePasswordIconCtx()) {
        pattern->GetHidePasswordIconCtx()->MakeCanvasImage(imageRect_.GetSize(), true, ImageFit::NONE);
    } else if (!pattern->GetTextObscured() && pattern->GetShowPasswordIconCtx()) {
        pattern->GetShowPasswordIconCtx()->MakeCanvasImage(imageRect_.GetSize(), true, ImageFit::NONE);
    }
    preferredHeight = std::min(static_cast<float>(paragraph_->GetHeight()), idealHeight);
#ifndef USE_GRAPHIC_TEXT_GINE
    textRect_.SetSize(SizeF(static_cast<float>(paragraph_->GetLongestLine()), static_cast<float>(preferredHeight)));
#else
    textRect_.SetSize(SizeF(static_cast<float>(paragraph_->GetActualWidth()), static_cast<float>(preferredHeight)));
#endif
    return SizeF(idealWidth - imageHotZoneWidth, std::min(idealHeight, preferredHeight));
}

void TextFieldLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    // update child position.
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
    CHECK_NULL_VOID(content);
    auto contentSize = content->GetRect().GetSize();
    auto layoutProperty = DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto context = layoutWrapper->GetHostNode()->GetContext();
    CHECK_NULL_VOID(context);
    parentGlobalOffset_ = layoutWrapper->GetHostNode()->GetPaintRectOffset() - context->GetRootRect().GetOffset();
    auto align = Alignment::CENTER;
    bool hasAlign = false;
    if (layoutWrapper->GetLayoutProperty()->GetPositionProperty()) {
        align = layoutWrapper->GetLayoutProperty()->GetPositionProperty()->GetAlignment().value_or(align);
        hasAlign = layoutWrapper->GetLayoutProperty()->GetPositionProperty()->GetAlignment().has_value();
    }
    // Update content position.
    OffsetF contentOffset = Alignment::GetAlignPosition(size, contentSize, align);
    if (pattern->IsTextArea()) {
        auto isInlineStyle = pattern->IsNormalInlineState();
        if (hasAlign) {
            if (isInlineStyle) {
                content->SetOffset(OffsetF(pattern->GetUtilPadding().Offset().GetX(), contentOffset.GetY()));
            } else {
                content->SetOffset(OffsetF(pattern->GetUtilPadding().Offset().GetX() + pattern->GetBorderLeft(),
                    contentOffset.GetY() + pattern->GetBorderTop()));
            }

            OffsetF textRectOffSet = Alignment::GetAlignPosition(size, textRect_.GetSize(), align);
            if (LessOrEqual(textRect_.Height(), content->GetRect().Height())) {
                textRect_.SetOffset(OffsetF(pattern->GetTextRect().GetOffset().GetX(), textRectOffSet.GetY()));
            } else {
                textRect_.SetOffset(pattern->GetTextRect().GetOffset());
            }
        } else {
            if (isInlineStyle) {
                content->SetOffset(pattern->GetUtilPadding().Offset());
            } else {
                content->SetOffset(OffsetF(pattern->GetUtilPadding().Offset().GetX() + pattern->GetBorderLeft(),
                    pattern->GetUtilPadding().Offset().GetY() + pattern->GetBorderTop()));
            }
            textRect_.SetOffset(pattern->GetTextRect().GetOffset());
        }
        return;
    }
    content->SetOffset(OffsetF(pattern->GetPaddingLeft(), contentOffset.GetY()));
    // if handler is moving, no need to adjust text rect in pattern
    auto isUsingMouse = pattern->GetMouseStatus() == MouseStatus::MOVE ||
                        pattern->GetMouseStatus() == MouseStatus::RELEASED || pattern->GetIsMousePressed();
    auto needForceCheck =
        ((pattern->GetCaretUpdateType() == CaretUpdateType::INPUT ||
             pattern->GetCaretUpdateType() == CaretUpdateType::DEL || pattern->GetTextRectWillChange()) &&
            (paragraphWidth_ <= contentSize.Width())) ||
        pattern->GetCaretUpdateType() == CaretUpdateType::ICON_PRESSED ||
        pattern->GetCaretUpdateType() == CaretUpdateType::VISIBLE_PASSWORD_ICON ||
        layoutProperty->GetTextAlignChangedValue(false);
    auto needToKeepTextRect = isUsingMouse || !needForceCheck;
    if (needToKeepTextRect) {
        textRect_.SetOffset(pattern->GetTextRect().GetOffset());
    }
    auto paintProperty = pattern->GetPaintProperty<TextFieldPaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    if (!pattern->IsTextArea() && !needToKeepTextRect) {
        auto textOffset = Alignment::GetAlignPosition(contentSize, textRect_.GetSize(), Alignment::CENTER_LEFT);
        // adjust text rect to the basic padding
        auto textRectOffsetX = pattern->GetPaddingLeft() + pattern->GetBorderLeft();
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            textRectOffsetX = pattern->GetPaddingLeft();
        }
        auto isEmptyTextEditValue = pattern->GetTextEditingValue().text.empty();
        auto isInlineStyle = pattern->IsNormalInlineState();
        if (!isEmptyTextEditValue && !isInlineStyle) {
            switch (layoutProperty->GetTextAlignValue(TextAlign::START)) {
                case TextAlign::START:
                    break;
                case TextAlign::CENTER:
                    textRectOffsetX += (contentSize.Width() - textRect_.Width()) * 0.5f;
                    break;
                case TextAlign::END:
                    textRectOffsetX += contentSize.Width() - textRect_.Width();
                    break;
                default:
                    break;
            }
        }
        textRect_.SetOffset(OffsetF(textRectOffsetX, textOffset.GetY()));
    }

    // update image rect.
    if (!imageRect_.IsEmpty()) {
        auto imageOffset = Alignment::GetAlignPosition(size, imageRect_.GetSize(), Alignment::CENTER_RIGHT);
        imageOffset.AddX(-pattern->GetIconRightOffset());
        imageRect_.SetOffset(imageOffset);
    }

    UpdateUnitLayout(layoutWrapper);
}

void TextFieldLayoutAlgorithm::UpdateTextStyle(const RefPtr<FrameNode>& frameNode,
    const RefPtr<TextFieldLayoutProperty>& layoutProperty, const RefPtr<TextFieldTheme>& theme, TextStyle& textStyle,
    bool isDisabled)
{
    const std::vector<std::string> defaultFontFamily = { "sans-serif" };
    textStyle.SetFontFamilies(layoutProperty->GetFontFamilyValue(defaultFontFamily));
    FontRegisterCallback(frameNode, textStyle.GetFontFamilies());

    Dimension fontSize;
    if (layoutProperty->HasFontSize() && layoutProperty->GetFontSize().value_or(Dimension()).IsNonNegative()) {
        fontSize = layoutProperty->GetFontSizeValue(Dimension());
    } else {
        fontSize = theme ? theme->GetFontSize() : textStyle.GetFontSize();
    }
    textStyle.SetFontSize(fontSize);
    textStyle.SetTextAlign(layoutProperty->GetTextAlignValue(TextAlign::START));
    textStyle.SetFontWeight(
        layoutProperty->GetFontWeightValue(theme ? theme->GetFontWeight() : textStyle.GetFontWeight()));
    if (isDisabled) {
        textStyle.SetTextColor(theme ? theme->GetDisableTextColor() : textStyle.GetTextColor());
        if (layoutProperty->GetShowUnderlineValue(false) &&
            layoutProperty->GetTextInputTypeValue(TextInputType::UNSPECIFIED) == TextInputType::UNSPECIFIED) {
            textStyle.SetTextColor(theme ? theme->GetTextColorDisable() : textStyle.GetTextColor());
        }
    } else {
        auto renderContext = frameNode->GetRenderContext();
        if (renderContext->HasForegroundColor()) {
            textStyle.SetTextColor(renderContext->GetForegroundColor().value());
        } else if (renderContext->HasForegroundColorStrategy()) {
            textStyle.SetTextColor(Color::BLACK);
        } else {
            textStyle.SetTextColor(
                layoutProperty->GetTextColorValue(theme ? theme->GetTextColor() : textStyle.GetTextColor()));
        }
    }
    if (layoutProperty->GetMaxLines()) {
        textStyle.SetMaxLines(layoutProperty->GetMaxLines().value());
    }
    if (layoutProperty->HasItalicFontStyle()) {
        textStyle.SetFontStyle(layoutProperty->GetItalicFontStyle().value());
    }
    if (layoutProperty->HasTextAlign()) {
        textStyle.SetTextAlign(layoutProperty->GetTextAlign().value());
    }
}

void TextFieldLayoutAlgorithm::UpdatePlaceholderTextStyle(const RefPtr<FrameNode>& frameNode,
    const RefPtr<TextFieldLayoutProperty>& layoutProperty, const RefPtr<TextFieldTheme>& theme, TextStyle& textStyle,
    bool isDisabled)
{
    const std::vector<std::string> defaultFontFamily = { "sans-serif" };
    textStyle.SetFontFamilies(layoutProperty->GetPlaceholderFontFamilyValue(defaultFontFamily));
    FontRegisterCallback(frameNode, textStyle.GetFontFamilies());

    Dimension fontSize;
    if (layoutProperty->GetPlaceholderValue("").empty()) {
        if (layoutProperty->HasFontSize() && layoutProperty->GetFontSize().value_or(Dimension()).IsNonNegative()) {
            fontSize = layoutProperty->GetFontSizeValue(Dimension());
        } else {
            fontSize = theme ? theme->GetFontSize() : textStyle.GetFontSize();
        }
    } else {
        if (layoutProperty->HasPlaceholderFontSize() &&
            layoutProperty->GetPlaceholderFontSize().value_or(Dimension()).IsNonNegative()) {
            fontSize = layoutProperty->GetPlaceholderFontSizeValue(Dimension());
        } else {
            fontSize = theme ? theme->GetFontSize() : textStyle.GetFontSize();
        }
    }

    textStyle.SetFontSize(fontSize);
    textStyle.SetFontWeight(
        layoutProperty->GetPlaceholderFontWeightValue(theme ? theme->GetFontWeight() : textStyle.GetFontWeight()));
    if (isDisabled) {
        textStyle.SetTextColor(theme ? theme->GetDisableTextColor() : textStyle.GetTextColor());
        if (layoutProperty->GetShowUnderlineValue(false) &&
            layoutProperty->GetTextInputTypeValue(TextInputType::UNSPECIFIED) == TextInputType::UNSPECIFIED) {
            textStyle.SetTextColor(theme ? theme->GetTextColorDisable() : textStyle.GetTextColor());
        }
    } else {
        textStyle.SetTextColor(layoutProperty->GetPlaceholderTextColorValue(
            theme ? theme->GetPlaceholderColor() : textStyle.GetTextColor()));
    }
    if (layoutProperty->HasPlaceholderMaxLines()) {
        textStyle.SetMaxLines(layoutProperty->GetPlaceholderMaxLines().value());
    }
    if (layoutProperty->HasPlaceholderItalicFontStyle()) {
        textStyle.SetFontStyle(layoutProperty->GetPlaceholderItalicFontStyle().value());
    }
    if (layoutProperty->HasPlaceholderTextAlign()) {
        textStyle.SetTextAlign(layoutProperty->GetPlaceholderTextAlign().value());
    }
    textStyle.SetTextOverflow(TextOverflow::ELLIPSIS);
    textStyle.SetTextAlign(layoutProperty->GetTextAlignValue(TextAlign::START));
}

void TextFieldLayoutAlgorithm::FontRegisterCallback(
    const RefPtr<FrameNode>& frameNode, const std::vector<std::string>& fontFamilies)
{
    auto callback = [weakNode = WeakPtr<FrameNode>(frameNode)] {
        auto frameNode = weakNode.Upgrade();
        CHECK_NULL_VOID(frameNode);
        frameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        auto pattern = frameNode->GetPattern<TextFieldPattern>();
        CHECK_NULL_VOID(pattern);
        auto modifier = DynamicCast<TextFieldContentModifier>(pattern->GetContentModifier());
        CHECK_NULL_VOID(modifier);
        modifier->SetFontReady(true);
    };
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_VOID(pipeline);
    auto fontManager = pipeline->GetFontManager();
    if (fontManager) {
        bool isCustomFont = false;
        for (const auto& familyName : fontFamilies) {
            bool customFont = fontManager->RegisterCallbackNG(frameNode, familyName, callback);
            if (customFont) {
                isCustomFont = true;
            }
        }
        fontManager->AddVariationNodeNG(frameNode);
        if (isCustomFont) {
            auto pattern = frameNode->GetPattern<TextFieldPattern>();
            CHECK_NULL_VOID(pattern);
            pattern->SetIsCustomFont(true);
            auto modifier = DynamicCast<TextFieldContentModifier>(pattern->GetContentModifier());
            CHECK_NULL_VOID(modifier);
            modifier->SetIsCustomFont(true);
        }
    }
}

void TextFieldLayoutAlgorithm::CreateParagraph(const TextStyle& textStyle, std::string content,
    bool needObscureText, int32_t nakedCharPosition, bool disableTextAlign)
{
    RSParagraphStyle paraStyle;
#ifndef USE_GRAPHIC_TEXT_GINE
    paraStyle.textDirection_ = ToRSTextDirection(GetTextDirection(content));
#else
    paraStyle.textDirection = ToRSTextDirection(GetTextDirection(content));
#endif
    if (!disableTextAlign) {
#ifndef USE_GRAPHIC_TEXT_GINE
        paraStyle.textAlign_ = ToRSTextAlign(textStyle.GetTextAlign());
#else
        paraStyle.textAlign = ToRSTextAlign(textStyle.GetTextAlign());
#endif
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    paraStyle.maxLines_ = textStyle.GetMaxLines();
    paraStyle.locale_ = Localization::GetInstance()->GetFontLocale();
    paraStyle.wordBreakType_ = ToRSWordBreakType(textStyle.GetWordBreak());
    paraStyle.fontSize_ = textStyle.GetFontSize().ConvertToPx();
    auto fontFamilies = textStyle.GetFontFamilies();
    if (!fontFamilies.empty()) {
        paraStyle.fontFamily_ = fontFamilies.at(0);
    }
#else
    paraStyle.maxLines = textStyle.GetMaxLines();
    paraStyle.locale = Localization::GetInstance()->GetFontLocale();
    paraStyle.wordBreakType = ToRSWordBreakType(textStyle.GetWordBreak());
    paraStyle.fontSize = textStyle.GetFontSize().ConvertToPx();
    auto fontFamilies = textStyle.GetFontFamilies();
    if (!fontFamilies.empty()) {
        paraStyle.fontFamily = fontFamilies.at(0);
    }
#endif
    if (textStyle.GetTextOverflow() == TextOverflow::ELLIPSIS) {
#ifndef USE_GRAPHIC_TEXT_GINE
        paraStyle.ellipsis_ = StringUtils::Str8ToStr16(StringUtils::ELLIPSIS);
#else
        paraStyle.ellipsis = StringUtils::Str8ToStr16(StringUtils::ELLIPSIS);
#endif
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    auto builder = RSParagraphBuilder::CreateRosenBuilder(paraStyle, RSFontCollection::GetInstance(false));
#else
    auto builder = RSParagraphBuilder::Create(paraStyle, RSFontCollection::Create());
#endif
    builder->PushStyle(ToRSTextStyle(PipelineContext::GetCurrentContext(), textStyle));
    StringUtils::TransformStrCase(content, static_cast<int32_t>(textStyle.GetTextCase()));
#ifndef USE_GRAPHIC_TEXT_GINE
    auto displayText = TextFieldPattern::CreateDisplayText(content, nakedCharPosition, needObscureText);
    builder->AddText(displayText);
#else
    auto displayText = TextFieldPattern::CreateDisplayText(content, nakedCharPosition, needObscureText);
    builder->AppendText(displayText);
#endif

#ifndef USE_GRAPHIC_TEXT_GINE
    builder->Pop();
    auto paragraph = builder->Build();
#else
    builder->PopStyle();
    auto paragraph = builder->CreateTypography();
#endif
    paragraph_.reset(paragraph.release());
}

void TextFieldLayoutAlgorithm::CreateParagraph(const std::vector<TextStyle>& textStyles,
    const std::vector<std::string>& contents, const std::string& content, bool needObscureText, bool disableTextAlign)
{
    auto textStyle = textStyles.begin();
    RSParagraphStyle paraStyle;
#ifndef USE_GRAPHIC_TEXT_GINE
    paraStyle.textDirection_ = ToRSTextDirection(GetTextDirection(content));
#else
    paraStyle.textDirection = ToRSTextDirection(GetTextDirection(content));
#endif
    if (!disableTextAlign) {
#ifndef USE_GRAPHIC_TEXT_GINE
        paraStyle.textAlign_ = ToRSTextAlign(textStyle->GetTextAlign());
#else
        paraStyle.textAlign = ToRSTextAlign(textStyle->GetTextAlign());
#endif
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    paraStyle.maxLines_ = textStyle->GetMaxLines();
    paraStyle.locale_ = Localization::GetInstance()->GetFontLocale();
    paraStyle.wordBreakType_ = ToRSWordBreakType(textStyle->GetWordBreak());
    paraStyle.fontSize_ = textStyle->GetFontSize().ConvertToPx();
    auto fontFamilies = textStyle->GetFontFamilies();
    if (!fontFamilies.empty()) {
        paraStyle.fontFamily_ = fontFamilies.at(0);
    }
#else
    paraStyle.maxLines = textStyle->GetMaxLines();
    paraStyle.locale = Localization::GetInstance()->GetFontLocale();
    paraStyle.wordBreakType = ToRSWordBreakType(textStyle->GetWordBreak());
    paraStyle.fontSize = textStyle->GetFontSize().ConvertToPx();
    paraStyle.fontFamily = textStyle->GetFontFamilies().at(0);
#endif
    if (textStyle->GetTextOverflow() == TextOverflow::ELLIPSIS) {
#ifndef USE_GRAPHIC_TEXT_GINE
        paraStyle.ellipsis_ = StringUtils::Str8ToStr16(StringUtils::ELLIPSIS);
#else
        paraStyle.ellipsis = StringUtils::Str8ToStr16(StringUtils::ELLIPSIS);
#endif
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    auto builder = RSParagraphBuilder::CreateRosenBuilder(paraStyle, RSFontCollection::GetInstance(false));
#else
    auto builder = RSParagraphBuilder::Create(paraStyle, RSFontCollection::Create());
#endif
    for (size_t i = 0; i < contents.size(); i++) {
        std::string splitStr = contents[i];
        if (splitStr.empty()) {
            continue;
        }
        auto& style = textStyles[i];
        builder->PushStyle(ToRSTextStyle(PipelineContext::GetCurrentContext(), style));
        StringUtils::TransformStrCase(splitStr, static_cast<int32_t>(style.GetTextCase()));
        if (needObscureText) {
#ifndef USE_GRAPHIC_TEXT_GINE
            builder->AddText(
                TextFieldPattern::CreateObscuredText(static_cast<int32_t>(StringUtils::ToWstring(splitStr).length())));
#else
            builder->AppendText(
                TextFieldPattern::CreateObscuredText(static_cast<int32_t>(StringUtils::ToWstring(splitStr).length())));
#endif
        } else {
#ifndef USE_GRAPHIC_TEXT_GINE
            builder->AddText(StringUtils::Str8ToStr16(splitStr));
#else
            builder->AppendText(StringUtils::Str8ToStr16(splitStr));
#endif
        }
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    builder->Pop();

    auto paragraph = builder->Build();
#else
    builder->PopStyle();
    auto paragraph = builder->CreateTypography();
#endif
    paragraph_.reset(paragraph.release());
}

void TextFieldLayoutAlgorithm::CreateCounterParagraph(
    int32_t textLength, int32_t maxLength, const RefPtr<TextFieldTheme>& theme)
{
    CHECK_NULL_VOID(theme);
    TextStyle countTextStyle = (textLength != maxLength) ? theme->GetCountTextStyle() : theme->GetOverCountTextStyle();
    std::string counterText = std::to_string(textLength) + "/" + std::to_string(maxLength);
    RSParagraphStyle paraStyle;
#ifndef USE_GRAPHIC_TEXT_GINE
    paraStyle.fontSize_ = countTextStyle.GetFontSize().ConvertToPx();
    paraStyle.textAlign_ = ToRSTextAlign(TextAlign::END);
    paraStyle.maxLines_ = COUNTER_TEXT_MAXLINE;
    auto builder = RSParagraphBuilder::CreateRosenBuilder(paraStyle, RSFontCollection::GetInstance(false));
#else
    paraStyle.fontSize = countTextStyle.GetFontSize().ConvertToPx();
    paraStyle.textAlign = ToRSTextAlign(TextAlign::END);
    paraStyle.maxLines = COUNTER_TEXT_MAXLINE;
    auto builder = RSParagraphBuilder::Create(paraStyle, RSFontCollection::Create());
#endif
    builder->PushStyle(ToRSTextStyle(PipelineContext::GetCurrentContext(), countTextStyle));
    StringUtils::TransformStrCase(counterText, static_cast<int32_t>(countTextStyle.GetTextCase()));
#ifndef USE_GRAPHIC_TEXT_GINE
    builder->AddText(StringUtils::Str8ToStr16(counterText));
    builder->Pop();
#else
    builder->AppendText(StringUtils::Str8ToStr16(counterText));
    builder->PopStyle();
#endif

#ifndef USE_GRAPHIC_TEXT_GINE
    auto paragraph = builder->Build();
#else
    auto paragraph = builder->CreateTypography();
#endif
    counterParagraph_.reset(paragraph.release());
}

void TextFieldLayoutAlgorithm::CreateErrorParagraph(const std::string& content, const RefPtr<TextFieldTheme>& theme)
{
    CHECK_NULL_VOID(theme);
    TextStyle errorTextStyle = theme->GetErrorTextStyle();
    std::string counterText = content;
    RSParagraphStyle paraStyle;
#ifndef USE_GRAPHIC_TEXT_GINE
    paraStyle.fontSize_ = errorTextStyle.GetFontSize().ConvertToPx();
    paraStyle.textAlign_ = ToRSTextAlign(TextAlign::START);
    auto builder = RSParagraphBuilder::CreateRosenBuilder(paraStyle, RSFontCollection::GetInstance(false));
#else
    paraStyle.fontSize = errorTextStyle.GetFontSize().ConvertToPx();
    paraStyle.textAlign = ToRSTextAlign(TextAlign::START);
    auto builder = RSParagraphBuilder::Create(paraStyle, RSFontCollection::Create());
#endif
    builder->PushStyle(ToRSTextStyle(PipelineContext::GetCurrentContext(), errorTextStyle));
    StringUtils::TransformStrCase(counterText, static_cast<int32_t>(errorTextStyle.GetTextCase()));
#ifndef USE_GRAPHIC_TEXT_GINE
    builder->AddText(StringUtils::Str8ToStr16(counterText));
    builder->Pop();

    auto paragraph = builder->Build();
#else
    builder->AppendText(StringUtils::Str8ToStr16(counterText));
    builder->PopStyle();
    auto paragraph = builder->CreateTypography();
#endif
    errorParagraph_.reset(paragraph.release());
}

TextDirection TextFieldLayoutAlgorithm::GetTextDirection(const std::string& content)
{
    TextDirection textDirection = TextDirection::LTR;
    auto showingTextForWString = StringUtils::ToWstring(content);
    for (const auto& charOfShowingText : showingTextForWString) {
        if (u_charDirection(charOfShowingText) == UCharDirection::U_LEFT_TO_RIGHT) {
            textDirection = TextDirection::LTR;
        } else if (u_charDirection(charOfShowingText) == UCharDirection::U_RIGHT_TO_LEFT) {
            textDirection = TextDirection::RTL;
        } else if (u_charDirection(charOfShowingText) == UCharDirection::U_RIGHT_TO_LEFT_ARABIC) {
            textDirection = TextDirection::RTL;
        }
    }
    return textDirection;
}

const std::shared_ptr<RSParagraph>& TextFieldLayoutAlgorithm::GetParagraph()
{
    return paragraph_;
}

const std::shared_ptr<RSParagraph>& TextFieldLayoutAlgorithm::GetCounterParagraph() const
{
    return counterParagraph_;
}

const std::shared_ptr<RSParagraph>& TextFieldLayoutAlgorithm::GetErrorParagraph() const
{
    return errorParagraph_;
}

float TextFieldLayoutAlgorithm::GetTextFieldDefaultHeight()
{
    const auto defaultHeight = 40.0_vp;
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, defaultHeight.ConvertToPx());
    auto textFieldTheme = pipeline->GetTheme<TextFieldTheme>();
    CHECK_NULL_RETURN(textFieldTheme, defaultHeight.ConvertToPx());
    auto height = textFieldTheme->GetHeight();
    return static_cast<float>(height.ConvertToPx());
}

float TextFieldLayoutAlgorithm::GetTextFieldDefaultImageHeight()
{
    const auto defaultHeight = 40.0_vp;
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, defaultHeight.ConvertToPx());
    auto textFieldTheme = pipeline->GetTheme<TextFieldTheme>();
    CHECK_NULL_RETURN(textFieldTheme, defaultHeight.ConvertToPx());
    auto height = textFieldTheme->GetIconHotZoneSize();
    return static_cast<float>(height.ConvertToPx());
}

void TextFieldLayoutAlgorithm::SetPropertyToModifier(
    const TextStyle& textStyle, RefPtr<TextFieldContentModifier> modifier)
{
    CHECK_NULL_VOID(modifier);
    modifier->SetFontFamilies(textStyle.GetFontFamilies());
    modifier->SetFontSize(textStyle.GetFontSize());
    modifier->SetFontWeight(textStyle.GetFontWeight());
    modifier->SetTextColor(textStyle.GetTextColor());
    modifier->SetFontStyle(textStyle.GetFontStyle());
}

void TextFieldLayoutAlgorithm::UpdateUnitLayout(LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    auto children = frameNode->GetChildren();
    const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
    CHECK_NULL_VOID(content);
    auto contentSize = content->GetRect().GetSize();
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    auto layoutProperty = AceType::DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    if (!children.empty() && layoutProperty->GetShowUnderlineValue(false) &&
        layoutProperty->GetTextInputTypeValue(TextInputType::UNSPECIFIED) == TextInputType::UNSPECIFIED) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
        CHECK_NULL_VOID(childWrapper);
        auto textLayoutProperty = DynamicCast<TextLayoutProperty>(childWrapper->GetLayoutProperty());
        auto textGeometryNode = childWrapper->GetGeometryNode();
        CHECK_NULL_VOID(textGeometryNode);
        auto childFrameSize = textGeometryNode->GetFrameSize();
        unitWidth_ = childFrameSize.Width();
        textGeometryNode->SetFrameOffset(
            OffsetF({ content->GetRect().GetX() + contentSize.Width() - childFrameSize.Width(), 0.0 }));
        if (childFrameSize.Height() < size.Height()) {
            childWrapper->GetGeometryNode()->SetFrameSize(SizeF({ unitWidth_, size.Height() }));
        }
        childWrapper->Layout();
    }
}
} // namespace OHOS::Ace::NG
