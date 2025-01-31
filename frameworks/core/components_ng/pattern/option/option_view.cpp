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
#include "core/components_ng/pattern/option/option_view.h"

#include "base/geometry/dimension.h"
#include "base/i18n/localization.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components/select/select_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/image/image_model_ng.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/option/option_event_hub.h"
#include "core/components_ng/pattern/option/option_pattern.h"
#include "core/components_ng/pattern/security_component/paste_button/paste_button_common.h"
#include "core/components_ng/pattern/security_component/paste_button/paste_button_model_ng.h"
#include "core/components_ng/pattern/security_component/security_component_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/image/image_source_info.h"

namespace OHOS::Ace::NG {

namespace {

RefPtr<FrameNode> Create(int32_t index)
{
    auto Id = ElementRegister::GetInstance()->MakeUniqueId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::OPTION_ETS_TAG, Id);
    auto node = FrameNode::CreateFrameNode(V2::OPTION_ETS_TAG, Id, AceType::MakeRefPtr<OptionPattern>(index));

    // set border radius
    auto renderContext = node->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, nullptr);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_RETURN(theme, nullptr);
    BorderRadiusProperty border;
    border.SetRadius(theme->GetInnerBorderRadius());
    renderContext->UpdateBorderRadius(border);

    auto props = node->GetPaintProperty<OptionPaintProperty>();
    CHECK_NULL_RETURN(props, nullptr);
    props->UpdateHover(false);
    props->UpdatePress(false);
    return node;
}
} // namespace

RefPtr<FrameNode> OptionView::CreateText(const std::string& value, const RefPtr<FrameNode>& parent)
{
    // create child text node
    auto textId = ElementRegister::GetInstance()->MakeUniqueId();
    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, textId, AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textNode, nullptr);

    auto textProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textProperty, nullptr);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_RETURN(theme, nullptr);

    textProperty->UpdateMaxLines(1);
    textProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    textProperty->UpdateFontSize(theme->GetMenuFontSize());
    textProperty->UpdateFontWeight(FontWeight::REGULAR);
    textProperty->UpdateTextColor(theme->GetMenuFontColor());
    // set default foregroundColor
    auto textRenderContext = textNode->GetRenderContext();
    textRenderContext->UpdateForegroundColor(theme->GetMenuFontColor());
    textProperty->UpdateContent(value);
    textNode->MountToParent(parent);
    textNode->MarkModifyDone();

    return textNode;
}

RefPtr<FrameNode> OptionView::CreateIcon(const std::string& icon, const RefPtr<FrameNode>& parent,
    const RefPtr<FrameNode>& child)
{
    auto iconNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    CHECK_NULL_RETURN(iconNode, nullptr);
    auto props = iconNode->GetLayoutProperty<ImageLayoutProperty>();
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_RETURN(theme, nullptr);
    if (!icon.empty()) {
        ImageSourceInfo info(icon);
        props->UpdateImageSourceInfo(info);
    }
    props->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(theme->GetIconSideLength()), CalcLength(theme->GetIconSideLength())));
    props->UpdateAlignment(Alignment::CENTER_LEFT);

    if (child) {
        parent->ReplaceChild(child, iconNode);
    } else {
        iconNode->MountToParent(parent, 0);
    }
    iconNode->MarkModifyDone();
    return iconNode;
}

RefPtr<FrameNode> OptionView::CreateSymbol(const std::function<void(WeakPtr<NG::FrameNode>)>& symbolApply,
    const RefPtr<FrameNode>& parent, const RefPtr<FrameNode>& child)
{
    auto iconNode = FrameNode::GetOrCreateFrameNode(V2::SYMBOL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<TextPattern>(); });
    CHECK_NULL_RETURN(iconNode, nullptr);
    auto props = iconNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(props, nullptr);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_RETURN(theme, nullptr);
    props->UpdateFontSize(theme->GetEndIconWidth());
    props->UpdateSymbolColorList({theme->GetMenuIconColor()});
    props->UpdateAlignment(Alignment::CENTER_LEFT);
    MarginProperty margin;
    margin.right = CalcLength(theme->GetIconContentPadding());
    props->UpdateMargin(margin);
    if (symbolApply != nullptr) {
        symbolApply(AccessibilityManager::WeakClaim(AccessibilityManager::RawPtr(iconNode)));
    }
    if (child) {
        parent->ReplaceChild(child, iconNode);
    } else {
        iconNode->MountToParent(parent, 0);
    }
    iconNode->MarkModifyDone();
    return iconNode;
}


void OptionView::CreatePasteButton(const RefPtr<FrameNode>& option, const RefPtr<FrameNode>& row,
    const std::function<void()>& onClickFunc)
{
    auto pasteNode =
        PasteButtonModelNG::GetInstance()->CreateNode(static_cast<int32_t>(PasteButtonPasteDescription::PASTE),
            static_cast<int32_t>(PasteButtonIconStyle::ICON_NULL), static_cast<int32_t>(ButtonType::NORMAL), true);
    CHECK_NULL_VOID(pasteNode);
    auto pattern = option->GetPattern<OptionPattern>();
    CHECK_NULL_VOID(pattern);

    auto pasteLayoutProperty = pasteNode->GetLayoutProperty<SecurityComponentLayoutProperty>();
    CHECK_NULL_VOID(pasteLayoutProperty);
    auto pastePaintProperty = pasteNode->GetPaintProperty<SecurityComponentPaintProperty>();
    CHECK_NULL_VOID(pastePaintProperty);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);

    pasteLayoutProperty->UpdateFontSize(theme->GetMenuFontSize());
    pasteLayoutProperty->UpdateFontWeight(FontWeight::REGULAR);
    pastePaintProperty->UpdateFontColor(theme->GetMenuFontColor());
    pastePaintProperty->UpdateBackgroundColor(Color::TRANSPARENT);
    pasteLayoutProperty->UpdateBackgroundBorderRadius(theme->GetInnerBorderRadius());
    pasteNode->MountToParent(row);
    pasteNode->MarkModifyDone();

    row->MountToParent(option);
    row->MarkModifyDone();
    auto eventHub = option->GetEventHub<OptionEventHub>();
    CHECK_NULL_VOID(eventHub);
    pasteNode->GetOrCreateGestureEventHub()->SetUserOnClick([onClickFunc](GestureEvent& info) {
        if (!PasteButtonModelNG::GetInstance()->IsClickResultSuccess(info)) {
            return;
        }
        if (onClickFunc) {
            onClickFunc();
        }
    });
    pattern->SetPasteButton(pasteNode);
}

void OptionView::CreateOption(bool optionsHasIcon, const std::string& value,
    const std::function<void(WeakPtr<NG::FrameNode>)>& symbol, const RefPtr<FrameNode>& row,
    const RefPtr<FrameNode>& option, const std::function<void()>& onClickFunc)
{
    auto pattern = option->GetPattern<OptionPattern>();
    CHECK_NULL_VOID(pattern);
    if (optionsHasIcon) {
        auto iconNode = CreateSymbol(symbol, row);
        pattern->SetIconNode(iconNode);
    }
    auto textNode = CreateText(value, row);
    row->MountToParent(option);
    row->MarkModifyDone();
    pattern->SetTextNode(textNode);

    auto eventHub = option->GetEventHub<OptionEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetMenuOnClick(onClickFunc);
}

void OptionView::CreateOption(bool optionsHasIcon, const std::string& value, const std::string& icon,
    const RefPtr<FrameNode>& row, const RefPtr<FrameNode>& option, const std::function<void()>& onClickFunc)
{
    auto pattern = option->GetPattern<OptionPattern>();
    CHECK_NULL_VOID(pattern);
    if (optionsHasIcon) {
        auto iconNode = CreateIcon(icon, row);
        pattern->SetIconNode(iconNode);
        pattern->SetIcon(icon);
    }
    auto textNode = CreateText(value, row);
    row->MountToParent(option);
    row->MarkModifyDone();
    pattern->SetTextNode(textNode);

    auto eventHub = option->GetEventHub<OptionEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetMenuOnClick(onClickFunc);
}

RefPtr<FrameNode> OptionView::CreateMenuOption(bool optionsHasIcon, const std::string& value,
    const std::function<void()>& onClickFunc, int32_t index, const std::function<void(WeakPtr<NG::FrameNode>)>& symbol)
{
    auto option = Create(index);
    CHECK_NULL_RETURN(option, nullptr);
    auto row = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));

#ifdef OHOS_PLATFORM
    constexpr char BUTTON_PASTE[] = "textoverlay.paste";
    if (value == Localization::GetInstance()->GetEntryLetters(BUTTON_PASTE)) {
        CreatePasteButton(option, row, onClickFunc);
    } else {
        CreateOption(optionsHasIcon, value, symbol, row, option, onClickFunc);
    }
#else
    CreateOption(optionsHasIcon, value, symbol, row, option, onClickFunc);
#endif
    return option;
}

RefPtr<FrameNode> OptionView::CreateMenuOption(bool optionsHasIcon, const std::string& value,
    const std::function<void()>& onClickFunc, int32_t index, const std::string& icon)
{
    auto option = Create(index);
    CHECK_NULL_RETURN(option, nullptr);
    auto row = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));

#ifdef OHOS_PLATFORM
    constexpr char BUTTON_PASTE[] = "textoverlay.paste";
    if (value == Localization::GetInstance()->GetEntryLetters(BUTTON_PASTE)) {
        CreatePasteButton(option, row, onClickFunc);
    } else {
        CreateOption(optionsHasIcon, value, icon, row, option, onClickFunc);
    }
#else
    CreateOption(optionsHasIcon, value, icon, row, option, onClickFunc);
#endif
    return option;
}

RefPtr<FrameNode> OptionView::CreateSelectOption(const SelectParam& param, int32_t index)
{
    LOGI("create option value = %s", param.text.c_str());
    auto option = Create(index);
    auto row = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    row->MountToParent(option);

    auto pattern = option->GetPattern<OptionPattern>();
    CHECK_NULL_RETURN(pattern, option);
    // create icon node
    RefPtr<FrameNode> iconNode;
    if (param.symbolIcon != nullptr) {
        iconNode = CreateSymbol(param.symbolIcon, row);
    } else if (!param.icon.empty()) {
        iconNode = CreateIcon(param.icon, row);
        pattern->SetIcon(param.icon);
    }
    pattern->SetIconNode(iconNode);

    auto text = CreateText(param.text, row);
    pattern->SetTextNode(text);
    return option;
}

} // namespace OHOS::Ace::NG
