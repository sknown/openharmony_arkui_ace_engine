/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/text/text_select_overlay.h"

#include <algorithm>
#include <optional>

#include "base/utils/utils.h"
#include "core/components_ng/manager/select_content_overlay/select_content_overlay_manager.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float BOX_EPSILON = 0.5f;
}

bool TextSelectOverlay::PreProcessOverlay(const OverlayRequest& request)
{
    auto pipeline = PipelineContext::GetCurrentContextSafely();
    CHECK_NULL_RETURN(pipeline, false);
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_RETURN(textPattern, false);
    SetUsingMouse(textPattern->IsUsingMouse());
    auto host = textPattern->GetHost();
    CHECK_NULL_RETURN(host, false);
    pipeline->AddOnAreaChangeNode(host->GetId());
    return true;
}

std::optional<SelectHandleInfo> TextSelectOverlay::GetFirstHandleInfo()
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_RETURN(textPattern, std::nullopt);
    SelectHandleInfo handleInfo;
    handleInfo.paintRect = textPattern->GetTextSelector().firstHandle;
    handleInfo.isShow = CheckHandleVisible(handleInfo.paintRect);
    return handleInfo;
}

std::optional<SelectHandleInfo> TextSelectOverlay::GetSecondHandleInfo()
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_RETURN(textPattern, std::nullopt);
    SelectHandleInfo handleInfo;
    handleInfo.paintRect = textPattern->GetTextSelector().secondHandle;
    handleInfo.isShow = CheckHandleVisible(handleInfo.paintRect);
    return handleInfo;
}

bool TextSelectOverlay::CheckHandleVisible(const RectF& paintRect)
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_RETURN(textPattern, false);
    auto host = textPattern->GetHost();
    CHECK_NULL_RETURN(host, false);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, false);
    if (!renderContext->GetClipEdge().value_or(true)) {
        return true;
    }
    auto contentRect = textPattern->GetTextContentRect();
    RectF visibleContentRect(contentRect.GetOffset() + textPattern->GetTextPaintOffset(), contentRect.GetSize());
    visibleContentRect = GetVisibleRect(host, visibleContentRect);
    PointF bottomPoint = { paintRect.Left(), paintRect.Bottom() - BOX_EPSILON };
    PointF topPoint = { paintRect.Left(), paintRect.Top() + BOX_EPSILON };
    return visibleContentRect.IsInRegion(bottomPoint) && visibleContentRect.IsInRegion(topPoint);
}

void TextSelectOverlay::OnResetTextSelection()
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    textPattern->ResetSelection();
}

void TextSelectOverlay::AfterCloseOverlay()
{
    RemoveAreaChangeInner();
}

void TextSelectOverlay::RemoveAreaChangeInner()
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    textPattern->RemoveAreaChangeInner();
}

void TextSelectOverlay::OnHandleMove(const RectF& handleRect, bool isFirst)
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    CHECK_NULL_VOID(textPattern->GetParagraph());
    auto host = textPattern->GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto contentRect = textPattern->GetTextContentRect();
    auto contentOffset = textPattern->GetTextPaintOffset() + contentRect.GetOffset();
    auto handleOffset = handleRect.GetOffset();
    if (renderContext->GetClipEdge().value_or(true)) {
        handleOffset.SetX(
            std::clamp(handleOffset.GetX(), contentOffset.GetX(), contentOffset.GetX() + contentRect.Width()));
        handleOffset.SetY(
            std::clamp(handleOffset.GetY(), contentOffset.GetY(), contentOffset.GetY() + contentRect.Height()));
    }
    auto textPaintOffset = contentOffset - OffsetF(0.0f, std::min(textPattern->GetBaselineOffset(), 0.0f));
    handleOffset -= textPaintOffset;
    // the handle position is calculated based on the middle of the handle height.
    if (isFirst) {
        auto deltaY = handleOffset.GetY() + (IsHandleReverse() ? handleRect.Height() : 0);
        auto start = textPattern->GetHandleIndex(Offset(handleOffset.GetX(), deltaY));
        textPattern->HandleSelectionChange(start, textPattern->GetTextSelector().destinationOffset);
    } else {
        auto deltaY =
            handleOffset.GetY() + (IsHandleReverse() || NearEqual(handleOffset.GetY(), 0) ? 0 : handleRect.Height());
        auto end = textPattern->GetHandleIndex(Offset(handleOffset.GetX(), deltaY));
        textPattern->HandleSelectionChange(textPattern->GetTextSelector().baseOffset, end);
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    auto overlayManager = GetManager<SelectContentOverlayManager>();
    CHECK_NULL_VOID(overlayManager);
    overlayManager->MarkInfoChange(DIRTY_SELECT_TEXT);
}

void TextSelectOverlay::OnHandleMoveDone(const RectF& rect, bool isFirst)
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    textPattern->SetTextResponseType(TextResponseType::LONG_PRESS);
    auto textSelector = textPattern->GetTextSelector();
    textPattern->UpdateSelectionSpanType(textSelector.GetTextStart(), textSelector.GetTextEnd());
    textPattern->CalculateHandleOffsetAndShowOverlay();
    auto overlayManager = GetManager<SelectContentOverlayManager>();
    CHECK_NULL_VOID(overlayManager);
    overlayManager->MarkInfoChange((isFirst ? DIRTY_FIRST_HANDLE : DIRTY_SECOND_HANDLE) | DIRTY_SELECT_AREA |
                                   DIRTY_SELECT_TEXT | DIRTY_COPY_ALL_ITEM);
    if (textPattern->CheckSelectedTypeChange()) {
        CloseOverlay(false, CloseReason::CLOSE_REASON_NORMAL);
        ProcessOverlay({ .animation = true });
    }
    overlayManager->ShowOptionMenu();
    auto host = textPattern->GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

std::string TextSelectOverlay::GetSelectedText()
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_RETURN(textPattern, "");
    auto start = textPattern->GetTextSelector().GetTextStart();
    auto end = textPattern->GetTextSelector().GetTextEnd();
    return textPattern->GetSelectedText(start, end);
}

RectF TextSelectOverlay::GetSelectArea()
{
    auto pattern = GetPattern<TextPattern>();
    RectF res;
    CHECK_NULL_RETURN(pattern, res);
    CHECK_NULL_RETURN(pattern->GetParagraph(), res);
    auto selectRects = pattern->GetTextBoxes();
    auto textPaintOffset = pattern->GetTextPaintOffset();
    if (selectRects.empty()) {
        res.SetOffset(res.GetOffset() + textPaintOffset);
        return res;
    }
    auto contentRect = pattern->GetTextContentRect();
    auto textRect = pattern->GetTextRect();
    res = MergeSelectedBoxes(selectRects, contentRect, textRect, textPaintOffset);
    RectF visibleContentRect(contentRect.GetOffset() + textPaintOffset, contentRect.GetSize());
    visibleContentRect = GetVisibleRect(pattern->GetHost(), visibleContentRect);
    return res.IntersectRectT(visibleContentRect);
}

void TextSelectOverlay::OnUpdateMenuInfo(SelectMenuInfo& menuInfo, SelectOverlayDirtyFlag dirtyFlag)
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    menuInfo.showCopyAll = !textPattern->IsSelectAll();
    if (dirtyFlag == DIRTY_COPY_ALL_ITEM) {
        return;
    }
    menuInfo.menuIsShow = IsShowMenu();
    menuInfo.showCopy = !textPattern->GetTextSelector().SelectNothing();
    menuInfo.showCut = false;
    menuInfo.showPaste = false;
}

void TextSelectOverlay::OnUpdateSelectOverlayInfo(SelectOverlayInfo& overlayInfo, int32_t requestCode)
{
    BaseTextSelectOverlay::OnUpdateSelectOverlayInfo(overlayInfo, requestCode);
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    auto menuIsShow = overlayInfo.menuInfo.menuIsShow;
    overlayInfo.menuInfo.menuIsShow = menuIsShow && (overlayInfo.firstHandle.isShow || overlayInfo.secondHandle.isShow);
    overlayInfo.menuOptionItems = textPattern->GetMenuOptionItems();
    textPattern->CopySelectionMenuParams(overlayInfo);
}

void TextSelectOverlay::OnMenuItemAction(OptionMenuActionId id, OptionMenuType type)
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    switch (id) {
        case OptionMenuActionId::COPY:
            textPattern->HandleOnCopy();
            break;
        case OptionMenuActionId::SELECT_ALL:
            textPattern->HandleOnSelectAll();
            break;
        default:
            TAG_LOGI(AceLogTag::ACE_TEXT, "Unsupported menu option id %{public}d", id);
            break;
    }
}

void TextSelectOverlay::OnCloseOverlay(OptionMenuType menuType, CloseReason reason)
{
    if (reason == CloseReason::CLOSE_REASON_HOLD_BY_OTHER || reason == CloseReason::CLOSE_REASON_TOOL_BAR) {
        auto textPattern = GetPattern<TextPattern>();
        CHECK_NULL_VOID(textPattern);
        textPattern->ResetSelection();
    }
    RemoveAreaChangeInner();
}

void TextSelectOverlay::OnHandleGlobalTouchEvent(SourceType sourceType, TouchType touchType)
{
    auto textPattern = GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    if (IsMouseClickDown(sourceType, touchType) || IsTouchUp(sourceType, touchType)) {
        CloseOverlay(false, CloseReason::CLOSE_REASON_CLICK_OUTSIDE);
        textPattern->ResetSelection();
    }
}
} // namespace OHOS::Ace::NG
