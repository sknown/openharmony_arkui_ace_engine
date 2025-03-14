/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <string>

#define private public
#define protected public

#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/render/mock_render_context.h"

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components/theme/blur_style_theme.h"
#include "core/components/theme/shadow_theme.h"
#include "core/components_ng/event/drag_event.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/grid/grid_item_pattern.h"
#include "core/components_ng/pattern/list/list_item_pattern.h"
#include "core/components_ng/pattern/grid/grid_pattern.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/event/touch_event.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr double GESTURE_EVENT_PROPERTY_DEFAULT_VALUE = 0.0;
constexpr double GESTURE_EVENT_PROPERTY_VALUE = 10.0;
const PanDirection DRAG_DIRECTION = { PanDirection::LEFT };
const TouchRestrict DRAG_TOUCH_RESTRICT = { TouchRestrict::CLICK };
const TouchRestrict DRAG_TOUCH_RESTRICT_MOUSE = {
    .forbiddenType = TouchRestrict::CLICK,
    .sourceType = SourceType::MOUSE
};
constexpr int32_t UNKNOWN = 1;
constexpr int32_t FINGERS_NUMBER = 2;
constexpr int32_t TOUCH_TEST_RESULT_SIZE = 1;
constexpr int32_t TOUCH_TEST_RESULT_SIZE_2 = 2;
constexpr int32_t NUMBER_BADGE_SIZE_3 = 3;
constexpr int32_t NUMBER_BADGE_SIZE_100 = 100;
constexpr float DISTANCE = 10.5f;
constexpr float WIDTH = 400.0f;
constexpr float HEIGHT = 400.0f;
const OffsetF COORDINATE_OFFSET(WIDTH, HEIGHT);
constexpr int32_t FINGERS_NUMBER_GREATER_THAN_DEFAULT = 2;
constexpr float DISTANCE_GREATER_THAN_DEFAULT = 6.0f;
constexpr float DISTANCE_EQUAL_DEFAULT = 5.0f;
constexpr float IMAGE_INVALID_RECT_WIDTH = 100.0f;
const std::string COMPONENT_ID = "id of component which you want to get screenshot from";
const std::string NO_COMPONENT_ID = "";
const std::string TOUCH_EVENT_INFO_TYPE = "onTouchDown";
} // namespace

class DragEventTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp() override;
    void TearDown() override;
};

void DragEventTestNg::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "DragEventTestNg SetUpTestCase";
}

void DragEventTestNg::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "DragEventTestNg TearDownTestCase";
}

void DragEventTestNg::SetUp()
{
    MockPipelineContext::SetUp();
}

void DragEventTestNg::TearDown()
{
    MockPipelineContext::TearDown();
}

/**
 * @tc.name: DragEventTest001
 * @tc.desc: Create DragEvent and execute its callback functions.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventFunc as the arguments of the construction of DragEvent.
     */
    double unknownPropertyValue = 0.0;
    GestureEventFunc actionStart = [&unknownPropertyValue](
                                       GestureEvent& info) { unknownPropertyValue = info.GetScale(); };
    GestureEventFunc actionUpdate = [&unknownPropertyValue](
                                        GestureEvent& info) { unknownPropertyValue = info.GetAngle(); };
    GestureEventFunc actionEnd = [&unknownPropertyValue](
                                     GestureEvent& info) { unknownPropertyValue = info.GetOffsetX(); };
    GestureEventNoParameter actionCancel = [&unknownPropertyValue]() {
        unknownPropertyValue = GESTURE_EVENT_PROPERTY_VALUE;
    };

    /**
     * @tc.steps: step2. Create DragEvent.
     */
    const DragEvent dragEvent =
        DragEvent(std::move(actionStart), std::move(actionUpdate), std::move(actionEnd), std::move(actionCancel));

    /**
     * @tc.steps: step3. Get and execute DragEvent ActionStartEvent.
     * @tc.expected: Execute ActionStartEvent which unknownPropertyValue is assigned in.
     */
    GestureEvent info = GestureEvent();
    info.SetScale(GESTURE_EVENT_PROPERTY_VALUE);
    dragEvent.GetActionStartEventFunc()(info);
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);

    /**
     * @tc.steps: step4. Get and execute DragEvent ActionUpdateEvent.
     * @tc.expected: Execute ActionUpdateEvent which unknownPropertyValue is assigned in.
     */
    unknownPropertyValue = 0.0;
    info.SetAngle(GESTURE_EVENT_PROPERTY_VALUE);
    dragEvent.GetActionUpdateEventFunc()(info);
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);

    /**
     * @tc.steps: step5. Get and execute DragEvent ActionEndEvent.
     * @tc.expected: Execute ActionEndEvent which unknownPropertyValue is assigned in.
     */
    unknownPropertyValue = 0.0;
    info.SetOffsetX(GESTURE_EVENT_PROPERTY_VALUE);
    dragEvent.GetActionEndEventFunc()(info);
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);

    /**
     * @tc.steps: step6. Get and execute DragEvent ActionCancelEvent.
     * @tc.expected: Execute ActionCancelEvent which unknownPropertyValue is assigned in.
     */
    unknownPropertyValue = 0.0;
    dragEvent.GetActionCancelEventFunc()();
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);
}

/**
 * @tc.name: DragEventActuatorPropertyTest002
 * @tc.desc: Create DragEventActuator and test its property value.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventActuatorPropertyTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    DragEventActuator dragEventActuator = DragEventActuator(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);

    /**
     * @tc.steps: step2. Create DragEventActuator when fingers number and distance are both greater than the default.
     * @tc.expected: panEventActuator is initialized with the fingers_ and distance_ defined before.
     */
    auto dragEventActuator2 =
        AceType::MakeRefPtr<DragEventActuator>(AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION,
            FINGERS_NUMBER_GREATER_THAN_DEFAULT, DISTANCE_GREATER_THAN_DEFAULT);
    EXPECT_NE(dragEventActuator2, nullptr);
    EXPECT_EQ(dragEventActuator2->fingers_, FINGERS_NUMBER_GREATER_THAN_DEFAULT);
    EXPECT_EQ(dragEventActuator2->distance_, DISTANCE_GREATER_THAN_DEFAULT);

    /**
     * @tc.steps: step3. Get DragEventActuator direction, fingers_ and distance_.
     * @tc.expected:  DragEventActuator's direction, fingers_ and distance_ are equal with the parameters passed in the
     * constructor.
     */
    EXPECT_EQ(dragEventActuator.GetDirection().type, DRAG_DIRECTION.type);
    EXPECT_EQ(dragEventActuator.fingers_, FINGERS_NUMBER);
    EXPECT_EQ(dragEventActuator.distance_, DISTANCE);
    /**
     * @tc.steps: step4. Create DragEvent and set as DragEventActuator's DragEvent and CustomDragEvent.
     * @tc.expected:  Get DragEventActuator's DragEvent and CustomDragEvent which are equal with the DragEvent create
     * before.
     */
    GestureEventFunc actionStart = [](GestureEvent& info) {};
    GestureEventFunc actionUpdate = [](GestureEvent& info) {};
    GestureEventFunc actionEnd = [](GestureEvent& info) {};
    GestureEventNoParameter actionCancel = []() {};

    auto dragEvent = AceType::MakeRefPtr<DragEvent>(
        std::move(actionStart), std::move(actionUpdate), std::move(actionEnd), std::move(actionCancel));
    dragEventActuator.ReplaceDragEvent(dragEvent);
    EXPECT_EQ(dragEvent, dragEventActuator.userCallback_);
    dragEventActuator.SetCustomDragEvent(dragEvent);
    EXPECT_EQ(dragEvent, dragEventActuator.customCallback_);
}

/**
 * @tc.name: DragEventActuatorOnCollectTouchTargetTest003
 * @tc.desc: Create DragEventActuator and invoke OnCollectTouchTarget function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventActuatorOnCollectTouchTargetTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto framenode = FrameNode::CreateFrameNode("test", 1, AceType::MakeRefPtr<Pattern>(), false);
    EXPECT_NE(framenode, nullptr);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(framenode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);

    /**
     * @tc.steps: step2. Invoke OnCollectTouchTarget without setting userCallback_.
     * @tc.expected:  userCallback_ is null and return directly.
     */
    auto getEventTargetImpl = eventHub->CreateGetEventTargetImpl();
    EXPECT_NE(getEventTargetImpl, nullptr);
    TouchTestResult finalResult;
    TouchTestResult responseLinkResult;
    framenode->GetOrCreateFocusHub();
    dragEventActuator->OnCollectTouchTarget(
        COORDINATE_OFFSET, DRAG_TOUCH_RESTRICT, getEventTargetImpl, finalResult, responseLinkResult);
    EXPECT_EQ(dragEventActuator->panRecognizer_->onActionStart_, nullptr);
    EXPECT_EQ(dragEventActuator->panRecognizer_->onActionStart_, nullptr);
    EXPECT_EQ(dragEventActuator->panRecognizer_->onActionUpdate_, nullptr);
    EXPECT_EQ(dragEventActuator->panRecognizer_->onActionEnd_, nullptr);
    EXPECT_EQ(dragEventActuator->panRecognizer_->onActionCancel_, nullptr);
    EXPECT_TRUE(finalResult.empty());

    /**
     * @tc.steps: step3. Create DragEvent and set as DragEventActuator's DragEvent.
     * @tc.expected: DragEventActuator's userCallback_ is not null.
     */
    double unknownPropertyValue = GESTURE_EVENT_PROPERTY_DEFAULT_VALUE;
    GestureEventFunc actionStart = [&unknownPropertyValue](
                                       GestureEvent& info) { unknownPropertyValue = info.GetScale(); };
    GestureEventFunc actionUpdate = [&unknownPropertyValue](
                                        GestureEvent& info) { unknownPropertyValue = info.GetScale(); };
    GestureEventFunc actionEnd = [&unknownPropertyValue](
                                     GestureEvent& info) { unknownPropertyValue = info.GetScale(); };
    GestureEventNoParameter actionCancel = [&unknownPropertyValue]() {
        unknownPropertyValue = GESTURE_EVENT_PROPERTY_VALUE;
    };
    auto dragEvent = AceType::MakeRefPtr<DragEvent>(
        std::move(actionStart), std::move(actionUpdate), std::move(actionEnd), std::move(actionCancel));
    dragEventActuator->ReplaceDragEvent(dragEvent);
    dragEventActuator->SetCustomDragEvent(dragEvent);
    EXPECT_NE(dragEventActuator->userCallback_, nullptr);

    /**
     * @tc.steps: step4. Invoke OnCollectTouchTarget when userCallback_ is not null.
     * @tc.expected: panRecognizer_ action and finalResult will be assigned value.
     */
    dragEventActuator->OnCollectTouchTarget(
        COORDINATE_OFFSET, DRAG_TOUCH_RESTRICT, getEventTargetImpl, finalResult, responseLinkResult);

    EXPECT_NE(dragEventActuator->panRecognizer_->onActionStart_, nullptr);
    EXPECT_NE(dragEventActuator->panRecognizer_->onActionUpdate_, nullptr);
    EXPECT_NE(dragEventActuator->panRecognizer_->onActionEnd_, nullptr);
    EXPECT_NE(dragEventActuator->panRecognizer_->onActionCancel_, nullptr);
    EXPECT_FALSE(finalResult.size() == TOUCH_TEST_RESULT_SIZE);

    /**
     * @tc.steps: step5. Invoke OnCollectTouchTarget when SequencedRecognizer_ is null.
     * @tc.expected: Result size will be increased by one.
     */
    dragEventActuator->SequencedRecognizer_ = nullptr;
    dragEventActuator->OnCollectTouchTarget(
        COORDINATE_OFFSET, DRAG_TOUCH_RESTRICT, getEventTargetImpl, finalResult, responseLinkResult);
    EXPECT_TRUE(finalResult.size() != TOUCH_TEST_RESULT_SIZE_2);

    /**
     * @tc.steps: step6. Invoke onActionStart, onActionUpdate, onActionEnd, onActionCancel when the onActionStart
     * function exists.
     * @tc.expected: The functions have been executed and the unknownPropertyValue has been assigned the correct
     * value.
     */
    GestureEvent info = GestureEvent();
    info.SetScale(GESTURE_EVENT_PROPERTY_VALUE);
    (*(dragEventActuator->panRecognizer_->onActionStart_))(info);
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);
    (*(dragEventActuator->panRecognizer_->onActionUpdate_))(info);
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);
    (*(dragEventActuator->panRecognizer_->onActionEnd_))(info);
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);
    (*(dragEventActuator->panRecognizer_->onActionCancel_))();
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);

    /**
     * @tc.steps: step7. Invoke onActionStart, onActionUpdate, onActionEnd, onActionCancel when the onActionStart
     * function not exist.
     * @tc.expected: The functions have not been executed.
     */
    SystemProperties::debugEnabled_ = true;
    auto dragEventNullptr = AceType::MakeRefPtr<DragEvent>(nullptr, nullptr, nullptr, nullptr);
    dragEventActuator->ReplaceDragEvent(dragEventNullptr);
    dragEventActuator->SetCustomDragEvent(dragEventNullptr);
    unknownPropertyValue = GESTURE_EVENT_PROPERTY_DEFAULT_VALUE;
    (*(dragEventActuator->panRecognizer_->onActionStart_))(info);
    (*(dragEventActuator->panRecognizer_->onActionUpdate_))(info);
    (*(dragEventActuator->panRecognizer_->onActionEnd_))(info);
    (*(dragEventActuator->panRecognizer_->onActionCancel_))();
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_DEFAULT_VALUE);
}

/**
 * @tc.name: DragEventTestNg001
 * @tc.desc: Create DragEventActuator and invoke OnCollectTouchTarget function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventTestNg001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, 0, 50.0f);
    dragEventActuator->StartDragTaskForWeb(GestureEvent());
    auto actionStart = [](GestureEvent&) {};
    auto longPressUpdate = [](GestureEvent&) {};
    dragEventActuator->actionStart_ = actionStart;
    dragEventActuator->StartDragTaskForWeb(GestureEvent());

    dragEventActuator->CancelDragForWeb();
    auto actionCancel = []() {};
    dragEventActuator->actionCancel_ = actionCancel;
    dragEventActuator->CancelDragForWeb();

    dragEventActuator->StartLongPressActionForWeb();
    dragEventActuator->isReceivedLongPress_ = true;
    dragEventActuator->StartLongPressActionForWeb();
    dragEventActuator->isReceivedLongPress_ = true;
    dragEventActuator->longPressUpdate_ = longPressUpdate;
    dragEventActuator->StartLongPressActionForWeb();
    ASSERT_FALSE(dragEventActuator->isReceivedLongPress_);
}

/**
 * @tc.name: DragEventTestNg002
 * @tc.desc: Create DragEventActuator and invoke thumbnail callback.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventTestNg002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create FrameNode and set dragPreview.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    frameNode->SetDraggable(true);
    NG::DragDropInfo dragPreviewInfo;
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(static_cast<void*>(new char[0]));
    dragPreviewInfo.pixelMap = pixelMap;
    frameNode->SetDragPreview(dragPreviewInfo);

    /**
     * @tc.steps: step2. Get eventHub and set onDragStart.
     * @tc.expected: eventHub is not nullptr.
     */
    auto eventHub = frameNode->GetEventHub<EventHub>();
    EXPECT_NE(eventHub, nullptr);
    eventHub->AttachHost(frameNode);
    auto onDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) -> DragDropInfo {
        DragDropInfo info;
        void* voidPtr = static_cast<void*>(new char[0]);
        info.pixelMap = PixelMap::CreatePixelMap(voidPtr);
        return info;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));

    /**
     * @tc.steps: step3. Create gestureEventHub and dragEventActuator.
     */
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    
    /**
     * @tc.steps: step4. Create DragEvent and set as DragEventActuator's DragEvent.
     * @tc.expected: dragEventActuator's userCallback_ is not null.
     */
    TouchTestResult finalResult;
    TouchTestResult responseLinkResult;
    double unknownPropertyValue = GESTURE_EVENT_PROPERTY_DEFAULT_VALUE;
    GestureEventFunc actionStart = [&unknownPropertyValue](GestureEvent& info) {
        unknownPropertyValue = info.GetScale();
    };
    GestureEventFunc actionUpdate = [&unknownPropertyValue](GestureEvent& info) {
        unknownPropertyValue = info.GetScale();
    };
    GestureEventFunc actionEnd = [&unknownPropertyValue](GestureEvent& info) {
        unknownPropertyValue = info.GetScale();
    };
    GestureEventNoParameter actionCancel = [&unknownPropertyValue]() {
        unknownPropertyValue = GESTURE_EVENT_PROPERTY_VALUE;
    };
    auto dragEvent = AceType::MakeRefPtr<DragEvent>(std::move(actionStart), std::move(actionUpdate),
        std::move(actionEnd), std::move(actionCancel));
    dragEventActuator->ReplaceDragEvent(dragEvent);
    dragEventActuator->SetCustomDragEvent(dragEvent);
    EXPECT_NE(dragEventActuator->userCallback_, nullptr);

    /**
     * @tc.steps: step5. Invoke OnCollectTouchTarget when userCallback_ is not null.
     * @tc.expected: longPressRecognizer is not nullptr and longPressRecognizer's callback is not nullptr.
     */
    auto getEventTargetImpl = eventHub->CreateGetEventTargetImpl();
    frameNode->GetOrCreateFocusHub();
    EXPECT_NE(getEventTargetImpl, nullptr);
    dragEventActuator->OnCollectTouchTarget(
        COORDINATE_OFFSET, DRAG_TOUCH_RESTRICT, getEventTargetImpl, finalResult, responseLinkResult);
    EXPECT_NE(dragEventActuator->longPressRecognizer_, nullptr);
    EXPECT_NE(dragEventActuator->longPressRecognizer_->callback_, nullptr);

    /**
     * @tc.steps: step6. Invoke thumbnail callback.
     * @tc.expected: GestureEventHub's pixelMap is not nullptr.
     */
    dragEventActuator->longPressRecognizer_->callback_(Offset(WIDTH, HEIGHT));
    EXPECT_NE(gestureEventHub->pixelMap_, nullptr);
}

/**
 * @tc.name: DragEventTestNg003
 * @tc.desc: Create DragEventActuator and invoke thumbnail callback.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventTestNg003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    SystemProperties::debugEnabled_ = true;
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    DragDropInfo dragDropInfo;
    frameNode->SetDragPreview(dragDropInfo);
    frameNode->SetDraggable(true);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE_EQUAL_DEFAULT);
    /**
     * @tc.steps: step2. Create DragEvent and set as DragEventActuator's DragEvent.
     * @tc.expected: dragEventActuator's userCallback_ is not null.
     */
    GestureEventFunc actionStart = [](GestureEvent& info) {};
    GestureEventFunc actionUpdate = [](GestureEvent& info) {};
    GestureEventFunc actionEnd = [](GestureEvent& info) {};
    GestureEventNoParameter actionCancel = []() {};
    auto dragEvent = AceType::MakeRefPtr<DragEvent>(std::move(actionStart), std::move(actionUpdate),
        std::move(actionEnd), std::move(actionCancel));
    dragEventActuator->ReplaceDragEvent(dragEvent);
    dragEventActuator->SetCustomDragEvent(dragEvent);
    EXPECT_NE(dragEventActuator->userCallback_, nullptr);
    /**
     * @tc.steps: step3. Invoke OnCollectTouchTarget when callback_ is not null.
     * @tc.expected: longPressRecognizer is not nullptr and longPressRecognizer's HasThumbnailCallback() return true.
     */
    TouchTestResult finalResult;
    TouchTestResult responseLinkResult;
    auto getEventTargetImpl = eventHub->CreateGetEventTargetImpl();
    EXPECT_NE(getEventTargetImpl, nullptr);
    EXPECT_EQ(dragEventActuator->longPressRecognizer_->HasThumbnailCallback(), false);
    frameNode->GetOrCreateFocusHub();
    dragEventActuator->OnCollectTouchTarget(
        COORDINATE_OFFSET, DRAG_TOUCH_RESTRICT, getEventTargetImpl, finalResult, responseLinkResult);
    EXPECT_EQ(dragEventActuator->longPressRecognizer_->HasThumbnailCallback(), true);
    dragEventActuator->OnCollectTouchTarget(
        COORDINATE_OFFSET, DRAG_TOUCH_RESTRICT, getEventTargetImpl, finalResult, responseLinkResult);
    EXPECT_EQ(dragEventActuator->longPressRecognizer_->HasThumbnailCallback(), true);
    /**
     * @tc.steps: step4. Invoke thumbnail callback.
     * @tc.expected: cover dragPreviewInfo.customNode == nullptr, dragPreviewInfo.pixelMap == nullptr.
     */
    EXPECT_EQ(frameNode->GetDragPreview().customNode, nullptr);
    dragEventActuator->longPressRecognizer_->callback_(Offset(WIDTH, HEIGHT));
    EXPECT_EQ(gestureEventHub->GetPixelMap(), frameNode->GetRenderContext()->GetThumbnailPixelMap());
    /**
     * @tc.steps: step5. Invoke thumbnail callback.
     * @tc.expected: cover dragPreviewInfo.customNode != nullptr, dragPreviewInfo.pixelMap == nullptr.
     */
    RefPtr<UINode> customNode = AceType::MakeRefPtr<FrameNode>("node", -1, AceType::MakeRefPtr<Pattern>());
    dragDropInfo.customNode = customNode;
    frameNode->SetDragPreview(dragDropInfo);
    dragEventActuator->longPressRecognizer_->callback_(Offset(WIDTH, HEIGHT));
    EXPECT_NE(frameNode->GetDragPreview().customNode, nullptr);
}

/**
 * @tc.name: DragEventTestNg004
 * @tc.desc: Create DragEventActuator and invoke OnCollectTouchTarget.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventTestNg004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    SystemProperties::debugEnabled_ = true;
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    DragDropInfo dragDropInfo;
    frameNode->SetDragPreview(dragDropInfo);
    frameNode->SetDraggable(true);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step2. Create DragEvent and set as DragEventActuator's DragEvent.
     * @tc.expected: dragEventActuator's userCallback_ is not null.
     */
    GestureEventFunc actionStart = [](GestureEvent& info) {};
    GestureEventFunc actionUpdate = [](GestureEvent& info) {};
    GestureEventFunc actionEnd = [](GestureEvent& info) {};
    GestureEventNoParameter actionCancel = []() {};
    auto dragEvent = AceType::MakeRefPtr<DragEvent>(std::move(actionStart), std::move(actionUpdate),
        std::move(actionEnd), std::move(actionCancel));
    dragEventActuator->ReplaceDragEvent(dragEvent);
    dragEventActuator->SetCustomDragEvent(dragEvent);
    EXPECT_NE(dragEventActuator->userCallback_, nullptr);
    /**
     * @tc.steps: step3. Call the function OnCollectTouchTarget when sourceType is SourceType::MOUSE.
     * @tc.expected: cover touchRestrict.sourceType == SourceType::MOUSE.
     */
    auto getEventTargetImpl = eventHub->CreateGetEventTargetImpl();
    EXPECT_NE(getEventTargetImpl, nullptr);
    TouchTestResult finalResult;
    TouchTestResult responseLinkResult;
    frameNode->GetOrCreateFocusHub();
    dragEventActuator->OnCollectTouchTarget(
        COORDINATE_OFFSET, DRAG_TOUCH_RESTRICT_MOUSE, getEventTargetImpl, finalResult, responseLinkResult);
    EXPECT_FALSE(finalResult.empty());
}

/**
 * @tc.name: DragEventTestNg005
 * @tc.desc: Create DragEventActuator and invoke longPressUpdate callback.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventTestNg005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<TextPattern>());
    DragDropInfo dragDropInfo;
    frameNode->SetDragPreview(dragDropInfo);
    frameNode->SetDraggable(true);
    frameNode->GetOrCreateFocusHub();
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step2. Create DragEvent and set as DragEventActuator's DragEvent.
     * @tc.expected: dragEventActuator's userCallback_ is not null.
     */
    GestureEventFunc actionStart = [](GestureEvent& info) {};
    GestureEventFunc actionUpdate = [](GestureEvent& info) {};
    GestureEventFunc actionEnd = [](GestureEvent& info) {};
    GestureEventNoParameter actionCancel = []() {};
    auto dragEvent = AceType::MakeRefPtr<DragEvent>(std::move(actionStart), std::move(actionUpdate),
        std::move(actionEnd), std::move(actionCancel));
    dragEventActuator->ReplaceDragEvent(dragEvent);
    dragEventActuator->SetCustomDragEvent(dragEvent);
    EXPECT_NE(dragEventActuator->userCallback_, nullptr);
    /**
     * @tc.steps: step3. Create callback and set as dragEventActuator's longPressUpdate_.
     * @tc.expected: previewLongPressRecognizer_'s onAction_ is not null.
     */
    auto getEventTargetImpl = eventHub->CreateGetEventTargetImpl();
    TouchTestResult finalResult;
    TouchTestResult responseLinkResult;
    frameNode->GetOrCreateFocusHub();
    dragEventActuator->OnCollectTouchTarget(
        COORDINATE_OFFSET, DRAG_TOUCH_RESTRICT, getEventTargetImpl, finalResult, responseLinkResult);
    EXPECT_NE(dragEventActuator->previewLongPressRecognizer_->onAction_, nullptr);
    /**
     * @tc.steps: step4. Invoke longPressUpdateValue callback.
     * @tc.expected: cover longPressUpdateValue callback.
     */
    SystemProperties::debugEnabled_ = true;
    GestureEvent info = GestureEvent();
    (*(dragEventActuator->longPressRecognizer_->onActionUpdate_))(info);
    EXPECT_EQ(dragEventActuator->GetIsNotInPreviewState(), true);
    SystemProperties::debugEnabled_ = false;
    (*(dragEventActuator->longPressRecognizer_->onActionUpdate_))(info);
    EXPECT_EQ(dragEventActuator->GetIsNotInPreviewState(), true);
    /**
     * @tc.steps: step5. Invoke longPressUpdate callback.
     * @tc.expected: cover longPressUpdate when GetTextDraggable() == false, isAllowedDrag == true.
     */
    SystemProperties::debugEnabled_ = true;
    EXPECT_EQ(gestureEventHub->GetTextDraggable(), false);
    EXPECT_EQ(dragEventActuator->IsAllowedDrag(), true);
    (*(dragEventActuator->previewLongPressRecognizer_->onAction_))(info);
    EXPECT_EQ(dragEventActuator->GetIsNotInPreviewState(), false);
    /**
     * @tc.steps: step6. Invoke longPressUpdate callback.
     * @tc.expected: cover longPressUpdate when GetTextDraggable() == false, isAllowedDrag == false.
     */
    SystemProperties::debugEnabled_ = false;
    frameNode->SetDraggable(false);
    EXPECT_EQ(dragEventActuator->IsAllowedDrag(), false);
    (*(dragEventActuator->previewLongPressRecognizer_->onAction_))(info);
    EXPECT_EQ(dragEventActuator->isReceivedLongPress_, true);
    /**
     * @tc.steps: step7. Invoke longPressUpdate callback.
     * @tc.expected: cover longPressUpdate when GetTextDraggable() == true, GetIsTextDraggable() == false.
     */
    gestureEventHub->SetTextDraggable(true);
    gestureEventHub->SetIsTextDraggable(false);
    dragEventActuator->SetIsNotInPreviewState(true);
    (*(dragEventActuator->previewLongPressRecognizer_->onAction_))(info);
    EXPECT_EQ(dragEventActuator->GetIsNotInPreviewState(), false);
    /**
     * @tc.steps: step8. Invoke longPressUpdate callback.
     * @tc.expected: cover longPressUpdate when GetTextDraggable() == true, GetIsTextDraggable() == true.
     */
    gestureEventHub->SetIsTextDraggable(true);
    dragEventActuator->SetIsNotInPreviewState(true);
    (*(dragEventActuator->previewLongPressRecognizer_->onAction_))(info);
    EXPECT_EQ(dragEventActuator->GetIsNotInPreviewState(), false);
}

/**
 * @tc.name: DragEventTestNg006
 * @tc.desc: Create DragEventActuator and invoke onActionStart callback.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventTestNg006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    SystemProperties::debugEnabled_ = true;
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    frameNode->SetDraggable(true);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step2. Create DragEvent and set as DragEventActuator's DragEvent.
     * @tc.expected: dragEventActuator's userCallback_ is not null.
     */
    double unknownPropertyValue = GESTURE_EVENT_PROPERTY_DEFAULT_VALUE;
    auto actionStart = [&unknownPropertyValue](GestureEvent& info) { unknownPropertyValue = info.GetScale(); };
    GestureEventFunc actionUpdate = [](GestureEvent& info) {};
    GestureEventFunc actionEnd = [](GestureEvent& info) {};
    GestureEventNoParameter actionCancel = []() {};
    auto dragEvent = AceType::MakeRefPtr<DragEvent>(std::move(actionStart), std::move(actionUpdate),
        std::move(actionEnd), std::move(actionCancel));
    dragEventActuator->ReplaceDragEvent(dragEvent);
    dragEventActuator->SetCustomDragEvent(dragEvent);
    EXPECT_NE(dragEventActuator->userCallback_, nullptr);
    /**
     * @tc.steps: step3. Invoke OnCollectTouchTarget when userCallback_ is not null.
     * @tc.expected: longPressRecognizer is not nullptr and panRecognizer's callback onActionStart is not nullptr.
     */
    auto getEventTargetImpl = eventHub->CreateGetEventTargetImpl();
    EXPECT_NE(getEventTargetImpl, nullptr);
    TouchTestResult finalResult;
    TouchTestResult responseLinkResult;
    frameNode->GetOrCreateFocusHub();
    dragEventActuator->OnCollectTouchTarget(
        COORDINATE_OFFSET, DRAG_TOUCH_RESTRICT, getEventTargetImpl, finalResult, responseLinkResult);
    EXPECT_NE(dragEventActuator->panRecognizer_->onActionStart_, nullptr);
    /**
     * @tc.steps: step4. Invoke onActionStart callback, when info.GetSourceDevice() is SourceType::MOUSE.
     * @tc.expected: cover pattern->IsSelected() == false or GetMouseStatus() == MouseStatus::MOVE
     */
    GestureEvent info = GestureEvent();
    info.SetScale(GESTURE_EVENT_PROPERTY_VALUE);
    info.SetSourceDevice(SourceType::MOUSE);
    gestureEventHub->SetTextDraggable(true);
    (*(dragEventActuator->panRecognizer_->onActionStart_))(info);
    EXPECT_EQ(gestureEventHub->GetIsTextDraggable(), false);

    auto pattern = frameNode->GetPattern<TextPattern>();
    pattern->mouseStatus_ = MouseStatus::MOVE;
    (*(dragEventActuator->panRecognizer_->onActionStart_))(info);
    EXPECT_EQ(gestureEventHub->GetIsTextDraggable(), false);
    EXPECT_EQ(pattern->GetMouseStatus(), MouseStatus::MOVE);
    /**
     * @tc.steps: step5. Invoke onActionStart callback, when info.GetSourceDevice() is SourceType::MOUSE.
     * @tc.expected: cover GetTextDraggable() is false, Trigger drag start event set by user.
     */
    gestureEventHub->SetTextDraggable(false);
    unknownPropertyValue = GESTURE_EVENT_PROPERTY_DEFAULT_VALUE;
    (*(dragEventActuator->panRecognizer_->onActionStart_))(info);
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);
    /**
     * @tc.steps: step6. Invoke onActionStart callback, when info.GetSourceDevice() is not SourceType::MOUSE.
     * @tc.expected: cover gestureHub->GetTextDraggable() is true
     */
    info.SetSourceDevice(SourceType::TOUCH);
    gestureEventHub->SetTextDraggable(true);
    EXPECT_EQ(gestureEventHub->GetTextDraggable(), true);
    (*(dragEventActuator->panRecognizer_->onActionStart_))(info);
    EXPECT_EQ(gestureEventHub->GetIsTextDraggable(), false);
}

/**
 * @tc.name: DragEventTestNg007
 * @tc.desc: Create DragEventActuator and invoke onActionCancel callback.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventTestNg007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<TextPattern>());
    frameNode->SetDraggable(true);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step2. Create DragEvent and set as DragEventActuator's DragEvent.
     * @tc.expected: dragEventActuator's userCallback_ is not null.
     */
    double unknownPropertyValue = GESTURE_EVENT_PROPERTY_DEFAULT_VALUE;
    GestureEventFunc actionStart = [](GestureEvent& info) {};
    GestureEventFunc actionUpdate = [](GestureEvent& info) {};
    GestureEventFunc actionEnd = [](GestureEvent& info) {};
    GestureEventNoParameter actionCancel = [&unknownPropertyValue]() {
        unknownPropertyValue = GESTURE_EVENT_PROPERTY_VALUE;
    };
    auto dragEvent = AceType::MakeRefPtr<DragEvent>(std::move(actionStart), std::move(actionUpdate),
        std::move(actionEnd), std::move(actionCancel));
    dragEventActuator->ReplaceDragEvent(dragEvent);
    dragEventActuator->SetCustomDragEvent(dragEvent);
    EXPECT_NE(dragEventActuator->userCallback_, nullptr);
    /**
     * @tc.steps: step3. Invoke OnCollectTouchTarget when userCallback_ is not null.
     * @tc.expected: longPressRecognizer is not nullptr and panRecognizer's callback onActionCancel is not nullptr.
     */
    auto getEventTargetImpl = eventHub->CreateGetEventTargetImpl();
    TouchTestResult finalResult;
    TouchTestResult responseLinkResult;
    frameNode->GetOrCreateFocusHub();
    dragEventActuator->OnCollectTouchTarget(
        COORDINATE_OFFSET, DRAG_TOUCH_RESTRICT, getEventTargetImpl, finalResult, responseLinkResult);
    EXPECT_NE(dragEventActuator->panRecognizer_->onActionCancel_, nullptr);
    /**
     * @tc.steps: step4. Invoke onActionCancel callback, when gestureHub->GetTextDraggable() is true.
     * @tc.expected: cover gestureHub->GetIsTextDraggable() is false.
     */
    SystemProperties::debugEnabled_ = true;
    GestureEvent info = GestureEvent();
    info.SetScale(GESTURE_EVENT_PROPERTY_VALUE);
    gestureEventHub->SetTextDraggable(true);
    unknownPropertyValue = GESTURE_EVENT_PROPERTY_DEFAULT_VALUE;
    (*(dragEventActuator->panRecognizer_->onActionCancel_))();
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);
    /**
     * @tc.steps: step5. Invoke onActionCancel callback, when gestureHub->GetTextDraggable() is true.
     * @tc.expected: cover gestureHub->GetIsTextDraggable() is true.
     */
    SystemProperties::debugEnabled_ = false;
    gestureEventHub->SetIsTextDraggable(true);
    unknownPropertyValue = GESTURE_EVENT_PROPERTY_DEFAULT_VALUE;
    (*(dragEventActuator->panRecognizer_->onActionCancel_))();
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);
    /**
     * @tc.steps: step6. Invoke onActionCancel callback, GetIsBindOverlayValue is true.
     * @tc.expected: cover getDeviceType() != SourceType::MOUSE.
     */
    eventHub->AttachHost(nullptr);
    EXPECT_EQ(dragEventActuator->GetIsBindOverlayValue(dragEventActuator), true);
    unknownPropertyValue = GESTURE_EVENT_PROPERTY_DEFAULT_VALUE;
    (*(dragEventActuator->panRecognizer_->onActionCancel_))();
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_DEFAULT_VALUE);
    /**
     * @tc.steps: step7. Invoke onActionCancel callback, GetIsBindOverlayValue is true.
     * @tc.expected: cover getDeviceType() == SourceType::MOUSE.
     */
    dragEventActuator->panRecognizer_->deviceType_ = SourceType::MOUSE;
    unknownPropertyValue = GESTURE_EVENT_PROPERTY_DEFAULT_VALUE;
    (*(dragEventActuator->panRecognizer_->onActionCancel_))();
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_DEFAULT_VALUE);

    gestureEventHub->SetTextDraggable(false);
    unknownPropertyValue = GESTURE_EVENT_PROPERTY_DEFAULT_VALUE;
    (*(dragEventActuator->panRecognizer_->onActionCancel_))();
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_DEFAULT_VALUE);
}

/**
 * @tc.name: DragEventTestNg008
 * @tc.desc: Create DragEventActuator and invoke HideTextAnimation function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventTestNg008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    NG::DragDropInfo dragPreviewInfo;
    void* voidPtr = static_cast<void*>(new char[0]);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    dragPreviewInfo.pixelMap = pixelMap;
    frameNode->SetDragPreview(dragPreviewInfo);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step2. Invoke HideTextAnimation function, when isAllowedDrag is false.
     * @tc.expected: cover branch gestureHub->GetIsTextDraggable().
     */
    SystemProperties::debugEnabled_ = true;
    frameNode->SetDraggable(false);
    gestureEventHub->SetTextDraggable(false);
    dragEventActuator->HideTextAnimation();
    EXPECT_EQ(gestureEventHub->GetTextDraggable(), false);
    gestureEventHub->SetTextDraggable(true);
    dragEventActuator->HideTextAnimation();
    EXPECT_EQ(gestureEventHub->GetTextDraggable(), true);
    /**
     * @tc.steps: step3. Invoke HideTextAnimation function, when isAllowedDrag is true.
     * @tc.expected: cover branch gestureHub->GetIsTextDraggable().
     */
    SystemProperties::debugEnabled_ = false;
    frameNode->SetDraggable(true);
    gestureEventHub->SetTextDraggable(false);
    dragEventActuator->HideTextAnimation();
    EXPECT_EQ(gestureEventHub->GetTextDraggable(), false);
    gestureEventHub->SetTextDraggable(true);
    dragEventActuator->HideTextAnimation();
    EXPECT_EQ(gestureEventHub->GetTextDraggable(), true);
}

/**
 * @tc.name: DragEventTestNg009
 * @tc.desc: Create DragEventActuator and invoke MountPixelMap function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventTestNg009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step2. CreatePixelMap and Invoke CreatePreviewNode function.
     * @tc.expected: GetClickEffectLevelValue is correct.
     */
    frameNode->eventHub_->SetEnabled(true);
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    auto mockRenderContext = AceType::MakeRefPtr<MockRenderContext>();
    ASSERT_NE(mockRenderContext, nullptr);
    frameNode->renderContext_ = mockRenderContext;
    void* voidPtr = static_cast<void*>(new char[0]);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    gestureHub->SetPixelMap(pixelMap);
    EXPECT_NE(frameNode->GetPixelMap(), nullptr);
    RefPtr<FrameNode> imageNode = AceType::DynamicCast<FrameNode>(frameNode->GetFirstChild());
    dragEventActuator->CreatePreviewNode(frameNode, imageNode);
    auto imageContext = imageNode->GetRenderContext();
    auto clickEffectInfo = imageContext->GetClickEffectLevelValue();
    EXPECT_EQ(clickEffectInfo.level, ClickEffectLevel::LIGHT);
    /**
     * @tc.steps: step3. Invoke MountPixelMap function.
     * @tc.expected: MountPixelMapToRootNode success, overlayManager's hasPixelMap is true.
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    auto overlayManager = pipeline->GetOverlayManager();
    EXPECT_NE(overlayManager, nullptr);
    dragEventActuator->MountPixelMap(overlayManager, gestureHub, imageNode, nullptr);
    EXPECT_EQ(overlayManager->hasPixelMap_, true);
    /**
     * @tc.steps: step4. Invoke SetPreviewDefaultAnimateProperty function.
     * @tc.expected: cover branch IsPreviewNeedScale() == true.
     */
    imageNode->GetGeometryNode()->frame_.rect_.width_ = IMAGE_INVALID_RECT_WIDTH;
    EXPECT_EQ(imageNode->IsPreviewNeedScale(), true);
    dragEventActuator->SetPreviewDefaultAnimateProperty(imageNode);
    TranslateOptions result = imageContext->GetTransformTranslate().value();
    TranslateOptions expectValue { 0.0f, 0.0f, 0.0f };
    EXPECT_EQ(result.x.calcvalue_, expectValue.x.calcvalue_);
}

/**
 * @tc.name: DragEventTestNg010
 * @tc.desc: Invoke GetPreviewPixelMap.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventTestNg010, TestSize.Level1)
{
    EXPECT_EQ(DragEventActuator::GetPreviewPixelMap(NO_COMPONENT_ID, nullptr), nullptr);
    EXPECT_EQ(DragEventActuator::GetPreviewPixelMap(COMPONENT_ID, nullptr), nullptr);

    auto frameNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    
    EXPECT_EQ(DragEventActuator::GetPreviewPixelMap(NO_COMPONENT_ID, frameNode), nullptr);
    EXPECT_EQ(DragEventActuator::GetPreviewPixelMap(COMPONENT_ID, frameNode), nullptr);
}

/**
 * @tc.name: DragEventExecutePreDragActionTest001
 * @tc.desc: Create DragEventActuator and test ExecutePreDragAction function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventExecutePreDragActionTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    frameNode->eventHub_ = eventHub;
    frameNode->SetDraggable(true);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step2. Create onPreDrag function and bind to eventHub.
     * @tc.expected: Bind onPreDrag function successful.
     */
    MockFunction<void(const PreDragStatus&)> mockOnPreFunction;
    EXPECT_CALL(mockOnPreFunction, Call(PreDragStatus::ACTION_DETECTING_STATUS)).WillOnce(Return());
    EXPECT_CALL(mockOnPreFunction, Call(PreDragStatus::READY_TO_TRIGGER_DRAG_ACTION)).WillOnce(Return());
    EXPECT_CALL(mockOnPreFunction, Call(PreDragStatus::PREVIEW_LIFT_STARTED)).WillOnce(Return());
    EXPECT_CALL(mockOnPreFunction, Call(PreDragStatus::PREVIEW_LANDING_STARTED)).WillOnce(Return());
    EXPECT_CALL(mockOnPreFunction, Call(PreDragStatus::ACTION_CANCELED_BEFORE_DRAG)).WillOnce(Return());
    std::function<void(const PreDragStatus&)> mockOnPreDragFunc = mockOnPreFunction.AsStdFunction();

    auto onDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) -> DragDropInfo {
        DragDropInfo info;
        return info;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));
    eventHub->SetOnPreDrag(mockOnPreDragFunc);
    EXPECT_NE(eventHub->GetOnPreDrag(), nullptr);

    /**
     * @tc.steps: step3. Call ExecutePreDragAction Function.
     * @tc.expected: Call function successful.
     */
    auto pipeline = PipelineContext::GetMainPipelineContext();
    auto dragDropManager = pipeline->GetDragDropManager();
    ASSERT_NE(dragDropManager, nullptr);
    EXPECT_EQ(dragDropManager->GetPreDragStatus(), PreDragStatus::ACTION_DETECTING_STATUS);
    DragEventActuator::ExecutePreDragAction(PreDragStatus::ACTION_DETECTING_STATUS, frameNode);
    EXPECT_EQ(dragDropManager->GetPreDragStatus(), PreDragStatus::READY_TO_TRIGGER_DRAG_ACTION);
    DragEventActuator::ExecutePreDragAction(PreDragStatus::READY_TO_TRIGGER_DRAG_ACTION, frameNode);
    EXPECT_EQ(dragDropManager->GetPreDragStatus(), PreDragStatus::PREVIEW_LIFT_STARTED);
    DragEventActuator::ExecutePreDragAction(PreDragStatus::PREVIEW_LIFT_STARTED, frameNode);
    EXPECT_EQ(dragDropManager->GetPreDragStatus(), PreDragStatus::PREVIEW_LIFT_FINISHED);
    DragEventActuator::ExecutePreDragAction(PreDragStatus::PREVIEW_LIFT_FINISHED, frameNode);
    EXPECT_EQ(dragDropManager->GetPreDragStatus(), PreDragStatus::PREVIEW_LANDING_STARTED);
    DragEventActuator::ExecutePreDragAction(PreDragStatus::PREVIEW_LANDING_STARTED, frameNode);
    EXPECT_EQ(dragDropManager->GetPreDragStatus(), PreDragStatus::PREVIEW_LANDING_FINISHED);
    DragEventActuator::ExecutePreDragAction(PreDragStatus::PREVIEW_LANDING_FINISHED, frameNode);
    EXPECT_EQ(dragDropManager->GetPreDragStatus(), PreDragStatus::ACTION_CANCELED_BEFORE_DRAG);
    DragEventActuator::ExecutePreDragAction(PreDragStatus::ACTION_CANCELED_BEFORE_DRAG, frameNode);
    EXPECT_EQ(dragDropManager->GetPreDragStatus(), PreDragStatus::ACTION_CANCELED_BEFORE_DRAG);
}

/**
 * @tc.name: DragEventExecutePreDragActionTest002
 * @tc.desc: Create DragEventActuator and test ExecutePreDragAction function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventExecutePreDragActionTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    frameNode->eventHub_ = eventHub;
    frameNode->SetDraggable(true);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    auto pipeline = PipelineContext::GetMainPipelineContext();
    auto dragDropManager = pipeline->GetDragDropManager();
    ASSERT_NE(dragDropManager, nullptr);
    dragDropManager->SetPrepareDragFrameNode(frameNode);
    /**
     * @tc.steps: step2. Create onPreDrag function and bind to eventHub.
     * @tc.expected: Bind onPreDrag function successful.
     */
    MockFunction<void(const PreDragStatus&)> mockOnPreFunction;
    EXPECT_CALL(mockOnPreFunction, Call(PreDragStatus::ACTION_DETECTING_STATUS)).WillOnce(Return());
    EXPECT_CALL(mockOnPreFunction, Call(PreDragStatus::ACTION_CANCELED_BEFORE_DRAG)).WillOnce(Return());
    std::function<void(const PreDragStatus&)> mockOnPreDragFunc = mockOnPreFunction.AsStdFunction();

    auto onDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) -> DragDropInfo {
        DragDropInfo info;
        return info;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));
    eventHub->SetOnPreDrag(mockOnPreDragFunc);
    EXPECT_NE(eventHub->GetOnPreDrag(), nullptr);

    /**
     * @tc.steps: step3. Call ExecutePreDragAction Function with same status.
     * @tc.expected: first call function successful, second call canceled.
     */
    DragEventActuator::ExecutePreDragAction(PreDragStatus::ACTION_DETECTING_STATUS);
    DragEventActuator::ExecutePreDragAction(PreDragStatus::ACTION_DETECTING_STATUS);

    /**
     * @tc.steps: step4. Call ExecutePreDragAction Function with fail parameters.
     * @tc.expected: not call any function.
     */
    gestureEventHub->SetTextDraggable(true);
    frameNode->SetDraggable(false);
    DragEventActuator::ExecutePreDragAction(PreDragStatus::ACTION_DETECTING_STATUS);
    dragDropManager->ResetDragging(DragDropMgrState::DRAGGING);
    DragEventActuator::ExecutePreDragAction(PreDragStatus::ACTION_DETECTING_STATUS);
}

/**
 * @tc.name: DragEventShowBadgeTest01
 * @tc.desc: Create DragEventActuator and invoke MountPixelMap function when multi object drag.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventShowBadgeTest01, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);

    /**
     * @tc.steps: step2. CreatePixelMap and Invoke CreatePreviewNode and CreateBadgeTextNode function.
     * @tc.expected:  imageNode and textNode is not nullptr, badge size is correct.
     */
    frameNode->eventHub_->SetEnabled(true);
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    void* voidPtr = static_cast<void*>(new char[0]);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    gestureHub->SetPixelMap(pixelMap);
    EXPECT_NE(frameNode->GetPixelMap(), nullptr);
    RefPtr<FrameNode> imageNode = nullptr;
    dragEventActuator->CreatePreviewNode(frameNode, imageNode);
    EXPECT_NE(imageNode, nullptr);
    const int32_t childSize = 3; // selected item count.
    auto textNode = dragEventActuator->CreateBadgeTextNode(frameNode, childSize, DEFALUT_DRAG_PPIXELMAP_SCALE, false);
    EXPECT_NE(textNode, nullptr);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    EXPECT_NE(textLayoutProperty, nullptr);
    auto content = textLayoutProperty->GetContent();
    EXPECT_EQ(content, std::to_string(childSize));

    /**
     * @tc.steps: step3. Invoke MountPixelMap function.
     * @tc.expected: MountPixelMapToRootNode success, overlayManager's hasPixelMap and badgeNode.
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    auto overlayManager = pipeline->GetOverlayManager();
    EXPECT_NE(overlayManager, nullptr);
    dragEventActuator->MountPixelMap(overlayManager, gestureHub, imageNode, textNode);
    EXPECT_EQ(overlayManager->hasPixelMap_, true);
    textNode = overlayManager->GetPixelMapBadgeNode();
    EXPECT_NE(textNode, nullptr);
}

/**
 * @tc.name: DragEventShowBadgeTest02
 * @tc.desc: Create DragEventActuator and invoke ShowPreviewBadgeAnimation function when multi object drag.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventShowBadgeTest02, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);

    /**
     * @tc.steps: step2. Create GatherNodeChildInfo.
     */
    auto imageNode1 = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    auto imageNode2 = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    GatherNodeChildInfo gatherNodeChildInfo1 = {imageNode1, {0, 0}, 100, 100};
    GatherNodeChildInfo gatherNodeChildInfo2 = {imageNode2, {100, 100}, 100, 100};
    auto pipeline = PipelineContext::GetCurrentContext();
    auto overlayManager = pipeline->GetOverlayManager();
    EXPECT_NE(overlayManager, nullptr);
    overlayManager->gatherNodeChildrenInfo_.push_back(gatherNodeChildInfo1);
    overlayManager->gatherNodeChildrenInfo_.push_back(gatherNodeChildInfo2);
    EXPECT_EQ(overlayManager->gatherNodeChildrenInfo_.size(), 2); //gatherNodeChildInfo size.

    /**
     * @tc.steps: step3. CreatePixelMap and Invoke CreatePreviewNode function.
     * @tc.expected:  imageNode is not nullptr.
     */
    frameNode->eventHub_->SetEnabled(true);
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    void* voidPtr = static_cast<void*>(new char[0]);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    gestureHub->SetPixelMap(pixelMap);
    EXPECT_NE(frameNode->GetPixelMap(), nullptr);
    RefPtr<FrameNode> imageNode = nullptr;
    dragEventActuator->CreatePreviewNode(frameNode, imageNode);
    EXPECT_NE(imageNode, nullptr);

    /**
     * @tc.steps: step4. Invoke MountPixelMap and ShowPreviewBadgeAnimation function.
     * @tc.expected: MountPixelMapToRootNode success, overlayManager has badgeNode, badge size is correct.
     */
    dragEventActuator->MountPixelMap(overlayManager, gestureHub, imageNode, nullptr);
    dragEventActuator->ShowPreviewBadgeAnimation(dragEventActuator, overlayManager);
    auto textNode = overlayManager->GetPixelMapBadgeNode();
    EXPECT_NE(textNode, nullptr);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    EXPECT_NE(textLayoutProperty, nullptr);
    auto content = textLayoutProperty->GetContent();
    EXPECT_EQ(content, std::to_string(overlayManager->gatherNodeChildrenInfo_.size() + 1));

    /**
     * @tc.steps: step5. Get dragDropManager and invoke IsShowBadgeAnimation function.
     * @tc.expected: IsShowBadgeAnimation_ is false.
     */
    auto pipelineContext = PipelineContext::GetCurrentContext();
    auto dragDropManager = pipelineContext->GetDragDropManager();
    EXPECT_NE(dragDropManager, nullptr);
    EXPECT_EQ(dragDropManager->IsShowBadgeAnimation(), false);
}

/**
 * @tc.name: DragEventShowBadgeTest03
 * @tc.desc: Test the GetCustomerBadgeNumber function of setting different NumberBadge.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventShowBadgeTest03, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create frameNode.
     */
    auto frameNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    EXPECT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. Do not set NumberBadge.
     * @tc.expected: badgeNumber has no value.
     */
    auto dragPreviewOptions = frameNode->GetDragPreviewOption();
    auto badgeNumber = dragPreviewOptions.GetCustomerBadgeNumber();
    EXPECT_EQ(badgeNumber.has_value(), false);

    /**
     * @tc.steps: step3. Set NumberBadge value is true.
     * @tc.expected: badgeNumber has no value.
     */
    NG::DragPreviewOption previewOptions;
    previewOptions.isNumber = false;
    previewOptions.badgeNumber = true;
    frameNode->SetDragPreviewOptions(previewOptions);
    dragPreviewOptions = frameNode->GetDragPreviewOption();
    badgeNumber = dragPreviewOptions.GetCustomerBadgeNumber();
    EXPECT_EQ(badgeNumber.has_value(), false);

    /**
     * @tc.steps: step4. Set NumberBadge value is false.
     * @tc.expected: badgeNumber has value.
     */
    previewOptions.isNumber = false;
    previewOptions.badgeNumber = false;
    frameNode->SetDragPreviewOptions(previewOptions);
    dragPreviewOptions = frameNode->GetDragPreviewOption();
    badgeNumber = dragPreviewOptions.GetCustomerBadgeNumber();
    EXPECT_EQ(badgeNumber.has_value(), true);
    EXPECT_EQ(badgeNumber.value(), 1);

    /**
     * @tc.steps: step5. Set the NumberBadge to a special value 3.
     * @tc.expected: badgeNumber is the set value.
     */
    previewOptions.isNumber = true;
    previewOptions.badgeNumber = NUMBER_BADGE_SIZE_3;
    frameNode->SetDragPreviewOptions(previewOptions);
    dragPreviewOptions = frameNode->GetDragPreviewOption();
    badgeNumber = dragPreviewOptions.GetCustomerBadgeNumber();
    EXPECT_EQ(badgeNumber.has_value(), true);
    EXPECT_EQ(badgeNumber.value(), NUMBER_BADGE_SIZE_3);

    /**
     * @tc.steps: step6. Set the NumberBadge to a special value 0.
     * @tc.expected: badgeNumber is 1.
     */
    previewOptions.isNumber = true;
    previewOptions.badgeNumber = 0;
    frameNode->SetDragPreviewOptions(previewOptions);
    dragPreviewOptions = frameNode->GetDragPreviewOption();
    badgeNumber = dragPreviewOptions.GetCustomerBadgeNumber();
    EXPECT_EQ(badgeNumber.has_value(), true);
    EXPECT_EQ(badgeNumber.value(), 1);

    /**
     * @tc.steps: step7. Set the NumberBadge to a special value -1.
     * @tc.expected: badgeNumber is 1.
     */
    previewOptions.isNumber = true;
    previewOptions.badgeNumber = -1;
    frameNode->SetDragPreviewOptions(previewOptions);
    dragPreviewOptions = frameNode->GetDragPreviewOption();
    badgeNumber = dragPreviewOptions.GetCustomerBadgeNumber();
    EXPECT_EQ(badgeNumber.has_value(), true);
    EXPECT_EQ(badgeNumber.value(), 1);

    /**
     * @tc.steps: step8. Set the NumberBadge to a special value 100.
     * @tc.expected: badgeNumber is the set value.
     */
    previewOptions.isNumber = true;
    previewOptions.badgeNumber = NUMBER_BADGE_SIZE_100;
    frameNode->SetDragPreviewOptions(previewOptions);
    dragPreviewOptions = frameNode->GetDragPreviewOption();
    badgeNumber = dragPreviewOptions.GetCustomerBadgeNumber();
    EXPECT_EQ(badgeNumber.has_value(), true);
    EXPECT_EQ(badgeNumber.value(), NUMBER_BADGE_SIZE_100);
}

/**
 * @tc.name: TestCreateGatherNode001
 * @tc.desc: Create List GatherNode
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestCreateGatherNode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List Node.
     */
    auto listNode = FrameNode::CreateFrameNode(
        V2::LIST_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ListPattern>());
    ASSERT_NE(listNode, nullptr);
    /**
     * @tc.steps: step2. Create List Item Node.
     */
    auto listItemNode1 = FrameNode::CreateFrameNode(V2::LIST_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<ListItemPattern>(nullptr, V2::ListItemStyle::NONE));
    ASSERT_NE(listItemNode1, nullptr);
    auto listItemNode2 = FrameNode::CreateFrameNode(V2::LIST_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<ListItemPattern>(nullptr, V2::ListItemStyle::NONE));
    ASSERT_NE(listItemNode2, nullptr);
    auto itemPattern1 = listItemNode1->GetPattern<ListItemPattern>();
    ASSERT_NE(itemPattern1, nullptr);
    itemPattern1->SetSelected(true);
    auto itemPattern2 = listItemNode2->GetPattern<ListItemPattern>();
    ASSERT_NE(itemPattern2, nullptr);
    itemPattern2->SetSelected(true);
    NG::DragPreviewOption option { true, false, true };
    listItemNode1->SetDragPreviewOptions(option);
    listNode->AddChild(listItemNode1);
    listNode->AddChild(listItemNode2);
    /**
     * @tc.steps: step3. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto onDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) -> DragDropInfo {
        DragDropInfo info;
        return info;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(listItemNode1));
    listItemNode1->eventHub_ = eventHub;
    listItemNode1->SetDraggable(true);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step4. Create GatherNode.
     */
    dragEventActuator->FindItemParentNode(listItemNode1);
    auto gatherNode = DragEventActuator::CreateGatherNode(dragEventActuator);
    EXPECT_EQ(gatherNode, nullptr);
}

/**
 * @tc.name: TestCreateGatherNode002
 * @tc.desc: Create Grid GatherNode
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestCreateGatherNode002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Grid Node.
     */
    auto gridNode = FrameNode::CreateFrameNode(
        V2::GRID_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<GridPattern>());
    ASSERT_NE(gridNode, nullptr);
    /**
     * @tc.steps: step2. Create Grid Item Node.
     */
    auto gridItemNode1 = FrameNode::CreateFrameNode(
        V2::GRID_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<GridItemPattern>(nullptr, GridItemStyle::NONE));
    ASSERT_NE(gridItemNode1, nullptr);
    auto gridItemNode2 = FrameNode::CreateFrameNode(
        V2::GRID_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<GridItemPattern>(nullptr, GridItemStyle::NONE));
    ASSERT_NE(gridItemNode2, nullptr);
    auto itemPattern1 = gridItemNode1->GetPattern<GridItemPattern>();
    ASSERT_NE(itemPattern1, nullptr);
    itemPattern1->SetSelected(true);
    auto itemPattern2 = gridItemNode2->GetPattern<GridItemPattern>();
    ASSERT_NE(itemPattern2, nullptr);
    itemPattern2->SetSelected(true);
    NG::DragPreviewOption option { true, false, true };
    gridItemNode1->SetDragPreviewOptions(option);
    gridNode->AddChild(gridItemNode1);
    gridNode->AddChild(gridItemNode2);
    /**
     * @tc.steps: step3. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(gridItemNode1));
    auto onDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) -> DragDropInfo {
        DragDropInfo info;
        return info;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));
    gridItemNode1->eventHub_ = eventHub;
    gridItemNode1->SetDraggable(true);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step4. Create GatherNode.
     */
    dragEventActuator->FindItemParentNode(gridItemNode1);
    auto gatherNode = DragEventActuator::CreateGatherNode(dragEventActuator);
    EXPECT_EQ(gatherNode, nullptr);
}

/**
 * @tc.name: TestCreateImageNode001
 * @tc.desc: Create ImageNode of FrameNode
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestCreateImageNode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create FrameNode.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    /**
     * @tc.steps: step2. Create GatherNodeChildInfo.
     */
    NG::GatherNodeChildInfo gatherNodeChildInfo;
    /**
     * @tc.steps: step3. Create ImageNode.
     */
    auto imageNode = DragEventActuator::CreateImageNode(frameNode, gatherNodeChildInfo);
    EXPECT_NE(imageNode, nullptr);
}

/**
 * @tc.name: TestResetNode001
 * @tc.desc: Reset Node scale
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestResetNode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create FrameNode.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    NG::DragPreviewOption option { true, true, false };
    frameNode->SetDragPreviewOptions(option);
    /**
     * @tc.steps: step2. Set Scale
     */
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    renderContext->UpdateTransformScale({0.9f, 0.9f});
    auto scale = renderContext->GetTransformScaleValue({ 1.0f, 1.0f });
    EXPECT_EQ(scale.x, 0.9f);
    EXPECT_EQ(scale.y, 0.9f);
    /**
     * @tc.steps: step3. Reset frameNode scale.
     */
    DragEventActuator::ResetNode(frameNode);
    auto resetScale = renderContext->GetTransformScaleValue({ 0.0f, 0.0f });
    EXPECT_EQ(resetScale.x, 1.0f);
    EXPECT_EQ(resetScale.y, 1.0f);
}

/**
 * @tc.name: TestGetFrameNodePreviewPixelMap001
 * @tc.desc: Create frameNode and get DragPreviewInfo.PixelMap.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestGetFrameNodePreviewPixelMap002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create FrameNode.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    /**
     * @tc.steps: step2. Set DragPreviewInfo.
     */
    NG::DragDropInfo dragPreviewInfo;
    void* voidPtr = static_cast<void*>(new char[0]);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    ASSERT_NE(pixelMap, nullptr);
    dragPreviewInfo.pixelMap = pixelMap;
    frameNode->SetDragPreview(dragPreviewInfo);
    /**
     * @tc.steps: step3. Get PixelMap.
     */
    DragEventActuator::GetFrameNodePreviewPixelMap(frameNode);
    /**
     * @tc.steps: step4. Get GestureEventHub.
     */
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    /**
     * @tc.steps: step5. Get DragPreviewPixelMap.
     */
    auto dragPreviewPixelMap = gestureHub->GetDragPreviewPixelMap();
    EXPECT_EQ(dragPreviewPixelMap, pixelMap);
}

/**
 * @tc.name: TestIsBelongToMultiItemNode001
 * @tc.desc: Test IsBelongToMultiItemNode
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestIsBelongToMultiItemNode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create ListItemNode.
     */
    auto listItemNode = FrameNode::CreateFrameNode(
        V2::LIST_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<ListItemPattern>(nullptr, V2::ListItemStyle::NONE));
    ASSERT_NE(listItemNode, nullptr);
    auto itemPattern = listItemNode->GetPattern<ListItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    itemPattern->SetSelected(true);
    NG::DragPreviewOption option { true, false, true };
    listItemNode->SetDragPreviewOptions(option);
    /**
     * @tc.steps: step2. Create frameNode.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    listItemNode->AddChild(frameNode);
    /**
     * @tc.steps: step3. Create frameNode's DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto onDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) -> DragDropInfo {
        DragDropInfo info;
        return info;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(listItemNode));
    listItemNode->eventHub_ = eventHub;
    listItemNode->SetDraggable(true);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureEventHub, nullptr);
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(dragEventActuator, nullptr);
    /**
     * @tc.steps: step4. Run IsBelongToMultiItemNode.
     */
    auto isBelongToMultiItemNode = dragEventActuator->IsBelongToMultiItemNode(frameNode);
    EXPECT_EQ(isBelongToMultiItemNode, true);
}

/**
 * @tc.name: TestIsBelongToMultiItemNode002
 * @tc.desc: Test IsBelongToMultiItemNode
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestIsBelongToMultiItemNode002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GridItemNode.
     */
    auto gridItemNode = FrameNode::CreateFrameNode(
        V2::GRID_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<GridItemPattern>(nullptr, GridItemStyle::NONE));
    ASSERT_NE(gridItemNode, nullptr);
    auto itemPattern = gridItemNode->GetPattern<GridItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    itemPattern->SetSelected(true);
    NG::DragPreviewOption option { true, false, true };
    gridItemNode->SetDragPreviewOptions(option);
    /**
     * @tc.steps: step2. Create frameNode.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    gridItemNode->AddChild(frameNode);
    /**
     * @tc.steps: step3. Create frameNode's DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto onDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) -> DragDropInfo {
        DragDropInfo info;
        return info;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(gridItemNode));
    gridItemNode->eventHub_ = eventHub;
    gridItemNode->SetDraggable(true);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureEventHub, nullptr);
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(dragEventActuator, nullptr);
    /**
     * @tc.steps: step4. Run IsBelongToMultiItemNode.
     */
    auto isBelongToMultiItemNode = dragEventActuator->IsBelongToMultiItemNode(frameNode);
    EXPECT_EQ(isBelongToMultiItemNode, true);
}

/**
 * @tc.name: TestIsSelectedItemNode001
 * @tc.desc: Test IsSelectedItemNode
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestIsSelectedItemNode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GridItemNode.
     */
    auto gridItemNode = FrameNode::CreateFrameNode(
        V2::GRID_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<GridItemPattern>(nullptr, GridItemStyle::NONE));
    ASSERT_NE(gridItemNode, nullptr);
    auto itemPattern = gridItemNode->GetPattern<GridItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    itemPattern->SetSelected(true);
    NG::DragPreviewOption option { true, false, true };
    gridItemNode->SetDragPreviewOptions(option);
    /**
     * @tc.steps: step2. Create GridItemNode's DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto onDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) -> DragDropInfo {
        DragDropInfo info;
        return info;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(gridItemNode));
    gridItemNode->eventHub_ = eventHub;
    gridItemNode->SetDraggable(true);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureEventHub, nullptr);
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(dragEventActuator, nullptr);
    /**
     * @tc.steps: step3. Run IsBelongToMultiItemNode.
     */
    auto isSelectedItemNode = dragEventActuator->IsSelectedItemNode(gridItemNode);
    EXPECT_EQ(isSelectedItemNode, true);
}

/**
 * @tc.name: TestIsSelectedItemNode002
 * @tc.desc: Test IsSelectedItemNode
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestIsSelectedItemNode002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create ListItemNode.
     */
    auto listItemNode = FrameNode::CreateFrameNode(
        V2::LIST_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<ListItemPattern>(nullptr, V2::ListItemStyle::NONE));
    ASSERT_NE(listItemNode, nullptr);
    auto itemPattern = listItemNode->GetPattern<ListItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    itemPattern->SetSelected(true);
    NG::DragPreviewOption option { true, false, true };
    listItemNode->SetDragPreviewOptions(option);
    /**
     * @tc.steps: step2. Create ListItemNode's DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto onDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) -> DragDropInfo {
        DragDropInfo info;
        return info;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(listItemNode));
    listItemNode->eventHub_ = eventHub;
    listItemNode->SetDraggable(true);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureEventHub, nullptr);
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(dragEventActuator, nullptr);
    /**
     * @tc.steps: step3. Run IsBelongToMultiItemNode.
     */
    auto isSelectedItemNode = dragEventActuator->IsSelectedItemNode(listItemNode);
    EXPECT_EQ(isSelectedItemNode, true);
}

/**
 * @tc.name: TestIsNeedGather001
 * @tc.desc: Test IsNeedGather
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestIsNeedGather001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List Node.
     */
    auto listNode = FrameNode::CreateFrameNode(
        V2::LIST_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ListPattern>());
    ASSERT_NE(listNode, nullptr);
    /**
     * @tc.steps: step2. Create List Item Node.
     */
    auto listItemNode1 = FrameNode::CreateFrameNode(
        V2::LIST_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<ListItemPattern>(nullptr, V2::ListItemStyle::NONE));
    ASSERT_NE(listItemNode1, nullptr);
    auto listItemNode2 = FrameNode::CreateFrameNode(
        V2::LIST_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<ListItemPattern>(nullptr, V2::ListItemStyle::NONE));
    ASSERT_NE(listItemNode2, nullptr);
    auto itemPattern1 = listItemNode1->GetPattern<ListItemPattern>();
    ASSERT_NE(itemPattern1, nullptr);
    itemPattern1->SetSelected(true);
    auto itemPattern2 = listItemNode2->GetPattern<ListItemPattern>();
    ASSERT_NE(itemPattern2, nullptr);
    itemPattern2->SetSelected(true);
    NG::DragPreviewOption option { true, false, true };
    listItemNode1->SetDragPreviewOptions(option);
    listNode->AddChild(listItemNode1);
    listNode->AddChild(listItemNode2);
    /**
     * @tc.steps: step3. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto onDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) -> DragDropInfo {
        DragDropInfo info;
        return info;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(listItemNode1));
    listItemNode1->eventHub_ = eventHub;
    listItemNode1->SetDraggable(true);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step4. Test IsNeedGather().
     */
    dragEventActuator->IsBelongToMultiItemNode(listItemNode1);
    EXPECT_EQ(dragEventActuator->isSelectedItemNode_, true);
    dragEventActuator->FindItemParentNode(listItemNode1);
    bool isNeedGather = dragEventActuator->IsNeedGather();
    EXPECT_EQ(isNeedGather, false);
}

/**
 * @tc.name: TestMountGatherNode001
 * @tc.desc: Test MountGatherNode.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestMountGatherNode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step2. Create a stackNode.
     */
    auto stackNode = FrameNode::GetOrCreateFrameNode(V2::STACK_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<StackPattern>(); });
    /**
     * @tc.steps: step3. Invoke MountGatherNode function.
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    auto overlayManager = pipeline->GetOverlayManager();
    EXPECT_NE(overlayManager, nullptr);
    std::vector<NG::GatherNodeChildInfo> gatherNodeChildrenInfo;
    dragEventActuator->MountGatherNode(overlayManager, frameNode, stackNode, gatherNodeChildrenInfo);
    EXPECT_EQ(overlayManager->hasGatherNode_, true);
}

/**
 * @tc.name: TestUpdateDefaultShadow
 * @tc.desc: Test get default shadow attribute.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestUpdateDefaultShadow, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create FrameNode.
     */
    auto frameNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    EXPECT_NE(frameNode, nullptr);
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step2. get DragPreviewOption.
     */
    auto dragPreviewOption = frameNode->GetDragPreviewOption();
    EXPECT_EQ(dragPreviewOption.options.shadow, std::nullopt);
    /**
     * @tc.steps: step3. set enableDefaultShadow.
     */
    dragPreviewOption.isDefaultShadowEnabled = true;
    frameNode->SetDragPreviewOptions(dragPreviewOption);
    /**
     * @tc.steps: step4. Invoke UpdatePreviewOptionDefaultAttr.
     */
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<ShadowTheme>()));

    dragEventActuator->UpdatePreviewOptionDefaultAttr(frameNode);
    dragPreviewOption = frameNode->GetDragPreviewOption();
    EXPECT_NE(dragPreviewOption.options.shadow, std::nullopt);
}

/**
 * @tc.name: TestApplyShadow
 * @tc.desc: Test set default shadow attribute.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestApplyShadow, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create FrameNode.
     */
    auto frameNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    EXPECT_NE(frameNode, nullptr);
    /**
     * @tc.steps: step2. get DragPreviewOption.
     */
    auto dragPreviewOption = frameNode->GetDragPreviewOption();
    EXPECT_EQ(dragPreviewOption.options.shadow, std::nullopt);
    /**
     * @tc.steps: step3. set defaultShadow.
     */
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<ShadowTheme>()));
    
    dragPreviewOption.options.shadow = DragEventActuator::GetDefaultShadow();
    frameNode->SetDragPreviewOptions(dragPreviewOption);
    /**
     * @tc.steps: step4. Invoke ApplyNewestOptionExecutedFromModifierToNode
     */
    auto imageNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    EXPECT_NE(imageNode, nullptr);
    imageNode->SetDragPreviewOptions(frameNode->GetDragPreviewOption());
    DragEventActuator::ApplyNewestOptionExecutedFromModifierToNode(frameNode, imageNode);
    auto imageContext = imageNode->GetRenderContext();
    EXPECT_NE(imageContext, nullptr);
    auto shadow = imageContext->GetBackShadow();
    EXPECT_NE(shadow, std::nullopt);
}

/**
 * @tc.name: TestBrulStyleToEffection001
 * @tc.desc: Test BrulStyleToEffection.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestBrulStyleToEffection001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step2. Invoke BrulStyleToEffection function.
     */
    std::vector<float> vecGrayScale = {0.0f, 0.0f};
    BlurStyleOption blurStyleInfo = {BlurStyle::NO_MATERIAL, ThemeColorMode::SYSTEM,
     AdaptiveColor::DEFAULT, 1.0, {vecGrayScale}};
    std::optional<BlurStyleOption> optBlurStyleInfo(blurStyleInfo);
    auto optEffectOption = dragEventActuator->BrulStyleToEffection(optBlurStyleInfo);
    auto pipeline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    EXPECT_EQ(optEffectOption.has_value(), false);
    /**
     * @tc.steps: step3. Create themeManager.
     */
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<BlurStyleTheme>()));
    auto blurStyleTheme = pipeline->GetTheme<BlurStyleTheme>();
    EXPECT_NE(blurStyleTheme, nullptr);
    auto resAdapter = RefPtr<ResourceAdapter>();
    auto themeConstants = AceType::MakeRefPtr<ThemeConstants>(resAdapter);
    std::unordered_map<std::string, ResValueWrapper> attributes;
    ResValueWrapper resValueWrapper;
    resValueWrapper.type = ThemeConstantsType::THEME;
    resValueWrapper.value = AceType::MakeRefPtr<ThemeStyle>();
    attributes.insert(std::pair<std::string, ResValueWrapper>(THEME_BLUR_STYLE_COMMON, resValueWrapper));
    themeConstants->currentThemeStyle_ = AceType::MakeRefPtr<ThemeStyle>();
    themeConstants->currentThemeStyle_->SetAttributes(attributes);
    auto blThemeInstance = BlurStyleTheme::Builder().Build(themeConstants);
    EXPECT_CALL(*themeManager, GetTheme(BlurStyleTheme::TypeId())).WillRepeatedly(Return(blThemeInstance));
    /**
     * @tc.steps: step4. Invoke BrulStyleToEffection function.
     */
    optEffectOption = dragEventActuator->BrulStyleToEffection(optBlurStyleInfo);
    ASSERT_NE(optEffectOption.has_value(), true);
}

/**
 * @tc.name: TestRadiusToSigma001
 * @tc.desc: Test RadiusToSigma.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, TestRadiusToSigma001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step2. Invoke RadiusToSigma function invalid.
     */
    float radius = -1.0f;
    auto sigMa = dragEventActuator->RadiusToSigma(radius);
    EXPECT_EQ(sigMa, 0.0f);
     /**
     * @tc.steps: step3. Invoke RadiusToSigma function.
     */
    float scaleHalf = 0.5f;
    float blurSigmaScale = 0.57735f;
    radius = 2.0f;
    float retSigMa = blurSigmaScale * radius + scaleHalf;
    sigMa = dragEventActuator->RadiusToSigma(radius);
    EXPECT_EQ(sigMa, retSigMa);
}

/**
 * @tc.name: GetDefaultBorderRadiusTest001
 * @tc.desc: Create DragEventActuator and invoke GetDefaultBorderRadius function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, GetDefaultBorderRadiusTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    /**
     * @tc.steps: step2. Test GetDefaultBorderRadius
     */
    NG::DragPreviewOption dragPreviewOptions { false, false, false, false, true };
    dragPreviewOptions.options.borderRadius = dragEventActuator->GetDefaultBorderRadius();
    frameNode->SetDragPreviewOptions(dragPreviewOptions);
    auto dragPreviewOption = frameNode->GetDragPreviewOption();
    auto borderRadius = dragPreviewOption.options.borderRadius;
    EXPECT_EQ(borderRadius.value().radiusTopLeft.value().Value(), 12.0);
    EXPECT_EQ(borderRadius.value().radiusTopRight.value().Value(), 12.0);
    EXPECT_EQ(borderRadius.value().radiusBottomRight.value().Value(), 12.0);
    EXPECT_EQ(borderRadius.value().radiusBottomLeft.value().Value(), 12.0);
    /**
     * @tc.steps: step3. Test PrepareRadiusParametersForDragData
     */
    auto arkExtraInfoJson = JsonUtil::Create(true);
    dragEventActuator->PrepareRadiusParametersForDragData(frameNode, arkExtraInfoJson);
    auto radiusTopLeft = arkExtraInfoJson->GetDouble("drag_corner_radius1", -1);
    auto radiusTopRight = arkExtraInfoJson->GetDouble("drag_corner_radius2", -1);
    auto radiusBottomRight = arkExtraInfoJson->GetDouble("drag_corner_radius3", -1);
    auto radiusBottomLeft = arkExtraInfoJson->GetDouble("drag_corner_radius4", -1);
    EXPECT_EQ(radiusTopLeft, 12.0);
    EXPECT_EQ(radiusTopRight, 12.0);
    EXPECT_EQ(radiusBottomRight, 12.0);
    EXPECT_EQ(radiusBottomLeft, 12.0);
}

/**
 * @tc.name: GetSetPressedKeyCodesTest001
 * @tc.desc: Test GetPressedKeyCodes and SetPressedKeyCodes function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, GetSetPressedKeyCodesTest001, TestSize.Level1)
{
    GestureEventFunc actionStart = [](GestureEvent& info) {};
    GestureEventFunc actionUpdate = [](GestureEvent& info) {};
    GestureEventFunc actionEnd = [](GestureEvent& info) {};
    GestureEventNoParameter actionCancel = []() {};
    auto dragEvent = AceType::MakeRefPtr<DragEvent>(std::move(actionStart), std::move(actionUpdate),
        std::move(actionEnd), std::move(actionCancel));
    dragEvent->SetPressedKeyCodes({KeyCode::KEY_DPAD_LEFT, KeyCode::KEY_DPAD_RIGHT});
    auto pressedKeyCodes = dragEvent->GetPressedKeyCodes();
    EXPECT_EQ(pressedKeyCodes.size(), 2);
    EXPECT_EQ(pressedKeyCodes[1], KeyCode::KEY_DPAD_RIGHT);
}

/**
 * @tc.name: DragEventActuatorRestartDragTaskTest001
 * @tc.desc: Test RestartDragTask function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventActuatorRestartDragTaskTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto framenode = FrameNode::CreateFrameNode("test", 1, AceType::MakeRefPtr<Pattern>(), false);
    ASSERT_NE(framenode, nullptr);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(framenode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureEventHub, nullptr);
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(dragEventActuator, nullptr);

    /**
     * @tc.steps: step2. Execute RestartDragTask when actionStart_ is null.
     */
    auto info = GestureEvent();
    dragEventActuator->RestartDragTask(info);
    ASSERT_EQ(dragEventActuator->actionStart_, nullptr);

    /**
     * @tc.steps: step3. Execute RestartDragTask when actionStart_ is not null.
     */
    
    int unknownPropertyValue = UNKNOWN;
    dragEventActuator->actionStart_ = [&unknownPropertyValue](GestureEvent& gestureInfo) mutable {
        unknownPropertyValue++;
    };
    dragEventActuator->RestartDragTask(info);
    ASSERT_NE(unknownPropertyValue, UNKNOWN);
}

/**
 * @tc.name: DragEventActuatorDragingStatusTest001
 * @tc.desc: Test IsGlobalStatusSuitableForDragging and IsCurrentCodeStatusSuitableForDragging function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventActuatorDragingStatusTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto framenode = FrameNode::CreateFrameNode("test", 1, AceType::MakeRefPtr<Pattern>(), false);
    ASSERT_NE(framenode, nullptr);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(framenode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureEventHub, nullptr);
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(dragEventActuator, nullptr);

    /**
     * @tc.steps: step2. Execute IsGlobalStatusSuitableForDragging.
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto dragDropManager = pipeline->GetDragDropManager();
    CHECK_NULL_VOID(dragDropManager);
    dragDropManager->ResetDragging(DragDropMgrState::DRAGGING);
    auto globalStatus = dragEventActuator->IsGlobalStatusSuitableForDragging();
    ASSERT_EQ(globalStatus, false);

    /**
     * @tc.steps: step3. Execute IsCurrentNodeStatusSuitableForDragging.
     */
    TouchRestrict dragTouchRestrict = { TouchRestrict::CLICK };
    dragTouchRestrict.inputEventType = InputEventType::AXIS;
    auto nodeStatus = dragEventActuator->IsCurrentNodeStatusSuitableForDragging(framenode, dragTouchRestrict);
    ASSERT_EQ(nodeStatus, false);
}

/**
 * @tc.name: DragEventActuatorSetDragDampStartPointInfoTest001
 * @tc.desc: Test SetDragDampStartPointInfo function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventActuatorSetDragDampStartPointInfoTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto framenode = FrameNode::CreateFrameNode("test", 1, AceType::MakeRefPtr<Pattern>(), false);
    ASSERT_NE(framenode, nullptr);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(framenode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureEventHub, nullptr);
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(dragEventActuator, nullptr);

    /**
     * @tc.steps: step2. Invoke SetDragDampStartPointInfo.
     * @tc.expected: cover SetDragDampStartPointInfo.
     */
    GestureEvent info = GestureEvent();
    dragEventActuator->SetDragDampStartPointInfo(info.GetGlobalPoint(), info.GetPointerId());
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto dragDropManager = pipeline->GetDragDropManager();
    CHECK_NULL_VOID(dragDropManager);
    ASSERT_EQ(dragDropManager->currentPointerId_, info.GetPointerId());
}

/**
 * @tc.name: DragEventActuatorHandleDragDampingMoveTest001
 * @tc.desc: Test HandleDragDampingMove function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventActuatorHandleDragDampingMoveTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto framenode = FrameNode::CreateFrameNode("test", 1, AceType::MakeRefPtr<Pattern>(), false);
    ASSERT_NE(framenode, nullptr);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(framenode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureEventHub, nullptr);
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(dragEventActuator, nullptr);

    /**
     * @tc.steps: step2. Invoke DragDampingMove with DragDropMgrState::IDLE and same Pointer.
     * @tc.expected: cover DragDampingMove.
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto dragDropManager = pipeline->GetDragDropManager();
    CHECK_NULL_VOID(dragDropManager);
    TouchEventInfo info(TOUCH_EVENT_INFO_TYPE);
    auto point = Point(info.GetTouches().front().GetGlobalLocation().GetX(),
                       info.GetTouches().front().GetGlobalLocation().GetY());
    dragDropManager->SetDraggingPointer(info.GetTouches().front().GetFingerId());
    dragDropManager->ResetDragging(DragDropMgrState::IDLE);
    dragEventActuator->HandleDragDampingMove(point, info.GetTouches().front().GetFingerId());
    ASSERT_FALSE(dragDropManager->IsAboutToPreview());
    ASSERT_FALSE(dragDropManager->IsDragging());
    EXPECT_FALSE(!dragDropManager->IsSameDraggingPointer(info.GetTouches().front().GetFingerId()));

    /**
     * @tc.steps: step3. Invoke DragDampingMove with DragDropMgrState::DRAGGING and same Pointer.
     * @tc.expected: cover DragDampingMove.
     */
    dragDropManager->ResetDragging(DragDropMgrState::DRAGGING);
    dragEventActuator->HandleDragDampingMove(point, info.GetTouches().front().GetFingerId());
    EXPECT_FALSE(dragDropManager->IsAboutToPreview());
    EXPECT_TRUE(dragDropManager->IsDragging());
    EXPECT_FALSE(!dragDropManager->IsSameDraggingPointer(info.GetTouches().front().GetFingerId()));

    /**
     * @tc.steps: step4. Invoke DragDampingMove with DragDropMgrState::ABOUT_TO_PREVIEW and same Pointer.
     * @tc.expected: cover DragDampingMove.
     */
    dragDropManager->ResetDragging(DragDropMgrState::ABOUT_TO_PREVIEW);
    dragEventActuator->HandleDragDampingMove(point, info.GetTouches().front().GetFingerId());
    EXPECT_TRUE(dragDropManager->IsAboutToPreview());
    EXPECT_FALSE(dragDropManager->IsDragging());
    EXPECT_FALSE(!dragDropManager->IsSameDraggingPointer(info.GetTouches().front().GetFingerId()));

    /**
     * @tc.steps: step5. Invoke DragDampingMove.
     * @tc.expected: cover DragDampingMove.
     */
    dragDropManager->ResetDragging(DragDropMgrState::IDLE);
    point.SetX(10.0);
    point.SetY(10.0);
    dragEventActuator->HandleDragDampingMove(point, info.GetTouches().front().GetFingerId());
    auto startPoint = dragDropManager->GetDragDampStartPoint();
    auto delta = Point(point.GetX(), point.GetY()) - startPoint;
    auto distance = SystemProperties::GetDragStartPanDistanceThreshold();
    EXPECT_TRUE(delta.GetDistance() > Dimension(distance, DimensionUnit::VP).ConvertToPx());

    point.SetX(1.0);
    point.SetY(1.0);
    dragEventActuator->HandleDragDampingMove(point, info.GetTouches().front().GetFingerId());
    startPoint = dragDropManager->GetDragDampStartPoint();
    delta = Point(point.GetX(), point.GetY()) - startPoint;
    distance = SystemProperties::GetDragStartPanDistanceThreshold();
    EXPECT_TRUE(delta.GetDistance() < Dimension(distance, DimensionUnit::VP).ConvertToPx());
}

/**
 * @tc.name: DragEventActuatorUpdatePreviewAttrTest001
 * @tc.desc: Test UpdatePreviewAttr function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventActuatorUpdatePreviewAttrTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>(), false);
    ASSERT_NE(frameNode, nullptr);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureEventHub, nullptr);
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(dragEventActuator, nullptr);

    /**
     * @tc.steps: step2. Invoke UpdatePreviewAttr.
     * @tc.expected: cover UpdatePreviewAttr with TextDraggable true.
     */
    auto frameTag = frameNode->GetTag();
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    EXPECT_NE(gestureHub, nullptr);
    auto imageNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<ImagePattern>(); });
    EXPECT_NE(imageNode, nullptr);
    gestureHub->SetTextDraggable(true);
    dragEventActuator->UpdatePreviewAttr(frameNode, imageNode);
    EXPECT_TRUE(gestureHub->IsTextCategoryComponent(frameTag));
    EXPECT_TRUE(gestureHub->GetTextDraggable());
}

/**
 * @tc.name: DragEventActuatorSetPixelMapTest001
 * @tc.desc: Test SetPixelMap function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventActuatorSetPixelMapTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto framenode = FrameNode::CreateFrameNode("test", 1, AceType::MakeRefPtr<Pattern>(), false);
    ASSERT_NE(framenode, nullptr);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(framenode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureEventHub, nullptr);
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(dragEventActuator, nullptr);

    /**
     * @tc.steps: step2. Invoke SetHasPixelMap.
     * @tc.desc: GetHasPixelMap true.
     */
    auto pipelineContext = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipelineContext, nullptr);
    auto manager = pipelineContext->GetOverlayManager();
    ASSERT_NE(manager, nullptr);
    manager->SetHasPixelMap(true);
    dragEventActuator->SetPixelMap(dragEventActuator);
    ASSERT_TRUE(manager->GetHasPixelMap());
}

/**
 * @tc.name: DragEventActuatorBrulStyleToEffectionTest001
 * @tc.desc: Test BrulStyleToEffection function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventActuatorBrulStyleToEffectionTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("test", 1, AceType::MakeRefPtr<Pattern>(), false);
    ASSERT_NE(frameNode, nullptr);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureEventHub, nullptr);
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(dragEventActuator, nullptr);

    /**
     * @tc.steps: step2. Invoke BrulStyleToEffection.
     */
    auto imageNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<ImagePattern>(); });
    ASSERT_NE(imageNode, nullptr);
    auto imageContext = imageNode->GetRenderContext();
    ASSERT_NE(imageContext, nullptr);
    auto blurstyletmp = imageContext->GetBackBlurStyle();
    blurstyletmp->colorMode = ThemeColorMode::DARK;
    dragEventActuator->BrulStyleToEffection(blurstyletmp);
    ASSERT_NE(blurstyletmp->colorMode, ThemeColorMode::SYSTEM);
}

/**
 * @tc.name: DragEventActuatorHidePixelMapTest001
 * @tc.desc: Test HidePixelMap function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventActuatorHidePixelMapTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("test", 1, AceType::MakeRefPtr<Pattern>(), false);
    ASSERT_NE(frameNode, nullptr);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureEventHub, nullptr);
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(dragEventActuator, nullptr);

    /**
     * @tc.steps: step2. Invoke HidePixelMap.
     */
    auto pipelineContext = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipelineContext, nullptr);
    auto manager = pipelineContext->GetOverlayManager();
    EXPECT_NE(manager, nullptr);
    dragEventActuator->HidePixelMap(true, 0, 0, false);
    EXPECT_FALSE(manager->hasPixelMap_);
}

/**
 * @tc.name: DragEventActuatorSetTextPixelMapTest001
 * @tc.desc: Test SetTextPixelMap function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventActuatorSetTextPixelMapTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("test", 1, AceType::MakeRefPtr<Pattern>(), false);
    ASSERT_NE(frameNode, nullptr);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureEventHub, nullptr);
    auto dragEventActuator = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(dragEventActuator, nullptr);

    /**
     * @tc.steps: step2. Invoke HidePixelMap.
     */
    dragEventActuator->SetTextPixelMap(gestureEventHub);
    EXPECT_EQ(dragEventActuator->textPixelMap_, nullptr);
}
} // namespace OHOS::Ace::NG
