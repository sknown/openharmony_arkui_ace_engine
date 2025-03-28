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
#include "test/unittest/core/gestures/gestures_common_test_ng.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
class GestureRecognizerTestNg : public GesturesCommonTestNg {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};

void GestureRecognizerTestNg::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
}

void GestureRecognizerTestNg::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
}

/**
 * @tc.name: TriggerGestureJudgeCallbackTest001
 * @tc.desc: Test Recognizer function: TriggerGestureJudgeCallbackTest001
 * @tc.type: FUNC
 */
HWTEST_F(GestureRecognizerTestNg, TriggerGestureJudgeCallbackTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create Recognizer、TargetComponent.
     */
    RefPtr<ClickRecognizer> clickRecognizerPtr = AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    RefPtr<LongPressRecognizer> longPressRecognizerPtr = AceType::MakeRefPtr<LongPressRecognizer>(LONG_PRESS_DURATION,
        FINGER_NUMBER, false);
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<PanRecognizer> panRecognizerPtr = AceType::MakeRefPtr<PanRecognizer>(panGestureOption);
    RefPtr<PinchRecognizer> pinchRecognizerPtr = AceType::MakeRefPtr<PinchRecognizer>(SINGLE_FINGER_NUMBER,
        PINCH_GESTURE_DISTANCE);
    RefPtr<RotationRecognizer> rotationRecognizerPtr =
        AceType::MakeRefPtr<RotationRecognizer>(SINGLE_FINGER_NUMBER, ROTATION_GESTURE_ANGLE);
    SwipeDirection swipeDirection;
    RefPtr<SwipeRecognizer> swipeRecognizerPtr =
        AceType::MakeRefPtr<SwipeRecognizer>(SINGLE_FINGER_NUMBER, swipeDirection, SWIPE_SPEED);

    RefPtr<NG::TargetComponent> targetComponent = AceType::MakeRefPtr<TargetComponent>();

    auto gestureJudgeFunc = [](const RefPtr<GestureInfo>& gestureInfo, const std::shared_ptr<BaseGestureEvent>& info) {
        return GestureJudgeResult::REJECT;};
    targetComponent->SetOnGestureJudgeBegin(gestureJudgeFunc);
    /**
     * @tc.steps: step2. call TriggerGestureJudgeCallback function and compare result.
     * @tc.expected: step2. result equals CONTINUE.
     */
    clickRecognizerPtr->gestureInfo_ = AceType::MakeRefPtr<GestureInfo>();
    clickRecognizerPtr->gestureInfo_->type_ = GestureTypeName::DRAG;
    auto result = clickRecognizerPtr->TriggerGestureJudgeCallback();
    EXPECT_EQ(result, GestureJudgeResult::CONTINUE);
    longPressRecognizerPtr->gestureInfo_ = AceType::MakeRefPtr<GestureInfo>();
    longPressRecognizerPtr->gestureInfo_->type_ = GestureTypeName::DRAG;
    result = longPressRecognizerPtr->TriggerGestureJudgeCallback();
    panRecognizerPtr->gestureInfo_ = AceType::MakeRefPtr<GestureInfo>();
    panRecognizerPtr->gestureInfo_->type_ = GestureTypeName::DRAG;
    result = panRecognizerPtr->TriggerGestureJudgeCallback();
    pinchRecognizerPtr->gestureInfo_ = AceType::MakeRefPtr<GestureInfo>();
    pinchRecognizerPtr->gestureInfo_->type_ = GestureTypeName::DRAG;
    result = pinchRecognizerPtr->TriggerGestureJudgeCallback();
    rotationRecognizerPtr->gestureInfo_ = AceType::MakeRefPtr<GestureInfo>();
    rotationRecognizerPtr->gestureInfo_->type_ = GestureTypeName::DRAG;
    result = rotationRecognizerPtr->TriggerGestureJudgeCallback();
    swipeRecognizerPtr->gestureInfo_ = AceType::MakeRefPtr<GestureInfo>();
    swipeRecognizerPtr->gestureInfo_->type_ = GestureTypeName::DRAG;
    result = swipeRecognizerPtr->TriggerGestureJudgeCallback();
    EXPECT_EQ(result, GestureJudgeResult::CONTINUE);
    /**
     * @tc.steps: step3. targetComponent_ is not null, call TriggerGestureJudgeCallback function and compare result.
     * @tc.expected: step3. result equals PREVENT.
     */
    clickRecognizerPtr->targetComponent_ = targetComponent;
    EXPECT_EQ(clickRecognizerPtr->TriggerGestureJudgeCallback(), GestureJudgeResult::REJECT);

    longPressRecognizerPtr->targetComponent_ = targetComponent;
    EXPECT_EQ(longPressRecognizerPtr->TriggerGestureJudgeCallback(), GestureJudgeResult::REJECT);

    panRecognizerPtr->targetComponent_ = targetComponent;
    EXPECT_EQ(panRecognizerPtr->TriggerGestureJudgeCallback(), GestureJudgeResult::REJECT);

    pinchRecognizerPtr->targetComponent_ = targetComponent;
    EXPECT_EQ(pinchRecognizerPtr->TriggerGestureJudgeCallback(), GestureJudgeResult::REJECT);

    rotationRecognizerPtr->targetComponent_ = targetComponent;
    EXPECT_EQ(rotationRecognizerPtr->TriggerGestureJudgeCallback(), GestureJudgeResult::REJECT);

    swipeRecognizerPtr->targetComponent_ = targetComponent;
    EXPECT_EQ(swipeRecognizerPtr->TriggerGestureJudgeCallback(), GestureJudgeResult::REJECT);
}

/**
 * @tc.name: TransformTest001
 * @tc.desc: Test Transform in Default Condition
 */
HWTEST_F(GestureRecognizerTestNg, TransformTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create FrameNode.
     */
    RefPtr<FrameNode> FRAME_NODE_0 = FrameNode::CreateFrameNode("0", 0, AceType::MakeRefPtr<Pattern>());
    RefPtr<FrameNode> FRAME_NODE_1 = FrameNode::CreateFrameNode("1", 1, AceType::MakeRefPtr<Pattern>());
    RefPtr<FrameNode> FRAME_NODE_2 = FrameNode::CreateFrameNode("2", 2, AceType::MakeRefPtr<Pattern>());
    FRAME_NODE_2->SetParent(WeakPtr<FrameNode>(FRAME_NODE_1));
    FRAME_NODE_1->SetParent(WeakPtr<FrameNode>(FRAME_NODE_0));

    /**
     * @tc.steps: step2. mock local matrix.
     */
    FRAME_NODE_0->localMat_ = Matrix4::CreateIdentity();
    FRAME_NODE_1->localMat_ = Matrix4::CreateIdentity();
    FRAME_NODE_2->localMat_ = Matrix4::CreateIdentity();

    /**
     * @tc.steps: step2. call callback function.
     */
    PointF f1(1.0, 1.0);
    NGGestureRecognizer::Transform(f1, WeakPtr<FrameNode>(FRAME_NODE_2));
    PointF f2(1.000000, 1.000000);
    EXPECT_EQ(f1, f2);
}

/**
 * @tc.name: TransformTest002
 * @tc.desc: Test Transform with Matrix
 */
HWTEST_F(GestureRecognizerTestNg, TransformTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create FrameNode.
     */
    RefPtr<FrameNode> FRAME_NODE_0 = FrameNode::CreateFrameNode("0", 0, AceType::MakeRefPtr<Pattern>());
    RefPtr<FrameNode> FRAME_NODE_1 = FrameNode::CreateFrameNode("1", 1, AceType::MakeRefPtr<Pattern>());
    RefPtr<FrameNode> FRAME_NODE_2 = FrameNode::CreateFrameNode("2", 2, AceType::MakeRefPtr<Pattern>());
    FRAME_NODE_2->SetParent(WeakPtr<FrameNode>(FRAME_NODE_1));
    FRAME_NODE_1->SetParent(WeakPtr<FrameNode>(FRAME_NODE_0));

    /**
     * @tc.steps: step2. mock local matrix.
     */
    FRAME_NODE_0->localMat_ = Matrix4::CreateIdentity();
    FRAME_NODE_1->localMat_ = Matrix4::Invert(
            Matrix4::CreateTranslate(100, 200, 0) * Matrix4::CreateRotate(90, 0, 0, 1) *
            Matrix4::CreateScale(0.6, 0.8, 1));
    FRAME_NODE_2->localMat_ = Matrix4::Invert(
            Matrix4::CreateTranslate(400, 300, 0) * Matrix4::CreateRotate(30, 0, 0, 1) *
            Matrix4::CreateScale(0.5, 0.5, 1));

    /**
     * @tc.steps: step3. call callback function.
     */
    PointF f1(1.0, 1.0);
    NGGestureRecognizer::Transform(f1, WeakPtr<FrameNode>(FRAME_NODE_2));
    PointF f2(-1443.533813, 426.392731);
    EXPECT_EQ(f1, f2);
}

/**
 * @tc.name: TransformTest003
 * @tc.desc: Test Transform with Matrix in Reverse Order
 */
HWTEST_F(GestureRecognizerTestNg, TransformTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create FrameNode.
     */
    RefPtr<FrameNode> FRAME_NODE_0 = FrameNode::CreateFrameNode("0", 0, AceType::MakeRefPtr<Pattern>());
    RefPtr<FrameNode> FRAME_NODE_1 = FrameNode::CreateFrameNode("1", 1, AceType::MakeRefPtr<Pattern>());
    RefPtr<FrameNode> FRAME_NODE_2 = FrameNode::CreateFrameNode("2", 2, AceType::MakeRefPtr<Pattern>());
    FRAME_NODE_2->SetParent(WeakPtr<FrameNode>(FRAME_NODE_1));
    FRAME_NODE_1->SetParent(WeakPtr<FrameNode>(FRAME_NODE_0));

    /**
     * @tc.steps: step2. mock local matrix.
     */
    FRAME_NODE_0->localMat_ = Matrix4::CreateIdentity();
    FRAME_NODE_2->localMat_ = Matrix4::Invert(
            Matrix4::CreateTranslate(100, 200, 0) * Matrix4::CreateRotate(90, 0, 0, 1) *
            Matrix4::CreateScale(0.6, 0.8, 1));
    FRAME_NODE_1->localMat_ = Matrix4::Invert(
            Matrix4::CreateTranslate(400, 300, 0) * Matrix4::CreateRotate(30, 0, 0, 1) *
            Matrix4::CreateScale(0.5, 0.5, 1));

    /**
     * @tc.steps: step3. call callback function.
     */
    PointF f1(1.0, 1.0);
    NGGestureRecognizer::Transform(f1, WeakPtr<FrameNode>(FRAME_NODE_2));
    PointF f2(-531.471924, 1362.610352);
    EXPECT_EQ(f1, f2);
}

/**
 * @tc.name: PanPressRecognizerHandleTouchMoveEventTest001
 * @tc.desc: Test PanPressRecognizer function: HandleTouchMoveEvent
 * @tc.type: FUNC
 */
HWTEST_F(GestureRecognizerTestNg, PanPressRecognizerHandleTouchMoveEventTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create and set Recognizer、TargetComponent.
     */

    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<PanRecognizer> panRecognizerPtr = AceType::MakeRefPtr<PanRecognizer>(panGestureOption);
    RefPtr<NG::TargetComponent> targetComponent = AceType::MakeRefPtr<TargetComponent>();
    auto gestureJudgeFunc = [](const RefPtr<GestureInfo>& gestureInfo, const std::shared_ptr<BaseGestureEvent>& info) {
        return GestureJudgeResult::REJECT;};
    auto frameNode = FrameNode::CreateFrameNode("myButton", 100, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    PanDirection panDirection;
    panRecognizerPtr->gestureInfo_ = AceType::MakeRefPtr<GestureInfo>();
    panRecognizerPtr->gestureInfo_->type_ = GestureTypeName::DRAG;
    targetComponent->SetOnGestureJudgeBegin(gestureJudgeFunc);
    panRecognizerPtr->targetComponent_ = targetComponent;
    panRecognizerPtr->targetComponent_->node_ = frameNode;
    TouchEvent touchEvent;
    touchEvent.tiltX.emplace(1.0f);
    touchEvent.tiltY.emplace(1.0f);
    panRecognizerPtr->touchPoints_[touchEvent.id] = touchEvent;
    panRecognizerPtr->direction_.type = PanDirection::ALL;
    panRecognizerPtr->isFlushTouchEventsEnd_ = true;
    panRecognizerPtr->touchInfoForPan.averageDistance_ = Offset(0, -1);
    panRecognizerPtr->distance_ = 0;
    panRecognizerPtr->currentFingers_ = 1;
    panRecognizerPtr->fingers_ = 1;

    /**
     * @tc.steps: step2. call HandleOverdueDeadline function and compare result.
     * @tc.steps: case1: gestureInfo_ is nullptr touchEvent
     * @tc.expected: step2. result equals REJECT.
     */
    panRecognizerPtr->refereeState_ = RefereeState::DETECTING;
    panRecognizerPtr->HandleTouchMoveEvent(touchEvent);
    EXPECT_EQ(panRecognizerPtr->disposal_, GestureDisposal::NONE);

    /**
     * @tc.steps: step2. call HandleOverdueDeadline function and compare result.
     * @tc.steps: case2: gestureInfo_ is not nullptr, gestureInfo_->type_ = DRAG
     *                   isDragUserReject_ = true touchEvent
     * @tc.expected: step2. result equals REJECT.
     */
    panRecognizerPtr->refereeState_ = RefereeState::DETECTING;
    panRecognizerPtr->gestureInfo_ = AceType::MakeRefPtr<GestureInfo>();
    panRecognizerPtr->gestureInfo_->type_ = GestureTypeName::DRAG;
    guestureEventHub->dragEventActuator_ = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(guestureEventHub)), panDirection, 1, 50.0f);
    guestureEventHub->dragEventActuator_->isDragUserReject_ = true;
    panRecognizerPtr->HandleTouchMoveEvent(touchEvent);
    EXPECT_EQ(panRecognizerPtr->disposal_, GestureDisposal::REJECT);

    /**
     * @tc.steps: step2. call HandleOverdueDeadline function and compare result.
     * @tc.steps: case3: gestureInfo_ is not nullptr, gestureInfo_->type_ = DRAG
     *                   isDragUserReject_ = false touchEvent
     * @tc.expected: step2. isDragUserReject_ = true.
     */
    panRecognizerPtr->refereeState_ = RefereeState::DETECTING;
    guestureEventHub->dragEventActuator_->isDragUserReject_ = false;
    panRecognizerPtr->HandleTouchMoveEvent(touchEvent);
    EXPECT_TRUE(guestureEventHub->dragEventActuator_->isDragUserReject_);
}

/**
 * @tc.name: PanPressRecognizerHandleTouchMoveEventTest002
 * @tc.desc: Test PanPressRecognizer function: HandleTouchMoveEvent
 * @tc.type: FUNC
 */
HWTEST_F(GestureRecognizerTestNg, PanPressRecognizerHandleTouchMoveEventTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create and set Recognizer、TargetComponent.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<PanRecognizer> panRecognizerPtr = AceType::MakeRefPtr<PanRecognizer>(panGestureOption);
    RefPtr<NG::TargetComponent> targetComponent = AceType::MakeRefPtr<TargetComponent>();
    auto gestureJudgeFunc = [](const RefPtr<GestureInfo>& gestureInfo, const std::shared_ptr<BaseGestureEvent>& info) {
        return GestureJudgeResult::REJECT;};
    auto frameNode = FrameNode::CreateFrameNode("myButton", 100, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    PanDirection panDirection;
    panRecognizerPtr->gestureInfo_ = AceType::MakeRefPtr<GestureInfo>();
    panRecognizerPtr->gestureInfo_->type_ = GestureTypeName::DRAG;
    targetComponent->SetOnGestureJudgeBegin(gestureJudgeFunc);
    panRecognizerPtr->targetComponent_ = targetComponent;
    panRecognizerPtr->targetComponent_->node_ = frameNode;
    TouchEvent touchEvent;
    touchEvent.tiltX.emplace(1.0f);
    touchEvent.tiltY.emplace(1.0f);
    AxisEvent axisEvent;
    panRecognizerPtr->touchPoints_[touchEvent.id] = touchEvent;
    panRecognizerPtr->direction_.type = PanDirection::ALL;
    panRecognizerPtr->isFlushTouchEventsEnd_ = true;
    panRecognizerPtr->axisInfoForPan.averageDistance_ = Offset(0, -1);
    panRecognizerPtr->distance_ = 0;
    panRecognizerPtr->currentFingers_ = 1;
    panRecognizerPtr->fingers_ = 1;

    /**
     * @tc.steps: step2. call HandleOverdueDeadline function and compare result.
     * @tc.steps: case1: gestureInfo_ is nullptr axisEvent
     * @tc.expected: step2. result equals REJECT.
     */
    panRecognizerPtr->inputEventType_ = InputEventType::AXIS;
    panRecognizerPtr->refereeState_ = RefereeState::DETECTING;
    panRecognizerPtr->HandleTouchMoveEvent(axisEvent);
    EXPECT_EQ(panRecognizerPtr->disposal_, GestureDisposal::NONE);

    /**
     * @tc.steps: step2. call HandleOverdueDeadline function and compare result.
     * @tc.steps: case2: gestureInfo_ is not nullptr, gestureInfo_->type_ = DRAG
     *                   isDragUserReject_ = true axisEvent
     * @tc.expected: step2. result equals REJECT.
     */
    panRecognizerPtr->refereeState_ = RefereeState::DETECTING;
    panRecognizerPtr->gestureInfo_ = AceType::MakeRefPtr<GestureInfo>();
    panRecognizerPtr->gestureInfo_->type_ = GestureTypeName::DRAG;
    guestureEventHub->dragEventActuator_ = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(guestureEventHub)), panDirection, 1, 50.0f);
    guestureEventHub->dragEventActuator_->isDragUserReject_ = true;
    panRecognizerPtr->HandleTouchMoveEvent(axisEvent);
    EXPECT_EQ(panRecognizerPtr->disposal_, GestureDisposal::REJECT);

    /**
     * @tc.steps: step2. call HandleOverdueDeadline function and compare result.
     * @tc.steps: case3: gestureInfo_ is not nullptr, gestureInfo_->type_ = DRAG
     *                   isDragUserReject_ = false axisEvent
     * @tc.expected: step2. isDragUserReject_ = true.
     */
    panRecognizerPtr->refereeState_ = RefereeState::DETECTING;
    guestureEventHub->dragEventActuator_->isDragUserReject_ = false;
    panRecognizerPtr->HandleTouchMoveEvent(axisEvent);
    EXPECT_TRUE(guestureEventHub->dragEventActuator_->isDragUserReject_);
}
} // namespace OHOS::Ace::NG