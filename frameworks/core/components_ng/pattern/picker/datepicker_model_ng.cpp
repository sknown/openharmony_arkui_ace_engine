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
#include "core/components_ng/pattern/picker/datepicker_model_ng.h"

#include <utility>

#include "base/geometry/dimension.h"
#include "base/i18n/date_time_sequence.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components/picker/picker_date_component.h"
#include "core/components/picker/picker_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/picker/datepicker_column_pattern.h"
#include "core/components_ng/pattern/picker/datepicker_pattern.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t BUFFER_NODE_NUMBER = 2;
constexpr uint8_t PIXEL_ROUND = 18;
} // namespace
void DatePickerModelNG::CreateDatePicker(RefPtr<PickerTheme> pickerTheme)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::DATE_PICKER_ETS_TAG, nodeId);
    RefPtr<FrameNode> dateNode = FrameNode::GetOrCreateFrameNode(
        V2::DATE_PICKER_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<DatePickerPattern>(); });
    auto datePickerPattern = dateNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetPickerTag(true);
    auto language = AceApplicationInfo::GetInstance().GetLanguage();
    OrderResult orderResult;
    DateTimeSequence sequence;
    orderResult = sequence.GetDateOrder(language);
    dateOrder = orderResult.dateOrder;
    dateTimeOrder = orderResult.dateTimeOrder;
    if (language == "ug") {
        datePickerPattern->SetDateOrder("y-d-M");
    } else {
        datePickerPattern->SetDateOrder(dateOrder);
    }
    bool hasYearNode = datePickerPattern->HasYearNode();
    bool hasMonthNode = datePickerPattern->HasMonthNode();
    bool hasDayNode = datePickerPattern->HasDayNode();

    auto yearId = datePickerPattern->GetYearId();
    auto monthId = datePickerPattern->GetMonthId();
    auto dayId = datePickerPattern->GetDayId();
    CHECK_NULL_VOID(pickerTheme);
    uint32_t showCount = pickerTheme->GetShowOptionCount() + BUFFER_NODE_NUMBER;
    datePickerPattern->SetShowCount(showCount);
    RefPtr<FrameNode> yearColumnNode = FrameNode::GetOrCreateFrameNode(
        V2::COLUMN_ETS_TAG, yearId, []() { return AceType::MakeRefPtr<DatePickerColumnPattern>(); });
    CHECK_NULL_VOID(yearColumnNode);
    if (!hasYearNode) {
        for (uint32_t index = 0; index < showCount; index++) {
            auto textNode = FrameNode::CreateFrameNode(
                V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
            CHECK_NULL_VOID(textNode);
            textNode->MountToParent(yearColumnNode);
        }
        yearColumnNode->MarkModifyDone();
        datePickerPattern->SetColumn(yearColumnNode);
    }

    RefPtr<FrameNode> monthColumnNode = FrameNode::GetOrCreateFrameNode(
        V2::COLUMN_ETS_TAG, monthId, []() { return AceType::MakeRefPtr<DatePickerColumnPattern>(); });
    CHECK_NULL_VOID(monthColumnNode);
    if (!hasMonthNode) {
        for (uint32_t index = 0; index < showCount; index++) {
            auto textNode = FrameNode::CreateFrameNode(
                V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
            CHECK_NULL_VOID(textNode);
            textNode->MountToParent(monthColumnNode);
        }
        monthColumnNode->MarkModifyDone();
        datePickerPattern->SetColumn(monthColumnNode);
    }

    RefPtr<FrameNode> dayColumnNode = FrameNode::GetOrCreateFrameNode(
        V2::COLUMN_ETS_TAG, dayId, []() { return AceType::MakeRefPtr<DatePickerColumnPattern>(); });
    CHECK_NULL_VOID(dayColumnNode);
    if (!hasDayNode) {
        for (uint32_t index = 0; index < showCount; index++) {
            auto textNode = FrameNode::CreateFrameNode(
                V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
            CHECK_NULL_VOID(textNode);
            textNode->MountToParent(dayColumnNode);
        }
        dayColumnNode->MarkModifyDone();
        datePickerPattern->SetColumn(dayColumnNode);
    }

    if (dateOrder == "M-d-y") {
        if (!hasMonthNode) {
            createMonthOrDayColumnNode(monthColumnNode, dateNode, Color::BLUE);
        }
        if (!hasDayNode) {
            createMonthOrDayColumnNode(dayColumnNode, dateNode, Color::GRAY);
        }
        if (!hasYearNode) {
            createYearColumnNode(yearColumnNode, dateNode);
        }
    } else {
        if (!hasYearNode) {
            createYearColumnNode(yearColumnNode, dateNode);
        }
        if (!hasMonthNode) {
            createMonthOrDayColumnNode(monthColumnNode, dateNode, Color::BLUE);
        }
        if (!hasDayNode) {
            createMonthOrDayColumnNode(dayColumnNode, dateNode, Color::GRAY);
        }
    }
    
    stack->Push(dateNode);
}

void DatePickerModelNG::createMonthOrDayColumnNode(const RefPtr<FrameNode>& columnNode,
    const RefPtr<FrameNode>& dateNode, Color buttonBackgroundColor)
{
    auto stackNode = CreateStackNode();
    auto blendNode = CreateColumnNode();
    auto buttonNode = CreateButtonNode();
    buttonNode->GetRenderContext()->UpdateBackgroundColor(buttonBackgroundColor);
    buttonNode->MountToParent(stackNode);
    columnNode->MountToParent(blendNode);
    blendNode->MountToParent(stackNode);
    auto layoutProperty = stackNode->GetLayoutProperty<LayoutProperty>();
    layoutProperty->UpdateAlignment(Alignment::CENTER);
    layoutProperty->UpdateLayoutWeight(1);
    stackNode->MountToParent(dateNode);
    columnNode->GetLayoutProperty<LayoutProperty>()->UpdatePixelRound(PIXEL_ROUND);
}

void DatePickerModelNG::createYearColumnNode(const RefPtr<FrameNode>& columnNode,
    const RefPtr<FrameNode>& dateNode)
{
    auto stackYearNode = CreateStackNode();
    auto blendYearNode = CreateColumnNode();
    auto buttonYearNode = CreateButtonNode();
    buttonYearNode->MountToParent(stackYearNode);
    columnNode->MountToParent(blendYearNode);
    blendYearNode->MountToParent(stackYearNode);
    auto layoutProperty = stackYearNode->GetLayoutProperty<LayoutProperty>();
    layoutProperty->UpdateAlignment(Alignment::CENTER);
    layoutProperty->UpdateLayoutWeight(1);
    stackYearNode->MountToParent(dateNode);
    columnNode->GetLayoutProperty<LayoutProperty>()->UpdatePixelRound(PIXEL_ROUND);
}

RefPtr<FrameNode> DatePickerModelNG::CreateStackNode()
{
    auto stackId = ElementRegister::GetInstance()->MakeUniqueId();
    return FrameNode::GetOrCreateFrameNode(
        V2::STACK_ETS_TAG, stackId, []() { return AceType::MakeRefPtr<StackPattern>(); });
}

RefPtr<FrameNode> DatePickerModelNG::CreateColumnNode()
{
    auto columnId = ElementRegister::GetInstance()->MakeUniqueId();
    return FrameNode::GetOrCreateFrameNode(
        V2::COLUMN_ETS_TAG, columnId, []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
}

RefPtr<FrameNode> DatePickerModelNG::CreateButtonNode()
{
    auto buttonId = ElementRegister::GetInstance()->MakeUniqueId();
    return FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, buttonId, []() { return AceType::MakeRefPtr<ButtonPattern>(); });
}

RefPtr<FrameNode> DatePickerModelNG::CreateFrameNode(int32_t nodeId)
{
    auto dateNode = FrameNode::GetOrCreateFrameNode(
        V2::DATE_PICKER_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<DatePickerPattern>(); });
    auto datePickerPattern = dateNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_RETURN(datePickerPattern, dateNode);
    datePickerPattern->SetPickerTag(true);
    bool hasYearNode = datePickerPattern->HasYearNode();
    bool hasMonthNode = datePickerPattern->HasMonthNode();
    bool hasDayNode = datePickerPattern->HasDayNode();

    auto yearId = datePickerPattern->GetYearId();
    auto monthId = datePickerPattern->GetMonthId();
    auto dayId = datePickerPattern->GetDayId();

    auto pipeline = PipelineBase::GetCurrentContextSafely();
    CHECK_NULL_RETURN(pipeline, dateNode);
    auto pickerTheme = pipeline->GetTheme<PickerTheme>();
    CHECK_NULL_RETURN(pickerTheme, dateNode);
    uint32_t showCount = pickerTheme->GetShowOptionCount() + BUFFER_NODE_NUMBER;
    datePickerPattern->SetShowCount(showCount);
    SetDefaultAttributes(dateNode, pickerTheme);

    auto yearColumnNode = FrameNode::GetOrCreateFrameNode(
        V2::COLUMN_ETS_TAG, yearId, []() { return AceType::MakeRefPtr<DatePickerColumnPattern>(); });
    CHECK_NULL_RETURN(yearColumnNode, dateNode);
    if (!hasYearNode) {
        for (uint32_t index = 0; index < showCount; index++) {
            auto textNode = FrameNode::CreateFrameNode(
                V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
            CHECK_NULL_RETURN(textNode, dateNode);
            textNode->MountToParent(yearColumnNode);
        }
        yearColumnNode->MarkModifyDone();
        datePickerPattern->SetColumn(yearColumnNode);
    }

    auto monthColumnNode = FrameNode::GetOrCreateFrameNode(
        V2::COLUMN_ETS_TAG, monthId, []() { return AceType::MakeRefPtr<DatePickerColumnPattern>(); });
    CHECK_NULL_RETURN(monthColumnNode, dateNode);
    if (!hasMonthNode) {
        for (uint32_t index = 0; index < showCount; index++) {
            auto textNode = FrameNode::CreateFrameNode(
                V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
            CHECK_NULL_RETURN(textNode, dateNode);
            textNode->MountToParent(monthColumnNode);
        }
        monthColumnNode->MarkModifyDone();
        datePickerPattern->SetColumn(monthColumnNode);
    }

    auto dayColumnNode = FrameNode::GetOrCreateFrameNode(
        V2::COLUMN_ETS_TAG, dayId, []() { return AceType::MakeRefPtr<DatePickerColumnPattern>(); });
    CHECK_NULL_RETURN(dayColumnNode, dateNode);
    if (!hasDayNode) {
        for (uint32_t index = 0; index < showCount; index++) {
            auto textNode = FrameNode::CreateFrameNode(
                V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
            CHECK_NULL_RETURN(textNode, dateNode);
            textNode->MountToParent(dayColumnNode);
        }
        dayColumnNode->MarkModifyDone();
        datePickerPattern->SetColumn(dayColumnNode);
    }

    if (!hasYearNode) {
        auto stackYearNode = CreateStackNode();
        auto blendYearNode = CreateColumnNode();
        auto buttonYearNode = CreateButtonNode();
        buttonYearNode->MountToParent(stackYearNode);
        yearColumnNode->MountToParent(blendYearNode);
        blendYearNode->MountToParent(stackYearNode);
        auto layoutProperty = stackYearNode->GetLayoutProperty<LayoutProperty>();
        layoutProperty->UpdateAlignment(Alignment::CENTER);
        layoutProperty->UpdateLayoutWeight(1);
        stackYearNode->MountToParent(dateNode);
        yearColumnNode->GetLayoutProperty<LayoutProperty>()->UpdatePixelRound(PIXEL_ROUND);
    }
    if (!hasMonthNode) {
        auto stackMonthNode = CreateStackNode();
        auto blendMonthNode = CreateColumnNode();
        auto buttonMonthNode = CreateButtonNode();
        buttonMonthNode->GetRenderContext()->UpdateBackgroundColor(Color::BLUE);
        buttonMonthNode->MountToParent(stackMonthNode);
        monthColumnNode->MountToParent(blendMonthNode);
        blendMonthNode->MountToParent(stackMonthNode);
        auto layoutProperty = stackMonthNode->GetLayoutProperty<LayoutProperty>();
        layoutProperty->UpdateAlignment(Alignment::CENTER);
        layoutProperty->UpdateLayoutWeight(1);
        stackMonthNode->MountToParent(dateNode);
        monthColumnNode->GetLayoutProperty<LayoutProperty>()->UpdatePixelRound(PIXEL_ROUND);
    }
    if (!hasDayNode) {
        auto stackDayNode = CreateStackNode();
        auto blendDayNode = CreateColumnNode();
        auto buttonDayNode = CreateButtonNode();
        buttonDayNode->GetRenderContext()->UpdateBackgroundColor(Color::GRAY);
        buttonDayNode->MountToParent(stackDayNode);
        dayColumnNode->MountToParent(blendDayNode);
        blendDayNode->MountToParent(stackDayNode);
        auto layoutProperty = stackDayNode->GetLayoutProperty<LayoutProperty>();
        layoutProperty->UpdateAlignment(Alignment::CENTER);
        layoutProperty->UpdateLayoutWeight(1);
        stackDayNode->MountToParent(dateNode);
        dayColumnNode->GetLayoutProperty<LayoutProperty>()->UpdatePixelRound(PIXEL_ROUND);
    }
    return dateNode;
}

void DatePickerModelNG::SetShowLunar(bool lunar)
{
    ACE_UPDATE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, Lunar, lunar);
}

void DatePickerModelNG::SetStartDate(const PickerDate& value)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetStartDate(value);
    ACE_UPDATE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, StartDate, datePickerPattern->GetStartDateLunar());
}

void DatePickerModelNG::SetEndDate(const PickerDate& value)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetEndDate(value);
    ACE_UPDATE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, EndDate, datePickerPattern->GetEndDateLunar());
}

void DatePickerModelNG::SetSelectedDate(const PickerDate& value)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetSelectDate(value);
    ACE_UPDATE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, SelectedDate, datePickerPattern->GetSelectDate());
}

void DatePickerModelNG::SetOnChange(DateChangeEvent&& onChange)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<DatePickerEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnChange(std::move(onChange));
}

void DatePickerModelNG::SetOnDateChange(DateChangeEvent&& onChange)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<DatePickerEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnDateChange(std::move(onChange));
}

void DatePickerModelNG::SetOnDateChange(FrameNode* frameNode, DateChangeEvent&& onChange)
{
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<DatePickerEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnDateChange(std::move(onChange));
}

PickerTextStyle DatePickerModelNG::getDisappearTextStyle(FrameNode* frameNode)
{
    PickerTextStyle pickerTextStyle;
    CHECK_NULL_RETURN(frameNode, pickerTextStyle);
    auto context = frameNode->GetContext();
    CHECK_NULL_RETURN(context, pickerTextStyle);
    auto theme = context->GetTheme<PickerTheme>();
    CHECK_NULL_RETURN(theme, pickerTextStyle);
    auto style = theme->GetDisappearOptionStyle();
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(
        DataPickerRowLayoutProperty, DisappearFontSize, pickerTextStyle.fontSize, frameNode, style.GetFontSize());
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(
        DataPickerRowLayoutProperty, DisappearColor, pickerTextStyle.textColor, frameNode, style.GetTextColor());
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(
        DataPickerRowLayoutProperty, DisappearWeight, pickerTextStyle.fontWeight, frameNode, style.GetFontWeight());
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(DataPickerRowLayoutProperty, DisappearFontFamily,
        pickerTextStyle.fontFamily, frameNode, style.GetFontFamilies());
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(DataPickerRowLayoutProperty, DisappearFontStyle,
        pickerTextStyle.fontStyle, frameNode, style.GetFontStyle());
    return pickerTextStyle;
}

void DatePickerModelNG::SetDisappearTextStyle(const RefPtr<PickerTheme>& theme, const PickerTextStyle& value)
{
    CHECK_NULL_VOID(theme);
    auto disappearStyle = theme->GetDisappearOptionStyle();
    if (value.fontSize.has_value() && value.fontSize->IsValid()) {
        ACE_UPDATE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, DisappearFontSize, value.fontSize.value());
    } else {
        ACE_UPDATE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, DisappearFontSize, disappearStyle.GetFontSize());
    }
    ACE_UPDATE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, DisappearColor, value.textColor.value_or(disappearStyle.GetTextColor()));
    ACE_UPDATE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, DisappearWeight, value.fontWeight.value_or(disappearStyle.GetFontWeight()));
    ACE_UPDATE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, DisappearFontFamily, value.fontFamily.value_or(disappearStyle.GetFontFamilies()));
    ACE_UPDATE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, DisappearFontStyle, value.fontStyle.value_or(disappearStyle.GetFontStyle()));
}

PickerTextStyle DatePickerModelNG::getNormalTextStyle(FrameNode* frameNode)
{
    PickerTextStyle pickerTextStyle;
    CHECK_NULL_RETURN(frameNode, pickerTextStyle);
    auto context = frameNode->GetContext();
    CHECK_NULL_RETURN(context, pickerTextStyle);
    auto theme = context->GetTheme<PickerTheme>();
    CHECK_NULL_RETURN(theme, pickerTextStyle);
    auto style = theme->GetOptionStyle(false, false);
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(
        DataPickerRowLayoutProperty, FontSize, pickerTextStyle.fontSize, frameNode, style.GetFontSize());
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(
        DataPickerRowLayoutProperty, Color, pickerTextStyle.textColor, frameNode, style.GetTextColor());
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(
        DataPickerRowLayoutProperty, Weight, pickerTextStyle.fontWeight, frameNode, style.GetFontWeight());
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(DataPickerRowLayoutProperty, FontFamily,
        pickerTextStyle.fontFamily, frameNode, style.GetFontFamilies());
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(DataPickerRowLayoutProperty, FontStyle,
        pickerTextStyle.fontStyle, frameNode, style.GetFontStyle());
    return pickerTextStyle;
}

void DatePickerModelNG::SetNormalTextStyle(const RefPtr<PickerTheme>& theme, const PickerTextStyle& value)
{
    CHECK_NULL_VOID(theme);
    auto normalStyle = theme->GetOptionStyle(false, false);
    if (value.fontSize.has_value() && value.fontSize->IsValid()) {
        ACE_UPDATE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, FontSize, value.fontSize.value());
    } else {
        ACE_UPDATE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, FontSize, normalStyle.GetFontSize());
    }
    ACE_UPDATE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, Color, value.textColor.value_or(normalStyle.GetTextColor()));
    ACE_UPDATE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, Weight, value.fontWeight.value_or(normalStyle.GetFontWeight()));
    ACE_UPDATE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, FontFamily, value.fontFamily.value_or(normalStyle.GetFontFamilies()));
    ACE_UPDATE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, FontStyle, value.fontStyle.value_or(normalStyle.GetFontStyle()));
}

PickerTextStyle DatePickerModelNG::getSelectedTextStyle(FrameNode* frameNode)
{
    PickerTextStyle pickerTextStyle;
    CHECK_NULL_RETURN(frameNode, pickerTextStyle);
    auto context = frameNode->GetContext();
    CHECK_NULL_RETURN(context, pickerTextStyle);
    auto theme = context->GetTheme<PickerTheme>();
    CHECK_NULL_RETURN(theme, pickerTextStyle);
    auto style = theme->GetOptionStyle(true, false);
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(
        DataPickerRowLayoutProperty, SelectedFontSize, pickerTextStyle.fontSize, frameNode, style.GetFontSize());
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(
        DataPickerRowLayoutProperty, SelectedColor, pickerTextStyle.textColor, frameNode, style.GetTextColor());
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(
        DataPickerRowLayoutProperty, SelectedWeight, pickerTextStyle.fontWeight, frameNode, style.GetFontWeight());
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(DataPickerRowLayoutProperty, SelectedFontFamily,
        pickerTextStyle.fontFamily, frameNode, style.GetFontFamilies());
    ACE_GET_NODE_LAYOUT_PROPERTY_WITH_DEFAULT_VALUE(DataPickerRowLayoutProperty, SelectedFontStyle,
        pickerTextStyle.fontStyle, frameNode, style.GetFontStyle());
    return pickerTextStyle;
}

void DatePickerModelNG::SetSelectedTextStyle(const RefPtr<PickerTheme>& theme, const PickerTextStyle& value)
{
    CHECK_NULL_VOID(theme);
    auto selectedStyle = theme->GetOptionStyle(true, false);
    if (value.fontSize.has_value() && value.fontSize->IsValid()) {
        ACE_UPDATE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, SelectedFontSize, value.fontSize.value());
    } else {
        ACE_UPDATE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, SelectedFontSize, selectedStyle.GetFontSize());
    }
    ACE_UPDATE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, SelectedColor, value.textColor.value_or(selectedStyle.GetTextColor()));
    ACE_UPDATE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, SelectedWeight, value.fontWeight.value_or(selectedStyle.GetFontWeight()));
    ACE_UPDATE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, SelectedFontFamily, value.fontFamily.value_or(selectedStyle.GetFontFamilies()));
    ACE_UPDATE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, SelectedFontStyle, value.fontStyle.value_or(selectedStyle.GetFontStyle()));
}

void DatePickerModelNG::SetDefaultAttributes(RefPtr<FrameNode>& frameNode, const RefPtr<PickerTheme>& pickerTheme)
{
    auto selectedStyle = pickerTheme->GetOptionStyle(true, false);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, SelectedFontSize, selectedStyle.GetFontSize(), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, SelectedColor, selectedStyle.GetTextColor(), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, SelectedWeight, selectedStyle.GetFontWeight(), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, SelectedFontFamily, selectedStyle.GetFontFamilies(), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, SelectedFontStyle, selectedStyle.GetFontStyle(), frameNode);

    auto disappearStyle = pickerTheme->GetDisappearOptionStyle();
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, DisappearFontSize, disappearStyle.GetFontSize(), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, DisappearColor, disappearStyle.GetTextColor(), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, DisappearWeight, disappearStyle.GetFontWeight(), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, DisappearFontFamily, disappearStyle.GetFontFamilies(), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, DisappearFontStyle, disappearStyle.GetFontStyle(), frameNode);

    auto normalStyle = pickerTheme->GetOptionStyle(false, false);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, FontSize, normalStyle.GetFontSize(), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, Color, normalStyle.GetTextColor(), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, Weight, normalStyle.GetFontWeight(), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, FontFamily, normalStyle.GetFontFamilies(), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, FontStyle, normalStyle.GetFontStyle(), frameNode);
}

void DatePickerModelNG::HasUserDefinedDisappearFontFamily(bool isUserDefined)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->HasUserDefinedDisappearFontFamily(isUserDefined);
}

void DatePickerModelNG::HasUserDefinedNormalFontFamily(bool isUserDefined)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->HasUserDefinedNormalFontFamily(isUserDefined);
}

void DatePickerModelNG::HasUserDefinedSelectedFontFamily(bool isUserDefined)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->HasUserDefinedSelectedFontFamily(isUserDefined);
}

void DatePickerModelNG::SetBackgroundColor(const Color& color)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetBackgroundColor(color);
}

void DatePickerModelNG::SetBackgroundColor(FrameNode* frameNode, const Color& color)
{
    CHECK_NULL_VOID(frameNode);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetBackgroundColor(color);
}

void DatePickerModelNG::SetStartDate(FrameNode* frameNode, const PickerDate& value)
{
    CHECK_NULL_VOID(frameNode);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetStartDate(value);
    ACE_UPDATE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, StartDate, datePickerPattern->GetStartDateLunar());
}

void DatePickerModelNG::SetEndDate(FrameNode* frameNode, const PickerDate& value)
{
    CHECK_NULL_VOID(frameNode);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetEndDate(value);
    ACE_UPDATE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, EndDate, datePickerPattern->GetEndDateLunar());
}

void DatePickerModelNG::SetSelectedDate(FrameNode* frameNode, const PickerDate& value)
{
    CHECK_NULL_VOID(frameNode);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_VOID(datePickerPattern);
    datePickerPattern->SetSelectDate(value);
    ACE_UPDATE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, SelectedDate, datePickerPattern->GetSelectDate());
}

void DatePickerModelNG::SetChangeEvent(DateChangeEvent&& onChange)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<DatePickerEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetChangeEvent(std::move(onChange));
}

void DatePickerDialogModelNG::SetDatePickerDialogShow(PickerDialogInfo& pickerDialog,
    NG::DatePickerSettingData& settingData, std::function<void()>&& onCancel,
    std::function<void(const std::string&)>&& onAccept, std::function<void(const std::string&)>&& onChange,
    std::function<void(const std::string&)>&& onDateAccept, std::function<void(const std::string&)>&& onDateChange,
    DatePickerType pickerType, PickerDialogEvent& pickerDialogEvent, const std::vector<ButtonInfo>& buttonInfos)
{
    auto container = Container::Current();
    if (!container) {
        return;
    }
    auto pipelineContext = AccessibilityManager::DynamicCast<NG::PipelineContext>(container->GetPipelineContext());
    if (!pipelineContext) {
        return;
    }

    auto executor = pipelineContext->GetTaskExecutor();
    if (!executor) {
        return;
    }
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<DialogTheme>();
    CHECK_NULL_VOID(theme);
    std::map<std::string, NG::DialogEvent> dialogEvent;
    std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent;
    std::map<std::string, NG::DialogCancelEvent> dialogLifeCycleEvent;
    dialogEvent["changeId"] = onChange;
    dialogEvent["acceptId"] = onAccept;
    dialogEvent["dateChangeId"] = onDateChange;
    dialogEvent["dateAcceptId"] = onDateAccept;
    auto func = [onCancel](const GestureEvent& /* info */) {
        if (onCancel) {
            onCancel();
        }
    };
    dialogCancelEvent["cancelId"] = func;
    dialogLifeCycleEvent["didAppearId"] = pickerDialogEvent.onDidAppear;
    dialogLifeCycleEvent["didDisappearId"] = pickerDialogEvent.onDidDisappear;
    dialogLifeCycleEvent["willAppearId"] = pickerDialogEvent.onWillAppear;
    dialogLifeCycleEvent["willDisappearId"] = pickerDialogEvent.onWillDisappear;
    DialogProperties properties;
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        properties.alignment = theme->GetAlignment();
    }
    if (pickerDialog.alignment.has_value()) {
        properties.alignment = pickerDialog.alignment.value();
    }
    if (pickerDialog.backgroundColor.has_value()) {
        properties.backgroundColor = pickerDialog.backgroundColor.value();
    }
    if (pickerDialog.backgroundBlurStyle.has_value()) {
        properties.backgroundBlurStyle = pickerDialog.backgroundBlurStyle.value();
    }
    if (pickerDialog.shadow.has_value()) {
        properties.shadow = pickerDialog.shadow.value();
    }

    properties.customStyle = false;
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        properties.offset = DimensionOffset(Offset(0, -theme->GetMarginBottom().ConvertToPx()));
    }
    if (pickerDialog.offset.has_value()) {
        properties.offset = pickerDialog.offset.value();
    }

    properties.maskRect = pickerDialog.maskRect;

    std::map<std::string, PickerDate> datePickerProperty;
    std::map<std::string, PickerTime> timePickerProperty;
    if (pickerDialog.isStartDate == true) {
        settingData.datePickerProperty["start"] = pickerDialog.parseStartDate;
    }
    if (pickerDialog.isEndDate == true) {
        settingData.datePickerProperty["end"] = pickerDialog.parseEndDate;
    }
    if (pickerDialog.isSelectedDate == true) {
        settingData.datePickerProperty["selected"] = pickerDialog.parseSelectedDate;
        settingData.timePickerProperty["selected"] = pickerDialog.pickerTime;
    }
    auto context = AccessibilityManager::DynamicCast<NG::PipelineContext>(pipelineContext);
    auto overlayManager = context ? context->GetOverlayManager() : nullptr;
    executor->PostTask(
        [properties, settingData, dialogEvent, dialogCancelEvent, dialogLifeCycleEvent, buttonInfos,
            weak = WeakPtr<NG::OverlayManager>(overlayManager)] {
            auto overlayManager = weak.Upgrade();
            CHECK_NULL_VOID(overlayManager);
            overlayManager->ShowDateDialog(
                properties, settingData, dialogEvent, dialogCancelEvent, dialogLifeCycleEvent, buttonInfos);
        },
        TaskExecutor::TaskType::UI, "ArkUIDatePickerShowDateDialog");
}

void DatePickerModelNG::SetSelectedTextStyle(
    FrameNode* frameNode, const RefPtr<PickerTheme>& theme, const PickerTextStyle& value)
{
    CHECK_NULL_VOID(theme);
    auto selectedStyle = theme->GetOptionStyle(true, false);
    if (value.fontSize.has_value() && value.fontSize->IsValid()) {
        ACE_UPDATE_NODE_LAYOUT_PROPERTY(
            DataPickerRowLayoutProperty, SelectedFontSize, value.fontSize.value(), frameNode);
    } else {
        ACE_UPDATE_NODE_LAYOUT_PROPERTY(
            DataPickerRowLayoutProperty, SelectedFontSize, selectedStyle.GetFontSize(), frameNode);
    }
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, SelectedColor, value.textColor.value_or(selectedStyle.GetTextColor()), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, SelectedWeight,
        value.fontWeight.value_or(selectedStyle.GetFontWeight()), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, SelectedFontFamily,
        value.fontFamily.value_or(selectedStyle.GetFontFamilies()), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, SelectedFontStyle,
        value.fontStyle.value_or(selectedStyle.GetFontStyle()), frameNode);
}

void DatePickerModelNG::SetNormalTextStyle(
    FrameNode* frameNode, const RefPtr<PickerTheme>& theme, const PickerTextStyle& value)
{
    CHECK_NULL_VOID(theme);
    auto normalStyle = theme->GetOptionStyle(false, false);
    if (value.fontSize.has_value() && value.fontSize->IsValid()) {
        ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, FontSize, value.fontSize.value(), frameNode);
    } else {
        ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, FontSize, normalStyle.GetFontSize(), frameNode);
    }
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, Color, value.textColor.value_or(normalStyle.GetTextColor()), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, Weight, value.fontWeight.value_or(normalStyle.GetFontWeight()), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, FontFamily, value.fontFamily.value_or(normalStyle.GetFontFamilies()), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        DataPickerRowLayoutProperty, FontStyle, value.fontStyle.value_or(normalStyle.GetFontStyle()), frameNode);
}

void DatePickerModelNG::SetDisappearTextStyle(
    FrameNode* frameNode, const RefPtr<PickerTheme>& theme, const PickerTextStyle& value)
{
    CHECK_NULL_VOID(theme);
    auto disappearStyle = theme->GetDisappearOptionStyle();
    if (value.fontSize.has_value() && value.fontSize->IsValid()) {
        ACE_UPDATE_NODE_LAYOUT_PROPERTY(
            DataPickerRowLayoutProperty, DisappearFontSize, value.fontSize.value(), frameNode);
    } else {
        ACE_UPDATE_NODE_LAYOUT_PROPERTY(
            DataPickerRowLayoutProperty, DisappearFontSize, disappearStyle.GetFontSize(), frameNode);
    }
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, DisappearColor,
        value.textColor.value_or(disappearStyle.GetTextColor()), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, DisappearWeight,
        value.fontWeight.value_or(disappearStyle.GetFontWeight()), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, DisappearFontFamily,
        value.fontFamily.value_or(disappearStyle.GetFontFamilies()), frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, DisappearFontStyle,
        value.fontStyle.value_or(disappearStyle.GetFontStyle()), frameNode);
}

void DatePickerModelNG::SetShowLunar(FrameNode* frameNode, bool lunar)
{
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(DataPickerRowLayoutProperty, Lunar, lunar, frameNode);
}

int32_t DatePickerModelNG::getLunar(FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, 0);
    return frameNode->GetLayoutProperty<DataPickerRowLayoutProperty>()->GetLunar().value_or(0);
}

LunarDate DatePickerModelNG::getStartDate(FrameNode* frameNode)
{
    LunarDate lunarDate;
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_RETURN(datePickerPattern, lunarDate);
    auto dateSolar = datePickerPattern->GetStartDateSolar();
    lunarDate.year = dateSolar.GetYear();
    lunarDate.month = dateSolar.GetMonth();
    lunarDate.day = dateSolar.GetDay();
    return lunarDate;
}

LunarDate DatePickerModelNG::getEndDate(FrameNode* frameNode)
{
    LunarDate lunarDate;
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_RETURN(datePickerPattern, lunarDate);
    auto dateSolar = datePickerPattern->GetEndDateSolar();
    lunarDate.year = dateSolar.GetYear();
    lunarDate.month = dateSolar.GetMonth();
    lunarDate.day = dateSolar.GetDay();
    return lunarDate;
}

LunarDate DatePickerModelNG::getSelectedDate(FrameNode* frameNode)
{
    LunarDate lunarDate;
    CHECK_NULL_RETURN(frameNode, lunarDate);
    auto context = frameNode->GetContext();
    CHECK_NULL_RETURN(context, lunarDate);
    auto theme = context->GetTheme<PickerTheme>();
    CHECK_NULL_RETURN(theme, lunarDate);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    CHECK_NULL_RETURN(datePickerPattern, lunarDate);
    PickerDate pickerDate = datePickerPattern->GetSelectedDate();
    lunarDate.year = pickerDate.GetYear();
    lunarDate.month = pickerDate.GetMonth();
    lunarDate.day = pickerDate.GetDay();
    return lunarDate;
}

uint32_t DatePickerModelNG::getBackgroundColor(FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, 0);
    auto value = frameNode->GetPaintProperty<ScrollablePaintProperty>()->GetScrollBarColor();
    return value->GetValue();
}


} // namespace OHOS::Ace::NG
