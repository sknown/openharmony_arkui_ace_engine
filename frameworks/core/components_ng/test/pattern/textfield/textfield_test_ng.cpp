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

#include "gtest/gtest.h"

#define private public
#define protected public
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/text_field/text_editing_value_ng.h"
#include "core/components_ng/pattern/text_field/text_field_accessibility_property.h"
#include "core/components_ng/pattern/text_field/text_field_content_modifier.h"
#include "core/components_ng/pattern/text_field/text_field_controller.h"
#include "core/components_ng/pattern/text_field/text_field_event_hub.h"
#include "core/components_ng/pattern/text_field/text_field_layout_algorithm.h"
#include "core/components_ng/pattern/text_field/text_field_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_manager.h"
#include "core/components_ng/pattern/text_field/text_field_model_ng.h"
#include "core/components_ng/pattern/text_field/text_field_paint_method.h"
#include "core/components_ng/pattern/text_field/text_field_paint_property.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/pattern/text_field/text_selector.h"
#include "core/components_ng/test/mock/pattern/text_field/mock_text_input_connection.h"
#include "core/components_ng/test/mock/rosen/mock_canvas.h"
#include "core/components_ng/test/mock/render/mock_render_context.h"
#include "core/components_ng/test/mock/theme/mock_theme_manager.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/test/mock/mock_pipeline_base.h"
#undef private
#undef protected
#include "core/components_ng/base/view_stack_processor.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::string TEXT_VALUE = "DEFAULT_TEXT";
const std::string INSERT_VALUE_SINGLE_NUMBER = "1";
const std::string INSERT_VALUE_SINGLE_CHAR = "X";
const std::string LOWERCASE_FILTER = "[a-z]";
const std::string NUMBER_FILTER = "^[0-9]*$";
const int32_t CARET_POSITION_1 = 10;
const int32_t CARET_POSITION_2 = 12;
const int32_t DELETE_LENGTH_1 = 1;
const int32_t CARET_POSITION = 0;
constexpr int32_t TEXT_SELECTION_ERR = -1;
constexpr int32_t TEXT_SELECTION_START = 0;
constexpr int32_t TEXT_SELECTION_END = 10;
const Dimension CURSOR_WIDTH_SIZE = 10.0_vp;
const Dimension DEFAULT_FONT_SIZE { 14, DimensionUnit::PX };
const SizeF CONTENT_SIZE = SizeF(400.0, 500.0);
const SizeF CONTENT_SIZE_FOR_SCROLL = SizeF(400.0, 20.0);
const SizeF TEXT_SIZE_FOR_SCROLL = SizeF(400.0, 40.0);
const OffsetF CONTENT_OFFSET = OffsetF(50.0, 60.0);
const Axis AXIS_VERTICAL = Axis::VERTICAL;
constexpr float CONTEXT_WIDTH_VALUE = 10.0f;
constexpr float CONTEXT_HEIGHT_VALUE = 10.0f;
constexpr float CONTENT_RECT_HEIGHT = 15.0f;
constexpr float CONTENT_RECT_WIDTH = 10.0f;
constexpr float CONTENT_RECT_Y = 5.0f;
constexpr float CONTENT_RECT_Y_LARGE = 50.0f;
constexpr float TEXT_AREA_SCROLL_OFFSET = 10.0f;
constexpr float TEXT_RECT_HEIGHT = 10.0f;
constexpr float TEXT_RECT_HEIGHT_LARGE = 20.0f;
constexpr float TEXT_RECT_WIDTH = 5.0f;
constexpr float TEXT_RECT_Y = 5.0f;
constexpr float TEXT_RECT_Y_LARGE = 50.0f;
constexpr float DEVICE_WIDTH = 480.f;
constexpr float DEVICE_HEIGHT = 800.f;
const std::string EMPTY_TEXT_VALUE;
const std::string TEXT_EDITING_VALUE("textEditingValue");
const std::string PLACEHOLDER("DEFAULT PLACEHOLDER");
const std::string PLACE_HOLDER_TEXT("Placeholdertext");
const std::string TEXT_CONTENT("Textvalue");
const std::string DEFAULT_PASSWORD = "******";
constexpr int32_t SELECT_OVERLAY_ID = 143;
} // namespace
class TextFieldPatternTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    RefPtr<FrameNode> GetFrameNode();
    RefPtr<TextFieldPattern> GetPattern();
    RefPtr<TextFieldPaintProperty> GetPaintProperty();
    RefPtr<TextFieldLayoutProperty> GetLayoutProperty();
    RefPtr<FrameNode> CreatTextFieldNode(const std::optional<std::string>& placeholder = PLACEHOLDER,
        const std::optional<std::string>& value = EMPTY_TEXT_VALUE, bool isTextArea = false);
    void RunSetUp();
    RefPtr<FrameNode> host_;
    RefPtr<TextFieldPattern> pattern_;
    RefPtr<TextFieldLayoutProperty> layoutProperty_;
};

void TextFieldPatternTestNg::SetUpTestSuite()
{
    MockPipelineBase::SetUp();
    MockPipelineBase::GetCurrent()->rootNode_ =
        FrameNode::CreateFrameNodeWithTree(V2::ROOT_ETS_TAG, 0, AceType::MakeRefPtr<RootPattern>());
}

void TextFieldPatternTestNg::TearDownTestSuite()
{
    MockPipelineBase::TearDown();
}

void TextFieldPatternTestNg::RunSetUp()
{
    host_ = CreatTextFieldNode();
    ASSERT_NE(host_, nullptr);
    pattern_ = host_->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern_, nullptr);
    layoutProperty_ = host_->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty_, nullptr);

    auto pipeline = MockPipelineBase::GetCurrent();
    auto clipboard = ClipboardProxy::GetInstance()->GetClipboard(pipeline->GetTaskExecutor());
    pattern_->clipboard_ = clipboard;
    pattern_->paragraph_ = std::make_shared<RSParagraph>();
}

RefPtr<FrameNode> TextFieldPatternTestNg::CreatTextFieldNode(
    const std::optional<std::string>& placeholder, const std::optional<std::string>& value, bool isTextArea)
{
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.CreateNode(placeholder, value, isTextArea);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    return frameNode;
}

RefPtr<TextFieldLayoutProperty> TextFieldPatternTestNg::GetLayoutProperty()
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    return frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
}

RefPtr<TextFieldPaintProperty> TextFieldPatternTestNg::GetPaintProperty()
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    return frameNode->GetPaintProperty<TextFieldPaintProperty>();
}

RefPtr<TextFieldPattern> TextFieldPatternTestNg::GetPattern()
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    return frameNode->GetPattern<TextFieldPattern>();
}

RefPtr<FrameNode> TextFieldPatternTestNg::GetFrameNode()
{
    TextFieldModelNG textFieldModelNG;
    const std::string placeholder = "DEFAULT PLACEHOLDER";
    const std::string value;
    bool isTextArea = false;
    textFieldModelNG.CreateNode(placeholder, value, isTextArea);
    textFieldModelNG.SetCaretPosition(CARET_POSITION);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    return frameNode;
}

PaddingProperty CreatePadding(float left, float top, float right, float bottom)
{
    PaddingProperty padding;
    padding.left = CalcLength(left);
    padding.right = CalcLength(right);
    padding.top = CalcLength(top);
    padding.bottom = CalcLength(bottom);
    return padding;
}

/**
 * @tc.name: TextFieldInsertValue001
 * @tc.desc: Test inserting value of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldInsertValue001, TestSize.Level1)
{
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.CreateTextInput(PLACEHOLDER, EMPTY_TEXT_VALUE);
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    pattern->InsertValue(INSERT_VALUE_SINGLE_CHAR);
    EXPECT_EQ(pattern->GetEditingValue().text, INSERT_VALUE_SINGLE_CHAR);
    EXPECT_EQ(pattern->GetEditingValue().caretPosition, INSERT_VALUE_SINGLE_CHAR.size());

    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    pattern->UpdateEditingValue(TEXT_VALUE, 0);
    layoutProperty->UpdateMaxLength(5);
    pattern->InsertValue(INSERT_VALUE_SINGLE_CHAR);
    pattern->UpdateEditingValue("TEXT", 0);
    pattern->selectionMode_ = SelectionMode::SELECT;
    pattern->textSelector_.baseOffset = 1;
    pattern->textSelector_.destinationOffset = 3;
    auto* stack = ViewStackProcessor::GetInstance();
    int32_t nodeId = stack->ClaimNodeId();
    auto searchNode = AceType::MakeRefPtr<FrameNode>(V2::SEARCH_ETS_TAG, nodeId, AceType::MakeRefPtr<Pattern>(), false);
    frameNode->MountToParent(searchNode);
    pattern->InsertValue(INSERT_VALUE_SINGLE_CHAR);
    EXPECT_EQ(pattern->GetEditingValue().text, "TXT");
}

/**
 * @tc.name: TextFieldInsertValue002
 * @tc.desc: Test textfield insert a value and clear it when type changed.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldInsertValue002, TestSize.Level1)
{
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.CreateTextInput(PLACEHOLDER, EMPTY_TEXT_VALUE);
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(frameNode);
    textFieldModelNG.SetType(TextInputType::TEXT);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->InsertValue(INSERT_VALUE_SINGLE_CHAR);
    EXPECT_EQ(pattern->GetEditingValue().text, INSERT_VALUE_SINGLE_CHAR);
    EXPECT_EQ(pattern->GetEditingValue().caretPosition, INSERT_VALUE_SINGLE_CHAR.size());
    auto secNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    textFieldModelNG.SetType(TextInputType::NUMBER);
    pattern = frameNode->GetPattern<TextFieldPattern>();
    pattern->ClearEditingValue();
    EXPECT_EQ(pattern->GetEditingValue().text.length(), 0);
}

/**
 * @tc.name: TextFieldDeleteForwardValue001
 * @tc.desc: Test deleting value of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldDeleteForwardValue001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.CreateTextInput(PLACEHOLDER, EMPTY_TEXT_VALUE);
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    pattern->InsertValue(TEXT_VALUE);
    EXPECT_EQ(pattern->GetEditingValue().text, TEXT_VALUE);
    EXPECT_EQ(pattern->GetEditingValue().caretPosition, static_cast<int32_t>(TEXT_VALUE.size()));
    pattern->DeleteForward(DELETE_LENGTH_1);
    EXPECT_EQ(pattern->GetEditingValue().text, TEXT_VALUE.substr(0, TEXT_VALUE.size()));
    EXPECT_EQ(pattern->GetEditingValue().caretPosition, TEXT_VALUE.size());

    /**
     * @tc.steps: step2. in select mode and call DeleteForward.
     * @tc.expected: Check the editting text.
     */
    pattern->selectionMode_ = SelectionMode::SELECT;
    pattern->textSelector_.baseOffset = 0;
    pattern->textSelector_.destinationOffset = 5;
    pattern->DeleteForward(DELETE_LENGTH_1);
    EXPECT_EQ(pattern->GetEditingValue().text, TEXT_VALUE.substr(5, TEXT_VALUE.size()));

    /**
     * @tc.steps: step3. not in select mode and call DeleteForward.
     * @tc.expected: Check the editting text.
     */
    pattern->selectionMode_ = SelectionMode::NONE;
    auto deleteLen = 2;
    auto caretPosition = 4;
    pattern->textEditingValue_.caretPosition = caretPosition;
    std::string expectStr =
        TEXT_VALUE.substr(0, caretPosition) + TEXT_VALUE.substr(caretPosition + deleteLen, TEXT_VALUE.size());

    pattern->textEditingValue_.text = TEXT_VALUE;
    pattern->DeleteForward(deleteLen);

    pattern->textEditingValue_.text = TEXT_VALUE;
    layoutProperty->UpdateMaxLines(2);
    layoutProperty->UpdateMaxLength(100);
    pattern->DeleteForward(deleteLen);

    pattern->textEditingValue_.text = TEXT_VALUE;
    layoutProperty->ResetMaxLength();
    pattern->DeleteForward(deleteLen);
    EXPECT_EQ(pattern->GetEditingValue().text, expectStr);
}

/**
 * @tc.name: DeleteBackward
 * @tc.desc: Test deleting value of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, DeleteBackward, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. in select mode and call DeleteBackward.
     * @tc.expected: Check the editting text.
     */
    pattern->UpdateEditingValue(TEXT_VALUE, 3);
    pattern->selectionMode_ = SelectionMode::SELECT;
    pattern->textSelector_.baseOffset = 0;
    pattern->textSelector_.destinationOffset = 5;
    pattern->DeleteBackward(DELETE_LENGTH_1);
    EXPECT_EQ(pattern->GetEditingValue().text, TEXT_VALUE.substr(5, TEXT_VALUE.size()));

    /**
     * @tc.steps: step3. not in select mode and call DeleteBackward.
     * @tc.expected: Check the editting text.
     */
    pattern->UpdateEditingValue(TEXT_VALUE, 0);
    pattern->selectionMode_ = SelectionMode::NONE;
    auto deleteLen = 2;
    auto caretPosition = 4;
    std::string expectStr =
        TEXT_VALUE.substr(0, caretPosition - deleteLen) + TEXT_VALUE.substr(caretPosition, TEXT_VALUE.size());
    pattern->DeleteBackward(deleteLen);
    layoutProperty->UpdateMaxLines(1);
    pattern->UpdateEditingValue(TEXT_VALUE, caretPosition);
    pattern->DeleteBackward(deleteLen);
    pattern->UpdateEditingValue(TEXT_VALUE, caretPosition);
    layoutProperty->UpdateMaxLines(2);
    layoutProperty->UpdateMaxLength(100);
    pattern->DeleteBackward(deleteLen);
    pattern->UpdateEditingValue(TEXT_VALUE, caretPosition);
    layoutProperty->ResetMaxLength();
    pattern->DeleteBackward(deleteLen);
    EXPECT_EQ(pattern->GetEditingValue().text, expectStr);
}

/**
 * @tc.name: TextFieldMoveCaret001
 * @tc.desc: Test textfield move caret.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldMoveCaret001, TestSize.Level1)
{
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.CreateTextInput(PLACEHOLDER, EMPTY_TEXT_VALUE);
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    pattern->InsertValue(TEXT_VALUE);
    EXPECT_EQ(pattern->GetEditingValue().text, TEXT_VALUE);
    pattern->SetCaretPosition(CARET_POSITION_1);
    // inserting text value length larger than caret position
    EXPECT_EQ(pattern->GetEditingValue().caretPosition, CARET_POSITION_1);
}

/**
 * @tc.name: TextFieldFilter001
 * @tc.desc: Test textfield filter.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldFilter001, TestSize.Level1)
{
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.CreateTextInput(PLACEHOLDER, EMPTY_TEXT_VALUE);
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateInputFilter(LOWERCASE_FILTER);
    pattern->InsertValue(INSERT_VALUE_SINGLE_CHAR);
    EXPECT_EQ(pattern->GetEditingValue().text, EMPTY_TEXT_VALUE);
    EXPECT_EQ(pattern->GetEditingValue().caretPosition, EMPTY_TEXT_VALUE.size());
}

/**
 * @tc.name: TextFieldFilter002
 * @tc.desc: Test textfield filter.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldFilter002, TestSize.Level1)
{
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.CreateTextInput(PLACEHOLDER, EMPTY_TEXT_VALUE);
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateInputFilter(NUMBER_FILTER);
    pattern->InsertValue(INSERT_VALUE_SINGLE_NUMBER);
    EXPECT_EQ(pattern->GetEditingValue().text, INSERT_VALUE_SINGLE_NUMBER);
    EXPECT_EQ(pattern->GetEditingValue().caretPosition, INSERT_VALUE_SINGLE_NUMBER.size());
}

/**
 * @tc.name: TextareaMoveCaret001
 * @tc.desc: Test textarea move caret.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextareaMoveCaret001, TestSize.Level1)
{
    TextFieldModelNG textFieldModelNG;
    auto controller = AccessibilityManager::DynamicCast<TextFieldController>(
        textFieldModelNG.CreateTextArea(PLACEHOLDER, EMPTY_TEXT_VALUE));
    ASSERT_NE(controller, nullptr);
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    pattern->InsertValue(TEXT_VALUE);
    EXPECT_EQ(pattern->GetEditingValue().text, TEXT_VALUE);
    controller->CaretPosition(CARET_POSITION_1);
    // inserting text value length larger than caret position
    EXPECT_EQ(pattern->GetEditingValue().caretPosition, CARET_POSITION_2);
}

/**
 * @tc.name: FilterWithRegex001
 * @tc.desc: test FilterWithRegex
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, FilterWithRegex001, TestSize.Level1)
{
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    std::string result;
    auto funcReturn = textFieldPattern->FilterWithRegex("test", "filter_valuetest", result, true);
    EXPECT_TRUE(funcReturn);
    EXPECT_EQ("test", result);
}

/**
 * @tc.name: FilterWithRegex002
 * @tc.desc: test FilterWithRegex
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, FilterWithRegex002, TestSize.Level2)
{
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    std::string result;
    auto funcReturn = textFieldPattern->FilterWithRegex("filter_value", "", result, true);
    EXPECT_FALSE(funcReturn);
    funcReturn = textFieldPattern->FilterWithRegex("filter_value", "", result);
    EXPECT_FALSE(funcReturn);
    EXPECT_TRUE(result.empty());
    funcReturn = textFieldPattern->FilterWithRegex("", "", result);
    EXPECT_FALSE(funcReturn);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: EditingValueFilter001
 * @tc.desc: test EditingValueFilter
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, EditingValueFilter001, TestSize.Level1)
{
    auto layoutProperty = GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateTextInputType(TextInputType::NUMBER);
    std::string result;
    std::string valueToUpdate = "filter_value1test";
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    textFieldPattern->EditingValueFilter(valueToUpdate, result);
    EXPECT_EQ(valueToUpdate, "filter_value1test");
    layoutProperty->UpdateInputFilter("test");
    result = "";
    valueToUpdate = "filter_value1test";
    textFieldPattern->EditingValueFilter(valueToUpdate, result);
    EXPECT_EQ(valueToUpdate, "test");
}

/**
 * @tc.name: EditingValueFilter002
 * @tc.desc: test EditingValueFilter
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, EditingValueFilter002, TestSize.Level1)
{
    auto layoutProperty = GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateTextInputType(TextInputType::PHONE);
    std::string result;
    std::string valueToUpdate = "filter_value\\dtest";
    layoutProperty->UpdateInputFilter("test");
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    textFieldPattern->EditingValueFilter(valueToUpdate, result);
    EXPECT_EQ(valueToUpdate, "test");
}

/**
 * @tc.name: EditingValueFilter003
 * @tc.desc: test EditingValueFilter
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, EditingValueFilter003, TestSize.Level1)
{
    auto layoutProperty = GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateTextInputType(TextInputType::EMAIL_ADDRESS);
    std::string result;
    std::string valueToUpdate = "filter_valuew+test";
    layoutProperty->UpdateInputFilter("test");
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    textFieldPattern->EditingValueFilter(valueToUpdate, result);
    EXPECT_EQ(valueToUpdate, "test");
}

/**
 * @tc.name: EditingValueFilter004
 * @tc.desc: test EditingValueFilter
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, EditingValueFilter004, TestSize.Level1)
{
    auto layoutProperty = GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateTextInputType(TextInputType::URL);
    std::string result;
    std::string valueToUpdate = "filter_value//test";
    layoutProperty->UpdateInputFilter("test");
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    textFieldPattern->EditingValueFilter(valueToUpdate, result);
    EXPECT_EQ(valueToUpdate, "test");
}

/**
 * @tc.name: GetTextOrPlaceHolderFontSize001
 * @tc.desc: test GetTextOrPlaceHolderFontSize
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetTextOrPlaceHolderFontSize001, TestSize.Level2)
{
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    auto size = textFieldPattern->GetTextOrPlaceHolderFontSize();
    EXPECT_EQ(size, 0.0f);
}

/**
 * @tc.name: UpdateCaretRect
 * @tc.desc: test UpdateCaretRect
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateCaretRect, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    frameNode->GetOrCreateFocusHub()->currentFocus_ = true;

    /**
     * @tc.steps: step2. set clipboard avoid nullptr and call UpdateCaretRect.
     * @tc.expected: Check it is not nullptr.
     */
    auto pipeline = MockPipelineBase::GetCurrent();
    auto clipboard = ClipboardProxy::GetInstance()->GetClipboard(pipeline->GetTaskExecutor());
    textFieldPattern->clipboard_ = clipboard;

    CaretUpdateType exptectFalseTypes[] = { CaretUpdateType::INPUT, CaretUpdateType::PRESSED,
        CaretUpdateType::LONG_PRESSED, CaretUpdateType::EVENT, CaretUpdateType::DEL, CaretUpdateType::ICON_PRESSED,
        CaretUpdateType::RIGHT_CLICK, CaretUpdateType::HANDLE_MOVE };
    for (auto caretType : exptectFalseTypes) {
        textFieldPattern->caretUpdateType_ = caretType;
        EXPECT_FALSE(textFieldPattern->UpdateCaretRect());
    }

    textFieldPattern->caretUpdateType_ = CaretUpdateType::PRESSED;
    textFieldPattern->isMousePressed_ = true;
    EXPECT_FALSE(textFieldPattern->UpdateCaretRect());

    textFieldPattern->caretUpdateType_ = CaretUpdateType::ICON_PRESSED;
    textFieldPattern->selectionMode_ = SelectionMode::SELECT;
    textFieldPattern->textSelector_.baseOffset = 0;
    textFieldPattern->textSelector_.destinationOffset = 5;
    EXPECT_FALSE(textFieldPattern->UpdateCaretRect());
    textFieldPattern->UpdateEditingValue("", 0);
    EXPECT_FALSE(textFieldPattern->UpdateCaretRect());

    textFieldPattern->caretUpdateType_ = CaretUpdateType::NONE;
    textFieldPattern->UpdateEditingValue(TEXT_VALUE, 0);
    EXPECT_TRUE(textFieldPattern->UpdateCaretRect());

    /**
     * @tc.steps: step3. when textEditing empty, return false
     * @tc.expected: Check it is not nullptr.
     */
    textFieldPattern->UpdateEditingValue("", 0);
    textFieldPattern->caretUpdateType_ = CaretUpdateType::NONE;
    EXPECT_FALSE(textFieldPattern->UpdateCaretRect());

    textFieldPattern->caretUpdateType_ = CaretUpdateType::EVENT;
    layoutProperty->UpdateShowPasswordIcon(false);
    textFieldPattern->needToRefreshSelectOverlay_ = false;
    textFieldPattern->cursorVisible_ = true;
    textFieldPattern->isMousePressed_ = true;
    textFieldPattern->UpdateEditingValue(TEXT_VALUE, 0);
    EXPECT_FALSE(textFieldPattern->UpdateCaretRect());

    textFieldPattern->caretUpdateType_ = CaretUpdateType::PRESSED;
    textFieldPattern->isMousePressed_ = false;
    textFieldPattern->isFocusedBeforeClick_ = true;
    EXPECT_FALSE(textFieldPattern->UpdateCaretRect());

    /**
     * @tc.steps: step3. set in search node and call UpdateCaretRect.
     * @tc.expected: Check it is not nullptr.
     */
    auto* stack = ViewStackProcessor::GetInstance();
    int32_t nodeId = stack->ClaimNodeId();
    auto searchNode = AceType::MakeRefPtr<FrameNode>(V2::SEARCH_ETS_TAG, nodeId, AceType::MakeRefPtr<Pattern>(), false);
    frameNode->MountToParent(searchNode);
    textFieldPattern->caretUpdateType_ = CaretUpdateType::HANDLE_MOVE;
    EXPECT_TRUE(textFieldPattern->UpdateCaretRect());
}

/**
 * @tc.name: AdjustTextRectOffset
 * @tc.desc: test AdjustTextRectOffset.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, AdjustTextRectOffset, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. set isTextArea to true and call AdjustTextRectOffsetX,AdjustTextAreaOffsetY.
     * @tc.expected: Check the return value.
     */
    layoutProperty->UpdateMaxLines(2);
    EXPECT_EQ(pattern->AdjustTextRectOffsetX(), 0.0f);
    pattern->caretRect_.SetTop(10);
    pattern->contentRect_.SetTop(8);
    pattern->contentRect_.SetHeight(4);
    EXPECT_EQ(pattern->AdjustTextAreaOffsetY(), 2.0f);
    pattern->caretRect_.SetTop(5);
    EXPECT_EQ(pattern->AdjustTextAreaOffsetY(), 7.0f);
    pattern->caretRect_.SetTop(13);
    EXPECT_EQ(pattern->AdjustTextAreaOffsetY(), -1.0f);

    /**
     * @tc.steps: step3. set isTextArea to false and call AdjustTextRectOffsetX,AdjustTextAreaOffsetY.
     * @tc.expected: Check the return value.
     */
    layoutProperty->UpdateMaxLines(1);
    EXPECT_EQ(pattern->AdjustTextAreaOffsetY(), 0.0f);
    EXPECT_EQ(pattern->AdjustTextRectOffsetX(), 0.0f);
    pattern->caretRect_.SetLeft(10);
    pattern->contentRect_.SetLeft(8);
    pattern->contentRect_.SetWidth(4);
    EXPECT_EQ(pattern->AdjustTextRectOffsetX(), 0.0f);
    pattern->caretRect_.SetLeft(7);
    EXPECT_EQ(pattern->AdjustTextRectOffsetX(), 1.0f);
    pattern->caretRect_.SetLeft(15);
    EXPECT_EQ(pattern->AdjustTextRectOffsetX(), -3.0f);
}

/**
 * @tc.name: IsTextArea001
 * @tc.desc: test IsTextArea
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, IsTextArea001, TestSize.Level1)
{
    auto layoutProperty = GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    EXPECT_FALSE(textFieldPattern->IsTextArea());
    layoutProperty->UpdateMaxLines(1);
    EXPECT_FALSE(textFieldPattern->IsTextArea());
    layoutProperty->UpdateMaxLines(2);
    EXPECT_TRUE(textFieldPattern->IsTextArea());
}

/**
 * @tc.name: UpdateDestinationToCaretByEvent001
 * @tc.desc: test UpdateDestinationToCaretByEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateDestinationToCaretByEvent001, TestSize.Level1)
{
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    textFieldPattern->isMousePressed_ = true;
    int32_t len = static_cast<int32_t>(TEXT_VALUE.size());
    textFieldPattern->UpdateEditingValue(TEXT_VALUE, len);
    textFieldPattern->selectionMode_ = SelectionMode::NONE;
    textFieldPattern->textSelector_.baseOffset = len;
    textFieldPattern->UpdateDestinationToCaretByEvent();
    EXPECT_EQ(textFieldPattern->GetSelectMode(), SelectionMode::NONE);
    textFieldPattern->textSelector_.baseOffset = 0;
    textFieldPattern->UpdateDestinationToCaretByEvent();
    EXPECT_EQ(textFieldPattern->GetSelectMode(), SelectionMode::SELECT);
}

/**
 * @tc.name: UpdateCaretPositionByLastTouchOffset001
 * @tc.desc: test UpdateCaretPositionByLastTouchOffset
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateCaretPositionByLastTouchOffset001, TestSize.Level1)
{
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    SizeF contentSize(730.0, 160.0);
    SizeF textSize(720.0, 150.0);
    textFieldPattern->contentRect_.Reset();
    textFieldPattern->textRect_.Reset();
    textFieldPattern->caretRect_.Reset();
    textFieldPattern->contentRect_.SetSize(contentSize);
    textFieldPattern->textRect_.SetSize(textSize);
    textFieldPattern->InitEditingValueText(TEXT_VALUE);
    textFieldPattern->lastTouchOffset_ = Offset(725.0, 0.0);
    textFieldPattern->UpdateCaretPositionByLastTouchOffset();
    EXPECT_EQ(textFieldPattern->GetTextEditingValue().caretPosition, 0);
    EXPECT_EQ(textFieldPattern->GetCaretOffsetX(), 0);
}

/**
 * @tc.name: UpdateCaretPositionByLastTouchOffset002
 * @tc.desc: test UpdateCaretPositionByLastTouchOffset
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateCaretPositionByLastTouchOffset002, TestSize.Level1)
{
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    SizeF contentSize(730.0, 160.0);
    SizeF textSize(720.0, 150.0);
    OffsetF textOffset(10.0, 0.0);
    textFieldPattern->contentRect_.Reset();
    textFieldPattern->textRect_.Reset();
    textFieldPattern->caretRect_.Reset();
    textFieldPattern->contentRect_.SetSize(contentSize);
    textFieldPattern->textRect_.SetSize(textSize);
    textFieldPattern->textRect_.SetOffset(textOffset);
    textFieldPattern->InitEditingValueText(TEXT_VALUE);
    textFieldPattern->UpdateCaretPositionByLastTouchOffset();
    EXPECT_EQ(textFieldPattern->GetTextEditingValue().caretPosition, 0);
}

/**
 * @tc.name: UpdateCaretPositionByLastTouchOffset003
 * @tc.desc: test UpdateCaretPositionByLastTouchOffset
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateCaretPositionByLastTouchOffset003, TestSize.Level2)
{
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    SizeF contentSize(710.0, 160.0);
    SizeF textSize(720.0, 150.0);
    textFieldPattern->contentRect_.Reset();
    textFieldPattern->textRect_.Reset();
    textFieldPattern->caretRect_.Reset();
    textFieldPattern->contentRect_.SetSize(contentSize);
    textFieldPattern->textRect_.SetSize(textSize);
    textFieldPattern->InitEditingValueText(TEXT_VALUE);
    textFieldPattern->UpdateCaretPositionByLastTouchOffset();
    EXPECT_EQ(textFieldPattern->GetTextEditingValue().caretPosition, 0);
    EXPECT_EQ(textFieldPattern->GetCaretOffsetX(), 0.0);
}

/**
 * @tc.name: CursorWidth001
 * @tc.desc: Verify that the CursorWidth interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CursorWidth001, TestSize.Level1)
{
    auto frameNode = GetFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(frameNode->GetPattern());
    ASSERT_NE(textFieldPattern, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    paintProperty->UpdateCursorWidth(CURSOR_WIDTH_SIZE);
    textFieldPattern->OnModifyDone();
    EXPECT_EQ(paintProperty->GetCursorWidthValue(), CURSOR_WIDTH_SIZE);
}

/**
 * @tc.name: SelectedColor001
 * @tc.desc: Verify that the SelectedColor interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SelectedColor001, TestSize.Level1)
{
    auto frameNode = GetFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(frameNode->GetPattern());
    ASSERT_NE(textFieldPattern, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    paintProperty->UpdateSelectedBackgroundColor(Color::BLACK);
    textFieldPattern->scrollableEvent_ = nullptr;
    textFieldPattern->OnModifyDone();
    textFieldPattern->scrollableEvent_ = AceType::MakeRefPtr<ScrollableEvent>(AXIS_VERTICAL);
    textFieldPattern->OnModifyDone();
    EXPECT_TRUE(paintProperty->GetSelectedBackgroundColor().has_value());
}

/**
 * @tc.name: CaretPosition001
 * @tc.desc: Verify that the CursorPosition interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CaretPosition001, TestSize.Level1)
{
    auto frameNode = GetFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateCaretPosition(CARET_POSITION);
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(frameNode->GetPattern());
    textFieldPattern->OnModifyDone();
    EXPECT_EQ(textFieldPattern->GetTextEditingValue().caretPosition, CARET_POSITION);
}

/**
 * @tc.name: CaretPosition002
 * @tc.desc: Verify that the CursorPosition interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CaretPosition002, TestSize.Level1)
{
    auto frameNode = GetFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(frameNode->GetPattern());
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateCaretPosition(CARET_POSITION);
    textFieldPattern->UpdateCaretRectByPosition(CARET_POSITION);
    EXPECT_EQ(textFieldPattern->GetTextEditingValue().caretPosition, CARET_POSITION);
}

/**
 * @tc.name: CaretPosition003
 * @tc.desc: Verify that the CursorPosition interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CaretPosition003, TestSize.Level1)
{
    auto frameNode = GetFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(frameNode->GetPattern());
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateCaretPosition(CARET_POSITION);
    textFieldPattern->UpdateCaretPositionByPressOffset();
    EXPECT_EQ(textFieldPattern->GetTextEditingValue().caretPosition, CARET_POSITION);
}

/**
 * @tc.name: CaretPosition004
 * @tc.desc: Verify that the CursorPosition interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CaretPosition004, TestSize.Level1)
{
    TextFieldModelNG textFieldModelNG;
    const std::string placeholder = "DEFAULT PLACEHOLDER";
    const std::string value;
    const std::string TEXT_VALUE = "DEFAULT_TEXT";
    bool isTextArea = false;
    textFieldModelNG.CreateNode(placeholder, value, isTextArea);
    textFieldModelNG.SetCaretPosition(CARET_POSITION);
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(frameNode->GetPattern());
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateCaretPosition(CARET_POSITION);
    textFieldPattern->InitEditingValueText(TEXT_VALUE);
    EXPECT_EQ(textFieldPattern->GetTextEditingValue().caretPosition, TEXT_VALUE.length());
}

/**
 * @tc.name: CaretPosition005
 * @tc.desc: Verify that the CursorPosition interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CaretPosition005, TestSize.Level1)
{
    auto frameNode = GetFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = AceType::DynamicCast<TextFieldPattern>(frameNode->GetPattern());
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateCaretPosition(CARET_POSITION);
    textFieldPattern->textEditingValue_.text = "text";
    textFieldPattern->UpdateCaretPositionByPressOffset();
    EXPECT_EQ(textFieldPattern->GetTextEditingValue().caretPosition, CARET_POSITION);
}

/**
 * @tc.name: ModifyTextStyle001
 * @tc.desc: Verify that the ModifyTextStyle interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, ModifyTextStyle001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextFieldTheme>()));
    TextFieldContentModifier textFieldContentModifier(pattern);
    TextStyle textStyle;
    textFieldContentModifier.ModifyTextStyle(textStyle);
    EXPECT_EQ(textStyle.GetFontSize().ConvertToPx(), DEFAULT_FONT_SIZE.ConvertToPx());
}

/**
 * @tc.name: OnScrollCallback001
 * @tc.desc: Verify that the OnScrollCallback interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnScrollCallback001, TestSize.Level1)
{
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    const float offset = 0.0;
    const int32_t source = 0;
    bool ret = false;
    ret = textFieldPattern->OnScrollCallback(offset, source);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: CheckScrollable001
 * @tc.desc: Verify that the CheckScrollable interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CheckScrollable001, TestSize.Level1)
{
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    textFieldPattern->textEditingValue_.text = "";
    EXPECT_TRUE(textFieldPattern->scrollable_);
    textFieldPattern->CheckScrollable();
    EXPECT_FALSE(textFieldPattern->scrollable_);
}

/**
 * @tc.name: CheckScrollable002
 * @tc.desc: Verify that the CheckScrollable interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CheckScrollable002, TestSize.Level1)
{
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    textFieldPattern->textEditingValue_.text = "checkScrollable";
    textFieldPattern->textRect_.SetHeight(1.0);
    textFieldPattern->contentRect_.SetHeight(0.0);
    textFieldPattern->scrollable_ = false;
    textFieldPattern->CheckScrollable();
    EXPECT_TRUE(textFieldPattern->scrollable_);
}

/**
 * @tc.name: CheckScrollable003
 * @tc.desc: Verify that the CheckScrollable interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CheckScrollable003, TestSize.Level1)
{
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    textFieldPattern->textEditingValue_.text = "checkScrollable";
    textFieldPattern->textRect_.SetHeight(0.0);
    textFieldPattern->contentRect_.SetHeight(0.0);
    EXPECT_TRUE(textFieldPattern->scrollable_);
    textFieldPattern->CheckScrollable();
    EXPECT_FALSE(textFieldPattern->scrollable_);
}

/**
 * @tc.name: CheckScrollable004
 * @tc.desc: Verify that the CheckScrollable interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CheckScrollable004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern and TextFieldLayoutProperty.
     * @tc.expected: Check it is not nullptr.
     */
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = textFieldPattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. setup prerequisite property and call CheckScrollable.
     * @tc.expected: Check scrollable_ value.
     */
    textFieldPattern->textEditingValue_.text = "checkScrollable";
    textFieldPattern->textRect_.SetHeight(0.0);
    textFieldPattern->contentRect_.SetHeight(1.0);
    EXPECT_FALSE(textFieldPattern->scrollable_);
    textFieldPattern->CheckScrollable();
    EXPECT_FALSE(textFieldPattern->scrollable_);

    layoutProperty->UpdateShowCounter(true);
    textFieldPattern->counterParagraph_ = std::make_shared<RSParagraph>();
    textFieldPattern->CheckScrollable();
    EXPECT_FALSE(textFieldPattern->scrollable_);
}

/**
 * @tc.name: UpdateCaretRectByPosition001
 * @tc.desc: Verify that the UpdateCaretRectByPosition interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateCaretRectByPosition001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    const int32_t position = 0;
    pattern->UpdateCaretRectByPosition(position);
    EXPECT_EQ(pattern->GetEditingValue().caretPosition, position);
}

/**
 * @tc.name: SetFrameSize001
 * @tc.desc: Verify that the SetFrameSize interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetFrameSize001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    TextFieldOverlayModifier textFieldOverlayModifier(pattern, scrollEdgeEffect);
    SizeF value;
    textFieldOverlayModifier.SetFrameSize(value);
    EXPECT_EQ(textFieldOverlayModifier.frameSize_->Get(), value);
}

/**
 * @tc.name: SetCurrentOffset001
 * @tc.desc: Verify that the SetCurrentOffset interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetCurrentOffset001, TestSize.Level1)
{
    auto pattern = GetPattern();
    ASSERT_NE(pattern, nullptr);
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    TextFieldOverlayModifier textFieldOverlayModifier(pattern, scrollEdgeEffect);
    float value = 1.0;
    textFieldOverlayModifier.SetCurrentOffset(value);
    EXPECT_EQ(textFieldOverlayModifier.currentOffset_->Get(), value);
}

/**
 * @tc.name: GetContentModifier001
 * @tc.desc: Verify that the GetContentModifier interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetContentModifier001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    auto textFieldOverlayModifier = AceType::MakeRefPtr<TextFieldOverlayModifier>(pattern, scrollEdgeEffect);
    auto textFieldContentModifier = AceType::MakeRefPtr<TextFieldContentModifier>(pattern);
    TextFieldPaintMethod textFieldPaintMethod(pattern, textFieldOverlayModifier, textFieldContentModifier);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetContentSize(CONTENT_SIZE);
    geometryNode->SetContentOffset(CONTENT_OFFSET);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty);
    EXPECT_NE(textFieldPaintMethod.GetContentModifier(&paintWrapper), nullptr);
}

/**
 * @tc.name: UpdateContentModifier001
 * @tc.desc: Verify that the UpdateContentModifier interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateContentModifier001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    auto textFieldOverlayModifier = AceType::MakeRefPtr<TextFieldOverlayModifier>(pattern, scrollEdgeEffect);
    auto textFieldContentModifier = AceType::MakeRefPtr<TextFieldContentModifier>(pattern);
    textFieldContentModifier->contentOffset_ = AceType::MakeRefPtr<PropertyOffsetF>(OffsetF());
    textFieldContentModifier->contentSize_ = AceType::MakeRefPtr<PropertySizeF>(SizeF());
    textFieldContentModifier->textValue_ = AceType::MakeRefPtr<PropertyString>("");
    textFieldContentModifier->textRectY_ = AceType::MakeRefPtr<PropertyFloat>(0.0);
    textFieldContentModifier->textRectX_ = AceType::MakeRefPtr<PropertyFloat>(0.0);
    TextFieldPaintMethod textFieldPaintMethod(pattern, textFieldOverlayModifier, textFieldContentModifier);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetContentSize(CONTENT_SIZE);
    geometryNode->SetContentOffset(CONTENT_OFFSET);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty);
    textFieldPaintMethod.UpdateContentModifier(&paintWrapper);
    EXPECT_EQ(textFieldContentModifier->contentOffset_->Get(), CONTENT_OFFSET);
}

/**
 * @tc.name: GetOverlayModifier001
 * @tc.desc: Verify that the GetOverlayModifier interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetOverlayModifier001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    auto textFieldOverlayModifier = AceType::MakeRefPtr<TextFieldOverlayModifier>(pattern, scrollEdgeEffect);
    auto textFieldContentModifier = AceType::MakeRefPtr<TextFieldContentModifier>(pattern);
    TextFieldPaintMethod textFieldPaintMethod(pattern, textFieldOverlayModifier, textFieldContentModifier);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetContentSize(CONTENT_SIZE);
    geometryNode->SetContentOffset(CONTENT_OFFSET);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty);
    EXPECT_NE(textFieldPaintMethod.GetOverlayModifier(&paintWrapper), nullptr);
}

/**
 * @tc.name: UpdateOverlayModifier001
 * @tc.desc: Verify that the UpdateOverlayModifier interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateOverlayModifier001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    auto textFieldOverlayModifier = AceType::MakeRefPtr<TextFieldOverlayModifier>(pattern, scrollEdgeEffect);
    auto textFieldContentModifier = AceType::MakeRefPtr<TextFieldContentModifier>(pattern);
    TextFieldPaintMethod textFieldPaintMethod(pattern, textFieldOverlayModifier, textFieldContentModifier);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetContentSize(CONTENT_SIZE);
    geometryNode->SetContentOffset(CONTENT_OFFSET);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty);
    textFieldPaintMethod.UpdateOverlayModifier(&paintWrapper);
    EXPECT_EQ(textFieldOverlayModifier->contentSize_->Get(), CONTENT_SIZE);
}

/**
 * @tc.name: SetContentOffset002
 * @tc.desc: Verify that the SetContentOffset interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetContentOffset002, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    OffsetF value = CONTENT_OFFSET;
    textFieldContentModifier.contentOffset_ = nullptr;
    textFieldContentModifier.SetContentOffset(value);
    textFieldContentModifier.contentOffset_ = AceType::MakeRefPtr<PropertyOffsetF>(OffsetF());
    textFieldContentModifier.SetContentOffset(value);
    EXPECT_EQ(textFieldContentModifier.contentOffset_->Get(), value);
}

/**
 * @tc.name: SetContentSize002
 * @tc.desc: Verify that the SetContentSize interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetContentSize002, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    SizeF value = CONTENT_SIZE;
    textFieldContentModifier.contentSize_ = nullptr;
    textFieldContentModifier.SetContentSize(value);
    textFieldContentModifier.contentSize_ = AceType::MakeRefPtr<PropertySizeF>(SizeF());
    textFieldContentModifier.SetContentSize(value);
    EXPECT_EQ(textFieldContentModifier.contentSize_->Get(), value);
}

/**
 * @tc.name: SetTextValue001
 * @tc.desc: Verify that the SetTextValue interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetTextValue001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    std::string value1 = "";
    textFieldContentModifier.textValue_ = AceType::MakeRefPtr<PropertyString>("");
    textFieldContentModifier.SetTextValue(value1);
    std::string value2 = "text";
    textFieldContentModifier.SetTextValue(value2);
    EXPECT_EQ(textFieldContentModifier.textValue_->Get(), value2);
}

/**
 * @tc.name: SetTextRectY001
 * @tc.desc: Verify that the SetTextRectY interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetTextRectY001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    textFieldContentModifier.textRectY_ = AceType::MakeRefPtr<PropertyFloat>(0.0);
    const float value = 1.0;
    textFieldContentModifier.SetTextRectY(value);
    EXPECT_EQ(textFieldContentModifier.textRectY_->Get(), value);
}

/**
 * @tc.name: TextFieldPatternUpdateScrollBarOffset001
 * @tc.desc: Verify that the UpdateScrollBarOffset interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldPatternUpdateScrollBarOffset001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->UpdateScrollBarOffset();
    EXPECT_EQ(pattern->textEditingValue_.text, "checkScrollable");
}

/**
 * @tc.name: TextFieldPatternUpdateScrollBarOffset002
 * @tc.desc: Verify that the UpdateScrollBarOffset interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldPatternUpdateScrollBarOffset002, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->textEditingValue_.Reset();
    pattern->UpdateScrollBarOffset();
    EXPECT_TRUE(pattern->textEditingValue_.text.empty());
    pattern->textEditingValue_.text = "updateScrollBarOffset";
    pattern->UpdateScrollBarOffset();
    EXPECT_EQ(pattern->GetScrollBar(), nullptr);
    auto scrollBar = AceType::MakeRefPtr<ScrollBar>();
    pattern->scrollBar_ = scrollBar;
    auto scrollBarProxy = AceType::MakeRefPtr<NG::ScrollBarProxy>();
    pattern->scrollBarProxy_ = scrollBarProxy;
    pattern->UpdateScrollBarOffset();
    ASSERT_FALSE(!pattern->GetScrollBar() && !pattern->GetScrollBarProxy());
}

/**
 * @tc.name: TextFieldPatternSearchNodeTest001
 * @tc.desc: Verify the parent search node branch.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldPatternSearchNodeTest001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto* stack = ViewStackProcessor::GetInstance();
    int32_t nodeId = stack->ClaimNodeId();
    auto searchNode = AceType::MakeRefPtr<FrameNode>(V2::SEARCH_ETS_TAG, nodeId, AceType::MakeRefPtr<Pattern>(), false);
    frameNode->MountToParent(searchNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->HasFocus();
    auto info = GestureEvent();
    pattern->UpdateCaretInfoToController();
    pattern->HandleClickEvent(info);
    pattern->HandleLongPress(info);
    auto mouseInfo = MouseInfo();
    pattern->HandleMouseEvent(mouseInfo);
    EXPECT_EQ(pattern->IsSearchParentNode(), true);
}

/**
 * @tc.name: OffsetInContentRegion
 * @tc.desc: Verify the parent search node branch.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OffsetInContentRegion, TestSize.Level2)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    Offset offset;
    EXPECT_TRUE(pattern->OffsetInContentRegion(offset));
}

/**
 * @tc.name: TextFieldPatternOnTextAreaScroll001
 * @tc.desc: Verify that the AddScrollEvent interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldPatternOnTextAreaScroll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create the TextFieldPattern.
     * @tc.expected: step1. Check the TextFieldPattern success.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. Call the OnTextAreaScroll.
     * @tc.expected: step2. Check the value set in OnTextAreaScroll.
     */
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateMaxLines(5);
    pattern->selectOverlayProxy_ = AceType::MakeRefPtr<SelectOverlayProxy>(SELECT_OVERLAY_ID);
    // Is TextArea, textRect_.Height() < contentRect_.Height()
    pattern->textRect_.SetHeight(TEXT_RECT_HEIGHT);
    pattern->textRect_.SetWidth(TEXT_RECT_WIDTH);
    pattern->contentRect_.SetHeight(CONTENT_RECT_HEIGHT);
    pattern->contentRect_.SetWidth(CONTENT_RECT_WIDTH);
    pattern->OnTextAreaScroll(TEXT_AREA_SCROLL_OFFSET);
    // Is TextArea, textRect Height > contentRect Height, textRect Y + offset > contentRect Y.
    pattern->textRect_.SetHeight(TEXT_RECT_HEIGHT_LARGE);
    pattern->textRect_.SetWidth(TEXT_RECT_WIDTH);
    pattern->textRect_.SetTop(TEXT_RECT_Y);
    pattern->contentRect_.SetHeight(CONTENT_RECT_HEIGHT);
    pattern->contentRect_.SetWidth(CONTENT_RECT_WIDTH);
    pattern->contentRect_.SetTop(CONTENT_RECT_Y);
    pattern->OnTextAreaScroll(TEXT_AREA_SCROLL_OFFSET);
    // Is TextArea, textRect Height > contentRect Height, textRect Y + offset < contentRect Y.
    // textRect Y + offset - contentRect Heigh > textRect Height.
    pattern->contentRect_.SetTop(CONTENT_RECT_Y_LARGE);
    pattern->textRect_.SetTop(TEXT_RECT_Y_LARGE);
    pattern->OnTextAreaScroll(TEXT_AREA_SCROLL_OFFSET);
    // Is TextArea, textRect Height > contentRect Height, textRect Y + offset < contentRect Y.
    // textRect Y + offset - contentRectHeight < textRect Height.
    pattern->contentRect_.SetTop(CONTENT_RECT_Y_LARGE);
    pattern->textRect_.SetTop(TEXT_RECT_Y);
    pattern->OnTextAreaScroll(TEXT_AREA_SCROLL_OFFSET);
    pattern->isSingleHandle_ = false;
    pattern->OnTextAreaScroll(TEXT_AREA_SCROLL_OFFSET);
    pattern->isSingleHandle_ = true;
    float oldCaretRectY = pattern->caretRect_.GetY();
    pattern->OnTextAreaScroll(TEXT_AREA_SCROLL_OFFSET);
    EXPECT_EQ(pattern->caretRect_.GetY(), oldCaretRectY);
    EXPECT_EQ(pattern->textRect_.GetOffset(), OffsetF(pattern->textRect_.GetX(), pattern->currentOffset_));

    // Scroll down, secondHandleOffset Y > contentRect Y
    pattern->isSingleHandle_ = false;
    pattern->contentRect_.SetTop(12.0f);
    pattern->contentRect_.SetLeft(24.0f);
    pattern->contentRect_.SetWidth(500.0f);
    pattern->contentRect_.SetHeight(400.0f);
    pattern->textRect_.SetTop(-300.0f);
    pattern->textRect_.SetHeight(800.0f);
    pattern->textSelector_.firstHandleOffset_.SetX(300.0f);
    pattern->textSelector_.firstHandleOffset_.SetY(300.0f);
    pattern->textSelector_.secondHandleOffset_.SetX(400.0f);
    pattern->textSelector_.secondHandleOffset_.SetY(300.0f);
    pattern->parentGlobalOffset_ = OffsetF(60.0f, 8.0f);
    pattern->OnTextAreaScroll(1.0f);
    EXPECT_EQ(pattern->textSelector_.secondHandleOffset_.GetY(), 301.0f);

    // Scroll up, secondHandleOffset Y + secondHandle Height > contentRect Y + contentRect Height
    pattern->textRect_.SetTop(-130.0f);
    pattern->textSelector_.firstHandleOffset_.SetX(400.0f);
    pattern->textSelector_.firstHandleOffset_.SetY(500.0f);
    pattern->textSelector_.secondHandleOffset_.SetX(500.0f);
    pattern->textSelector_.secondHandleOffset_.SetY(500.0f);
    pattern->OnTextAreaScroll(-20.0f);
    EXPECT_EQ(pattern->textSelector_.secondHandleOffset_.GetY(), 480.0f);

    // Select overlay is not on.
    pattern->selectOverlayProxy_ = AceType::MakeRefPtr<SelectOverlayProxy>(-1);
    pattern->textRect_.SetTop(30.0f);
    pattern->OnTextAreaScroll(-20.0f);
    EXPECT_EQ(pattern->textRect_.GetY(), 10.0f);
}

/**
 * @tc.name: MakeEmptyOffset
 * @tc.desc: test MakeEmptyOffset.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, MakeEmptyOffset, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create the TextFieldPattern.
     * @tc.expected: step1. Check the TextFieldPattern success.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. Call the MakeEmptyOffset.
     * @tc.expected: step2. Check the return value.
     */
    auto width = pattern->contentRect_.Width();
    std::pair<TextAlign, OffsetF> textAlignPairs[] = { std::make_pair(TextAlign::CENTER, OffsetF(width * 0.5f, 0.0f)),
        std::make_pair(TextAlign::END, OffsetF(width, 0.0f)), std::make_pair(TextAlign::START, OffsetF()),
        std::make_pair(TextAlign::JUSTIFY, OffsetF()) };
    for (auto pair : textAlignPairs) {
        layoutProperty->UpdateTextAlign(pair.first);
        EXPECT_EQ(pattern->MakeEmptyOffset(), pair.second);
    }
}

/**
 * @tc.name: onDraw001
 * @tc.desc: Verify that the onDraw interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, onDraw001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->scrollableEvent_ = AceType::MakeRefPtr<ScrollableEvent>(AXIS_VERTICAL);
    pattern->AddScrollEvent();
    pattern->scrollable_ = true;
    pattern->CheckScrollable();
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    TextFieldOverlayModifier textFieldOverlayModifier(pattern, scrollEdgeEffect);
    Testing::MockCanvas rsCanvas;
    DrawingContext context { rsCanvas, CONTEXT_WIDTH_VALUE, CONTEXT_HEIGHT_VALUE };
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    textFieldOverlayModifier.onDraw(context);
}

/**
 * @tc.name: PaintSelection001
 * @tc.desc: Verify that the PaintSelection interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, PaintSelection001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    TextFieldOverlayModifier textFieldOverlayModifier(pattern, scrollEdgeEffect);
    Testing::MockCanvas rsCanvas;
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    DrawingContext context { rsCanvas, CONTEXT_WIDTH_VALUE, CONTEXT_HEIGHT_VALUE };
    textFieldOverlayModifier.PaintSelection(context);
}

/**
 * @tc.name: PaintSelection002
 * @tc.desc: Verify that the PaintSelection interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, PaintSelection002, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectionMode_ = SelectionMode::SELECT;
    pattern->textSelector_.baseOffset = 1;
    pattern->textSelector_.destinationOffset = 0;
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    TextFieldOverlayModifier textFieldOverlayModifier(pattern, scrollEdgeEffect);
    Testing::MockCanvas rsCanvas;
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DetachBrush()).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, AttachPen(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DetachPen()).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DrawRoundRect(_)).WillRepeatedly(Return());
    EXPECT_CALL(rsCanvas, DrawCircle(_, _)).WillRepeatedly(Return());
    DrawingContext context { rsCanvas, CONTEXT_WIDTH_VALUE, CONTEXT_HEIGHT_VALUE };
    textFieldOverlayModifier.PaintSelection(context);
}

/**
 * @tc.name: PaintSelection003
 * @tc.desc: Verify that the PaintSelection interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, PaintSelection003, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectionMode_ = SelectionMode::SELECT;
    pattern->textSelector_.baseOffset = 1;
    pattern->textSelector_.destinationOffset = 0;
#ifndef USE_GRAPHIC_TEXT_GINE
    std::vector<RSTypographyProperties::TextBox> textBoxes;
    RSTypographyProperties::TextBox textBox;
    textBoxes.emplace_back(textBox);
    pattern->textBoxes_ = textBoxes;
#else
    std::vector<RSTypographyProperties::TextRect> textBoxs;
    RSTypographyProperties::TextRect textBox;
    textBoxs.emplace_back(textBox);
    pattern->textBoxes_ = textBoxs;
#endif
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    TextFieldOverlayModifier textFieldOverlayModifier(pattern, scrollEdgeEffect);
    Testing::MockCanvas rsCanvas;
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DetachBrush()).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, AttachPen(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DetachPen()).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DrawRoundRect(_)).WillRepeatedly(Return());
    EXPECT_CALL(rsCanvas, DrawCircle(_, _)).WillRepeatedly(Return());
    DrawingContext context { rsCanvas, CONTEXT_WIDTH_VALUE, CONTEXT_HEIGHT_VALUE };
    textFieldOverlayModifier.inputStyle_ = InputStyle::INLINE;
    textFieldOverlayModifier.PaintSelection(context);
    textFieldOverlayModifier.inputStyle_ = InputStyle::DEFAULT;
    textFieldOverlayModifier.PaintSelection(context);
}

/**
 * @tc.name: SetCursortWidth001
 * @tc.desc: Verify that the SetCursorWidth interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetCursortWidth001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    TextFieldOverlayModifier textFieldOverlayModifier(pattern, scrollEdgeEffect);
    float value = 1.0;
    textFieldOverlayModifier.SetCursorWidth(value);
    EXPECT_EQ(textFieldOverlayModifier.cursorWidth_->Get(), value);
}
/**
 * @tc.name: PaintCursor001
 * @tc.desc: Verify that the PaintCursor interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, PaintCursor001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectionMode_ = SelectionMode::NONE;
    pattern->paragraph_ = std::make_shared<RSParagraph>();
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    TextFieldOverlayModifier textFieldOverlayModifier(pattern, scrollEdgeEffect);
    Testing::MockCanvas rsCanvas;
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DetachBrush()).WillRepeatedly(ReturnRef(rsCanvas));
    DrawingContext context { rsCanvas, CONTEXT_WIDTH_VALUE, CONTEXT_HEIGHT_VALUE };
    textFieldOverlayModifier.cursorVisible_ = AceType::MakeRefPtr<PropertyBool>(true);
    textFieldOverlayModifier.PaintCursor(context);
    textFieldOverlayModifier.cursorVisible_ = AceType::MakeRefPtr<PropertyBool>(true);
    textFieldOverlayModifier.PaintCursor(context);
}

/**
 * @tc.name: SetTextRectX001
 * @tc.desc: Verify that the SetTextRectX interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetTextRectX001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    textFieldContentModifier.textRectX_ = AceType::MakeRefPtr<PropertyFloat>(0.0);
    const float value = 1.0;
    textFieldContentModifier.SetTextRectX(value);
    EXPECT_EQ(textFieldContentModifier.textRectX_->Get(), value);
}

/**
 * @tc.name: TextFieldModelSetType
 * @tc.desc: Create frameNode and settype
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldModelSetType, TestSize.Level1)
{
    TextFieldModelNG textFieldModelInstance;
    auto layoutProperty = GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    textFieldModelInstance.SetType(TextInputType::TEXT);
    EXPECT_EQ(layoutProperty->GetTextInputTypeValue(), TextInputType::TEXT);
    textFieldModelInstance.SetType(TextInputType::MULTILINE);
    EXPECT_EQ(layoutProperty->GetTextInputTypeValue(), TextInputType::MULTILINE);
    EXPECT_TRUE(layoutProperty->GetTypeChangedValue());
}

/**
 * @tc.name: TextFieldModelSetPlaceholderColor
 * @tc.desc: Create frameNode and PlaceholderColor
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldModelSetPlaceholderColor, TestSize.Level1)
{
    TextFieldModelNG textFieldModelInstance;
    auto layoutProperty = GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    textFieldModelInstance.SetPlaceholderColor(Color::WHITE);
    EXPECT_TRUE(layoutProperty->HasPlaceholderTextColor());
}

/**
 * @tc.name: TextFieldModelSetEnterKeyType
 * @tc.desc: Create frameNode and SetEnterKeyType
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldModelSetEnterKeyType, TestSize.Level1)
{
    TextFieldModelNG textFieldModelInstance;
    textFieldModelInstance.SetEnterKeyType(TextInputAction::GO);
    auto textFieldPattern = GetPattern();
    EXPECT_EQ(textFieldPattern->GetTextInputActionValue(), TextInputAction::GO);
}

/**
 * @tc.name: SetOnEditChanged
 * @tc.desc: Create frameNode and SetOnEditChanged
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldModelSetOnEditChanged, TestSize.Level1)
{
    TextFieldModelNG textFieldModelInstance;
    bool callback = false;
    textFieldModelInstance.SetOnEditChanged([&callback](bool info) { callback = info; });
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<TextFieldEventHub>();
    EXPECT_TRUE(eventHub);
    eventHub->FireOnEditChanged(true);
    EXPECT_TRUE(callback);
}

/**
 * @tc.name: SetTextObscured
 * @tc.desc: Verify that the SetTextObscured interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetTextObscured, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    bool value = true;
    textFieldContentModifier.textObscured_ = nullptr;
    textFieldContentModifier.SetTextObscured(value);
    textFieldContentModifier.textObscured_ = AceType::MakeRefPtr<PropertyBool>(value);

    /**
     * @tc.steps: step2. call SetTextObscured function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    textFieldContentModifier.SetTextObscured(value);
    EXPECT_EQ(textFieldContentModifier.textObscured_->Get(), value);
}

/**
 * @tc.name: SetTextAlign
 * @tc.desc: Verify that the SetTextAlign interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetTextAlign, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    const TextAlign value1 = TextAlign::START;
    textFieldContentModifier.SetTextAlign(value1);
    textFieldContentModifier.textAlign_ = AceType::MakeRefPtr<PropertyInt>(static_cast<int32_t>(value1));
    const TextAlign value2 = TextAlign::END;

    /**
     * @tc.steps: step2. call SetTextAlign function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    textFieldContentModifier.SetTextAlign(value2);
    EXPECT_EQ(textFieldContentModifier.textAlign_->Get(), static_cast<int32_t>(value2));
}

/**
 * @tc.name: NeedMeasureUpdate001
 * @tc.desc: Verify that the NeedMeasureUpdate interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, NeedMeasureUpdate001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    PropertyChangeFlag flag;

    /**
     * @tc.steps: step2. call NeedMeasureUpdate function.
     * @tc.expected: The NeedMeasureUpdate function returns a value of false.
     */
    bool ret = true;
    ret = textFieldContentModifier.NeedMeasureUpdate(flag);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: NeedMeasureUpdate002
 * @tc.desc: Verify that the NeedMeasureUpdate interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, NeedMeasureUpdate002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    float fontSizeValue1 = 5.0;
    float fontSizeValue2 = 1.0;
    const Dimension fontSize = Dimension(5.0);
    textFieldContentModifier.SetFontSize(fontSize);
    PropertyChangeFlag flag;

    /**
     * @tc.steps: step2. call NeedMeasureUpdate function.
     * @tc.expected: The NeedMeasureUpdate function returns a value of false.
     */
    textFieldContentModifier.fontSizeFloat_ = nullptr;
    EXPECT_FALSE(textFieldContentModifier.NeedMeasureUpdate(flag));

    /**
     * @tc.steps: step3. call NeedMeasureUpdate function.
     * @tc.expected: The NeedMeasureUpdate function returns a value of false.
     */
    textFieldContentModifier.fontSizeFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(fontSizeValue1);
    textFieldContentModifier.fontSizeFloat_->Set(fontSizeValue1);
    EXPECT_FALSE(textFieldContentModifier.NeedMeasureUpdate(flag));

    /**
     * @tc.steps: step4. call NeedMeasureUpdate function.
     * @tc.expected: The NeedMeasureUpdate function returns a value of true.
     */
    textFieldContentModifier.fontSizeFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(fontSizeValue2);
    textFieldContentModifier.fontSizeFloat_->Set(fontSizeValue2);
    EXPECT_TRUE(textFieldContentModifier.NeedMeasureUpdate(flag));
}

/**
 * @tc.name: NeedMeasureUpdate003
 * @tc.desc: Verify that the NeedMeasureUpdate interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, NeedMeasureUpdate003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    PropertyChangeFlag flag;
    const FontWeight fontWeight = FontWeight::W100;
    textFieldContentModifier.SetFontWeight(fontWeight);
    float fontWeightValue1 = 0.0;
    float fontWeightValue2 = 1.0;

    /**
     * @tc.steps: step2. call NeedMeasureUpdate function.
     * @tc.expected: The NeedMeasureUpdate function returns a value of false.
     */
    textFieldContentModifier.fontWeightFloat_ = nullptr;
    EXPECT_FALSE(textFieldContentModifier.NeedMeasureUpdate(flag));

    /**
     * @tc.steps: step3. call NeedMeasureUpdate function.
     * @tc.expected: The NeedMeasureUpdate function returns a value of false.
     */
    textFieldContentModifier.fontWeightFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(fontWeightValue1);
    textFieldContentModifier.fontWeightFloat_->Set(fontWeightValue1);
    EXPECT_FALSE(textFieldContentModifier.NeedMeasureUpdate(flag));

    /**
     * @tc.steps: step4. call NeedMeasureUpdate function.
     * @tc.expected: The NeedMeasureUpdate function returns a value of true.
     */
    textFieldContentModifier.fontWeightFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(fontWeightValue2);
    textFieldContentModifier.fontWeightFloat_->Set(fontWeightValue2);
    EXPECT_TRUE(textFieldContentModifier.NeedMeasureUpdate(flag));
}

/**
 * @tc.name: NeedMeasureUpdate004
 * @tc.desc: Verify that the NeedMeasureUpdate interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, NeedMeasureUpdate004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    PropertyChangeFlag flag;
    const Color color1 = Color::WHITE;
    const Color color2 = Color::RED;

    /**
     * @tc.steps: step2. call NeedMeasureUpdate function.
     * @tc.expected: The NeedMeasureUpdate function returns a value of false.
     */
    textFieldContentModifier.SetTextColor(color2);
    textFieldContentModifier.animatableTextColor_ = nullptr;
    EXPECT_FALSE(textFieldContentModifier.NeedMeasureUpdate(flag));

    /**
     * @tc.steps: step3. call NeedMeasureUpdate function.
     * @tc.expected: The NeedMeasureUpdate function returns a value of false.
     */
    textFieldContentModifier.animatableTextColor_ = AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(color2));
    textFieldContentModifier.animatableTextColor_->Set(LinearColor(color2));
    EXPECT_FALSE(textFieldContentModifier.NeedMeasureUpdate(flag));

    /**
     * @tc.steps: step4. call NeedMeasureUpdate function.
     * @tc.expected: The NeedMeasureUpdate function returns a value of true.
     */
    textFieldContentModifier.animatableTextColor_ = AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(color1));
    textFieldContentModifier.animatableTextColor_->Set(LinearColor(color1));
    EXPECT_TRUE(textFieldContentModifier.NeedMeasureUpdate(flag));
}

/**
 * @tc.name: SetFontSize
 * @tc.desc: Verify that the SetFontSize interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetFontSize, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    const Dimension fontSize = Dimension(5.0);

    /**
     * @tc.steps: step2. call SetFontSize function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    textFieldContentModifier.SetFontSize(fontSize);
    EXPECT_EQ(textFieldContentModifier.fontSize_, fontSize);
}

/**
 * @tc.name: UpdateContentModifier002
 * @tc.desc: Verify that the UpdateContentModifier interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateContentModifier002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);

    /**
     * @tc.steps: step2. create textFieldOverlayModifier and textFieldContentModifier.
     */
    auto textFieldOverlayModifier = AceType::MakeRefPtr<TextFieldOverlayModifier>(pattern, scrollEdgeEffect);
    auto textFieldContentModifier = AceType::MakeRefPtr<TextFieldContentModifier>(pattern);
    textFieldContentModifier->contentOffset_ = AceType::MakeRefPtr<PropertyOffsetF>(OffsetF());
    textFieldContentModifier->contentSize_ = AceType::MakeRefPtr<PropertySizeF>(SizeF());
    textFieldContentModifier->textValue_ = AceType::MakeRefPtr<PropertyString>("");
    textFieldContentModifier->textRectY_ = AceType::MakeRefPtr<PropertyFloat>(0.0);
    textFieldContentModifier->textRectX_ = AceType::MakeRefPtr<PropertyFloat>(0.0);
    float fontSizeValue = 1.0;
    textFieldContentModifier->fontSizeFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(fontSizeValue);
    const Dimension fontSize = Dimension(5.0);
    textFieldContentModifier->SetFontSize(fontSize);
    textFieldContentModifier->fontSizeFloat_->Set(fontSizeValue);
    TextFieldPaintMethod textFieldPaintMethod(pattern, textFieldOverlayModifier, textFieldContentModifier);

    /**
     * @tc.steps: step3. create paintWrapper.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetContentSize(CONTENT_SIZE);
    geometryNode->SetContentOffset(CONTENT_OFFSET);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty);

    /**
     * @tc.steps: step4. call UpdateContentModifier function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    textFieldPaintMethod.UpdateContentModifier(&paintWrapper);
    EXPECT_EQ(textFieldContentModifier->contentOffset_->Get(), CONTENT_OFFSET);
}

/**
 * @tc.name: UpdateOverlayModifier002
 * @tc.desc: Verify that the UpdateOverlayModifier interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateOverlayModifier002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    auto textFieldOverlayModifier = AceType::MakeRefPtr<TextFieldOverlayModifier>(pattern, scrollEdgeEffect);
    auto textFieldContentModifier = AceType::MakeRefPtr<TextFieldContentModifier>(pattern);
    TextFieldPaintMethod textFieldPaintMethod(pattern, textFieldOverlayModifier, textFieldContentModifier);

    /**
     * @tc.steps: step2. create paintWrapper.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetContentSize(CONTENT_SIZE);
    geometryNode->SetContentOffset(CONTENT_OFFSET);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    paintProperty->UpdateCursorWidth(CURSOR_WIDTH_SIZE);
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty);

    /**
     * @tc.steps: step3. call UpdateOverlayModifier function.
     * @tc.expected: The member variable value of textFieldOverlayModifier is the value set above.
     */
    textFieldPaintMethod.UpdateOverlayModifier(&paintWrapper);
    EXPECT_EQ(textFieldOverlayModifier->contentSize_->Get(), CONTENT_SIZE);
}

/**
 * @tc.name: SetFontWeight
 * @tc.desc: Verify that the SetFontWeight interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetFontWeight, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    const FontWeight fontWeight = FontWeight::W100;

    /**
     * @tc.steps: step2. call SetFontWeight function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    textFieldContentModifier.SetFontWeight(fontWeight);
    EXPECT_EQ(textFieldContentModifier.fontWeight_, fontWeight);
}

/**
 * @tc.name: SetTextColor
 * @tc.desc: Verify that the SetTextColor interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetTextColor, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    const Color color = Color::RED;

    /**
     * @tc.steps: step2. call SetTextColor function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    textFieldContentModifier.SetTextColor(color);
    EXPECT_EQ(textFieldContentModifier.textColor_.value(), color);
}

/**
 * @tc.name: ModifyTextStyle002
 * @tc.desc: Verify that the ModifyTextStyle interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, ModifyTextStyle002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    float fontSizeValue;
    textFieldContentModifier.fontSizeFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(fontSizeValue);
    const Dimension fontSize = Dimension(2.0);
    textFieldContentModifier.SetFontSize(fontSize);
    float fontWeightValue;
    textFieldContentModifier.fontWeightFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(fontWeightValue);
    const FontWeight fontWeight = FontWeight::W100;
    textFieldContentModifier.SetFontWeight(fontWeight);
    const Color color = Color::WHITE;
    textFieldContentModifier.animatableTextColor_ = AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(color));
    textFieldContentModifier.SetTextColor(color);

    /**
     * @tc.steps: step2. call ModifyTextStyle function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    TextStyle textStyle;
    textFieldContentModifier.ModifyTextStyle(textStyle);
    EXPECT_EQ(textStyle.GetFontSize().ConvertToPx(), fontSize.ConvertToPx());
    EXPECT_EQ(textStyle.GetFontWeight(), fontWeight);
    EXPECT_EQ(textStyle.GetTextColor(), color);
}

/**
 * @tc.name: ModifyTextStyle003
 * @tc.desc: Verify that the ModifyTextStyle interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, ModifyTextStyle003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    const Dimension fontSize = Dimension(2.0);
    textFieldContentModifier.SetFontSize(fontSize);
    textFieldContentModifier.fontSizeFloat_ = nullptr;
    const FontWeight fontWeight = FontWeight::W100;
    textFieldContentModifier.SetFontWeight(fontWeight);
    textFieldContentModifier.fontWeightFloat_ = nullptr;
    const Color color = Color::WHITE;
    textFieldContentModifier.SetTextColor(color);
    textFieldContentModifier.animatableTextColor_ = nullptr;

    /**
     * @tc.steps: step2. call ModifyTextStyle function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    TextStyle textStyle;
    textFieldContentModifier.ModifyTextStyle(textStyle);
    EXPECT_EQ(textStyle.GetFontSize().ConvertToPx(), DEFAULT_FONT_SIZE.ConvertToPx());
}

/**
 * @tc.name: ModifyTextStyle004
 * @tc.desc: Verify that the ModifyTextStyle interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, ModifyTextStyle004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    float fontSizeValue;
    textFieldContentModifier.fontSizeFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(fontSizeValue);
    float fontWeightValue;
    textFieldContentModifier.fontWeightFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(fontWeightValue);
    const Color color = Color::WHITE;
    textFieldContentModifier.animatableTextColor_ = AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(color));

    /**
     * @tc.steps: step2. call ModifyTextStyle function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    TextStyle textStyle;
    textFieldContentModifier.ModifyTextStyle(textStyle);
    EXPECT_EQ(textStyle.GetFontSize().ConvertToPx(), DEFAULT_FONT_SIZE.ConvertToPx());
}

/**
 * @tc.name: onDraw002
 * @tc.desc: Verify that the onDraw interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, onDraw002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textFieldContentModifier = AceType::MakeRefPtr<TextFieldContentModifier>(pattern);
    textFieldContentModifier->contentOffset_ = AceType::MakeRefPtr<PropertyOffsetF>(OffsetF());
    textFieldContentModifier->contentSize_ = AceType::MakeRefPtr<PropertySizeF>(SizeF());
    textFieldContentModifier->textValue_ = AceType::MakeRefPtr<PropertyString>("");
    textFieldContentModifier->textRectY_ = AceType::MakeRefPtr<PropertyFloat>(0.0);
    textFieldContentModifier->textRectX_ = AceType::MakeRefPtr<PropertyFloat>(0.0);
    textFieldContentModifier->textObscured_ = AceType::MakeRefPtr<PropertyBool>(true);

    /**
     * @tc.steps: step2. call onDraw function.
     * @tc.expected: The onDraw interface calls normally and exits without exception.
     */
    Testing::MockCanvas rsCanvas;
    DrawingContext context { rsCanvas, CONTEXT_WIDTH_VALUE, CONTEXT_HEIGHT_VALUE };
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    textFieldContentModifier->onDraw(context);
}

/**
 * @tc.name: onDraw003
 * @tc.desc: Verify that the onDraw interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, onDraw003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. paragraph_ Pointer instantiation.
     */
    pattern->paragraph_ = std::make_shared<RSParagraph>();
    auto textFieldContentModifier = AceType::MakeRefPtr<TextFieldContentModifier>(pattern);
    textFieldContentModifier->contentOffset_ = AceType::MakeRefPtr<PropertyOffsetF>(OffsetF());
    textFieldContentModifier->contentSize_ = AceType::MakeRefPtr<PropertySizeF>(SizeF());
    textFieldContentModifier->textValue_ = AceType::MakeRefPtr<PropertyString>("");
    textFieldContentModifier->textRectY_ = AceType::MakeRefPtr<PropertyFloat>(0.0);
    textFieldContentModifier->textRectX_ = AceType::MakeRefPtr<PropertyFloat>(0.0);
    textFieldContentModifier->textObscured_ = AceType::MakeRefPtr<PropertyBool>(true);

    /**
     * @tc.steps: step3. call onDraw function.
     * @tc.expected: The onDraw interface calls normally and exits without exception.
     */
    Testing::MockCanvas rsCanvas;
    DrawingContext context { rsCanvas, CONTEXT_WIDTH_VALUE, CONTEXT_HEIGHT_VALUE };
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    textFieldContentModifier->onDraw(context);
}

/**
 * @tc.name: SetPropertyToModifier
 * @tc.desc: Verify that the SetPropertyToModifier interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetPropertyToModifier, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldLayoutAlgorithm textFieldLayoutAlgorithm;
    TextStyle textStyle;
    const Color color = Color::RED;
    textStyle.SetTextColor(color);
    auto textFieldContentModifier = AceType::MakeRefPtr<TextFieldContentModifier>(pattern);

    /**
     * @tc.steps: step2. call SetPropertyToModifier function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    textFieldLayoutAlgorithm.SetPropertyToModifier(textStyle, textFieldContentModifier);
    EXPECT_EQ(textFieldContentModifier->textColor_.value(), color);
}

/**
 * @tc.name: SetCaretStyle
 * @tc.desc: Create frameNode and SetCaretStyle
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetCaretStyle, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFieldModelInstance.
     */
    TextFieldModelNG textFieldModelInstance;
    const CaretStyle value1 = {};
    textFieldModelInstance.SetCaretStyle(value1);
    const CaretStyle value2 = { CURSOR_WIDTH_SIZE };
    textFieldModelInstance.SetCaretStyle(value2);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step2. call GetCursorWidthValue function.
     * @tc.expected: The GetCursorWidthValue function returns the value set above.
     */
    EXPECT_EQ(paintProperty->GetCursorWidthValue(), CURSOR_WIDTH_SIZE);
}

/**
 * @tc.name: SetSelectedBackgroundColor
 * @tc.desc: Verify that the SetSelectedBackgroundColor interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetSelectedBackgroundColor, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFieldModelInstance.
     */
    TextFieldModelNG textFieldModelInstance;
    const Color value = Color::BLACK;
    textFieldModelInstance.SetSelectedBackgroundColor(value);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step2. call GetSelectedBackgroundColor function.
     * @tc.expected: The GetSelectedBackgroundColor function returns the value set above.
     */
    EXPECT_EQ(paintProperty->GetSelectedBackgroundColor(), value);
}

/**
 * @tc.name: OnScrollCallback002
 * @tc.desc: Verify that the OnScrollCallback interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnScrollCallback002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFieldPattern.
     */
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    const float offset = 0.0;
    const int32_t source = 0;
    bool ret = false;
    textFieldPattern->scrollBar_ = AccessibilityManager::MakeRefPtr<ScrollBar>();

    /**
     * @tc.steps: step2. call OnScrollCallback function.
     * @tc.expected: The OnScrollCallback function returns a value of true.
     */
    ret = textFieldPattern->OnScrollCallback(offset, source);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnScrollCallback003
 * @tc.desc: Verify that the OnScrollCallback interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnScrollCallback003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFieldPattern.
     */
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);
    const float offset = 0.0;
    const int32_t source = 0;
    bool ret = false;
    textFieldPattern->scrollBar_ = AccessibilityManager::MakeRefPtr<ScrollBar>();

    /**
     * @tc.steps: step2. call OnScrollCallback function.
     * @tc.expected: The OnScrollCallback function returns a value of true.
     */
    ret = textFieldPattern->OnScrollCallback(offset, source);
    EXPECT_TRUE(ret);
    const int32_t SCROLL_FROM_START = 10;
    EXPECT_TRUE(textFieldPattern->OnScrollCallback(offset, SCROLL_FROM_START));
}

/**
 * @tc.name: TextFieldAccessibilityPropertyGetTextLengthLimit001
 * @tc.desc: Test GetTextLengthLimit of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyGetTextLengthLimit001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto textFieldLayoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    EXPECT_EQ(textFieldAccessibilityProperty->GetTextLengthLimit(), Infinity<uint32_t>());

    uint32_t length = 32;
    EXPECT_FALSE(textFieldLayoutProperty->HasMaxLength());
    textFieldLayoutProperty->UpdateMaxLength(length);
    EXPECT_TRUE(textFieldLayoutProperty->HasMaxLength());
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextLengthLimit(), length);
}

/**
 * @tc.name: TextFieldAccessibilityPropertyIsPassword001
 * @tc.desc: Test IsPassword of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyIsPassword001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto textFieldLayoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    EXPECT_FALSE(textFieldAccessibilityProperty->IsPassword());

    EXPECT_FALSE(textFieldLayoutProperty->HasTextInputType());
    textFieldLayoutProperty->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    EXPECT_TRUE(textFieldLayoutProperty->HasTextInputType());
    EXPECT_TRUE(textFieldAccessibilityProperty->IsPassword());
}

/**
 * @tc.name: TextFieldAccessibilityPropertyGetTextInputType001
 * @tc.desc: Test GetTextInputType of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyGetTextInputType001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto textFieldLayoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    EXPECT_EQ(textFieldAccessibilityProperty->GetTextInputType(), AceTextCategory::INPUT_TYPE_DEFAULT);

    textFieldLayoutProperty->UpdateTextInputType(TextInputType::BEGIN);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextInputType(), AceTextCategory::INPUT_TYPE_DEFAULT);

    textFieldLayoutProperty->UpdateTextInputType(TextInputType::UNSPECIFIED);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextInputType(), AceTextCategory::INPUT_TYPE_DEFAULT);

    textFieldLayoutProperty->UpdateTextInputType(TextInputType::TEXT);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextInputType(), AceTextCategory::INPUT_TYPE_TEXT);

    textFieldLayoutProperty->UpdateTextInputType(TextInputType::MULTILINE);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextInputType(), AceTextCategory::INPUT_TYPE_DEFAULT);

    textFieldLayoutProperty->UpdateTextInputType(TextInputType::NUMBER);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextInputType(), AceTextCategory::INPUT_TYPE_NUMBER);

    textFieldLayoutProperty->UpdateTextInputType(TextInputType::PHONE);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextInputType(), AceTextCategory::INPUT_TYPE_PHONENUMBER);

    textFieldLayoutProperty->UpdateTextInputType(TextInputType::DATETIME);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextInputType(), AceTextCategory::INPUT_TYPE_DATE);

    textFieldLayoutProperty->UpdateTextInputType(TextInputType::EMAIL_ADDRESS);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextInputType(), AceTextCategory::INPUT_TYPE_EMAIL);

    textFieldLayoutProperty->UpdateTextInputType(TextInputType::URL);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextInputType(), AceTextCategory::INPUT_TYPE_DEFAULT);

    textFieldLayoutProperty->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextInputType(), AceTextCategory::INPUT_TYPE_PASSWORD);

    textFieldLayoutProperty->UpdateTextInputType(TextInputType::END);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextInputType(), AceTextCategory::INPUT_TYPE_PASSWORD);
}

/**
 * @tc.name: TextFieldAccessibilityPropertyIsEditable001
 * @tc.desc: Test IsEditable of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyIsEditable001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    EXPECT_TRUE(textFieldAccessibilityProperty->IsEditable());
}

/**
 * @tc.name: TextFieldAccessibilityPropertyIsMultiLine001
 * @tc.desc: Test IsMultiLine of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyIsMultiLine001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto textFieldLayoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    EXPECT_TRUE(textFieldAccessibilityProperty->IsMultiLine());

    textFieldLayoutProperty->UpdateMaxLines(1);
    EXPECT_FALSE(textFieldAccessibilityProperty->IsMultiLine());
}

/**
 * @tc.name: TextFieldAccessibilityPropertyGetTextSelection001
 * @tc.desc: Test GetTextSelection Start and End of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyGetTextSelection001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    EXPECT_EQ(textFieldAccessibilityProperty->GetTextSelectionStart(), TEXT_SELECTION_ERR);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextSelectionEnd(), TEXT_SELECTION_ERR);

    textFieldPattern->textSelector_.Update(TEXT_SELECTION_START, TEXT_SELECTION_END);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextSelectionStart(), TEXT_SELECTION_START);
    EXPECT_EQ(textFieldAccessibilityProperty->GetTextSelectionEnd(), TEXT_SELECTION_END);
}

/**
 * @tc.name: TextFieldAccessibilityPropertyGetText001
 * @tc.desc: Test GetText of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyGetText001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto textFieldLayoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    EXPECT_EQ(textFieldAccessibilityProperty->GetText(), EMPTY_TEXT_VALUE);

    textFieldLayoutProperty->UpdateValue(TEXT_CONTENT);
    EXPECT_EQ(textFieldAccessibilityProperty->GetText(), TEXT_CONTENT);

    textFieldLayoutProperty->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    EXPECT_EQ(textFieldAccessibilityProperty->GetText(), DEFAULT_PASSWORD);
}

/**
 * @tc.name: TextFieldAccessibilityPropertyIsHint001
 * @tc.desc: Test IsHint of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyIsHint001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto textFieldLayoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    EXPECT_FALSE(textFieldAccessibilityProperty->IsHint());

    textFieldLayoutProperty->UpdatePlaceholder(PLACE_HOLDER_TEXT);
    EXPECT_TRUE(textFieldAccessibilityProperty->IsHint());

    textFieldLayoutProperty->UpdateValue(TEXT_CONTENT);
    EXPECT_FALSE(textFieldAccessibilityProperty->IsHint());
}

/**
 * @tc.name: TextFieldAccessibilityPropertyGetHintText001
 * @tc.desc: Test GetHintText of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyGetHintText001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto textFieldLayoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    EXPECT_EQ(textFieldAccessibilityProperty->GetHintText(), EMPTY_TEXT_VALUE);

    textFieldLayoutProperty->UpdatePlaceholder(PLACE_HOLDER_TEXT);
    EXPECT_EQ(textFieldAccessibilityProperty->GetHintText(), PLACE_HOLDER_TEXT);
}

/**
 * @tc.name: TextFieldAccessibilityPropertyGetErrorText001
 * @tc.desc: Test GetErrorText of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyGetErrorText001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    EXPECT_EQ(textFieldAccessibilityProperty->GetErrorText(), EMPTY_TEXT_VALUE);

    textFieldAccessibilityProperty->SetErrorText(TEXT_CONTENT);
    EXPECT_EQ(textFieldAccessibilityProperty->GetErrorText(), TEXT_CONTENT);
}

/**
 * @tc.name: TextFieldAccessibilityPropertyGetContentInvalid001
 * @tc.desc: Test GetContentInvalid of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyGetContentInvalid001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    EXPECT_FALSE(textFieldAccessibilityProperty->GetContentInvalid());

    textFieldAccessibilityProperty->SetErrorText(TEXT_CONTENT);
    EXPECT_TRUE(textFieldAccessibilityProperty->GetContentInvalid());
}

/**
 * @tc.name: TextFieldAccessibilityPropertyIsScrollable001
 * @tc.desc: Test IsScrollable of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyIsScrollable001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    EXPECT_TRUE(textFieldAccessibilityProperty->IsScrollable());

    textFieldPattern->scrollable_ = false;
    EXPECT_FALSE(textFieldAccessibilityProperty->IsScrollable());
}

/**
 * @tc.name: TextFieldAccessibilityPropertyGetSupportAction001
 * @tc.desc: Test GetSupportAction of textfield.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyGetSupportAction001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto textFieldLayoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    textFieldLayoutProperty->UpdateCopyOptions(CopyOptions::InApp);
    textFieldAccessibilityProperty->ResetSupportAction();
    std::unordered_set<AceAction> supportAceActions = textFieldAccessibilityProperty->GetSupportAction();
    uint64_t actions = 0, expectActions = 0;
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_COPY);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_PASTE);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_CUT);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SET_SELECTION);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_CLEAR_SELECTION);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SET_TEXT);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_NEXT_AT_MOVEMENT_GRANULARITY);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY);
    for (auto action : supportAceActions) {
        actions |= 1UL << static_cast<uint32_t>(action);
    }
    EXPECT_EQ(actions, expectActions);
}

/**
 * @tc.name: HandleExtendAction
 * @tc.desc: Verify that the HandleExtendAction interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleExtendAction, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);

    /**
     * @tc.steps: step2. call HandleExtendAction.
     * @tc.expected: No exception occurred during the call.
     */
    auto pipeline = MockPipelineBase::GetCurrent();
    auto clipboard = ClipboardProxy::GetInstance()->GetClipboard(pipeline->GetTaskExecutor());
    textFieldPattern->clipboard_ = clipboard;
    int32_t actions[] = {
        0, // ACTION_SELECT_ALL
        3, // ACTION_CUT
        4, // ACTION_COPY
        5, // ACTION_PASTE
        6, // ACTION_SHARE
    };

    for (auto action : actions) {
        textFieldPattern->HandleExtendAction(action);
    }
}

/**
 * @tc.name: AdjustTextSelectionRectOffsetX
 * @tc.desc: test AdjustTextSelectionRectOffsetX
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, AdjustTextSelectionRectOffsetX, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    layoutProperty->UpdateMaxLines(1);
    textFieldPattern->contentRect_.SetLeft(60.0f);
    textFieldPattern->contentRect_.SetWidth(100.0f);
    textFieldPattern->textRect_.SetLeft(0.0f);
#ifndef USE_GRAPHIC_TEXT_GINE
    RSTypographyProperties::TextBox textBox;
#else
    RSTypographyProperties::TextRect textBox;
#endif
    textFieldPattern->textBoxes_.emplace_back(textBox);

#ifndef USE_GRAPHIC_TEXT_GINE
    textFieldPattern->textBoxes_.begin()->rect_.SetRight(-30.0f);
#else
    textFieldPattern->textBoxes_.begin()->rect.SetRight(-30.0f);
#endif
    textFieldPattern->AdjustTextSelectionRectOffsetX();
    EXPECT_EQ(textFieldPattern->textRect_.GetX(), 60.0f);

#ifndef USE_GRAPHIC_TEXT_GINE
    textFieldPattern->textBoxes_.begin()->rect_.SetLeft(-20.0f);
    textFieldPattern->textBoxes_.begin()->rect_.SetRight(-10.0f);
#else
    textFieldPattern->textBoxes_.begin()->rect.SetLeft(-20.0f);
    textFieldPattern->textBoxes_.begin()->rect.SetRight(-10.0f);
#endif
    textFieldPattern->AdjustTextSelectionRectOffsetX();
    EXPECT_EQ(textFieldPattern->textRect_.GetX(), 60.0f);
    EXPECT_EQ(textFieldPattern->unitWidth_, 0.0f);

#ifndef USE_GRAPHIC_TEXT_GINE
    textFieldPattern->textBoxes_.begin()->rect_.SetRight(110.0f);
#else
    textFieldPattern->textBoxes_.begin()->rect.SetRight(110.0f);
#endif
    textFieldPattern->AdjustTextSelectionRectOffsetX();
    EXPECT_EQ(textFieldPattern->textRect_.GetX(), 60.0f);
}

/**
 * @tc.name: AdjustTextSelectionRectOffsetX001
 * @tc.desc: test AdjustTextSelectionRectOffsetX
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, AdjustTextSelectionRectOffsetX001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    layoutProperty->UpdateMaxLines(1);
    textFieldPattern->contentRect_.SetLeft(0.0f);
    textFieldPattern->contentRect_.SetWidth(100.0f);
    textFieldPattern->textRect_.SetLeft(0.0f);
#ifndef USE_GRAPHIC_TEXT_GINE
    RSTypographyProperties::TextBox textBox;
#else
    RSTypographyProperties::TextRect textBox;
#endif
    textFieldPattern->textBoxes_.emplace_back(textBox);

#ifndef USE_GRAPHIC_TEXT_GINE
    textFieldPattern->textBoxes_.begin()->rect_.SetRight(50.0f);
#else
    textFieldPattern->textBoxes_.begin()->rect.SetRight(50.0f);
#endif
    textFieldPattern->AdjustTextSelectionRectOffsetX();
    EXPECT_EQ(textFieldPattern->textRect_.GetX(), 0.0f);

#ifndef USE_GRAPHIC_TEXT_GINE
    textFieldPattern->textBoxes_.begin()->rect_.SetLeft(-20.0f);
    textFieldPattern->textBoxes_.begin()->rect_.SetRight(-10.0f);
#else
    textFieldPattern->textBoxes_.begin()->rect.SetLeft(-20.0f);
    textFieldPattern->textBoxes_.begin()->rect.SetRight(-10.0f);
#endif
    textFieldPattern->AdjustTextSelectionRectOffsetX();
    EXPECT_EQ(textFieldPattern->textRect_.GetX(), 0.0f);
#ifndef USE_GRAPHIC_TEXT_GINE
    textFieldPattern->textBoxes_.begin()->rect_.SetLeft(0.0f);
#else
    textFieldPattern->textBoxes_.begin()->rect.SetLeft(0.0f);
#endif
    textFieldPattern->textRect_.SetLeft(0.0f);
    textFieldPattern->AdjustTextSelectionRectOffsetX();
    EXPECT_EQ(textFieldPattern->textRect_.GetX(), 0.0f);

#ifndef USE_GRAPHIC_TEXT_GINE
    textFieldPattern->textBoxes_.begin()->rect_.SetLeft(100.0f);
    textFieldPattern->textBoxes_.begin()->rect_.SetRight(200.0f);
#else
    textFieldPattern->textBoxes_.begin()->rect.SetLeft(100.0f);
    textFieldPattern->textBoxes_.begin()->rect.SetRight(200.0f);
#endif
    textFieldPattern->textRect_.SetLeft(0.0f);
    textFieldPattern->AdjustTextSelectionRectOffsetX();
    EXPECT_EQ(textFieldPattern->textRect_.GetX(), 0.0f);
#ifndef USE_GRAPHIC_TEXT_GINE
    textFieldPattern->textBoxes_.begin()->rect_.SetLeft(300.0f);
#else
    textFieldPattern->textBoxes_.begin()->rect.SetLeft(300.0f);
#endif
    textFieldPattern->textRect_.SetLeft(0.0f);
    textFieldPattern->AdjustTextSelectionRectOffsetX();
    EXPECT_EQ(textFieldPattern->textRect_.GetX(), 0.0f);
}

/**
 * @tc.name: SetTextSelection001
 * @tc.desc: test CaretPosition
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetTextSelection001, TestSize.Level1)
{
    int32_t caretPositionCallback = 0;

    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);

    auto textFieldController = AceType::MakeRefPtr<TextFieldController>();
    ASSERT_NE(textFieldController, nullptr);

    textFieldPattern->InitEditingValueText(TEXT_VALUE);
    textFieldController->SetPattern(textFieldPattern);
    textFieldController->SetCaretPosition(
        [&caretPositionCallback](const int32_t caretPosition) { caretPositionCallback = caretPosition; });
    textFieldPattern->SetTextSelection(TEXT_SELECTION_START, TEXT_SELECTION_END);
    textFieldPattern->SetTextSelection(TEXT_SELECTION_ERR, TEXT_SELECTION_END);
    textFieldPattern->SetTextSelection(TEXT_SELECTION_END, TEXT_SELECTION_END);
    EXPECT_EQ(textFieldPattern->GetTextEditingValue().caretPosition, CARET_POSITION_2);
    EXPECT_EQ(textFieldPattern->GetEditingValue().caretPosition, CARET_POSITION_2);
    textFieldController->GetCaretPosition();
    auto index = textFieldController->GetCaretIndex();
    EXPECT_EQ(index, CARET_POSITION_2);
}

/**
 * @tc.name: SetUnderlineWidth
 * @tc.desc: Verify that the SetUnderlineWidth interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetUnderlineWidth, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFieldOverlayModifier.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    TextFieldOverlayModifier textFieldOverlayModifier(pattern, scrollEdgeEffect);

    /**
     * @tc.steps: step2. call SetUnderlineWidth function.
     * @tc.expected: The member variable value of textFieldOverlayModifier is the same as expected.
     */
    float value = 1.0f;
    textFieldOverlayModifier.SetUnderlineWidth(value);
    EXPECT_EQ(textFieldOverlayModifier.underlineWidth_->Get(), value);
}

/**
 * @tc.name: SetUnderlineColor
 * @tc.desc: Verify that the SetUnderlineColor interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetUnderlineColor, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFieldOverlayModifier.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    TextFieldOverlayModifier textFieldOverlayModifier(pattern, scrollEdgeEffect);

    /**
     * @tc.steps: step2. call SetUnderlineColor function.
     * @tc.expected: The member variable value of textFieldOverlayModifier is the same as expected.
     */
    const Color value = Color::BLUE;
    textFieldOverlayModifier.SetUnderlineColor(value);
    EXPECT_EQ(textFieldOverlayModifier.underlineColor_->Get(), value);
}

/**
 * @tc.name: GetUnitWidth
 * @tc.desc: Verify that the GetUnitWidth interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetUnitWidth, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFieldPattern.
     */
    auto textFieldPattern = GetPattern();
    ASSERT_NE(textFieldPattern, nullptr);

    /**
     * @tc.steps: step2. call GetUnitWidth function.
     * @tc.expected: The member variable value of textFieldPattern is the same as expected.
     */
    EXPECT_EQ(textFieldPattern->GetUnitWidth(), 0.0f);

    float unitWidth = 1.0;
    textFieldPattern->unitWidth_ = unitWidth;
    EXPECT_EQ(textFieldPattern->GetUnitWidth(), unitWidth);
}

/**
 * @tc.name: SetShowUnderline001
 * @tc.desc: Verify that the SetShowUnderline interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetShowUnderline001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create frameNode.Get pattern.
     */
    TextFieldModelNG textFieldModelInstance;
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);

    /**
     * @tc.steps: step2. call SetShowUnderline function.
     * @tc.expected: The member variable value of textFieldModelInstance is the same as expected.
     */
    auto layoutProperty = textFieldPattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    const bool hideUnderLine = false;
    textFieldModelInstance.SetShowUnderline(hideUnderLine);
    EXPECT_FALSE(layoutProperty->GetShowUnderlineValue(true));
    const bool showUnderLine = true;
    textFieldModelInstance.SetShowUnderline(showUnderLine);
    EXPECT_TRUE(layoutProperty->GetShowUnderlineValue(false));
}

/**
 * @tc.name: SetShowUnderline002
 * @tc.desc: Verify that the SetShowUnderline interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetShowUnderline002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create frameNode.Get pattern.
     */
    TextFieldModelNG textFieldModelInstance;
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. call SetShowUnderline function.
     * @tc.expected: The member variable value of textFieldModelInstance is the same as expected.
     */
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->propertyChangeFlag_ = PROPERTY_UPDATE_NORMAL;
    textFieldModelInstance.SetShowUnderline(true);
    EXPECT_TRUE(layoutProperty->GetShowUnderlineValue(false));
}

/**
 * @tc.name: HandleFocusEvent
 * @tc.desc: Test the HandleFocusEvent.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleFocusEvent, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = textFieldPattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto paintProperty = textFieldPattern->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step2. Set showUnderLine. Call function HandleFocusEvent.
     * @tc.expected: Check the showUnderLine set successfully.
     */
    auto pipeline = MockPipelineBase::GetCurrent();
    auto clipboard = ClipboardProxy::GetInstance()->GetClipboard(pipeline->GetTaskExecutor());
    textFieldPattern->HandleFocusEvent();
    EXPECT_TRUE(layoutProperty->GetShowUnderlineValue(false));
    layoutProperty->UpdateShowUnderline(true);
    textFieldPattern->HandleFocusEvent();
    EXPECT_TRUE(layoutProperty->GetShowUnderlineValue(false));

    layoutProperty->UpdateTextInputType(TextInputType::UNSPECIFIED);
    textFieldPattern->HandleFocusEvent();
    EXPECT_EQ(layoutProperty->GetTextInputTypeValue(TextInputType::UNSPECIFIED), TextInputType::UNSPECIFIED);

    layoutProperty->UpdateShowErrorText(true);
    textFieldPattern->HandleFocusEvent();
    EXPECT_TRUE(layoutProperty->GetShowErrorTextValue(false));
    layoutProperty->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    textFieldPattern->HandleFocusEvent();

    layoutProperty->UpdateShowErrorText(false);
    textFieldPattern->HandleFocusEvent();
    textFieldPattern->HandleBlurEvent();
}

/**
 * @tc.name: HandleFocusEvent001
 * @tc.desc: Test the HandleFocusEvent.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleFocusEvent001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = textFieldPattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto paintProperty = textFieldPattern->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step2. Set showUnderLine. Call function HandleFocusEvent.
     * @tc.expected: Check the showUnderLine set successfully.
     */
    layoutProperty->UpdateShowUnderline(true);
    textFieldPattern->HandleFocusEvent();
    EXPECT_TRUE(layoutProperty->GetShowUnderlineValue(false));
    layoutProperty->UpdateShowUnderline(false);
    textFieldPattern->HandleFocusEvent();
    EXPECT_FALSE(layoutProperty->GetShowUnderlineValue(false));

    textFieldPattern->caretUpdateType_ = CaretUpdateType::RIGHT_CLICK;
    textFieldPattern->UpdateEditingValue("", 0);
    paintProperty->UpdateInputStyle(InputStyle::INLINE);
    textFieldPattern->HandleFocusEvent();

    paintProperty->UpdateInputStyle(InputStyle::DEFAULT);
    textFieldPattern->HandleFocusEvent();

    paintProperty->UpdateInputStyle(InputStyle::INLINE);
    textFieldPattern->UpdateEditingValue(TEXT_VALUE, 0);
    textFieldPattern->HandleFocusEvent();
    EXPECT_FALSE(layoutProperty->GetShowUnderlineValue(false));
}

/**
 * @tc.name: onDraw004
 * @tc.desc: Verify that the onDraw interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, onDraw004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create frameNode.Get pattern.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    pattern->scrollableEvent_ = AceType::MakeRefPtr<ScrollableEvent>(AXIS_VERTICAL);
    pattern->AddScrollEvent();
    pattern->scrollable_ = true;
    pattern->CheckScrollable();
    bool showUnderLine = true;
    layoutProperty->UpdateShowUnderline(showUnderLine);
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);

    /**
     * @tc.steps: step2. Create the textFieldOverlayModifier.Set different properties.Call function onDraw.
     * @tc.expected: Check the properties.
     */
    TextFieldOverlayModifier textFieldOverlayModifier(pattern, scrollEdgeEffect);
    Testing::MockCanvas rsCanvas;
    DrawingContext context { rsCanvas, CONTEXT_WIDTH_VALUE, CONTEXT_HEIGHT_VALUE };
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DetachBrush()).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, AttachPen(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DetachPen()).WillRepeatedly(ReturnRef(rsCanvas));
    textFieldOverlayModifier.onDraw(context);
}

/**
 * @tc.name: onDraw005
 * @tc.desc: Verify that the onDraw interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, onDraw005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create frameNode.Get pattern.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. Create the TextFieldContentModifier.Set different properties.Call function onDraw.
     * @tc.expected: Check the properties.
     */
    Testing::MockCanvas rsCanvas;
    DrawingContext context { rsCanvas, CONTEXT_WIDTH_VALUE, CONTEXT_HEIGHT_VALUE };
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    layoutProperty->UpdateShowUnderline(true);
    bool value = true;
    textFieldContentModifier.showErrorState_ = AceType::MakeRefPtr<PropertyBool>(value);
    textFieldContentModifier.showCounter_ = AceType::MakeRefPtr<PropertyBool>(value);
    pattern->counterParagraph_ = std::make_shared<RSParagraph>();
    pattern->errorParagraph_ = std::make_shared<RSParagraph>();
    textFieldContentModifier.onDraw(context);
}

/**
 * @tc.name: SetShowUnit
 * @tc.desc: Verify that the SetShowUnit interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetShowUnit, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create frameNode.Get pattern.
     */
    TextFieldModelNG textFieldModelInstance;
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto host = pattern->GetHost();
    ASSERT_NE(host, nullptr);
    std::function<void()> unitAction;

    /**
     * @tc.steps: step2. Create the textFieldModelInstance function SetShowUnit.
     * @tc.expected: Check the properties.
     */
    textFieldModelInstance.SetShowUnit(std::move(unitAction));
    EXPECT_EQ(static_cast<int32_t>(host->GetChildren().size()), 0);

    textFieldModelInstance.SetShowUnit([]() {});
    EXPECT_EQ(static_cast<int32_t>(host->GetChildren().size()), 0);
}

/**
 * @tc.name: ProcessDefaultPadding
 * @tc.desc: Verify that the ProcessDefaultPadding interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, ProcessDefaultPadding, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create frameNode.Get pattern.
     */
    TextFieldModelNG textFieldModelInstance;
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pipeline = frameNode->GetContext();
    ASSERT_NE(pipeline, nullptr);
    auto themeManager = pipeline->GetThemeManager();
    ASSERT_NE(themeManager, nullptr);
    auto textFieldTheme = themeManager->GetTheme<TextFieldTheme>();
    ASSERT_NE(textFieldTheme, nullptr);
    auto themePadding = textFieldTheme->GetPadding();
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. Create the textFieldModelInstance function ProcessDefaultPadding.
     * @tc.expected: Check the properties.
     */
    bool showUnderLine = false;
    layoutProperty->UpdateShowUnderline(showUnderLine);
    auto renderContext = frameNode->GetRenderContext();
    PaddingProperty paddings;
    textFieldModelInstance.ProcessDefaultPadding(paddings);
    EXPECT_EQ(paddings.top, NG::CalcLength(themePadding.Top().ConvertToPx()));
}

/**
 * @tc.name: SetShowCounter
 * @tc.desc: Verify that the SetShowCounter interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetShowCounter, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    bool value = true;

    /**
     * @tc.steps: step2. call SetShowCounter function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    textFieldContentModifier.showCounter_ = nullptr;
    textFieldContentModifier.SetShowCounter(value);
    textFieldContentModifier.showCounter_ = AceType::MakeRefPtr<PropertyBool>(value);
    textFieldContentModifier.SetShowCounter(value);
    EXPECT_EQ(textFieldContentModifier.showCounter_->Get(), value);
}

/**
 * @tc.name: SetShowErrorState
 * @tc.desc: Verify that the SetShowErrorState interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetShowErrorState, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    bool value = true;

    /**
     * @tc.steps: step2. call SetShowErrorState function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    textFieldContentModifier.showErrorState_ = nullptr;
    textFieldContentModifier.SetShowErrorState(value);
    textFieldContentModifier.showErrorState_ = AceType::MakeRefPtr<PropertyBool>(value);
    textFieldContentModifier.SetShowErrorState(value);
    EXPECT_EQ(textFieldContentModifier.showErrorState_->Get(), value);
}

/**
 * @tc.name: CreateCounterParagraph001
 * @tc.desc: Verify that the CreateCounterParagraph interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CreateCounterParagraph001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFieldLayoutAlgorithm.
     */
    TextFieldLayoutAlgorithm textFieldLayoutAlgorithm;

    /**
     * @tc.steps: step2. call CreateCounterParagraph function.
     * @tc.expected: The member variable value of textFieldLayoutAlgorithm is the value set above.
     */
    constexpr int32_t textLength = 5;
    constexpr int32_t maxLength = 10;
    const RefPtr<TextFieldTheme> theme = AceType::MakeRefPtr<TextFieldTheme>();
    EXPECT_EQ(textFieldLayoutAlgorithm.counterParagraph_, nullptr);
    textFieldLayoutAlgorithm.counterParagraph_ = std::make_shared<RSParagraph>();
    textFieldLayoutAlgorithm.CreateCounterParagraph(textLength, maxLength, theme);
    EXPECT_NE(textFieldLayoutAlgorithm.counterParagraph_, nullptr);
}

/**
 * @tc.name: CreateCounterParagraph002
 * @tc.desc: Verify that the CreateCounterParagraph interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CreateCounterParagraph002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFieldLayoutAlgorithm.
     */
    TextFieldLayoutAlgorithm textFieldLayoutAlgorithm;

    /**
     * @tc.steps: step2. call CreateCounterParagraph function.
     * @tc.expected: The member variable value of textFieldLayoutAlgorithm is the value set above.
     */
    constexpr int32_t textLength = 10;
    constexpr int32_t maxLength = 10;
    const RefPtr<TextFieldTheme> theme = AceType::MakeRefPtr<TextFieldTheme>();
    EXPECT_EQ(textFieldLayoutAlgorithm.counterParagraph_, nullptr);
    textFieldLayoutAlgorithm.counterParagraph_ = std::make_shared<RSParagraph>();
    textFieldLayoutAlgorithm.CreateCounterParagraph(textLength, maxLength, theme);
    EXPECT_NE(textFieldLayoutAlgorithm.counterParagraph_, nullptr);
}

/**
 * @tc.name: GetCounterParagraph
 * @tc.desc: Verify that the GetCounterParagraph interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetCounterParagraph, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFieldLayoutAlgorithm.
     */
    TextFieldLayoutAlgorithm textFieldLayoutAlgorithm;

    /**
     * @tc.steps: step2. call CreateCounterParagraph function.
     * @tc.expected: The member variable value of textFieldLayoutAlgorithm is the value set above.
     */
    EXPECT_EQ(textFieldLayoutAlgorithm.GetCounterParagraph(), nullptr);
    textFieldLayoutAlgorithm.counterParagraph_ = std::make_shared<RSParagraph>();
    EXPECT_NE(textFieldLayoutAlgorithm.GetCounterParagraph(), nullptr);
}

/**
 * @tc.name: HandleCounterBorder
 * @tc.desc: Verify that the HandleCounterBorder interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleCounterBorder, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern, TextFieldLayoutProperty.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto textFieldLayoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. Set different properties.Call HandleCounterBorder function.
     * @tc.expected: Check the properties.
     */
    uint32_t length = 32;
    textFieldLayoutProperty->UpdateMaxLength(length);
    auto maxLength = textFieldPattern->GetMaxLength();
    auto editingValueText = std::string("a", maxLength);
    textFieldPattern->UpdateEditingValue(editingValueText, CARET_POSITION_1);
    textFieldPattern->HandleCounterBorder();
    EXPECT_EQ(maxLength, static_cast<uint32_t>(textFieldPattern->textEditingValue_.GetWideText().length()));
    BorderWidthProperty overCountBorderWidth;
    overCountBorderWidth.SetBorderWidth(Dimension(1, DimensionUnit::VP));
    textFieldLayoutProperty->UpdateBorderWidth(overCountBorderWidth);
    textFieldPattern->HandleCounterBorder();
    EXPECT_EQ(maxLength, static_cast<uint32_t>(textFieldPattern->textEditingValue_.GetWideText().length()));
    editingValueText = std::string("a");
    textFieldPattern->UpdateEditingValue(editingValueText, CARET_POSITION_1);
    textFieldPattern->HandleCounterBorder();
    ASSERT_NE(maxLength, static_cast<uint32_t>(textFieldPattern->textEditingValue_.GetWideText().length()));
}

/**
 * @tc.name: HandleTouchDown001
 * @tc.desc: Test the HandleTouchDown.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleTouchDown001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = textFieldPattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. Set properties. Call function HandleTouchDown.
     * @tc.expected: Check the properties set successfully.
     */
    layoutProperty->UpdateShowUnderline(true);
    textFieldPattern->SetEnableTouchAndHoverEffect(true);
    textFieldPattern->isMousePressed_ = false;
    Offset offset;
    textFieldPattern->HandleTouchDown(offset);
    EXPECT_TRUE(layoutProperty->GetShowUnderlineValue(false));
    EXPECT_TRUE(textFieldPattern->enableTouchAndHoverEffect_);
    EXPECT_FALSE(textFieldPattern->GetIsMousePressed());
    layoutProperty->UpdateShowUnderline(false);
    textFieldPattern->HandleTouchDown(offset);
    EXPECT_FALSE(layoutProperty->GetShowUnderlineValue(false));
}

/**
 * @tc.name: HandleTouchUp001
 * @tc.desc: Test the HandleTouchUp.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleTouchUp001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = textFieldPattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. Set properties. Call function HandleTouchUp.
     * @tc.expected: Check the properties set successfully.
     */
    layoutProperty->UpdateShowUnderline(true);
    textFieldPattern->SetEnableTouchAndHoverEffect(true);
    textFieldPattern->isOnHover_ = false;
    textFieldPattern->HandleTouchUp();
    EXPECT_TRUE(textFieldPattern->enableTouchAndHoverEffect_);
    EXPECT_TRUE(layoutProperty->GetShowUnderlineValue(false));
    layoutProperty->UpdateShowUnderline(false);
    textFieldPattern->HandleTouchUp();
    EXPECT_FALSE(layoutProperty->GetShowUnderlineValue(false));
}

/**
 * @tc.name: OnModifyDone001
 * @tc.desc: Test the OnModifyDone.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnModifyDone001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = textFieldPattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. Set showUnderLine. Call function OnModifyDone.
     * @tc.expected: Check the showUnderLine set successfully.
     */
    layoutProperty->UpdateShowUnderline(true);
    textFieldPattern->OnModifyDone();
    EXPECT_TRUE(layoutProperty->GetShowUnderlineValue(false));
    layoutProperty->UpdateShowUnderline(false);
    textFieldPattern->OnModifyDone();
    EXPECT_FALSE(layoutProperty->GetShowUnderlineValue(false));
}

/**
 * @tc.name: OnHover001
 * @tc.desc: Test the OnHover.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnHover001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = textFieldPattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. Set properties. Call function OnHover.
     * @tc.expected: Check the properties set successfully.
     */
    layoutProperty->UpdateShowUnderline(true);
    textFieldPattern->SetEnableTouchAndHoverEffect(true);
    textFieldPattern->OnHover(true);
    EXPECT_TRUE(layoutProperty->GetShowUnderlineValue(false));
    EXPECT_TRUE(textFieldPattern->enableTouchAndHoverEffect_);
    layoutProperty->UpdateShowUnderline(true);
    textFieldPattern->isMousePressed_ = false;
    textFieldPattern->OnHover(false);
    EXPECT_TRUE(layoutProperty->GetShowUnderlineValue(false));
    layoutProperty->UpdateShowUnderline(false);
    textFieldPattern->OnHover(true);
    EXPECT_FALSE(layoutProperty->GetShowUnderlineValue(false));
}

/**
 * @tc.name: PaintCursor002
 * @tc.desc: Verify that the PaintCursor interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, PaintCursor002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectionMode_ = SelectionMode::NONE;
    pattern->paragraph_ = std::make_shared<RSParagraph>();
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    TextFieldOverlayModifier textFieldOverlayModifier(pattern, scrollEdgeEffect);
    Testing::MockCanvas rsCanvas;
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DetachBrush()).WillRepeatedly(ReturnRef(rsCanvas));
    DrawingContext context { rsCanvas, CONTEXT_WIDTH_VALUE, CONTEXT_HEIGHT_VALUE };

    /**
     * @tc.steps: step2. Set properties. Call function PaintCursor.
     * @tc.expected: Check the properties set successfully.
     */
    textFieldOverlayModifier.cursorVisible_ = AceType::MakeRefPtr<PropertyBool>(true);
    textFieldOverlayModifier.showCounter_ = AceType::MakeRefPtr<PropertyBool>(true);
    pattern->counterParagraph_ = std::make_shared<RSParagraph>();
    textFieldOverlayModifier.PaintCursor(context);
}

/**
 * @tc.name: PaintSelection004
 * @tc.desc: Verify that the PaintSelection interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, PaintSelection004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectionMode_ = SelectionMode::SELECT;
    pattern->textSelector_.baseOffset = 1;
    pattern->textSelector_.destinationOffset = 0;
#ifndef USE_GRAPHIC_TEXT_GINE
    std::vector<RSTypographyProperties::TextBox> textBoxes;
    RSTypographyProperties::TextBox textBox;
#else
    std::vector<RSTypographyProperties::TextRect> textBoxes;
    RSTypographyProperties::TextRect textBox;
#endif
    textBoxes.emplace_back(textBox);
    pattern->textBoxes_ = textBoxes;
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    TextFieldOverlayModifier textFieldOverlayModifier(pattern, scrollEdgeEffect);
    Testing::MockCanvas rsCanvas;
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DetachBrush()).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, AttachPen(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DetachPen()).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DrawRoundRect(_)).WillRepeatedly(Return());
    EXPECT_CALL(rsCanvas, DrawCircle(_, _)).WillRepeatedly(Return());
    DrawingContext context { rsCanvas, CONTEXT_WIDTH_VALUE, CONTEXT_HEIGHT_VALUE };

    /**
     * @tc.steps: step2. Set properties. Call function PaintSelection.
     * @tc.expected: Check the properties set successfully.
     */
    textFieldOverlayModifier.showCounter_ = AceType::MakeRefPtr<PropertyBool>(true);
    pattern->counterParagraph_ = std::make_shared<RSParagraph>();
    textFieldOverlayModifier.inputStyle_ = InputStyle::DEFAULT;
    textFieldOverlayModifier.PaintSelection(context);
}

/**
 * @tc.name: PerformActionTest001
 * @tc.desc: TextField Accessibility PerformAction test SetText, SetSelection, ClearSelection, Copy, Cut and Paste.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, PerformActionTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create textField, get textField frameNode and pattern, set callback function.
     * @tc.expected: FrameNode and pattern is not null, related function is called.
     */
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.CreateTextInput(PLACEHOLDER, EMPTY_TEXT_VALUE);
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto textFieldLayoutProperty = textFieldPattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);
    textFieldLayoutProperty->UpdateCopyOptions(CopyOptions::None);
    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);
    textFieldPattern->SetAccessibilityAction();

    /**
     * @tc.steps: step2. When textField is not AllowCopy, call the callback function in textFieldAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionSetText(TEXT_CONTENT));
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionSetSelection(TEXT_SELECTION_START, TEXT_SELECTION_END));
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionClearSelection());
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionCopy());
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionCut());
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionPaste());

    /**
     * @tc.steps: step3. When textField is AllowCopy, call the callback function in textFieldAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    textFieldLayoutProperty->UpdateCopyOptions(CopyOptions::InApp);
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionSetText(TEXT_CONTENT));
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionSetSelection(TEXT_SELECTION_START, TEXT_SELECTION_END));
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionClearSelection());
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionCopy());
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionCut());
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionPaste());
}

/**
 * @tc.name: PerformActionTest002
 * @tc.desc: TextField Accessibility PerformAction test Scroll Forward and Backward.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, PerformActionTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create textField, get textField frameNode and pattern, set callback function.
     * @tc.expected: FrameNode and pattern is not null, related function is called.
     */
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.CreateTextInput(PLACEHOLDER, EMPTY_TEXT_VALUE);
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto textFieldLayoutProperty = textFieldPattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);
    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);
    textFieldPattern->SetAccessibilityAction();

    /**
     * @tc.steps: step2. When textField is not scrollable, call the callback function in textFieldAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    textFieldPattern->scrollable_ = false;
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionScrollForward());
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionScrollBackward());

    /**
     * @tc.steps: step3. When textField is scrollable and scroll distance more than scrollable distance, call the
     *                   callback function in textFieldAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    textFieldPattern->scrollable_ = true;
    textFieldPattern->contentRect_.SetSize(CONTENT_SIZE_FOR_SCROLL);
    textFieldPattern->textRect_.SetSize(TEXT_SIZE_FOR_SCROLL);
    textFieldPattern->textRect_.SetTop(CONTENT_RECT_Y_LARGE);
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionScrollForward());
    textFieldPattern->contentRect_.SetSize(CONTENT_SIZE_FOR_SCROLL);
    textFieldPattern->textRect_.SetSize(TEXT_SIZE_FOR_SCROLL);
    textFieldPattern->textRect_.SetTop(CONTENT_RECT_Y_LARGE);
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionScrollBackward());

    /**
     * @tc.steps: step4. When textField is scrollable and scroll distance less than scrollable distance, call the
     *                   callback function in textFieldAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    textFieldPattern->scrollable_ = true;
    textFieldPattern->contentRect_.SetSize(CONTENT_SIZE_FOR_SCROLL);
    textFieldPattern->textRect_.SetSize(TEXT_SIZE_FOR_SCROLL);
    textFieldPattern->textRect_.SetTop(CONTENT_RECT_Y);
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionScrollForward());
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionScrollBackward());
}

/**
 * @tc.name: PerformActionTest003
 * @tc.desc: TextField Accessibility PerformAction test Move Text.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, PerformActionTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create textField, get textField frameNode and pattern, set callback function.
     * @tc.expected: FrameNode and pattern is not null, related function is called.
     */
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.CreateTextInput(PLACEHOLDER, EMPTY_TEXT_VALUE);
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto textFieldLayoutProperty = textFieldPattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);
    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);
    textFieldPattern->SetAccessibilityAction();

    /**
     * @tc.steps: step2. When editingValue is null, call the callback function in textFieldAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionMoveText(0, false));

    /**
     * @tc.steps: step3. When editingValue is not null, call the callback function in textFieldAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    textFieldPattern->UpdateEditingValue(TEXT_VALUE, static_cast<int32_t>(TEXT_VALUE.size()));
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionMoveText(0, false));
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionMoveText(0, true));
    EXPECT_TRUE(textFieldAccessibilityProperty->ActActionMoveText(1, true));
}

/**
 * @tc.name: TextFieldAccessibilityPropertyIsSelected001
 * @tc.desc: Test text is selected.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldAccessibilityPropertyIsSelected001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create frameNode and get accessibilityProperty
     * @tc.expected: FrameNode and accessibilityProperty is not null
     */
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);

    auto textFieldAccessibilityProperty = frameNode->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    ASSERT_NE(textFieldAccessibilityProperty, nullptr);

    /**
     * @tc.steps: step2. Frame request focus
     * @tc.expected: Default isSelected is false, after reauesting focus, isSelected is true
     */
    EXPECT_FALSE(textFieldAccessibilityProperty->IsSelected());
    textFieldPattern->selectionMode_ = SelectionMode::SELECT;
    EXPECT_EQ(textFieldPattern->textSelector_.destinationOffset, textFieldPattern->textSelector_.baseOffset);
    frameNode->GetOrCreateFocusHub()->currentFocus_ = true;
    textFieldPattern->textSelector_.destinationOffset = 0;
    textFieldPattern->textSelector_.baseOffset = 1;
    EXPECT_TRUE(textFieldAccessibilityProperty->IsSelected());
}

/**
 * @tc.name: TextFieldModel001
 * @tc.desc: Test textfield model
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldModel001, TestSize.Level1)
{
    TextFieldModelNG textFieldModelInstance;
    auto layoutProperty = GetLayoutProperty();
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    ASSERT_NE(layoutProperty, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();

    textFieldModelInstance.SetWidthAuto(true);
    textFieldModelInstance.RequestKeyboardOnFocus(true);
    textFieldModelInstance.SetCaretColor(Color::BLACK);
    textFieldModelInstance.SetTextAlign(TextAlign::CENTER);
    textFieldModelInstance.SetMaxLength(10);
    textFieldModelInstance.ResetMaxLength();
    textFieldModelInstance.SetMaxLines(10);
    textFieldModelInstance.SetFontSize(Dimension(10));
    textFieldModelInstance.SetFontWeight(FontWeight::LIGHTER);
    textFieldModelInstance.SetTextColor(Color::BLACK);
    textFieldModelInstance.SetInputStyle(InputStyle::DEFAULT);
    textFieldModelInstance.SetShowPasswordIcon(true);
    EXPECT_EQ(paintProperty->GetCursorColor(), Color::BLACK);
    EXPECT_EQ(layoutProperty->GetTextAlign(), TextAlign::CENTER);
    EXPECT_EQ(layoutProperty->GetFontWeight(), FontWeight::LIGHTER);
    EXPECT_EQ(layoutProperty->GetTextColor(), Color::BLACK);
    EXPECT_EQ(paintProperty->GetInputStyle(), InputStyle::DEFAULT);
    EXPECT_TRUE(layoutProperty->GetShowPasswordIcon());
}

/**
 * @tc.name: TextFieldModel002
 * @tc.desc: Test textfield model
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldModel002, TestSize.Level1)
{
    TextFieldModelNG textFieldModelInstance;
    auto layoutProperty = GetLayoutProperty();
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    ASSERT_NE(layoutProperty, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();

    Font font;
    textFieldModelInstance.SetPlaceholderFont(font);
    std::vector<std::string> fontFamilies { "Georgia", "Serif" };
    Font otherFont { FontWeight::W200, Dimension(12), OHOS::Ace::FontStyle::ITALIC, fontFamilies };
    PaddingProperty noPadding = CreatePadding(0.0f, 0.0f, 0.0f, 0.0f);
    PaddingProperty Padding = CreatePadding(10.0f, 10.0f, 10.0f, 10.0f);
    Edge edgePadding = Edge(10.0f, 10.0f, 10.0f, 10.0f);
    textFieldModelInstance.SetFontFamily(fontFamilies);
    textFieldModelInstance.SetPlaceholderFont(otherFont);
    textFieldModelInstance.SetFontStyle(Ace::FontStyle::ITALIC);
    textFieldModelInstance.ProcessDefaultPadding(Padding);
    textFieldModelInstance.SetPadding(noPadding, edgePadding, true);
    textFieldModelInstance.SetPadding(Padding, edgePadding, true);
    EXPECT_EQ(layoutProperty->GetItalicFontStyle(), Ace::FontStyle::ITALIC);
}

/**
 * @tc.name: TextFieldModel003
 * @tc.desc: Test textfield model
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldModel003, TestSize.Level1)
{
    TextFieldModelNG textFieldModelInstance;
    auto layoutProperty = GetLayoutProperty();
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    ASSERT_NE(layoutProperty, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<TextFieldEventHub>();
    EXPECT_TRUE(eventHub);
    std::string EventValue;
    auto Event = [&EventValue](const std::string& param) { EventValue = param; };
    textFieldModelInstance.SetOnChange(Event);
    textFieldModelInstance.SetOnCopy(Event);
    textFieldModelInstance.SetOnCut(Event);
    textFieldModelInstance.SetOnPaste(Event);
    eventHub->SetOnChange(std::move(Event));
    eventHub->SetOnCopy(std::move(Event));
    eventHub->SetOnCut(std::move(Event));
    eventHub->SetOnPaste(std::move(Event));

    std::vector<MenuOptionsParam> menuOptionsParam;
    textFieldModelInstance.SetCopyOption(CopyOptions::Local);
    textFieldModelInstance.SetForegroundColor(Color::BLACK);
    textFieldModelInstance.SetMenuOptionItems(std::move(menuOptionsParam));
    textFieldModelInstance.AddDragFrameNodeToManager();
    textFieldModelInstance.SetBackgroundColor(Color::BLACK, true);
    textFieldModelInstance.SetBackgroundColor(Color::BLACK, false);
    textFieldModelInstance.SetHeight(Dimension(10));
    textFieldModelInstance.SetHoverEffect(HoverEffectType::AUTO);
    EXPECT_EQ(layoutProperty->GetTextColor(), Color::BLACK);
    EXPECT_EQ(layoutProperty->GetCopyOptions(), CopyOptions::Local);
}

/**
 * @tc.name: SetShowUnderlineState
 * @tc.desc: Verify that the SetShowUnderlineState interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetShowUnderlineState, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);
    bool value = true;

    /**
     * @tc.steps: step2. call SetShowUnderlineState function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    textFieldContentModifier.showUnderline_ = nullptr;
    textFieldContentModifier.SetShowUnderlineState(value);
    textFieldContentModifier.showUnderline_ = AceType::MakeRefPtr<PropertyBool>(value);
    textFieldContentModifier.SetShowUnderlineState(value);
    EXPECT_EQ(textFieldContentModifier.showUnderline_->Get(), value);
}

/**
 * @tc.name: SetErrorTextValue
 * @tc.desc: Verify that the SetErrorTextValue interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetErrorTextValue, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    TextFieldContentModifier textFieldContentModifier(pattern);

    /**
     * @tc.steps: step2. call SetErrorTextValue function.
     * @tc.expected: The member variable value of textFieldContentModifier is the value set above.
     */
    textFieldContentModifier.errorTextValue_ = AceType::MakeRefPtr<PropertyString>("");
    const std::string value1 = "";
    textFieldContentModifier.SetErrorTextValue(value1);
    const std::string value2 = "ErrorText";
    textFieldContentModifier.SetErrorTextValue(value2);
    EXPECT_EQ(textFieldContentModifier.errorTextValue_->Get(), value2);
}

/**
 * @tc.name: SelectorTest001
 * @tc.desc: test Update function in TextSelector
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SelectorTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. set callback function.
     */
    auto textChange = false;
    auto onTextSelectorChange = [&textChange]() { textChange = true; };
    pattern->textSelector_.SetOnAccessibility(std::move(onTextSelectorChange));

    /**
     * @tc.steps: step3. call callback function.
     * @tc.expected: textSelector_ update successfully.
     */
    pattern->textSelector_.baseOffset = 1;
    pattern->textSelector_.destinationOffset = 5;
    pattern->textSelector_.Update(0);
    EXPECT_TRUE(textChange);
    EXPECT_EQ(pattern->textSelector_.baseOffset, 0);
    EXPECT_EQ(pattern->textSelector_.destinationOffset, 0);

    textChange = false;
    pattern->textSelector_.baseOffset = 1;
    pattern->textSelector_.destinationOffset = 5;
    pattern->textSelector_.Update(0, 0);
    EXPECT_TRUE(textChange);
    EXPECT_EQ(pattern->textSelector_.baseOffset, 0);
    EXPECT_EQ(pattern->textSelector_.destinationOffset, 0);
    pattern->textSelector_.onAccessibilityCallback_ = nullptr;
}

/**
 * @tc.name: ShowOverlay001
 * @tc.desc: Test CheckHandles function
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, ShowOverlay001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create frameNode.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->contentRect_ = RectF(10, 10, 30, 30);

    pattern->parentGlobalOffset_ = OffsetF(50, 50);
    // handles are outside contentRect
    pattern->textSelector_.firstHandleOffset_ = OffsetF(30, 30);
    pattern->textSelector_.secondHandleOffset_ = OffsetF(300, 300);

    std::optional<RectF> firstHandle = RectF(0, 0, 0, 0);
    std::optional<RectF> secondHandle = RectF(0, 0, 0, 0);

    pattern->CheckHandles(firstHandle, secondHandle);
    // handles should be reset after CheckHandles
    EXPECT_EQ(firstHandle, std::nullopt);
    EXPECT_EQ(secondHandle, std::nullopt);
}

/**
 * @tc.name: TextFieldModelNGSetShowUnderLine
 * @tc.desc: test SetShowUnderLine.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldModelNGSetShowUnderLine, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern and TextFieldLayoutProperty.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. test branch - showUnderLine is true and PropertyChangeFlag is PROPERTY_UPDATE_NORMAL.
     * @tc.expected: Check the ShowUnderLine of the pattern.
     */
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.SetShowUnderline(true);
    layoutProperty->UpdatePropertyChangeFlag(PROPERTY_UPDATE_NORMAL);
    EXPECT_TRUE(layoutProperty->GetShowUnderline().value());

    /**
     * @tc.steps: step3. test branch - showUnderLine is false.
     * @tc.expected: Check the ShowUnderLine of the pattern.
     */
    textFieldModelNG.SetShowUnderline(false);
    EXPECT_FALSE(layoutProperty->GetShowUnderline().value());

    /**
     * @tc.steps: step4. test branch - showUnderLine is true and PropertyChangeFlag not equal PROPERTY_UPDATE_NORMAL.
     * @tc.expected: Check the ShowUnderLine of the pattern.
     */
    layoutProperty->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    textFieldModelNG.SetShowUnderline(true);
    EXPECT_TRUE(layoutProperty->GetShowUnderline().value());

    /**
     * @tc.steps: step5. test branch - showUnderLine is true and layoutProperty has text color.
     * @tc.expected: Check the ShowUnderLine of the pattern.
     */
    layoutProperty->UpdateTextColor(Color::FromString("#FF909090"));
    textFieldModelNG.SetShowUnderline(true);
    EXPECT_TRUE(layoutProperty->GetShowUnderline().value());
}

/**
 * @tc.name: TextFieldModelNGProcessDefaultPadding
 * @tc.desc: test ProcessDefaultPadding.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldModelNGProcessDefaultPadding, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Crate TextFieldLayoutProperty and MockThemeManager.
     * @tc.expected: Check if they are nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextFieldTheme>()));

    /**
     * @tc.steps: step2. let ShowUnderLine be true.
     * @tc.expected: Check the paddingProperty.
     */
    TextFieldModelNG textFieldModelNG;
    PaddingProperty paddingProperty;

    layoutProperty->UpdateShowUnderline(true);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->ApplyUnderlineStates();
    Dimension vertical { 12.0, DimensionUnit::PX };
    Dimension horizontal { 0.0, DimensionUnit::PX };
    const std::unique_ptr<PaddingProperty>& property = layoutProperty->GetPaddingProperty();
    ASSERT_TRUE(property->top.has_value());
    EXPECT_EQ(property->top.value().GetDimension().Value(), horizontal.ConvertToPx());
    ASSERT_TRUE(property->bottom.has_value());
    EXPECT_EQ(property->bottom.value().GetDimension().Value(), horizontal.ConvertToPx());
    ASSERT_TRUE(property->left.has_value());
    EXPECT_EQ(property->left.value().GetDimension().Value(), vertical.ConvertToPx());
    ASSERT_TRUE(property->right.has_value());
    EXPECT_EQ(property->right.value().GetDimension().Value(), vertical.ConvertToPx());

    /**
     * @tc.steps: step3. let ShowUnderLine be false.
     * @tc.expected: Check the value of the updated property.
     */
    layoutProperty->UpdateShowUnderline(false);
    textFieldModelNG.ProcessDefaultPadding(paddingProperty);
    Dimension zero { 0.0, DimensionUnit::PX };
    EXPECT_EQ(paddingProperty.top.value().GetDimension(), zero);
    EXPECT_EQ(paddingProperty.bottom.value().GetDimension(), zero);
    EXPECT_EQ(paddingProperty.left.value().GetDimension(), zero);
    EXPECT_EQ(paddingProperty.right.value().GetDimension(), zero);
}

/**
 * @tc.name: TextFieldModelNGSetWidthAuto
 * @tc.desc: test SetWidthAuto.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldModelNGSetWidthAuto, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Crate TextFieldLayoutProperty
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. call SetWidthAuto.
     * @tc.expected: Check the value of WidthAuto.
     */
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.SetWidthAuto(true);
    EXPECT_TRUE(layoutProperty->GetWidthAuto().value_or(false));

    textFieldModelNG.SetWidthAuto(false);
    EXPECT_FALSE(layoutProperty->GetWidthAuto().value_or(true));
}

/**
 * @tc.name: OnDirtyLayoutWrapperSwap
 * @tc.desc: test OnDirtyLayoutWrapperSwap
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnDirtyLayoutWrapperSwap, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create LayoutWrapper.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto geometryNode = frameNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        AceType::WeakClaim(AceType::RawPtr(frameNode)), geometryNode->Clone(), layoutProperty->Clone());
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutAlgorithm = AceType::MakeRefPtr<TextFieldLayoutAlgorithm>();
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));

    /**
     * @tc.steps: step2. Call OnDirtyLayoutWrapperSwap.
     * @tc.expected: Check the return value.
     */
    DirtySwapConfig dirtySwapConfig;
    EXPECT_TRUE(textFieldPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, dirtySwapConfig));

    layoutAlgorithm->paragraph_ = std::make_shared<RSParagraph>();
    layoutAlgorithm->counterParagraph_ = std::make_shared<RSParagraph>();
    layoutAlgorithm->errorParagraph_ = std::make_shared<RSParagraph>();
    layoutAlgorithm->errorParagraph_ = std::make_shared<RSParagraph>();
    textFieldPattern->needToRefreshSelectOverlay_ = true;
    textFieldPattern->mouseStatus_ = MouseStatus::RELEASED;
    auto focusHub = frameNode->GetOrCreateFocusHub();
    if (focusHub) {
        focusHub->currentFocus_ = true;
    }
    auto pipeline = MockPipelineBase::GetCurrent();
    auto clipboard = ClipboardProxy::GetInstance()->GetClipboard(pipeline->GetTaskExecutor());
    textFieldPattern->clipboard_ = clipboard;
    EXPECT_TRUE(textFieldPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, dirtySwapConfig));
    textFieldPattern->inlineFocusState_ = true;
    textFieldPattern->inlineSelectAllFlag_ = true;
    textFieldPattern->updateSelectionAfterObscure_ = true;
    EXPECT_TRUE(textFieldPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, dirtySwapConfig));
    dirtySwapConfig.frameSizeChange = true;
    EXPECT_TRUE(textFieldPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, dirtySwapConfig));
    textFieldPattern->inlineFocusState_ = false;
    dirtySwapConfig.skipMeasure = false;
    EXPECT_TRUE(textFieldPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, dirtySwapConfig));
    textFieldPattern->textEditingValue_.text = "";
    textFieldPattern->CheckScrollable();
    EXPECT_TRUE(textFieldPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, dirtySwapConfig));
    textFieldPattern->updateSelectionAfterObscure_ = true;
    EXPECT_TRUE(textFieldPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, dirtySwapConfig));
    layoutWrapper->skipMeasureContent_ = false;
    dirtySwapConfig.frameSizeChange = false;
    EXPECT_TRUE(textFieldPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, dirtySwapConfig));
    dirtySwapConfig.skipMeasure = true;
    EXPECT_FALSE(textFieldPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, dirtySwapConfig));
    layoutWrapper->skipMeasureContent_ = true;
}

/**
 * @tc.name: TextFieldLayoutAlgorithmMeasure
 * @tc.desc: test TextFieldLayoutAlgorithm.Measure
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldLayoutAlgorithmMeasure, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldLayoutProperty, TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto geometryNode = frameNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. Create LayoutWrapper and call Measure.
     * @tc.expected: Check frame size of GeometryNode.
     */
    auto cloneLayoutProperty = AceType::DynamicCast<TextFieldLayoutProperty>(layoutProperty->Clone());
    auto cloneGeometryNode = geometryNode->Clone();
    auto layoutConstraint = LayoutConstraintF();
    layoutConstraint.selfIdealSize.SetWidth(20);
    layoutConstraint.selfIdealSize.SetHeight(20);
    cloneLayoutProperty->layoutConstraint_ = layoutConstraint;
    LayoutWrapperNode layoutWrapper(
        AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    auto layoutAlgorithm = AceType::MakeRefPtr<TextFieldLayoutAlgorithm>();
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 20);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 20);

    /**
     * @tc.steps: step3. set pattern to TextArea and call Measure.
     * @tc.expected: Check frame size of GeometryNode.
     */
    layoutProperty->UpdateMaxLines(2);
    cloneGeometryNode->SetContentSize(SizeF(10, 10));
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 20);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 20);

    layoutConstraint.maxSize.SetHeight(30);
    layoutConstraint.minSize.SetHeight(40);
    layoutConstraint.maxSize.SetWidth(30);
    layoutConstraint.minSize.SetWidth(40);
    cloneLayoutProperty->layoutConstraint_ = layoutConstraint;
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);

    layoutConstraint.selfIdealSize.Reset();
    cloneLayoutProperty->layoutConstraint_ = layoutConstraint;
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);

    layoutConstraint.selfIdealSize.Reset();
    cloneLayoutProperty->layoutConstraint_ = layoutConstraint;
    cloneLayoutProperty->calcLayoutConstraint_ = std::make_unique<MeasureProperty>();
    cloneLayoutProperty->calcLayoutConstraint_->maxSize = CalcSize(CalcLength(50), CalcLength(50));
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);

    cloneLayoutProperty->calcLayoutConstraint_->maxSize.value().height_.reset();
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);

    cloneLayoutProperty->calcLayoutConstraint_->Reset();
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 40);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 40);

    cloneLayoutProperty->calcLayoutConstraint_->Reset();
    layoutConstraint.minSize.SetHeight(0);
    cloneLayoutProperty->layoutConstraint_ = layoutConstraint;
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 40);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 10);

    /**
     * @tc.steps: step4. Set pattern not be TextArea and call Measure.
     * @tc.expected: Check frame size of GeometryNode.
     */
    layoutProperty->UpdateMaxLines(1);
    layoutConstraint.selfIdealSize.height_.reset();
    cloneLayoutProperty->layoutConstraint_ = layoutConstraint;
    cloneLayoutProperty->UpdateWidthAuto(true);
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 40);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 10);

    cloneLayoutProperty->calcLayoutConstraint_->maxSize = CalcSize(CalcLength(50), CalcLength(50));
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 40);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 30);

    cloneLayoutProperty->calcLayoutConstraint_->Reset();
    layoutConstraint.minSize.SetHeight(5);
    cloneLayoutProperty->layoutConstraint_ = layoutConstraint;
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 40);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 5);

    layoutConstraint.maxSize.SetHeight(10);
    layoutConstraint.minSize.SetHeight(20);
    cloneLayoutProperty->calcLayoutConstraint_ = nullptr;
    cloneLayoutProperty->UpdateShowUnderline(true);
    cloneLayoutProperty->layoutConstraint_ = layoutConstraint;
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 40);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 20);

    auto childrenNode = CreatTextFieldNode();
    ASSERT_NE(childrenNode, nullptr);
    frameNode->children_.push_back(childrenNode);

    layoutConstraint.maxSize.SetHeight(30);
    layoutConstraint.minSize.SetHeight(40);
    layoutConstraint.maxSize.SetWidth(30);
    layoutConstraint.minSize.SetWidth(40);
    cloneLayoutProperty->layoutConstraint_ = layoutConstraint;
    auto contentConstraint = LayoutConstraintF();
    contentConstraint.maxSize.SetHeight(30);
    contentConstraint.minSize.SetHeight(40);
    contentConstraint.maxSize.SetWidth(30);
    contentConstraint.minSize.SetWidth(40);
    cloneLayoutProperty->contentConstraint_ = contentConstraint;
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 40);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 40);
    cloneLayoutProperty->UpdateShowUnderline(true);
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 40);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 40);
    cloneLayoutProperty->UpdateTextInputType(TextInputType::UNSPECIFIED);
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 40);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 40);
}

/**
 * @tc.name: TFLayoutAlgorithmMeasureContent
 * @tc.desc: test TextFieldLayoutAlgorithm.MeasureContent
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TFLayoutAlgorithmMeasureContent, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldLayoutProperty and MockThemeManager, TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto geometryNode = frameNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    auto textFieldTheme = AceType::MakeRefPtr<TextFieldTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_))
        .WillRepeatedly([this, textFieldTheme = textFieldTheme](ThemeType type) -> RefPtr<Theme> {
            if (type == ScrollBarTheme::TypeId()) {
                return AceType::MakeRefPtr<ScrollBarTheme>();
            }
            return textFieldTheme;
        });

    /**
     * @tc.steps: step2. Create LayoutWrapper and call MeasureContent.
     * @tc.expected: Check frame size of GeometryNode.
     */
    auto cloneLayoutProperty = AceType::DynamicCast<TextFieldLayoutProperty>(layoutProperty->Clone());
    auto cloneGeometryNode = geometryNode->Clone();
    auto layoutConstraint = LayoutConstraintF();
    layoutConstraint.maxSize.SetHeight(40);
    layoutConstraint.minSize.SetHeight(30);
    layoutConstraint.maxSize.SetWidth(40);
    layoutConstraint.minSize.SetWidth(30);
    auto contentConstraint = LayoutConstraintF();
    contentConstraint.maxSize.SetHeight(40);
    contentConstraint.minSize.SetHeight(30);
    contentConstraint.maxSize.SetWidth(40);
    contentConstraint.minSize.SetWidth(30);
    cloneLayoutProperty->contentConstraint_ = contentConstraint;
    cloneLayoutProperty->layoutConstraint_ = layoutConstraint;
    LayoutWrapperNode layoutWrapper(
        AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    auto layoutAlgorithm = AceType::MakeRefPtr<TextFieldLayoutAlgorithm>();
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));
    layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 0);
    pattern->dragStatus_ = DragStatus::ON_DROP;
    layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 0);
    cloneLayoutProperty->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));
    layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 0);
    layoutProperty->UpdateMaxLines(1);
    layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 0);

    /**
     * @tc.steps: step3. set pattern to TextArea and call MeasureContent.
     * @tc.expected: Check frame size of GeometryNode.
     */
    cloneLayoutProperty->UpdateValue(TEXT_CONTENT);
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    pattern->dragStatus_ = DragStatus::DRAGGING;
    layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 0);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 0);
    layoutProperty->UpdateMaxLines(1);
    paintProperty->UpdateInputStyle(InputStyle::INLINE);
    cloneLayoutProperty->UpdateTextInputType(TextInputType::UNSPECIFIED);
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    auto focusHub = frameNode->GetOrCreateFocusHub();
    if (focusHub) {
        focusHub->currentFocus_ = true;
    }
    layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 0);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 0);

    layoutProperty->UpdateMaxLines(2);
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 0);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Height(), 0);

    /**
     * @tc.steps: step4. Set pattern not be TextArea and call Measure.
     * @tc.expected: Check frame size of GeometryNode.
     */
    auto textFieldContentModifier = AceType::MakeRefPtr<TextFieldContentModifier>(pattern);
    pattern->textFieldContentModifier_ = textFieldContentModifier;
    layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 0);

    cloneLayoutProperty->UpdateShowCounter(true);
    cloneLayoutProperty->UpdateMaxLength(10);
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 0);
    pattern->counterParagraph_ = std::make_shared<RSParagraph>();
    layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 0);
    cloneLayoutProperty->UpdateShowErrorText(true);
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 0);
    cloneLayoutProperty->UpdateWidthAuto(true);
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    EXPECT_EQ(layoutWrapper.GetGeometryNode()->GetFrameSize().Width(), 0);

    focusHub->currentFocus_ = false;
}

/**
 * @tc.name: TextFieldLayoutAlgorithmLayout
 * @tc.desc: test TextFieldLayoutAlgorithm.Layout
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldLayoutAlgorithmLayout, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldLayoutProperty, TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto geometryNode = frameNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. Create LayoutWrapper and call Layout.
     * @tc.expected: Check text rect of TextFieldLayoutAlgorithm.
     */
    auto cloneLayoutProperty = AceType::DynamicCast<TextFieldLayoutProperty>(layoutProperty->Clone());
    auto cloneGeometryNode = geometryNode->Clone();
    cloneGeometryNode->SetContentSize(SizeF(10, 10));
    auto layoutConstraint = LayoutConstraintF();
    layoutConstraint.selfIdealSize.SetWidth(20);
    layoutConstraint.selfIdealSize.SetHeight(20);
    cloneLayoutProperty->layoutConstraint_ = layoutConstraint;
    LayoutWrapperNode layoutWrapper(
        AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    auto layoutAlgorithm = AceType::MakeRefPtr<TextFieldLayoutAlgorithm>();
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));
    layoutAlgorithm->Layout(&layoutWrapper);

    cloneLayoutProperty->UpdateShowUnderline(true);
    cloneLayoutProperty->UpdateAlignment(Alignment::CENTER);
    layoutProperty->UpdateMaxLines(2);
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->textRect_.SetHeight(5);
    layoutAlgorithm->Layout(&layoutWrapper);
    EXPECT_EQ(layoutAlgorithm->textRect_.Height(), 5);

    layoutAlgorithm->textRect_.SetHeight(20);
    layoutAlgorithm->Layout(&layoutWrapper);
    EXPECT_EQ(layoutAlgorithm->textRect_.Height(), 20);

    cloneLayoutProperty->positionProperty_->ResetAlignment();
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Layout(&layoutWrapper);
    EXPECT_EQ(layoutAlgorithm->textRect_.Height(), 20);

    /**
     * @tc.steps: step3. set non-TextArea and call Layout.
     * @tc.expected: Check text rect of TextFieldLayoutAlgorithm.
     */
    layoutProperty->UpdateMaxLines(1);
    pattern->caretUpdateType_ = CaretUpdateType::INPUT;
    pattern->mouseStatus_ = MouseStatus::PRESSED;
    pattern->isMousePressed_ = false;
    layoutAlgorithm->imageRect_.SetWidth(5);
    layoutAlgorithm->imageRect_.SetHeight(5);
    cloneLayoutProperty->UpdateShowUnderline(true);
    cloneLayoutProperty->UpdateShowErrorText(true);
    layoutProperty->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    layoutProperty->UpdateShowPasswordIcon(true);
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Layout(&layoutWrapper);
    EXPECT_EQ(layoutAlgorithm->textRect_.Height(), 20);

    pattern->UpdateEditingValue(TEXT_VALUE, 0);
    TextAlign textaligns[] = { TextAlign::START, TextAlign::CENTER, TextAlign::END, TextAlign::LEFT };
    for (auto textAlign : textaligns) {
        cloneLayoutProperty->UpdateTextAlign(textAlign);
        layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
        layoutAlgorithm->Layout(&layoutWrapper);
    }

    cloneLayoutProperty->UpdateShowErrorText(false);
    layoutWrapper.SetActive(true);
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Layout(&layoutWrapper);

    layoutWrapper.SetActive(false);
    layoutWrapper.Update(AceType::WeakClaim(AceType::RawPtr(frameNode)), cloneGeometryNode, cloneLayoutProperty);
    layoutAlgorithm->Layout(&layoutWrapper);
    EXPECT_EQ(layoutAlgorithm->textRect_.Height(), 20);
}

/**
 * @tc.name: GetFontFamily
 * @tc.desc: test GetFontFamily.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetFontFamily, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldLayoutProperty.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. Call GetFontFamily.
     * @tc.expected: Check the font family return from GetFontFamily.
     */
    std::vector<std::string> fontFamily = { "Sans", "serif" };
    layoutProperty->UpdateFontFamily(fontFamily);

    auto getFontFamily = pattern->GetFontFamily();
    EXPECT_STREQ(getFontFamily.c_str(), "Sans,serif");
}

/**
 * @tc.name: UpdateCaretPositionByMouseMovement
 * @tc.desc: test UpdateCaretPositionByMouseMovement.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateCaretPositionByMouseMovement, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create frameNode.Get pattern.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. call function UpdateCaretPositionByMouseMovement.
     * @tc.expected: Check the return value.
     */
    EXPECT_FALSE(pattern->UpdateCaretPositionByMouseMovement());
    pattern->UpdateEditingValue(TEXT_VALUE, 5);
    pattern->UpdateCaretPositionByMouseMovement();
    EXPECT_FALSE(pattern->UpdateCaretPositionByMouseMovement());

    pattern->lastTouchOffset_.SetX(8);
    pattern->contentRect_.SetLeft(4);
    pattern->contentRect_.SetWidth(2);
    EXPECT_TRUE(pattern->UpdateCaretPositionByMouseMovement());
    pattern->lastTouchOffset_.SetX(2);
    EXPECT_TRUE(pattern->UpdateCaretPositionByMouseMovement());

    pattern->UpdateEditingValue("", 0);
    EXPECT_FALSE(pattern->UpdateCaretPositionByMouseMovement());
}

/**
 * @tc.name: UpdateSelectionOffset
 * @tc.desc: test UpdateSelectionOffset.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateSelectionOffset, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. setup condition parameters.
     * @tc.expected: Check the value of the updated property.
     */

    /** set clipboard avoid nullpter */
    auto pipeline = MockPipelineBase::GetCurrent();
    auto clipboard = ClipboardProxy::GetInstance()->GetClipboard(pipeline->GetTaskExecutor());
    pattern->clipboard_ = clipboard;
    pattern->SetInSelectMode(SelectionMode::SELECT_ALL);
    pattern->textSelector_.baseOffset = 5;
    pattern->textSelector_.destinationOffset = 6;
    pattern->textRect_.SetLeft(4);
    pattern->textRect_.SetWidth(4);
    pattern->UpdateSelectionOffset();
    EXPECT_EQ(pattern->GetTextSelector().selectionBaseOffset.GetX(), 4);
    EXPECT_EQ(pattern->GetTextSelector().selectionDestinationOffset.GetX(), 8);

#ifndef USE_GRAPHIC_TEXT_GINE
    std::vector<RSTypographyProperties::TextBox> textBoxes;
    RSTypographyProperties::TextBox firstTextBox;
    RSTypographyProperties::TextBox secondTextBox;
#else
    std::vector<RSTypographyProperties::TextRect> textBoxes;
    RSTypographyProperties::TextRect firstTextBox;
    RSTypographyProperties::TextRect secondTextBox;
#endif
    textBoxes.emplace_back(firstTextBox);
    textBoxes.emplace_back(secondTextBox);
    pattern->textBoxes_ = textBoxes;
    pattern->textRect_.SetLeft(5);
    pattern->textRect_.SetWidth(5);
    pattern->textRect_.SetTop(5);
    pattern->textRect_.SetHeight(5);
    pattern->selectOverlayProxy_ = AceType::MakeRefPtr<SelectOverlayProxy>(143);
    EXPECT_EQ(pattern->GetTextSelector().selectionBaseOffset.GetX(), 4);
    EXPECT_EQ(pattern->GetTextSelector().selectionDestinationOffset.GetX(), 8);

    layoutProperty->UpdateMaxLines(1);
    pattern->contentRect_.SetLeft(1);
    pattern->contentRect_.SetWidth(5);
    pattern->contentRect_.SetTop(8);
    pattern->contentRect_.SetHeight(2);
    pattern->parentGlobalOffset_ = OffsetF(0, 0);
    EXPECT_FALSE(pattern->textSelector_.StartEqualToDest());
    EXPECT_TRUE(pattern->selectionMode_ != SelectionMode::NONE);
    pattern->UpdateSelectionOffset();
    EXPECT_EQ(pattern->GetTextSelector().selectionBaseOffset.GetX(), 5);
    EXPECT_EQ(pattern->GetTextSelector().selectionDestinationOffset.GetX(), 10);
    EXPECT_EQ(pattern->textSelector_.firstHandleOffset_.GetX(), 5.0f);
    EXPECT_EQ(pattern->textSelector_.firstHandleOffset_.GetY(), 8.0f);
    EXPECT_EQ(pattern->textSelector_.secondHandleOffset_.GetX(), 5.0f);
    EXPECT_EQ(pattern->textSelector_.secondHandleOffset_.GetY(), 8.0f);
}

/**
 * @tc.name: UpdateCaretPositionByTextEdit
 * @tc.desc: test UpdateCaretPositionByTextEdit.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateCaretPositionByTextEdit, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. let editing text be empty.
     * @tc.expected: Check CaretRect property.
     */
    layoutProperty->UpdateMaxLines(2);
    layoutProperty->UpdateTextAlign(TextAlign::START);
    pattern->textRect_.SetLeft(8);
    pattern->textRect_.SetTop(5);
    pattern->contentRect_.SetLeft(0);
    pattern->contentRect_.SetWidth(6);
    pattern->UpdateEditingValue("", 0);
    std::pair<TextAlign, float> textAligns[] = {
        std::make_pair(TextAlign::START, 8.0f),
        std::make_pair(TextAlign::CENTER, 3.0f),
        std::make_pair(TextAlign::END, 4.5f),
        std::make_pair(TextAlign::RIGHT, 8.0f),
    };
    for (auto align : textAligns) {
        layoutProperty->UpdateTextAlign(align.first);
        pattern->UpdateCaretPositionByTextEdit();
        EXPECT_EQ(pattern->GetCaretRect().GetX(), align.second);
    }
    layoutProperty->UpdateTextAlign(TextAlign::START);

    /**
     * @tc.steps: step3. let editing text caret position be zero.
     * @tc.expected: Check CaretRect property.
     */
    pattern->UpdateEditingValue(TEXT_VALUE, 0);
    pattern->textEditingValue_.caretPosition = 0;
    pattern->UpdateCaretPositionByTextEdit();
    EXPECT_EQ(pattern->GetCaretRect().GetX(), 8);
    EXPECT_EQ(pattern->GetCaretRect().GetY(), 5);

    /**
     * @tc.steps: step4. editing text not empty and caret position > 0.
     * @tc.expected: Check CaretRect property.
     */
    pattern->textEditingValue_.caretPosition = 1;
    pattern->contentRect_.SetLeft(4);
    pattern->UpdateCaretPositionByTextEdit();
    EXPECT_EQ(pattern->GetCaretRect().GetX(), 4);
    EXPECT_EQ(pattern->GetCaretRect().GetY(), 8);
}

/**
 * @tc.name: GetCopyOptionString
 * @tc.desc: test GetCopyOptionString.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetCopyOptionString, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. build all test items and call GetCopyOptionString.
     * @tc.expected: Check the return value.
     */
    struct OptionData {
        CopyOptions options;
        std::string expectOptionString;
    };
    std::vector<OptionData> allOptions = { { CopyOptions::InApp, "CopyOptions.InApp" },
        { CopyOptions::Local, "CopyOptions.Local" }, { CopyOptions::Distributed, "CopyOptions.Distributed" },
        { CopyOptions::None, "CopyOptions.None" } };
    for (auto optionData : allOptions) {
        layoutProperty->UpdateCopyOptions(optionData.options);
        EXPECT_STREQ(pattern->GetCopyOptionString().c_str(), optionData.expectOptionString.c_str());
    }
}

/**
 * @tc.name: UpdateOtherHandleOnMove001
 * @tc.desc: test function UpdateOtherHandleOnMove.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateOtherHandleOnMove001, TestSize.Level1)
{
    auto pattern = GetPattern();
    ASSERT_NE(pattern, nullptr);
    pattern->selectOverlayProxy_ = AceType::MakeRefPtr<SelectOverlayProxy>(-1);
    pattern->isFirstHandle_ = true;
    auto oldSecondHandleOffset = pattern->textSelector_.secondHandleOffset_;
    pattern->UpdateOtherHandleOnMove(1.0f, 1.0f);
    ASSERT_EQ(pattern->textSelector_.secondHandleOffset_.GetX(), oldSecondHandleOffset.GetX() + 1.0f);
    ASSERT_EQ(pattern->textSelector_.secondHandleOffset_.GetY(), oldSecondHandleOffset.GetY() + 1.0f);

    pattern->isFirstHandle_ = false;
    auto oldFirstHandleOffset = pattern->textSelector_.firstHandleOffset_;
    pattern->UpdateOtherHandleOnMove(-1.0f, -1.0f);
    ASSERT_EQ(pattern->textSelector_.firstHandleOffset_.GetX(), oldFirstHandleOffset.GetX() - 1.0f);
    ASSERT_EQ(pattern->textSelector_.firstHandleOffset_.GetY(), oldFirstHandleOffset.GetY() - 1.0f);
}

/**
 * @tc.name: GetMarginBottom
 * @tc.desc: test GetMarginBottom.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetMarginBottom, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. update margin property and call GetMarginBottom.
     * @tc.expected: Check the return value.
     */
    layoutProperty->margin_ = nullptr;
    EXPECT_EQ(pattern->GetMarginBottom(), 0.0f);

    MarginProperty margin;
    layoutProperty->UpdateMargin(margin);
    EXPECT_EQ(pattern->GetMarginBottom(), 0.0f);

    margin.bottom = CalcLength(8);
    layoutProperty->UpdateMargin(margin);
    EXPECT_EQ(pattern->GetMarginBottom(), 8.0f);
}

/**
 * @tc.name: SavePasswordModeStates
 * @tc.desc: test SavePasswordModeStates.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SavePasswordModeStates, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextFieldTheme>()));

    /**
     * @tc.steps: step2. setup layoutProperty and call SavePasswordModeStates.
     * @tc.expected: Check the passwordModeStyle_ padding and borderwidth.
     */
    layoutProperty->borderWidth_ = nullptr;
    layoutProperty->padding_ = nullptr;
    layoutProperty->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    pattern->SavePasswordModeStates();
    auto passwordPadding = &pattern->passwordModeStyle_.padding;
    ASSERT_TRUE(passwordPadding->right.has_value());
    EXPECT_EQ(passwordPadding->right.value().GetDimension().Value(), 0.0);
    auto defaultBorder = &pattern->passwordModeStyle_.borderwidth;
    ASSERT_TRUE(defaultBorder->bottomDimen.has_value());
    EXPECT_EQ(defaultBorder->bottomDimen.value().Value(), 0.0);

    /**
     * @tc.steps: step3. set PaddingProperty, BorderWithProperty and call SavePasswordModeStates.
     * @tc.expected: Check the passwordModeStyle_ padding and borderwidth.
     */
    const double padingLen = 4.0;
    PaddingProperty paddingProperty;
    paddingProperty.right = CalcLength(padingLen);
    layoutProperty->UpdatePadding(paddingProperty);

    const float borderWidth = 3.0;
    BorderWidthProperty borderWidthProperty;
    borderWidthProperty.SetBorderWidth(Dimension(borderWidth));
    layoutProperty->UpdateBorderWidth(borderWidthProperty);

    pattern->SavePasswordModeStates();
    ASSERT_TRUE(passwordPadding->right.has_value());
    EXPECT_EQ(passwordPadding->right.value().GetDimension().Value(), padingLen);
    auto border = &pattern->passwordModeStyle_.borderwidth;
    ASSERT_TRUE(border->bottomDimen.has_value());
    EXPECT_EQ(border->bottomDimen.value().Value(), borderWidth);
}

/**
 * @tc.name: SetShowError
 * @tc.desc: test SetShowError.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, SetShowError, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextFieldTheme>()));

    /**
     * @tc.steps: step2. test scense - error text is visible and in password mode and show underline.
     * @tc.expected: Check several properties that have been changed.
     */
    layoutProperty->UpdateShowErrorText(true);
    layoutProperty->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    layoutProperty->UpdateShowUnderline(true);
    pattern->SetShowError();
    EXPECT_EQ(pattern->GetUnderlineWidth(), 1.0);
    auto borderWithProperty = *(layoutProperty->GetBorderWidthProperty());
    ASSERT_TRUE(borderWithProperty.bottomDimen.has_value());
    EXPECT_EQ(borderWithProperty.bottomDimen.value().Value(), 1.0);

    /**
     * @tc.steps: step3. test scense - error text is visible and not in password mode and hide underline.
     * @tc.expected: no properties that have been changed.
     */
    layoutProperty->UpdateShowUnderline(false);
    layoutProperty->UpdateTextInputType(TextInputType::TEXT);
    BorderWidthProperty borderWidth;
    borderWidth.SetBorderWidth(Dimension(10.0));
    pattern->passwordModeStyle_.borderwidth = borderWidth;
    pattern->SetShowError();
    EXPECT_EQ(pattern->GetUnderlineWidth(), 1.0);
    borderWithProperty = *(layoutProperty->GetBorderWidthProperty());
    ASSERT_TRUE(borderWithProperty.bottomDimen.has_value());
    EXPECT_EQ(borderWithProperty.bottomDimen.value().Value(), 10.0);

    /**
     * @tc.steps: step4. test scense - error text is invisible and in password mode and show underline.
     * @tc.expected: Check several properties that have been changed..
     */
    layoutProperty->UpdateShowErrorText(false);
    layoutProperty->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    layoutProperty->UpdateShowUnderline(true);

    borderWidth.SetBorderWidth(Dimension(5.0));
    pattern->passwordModeStyle_.borderwidth = borderWidth;
    pattern->SetShowError();
    EXPECT_EQ(pattern->GetUnderlineWidth(), 1.0);
    borderWithProperty = *(layoutProperty->GetBorderWidthProperty());
    ASSERT_TRUE(borderWithProperty.bottomDimen.has_value());
    EXPECT_EQ(borderWithProperty.bottomDimen.value().Value(), 5.0);

    /**
     * @tc.steps: step5. test scense - error text is invisible and not in password mode and hide underline.
     * @tc.expected: no properties that have been changed.
     */
    layoutProperty->UpdateShowErrorText(false);
    layoutProperty->UpdateTextInputType(TextInputType::TEXT);
    layoutProperty->UpdateShowUnderline(false);
    pattern->SetShowError();
    EXPECT_EQ(pattern->GetUnderlineWidth(), 1.0);
    borderWithProperty = *(layoutProperty->GetBorderWidthProperty());
    ASSERT_TRUE(borderWithProperty.bottomDimen.has_value());
    EXPECT_EQ(borderWithProperty.bottomDimen.value().Value(), 5.0);
}

/**
 * @tc.name: GetScrollBarWidth
 * @tc.desc: test GetScrollBarWidth.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetScrollBarWidth, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. show scrollbar and call GetScrollBarWidth.
     * @tc.expected: Check the return value.
     */
    pattern->scrollBar_ = nullptr;
    EXPECT_EQ(pattern->GetScrollBarWidth(), 0.0);
    auto scrollBar = AceType::MakeRefPtr<ScrollBar>();
    Offset offset;
    pattern->scrollBar_ = scrollBar;
    EXPECT_EQ(pattern->GetScrollBarWidth(), 0.0);
}

/**
 * @tc.name: GetMaxLines
 * @tc.desc: test GetMaxLines.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetMaxLines, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. call GetMaxLines.
     * @tc.expected: Check the return value.
     */
    layoutProperty->ResetMaxLines();
    EXPECT_EQ(pattern->GetMaxLines(), Infinity<uint32_t>());
    layoutProperty->UpdateMaxLines(20);
    EXPECT_EQ(pattern->GetMaxLines(), 20);
}

/**
 * @tc.name: GetPlaceholderFont
 * @tc.desc: test GetPlaceholderFont.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetPlaceholderFont, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextFieldTheme>()));

    /**
     * @tc.steps: step2. set the required parameters and call GetPlaceholderFont.
     * @tc.expected: Check the return value.
     */
    layoutProperty->ResetPlaceholderFontStyle();
    layoutProperty->UpdateFontSize(Dimension(5.0));
    layoutProperty->UpdatePlaceholderItalicFontStyle(Ace::FontStyle::NORMAL);
    layoutProperty->UpdatePlaceholderFontWeight(FontWeight::BOLD);
    std::vector<std::string> fontFamilies { "Georgia", "Serif" };
    layoutProperty->UpdatePlaceholderFontFamily(fontFamilies);
    auto placeholderFont = pattern->GetPlaceholderFont();
    auto parsedValue = JsonUtil::ParseJsonString(placeholderFont);
    EXPECT_STREQ(parsedValue->GetString("style").c_str(), "FontStyle.Normal");
    EXPECT_STREQ(parsedValue->GetString("size").c_str(), "5.00px");
    EXPECT_STREQ(parsedValue->GetString("fontWeight").c_str(), "FontWeight.Bold");
    EXPECT_STREQ(parsedValue->GetString("fontFamily").c_str(), "Georgia,Serif");
    layoutProperty->UpdatePlaceholderItalicFontStyle(Ace::FontStyle::ITALIC);
    layoutProperty->UpdatePlaceholderFontSize(Dimension(8.0));
    placeholderFont = pattern->GetPlaceholderFont();
    parsedValue = JsonUtil::ParseJsonString(placeholderFont);
    EXPECT_STREQ(parsedValue->GetString("style").c_str(), "FontStyle.Italic");
    EXPECT_STREQ(parsedValue->GetString("size").c_str(), "8.00px");

    /**
     * @tc.steps: step2. test all number font weight.
     * @tc.expected: Check the return value.
     */
    std::pair<FontWeight, std::string> numberFontWeights[] = { std::make_pair(FontWeight::W100, "100"),
        std::make_pair(FontWeight::W200, "200"), std::make_pair(FontWeight::W300, "300"),
        std::make_pair(FontWeight::W400, "400"), std::make_pair(FontWeight::W500, "500"),
        std::make_pair(FontWeight::W600, "600"), std::make_pair(FontWeight::W700, "700"),
        std::make_pair(FontWeight::W800, "800"), std::make_pair(FontWeight::W900, "900") };
    for (auto weight : numberFontWeights) {
        layoutProperty->UpdatePlaceholderFontWeight(weight.first);
        placeholderFont = pattern->GetPlaceholderFont();
        parsedValue = JsonUtil::ParseJsonString(placeholderFont);
        EXPECT_STREQ(parsedValue->GetString("weight").c_str(), weight.second.c_str());
    }
}

/**
 * @tc.name: ToJsonValue and FromJson
 * @tc.desc: test ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, ToJsonValue, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. updates the properties to be checked and call ToJsonValue.
     * @tc.expected: Check the properties.
     */
    layoutProperty->UpdateItalicFontStyle(Ace::FontStyle::NORMAL);
    layoutProperty->ResetMaxLength();
    layoutProperty->UpdateShowErrorText(true);
    layoutProperty->UpdateErrorText("Error Text");
    layoutProperty->ResetMaxLines();
    auto json = JsonUtil::Create(true);
    pattern->ToJsonValue(json);
    EXPECT_STREQ(json->GetString("fontStyle").c_str(), "FontStyle.Normal");
    EXPECT_STREQ(json->GetString("maxLength").c_str(), "INF");
    EXPECT_STREQ(json->GetString("showError").c_str(), "Error Text");
    EXPECT_STREQ(json->GetString("maxLines").c_str(), "INF");

    layoutProperty->UpdateItalicFontStyle(Ace::FontStyle::ITALIC);
    layoutProperty->UpdateMaxLength(18);
    layoutProperty->UpdateShowErrorText(false);
    layoutProperty->UpdateMaxLines(5);
    json = JsonUtil::Create(true);
    pattern->ToJsonValue(json);
    EXPECT_STREQ(json->GetString("fontStyle").c_str(), "FontStyle.Italic");
    EXPECT_STREQ(json->GetString("maxLength").c_str(), "18");
    EXPECT_STREQ(json->GetString("showError").c_str(), "undefined");
    EXPECT_STREQ(json->GetString("maxLines").c_str(), "5");
}

/**
 * @tc.name: TextInputActionToString.
 * @tc.desc: test TextInputActionToString.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextInputActionToString, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. update TextInputAction and call TextInputActionToString.
     * @tc.expected: Check the return string.
     */
    std::pair<TextInputAction, std::string> inputTextInputActions[] = { std::make_pair(
                                                                            TextInputAction::GO, "EnterKeyType.Go"),
        std::make_pair(TextInputAction::SEARCH, "EnterKeyType.Search"),
        std::make_pair(TextInputAction::SEND, "EnterKeyType.Send"),
        std::make_pair(TextInputAction::DONE, "EnterKeyType.Done"),
        std::make_pair(TextInputAction::NEXT, "EnterKeyType.Next") };
    for (auto action : inputTextInputActions) {
        pattern->UpdateTextInputAction(action.first);
        EXPECT_STREQ(pattern->TextInputActionToString().c_str(), action.second.c_str());
    }
}

/**
 * @tc.name: CaretMoveToLastNewLineChar.
 * @tc.desc: test CaretMoveToLastNewLineChar.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CaretMoveToLastNewLineChar, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. update editing value and  call CaretMoveToLastNewLineChar.
     * @tc.expected: Check the editing value's crePosition.
     */
    pattern->UpdateEditingValue("New\nLine\nChar", 0);
    pattern->CaretMoveToLastNewLineChar();
    EXPECT_EQ(pattern->textEditingValue_.caretPosition, 0);
    pattern->UpdateEditingValue("New\nLine\nChar", 10);
    pattern->CaretMoveToLastNewLineChar();
    EXPECT_EQ(pattern->textEditingValue_.caretPosition, 8);
}

/**
 * @tc.name: HandleSelect.
 * @tc.desc: test HandleSelect.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleSelect, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. call HandleSelect with expectd parameters.
     * @tc.expected: Check the value of the updated property.
     */
    int32_t keyCodes[] = {
        2014, // KeyCode::KEY_DPAD_LEFT,
        2015, // KeyCode::KEY_DPAD_RIGHT,
        2012, // KeyCode::KEY_DPAD_UP,
        2013, // KeyCode::KEY_DPAD_DOWN
        2016, // KeyCode::KEY_DPAD_CENTER
    };
    for (auto kCode : keyCodes) {
        pattern->caretUpdateType_ = CaretUpdateType::NONE;
        pattern->HandleSelect(kCode, 0);
        EXPECT_EQ(pattern->caretUpdateType_, CaretUpdateType::EVENT);
    }
}

/**
 * @tc.name: HandleSelectionRight.
 * @tc.desc: test HandleSelectionRight.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleSelectionRight, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. not in select mode and call HandleSelectionRight.
     * @tc.expected: Check the value of the updated property.
     */
    pattern->selectionMode_ = SelectionMode::NONE;
    pattern->textSelector_.baseOffset = 0;
    pattern->UpdateEditingValue("HandleSelectionRight", 20);
    pattern->HandleSelectionRight();
    pattern->UpdateEditingValue("HandleSelectionRight", 10);
    pattern->HandleSelectionRight();
    EXPECT_EQ(pattern->GetSelectMode(), SelectionMode::SELECT);
    EXPECT_EQ(pattern->GetTextSelector().GetStart(), 10);
    EXPECT_EQ(pattern->GetTextSelector().GetEnd(), 1);

    /**
     * @tc.steps: step2. in select mode and call HandleSelectionRight.
     * @tc.expected: Check the value of the updated property.
     */
    pattern->selectionMode_ = SelectionMode::SELECT;
    pattern->textSelector_.baseOffset = 12;
    pattern->textSelector_.destinationOffset = 10;
    pattern->UpdateEditingValue("HandleSelectionRight", 20);
    pattern->HandleSelectionRight();
    EXPECT_EQ(pattern->GetSelectMode(), SelectionMode::SELECT);
    EXPECT_EQ(pattern->GetTextSelector().GetEnd(), 11);

    pattern->selectionMode_ = SelectionMode::SELECT;
    pattern->textSelector_.baseOffset = 11;
    pattern->textSelector_.destinationOffset = 10;
    pattern->UpdateEditingValue("HandleSelectionRight", 20);
    pattern->HandleSelectionRight();
    EXPECT_EQ(pattern->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern->GetTextSelector().GetEnd(), 11);
}

/**
 * @tc.name: HandleSelectionLeft.
 * @tc.desc: test HandleSelectionLeft.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleSelectionLeft, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. not in select mode and call HandleSelectionLeft.
     * @tc.expected: Check the value of the updated property.
     */
    pattern->UpdateEditingValue("HandleSelectionLeft", 0);
    pattern->selectionMode_ = SelectionMode::NONE;
    pattern->HandleSelectionLeft();
    EXPECT_EQ(pattern->GetSelectMode(), SelectionMode::NONE);

    pattern->textEditingValue_.caretPosition = 10;
    pattern->textSelector_.baseOffset = 5;
    pattern->HandleSelectionLeft();
    EXPECT_EQ(pattern->GetSelectMode(), SelectionMode::SELECT);

    /**
     * @tc.steps: step3. in select mode and call HandleSelectionLeft.
     * @tc.expected: Check the value of the updated property.
     */
    pattern->selectionMode_ = SelectionMode::SELECT;
    pattern->textSelector_.baseOffset = 5;
    pattern->HandleSelectionLeft();
    EXPECT_EQ(pattern->GetSelectMode(), SelectionMode::SELECT);

    pattern->textSelector_.baseOffset = 4;
    pattern->textSelector_.destinationOffset = 5;
    pattern->HandleSelectionLeft();
    EXPECT_EQ(pattern->GetSelectMode(), SelectionMode::NONE);
}

/**
 * @tc.name: HandleSelectionDown.
 * @tc.desc: test HandleSelectionDown.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleSelectionDown, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. not in text area and call HandleSelectionDown.
     * @tc.expected: Check the value of the updated property.
     */
    layoutProperty->UpdateMaxLines(1);
    pattern->selectionMode_ = SelectionMode::NONE;
    pattern->HandleSelectionDown();
    EXPECT_EQ(pattern->GetSelectMode(), SelectionMode::NONE);

    /**
     * @tc.steps: step3. in text area and call HandleSelectionDown.
     * @tc.expected: Check the value of the updated property.
     */
    pattern->paragraph_ = std::make_shared<RSParagraph>();
    layoutProperty->UpdateMaxLines(2);
    pattern->selectionMode_ = SelectionMode::SELECT;
    pattern->HandleSelectionDown();
    pattern->selectionMode_ = SelectionMode::NONE;
    pattern->caretRect_.SetTop(0);
    pattern->caretRect_.SetLeft(0);
    pattern->textRect_.SetTop(0);
    pattern->HandleSelectionDown();
    EXPECT_EQ(pattern->GetSelectMode(), SelectionMode::NONE);

    pattern->textSelector_.baseOffset = 1;
    pattern->HandleSelectionDown();
    EXPECT_EQ(pattern->GetSelectMode(), SelectionMode::NONE);
}

/**
 * @tc.name: HandleSelectionUp.
 * @tc.desc: test HandleSelectionUp.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleSelectionUp, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. not in text area and call HandleSelectionUp.
     * @tc.expected: Check the value of the updated property.
     */
    layoutProperty->UpdateMaxLines(1);
    pattern->selectionMode_ = SelectionMode::NONE;
    pattern->HandleSelectionUp();
    EXPECT_EQ(pattern->GetSelectMode(), SelectionMode::NONE);

    /**
     * @tc.steps: step3. in text area and call HandleSelectionUp.
     * @tc.expected: Check the value of the updated property.
     */
    pattern->paragraph_ = std::make_shared<RSParagraph>();
    layoutProperty->UpdateMaxLines(2);
    pattern->selectionMode_ = SelectionMode::SELECT;
    pattern->HandleSelectionUp();
    pattern->selectionMode_ = SelectionMode::NONE;
    pattern->caretRect_.SetTop(0);
    pattern->caretRect_.SetLeft(0);
    pattern->textRect_.SetTop(0);
    pattern->HandleSelectionUp();
    EXPECT_EQ(pattern->GetSelectMode(), SelectionMode::NONE);
    pattern->textSelector_.baseOffset = 1;
    pattern->HandleSelectionUp();
    EXPECT_EQ(pattern->GetSelectMode(), SelectionMode::NONE);
}

/**
 * @tc.name: TextFieldModelSetPadding
 * @tc.desc: Test textfield model SetPadding
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldModelSetPadding, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldModelNG and TextFieldLayoutProperty.
     * @tc.expected: Check it is not nullptr.
     */
    TextFieldModelNG textFieldModelInstance;
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. set padding use new padding.
     * @tc.expected: Check paddingProperty value.
     */
    PaddingProperty noPadding = CreatePadding(0.0f, 0.0f, 0.0f, 0.0f);
    PaddingProperty padding = CreatePadding(10.0f, 10.0f, 10.0f, 10.0f);
    Edge edgePadding = Edge(10.0f, 10.0f, 10.0f, 10.0f);
    const std::unique_ptr<PaddingProperty>& paddingProperty = layoutProperty->GetPaddingProperty();
    textFieldModelInstance.SetPadding(padding, edgePadding, false);
    ASSERT_TRUE(paddingProperty->left.has_value());
    EXPECT_EQ(paddingProperty->left.value().GetDimension().Value(), 10.0f);

    /**
     * @tc.steps: step3. update theme padding and call SetPadding.
     * @tc.expected: Check paddingProperty value.
     */
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    auto theme = pipeline->GetThemeManager()->GetTheme<TextFieldTheme>();
    ASSERT_NE(theme, nullptr);
    std::vector<AnimatableDimension*> paddings = { &theme->padding_.top_, &theme->padding_.bottom_,
        &theme->padding_.left_, &theme->padding_.right_ };
    for (auto pd : paddings) {
        pd->SetValue(5);
        pd->SetCalcValue("10");
        pd->SetUnit(DimensionUnit::CALC);
    }
    textFieldModelInstance.SetPadding(padding, edgePadding, true);
    std::vector<std::optional<CalcLength>*> checkPaddings = { &paddingProperty->top, &paddingProperty->bottom,
        &paddingProperty->left, &paddingProperty->right };
    for (auto pd : checkPaddings) {
        ASSERT_TRUE(pd->has_value());
        EXPECT_STREQ(pd->value().CalcValue().c_str(), "10");
    }

    /**
     * @tc.steps: step4. set padding unit to PX and call SetPadding.
     * @tc.expected: Check paddingProperty value.
     */
    for (auto pd : paddings) {
        pd->SetValue(5);
        pd->SetUnit(DimensionUnit::PX);
    }
    textFieldModelInstance.SetPadding(padding, edgePadding, true);
    for (auto pd : checkPaddings) {
        ASSERT_TRUE(pd->has_value());
        EXPECT_EQ(pd->value().GetDimension().Value(), 5);
    }
}

/**
 * @tc.name: GetInputStyleString
 * @tc.desc: test GetInputStyleString.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetInputStyleString, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto paintProperty = pattern->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    /**
     * @tc.steps: step2. call GetInputStyleString.
     * @tc.expected: Check the return value.
     */
    paintProperty->UpdateInputStyle(InputStyle::INLINE);
    EXPECT_STREQ(pattern->GetInputStyleString().c_str(), "TextContentStyle.INLINE");
    paintProperty->UpdateInputStyle(InputStyle::DEFAULT);
    EXPECT_STREQ(pattern->GetInputStyleString().c_str(), "TextContentStyle.DEFAULT");
}

/**
 * @tc.name: TextInputTypeToString
 * @tc.desc: test TextInputTypeToString.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextInputTypeToString, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    /**
     * @tc.steps: step2. call TextInputTypeToString.
     * @tc.expected: Check the return value.
     */
    std::pair<TextInputType, std::string> inputTypes[] = {
        std::make_pair(TextInputType::NUMBER, "InputType.Number"),
        std::make_pair(TextInputType::EMAIL_ADDRESS, "InputType.Email"),
        std::make_pair(TextInputType::VISIBLE_PASSWORD, "InputType.Password"),
        std::make_pair(TextInputType::PHONE, "InputType.Normal"),
        std::make_pair(TextInputType::UNSPECIFIED, "InputType.Normal"),
    };
    for (auto inputType : inputTypes) {
        layoutProperty->UpdateTextInputType(inputType.first);
        EXPECT_STREQ(pattern->TextInputTypeToString().c_str(), inputType.second.c_str());
    }
}

/**
 * @tc.name: OnVisibleChange
 * @tc.desc: test OnVisibleChange.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnVisibleChange, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    /**
     * @tc.steps: step2. call OnVisibleChange.
     * @tc.expected: Check the value of the updated property.
     */
    pattern->caretUpdateType_ = CaretUpdateType::NONE;
    pattern->OnVisibleChange(true);
    EXPECT_EQ(pattern->caretUpdateType_, CaretUpdateType::NONE);
    pattern->OnVisibleChange(false);
    EXPECT_EQ(pattern->caretUpdateType_, CaretUpdateType::INPUT);
}

/**
 * @tc.name: OnAreaChangedInner
 * @tc.desc: test OnAreaChangedInner.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnAreaChangedInner, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    /**
     * @tc.steps: step2. call OnAreaChangedInner.
     * @tc.expected: Check the value of the updated property.
     */
    pattern->OnAreaChangedInner();
    pattern->needToRequestKeyboardOnFocus_ = true;
    pattern->needToRequestKeyboardInner_ = true;
    pattern->OnAreaChangedInner();

    pattern->parentGlobalOffset_.SetX(1);
    pattern->parentGlobalOffset_.SetY(1);
    pattern->isSingleHandle_ = true;
    pattern->selectOverlayProxy_ = AceType::MakeRefPtr<SelectOverlayProxy>(-1);
    pattern->OnAreaChangedInner();
    EXPECT_EQ(pattern->parentGlobalOffset_.GetX(), 0);
    EXPECT_EQ(pattern->parentGlobalOffset_.GetY(), 0);

    pattern->parentGlobalOffset_.SetX(1);
    pattern->parentGlobalOffset_.SetY(1);
    pattern->isSingleHandle_ = false;
    pattern->OnAreaChangedInner();
    EXPECT_EQ(pattern->selectionMode_, SelectionMode::NONE);
}

/**
 * @tc.name: CreateSingleHandle
 * @tc.desc: test CreateSingleHandle.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CreateSingleHandle, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    /**
     * @tc.steps: step2. call CreateSingleHandle.
     * @tc.expected: Check the value of the updated property.
     */
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->OnModifyDone();
    pattern->textEditingValue_.Reset();
    EXPECT_TRUE(pattern->textEditingValue_.Empty());
    pattern->CreateSingleHandle();
}

/**
 * @tc.name: UpdateEditingValue
 * @tc.desc: test UpdateEditingValue.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateEditingValue, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    /**
     * @tc.steps: step2. call UpdateEditingValue.
     * @tc.expected: Check the value of the updated property.
     */
    auto value = std::make_shared<TextEditingValue>();
    value->text = TEXT_VALUE;
    value->selection.baseOffset = 5;
    pattern->UpdateEditingValue(value, true);
    EXPECT_STREQ(pattern->textEditingValue_.text.c_str(), TEXT_VALUE.c_str());
    pattern->UpdateEditingValue(value, false);
    EXPECT_STREQ(pattern->textEditingValue_.text.c_str(), TEXT_VALUE.c_str());

    auto* stack = ViewStackProcessor::GetInstance();
    int32_t nodeId = stack->ClaimNodeId();
    auto searchNode = AceType::MakeRefPtr<FrameNode>(V2::SEARCH_ETS_TAG, nodeId, AceType::MakeRefPtr<Pattern>(), false);
    frameNode->MountToParent(searchNode);
    pattern->UpdateEditingValue(value, false);
    EXPECT_STREQ(pattern->textEditingValue_.text.c_str(), TEXT_VALUE.c_str());
}

/**
 * @tc.name: PerformAction
 * @tc.desc: test PerformAction.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, PerformAction, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. call PerformAction.
     * @tc.expected: Check the value of the updated property.
     */
    layoutProperty->UpdateMaxLines(1);
    auto eventHub = frameNode->GetEventHub<TextFieldEventHub>();
    int32_t resultAction = -1;
    eventHub->SetOnSubmit([&resultAction](int32_t action) { resultAction = action; });
    pattern->PerformAction(TextInputAction::SEARCH);
    layoutProperty->UpdateMaxLines(2);
    layoutProperty->UpdateInputFilter("\n");
    pattern->PerformAction(TextInputAction::SEARCH);
    pattern->textEditingValue_.text = TEXT_VALUE;
    layoutProperty->UpdateInputFilter("TEXT");
    pattern->PerformAction(TextInputAction::SEARCH);
    auto* stack = ViewStackProcessor::GetInstance();
    int32_t nodeId = stack->ClaimNodeId();
    auto searchNode = AceType::MakeRefPtr<FrameNode>(V2::SEARCH_ETS_TAG, nodeId, AceType::MakeRefPtr<Pattern>(), false);
    frameNode->MountToParent(searchNode);
    pattern->PerformAction(TextInputAction::SEARCH);
    EXPECT_EQ(resultAction, 3);
}

/**
 * @tc.name: CursorMove
 * @tc.desc: test cursor move to up,down,right,left.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CursorMove, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. call CursorMoveDown.
     * @tc.expected: Check the return value.
     */
    layoutProperty->UpdateMaxLines(2);
    pattern->UpdateEditingValue(TEXT_VALUE, 5);
    EXPECT_TRUE(pattern->CursorMoveDown());

    /**
     * @tc.steps: step3. call CursorMoveUp.
     * @tc.expected: Check the return value.
     */
    pattern->UpdateEditingValue(TEXT_VALUE, 5);
    EXPECT_TRUE(pattern->CursorMoveUp());

    /**
     * @tc.steps: step4. call CursorMoveRight.
     * @tc.expected: Check the return value.
     */
    pattern->selectionMode_ = SelectionMode::SELECT_ALL;
    pattern->textSelector_.baseOffset = 0;
    pattern->textSelector_.destinationOffset = 4;
    pattern->UpdateEditingValue(TEXT_VALUE, 5);
    EXPECT_TRUE(pattern->CursorMoveRight());
    EXPECT_EQ(pattern->textEditingValue_.caretPosition, TEXT_VALUE.size());
    pattern->selectionMode_ = SelectionMode::NONE;
    pattern->UpdateEditingValue(TEXT_VALUE, 5);
    EXPECT_TRUE(pattern->CursorMoveRight());

    /**
     * @tc.steps: step5. call CursorMoveLeft.
     * @tc.expected: Check the return value.
     */
    pattern->selectionMode_ = SelectionMode::SELECT_ALL;
    pattern->textSelector_.baseOffset = 0;
    pattern->textSelector_.destinationOffset = 4;
    pattern->UpdateEditingValue(TEXT_VALUE, 5);
    EXPECT_TRUE(pattern->CursorMoveLeft());
    EXPECT_EQ(pattern->textEditingValue_.caretPosition, 0);
    pattern->selectionMode_ = SelectionMode::NONE;
    pattern->UpdateEditingValue(TEXT_VALUE, 5);
    EXPECT_TRUE(pattern->CursorMoveLeft());
}

/**
 * @tc.name: HandleMouseEvent001
 * @tc.desc: Verify that the HandleMouseEvent interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleMouseEvent001, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. call HandleMouseEvent.
     * @tc.expected: Check that no exception occurs.
     */
    MouseAction actions[] = { MouseAction::PRESS, MouseAction::RELEASE, MouseAction::MOVE };
    MouseInfo mouseInfo;
    for (auto action : actions) {
        mouseInfo.SetButton(MouseButton::NONE_BUTTON);
        mouseInfo.SetAction(action);
        pattern->HandleMouseEvent(mouseInfo);
        mouseInfo.SetButton(MouseButton::RIGHT_BUTTON);
        pattern->HandleMouseEvent(mouseInfo);
    }
    layoutProperty->UpdateShowPasswordIcon(false);
    mouseInfo.SetLocalLocation(Offset(10, 10));
    pattern->HandleMouseEvent(mouseInfo);
    layoutProperty->UpdateShowPasswordIcon(true);
    layoutProperty->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    pattern->HandleMouseEvent(mouseInfo);
}

/**
 * @tc.name: HandleMouseEvent002
 * @tc.desc: Verify that the HandleMouseEvent interface calls normally when isDoubleClick_ is true.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleMouseEvent002, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. call HandleMouseEvent.
     * @tc.expected: Check pattern->isMousePressed_ is false.
     */
    pattern->isDoubleClick_ = true;
    pattern->isMousePressed_ = true;
    MouseAction action = MouseAction::RELEASE;
    MouseInfo mouseInfo;
    mouseInfo.SetButton(MouseButton::NONE_BUTTON);
    mouseInfo.SetAction(action);
    pattern->HandleMouseEvent(mouseInfo);
    EXPECT_FALSE(pattern->isMousePressed_);
}

/**
 * @tc.name: OnKeyEvent
 * @tc.desc: Test TextFiledPattern.OnKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnKeyEvent, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldModelNG and TextFieldLayoutProperty.
     * @tc.expected: Check it is not nullptr.
     */
    TextFieldModelNG textFieldModelInstance;
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    context->SetTextFieldManager(AceType::MakeRefPtr<TextFieldManagerNG>());
    pattern->paragraph_ = std::make_shared<RSParagraph>();
    pattern->imeAttached_ = true;

    KeyEvent event;
    EXPECT_FALSE(pattern->OnKeyEvent(event));
    event.action = KeyAction::DOWN;

    /**
     * @tc.steps: step2. test center keys.
     * @tc.expected: Check the return value.
     */
    KeyCode centerCodes[] = { KeyCode::KEY_ENTER, KeyCode::KEY_NUMPAD_ENTER, KeyCode::KEY_DPAD_CENTER };
    for (auto centerCode : centerCodes) {
        event.code = centerCode;
        EXPECT_TRUE(pattern->OnKeyEvent(event));
    }

    /**
     * @tc.steps: step3. test symbol keys.
     * @tc.expected: Check the return value.
     */
    std::chrono::milliseconds milliseconds(0);
    TimeStamp time(milliseconds);
    KeyEvent symbolKeyWithOnePressedCode(
        KeyCode::KEY_EQUALS, KeyAction::DOWN, { KeyCode::KEY_EQUALS }, 1, time, 0, 0, SourceType::KEYBOARD);
    EXPECT_TRUE(pattern->OnKeyEvent(symbolKeyWithOnePressedCode));

    KeyEvent symbolKeyWithMaxKeySize(KeyCode::KEY_EQUALS, KeyAction::DOWN,
        { KeyCode::KEY_SHIFT_LEFT, KeyCode::KEY_EQUALS }, 1, time, 0, 0, SourceType::KEYBOARD);
    EXPECT_TRUE(pattern->OnKeyEvent(symbolKeyWithMaxKeySize));

    /**
     * @tc.steps: step4. test direction keys.
     * @tc.expected: Check the return value.
     */
    KeyCode directionCodes[] = { KeyCode::KEY_DPAD_UP, KeyCode::KEY_DPAD_DOWN, KeyCode::KEY_DPAD_LEFT,
        KeyCode::KEY_DPAD_RIGHT };
    KeyCode shiftKeyCodes[] = { KeyCode::KEY_SHIFT_LEFT, KeyCode::KEY_SHIFT_RIGHT };
    for (auto directionKeyCode : directionCodes) {
        for (auto shiftCode : shiftKeyCodes) {
            KeyEvent directionKey(directionKeyCode, KeyAction::DOWN, { shiftCode, directionKeyCode }, 1, time, 0, 0,
                SourceType::KEYBOARD);
            EXPECT_TRUE(pattern->OnKeyEvent(directionKey));
        }
        KeyEvent directionKey(directionKeyCode, KeyAction::DOWN);
        EXPECT_TRUE(pattern->OnKeyEvent(directionKey));
    }
    event.code = KeyCode::KEY_9;
    EXPECT_TRUE(pattern->OnKeyEvent(event));

    /**
     * @tc.steps: step5. test letter keys.
     * @tc.expected: Check the return value.
     */
    KeyCode ctrlCodes[] = { KeyCode::KEY_CTRL_LEFT, KeyCode::KEY_CTRL_RIGHT };
    KeyCode letterCodes[] = { KeyCode::KEY_Z, KeyCode::KEY_A, KeyCode::KEY_C, KeyCode::KEY_V, KeyCode::KEY_X };
    // redo action
    for (auto ctrl : ctrlCodes) {
        for (auto shift : shiftKeyCodes) {
            KeyEvent letterKey(
                KeyCode::KEY_Z, KeyAction::DOWN, { ctrl, shift, KeyCode::KEY_Z }, 1, time, 0, 0, SourceType::KEYBOARD);
            EXPECT_FALSE(pattern->OnKeyEvent(letterKey));
        }
        KeyEvent letterKey(
            KeyCode::KEY_Y, KeyAction::DOWN, { ctrl, KeyCode::KEY_Y }, 1, time, 0, 0, SourceType::KEYBOARD);
        EXPECT_FALSE(pattern->OnKeyEvent(letterKey));
    }

    for (auto ctrl : ctrlCodes) {
        for (auto letter : letterCodes) {
            KeyEvent letterKey(letter, KeyAction::DOWN, { ctrl, letter }, 1, time, 0, 0, SourceType::KEYBOARD);
            EXPECT_FALSE(pattern->OnKeyEvent(letterKey));
        }
    }

    event.code = KeyCode::KEY_DEL;
    EXPECT_TRUE(pattern->OnKeyEvent(event));
    event.code = KeyCode::KEY_FORWARD_DEL;
    EXPECT_TRUE(pattern->OnKeyEvent(event));
    pattern->imeAttached_ = false;
}

/**
 * @tc.name: StopEditing001
 * @tc.desc: test StopEditing
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, StopEditing001, TestSize.Level2)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto textFieldController = AceType::MakeRefPtr<TextFieldController>();
    ASSERT_NE(textFieldController, nullptr);
    textFieldController->SetPattern(textFieldPattern);
    frameNode->GetOrCreateFocusHub()->currentFocus_ = false;
    textFieldPattern->StopEditing();
    frameNode->GetOrCreateFocusHub()->currentFocus_ = true;
    textFieldPattern->OnModifyDone();
    textFieldPattern->textEditingValue_.Reset();
    textFieldPattern->StopEditing();
    textFieldController->StopEditing();
    textFieldController->stopEditing_ = []() {};
    textFieldController->StopEditing();
    EXPECT_EQ(textFieldPattern->imeAttached_, false);
}

/**
 * @tc.name: TextFieldPatternOnTextInputScroll001
 * @tc.desc: Verify that the AddScrollEvent interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldPatternOnTextInputScroll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create the TextFieldPattern.
     * @tc.expected: step1. Check the TextFieldPattern success.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. Call the OnTextInputScroll.
     * @tc.expected: step2. Check the value set in OnTextInputScroll.
     */
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateMaxLines(2);
    pattern->OnTextInputScroll(0.0f);
    layoutProperty->UpdateMaxLines(1);
    pattern->selectOverlayProxy_ = AceType::MakeRefPtr<SelectOverlayProxy>(SELECT_OVERLAY_ID);
    pattern->textRect_.x_ = 10.0f;
    pattern->textRect_.width_ = 200.0f;
    pattern->contentRect_.x_ = 20.0f;
    pattern->contentRect_.width_ = 100.0f;
    pattern->OnTextInputScroll(-1000.0f);
    pattern->isSingleHandle_ = false;
    pattern->OnTextInputScroll(0.0f);
    pattern->isSingleHandle_ = true;
    pattern->OnTextInputScroll(0.0f);
    EXPECT_EQ(pattern->caretRect_.GetX(), -90.0f);
    EXPECT_EQ(pattern->textRect_.GetOffset(), OffsetF(pattern->currentOffset_, pattern->textRect_.GetY()));

    // first handle and second handle are in contentRect's region.
    pattern->isSingleHandle_ = false;
    pattern->contentRect_.SetTop(20.0f);
    pattern->contentRect_.SetLeft(24.0f);
    pattern->contentRect_.SetWidth(500.0f);
    pattern->contentRect_.SetHeight(30.0f);
    pattern->textRect_.SetLeft(-50.0f);
    pattern->textRect_.SetWidth(800.0f);
    pattern->textSelector_.firstHandleOffset_.SetX(260.0f);
    pattern->textSelector_.firstHandleOffset_.SetY(40.0f);
    pattern->textSelector_.secondHandleOffset_.SetX(300.0f);
    pattern->textSelector_.secondHandleOffset_.SetY(40.0f);
    pattern->parentGlobalOffset_ = OffsetF(60.0f, 8.0f);
    pattern->OnTextInputScroll(-1.0f);
    EXPECT_EQ(pattern->textSelector_.firstHandleOffset_.GetX(), 259.0f);

    // select overlay is not on.
    pattern->selectOverlayProxy_ = AceType::MakeRefPtr<SelectOverlayProxy>(-1);
    pattern->textRect_.SetLeft(-50.0f);
    pattern->OnTextInputScroll(-1.0f);
    EXPECT_EQ(pattern->textRect_.GetX(), -51.0f);
}

/**
 * @tc.name: FitInSafeArea
 * @tc.desc: Test TextArea.FitInSafeArea
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, FitInSafeArea, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateMaxLines(2);

    auto pipeline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    AceType::DynamicCast<MockPipelineBase>(pipeline)->SetRootSize(1000, 1000);
    pipeline->GetSafeAreaManager()->UpdateKeyboardSafeArea(500.0f);
    pattern->caretUpdateType_ = CaretUpdateType::INPUT;

    // test caret inside safeArea
    pattern->contentRect_ = RectF { 0.0f, 0.0f, 1000.0f, 1000.0f };
    const RectF CARET_RECT_SAFE { 0.0f, 0.0f, 100.0f, 100.0f };
    pattern->caretRect_ = CARET_RECT_SAFE;
    auto dy = pattern->AdjustTextAreaOffsetY();
    EXPECT_EQ(dy, 0.0f);
    EXPECT_EQ(pattern->caretRect_, CARET_RECT_SAFE);

    // test caret outside safeArea
    const RectF CARE_RECT_DANGEROUS { 600.0f, 600.0f, 100.0f, 100.0f };
    pattern->caretRect_ = CARE_RECT_DANGEROUS;
    dy = pattern->AdjustTextAreaOffsetY();
    // caretBottom is 200 below safeAreaBottom
    EXPECT_EQ(dy, -200.0f);
    EXPECT_EQ(pattern->caretRect_, CARE_RECT_DANGEROUS - OffsetF(0.0f, 200.0f));

    // test caret when keyboard is down
    pipeline->GetSafeAreaManager()->UpdateKeyboardSafeArea(0.0f);
    pattern->caretRect_ = CARE_RECT_DANGEROUS;
    dy = pattern->AdjustTextAreaOffsetY();
    EXPECT_EQ(dy, 195.0f);
    EXPECT_EQ(pattern->caretRect_.GetX(), CARE_RECT_DANGEROUS.GetX());
    int32_t charPosition[3] = {-1, 0, 2};
    auto content = pattern->CreateDisplayText(TEXT_CONTENT, charPosition[1], true);;
    for (int i = 0; i < 3; i++) {
        content = pattern->CreateDisplayText(TEXT_CONTENT, charPosition[i], true);
        content = pattern->CreateDisplayText(TEXT_CONTENT, charPosition[i], false);
        content = pattern->CreateDisplayText(EMPTY_TEXT_VALUE, charPosition[i], true);
        content = pattern->CreateDisplayText(EMPTY_TEXT_VALUE, charPosition[i], false);
    }
}

/**
 * @tc.name: GetTextOrPlaceHolderFontSize
 * @tc.desc: test GetTextOrPlaceHolderFontSize
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetTextOrPlaceHolderFontSize, TestSize.Level2)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto size = textFieldPattern->GetTextOrPlaceHolderFontSize();
    EXPECT_EQ(size, 0.0f);
    const Dimension fontSize = Dimension(5.0);
    layoutProperty->UpdateFontSize(fontSize);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextFieldTheme>()));
    size = textFieldPattern->GetTextOrPlaceHolderFontSize();
    EXPECT_EQ(size, 5.0f);
}

/**
 * @tc.name: UpdateSelectorByPosition
 * @tc.desc: Verify that the UpdateSelectorByPosition interface calls normally and exits without exception.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateSelectorByPosition001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFieldPattern.
     */
    auto frameNode = CreatTextFieldNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->paragraph_ = std::make_shared<RSParagraph>();
    /*
     * @tc.steps: step2. call UpdateSelectorByPosition function.
     * @tc.expected: The UpdateSelectorByPosition function will exit without exception.
     */
    pattern->textSelector_.baseOffset = 0;
    pattern->textSelector_.destinationOffset = 5;
    pattern->UpdateSelectorByPosition(10);
    EXPECT_EQ(pattern->textSelector_.GetStart(), 10);
    EXPECT_EQ(pattern->textSelector_.GetEnd(), 11);
}

/**
 * @tc.name: OnScrollEndCallback001
 * @tc.desc: test textfield OnScrollEndCallback function.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnScrollEndCallback001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create the TextFieldPattern.
     * @tc.expected: step1. Check the TextFieldPattern success.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectOverlayProxy_ = AceType::MakeRefPtr<SelectOverlayProxy>(SELECT_OVERLAY_ID);

    /**
     * @tc.steps: step2. Call the OnScrollEndCallback.
     * @tc.expected: the OnScrollEndCallback function called success without expection.
     */
    bool originalIsMenuShow[2] = { true, false };
    for (int i = 0; i < 2; i++) {
        pattern->originalIsMenuShow_ = originalIsMenuShow[i];
        EXPECT_EQ(pattern->originalIsMenuShow_, originalIsMenuShow[i]);
        pattern->OnScrollEndCallback();
        EXPECT_NE(pattern->GetSelectOverlay(), nullptr);
    }
}

/**
 * @tc.name: OnHandleMove001
 * @tc.desc: test textfield OnHandleMove001 function.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnHandleMove001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create the TextFieldPattern.
     * @tc.expected: step1. Check the TextFieldPattern success.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    /**
     * @tc.steps: step2. Call the OnHandleMove.
     * @tc.expected: caretUpdateType_ is HANDLE_MOVE.
     */
    pattern->selectOverlayProxy_ = AceType::MakeRefPtr<SelectOverlayProxy>(SELECT_OVERLAY_ID);
    pattern->UpdateEditingValue(TEXT_VALUE, 0);
    pattern->OnHandleMove(RectF(110.0f, 110.0f, 10.0f, 10.0f), false);
    EXPECT_EQ(pattern->caretUpdateType_, CaretUpdateType::HANDLE_MOVE);
}

/**
 * @tc.name: OnHandleMoveDone001
 * @tc.desc: test textfield OnHandleMoveDone001 function.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnHandleMoveDone001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create the TextFieldPattern.
     * @tc.expected: step1. Check the TextFieldPattern success.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    /**
     * @tc.steps: step2. Call the OnHandleMoveDone.
     * @tc.expected: caretUpdateType_ is HANDLE_MOVE_DONE.
     */
    pattern->selectOverlayProxy_ = AceType::MakeRefPtr<SelectOverlayProxy>(SELECT_OVERLAY_ID);
    pattern->isSingleHandle_ = true;
    pattern->OnHandleMoveDone(RectF(0.0f, 0.0f, 10.0f, 10.0f), false);
    pattern->isSingleHandle_ = false;
    pattern->OnHandleMoveDone(RectF(0.0f, 0.0f, 10.0f, 10.0f), true);
    EXPECT_EQ(pattern->caretUpdateType_, CaretUpdateType::HANDLE_MOVE_DONE);
}

/**
 * @tc.name: TextFieldPatternSaveInlineState001
 * @tc.desc: test SaveInlineState
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldPatternSaveInlineState001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto pipeline = frameNode->GetContext();
    ASSERT_NE(pipeline, nullptr);
    auto themeManager = pipeline->GetThemeManager();
    ASSERT_NE(themeManager, nullptr);
    auto textFieldTheme = themeManager->GetTheme<TextFieldTheme>();
    ASSERT_NE(textFieldTheme, nullptr);

    textFieldPattern->SaveInlineStates();
    EXPECT_FALSE(textFieldPattern->GetTextInputFlag());
    auto textColor = layoutProperty->GetTextColorValue(textFieldTheme->GetTextColor());
    EXPECT_EQ(textFieldPattern->inlineState_.textColor, textColor);
}

/**
 * @tc.name: TextFieldPatternApplyInlineState001
 * @tc.desc: test ApplyInlineState
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldPatternApplyInlineState001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto pipeline = frameNode->GetContext();
    ASSERT_NE(pipeline, nullptr);
    auto themeManager = pipeline->GetThemeManager();
    ASSERT_NE(themeManager, nullptr);
    auto textFieldTheme = themeManager->GetTheme<TextFieldTheme>();
    ASSERT_NE(textFieldTheme, nullptr);
    textFieldPattern->ApplyInlineStates(true);
    auto textColor = layoutProperty->GetTextColorValue(textFieldTheme->GetTextColor());
    EXPECT_EQ(textColor, textFieldTheme->GetInlineTextColor());
}

/**
 * @tc.name: TextFieldPatternSaveInlineState002
 * @tc.desc: test SaveInlineState
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldPatternSaveInlineState002, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    auto pipeline = frameNode->GetContext();
    ASSERT_NE(pipeline, nullptr);
    auto themeManager = pipeline->GetThemeManager();
    ASSERT_NE(themeManager, nullptr);
    auto textFieldTheme = themeManager->GetTheme<TextFieldTheme>();
    ASSERT_NE(textFieldTheme, nullptr);

    textFieldPattern->SaveInlineStates();
    EXPECT_FALSE(textFieldPattern->GetTextInputFlag());
    auto bgColor = renderContext->GetBackgroundColor().value_or(textFieldTheme->GetBgColor());
    EXPECT_EQ(textFieldPattern->inlineState_.bgColor, bgColor);
}

/**
 * @tc.name: TextFieldPatternRestorePreInlineStates001
 * @tc.desc: test RestorePreInlineStates
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldPatternRestorePreInlineStates001, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    textFieldPattern->RestorePreInlineStates();
    textFieldPattern->inlineState_.textColor = Color::RED;
    textFieldPattern->inlineState_.bgColor = Color::WHITE;
    layoutProperty->UpdateTextColor(textFieldPattern->inlineState_.textColor);
    renderContext->UpdateBackgroundColor(textFieldPattern->inlineState_.bgColor);
    textFieldPattern->RestorePreInlineStates();
    auto borderWithProperty = *(layoutProperty->GetBorderWidthProperty());
    ASSERT_TRUE(borderWithProperty.bottomDimen.has_value());
    EXPECT_EQ(borderWithProperty.bottomDimen.value().Value(), 2.0);
    EXPECT_EQ(layoutProperty->GetTextColor(), textFieldPattern->inlineState_.textColor);
    EXPECT_EQ(renderContext->GetBackgroundColor(), textFieldPattern->inlineState_.bgColor);

    layoutProperty->UpdateUserDefinedIdealSize(CalcSize(CalcLength(DEVICE_WIDTH), CalcLength(DEVICE_HEIGHT)));
    auto idealSize = layoutProperty->GetCalcLayoutConstraint()->selfIdealSize;
    EXPECT_EQ(idealSize->Width().value(), CalcLength(DEVICE_WIDTH));
    EXPECT_EQ(idealSize->Height().value(), CalcLength(DEVICE_HEIGHT));
}

/**
 * @tc.name: TextFieldPatternApplyInlineState002
 * @tc.desc: test ApplyInlineState
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFieldPatternApplyInlineState002, TestSize.Level1)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto pipeline = frameNode->GetContext();
    ASSERT_NE(pipeline, nullptr);
    auto themeManager = pipeline->GetThemeManager();
    ASSERT_NE(themeManager, nullptr);
    auto textFieldTheme = themeManager->GetTheme<TextFieldTheme>();
    ASSERT_NE(textFieldTheme, nullptr);
    textFieldPattern->ApplyInlineStates(true);
    renderContext->UpdateBackgroundColor(textFieldTheme->GetInlineBgColor());
    EXPECT_EQ(renderContext->GetBackgroundColor(), textFieldTheme->GetInlineBgColor());
}

/**
 * @tc.name: BeforeCreateLayoutWrapper001
 * @tc.desc: test BeforeCreateLayoutWrapper
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, BeforeCreateLayoutWrapper001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    frameNode->GetOrCreateFocusHub()->currentFocus_ = true;

    /**
     * @tc.steps: step2. set clipboard avoid nullptr and call BeforeCreateLayoutWrapper.
     * @tc.expected: Check it is not nullptr.
     */
    auto pipeline = MockPipelineBase::GetCurrent();
    auto clipboard = ClipboardProxy::GetInstance()->GetClipboard(pipeline->GetTaskExecutor());
    textFieldPattern->clipboard_ = clipboard;

    CaretUpdateType exptectFalseTypes[] = { CaretUpdateType::INPUT, CaretUpdateType::PRESSED,
        CaretUpdateType::LONG_PRESSED, CaretUpdateType::EVENT, CaretUpdateType::DEL, CaretUpdateType::ICON_PRESSED,
        CaretUpdateType::RIGHT_CLICK, CaretUpdateType::HANDLE_MOVE };
    for (auto caretType : exptectFalseTypes) {
        textFieldPattern->caretUpdateType_ = caretType;
        textFieldPattern->BeforeCreateLayoutWrapper();
        EXPECT_FALSE(textFieldPattern->UpdateCaretRect());
    }

    EXPECT_TRUE(textFieldPattern->CaretPositionCloseToTouchPosition());
    EXPECT_FALSE(textFieldPattern->SelectOverlayIsOn());
    EXPECT_TRUE(textFieldPattern->caretUpdateType_ != CaretUpdateType::LONG_PRESSED);
    EXPECT_FALSE(textFieldPattern->isMousePressed_);
    textFieldPattern->UpdateCaretByPressOrLongPress();
    textFieldPattern->UpdateCaretByRightClick();

    textFieldPattern->caretUpdateType_ = CaretUpdateType::EVENT;
    textFieldPattern->isMousePressed_ = true;
    textFieldPattern->BeforeCreateLayoutWrapper();
    textFieldPattern->caretUpdateType_ = CaretUpdateType::PRESSED;
    EXPECT_FALSE(textFieldPattern->UpdateCaretPositionByMouseMovement());
}

/**
 * @tc.name: BeforeCreateLayoutWrapper002
 * @tc.desc: test BeforeCreateLayoutWrapper
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, BeforeCreateLayoutWrapper002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    frameNode->GetOrCreateFocusHub()->currentFocus_ = true;

    /**
     * @tc.steps: step2. set clipboard avoid nullptr and call BeforeCreateLayoutWrapper.
     * @tc.expected: Check it is not nullptr.
     */
    auto pipeline = MockPipelineBase::GetCurrent();
    auto clipboard = ClipboardProxy::GetInstance()->GetClipboard(pipeline->GetTaskExecutor());
    textFieldPattern->clipboard_ = clipboard;

    textFieldPattern->caretUpdateType_ = CaretUpdateType::DOUBLE_CLICK;
    textFieldPattern->BeforeCreateLayoutWrapper();
    EXPECT_FALSE(textFieldPattern->UpdateCaretRect());
    EXPECT_FALSE(textFieldPattern->isDoubleClick_);

    textFieldPattern->caretUpdateType_ = CaretUpdateType::EVENT;
    textFieldPattern->isMousePressed_ = true;
    textFieldPattern->isDoubleClick_ = true;
    textFieldPattern->BeforeCreateLayoutWrapper();
    textFieldPattern->caretUpdateType_ = CaretUpdateType::PRESSED;
    EXPECT_FALSE(textFieldPattern->UpdateCaretPositionByMouseMovement());
    EXPECT_FALSE(textFieldPattern->isDoubleClick_);
}

/**
 * @tc.name: UpdateCaretOffsetByEvent
 * @tc.desc: test UpdateCaretOffsetByEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateCaretOffsetByEvent, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    frameNode->GetOrCreateFocusHub()->currentFocus_ = true;

    /**
     * @tc.steps: step2. set clipboard avoid nullptr and call UpdateCaretOffsetByEvent.
     * @tc.expected: Check it is not nullptr.
     */
    textFieldPattern->UpdateCaretOffsetByEvent();
    EXPECT_FALSE(textFieldPattern->textEditingValue_.text.empty());

    textFieldPattern->textEditingValue_.Reset();
    textFieldPattern->textEditingValue_.text = TEXT_VALUE;
    textFieldPattern->UpdateCaretOffsetByEvent();
    EXPECT_TRUE(textFieldPattern->isMousePressed_);
    textFieldPattern->textEditingValue_.caretPosition = 10;

    textFieldPattern->isMousePressed_ = true;
    textFieldPattern->UpdateCaretOffsetByEvent();
    EXPECT_FALSE(textFieldPattern->IsSelected());

    frameNode->GetOrCreateFocusHub()->currentFocus_ = true;
    textFieldPattern->textEditingValue_.text = TEXT_VALUE;
    textFieldPattern->isMousePressed_ = false;
    textFieldPattern->UpdateCaretOffsetByEvent();

    textFieldPattern->textEditingValue_.text = "";
    textFieldPattern->UpdateCaretOffsetByEvent();
}

/**
 * @tc.name: CalcCursorOffsetByPosition
 * @tc.desc: test CalcCursorOffsetByPosition
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CalcCursorOffsetByPosition, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(textFieldPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    frameNode->GetOrCreateFocusHub()->currentFocus_ = true;

    /**
     * @tc.steps: step2. set clipboard avoid nullptr and call CalcCursorOffsetByPosition.
     * @tc.expected: Check it is not nullptr.
     */
    textFieldPattern->textEditingValue_.caretPosition = 10;
    textFieldPattern->textEditingValue_.text = TEXT_VALUE;
    auto position = textFieldPattern->textEditingValue_.caretPosition;
    textFieldPattern->CalcCursorOffsetByPosition(position, false);
    textFieldPattern->CalcCursorOffsetByPosition(position, true);

    /**
     * @tc.steps: step3. set contentSize and textSize. contentWidth less than textWidth.
     * @tc.expected: Check it is not nullptr.
     */
    SizeF textSize(730.0, 160.0);
    SizeF contentSize(720.0, 150.0);
    textFieldPattern->contentRect_.Reset();
    textFieldPattern->textRect_.Reset();
    textFieldPattern->caretRect_.Reset();
    textFieldPattern->contentRect_.SetSize(contentSize);
    textFieldPattern->textRect_.SetSize(textSize);
    layoutProperty->UpdateMaxLines(1);

    EXPECT_FALSE(textFieldPattern->AdjustTextRectOffsetX());
    layoutProperty->UpdateMaxLines(2);
    EXPECT_TRUE(textFieldPattern->IsTextArea());
}

/**
 * @tc.name: GetDragUpperLeftCoordinates
 * @tc.desc: test GetDragUpperLeftCoordinates
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetDragUpperLeftCoordinates, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    layoutProperty_->UpdateMaxLines(1);

    /**
     * @tc.steps: step2. call GetDragUpperLeftCoordinates.
     * @tc.expected: Check result.
     */
    pattern_->textBoxes_.clear();
    EXPECT_EQ(pattern_->GetDragUpperLeftCoordinates(), OffsetF(0.0f, 0.0f));

    RSTextRect front = {};
    front.rect.top_ = 0;
    front.rect.left_ = 0;
    pattern_->textBoxes_.push_back(front);
    RSTextRect back = {};
    back.rect.top_ = 10;
    pattern_->textBoxes_.push_back(back);
    EXPECT_EQ(pattern_->GetDragUpperLeftCoordinates(), OffsetF(0.0f, 0.0f));

    pattern_->textBoxes_.clear();
    front.rect.top_ = -100;
    front.rect.left_ = -100;
    pattern_->textBoxes_.push_back(front);
    back.rect.top_ = -100;
    pattern_->textBoxes_.push_back(back);
    EXPECT_EQ(pattern_->GetDragUpperLeftCoordinates(), OffsetF(0.0f, 0.0f));
}

/**
 * @tc.name: ProcessOverlay
 * @tc.desc: test ProcessOverlay
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, ProcessOverlay, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call ProcessOverlay.
     * @tc.expected: Check result.
     */
    pattern_->caretUpdateType_ = CaretUpdateType::LONG_PRESSED;
    pattern_->UpdateEditingValue("", 0);
    pattern_->isSingleHandle_ = false;
    pattern_->ProcessOverlay(false);
    EXPECT_TRUE(pattern_->isSingleHandle_) << "create single handle";

    pattern_->UpdateEditingValue("X", 0);
    pattern_->textRect_.SetLeft(50);
    pattern_->lastTouchOffset_.SetX(25);
    pattern_->isSingleHandle_ = false;
    pattern_->ProcessOverlay(false);
    EXPECT_TRUE(pattern_->isSingleHandle_) << "create single handle";
    EXPECT_EQ(pattern_->textSelector_.GetStart(), 0) << "update selection to 0";
    EXPECT_EQ(pattern_->textSelector_.GetEnd(), 0) << "update selection to 0";

    pattern_->UpdateEditingValue("X", 1);
    pattern_->textRect_.SetLeft(25);
    pattern_->lastTouchOffset_.SetX(50);
    pattern_->isSingleHandle_ = false;
    pattern_->ProcessOverlay(false);
    EXPECT_TRUE(pattern_->isSingleHandle_) << "create single handle";
    EXPECT_EQ(pattern_->textSelector_.GetStart(), 1) << "update selection to caretPosition";
    EXPECT_EQ(pattern_->textSelector_.GetEnd(), 1) << "update selection to caretPosition";

    pattern_->UpdateEditingValue("XXX", 1);
    pattern_->isSingleHandle_ = false;
    pattern_->textRect_.SetLeft(25);
    pattern_->lastTouchOffset_.SetX(25);
    pattern_->ProcessOverlay(false);
    EXPECT_FALSE(pattern_->isSingleHandle_) << "create handles";
    EXPECT_EQ(pattern_->textSelector_.GetStart(), 1);
    EXPECT_EQ(pattern_->textSelector_.GetEnd(), 2);
}

/**
 * @tc.name: UpdateRectByAlignment
 * @tc.desc: test UpdateRectByAlignment
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, UpdateRectByAlignment, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call UpdateRectByAlignment.
     * @tc.expected: Check result.
     */
    pattern_->contentRect_.SetHeight(25);
    pattern_->frameRect_.SetHeight(200);
    RectF rect(0, 0, 100, 100);
    pattern_->UpdateRectByAlignment(rect);
    EXPECT_EQ(rect.Top(), 0);

    struct TestItem {
        Alignment alignment;
        float expectedTop;
        std::string message;
    };
    pattern_->contentRect_.SetHeight(150);
    layoutProperty_->positionProperty_ = std::make_unique<PositionProperty>();
    std::vector<TestItem> testItems = {
        { .alignment = Alignment::CENTER_LEFT, .expectedTop = 50, .message = "Alignment::CENTER_LEFT" },
        { .alignment = Alignment::CENTER, .expectedTop = 50, .message = "Alignment::CENTER" },
        { .alignment = Alignment::CENTER_RIGHT, .expectedTop = 50, .message = "Alignment::CENTER_RIGHT" },
        { .alignment = Alignment::BOTTOM_LEFT, .expectedTop = 100, .message = "Alignment::BOTTOM_LEFT" },
        { .alignment = Alignment::BOTTOM_CENTER, .expectedTop = 100, .message = "Alignment::BOTTOM_CENTER" },
        { .alignment = Alignment::BOTTOM_RIGHT, .expectedTop = 100, .message = "Alignment::BOTTOM_RIGHT" },
        { .alignment = Alignment::TOP_LEFT, .expectedTop = 0, .message = "Alignment::TOP_LEFT" },
        { .alignment = Alignment::TOP_CENTER, .expectedTop = 0, .message = "Alignment::TOP_CENTER" },
        { .alignment = Alignment::TOP_RIGHT, .expectedTop = 0, .message = "Alignment::TOP_RIGHT" },
        { .alignment = Alignment(2, 2), .expectedTop = 0, .message = "Alignment(2, 2)" },
    };
    for (auto testItem : testItems) {
        layoutProperty_->GetPositionProperty()->UpdateAlignment(testItem.alignment);
        pattern_->UpdateRectByAlignment(rect);
        EXPECT_EQ(rect.Top(), testItem.expectedTop) << testItem.message;
    }
}

/**
 * @tc.name: TextAreaInputRectUpdate
 * @tc.desc: test TextAreaInputRectUpdate
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextAreaInputRectUpdate, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call TextAreaInputRectUpdate.
     * @tc.expected: Check result.
     */
    RectF rect (0, 0, 100, 100);
    layoutProperty_->UpdateMaxLines(1);
    pattern_->TextAreaInputRectUpdate(rect);
    EXPECT_EQ(rect.Left(), 0);
    EXPECT_EQ(rect.Width(), 100);

    layoutProperty_->UpdateMaxLines(10);
    pattern_->UpdateEditingValue("", 0);
    pattern_->TextAreaInputRectUpdate(rect);
    EXPECT_EQ(rect.Left(), 0);
    EXPECT_EQ(rect.Width(), 100);

    layoutProperty_->UpdateMaxLines(10);
    pattern_->UpdateEditingValue("XXX", 0);

    struct TestItem {
        TextAlign align;
        float contentWidth;
        float expectedLeft;
        float expectedWidth;
        float textIsEmptyRectExpectedLeft;
    };
    std::vector<TestItem> testItems = {
        { TextAlign::START, 10, 0, 1, 0 },
        { TextAlign::START, 0, 0, 10, 0 },
        { TextAlign::CENTER, 10, 4.5, 1, 5 },
        {TextAlign::CENTER, 0, 0, 10, 0 },
        { TextAlign::END, 10, 7.5, 1, 8.5 },
        { TextAlign::END, 0, 0, 10, -1.5 },
        { TextAlign::LEFT, 0, 0, 10, 0 },
    };

    for (auto testItem : testItems) {
        rect.SetLeft(0);
        rect.SetWidth(10);
        layoutProperty_->UpdateTextAlign(testItem.align);
        pattern_->contentRect_.SetWidth(testItem.contentWidth);
        pattern_->TextAreaInputRectUpdate(rect);
        EXPECT_EQ(rect.Left(), testItem.expectedLeft);
        EXPECT_EQ(rect.Width(), testItem.expectedWidth);
        pattern_->TextIsEmptyRect(rect);
    }
    pattern_->UpdateEditingValue("", 0);
    for (auto testItem : testItems) {
        rect.SetLeft(0);
        rect.SetWidth(10);
        layoutProperty_->UpdateTextAlign(testItem.align);
        pattern_->contentRect_.SetWidth(testItem.contentWidth);
        pattern_->TextIsEmptyRect(rect);
        EXPECT_EQ(rect.Left(), testItem.textIsEmptyRectExpectedLeft);
    }
}

/**
 * @tc.name: GetNakedCharPosition
 * @tc.desc: test GetNakedCharPosition
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetNakedCharPosition, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call GetNakedCharPosition.
     * @tc.expected: Check result.
     */
    layoutProperty_->UpdateMaxLines(2);
    EXPECT_EQ(pattern_->GetNakedCharPosition(), -1);
    layoutProperty_->UpdateMaxLines(1);
    layoutProperty_->UpdateTextInputType(TextInputType::TEXT);
    EXPECT_EQ(pattern_->GetNakedCharPosition(), -1);
    layoutProperty_->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    pattern_->obscureTickCountDown_ = 0;
    EXPECT_EQ(pattern_->GetNakedCharPosition(), -1);
    pattern_->obscureTickCountDown_ = 1;
    pattern_->textObscured_ = false;
    EXPECT_EQ(pattern_->GetNakedCharPosition(), -1);
    pattern_->textObscured_ = true;
    pattern_->UpdateEditingValue("", 0);
    EXPECT_EQ(pattern_->GetNakedCharPosition(), -1);
    layoutProperty_->UpdateValue("xxx");
    pattern_->nakedCharPosition_ = 1;
    EXPECT_EQ(pattern_->GetNakedCharPosition(), 1);
}

/**
 * @tc.name: OnBackPressed
 * @tc.desc: test OnBackPressed
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnBackPressed, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call OnBackPressed.
     * @tc.expected: Check result.
     */
    struct TestItem {
        bool attached;
        bool imeShow;
        bool customKeyboardAttached;
        bool expected;
        std::string message;
    };
    std::vector<TestItem> testItems = {
        { false, false, false, false, "A" },
        { false, false, true, true, "B" },
        { false, true, false, false, "C" },
        { false, true, true, true, "D" },
        { true, false, false, false, "E" },
        { true, false, true, true, "F" },
        { true, true, false, true, "G" },
        { true, true, true, true, "H" },
    };

    for (auto testItem : testItems) {
        pattern_->imeAttached_ = testItem.attached;
        pattern_->imeShown_ = testItem.imeShow;
        pattern_->isCustomKeyboardAttached_ = testItem.customKeyboardAttached;
        pattern_->connection_ = AceType::MakeRefPtr<MockTextInputConnection>();
        EXPECT_EQ(pattern_->OnBackPressed(), testItem.expected) << testItem.message;
    }
    pattern_->connection_ = AceType::MakeRefPtr<MockTextInputConnection>();
}

/**
 * @tc.name: HandleSelection001
 * @tc.desc: test HandleSelection
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleSelection001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call HandleSelectionEnd.
     * @tc.expected: Check result.
     */
    auto switchSelectMode = [pattern = pattern_](int32_t start, int32_t end) {
        pattern->selectionMode_ = SelectionMode::SELECT;
        pattern->textSelector_.baseOffset = start;
        pattern->textSelector_.destinationOffset = end;
    };

    auto switchNormalMode = [pattern = pattern_]() {
        pattern->selectionMode_ = SelectionMode::NONE;
    };

    pattern_->UpdateEditingValue("ABCD", 4);
    pattern_->HandleSelectionEnd();
    EXPECT_EQ(pattern_->selectionMode_, SelectionMode::NONE);

    pattern_->UpdateEditingValue("ABCD", 3);
    switchNormalMode();
    pattern_->HandleSelectionEnd();
    EXPECT_EQ(pattern_->selectionMode_, SelectionMode::SELECT);

    pattern_->UpdateEditingValue("ABCD", 1);
    switchSelectMode(0, 2);
    pattern_->HandleSelectionEnd();
    EXPECT_EQ(pattern_->textSelector_.destinationOffset, 4);

    pattern_->UpdateEditingValue("ABCD", 1);
    switchSelectMode(4, 2);
    pattern_->HandleSelectionEnd();
    EXPECT_EQ(pattern_->selectionMode_, SelectionMode::NONE);
}

/**
 * @tc.name: HandleSelection002
 * @tc.desc: test HandleSelection
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleSelection002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call HandleSelectionEnd.
     * @tc.expected: Check result.
     */
    auto switchSelectMode = [pattern = pattern_](int32_t start, int32_t end) {
        pattern->selectionMode_ = SelectionMode::SELECT;
        pattern->textSelector_.baseOffset = start;
        pattern->textSelector_.destinationOffset = end;
    };

    auto switchNormalMode = [pattern = pattern_]() {
        pattern->selectionMode_ = SelectionMode::NONE;
    };

    /**
     * @tc.steps: step1. call HandleSelectionLineEnd.
     * @tc.expected: Check result.
     */
    pattern_->UpdateEditingValue("ABCD", 4);
    pattern_->HandleSelectionLineEnd();

    pattern_->UpdateEditingValue("ABCD", 3);
    switchNormalMode();
    pattern_->HandleSelectionLineEnd();
    EXPECT_EQ(pattern_->selectionMode_, SelectionMode::SELECT);

    pattern_->UpdateEditingValue("ABCD", 3);
    switchSelectMode(0, 2);
    pattern_->HandleSelectionLineEnd();
    EXPECT_EQ(pattern_->textSelector_.destinationOffset, 4);

    pattern_->UpdateEditingValue("ABCD", 1);
    switchSelectMode(4, 2);
    pattern_->HandleSelectionLineEnd();
    EXPECT_EQ(pattern_->selectionMode_, SelectionMode::NONE);

    /**
     * @tc.steps: step2. call HandleSelectionRightWord.
     * @tc.expected: Check result.
     */
    pattern_->UpdateEditingValue("ABCD", 4);
    pattern_->HandleSelectionRightWord();

    pattern_->UpdateEditingValue("ABCD", 3);
    switchNormalMode();
    pattern_->HandleSelectionRightWord();
    EXPECT_EQ(pattern_->selectionMode_, SelectionMode::SELECT);

    pattern_->UpdateEditingValue("ABCD", 3);
    switchSelectMode(0, 4);
    pattern_->HandleSelectionRightWord();
    EXPECT_EQ(pattern_->textSelector_.destinationOffset, 4);

    pattern_->UpdateEditingValue("ABCD", 3);
    switchSelectMode(4, 2);
    pattern_->HandleSelectionRightWord();
    EXPECT_EQ(pattern_->selectionMode_, SelectionMode::NONE);
}

/**
 * @tc.name: HandleSelection003
 * @tc.desc: test HandleSelection
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleSelection003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call HandleSelectionEnd.
     * @tc.expected: Check result.
     */
    auto switchSelectMode = [pattern = pattern_](int32_t start, int32_t end) {
        pattern->selectionMode_ = SelectionMode::SELECT;
        pattern->textSelector_.baseOffset = start;
        pattern->textSelector_.destinationOffset = end;
    };

    auto switchNormalMode = [pattern = pattern_]() {
        pattern->selectionMode_ = SelectionMode::NONE;
    };

    /**
     * @tc.steps: step3. call HandleSelectionHome.
     * @tc.expected: Check result.
     */
    pattern_->UpdateEditingValue("ABCD", 0);
    pattern_->HandleSelectionHome();

    pattern_->UpdateEditingValue("ABCD", 1);
    switchNormalMode();
    pattern_->HandleSelectionHome();
    EXPECT_EQ(pattern_->selectionMode_, SelectionMode::SELECT);
    EXPECT_EQ(pattern_->textSelector_.destinationOffset, 0);

    pattern_->UpdateEditingValue("ABCD", 1);
    switchSelectMode(1, 3);
    pattern_->HandleSelectionHome();
    EXPECT_EQ(pattern_->textSelector_.destinationOffset, 0);

    pattern_->UpdateEditingValue("ABCD", 1);
    switchSelectMode(0, 3);
    pattern_->HandleSelectionHome();
    EXPECT_EQ(pattern_->textSelector_.destinationOffset, 0);
    EXPECT_EQ(pattern_->selectionMode_, SelectionMode::NONE);
}

/**
 * @tc.name: HandleSelection004
 * @tc.desc: test HandleSelection
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleSelection004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call HandleSelectionEnd.
     * @tc.expected: Check result.
     */
    auto switchSelectMode = [pattern = pattern_](int32_t start, int32_t end) {
        pattern->selectionMode_ = SelectionMode::SELECT;
        pattern->textSelector_.baseOffset = start;
        pattern->textSelector_.destinationOffset = end;
    };

    auto switchNormalMode = [pattern = pattern_]() {
        pattern->selectionMode_ = SelectionMode::NONE;
    };

     /**
     * @tc.steps: step3. call HandleSelectionLineBegin.
     * @tc.expected: Check result.
     */
    pattern_->UpdateEditingValue("ABCD", 0);
    pattern_->HandleSelectionLineBegin();

    pattern_->UpdateEditingValue("ABCD", 1);
    switchNormalMode();
    pattern_->HandleSelectionLineBegin();
    EXPECT_EQ(pattern_->selectionMode_, SelectionMode::SELECT);
    EXPECT_EQ(pattern_->textSelector_.destinationOffset, 0);

    pattern_->UpdateEditingValue("ABCD", 1);
    switchSelectMode(1, 3);
    pattern_->HandleSelectionLineBegin();
    EXPECT_EQ(pattern_->textSelector_.destinationOffset, 0);

    pattern_->UpdateEditingValue("ABCD", 1);
    switchSelectMode(0, 3);
    pattern_->HandleSelectionLineBegin();
    EXPECT_EQ(pattern_->textSelector_.destinationOffset, 0);
    EXPECT_EQ(pattern_->selectionMode_, SelectionMode::NONE);

    /**
     * @tc.steps: step4. call HandleSelectionLeftWord.
     * @tc.expected: Check result.
     */
    pattern_->UpdateEditingValue("ABCD", 0);
    pattern_->HandleSelectionLeftWord();

    pattern_->UpdateEditingValue("ABCD", 1);
    switchNormalMode();
    pattern_->HandleSelectionLeftWord();
    EXPECT_EQ(pattern_->selectionMode_, SelectionMode::SELECT);
    EXPECT_EQ(pattern_->textSelector_.destinationOffset, 0);

    pattern_->UpdateEditingValue("ABCD", 1);
    switchSelectMode(1, 3);
    pattern_->HandleSelectionLeftWord();
    EXPECT_EQ(pattern_->textSelector_.destinationOffset, 0);

    pattern_->UpdateEditingValue("ABCD", 1);
    switchSelectMode(0, 3);
    pattern_->HandleSelectionLeftWord();
    EXPECT_EQ(pattern_->textSelector_.destinationOffset, 0);
    EXPECT_EQ(pattern_->selectionMode_, SelectionMode::NONE);
}

/**
 * @tc.name: GetTextOfCursor
 * @tc.desc: test GetRightTextOfCursor and GetLeftTextOfCursor
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, GetTextOfCursor, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call GetRightTextOfCursor.
     * @tc.expected: Check result.
     */
    pattern_->UpdateEditingValue("ABCD", 0);
    pattern_->selectionMode_ = SelectionMode::SELECT;
    pattern_->textSelector_.baseOffset = 0;
    pattern_->textSelector_.destinationOffset = 3;
    EXPECT_EQ(pattern_->GetRightTextOfCursor(1), u"D");

    pattern_->UpdateEditingValue("ABCD", 1);
    pattern_->selectionMode_ = SelectionMode::NONE;
    EXPECT_EQ(pattern_->GetRightTextOfCursor(1), u"B");

    /**
     * @tc.steps: step3. call GetLeftTextOfCursor.
     * @tc.expected: Check result.
     */
    pattern_->UpdateEditingValue("ABCD", 1);
    pattern_->selectionMode_ = SelectionMode::SELECT;
    pattern_->textSelector_.baseOffset = 1;
    pattern_->textSelector_.destinationOffset = 3;
    EXPECT_EQ(pattern_->GetLeftTextOfCursor(1), u"A");

    pattern_->UpdateEditingValue("ABCD", 1);
    pattern_->selectionMode_ = SelectionMode::NONE;
    EXPECT_EQ(pattern_->GetLeftTextOfCursor(1), u"A");
}

/**
 * @tc.name: CursorMove001
 * @tc.desc: test method of cursor movement
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CursorMove001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call CursorMoveEnd.
     * @tc.expected: Check result.
     */
    pattern_->UpdateEditingValue("ABCD", 4);
    EXPECT_TRUE(pattern_->CursorMoveEnd());

    pattern_->textEditingValue_.caretPosition = 1;
    EXPECT_TRUE(pattern_->CursorMoveEnd());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 4);

    /**
     * @tc.steps: step3. call CursorMoveToParagraphEnd.
     * @tc.expected: Check result.
     */
    pattern_->UpdateEditingValue("ABCD", 4);
    EXPECT_TRUE(pattern_->CursorMoveToParagraphEnd());

    pattern_->textEditingValue_.caretPosition = 1;
    EXPECT_TRUE(pattern_->CursorMoveToParagraphEnd());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 4);

    /**
     * @tc.steps: step4. call CursorMoveLineEnd.
     * @tc.expected: Check result.
     */
    pattern_->UpdateEditingValue("ABCD", 4);
    EXPECT_TRUE(pattern_->CursorMoveLineEnd());

    pattern_->textEditingValue_.caretPosition = 1;
    pattern_->selectionMode_ = SelectionMode::SELECT_ALL;
    pattern_->textSelector_.baseOffset = 1;
    pattern_->textSelector_.destinationOffset = 3;
    EXPECT_TRUE(pattern_->CursorMoveLineEnd());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 4);

    pattern_->textEditingValue_.caretPosition = 1;
    pattern_->selectionMode_ = SelectionMode::SELECT;
    layoutProperty_->UpdateMaxLines(2);
    pattern_->CursorMoveLineEnd();
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 4);

    pattern_->textEditingValue_.caretPosition = 1;
    pattern_->selectionMode_ = SelectionMode::NONE;
    layoutProperty_->UpdateMaxLines(1);
    pattern_->CursorMoveLineEnd();
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 4);

    pattern_->textEditingValue_.caretPosition = -1;
    EXPECT_FALSE(pattern_->CursorMoveLineEnd());

    pattern_->textEditingValue_.caretPosition = 5;
    EXPECT_FALSE(pattern_->CursorMoveLineEnd());
}

/**
 * @tc.name: CursorMove002
 * @tc.desc: test method of cursor movement
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CursorMove002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call CursorMoveRightWord.
     * @tc.expected: Check result.
     */
    pattern_->UpdateEditingValue("ABCD", 4);
    EXPECT_TRUE(pattern_->CursorMoveRightWord());

    pattern_->textEditingValue_.caretPosition = 1;
    pattern_->selectionMode_ = SelectionMode::SELECT_ALL;
    pattern_->textSelector_.baseOffset = 1;
    pattern_->textSelector_.destinationOffset = 3;
    EXPECT_TRUE(pattern_->CursorMoveRightWord());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 4);

    pattern_->textEditingValue_.caretPosition = 1;
    pattern_->selectionMode_ = SelectionMode::SELECT;
    EXPECT_TRUE(pattern_->CursorMoveRightWord());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 4);

    pattern_->textEditingValue_.caretPosition = 1;
    pattern_->selectionMode_ = SelectionMode::NONE;
    EXPECT_TRUE(pattern_->CursorMoveRightWord());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 4);

    pattern_->textEditingValue_.caretPosition = 5;
    EXPECT_FALSE(pattern_->CursorMoveRightWord());

    /**
     * @tc.steps: step3. call CursorMoveHome.
     * @tc.expected: Check result.
     */
    pattern_->textEditingValue_.caretPosition = 0;
    EXPECT_TRUE(pattern_->CursorMoveHome());

    pattern_->textEditingValue_.caretPosition = 1;
    EXPECT_TRUE(pattern_->CursorMoveHome());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 0);

    /**
     * @tc.steps: step4. call CursorMoveToParagraphBegin.
     * @tc.expected: Check result.
     */
    pattern_->textEditingValue_.caretPosition = 0;
    EXPECT_TRUE(pattern_->CursorMoveToParagraphBegin());

    pattern_->textEditingValue_.caretPosition = 1;
    EXPECT_TRUE(pattern_->CursorMoveToParagraphBegin());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 0);
}

/**
 * @tc.name: CursorMove003
 * @tc.desc: test method of cursor movement
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, CursorMove003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call CursorMoveLineBegin.
     * @tc.expected: Check result.
     */
    pattern_->UpdateEditingValue("ABCD", 0);
    EXPECT_TRUE(pattern_->CursorMoveLineBegin());

    pattern_->textEditingValue_.caretPosition = 1;
    pattern_->selectionMode_ = SelectionMode::SELECT_ALL;
    pattern_->textSelector_.baseOffset = 1;
    pattern_->textSelector_.destinationOffset = 3;
    EXPECT_TRUE(pattern_->CursorMoveLineBegin());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 0);

    pattern_->textEditingValue_.caretPosition = 1;
    pattern_->selectionMode_ = SelectionMode::SELECT;
    layoutProperty_->UpdateMaxLines(2);
    EXPECT_TRUE(pattern_->CursorMoveLineBegin());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 0);

    pattern_->textEditingValue_.caretPosition = 1;
    pattern_->selectionMode_ = SelectionMode::NONE;
    layoutProperty_->UpdateMaxLines(1);
    EXPECT_TRUE(pattern_->CursorMoveLineBegin());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 0);

    /**
     * @tc.steps: step3. call CursorMoveLeftWord.
     * @tc.expected: Check result.
     */
    pattern_->UpdateEditingValue("AB CD", 0);
    EXPECT_TRUE(pattern_->CursorMoveLeftWord());

    pattern_->textEditingValue_.caretPosition = 1;
    pattern_->selectionMode_ = SelectionMode::SELECT_ALL;
    pattern_->textSelector_.baseOffset = 1;
    pattern_->textSelector_.destinationOffset = 3;
    EXPECT_TRUE(pattern_->CursorMoveLeftWord());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 0);

    pattern_->textEditingValue_.caretPosition = 1;
    pattern_->textSelector_.baseOffset = 1;
    pattern_->textSelector_.destinationOffset = 3;
    pattern_->selectionMode_ = SelectionMode::SELECT;
    EXPECT_FALSE(pattern_->CursorMoveLeftWord());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 1);

    pattern_->textEditingValue_.caretPosition = 4;
    pattern_->selectionMode_ = SelectionMode::NONE;
    layoutProperty_->UpdateMaxLines(1);
    EXPECT_TRUE(pattern_->CursorMoveLeftWord());
    EXPECT_EQ(pattern_->textEditingValue_.caretPosition, 3);
}

/**
 * @tc.name: TextFilter
 * @tc.desc: test method of line position
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, TextFilter, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call FilterWithEmail and FilterWithAscii.
     * @tc.expected: Check result.
     */
    std::string result = "wyz@@xx.com";
    EXPECT_TRUE(pattern_->FilterWithEmail(result));
    result = "wyz@xx.com";
    EXPECT_FALSE(pattern_->FilterWithEmail(result));

    std::string valueToUpdate = "wyz测试";
    result = "";
    EXPECT_TRUE(pattern_->FilterWithAscii(valueToUpdate, result));
    EXPECT_STREQ(result.c_str(), "wyz");
}

/**
 * @tc.name: ProcessPasswordIcon
 * @tc.desc: test method of ProcessPasswordIcon
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, ProcessPasswordIcon, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call ProcessPasswordIcon.
     * @tc.expected: Check result.
     */
    layoutProperty_->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    layoutProperty_->UpdateShowPasswordIcon(false);
    pattern_->ProcessPasswordIcon();

    layoutProperty_->UpdateShowPasswordIcon(true);
    ImageSourceInfo imageSourceInfo;
    layoutProperty_->UpdateHidePasswordSourceInfo(imageSourceInfo);
    pattern_->textObscured_ = true;
    pattern_->hideUserDefinedIcon_ = true;

    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
        if (type == IconTheme::TypeId()) {
            return AceType::MakeRefPtr<IconTheme>();
        }
        return AceType::MakeRefPtr<TextFieldTheme>();
    });

    pattern_->ProcessPasswordIcon();
    ASSERT_NE(pattern_->hidePasswordImageLoadingCtx_, nullptr);
    auto loadSuccessTask = pattern_->CreateLoadSuccessCallback(pattern_->textObscured_);
    loadSuccessTask(imageSourceInfo);
    EXPECT_NE(pattern_->hidePasswordCanvasImage_, nullptr);
    auto loadFailTask = pattern_->CreateLoadFailCallback(pattern_->textObscured_);
    loadFailTask(imageSourceInfo, "tdd error");
    auto dataReadyTask = pattern_->CreateDataReadyCallback(pattern_->textObscured_);
    dataReadyTask(imageSourceInfo);

    pattern_->hideUserDefinedIcon_ = false;
    imageSourceInfo.SetSrc("/pages/media/test.jpg");
    pattern_->ProcessPasswordIcon();
    loadFailTask(imageSourceInfo, "tdd error");

    pattern_->textObscured_ = false;
    pattern_->showUserDefinedIcon_ = true;
    layoutProperty_->UpdateShowPasswordSourceInfo(imageSourceInfo);
    pattern_->ProcessPasswordIcon();
    ASSERT_NE(pattern_->showPasswordImageLoadingCtx_, nullptr);
    loadSuccessTask = pattern_->CreateLoadSuccessCallback(pattern_->textObscured_);
    loadSuccessTask(imageSourceInfo);
    loadFailTask = pattern_->CreateLoadFailCallback(pattern_->textObscured_);
    loadFailTask(imageSourceInfo, "tdd error");

    pattern_->showUserDefinedIcon_ = false;
    pattern_->ProcessPasswordIcon();
    loadFailTask(imageSourceInfo, "tdd error");
    EXPECT_NE(pattern_->showPasswordCanvasImage_, nullptr);
}

/**
 * @tc.name: OnCursorTwinkling
 * @tc.desc: test method of OnCursorTwinkling
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnCursorTwinkling, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call OnCursorTwinkling.
     * @tc.expected: Check result.
     */
    pattern_->cursorVisible_ = true;
    pattern_->obscureTickCountDown_ = 5;
    layoutProperty_->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    pattern_->textObscured_ = true;
    pattern_->OnCursorTwinkling();
    EXPECT_EQ(pattern_->obscureTickCountDown_, 4);
    EXPECT_FALSE(pattern_->cursorVisible_);

    pattern_->obscureTickCountDown_ = 0;
    pattern_->OnCursorTwinkling();
    EXPECT_EQ(pattern_->obscureTickCountDown_, 0);
    EXPECT_TRUE(pattern_->cursorVisible_);

    pattern_->obscureTickCountDown_ = 1;
    pattern_->textObscured_ = false;
    pattern_->OnCursorTwinkling();
    EXPECT_FALSE(pattern_->cursorVisible_);
    EXPECT_EQ(pattern_->obscureTickCountDown_, 1);

    layoutProperty_->UpdateTextInputType(TextInputType::TEXT);
    pattern_->OnCursorTwinkling();
    EXPECT_TRUE(pattern_->cursorVisible_);
    EXPECT_EQ(pattern_->obscureTickCountDown_, 1);

    layoutProperty_->UpdateMaxLines(1);
    pattern_->textObscured_ = true;
    layoutProperty_->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    pattern_->obscureTickCountDown_ = 1;
    pattern_->OnCursorTwinkling();
    EXPECT_FALSE(pattern_->cursorVisible_);
    EXPECT_EQ(pattern_->obscureTickCountDown_, 0);
}

/**
 * @tc.name: OnColorConfigurationUpdate001
 * @tc.desc: test on color configuration update
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, OnColorConfigurationUpdate001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTINPUT_ETS_TAG, 1, []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextTheme>()));
    auto context = frameNode->GetContext();
    ASSERT_NE(context, nullptr);
    auto theme = context->GetTheme<TextTheme>();
    ASSERT_NE(theme, nullptr);
    theme->textStyle_.textColor_ = Color::BLACK;
    pattern->OnColorConfigurationUpdate();
    auto layoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetTextColor(), Color::BLACK);
}

/**
 * @tc.name: HandleClickEvent
 * @tc.desc: test method of HandleClickEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleClickEvent, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call HandleClickEvent.
     * @tc.expected: Check result.
     */
    auto renderContext = host_->GetRenderContext();
    auto mockRenderContext = AceType::DynamicCast<MockRenderContext>(renderContext);
    EXPECT_CALL(*mockRenderContext, GetPaintRectWithTransform()).WillRepeatedly(Return(RectF()));

    auto focusHub = host_->GetOrCreateFocusHub();
    focusHub->eventHub_.Upgrade()->SetEnabled(true);
    focusHub->focusType_ = FocusType::NODE;
    layoutProperty_->UpdateVisibility(VisibleType::VISIBLE);
    focusHub->focusable_ = true;
    focusHub->parentFocusable_ = true;
    GestureEvent info;
    pattern_->HandleClickEvent(info);

    focusHub->currentFocus_ = true;
    pattern_->hasClicked_ = false;
    pattern_->HandleClickEvent(info);

    focusHub->focusCallbackEvents_ = AceType::MakeRefPtr<FocusCallbackEvents>();
    focusHub->focusCallbackEvents_->isFocusOnTouch_ = false;
    pattern_->hasClicked_ = false;
    pattern_->HandleClickEvent(info);

    pattern_->isMousePressed_ = true;
    pattern_->hasClicked_ = false;
    pattern_->HandleClickEvent(info);
    EXPECT_FALSE(pattern_->isMousePressed_);

    info.SetLocalLocation(Offset(90, 90));
    pattern_->imageRect_.SetWidth(20);
    pattern_->frameRect_.SetWidth(100);
    layoutProperty_->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    layoutProperty_->UpdateShowPasswordIcon(true);
    pattern_->hasClicked_ = false;
    pattern_->HandleClickEvent(info);
    EXPECT_EQ(pattern_->caretUpdateType_, CaretUpdateType::ICON_PRESSED);

    layoutProperty_->UpdateShowPasswordIcon(false);
    pattern_->hasClicked_ = false;
    pattern_->HandleClickEvent(info);

    info.SetLocalLocation(Offset(10, 90));
    pattern_->hasClicked_ = false;
    pattern_->HandleClickEvent(info);
}

/**
 * @tc.name: HandleDoubleClickEvent
 * @tc.desc: test method of HandleDoubleClickEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestNg, HandleDoubleClickEvent, TestSize.Level1)
{
    /**
     * @tc.steps: step1. RunSetUp to Create TextFieldPattern.
     * @tc.expected: Check it is not nullptr.
     */
    RunSetUp();
    /**
     * @tc.steps: step2. call HandleClickEvent function quickly to trigger doubleClick.
     * @tc.expected: Check result.
     */
    auto renderContext = host_->GetRenderContext();
    auto mockRenderContext = AceType::DynamicCast<MockRenderContext>(renderContext);
    EXPECT_CALL(*mockRenderContext, GetPaintRectWithTransform()).WillRepeatedly(Return(RectF()));

    auto focusHub = host_->GetOrCreateFocusHub();
    focusHub->eventHub_.Upgrade()->SetEnabled(true);
    focusHub->focusType_ = FocusType::NODE;
    layoutProperty_->UpdateVisibility(VisibleType::VISIBLE);
    focusHub->focusable_ = true;
    focusHub->parentFocusable_ = true;
    GestureEvent info;
    pattern_->HandleClickEvent(info);

    focusHub->currentFocus_ = true;
    pattern_->hasClicked_ = false;
    pattern_->HandleClickEvent(info);
    focusHub->focusCallbackEvents_ = AceType::MakeRefPtr<FocusCallbackEvents>();
    focusHub->focusCallbackEvents_->isFocusOnTouch_ = false;
    pattern_->hasClicked_ = false;

    // test gesture doubleClick
    pattern_->HandleClickEvent(info);
    EXPECT_TRUE(pattern_->hasClicked_);
    pattern_->isUsingMouse_ = false;
    pattern_->HandleClickEvent(info);
    EXPECT_FALSE(pattern_->hasClicked_);

    // test mouse doubleClick
    pattern_->HandleClickEvent(info);
    EXPECT_TRUE(pattern_->hasClicked_);
    pattern_->isUsingMouse_ = true;
    pattern_->HandleClickEvent(info);
    EXPECT_FALSE(pattern_->hasClicked_);

    // test timeout > 300 ms
    pattern_->HandleClickEvent(info);
    EXPECT_TRUE(pattern_->hasClicked_);
    pattern_->lastClickTimeStamp_ = pattern_->lastClickTimeStamp_ - std::chrono::seconds(1);
    pattern_->HandleClickEvent(info);
    EXPECT_TRUE(pattern_->hasClicked_);
}

} // namespace OHOS::Ace::NG
