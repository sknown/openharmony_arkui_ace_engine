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

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

#define private public
#define protected public

#include "test/mock/base/mock_task_executor.h"
#include "test/mock/core/common/mock_container.h"
#include "test/mock/core/common/mock_data_detector_mgr.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/render/mock_paragraph.h"
#include "test/mock/core/render/mock_render_context.h"
#include "test/mock/core/rosen/mock_canvas.h"
#include "test/unittest/core/pattern/test_ng.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/offset.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/string_utils.h"
#include "base/utils/type_definition.h"
#include "core/common/ai/data_detector_mgr.h"
#include "core/common/ime/constant.h"
#include "core/common/ime/text_editing_value.h"
#include "core/common/ime/text_input_action.h"
#include "core/common/ime/text_input_type.h"
#include "core/common/ime/text_selection.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/text_style.h"
#include "core/components/scroll/scroll_bar_theme.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/components/theme/theme_manager.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_manager.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"
#include "core/components_ng/pattern/text_field/text_field_model_ng.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_event_hub.h"
#include "core/event/key_event.h"
#include "core/event/touch_event.h"
#include "core/gestures/gesture_info.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr double ICON_SIZE = 24;
constexpr double ICON_HOT_ZONE_SIZE = 40;
constexpr double FONT_SIZE = 16;
constexpr float OFFSET = 3;
constexpr int32_t DEFAULT_NODE_ID = 1;
constexpr int32_t MIN_PLATFORM_VERSION = 10;
const std::string DEFAULT_TEXT = "abcdefghijklmnopqrstuvwxyz";
const std::string DEFAULT_TEXT_THREE_LINE = "abcdef\nghijkl\nmnopqr\n";
const std::string HELLO_TEXT = "hello";
const std::string DEFAULT_PLACE_HOLDER = "please input text here";
const std::string LOWERCASE_FILTER = "[a-z]";
const std::string NUMBER_FILTER = "^[0-9]*$";
const std::string DEFAULT_INPUT_FILTER = "[a-z]";
template<typename CheckItem, typename Expected>
struct TestItem {
    CheckItem item;
    Expected expected;
    std::string error;
    TestItem(CheckItem checkItem, Expected expectedValue, std::string message = "")
        : item(checkItem), expected(expectedValue), error(std::move(message))
    {}
    TestItem() = default;
};
struct ExpectParagraphParams {
    float height = 50.f;
    float longestLine = 460.f;
    float maxWidth = 460.f;
    size_t lineCount = 1;
    bool firstCalc = true;
    bool secondCalc = true;
};
} // namespace

class TextAreaBase : public TestNG {
protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void TearDown() override;

    void CreateTextField(const std::string& text = "", const std::string& placeHolder = "",
        const std::function<void(TextFieldModelNG&)>& callback = nullptr);
    static void ExpectCallParagraphMethods(ExpectParagraphParams params);
    void GetFocus();

    RefPtr<FrameNode> frameNode_;
    RefPtr<TextFieldPattern> pattern_;
    RefPtr<TextFieldEventHub> eventHub_;
    RefPtr<TextFieldLayoutProperty> layoutProperty_;
    RefPtr<TextFieldAccessibilityProperty> accessibilityProperty_;
};

void TextAreaBase::SetUpTestSuite()
{
    TestNG::SetUpTestSuite();
    ExpectCallParagraphMethods(ExpectParagraphParams());
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto textFieldTheme = AceType::MakeRefPtr<TextFieldTheme>();
    textFieldTheme->iconSize_ = Dimension(ICON_SIZE, DimensionUnit::VP);
    textFieldTheme->iconHotZoneSize_ = Dimension(ICON_HOT_ZONE_SIZE, DimensionUnit::VP);
    textFieldTheme->fontSize_ = Dimension(FONT_SIZE, DimensionUnit::FP);
    textFieldTheme->fontWeight_ = FontWeight::W400;
    textFieldTheme->textColor_ = Color::FromString("#ff182431");
    EXPECT_CALL(*themeManager, GetTheme(_))
        .WillRepeatedly([textFieldTheme = textFieldTheme](ThemeType type) -> RefPtr<Theme> {
            if (type == ScrollBarTheme::TypeId()) {
                return AceType::MakeRefPtr<ScrollBarTheme>();
            }
            return textFieldTheme;
        });
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(MIN_PLATFORM_VERSION);
    MockPipelineContext::GetCurrent()->SetTextFieldManager(AceType::MakeRefPtr<TextFieldManagerNG>());
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
}

void TextAreaBase::TearDownTestSuite()
{
    TestNG::TearDownTestSuite();
    MockParagraph::TearDown();
}

void TextAreaBase::TearDown()
{
    frameNode_ = nullptr;
    pattern_ = nullptr;
    eventHub_ = nullptr;
    layoutProperty_ = nullptr;
    accessibilityProperty_ = nullptr;
}

void TextAreaBase::ExpectCallParagraphMethods(ExpectParagraphParams params)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, PushStyle(_)).Times(AnyNumber());
    EXPECT_CALL(*paragraph, AddText(_)).Times(AnyNumber());
    EXPECT_CALL(*paragraph, PopStyle()).Times(AnyNumber());
    EXPECT_CALL(*paragraph, Build()).Times(AnyNumber());
    EXPECT_CALL(*paragraph, Layout(_)).Times(AnyNumber());
    EXPECT_CALL(*paragraph, GetTextWidth()).WillRepeatedly(Return(params.maxWidth));
    EXPECT_CALL(*paragraph, GetAlphabeticBaseline()).WillRepeatedly(Return(0.f));
    EXPECT_CALL(*paragraph, GetHeight()).WillRepeatedly(Return(params.height));
    EXPECT_CALL(*paragraph, GetLongestLine()).WillRepeatedly(Return(params.longestLine));
    EXPECT_CALL(*paragraph, GetMaxWidth()).WillRepeatedly(Return(params.maxWidth));
    EXPECT_CALL(*paragraph, GetLineCount()).WillRepeatedly(Return(params.lineCount));
}

void TextAreaBase::CreateTextField(
    const std::string& text, const std::string& placeHolder, const std::function<void(TextFieldModelNG&)>& callback)
{
    auto* stack = ViewStackProcessor::GetInstance();
    stack->StartGetAccessRecordingFor(DEFAULT_NODE_ID);
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.CreateTextArea(placeHolder, text);
    if (callback) {
        callback(textFieldModelNG);
    }
    stack->StopGetAccessRecording();
    frameNode_ = AceType::DynamicCast<FrameNode>(stack->Finish());
    pattern_ = frameNode_->GetPattern<TextFieldPattern>();
    eventHub_ = frameNode_->GetEventHub<TextFieldEventHub>();
    layoutProperty_ = frameNode_->GetLayoutProperty<TextFieldLayoutProperty>();
    accessibilityProperty_ = frameNode_->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    FlushLayoutTask(frameNode_);
}

void TextAreaBase::GetFocus()
{
    auto focushHub = pattern_->GetFocusHub();
    focushHub->currentFocus_ = true;
    pattern_->HandleFocusEvent();
    FlushLayoutTask(frameNode_);
}

class TextFieldUXTest : public TextAreaBase {};

/**
 * @tc.name: IsTextArea001
 * @tc.desc: Test is text area or text input.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, IsTextArea001, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.expected: Current caret position is end of text
     */
    GetFocus();
    EXPECT_TRUE(pattern_->IsTextArea());
}

/**
 * @tc.name: PerformAction001
 * @tc.desc: Test function PerformAction.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, PerformAction001, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.expected: Current caret position is end of text
     */
    GetFocus();

    /**
     * @tc.steps: set TextInputAction NEW_LINE and call PerformAction
     * @tc.expected: text will wrap
     */
    auto paintProperty = frameNode_->GetPaintProperty<TextFieldPaintProperty>();
    paintProperty->UpdateInputStyle(InputStyle::INLINE);
    frameNode_->MarkModifyDone();
    pattern_->OnModifyDone();
    auto textInputAction = pattern_->GetDefaultTextInputAction();
    EXPECT_EQ(textInputAction, TextInputAction::NEW_LINE);
    pattern_->focusIndex_ = FocuseIndex::TEXT;
    EXPECT_TRUE(pattern_->IsTextArea());
    EXPECT_TRUE(pattern_->GetInputFilter() != "\n");
    pattern_->PerformAction(textInputAction, false);
    EXPECT_EQ(pattern_->TextInputActionToString(), "EnterKeyType.Done");
}

/**
 * @tc.name: CursorInContentRegion001
 * @tc.desc: Test function CursorInContentRegion.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, CursorInContentRegion001, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.expected: Cursor realy in the content region
     */
    GetFocus();
    EXPECT_EQ(pattern_->GetTextOrPlaceHolderFontSize(), FONT_SIZE);
    EXPECT_TRUE(pattern_->CursorInContentRegion());
}

/**
 * @tc.name: OnTextAreaScroll001
 * @tc.desc: Test textfield to create paint.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, OnTextAreaScroll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input.
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.steps: step2. call OnTextAreaScroll
     * tc.expected: step2. Check if the currentOffset_ is right.
     */
    auto accessibilityProperty = frameNode_->GetAccessibilityProperty<AccessibilityProperty>();
    EXPECT_TRUE(accessibilityProperty->ActActionScrollForward());

    /**
     * @tc.steps: step3.set contentRect_.GetY() = 1
     */
    pattern_->contentRect_ = RectF(1.0f, 1.0f, 1.0f, 1.0f);
    FlushLayoutTask(frameNode_);
    pattern_->OnTextAreaScroll(OFFSET);
    EXPECT_EQ(pattern_->currentOffset_, 1);
}

/**
 * @tc.name: CursorOperation001
 * @tc.desc: Test cursor move
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, CursorOperation001, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text field node with three lines of text.
     */
    CreateTextField(DEFAULT_TEXT_THREE_LINE);

    /**
     * @tc.expected: Current caret position at 2nd line.
     */
    GetFocus();
    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(10);
    EXPECT_EQ(pattern_->GetCaretIndex(), 10);

    /**
     * @tc.steps: set InputOperation CURSOR_UP and call BeforeCreateLayoutWrapper
     * @tc.expected: caret will move up
     */
    pattern_->inputOperations_.push(InputOperation::CURSOR_UP);
    auto ret = pattern_->CursorMoveUp();
    EXPECT_FALSE(ret);
    pattern_->BeforeCreateLayoutWrapper();
    EXPECT_EQ(pattern_->GetCaretIndex(), 0);
}

/**
 * @tc.name: CursorOperation002
 * @tc.desc: Test cursor move
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, CursorOperation002, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text field node with three lines of text.
     */
    CreateTextField(DEFAULT_TEXT_THREE_LINE);

    /**
     * @tc.expected: Current caret position at 2nd line.
     */
    GetFocus();
    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(10);
    EXPECT_EQ(pattern_->GetCaretIndex(), 10);

    /**
     * @tc.steps: set InputOperation CURSOR_DOWN and call BeforeCreateLayoutWrapper
     * @tc.expected: caret will move down
     */
    pattern_->inputOperations_.push(InputOperation::CURSOR_DOWN);
    auto ret = pattern_->CursorMoveDown();
    EXPECT_FALSE(ret);
    pattern_->BeforeCreateLayoutWrapper();
    EXPECT_EQ(pattern_->GetCaretIndex(), 0);
}

/**
 * @tc.name: CursorOperation003
 * @tc.desc: Test cursor move
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, CursorOperation003, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text field node with three lines of text.
     */
    CreateTextField(DEFAULT_TEXT_THREE_LINE);

    /**
     * @tc.expected: Current caret position at 2nd line.
     */
    GetFocus();
    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(10);
    EXPECT_EQ(pattern_->GetCaretIndex(), 10);

    /**
     * @tc.steps: set InputOperation CURSOR_LEFT and call BeforeCreateLayoutWrapper
     * @tc.expected: caret will move left
     */
    pattern_->inputOperations_.push(InputOperation::CURSOR_LEFT);
    auto ret = pattern_->CursorMoveLeft();
    EXPECT_FALSE(ret);
    pattern_->BeforeCreateLayoutWrapper();
    EXPECT_EQ(pattern_->GetCaretIndex(), 8);
}

/**
 * @tc.name: CursorOperation004
 * @tc.desc: Test cursor move
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, CursorOperation004, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text field node with three lines of text.
     */
    CreateTextField(DEFAULT_TEXT_THREE_LINE);

    /**
     * @tc.expected: Current caret position at 2nd line.
     */
    GetFocus();
    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(10);
    EXPECT_EQ(pattern_->GetCaretIndex(), 10);

    /**
     * @tc.steps: set InputOperation CURSOR_RIGHT and call BeforeCreateLayoutWrapper
     * @tc.expected: caret will move right
     */
    pattern_->inputOperations_.push(InputOperation::CURSOR_RIGHT);
    auto ret = pattern_->CursorMoveRight();
    EXPECT_FALSE(ret);
    pattern_->BeforeCreateLayoutWrapper();
    EXPECT_EQ(pattern_->GetCaretIndex(), 12);
}

/**
 * @tc.name: CursorOperation004
 * @tc.desc: Test delete text with ControllerSelected.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, DeleteTextWithControllerSelected001, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text input node and call delete backward
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.steps: getFocus and set controller
     * @tc.steps: firsthandle 5 secondhandle 10
     */
    GetFocus();
    pattern_->HandleSetSelection(5, 10, false);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: call DeleteBackward
     * @tc.expected: text will be reduced by five characters
     */
    pattern_->DeleteBackward(5);
    EXPECT_EQ(pattern_->GetTextValue().compare("abcdeklmnopqrstuvwxyz"), 0) << "Text is " + pattern_->GetTextValue();
    EXPECT_EQ(pattern_->GetCaretIndex(), 5);
}

/**
 * @tc.name: InitSurfaceChangedCallback001
 * @tc.desc: Test init syrface change and callback.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, InitSurfaceChangedCallback001, TestSize.Level1)
{
    /**
     * @tc.steps: Create textfield node
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.expected: Verify is the callback successful
     */
    GetFocus();
    FlushLayoutTask(frameNode_);
    pattern_->HandleSurfaceChanged(0, 0, 0, 0);
    EXPECT_NE(pattern_->surfaceChangedCallbackId_, std::nullopt);
    EXPECT_TRUE(pattern_->HasSurfaceChangedCallback());

    /**
     * @tc.expected: set selectOverlay
     */
    TouchLocationInfo touchLocationInfo1(0);
    touchLocationInfo1.touchType_ = TouchType::DOWN;
    touchLocationInfo1.localLocation_ = Offset(0.0f, 0.0f);

    /**
     * @tc.steps: step3. create touch info, touch type DOWN
     */
    TouchEventInfo touchInfo1("");
    touchInfo1.AddTouchLocationInfo(std::move(touchLocationInfo1));

    /**
     * @tc.steps: step4. test touch down
     */
    pattern_->ProcessOverlay(true, true, true);
    pattern_->HandleSurfaceChanged(0, 0, 0, 0);
    pattern_->processOverlayDelayTask_.operator()();
    EXPECT_FALSE(pattern_->GetOriginIsMenuShow());
}

/**
 * @tc.name: InitSurfacePositionChangedCallback001
 * @tc.desc: Test init syrface Position change and callback.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, InitSurfacePositionChangedCallback001, TestSize.Level1)
{
    /**
     * @tc.steps: Create textfield node
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.expected: Verify is the callback successful
     */
    GetFocus();
    FlushLayoutTask(frameNode_);
    pattern_->HandleSurfacePositionChanged(1, 1);
    EXPECT_NE(pattern_->surfacePositionChangedCallbackId_, std::nullopt);
    EXPECT_TRUE(pattern_->HasSurfacePositionChangedCallback());
}

/**
 * @tc.name: TextAreaCursorInContentRegion001
 * @tc.desc: Test is cursor in the content region.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, TextAreaCursorInContentRegion001, TestSize.Level1)
{
    /**
     * @tc.steps: Create textfield node
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.expected: get cursor
     */
    GetFocus();

    /**
     * @tc.expected: test if caret in the right position
     */
    EXPECT_TRUE(pattern_->CursorInContentRegion());
}
}
