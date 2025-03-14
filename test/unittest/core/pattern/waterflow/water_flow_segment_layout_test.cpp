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
#include "test/unittest/core/pattern/waterflow/water_flow_item_maps.h"
#include "test/unittest/core/pattern/waterflow/water_flow_test_ng.h"

#include "core/components_ng/pattern/waterflow/water_flow_item_pattern.h"
#include "core/components_ng/pattern/waterflow/layout/top_down/water_flow_layout_info.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_ng/property/measure_property.h"

#define protected public
#define private public
#include "test/mock/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/pattern/waterflow/water_flow_item_node.h"
#include "core/components_ng/pattern/waterflow/layout/top_down/water_flow_segmented_layout.h"

namespace OHOS::Ace::NG {
class WaterFlowSegmentTest : public WaterFlowTestNg {
public:
    static void SetUpTestSuite()
    {
        MockPipelineContext::SetUp();
    }
    static void TearDownTestSuite()
    {
        MockPipelineContext::TearDown();
    }
    void SetUpConfig1();
    void SetUpConfig2();
    void SetUpConfig5();
};

void WaterFlowSegmentTest::SetUpConfig1()
{
    Create(
        [](WaterFlowModelNG model) {
            model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
            model.SetColumnsGap(Dimension(5.0f));
            model.SetRowsGap(Dimension(1.0f));
            auto footer = GetDefaultHeaderBuilder();
            model.SetFooter(std::move(footer));
            CreateItem(10);
            ViewStackProcessor::GetInstance()->Pop();
        },
        false);

    LayoutConstraintF constraint { .maxSize = { 480.0f, 800.0f }, .percentReference = { 480.0f, 800.0f } };
    layoutProperty_->UpdateLayoutConstraint(constraint);
    layoutProperty_->UpdateContentConstraint();
}

void WaterFlowSegmentTest::SetUpConfig2()
{
    Create(
        [](WaterFlowModelNG model) {
            model.SetColumnsTemplate("1fr 1fr 1fr 1fr 1fr");
            model.SetColumnsGap(Dimension(5.0f));
            model.SetRowsGap(Dimension(1.0f));
            auto footer = GetDefaultHeaderBuilder();
            model.SetFooter(std::move(footer));
            CreateItem(100);
            ViewStackProcessor::GetInstance()->Pop();
        },
        false);

    LayoutConstraintF constraint { .maxSize = { 480.0f, 800.0f }, .percentReference = { 480.0f, 800.0f } };
    layoutProperty_->UpdateLayoutConstraint(constraint);
    layoutProperty_->UpdateContentConstraint();
}

void WaterFlowSegmentTest::SetUpConfig5()
{
    Create(
        [](WaterFlowModelNG model) {
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(600.f));
            CreateItem(60);
        },
        false);
    LayoutConstraintF constraint { .maxSize = { 400.0f, 600.0f }, .percentReference = { 400.0f, 600.0f } };
    layoutProperty_->UpdateLayoutConstraint(constraint);
    layoutProperty_->UpdateContentConstraint();
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_5);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
}

/**
 * @tc.name: Fill001
 * @tc.desc: Test SegmentedLayout::Fill.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Fill001, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            model.SetColumnsTemplate("1fr 1fr");
            CreateItemWithHeight(50.0f);
            CreateItemWithHeight(30.0f);
            CreateItemWithHeight(40.0f);
            CreateItemWithHeight(60.0f);
            CreateItemWithHeight(20.0f);
            CreateItemWithHeight(50.0f);
            CreateItemWithHeight(30.0f);
            CreateItemWithHeight(40.0f);
            CreateItemWithHeight(2.0f);
            CreateItemWithHeight(20.0f);
        },
        false);

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::MakeRefPtr<WaterFlowLayoutInfo>());
    algo->wrapper_ = AceType::RawPtr(frameNode_);
    algo->mainSize_ = 2000.0f;
    algo->itemsCrossSize_ = { { 50.0f, 50.0f, 50.0f, 50.0f }, {}, { 70.0f, 70.0f, 70.0f } };
    algo->mainGaps_ = { 5.0f, 0.0f, 1.0f };

    auto& info = algo->info_;
    info->margins_ = { {}, {}, PaddingPropertyF { .top = 5.0f } };
    info->childrenCount_ = 10;

    info->items_.resize(3);
    for (int i = 0; i < 3; ++i) {
        info->items_[0][i] = {};
        info->items_[2][i] = {};
    }
    info->items_[0][3] = {};

    info->segmentTails_ = SEGMENT_TAILS_1;

    algo->Fill(0);
    EXPECT_EQ(info->items_, ITEM_MAP_1);
}

/**
 * @tc.name: MeasureOnOffset001
 * @tc.desc: Test SegmentedLayout::MeasureOnOffset.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, MeasureOnOffset001, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
            model.SetColumnsGap(Dimension(5.0f));
            model.SetRowsGap(Dimension(1.0f));
            auto footer = GetDefaultHeaderBuilder();
            model.SetFooter(std::move(footer));
            CreateItem(10);
            ViewStackProcessor::GetInstance()->Pop();
        },
        false);

    LayoutConstraintF constraint { .maxSize = { 480.0f, 800.0f }, .percentReference = { 480.0f, 800.0f } };
    layoutProperty_->UpdateLayoutConstraint(constraint);
    layoutProperty_->UpdateContentConstraint();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::MakeRefPtr<WaterFlowLayoutInfo>());
    auto& info = algo->info_;

    info->footerIndex_ = 0;

    for (int i = 0; i < 2; ++i) {
        algo->Measure(AceType::RawPtr(frameNode_));

        EXPECT_EQ(info->childrenCount_, 11);
        EXPECT_EQ(info->items_, ITEM_MAP_2);
        EXPECT_EQ(info->itemInfos_, ITEM_INFO_2);
        EXPECT_EQ(info->segmentTails_, SEGMENT_TAILS_2);
        EXPECT_EQ(info->endPosArray_, END_POS_ARRAY_2);
        EXPECT_EQ(info->segmentStartPos_, SEGMENT_START_POS_2);
    }

    info->prevOffset_ = 0.0f;
    info->currentOffset_ = -100.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->currentOffset_, 0.0f);
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 10);

    algo->overScroll_ = true;
    info->prevOffset_ = 0.0f;
    info->currentOffset_ = -200.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->currentOffset_, -200.0f);
    EXPECT_EQ(info->startIndex_, 4);
    EXPECT_EQ(info->endIndex_, 10);

    info->Reset();
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->currentOffset_, -200.0f);
    EXPECT_EQ(info->startIndex_, 4);
    EXPECT_EQ(info->endIndex_, 10);
}

/**
 * @tc.name: MeasureFooter001
 * @tc.desc: Test SegmentedLayout with changing footer
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, MeasureFooter001, TestSize.Level1)
{
    Create([](WaterFlowModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetColumnsGap(Dimension(5.0f));
        model.SetRowsGap(Dimension(1.0f));
        model.SetFooter(GetDefaultHeaderBuilder());
        CreateItem(10);
        ViewStackProcessor::GetInstance()->Pop();
    });

    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    EXPECT_EQ(info->childrenCount_, 11);
    EXPECT_EQ(info->footerIndex_, 10);
    auto footer = WaterFlowItemNode::GetOrCreateFlowItem(
        V2::FLOW_ITEM_ETS_TAG, -1, []() { return AceType::MakeRefPtr<WaterFlowItemPattern>(); });
    footer->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(100.0f), CalcLength(Dimension(200.0f))));
    pattern_->AddFooter(footer);
    EXPECT_EQ(frameNode_->GetTotalChildCount(), 11);
    AddItems(2);
    EXPECT_EQ(frameNode_->GetTotalChildCount(), 13);
    frameNode_->ChildrenUpdatedFrom(10);

    pattern_->MarkDirtyNodeSelf();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(GetChildFrameNode(frameNode_, info->footerIndex_), footer);
    UpdateCurrentOffset(-1000.0f);
    EXPECT_EQ(info->items_.size(), 2);
    EXPECT_EQ(info->items_[1][0].size(), 1);
    EXPECT_EQ(info->childrenCount_, 13);
    EXPECT_EQ(info->endIndex_, 12);
    EXPECT_EQ(info->segmentTails_[0], 11);
    EXPECT_EQ(info->segmentTails_[1], 12);
    EXPECT_EQ(info->footerIndex_, 12);
    EXPECT_EQ(info->itemInfos_[12].mainOffset, 503.0f);
    EXPECT_EQ(info->itemInfos_[12].mainSize, 200.0f);
}

/**
 * @tc.name: MeasureFooter002
 * @tc.desc: Test SegmentedLayout with changing footer and delete items.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, MeasureFooter002, TestSize.Level1)
{
    Create([](WaterFlowModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetColumnsGap(Dimension(5.0f));
        model.SetRowsGap(Dimension(1.0f));
        model.SetFooter(GetDefaultHeaderBuilder());
        CreateItem(10);
        ViewStackProcessor::GetInstance()->Pop();
    });

    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    EXPECT_EQ(info->childrenCount_, 11);
    EXPECT_EQ(info->footerIndex_, 10);
    EXPECT_EQ(info->itemInfos_[8].mainOffset, 202.0f);

    auto footer = WaterFlowItemNode::GetOrCreateFlowItem(
        V2::FLOW_ITEM_ETS_TAG, -1, []() { return AceType::MakeRefPtr<WaterFlowItemPattern>(); });
    footer->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(100.0f), CalcLength(Dimension(200.0f))));
    pattern_->AddFooter(footer);
    EXPECT_EQ(frameNode_->GetTotalChildCount(), 11);
    for (int i = 0; i < 2; ++i) {
        frameNode_->RemoveChildAtIndex(8);
    }
    frameNode_->ChildrenUpdatedFrom(8);

    pattern_->MarkDirtyNodeSelf();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(GetChildFrameNode(frameNode_, info->footerIndex_), footer);
    UpdateCurrentOffset(-1000.0f);
    EXPECT_EQ(info->items_.size(), 2);
    EXPECT_EQ(info->items_[1][0].size(), 1);
    EXPECT_EQ(info->segmentStartPos_[1], 401.0f);
    EXPECT_EQ(info->childrenCount_, 9);
    EXPECT_EQ(info->endIndex_, 8);
    EXPECT_EQ(info->segmentTails_[0], 7);
    EXPECT_EQ(info->segmentTails_[1], 8);
    EXPECT_EQ(info->footerIndex_, 8);
    EXPECT_EQ(info->itemInfos_[8].mainOffset, 401.0f);
    EXPECT_EQ(info->itemInfos_[8].mainSize, 200.0f);
}

/**
 * @tc.name: Layout001
 * @tc.desc: Test SegmentedLayout::Layout.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Layout001, TestSize.Level1)
{
    SetUpConfig1();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::MakeRefPtr<WaterFlowLayoutInfo>());
    auto& info = algo->info_;

    info->footerIndex_ = 0;

    algo->Measure(AceType::RawPtr(frameNode_));
    const std::vector<std::vector<float>> crossSize = { { 116.25f, 116.25f, 116.25f, 116.25f }, { 480.0f } };
    EXPECT_EQ(algo->itemsCrossSize_, crossSize);
    algo->Layout(AceType::RawPtr(frameNode_));
    EXPECT_EQ(GetChildOffset(frameNode_, 0), OffsetF(0.0f, 0.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 1), OffsetF(121.25f, 0.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 2), OffsetF(242.5f, 0.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 3), OffsetF(363.75f, 0.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 4), OffsetF(0.0f, 101.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 5), OffsetF(242.5f, 101.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 6), OffsetF(121.25f, 201.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 7), OffsetF(363.75f, 201.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 8), OffsetF(0.0f, 202.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 9), OffsetF(121.25f, 302.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 10), OffsetF(0.0f, 502.0f));
}

/**
 * @tc.name: Layout002
 * @tc.desc: Test SegmentedLayout::Layout.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Layout002, TestSize.Level1)
{
    SetUpConfig1();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::MakeRefPtr<WaterFlowLayoutInfo>());
    auto& info = algo->info_;

    info->footerIndex_ = 0;

    info->prevOffset_ = 0.0f;
    info->currentOffset_ = -100.0f;
    algo->overScroll_ = true;
    algo->Measure(AceType::RawPtr(frameNode_));
    algo->Layout(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 1);
    EXPECT_EQ(info->endIndex_, 10);
    EXPECT_EQ(GetChildOffset(frameNode_, 1), OffsetF(121.25f, -100.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 2), OffsetF(242.5f, -100.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 3), OffsetF(363.75f, -100.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 4), OffsetF(0.0f, 1.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 5), OffsetF(242.5f, 1.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 6), OffsetF(121.25f, 101.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 7), OffsetF(363.75f, 101.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 8), OffsetF(0.0f, 102.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 9), OffsetF(121.25f, 202.0f));
    EXPECT_EQ(GetChildOffset(frameNode_, 10), OffsetF(0.0f, 402.0f));
}

/**
 * @tc.name: MeasureOnOffset002
 * @tc.desc: Test SegmentedLayout::MeasureOnOffset.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, MeasureOnOffset002, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::MakeRefPtr<WaterFlowLayoutInfo>());
    auto& info = algo->info_;

    info->footerIndex_ = 0;

    for (int i = 0; i < 2; ++i) {
        algo->Measure(AceType::RawPtr(frameNode_));

        EXPECT_EQ(info->childrenCount_, 101);
        EXPECT_EQ(info->items_.size(), 2);
        EXPECT_EQ(info->startIndex_, 0);
        EXPECT_EQ(info->endIndex_, 27);
        EXPECT_EQ(info->segmentTails_, SEGMENT_TAILS_3);
    }

    info->prevOffset_ = 0.0f;
    info->currentOffset_ = -100.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->currentOffset_, -100.0f);
    EXPECT_EQ(info->startIndex_, 1);
    EXPECT_EQ(info->endIndex_, 30);

    info->prevOffset_ = -100.0f;
    info->currentOffset_ = -500.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->currentOffset_, -500.0f);
    EXPECT_EQ(info->startIndex_, 11);
    EXPECT_EQ(info->endIndex_, 44);

    const auto itemMap = info->items_;
    const auto itemInfo = info->itemInfos_;
    const auto endPosArr = info->endPosArray_;

    info->prevOffset_ = -500.0f;
    info->currentOffset_ = -300.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->items_, itemMap);
    EXPECT_EQ(info->itemInfos_, itemInfo);
    EXPECT_EQ(info->endPosArray_, endPosArr);
    EXPECT_EQ(info->startIndex_, 5);
    EXPECT_EQ(info->endIndex_, 37);
    EXPECT_EQ(info->segmentStartPos_, std::vector<float> { 0.0f });

    info->prevOffset_ = -300.0f;
    info->currentOffset_ = -700.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 19);
    EXPECT_EQ(info->endIndex_, 50);
    EXPECT_EQ(info->segmentStartPos_, std::vector<float> { 0.0f });
}

/**
 * @tc.name: MeasureOnJump001
 * @tc.desc: Test SegmentedLayout::MeasureOnJump END.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, MeasureOnJump001, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::MakeRefPtr<WaterFlowLayoutInfo>());
    auto& info = algo->info_;

    info->footerIndex_ = 0;

    info->align_ = ScrollAlign::END;
    info->jumpIndex_ = 5;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 27);
    EXPECT_EQ(info->currentOffset_, 0.0f);

    info->jumpIndex_ = 99;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 75);
    EXPECT_EQ(info->endIndex_, 99);
    EXPECT_EQ(info->currentOffset_, -2320.0f);

    info->jumpIndex_ = 100;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 75);
    EXPECT_EQ(info->endIndex_, 100);
    EXPECT_EQ(info->currentOffset_, -2370.0f);

    info->jumpIndex_ = 105;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 75);
    EXPECT_EQ(info->endIndex_, 100);
    EXPECT_EQ(info->currentOffset_, -2370.0f);
}

/**
 * @tc.name: MeasureOnJump002
 * @tc.desc: Test SegmentedLayout::MeasureOnJump with AUTO scroll.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, MeasureOnJump002, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::MakeRefPtr<WaterFlowLayoutInfo>());
    auto& info = algo->info_;

    info->footerIndex_ = 0;

    info->align_ = ScrollAlign::AUTO;
    info->jumpIndex_ = 10;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 27);
    EXPECT_EQ(info->currentOffset_, 0.0f);
    EXPECT_EQ(info->align_, ScrollAlign::NONE);

    info->align_ = ScrollAlign::AUTO;
    info->jumpIndex_ = 53;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 29);
    EXPECT_EQ(info->endIndex_, 58);
    EXPECT_EQ(info->currentOffset_, -911.0f);
    EXPECT_EQ(info->align_, ScrollAlign::END);

    info->align_ = ScrollAlign::AUTO;
    info->jumpIndex_ = 5;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 1);
    EXPECT_EQ(info->endIndex_, 30);
    EXPECT_EQ(info->currentOffset_, -101.0f);
    EXPECT_EQ(info->align_, ScrollAlign::START);

    info->align_ = ScrollAlign::AUTO;
    info->jumpIndex_ = 5;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->align_, ScrollAlign::NONE);

    info->align_ = ScrollAlign::AUTO;
    info->jumpIndex_ = 7;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 1);
    EXPECT_EQ(info->endIndex_, 30);
    EXPECT_EQ(info->currentOffset_, -101.0f);
    EXPECT_EQ(info->align_, ScrollAlign::NONE);
}

/**
 * @tc.name: MeasureOnJump003
 * @tc.desc: Test SegmentedLayout::MeasureOnJump START.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, MeasureOnJump003, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::MakeRefPtr<WaterFlowLayoutInfo>());
    auto& info = algo->info_;

    info->footerIndex_ = 0;

    info->align_ = ScrollAlign::START;
    info->jumpIndex_ = 10;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 5);
    EXPECT_EQ(info->endIndex_, 34);
    EXPECT_EQ(info->currentOffset_, -202.0f);

    info->jumpIndex_ = 99;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 75);
    EXPECT_EQ(info->endIndex_, 100);
    EXPECT_EQ(info->currentOffset_, -2370.0f);

    info->jumpIndex_ = 42;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 37);
    EXPECT_EQ(info->endIndex_, 67);
    EXPECT_EQ(info->currentOffset_, -1207.0f);

    // invalid
    info->jumpIndex_ = 101;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 37);
    EXPECT_EQ(info->endIndex_, 67);
    EXPECT_EQ(info->currentOffset_, -1207.0f);
}

/**
 * @tc.name: MeasureOnJump004
 * @tc.desc: Test SegmentedLayout::MeasureOnJump CENTER.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, MeasureOnJump004, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::MakeRefPtr<WaterFlowLayoutInfo>());
    auto& info = algo->info_;

    info->footerIndex_ = 0;

    info->align_ = ScrollAlign::CENTER;
    info->jumpIndex_ = 10;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 27);
    EXPECT_EQ(info->currentOffset_, -0.0f);

    info->jumpIndex_ = 99;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 75);
    EXPECT_EQ(info->endIndex_, 100);
    EXPECT_EQ(info->currentOffset_, -2370.0f);
    EXPECT_EQ(info->segmentStartPos_.size(), 2);

    info->jumpIndex_ = 42;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 25);
    EXPECT_EQ(info->endIndex_, 57);
    EXPECT_EQ(info->currentOffset_, -857.0f);

    info->jumpIndex_ = 0;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 27);
    EXPECT_EQ(info->currentOffset_, 0.0f);

    // invalid jumpIndex
    info->jumpIndex_ = 500;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 27);
    EXPECT_EQ(info->currentOffset_, 0.0f);
    EXPECT_EQ(info->jumpIndex_, EMPTY_JUMP_INDEX);
}

/**
 * @tc.name: Reset001
 * @tc.desc: Test SegmentedLayout::CheckReset.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Reset001, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::MakeRefPtr<WaterFlowLayoutInfo>());
    auto& info = algo->info_;

    info->footerIndex_ = 0;

    info->align_ = ScrollAlign::CENTER;

    info->jumpIndex_ = 99;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 75);
    EXPECT_EQ(info->endIndex_, 100);
    EXPECT_EQ(info->currentOffset_, -2370.0f);
    EXPECT_EQ(info->segmentStartPos_.size(), 2);

    // change crossCount, should jump back to index 75
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr 1fr");
    info->Reset();
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 45);
    EXPECT_EQ(info->endIndex_, 64);
    EXPECT_EQ(info->currentOffset_, -2370.0f);
    EXPECT_EQ(algo->itemsCrossSize_[0].size(), 3);
}

/**
 * @tc.name: Reset002
 * @tc.desc: Test SegmentedLayout::CheckReset.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Reset002, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::MakeRefPtr<WaterFlowLayoutInfo>());
    auto& info = algo->info_;

    info->footerIndex_ = 0;

    info->align_ = ScrollAlign::CENTER;
    info->jumpIndex_ = 99;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 75);
    EXPECT_EQ(info->endIndex_, 100);
    EXPECT_EQ(info->currentOffset_, -2370.0f);
    EXPECT_EQ(info->segmentStartPos_.size(), 2);

    info->Reset();
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 75);
    EXPECT_EQ(info->endIndex_, 100);
    EXPECT_EQ(info->currentOffset_, -2370.0f);
    EXPECT_EQ(info->segmentStartPos_.size(), 2);

    info->jumpIndex_ = 42;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 25);
    EXPECT_EQ(info->endIndex_, 57);
    EXPECT_EQ(info->currentOffset_, -857.0f);

    // child requires fresh layout, should jump back to index 75
    layoutProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_BY_CHILD_REQUEST);
    frameNode_->ChildrenUpdatedFrom(0);
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 25);
    EXPECT_EQ(info->endIndex_, 57);
    EXPECT_EQ(info->currentOffset_, -857.0f);
    EXPECT_EQ(info->align_, ScrollAlign::START);
    // items should be cleared before jumping
    EXPECT_EQ(info->items_[1][0].size(), 0);
    EXPECT_EQ(info->segmentStartPos_.size(), 1);
    EXPECT_EQ(info->itemInfos_.size(), 58);

    info->Reset();
    layoutProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_BY_CHILD_REQUEST);
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 25);
    EXPECT_EQ(info->endIndex_, 57);
    EXPECT_EQ(info->currentOffset_, -857.0f);
    // items should be cleared before jumping
    EXPECT_EQ(info->items_[1][0].size(), 0);
    EXPECT_EQ(info->segmentStartPos_.size(), 1);
    EXPECT_EQ(info->itemInfos_.size(), 58);
}

/**
 * @tc.name: Reset003
 * @tc.desc: Test SegmentedLayout::CheckReset.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Reset003, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::MakeRefPtr<WaterFlowLayoutInfo>());
    auto& info = algo->info_;

    info->footerIndex_ = 0;

    info->align_ = ScrollAlign::CENTER;
    info->jumpIndex_ = 99;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 75);
    EXPECT_EQ(info->endIndex_, 100);
    EXPECT_EQ(info->currentOffset_, -2370.0f);
    EXPECT_EQ(info->segmentStartPos_.size(), 2);
    EXPECT_EQ(info->itemInfos_.size(), 101);

    info->jumpIndex_ = 42;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 25);
    EXPECT_EQ(info->endIndex_, 57);
    EXPECT_EQ(info->currentOffset_, -857.0f);

    // index 70 doesn't affect the current layout
    frameNode_->ChildrenUpdatedFrom(70);
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 25);
    EXPECT_EQ(info->endIndex_, 57);
    EXPECT_EQ(info->currentOffset_, -857.0f);
    EXPECT_EQ(info->align_, ScrollAlign::CENTER);
    // items starting from 70 are cleared
    EXPECT_EQ(info->items_[1][0].size(), 0);
    EXPECT_EQ(info->segmentStartPos_.size(), 1);
    EXPECT_EQ(info->itemInfos_.size(), 70);

    // index 20 would reset all and trigger jump
    frameNode_->ChildrenUpdatedFrom(20);
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 25);
    EXPECT_EQ(info->endIndex_, 57);
    EXPECT_EQ(info->currentOffset_, -857.0f);
    EXPECT_EQ(info->align_, ScrollAlign::START);
    // items should be cleared before jumping
    EXPECT_EQ(info->itemInfos_.size(), 58);
}

/**
 * @tc.name: Reset004
 * @tc.desc: Test Changing crossCount and deleting items.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Reset004, TestSize.Level1)
{
    Create([](WaterFlowModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetColumnsGap(Dimension(5.0f));
        model.SetRowsGap(Dimension(1.0f));
        model.SetFooter(GetDefaultHeaderBuilder());
        CreateItem(200);
        ViewStackProcessor::GetInstance()->Pop();
    });

    UpdateCurrentOffset(-2000.0f);
    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    EXPECT_EQ(info->footerIndex_, 200);
    EXPECT_EQ(info->endIndex_, 75);
    EXPECT_EQ(info->startIndex_, 49);
    EXPECT_EQ(info->itemInfos_[info->startIndex_].mainOffset + info->currentOffset_, -188.0f);
    auto footer = WaterFlowItemNode::GetOrCreateFlowItem(
        V2::FLOW_ITEM_ETS_TAG, -1, []() { return AceType::MakeRefPtr<WaterFlowItemPattern>(); });
    footer->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(100.0f), CalcLength(Dimension(200.0f))));
    pattern_->AddFooter(footer);
    for (int i = 0; i < 2; ++i) {
        frameNode_->RemoveChildAtIndex(80);
    }
    frameNode_->ChildrenUpdatedFrom(80);
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr");

    FlushLayoutTask(frameNode_);
    EXPECT_EQ(GetChildFrameNode(frameNode_, info->footerIndex_), footer);
    EXPECT_EQ(info->startIndex_, 25);
    EXPECT_EQ(info->currentOffset_, -2000.0f);
    EXPECT_EQ(GetChildWidth(frameNode_, 25), 237.5f);
}

/**
 * @tc.name: Reset005
 * @tc.desc: Test Changing cross gap.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Reset005, TestSize.Level1)
{
    Create([](WaterFlowModelNG model) {
        model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
        model.SetColumnsGap(Dimension(5.0f));
        model.SetRowsGap(Dimension(1.0f));
        model.SetFooter(GetDefaultHeaderBuilder());
        CreateItem(200);
        ViewStackProcessor::GetInstance()->Pop();
    });
    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 21);
    for (int i = 0; i <= 21; ++i) {
        EXPECT_EQ(GetChildWidth(frameNode_, i), 116.25f);
    }

    layoutProperty_->UpdateColumnsGap(Dimension(1.0f));
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 21);
    for (int i = 0; i <= 21; ++i) {
        EXPECT_EQ(GetChildWidth(frameNode_, i), 119.25f);
    }
}

/**
 * @tc.name: Segmented001
 * @tc.desc: Layout WaterFlow with multiple sections
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Segmented001, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(600.f));
            CreateItem(60);
        },
        false);
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_4);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    LayoutConstraintF constraint { .maxSize = { 400.0f, 600.0f }, .percentReference = { 400.0f, 600.0f } };
    layoutProperty_->UpdateLayoutConstraint(constraint);
    layoutProperty_->UpdateContentConstraint();

    auto algo =
        AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_));

    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(algo->info_);
    algo->Measure(AceType::RawPtr(frameNode_));

    EXPECT_EQ(info->endIndex_, 12);
    EXPECT_EQ(info->margins_.size(), 4);
    EXPECT_EQ(info->segmentTails_, SEGMENT_TAILS_4);
    EXPECT_EQ(info->currentOffset_, 0.0f);
    const std::vector<float> crossGaps = { 0.0f, 0.0f, 0.0f };
    EXPECT_EQ(algo->mainGaps_.size(), 4);

    info->currentOffset_ = -200.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 3);
    EXPECT_EQ(info->endIndex_, 16);
    EXPECT_EQ(info->currentOffset_, -200.0f);
    EXPECT_EQ(info->segmentStartPos_.size(), 1);

    info->prevOffset_ = -200.0f;
    info->currentOffset_ = -4050.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 47);
    EXPECT_EQ(info->endIndex_, 51);
    EXPECT_EQ(info->segmentStartPos_.size(), 4);
    EXPECT_EQ(info->endPosArray_.size(), 37);
}

/**
 * @tc.name: Segmented005
 * @tc.desc: Layout WaterFlow with multiple sections
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Segmented005, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(600.f));
            CreateItem(60);
        },
        false);
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_4);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    LayoutConstraintF constraint { .maxSize = { 400.0f, 600.0f }, .percentReference = { 400.0f, 600.0f } };
    layoutProperty_->UpdateLayoutConstraint(constraint);
    layoutProperty_->UpdateContentConstraint();

    auto algo =
        AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_));
    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    info->jumpIndex_ = 50;
    info->align_ = ScrollAlign::END;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 47);
    EXPECT_EQ(info->endIndex_, 50);
    EXPECT_EQ(info->currentOffset_, -4000.0f);
    EXPECT_EQ(info->segmentStartPos_.size(), 4);
    pattern_->layoutInfo_ = info;

    secObj->ChangeData(0, 4, SECTION_5);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    EXPECT_EQ(secObj->GetSectionInfo().size(), 5);
    info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    EXPECT_EQ(info->startIndex_, 47);
    EXPECT_EQ(info->segmentTails_.size(), 5);
    EXPECT_EQ(info->segmentTails_[4], 59);
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 47);
    EXPECT_EQ(info->endIndex_, 54);
    EXPECT_EQ(info->currentOffset_, -5546.0f);
    EXPECT_EQ(info->segmentStartPos_.size(), 5);
    EXPECT_EQ(info->align_, ScrollAlign::START);
    EXPECT_EQ(algo->crossGaps_, CROSS_GAP_5);
    EXPECT_EQ(algo->mainGaps_, MAIN_GAP_5);
    EXPECT_EQ(algo->itemsCrossSize_.size(), 5);
}

/**
 * @tc.name: Segmented002
 * @tc.desc: Layout WaterFlow with multiple sections
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Segmented002, TestSize.Level1)
{
    SetUpConfig5();

    auto algo =
        AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_));

    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    algo->Measure(AceType::RawPtr(frameNode_));

    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 10);
    EXPECT_EQ(info->margins_.size(), 5);
    EXPECT_EQ(info->segmentTails_, SEGMENT_TAILS_5);
    EXPECT_EQ(info->currentOffset_, 0.0f);
    EXPECT_EQ(algo->crossGaps_, CROSS_GAP_5);
    EXPECT_EQ(algo->mainGaps_, MAIN_GAP_5);
    EXPECT_EQ(algo->itemsCrossSize_, ITEM_CROSS_SIZE_5);

    info->currentOffset_ = -200.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 3);
    EXPECT_EQ(info->endIndex_, 11);
    EXPECT_EQ(info->segmentStartPos_.size(), 4);

    info->currentOffset_ = -305.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 5);
    EXPECT_EQ(info->endIndex_, 12);
    EXPECT_EQ(info->segmentStartPos_.size(), 4);

    info->Reset();
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->currentOffset_, -305.0f);
    EXPECT_EQ(info->startIndex_, 5);
    EXPECT_EQ(info->endIndex_, 12);
    EXPECT_EQ(info->margins_.size(), 5);
    EXPECT_EQ(info->segmentTails_, SEGMENT_TAILS_5);
}

/**
 * @tc.name: Segmented003
 * @tc.desc: Layout WaterFlow with multiple sections
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Segmented003, TestSize.Level1)
{
    SetUpConfig5();
    auto algo =
        AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_));

    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    algo->Measure(AceType::RawPtr(frameNode_));

    info->currentOffset_ = -800.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 11);
    EXPECT_EQ(info->endIndex_, 15);
    EXPECT_EQ(info->segmentStartPos_.size(), 4);

    info->currentOffset_ = -1200.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 14);
    EXPECT_EQ(info->endIndex_, 18);

    info->currentOffset_ = -2300.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 21);
    EXPECT_EQ(info->endIndex_, 25);
    EXPECT_EQ(info->segmentStartPos_.size(), 4);

    info->prevOffset_ = -2300.0f;
    info->currentOffset_ = -1800.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 18);
    EXPECT_EQ(info->endIndex_, 22);
    EXPECT_EQ(info->segmentStartPos_.size(), 4);

    info->currentOffset_ = -10000.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 53);
    EXPECT_EQ(info->endIndex_, 59);
    EXPECT_EQ(info->currentOffset_, -6058.0f);
    EXPECT_EQ(*info->margins_[1].top, 1.0f);
    EXPECT_EQ(*info->margins_[1].bottom, 5.0f);
    EXPECT_EQ(info->segmentStartPos_.size(), 5);
    EXPECT_TRUE(info->offsetEnd_);
    EXPECT_TRUE(info->itemEnd_);
    EXPECT_FALSE(info->itemStart_);

    algo->Layout(AceType::RawPtr(frameNode_));
}

/**
 * @tc.name: Segmented004
 * @tc.desc: Layout WaterFlow and add a section
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Segmented004, TestSize.Level1)
{
    SetUpConfig5();

    auto algo =
        AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_));

    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->endIndex_, 10);
    EXPECT_EQ(info->itemInfos_.size(), 11);

    info->currentOffset_ = -800.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 11);
    EXPECT_EQ(info->endIndex_, 15);
    EXPECT_EQ(info->segmentStartPos_.size(), 4);
    EXPECT_EQ(info->itemInfos_.size(), 16);

    algo->Layout(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->itemInfos_.size(), 16);

    pattern_->layoutInfo_ = info;
    auto secObj = pattern_->GetSections();
    secObj->ChangeData(5, 0, ADD_SECTION_6);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    AddItems(10);
    info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    EXPECT_EQ(info->itemInfos_.size(), 16);

    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->currentOffset_, -800.0f);
    EXPECT_EQ(info->startIndex_, 11);
    EXPECT_EQ(info->endIndex_, 15);
    EXPECT_EQ(info->childrenCount_, 70);
    algo->Layout(AceType::RawPtr(frameNode_));

    info->prevOffset_ = -800.0f;
    info->currentOffset_ = -10000.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 63);
    EXPECT_EQ(info->endIndex_, 69);
    EXPECT_EQ(info->itemInfos_.size(), 70);
    EXPECT_EQ(info->itemInfos_[69].mainOffset, 7283.0f);
    EXPECT_EQ(info->itemInfos_[69].crossIdx, 0);
    EXPECT_EQ(info->itemInfos_[69].mainSize, 200.0f);
    algo->Layout(AceType::RawPtr(frameNode_));
}

/**
 * @tc.name: Segmented006
 * @tc.desc: Layout WaterFlow with SEGMENT_7 and change RowGaps
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Segmented006, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(600.f));
            CreateItem(37);
        },
        false);
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_7);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    FlushLayoutTask(frameNode_);

    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    EXPECT_EQ(*info->margins_[0].top, 5.0f);
    EXPECT_EQ(info->segmentStartPos_[0], 5.0f);
    EXPECT_EQ(info->segmentStartPos_[1], 408.0f);

    UpdateCurrentOffset(-600.0f);

    EXPECT_EQ(info->segmentStartPos_[2], 613.0f);
    EXPECT_EQ(info->segmentStartPos_[3], 621.0f);
    EXPECT_EQ(info->startIndex_, 6);

    layoutProperty_->UpdateRowsGap(10.0_vp);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(info->currentOffset_, -600.0f);
    EXPECT_EQ(info->startIndex_, 6);
    EXPECT_EQ(info->segmentStartPos_[0], 5.0f);
    EXPECT_EQ(info->segmentStartPos_[1], 438.0f);
    EXPECT_EQ(info->itemInfos_[4].mainOffset, 438.0f);
    EXPECT_EQ(info->segmentStartPos_[2], 653.0f);
    EXPECT_EQ(info->segmentStartPos_[3], 661.0f);

    UpdateCurrentOffset(600.0f);
    layoutProperty_->UpdateRowsGap(11.0_vp);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(info->currentOffset_, 0.0f);
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 6);
    EXPECT_EQ(info->segmentStartPos_[0], 5.0f);
    EXPECT_EQ(info->segmentStartPos_[1], 441.0f);
}

/**
 * @tc.name: Segmented007
 * @tc.desc: Layout WaterFlow with SEGMENT_7 and change RowGaps
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Segmented007, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(600.f));
            CreateItem(60);
        },
        false);
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_4);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(0), Rect(0, 0, 400.0f / 3, 100)));
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(2), Rect(400.0f / 3 * 2, 0, 400.0f / 3, 100)));

    layoutProperty_->UpdateLayoutDirection(TextDirection::RTL);
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(0), Rect(400.0f / 3 * 2, 0, 400.0f / 3, 100)));
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(2), Rect(0, 0, 400.0f / 3, 100)));
}

/**
 * @tc.name: CheckHeight001
 * @tc.desc: Layout WaterFlow and check if callback height is used
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, CheckHeight001, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(600.f));
            CreateItem(37);
        },
        false);
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_7);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    FlushLayoutTask(frameNode_);

    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);

    UpdateCurrentOffset(-10000.0f);
    EXPECT_EQ(info->currentOffset_, -3082.0f);
    EXPECT_EQ(info->startIndex_, 31);
    EXPECT_EQ(info->endIndex_, 36);
    for (int i = 31; i <= 36; ++i) {
        EXPECT_EQ(GetChildHeight(frameNode_, i), 100.0f);
    }

    for (const auto& child : frameNode_->GetChildren()) {
        auto node = AceType::DynamicCast<FrameNode>(child);
        node->GetGeometryNode()->SetFrameSize({});
        node->GetLayoutProperty()->UpdateUserDefinedIdealSize({ CalcLength(50.0f), CalcLength(50.0f) });
    }

    UpdateCurrentOffset(10000.0f);
    EXPECT_EQ(info->currentOffset_, 0.0f);
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 6);
    for (int i = 0; i <= 6; ++i) {
        EXPECT_EQ(GetChildHeight(frameNode_, i), 100.0f);
    }
}

/**
 * @tc.name: TargetIndex001
 * @tc.desc: Layout WaterFlow with target index
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, TargetIndex001, TestSize.Level1)
{
    SetUpConfig5();

    auto algo =
        AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_));

    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    info->targetIndex_ = 50;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->currentOffset_, 0.0f);
    EXPECT_EQ(info->segmentStartPos_.size(), 5);
    EXPECT_EQ(info->itemInfos_.size(), 51);

    algo->Layout(AceType::RawPtr(frameNode_));
}

/**
 * @tc.name: ChildrenCount001
 * @tc.desc: Layout WaterFlow with fewer children than claimed in sectionInfo
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, ChildrenCount001, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(600.f));
            CreateItem(40);
        },
        false);
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_5);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    LayoutConstraintF constraint { .maxSize = { 400.0f, 600.0f }, .percentReference = { 400.0f, 600.0f } };
    layoutProperty_->UpdateLayoutConstraint(constraint);
    layoutProperty_->UpdateContentConstraint();

    auto algo =
        AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_));

    // cause layout abort
    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    info->targetIndex_ = 50;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->endIndex_, -1);
    EXPECT_EQ(info->currentOffset_, 0.0f);
    EXPECT_EQ(info->segmentStartPos_.size(), 1);
    EXPECT_EQ(info->itemInfos_.size(), 0);

    algo->Layout(AceType::RawPtr(frameNode_));

    info->currentOffset_ = -1050.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, -1);
    EXPECT_EQ(info->segmentStartPos_.size(), 1);

    info->prevOffset_ = -1050.0f;
    info->currentOffset_ = -10000.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    // as long as no crash happens
    EXPECT_EQ(info->segmentStartPos_.size(), 1);
    EXPECT_EQ(info->itemInfos_.size(), 0);
}

/**
 * @tc.name: ChildrenCount002
 * @tc.desc: Layout WaterFlow with more children than claimed in sectionInfo
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, ChildrenCount002, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(600.f));
            CreateItem(80);
        },
        false);
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_5);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    LayoutConstraintF constraint { .maxSize = { 400.0f, 600.0f }, .percentReference = { 400.0f, 600.0f } };
    layoutProperty_->UpdateLayoutConstraint(constraint);
    layoutProperty_->UpdateContentConstraint();

    auto algo =
        AceType::MakeRefPtr<WaterFlowSegmentedLayout>(AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_));
    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);

    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, -1);
    EXPECT_EQ(info->segmentStartPos_.size(), 1);

    info->currentOffset_ = -10000.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, -1);
    EXPECT_EQ(info->segmentStartPos_.size(), 1);
    EXPECT_EQ(info->itemInfos_.size(), 0);
    algo->Layout(AceType::RawPtr(frameNode_));

    info->jumpIndex_ = 70;
    info->align_ = ScrollAlign::AUTO;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, -1);
    algo->Layout(AceType::RawPtr(frameNode_));
}


/**
 * @tc.name: Illegal001
 * @tc.desc: Layout WaterFlow with empty sections.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Illegal001, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(600.f));
            CreateItem(37);
        },
        false);
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, {});
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    FlushLayoutTask(frameNode_);
    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);

    UpdateCurrentOffset(-205.0f);
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, -1);
}

/**
 * @tc.name: Illegal002
 * @tc.desc: Layout WaterFlow with negative main size.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Illegal002, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(600.f));
            CreateItem(10);
        },
        false);
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_8);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    FlushLayoutTask(frameNode_);
    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);

    // user-defined negative size would be treated as 0
    UpdateCurrentOffset(-205.0f);
    EXPECT_EQ(info->currentOffset_, 0.0f);
    EXPECT_EQ(info->itemInfos_[0].mainSize, 0.0f);
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 9);
}

/**
 * @tc.name: Illegal003
 * @tc.desc: Layout WaterFlow without items.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Illegal003, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(600.f));
            CreateItem(6);
        },
        false);
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_9);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    FlushLayoutTask(frameNode_);
    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);
    EXPECT_EQ(info->endIndex_, 5);

    auto newSection = SECTION_8;
    newSection[0].itemsCount = 0;
    secObj->ChangeData(0, 1, newSection);
    for (int i = 0; i < 10; ++i) {
        frameNode_->RemoveChildAtIndex(0);
    }
    frameNode_->ChildrenUpdatedFrom(0);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    FlushLayoutTask(frameNode_);

    pattern_->ScrollToIndex(LAST_ITEM);
    EXPECT_EQ(info->jumpIndex_, LAST_ITEM);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(info->endIndex_, -1);
    EXPECT_EQ(info->childrenCount_, 0);
}

/**
 * @tc.name: Constraint001
 * @tc.desc: Test Layout when the layoutConstraint changes.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Constraint001, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(600.f));
            CreateItem(60);
        },
        false);
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_5);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    FlushLayoutTask(frameNode_);

    auto& info = pattern_->layoutInfo_;
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 10);
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(0), Rect(0, 0, 400.f / 3, 100)));

    layoutProperty_->UpdateUserDefinedIdealSize(CalcSize(CalcLength(500.0f), CalcLength(Dimension(600.0f))));
    FlushLayoutTask(frameNode_);
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(GetChildWidth(frameNode_, i), 500.f / 3);
    }
    for (int i = 5; i < 10; i++) {
        EXPECT_EQ(GetChildWidth(frameNode_, i), (500.f - 3) / 5);
    }
    EXPECT_EQ(GetChildWidth(frameNode_, 10), 500.f);
    EXPECT_EQ(info->endIndex_, 10);

    layoutProperty_->UpdateUserDefinedIdealSize(CalcSize(CalcLength(400.0f), CalcLength(Dimension(700.0f))));
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(0), Rect(0, 0, 400.f / 3, 100)));
    EXPECT_EQ(info->endIndex_, 11);

    layoutProperty_->UpdateUserDefinedIdealSize(CalcSize(CalcLength(500.0f), CalcLength(Dimension(700.0f))));
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(IsEqual(pattern_->GetItemRect(0), Rect(0, 0, 500.f / 3, 100)));
    EXPECT_EQ(info->endIndex_, 11);
}

/**
 * @tc.name: ResetSections001
 * @tc.desc: Layout WaterFlow and then reset to old layout
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, ResetSections001, TestSize.Level1)
{
    Create(
        [](WaterFlowModelNG model) {
            ViewAbstract::SetWidth(CalcLength(400.0f));
            ViewAbstract::SetHeight(CalcLength(600.f));
            CreateItem(60);
        },
        false);
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_5);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    FlushLayoutTask(frameNode_);
    auto info = AceType::DynamicCast<WaterFlowLayoutInfo>(pattern_->layoutInfo_);

    UpdateCurrentOffset(-205.0f);
    EXPECT_EQ(info->currentOffset_, -205.0f);
    EXPECT_EQ(info->startIndex_, 3);
    EXPECT_EQ(info->endIndex_, 11);

    // fallback to layout without sections
    pattern_->ResetSections();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(info->currentOffset_, -205.0f);
    EXPECT_EQ(info->startIndex_, 1);
    EXPECT_EQ(info->endIndex_, 5);
    EXPECT_EQ(info->GetCrossCount(), 1);
    if (SystemProperties::WaterFlowUseSegmentedLayout()) {
        EXPECT_EQ(info->segmentTails_.size(), 1);
        EXPECT_EQ(info->margins_.size(), 1);
    } else {
        EXPECT_TRUE(info->segmentTails_.empty());
        EXPECT_TRUE(info->margins_.empty());
    }

    UpdateCurrentOffset(250.0f);
    EXPECT_EQ(info->currentOffset_, 0.0f);
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 3);
}
} // namespace OHOS::Ace::NG
