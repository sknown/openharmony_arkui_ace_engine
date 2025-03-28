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

#include "core/components_ng/pattern/rich_editor/rich_editor_base_controller.h"

#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
namespace OHOS::Ace::NG {
void RichEditorBaseController::SetPattern(const WeakPtr<RichEditorPattern>& pattern)
{
    pattern_ = pattern;
}

WeakPtr<LayoutInfoInterface> RichEditorBaseController::GetLayoutInfoInterface()
{
    auto richEditorPattern = pattern_.Upgrade();
    CHECK_NULL_RETURN(richEditorPattern, nullptr);
    return richEditorPattern->GetLayoutInfoInterface();
}

int32_t RichEditorBaseController::GetCaretOffset()
{
    int32_t position = -1;
    auto richEditorPattern = pattern_.Upgrade();
    CHECK_NULL_RETURN(richEditorPattern, -1);
    position = richEditorPattern->GetCaretPosition();
    return position;
}

bool RichEditorBaseController::SetCaretOffset(int32_t caretPosition)
{
    auto richEditorPattern = pattern_.Upgrade();
    CHECK_NULL_RETURN(richEditorPattern, false);
    return richEditorPattern->SetCaretOffset(caretPosition);
}

void RichEditorBaseController::SetTypingStyle(struct UpdateSpanStyle& typingStyle, TextStyle textStyle)
{
    auto richEditorPattern = AceType::DynamicCast<RichEditorPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(richEditorPattern);
    richEditorPattern->SetTypingStyle(typingStyle, textStyle);
}

void RichEditorBaseController::CloseSelectionMenu()
{
    auto richEditorPattern = pattern_.Upgrade();
    CHECK_NULL_VOID(richEditorPattern);
    richEditorPattern->CloseSelectionMenu();
}

bool RichEditorBaseController::IsEditing()
{
    auto richEditorPattern = pattern_.Upgrade();
    CHECK_NULL_RETURN(richEditorPattern, false);
    return richEditorPattern->IsEditing();
}

void RichEditorBaseController::StopEditing()
{
    auto richEditorPattern = pattern_.Upgrade();
    CHECK_NULL_VOID(richEditorPattern);
    richEditorPattern->StopEditing();
}

void RichEditorBaseController::SetSelection(
    int32_t selectionStart, int32_t selectionEnd, const std::optional<SelectionOptions>& options, bool isForward)
{
    auto richEditorPattern = pattern_.Upgrade();
    CHECK_NULL_VOID(richEditorPattern);
    richEditorPattern->SetSelection(selectionStart, selectionEnd, options, isForward);
}
} // namespace OHOS::Ace::NG
