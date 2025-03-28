/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "text_input_base.h"

#include "core/components_ng/pattern/indexer/indexer_layout_property.h"

namespace OHOS::Ace::NG {

namespace {} // namespace

class TextInputCursorTest : public TextInputBases {
public:
    RefPtr<TextFieldLayoutProperty> layoutProperty_;
    RefPtr<TextFieldPattern> pattern_;
};

/**
 * @tc.name: CaretPosition001
 * @tc.desc: Test caret position on TextFieldModelNG::CreateNode.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition001, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.expected: Current caret position is end of text
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size()));

    /**
     * @tc.steps: Changed new text and remeasure and layout
     */
    pattern_->InsertValue("new");
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Current caret position is end of text
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size() + 3));
}

/**
 * @tc.name: CaretPosition002
 * @tc.desc: Test caret position on SetType.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition002, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder and set input type
     */
    std::string text = "openharmony@harmony.com+* ()0123456789";
    std::vector<TestItem<TextInputType, int32_t>> testItems;
    testItems.emplace_back(TextInputType::TEXT, text.length(), "TextInputType::TEXT");
    testItems.emplace_back(TextInputType::NUMBER, 10, "TextInputType::NUMBER");
    testItems.emplace_back(TextInputType::PHONE, 15, "TextInputType::PHONE");
    testItems.emplace_back(TextInputType::EMAIL_ADDRESS, text.length() - 5, "TextInputType::EMAIL_ADDRESS");
    testItems.emplace_back(TextInputType::VISIBLE_PASSWORD, text.length(), "TextInputType::VISIBLE_PASSWORD");
    testItems.emplace_back(TextInputType::NUMBER_PASSWORD, 10, "TextInputType::NUMBER_PASSWORD");
    testItems.emplace_back(TextInputType::SCREEN_LOCK_PASSWORD, text.length(), "TextInputType::SCREEN_LOCK_PASSWORD");

    /**
     * @tc.expected: Check if the text filter rules for the input box are compliant
     */
    for (const auto& testItem : testItems) {
        CreateTextField(text, "", [testItem](TextFieldModelNG& model) { model.SetType(testItem.item); });
        auto errorMessage = "InputType is " + testItem.error + ", text is " + pattern_->GetTextValue();
        EXPECT_EQ(pattern_->GetCaretIndex(), testItem.expected) << errorMessage;
        TearDown();
    }
}

/**
 * @tc.name: CaretPosition003
 * @tc.desc: Test caret position on SetCaretPosition
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition003, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text
     * @tc.expected: Cursor movement position matches the actual position
     */
    CreateTextField(DEFAULT_TEXT);
    auto controller = pattern_->GetTextFieldController();
    controller->CaretPosition(static_cast<int>(DEFAULT_TEXT.size() - 2));
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size() - 2));
}

/**
 * @tc.name: CaretPosition004
 * @tc.desc: Test caret position on SetMaxLength
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition004, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     * @tc.expected: Cursor movement position matches the actual position
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG& model) { model.SetMaxLength(DEFAULT_TEXT.size() - 2); });
    auto controller = pattern_->GetTextFieldController();
    controller->CaretPosition(static_cast<int>(DEFAULT_TEXT.size() - 2));
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size() - 2));
}

/**
 * @tc.name: CaretPosition005
 * @tc.desc: Test caret position on SetInputFilter.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition005, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text and filter patterns
     */
    std::string text = "abcdefghABCDEFG0123456789";
    std::vector<TestItem<std::string, int32_t>> testItems;
    testItems.emplace_back("", StringUtils::ToWstring(text).length());
    testItems.emplace_back("[0-9]", 10);
    testItems.emplace_back("[A-Z]", 7);
    testItems.emplace_back("[a-z]", 8);

    /**
     * @tc.expected: Check if the text filter patterns for the input box are compliant
     */
    for (const auto& testItem : testItems) {
        CreateTextField(
            text, "", [testItem](TextFieldModelNG& model) { model.SetInputFilter(testItem.item, nullptr); });
        auto errorMessage = "InputType is " + testItem.item + ", text is " + pattern_->GetTextValue();
        EXPECT_EQ(pattern_->GetCaretIndex(), testItem.expected) << errorMessage;
        TearDown();
    }
}

/**
 * @tc.name: CaretPosition005
 * @tc.desc: Test input string at the cursor position
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition006, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text input and get select controller, update caret position and insert value
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(2);
    pattern_->InsertValue("new");
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue(), "abnewcdefghijklmnopqrstuvwxyz");
    EXPECT_EQ(controller->GetCaretIndex(), 5);
}

/**
 * @tc.name: CaretPosition006
 * @tc.desc: Test stop edting input mode
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition007, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text input node
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.expected: The cursor is neither blinking nor visible when unfocused
     */
    EXPECT_FALSE(pattern_->GetCursorVisible());

    /**
     * @tc.steps: Manually trigger focus and perform measure and layout again
     * @tc.expected: Check if the cursor is twinking
     */
    GetFocus();
    EXPECT_TRUE(pattern_->GetCursorVisible());

    /**
     * @tc.steps: Get text filed controller and stop editing
     */
    auto controller = pattern_->GetTextFieldController();
    controller->StopEditing();
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the cursor stop twinking
     */
    EXPECT_FALSE(pattern_->GetCursorVisible());
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition001
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition001, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text input node and call text changed listener update edting value
     */
    CreateTextField(DEFAULT_TEXT);

    GetFocus();
    TextEditingValue value;
    TextSelection selection;
    value.text = "new text";
    selection.baseOffset = value.text.length();
    value.selection = selection;
    pattern_->UpdateEditingValue(std::make_shared<TextEditingValue>(value));
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare("new text"), 0);
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(value.text.length()));
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition002
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition002, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text input node and call delete backward
     */
    CreateTextField(DEFAULT_TEXT);

    GetFocus();
    pattern_->DeleteBackward(5);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare("abcdefghijklmnopqrstu"), 0) << "Text is " + pattern_->GetTextValue();
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.length() - 5));

    /**
     * @tc.steps: Move the cursor and then delete text
     */
    auto textFiledController = pattern_->GetTextFieldController();
    textFiledController->CaretPosition(5);
    pattern_->DeleteBackward(5);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare("fghijklmnopqrstu"), 0) << "Text is " + pattern_->GetTextValue();
    EXPECT_EQ(pattern_->GetCaretIndex(), 0);

    /**
     * @tc.steps: Trigger a backspace key press that exceeds the length of the text
     */
    pattern_->DeleteBackward(MAX_BACKWARD_NUMBER);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare("fghijklmnopqrstu"), 0) << "Text is " + pattern_->GetTextValue();
    EXPECT_EQ(pattern_->GetCaretIndex(), 0);
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition003
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition003, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize insert text and expected values
     */
    CreateTextField(DEFAULT_TEXT);

    GetFocus();
    pattern_->DeleteForward(5);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare(DEFAULT_TEXT), 0);
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.length()));

    /**
     * @tc.steps: Move the cursor and then delete text forward.
     */
    auto textFiledController = pattern_->GetTextFieldController();
    textFiledController->CaretPosition(5);
    pattern_->DeleteForward(MAX_FORWARD_NUMBER);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare("abcde"), 0) << "Text is " + pattern_->GetTextValue();
    EXPECT_EQ(pattern_->GetCaretIndex(), 5) << "Caret position is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition004
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition004, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize insert text and expected values when 'IsSelected() = false'
     */
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);
    GetFocus();
    pattern_->InsertValue("abc");
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare(DEFAULT_TEXT + "abc"), 0);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length() + 3);

    /**
     * @tc.steps: Move the cursor and then insert text forward.
     */
    auto textFiledController = pattern_->GetTextFieldController();
    textFiledController->CaretPosition(0);
    pattern_->InsertValue("abcde");
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare("abcde" + DEFAULT_TEXT + "abc"), 0);
    EXPECT_EQ(pattern_->GetCaretIndex(), 5);
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition005
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition005, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize insert text and expected values
     */
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);
    int32_t start = 5;
    int32_t end = 10;
    pattern_->HandleSetSelection(start, end, false);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the new handle positions are correct
     */
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, end)
        << "Second index is " + std::to_string(pattern_->selectController_->GetSecondHandleInfo().index);
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition006
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition006, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize insert text and expected values
     */
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);
    std::vector<std::int32_t> action = {
        ACTION_SELECT_ALL,
        ACTION_CUT,
        ACTION_COPY,
        ACTION_PASTE,
    };
    pattern_->HandleExtendAction(action[0]);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the new handle positions are correct
     */
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, 0);
    EXPECT_EQ(pattern_->selectController_->GetCaretRect().GetX(), 0);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 0)
        << "Second index is " + std::to_string(pattern_->selectController_->GetSecondHandleInfo().index);

    /**
     * @tc.steps: Move the handles and then cut text snippet.
     */
    int32_t start = 5;
    int32_t end = 10;
    pattern_->HandleSetSelection(start, end, false);
    pattern_->HandleExtendAction(action[1]);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the new handle positions are correct
     * Cut data hasn't simulated
     */
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, 5);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 5)
        << "Second index is " + std::to_string(pattern_->selectController_->GetSecondHandleInfo().index);
    EXPECT_EQ(pattern_->GetTextSelectController()->GetCaretIndex(), 5);
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition007
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition007, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Initialize text input and Move the handles and then cut text snippet.
     */
    int32_t start = 5;
    int32_t end = 10;
    std::string expectStr = "fghij";
    std::vector<std::int32_t> action = {
        ACTION_SELECT_ALL,
        ACTION_CUT,
        ACTION_COPY,
        ACTION_PASTE,
    };
    auto callback = [expectStr](const std::string& str) { EXPECT_EQ(expectStr, str); };
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER, [&](TextFieldModel& model) { model.SetOnCut(callback); });
    pattern_->HandleSetSelection(start, end, false);
    pattern_->HandleExtendAction(action[1]);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the new handle positions are correct
     *               Verify the cut data
     */
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, start)
        << "Second index is " + std::to_string(pattern_->selectController_->GetSecondHandleInfo().index);
    EXPECT_EQ(pattern_->GetTextSelectController()->GetCaretIndex(), start);
    EXPECT_EQ(pattern_->contentController_->GetTextValue().compare("abcdeklmnopqrstuvwxyz"), 0);
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition008
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition008, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Initialize text input and Move the handles and then cut text snippet.
     */
    int32_t start = 5;
    int32_t end = 10;
    std::string expectStr = "fghij";
    std::vector<std::int32_t> action = {
        ACTION_SELECT_ALL,
        ACTION_CUT,
        ACTION_COPY,
        ACTION_PASTE,
    };
    auto onCopy = [expectStr](const std::string& str) { EXPECT_EQ(expectStr, str); };
    auto onPaste = [expectStr](const std::string& str) { EXPECT_EQ(expectStr, str); };
    auto onPasteWithEvent = [expectStr](const std::string& str, NG::TextCommonEvent& event) {
        EXPECT_EQ(expectStr, str); };
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER, [&](TextFieldModel& model) -> void {
        model.SetOnCopy(onCopy);
        model.SetOnPaste(onPaste);
        model.SetOnPasteWithEvent(onPasteWithEvent);
    });

    /**
     * @tc.steps: Move the handles and then cut text snippet.
     *            Verify the copy and paste data.
     */
    pattern_->HandleSetSelection(start, end, false);
    pattern_->HandleExtendAction(action[2]);
    pattern_->HandleExtendAction(action[3]);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetTextValue().compare("abcdefghijklmnopqrstuvwxyz"), 0)
        << "Text is " + pattern_->GetTextValue();
}

/**
 * @tc.name: OnHandleMove001
 * @tc.desc: Test the clip board interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnHandleMove001, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Initialize text input and Move the handles and then do handle selection.
     */
    int32_t start = 5;
    int32_t end = 10;
    std::vector<CaretMoveIntent> select = { CaretMoveIntent::Left, CaretMoveIntent::Right, CaretMoveIntent::Up,
        CaretMoveIntent::Down };
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);

    /**
     * @tc.steps: Move the handles and selection left.
     *            Verify the selection data.
     */
    pattern_->HandleSetSelection(start, end, false);
    pattern_->HandleSelect(select[0]);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, end - 1);

    /**
     * @tc.steps: Move the handles and selection right.
     *            Verify the selection data.
     */
    pattern_->HandleSetSelection(start, end, false);
    pattern_->HandleSelect(select[1]);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, end + 1);
}

/**
 * @tc.name: OnHandleMove002
 * @tc.desc: Test the clip board interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnHandleMove002, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Initialize text input and Move the handles and then do handle selection.
     */
    int32_t start = 5;
    int32_t end = 10;
    std::vector<CaretMoveIntent> select = { CaretMoveIntent::Left, CaretMoveIntent::Right, CaretMoveIntent::Up,
        CaretMoveIntent::Down };
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);

    /**
     * @tc.steps: Move the handles and selection up.
     *            Verify the selection data.
     */
    EXPECT_FALSE(pattern_->IsTextArea());
    pattern_->HandleSetSelection(start, end, false);
    pattern_->HandleSelect(select[2]);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, end);

    /**
     * @tc.steps: Move the handles and selection down.
     *            Verify the selection data.
     */
    EXPECT_FALSE(pattern_->IsTextArea());
    pattern_->HandleSetSelection(start, end, false);
    pattern_->HandleSelect(select[3]);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, end);
}

/**
 * @tc.name: OnHandleMove003
 * @tc.desc: Test the clip board interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnHandleMove003, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Initialize text input and Move the handles and then do handle selection.
     */
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);

    /**
     * @tc.steps: Move the handles and selection left word.
     *            Verify the selection data.
     */
    auto textFiledController = pattern_->GetTextFieldController();
    textFiledController->CaretPosition(5);
    pattern_->HandleSelectionLeftWord();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, 5);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 0);

    /**
     * @tc.steps: Move the handles and selection right word.
     *            Verify the selection data.
     */
    pattern_->HandleSelectionRightWord();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, 5);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 26);
}

/**
 * @tc.name: OnHandleMove004
 * @tc.desc: Test the clip board interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnHandleMove004, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Initialize text input and Move the handles and then do handle selection.
     */
    std::vector<CaretMoveIntent> select = { CaretMoveIntent::LeftWord, CaretMoveIntent::RightWord,
        CaretMoveIntent::Home, CaretMoveIntent::End };
    CreateTextField(DEFAULT_PLACE_HOLDER, DEFAULT_PLACE_HOLDER);

    /**
     * @tc.steps: Move the handles and selection leftword "please".
     *            Verify the selection data.
     */
    pattern_->HandleSetSelection(6, 6, false);
    pattern_->HandleSelect(select[0]);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, 6);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 0);

    /**
     * @tc.steps: Move the handles and selection rightword "input".
     *            Verify the selection data.
     */
    pattern_->HandleSetSelection(7, 7, false);
    pattern_->HandleSelect(select[1]);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, 7);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 13);
}

/**
 * @tc.name: OnHandleMove005
 * @tc.desc: Test the clip board interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnHandleMove005, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Initialize text input and Move the handles and then do handle selection.
     */
    int32_t start = 6;
    int32_t end = 6;
    std::vector<CaretMoveIntent> select = { CaretMoveIntent::LeftWord, CaretMoveIntent::RightWord,
        CaretMoveIntent::Home, CaretMoveIntent::End };
    CreateTextField(DEFAULT_PLACE_HOLDER, DEFAULT_PLACE_HOLDER);

    /**
     * @tc.steps: Move the handles and selection home.
     *            Verify the selection data.
     */
    pattern_->HandleSetSelection(start, end, false);
    pattern_->HandleSelect(select[2]);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 0);

    /**
     * @tc.steps: Move the handles and selection end.
     *            Verify the selection data.
     */
    pattern_->HandleSetSelection(start, end, false);
    pattern_->HandleSelect(select[3]);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 22);
}

/**
 * @tc.name: CursonMoveLeftTest001
 * @tc.desc: Test the curson move left
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursonMoveLeftTest001, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize insert text and expected values
     */
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);
    GetFocus();
    auto ret = pattern_->CursorMoveLeft();
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: In a situation where no text is selected, the movement is successfull
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length() - 1);

    /**
     * @tc.steps: In a situation where text is selected, the movement is successful
     */
    pattern_->HandleSetSelection(5, 5, false);
    FlushLayoutTask(frameNode_);
    ret = pattern_->CursorMoveLeft();

    /**
     * @tc.expected: The cursor moves to the position after the selected text is deleted
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 4)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursonMoveLeftWordTest001
 * @tc.desc: Test the curson move left word
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursonMoveLeftWordTest001, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text input node and default text
     */
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);
    GetFocus();

    auto ret = pattern_->CursorMoveLeftWord();

    /**
     * @tc.expected: In a situation where no text is selected, the movement is successfull
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 0)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: In a situation where text is selected, the movement is successful
     */
    pattern_->HandleSetSelection(3, 5, false);
    FlushLayoutTask(frameNode_);
    ret = pattern_->CursorMoveLeftWord();

    /**
     * @tc.expected: The cursor moves to the position after the selected text is deleted
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 0)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveLineBeginTest001
 * @tc.desc: Test the cursor move line begin
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveLineBeginTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize empty text and CursorMoveLineBegin
     */
    CreateTextField();
    GetFocus();
    auto ret = pattern_->CursorMoveLineBegin();
    EXPECT_TRUE(ret);

    /**
     * @tc.steps: step2. Insert text and move line begin
     */
    pattern_->InsertValue("hello world");
    FlushLayoutTask(frameNode_);
    ret = pattern_->CursorMoveLineBegin();

    /**
     * @tc.expected: Cursor move to the line head
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 0)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: In a situation where text is all selected, the movement is successful
     */
    pattern_->HandleSetSelection(0, 11, false);
    FlushLayoutTask(frameNode_);
    ret = pattern_->CursorMoveLineBegin();

    /**
     * @tc.expected: Cursor move to the line head
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 0)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveToParagraphBeginTest001
 * @tc.desc: Test the cursor move paragraph begin
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveToParagraphBeginTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize empty text and cursor move to paragraph begin
     */
    CreateTextField();
    GetFocus();
    auto ret = pattern_->CursorMoveToParagraphBegin();
    EXPECT_TRUE(ret);

    /**
     * @tc.steps: step2. Insert text
     */
    pattern_->InsertValue("hello world");
    FlushLayoutTask(frameNode_);
    ret = pattern_->CursorMoveToParagraphBegin();

    /**
     * @tc.expected: Cursor move to the line head
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 0)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveHomeTest001
 * @tc.desc: Test the cursor move home
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveHomeTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize empty text and curson move home
     */
    CreateTextField();
    GetFocus();
    auto ret = pattern_->CursorMoveHome();
    EXPECT_TRUE(ret);

    /**
     * @tc.steps: step2. Insert text
     */
    pattern_->InsertValue("hello world");
    FlushLayoutTask(frameNode_);
    ret = pattern_->CursorMoveHome();

    /**
     * @tc.expected: Cursor move to the line head
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 0)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveRightTest001
 * @tc.desc: Test the cursor move right
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveRightTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize default text and curson move right
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();
    auto ret = pattern_->CursorMoveRight();

    /**
     * @tc.expected: Unable to move
     */
    EXPECT_FALSE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 26)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: step2. Move cursor back to the line head and move right
     */
    ret = pattern_->CursorMoveLineBegin();
    ret = pattern_->CursorMoveRight();

    /**
     * @tc.expected: In a situation where no text is selected, the movement is successfull
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 1)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: step3. Select the text within coordinates 3 to 5 and move cursor right
     */
    pattern_->HandleSetSelection(5, 5, false);
    FlushLayoutTask(frameNode_);
    ret = pattern_->CursorMoveRight();

    /**
     * @tc.expected: Select from 3 to 5, move the cursor to 6.
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 6)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveRightWordTest001
 * @tc.desc: Test the cursor move right word
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveRightWordTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize default text and curson move right
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();
    auto ret = pattern_->CursorMoveRightWord();

    /**
     * @tc.expected: Moving to the right character when there is initial text
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length())
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: steps2. Move to the left 2 times first.
     */
    ret = pattern_->CursorMoveLeft();
    ret = pattern_->CursorMoveLeft();
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected:  the current text length - 2
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length() - 2)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: steps3. Continue moving to the right word.
     */
    ret = pattern_->CursorMoveRightWord();

    /**
     * @tc.expected: Moving to the right character when there is initial text
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length())
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: step4. Move to the beginning of the line and select all text.
     */
    ret = pattern_->CursorMoveLineBegin();
    pattern_->HandleSetSelection(0, DEFAULT_TEXT.length(), false);
    FlushLayoutTask(frameNode_);
    ret = pattern_->CursorMoveRightWord();

    /**
     * @tc.expected: Moving to the right character when there is initial text
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length())
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveLineEndTest001
 * @tc.desc: Test the cursor move line end
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveLineEndTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize  text and move to the end of the line
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();
    auto ret = pattern_->CursorMoveLineEnd();

    /**
     * @tc.expected: Moving to the right character when there is initial text
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length())
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: step2. Move to the beginning of the line and select all text
     */
    pattern_->HandleSetSelection(0, DEFAULT_TEXT.length(), false);
    FlushLayoutTask(frameNode_);
    ret = pattern_->CursorMoveLineEnd();

    /**
     * @tc.expected: Moving to the right character when there is initial text
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length())
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveToParagraphEndTest001
 * @tc.desc: Test the cursor move to pragraph to the end
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveToParagraphEndTest001, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text and move to the pargraph of the line
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();
    auto ret = pattern_->CursorMoveToParagraphEnd();
    EXPECT_TRUE(ret);

    /**
     * @tc.expected: Moving to the paragraph end and check if cursor is on pargraph end
     */
    ret = pattern_->CursorMoveLeft();
    ret = pattern_->CursorMoveToParagraphEnd();
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length())
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveEndTest001
 * @tc.desc: Test the cursor move end
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveEndTest001, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text and move to the pargraph of the line
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();
    auto ret = pattern_->CursorMoveToParagraphEnd();
    EXPECT_TRUE(ret);

    /**
     * @tc.expected: Move left once first, and then move to the end
     *               Check if the cursor is at the end of the text.
     */
    ret = pattern_->CursorMoveLeft();
    ret = pattern_->CursorMoveEnd();
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length());
}

/**
 * @tc.name: CursorMoveUpTest001
 * @tc.desc: Test the cursor move up
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveUpTest001, TestSize.Level1)
{
    /**
     * @tc.steps: Move up and down in a single-line text
     * @tc.expected: In single-line text, there is no up and down movement
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();
    auto ret = pattern_->CursorMoveUp();
    EXPECT_FALSE(ret);
    ret = pattern_->CursorMoveDown();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetLeftTextOfCursor001
 * @tc.desc: Test get text of left cursor
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, GetLeftTextOfCursor001, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Create default text and retrieve the left 5 characters before the cursor
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.expected: Check if it equals "vwxyz"
     */
    EXPECT_EQ(StringUtils::Str16ToStr8(pattern_->GetLeftTextOfCursor(5)), "vwxyz");

    /**
     * @tc.steps: step2. Select the text from position 3 to 5
     */
    pattern_->HandleSetSelection(3, 5, false);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if it equals "vwxyz"
     */
    EXPECT_EQ(StringUtils::Str16ToStr8(pattern_->GetLeftTextOfCursor(5)), "abc");
}

/**
 * @tc.name: GetRightTextOfCursor001
 * @tc.desc: Test get text of left cursor
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, GetRightTextOfCursor001, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Create default text and retrieve the left 5 characters before the cursor
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.expected: Check if it equals "vwxyz"
     */
    EXPECT_EQ(StringUtils::Str16ToStr8(pattern_->GetRightTextOfCursor(5)), "");

    /**
     * @tc.steps: step2. Select the text from position 3 to 5
     */
    pattern_->HandleSetSelection(3, 5, false);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if it equals "vwxyz"
     */
    EXPECT_EQ(StringUtils::Str16ToStr8(pattern_->GetRightTextOfCursor(5)), "fghij");
}

/**
 * @tc.name: SetPreviewTextOperation001
 * @tc.desc: Test set preview text.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, SetPreviewTextOperation001, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.expected: Current caret position is end of text
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size()));

    /**
     * @tc.steps: Set caretPosition and call SetPreviewTextOperation
     */
    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(0);
    pattern_->SetPreviewTextOperation(PREVIEW_ONE);

    /**
     * @tc.expected: Check if the text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue(), "niabcdefghijklmnopqrstuvwxyz");
    EXPECT_EQ(controller->GetCaretIndex(), 2);

    /**
     * @tc.steps:step2 continue call SetPreviewTextOperation
     */
    pattern_->SetPreviewTextOperation(PREVIEW_TWO);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue(), "你abcdefghijklmnopqrstuvwxyz");
    EXPECT_EQ(controller->GetCaretIndex(), 1);
    pattern_->FinishTextPreview();
}
/**
 * @tc.name: SetPreviewTextOperation002
 * @tc.desc: Test set preview text.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, SetPreviewTextOperation002, TestSize.Level1)
{
        /**
     * @tc.steps: Create Text filed node with default text and placeholder
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.expected: Current caret position is end of text
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size()));

    /**
     * @tc.steps: Set caretPosition and call SetPreviewTextOperation
     */
    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(0);
    pattern_->SetPreviewTextOperation(PREVIEW_THR);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue(), "hellofghijklmnopqrstuvwxyz");
    EXPECT_EQ(controller->GetCaretIndex(), 5);

    /**
     * @tc.steps:step2 continue call SetPreviewTextOperation
     */
    pattern_->SetPreviewTextOperation(PREVIEW_FOR);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.expected: Check if the text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue(), "abllofghijklmnopqrstuvwxyz");
    EXPECT_EQ(controller->GetCaretIndex(), 2);
}
/**
 * @tc.name: CheckPreviewTextValidate001
 * @tc.desc: Test CheckPreviewTextValidate before set preview text.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CheckPreviewTextValidate001, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.expected: Current caret position is end of text
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size()));

    /**
     * @tc.expected: call CheckPreviewTextValidate and check return true
     */
    EXPECT_EQ(pattern_->CheckPreviewTextValidate(PREVIEW_THR.text, PREVIEW_THR.range), 0);

    /**
     * @tc.steps:call invalid CheckPreviewTextValidate
     * @tc.expected: check return false
     */
    EXPECT_EQ(pattern_->CheckPreviewTextValidate(PREVIEW_BAD_DATA.text, PREVIEW_BAD_DATA.range), -1);

    /**
     * @tc.steps:Set select and call CheckPreviewTextValidate
     * @tc.expected: check return false
     */
    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(5);
    pattern_->HandleSelectionLeftWord();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->CheckPreviewTextValidate(PREVIEW_THR.text, PREVIEW_THR.range), -1);
}

/**
 * @tc.name: NeedDrawPreviewText001
 * @tc.desc: Test NeedDrawPreviewText before set preview text.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, NeedDrawPreviewText001, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.expected: Current caret position is end of text
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size()));

    /**
     * @tc.expected: call NeedDrawPreviewText and check return false
     */
    EXPECT_FALSE(pattern_->NeedDrawPreviewText());

    /**
     * @tc.steps:Set caretPosition and call NeedDrawPreviewText001
     * @tc.expected: check return true
     */
    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(5);
    pattern_->SetPreviewTextOperation(PREVIEW_ONE);
    EXPECT_TRUE(pattern_->NeedDrawPreviewText());
    FlushLayoutTask(frameNode_);
}

/**
 * @tc.name: FinishTextPreview001
 * @tc.desc: Test FinishTextPreview after set preview text.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, FinishTextPreview001, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.expected: Current caret position is end of text
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size()));

    /**
     * @tc.steps:Set caretPosition and call SetPreviewTextOperation
     * @tc.expected: check GetIsPreviewText return true
     */
    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(5);
    pattern_->SetPreviewTextOperation(PREVIEW_ONE);
    EXPECT_TRUE(pattern_->GetIsPreviewText());
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: call InitEditingValueText value is ""
     * @tc.expected: check GetIsPreviewText return false
     */
    pattern_->InitEditingValueText("");
    EXPECT_FALSE(pattern_->GetIsPreviewText());
    FlushLayoutTask(frameNode_);
}

/**
 * @tc.name: FinishTextPreview002
 * @tc.desc: Test FinishTextPreview after set preview text.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, FinishTextPreview002, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.expected: Current caret position is end of text
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size()));

    /**
     * @tc.steps:Set caretPosition and call SetPreviewTextOperation
     * @tc.expected: check GetIsPreviewText return true
     */
    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(5);
    pattern_->SetPreviewTextOperation(PREVIEW_ONE);
    EXPECT_TRUE(pattern_->GetIsPreviewText());
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: call HandleBlurEvent
     * @tc.expected: check GetIsPreviewText return false
     */
    pattern_->HandleBlurEvent();
    EXPECT_FALSE(pattern_->GetIsPreviewText());
    FlushLayoutTask(frameNode_);
}

/**
 * @tc.name: FinishTextPreview003
 * @tc.desc: Test FinishTextPreview after set preview text.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, FinishTextPreview003, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.expected: Current caret position is end of text
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size()));

    /**
     * @tc.steps:Set caretPosition and call SetPreviewTextOperation
     * @tc.expected: check GetIsPreviewText return true
     */
    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(5);
    pattern_->SetPreviewTextOperation(PREVIEW_ONE);
    EXPECT_TRUE(pattern_->GetIsPreviewText());
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: call InsertValueOperation
     * @tc.expected: check GetIsPreviewText return false
     */
    SourceAndValueInfo info;
    info.insertValue = HELLO_TEXT;
    pattern_->InsertValueOperation(info);
    EXPECT_TRUE(pattern_->inputOperations_.front() == InputOperation::SET_PREVIEW_FINISH);
    FlushLayoutTask(frameNode_);
}

/**
 * @tc.name: FinishTextPreviewOperation001
 * @tc.desc: Test FinishTextPreview after set preview text.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, FinishTextPreviewOperation001, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.expected: Current caret position is end of text
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size()));

    /**
     * @tc.steps:Set caretPosition and call SetPreviewTextOperation
     * @tc.expected: check GetIsPreviewText return true
     */
    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(5);
    pattern_->SetPreviewTextOperation(PREVIEW_ONE);
    EXPECT_TRUE(pattern_->GetIsPreviewText());
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: call FinishTextPreviewOperation
     * @tc.expected: check GetIsPreviewText return false
     */
    pattern_->FinishTextPreviewOperation();
    EXPECT_FALSE(pattern_->GetIsPreviewText());
    FlushLayoutTask(frameNode_);
}

/**
 * @tc.name: TextInputLineBreakStrategy001
 * @tc.desc: test testInput text lineBreakStrategy
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextInputLineBreakStrategy001, TestSize.Level1)
{
    /**
     * @tc.step1: Create Text filed node
     * @tc.expected: style is Inline
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetInputStyle(DEFAULT_INPUT_STYLE);
    });

    /**
     * @tc.step: step2. Set lineBreakStrategy GREEDY
     */
    layoutProperty_->UpdateLineBreakStrategy(LineBreakStrategy::GREEDY);
    frameNode_->MarkModifyDone();
    EXPECT_EQ(layoutProperty_->GetLineBreakStrategy(), LineBreakStrategy::GREEDY);

    /**
     * @tc.step: step3. Set lineBreakStrategy HIGH_QUALITY
     */
    layoutProperty_->UpdateLineBreakStrategy(LineBreakStrategy::HIGH_QUALITY);
    frameNode_->MarkModifyDone();
    EXPECT_EQ(layoutProperty_->GetLineBreakStrategy(), LineBreakStrategy::HIGH_QUALITY);

    /**
     * @tc.step: step4. Set lineBreakStrategy BALANCED
     */
    layoutProperty_->UpdateLineBreakStrategy(LineBreakStrategy::BALANCED);
    frameNode_->MarkModifyDone();
    EXPECT_EQ(layoutProperty_->GetLineBreakStrategy(), LineBreakStrategy::BALANCED);
}

HWTEST_F(TextInputCursorTest, OnFocusNodeChange_001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create target node.
     */
    CreateTextField();
    auto textFieldNode_1 = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    auto textFieldNode_2 = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode_1->SetParent(frameNode_);
    textFieldNode_2->SetParent(frameNode_);

    auto focusHub_1 = textFieldNode_1->GetFocusHub();
    ASSERT_NE(focusHub_1, nullptr);
    RefPtr<TextFieldPattern> pattern_1 = textFieldNode_1->GetPattern<TextFieldPattern>();
    auto focusHub_2 = textFieldNode_2->GetFocusHub();
    RefPtr<TextFieldPattern> pattern_2 = textFieldNode_2->GetPattern<TextFieldPattern>();

    focusHub_1->currentFocus_ = true;
    pattern_1->HandleFocusEvent();

    FlushLayoutTask(frameNode_);
    FlushLayoutTask(textFieldNode_1);
    FlushLayoutTask(textFieldNode_2);
    EXPECT_TRUE(pattern_1->needToRequestKeyboardInner_);
    EXPECT_FALSE(pattern_2->needToRequestKeyboardInner_);

    focusHub_1->currentFocus_ = false;
    focusHub_2->currentFocus_ = true;
    pattern_1->HandleBlurEvent();
    pattern_2->HandleFocusEvent();
    FlushLayoutTask(frameNode_);
    EXPECT_FALSE(pattern_1->needToRequestKeyboardInner_);
    EXPECT_TRUE(pattern_2->needToRequestKeyboardInner_);
}

/**
 * @tc.name: TextPattern001
 * @tc.desc: Test TextPattern UpdateCaretRect
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->obscuredChange_ = false;
    pattern->UpdateCaretRect(true);
}

/**
 * @tc.name: TextPattern002
 * @tc.desc: Test TextPattern CheckSelectAreaVisible
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern ->CheckSelectAreaVisible();
}

/**
 * @tc.name: TextPattern003
 * @tc.desc: Test TextPattern OnIsFocusActiveUpdate
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->OnIsFocusActiveUpdate(true);
    pattern->OnIsFocusActiveUpdate(false);
}

/**
 * @tc.name: TextPattern004
 * @tc.desc: Test TextPattern HandleExtendAction
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->HandleExtendAction(ACTION_AUTOFILL);
}

/**
 * @tc.name: TextPattern005
 * @tc.desc: Test TextPattern HandleBlurEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->isCustomKeyboardAttached_ = true;
    auto func1 = ViewStackProcessor::GetInstance()->GetMainFrameNode();;
    auto func2 = [] {};
    pattern->customKeyboard_ = AceType::Claim<UINode>(func1);
    pattern->customKeyboardBuilder_ = func2;
    pattern->HandleBlurEvent();
}

/**
 * @tc.name: TextPattern006
 * @tc.desc: Test TextPattern HandleOnUndoAction
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->HandleOnUndoAction();
    TextEditingValueNG record {
        .text = pattern->contentController_->GetTextValue(),
        .caretPosition = pattern->selectController_->GetCaretIndex(),
    };
    pattern->operationRecords_.emplace_back(record);
    pattern->HandleOnUndoAction();
}

/**
 * @tc.name: TextPattern007
 * @tc.desc: Test TextPattern HandleOnRedoAction
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->HandleOnRedoAction();
    TextEditingValueNG record {
        .text = pattern->contentController_->GetTextValue(),
        .caretPosition = pattern->selectController_->GetCaretIndex(),
    };
    pattern->operationRecords_.emplace_back(record);
    auto textEditingValue = pattern->operationRecords_.back();
    pattern->redoOperationRecords_.push_back(textEditingValue);
    pattern->HandleOnRedoAction();
}

/**
 * @tc.name: TextPattern008
 * @tc.desc: Test TextPattern CheckAutoFillType
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->CheckAutoFillType(AceAutoFillType::ACE_UNSPECIFIED, true);
}

/**
 * @tc.name: TextPattern009
 * @tc.desc: Test TextPattern InitDragDropCallBack
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->dragStatus_ = DragStatus::ON_DROP;
    pattern->InitDragDropCallBack();
}

/**
 * @tc.name: TextPattern010
 * @tc.desc: Test TextPattern InitDragDropCallBack
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->dragStatus_ = DragStatus::DRAGGING;
    pattern->isDetachFromMainTree_ = false;
    pattern->InitDragDropCallBack();
}

/**
 * @tc.name: TextPattern011
 * @tc.desc: Test TextPattern InitDragDropCallBack
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->dragStatus_ = DragStatus::DRAGGING;
    pattern->isDetachFromMainTree_ = true;
    pattern->InitDragDropCallBack();
}

/**
 * @tc.name: TextPattern012
 * @tc.desc: Test TextPattern HandleDoubleClickEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    GestureEvent info;
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->hasPreviewText_ = true;
    pattern->HandleDoubleClickEvent(info);
}

/**
 * @tc.name: TextPattern013
 * @tc.desc: Test TextPattern GetAutoFillTriggeredStateByType
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->GetAutoFillTriggeredStateByType(AceAutoFillType::ACE_USER_NAME);
    pattern->GetAutoFillTriggeredStateByType(AceAutoFillType::ACE_NEW_PASSWORD);
    pattern->GetAutoFillTriggeredStateByType(AceAutoFillType::ACE_PASSWORD);
    pattern->GetAutoFillTriggeredStateByType(AceAutoFillType::ACE_FULL_STREET_ADDRESS);
}

/**
 * @tc.name: TextPattern014
 * @tc.desc: Test TextPattern SetAutoFillTriggeredStateByType
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->SetAutoFillTriggeredStateByType(AceAutoFillType::ACE_USER_NAME);
    pattern->SetAutoFillTriggeredStateByType(AceAutoFillType::ACE_PASSWORD);
    pattern->SetAutoFillTriggeredStateByType(AceAutoFillType::ACE_NEW_PASSWORD);
}

/**
 * @tc.name: TextPattern015
 * @tc.desc: Test TextPattern HandleTripleClickEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern015, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    GestureEvent info1;
    auto textFieldNode1 = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    RefPtr<TextFieldPattern> pattern1 = textFieldNode1->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern1, nullptr);
    pattern1->hasPreviewText_ = true;
    pattern1->HandleTripleClickEvent(info1);

    GestureEvent info2;
    auto textFieldNode2 = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    RefPtr<TextFieldPattern> pattern2 = textFieldNode2->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern2, nullptr);
    pattern2->hasPreviewText_ = false;
    pattern2->showSelect_ = false;
    pattern2->HandleTripleClickEvent(info2);
}

/**
 * @tc.name: TextPattern016
 * @tc.desc: Test TextPattern HandleSingleClickEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern016, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    GestureEvent info;
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->mouseStatus_ = MouseStatus::MOVE;
    pattern->HandleSingleClickEvent(info);
}

/**
 * @tc.name: TextPattern017
 * @tc.desc: Test TextPattern StartTwinkling
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern017, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->isTransparent_ = true;
    pattern->StartTwinkling();
}

/**
 * @tc.name: TextPattern018
 * @tc.desc: Test TextPattern FilterInitializeText
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern018, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    textFieldNode->SetParent(frameNode_);
    ASSERT_NE(textFieldNode, nullptr);

    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->deleteBackwardOperations_.emplace(10);
    pattern->FilterInitializeText();
}

/**
 * @tc.name: TextPattern019
 * @tc.desc: Test TextPattern HandleRightMouseReleaseEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern019, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    MouseInfo info1;
    auto textFieldNode1 = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    RefPtr<TextFieldPattern> pattern1 = textFieldNode1->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern1, nullptr);
    pattern1->hasPreviewText_ = true;
    pattern1->HandleRightMouseReleaseEvent(info1);

    MouseInfo info2;
    auto textFieldNode2 = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    RefPtr<TextFieldPattern> pattern2 = textFieldNode2->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern2, nullptr);
    pattern2->hasPreviewText_ = false;
    pattern2->HandleRightMouseReleaseEvent(info2);
}

/**
 * @tc.name: TextPattern020
 * @tc.desc: Test TextPattern HandleLeftMouseMoveEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern020, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    MouseInfo info1;
    auto textFieldNode1 = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    RefPtr<TextFieldPattern> pattern1 = textFieldNode1->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern1, nullptr);
    pattern1->leftMouseCanMove_ = true;
    pattern1->blockPress_ = true;
    pattern1->HandleLeftMouseMoveEvent(info1);

    MouseInfo info2;
    auto textFieldNode2 = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    RefPtr<TextFieldPattern> pattern2 = textFieldNode2->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern2, nullptr);
    pattern2->leftMouseCanMove_ = true;
    pattern2->blockPress_ = false;
    pattern2->HandleLeftMouseMoveEvent(info2);
}

/**
 * @tc.name: TextPattern021
 * @tc.desc: Test TextPattern HandleLeftMouseReleaseEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern021, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    MouseInfo info1;
    auto textFieldNode1 = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    RefPtr<TextFieldPattern> pattern1 = textFieldNode1->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern1, nullptr);
    pattern1->blockPress_ = true;
    pattern1->mouseStatus_ = MouseStatus::PRESSED;
    pattern1->HandleLeftMouseReleaseEvent(info1);

    MouseInfo info2;
    auto textFieldNode2 = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    RefPtr<TextFieldPattern> pattern2 = textFieldNode2->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern2, nullptr);
    pattern2->blockPress_ = true;
    pattern2->mouseStatus_ = MouseStatus::NONE;
    pattern2->HandleLeftMouseReleaseEvent(info2);
}

/**
 * @tc.name: TextPattern022
 * @tc.desc: Test TextPattern RequestKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern022, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->showKeyBoardOnFocus_ = true;
    pattern->RequestKeyboard(false, true, true);
    pattern->showKeyBoardOnFocus_ = false;
    auto func1 = ViewStackProcessor::GetInstance()->GetMainFrameNode();;
    pattern->customKeyboard_ = AceType::Claim<UINode>(func1);
    pattern->RequestKeyboard(false, true, true);
}

/**
 * @tc.name: TextPattern023
 * @tc.desc: Test TextPattern CloseKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern023, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto func1 = ViewStackProcessor::GetInstance()->GetMainFrameNode();;
    auto func2 = [] {};
    pattern->customKeyboard_ = AceType::Claim<UINode>(func1);
    pattern->customKeyboardBuilder_ = func2;
    pattern->showKeyBoardOnFocus_ = true;
    pattern->CloseKeyboard(true, false);
    pattern->isCustomKeyboardAttached_ = true;
    pattern->CloseKeyboard(true, false);
}

/**
 * @tc.name: TextPattern024
 * @tc.desc: Test TextPattern AutoFillValueChanged
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern024, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->isFillRequestFinish_ = true;
    pattern->AutoFillValueChanged();
}

/**
 * @tc.name: TextPattern025
 * @tc.desc: Test TextPattern HandleLongPress
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern025, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    GestureEvent info;
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->hasPreviewText_ = true;
    pattern->HandleLongPress(info);
}

/**
 * @tc.name: TextPattern026
 * @tc.desc: Test TextPattern InitEditingValueText
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern026, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    GestureEvent info;
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->deleteBackwardOperations_.emplace(10);
    pattern->InitEditingValueText("");
}

/**
 * @tc.name: TextPattern027
 * @tc.desc: Test TextPattern InitValueText
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern027, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    GestureEvent info;
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->hasPreviewText_ = true;
    pattern->deleteBackwardOperations_.emplace(10);
    pattern->InitValueText("");
    pattern->InitValueText("123");
}

/**
 * @tc.name: TextPattern028
 * @tc.desc: Test TextPattern HandleRightMouseEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern028, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    MouseInfo info;
    info.SetAction(OHOS::Ace::MouseAction::RELEASE);
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->hasPreviewText_ = true;
    pattern->HandleRightMouseEvent(info);
}

/**
 * @tc.name: TextPattern029
 * @tc.desc: Test TextPattern HandleRightMousePressEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern029, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    MouseInfo info;
    info.SetAction(OHOS::Ace::MouseAction::RELEASE);
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->hasPreviewText_ = true;
    pattern->HandleRightMousePressEvent(info);
}

/**
 * @tc.name: TextPattern030
 * @tc.desc: Test TextPattern GetWordLength
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern030, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->GetWordLength(10, 10);
}

/**
 * @tc.name: TextPattern031
 * @tc.desc: Test TextPattern GetLineEndPosition
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern031, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->GetLineEndPosition(10, true);
}

/**
 * @tc.name: TextPattern032
 * @tc.desc: Test TextPattern GetLineBeginPosition
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern032, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->GetLineBeginPosition(10, true);
}

/**
 * @tc.name: TextPattern033
 * @tc.desc: Test TextPattern CursorMoveLeftOperation
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern033, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    pattern->CursorMoveLeftOperation();
}

/**
 * @tc.name: TextPattern034
 * @tc.desc: Test TextPattern CursorMoveLeftWord
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern034, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectController_->caretInfo_.index = -1;
    pattern->CursorMoveLeftWord();
}

/**
 * @tc.name: TextPattern035
 * @tc.desc: Test TextPattern CursorMoveLineBegin
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern035, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::TEXT;
    pattern->selectController_->firstHandleInfo_.index = 1;
    pattern->selectController_->secondHandleInfo_.index = 2;
    pattern->CursorMoveRightOperation();
}

/**
 * @tc.name: TextPattern036
 * @tc.desc: Test TextPattern CursorMoveLineBegin
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern036, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::TEXT;
    pattern->contentController_->SetTextValue("");
    pattern->selectController_->caretInfo_.index = 2;
    pattern->CursorMoveRightWord();
    pattern->contentController_->SetTextValue("123");
    pattern->selectController_->caretInfo_.index = 0;
    pattern->selectController_->firstHandleInfo_.index = 0;
    pattern->selectController_->secondHandleInfo_.index = 3;
    pattern->CursorMoveRightWord();
}

/**
 * @tc.name: TextPattern037
 * @tc.desc: Test TextPattern CursorMoveLineEnd
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern037, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->contentController_->SetTextValue("123");
    pattern->selectController_->caretInfo_.index = 0;
    pattern->selectController_->firstHandleInfo_.index = 0;
    pattern->selectController_->secondHandleInfo_.index = 3;
    pattern->CursorMoveLineEnd();
}

/**
 * @tc.name: TextPattern038
 * @tc.desc: Test TextPattern HandleCounterBorder
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern038, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->showCountBorderStyle_ = true;
    pattern->HandleCounterBorder();
}

/**
 * @tc.name: TextPattern039
 * @tc.desc: Test TextPattern PerformAction
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern039, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    pattern->PerformAction(TextInputAction::NEW_LINE, false);
    pattern->focusIndex_ = FocuseIndex::UNIT;
    pattern->PerformAction(TextInputAction::NEW_LINE, false);
    pattern->textInputBlurOnSubmit_ = false;
    pattern->focusIndex_ = FocuseIndex::TEXT;
    pattern->PerformAction(TextInputAction::DONE, false);
}

/**
 * @tc.name: TextPattern040
 * @tc.desc: Test TextPattern OnHandleAreaChanged
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern040, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->parentGlobalOffset_ = pattern->GetPaintRectGlobalOffset();
    pattern->OnHandleAreaChanged();
    pattern->parentGlobalOffset_ = OffsetF(1.0f, 1.0f);
    pattern->OnHandleAreaChanged();
}

/**
 * @tc.name: TextPattern041
 * @tc.desc: Test TextPattern OnVisibleChange
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern041, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->GetFocusHub()->currentFocus_ = true;
    pattern->OnVisibleChange(false);
}

/**
 * @tc.name: TextPattern042
 * @tc.desc: Test TextPattern OnVisibleChange
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern042, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->hasPreviewText_ = true;
    pattern->DeleteBackwardOperation(1);
    pattern->DeleteForwardOperation(1);
}

/**
 * @tc.name: TextPattern043
 * @tc.desc: Test TextPattern HandleSelectionLeft
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern043, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectController_->caretInfo_.index = 0;
    pattern->HandleSelectionLeft();
}

/**
 * @tc.name: TextPattern044
 * @tc.desc: Test TextPattern HandleSelectionLeftWord
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern044, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectController_->caretInfo_.index = 0;
    pattern->HandleSelectionLeftWord();
    pattern->selectController_->caretInfo_.index = -1;
    pattern->HandleSelectionLeftWord();
    pattern->selectController_->caretInfo_.index = 1;
    pattern->selectController_->firstHandleInfo_.index = 1;
    pattern->selectController_->secondHandleInfo_.index = 2;
    pattern->HandleSelectionLeftWord();
}

/**
 * @tc.name: TextPattern045
 * @tc.desc: Test TextPattern HandleSelectionLineBegin
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern045, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectController_->caretInfo_.index = 0;
    pattern->HandleSelectionLineBegin();
    pattern->selectController_->caretInfo_.index = 1;
    pattern->selectController_->firstHandleInfo_.index = 1;
    pattern->selectController_->secondHandleInfo_.index = 2;
    pattern->HandleSelectionLineBegin();
}

/**
 * @tc.name: TextPattern046
 * @tc.desc: Test TextPattern HandleSelectionHome
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern046, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectController_->caretInfo_.index = 0;
    pattern->HandleSelectionHome();
    pattern->selectController_->caretInfo_.index = 1;
    pattern->selectController_->firstHandleInfo_.index = 1;
    pattern->selectController_->secondHandleInfo_.index = 2;
    pattern->HandleSelectionHome();
}

/**
 * @tc.name: TextPattern047
 * @tc.desc: Test TextPattern HandleSelectionRight
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern047, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectController_->caretInfo_.index = 1;
    pattern->contentController_->SetTextValue("");
    pattern->HandleSelectionRight();
}

/**
 * @tc.name: TextPattern048
 * @tc.desc: Test TextPattern HandleSelectionRightWord
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern048, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectController_->caretInfo_.index = 1;
    pattern->contentController_->SetTextValue("1");
    pattern->HandleSelectionRightWord();
    pattern->selectController_->caretInfo_.index = 4;
    pattern->contentController_->SetTextValue("1");
    pattern->HandleSelectionRightWord();
}

/**
 * @tc.name: TextPattern049
 * @tc.desc: Test TextPattern HandleSelectionLineEnd
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern049, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectController_->caretInfo_.index = 1;
    pattern->contentController_->SetTextValue("1");
    pattern->HandleSelectionLineEnd();
    pattern->selectController_->caretInfo_.index = 0;
    pattern->contentController_->SetTextValue("1");
    pattern->HandleSelectionLineEnd();
}

/**
 * @tc.name: TextPattern050
 * @tc.desc: Test TextPattern HandleSelectionEnd
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern050, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectController_->caretInfo_.index = 1;
    pattern->contentController_->SetTextValue("1");
    pattern->HandleSelectionEnd();
    pattern->selectController_->caretInfo_.index = 4;
    pattern->contentController_->SetTextValue("1");
    pattern->selectController_->firstHandleInfo_.index = 1;
    pattern->selectController_->secondHandleInfo_.index = 2;
    pattern->HandleSelectionEnd();
}

/**
 * @tc.name: TextPattern050
 * @tc.desc: Test TextPattern SetCaretPosition
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern051, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->GetFocusHub()->currentFocus_ = true;
    pattern->magnifierController_->isShowMagnifier_ = true;
    pattern->SetCaretPosition(0);
}

/**
 * @tc.name: TextPattern052
 * @tc.desc: Test TextPattern SetSelectionFlag
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern052, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->GetFocusHub()->currentFocus_ = true;
    pattern->hasPreviewText_ = true;
    pattern->SetSelectionFlag(1, 10, std::nullopt, true);
}

/**
 * @tc.name: TextPattern053
 * @tc.desc: Test TextPattern OnScrollCallback
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, TextPattern053, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->OnScrollCallback(1.0f, SCROLL_FROM_START);
}
} // namespace OHOS::Ace::NG