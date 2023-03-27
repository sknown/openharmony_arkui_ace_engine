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
#include "core/components_ng/pattern/picker/datepicker_dialog_view.h"

#include <utility>

#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components/theme/icon_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/calendar/calendar_paint_property.h"
#include "core/components_ng/pattern/dialog/dialog_view.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/picker/date_time_animation_controller.h"
#include "core/components_ng/pattern/picker/datepicker_pattern.h"
#include "core/components_ng/pattern/picker/datepicker_row_layout_property.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
const uint32_t OPTION_COUNT_PHONE_LANDSCAPE = 3;
const int32_t MARGIN_HALF = 2;
constexpr double MONTHDAYS_WIDTH_PERCENT_ONE = 0.4285;
constexpr double TIME_WIDTH_PERCENT_ONE = 0.5714;
constexpr double MONTHDAYS_WIDTH_PERCENT_TWO = 0.3636;
constexpr double TIME_WIDTH_PERCENT_TWO = 0.6363;
} // namespace
bool DatePickerDialogView::switchFlag_ = false;

RefPtr<FrameNode> DatePickerDialogView::Show(const DialogProperties& dialogProperties,
    const DatePickerSettingData& settingData,
    std::map<std::string, NG::DialogEvent> dialogEvent,
    std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent)
{
    auto contentColumn = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto pickerStack = CreateStackNode();
    auto dateNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto monthDaysNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto dateNode = CreateDateNode(
        dateNodeId, settingData.datePickerProperty, settingData.properties, settingData.isLunar, false);
    ViewStackProcessor::GetInstance()->Push(dateNode);
    dateNode->MountToParent(pickerStack);

    // create title node and bind title text id to date picker, then mark picker node modify done
    auto buttonTitleNode = CreateTitleButtonNode(dateNode);
    CHECK_NULL_RETURN(buttonTitleNode, nullptr);
    buttonTitleNode->MountToParent(contentColumn);
    RefPtr<FrameNode> acceptNode = dateNode;
    if (settingData.showTime) {
        switchFlag_ = false;
        auto pickerRow = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
        CHECK_NULL_RETURN(pickerRow, nullptr);
        auto layoutProperty = dateNode->GetLayoutProperty<LayoutProperty>();
        CHECK_NULL_RETURN(layoutProperty, nullptr);
        layoutProperty->UpdateVisibility(VisibleType::INVISIBLE);
        auto pickerPattern = dateNode->GetPattern<DatePickerPattern>();
        CHECK_NULL_RETURN(pickerPattern, nullptr);
        auto monthDaysNode = CreateDateNode(
            monthDaysNodeId, settingData.datePickerProperty, settingData.properties, settingData.isLunar, true);
        auto monthDaysPickerPattern = monthDaysNode->GetPattern<DatePickerPattern>();
        CHECK_NULL_RETURN(monthDaysPickerPattern, nullptr);
        monthDaysPickerPattern->SetTitleId(pickerPattern->GetTitleId());
        auto monthDaysLayoutProperty = monthDaysNode->GetLayoutProperty();
        CHECK_NULL_RETURN(monthDaysLayoutProperty, nullptr);
        monthDaysLayoutProperty->UpdateUserDefinedIdealSize(CalcSize(NG::CalcLength(
            Dimension(settingData.useMilitary ? MONTHDAYS_WIDTH_PERCENT_ONE : MONTHDAYS_WIDTH_PERCENT_TWO,
            DimensionUnit::PERCENT)), std::nullopt));
        monthDaysNode->MarkModifyDone();
        monthDaysNode->MountToParent(pickerRow);
        auto timeNode = CreateTimeNode(settingData.timePickerProperty, settingData.properties, settingData.useMilitary);
        auto timeLayoutProperty = timeNode->GetLayoutProperty();
        CHECK_NULL_RETURN(timeLayoutProperty, nullptr);
        timeLayoutProperty->UpdateUserDefinedIdealSize(CalcSize(NG::CalcLength(
            Dimension(settingData.useMilitary ? TIME_WIDTH_PERCENT_ONE : TIME_WIDTH_PERCENT_TWO,
            DimensionUnit::PERCENT)), std::nullopt));
        timeNode->MarkModifyDone();
        timeNode->MountToParent(pickerRow);
        pickerRow->MountToParent(pickerStack);

        CreateTitleIconNode(buttonTitleNode);
        buttonTitleNode->MarkModifyDone();
        RefPtr<DateTimeAnimationController> animationController = AceType::MakeRefPtr<DateTimeAnimationController>();
        auto titleSwitchEvent = [contentColumn, pickerStack, animationController]() {
            // switch  picker page.
            auto pickerRow = pickerStack->GetLastChild();
            CHECK_NULL_VOID(pickerRow);
            auto dateNode = AceType::DynamicCast<FrameNode>(pickerStack->GetChildAtIndex(0));
            CHECK_NULL_VOID(dateNode);
            auto datePickerPattern = dateNode->GetPattern<DatePickerPattern>();
            CHECK_NULL_VOID(datePickerPattern);
            auto monthDaysNode = AceType::DynamicCast<FrameNode>(pickerRow->GetChildAtIndex(0));
            auto timeNode = AceType::DynamicCast<FrameNode>(pickerRow->GetChildAtIndex(1));
            CHECK_NULL_VOID(monthDaysNode);
            CHECK_NULL_VOID(timeNode);
            auto monthDaysPickerPattern = monthDaysNode->GetPattern<DatePickerPattern>();
            CHECK_NULL_VOID(monthDaysPickerPattern);

            PickerDate selectedDate = switchFlag_ ? datePickerPattern->GetCurrentDate() :
                monthDaysPickerPattern->GetCurrentDate();
            SetSelectedDate(switchFlag_ ? monthDaysNode : dateNode, selectedDate);
            switchFlag_ ? monthDaysNode->MarkModifyDone() : dateNode->MarkModifyDone();

            auto contentRow = AceType::DynamicCast<FrameNode>(contentColumn->GetLastChild());
            auto titleRow = AceType::DynamicCast<FrameNode>(contentColumn->GetChildAtIndex(0));
            CHECK_NULL_VOID(titleRow);
            auto spinnerNode = AceType::DynamicCast<FrameNode>(titleRow->GetLastChild());
            CHECK_NULL_VOID(spinnerNode);
            animationController->SetButtonIcon(spinnerNode);
            animationController->SetMonthDays(monthDaysNode);
            animationController->SetDatePicker(dateNode);
            animationController->SetTimePicker(timeNode);
            animationController->SetButtonRow(contentRow);
            switchFlag_ = !switchFlag_;

            animationController->Play(switchFlag_);
        };
        auto switchEvent = [func = titleSwitchEvent]() {
            if (switchFlag_) {
                func();
                return true;
            }
            return false;
        };
        SetDialogSwitchEvent(switchEvent);
        auto titleClickEvent = [func = std::move(titleSwitchEvent)](const GestureEvent& /* info */) {
            func();
        };
        auto titleEventHub = buttonTitleNode->GetOrCreateGestureEventHub();
        auto onClick = AceType::MakeRefPtr<NG::ClickEvent>(std::move(titleClickEvent));
        titleEventHub->AddClickEvent(onClick);
        acceptNode = monthDaysNode;
    }

    auto dateLayoutProperty = dateNode->GetLayoutProperty();
    CHECK_NULL_RETURN(dateLayoutProperty, nullptr);
    dateLayoutProperty->UpdateUserDefinedIdealSize(CalcSize(NG::CalcLength(Dimension(1,
        DimensionUnit::PERCENT)), std::nullopt));
    dateNode->MarkModifyDone();

    ViewStackProcessor::GetInstance()->Finish();
    auto stackLayoutProperty = pickerStack->GetLayoutProperty();
    CHECK_NULL_RETURN(stackLayoutProperty, nullptr);
    stackLayoutProperty->UpdateUserDefinedIdealSize(CalcSize(NG::CalcLength(Dimension(1,
        DimensionUnit::PERCENT)), std::nullopt));
    pickerStack->MountToParent(contentColumn);

    auto dialogNode = DialogView::CreateDialogNode(dialogProperties, contentColumn);
    CHECK_NULL_RETURN(dialogNode, nullptr);

    // build dialog accept and cancel button
    auto changeEvent = dialogEvent["changeId"];
    SetDialogChange(dateNode, std::move(changeEvent));
    if (settingData.showTime) {
        SetDialogChange(acceptNode, std::move(changeEvent));
    }
    auto contentRow = CreateButtonNode(acceptNode, dialogEvent, std::move(dialogCancelEvent));
    CHECK_NULL_RETURN(contentRow, nullptr);
    auto event = [dialogNode](const GestureEvent& /* info */) {
        auto pipeline = PipelineContext::GetCurrentContext();
        auto overlayManager = pipeline->GetOverlayManager();
        overlayManager->CloseDialog(dialogNode);
    };
    for (const auto& child : contentRow->GetChildren()) {
        auto firstChild = AceType::DynamicCast<FrameNode>(child);
        auto gesturHub = firstChild->GetOrCreateGestureEventHub();
        auto onClick = AceType::MakeRefPtr<NG::ClickEvent>(event);
        gesturHub->AddClickEvent(onClick);
    }
    contentRow->AddChild(CreateDividerNode(dateNode), 1);
    auto contentRowLayoutProperty = contentRow->GetLayoutProperty();
    CHECK_NULL_RETURN(contentRowLayoutProperty, nullptr);
    contentRowLayoutProperty->UpdateUserDefinedIdealSize(CalcSize(NG::CalcLength(Dimension(1,
        DimensionUnit::PERCENT)), std::nullopt));
    contentRow->MountToParent(contentColumn);
    dialogNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    return dialogNode;
}

RefPtr<FrameNode> DatePickerDialogView::CreateStackNode()
{
    auto stackId = ElementRegister::GetInstance()->MakeUniqueId();
    return FrameNode::GetOrCreateFrameNode(
        V2::STACK_ETS_TAG, stackId, []() { return AceType::MakeRefPtr<StackPattern>(); });
}

RefPtr<FrameNode> DatePickerDialogView::CreateButtonNode()
{
    auto buttonId = ElementRegister::GetInstance()->MakeUniqueId();
    return FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, buttonId, []() { return AceType::MakeRefPtr<ButtonPattern>(); });
}

RefPtr<FrameNode> DatePickerDialogView::CreateTitleButtonNode(const RefPtr<FrameNode>& dateNode)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto dialogTheme = pipeline->GetTheme<DialogTheme>();
    auto pickerTheme = pipeline->GetTheme<PickerTheme>();
    auto pickerPattern = dateNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_RETURN(pickerPattern, nullptr);
    auto titleRow = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    CHECK_NULL_RETURN(titleRow, nullptr);
    auto layoutProps = titleRow->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(layoutProps, nullptr);
    layoutProps->UpdateMainAxisAlign(FlexAlign::CENTER);
    layoutProps->UpdateCrossAxisAlign(FlexAlign::CENTER);
    layoutProps->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);

    auto buttonTitleNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, pickerPattern->GetButtonTitleId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    auto textTitleNodeId = pickerPattern->GetTitleId();
    auto textTitleNode =
        FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, textTitleNodeId, AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textTitleNode, nullptr);
    auto textLayoutProperty = textTitleNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, nullptr);
    textLayoutProperty->UpdateContent("");
    textLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);

    textLayoutProperty->UpdateTextColor(pickerTheme->GetTitleStyle().GetTextColor());
    textLayoutProperty->UpdateFontSize(pickerTheme->GetTitleStyle().GetFontSize());
    textLayoutProperty->UpdateFontWeight(pickerTheme->GetTitleStyle().GetFontWeight());
    textLayoutProperty->UpdateTextOverflow(pickerTheme->GetTitleStyle().GetTextOverflow());
    textLayoutProperty->UpdateMaxLines(pickerTheme->GetTitleStyle().GetMaxLines());
    auto buttonTitleRenderContext = buttonTitleNode->GetRenderContext();
    CHECK_NULL_RETURN(buttonTitleRenderContext, nullptr);
    buttonTitleRenderContext->UpdateBackgroundColor(Color::TRANSPARENT);
    MarginProperty margin;
    margin.left = CalcLength(dialogTheme->GetDividerPadding().Left());
    margin.right = CalcLength(dialogTheme->GetDividerPadding().Right());
    margin.top = CalcLength(dialogTheme->GetDividerHeight() / MARGIN_HALF);
    margin.bottom = CalcLength(dialogTheme->GetDividerHeight() / MARGIN_HALF);
    buttonTitleNode->GetLayoutProperty()->UpdateMargin(margin);
    textTitleNode->MountToParent(buttonTitleNode);
    buttonTitleNode->MountToParent(titleRow);

    return titleRow;
}

void DatePickerDialogView::CreateTitleIconNode(const RefPtr<FrameNode>& titleNode)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto iconTheme = pipeline->GetTheme<IconTheme>();
    auto pickerTheme = pipeline->GetTheme<PickerTheme>();
    auto spinnerNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    CHECK_NULL_VOID(spinnerNode);
    ImageSourceInfo imageSourceInfo;
    auto iconPath = iconTheme->GetIconPath(InternalResource::ResourceId::SPINNER);
    imageSourceInfo.SetSrc(iconPath);
    imageSourceInfo.SetFillColor(pickerTheme->GetTitleStyle().GetTextColor());

    auto spinnerLayoutProperty = spinnerNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(spinnerLayoutProperty);
    spinnerLayoutProperty->UpdateImageSourceInfo(imageSourceInfo);
    CalcSize idealSize = { CalcLength(pickerTheme->GetTitleStyle().GetFontSize()),
        CalcLength(pickerTheme->GetTitleStyle().GetFontSize()) };
    MeasureProperty layoutConstraint;
    layoutConstraint.selfIdealSize = idealSize;
    spinnerLayoutProperty->UpdateCalcLayoutProperty(layoutConstraint);
    spinnerNode->MarkModifyDone();
    spinnerNode->MountToParent(titleNode);
}

RefPtr<FrameNode> DatePickerDialogView::CreateDividerNode(const RefPtr<FrameNode>& dateNode)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto dialogTheme = pipeline->GetTheme<DialogTheme>();
    auto pickerPattern = dateNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_RETURN(pickerPattern, nullptr);
    auto dividerNode = FrameNode::GetOrCreateFrameNode(
        V2::DIVIDER_ETS_TAG, pickerPattern->GetDividerId(), []() { return AceType::MakeRefPtr<DividerPattern>(); });
    auto dividerRenderContext = dividerNode->GetRenderContext();
    CHECK_NULL_RETURN(dividerRenderContext, nullptr);
    dividerRenderContext->UpdateBackgroundColor(dialogTheme->GetDividerColor());

    MarginProperty margin;
    margin.top = CalcLength(dialogTheme->GetDividerHeight());
    margin.bottom = CalcLength(dialogTheme->GetDividerPadding().Bottom());
    dividerNode->GetLayoutProperty()->UpdateMargin(margin);
    dividerNode->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(dialogTheme->GetDividerWidth()), CalcLength(dialogTheme->GetDividerHeight())));

    return dividerNode;
}

RefPtr<FrameNode> DatePickerDialogView::CreateButtonNode(const RefPtr<FrameNode>& dateNode,
    std::map<std::string, NG::DialogEvent> dialogEvent, std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent)
{
    auto acceptEvent = dialogEvent["acceptId"];
    auto cancelEvent = dialogCancelEvent["cancelId"];
    auto contentRow = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    CHECK_NULL_RETURN(contentRow, nullptr);
    auto layoutProps = contentRow->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(layoutProps, nullptr);
    layoutProps->UpdateMainAxisAlign(FlexAlign::SPACE_AROUND);
    layoutProps->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);

    auto buttonCancelNode = CreateCancelNode(cancelEvent);
    auto buttonConfirmNode = CreateConfirmNode(dateNode, acceptEvent);

    buttonCancelNode->MountToParent(contentRow);
    buttonConfirmNode->MountToParent(contentRow);
    return contentRow;
}

RefPtr<FrameNode> DatePickerDialogView::CreateConfirmNode(const RefPtr<FrameNode>& dateNode, DialogEvent& acceptEvent)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto dialogTheme = pipeline->GetTheme<DialogTheme>();
    auto pickerTheme = pipeline->GetTheme<PickerTheme>();

    auto buttonConfirmNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    auto textConfirmNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(buttonConfirmNode, nullptr);
    CHECK_NULL_RETURN(textConfirmNode, nullptr);
    auto textLayoutProperty = textConfirmNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, nullptr);
    textLayoutProperty->UpdateContent(Localization::GetInstance()->GetEntryLetters("common.ok"));
    textLayoutProperty->UpdateTextColor(pickerTheme->GetOptionStyle(true, false).GetTextColor());
    textLayoutProperty->UpdateFontSize(pickerTheme->GetOptionStyle(false, false).GetFontSize());
    textLayoutProperty->UpdateFontWeight(pickerTheme->GetOptionStyle(true, false).GetFontWeight());
    auto buttonConfirmEventHub = buttonConfirmNode->GetEventHub<ButtonEventHub>();
    CHECK_NULL_RETURN(buttonConfirmEventHub, nullptr);
    buttonConfirmEventHub->SetStateEffect(true);

    auto buttonConfirmLayoutProperty = buttonConfirmNode->GetLayoutProperty<ButtonLayoutProperty>();
    buttonConfirmLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);
    buttonConfirmLayoutProperty->UpdateType(ButtonType::CAPSULE);
    buttonConfirmLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(pickerTheme->GetButtonWidth()), CalcLength(pickerTheme->GetButtonHeight())));
    auto buttonConfirmRenderContext = buttonConfirmNode->GetRenderContext();
    buttonConfirmRenderContext->UpdateBackgroundColor(Color::TRANSPARENT);

    MarginProperty margin;
    margin.right = CalcLength(dialogTheme->GetDividerPadding().Right());
    margin.top = CalcLength(dialogTheme->GetDividerHeight());
    margin.bottom = CalcLength(dialogTheme->GetDividerPadding().Bottom());
    buttonConfirmNode->GetLayoutProperty()->UpdateMargin(margin);

    textConfirmNode->MountToParent(buttonConfirmNode);
    auto eventConfirmHub = buttonConfirmNode->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(eventConfirmHub, nullptr);
    CHECK_NULL_RETURN(dateNode, nullptr);
    SetDialogAcceptEvent(dateNode, std::move(acceptEvent));
    auto clickCallback = [dateNode](const GestureEvent& /* info */) {
        auto pickerPattern = dateNode->GetPattern<DatePickerPattern>();
        CHECK_NULL_VOID(pickerPattern);
        auto str = pickerPattern->GetSelectedObject(true);
        auto datePickerEventHub = pickerPattern->GetEventHub<DatePickerEventHub>();
        CHECK_NULL_VOID(datePickerEventHub);
        datePickerEventHub->FireDialogAcceptEvent(str);
    };
    eventConfirmHub->AddClickEvent(AceType::MakeRefPtr<NG::ClickEvent>(clickCallback));
    buttonConfirmNode->MarkModifyDone();
    return buttonConfirmNode;
}

RefPtr<FrameNode> DatePickerDialogView::CreateDateNode(int32_t dateNodeId,
    std::map<std::string, PickerDate> datePickerProperty,
    const PickerTextProperties& properties, bool isLunar, bool showTime)
{
    auto dateNode = FrameNode::GetOrCreateFrameNode(V2::DATE_PICKER_ETS_TAG,
        dateNodeId, []() { return AceType::MakeRefPtr<DatePickerPattern>(); });
    CHECK_NULL_RETURN(dateNode, nullptr);
    auto datePickerPattern = dateNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_RETURN(datePickerPattern, nullptr);

    auto context = dateNode->GetContext();
    CHECK_NULL_RETURN(context, nullptr);
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_RETURN(themeManager, nullptr);
    auto pickerTheme = themeManager->GetTheme<PickerTheme>();
    CHECK_NULL_RETURN(pickerTheme, nullptr);
    uint32_t showCount = pickerTheme->GetShowOptionCount();
    if (SystemProperties::GetDeviceType() == DeviceType::PHONE &&
        SystemProperties::GetDeviceOrientation() == DeviceOrientation::LANDSCAPE) {
        showCount = OPTION_COUNT_PHONE_LANDSCAPE;
    }
    datePickerPattern->SetShowCount(showCount);

    if (showTime) {
        CreateSingleDateNode(dateNode, showCount);
    } else {
        CreateNormalDateNode(dateNode, showCount);
    }

    PickerDate parseStartDate;
    PickerDate parseEndDate;
    PickerDate parseSelectedDate;
    SetShowLunar(dateNode, isLunar);
    SetDateTextProperties(dateNode, properties);
    if (datePickerProperty.find("start") != datePickerProperty.end()) {
        parseStartDate = datePickerProperty["start"];
        SetStartDate(dateNode, parseStartDate);
    }
    if (datePickerProperty.find("end") != datePickerProperty.end()) {
        parseEndDate = datePickerProperty["end"];
        SetEndDate(dateNode, parseEndDate);
    }
    if (datePickerProperty.find("selected") != datePickerProperty.end()) {
        parseSelectedDate = datePickerProperty["selected"];
        SetSelectedDate(dateNode, parseSelectedDate);
    }
    return dateNode;
}

RefPtr<FrameNode> DatePickerDialogView::CreateColumnNode(int32_t nodeId, uint32_t showCount, bool isDate)
{
    RefPtr<FrameNode> columnNode;
    if (isDate) {
        columnNode = FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, nodeId,
            []() {return AceType::MakeRefPtr<DatePickerColumnPattern>();});
    } else {
        columnNode = FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, nodeId,
            []() {return AceType::MakeRefPtr<TimePickerColumnPattern>();});
    }
    CHECK_NULL_RETURN(columnNode, nullptr);
    columnNode->Clean();
    for (uint32_t index = 0; index < showCount; index++) {
        auto textNode = FrameNode::CreateFrameNode(
            V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
        CHECK_NULL_RETURN(textNode, nullptr);
        textNode->MountToParent(columnNode);
    }
    columnNode->MarkModifyDone();
    return columnNode;
}

void DatePickerDialogView::CreateNormalDateNode(const RefPtr<FrameNode>& dateNode, uint32_t showCount)
{
    auto datePickerPattern = dateNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetShowMonthDaysFlag(false);

    auto yearColumnNode = CreateColumnNode(datePickerPattern->GetYearId(), showCount);
    auto monthColumnNode = CreateColumnNode(datePickerPattern->GetMonthId(), showCount);
    auto dayColumnNode = CreateColumnNode(datePickerPattern->GetDayId(), showCount);
    CHECK_NULL_VOID(yearColumnNode);
    CHECK_NULL_VOID(monthColumnNode);
    CHECK_NULL_VOID(dayColumnNode);
    datePickerPattern->SetColumn(yearColumnNode);
    datePickerPattern->SetColumn(monthColumnNode);
    datePickerPattern->SetColumn(dayColumnNode);

    {
        auto stackYearNode = CreateStackNode();
        auto buttonYearNode = CreateButtonNode();
        buttonYearNode->MountToParent(stackYearNode);
        yearColumnNode->MountToParent(stackYearNode);
        auto layoutProperty = stackYearNode->GetLayoutProperty<LayoutProperty>();
        layoutProperty->UpdateAlignment(Alignment::CENTER);
        stackYearNode->MountToParent(dateNode);
    }
    {
        auto stackMonthNode = CreateStackNode();
        auto buttonMonthNode = CreateButtonNode();
        buttonMonthNode->MountToParent(stackMonthNode);
        monthColumnNode->MountToParent(stackMonthNode);
        auto layoutProperty = stackMonthNode->GetLayoutProperty<LayoutProperty>();
        layoutProperty->UpdateAlignment(Alignment::CENTER);
        stackMonthNode->MountToParent(dateNode);
    }
    {
        auto stackDayNode = CreateStackNode();
        auto buttonDayNode = CreateButtonNode();
        buttonDayNode->MountToParent(stackDayNode);
        dayColumnNode->MountToParent(stackDayNode);
        auto layoutProperty = stackDayNode->GetLayoutProperty<LayoutProperty>();
        layoutProperty->UpdateAlignment(Alignment::CENTER);
        stackDayNode->MountToParent(dateNode);
    }
}

void DatePickerDialogView::CreateSingleDateNode(const RefPtr<FrameNode>& dateNode, uint32_t showCount)
{
    auto datePickerPattern = dateNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetShowMonthDaysFlag(true);

    auto monthDaysColumnNode = CreateColumnNode(datePickerPattern->GetMonthDaysId(), showCount);
    auto yearColumnNode = CreateColumnNode(datePickerPattern->GetYearId(), showCount);
    CHECK_NULL_VOID(monthDaysColumnNode);
    CHECK_NULL_VOID(yearColumnNode);
    datePickerPattern->SetColumn(monthDaysColumnNode);
    datePickerPattern->SetColumn(yearColumnNode);

    {
        auto stackMonthNode = CreateStackNode();
        auto buttonMonthNode = CreateButtonNode();
        buttonMonthNode->MountToParent(stackMonthNode);
        monthDaysColumnNode->MountToParent(stackMonthNode);
        auto layoutProperty = stackMonthNode->GetLayoutProperty<LayoutProperty>();
        layoutProperty->UpdateAlignment(Alignment::CENTER);
        stackMonthNode->MountToParent(dateNode);
    }

    {
        auto stackYearNode = CreateStackNode();
        auto buttonYearNode = CreateButtonNode();
        buttonYearNode->MountToParent(stackYearNode);
        yearColumnNode->MountToParent(stackYearNode);
        auto layoutProperty = stackYearNode->GetLayoutProperty<LayoutProperty>();
        layoutProperty->UpdateAlignment(Alignment::CENTER);
        layoutProperty->UpdateVisibility(VisibleType::GONE);
        stackYearNode->MountToParent(dateNode);
    }
}

RefPtr<FrameNode> DatePickerDialogView::CreateTimeNode(std::map<std::string, PickerTime> timePickerProperty,
    const PickerTextProperties& properties, bool useMilitaryTime)
{
    auto timePickerNode = FrameNode::GetOrCreateFrameNode(V2::TIME_PICKER_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TimePickerRowPattern>(); });
    CHECK_NULL_RETURN(timePickerNode, nullptr);
    auto timePickerRowPattern = timePickerNode->GetPattern<TimePickerRowPattern>();
    CHECK_NULL_RETURN(timePickerRowPattern, nullptr);

    auto context = timePickerNode->GetContext();
    CHECK_NULL_RETURN(context, nullptr);
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_RETURN(themeManager, nullptr);
    auto pickerTheme = themeManager->GetTheme<PickerTheme>();
    CHECK_NULL_RETURN(pickerTheme, nullptr);
    uint32_t showCount = pickerTheme->GetShowOptionCount();
    if (SystemProperties::GetDeviceType() == DeviceType::PHONE &&
        SystemProperties::GetDeviceOrientation() == DeviceOrientation::LANDSCAPE) {
        showCount = OPTION_COUNT_PHONE_LANDSCAPE;
    }
    timePickerRowPattern->SetShowCount(showCount);

    auto hasHourNode = timePickerRowPattern->HasHourNode();
    auto hasMinuteNode = timePickerRowPattern->HasMinuteNode();

    auto hourColumnNode = CreateColumnNode(timePickerRowPattern->GetHourId(), showCount, false);
    auto minuteColumnNode = CreateColumnNode(timePickerRowPattern->GetMinuteId(), showCount, false);
    CHECK_NULL_RETURN(hourColumnNode, nullptr);
    CHECK_NULL_RETURN(minuteColumnNode, nullptr);
    timePickerRowPattern->SetColumn(hourColumnNode);
    timePickerRowPattern->SetColumn(minuteColumnNode);

    if (!hasHourNode) {
        auto stackHourNode = CreateStackNode();
        auto buttonYearNode = CreateButtonNode();
        buttonYearNode->MountToParent(stackHourNode);
        hourColumnNode->MountToParent(stackHourNode);
        auto layoutProperty = stackHourNode->GetLayoutProperty<LayoutProperty>();
        layoutProperty->UpdateAlignment(Alignment::CENTER);
        stackHourNode->MountToParent(timePickerNode);
    }
    if (!hasMinuteNode) {
        auto stackMinuteNode = CreateStackNode();
        auto buttonYearNode = CreateButtonNode();
        buttonYearNode->MountToParent(stackMinuteNode);
        minuteColumnNode->MountToParent(stackMinuteNode);
        auto layoutProperty = stackMinuteNode->GetLayoutProperty<LayoutProperty>();
        layoutProperty->UpdateAlignment(Alignment::CENTER);
        stackMinuteNode->MountToParent(timePickerNode);
    }
    if (timePickerProperty.find("selected") != timePickerProperty.end()) {
        auto selectedTime = timePickerProperty["selected"];
        timePickerRowPattern->SetSelectedTime(selectedTime);
    }
    timePickerRowPattern->SetHour24(useMilitaryTime);

    SetTimeTextProperties(timePickerNode, properties);
    return timePickerNode;
}

RefPtr<FrameNode> DatePickerDialogView::CreateCancelNode(NG::DialogGestureEvent& cancelEvent)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto dialogTheme = pipeline->GetTheme<DialogTheme>();
    auto pickerTheme = pipeline->GetTheme<PickerTheme>();
    auto buttonCancelNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    CHECK_NULL_RETURN(buttonCancelNode, nullptr);
    auto textCancelNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textCancelNode, nullptr);
    auto textCancelLayoutProperty = textCancelNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textCancelLayoutProperty, nullptr);
    textCancelLayoutProperty->UpdateContent(Localization::GetInstance()->GetEntryLetters("common.cancel"));
    textCancelLayoutProperty->UpdateTextColor(pickerTheme->GetOptionStyle(true, false).GetTextColor());
    textCancelLayoutProperty->UpdateFontSize(pickerTheme->GetOptionStyle(false, false).GetFontSize());
    textCancelLayoutProperty->UpdateFontWeight(pickerTheme->GetOptionStyle(true, false).GetFontWeight());
    textCancelNode->MountToParent(buttonCancelNode);
    auto eventCancelHub = buttonCancelNode->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(eventCancelHub, nullptr);
    eventCancelHub->AddClickEvent(AceType::MakeRefPtr<NG::ClickEvent>(std::move(cancelEvent)));

    auto buttonCancelEventHub = buttonCancelNode->GetEventHub<ButtonEventHub>();
    CHECK_NULL_RETURN(buttonCancelEventHub, nullptr);
    buttonCancelEventHub->SetStateEffect(true);

    MarginProperty margin;
    margin.left = CalcLength(dialogTheme->GetDividerPadding().Left());
    margin.top = CalcLength(dialogTheme->GetDividerHeight());
    margin.bottom = CalcLength(dialogTheme->GetDividerPadding().Bottom());
    buttonCancelNode->GetLayoutProperty()->UpdateMargin(margin);

    auto buttonCancelLayoutProperty = buttonCancelNode->GetLayoutProperty<ButtonLayoutProperty>();
    buttonCancelLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);
    buttonCancelLayoutProperty->UpdateType(ButtonType::CAPSULE);
    buttonCancelLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(pickerTheme->GetButtonWidth()), CalcLength(pickerTheme->GetButtonHeight())));

    auto buttonCancelRenderContext = buttonCancelNode->GetRenderContext();
    buttonCancelRenderContext->UpdateBackgroundColor(Color::TRANSPARENT);
    buttonCancelNode->MarkModifyDone();
    return buttonCancelNode;
}

void DatePickerDialogView::SetStartDate(const RefPtr<FrameNode>& frameNode, const PickerDate& value)
{
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetStartDate(value);
    auto pickerProperty = frameNode->GetLayoutProperty<DataPickerRowLayoutProperty>();
    CHECK_NULL_VOID(pickerProperty);
    pickerProperty->UpdateStartDate(datePickerPattern->GetStartDateLunar());
}

void DatePickerDialogView::SetEndDate(const RefPtr<FrameNode>& frameNode, const PickerDate& value)
{
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetEndDate(value);
    auto pickerProperty = frameNode->GetLayoutProperty<DataPickerRowLayoutProperty>();
    CHECK_NULL_VOID(pickerProperty);
    pickerProperty->UpdateEndDate(datePickerPattern->GetEndDateLunar());
}

void DatePickerDialogView::SetSelectedDate(const RefPtr<FrameNode>& frameNode, const PickerDate& value)
{
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetSelectDate(value);
    auto pickerProperty = frameNode->GetLayoutProperty<DataPickerRowLayoutProperty>();
    CHECK_NULL_VOID(pickerProperty);
    pickerProperty->UpdateSelectedDate(datePickerPattern->GetSelectDate());
}

void DatePickerDialogView::SetShowLunar(const RefPtr<FrameNode>& frameNode, bool lunar)
{
    auto pickerProperty = frameNode->GetLayoutProperty<DataPickerRowLayoutProperty>();
    CHECK_NULL_VOID(pickerProperty);
    pickerProperty->UpdateLunar(lunar);
}

void DatePickerDialogView::SetDialogChange(const RefPtr<FrameNode>& frameNode, DialogEvent&& onChange)
{
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<DatePickerEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetDialogChange(std::move(onChange));
}

void DatePickerDialogView::SetDialogAcceptEvent(const RefPtr<FrameNode>& frameNode, DialogEvent&& onChange)
{
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<DatePickerEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetDialogAcceptEvent(std::move(onChange));
}

void DatePickerDialogView::SetDialogSwitchEvent(std::function<bool()> switchEvent)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto overlayManger = pipeline->GetOverlayManager();
    CHECK_NULL_VOID(overlayManger);
    overlayManger->SetBackPressEvent(switchEvent);
}

void DatePickerDialogView::SetDateTextProperties(const RefPtr<FrameNode>& frameNode,
    const PickerTextProperties& properties)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto pickerTheme = pipeline->GetTheme<PickerTheme>();
    CHECK_NULL_VOID(pickerTheme);
    auto selectedStyle = pickerTheme->GetOptionStyle(true, false);
    auto disappearStyle = pickerTheme->GetDisappearOptionStyle();
    auto normalStyle = pickerTheme->GetOptionStyle(false, false);
    auto pickerProperty = frameNode->GetLayoutProperty<DataPickerRowLayoutProperty>();
    CHECK_NULL_VOID(pickerProperty);

    if (properties.disappearTextStyle_.fontSize.has_value() && properties.disappearTextStyle_.fontSize->IsValid()) {
        pickerProperty->UpdateDisappearFontSize(properties.disappearTextStyle_.fontSize.value());
    } else {
        pickerProperty->UpdateDisappearFontSize(disappearStyle.GetFontSize());
    }
    pickerProperty->UpdateDisappearColor(
        properties.disappearTextStyle_.textColor.value_or(disappearStyle.GetTextColor()));
    pickerProperty->UpdateDisappearWeight(
        properties.disappearTextStyle_.fontWeight.value_or(disappearStyle.GetFontWeight()));

    if (properties.normalTextStyle_.fontSize.has_value() && properties.normalTextStyle_.fontSize->IsValid()) {
        pickerProperty->UpdateFontSize(properties.normalTextStyle_.fontSize.value());
    } else {
        pickerProperty->UpdateFontSize(normalStyle.GetFontSize());
    }
    pickerProperty->UpdateColor(
        properties.normalTextStyle_.textColor.value_or(normalStyle.GetTextColor()));
    pickerProperty->UpdateWeight(
        properties.normalTextStyle_.fontWeight.value_or(normalStyle.GetFontWeight()));

    if (properties.selectedTextStyle_.fontSize.has_value() && properties.selectedTextStyle_.fontSize->IsValid()) {
        pickerProperty->UpdateSelectedFontSize(properties.selectedTextStyle_.fontSize.value());
    } else {
        pickerProperty->UpdateSelectedFontSize(selectedStyle.GetFontSize());
    }
    pickerProperty->UpdateSelectedColor(
        properties.selectedTextStyle_.textColor.value_or(selectedStyle.GetTextColor()));
    pickerProperty->UpdateSelectedWeight(
        properties.selectedTextStyle_.fontWeight.value_or(selectedStyle.GetFontWeight()));
}

void DatePickerDialogView::SetTimeTextProperties(const RefPtr<FrameNode>& frameNode,
    const PickerTextProperties& properties)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto pickerTheme = pipeline->GetTheme<PickerTheme>();
    CHECK_NULL_VOID(pickerTheme);
    auto selectedStyle = pickerTheme->GetOptionStyle(true, false);
    auto disappearStyle = pickerTheme->GetDisappearOptionStyle();
    auto normalStyle = pickerTheme->GetOptionStyle(false, false);
    auto pickerProperty = frameNode->GetLayoutProperty<TimePickerLayoutProperty>();
    CHECK_NULL_VOID(pickerProperty);

    if (properties.disappearTextStyle_.fontSize.has_value() && properties.disappearTextStyle_.fontSize->IsValid()) {
        pickerProperty->UpdateDisappearFontSize(properties.disappearTextStyle_.fontSize.value());
    } else {
        pickerProperty->UpdateDisappearFontSize(disappearStyle.GetFontSize());
    }
    pickerProperty->UpdateDisappearColor(
        properties.disappearTextStyle_.textColor.value_or(disappearStyle.GetTextColor()));
    pickerProperty->UpdateDisappearWeight(
        properties.disappearTextStyle_.fontWeight.value_or(disappearStyle.GetFontWeight()));

    if (properties.normalTextStyle_.fontSize.has_value() && properties.normalTextStyle_.fontSize->IsValid()) {
        pickerProperty->UpdateFontSize(properties.normalTextStyle_.fontSize.value());
    } else {
        pickerProperty->UpdateFontSize(normalStyle.GetFontSize());
    }
    pickerProperty->UpdateColor(
        properties.normalTextStyle_.textColor.value_or(normalStyle.GetTextColor()));
    pickerProperty->UpdateWeight(
        properties.normalTextStyle_.fontWeight.value_or(normalStyle.GetFontWeight()));

    if (properties.selectedTextStyle_.fontSize.has_value() && properties.selectedTextStyle_.fontSize->IsValid()) {
        pickerProperty->UpdateSelectedFontSize(properties.selectedTextStyle_.fontSize.value());
    } else {
        pickerProperty->UpdateSelectedFontSize(selectedStyle.GetFontSize());
    }
    pickerProperty->UpdateSelectedColor(
        properties.selectedTextStyle_.textColor.value_or(selectedStyle.GetTextColor()));
    pickerProperty->UpdateSelectedWeight(
        properties.selectedTextStyle_.fontWeight.value_or(selectedStyle.GetFontWeight()));
}
} // namespace OHOS::Ace::NG