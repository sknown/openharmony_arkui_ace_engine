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

#include <cstddef>
#include <optional>

#include "gtest/gtest.h"

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

#define protected public
#define private public
#include "core/components/button/button_theme.h"
#include "core/components/toggle/toggle_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/button/toggle_button_model_ng.h"
#include "core/components_ng/pattern/button/toggle_button_paint_property.h"
#include "core/components_ng/pattern/button/toggle_button_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr bool IS_ON = true;
constexpr Color SELECTED_COLOR = Color(0XFFFF0000);
constexpr Color BACKGROUND_COLOR = Color(0XFFFF0000);
const char TOGGLE_ETS_TAG[] = "Toggle";
const Alignment ALIGNMENT = Alignment::BOTTOM_RIGHT;
const double dimensionValue = 1.0;
const double childDimensionValue = 5.0;
constexpr float HOVER_OPACITY = 0.05f;
constexpr int32_t TOUCH_DURATION = 250;
} // namespace

struct TestProperty {
    std::optional<bool> isOn = std::nullopt;
    std::optional<Color> selectedColor = std::nullopt;
    std::optional<Color> backgroundColor = std::nullopt;
};

class ToggleButtonTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

protected:
    RefPtr<FrameNode> CreateToggleButtonFrameNode(const TestProperty& testProperty);
};

void ToggleButtonTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    // set buttonTheme to themeManager before using themeManager to get buttonTheme
    auto themeManagerOne = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManagerOne);
    EXPECT_CALL(*themeManagerOne, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<ButtonTheme>()));
}

void ToggleButtonTestNg::TearDownTestCase()
{
    MockPipelineContext::TearDown();
}

RefPtr<FrameNode> ToggleButtonTestNg::CreateToggleButtonFrameNode(const TestProperty& testProperty)
{
    ToggleButtonModelNG toggleButtonModelNG;
    toggleButtonModelNG.Create(TOGGLE_ETS_TAG);
    if (testProperty.isOn.has_value()) {
        toggleButtonModelNG.SetIsOn(testProperty.isOn.value());
    }
    if (testProperty.selectedColor.has_value()) {
        toggleButtonModelNG.SetSelectedColor(testProperty.selectedColor.value());
    }
    if (testProperty.backgroundColor.has_value()) {
        toggleButtonModelNG.SetBackgroundColor(testProperty.backgroundColor.value(), true);
    }

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    return AceType::DynamicCast<FrameNode>(element);
}

/**
 * @tc.name: ToggleButtonPatternTest001
 * @tc.desc: Test all the properties of bubble.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonTestNg, ToggleButtonPatternTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize all properties of bubble.
     */
    TestProperty testProperty;
    testProperty.isOn = std::make_optional(IS_ON);
    testProperty.selectedColor = std::make_optional(SELECTED_COLOR);
    testProperty.backgroundColor = std::make_optional(BACKGROUND_COLOR);

    /**
     * @tc.steps: step2. create list frameNode and get LayoutProperty and paintProperty.
     * @tc.expected: step2. get paintProperty success.
     */
    RefPtr<FrameNode> frameNode = CreateToggleButtonFrameNode(testProperty);
    ASSERT_NE(frameNode, nullptr);
    EXPECT_EQ(frameNode->GetTag(), V2::TOGGLE_ETS_TAG);
    auto pattern = AceType::DynamicCast<ToggleButtonPattern>(frameNode->GetPattern());
    ASSERT_NE(pattern, nullptr);
    auto paintProperty = pattern->GetPaintProperty<ToggleButtonPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto toggleButtonEventHub = frameNode->GetEventHub<ToggleButtonEventHub>();
    CHECK_NULL_VOID(toggleButtonEventHub);

    /**
     * @tc.steps: step3. get the properties of all settings.
     * @tc.expected: step3. check whether the properties is correct.
     */
    EXPECT_EQ(paintProperty->GetIsOn(), IS_ON);
    EXPECT_EQ(paintProperty->GetSelectedColor(), SELECTED_COLOR);
    EXPECT_EQ(paintProperty->GetBackgroundColor(), BACKGROUND_COLOR);
}

/**
 * @tc.name: ToggleButtonPatternTest002
 * @tc.desc: test toggleButton pattern OnModifyDone .
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonTestNg, ToggleButtonPatternTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create bubble and get frameNode.
     */
    TestProperty testProperty;
    testProperty.isOn = std::make_optional(IS_ON);
    testProperty.selectedColor = std::make_optional(SELECTED_COLOR);
    testProperty.backgroundColor = std::make_optional(BACKGROUND_COLOR);
    RefPtr<FrameNode> frameNode = CreateToggleButtonFrameNode(testProperty);
    ASSERT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. get pattern and update frameNode.
     * @tc.expected: step2. related function is called.
     */
    auto togglePattern = AceType::DynamicCast<ToggleButtonPattern>(frameNode->GetPattern());
    ASSERT_NE(togglePattern, nullptr);

    // set toggleTheme to themeManager before using themeManager to get toggleTheme
    auto themeManagerSecond = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManagerSecond);
    EXPECT_CALL(*themeManagerSecond, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<ToggleTheme>()));

    /**
     * @tc.steps: step3. pattern OnModifyDone and OnClick.
     * @tc.expected: step3. check whether the function is executed successfully.
     */
    togglePattern->isOn_ = false;
    togglePattern->OnModifyDone();
}

/**
 * @tc.name: ToggleButtonPatternTest003
 * @tc.desc: test toggleButton pattern OnModifyDone and onClick.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonTestNg, ToggleButtonPatternTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create bubble and get frameNode.
     */
    TestProperty testProperty;
    testProperty.isOn = std::make_optional(IS_ON);
    RefPtr<FrameNode> frameNode = CreateToggleButtonFrameNode(testProperty);
    ASSERT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. get pattern and update frameNode.
     * @tc.expected: step2. related function is called.
     */
    auto togglePattern = AceType::DynamicCast<ToggleButtonPattern>(frameNode->GetPattern());
    ASSERT_NE(togglePattern, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<ToggleButtonPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty();
    paintProperty->UpdateSelectedColor(SELECTED_COLOR);
    paintProperty->UpdateBackgroundColor(BACKGROUND_COLOR);

    // set toggleTheme to themeManager before using themeManager to get toggleTheme
    auto themeManagerSecond = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManagerSecond);
    EXPECT_CALL(*themeManagerSecond, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<ToggleTheme>()));

    /**
     * @tc.steps: step3. pattern OnModifyDone and OnClick.
     * @tc.expected: step3. check whether the function is executed successfully.
     */
    togglePattern->OnModifyDone();
    togglePattern->OnClick();
    layoutProperty->UpdateAlignment(ALIGNMENT);
    EXPECT_EQ(ALIGNMENT, layoutProperty->GetPositionProperty()->GetAlignmentValue());

    // update isOn value
    togglePattern->isOn_ = false;
    togglePattern->OnModifyDone();
    paintProperty->UpdateIsOn(false);
    togglePattern->OnClick();
}

/**
 * @tc.name: ToggleButtonPatternTest004
 * @tc.desc: test toggleButton pattern OnKeyEvent.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonTestNg, ToggleButtonPatternTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create and get frameNode.
     */
    TestProperty testProperty;
    testProperty.isOn = std::make_optional(IS_ON);
    RefPtr<FrameNode> frameNode = CreateToggleButtonFrameNode(testProperty);
    ASSERT_NE(frameNode, nullptr);
    auto togglePattern = AceType::DynamicCast<ToggleButtonPattern>(frameNode->GetPattern());
    ASSERT_NE(togglePattern, nullptr);

    /**
     * @tc.steps: step2. call pattern OnKeyEvent.
     * @tc.expected: step2. check whether the function is executed successfully.
     */
    auto event = KeyEvent();
    event.action = KeyAction::UNKNOWN;
    EXPECT_FALSE(togglePattern->OnKeyEvent(event));
    event.action = KeyAction::DOWN;
    event.code = KeyCode::KEY_BRIGHTNESS_DOWN;
    EXPECT_FALSE(togglePattern->OnKeyEvent(event));
    event.code = KeyCode::KEY_SPACE;
    EXPECT_TRUE(togglePattern->OnKeyEvent(event));
}

HWTEST_F(ToggleButtonTestNg, ToggleButtonPatternTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create bubble and get frameNode.
     */
    TestProperty testProperty;
    testProperty.isOn = std::make_optional(IS_ON);
    RefPtr<FrameNode> frameNode = CreateToggleButtonFrameNode(testProperty);
    ASSERT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. get pattern and update frameNode.
     * @tc.expected: step2. related function is called.
     */
    auto togglePattern = AceType::DynamicCast<ToggleButtonPattern>(frameNode->GetPattern());
    ASSERT_NE(togglePattern, nullptr);

    // set toggleTheme to themeManager before using themeManager to get toggleTheme
    auto themeManagerSecond = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManagerSecond);
    EXPECT_CALL(*themeManagerSecond, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<ToggleTheme>()));

    /**
     * @tc.steps: step3. pattern OnModifyDone.
     * @tc.expected: step3. check whether the function is executed successfully.
     */
    auto eventHub = frameNode->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    eventHub->SetEnabled(false);
    togglePattern->OnModifyDone();
}

HWTEST_F(ToggleButtonTestNg, ToggleButtonPatternTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create bubble and get frameNode.
     */
    TestProperty testProperty;
    testProperty.isOn = std::make_optional(IS_ON);
    RefPtr<FrameNode> frameNode = CreateToggleButtonFrameNode(testProperty);
    ASSERT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. get pattern and update frameNode.
     * @tc.expected: step2. related function is called.
     */
    auto togglePattern = AceType::DynamicCast<ToggleButtonPattern>(frameNode->GetPattern());
    ASSERT_NE(togglePattern, nullptr);

    // set toggleTheme to themeManager before using themeManager to get toggleTheme
    auto themeManagerSecond = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManagerSecond);
    EXPECT_CALL(*themeManagerSecond, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<ToggleTheme>()));

    /**
     * @tc.steps: step3. pattern InitClickEvent.
     * @tc.expected: step3. check whether clickListener_ is initialized.
     */
    EXPECT_TRUE(togglePattern->clickListener_);
}

HWTEST_F(ToggleButtonTestNg, ToggleButtonPatternTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create bubble and get frameNode.
     */
    TestProperty testProperty;
    testProperty.isOn = std::make_optional(IS_ON);
    RefPtr<FrameNode> frameNode = CreateToggleButtonFrameNode(testProperty);
    ASSERT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. get pattern and update frameNode.
     * @tc.expected: step2. related function is called.
     */
    auto togglePattern = AceType::DynamicCast<ToggleButtonPattern>(frameNode->GetPattern());
    ASSERT_NE(togglePattern, nullptr);

    // set toggleTheme to themeManager before using themeManager to get toggleTheme
    auto themeManagerSecond = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManagerSecond);
    EXPECT_CALL(*themeManagerSecond, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<ToggleTheme>()));

    /**
     * @tc.steps: step3. Creat child node but not set font size.
     * @tc.expected: step3. Child frame node's font size synchronized with parent.
     */
    Dimension buttonFontSize(dimensionValue);
    Dimension childFontSize(childDimensionValue);
    RefPtr<FrameNode> childrenNode =
        FrameNode::GetOrCreateFrameNode("childTag", 1, []() { return AceType::MakeRefPtr<TextPattern>(); });
    frameNode->children_.emplace_back(childrenNode);
    togglePattern->textFontSize_ = buttonFontSize;
    togglePattern->OnModifyDone();
    auto textNode = AceType::DynamicCast<FrameNode>(frameNode->GetFirstChild());
    ASSERT_NE(textNode, nullptr);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);
    EXPECT_EQ(textLayoutProperty->GetFontSizeValue(childFontSize).Value(), childDimensionValue);

    /**
     * @tc.steps: step4. Creat child node and set font size.
     * @tc.expected: step4. Parent frame node's font size synchronized with child.
     */
    frameNode->children_.clear();
    auto childrenTextNode = AceType::DynamicCast<FrameNode>(childrenNode);
    ASSERT_NE(childrenTextNode, nullptr);
    auto childrenTextLayoutProperty = childrenTextNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(childrenTextLayoutProperty, nullptr);
    childrenTextLayoutProperty->UpdateFontSize(childFontSize);
    frameNode->children_.emplace_back(childrenNode);
    togglePattern->isOn_ = false;
    togglePattern->OnModifyDone();
    auto layoutProperty = frameNode->GetLayoutProperty<ButtonLayoutProperty>();
    EXPECT_EQ(layoutProperty->GetFontSizeValue(buttonFontSize).Value(), childDimensionValue);
}

HWTEST_F(ToggleButtonTestNg, ToggleButtonPatternTest008, TestSize.Level1)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(TOGGLE_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<TextPattern>(); });
    stack->Push(frameNode);
    ToggleButtonModelNG::SetBackgroundColor(BACKGROUND_COLOR, true);
    auto context = frameNode->GetRenderContext();
    EXPECT_EQ(context->GetBackgroundColorValue(), BACKGROUND_COLOR);
}

/**
 * @tc.name: ToggleButtonPatternTest009
 * @tc.desc: test button pattern AnimateTouchAndHover.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonTestNg, ToggleButtonPatternTest009, TestSize.Level1)
{
    TestProperty testProperty;
    testProperty.isOn = std::make_optional(IS_ON);
    RefPtr<FrameNode> frameNode = CreateToggleButtonFrameNode(testProperty);
    ASSERT_NE(frameNode, nullptr);
    auto buttonPattern = frameNode->GetPattern<ToggleButtonPattern>();
    ASSERT_NE(buttonPattern, nullptr);
    auto context = frameNode->GetRenderContext();
    ASSERT_NE(context, nullptr);
    buttonPattern->AnimateTouchAndHover(context, 0.0f, HOVER_OPACITY, TOUCH_DURATION, Curves::FRICTION);
    buttonPattern->AnimateTouchAndHover(context, HOVER_OPACITY, 0.0f, TOUCH_DURATION, Curves::FRICTION);
    EXPECT_TRUE(buttonPattern->isOn_);
}

/**
 * @tc.name: ToggleButtonPatternTest010
 * @tc.desc: Test the Distributed capability of ToggleButton.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonTestNg, ToggleButtonPatternTest010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create bubble and get frameNode.
     */
    TestProperty testProperty;
    testProperty.isOn = std::make_optional(IS_ON);
    RefPtr<FrameNode> frameNode = CreateToggleButtonFrameNode(testProperty);
    ASSERT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. get pattern and update frameNode.
     * @tc.expected: function ProvideRestoreInfo is called.
     */
    auto buttonPattern = frameNode->GetPattern<ToggleButtonPattern>();
    ASSERT_NE(buttonPattern, nullptr);
    buttonPattern->isOn_ = false;
    auto ret = buttonPattern->ProvideRestoreInfo();
    EXPECT_TRUE(ret == R"({"IsOn":false})");

    /**
     * @tc.steps: step3. function OnRestoreInfo is called.
     * @tc.expected: Passing invalid & valid JSON format.
     */
    std::string restoreInfo_ = "invalid_json_string";
    buttonPattern->OnRestoreInfo(restoreInfo_);
    auto toggleButtonPaintProperty = buttonPattern->GetPaintProperty<ToggleButtonPaintProperty>();
    ASSERT_NE(toggleButtonPaintProperty, nullptr);
    EXPECT_TRUE(toggleButtonPaintProperty->GetIsOnValue(false));
    restoreInfo_ = R"({"IsOn":true})";
    buttonPattern->OnRestoreInfo(restoreInfo_);
    ASSERT_NE(toggleButtonPaintProperty, nullptr);
    EXPECT_TRUE(toggleButtonPaintProperty->GetIsOnValue(false));
}

/**
 * @tc.name: ToggleButtonPatternTest011
 * @tc.desc: ToggleButton test Select and ClearSelection.
 */
HWTEST_F(ToggleButtonTestNg, ToggleButtonPatternTest011, TestSize.Level1)
{
    ToggleButtonModelNG toggleButtonModelNG;
    toggleButtonModelNG.Create(TOGGLE_ETS_TAG);
    toggleButtonModelNG.SetIsOn(true);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<ToggleButtonPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->isOn_ = false;
    pattern->SetAccessibilityAction();

    auto accessibilityProperty = frameNode->GetAccessibilityProperty<ToggleButtonAccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    EXPECT_TRUE(accessibilityProperty->ActActionSelect());

    bool isSelected = true;
    pattern->isOn_ = false;
    pattern->UpdateSelectStatus(isSelected);
    EXPECT_TRUE(accessibilityProperty->ActActionClearSelection());
}
} // namespace OHOS::Ace::NG
