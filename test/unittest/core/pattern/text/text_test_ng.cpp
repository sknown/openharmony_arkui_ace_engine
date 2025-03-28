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

#include "text_base.h"
#include "core/components/text_overlay/text_overlay_theme.h"

namespace OHOS::Ace::NG {

namespace {
int32_t callBack1 = 0;
int32_t callBack2 = 0;
int32_t callBack3 = 0;
const std::string IMAGE_VALUE = "image1";
const std::string BUNDLE_NAME = "bundleName";
const std::string MODULE_NAME = "moduleName";
constexpr uint32_t DEFAULT_NODE_ID = 0;
constexpr uint32_t UKNOWN_VALUE = 0;
constexpr uint32_t RENDERINGSTRATEGY_MULTIPLE_COLOR = 1;
constexpr uint32_t RENDERINGSTRATEGY_MULTIPLE_OPACITY = 2;
void ConstructGestureStyle(GestureStyle& gestureInfo)
{
    auto onClick = [](const BaseEventInfo* info) {};
    auto tmpClickFunc = [func = std::move(onClick)](GestureEvent& info) { func(&info); };
    gestureInfo.onClick = std::move(tmpClickFunc);

    auto onLongPress = [](const BaseEventInfo* info) {};
    auto tmpLongPressFunc = [func = std::move(onLongPress)](GestureEvent& info) { func(&info); };
    gestureInfo.onLongPress = std::move(tmpLongPressFunc);
}
} // namespace

class TextTestNg : public TextBases {
public:
};

/**
 * @tc.name: TextFrameNodeCreator001
 * @tc.desc: Test all the properties of text.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextFrameNodeCreator001, TestSize.Level1)
{
    TestProperty testProperty;
    testProperty.fontSizeValue = std::make_optional(FONT_SIZE_VALUE);
    testProperty.textColorValue = std::make_optional(TEXT_COLOR_VALUE);
    testProperty.italicFontStyleValue = std::make_optional(ITALIC_FONT_STYLE_VALUE);
    testProperty.fontWeightValue = std::make_optional(FONT_WEIGHT_VALUE);
    testProperty.fontFamilyValue = std::make_optional(FONT_FAMILY_VALUE);
    testProperty.textAlignValue = std::make_optional(TEXT_ALIGN_VALUE);
    testProperty.textOverflowValue = std::make_optional(TEXT_OVERFLOW_VALUE);
    testProperty.maxLinesValue = std::make_optional(MAX_LINES_VALUE);
    testProperty.lineHeightValue = std::make_optional(LINE_HEIGHT_VALUE);
    testProperty.lineSpacingValue = std::make_optional(LINE_SPACING_VALUE);
    testProperty.textDecorationValue = std::make_optional(TEXT_DECORATION_VALUE);
    testProperty.textDecorationColorValue = std::make_optional(TEXT_DECORATION_COLOR_VALUE);
    testProperty.baselineOffsetValue = std::make_optional(BASELINE_OFFSET_VALUE);
    testProperty.textCaseValue = std::make_optional(TEXT_CASE_VALUE);
    testProperty.adaptMinFontSize = std::make_optional(ADAPT_MIN_FONT_SIZE_VALUE);
    testProperty.adaptMaxFontSize = std::make_optional(ADAPT_MAX_FONT_SIZE_VALUE);
    testProperty.textIndent = std::make_optional(TEXT_INDENT);
    testProperty.wordBreak = std::make_optional(TEXT_WORD_BREAK);
    testProperty.lineBreakStrategy = std::make_optional(TEXT_LINE_BREAK_STRATEGY);

    RefPtr<FrameNode> frameNode = CreateTextParagraph(CREATE_VALUE, testProperty);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextLayoutProperty> textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutProperty);
    ASSERT_NE(textLayoutProperty, nullptr);
    EXPECT_EQ(textLayoutProperty->GetContentValue(), CREATE_VALUE);
    const std::unique_ptr<FontStyle>& fontStyle = textLayoutProperty->GetFontStyle();
    ASSERT_NE(fontStyle, nullptr);
    const std::unique_ptr<TextLineStyle>& textLineStyle = textLayoutProperty->GetTextLineStyle();
    ASSERT_NE(textLineStyle, nullptr);
    TextStyle textStyle = CreateTextStyleUsingTheme(fontStyle, textLineStyle, nullptr);
    EXPECT_EQ(textStyle.GetFontSize(), FONT_SIZE_VALUE);
    EXPECT_EQ(textStyle.GetTextColor(), TEXT_COLOR_VALUE);
    EXPECT_EQ(textStyle.GetFontStyle(), ITALIC_FONT_STYLE_VALUE);
    EXPECT_EQ(textStyle.GetFontWeight(), FONT_WEIGHT_VALUE);
    EXPECT_EQ(textStyle.GetFontFamilies(), FONT_FAMILY_VALUE);
    EXPECT_EQ(textStyle.GetTextAlign(), TEXT_ALIGN_VALUE);
    EXPECT_EQ(textStyle.GetTextOverflow(), TEXT_OVERFLOW_VALUE);
    EXPECT_EQ(textStyle.GetMaxLines(), MAX_LINES_VALUE);
    EXPECT_EQ(textStyle.GetLineHeight(), LINE_HEIGHT_VALUE);
    EXPECT_EQ(textStyle.GetLineSpacing(), LINE_SPACING_VALUE);
    EXPECT_EQ(textStyle.GetTextDecoration(), TEXT_DECORATION_VALUE);
    EXPECT_EQ(textStyle.GetTextDecorationColor(), TEXT_DECORATION_COLOR_VALUE);
    EXPECT_EQ(textStyle.GetBaselineOffset(), BASELINE_OFFSET_VALUE);
    EXPECT_EQ(textStyle.GetTextCase(), TEXT_CASE_VALUE);
    EXPECT_EQ(textStyle.GetAdaptMinFontSize(), ADAPT_MIN_FONT_SIZE_VALUE);
    EXPECT_EQ(textStyle.GetAdaptMaxFontSize(), ADAPT_MAX_FONT_SIZE_VALUE);
    EXPECT_EQ(textStyle.GetAdaptTextSize(),
        testProperty.adaptMinFontSize.has_value() || testProperty.adaptMaxFontSize.has_value());
    EXPECT_EQ(textStyle.GetWordBreak(), TEXT_WORD_BREAK);
    EXPECT_EQ(textStyle.GetLineBreakStrategy(), TEXT_LINE_BREAK_STRATEGY);

    /**
     * @tc.cases: case2. renderContext has foreground color and modifier will foreground color flag
     */
    frameNode->GetRenderContext()->UpdateForegroundColor(Color::RED);
    textStyle = CreateTextStyleUsingThemeWithText(frameNode, fontStyle, textLineStyle, nullptr);
    EXPECT_EQ(textStyle.GetTextColor(), FOREGROUND_COLOR_VALUE);
}

/**
 * @tc.name: TextFrameNodeCreator002
 * @tc.desc: Test all the property of text.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextFrameNodeCreator002, TestSize.Level1)
{
    TestProperty testProperty;
    testProperty.letterSpacing = std::make_optional(LETTER_SPACING);
    testProperty.baselineOffsetValue = std::make_optional(BASELINE_OFFSET_VALUE);
    auto frameNode = CreateTextParagraph(CREATE_VALUE, testProperty);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    auto textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutProperty);
    ASSERT_NE(textLayoutProperty, nullptr);
    EXPECT_EQ(textLayoutProperty->GetContentValue(), CREATE_VALUE);
    const std::unique_ptr<FontStyle>& fontStyle = textLayoutProperty->GetFontStyle();
    ASSERT_NE(fontStyle, nullptr);
    const std::unique_ptr<TextLineStyle>& textLineStyle = textLayoutProperty->GetTextLineStyle();
    ASSERT_NE(textLineStyle, nullptr);
    auto textStyle = CreateTextStyleUsingTheme(fontStyle, textLineStyle, nullptr);
    EXPECT_EQ(textStyle.GetLetterSpacing(), LETTER_SPACING);
}

/**
 * @tc.name: TextFrameNodeCreator003
 * @tc.desc: Test all the fontWeight and events of text.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextFrameNodeCreator003, TestSize.Level1)
{
    TextModelNG textModelNG;
    textModelNG.Create(CREATE_VALUE);
    auto frameNode = AceType::Claim(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextLayoutProperty> textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutProperty);
    ASSERT_NE(textLayoutProperty, nullptr);
    EXPECT_EQ(textLayoutProperty->GetContentValue(), CREATE_VALUE);
    textModelNG.SetFontWeight(FontWeight::LIGHTER);
    textModelNG.SetLineHeight(LINE_HEIGHT_VALUE);
    const std::unique_ptr<FontStyle>& fontStyle = textLayoutProperty->GetFontStyle();
    ASSERT_NE(textLayoutProperty, nullptr);
    const std::unique_ptr<TextLineStyle>& textLineStyle = textLayoutProperty->GetTextLineStyle();
    ASSERT_NE(textLayoutProperty, nullptr);
    TextStyle textStyle = CreateTextStyleUsingTheme(fontStyle, textLineStyle, nullptr);

    textModelNG.SetFontWeight(FontWeight::LIGHTER);
    textStyle.SetFontWeight(FontWeight::W100);
    EXPECT_EQ(textStyle.GetFontWeight(), FontWeight::W100);
    textModelNG.SetFontWeight(FontWeight::REGULAR);
    textStyle.SetFontWeight(FontWeight::W400);
    EXPECT_EQ(textStyle.GetFontWeight(), FontWeight::W400);
    textModelNG.SetFontWeight(FontWeight::NORMAL);
    textStyle.SetFontWeight(FontWeight::W400);
    EXPECT_EQ(textStyle.GetFontWeight(), FontWeight::W400);
    textModelNG.SetFontWeight(FontWeight::MEDIUM);
    textStyle.SetFontWeight(FontWeight::W500);
    EXPECT_EQ(textStyle.GetFontWeight(), FontWeight::W500);
    textModelNG.SetFontWeight(FontWeight::BOLD);
    textStyle.SetFontWeight(FontWeight::W700);
    EXPECT_EQ(textStyle.GetFontWeight(), FontWeight::W700);
    textModelNG.SetFontWeight(FontWeight::BOLDER);
    textStyle.SetFontWeight(FontWeight::W900);
    EXPECT_EQ(textStyle.GetFontWeight(), FontWeight::W900);
    textModelNG.SetFontWeight(FontWeight::W900);
    textStyle.SetFontWeight(FontWeight::W900);
    EXPECT_EQ(textStyle.GetFontWeight(), FontWeight::W900);

    textModelNG.SetOnClick(onClickFunc);
    textModelNG.SetRemoteMessage(onRemoteMessage);
    textModelNG.SetCopyOption(copyOption);
    textModelNG.SetOnDragStart(OnDragStartFunction);
    textModelNG.SetOnDragEnter(OnDragDropFunction);
    textModelNG.SetOnDragMove(OnDragDropFunction);
    textModelNG.SetOnDragLeave(OnDragDropFunction);
    textModelNG.SetOnDrop(OnDragDropFunction);
}

/**
 * @tc.name: SetTextDetectEnable002
 * @tc.desc: Test SetTextDetectEnable.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, SetTextDetectEnable002, TestSize.Level1)
{
    TextModelNG textModelNG;
    textModelNG.Create(CREATE_VALUE);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextLayoutProperty> textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutProperty);
    ASSERT_NE(textLayoutProperty, nullptr);
    EXPECT_EQ(textLayoutProperty->GetContentValue(), CREATE_VALUE);

    ASSERT_EQ(textModelNG.GetTextOverflow(frameNode), TextOverflow::CLIP);
    ASSERT_EQ(textModelNG.GetTextIndent(frameNode), ADAPT_ZERO_FONT_SIZE_VALUE);
    ASSERT_EQ(textModelNG.GetCopyOption(frameNode), CopyOptions::None);
    textModelNG.GetMarqueeOptions(frameNode);
    ASSERT_EQ(textModelNG.GetHeightAdaptivePolicy(frameNode), TextHeightAdaptivePolicy::MAX_LINES_FIRST);
    ASSERT_EQ(textModelNG.GetAdaptMinFontSize(frameNode), ADAPT_ZERO_FONT_SIZE_VALUE);
    ASSERT_EQ(textModelNG.GetAdaptMaxFontSize(frameNode), ADAPT_ZERO_FONT_SIZE_VALUE);
    ASSERT_EQ(textModelNG.GetDefaultColor(), Color::BLACK);
    ASSERT_EQ(textModelNG.GetFontColor(frameNode), Color::BLACK);
    ASSERT_EQ(textModelNG.GetTextBaselineOffset(frameNode), ADAPT_ZERO_FONT_SIZE_VALUE);
    std::vector<Shadow> defaultShadow;
    ASSERT_EQ(textModelNG.GetTextShadow(frameNode), defaultShadow);
    ASSERT_EQ(textModelNG.GetWordBreak(frameNode), WordBreak::BREAK_WORD);
}

/**
 * @tc.name: SetTextDetectEnable003
 * @tc.desc: Test SetTextDetectEnable.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, SetTextDetectEnable003, TestSize.Level1)
{
    TextModelNG textModelNG;
    textModelNG.Create(CREATE_VALUE);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextLayoutProperty> textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutProperty);
    ASSERT_NE(textLayoutProperty, nullptr);
    EXPECT_EQ(textLayoutProperty->GetContentValue(), CREATE_VALUE);

    textModelNG.SetFontSize(frameNode, ADAPT_ZERO_FONT_SIZE_VALUE);
    EXPECT_EQ(textModelNG.GetFontSize(frameNode), ADAPT_ZERO_FONT_SIZE_VALUE);

    textModelNG.SetTextDetectConfig(frameNode, "apple, orange, banana");
    ASSERT_NE(textModelNG.GetTextDetectConfig(frameNode), "apple, orange, banana");

    auto onResult = [](const std::string&) {};
    textModelNG.SetTextDetectConfig(frameNode, "apple, orange, banana", std::move(onResult));
    ASSERT_NE(textModelNG.GetTextDetectConfig(frameNode), "apple, orange, banana");

    auto textPattern = frameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    textModelNG.SetOnDetectResultUpdate(frameNode, std::move(onResult));
    EXPECT_NE(textPattern->dataDetectorAdapter_->onResult_, nullptr);

    FONT_FEATURES_LIST value;
    ASSERT_EQ(textModelNG.GetFontFeature(frameNode), value);
    ASSERT_EQ(textModelNG.GetLineBreakStrategy(frameNode), TEXT_LINE_BREAK_STRATEGY);

    textModelNG.SetSelectedBackgroundColor(frameNode, Color::BLACK);
    ASSERT_EQ(textModelNG.GetSelectedBackgroundColor(frameNode), Color::BLACK);

    textModelNG.ResetSelectedBackgroundColor(frameNode);
    ASSERT_EQ(textModelNG.GetSelectedBackgroundColor(frameNode), Color::BLACK);

    textModelNG.SetTextContentWithStyledString(frameNode, nullptr);
    ASSERT_EQ(textPattern->GetExternalParagraph(), nullptr);

    int32_t startIndex = 1;
    int32_t endIndex = 10;
    textModelNG.SetTextSelection(frameNode, startIndex, endIndex);
    EXPECT_NE(textPattern->textSelector_.GetStart(), startIndex);

    std::string EventValue;
    auto onCopyResult = [&EventValue](const std::string& param) { EventValue = param; };

    auto eventHub = frameNode->GetEventHub<TextEventHub>();
    textModelNG.SetOnCopy(frameNode, onCopyResult);
    EXPECT_NE(eventHub->onCopy_, nullptr);

    bool isSelectChanged = false;
    auto onSelectionChanged = [&isSelectChanged](int32_t, int32_t) { isSelectChanged = true; };
    textModelNG.SetOnTextSelectionChange(frameNode, onSelectionChanged);
    EXPECT_NE(eventHub->onSelectionChange_, nullptr);
}

/**
 * @tc.name: GetSelectedBackgroundColor001
 * @tc.desc: Test GetSelectedBackgroundColor when GetHost is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, GetSelectedBackgroundColor001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create.
     */
    TextModelNG textModelNG;
    textModelNG.Create(CREATE_VALUE);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextLayoutProperty> textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutProperty);
    ASSERT_NE(textLayoutProperty, nullptr);
    EXPECT_EQ(textLayoutProperty->GetContentValue(), CREATE_VALUE);

    /**
     * @tc.steps: step2. set theme.
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    auto theme = AceType::MakeRefPtr<MockThemeManager>();
    pipeline->SetThemeManager(theme);
    EXPECT_CALL(*theme, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextTheme>()));
    ASSERT_EQ(textModelNG.GetSelectedBackgroundColor(frameNode), Color::BLACK);

    Font font;
    textModelNG.SetFont(font);
    EXPECT_EQ(textModelNG.GetFontSize(frameNode), ADAPT_ZERO_FONT_SIZE_VALUE);
}

/**
 * @tc.name: GetMarqueeOptions001
 * @tc.desc: Test GetMarqueeOptions when GetHost is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, GetMarqueeOptions001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create.
     */
    TextModelNG textModelNG;
    textModelNG.Create(CREATE_VALUE);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextLayoutProperty> textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutProperty);
    ASSERT_NE(textLayoutProperty, nullptr);
    EXPECT_EQ(textLayoutProperty->GetContentValue(), CREATE_VALUE);

    /**
     * @tc.steps: step2. set theme.
     */
    TextMarqueeOptions options;
    options.UpdateTextMarqueeStart(true);
    options.UpdateTextMarqueeStep(3);
    options.UpdateTextMarqueeLoop(3);
    options.UpdateTextMarqueeDirection(MarqueeDirection::RIGHT);
    options.UpdateTextMarqueeDelay(3);
    options.UpdateTextMarqueeFadeout(false);
    options.UpdateTextMarqueeStartPolicy(MarqueeStartPolicy::ON_FOCUS);
    textModelNG.SetMarqueeOptions(options);
    textModelNG.GetMarqueeOptions(frameNode);

    EXPECT_EQ(textLayoutProperty->HasTextMarqueeStart(), true);
}

/**
 * @tc.name: SetOnMarqueeStateChange001
 * @tc.desc: Test SetOnMarqueeStateChange.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, SetOnMarqueeStateChange001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create.
     */
    TextModelNG textModelNG;
    textModelNG.Create(CREATE_VALUE);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextLayoutProperty> textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutProperty);
    ASSERT_NE(textLayoutProperty, nullptr);
    EXPECT_EQ(textLayoutProperty->GetContentValue(), CREATE_VALUE);

    /**
     * @tc.steps: step2. set theme.
     */
    bool isSelectChanged = false;
    auto onSelectionChanged = [&isSelectChanged](int32_t) { isSelectChanged = true; };
    textModelNG.SetOnMarqueeStateChange(onSelectionChanged);

    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<TextEventHub>();
    EXPECT_NE(eventHub->onMarqueeStateChange_, nullptr);
}

/**
 * @tc.name: OnAttachToFrameNode001
 * @tc.desc: Test TextPattern OnAttachToFrameNode when GetHost is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, OnAttachToFrameNode001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    pattern->OnAttachToFrameNode();
    EXPECT_EQ(pattern->longPressEvent_, nullptr);
}

/**
 * @tc.name: OnHandleMoveDone001
 * @tc.desc: Test TextPattern OnHandleMoveDone
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, OnHandleMoveDone001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and pattern
     */
    auto [frameNode, pattern] = Init();
    pattern->textSelector_.Update(0, TEXT_SIZE_INT);

    /**
     * @tc.steps: step2. call OnHandleMoveDone when SelectOverlay is off.
     * @tc.expected: the function exits normally
     */
    RectF handleRect = CONTENT_RECT;
    pattern->selectOverlay_->OnHandleMoveDone(handleRect, true);
    EXPECT_EQ(pattern->textSelector_.GetTextStart(), 0);
    EXPECT_EQ(pattern->textSelector_.GetTextEnd(), TEXT_SIZE_INT);

    /**
     * @tc.steps: step3. call OnHandleMoveDone when SelectOverlay is on.
     * @tc.expected: the OnHandleMoveDone function exits normally
     */
    pattern->ShowSelectOverlay();
    bool isFirstHandle[2] = { true, false };
    for (int i = 0; i < 2; i++) {
        pattern->textForDisplay_ = "abcdefghij";
        pattern->selectOverlay_->OnHandleMoveDone(handleRect, isFirstHandle[i]);
        EXPECT_EQ(pattern->textSelector_.GetTextStart(), 0);
        EXPECT_EQ(pattern->textSelector_.GetTextEnd(), TEXT_SIZE_INT);
    }
}

/**
 * @tc.name: ShowSelectOverlay001
 * @tc.desc: Test TextPattern ShowSelectOverlay when SelectOverlayProxy is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, ShowSelectOverlay001, TestSize.Level1)
{
    auto [frameNode, pattern] = Init();
    /**
     * @tc.steps: step1. call ShowSelectOverlay
     * @tc.expected: select overlay is on.
     */
    pattern->ShowSelectOverlay();
    EXPECT_TRUE(pattern->selectOverlay_->SelectOverlayIsOn());
}

/**
 * @tc.name: OnModifyDone001
 * @tc.desc: Test TextPattern OnModifyDone when frameNode is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, OnModifyDone001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    pattern->OnModifyDone();
    EXPECT_EQ(pattern->selectOverlayProxy_, nullptr);
}

/**
 * @tc.name: OnModifyDone002
 * @tc.desc: Test TextPattern OnModifyDone longPressEvent.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, OnModifyDone002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     * copyOption: CopyOptions::InApp
     */
    textLayoutProperty->UpdateCopyOption(CopyOptions::InApp);
    textLayoutProperty->UpdateContent(TEXT_CONTENT);

    /**
     * @tc.steps: step3. check the longPressEvent.
     * @tc.expected: longPressEvent is triggered
     */
    textPattern->OnModifyDone();
    EXPECT_NE(textPattern->longPressEvent_, nullptr);
    EXPECT_EQ(textPattern->textForDisplay_, TEXT_CONTENT);
}

/**
 * @tc.name: OnDirtyLayoutWrapperSwap001
 * @tc.desc: Test TextPattern OnDirtyLayoutWrapperSwap when skipMeasure is true.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, OnDirtyLayoutWrapperSwap001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    DirtySwapConfig config;
    config.skipMeasure = true;
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, AceType::MakeRefPtr<GeometryNode>(), frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    auto rowLayoutAlgorithm = pattern->CreateLayoutAlgorithm();
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    auto ret = pattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_FALSE(ret);
    EXPECT_EQ(pattern->selectOverlayProxy_, nullptr);
}

/**
 * @tc.name: OnDirtyLayoutWrapperSwap002
 * @tc.desc: Test TextPattern OnDirtyLayoutWrapperSwap when skipMeasure is fasle.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, OnDirtyLayoutWrapperSwap002, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    DirtySwapConfig config;
    config.skipMeasure = false;
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, AceType::MakeRefPtr<GeometryNode>(), frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    auto rowLayoutAlgorithm = pattern->CreateLayoutAlgorithm();
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    auto ret = pattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern->selectOverlayProxy_, nullptr);
}

/**
 * @tc.name: OnDirtyLayoutWrapperSwap003
 * @tc.desc: Test TextPattern OnDirtyLayoutWrapperSwap when skipMeasure is fasle and paragraph is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, OnDirtyLayoutWrapperSwap003, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;

    DirtySwapConfig config;
    config.skipMeasure = false;
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, AceType::MakeRefPtr<GeometryNode>(), frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);

    auto rowLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    TextStyle textStyle;
    LayoutConstraintF contentConstraint;
    auto ret =
        rowLayoutAlgorithm->CreateParagraphAndLayout(textStyle, "", contentConstraint, AceType::RawPtr(frameNode));
    EXPECT_TRUE(ret);
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    ret = pattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern->selectOverlayProxy_, nullptr);

    SelectOverlayInfo selectOverlayInfo;
    selectOverlayInfo.singleLineHeight = NODE_ID;
    auto root = AceType::MakeRefPtr<FrameNode>(ROOT_TAG, -1, AceType::MakeRefPtr<Pattern>(), true);
    auto selectOverlayManager = AceType::MakeRefPtr<SelectOverlayManager>(root);
    auto proxy = selectOverlayManager->CreateAndShowSelectOverlay(selectOverlayInfo, nullptr, false);
    pattern->selectOverlayProxy_ = proxy;
    ret = pattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: TextLayoutAlgorithm001
 * @tc.desc: Test TextLayoutAlgorithm.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutAlgorithm001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. spans is empty and call function
     */
    std::list<RefPtr<SpanItem>> spans1_;
    auto pManager_ = AceType::MakeRefPtr<ParagraphManager>();
    ASSERT_NE(pManager_, nullptr);
    auto textLayoutAlgorithm1 = AceType::MakeRefPtr<TextLayoutAlgorithm>(spans1_, pManager_, true);
    ASSERT_NE(textLayoutAlgorithm1, nullptr);
    /**
     * @tc.steps: step2. call function.
     */
    std::list<RefPtr<SpanItem>> spans2_;
    ConstructSpanItemList1(spans2_);
    EXPECT_EQ(spans2_.size(), 4);
    auto pManager_1 = AceType::MakeRefPtr<ParagraphManager>();
    ASSERT_NE(pManager_1, nullptr);
    auto textLayoutAlgorithm2 = AceType::MakeRefPtr<TextLayoutAlgorithm>(spans2_, pManager_1, true, true);
    ASSERT_NE(textLayoutAlgorithm2, nullptr);
    /**
     * @tc.steps: step3. call function.
     */
    std::list<RefPtr<SpanItem>> spans3_;
    ConstructSpanItemList1(spans3_);
    EXPECT_EQ(spans3_.size(), 4);
    RefPtr<SpanItem> span0 = nullptr;
    spans3_.emplace_back(span0);
    RefPtr<SpanItem> span1 = AceType::MakeRefPtr<SpanItem>();
    span1->content = "span1\n";
    spans3_.emplace_back(span1);
    auto pManager_2 = AceType::MakeRefPtr<ParagraphManager>();
    ASSERT_NE(pManager_2, nullptr);
    auto textLayoutAlgorithm3 = AceType::MakeRefPtr<TextLayoutAlgorithm>(spans3_, pManager_2, true, false);
    ASSERT_NE(textLayoutAlgorithm3, nullptr);
}

/**
 * @tc.name: TextLayoutAlgorithm002
 * @tc.desc: Test TextLayoutAlgorithm.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutAlgorithm002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. call function.
     */
    std::list<RefPtr<SpanItem>> spans_;
    ConstructSpanItemList1(spans_);
    EXPECT_EQ(spans_.size(), 4);
    void* voidPtr = static_cast<void*>(new char[0]);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    ASSERT_NE(pixelMap, nullptr);
    RefPtr<SpanItem> span1 = AceType::MakeRefPtr<SpanItem>();
    span1->content = "span1\n";
    spans_.emplace_back(span1);
    RefPtr<SpanItem> span2 = AceType::MakeRefPtr<SpanItem>();
    span2->content = "span2\n";
    spans_.emplace_back(span2);
    auto pManager_ = AceType::MakeRefPtr<ParagraphManager>();
    ASSERT_NE(pManager_, nullptr);
    auto textLayoutAlgorithm4 = AceType::MakeRefPtr<TextLayoutAlgorithm>(spans_, pManager_, true, false);
    ASSERT_NE(textLayoutAlgorithm4, nullptr);
}

/**
 * @tc.name: UpdateSensitiveContent001
 * @tc.desc: Test UpdateSensitiveContent.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, UpdateSensitiveContent001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init and Create function
     */
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    auto rowLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_NE(rowLayoutAlgorithm, nullptr);
    /**
     * @tc.steps: step2. call function.
     */
    std::string content("test123\n");
    rowLayoutAlgorithm->OnReset();
    auto temp = content;
    rowLayoutAlgorithm->UpdateSensitiveContent(temp);
    EXPECT_NE(temp, content);
}

/**
 * @tc.name: TextCreateParagraph001
 * @tc.desc: Test CreateParagraph.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextCreateParagraph001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init
     */
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    auto rowLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_NE(rowLayoutAlgorithm, nullptr);
    /**
     * @tc.steps: step2. call function.
     */
    pattern->isSensitive_ = true;
    pattern->GetHost()->SetPrivacySensitive(true);
    TextStyle textStyle;
    LayoutConstraintF contentConstraint;
    auto maxSize = MultipleParagraphLayoutAlgorithm::GetMaxMeasureSize(contentConstraint);
    auto ret = rowLayoutAlgorithm->CreateParagraph(textStyle, "", AceType::RawPtr(frameNode), maxSize.Width());
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: MeasureContent001
 * @tc.desc: Test MeasureContent.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, MeasureContent001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init
     */
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    auto rowLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_NE(rowLayoutAlgorithm, nullptr);
    /**
     * @tc.steps: step2. change param and call function.
     */
    LayoutConstraintF contentConstraint;
    rowLayoutAlgorithm->isSpanStringMode_ = true;
    rowLayoutAlgorithm->spanStringHasMaxLines_ = true;
    contentConstraint.maxSize.SetHeight(0.0f);
    auto ret = rowLayoutAlgorithm->MeasureContent(contentConstraint, AceType::RawPtr(frameNode));
    EXPECT_EQ(ret.has_value(), true);
    /**
     * @tc.steps: step3. change param and call function.
     */
    rowLayoutAlgorithm->isSpanStringMode_ = false;
    ret = rowLayoutAlgorithm->MeasureContent(contentConstraint, AceType::RawPtr(frameNode));
    EXPECT_EQ(ret.has_value(), true);
}

/**
 * @tc.name: ResetAiSpanTextStyle001
 * @tc.desc: Test ResetAiSpanTextStyle.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, ResetAiSpanTextStyle001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init
     */
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;

    DirtySwapConfig config;
    config.skipMeasure = false;
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, AceType::MakeRefPtr<GeometryNode>(), frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    auto rowLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_NE(rowLayoutAlgorithm, nullptr);
    /**
     * @tc.steps: step2. call function.
     */
    TextStyle textStyle;
    rowLayoutAlgorithm->ResetAiSpanTextStyle(frameNode, textStyle);
    EXPECT_EQ(textStyle.textDecoration_, TextDecoration::NONE);
}

/**
 * @tc.name: UpdateParagraphForAISpan001
 * @tc.desc: Test UpdateParagraphForAISpan
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, UpdateParagraphForAISpan001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init.
     */
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    auto rowLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_NE(rowLayoutAlgorithm, nullptr);
    TextStyle textStyle;
    ParagraphStyle paraStyle = { .direction = TextDirection::LTR,
        .align = textStyle.GetTextAlign(),
        .maxLines = textStyle.GetMaxLines(),
        .fontLocale = "zh-CN",
        .wordBreak = textStyle.GetWordBreak(),
        .textOverflow = textStyle.GetTextOverflow() };
    RefPtr<Paragraph> paragraph = Paragraph::Create(paraStyle, FontCollection::Current());
    ASSERT_NE(paragraph, nullptr);
    pattern->pManager_->AddParagraph({ .paragraph = paragraph, .start = 0, .end = 100 });
    /**
     * @tc.steps: step2. change param and call function.
     */
    std::map<int32_t, AISpan> aiSpanMap;
    AISpan aiSpan0;
    aiSpanMap[0] = aiSpan0;
    pattern->dataDetectorAdapter_->aiSpanMap_ = aiSpanMap;
    std::string textForAI = "";
    pattern->dataDetectorAdapter_->textForAI_ = textForAI;
    rowLayoutAlgorithm->UpdateParagraphForAISpan(textStyle, AceType::RawPtr(frameNode), paragraph);
    EXPECT_EQ(pattern->dataDetectorAdapter_->textForAI_, textForAI);
    /**
     * @tc.steps: step3. change param and call function.
     */
    AISpan aiSpan1;
    aiSpan1.start = -1;
    aiSpan1.end = 0;
    aiSpan1.content = SPAN_PHONE;
    aiSpan1.type = TextDataDetectType::PHONE_NUMBER;
    aiSpanMap[1] = aiSpan1;
    pattern->dataDetectorAdapter_->aiSpanMap_ = aiSpanMap;
    pattern->dataDetectorAdapter_->textForAI_ = textForAI;
    rowLayoutAlgorithm->UpdateParagraphForAISpan(textStyle, AceType::RawPtr(frameNode), paragraph);
    EXPECT_EQ(pattern->dataDetectorAdapter_->textForAI_, textForAI);
    /**
     * @tc.steps: step4. change param and call function.
     */
    AISpan aiSpan2;
    aiSpan2.start = 2;
    aiSpan2.end = 5;
    aiSpan2.content = SPAN_PHONE;
    aiSpan2.type = TextDataDetectType::PHONE_NUMBER;
    aiSpanMap[2] = aiSpan2;
    pattern->dataDetectorAdapter_->aiSpanMap_ = aiSpanMap;
    textForAI = "Test1234";
    pattern->dataDetectorAdapter_->textForAI_ = textForAI;
    rowLayoutAlgorithm->UpdateParagraphForAISpan(textStyle, AceType::RawPtr(frameNode), paragraph);
    EXPECT_EQ(pattern->dataDetectorAdapter_->textForAI_, textForAI);
}

/**
 * @tc.name: GrayDisplayAISpan001
 * @tc.desc: Test GrayDisplayAISpan
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, GrayDisplayAISpan001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init.
     */
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    DirtySwapConfig config;
    config.skipMeasure = false;
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, AceType::MakeRefPtr<GeometryNode>(), frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    auto rowLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_NE(rowLayoutAlgorithm, nullptr);
    TextStyle textStyle;
    ParagraphStyle paragraphStyle = { .direction = TextDirection::LTR,
        .align = textStyle.GetTextAlign(),
        .maxLines = textStyle.GetMaxLines(),
        .fontLocale = "zh-CN",
        .wordBreak = textStyle.GetWordBreak(),
        .textOverflow = textStyle.GetTextOverflow() };
    RefPtr<Paragraph> paragraph = Paragraph::Create(paragraphStyle, FontCollection::Current());
    ASSERT_NE(paragraph, nullptr);
    pattern->pManager_->AddParagraph({ .paragraph = paragraph, .start = 0, .end = 100 });
    /**
     * @tc.steps: step2. change param and call function.
     * spanStart <= dragStart && spanEnd >= dragStart && spanEnd <= dragEnd
    */
    DragSpanPosition dragSpanPosition;
    dragSpanPosition.dragStart = 2;
    dragSpanPosition.dragEnd = 4;
    dragSpanPosition.spanStart = 1;
    dragSpanPosition.spanEnd = 3;
    std::string textForAI = "Test1234";
    auto wTextForAI = StringUtils::ToWstring(textForAI);
    pattern->dataDetectorAdapter_->textForAI_ = textForAI;
    rowLayoutAlgorithm->GrayDisplayAISpan(dragSpanPosition, wTextForAI, textStyle, true, paragraph);
    EXPECT_EQ(pattern->dataDetectorAdapter_->textForAI_, textForAI);
    /**
     * @tc.steps: step3. change param and call function.
     * spanStart >= dragStart && spanEnd <= dragEnd
    */
    dragSpanPosition.dragStart = 1;
    dragSpanPosition.dragEnd = 4;
    dragSpanPosition.spanStart = 2;
    dragSpanPosition.spanEnd = 3;
    textForAI = "Test1234";
    wTextForAI = StringUtils::ToWstring(textForAI);
    pattern->dataDetectorAdapter_->textForAI_ = textForAI;
    rowLayoutAlgorithm->GrayDisplayAISpan(dragSpanPosition, wTextForAI, textStyle, true, paragraph);
    EXPECT_EQ(pattern->dataDetectorAdapter_->textForAI_, textForAI);
}

/**
 * @tc.name: GrayDisplayAISpan002
 * @tc.desc: Test GrayDisplayAISpan
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, GrayDisplayAISpan002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init.
     */
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    DirtySwapConfig config;
    config.skipMeasure = false;
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, AceType::MakeRefPtr<GeometryNode>(), frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    auto rowLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_NE(rowLayoutAlgorithm, nullptr);
    TextStyle textStyle;
    ParagraphStyle paragraphStyle = { .direction = TextDirection::LTR,
        .align = textStyle.GetTextAlign(),
        .maxLines = textStyle.GetMaxLines(),
        .fontLocale = "zh-CN",
        .wordBreak = textStyle.GetWordBreak(),
        .textOverflow = textStyle.GetTextOverflow() };
    RefPtr<Paragraph> paragraph = Paragraph::Create(paragraphStyle, FontCollection::Current());
    ASSERT_NE(paragraph, nullptr);
    pattern->pManager_->AddParagraph({ .paragraph = paragraph, .start = 0, .end = 100 });
    /**
     * @tc.steps: step2. change param and call function.
     * spanStart <= dragStart && spanEnd >= dragEnd
    */
    DragSpanPosition dragSpanPosition;
    dragSpanPosition.dragStart = 2;
    dragSpanPosition.dragEnd = 3;
    dragSpanPosition.spanStart = 1;
    dragSpanPosition.spanEnd = 4;
    std::string textForAI = "Test1234";
    auto wTextForAI = StringUtils::ToWstring(textForAI);
    pattern->dataDetectorAdapter_->textForAI_ = textForAI;
    rowLayoutAlgorithm->GrayDisplayAISpan(dragSpanPosition, wTextForAI, textStyle, true, paragraph);
    EXPECT_EQ(pattern->dataDetectorAdapter_->textForAI_, textForAI);
    /**
     * @tc.steps: step3. change param and call function.
     * default
    */
    dragSpanPosition.dragStart = 1;
    dragSpanPosition.dragEnd = 3;
    dragSpanPosition.spanStart = 2;
    dragSpanPosition.spanEnd = 4;
    textForAI = "Test1234";
    wTextForAI = StringUtils::ToWstring(textForAI);
    pattern->dataDetectorAdapter_->textForAI_ = textForAI;
    rowLayoutAlgorithm->GrayDisplayAISpan(dragSpanPosition, wTextForAI, textStyle, true, paragraph);
    EXPECT_EQ(pattern->dataDetectorAdapter_->textForAI_, textForAI);
}

/**
 * @tc.name: GrayDisplayAISpan003
 * @tc.desc: Test GrayDisplayAISpan
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, GrayDisplayAISpan003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init.
     */
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    DirtySwapConfig config;
    config.skipMeasure = false;
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, AceType::MakeRefPtr<GeometryNode>(), frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    auto rowLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_NE(rowLayoutAlgorithm, nullptr);
    TextStyle textStyle;
    ParagraphStyle paragraphStyle = { .direction = TextDirection::LTR,
        .align = textStyle.GetTextAlign(),
        .maxLines = textStyle.GetMaxLines(),
        .fontLocale = "zh-CN",
        .wordBreak = textStyle.GetWordBreak(),
        .textOverflow = textStyle.GetTextOverflow() };
    RefPtr<Paragraph> paragraph = Paragraph::Create(paragraphStyle, FontCollection::Current());
    ASSERT_NE(paragraph, nullptr);
    pattern->pManager_->AddParagraph({ .paragraph = paragraph, .start = 0, .end = 100 });
    /**
     * @tc.steps: step2. change param and call function.
     * spanStart <= dragStart && spanEnd >= dragEnd
    */
    DragSpanPosition dragSpanPosition;
    dragSpanPosition.dragStart = 2;
    dragSpanPosition.dragEnd = 15;
    dragSpanPosition.spanStart = 1;
    dragSpanPosition.spanEnd = 20;
    std::string textForAI = "Test1234";
    auto wTextForAI = StringUtils::ToWstring(textForAI);
    pattern->dataDetectorAdapter_->textForAI_ = textForAI;
    rowLayoutAlgorithm->GrayDisplayAISpan(dragSpanPosition, wTextForAI, textStyle, true, paragraph);
    EXPECT_EQ(pattern->dataDetectorAdapter_->textForAI_, textForAI);
}

/**
 * @tc.name: BuildTextRaceParagraph001
 * @tc.desc: Test BuildTextRaceParagraph
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, BuildTextRaceParagraph001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init and Create TextLayoutAlgorithm
     */
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    DirtySwapConfig config;
    config.skipMeasure = false;
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, AceType::MakeRefPtr<GeometryNode>(), frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    auto rowLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_NE(rowLayoutAlgorithm, nullptr);
    /**
     * @tc.steps: step2. change param and call function.
     */
    LayoutConstraintF contentConstraint;
    contentConstraint.selfIdealSize.width_ = 100.0f;
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextLayoutProperty> textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutProperty);
    ASSERT_NE(textLayoutProperty, nullptr);
    TextStyle textStyle;
    textStyle.adaptTextSize_ = true;
    ParagraphStyle paragraphStyle = { .direction = TextDirection::LTR,
        .align = textStyle.GetTextAlign(),
        .maxLines = textStyle.GetMaxLines(),
        .fontLocale = "zh-CN",
        .wordBreak = textStyle.GetWordBreak(),
        .textOverflow = textStyle.GetTextOverflow() };
    RefPtr<Paragraph> paragraph = Paragraph::Create(paragraphStyle, FontCollection::Current());
    ASSERT_NE(paragraph, nullptr);
    rowLayoutAlgorithm->paragraphManager_->AddParagraph(
        { .paragraph = paragraph, .paragraphStyle = paragraphStyle, .start = 50.0f, .end = 150.0f });
    rowLayoutAlgorithm->BuildTextRaceParagraph(
        textStyle, textLayoutProperty, contentConstraint, AceType::RawPtr(frameNode));
    EXPECT_EQ(rowLayoutAlgorithm->textStyle_, textStyle);
    /**
     * @tc.steps: step3. change param and call function.
     */
    TextStyle textStyle1;
    textStyle1.adaptTextSize_ = true;
    LayoutConstraintF contentConstraint1;
    contentConstraint1.selfIdealSize.width_ = std::nullopt;
    rowLayoutAlgorithm->BuildTextRaceParagraph(
        textStyle1, textLayoutProperty, contentConstraint1, AceType::RawPtr(frameNode));
    EXPECT_EQ(rowLayoutAlgorithm->textStyle_, textStyle1);
}

/**
 * @tc.name: BeforeCreateLayoutWrapper001
 * @tc.desc: Test TextPattern BeforeCreateLayoutWrapper when paragraph is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, BeforeCreateLayoutWrapper001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;

    auto rowLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    TextStyle textStyle;
    LayoutConstraintF contentConstraint;
    auto ret =
        rowLayoutAlgorithm->CreateParagraphAndLayout(textStyle, "", contentConstraint, AceType::RawPtr(frameNode));
    EXPECT_TRUE(ret);
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, AceType::MakeRefPtr<GeometryNode>(), frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    pattern->BeforeCreateLayoutWrapper();
    EXPECT_EQ(pattern->selectOverlayProxy_, nullptr);
}

/**
 * @tc.name: BeforeCreateLayoutWrapper002
 * @tc.desc: Test TextPattern BeforeCreateLayoutWrapper when frameNode child is empty.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, BeforeCreateLayoutWrapper002, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);

    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;

    auto rowLayoutAlgorithm = pattern->CreateLayoutAlgorithm();
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, AceType::MakeRefPtr<GeometryNode>(), frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    pattern->BeforeCreateLayoutWrapper();
    EXPECT_EQ(pattern->selectOverlayProxy_, nullptr);
}

/**
 * @tc.name: BeforeCreateLayoutWrapper003
 * @tc.desc: Test TextPattern BeforeCreateLayoutWrapper when paragraph is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, BeforeCreateLayoutWrapper003, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;

    auto rowLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    TextStyle textStyle;
    LayoutConstraintF contentConstraint;
    auto ret =
        rowLayoutAlgorithm->CreateParagraphAndLayout(textStyle, "", contentConstraint, AceType::RawPtr(frameNode));
    EXPECT_TRUE(ret);

    DirtySwapConfig config;
    config.skipMeasure = false;
    auto layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, AceType::MakeRefPtr<GeometryNode>(), nullptr);
    ASSERT_NE(layoutWrapper, nullptr);
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    ret = pattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_TRUE(ret);
    pattern->BeforeCreateLayoutWrapper();
    EXPECT_EQ(pattern->selectOverlayProxy_, nullptr);
}

/**
 * @tc.name: BeforeCreateLayoutWrapper004
 * @tc.desc: Test TextPattern BeforeCreateLayoutWrapper when frameNode child is not empty.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, BeforeCreateLayoutWrapper004, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);

    auto patternChild = AceType::MakeRefPtr<TextPattern>();
    auto frameNodeChild = FrameNode::CreateFrameNode("Test", 1, patternChild);
    ASSERT_NE(frameNodeChild, nullptr);

    frameNode->AddChild(frameNodeChild);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;

    auto rowLayoutAlgorithm = pattern->CreateLayoutAlgorithm();
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, AceType::MakeRefPtr<GeometryNode>(), frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    pattern->BeforeCreateLayoutWrapper();
    EXPECT_EQ(pattern->selectOverlayProxy_, nullptr);
}

/**
 * @tc.name: OnHandleMove001
 * @tc.desc: Test TextPattern OnHandleMove when SelectOverlayProxy is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, OnHandleMove001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    pattern->AttachToFrameNode(frameNode);
    /**
     * @tc.steps: step1. construct a SelectOverlayManager
     */
    SelectOverlayInfo selectOverlayInfo;
    selectOverlayInfo.singleLineHeight = NODE_ID;
    auto root = AceType::MakeRefPtr<FrameNode>(ROOT_TAG, -1, AceType::MakeRefPtr<Pattern>(), true);
    auto selectOverlayManager = AceType::MakeRefPtr<SelectOverlayManager>(root);

    /**
     * @tc.steps: step2. call CreateAndShowSelectOverlay
     * @tc.expected: return the proxy which has the right SelectOverlayId
     */
    auto proxy = selectOverlayManager->CreateAndShowSelectOverlay(selectOverlayInfo, nullptr, false);
    pattern->selectOverlayProxy_ = proxy;
    EXPECT_NE(pattern->selectOverlayProxy_, nullptr);
}

/**
 * @tc.name: TextCreateParagraph002
 * @tc.desc: Correctly create Paragraph only Set content.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextCreateParagraph002, TestSize.Level1)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, GetMaxWidth).WillRepeatedly(Return(150));
    EXPECT_CALL(*paragraph, GetHeight).WillRepeatedly(Return(50));
    EXPECT_CALL(*paragraph, GetAlphabeticBaseline).WillOnce(Return(0));
    EXPECT_CALL(*paragraph, PushStyle);
    EXPECT_CALL(*paragraph, Build);
    // investigate why it layouts twice
    EXPECT_CALL(*paragraph, Layout).Times(2);

    /**
     * @tc.steps: step1. create textFrameNode.
     */

    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set Content.
     */

    textLayoutProperty->UpdateContent(CREATE_VALUE);

    /**
     * @tc.steps: step3. create textLayoutAlgorithm.
     */

    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    EXPECT_FALSE(textLayoutAlgorithm == nullptr);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));

    textLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
}

/**
 * @tc.name: TextLayoutTest001
 * @tc.desc: Set content , width and height to Text and the check result.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutTest001, TestSize.Level1)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, GetMaxWidth).WillRepeatedly(Return(TEXT_WIDTH));
    EXPECT_CALL(*paragraph, GetHeight).WillRepeatedly(Return(150));
    EXPECT_CALL(*paragraph, Layout);
    /**
     * @tc.steps: step1. create textFrameNode.
     */

    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     * content: CREATE_VALUE
     * width: 100.0f
     * height: 75.0f
     */

    textLayoutProperty->UpdateContent(CREATE_VALUE);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.selfIdealSize.SetSize(TEXT_SIZE);
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;

    /**
     * @tc.steps: step3. create textLayoutAlgorithm.
     */

    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
}

/**
 * @tc.name: TextLayoutTest002
 * @tc.desc: set baselineOffset and maxsize of Text check height
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutTest002, TestSize.Level1)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, GetLongestLine).WillRepeatedly(Return(100));
    EXPECT_CALL(*paragraph, GetMaxWidth).WillRepeatedly(Return(150));
    EXPECT_CALL(*paragraph, GetHeight).WillRepeatedly(Return(50));
    EXPECT_CALL(*paragraph, Layout).Times(2);
    /**
     * @tc.steps: step1. create textFrameNode.
     */

    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     * baselineOffset: 20_px
     */

    textLayoutProperty->UpdateContent(CREATE_VALUE);
    textLayoutProperty->UpdateBaselineOffset(BASELINE_OFFSET_VALUE);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;

    /**
     * @tc.steps: step3. create textLayoutAlgorithm.
     */

    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    auto sizeX =
        textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));

    /**
     * @tc.steps: step4. check the size.
     */

    EXPECT_EQ(sizeX->Height(), BASELINE_OFFSET_VALUE.Value() + 50.0);
}

/**
 * @tc.name: TextLayoutTest003
 * @tc.desc: set minFontSize and maxFontSize of Text check then check
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutTest003, TestSize.Level1)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, Build).Times(1);
    /**
     * @tc.steps: step1. create textFrameNode.
     */

    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     * minFontSize: 50_px
     * macFontSize: 80_px
     */

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;

    TextStyle textStyle;
    textStyle.SetTextAlign(TextAlign::CENTER);
    textStyle.SetAdaptTextSize(ADAPT_MIN_FONT_SIZE_VALUE, ADAPT_MAX_FONT_SIZE_VALUE_S);
    textStyle.SetAdaptFontSizeStep(ADAPT_FONT_SIZE_STEP_VALUE);

    auto pipeline = textFrameNode->GetContextRefPtr();

    /**
     * @tc.steps: step3. create textLayoutAlgorithm.
     */

    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    auto result = textLayoutAlgorithm->AdaptMinTextSize(
        textStyle, CREATE_VALUE, parentLayoutConstraint, AceType::RawPtr(textFrameNode));

    /**
     * @tc.steps: step4. check the fontSize.
     */

    EXPECT_TRUE(result);
}

/**
 * @tc.name: TextLayoutTest004
 * @tc.desc: set minFontSize and maxFontSize of Text check then check
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutTest004, TestSize.Level1)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, Build).Times(1);
    /**
     * @tc.steps: step1. create textFrameNode.
     */

    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     * minFontSize: 50_px
     * macFontSize: 80_px
     */

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;

    TextStyle textStyle;
    textStyle.SetMaxLines(MAX_LINES);
    textStyle.SetTextAlign(TextAlign::CENTER);
    textStyle.SetAdaptTextSize(ADAPT_MIN_FONT_SIZE_VALUE, ADAPT_MAX_FONT_SIZE_VALUE_S);
    textStyle.SetAdaptFontSizeStep(ADAPT_FONT_SIZE_STEP_VALUE);

    auto pipeline = textFrameNode->GetContextRefPtr();

    /**
     * @tc.steps: step3. create textLayoutAlgorithm.
     */

    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    auto result = textLayoutAlgorithm->AdaptMinTextSize(
        textStyle, CREATE_VALUE, parentLayoutConstraint, AceType::RawPtr(textFrameNode));

    /**
     * @tc.steps: step4. check the fontSize.
     */

    EXPECT_TRUE(result);
}

/**
 * @tc.name: TextLayoutTest005
 * @tc.desc: set minFontSize and maxFontSize of Text check then check when maxFontSize >= minFontSize
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutTest005, TestSize.Level1)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, Build).Times(1);
    /**
     * @tc.steps: step1. create textFrameNode.
     */

    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     * minFontSize: 50_px
     * macFontSize: 80_px
     */

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;

    TextStyle textStyle;
    textStyle.SetMaxLines(MAX_LINES);
    textStyle.SetTextAlign(TextAlign::CENTER);
    textStyle.SetAdaptTextSize(ADAPT_MAX_FONT_SIZE_VALUE_S, ADAPT_MIN_FONT_SIZE_VALUE);
    textStyle.SetAdaptFontSizeStep(ADAPT_FONT_SIZE_STEP_VALUE);

    auto pipeline = textFrameNode->GetContextRefPtr();

    /**
     * @tc.steps: step3. create textLayoutAlgorithm.
     */

    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    auto result = textLayoutAlgorithm->AdaptMinTextSize(
        textStyle, CREATE_VALUE, parentLayoutConstraint, AceType::RawPtr(textFrameNode));

    /**
     * @tc.steps: step4. check the fontSize.
     */

    EXPECT_TRUE(result);
}

/**
 * @tc.name: TextLayoutTest006
 * @tc.desc: set baselineOffset and maxsize of Text check height when paragraph_ is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutTest006, TestSize.Level1)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, GetLongestLine).WillRepeatedly(Return(100));
    EXPECT_CALL(*paragraph, GetMaxWidth).WillRepeatedly(Return(150));
    EXPECT_CALL(*paragraph, GetHeight).WillRepeatedly(Return(50));
    EXPECT_CALL(*paragraph, AddText).Times(2);
    EXPECT_CALL(*paragraph, Layout).Times(3);
    /**
     * @tc.steps: step1. create textFrameNode.
     */

    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     * baselineOffset: 20_px
     */

    textLayoutProperty->UpdateContent(CREATE_VALUE);
    textLayoutProperty->UpdateBaselineOffset(BASELINE_OFFSET_VALUE);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;

    /**
     * @tc.steps: step3. create textLayoutAlgorithm.
     */

    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    TextStyle textStyle;
    LayoutConstraintF contentConstraint;
    auto ret =
        textLayoutAlgorithm->CreateParagraphAndLayout(textStyle, "", contentConstraint, AceType::RawPtr(textFrameNode));
    EXPECT_TRUE(ret);
    auto sizeX =
        textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));

    /**
     * @tc.steps: step4. check the size.
     */

    EXPECT_EQ(sizeX->Height(), BASELINE_OFFSET_VALUE.Value() + 50.0);
}

/**
 * @tc.name: TextLayoutTest007
 * @tc.desc: set baselineOffset and maxsize of Text check height when AdaptTextSize is disable.
 *           contentConstraint.maxSize.IsPositive() is false.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */

    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     * baselineOffset: 20_px
     */

    textLayoutProperty->UpdateContent(CREATE_VALUE);
    textLayoutProperty->UpdateBaselineOffset(BASELINE_OFFSET_VALUE);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize.SetHeight(100.0);

    /**
     * @tc.steps: step3. create textLayoutAlgorithm.
     */

    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    TextStyle textStyle;
    Dimension adaptMinFontSize(DIMENSION, DimensionUnit::PERCENT);
    textStyle.SetAdaptMinFontSize(adaptMinFontSize);
    auto ret = textLayoutAlgorithm->CreateParagraphAndLayout(
        textStyle, "", parentLayoutConstraint, AceType::RawPtr(textFrameNode));
    EXPECT_TRUE(ret);
    auto sizeX =
        textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));

    /**
     * @tc.steps: step4. check the size.
     */
    EXPECT_EQ(sizeX, SizeF(0.0f, 20.0f));
}

/**
 * @tc.name: TextLayoutTest008
 * @tc.desc: set baselineOffset and maxsize of Text check height when AdaptTextSize is disable.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutTest008, TestSize.Level1)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, GetLongestLine).WillRepeatedly(Return(100));
    /**
     * @tc.steps: step1. create textFrameNode.
     */

    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     * baselineOffset: 20_px
     */

    textLayoutProperty->UpdateContent(CREATE_VALUE);
    textLayoutProperty->UpdateBaselineOffset(BASELINE_OFFSET_VALUE);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.selfIdealSize.SetWidth(-1.0);
    parentLayoutConstraint.maxSize.SetHeight(1.0);
    parentLayoutConstraint.maxSize.SetWidth(1.0);

    /**
     * @tc.steps: step3. create textLayoutAlgorithm.
     */

    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    TextStyle textStyle;
    Dimension adaptMinFontSize(DIMENSION, DimensionUnit::PERCENT);
    textStyle.SetAdaptMinFontSize(adaptMinFontSize);
    auto ret = textLayoutAlgorithm->CreateParagraphAndLayout(
        textStyle, "", parentLayoutConstraint, AceType::RawPtr(textFrameNode));
    EXPECT_TRUE(ret);
    auto sizeX =
        textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));

    /**
     * @tc.steps: step4. check the size.
     */

    EXPECT_EQ(sizeX->Height(), 1.0);
}

/**
 * @tc.name: ToJsonValue001
 * @tc.desc: Test TextLayoutProperty ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, ToJsonValue001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. run ToJsonValue().
     */
    auto json = std::make_unique<JsonValue>();
    textLayoutProperty->ToJsonValue(json, filter);
}

/**
 * @tc.name: ToJsonValue002
 * @tc.desc: Test TextLayoutProperty ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, ToJsonValue002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. run ToJsonValue().
     */
    Dimension dim(DIMENSION);
    textLayoutProperty->UpdateAdaptMinFontSize(dim);
    textLayoutProperty->UpdateAdaptMaxFontSize(dim);
    textLayoutProperty->UpdateLetterSpacing(dim);
    textLayoutProperty->UpdateTextBaseline(TextBaseline::IDEOGRAPHIC);
    auto json = std::make_unique<JsonValue>();
    textLayoutProperty->ToJsonValue(json, filter);
}

/**
 * @tc.name: ToJsonValue003
 * @tc.desc: Test TextLayoutProperty ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, ToJsonValue003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. run ToJsonValue().
     */
    Dimension dim(DIMENSION);
    textLayoutProperty->UpdateAdaptMinFontSize(dim);
    textLayoutProperty->UpdateAdaptMaxFontSize(dim);
    textLayoutProperty->UpdateLetterSpacing(dim);
    textLayoutProperty->UpdateTextBaseline(TextBaseline::TOP);
    auto json = std::make_unique<JsonValue>();
    textLayoutProperty->ToJsonValue(json, filter);
}

/**
 * @tc.name: ToJsonValue004
 * @tc.desc: Test TextLayoutProperty ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, ToJsonValue004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. run ToJsonValue().
     */
    Dimension dim(DIMENSION);
    textLayoutProperty->UpdateAdaptMinFontSize(dim);
    textLayoutProperty->UpdateAdaptMaxFontSize(dim);
    textLayoutProperty->UpdateLetterSpacing(dim);
    textLayoutProperty->UpdateTextBaseline(TextBaseline::BOTTOM);
    auto json = std::make_unique<JsonValue>();
    textLayoutProperty->ToJsonValue(json, filter);
}

/**
 * @tc.name: ToJsonValue005
 * @tc.desc: Test TextLayoutProperty ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, ToJsonValue005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. run ToJsonValue().
     */
    Dimension dim(DIMENSION);
    textLayoutProperty->UpdateAdaptMinFontSize(dim);
    textLayoutProperty->UpdateAdaptMaxFontSize(dim);
    textLayoutProperty->UpdateLetterSpacing(dim);
    textLayoutProperty->UpdateTextBaseline(TextBaseline::MIDDLE);
    auto json = std::make_unique<JsonValue>();
    textLayoutProperty->ToJsonValue(json, filter);
}

/**
 * @tc.name: ToJsonValue006
 * @tc.desc: Test TextLayoutProperty ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, ToJsonValue006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. run ToJsonValue().
     */
    Dimension dim(DIMENSION);
    textLayoutProperty->UpdateAdaptMinFontSize(dim);
    textLayoutProperty->UpdateAdaptMaxFontSize(dim);
    textLayoutProperty->UpdateLetterSpacing(dim);
    textLayoutProperty->UpdateTextBaseline(TextBaseline::HANGING);
    auto json = std::make_unique<JsonValue>();
    textLayoutProperty->ToJsonValue(json, filter);
}

/**
 * @tc.name: ToJsonValue007
 * @tc.desc: Test textPattern ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, ToJsonValue007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    TextModelNG textModelNG;
    textModelNG.Create("");
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    auto pattern = frameNode->GetPattern<TextPattern>();
    auto json = JsonUtil::Create(true);
    /**
     * @tc.steps: step2. expect default textDetectEnable_ false.
     */
    pattern->SetTextDetectEnable(true);
    pattern->ToJsonValue(json, filter);
    EXPECT_EQ(json->GetString("enableDataDetector"), "true");
}

/**
 * @tc.name: DidExceedMaxLines001
 * @tc.desc: Test TextLayoutAlgorithm DidExceedMaxLines.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, DidExceedMaxLines001, TestSize.Level1)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, Layout);
    EXPECT_CALL(*paragraph, DidExceedMaxLines).WillOnce(Return(true));
    /**
     * @tc.steps: step1. create textFrameNode.
     */

    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     * baselineOffset: 20_px
     */

    textLayoutProperty->UpdateContent(CREATE_VALUE);
    textLayoutProperty->UpdateBaselineOffset(BASELINE_OFFSET_VALUE);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;

    /**
     * @tc.steps: step3. create textLayoutAlgorithm.
     */

    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    TextStyle textStyle;
    LayoutConstraintF contentConstraint;
    auto ret =
        textLayoutAlgorithm->CreateParagraphAndLayout(textStyle, "", contentConstraint, AceType::RawPtr(textFrameNode));
    EXPECT_TRUE(ret);
    SizeF maxSize;
    ret = textLayoutAlgorithm->DidExceedMaxLines(maxSize);

    /**
     * @tc.steps: step4. check the size.
     */

    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DidExceedMaxLines002
 * @tc.desc: Test TextLayoutAlgorithm DidExceedMaxLines.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, DidExceedMaxLines002, TestSize.Level1)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, DidExceedMaxLines).WillOnce(Return(true));
    /**
     * @tc.steps: step1. create textFrameNode.
     */

    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     * baselineOffset: 20_px
     */

    textLayoutProperty->UpdateContent(CREATE_VALUE);
    textLayoutProperty->UpdateBaselineOffset(BASELINE_OFFSET_VALUE);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;

    /**
     * @tc.steps: step3. create textLayoutAlgorithm.
     */

    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    TextStyle textStyle;
    textStyle.SetMaxLines(MAX_LINES);
    LayoutConstraintF contentConstraint;
    auto ret =
        textLayoutAlgorithm->CreateParagraphAndLayout(textStyle, "", contentConstraint, AceType::RawPtr(textFrameNode));
    EXPECT_TRUE(ret);
    SizeF maxSize;
    ret = textLayoutAlgorithm->DidExceedMaxLines(maxSize);

    /**
     * @tc.steps: step4. check the size.
     */

    EXPECT_TRUE(ret);
}

/**
 * @tc.name: TextLayoutAlgorithmTest001
 * @tc.desc: test text_layout_algorithm.cpp:Set textHeightAdaptivePolicy to MIN_FONT_SIZE_FIRST
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutAlgorithmTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    textPattern->contentMod_ = AceType::MakeRefPtr<TextContentModifier>(std::optional<TextStyle>(TextStyle()));
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     */
    textLayoutProperty->UpdateContent(CREATE_VALUE);
    textLayoutProperty->UpdateHeightAdaptivePolicy(TextHeightAdaptivePolicy::MIN_FONT_SIZE_FIRST);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.selfIdealSize.SetSize(TEXT_SIZE);
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;

    /**
     * @tc.steps: step3. create textLayoutAlgorithm and call MeasureContent function.
     * @tc.expected: The width of the return value of MeasureContent is equal to maxWidth of paragraph_
     */
    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    // set TextHeightAdaptivePolicy MAX_LINES_FIRST
    textLayoutProperty->UpdateHeightAdaptivePolicy(TextHeightAdaptivePolicy::MAX_LINES_FIRST);
    // MockTxtParagraph::SetCanConstruct(false);
    textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));
    // set SetCanConstruct true
    // MockTxtParagraph::SetCanConstruct(true);
    auto contentSize =
        textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->baselineOffset_ = BASE_LINE_OFFSET;
    textLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(contentSize.value().Width(), textLayoutAlgorithm->paragraphManager_->GetMaxWidth());
}

/**
 * @tc.name: TextLayoutAlgorithmTest002
 * @tc.desc: test text_layout_algorithm.cpp:Set textHeightAdaptivePolicy to LAYOUT_CONSTRAINT_FIRST and set lineHeight
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutAlgorithmTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    auto textLayoutProperty = textFrameNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textLayoutProperty);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     */
    textLayoutProperty->UpdateContent(CREATE_VALUE);
    textLayoutProperty->UpdateHeightAdaptivePolicy(TextHeightAdaptivePolicy::LAYOUT_CONSTRAINT_FIRST);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.selfIdealSize.SetSize(TEXT_SIZE);
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    textLayoutProperty->UpdateAdaptMinFontSize(ADAPT_MIN_FONT_SIZE_VALUE);
    textLayoutProperty->UpdateLineHeight(ADAPT_LINE_HEIGHT_VALUE);
    textLayoutProperty->UpdateFontSize(ADAPT_FONT_SIZE_VALUE);
    textLayoutProperty->UpdateFontWeight(Ace::FontWeight::W200);
    textLayoutProperty->UpdateTextColor(TEXT_COLOR_VALUE);
    Shadow textShadow;
    textLayoutProperty->UpdateTextShadow({ textShadow });
    textLayoutProperty->UpdateTextDecorationColor(TEXT_COLOR_VALUE);
    textLayoutProperty->UpdateTextDecoration(TextDecoration::OVERLINE);
    textLayoutProperty->UpdateBaselineOffset(BASELINE_OFFSET_VALUE);

    /**
     * @tc.steps: step3. create textLayoutAlgorithm and call MeasureContent/SetPropertyToModifier function.
     * @tc.expected: The width of the return value of MeasureContent is equal to maxWidth of paragraph_
     */
    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    auto contentSize =
        textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    auto frameNode = layoutWrapper->GetHostNode();
    auto pipeline = frameNode->GetContextRefPtr();
    TextStyle textStyle = CreateTextStyleUsingTheme(
        textLayoutProperty->GetFontStyle(), textLayoutProperty->GetTextLineStyle(), pipeline->GetTheme<TextTheme>());
    textPattern->contentMod_ = AceType::MakeRefPtr<TextContentModifier>(std::optional<TextStyle>(std::move(textStyle)));
    auto contentModifier = textPattern->GetContentModifier();
    textLayoutAlgorithm->SetPropertyToModifier(textLayoutProperty, contentModifier, textStyle);
    EXPECT_EQ(contentSize.value().Width(), textLayoutAlgorithm->paragraphManager_->GetMaxWidth());
}

/**
 * @tc.name: TextLayoutAlgorithmTest003
 * @tc.desc: test text_layout_algorithm.cpp:Set textHeightAdaptivePolicy to MIN_FONT_SIZE_FIRST, minFontSize and
 *           fontSize are equal
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutAlgorithmTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     */
    textLayoutProperty->UpdateContent(CREATE_VALUE);
    textLayoutProperty->UpdateHeightAdaptivePolicy(TextHeightAdaptivePolicy::MIN_FONT_SIZE_FIRST);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.selfIdealSize.SetSize(TEXT_SIZE);
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    textLayoutProperty->UpdateAdaptMinFontSize(ADAPT_MIN_FONT_SIZE_VALUE);
    textLayoutProperty->UpdateFontSize(ADAPT_MIN_FONT_SIZE_VALUE);
    textLayoutProperty->UpdateLineHeight(ADAPT_LINE_HEIGHT_VALUE);

    /**
     * @tc.steps: step3. create textLayoutAlgorithm and call MeasureContent function.
     * @tc.expected: The width of the return value of MeasureContent is equal to maxWidth of paragraph_
     */
    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    auto contentSize =
        textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(contentSize.value().Width(), textLayoutAlgorithm->paragraphManager_->GetMaxWidth());
}

/**
 * @tc.name: TextLayoutAlgorithmTest004
 * @tc.desc: test text_layout_algorithm.cpp:Set textHeightAdaptivePolicy to LAYOUT_CONSTRAINT_FIRST and set lineHeight,
 *     the height of text is bigger than the height of container
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutAlgorithmTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     */
    textLayoutProperty->UpdateContent(CREATE_VALUE);
    textLayoutProperty->UpdateHeightAdaptivePolicy(TextHeightAdaptivePolicy::LAYOUT_CONSTRAINT_FIRST);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.selfIdealSize.SetSize(TEXT_SIZE);
    parentLayoutConstraint.maxSize = CONTAINER_LOW_SIZE;
    textLayoutProperty->UpdateAdaptMinFontSize(ADAPT_OVER_MIN_FONT_SIZE_VALUE);
    textLayoutProperty->UpdateLineHeight(ADAPT_LINE_HEIGHT_VALUE);

    /**
     * @tc.steps: step3. create textLayoutAlgorithm and call MeasureContent function.
     * @tc.expected: The width of the return value of MeasureContent is equal to maxWidth of paragraph_
     */
    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    auto contentSize =
        textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(contentSize.value().Width(), textLayoutAlgorithm->paragraphManager_->GetMaxWidth());
}

/**
 * @tc.name: TextLayoutAlgorithmTest005
 * @tc.desc: test text_layout_algorithm.cpp:Set textHeightAdaptivePolicy to LAYOUT_CONSTRAINT_FIRST and set lineHeight,
 *     the height of text is bigger than the height of container,set maxlines.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutAlgorithmTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     */
    textLayoutProperty->UpdateContent(CREATE_VALUE);
    textLayoutProperty->UpdateHeightAdaptivePolicy(TextHeightAdaptivePolicy::LAYOUT_CONSTRAINT_FIRST);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.selfIdealSize.SetSize(TEXT_SIZE);
    parentLayoutConstraint.maxSize = CONTAINER_LOW_SIZE;
    textLayoutProperty->UpdateAdaptMinFontSize(ADAPT_OVER_MIN_FONT_SIZE_VALUE);
    textLayoutProperty->UpdateLineHeight(ADAPT_LINE_HEIGHT_VALUE);
    textLayoutProperty->UpdateMaxLines(MAX_LINES);

    /**
     * @tc.steps: step3. create textLayoutAlgorithm and call MeasureContent function.
     * @tc.expected: The width of the return value of MeasureContent is equal to maxWidth of paragraph_
     */
    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    auto contentSize =
        textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(contentSize.value().Width(), textLayoutAlgorithm->paragraphManager_->GetMaxWidth());
}

/**
 * @tc.name: TextPaintMethodTest001
 * @tc.desc: test text_paint_method.cpp :set textOverflow to MARQUEE
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPaintMethodTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set theme.
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    auto theme = AceType::MakeRefPtr<MockThemeManager>();
    pipeline->SetThemeManager(theme);
    EXPECT_CALL(*theme, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextTheme>()));

    /**
     * @tc.steps: step3. create textPaintMethod and call UpdateContentModifier function.
     * @tc.expected: The return value of GetOverlayModifier is not null.
     */
    auto pattern = textFrameNode->GetPattern<Pattern>();
    AceType::DynamicCast<TextPattern>(pattern)->textSelector_.Update(0, -1);
    ParagraphStyle paragraphStyle;
    RefPtr<Paragraph> paragraph = Paragraph::Create(paragraphStyle, FontCollection::Current());
    RefPtr<TextContentModifier> textContentModifier =
        AceType::MakeRefPtr<TextContentModifier>(std::optional<TextStyle>(TextStyle()));
    RefPtr<TextOverlayModifier> textOverlayModifier = AceType::MakeRefPtr<TextOverlayModifier>();
    TextPaintMethod textPaintMethod(pattern, BASE_LINE_OFFSET_VALUE, textContentModifier, textOverlayModifier);
    UpdateTextLayoutProperty(textLayoutProperty);
    RefPtr<RenderContext> renderContext = RenderContext::Create();
    auto paintProperty = textPattern->CreatePaintProperty();
    auto paintWrapper = AceType::MakeRefPtr<PaintWrapper>(renderContext, geometryNode, paintProperty);
    textPaintMethod.UpdateContentModifier(AceType::RawPtr(paintWrapper));
    SetPaintMethodModifier(textPaintMethod);
    textPaintMethod.UpdateContentModifier(AceType::RawPtr(paintWrapper));
    textPaintMethod.textContentModifier_->textDecoration_ = TextDecoration::UNDERLINE;
    textPaintMethod.UpdateContentModifier(AceType::RawPtr(paintWrapper));
    ASSERT_NE(textPaintMethod.GetOverlayModifier(AceType::RawPtr(paintWrapper)), nullptr);
}

/**
 * @tc.name: TextContentModifier001
 * @tc.desc: test text_content_modifier.cpp onDraw function.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextContentModifier001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. call onDraw function.
     * @tc.expected: The member variable value of textContentModifier is the value set above
     */
    auto frameNode = layoutWrapper->GetHostNode();
    auto pipeline = frameNode->GetContextRefPtr();
    TextStyle textStyle = CreateTextStyleUsingTheme(
        textLayoutProperty->GetFontStyle(), textLayoutProperty->GetTextLineStyle(), pipeline->GetTheme<TextTheme>());
    TextContentModifier textContentModifier(std::optional<TextStyle>(std::move(textStyle)));
    textStyle.SetTextDecorationColor(TEXT_COLOR_VALUE);
    SetContentModifier(textContentModifier);
    auto pattern = textFrameNode->GetPattern<Pattern>();
    ParagraphStyle paragraphStyle;
    RefPtr<Paragraph> paragraph = Paragraph::Create(paragraphStyle, FontCollection::Current());
    RefPtr<TextContentModifier> contentModifier =
        AceType::MakeRefPtr<TextContentModifier>(std::optional<TextStyle>(TextStyle()));
    RefPtr<TextOverlayModifier> textOverlayModifier = AceType::MakeRefPtr<TextOverlayModifier>();
    TextPaintMethod textPaintMethod(pattern, BASE_LINE_OFFSET_VALUE, contentModifier, textOverlayModifier);
    // set pipelineContext nullptr
    MockPipelineContext::TearDown();
    textContentModifier.SetFontSize(ADAPT_FONT_SIZE_VALUE, textStyle);
    textContentModifier.SetBaselineOffset(BASELINE_OFFSET_VALUE);
    MockPipelineContext::SetUp();
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, ClipRect(_, _, _)).WillRepeatedly(Return());
    DrawingContext context { canvas, CONTEXT_WIDTH_VALUE, CONTEXT_HEIGHT_VALUE };
    textPattern->pManager_->AddParagraph({ .paragraph = paragraph, .start = 0, .end = 100 });
    // call onDraw function(textRacing_ = true)
    MarqueeOption option;
    textContentModifier.StartTextRace(option);
    context.width = CONTEXT_LARGE_WIDTH_VALUE;
    textContentModifier.onDraw(context);
    // call onDraw function(textRacing_ = false)
    textContentModifier.StopTextRace();
    textContentModifier.onDraw(context);
    EXPECT_EQ(textContentModifier.fontSizeFloat_->Get(), ADAPT_FONT_SIZE_VALUE.Value());
    EXPECT_EQ(textContentModifier.baselineOffsetFloat_->Get(), BASELINE_OFFSET_VALUE.Value());
    textPattern->pManager_->Reset();
}

/**
 * @tc.name: TextContentModifier002
 * @tc.desc: test text_content_modifier.cpp ModifyTextStyle function.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextContentModifier002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. call ModifyTextStyle function.
     * @tc.expected: The member variable value of textContentModifier is the value set above
     */
    auto frameNode = layoutWrapper->GetHostNode();
    auto pipeline = frameNode->GetContextRefPtr();
    TextStyle textStyle = CreateTextStyleUsingTheme(
        textLayoutProperty->GetFontStyle(), textLayoutProperty->GetTextLineStyle(), pipeline->GetTheme<TextTheme>());
    TextContentModifier textContentModifier(std::optional<TextStyle>(std::move(textStyle)));
    textStyle.SetTextDecorationColor(TEXT_COLOR_VALUE);
    SetContentModifier(textContentModifier);
    auto pattern = textFrameNode->GetPattern<Pattern>();
    ParagraphStyle paragraphStyle;
    RefPtr<Paragraph> paragraph = Paragraph::Create(paragraphStyle, FontCollection::Current());
    RefPtr<TextContentModifier> contentModifier =
        AceType::MakeRefPtr<TextContentModifier>(std::optional<TextStyle>(TextStyle()));
    RefPtr<TextOverlayModifier> textOverlayModifier = AceType::MakeRefPtr<TextOverlayModifier>();
    TextPaintMethod textPaintMethod(pattern, BASE_LINE_OFFSET_VALUE, contentModifier, textOverlayModifier);
    // set pipelineContext nullptr
    MockPipelineContext::TearDown();
    textContentModifier.SetFontSize(ADAPT_FONT_SIZE_VALUE, textStyle);
    textContentModifier.SetBaselineOffset(BASELINE_OFFSET_VALUE);
    MockPipelineContext::SetUp();
    // set textDecorationAnimatable_ true
    textContentModifier.textDecorationAnimatable_ = true;
    textContentModifier.textDecoration_ = TextDecoration::UNDERLINE;
    textContentModifier.SetTextDecoration(TextDecoration::NONE);
    textContentModifier.ModifyTextStyle(textStyle);
    // set textDecorationColorAlpha_ value
    textContentModifier.textDecorationColorAlpha_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(1000.0f);
    textContentModifier.ModifyTextStyle(textStyle);
    // set textDecorationAnimatable_ false
    textContentModifier.SetTextDecoration(TextDecoration::LINE_THROUGH);
    textContentModifier.ModifyTextStyle(textStyle);
    EXPECT_EQ(textContentModifier.fontSizeFloat_->Get(), ADAPT_FONT_SIZE_VALUE.Value());
    EXPECT_EQ(textContentModifier.baselineOffsetFloat_->Get(), BASELINE_OFFSET_VALUE.Value());
    EXPECT_EQ(textStyle.GetFontSize().Value(), textContentModifier.fontSizeFloat_->Get());
}

/**
 * @tc.name: TextLayoutAlgorithmTest006
 * @tc.desc: text_layout_algorithm.cpp:Set TextOverflow to MARQUEE
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutAlgorithmTest006, TestSize.Level1)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, GetLongestLine).WillOnce(Return(100.0f));
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    textPattern->pManager_->AddParagraph({ .paragraph = paragraph, .start = 0, .end = 100 });

    /**
     * @tc.steps: step2. set textLayoutProperty.
     */
    textLayoutProperty->UpdateContent(CREATE_VALUE);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.selfIdealSize.SetSize(TEXT_SIZE);
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    textLayoutProperty->UpdateTextOverflow(TextOverflow::MARQUEE);

    /**
     * @tc.steps: step3. create textLayoutAlgorithm and call MeasureContent function.
     * @tc.expected: The width of the return value of MeasureContent is equal to 100.0f
     */
    auto textLayoutAlgorithm = textPattern->CreateLayoutAlgorithm();
    auto contentSize =
        textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(contentSize.value().Width(), 100.0f);
    textPattern->pManager_->Reset();
}

/**
 * @tc.name: TextLayoutAlgorithmTest007
 * @tc.desc: text_layout_algorithm.cpp:set TextHeightAdaptivePolicy to LAYOUT_CONSTRAINT_FIRST and set minFontSize to
 *           zero. ADAPT_ZERO_FONT_SIZE_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutAlgorithmTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. set textLayoutProperty.
     */
    textLayoutProperty->UpdateContent(CREATE_VALUE);
    textLayoutProperty->UpdateHeightAdaptivePolicy(TextHeightAdaptivePolicy::LAYOUT_CONSTRAINT_FIRST);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.selfIdealSize.SetSize(TEXT_SIZE);
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    textLayoutProperty->UpdateAdaptMinFontSize(ADAPT_ZERO_FONT_SIZE_VALUE);
    textLayoutProperty->UpdateLineHeight(ADAPT_LINE_HEIGHT_VALUE);
    textLayoutProperty->UpdateMaxLines(MAX_LINES);

    /**
     * @tc.steps: step3. create textLayoutAlgorithm.
     * @tc.expected: The width of the return value of MeasureContent is equal to maxWidth of paragraph_
     */
    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    auto contentSize =
        textLayoutAlgorithm->MeasureContent(parentLayoutConstraint, AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    textLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(contentSize.value().Width(), textLayoutAlgorithm->paragraphManager_->GetMaxWidth());
}

/**
 * @tc.name: TextLayoutAlgorithmTest008
 * @tc.desc: text_layout_algorithm.cpp:call AdaptMaxTextSize
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextLayoutAlgorithmTest008, TestSize.Level1)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, PushStyle).Times(2);
    EXPECT_CALL(*paragraph, AddText).Times(2);
    EXPECT_CALL(*paragraph, Layout).Times(2);
    EXPECT_CALL(*paragraph, Build).Times(2);
    EXPECT_CALL(*paragraph, GetLineCount).WillRepeatedly(Return(2));
    EXPECT_CALL(*paragraph, GetLongestLine).WillRepeatedly(Return(100));

    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textFrameNode, geometryNode, textFrameNode->GetLayoutProperty());
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textLayoutProperty = textPattern->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    auto frameNode = layoutWrapper->GetHostNode();
    auto pipeline = frameNode->GetContextRefPtr();
    TextStyle textStyle = CreateTextStyleUsingTheme(
        textLayoutProperty->GetFontStyle(), textLayoutProperty->GetTextLineStyle(), pipeline->GetTheme<TextTheme>());
    textStyle.SetAdaptFontSizeStep(ADAPT_FONT_SIZE_STEP_VALUE);

    /**
     * @tc.steps: step2. create textLayoutAlgorithm and call AdaptMaxTextSize.
     * @tc.expected: when maxFontSize < minFontSize,The return value of AdaptMaxTextSize is false.
     *               when create paragraph failed,The return value of AdaptMaxTextSize is false.
     *               when increase font size,The return value of AdaptMaxTextSize is true.
     *               when set NormalizeToPx false,The return value of AdaptMaxTextSize is false.
     */
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.selfIdealSize.SetSize(TEXT_SIZE);
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    auto textLayoutAlgorithm = AceType::MakeRefPtr<TextLayoutAlgorithm>();

    // maxFontSize < minFontSize
    textStyle.SetAdaptMaxFontSize(ADAPT_MIN_FONT_SIZE_VALUE);
    textStyle.SetAdaptMinFontSize(ADAPT_MAX_FONT_SIZE_VALUE);
    EXPECT_EQ(
        textLayoutAlgorithm->AdaptMaxTextSize(textStyle, "abc", parentLayoutConstraint, AceType::RawPtr(textFrameNode)),
        true);

    // create paragraph failed
    MockParagraph::enabled_ = false;
    textStyle.SetAdaptMaxFontSize(ADAPT_MAX_FONT_SIZE_VALUE);
    textStyle.SetAdaptMinFontSize(ADAPT_MIN_FONT_SIZE_VALUE);
    EXPECT_EQ(
        textLayoutAlgorithm->AdaptMaxTextSize(textStyle, "abc", parentLayoutConstraint, AceType::RawPtr(textFrameNode)),
        false);
    MockParagraph::enabled_ = true;

    // increase font size
    EXPECT_EQ(
        textLayoutAlgorithm->AdaptMaxTextSize(textStyle, "abc", parentLayoutConstraint, AceType::RawPtr(textFrameNode)),
        true);

    // set NormalizeToPx false
    textStyle.adaptFontSizeStep_.SetUnit(DimensionUnit::CALC);
    EXPECT_EQ(
        textLayoutAlgorithm->AdaptMaxTextSize(textStyle, "abc", parentLayoutConstraint, AceType::RawPtr(textFrameNode)),
        false);
}

/**
 * @tc.name: TextSelectOverlayTestPreProcessOverlay001
 * @tc.desc: Verify PreProcessOverlay
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestPreProcessOverlay001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    OverlayRequest request;

    textSelectOverlay->hostTextBase_ = nullptr;
    ASSERT_EQ(textSelectOverlay->PreProcessOverlay(request), false);
    textSelectOverlay->hostTextBase_ = pattern;

    ASSERT_EQ(textSelectOverlay->PreProcessOverlay(request), false);
}

/**
 * @tc.name: TextSelectOverlayTestGetFirstHandleInfo001
 * @tc.desc: Verify GetFirstHandleInfo
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestGetFirstHandleInfo001, TestSize.Level1)
{
    WeakPtr<TextBase> textBase = nullptr;
    TextSelectOverlay textSelectOverlay(textBase);

    ASSERT_EQ(textSelectOverlay.GetFirstHandleInfo(), std::nullopt);
}

/**
 * @tc.name: TextSelectOverlayTestGetSecondHandleInfo001
 * @tc.desc: Verify GetSecondHandleInfo
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestGetSecondHandleInfo001, TestSize.Level1)
{
    WeakPtr<TextBase> textBase = nullptr;
    TextSelectOverlay textSelectOverlay(textBase);

    ASSERT_EQ(textSelectOverlay.GetSecondHandleInfo(), std::nullopt);
}

/**
 * @tc.name: TextSelectOverlayTestCheckAndAdjustHandle001
 * @tc.desc: Verify CheckAndAdjustHandle
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestCheckAndAdjustHandle001, TestSize.Level1)
{
    /**
     * @tc.steps: set theme, used in CheckAndAdjustHandle.
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    auto theme = AceType::MakeRefPtr<MockThemeManager>();
    pipeline->SetThemeManager(theme);
    EXPECT_CALL(*theme, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextOverlayTheme>()));

    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    RectF paintRect(RECT_X_VALUE, RECT_Y_VALUE, RECT_WIDTH_VALUE, RECT_HEIGHT_VALUE);

    textSelectOverlay->hostTextBase_ = nullptr;
    ASSERT_EQ(textSelectOverlay->CheckAndAdjustHandle(paintRect), false);
    textSelectOverlay->hostTextBase_ = pattern;

    ASSERT_EQ(textSelectOverlay->CheckAndAdjustHandle(paintRect), false);

    auto frameNode = FrameNode::CreateFrameNode("Test", DEFAULT_NODE_ID, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);

    auto renderContext = frameNode->renderContext_;
    frameNode->renderContext_ = nullptr;
    ASSERT_EQ(textSelectOverlay->CheckAndAdjustHandle(paintRect), false);
    frameNode->renderContext_ = renderContext;
}

/**
 * @tc.name: TextSelectOverlayTestCheckAndAdjustHandle002
 * @tc.desc: Verify CheckAndAdjustHandle
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestCheckAndAdjustHandle002, TestSize.Level1)
{
    /**
     * @tc.steps: set theme, used in CheckAndAdjustHandle.
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    auto theme = AceType::MakeRefPtr<MockThemeManager>();
    pipeline->SetThemeManager(theme);
    EXPECT_CALL(*theme, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextOverlayTheme>()));

    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", DEFAULT_NODE_ID, pattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->geometryNode_ = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(frameNode->geometryNode_, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    RectF paintRect;
    pattern->contentRect_.SetRect(RECT_X_VALUE, RECT_Y_VALUE, RECT_WIDTH_VALUE, RECT_HEIGHT_VALUE);

    paintRect.SetRect(RECT_X_VALUE, RECT_Y_VALUE - RECT_Y_VALUE, RECT_WIDTH_VALUE, RECT_HEIGHT_VALUE + RECT_Y_VALUE);
    ASSERT_EQ(textSelectOverlay->CheckAndAdjustHandle(paintRect), true);

    paintRect.SetRect(RECT_X_VALUE, RECT_Y_VALUE + RECT_Y_VALUE, RECT_WIDTH_VALUE, RECT_HEIGHT_VALUE - RECT_Y_VALUE);
    ASSERT_EQ(textSelectOverlay->CheckAndAdjustHandle(paintRect), true);
}

/**
 * @tc.name: TextSelectOverlayTestCheckHandleVisible001
 * @tc.desc: Verify CheckHandleVisible
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestCheckHandleVisible001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", DEFAULT_NODE_ID, pattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->geometryNode_ = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(frameNode->geometryNode_, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    RectF paintRect(RECT_X_VALUE, RECT_Y_VALUE, RECT_WIDTH_VALUE, RECT_HEIGHT_VALUE);
    ASSERT_EQ(textSelectOverlay->CheckHandleVisible(paintRect), false);
}

/**
 * @tc.name: TextSelectOverlayTestCheckHandleVisible002
 * @tc.desc: Verify CheckHandleVisible
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestCheckHandleVisible002, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    RectF paintRect(RECT_X_VALUE, RECT_Y_VALUE, RECT_WIDTH_VALUE, RECT_HEIGHT_VALUE);

    textSelectOverlay->hostTextBase_ = nullptr;
    ASSERT_EQ(textSelectOverlay->CheckHandleVisible(paintRect), false);
    textSelectOverlay->hostTextBase_ = pattern;

    ASSERT_EQ(textSelectOverlay->CheckHandleVisible(paintRect), false);

    auto frameNode = FrameNode::CreateFrameNode("Test", DEFAULT_NODE_ID, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);

    auto renderContext = frameNode->renderContext_;
    frameNode->renderContext_ = nullptr;
    ASSERT_EQ(textSelectOverlay->CheckHandleVisible(paintRect), false);
    frameNode->renderContext_ = renderContext;
}

/**
 * @tc.name: TextSelectOverlayTestOnResetTextSelection001
 * @tc.desc: Verify OnResetTextSelection
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestOnResetTextSelection001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);
    auto textPattern = textSelectOverlay->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);

    OffsetF firstOffset(ADAPT_OFFSETY_VALUE, ADAPT_OFFSETX_VALUE);
    OffsetF secondOffset(ADAPT_OFFSETY_VALUE + ADAPT_OFFSETY_VALUE, ADAPT_OFFSETX_VALUE + ADAPT_OFFSETX_VALUE);

    textSelectOverlay->UpdateSelectorOnHandleMove(firstOffset, true);
    textSelectOverlay->UpdateSelectorOnHandleMove(secondOffset, false);
    ASSERT_NE(textPattern->textSelector_.GetStart(), TEXT_ERROR);
    textSelectOverlay->OnResetTextSelection();
    ASSERT_EQ(textPattern->textSelector_.GetStart(), TEXT_ERROR);
}

/**
 * @tc.name: TextSelectOverlayTestOnHandleMove001
 * @tc.desc: Verify OnHandleMove
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestOnHandleMove001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", DEFAULT_NODE_ID, pattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->geometryNode_ = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(frameNode->geometryNode_, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    RectF handleRect(RECT_X_VALUE, RECT_Y_VALUE, RECT_WIDTH_VALUE, RECT_HEIGHT_VALUE);

    ASSERT_EQ(pattern->textSelector_.GetStart(), TEXT_ERROR);
    ASSERT_EQ(pattern->textSelector_.GetEnd(), TEXT_ERROR);
    textSelectOverlay->OnHandleMove(handleRect, true);
    ASSERT_EQ(pattern->textSelector_.GetStart(), 0);
    ASSERT_EQ(pattern->textSelector_.GetEnd(), TEXT_ERROR);
    textSelectOverlay->OnHandleMove(handleRect, false);
    ASSERT_EQ(pattern->textSelector_.GetStart(), 0);
    ASSERT_EQ(pattern->textSelector_.GetEnd(), 0);
}

/**
 * @tc.name: TextSelectOverlayTestUpdateSelectorOnHandleMove001
 * @tc.desc: Verify UpdateSelectorOnHandleMove
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestUpdateSelectorOnHandleMove001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    OffsetF handleOffset(ADAPT_OFFSETY_VALUE, ADAPT_OFFSETX_VALUE);

    ASSERT_EQ(pattern->textSelector_.GetStart(), TEXT_ERROR);
    ASSERT_EQ(pattern->textSelector_.GetEnd(), TEXT_ERROR);
    textSelectOverlay->UpdateSelectorOnHandleMove(handleOffset, true);
    textSelectOverlay->UpdateSelectorOnHandleMove(handleOffset, false);
    ASSERT_EQ(pattern->textSelector_.GetStart(), 0);
    ASSERT_EQ(pattern->textSelector_.GetEnd(), 0);
}

/**
 * @tc.name: TextSelectOverlayTestGetSelectedText001
 * @tc.desc: Verify GetSelectedText
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestGetSelectedText001, TestSize.Level1)
{
    WeakPtr<TextBase> textBase = nullptr;
    TextSelectOverlay textSelectOverlay(textBase);

    ASSERT_EQ(textSelectOverlay.GetSelectedText(), "");
}

/**
 * @tc.name: TextSelectOverlayTestGetSelectArea001
 * @tc.desc: Verify GetSelectArea
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestGetSelectArea001, TestSize.Level1)
{
    /**
     * @tc.steps: set theme, used in CheckAndAdjustHandle.
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    auto theme = AceType::MakeRefPtr<MockThemeManager>();
    pipeline->SetThemeManager(theme);
    EXPECT_CALL(*theme, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextOverlayTheme>()));

    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", DEFAULT_NODE_ID, pattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->geometryNode_ = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(frameNode->geometryNode_, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    bool hasTransform = textSelectOverlay->hasTransform_;
    textSelectOverlay->hasTransform_ = true;
    ASSERT_EQ(textSelectOverlay->GetSelectArea(), RectF());
    textSelectOverlay->hasTransform_ = hasTransform;
}

/**
 * @tc.name: TextSelectOverlayTestGetSelectArea002
 * @tc.desc: Verify GetSelectArea
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestGetSelectArea002, TestSize.Level1)
{
    WeakPtr<TextBase> textBase = nullptr;
    TextSelectOverlay textSelectOverlay(textBase);

    ASSERT_EQ(textSelectOverlay.GetSelectArea(), RectF());
}

/**
 * @tc.name: TextSelectOverlayTestGetSelectAreaFromHandle001
 * @tc.desc: Verify GetSelectAreaFromHandle
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestGetSelectAreaFromHandle001, TestSize.Level1)
{
    /**
     * @tc.steps: set theme, used in CheckAndAdjustHandle.
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    auto theme = AceType::MakeRefPtr<MockThemeManager>();
    pipeline->SetThemeManager(theme);
    EXPECT_CALL(*theme, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextOverlayTheme>()));

    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", DEFAULT_NODE_ID, pattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->geometryNode_ = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(frameNode->geometryNode_, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    RectF rect;
    textSelectOverlay->hasTransform_ = !textSelectOverlay->hasTransform_;
    textSelectOverlay->GetSelectAreaFromHandle(rect);
    textSelectOverlay->hasTransform_ = !textSelectOverlay->hasTransform_;
    ASSERT_EQ(rect.GetX(), 0.0);
    ASSERT_EQ(rect.GetY(), 0.0);
}

/**
 * @tc.name: TextSelectOverlayTestGetSelectAreaFromHandle002
 * @tc.desc: Verify GetSelectAreaFromHandle
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestGetSelectAreaFromHandle002, TestSize.Level1)
{
    WeakPtr<TextBase> textBase = nullptr;
    TextSelectOverlay textSelectOverlay(textBase);

    RectF rect(RECT_X_VALUE, RECT_Y_VALUE, RECT_WIDTH_VALUE, RECT_HEIGHT_VALUE);

    textSelectOverlay.OnHandleMove(rect, false);
    textSelectOverlay.GetSelectAreaFromHandle(rect);
    ASSERT_EQ(rect.GetX(), RECT_X_VALUE);
    ASSERT_EQ(rect.GetY(), RECT_Y_VALUE);
}

/**
 * @tc.name: TextSelectOverlayTestOnUpdateMenuInfo001
 * @tc.desc: Verify OnUpdateMenuInfo
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestOnUpdateMenuInfo001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    SelectMenuInfo menuInfo;

    textSelectOverlay->OnUpdateMenuInfo(menuInfo, DIRTY_COPY_ALL_ITEM);
    ASSERT_EQ(menuInfo.showCut, true);

    textSelectOverlay->OnUpdateMenuInfo(menuInfo, DIRTY_FIRST_HANDLE);
    ASSERT_EQ(menuInfo.showCut, false);
}

/**
 * @tc.name: TextSelectOverlayTestOnMenuItemAction001
 * @tc.desc: Verify OnMenuItemAction
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestOnMenuItemAction001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    pattern->textForDisplay_ = TEXT_CONTENT;
    textSelectOverlay->OnMenuItemAction(OptionMenuActionId::CUT, OptionMenuType::NO_MENU);
    textSelectOverlay->OnMenuItemAction(OptionMenuActionId::COPY, OptionMenuType::NO_MENU);
    textSelectOverlay->OnMenuItemAction(OptionMenuActionId::SELECT_ALL, OptionMenuType::NO_MENU);
    ASSERT_EQ(pattern->textSelector_.GetStart(), 0);
    ASSERT_EQ(pattern->textSelector_.GetEnd(), TEXT_CONTENT.size());
}

/**
 * @tc.name: TextSelectOverlayTestOnCloseOverlay001
 * @tc.desc: Verify OnCloseOverlay
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestOnCloseOverlay001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    pattern->textForDisplay_ = TEXT_CONTENT;
    textSelectOverlay->OnMenuItemAction(OptionMenuActionId::SELECT_ALL, OptionMenuType::NO_MENU);
    ASSERT_EQ(pattern->textSelector_.GetStart(), 0);
    ASSERT_EQ(pattern->textSelector_.GetEnd(), TEXT_CONTENT.size());

    textSelectOverlay->OnCloseOverlay(OptionMenuType::NO_MENU, CloseReason::CLOSE_REASON_TOOL_BAR);
    textSelectOverlay->OnCloseOverlay(OptionMenuType::NO_MENU, CloseReason::CLOSE_REASON_HOLD_BY_OTHER);

    ASSERT_EQ(pattern->textSelector_.GetStart(), -1);
    ASSERT_EQ(pattern->textSelector_.GetEnd(), -1);
}

/**
 * @tc.name: TextSelectOverlayTestOnHandleGlobalTouchEvent001
 * @tc.desc: Verify OnHandleGlobalTouchEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextSelectOverlayTestOnHandleGlobalTouchEvent001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    pattern->textForDisplay_ = TEXT_CONTENT;
    textSelectOverlay->OnMenuItemAction(OptionMenuActionId::SELECT_ALL, OptionMenuType::NO_MENU);
    ASSERT_EQ(pattern->textSelector_.GetStart(), 0);
    ASSERT_EQ(pattern->textSelector_.GetEnd(), TEXT_CONTENT.size());

    textSelectOverlay->OnHandleGlobalTouchEvent(SourceType::TOUCH, TouchType::DOWN);
    textSelectOverlay->OnHandleGlobalTouchEvent(SourceType::TOUCH, TouchType::UP);
    textSelectOverlay->OnHandleGlobalTouchEvent(SourceType::MOUSE, TouchType::DOWN);
    textSelectOverlay->OnHandleGlobalTouchEvent(SourceType::MOUSE, TouchType::UP);

    ASSERT_EQ(pattern->textSelector_.GetStart(), -1);
    ASSERT_EQ(pattern->textSelector_.GetEnd(), -1);
}

/**
 * @tc.name: TextStylesTestGetFontFamilyInJson001
 * @tc.desc: Verify GetFontFamilyInJson
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextStylesTestGetFontFamilyInJson001, TestSize.Level1)
{
    std::optional<std::vector<std::string>> value;

    value = std::vector<std::string>{ "Arial", "Calibri" };

    ASSERT_EQ(GetFontFamilyInJson(value), "Arial,Calibri");
}

/**
 * @tc.name: TextStylesTestGetSymbolRenderingStrategyInJson001
 * @tc.desc: Verify GetSymbolRenderingStrategyInJson
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextStylesTestGetSymbolRenderingStrategyInJson001, TestSize.Level1)
{
    std::optional<uint32_t> value = RENDERINGSTRATEGY_MULTIPLE_COLOR;
    ASSERT_EQ(GetSymbolRenderingStrategyInJson(value), "SymbolRenderingStrategy.MULTIPLE_COLOR");

    value = RENDERINGSTRATEGY_MULTIPLE_OPACITY;
    ASSERT_EQ(GetSymbolRenderingStrategyInJson(value), "SymbolRenderingStrategy.MULTIPLE_OPACITY");

    value = UKNOWN_VALUE;
    ASSERT_EQ(GetSymbolRenderingStrategyInJson(value), "SymbolRenderingStrategy.SINGLE");
}

/**
 * @tc.name: TextStylesTestGetSymbolEffectStrategyInJson001
 * @tc.desc: Verify GetSymbolEffectStrategyInJson
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextStylesTestGetSymbolEffectStrategyInJson001, TestSize.Level1)
{
    std::optional<uint32_t> value = static_cast<uint32_t>(SymbolEffectType::SCALE);
    ASSERT_EQ(GetSymbolEffectStrategyInJson(value), "SymbolEffectStrategy.SCALE");

    value = static_cast<uint32_t>(SymbolEffectType::HIERARCHICAL);
    ASSERT_EQ(GetSymbolEffectStrategyInJson(value), "SymbolEffectStrategy.HIERARCHICAL");

    value = UKNOWN_VALUE;
    ASSERT_EQ(GetSymbolEffectStrategyInJson(value), "SymbolEffectStrategy.NONE");
}

/**
 * @tc.name: UpdateSelectOverlayOrCreate001
 * @tc.desc: Test TextPattern UpdateSelectOverlayOrCreate.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, UpdateSelectOverlayOrCreate001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    /**
     * @tc.steps: step2. Construct data and call UpdateSelectOverlayOrCreate
     */
    SelectOverlayInfo selectOverlayInfo;
    selectOverlayInfo.singleLineHeight = NODE_ID;
    textPattern->UpdateSelectOverlayOrCreate(selectOverlayInfo, true);
    EXPECT_EQ(selectOverlayInfo.hitTestMode, HitTestMode::HTMDEFAULT);

    auto root = AceType::MakeRefPtr<FrameNode>(ROOT_TAG, -1, AceType::MakeRefPtr<Pattern>(), true);
    auto selectOverlayManager = AceType::MakeRefPtr<SelectOverlayManager>(root);
    auto proxy = selectOverlayManager->CreateAndShowSelectOverlay(selectOverlayInfo, nullptr, false);
    textPattern->selectOverlayProxy_ = proxy;
    textPattern->UpdateSelectOverlayOrCreate(selectOverlayInfo, true);
    EXPECT_TRUE(textPattern->selectOverlayProxy_ && !textPattern->selectOverlayProxy_->IsClosed());
}

/**
 * @tc.name: HandleOnSelectAll
 * @tc.desc: Test TextPattern HandleOnSelectAll
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, OnModifyDone003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textPattern.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    /**
     * @tc.steps: step2. Construct data and call HandleOnSelectAll
     */
    textPattern->sourceType_ = SourceType::MOUSE;
    textPattern->HandleOnSelectAll();
    EXPECT_TRUE(textPattern->IsUsingMouse());
}

/**
 * @tc.name: CreateImageSourceInfo001
 * @tc.desc: Test TextPattern HandleOnSelectAll
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, CreateImageSourceInfo001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textPattern.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    /**
     * @tc.steps: step2. Construct data and call CreatImageSourceInfo
     */
    ImageSpanOptions textOptions;
    textPattern->CreateImageSourceInfo(textOptions);
    textOptions.image = "textImage";
    textOptions.bundleName = "textBundleName";
    textOptions.moduleName = "textModuleName";
    textPattern->CreateImageSourceInfo(textOptions);
    EXPECT_TRUE(textOptions.image.has_value());
    EXPECT_TRUE(textOptions.bundleName.has_value());
    EXPECT_TRUE(textOptions.moduleName.has_value());
}

/**
 * @tc.name: create001
 * @tc.desc: Test create with spanstring.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, create001, TestSize.Level1)
{
    TextModelNG textModelNG;
    textModelNG.Create(CREATE_VALUE);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
     /**
     * @tc.steps: step1. Create spanBases
     */
    auto spanStringWithSpans = AceType::MakeRefPtr<SpanString>("01234567891");
    /**
     * @tc.steps: step2. call spanBases
     */
    textModelNG.Create(spanStringWithSpans);
    auto textPattern = frameNode->GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    EXPECT_NE(textPattern->GetSpanStringMode(), true);
}

/**
 * @tc.name: SetTextSelectableMode001
 * @tc.desc: Test SetTextSelectableMode by frameNode..
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, SetTextSelectableMode001, TestSize.Level1)
{
    TextModelNG textModelNG;
    textModelNG.Create(CREATE_VALUE);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextLayoutProperty> textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutProperty);
    ASSERT_NE(textLayoutProperty, nullptr);
    EXPECT_EQ(textLayoutProperty->GetContentValue(), CREATE_VALUE);

    auto retFont = textModelNG.GetFont(frameNode);
    textModelNG.SetFontSize(ADAPT_ZERO_FONT_SIZE_VALUE);
    EXPECT_EQ(textModelNG.GetFontSize(frameNode), ADAPT_ZERO_FONT_SIZE_VALUE);

    textModelNG.SetTextSelectableMode(frameNode, TextSelectableMode::SELECTABLE_UNFOCUSABLE);
    textModelNG.SetTextSelectableMode(TextSelectableMode::SELECTABLE_UNFOCUSABLE);
    ASSERT_EQ(textModelNG.GetTextSelectableMode(frameNode), TextSelectableMode::SELECTABLE_UNFOCUSABLE);

    textModelNG.SetEllipsisMode(frameNode, EllipsisMode::HEAD);
    textModelNG.SetEllipsisMode(EllipsisMode::HEAD);
    ASSERT_EQ(textModelNG.GetEllipsisMode(frameNode), EllipsisMode::HEAD);
}

/**
 * @tc.name: InitSpanStringController001
 * @tc.desc: Test InitSpanStringController.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, InitSpanStringController001, TestSize.Level1)
{
    TextModelNG textModelNG;
    textModelNG.Create(CREATE_VALUE);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextLayoutProperty> textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutProperty);
    ASSERT_NE(textLayoutProperty, nullptr);
    EXPECT_EQ(textLayoutProperty->GetContentValue(), CREATE_VALUE);

    auto spanStringWithSpans = AceType::MakeRefPtr<SpanString>("01234567893421");

    textModelNG.InitSpanStringController(frameNode, spanStringWithSpans);
    auto textPattern = frameNode->GetPattern<TextPattern>();
    CHECK_NULL_VOID(textPattern);
    EXPECT_EQ(textPattern->GetSpanStringMode(), true);
}

/**
 * @tc.name: InitTextController001
 * @tc.desc: Test InitTextController.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, InitTextController001, TestSize.Level1)
{
    TextModelNG textModelNG;
    textModelNG.Create(CREATE_VALUE);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextLayoutProperty> textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutProperty);
    ASSERT_NE(textLayoutProperty, nullptr);
    EXPECT_EQ(textLayoutProperty->GetContentValue(), CREATE_VALUE);

    auto ret = textModelNG.InitTextController(frameNode);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: SetTextDetectEnable001
 * @tc.desc: Test SetTextDetectEnable.
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, SetTextDetectEnable001, TestSize.Level1)
{
    TextModelNG textModelNG;
    textModelNG.Create(CREATE_VALUE);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextLayoutProperty> textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutProperty);
    ASSERT_NE(textLayoutProperty, nullptr);
    EXPECT_EQ(textLayoutProperty->GetContentValue(), CREATE_VALUE);

    textModelNG.SetTextDetectEnable(frameNode, true);
    ASSERT_NE(textModelNG.GetTextDetectEnable(frameNode), false);

    auto ret = textModelNG.GetTextController();
    EXPECT_NE(ret, nullptr);

    textModelNG.SetClipEdge(true);
    EXPECT_NE(frameNode->renderContext_, nullptr);

    auto strContent = textModelNG.GetContent(frameNode);
    EXPECT_EQ(strContent, CREATE_VALUE);

    auto retLineHeight = textModelNG.GetLineHeight(frameNode);
    EXPECT_EQ(retLineHeight, 0.0f);

    auto retLineSpacing = textModelNG.GetLineSpacing(frameNode);
    EXPECT_EQ(retLineSpacing, 0.0f);

    auto retDecoration = textModelNG.GetDecoration(frameNode);
    EXPECT_EQ(retDecoration, TextDecoration::NONE);

    auto retTextDecorationColor = textModelNG.GetTextDecorationColor(frameNode);
    EXPECT_EQ(retTextDecorationColor, Color::BLACK);

    auto retTextDecorationStyle = textModelNG.GetTextDecorationStyle(frameNode);
    EXPECT_EQ(retTextDecorationStyle, TextDecorationStyle::SOLID);

    auto retTextCase = textModelNG.GetTextCase(frameNode);
    EXPECT_EQ(retTextCase, TextCase::NORMAL);

    auto retLetterSpacing = textModelNG.GetLetterSpacing(frameNode);
    EXPECT_EQ(retLetterSpacing, ADAPT_ZERO_FONT_SIZE_VALUE);

    auto retMaxLines = textModelNG.GetMaxLines(frameNode);
    EXPECT_NE(retMaxLines, 0.0f);

    auto retTextAlign = textModelNG.GetTextAlign(frameNode);
    EXPECT_EQ(retTextAlign, TextAlign::START);
}

/**
 * @tc.name: TextPattern001
 * @tc.desc: Test TextPattern GetGlyphPositionAtCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern GetGlyphPositionAtCoordinate
     */
    auto [frameNode, pattern] = Init();
    auto positionWithAffinity = pattern->GetGlyphPositionAtCoordinate(1, 1);
    EXPECT_EQ(positionWithAffinity.position_, 0);
}

/**
 * @tc.name: TextPattern002
 * @tc.desc: Test TextPattern ConvertLocalOffsetToParagraphOffset
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern ConvertLocalOffsetToParagraphOffset
     */
    auto [frameNode, pattern] = Init();
    Offset offset = Offset(1, 1);
    auto paragraphOffset = pattern->ConvertLocalOffsetToParagraphOffset(offset);
    EXPECT_EQ(paragraphOffset.GetX(), 1);
}

/**
 * @tc.name: TextPattern003
 * @tc.desc: Test TextPattern GetLineMetrics
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern GetLineMetrics
     */
    auto [frameNode, pattern] = Init();
    auto textLineMetrics = pattern->GetLineMetrics(1);
    EXPECT_EQ(textLineMetrics.lineNumber, 0);
}

/**
 * @tc.name: TextPattern004
 * @tc.desc: Test TextPattern DidExceedMaxLines
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern DidExceedMaxLines
     */
    auto [frameNode, pattern] = Init();
    auto didExceedMaxLines = pattern->DidExceedMaxLines();
    EXPECT_EQ(didExceedMaxLines, false);
}

/**
 * @tc.name: TextPattern005
 * @tc.desc: Test TextPattern GetTextContentRect
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern GetTextContentRect
     */
    auto [frameNode, pattern] = Init();
    auto textRect = pattern->GetTextContentRect(true);
    EXPECT_EQ(textRect.GetY(), 0);
}

/**
 * @tc.name: TextPattern006
 * @tc.desc: Test TextPattern SetExternalSpanItem
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern SetExternalSpanItem
     */
    auto [frameNode, pattern] = Init();
    std::list<RefPtr<SpanItem>> spanItems;
    pattern->SetExternalSpanItem(spanItems);
    EXPECT_EQ(pattern->isSpanStringMode_, false);
}

/**
 * @tc.name: TextPattern007
 * @tc.desc: Test TextPattern ConvertGlobalToLocalOffset
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern ConvertGlobalToLocalOffset
     */
    auto [frameNode, pattern] = Init();
    Offset globalOffset = Offset(1, 1);
    auto localPoint = pattern->ConvertGlobalToLocalOffset(globalOffset);
    EXPECT_EQ(localPoint.GetY(), 1);
}

/**
 * @tc.name: TextPattern008
 * @tc.desc: Test TextPattern OnSensitiveStyleChange
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern OnSensitiveStyleChange
     */
    auto [frameNode, pattern] = Init();
    pattern->OnSensitiveStyleChange(true);
    EXPECT_EQ(pattern->isSensitive_, 1);
}

/**
 * @tc.name: TextPattern009
 * @tc.desc: Test TextPattern ProcessSpanString
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern ProcessSpanString
     */
    auto [frameNode, pattern] = Init();
    pattern->dataDetectorAdapter_->aiDetectInitialized_ = false;
    pattern->ProcessSpanString();
    EXPECT_EQ(StringUtils::ToWstring(pattern->textForDisplay_).length(), 0);
}

/**
 * @tc.name: TextPattern010
 * @tc.desc: Test TextPattern CreateImageSourceInfo
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern CreateImageSourceInfo
     */
    auto [frameNode, pattern] = Init();
    ImageSpanAttribute imageStyle;
    ImageSpanOptions options;
    options.imageAttribute = imageStyle;
    options.image = IMAGE_VALUE;
    options.bundleName = BUNDLE_NAME;
    options.moduleName = MODULE_NAME;
    options.offset = 1;
    auto imageSourceInfo = pattern->CreateImageSourceInfo(options);
    EXPECT_EQ(imageSourceInfo.isFromReset_, 0);
}

/**
 * @tc.name: TextPattern011
 * @tc.desc: Test TextPattern CreateImageSourceInfo
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern CreateImageSourceInfo
     */
    auto [frameNode, pattern] = Init();

    /**
     * @tc.steps: step2. Create spanBases and gestureInfo
     */
    std::vector<RefPtr<SpanBase>> spanBases;
    GestureStyle gestureInfo;
    ConstructGestureStyle(gestureInfo);
    spanBases.emplace_back(AceType::MakeRefPtr<GestureSpan>(gestureInfo, 0, 3));
    spanBases.emplace_back(AceType::MakeRefPtr<GestureSpan>(gestureInfo, 8, 11));
    auto spanStringWithSpans = AceType::MakeRefPtr<SpanString>("01234567891");
    spanStringWithSpans->BindWithSpans(spanBases);
    auto spans = spanStringWithSpans->GetSpanItems();
    pattern->SetSpanItemChildren(spans);

    /**
     * @tc.steps: step3. test pattern MountImageNode
     */
    auto spanItem = AceType::MakeRefPtr<ImageSpanItem>();
    spanItem->content = " ";
    spanItem->placeholderIndex = 0;
    pattern->MountImageNode(spanItem);
    EXPECT_EQ(pattern->childNodes_.size(), 1);
}

/**
 * @tc.name: TextPattern012
 * @tc.desc: Test TextPattern CreateImageSourceInfo
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern CreateImageSourceInfo
     */
    auto [frameNode, pattern] = Init();
    TextMarqueeState state = TextMarqueeState::START;
    pattern->FireOnMarqueeStateChange(state);
    EXPECT_EQ(pattern->isMarqueeRunning_, true);
    state = TextMarqueeState::FINISH;
    pattern->FireOnMarqueeStateChange(state);
    EXPECT_EQ(pattern->isMarqueeRunning_, false);
}

/**
 * @tc.name: TextPattern013
 * @tc.desc: Test TextPattern CopyBindSelectionMenuParams
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode
     */
    auto [frameNode, pattern] = Init();
    pattern->RemoveAreaChangeInner();

    /**
     * @tc.steps: step2. create SelectOverlayInfo
     */
    SelectOverlayInfo selectInfo;
    selectInfo.singleLineHeight = NODE_ID;
    auto root = AceType::MakeRefPtr<FrameNode>(ROOT_TAG, -1, AceType::MakeRefPtr<Pattern>(), true);
    auto selectOverlayManager = AceType::MakeRefPtr<SelectOverlayManager>(root);
    auto proxy = selectOverlayManager->CreateAndShowSelectOverlay(selectInfo, nullptr, false);
    std::function<void()> buildFunc = []() {
        callBack1 = 1;
        return;
    };
    std::function<void(int32_t, int32_t)> onAppear = [](int32_t a, int32_t b) {
        callBack2 = 2;
        return;
    };
    std::function<void()> onDisappear = []() {
        callBack3 = 3;
        return;
    };

    /**
     * @tc.steps: step3. test pattern CopyBindSelectionMenuParams
     */
    std::shared_ptr<SelectionMenuParams> menuParams = std::make_shared<SelectionMenuParams>(
        TextSpanType::MIXED, buildFunc, onAppear, onDisappear, TextResponseType::RIGHT_CLICK);
    pattern->CopyBindSelectionMenuParams(selectInfo, menuParams);
    EXPECT_EQ(selectInfo.singleLineHeight, NODE_ID);
}

/**
 * @tc.name: TextPattern014
 * @tc.desc: Test TextPattern CopySelectionMenuParams
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode
     */
    auto [frameNode, pattern] = Init();

    /**
     * @tc.steps: step2. create SelectOverlayInfo
     */
    SelectOverlayInfo selectInfo;
    selectInfo.singleLineHeight = NODE_ID;
    auto root = AceType::MakeRefPtr<FrameNode>(ROOT_TAG, -1, AceType::MakeRefPtr<Pattern>(), true);
    auto selectOverlayManager = AceType::MakeRefPtr<SelectOverlayManager>(root);
    auto proxy = selectOverlayManager->CreateAndShowSelectOverlay(selectInfo, nullptr, false);
    std::function<void()> buildFunc = []() {
        callBack1 = 1;
        return;
    };
    std::function<void(int32_t, int32_t)> onAppear = [](int32_t a, int32_t b) {
        callBack2 = 2;
        return;
    };
    std::function<void()> onDisappear = []() {
        callBack3 = 3;
        return;
    };

    /**
     * @tc.steps: step3. test pattern CopySelectionMenuParams
     */
    selectInfo.isUsingMouse = true;
    pattern->selectedType_ = TextSpanType::MIXED;
    pattern->CopySelectionMenuParams(selectInfo, TextResponseType::LONG_PRESS);
    EXPECT_EQ(selectInfo.menuInfo.menuIsShow, false);

    pattern->selectedType_ = TextSpanType::TEXT;
    pattern->CopySelectionMenuParams(selectInfo, TextResponseType::LONG_PRESS);
    EXPECT_EQ(selectInfo.singleLineHeight, NODE_ID);

    pattern->selectedType_ = TextSpanType::IMAGE;
    pattern->CopySelectionMenuParams(selectInfo, TextResponseType::LONG_PRESS);
    EXPECT_EQ(selectInfo.singleLineHeight, NODE_ID);

    pattern->selectedType_ = TextSpanType(-1);
    pattern->CopySelectionMenuParams(selectInfo, TextResponseType::LONG_PRESS);
    EXPECT_EQ(selectInfo.singleLineHeight, NODE_ID);
}

/**
 * @tc.name: TextPattern015
 * @tc.desc: Test TextPattern CopySelectionMenuParams
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern015, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and pattern
     */
    auto [frameNode, pattern] = Init();
    pattern->textSelector_.Update(0, TEXT_SIZE_INT);

    /**
     * @tc.steps: step2. test pattern OnHandleAreaChanged
     */
    RectF handleRect = CONTENT_RECT;
    pattern->selectOverlay_->OnHandleMoveDone(handleRect, true);
    pattern->OnHandleAreaChanged();
    EXPECT_EQ(pattern->selectOverlay_->SelectOverlayIsOn(), false);
}

/**
 * @tc.name: TextPattern016
 * @tc.desc: Test TextPattern CopySelectionMenuParams
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern016, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and pattern
     */
    auto [frameNode, pattern] = Init();
    pattern->textSelector_.Update(0, TEXT_SIZE_INT);

    /**
     * @tc.steps: step2. test pattern HandleSurfaceChanged
     */
    int32_t newWidth = 1;
    int32_t newHeight = 1;
    int32_t prevWidth = 0;
    int32_t prevHeight = 0;
    pattern->HandleSurfaceChanged(newWidth, newHeight, prevWidth, prevHeight);
    EXPECT_EQ(pattern->selectOverlay_->IsShowMouseMenu(), false);

    /**
     * @tc.steps: step3. set Width Height equal before adn after, test pattern HandleSurfaceChanged
     */
    newWidth = 1;
    newHeight = 1;
    prevWidth = 1;
    prevHeight = 1;
    pattern->HandleSurfaceChanged(newWidth, newHeight, prevWidth, prevHeight);
    EXPECT_EQ(pattern->selectOverlay_->IsShowMouseMenu(), false);
}

/**
 * @tc.name: TextPattern017
 * @tc.desc: Test TextPattern CopySelectionMenuParams
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern017, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and pattern
     */
    auto [frameNode, pattern] = Init();
    pattern->textSelector_.Update(0, TEXT_SIZE_INT);

    /**
     * @tc.steps: step2. create SelectOverlayInfo
     */
    SelectOverlayInfo selectInfo;
    selectInfo.singleLineHeight = NODE_ID;
    auto root = AceType::MakeRefPtr<FrameNode>(ROOT_TAG, -1, AceType::MakeRefPtr<Pattern>(), true);
    auto selectOverlayManager = AceType::MakeRefPtr<SelectOverlayManager>(root);
    auto proxy = selectOverlayManager->CreateAndShowSelectOverlay(selectInfo, nullptr, false);
    std::function<void()> buildFunc = []() {
        callBack1 = 1;
        return;
    };
    std::function<void(int32_t, int32_t)> onAppear = [](int32_t a, int32_t b) {
        callBack2 = 2;
        return;
    };
    std::function<void()> onDisappear = []() {
        callBack3 = 3;
        return;
    };

    /**
     * @tc.steps: step3. test pattern UpdateSelectOverlayOrCreate
     */
    pattern->UpdateSelectOverlayOrCreate(selectInfo, true);
    EXPECT_EQ(pattern->selectOverlay_->IsShowMouseMenu(), false);

    pattern->UpdateSelectOverlayOrCreate(selectInfo, false);
    EXPECT_EQ(pattern->selectOverlay_->IsShowMouseMenu(), false);
}

/**
 * @tc.name: TextPattern018
 * @tc.desc: Test TextPattern CopySelectionMenuParams
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNg, TextPattern018, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode and test pattern IsShowHandle
     */
    auto [frameNode, pattern] = Init();
    pattern->textSelector_.Update(0, TEXT_SIZE_INT);

    auto IsShowHandle = pattern->IsShowHandle();
    EXPECT_EQ(IsShowHandle, false);
}
} // namespace OHOS::Ace::NG
