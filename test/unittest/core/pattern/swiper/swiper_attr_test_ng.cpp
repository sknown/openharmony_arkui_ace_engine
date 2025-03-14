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

#include "swiper_test_ng.h"

namespace OHOS::Ace::NG {

namespace {
const InspectorFilter filter;
} // namespace

class SwiperAttrTestNg : public SwiperTestNg {
public:
};

/**
 * @tc.name: AttrIndex001
 * @tc.desc: Test property about index
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrIndex001, TestSize.Level1)
{
    /**
     * @tc.cases: Do not set index
     * @tc.expected: Default show(size>0) first item
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_GT(GetChildWidth(frameNode_, 0), 0.f); // item size > 0
}

/**
 * @tc.name: AttrIndex002
 * @tc.desc: Test property about index
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrIndex002, TestSize.Level1)
{
    /**
     * @tc.cases: Set index:1
     * @tc.expected: Default show(size>0) second item
     */
    CreateWithItem([](SwiperModelNG model) { model.SetIndex(1); });
    EXPECT_GT(GetChildWidth(frameNode_, 1), 0.f);
}

/**
 * @tc.name: AttrIndex003
 * @tc.desc: Test property about index
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrIndex003, TestSize.Level1)
{
    /**
     * @tc.cases: Set invalid index:-1
     * @tc.expected: Default show(size>0) first item
     */
    CreateWithItem([](SwiperModelNG model) { model.SetIndex(-1); });
    EXPECT_GT(GetChildWidth(frameNode_, 0), 0.f);
}

/**
 * @tc.name: AttrIndex004
 * @tc.desc: Test property about index
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrIndex004, TestSize.Level1)
{
    /**
     * @tc.cases: Set invalid index:ITEM_NUMBER(index>maxIndex)
     * @tc.expected: Default show(size>0) first item
     */
    CreateWithItem([](SwiperModelNG model) { model.SetIndex(ITEM_NUMBER); });
    EXPECT_GT(GetChildWidth(frameNode_, 0), 0.f);
}

/**
 * @tc.name: AttrAutoPlay001
 * @tc.desc: Test property about autoPlay/interval/loop
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrAutoPlay001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Do not set value
     * @tc.expected: AutoPlay is false, interval is default, isLoop is true
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_FALSE(pattern_->IsAutoPlay());
    EXPECT_EQ(pattern_->GetInterval(), DEFAULT_INTERVAL);
    EXPECT_TRUE(pattern_->IsLoop());
}

/**
 * @tc.name: AttrAutoPlay002
 * @tc.desc: Test property about autoPlay/interval/loop
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrAutoPlay002, TestSize.Level1)
{
    /**
     * @tc.cases: Set autoPlay to true
     * @tc.expected: AutoPlay is true
     */
    CreateWithItem([](SwiperModelNG model) { model.SetAutoPlay(true); });
    EXPECT_TRUE(pattern_->IsAutoPlay());
}

/**
 * @tc.name: AttrAutoPlay003
 * @tc.desc: Test property about autoPlay/interval/loop
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrAutoPlay003, TestSize.Level1)
{
    /**
     * @tc.cases: Set autoPlay to true, set autoPlayInterval to 4000
     * @tc.expected: AutoPlayInterval is 4000
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetAutoPlay(true);
        model.SetAutoPlayInterval(4000);
    });
    EXPECT_EQ(pattern_->GetInterval(), 4000);
}

/**
 * @tc.name: AttrAutoPlay004
 * @tc.desc: Test property about autoPlay/interval/loop
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrAutoPlay004, TestSize.Level1)
{
    /**
     * @tc.cases: Set autoPlay to true, set loop to false
     * @tc.expected: Loop is false
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetAutoPlay(true);
        model.SetLoop(false);
    });
    EXPECT_FALSE(pattern_->IsLoop());
}

/**
 * @tc.name: AttrIndicator001
 * @tc.desc: Test property about indicator
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrIndicator001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Do not set value
     * @tc.expected: Show indicator, indicator type is DOT
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_TRUE(pattern_->IsShowIndicator());
    EXPECT_EQ(pattern_->GetIndicatorType(), SwiperIndicatorType::DOT);
    EXPECT_EQ(frameNode_->GetTotalChildCount(), 5); // 4 items and indicator
    EXPECT_EQ(indicatorNode_->GetTag(), V2::SWIPER_INDICATOR_ETS_TAG);
    EXPECT_TRUE(IsEqual(GetChildRect(frameNode_, 4), RectF(196.05f, 768.02f, 87.9f, 31.98f)));

    auto wrapper = FlushLayoutTask(indicatorNode_);
    auto paintMethod = AceType::DynamicCast<DotIndicatorPaintMethod>(wrapper->nodePaintImpl_);
    EXPECT_FALSE(paintMethod->dotIndicatorModifier_->indicatorMask_);
    EXPECT_EQ(paintMethod->dotIndicatorModifier_->unselectedColor_->Get(), Color::FromString("#182431"));
    EXPECT_EQ(paintMethod->dotIndicatorModifier_->selectedColor_->Get().ToColor(), Color::FromString("#007DFF"));
    LinearVector<float> itemHalfSizes;
    itemHalfSizes.emplace_back(3.f);
    itemHalfSizes.emplace_back(3.f);
    itemHalfSizes.emplace_back(3.f);
    itemHalfSizes.emplace_back(3.f);
    EXPECT_EQ(paintMethod->dotIndicatorModifier_->itemHalfSizes_->Get(), itemHalfSizes);
}

/**
 * @tc.name: AttrIndicator002
 * @tc.desc: Test property about indicator
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrIndicator002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set value
     * @tc.expected: Show indicator, indicator type is DOT
     */
    SwiperParameters swiperParameters;
    swiperParameters.dimLeft = std::make_optional<Dimension>(10.f);
    swiperParameters.dimTop = std::make_optional<Dimension>(10.f);
    swiperParameters.dimRight = std::make_optional<Dimension>(10.f);
    swiperParameters.dimBottom = std::make_optional<Dimension>(10.f);
    swiperParameters.itemWidth = std::make_optional<Dimension>(10.f);
    swiperParameters.itemHeight = std::make_optional<Dimension>(10.f);
    swiperParameters.selectedItemWidth = std::make_optional<Dimension>(10.f);
    swiperParameters.selectedItemHeight = std::make_optional<Dimension>(10.f);
    swiperParameters.maskValue = std::make_optional<bool>(true);
    swiperParameters.colorVal = std::make_optional<Color>(Color::RED);
    swiperParameters.selectedColorVal = std::make_optional<Color>(Color::GREEN);
    CreateWithItem([=](SwiperModelNG model) {
        model.SetDotIndicatorStyle(swiperParameters);
    });
    EXPECT_TRUE(pattern_->IsShowIndicator());
    EXPECT_EQ(pattern_->GetIndicatorType(), SwiperIndicatorType::DOT);
    EXPECT_EQ(frameNode_->GetTotalChildCount(), 5); // 4 items and indicator
    EXPECT_EQ(indicatorNode_->GetTag(), V2::SWIPER_INDICATOR_ETS_TAG);
    EXPECT_TRUE(IsEqual(GetChildRect(frameNode_, 4), RectF(10.f, 10.f, 114.5f, 37.3f)));

    auto wrapper = FlushLayoutTask(indicatorNode_);
    auto paintMethod = AceType::DynamicCast<DotIndicatorPaintMethod>(wrapper->nodePaintImpl_);
    EXPECT_TRUE(paintMethod->dotIndicatorModifier_->indicatorMask_);
    EXPECT_EQ(paintMethod->dotIndicatorModifier_->unselectedColor_->Get(), Color::RED);
    EXPECT_EQ(paintMethod->dotIndicatorModifier_->selectedColor_->Get().ToColor(), Color::GREEN);
    LinearVector<float> itemHalfSizes;
    itemHalfSizes.emplace_back(5.f);
    itemHalfSizes.emplace_back(5.f);
    itemHalfSizes.emplace_back(5.f);
    itemHalfSizes.emplace_back(5.f);
    EXPECT_EQ(paintMethod->dotIndicatorModifier_->itemHalfSizes_->Get(), itemHalfSizes);
}

/**
 * @tc.name: AttrIndicator003
 * @tc.desc: Test property about indicator
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrIndicator003, TestSize.Level1)
{
    /**
     * @tc.cases: Set indicator type to DIGIT
     * @tc.expected: Show indicator, indicator type is DIGIT
     */
    CreateWithItem([](SwiperModelNG model) { model.SetIndicatorType(SwiperIndicatorType::DIGIT); });
    EXPECT_EQ(pattern_->GetIndicatorType(), SwiperIndicatorType::DIGIT);
    auto firstTextNode = GetChildFrameNode(indicatorNode_, 0);
    auto lastTextNode = GetChildFrameNode(indicatorNode_, 1);
    auto firstTextLayoutProperty = firstTextNode->GetLayoutProperty<TextLayoutProperty>();
    auto lastTextLayoutProperty = lastTextNode->GetLayoutProperty<TextLayoutProperty>();
    EXPECT_EQ(firstTextLayoutProperty->GetTextColor(), INDICATOR_TEXT_FONT_COLOR);
    EXPECT_EQ(firstTextLayoutProperty->GetFontSize(), INDICATOR_TEXT_FONT_SIZE);
    EXPECT_EQ(firstTextLayoutProperty->GetFontWeight(), INDICATOR_TEXT_FONT_WEIGHT);
    EXPECT_EQ(lastTextLayoutProperty->GetTextColor(), INDICATOR_TEXT_FONT_COLOR);
    EXPECT_EQ(lastTextLayoutProperty->GetFontSize(), INDICATOR_TEXT_FONT_SIZE);
    EXPECT_EQ(lastTextLayoutProperty->GetFontWeight(), INDICATOR_TEXT_FONT_WEIGHT);
}

/**
 * @tc.name: AttrIndicator004
 * @tc.desc: Test property about indicator
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrIndicator004, TestSize.Level1)
{
    /**
     * @tc.cases: Set indicator type to DIGIT
     * @tc.expected: Show indicator, indicator type is DIGIT
     */
    SwiperDigitalParameters swiperDigitalParameters;
    swiperDigitalParameters.fontColor = Color::RED;
    swiperDigitalParameters.selectedFontColor = Color::GREEN;
    swiperDigitalParameters.fontSize = Dimension(12.f);
    swiperDigitalParameters.selectedFontSize = Dimension(16.f);
    swiperDigitalParameters.fontWeight = FontWeight::W500;
    swiperDigitalParameters.selectedFontWeight = FontWeight::W900;
    CreateWithItem([=](SwiperModelNG model) {
        model.SetIndicatorType(SwiperIndicatorType::DIGIT);
        model.SetDigitIndicatorStyle(swiperDigitalParameters);
    });
    EXPECT_EQ(pattern_->GetIndicatorType(), SwiperIndicatorType::DIGIT);
    auto firstTextNode = GetChildFrameNode(indicatorNode_, 0);
    auto lastTextNode = GetChildFrameNode(indicatorNode_, 1);
    auto firstTextLayoutProperty = firstTextNode->GetLayoutProperty<TextLayoutProperty>();
    auto lastTextLayoutProperty = lastTextNode->GetLayoutProperty<TextLayoutProperty>();
    EXPECT_EQ(firstTextLayoutProperty->GetTextColor(), Color::GREEN);
    EXPECT_EQ(firstTextLayoutProperty->GetFontSize(), Dimension(16.f));
    EXPECT_EQ(firstTextLayoutProperty->GetFontWeight(), FontWeight::W900);
    EXPECT_EQ(lastTextLayoutProperty->GetTextColor(), Color::RED);
    EXPECT_EQ(lastTextLayoutProperty->GetFontSize(), Dimension(12.f));
    EXPECT_EQ(lastTextLayoutProperty->GetFontWeight(), FontWeight::W500);
}

/**
 * @tc.name: AttrDuration001
 * @tc.desc: Test property about duration
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrDuration001, TestSize.Level1)
{
    /**
     * @tc.cases: Do not set duration
     * @tc.expected: Duration is default
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_EQ(pattern_->GetDuration(), DEFAULT_DURATION);
}

/**
 * @tc.name: AttrDuration002
 * @tc.desc: Test property about duration
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrDuration002, TestSize.Level1)
{
    /**
     * @tc.cases: Set duration to 500
     * @tc.expected: Duration is 500
     */
    CreateWithItem([](SwiperModelNG model) { model.SetDuration(500); });
    EXPECT_EQ(pattern_->GetDuration(), 500);
}

/**
 * @tc.name: AttrVertical001
 * @tc.desc: Test property about vertical
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrVertical001, TestSize.Level1)
{
    /**
     * @tc.cases: Do not set direction
     * @tc.expected: Axis is HORIZONTAL
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_EQ(pattern_->GetDirection(), Axis::HORIZONTAL);
}

/**
 * @tc.name: AttrVertical002
 * @tc.desc: Test property about vertical
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrVertical002, TestSize.Level1)
{
    /**
     * @tc.cases: Set direction to VERTICAL
     * @tc.expected: Axis is VERTICAL
     */
    CreateWithItem([](SwiperModelNG model) { model.SetDirection(Axis::VERTICAL); });
    EXPECT_EQ(pattern_->GetDirection(), Axis::VERTICAL);
}

/**
 * @tc.name: AttrItemSpace001
 * @tc.desc: Test property about itemSpace
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrItemSpace001, TestSize.Level1)
{
    /**
     * @tc.cases: Do not set itemSpace
     * @tc.expected: ItemSpace default is 0
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_EQ(pattern_->GetItemSpace(), 0.f);
}

/**
 * @tc.name: AttrItemSpace002
 * @tc.desc: Test property about itemSpace
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrItemSpace002, TestSize.Level1)
{
    /**
     * @tc.cases: Set itemSpace to 10
     * @tc.expected: ItemSpace is 10
     */
    CreateWithItem([](SwiperModelNG model) { model.SetItemSpace(Dimension(10.f)); });
    EXPECT_EQ(pattern_->GetItemSpace(), 10.f);
}

/**
 * @tc.name: AttrDisplayMode001
 * @tc.desc: Test property about displayMode
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrDisplayMode001, TestSize.Level1)
{
    /**
     * @tc.cases: Do not set displayMode
     * @tc.expected: IsStretch is true
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_TRUE(SwiperUtils::IsStretch(layoutProperty_));
}

/**
 * @tc.name: AttrDisplayMode002
 * @tc.desc: Test property about displayMode
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrDisplayMode002, TestSize.Level1)
{
    /**
     * @tc.cases: Set displayMode to AUTO_LINEAR
     * @tc.expected: IsStretch is false
     */
    CreateWithItem([](SwiperModelNG model) { model.SetDisplayMode(SwiperDisplayMode::AUTO_LINEAR); });
    EXPECT_FALSE(SwiperUtils::IsStretch(layoutProperty_));
}

/**
 * @tc.name: AttrCachedCount001
 * @tc.desc: Test property about cachedCount
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrCachedCount001, TestSize.Level1)
{
    /**
     * @tc.cases: Do not set cachedCount
     * @tc.expected: CachedCount is 1
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_EQ(layoutProperty_->GetCachedCountValue(1), 1);
}

/**
 * @tc.name: AttrCachedCount002
 * @tc.desc: Test property about cachedCount
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrCachedCount002, TestSize.Level1)
{
    /**
     * @tc.cases: Set cachedCount to 2
     * @tc.expected: CachedCount is 2
     */
    CreateWithItem([](SwiperModelNG model) { model.SetCachedCount(2); });
    EXPECT_EQ(layoutProperty_->GetCachedCountValue(1), 2);
}

/**
 * @tc.name: AttrDisableSwipe001
 * @tc.desc: Test property about disableSwipe
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrDisableSwipe001, TestSize.Level1)
{
    /**
     * @tc.cases: Do not set disableSwipe
     * @tc.expected: DisableSwipe is false
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_FALSE(pattern_->IsDisableSwipe());
}

/**
 * @tc.name: AttrDisableSwipe002
 * @tc.desc: Test property about disableSwipe
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrDisableSwipe002, TestSize.Level1)
{
    /**
     * @tc.cases: Set disableSwipe to true
     * @tc.expected: DisableSwipe is true
     */
    CreateWithItem([](SwiperModelNG model) { model.SetDisableSwipe(true); });
    EXPECT_TRUE(pattern_->IsDisableSwipe());
}

/**
 * @tc.name: AttrCurve001
 * @tc.desc: Test property about Curve
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrCurve001, TestSize.Level1)
{
    /**
     * @tc.cases: Do not set curve
     * @tc.expected: Curve is null
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_EQ(pattern_->GetCurve(), nullptr);
}

/**
 * @tc.name: AttrCurve002
 * @tc.desc: Test property about Curve
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrCurve002, TestSize.Level1)
{
    /**
     * @tc.cases: Set curve to SMOOTH
     * @tc.expected: Curve is SMOOTH
     */
    CreateWithItem([](SwiperModelNG model) { model.SetCurve(Curves::SMOOTH); });
    EXPECT_EQ(pattern_->GetCurve(), Curves::SMOOTH);
}

/**
 * @tc.name: AttrDisplayCount001
 * @tc.desc: Test property about DisplayCount
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrDisplayCount001, TestSize.Level1)
{
    /**
     * @tc.cases: Do not set displayCount
     * @tc.expected: DisplayCount is 1
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_EQ(pattern_->GetDisplayCount(), 1);
    EXPECT_GT(GetChildWidth(frameNode_, 0), 0.f); // item size > 0
    EXPECT_EQ(GetChildWidth(frameNode_, 1), 0.f);
}

/**
 * @tc.name: AttrDisplayCount002
 * @tc.desc: Test property about DisplayCount
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrDisplayCount002, TestSize.Level1)
{
    /**
     * @tc.cases: Set displayCount to 2
     * @tc.expected: DisplayCount is 2
     */
    CreateWithItem([](SwiperModelNG model) { model.SetDisplayCount(2); });
    EXPECT_EQ(pattern_->GetDisplayCount(), 2);
    EXPECT_GT(GetChildWidth(frameNode_, 0), 0.f); // item size > 0
    EXPECT_GT(GetChildWidth(frameNode_, 1), 0.f);
}

/**
 * @tc.name: AttrDisplayCount003
 * @tc.desc: Test property about DisplayCount
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrDisplayCount003, TestSize.Level1)
{
    /**
     * @tc.cases: Set displayCount to ITEM_NUMBER+1
     * @tc.expected: DisplayCount is ITEM_NUMBER+1, last item place has placeholder child
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetDisplayCount(ITEM_NUMBER + 1);
        model.SetIndicatorType(SwiperIndicatorType::DOT);
    });
    EXPECT_EQ(pattern_->GetDisplayCount(), 5);
    EXPECT_EQ(pattern_->TotalCount(), ITEM_NUMBER); // child number still is 4
    EXPECT_GT(GetChildWidth(frameNode_, 3), 0.f); // item size > 0
    EXPECT_GT(GetChildWidth(frameNode_, 4), 0.f); // placeholder child
}

/**
 * @tc.name: AttrDisplayCount004
 * @tc.desc: Test property about DisplayCount
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrDisplayCount004, TestSize.Level1)
{
    /**
     * @tc.cases: Set minsize to half of swiper width
     * @tc.expected: show 2 item in one page
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetMinSize(Dimension(SWIPER_WIDTH / 3));
        model.SetIndicatorType(SwiperIndicatorType::DOT);
    });
    EXPECT_TRUE(pattern_->IsAutoFill());
    EXPECT_EQ(pattern_->GetDisplayCount(), 2);
    EXPECT_GT(GetChildWidth(frameNode_, 0), 0.f); // item size > 0
    EXPECT_GT(GetChildWidth(frameNode_, 1), 0.f); // item size > 0
    EXPECT_EQ(GetChildWidth(frameNode_, 2), 0.f);
}

/**
 * @tc.name: AttrDisplayCount005
 * @tc.desc: Test property about DisplayCount
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrDisplayCount005, TestSize.Level1)
{
    /**
     * @tc.cases: Set displayCount to invalid 0
     * @tc.expected: DisplayCount is default 1
     */
    CreateWithItem([](SwiperModelNG model) { model.SetDisplayCount(0); });
    EXPECT_EQ(pattern_->GetDisplayCount(), 1);
}

/**
 * @tc.name: AttrDisplayCount006
 * @tc.desc: Test property about DisplayCount
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrDisplayCount006, TestSize.Level1)
{
    /**
     * @tc.cases: Do not set displayCount
     * @tc.expected: DisplayCount is 1
     */
    SwiperModelNG model;
    model.Create();
    ViewAbstract::SetWidth(CalcLength(SWIPER_WIDTH));
    ViewAbstract::SetHeight(CalcLength(SWIPER_HEIGHT));
    CreateItem();
    GetInstance();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetDisplayCount(), 1);

    /**
     * @tc.cases: Set displayCount to invalid -1
     * @tc.expected: DisplayCount is default 1
     */
    model.SetDisplayCount(AceType::RawPtr(frameNode_), -1);
    EXPECT_EQ(pattern_->GetDisplayCount(), 1);

    /**
     * @tc.cases: Set displayCount to 2
     * @tc.expected: DisplayCount is 2
     */
    model.SetDisplayCount(AceType::RawPtr(frameNode_), 2);
    EXPECT_EQ(pattern_->GetDisplayCount(), 2);
}

/**
 * @tc.name: AttrEdgeEffect001
 * @tc.desc: Test property about EdgeEffect
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrEdgeEffect001, TestSize.Level1)
{
    /**
     * @tc.cases: Do not set edgeEffect
     * @tc.expected: EdgeEffect is SPRING
     */
    CreateWithItem([](SwiperModelNG model) { model.SetLoop(false); });
    EXPECT_EQ(pattern_->GetEdgeEffect(), EdgeEffect::SPRING);
}

/**
 * @tc.name: AttrEdgeEffect002
 * @tc.desc: Test property about EdgeEffect
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrEdgeEffect002, TestSize.Level1)
{
    /**
     * @tc.cases: Set edgeEffect to FADE
     * @tc.expected: EdgeEffect is FADE
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetLoop(false);
        model.SetEdgeEffect(EdgeEffect::FADE);
    });
    EXPECT_EQ(pattern_->GetEdgeEffect(), EdgeEffect::FADE);
}

/**
 * @tc.name: AttrEdgeEffect003
 * @tc.desc: Test property about EdgeEffect
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrEdgeEffect003, TestSize.Level1)
{
    /**
     * @tc.cases: Set edgeEffect to NONE
     * @tc.expected: EdgeEffect is NONE
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetLoop(false);
        model.SetEdgeEffect(EdgeEffect::NONE);
    });
    EXPECT_EQ(pattern_->GetEdgeEffect(), EdgeEffect::NONE);
}

/**
 * @tc.name: AttrMargin001
 * @tc.desc: Test property about Margin
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrMargin001, TestSize.Level1)
{
    /**
     * @tc.cases: Do not set margin
     * @tc.expected: Margin is 0
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_EQ(pattern_->GetNextMargin(), 0.f);
    EXPECT_EQ(pattern_->GetPrevMargin(), 0.f);
}

/**
 * @tc.name: AttrMargin002
 * @tc.desc: Test property about Margin
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrMargin002, TestSize.Level1)
{
    /**
     * @tc.cases: Set margin to 10,5
     * @tc.expected: Margin is 10,5
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetNextMargin(Dimension(10.f), false);
        model.SetPreviousMargin(Dimension(5.f), false);
    });
    EXPECT_EQ(pattern_->GetNextMargin(), 10.f);
    EXPECT_EQ(pattern_->GetPrevMargin(), 5.f);
}

/**
 * @tc.name: AttrMargin003
 * @tc.desc: Test property about Margin
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrMargin003, TestSize.Level1)
{
    /**
     * @tc.cases: Set margin to invalid SWIPER_WIDTH+1 (>swiperSize)
     * @tc.expected: Margin is 0
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetNextMargin(Dimension(SWIPER_WIDTH + 1.f), false);
        model.SetPreviousMargin(Dimension(5), false);
    });
    EXPECT_EQ(pattern_->GetNextMargin(), 0.f);
    EXPECT_EQ(pattern_->GetPrevMargin(), 0.f);
}

/**
 * @tc.name: AttrMargin004
 * @tc.desc: Test property about Margin
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrMargin004, TestSize.Level1)
{
    /**
     * @tc.cases: Set margin to invalid SWIPER_WIDTH+1 (>swiperSize)
     * @tc.expected: Margin is 0
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetNextMargin(Dimension(10.f), false);
        model.SetPreviousMargin(Dimension(SWIPER_WIDTH + 1.f), false);
    });
    EXPECT_EQ(pattern_->GetNextMargin(), 0.f);
    EXPECT_EQ(pattern_->GetPrevMargin(), 0.f);
}

/**
 * @tc.name: AttrMargin005
 * @tc.desc: Test property about Margin
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrMargin005, TestSize.Level1)
{
    /**
     * @tc.cases: Only set nextMargin
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetNextMargin(Dimension(10.f), false);
    });
    EXPECT_EQ(pattern_->GetNextMargin(), 10.f);
    EXPECT_EQ(pattern_->GetPrevMargin(), 0.f);
}

/**
 * @tc.name: AttrMargin006
 * @tc.desc: Test property about Margin
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrMargin006, TestSize.Level1)
{
    /**
     * @tc.cases: Only set preMargin
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetPreviousMargin(Dimension(5.f), false);
    });
    EXPECT_EQ(pattern_->GetNextMargin(), 0.f);
    EXPECT_EQ(pattern_->GetPrevMargin(), 5.f);
}
/**
 * @tc.name: AttrNestedScroll001
 * @tc.desc: Test property about NestedScroll
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrNestedScroll001, TestSize.Level1)
{
    /**
     * @tc.cases: NestedScroll is default
     * @tc.expected: enableNestedScroll_ is false
     */
    CreateWithItem([](SwiperModelNG model) { model.SetLoop(false); });
    EXPECT_FALSE(pattern_->IsLoop());
    auto nestedScroll = pattern_->GetNestedScroll();
    EXPECT_EQ(nestedScroll.forward, NestedScrollMode::SELF_ONLY);
    EXPECT_EQ(nestedScroll.backward, NestedScrollMode::SELF_ONLY);
}

/**
 * @tc.name: AttrNestedScroll002
 * @tc.desc: Test property about NestedScroll
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrNestedScroll002, TestSize.Level1)
{
    /**
     * @tc.cases: NestedScroll is SELF_FIRST
     * @tc.expected: enableNestedScroll_ is true
     */
    NestedScrollOptions nestedOpt = {
        .forward = NestedScrollMode::SELF_FIRST,
        .backward = NestedScrollMode::SELF_FIRST,
    };
    CreateWithItem([nestedOpt](SwiperModelNG model) {
        model.SetLoop(false);
        model.SetNestedScroll(nestedOpt);
    });
    EXPECT_FALSE(pattern_->IsLoop());
    auto nestedScroll = pattern_->GetNestedScroll();
    EXPECT_EQ(nestedScroll.forward, NestedScrollMode::SELF_FIRST);
    EXPECT_EQ(nestedScroll.backward, NestedScrollMode::SELF_FIRST);
}

/**
 * @tc.name: AttrDisplayArrow001
 * @tc.desc: Test property about DisplayArrow
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, AttrDisplayArrow001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create swiper.
     */
    SwiperModelNG model;
    model.Create();
    ViewAbstract::SetWidth(CalcLength(SWIPER_WIDTH));
    ViewAbstract::SetHeight(CalcLength(SWIPER_HEIGHT));
    CreateItem();
    GetInstance();
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step2. set invalid SwiperArrowParameters.
     * @tc.expected: check whether the properties is correct.
     */
    SwiperArrowParameters swiperArrowParameters;
    model.SetArrowStyle(AceType::RawPtr(frameNode_), swiperArrowParameters);
    EXPECT_FALSE(layoutProperty_->HasIsShowBackground());
    EXPECT_FALSE(layoutProperty_->HasBackgroundSize());
    EXPECT_FALSE(layoutProperty_->HasBackgroundColor());
    EXPECT_FALSE(layoutProperty_->HasArrowSize());
    EXPECT_FALSE(layoutProperty_->HasArrowColor());
    EXPECT_FALSE(layoutProperty_->HasIsSidebarMiddle());

    /**
     * @tc.steps: step3. set valid SwiperArrowParameters.
     * @tc.expected: check whether the properties is correct.
     */
    swiperArrowParameters.isShowBackground = false;
    swiperArrowParameters.isSidebarMiddle = false;
    swiperArrowParameters.backgroundSize = Dimension(24.0);
    swiperArrowParameters.backgroundColor = Color::BLUE;
    swiperArrowParameters.arrowSize = Dimension(24.0);
    swiperArrowParameters.arrowColor = Color::BLUE;
    model.SetArrowStyle(AceType::RawPtr(frameNode_), swiperArrowParameters);
    EXPECT_TRUE(layoutProperty_->GetIsShowBackground());
    EXPECT_TRUE(layoutProperty_->GetIsSidebarMiddle());
    EXPECT_EQ(layoutProperty_->GetBackgroundSize(), Dimension(24.0));
    EXPECT_EQ(layoutProperty_->GetBackgroundColor(), Color::BLUE);
    EXPECT_EQ(layoutProperty_->GetArrowSize(), Dimension(24.0));
    EXPECT_EQ(layoutProperty_->GetArrowColor(), Color::BLUE);
}

/**
 * @tc.name: SwiperModelNg001
 * @tc.desc: Swiper Model NG.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, SwiperModelNg001, TestSize.Level1)
{
    SwiperModelNG model;
    model.Create();
    ViewAbstract::SetWidth(CalcLength(SWIPER_WIDTH));
    ViewAbstract::SetHeight(CalcLength(SWIPER_HEIGHT));
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto pattern = frameNode->GetPattern<SwiperPattern>();
    auto layoutProperty = frameNode->GetLayoutProperty<SwiperLayoutProperty>();
    auto paintProperty = frameNode->GetPaintProperty<SwiperPaintProperty>();

    /**
     * @tc.steps: step3.1. Test SetIndex function.
     * @tc.expected: layoutProperty->GetIndex() is equal to 1.
     */
    model.SetIndex(1);
    EXPECT_EQ(layoutProperty->GetIndex(), 1);

    /**
     * @tc.steps: step3.2. Test SetDisplayMode function.
     * @tc.expected: layoutProperty->GetDisplayMode() is equal to swiperDisplayMode.
     */
    model.SetDisplayMode(SwiperDisplayMode::STRETCH);
    EXPECT_EQ(layoutProperty->GetDisplayMode(), SwiperDisplayMode::STRETCH);

    /**
     * @tc.steps: step3.3. Test SetShowIndicator function.
     * @tc.expected: layoutProperty->GetIndex() is equal to 1.
     */
    model.SetShowIndicator(true);
    EXPECT_TRUE(layoutProperty->GetShowIndicator());

    /**
     * @tc.steps: step3.4. Test SetItemSpace function.
     * @tc.expected: layoutProperty->GetItemSpace() is equal to dimension.
     */
    auto dimension = Dimension(-1.0);
    model.SetItemSpace(dimension);
    EXPECT_EQ(layoutProperty->GetItemSpace(), dimension);

    /**
     * @tc.steps: step3.5. Test SetCachedCount function.
     * @tc.expected:DisplayCount = -1 layoutProperty->SetCachedCount() is equal to 1.
     * @tc.expected:DisplayCount = 1 layoutProperty->SetCachedCount() is equal to 1.
     */
    model.SetCachedCount(-1);
    model.SetCachedCount(1);
    EXPECT_EQ(layoutProperty->GetCachedCount(), 1);

    /**
     * @tc.steps: step3.6. Test SetIsIndicatorCustomSize function.
     * @tc.expected: pattern->IsIndicatorCustomSize() is equal to true.
     */
    model.SetIsIndicatorCustomSize(true);
    EXPECT_TRUE(pattern->IsIndicatorCustomSize());

    /**
     * @tc.steps: step3.7. Test SetAutoPlay function.
     * @tc.expected: SwiperPaintProperty->GetAutoPlay() is equal to true.
     */
    model.SetAutoPlay(true);
    EXPECT_TRUE(paintProperty->GetAutoPlay());

    /**
     * @tc.steps: step3.8. Test SetAutoPlayInterval function.
     * @tc.expected: SwiperPaintProperty->GetAutoPlayInterval() is equal to 1.
     */
    model.SetAutoPlayInterval(1);
    EXPECT_EQ(paintProperty->GetAutoPlayInterval(), 1);

    /**
     * @tc.steps: step3.9. Test SetDuration function.
     * @tc.expected: SwiperPaintProperty->GetDuration() is equal to 1.
     */
    model.SetDuration(1);
    EXPECT_EQ(paintProperty->GetDuration(), 1);
}

/**
 * @tc.name: SwiperModelNg002
 * @tc.desc: Swiper Model NG.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, SwiperModelNg002, TestSize.Level1)
{
    SwiperModelNG model;
    model.Create();
    ViewAbstract::SetWidth(CalcLength(SWIPER_WIDTH));
    ViewAbstract::SetHeight(CalcLength(SWIPER_HEIGHT));
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto pattern = frameNode->GetPattern<SwiperPattern>();
    auto layoutProperty = frameNode->GetLayoutProperty<SwiperLayoutProperty>();
    auto paintProperty = frameNode->GetPaintProperty<SwiperPaintProperty>();
    auto eventHub = frameNode->GetEventHub<SwiperEventHub>();

    /**
     * @tc.steps: step3.1. Test SetLoop function.
     * @tc.expected: SwiperPaintProperty->GetLoop() is true.
     */
    model.SetLoop(true);
    EXPECT_TRUE(layoutProperty->GetLoop());

    /**
     * @tc.steps: step3.2. Test SetEnabled function.
     * @tc.expected: SwiperPaintProperty->GetEnabled() is true.
     */
    model.SetEnabled(true);
    EXPECT_TRUE(paintProperty->GetEnabled());

    /**
     * @tc.steps: step3.3. Test SetDisableSwipe function.
     * @tc.expected: layoutProperty->GetDisableSwipe() is true.
     */
    model.SetDisableSwipe(true);
    EXPECT_TRUE(layoutProperty->GetDisableSwipe());

    /**
     * @tc.steps: step3.4. Test SetEdgeEffect function.
     * @tc.expected: SwiperPaintProperty->GetEdgeEffect() is true.
     */
    model.SetEdgeEffect(EdgeEffect::FADE);
    EXPECT_EQ(paintProperty->GetEdgeEffect(), EdgeEffect::FADE);

    /**
     * @tc.steps: step3.5. Test SetOnChange function.
     * @tc.expected:pattern->changeEvent_ not null.
     */
    auto onChange = [](const BaseEventInfo* info) {};
    model.SetOnChange(std::move(onChange));
    EXPECT_NE(pattern->changeEvent_, nullptr);

    /**
     * @tc.steps: step3.6. Test SetOnAnimationStart function.
     * @tc.expected:pattern->animationStartEvent_ not null.
     */
    auto onAnimationStart = [](int32_t index, int32_t targetIndex, const AnimationCallbackInfo& info) {};
    model.SetOnAnimationStart(std::move(onAnimationStart));
    EXPECT_NE(pattern->animationStartEvent_, nullptr);

    /**
     * @tc.steps: step3.7. Test SetOnAnimationEnd function.
     * @tc.expected:pattern->animationEndEvent_ not null.
     */
    auto onAnimationEnd = [](int32_t index, const AnimationCallbackInfo& info) {};
    model.SetOnAnimationEnd(std::move(onAnimationEnd));
    EXPECT_NE(pattern->animationEndEvent_, nullptr);
}

/**
 * @tc.name: SwiperModelNg003
 * @tc.desc: Swiper Model NG.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, SwiperModelNg003, TestSize.Level1)
{
    SwiperModelNG model;
    model.Create();
    ViewAbstract::SetWidth(CalcLength(SWIPER_WIDTH));
    ViewAbstract::SetHeight(CalcLength(SWIPER_HEIGHT));
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto pattern = frameNode->GetPattern<SwiperPattern>();
    auto layoutProperty = frameNode->GetLayoutProperty<SwiperLayoutProperty>();
    auto paintProperty = frameNode->GetPaintProperty<SwiperPaintProperty>();

    /**
     * @tc.steps: step3.1. Test SetIndicatorStyle function.
     * @tc.expected: SwiperPaintProperty->swiperParameters_->colorVal is swiperParameters.colorVal.
     */
    SwiperParameters swiperParameters;
    swiperParameters.colorVal = Color(Color::BLUE);
    model.SetIndicatorStyle(swiperParameters);
    EXPECT_EQ(pattern->swiperParameters_->colorVal, swiperParameters.colorVal);

    /**
     * @tc.steps: step3.2. Test SetPreviousMargin function.
     * @tc.expected: paintProperty->GetPrevMargin() is equal to dimension.
     */
    auto dimension = Dimension(-1.0);
    model.SetPreviousMargin(dimension, false);
    EXPECT_EQ(layoutProperty->GetPrevMargin(), dimension);

    /**
     * @tc.steps: step3.3. Test SetNextMargin function.
     * @tc.expected: paintProperty->GetNextMargin() is equal to dimension.
     */
    model.SetNextMargin(dimension, false);
    EXPECT_EQ(layoutProperty->GetNextMargin(), dimension);

    /**
     * @tc.steps: step3.5. Test SetIndicatorIsBoolean function.
     * @tc.expected: pattern->indicatorIsBoolean_ is true.
     */
    model.SetIndicatorIsBoolean(true);
    EXPECT_TRUE(pattern->indicatorIsBoolean_);

    /**
     * @tc.steps: step3.6. Test SetArrowStyle function.
     * @tc.expected: before set swiperArrowParameters, all result is null.
     */
    SwiperArrowParameters swiperArrowParameters;
    model.SetArrowStyle(swiperArrowParameters);

    /**
     * @tc.steps: step3.7. Test SetArrowStyle function.
     * @tc.expected: after set swiperArrowParameters, layoutProperty->IsShowBoard is true.
     */
    swiperArrowParameters.isShowBackground = true;
    swiperArrowParameters.backgroundSize = dimension;
    swiperArrowParameters.backgroundColor = Color(Color::BLUE);
    swiperArrowParameters.arrowSize = dimension;
    swiperArrowParameters.arrowColor = Color(Color::RED);
    swiperArrowParameters.isSidebarMiddle = true;
    model.SetArrowStyle(swiperArrowParameters);
    EXPECT_TRUE(layoutProperty->GetIsShowBackground());
    EXPECT_EQ(layoutProperty->GetBackgroundSize(), dimension);
    EXPECT_EQ(layoutProperty->GetBackgroundColor(), Color(Color::BLUE));
    EXPECT_EQ(layoutProperty->GetArrowSize(), dimension);
    EXPECT_EQ(layoutProperty->GetArrowColor(), Color(Color::RED));
    EXPECT_TRUE(layoutProperty->GetIsSidebarMiddle());

    /**
     * @tc.steps: step3.8. Test SetDisplayArrow function.
     * @tc.expected: layoutProperty->GetDisplayArrow() is true.
     */
    model.SetDisplayArrow(true);
    EXPECT_TRUE(layoutProperty->GetDisplayArrow());

    /**
     * @tc.steps: step3.9. Test SetHoverShow function.
     * @tc.expected: layoutProperty->GetHoverShow() is true.
     */
    model.SetHoverShow(true);
    EXPECT_TRUE(layoutProperty->GetHoverShow());
}

/**
 * @tc.name: SetMinSize001
 * @tc.desc: Swiper Model NG.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, SetMinSize003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Default value
     */
    SwiperModelNG model;
    model.Create();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto pattern = frameNode->GetPattern<SwiperPattern>();
    auto layoutProperty = frameNode->GetLayoutProperty<SwiperLayoutProperty>();
    auto paintProperty = frameNode->GetPaintProperty<SwiperPaintProperty>();

    /**
     * @tc.steps: step2. Calling the SetMinSize interface to set Dimension
     * @tc.expected: LayoutProperty ->GetMinSize(), equal to Dimension()
     */
    model.SetMinSize(Dimension(10));
    layoutProperty->GetMinSize();
    EXPECT_EQ(layoutProperty->GetMinSize(), Dimension(10));
}

/**
 * @tc.name: SwiperPaintProperty001
 * @tc.desc: Swiper Paint Property.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, SwiperPaintProperty001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});

    /**
     * @tc.steps: step1. Test ToJsonValue function.
     * @tc.expected: Check the swiper property value
     */
    auto json = JsonUtil::Create(true);
    paintProperty_->ToJsonValue(json, filter);
    EXPECT_EQ(json->GetString("autoPlay"), "false");

    /**
     * @tc.steps: step2. call UpdateCalcLayoutProperty, push constraint is null.
     * @tc.expected: Return expected results.
     */
    MeasureProperty constraint;
    layoutProperty_->UpdateCalcLayoutProperty(std::move(constraint));
    EXPECT_EQ(layoutProperty_->propertyChangeFlag_, 1);

    /**
     * @tc.steps: step3. Test FromJson function.
     * @tc.expected: Check the swiper property value
     */
    auto jsonFrom = JsonUtil::Create(true);
    paintProperty_->FromJson(jsonFrom);
    EXPECT_TRUE(jsonFrom);
}

/**
 * @tc.name: ArcDotIndicator001
 * @tc.desc: Test property about indicator
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, ArcDotIndicator001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create swiper and set parameters.
     */
    SwiperArcDotParameters swiperArcDotParameters;
    swiperArcDotParameters.arcDirection = SwiperArcDirection::NINE_CLOCK_DIRECTION;
    swiperArcDotParameters.itemColor = Color::GREEN;
    swiperArcDotParameters.selectedItemColor = Color::RED;
    swiperArcDotParameters.containerColor = Color::BLUE;
    CreateWithItem([=](SwiperModelNG model) {
        model.Create(true);
        model.SetIndicatorType(SwiperIndicatorType::ARC_DOT);
        model.SetArcDotIndicatorStyle(swiperArcDotParameters);
    });
    auto paintProperty = indicatorNode_->GetPaintProperty<CircleDotIndicatorPaintProperty>();
    RefPtr<SwiperPattern> indicatorPattern = frameNode_->GetPattern<SwiperPattern>();
    indicatorPattern->OnModifyDone();
    EXPECT_EQ(pattern_->GetIndicatorType(), SwiperIndicatorType::ARC_DOT);
    EXPECT_EQ(paintProperty->GetArcDirection(), SwiperArcDirection::NINE_CLOCK_DIRECTION);
    EXPECT_EQ(paintProperty->GetColor(), Color::GREEN);
    EXPECT_EQ(paintProperty->GetSelectedColor(), Color::RED);
    EXPECT_EQ(paintProperty->GetContainerColor(), Color::BLUE);
}

/**
 * @tc.name: ArcDotIndicator001
 * @tc.desc: Test property about indicator
 * @tc.type: FUNC
 */
HWTEST_F(SwiperAttrTestNg, ArcDotIndicator002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create swiper and set parameters.
     */
    CreateWithItem([=](SwiperModelNG model) {
        model.Create(true);
        model.SetIndicatorType(SwiperIndicatorType::ARC_DOT);
    });
    RefPtr<ArcSwiperPattern> indicatorPattern = frameNode_->GetPattern<ArcSwiperPattern>();
    indicatorPattern->GetSwiperArcDotParameters();
    EXPECT_NE(indicatorPattern->swiperArcDotParameters_, nullptr);
}
} // namespace OHOS::Ace::NG
