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


#include <cstdint>
#include <memory>
#include <utility>

#include "gtest/gtest-test-part.h"
#include "gtest/gtest.h"

#define private public
#define protected public
#include "base/geometry/ng/size_t.h"
#include "base/geometry/offset.h"
#include "base/memory/ace_type.h"
#include "core/animation/animator.h"
#include "core/components/common/properties/color.h"
#include "core/components/scroll/scroll_bar_theme.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_abstract_model_ng.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/linear_layout/column_model_ng.h"
#include "core/components_ng/pattern/linear_layout/row_model_ng.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/scroll/effect/scroll_fade_effect.h"
#include "core/components_ng/pattern/scroll/scroll_model_ng.h"
#include "core/components_ng/pattern/scroll/scroll_paint_property.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_spring_effect.h"
#include "core/components_ng/test/mock/theme/mock_theme_manager.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/constants.h"
#include "core/pipeline_ng/test/mock/mock_pipeline_base.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr float ROOT_WIDTH = 720.0f;
constexpr float ROOT_HEIGHT = 1136.0f;
constexpr Dimension FILL_LENGTH = Dimension(1.0, DimensionUnit::PERCENT);
constexpr float COLUMN_CHILD_WIDTH = ROOT_WIDTH / 10;
constexpr float COLUMN_CHILD_HEIGHT = ROOT_HEIGHT / 10;
constexpr int32_t CHILD_NUMBER = 12;
constexpr float COLUMN_WIDTH = COLUMN_CHILD_WIDTH * CHILD_NUMBER;
constexpr float COLUMN_HEIGHT = COLUMN_CHILD_HEIGHT * CHILD_NUMBER;
} // namespace

class ScrollTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp() override;
    void TearDown() override;
    void GetInstance();
    static void SetWidth(const Dimension& width);
    static void SetHeight(const Dimension& height);
    RefPtr<LayoutWrapper> CreateScroll(Axis axis = Axis::VERTICAL);
    RefPtr<LayoutWrapper> CreateScroll(NG::DisplayMode displayMode);
    RefPtr<LayoutWrapper> CreateScroll(Color color);
    RefPtr<LayoutWrapper> CreateScroll(Dimension barWidth);
    RefPtr<LayoutWrapper> CreateScroll(EdgeEffect edgeEffect);
    RefPtr<LayoutWrapper> CreateScroll(Axis axis, NG::ScrollEvent&& event);
    RefPtr<LayoutWrapper> CreateScroll(Axis axis, NG::ScrollEdgeEvent&& event);
    RefPtr<LayoutWrapper> CreateScroll(Axis axis, OnScrollStartEvent&& event);
    void CreateContent(Axis axis = Axis::VERTICAL);
    RefPtr<LayoutWrapper> RunMeasureAndLayout(
        float width = ROOT_WIDTH, float height = ROOT_HEIGHT);
    RefPtr<FrameNode> GetColumnChild(int32_t index);
    void UpdateCurrentOffset(float offset);
    testing::AssertionResult IsEqualCurrentOffset(Offset expectOffset);

    RefPtr<FrameNode> frameNode_;
    RefPtr<ScrollPattern> pattern_;
    RefPtr<ScrollEventHub> eventHub_;
    RefPtr<ScrollLayoutProperty> layoutProperty_;
    RefPtr<ScrollPaintProperty> paintProperty_;
    RefPtr<ScrollAccessibilityProperty> accessibilityProperty_;
};

void ScrollTestNg::SetUpTestSuite()
{
    MockPipelineBase::SetUp();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    auto scrollBarTheme = AceType::MakeRefPtr<ScrollBarTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(scrollBarTheme));
}

void ScrollTestNg::TearDownTestSuite()
{
    MockPipelineBase::TearDown();
}

void ScrollTestNg::SetUp() {}

void ScrollTestNg::TearDown()
{
    frameNode_ = nullptr;
    pattern_ = nullptr;
    eventHub_ = nullptr;
    layoutProperty_ = nullptr;
    paintProperty_ = nullptr;
    accessibilityProperty_ = nullptr;
}

void ScrollTestNg::GetInstance()
{
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    frameNode_ = AceType::DynamicCast<FrameNode>(element);
    pattern_ = frameNode_->GetPattern<ScrollPattern>();
    eventHub_ = frameNode_->GetEventHub<ScrollEventHub>();
    layoutProperty_ = frameNode_->GetLayoutProperty<ScrollLayoutProperty>();
    paintProperty_ = frameNode_->GetPaintProperty<ScrollPaintProperty>();
    accessibilityProperty_ = frameNode_->GetAccessibilityProperty<ScrollAccessibilityProperty>();
}

void ScrollTestNg::SetWidth(const Dimension& width)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto layoutProperty = frameNode->GetLayoutProperty();
    layoutProperty->UpdateUserDefinedIdealSize(CalcSize(CalcLength(width), std::nullopt));
}

void ScrollTestNg::SetHeight(const Dimension& height)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto layoutProperty = frameNode->GetLayoutProperty();
    layoutProperty->UpdateUserDefinedIdealSize(CalcSize(std::nullopt, CalcLength(height)));
}

RefPtr<LayoutWrapper> ScrollTestNg::CreateScroll(Axis axis)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetAxis(axis);
    CreateContent(axis);
    GetInstance();
    return RunMeasureAndLayout();
}

RefPtr<LayoutWrapper> ScrollTestNg::CreateScroll(NG::DisplayMode displayMode)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetDisplayMode(static_cast<int>(displayMode));
    CreateContent();
    GetInstance();
    return RunMeasureAndLayout();
}

RefPtr<LayoutWrapper> ScrollTestNg::CreateScroll(Color color)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetScrollBarColor(color);
    CreateContent();
    GetInstance();
    return RunMeasureAndLayout();
}

RefPtr<LayoutWrapper> ScrollTestNg::CreateScroll(Dimension barWidth)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetScrollBarWidth(barWidth);
    CreateContent();
    GetInstance();
    return RunMeasureAndLayout();
}

RefPtr<LayoutWrapper> ScrollTestNg::CreateScroll(EdgeEffect edgeEffect)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetEdgeEffect(edgeEffect);
    CreateContent();
    GetInstance();
    return RunMeasureAndLayout();
}

RefPtr<LayoutWrapper> ScrollTestNg::CreateScroll(Axis axis, NG::ScrollEvent&& event)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetAxis(axis);
    scrollModel.SetOnScroll(std::move(event));
    CreateContent(axis);
    GetInstance();
    return RunMeasureAndLayout();
}

RefPtr<LayoutWrapper> ScrollTestNg::CreateScroll(Axis axis, NG::ScrollEdgeEvent&& event)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetAxis(axis);
    scrollModel.SetOnScrollEdge(std::move(event));
    CreateContent(axis);
    GetInstance();
    return RunMeasureAndLayout();
}

RefPtr<LayoutWrapper> ScrollTestNg::CreateScroll(Axis axis, OnScrollStartEvent&& event)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetAxis(axis);
    scrollModel.SetOnScrollStart(std::move(event));
    scrollModel.SetOnScrollStop(std::move(event));
    CreateContent(axis);
    GetInstance();
    return RunMeasureAndLayout();
}

void ScrollTestNg::CreateContent(Axis axis)
{
    if (axis == Axis::HORIZONTAL) {
        RowModelNG rowModelNG;
        rowModelNG.Create(Dimension(0), nullptr, "");
        SetWidth(Dimension(COLUMN_WIDTH));
        SetHeight(FILL_LENGTH);
        for (int32_t index = 0; index < CHILD_NUMBER; index++) {
            RowModelNG rowModelNG;
            rowModelNG.Create(Dimension(0), nullptr, "");
            SetWidth(Dimension(COLUMN_CHILD_WIDTH));
            SetHeight(FILL_LENGTH);
            ViewStackProcessor::GetInstance()->Pop();
        }
    } else {
        ColumnModelNG columnModel;
        columnModel.Create(Dimension(0), nullptr, "");
        SetWidth(FILL_LENGTH);
        SetHeight(Dimension(COLUMN_HEIGHT));
        for (int32_t index = 0; index < CHILD_NUMBER; index++) {
            ColumnModelNG columnModel;
            columnModel.Create(Dimension(0), nullptr, "");
            SetWidth(FILL_LENGTH);
            SetHeight(Dimension(COLUMN_CHILD_HEIGHT));
            ViewStackProcessor::GetInstance()->Pop();
        }
    }
    ViewStackProcessor::GetInstance()->Pop();
}

RefPtr<LayoutWrapper> ScrollTestNg::RunMeasureAndLayout(float width, float height)
{
    RefPtr<LayoutWrapper> layoutWrapper = frameNode_->CreateLayoutWrapper(false, false);
    layoutWrapper->SetActive();
    layoutWrapper->SetRootMeasureNode();
    LayoutConstraintF LayoutConstraint;
    LayoutConstraint.parentIdealSize = { ROOT_WIDTH, ROOT_HEIGHT };
    LayoutConstraint.percentReference = { ROOT_WIDTH, ROOT_HEIGHT };
    LayoutConstraint.selfIdealSize = { width, height };
    LayoutConstraint.maxSize = { width, height };
    layoutWrapper->Measure(LayoutConstraint);
    layoutWrapper->Layout();
    layoutWrapper->MountToHostOnMainThread();
    return layoutWrapper;
}

void ScrollTestNg::UpdateCurrentOffset(float offset)
{
    pattern_->UpdateCurrentOffset(offset, SCROLL_FROM_UPDATE);
    RunMeasureAndLayout();
}

RefPtr<FrameNode> ScrollTestNg::GetColumnChild(int32_t index)
{
    auto column = AceType::DynamicCast<FrameNode>(frameNode_->GetChildAtIndex(0));
    auto columnChild = AceType::DynamicCast<FrameNode>(column->GetChildAtIndex(index));
    return columnChild;
}

testing::AssertionResult ScrollTestNg::IsEqualCurrentOffset(Offset expectOffset)
{
    RunMeasureAndLayout();
    Offset currentOffset = pattern_->GetCurrentOffset();
    if (expectOffset == currentOffset) {
        return testing::AssertionSuccess();
    }
    return testing::AssertionFailure() <<
        "GetCurrentOffset(): " << currentOffset.ToString() <<
        " But expect offset: " << expectOffset.ToString();
}

/**
 * @tc.name: AttrScrollable001
 * @tc.desc: Test attribute about scrollable,
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, AttrScrollable001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Text default value: Vertical
     */
    CreateScroll();
    const float delta = -100.f;
    UpdateCurrentOffset(delta);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, delta)));

    /**
     * @tc.steps: step2. Text set value: Horizontal
     */
    CreateScroll(Axis::HORIZONTAL);
    UpdateCurrentOffset(delta);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(delta, 0)));

    /**
     * @tc.steps: step3. Text set value: None
     */
    CreateScroll(Axis::NONE);
    UpdateCurrentOffset(delta);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, delta)));
}

/**
 * @tc.name: AttrScrollBar001
 * @tc.desc: Test attribute about scrollBar,
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, AttrScrollBar001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Text default value: Auto
     */
    CreateScroll();
    EXPECT_EQ(paintProperty_->GetBarStateString(), "BarState.Auto");

    /**
     * @tc.steps: step2. Text set value: Off
     */
    CreateScroll(NG::DisplayMode::OFF);
    EXPECT_EQ(paintProperty_->GetBarStateString(), "BarState.Off");

    /**
     * @tc.steps: step3. Text set value: On
     */
    CreateScroll(NG::DisplayMode::ON);
    EXPECT_EQ(paintProperty_->GetBarStateString(), "BarState.On");
}

/**
 * @tc.name: AttrScrollBarColorWidth001
 * @tc.desc: Test attribute about scrollBarColor/scrollBarWidth,
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, AttrScrollBarColorWidth001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Text default value:[color:foregroundColor_, width: 4]
     */
    CreateScroll();
    auto themeManager = MockPipelineBase::GetCurrent()->GetThemeManager();
    auto scrollBarTheme = themeManager->GetTheme<ScrollBarTheme>();
    EXPECT_EQ(paintProperty_->GetBarColor(), scrollBarTheme->GetForegroundColor());
    EXPECT_EQ(paintProperty_->GetBarWidth(), scrollBarTheme->GetNormalWidth());

    /**
     * @tc.steps: step2. Text set value: Color::RED
     */
    CreateScroll(Color::RED);
    EXPECT_EQ(paintProperty_->GetBarColor(), Color::RED);

    /**
     * @tc.steps: step3. Text set width value
     */
    CreateScroll(Dimension(10));
    EXPECT_EQ(paintProperty_->GetBarWidth(), Dimension(10));
}

/**
 * @tc.name: AttrEdgeEffect001
 * @tc.desc: Test attribute about edgeEffect,
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, AttrEdgeEffect001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Text default value: None
     */
    CreateScroll();
    EXPECT_EQ(layoutProperty_->GetEdgeEffectValue(), EdgeEffect::NONE);

    /**
     * @tc.steps: step2. Text set value: SPRING
     */
    CreateScroll(EdgeEffect::SPRING);
    EXPECT_EQ(layoutProperty_->GetEdgeEffectValue(), EdgeEffect::SPRING);

    /**
     * @tc.steps: step3. Text set value: SPRING
     */
    CreateScroll(EdgeEffect::FADE);
    EXPECT_EQ(layoutProperty_->GetEdgeEffectValue(), EdgeEffect::FADE);
}

/**
 * @tc.name: Event001
 * @tc.desc: Test RegisterScrollEventTask
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Event001, TestSize.Level1)
{
    auto event1 = [](Dimension, ScrollState) {
        ScrollFrameResult result;
        return result;
    };
    auto event2 = [](Dimension, Dimension) {
        ScrollInfo result;
        return result;
    };
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetOnScrollFrameBegin(event1);
    scrollModel.SetOnScrollBegin(event2);
    CreateContent();
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. When set event
     * @tc.expected: scrollableEvent would has event that setted
     */
    EXPECT_NE(eventHub_->GetScrollFrameBeginEvent(), nullptr);
    EXPECT_NE(eventHub_->GetScrollBeginEvent(), nullptr);
}

/**
 * @tc.name: Event002
 * @tc.desc: Test attribute about onScroll,
 * Event is triggered while scrolling
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Event002, TestSize.Level1)
{
    bool isTrigger = false;
    NG::ScrollEvent event = [&isTrigger](Dimension, Dimension) { isTrigger = true; };

    /**
     * @tc.steps: step1. Test event in VERTICAL
     */
    CreateScroll(Axis::VERTICAL, std::move(event));

    /**
     * @tc.steps: step2. Trigger event by OnScrollCallback
     * @tc.expected: isTrigger is true
     */
    pattern_->OnScrollCallback(-COLUMN_HEIGHT, SCROLL_FROM_UPDATE);
    EXPECT_TRUE(isTrigger);

    /**
     * @tc.steps: step3. Trigger event by ScrollToEdge
     * @tc.expected: isTrigger is true
     */
    isTrigger = false;
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, false);
    EXPECT_TRUE(isTrigger);

    /**
     * @tc.steps: step4. Test event in HORIZONTAL
     */
    isTrigger = false;
    CreateScroll(Axis::HORIZONTAL, std::move(event));

    /**
     * @tc.steps: step5. Trigger event by OnScrollCallback
     * @tc.expected: isTrigger is true
     */
    isTrigger = false;
    pattern_->OnScrollCallback(-COLUMN_WIDTH, SCROLL_FROM_UPDATE);
    EXPECT_TRUE(isTrigger);

    /**
     * @tc.steps: step6. Trigger event by ScrollToEdge
     * @tc.expected: isTrigger is true
     */
    isTrigger = false;
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, false);
    EXPECT_TRUE(isTrigger);
}

/**
 * @tc.name: Event003
 * @tc.desc: Test attribute about onScrollEdge,
 * Event is triggered while scroll to edge
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Event003, TestSize.Level1)
{
    bool isTrigger = false;
    NG::ScrollEdgeEvent event = [&isTrigger](ScrollEdge) { isTrigger = true; };

    /**
     * @tc.steps: step1. Test event in VERTICAL
     */
    CreateScroll(Axis::VERTICAL, std::move(event));

    /**
     * @tc.steps: step2. Trigger event by OnScrollCallback
     * @tc.expected: isTrigger is true
     */
    pattern_->OnScrollCallback(-COLUMN_HEIGHT, SCROLL_FROM_UPDATE);
    EXPECT_TRUE(isTrigger);

    /**
     * @tc.steps: step3. Trigger event by ScrollToEdge
     * @tc.expected: isTrigger is true
     */
    isTrigger = false;
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, false);
    EXPECT_TRUE(isTrigger);

    /**
     * @tc.steps: step4. Test event in HORIZONTAL
     */
    isTrigger = false;
    CreateScroll(Axis::HORIZONTAL, std::move(event));

    /**
     * @tc.steps: step5. Trigger event by OnScrollCallback
     * @tc.expected: isTrigger is true
     */
    pattern_->OnScrollCallback(-COLUMN_WIDTH, SCROLL_FROM_UPDATE);
    EXPECT_TRUE(isTrigger);

    /**
     * @tc.steps: step6. Trigger event by ScrollToEdge
     * @tc.expected: isTrigger is true
     */
    isTrigger = false;
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, false);
    EXPECT_TRUE(isTrigger);
}

/**
 * @tc.name: Event004
 * @tc.desc: Test attribute about onScrollStart and onScrollStop,
 * Event is triggered while scrolling start
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Event004, TestSize.Level1)
{
    bool isTrigger = false;
    OnScrollStartEvent event = [&isTrigger]() { isTrigger = true; };

    /**
     * @tc.steps: step1. Test onScrollStart event in VERTICAL
     */
    CreateScroll(Axis::VERTICAL, std::move(event));

    /**
     * @tc.steps: step2. Trigger onScrollStart event by OnScrollCallback
     * @tc.expected: isTrigger is true
     */
    pattern_->OnScrollCallback(-COLUMN_HEIGHT, SCROLL_FROM_START);
    EXPECT_TRUE(isTrigger);

    /**
     * @tc.steps: step3. Trigger onScrollStart event by ScrollToEdge
     * @tc.expected: isTrigger is true
     */
    isTrigger = false;
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, true);
    EXPECT_TRUE(isTrigger);

    /**
     * @tc.steps: step4. Trigger onScrollStop event by OnDirtyLayoutWrapperSwap
     * @tc.expected: isTrigger is true
     */
    isTrigger = false;
    pattern_->OnScrollEndCallback(); // set scrollStop_ = true;
    RunMeasureAndLayout();
    EXPECT_TRUE(isTrigger);
}

/**
 * @tc.name: ScrollPositionController001
 * @tc.desc: Test ScrollPositionController with Axis::VERTICAL
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollPositionController001, TestSize.Level1)
{
    CreateScroll();

    auto controller = pattern_->GetScrollPositionController();
    const Dimension position1(COLUMN_HEIGHT);
    const float duration1 = 1.f;
    bool animate = controller->AnimateTo(position1, duration1, Curves::LINEAR, false);
    EXPECT_TRUE(animate);

    const Dimension position2(1.0, DimensionUnit::PERCENT);
    const float duration2 = 1.f;
    animate = controller->AnimateTo(position2, duration2, Curves::LINEAR, false);
    EXPECT_FALSE(animate);

    controller->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    const Offset expectOffset1(0, ROOT_HEIGHT - COLUMN_HEIGHT);
    EXPECT_TRUE(IsEqualCurrentOffset(expectOffset1));

    controller->ScrollPage(false, false);
    const Offset expectOffset2(0, ROOT_HEIGHT - COLUMN_HEIGHT);
    EXPECT_TRUE(IsEqualCurrentOffset(expectOffset2));

    controller->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, false);
    EXPECT_FALSE(controller->IsAtEnd());
    const float delta = COLUMN_HEIGHT - ROOT_HEIGHT;
    UpdateCurrentOffset(-delta);
    EXPECT_TRUE(controller->IsAtEnd());
}

/**
 * @tc.name: ScrollPositionControlle002
 * @tc.desc: Test ScrollPositionController with Axis::NONE
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollPositionControlle002, TestSize.Level1)
{
    CreateScroll(Axis::NONE);

    auto controller = pattern_->GetScrollPositionController();
    const Dimension position(COLUMN_HEIGHT);
    const float duration = 1.f;
    bool animate = controller->AnimateTo(position, duration, Curves::LINEAR, false);
    EXPECT_FALSE(animate);

    controller->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    const Offset expectOffset1(0, 0);
    EXPECT_TRUE(IsEqualCurrentOffset(expectOffset1));

    controller->ScrollPage(false, false);
    const Offset expectOffset2(0, 0);
    EXPECT_TRUE(IsEqualCurrentOffset(expectOffset2));
}

/**
 * @tc.name: PaintMethod001
 * @tc.desc: Test PaintMethod
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, PaintMethod001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Do not SetScrollBarWidth
     * @tc.expected: scrollBar->NeedPaint() is false
     */
    CreateScroll();
    auto paint_1 = pattern_->CreateNodePaintMethod();
    auto scrollPaint_1 = AceType::DynamicCast<ScrollPaintMethod>(paint_1);
    RSCanvas canvas_1;
    RefPtr<GeometryNode> geometryNode_1 = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper_1(nullptr, geometryNode_1, paintProperty_);
    scrollPaint_1->PaintScrollBar(canvas_1, &paintWrapper_1);
    EXPECT_FALSE(scrollPaint_1->scrollBar_.Upgrade()->NeedPaint());

    /**
     * @tc.steps: step1. SetScrollBarWidth
     * @tc.expected: scrollBar->NeedPaint() is true
     */
    CreateScroll(Dimension(10.f));
    auto paint_2 = pattern_->CreateNodePaintMethod();
    auto scrollPaint_2 = AceType::DynamicCast<ScrollPaintMethod>(paint_2);
    RSCanvas canvas_2;
    RefPtr<GeometryNode> geometryNode_2 = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper_2(nullptr, geometryNode_2, paintProperty_);
    scrollPaint_2->PaintScrollBar(canvas_2, &paintWrapper_2);
    EXPECT_TRUE(scrollPaint_2->scrollBar_.Upgrade()->NeedPaint());
}

/**
 * @tc.name: SpringEffect001
 * @tc.desc: Test SpringEffect
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, SpringEffect001, TestSize.Level1)
{
    auto springEffect = AceType::MakeRefPtr<ScrollSpringEffect>();
    springEffect->ProcessScrollOver(0.0);

    CreateScroll(EdgeEffect::SPRING);
    springEffect = AceType::DynamicCast<ScrollSpringEffect>(pattern_->GetScrollEdgeEffect());
    auto scrollable = AceType::MakeRefPtr<Scrollable>();
    springEffect->SetScrollable(scrollable);
    springEffect->ProcessScrollOver(0.0);

    scrollable->MarkAvailable(false);
    springEffect->ProcessScrollOver(0.0);

    pattern_->SetDirection(FlexDirection::ROW_REVERSE);
    pattern_->SetEdgeEffect(EdgeEffect::SPRING);
    springEffect->ProcessScrollOver(0.0);

    EXPECT_TRUE(true);
}

/**
 * @tc.name: ScrollTest001
 * @tc.desc: Test OnDirtyLayoutWrapperSwap
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollTest001, TestSize.Level1)
{
    auto layoutWrapper = CreateScroll();
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = false;
    auto dirty = pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_FALSE(dirty);

    config.skipMeasure = true;
    config.skipLayout = true;
    dirty = pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_FALSE(dirty);

    config.skipMeasure = false;
    config.skipLayout = false;
    dirty = pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_FALSE(dirty);
}

/**
 * @tc.name: ScrollTest002
 * @tc.desc: When setting a fixed length and width, verify the related functions in the scroll pattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollTest002, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetAxis(Axis::HORIZONTAL);
    scrollModel.SetDisplayMode(static_cast<int>(NG::DisplayMode::OFF));
    auto scrollProxy = scrollModel.CreateScrollBarProxy();
    scrollModel.SetScrollBarProxy(scrollProxy);
    CreateContent();
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step5. When Axis is HORIZONTAL, Verify the callback function registered in scrollBarProxy.
     * @tc.expected: step5. Check whether the return value is as expected.
     */
    auto scrollBarProxy = pattern_->GetScrollBarProxy();
    EXPECT_FALSE(scrollBarProxy->scrollableNodes_.empty());
    bool ret = scrollBarProxy->scrollableNodes_.back().onPositionChanged(0.0, SCROLL_FROM_BAR);
    EXPECT_TRUE(ret);
    ret = scrollBarProxy->scrollableNodes_.back().onPositionChanged(0.0, SCROLL_FROM_START);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: ScrollTest003
 * @tc.desc: When setting a fixed length and width, verify the related callback functions in the scroll pattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollTest003, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetAxis(Axis::VERTICAL);
    scrollModel.SetDisplayMode(1);
    scrollModel.SetScrollBarWidth(Dimension(20.1));
    scrollModel.SetScrollBarColor(Color::FromRGB(255, 100, 100));
    CreateContent();
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step5. Set ScrollSpringEffect and call relevant callback functions.
     * @tc.expected: step5. Check whether the return value is correct.
     */
    pattern_->SetEdgeEffect(EdgeEffect::SPRING);
    RefPtr<ScrollEdgeEffect> scrollEdgeEffect = pattern_->GetScrollEdgeEffect();
    EXPECT_NE(scrollEdgeEffect, nullptr);
    auto springEffect = AceType::DynamicCast<ScrollSpringEffect>(scrollEdgeEffect);
    EXPECT_NE(springEffect, nullptr);
    pattern_->currentOffset_ = 100.f;
    pattern_->scrollableDistance_ = 100.f;
    auto isOutBoundary = springEffect->outBoundaryCallback_();
    EXPECT_TRUE(isOutBoundary);
    auto currentPosition = scrollEdgeEffect->currentPositionCallback_();
    EXPECT_EQ(currentPosition, 100.0);

    /**
     * @tc.steps: step6. When direction is the default value, call the relevant callback function.
     * @tc.expected: step6. Check whether the return value is correct.
     */
    auto leading = scrollEdgeEffect->leadingCallback_();
    EXPECT_EQ(leading, -100.f);
    auto trailing = scrollEdgeEffect->trailingCallback_();
    EXPECT_EQ(trailing, 0.0);
    auto initLeading = scrollEdgeEffect->initLeadingCallback_();
    EXPECT_EQ(initLeading, -100.f);
    auto initTrailing = scrollEdgeEffect->initTrailingCallback_();
    EXPECT_EQ(initTrailing, 0.0);

    /**
     * @tc.steps: step7. When direction is ROW_REVERSE, call the relevant callback function.
     * @tc.expected: step7. Check whether the return value is correct.
     */
    pattern_->direction_ = FlexDirection::ROW_REVERSE;
    leading = scrollEdgeEffect->leadingCallback_();
    EXPECT_EQ(leading, 0.0);
    trailing = scrollEdgeEffect->trailingCallback_();
    EXPECT_EQ(trailing, 100.f);
    initLeading = scrollEdgeEffect->initLeadingCallback_();
    EXPECT_EQ(initLeading, 0.0);
    initTrailing = scrollEdgeEffect->initTrailingCallback_();
    EXPECT_EQ(initTrailing, 100.f);

    /**
     * @tc.steps: step8. When direction is COLUMN_REVERSE, call the relevant callback function.
     * @tc.expected: step8. Check whether the return value is correct.
     */
    pattern_->direction_ = FlexDirection::COLUMN_REVERSE;
    leading = scrollEdgeEffect->leadingCallback_();
    EXPECT_EQ(leading, 0.0);
    trailing = scrollEdgeEffect->trailingCallback_();
    EXPECT_EQ(trailing, 100.f);
    initLeading = scrollEdgeEffect->initLeadingCallback_();
    EXPECT_EQ(initLeading, 0.0);
    initTrailing = scrollEdgeEffect->initTrailingCallback_();
    EXPECT_EQ(initTrailing, 100.f);
}

/**
 * @tc.name: ScrollTest004
 * @tc.desc: When setting a fixed length and width, verify the related functions in the scroll pattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollTest004, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetAxis(Axis::VERTICAL);
    scrollModel.SetDisplayMode(1);
    scrollModel.SetScrollBarWidth(Dimension(20.1));
    scrollModel.SetScrollBarColor(Color::FromRGB(255, 100, 100));
    CreateContent();
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step6. Set ScrollFadeEffect and call relevant callback functions.
     * @tc.expected: step6. Check whether the return value is correct.
     */
    pattern_->SetEdgeEffect(EdgeEffect::FADE);
    RefPtr<ScrollEdgeEffect> scrollEdgeEffect = pattern_->GetScrollEdgeEffect();
    EXPECT_NE(scrollEdgeEffect, nullptr);
    pattern_->currentOffset_ = 100.f;
    pattern_->scrollableDistance_ = 100.f;
    auto scrollFade = AceType::DynamicCast<ScrollFadeEffect>(scrollEdgeEffect);
    EXPECT_NE(scrollFade, nullptr);
    scrollFade->handleOverScrollCallback_();
    EXPECT_NE(scrollFade->fadeController_, nullptr);
    pattern_->SetEdgeEffect(EdgeEffect::NONE);
    EXPECT_EQ(pattern_->scrollEffect_, nullptr);
}

/**
 * @tc.name: ScrollTest005
 * @tc.desc: Scroll Accessibility PerformAction test ScrollForward and ScrollBackward..
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scroll and initialize related properties.
     */
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetAxis(Axis::NONE);

    /**
     * @tc.steps: step2. Get scroll frameNode and pattern, set callback function.
     * @tc.expected: Related function is called.
     */
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto scrollPattern = frameNode->GetPattern<ScrollPattern>();
    ASSERT_NE(scrollPattern, nullptr);
    scrollPattern->scrollableDistance_ = 0.0;
    scrollPattern->SetAccessibilityAction();

    /**
     * @tc.steps: step3. Get scroll accessibilityProperty to call callback function.
     * @tc.expected: Related function is called.
     */
    auto scrollAccessibilityProperty = frameNode->GetAccessibilityProperty<ScrollAccessibilityProperty>();
    ASSERT_NE(scrollAccessibilityProperty, nullptr);

    /**
     * @tc.steps: step4. When scroll is not scrollable and scrollable distance is 0, call the callback function in
     *                   scrollAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    EXPECT_TRUE(scrollAccessibilityProperty->ActActionScrollForward());
    EXPECT_TRUE(scrollAccessibilityProperty->ActActionScrollBackward());

    /**
     * @tc.steps: step5. When scroll is not scrollable and scrollable distance is not 0, call the callback function in
     *                   scrollAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    scrollPattern->scrollableDistance_ = 100.f;
    EXPECT_TRUE(scrollAccessibilityProperty->ActActionScrollForward());
    EXPECT_TRUE(scrollAccessibilityProperty->ActActionScrollBackward());

    /**
     * @tc.steps: step6. When scroll is scrollable and scrollable distance is not 0, call the callback function in
     *                   scrollAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    scrollPattern->SetAxis(Axis::VERTICAL);
    EXPECT_TRUE(scrollAccessibilityProperty->ActActionScrollForward());
    EXPECT_TRUE(scrollAccessibilityProperty->ActActionScrollBackward());

    /**
     * @tc.steps: step7. When scroll is scrollable and scrollable distance is 0, call the callback function in
     *                   scrollAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    scrollPattern->scrollableDistance_ = 0.0;
    EXPECT_TRUE(scrollAccessibilityProperty->ActActionScrollForward());
    EXPECT_TRUE(scrollAccessibilityProperty->ActActionScrollBackward());
}

/**
 * @tc.name: UpdateCurrentOffset001
 * @tc.desc: Test UpdateCurrentOffset
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, UpdateCurrentOffset001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. When unscrollable
     * @tc.expected: currentOffset would not change
     */
    ScrollModelNG scrollModel;
    scrollModel.Create();
    GetInstance();
    RunMeasureAndLayout();
    pattern_->UpdateCurrentOffset(10.f, SCROLL_FROM_UPDATE);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset::Zero()));

    /**
     * @tc.steps: step1. When Axis::VERTICAL
     */
    CreateScroll();
    pattern_->UpdateCurrentOffset(-10.f, SCROLL_FROM_JUMP);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, -10.f)));
    pattern_->UpdateCurrentOffset(-10.f, SCROLL_FROM_BAR);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, -20.f)));
    pattern_->UpdateCurrentOffset(-10.f, SCROLL_FROM_ROTATE);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, -30.f)));
    pattern_->UpdateCurrentOffset(-10.f, SCROLL_FROM_ANIMATION);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, -40.f)));
    pattern_->UpdateCurrentOffset(-10.f, SCROLL_FROM_ANIMATION_SPRING);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, -50.f)));

    /**
     * @tc.steps: step1. When Axis::HORIZONTAL
     */
    CreateScroll(Axis::HORIZONTAL);
    pattern_->UpdateCurrentOffset(-10.f, SCROLL_FROM_JUMP);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(-10.f, 0)));
    pattern_->UpdateCurrentOffset(-10.f, SCROLL_FROM_BAR);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(-20.f, 0)));
    pattern_->UpdateCurrentOffset(-10.f, SCROLL_FROM_ROTATE);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(-30.f, 0)));
    pattern_->UpdateCurrentOffset(-10.f, SCROLL_FROM_ANIMATION);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(-40.f, 0)));
    pattern_->UpdateCurrentOffset(-10.f, SCROLL_FROM_ANIMATION_SPRING);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(-50.f, 0)));

    /**
     * @tc.steps: step1. When EdgeEffect::SPRING, Test ValidateOffset
     * @tc.expected: will trigger SetScrollBarOutBoundaryExtent
     */
    CreateScroll(EdgeEffect::SPRING);
    EXPECT_FALSE(pattern_->IsRestrictBoundary());
    pattern_->currentOffset_ = 10.f;
    pattern_->UpdateCurrentOffset(-5.f, SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetScrollBarOutBoundaryExtent(), 5.f);
    pattern_->currentOffset_ = -1000.f;
    pattern_->UpdateCurrentOffset(-10.f, SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->GetScrollBarOutBoundaryExtent(), -pattern_->currentOffset_ - (COLUMN_HEIGHT - ROOT_HEIGHT));
}

/**
 * @tc.name: ScrollFadeEffect001
 * @tc.desc: Test the correlation function in ScrollFadeEffect under different conditions.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollFadeEffect001, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetAxis(Axis::HORIZONTAL);
    scrollModel.SetDisplayMode(2);
    scrollModel.SetScrollBarWidth(Dimension(20.1));
    scrollModel.SetScrollBarColor(Color::FromRGB(255, 100, 100));
    CreateContent();
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step5. Create ScrollFadeEffect and  verify related callback functions.
     * @tc.expected: step5. Check whether relevant parameters are correct.
     */
    pattern_->SetEdgeEffect(EdgeEffect::FADE);
    RefPtr<ScrollEdgeEffect> scrollEdgeEffect = pattern_->GetScrollEdgeEffect();
    EXPECT_NE(scrollEdgeEffect, nullptr);
    auto scrollable = AceType::MakeRefPtr<Scrollable>();
    EXPECT_NE(scrollable, nullptr);
    scrollable->controller_ = CREATE_ANIMATOR();
    scrollable->springController_ = CREATE_ANIMATOR();
    scrollEdgeEffect->SetScrollable(scrollable);
    auto scrollFadeEffect = AceType::DynamicCast<ScrollFadeEffect>(scrollEdgeEffect);
    scrollEdgeEffect->InitialEdgeEffect();
    EXPECT_NE(scrollFadeEffect->scrollable_, nullptr);
    EXPECT_NE(scrollFadeEffect, nullptr);
    EXPECT_NE(scrollFadeEffect->fadeController_, nullptr);
    EXPECT_NE(scrollFadeEffect->fadePainter_, nullptr);
    EXPECT_EQ(scrollFadeEffect->fadeColor_, Color::GRAY);
    scrollFadeEffect->fadeController_->callback_(1.0f, 1.0f);
    EXPECT_EQ(scrollFadeEffect->fadePainter_->opacity_, 1.0f);
    EXPECT_EQ(scrollFadeEffect->fadePainter_->scaleFactor_, 1.0f);

    /**
     * @tc.steps: step6. Verify the HandleOverScroll function in ScrollFadeEffect under different parameters.
     * @tc.expected: step6. Check whether relevant parameters are correct.
     */
    scrollFadeEffect->HandleOverScroll(Axis::VERTICAL, 0.0f, SizeF(1.f, 1.f));
    EXPECT_EQ(scrollFadeEffect->fadePainter_->direction_, OverScrollDirection::UP);
    scrollFadeEffect->HandleOverScroll(Axis::VERTICAL, -1.0f, SizeF(1.f, 1.f));
    EXPECT_EQ(scrollFadeEffect->fadePainter_->direction_, OverScrollDirection::UP);
    scrollFadeEffect->HandleOverScroll(Axis::VERTICAL, 1.0f, SizeF(1.f, 1.f));
    EXPECT_EQ(scrollFadeEffect->fadePainter_->direction_, OverScrollDirection::DOWN);
    scrollFadeEffect->HandleOverScroll(Axis::HORIZONTAL, -1.0f, SizeF(1.f, 1.f));
    EXPECT_EQ(scrollFadeEffect->fadePainter_->direction_, OverScrollDirection::LEFT);
    scrollFadeEffect->HandleOverScroll(Axis::HORIZONTAL, 1.0f, SizeF(1.f, 1.f));
    EXPECT_EQ(scrollFadeEffect->fadePainter_->direction_, OverScrollDirection::RIGHT);
    scrollFadeEffect->fadeController_ = nullptr;
    scrollFadeEffect->scrollable_->currentVelocity_ = 1.0;
    scrollFadeEffect->HandleOverScroll(Axis::VERTICAL, 1.0f, SizeF(1.f, 1.f));
    EXPECT_NE(scrollFadeEffect->fadeController_, nullptr);
    scrollFadeEffect->SetPaintDirection(Axis::HORIZONTAL, 0.0f);
    EXPECT_EQ(scrollFadeEffect->fadePainter_->direction_, OverScrollDirection::DOWN);

    /**
     * @tc.steps: step7. Verify the CalculateOverScroll function in ScrollFadeEffect under different parameters.
     * @tc.expected: step7. Check whether the return value is correct.
     */
    pattern_->currentOffset_ = 100.f;
    pattern_->scrollableDistance_ = -1.0f;
    auto ret = scrollFadeEffect->CalculateOverScroll(-1.0, true);
    EXPECT_EQ(ret, -99.0);
    pattern_->currentOffset_ = -100.f;
    pattern_->scrollableDistance_ = -1.0f;
    ret = scrollFadeEffect->CalculateOverScroll(1.0, true);
    EXPECT_EQ(ret, 99.0);
    pattern_->currentOffset_ = 100.f;
    pattern_->scrollableDistance_ = -1.0f;
    ret = scrollFadeEffect->CalculateOverScroll(1.0, true);
    EXPECT_EQ(ret, -100.0);
    pattern_->currentOffset_ = -100.f;
    pattern_->scrollableDistance_ = 1.0f;
    ret = scrollFadeEffect->CalculateOverScroll(-1.0, true);
    EXPECT_EQ(ret, 99.0);
    pattern_->currentOffset_ = -100.f;
    pattern_->scrollableDistance_ = 1.0f;
    ret = scrollFadeEffect->CalculateOverScroll(-1.0, false);
    EXPECT_EQ(ret, 0.0);
}

/**
 * @tc.name: FadeController001
 * @tc.desc: Test scroll_fade_controller
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, FadeController001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create ScrollFadeController and set callback function.
     */
    auto fadeController = AceType::MakeRefPtr<ScrollFadeController>();
    EXPECT_NE(fadeController, nullptr);
    double param1 = 10.f;
    double param2 = -10.0;
    auto callback = [&param1, &param2](double parameter1, double parameter2) {
        param1 = parameter1;
        param2 = parameter2;
    };
    fadeController->SetCallback(callback);

    /**
     * @tc.steps: step2. Verify the ProcessAbsorb function and callback function in fadeController.
     * @tc.expected: step2. Check whether relevant parameters are correct.
     */
    fadeController->ProcessAbsorb(100.0);
    EXPECT_EQ(fadeController->opacityFloor_, 0.3);
    EXPECT_EQ(fadeController->opacityCeil_, 0.3);
    EXPECT_EQ(fadeController->scaleSizeFloor_, 0.0);
    EXPECT_EQ(fadeController->scaleSizeCeil_, 0.0325);
    EXPECT_EQ(fadeController->state_, OverScrollState::ABSORB);
    fadeController->decele_->NotifyListener(100.0);
    EXPECT_EQ(fadeController->opacity_, 0.3);
    EXPECT_EQ(fadeController->scaleSize_, 3.25);
    EXPECT_EQ(param1, fadeController->opacity_);
    EXPECT_EQ(param2, fadeController->scaleSize_);

    /**
     * @tc.steps: step2. When OverScrollState is ABSORB, call the callback function in fadeController.
     * @tc.expected: step2. Check whether relevant parameters are correct.
     */
    fadeController->controller_->NotifyStopListener();
    EXPECT_EQ(fadeController->opacityCeil_, 0.0);
    EXPECT_EQ(fadeController->scaleSizeCeil_, 0.0);
    EXPECT_EQ(fadeController->state_, OverScrollState::RECEDE);

    /**
     * @tc.steps: step3. When OverScrollState is RECEDE, call the ProcessRecede function and callback function in
     *                   fadeController.
     * @tc.expected: step3. Check whether relevant parameters are correct.
     */
    fadeController->ProcessRecede(1000);
    fadeController->controller_->NotifyStopListener();
    EXPECT_EQ(fadeController->state_, OverScrollState::IDLE);
    EXPECT_EQ(fadeController->pullDistance_, 0.0);
    fadeController->ProcessRecede(1000);
    EXPECT_EQ(fadeController->pullDistance_, 0.0);

    /**
     * @tc.steps: step4. When OverScrollState is IDLE, call the ProcessPull function and callback function in
     *                   fadeController.
     * @tc.expected: step4. Check whether relevant parameters are correct.
     */
    fadeController->ProcessPull(1.0, 1.0, 1.0);
    EXPECT_EQ(fadeController->opacityFloor_, 0.3);
    EXPECT_EQ(fadeController->opacityCeil_, 0.5);
    EXPECT_EQ(fadeController->scaleSizeFloor_, 3.25);
    EXPECT_EQ(fadeController->scaleSizeCeil_, 3.25);
    EXPECT_EQ(fadeController->state_, OverScrollState::PULL);

    /**
     * @tc.steps: step5. When OverScrollState is PULL, call the ProcessAbsorb function and callback function in
     *                   fadeController.
     * @tc.expected: step5. Check whether relevant parameters are correct.
     */
    fadeController->ProcessAbsorb(-10.0);
    fadeController->decele_->NotifyListener(100.0);
    EXPECT_EQ(fadeController->opacity_, 20.3);
    EXPECT_EQ(fadeController->scaleSize_, 3.25);
    fadeController->controller_->NotifyStopListener();
    EXPECT_EQ(fadeController->state_, OverScrollState::PULL);
    fadeController->ProcessAbsorb(100.0);
    fadeController->ProcessPull(1.0, 1.0, 1.0);
    fadeController->decele_->NotifyListener(100.0);
    EXPECT_EQ(param1, 20.3);
    EXPECT_EQ(param2, 3.25);
}

/**
 * @tc.name: ScrollBar001
 * @tc.desc: Test UpdateScrollBarRegion function in ScrollBar under different conditions.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollBar001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. When the ShapeMode is RECT and DisplayMode is LEFT, verify the UpdateScrollBarRegion function.
     * @tc.expected: step1. Check whether relevant parameters are correct.
     */
    auto scrollBar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    EXPECT_NE(scrollBar, nullptr);
    scrollBar->shapeMode_ = ShapeMode::RECT;
    scrollBar->positionMode_ = PositionMode::LEFT;
    scrollBar->UpdateScrollBarRegion(Offset::Zero(), Size(100.0, 100.0), Offset(1.f, 1.f), 1.0);
    auto barRect = Rect(0.0, 0.0, 0.0, 100.0) + Offset::Zero();
    EXPECT_EQ(scrollBar->barRect_, barRect);
    EXPECT_EQ(scrollBar->activeRect_, Rect(0.0, -9900.0, 0.0, 10000.0));
    EXPECT_EQ(scrollBar->touchRegion_, Rect(0.0, -9900.0, 0.0, 10000.0));

    /**
     * @tc.steps: step2. When the ShapeMode is RECT and DisplayMode is BOTTOM, verify the UpdateScrollBarRegion
     *            function.
     * @tc.expected: step2. Check whether relevant parameters are correct.
     */
    scrollBar->positionModeUpdate_ = true;
    scrollBar->positionMode_ = PositionMode::BOTTOM;
    scrollBar->SetOutBoundary(1.0);
    scrollBar->UpdateScrollBarRegion(Offset::Zero(), Size(100.0, 100.0), Offset(1.f, 1.f), 1.0);
    barRect = Rect(0.0, 100.0, 100.0, 0.0) + Offset::Zero();
    EXPECT_EQ(scrollBar->barRect_, barRect);
    EXPECT_EQ(scrollBar->activeRect_, Rect(-9899.0, 100.0, 9999.0, 0.0));
    EXPECT_EQ(scrollBar->touchRegion_, Rect(-9899.0, 100.0, 9999.0, 0.0));

    /**
     * @tc.steps: step3. When the ShapeMode is RECT and DisplayMode is RIGHT, verify the UpdateScrollBarRegion function.
     * @tc.expected: step3. Check whether relevant parameters are correct.
     */
    scrollBar->positionModeUpdate_ = true;
    scrollBar->positionMode_ = PositionMode::RIGHT;
    scrollBar->UpdateScrollBarRegion(Offset::Zero(), Size(100.0, 100.0), Offset(1.f, 1.f), 1.0);
    barRect = Rect(100.0, 0.0, 0.0, 100.0) + Offset::Zero();
    EXPECT_EQ(scrollBar->barRect_, barRect);
    EXPECT_EQ(scrollBar->activeRect_, Rect(100.0, -9899.0, 0.0, 9999.0));
    EXPECT_EQ(scrollBar->touchRegion_, Rect(100.0, -9899.0, 0.0, 9999.0));

    /**
     * @tc.steps: step4. When the ShapeMode is ROUND and DisplayMode is LEFT, verify the UpdateScrollBarRegion function.
     * @tc.expected: step4. Check whether relevant parameters are correct.
     */
    scrollBar->positionModeUpdate_ = true;
    scrollBar->shapeMode_ = ShapeMode::ROUND;
    scrollBar->positionMode_ = PositionMode::LEFT;
    scrollBar->UpdateScrollBarRegion(Offset::Zero(), Size(100.0, 100.0), Offset(1.f, 1.f), 1.0);
    EXPECT_EQ(scrollBar->trickStartAngle_, 150);
    EXPECT_EQ(scrollBar->trickSweepAngle_, -6000);

    /**
     * @tc.steps: step5. When the ShapeMode is ROUND and DisplayMode is RIGHT, verify the UpdateScrollBarRegion
     *                   function.
     * @tc.expected: step5. Check whether relevant parameters are correct.
     */
    scrollBar->positionModeUpdate_ = true;
    scrollBar->positionMode_ = PositionMode::RIGHT;
    scrollBar->UpdateScrollBarRegion(Offset::Zero(), Size(100.0, 100.0), Offset(1.f, 1.f), 1.0);
    EXPECT_EQ(scrollBar->trickStartAngle_, 30);
    EXPECT_EQ(scrollBar->trickSweepAngle_, 6000);

    /**
     * @tc.steps: step6. When the ShapeMode is ROUND and DisplayMode is LEFT, verify the UpdateScrollBarRegion function.
     * @tc.expected: step6. Check whether relevant parameters are correct.
     */
    scrollBar->positionModeUpdate_ = true;
    scrollBar->positionMode_ = PositionMode::LEFT;
    scrollBar->bottomAngle_ = 50.f;
    scrollBar->topAngle_ = 100.0;
    scrollBar->SetOutBoundary(1.0);
    scrollBar->UpdateScrollBarRegion(Offset::Zero(), Size(100.0, 100.0), Offset(1.f, 1.f), 200.0);
    EXPECT_EQ(scrollBar->trickStartAngle_, -155);
    EXPECT_EQ(scrollBar->trickSweepAngle_, -10);
}

/**
 * @tc.name: Measure001
 * @tc.desc: Test Measure
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Measure001, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetAxis(Axis::NONE);
    CreateContent();
    GetInstance();

    /**
     * @tc.steps: step1. Do not set idealSize
     * @tc.expected: The idealSize would be child size
     */
    RefPtr<LayoutWrapper> layoutWrapper = frameNode_->CreateLayoutWrapper(false, false);
    layoutWrapper->SetActive();
    layoutWrapper->SetRootMeasureNode();
    LayoutConstraintF LayoutConstraint;
    LayoutConstraint.parentIdealSize = { ROOT_WIDTH, ROOT_HEIGHT };
    LayoutConstraint.percentReference = { ROOT_WIDTH, ROOT_HEIGHT };
    layoutWrapper->Measure(LayoutConstraint);
    layoutWrapper->Layout();
    layoutWrapper->MountToHostOnMainThread();
    auto scrollSize = frameNode_->GetGeometryNode()->GetFrameSize();
    auto expectSize = SizeF(ROOT_WIDTH, COLUMN_HEIGHT);
    EXPECT_EQ(scrollSize, expectSize) <<
        "scrollSize: " << scrollSize.ToString() <<
        " expectSize: " << expectSize.ToString();
}

/**
 * @tc.name: Layout001
 * @tc.desc: Test Layout
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Layout001, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetAxis(Axis::NONE);
    CreateContent();
    GetInstance();
    RunMeasureAndLayout();

    layoutProperty_->UpdateAlignment(Alignment::CENTER);
    RunMeasureAndLayout();
    auto col = frameNode_->GetChildAtIndex(0);
    auto colNode = AceType::DynamicCast<FrameNode>(col);
    auto colOffset = colNode->GetGeometryNode()->GetMarginFrameOffset();
    auto expectOffset = OffsetF(0, 0);
    EXPECT_EQ(colOffset, expectOffset) <<
        "colOffset: " << colOffset.ToString() <<
        " expectOffset: " << expectOffset.ToString();
}

/**
 * @tc.name: OnScrollCallback001
 * @tc.desc: Test OnScrollCallback
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, OnScrollCallback001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Axis::NONE and SCROLL_FROM_UPDATE
     * @tc.expected: Do nothing
     */
    CreateScroll(Axis::NONE);
    EXPECT_FALSE(pattern_->OnScrollCallback(-100.f, SCROLL_FROM_UPDATE));

    /**
     * @tc.steps: step2. animator is running and SCROLL_FROM_UPDATE
     * @tc.expected: Do nothing
     */
    CreateScroll();
    pattern_->CreateOrStopAnimator();
    pattern_->animator_->Resume();
    EXPECT_FALSE(pattern_->OnScrollCallback(-100.f, SCROLL_FROM_UPDATE));

    /**
     * @tc.steps: step3. animator is stopped and SCROLL_FROM_UPDATE
     * @tc.expected: Trigger UpdateCurrentOffset()
     */
    CreateScroll();
    pattern_->CreateOrStopAnimator();
    pattern_->animator_->Stop();
    EXPECT_TRUE(pattern_->OnScrollCallback(-100.f, SCROLL_FROM_UPDATE));
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, -100.f)));

    /**
     * @tc.steps: step4. scrollBar->IsDriving() is true and SCROLL_FROM_UPDATE
     * @tc.expected: Trigger UpdateCurrentOffset()
     */
    CreateScroll();
    pattern_->GetScrollBar()->SetDriving(true);
    EXPECT_TRUE(pattern_->OnScrollCallback(100.f, SCROLL_FROM_UPDATE));
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, -100.f / pattern_->GetScrollBar()->offsetScale_)));

    /**
     * @tc.steps: step5. no animator and SCROLL_FROM_START
     * @tc.expected: Trigger FireOnScrollStart()
     */
    bool isTrigger = false;
    OnScrollStartEvent event = [&isTrigger]() { isTrigger = true; };
    CreateScroll(Axis::VERTICAL, std::move(event));
    EXPECT_TRUE(pattern_->OnScrollCallback(-100.f, SCROLL_FROM_START));
    EXPECT_TRUE(isTrigger);

    /**
     * @tc.steps: step6. animator is Stopped and SCROLL_FROM_START
     * @tc.expected: Trigger FireOnScrollStart()
     */
    isTrigger = false;
    CreateScroll(Axis::VERTICAL, std::move(event));
    pattern_->CreateOrStopAnimator();
    pattern_->animator_->Stop();
    EXPECT_TRUE(pattern_->OnScrollCallback(-100.f, SCROLL_FROM_START));
    EXPECT_TRUE(isTrigger);

    /**
     * @tc.steps: step7. animator is running and SCROLL_FROM_START
     * @tc.expected: because of scrollAbort_ is true, would not trigger event, and animator stop()
     */
    isTrigger = false;
    CreateScroll(Axis::VERTICAL, std::move(event));
    pattern_->CreateOrStopAnimator();
    pattern_->animator_->Resume();
    EXPECT_TRUE(pattern_->OnScrollCallback(-100.f, SCROLL_FROM_START));
    EXPECT_TRUE(pattern_->animator_->IsStopped());
    EXPECT_FALSE(isTrigger);
}

/**
 * @tc.name: ScrollToNode001
 * @tc.desc: Test ScrollToNode
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, ScrollToNode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. ScrollToNode in VERTICAL
     * @tc.expected: currentOffset_ is correct
     */
    CreateScroll();
    pattern_->ScrollToNode(GetColumnChild(5));
    EXPECT_TRUE(IsEqualCurrentOffset(Offset::Zero()));
    pattern_->ScrollToNode(GetColumnChild(10));
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, -COLUMN_CHILD_HEIGHT)));
    pattern_->ScrollToNode(GetColumnChild(11));
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, -COLUMN_CHILD_HEIGHT * 2)));
    pattern_->ScrollToNode(GetColumnChild(0));
    EXPECT_TRUE(IsEqualCurrentOffset(Offset::Zero()));

    /**
     * @tc.steps: step2. ScrollToNode in HORIZONTAL
     * @tc.expected: currentOffset_ is correct
     */
    CreateScroll(Axis::HORIZONTAL);
    pattern_->ScrollToNode(GetColumnChild(5));
    EXPECT_TRUE(IsEqualCurrentOffset(Offset::Zero()));
    pattern_->ScrollToNode(GetColumnChild(10));
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(-COLUMN_CHILD_WIDTH, 0)));
    pattern_->ScrollToNode(GetColumnChild(11));
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(-COLUMN_CHILD_WIDTH * 2, 0)));
    pattern_->ScrollToNode(GetColumnChild(0));
    EXPECT_TRUE(IsEqualCurrentOffset(Offset::Zero()));
}

/**
 * @tc.name: Pattern001
 * @tc.desc: Test OnModifyDone
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Pattern001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Trigger OnModifyDone
     * @tc.expected: Has ScrollableEvent, has AccessibilityAction
     */
    CreateScroll();
    EXPECT_NE(pattern_->GetScrollableEvent(), nullptr);
    EXPECT_NE(accessibilityProperty_->actionScrollForwardImpl_, nullptr);
    EXPECT_NE(accessibilityProperty_->actionScrollBackwardImpl_, nullptr);

    /**
     * @tc.steps: step1. Change axis and trigger OnModifyDone
     * @tc.expected: Axis would be changed and Would not RegisterScrollEventTask again
     */
    layoutProperty_->UpdateAxis(Axis::HORIZONTAL);
    pattern_->OnModifyDone();
    EXPECT_EQ(pattern_->GetAxis(), Axis::HORIZONTAL);
    EXPECT_NE(pattern_->GetScrollableEvent(), nullptr);
}

/**
 * @tc.name: Pattern002
 * @tc.desc: Test SetAccessibilityAction
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Pattern002, TestSize.Level1)
{
    CreateScroll();

    /**
     * @tc.steps: step1. Trigger actionScrollForwardImpl_
     * @tc.expected: ScrollPage forward, would trigger AnimateTo()
     */
    accessibilityProperty_->actionScrollForwardImpl_();
    EXPECT_NE(pattern_->animator_, nullptr);

    /**
     * @tc.steps: step2. Trigger actionScrollBackwardImpl_
     * @tc.expected: ScrollPage backward, would trigger AnimateTo()
     */
    pattern_->animator_ = nullptr;
    accessibilityProperty_->actionScrollBackwardImpl_();
    EXPECT_NE(pattern_->animator_, nullptr);
}

/**
 * @tc.name: Pattern003
 * @tc.desc: Test SetAccessibilityAction with unScrollable scroll
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Pattern003, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Trigger actionScrollForwardImpl_
     * @tc.expected: Can not ScrollPage
     */
    accessibilityProperty_->actionScrollForwardImpl_();
    EXPECT_EQ(pattern_->animator_, nullptr);

    /**
     * @tc.steps: step2. Trigger actionScrollBackwardImpl_
     * @tc.expected: Can not ScrollPage
     */
    accessibilityProperty_->actionScrollBackwardImpl_();
    EXPECT_EQ(pattern_->animator_, nullptr);
}

/**
 * @tc.name: Pattern004
 * @tc.desc: Test SetAccessibilityAction with Axis::NONE
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Pattern004, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    scrollModel.SetAxis(Axis::NONE);
    CreateContent();
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Trigger actionScrollForwardImpl_
     * @tc.expected: Can not ScrollPage
     */
    accessibilityProperty_->actionScrollForwardImpl_();
    EXPECT_EQ(pattern_->animator_, nullptr);

    /**
     * @tc.steps: step2. Trigger actionScrollBackwardImpl_
     * @tc.expected: Can not ScrollPage
     */
    accessibilityProperty_->actionScrollBackwardImpl_();
    EXPECT_EQ(pattern_->animator_, nullptr);
}

/**
 * @tc.name: Pattern006
 * @tc.desc: Test FireOnScrollStop
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Pattern006, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    CreateContent();
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. When scrollAbort_ is true
     * @tc.expected: scrollAbort_ would be false
     */
    pattern_->scrollAbort_ = true;
    pattern_->FireOnScrollStop();
    EXPECT_FALSE(pattern_->scrollAbort_);
}

/**
 * @tc.name: Pattern007
 * @tc.desc: Test HandleScrollBarOutBoundary
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Pattern007, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    CreateContent();
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. When scrollBar is not OFF
     * @tc.expected: outBoundary_ would be set
     */
    pattern_->SetScrollBarOutBoundaryExtent(100.f);
    pattern_->HandleScrollBarOutBoundary();
    auto scrollBar = pattern_->GetScrollBar();
    EXPECT_EQ(scrollBar->outBoundary_, 100.f);

    /**
     * @tc.steps: step1. When scrollBar is OFF
     * @tc.expected: outBoundary_ would not be set
     */
    pattern_->SetScrollBarOutBoundaryExtent(200.f);
    scrollBar->displayMode_ = DisplayMode::OFF;
    pattern_->HandleScrollBarOutBoundary();
    EXPECT_EQ(scrollBar->outBoundary_, 100.f);
}

/**
 * @tc.name: Pattern008
 * @tc.desc: Test CreateOrStopAnimator
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Pattern008, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    CreateContent();
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Trigger CreateOrStopAnimator by ScrollToEdge
     * @tc.expected: animator_ would be create
     */
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, true);
    EXPECT_NE(pattern_->animator_, nullptr);

    /**
     * @tc.steps: step2. animator is running and Trigger CreateOrStopAnimator by ScrollToEdge again
     * @tc.expected: animator_ would be stop
     */
    pattern_->animator_->Resume();
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, true);
    EXPECT_TRUE(pattern_->scrollAbort_);
    EXPECT_TRUE(pattern_->animator_->IsStopped());

    /**
     * @tc.steps: step3. animator is stop and Trigger CreateOrStopAnimator by ScrollToEdge again
     * @tc.expected: animator_ would be stop
     */
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, true);
    EXPECT_TRUE(pattern_->scrollAbort_);
    EXPECT_TRUE(pattern_->animator_->IsStopped());
}

/**
 * @tc.name: Pattern009
 * @tc.desc: Test ScrollToEdge
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Pattern009, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    CreateContent();
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. ScrollEdgeType::SCROLL_NONE
     * @tc.expected: CurrentOffset would not be change
     */
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_NONE, false);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset::Zero()));

    /**
     * @tc.steps: step1. ScrollEdgeType::SCROLL_BOTTOM
     * @tc.expected: CurrentOffset would down to bottom
     */
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, ROOT_HEIGHT - COLUMN_HEIGHT)));

    /**
     * @tc.steps: step1. ScrollEdgeType::SCROLL_TOP
     * @tc.expected: CurrentOffset would up to top
     */
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, false);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset::Zero()));
}

/**
 * @tc.name: Pattern010
 * @tc.desc: Test DoJump
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Pattern010, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    CreateContent();
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. jump to a position
     * @tc.expected: CurrentOffset would be to the position
     */
    pattern_->DoJump(-100.f, SCROLL_FROM_UPDATE);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, -100.f)));

    /**
     * @tc.steps: step2. jump to the same position
     * @tc.expected: CurrentOffset would not be change
     */
    pattern_->DoJump(-100.f, SCROLL_FROM_UPDATE);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, -100.f)));
}

/**
 * @tc.name: Pattern011
 * @tc.desc: Test ScrollBy
 * @tc.type: FUNC
 */
HWTEST_F(ScrollTestNg, Pattern011, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    CreateContent();
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. ScrollBy 0
     * @tc.expected: CurrentOffset would not change
     */
    pattern_->ScrollBy(0, 0, false);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset::Zero()));

    /**
     * @tc.steps: step2. ScrollBy a distance
     * @tc.expected: Scroll by the distance
     */
    pattern_->ScrollBy(0, -100.f, false);
    EXPECT_TRUE(IsEqualCurrentOffset(Offset(0, -100.f)));
}
} // namespace OHOS::Ace::NG
