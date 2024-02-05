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
#include "irregular_matrices.h"

#include "core/components/scroll/scroll_controller_base.h"
#include "core/components_ng/pattern/grid/grid_layout_info.h"
#include "core/components_ng/pattern/grid/irregular/grid_irregular_filler.h"
#include "core/components_ng/pattern/grid/irregular/grid_irregular_layout_algorithm.h"
#include "core/components_ng/pattern/grid/irregular/grid_layout_range_solver.h"

namespace OHOS::Ace::NG {
class GridIrregularLayoutTest : public GridTestNg {};

/**
 * @tc.name: IrregularFiller::AdvancePos001
 * @tc.desc: Test IrregularFiller::AdvancePos
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, AdvancePos001, TestSize.Level1)
{
    // empty matrix
    GridLayoutInfo info;
    info.crossCount_ = 2;
    GridIrregularFiller filler(&info, nullptr);
    EXPECT_FALSE(filler.AdvancePos());

    filler.posX_ = 1;
    filler.posY_ = 0;
    EXPECT_FALSE(filler.AdvancePos());
    EXPECT_EQ(filler.posX_, 0);
    EXPECT_EQ(filler.posY_, 1);

    // init matrix
    info.gridMatrix_[0][0] = 1;
    info.gridMatrix_[0][1] = -1;
    info.gridMatrix_[1][0] = -1;
    EXPECT_FALSE(filler.AdvancePos());
    EXPECT_EQ(filler.posX_, 1);
    EXPECT_EQ(filler.posY_, 1);

    // reset pos and make [1][1] available
    filler.posX_ = 0;
    filler.posY_ = 1;
    info.gridMatrix_[1][1] = -1;
    EXPECT_TRUE(filler.AdvancePos());
}

/**
 * @tc.name: IrregularFiller::FindNextItem001
 * @tc.desc: Test IrregularFiller::FindNextItem
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, FindNextItem001, TestSize.Level1)
{
    // empty matrix
    GridLayoutInfo info;
    info.crossCount_ = 2;
    {
        GridIrregularFiller filler(&info, nullptr);

        EXPECT_FALSE(filler.FindNextItem(0));
    }

    info.gridMatrix_[0][0] = 1;
    info.gridMatrix_[0][1] = 2;
    info.gridMatrix_[1][0] = 3;
    info.gridMatrix_[1][1] = -1;
    {
        GridIrregularFiller filler(&info, nullptr);

        EXPECT_TRUE(filler.FindNextItem(1));
        EXPECT_EQ(filler.posX_, 0);
        EXPECT_EQ(filler.posY_, 0);

        EXPECT_TRUE(filler.FindNextItem(2));
        EXPECT_EQ(filler.posX_, 1);
        EXPECT_EQ(filler.posY_, 0);

        EXPECT_TRUE(filler.FindNextItem(3));
        EXPECT_EQ(filler.posX_, 0);
        EXPECT_EQ(filler.posY_, 1);

        EXPECT_FALSE(filler.FindNextItem(4));
    }

    info.gridMatrix_[0][1] = -1;
    info.gridMatrix_[1][0] = 2;
    info.gridMatrix_[1].erase(1);
    {
        GridIrregularFiller filler(&info, nullptr);

        EXPECT_TRUE(filler.FindNextItem(1));
        EXPECT_EQ(filler.posX_, 0);
        EXPECT_EQ(filler.posY_, 0);

        EXPECT_TRUE(filler.FindNextItem(2));
        EXPECT_EQ(filler.posX_, 0);
        EXPECT_EQ(filler.posY_, 1);

        EXPECT_FALSE(filler.FindNextItem(3));
        EXPECT_EQ(filler.posX_, 1);
        EXPECT_EQ(filler.posY_, 1);
    }
}

/**
 * @tc.name: IrregularFiller::UpdateLength001
 * @tc.desc: Test IrregularFiller::UpdateLength
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, UpdateLength001, TestSize.Level1)
{
    GridLayoutInfo info;
    info.lineHeightMap_[0] = 50.0f;
    info.lineHeightMap_[1] = 30.0f;

    GridIrregularFiller filler(&info, nullptr);
    float len = -5.0f;
    int32_t row = 0;
    EXPECT_TRUE(filler.UpdateLength(len, 50.0f, row, 2, 5.0f));
    EXPECT_EQ(len, 50.0f);

    len = -5.0f;
    row = 0;
    EXPECT_FALSE(filler.UpdateLength(len, 100.0f, row, 2, 5.0f));
    EXPECT_EQ(len, 85.0f);

    info.lineHeightMap_[2] = 50.0f;
    row = 2;
    EXPECT_TRUE(filler.UpdateLength(len, 100.0f, row, 3, 10.0f));
    EXPECT_EQ(len, 85.0f + 50.0f + 10.0f);

    len = 85.0f;
    row = 2;
    EXPECT_FALSE(filler.UpdateLength(len, 200.0f, row, 3, 10.0f));
    EXPECT_EQ(len, 85.0f + 50.0f + 10.0f);
}

/**
 * @tc.name: IrregularFiller::FillOne001
 * @tc.desc: Test IrregularFiller::FillOne
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, FillOne001, TestSize.Level1)
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
    Create([option](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetLayoutOptions(option);
    });

    GridLayoutInfo info;
    info.crossCount_ = 2;
    GridIrregularFiller filler(&info, AceType::RawPtr(frameNode_));

    filler.FillOne(0);
    EXPECT_EQ(info.gridMatrix_.at(0).at(0), 0);
    EXPECT_EQ(info.gridMatrix_.at(0).at(1), 0);
    EXPECT_EQ(filler.posX_, 0);
    EXPECT_EQ(filler.posY_, 0);

    filler.FillOne(1);
    EXPECT_EQ(info.gridMatrix_.at(1).at(0), 1);
    EXPECT_EQ(info.gridMatrix_.at(1).size(), 1);
    EXPECT_EQ(info.gridMatrix_.at(2).at(0), -1);
    EXPECT_EQ(info.gridMatrix_.at(2).size(), 1);
    EXPECT_TRUE(info.gridMatrix_.find(3) == info.gridMatrix_.end());
    EXPECT_EQ(filler.posX_, 0);
    EXPECT_EQ(filler.posY_, 1);

    filler.FillOne(2);
    EXPECT_EQ(info.gridMatrix_.at(3).at(0), 2);
    EXPECT_EQ(info.gridMatrix_.at(3).at(1), -2);
    EXPECT_TRUE(info.gridMatrix_.find(4) == info.gridMatrix_.end());
    EXPECT_EQ(filler.posX_, 0);
    EXPECT_EQ(filler.posY_, 3);
}

/**
 * @tc.name: IrregularFiller::FillOne002
 * @tc.desc: Test IrregularFiller::FillOne with 3 columns
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, FillOne002, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo10());
    });

    GridLayoutInfo info;
    info.crossCount_ = 3;
    GridIrregularFiller filler(&info, AceType::RawPtr(frameNode_));

    for (int i = 0; i < 8; ++i) {
        filler.FillOne(i);
    }

    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_10);
}

/**
 * @tc.name: IrregularFiller::MeasureItem001
 * @tc.desc: Test IrregularFiller::MeasureItem
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, MeasureItem001, TestSize.Level1)
{
    GridLayoutOptions option;
    option.irregularIndexes = {
        0,
    };
    auto onGetIrregularSizeByIndex = [](int32_t index) -> GridItemSize { return { .columns = 1, .rows = 2 }; };
    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    Create([option](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetLayoutOptions(option);
        CreateRowItem(10);
    });

    GridLayoutInfo info;
    info.crossCount_ = 4;
    GridIrregularFiller filler(&info, AceType::RawPtr(frameNode_));

    info.endIndex_ = 0;

    GridIrregularFiller::FillParameters params {
        .crossLens = { 50.0f, 50.0f, 100.0f, 100.0f }, .crossGap = 5.0f, .mainGap = 1.0f
    };
    filler.MeasureItem(params, 0, 0, 0);

    EXPECT_TRUE(info.lineHeightMap_.find(0) != info.lineHeightMap_.end());
    EXPECT_TRUE(info.lineHeightMap_.find(1) != info.lineHeightMap_.end());
    auto child = frameNode_->GetChildByIndex(0);
    ASSERT_TRUE(child);
    auto constraint = *child->GetGeometryNode()->GetParentLayoutConstraint();
    EXPECT_EQ(constraint.maxSize.Width(), 50.0f);
    EXPECT_EQ(*constraint.selfIdealSize.Width(), 50.0f);
    EXPECT_EQ(constraint.percentReference.Width(), 50.0f);
}

/**
 * @tc.name: IrregularFiller::MeasureItem002
 * @tc.desc: Test IrregularFiller::MeasureItem
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, MeasureItem002, TestSize.Level1)
{
    GridLayoutOptions option;
    option.irregularIndexes = {
        0,
    };
    Create([option](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetLayoutOptions(option);
        CreateRowItem(10);
    });

    GridLayoutInfo info;
    info.crossCount_ = 4;
    GridIrregularFiller filler(&info, AceType::RawPtr(frameNode_));

    info.endIndex_ = 0;
    filler.posY_ = 0;

    GridIrregularFiller::FillParameters params {
        .crossLens = { 50.0f, 50.0f, 100.0f, 100.0f }, .crossGap = 5.0f, .mainGap = 1.0f
    };
    filler.MeasureItem(params, 0, 0, 0);

    EXPECT_TRUE(info.lineHeightMap_.find(0) != info.lineHeightMap_.end());
    EXPECT_TRUE(info.lineHeightMap_.find(1) == info.lineHeightMap_.end());
    auto child = frameNode_->GetChildByIndex(0);
    ASSERT_TRUE(child);
    auto constraint = *child->GetGeometryNode()->GetParentLayoutConstraint();
    EXPECT_EQ(constraint.maxSize.Width(), 315.0f);
    EXPECT_EQ(*constraint.selfIdealSize.Width(), 315.0f);
    EXPECT_EQ(constraint.percentReference.Width(), 315.0f);
}

/**
 * @tc.name: IrregularFiller::Fill001
 * @tc.desc: Test IrregularFiller::Fill
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, Fill001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo9());
        CreateRowItem(10);
    });

    GridLayoutInfo info;
    info.crossCount_ = 3;
    info.childrenCount_ = 10;
    GridIrregularFiller filler(&info, AceType::RawPtr(frameNode_));

    auto res = filler.Fill(
        { .crossLens = { 50.0f, 50.0f, 100.0f }, .crossGap = 5.0f, .mainGap = 1.0f }, 1000.0f, 0);

    EXPECT_EQ(res.length, 6.0f);

    EXPECT_EQ(res.endIndex, 9);
    EXPECT_EQ(res.endMainLineIndex, 6);

    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_9);
}

/**
 * @tc.name: IrregularFiller::Fill002
 * @tc.desc: Test IrregularFiller::Fill
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, Fill002, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo11());
        CreateRowItem(10);
    });

    GridLayoutInfo info;
    info.crossCount_ = 3;
    info.childrenCount_ = 10;
    GridIrregularFiller filler(&info, AceType::RawPtr(frameNode_));

    auto res = filler.Fill(
        { .crossLens = { 50.0f, 50.0f, 100.0f }, .crossGap = 5.0f, .mainGap = 1.0f }, 1000.0f,  0);

    EXPECT_EQ(res.length, 6.0f);

    EXPECT_EQ(res.endIndex, 9);
    EXPECT_EQ(res.endMainLineIndex, 6);

    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_11);

    // call Fill on an already filled matrix
    res = filler.Fill(
        { .crossLens = { 50.0f, 50.0f, 100.0f }, .crossGap = 5.0f, .mainGap = 1.0f }, 1000.0f, 0);

    EXPECT_EQ(res.length, 6.0f);

    EXPECT_EQ(res.endIndex, 9);
    EXPECT_EQ(res.endMainLineIndex, 6);

    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_11);
}

/**
 * @tc.name: IrregularFiller::Fill003
 * @tc.desc: Test IrregularFiller::Fill
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, Fill003, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo5());
        CreateFixedItem(11);
    });

    GridLayoutInfo info;
    info.crossCount_ = 2;
    info.childrenCount_ = 11;
    GridIrregularFiller filler(&info, AceType::RawPtr(frameNode_));

    auto res = filler.Fill(
        { .crossLens = { 50.0f, 50.0f }, .crossGap = 5.0f, .mainGap = 1.0f }, 600.0f, 0);

    EXPECT_EQ(res.length, 602.0f);

    EXPECT_EQ(res.endIndex, 3);
    EXPECT_EQ(res.endMainLineIndex, 5);

    res = filler.Fill(
        { .crossLens = { 50.0f, 50.0f }, .crossGap = 5.0f, .mainGap = 1.0f }, 1000.0f, 2);

    EXPECT_EQ(res.length, 1004.0f);

    EXPECT_EQ(res.endIndex, 8);
    EXPECT_EQ(res.endMainLineIndex, 10);

    res = filler.Fill(
        { .crossLens = { 50.0f, 50.0f }, .crossGap = 5.0f, .mainGap = 1.0f }, 1000.0f, 7);

    EXPECT_EQ(res.length, 803.0f);

    EXPECT_EQ(res.endIndex, 10);
    EXPECT_EQ(res.endMainLineIndex, 11);
    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_5);
}

/**
 * @tc.name: IrregularFiller::Fill004
 * @tc.desc: Test IrregularFiller::Fill
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, Fill004, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetLayoutOptions(GetOptionDemo2());
        CreateFixedItem(8);
        model.SetColumnsTemplate("1fr 1fr 1fr");
    });

    GridLayoutInfo info;
    info.crossCount_ = 3;
    info.childrenCount_ = 8;
    GridIrregularFiller filler(&info, AceType::RawPtr(frameNode_));

    auto res = filler.Fill(
        { .crossLens = { 50.0f, 50.0f }, .crossGap = 5.0f, .mainGap = 1.0f }, 300.0f, 0);

    EXPECT_EQ(res.length, 401.0f);

    EXPECT_EQ(res.endIndex, 3);
    EXPECT_EQ(res.endMainLineIndex, 2);

    // call Fill on an already filled matrix
    res = filler.Fill(
        { .crossLens = { 50.0f, 50.0f }, .crossGap = 5.0f, .mainGap = 1.0f }, 500.0f, 2);

    EXPECT_EQ(res.length, 602.0f);

    EXPECT_EQ(res.endIndex, 7);
    EXPECT_EQ(res.endMainLineIndex, 4);

    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_2);
}

/**
 * @tc.name: LayoutRangeSolver::AddNextRow001
 * @tc.desc: Test LayoutRangeSolver::AddNextRow
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, AddNextRow001, TestSize.Level1)
{
    GridLayoutOptions option;
    option.irregularIndexes = {
        0, // [1 x 2]
        3, // [2 x 1]
    };
    auto onGetIrregularSizeByIndex = [](int32_t index) -> GridItemSize {
        if (index == 0) {
            return { .rows = 2, .columns = 1 };
        }
        return { .rows = 1, .columns = 2 };
    };

    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    Create([option](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(option);
    });

    GridLayoutInfo info;
    info.crossCount_ = 3;
    info.gridMatrix_ = {
        { 0, { { 0, 0 }, { 1, 1 }, { 2, 2 } } },  // 0 | 1 | 2
        { 1, { { 0, 0 }, { 1, 3 }, { 2, -3 } } }, // 0 | 3 | 3
    };
    info.lineHeightMap_ = { { 0, 50.0f }, { 1, 30.0f } };

    GridLayoutRangeSolver solver(&info, AceType::RawPtr(frameNode_));
    auto res = solver.AddNextRows(5.0f, 0);
    EXPECT_EQ(res.first, 2);
    EXPECT_EQ(res.second, 80.0f + 5.0f); // top line doesn't have main gap
}

/**
 * @tc.name: LayoutRangeSolver::AddNextRows002
 * @tc.desc: Test LayoutRangeSolver::AddNextRows
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, AddNextRows002, TestSize.Level1)
{
    GridLayoutOptions option;
    option.irregularIndexes = {
        0, // [1 x 3]
        3, // [2 x 1]
    };
    auto onGetIrregularSizeByIndex = [](int32_t index) -> GridItemSize {
        if (index == 0) {
            return { .rows = 3, .columns = 1 };
        }
        return { .rows = 1, .columns = 2 };
    };

    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    Create([option](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(option);
    });

    GridLayoutInfo info;
    info.crossCount_ = 3;
    info.gridMatrix_ = {
        { 0, { { 0, 0 }, { 0, 1 }, { 2, 2 } } },  // 0 | 1 | 2
        { 1, { { 0, 0 }, { 1, 3 }, { 2, -3 } } }, // 0 | 3 | 3
        { 2, { { 0, 0 } } },                      // 0 | x | x
    };
    info.lineHeightMap_ = { { 0, 50.0f }, { 1, 60.0f }, { 2, 40.0f } };

    GridLayoutRangeSolver solver(&info, AceType::RawPtr(frameNode_));
    auto res = solver.AddNextRows(5.0f, 0);
    EXPECT_EQ(res.first, 3);
    EXPECT_EQ(res.second, 160.0f);

    // in real scenario, parameter rowIdx = 1 is impossible
    res = solver.AddNextRows(5.0f, 1);
    EXPECT_EQ(res.first, 1);
    EXPECT_EQ(res.second, 65.0f);

    res = solver.AddNextRows(5.0f, 2);
    EXPECT_EQ(res.first, 1);
    EXPECT_EQ(res.second, 45.0f);
}

/**
 * @tc.name: LayoutRangeSolver::SolveForward001
 * @tc.desc: Test LayoutRangeSolver::SolveForward
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, SolveForward001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo2());
    });

    GridLayoutInfo info;
    info.crossCount_ = 3;
    info.gridMatrix_ = MATRIX_DEMO_2;
    info.lineHeightMap_ = { { 0, 20.0f }, { 1, 40.0f }, { 2, 40.0f }, { 3, 10.0f }, { 4, 50.0f }, { 5, 70.0f } };

    GridLayoutRangeSolver solver(&info, AceType::RawPtr(frameNode_));

    info.currentOffset_ = 0.0f;
    info.startMainLineIndex_ = 3;
    auto res = solver.FindStartingRow(1.0f);
    EXPECT_EQ(res.row, 3);
    EXPECT_EQ(res.pos, 0.0f);

    info.currentOffset_ = -20.0f;
    info.startMainLineIndex_ = 0;
    res = solver.FindStartingRow(1.0f);
    EXPECT_EQ(res.row, 0);
    EXPECT_EQ(res.pos, -20.0f);

    info.currentOffset_ = -70.0f;
    info.startMainLineIndex_ = 0;
    res = solver.FindStartingRow(1.0f);
    EXPECT_EQ(res.row, 0);
    EXPECT_EQ(res.pos, -70.0f);

    // startMainLineIndex_ == 1 || startMainLineIndex_ == 2 is impossible.
    // LayoutRangeSolver always finds the first row of irregular items.

    info.currentOffset_ = -11.0f;
    info.startMainLineIndex_ = 3;
    res = solver.FindStartingRow(1.0f);
    EXPECT_EQ(res.row, 4);
    EXPECT_EQ(res.pos, 0.0f);

    info.currentOffset_ = -10.0f;
    info.startMainLineIndex_ = 3;
    res = solver.FindStartingRow(1.0f);
    EXPECT_EQ(res.row, 3);
    EXPECT_EQ(res.pos, -10.0f);

    info.currentOffset_ = -110.0f;
    info.startMainLineIndex_ = 3;
    res = solver.FindStartingRow(1.0f);
    EXPECT_EQ(res.row, 4);
    EXPECT_EQ(res.pos, -99.0f);
}

/**
 * @tc.name: LayoutRangeSolver::CheckMultiRow001
 * @tc.desc: Test LayoutRangeSolver::CheckMultiRow
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, CheckMultiRow001, TestSize.Level1)
{
    GridLayoutOptions option;
    option.irregularIndexes = {
        0, // [2 x 1]
        3, // [3 x 2]
    };
    auto onGetIrregularSizeByIndex = [](int32_t index) -> GridItemSize {
        if (index == 0) {
            return { .rows = 1, .columns = 2 };
        }
        return { .rows = 2, .columns = 3 };
    };

    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    Create([option](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(option);
    });

    GridLayoutInfo info;
    info.crossCount_ = 3;
    info.gridMatrix_ = {
        { 0, { { 0, 0 }, { 0, -1 }, { 2, 2 } } },   // 0 | 0 | 2
        { 1, { { 0, 3 }, { 1, -3 }, { 2, -3 } } },  // 3 | 3 | 3
        { 2, { { 0, -3 }, { 1, -3 }, { 2, -3 } } }, // 3 | 3 | 3
    };

    GridLayoutRangeSolver solver(&info, AceType::RawPtr(frameNode_));
    EXPECT_EQ(solver.CheckMultiRow(2), 2);

    EXPECT_EQ(solver.CheckMultiRow(0), 1);
    EXPECT_EQ(solver.CheckMultiRow(1), 1);
}

/**
 * @tc.name: LayoutRangeSolver::SolveBackward001
 * @tc.desc: Test LayoutRangeSolver::SolveBackward
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, SolveBackward001, TestSize.Level1)
{
    GridLayoutOptions option;
    option.irregularIndexes = {
        0, // [2 x 1]
        3, // [2 x 2]
        4, // [1 x 2]
        6, // [2 x 1]
    };
    auto onGetIrregularSizeByIndex = [](int32_t index) -> GridItemSize {
        if (index == 4) {
            return { .rows = 2, .columns = 1 };
        }
        if (index == 3) {
            return { 2, 2 };
        }
        return { .rows = 1, .columns = 2 };
    };

    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    Create([option](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(option);
    });

    GridLayoutInfo info;
    info.crossCount_ = 3;
    info.gridMatrix_ = {
        { 0, { { 0, 0 }, { 0, 0 }, { 2, 1 } } },   // 0 | 0 | 1
        { 1, { { 0, 2 }, { 1, 3 }, { 2, -3 } } },  // 2 | 3 | 3
        { 2, { { 0, 4 }, { 1, -3 }, { 2, -3 } } }, // 4 | 3 | 3
        { 3, { { 0, -4 }, { 1, 5 } } },            // 4 | 5 | x
        { 4, { { 0, 6 }, { 1, -6 }, { 2, 7 } } },  // 6 | 6 | 7
    };
    info.lineHeightMap_ = { { 0, 50.0f }, { 1, 30.0f }, { 2, 40.0f }, { 3, 30.0f }, { 4, 50.0f } };

    info.currentOffset_ = 20.0f;
    info.startMainLineIndex_ = 4;

    GridLayoutRangeSolver solver(&info, AceType::RawPtr(frameNode_));
    auto res = solver.FindStartingRow(5.0f);
    EXPECT_EQ(res.pos, -60.0f);
    EXPECT_EQ(res.row, 2);

    info.currentOffset_ = 80.0f;
    info.startMainLineIndex_ = 4;

    res = solver.FindStartingRow(5.0f);
    EXPECT_EQ(res.pos, -35.0f);
    EXPECT_EQ(res.row, 1);

    info.currentOffset_ = 200.0f;
    info.startMainLineIndex_ = 4;

    res = solver.FindStartingRow(5.0f);
    EXPECT_EQ(res.pos, 30.0f);
    EXPECT_EQ(res.row, 0);
}

/**
 * @tc.name: LayoutRangeSolver::Solve001
 * @tc.desc: Test LayoutRangeSolver::FindStartingRow when matrix is empty.
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, Solve001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions({});
    });

    GridLayoutInfo info;
    info.crossCount_ = 3;

    info.currentOffset_ = 0.0f;
    info.startMainLineIndex_ = 0;

    GridLayoutRangeSolver solver(&info, AceType::RawPtr(frameNode_));
    auto res = solver.FindStartingRow(5.0f);
    EXPECT_EQ(res.pos, 0.0f);
    EXPECT_EQ(res.row, 0);
}

/**
 * @tc.name: GridIrregularLayout::LayoutChildren001
 * @tc.desc: Test GridIrregularLayout::LayoutChildren
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, LayoutChildren001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        CreateRowItem(10);
    });

    frameNode_->GetGeometryNode()->UpdatePaddingWithBorder(PaddingPropertyF { .left = 5.0f, .top = 3.0f });

    GridLayoutInfo info;
    info.gridMatrix_ = {
        { 0, { { 0, 0 }, { 0, 0 }, { 2, 1 } } }, // 0 | 0 | 1
        { 1, { { 0, 2 }, { 1, 3 }, { 2, 4 } } }, // 2 | 3 | 4
        { 2, { { 0, 5 }, { 1, 6 }, { 2, 7 } } }, // 5 | 6 | 7
        { 3, { { 0, 8 }, { 1, -8 } } },          // 8 | 8 | x
        { 4, { { 0, 9 }, { 1, -9 } } },          // 9 | 9 | x
    };
    info.lineHeightMap_ = { { 0, 20.0f }, { 1, 20.0f }, { 2, 10.0f }, { 3, 15.0f }, { 4, 30.0f } };
    info.crossCount_ = 3;
    info.startMainLineIndex_ = 0;
    info.endMainLineIndex_ = 4;

    auto algorithm = AceType::MakeRefPtr<GridIrregularLayoutAlgorithm>(info);
    algorithm->wrapper_ = AceType::RawPtr(frameNode_);
    algorithm->crossLens_ = { 50.0f, 50.0f, 100.0f };
    algorithm->crossGap_ = 5.0f;
    algorithm->mainGap_ = 1.0f;
    algorithm->LayoutChildren(0.0f);

    EXPECT_EQ(frameNode_->GetChildByIndex(0)->GetGeometryNode()->GetFrameOffset().GetX(), 5.0f);
    EXPECT_EQ(frameNode_->GetChildByIndex(0)->GetGeometryNode()->GetFrameOffset().GetY(), 3.0f);
    EXPECT_EQ(frameNode_->GetChildByIndex(1)->GetGeometryNode()->GetFrameOffset(), OffsetF(115.0f, 3.0f));
    EXPECT_EQ(frameNode_->GetChildByIndex(2)->GetGeometryNode()->GetFrameOffset(), OffsetF(5.0f, 24.0f));
    EXPECT_EQ(frameNode_->GetChildByIndex(3)->GetGeometryNode()->GetFrameOffset(), OffsetF(60.0f, 24.0f));
    EXPECT_EQ(frameNode_->GetChildByIndex(4)->GetGeometryNode()->GetFrameOffset(), OffsetF(115.0f, 24.0f));
    EXPECT_EQ(frameNode_->GetChildByIndex(5)->GetGeometryNode()->GetFrameOffset(), OffsetF(5.0f, 45.0f));
    EXPECT_EQ(frameNode_->GetChildByIndex(6)->GetGeometryNode()->GetFrameOffset(), OffsetF(60.0f, 45.0f));
    EXPECT_EQ(frameNode_->GetChildByIndex(7)->GetGeometryNode()->GetFrameOffset(), OffsetF(115.0f, 45.0f));
    EXPECT_EQ(frameNode_->GetChildByIndex(8)->GetGeometryNode()->GetFrameOffset(), OffsetF(5.0f, 56.0f));
    EXPECT_EQ(frameNode_->GetChildByIndex(9)->GetGeometryNode()->GetFrameOffset(), OffsetF(5.0f, 72.0f));
}

/**
 * @tc.name: GridIrregularLayout::Measure001
 * @tc.desc: Test GridIrregularLayout::Measure with offset
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, Measure001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo11());
        model.SetColumnsGap(Dimension { 5.0f });
        model.SetRowsGap(Dimension { 1.0f });
        CreateRowItem(10);
    });
    LayoutConstraintF constraint { .maxSize = { 610.0f, 600.0f }, .percentReference = { 610.0f, 600.0f } };
    layoutProperty_->layoutConstraint_ = constraint;

    auto algorithm = AceType::MakeRefPtr<GridIrregularLayoutAlgorithm>(GridLayoutInfo {});
    algorithm->gridLayoutInfo_.currentOffset_ = 0.0f;
    algorithm->gridLayoutInfo_.childrenCount_ = 10;
    algorithm->Measure(AceType::RawPtr(frameNode_));

    std::vector<float> cmp = { 200.0f, 200.0f, 200.0f };
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Width(), 610.0f);
    EXPECT_EQ(algorithm->crossLens_, cmp);

    const auto& info = algorithm->gridLayoutInfo_;
    EXPECT_EQ(algorithm->mainGap_, 1.0f);
    EXPECT_EQ(algorithm->crossGap_, 5.0f);
    EXPECT_EQ(info.startMainLineIndex_, 0);
    EXPECT_EQ(info.endMainLineIndex_, 6);
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 9);

    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_11);

    algorithm->gridLayoutInfo_.currentOffset_ = 5.0f;
    algorithm->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_11);
    EXPECT_EQ(info.startMainLineIndex_, 0);
    EXPECT_EQ(info.endMainLineIndex_, 6);
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 9);
}

/**
 * @tc.name: GridIrregularLayout::MeasureJump001
 * @tc.desc: Test GridIrregularLayout::Measure with jump index
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, MeasureJump001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo11());
        model.SetColumnsGap(Dimension { 5.0f });
        model.SetRowsGap(Dimension { 1.0f });
        CreateFixedItem(10);
    });
    LayoutConstraintF constraint { .maxSize = { 610.0f, 600.0f }, .percentReference = { 610.0f, 600.0f } };
    layoutProperty_->layoutConstraint_ = constraint;

    auto algorithm = AceType::MakeRefPtr<GridIrregularLayoutAlgorithm>(GridLayoutInfo {});
    auto& info = algorithm->gridLayoutInfo_;
    info.jumpIndex_ = 7;
    info.scrollAlign_ = ScrollAlign::AUTO;
    info.childrenCount_ = 10;
    algorithm->Measure(AceType::RawPtr(frameNode_));

    std::vector<float> cmp = { 200.0f, 200.0f, 200.0f };
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Width(), 610.0f);
    EXPECT_EQ(algorithm->crossLens_, cmp);

    EXPECT_EQ(algorithm->mainGap_, 1.0f);
    EXPECT_EQ(algorithm->crossGap_, 5.0f);

    EXPECT_EQ(info.endMainLineIndex_, 6);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::END);
    EXPECT_EQ(info.jumpIndex_, EMPTY_JUMP_INDEX);
    EXPECT_EQ(info.endIndex_, 9);

    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_11);

    info.jumpIndex_ = 6;
    info.scrollAlign_ = ScrollAlign::END;
    algorithm->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.jumpIndex_, EMPTY_JUMP_INDEX);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::END);
    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_11);
    EXPECT_EQ(info.endMainLineIndex_, 5);
    EXPECT_EQ(info.endIndex_, 6);
}

/**
 * @tc.name: GridIrregularLayout::MeasureTarget001
 * @tc.desc: Test GridIrregularLayout::Measure with target index
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, MeasureTarget001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo5());
        model.SetColumnsGap(Dimension { 5.0f });
        model.SetRowsGap(Dimension { 1.0f });
        CreateFixedItem(11);
    });
    LayoutConstraintF constraint { .maxSize = { 610.0f, 600.0f }, .percentReference = { 610.0f, 600.0f } };
    layoutProperty_->layoutConstraint_ = constraint;

    auto algorithm = AceType::MakeRefPtr<GridIrregularLayoutAlgorithm>(GridLayoutInfo {});
    auto& info = algorithm->gridLayoutInfo_;
    info.childrenCount_ = 11;

    info.targetIndex_ = 10;
    algorithm->Measure(AceType::RawPtr(frameNode_));

    EXPECT_EQ(info.crossCount_, 2);
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.startMainLineIndex_, 0);
    EXPECT_EQ(info.endMainLineIndex_, 5);
    EXPECT_EQ(info.endIndex_, 3);
    EXPECT_EQ(info.lineHeightMap_.size(), 12);
    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_5);

    info.lineHeightMap_.erase(info.lineHeightMap_.begin(), info.lineHeightMap_.find(7));
    info.startIndex_ = 5;
    info.startMainLineIndex_ = 7;
    info.endIndex_ = 10;
    info.endMainLineIndex_ = 11;

    info.targetIndex_ = 2;
    algorithm->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_5);
    EXPECT_EQ(info.lineHeightMap_.size(), 11);
    EXPECT_EQ(info.endMainLineIndex_, 10);
    EXPECT_EQ(info.endIndex_, 8);
    EXPECT_EQ(info.startIndex_, 5);
    EXPECT_EQ(info.startMainLineIndex_, 7);
}

/**
 * @tc.name: GridIrregularLayout::Layout001
 * @tc.desc: Test GridIrregularLayout::Layout
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, Layout001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        CreateRowItem(10);
    });
    frameNode_->GetGeometryNode()->UpdatePaddingWithBorder(PaddingPropertyF { .left = 1.0f, .top = 1.0f });
    frameNode_->GetGeometryNode()->SetFrameSize(SizeF { 200.0f, 500.0f });

    auto algorithm = AceType::MakeRefPtr<GridIrregularLayoutAlgorithm>(GridLayoutInfo {});
    algorithm->crossLens_ = { 50.0f, 50.0f, 50.0f };
    auto& info = algorithm->gridLayoutInfo_;
    info.gridMatrix_ = {
        { 0, { { 0, 0 }, { 1, 0 }, { 2, 0 } } },  // 0 | 0 | 0
        { 1, { { 0, 2 }, { 1, 3 }, { 2, 4 } } },  // 2 | 3 | 4
        { 2, { { 0, 5 }, { 1, 6 }, { 2, 7 } } },  // 5 | 6 | 7
        { 3, { { 0, 8 }, { 1, -6 }, { 2, 9 } } }, // 8 | 6 | 9
    };
    info.lineHeightMap_ = { { 0, 20.0f }, { 1, 20.0f }, { 2, 10.0f }, { 3, 15.0f } };
    info.childrenCount_ = 10;
    info.crossCount_ = 3;
    info.startMainLineIndex_ = 0;
    info.endMainLineIndex_ = 3;
    info.startIndex_ = 0;
    info.endIndex_ = 9;
    info.currentOffset_ = 10.0f;
    algorithm->Layout(AceType::RawPtr(frameNode_));

    EXPECT_TRUE(info.reachStart_);
    EXPECT_TRUE(info.reachEnd_);
    EXPECT_TRUE(info.offsetEnd_);
}

/**
 * @tc.name: GridIrregularLayout::FillMatrixOnly001
 * @tc.desc: Test GridIrregularFiller::FillMatrixOnly
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, FillMatrixOnly001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo8());
        CreateColItem(7);
    });

    GridLayoutInfo info;
    info.crossCount_ = 3;
    // partially filled
    info.gridMatrix_ = {
        { 0, { { 0, 0 }, { 1, 0 }, { 2, 1 } } }, // 0 | 0 | 1
    };
    info.childrenCount_ = 7;

    GridIrregularFiller filler(&info, AceType::RawPtr(frameNode_));
    EXPECT_EQ(filler.FillMatrixOnly(0, 6), 4);
    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_8);
}

/**
 * @tc.name: GridIrregularLayout::MeasureBackward001
 * @tc.desc: Test GridIrregularFiller::MeasureBackward
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, MeasureBackward001, TestSize.Level1)
{
    GridLayoutInfo info;
    info.gridMatrix_ = MATRIX_DEMO_8;
    info.crossCount_ = 3;

    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo8());
        CreateRowItem(10);
    });

    GridIrregularFiller filler(&info, AceType::RawPtr(frameNode_));
    float len = filler.MeasureBackward({ { 50.0f, 50.0f, 50.0f }, 5.0f, 5.0f }, 1000.0f, 5);

    EXPECT_EQ(len, 30.0f);
    EXPECT_EQ(info.lineHeightMap_.size(), 6);
}

/**
 * @tc.name: GridIrregularLayout::FindJumpLineIndex001
 * @tc.desc: Test GridLayoutRangeFinder::FindJumpLineIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, FindJumpLineIndex001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo1());
    });

    auto algo = AceType::MakeRefPtr<GridIrregularLayoutAlgorithm>(GridLayoutInfo {});
    algo->wrapper_ = AceType::RawPtr(frameNode_);

    auto& info = algo->gridLayoutInfo_;
    info.childrenCount_ = 11;
    info.crossCount_ = 3;

    info.scrollAlign_ = ScrollAlign::END;
    EXPECT_EQ(algo->FindJumpLineIdx(5), 6);
    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_1);

    info.scrollAlign_ = ScrollAlign::CENTER;
    EXPECT_EQ(algo->FindJumpLineIdx(5), 3);
    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_1);

    info.gridMatrix_.clear();
    info.scrollAlign_ = ScrollAlign::START;
    EXPECT_EQ(algo->FindJumpLineIdx(10), 5);
    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_1);

    info.startIndex_ = 2;
    info.endIndex_ = 7;
    info.startMainLineIndex_ = 1;
    info.endMainLineIndex_ = 4;
    EXPECT_EQ(algo->FindJumpLineIdx(6), 3);
    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_1);
}

/**
 * @tc.name: GridIrregularLayout::FindRangeOnJump001
 * @tc.desc: Test GridLayoutRangeFinder::FindRangeOnJump
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, FindRangeOnJump001, TestSize.Level1)
{
    GridLayoutInfo info;
    info.crossCount_ = 3;
    info.lineHeightMap_ = { { 0, 50.0f }, { 1, 100.0f }, { 2, 50.0f }, { 3, 50.0f }, { 4, 80.0f }, { 5, 75.0f },
        { 6, 10.0f } };
    info.gridMatrix_ = MATRIX_DEMO_1;

    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo1());
    });
    frameNode_->GetGeometryNode()->SetContentSize({ 500.0f, 250.0f });

    GridLayoutRangeSolver solver(&info, AceType::RawPtr(frameNode_));

    info.scrollAlign_ = ScrollAlign::START;
    auto res = solver.FindRangeOnJump(2, 5.0f);
    EXPECT_EQ(res.startRow, 0);
    EXPECT_EQ(res.pos, -160.0f);
    EXPECT_EQ(res.endIdx, 10);
    EXPECT_EQ(res.endRow, 5);

    info.scrollAlign_ = ScrollAlign::CENTER;
    res = solver.FindRangeOnJump(4, 5.0f);
    EXPECT_EQ(res.startRow, 0);
    EXPECT_EQ(res.pos, -185.0f);
    EXPECT_EQ(res.endIdx, 10);
    EXPECT_EQ(res.endRow, 5);

    info.scrollAlign_ = ScrollAlign::END;
    res = solver.FindRangeOnJump(4, 5.0f);
    EXPECT_EQ(res.startRow, 0);
    EXPECT_EQ(res.pos, -100.0f);
    EXPECT_EQ(res.endIdx, 8);
    EXPECT_EQ(res.endRow, 4);
}

/**
 * @tc.name: GridIrregularLayout::FindRangeOnJump002
 * @tc.desc: Test GridLayoutRangeFinder::FindRangeOnJump special endIndex (endIndex not on the last line).
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, FindRangeOnJump002, TestSize.Level1)
{
    GridLayoutInfo info;
    info.crossCount_ = 3;
    info.lineHeightMap_ = { { 0, 50.0f }, { 1, 100.0f }, { 2, 50.0f }, { 3, 50.0f }, { 4, 80.0f }, { 5, 75.0f } };
    info.gridMatrix_ = MATRIX_DEMO_8;

    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo8());
    });
    frameNode_->GetGeometryNode()->SetContentSize({ 500.0f, 250.0f });

    GridLayoutRangeSolver solver(&info, AceType::RawPtr(frameNode_));

    info.scrollAlign_ = ScrollAlign::END;
    auto res = solver.FindRangeOnJump(5, 5.0f);
    EXPECT_EQ(res.startRow, 1);
    EXPECT_EQ(res.pos, -125.0f);
    EXPECT_EQ(res.endIdx, 6);
    EXPECT_EQ(res.endRow, 5);
}

/**
 * @tc.name: GridIrregularLayout::SolveForwardForEndIdx001
 * @tc.desc: Test GridLayoutRangeFinder::SolveForwardForEndIdx
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, SolveForwardForEndIdx001, TestSize.Level1)
{
    GridLayoutInfo info;
    info.lineHeightMap_ = { { 0, 50.0f }, { 1, 100.0f }, { 2, 50.0f }, { 3, 50.0f }, { 4, 80.0f } };
    info.gridMatrix_ = {
        { 0, { { 0, 0 }, { 1, 0 }, { 2, 1 } } },    // 0 | 0 | 1
        { 1, { { 0, 2 }, { 1, -2 }, { 2, -2 } } },  // 2 | 2 | 2
        { 2, { { 0, -2 }, { 1, -2 }, { 2, -2 } } }, // 2 | 2 | 2
        { 3, { { 0, 3 }, { 1, 4 }, { 2, 5 } } },    // 3 | 4 | 5
        { 4, { { 0, 6 }, { 1, -6 }, { 2, -5 } } },  // 6 | 6 | 5
    };

    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions({});
    });

    GridLayoutRangeSolver solver(&info, AceType::RawPtr(frameNode_));
    auto [endLineIdx, endIdx] = solver.SolveForwardForEndIdx(5.0f, 250.0f, 1);
    EXPECT_EQ(endIdx, 6);
    EXPECT_EQ(endLineIdx, 4);
}

namespace {
void CheckAlignStart(const RefPtr<GridIrregularLayoutAlgorithm>& algorithm, GridLayoutInfo& info)
{
    info.scrollAlign_ = ScrollAlign::START;
    int32_t idx = 0;
    algorithm->PrepareLineHeight(70.0f, idx);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::START);
    EXPECT_EQ(idx, 0);

    info.scrollAlign_ = ScrollAlign::START;
    idx = 2;
    algorithm->PrepareLineHeight(300.0f, idx);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::START);
    EXPECT_EQ(idx, 2);

    // can't align start with idx 4
    info.scrollAlign_ = ScrollAlign::START;
    idx = 4;
    algorithm->PrepareLineHeight(300.0f, idx);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::END);
    EXPECT_EQ(idx, 4);
}

void CheckAlignCenter(const RefPtr<GridIrregularLayoutAlgorithm>& algorithm, GridLayoutInfo& info)
{
    // can't align center with idx 0
    info.scrollAlign_ = ScrollAlign::CENTER;
    int32_t idx = 0;
    algorithm->PrepareLineHeight(350.0f, idx);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::START);
    EXPECT_EQ(idx, 0);

    // can't align center with idx 4
    info.scrollAlign_ = ScrollAlign::CENTER;
    idx = 4;
    algorithm->PrepareLineHeight(350.0f, idx);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::END);
    EXPECT_EQ(idx, 4);

    // align center with idx 4 and len 30.0f
    info.scrollAlign_ = ScrollAlign::CENTER;
    idx = 4;
    algorithm->PrepareLineHeight(30.0f, idx);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::CENTER);
    EXPECT_EQ(idx, 4);
}

void CheckAlignEnd(const RefPtr<GridIrregularLayoutAlgorithm>& algorithm, GridLayoutInfo& info)
{
    // can't align end with idx 1 and len 200.0f
    info.scrollAlign_ = ScrollAlign::END;
    int32_t idx = 1;
    algorithm->PrepareLineHeight(500.0f, idx);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::START);
    EXPECT_EQ(idx, 0);

    info.scrollAlign_ = ScrollAlign::END;
    idx = 3;
    algorithm->PrepareLineHeight(300.0f, idx);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::END);
    EXPECT_EQ(idx, 3);

    info.scrollAlign_ = ScrollAlign::END;
    idx = 4;
    algorithm->PrepareLineHeight(1000.0f, idx);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::END);
    EXPECT_EQ(idx, 4);

    // can't align end with len 340
    info.scrollAlign_ = ScrollAlign::END;
    idx = 4;
    algorithm->PrepareLineHeight(1040.0f, idx);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::START);
    EXPECT_EQ(idx, 0);
}
} // namespace

/**
 * @tc.name: GridIrregularLayout::PrepareLineHeights001
 * @tc.desc: Test GridIrregularLayout::PrepareLineHeights001
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, PrepareLineHeights001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions({});
        CreateColItem(15);
    });

    auto algorithm = AceType::MakeRefPtr<GridIrregularLayoutAlgorithm>(GridLayoutInfo {});
    algorithm->wrapper_ = AceType::RawPtr(frameNode_);
    algorithm->crossLens_ = { 1.0f, 1.0f, 1.0f };
    auto& info = algorithm->gridLayoutInfo_;
    // because measuring children might not generate proper heights in test, we set them manually.
    decltype(info.lineHeightMap_) cmpH = { { 0, 200.0f }, { 1, 200.0f }, { 2, 200.0f }, { 3, 200.0f }, { 4, 200.0f } };
    info.lineHeightMap_ = cmpH;
    decltype(info.gridMatrix_) cmp = {
        { 0, { { 0, 0 }, { 1, 1 }, { 2, 2 } } },
        { 1, { { 0, 3 }, { 1, 4 }, { 2, 5 } } },
        { 2, { { 0, 6 }, { 1, 7 }, { 2, 8 } } },
        { 3, { { 0, 9 }, { 1, 10 }, { 2, 11 } } },
        { 4, { { 0, 12 }, { 1, 13 }, { 2, 14 } } },
    };
    info.gridMatrix_ = cmp;

    info.crossCount_ = 3;
    info.childrenCount_ = 15;

    CheckAlignStart(algorithm, info);
    CheckAlignCenter(algorithm, info);
    CheckAlignEnd(algorithm, info);

    EXPECT_EQ(cmp, info.gridMatrix_);
    EXPECT_EQ(cmpH, info.lineHeightMap_);
}

/**
 * @tc.name: GridIrregularLayout::SkipLines001
 * @tc.desc: Test GridIrregularLayout::SkipLines001
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, SkipLines001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo1());
    });

    auto algorithm = AceType::MakeRefPtr<GridIrregularLayoutAlgorithm>(GridLayoutInfo {});
    algorithm->wrapper_ = AceType::RawPtr(frameNode_);

    auto& info = algorithm->gridLayoutInfo_;
    info.crossCount_ = 3;
    info.lineHeightMap_ = { { 0, 200.0f }, { 1, 200.0f }, { 2, 200.0f } };
    info.gridMatrix_ = MATRIX_DEMO_1;
    info.childrenCount_ = 11;

    info.currentOffset_ = -500.0f;
    EXPECT_EQ(algorithm->SkipLinesForward(), 5);
    info.currentOffset_ = -900.0f;
    EXPECT_EQ(algorithm->SkipLinesForward(), 9);
    info.currentOffset_ = -1500.0f;
    EXPECT_EQ(algorithm->SkipLinesForward(), 10);

    info.lineHeightMap_ = { { 3, 200.0f }, { 4, 200.0f } };
    info.startIndex_ = 5;
    info.startMainLineIndex_ = 3;
    info.endMainLineIndex_ = 4;

    info.currentOffset_ = 400.0f;
    EXPECT_EQ(algorithm->SkipLinesBackward(), 2);

    info.currentOffset_ = 800.0f;
    EXPECT_EQ(algorithm->SkipLinesBackward(), 0);

    info.currentOffset_ = 1500.0f;
    EXPECT_EQ(algorithm->SkipLinesBackward(), 0);
}

/**
 * @tc.name: GridIrregularLayout::TrySkipping001
 * @tc.desc: Test GridIrregularLayout::TrySkipping001
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, TrySkipping001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo2());
        CreateFixedItem(8);
    });

    auto algorithm = AceType::MakeRefPtr<GridIrregularLayoutAlgorithm>(GridLayoutInfo {});
    algorithm->wrapper_ = AceType::RawPtr(frameNode_);
    algorithm->crossLens_ = { 200.0f, 200.0f, 200.0f };

    auto& info = algorithm->gridLayoutInfo_;
    info.crossCount_ = 3;
    info.childrenCount_ = 8;

    info.lineHeightMap_ = { { 0, 200.0f }, { 1, 200.0f }, { 2, 200.0f } };
    info.gridMatrix_ = MATRIX_DEMO_2;
    info.startMainLineIndex_ = 0;
    info.endMainLineIndex_ = 1;
    info.startIndex_ = 0;
    info.endIndex_ = 1;

    info.currentOffset_ = -50.0f;
    EXPECT_FALSE(algorithm->TrySkipping(300.0f));
    info.currentOffset_ = -300.0f;
    EXPECT_FALSE(algorithm->TrySkipping(300.0f));
    info.currentOffset_ = -400.0f;
    EXPECT_FALSE(algorithm->TrySkipping(300.0f));
    info.currentOffset_ = -401.0f;
    EXPECT_TRUE(algorithm->TrySkipping(300.0f));
    EXPECT_EQ(info.startIndex_, 4);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::START);

    info.scrollAlign_ = ScrollAlign::NONE;
    info.lineHeightMap_ = { { 3, 200.0f }, { 4, 200.0f } };
    info.startMainLineIndex_ = 3;
    info.endMainLineIndex_ = 4;
    info.startIndex_ = 4;
    info.endIndex_ = 7;

    info.currentOffset_ = 50.0f;
    EXPECT_FALSE(algorithm->TrySkipping(300.0f));
    info.currentOffset_ = 300.0f;
    EXPECT_FALSE(algorithm->TrySkipping(300.0f));
    info.currentOffset_ = 400.0f;
    EXPECT_FALSE(algorithm->TrySkipping(300.0f));
    info.currentOffset_ = 401.0f;
    EXPECT_TRUE(algorithm->TrySkipping(300.0f));
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.scrollAlign_, ScrollAlign::START);
}

/**
 * @tc.name: GridIrregularFiller::FillMatrixByLine001
 * @tc.desc: Test GridIrregularFiller::FillMatrixByLine
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, FillMatrixByLine001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo2());
        CreateFixedItem(8);
    });

    GridLayoutInfo info;
    info.crossCount_ = 3;
    info.childrenCount_ = 8;
    info.gridMatrix_ = MATRIX_DEMO_2;
    info.startMainLineIndex_ = 0;
    info.endMainLineIndex_ = 1;
    info.startIndex_ = 0;
    info.endIndex_ = 1;

    GridIrregularFiller filler(&info, AceType::RawPtr(frameNode_));
    int32_t idx = filler.FillMatrixByLine(0, 3);
    EXPECT_EQ(idx, 4);

    idx = filler.FillMatrixByLine(0, 10);
    EXPECT_EQ(idx, 7);

    info.gridMatrix_.clear();
    idx = filler.FillMatrixByLine(0, 3);
    EXPECT_EQ(idx, 4);
    idx = filler.FillMatrixByLine(2, 5);
    EXPECT_EQ(idx, 7);

    info.gridMatrix_.clear();
    idx = filler.FillMatrixByLine(0, 10);
    EXPECT_EQ(idx, 7);
    EXPECT_EQ(info.gridMatrix_, MATRIX_DEMO_2);
}

/**
 * @tc.name: GridIrregularLayout::TransformAutoScrollAlign001
 * @tc.desc: Test IrregularLayout::TransformAutoScrollAlign
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, TransformAutoScrollAlign001, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo8());
    });

    auto algo = AceType::MakeRefPtr<GridIrregularLayoutAlgorithm>(GridLayoutInfo {});
    algo->wrapper_ = AceType::RawPtr(frameNode_);

    auto& info = algo->gridLayoutInfo_;
    info.lineHeightMap_ = { { 0, 50.0f }, { 1, 300.0f }, { 2, 30.0f }, { 3, 50.0f }, { 4, 80.0f } };
    info.gridMatrix_ = MATRIX_DEMO_8;
    algo->mainGap_ = 5.0f;

    info.jumpIndex_ = 2;
    info.startMainLineIndex_ = 0;
    info.endMainLineIndex_ = 4;
    info.startIndex_ = 0;
    info.endIndex_ = 6;
    EXPECT_EQ(algo->TransformAutoScrollAlign(500.0f), ScrollAlign::NONE);

    info.jumpIndex_ = 0;
    info.startMainLineIndex_ = 3;
    info.endMainLineIndex_ = 4;
    info.startIndex_ = 3;
    info.endIndex_ = 6;
    info.currentOffset_ = -10.0f;
    EXPECT_EQ(algo->TransformAutoScrollAlign(100.0f), ScrollAlign::START);

    info.jumpIndex_ = 2;
    info.startMainLineIndex_ = 1;
    info.endMainLineIndex_ = 2;
    info.startIndex_ = 2;
    info.endIndex_ = 2;
    info.currentOffset_ = -25.0f;
    EXPECT_EQ(algo->TransformAutoScrollAlign(310.0f), ScrollAlign::NONE);
}

/**
 * @tc.name: GridIrregularLayout::TransformAutoScrollAlign002
 * @tc.desc: Test IrregularLayout::TransformAutoScrollAlign
 * @tc.type: FUNC
 */
HWTEST_F(GridIrregularLayoutTest, TransformAutoScrollAlign002, TestSize.Level1)
{
    Create([](GridModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetLayoutOptions(GetOptionDemo8());
    });

    auto algo = AceType::MakeRefPtr<GridIrregularLayoutAlgorithm>(GridLayoutInfo {});
    algo->wrapper_ = AceType::RawPtr(frameNode_);

    auto& info = algo->gridLayoutInfo_;
    info.lineHeightMap_ = { { 0, 50.0f }, { 1, 300.0f }, { 2, 30.0f }, { 3, 50.0f }, { 4, 80.0f } };
    info.gridMatrix_ = MATRIX_DEMO_8;
    algo->mainGap_ = 5.0f;

    // line 3 now matches with the end of the viewport, should endMainlineIndex_ be updated to 3?
    info.currentOffset_ = -30.0f;
    info.endMainLineIndex_ = 3;
    info.endIndex_ = 5;
    EXPECT_EQ(algo->TransformAutoScrollAlign(310.0f), ScrollAlign::START);
    info.currentOffset_ = -31.0f;
    EXPECT_EQ(algo->TransformAutoScrollAlign(310.0f), ScrollAlign::START);

    info.jumpIndex_ = 0;
    info.startMainLineIndex_ = 3;
    info.endMainLineIndex_ = 4;
    info.startIndex_ = 3;
    info.endIndex_ = 6;
    EXPECT_EQ(algo->TransformAutoScrollAlign(100.0f), ScrollAlign::START);

    info.jumpIndex_ = 4;
    info.startMainLineIndex_ = 1;
    info.endMainLineIndex_ = 4;
    info.startIndex_ = 2;
    info.endIndex_ = 6;

    info.currentOffset_ = -379.0f;
    algo->mainGap_ = 50.0f;
    EXPECT_EQ(algo->TransformAutoScrollAlign(152.0f), ScrollAlign::NONE);

    algo->mainGap_ = 5.0f;
    // emulate init
    info.lineHeightMap_.clear();
    info.gridMatrix_.clear();
    info.jumpIndex_ = 3;
    info.startMainLineIndex_ = 0;
    info.endMainLineIndex_ = 0;
    info.startIndex_ = 0;
    info.endIndex_ = -1;
    EXPECT_EQ(algo->TransformAutoScrollAlign(300.0f), ScrollAlign::END);
}
} // namespace OHOS::Ace::NG
