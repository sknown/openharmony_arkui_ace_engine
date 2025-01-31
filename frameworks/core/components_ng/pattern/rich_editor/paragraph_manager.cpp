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

#include "core/components_ng/pattern/rich_editor/paragraph_manager.h"

#include <iterator>
#include <ostream>

#include "base/utils/utils.h"
#include "core/components/common/properties/text_layout_info.h"

namespace OHOS::Ace::NG {
float ParagraphManager::GetHeight() const
{
    float res = 0.0f;
    for (auto&& info : paragraphs_) {
        res += info.paragraph->GetHeight();
    }
    return res;
}

float ParagraphManager::GetMaxIntrinsicWidth() const
{
    float res = 0.0f;
    for (auto &&info : paragraphs_) {
        res = std::max(res, info.paragraph->GetMaxIntrinsicWidth());
    }
    return res;
}
bool ParagraphManager::DidExceedMaxLines() const
{
    bool res = false;
    for (auto &&info : paragraphs_) {
        res |= info.paragraph->DidExceedMaxLines();
    }
    return res;
}
float ParagraphManager::GetLongestLine() const
{
    float res = 0.0f;
    for (auto &&info : paragraphs_) {
        res = std::max(res, info.paragraph->GetLongestLine());
    }
    return res;
}
float ParagraphManager::GetMaxWidth() const
{
    float res = 0.0f;
    for (auto &&info : paragraphs_) {
        res = std::max(res, info.paragraph->GetMaxWidth());
    }
    return res;
}
float ParagraphManager::GetTextWidth() const
{
    float res = 0.0f;
    for (auto &&info : paragraphs_) {
        res = std::max(res, info.paragraph->GetTextWidth());
    }
    return res;
}

float ParagraphManager::GetTextWidthIncludeIndent() const
{
    float res = 0.0f;
    for (auto &&info : paragraphs_) {
        auto width = info.paragraph->GetTextWidth();
        if (info.paragraph->GetLineCount() == 1) {
            width += static_cast<float>(info.paragraphStyle.indent.ConvertToPx());
        }
        if (info.paragraphStyle.leadingMargin.has_value()) {
            width += static_cast<float>(info.paragraphStyle.leadingMargin->size.Width().ConvertToPx());
        }
        res = std::max(res, width);
    }
    return res;
}

size_t ParagraphManager::GetLineCount() const
{
    size_t count = 0;
    for (auto &&info : paragraphs_) {
        count += info.paragraph->GetLineCount();
    }
    return count;
}

int32_t ParagraphManager::GetIndex(Offset offset, bool clamp) const
{
    CHECK_NULL_RETURN(!paragraphs_.empty(), 0);
    if (clamp && LessNotEqual(offset.GetY(), 0.0)) {
        return 0;
    }
    int idx = 0;
    for (auto it = paragraphs_.begin(); it != paragraphs_.end(); ++it, ++idx) {
        auto&& info = *it;
        if (LessOrEqual(offset.GetY(), info.paragraph->GetHeight()) ||
            (!clamp && idx == static_cast<int>(paragraphs_.size()) - 1)) {
            return info.paragraph->GetGlyphIndexByCoordinate(offset) + info.start;
        }
        // get offset relative to each paragraph
        offset.SetY(offset.GetY() - info.paragraph->GetHeight());
    }
    return paragraphs_.back().end;
}

PositionWithAffinity ParagraphManager::GetGlyphPositionAtCoordinate(Offset offset)
{
    TAG_LOGI(AceLogTag::ACE_TEXT,
        "Get Glyph Position, coordinate = [%{public}.2f %{public}.2f]", offset.GetX(), offset.GetY());
    PositionWithAffinity finalResult(0, TextAffinity::UPSTREAM);
    CHECK_NULL_RETURN(!paragraphs_.empty(), finalResult);
    if (LessNotEqual(offset.GetY(), 0.0)) {
        return finalResult;
    }
    int idx = 0;
    for (auto it = paragraphs_.begin(); it != paragraphs_.end(); ++it, ++idx) {
        auto& info = *it;
        if (LessOrEqual(offset.GetY(), info.paragraph->GetHeight()) ||
            (idx == static_cast<int>(paragraphs_.size()) - 1)) {
            auto result = info.paragraph->GetGlyphPositionAtCoordinate(offset);
            finalResult.position_ = result.position_ + static_cast<size_t>(info.start);
            TAG_LOGI(AceLogTag::ACE_TEXT,
                "Current paragraph, originPos = %{public}zu, finalPos =%{public}zu and affinity = %{public}d",
                result.position_, finalResult.position_, result.affinity_);
            finalResult.affinity_ = static_cast<TextAffinity>(result.affinity_);
            return finalResult;
        }
        // get offset relative to each paragraph
        offset.SetY(offset.GetY() - info.paragraph->GetHeight());
    }
    auto info = paragraphs_.back();
    auto result = info.paragraph->GetGlyphPositionAtCoordinate(offset);
    finalResult.position_ = static_cast<size_t>(info.end);
    finalResult.affinity_ = static_cast<TextAffinity>(result.affinity_);
    TAG_LOGI(AceLogTag::ACE_TEXT,
        "Current paragraph, final position = %{public}zu and affinity = %{public}d", finalResult.position_,
        finalResult.affinity_);
    return finalResult;
}

int32_t ParagraphManager::GetGlyphIndexByCoordinate(Offset offset, bool isSelectionPos) const
{
    CHECK_NULL_RETURN(!paragraphs_.empty(), 0);
    for (auto it = paragraphs_.begin(); it != paragraphs_.end(); ++it) {
        auto &&info = *it;
        if (LessOrEqual(offset.GetY(), info.paragraph->GetHeight())) {
            return info.paragraph->GetGlyphIndexByCoordinate(offset, isSelectionPos) + info.start;
        }
        // get offset relative to each paragraph
        offset.SetY(offset.GetY() - info.paragraph->GetHeight());
    }
    return paragraphs_.back().end;
}

bool ParagraphManager::GetWordBoundary(int32_t offset, int32_t& start, int32_t& end) const
{
    CHECK_NULL_RETURN(!paragraphs_.empty(), false);
    auto offsetIndex = offset;
    auto startIndex = 0;
    auto endIndex = 0;
    for (auto it = paragraphs_.begin(); it != paragraphs_.end(); ++it) {
        auto &&info = *it;
        if (LessNotEqual(offset, info.end)) {
            auto flag = info.paragraph->GetWordBoundary(offsetIndex, start, end);
            start += startIndex;
            end += endIndex;
            return flag;
        }
        // get offset relative to each paragraph
        offsetIndex = offset - info.end;
        startIndex = info.end;
        endIndex = info.end;
    }
    return false;
}

bool ParagraphManager::CalcCaretMetricsByPosition(
    int32_t extent, CaretMetricsF& caretCaretMetric, TextAffinity textAffinity) const
{
    CHECK_NULL_RETURN(!paragraphs_.empty(), false);
    auto offsetIndex = extent;
    auto offsetY = 0.0f;
    auto result = false;
    for (auto it = paragraphs_.begin(); it != paragraphs_.end(); ++it) {
        auto &&info = *it;
        if (textAffinity == TextAffinity::UPSTREAM || std::next(it) == paragraphs_.end()) {
            if (LessOrEqual(extent, info.end)) {
                result = info.paragraph->CalcCaretMetricsByPosition(offsetIndex, caretCaretMetric, textAffinity);
                break;
            }
        } else {
            if (LessNotEqual(extent, info.end)) {
                result = info.paragraph->CalcCaretMetricsByPosition(offsetIndex, caretCaretMetric, textAffinity);
                break;
            }
        }
        // get offset relative to each paragraph
        offsetIndex = extent - info.end;
        offsetY += info.paragraph->GetHeight();
    }
    caretCaretMetric.offset += OffsetF(0.0f, offsetY);
    return result;
}

LineMetrics ParagraphManager::GetLineMetricsByRectF(RectF rect, int32_t paragraphIndex) const
{
    auto index = 0;
    float height = 0;
    auto iter = paragraphs_.begin();
    while (index < paragraphIndex) {
        auto paragraphInfo = *iter;
        height += paragraphInfo.paragraph->GetHeight();
        iter++;
        index++;
    }
    auto paragraphInfo = *iter;
    rect.SetTop(rect.GetY() - height);
    auto lineMetrics = paragraphInfo.paragraph->GetLineMetricsByRectF(rect);
    lineMetrics.y += height;
    return lineMetrics;
}

TextLineMetrics ParagraphManager::GetLineMetrics(size_t lineNumber)
{
    if (lineNumber > GetLineCount() - 1) {
        TAG_LOGE(AceLogTag::ACE_TEXT,
            "GetLineMetrics failed, lineNumber is greater than max lines:%{public}zu", lineNumber);
        return TextLineMetrics();
    }
    size_t endIndex = 0;
    double paragraphsHeight = 0.0;
    size_t lineNumberParam = lineNumber;
    for (auto &&info : paragraphs_) {
        auto lineCount = info.paragraph->GetLineCount();
        if (lineNumber > lineCount - 1) {
            lineNumber -= lineCount;
            paragraphsHeight += info.paragraph->GetHeight();
            auto lastLineMetrics = info.paragraph->GetLineMetrics(lineCount - 1);
            endIndex += lastLineMetrics.endIndex + 1;
            continue;
        }
        auto lineMetrics = info.paragraph->GetLineMetrics(lineNumber);
        lineMetrics.startIndex += endIndex;
        lineMetrics.endIndex += endIndex;
        lineMetrics.lineNumber = lineNumberParam;
        lineMetrics.y += paragraphsHeight;
        lineMetrics.baseline += paragraphsHeight;
        return lineMetrics;
    }
    return TextLineMetrics();
}

std::vector<RectF> ParagraphManager::GetRects(int32_t start, int32_t end) const
{
    std::vector<RectF> res;
    float y = 0.0f;
    for (auto&& info : paragraphs_) {
        std::vector<RectF> rects;
        if (info.start > end) {
            break;
        }
        if (info.end > start) {
            auto relativeStart = (start < info.start) ? 0 : start - info.start;
            info.paragraph->GetRectsForRange(relativeStart, end - info.start, rects);
            for (auto&& rect : rects) {
                rect.SetTop(rect.Top() + y);
            }
            res.insert(res.end(), rects.begin(), rects.end());
        }
        y += info.paragraph->GetHeight();
    }
    return res;
}

bool ParagraphManager::IsSelectLineHeadAndUseLeadingMargin(int32_t start) const
{
    for (auto iter = paragraphs_.begin(); iter != paragraphs_.end(); iter++) {
        auto curParagraph = *iter;
        if (curParagraph.paragraph && curParagraph.paragraph->GetParagraphStyle().leadingMargin &&
            curParagraph.start == start) {
            return true;
        }
        auto next = std::next(iter);
        if (next != paragraphs_.end()) {
            auto nextParagraph = *next;
            if (nextParagraph.paragraph && nextParagraph.paragraph->GetParagraphStyle().leadingMargin &&
                nextParagraph.start == start + 1) {
                return true;
            }
        }
    }
    return false;
}

std::vector<RectF> ParagraphManager::GetPlaceholderRects() const
{
    std::vector<RectF> res;
    float y = 0.0f;
    for (auto&& info : paragraphs_) {
        std::vector<RectF> rects;
        info.paragraph->GetRectsForPlaceholders(rects);
        for (auto& rect : rects) {
            rect.SetTop(rect.Top() + y);
        }
        y += info.paragraph->GetHeight();

        res.insert(res.end(), rects.begin(), rects.end());
    }
    return res;
}

OffsetF ParagraphManager::ComputeCursorOffset(
    int32_t index, float& selectLineHeight, bool downStreamFirst, bool needLineHighest) const
{
    CHECK_NULL_RETURN(!paragraphs_.empty(), {});
    auto it = paragraphs_.begin();
    float y = 0.0f;
    while (it != paragraphs_.end()) {
        if (index >= it->start && index < it->end) {
            break;
        }
        y += it->paragraph->GetHeight();
        ++it;
    }

    if (index == paragraphs_.back().end) {
        --it;
        y -= it->paragraph->GetHeight();
    }

    CHECK_NULL_RETURN(it != paragraphs_.end(), OffsetF(0.0f, y));

    int32_t relativeIndex = index - it->start;
    auto&& paragraph = it->paragraph;
    CaretMetricsF metrics;
    auto computeSuccess = false;
    if (downStreamFirst) {
        computeSuccess = paragraph->ComputeOffsetForCaretDownstream(relativeIndex, metrics, needLineHighest) ||
                         paragraph->ComputeOffsetForCaretUpstream(relativeIndex, metrics, needLineHighest);
    } else {
        computeSuccess = paragraph->ComputeOffsetForCaretUpstream(relativeIndex, metrics, needLineHighest) ||
                         paragraph->ComputeOffsetForCaretDownstream(relativeIndex, metrics, needLineHighest);
    }
    CHECK_NULL_RETURN(computeSuccess, OffsetF(0.0f, y));
    selectLineHeight = metrics.height;
    return { static_cast<float>(metrics.offset.GetX()), static_cast<float>(metrics.offset.GetY() + y) };
}

OffsetF ParagraphManager::ComputeCursorInfoByClick(
    int32_t index, float& selectLineHeight, const OffsetF& lastTouchOffset) const
{
    CHECK_NULL_RETURN(!paragraphs_.empty(), {});
    auto it = paragraphs_.begin();
    float y = 0.0f;
    while (it != paragraphs_.end()) {
        if (index >= it->start && index < it->end) {
            break;
        }
        y += it->paragraph->GetHeight();
        ++it;
    }

    if (index == paragraphs_.back().end) {
        --it;
        y -= it->paragraph->GetHeight();
    }

    CHECK_NULL_RETURN(it != paragraphs_.end(), OffsetF(0.0f, y));

    int32_t relativeIndex = index - it->start;
    auto&& paragraph = it->paragraph;

    CaretMetricsF caretCaretMetric;
    auto touchOffsetInCurrentParagraph = OffsetF(static_cast<float>(lastTouchOffset.GetX()),
        static_cast<float>(lastTouchOffset.GetY() - y));
    TextAffinity textAffinity;
    paragraph->CalcCaretMetricsByPosition(relativeIndex, caretCaretMetric, touchOffsetInCurrentParagraph, textAffinity);
    selectLineHeight = caretCaretMetric.height;
    return { static_cast<float>(caretCaretMetric.offset.GetX()),
        static_cast<float>(caretCaretMetric.offset.GetY() + y) };
}

void ParagraphManager::Reset()
{
    paragraphs_.clear();
}

std::string ParagraphManager::ParagraphInfo::ToString() const
{
    return "Paragraph start: " + std::to_string(start) + ", end: " + std::to_string(end);
}
} // namespace OHOS::Ace::NG
