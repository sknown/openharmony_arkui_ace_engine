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

#include "grid_test_ng.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/render/mock_render_context.h"
#include "test/mock/core/rosen/mock_canvas.h"

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/grid/grid_item_model_ng.h"
#include "core/components_ng/pattern/grid/grid_item_pattern.h"
#include "core/components_ng/pattern/grid/grid_item_theme.h"
#include "core/components_ng/pattern/grid/grid_layout/grid_layout_algorithm.h"
#include "core/components_ng/pattern/grid/grid_scroll/grid_scroll_with_options_layout_algorithm.h"
#include "core/components_ng/pattern/grid/irregular/grid_irregular_layout_algorithm.h"
#include "core/components_ng/pattern/grid/irregular/grid_layout_utils.h"
#include "core/components_ng/pattern/text_field/text_field_manager.h"

namespace OHOS::Ace::NG {

namespace {} // namespace

class GridOptionLayoutTestNg : public GridTestNg {
public:
};

/**
 * @tc.name: GridScrollWithOptions001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, GridScrollWithOptions001, TestSize.Level1)
{
    GridLayoutOptions option;
    option.regularSize.rows = 1;
    option.regularSize.columns = 1;
    option.irregularIndexes = { 6, 1, 2, 3, 4, 5, 0 };
    auto onGetIrregularSizeByIndex = [](int32_t index) {
        GridItemSize gridItemSize;
        return gridItemSize;
    };
    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    model.SetLayoutOptions(option);
    CreateFixedItems(10);
    CreateDone(frameNode_);

    auto layoutAlgorithmWrapper = AceType::DynamicCast<LayoutAlgorithmWrapper>(frameNode_->GetLayoutAlgorithm());
    auto layoutAlgorithm =
        AceType::DynamicCast<GridScrollWithOptionsLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    if (AceType::InstanceOf<GridIrregularLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm())) {
        return;
    }
    layoutAlgorithm->GetTargetIndexInfoWithBenchMark(AccessibilityManager::RawPtr(frameNode_), false, 5);
    EXPECT_EQ(layoutAlgorithm->gridLayoutInfo_.startMainLineIndex_, 1);
}

/**
 * @tc.name: GridScrollWithOptions002
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, GridScrollWithOptions002, TestSize.Level1)
{
    GridLayoutOptions option;
    option.regularSize.rows = 1;
    option.regularSize.columns = 1;
    option.irregularIndexes = { 6, 1, 2, 3, 4, 5, 0 };
    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr 1fr 1fr");
    model.SetLayoutOptions(option);
    CreateFixedItems(10);
    CreateDone(frameNode_);

    auto layoutAlgorithmWrapper = AceType::DynamicCast<LayoutAlgorithmWrapper>(frameNode_->GetLayoutAlgorithm());
    auto layoutAlgorithm =
        AceType::DynamicCast<GridScrollWithOptionsLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    if (AceType::InstanceOf<GridIrregularLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm())) {
        return;
    }
    layoutAlgorithm->GetTargetIndexInfoWithBenchMark(AccessibilityManager::RawPtr(frameNode_), false, 5);
    EXPECT_EQ(layoutAlgorithm->gridLayoutInfo_.startMainLineIndex_, 5);
}

/**
 * @tc.name: GridScrollWithOptions003
 * @tc.desc: change grid columns after scroll
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, GridScrollWithOptions003, TestSize.Level1)
{
    GridLayoutOptions option;
    option.regularSize.rows = 1;
    option.regularSize.columns = 1;
    option.irregularIndexes = { 6, 1, 2, 3, 4, 5 };
    auto onGetIrregularSizeByIndex = [](int32_t index) {
        GridItemSize gridItemSize { 1, 2 };
        return gridItemSize;
    };
    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetLayoutOptions(option);
    CreateFixedItems(10);
    CreateDone(frameNode_);
    pattern_->UpdateStartIndex(3);
    FlushLayoutTask(frameNode_);
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr 1fr 1fr 1fr");
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushLayoutTask(frameNode_);
    auto layoutAlgorithmWrapper = AceType::DynamicCast<LayoutAlgorithmWrapper>(frameNode_->GetLayoutAlgorithm());
    auto layoutAlgorithm =
        AceType::DynamicCast<GridScrollWithOptionsLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    if (AceType::InstanceOf<GridIrregularLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm())) {
        return;
    }
    EXPECT_EQ(layoutAlgorithm->GetCrossStartAndSpanWithUserFunction(3, option, 1), std::make_pair(0, 2));
}

/**
 * @tc.name: GridScrollWithOptions004
 * @tc.desc: change grid columns after scroll, first line has empty position
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, GridScrollWithOptions004, TestSize.Level1)
{
    GridLayoutOptions option;
    option.regularSize.rows = 1;
    option.regularSize.columns = 1;
    option.irregularIndexes = { 6, 1, 2, 3, 4, 5 };
    auto onGetIrregularSizeByIndex = [](int32_t index) {
        GridItemSize gridItemSize { 1, 2 };
        return gridItemSize;
    };
    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetLayoutOptions(option);
    CreateFixedItems(10);
    CreateDone(frameNode_);
    pattern_->UpdateStartIndex(3);
    FlushLayoutTask(frameNode_);
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr 1fr 1fr 1fr 1fr");
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushLayoutTask(frameNode_);
    auto layoutAlgorithmWrapper = AceType::DynamicCast<LayoutAlgorithmWrapper>(frameNode_->GetLayoutAlgorithm());
    auto layoutAlgorithm =
        AceType::DynamicCast<GridScrollWithOptionsLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    if (AceType::InstanceOf<GridIrregularLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm())) {
        return;
    }
    EXPECT_EQ(layoutAlgorithm->GetCrossStartAndSpanWithUserFunction(3, option, 1), std::make_pair(0, 2));
    EXPECT_EQ(layoutAlgorithm->GetCrossStartAndSpanWithUserFunction(2, option, 1), std::make_pair(3, 2));
    EXPECT_EQ(layoutAlgorithm->GetCrossStartAndSpanWithUserFunction(1, option, 1), std::make_pair(1, 2));
}

/**
 * @tc.name: GridScrollWithOptions005
 * @tc.desc: second line full
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, GridScrollWithOptions005, TestSize.Level1)
{
    GridLayoutOptions option;
    option.regularSize.rows = 1;
    option.regularSize.columns = 1;
    option.irregularIndexes = { 6, 1, 2, 3, 4, 5 };
    auto onGetIrregularSizeByIndex = [](int32_t index) {
        GridItemSize gridItemSize { 1, 2 };
        return gridItemSize;
    };
    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    model.SetLayoutOptions(option);
    CreateFixedItems(10);
    CreateDone(frameNode_);
    auto layoutAlgorithmWrapper = AceType::DynamicCast<LayoutAlgorithmWrapper>(frameNode_->GetLayoutAlgorithm());
    auto layoutAlgorithm =
        AceType::DynamicCast<GridScrollWithOptionsLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    if (AceType::InstanceOf<GridIrregularLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm())) {
        return;
    }
    EXPECT_EQ(layoutAlgorithm->GetCrossStartAndSpanWithUserFunction(3, option, 1), std::make_pair(2, 2));
    EXPECT_EQ(layoutAlgorithm->GetCrossStartAndSpanWithUserFunction(2, option, 1), std::make_pair(0, 2));
    EXPECT_EQ(layoutAlgorithm->GetCrossStartAndSpanWithUserFunction(1, option, 1), std::make_pair(1, 2));
}

/**
 * @tc.name: GridScrollWithOptions006
 * @tc.desc: first irregular item in new line
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, GridScrollWithOptions006, TestSize.Level1)
{
    GridLayoutOptions option;
    option.regularSize.rows = 1;
    option.regularSize.columns = 1;
    option.irregularIndexes = { 6, 3, 4, 5 };
    auto onGetIrregularSizeByIndex = [](int32_t index) {
        GridItemSize gridItemSize { 1, 2 };
        return gridItemSize;
    };
    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    model.SetLayoutOptions(option);
    CreateFixedItems(10);
    CreateDone(frameNode_);
    auto layoutAlgorithmWrapper = AceType::DynamicCast<LayoutAlgorithmWrapper>(frameNode_->GetLayoutAlgorithm());
    auto layoutAlgorithm =
        AceType::DynamicCast<GridScrollWithOptionsLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    if (AceType::InstanceOf<GridIrregularLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm())) {
        return;
    }
    EXPECT_EQ(layoutAlgorithm->GetCrossStartAndSpanWithUserFunction(4, option, 1), std::make_pair(2, 2));
}

/**
 * @tc.name: SearchIrregularFocusableChildInScroll001
 * @tc.desc: Test the function when the gridItem cannot be focused
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, SearchIrregularFocusableChildInScroll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create gridItems with irregular shape in scroll grid.
     */
    GridLayoutOptions option;
    option.regularSize.rows = 1;
    option.regularSize.columns = 1;
    option.irregularIndexes = { 6, 1, 2, 3, 4, 5, 0 };
    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr 1fr 1fr");
    model.SetLayoutOptions(option);
    CreateFixedItems(10);
    CreateDone(frameNode_);

    /**
     * @tc.steps: step2. Find target child with specified index parameters.
     * @tc.expected: Can not find the target focus child.
     */
    int32_t tarMainIndex = 1;
    int32_t tarCrossIndex = 1;
    auto IrregularFocusableChild = pattern_->SearchIrregularFocusableChild(tarMainIndex, tarCrossIndex);
    RefPtr<FocusHub> result = IrregularFocusableChild.Upgrade();
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: SearchIrregularFocusableChildInScroll002
 * @tc.desc: Test the function when the gridItem can be focused
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, SearchIrregularFocusableChildInScroll002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create gridItems with irregular shape in scroll grid.
     */
    GridLayoutOptions option;
    option.regularSize.rows = 1;
    option.regularSize.columns = 1;
    option.irregularIndexes = { 6, 1, 2, 3, 4, 5, 0 };
    auto onGetIrregularSizeByIndex = [](int32_t index) {
        GridItemSize gridItemSize { 1, 2 };
        return gridItemSize;
    };
    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr 1fr 1fr");
    model.SetLayoutOptions(option);
    CreateFocusableGridItems(10, ITEM_WIDTH, ITEM_HEIGHT);
    CreateDone(frameNode_);

    /**
     * @tc.steps: step2. Find target child with specified index parameters.
     * @tc.expected: Can not find the target focus child.
     */
    int32_t tarMainIndex = 1;
    int32_t tarCrossIndex = 1;
    auto IrregularFocusableChild = pattern_->SearchIrregularFocusableChild(tarMainIndex, tarCrossIndex);
    RefPtr<FocusHub> result = IrregularFocusableChild.Upgrade();
    EXPECT_EQ(result, nullptr);

    /**
     * @tc.steps: step3. Call the function when isLeftStep_ is true.
     * @tc.expected: Can find the target focus child.
     */
    pattern_->isLeftStep_ = true;
    IrregularFocusableChild = pattern_->SearchIrregularFocusableChild(tarMainIndex, tarCrossIndex);
    result = IrregularFocusableChild.Upgrade();
    EXPECT_NE(result, nullptr);
    pattern_->isLeftStep_ = false;

    /**
     * @tc.steps: step4. Call the function when isRightStep_ is true.
     * @tc.expected: Can find the target focus child.
     */
    tarCrossIndex = 0;
    pattern_->isRightStep_ = true;
    IrregularFocusableChild = pattern_->SearchIrregularFocusableChild(tarMainIndex, tarCrossIndex);
    result = IrregularFocusableChild.Upgrade();
    EXPECT_NE(result, nullptr);
    pattern_->isRightStep_ = false;

    /**
     * @tc.steps: step5. Call the function when isUpStep_ is true.
     * @tc.expected: Can find the target focus child.
     */
    pattern_->isUpStep_ = true;
    IrregularFocusableChild = pattern_->SearchIrregularFocusableChild(tarMainIndex, tarCrossIndex);
    result = IrregularFocusableChild.Upgrade();
    EXPECT_NE(result, nullptr);
    pattern_->isUpStep_ = false;

    /**
     * @tc.steps: step6. Call the function when isDownStep_ is true.
     * @tc.expected: Can find the target focus child.
     */
    pattern_->isDownStep_ = true;
    IrregularFocusableChild = pattern_->SearchIrregularFocusableChild(tarMainIndex, tarCrossIndex);
    result = IrregularFocusableChild.Upgrade();
    EXPECT_NE(result, nullptr);
    pattern_->isDownStep_ = false;

    /**
     * @tc.steps: step7. Call the function when isLeftEndStep_ is true.
     * @tc.expected: Can find the target focus child.
     */
    pattern_->isLeftEndStep_ = true;
    IrregularFocusableChild = pattern_->SearchIrregularFocusableChild(tarMainIndex, tarCrossIndex);
    result = IrregularFocusableChild.Upgrade();
    EXPECT_NE(result, nullptr);
    pattern_->isLeftEndStep_ = false;

    /**
     * @tc.steps: step8. Call the function when isRightEndStep_ is true.
     * @tc.expected: Can find the target focus child.
     */
    pattern_->isRightEndStep_ = true;
    IrregularFocusableChild = pattern_->SearchIrregularFocusableChild(tarMainIndex, tarCrossIndex);
    result = IrregularFocusableChild.Upgrade();
    EXPECT_NE(result, nullptr);
    pattern_->isRightEndStep_ = false;
}

/**
 * @tc.name: GridPattern_GetItemRect001
 * @tc.desc: Test the GetItemRect function of Grid.
 * @tc.type: FUNCgetitemre
 */
HWTEST_F(GridOptionLayoutTestNg, GridPattern_GetItemRect001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Init Grid then slide Grid by Scroller.
     */
    GridLayoutOptions option;
    option.regularSize.rows = 1;
    option.regularSize.columns = 1;
    option.irregularIndexes = { 1, 3 };
    auto onGetIrregularSizeByIndex = [](int32_t index) {
        GridItemSize gridItemSize { 1, 2 };
        return gridItemSize;
    };
    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(option);
    CreateGridItems(10, -2, ITEM_HEIGHT);
    CreateDone(frameNode_);
    pattern_->UpdateStartIndex(3, ScrollAlign::START);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step2. Get invalid GridItem Rect.
     * @tc.expected: Return 0 when input invalid index.
     */
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(-1), Rect()));
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(2), Rect()));
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(10), Rect()));

    /**
     * @tc.steps: step3. Get valid GridItem Rect.
     * @tc.expected: Return actual Rect when input valid index.
     */
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(3), Rect(0, 0, GRID_WIDTH, ITEM_HEIGHT)));
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(4), Rect(0, ITEM_HEIGHT, GRID_WIDTH / 2, ITEM_HEIGHT)));
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(7), Rect(GRID_WIDTH / 2, ITEM_HEIGHT * 2, GRID_WIDTH / 2, ITEM_HEIGHT)));

    /**
     * @tc.steps: step4. Slide Grid by Scroller.
     */
    UpdateCurrentOffset(ITEM_HEIGHT + ITEM_HEIGHT / 2);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step5. Get invalid GridItem Rect.
     * @tc.expected: Return 0 when input invalid index.
     */
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(-1), Rect()));
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(0), Rect()));
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(10), Rect()));

    /**
     * @tc.steps: step6. Get valid GridItem Rect.
     * @tc.expected: Return actual Rect when input valid index.
     */
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(1), Rect(0, -ITEM_HEIGHT / 2, GRID_WIDTH, ITEM_HEIGHT)));
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(2), Rect(0, ITEM_HEIGHT / 2, GRID_WIDTH / 2, ITEM_HEIGHT)));
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(3), Rect(0, ITEM_HEIGHT + ITEM_HEIGHT / 2, GRID_WIDTH, ITEM_HEIGHT)));
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(5),
        Rect(GRID_WIDTH / 2, ITEM_HEIGHT * 2 + ITEM_HEIGHT / 2, GRID_WIDTH / 2, ITEM_HEIGHT)));
}

/**
 * @tc.name: LayoutUtils::GetItemSize001
 * @tc.desc: Test LayoutUtils::GetItemSize
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, GetItemSize001, TestSize.Level1)
{
    GridLayoutOptions option;
    option.irregularIndexes = {
        0, // [2 x 1]
        1, // [1 x 2]
        2  // [2 x 1]
    };
    auto onGetIrregularSizeByIndex = [](int32_t index) -> GridItemSize {
        if (index == 1) {
            return { .rows = 2, .columns = 1 };
        }
        return { .rows = 1, .columns = 2 };
    };

    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(option);
    CreateDone(frameNode_);

    GridLayoutInfo info;
    auto* wrapper = AceType::RawPtr(frameNode_);

    info.crossCount_ = 2;
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 0).rows, 1);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 0).columns, 2);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 1).rows, 2);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 1).columns, 1);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 2).rows, 1);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 2).columns, 2);

    info.axis_ = Axis::HORIZONTAL;
    // rows and columns should be flipped when horizontal
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 0).rows, 2);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 0).columns, 1);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 1).rows, 1);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 1).columns, 2);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 2).rows, 2);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 2).columns, 1);
}

/**
 * @tc.name: LayoutUtils::GetItemSize002
 * @tc.desc: Test LayoutUtils::GetItemSize with null callback
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, GetItemSize002, TestSize.Level1)
{
    GridLayoutOptions option;
    option.irregularIndexes = {
        0, // [2 x 1]
        1, // [1 x 2]
        2  // [2 x 1]
    };
    option.getSizeByIndex = nullptr;

    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(option);
    CreateGridItems(3, ITEM_WIDTH, NULL_VALUE, GridItemStyle::NONE);
    CreateDone(frameNode_);

    GridLayoutInfo info;
    auto* wrapper = AceType::RawPtr(frameNode_);
    info.crossCount_ = 3;

    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 0).rows, 1);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 0).columns, 3);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 1).rows, 1);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 1).columns, 3);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 2).rows, 1);
    EXPECT_EQ(GridLayoutUtils::GetItemSize(&info, wrapper, 2).columns, 3);
}

/**
 * @tc.name: GridLayout005
 * @tc.desc: Test GridLayoutAlgorithm for coverage
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, GridLayout005, TestSize.Level1)
{
    GridLayoutOptions option;
    option.regularSize.rows = 1;
    option.regularSize.columns = 1;
    option.irregularIndexes = { 6, 1, 2, 3, 4, 5 };
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetLayoutOptions(option);
    CreateFixedItems(10);
    CreateDone(frameNode_);

    /**
     * @tc.steps: step2.call GetItemRect
     * @tc.expected: The GetItemRect is crrect
     */
    GridItemRect retItemRect;
    auto pattern = frameNode_->GetPattern<GridPattern>();
    auto algorithm = AceType::MakeRefPtr<GridLayoutAlgorithm>(GridLayoutInfo {}, 2, 5);
    auto childLayoutProperty = GetChildLayoutProperty<GridItemLayoutProperty>(frameNode_, 0);
    ASSERT_NE(layoutProperty_, nullptr);
    ASSERT_NE(childLayoutProperty, nullptr);
    algorithm->GetItemRect(layoutProperty_, childLayoutProperty, 0);
    EXPECT_EQ(retItemRect.rowStart, -1);
    EXPECT_EQ(retItemRect.rowSpan, 1);
    EXPECT_EQ(retItemRect.columnStart, -1);
    EXPECT_EQ(retItemRect.columnSpan, 1);
}

/**
 * @tc.name: GetEndOffset004
 * @tc.desc: test EndOffset when content < viewport
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, GetEndOffset004, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetLayoutOptions({ .irregularIndexes = { 1, 5 } });
    model.SetColumnsGap(Dimension { 5.0f });
    model.SetRowsGap(Dimension { 5.0f });
        CreateFixedHeightItems(6, 100.0f);
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
        // make content smaller than viewport
        ViewAbstract::SetHeight(CalcLength(700.0f));
    CreateDone(frameNode_);
    auto& info = pattern_->gridLayoutInfo_;
    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    // line height + gap = 105
    for (int i = 0; i < 160; ++i) {
        UpdateCurrentOffset(-50.0f);
        EXPECT_EQ(pattern_->GetEndOffset(), info.startMainLineIndex_ * 105.0f);
    }
    EXPECT_LE(info.currentOffset_, -1000.0f);
    EXPECT_GE(info.startMainLineIndex_, 3);
}

/**
 * @tc.name: TestChildrenUpdate001
 * @tc.desc: Test updating existing children and adding children
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, TestChildrenUpdate001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        CreateFixedHeightItems(2, 100.0f);
    model.SetLayoutOptions({});
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateDone(frameNode_);
    auto& info = pattern_->gridLayoutInfo_;
    pattern_->scrollableEvent_->scrollable_->isTouching_ = true;
    EXPECT_FALSE(pattern_->irregular_);
    for (int i = 0; i < 2; ++i) {
        frameNode_->ChildrenUpdatedFrom(i);
        frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        FlushLayoutTask(frameNode_);
        EXPECT_EQ(GetChildOffset(frameNode_, 0), OffsetF(0, 0));
        EXPECT_EQ(GetChildOffset(frameNode_, 1), OffsetF(GRID_WIDTH / 4.0f, 0));
        const decltype(info.gridMatrix_) cmp = { { 0, { { 0, 0 }, { 1, 1 } } } };
        EXPECT_EQ(info.gridMatrix_, cmp);
        EXPECT_EQ(info.lineHeightMap_.size(), 1);
    }

    AddFixedHeightItems(3, 100.0f);
    frameNode_->ChildrenUpdatedFrom(2);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushLayoutTask(frameNode_);
    const decltype(info.gridMatrix_) cmp2 = { { 0, { { 0, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 } } }, { 1, { { 0, 4 } } } };
    EXPECT_EQ(info.gridMatrix_, cmp2);
    EXPECT_EQ(info.lineHeightMap_.size(), 2);
    EXPECT_EQ(GetChildOffset(frameNode_, 4), OffsetF(0.0f, 100.0f));
}

/**
 * @tc.name: GridLayoutTest001
 * @tc.desc: Test Grid Measure with GrdiLayoutOptions
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, GridLayoutTest001, TestSize.Level1)
{
    GridLayoutOptions option;
    option.regularSize = { 1, 1 };
    int32_t gridItems[4][4] = { { 0, 0, 1, 1 }, { 0, 1, 1, 1 }, { 0, 2, 1, 1 }, { 1, 0, 1, 1 } };

    auto onGetRectByIndex = [gridItems](int32_t index) {
        GridItemRect itemRect;
        itemRect.rowStart = gridItems[index][0];
        itemRect.columnStart = gridItems[index][1];
        itemRect.rowSpan = gridItems[index][2];
        itemRect.columnSpan = gridItems[index][3];
        return itemRect;
    };
    option.getRectByIndex = std::move(onGetRectByIndex);
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetRowsTemplate("1fr 1fr 1fr");
    model.SetLayoutOptions(option);
    model.SetColumnsGap(Dimension(COL_GAP));
    model.SetRowsGap(Dimension(ROW_GAP));
    CreateFixedItems(4);
    CreateDone(frameNode_);

    EXPECT_EQ(pattern_->GetGridLayoutInfo().startMainLineIndex_, 0);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().endMainLineIndex_, 1);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, 0);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().endIndex_, 3);
}

/**
 * @tc.name: GridLayout006
 * @tc.desc: Test GridLayoutAlgorithm for coverage
 * @tc.type: FUNC
 */
HWTEST_F(GridOptionLayoutTestNg, GridLayout006, TestSize.Level1)
{
    GridLayoutOptions option;
    option.regularSize.rows = 1;
    option.regularSize.columns = 1;
    option.irregularIndexes = { 6, 1, 2, 3, 4, 5 };
    option.getRectByIndex = [](int32_t index) {
        GridItemRect tmpItemRect;
        tmpItemRect.rowStart = 1;
        tmpItemRect.rowSpan = 20;
        tmpItemRect.columnStart = 1;
        tmpItemRect.columnSpan = 20;
        return tmpItemRect;
    };
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetLayoutOptions(option);
    CreateFixedItems(10);
    CreateDone(frameNode_);
    pattern_->UpdateStartIndex(3);
    FlushLayoutTask(frameNode_);
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr 1fr 1fr 1fr");

    /**
     * @tc.steps: step2.call GetItemRect
     * @tc.expected: The GetItemRect is crrect
     */
    GridItemRect retItemRect;
    auto pattern = frameNode_->GetPattern<GridPattern>();
    auto algorithm = AceType::MakeRefPtr<GridLayoutAlgorithm>(GridLayoutInfo {}, 2, 5);
    auto childLayoutProperty = GetChildLayoutProperty<GridItemLayoutProperty>(frameNode_, 0);
    ASSERT_NE(layoutProperty_, nullptr);
    ASSERT_NE(childLayoutProperty, nullptr);
    retItemRect = algorithm->GetItemRect(layoutProperty_, childLayoutProperty, 0);
    EXPECT_EQ(retItemRect.rowStart, 1);
    EXPECT_EQ(retItemRect.rowSpan, 20);
    EXPECT_EQ(retItemRect.columnStart, 1);
    EXPECT_EQ(retItemRect.columnSpan, 20);
}
} // namespace OHOS::Ace::NG
