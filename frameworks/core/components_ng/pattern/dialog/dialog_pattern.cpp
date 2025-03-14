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
#include "core/components_ng/pattern/dialog/dialog_pattern.h"

#include <climits>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>

#include "base/geometry/dimension.h"
#include "base/json/json_util.h"
#include "base/log/dump_log.h"
#include "base/log/log.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/subwindow/subwindow_manager.h"
#include "base/utils/utils.h"
#include "bridge/common/dom/dom_type.h"
#include "core/common/ace_engine.h"
#include "core/common/container.h"
#include "core/common/recorder/event_recorder.h"
#include "core/components/button/button_theme.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/theme/icon_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/divider/divider_layout_property.h"
#include "core/components_ng/pattern/divider/divider_model_ng.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/flex/flex_layout_algorithm.h"
#include "core/components_ng/pattern/flex/flex_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/relative_container/relative_container_model_ng.h"
#include "core/components_ng/pattern/relative_container/relative_container_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/key_event.h"
#include "core/event/touch_event.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

namespace {
constexpr int32_t SHEET_INFO_IDX = -2;
constexpr Dimension SHEET_IMAGE_MARGIN = 16.0_vp;
constexpr Dimension SHEET_DIVIDER_WIDTH = 1.0_px;
constexpr Dimension SHEET_LIST_PADDING = 24.0_vp;
constexpr Dimension DIALOG_BUTTON_TEXT_SIZE = 16.0_fp;
constexpr Color DEFAULT_BUTTON_COLOR = Color(0xff007dff);
const CalcLength SHEET_IMAGE_SIZE(40.0_vp);
constexpr int32_t TWO_BUTTON_MODE = 2;
constexpr int32_t ONE_BUTTON_MODE = 1;
constexpr int32_t START_CHILD_INDEX = 0;
constexpr uint32_t DIALOG_TITLE_MAXLINES = 1;
constexpr Dimension DIALOG_ONE_TITLE_ALL_HEIGHT = 56.0_vp;
constexpr Dimension DIALOG_TITLE_CONTENT_HEIGHT = 35.0_px;
constexpr int32_t DIALOG_TITLE_AVE_BY_2 = 2;
constexpr Dimension DIALOG_CONTENT_PADDING_TOP = 0.0_vp;
constexpr Dimension DIALOG_SUBTITLE_PADDING_LEFT = 24.0_vp;
constexpr Dimension DIALOG_SUBTITLE_PADDING_RIGHT = 24.0_vp;
constexpr Dimension DIALOG_TWO_TITLE_ZERO_SPACE = 0.0_vp;
constexpr Dimension ADAPT_TITLE_MIN_FONT_SIZE = 16.0_fp;
constexpr Dimension ADAPT_SUBTITLE_MIN_FONT_SIZE = 12.0_fp;
constexpr uint32_t ADAPT_TITLE_MAX_LINES = 2;
constexpr int32_t TEXT_ALIGN_TITLE_CENTER = 1;
constexpr uint32_t DIALOG_TEXT_MAXLINES_WITH_ELDERLY = 2;
constexpr uint32_t THREE_BUTON_DIALOG_BUTTON_NUMBER = 3;
constexpr float DIALOG_MIN_FONT_SCALE_FOR_AGING = 1.75f;
constexpr float LANDSCAPE_MAX_FONT_SCALE_FOR_AGING = 2.0f;
constexpr Dimension LANDSCAPE_MAINTitle_FONT_SIZE = 40.0_vp;
constexpr Dimension LANDSCAPE_SUBTITLE_FONT_SIZE = 28.0_vp;
constexpr Dimension LANDSCAPE_CONTENT_TEXT_FONT_SIZE = 32.0_vp;
constexpr Dimension LANDSCAPE_BUTTON_TEXT_FONT_SIZE = 32.0_vp;
constexpr int32_t BUTTON_TYPE_NORMAL = 1;
constexpr Dimension DIALOG_LANDSCAPE_BOUNDARY = 640.0_vp;

std::string GetBoolStr(bool isTure)
{
    return isTure ? "True" : "False";
}
} // namespace

void DialogPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureHub = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);

    if (!onClick_) {
        InitClickEvent(gestureHub);
    }
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    RegisterOnKeyEvent(focusHub);
    InitFocusEvent(focusHub);
}

void DialogPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->AddWindowSizeChangeCallback(host->GetId());
    InitHostWindowRect();
    auto foldModeChangeCallback = [weak = WeakClaim(this)](FoldDisplayMode foldDisplayMode) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->isFoldStatusChanged_ = true;
    };
    auto callbackId = pipelineContext->RegisterFoldDisplayModeChangedCallback(std::move(foldModeChangeCallback));
    UpdateFoldDisplayModeChangedCallbackId(callbackId);
}

void DialogPattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->RemoveWindowSizeChangeCallback(frameNode->GetId());
    if (HasFoldDisplayModeChangedCallbackId()) {
        pipeline->UnRegisterFoldDisplayModeChangedCallback(foldDisplayModeChangedCallbackId_.value_or(-1));
    }
}

void DialogPattern::InitClickEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    GestureEventFunc task = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleClick(info);
    };
    onClick_ = MakeRefPtr<ClickEvent>(std::move(task));
    gestureHub->AddClickEvent(onClick_);
}

void DialogPattern::HandleClick(const GestureEvent& info)
{
    if (info.GetSourceDevice() == SourceType::KEYBOARD) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto props = host->GetLayoutProperty<DialogLayoutProperty>();
    CHECK_NULL_VOID(props);
    auto globalOffset = host->GetPaintRectOffset();
    auto autoCancel = props->GetAutoCancel().value_or(true);
    if (autoCancel) {
        auto content = DynamicCast<FrameNode>(host->GetChildAtIndex(0));
        CHECK_NULL_VOID(content);
        auto contentRect = content->GetGeometryNode()->GetFrameRect();
        // close dialog if clicked outside content rect
        auto&& clickPosition = info.GetGlobalLocation();
        if (!contentRect.IsInRegion(
                PointF(clickPosition.GetX() - globalOffset.GetX(), clickPosition.GetY() - globalOffset.GetY()))) {
            auto pipeline = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto overlayManager = pipeline->GetOverlayManager();
            CHECK_NULL_VOID(overlayManager);
            if (this->ShouldDismiss()) {
                overlayManager->SetDismissDialogId(host->GetId());
                this->CallOnWillDismiss(static_cast<int32_t>(DialogDismissReason::DIALOG_TOUCH_OUTSIDE));
                TAG_LOGI(AceLogTag::ACE_DIALOG, "Dialog Should Dismiss");
                return;
            }
            PopDialog(-1);
            if (overlayManager->isMaskNode(GetHost()->GetId())) {
                overlayManager->PopModalDialog(GetHost()->GetId());
            }
        }
    }
}

void DialogPattern::PopDialog(int32_t buttonIdx = -1)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto overlayManager = pipeline->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (host->IsRemoving()) {
        return;
    }

    auto hub = host->GetEventHub<DialogEventHub>();
    if (buttonIdx != -1) {
        hub->FireSuccessEvent(buttonIdx);
        RecordEvent(buttonIdx);
    } else {
        // trigger onCancel callback
        hub->FireCancelEvent();
        RecordEvent(buttonIdx);
    }
    if (dialogProperties_.isShowInSubWindow) {
        SubwindowManager::GetInstance()->DeleteHotAreas(
            SubwindowManager::GetInstance()->GetDialogSubWindowId(), host->GetId());
        SubwindowManager::GetInstance()->HideDialogSubWindow(SubwindowManager::GetInstance()->GetDialogSubWindowId());
    }
    overlayManager->CloseDialog(host);
}

void DialogPattern::RecordEvent(int32_t btnIndex) const
{
    if (!Recorder::EventRecorder::Get().IsComponentRecordEnable()) {
        return;
    }
    std::string btnText;
    if (btnIndex >= 0 && static_cast<size_t>(btnIndex) < dialogProperties_.buttons.size()) {
        btnText = dialogProperties_.buttons.at(btnIndex).text;
    }
    Recorder::EventType eventType;
    if (btnIndex == -1) {
        eventType = Recorder::EventType::DIALOG_CANCEL;
    } else {
        eventType = Recorder::EventType::DIALOG_ACTION;
    }
    Recorder::EventParamsBuilder builder;
    builder.SetEventType(eventType)
        .SetText(btnText)
        .SetExtra(Recorder::KEY_TITLE, title_)
        .SetExtra(Recorder::KEY_SUB_TITLE, subtitle_);
    Recorder::EventRecorder::Get().OnEvent(std::move(builder));
}

// set render context properties of content frame
void DialogPattern::UpdateContentRenderContext(const RefPtr<FrameNode>& contentNode, const DialogProperties& props)
{
    auto contentRenderContext = contentNode->GetRenderContext();
    CHECK_NULL_VOID(contentRenderContext);
    contentRenderContext_ = contentRenderContext;
    UpdateBgBlurStyle(contentRenderContext, props);
    if (props.borderRadius.has_value()) {
        if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            auto radiusValue = props.borderRadius.value();
            ParseBorderRadius(radiusValue);
            contentRenderContext->UpdateBorderRadius(radiusValue);
        } else {
            contentRenderContext->UpdateBorderRadius(props.borderRadius.value());
        }
    } else {
        BorderRadiusProperty radius;
        radius.SetRadius(dialogTheme_->GetRadius().GetX());
        contentRenderContext->UpdateBorderRadius(radius);
    }
    if (props.borderWidth.has_value()) {
        auto layoutProps = contentNode->GetLayoutProperty<LinearLayoutProperty>();
        CHECK_NULL_VOID(layoutProps);
        layoutProps->UpdateBorderWidth(props.borderWidth.value());
        contentRenderContext->UpdateBorderWidth(props.borderWidth.value());
        contentNodeMap_[DialogContentNode::BORDERWIDTH] = contentNode;
    } else {
        BorderWidthProperty borderWidth;
        Dimension width = dialogTheme_->GetBackgroudBorderWidth();
        borderWidth.SetBorderWidth(width);
        contentRenderContext->UpdateBorderWidth(borderWidth);
        contentNodeMap_[DialogContentNode::BORDERWIDTH] = contentNode;
    }
    if (props.borderStyle.has_value()) {
        contentRenderContext->UpdateBorderStyle(props.borderStyle.value());
    }
    if (props.borderColor.has_value()) {
        contentRenderContext->UpdateBorderColor(props.borderColor.value());
    } else {
        BorderColorProperty borderColor;
        Color color = dialogTheme_->GetBackgroudBorderColor();
        borderColor.SetColor(color);
        contentRenderContext->UpdateBorderColor(borderColor);
    }
    if (props.shadow.has_value()) {
        contentRenderContext->UpdateBackShadow(props.shadow.value());
    } else {
        Shadow shadow = Shadow::CreateShadow(static_cast<ShadowStyle>(dialogTheme_->GetShadowDialog()));
        contentRenderContext->UpdateBackShadow(shadow);
    }
    contentRenderContext->SetClipToBounds(true);
}
void DialogPattern::UpdateBgBlurStyle(const RefPtr<RenderContext>& contentRenderContext, const DialogProperties& props)
{
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN) &&
        contentRenderContext->IsUniRenderEnabled() && props.isSysBlurStyle) {
        BlurStyleOption styleOption;
        styleOption.blurStyle = static_cast<BlurStyle>(
            props.backgroundBlurStyle.value_or(static_cast<int>(BlurStyle::COMPONENT_ULTRA_THICK)));
        contentRenderContext->UpdateBackBlurStyle(styleOption);
        contentRenderContext->UpdateBackgroundColor(props.backgroundColor.value_or(dialogTheme_->GetColorBgWithBlur()));
    } else {
        contentRenderContext->UpdateBackgroundColor(props.backgroundColor.value_or(dialogTheme_->GetBackgroundColor()));
    }
}
void DialogPattern::ParseBorderRadius(BorderRadiusProperty& raidus)
{
    if (!raidus.radiusTopLeft.has_value() || raidus.radiusTopLeft.value().Value() < 0) {
        raidus.radiusTopLeft = dialogTheme_->GetRadius().GetX();
    }
    if (!raidus.radiusTopRight.has_value() || raidus.radiusTopRight.value().Value() < 0) {
        raidus.radiusTopRight = dialogTheme_->GetRadius().GetX();
    }
    if (!raidus.radiusBottomLeft.has_value() || raidus.radiusBottomLeft.value().Value() < 0) {
        raidus.radiusBottomLeft = dialogTheme_->GetRadius().GetX();
    }
    if (!raidus.radiusBottomRight.has_value() || raidus.radiusBottomRight.value().Value() < 0) {
        raidus.radiusBottomRight = dialogTheme_->GetRadius().GetX();
    }
}

RefPtr<FrameNode> DialogPattern::CreateDialogScroll(const DialogProperties& dialogProps)
{
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    CHECK_NULL_RETURN(scroll, nullptr);
    auto props = scroll->GetLayoutProperty<ScrollLayoutProperty>();
    props->UpdateAxis(Axis::VERTICAL);
    props->UpdateAlignment(Alignment::CENTER_LEFT);
    // If title not exist, set scroll align center so that text align center.
    auto scrollFlexAlign = dialogTheme_->GetScrollFlexAlign();
    if ((dialogProps.title.empty() && dialogProps.subtitle.empty())) {
        scrollFlexAlign = FlexAlign::CENTER;
    }
    props->UpdateAlignSelf(scrollFlexAlign);
    return scroll;
}

void DialogPattern::BuildChild(const DialogProperties& props)
{
    UpdatePropertyForElderly(props.buttons);
    // append customNode
    auto customNode = customNode_.Upgrade();
    if (customNode) {
        BuildCustomChild(props, customNode);
        return;
    }
    if (isSuitableForElderly_) {
        neeedUpdateOrientation_ = true;
    }
    // Make dialog Content Column
    auto contentColumn = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    CHECK_NULL_VOID(contentColumn);
    if (!props.title.empty() || !props.subtitle.empty()) {
        auto title = BuildTitle(props);
        CHECK_NULL_VOID(title);
        titleContainer_ = title;
        contentColumn->AddChild(title);
    }

    if (!props.content.empty()) {
        auto content = BuildContent(props);
        CHECK_NULL_VOID(content);
        // create a scroll
        auto scroll = CreateDialogScroll(props);
        CHECK_NULL_VOID(scroll);
        content->MountToParent(scroll);
        scroll->MountToParent(contentColumn);
        scroll->MarkModifyDone();
    }

    if (!props.customStyle) {
        UpdateContentRenderContext(contentColumn, props);
        if (props.height.has_value()) {
            auto layoutProps = contentColumn->GetLayoutProperty<LinearLayoutProperty>();
            CHECK_NULL_VOID(layoutProps);
            layoutProps->UpdateMainAxisAlign(FlexAlign::SPACE_BETWEEN);
            layoutProps->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);
        }
    }

    auto columnProp = AceType::DynamicCast<LinearLayoutProperty>(contentColumn->GetLayoutProperty());
    CHECK_NULL_VOID(columnProp);
    // content is full screen in Watch mode
    auto measureType = dialogTheme_->GetColumnMeasureType();
    columnProp->UpdateMeasureType(measureType);

    // build ActionSheet child
    if (props.type == DialogType::ACTION_SHEET && !props.sheetsInfo.empty()) {
        auto sheetContainer = BuildSheet(props.sheetsInfo);
        contentNodeMap_[DialogContentNode::SHEET] = sheetContainer;
        CHECK_NULL_VOID(sheetContainer);
        sheetContainer->MountToParent(contentColumn);
        // scrollable
        sheetContainer->MarkModifyDone();
    }

    // Make Menu node if hasMenu (actionMenu)
    if (props.isMenu) {
        bool hasTitle = !props.title.empty() || !props.subtitle.empty();
        auto menu = BuildMenu(props.buttons, hasTitle);
        CHECK_NULL_VOID(menu);
        menu->MountToParent(contentColumn);
    } else {
        // build buttons
        if (!props.buttons.empty()) {
            auto buttonContainer = BuildButtons(props.buttons, props.buttonDirection);
            CHECK_NULL_VOID(buttonContainer);
            buttonContainer->MountToParent(contentColumn);
        }
    }

    auto dialog = GetHost();
    contentColumn->MountToParent(dialog);
    if (isSuitableForElderly_ && NeedsButtonDirectionChange(props.buttons)) {
        //remove buttonContainer when Button text is too long
        contentColumn->RemoveChild(buttonContainer_);
        auto buttonContainerNew = BuildButtons(props.buttons, DialogButtonDirection::VERTICAL);
        buttonContainerNew->MountToParent(contentColumn);
        buttonContainer_ = buttonContainerNew;
    }
}

void DialogPattern::BuildCustomChild(const DialogProperties& props, const RefPtr<UINode>& customNode)
{
    auto contentWrapper = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    CHECK_NULL_VOID(contentWrapper);
    if (!props.customStyle) {
        UpdateContentRenderContext(contentWrapper, props);
    }
    customNode->MountToParent(contentWrapper);
    auto dialog = GetHost();
    contentWrapper->MountToParent(dialog);
}

RefPtr<FrameNode> DialogPattern::BuildMainTitle(const DialogProperties& dialogProperties)
{
    auto title = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    auto titleProp = AceType::DynamicCast<TextLayoutProperty>(title->GetLayoutProperty());
    CHECK_NULL_RETURN(titleProp, nullptr);
    titleProp->UpdateMaxLines(DIALOG_TITLE_MAXLINES);
    titleProp->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    std::string titleContent = dialogProperties.title.empty() ? dialogProperties.subtitle : dialogProperties.title;
    titleProp->UpdateContent(titleContent);
    auto titleStyle = dialogTheme_->GetTitleTextStyle();
    titleProp->UpdateFontSize(titleStyle.GetFontSize());
    titleProp->UpdateFontWeight(titleStyle.GetFontWeight());
    titleProp->UpdateTextColor(titleStyle.GetTextColor());
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        titleProp->UpdateAdaptMaxFontSize(dialogTheme_->GetTitleTextStyle().GetFontSize());
        titleProp->UpdateAdaptMinFontSize(ADAPT_TITLE_MIN_FONT_SIZE);
        titleProp->UpdateHeightAdaptivePolicy(TextHeightAdaptivePolicy::MIN_FONT_SIZE_FIRST);
        titleProp->UpdateMaxLines(ADAPT_TITLE_MAX_LINES);
    }
    if (isSuitableForElderly_) {
        if ((isLandspace_ && !isThreeButtonsDialog_) ||
            (deviceOrientation_ == DeviceOrientation::PORTRAIT &&
                GreatOrEqual(fontScaleForElderly_, LANDSCAPE_MAX_FONT_SCALE_FOR_AGING))) {
            titleProp->UpdateMaxLines(DIALOG_TEXT_MAXLINES_WITH_ELDERLY);
            titleProp->UpdateFontSize(LANDSCAPE_MAINTitle_FONT_SIZE);
            titleProp->UpdateAdaptMaxFontSize(LANDSCAPE_MAINTitle_FONT_SIZE);
            titleProp->UpdateAdaptMinFontSize(LANDSCAPE_MAINTitle_FONT_SIZE);
        } else {
            titleProp->UpdateMaxLines(DIALOG_TEXT_MAXLINES_WITH_ELDERLY);
            titleProp->UpdateFontSize(titleStyle.GetFontSize());
            titleProp->UpdateAdaptMaxFontSize(titleStyle.GetFontSize());
            titleProp->UpdateAdaptMinFontSize(titleStyle.GetFontSize());
        }
        if (isNeedToVp_) {
            Dimension defaultFontSize = Dimension(titleStyle.GetFontSize().Value(), DimensionUnit::VP);
            titleProp->UpdateFontSize(defaultFontSize);
            titleProp->UpdateAdaptMaxFontSize(defaultFontSize);
            titleProp->UpdateAdaptMinFontSize(defaultFontSize);
        }
    }
    PaddingProperty titlePadding;
    CreateTitleRowNode(dialogProperties, titlePadding);
    titleProp->UpdatePadding(titlePadding);
    // XTS inspector value
    title_ = dialogProperties.title;
    subtitle_ = dialogProperties.subtitle;
    auto titleRow = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    CHECK_NULL_RETURN(titleRow, nullptr);
    auto titleRowProps = titleRow->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(titleRowProps, nullptr);
    if (dialogTheme_->GetTextAlignTitle() == TEXT_ALIGN_TITLE_CENTER) {
        titleRowProps->UpdateMainAxisAlign(FlexAlign::CENTER);
    } else {
        titleRowProps->UpdateMainAxisAlign(FlexAlign::FLEX_START);
    }
    titleRowProps->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);
    title->MountToParent(titleRow);
    title->MarkModifyDone();
    contentNodeMap_[dialogProperties.title.empty() ? DialogContentNode::SUBTITLE : DialogContentNode::TITLE] = title;
    return titleRow;
}
void DialogPattern::CreateTitleRowNode(const DialogProperties& dialogProperties, PaddingProperty& titlePadding)
{
    auto paddingInTheme = (dialogProperties.content.empty() && dialogProperties.buttons.empty())
                              ? dialogTheme_->GetTitleDefaultPadding()
                              : dialogTheme_->GetTitleAdjustPadding();
    titlePadding.left = CalcLength(paddingInTheme.Left());
    titlePadding.right = CalcLength(paddingInTheme.Right());
    if (!dialogProperties.title.empty() && !dialogProperties.subtitle.empty()) {
        titlePadding.top = CalcLength(dialogTheme_->GetPaddingTopTitle());
        titlePadding.bottom = CalcLength(DIALOG_TWO_TITLE_ZERO_SPACE);
    } else {
        auto padding =
            DIALOG_ONE_TITLE_ALL_HEIGHT - Dimension(DIALOG_TITLE_CONTENT_HEIGHT.ConvertToVp(), DimensionUnit::VP);
        if (dialogTheme_->GetPaddingSingleTitle().ConvertToVp() > 0) {
            padding = dialogTheme_->GetPaddingSingleTitle();
        }
        titlePadding.top = CalcLength(padding / DIALOG_TITLE_AVE_BY_2);
        titlePadding.bottom = CalcLength(padding / DIALOG_TITLE_AVE_BY_2);
    }
}
RefPtr<FrameNode> DialogPattern::BuildSubTitle(const DialogProperties& dialogProperties)
{
    auto subtitle = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    auto titleProp = AceType::DynamicCast<TextLayoutProperty>(subtitle->GetLayoutProperty());
    CHECK_NULL_RETURN(titleProp, nullptr);
    auto titleStyle = dialogTheme_->GetSubTitleTextStyle();
    titleProp->UpdateMaxLines(DIALOG_TITLE_MAXLINES);
    titleProp->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    titleProp->UpdateContent(dialogProperties.subtitle);
    titleProp->UpdateFontSize(titleStyle.GetFontSize());
    titleProp->UpdateTextColor(titleStyle.GetTextColor());
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        titleProp->UpdateAdaptMaxFontSize(titleStyle.GetFontSize());
        titleProp->UpdateAdaptMinFontSize(ADAPT_SUBTITLE_MIN_FONT_SIZE);
        titleProp->UpdateHeightAdaptivePolicy(TextHeightAdaptivePolicy::MIN_FONT_SIZE_FIRST);
        titleProp->UpdateMaxLines(ADAPT_TITLE_MAX_LINES);
    }
    PaddingProperty titlePadding;
    titlePadding.left = CalcLength(DIALOG_SUBTITLE_PADDING_LEFT);
    titlePadding.right = CalcLength(DIALOG_SUBTITLE_PADDING_RIGHT);
    titlePadding.top = CalcLength(DIALOG_TWO_TITLE_ZERO_SPACE);
    titlePadding.bottom = CalcLength(dialogTheme_->GetPaddingTopTitle());
    titleProp->UpdatePadding(titlePadding);

    if (isSuitableForElderly_) {
        if ((isLandspace_ && !isThreeButtonsDialog_) ||
            (deviceOrientation_ == DeviceOrientation::PORTRAIT &&
                GreatOrEqual(fontScaleForElderly_, LANDSCAPE_MAX_FONT_SCALE_FOR_AGING))) {
            titleProp->UpdateMaxLines(DIALOG_TEXT_MAXLINES_WITH_ELDERLY);
            titleProp->UpdateFontSize(LANDSCAPE_SUBTITLE_FONT_SIZE);
            titleProp->UpdateAdaptMaxFontSize(LANDSCAPE_SUBTITLE_FONT_SIZE);
            titleProp->UpdateAdaptMinFontSize(LANDSCAPE_SUBTITLE_FONT_SIZE);
        } else {
            titleProp->UpdateMaxLines(DIALOG_TEXT_MAXLINES_WITH_ELDERLY);
            titleProp->UpdateFontSize(titleStyle.GetFontSize());
            titleProp->UpdateAdaptMaxFontSize(titleStyle.GetFontSize());
            titleProp->UpdateAdaptMinFontSize(titleStyle.GetFontSize());
        }
        if (isNeedToVp_) {
            Dimension defaultFontSize = Dimension(titleStyle.GetFontSize().Value(), DimensionUnit::VP);
            titleProp->UpdateFontSize(defaultFontSize);
            titleProp->UpdateAdaptMaxFontSize(defaultFontSize);
            titleProp->UpdateAdaptMinFontSize(defaultFontSize);
        }
    }
    // XTS inspector value
    subtitle_ = dialogProperties.subtitle;

    auto subtitleRow = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    CHECK_NULL_RETURN(subtitleRow, nullptr);
    auto subtitleRowProps = subtitleRow->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(subtitleRowProps, nullptr);
    if (dialogTheme_->GetTextAlignTitle() == TEXT_ALIGN_TITLE_CENTER) {
        subtitleRowProps->UpdateMainAxisAlign(FlexAlign::CENTER);
    } else {
        subtitleRowProps->UpdateMainAxisAlign(FlexAlign::FLEX_START);
    }
    subtitleRowProps->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);
    subtitle->MountToParent(subtitleRow);
    subtitle->MarkModifyDone();
    contentNodeMap_[DialogContentNode::SUBTITLE] = subtitle;
    return subtitleRow;
}

RefPtr<FrameNode> DialogPattern::BuildTitle(const DialogProperties& dialogProperties)
{
    auto titleRow = BuildMainTitle(dialogProperties);
    if (!dialogProperties.title.empty() && !dialogProperties.subtitle.empty()) {
        auto titleColumn = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
        CHECK_NULL_RETURN(titleColumn, nullptr);
        auto columnProps = titleColumn->GetLayoutProperty<LinearLayoutProperty>();
        CHECK_NULL_RETURN(columnProps, nullptr);
        columnProps->UpdateMainAxisAlign(FlexAlign::FLEX_START);
        columnProps->UpdateMeasureType(MeasureType::MATCH_CONTENT);
        auto subtitleRow = BuildSubTitle(dialogProperties);
        titleColumn->AddChild(titleRow);
        titleColumn->AddChild(subtitleRow);
        return titleColumn;
    }
    return titleRow;
}

RefPtr<FrameNode> DialogPattern::BuildContent(const DialogProperties& props)
{
    // Make Content node
    auto contentNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    auto contentProp = AceType::DynamicCast<TextLayoutProperty>(contentNode->GetLayoutProperty());
    CHECK_NULL_RETURN(contentProp, nullptr);
    // textAlign always align start. When text line count 1 and title doesn't exist, set text center position.
    contentProp->UpdateTextAlign(TextAlign::START);
    contentProp->UpdateContent(props.content);
    auto contentStyle = dialogTheme_->GetContentTextStyle();
    contentProp->UpdateFontSize(contentStyle.GetFontSize());
    contentProp->UpdateTextColor(contentStyle.GetTextColor());
    if (isSuitableForElderly_ && isLandspace_ && !isThreeButtonsDialog_) {
        contentProp->UpdateFontSize(LANDSCAPE_CONTENT_TEXT_FONT_SIZE);
    }
    if (isNeedToVp_) {
        Dimension defaultFontSize = Dimension(contentStyle.GetFontSize().Value(), DimensionUnit::VP);
        contentProp->UpdateFontSize(defaultFontSize);
    }
    // update padding
    Edge contentPaddingInTheme;
    PaddingProperty contentPadding;
    if (!props.title.empty() || !props.subtitle.empty()) {
        contentPaddingInTheme =
            props.buttons.empty() ? dialogTheme_->GetDefaultPadding() : dialogTheme_->GetAdjustPadding();
        contentPadding.top = CalcLength(DIALOG_CONTENT_PADDING_TOP);
    } else {
        contentPaddingInTheme =
            props.buttons.empty() ? dialogTheme_->GetContentDefaultPadding() : dialogTheme_->GetContentAdjustPadding();
        contentPadding.top = CalcLength(contentPaddingInTheme.Top());
    }
    contentPadding.left = CalcLength(contentPaddingInTheme.Left());
    contentPadding.right = CalcLength(contentPaddingInTheme.Right());
    contentPadding.bottom = CalcLength(contentPaddingInTheme.Bottom());
    contentProp->UpdatePadding(contentPadding);

    // XTS inspector value
    message_ = props.content;
    contentNode->MarkModifyDone();
    contentNodeMap_[DialogContentNode::MESSAGE] = contentNode;
    return contentNode;
}

// to close dialog when clicked, use button index in Prompt to trigger success callback
void DialogPattern::BindCloseCallBack(const RefPtr<GestureEventHub>& hub, int32_t buttonIdx)
{
    auto host = GetHost();
    auto closeCallback = [weak = WeakClaim(RawPtr(host)), buttonIdx](GestureEvent& /*info*/) {
        auto dialog = weak.Upgrade();
        CHECK_NULL_VOID(dialog);
        dialog->GetPattern<DialogPattern>()->PopDialog(buttonIdx);
    };

    hub->AddClickEvent(AceType::MakeRefPtr<ClickEvent>(closeCallback));
}

void DialogPattern::ParseButtonFontColorAndBgColor(
    const ButtonInfo& params, std::string& textColor, std::optional<Color>& bgColor)
{
    // Parse Button Style
    if (params.dlgButtonStyle.has_value()) {
        switch (params.dlgButtonStyle.value()) {
            case DialogButtonStyle::DEFAULT:
                textColor = dialogTheme_->GetButtonDefaultFontColor().ColorToString();
                bgColor = dialogTheme_->GetButtonDefaultBgColor();
                break;
            case DialogButtonStyle::HIGHTLIGHT:
                textColor = dialogTheme_->GetButtonHighlightFontColor().ColorToString();
                bgColor = dialogTheme_->GetButtonHighlightBgColor();
                break;
            default:
                break;
        }
    }

    // font color and background color
    if (!params.textColor.empty()) {
        textColor = params.textColor;
    }
    if (params.isBgColorSetted) {
        bgColor = params.bgColor;
    }

    // Parse default focus
    if (textColor.empty()) {
        if (params.defaultFocus && isFirstDefaultFocus_) {
            textColor = dialogTheme_->GetButtonHighlightFontColor().ColorToString();
        } else {
            textColor = dialogTheme_->GetButtonDefaultFontColor().ColorToString();
        }
    }
    if (!bgColor.has_value()) {
        if (params.defaultFocus && isFirstDefaultFocus_) {
            bgColor = dialogTheme_->GetButtonHighlightBgColor();
            isFirstDefaultFocus_ = false;
        } else {
            bgColor = dialogTheme_->GetButtonDefaultBgColor();
        }
    }
}

RefPtr<FrameNode> DialogPattern::CreateButton(
    const ButtonInfo& params, int32_t index, bool isCancel, bool isVertical, int32_t length)
{
    auto buttonNode = FrameNode::CreateFrameNode(
        V2::BUTTON_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), MakeRefPtr<ButtonPattern>());
    CHECK_NULL_RETURN(buttonNode, nullptr);
    UpdateDialogButtonProperty(buttonNode, index, isVertical, length);
    // parse button text color and background color
    std::string textColor;
    std::optional<Color> bgColor;
    ParseButtonFontColorAndBgColor(params, textColor, bgColor);

    // append text inside button
    auto textNode = CreateButtonText(params.text, textColor);
    CHECK_NULL_RETURN(textNode, nullptr);
    textNode->MountToParent(buttonNode);
    textNode->MarkModifyDone();
    SetButtonEnabled(buttonNode, params.enabled);
    auto hub = buttonNode->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(hub, nullptr);
    // bind click event
    if (params.action) {
        hub->AddClickEvent(params.action);
    }

    if (params.isPrimary) {
        auto focusHub = buttonNode->GetFocusHub();
        CHECK_NULL_RETURN(focusHub, nullptr);
        focusHub->SetIsDefaultFocus(params.isPrimary);
    }

    // to close dialog when clicked inside button rect
    if (!isCancel) {
        BindCloseCallBack(hub, index);
    } else {
        BindCloseCallBack(hub, -1);
    }

    // add scale animation
    auto inputHub = buttonNode->GetOrCreateInputEventHub();
    CHECK_NULL_RETURN(inputHub, nullptr);
    inputHub->SetHoverEffect(HoverEffectType::AUTO);

    // update background color
    auto renderContext = buttonNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, nullptr);
    renderContext->UpdateBackgroundColor(bgColor.value());

    // set button default height
    auto layoutProps = buttonNode->GetLayoutProperty();
    CHECK_NULL_RETURN(layoutProps, nullptr);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto theme = pipeline->GetTheme<ButtonTheme>();
    CHECK_NULL_RETURN(theme, nullptr);
    if (!isSuitableForElderly_) {
        layoutProps->UpdateUserDefinedIdealSize(CalcSize(std::nullopt, CalcLength(theme->GetHeight())));
    }
    return buttonNode;
}

void DialogPattern::UpdateDialogButtonProperty(
    RefPtr<FrameNode>& buttonNode, int32_t index, bool isVertical, int32_t length)
{
    // update button padding
    auto buttonProp = AceType::DynamicCast<ButtonLayoutProperty>(buttonNode->GetLayoutProperty());
    if (dialogTheme_->GetButtonType() == BUTTON_TYPE_NORMAL) {
        buttonProp->UpdateButtonStyle(ButtonStyleMode::NORMAL);
    }
    PaddingProperty buttonPadding;
    buttonPadding.left = CalcLength(SHEET_LIST_PADDING);
    buttonPadding.right = CalcLength(SHEET_LIST_PADDING);
    buttonProp->UpdatePadding(buttonPadding);
    if (!isVertical) {
        // set flex grow to fill horizontal space
        buttonProp->UpdateLayoutWeight(1);
        buttonProp->UpdateFlexGrow(1.0);
        buttonProp->UpdateFlexShrink(1.0);
        if (isSuitableForElderly_ && index != 0) {
            MarginProperty margin = {
                .left = CalcLength(dialogTheme_->GetMarginLeft()),
            };
            buttonProp->UpdateMargin(margin);
        }
    } else if (isVertical && index != (length - 1)) {
        // update button space in vertical
        auto buttonSpace = dialogTheme_->GetMutiButtonPaddingVertical();
        MarginProperty margin = {
            .bottom = CalcLength(buttonSpace),
        };
        buttonProp->UpdateMargin(margin);
    }
}

RefPtr<FrameNode> DialogPattern::CreateDivider(
    const Dimension& dividerLength, const Dimension& dividerWidth, const Color& color, const Dimension& space)
{
    auto dividerNode = FrameNode::CreateFrameNode(
        V2::DIVIDER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<DividerPattern>());
    CHECK_NULL_RETURN(dividerNode, nullptr);
    auto dividerProps = dividerNode->GetLayoutProperty<DividerLayoutProperty>();
    CHECK_NULL_RETURN(dividerProps, nullptr);
    dividerProps->UpdateVertical(true);
    dividerProps->UpdateStrokeWidth(dividerWidth);
    dividerProps->UpdateUserDefinedIdealSize(CalcSize(std::nullopt, CalcLength(dividerLength)));
    auto dividerPaintProps = dividerNode->GetPaintProperty<DividerRenderProperty>();
    CHECK_NULL_RETURN(dividerPaintProps, nullptr);
    dividerPaintProps->UpdateDividerColor(color);
    // add divider margin
    MarginProperty margin = {
        .left = CalcLength((space - dividerWidth) / 2),
        .right = CalcLength((space - dividerWidth) / 2),
    };
    dividerProps->UpdateMargin(margin);
    return dividerNode;
}

// alert dialog buttons
RefPtr<FrameNode> DialogPattern::BuildButtons(
    const std::vector<ButtonInfo>& buttons, const DialogButtonDirection& direction)
{
    auto Id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> container;
    bool isVertical;
    if (direction == DialogButtonDirection::HORIZONTAL ||
        (direction == DialogButtonDirection::AUTO && buttons.size() == TWO_BUTTON_MODE)) {
        // use horizontal layout
        isVertical = false;
        container = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, Id, AceType::MakeRefPtr<LinearLayoutPattern>(false));
        CHECK_NULL_RETURN(container, nullptr);
        auto layoutProps = container->GetLayoutProperty<LinearLayoutProperty>();
        layoutProps->UpdateMainAxisAlign(FlexAlign::SPACE_BETWEEN);
        layoutProps->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);
    } else {
        // use vertical layout
        isVertical = true;
        container = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, Id, AceType::MakeRefPtr<LinearLayoutPattern>(true));
        auto layoutProps = container->GetLayoutProperty<LinearLayoutProperty>();
        layoutProps->UpdateCrossAxisAlign(FlexAlign::STRETCH);
        layoutProps->UpdateMeasureType(MeasureType::MATCH_PARENT_CROSS_AXIS);
    }
    CHECK_NULL_RETURN(container, nullptr);
    // set action's padding
    PaddingProperty actionPadding;
    if (buttons.size() == ONE_BUTTON_MODE || isVertical) {
        actionPadding.left = CalcLength(dialogTheme_->GetSingleButtonPaddingStart());
        actionPadding.right = CalcLength(dialogTheme_->GetSingleButtonPaddingEnd());
    } else {
        actionPadding.left = CalcLength(dialogTheme_->GetMutiButtonPaddingStart());
        actionPadding.right = CalcLength(dialogTheme_->GetMutiButtonPaddingEnd());
    }
    auto padding = dialogTheme_->GetActionsPadding();
    actionPadding.top = CalcLength(dialogTheme_->GetButtonWithContentPadding());
    actionPadding.bottom = CalcLength(dialogTheme_->GetButtonPaddingBottom());
    container->GetLayoutProperty()->UpdatePadding(actionPadding);
    AddButtonAndDivider(buttons, container, isVertical);
    container->MarkModifyDone();
    buttonContainer_ = container;
    return container;
}

void DialogPattern::AddButtonAndDivider(
    const std::vector<ButtonInfo>& buttons, const RefPtr<NG::FrameNode>& container, bool isVertical)
{
    auto dividerLength = dialogTheme_->GetDividerLength();
    auto dividerWidth = dialogTheme_->GetDividerBetweenButtonWidth_();
    auto dividerColor = dialogTheme_->GetDividerColor();
    auto buttonSpace = dialogTheme_->GetMutiButtonPaddingHorizontal();
    auto length = buttons.size();
    for (size_t i = 0; i < length; ++i) {
        if (i != 0 && !isVertical && !isSuitableForElderly_) {
            auto dividerNode = CreateDivider(dividerLength, dividerWidth, dividerColor, buttonSpace);
            CHECK_NULL_VOID(dividerNode);
            container->AddChild(dividerNode);
        }
        auto buttonNode = CreateButton(buttons[i], i, false, isVertical, length);
        CHECK_NULL_VOID(buttonNode);
        auto buttonPattern = buttonNode->GetPattern<ButtonPattern>();
        CHECK_NULL_VOID(buttonPattern);
        buttonPattern->SetSkipColorConfigurationUpdate();
        buttonNode->MountToParent(container);
        buttonNode->MarkModifyDone();
    }
}

RefPtr<FrameNode> DialogPattern::CreateButtonText(const std::string& text, const std::string& colorStr)
{
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textNode, nullptr);
    textNode->GetOrCreateFocusHub()->SetFocusable(true);
    auto textProps = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textProps, nullptr);
    if (isSuitableForElderly_) {
        MarginProperty margin;
        margin.top = CalcLength(8.0_vp);
        margin.bottom = CalcLength(8.0_vp);
        textProps->UpdateMargin(margin);
    }
    textProps->UpdateContent(text);
    textProps->UpdateFontWeight(FontWeight::MEDIUM);
    textProps->UpdateMaxLines(1);
    textProps->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    Dimension buttonTextSize =
        dialogTheme_->GetButtonTextSize().IsValid() ? dialogTheme_->GetButtonTextSize() : DIALOG_BUTTON_TEXT_SIZE;
    textProps->UpdateFontSize(buttonTextSize);
    if (isSuitableForElderly_ && isLandspace_ && !isThreeButtonsDialog_) {
        textProps->UpdateFontSize(LANDSCAPE_BUTTON_TEXT_FONT_SIZE);
    }
    if (isNeedToVp_) {
        Dimension defaultFontSize = Dimension(buttonTextSize.Value(), DimensionUnit::VP);
        textProps->UpdateFontSize(defaultFontSize);
    }
    // update text color
    Color color;
    if (Color::ParseColorString(colorStr, color)) {
        textProps->UpdateTextColor(color);
    } else {
        textProps->UpdateTextColor(DEFAULT_BUTTON_COLOR);
    }
    return textNode;
}

RefPtr<FrameNode> DialogPattern::BuildSheetItem(const ActionSheetInfo& item)
{
    // ListItem -> Row -> title + icon
    auto Id = ElementRegister::GetInstance()->MakeUniqueId();
    RefPtr<FrameNode> itemNode = FrameNode::CreateFrameNode(
        V2::LIST_ITEM_ETS_TAG, Id, AceType::MakeRefPtr<ListItemPattern>(nullptr, V2::ListItemStyle::NONE));
    CHECK_NULL_RETURN(itemNode, nullptr);

    // update sheet row flex align
    auto rowId = ElementRegister::GetInstance()->MakeUniqueId();
    auto itemRow = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, rowId, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    CHECK_NULL_RETURN(itemRow, nullptr);
    auto layoutProps = itemRow->GetLayoutProperty<LinearLayoutProperty>();
    layoutProps->UpdateMainAxisAlign(FlexAlign::FLEX_START);
    layoutProps->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);

    // mount icon
    if (!item.icon.empty()) {
        auto iconNode = BuildSheetInfoIcon(item.icon);
        iconNode->MountToParent(itemRow);
        iconNode->MarkModifyDone();
    }

    // mount title
    if (!item.title.empty()) {
        auto titleNode = BuildSheetInfoTitle(item.title);
        titleNode->MountToParent(itemRow);
        titleNode->MarkModifyDone();
    }

    // set sheetItem action
    auto hub = itemRow->GetOrCreateGestureEventHub();
    if (item.action) {
        hub->AddClickEvent(item.action);
        auto recordEvent = [weak = WeakClaim(this), title = item.title](GestureEvent& info) {
            if (!Recorder::EventRecorder::Get().IsComponentRecordEnable()) {
                return;
            }
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            Recorder::EventParamsBuilder builder;
            builder.SetEventType(Recorder::EventType::DIALOG_SELECT)
                .SetText(title)
                .SetExtra(Recorder::KEY_TITLE, pattern->title_)
                .SetExtra(Recorder::KEY_SUB_TITLE, pattern->subtitle_);
            Recorder::EventRecorder::Get().OnEvent(std::move(builder));
        };
        auto recordEventPtr = MakeRefPtr<ClickEvent>(std::move(recordEvent));
        hub->AddClickEvent(recordEventPtr);
    }

    // close dialog when clicked
    BindCloseCallBack(hub, SHEET_INFO_IDX);
    itemRow->MountToParent(itemNode);
    return itemNode;
}

RefPtr<FrameNode> DialogPattern::BuildSheetInfoTitle(const std::string& title)
{
    auto titleId = ElementRegister::GetInstance()->MakeUniqueId();
    auto titleNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, titleId, AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(titleNode, nullptr);
    // update text style
    auto style = dialogTheme_->GetContentTextStyle();
    auto props = titleNode->GetLayoutProperty<TextLayoutProperty>();
    props->UpdateContent(title);
    props->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    props->UpdateAdaptMaxFontSize(style.GetFontSize());
    props->UpdateAdaptMinFontSize(dialogTheme_->GetTitleMinFontSize());
    props->UpdateMaxLines(style.GetMaxLines());
    props->UpdateFlexGrow(1.0f);
    props->UpdateFlexShrink(1.0f);
    return titleNode;
}

RefPtr<FrameNode> DialogPattern::BuildSheetInfoIcon(const std::string& icon)
{
    auto iconId = ElementRegister::GetInstance()->MakeUniqueId();
    auto iconNode = FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, iconId, AceType::MakeRefPtr<ImagePattern>());
    CHECK_NULL_RETURN(iconNode, nullptr);
    // add image margin
    MarginProperty margin = {
        .left = CalcLength(SHEET_IMAGE_MARGIN),
        .right = CalcLength(SHEET_IMAGE_MARGIN),
        .top = CalcLength(SHEET_IMAGE_MARGIN),
        .bottom = CalcLength(SHEET_IMAGE_MARGIN),
    };
    auto iconProps = iconNode->GetLayoutProperty<ImageLayoutProperty>();
    iconProps->UpdateMargin(margin);
    auto imageSrc = ImageSourceInfo(icon);
    iconProps->UpdateImageSourceInfo(imageSrc);
    iconProps->UpdateUserDefinedIdealSize(CalcSize(SHEET_IMAGE_SIZE, SHEET_IMAGE_SIZE));
    return iconNode;
}

RefPtr<FrameNode> DialogPattern::BuildSheet(const std::vector<ActionSheetInfo>& sheets)
{
    auto listId = ElementRegister::GetInstance()->MakeUniqueId();
    auto list = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, listId, AceType::MakeRefPtr<ListPattern>());
    CHECK_NULL_RETURN(list, nullptr);

    // set sheet padding
    CalcLength padding(SHEET_LIST_PADDING.ConvertToPx());
    PaddingProperty sheetPadding = {
        .left = padding,
        .right = padding,
        .top = padding,
        .bottom = padding,
    };
    list->GetLayoutProperty()->UpdatePadding(sheetPadding);
    list->GetPaintProperty<ScrollablePaintProperty>()->UpdateScrollBarMode(DisplayMode::OFF);

    for (auto&& item : sheets) {
        auto itemNode = BuildSheetItem(item);
        CHECK_NULL_RETURN(itemNode, nullptr);
        list->AddChild(itemNode);
    }

    // set list divider
    auto divider = V2::ItemDivider {
        .strokeWidth = SHEET_DIVIDER_WIDTH,
        .color = Color::GRAY,
    };
    auto props = list->GetLayoutProperty<ListLayoutProperty>();
    props->UpdateDivider(divider);
    props->UpdateListDirection(Axis::VERTICAL);
    return list;
}

RefPtr<FrameNode> DialogPattern::BuildMenu(const std::vector<ButtonInfo>& buttons, bool hasTitle)
{
    auto menu = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), MakeRefPtr<LinearLayoutPattern>(true));
    menuNode_ = menu;
    // column -> button
    for (size_t i = 0; i < buttons.size(); ++i) {
        RefPtr<FrameNode> button;
        uint32_t val = buttons.size() > 0 ? buttons.size() - 1 : 0;
        if (i != val) {
            button = CreateButton(buttons[i], i);
        } else {
            button = CreateButton(buttons[i], i, true);
        }
        CHECK_NULL_RETURN(button, nullptr);
        auto props = DynamicCast<FrameNode>(button)->GetLayoutProperty();
        auto buttonRow = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
        CHECK_NULL_RETURN(buttonRow, nullptr);
        auto buttonRowProps = buttonRow->GetLayoutProperty<LinearLayoutProperty>();
        CHECK_NULL_RETURN(buttonRowProps, nullptr);
        buttonRowProps->UpdateMainAxisAlign(FlexAlign::FLEX_START);
        buttonRowProps->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);

        button->MountToParent(buttonRow);
        button->MarkModifyDone();
        menu->AddChild(buttonRow);
    }
    auto menuProps = menu->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(menuProps, nullptr);
    PaddingProperty menuPadding;
    if (!hasTitle) {
        menuPadding.top = CalcLength(dialogTheme_->GetContentAdjustPadding().Top());
    }
    menuPadding.left = CalcLength(dialogTheme_->GetDefaultPadding().Left());
    menuPadding.right = CalcLength(dialogTheme_->GetDefaultPadding().Right());
    menuPadding.bottom = CalcLength(dialogTheme_->GetButtonPaddingBottom());
    menuProps->UpdatePadding(menuPadding);
    return menu;
}

void DialogPattern::RegisterOnKeyEvent(const RefPtr<FocusHub>& focusHub)
{
    auto onKeyEvent = [wp = WeakClaim(this)](const KeyEvent& event) -> bool {
        auto pattern = wp.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->OnKeyEvent(event);
    };
    focusHub->SetOnKeyEventInternal(std::move(onKeyEvent));
}

bool DialogPattern::OnKeyEvent(const KeyEvent& event)
{
    if (event.action != KeyAction::DOWN) {
        return false;
    }
    return false;
}

void DialogPattern::InitFocusEvent(const RefPtr<FocusHub>& focusHub)
{
    auto onFocus = [wp = WeakClaim(this)]() {
        auto pattern = wp.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleFocusEvent();
    };
    focusHub->SetOnFocusInternal(std::move(onFocus));

    auto onBlur = [wp = WeakClaim(this)]() {
        auto pattern = wp.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleBlurEvent();
    };
    focusHub->SetOnBlurInternal(std::move(onBlur));
}

void DialogPattern::HandleBlurEvent()
{
    CHECK_NULL_VOID(contentRenderContext_ && Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TWELVE));
    auto defaultShadowOff = static_cast<ShadowStyle>(dialogTheme_->GetDefaultShadowOff());
    contentRenderContext_->UpdateBackShadow(dialogProperties_.shadow.value_or(Shadow::CreateShadow(defaultShadowOff)));
}

void DialogPattern::HandleFocusEvent()
{
    CHECK_NULL_VOID(contentRenderContext_ && Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TWELVE));
    auto defaultShadowOn = static_cast<ShadowStyle>(dialogTheme_->GetDefaultShadowOn());
    contentRenderContext_->UpdateBackShadow(dialogProperties_.shadow.value_or(Shadow::CreateShadow(defaultShadowOn)));
}

// XTS inspector
void DialogPattern::ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    /* no fixed attr below, just return */
    if (filter.IsFastFilter()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (host->GetTag() == V2::ALERT_DIALOG_ETS_TAG || host->GetTag() == V2::ACTION_SHEET_DIALOG_ETS_TAG) {
        json->PutExtAttr("title", title_.c_str(), filter);
        json->PutExtAttr("subtitle", subtitle_.c_str(), filter);
        json->PutExtAttr("message", message_.c_str(), filter);
    }
}

void DialogPattern::OnColorConfigurationUpdate()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
    CHECK_NULL_VOID(context);
    auto dialogTheme = context->GetTheme<DialogTheme>();
    CHECK_NULL_VOID(dialogTheme);
    dialogTheme_ = dialogTheme;
    UpdateWrapperBackgroundStyle(host, dialogTheme);
    UpdateButtonsProperty();
    OnModifyDone();
    host->MarkDirtyNode();
}

void DialogPattern::UpdateAlignmentAndOffset()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto props = host->GetLayoutProperty<DialogLayoutProperty>();
    CHECK_NULL_VOID(props);
    auto dialogProp = GetDialogProperties();
    props->UpdateDialogAlignment(dialogProp.alignment);
    props->UpdateDialogOffset(dialogProp.offset);
}

void DialogPattern::OnLanguageConfigurationUpdate()
{
    CHECK_NULL_VOID(dialogProperties_.onLanguageChange);
    dialogProperties_.onLanguageChange(dialogProperties_);
    UpdateAlignmentAndOffset();
    if (!dialogProperties_.title.empty() && contentNodeMap_.find(DialogContentNode::TITLE) != contentNodeMap_.end()) {
        UpdateNodeContent(contentNodeMap_[DialogContentNode::TITLE], dialogProperties_.title);
        title_ = dialogProperties_.title;
    }

    if (!dialogProperties_.subtitle.empty() &&
        contentNodeMap_.find(DialogContentNode::SUBTITLE) != contentNodeMap_.end()) {
        UpdateNodeContent(contentNodeMap_[DialogContentNode::SUBTITLE], dialogProperties_.subtitle);
        subtitle_ = dialogProperties_.subtitle;
    }

    if (!dialogProperties_.content.empty() &&
        contentNodeMap_.find(DialogContentNode::MESSAGE) != contentNodeMap_.end()) {
        UpdateNodeContent(contentNodeMap_[DialogContentNode::MESSAGE], dialogProperties_.content);
        message_ = dialogProperties_.content;
    }

    if (!dialogProperties_.buttons.empty()) {
        UpdateButtonsProperty();
    }

    if (dialogProperties_.type == DialogType::ACTION_SHEET) {
        UpdateSheetIconAndText();
    }

    if (dialogProperties_.shadow.has_value()) {
        contentRenderContext_->UpdateBackShadow(dialogProperties_.shadow.value());
    }

    if (dialogProperties_.borderWidth.has_value() &&
        contentNodeMap_.find(DialogContentNode::BORDERWIDTH) != contentNodeMap_.end()) {
        auto layoutProps = contentNodeMap_[DialogContentNode::BORDERWIDTH]->GetLayoutProperty<LinearLayoutProperty>();
        layoutProps->UpdateBorderWidth(dialogProperties_.borderWidth.value());
        contentRenderContext_->UpdateBorderWidth(dialogProperties_.borderWidth.value());
    }

    if (dialogProperties_.borderColor.has_value()) {
        contentRenderContext_->UpdateBorderColor(dialogProperties_.borderColor.value());
    }

    if (dialogProperties_.borderRadius.has_value()) {
        contentRenderContext_->UpdateBorderRadius(dialogProperties_.borderRadius.value());
    }
}

void DialogPattern::UpdateNodeContent(const RefPtr<FrameNode>& node, std::string& text)
{
    CHECK_NULL_VOID(node);
    auto layoutProps = AceType::DynamicCast<TextLayoutProperty>(node->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProps);
    layoutProps->UpdateContent(text);
    node->MarkModifyDone();
}

void DialogPattern::UpdateSheetIconAndText()
{
    if (dialogProperties_.sheetsInfo.empty()) {
        return;
    }

    auto sheetContainer = contentNodeMap_[DialogContentNode::SHEET];
    CHECK_NULL_VOID(sheetContainer);
    int32_t sheetIndex = 0;
    for (const auto& sheet : sheetContainer->GetChildren()) {
        auto itemRow = DynamicCast<FrameNode>(sheet->GetFirstChild());
        CHECK_NULL_VOID(itemRow);

        auto sheetInfo = dialogProperties_.sheetsInfo.at(sheetIndex);
        if (!sheetInfo.icon.empty()) {
            auto iconNode = DynamicCast<FrameNode>(itemRow->GetFirstChild());
            CHECK_NULL_VOID(iconNode);
            auto iconProps = iconNode->GetLayoutProperty<ImageLayoutProperty>();
            CHECK_NULL_VOID(iconProps);
            iconProps->UpdateImageSourceInfo(ImageSourceInfo(sheetInfo.icon));
            iconNode->MarkModifyDone();
        }
        if (!sheetInfo.title.empty()) {
            auto titleNode = DynamicCast<FrameNode>(itemRow->GetLastChild());
            CHECK_NULL_VOID(titleNode);
            auto titleProps = titleNode->GetLayoutProperty<TextLayoutProperty>();
            CHECK_NULL_VOID(titleProps);
            titleProps->UpdateContent(sheetInfo.title);
            titleNode->MarkModifyDone();
        }
        ++sheetIndex;
    }
}

void DialogPattern::UpdateButtonsProperty()
{
    CHECK_NULL_VOID(buttonContainer_);
    int32_t btnIndex = 0;
    isFirstDefaultFocus_ = true;
    for (const auto& buttonNode : buttonContainer_->GetChildren()) {
        if (buttonNode->GetTag() != V2::BUTTON_ETS_TAG) {
            continue;
        }
        auto buttonFrameNode = DynamicCast<FrameNode>(buttonNode);
        CHECK_NULL_VOID(buttonFrameNode);
        auto pattern = buttonFrameNode->GetPattern<ButtonPattern>();
        CHECK_NULL_VOID(pattern);
        pattern->SetSkipColorConfigurationUpdate();
        // parse button text color and background color
        std::string textColorStr;
        std::optional<Color> bgColor;
        ParseButtonFontColorAndBgColor(dialogProperties_.buttons[btnIndex], textColorStr, bgColor);
        // update background color
        auto renderContext = buttonFrameNode->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        renderContext->UpdateBackgroundColor(bgColor.value());
        auto buttonTextNode = DynamicCast<FrameNode>(buttonFrameNode->GetFirstChild());
        CHECK_NULL_VOID(buttonTextNode);
        auto buttonTextLayoutProperty = buttonTextNode->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_VOID(buttonTextLayoutProperty);
        Color textColor;
        Color::ParseColorString(textColorStr, textColor);
        buttonTextLayoutProperty->UpdateContent(dialogProperties_.buttons[btnIndex].text);
        buttonTextLayoutProperty->UpdateTextColor(textColor);
        buttonTextNode->MarkModifyDone();
        buttonTextNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        ++btnIndex;
    }
}

void DialogPattern::UpdatePropertyForElderly(const std::vector<ButtonInfo>& buttons)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto windowManager = pipeline->GetWindowManager();
    if (GreatOrEqual(pipeline->GetFontScale(), DIALOG_MIN_FONT_SCALE_FOR_AGING)) {
        fontScaleForElderly_ = pipeline->GetFontScale();
        if (pipeline->GetRootHeight() < DIALOG_LANDSCAPE_BOUNDARY.ConvertToPx() &&
            windowManager->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY) {
            isNeedToVp_ = true;
            return;
        }
        isSuitableForElderly_ = true;
        deviceOrientation_ = SystemProperties::GetDeviceOrientation();
        if (deviceOrientation_ == DeviceOrientation::LANDSCAPE &&
            GreatOrEqual(pipeline->GetFontScale(), LANDSCAPE_MAX_FONT_SCALE_FOR_AGING)) {
            isLandspace_ = true;
        }
        if (buttons.size() >= THREE_BUTON_DIALOG_BUTTON_NUMBER) {
            isThreeButtonsDialog_ = true;
        }
    }
    if (isThreeButtonsDialog_ && deviceOrientation_ == DeviceOrientation::LANDSCAPE) {
        isNeedToVp_ = true;
    }
    std::cout << pipeline->GetFontScale() << std::endl;
}

bool DialogPattern::NeedsButtonDirectionChange(const std::vector<ButtonInfo>& buttons)
{
    CHECK_NULL_RETURN(buttonContainer_, false);
    if (buttons.size() == ONE_BUTTON_MODE || buttonContainer_->GetTag() != V2::ROW_ETS_TAG) {
        return false;
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto props = host->GetLayoutProperty<DialogLayoutProperty>();
    CHECK_NULL_RETURN(props, false);
    auto buttonLayoutConstraint = props->GetLayoutConstraint();
    host->Measure(buttonLayoutConstraint);
    const auto& children = buttonContainer_->GetChildren();
    for (const auto& child : children) {
        if (child->GetTag() == V2::BUTTON_ETS_TAG) {
            auto buttonNode = AceType::DynamicCast<FrameNode>(child);
            CHECK_NULL_RETURN(buttonNode, false);
            auto buttonTextNode = DynamicCast<FrameNode>(buttonNode->GetFirstChild());
            CHECK_NULL_RETURN(buttonTextNode, false);
            auto textGeometryNode = buttonTextNode->GetGeometryNode();
            CHECK_NULL_RETURN(textGeometryNode, false);
            auto textFarmeSize = textGeometryNode->GetFrameSize();
            auto textPattern = buttonTextNode->GetPattern<TextPattern>();
            CHECK_NULL_RETURN(textPattern, false);
            auto textDisplay = textPattern->GetTextForDisplay();
            auto textProps = buttonTextNode->GetLayoutProperty<TextLayoutProperty>();
            CHECK_NULL_RETURN(textProps, false);
            Dimension buttonTextSize = textProps->GetFontSize().value_or(dialogTheme_->GetButtonTextSize());
            MeasureContext measureContext;
            measureContext.textContent = textDisplay;
            measureContext.fontSize = buttonTextSize;
            auto fontweight = StringUtils::FontWeightToString(FontWeight::MEDIUM);
            measureContext.fontWeight = fontweight;
            Size measureSize = MeasureUtil::MeasureTextSize(measureContext);
            if (GreatNotEqual(measureSize.Width(), textFarmeSize.Width())) {
                return true;
            }
        }
    }
    return false;
}

void DialogPattern::UpdateDeviceOrientation(const DeviceOrientation& deviceOrientation)
{
    if (deviceOrientation_ != deviceOrientation) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        if (deviceOrientation == DeviceOrientation::LANDSCAPE) {
            if (!isThreeButtonsDialog_) {
                UpdateLandSpaceTextFontSizeForElderly(true);
            }
        }
        if (deviceOrientation == DeviceOrientation::PORTRAIT) {
            UpdateLandSpaceTextFontSizeForElderly(false);
        }
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        buttonContainer_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        deviceOrientation_ = deviceOrientation;
    }
}

void DialogPattern::UpdateTitleTextFontSizeForElderly(bool isLandSpace)
{
    CHECK_NULL_VOID(titleContainer_);
    if (titleContainer_->GetTag() == V2::COLUMN_ETS_TAG) {
        auto children = titleContainer_->GetChildren();
        for (const auto& child : children) {
            int index = 0;
            auto textNode = AceType::DynamicCast<FrameNode>(child->GetChildAtIndex(START_CHILD_INDEX));
            CHECK_NULL_VOID(textNode);
            auto titleProp = AceType::DynamicCast<TextLayoutProperty>(textNode->GetLayoutProperty());
            CHECK_NULL_VOID(titleProp);
            auto fontSize = (index == 0) ? LANDSCAPE_MAINTitle_FONT_SIZE : LANDSCAPE_SUBTITLE_FONT_SIZE;
            if (isLandSpace || GreatOrEqual(fontScaleForElderly_, LANDSCAPE_MAX_FONT_SCALE_FOR_AGING)) {
                titleProp->UpdateFontSize(fontSize);
                titleProp->UpdateAdaptMaxFontSize(fontSize);
                titleProp->UpdateAdaptMinFontSize(fontSize);
                return;
            }
            Dimension titlefontSize;
            if (index == 0) {
                titlefontSize = dialogTheme_->GetTitleTextStyle().GetFontSize();
            } else {
                titlefontSize = dialogTheme_->GetSubTitleTextStyle().GetFontSize();
            }
            titleProp->UpdateFontSize(titlefontSize);
            titleProp->UpdateAdaptMaxFontSize(titlefontSize);
            titleProp->UpdateAdaptMinFontSize(titlefontSize);
        }
    } else {
        auto textNode = AceType::DynamicCast<FrameNode>(titleContainer_->GetChildAtIndex(START_CHILD_INDEX));
        CHECK_NULL_VOID(textNode);
        auto titleProp = AceType::DynamicCast<TextLayoutProperty>(textNode->GetLayoutProperty());
        CHECK_NULL_VOID(titleProp);
        if (isLandSpace) {
            titleProp->UpdateFontSize(LANDSCAPE_MAINTitle_FONT_SIZE);
            titleProp->UpdateAdaptMaxFontSize(LANDSCAPE_MAINTitle_FONT_SIZE);
            titleProp->UpdateAdaptMinFontSize(LANDSCAPE_MAINTitle_FONT_SIZE);
        } else {
            auto titleStyle = dialogTheme_->GetTitleTextStyle();
            titleProp->UpdateFontSize(titleStyle.GetFontSize());
            titleProp->UpdateAdaptMaxFontSize(titleStyle.GetFontSize());
            titleProp->UpdateAdaptMinFontSize(titleStyle.GetFontSize());
        }
    }
}

void DialogPattern::UpdateLandSpaceTextFontSizeForElderly(bool isLandSpace)
{
    UpdateTitleTextFontSizeForElderly(isLandSpace);
    CHECK_NULL_VOID(contentNodeMap_[DialogContentNode::MESSAGE]);
    auto contentProp =
        AceType::DynamicCast<TextLayoutProperty>(contentNodeMap_[DialogContentNode::MESSAGE]->GetLayoutProperty());
    CHECK_NULL_VOID(contentProp);
    if (isLandSpace) {
        contentProp->UpdateFontSize(LANDSCAPE_CONTENT_TEXT_FONT_SIZE);
    } else {
        auto contentStyle = dialogTheme_->GetContentTextStyle();
        contentProp->UpdateFontSize(contentStyle.GetFontSize());
    }

    CHECK_NULL_VOID(buttonContainer_);
    const auto& children = buttonContainer_->GetChildren();
    for (const auto& child : children) {
        if (child->GetTag() == V2::BUTTON_ETS_TAG) {
            auto buttonNode = AceType::DynamicCast<FrameNode>(child);
            CHECK_NULL_VOID(buttonNode);
            auto buttonProp = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
            CHECK_NULL_VOID(buttonProp);
            auto buttonTextNode = DynamicCast<FrameNode>(buttonNode->GetFirstChild());
            CHECK_NULL_VOID(buttonTextNode);
            auto titleProp = AceType::DynamicCast<TextLayoutProperty>(buttonTextNode->GetLayoutProperty());
            CHECK_NULL_VOID(titleProp);
            if (isLandSpace) {
                buttonProp->UpdateFontSize(LANDSCAPE_BUTTON_TEXT_FONT_SIZE);
                titleProp->UpdateFontSize(LANDSCAPE_BUTTON_TEXT_FONT_SIZE);
            } else {
                Dimension buttonTextSize = dialogTheme_->GetButtonTextSize().IsValid()
                                               ? dialogTheme_->GetButtonTextSize()
                                               : DIALOG_BUTTON_TEXT_SIZE;
                buttonProp->UpdateFontSize(buttonTextSize);
                titleProp->UpdateFontSize(buttonTextSize);
            }
        }
    }
}

void DialogPattern::SetButtonEnabled(const RefPtr<FrameNode>& buttonNode, bool enabled)
{
    // set Enabled and Focusable
    auto buttonButtonEvent = buttonNode->GetEventHub<ButtonEventHub>();
    CHECK_NULL_VOID(buttonButtonEvent);
    buttonButtonEvent->SetEnabled(enabled);
    buttonNode->GetOrCreateFocusHub()->SetFocusable(enabled);
}

void DialogPattern::UpdateWrapperBackgroundStyle(const RefPtr<FrameNode>& host, const RefPtr<DialogTheme>& dialogTheme)
{
    auto col = DynamicCast<FrameNode>(host->GetChildAtIndex(START_CHILD_INDEX));
    CHECK_NULL_VOID(col);
    auto colRenderContext = col->GetRenderContext();
    CHECK_NULL_VOID(colRenderContext);
    if (!dialogProperties_.customStyle && !dialogProperties_.backgroundColor.has_value() &&
        (Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN) || !colRenderContext->IsUniRenderEnabled() ||
            !dialogProperties_.isSysBlurStyle)) {
        colRenderContext->UpdateBackgroundColor(dialogTheme->GetBackgroundColor());
    }
    if (colRenderContext->GetBackBlurStyle().has_value()) {
        colRenderContext->UpdateBackBlurStyle(colRenderContext->GetBackBlurStyle());
    }
}

void DialogPattern::DumpInfo()
{
    DumpLog::GetInstance().AddDesc("Type: " + DialogTypeUtils::ConvertDialogTypeToString(dialogProperties_.type));
    if (!dialogProperties_.title.empty()) {
        DumpLog::GetInstance().AddDesc("Title: " + dialogProperties_.title);
    }
    if (!dialogProperties_.subtitle.empty()) {
        DumpLog::GetInstance().AddDesc("Subtitle: " + dialogProperties_.subtitle);
    }
    if (!dialogProperties_.content.empty()) {
        DumpLog::GetInstance().AddDesc("Content: " + dialogProperties_.content);
    }
    DumpLog::GetInstance().AddDesc(
        "DialogButtonDirection: " +
        DialogButtonDirectionUtils::ConvertDialogButtonDirectionToString(dialogProperties_.buttonDirection));
    if (dialogProperties_.width.has_value()) {
        DumpLog::GetInstance().AddDesc("Width: " + dialogProperties_.width.value().ToString());
    }
    if (dialogProperties_.height.has_value()) {
        DumpLog::GetInstance().AddDesc("Height: " + dialogProperties_.height.value().ToString());
    }
    if (dialogProperties_.backgroundBlurStyle.has_value()) {
        DumpLog::GetInstance().AddDesc(
            "BackgroundBlurStyle: " + std::to_string(dialogProperties_.backgroundBlurStyle.value()));
    }
    if (dialogProperties_.borderWidth.has_value()) {
        DumpLog::GetInstance().AddDesc("BorderWidth: " + dialogProperties_.borderWidth.value().ToString());
    }
    if (dialogProperties_.borderColor.has_value()) {
        DumpLog::GetInstance().AddDesc("BorderColor: " + dialogProperties_.borderColor.value().ToString());
    }
    if (dialogProperties_.backgroundColor.has_value()) {
        DumpLog::GetInstance().AddDesc("BackgroundColor: " + dialogProperties_.backgroundColor.value().ToString());
    }
    if (dialogProperties_.borderRadius.has_value()) {
        DumpLog::GetInstance().AddDesc("BorderRadius: " + dialogProperties_.borderRadius.value().ToString());
    }
    DumpBoolProperty();
    DumpObjectProperty();
}

void DialogPattern::DumpBoolProperty()
{
    DumpLog::GetInstance().AddDesc("AutoCancel: " + GetBoolStr(dialogProperties_.autoCancel));
    DumpLog::GetInstance().AddDesc("CustomStyle: " + GetBoolStr(dialogProperties_.customStyle));
    DumpLog::GetInstance().AddDesc("IsMenu: " + GetBoolStr(dialogProperties_.isMenu));
    DumpLog::GetInstance().AddDesc("IsMask: " + GetBoolStr(dialogProperties_.isMask));
    DumpLog::GetInstance().AddDesc("IsModal: " + GetBoolStr(dialogProperties_.isModal));
    DumpLog::GetInstance().AddDesc("IsScenceBoardDialog: " + GetBoolStr(dialogProperties_.isScenceBoardDialog));
    DumpLog::GetInstance().AddDesc("IsSysBlurStyle: " + GetBoolStr(dialogProperties_.isSysBlurStyle));
    DumpLog::GetInstance().AddDesc("IsShowInSubWindow: " + GetBoolStr(dialogProperties_.isShowInSubWindow));
}

void DialogPattern::DumpObjectProperty()
{
    DumpLog::GetInstance().AddDesc(
        "Alignment: " + DialogAlignmentUtils::ConvertDialogAlignmentToString(dialogProperties_.alignment));
    DumpLog::GetInstance().AddDesc("Offset: { dx: " + dialogProperties_.offset.GetX().ToString() +
                                   " dy: " + dialogProperties_.offset.GetY().ToString() + " }");
    if (dialogProperties_.buttons.size() > 0) {
        std::stringstream butonInfoSteam;
        butonInfoSteam << "Buttons: [";
        for (auto buttonInfo : dialogProperties_.buttons) {
            butonInfoSteam << "{ text: " << buttonInfo.text << " , color: " << buttonInfo.textColor << " }, ";
        }
        butonInfoSteam << "]";
        DumpLog::GetInstance().AddDesc(butonInfoSteam.str());
    }
    if (dialogProperties_.shadow.has_value()) {
        auto shadow = dialogProperties_.shadow.value();
        std::stringstream butonInfoSteam;
        static const int32_t precision = 2;
        butonInfoSteam << "Shadow: {";
        butonInfoSteam << " radius:" << std::fixed << std::setprecision(precision) << shadow.GetBlurRadius();
        butonInfoSteam << " style:" << std::to_string(static_cast<int32_t>(shadow.GetStyle()));
        butonInfoSteam << " type:" << std::to_string(static_cast<int32_t>(shadow.GetShadowType()));
        butonInfoSteam << " fill:" << GetBoolStr(shadow.GetIsFilled());
        butonInfoSteam << " offset:" << shadow.GetOffset().ToString();
        butonInfoSteam << " }";
        DumpLog::GetInstance().AddDesc(butonInfoSteam.str());
    }
    if (dialogProperties_.maskColor.has_value()) {
        DumpLog::GetInstance().AddDesc("MaskColor: " + dialogProperties_.maskColor.value().ToString());
    }
    if (dialogProperties_.maskRect.has_value()) {
        DumpLog::GetInstance().AddDesc("MaskRect: " + dialogProperties_.maskRect.value().ToString());
    }
}
void DialogPattern::OnWindowSizeChanged(int32_t width, int32_t height, WindowSizeChangeReason type)
{
    if (isFoldStatusChanged_ || type == WindowSizeChangeReason::ROTATION || type == WindowSizeChangeReason::RESIZE) {
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        InitHostWindowRect();
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        isFoldStatusChanged_ = false;
    }
}

void DialogPattern::InitHostWindowRect()
{
    if (!dialogProperties_.isShowInSubWindow) {
        isUIExtensionSubWindow_ = false;
        hostWindowRect_.Reset();
        return;
    }

    auto currentId = Container::CurrentId();
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    if (container->IsSubContainer()) {
        currentId = SubwindowManager::GetInstance()->GetParentContainerId(currentId);
        container = AceEngine::Get().GetContainer(currentId);
        CHECK_NULL_VOID(container);
    }

    if (container->IsUIExtensionWindow()) {
        isUIExtensionSubWindow_ = true;
        auto subwindow = SubwindowManager::GetInstance()->GetSubwindow(currentId);
        CHECK_NULL_VOID(subwindow);
        auto rect = subwindow->GetUIExtensionHostWindowRect();
        hostWindowRect_ = RectF(rect.Left(), rect.Top(), rect.Width(), rect.Height());
    }
}
} // namespace OHOS::Ace::NG
