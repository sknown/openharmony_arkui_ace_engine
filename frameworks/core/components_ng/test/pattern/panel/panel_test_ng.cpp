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

#define private public
#define protected public
#include "base/geometry/dimension.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/common/container.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/panel/sliding_panel_event_hub.h"
#include "core/components_ng/pattern/panel/sliding_panel_layout_algorithm.h"
#include "core/components_ng/pattern/panel/sliding_panel_model_ng.h"
#include "core/components_ng/pattern/panel/sliding_panel_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/ace_events.h"
#include "core/pipeline_ng/test/mock/mock_pipeline_base.h"
#include "core/components_ng/test/mock/rosen/mock_canvas.h"
#include "core/components_ng/test/mock/render/mock_render_context.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
constexpr bool SLIDING_PANEL_SHOW = true;
constexpr bool SLIDING_PANEL_LAYOUT_PROPERTY_HAS_DRAG_BAR = false;
constexpr bool SLIDING_PANEL_HAS_DRAG_BAR_FALSE = false;
constexpr bool SLIDING_PANEL_HAS_DRAG_BAR_TRUE = true;

constexpr float ZERO = 0.0f;
constexpr float FULL_SCREEN_WIDTH = 720.0f;
constexpr float FULL_SCREEN_HEIGHT = 1136.0f;
constexpr float PANEL_HEIGHT = 900.0f;
constexpr float COLUMN_HEIGHT = 100.0f;
constexpr float ROW_WIDTH = 300.f;
constexpr float ROW_HEIGHT = 100.f;
constexpr float HEIGHT_1 = 100.0f;
constexpr float HEIGHT_2 = 400.0f;
constexpr float START_X = 50.0f;
constexpr float START_Y = 100.0f;
constexpr float MAIN_DELTA = 80.0f;
constexpr float MAIN_VELOCITY = 500.0f;
constexpr float CURRENT_OFFSET = 200.f;
constexpr float DRAG_LENGTH = 100.0f;
constexpr float DRAG_VELOCITY = 50.0f;
constexpr float VELOCITY_THRESHOLD = 1000.0; // Move 1000px per second.
constexpr float FULL_MIN_BOUNDARY = 500.0f;
constexpr float FULL_HALF_BOUNDARY = 800.0f;
constexpr float HALF_MINI_BOUNDARY = 200.0f;
constexpr float TARGET_LOCATION = 480.0f;
constexpr float DEFAULT_BLANK_HEIGHT_MODE_HALF = 568.0f;
constexpr float DRAG_ICON_WIDTH = 64.0f;
constexpr float DRAG_ICON_HEIGHT = 24.0f;

const Offset START_POINT(START_X, START_Y);
const SizeF CONTAINER_SIZE(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
const SizeF DRAGBAR_SIZE(DRAG_ICON_WIDTH, DRAG_ICON_HEIGHT);
const SizeF PANEL_SIZE(FULL_SCREEN_WIDTH, PANEL_HEIGHT);
const SizeF COLUMN_SIZE(FULL_SCREEN_WIDTH, COLUMN_HEIGHT);
const SizeF ROW_SIZE(ROW_WIDTH, ROW_HEIGHT);
const OffsetF ORIGIN_POINT(ZERO, ZERO);
const OffsetF COLUMN_OFFSET(ZERO, ZERO);
const OffsetF ROW_OFFSET(ZERO, COLUMN_HEIGHT);
constexpr Color BACKGROUND_COLOR_VALUE = Color(0XFFFF0000);
constexpr Color BORDER_COLOR = Color(0XFFDD0000);
constexpr Dimension BORDER_WIDTH = 5.0_px;
constexpr Dimension SLIDING_PANEL_MINI_HEIGHT = 50.0_px;
constexpr Dimension SLIDING_PANEL_HALF_HEIGHT = 300.0_px;
constexpr Dimension SLIDING_PANEL_FULL_HEIGHT = 600.0_px;
constexpr PanelType PANEL_TYPE_VALUE = PanelType::MINI_BAR;
constexpr PanelMode PANEL_MODE_VALUE = PanelMode::FULL;
constexpr BorderStyle BORDER_STYLE = BorderStyle::DASHED;
const std::vector<PanelType> PANEL_PANEL_TYPE = { PanelType::MINI_BAR, PanelType::FOLDABLE_BAR,
    PanelType::TEMP_DISPLAY };
} // namespace

struct TestProperty {
    std::optional<PanelType> panelType = std::nullopt;
    std::optional<PanelMode> panelMode = std::nullopt;
    std::optional<bool> hasDragBar = std::nullopt;
    std::optional<Dimension> miniHeight = std::nullopt;
    std::optional<Dimension> halfHeight = std::nullopt;
    std::optional<Dimension> fullHeight = std::nullopt;
    std::optional<bool> isShow = std::nullopt;
};

class PanelTestNg : public testing::Test {
public:
    static void SetUpTestSuite() {};
    static void TearDownTestSuite() {};
    void SetUp() override;
    void TearDown() override;

    PaddingProperty CreatePadding(float left, float top, float right, float bottom)
    {
        PaddingProperty padding;
        padding.left = CalcLength(left);
        padding.right = CalcLength(right);
        padding.top = CalcLength(top);
        padding.bottom = CalcLength(bottom);
        return padding;
    }

protected:
    static RefPtr<FrameNode> CreateSlidingPanel(const TestProperty& testProperty);
};

void PanelTestNg::SetUp()
{
    MockPipelineBase::SetUp();
}
void PanelTestNg::TearDown()
{
    MockPipelineBase::TearDown();
}

RefPtr<FrameNode> PanelTestNg::CreateSlidingPanel(const TestProperty& testProperty)
{
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    if (testProperty.panelType.has_value()) {
        slidingPanelModelNG.SetPanelType(testProperty.panelType.value());
    }
    if (testProperty.panelMode.has_value()) {
        slidingPanelModelNG.SetPanelMode(testProperty.panelMode.value());
    }
    if (testProperty.hasDragBar.has_value()) {
        slidingPanelModelNG.SetHasDragBar(testProperty.hasDragBar.value());
    }
    if (testProperty.miniHeight.has_value()) {
        slidingPanelModelNG.SetMiniHeight(testProperty.miniHeight.value());
    }
    if (testProperty.halfHeight.has_value()) {
        slidingPanelModelNG.SetHalfHeight(testProperty.halfHeight.value());
    }
    if (testProperty.fullHeight.has_value()) {
        slidingPanelModelNG.SetFullHeight(testProperty.fullHeight.value());
    }
    if (testProperty.isShow.has_value()) {
        slidingPanelModelNG.SetIsShow(testProperty.isShow.value());
    }

    return ViewStackProcessor::GetInstance()->GetMainFrameNode();
}

/**
 * @tc.name: PanelTestNg001
 * @tc.desc: Verify whether the layout property, layoutAlgorithm and event functions are created.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelType(PANEL_TYPE_VALUE);
    slidingPanelModelNG.SetPanelMode(PANEL_MODE_VALUE);
    slidingPanelModelNG.SetBackgroundColor(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_HAS_DRAG_BAR_TRUE);
    slidingPanelModelNG.SetIsShow(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetBorderColor(BORDER_COLOR);
    slidingPanelModelNG.SetBorderWidth(BORDER_WIDTH);
    slidingPanelModelNG.SetBorderStyle(BORDER_STYLE);
    slidingPanelModelNG.SetBorder(BORDER_STYLE, BORDER_WIDTH);
    slidingPanelModelNG.SetBackgroundMask(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.Pop();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);

    /**
     * @tc.steps: step2. get pattern and create layout property, layoutAlgorithm, event.
     * @tc.expected: step2. related function is called.
     */
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(slidingPanelPattern == nullptr);
    auto slidingPanelLayoutProperty = slidingPanelPattern->CreateLayoutProperty();
    EXPECT_FALSE(slidingPanelLayoutProperty == nullptr);
    auto layoutAlgorithm = slidingPanelPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(layoutAlgorithm == nullptr);
    auto eventHub = slidingPanelPattern->GetEventHub<SlidingPanelEventHub>();
    EXPECT_FALSE(eventHub == nullptr);

    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(frameNode, geometryNode, frameNode->GetLayoutProperty());
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));
    auto layoutAlgorithmWrapper = AceType::DynamicCast<LayoutAlgorithmWrapper>(layoutWrapper->GetLayoutAlgorithm());
    EXPECT_FALSE(layoutAlgorithmWrapper == nullptr);
    auto slidingPanelLayoutAlgorithm =
        AceType::DynamicCast<SlidingPanelLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    EXPECT_FALSE(slidingPanelLayoutAlgorithm == nullptr);
}

/**
 * @tc.name: PanelTestNg002
 * @tc.desc: Test event function of slidingPanel.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);

    /**
     * @tc.steps: step2. set slidingPanel event.
     * @tc.expected: step2. function is called.
     */
    auto height_1 = HEIGHT_1;
    auto onHeightChange = [&height_1](float isHeight) { height_1 = isHeight; };
    slidingPanelModelNG.SetOnHeightChange(onHeightChange);

    auto height_2 = HEIGHT_2;
    PanelMode mode = PanelMode::MINI;
    auto sizeChangeEvent = [&mode, &height_2](const BaseEventInfo* info) {
        auto eventInfo = TypeInfoHelper::DynamicCast<SlidingPanelSizeChangeEvent>(info);
        mode = eventInfo->GetMode();
        height_2 = eventInfo->GetHeight();
    };
    slidingPanelModelNG.SetOnSizeChange(sizeChangeEvent);

    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);
    auto heightEventHub = frameNode->GetEventHub<SlidingPanelEventHub>();
    CHECK_NULL_VOID(heightEventHub);
    auto changeEvent = frameNode->GetEventHub<SlidingPanelEventHub>();
    CHECK_NULL_VOID(changeEvent);

    /**
     * @tc.steps: step3. call the event entry function.
     * @tc.expected: step3. check whether the value is correct.
     */
    heightEventHub->FireHeightChangeEvent(HEIGHT_2);
    EXPECT_EQ(height_1, HEIGHT_2);
    changeEvent->FireSizeChangeEvent(PanelMode::HALF, FULL_SCREEN_WIDTH, HEIGHT_2);
    EXPECT_EQ(mode, PanelMode::HALF);
    EXPECT_EQ(height_2, HEIGHT_2);
}

/**
 * @tc.name: PanelTestNg003
 * @tc.desc: Test all the properties of slidingPanel.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize all properties of panel.
     */
    TestProperty testProperty;
    testProperty.panelType = std::make_optional(PANEL_TYPE_VALUE);
    testProperty.panelMode = std::make_optional(PANEL_MODE_VALUE);
    testProperty.hasDragBar = std::make_optional(SLIDING_PANEL_HAS_DRAG_BAR_FALSE);
    testProperty.miniHeight = std::make_optional(SLIDING_PANEL_MINI_HEIGHT);
    testProperty.halfHeight = std::make_optional(SLIDING_PANEL_HALF_HEIGHT);
    testProperty.fullHeight = std::make_optional(SLIDING_PANEL_FULL_HEIGHT);
    testProperty.isShow = std::make_optional(SLIDING_PANEL_SHOW);

    /**
     * @tc.steps: step2. create frameNode to get layout properties.
     * @tc.expected: step2. related function is called.
     */
    RefPtr<FrameNode> frameNode = CreateSlidingPanel(testProperty);
    EXPECT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    EXPECT_NE(layoutProperty, nullptr);
    RefPtr<SlidingPanelLayoutProperty> slidingPanelLayoutProperty =
        AceType::DynamicCast<SlidingPanelLayoutProperty>(layoutProperty);
    EXPECT_NE(slidingPanelLayoutProperty, nullptr);

    /**
     * @tc.steps: step3. get the properties of all settings.
     * @tc.expected: step3. check whether the properties is correct.
     */
    EXPECT_EQ(slidingPanelLayoutProperty->GetPanelType(), PANEL_TYPE_VALUE);
    EXPECT_EQ(slidingPanelLayoutProperty->GetPanelMode(), PANEL_MODE_VALUE);
    EXPECT_EQ(slidingPanelLayoutProperty->GetHasDragBar(), SLIDING_PANEL_HAS_DRAG_BAR_FALSE);
    EXPECT_EQ(slidingPanelLayoutProperty->GetMiniHeight(), SLIDING_PANEL_MINI_HEIGHT);
    EXPECT_EQ(slidingPanelLayoutProperty->GetHalfHeight(), SLIDING_PANEL_HALF_HEIGHT);
    EXPECT_EQ(slidingPanelLayoutProperty->GetFullHeight(), SLIDING_PANEL_FULL_HEIGHT);
    EXPECT_EQ(slidingPanelLayoutProperty->GetIsShow(), SLIDING_PANEL_SHOW);
}

/**
 * @tc.name: PanelTestNg004
 * @tc.desc: Test panel onModifyDone.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetHalfHeight(SLIDING_PANEL_HALF_HEIGHT);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);
    frameNode->MarkModifyDone();

    /**
     * @tc.steps: step2. get pattern and update frameNode.
     * @tc.expected: step2. related function is called.
     */
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(slidingPanelPattern == nullptr);
    auto slidingPanelLayoutProperty = slidingPanelPattern->CreateLayoutProperty();
    EXPECT_FALSE(slidingPanelLayoutProperty == nullptr);
    auto eventHub = slidingPanelPattern->CreateEventHub();
    EXPECT_FALSE(eventHub == nullptr);
    slidingPanelModelNG.SetFullHeight(SLIDING_PANEL_FULL_HEIGHT);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_LAYOUT_PROPERTY_HAS_DRAG_BAR);
    auto newFrameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(newFrameNode == nullptr);
    /**
     * @tc.steps: step3. frameNode markOnModifyDone.
     * @tc.expected: step3. check whether the properties is correct.
     */
    newFrameNode->MarkModifyDone();
    auto layoutProperty = newFrameNode->GetLayoutProperty<SlidingPanelLayoutProperty>();
    EXPECT_FALSE(layoutProperty == nullptr);
    EXPECT_EQ(layoutProperty->GetFullHeight().value_or(Dimension(700)).Value(), SLIDING_PANEL_FULL_HEIGHT.Value());
    EXPECT_EQ(layoutProperty->GetHasDragBar().value_or(true), SLIDING_PANEL_LAYOUT_PROPERTY_HAS_DRAG_BAR);
}

/**
 * @tc.name: PanelTestNg005
 * @tc.desc: Test panel pattern OnDirtyLayoutWrapperSwap.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelMode(PanelMode::FULL);
    auto panelFrameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(panelFrameNode == nullptr);

    /**
     * @tc.steps: step2. get layout property, layoutAlgorithm and create layoutWrapper.
     * @tc.expected: step2. related function is called.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(panelFrameNode, geometryNode, panelFrameNode->GetLayoutProperty());
    auto panelPattern = panelFrameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(panelPattern == nullptr);
    auto panelLayoutProperty = panelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    EXPECT_FALSE(panelLayoutProperty == nullptr);
    auto layoutAlgorithm = AceType::DynamicCast<SlidingPanelLayoutAlgorithm>(panelPattern->CreateLayoutAlgorithm());
    EXPECT_FALSE(layoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));

    /**
     * @tc.steps: step3. update layoutWrapper.
     */
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(PANEL_HEIGHT)));

    /**
     * @tc.steps: step4. frameNode onDirtyLayoutWrapperSwap.
     */
    DirtySwapConfig config;
    config.skipLayout = false;
    config.skipMeasure = false;
    auto layoutAlgorithmWrapper = AceType::DynamicCast<LayoutAlgorithmWrapper>(layoutWrapper->GetLayoutAlgorithm());
    EXPECT_FALSE(layoutAlgorithmWrapper == nullptr);
    auto slidingPanelLayoutAlgorithm =
        AceType::DynamicCast<SlidingPanelLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    EXPECT_FALSE(slidingPanelLayoutAlgorithm == nullptr);
    panelPattern->mode_ = panelLayoutProperty->GetPanelMode().value_or(PanelMode::HALF);
    panelPattern->type_ = panelLayoutProperty->GetPanelType().value_or(PanelType::FOLDABLE_BAR);
    panelPattern->previousMode_ = panelPattern->mode_.value_or(PanelMode::HALF);
    panelPattern->isFirstLayout_ = true;
    panelPattern->currentOffset_ = 0.0f;
    panelPattern->isFirstLayout_ = false;
    panelPattern->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    bool flag = panelPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_TRUE(flag == true);
    panelPattern->isShow_ = true;
    config.skipLayout = true;
    config.skipMeasure = true;
    flag = panelPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_FALSE(flag);
    flag = true;
    /**
     * @tc.steps: step5. frameNode HeightDynamicUpdate.
     */
    panelPattern->isDrag_ = false;
    panelPattern->isAnimating_ = false;
    panelPattern->previousMode_ = PanelMode::HALF;
    panelPattern->currentOffset_ = CURRENT_OFFSET;
    panelPattern->defaultBlankHeights_[PanelMode::HALF] = CURRENT_OFFSET;
    panelPattern->HeightDynamicUpdate();
    panelPattern->previousMode_ = PanelMode::FULL;
    panelPattern->currentOffset_ = CURRENT_OFFSET;
    panelPattern->defaultBlankHeights_[PanelMode::FULL] = CURRENT_OFFSET;
    panelPattern->HeightDynamicUpdate();
    panelPattern->previousMode_ = PanelMode::MINI;
    panelPattern->currentOffset_ = CURRENT_OFFSET;
    panelPattern->defaultBlankHeights_[PanelMode::MINI] = CURRENT_OFFSET;
    panelPattern->HeightDynamicUpdate();
    panelPattern->previousMode_ = PanelMode::AUTO;
    panelPattern->HeightDynamicUpdate();
    EXPECT_TRUE(flag);
}

/**
 * @tc.name: PanelTestNg006
 * @tc.desc: Test panel pattern HandleDragStart .
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetHalfHeight(SLIDING_PANEL_HALF_HEIGHT);
    slidingPanelModelNG.SetPanelType(PANEL_PANEL_TYPE[0]);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);

    /**
     * @tc.steps: step2. get pattern and test handleDrag.
     * @tc.expected: step2. check whether the gestureEvent info is correct.
     */
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(slidingPanelPattern == nullptr);
    auto panelLayoutProperty = slidingPanelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    EXPECT_FALSE(panelLayoutProperty == nullptr);
    slidingPanelPattern->isAnimating_ = false;
    slidingPanelPattern->currentOffset_ = CURRENT_OFFSET;
    slidingPanelPattern->dragStartCurrentOffset_ = ZERO;
    slidingPanelPattern->mode_ = panelLayoutProperty->GetPanelMode().value_or(PanelMode::HALF);
    slidingPanelPattern->type_ = panelLayoutProperty->GetPanelType().value_or(PanelType::FOLDABLE_BAR);
    slidingPanelPattern->previousMode_ = slidingPanelPattern->mode_.value_or(PanelMode::HALF);
    slidingPanelPattern->defaultBlankHeights_[slidingPanelPattern->mode_.value_or(PanelMode::HALF)] =
        DEFAULT_BLANK_HEIGHT_MODE_HALF;

    GestureEvent startInfo;
    startInfo.SetLocalLocation(START_POINT);
    slidingPanelPattern->HandleDragStart(startInfo.GetLocalLocation());
    EXPECT_TRUE(startInfo.GetLocalLocation() == START_POINT);

    GestureEvent endInfo;
    endInfo.SetMainVelocity(MAIN_VELOCITY);
    slidingPanelPattern->HandleDragEnd(static_cast<float>(endInfo.GetMainVelocity()));
    EXPECT_TRUE(endInfo.GetMainVelocity() == MAIN_VELOCITY);
}

/**
 * @tc.name: PanelTestNg007
 * @tc.desc: Test panel pattern HandleDragUpdate .
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetHalfHeight(SLIDING_PANEL_HALF_HEIGHT);
    slidingPanelModelNG.SetPanelType(PANEL_PANEL_TYPE[1]);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);

    /**
     * @tc.steps: step2. get pattern and test handleDrag.
     * @tc.expected: step2. check whether the gestureEvent info is correct.
     */
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(slidingPanelPattern == nullptr);
    auto panelLayoutProperty = slidingPanelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    EXPECT_FALSE(panelLayoutProperty == nullptr);
    slidingPanelPattern->isAnimating_ = false;
    slidingPanelPattern->currentOffset_ = CURRENT_OFFSET;
    slidingPanelPattern->dragStartCurrentOffset_ = ZERO;
    slidingPanelPattern->mode_ = panelLayoutProperty->GetPanelMode().value_or(PanelMode::HALF);
    slidingPanelPattern->type_ = panelLayoutProperty->GetPanelType().value_or(PanelType::FOLDABLE_BAR);
    slidingPanelPattern->previousMode_ = slidingPanelPattern->mode_.value_or(PanelMode::HALF);
    slidingPanelPattern->defaultBlankHeights_[slidingPanelPattern->mode_.value_or(PanelMode::HALF)] =
        DEFAULT_BLANK_HEIGHT_MODE_HALF;
    slidingPanelPattern->fullHalfBoundary_ = FULL_HALF_BOUNDARY;
    slidingPanelPattern->halfMiniBoundary_ = HALF_MINI_BOUNDARY;

    GestureEvent updateInfo;
    updateInfo.SetMainDelta(MAIN_DELTA);
    slidingPanelPattern->HandleDragUpdate(updateInfo);
    EXPECT_TRUE(updateInfo.GetMainDelta() == MAIN_DELTA);

    GestureEvent endInfo;
    endInfo.SetMainVelocity(MAIN_VELOCITY);
    slidingPanelPattern->HandleDragEnd(static_cast<float>(endInfo.GetMainVelocity()));
    EXPECT_TRUE(endInfo.GetMainVelocity() == MAIN_VELOCITY);
}

/**
 * @tc.name: PanelTestNg008
 * @tc.desc: Test panel pattern HandleDragEnd .
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetHalfHeight(SLIDING_PANEL_HALF_HEIGHT);
    slidingPanelModelNG.SetPanelType(PANEL_PANEL_TYPE[2]);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);

    /**
     * @tc.steps: step2. get pattern and test handleDrag.
     * @tc.expected: step2. check whether the gestureEvent info is correct.
     */
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(slidingPanelPattern == nullptr);
    auto panelLayoutProperty = slidingPanelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    EXPECT_FALSE(panelLayoutProperty == nullptr);
    slidingPanelPattern->isAnimating_ = false;
    slidingPanelPattern->mode_ = panelLayoutProperty->GetPanelMode().value_or(PanelMode::HALF);
    slidingPanelPattern->type_ = panelLayoutProperty->GetPanelType().value_or(PanelType::FOLDABLE_BAR);
    slidingPanelPattern->previousMode_ = slidingPanelPattern->mode_.value_or(PanelMode::HALF);

    GestureEvent endInfo;
    endInfo.SetMainVelocity(MAIN_VELOCITY);
    slidingPanelPattern->HandleDragEnd(static_cast<float>(endInfo.GetMainVelocity()));
    EXPECT_TRUE(endInfo.GetMainVelocity() == MAIN_VELOCITY);
}

/**
 * @tc.name: PanelTestNg009
 * @tc.desc: Test panel pattern CalculateModeTypeMini.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);

    /**
     * @tc.steps: step2. get pattern and update pattern member variable.
     * @tc.expected: step2. check whether CalculateModeTypeMini function can be executed.
     */
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(slidingPanelPattern == nullptr);

    // Drag velocity not reached to threshold, mode based on the location.
    slidingPanelPattern->currentOffset_ = CURRENT_OFFSET;
    slidingPanelPattern->fullMiniBoundary_ = FULL_MIN_BOUNDARY;
    slidingPanelPattern->CalculateModeTypeMini(DRAG_LENGTH, DRAG_VELOCITY);

    slidingPanelPattern->currentOffset_ = FULL_MIN_BOUNDARY;
    slidingPanelPattern->fullMiniBoundary_ = FULL_MIN_BOUNDARY;
    slidingPanelPattern->CalculateModeTypeMini(DRAG_LENGTH, DRAG_VELOCITY);

    // Drag velocity reached to threshold, mode based on the drag direction.
    slidingPanelPattern->CalculateModeTypeMini(DRAG_LENGTH, VELOCITY_THRESHOLD);
    slidingPanelPattern->CalculateModeTypeMini(DRAG_LENGTH, -VELOCITY_THRESHOLD);
}

/**
 * @tc.name: PanelTestNg0010
 * @tc.desc: Test panel pattern CalculateModeTypeFold.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);

    /**
     * @tc.steps: step2. get pattern and update pattern member variable.
     * @tc.expected: step2. check whether CalculateModeTypeFold function can be executed.
     */
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(slidingPanelPattern == nullptr);

    // Drag velocity not reached to threshold, mode based on the location.
    slidingPanelPattern->currentOffset_ = CURRENT_OFFSET;
    slidingPanelPattern->fullHalfBoundary_ = FULL_HALF_BOUNDARY;
    slidingPanelPattern->halfMiniBoundary_ = HALF_MINI_BOUNDARY;
    slidingPanelPattern->CalculateModeTypeFold(DRAG_LENGTH, DRAG_VELOCITY);

    slidingPanelPattern->currentOffset_ = FULL_HALF_BOUNDARY;
    slidingPanelPattern->CalculateModeTypeFold(DRAG_LENGTH, DRAG_VELOCITY);

    // Drag velocity reached to threshold, mode based on the drag direction.
    slidingPanelPattern->defaultBlankHeights_[PanelMode::HALF] = DEFAULT_BLANK_HEIGHT_MODE_HALF;
    slidingPanelPattern->currentOffset_ = CURRENT_OFFSET;
    slidingPanelPattern->CalculateModeTypeFold(DRAG_LENGTH, VELOCITY_THRESHOLD);

    slidingPanelPattern->currentOffset_ = DEFAULT_BLANK_HEIGHT_MODE_HALF;
    slidingPanelPattern->CalculateModeTypeFold(DRAG_LENGTH, VELOCITY_THRESHOLD);

    slidingPanelPattern->currentOffset_ = CURRENT_OFFSET;
    slidingPanelPattern->CalculateModeTypeFold(DRAG_LENGTH, -VELOCITY_THRESHOLD);

    slidingPanelPattern->currentOffset_ = DEFAULT_BLANK_HEIGHT_MODE_HALF;
    slidingPanelPattern->CalculateModeTypeFold(DRAG_LENGTH, -VELOCITY_THRESHOLD);
}

/**
 * @tc.name: PanelTestNg0011
 * @tc.desc: Test panel pattern CalculateModeTypeFold.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);

    /**
     * @tc.steps: step2. get pattern and update pattern member variable.
     * @tc.expected: step2. check whether CalculateModeTypeFold function can be executed.
     */
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(slidingPanelPattern == nullptr);

    // Drag velocity not reached to threshold, mode based on the location.
    slidingPanelPattern->currentOffset_ = CURRENT_OFFSET;
    slidingPanelPattern->fullHalfBoundary_ = FULL_HALF_BOUNDARY;
    slidingPanelPattern->CalculateModeTypeTemp(DRAG_LENGTH, DRAG_VELOCITY);

    slidingPanelPattern->currentOffset_ = FULL_HALF_BOUNDARY;
    slidingPanelPattern->CalculateModeTypeTemp(DRAG_LENGTH, DRAG_VELOCITY);

    // Drag velocity reached to threshold, mode based on the drag direction.
    slidingPanelPattern->currentOffset_ = CURRENT_OFFSET;
    slidingPanelPattern->CalculateModeTypeTemp(DRAG_LENGTH, VELOCITY_THRESHOLD);
    slidingPanelPattern->CalculateModeTypeTemp(DRAG_LENGTH, -VELOCITY_THRESHOLD);
}

/**
 * @tc.name: PanelTestNg0012
 * @tc.desc: Test panel pattern AnimateTo.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);

    /**
     * @tc.steps: step2. get pattern, layoutAlgorithm and create layoutWrapper.
     * @tc.expected: step2. related function is called.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(frameNode, geometryNode, frameNode->GetLayoutProperty());
    auto panelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(panelPattern == nullptr);
    auto layoutAlgorithm = AceType::DynamicCast<SlidingPanelLayoutAlgorithm>(panelPattern->CreateLayoutAlgorithm());
    EXPECT_FALSE(layoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));

    /**
     * @tc.steps: step3. update layoutWrapper and measure layout.
     */
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(PANEL_HEIGHT)));
    panelPattern->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    layoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    layoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));

    /**
     * @tc.steps: step4.  update pattern member variable.
     * @tc.expected: step4. check whether AnimateTo function can be executed.
     */
    panelPattern->currentOffset_ = CURRENT_OFFSET;
    panelPattern->AnimateTo(TARGET_LOCATION, PANEL_MODE_VALUE);
}

/**
 * @tc.name: PanelTestNg0013
 * @tc.desc: Test panel pattern GetDragBarNode GetPanelType GetPanelMode.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);

    /**
     * @tc.steps: step2. get pattern.
     * @tc.expected: step2. check whether some function can be executed.
     */
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(slidingPanelPattern == nullptr);
    auto panelLayoutProperty = slidingPanelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    EXPECT_FALSE(panelLayoutProperty == nullptr);

    auto panelDragBar = slidingPanelPattern->GetDragBarNode();
    auto panelMode = slidingPanelPattern->GetPanelMode();
    EXPECT_TRUE(panelMode == panelLayoutProperty->GetPanelMode().value_or(PanelMode::HALF));
    auto panelType = slidingPanelPattern->GetPanelType();
    EXPECT_TRUE(panelType == panelLayoutProperty->GetPanelType().value_or(PanelType::FOLDABLE_BAR));
}

/**
 * @tc.name: PanelTestNg0014
 * @tc.desc: Test panel pattern OnAnimationStop UpdateCurrentOffset UpdateCurrentOffsetOnAnimate.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);

    /**
     * @tc.steps: step2. get pattern, layoutAlgorithm and create layoutWrapper.
     * @tc.expected: step2. related function is called.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(frameNode, geometryNode, frameNode->GetLayoutProperty());
    auto panelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(panelPattern == nullptr);
    auto layoutAlgorithm = AceType::DynamicCast<SlidingPanelLayoutAlgorithm>(panelPattern->CreateLayoutAlgorithm());
    EXPECT_FALSE(layoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));

    /**
     * @tc.steps: step3. update layoutWrapper and measure layout.
     */
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(PANEL_HEIGHT)));
    panelPattern->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    layoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    auto layoutProperty = frameNode->GetLayoutProperty();
    RefPtr<LayoutWrapper> child =
        AceType::MakeRefPtr<LayoutWrapper>(frameNode, geometryNode, layoutProperty);
    layoutWrapper->AppendChild(child);
    layoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));

    /**
     * @tc.steps: step4. get pattern.
     * @tc.expected: step4. check whether some function can be executed.
     */
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(slidingPanelPattern == nullptr);

    slidingPanelPattern->OnAnimationStop();
    EXPECT_TRUE(slidingPanelPattern->isAnimating_ == false);
    slidingPanelPattern->UpdateCurrentOffset(CURRENT_OFFSET);
    slidingPanelPattern->UpdateCurrentOffsetOnAnimate(CURRENT_OFFSET);
}

/**
 * @tc.name: PanelTestNg0015
 * @tc.desc: Test panel pattern GetLinearLayoutProperty IsShowChanged FireSizeChangeEvent.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0015, TestSize.Level1)
{
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);

    /**
     * step1. GetLinearLayoutProperty
     */
    auto columnLayoutProperty = slidingPanelModelNG.GetLinearLayoutProperty();
    EXPECT_NE(columnLayoutProperty, nullptr);

    /**
     * step2. IsShowChanged
     */
    auto panelFrameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(panelFrameNode, nullptr);
    auto panelPattern = panelFrameNode->GetPattern<SlidingPanelPattern>();
    bool isShow = true;
    panelPattern->IsShowChanged(isShow);
    EXPECT_TRUE(isShow);
    isShow = false;
    panelPattern->IsShowChanged(isShow);
    EXPECT_FALSE(isShow);
    /**
     * step3. FireSizeChangeEvent
     */
    panelPattern->FireSizeChangeEvent();
    EXPECT_EQ(panelPattern->previousMode_, PanelMode::HALF);
}

/**
 * @tc.name: PanelTestNg0016
 * @tc.desc: Test panel pattern HandleDragStart HandleDragUpdate HandleDragEnd.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0016, TestSize.Level1)
{
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetHalfHeight(SLIDING_PANEL_HALF_HEIGHT);
    slidingPanelModelNG.SetPanelType(PANEL_PANEL_TYPE[0]);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(slidingPanelPattern, nullptr);
    auto panelLayoutProperty = slidingPanelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    ASSERT_NE(panelLayoutProperty, nullptr);
    slidingPanelPattern->isAnimating_ = false;
    slidingPanelPattern->currentOffset_ = CURRENT_OFFSET;
    slidingPanelPattern->dragStartCurrentOffset_ = ZERO;
    slidingPanelPattern->mode_ = panelLayoutProperty->GetPanelMode().value_or(PanelMode::HALF);
    slidingPanelPattern->type_ = panelLayoutProperty->GetPanelType().value_or(PanelType::FOLDABLE_BAR);
    slidingPanelPattern->previousMode_ = slidingPanelPattern->mode_.value_or(PanelMode::FULL);

    slidingPanelPattern->isShow_ = true;

    /**
     * @tc.steps: step1. HandleDragStart.
     * @tc.expected: step1. Check that the branch is covered by the test.
     */
    GestureEvent startInfo;
    startInfo.SetLocalLocation(START_POINT);
    slidingPanelPattern->HandleDragStart(startInfo.GetLocalLocation());
    EXPECT_EQ(startInfo.GetLocalLocation(), START_POINT);

    /**
     * @tc.steps: step2. HandleDragUpdate.
     * @tc.expected: step2. Check that the branch is covered by the test.
     */
    GestureEvent updateInfo;
    updateInfo.SetMainDelta(MAIN_DELTA);
    slidingPanelPattern->HandleDragUpdate(updateInfo);
    EXPECT_EQ(updateInfo.GetMainDelta(), MAIN_DELTA);

    /**
     * @tc.steps: step3. HandleDragEnd.
     * @tc.expected: step3. Check that the branch is covered by the test.
     */
    auto slidingLayoutProperty = slidingPanelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    GestureEvent endInfo;
    endInfo.SetMainVelocity(MAIN_VELOCITY);
    slidingLayoutProperty->propPanelType_ = PanelType::MINI_BAR;
    slidingPanelPattern->HandleDragEnd(static_cast<float>(endInfo.GetMainVelocity()));
    slidingLayoutProperty->propPanelType_ = PanelType::FOLDABLE_BAR;
    slidingPanelPattern->halfMiniBoundary_ = 100.0f;
    slidingPanelPattern->HandleDragEnd(static_cast<float>(endInfo.GetMainVelocity()));
    slidingLayoutProperty->propPanelType_ = PanelType::TEMP_DISPLAY;
    slidingPanelPattern->currentOffset_ = 110.0f;
    slidingPanelPattern->HandleDragEnd(static_cast<float>(endInfo.GetMainVelocity()));
    EXPECT_EQ(endInfo.GetMainVelocity(), MAIN_VELOCITY);
}

/**
 * @tc.name: PanelTestNg0017
 * @tc.desc: Test FireSizeChangeEvent and FireHeightChangeEvent function of slidingPanel without dragBar.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0017, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_LAYOUT_PROPERTY_HAS_DRAG_BAR);
    auto height_1 = HEIGHT_1;
    auto onHeightChange = [&height_1](float isHeight) { height_1 = isHeight; };
    slidingPanelModelNG.SetOnHeightChange(onHeightChange);

    auto height_2 = HEIGHT_2;
    PanelMode mode = PanelMode::MINI;
    auto sizeChangeEvent = [&mode, &height_2](const BaseEventInfo* info) {
        auto eventInfo = TypeInfoHelper::DynamicCast<SlidingPanelSizeChangeEvent>(info);
        mode = eventInfo->GetMode();
        height_2 = eventInfo->GetHeight();
    };
    slidingPanelModelNG.SetOnSizeChange(sizeChangeEvent);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. get pattern and update frameNode.
     * @tc.expected: related function is called.
     */

    auto geometryNode = frameNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameSize(CONTAINER_SIZE);
    auto panelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(panelPattern, nullptr);

    panelPattern->currentOffset_ = CURRENT_OFFSET;
    panelPattern->mode_ = PanelMode::HALF;
    panelPattern->defaultBlankHeights_[panelPattern->mode_.value_or(PanelMode::HALF)] = DEFAULT_BLANK_HEIGHT_MODE_HALF;

    /**
     * @tc.steps: step3. call the event entry function.
     * @tc.expected: check whether the value is correct.
     */
    panelPattern->FireHeightChangeEvent();
    EXPECT_EQ(height_1, FULL_SCREEN_HEIGHT - CURRENT_OFFSET);
    panelPattern->FireSizeChangeEvent();
    EXPECT_EQ(mode, PanelMode::HALF);
    EXPECT_EQ(height_2, FULL_SCREEN_HEIGHT - DEFAULT_BLANK_HEIGHT_MODE_HALF);
}

/**
 * @tc.name: PanelTestNg0018
 * @tc.desc: Test FireSizeChangeEvent and FireHeightChangeEvent function of slidingPanel with dragBar.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0018, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_HAS_DRAG_BAR_TRUE);
    auto height_1 = HEIGHT_1;
    auto onHeightChange = [&height_1](float isHeight) { height_1 = isHeight; };
    slidingPanelModelNG.SetOnHeightChange(onHeightChange);

    auto height_2 = HEIGHT_2;
    PanelMode mode = PanelMode::MINI;
    auto sizeChangeEvent = [&mode, &height_2](const BaseEventInfo* info) {
        auto eventInfo = TypeInfoHelper::DynamicCast<SlidingPanelSizeChangeEvent>(info);
        mode = eventInfo->GetMode();
        height_2 = eventInfo->GetHeight();
    };
    slidingPanelModelNG.SetOnSizeChange(sizeChangeEvent);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto column = AceType::DynamicCast<FrameNode>(frameNode->GetChildAtIndex(0));
    ASSERT_NE(column, nullptr);
    auto dragBarFrameNode = FrameNode::GetOrCreateFrameNode(V2::DRAG_BAR_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<DragBarPattern>(); });
    dragBarFrameNode->MountToParent(column, 0);
    dragBarFrameNode->MarkModifyDone();
    auto dragBarNode = AceType::DynamicCast<FrameNode>(column->GetChildren().front());
    ASSERT_NE(dragBarNode, nullptr);
    EXPECT_EQ(dragBarFrameNode, dragBarNode);

    /**
     * @tc.steps: step2. get pattern and update frameNode.
     * @tc.expected: related function is called.
     */

    auto geometryNode = frameNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameSize(CONTAINER_SIZE);

    auto dragBarGeometryNode = dragBarNode->GetGeometryNode();
    ASSERT_NE(dragBarGeometryNode, nullptr);
    dragBarGeometryNode->SetFrameSize(DRAGBAR_SIZE);

    auto panelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(panelPattern, nullptr);

    panelPattern->currentOffset_ = CURRENT_OFFSET;
    panelPattern->mode_ = PanelMode::HALF;
    panelPattern->defaultBlankHeights_[panelPattern->mode_.value_or(PanelMode::HALF)] = DEFAULT_BLANK_HEIGHT_MODE_HALF;

    /**
     * @tc.steps: step3. call the event entry function.
     * @tc.expected: check whether the value is correct.
     */
    panelPattern->FireHeightChangeEvent();
    EXPECT_EQ(height_1, FULL_SCREEN_HEIGHT - CURRENT_OFFSET - DRAG_ICON_HEIGHT);
    panelPattern->FireSizeChangeEvent();
    EXPECT_EQ(mode, PanelMode::HALF);
    EXPECT_EQ(height_2, FULL_SCREEN_HEIGHT - DEFAULT_BLANK_HEIGHT_MODE_HALF - DRAG_ICON_HEIGHT);
}

/**
 * @tc.name: PanelLayoutTest001
 * @tc.desc: test panel Layout in different panelMode and panelType
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelLayoutTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelMode(PanelMode::FULL);
    auto panelFrameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(panelFrameNode == nullptr);
    auto columnNode = AceType::DynamicCast<NG::FrameNode>(panelFrameNode->GetChildren().front());
    EXPECT_FALSE(columnNode == nullptr);

    /**
     * @tc.steps: step2. get layout property, layoutAlgorithm and create layoutWrapper.
     * @tc.expected: step2. related function is called.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(panelFrameNode, geometryNode, panelFrameNode->GetLayoutProperty());
    auto panelPattern = panelFrameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(panelPattern == nullptr);
    auto panelLayoutProperty = panelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    EXPECT_FALSE(panelLayoutProperty == nullptr);
    auto panelLayoutAlgorithm = panelPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(panelLayoutAlgorithm == nullptr);

    /**
     * @tc.steps: step3. update layoutWrapper.
     */
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(PANEL_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();
    /* corresponding ets code:
        Panel(true){
        }
        .width('100%')
        .height(900)
        .mode(PanelMode.FULL)
    */

    /**
     * @tc.steps: step3. use layoutAlgorithm to measure and layout.
     * @tc.expected: step3. check whether the value of geometry's frameSize and frameOffset is correct.
     */
    panelLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    panelLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), PANEL_SIZE);
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), ORIGIN_POINT);
}

/**
 * @tc.name: PanelLayoutTest002
 * @tc.desc: test panel Layout in different panelMode and panelType
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelLayoutTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelMode(PanelMode::HALF);
    slidingPanelModelNG.SetPanelType(PanelType::MINI_BAR);
    auto panelFrameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(panelFrameNode == nullptr);
    auto columnNode = AceType::DynamicCast<NG::FrameNode>(panelFrameNode->GetChildren().front());
    EXPECT_FALSE(columnNode == nullptr);

    /**
     * @tc.steps: step2. get layout property, layoutAlgorithm and create layoutWrapper.
     * @tc.expected: step2. related function is called.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(panelFrameNode, geometryNode, panelFrameNode->GetLayoutProperty());
    auto panelPattern = panelFrameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(panelPattern == nullptr);
    auto panelLayoutProperty = panelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    EXPECT_FALSE(panelLayoutProperty == nullptr);
    auto panelLayoutAlgorithm = panelPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(panelLayoutAlgorithm == nullptr);

    /**
     * @tc.steps: step3. update layoutWrapper.
     */
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(PANEL_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();
    /* corresponding ets code:
        Panel(true){
        }
        .width('100%')
        .height(900)
        .mode(PanelMode.Half)
        .type(PanelType.Minibar)
    */

    /**
     * @tc.steps: step3. use layoutAlgorithm to measure and layout.
     * @tc.expected: step3. check whether the value of geometry's frameSize and frameOffset is correct.
     */
    panelLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    panelLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), PANEL_SIZE);
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), ORIGIN_POINT);
}

/**
 * @tc.name: PanelLayoutTest003
 * @tc.desc: test panel Layout in different panelMode and panelType
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelLayoutTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelMode(PanelMode::MINI);
    slidingPanelModelNG.SetPanelType(PanelType::TEMP_DISPLAY);
    auto panelFrameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(panelFrameNode == nullptr);
    auto columnNode = AceType::DynamicCast<NG::FrameNode>(panelFrameNode->GetChildren().front());
    EXPECT_FALSE(columnNode == nullptr);

    /**
     * @tc.steps: step2. get layout property, layoutAlgorithm and create layoutWrapper.
     * @tc.expected: step2. related function is called.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(panelFrameNode, geometryNode, panelFrameNode->GetLayoutProperty());
    auto panelPattern = panelFrameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(panelPattern == nullptr);
    auto panelLayoutProperty = panelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    EXPECT_FALSE(panelLayoutProperty == nullptr);
    auto panelLayoutAlgorithm = panelPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(panelLayoutAlgorithm == nullptr);

    /**
     * @tc.steps: step3. update layoutWrapper.
     */
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(PANEL_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();
    /* corresponding ets code:
        Panel(true){
        }
        .width('100%')
        .height(900)
        .mode(PanelMode.Mini)
        .type(PanelType.Temporary)
    */

    /**
     * @tc.steps: step3. use layoutAlgorithm to measure and layout.
     * @tc.expected: step3. check whether the value of geometry's frameSize and frameOffset is correct.
     */
    panelLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    panelLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), PANEL_SIZE);
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), ORIGIN_POINT);
}

/**
 * @tc.name: PanelLayoutTest004
 * @tc.desc: test panel Layout in different panelMode and panelType
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelLayoutTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelMode(PanelMode::AUTO);
    slidingPanelModelNG.SetPanelType(PanelType::FOLDABLE_BAR);
    auto panelFrameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(panelFrameNode == nullptr);
    auto columnNode = AceType::DynamicCast<NG::FrameNode>(panelFrameNode->GetChildren().front());
    EXPECT_FALSE(columnNode == nullptr);

    /**
     * @tc.steps: step2. get layout property, layoutAlgorithm and create layoutWrapper.
     * @tc.expected: step2. related function is called.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(panelFrameNode, geometryNode, panelFrameNode->GetLayoutProperty());
    auto panelPattern = panelFrameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(panelPattern == nullptr);
    auto panelLayoutProperty = panelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    EXPECT_FALSE(panelLayoutProperty == nullptr);
    auto panelLayoutAlgorithm = panelPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(panelLayoutAlgorithm == nullptr);

    /**
     * @tc.steps: step3. update layoutWrapper.
     */
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(PANEL_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();
    /* corresponding ets code:
        Panel(true){
        }
        .width('100%')
        .height(900)
        .type(PanelType.Foldable)
    */

    /**
     * @tc.steps: step3. use layoutAlgorithm to measure and layout.
     * @tc.expected: step3. check whether the value of geometry's frameSize and frameOffset is correct.
     */
    panelLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    panelLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), PANEL_SIZE);
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), ORIGIN_POINT);
}

/**
 * @tc.name: PanelLayoutTest005
 * @tc.desc:  test panel Layout in different panelMode and panelType.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelLayoutTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelMode(PanelMode::HALF);
    slidingPanelModelNG.SetPanelType(PanelType::FOLDABLE_BAR);
    auto panelFrameNode = FrameNode::CreateFrameNode(V2::PANEL_ETS_TAG, 1, AceType::MakeRefPtr<SlidingPanelPattern>());
    EXPECT_FALSE(panelFrameNode == nullptr);

    /**
     * @tc.steps: step2. get layout property, layoutAlgorithm and create layoutWrapper.
     * @tc.expected: step2. related function is called.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(panelFrameNode, geometryNode, panelFrameNode->GetLayoutProperty());
    auto panelPattern = panelFrameNode->GetPattern<SlidingPanelPattern>();
    EXPECT_FALSE(panelPattern == nullptr);
    auto panelLayoutProperty = panelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    EXPECT_FALSE(panelLayoutProperty == nullptr);
    auto panelLayoutAlgorithm = panelPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(panelLayoutAlgorithm == nullptr);

    /**
     * @tc.steps: step3. update layoutWrapper.
     */
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(PANEL_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Panel(true){
        }
        .width('100%')
        .height(900)
        .mode(PanelMode.Half)
        .type(PanelType.Foldable)
    */

    /**
     * @tc.steps: step3. use layoutAlgorithm to measure and layout.
     * @tc.expected: step3. check whether the value of geometry's frameSize and frameOffset is correct.
     */
    panelLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    panelLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), PANEL_SIZE);
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), ORIGIN_POINT);
}

/**
 * @tc.name: PanelChangeEventTestNg001
 * @tc.desc: Test FireSizeChangeEvent and FireHeightChangeEvent function of slidingPanel with dragBar.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelChangeEventTestNg001, TestSize.Level1)
{
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_HAS_DRAG_BAR_TRUE);
    auto height_2 = HEIGHT_2;
    PanelMode mode = PanelMode::MINI;
    auto sizeChangeEvent = [&mode, &height_2](const BaseEventInfo* info) {
        auto eventInfo = TypeInfoHelper::DynamicCast<SlidingPanelSizeChangeEvent>(info);
        mode = eventInfo->GetMode();
        height_2 = eventInfo->GetHeight();
    };
    slidingPanelModelNG.SetOnSizeChange(sizeChangeEvent);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
}

/**
 * @tc.name: PanelTestNg0019
 * @tc.desc: Test DragBarPaintMethod::GetContentDrawFunction.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0019, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get renderContext_ and paintWrapper, then call func.
     * @tc.expected: func function is called.
     */
    MockRenderContext renderContext_;
    auto renderContext = AceType::WeakClaim<RenderContext>(&renderContext_);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    auto layoutProperty = AceType::MakeRefPtr<PaintProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    PaintWrapper paintWrapper(renderContext, geometryNode, layoutProperty);
    DragBarPaintMethod method;
    Testing::MockCanvas canvas;
    auto func = method.GetContentDrawFunction(&paintWrapper);
    ASSERT_NE(func, nullptr);
    func(canvas);
    auto weak = AceType::WeakClaim<DragBarPaintMethod>(&method);
    EXPECT_FALSE(weak.Upgrade());
}

/**
 * @tc.name: PanelTestNg0020
 * @tc.desc: Test SlidingPanelNode::AddChildToGroup.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0020, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get slidingPanelNode, set children_.
     */
    auto pattern = AceType::MakeRefPtr<Pattern>();
    ASSERT_NE(pattern, nullptr);
    SlidingPanelNode slidingPanelNode("SlidingPanelNode", 100, pattern);
    slidingPanelNode.columnChildren_.insert(1);
    auto child = AceType::MakeRefPtr<SlidingPanelNode>("child1", 1, pattern);
    ASSERT_NE(child, nullptr);
    slidingPanelNode.AddChildToGroup(child, 1);
    EXPECT_NE(
        slidingPanelNode.columnChildren_.find(child->GetId()), slidingPanelNode.columnChildren_.end());
    /**
     * @tc.steps: step2. construct a UINode of children.
     * @tc.expected: check whether the value is correct.
     */
    std::list<RefPtr<UINode>> list;
    list.push_back(AceType::MakeRefPtr<SlidingPanelNode>("child3", 3, pattern));
    auto child4 = AceType::MakeRefPtr<SlidingPanelNode>("child4", 4, pattern);
    child4->children_ = list;
    slidingPanelNode.children_.clear();
    slidingPanelNode.children_.push_back(child4);
    auto child2 = AceType::MakeRefPtr<SlidingPanelNode>("child2", 2, pattern);
    slidingPanelNode.AddChildToGroup(child2, 1);
        EXPECT_NE(
        slidingPanelNode.columnChildren_.find(child2->GetId()), slidingPanelNode.columnChildren_.end());
}

/**
 * @tc.name: PanelTestNg0021
 * @tc.desc: Test SlidingPanelPattern::CheckPanelModeAndType.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0021, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelMode(PanelMode::FULL);
    auto panelFrameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(panelFrameNode, nullptr);
    /**
     * @tc.steps: step2. get layoutproperty, geometryNode and create layoutWrapper.
     * @tc.expected: related function is called.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(panelFrameNode, geometryNode, panelFrameNode->GetLayoutProperty());
    auto panelPattern = panelFrameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(panelPattern, nullptr);
    panelPattern->mode_ = PanelMode::HALF;
    panelPattern->type_ = PanelType::MINI_BAR;
    panelPattern->CheckPanelModeAndType();
    EXPECT_EQ(panelPattern->mode_, PanelMode::MINI);
    panelPattern->type_ = PanelType::TEMP_DISPLAY;
    panelPattern->CheckPanelModeAndType();
    EXPECT_EQ(panelPattern->mode_, PanelMode::HALF);
}

/**
 * @tc.name: PanelTestNg0022
 * @tc.desc: Test SlidingPanelPattern::OnModifyDone.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0022, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelType(PANEL_TYPE_VALUE);
    slidingPanelModelNG.SetPanelMode(PANEL_MODE_VALUE);
    slidingPanelModelNG.SetBackgroundColor(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_HAS_DRAG_BAR_TRUE);
    slidingPanelModelNG.SetIsShow(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetBorderColor(BORDER_COLOR);
    slidingPanelModelNG.SetBorderWidth(BORDER_WIDTH);
    slidingPanelModelNG.SetBorderStyle(BORDER_STYLE);
    slidingPanelModelNG.SetBorder(BORDER_STYLE, BORDER_WIDTH);
    slidingPanelModelNG.SetBackgroundMask(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.Pop();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(slidingPanelPattern, nullptr);
    /**
     * @tc.steps: step2. change slidingPanelPattern->isShowQueue_ and slidingPanelPattern->isShow_.
     * @tc.expected: slidingPanelPattern->isShowQueue_.empty() is correct.
     */
    auto host = slidingPanelPattern->GetHost();
    ASSERT_NE(host, nullptr);
    auto layoutProperty = host->GetLayoutProperty<SlidingPanelLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto isShow = layoutProperty->GetIsShowValue(false);
    EXPECT_TRUE(isShow);
    slidingPanelPattern->isShowQueue_ = std::queue<bool>();
    EXPECT_TRUE(slidingPanelPattern->isShowQueue_.empty());
    slidingPanelPattern->isShow_ = false;
    slidingPanelPattern->OnModifyDone();
    EXPECT_TRUE(slidingPanelPattern->isShowQueue_.empty());
}

/**
 * @tc.name: PanelTestNg0023
 * @tc.desc: Test SlidingPanelPattern::Update.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0023, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelType(PANEL_TYPE_VALUE);
    slidingPanelModelNG.SetPanelMode(PANEL_MODE_VALUE);
    slidingPanelModelNG.SetBackgroundColor(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_HAS_DRAG_BAR_TRUE);
    slidingPanelModelNG.SetIsShow(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetBorderColor(BORDER_COLOR);
    slidingPanelModelNG.SetBorderWidth(BORDER_WIDTH);
    slidingPanelModelNG.SetBorderStyle(BORDER_STYLE);
    slidingPanelModelNG.SetBorder(BORDER_STYLE, BORDER_WIDTH);
    slidingPanelModelNG.SetBackgroundMask(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.Pop();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(slidingPanelPattern, nullptr);
    /**
     * @tc.steps: step2. change slidingPanelPattern->mode_, layoutProperty->propPanelType_ and so on.
     * @tc.expected: slidingPanelPattern->mode_.value() is correct.
     */
    auto layoutProperty = slidingPanelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->propPanelMode_ = PanelMode::HALF;
    slidingPanelPattern->mode_ = PanelMode::HALF;
    auto mode = layoutProperty->GetPanelMode() == PanelMode::AUTO
                    ? PanelMode::FULL
                    : layoutProperty->GetPanelMode().value_or(PanelMode::HALF);
    slidingPanelPattern->type_ = layoutProperty->GetPanelType().value_or(PanelType::FOLDABLE_BAR);
    EXPECT_EQ(slidingPanelPattern->type_, PanelType::MINI_BAR);
    EXPECT_EQ(slidingPanelPattern->mode_.value(), mode);
    EXPECT_EQ(slidingPanelPattern->mode_.value(), PanelMode::HALF);
    slidingPanelPattern->isShow_ = true;
    EXPECT_TRUE(slidingPanelPattern->isShow_);
    layoutProperty->propIsShow_ = true;
    slidingPanelPattern->previousMode_ = PanelMode::AUTO;
    slidingPanelPattern->Update();

    layoutProperty->propPanelType_ = PanelType::TEMP_DISPLAY;
    layoutProperty->propPanelMode_ = PanelMode::MINI;
    slidingPanelPattern->mode_ = PanelMode::MINI;
    slidingPanelPattern->Update();
    EXPECT_EQ(slidingPanelPattern->mode_.value(), PanelMode::HALF);
}

/**
 * @tc.name: PanelTestNg0024
 * @tc.desc: Test SlidingPanelPattern::InitializeLayoutProps.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0024, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelType(PANEL_TYPE_VALUE);
    slidingPanelModelNG.SetPanelMode(PANEL_MODE_VALUE);
    slidingPanelModelNG.SetBackgroundColor(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_HAS_DRAG_BAR_TRUE);
    slidingPanelModelNG.SetIsShow(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetBorderColor(BORDER_COLOR);
    slidingPanelModelNG.SetBorderWidth(BORDER_WIDTH);
    slidingPanelModelNG.SetBorderStyle(BORDER_STYLE);
    slidingPanelModelNG.SetBorder(BORDER_STYLE, BORDER_WIDTH);
    slidingPanelModelNG.SetBackgroundMask(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.Pop();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(slidingPanelPattern, nullptr);
    /**
     * @tc.steps: step2. change slidingPanelPattern->isShow_, child.
     * @tc.expected: slidingPanelPattern->isShow_ is correct.
     */
    auto layoutProperty = slidingPanelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto isShow = layoutProperty->GetIsShowValue(false);
    EXPECT_TRUE(isShow);
    slidingPanelPattern->isShow_ = false;
    EXPECT_TRUE(slidingPanelPattern->isShow_.value() != isShow);
    slidingPanelPattern->InitializeLayoutProps();
    EXPECT_FALSE(slidingPanelPattern->isShow_.value() != isShow);

    auto host = slidingPanelPattern->GetHost();
    auto child = host->GetChildren();
    EXPECT_FALSE(child.empty() || child.size() != 1);
    child.clear();
    EXPECT_TRUE(child.empty() || child.size() != 1);
    slidingPanelPattern->InitializeLayoutProps();
    EXPECT_TRUE(slidingPanelPattern->isShow_);
}

/**
 * @tc.name: PanelTestNg0025
 * @tc.desc: Test SlidingPanelPattern::FirstLayout.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0025, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelType(PANEL_TYPE_VALUE);
    slidingPanelModelNG.SetPanelMode(PANEL_MODE_VALUE);
    slidingPanelModelNG.SetBackgroundColor(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_HAS_DRAG_BAR_TRUE);
    slidingPanelModelNG.SetIsShow(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetBorderColor(BORDER_COLOR);
    slidingPanelModelNG.SetBorderWidth(BORDER_WIDTH);
    slidingPanelModelNG.SetBorderStyle(BORDER_STYLE);
    slidingPanelModelNG.SetBorder(BORDER_STYLE, BORDER_WIDTH);
    slidingPanelModelNG.SetBackgroundMask(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.Pop();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(slidingPanelPattern, nullptr);
    /**
     * @tc.steps: step2. change layoutProperty->propIsShow_.
     * @tc.expected: slidingPanelPattern->FirstLayout() is called.
     */
    auto layoutProperty = slidingPanelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_TRUE(layoutProperty->GetIsShowValue(false));
    EXPECT_TRUE(slidingPanelPattern->previousMode_ != slidingPanelPattern->mode_.value_or(PanelMode::HALF));
    EXPECT_TRUE(layoutProperty->GetHasDragBarValue(true));
    auto dragBar = slidingPanelPattern->GetDragBarNode();
    ASSERT_NE(dragBar, nullptr);
    auto dragBarPattern = dragBar->GetPattern<DragBarPattern>();
    ASSERT_NE(dragBarPattern, nullptr);
    slidingPanelPattern->FirstLayout();
    layoutProperty->propIsShow_ = false;
    
    EXPECT_FALSE(layoutProperty->GetIsShowValue(false));
    slidingPanelPattern->FirstLayout();
}

/**
 * @tc.name: PanelTestNg0026
 * @tc.desc: Test SlidingPanelPattern::FirstLayout.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0026, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelType(PANEL_TYPE_VALUE);
    slidingPanelModelNG.SetPanelMode(PANEL_MODE_VALUE);
    slidingPanelModelNG.SetBackgroundColor(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_HAS_DRAG_BAR_TRUE);
    slidingPanelModelNG.SetIsShow(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetBorderColor(BORDER_COLOR);
    slidingPanelModelNG.SetBorderWidth(BORDER_WIDTH);
    slidingPanelModelNG.SetBorderStyle(BORDER_STYLE);
    slidingPanelModelNG.SetBorder(BORDER_STYLE, BORDER_WIDTH);
    slidingPanelModelNG.SetBackgroundMask(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.Pop();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(slidingPanelPattern, nullptr);
    /**
     * @tc.steps: step2. change slidingPanelPattern->previousMode_, slidingPanelPattern->mode_.
     * @tc.expected: hasDragBar is correct.
     */
    auto layoutProperty = slidingPanelPattern->GetLayoutProperty<SlidingPanelLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto hasDragBar = layoutProperty->GetHasDragBarValue(true);
    slidingPanelPattern->previousMode_ = PanelMode::HALF;
    slidingPanelPattern->mode_ = PanelMode::FULL;
    slidingPanelPattern->IsShowChanged(true);
    EXPECT_EQ(slidingPanelPattern->previousMode_, slidingPanelPattern->mode_);
    slidingPanelPattern->IsShowChanged(false);
    EXPECT_TRUE(hasDragBar);
}

/**
 * @tc.name: PanelTestNg0027
 * @tc.desc: Test SlidingPanelPattern::HeightDynamicUpdate.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0027, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelType(PANEL_TYPE_VALUE);
    slidingPanelModelNG.SetPanelMode(PANEL_MODE_VALUE);
    slidingPanelModelNG.SetBackgroundColor(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_HAS_DRAG_BAR_TRUE);
    slidingPanelModelNG.SetIsShow(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetBorderColor(BORDER_COLOR);
    slidingPanelModelNG.SetBorderWidth(BORDER_WIDTH);
    slidingPanelModelNG.SetBorderStyle(BORDER_STYLE);
    slidingPanelModelNG.SetBorder(BORDER_STYLE, BORDER_WIDTH);
    slidingPanelModelNG.SetBackgroundMask(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.Pop();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(slidingPanelPattern, nullptr);
    /**
     * @tc.steps: step2. change slidingPanelPattern->previousMode_, slidingPanelPattern->isAnimating_ and so on.
     * @tc.expected: slidingPanelPattern->previousMode_ is correct.
     */
    slidingPanelPattern->isShow_ = true;
    EXPECT_TRUE(slidingPanelPattern->isShow_.value_or(false));
    EXPECT_FALSE(slidingPanelPattern->isDrag_);
    EXPECT_FALSE(slidingPanelPattern->isAnimating_);

    slidingPanelPattern->previousMode_ = PanelMode::FULL;
    slidingPanelPattern->currentOffset_ = 0.3f;
    slidingPanelPattern->defaultBlankHeights_[PanelMode::FULL] = 0.5f;
    slidingPanelPattern->HeightDynamicUpdate();

    EXPECT_TRUE(slidingPanelPattern->isShow_.value_or(false));
    EXPECT_FALSE(slidingPanelPattern->isDrag_);
    EXPECT_TRUE(slidingPanelPattern->isAnimating_);
    slidingPanelPattern->isAnimating_ = false;
    slidingPanelPattern->previousMode_ = PanelMode::HALF;
    slidingPanelPattern->currentOffset_ = 0.3f;
    slidingPanelPattern->defaultBlankHeights_[PanelMode::HALF] = 0.5f;
    slidingPanelPattern->HeightDynamicUpdate();
    EXPECT_TRUE(slidingPanelPattern->isShow_.value_or(false));
    EXPECT_FALSE(slidingPanelPattern->isDrag_);
    EXPECT_TRUE(slidingPanelPattern->isAnimating_);
    slidingPanelPattern->isAnimating_ = false;
    slidingPanelPattern->previousMode_ = PanelMode::MINI;
    slidingPanelPattern->currentOffset_ = 0.3f;
    slidingPanelPattern->defaultBlankHeights_[PanelMode::MINI] = 0.5f;
    slidingPanelPattern->HeightDynamicUpdate();
    EXPECT_EQ(slidingPanelPattern->previousMode_, PanelMode::MINI);
    slidingPanelPattern->previousMode_ = PanelMode::AUTO;
    slidingPanelPattern->HeightDynamicUpdate();
    EXPECT_EQ(slidingPanelPattern->previousMode_, PanelMode::AUTO);
}

/**
 * @tc.name: PanelTestNg0028
 * @tc.desc: Test SlidingPanelPattern::CheckPanelModeAndType.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0028, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelType(PANEL_TYPE_VALUE);
    slidingPanelModelNG.SetPanelMode(PANEL_MODE_VALUE);
    slidingPanelModelNG.SetBackgroundColor(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_HAS_DRAG_BAR_TRUE);
    slidingPanelModelNG.SetIsShow(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetBorderColor(BORDER_COLOR);
    slidingPanelModelNG.SetBorderWidth(BORDER_WIDTH);
    slidingPanelModelNG.SetBorderStyle(BORDER_STYLE);
    slidingPanelModelNG.SetBorder(BORDER_STYLE, BORDER_WIDTH);
    slidingPanelModelNG.SetBackgroundMask(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.Pop();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(slidingPanelPattern, nullptr);
    /**
     * @tc.steps: step2. change slidingPanelPattern->mode_, slidingPanelPattern->type_.
     * @tc.expected: slidingPanelPattern->mode_ and slidingPanelPattern->type_ are correct.
     */
    slidingPanelPattern->mode_ = PanelMode::HALF;
    slidingPanelPattern->type_ = PanelType::MINI_BAR;
    slidingPanelPattern->CheckPanelModeAndType();
    EXPECT_EQ(slidingPanelPattern->mode_, PanelMode::MINI);
    EXPECT_EQ(slidingPanelPattern->type_, PanelType::MINI_BAR);

    slidingPanelPattern->type_ = PanelType::TEMP_DISPLAY;
    slidingPanelPattern->CheckPanelModeAndType();
    EXPECT_EQ(slidingPanelPattern->mode_, PanelMode::HALF);
    EXPECT_EQ(slidingPanelPattern->type_, PanelType::TEMP_DISPLAY);
}

/**
 * @tc.name: PanelTestNg0029
 * @tc.desc: Test SlidingPanelPattern::CalculateModeTypeFold.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0029, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelType(PANEL_TYPE_VALUE);
    slidingPanelModelNG.SetPanelMode(PANEL_MODE_VALUE);
    slidingPanelModelNG.SetBackgroundColor(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_HAS_DRAG_BAR_TRUE);
    slidingPanelModelNG.SetIsShow(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetBorderColor(BORDER_COLOR);
    slidingPanelModelNG.SetBorderWidth(BORDER_WIDTH);
    slidingPanelModelNG.SetBorderStyle(BORDER_STYLE);
    slidingPanelModelNG.SetBorder(BORDER_STYLE, BORDER_WIDTH);
    slidingPanelModelNG.SetBackgroundMask(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.Pop();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(slidingPanelPattern, nullptr);
    /**
     * @tc.steps: step2. change slidingPanelPattern->currentOffset_.
     * @tc.expected: slidingPanelPattern->currentOffset_ is correct.
     */
    slidingPanelPattern->currentOffset_ = 55.0f;
    slidingPanelPattern->fullHalfBoundary_ = 50.0f;
    slidingPanelPattern->halfMiniBoundary_ = 60.0f;
    slidingPanelPattern->CalculateModeTypeFold(DRAG_LENGTH, DRAG_VELOCITY);
    EXPECT_GE(slidingPanelPattern->currentOffset_, slidingPanelPattern->fullHalfBoundary_);
    EXPECT_LT(slidingPanelPattern->currentOffset_, slidingPanelPattern->halfMiniBoundary_);
    slidingPanelPattern->currentOffset_ = 100.0f;
    slidingPanelPattern->CalculateModeTypeFold(DRAG_LENGTH, DRAG_VELOCITY);
    EXPECT_GE(slidingPanelPattern->currentOffset_, slidingPanelPattern->fullHalfBoundary_);
    EXPECT_GE(slidingPanelPattern->currentOffset_, slidingPanelPattern->halfMiniBoundary_);
}

/**
 * @tc.name: PanelTestNg0030
 * @tc.desc: Test SlidingPanelPattern::CalculateModeTypeFold.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0030, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelType(PANEL_TYPE_VALUE);
    slidingPanelModelNG.SetPanelMode(PANEL_MODE_VALUE);
    slidingPanelModelNG.SetBackgroundColor(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_HAS_DRAG_BAR_TRUE);
    slidingPanelModelNG.SetIsShow(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetBorderColor(BORDER_COLOR);
    slidingPanelModelNG.SetBorderWidth(BORDER_WIDTH);
    slidingPanelModelNG.SetBorderStyle(BORDER_STYLE);
    slidingPanelModelNG.SetBorder(BORDER_STYLE, BORDER_WIDTH);
    slidingPanelModelNG.SetBackgroundMask(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.Pop();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(slidingPanelPattern, nullptr);
    /**
     * @tc.steps: step2. call GetAnimationDuration.
     * @tc.expected: dragRange is correct.
     */
    float delta = 10.0f;
    float dragRange = 5.0f;
    slidingPanelPattern->GetAnimationDuration(delta, dragRange);
    EXPECT_NE(dragRange, 0.0f);
}

/**
 * @tc.name: PanelTestNg0031
 * @tc.desc: Test SlidingPanelPattern::CalculateModeTypeFold.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0031, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create slidingPanel and get frameNode.
     */
    SlidingPanelModelNG slidingPanelModelNG;
    slidingPanelModelNG.Create(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetPanelType(PANEL_TYPE_VALUE);
    slidingPanelModelNG.SetPanelMode(PANEL_MODE_VALUE);
    slidingPanelModelNG.SetBackgroundColor(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.SetHasDragBar(SLIDING_PANEL_HAS_DRAG_BAR_TRUE);
    slidingPanelModelNG.SetIsShow(SLIDING_PANEL_SHOW);
    slidingPanelModelNG.SetBorderColor(BORDER_COLOR);
    slidingPanelModelNG.SetBorderWidth(BORDER_WIDTH);
    slidingPanelModelNG.SetBorderStyle(BORDER_STYLE);
    slidingPanelModelNG.SetBorder(BORDER_STYLE, BORDER_WIDTH);
    slidingPanelModelNG.SetBackgroundMask(BACKGROUND_COLOR_VALUE);
    slidingPanelModelNG.Pop();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto slidingPanelPattern = frameNode->GetPattern<SlidingPanelPattern>();
    ASSERT_NE(slidingPanelPattern, nullptr);
    /**
     * @tc.steps: step2. change slidingPanelPattern->isShowQueue_.
     * @tc.expected: slidingPanelPattern->invisibleFlag_.value_or(true) is correct.
     */
    EXPECT_TRUE(slidingPanelPattern->isShowQueue_.empty());
    slidingPanelPattern->isShowQueue_.push(true);
    slidingPanelPattern->isShowQueue_.push(true);
    slidingPanelPattern->OnAnimationStop();
    EXPECT_FALSE(slidingPanelPattern->invisibleFlag_.value_or(true));
}

/**
 * @tc.name: PanelTestNg0032
 * @tc.desc: Test DragBarPattern::ShowArrow.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0032, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create dragBarPattern, change showMode_.
     * @tc.expected: dragBarPattern.showMode_ is correct.
     */
    DragBarPattern dragBarPattern;
    dragBarPattern.showMode_ = PanelMode::FULL;
    dragBarPattern.ShowArrow(true);
    EXPECT_EQ(dragBarPattern.showMode_, PanelMode::FULL);
    dragBarPattern.ShowArrow(false);
    EXPECT_EQ(dragBarPattern.showMode_, PanelMode::HALF);
}

/**
 * @tc.name: PanelTestNg0033
 * @tc.desc: Test DragBarPattern::ShowArrow.
 * @tc.type: FUNC
 */
HWTEST_F(PanelTestNg, PanelTestNg0033, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create dragBarPattern, set clickArrowCallback_.
     * @tc.expected: dragBarPattern.clickArrowCallback_ is correct.
     */
    DragBarPattern dragBarPattern;
    dragBarPattern.OnClick();
    EXPECT_FALSE(dragBarPattern.clickArrowCallback_);
    dragBarPattern.clickArrowCallback_ = []() {};
    dragBarPattern.OnClick();
    EXPECT_TRUE(dragBarPattern.clickArrowCallback_);
}
} // namespace OHOS::Ace::NG
