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

#include "core/components_ng/pattern/waterflow/water_flow_layout_info.h"
#include "core/components_ng/property/measure_property.h"

#define protected public
#define private public
#include "frameworks/core/components_ng/pattern/waterflow/water_flow_segmented_layout.h"
#undef private
#undef protected

namespace OHOS::Ace::NG {
class WaterFlowSegmentTest : public WaterFlowTestNg {
public:
    void SetUpConfig2();
};

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
    layoutProperty_->layoutConstraint_ = constraint;
    layoutProperty_->contentConstraint_ = constraint;
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

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo {});
    algo->wrapper_ = AceType::RawPtr(frameNode_);
    algo->mainSize_ = 2000.0f;
    algo->itemsCrossSize_ = { { 50.0f, 50.0f, 50.0f, 50.0f }, {}, { 70.0f, 70.0f, 70.0f } };
    algo->mainGaps_ = { 5.0f, 0.0f, 1.0f };

    auto& info = algo->info_;
    info.margins_ = { {}, {}, PaddingPropertyF { .top = 5.0f } };
    info.childrenCount_ = 10;

    info.items_.resize(3);
    for (int i = 0; i < 3; ++i) {
        info.items_[0][i] = {};
        info.items_[2][i] = {};
    }
    info.items_[0][3] = {};

    info.segmentTails_ = SEGMENT_TAILS_1;

    algo->Fill(0);
    EXPECT_EQ(info.items_, ITEM_MAP_1);
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
    layoutProperty_->layoutConstraint_ = constraint;
    layoutProperty_->contentConstraint_ = constraint;

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo {});
    auto& info = algo->info_;

    info.footerIndex_ = 0;

    for (int i = 0; i < 2; ++i) {
        algo->Measure(AceType::RawPtr(frameNode_));

        EXPECT_EQ(info.childrenCount_, 11);
        EXPECT_EQ(info.items_, ITEM_MAP_2);
        EXPECT_EQ(info.itemInfos_, ITEM_INFO_2);
        EXPECT_EQ(info.segmentTails_, SEGMENT_TAILS_2);
        EXPECT_EQ(info.endPosArray_, END_POS_ARRAY_2);
        EXPECT_EQ(info.segmentStartPos_, SEGMENT_START_POS_2);
    }

    info.prevOffset_ = 0.0f;
    info.currentOffset_ = -100.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.currentOffset_, 0.0f);
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 10);

    algo->overScroll_ = true;
    info.prevOffset_ = 0.0f;
    info.currentOffset_ = -200.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.currentOffset_, -200.0f);
    EXPECT_EQ(info.startIndex_, 4);
    EXPECT_EQ(info.endIndex_, 10);

    info.Reset();
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.currentOffset_, -200.0f);
    EXPECT_EQ(info.startIndex_, 4);
    EXPECT_EQ(info.endIndex_, 10);
}

/**
 * @tc.name: Layout001
 * @tc.desc: Test SegmentedLayout::Layout.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Layout001, TestSize.Level1)
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
    layoutProperty_->layoutConstraint_ = constraint;
    layoutProperty_->contentConstraint_ = constraint;

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo {});
    auto& info = algo->info_;

    info.footerIndex_ = 0;

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

    info.prevOffset_ = 0.0f;
    info.currentOffset_ = -100.0f;
    algo->overScroll_ = true;
    algo->Measure(AceType::RawPtr(frameNode_));
    algo->Layout(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 1);
    EXPECT_EQ(info.endIndex_, 10);
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

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo {});
    auto& info = algo->info_;

    info.footerIndex_ = 0;

    for (int i = 0; i < 2; ++i) {
        algo->Measure(AceType::RawPtr(frameNode_));

        EXPECT_EQ(info.childrenCount_, 101);
        EXPECT_EQ(info.items_.size(), 2);
        EXPECT_EQ(info.startIndex_, 0);
        EXPECT_EQ(info.endIndex_, 27);
        EXPECT_EQ(info.segmentTails_, SEGMENT_TAILS_3);
    }

    info.prevOffset_ = 0.0f;
    info.currentOffset_ = -100.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.currentOffset_, -100.0f);
    EXPECT_EQ(info.startIndex_, 1);
    EXPECT_EQ(info.endIndex_, 30);

    info.prevOffset_ = -100.0f;
    info.currentOffset_ = -500.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.currentOffset_, -500.0f);
    EXPECT_EQ(info.startIndex_, 11);
    EXPECT_EQ(info.endIndex_, 44);

    const auto itemMap = info.items_;
    const auto itemInfo = info.itemInfos_;
    const auto endPosArr = info.endPosArray_;

    info.prevOffset_ = -500.0f;
    info.currentOffset_ = -300.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.items_, itemMap);
    EXPECT_EQ(info.itemInfos_, itemInfo);
    EXPECT_EQ(info.endPosArray_, endPosArr);
    EXPECT_EQ(info.startIndex_, 5);
    EXPECT_EQ(info.endIndex_, 37);
    EXPECT_EQ(info.segmentStartPos_, std::vector<float> { 0.0f });

    info.prevOffset_ = -300.0f;
    info.currentOffset_ = -700.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 19);
    EXPECT_EQ(info.endIndex_, 50);
    EXPECT_EQ(info.segmentStartPos_, std::vector<float> { 0.0f });
}

/**
 * @tc.name: MeasureOnJump001
 * @tc.desc: Test SegmentedLayout::MeasureOnJump END.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, MeasureOnJump001, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo {});
    auto& info = algo->info_;

    info.footerIndex_ = 0;

    info.align_ = ScrollAlign::END;
    info.jumpIndex_ = 5;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 27);
    EXPECT_EQ(info.currentOffset_, 0.0f);

    info.jumpIndex_ = 99;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 75);
    EXPECT_EQ(info.endIndex_, 99);
    EXPECT_EQ(info.currentOffset_, -2320.0f);

    info.jumpIndex_ = 100;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 75);
    EXPECT_EQ(info.endIndex_, 100);
    EXPECT_EQ(info.currentOffset_, -2370.0f);

    info.jumpIndex_ = 105;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 75);
    EXPECT_EQ(info.endIndex_, 100);
    EXPECT_EQ(info.currentOffset_, -2370.0f);
}

/**
 * @tc.name: MeasureOnJump002
 * @tc.desc: Test SegmentedLayout::MeasureOnJump with AUTO scroll.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, MeasureOnJump002, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo {});
    auto& info = algo->info_;

    info.footerIndex_ = 0;

    info.align_ = ScrollAlign::AUTO;
    info.jumpIndex_ = 10;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 27);
    EXPECT_EQ(info.currentOffset_, 0.0f);
    EXPECT_EQ(info.align_, ScrollAlign::NONE);

    info.align_ = ScrollAlign::AUTO;
    info.jumpIndex_ = 53;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 29);
    EXPECT_EQ(info.endIndex_, 58);
    EXPECT_EQ(info.currentOffset_, -911.0f);
    EXPECT_EQ(info.align_, ScrollAlign::END);

    info.align_ = ScrollAlign::AUTO;
    info.jumpIndex_ = 5;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 1);
    EXPECT_EQ(info.endIndex_, 30);
    EXPECT_EQ(info.currentOffset_, -101.0f);
    EXPECT_EQ(info.align_, ScrollAlign::START);

    info.align_ = ScrollAlign::AUTO;
    info.jumpIndex_ = 5;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.align_, ScrollAlign::NONE);

    info.align_ = ScrollAlign::AUTO;
    info.jumpIndex_ = 7;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 1);
    EXPECT_EQ(info.endIndex_, 30);
    EXPECT_EQ(info.currentOffset_, -101.0f);
    EXPECT_EQ(info.align_, ScrollAlign::NONE);
}

/**
 * @tc.name: MeasureOnJump003
 * @tc.desc: Test SegmentedLayout::MeasureOnJump START.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, MeasureOnJump003, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo {});
    auto& info = algo->info_;

    info.footerIndex_ = 0;

    info.align_ = ScrollAlign::START;
    info.jumpIndex_ = 10;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 5);
    EXPECT_EQ(info.endIndex_, 34);
    EXPECT_EQ(info.currentOffset_, -202.0f);

    info.jumpIndex_ = 99;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 75);
    EXPECT_EQ(info.endIndex_, 100);
    EXPECT_EQ(info.currentOffset_, -2370.0f);

    info.jumpIndex_ = 42;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 37);
    EXPECT_EQ(info.endIndex_, 67);
    EXPECT_EQ(info.currentOffset_, -1207.0f);
}

/**
 * @tc.name: MeasureOnJump004
 * @tc.desc: Test SegmentedLayout::MeasureOnJump CENTER.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, MeasureOnJump004, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo {});
    auto& info = algo->info_;

    info.footerIndex_ = 0;

    info.align_ = ScrollAlign::CENTER;
    info.jumpIndex_ = 10;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 27);
    EXPECT_EQ(info.currentOffset_, -0.0f);

    info.jumpIndex_ = 99;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 75);
    EXPECT_EQ(info.endIndex_, 100);
    EXPECT_EQ(info.currentOffset_, -2370.0f);
    EXPECT_EQ(info.segmentStartPos_.size(), 2);

    info.jumpIndex_ = 42;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 25);
    EXPECT_EQ(info.endIndex_, 57);
    EXPECT_EQ(info.currentOffset_, -857.0f);

    info.jumpIndex_ = 0;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 27);
    EXPECT_EQ(info.currentOffset_, 0.0f);
}

/**
 * @tc.name: Reset001
 * @tc.desc: Test SegmentedLayout::CheckReset.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Reset001, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo {});
    auto& info = algo->info_;

    info.footerIndex_ = 0;

    info.align_ = ScrollAlign::CENTER;

    info.jumpIndex_ = 99;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 75);
    EXPECT_EQ(info.endIndex_, 100);
    EXPECT_EQ(info.currentOffset_, -2370.0f);
    EXPECT_EQ(info.segmentStartPos_.size(), 2);

    // change crossCount, should jump back to index 75
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr 1fr");
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 75);
    EXPECT_EQ(info.endIndex_, 94);
    EXPECT_EQ(info.currentOffset_, -3875.0f);
    EXPECT_EQ(algo->itemsCrossSize_[0].size(), 3);
    EXPECT_EQ(info.align_, ScrollAlign::START);
}

/**
 * @tc.name: Reset002
 * @tc.desc: Test SegmentedLayout::CheckReset.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Reset002, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo {});
    auto& info = algo->info_;

    info.footerIndex_ = 0;

    info.align_ = ScrollAlign::CENTER;
    info.jumpIndex_ = 99;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 75);
    EXPECT_EQ(info.endIndex_, 100);
    EXPECT_EQ(info.currentOffset_, -2370.0f);
    EXPECT_EQ(info.segmentStartPos_.size(), 2);

    info.jumpIndex_ = 42;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 25);
    EXPECT_EQ(info.endIndex_, 57);
    EXPECT_EQ(info.currentOffset_, -857.0f);

    // child requires fresh layout, should jump back to index 75
    layoutProperty_->propertyChangeFlag_ = PROPERTY_UPDATE_BY_CHILD_REQUEST;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 25);
    EXPECT_EQ(info.endIndex_, 57);
    EXPECT_EQ(info.currentOffset_, -857.0f);
    EXPECT_EQ(info.align_, ScrollAlign::START);
    // items should be cleared before jumping
    EXPECT_EQ(info.items_[1][0].size(), 0);
    EXPECT_EQ(info.segmentStartPos_.size(), 1);
    EXPECT_EQ(info.itemInfos_.size(), 58);
}

/**
 * @tc.name: Reset003
 * @tc.desc: Test SegmentedLayout::CheckReset.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSegmentTest, Reset003, TestSize.Level1)
{
    SetUpConfig2();

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo {});
    auto& info = algo->info_;

    info.footerIndex_ = 0;

    info.align_ = ScrollAlign::CENTER;
    info.jumpIndex_ = 99;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 75);
    EXPECT_EQ(info.endIndex_, 100);
    EXPECT_EQ(info.currentOffset_, -2370.0f);
    EXPECT_EQ(info.segmentStartPos_.size(), 2);
    EXPECT_EQ(info.itemInfos_.size(), 101);

    info.jumpIndex_ = 42;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 25);
    EXPECT_EQ(info.endIndex_, 57);
    EXPECT_EQ(info.currentOffset_, -857.0f);

    // index 70 doesn't affect the current layout
    frameNode_->ChildrenUpdatedFrom(70);
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 25);
    EXPECT_EQ(info.endIndex_, 57);
    EXPECT_EQ(info.currentOffset_, -857.0f);
    EXPECT_EQ(info.align_, ScrollAlign::CENTER);
    // items starting from 70 are cleared
    EXPECT_EQ(info.items_[1][0].size(), 0);
    EXPECT_EQ(info.segmentStartPos_.size(), 1);
    EXPECT_EQ(info.itemInfos_.size(), 70);

    // index 20 would reset all and trigger jump
    frameNode_->ChildrenUpdatedFrom(20);
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 25);
    EXPECT_EQ(info.endIndex_, 57);
    EXPECT_EQ(info.currentOffset_, -857.0f);
    EXPECT_EQ(info.align_, ScrollAlign::START);
    // items should be cleared before jumping
    EXPECT_EQ(info.itemInfos_.size(), 58);
}

/**
 * @tc.name: Segmented001
 * @tc.desc: Layout WaterFlow with multiple sections
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, Segmented001, TestSize.Level1)
{
    Create([](WaterFlowModelNG model) {
        ViewAbstract::SetWidth(CalcLength(400.0f));
        ViewAbstract::SetHeight(CalcLength(600.f));
        CreateItem(60);
    });
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_4);

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo{});

    auto& info = algo->info_;
    algo->Measure(AceType::RawPtr(frameNode_));

    EXPECT_EQ(info.endIndex_, 12);
    EXPECT_EQ(info.margins_.size(), 3);
    EXPECT_EQ(info.segmentTails_, SEGMENT_TAILS_4);
    EXPECT_EQ(info.currentOffset_, 0.0f);
    EXPECT_EQ(algo->crossGaps_.size(), 3);
    EXPECT_EQ(algo->mainGaps_.size(), 3);

    info.currentOffset_ = -200.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 3);
    EXPECT_EQ(info.endIndex_, 16);
    EXPECT_EQ(info.currentOffset_, -200.0f);
    EXPECT_EQ(info.segmentStartPos_.size(), 1);

    info.jumpIndex_ = 50;
    info.align_ = ScrollAlign::END;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 47);
    EXPECT_EQ(info.endIndex_, 50);
    EXPECT_EQ(info.currentOffset_, -4000.0f);
    EXPECT_EQ(info.segmentStartPos_.size(), 3);

    secObj->ChangeData(0, 3, SECTION_5);
    // section change
    info.segmentTails_.clear();
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 47);
    EXPECT_EQ(info.endIndex_, 54);
    EXPECT_EQ(info.currentOffset_, -5546.0f);
    EXPECT_EQ(info.segmentStartPos_.size(), 4);
    EXPECT_EQ(info.align_, ScrollAlign::START);
    EXPECT_EQ(algo->crossGaps_, CROSS_GAP_5);
    EXPECT_EQ(algo->mainGaps_, MAIN_GAP_5);
    EXPECT_EQ(algo->itemsCrossSize_.size(), 4);
}

/**
 * @tc.name: Segmented002
 * @tc.desc: Layout WaterFlow with multiple sections
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, Segmented002, TestSize.Level1)
{
    Create([](WaterFlowModelNG model) {
        ViewAbstract::SetWidth(CalcLength(400.0f));
        ViewAbstract::SetHeight(CalcLength(600.f));
        CreateItem(60);
    });
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_5);

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo{});

    auto& info = algo->info_;
    algo->Measure(AceType::RawPtr(frameNode_));

    EXPECT_EQ(info.startIndex_, 0);
    EXPECT_EQ(info.endIndex_, 10);
    EXPECT_EQ(info.margins_.size(), 4);
    EXPECT_EQ(info.segmentTails_, SEGMENT_TAILS_5);
    EXPECT_EQ(info.currentOffset_, 0.0f);
    EXPECT_EQ(algo->crossGaps_.size(), 4);
    EXPECT_EQ(algo->mainGaps_.size(), 4);

    info.currentOffset_ = -200.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 3);
    EXPECT_EQ(info.endIndex_, 11);
    EXPECT_EQ(info.segmentStartPos_.size(), 3);

    info.currentOffset_ = -304.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 3);
    EXPECT_EQ(info.endIndex_, 12);
    EXPECT_EQ(info.segmentStartPos_.size(), 3);

    info.currentOffset_ = -305.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 5);
    EXPECT_EQ(info.endIndex_, 12);
    EXPECT_EQ(info.segmentStartPos_.size(), 3);

    info.Reset();
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.currentOffset_, -305.0f);
    EXPECT_EQ(info.startIndex_, 5);
    EXPECT_EQ(info.endIndex_, 12);
    EXPECT_EQ(info.margins_.size(), 4);
    EXPECT_EQ(info.segmentTails_, SEGMENT_TAILS_5);
}

/**
 * @tc.name: Segmented003
 * @tc.desc: Layout WaterFlow with multiple sections
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, Segmented003, TestSize.Level1)
{
    Create([](WaterFlowModelNG model) {
        ViewAbstract::SetWidth(CalcLength(400.0f));
        ViewAbstract::SetHeight(CalcLength(600.f));
        CreateItem(60);
    });
    auto secObj = pattern_->GetOrCreateWaterFlowSections();
    secObj->ChangeData(0, 0, SECTION_5);

    auto algo = AceType::MakeRefPtr<WaterFlowSegmentedLayout>(WaterFlowLayoutInfo{});

    auto& info = algo->info_;
    algo->Measure(AceType::RawPtr(frameNode_));

    info.currentOffset_ = -800.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 11);
    EXPECT_EQ(info.endIndex_, 15);
    EXPECT_EQ(info.segmentStartPos_.size(), 3);

    info.currentOffset_ = -1200.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 14);
    EXPECT_EQ(info.endIndex_, 18);

    info.currentOffset_ = -2300.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 21);
    EXPECT_EQ(info.endIndex_, 25);
    EXPECT_EQ(info.segmentStartPos_.size(), 3);

    info.prevOffset_ = -2300.0f;
    info.currentOffset_ = -1800.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 18);
    EXPECT_EQ(info.endIndex_, 22);
    EXPECT_EQ(info.segmentStartPos_.size(), 3);

    info.currentOffset_ = -10000.0f;
    algo->Measure(AceType::RawPtr(frameNode_));
    EXPECT_EQ(info.startIndex_, 53);
    EXPECT_EQ(info.endIndex_, 59);
    EXPECT_EQ(info.currentOffset_, -6058.0f);
    EXPECT_EQ(info.segmentStartPos_.size(), 4);

    algo->Layout(AceType::RawPtr(frameNode_));
}
} // namespace OHOS::Ace::NG