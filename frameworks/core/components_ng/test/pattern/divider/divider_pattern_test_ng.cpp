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

#include "gtest/gtest.h"
#define private public
#include "base/geometry/ng/size_t.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/divider/divider_layout_algorithm.h"
#include "core/components_ng/pattern/divider/divider_layout_property.h"
#include "core/components_ng/pattern/divider/divider_model_ng.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/divider/divider_render_property.h"
#include "core/pipeline_ng/test/mock/mock_pipeline_base.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
struct TestProperty {
    std::optional<bool> vertical = std::nullopt;
    std::optional<Color> dividerColor = std::nullopt;
    std::optional<Dimension> strokeWidth = std::nullopt;
    std::optional<Ace::LineCap> lineCap = std::nullopt;
};
namespace {
const bool VERTICAL_TRUE = true;
const bool VERTICAL_FALSE = false;
const Color DIVIDER_COLOR = Color(0xff000000);
const Dimension STROKE_WIDTH = Dimension(1.0);
const Ace::LineCap LINE_CAP = Ace::LineCap::BUTT;
const OptionalSize<float> SELF_IDEAL_SIZE = OptionalSize<float>(100.0f, 100.0f);
const SizeT<float> MAX_SIZE = SizeT<float>(1000.0f, 1000.0f);
TestProperty testProperty;
DirtySwapConfig config;
} // namespace

class DividerPatternTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

protected:
    static RefPtr<FrameNode> CreateDividerNode(TestProperty& testProperty);
};

void DividerPatternTestNg::SetUpTestCase()
{
    MockPipelineBase::SetUp();
    testProperty.vertical = VERTICAL_TRUE;
    testProperty.dividerColor = DIVIDER_COLOR;
    testProperty.strokeWidth = STROKE_WIDTH;
    testProperty.lineCap = LINE_CAP;
}

void DividerPatternTestNg::TearDownTestCase()
{
    MockPipelineBase::TearDown();
}

RefPtr<FrameNode> DividerPatternTestNg::CreateDividerNode(TestProperty& testProperty)
{
    DividerModelNG().Create();
    if (testProperty.vertical.has_value()) {
        DividerModelNG().Vertical(testProperty.vertical.value());
    }
    if (testProperty.dividerColor.has_value()) {
        DividerModelNG().DividerColor(testProperty.dividerColor.value());
    }
    if (testProperty.strokeWidth.has_value()) {
        DividerModelNG().StrokeWidth(testProperty.strokeWidth.value());
    }
    if (testProperty.lineCap.has_value()) {
        DividerModelNG().LineCap(testProperty.lineCap.value());
    }

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish(); // TextView pop
    return AceType::DynamicCast<FrameNode>(element);
}

/**
 * @tc.name: DividerFrameNodeCreator001
 * @tc.desc: Test all the property of divider.
 * @tc.type: FUNC
 */
HWTEST_F(DividerPatternTestNg, DividerPatternTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode
     * @tc.expected: step1. create frameNode success
     */
    RefPtr<FrameNode> frameNode = CreateDividerNode(testProperty);
    EXPECT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. create divider layout and paint property
     * @tc.expected: step2. create property success
     */
    RefPtr<DividerLayoutProperty> layoutProperty = frameNode->GetLayoutProperty<DividerLayoutProperty>();
    EXPECT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetVerticalValue(), VERTICAL_TRUE);
    EXPECT_EQ(layoutProperty->GetStrokeWidthValue(), STROKE_WIDTH);
    RefPtr<DividerRenderProperty> paintProperty = frameNode->GetPaintProperty<DividerRenderProperty>();
    EXPECT_EQ(paintProperty->GetDividerColorValue(), DIVIDER_COLOR);
    EXPECT_EQ(paintProperty->GetLineCapValue(), LINE_CAP);
}

/**
 * @tc.name: DividerFrameNodeCreator002
 * @tc.desc: Test all the layoutAlgorithm of divider
 * @tc.type: FUNC
 */
HWTEST_F(DividerPatternTestNg, DividerPatternTest002, TestSize.Level1)
{
    bool vertical[2] = { VERTICAL_TRUE, VERTICAL_FALSE };
    testProperty.strokeWidth = STROKE_WIDTH;
    LayoutConstraintF layoutConstraintF;
    layoutConstraintF.selfIdealSize = SELF_IDEAL_SIZE;
    layoutConstraintF.maxSize = MAX_SIZE;
    RefPtr<DividerLayoutAlgorithm> dividerLayoutAlgorithm = AceType::MakeRefPtr<DividerLayoutAlgorithm>();
    /**
     * @tc.steps: step1. layout algorithm measureContent method about vertical
     * @tc.expected: step1. vertical is false or true, the constrainSize is (1.0, 100.0)
     */
    SizeF sizes[2] = {{1.0f, 100.0f}, {100.f, 1.0f}};
    for (int32_t i = 0; i < 2; ++i) {
        testProperty.vertical = vertical[i];
        RefPtr<FrameNode> frameNode = CreateDividerNode(testProperty);
        EXPECT_NE(frameNode, nullptr);
        RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
        EXPECT_NE(geometryNode, nullptr);
        RefPtr<DividerLayoutProperty> layoutProperty = frameNode->GetLayoutProperty<DividerLayoutProperty>();
        EXPECT_NE(layoutProperty, nullptr);
        LayoutWrapper layoutWrapper = LayoutWrapper(frameNode, geometryNode, layoutProperty);
        auto constrainSize = dividerLayoutAlgorithm->MeasureContent(layoutConstraintF, &layoutWrapper);
        EXPECT_EQ(constrainSize.value(), sizes[i]);
    }
    /**
     * @tc.steps: step2. layout algorithm test
     * @tc.expected: step2. constrainStrokeWidth is 1.0f, dividerLength is 100.0f, and vertical is false
     */
    EXPECT_EQ(dividerLayoutAlgorithm->GetConstrainStrokeWidth(), 1.0f);
    EXPECT_EQ(dividerLayoutAlgorithm->GetDividerLength(), 100.0f);
    EXPECT_EQ(dividerLayoutAlgorithm->GetVertical(), VERTICAL_FALSE);
}

/**
 * @tc.name: DividerFrameNodeCreator002
 * @tc.desc: Test all the pattern of divider
 * @tc.type: FUNC
 */
HWTEST_F(DividerPatternTestNg, DividerPatternTest003, TestSize.Level1)
{
    RefPtr<DividerLayoutAlgorithm> dividerLayoutAlgorithm = AceType::MakeRefPtr<DividerLayoutAlgorithm>();
    RefPtr<FrameNode> frameNode = CreateDividerNode(testProperty);
    EXPECT_NE(frameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    RefPtr<DividerLayoutProperty> layoutProperty = frameNode->GetLayoutProperty<DividerLayoutProperty>();
    EXPECT_NE(layoutProperty, nullptr);
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapper>(frameNode, geometryNode, layoutProperty);
    EXPECT_NE(layoutWrapper, nullptr);
    bool skipMeasures[2] = { false, true };
    bool isSwaps[4] = {true, false, false, false};
    /**
     * @tc.steps: step1. OnDirtyLayoutWrapperSwap test
     * @tc.expected: step1. adjust skipMeasure and config, OnDirtyLayoutWrapperSwap return true, false
     */
    int k = 0;
    for (int i = 0; i < 2; ++i) {
        config.skipMeasure = skipMeasures[i];
        for (int j = 0; j < 2; ++j) {
            auto layoutAlgorithmWrapper =
                AceType::MakeRefPtr<LayoutAlgorithmWrapper>(dividerLayoutAlgorithm, skipMeasures[j]);
            layoutWrapper->SetLayoutAlgorithm(layoutAlgorithmWrapper);
            layoutWrapper->skipMeasureContent_ = skipMeasures[j];
            auto pattern = frameNode->GetPattern<DividerPattern>();
            EXPECT_NE(pattern, nullptr);
            auto isSwap = pattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
            EXPECT_EQ(isSwap, isSwaps[k]);
            k++;
        }
    }
}
} // namespace OHOS::Ace::NG
