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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SEARCH_SEARCH_MODEL_NG_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SEARCH_SEARCH_MODEL_NG_H

#include "base/memory/referenced.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/search/search_model.h"
#include "core/components_ng/pattern/search/search_node.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT SearchModelNG : public OHOS::Ace::SearchModel {
public:
    RefPtr<TextFieldControllerBase> Create(const std::optional<std::string>& value,
        const std::optional<std::string>& placeholder, const std::optional<std::string>& icon) override;
    void RequestKeyboardOnFocus(bool needToRequest) override;
    void SetSearchButton(const std::string& text) override;
    void SetCaretWidth(const Dimension& value) override;
    void SetCaretColor(const Color& color) override;
    void SetSearchIconSize(const Dimension& value) override;
    void SetSearchIconColor(const Color& color) override;
    void SetSearchSrcPath(
        const std::string& src, const std::string& bundleName, const std::string& moduleName) override;
    void SetRightIconSrcPath(const std::string& src) override;
    void SetCancelButtonStyle(CancelButtonStyle cancelButtonStyle) override;
    void SetCancelIconSize(const Dimension& value) override;
    void SetCancelIconColor(const Color& color) override;
    void SetSearchButtonFontSize(const Dimension& value) override;
    void SetSearchButtonFontColor(const Color& color) override;
    void SetPlaceholderColor(const Color& color) override;
    void SetPlaceholderFont(const Font& font) override;
    void SetTextFont(const Font& font) override;
    void SetTextColor(const Color& color) override;
    void SetTextAlign(const TextAlign& textAlign) override;
    void SetCopyOption(const CopyOptions& copyOptions) override;
    void SetMenuOptionItems(std::vector<MenuOptionsParam>&& menuOptionsItems) override;
    void SetHeight(const Dimension& height) override;
    void SetOnSubmit(std::function<void(const std::string&)>&& onSubmit) override;
    void SetOnChange(std::function<void(const std::string&, TextRange&)>&& onChange) override;
    void SetOnTextSelectionChange(std::function<void(int32_t, int32_t)>&& func) override;
    void SetOnScroll(std::function<void(float, float)>&& func) override;
    void SetOnCopy(std::function<void(const std::string&)>&& func) override;
    void SetOnCut(std::function<void(const std::string&)>&& func) override;
    void SetOnWillInsertValueEvent(std::function<bool(const InsertValueInfo&)>&& func) override;
    void SetOnDidInsertValueEvent(std::function<void(const InsertValueInfo&)>&& func) override;
    void SetOnWillDeleteEvent(std::function<bool(const DeleteValueInfo&)>&& func) override;
    void SetOnDidDeleteEvent(std::function<void(const DeleteValueInfo&)>&& func) override;
    void SetOnPaste(std::function<void(const std::string&)>&& func) override;
    void SetOnPasteWithEvent(std::function<void(const std::string&, NG::TextCommonEvent&)>&& func) override;
    void SetOnChangeEvent(std::function<void(const std::string&)>&& onChangeEvent) override;
    void SetSelectionMenuHidden(bool selectionMenuHidden) override;
    void SetCustomKeyboard(const std::function<void ()> &&buildFunc, bool supportAvoidance = false) override;
    void SetSearchEnterKeyType(TextInputAction value) override;
    void SetInputFilter(const std::string& value, const std::function<void(const std::string&)>& onError) override;
    void SetOnEditChanged(std::function<void(bool)>&& func) override;
    void SetTextIndent(const Dimension& value) override;
    void SetMaxLength(uint32_t value) override;
    void ResetMaxLength() override;
    void SetType(TextInputType value) override;
    void SetLetterSpacing(const Dimension& value) override;
    void SetLineHeight(const Dimension& value) override;
    void SetAdaptMinFontSize(const Dimension& value) override;
    void SetAdaptMaxFontSize(const Dimension& value) override;
    void SetTextDecoration(Ace::TextDecoration value) override;
    void SetTextDecorationColor(const Color& value) override;
    void SetTextDecorationStyle(Ace::TextDecorationStyle value) override;
    void SetFontFeature(const FONT_FEATURES_LIST& value) override;
    void UpdateInspectorId(const std::string& key) override;
    void SetDragPreviewOptions(const NG::DragPreviewOption option) override;
    void SetSelectedBackgroundColor(const Color& value) override;
    void SetSelectionMenuOptions(const std::vector<MenuOptionsParam>&& menuOptionsItems) override;
    void SetEnablePreviewText(bool enablePreviewText) override;
    static RefPtr<SearchNode> CreateFrameNode(int32_t nodeId);
    static void SetTextValue(FrameNode* frameNode, const std::optional<std::string>& value);
    static void SetPlaceholder(FrameNode* frameNode, const std::optional<std::string>& placeholder);
    static void SetIcon(FrameNode* frameNode, const std::optional<std::string>& icon);
    static void SetCaretPosition(FrameNode* frameNode, const int32_t& value);
    static void SetAdaptMinFontSize(FrameNode* frameNode, const Dimension& value);
    static void SetInputFilter(
        FrameNode* frameNode, const std::string& value, const std::function<void(const std::string&)>& onError);
    static void SetAdaptMaxFontSize(FrameNode* frameNode, const Dimension& value);
    static void SetTextIndent(FrameNode* frameNode, const Dimension& value);
    static void RequestKeyboardOnFocus(FrameNode* frameNode, bool needToRequest);
    static void SetPlaceholderFont(FrameNode* frameNode, const Font& font);
    static void SetSearchIconSize(FrameNode* frameNode, const Dimension& value);
    static void SetSearchSrcPath(FrameNode* frameNode, const std::string& src);
    static void SetSearchIconColor(FrameNode* frameNode, const Color& color);
    static void SetSearchButton(FrameNode* frameNode, const std::string& text);
    static void SetSearchButtonFontSize(FrameNode* frameNode, const Dimension& value);
    static void SetSearchButtonFontColor(FrameNode* frameNode, const Color& color);
    static void SetTextColor(FrameNode* frameNode, const Color& color);
    static void SetCopyOption(FrameNode* frameNode, const CopyOptions& copyOptions);
    static void SetTextFont(FrameNode* frameNode, const Font& font);
    static void SetPlaceholderColor(FrameNode* frameNode, const Color& color);
    static void SetSelectionMenuHidden(FrameNode* frameNode, bool selectionMenuHidden);
    static void SetCaretWidth(FrameNode* frameNode, const Dimension& value);
    static void SetCaretColor(FrameNode* frameNode, const Color& color);
    static void SetTextAlign(FrameNode* frameNode, const TextAlign& textAlign);
    static void SetRightIconSrcPath(FrameNode* frameNode, const std::string& src);
    static void SetCancelIconColor(FrameNode* frameNode, const Color& color);
    static void SetCancelIconSize(FrameNode* frameNode, const Dimension& value);
    static void SetCancelButtonStyle(FrameNode* frameNode, CancelButtonStyle style);
    static void SetHeight(FrameNode* frameNode, const Dimension& height);
    static void SetSearchEnterKeyType(FrameNode* frameNode, TextInputAction value);
    static void SetId(FrameNode* frameNode, const std::string& key);
    static void SetTextDecoration(FrameNode* frameNode, Ace::TextDecoration value);
    static void SetTextDecorationColor(FrameNode* frameNode, const Color& value);
    static void SetTextDecorationStyle(FrameNode* frameNode, Ace::TextDecorationStyle value);
    static void SetLetterSpacing(FrameNode* frameNode, const Dimension& value);
    static void SetLineHeight(FrameNode* frameNode, const Dimension& value);
    static void SetFontFeature(FrameNode* frameNode, const FONT_FEATURES_LIST& value);
    static void SetSelectedBackgroundColor(FrameNode* frameNode, const Color& value);
    static void SetOnSubmit(FrameNode* frameNode, std::function<void(const std::string&)>&& onSubmit);
    static void SetOnChange(FrameNode* frameNode, std::function<void(const std::string&, TextRange&)>&& onChange);
    static void SetOnCopy(FrameNode* frameNode, std::function<void(const std::string&)>&& func);
    static void SetOnCut(FrameNode* frameNode, std::function<void(const std::string&)>&& func);
    static void SetOnPasteWithEvent(FrameNode* frameNode,
                                    std::function<void(const std::string&, NG::TextCommonEvent&)>&& func);
    static void SetMaxLength(FrameNode* frameNode, uint32_t value);
    static void ResetMaxLength(FrameNode* frameNode);
    static void SetType(FrameNode* frameNode, TextInputType value);
    static void SetOnEditChange(FrameNode* frameNode, std::function<void(bool)>&& func);
    static void SetOnTextSelectionChange(FrameNode* frameNode, std::function<void(int32_t, int32_t)>&& func);
    static void SetOnContentScroll(FrameNode* frameNode, std::function<void(float, float)>&& func);
    static void SetShowCounter(FrameNode* frameNode, bool value);
    static void SetCounterType(FrameNode* frameNode, int32_t value);
    static void SetShowCounterBorder(FrameNode* frameNode, bool value);
    static RefPtr<TextFieldControllerBase> GetSearchController(FrameNode* frameNode);
    static void SetOnWillInsertValueEvent(FrameNode* frameNode, std::function<bool(const InsertValueInfo&)>&& func);
    static void SetOnDidInsertValueEvent(FrameNode* frameNode, std::function<void(const InsertValueInfo&)>&& func);
    static void SetOnWillDeleteEvent(FrameNode* frameNode, std::function<bool(const DeleteValueInfo&)>&& func);
    static void SetOnDidDeleteEvent(FrameNode* frameNode, std::function<void(const DeleteValueInfo&)>&& func);
    static void SetEnablePreviewText(FrameNode* frameNode, bool enablePreviewText);

private:
    static RefPtr<SearchNode> CreateSearchNode(int32_t nodeId, const std::optional<std::string>& value,
        const std::optional<std::string>& placeholder, const std::optional<std::string>& icon);
    static void CreateTextField(const RefPtr<SearchNode>& parentNode,
        const std::optional<std::string>& placeholder, const std::optional<std::string>& value, bool hasTextFieldNode);
    static void CreateButton(const RefPtr<SearchNode>& parentNode, bool hasButtonNode);
    static void CreateCancelButton(const RefPtr<SearchNode>& parentNode, bool hasCancelButtonNode);
    static RefPtr<SearchNode> GetOrCreateSearchNode(
        const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator);
    RefPtr<FrameNode> GetSearchTextFieldFrameNode() const;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SEARCH_SEARCH_MODEL_NG_H
