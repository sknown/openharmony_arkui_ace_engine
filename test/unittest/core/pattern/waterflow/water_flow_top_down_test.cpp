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
#include "test/unittest/core/pattern/waterflow/water_flow_test_ng.h"

#include "core/components_ng/pattern/waterflow/water_flow_layout_info.h"

namespace OHOS::Ace::NG {
/**
 * @tc.name: WaterFlowLayoutInfoTest002
 * @tc.desc: Test functions in WaterFlowLayoutInfo.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, WaterFlowLayoutInfoTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Init Waterflow node
     */
    CreateWithItem([](WaterFlowModelNG model) {});

    /**
     * @tc.steps: Test GetStartMainPos and GetMainHeight
     * @tc.expected: step2. Check whether the return value is correct.
     */
    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    int32_t crossIndex = info->items_[0].rbegin()->first;
    int32_t itemIndex = info->items_[0].rbegin()->second.rbegin()->first;
    EXPECT_EQ(info->GetStartMainPos(crossIndex + 1, itemIndex), 0.0f);
    EXPECT_EQ(info->GetMainHeight(crossIndex + 1, itemIndex), 0.0f);

    EXPECT_EQ(info->GetStartMainPos(crossIndex, itemIndex + 1), 0.0f);
    EXPECT_EQ(info->GetMainHeight(crossIndex, itemIndex + 1), 0.0f);
}

/**
 * @tc.name: WaterFlowLayoutInfoTest003
 * @tc.desc: Test functions in WaterFlowLayoutInfo.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, WaterFlowLayoutInfoTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Init Waterflow node
     */
    CreateWithItem([](WaterFlowModelNG model) {});

    /**
     * @tc.steps: Test GetMainCount function
     * @tc.expected: step2. Check whether the size is correct.
     */
    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);

    std::size_t waterFlowItemsSize = info->items_[0].size();
    int32_t mainCount = info->GetMainCount();

    int32_t index = info->items_[0].rbegin()->first;
    info->items_[0][index + 1] = std::map<int32_t, std::pair<float, float>>();
    EXPECT_EQ(info->items_[0].size(), waterFlowItemsSize + 1);
    EXPECT_EQ(info->GetMainCount(), mainCount);

    auto lastItem = info->items_[0].begin()->second.rbegin();
    float mainSize = lastItem->second.first + lastItem->second.second - 1.0f;
    EXPECT_FALSE(info->IsAllCrossReachEnd(mainSize));

    info->ClearCacheAfterIndex(index + 1);
    EXPECT_EQ(info->items_[0].size(), waterFlowItemsSize + 1);
}

/**
 * @tc.name: WaterFlowLayoutInfoTest004
 * @tc.desc: Test Reset functions in WaterFlowLayoutInfo.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, WaterFlowLayoutInfoTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Init Waterflow node
     */
    CreateWithItem([](WaterFlowModelNG model) {});

    /**
     * @tc.steps: Test Reset function
     * @tc.expected: step2. Check whether the endIndex_ is correct.
     */
    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);

    int32_t resetFrom = pattern_->layoutInfo_->endIndex_;
    info->Reset(resetFrom + 1);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, resetFrom);

    info->Reset(resetFrom - 1);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, -1);
}

/**
 * @tc.name: WaterFlowLayoutInfoTest005
 * @tc.desc: Test functions in WaterFlowLayoutInfo.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, WaterFlowLayoutInfoTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Init Waterflow node
     */
    CreateWithItem([](WaterFlowModelNG model) {});

    /**
     * @tc.steps: Test GetMaxMainHeight function
     * @tc.expected: step2. Check whether the return value is correct.
     */
    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);

    float maxMainHeight = info->GetMaxMainHeight();
    int32_t crossIndex = info->items_[0].rbegin()->first;
    info->items_[0][crossIndex + 1][0] = std::pair<float, float>(1.0f, maxMainHeight);
    info->itemInfos_.clear();
    info->endPosArray_.clear();
    EXPECT_EQ(info->GetMaxMainHeight(), maxMainHeight + 1.0f);

    /**
     * @tc.steps: Test GetCrossIndexForNextItem function
     * @tc.expected: step3. Check whether the return value is correct.
     */
    info->items_[0][crossIndex + 1][1] = std::pair<float, float>(0.0f, 0.0f);
    FlowItemIndex position = info->GetCrossIndexForNextItem(0);
    EXPECT_EQ(position.crossIndex, crossIndex + 1);
    EXPECT_EQ(position.lastItemIndex, 1);
}
} // namespace OHOS::Ace::NG
