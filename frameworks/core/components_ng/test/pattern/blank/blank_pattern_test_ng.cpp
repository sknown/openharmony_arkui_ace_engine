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

#include <optional>

#include "gtest/gtest.h"

#define private public
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/blank/blank_layout_property.h"
#include "core/components_ng/pattern/blank/blank_pattern.h"
#include "core/components_ng/pattern/blank/blank_view.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_utils.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "frameworks/bridge/declarative_frontend/view_stack_processor.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
constexpr float COLUMN_HEIGHT = 200.0f;
constexpr float FULL_SCREEN_WIDTH = 720.0f;
constexpr float FULL_SCREEN_HEIGHT = 1136.0f;
constexpr float ZERO = 0.0f;
constexpr float ROW_HEIGHT = 120.0f;
constexpr float SMALL_ITEM_WIDTH = 150.0f;
constexpr float SMALL_ITEM_HEIGHT = 60.0f;

constexpr Dimension BLANK_MIN(10.0f);
constexpr Dimension SMALL_BLANK_HEIGHT(5.0f);
constexpr Dimension LARGE_BLANK_HEIGHT(20.0f);
constexpr Dimension MIN(0.0f);

const OffsetF ORIGIN_POINT(ZERO, ZERO);
const SizeF SMALL_ITEM_SIZE(SMALL_ITEM_WIDTH, SMALL_ITEM_HEIGHT);
const std::string COLOR_WHITE = "#FFFFFFFF";
const SizeF CONTAINER_SIZE(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
} // namespace

class BlankPatternTestNg : public testing::Test {
public:
    static void SetUpTestSuite() {};
    static void TearDownTestSuite() {};

protected:
    std::pair<RefPtr<FrameNode>, RefPtr<LayoutWrapper>> CreateRow();
    std::pair<RefPtr<FrameNode>, RefPtr<LayoutWrapper>> CreateColumn();
    std::pair<RefPtr<FrameNode>, RefPtr<LayoutWrapper>> CreateFlexRow();
    std::pair<RefPtr<FrameNode>, RefPtr<LayoutWrapper>> CreateFlexColumn();
    PaddingProperty CreatePadding(float left, float top, float right, float bottom);
};

std::pair<RefPtr<FrameNode>, RefPtr<LayoutWrapper>> BlankPatternTestNg::CreateRow()
{
    auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto rowFrameNode =
        FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, nodeId, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_NE(rowFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_NE(rowLayoutPattern, nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_NE(rowLayoutProperty, nullptr);

    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(ROW_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    return { rowFrameNode, layoutWrapper };
}

std::pair<RefPtr<FrameNode>, RefPtr<LayoutWrapper>> BlankPatternTestNg::CreateColumn()
{
    auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto columnFrameNode =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, nodeId, AceType::MakeRefPtr<LinearLayoutPattern>(true));
    EXPECT_NE(columnFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(columnFrameNode, geometryNode, columnFrameNode->GetLayoutProperty());

    auto columnLayoutPattern = columnFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_NE(columnLayoutPattern, nullptr);
    auto rowLayoutProperty = columnLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_NE(rowLayoutProperty, nullptr);

    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(COLUMN_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    return { columnFrameNode, layoutWrapper };
}

std::pair<RefPtr<FrameNode>, RefPtr<LayoutWrapper>> BlankPatternTestNg::CreateFlexRow()
{
    auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto rowFrameNode =
        FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, nodeId, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_NE(rowFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());
    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_NE(rowLayoutPattern, nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_NE(rowLayoutProperty, nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_NE(rowLayoutAlgorithm, nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(ROW_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    return { rowFrameNode, layoutWrapper };
}

std::pair<RefPtr<FrameNode>, RefPtr<LayoutWrapper>> BlankPatternTestNg::CreateFlexColumn()
{
    auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto columnFrameNode =
        FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, nodeId, AceType::MakeRefPtr<LinearLayoutPattern>(true));
    EXPECT_NE(columnFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(columnFrameNode, geometryNode, columnFrameNode->GetLayoutProperty());
    auto columnLayoutPattern = columnFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_NE(columnLayoutPattern, nullptr);
    auto columnLayoutProperty = columnLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_NE(columnLayoutProperty, nullptr);
    columnLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN);
    columnLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    auto columnLayoutAlgorithm = columnLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_NE(columnLayoutAlgorithm, nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(columnLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(COLUMN_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    return { columnFrameNode, layoutWrapper };
}

PaddingProperty BlankPatternTestNg::CreatePadding(float left, float top, float right, float bottom)
{
    PaddingProperty padding;
    padding.left = CalcLength(left);
    padding.right = CalcLength(right);
    padding.top = CalcLength(top);
    padding.bottom = CalcLength(bottom);
    return padding;
}

/**
 * @tc.name: BlankFrameNodeCreator001
 * @tc.desc: Test blankMin of Blank.
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(BlankPatternTestNg, BlankFrameNodeCreator001, TestSize.Level1)
{
    BlankView blank;
    blank.Create();
    blank.SetBlankMin(BLANK_MIN);

    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_NE(frameNode, nullptr);
    auto blankLayoutProperty = frameNode->GetLayoutProperty<BlankLayoutProperty>();
    EXPECT_NE(blankLayoutProperty, nullptr);
    EXPECT_EQ(blankLayoutProperty->GetMinSize().value_or(Dimension()), MIN);
}

/**
 * @tc.name: BlankFrameNodeCreator002
 * @tc.desc: Test height of Blank.
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(BlankPatternTestNg, BlankFrameNodeCreator002, TestSize.Level1)
{
    BlankView blank;
    blank.Create();
    blank.SetHeight(SMALL_BLANK_HEIGHT);

    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_NE(frameNode, nullptr);
    auto blankLayoutProperty = frameNode->GetLayoutProperty<BlankLayoutProperty>();
    EXPECT_NE(blankLayoutProperty, nullptr);
    EXPECT_EQ(blankLayoutProperty->GetHeight(), SMALL_BLANK_HEIGHT);

    auto blankPattern = frameNode->GetPattern<BlankPattern>();
    EXPECT_NE(blankPattern, nullptr);
    auto layoutProperty = blankPattern->CreateLayoutProperty();
    EXPECT_NE(layoutProperty, nullptr);
}

/**
 * @tc.name: BlankPatternTest001
 * @tc.desc: Test Blank layout: parent = Row.
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(BlankPatternTestNg, BlankPatternTest001, TestSize.Level1)
{
    auto row = CreateRow();
    auto rowFrameNode = row.first;
    EXPECT_NE(rowFrameNode, nullptr);
    auto rowLayoutWrapper = row.second;

    auto childLayoutConstraint = rowLayoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);

    // create child
    BlankView blank;
    blank.Create();
    blank.SetBlankMin(BLANK_MIN);
    blank.SetHeight(SMALL_BLANK_HEIGHT);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_EQ(frameNode->GetTag(), V2::BLANK_ETS_TAG);
    EXPECT_NE(frameNode, nullptr);
    auto blankLayoutProperty = frameNode->GetLayoutProperty<BlankLayoutProperty>();
    EXPECT_NE(blankLayoutProperty, nullptr);
    auto blankPattern = frameNode->GetPattern<BlankPattern>();
    EXPECT_NE(blankPattern, nullptr);
    RefPtr<GeometryNode> firstGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> firstLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(frameNode, firstGeometryNode, frameNode->GetLayoutProperty());
    firstLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    firstLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    firstLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    auto blankLayoutAlgorithm = frameNode->GetPattern<BlankPattern>()->CreateLayoutAlgorithm();
    EXPECT_NE(blankLayoutAlgorithm, nullptr);
    firstLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(blankLayoutAlgorithm));

    rowFrameNode->AddChild(frameNode);
    rowLayoutWrapper->AppendChild(firstLayoutWrapper);
    blankPattern->OnMountToParentDone();
    auto parent = frameNode->GetParent();
    EXPECT_EQ(parent->GetTag(), V2::ROW_ETS_TAG);
    LinearLayoutUtils::Measure(AccessibilityManager::RawPtr(rowLayoutWrapper), false);
    LinearLayoutUtils::Layout(
        AccessibilityManager::RawPtr(rowLayoutWrapper), false, FlexAlign::FLEX_START, FlexAlign::FLEX_START);

    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameOffset(), ORIGIN_POINT);

    blankLayoutProperty->UpdateHeight(LARGE_BLANK_HEIGHT);
    blankPattern->OnModifyDone();
    blankPattern->OnMountToParentDone();

    LinearLayoutUtils::Measure(AccessibilityManager::RawPtr(rowLayoutWrapper), false);
    LinearLayoutUtils::Layout(
        AccessibilityManager::RawPtr(rowLayoutWrapper), false, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
}

/**
 * @tc.name: BlankPatternTest002
 * @tc.desc: Test Blank layout: parent = Column.
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(BlankPatternTestNg, BlankPatternTest002, TestSize.Level1)
{
    auto column = CreateColumn();
    auto columnFrameNode = column.first;
    auto columnLayoutWrapper = column.second;

    auto childLayoutConstraint = columnLayoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);

    BlankView blank;
    blank.Create();
    blank.SetBlankMin(BLANK_MIN);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_NE(frameNode, nullptr);
    auto blankLayoutProperty = frameNode->GetLayoutProperty<BlankLayoutProperty>();
    EXPECT_NE(blankLayoutProperty, nullptr);
    auto blankPattern = frameNode->GetPattern<BlankPattern>();
    EXPECT_NE(blankPattern, nullptr);
    RefPtr<GeometryNode> firstGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> firstLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(frameNode, firstGeometryNode, frameNode->GetLayoutProperty());
    firstLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    firstLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    firstLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    auto blankLayoutAlgorithm = frameNode->GetPattern<BlankPattern>()->CreateLayoutAlgorithm();
    EXPECT_NE(blankLayoutAlgorithm, nullptr);
    firstLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(blankLayoutAlgorithm));
    columnFrameNode->AddChild(frameNode);
    columnLayoutWrapper->AppendChild(firstLayoutWrapper);

    blankPattern->OnMountToParentDone();
    auto parent = frameNode->GetParent();
    EXPECT_EQ(parent->GetTag(), V2::COLUMN_ETS_TAG);

    LinearLayoutUtils::Measure(AccessibilityManager::RawPtr(columnLayoutWrapper), true);
    LinearLayoutUtils::Layout(
        AccessibilityManager::RawPtr(columnLayoutWrapper), false, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameOffset(), ORIGIN_POINT);

    blankLayoutProperty->UpdateHeight(LARGE_BLANK_HEIGHT);
    blankPattern->OnModifyDone();
    blankPattern->OnMountToParentDone();

    LinearLayoutUtils::Measure(AccessibilityManager::RawPtr(columnLayoutWrapper), false);
    LinearLayoutUtils::Layout(
        AccessibilityManager::RawPtr(columnLayoutWrapper), false, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
}

/**
 * @tc.name: BlankPatternTest003
 * @tc.desc: Test Blank layout: parent = Flex_Row.
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(BlankPatternTestNg, BlankPatternTest003, TestSize.Level1)
{
    auto flexRow = CreateFlexRow();
    auto flexRowFrameNode = flexRow.first;
    auto flexRowLayoutWrapper = flexRow.second;

    auto childLayoutConstraint = flexRowLayoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);

    BlankView blank;
    blank.Create();
    blank.SetBlankMin(BLANK_MIN);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_NE(frameNode, nullptr);
    auto blankLayoutProperty = frameNode->GetLayoutProperty<BlankLayoutProperty>();
    EXPECT_NE(blankLayoutProperty, nullptr);
    auto blankPattern = frameNode->GetPattern<BlankPattern>();
    EXPECT_NE(blankPattern, nullptr);
    RefPtr<GeometryNode> firstGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> firstLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(frameNode, firstGeometryNode, frameNode->GetLayoutProperty());
    firstLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    firstLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    firstLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    auto blankLayoutAlgorithm = frameNode->GetPattern<BlankPattern>()->CreateLayoutAlgorithm();
    EXPECT_NE(blankLayoutAlgorithm, nullptr);
    firstLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(blankLayoutAlgorithm));
    flexRowFrameNode->AddChild(frameNode);
    flexRowLayoutWrapper->AppendChild(firstLayoutWrapper);

    blankPattern->OnMountToParentDone();
    auto parent = frameNode->GetParent();
    EXPECT_EQ(parent->GetTag(), V2::FLEX_ETS_TAG);

    LinearLayoutUtils::Measure(AccessibilityManager::RawPtr(flexRowLayoutWrapper), false);
    LinearLayoutUtils::Layout(
        AccessibilityManager::RawPtr(flexRowLayoutWrapper), false, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameOffset(), ORIGIN_POINT);

    blankLayoutProperty->UpdateHeight(LARGE_BLANK_HEIGHT);
    blankPattern->OnModifyDone();
    blankPattern->OnMountToParentDone();

    LinearLayoutUtils::Measure(AccessibilityManager::RawPtr(flexRowLayoutWrapper), false);
    LinearLayoutUtils::Layout(
        AccessibilityManager::RawPtr(flexRowLayoutWrapper), false, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
}

/**
 * @tc.name: BlankPatternTest004
 * @tc.desc: Test Blank layout: parent = Flex_Column.
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(BlankPatternTestNg, BlankPatternTest004, TestSize.Level1)
{
    auto flexColumn = CreateFlexColumn();
    auto flexColumnFrameNode = flexColumn.first;
    auto flexColumnLayoutWrapper = flexColumn.second;

    auto childLayoutConstraint = flexColumnLayoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);

    BlankView blank;
    blank.Create();
    blank.SetBlankMin(BLANK_MIN);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_NE(frameNode, nullptr);
    auto blankLayoutProperty = frameNode->GetLayoutProperty<BlankLayoutProperty>();
    EXPECT_NE(blankLayoutProperty, nullptr);
    auto blankPattern = frameNode->GetPattern<BlankPattern>();
    EXPECT_NE(blankPattern, nullptr);
    RefPtr<GeometryNode> firstGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> firstLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(frameNode, firstGeometryNode, frameNode->GetLayoutProperty());
    firstLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    firstLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    PaddingProperty noPadding = CreatePadding(ZERO, ZERO, ZERO, ZERO);
    firstLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    auto blankLayoutAlgorithm = frameNode->GetPattern<BlankPattern>()->CreateLayoutAlgorithm();
    EXPECT_NE(blankLayoutAlgorithm, nullptr);
    firstLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(blankLayoutAlgorithm));
    flexColumnFrameNode->AddChild(frameNode);
    flexColumnLayoutWrapper->AppendChild(firstLayoutWrapper);

    blankPattern->OnMountToParentDone();
    auto parent = frameNode->GetParent();
    EXPECT_EQ(parent->GetTag(), V2::FLEX_ETS_TAG);

    LinearLayoutUtils::Measure(AccessibilityManager::RawPtr(flexColumnLayoutWrapper), true);
    LinearLayoutUtils::Layout(
        AccessibilityManager::RawPtr(flexColumnLayoutWrapper), false, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameOffset(), ORIGIN_POINT);

    blankLayoutProperty->UpdateHeight(LARGE_BLANK_HEIGHT);
    blankPattern->OnModifyDone();
    blankPattern->OnMountToParentDone();

    LinearLayoutUtils::Measure(AccessibilityManager::RawPtr(flexColumnLayoutWrapper), true);
    LinearLayoutUtils::Layout(
        AccessibilityManager::RawPtr(flexColumnLayoutWrapper), false, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
}

/**
 * @tc.name: BlankPatternTest005
 * @tc.desc: Test invalid tag of Blank.
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(BlankPatternTestNg, BlankPatternTest005, TestSize.Level1)
{
    BlankView blank;
    Dimension blankMin;

    auto rowFrameNode =
        FrameNode::CreateFrameNode(V2::BADGE_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_NE(rowFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());
    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_NE(rowLayoutPattern, nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_NE(rowLayoutProperty, nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_NE(rowLayoutAlgorithm, nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(ROW_HEIGHT)));
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

    blank.Create();
    blankMin.SetValue(10);
    blank.SetBlankMin(blankMin);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_NE(frameNode, nullptr);
    RefPtr<GeometryNode> firstGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> firstLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(frameNode, firstGeometryNode, frameNode->GetLayoutProperty());
    firstLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    firstLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    firstLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    auto blankLayoutAlgorithm = frameNode->GetPattern<BlankPattern>()->CreateLayoutAlgorithm();
    EXPECT_NE(blankLayoutAlgorithm, nullptr);
    firstLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(blankLayoutAlgorithm));
    rowFrameNode->AddChild(frameNode);
    layoutWrapper->AppendChild(firstLayoutWrapper);

    frameNode->GetPattern<BlankPattern>()->OnMountToParentDone();
    auto parent = frameNode->GetParent();
    EXPECT_EQ(parent->GetTag(), V2::BADGE_ETS_TAG);
}

/**
 * @tc.name: BlankPatternTest006
 * @tc.desc: Test invalid flexDirection of Blank.
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(BlankPatternTestNg, BlankPatternTest006, TestSize.Level1)
{
    BlankView blank;
    Dimension blankMin;

    auto rowFrameNode =
        FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_NE(rowFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());
    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_NE(rowLayoutPattern, nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_NE(rowLayoutProperty, nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_NE(rowLayoutAlgorithm, nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(ROW_HEIGHT)));
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

    blank.Create();
    blankMin.SetValue(10);
    blank.SetBlankMin(blankMin);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_NE(frameNode, nullptr);
    RefPtr<GeometryNode> firstGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> firstLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(frameNode, firstGeometryNode, frameNode->GetLayoutProperty());
    firstLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    firstLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    firstLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    auto blankLayoutAlgorithm = frameNode->GetPattern<BlankPattern>()->CreateLayoutAlgorithm();
    EXPECT_NE(blankLayoutAlgorithm, nullptr);
    firstLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(blankLayoutAlgorithm));
    rowFrameNode->AddChild(frameNode);
    layoutWrapper->AppendChild(firstLayoutWrapper);

    frameNode->GetPattern<BlankPattern>()->OnMountToParentDone();
    auto parent = frameNode->GetParent();
    EXPECT_EQ(parent->GetTag(), V2::FLEX_ETS_TAG);
}

/**
 * @tc.name: BlankPatternTest007
 * @tc.desc: Test invalid minsize of Blank.
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(BlankPatternTestNg, BlankPatternTest007, TestSize.Level1)
{
    BlankView blank;
    blank.Create();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_NE(frameNode, nullptr);
    auto blankLayoutProperty = frameNode->GetLayoutProperty<BlankLayoutProperty>();
    EXPECT_NE(blankLayoutProperty, nullptr);
    frameNode->GetPattern<BlankPattern>()->OnMountToParentDone();
    EXPECT_NE(blankLayoutProperty->HasMinSize(), true);
}

/**
 * @tc.name: ToJsonValue001
 * @tc.desc: Test ToJsonValue of Blank.
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(BlankPatternTestNg, ToJsonValue001, TestSize.Level1)
{
    BlankView blank;
    blank.Create();
    blank.SetBlankMin(BLANK_MIN);

    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_NE(frameNode, nullptr);
    auto blankLayoutProperty = frameNode->GetLayoutProperty<BlankLayoutProperty>();
    EXPECT_NE(blankLayoutProperty, nullptr);

    std::string colorString = frameNode->GetPattern<BlankPattern>()->GetColorString();
    EXPECT_EQ(colorString.c_str(), COLOR_WHITE);

    auto jsonValue = JsonUtil::Create(true);
    frameNode->GetPattern<BlankPattern>()->ToJsonValue(jsonValue);
    EXPECT_EQ(jsonValue->GetValue("color")->GetString().c_str(), COLOR_WHITE);
}
} // namespace OHOS::Ace::NG
