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
int32_t testOnReadyEvent = 0;
int32_t testAboutToIMEInput = 0;
int32_t testOnIMEInputComplete = 0;
int32_t testAboutToDelete = 0;
int32_t testOnDeleteComplete = 0;
} // namespace

class RichEditorBaseTestNg : public RichEditorCommonTestNg {
public:
    void SetUp() override;
    void TearDown() override;
    static void TearDownTestSuite();
};

void RichEditorBaseTestNg::SetUp()
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
    richEditorPattern->SetRichEditorController(AceType::MakeRefPtr<RichEditorController>());
    richEditorPattern->GetRichEditorController()->SetPattern(AceType::WeakClaim(AceType::RawPtr(richEditorPattern)));
    richEditorPattern->CreateNodePaintMethod();
    richEditorNode_->GetGeometryNode()->SetContentSize({});
}

void RichEditorBaseTestNg::TearDown()
{
    richEditorNode_ = nullptr;
    testOnReadyEvent = 0;
    testAboutToIMEInput = 0;
    testOnIMEInputComplete = 0;
    testAboutToDelete = 0;
    testOnDeleteComplete = 0;
    MockParagraph::TearDown();
}

void RichEditorBaseTestNg::TearDownTestSuite()
{
    TestNG::TearDownTestSuite();
}

/**
 * @tc.name: RichEditorModel001
 * @tc.desc: test create
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    EXPECT_EQ(static_cast<int32_t>(ViewStackProcessor::GetInstance()->elementsStack_.size()), 1);
    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: RichEditorModel002
 * @tc.desc: test create
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel002, TestSize.Level1)
{
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    richEditorModel.SetDraggable(true);
    EXPECT_TRUE(ViewStackProcessor::GetInstance()->GetMainFrameNode()->draggable_);
    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: RichEditorModel003
 * @tc.desc: test set on ready
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel003, TestSize.Level1)
{
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    auto func = []() { testOnReadyEvent = 1; };
    richEditorModel.SetOnReady(std::move(func));
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto eventHub = richEditorPattern->GetEventHub<RichEditorEventHub>();
    ASSERT_NE(eventHub, nullptr);
    eventHub->FireOnReady();
    EXPECT_EQ(testOnReadyEvent, 1);
    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: RichEditorModel004
 * @tc.desc: test set about to IME input
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel004, TestSize.Level1)
{
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    auto func = [](const RichEditorInsertValue&) {
        testAboutToIMEInput = 1;
        return true;
    };
    richEditorModel.SetAboutToIMEInput(std::move(func));
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto eventHub = richEditorPattern->GetEventHub<RichEditorEventHub>();
    ASSERT_NE(eventHub, nullptr);
    RichEditorInsertValue info;
    eventHub->FireAboutToIMEInput(info);
    EXPECT_EQ(testAboutToIMEInput, 1);
    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: RichEditorModel005
 * @tc.desc: test set on IME input complete
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel005, TestSize.Level1)
{
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    auto func = [](const RichEditorAbstractSpanResult&) { testOnIMEInputComplete = 1; };
    richEditorModel.SetOnIMEInputComplete(std::move(func));
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto eventHub = richEditorPattern->GetEventHub<RichEditorEventHub>();
    ASSERT_NE(eventHub, nullptr);
    RichEditorAbstractSpanResult info;
    eventHub->FireOnIMEInputComplete(info);
    EXPECT_EQ(testOnIMEInputComplete, 1);
    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: RichEditorModel006
 * @tc.desc: test set about to delete
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel006, TestSize.Level1)
{
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    auto func = [](const RichEditorDeleteValue&) {
        testAboutToDelete = 1;
        return true;
    };
    richEditorModel.SetAboutToDelete(std::move(func));
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto eventHub = richEditorPattern->GetEventHub<RichEditorEventHub>();
    ASSERT_NE(eventHub, nullptr);
    RichEditorDeleteValue info;
    eventHub->FireAboutToDelete(info);
    EXPECT_EQ(testAboutToDelete, 1);
    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: RichEditorModel007
 * @tc.desc: test set on delete complete
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel007, TestSize.Level1)
{
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    auto func = []() { testOnDeleteComplete = 1; };
    richEditorModel.SetOnDeleteComplete(std::move(func));
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto eventHub = richEditorPattern->GetEventHub<RichEditorEventHub>();
    ASSERT_NE(eventHub, nullptr);
    eventHub->FireOnDeleteComplete();
    EXPECT_EQ(testOnDeleteComplete, 1);
    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: RichEditorModel009
 * @tc.desc: test set on text selection change
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel009, TestSize.Level1)
{
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    auto func = [](const BaseEventInfo* info) {
        const auto* selectionRange = TypeInfoHelper::DynamicCast<SelectionRangeInfo>(info);
        ASSERT_NE(selectionRange, nullptr);
        testSelectionRange = *selectionRange;
    };
    richEditorModel.SetOnSelectionChange(std::move(func));
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    ClearSpan();
    auto focusHub = richEditorPattern->GetFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();

    // insert value 1
    richEditorPattern->InsertValue(INIT_VALUE_1);
    EXPECT_EQ(testSelectionRange.start_, 6);
    EXPECT_EQ(testSelectionRange.end_, 6);

    // insert value 2
    richEditorPattern->InsertValue(INIT_VALUE_1);
    EXPECT_EQ(testSelectionRange.start_, 12);
    EXPECT_EQ(testSelectionRange.end_, 12);

    // set caret position
    richEditorPattern->SetCaretPosition(3);
    EXPECT_EQ(testSelectionRange.start_, 3);
    EXPECT_EQ(testSelectionRange.end_, 3);

    // update selector
    richEditorPattern->textSelector_.Update(0, 10);
    EXPECT_EQ(testSelectionRange.start_, 0);
    EXPECT_EQ(testSelectionRange.end_, 10);

    // update selector, reverse handle
    richEditorPattern->textSelector_.Update(10, 8);
    EXPECT_EQ(testSelectionRange.start_, 8);
    EXPECT_EQ(testSelectionRange.end_, 10);

    // select all
    richEditorPattern->HandleOnSelectAll();
    EXPECT_EQ(testSelectionRange.start_, 0);
    EXPECT_EQ(testSelectionRange.end_, 12);

    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: RichEditorModel010
 * @tc.desc: test set on text/image/symbol selection change
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel010, TestSize.Level1)
{
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    auto func = [](const BaseEventInfo* info) {
        const auto* selectionRange = TypeInfoHelper::DynamicCast<SelectionRangeInfo>(info);
        ASSERT_NE(selectionRange, nullptr);
        testSelectionRange = *selectionRange;
    };
    richEditorModel.SetOnSelectionChange(std::move(func));
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    ClearSpan();
    auto focusHub = richEditorPattern->GetFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();

    // insert value
    richEditorPattern->InsertValue(INIT_VALUE_1);

    // add image
    ImageSpanOptions imageSpanOptions;
    richEditorPattern->AddImageSpan(imageSpanOptions);
    richEditorPattern->HandleOnSelectAll();
    EXPECT_EQ(testSelectionRange.start_, 0);
    EXPECT_EQ(testSelectionRange.end_, 7);

    // add symbol
    SymbolSpanOptions symbolSpanOptions;
    symbolSpanOptions.symbolId = SYMBOL_ID;
    auto richEditorController =  richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    richEditorController->AddSymbolSpan(symbolSpanOptions);
    richEditorPattern->HandleOnSelectAll();
    EXPECT_EQ(testSelectionRange.start_, 0);
    EXPECT_EQ(testSelectionRange.end_, 9);

    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: RichEditorModel011
 * @tc.desc: test placeholder appear and disappear
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel011, TestSize.Level1)
{
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    PlaceholderOptions options;
    options.value = INIT_VALUE_1;
    richEditorModel.SetPlaceholder(options);

    auto richEditorNode = AceType::Claim(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->SetRichEditorController(AceType::MakeRefPtr<RichEditorController>());
    richEditorPattern->GetRichEditorController()->SetPattern(AceType::WeakClaim(AceType::RawPtr(richEditorPattern)));
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        richEditorNode, AceType::MakeRefPtr<GeometryNode>(), richEditorNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    auto layoutAlgorithm = AceType::DynamicCast<RichEditorLayoutAlgorithm>(richEditorPattern->CreateLayoutAlgorithm());
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));

    // test placeholder appear when there is nothing in richEditor
    layoutAlgorithm->MeasureContent(parentLayoutConstraint, AceType::RawPtr(layoutWrapper));
    auto spanItemChildren = layoutAlgorithm->GetSpans();
    EXPECT_EQ(spanItemChildren.size(), 0);

    // test add Text then placeholder disappear
    TextSpanOptions textOptions;
    textOptions.value = INIT_VALUE_2;
    richEditorController->AddTextSpan(textOptions);
    layoutAlgorithm = AceType::DynamicCast<RichEditorLayoutAlgorithm>(richEditorPattern->CreateLayoutAlgorithm());
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));
    layoutAlgorithm->MeasureContent(parentLayoutConstraint, AceType::RawPtr(layoutWrapper));
    spanItemChildren = layoutAlgorithm->GetSpans();
    EXPECT_EQ(spanItemChildren.size(), 1);
    EXPECT_EQ(spanItemChildren.back()->GetSpanContent(), INIT_VALUE_2);

    // test when richEitor empty again,placeholder Appear again
    RangeOptions rangeoptions;
    richEditorController->DeleteSpans(rangeoptions);
    richEditorPattern->BeforeCreateLayoutWrapper();
    layoutAlgorithm = AceType::DynamicCast<RichEditorLayoutAlgorithm>(richEditorPattern->CreateLayoutAlgorithm());
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));
    layoutAlgorithm->MeasureContent(parentLayoutConstraint, AceType::RawPtr(layoutWrapper));
    spanItemChildren = layoutAlgorithm->GetSpans();
    EXPECT_EQ(spanItemChildren.size(), 0);

    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: RichEditorModel012
 * @tc.desc: test placeholder styel value
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel012, TestSize.Level1)
{
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    PlaceholderOptions options;
    options.value = INIT_VALUE_1;
    options.fontColor = TEXT_COLOR_VALUE;
    options.fontSize = FONT_SIZE_VALUE;
    options.fontStyle = ITALIC_FONT_STYLE_VALUE;
    options.fontWeight = FONT_WEIGHT_VALUE;
    options.fontFamilies = FONT_FAMILY_VALUE;
    richEditorModel.SetPlaceholder(options);

    auto richEditorNode = AceType::Claim(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->SetRichEditorController(AceType::MakeRefPtr<RichEditorController>());
    richEditorPattern->GetRichEditorController()->SetPattern(AceType::WeakClaim(AceType::RawPtr(richEditorPattern)));
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        richEditorNode, AceType::MakeRefPtr<GeometryNode>(), richEditorNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    auto layoutAlgorithm = AceType::DynamicCast<RichEditorLayoutAlgorithm>(richEditorPattern->CreateLayoutAlgorithm());
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));

    // test placeholder value and style is correct
    layoutAlgorithm->MeasureContent(parentLayoutConstraint, AceType::RawPtr(layoutWrapper));
    auto spanItemChildren = layoutAlgorithm->GetSpans();
    EXPECT_EQ(spanItemChildren.size(), 0);

    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: RichEditorModel013
 * @tc.desc: test placeholder styel value
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel013, TestSize.Level1)
{
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    PlaceholderOptions options;
    options.value = INIT_VALUE_1;
    richEditorModel.SetPlaceholder(options);

    auto richEditorNode = AceType::Claim(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->SetRichEditorController(AceType::MakeRefPtr<RichEditorController>());
    richEditorPattern->GetRichEditorController()->SetPattern(AceType::WeakClaim(AceType::RawPtr(richEditorPattern)));
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        richEditorNode, AceType::MakeRefPtr<GeometryNode>(), richEditorNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    auto layoutAlgorithm = AceType::DynamicCast<RichEditorLayoutAlgorithm>(richEditorPattern->CreateLayoutAlgorithm());
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));

    // test placeholder value and style is correct
    layoutAlgorithm->MeasureContent(parentLayoutConstraint, AceType::RawPtr(layoutWrapper));
    auto spanItemChildren = layoutAlgorithm->GetSpans();
    EXPECT_EQ(spanItemChildren.size(), 0);

    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: RichEditorModel014
 * @tc.desc: test paragraph style wordBreak attribute
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel014, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    TextSpanOptions options;
    options.value = INIT_VALUE_1;

    // test paragraph  style wordBreak default value
    richEditorController->AddTextSpan(options);
    auto info = richEditorController->GetParagraphsInfo(1, sizeof(INIT_VALUE_1));
    EXPECT_EQ(static_cast<WordBreak>(info[0].wordBreak), WordBreak::BREAK_WORD);

    // test paragraph style wordBreak value of WordBreak.NORMAL
    struct UpdateParagraphStyle style;
    style.wordBreak = WordBreak::NORMAL;
    richEditorController->UpdateParagraphStyle(1, sizeof(INIT_VALUE_1), style);
    info = richEditorController->GetParagraphsInfo(1, sizeof(INIT_VALUE_1));
    EXPECT_EQ(static_cast<WordBreak>(info[0].wordBreak), WordBreak::NORMAL);

    // test paragraph style wordBreak value of WordBreak.BREAK_ALL
    style.wordBreak = WordBreak::BREAK_ALL;
    richEditorController->UpdateParagraphStyle(1, sizeof(INIT_VALUE_1), style);
    info = richEditorController->GetParagraphsInfo(1, sizeof(INIT_VALUE_1));
    EXPECT_EQ(static_cast<WordBreak>(info[0].wordBreak), WordBreak::BREAK_ALL);
    
    // test paragraph style wordBreak value of WordBreak.BREAK_WORD
    style.wordBreak = WordBreak::BREAK_WORD;
    richEditorController->UpdateParagraphStyle(1, sizeof(INIT_VALUE_1), style);
    info = richEditorController->GetParagraphsInfo(1, sizeof(INIT_VALUE_1));
    EXPECT_EQ(static_cast<WordBreak>(info[0].wordBreak), WordBreak::BREAK_WORD);
}

/**
 * @tc.name: RichEditorModel015
 * @tc.desc: test textstyle Color
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel015, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto richEditorTheme = AceType::MakeRefPtr<RichEditorTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(richEditorTheme));
    richEditorTheme->textStyle_.SetTextColor(DEFAULT_TEXT_COLOR_VALUE);
    richEditorTheme->textStyle_.SetTextDecorationColor(DEFAULT_TEXT_COLOR_VALUE);

    TextSpanOptions textOptions;
    textOptions.value = INIT_VALUE_1;
    richEditorController->AddTextSpan(textOptions);
    auto info1 = richEditorController->GetSpansInfo(1, 3);
    ASSERT_NE(info1.selection_.resultObjects.size(), 0);
    TextStyleResult textStyle1 = info1.selection_.resultObjects.front().textStyle;
    EXPECT_EQ(textStyle1.fontSize, 16);
    EXPECT_EQ(Color::FromString(textStyle1.fontColor), DEFAULT_TEXT_COLOR_VALUE);
    EXPECT_EQ(Color::FromString(textStyle1.decorationColor), DEFAULT_TEXT_COLOR_VALUE);

    ClearSpan();
    richEditorPattern->InsertValue(INIT_VALUE_2);
    auto info2 = richEditorController->GetSpansInfo(1, 2);
    ASSERT_NE(info2.selection_.resultObjects.size(), 0);
    TextStyleResult textStyle2 = info2.selection_.resultObjects.front().textStyle;
    EXPECT_EQ(Color::FromString(textStyle2.fontColor), DEFAULT_TEXT_COLOR_VALUE);
    EXPECT_EQ(Color::FromString(textStyle2.decorationColor), DEFAULT_TEXT_COLOR_VALUE);
}

/**
 * @tc.name: RichEditorModel016
 * @tc.desc: test paragraph style linebreakstrategy attribute
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorModel016, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    TextSpanOptions options;
    options.value = INIT_VALUE_1;

    // test paragraph  style linebreakstrategy default value
    richEditorController->AddTextSpan(options);
    auto info = richEditorController->GetParagraphsInfo(1, sizeof(INIT_VALUE_1));
    EXPECT_EQ(static_cast<LineBreakStrategy>(info[0].lineBreakStrategy), LineBreakStrategy::GREEDY);

    // test paragraph style linebreakstrategy value of LineBreakStrategy.GREEDY
    struct UpdateParagraphStyle style;
    style.lineBreakStrategy = LineBreakStrategy::GREEDY;
    richEditorController->UpdateParagraphStyle(1, sizeof(INIT_VALUE_1), style);
    info = richEditorController->GetParagraphsInfo(1, sizeof(INIT_VALUE_1));
    EXPECT_EQ(static_cast<LineBreakStrategy>(info[0].lineBreakStrategy), LineBreakStrategy::GREEDY);

    // test paragraph style linebreakstrategy value of LineBreakStrategy.HIGH_QUALITY
    style.lineBreakStrategy = LineBreakStrategy::HIGH_QUALITY;
    richEditorController->UpdateParagraphStyle(1, sizeof(INIT_VALUE_1), style);
    info = richEditorController->GetParagraphsInfo(1, sizeof(INIT_VALUE_1));
    EXPECT_EQ(static_cast<LineBreakStrategy>(info[0].lineBreakStrategy), LineBreakStrategy::HIGH_QUALITY);

    // test paragraph style linebreakstrategy value of LineBreakStrategy.BALANCED
    style.lineBreakStrategy = LineBreakStrategy::BALANCED;
    richEditorController->UpdateParagraphStyle(1, sizeof(INIT_VALUE_1), style);
    info = richEditorController->GetParagraphsInfo(1, sizeof(INIT_VALUE_1));
    EXPECT_EQ(static_cast<LineBreakStrategy>(info[0].lineBreakStrategy), LineBreakStrategy::BALANCED);
}

/**
 * @tc.name: RichEditorController001
 * @tc.desc: test add image span
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    ImageSpanAttribute imageStyle;
    AddSpan("test");
    ImageSpanOptions options;
    options.imageAttribute = imageStyle;
    options.image = IMAGE_VALUE;
    options.bundleName = BUNDLE_NAME;
    options.moduleName = MODULE_NAME;
    options.offset = 1;
    auto index1 = richEditorController->AddImageSpan(options);
    EXPECT_EQ(index1, 1);
    options.image = IMAGE_VALUE;
    options.bundleName = BUNDLE_NAME;
    options.moduleName = MODULE_NAME;
    options.offset = 2;
    auto index2 = richEditorController->AddImageSpan(options);
    EXPECT_EQ(index2, 2);

    options.offset = std::nullopt;
    auto index3 = richEditorPattern->AddImageSpan(options, false, 0);
    EXPECT_EQ(index3, 4);

    std::optional<Ace::NG::MarginProperty> marginProp = std::nullopt;
    std::optional<Ace::NG::BorderRadiusProperty> borderRadius = std::nullopt;
    imageStyle.marginProp = marginProp;
    imageStyle.borderRadius = borderRadius;
    options.imageAttribute = imageStyle;
    auto index4 = richEditorPattern->AddImageSpan(options, false, 0);
    EXPECT_EQ(index4, 5);

    marginProp = { CALC_LENGTH_CALC, CALC_LENGTH_CALC, CALC_LENGTH_CALC, CALC_LENGTH_CALC };
    borderRadius = { CALC_TEST, CALC_TEST, CALC_TEST, CALC_TEST };
    imageStyle.marginProp = marginProp;
    imageStyle.borderRadius = borderRadius;
    options.imageAttribute = imageStyle;
    auto index5 = richEditorPattern->AddImageSpan(options, false, 0);
    EXPECT_EQ(index5, 6);

    marginProp = { ERROR_CALC_LENGTH_CALC, ERROR_CALC_LENGTH_CALC, ERROR_CALC_LENGTH_CALC, ERROR_CALC_LENGTH_CALC };
    borderRadius = { ERROR_CALC_TEST, ERROR_CALC_TEST, ERROR_CALC_TEST, ERROR_CALC_TEST };
    imageStyle.marginProp = marginProp;
    imageStyle.borderRadius = borderRadius;
    options.imageAttribute = imageStyle;
    auto index6 = richEditorPattern->AddImageSpan(options, false, -1);
    EXPECT_EQ(index6, 7);
}

/**
 * @tc.name: RichEditorController002
 * @tc.desc: test add text span
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController002, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    TextStyle style;
    style.SetTextColor(TEXT_COLOR_VALUE);
    style.SetFontSize(FONT_SIZE_VALUE);
    style.SetFontStyle(ITALIC_FONT_STYLE_VALUE);
    style.SetFontWeight(FONT_WEIGHT_VALUE);
    style.SetFontFamilies(FONT_FAMILY_VALUE);
    style.SetTextDecoration(TEXT_DECORATION_VALUE);
    style.SetTextDecorationColor(TEXT_DECORATION_COLOR_VALUE);
    TextSpanOptions options;
    options.offset = 1;
    options.value = INIT_VALUE_1;
    options.style = style;
    auto index1 = richEditorController->AddTextSpan(options);
    EXPECT_EQ(index1, 0);
    auto index2 = richEditorController->AddTextSpan(options);
    EXPECT_EQ(index2, 1);
    options.value = "hello\n";
    auto index3 = richEditorController->AddTextSpan(options);
    EXPECT_EQ(index3, 1);
}

/**
 * @tc.name: RichEditorController003
 * @tc.desc: test get caret offset
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController003, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    AddSpan(INIT_VALUE_1);
    richEditorPattern->caretPosition_ = 1;
    auto offset1 = richEditorController->GetCaretOffset();
    EXPECT_EQ(offset1, 1);
    richEditorPattern->caretPosition_ = 2;
    auto offset2 = richEditorController->GetCaretOffset();
    EXPECT_EQ(offset2, 2);
}

/**
 * @tc.name: RichEditorController004
 * @tc.desc: test set caret offset
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController004, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    AddSpan(INIT_VALUE_1);
    richEditorController->SetCaretOffset(2);
    EXPECT_EQ(richEditorPattern->caretPosition_, 2);
    richEditorController->SetCaretOffset(-1);
    EXPECT_EQ(richEditorPattern->caretPosition_, 2);
}

/**
 * @tc.name: RichEditorController005
 * @tc.desc: test update span style
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController005, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    TextStyle textStyle;
    ImageSpanAttribute imageStyle;
    textStyle.SetTextColor(TEXT_COLOR_VALUE);
    textStyle.SetTextShadows(SHADOWS);
    textStyle.SetFontSize(FONT_SIZE_VALUE);
    textStyle.SetFontStyle(ITALIC_FONT_STYLE_VALUE);
    textStyle.SetFontWeight(FONT_WEIGHT_VALUE);
    textStyle.SetFontFamilies(FONT_FAMILY_VALUE);
    textStyle.SetTextDecoration(TEXT_DECORATION_VALUE);
    textStyle.SetTextDecorationColor(TEXT_DECORATION_COLOR_VALUE);
    struct UpdateSpanStyle updateSpanStyle;
    updateSpanStyle.updateTextColor = TEXT_COLOR_VALUE;
    updateSpanStyle.updateTextShadows = SHADOWS;
    updateSpanStyle.updateFontSize = FONT_SIZE_VALUE;
    updateSpanStyle.updateItalicFontStyle = ITALIC_FONT_STYLE_VALUE;
    updateSpanStyle.updateFontWeight = FONT_WEIGHT_VALUE;
    updateSpanStyle.updateFontFamily = FONT_FAMILY_VALUE;
    updateSpanStyle.updateTextDecoration = TEXT_DECORATION_VALUE;
    updateSpanStyle.updateTextDecorationColor = TEXT_DECORATION_COLOR_VALUE;
    richEditorController->SetUpdateSpanStyle(updateSpanStyle);
    richEditorController->UpdateSpanStyle(5, 10, textStyle, imageStyle);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 5);
    auto newSpan1 = AceType::DynamicCast<SpanNode>(richEditorNode_->GetChildAtIndex(1));
    ASSERT_NE(newSpan1, nullptr);
    EXPECT_EQ(newSpan1->GetFontSize(), FONT_SIZE_VALUE);
    EXPECT_EQ(newSpan1->GetTextColor(), TEXT_COLOR_VALUE);
    EXPECT_EQ(newSpan1->GetTextShadow(), SHADOWS);
    EXPECT_EQ(newSpan1->GetItalicFontStyle(), ITALIC_FONT_STYLE_VALUE);
    EXPECT_EQ(newSpan1->GetFontWeight(), FONT_WEIGHT_VALUE);
    EXPECT_EQ(newSpan1->GetFontFamily(), FONT_FAMILY_VALUE);
    EXPECT_EQ(newSpan1->GetTextDecoration(), TEXT_DECORATION_VALUE);
    EXPECT_EQ(newSpan1->GetTextDecorationColor(), TEXT_DECORATION_COLOR_VALUE);
}

/**
 * @tc.name: RichEditorController105
 * @tc.desc: test update span style
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController105, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    TextStyle textStyle;
    ImageSpanAttribute imageStyle;
    textStyle.SetTextColor(TEXT_COLOR_VALUE);
    textStyle.SetTextShadows(SHADOWS);
    textStyle.SetFontSize(FONT_SIZE_VALUE);
    textStyle.SetFontStyle(ITALIC_FONT_STYLE_VALUE);
    textStyle.SetFontWeight(FONT_WEIGHT_VALUE);
    textStyle.SetFontFamilies(FONT_FAMILY_VALUE);
    textStyle.SetTextDecoration(TEXT_DECORATION_VALUE);
    textStyle.SetTextDecorationColor(TEXT_DECORATION_COLOR_VALUE);
    struct UpdateSpanStyle updateSpanStyle;
    updateSpanStyle.updateTextColor = TEXT_COLOR_VALUE;
    updateSpanStyle.updateTextShadows = SHADOWS;
    updateSpanStyle.updateFontSize = FONT_SIZE_VALUE;
    updateSpanStyle.updateItalicFontStyle = ITALIC_FONT_STYLE_VALUE;
    updateSpanStyle.updateFontWeight = FONT_WEIGHT_VALUE;
    updateSpanStyle.updateFontFamily = FONT_FAMILY_VALUE;
    updateSpanStyle.updateTextDecoration = TEXT_DECORATION_VALUE;
    updateSpanStyle.updateTextDecorationColor = TEXT_DECORATION_COLOR_VALUE;
    richEditorController->SetUpdateSpanStyle(updateSpanStyle);
    richEditorController->UpdateSpanStyle(5, 10, textStyle, imageStyle);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 5);
    auto newSpan2 = AceType::DynamicCast<SpanNode>(richEditorNode_->GetChildAtIndex(3));
    ASSERT_NE(newSpan2, nullptr);
    EXPECT_EQ(newSpan2->GetFontSize(), FONT_SIZE_VALUE);
    EXPECT_EQ(newSpan2->GetTextColor(), TEXT_COLOR_VALUE);
    EXPECT_EQ(newSpan2->GetTextShadow(), SHADOWS);
    EXPECT_EQ(newSpan2->GetItalicFontStyle(), ITALIC_FONT_STYLE_VALUE);
    EXPECT_EQ(newSpan2->GetFontWeight(), FONT_WEIGHT_VALUE);
    EXPECT_EQ(newSpan2->GetFontFamily(), FONT_FAMILY_VALUE);
    EXPECT_EQ(newSpan2->GetTextDecoration(), TEXT_DECORATION_VALUE);
    EXPECT_EQ(newSpan2->GetTextDecorationColor(), TEXT_DECORATION_COLOR_VALUE);
}

/**
 * @tc.name: RichEditorController006
 * @tc.desc: test get span info
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController006, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    auto info1 = richEditorController->GetSpansInfo(1, 10);
    EXPECT_EQ(info1.selection_.selection[0], 1);
    EXPECT_EQ(info1.selection_.selection[1], 10);
    EXPECT_EQ(info1.selection_.resultObjects.size(), 3);
    auto info2 = richEditorController->GetSpansInfo(10, 1);
    EXPECT_EQ(info2.selection_.selection[0], 1);
    EXPECT_EQ(info2.selection_.selection[1], 10);
    auto info3 = richEditorController->GetSpansInfo(-1, 10);
    EXPECT_EQ(info3.selection_.selection[0], 0);
    EXPECT_EQ(info3.selection_.selection[1], 10);
    auto info4 = richEditorController->GetSpansInfo(1, -10);
    EXPECT_EQ(info4.selection_.selection[0], 0);
    EXPECT_EQ(info4.selection_.selection[1], 1);
}

/**
 * @tc.name: RichEditorController007
 * @tc.desc: test delete span
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController007, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    RangeOptions option1;
    option1.start = 5;
    option1.end = 10;
    richEditorController->DeleteSpans(option1);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 2);
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    RangeOptions option2;
    option2.start = 10;
    option2.end = 5;
    richEditorController->DeleteSpans(option2);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 2);
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    RangeOptions option3;
    option3.start = -5;
    option3.end = 10;
    richEditorController->DeleteSpans(option3);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 1);
}

/**
 * @tc.name: RichEditorController108
 * @tc.desc: test delete span
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController108, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    RangeOptions option4;
    option4.start = 5;
    option4.end = -10;
    richEditorController->DeleteSpans(option4);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 3);
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    RangeOptions option5;
    richEditorController->DeleteSpans(option5);
    EXPECT_TRUE(richEditorNode_->GetChildren().empty());
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    option5.start = 100;
    option5.end = 10;
    richEditorController->DeleteSpans(option5);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 3);
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    option5.start = 3;
    option5.end = 3;
    richEditorController->DeleteSpans(option5);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 3);
    ClearSpan();
    richEditorController->DeleteSpans(option5);
}

/**
 * @tc.name: OnDirtyLayoutWrapper001
 * @tc.desc: test on dirty layout wrapper
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, OnDirtyLayoutWrapper001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        richEditorNode_, AceType::MakeRefPtr<GeometryNode>(), richEditorNode_->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    auto layoutAlgorithm = richEditorPattern->CreateLayoutAlgorithm();
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));
    DirtySwapConfig config;
    config.skipMeasure = true;
    auto focusHub = richEditorPattern->GetHost()->GetOrCreateFocusHub();
    focusHub->currentFocus_ = true;
    auto ret = richEditorPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_FALSE(ret);
    richEditorPattern->isRichEditorInit_ = true;

    richEditorPattern->textSelector_.baseOffset = -1;
    richEditorPattern->textSelector_.destinationOffset = 2;
    ret = richEditorPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_FALSE(ret);

    richEditorPattern->textSelector_.baseOffset = 0;
    richEditorPattern->textSelector_.destinationOffset = -1;
    ret = richEditorPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CreateImageSourceInfo001
 * @tc.desc: test CreateImageSourceInfo
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, CreateImageSourceInfo001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    ImageSpanOptions info;
    auto ret = richEditorPattern->CreateImageSourceInfo(info);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: PreventDefault001
 * @tc.desc: test PreventDefault001 in ImageSpan and TextSpan
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, PreventDefault001, TestSize.Level1)
{
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    // add imageSpan
    ClearSpan();
    ImageSpanOptions imageSpanOptions;
    GestureEventFunc callback2 = [](GestureEvent& info) {
        info.SetPreventDefault(true);
    };
    imageSpanOptions.userGestureOption.onClick = callback2;
    richEditorController->AddImageSpan(imageSpanOptions);

    /**
     * @tc.steps: step1. Click on imagespan
     */
    GestureEvent info2;
    info2.localLocation_ = Offset(0, 0);
    richEditorPattern->HandleClickEvent(info2);
    EXPECT_FALSE(richEditorPattern->HasFocus());
}

/**
 * @tc.name: NeedSoftKeyboard001
 * @tc.desc: test NeedSoftKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, NeedSoftKeyboard001, TestSize.Level1)
{
    /**
     * @tc.step: step1. Get frameNode and pattern.
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    /**
     * @tc.steps: step2. Test whether rich editor need soft keyboard.
     */
    EXPECT_TRUE(richEditorPattern->NeedSoftKeyboard());
}

/**
 * @tc.name: RichEditorController008
 * @tc.desc: test add builder span
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController008, TestSize.Level1)
{
    auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
    richEditorNode_ = FrameNode::GetOrCreateFrameNode(
        V2::RICH_EDITOR_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<RichEditorPattern>(); });
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->SetRichEditorController(AceType::MakeRefPtr<RichEditorController>());
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    richEditorPattern->GetRichEditorController()->SetPattern(AceType::WeakClaim(AceType::RawPtr(richEditorPattern)));
    AddSpan("test");
    auto builderId1 = ElementRegister::GetInstance()->MakeUniqueId();
    auto builderNode1 = FrameNode::GetOrCreateFrameNode(
        V2::ROW_ETS_TAG, builderId1, []() { return AceType::MakeRefPtr<LinearLayoutPattern>(false); });
    auto index1 = richEditorController->AddPlaceholderSpan(builderNode1, {});
    EXPECT_EQ(index1, 1);
    EXPECT_EQ(static_cast<int32_t>(richEditorNode_->GetChildren().size()), 2);
    auto builderSpanChildren = richEditorNode_->GetChildren();
    ASSERT_NE(static_cast<int32_t>(builderSpanChildren.size()), 0);
    auto builderSpanChild = builderSpanChildren.begin();
    EXPECT_EQ((*builderSpanChild)->GetTag(), "Span");
    ClearSpan();
}

/**
 * @tc.name: RichEditorController009
 * @tc.desc: test add builder span
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController009, TestSize.Level1)
{
    auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
    richEditorNode_ = FrameNode::GetOrCreateFrameNode(
        V2::RICH_EDITOR_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<RichEditorPattern>(); });
    ASSERT_NE(richEditorNode_, nullptr);
    RefPtr<GeometryNode> containerGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(containerGeometryNode == nullptr);
    containerGeometryNode->SetFrameSize(SizeF(CONTAINER_WIDTH, CONTAINER_HEIGHT));
    ASSERT_NE(richEditorNode_->GetLayoutProperty(), nullptr);
    LayoutWrapperNode layoutWrapper =
        LayoutWrapperNode(richEditorNode_, containerGeometryNode, richEditorNode_->GetLayoutProperty());

    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->SetRichEditorController(AceType::MakeRefPtr<RichEditorController>());
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    richEditorPattern->GetRichEditorController()->SetPattern(AceType::WeakClaim(AceType::RawPtr(richEditorPattern)));

    auto builderId1 = ElementRegister::GetInstance()->MakeUniqueId();
    auto builderNode1 = FrameNode::GetOrCreateFrameNode(
        V2::ROW_ETS_TAG, builderId1, []() { return AceType::MakeRefPtr<LinearLayoutPattern>(false); });
    auto index1 = richEditorController->AddPlaceholderSpan(builderNode1, {});
    EXPECT_EQ(index1, 0);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 1);
    auto builderSpanChildren = richEditorNode_->GetChildren();
    ASSERT_NE(static_cast<int32_t>(builderSpanChildren.size()), 0);
    auto builderSpan = builderSpanChildren.begin();
    auto builderSpanChild = AceType::DynamicCast<FrameNode>(*builderSpan);
    ASSERT_NE(builderSpanChild, nullptr);
    EXPECT_EQ(builderSpanChild->GetTag(), V2::PLACEHOLDER_SPAN_ETS_TAG);

    auto richEditorLayoutAlgorithm = richEditorPattern->CreateLayoutAlgorithm();
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(richEditorLayoutAlgorithm));

    auto childLayoutConstraint = layoutWrapper.GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.selfIdealSize = OptionalSizeF(BUILDER_SIZE);

    RefPtr<GeometryNode> builderGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    builderGeometryNode->Reset();
    RefPtr<LayoutWrapperNode> builderLayoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        builderSpanChild, builderGeometryNode, builderSpanChild->GetLayoutProperty());
    EXPECT_FALSE(builderLayoutWrapper == nullptr);
    auto builderPattern = builderSpanChild->GetPattern();
    builderLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    auto firstItemLayoutAlgorithm = builderPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(firstItemLayoutAlgorithm == nullptr);
}

/**
 * @tc.name: RichEditorController010
 * @tc.desc: test add builder span
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController010, TestSize.Level1)
{
    auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
    richEditorNode_ = FrameNode::GetOrCreateFrameNode(
        V2::RICH_EDITOR_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<RichEditorPattern>(); });
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->SetRichEditorController(AceType::MakeRefPtr<RichEditorController>());
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    richEditorPattern->GetRichEditorController()->SetPattern(AceType::WeakClaim(AceType::RawPtr(richEditorPattern)));
    AddSpan("test");
    RefPtr<FrameNode> builderNode1 = nullptr;
    auto index1 = richEditorController->AddPlaceholderSpan(builderNode1, {});
    EXPECT_EQ(index1, 0);
    EXPECT_EQ(static_cast<int32_t>(richEditorNode_->GetChildren().size()), 1);
    ClearSpan();
}

/**
 * @tc.name: RichEditorController011
 * @tc.desc: test add symbol span
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get richEditor controller
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    /**
     * @tc.steps: step2. initalize symbol span properties
     */
    TextStyle style;
    style.SetFontSize(FONT_SIZE_VALUE);
    style.SetFontWeight(FONT_WEIGHT_VALUE);
    style.SetSymbolColorList(SYMBOL_COLOR_LIST_1);
    style.SetRenderStrategy(RENDER_STRATEGY_SINGLE);
    style.SetEffectStrategy(EFFECT_STRATEGY_NONE);
    SymbolSpanOptions options;
    options.symbolId = SYMBOL_ID;
    options.style = style;

    /**
     * @tc.steps: step3. test add symbol span
     */
    auto index1 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index1, 0);
    auto index2 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index2, 1);
    EXPECT_EQ(static_cast<int32_t>(richEditorNode_->GetChildren().size()), 2);
    ClearSpan();
}

/**
 * @tc.name: RichEditorController012
 * @tc.desc: test update symbol span
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get richEditor controller
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    /**
     * @tc.steps: step2. initalize symbol span properties
     */
    TextStyle style;
    style.SetFontSize(FONT_SIZE_VALUE);
    style.SetFontWeight(FONT_WEIGHT_VALUE);
    style.SetSymbolColorList(SYMBOL_COLOR_LIST_1);
    style.SetRenderStrategy(RENDER_STRATEGY_SINGLE);
    style.SetEffectStrategy(EFFECT_STRATEGY_NONE);
    SymbolSpanOptions options;
    options.symbolId = SYMBOL_ID;
    options.style = style;

    /**
     * @tc.steps: step3. add symbol span
     */
    auto index1 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index1, 0);
    auto index2 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index2, 1);

    /**
     * @tc.steps: step4. update symbol span style
     */
    struct UpdateSpanStyle updateSpanStyle;
    updateSpanStyle.updateFontSize = FONT_SIZE_VALUE_2;
    updateSpanStyle.updateFontWeight = FONT_WEIGHT_BOLD;
    updateSpanStyle.updateSymbolColor = SYMBOL_COLOR_LIST_2;
    updateSpanStyle.updateSymbolRenderingStrategy = RENDER_STRATEGY_MULTI_COLOR;
    updateSpanStyle.updateSymbolEffectStrategy = EFFECT_STRATEGY_SCALE;
    updateSpanStyle.isSymbolStyle = true;
    richEditorController->SetUpdateSpanStyle(updateSpanStyle);

    ImageSpanAttribute imageStyle;
    style.SetFontSize(FONT_SIZE_VALUE_2);
    style.SetFontWeight(FONT_WEIGHT_BOLD);
    style.SetSymbolColorList(SYMBOL_COLOR_LIST_2);
    style.SetRenderStrategy(RENDER_STRATEGY_MULTI_COLOR);
    style.SetEffectStrategy(EFFECT_STRATEGY_SCALE);

    // update the first symbol span
    richEditorController->UpdateSpanStyle(0, 2, style, imageStyle);

    /**
     * @tc.steps: step5. test symbol span style
     */
    auto newSpan1 = AceType::DynamicCast<SpanNode>(richEditorNode_->GetChildAtIndex(0));
    ASSERT_NE(newSpan1, nullptr);
    EXPECT_EQ(newSpan1->GetFontSize(), FONT_SIZE_VALUE_2);
    EXPECT_EQ(newSpan1->GetFontWeight(), FONT_WEIGHT_BOLD);
    EXPECT_EQ(newSpan1->GetSymbolColorList(), SYMBOL_COLOR_LIST_2);
    EXPECT_EQ(newSpan1->GetSymbolRenderingStrategy(), RENDER_STRATEGY_MULTI_COLOR);
    EXPECT_EQ(newSpan1->GetSymbolEffectStrategy(), EFFECT_STRATEGY_SCALE);

    auto newSpan2 = AceType::DynamicCast<SpanNode>(richEditorNode_->GetChildAtIndex(1));
    ASSERT_NE(newSpan2, nullptr);
    EXPECT_EQ(newSpan2->GetFontSize(), FONT_SIZE_VALUE);
    EXPECT_EQ(newSpan2->GetFontWeight(), FONT_WEIGHT_VALUE);
    EXPECT_EQ(newSpan2->GetSymbolColorList(), SYMBOL_COLOR_LIST_1);
    EXPECT_EQ(newSpan2->GetSymbolRenderingStrategy(), RENDER_STRATEGY_SINGLE);
    EXPECT_EQ(newSpan2->GetSymbolEffectStrategy(), EFFECT_STRATEGY_NONE);

    ClearSpan();
}

/**
 * @tc.name: RichEditorController013
 * @tc.desc: test get symbol span info
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get richEditor controller
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    /**
     * @tc.steps: step2. initalize symbol span properties
     */
    TextStyle style;
    style.SetFontSize(FONT_SIZE_VALUE);
    style.SetFontWeight(FONT_WEIGHT_VALUE);
    style.SetSymbolColorList(SYMBOL_COLOR_LIST_1);
    style.SetRenderStrategy(RENDER_STRATEGY_SINGLE);
    style.SetEffectStrategy(EFFECT_STRATEGY_NONE);
    SymbolSpanOptions options;
    options.symbolId = SYMBOL_ID;
    options.style = style;

    /**
     * @tc.steps: step3. add symbol span
     */
    auto index1 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index1, 0);
    auto index2 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index2, 1);
    auto index3 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index3, 2);
    auto index4 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index4, 3);

    /**
     * @tc.steps: step4. get symbol span info
     */
    auto info1 = richEditorController->GetSpansInfo(2, 5);
    EXPECT_EQ(info1.selection_.selection[0], 2);
    EXPECT_EQ(info1.selection_.selection[1], 5);
    EXPECT_EQ(info1.selection_.resultObjects.size(), 2);

    auto info2 = richEditorController->GetSpansInfo(5, 2);
    EXPECT_EQ(info2.selection_.selection[0], 2);
    EXPECT_EQ(info2.selection_.selection[1], 5);
    EXPECT_EQ(info2.selection_.resultObjects.size(), 2);

    auto info3 = richEditorController->GetSpansInfo(-2, 5);
    EXPECT_EQ(info3.selection_.selection[0], 0);
    EXPECT_EQ(info3.selection_.selection[1], 5);
    EXPECT_EQ(info3.selection_.resultObjects.size(), 3);

    auto info4 = richEditorController->GetSpansInfo(2, -5);
    EXPECT_EQ(info4.selection_.selection[0], 0);
    EXPECT_EQ(info4.selection_.selection[1], 2);
    EXPECT_EQ(info4.selection_.resultObjects.size(), 1);

    ClearSpan();
}

/**
 * @tc.name: RichEditorController014
 * @tc.desc: test delete symbol span
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get richEditor controller
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    /**
     * @tc.steps: step2. initalize symbol span properties
     */
    TextStyle style;
    style.SetFontSize(FONT_SIZE_VALUE);
    SymbolSpanOptions options;
    options.symbolId = SYMBOL_ID;
    options.style = style;

    /**
     * @tc.steps: step3. add symbol span
     */
    auto index1 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index1, 0);
    auto index2 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index2, 1);
    auto index3 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index3, 2);
    auto index4 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index4, 3);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 4);

    /**
     * @tc.steps: step4. delete single symbol span
     */
    RangeOptions option2;
    option2.start = 0;
    option2.end = 2;
    richEditorController->DeleteSpans(option2);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 3);

    option2.start = 2;
    option2.end = 0;
    richEditorController->DeleteSpans(option2);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 2);

    option2.start = -1;
    option2.end = 2;
    richEditorController->DeleteSpans(option2);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 1);

    option2.start = 2;
    option2.end = -1;
    richEditorController->DeleteSpans(option2);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 0);

    /**
     * @tc.steps: step5. add symbol span
     */
    auto index5 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index5, 0);

    /**
     * @tc.steps: step6. delete symbol span
     */
    option2.start = 0;
    option2.end = 1;
    richEditorController->DeleteSpans(option2); // delete half symbol span, will fail
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 0);

    ClearSpan();
}

/**
 * @tc.name: RichEditorController015
 * @tc.desc: test use span & imagespan & symbolspan together
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController015, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get richEditor controller
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    /**
     * @tc.steps: step2. initalize span properties
     */
    SymbolSpanOptions options1;
    options1.symbolId = SYMBOL_ID;
    TextSpanOptions options2;
    options2.value = INIT_VALUE_1;
    ImageSpanOptions options3;
    options3.image = IMAGE_VALUE;

    /**
     * @tc.steps: step3. test add span
     */
    richEditorController->AddSymbolSpan(options1);
    richEditorController->AddTextSpan(options2);
    richEditorController->AddImageSpan(options3);
    richEditorController->AddTextSpan(options2);
    richEditorController->AddSymbolSpan(options1);
    EXPECT_EQ(static_cast<int32_t>(richEditorNode_->GetChildren().size()), 5);

    /**
     * @tc.steps: step4. test get span
     */
    auto info1 = richEditorController->GetSpansInfo(0, 3);
    EXPECT_EQ(info1.selection_.resultObjects.size(), 2);
    auto info2 = richEditorController->GetSpansInfo(8, 9);
    EXPECT_EQ(info2.selection_.resultObjects.size(), 1);

    /**
     * @tc.steps: step5. test update span
     */
    TextStyle textStyle;
    textStyle.SetFontSize(FONT_SIZE_VALUE_2);
    struct UpdateSpanStyle updateSpanStyle;
    updateSpanStyle.updateFontSize = FONT_SIZE_VALUE_2;

    richEditorController->SetUpdateSpanStyle(updateSpanStyle);
    ImageSpanAttribute imageStyle;
    richEditorController->UpdateSpanStyle(2, 8, textStyle, imageStyle);

    auto newSpan2 = AceType::DynamicCast<SpanNode>(richEditorNode_->GetChildAtIndex(1));
    ASSERT_NE(newSpan2, nullptr);
    EXPECT_EQ(newSpan2->GetFontSize(), FONT_SIZE_VALUE_2);

    /**
     * @tc.steps: step6. test delete span
     */
    RangeOptions option;
    option.start = 8;
    option.end = 15;
    richEditorController->DeleteSpans(option);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 3);

    option.start = 0;
    option.end = 2;
    richEditorController->DeleteSpans(option);
    EXPECT_EQ(richEditorNode_->GetChildren().size(), 2);

    ClearSpan();
}

/**
 * @tc.name: RichEditorController016
 * @tc.desc: test add many spans
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController016, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get richEditor controller
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    /**
     * @tc.steps: step2. initalize span properties
     */
    SymbolSpanOptions options1;
    options1.symbolId = SYMBOL_ID;
    TextSpanOptions options2;
    options2.value = INIT_VALUE_1;
    ImageSpanOptions options3;
    options3.image = IMAGE_VALUE;

    /**
     * @tc.steps: step3. test add span
     */
    for (int i = 0; i < 100; i++) {
        richEditorController->AddSymbolSpan(options1);
        richEditorController->AddTextSpan(options2);
        richEditorController->AddImageSpan(options3);
        richEditorController->AddTextSpan(options2);
        richEditorController->AddSymbolSpan(options1);
    }
    EXPECT_EQ(static_cast<int32_t>(richEditorNode_->GetChildren().size()), 500);

    ClearSpan();
}

/**
 * @tc.name: SetEnterKeyType
 * @tc.desc: test SetEnterKeyType
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, SetEnterKeyType, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get richEditor controller
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    richEditorModel.SetEnterKeyType(TextInputAction::NEW_LINE);
    richEditorNode_->MarkModifyDone();
    EXPECT_EQ(richEditorPattern->GetTextInputActionValue(richEditorPattern->GetDefaultTextInputAction()),
        TextInputAction::NEW_LINE);
    richEditorModel.SetEnterKeyType(TextInputAction::UNSPECIFIED);
    richEditorNode_->MarkModifyDone();
    EXPECT_EQ(richEditorPattern->GetTextInputActionValue(richEditorPattern->GetDefaultTextInputAction()),
        TextInputAction::NEW_LINE);
    ClearSpan();
}

/**
 * @tc.name: SupportAvoidanceTest
 * @tc.desc: test whether the custom keyboard supports the collision avoidance function
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, SupportAvoidanceTest, TestSize.Level1)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    auto overlayManager = pipeline->GetOverlayManager();
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    auto supportAvoidance = true;
    richEditorPattern->SetCustomKeyboardOption(supportAvoidance);
    auto support = richEditorPattern->keyboardAvoidance_;
    overlayManager->SetCustomKeyboardOption(support);
    EXPECT_TRUE(richEditorPattern->keyboardAvoidance_);
    supportAvoidance = false;
    richEditorPattern->SetCustomKeyboardOption(supportAvoidance);
    overlayManager->SetCustomKeyboardOption(support);
    EXPECT_FALSE(richEditorPattern->keyboardAvoidance_);
}

/**
 * @tc.name: IsSelectAreaVisible001
 * @tc.desc: test selectArea inVisible
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, IsSelectAreaVisible001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    /**
     * @tc.steps: step1. add text and paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);
    richEditorPattern->textSelector_.baseOffset = 0;
    richEditorPattern->textSelector_.destinationOffset = 6;
    richEditorPattern->contentRect_ = { 0.0, 0.0, 500.0, 500.0 };
    /**
     * @tc.steps: step2. test IsSelectAreaVisible
     */
    auto res = richEditorPattern->IsSelectAreaVisible();
    EXPECT_FALSE(res);
}

/**
 * @tc.name: IsSelectAreaVisible002
 * @tc.desc: test selectArea Visible
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, IsSelectAreaVisible002, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    /**
     * @tc.steps: step1. add text and paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { -400.0, -400.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);
    richEditorPattern->textSelector_.baseOffset = 0;
    richEditorPattern->textSelector_.destinationOffset = 6;
    richEditorPattern->contentRect_ = { -500.0, -500.0, 500.0, 500.0 };
    /**
     * @tc.steps: step2. test IsSelectAreaVisible
     */
    auto res = richEditorPattern->IsSelectAreaVisible();
    EXPECT_TRUE(res);
}

/**
 * @tc.name: onDraw001
 * @tc.desc: Verify the onDraw Magnifier.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, onDraw001, TestSize.Level1)
{
    Offset localOffset(0, 0);
    SymbolSpanOptions symbolSpanOptions;
    symbolSpanOptions.symbolId = SYMBOL_ID;

    //Verify the selected single line text magnifying glass
    OnDrawVerify(SelectSpanType::TYPESPAN, INIT_VALUE_1, symbolSpanOptions, localOffset, true);

    //Verify the selected multi line text magnifying glass
    OnDrawVerify(SelectSpanType::TYPESPAN, INIT_VALUE_3, symbolSpanOptions, localOffset, true);

    //Verify the selected image magnifying glass
    OnDrawVerify(SelectSpanType::TYPEIMAGE, INIT_VALUE_1, symbolSpanOptions, localOffset, true);

    //Verify the selected symbol magnifying glass
    OnDrawVerify(SelectSpanType::TYPESYMBOLSPAN, INIT_VALUE_1, symbolSpanOptions, localOffset, true);

    //Verify insertion status with a regular text magnifying glass
    OnDrawVerify(SelectSpanType::TYPESPAN, INIT_VALUE_1, symbolSpanOptions, localOffset);

    //Verify the insertion status of the image magnifying glass
    OnDrawVerify(SelectSpanType::TYPEIMAGE, INIT_VALUE_1, symbolSpanOptions, localOffset);

    //Verify the insertion state symbol magnifying glass
    OnDrawVerify(SelectSpanType::TYPESYMBOLSPAN, INIT_VALUE_1, symbolSpanOptions, localOffset);
}

/**
 * @tc.name: RichEditorController017
 * @tc.desc: test update span style
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorController017, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get richEditor controller
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    /**
     * @tc.steps: step2. test add span
     */
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    EXPECT_EQ(static_cast<int32_t>(richEditorNode_->GetChildren().size()), 3);

    /**
     * @tc.steps: step3. test update span
     */
    TextStyle textStyle;
    textStyle.SetFontSize(FONT_SIZE_VALUE);
    TextStyle textStyle2;
    textStyle2.SetFontSize(FONT_SIZE_VALUE_2);
    struct UpdateSpanStyle updateSpanStyle;
    updateSpanStyle.updateFontSize = FONT_SIZE_VALUE_2;
    richEditorController->SetUpdateSpanStyle(updateSpanStyle);
    ImageSpanAttribute imageStyle;

    /**
     * @tc.cases: case. call UpdateSpanStyle(), cover branch end < 0
     * @tc.expected: expect GetFontSize() is equal to FONT_SIZE_VALUE
     */
    richEditorController->UpdateSpanStyle(0, -1, textStyle, imageStyle);
    auto newSpan1 = AceType::DynamicCast<SpanNode>(richEditorNode_->GetChildAtIndex(0));
    ASSERT_NE(newSpan1, nullptr);
    EXPECT_EQ(newSpan1->GetFontSize(), FONT_SIZE_VALUE);

    /**
     * @tc.cases: case. call UpdateSpanStyle(), update FontSize to FONT_SIZE_VALUE_2, cover branch end > length
     * @tc.expected: expect GetFontSize() is equal to FONT_SIZE_VALUE_2
     */
    richEditorController->UpdateSpanStyle(0, 20, textStyle2, imageStyle);
    EXPECT_EQ(newSpan1->GetFontSize(), FONT_SIZE_VALUE_2);

    /**
     * @tc.cases: case. call UpdateSpanStyle(), update FontSize to FONT_SIZE_VALUE, cover branch start > end
     * @tc.expected: expect GetFontSize() is equal to FONT_SIZE_VALUE
     */
    richEditorController->UpdateSpanStyle(10, 0, textStyle, imageStyle);
    EXPECT_EQ(newSpan1->GetFontSize(), FONT_SIZE_VALUE);

    /**
     * @tc.cases: case. call UpdateSpanStyle(), update FontSize to FONT_SIZE_VALUE_2, cover branch start == end
     * @tc.expected: expect GetFontSize() is still equal to FONT_SIZE_VALUE
     */
    richEditorController->UpdateSpanStyle(0, 0, textStyle2, imageStyle);
    EXPECT_EQ(newSpan1->GetFontSize(), FONT_SIZE_VALUE);

    /**
     * @tc.cases: case. call UpdateSpanStyle(), update FontSize to FONT_SIZE_VALUE_2, cover branch start > length
     * @tc.expected: expect GetFontSize() is still equal to FONT_SIZE_VALUE
     */
    richEditorController->UpdateSpanStyle(20, 30, textStyle2, imageStyle);
    EXPECT_EQ(newSpan1->GetFontSize(), FONT_SIZE_VALUE);
    ClearSpan();
}

/**
 * @tc.name: RichEditorEventHub001
 * @tc.desc: test get insert
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorEventHub001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. set insert value
     */
    RichEditorInsertValue insertValueInfo;
    insertValueInfo.SetInsertOffset(1);
    insertValueInfo.SetInsertValue(INIT_VALUE_1);
    /**
     * @tc.steps: step2. get insert value
     */
    EXPECT_EQ(insertValueInfo.GetInsertOffset(), 1);
    EXPECT_EQ(insertValueInfo.GetInsertValue(), INIT_VALUE_1);
}

/**
 * @tc.name: RichEditorEventHub002
 * @tc.desc: test span result
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorEventHub002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. set span result
     */
    RichEditorAbstractSpanResult result;
    FONT_FEATURES_LIST fontFeature;
    RefPtr<ResourceObject> valueResource;
    SymbolSpanStyle symbolSpanStyle;

    result.SetSpanRangeEnd(1);
    result.SetFontFeature(fontFeature);
    result.SetLineHeight(20.0);
    result.SetLetterspacing(20.0);
    result.SetValueResource(valueResource);
    result.SetValueString(INIT_VALUE_1);
    result.SetSymbolSpanStyle(symbolSpanStyle);
    result.SetTextDecoration(TextDecoration::UNDERLINE);
    result.SetColor("");

    /**
     * @tc.steps: step2. get span result
     */
    EXPECT_EQ(result.GetSpanRangeEnd(), 1);
    EXPECT_EQ(result.GetFontFeatures(), fontFeature);
    EXPECT_EQ(result.GetLineHeight(), 20.0);
    EXPECT_EQ(result.GetLetterspacing(), 20.0);
    EXPECT_EQ(result.GetFontColor(), "");
    EXPECT_EQ(result.GetFontSize(), 0);
    EXPECT_EQ(result.GetValueResource(), valueResource);
    EXPECT_EQ(result.GetValueString(), INIT_VALUE_1);
    EXPECT_EQ(result.GetSymbolSpanStyle().lineHeight, 0.0);
    EXPECT_EQ(result.GetFontWeight(), 0);
    EXPECT_EQ(result.GetFontFamily(), "");
    EXPECT_EQ(result.GetTextDecoration(), TextDecoration::UNDERLINE);
    EXPECT_EQ(result.GetColor(), "");
}

/**
 * @tc.name: RichEditorEventHub003
 * @tc.desc: test edit change event
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorEventHub003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. set OnEditingChange func
     */
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    auto func = [](bool value) {
        g_isOnEditChangeCalled = value;
    };
    richEditorModel.SetOnEditingChange(std::move(func));

    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto eventHub = richEditorPattern->GetEventHub<RichEditorEventHub>();
    ASSERT_NE(eventHub, nullptr);

    /**
     * @tc.steps: step2. fire OnEditingChange func
     * @tc.expected: expect g_isOnEditChangeCalled is true
     */
    eventHub->FireOnEditingChange(true);
    EXPECT_EQ(g_isOnEditChangeCalled, true);

    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: RichEditorEventHub004
 * @tc.desc: test GetDragExtraParams
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorEventHub004, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto eventHub = richEditorPattern->GetEventHub<RichEditorEventHub>();
    ASSERT_NE(eventHub, nullptr);
    /**
     * @tc.cases: case. call GetDragExtraParams(), cover branch !extraInfo.empty()
     * @tc.expected: expect return jsonStr is {"extraInfo":"info"}
     */
    auto jsonStr = eventHub->GetDragExtraParams("info", Point(0, 250.f), DragEventType::MOVE);
    EXPECT_EQ(jsonStr, "{\"extraInfo\":\"info\"}");

    /**
     * @tc.cases: case. call GetDragExtraParams(), cover branch type == DragEventType::DROP
     * @tc.expected: expect return jsonStr is {"extraInfo":"info"}
     */
    jsonStr = eventHub->GetDragExtraParams("info", Point(0, 250.f), DragEventType::DROP);
    EXPECT_EQ(jsonStr, "{\"extraInfo\":\"info\"}");

    /**
     * @tc.cases: case. call GetDragExtraParams(), cover branch timestamp_ != 0
     * @tc.expected: expect return jsonStr is {}
     */
    auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    eventHub->timestamp_ = timestamp;
    jsonStr = eventHub->GetDragExtraParams("", Point(0, 250.f), DragEventType::DROP);
    EXPECT_EQ(jsonStr, "{}");

    /**
     * @tc.cases: case. call GetDragExtraParams(), cover branch pattern->GetTimestamp() == timestamp_
     * @tc.expected: expect return jsonStr is {"isInComponent":true}
     */
    richEditorPattern->timestamp_ = timestamp;
    jsonStr = eventHub->GetDragExtraParams("", Point(0, 250.f), DragEventType::DROP);
    EXPECT_EQ(jsonStr, "{\"isInComponent\":true}");
    EXPECT_EQ(eventHub->timestamp_, 0);
}

/**
 * @tc.name: RichEditorEventHub005
 * @tc.desc: test fire event
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, RichEditorEventHub005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init eventHub
     */
    RichEditorModelNG richEditorModel;
    richEditorModel.Create();
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto eventHub = richEditorPattern->GetEventHub<RichEditorEventHub>();
    ASSERT_NE(eventHub, nullptr);

    /**
     * @tc.steps: step2. fire event when there is null func
     */
    RichEditorChangeValue value;
    StyledStringChangeValue info;
    TextCommonEvent event;
    eventHub->FireOnDidChange(value);
    eventHub->FireOnCut(event);
    eventHub->FireOnCopy(event);
    EXPECT_TRUE(eventHub->FireOnWillChange(value));
    EXPECT_TRUE(eventHub->FireOnStyledStringWillChange(info));

    while (!ViewStackProcessor::GetInstance()->elementsStack_.empty()) {
        ViewStackProcessor::GetInstance()->elementsStack_.pop();
    }
}

/**
 * @tc.name: ParagraphManager001
 * @tc.desc: Test the paragraph manager GetHeight function.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, ParagraphManager001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    /**
     * @tc.steps: step1. test to obtain initial height
     */
    auto height = richEditorPattern->paragraphs_.GetHeight();
    EXPECT_EQ(height, 0.0f);

    /**
     * @tc.steps: step2. add paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);

    /**
     * @tc.steps: step3. test get height
     */
    height = richEditorPattern->paragraphs_.GetHeight();
    EXPECT_EQ(height, 0.0f);
}

/**
 * @tc.name: ParagraphManager002
 * @tc.desc: Test the paragraph manager GetMaxIntrinsicWidth function.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, ParagraphManager002, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    /**
     * @tc.steps: step1. test to obtain initial max intrinsic width
     */
    double maxIntrinsicWidth = richEditorPattern->paragraphs_.GetMaxIntrinsicWidth();
    EXPECT_EQ(maxIntrinsicWidth, 0.0f);

    /**
     * @tc.steps: step2. add paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);

    /**
     * @tc.steps: step3. test get maxIntrinsicWidth
     */
    maxIntrinsicWidth = richEditorPattern->paragraphs_.GetMaxIntrinsicWidth();
    EXPECT_EQ(maxIntrinsicWidth, 0.0f);
}

/**
 * @tc.name: ParagraphManager003
 * @tc.desc: Test the paragraph manager DidExceedMaxLines function.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, ParagraphManager003, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    /**
     * @tc.steps: step1. test to obtain initial exceed max lines
     */
    bool didExceedMaxLines = richEditorPattern->paragraphs_.DidExceedMaxLines();
    EXPECT_EQ(didExceedMaxLines, false);

    /**
     * @tc.steps: step2. add paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);

    /**
     * @tc.steps: step3. test didExceedMaxLines
     */
    didExceedMaxLines = richEditorPattern->paragraphs_.DidExceedMaxLines();
    EXPECT_EQ(didExceedMaxLines, false);
}

/**
 * @tc.name: ParagraphManager004
 * @tc.desc: Test the paragraph manager GetLongestLine function.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, ParagraphManager004, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    /**
     * @tc.steps: step1. test to obtain initial max intrinsic width
     */
    double longestLine = richEditorPattern->paragraphs_.GetLongestLine();
    EXPECT_EQ(longestLine, 0.0f);

    /**
     * @tc.steps: step2. add paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);

    /**
     * @tc.steps: step3. test get longest line
     */
    longestLine = richEditorPattern->paragraphs_.GetLongestLine();
    EXPECT_EQ(longestLine, 0.0f);
}

/**
 * @tc.name: ParagraphManager005
 * @tc.desc: Test the paragraph manager GetMaxWidth function.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, ParagraphManager005, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    /**
     * @tc.steps: step1. test to obtain initial max width
     */
    double maxWidth = richEditorPattern->paragraphs_.GetMaxWidth();
    EXPECT_EQ(maxWidth, 0.0f);

    /**
     * @tc.steps: step2. add paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);

    /**
     * @tc.steps: step3. test get max width
     */
    maxWidth = richEditorPattern->paragraphs_.GetMaxWidth();
    EXPECT_EQ(maxWidth, 0.0f);
}

/**
 * @tc.name: ParagraphManager006
 * @tc.desc: Test the paragraph manager GetTextWidth function.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, ParagraphManager006, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    /**
     * @tc.steps: step1. test to obtain initial max width
     */
    double maxTextWidth = richEditorPattern->paragraphs_.GetTextWidth();
    EXPECT_EQ(maxTextWidth, 0.0f);

    /**
     * @tc.steps: step2. add paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);

    /**
     * @tc.steps: step3. test get text width
     */
    maxTextWidth = richEditorPattern->paragraphs_.GetTextWidth();
    EXPECT_EQ(maxTextWidth, 0.0f);
}

/**
 * @tc.name: ParagraphManager007
 * @tc.desc: Test the paragraph manager GetTextWidthIncludeIndent function.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, ParagraphManager007, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    /**
     * @tc.steps: step1. test to obtain initial text width include indent
     */
    double textWidthIncludeIndent = richEditorPattern->paragraphs_.GetTextWidthIncludeIndent();
    EXPECT_EQ(textWidthIncludeIndent, 0.0f);

    /**
     * @tc.steps: step2. add paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);

    /**
     * @tc.steps: step3. test get text width include indent
     */
    textWidthIncludeIndent = richEditorPattern->paragraphs_.GetTextWidthIncludeIndent();
    EXPECT_EQ(textWidthIncludeIndent, 0.0f);
}

/**
 * @tc.name: ParagraphManager008
 * @tc.desc: Test the paragraph manager GetLineCount function.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, ParagraphManager008, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    /**
     * @tc.steps: step1. test to obtain initial line count
     */
    size_t dineCount = richEditorPattern->paragraphs_.GetLineCount();
    EXPECT_EQ(dineCount, 0);

    /**
     * @tc.steps: step2. add paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);

    /**
     * @tc.steps: step3. test get line count
     */
    dineCount = richEditorPattern->paragraphs_.GetLineCount();
    EXPECT_EQ(dineCount, 0);
}

/**
 * @tc.name: ParagraphManager009
 * @tc.desc: Test the paragraph manager function.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, ParagraphManager009, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    /**
     * @tc.steps: step1. add paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);

    /**
     * @tc.steps: step2. test get index
     */
    auto textRect = richEditorPattern->GetTextRect();
    textRect.SetTop(textRect.GetY() - 0.0f);
    textRect.SetHeight(textRect.Height() - 0.0f);
    Offset offset = Offset(textRect.GetX(), textRect.GetY());
    bool clamp = false;
    int32_t paragraphsIndex = richEditorPattern->paragraphs_.GetIndex(offset, clamp);
    EXPECT_EQ(paragraphsIndex, 0);

    /**
     * @tc.steps: step3. test get glyph index
     */
    int32_t glyphIndex = richEditorPattern->paragraphs_.GetGlyphIndexByCoordinate(offset, clamp);
    EXPECT_EQ(glyphIndex, 0);

    /**
     * @tc.steps: step4. test get word boundary
     */
    int32_t offset1 = 0;
    int32_t start = 0;
    int32_t end = 0;
    bool wordBoundary = richEditorPattern->paragraphs_.GetWordBoundary(offset1, start, end);
    EXPECT_EQ(wordBoundary, false);

    /**
     * @tc.steps: step5. test calc caret metrics
     */
    int32_t extent = 0;
    CaretMetricsF caretCaretMetric;
    TextAffinity textAffinity = TextAffinity::DOWNSTREAM;
    bool caretMetrics = richEditorPattern->paragraphs_.CalcCaretMetricsByPosition(extent, caretCaretMetric,
        textAffinity);
    EXPECT_EQ(caretMetrics, false);
}

/**
 * @tc.name: ParagraphManager010
 * @tc.desc: Test the paragraph manager function.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, ParagraphManager010, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    /**
     * @tc.steps: step1. add paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);

    /**
     * @tc.steps: step2. test get line metrics
     */
    RectF rect = { 0.0, 0.0, 100.0, 100.0 };
    int32_t paragraphIndex = 0;
    LineMetrics lineMetrics = richEditorPattern->paragraphs_.GetLineMetricsByRectF(rect, paragraphIndex);
    EXPECT_EQ(lineMetrics.ascender, 0.0f);

    /**
     * @tc.steps: step3. test editable status
     */
    auto accessibilityProperty = richEditorPattern->CreateAccessibilityProperty();
    bool isEditable = accessibilityProperty->IsEditable();
    EXPECT_EQ(isEditable, true);
}

/**
 * @tc.name: Controller001
 * @tc.desc: Test the controller function.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, Controller001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    /**
     * @tc.steps: step1. add paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);

    TextStyle style;
    style.SetLineHeight(LINE_HEIGHT_VALUE);
    style.SetLetterSpacing(LETTER_SPACING);
    style.SetFontFeatures(TEXT_FONTFEATURE);
    TextSpanOptions options;
    options.value = INIT_VALUE_1;
    options.style = style;
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    richEditorController->AddTextSpan(options);
    AddSpan(INIT_VALUE_1);
    auto info = richEditorController->GetSpansInfo(1, 5);
    TextStyleResult textStyle1 = info.selection_.resultObjects.front().textStyle;
    UpdateSpanStyle typingStyle;
    richEditorController->SetTypingStyle(typingStyle, style);
    TextSpanOptions options1;
    options1.style = richEditorPattern->typingTextStyle_;
    AddSpan(INIT_VALUE_1);
    auto info1 = richEditorController->GetSpansInfo(1, 5);
    TextStyleResult textStyle2 = info1.selection_.resultObjects.front().textStyle;
    EXPECT_EQ(textStyle2.lineHeight, LINE_HEIGHT_VALUE.ConvertToVp());
    EXPECT_EQ(textStyle2.letterSpacing, LETTER_SPACING.ConvertToVp());
    for (const auto& pair : textStyle1.fontFeature) {
        EXPECT_EQ(pair.first, "subs");
        EXPECT_EQ(pair.second, 1);
    }
    ClearSpan();
}

/**
 * @tc.name: Controller002
 * @tc.desc: Test the controller function.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, Controller002, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    /**
     * @tc.steps: step1. add paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);

    TextStyle style;
    style.SetLineHeight(LINE_HEIGHT_VALUE);
    style.SetLetterSpacing(LETTER_SPACING);
    style.SetFontFeatures(TEXT_FONTFEATURE);
    TextSpanOptions options;
    options.value = INIT_VALUE_1;
    options.style = style;
    richEditorController->AddTextSpan(options);
    AddSpan(INIT_VALUE_1);
    richEditorController->SetSelection(1, 3);
    auto info1 = richEditorController->GetSpansInfo(1, 2);
    EXPECT_EQ(info1.selection_.resultObjects.front().textStyle.lineHeight, LINE_HEIGHT_VALUE.ConvertToVp());
    EXPECT_EQ(info1.selection_.resultObjects.front().textStyle.letterSpacing, LETTER_SPACING.ConvertToVp());
    for (const auto& pair : info1.selection_.resultObjects.front().textStyle.fontFeature) {
        EXPECT_EQ(pair.first, "subs");
        EXPECT_EQ(pair.second, 1);
    }
    ClearSpan();
    richEditorController->CloseSelectionMenu();
}

/**
 * @tc.name: ToStyledString001
 * @tc.desc: Test spans to styledString.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, ToStyledString001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    /**
     * @tc.steps: step1. init spans
     */
    TextSpanOptions options;
    options.value = INIT_VALUE_1;
    richEditorController->AddTextSpan(options);
    options.value = INIT_VALUE_2;
    richEditorController->AddTextSpan(options);

    /**
     * @tc.steps: step2. test ToStyledString
     */
    auto spanString = richEditorPattern->ToStyledString(0, 8);
    ASSERT_NE(spanString, nullptr);
    EXPECT_EQ(spanString->GetSpanItems().size(), 2);
}

/**
 * @tc.name: AddSpanByPasteData001
 * @tc.desc: Test add span by pasteData.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorBaseTestNg, AddSpanByPasteData001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    /**
     * @tc.steps: step1. init spans
     */
    AddImageSpan();
    TextSpanOptions options;
    options.value = INIT_VALUE_1;
    richEditorController->AddTextSpan(options);

    /**
     * @tc.steps: step2. test AddSpanByPasteData001
     */
    auto spanString = richEditorPattern->ToStyledString(0, 8);
    ASSERT_NE(spanString, nullptr);
    richEditorPattern->spans_.clear();
    richEditorPattern->isSpanStringMode_ = false;
    richEditorPattern->AddSpanByPasteData(spanString);
    EXPECT_EQ(richEditorPattern->spans_.size(), 2);
}
} // namespace OHOS::Ace::NG
