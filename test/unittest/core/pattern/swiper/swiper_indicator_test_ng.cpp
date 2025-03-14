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
// padding:12 space:8 size:8
Offset FIRST_POINT = Offset(16.f, 16.f);
Offset SECOND_POINT = Offset(40.f, 16.f);
Offset FOURTH_POINT = Offset(72.f, 16.f);
} // namespace

class SwiperIndicatorTestNg : public SwiperTestNg {
public:
    void MouseClickIndicator(SourceType sourceType, Offset hoverPoint);
    void TouchClickIndicator(SourceType sourceType, Offset touchPoint);
    void LongPressIndicator(Offset startPoint, Offset endPoint);
};

void SwiperIndicatorTestNg::MouseClickIndicator(SourceType sourceType, Offset hoverPoint)
{
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    HoverInfo hoverInfo;
    hoverInfo.SetSourceDevice(sourceType);
    indicatorPattern->hoverEvent_->GetOnHoverFunc()(true, hoverInfo);

    MouseInfo mouseInfo;
    mouseInfo.SetSourceDevice(sourceType);
    mouseInfo.SetAction(MouseAction::PRESS);
    mouseInfo.SetLocalLocation(hoverPoint);
    indicatorPattern->mouseEvent_->GetOnMouseEventFunc()(mouseInfo);

    GestureEvent gestureEvent;
    gestureEvent.SetSourceDevice(sourceType);
    indicatorPattern->HandleClick(gestureEvent);
    FlushLayoutTask(frameNode_);
}

void SwiperIndicatorTestNg::TouchClickIndicator(SourceType sourceType, Offset touchPoint)
{
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    indicatorPattern->HandleTouchEvent(CreateTouchEventInfo(TouchType::DOWN, touchPoint));
    indicatorPattern->HandleTouchEvent(CreateTouchEventInfo(TouchType::UP, touchPoint));

    GestureEvent gestureEvent;
    gestureEvent.SetSourceDevice(sourceType);
    gestureEvent.SetLocalLocation(touchPoint);
    indicatorPattern->HandleClick(gestureEvent);
    FlushLayoutTask(frameNode_);
}

void SwiperIndicatorTestNg::LongPressIndicator(Offset startPoint, Offset endPoint)
{
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    indicatorPattern->HandleTouchEvent(CreateTouchEventInfo(TouchType::DOWN, startPoint));
    GestureEvent gestureEvent;
    gestureEvent.SetLocalLocation(startPoint);
    indicatorPattern->HandleLongPress(gestureEvent);

    indicatorPattern->HandleTouchEvent(CreateTouchEventInfo(TouchType::MOVE, endPoint));
    indicatorPattern->HandleTouchEvent(CreateTouchEventInfo(TouchType::UP, endPoint));
    FlushLayoutTask(frameNode_);
}

/**
 * @tc.name: OnIndicatorChangeEvent001
 * @tc.desc: Test IndicatorChangeEvent, only effected with DIGIT
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, OnIndicatorChangeEvent001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetIndicatorType(SwiperIndicatorType::DIGIT);
    });
    auto firstTextNode = AceType::DynamicCast<FrameNode>(indicatorNode_->GetFirstChild());
    auto lastTextNode = AceType::DynamicCast<FrameNode>(indicatorNode_->GetLastChild());
    auto firstTextLayoutProperty = firstTextNode->GetLayoutProperty<TextLayoutProperty>();
    auto lastTextLayoutProperty = lastTextNode->GetLayoutProperty<TextLayoutProperty>();

    /**
     * @tc.steps: step1. Default
     * @tc.expected: text is "1/4"
     */
    EXPECT_EQ(firstTextLayoutProperty->GetContentValue(), "1");
    EXPECT_EQ(lastTextLayoutProperty->GetContentValue(), "/4");

    /**
     * @tc.steps: step2. Call ShowNext
     * @tc.expected: Change firstText
     */
    ShowNext();
    EXPECT_EQ(firstTextLayoutProperty->GetContentValue(), "2");

    /**
     * @tc.steps: step3. Call ShowPrevious
     * @tc.expected: Change firstText
     */
    ShowPrevious();
    EXPECT_EQ(firstTextLayoutProperty->GetContentValue(), "1");

    /**
     * @tc.steps: step4. Call ChangeIndex
     * @tc.expected: Change firstText
     */
    ChangeIndex(3);
    EXPECT_EQ(firstTextLayoutProperty->GetContentValue(), "4");
}

/**
 * @tc.name: HandleMouseClick001
 * @tc.desc: Test SwiperIndicator HandleMouseClick
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, HandleMouseClick001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetIndicatorType(SwiperIndicatorType::DOT);
    });

    /**
     * @tc.steps: step1. Click item(index:1)
     * @tc.expected: Swipe to item(index:1)
     */
    MouseClickIndicator(SourceType::MOUSE, SECOND_POINT);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 1);

    /**
     * @tc.steps: step2. Click item(index:2)
     * @tc.expected: Still is item(index:1)
     */
    MouseClickIndicator(SourceType::MOUSE, SECOND_POINT);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 1);

    /**
     * @tc.steps: step3. Click item(index:3)
     * @tc.expected: Swipe to item(index:3)
     */
    MouseClickIndicator(SourceType::MOUSE, FOURTH_POINT);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 3);

    /**
     * @tc.steps: step4. Click item(index:0)
     * @tc.expected: Swipe to item(index:0)
     */
    MouseClickIndicator(SourceType::MOUSE, FIRST_POINT);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 0);
}

/**
 * @tc.name: HandleTouchClick001
 * @tc.desc: Test SwiperIndicator HandleTouchClick
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, HandleTouchClick001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetIndicatorType(SwiperIndicatorType::DOT);
    });

    /**
     * @tc.steps: step1. Click item(index:1)
     * @tc.expected: Swipe to item(index:1)
     */
    TouchClickIndicator(SourceType::TOUCH, SECOND_POINT);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 1);

    /**
     * @tc.steps: step2. Click item(index:3)
     * @tc.expected: Swipe to item(index:2)
     */
    TouchClickIndicator(SourceType::TOUCH, FOURTH_POINT);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 2);

    /**
     * @tc.steps: step3. Click item(index:0)
     * @tc.expected: Swipe to item(index:1)
     */
    TouchClickIndicator(SourceType::TOUCH, FIRST_POINT);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 1);
}

/**
 * @tc.name: HandleLongPress001
 * @tc.desc: Test SwiperIndicator HandleLongPress
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, HandleLongPress001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});

    /**
     * @tc.steps: step1. Touch and move right
     * @tc.expected: Swipe to item(index:1)
     */
    LongPressIndicator(FIRST_POINT, FOURTH_POINT);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 1);

    /**
     * @tc.steps: step1. Touch and move left
     * @tc.expected: Swipe to item(index:0)
     */
    LongPressIndicator(FOURTH_POINT, SECOND_POINT);
    EXPECT_EQ(pattern_->GetCurrentIndex(), 0);
}

/**
 * @tc.name: SetDotIndicatorStyle001
 * @tc.desc: Test SwiperModelNG SetDotIndicatorStyle
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SetDotIndicatorStyle001, TestSize.Level1)
{
    SwiperParameters swiperParameters;
    swiperParameters.colorVal = Color(Color::BLUE);
    CreateWithItem([swiperParameters](SwiperModelNG model) { model.SetDotIndicatorStyle(swiperParameters); });
    ASSERT_EQ(pattern_->swiperParameters_->colorVal, swiperParameters.colorVal);
}

/**
 * @tc.name: SetDigitIndicatorStyle001
 * @tc.desc: Test SwiperModelNG SetDigitIndicatorStyle
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SetDigitIndicatorStyle001, TestSize.Level1)
{
    SwiperDigitalParameters digitalParameters;
    digitalParameters.fontColor = Color(Color::GREEN);
    CreateWithItem([digitalParameters](SwiperModelNG model) { model.SetDigitIndicatorStyle(digitalParameters); });
    ASSERT_EQ(pattern_->swiperDigitalParameters_->fontColor, digitalParameters.fontColor);
}

/**
 * @tc.name: SwiperPatternPlayIndicatorTranslateAnimation001
 * @tc.desc: PlayIndicatorTranslateAnimation
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperPatternPlayIndicatorTranslateAnimation001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    pattern_->indicatorId_.reset();
    float translate = 0.1f;

    /**
     * @tc.steps: step2. call PlayIndicatorTranslateAnimation.
     * @tc.expected: Related function runs ok.
     */
    for (int i = 0; i <= 1; i++) {
        for (int j = 0; j <= 1; j++) {
            pattern_->PlayIndicatorTranslateAnimation(translate);
            if (i == 1) {
                break;
            }
            pattern_->indicatorId_ = 1;
        }
    }
}

/**
 * @tc.name: SwiperPatternPlayIndicatorTranslateAnimation002
 * @tc.desc: PlayIndicatorTranslateAnimation
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperPatternPlayIndicatorTranslateAnimation002, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});

    /**
     * @tc.steps: step2. call PlayIndicatorTranslateAnimation.
     * @tc.expected: Related function runs ok.
     */
    TurnPageRateFunc callback = [](const int32_t i, float f) {};
    pattern_->swiperController_->SetTurnPageRateCallback(callback);
    pattern_->PlayIndicatorTranslateAnimation(0.1f);
    EXPECT_NE(pattern_->swiperController_->GetTurnPageRateCallback(), nullptr);
}

/**
 * @tc.name: SwiperInitIndicator006
 * @tc.desc: Test SwiperPattern SwiperInitIndicator
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperInitIndicator006, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    layoutProperty_->UpdateShowIndicator(true);
    layoutProperty_->UpdateIndicatorType(SwiperIndicatorType::DIGIT);
    pattern_->lastSwiperIndicatorType_ = SwiperIndicatorType::DOT;

    /**
     * @tc.steps: step2. call InitIndicator.
     * @tc.expected: frameNode_ lastChild is SWIPER_INDICATOR_ETS_TAG
     */
    pattern_->InitIndicator();
    ASSERT_EQ(frameNode_->GetLastChild()->GetTag(), V2::SWIPER_INDICATOR_ETS_TAG);
}

/**
 * @tc.name: SwiperIndicatorPatternTestNg005
 * @tc.desc: HandleHoverEvent
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperIndicatorPatternTestNg005, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetDirection(Axis::VERTICAL);
    });
    auto indicatorNode = GetChildFrameNode(frameNode_, 4);
    auto indicatorPattern = indicatorNode->GetPattern<SwiperIndicatorPattern>();
    auto eventHub = indicatorNode->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    indicatorPattern->SetIndicatorInteractive(true);
    EXPECT_TRUE(eventHub->IsEnabled());
    indicatorPattern->SetIndicatorInteractive(false);
    EXPECT_FALSE(eventHub->IsEnabled());
}

/**
 * @tc.name: SwiperPatternCheckMarkDirtyNodeForRenderIndicator001
 * @tc.desc: Test CheckMarkDirtyNodeForRenderIndicator
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperPatternCheckMarkDirtyNodeForRenderIndicator001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    RefPtr<SwiperPattern> indicatorPattern = frameNode_->GetPattern<SwiperPattern>();

    /**
     * @tc.steps: step2. test CheckMarkDirtyNodeForRenderIndicator.
     * @tc.expected: Related function runs ok.
     */
    indicatorPattern->indicatorId_.reset();
    float additionalOffset = 0.1f;
    indicatorPattern->CheckMarkDirtyNodeForRenderIndicator(additionalOffset);
    indicatorPattern->indicatorId_ = 1;
    indicatorPattern->CheckMarkDirtyNodeForRenderIndicator(additionalOffset);
}

/**
 * @tc.name: SwiperIndicatorPatternCheckIsTouchBottom001
 * @tc.desc: CheckIsTouchBottom
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperIndicatorPatternCheckIsTouchBottom001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    GestureEvent info;
    TouchLocationInfo touchLocationInfo("down", 0);
    touchLocationInfo.SetTouchType(TouchType::DOWN);
    std::list<TouchLocationInfo> infoSwiper;
    infoSwiper.emplace_back(touchLocationInfo);
    TouchEventInfo touchEventInfo("down");
    touchEventInfo.touches_ = infoSwiper;
    pattern_->currentIndex_ = -5;
    layoutProperty_->UpdateLoop(false);
    pattern_->leftButtonId_.reset();
    pattern_->rightButtonId_.reset();
    pattern_->GetLayoutProperty<SwiperLayoutProperty>()->UpdateShowIndicator(false);
    EXPECT_TRUE(indicatorPattern->CheckIsTouchBottom(info));
    EXPECT_TRUE(indicatorPattern->CheckIsTouchBottom(touchEventInfo.GetTouches().front()));
}

/**
 * @tc.name: SwiperIndicatorPatternCheckIsTouchBottom002
 * @tc.desc: CheckIsTouchBottom
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperIndicatorPatternCheckIsTouchBottom002, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    auto displayCount = pattern_->GetLayoutProperty<SwiperLayoutProperty>()->GetDisplayCount().value_or(1);
    auto childrenSize = pattern_->RealTotalCount();
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    GestureEvent info;
    layoutProperty_->UpdateLoop(false);
    pattern_->leftButtonId_.reset();
    pattern_->rightButtonId_.reset();
    pattern_->GetLayoutProperty<SwiperLayoutProperty>()->UpdateShowIndicator(false);
    /**
     * @tc.steps: step1. call no mirror func.
     */
    pattern_->currentIndex_ = -5;
    layoutProperty_->UpdateLayoutDirection(TextDirection::LTR);
    EXPECT_TRUE(indicatorPattern->CheckIsTouchBottom(info));
    pattern_->currentIndex_ = 5;
    EXPECT_TRUE(pattern_->currentIndex_ >= childrenSize - displayCount);
    EXPECT_TRUE(indicatorPattern->CheckIsTouchBottom(info));
    /**
     * @tc.steps: step2. call mirror func.
     */
    pattern_->currentIndex_ = -5;
    layoutProperty_->UpdateLayoutDirection(TextDirection::RTL);
    EXPECT_TRUE(indicatorPattern->CheckIsTouchBottom(info));
    pattern_->currentIndex_ = 5;
    EXPECT_TRUE(pattern_->currentIndex_ >= childrenSize - displayCount);
    EXPECT_TRUE(indicatorPattern->CheckIsTouchBottom(info));
}

/**
 * @tc.name: SwiperPatternPlayIndicatorTranslateAnimation003
 * @tc.desc: PlayIndicatorTranslateAnimation
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperPatternPlayIndicatorTranslateAnimation003, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    pattern_->stopIndicatorAnimation_ = false;
    pattern_->itemPosition_.clear();
    float translate = 0.1f;

    /**
     * @tc.steps: step2. call PlayIndicatorTranslateAnimation.
     * @tc.expected: Related function runs ok.
     */
    pattern_->PlayIndicatorTranslateAnimation(translate);
    pattern_->stopIndicatorAnimation_ = true;
    pattern_->indicatorId_ = 1;
    pattern_->PlayIndicatorTranslateAnimation(translate);
}

/**
 * @tc.name: SwiperIndicatorPatternTouchBottom001
 * @tc.desc: CheckIsTouchBottom
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperIndicatorPatternTouchBottom001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetDirection(Axis::VERTICAL);
    });
    auto indicatorNode_ = AceType::DynamicCast<FrameNode>(frameNode_->GetChildAtIndex(4));
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();

    GestureEvent info;
    info.mainDelta_ = 1.0f;
    TouchLocationInfo touchLocationInfo("down", 0);
    touchLocationInfo.SetTouchType(TouchType::DOWN);
    EXPECT_FALSE(indicatorPattern->CheckIsTouchBottom(info));
    EXPECT_TRUE(indicatorPattern->CheckIsTouchBottom(touchLocationInfo));

    pattern_->currentIndex_ = 0;
    layoutProperty_->UpdateLoop(false);
    pattern_->leftButtonId_ = 1;
    pattern_->rightButtonId_ = 1;
    pattern_->GetLayoutProperty<SwiperLayoutProperty>()->UpdateShowIndicator(true);
    EXPECT_TRUE(indicatorPattern->CheckIsTouchBottom(info));
    EXPECT_TRUE(indicatorPattern->CheckIsTouchBottom(touchLocationInfo));
}

/**
 * @tc.name: SwiperIndicatorGetMouseClickIndex001
 * @tc.desc: Test GetMouseClickIndex
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperIndicatorGetMouseClickIndex001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetDirection(Axis::VERTICAL);
        model.SetIndicatorType(SwiperIndicatorType::DOT);
    });
    RefPtr<SwiperIndicatorPattern> indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    auto paintProperty = indicatorNode_->GetPaintProperty<DotIndicatorPaintProperty>();
    indicatorPattern->GetMouseClickIndex();
    paintProperty->UpdateIsCustomSize(true);
    indicatorPattern->GetMouseClickIndex();
    ASSERT_TRUE(paintProperty->GetIsCustomSizeValue(false));
}

/**
 * @tc.name: SwiperIndicatorGetMouseClickIndex002
 * @tc.desc: Test GetMouseClickIndex
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperIndicatorGetMouseClickIndex002, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetDirection(Axis::HORIZONTAL);
    });
    RefPtr<SwiperIndicatorPattern> indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    auto paintProperty = indicatorNode_->GetPaintProperty<DotIndicatorPaintProperty>();
    /**
     * @tc.steps: step1. call no mirror func.
     */
    layoutProperty_->UpdateLayoutDirection(TextDirection::LTR);
    MouseClickIndicator(SourceType::MOUSE, Offset(72.f, 16.f));
    indicatorPattern->GetMouseClickIndex();
    EXPECT_EQ(indicatorPattern->mouseClickIndex_, 3);
    /**
     * @tc.steps: step2. call mirror func.
     */
    layoutProperty_->UpdateLayoutDirection(TextDirection::RTL);
    indicatorPattern->GetMouseClickIndex();
    EXPECT_EQ(indicatorPattern->mouseClickIndex_, 0);
}

/**
 * @tc.name: SwiperIndicatorPatternTestNg0014
 * @tc.desc: HandleMouseClick
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperIndicatorPatternTestNg0014, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    auto paintProperty_ = pattern_->GetPaintProperty<SwiperPaintProperty>();
    CHECK_NULL_VOID(paintProperty_);
    indicatorPattern->isRepeatClicked_ = true;
    auto info = GestureEvent();
    indicatorPattern->HandleMouseClick(info);
}

/**
 * @tc.name: SwiperIndicatorPatternTestNg0017
 * @tc.desc: InitTouchEvent
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperIndicatorPatternTestNg0017, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    auto paintProperty_ = pattern_->GetPaintProperty<SwiperPaintProperty>();
    CHECK_NULL_VOID(paintProperty_);
    TouchEventInfo touchEventInfo("down");
    indicatorPattern->touchEvent_ = nullptr;
    auto gestureHub = frameNode_->GetOrCreateGestureEventHub();
    indicatorPattern->InitTouchEvent(gestureHub);
    indicatorPattern->touchEvent_->callback_(touchEventInfo);
}

/**
 * @tc.name: SwiperPatternCheckMarkDirtyNodeForRenderIndicator002
 * @tc.desc: Test CheckMarkDirtyNodeForRenderIndicator
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperPatternCheckMarkDirtyNodeForRenderIndicator002, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    RefPtr<SwiperPattern> indicatorPattern = frameNode_->GetPattern<SwiperPattern>();
    float additionalOffset = -1.0f;
    indicatorPattern->itemPosition_.emplace(std::make_pair(0, SwiperItemInfo { 0.0f, 0.0f }));
    indicatorPattern->itemPosition_.emplace(std::make_pair(3, SwiperItemInfo { 1.0f, 0.0f }));
    indicatorPattern->itemPosition_.emplace(std::make_pair(1, SwiperItemInfo { 0.0f, 2.0f }));
    indicatorPattern->itemPosition_.emplace(std::make_pair(2, SwiperItemInfo { 1.0f, 2.0f }));

    /**
     * @tc.steps: step2. test CheckMarkDirtyNodeForRenderIndicator.
     * @tc.expected: Related function runs ok.
     */
    indicatorPattern->indicatorId_ = 1;
    indicatorPattern->CheckMarkDirtyNodeForRenderIndicator(additionalOffset);
}

/**
 * @tc.name: SwiperPatternPlayIndicatorTranslateAnimation004
 * @tc.desc: PlayIndicatorTranslateAnimation
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperPatternPlayIndicatorTranslateAnimation004, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    pattern_->stopIndicatorAnimation_ = false;
    pattern_->itemPosition_.emplace(std::make_pair(0, SwiperItemInfo { 0.0f, 0.0f }));
    float translate = 0.1f;
    pattern_->swiperController_->SetTurnPageRateCallback(nullptr);

    /**
     * @tc.steps: step2. call PlayIndicatorTranslateAnimation.
     * @tc.expected: Related function runs ok.
     */
    pattern_->stopIndicatorAnimation_ = true;
    pattern_->indicatorId_ = 1;
    for (int i = 0; i <= 1; i++) {
        for (int j = 0; j <= 1; j++) {
            pattern_->PlayIndicatorTranslateAnimation(translate);
            if (i == 1) {
                pattern_->swiperController_->SetTurnPageRateCallback(nullptr);
                continue;
            }
            pattern_->swiperController_->SetTurnPageRateCallback([](int32_t, float) {});
        }
        pattern_->itemPosition_.clear();
        pattern_->itemPosition_.emplace(std::make_pair(0, SwiperItemInfo { 1.0f, 2.0f }));
    }
}

/**
 * @tc.name: SwiperIndicatorPatternTestNg0018
 * @tc.desc: HandleTouchClick
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperIndicatorPatternTestNg0018, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    auto paintProperty_ = pattern_->GetPaintProperty<SwiperPaintProperty>();
    CHECK_NULL_VOID(paintProperty_);
    auto info = GestureEvent();
    layoutProperty_->UpdateDirection(Axis::NONE);
    info.localLocation_.SetX(5.0f);
    indicatorPattern->HandleTouchClick(info);
    layoutProperty_->UpdateDirection(Axis::NONE);
    info.localLocation_.SetX(500.0f);
    indicatorPattern->HandleTouchClick(info);
}

/**
 * @tc.name: SwiperIndicatorPatternTestNg0019
 * @tc.desc: UpdateTextContentSub
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperIndicatorPatternTestNg0019, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    auto layoutProperty = indicatorNode_->GetLayoutProperty<SwiperIndicatorLayoutProperty>();
    auto paintProperty_ = pattern_->GetPaintProperty<SwiperPaintProperty>();
    CHECK_NULL_VOID(paintProperty_);

    auto firstTextNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    auto lastTextNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    pattern_->currentFirstIndex_ = -2;
    layoutProperty_->UpdateIndex(1);
    indicatorPattern->UpdateTextContentSub(layoutProperty, firstTextNode, lastTextNode);
}

/**
 * @tc.name: SwiperIndicatorPatternTestNg0020
 * @tc.desc: CheckIsTouchBottom
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, SwiperIndicatorPatternTestNg0020, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    GestureEvent info;
    info.mainDelta_ = 1.0f;
    TouchLocationInfo touchLocationInfo("down", 0);
    touchLocationInfo.SetTouchType(TouchType::DOWN);
    std::list<TouchLocationInfo> infoSwiper;
    infoSwiper.emplace_back(touchLocationInfo);
    TouchEventInfo touchEventInfo("down");
    touchEventInfo.touches_ = infoSwiper;
    pattern_->currentIndex_ = 0;
    EXPECT_TRUE(indicatorPattern->CheckIsTouchBottom(touchEventInfo.GetTouches().front()));
    layoutProperty_->UpdateLoop(false);
    ASSERT_FALSE(layoutProperty_->GetLoop().value_or(true));
    pattern_->leftButtonId_ = 1;
    pattern_->rightButtonId_ = 1;
    pattern_->GetLayoutProperty<SwiperLayoutProperty>()->UpdateShowIndicator(true);
    layoutProperty_->UpdateDirection(Axis::HORIZONTAL);
    touchEventInfo.touches_.front().localLocation_.SetX(2.0f);
    indicatorPattern->dragStartPoint_.SetX(1.0f);
    EXPECT_FALSE(indicatorPattern->CheckIsTouchBottom(touchEventInfo.GetTouches().front()));
}

/**
 * @tc.name: CircleSwiperIndicatorPatternCheckIsTouchBottom001
 * @tc.desc: CheckIsTouchBottom
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, CircleSwiperIndicatorPatternCheckIsTouchBottom001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create swiper and set parameters.
     */
    CreateWithItem([](SwiperModelNG model) {
        model.Create(true);
        model.SetDirection(Axis::VERTICAL);
        model.SetIndicatorType(SwiperIndicatorType::ARC_DOT);
    });
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    GestureEvent info;
    TouchLocationInfo touchLocationInfo("down", 0);
    touchLocationInfo.SetTouchType(TouchType::DOWN);
    std::list<TouchLocationInfo> infoSwiper;
    infoSwiper.emplace_back(touchLocationInfo);
    TouchEventInfo touchEventInfo("down");
    touchEventInfo.touches_ = infoSwiper;
    indicatorPattern->swiperIndicatorType_ = SwiperIndicatorType::ARC_DOT;
    indicatorPattern->HandleLongDragUpdate(touchLocationInfo);

    /**
     * @tc.steps: step2. call CheckIsTouchBottom.
     */
    EXPECT_FALSE(indicatorPattern->CheckIsTouchBottom(info));
    EXPECT_TRUE(indicatorPattern->CheckIsTouchBottom(touchEventInfo.GetTouches().front()));
}

/**
 * @tc.name: CircleSwiperIndicatorPatternCheckIsTouchBottom002
 * @tc.desc: CheckIsTouchBottom
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, CircleSwiperIndicatorPatternCheckIsTouchBottom002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create swiper and set parameters.
     */
    CreateWithItem([](SwiperModelNG model) {
        model.Create(true);
        model.SetDirection(Axis::VERTICAL);
        model.SetIndicatorType(SwiperIndicatorType::ARC_DOT);
    });
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();
    TouchLocationInfo touchLocationInfo("down", 2);
    indicatorPattern->swiperIndicatorType_ = SwiperIndicatorType::ARC_DOT;
    indicatorPattern->HandleLongDragUpdate(touchLocationInfo);

    /**
     * @tc.steps: step2. call CheckIsTouchBottom.
     */
    EXPECT_TRUE(indicatorPattern->CheckIsTouchBottom(touchLocationInfo));
}

/**
 * @tc.name: CircleSwiperIndicatorPatternConvertAngleWithArcDirection001
 * @tc.desc: ConvertAngleWithArcDirection
 * @tc.type: FUNC
 */
HWTEST_F(SwiperIndicatorTestNg, CircleSwiperIndicatorPatternConvertAngleWithArcDirection001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create swiper and set parameters.
     */
    CreateWithItem([](SwiperModelNG model) {
        model.Create(true);
        model.SetIndicatorType(SwiperIndicatorType::ARC_DOT);
    });
    auto indicatorPattern = indicatorNode_->GetPattern<SwiperIndicatorPattern>();

    /**
     * @tc.steps: step2. call ConvertAngleWithArcDirection.
     */
    EXPECT_EQ(indicatorPattern->ConvertAngleWithArcDirection(SwiperArcDirection::THREE_CLOCK_DIRECTION, 91.0f),
        -179.0f);
    EXPECT_EQ(indicatorPattern->ConvertAngleWithArcDirection(SwiperArcDirection::THREE_CLOCK_DIRECTION, 89.0f),
        179.0f);
    EXPECT_EQ(indicatorPattern->ConvertAngleWithArcDirection(SwiperArcDirection::NINE_CLOCK_DIRECTION, -91.0f),
        179.0f);
    EXPECT_EQ(indicatorPattern->ConvertAngleWithArcDirection(SwiperArcDirection::NINE_CLOCK_DIRECTION, 1.0f),
        -89.0f);
}
} // namespace OHOS::Ace::NG
