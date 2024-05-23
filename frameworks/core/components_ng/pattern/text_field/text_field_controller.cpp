/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/text_field/text_field_controller.h"

#include "base/log/log_wrapper.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"

namespace OHOS::Ace::NG {
constexpr int32_t ERROR = -1;

void TextFieldController::Focus(bool focus) {}

void TextFieldController::ShowError(const std::string& errorText) {}

void TextFieldController::Delete() {}

void TextFieldController::CaretPosition(int32_t caretPosition)
{
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    if (textFieldPattern) {
        textFieldPattern->SetCaretPosition(caretPosition);
    }
    if (setCaretPosition_) {
        setCaretPosition_(caretPosition);
    }
}

int32_t TextFieldController::GetCaretIndex()
{
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    if (textFieldPattern) {
        return textFieldPattern->GetCaretIndex();
    }
    if (getCaretIndex_) {
        return getCaretIndex_();
    }

    return ERROR;
}

NG::OffsetF TextFieldController::GetCaretPosition()
{
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    if (textFieldPattern) {
        return textFieldPattern->GetCaretRect().GetOffset();
    }
    if (getCaretPosition_) {
        return getCaretPosition_();
    }

    return OffsetF(ERROR, ERROR);
}

void TextFieldController::SetTextSelection(
    int32_t selectionStart, int32_t selectionEnd, const std::optional<SelectionOptions>& options)
{
    if (selectionEnd < selectionStart) {
        return;
    }
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textFieldPattern);
    auto wideText = textFieldPattern->GetWideText();
    int32_t length = static_cast<int32_t>(wideText.length());
    selectionStart = std::clamp(selectionStart, 0, length);
    selectionEnd = std::clamp(selectionEnd, 0, length);
    textFieldPattern->SetSelectionFlag(selectionStart, selectionEnd, options);
}

Rect TextFieldController::GetTextContentRect()
{
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    if (textFieldPattern == nullptr) {
        if (getTextContentRect_) {
            return getTextContentRect_();
        }
    } else {
        RectF rect = textFieldPattern->GetTextRect();
        if (textFieldPattern->IsTextArea()) {
            textFieldPattern->UpdateRectByTextAlign(rect);
        }
        if (textFieldPattern->IsOperation()) {
            return { rect.GetX(), rect.GetY(), rect.Width(), rect.Height() };
        }
        auto controller = textFieldPattern->GetTextSelectController();
        return { controller->GetCaretRect().GetX(), controller->GetCaretRect().GetY(), 0, 0 };
    }
    return { 0, 0, 0, 0 };
}

int32_t TextFieldController::GetTextContentLinesNum()
{
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    int lines = 0;
    if (textFieldPattern) {
        if (!textFieldPattern->IsOperation()) {
            return lines;
        }
        lines = textFieldPattern->GetLineCount();
        return lines;
    }
    lines = getTextContentLinesNum_();
    return lines;
}

void TextFieldController::StopEditing()
{
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    if (textFieldPattern) {
        textFieldPattern->StopEditing();
    }
    if (stopEditing_) {
        stopEditing_();
    }
}

void TextFieldController::SetPasswordState(bool flag)
{
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textFieldPattern);
    auto passwordArea = AceType::DynamicCast<PasswordResponseArea>(textFieldPattern->GetResponseArea());
    CHECK_NULL_VOID(passwordArea);
    passwordArea->SetObscured(flag);
}

void TextFieldController::Insert(const std::string& args) {}

} // namespace OHOS::Ace::NG
