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

#include "core/components_ng/render/adapter/txt_paragraph.h"

#include "base/log/ace_trace.h"
#include "base/utils/utils.h"
#include "base/geometry/dimension.h"
#include "core/components/font/constants_converter.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/render/adapter/pixelmap_image.h"
#include "core/components_ng/render/adapter/txt_font_collection.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/components/common/properties/text_layout_info.h"

namespace OHOS::Ace::NG {
namespace {
const std::u16string ELLIPSIS = u"\u2026";
const std::u16string SYMBOL_TRANS = u"\uF0001";
const int32_t LENGTH_INCREMENT = 2;
constexpr char16_t NEWLINE_CODE = u'\n';
constexpr float TEXT_SPLIT_RATIO = 0.6f;
} // namespace
RefPtr<Paragraph> Paragraph::Create(const ParagraphStyle& paraStyle, const RefPtr<FontCollection>& fontCollection)
{
    auto txtFontCollection = DynamicCast<TxtFontCollection>(fontCollection);
    CHECK_NULL_RETURN(txtFontCollection, nullptr);
    auto sharedFontCollection = txtFontCollection->GetRawFontCollection();
    return AceType::MakeRefPtr<TxtParagraph>(paraStyle, sharedFontCollection);
}

RefPtr<Paragraph> Paragraph::Create(void* rsParagraph)
{
    return AceType::MakeRefPtr<TxtParagraph>(rsParagraph);
}

bool TxtParagraph::IsValid()
{
    return GetParagraph() != nullptr;
}

void TxtParagraph::CreateBuilder()
{
    ACE_TEXT_SCOPED_TRACE("TxtParagraph::CreateBuilder");
    CHECK_NULL_VOID(!hasExternalParagraph_);
    placeholderPosition_.clear();
#ifndef USE_GRAPHIC_TEXT_GINE
    txt::ParagraphStyle style;
    style.text_direction = Constants::ConvertTxtTextDirection(paraStyle_.direction);
    style.text_align = Constants::ConvertTxtTextAlign(paraStyle_.align);
    style.max_lines = paraStyle_.maxLines;
    style.font_size = paraStyle_.fontSize; // libtxt style.font_size
    style.word_break_type = static_cast<minikin::WordBreakType>(paraStyle_.wordBreak);
#else
    Rosen::TypographyStyle style;
    style.textDirection = Constants::ConvertTxtTextDirection(paraStyle_.direction);
    style.textAlign = Constants::ConvertTxtTextAlign(paraStyle_.align);
    style.maxLines = paraStyle_.maxLines;
    style.fontSize = paraStyle_.fontSize; // Rosen style.fontSize
    style.ellipsisModal = static_cast<Rosen::EllipsisModal>(paraStyle_.ellipsisMode);
    style.wordBreakType = static_cast<Rosen::WordBreakType>(paraStyle_.wordBreak);
    style.textSplitRatio = TEXT_SPLIT_RATIO;
    style.breakStrategy = static_cast<Rosen::BreakStrategy>(paraStyle_.lineBreakStrategy);
#endif
    style.locale = paraStyle_.fontLocale;
    if (paraStyle_.textOverflow == TextOverflow::ELLIPSIS) {
        style.ellipsis = ELLIPSIS;
    }
#if !defined(FLUTTER_2_5) && !defined(NEW_SKIA)
    // keep WordBreak define same with WordBreakType in minikin
#ifndef USE_GRAPHIC_TEXT_GINE
    style.word_break_type = static_cast<minikin::WordBreakType>(paraStyle_.wordBreak);
#else
    style.wordBreakType = static_cast<Rosen::WordBreakType>(paraStyle_.wordBreak);
    style.breakStrategy = static_cast<Rosen::BreakStrategy>(paraStyle_.lineBreakStrategy);
#endif
#endif
#ifndef USE_GRAPHIC_TEXT_GINE
    builder_ = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection_);
#else
    builder_ = Rosen::TypographyCreate::Create(style, fontCollection_);
#endif
}

void TxtParagraph::PushStyle(const TextStyle& style)
{
    ACE_TEXT_SCOPED_TRACE("TxtParagraph::PushStyle");
    CHECK_NULL_VOID(!hasExternalParagraph_);
    if (!builder_) {
        CreateBuilder();
    }

#ifndef USE_GRAPHIC_TEXT_GINE
    txt::TextStyle txtStyle;
#else
    Rosen::TextStyle txtStyle;
#endif
    textAlign_ = style.GetTextAlign();
    Constants::ConvertTxtStyle(style, PipelineContext::GetCurrentContext(), txtStyle);
    builder_->PushStyle(txtStyle);
}

void TxtParagraph::PopStyle()
{
    ACE_TEXT_SCOPED_TRACE("TxtParagraph::PopStyle");
    CHECK_NULL_VOID(!hasExternalParagraph_ && builder_);
#ifndef USE_GRAPHIC_TEXT_GINE
    builder_->Pop();
#else
    builder_->PopStyle();
#endif
}

void TxtParagraph::AddText(const std::u16string& text)
{
    ACE_TEXT_SCOPED_TRACE("TxtParagraph::AddText");
    if (!builder_) {
        CreateBuilder();
    }
    text_ += text;
    CHECK_NULL_VOID(!hasExternalParagraph_);
#ifndef USE_GRAPHIC_TEXT_GINE
    builder_->AddText(text);
#else
    builder_->AppendText(text);
#endif
}

void TxtParagraph::AddSymbol(const std::uint32_t& symbolId)
{
    ACE_TEXT_SCOPED_TRACE("TxtParagraph::AddSymbol");
    CHECK_NULL_VOID(!hasExternalParagraph_);
    if (!builder_) {
        CreateBuilder();
    }
    text_ += SYMBOL_TRANS;
    builder_->AppendSymbol(symbolId);
}

int32_t TxtParagraph::AddPlaceholder(const PlaceholderRun& span)
{
    ACE_TEXT_SCOPED_TRACE("TxtParagraph::AddPlaceholder");
    CHECK_NULL_RETURN(!hasExternalParagraph_, 0);
    if (!builder_) {
        CreateBuilder();
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    txt::PlaceholderRun txtSpan;
#else
    OHOS::Rosen::PlaceholderSpan txtSpan;
#endif
    Constants::ConvertPlaceholderRun(span, txtSpan);
#ifndef USE_GRAPHIC_TEXT_GINE
    builder_->AddPlaceholder(txtSpan);
#else
    builder_->AppendPlaceholder(txtSpan);
#endif
    auto position = static_cast<size_t>(placeholderCnt_) + text_.length();
    placeholderPosition_.emplace_back(position);
    return placeholderCnt_++;
}

void TxtParagraph::Build()
{
    ACE_TEXT_SCOPED_TRACE("TxtParagraph::Build");
    CHECK_NULL_VOID(!hasExternalParagraph_ && builder_);
#ifndef USE_GRAPHIC_TEXT_GINE
    paragraph_ = builder_->Build();
#else
    paragraph_ = builder_->CreateTypography();
#endif
}

uint32_t TxtParagraph::destructCount = 0;

TxtParagraph::~TxtParagraph()
{
    if (destructCount % 100 == 0) {
        TAG_LOGI(AceLogTag::ACE_TEXT_FIELD,
            "destroy TxtParagraph with placeholderCnt_ %{public}d, textAlign_ %{public}d, count %{public}u",
            placeholderCnt_, static_cast<int>(textAlign_), destructCount);
    }
    destructCount++;
}

void TxtParagraph::Reset()
{
    paragraph_.reset();
    builder_.reset();
    fontCollection_.reset();
}

void TxtParagraph::Layout(float width)
{
    ACE_TEXT_SCOPED_TRACE("TxtParagraph::Layout");
    CHECK_NULL_VOID(!hasExternalParagraph_ && paragraph_);
    paragraph_->Layout(width);
}

float TxtParagraph::GetHeight()
{
    auto paragrah = GetParagraph();
    CHECK_NULL_RETURN(paragrah, 0.0f);
    return static_cast<float>(paragrah->GetHeight());
}

float TxtParagraph::GetTextWidth()
{
    auto paragrah = GetParagraph();
    CHECK_NULL_RETURN(paragrah, 0.0f);
    if (GetLineCount() == 1) {
#ifndef USE_GRAPHIC_TEXT_GINE
        return std::max(paragrah->GetLongestLine(), paragrah->GetMaxIntrinsicWidth());
#else
        return std::max(paragrah->GetActualWidth(), paragrah->GetMaxIntrinsicWidth());
#endif
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    return paragrah->GetLongestLine();
#else
    return paragrah->GetActualWidth();
#endif
}

float TxtParagraph::GetMaxIntrinsicWidth()
{
    auto paragrah = GetParagraph();
    CHECK_NULL_RETURN(paragrah, 0.0f);
    return static_cast<float>(paragrah->GetMaxIntrinsicWidth());
}

bool TxtParagraph::DidExceedMaxLines()
{
    auto paragrah = GetParagraph();
    CHECK_NULL_RETURN(paragrah, false);
    return paragrah->DidExceedMaxLines();
}

float TxtParagraph::GetLongestLine()
{
    auto paragrah = GetParagraph();
    CHECK_NULL_RETURN(paragrah, 0.0f);
#ifndef USE_GRAPHIC_TEXT_GINE
    return static_cast<float>(paragrah->GetLongestLine());
#else
    return static_cast<float>(paragrah->GetActualWidth());
#endif
}

float TxtParagraph::GetMaxWidth()
{
    auto paragrah = GetParagraph();
    CHECK_NULL_RETURN(paragrah, 0.0f);
    return static_cast<float>(paragrah->GetMaxWidth());
}

float TxtParagraph::GetAlphabeticBaseline()
{
    auto paragrah = GetParagraph();
    CHECK_NULL_RETURN(paragrah, 0.0f);
    return static_cast<float>(paragrah->GetAlphabeticBaseline());
}

size_t TxtParagraph::GetLineCount()
{
#ifndef USE_GRAPHIC_TEXT_GINE
    auto* paragraphTxt = static_cast<txt::ParagraphTxt*>(GetParagraph());
    CHECK_NULL_RETURN(paragraphTxt, 0);
    return paragraphTxt->GetLineCount();
#else
    auto paragrah = GetParagraph();
    CHECK_NULL_RETURN(paragrah, 0);
    return paragrah->GetLineCount();
#endif
}

float TxtParagraph::GetCharacterWidth(int32_t index)
{
    auto paragrah = GetParagraph();
    CHECK_NULL_RETURN(paragrah, 0.0f);
    auto next = index + 1;
#ifndef USE_GRAPHIC_TEXT_GINE
    auto boxes = paragrah->GetRectsForRange(
        index, next, txt::Paragraph::RectHeightStyle::kMax, txt::Paragraph::RectWidthStyle::kTight);
#else
    auto boxes = paragrah->GetTextRectsByBoundary(
        index, next, Rosen::TextRectHeightStyle::COVER_TOP_AND_BOTTOM, Rosen::TextRectWidthStyle::TIGHT);
#endif
    if (boxes.empty()) {
        return 0.0f;
    }
    const auto& textBox = *boxes.begin();
#ifndef USE_GRAPHIC_TEXT_GINE
    return textBox.rect.fRight - textBox.rect.fLeft;
#else
    return textBox.rect.GetRight() - textBox.rect.GetLeft();
#endif
}

void TxtParagraph::Paint(RSCanvas& canvas, float x, float y)
{
    ACE_TEXT_SCOPED_TRACE("TxtParagraph::Paint");
    auto paragrah = GetParagraph();
    CHECK_NULL_VOID(paragrah);
#ifndef USE_ROSEN_DRAWING
    SkCanvas* skCanvas = canvas.GetImpl<RSSkCanvas>()->ExportSkCanvas();
    CHECK_NULL_VOID(skCanvas);
    paragrah->Paint(skCanvas, x, y);
#else
    paragrah->Paint(&canvas, x, y);
#endif
    if (paraStyle_.leadingMargin && paraStyle_.leadingMargin->pixmap) {
        auto size = paraStyle_.leadingMargin->size;
        CaretMetricsF metrics;
        auto flag = ComputeOffsetForCaretUpstream(0, metrics) || ComputeOffsetForCaretDownstream(0, metrics);
        if (flag) {
            x += metrics.offset.GetX() - size.Width().ConvertToPx();
            auto sizeRect = SizeF(size.Width().ConvertToPx(), size.Height().ConvertToPx());
            y += Alignment::GetAlignPosition(
                SizeF(sizeRect.Width(), metrics.height), sizeRect, paraStyle_.leadingMarginAlign)
                    .GetY();
        }
        auto canvasImage = PixelMapImage::Create(paraStyle_.leadingMargin->pixmap);
        auto pixelMapImage = DynamicCast<PixelMapImage>(canvasImage);
        CHECK_NULL_VOID(pixelMapImage);
        auto& rsCanvas = const_cast<RSCanvas&>(canvas);
        auto width = size.Width().ConvertToPx();
        auto height = size.Height().ConvertToPx();
        pixelMapImage->DrawRect(rsCanvas, ToRSRect(RectF(x, y, width, height)));
    }
}

#ifndef USE_ROSEN_DRAWING
void TxtParagraph::Paint(SkCanvas* skCanvas, float x, float y)
{
    auto paragrah = GetParagraph();
    CHECK_NULL_VOID(skCanvas && paragrah);
    paragrah->Paint(skCanvas, x, y);
}
#endif

// ToDo:adjust index
int32_t TxtParagraph::GetGlyphIndexByCoordinate(const Offset& offset, bool isSelectionPos)
{
    auto paragrah = GetParagraph();
    if (!paragrah) {
        return 0;
    }
    int32_t index;
#ifndef USE_GRAPHIC_TEXT_GINE
    index = static_cast<int32_t>(paragrah->GetGlyphPositionAtCoordinate(offset.GetX(), offset.GetY()).position);
#else
    index = static_cast<int32_t>(paragrah->GetGlyphIndexByCoordinate(offset.GetX(), offset.GetY()).index);
#endif
    if (isSelectionPos) {
        AdjustIndexForward(offset, true, index);
    }
    return index;
}

PositionWithAffinity TxtParagraph::GetGlyphPositionAtCoordinate(const Offset& offset)
{
    PositionWithAffinity finalResult(0, TextAffinity::UPSTREAM);
    auto paragrah = GetParagraph();
    CHECK_NULL_RETURN(paragrah, finalResult);
#ifndef USE_GRAPHIC_TEXT_GINE
    auto result = paragrah->GetGlyphPositionAtCoordinate(offset.GetX(), offset.GetY());
    finalResult.position_ = result.pos_;
    finalResult.affinity_ = static_cast<TextAffinity>(result.affinity_);
#else
    auto result = paragrah->GetGlyphIndexByCoordinate(offset.GetX(), offset.GetY());
    finalResult.position_ = result.index;
    finalResult.affinity_ = static_cast<TextAffinity>(result.affinity);
#endif
    return finalResult;
}

void TxtParagraph::AdjustIndexForward(const Offset& offset, bool compareOffset, int32_t& index)
{
    if (index < 0) {
        index = 0;
        return;
    }
    auto totalLen = static_cast<size_t>(placeholderCnt_) + text_.length();
    if (static_cast<unsigned int>(index) == totalLen) {
        --index;
        AdjustIndexForward(offset, false, index);
        return;
    }
    auto next = index + 1;
    auto start = 0;
    auto end = 0;
    if (IsIndexInEmoji(index, start, end)) {
        index = start;
        next = end;
    }
    auto paragrah = GetParagraph();
#ifndef USE_GRAPHIC_TEXT_GINE
    auto boxes = paragrah->GetRectsForRange(
        index, next, txt::Paragraph::RectHeightStyle::kMax, txt::Paragraph::RectWidthStyle::kTight);
#else
    auto boxes = paragrah->GetTextRectsByBoundary(
        index, next, Rosen::TextRectHeightStyle::COVER_TOP_AND_BOTTOM, Rosen::TextRectWidthStyle::TIGHT);
#endif
    if (boxes.empty()) {
        --index;
        AdjustIndexForward(offset, false, index);
        return;
    }
    const auto& textBox = *boxes.begin();
#ifndef USE_GRAPHIC_TEXT_GINE
    auto left = textBox.rect.fLeft;
    auto right = textBox.rect.fRight;
    auto top = textBox.rect.fTop;
#else
    auto left = textBox.rect.GetLeft();
    auto right = textBox.rect.GetRight();
    auto top = textBox.rect.GetTop();
#endif
    if (compareOffset && (LessNotEqual(offset.GetY(), top) || LessNotEqual(offset.GetX(), left))) {
        --index;
        AdjustIndexForward(offset, false, index);
    } else if (NearEqual(left, right)) {
        --index;
        AdjustIndexForward(offset, false, index);
    }
}

bool TxtParagraph::CalCulateAndCheckPreIsPlaceholder(int32_t index, int32_t& extent)
{
    for (auto placeholderIndex : placeholderPosition_) {
        if (placeholderIndex == static_cast<size_t>(index)) {
            return true;
        } else if (placeholderIndex < static_cast<size_t>(index)) {
            extent--;
        }
    }
    return false;
}

bool TxtParagraph::ComputeOffsetForCaretUpstream(int32_t extent, CaretMetricsF& result, bool needLineHighest)
{
    auto paragrah = GetParagraph();
    if (!paragrah) {
        return false;
    }
    if (empty()) {
        return HandleCaretWhenEmpty(result);
    }
    if (static_cast<size_t>(extent) > GetParagraphLength()) {
        extent = GetParagraphLength();
    }

    extent = AdjustIndexForEmoji(extent);
    char16_t prevChar = 0;
    if (static_cast<size_t>(extent) <= text_.length()) {
        prevChar = text_[std::max(0, extent - 1)];
    }

    result.Reset();
    int32_t graphemeClusterLength = StringUtils::NotInUtf16Bmp(prevChar) ? 2 : 1;
    int32_t prev = extent - graphemeClusterLength;
#ifndef USE_GRAPHIC_TEXT_GINE
    auto boxes = paragrah->GetRectsForRange(
        prev, extent, txt::Paragraph::RectHeightStyle::kMax, txt::Paragraph::RectWidthStyle::kTight);
#else
    auto boxes = paragrah->GetTextRectsByBoundary(prev, extent,
        needLineHighest ? Rosen::TextRectHeightStyle::COVER_TOP_AND_BOTTOM : Rosen::TextRectHeightStyle::TIGHT,
        Rosen::TextRectWidthStyle::TIGHT);
#endif
    while (boxes.empty() && !text_.empty()) {
        graphemeClusterLength *= 2;
        prev = extent - graphemeClusterLength;
        if (prev < 0) {
#ifndef USE_GRAPHIC_TEXT_GINE
            boxes = paragrah->GetRectsForRange(
                0, extent, txt::Paragraph::RectHeightStyle::kMax, txt::Paragraph::RectWidthStyle::kTight);
#else
            boxes = paragrah->GetTextRectsByBoundary(0, extent,
                needLineHighest ? Rosen::TextRectHeightStyle::COVER_TOP_AND_BOTTOM : Rosen::TextRectHeightStyle::TIGHT,
                Rosen::TextRectWidthStyle::TIGHT);
#endif
            break;
        }
#ifndef USE_GRAPHIC_TEXT_GINE
        boxes = paragrah->GetRectsForRange(
            prev, extent, txt::Paragraph::RectHeightStyle::kMax, txt::Paragraph::RectWidthStyle::kTight);
#else
        boxes = paragrah->GetTextRectsByBoundary(prev, extent,
            needLineHighest ? Rosen::TextRectHeightStyle::COVER_TOP_AND_BOTTOM : Rosen::TextRectHeightStyle::TIGHT,
            Rosen::TextRectWidthStyle::TIGHT);
#endif
    }
    if (boxes.empty()) {
        return false;
    }

    const auto& textBox = boxes.back();
    // when text_ ends with a \n, return the top position of the next line.
    auto preIsPlaceholder = CalCulateAndCheckPreIsPlaceholder(extent - 1, extent);
    prevChar = text_[std::max(0, extent - 1)];
    if (prevChar == NEWLINE_CODE && !text_[extent] && !preIsPlaceholder) {
        // Return the start of next line.
        result.offset.SetX(MakeEmptyOffsetX());
#ifndef USE_GRAPHIC_TEXT_GINE
        result.offset.SetY(textBox.rect.fBottom);
        result.height = textBox.rect.fBottom - textBox.rect.fTop;
#else
        result.offset.SetY(textBox.rect.GetBottom());
        result.height = textBox.rect.GetBottom() - textBox.rect.GetTop();
#endif
        return true;
    }

#ifndef USE_GRAPHIC_TEXT_GINE
    bool isLtr = textBox.direction == txt::TextDirection::ltr;
#else
    if (isnan(textBox.rect.GetRight()) || isnan(textBox.rect.GetLeft())) {
        LOGI("Right or left of textBox is NaN.");
        return false;
    }
    bool isLtr = textBox.direction == Rosen::TextDirection::LTR;
#endif
    // Caret is within width of the downstream glyphs.
#ifndef USE_GRAPHIC_TEXT_GINE
    double caretStart = isLtr ? textBox.rect.fRight : textBox.rect.fLeft;
#else
    double caretStart = isLtr ? textBox.rect.GetRight() : textBox.rect.GetLeft();
#endif
    float offsetX = std::min(
        static_cast<float>(caretStart), std::max(GetLongestLine(), static_cast<float>(paragrah->GetMaxWidth())));
    result.offset.SetX(offsetX);
#ifndef USE_GRAPHIC_TEXT_GINE
    result.offset.SetY(textBox.rect.fTop);
    result.height = textBox.rect.fBottom - textBox.rect.fTop;
#else
    result.offset.SetY(textBox.rect.GetTop());
    result.height = textBox.rect.GetBottom() - textBox.rect.GetTop();
#endif

    return true;
}

float TxtParagraph::MakeEmptyOffsetX()
{
    auto width = GetMaxWidth();
    switch (textAlign_) {
        case TextAlign::CENTER:
            return width * 0.5f;
        case TextAlign::END:
            return width;
        case TextAlign::START:
        default:
            return 0.0f;
    }
}

bool TxtParagraph::ComputeOffsetForCaretDownstream(int32_t extent, CaretMetricsF& result, bool needLineHighest)
{
    auto paragrah = GetParagraph();
    if (!paragrah || static_cast<size_t>(extent) >= GetParagraphLength()) {
        return false;
    }

    result.Reset();
#ifndef USE_GRAPHIC_TEXT_GINE
    auto getTextRects = [parapraph = paragrah](int32_t extent, int32_t next) {
        return parapraph->GetRectsForRange(
            extent, next, txt::Paragraph::RectHeightStyle::kMax, txt::Paragraph::RectWidthStyle::kTight);
    };
#else
    auto getTextRects = [parapraph = paragrah, needLineHighest](int32_t extent, int32_t next) {
        return parapraph->GetTextRectsByBoundary(extent, next,
            needLineHighest ? Rosen::TextRectHeightStyle::COVER_TOP_AND_BOTTOM : Rosen::TextRectHeightStyle::TIGHT,
            Rosen::TextRectWidthStyle::TIGHT);
    };
#endif
    extent = AdjustIndexForEmoji(extent);
    auto boxes = getTextRects(extent, extent + 1);
    if (boxes.empty() && !text_.empty()) {
        boxes = getTextRects(extent, extent + LENGTH_INCREMENT);

        int32_t start = 0;
        int32_t end = 0;
        // it could be emoji.
        if (boxes.empty() && GetWordBoundary(extent, start, end)) {
            boxes = getTextRects(extent, end);
        }
    }
    if (boxes.empty()) {
        return false;
    }

    const auto& textBox = *boxes.begin();
#ifndef USE_GRAPHIC_TEXT_GINE
    bool isLtr = textBox.direction == txt::TextDirection::ltr;
    double caretStart = isLtr ? textBox.rect.fLeft : textBox.rect.fRight;
#else
    bool isLtr = textBox.direction == Rosen::TextDirection::LTR;
    double caretStart = isLtr ? textBox.rect.GetLeft() : textBox.rect.GetRight();
#endif
    // Caret is within width of the downstream glyphs.
    double offsetX = std::min(caretStart, paragrah->GetMaxWidth());
    result.offset.SetX(offsetX);
#ifndef USE_GRAPHIC_TEXT_GINE
    result.offset.SetY(textBox.rect.fTop);
    result.height = textBox.rect.fBottom - textBox.rect.fTop;
#else
    result.offset.SetY(textBox.rect.GetTop());
    result.height = textBox.rect.GetBottom() - textBox.rect.GetTop();
#endif

    return true;
}

void TxtParagraph::GetRectsForRange(int32_t start, int32_t end, std::vector<RectF>& selectedRects)
{
    auto adjustStart = AdjustIndexForEmoji(start);
    auto adjustEnd = AdjustIndexForEmoji(end);
    GetRectsForRangeInner(adjustStart, adjustEnd, selectedRects);
}

void TxtParagraph::GetRectsForRangeInner(int32_t start, int32_t end, std::vector<RectF>& selectedRects)
{
    auto paragrah = GetParagraph();
    CHECK_NULL_VOID(paragrah);
#ifndef USE_GRAPHIC_TEXT_GINE
    const auto& boxes = paragrah->GetRectsForRange(
        start, end, txt::Paragraph::RectHeightStyle::kMax, txt::Paragraph::RectWidthStyle::kTight);
#else
    const auto& boxes = paragrah->GetTextRectsByBoundary(
        start, end, Rosen::TextRectHeightStyle::COVER_TOP_AND_BOTTOM, Rosen::TextRectWidthStyle::TIGHT);
#endif
    if (boxes.empty()) {
        return;
    }
    for (const auto& box : boxes) {
        auto rect = Constants::ConvertSkRect(box.rect);
        RectF selectionRect(static_cast<float>(rect.Left()), static_cast<float>(rect.Top()),
            static_cast<float>(rect.Width()), static_cast<float>(rect.Height()));
        selectedRects.emplace_back(selectionRect);
    }
}

int32_t TxtParagraph::AdjustIndexForEmoji(int32_t index)
{
    int32_t start = 0;
    int32_t end = 0;
    if (IsIndexInEmoji(index, start, end)) {
        return end;
    }
    return index;
}

bool TxtParagraph::IsIndexInEmoji(int32_t index, int32_t& emojiStart, int32_t& emojiEnd)
{
    auto paragrah = GetParagraph();
    CHECK_NULL_RETURN(paragrah, false);
    int32_t start = 0;
    int32_t end = 0;
    if (!GetWordBoundary(index, start, end)) {
        return false;
    }
    std::vector<RectF> selectedRects;
    // if index in emoji or the first or the last, selectedRects is empty and
    // 'end' will be emoji's end index or 0 or the max index.
    GetRectsForRangeInner(index, end, selectedRects);
    if (selectedRects.empty()) {
        emojiStart = start;
        emojiEnd = end;
        return true;
    }
    return false;
}

void TxtParagraph::GetRectsForPlaceholders(std::vector<RectF>& selectedRects)
{
    auto paragrah = GetParagraph();
    CHECK_NULL_VOID(paragrah);
#ifndef USE_GRAPHIC_TEXT_GINE
    const auto& boxes = paragrah->GetRectsForPlaceholders();
#else
    const auto& boxes = paragrah->GetTextRectsOfPlaceholders();
#endif
    if (boxes.empty()) {
        return;
    }
    for (const auto& box : boxes) {
        auto rect = Constants::ConvertSkRect(box.rect);
        RectF selectionRect(static_cast<float>(rect.Left()), static_cast<float>(rect.Top()),
            static_cast<float>(rect.Width()), static_cast<float>(rect.Height()));
        selectedRects.emplace_back(selectionRect);
    }
}

bool TxtParagraph::CalcCaretMetricsByPosition(
    int32_t extent, CaretMetricsF& caretCaretMetric, TextAffinity textAffinity)
{
    CaretMetricsF metrics;
    bool computeSuccess = false;
    if (textAffinity == TextAffinity::DOWNSTREAM) {
        computeSuccess =
            ComputeOffsetForCaretDownstream(extent, metrics) || ComputeOffsetForCaretUpstream(extent, metrics);
    } else {
        computeSuccess =
            ComputeOffsetForCaretUpstream(extent, metrics) || ComputeOffsetForCaretDownstream(extent, metrics);
    }
    if (computeSuccess) {
        if (metrics.height <= 0 || std::isnan(metrics.height)) {
            // The reason may be text lines is exceed the paragraph maxline.
            return false;
        }
        caretCaretMetric = metrics;
        return true;
    }
    return false;
}

bool TxtParagraph::CalcCaretMetricsByPosition(
    int32_t extent, CaretMetricsF& caretCaretMetric, const OffsetF& lastTouchOffset, TextAffinity& textAffinity)
{
    CaretMetricsF metricsUpstream;
    CaretMetricsF metricsDownstream;
    auto downStreamSuccess = ComputeOffsetForCaretDownstream(extent, metricsDownstream);
    auto upStreamSuccess = ComputeOffsetForCaretUpstream(extent, metricsUpstream);
    if (downStreamSuccess || upStreamSuccess) {
        if ((metricsDownstream.offset.GetY() < lastTouchOffset.GetY()) && downStreamSuccess) {
            caretCaretMetric = metricsDownstream;
            textAffinity = TextAffinity::DOWNSTREAM;
        } else if (upStreamSuccess) {
            caretCaretMetric = metricsUpstream;
            textAffinity = TextAffinity::UPSTREAM;
        } else {
            caretCaretMetric = metricsDownstream;
            textAffinity = TextAffinity::DOWNSTREAM;
        }
        return true;
    }
    textAffinity = TextAffinity::DOWNSTREAM;
    return false;
}

void TxtParagraph::SetIndents(const std::vector<float>& indents)
{
#ifndef USE_GRAPHIC_TEXT_GINE
    auto* paragraphTxt = static_cast<txt::ParagraphTxt*>(GetParagraph());
#else
    auto* paragraphTxt = static_cast<OHOS::Rosen::Typography*>(GetParagraph());
#endif
    CHECK_NULL_VOID(paragraphTxt);
    paragraphTxt->SetIndents(indents);
}

bool TxtParagraph::GetWordBoundary(int32_t offset, int32_t& start, int32_t& end)
{
    auto paragrah = GetParagraph();
    CHECK_NULL_RETURN(paragrah, false);
#ifndef USE_GRAPHIC_TEXT_GINE
    auto* paragraphTxt = static_cast<txt::ParagraphTxt*>(paragrah);
#else
    auto* paragraphTxt = static_cast<OHOS::Rosen::Typography*>(paragrah);
#endif
    CHECK_NULL_RETURN(paragraphTxt, false);
#ifndef USE_GRAPHIC_TEXT_GINE
    auto range = paragraphTxt->GetWordBoundary(static_cast<size_t>(offset));
    start = static_cast<int32_t>(range.start);
    end = static_cast<int32_t>(range.end);
#else
    auto range = paragraphTxt->GetWordBoundaryByIndex(static_cast<size_t>(offset));
    start = static_cast<int32_t>(range.leftIndex);
    end = static_cast<int32_t>(range.rightIndex);
#endif
    return true;
}

void TxtParagraph::HandleTextAlign(CaretMetricsF& result, TextAlign align)
{
    auto width = GetMaxWidth();
    float offsetX = 0.0f;
    switch (align) {
        case TextAlign::CENTER:
            offsetX = width * 0.5f;
            break;
        case TextAlign::END:
            offsetX = width;
            break;
        case TextAlign::START:
        default:
            break;
    }
    result.offset.SetX(offsetX);
}

void TxtParagraph::HandleLeadingMargin(CaretMetricsF& result, LeadingMargin leadingMargin)
{
    result.offset.SetX(leadingMargin.size.Width().ConvertToPx());
}

bool TxtParagraph::HandleCaretWhenEmpty(CaretMetricsF& result)
{
    auto paragrah = GetParagraph();
    if (!paragrah || paragrah->GetLineCount() == 0) {
        return false;
    }

    result.offset.Reset();
    result.height = paragrah->GetHeight();
    auto lineHeight = paraStyle_.lineHeight;
    if (lineHeight.IsValid()) {
        result.offset.SetY(std::max(lineHeight.ConvertToPx() - result.height, 0.0));
    }
    if (paraStyle_.align != TextAlign::START) {
        HandleTextAlign(result, paraStyle_.align);
    } else if (paraStyle_.leadingMargin) {
        HandleLeadingMargin(result, *(paraStyle_.leadingMargin));
    }
    return true;
}

LineMetrics TxtParagraph::GetLineMetricsByRectF(RectF& rect)
{
#ifndef USE_GRAPHIC_TEXT_GINE
    auto* paragraphTxt = static_cast<txt::ParagraphTxt*>(GetParagraph());
#else
    auto* paragraphTxt = static_cast<OHOS::Rosen::Typography*>(GetParagraph());
#endif
    LineMetrics lineMetrics;
    CHECK_NULL_RETURN(paragraphTxt, lineMetrics);
    auto metrics = paragraphTxt->GetLineMetrics();
    if (metrics.empty()) {
        return lineMetrics;
    }
    auto top = std::floor(rect.Top() + 0.5);
    auto bottom = std::floor(rect.Bottom() + 0.5);
    auto res = metrics.size() - 1;
    for (size_t index = 0; index < metrics.size() - 1; index++) {
        if (metrics[index].y <= top && metrics[index + 1].y >= bottom) {
            res = index;
            break;
        }
    }
    auto resMetric = metrics[res];
    lineMetrics.x = resMetric.x;
    lineMetrics.y = resMetric.y;
    lineMetrics.ascender = resMetric.ascender;
    lineMetrics.height = resMetric.height;
    return lineMetrics;
}

TextLineMetrics TxtParagraph::GetLineMetrics(size_t lineNumber)
{
#ifndef USE_GRAPHIC_TEXT_GINE
    auto* paragraphTxt = static_cast<txt::ParagraphTxt*>(GetParagraph());
#else
    auto* paragraphTxt = static_cast<OHOS::Rosen::Typography*>(GetParagraph());
#endif
    TextLineMetrics lineMetrics;
    OHOS::Rosen::LineMetrics resMetric;
    CHECK_NULL_RETURN(paragraphTxt, lineMetrics);
    bool success = paragraphTxt->GetLineMetricsAt(lineNumber, &resMetric);
    if (!success) {
        TAG_LOGE(AceLogTag::ACE_TEXT, "TxtParagraph::GetLineMetrics failed");
    }
 
    lineMetrics.ascender = resMetric.ascender;
    lineMetrics.descender = resMetric.descender;
    lineMetrics.capHeight = resMetric.capHeight;
    lineMetrics.xHeight = resMetric.xHeight;
    lineMetrics.width = resMetric.width;
    lineMetrics.height = resMetric.height;
    lineMetrics.x = resMetric.x;
    lineMetrics.y = resMetric.y;
    lineMetrics.startIndex = resMetric.startIndex;
    lineMetrics.endIndex = resMetric.endIndex;
    lineMetrics.baseline = resMetric.baseline;
    lineMetrics.lineNumber = resMetric.lineNumber;
 
    if (resMetric.runMetrics.empty()) {
        TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "GetLineMetrics runMetrics is empty.");
        return lineMetrics;
    }
 
    auto runMetricsResMap = resMetric.runMetrics;
    for (const auto& it : runMetricsResMap) {
        RunMetrics runMetrics;
        auto runMetricsRes = it.second;
        SetRunMetrics(runMetrics, runMetricsRes);
        lineMetrics.runMetrics.insert(std::map<size_t, RunMetrics>::value_type(it.first, runMetrics));
    }
    return lineMetrics;
}
 
void TxtParagraph::SetRunMetrics(RunMetrics& runMetrics, const OHOS::Rosen::RunMetrics& runMetricsRes)
{
    auto textStyleRes = runMetricsRes.textStyle;
    runMetrics.textStyle.SetTextDecoration(static_cast<TextDecoration>(textStyleRes->decoration));
    runMetrics.textStyle.SetFontWeight(static_cast<FontWeight>(textStyleRes->fontWeight));
    runMetrics.textStyle.SetFontStyle(static_cast<FontStyle>(textStyleRes->fontStyle));
    runMetrics.textStyle.SetTextBaseline(static_cast<TextBaseline>(textStyleRes->baseline));
    runMetrics.textStyle.SetFontFamilies(textStyleRes->fontFamilies);
    runMetrics.textStyle.SetFontSize(Dimension(textStyleRes->fontSize, DimensionUnit::VP));
    runMetrics.textStyle.SetLetterSpacing(Dimension(textStyleRes->letterSpacing, DimensionUnit::VP));
    runMetrics.textStyle.SetWordSpacing(Dimension(textStyleRes->wordSpacing, DimensionUnit::VP));
    runMetrics.textStyle.SetHeightScale(textStyleRes->heightScale);
    runMetrics.textStyle.SetHalfLeading(textStyleRes->halfLeading);
    runMetrics.textStyle.SetHeightOnly(textStyleRes->heightOnly);
    runMetrics.textStyle.SetEllipsis(textStyleRes->ellipsis);
    runMetrics.textStyle.SetEllipsisMode(static_cast<EllipsisMode>(textStyleRes->ellipsisModal));
    runMetrics.textStyle.SetLocale(textStyleRes->locale);
 
    auto fontMetricsRes = runMetricsRes.fontMetrics;
    runMetrics.fontMetrics.fFlags = fontMetricsRes.fFlags;
    runMetrics.fontMetrics.fTop = fontMetricsRes.fTop;
    runMetrics.fontMetrics.fAscent = fontMetricsRes.fAscent;
    runMetrics.fontMetrics.fDescent = fontMetricsRes.fDescent;
    runMetrics.fontMetrics.fBottom = fontMetricsRes.fBottom;
    runMetrics.fontMetrics.fLeading = fontMetricsRes.fLeading;
    runMetrics.fontMetrics.fAvgCharWidth = fontMetricsRes.fAvgCharWidth;
    runMetrics.fontMetrics.fMaxCharWidth = fontMetricsRes.fMaxCharWidth;
    runMetrics.fontMetrics.fXMin = fontMetricsRes.fXMin;
    runMetrics.fontMetrics.fXMax = fontMetricsRes.fXMax;
    runMetrics.fontMetrics.fXHeight = fontMetricsRes.fXHeight;
    runMetrics.fontMetrics.fCapHeight = fontMetricsRes.fCapHeight;
    runMetrics.fontMetrics.fUnderlineThickness = fontMetricsRes.fUnderlineThickness;
    runMetrics.fontMetrics.fUnderlinePosition = fontMetricsRes.fUnderlinePosition;
    runMetrics.fontMetrics.fStrikeoutThickness = fontMetricsRes.fStrikeoutThickness;
    runMetrics.fontMetrics.fStrikeoutPosition = fontMetricsRes.fStrikeoutPosition;
}

bool TxtParagraph::GetLineMetricsByCoordinate(const Offset& offset, LineMetrics& lineMetrics)
{
#ifndef USE_GRAPHIC_TEXT_GINE
    auto* paragraphTxt = static_cast<txt::ParagraphTxt*>(GetParagraph());
#else
    auto* paragraphTxt = static_cast<OHOS::Rosen::Typography*>(GetParagraph());
#endif
    CHECK_NULL_RETURN(paragraphTxt, false);
    auto lineCount = static_cast<int32_t>(GetLineCount());
    if (lineCount <= 0) {
        return false;
    }
    auto height = GetHeight();
    if (height <= 0) {
        return false;
    }
    auto averageLineHeight = height / lineCount;
    auto lineNumber = std::clamp(static_cast<int32_t>(offset.GetY() / averageLineHeight), 0, lineCount - 1);
    Rosen::LineMetrics resMetric;
    auto ret = paragraphTxt->GetLineMetricsAt(lineNumber, &resMetric);
    while (ret) {
        if (GreatOrEqual(offset.GetY(), resMetric.y) && LessOrEqual(offset.GetY(), resMetric.y + resMetric.height)) {
            break;
        }
        if (LessNotEqual(offset.GetY(), resMetric.y)) {
            lineNumber--;
            ret = paragraphTxt->GetLineMetricsAt(lineNumber, &resMetric);
            continue;
        }
        if (GreatNotEqual(offset.GetY(), resMetric.y + resMetric.height)) {
            lineNumber++;
            ret = paragraphTxt->GetLineMetricsAt(lineNumber, &resMetric);
            continue;
        }
        ret = false;
    }
    if (ret) {
        lineMetrics.x = resMetric.x;
        lineMetrics.y = resMetric.y;
        lineMetrics.ascender = resMetric.ascender;
        lineMetrics.width = resMetric.width;
        lineMetrics.height = resMetric.height;
        lineMetrics.descender = resMetric.descender;
        lineMetrics.capHeight = resMetric.capHeight;
        lineMetrics.xHeight = resMetric.xHeight;
    }
    return ret;
}

std::u16string TxtParagraph::GetParagraphText()
{
    return text_;
}

const ParagraphStyle& TxtParagraph::GetParagraphStyle() const
{
    return paraStyle_;
}

#ifndef USE_GRAPHIC_TEXT_GINE
txt::Paragraph* TxtParagraph::GetParagraph()
{
    return paragraph_.get();
}
#else
RSParagraph* TxtParagraph::GetParagraph()
{
    if (paragraph_) {
        return paragraph_.get();
    }
    return externalParagraph_;
}
#endif
} // namespace OHOS::Ace::NG
