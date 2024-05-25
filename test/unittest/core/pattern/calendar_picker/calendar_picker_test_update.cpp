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

#include <cstddef>
#include <optional>
#include <string>
#include <utility>

#include "gtest/gtest.h"

#define private public
#define protected public

#include "test/mock/base/mock_task_executor.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/render/mock_render_context.h"

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/geometry/ng/size_t.h"
#include "base/geometry/offset.h"
#include "base/json/json_util.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components/calendar/calendar_theme.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/theme/icon_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/calendar/calendar_month_pattern.h"
#include "core/components_ng/pattern/calendar/calendar_paint_property.h"
#include "core/components_ng/pattern/calendar/calendar_pattern.h"
#include "core/components_ng/pattern/calendar_picker/calendar_dialog_pattern.h"
#include "core/components_ng/pattern/calendar_picker/calendar_dialog_view.h"
#include "core/components_ng/pattern/calendar_picker/calendar_picker_event_hub.h"
#include "core/components_ng/pattern/calendar_picker/calendar_picker_layout_algorithm.h"
#include "core/components_ng/pattern/calendar_picker/calendar_picker_layout_property.h"
#include "core/components_ng/pattern/calendar_picker/calendar_picker_model_ng.h"
#include "core/components_ng/pattern/dialog/dialog_view.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/flex/flex_layout_pattern.h"
#include "core/components_ng/pattern/flex/flex_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/picker/date_time_animation_controller.h"
#include "core/components_ng/pattern/picker/datepicker_pattern.h"
#include "core/components_ng/pattern/picker/datepicker_row_layout_property.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
const InspectorFilter filter;
constexpr Dimension TEST_SETTING_RADIUS = Dimension(10.0, DimensionUnit::VP);
} // namespace
class CalendarPickerTestUpdate : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void MockPipelineContextGetTheme();

protected:
    static void CreateCalendarPicker();
    static RefPtr<FrameNode> CalendarDialogShow(RefPtr<FrameNode> entryNode);
};

void CalendarPickerTestUpdate::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
        if (type == CalendarTheme::TypeId()) {
            return AceType::MakeRefPtr<CalendarTheme>();
        } else if (type == IconTheme::TypeId()) {
            return AceType::MakeRefPtr<IconTheme>();
        } else if (type == DialogTheme::TypeId()) {
            return AceType::MakeRefPtr<DialogTheme>();
        } else {
            return AceType::MakeRefPtr<PickerTheme>();
        }
    });
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
}

void CalendarPickerTestUpdate::TearDownTestCase()
{
    MockPipelineContext::GetCurrent()->SetThemeManager(nullptr);
    MockPipelineContext::TearDown();
}

void CalendarPickerTestUpdate::CreateCalendarPicker()
{
    CalendarSettingData settingData;
    CalendarPickerModelNG calendarPickerModel;

    calendarPickerModel.Create(settingData);
    DimensionOffset offset;
    calendarPickerModel.SetEdgeAlign(CalendarEdgeAlign::EDGE_ALIGN_START, offset);

    PickerTextStyle textStyle;
    calendarPickerModel.SetTextStyle(textStyle);
    auto onChange = [](const std::string& /* info */) {};
    calendarPickerModel.SetOnChange(onChange);
    calendarPickerModel.SetChangeEvent(onChange);
}

RefPtr<FrameNode> CalendarPickerTestUpdate::CalendarDialogShow(RefPtr<FrameNode> entryNode)
{
    CalendarSettingData settingData;
    DialogProperties properties;
    properties.alignment = DialogAlignment::BOTTOM;
    properties.customStyle = false;
    properties.offset = DimensionOffset(Offset(0, -1.0f));
    auto selectedDate = PickerDate(2000, 1, 1);
    settingData.selectedDate = selectedDate;
    settingData.dayRadius = TEST_SETTING_RADIUS;
    settingData.entryNode = entryNode;
    std::map<std::string, NG::DialogEvent> dialogEvent;
    std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent;
    std::vector<ButtonInfo> buttonInfos;
    auto dialogNode = CalendarDialogView::Show(properties, settingData, buttonInfos, dialogEvent, dialogCancelEvent);
    return dialogNode;
}

/**
 * @tc.name: CalendarPickerUpdateButtonStyles002
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles002, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    ButtonInfo info2;
    info1.fontWeight = FontWeight::W100;
    info2.fontWeight = FontWeight::W200;
    
    buttonInfos.push_back(info1);
    buttonInfos.push_back(info2);
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);
    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    
    size_t sizet = 0;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontWeightValue();
    EXPECT_EQ(testval, FontWeight::W100);

    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval2 = buttonLayoutProperty->GetFontWeightValue();
    EXPECT_EQ(testval2, FontWeight::W200);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles003
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles003, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontSize = Dimension(100);
    buttonInfos.push_back(info1);


    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval, Dimension(100));
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles004
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles004, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontColor = std::make_optional<Color>(Color::BLACK);
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);

    auto testval = buttonLayoutProperty->GetFontColorValue();
    EXPECT_EQ(testval, Color::BLACK);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles005
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles005, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontStyle = Ace::FontStyle::ITALIC;
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    
    auto testval = buttonLayoutProperty->GetFontStyleValue();
    EXPECT_EQ(testval, Ace::FontStyle::ITALIC);
}

/**
 * @tc.name: CalendarPickerUpdateButtonStyles006
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles006, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.type = std::make_optional<ButtonType>(ButtonType::CIRCLE);
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    
    auto testval = buttonLayoutProperty->GetTypeValue();
    EXPECT_EQ(testval, ButtonType::CIRCLE);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles007
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles007, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.buttonStyle = std::make_optional<ButtonStyleMode>(ButtonStyleMode::EMPHASIZE);
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    
    auto testval = buttonLayoutProperty->GetButtonStyleValue();
    EXPECT_EQ(testval, ButtonStyleMode::EMPHASIZE);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles008
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles008, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.role = std::make_optional<ButtonRole>(ButtonRole::ERROR);
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    
    auto testval = buttonLayoutProperty->GetButtonRoleValue();
    EXPECT_EQ(testval, ButtonRole::ERROR);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles009
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles009, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    ButtonInfo info2;
    info1.role = std::make_optional<ButtonRole>(ButtonRole::ERROR);
    info2.role = std::make_optional<ButtonRole>(ButtonRole::NORMAL);
    buttonInfos.push_back(info1);
    buttonInfos.push_back(info2);
    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetButtonRoleValue();
    EXPECT_EQ(testval, ButtonRole::ERROR);

    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval2 = buttonLayoutProperty->GetButtonRoleValue();
    EXPECT_EQ(testval2, ButtonRole::NORMAL);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles010
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles010, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    ButtonInfo info2;
    info1.buttonStyle = std::make_optional<ButtonStyleMode>(ButtonStyleMode::EMPHASIZE);
    info2.buttonStyle = ButtonStyleMode::TEXT;
    buttonInfos.push_back(info1);
    buttonInfos.push_back(info2);
    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetButtonStyleValue();
    EXPECT_EQ(testval, ButtonStyleMode::EMPHASIZE);

    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval2 = buttonLayoutProperty->GetButtonStyleValue();
    EXPECT_EQ(testval2, ButtonStyleMode::TEXT);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles011
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles011, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    ButtonInfo info2;
    info1.type = std::make_optional<ButtonType>(ButtonType::CIRCLE);
    info2.type = std::make_optional<ButtonType>(ButtonType::CAPSULE);
    buttonInfos.push_back(info1);
    buttonInfos.push_back(info2);
    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetTypeValue();
    EXPECT_EQ(testval, ButtonType::CIRCLE);

    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval2 = buttonLayoutProperty->GetTypeValue();
    EXPECT_EQ(testval2, ButtonType::CAPSULE);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles012
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles012, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    ButtonInfo info2;
    info1.fontStyle = Ace::FontStyle::ITALIC;
    info2.fontStyle = Ace::FontStyle::NONE;
    buttonInfos.push_back(info1);
    buttonInfos.push_back(info2);
    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontStyleValue();
    EXPECT_EQ(testval, Ace::FontStyle::ITALIC);

    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval2 = buttonLayoutProperty->GetFontStyleValue();
    EXPECT_EQ(testval2, Ace::FontStyle::NONE);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles013
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles013, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontStyle = Ace::FontStyle::ITALIC;
    info1.fontWeight = FontWeight::W500;
    info1.fontColor = Color::BLACK;
    buttonInfos.push_back(info1);
    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontStyleValue();
    EXPECT_EQ(testval, Ace::FontStyle::ITALIC);

    auto testval2 = buttonLayoutProperty->GetFontWeightValue();
    EXPECT_EQ(testval2, FontWeight::W500);

    auto testval3 = buttonLayoutProperty->GetFontColorValue();
    EXPECT_EQ(testval3, Color::BLACK);
}
// /**
//  * @tc.name: CalendarPickerUpdateButtonStyles014
//  * @tc.desc: Test UpdateButtonStyle.
//  * @tc.type: FUNC
//  */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles014, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.type = std::make_optional<ButtonType>(ButtonType::CIRCLE);
    info1.buttonStyle = std::make_optional<ButtonStyleMode>(ButtonStyleMode::EMPHASIZE);
    info1.role = std::make_optional<ButtonRole>(ButtonRole::NORMAL);
    buttonInfos.push_back(info1);
    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetTypeValue();
    EXPECT_EQ(testval, ButtonType::CIRCLE);

    auto testval2 = buttonLayoutProperty->GetButtonStyleValue();
    EXPECT_EQ(testval2, ButtonStyleMode::EMPHASIZE);

    auto testval3 = buttonLayoutProperty->GetButtonRoleValue();
    EXPECT_EQ(testval3, ButtonRole::NORMAL);
}
} // namespace OHOS::Ace::NG