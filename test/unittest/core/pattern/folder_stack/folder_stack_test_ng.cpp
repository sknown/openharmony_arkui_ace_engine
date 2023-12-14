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

#include "gtest/gtest.h"

#include "core/components/common/properties/alignment.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/blank/blank_model_ng.h"

#define private public
#include "test/mock/core/pipeline/mock_pipeline_context.h"

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/blank/blank_model_ng.h"
#include "core/components_ng/pattern/folder_stack/folder_stack_layout_algorithm.h"
#include "core/components_ng/pattern/folder_stack/folder_stack_layout_property.h"
#include "core/components_ng/pattern/folder_stack/folder_stack_model_ng.h"
#include "core/components_ng/pattern/folder_stack/folder_stack_pattern.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_v2/inspector/inspector_constants.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
constexpr float ZERO = 0.0f;
const OffsetF ORIGIN_POINT(ZERO, ZERO);
constexpr float STACK_HEIGHT = 120.0f;
constexpr float FULL_SCREEN_WIDTH = 2224.0f;
constexpr float FULL_SCREEN_HEIGHT = 2496.0f;
const SizeF CONTAINER_SIZE(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);

constexpr float SMALL_ITEM_WIDTH = 100.0f;
constexpr float SMALL_ITEM_HEIGHT = 40.0f;
const SizeF SMALL_ITEM_SIZE(SMALL_ITEM_WIDTH, SMALL_ITEM_HEIGHT);
} // namespace
class FolderStackTestNg : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        MockPipelineContext::SetUp();
    }

    static void TearDownTestSuite()
    {
        MockPipelineContext::TearDown();
    }

protected:
    // std::pair<RefPtr<FrameNode>, RefPtr<LayoutWrapperNode>> CreateStack(const Alignment alignment);
    RefPtr<UINode> CreateWithItem(bool enableAnimation, bool autoHalfFold, Alignment align);
    PaddingProperty CreatePadding(float left, float top, float right, float bottom);
};

PaddingProperty FolderStackTestNg::CreatePadding(float left, float top, float right, float bottom)
{
    PaddingProperty padding;
    padding.left = CalcLength(left);
    padding.right = CalcLength(right);
    padding.top = CalcLength(top);
    padding.bottom = CalcLength(bottom);
    return padding;
}

RefPtr<UINode> FolderStackTestNg::CreateWithItem(bool enableAnimation, bool autoHalfFold, Alignment align)
{
    FolderStackModelNG folderStackModelNG;
    folderStackModelNG.Create();
    folderStackModelNG.SetAlignment(align);
    folderStackModelNG.SetEnableAnimation(enableAnimation);
    folderStackModelNG.SetAutoHalfFold(autoHalfFold);
    {
        BlankModelNG blankModelNG;
        blankModelNG.Create();
        ViewAbstract::SetWidth(CalcLength(SMALL_ITEM_WIDTH));
        ViewAbstract::SetHeight(CalcLength(SMALL_ITEM_HEIGHT));
        PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
        ViewAbstract::SetPadding(noPadding);
        ViewStackProcessor::GetInstance()->Pop();
    }
    auto frameNode = ViewStackProcessor::GetInstance()->Finish();
    return frameNode;
}

/**
 * @tc.name: FolderStackTestNgTest001
 * @tc.desc: Test folderStack Layout with Alignment
 * @tc.type: FUNC
 */
HWTEST_F(FolderStackTestNg, FolderStackTestNgTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create folderStack and get frameNode.
     */
    Alignment align = Alignment::TOP_LEFT;
    bool enableAnimation = false;
    bool autoHalfFold = false;
    auto frameNode = AceType::DynamicCast<FrameNode>(CreateWithItem(enableAnimation, autoHalfFold, align));
    auto layoutProperty = frameNode->GetLayoutProperty<FolderStackLayoutProperty>();
    auto alignment = layoutProperty->GetPositionProperty()->GetAlignment();
    ASSERT_EQ(alignment, Alignment::TOP_LEFT);
    ASSERT_EQ(layoutProperty->GetEnableAnimation(), false);
    ASSERT_EQ(layoutProperty->GetAutoHalfFold(), false);

    /**
     * @tc.steps: step2. get layout property, layoutAlgorithm and create layoutWrapper.
     * @tc.expected: related function is called.
     */
    RefPtr<GeometryNode> geometryNode = frameNode->GetGeometryNode();
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, frameNode->GetLayoutProperty());
    auto folderStackPattern = frameNode->GetPattern<FolderStackPattern>();
    ASSERT_NE(folderStackPattern, nullptr);
    auto layoutAlgorithm =
        AceType::DynamicCast<FolderStackLayoutAlgorithm>(folderStackPattern->CreateLayoutAlgorithm());
    ASSERT_NE(layoutAlgorithm, nullptr);
    layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm));

    /**
     * @tc.steps: step3. update layoutWrapper.
     */
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(STACK_HEIGHT)));
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

    /**
     * @tc.steps: step4. use layoutAlgorithm to layout.
     * @tc.expected: check whether the value of geometry frameSize and frameOffset is correct.
     */
    layoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    layoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    ASSERT_EQ(layoutWrapper->GetGeometryNode()->GetMarginFrameSize(), SizeF(FULL_SCREEN_WIDTH, STACK_HEIGHT));
    ASSERT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OffsetF(ZERO, ZERO));
}

/**
 * @tc.name: FolderStackTestNgTest002
 * @tc.desc: Test folderStack Layout with AlignmentContent
 * @tc.type: FUNC
 */
HWTEST_F(FolderStackTestNg, FolderStackTestNgTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create folderStack and get frameNode.
     */
    FolderStackModelNG folderStackModelNG;
    folderStackModelNG.Create();
    folderStackModelNG.SetAlignment(Alignment::TOP_LEFT);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<FolderStackLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    ASSERT_NE(layoutProperty->GetPositionProperty(), nullptr);
    auto alignmentContent = layoutProperty->GetPositionProperty()->GetAlignment();
    ASSERT_EQ(alignmentContent, Alignment::TOP_LEFT);
    layoutProperty->UpdateLayoutDirection(TextDirection::LTR);

    /**
     * @tc.steps: step2. get layout property, layoutAlgorithm and create layoutWrapper.
     * @tc.expected: related function is called.
     */

    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, frameNode->GetLayoutProperty());
    auto folderStackPattern = frameNode->GetPattern<FolderStackPattern>();
    ASSERT_NE(folderStackPattern, nullptr);
    auto layoutAlgorithm = folderStackPattern->CreateLayoutAlgorithm();
    ASSERT_NE(layoutAlgorithm, nullptr);

    /**
     * @tc.steps: step3. update layoutWrapper.
     */
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(STACK_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    /**
     * @tc.steps: step4. use layoutAlgorithm to layout.
     * @tc.expected: check whether the value of geometry frameSize and frameOffset is correct.
     */
    layoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    layoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    ASSERT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(FULL_SCREEN_WIDTH, STACK_HEIGHT));
    ASSERT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OffsetF(ZERO, ZERO));
}
} // namespace OHOS::Ace::NG
