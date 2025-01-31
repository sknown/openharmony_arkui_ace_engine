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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RICH_EDITOR_PARAGRAPH_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RICH_EDITOR_PARAGRAPH_MANAGER_H
#include <list>
#include <optional>

#include "base/geometry/offset.h"
#include "base/memory/ace_type.h"
#include "core/components/common/properties/text_layout_info.h"
#include "core/components_ng/render/paragraph.h"
namespace OHOS::Ace::NG {
class ParagraphManager : public virtual AceType {
    DECLARE_ACE_TYPE(ParagraphManager, AceType);

public:
    struct ParagraphInfo {
        RefPtr<Paragraph> paragraph;
        ParagraphStyle paragraphStyle;
        int32_t start = 0;
        int32_t end = 0;

        std::string ToString() const;
    };
    ParagraphManager() = default;
    std::optional<double> minParagraphFontSize = std::nullopt;

    int32_t GetIndex(Offset offset, bool clamp = false) const;
    PositionWithAffinity GetGlyphPositionAtCoordinate(Offset offset);
    float GetHeight() const;

    const std::list<ParagraphInfo>& GetParagraphs() const
    {
        return paragraphs_;
    }
    void Reset();

    std::vector<RectF> GetRects(int32_t start, int32_t end) const;
    std::vector<RectF> GetPlaceholderRects() const;
    OffsetF ComputeCursorOffset(int32_t index, float& selectLineHeight, bool downStreamFirst = false,
            bool needLineHighest = true) const;
    OffsetF ComputeCursorInfoByClick(int32_t index, float& selectLineHeight, const OffsetF& lastTouchOffset) const;
    bool IsSelectLineHeadAndUseLeadingMargin(int32_t start) const;

    void AddParagraph(ParagraphInfo&& info)
    {
        paragraphs_.emplace_back(std::move(info));
    }

    void SetParagraphs(const std::list<ParagraphInfo>& paragraphs)
    {
        paragraphs_ = paragraphs;
    }

    // add for text
    int32_t GetGlyphIndexByCoordinate(Offset offset, bool isSelectionPos = false) const;
    bool GetWordBoundary(int32_t offset, int32_t& start, int32_t& end) const;
    bool CalcCaretMetricsByPosition(int32_t extent, CaretMetricsF& caretCaretMetric, TextAffinity textAffinity) const;
    float GetMaxIntrinsicWidth() const;
    bool DidExceedMaxLines() const;
    float GetLongestLine() const;
    float GetMaxWidth() const;
    float GetTextWidth() const;
    float GetTextWidthIncludeIndent() const;
    size_t GetLineCount() const;
    LineMetrics GetLineMetricsByRectF(RectF rect, int32_t paragraphIndex) const;
    TextLineMetrics GetLineMetrics(size_t lineNumber);

private:
    std::list<ParagraphInfo> paragraphs_;
};
} // namespace OHOS::Ace::NG
#endif
