/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/select_overlay/select_overlay_node.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <securec.h>

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/i18n/localization.h"
#include "base/utils/utils.h"
#include "core/animation/curves.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/placement.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/text_overlay/text_overlay_theme.h"
#include "core/components/theme/shadow_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/menu/menu_layout_property.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/security_component/paste_button/paste_button_common.h"
#include "core/components_ng/pattern/security_component/paste_button/paste_button_model_ng.h"
#include "core/components_ng/pattern/security_component/security_component_pattern.h"
#include "core/components_ng/pattern/select_content_overlay/select_content_overlay_pattern.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_ng/property/menu_property.h"
#include "core/components_ng/property/property.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/pipeline_context.h"
#ifdef ENABLE_ROSEN_BACKEND
#include "core/components/custom_paint/rosen_render_custom_paint.h"
#endif

namespace OHOS::Ace::NG {
namespace {
constexpr char BUTTON_COPY_ALL[] = "textoverlay.select_all";
constexpr char BUTTON_CUT[] = "textoverlay.cut";
constexpr char BUTTON_COPY[] = "textoverlay.copy";
constexpr char BUTTON_PASTE[] = "textoverlay.paste";
constexpr char BUTTON_SHARE[] = "textoverlay.share";
constexpr char BUTTON_TRANSLATE[] = "textoverlay.translate";
constexpr char BUTTON_SEARCH[] = "textoverlay.search";

constexpr int32_t OPTION_INDEX_CUT = 0;
constexpr int32_t OPTION_INDEX_COPY = 1;
constexpr int32_t OPTION_INDEX_PASTE = 2;
constexpr int32_t OPTION_INDEX_COPY_ALL = 3;
constexpr int32_t OPTION_INDEX_SHARE = 4;
constexpr int32_t OPTION_INDEX_TRANSLATE = 5;
constexpr int32_t OPTION_INDEX_SEARCH = 6;
constexpr int32_t OPTION_INDEX_CAMERA_INPUT = 7;
constexpr int32_t ANIMATION_DURATION1 = 350;
constexpr int32_t ANIMATION_DURATION2 = 150;

constexpr Dimension MORE_MENU_TRANSLATE = -7.5_vp;
constexpr Dimension MAX_DIAMETER = 3.5_vp;
constexpr Dimension MIN_DIAMETER = 1.5_vp;
constexpr Dimension MIN_ARROWHEAD_DIAMETER = 2.0_vp;
constexpr Dimension ANIMATION_TEXT_OFFSET = 12.0_vp;
constexpr float AGING_MIN_SCALE = 1.75f;

void SetResponseRegion(RefPtr<FrameNode>& node)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto textOverlayTheme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_VOID(textOverlayTheme);
    auto gestureHub = node->GetOrCreateGestureEventHub();
    std::vector<DimensionRect> vector;
    auto menuPadding = textOverlayTheme->GetMenuPadding();
    auto buttonHeight = textOverlayTheme->GetMenuButtonHeight();
    auto top = menuPadding.Top();
    auto responseHeight = top.Value() + menuPadding.Bottom().Value() + buttonHeight.Value();
    vector.emplace_back(
        DimensionRect(Dimension(1, DimensionUnit::PERCENT), Dimension(responseHeight, DimensionUnit::VP),
            DimensionOffset(Dimension(0), Dimension(-top.Value(), top.Unit()))));
    gestureHub->SetResponseRegion(vector);
}

float MeasureTextWidth(const TextStyle& textStyle, const std::string& text)
{
#ifdef ENABLE_ROSEN_BACKEND
    MeasureContext content;
    content.textContent = text;
    content.fontSize = textStyle.GetFontSize();
    auto fontweight = StringUtils::FontWeightToString(textStyle.GetFontWeight());
    content.fontWeight = fontweight;
    return static_cast<float>(RosenRenderCustomPaint::MeasureTextSizeInner(content).Width());
#else
    return 0.0f;
#endif
}

#ifdef OHOS_PLATFORM
RefPtr<FrameNode> BuildPasteButton(const std::function<void()>& callback, int32_t overlayId,
    float& buttonWidth, bool isSelectAll = false)
{
    auto descriptionId = static_cast<int32_t>(PasteButtonPasteDescription::PASTE);
    auto pasteButton = PasteButtonModelNG::GetInstance()->CreateNode(
        descriptionId,
        static_cast<int32_t>(PasteButtonIconStyle::ICON_NULL),
        static_cast<int32_t>(ButtonType::CAPSULE), true);
    CHECK_NULL_RETURN(pasteButton, nullptr);

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, pasteButton);
    auto textOverlayTheme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_RETURN(textOverlayTheme, pasteButton);
    auto textStyle = textOverlayTheme->GetMenuButtonTextStyle();

    auto buttonLayoutProperty = pasteButton->GetLayoutProperty<SecurityComponentLayoutProperty>();
    buttonLayoutProperty->UpdateFontSize(textStyle.GetFontSize());
    buttonLayoutProperty->UpdateFontWeight(textStyle.GetFontWeight());

    auto buttonPaintProperty = pasteButton->GetPaintProperty<SecurityComponentPaintProperty>();
    if (callback) {
        buttonPaintProperty->UpdateFontColor(textStyle.GetTextColor());
    } else {
        buttonPaintProperty->UpdateFontColor(
            textStyle.GetTextColor().BlendOpacity(textOverlayTheme->GetAlphaDisabled()));
    }
    const auto& padding = textOverlayTheme->GetMenuButtonPadding();
    buttonLayoutProperty->UpdateBackgroundLeftPadding(padding.Left());
    buttonLayoutProperty->UpdateBackgroundRightPadding(padding.Right());
    std::string buttonContent;
    PasteButtonModelNG::GetInstance()->GetTextResource(descriptionId, buttonContent);
    buttonWidth = MeasureTextWidth(textStyle, buttonContent);
    buttonWidth = buttonWidth + padding.Left().ConvertToPx() + padding.Right().ConvertToPx();
    buttonLayoutProperty->UpdateUserDefinedIdealSize(
        { CalcLength(buttonWidth), std::optional<CalcLength>(textOverlayTheme->GetMenuButtonHeight()) });
    buttonPaintProperty->UpdateBackgroundColor(Color::TRANSPARENT);
    if (callback) {
        pasteButton->GetOrCreateGestureEventHub()->SetUserOnClick([callback](GestureEvent& info) {
            if (!PasteButtonModelNG::GetInstance()->IsClickResultSuccess(info)) {
                return;
            }
            if (callback) {
                callback();
            }
        });
    } else {
        auto buttonEventHub = pasteButton->GetEventHub<OptionEventHub>();
        CHECK_NULL_RETURN(buttonEventHub, pasteButton);
        buttonEventHub->SetEnabled(false);
    }
    SetResponseRegion(pasteButton);
    pasteButton->MarkModifyDone();
    return pasteButton;
}
#endif

RefPtr<FrameNode> BuildButton(const std::string& data, const std::function<void()>& callback, int32_t overlayId,
    float& buttonWidth, bool isSelectAll = false)
{
    auto button = FrameNode::GetOrCreateFrameNode("SelectMenuButton", ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    auto text = FrameNode::GetOrCreateFrameNode("SelectMenuButtonText", ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<TextPattern>(); });
    auto textLayoutProperty = text->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, button);
    textLayoutProperty->UpdateContent(data);
    text->MountToParent(button);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, button);
    auto textOverlayTheme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_RETURN(textOverlayTheme, button);
    auto textStyle = textOverlayTheme->GetMenuButtonTextStyle();
    textLayoutProperty->UpdateFontSize(textStyle.GetFontSize());
    textLayoutProperty->UpdateFontWeight(textStyle.GetFontWeight());
    textLayoutProperty->UpdateMaxLines(1);
    if (callback) {
        textLayoutProperty->UpdateTextColor(textStyle.GetTextColor());
    } else {
        textLayoutProperty->UpdateTextColor(
            textStyle.GetTextColor().BlendOpacity(textOverlayTheme->GetAlphaDisabled()));
    }
    text->MarkModifyDone();

    auto buttonLayoutProperty = button->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_RETURN(buttonLayoutProperty, button);
    const auto& padding = textOverlayTheme->GetMenuButtonPadding();
    auto left = CalcLength(padding.Left().ConvertToPx());
    auto right = CalcLength(padding.Right().ConvertToPx());
    auto top = CalcLength(padding.Top().ConvertToPx());
    auto bottom = CalcLength(padding.Bottom().ConvertToPx());
    buttonLayoutProperty->UpdatePadding({ left, right, top, bottom });
    buttonWidth = MeasureTextWidth(textStyle, data);
    // Calculate the width of default option include button padding.
    buttonWidth = buttonWidth + padding.Left().ConvertToPx() + padding.Right().ConvertToPx();
    if (GreatOrEqual(pipeline->GetFontScale(), AGING_MIN_SCALE)) {
        buttonLayoutProperty->UpdateUserDefinedIdealSize({ CalcLength(buttonWidth), std::nullopt });
    } else {
        buttonLayoutProperty->UpdateUserDefinedIdealSize(
            { CalcLength(buttonWidth), CalcLength(textOverlayTheme->GetMenuButtonHeight()) });
    }
    buttonLayoutProperty->UpdateFlexShrink(0);
    button->GetRenderContext()->UpdateBackgroundColor(Color::TRANSPARENT);

    if (callback) {
        button->GetOrCreateGestureEventHub()->SetUserOnClick(
            [callback, overlayId, isSelectAll](GestureEvent& /*info*/) {
                auto pipeline = PipelineContext::GetCurrentContext();
                CHECK_NULL_VOID(pipeline);
                auto overlayManager = pipeline->GetSelectOverlayManager();
                CHECK_NULL_VOID(overlayManager);
                auto selectOverlay = overlayManager->GetSelectOverlayNode(overlayId);
                CHECK_NULL_VOID(selectOverlay);
                auto isDoingAnimation = selectOverlay->GetAnimationStatus();
                CHECK_NULL_VOID(!isDoingAnimation);
                auto isExtensionMenu = selectOverlay->GetIsExtensionMenu();
                CHECK_NULL_VOID(!isExtensionMenu);
                if (callback) {
                    callback();
                }
            });
    } else {
        auto buttonEventHub = button->GetEventHub<OptionEventHub>();
        CHECK_NULL_RETURN(buttonEventHub, button);
        buttonEventHub->SetEnabled(false);
    }
    SetResponseRegion(button);
    button->MarkModifyDone();
    return button;
}

void BindButtonClickEvent(const RefPtr<FrameNode>& button, const MenuOptionsParam& menuOption, int32_t overlayId)
{
    auto callback = menuOption.action;
    auto actionRange = menuOption.actionRange;
    button->GetOrCreateGestureEventHub()->SetUserOnClick([callback, actionRange, overlayId](GestureEvent& /*info*/) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto overlayManager = pipeline->GetSelectOverlayManager();
        CHECK_NULL_VOID(overlayManager);

        auto selectOverlay = overlayManager->GetSelectOverlayNode(overlayId);
        CHECK_NULL_VOID(selectOverlay);
        auto pattern = selectOverlay->GetPattern<SelectOverlayPattern>();
        auto selectInfo = pattern->GetSelectInfo();
        if (callback) {
            callback(selectInfo);
        }
        // close text overlay.
        if (!actionRange) {
            overlayManager->DestroySelectOverlay(overlayId);
            overlayManager->CloseSelectContentOverlay(overlayId, CloseReason::CLOSE_REASON_TOOL_BAR, false);
        }
    });
}

RefPtr<FrameNode> BuildButton(const MenuOptionsParam& menuOption, int32_t overlayId, float& contentWidth)
{
    auto button = FrameNode::GetOrCreateFrameNode("SelectMenuButton", ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    auto text = FrameNode::GetOrCreateFrameNode("SelectMenuButtonText", ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<TextPattern>(); });

    // Update text property and mount to button.
    auto textLayoutProperty = text->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, button);
    auto data = menuOption.content.value_or("");
    textLayoutProperty->UpdateContent(data);
    text->MountToParent(button);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, button);
    auto textOverlayTheme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_RETURN(textOverlayTheme, button);
    auto textStyle = textOverlayTheme->GetMenuButtonTextStyle();
    textLayoutProperty->UpdateFontSize(textStyle.GetFontSize());
    textLayoutProperty->UpdateTextColor(textStyle.GetTextColor());
    textLayoutProperty->UpdateFontWeight(textStyle.GetFontWeight());
    text->MarkModifyDone();
    // Calculate the width of entension option include button padding.
    contentWidth = MeasureTextWidth(textStyle, data);
    const auto& padding = textOverlayTheme->GetMenuButtonPadding();
    auto left = CalcLength(padding.Left().ConvertToPx());
    auto right = CalcLength(padding.Right().ConvertToPx());
    auto top = CalcLength(padding.Top().ConvertToPx());
    auto bottom = CalcLength(padding.Bottom().ConvertToPx());
    contentWidth = contentWidth + padding.Left().ConvertToPx() + padding.Right().ConvertToPx();

    // Update button property.
    auto buttonLayoutProperty = button->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_RETURN(buttonLayoutProperty, button);
    buttonLayoutProperty->UpdatePadding({ left, right, top, bottom });
    if (GreatOrEqual(pipeline->GetFontScale(), AGING_MIN_SCALE)) {
        buttonLayoutProperty->UpdateUserDefinedIdealSize({ CalcLength(contentWidth), std::nullopt });
    } else {
        buttonLayoutProperty->UpdateUserDefinedIdealSize(
            { CalcLength(contentWidth), CalcLength(textOverlayTheme->GetMenuButtonHeight()) });
    }
    buttonLayoutProperty->UpdateFlexShrink(0);
    button->GetRenderContext()->UpdateBackgroundColor(Color::TRANSPARENT);
    BindButtonClickEvent(button, menuOption, overlayId);
    SetResponseRegion(button);
    button->MarkModifyDone();
    return button;
}

RefPtr<FrameNode> BuildMoreOrBackButton(int32_t overlayId, bool isMoreButton)
{
    auto button = FrameNode::GetOrCreateFrameNode("SelectMoreOrBackButton",
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, button);
    auto textOverlayTheme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_RETURN(textOverlayTheme, button);

    // Update property.
    auto buttonLayoutProperty = button->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_RETURN(buttonLayoutProperty, button);

    const auto& padding = textOverlayTheme->GetMenuPadding();

    if (isMoreButton) {
        auto sideWidth = CalcLength(textOverlayTheme->GetMenuToolbarHeight().ConvertToPx() -
                                    padding.Top().ConvertToPx() - padding.Bottom().ConvertToPx());
        buttonLayoutProperty->UpdateUserDefinedIdealSize({ sideWidth, sideWidth });
    } else {
        auto sideWidth = CalcLength(textOverlayTheme->GetMenuToolbarHeight().ConvertToPx());
        buttonLayoutProperty->UpdateUserDefinedIdealSize({ sideWidth, sideWidth });
        auto left = CalcLength(padding.Left().ConvertToPx());
        auto right = CalcLength(padding.Right().ConvertToPx());
        auto top = CalcLength(padding.Top().ConvertToPx());
        auto bottom = CalcLength(padding.Bottom().ConvertToPx());
        if (GreatOrEqual(pipeline->GetFontScale(), AGING_MIN_SCALE)) {
            auto overlayManager = pipeline->GetSelectOverlayManager();
            CHECK_NULL_RETURN(overlayManager, button);
            auto selectOverlay = overlayManager->GetSelectOverlayNode(overlayId);
            if (selectOverlay) {
                auto selectMenu = AceType::DynamicCast<FrameNode>(selectOverlay->GetFirstChild());
                CHECK_NULL_RETURN(selectMenu, button);
                auto geometryNode = selectMenu->GetGeometryNode();
                CHECK_NULL_RETURN(geometryNode, button);
                auto selectMenuHeight = geometryNode->GetFrameSize().Height();
                top = CalcLength((selectMenuHeight - sideWidth.GetDimension().Value()) / 2.0f);
            }
        }
        buttonLayoutProperty->UpdatePadding({ left, right, top, bottom });
        button->GetOrCreateGestureEventHub()->SetUserOnClick([overlayId](GestureEvent& /*info*/) {
            auto pipeline = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto overlayManager = pipeline->GetSelectOverlayManager();
            CHECK_NULL_VOID(overlayManager);
            auto selectOverlay = overlayManager->GetSelectOverlayNode(overlayId);
            CHECK_NULL_VOID(selectOverlay);
            // When click button , change to extensionMenu or change to the default menu(selectMenu_).
            auto isMore = !selectOverlay->GetIsExtensionMenu();
            selectOverlay->MoreOrBackAnimation(isMore);
        });
    }

    button->GetRenderContext()->UpdateBackgroundColor(Color::TRANSPARENT);
    button->MarkModifyDone();
    return button;
}

OffsetF GetPageOffset()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, OffsetF());
    auto stageManager = pipeline->GetStageManager();
    CHECK_NULL_RETURN(stageManager, OffsetF());
    auto page = stageManager->GetLastPage();
    CHECK_NULL_RETURN(page, OffsetF());
    return page->GetOffsetRelativeToWindow();
}

std::vector<OptionParam> GetOptionsParams(const std::shared_ptr<SelectOverlayInfo>& info)
{
    std::vector<OptionParam> params;
    params.emplace_back(Localization::GetInstance()->GetEntryLetters(BUTTON_CUT), info->menuCallback.onCut);
    params.emplace_back(Localization::GetInstance()->GetEntryLetters(BUTTON_COPY), info->menuCallback.onCopy);
    params.emplace_back(Localization::GetInstance()->GetEntryLetters(BUTTON_PASTE), info->menuCallback.onPaste);
    params.emplace_back(Localization::GetInstance()->GetEntryLetters(BUTTON_COPY_ALL), info->menuCallback.onSelectAll);
    return params;
}

void SetOptionDisable(const RefPtr<FrameNode>& option)
{
    CHECK_NULL_VOID(option);
    auto optionEventHub = option->GetEventHub<OptionEventHub>();
    CHECK_NULL_VOID(optionEventHub);
    optionEventHub->SetEnabled(false);
    option->MarkModifyDone();
}

void SetOptionsAction(const std::shared_ptr<SelectOverlayInfo>& info, const std::vector<RefPtr<FrameNode>>& options)
{
    if (options.empty()) {
        return;
    }
    if (!info->menuInfo.showCut) {
        SetOptionDisable(options[OPTION_INDEX_CUT]);
    }
    if (!info->menuInfo.showCopy) {
        SetOptionDisable(options[OPTION_INDEX_COPY]);
    }
    if (!info->menuInfo.showPaste) {
        SetOptionDisable(options[OPTION_INDEX_PASTE]);
    }
    if (!info->menuInfo.showCopyAll) {
        SetOptionDisable(options[OPTION_INDEX_COPY_ALL]);
    }
}

} // namespace

SelectOverlayNode::SelectOverlayNode(const RefPtr<Pattern>& pattern)
    : FrameNode(V2::SELECT_OVERLAY_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), pattern)
{
    stateFuncs_[FrameNodeStatus::VISIBLE] = &SelectOverlayNode::DispatchVisibleState;
    stateFuncs_[FrameNodeStatus::VISIBLETOGONE] = &SelectOverlayNode::DispatchVisibleToGoneState;
    stateFuncs_[FrameNodeStatus::GONE] = &SelectOverlayNode::DispatchGoneState;
    stateFuncs_[FrameNodeStatus::GONETOVISIBLE] = &SelectOverlayNode::DispatchGoneToVisibleState;
}

void SelectOverlayNode::DispatchVisibleState(FrameNodeType type, FrameNodeTrigger trigger)
{
    AnimationOption option;
    option.SetDuration(MENU_HIDE_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);

    switch (trigger) {
        case FrameNodeTrigger::HIDE:
            SetFrameNodeStatus(type, FrameNodeStatus::VISIBLETOGONE);
            AnimationUtils::Animate(
                option,
                [weak = WeakClaim(this), type, id = Container::CurrentId()]() {
                    ContainerScope scope(id);
                    auto node = weak.Upgrade();
                    CHECK_NULL_VOID(node);
                    node->SetFrameNodeOpacity(type, 0.0);
                },
                [weak = WeakClaim(this), type, id = Container::CurrentId()]() {
                    ContainerScope scope(id);
                    auto node = weak.Upgrade();
                    CHECK_NULL_VOID(node);
                    node->ExecuteOverlayStatus(type, FrameNodeTrigger::HIDDEN);
                });
            break;
        case FrameNodeTrigger::SHOW:
        case FrameNodeTrigger::SHOWN:
        case FrameNodeTrigger::HIDDEN:
        default:
            break;
    }
}

void SelectOverlayNode::DispatchVisibleToGoneState(FrameNodeType type, FrameNodeTrigger trigger)
{
    AnimationOption option;
    option.SetDuration(MENU_SHOW_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);

    switch (trigger) {
        case FrameNodeTrigger::SHOW:
            SetFrameNodeStatus(type, FrameNodeStatus::GONETOVISIBLE);
            SetFrameNodeVisibility(type, VisibleType::VISIBLE);
            AnimationUtils::Animate(
                option,
                [weak = WeakClaim(this), type, id = Container::CurrentId()]() {
                    ContainerScope scope(id);
                    auto node = weak.Upgrade();
                    CHECK_NULL_VOID(node);
                    node->SetFrameNodeOpacity(type, 1.0);
                },
                [weak = WeakClaim(this), type, id = Container::CurrentId()]() {
                    ContainerScope scope(id);
                    auto node = weak.Upgrade();
                    CHECK_NULL_VOID(node);
                    node->ExecuteOverlayStatus(type, FrameNodeTrigger::SHOWN);
                });
            break;
        case FrameNodeTrigger::HIDDEN:
            SetFrameNodeStatus(type, FrameNodeStatus::GONE);
            SetFrameNodeVisibility(type, VisibleType::GONE);
            break;
        case FrameNodeTrigger::SHOWN:
        case FrameNodeTrigger::HIDE:
        default:
            break;
    }
}

void SelectOverlayNode::DispatchGoneState(FrameNodeType type, FrameNodeTrigger trigger)
{
    AnimationOption option;
    option.SetDuration(MENU_SHOW_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);

    switch (trigger) {
        case FrameNodeTrigger::SHOW:
            SetFrameNodeStatus(type, FrameNodeStatus::GONETOVISIBLE);
            SetFrameNodeVisibility(type, VisibleType::VISIBLE);
            AnimationUtils::Animate(
                option,
                [weak = WeakClaim(this), type, id = Container::CurrentId()]() {
                    ContainerScope scope(id);
                    auto node = weak.Upgrade();
                    CHECK_NULL_VOID(node);
                    node->SetFrameNodeOpacity(type, 1.0);
                },
                [weak = WeakClaim(this), type, id = Container::CurrentId()]() {
                    ContainerScope scope(id);
                    auto node = weak.Upgrade();
                    CHECK_NULL_VOID(node);
                    node->ExecuteOverlayStatus(type, FrameNodeTrigger::SHOWN);
                });
            break;
        case FrameNodeTrigger::SHOWN:
        case FrameNodeTrigger::HIDE:
        case FrameNodeTrigger::HIDDEN:
        default:
            break;
    }
}

void SelectOverlayNode::DispatchGoneToVisibleState(FrameNodeType type, FrameNodeTrigger trigger)
{
    AnimationOption option;
    option.SetDuration(MENU_HIDE_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);

    switch (trigger) {
        case FrameNodeTrigger::SHOWN:
            SetFrameNodeStatus(type, FrameNodeStatus::VISIBLE);
            break;
        case FrameNodeTrigger::HIDE:
            SetFrameNodeStatus(type, FrameNodeStatus::VISIBLETOGONE);
            AnimationUtils::Animate(
                option,
                [weak = WeakClaim(this), type, id = Container::CurrentId()]() {
                    ContainerScope scope(id);
                    auto node = weak.Upgrade();
                    CHECK_NULL_VOID(node);
                    node->SetFrameNodeOpacity(type, 0.0);
                },
                [weak = WeakClaim(this), type, id = Container::CurrentId()]() {
                    ContainerScope scope(id);
                    auto node = weak.Upgrade();
                    CHECK_NULL_VOID(node);
                    node->ExecuteOverlayStatus(type, FrameNodeTrigger::HIDDEN);
                });
            break;
        case FrameNodeTrigger::SHOW:
        case FrameNodeTrigger::HIDDEN:
            break;
        default:
            break;
    }
}

RefPtr<UINode> CreateCustomSelectMenu(const std::shared_ptr<SelectOverlayInfo>& info)
{
    CHECK_NULL_RETURN(info, nullptr);
    CHECK_NULL_RETURN(info->menuInfo.menuBuilder, nullptr);
    NG::ScopedViewStackProcessor builderViewStackProcessor;
    info->menuInfo.menuBuilder();
    auto customNode = NG::ViewStackProcessor::GetInstance()->Finish();
    CHECK_NULL_RETURN(customNode, nullptr);
    return customNode;
}

RefPtr<FrameNode> SelectOverlayNode::CreateSelectOverlayNode(const std::shared_ptr<SelectOverlayInfo>& info)
{
    if (info->isUsingMouse && !info->menuInfo.menuBuilder) {
        return CreateMenuNode(info);
    }
    RefPtr<Pattern> selectOverlayPattern;
    if (info->isUseOverlayNG) {
        selectOverlayPattern = AceType::MakeRefPtr<SelectContentOverlayPattern>(info);
    } else {
        selectOverlayPattern = AceType::MakeRefPtr<SelectOverlayPattern>(info);
    }
    auto selectOverlayNode = AceType::MakeRefPtr<SelectOverlayNode>(selectOverlayPattern);
    selectOverlayNode->InitializePatternAndContext();
    ElementRegister::GetInstance()->AddUINode(selectOverlayNode);
    selectOverlayNode->CreateToolBar();
    selectOverlayNode->UpdateToolBar(true);
    auto selectContext = selectOverlayNode->GetRenderContext();
    selectContext->UpdateUseShadowBatching(true);

    auto accessibilityProperty = selectOverlayNode->GetAccessibilityProperty<AccessibilityProperty>();
    if (accessibilityProperty) {
        accessibilityProperty->SetAccessibilityLevel("no");
    }
    return selectOverlayNode;
}

void SelectOverlayNode::CreateCustomSelectOverlay(const std::shared_ptr<SelectOverlayInfo>& info)
{
    selectMenu_ = FrameNode::GetOrCreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), [id = GetId()]() {
            return AceType::MakeRefPtr<MenuPattern>(id, V2::MENU_ETS_TAG, MenuType::SELECT_OVERLAY_CUSTOM_MENU);
        });
    selectMenu_->MountToParent(Claim(this));
    auto eventHub = selectMenu_->GetEventHub<EventHub>();
    if (eventHub && info->menuCallback.onAppear) {
        eventHub->SetOnAppear(std::move(info->menuCallback.onAppear));
    }
    if (eventHub && info->menuCallback.onDisappear) {
        eventHub->SetOnDisappear(std::move(info->menuCallback.onDisappear));
    }
    auto pattern = selectMenu_->GetPattern<MenuPattern>();
    auto renderContext = selectMenu_->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateClipEdge(false);
    renderContext->UpdateBackgroundColor(Color::TRANSPARENT);
    renderContext->UpdateBackShadow(ShadowConfig::NoneShadow);
    auto layoutProperty = selectMenu_->GetLayoutProperty<MenuLayoutProperty>();
    auto customMenu = CreateCustomSelectMenu(info);
    CHECK_NULL_VOID(selectMenu_);
    CHECK_NULL_VOID(customMenu);
    customMenu->MountToParent(selectMenu_);
    if (info->menuInfo.menuIsShow) {
        selectMenu_->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
        selectMenuStatus_ = FrameNodeStatus::VISIBLE;
    } else {
        selectMenu_->GetLayoutProperty()->UpdateVisibility(VisibleType::GONE);
        selectMenuStatus_ = FrameNodeStatus::GONE;
    }
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto shadowTheme = pipeline->GetTheme<ShadowTheme>();
    CHECK_NULL_VOID(shadowTheme);
    auto colorMode = SystemProperties::GetColorMode();
    selectMenu_->GetRenderContext()->UpdateBackShadow(shadowTheme->GetShadow(ShadowStyle::OuterDefaultMD, colorMode));
    selectMenu_->MarkModifyDone();
}

void SelectOverlayNode::MoreOrBackAnimation(bool isMore)
{
    CHECK_NULL_VOID(!isDoingAnimation_);
    CHECK_NULL_VOID(selectMenu_);
    CHECK_NULL_VOID(selectMenuInner_);
    CHECK_NULL_VOID(extensionMenu_);
    CHECK_NULL_VOID(backButton_);
    if (isMore && !isExtensionMenu_) {
        MoreAnimation();
    } else if (!isMore && isExtensionMenu_) {
        BackAnimation();
    }
}

void SelectOverlayNode::MoreAnimation()
{
    auto extensionContext = extensionMenu_->GetRenderContext();
    CHECK_NULL_VOID(extensionContext);
    auto selectMenuInnerContext = selectMenuInner_->GetRenderContext();
    CHECK_NULL_VOID(selectMenuInnerContext);

    auto extensionProperty = extensionMenu_->GetLayoutProperty();
    CHECK_NULL_VOID(extensionProperty);
    auto selectProperty = selectMenu_->GetLayoutProperty();
    CHECK_NULL_VOID(selectProperty);
    auto selectMenuInnerProperty = selectMenuInner_->GetLayoutProperty();
    CHECK_NULL_VOID(selectMenuInnerProperty);
    auto backButtonProperty = backButton_->GetLayoutProperty();
    CHECK_NULL_VOID(backButtonProperty);

    auto pattern = GetPattern<SelectOverlayPattern>();
    CHECK_NULL_VOID(pattern);
    auto modifier = pattern->GetOverlayModifier();
    CHECK_NULL_VOID(modifier);

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);

    auto textOverlayTheme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_VOID(textOverlayTheme);

    auto shadowTheme = pipeline->GetTheme<ShadowTheme>();
    CHECK_NULL_VOID(shadowTheme);

    isDoingAnimation_ = true;
    isExtensionMenu_ = true;

    extensionProperty->UpdateVisibility(VisibleType::VISIBLE);
    extensionMenuStatus_ = FrameNodeStatus::VISIBLE;
    AnimationOption extensionOption;
    extensionOption.SetDuration(ANIMATION_DURATION2);
    extensionOption.SetCurve(Curves::FAST_OUT_SLOW_IN);
    auto toolbarHeight = textOverlayTheme->GetMenuToolbarHeight();
    auto frameSize = CalcSize(CalcLength(toolbarHeight.ConvertToPx()), CalcLength(toolbarHeight.ConvertToPx()));

    AnimationUtils::Animate(
        extensionOption, [extensionContext, selectMenuInnerContext, id = Container::CurrentId(), shadowTheme]() {
            ContainerScope scope(id);
            if (Container::LessThanAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
                extensionContext->UpdateOpacity(1.0);
            }
            extensionContext->UpdateTransformTranslate({ 0.0f, 0.0f, 0.0f });
            auto colorMode = SystemProperties::GetColorMode();
            extensionContext->UpdateBackShadow(shadowTheme->GetShadow(ShadowStyle::OuterDefaultMD, colorMode));
            selectMenuInnerContext->UpdateOpacity(0.0);
        });
    modifier->SetOtherPointRadius(MIN_DIAMETER / 2.0f);
    modifier->SetHeadPointRadius(MIN_ARROWHEAD_DIAMETER / 2.0f);
    modifier->SetLineEndOffset(true);
    auto menuPattern = extensionMenu_->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    menuPattern->SetMenuShow();

    FinishCallback callback = [selectMenuInnerProperty, extensionProperty, backButtonProperty,
                                  id = Container::CurrentId(), weak = WeakClaim(this)]() {
        ContainerScope scope(id);
        selectMenuInnerProperty->UpdateVisibility(VisibleType::GONE);
        extensionProperty->UpdateVisibility(VisibleType::VISIBLE);
        auto selectOverlay = weak.Upgrade();
        CHECK_NULL_VOID(selectOverlay);
        selectOverlay->SetAnimationStatus(false);
        selectOverlay->OnAccessibilityEvent(AccessibilityEventType::PAGE_CHANGE);
    };
    AnimationOption selectOption;
    selectOption.SetDuration(ANIMATION_DURATION1);
    selectOption.SetCurve(Curves::FRICTION);
    pipeline->FlushUITasks();
    AnimationUtils::OpenImplicitAnimation(selectOption, Curves::FRICTION, callback);
    if (GreatOrEqual(pipeline->GetFontScale(), AGING_MIN_SCALE)) {
        auto geometryNode = selectMenuInner_->GetGeometryNode();
        CHECK_NULL_VOID(geometryNode);
        auto selectMenuHeight = geometryNode->GetFrameSize().Height();
        frameSize = CalcSize(CalcLength(toolbarHeight.ConvertToPx()), CalcLength(selectMenuHeight));
    }
    selectProperty->UpdateUserDefinedIdealSize(frameSize);
    selectMenuInnerContext->UpdateTransformTranslate({ ANIMATION_TEXT_OFFSET.ConvertToPx(), 0.0f, 0.0f });
    selectMenu_->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    pipeline->FlushUITasks();
    AnimationUtils::CloseImplicitAnimation();
}

void SelectOverlayNode::BackAnimation()
{
    auto selectContext = selectMenu_->GetRenderContext();
    CHECK_NULL_VOID(selectContext);
    auto extensionContext = extensionMenu_->GetRenderContext();
    CHECK_NULL_VOID(extensionContext);
    auto selectMenuInnerContext = selectMenuInner_->GetRenderContext();
    CHECK_NULL_VOID(selectMenuInnerContext);

    auto extensionProperty = extensionMenu_->GetLayoutProperty();
    CHECK_NULL_VOID(extensionProperty);
    auto selectProperty = selectMenu_->GetLayoutProperty();
    CHECK_NULL_VOID(selectProperty);
    auto selectMenuInnerProperty = selectMenuInner_->GetLayoutProperty();
    CHECK_NULL_VOID(selectMenuInnerProperty);
    auto backButtonProperty = backButton_->GetLayoutProperty();
    CHECK_NULL_VOID(backButtonProperty);

    auto pattern = GetPattern<SelectOverlayPattern>();
    CHECK_NULL_VOID(pattern);
    auto modifier = pattern->GetOverlayModifier();
    CHECK_NULL_VOID(modifier);

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);

    auto textOverlayTheme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_VOID(textOverlayTheme);

    isDoingAnimation_ = true;
    isExtensionMenu_ = false;
    auto meanuWidth = pattern->GetMenuWidth();

    selectMenuInnerProperty->UpdateVisibility(VisibleType::VISIBLE);

    auto menuPattern = extensionMenu_->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    menuPattern->ShowMenuDisappearAnimation();
    AnimationOption extensionOption;
    extensionOption.SetDuration(ANIMATION_DURATION2);
    extensionOption.SetCurve(Curves::FAST_OUT_SLOW_IN);

    AnimationUtils::Animate(
        extensionOption, [extensionContext, selectMenuInnerContext, id = Container::CurrentId()]() {
        ContainerScope scope(id);
        if (Container::LessThanAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
            extensionContext->UpdateOpacity(0.0);
        }
        extensionContext->UpdateTransformTranslate({ 0.0f, MORE_MENU_TRANSLATE.ConvertToPx(), 0.0f });
        selectMenuInnerContext->UpdateOpacity(1.0);
    });

    modifier->SetOtherPointRadius(MAX_DIAMETER / 2.0f);
    modifier->SetHeadPointRadius(MAX_DIAMETER / 2.0f);
    modifier->SetLineEndOffset(false);

    auto toolbarHeight = textOverlayTheme->GetMenuToolbarHeight();
    auto frameSize = CalcSize(CalcLength(meanuWidth), CalcLength(toolbarHeight.ConvertToPx()));

    FinishCallback callback = [selectMenuInnerProperty, extensionProperty, backButtonProperty,
                                  id = Container::CurrentId(), weak = WeakClaim(this)]() {
        ContainerScope scope(id);
        selectMenuInnerProperty->UpdateVisibility(VisibleType::VISIBLE);
        extensionProperty->UpdateVisibility(VisibleType::GONE);
        auto selectOverlay = weak.Upgrade();
        CHECK_NULL_VOID(selectOverlay);
        selectOverlay->SetAnimationStatus(false);
        selectOverlay->OnAccessibilityEvent(AccessibilityEventType::PAGE_CHANGE);
    };

    AnimationOption selectOption;
    selectOption.SetDuration(ANIMATION_DURATION1);
    selectOption.SetCurve(Curves::FRICTION);
    pipeline->FlushUITasks();
    AnimationUtils::OpenImplicitAnimation(selectOption, Curves::FRICTION, callback);
    if (GreatOrEqual(pipeline->GetFontScale(), AGING_MIN_SCALE)) {
        auto geometryNode = selectMenu_->GetGeometryNode();
        CHECK_NULL_VOID(geometryNode);
        auto selectMenuHeight = geometryNode->GetFrameSize().Height();
        frameSize = CalcSize(CalcLength(meanuWidth), CalcLength(selectMenuHeight));
    }
    selectProperty->UpdateUserDefinedIdealSize(frameSize);
    selectMenuInnerContext->UpdateTransformTranslate({ 0.0f, 0.0f, 0.0f });
    selectContext->UpdateOffset(OffsetT<Dimension>(0.0_px, 0.0_px));
    selectMenu_->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    pipeline->FlushUITasks();
    AnimationUtils::CloseImplicitAnimation();
}

void SelectOverlayNode::AddExtensionMenuOptions(const std::vector<MenuOptionsParam>& menuOptionItems, int32_t index)
{
    CHECK_NULL_VOID(!extensionMenu_);
    std::vector<OptionParam> params;
    auto id = GetId();

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto iconTheme = pipeline->GetTheme<IconTheme>();
    auto defaultOptionCallback = [overlayId = id]() {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto overlayManager = pipeline->GetSelectOverlayManager();
        CHECK_NULL_VOID(overlayManager);
        overlayManager->DestroySelectOverlay(overlayId);
        overlayManager->CloseSelectContentOverlay(overlayId, CloseReason::CLOSE_REASON_TOOL_BAR, false);
    };
    if (!isShowInDefaultMenu_[OPTION_INDEX_CUT]) {
        auto iconPath = iconTheme ? iconTheme->GetIconPath(InternalResource::ResourceId::IC_CUT_SVG) : "";
        params.emplace_back(Localization::GetInstance()->GetEntryLetters(BUTTON_CUT), iconPath, defaultOptionCallback);
    }
    if (!isShowInDefaultMenu_[OPTION_INDEX_COPY]) {
        auto iconPath = iconTheme ? iconTheme->GetIconPath(InternalResource::ResourceId::IC_COPY_SVG) : "";
        params.emplace_back(Localization::GetInstance()->GetEntryLetters(BUTTON_COPY), iconPath, defaultOptionCallback);
    }
    if (!isShowInDefaultMenu_[OPTION_INDEX_PASTE]) {
        auto iconPath = iconTheme ? iconTheme->GetIconPath(InternalResource::ResourceId::IC_PASTE_SVG) : "";
        params.emplace_back(
            Localization::GetInstance()->GetEntryLetters(BUTTON_PASTE), iconPath, defaultOptionCallback);
    }
    if (!isShowInDefaultMenu_[OPTION_INDEX_COPY_ALL]) {
        auto iconPath = iconTheme ? iconTheme->GetIconPath(InternalResource::ResourceId::IC_SELECT_ALL_SVG) : "";
        params.emplace_back(
            Localization::GetInstance()->GetEntryLetters(BUTTON_COPY_ALL), iconPath, defaultOptionCallback);
    }
    if (!isShowInDefaultMenu_[OPTION_INDEX_SHARE]) {
        auto iconPath = iconTheme ? iconTheme->GetIconPath(InternalResource::ResourceId::IC_SHARE_SVG) : "";
        params.emplace_back(
            Localization::GetInstance()->GetEntryLetters(BUTTON_SHARE), iconPath, defaultOptionCallback);
    }
    if (!isShowInDefaultMenu_[OPTION_INDEX_TRANSLATE]) {
        auto iconPath = iconTheme ? iconTheme->GetIconPath(InternalResource::ResourceId::IC_TRANSLATE_SVG) : "";
        params.emplace_back(
            Localization::GetInstance()->GetEntryLetters(BUTTON_TRANSLATE), iconPath, defaultOptionCallback);
    }
    if (!isShowInDefaultMenu_[OPTION_INDEX_SEARCH]) {
        auto iconPath = iconTheme ? iconTheme->GetIconPath(InternalResource::ResourceId::IC_SEARCH_SVG) : "";
        params.emplace_back(
            Localization::GetInstance()->GetEntryLetters(BUTTON_SEARCH), iconPath, defaultOptionCallback);
    }
    int32_t itemNum = 0;
    for (auto item : menuOptionItems) {
        if (itemNum >= index) {
            auto callback = [overlayId = id, func = std::move(item.action),
                                actionRange = std::move(item.actionRange)]() {
                auto pipeline = PipelineContext::GetCurrentContext();
                CHECK_NULL_VOID(pipeline);
                auto overlayManager = pipeline->GetSelectOverlayManager();
                CHECK_NULL_VOID(overlayManager);

                auto selectOverlay = overlayManager->GetSelectOverlayNode(overlayId);
                auto pattern = selectOverlay->GetPattern<SelectOverlayPattern>();
                auto selectInfo = pattern->GetSelectInfo();
                func(selectInfo);
                CHECK_NULL_VOID(!actionRange);
                overlayManager->DestroySelectOverlay(overlayId);
                overlayManager->CloseSelectContentOverlay(overlayId, CloseReason::CLOSE_REASON_TOOL_BAR, false);
            };
            params.emplace_back(item.content.value_or("null"), item.icon.value_or(" "), callback);
        }
        itemNum++;
    }
    if (!params.empty()) {
        CHECK_NULL_VOID(backButton_);
        auto buttonId = backButton_->GetId();
        MenuParam menuParam;
        menuParam.placement = Placement::BOTTOM_RIGHT;
        auto menuWrapper = MenuView::Create(
            std::move(params), buttonId, "SelectMoreOrBackButton", MenuType::SELECT_OVERLAY_EXTENSION_MENU, menuParam);
        CHECK_NULL_VOID(menuWrapper);
        auto menu = DynamicCast<FrameNode>(menuWrapper->GetChildAtIndex(0));
        CHECK_NULL_VOID(menu);
        menuWrapper->RemoveChild(menu);
        menuWrapper.Reset();

        // set click position to menu
        auto props = menu->GetLayoutProperty<MenuLayoutProperty>();
        auto context = menu->GetRenderContext();
        CHECK_NULL_VOID(props);
        auto offsetY = 0.0f;
        auto textOverlayTheme = pipeline->GetTheme<TextOverlayTheme>();
        if (textOverlayTheme) {
            offsetY = textOverlayTheme->GetMenuToolbarHeight().ConvertToPx();
        }
        props->UpdateMenuOffset(GetPageOffset());
        context->UpdateBackShadow(ShadowConfig::NoneShadow);
        auto menuPattern = menu->GetPattern<MenuPattern>();
        CHECK_NULL_VOID(menuPattern);
        auto options = menuPattern->GetOptions();
        ElementRegister::GetInstance()->AddUINode(menu);
        menu->MountToParent(Claim(this));

        extensionMenu_ = menu;
        auto extensionMenuContext = extensionMenu_->GetRenderContext();
        CHECK_NULL_VOID(extensionMenuContext);

        extensionMenu_->GetLayoutProperty()->UpdateVisibility(VisibleType::GONE);
        extensionMenuStatus_ = FrameNodeStatus::GONE;
        if (Container::LessThanAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
            extensionMenuContext->UpdateOpacity(0.0);
        }
        extensionMenuContext->UpdateTransformTranslate({ 0.0f, MORE_MENU_TRANSLATE.ConvertToPx(), 0.0f });
        extensionMenu_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        extensionMenu_->MarkModifyDone();
        menuPattern->SetSelectOverlayExtensionMenuShow();
    }
}

void SelectOverlayNode::CreateToolBar()
{
    auto info = GetPattern<SelectOverlayPattern>()->GetSelectOverlayInfo();
    if (info->menuInfo.menuBuilder) {
        CreateCustomSelectOverlay(info);
        return;
    }

    selectMenu_ = FrameNode::GetOrCreateFrameNode("SelectMenu", ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<LinearLayoutPattern>(false); });
    selectMenu_->GetLayoutProperty<LinearLayoutProperty>()->UpdateMainAxisAlign(FlexAlign::FLEX_END);
    selectMenu_->GetRenderContext()->SetClipToFrame(true);
    selectMenu_->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_CONTENT);

    // Increase the node to realize the animation effect of font transparency and offset.
    selectMenuInner_ =
        FrameNode::GetOrCreateFrameNode("SelectMenuInner", ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<LinearLayoutPattern>(false); });
    selectMenuInner_->GetLayoutProperty<LinearLayoutProperty>()->UpdateMainAxisAlign(FlexAlign::FLEX_END);
    selectMenuInner_->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_CONTENT);

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto textOverlayTheme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_VOID(textOverlayTheme);
    selectMenu_->GetRenderContext()->UpdateOpacity(0.0);
    selectMenu_->GetRenderContext()->UpdateBackgroundColor(textOverlayTheme->GetMenuBackgroundColor());
    selectMenuInner_->GetRenderContext()->UpdateOpacity(1.0);
    selectMenuInner_->GetRenderContext()->UpdateTransformTranslate({ 0.0f, 0.0f, 0.0f });

    const auto& border = textOverlayTheme->GetMenuBorder();
    auto borderWidth = Dimension(border.Left().GetWidth().ConvertToPx());
    selectMenu_->GetLayoutProperty()->UpdateBorderWidth({ borderWidth, borderWidth, borderWidth, borderWidth });
    auto borderRadius = textOverlayTheme->GetMenuToolbarHeight() / 2.0f;
    selectMenu_->GetRenderContext()->UpdateBorderRadius({ borderRadius, borderRadius, borderRadius, borderRadius });
    auto borderColor = border.Left().GetColor();
    selectMenu_->GetRenderContext()->UpdateBorderColor({ borderColor, borderColor, borderColor, borderColor });
    auto borderStyle = border.Left().GetBorderStyle();
    selectMenu_->GetRenderContext()->UpdateBorderStyle({ borderStyle, borderStyle, borderStyle, borderStyle });

    const auto& padding = textOverlayTheme->GetMenuPadding();
    auto left = CalcLength(padding.Left().ConvertToPx());
    auto right = CalcLength(padding.Right().ConvertToPx());
    auto top = CalcLength(padding.Top().ConvertToPx());
    auto bottom = CalcLength(padding.Bottom().ConvertToPx());
    selectMenuInner_->GetLayoutProperty()->UpdatePadding({ left, right, top, bottom });

    if (LessNotEqual(pipeline->GetFontScale(), AGING_MIN_SCALE)) {
        selectMenuInner_->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            { std::nullopt, CalcLength(textOverlayTheme->GetMenuToolbarHeight()) });
    }

    if (info->menuInfo.menuIsShow) {
        selectMenu_->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
        selectMenuStatus_ = FrameNodeStatus::VISIBLE;
    } else {
        selectMenu_->GetLayoutProperty()->UpdateVisibility(VisibleType::GONE);
        selectMenuStatus_ = FrameNodeStatus::GONE;
    }

    selectMenuInner_->MountToParent(selectMenu_);

    auto shadowTheme = pipeline->GetTheme<ShadowTheme>();
    CHECK_NULL_VOID(shadowTheme);
    auto colorMode = SystemProperties::GetColorMode();
    selectMenu_->GetRenderContext()->UpdateBackShadow(shadowTheme->GetShadow(ShadowStyle::OuterDefaultMD, colorMode));
    selectMenu_->MountToParent(Claim(this));
    selectMenu_->MarkModifyDone();
}

void SelectOverlayNode::GetDefaultButtonAndMenuWidth(float& maxWidth)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto textOverlayTheme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_VOID(textOverlayTheme);
    auto selectOverlayMaxWidth = textOverlayTheme->GetSelectOverlayMaxWidth().ConvertToPx();

    const auto& menuPadding = textOverlayTheme->GetMenuPadding();

    maxWidth = selectOverlayMaxWidth - menuPadding.Left().ConvertToPx() - menuPadding.Right().ConvertToPx() -
               textOverlayTheme->GetMoreButtonHeight().ConvertToPx();
}

bool SelectOverlayNode::AddSystemDefaultOptions(float maxWidth, float& allocatedSize)
{
    auto info = GetPattern<SelectOverlayPattern>()->GetSelectOverlayInfo();
    memset_s(isShowInDefaultMenu_, sizeof(isShowInDefaultMenu_), 0, sizeof(isShowInDefaultMenu_));

    if (ShowCutCopy(maxWidth, allocatedSize, info)) {
        return true;
    }

    if (ShowPasteCopyAll(maxWidth, allocatedSize, info)) {
        return true;
    }

    if (ShowShare(maxWidth, allocatedSize, info)) {
        return true;
    }

    if (ShowCamera(maxWidth, allocatedSize, info)) {
        return true;
    }
    return false;
}

bool SelectOverlayNode::ShowCutCopy(float maxWidth, float& allocatedSize, std::shared_ptr<SelectOverlayInfo>& info)
{
    if (info->menuInfo.showCut) {
        float buttonWidth = 0.0f;
        auto button = BuildButton(
            Localization::GetInstance()->GetEntryLetters(BUTTON_CUT), info->menuCallback.onCut, GetId(), buttonWidth);
        if (maxWidth - allocatedSize >= buttonWidth) {
            button->MountToParent(selectMenuInner_);
            allocatedSize += buttonWidth;
            isShowInDefaultMenu_[OPTION_INDEX_CUT] = true;
        } else {
            button.Reset();
        }
    } else {
        isShowInDefaultMenu_[OPTION_INDEX_CUT] = true;
    }
    if (info->menuInfo.showCopy) {
        float buttonWidth = 0.0f;
        auto button = BuildButton(
            Localization::GetInstance()->GetEntryLetters(BUTTON_COPY), info->menuCallback.onCopy, GetId(), buttonWidth);
        if (maxWidth - allocatedSize >= buttonWidth) {
            button->MountToParent(selectMenuInner_);
            allocatedSize += buttonWidth;
            isShowInDefaultMenu_[OPTION_INDEX_COPY] = true;
        } else {
            button.Reset();
            return true;
        }
    } else {
        isShowInDefaultMenu_[OPTION_INDEX_COPY] = true;
    }
    return false;
}

bool SelectOverlayNode::ShowPasteCopyAll(float maxWidth, float& allocatedSize, std::shared_ptr<SelectOverlayInfo>& info)
{
    if (info->menuInfo.showPaste) {
        float buttonWidth = 0.0f;
#ifdef OHOS_PLATFORM
        auto button = BuildPasteButton(info->menuCallback.onPaste, GetId(), buttonWidth);
#else
        auto button = BuildButton(Localization::GetInstance()->GetEntryLetters(BUTTON_PASTE),
            info->menuCallback.onPaste, GetId(), buttonWidth);
#endif
        if (maxWidth - allocatedSize >= buttonWidth) {
            button->MountToParent(selectMenuInner_);
            allocatedSize += buttonWidth;
            isShowInDefaultMenu_[OPTION_INDEX_PASTE] = true;
        } else {
            button.Reset();
            return true;
        }
    } else {
        isShowInDefaultMenu_[OPTION_INDEX_PASTE] = true;
    }
    if (info->menuInfo.showCopyAll) {
        float buttonWidth = 0.0f;
        auto button = BuildButton(Localization::GetInstance()->GetEntryLetters(BUTTON_COPY_ALL),
            info->menuCallback.onSelectAll, GetId(), buttonWidth, true);
        if (maxWidth - allocatedSize >= buttonWidth) {
            button->MountToParent(selectMenuInner_);
            allocatedSize += buttonWidth;
            isShowInDefaultMenu_[OPTION_INDEX_COPY_ALL] = true;
        } else {
            button.Reset();
            return true;
        }
    } else {
        isShowInDefaultMenu_[OPTION_INDEX_COPY_ALL] = true;
    }
    return false;
}

bool SelectOverlayNode::ShowShare(float maxWidth, float& allocatedSize, std::shared_ptr<SelectOverlayInfo>& info)
{
    bool enableMenuShare = true;
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        enableMenuShare = false;
    }
    if (info->menuInfo.showCopy && enableMenuShare) {
        float buttonWidth = 0.0f;
        auto buttonShare = BuildButton(
            Localization::GetInstance()->GetEntryLetters(BUTTON_SHARE), nullptr, GetId(), buttonWidth, false);
        if (maxWidth - allocatedSize >= buttonWidth) {
            buttonShare->MountToParent(selectMenuInner_);
            allocatedSize += buttonWidth;
            isShowInDefaultMenu_[OPTION_INDEX_SHARE] = true;
        } else {
            buttonShare.Reset();
            return true;
        }
        auto buttonTranslase = BuildButton(
            Localization::GetInstance()->GetEntryLetters(BUTTON_TRANSLATE), nullptr, GetId(), buttonWidth, false);
        if (maxWidth - allocatedSize >= buttonWidth) {
            buttonTranslase->MountToParent(selectMenuInner_);
            allocatedSize += buttonWidth;
            isShowInDefaultMenu_[OPTION_INDEX_TRANSLATE] = true;
        } else {
            buttonTranslase.Reset();
            return true;
        }
        auto buttonSearch = BuildButton(
            Localization::GetInstance()->GetEntryLetters(BUTTON_SEARCH), nullptr, GetId(), buttonWidth, false);
        if (maxWidth - allocatedSize >= buttonWidth) {
            buttonSearch->MountToParent(selectMenuInner_);
            allocatedSize += buttonWidth;
            isShowInDefaultMenu_[OPTION_INDEX_SEARCH] = true;
        } else {
            buttonSearch.Reset();
            return true;
        }
    } else {
        isShowInDefaultMenu_[OPTION_INDEX_SHARE] = true;
        isShowInDefaultMenu_[OPTION_INDEX_TRANSLATE] = true;
        isShowInDefaultMenu_[OPTION_INDEX_SEARCH] = true;
    }
    return false;
}

bool SelectOverlayNode::ShowCamera(float maxWidth, float& allocatedSize, std::shared_ptr<SelectOverlayInfo>& info)
{
    if (info->menuInfo.showCameraInput) {
        float buttonWidth = 0.0f;
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_RETURN(pipeline, false);
        auto theme = pipeline->GetTheme<TextOverlayTheme>();
        CHECK_NULL_RETURN(theme, false);
        auto button = BuildButton(theme->GetCameraInput(), info->menuCallback.onCameraInput, GetId(), buttonWidth,
            false);
        if (maxWidth - allocatedSize >= buttonWidth) {
            button->MountToParent(selectMenuInner_);
            allocatedSize += buttonWidth;
            isShowInDefaultMenu_[OPTION_INDEX_CAMERA_INPUT] = true;
        } else {
            button.Reset();
            return true;
        }
    } else {
        isShowInDefaultMenu_[OPTION_INDEX_CAMERA_INPUT] = true;
    }
    return false;
}

void SelectOverlayNode::UpdateToolBar(bool menuItemChanged, bool noAnimation)
{
    auto info = GetPattern<SelectOverlayPattern>()->GetSelectOverlayInfo();
    if (menuItemChanged && info->menuInfo.menuBuilder == nullptr) {
        UpdateMenuInner(info);
    }
    if (info->menuInfo.menuDisable || !info->menuInfo.menuIsShow) {
        (noAnimation) ? HideFrameNodeImmediately(FrameNodeType::SELECTMENU)
                      : ExecuteOverlayStatus(FrameNodeType::SELECTMENU, FrameNodeTrigger::HIDE);
    } else {
        ExecuteOverlayStatus(FrameNodeType::SELECTMENU, FrameNodeTrigger::SHOW);
    }
    selectMenu_->MarkModifyDone();
    if (isExtensionMenu_ && extensionMenu_) {
        auto nodeTrigger = FrameNodeTrigger::SHOW;
        if (info->menuInfo.menuDisable || !info->menuInfo.menuIsShow) {
            nodeTrigger = FrameNodeTrigger::HIDE;
        }
        ExecuteOverlayStatus(FrameNodeType::EXTENSIONMENU, nodeTrigger);
        if (backButton_) {
            ExecuteOverlayStatus(FrameNodeType::BACKBUTTON, nodeTrigger);
        }
        extensionMenu_->MarkModifyDone();
        if (backButton_) {
            backButton_->MarkModifyDone();
        }
    }
    MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void SelectOverlayNode::UpdateMenuInner(const std::shared_ptr<SelectOverlayInfo>& info)
{
    CHECK_NULL_VOID(selectMenuInner_);
    selectMenuInner_->Clean();
    selectMenuInner_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    if (isExtensionMenu_) {
        MoreOrBackAnimation(false);
    }
    auto selectProperty = selectMenu_->GetLayoutProperty();
    CHECK_NULL_VOID(selectProperty);
    selectProperty->ClearUserDefinedIdealSize(true, false);
    float allocatedSize = 0.0f;
    float maxWidth = 0.0f;
    GetDefaultButtonAndMenuWidth(maxWidth);
    bool isDefaultOverMaxWidth = AddSystemDefaultOptions(maxWidth, allocatedSize);
    auto itemNum = -1;
    auto extensionOptionStartIndex = -1;
    for (auto item : info->menuOptionItems) {
        itemNum++;
        float extensionOptionWidth = 0.0f;
        auto button = BuildButton(item, GetId(), extensionOptionWidth);
        allocatedSize += extensionOptionWidth;
        if (allocatedSize > maxWidth) {
            button.Reset();
            extensionOptionStartIndex = itemNum;
            break;
        }
        button->MountToParent(selectMenuInner_);
    }
    if (backButton_) {
        isExtensionMenu_ = false;
        RemoveChild(backButton_);
        backButton_.Reset();
    }
    if (extensionMenu_) {
        RemoveChild(extensionMenu_);
        extensionMenu_.Reset();
    }
    if (extensionOptionStartIndex != -1 || isDefaultOverMaxWidth) {
        auto backButton = BuildMoreOrBackButton(GetId(), true);
        backButton->MountToParent(selectMenuInner_);
        // add back button
        if (!backButton_) {
            backButton_ = BuildMoreOrBackButton(GetId(), false);
            CHECK_NULL_VOID(backButton_);
            backButton_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
            backButton_->MountToParent(Claim(this));
        }
    }
    AddExtensionMenuOptions(info->menuOptionItems, extensionOptionStartIndex);
}

RefPtr<FrameNode> SelectOverlayNode::CreateMenuNode(const std::shared_ptr<SelectOverlayInfo>& info)
{
    RefPtr<FrameNode> menuWrapper;
    std::vector<OptionParam> params = GetOptionsParams(info);
    menuWrapper = MenuView::Create(
        std::move(params), -1, "SelectOverlayMenuByRightClick", MenuType::SELECT_OVERLAY_RIGHT_CLICK_MENU,
        { .isShowInSubWindow = false });
    CHECK_NULL_RETURN(menuWrapper, nullptr);
    auto menu = DynamicCast<FrameNode>(menuWrapper->GetChildAtIndex(0));
    // set click position to menu
    CHECK_NULL_RETURN(menu, nullptr);
    auto props = menu->GetLayoutProperty<MenuLayoutProperty>();
    CHECK_NULL_RETURN(props, nullptr);
    OffsetF pageOffset;
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto windowManager = pipeline->GetWindowManager();
    auto isContainerModal = pipeline->GetWindowModal() == WindowModal::CONTAINER_MODAL && windowManager &&
                            windowManager->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING;
    if (isContainerModal) {
        pageOffset = GetPageOffset();
    }
    props->UpdateMenuOffset(info->rightClickOffset + pageOffset);

    auto menuPattern = menu->GetPattern<MenuPattern>();
    CHECK_NULL_RETURN(menuPattern, nullptr);
    auto options = menuPattern->GetOptions();
    SetOptionsAction(info, options);

    menu->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    ElementRegister::GetInstance()->AddUINode(menu);

    auto gestureEventHub = menuWrapper->GetOrCreateGestureEventHub();
    if (gestureEventHub) {
        gestureEventHub->SetHitTestMode(HitTestMode::HTMTRANSPARENT_SELF);
    }
    return menuWrapper;
}

bool SelectOverlayNode::IsInSelectedOrSelectOverlayArea(const PointF& point)
{
    auto pattern = GetPattern<SelectOverlayPattern>();
    CHECK_NULL_RETURN(pattern, false);

    std::vector<RectF> rects;
    auto offset = GetGeometryNode() ? GetGeometryNode()->GetFrameOffset() : OffsetF();
    rects.emplace_back(pattern->GetHandleRegion(true) + offset);
    rects.emplace_back(pattern->GetHandleRegion(false) + offset);
    if (selectMenu_ && selectMenu_->GetGeometryNode()) {
        rects.emplace_back(selectMenu_->GetGeometryNode()->GetFrameRect() + offset);
    }
    if (extensionMenu_ && extensionMenu_->GetGeometryNode()) {
        rects.emplace_back(extensionMenu_->GetGeometryNode()->GetFrameRect() + offset);
    }

    if (pattern->IsCustomMenu()) {
        for (auto& child : pattern->GetHost()->GetChildren()) {
            auto childFrameNode = DynamicCast<FrameNode>(child);
            if (!childFrameNode) {
                continue;
            }
            rects.emplace_back(childFrameNode->GetGeometryNode()->GetFrameRect() + offset);
        }
    }

    for (const auto& rect : rects) {
        if (rect.IsInRegion(point)) {
            return true;
        }
    }
    return false;
}

void SelectOverlayNode::SetClosedByGlobalEvent(bool closedByGlobalEvent)
{
    auto selectOverlayPattern = GetPattern<SelectOverlayPattern>();
    CHECK_NULL_VOID(selectOverlayPattern);
    selectOverlayPattern->SetClosedByGlobalTouchEvent(closedByGlobalEvent);
}

void SelectOverlayNode::ShowSelectOverlay(bool animation)
{
    auto pattern = GetPattern<SelectOverlayPattern>();
    CHECK_NULL_VOID(pattern);

    if (animation) {
        AnimationOption option;
        option.SetDuration(MENU_SHOW_ANIMATION_DURATION);
        option.SetCurve(Curves::SHARP);

        AnimationUtils::Animate(option, [weak = WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto node = weak.Upgrade();
            CHECK_NULL_VOID(node);
            node->SetSelectMenuOpacity(1.0);
            node->SetExtensionMenuOpacity(1.0);
            node->SetBackButtonOpacity(1.0);
        });
    } else {
        SetSelectMenuOpacity(1.0);
        SetExtensionMenuOpacity(1.0);
        SetBackButtonOpacity(1.0);
    }

    pattern->SetHasShowAnimation(animation);
}

void SelectOverlayNode::HideSelectOverlay(const std::function<void()>& callback)
{
    AnimationOption handleOption;
    handleOption.SetDuration(HANDLE_ANIMATION_DURATION);
    handleOption.SetCurve(Curves::SHARP);

    AnimationUtils::Animate(handleOption, [weak = WeakClaim(this), id = Container::CurrentId()]() {
        ContainerScope scope(id);
        auto node = weak.Upgrade();
        CHECK_NULL_VOID(node);
        auto pattern = node->GetPattern<SelectOverlayPattern>();
        CHECK_NULL_VOID(pattern);
        auto contentModifier = pattern->GetContentModifier();
        CHECK_NULL_VOID(contentModifier);
        contentModifier->SetHandleOpacity(0.0);
    });

    AnimationOption overlayOption;
    overlayOption.SetDuration(MENU_HIDE_ANIMATION_DURATION);
    overlayOption.SetCurve(Curves::SHARP);

    AnimationUtils::Animate(
        overlayOption,
        [weak = WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto node = weak.Upgrade();
            CHECK_NULL_VOID(node);
            node->SetSelectMenuOpacity(0.0);
            node->SetExtensionMenuOpacity(0.0);
            node->SetBackButtonOpacity(0.0);
            auto pattern = node->GetPattern<SelectOverlayPattern>();
            CHECK_NULL_VOID(pattern);
            auto overlayModifier = pattern->GetOverlayModifier();
            CHECK_NULL_VOID(overlayModifier);
            overlayModifier->SetCirclesAndBackArrowOpacity(0.0);
        },
        callback);
}

void SelectOverlayNode::ExecuteOverlayStatus(FrameNodeType type, FrameNodeTrigger trigger)
{
    FrameNodeStatus status = FrameNodeStatus::VISIBLE;
    switch (type) {
        case FrameNodeType::SELECTMENU:
            status = selectMenuStatus_;
            break;
        case FrameNodeType::EXTENSIONMENU:
            status = extensionMenuStatus_;
            break;
        case FrameNodeType::BACKBUTTON:
            status = backButtonStatus_;
            break;
        default:
            break;
    }

    auto stateFuncIter = stateFuncs_.find(status);
    if (stateFuncIter != stateFuncs_.end()) {
        auto stateFunc = stateFuncIter->second;
        CHECK_NULL_VOID(stateFunc);
        (this->*stateFunc)(type, trigger);
    }
}

void SelectOverlayNode::SetFrameNodeStatus(FrameNodeType type, FrameNodeStatus status)
{
    switch (type) {
        case FrameNodeType::SELECTMENU:
            selectMenuStatus_ = status;
            break;
        case FrameNodeType::EXTENSIONMENU:
            extensionMenuStatus_ = status;
            break;
        case FrameNodeType::BACKBUTTON:
            backButtonStatus_ = status;
            break;
        default:
            break;
    }
}

void SelectOverlayNode::SetFrameNodeVisibility(FrameNodeType type, VisibleType visibleType)
{
    switch (type) {
        case FrameNodeType::SELECTMENU:
            selectMenu_->GetLayoutProperty()->UpdateVisibility(visibleType);
            break;
        case FrameNodeType::EXTENSIONMENU:
            extensionMenu_->GetLayoutProperty()->UpdateVisibility(visibleType);
            break;
        case FrameNodeType::BACKBUTTON:
            backButton_->GetLayoutProperty()->UpdateVisibility(visibleType);
            break;
        default:
            break;
    }
}

void SelectOverlayNode::SetFrameNodeOpacity(FrameNodeType type, float opacity)
{
    switch (type) {
        case FrameNodeType::SELECTMENU:
            SetSelectMenuOpacity(opacity);
            break;
        case FrameNodeType::EXTENSIONMENU:
            SetExtensionMenuOpacity(opacity);
            break;
        case FrameNodeType::BACKBUTTON:
            SetBackButtonOpacity(opacity);
            break;
        default:
            break;
    }
}

void SelectOverlayNode::HideFrameNodeImmediately(FrameNodeType type)
{
    SetFrameNodeStatus(type, FrameNodeStatus::GONE);
    SetFrameNodeVisibility(type, VisibleType::GONE);
    SetFrameNodeOpacity(type, 0.0f);
    if (type == FrameNodeType::SELECTMENU) { // select menu
        auto pattern = GetPattern<SelectOverlayPattern>();
        CHECK_NULL_VOID(pattern);
        auto overlayModifier = pattern->GetOverlayModifier();
        CHECK_NULL_VOID(overlayModifier);
        overlayModifier->SetCirclesAndBackArrowOpacity(0.0);
    }
}

void SelectOverlayNode::SetSelectMenuOpacity(float value)
{
    CHECK_NULL_VOID(selectMenu_);
    CHECK_NULL_VOID(selectMenu_->GetRenderContext());
    selectMenu_->GetRenderContext()->UpdateOpacity(value);
}

void SelectOverlayNode::SetExtensionMenuOpacity(float value)
{
    CHECK_NULL_VOID(extensionMenu_);
    CHECK_NULL_VOID(extensionMenu_->GetRenderContext());
    extensionMenu_->GetRenderContext()->UpdateOpacity(value);
}

void SelectOverlayNode::SetBackButtonOpacity(float value)
{
    CHECK_NULL_VOID(backButton_);
    CHECK_NULL_VOID(backButton_->GetRenderContext());
    backButton_->GetRenderContext()->UpdateOpacity(value);
}

} // namespace OHOS::Ace::NG
