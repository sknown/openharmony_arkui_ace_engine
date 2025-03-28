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

#include "bridge/declarative_frontend/jsview/models/richeditor_model_impl.h"

#include "core/components_ng/pattern/rich_editor/rich_editor_model.h"

namespace OHOS::Ace::Framework {
void RichEditorModelImpl::Create(bool isStyledStringMode) {}

RefPtr<RichEditorBaseControllerBase> RichEditorModelImpl::GetRichEditorController()
{
    return nullptr;
}

void RichEditorModelImpl::SetOnReady(std::function<void()>&& func) {}

void RichEditorModelImpl::SetOnSelect(std::function<void(const BaseEventInfo*)>&& func) {}

void RichEditorModelImpl::SetOnSelectionChange(std::function<void(const BaseEventInfo*)>&& func) {}

void RichEditorModelImpl::SetAboutToIMEInput(std::function<bool(const NG::RichEditorInsertValue&)>&& func) {}

void RichEditorModelImpl::SetOnIMEInputComplete(std::function<void(const NG::RichEditorAbstractSpanResult&)>&& func) {}

void RichEditorModelImpl::SetAboutToDelete(std::function<bool(const NG::RichEditorDeleteValue&)>&& func) {}

void RichEditorModelImpl::SetOnDeleteComplete(std::function<void()>&& func) {}

void RichEditorModelImpl::SetCustomKeyboard(std::function<void()>&& func, bool supportAvoidance) {}

void RichEditorModelImpl::SetCopyOption(CopyOptions& copyOptions) {}

void RichEditorModelImpl::BindSelectionMenu(NG::TextSpanType& editorType, NG::TextResponseType& responseType,
    std::function<void()>& buildFunc, NG::SelectMenuParam& menuParam)
{}

void RichEditorModelImpl::SetOnPaste(std::function<void(NG::TextCommonEvent&)>&& func) {}

void RichEditorModelImpl::SetPlaceholder(PlaceholderOptions& options) {}

void RichEditorModelImpl::SetTextDetectEnable(bool value) {}

void RichEditorModelImpl::SetSupportPreviewText(bool value) {}

void RichEditorModelImpl::SetTextDetectConfig(const std::string& value,
    std::function<void(const std::string&)>&& onResult) {}

void RichEditorModelImpl::SetSelectedBackgroundColor(const Color& selectedColor) {}

void RichEditorModelImpl::SetCaretColor(const Color& color) {}

void RichEditorModelImpl::SetOnEditingChange(std::function<void(const bool&)>&& func) {}
void RichEditorModelImpl::SetOnSubmit(std::function<void(int32_t, NG::TextFieldCommonEvent&)>&& func) {}
void RichEditorModelImpl::SetEnterKeyType(TextInputAction value) {}

void RichEditorModelImpl::SetOnWillChange(std::function<bool(const NG::RichEditorChangeValue&)>&& func) {}
void RichEditorModelImpl::SetOnDidChange(std::function<void(const NG::RichEditorChangeValue&)>&& func) {}
void RichEditorModelImpl::SetOnCut(std::function<void(NG::TextCommonEvent&)>&& func) {}
void RichEditorModelImpl::SetOnCopy(std::function<void(NG::TextCommonEvent&)>&& func) {}

} // namespace OHOS::Ace::Framework
