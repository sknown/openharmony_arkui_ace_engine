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

#include "base/memory/ace_type.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"

namespace OHOS::Ace::NG {
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

void TextFieldController::SetTextSelection(int32_t selectionStart, int32_t selectionEnd)
{
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    textFieldPattern->SetSelectionFlag(selectionStart, selectionEnd);
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
        auto y = rect.GetY();
        if (NearEqual(rect.GetY(), 0)) {
            y = textFieldPattern->GetPaddingTop() + textFieldPattern->GetBorderTop();
        }
        textFieldPattern->TextIsEmptyRect(rect);
        textFieldPattern->TextAreaInputRectUpdate(rect);
        rect.SetTop(y);
        textFieldPattern->UpdateRectByAlignment(rect);
        if (textFieldPattern->IsOperation()) {
            return { rect.GetX(), rect.GetY(), rect.Width(), rect.Height() };
        }
        if (NearEqual(rect.GetX(), -Infinity<float>())) {
            return { textFieldPattern->GetPaddingLeft(), rect.GetY(), 0, 0 };
        }
        return { rect.GetX(), rect.GetY(), 0, 0 };
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

void TextFieldController::Insert(const std::string& args) {}

} // namespace OHOS::Ace::NG
