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

#include "core/components_ng/pattern/rich_editor/rich_editor_paint_method.h"

#include "core/animation/scheduler.h"
#include "core/components_ng/pattern/rich_editor/paragraph_manager.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_overlay_modifier.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_theme.h"
#include "core/components_ng/pattern/text/text_content_modifier.h"
#include "core/components_ng/pattern/text/text_overlay_modifier.h"

namespace OHOS::Ace::NG {
RichEditorPaintMethod::RichEditorPaintMethod(const WeakPtr<Pattern>& pattern, const ParagraphManager* pManager,
    float baselineOffset, const RefPtr<TextContentModifier>& contentMod, const RefPtr<TextOverlayModifier>& overlayMod)
    : TextPaintMethod(pattern, baselineOffset, contentMod, overlayMod), pManager_(pManager)
{}

void RichEditorPaintMethod::UpdateOverlayModifier(PaintWrapper* paintWrapper)
{
    TextPaintMethod::UpdateOverlayModifier(paintWrapper);
    auto richEditorPattern = DynamicCast<RichEditorPattern>(GetPattern().Upgrade());
    CHECK_NULL_VOID(richEditorPattern);
    auto overlayMod = DynamicCast<RichEditorOverlayModifier>(GetOverlayModifier(paintWrapper));
    overlayMod->SetPrintOffset(richEditorPattern->GetTextRect().GetOffset());
    overlayMod->SetTextHeight(richEditorPattern->GetTextRect().Height());
    overlayMod->SetScrollOffset(richEditorPattern->GetScrollOffset());
    if (!richEditorPattern->HasFocus()) {
        overlayMod->UpdateScrollBar(paintWrapper);
        overlayMod->SetCaretVisible(false);
        const auto& selection = richEditorPattern->GetTextSelector();
        if (richEditorPattern->GetTextContentLength() > 0 && selection.GetTextStart() != selection.GetTextEnd()) {
            overlayMod->SetSelectedRects(pManager_->GetRects(selection.GetTextStart(), selection.GetTextEnd()));
        }
        return;
    }
    auto caretVisible = richEditorPattern->GetCaretVisible();
    overlayMod->SetShowSelect(richEditorPattern->GetShowSelect());
    overlayMod->SetCaretVisible(caretVisible);
    overlayMod->SetCaretColor(Color::BLUE.GetValue());
    constexpr float CARET_WIDTH = 1.5f;
    overlayMod->SetCaretWidth(static_cast<float>(Dimension(CARET_WIDTH, DimensionUnit::VP).ConvertToPx()));
    SetCaretOffsetAndHeight(paintWrapper);
    std::vector<RectF> selectedRects;
    const auto& selection = richEditorPattern->GetTextSelector();
    if (richEditorPattern->GetTextContentLength() > 0 && selection.GetTextStart() != selection.GetTextEnd()) {
        selectedRects = pManager_->GetRects(selection.GetTextStart(), selection.GetTextEnd());
    }
    auto contentRect = richEditorPattern->GetTextContentRect();
    overlayMod->SetContentRect(contentRect);
    overlayMod->SetSelectedRects(selectedRects);
    auto frameSize = paintWrapper->GetGeometryNode()->GetFrameSize();
    overlayMod->SetFrameSize(frameSize);
    overlayMod->UpdateScrollBar(paintWrapper);
    overlayMod->SetIsClip(false);
}

void RichEditorPaintMethod::SetCaretOffsetAndHeight(PaintWrapper* paintWrapper)
{
    auto richEditorPattern = DynamicCast<RichEditorPattern>(GetPattern().Upgrade());
    CHECK_NULL_VOID(richEditorPattern);
    auto overlayMod = DynamicCast<RichEditorOverlayModifier>(GetOverlayModifier(paintWrapper));
    CHECK_NULL_VOID(overlayMod);
    auto caretPosition = richEditorPattern->GetCaretPosition();
    if (richEditorPattern->GetTextContentLength() <= 0) {
        auto rect = richEditorPattern->GetTextContentRect();
        constexpr float DEFAULT_CARET_HEIGHT = 18.5f;
        overlayMod->SetCaretOffsetAndHeight(
            OffsetF(rect.GetX(), rect.GetY()), Dimension(DEFAULT_CARET_HEIGHT, DimensionUnit::VP).ConvertToPx());
        return;
    }
    float caretHeightUp = 0.0f;
    float caretHeightDown = 0.0f;
    OffsetF caretOffsetUp =
        richEditorPattern->CalcCursorOffsetByPosition(caretPosition, caretHeightUp, false, false);
    OffsetF caretOffsetDown =
        richEditorPattern->CalcCursorOffsetByPosition(caretPosition, caretHeightDown, true, false);
    OffsetF lastClickOffset = richEditorPattern->GetLastClickOffset();
    if (lastClickOffset.NonNegative()) {
        // set caret position by click
        if (NearEqual(lastClickOffset.GetX(), caretOffsetUp.GetX())) {
            overlayMod->SetCaretOffsetAndHeight(caretOffsetUp, caretHeightUp);
        } else {
            overlayMod->SetCaretOffsetAndHeight(caretOffsetDown, caretHeightDown);
        }
    } else {
        if (GreatNotEqual(caretOffsetDown.GetY() + 0.5f, caretOffsetUp.GetY() + caretHeightUp)) {
            overlayMod->SetCaretOffsetAndHeight(caretOffsetDown, caretHeightDown);
        } else {
            overlayMod->SetCaretOffsetAndHeight(caretOffsetUp, caretHeightUp);
        }
    }
}

void RichEditorPaintMethod::UpdateContentModifier(PaintWrapper* paintWrapper)
{
    auto contentMod = DynamicCast<RichEditorContentModifier>(GetContentModifier(paintWrapper));
    CHECK_NULL_VOID(contentMod);
    TextPaintMethod::UpdateContentModifier(paintWrapper);
    auto richEditorPattern = DynamicCast<RichEditorPattern>(GetPattern().Upgrade());
    CHECK_NULL_VOID(richEditorPattern);
    auto richtTextOffset = richEditorPattern->GetTextRect().GetOffset();
    contentMod->SetRichTextRectX(richtTextOffset.GetX());
    contentMod->SetRichTextRectY(richtTextOffset.GetY());

    const auto& geometryNode = paintWrapper->GetGeometryNode();
    auto frameSize = geometryNode->GetPaddingSize();
    OffsetF paddingOffset = geometryNode->GetPaddingOffset() - geometryNode->GetFrameOffset();
    contentMod->SetClipOffset(paddingOffset);
    contentMod->SetClipSize(frameSize);
}
} // namespace OHOS::Ace::NG
