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
#include "gtest/gtest.h"

#define private public
#include "test/unittest/core/pattern/grid/irregular/irregular_matrices.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
class GridLayoutInfoTest : public testing::Test {};

/**
 * @tc.name: GridLayoutInfo::GetContentHeight001
 * @tc.desc: test GetContentHeight while changing endIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, GetContentHeight001, TestSize.Level1)
{
    GridLayoutInfo info;
    info.lineHeightMap_ = { { 0, 5.0f }, { 1, 10.0f }, { 2, 5.0f }, { 3, 10.0f }, { 4, 5.0f }, { 5, 5.0f },
        { 6, 10.0f }, { 7, 5.0f } };
    info.gridMatrix_ = MATRIX_DEMO_3;

    GridLayoutOptions option {
        .irregularIndexes = { 2, 5, 10 },
    };

    info.crossCount_ = 2;
    info.childrenCount_ = 12;
    EXPECT_EQ(info.GetContentHeight(option, 12, 1.0f), 62.0f);
    info.childrenCount_ = 11;
    EXPECT_EQ(info.GetContentHeight(option, 11, 1.0f), 56.0f);
    info.childrenCount_ = 10;
    EXPECT_EQ(info.GetContentHeight(option, 10, 1.0f), 45.0f);
    info.childrenCount_ = 6;
    EXPECT_EQ(info.GetContentHeight(option, 6, 1.0f), 33.0f);
    info.childrenCount_ = 5;
    EXPECT_EQ(info.GetContentHeight(option, 5, 1.0f), 22.0f);
    info.childrenCount_ = 2;
    EXPECT_EQ(info.GetContentHeight(option, 2, 1.0f), 5.0f);
}

/**
 * @tc.name: GridLayoutInfo::GetContentHeight002
 * @tc.desc: test GetContentHeight, adapted from test demo
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, GetContentHeight002, TestSize.Level1)
{
    GridLayoutInfo info;
    info.lineHeightMap_ = { { 0, 5.0f }, { 1, 5.0f }, { 2, 5.0f } };
    info.gridMatrix_ = {
        { 0, { { 0, 0 }, { 1, 0 }, { 2, 0 } } },
        { 1, { { 0, 1 }, { 1, 2 }, { 2, 3 } } },
        { 2, { { 0, 4 } } },
    };

    GridLayoutOptions option {
        .irregularIndexes = { 0 },
    };

    info.childrenCount_ = 5;
    info.crossCount_ = 3;
    EXPECT_EQ(info.GetContentHeight(option, 5, 1.0f), 17.0f);
}

/**
 * @tc.name: GridLayoutInfo::GetContentHeightBigItem001
 * @tc.desc: test GetContentHeight when hasBigItem_ == true.
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, GetContentHeightBigItem001, TestSize.Level1)
{
    GridLayoutInfo info;
    info.hasBigItem_ = true;
    info.crossCount_ = 2;
    info.childrenCount_ = 12;

    info.gridMatrix_ = MATRIX_DEMO_3;
    constexpr float trueHeight = 47.0f;
    constexpr float error = trueHeight * 0.1f;

    info.lineHeightMap_ = { { 0, 5.0f }, { 1, 5.0f }, { 2, 5.0f }, { 3, 5.0f }, { 4, 5.0f }, { 5, 5.0f }, { 6, 5.0f },
        { 7, 5.0f } };

    EXPECT_EQ(info.GetContentHeight(1.0f), trueHeight);

    info.lineHeightMap_ = { { 4, 5.0f }, { 5, 5.0f }, { 6, 5.0f }, { 7, 5.0f } }; // total height = 47.0f
    float estimation = info.GetContentHeight(1.0f);
    EXPECT_LE(std::abs(estimation - trueHeight), error);

    info.lineHeightMap_ = { { 0, 5.0f }, { 1, 5.0f }, { 2, 5.0f }, { 3, 5.0f }, { 4, 5.0f }, { 5, 5.0f } };
    estimation = info.GetContentHeight(1.0f);
    EXPECT_LE(std::abs(estimation - trueHeight), error);
}

/**
 * @tc.name: GridLayoutInfo::GetContentHeightBigItem002
 * @tc.desc: test GetContentHeight when hasBigItem_ == true, adapted from test demo.
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, GetContentHeightBigItem002, TestSize.Level1)
{
    GridLayoutInfo info;
    info.hasBigItem_ = true;
    info.crossCount_ = 3;
    info.childrenCount_ = 32;

    info.gridMatrix_ = MATRIX_DEMO_4;
    // using mainGap = 5.0f and lineHeight = 100.0f
    constexpr float trueHeight = 2095.0f;
    constexpr float error = trueHeight * 0.1f;

    info.lineHeightMap_ = { { 0, 100.0f }, { 1, 100.0f }, { 2, 100.0f }, { 3, 100.0f }, { 4, 100.0f }, { 5, 100.0f },
        { 6, 100.0f } };

    float estimation = info.GetContentHeight(5.0f);
    EXPECT_LE(std::abs(estimation - trueHeight), error);

    info.lineHeightMap_ = { { 3, 100.0f }, { 4, 100.0f }, { 5, 100.0f }, { 6, 100.0f }, { 7, 100.0f }, { 8, 100.0f },
        { 9, 100.0f }, { 10, 100.0f }, { 11, 100.0f }, { 12, 100.0f }, { 13, 100.0f }, { 14, 100.0f }, { 15, 100.0f },
        { 16, 100.0f }, { 17, 100.0f } };
    estimation = info.GetContentHeight(5.0f);
    EXPECT_LE(std::abs(estimation - trueHeight), error);

    info.lineHeightMap_ = { { 0, 100.0f }, { 1, 100.0f }, { 2, 100.0f }, { 3, 100.0f }, { 4, 100.0f }, { 5, 100.0f },
        { 6, 100.0f }, { 7, 100.0f }, { 8, 100.0f }, { 9, 100.0f }, { 10, 100.0f }, { 11, 100.0f }, { 12, 100.0f },
        { 13, 100.0f }, { 14, 100.0f }, { 15, 100.0f }, { 16, 100.0f }, { 17, 100.0f }, { 18, 100.0f },
        { 19, 100.0f } };
    EXPECT_EQ(info.GetContentHeight(5.0f), trueHeight);
}

/**
 * @tc.name: GridLayoutInfo::GetContentOffsetBigItem001
 * @tc.desc: test GetContentOffset when hasBigItem_ == true
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, GetContentOffsetBigItem001, TestSize.Level1)
{
    GridLayoutInfo info;
    info.hasBigItem_ = true;
    info.crossCount_ = 3;
    info.childrenCount_ = 32;

    info.gridMatrix_ = MATRIX_DEMO_4;

    info.startIndex_ = 8;
    info.startMainLineIndex_ = 9;
    info.currentOffset_ = -1.0f;
    info.lineHeightMap_ = { { 0, 100.0f }, { 1, 100.0f }, { 2, 100.0f }, { 3, 100.0f }, { 4, 100.0f }, { 5, 100.0f },
        { 6, 100.0f }, { 7, 100.0f }, { 8, 100.0f }, { 9, 100.0f }, { 10, 100.0f } };

    EXPECT_EQ(info.GetContentOffset(5.0f), 946.0f);

    info.startIndex_ = 26;
    info.startMainLineIndex_ = 16;
    info.currentOffset_ = -7.0f;
    info.lineHeightMap_ = { { 0, 100.0f }, { 1, 100.0f }, { 2, 100.0f }, { 3, 100.0f }, { 4, 100.0f }, { 5, 100.0f },
        { 6, 100.0f }, { 7, 100.0f }, { 8, 100.0f }, { 9, 100.0f }, { 10, 100.0f }, { 11, 100.0f }, { 12, 100.0f },
        { 13, 100.0f }, { 14, 100.0f }, { 15, 100.0f }, { 16, 100.0f }, { 17, 100.0f }, { 18, 100.0f },
        { 19, 100.0f } };

    EXPECT_EQ(info.GetContentOffset(5.0f), 1687.0f);
}

/**
 * @tc.name: GridLayoutInfo::GetContentOffsetBigItem002
 * @tc.desc: test GetContentOffset estimation when hasBigItem_ == true
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, GetContentOffsetBigItem002, TestSize.Level1)
{
    GridLayoutInfo info;
    info.hasBigItem_ = true;
    info.crossCount_ = 3;
    info.childrenCount_ = 32;

    info.gridMatrix_ = MATRIX_DEMO_4;

    info.startIndex_ = 15;
    info.startMainLineIndex_ = 11;
    info.currentOffset_ = -10.0f;
    info.lineHeightMap_ = { { 3, 100.0f }, { 4, 100.0f }, { 5, 100.0f }, { 6, 100.0f }, { 7, 100.0f }, { 8, 100.0f },
        { 9, 100.0f }, { 10, 100.0f }, { 11, 100.0f }, { 12, 100.0f }, { 13, 100.0f }, { 14, 100.0f }, { 15, 100.0f },
        { 16, 100.0f } };
    constexpr float trueOffset = 1165.0f;
    constexpr float error = trueOffset * 0.25f;
    EXPECT_LE(std::abs(info.GetContentOffset(5.0f) - trueOffset), error);
}

/**
 * @tc.name: GridLayoutInfo::GetContentOffsetBigItem003
 * @tc.desc: test GetContentOffset estimation when hasBigItem_ == true
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, GetContentOffsetBigItem003, TestSize.Level1)
{
    GridLayoutInfo info;
    info.hasBigItem_ = true;
    info.crossCount_ = 3;
    info.childrenCount_ = 32;

    info.gridMatrix_ = MATRIX_DEMO_4;

    info.startIndex_ = 20;
    info.startMainLineIndex_ = 14;
    info.currentOffset_ = -15.0f;
    // simulating layout after jump
    info.lineHeightMap_ = { { 13, 100.0f }, { 14, 100.0f }, { 15, 100.0f }, { 16, 100.0f } };
    constexpr float trueOffset = 1380.0f;
    constexpr float error = trueOffset * 0.4f;
    EXPECT_LE(std::abs(info.GetContentOffset(5.0f) - trueOffset), error);
}

/**
 * @tc.name: GridLayoutInfo::GetContentOffset001
 * @tc.desc: test GetContentOffset with irregular items
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, GetContentOffset001, TestSize.Level1)
{
    GridLayoutInfo info;
    info.lineHeightMap_ = { { 0, 5.0f }, { 1, 10.0f }, { 2, 5.0f }, { 3, 10.0f }, { 4, 5.0f }, { 5, 5.0f },
        { 6, 10.0f }, { 7, 5.0f } };
    info.gridMatrix_ = MATRIX_DEMO_6;

    GridLayoutOptions option {
        .irregularIndexes = { 2, 5, 10 },
    };

    info.crossCount_ = 2;
    info.childrenCount_ = 12;

    info.startIndex_ = 0;
    info.currentOffset_ = -1.0f;
    EXPECT_EQ(info.GetContentOffset(option, 1.0f), 1.0f);

    info.startIndex_ = 2;
    info.currentOffset_ = -1.0f;
    EXPECT_EQ(info.GetContentOffset(option, 1.0f), 7.0f);

    info.startIndex_ = 3;
    info.currentOffset_ = -2.0f;
    EXPECT_EQ(info.GetContentOffset(option, 1.0f), 19.0f);

    info.startIndex_ = 5;
    info.currentOffset_ = -3.0f;
    EXPECT_EQ(info.GetContentOffset(option, 1.0f), 26.0f);

    info.startIndex_ = 8;
    info.currentOffset_ = 0.0f;
    EXPECT_EQ(info.GetContentOffset(option, 1.0f), 40.0f);

    info.startIndex_ = 10;
    info.currentOffset_ = -6.0f;
    EXPECT_EQ(info.GetContentOffset(option, 1.0f), 52.0f);
}

/**
 * @tc.name: GridLayoutInfo::GetContentOffset002
 * @tc.desc: test GetContentOffset with irregular items
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, GetContentOffset002, TestSize.Level1)
{
    GridLayoutInfo info;
    info.lineHeightMap_ = { { 0, 5.0f }, { 1, 5.0f }, { 2, 5.0f }, { 3, 5.0f }, { 4, 5.0f }, { 5, 5.0f },
        { 6, 5.0f } };
    info.gridMatrix_ = MATRIX_DEMO_12;

    auto option = GetOptionDemo12();

    info.crossCount_ = 3;
    info.childrenCount_ = 7;
    info.hasBigItem_ = true;

    info.startIndex_ = 0;
    info.startMainLineIndex_ = 0;
    info.currentOffset_ = -4.0f;
    EXPECT_EQ(info.GetContentOffset(option, 1.0f), 4.0f);

    info.startIndex_ = 2;
    info.startMainLineIndex_ = 1;
    info.currentOffset_ = -11.0f;
    EXPECT_EQ(info.GetContentOffset(option, 1.0f), 17.0f);

    info.currentOffset_ = -20.0f;
    EXPECT_EQ(info.GetContentOffset(option, 1.0f), 26.0f);

    info.currentOffset_ = -29.0f;
    EXPECT_EQ(info.GetContentOffset(option, 1.0f), 35.0f);
}

/**
 * @tc.name: GridLayoutInfo::GetCurrentOffsetOfRegularGrid001
 * @tc.desc: test GetCurrentOffsetOfRegularGrid with varying lineHeights
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, GetCurrentOffsetOfRegularGrid001, TestSize.Level1)
{
    GridLayoutInfo info;
    info.lineHeightMap_ = { { 0, 5.0f }, { 1, 5.0f }, { 2, 5.0f }, { 3, 5.0f }, { 4, 10.0f }, { 5, 10.0f },
        { 6, 10.0f } };
    info.startIndex_ = 16;
    info.startMainLineIndex_ = 5;
    info.crossCount_ = 3;

    EXPECT_EQ(info.GetCurrentOffsetOfRegularGrid(1.0f), 35.0f);
}

/**
 * @tc.name: GridLayoutInfo::GetContentHeightRegular001
 * @tc.desc: test GetContentHeight with regular children but different line heights
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, GetContentHeightRegular001, TestSize.Level1)
{
    GridLayoutInfo info;
    info.hasBigItem_ = false;
    info.lineHeightMap_ = { { 0, 5.0f }, { 1, 5.0f }, { 2, 5.0f }, { 3, 5.0f }, { 4, 10.0f }, { 5, 10.0f },
        { 6, 10.0f } };
    info.startIndex_ = 10;
    info.startMainLineIndex_ = 5;
    info.crossCount_ = 2;

    info.childrenCount_ = 14;
    EXPECT_EQ(info.GetContentHeight(1.0f), 56.0f);

    info.childrenCount_ = 13;
    EXPECT_EQ(info.GetContentHeight(1.0f), 56.0f);
}

/**
 * @tc.name: FindItemInRange001
 * @tc.desc: Test GridLayoutInfo::FindItemInRange
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, FindItemInRange001, TestSize.Level1)
{
    GridLayoutInfo info;
    info.gridMatrix_ = MATRIX_DEMO_7;
    info.startMainLineIndex_ = 0;
    info.endMainLineIndex_ = 3;
    auto res = info.FindItemInRange(5);
    EXPECT_EQ(res.first, 2);
    EXPECT_EQ(res.second, 0);

    res = info.FindItemInRange(7);
    EXPECT_EQ(res.first, 3);
    EXPECT_EQ(res.second, 0);

    res = info.FindItemInRange(3);
    EXPECT_EQ(res.first, 1);
    EXPECT_EQ(res.second, 1);

    res = info.FindItemInRange(10);
    EXPECT_EQ(res.first, -1);
    EXPECT_EQ(res.second, -1);

    info.gridMatrix_.clear();
    EXPECT_EQ(info.FindItemInRange(7).first, -1);
}

/**
 * @tc.name: GetTotalHeightOfItemsInView001
 * @tc.desc: Test GridLayoutInfo::GetTotalHeightOfItemsInView
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, GetTotalHeightOfItemsInView001, TestSize.Level1)
{
    GridLayoutInfo info;
    info.lineHeightMap_ = {{0, 100.0f}, {1, 0.0f}, {2, 100.0f}, {3, 200.0f}};
    info.startMainLineIndex_ = 0;
    info.endMainLineIndex_ = 3;
    info.currentOffset_ = -50.0f;
    EXPECT_EQ(info.GetTotalHeightOfItemsInView(5.0f, false), 415.0f);
    EXPECT_EQ(info.GetTotalHeightOfItemsInView(5.0f, true), 415.0f);
}

namespace {
void CheckEachIndex(const GridLayoutInfo& info, int32_t maxIdx)
{
    for (int i = 0; i <= maxIdx; ++i) {
        auto it = info.FindInMatrix(i);
        EXPECT_NE(it, info.gridMatrix_.end());
        bool foundFlag = false;
        for (auto [_, item] : it->second) {
            if (item == i) {
                foundFlag = true;
                break;
            }
        }
        EXPECT_TRUE(foundFlag);
    }
}
} // namespace

/**
 * @tc.name: FindInMatrix001
 * @tc.desc: Test GridLayoutInfo::FindInMatrix
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, FindInMatrix, TestSize.Level1)
{
    GridLayoutInfo info;
    auto nullIt = info.FindInMatrix(1);
    EXPECT_EQ(nullIt, info.gridMatrix_.end());

    info.gridMatrix_ = MATRIX_DEMO_4;

    CheckEachIndex(info, 31);

    nullIt = info.FindInMatrix(32);
    EXPECT_EQ(nullIt, info.gridMatrix_.end());
}

/**
 * @tc.name: FindInMatrix002
 * @tc.desc: Test GridLayoutInfo::FindInMatrix
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, FindInMatrix002, TestSize.Level1)
{
    GridLayoutInfo info;
    info.gridMatrix_ = MATRIX_DEMO_3;

    CheckEachIndex(info, 11);

    auto nullIt = info.FindInMatrix(12);
    EXPECT_EQ(nullIt, info.gridMatrix_.end());
}

/**
 * @tc.name: FindInMatrix003
 * @tc.desc: Test GridLayoutInfo::FindInMatrix
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, FindInMatrix003, TestSize.Level1)
{
    GridLayoutInfo info;
    info.gridMatrix_ = MATRIX_DEMO_5;

    CheckEachIndex(info, 10);

    auto nullIt = info.FindInMatrix(11);
    EXPECT_EQ(nullIt, info.gridMatrix_.end());
}

/**
 * @tc.name: ClearMatrixToEnd001
 * @tc.desc: Test GridLayoutInfo::ClearMatrixToEnd
 * @tc.type: FUNC
 */
HWTEST_F(GridLayoutInfoTest, ClearMatrixToEnd001, TestSize.Level1)
{
    GridLayoutInfo info;
    info.gridMatrix_ = MATRIX_DEMO_2;

    info.ClearMatrixToEnd(7, 4);
    const decltype(GridLayoutInfo::gridMatrix_) cmp7 = {
        { 0, { { 0, 0 }, { 1, 0 }, { 2, 1 } } },
        { 1, { { 0, 0 }, { 1, 0 }, { 2, -1 } } },
        { 2, { { 0, 2 }, { 1, 3 }, { 2, -1 } } },
        { 3, { { 0, 4 }, { 1, -4 }, { 2, -4 } } },
        { 4, { { 0, 5 }, { 1, 6 } } },
        { 5, { { 0, -5 } } },
    };
    EXPECT_EQ(info.gridMatrix_, cmp7);

    info.gridMatrix_ = MATRIX_DEMO_2;
    info.ClearMatrixToEnd(6, 4);
    const decltype(GridLayoutInfo::gridMatrix_) cmp6 = {
        { 0, { { 0, 0 }, { 1, 0 }, { 2, 1 } } },
        { 1, { { 0, 0 }, { 1, 0 }, { 2, -1 } } },
        { 2, { { 0, 2 }, { 1, 3 }, { 2, -1 } } },
        { 3, { { 0, 4 }, { 1, -4 }, { 2, -4 } } },
        { 4, { { 0, 5 } } },
        { 5, { { 0, -5 } } },
    };
    EXPECT_EQ(info.gridMatrix_, cmp6);

    info.gridMatrix_ = MATRIX_DEMO_2;
    info.ClearMatrixToEnd(5, 4);
    const decltype(GridLayoutInfo::gridMatrix_) cmp5 = {
        { 0, { { 0, 0 }, { 1, 0 }, { 2, 1 } } },
        { 1, { { 0, 0 }, { 1, 0 }, { 2, -1 } } },
        { 2, { { 0, 2 }, { 1, 3 }, { 2, -1 } } },
        { 3, { { 0, 4 }, { 1, -4 }, { 2, -4 } } },
    };
    EXPECT_EQ(info.gridMatrix_, cmp5);

    info.gridMatrix_ = MATRIX_DEMO_2;
    info.ClearMatrixToEnd(2, 2);
    const decltype(GridLayoutInfo::gridMatrix_) cmp2 = {
        { 0, { { 0, 0 }, { 1, 0 }, { 2, 1 } } },
        { 1, { { 0, 0 }, { 1, 0 }, { 2, -1 } } },
        { 2, { { 2, -1 } } },
    };
    EXPECT_EQ(info.gridMatrix_, cmp2);

    info.gridMatrix_ = MATRIX_DEMO_2;
    info.ClearMatrixToEnd(1, 0);
    const decltype(GridLayoutInfo::gridMatrix_) cmp1 = { { 0, { { 0, 0 }, { 1, 0 } } }, { 1, { { 0, 0 }, { 1, 0 } } } };
    EXPECT_EQ(info.gridMatrix_, cmp1);

    info.gridMatrix_ = MATRIX_DEMO_2;
    info.ClearMatrixToEnd(0, 0);
    const decltype(GridLayoutInfo::gridMatrix_) cmp0 = {};
    EXPECT_EQ(info.gridMatrix_, cmp0);
}
} // namespace OHOS::Ace::NG
