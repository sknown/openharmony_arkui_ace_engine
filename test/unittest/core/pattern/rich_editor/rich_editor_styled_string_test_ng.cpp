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
#include "test/unittest/core/pattern/rich_editor/rich_editor_common_test_ng.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::string INIT_STRING_1 = "初始属性字符串";
const std::string INIT_STRING_2 = "Hellow World";
const std::string INIT_STRING_3 = "123456";
const std::string TEST_IMAGE_SOURCE = "src/image.png";
const int32_t TEST_MAX_LINE = 10;
const Dimension TEST_BASELINE_OFFSET = Dimension(5, DimensionUnit::PX);
const Dimension TEST_TEXT_INDENT = Dimension(20, DimensionUnit::PX);
const CalcLength TEST_MARGIN_CALC { 10.0, DimensionUnit::CALC };
const CalcLength TEST_PADDING_CALC { 5.0, DimensionUnit::CALC };
const ImageSpanSize TEST_IMAGE_SIZE = { .width = 50.0_vp, .height = 50.0_vp };
const BorderRadiusProperty TEST_BORDER_RADIUS = { 4.0_vp, 4.0_vp, 4.0_vp, 4.0_vp };
const LeadingMarginSize TEST_LEADING_MARGIN_SIZE = { Dimension(5.0), Dimension(10.0) };
const LeadingMargin TEST_LEADING_MARGIN = { .size = TEST_LEADING_MARGIN_SIZE };
const Font TEST_FONT = { FONT_WEIGHT_BOLD, FONT_SIZE_VALUE, ITALIC_FONT_STYLE_VALUE, FONT_FAMILY_VALUE,
    OHOS::Ace::Color::RED, FONT_FAMILY_VALUE};
const SpanParagraphStyle TEST_PARAGRAPH_STYLE = { TextAlign::END, TEST_MAX_LINE, WordBreak::BREAK_ALL,
    TextOverflow::ELLIPSIS, TEST_LEADING_MARGIN, TEST_TEXT_INDENT};
StyledStringChangeValue onStyledStringWillChangeValue;
StyledStringChangeValue onStyledStringDidChangeValue;
} // namespace

class RichEditorStyledStringTestNg : public RichEditorCommonTestNg {
public:
    void SetUp() override;
    void TearDown() override;
    static void TearDownTestSuite();
    RefPtr<MutableSpanString> CreateTextStyledString(const std::string& content);
    RefPtr<MutableSpanString> CreateImageStyledString();
    RefPtr<MutableSpanString> CreateCustomSpanStyledString();
    void SetTypingStyle();
};

void RichEditorStyledStringTestNg::SetUp()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    richEditorNode_ = FrameNode::GetOrCreateFrameNode(
        V2::RICH_EDITOR_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<RichEditorPattern>(); });
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    richEditorPattern->InitScrollablePattern();
    richEditorPattern->SetSpanStringMode(true);
    richEditorPattern->SetRichEditorStyledStringController(AceType::MakeRefPtr<RichEditorStyledStringController>());
    richEditorPattern->GetRichEditorStyledStringController()->SetPattern(WeakPtr(richEditorPattern));
    richEditorPattern->CreateNodePaintMethod();
    richEditorNode_->GetGeometryNode()->SetContentSize({});
}

void RichEditorStyledStringTestNg::TearDown()
{
    richEditorNode_ = nullptr;
    MockParagraph::TearDown();
}

void RichEditorStyledStringTestNg::TearDownTestSuite()
{
    TestNG::TearDownTestSuite();
}

RefPtr<MutableSpanString> RichEditorStyledStringTestNg::CreateTextStyledString(const std::string& content)
{
    auto styledString = AceType::MakeRefPtr<MutableSpanString>(content);
    auto length = styledString->GetLength();
    styledString->AddSpan(AceType::MakeRefPtr<FontSpan>(TEST_FONT, 0, length));
    styledString->AddSpan(AceType::MakeRefPtr<DecorationSpan>(TEXT_DECORATION_VALUE, TEXT_DECORATION_COLOR_VALUE,
        TextDecorationStyle::WAVY, 0, length));
    styledString->AddSpan(AceType::MakeRefPtr<BaselineOffsetSpan>(TEST_BASELINE_OFFSET, 0, length));
    styledString->AddSpan(AceType::MakeRefPtr<LetterSpacingSpan>(LETTER_SPACING, 0, length));
    styledString->AddSpan(AceType::MakeRefPtr<TextShadowSpan>(SHADOWS, 0, length));
    styledString->AddSpan(AceType::MakeRefPtr<ParagraphStyleSpan>(TEST_PARAGRAPH_STYLE, 0, length));
    styledString->AddSpan(AceType::MakeRefPtr<LineHeightSpan>(LINE_HEIGHT_VALUE, 0, length));
    return styledString;
}

RefPtr<MutableSpanString> RichEditorStyledStringTestNg::CreateImageStyledString()
{
    MarginProperty margins;
    margins.SetEdges(TEST_MARGIN_CALC);
    PaddingProperty paddings;
    paddings.SetEdges(TEST_PADDING_CALC);
    ImageSpanAttribute attr { .size = TEST_IMAGE_SIZE,
        .paddingProp = paddings,
        .marginProp = margins,
        .borderRadius = TEST_BORDER_RADIUS,
        .objectFit = ImageFit::COVER,
        .verticalAlign = VerticalAlign::BOTTOM };
    ImageSpanOptions imageOption { .image = TEST_IMAGE_SOURCE, .imageAttribute = attr };
    return AceType::MakeRefPtr<MutableSpanString>(imageOption);
}

RefPtr<MutableSpanString> RichEditorStyledStringTestNg::CreateCustomSpanStyledString()
{
    auto customSpan = AceType::MakeRefPtr<CustomSpan>();
    return AceType::MakeRefPtr<MutableSpanString>(customSpan);
}

void RichEditorStyledStringTestNg::SetTypingStyle()
{
    TextStyle textStyle;
    textStyle.SetTextColor(TEXT_COLOR_VALUE);
    textStyle.SetTextShadows(SHADOWS);
    textStyle.SetFontSize(FONT_SIZE_VALUE);
    textStyle.SetFontStyle(ITALIC_FONT_STYLE_VALUE);
    textStyle.SetFontWeight(FONT_WEIGHT_VALUE);
    textStyle.SetTextDecoration(TEXT_DECORATION_VALUE);
    textStyle.SetTextDecorationColor(TEXT_DECORATION_COLOR_VALUE);
    textStyle.SetLineHeight(LINE_HEIGHT_VALUE);
    textStyle.SetLetterSpacing(LETTER_SPACING);
    UpdateSpanStyle typingStyle;
    typingStyle.updateTextColor = TEXT_COLOR_VALUE;
    typingStyle.updateTextShadows = SHADOWS;
    typingStyle.updateFontSize = FONT_SIZE_VALUE;
    typingStyle.updateItalicFontStyle = ITALIC_FONT_STYLE_VALUE;
    typingStyle.updateFontWeight = FONT_WEIGHT_VALUE;
    typingStyle.updateTextDecoration = TEXT_DECORATION_VALUE;
    typingStyle.updateTextDecorationColor = TEXT_DECORATION_COLOR_VALUE;
    typingStyle.updateLineHeight = LINE_HEIGHT_VALUE;
    typingStyle.updateLetterSpacing = LETTER_SPACING;
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    richEditorPattern->SetTypingStyle(typingStyle, textStyle);
}

/**
 * @tc.name: RichEditorModel001
 * @tc.desc: Test events not supported in styledString mode.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, RichEditorModel001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create richEditorModel in styledString mode
     */
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    richEditorModel.isStyledStringMode_ = true;
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto eventHub = richEditorPattern->GetEventHub<RichEditorEventHub>();
    ASSERT_NE(eventHub, nullptr);

    /**
     * @tc.steps: step2. init events not supported in styledString mode
     */
    auto onSelectFunc = [](const BaseEventInfo* info) {};
    richEditorModel.SetOnSelect(std::move(onSelectFunc));
    auto aboutToIMEInputFunc = [](const RichEditorInsertValue&) { return true; };
    richEditorModel.SetAboutToIMEInput(std::move(aboutToIMEInputFunc));
    auto iMEInputCompleteFunc = [](const RichEditorAbstractSpanResult&) {};
    richEditorModel.SetOnIMEInputComplete(std::move(iMEInputCompleteFunc));
    auto aboutToDeleteFunc = [](const RichEditorDeleteValue&) { return true; };
    richEditorModel.SetAboutToDelete(std::move(aboutToDeleteFunc));
    auto deleteCompleteFunc = []() {};
    richEditorModel.SetOnDeleteComplete(std::move(deleteCompleteFunc));
    auto onWillChange = [](const RichEditorChangeValue& beforeResult) { return false; };
    richEditorModel.SetOnWillChange(std::move(onWillChange));
    auto onDidChange = [](const RichEditorChangeValue& afterResult) {};
    richEditorModel.SetOnDidChange(std::move(onDidChange));

    /**
     * @tc.steps: step3. verify whether the event settings have failed
     */
    EXPECT_FALSE(eventHub->onSelect_);
    EXPECT_FALSE(eventHub->aboutToIMEInput_);
    EXPECT_FALSE(eventHub->onIMEIputComplete_);
    EXPECT_FALSE(eventHub->aboutToDelete_);
    EXPECT_FALSE(eventHub->onDeleteComplete_);
    EXPECT_FALSE(eventHub->onWillChange_);
    EXPECT_FALSE(eventHub->onDidChange_);
}

/**
 * @tc.name: StyledStringController001
 * @tc.desc: Test SetStyledString.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringController001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with text
     */
    auto mutableStr = CreateTextStyledString(INIT_STRING_1);

    /**
     * @tc.steps: step2. get richEditor styledString controller
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto styledStringController = richEditorPattern->GetRichEditorStyledStringController();
    ASSERT_NE(styledStringController, nullptr);

    /**
     * @tc.steps: step3. set styledString
     */
    styledStringController->SetStyledString(mutableStr);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 7);
    EXPECT_EQ(richEditorPattern->dataDetectorAdapter_->textForAI_, INIT_STRING_1);
    auto spanItem = richEditorPattern->spans_.front();
    auto& fontStyle = spanItem->fontStyle;
    ASSERT_NE(fontStyle, nullptr);
    EXPECT_EQ(fontStyle->GetFontWeight(), FONT_WEIGHT_BOLD);
    EXPECT_EQ(fontStyle->GetFontSize(), FONT_SIZE_VALUE);
    EXPECT_EQ(fontStyle->GetItalicFontStyle(), ITALIC_FONT_STYLE_VALUE);
    EXPECT_EQ(fontStyle->GetFontFamily(), FONT_FAMILY_VALUE);
    EXPECT_EQ(fontStyle->GetTextColor(), OHOS::Ace::Color::RED);
    EXPECT_EQ(fontStyle->GetTextDecoration(), TEXT_DECORATION_VALUE);
    EXPECT_EQ(fontStyle->GetTextDecorationColor(), TEXT_DECORATION_COLOR_VALUE);
    EXPECT_EQ(fontStyle->GetTextDecorationStyle(), TextDecorationStyle::WAVY);
    EXPECT_EQ(fontStyle->GetLetterSpacing(), LETTER_SPACING);
    EXPECT_EQ(fontStyle->GetTextShadow(), SHADOWS);

    auto& textLineStyle = spanItem->textLineStyle;
    EXPECT_EQ(textLineStyle->GetBaselineOffset(), TEST_BASELINE_OFFSET);
    EXPECT_EQ(textLineStyle->GetTextAlign(), TextAlign::END);
    EXPECT_EQ(textLineStyle->GetMaxLines(), TEST_MAX_LINE);
    EXPECT_EQ(textLineStyle->GetTextOverflow(), TextOverflow::ELLIPSIS);
    EXPECT_EQ(textLineStyle->GetLeadingMargin(), TEST_LEADING_MARGIN);
    EXPECT_EQ(textLineStyle->GetWordBreak(), WordBreak::BREAK_ALL);
    EXPECT_EQ(textLineStyle->GetTextIndent(), TEST_TEXT_INDENT);
    EXPECT_EQ(textLineStyle->GetLineHeight(), LINE_HEIGHT_VALUE);
}

/**
 * @tc.name: StyledStringController002
 * @tc.desc: Test SetStyledString.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringController002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with image
     */
    auto mutableStr = CreateImageStyledString();

    /**
     * @tc.steps: step2. get richEditor styledString controller
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto styledStringController = richEditorPattern->GetRichEditorStyledStringController();
    ASSERT_NE(styledStringController, nullptr);

    /**
     * @tc.steps: step3. set styledString
     */
    styledStringController->SetStyledString(mutableStr);
    EXPECT_EQ(static_cast<int32_t>(richEditorNode_->GetChildren().size()), 1);
    auto imageSpanItem = AceType::DynamicCast<ImageSpanItem>(richEditorPattern->spans_.front());
    ASSERT_NE(imageSpanItem, nullptr);
    auto imageSpanoptions = imageSpanItem->options;
    auto imageAttr = imageSpanoptions.imageAttribute.value();
    EXPECT_EQ(imageAttr.size->GetSize(), TEST_IMAGE_SIZE.GetSize());
    EXPECT_EQ(imageAttr.verticalAlign.value(), VerticalAlign::BOTTOM);
    EXPECT_EQ(imageAttr.objectFit.value(), ImageFit::COVER);
    auto& padding = imageAttr.paddingProp.value();
    PaddingProperty paddings;
    paddings.SetEdges(TEST_PADDING_CALC);
    EXPECT_EQ(padding.ToString(), paddings.ToString());
    auto& margin = imageAttr.marginProp.value();
    MarginProperty margins;
    margins.SetEdges(TEST_MARGIN_CALC);
    EXPECT_EQ(margin.ToString(), margins.ToString());
    EXPECT_EQ(imageAttr.borderRadius.value(), TEST_BORDER_RADIUS);
}

/**
 * @tc.name: StyledStringController003
 * @tc.desc: Test SetStyledString.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringController003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with customSpan
     */
    auto mutableStr = CreateCustomSpanStyledString();

    /**
     * @tc.steps: step2. get richEditor styledString controller
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto styledStringController = richEditorPattern->GetRichEditorStyledStringController();
    ASSERT_NE(styledStringController, nullptr);

    /**
     * @tc.steps: step3. set styledString
     */
    styledStringController->SetStyledString(mutableStr);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 1);
    auto customSpanItem = AceType::DynamicCast<CustomSpanItem>(richEditorPattern->spans_.front());
    EXPECT_NE(customSpanItem, nullptr);
}

/**
 * @tc.name: StyledStringController004
 * @tc.desc: Test SetStyledString.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringController004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with customSpan、image and text
     */
    auto mutableStr = CreateCustomSpanStyledString();
    auto mutableTextStr = CreateTextStyledString(INIT_STRING_1);
    auto mutableImageStr = CreateImageStyledString();
    mutableStr->AppendSpanString(mutableTextStr);
    mutableStr->InsertSpanString(3, mutableImageStr);

    /**
     * @tc.steps: step2. get richEditor styledString controller
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto styledStringController = richEditorPattern->GetRichEditorStyledStringController();
    ASSERT_NE(styledStringController, nullptr);

    /**
     * @tc.steps: step3. set styledString
     */
    styledStringController->SetStyledString(mutableStr);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 9);
    EXPECT_EQ(static_cast<int32_t>(richEditorNode_->GetChildren().size()), 1);
    auto customSpanItem = AceType::DynamicCast<CustomSpanItem>(richEditorPattern->spans_.front());
    EXPECT_NE(customSpanItem, nullptr);
}

/**
 * @tc.name: StyledStringController005
 * @tc.desc: Test GetStyledString.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringController005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with customSpan、image and text
     */
    auto mutableStr = CreateCustomSpanStyledString();
    auto mutableTextStr = CreateTextStyledString(INIT_STRING_1);
    auto mutableImageStr = CreateImageStyledString();
    mutableStr->AppendSpanString(mutableTextStr);
    mutableStr->InsertSpanString(3, mutableImageStr);

    /**
     * @tc.steps: step2. get richEditor styledString controller and set styledString
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto styledStringController = richEditorPattern->GetRichEditorStyledStringController();
    ASSERT_NE(styledStringController, nullptr);
    styledStringController->SetStyledString(mutableStr);

    /**
     * @tc.steps: step3. get styledString
     */
    auto styledString = AceType::DynamicCast<MutableSpanString>(styledStringController->GetStyledString());
    ASSERT_NE(styledString, nullptr);
    EXPECT_EQ(styledString->GetString(), richEditorPattern->styledString_->GetString());
    richEditorPattern->caretPosition_ = 5;
    richEditorPattern->InsertValue(INIT_STRING_2);
    styledString = AceType::DynamicCast<MutableSpanString>(styledStringController->GetStyledString());
    ASSERT_NE(styledString, nullptr);
    EXPECT_EQ(styledString->GetString(), richEditorPattern->styledString_->GetString());
    richEditorPattern->caretPosition_ = 10;
    richEditorPattern->DeleteBackward(1);
    styledString = AceType::DynamicCast<MutableSpanString>(styledStringController->GetStyledString());
    ASSERT_NE(styledString, nullptr);
    EXPECT_EQ(styledString->GetString(), richEditorPattern->styledString_->GetString());
    richEditorPattern->caretPosition_ = 3;
    richEditorPattern->DeleteBackward(1);
    styledString = AceType::DynamicCast<MutableSpanString>(styledStringController->GetStyledString());
    ASSERT_NE(styledString, nullptr);
    EXPECT_EQ(styledString->GetString(), richEditorPattern->styledString_->GetString());
    richEditorPattern->caretPosition_ = 1;
    richEditorPattern->DeleteBackward(1);
    styledString = AceType::DynamicCast<MutableSpanString>(styledStringController->GetStyledString());
    ASSERT_NE(styledString, nullptr);
    EXPECT_EQ(styledString->GetString(), richEditorPattern->styledString_->GetString());
}

/**
 * @tc.name: StyledStringController006
 * @tc.desc: Test OnWillChange.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringController006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with text
     */
    auto mutableStr = CreateTextStyledString(INIT_STRING_1);

    /**
     * @tc.steps: step2. get richEditor styledString controller and set styledString
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto styledStringController = richEditorPattern->GetRichEditorStyledStringController();
    ASSERT_NE(styledStringController, nullptr);
    styledStringController->SetStyledString(mutableStr);

    /**
     * @tc.steps: step3. SetOnWillChange
     */
    auto onWillChange = [](const StyledStringChangeValue& changeValue) {
        onStyledStringWillChangeValue = changeValue;
        return true;
    };
    styledStringController->SetOnWillChange(onWillChange);

    /**
     * @tc.steps: step3. addition、deletion and substitution in styledString
     */
    std::string replacementString;
    richEditorPattern->caretPosition_ = 5;
    richEditorPattern->InsertValue(INIT_STRING_3);
    EXPECT_EQ(onStyledStringWillChangeValue.GetRangeBefore().start, 5);
    EXPECT_EQ(onStyledStringWillChangeValue.GetRangeBefore().end, 5);
    replacementString =
        AceType::DynamicCast<MutableSpanString>(onStyledStringWillChangeValue.GetReplacementString())->GetString();
    EXPECT_EQ(replacementString, INIT_STRING_3);

    richEditorPattern->caretPosition_ = 13;
    richEditorPattern->DeleteBackward(6);
    EXPECT_EQ(onStyledStringWillChangeValue.GetRangeBefore().start, 7);
    EXPECT_EQ(onStyledStringWillChangeValue.GetRangeBefore().end, 13);
    replacementString =
        AceType::DynamicCast<MutableSpanString>(onStyledStringWillChangeValue.GetReplacementString())->GetString();
    EXPECT_TRUE(replacementString.empty());

    richEditorPattern->textSelector_.Update(3, 4);
    richEditorPattern->InsertValue(INIT_STRING_3);
    EXPECT_EQ(onStyledStringWillChangeValue.GetRangeBefore().start, 3);
    EXPECT_EQ(onStyledStringWillChangeValue.GetRangeBefore().end, 4);
    replacementString =
        AceType::DynamicCast<MutableSpanString>(onStyledStringWillChangeValue.GetReplacementString())->GetString();
    EXPECT_EQ(replacementString, INIT_STRING_3);
}

/**
 * @tc.name: StyledStringController007
 * @tc.desc: Test OnDidChange.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringController007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with text
     */
    auto mutableStr = CreateTextStyledString(INIT_STRING_1);

    /**
     * @tc.steps: step2. get richEditor styledString controller and set styledString
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto styledStringController = richEditorPattern->GetRichEditorStyledStringController();
    ASSERT_NE(styledStringController, nullptr);
    styledStringController->SetStyledString(mutableStr);

    /**
     * @tc.steps: step3. SetOnDidChange
     */
    auto onDidChange = [](const StyledStringChangeValue& changeValue) {
        onStyledStringDidChangeValue = changeValue;
    };
    styledStringController->SetOnDidChange(onDidChange);

    /**
     * @tc.steps: step3. addition、deletion and substitution in styledString
     */
    richEditorPattern->caretPosition_ = 5;
    richEditorPattern->InsertValue(INIT_STRING_3);
    EXPECT_EQ(onStyledStringDidChangeValue.GetRangeBefore().start, 5);
    EXPECT_EQ(onStyledStringDidChangeValue.GetRangeBefore().end, 5);
    EXPECT_EQ(onStyledStringDidChangeValue.GetRangeAfter().start, 5);
    EXPECT_EQ(onStyledStringDidChangeValue.GetRangeAfter().end, 11);

    richEditorPattern->caretPosition_ = 5;
    richEditorPattern->ResetSelection();
    richEditorPattern->DeleteBackward(1);
    EXPECT_EQ(onStyledStringDidChangeValue.GetRangeBefore().start, 4);
    EXPECT_EQ(onStyledStringDidChangeValue.GetRangeBefore().end, 5);
    EXPECT_EQ(onStyledStringDidChangeValue.GetRangeAfter().start, 4);
    EXPECT_EQ(onStyledStringDidChangeValue.GetRangeAfter().end, 4);

    richEditorPattern->textSelector_.Update(3, 4);
    richEditorPattern->InsertValue(INIT_STRING_3);
    EXPECT_EQ(onStyledStringDidChangeValue.GetRangeBefore().start, 3);
    EXPECT_EQ(onStyledStringDidChangeValue.GetRangeBefore().end, 4);
    EXPECT_EQ(onStyledStringDidChangeValue.GetRangeAfter().start, 3);
    EXPECT_EQ(onStyledStringDidChangeValue.GetRangeAfter().end, 9);
}

/**
 * @tc.name: StyledStringController008
 * @tc.desc: Test SetTypingStyle.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringController008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with text
     */
    auto mutableStr = CreateTextStyledString(INIT_STRING_1);

    /**
     * @tc.steps: step2. get richEditor styledString controller and set styledString
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto styledStringController = richEditorPattern->GetRichEditorStyledStringController();
    ASSERT_NE(styledStringController, nullptr);
    styledStringController->SetStyledString(mutableStr);

    /**
     * @tc.steps: step3. set typingStyle
     */
    SetTypingStyle();

    /**
     * @tc.steps: step4. insert value
     */
    richEditorPattern->caretPosition_ = 0;
    richEditorPattern->InsertValue(INIT_STRING_3);
    auto spanItem = richEditorPattern->spans_.front();
    auto& fontStyle = spanItem->fontStyle;
    ASSERT_NE(fontStyle, nullptr);
    EXPECT_EQ(fontStyle->GetFontWeight(), FONT_WEIGHT_VALUE);
    EXPECT_EQ(fontStyle->GetFontSize(), FONT_SIZE_VALUE);
    EXPECT_EQ(fontStyle->GetItalicFontStyle(), ITALIC_FONT_STYLE_VALUE);
    EXPECT_EQ(fontStyle->GetTextColor(), TEXT_COLOR_VALUE);
    EXPECT_EQ(fontStyle->GetTextDecoration(), TEXT_DECORATION_VALUE);
    EXPECT_EQ(fontStyle->GetTextDecorationColor(), TEXT_DECORATION_COLOR_VALUE);
    EXPECT_EQ(fontStyle->GetLetterSpacing(), LETTER_SPACING);
    EXPECT_EQ(fontStyle->GetTextShadow(), SHADOWS);
}

/**
 * @tc.name: StyledStringInsertValue001
 * @tc.desc: Test insert value in styledString mode.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringInsertValue001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with customSpan、image and text
     */
    auto mutableStr = CreateCustomSpanStyledString();
    auto mutableTextStr = CreateTextStyledString(INIT_STRING_1);
    auto mutableImageStr = CreateImageStyledString();
    mutableStr->AppendSpanString(mutableTextStr);
    mutableStr->InsertSpanString(3, mutableImageStr);

    /**
     * @tc.steps: step2. set styledString
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->SetStyledString(mutableStr);

    /**
     * @tc.steps: step3. insert value
     */
    richEditorPattern->caretPosition_ = 1;
    richEditorPattern->InsertValue(INIT_STRING_3);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 15);

    richEditorPattern->caretPosition_ = 15;
    richEditorPattern->InsertValue(INIT_STRING_2);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 27);

    richEditorPattern->caretPosition_ = 10;
    richEditorPattern->InsertValue(INIT_STRING_1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 34);
}

/**
 * @tc.name: StyledStringInsertValue002
 * @tc.desc: Test insert value in styledString mode.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringInsertValue002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with customSpan、image and text
     */
    auto mutableStr = CreateCustomSpanStyledString();
    auto mutableTextStr = CreateTextStyledString(INIT_STRING_1);
    auto mutableImageStr = CreateImageStyledString();
    mutableStr->AppendSpanString(mutableTextStr);
    mutableStr->InsertSpanString(3, mutableImageStr);

    /**
     * @tc.steps: step2. set styledString
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->SetStyledString(mutableStr);

    /**
     * @tc.steps: step3. insert value
     */
    richEditorPattern->textSelector_.Update(0, 2);
    richEditorPattern->InsertValue(INIT_STRING_3);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 13);

    richEditorPattern->textSelector_.Update(2, 4);
    richEditorPattern->InsertValue(INIT_STRING_2);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 23);

    richEditorPattern->textSelector_.Update(13, 14);
    richEditorPattern->InsertValue(INIT_STRING_1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 29);
}

/**
 * @tc.name: StyledStringDeleteBackward001
 * @tc.desc: Test delete backward in styledString mode.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringDeleteBackward001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with customSpan、image and text
     */
    auto mutableStr = CreateCustomSpanStyledString();
    auto mutableTextStr = CreateTextStyledString(INIT_STRING_1);
    auto mutableImageStr = CreateImageStyledString();
    mutableStr->AppendSpanString(mutableTextStr);
    mutableStr->InsertSpanString(3, mutableImageStr);

    /**
     * @tc.steps: step2. set styledString
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->SetStyledString(mutableStr);

    /**
     * @tc.steps: step3. delete backward
     */
    richEditorPattern->caretPosition_ = 1;
    richEditorPattern->DeleteBackward(1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 8);

    richEditorPattern->caretPosition_ = 3;
    richEditorPattern->DeleteBackward(1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 7);

    richEditorPattern->caretPosition_ = 2;
    richEditorPattern->DeleteBackward(1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 6);
}

/**
 * @tc.name: StyledStringDeleteBackward002
 * @tc.desc: Test delete backward in styledString mode.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringDeleteBackward002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with customSpan、image and text
     */
    auto mutableStr = CreateCustomSpanStyledString();
    auto mutableTextStr = CreateTextStyledString(INIT_STRING_1);
    auto mutableImageStr = CreateImageStyledString();
    mutableStr->AppendSpanString(mutableTextStr);
    mutableStr->InsertSpanString(3, mutableImageStr);

    /**
     * @tc.steps: step2. set styledString
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->SetStyledString(mutableStr);

    /**
     * @tc.steps: step3. delete backward
     */
    richEditorPattern->textSelector_.Update(0, 2);
    richEditorPattern->caretPosition_ = 2;
    richEditorPattern->DeleteBackward(1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 7);

    richEditorPattern->textSelector_.Update(2, 5);
    richEditorPattern->caretPosition_ = 5;
    richEditorPattern->DeleteBackward(1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 4);

    richEditorPattern->textSelector_.Update(1, 3);
    richEditorPattern->caretPosition_ = 3;
    richEditorPattern->DeleteBackward(1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 2);
}

/**
 * @tc.name: StyledStringDeleteForward001
 * @tc.desc: Test delete forward in styledString mode.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringDeleteForward001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with customSpan、image and text
     */
    auto mutableStr = CreateCustomSpanStyledString();
    auto mutableTextStr = CreateTextStyledString(INIT_STRING_1);
    auto mutableImageStr = CreateImageStyledString();
    mutableStr->AppendSpanString(mutableTextStr);
    mutableStr->InsertSpanString(3, mutableImageStr);

    /**
     * @tc.steps: step2. set styledString
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->SetStyledString(mutableStr);

    /**
     * @tc.steps: step3. delete forward
     */
    richEditorPattern->caretPosition_ = 0;
    richEditorPattern->DeleteForward(1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 8);

    richEditorPattern->caretPosition_ = 2;
    richEditorPattern->DeleteForward(1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 7);

    richEditorPattern->caretPosition_ = 1;
    richEditorPattern->DeleteForward(1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 6);
}

/**
 * @tc.name: StyledStringDeleteForward002
 * @tc.desc: Test delete backward in styledString mode.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, StyledStringDeleteForward002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with customSpan、image and text
     */
    auto mutableStr = CreateCustomSpanStyledString();
    auto mutableTextStr = CreateTextStyledString(INIT_STRING_1);
    auto mutableImageStr = CreateImageStyledString();
    mutableStr->AppendSpanString(mutableTextStr);
    mutableStr->InsertSpanString(3, mutableImageStr);

    /**
     * @tc.steps: step2. set styledString
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->SetStyledString(mutableStr);

    /**
     * @tc.steps: step3. delete forward
     */
    richEditorPattern->textSelector_.Update(0, 2);
    richEditorPattern->DeleteForward(1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 7);

    richEditorPattern->textSelector_.Update(2, 5);
    richEditorPattern->DeleteForward(1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 4);

    richEditorPattern->textSelector_.Update(1, 3);
    richEditorPattern->DeleteForward(1);
    EXPECT_EQ(richEditorPattern->GetTextContentLength(), 2);
}

/**
 * @tc.name: CustomSpan001
 * @tc.desc: Test caret and handles with customSpan.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, CustomSpan001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with customSpan and text
     */
    auto mutableStr = CreateCustomSpanStyledString();
    auto mutableTextStr = CreateTextStyledString(INIT_STRING_1);
    mutableStr->AppendSpanString(mutableTextStr);

    /**
     * @tc.steps: step2. set styledString
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->SetStyledString(mutableStr);

    /**
     * @tc.steps: step3. add paragraph
     */
    CaretMetricsF caretMetricsLeft = { OffsetF(0, 0), 50.0f };
    CaretMetricsF caretMetricsRight = { OffsetF(100.f, 0), 50.0f };
    TestParagraphItem paragraphItem = { .start = 0, .end = 8,
        .testCursorItems = { { 0, caretMetricsLeft, caretMetricsLeft}, {1, caretMetricsRight, caretMetricsRight} } };
    AddParagraph(paragraphItem);
    LayoutConstraintF layoutConstraintF { .selfIdealSize = OptionalSizeF(240.f, 60.f) };
    richEditorNode_->Measure(layoutConstraintF);
    richEditorNode_->Layout();

    /**
     * @tc.steps: step4. calculate caret and handles with customSpan
     */
    richEditorPattern->caretPosition_ = 1;
    auto [caretOffset, caretHeight] = richEditorPattern->CalculateCaretOffsetAndHeight();
    EXPECT_EQ(caretOffset, OffsetF(100.f, 0));
    EXPECT_EQ(caretHeight, 50.0f);
    richEditorPattern->textSelector_.Update(0, 1);
    richEditorPattern->CalculateHandleOffsetAndShowOverlay();
    SizeF handlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), 50.f };
    EXPECT_EQ(richEditorPattern->textSelector_.firstHandle, RectF(OffsetF(0, 0), handlePaintSize));
    EXPECT_EQ(richEditorPattern->textSelector_.secondHandle, RectF(OffsetF(100.f, 0), handlePaintSize));
}

/**
 * @tc.name: FromStyledStrign001
 * @tc.desc: Test FromStyledString.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorStyledStringTestNg, FromStyledString001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create styledString with text
     */
    auto mutableStr = CreateTextStyledString(INIT_STRING_1);

    /**
     * @tc.steps: step2. get richEditor styledString controller and set styledString
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto styledStringController = richEditorPattern->GetRichEditorStyledStringController();
    ASSERT_NE(styledStringController, nullptr);
    styledStringController->SetStyledString(mutableStr);

    /**
     * @tc.steps: step3. FromStyledString
     */
    auto info = richEditorPattern->FromStyledString(mutableStr);
    EXPECT_EQ(info.selection_.resultObjects.size(), mutableStr->GetSpanItems().size());
}
} // namespace OHOS::Ace::NG
