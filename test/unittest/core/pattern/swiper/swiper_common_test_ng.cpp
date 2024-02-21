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
#include "core/components/swiper/swiper_component.h"

namespace OHOS::Ace::NG {

namespace {} // namespace

class SwiperCommonTestNg : public SwiperTestNg {
public:
};

/**
 * @tc.name: HandleTouchEvent001
 * @tc.desc: Test HandleTouchEvent invalid args
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, HandleTouchEvent001, TestSize.Level1)
{
    /**
     * @tc.cases: Call HandleTouchEvent with empty TouchLocationInfo
     * @tc.expected: isTouchDown_ is still false when touch
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_FALSE(pattern_->isTouchDown_);

    pattern_->HandleTouchEvent(TouchEventInfo("touch"));
    EXPECT_FALSE(pattern_->isTouchDown_);
}

/**
 * @tc.name: HandleTouchEvent002
 * @tc.desc: Test HandleTouchEvent invalid args
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, HandleTouchEvent002, TestSize.Level1)
{
    /**
     * @tc.cases: Call HandleTouchEvent with invalid TouchType::UNKNOWN
     * @tc.expected: isTouchDown_ is still false when touch
     */
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_FALSE(pattern_->isTouchDown_);

    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::UNKNOWN, Offset()));
    EXPECT_FALSE(pattern_->isTouchDown_);
}

/**
 * @tc.name: HandleTouchEvent003
 * @tc.desc: When touch down, the animation will stop. When touch up, the animation will run.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, HandleTouchEvent003, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    pattern_->springAnimationIsRunning_ = true;

    /**
     * @tc.steps: step1. Touch down a location
     * @tc.expected: isTouchDown_ is true, Animation stop
     */
    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::DOWN, Offset()));
    EXPECT_TRUE(pattern_->isTouchDown_);
    EXPECT_FALSE(pattern_->springAnimationIsRunning_);

    /**
     * @tc.steps: step2. Touch up
     * @tc.expected: isTouchDown_ is false, Animation resume
     */
    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::UP, Offset()));
    EXPECT_FALSE(pattern_->isTouchDown_);
    EXPECT_TRUE(pattern_->springAnimationIsRunning_);
}

/**
 * @tc.name: HandleTouchEvent004
 * @tc.desc: When touch down, the animation will stop. When touch cancel, the animation will run.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, HandleTouchEvent004, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    pattern_->fadeAnimationIsRunning_ = true;

    /**
     * @tc.steps: step1. Touch down a location
     * @tc.expected: Animation stoped
     */
    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::DOWN, Offset()));
    EXPECT_TRUE(pattern_->isTouchDown_);
    EXPECT_FALSE(pattern_->fadeAnimationIsRunning_);

    /**
     * @tc.steps: step2. Touch cancel
     * @tc.expected: Animation resume
     */
    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::CANCEL, Offset()));
    EXPECT_FALSE(pattern_->isTouchDown_);
    EXPECT_TRUE(pattern_->fadeAnimationIsRunning_);
}

/**
 * @tc.name: HandleTouchEvent005
 * @tc.desc: When touch down on indicatorNode, the animation will not stop
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, HandleTouchEvent005, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    pattern_->springAnimationIsRunning_ = true;

    /**
     * @tc.steps: step1. Touch down on indicatorNode
     * @tc.expected: Animation still running
     */
    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::DOWN, Offset(SWIPER_WIDTH / 2, SWIPER_HEIGHT)));
    EXPECT_TRUE(pattern_->isTouchDown_);
    EXPECT_TRUE(pattern_->springAnimationIsRunning_);
    
    /**
     * @tc.steps: step2. Touch up
     * @tc.expected: Animation still running
     */
    pattern_->HandleTouchEvent(CreateTouchEventInfo(TouchType::UP, Offset(SWIPER_WIDTH / 2, SWIPER_HEIGHT)));
    EXPECT_FALSE(pattern_->isTouchDown_);
    EXPECT_TRUE(pattern_->springAnimationIsRunning_);
}

/**
 * @tc.name: SwiperController001
 * @tc.desc: Test SwiperController func
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, SwiperController001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {
        model.SetDuration(0.f); // for SwipeToWithoutAnimation
    });
    RefPtr<SwiperController> controller = pattern_->GetSwiperController();
    EXPECT_EQ(pattern_->GetCurrentShownIndex(), 0);

    /**
     * @tc.steps: step1. Call ShowNext
     * @tc.expected: Show next page
     */
    controller->ShowNext();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetCurrentShownIndex(), 1);
    
    /**
     * @tc.steps: step2. Call ShowPrevious
     * @tc.expected: Show previous page
     */
    controller->ShowPrevious();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->GetCurrentShownIndex(), 0);
    
    /**
     * @tc.steps: step3. Call FinishAnimation
     * @tc.expected: Animation stoped
     */
    controller->FinishAnimation();
    EXPECT_TRUE(pattern_->isUserFinish_);
}

/**
 * @tc.name: Event001
 * @tc.desc: Test OnChange event
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, Event001, TestSize.Level1)
{
    int32_t currentIndex = 0;
    auto onChange = [&currentIndex](const BaseEventInfo* info) {
        const auto* swiperInfo = TypeInfoHelper::DynamicCast<SwiperChangeEvent>(info);
        currentIndex = swiperInfo->GetIndex();
    };
    CreateWithItem([=](SwiperModelNG model) {
        model.SetOnChange(std::move(onChange));
    });
    RefPtr<SwiperController> controller = pattern_->GetSwiperController();

    /**
     * @tc.steps: step1. Show next page
     * @tc.expected: currentIndex change to 1
     */
    controller->ShowNext();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(currentIndex, 1);

    /**
     * @tc.steps: step2. Show previous page
     * @tc.expected: currentIndex change to 0
     */
    controller->ShowPrevious();
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(currentIndex, 0);
}

/**
 * @tc.name: Event002
 * @tc.desc: Test OnAnimationStart OnAnimationEnd event
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, Event002, TestSize.Level1)
{
    bool isAnimationStart = false;
    auto onAnimationStart =
        [&isAnimationStart](int32_t index, int32_t targetIndex, const AnimationCallbackInfo& info) {
            isAnimationStart = true;
        };
    bool isAnimationEnd = false;
    auto onAnimationEnd = [&isAnimationEnd](int32_t index, const AnimationCallbackInfo& info) {
        isAnimationEnd = true;
    };
    CreateWithItem([=](SwiperModelNG model) {
        model.SetOnAnimationStart(std::move(onAnimationStart));
        model.SetOnAnimationEnd(std::move(onAnimationEnd));
    });
    RefPtr<SwiperController> controller = pattern_->GetSwiperController();

    /**
     * @tc.steps: step1. Show next page
     * @tc.expected: Animation event will be called
     */
    controller->ShowNext();
    FlushLayoutTask(frameNode_);
    EXPECT_TRUE(isAnimationStart);
    EXPECT_TRUE(isAnimationEnd);
}

/**
 * @tc.name: AccessibilityProperty001
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, AccessibilityProperty001, TestSize.Level1)
{
    CreateWithItem([](SwiperModelNG model) {});
    EXPECT_EQ(accessibilityProperty_->GetCurrentIndex(), 0);
    EXPECT_EQ(accessibilityProperty_->GetBeginIndex(), 0);
    EXPECT_EQ(accessibilityProperty_->GetEndIndex(), 0);
    AccessibilityValue result = accessibilityProperty_->GetAccessibilityValue();
    EXPECT_EQ(result.min, 0);
    EXPECT_EQ(result.max, 3);
    EXPECT_EQ(result.current, 0);
    EXPECT_TRUE(accessibilityProperty_->IsScrollable());
    EXPECT_EQ(accessibilityProperty_->GetCollectionItemCounts(), 4);
}

/**
 * @tc.name: AccessibilityProperty002
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, AccessibilityProperty002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set loop to false
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetLoop(false);
    });

    /**
     * @tc.steps: step2. Current is first page
     * @tc.expected: ACTION_SCROLL_FORWARD
     */
    accessibilityProperty_->ResetSupportAction(); // call SetSpecificSupportAction
    uint64_t exptectActions = 0;
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD);
    EXPECT_EQ(GetActions(accessibilityProperty_), exptectActions);
}

/**
 * @tc.name: AccessibilityProperty003
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, AccessibilityProperty003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set loop to false
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetLoop(false);
    });

    /**
     * @tc.steps: step2. Show next page, Current is second(middle) page
     * @tc.expected: ACTION_SCROLL_FORWARD ACTION_SCROLL_BACKWARD
     */
    RefPtr<SwiperController> controller = pattern_->GetSwiperController();
    controller->ShowNext();
    FlushLayoutTask(frameNode_);
    accessibilityProperty_->ResetSupportAction();
    uint64_t exptectActions = 0;
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD);
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD);
    EXPECT_EQ(GetActions(accessibilityProperty_), exptectActions);
}

/**
 * @tc.name: AccessibilityProperty004
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, AccessibilityProperty004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set loop to false
     */
    CreateWithItem([](SwiperModelNG model) {
        model.SetLoop(false);
    });

    /**
     * @tc.steps: step2. Show last page, Current is last page
     * @tc.expected: ACTION_SCROLL_BACKWARD
     */
    RefPtr<SwiperController> controller = pattern_->GetSwiperController();
    controller->ShowNext(); // call three times to last page
    FlushLayoutTask(frameNode_);
    controller->ShowNext();
    FlushLayoutTask(frameNode_);
    controller->ShowNext();
    FlushLayoutTask(frameNode_);
    accessibilityProperty_->ResetSupportAction();
    uint64_t exptectActions = 0;
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD);
    EXPECT_EQ(GetActions(accessibilityProperty_), exptectActions);
}

/**
 * @tc.name: AccessibilityProperty005
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, AccessibilityProperty005, TestSize.Level1)
{
    /**
     * @tc.cases: Swiper is loop
     * @tc.expected: ACTION_SCROLL_FORWARD ACTION_SCROLL_BACKWARD
     */
    CreateWithItem([](SwiperModelNG model) {});
    accessibilityProperty_->ResetSupportAction(); // call SetSpecificSupportAction
    uint64_t exptectActions = 0;
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD);
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD);
    EXPECT_EQ(GetActions(accessibilityProperty_), exptectActions);
}

/**
 * @tc.name: AccessibilityProperty006
 * @tc.desc: Test AccessibilityProperty
 * @tc.type: FUNC
 */
HWTEST_F(SwiperCommonTestNg, AccessibilityProperty006, TestSize.Level1)
{
    /**
     * @tc.cases: Create unscrollable swiepr
     * @tc.expected: exptectActions is 0
     */
    Create([](SwiperModelNG model) {
        model.SetLoop(false);
        CreateItem(1);
    });
    accessibilityProperty_->ResetSupportAction(); // call SetSpecificSupportAction
    uint64_t exptectActions = 0;
    EXPECT_EQ(GetActions(accessibilityProperty_), exptectActions);
}
} // namespace OHOS::Ace::NG
