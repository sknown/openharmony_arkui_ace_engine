/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#undef SECURITY_COMPONENT_ENABLE

#include "test/mock/base/mock_drag_window.h"
#include "test/mock/base/mock_task_executor.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/common/mock_udmf.h"

#include "base/geometry/axis.h"
#include "base/geometry/ng/offset_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/event/pan_event.h"
#include "core/components_ng/event/touch_event.h"
#include "core/components_ng/gestures/long_press_gesture.h"
#include "core/components_ng/gestures/pan_gesture.h"
#include "core/components_ng/gestures/recognizers/click_recognizer.h"
#include "core/components_ng/gestures/tap_gesture.h"
#include "core/components_ng/manager/drag_drop/drag_drop_proxy.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "frameworks/core/common/event_manager.h"
#include "frameworks/core/components_ng/pattern/text/text_pattern.h"
#include "frameworks/core/components_ng/pattern/text_drag/text_drag_base.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::string NODE_TAG("node");
const OffsetF COORDINATE_OFFSET(20.0f, 20.0f);
constexpr int32_t TOUCH_ID = 0;
const Axis AXIS_VERTICAL = Axis::VERTICAL;
const PanDirection PAN_DIRECTION_ALL;
constexpr int32_t FINGERS = 1;
constexpr int32_t DOUBLE_FINGERS = 2;
constexpr int32_t CLICK_COUNTS = 2;
constexpr Dimension DISTANCE = 10.0_vp;
const std::string CHECK_TAG_1("HELLO");
const std::string CHECK_TAG_2("WORLD");
const PointF GLOBAL_POINT { 20.0f, 20.0f };
const PointF LOCAL_POINT { 15.0f, 15.0f };
RefPtr<DragWindow> MOCK_DRAG_WINDOW;
constexpr int32_t GESTURES_COUNTS = 2;
RefPtr<MockTaskExecutor> MOCK_TASK_EXECUTOR;
} // namespace

class GestureEventHubTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};

void GestureEventHubTestNg::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
    MOCK_DRAG_WINDOW = DragWindow::CreateDragWindow("", 0, 0, 0, 0);
    GTEST_LOG_(INFO) << "GestureEventHubTestNg SetUpTestCase";
    MOCK_TASK_EXECUTOR = AceType::MakeRefPtr<MockTaskExecutor>();
}

void GestureEventHubTestNg::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
    MOCK_DRAG_WINDOW = nullptr;
    GTEST_LOG_(INFO) << "GestureEventHubTestNg TearDownTestCase";
}

/**
 * @tc.name: GestureEventHubTest001
 * @tc.desc: Create GestureEventHub and call GetFrameNode
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: eventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. Test GetFrameNode
     *            case: eventHub is valid
     * @tc.expected: frameNodeOfEvent is not null.
     */
    auto frameNodeOfEvent = gestureEventHub->GetFrameNode();
    EXPECT_TRUE(frameNodeOfEvent);

    /**
     * @tc.steps: step2. Test GetFrameNode
     *            case: eventHub is invalid
     * @tc.expected: frameNodeOfEvent is null.
     */
    eventHub = nullptr;
    frameNode = nullptr;
    frameNodeOfEvent = gestureEventHub->GetFrameNode();
    EXPECT_FALSE(frameNodeOfEvent);
}

/**
 * @tc.name: GestureEventHubTest002
 * @tc.desc: Test ProcessTouchTestHit part1
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. Test ProcessTouchTestHit
     *            case: eventHub is null && actuators such as scrollableActuator_ & touchEventActuator_ are all null
     * @tc.expected: ProcessTouchTestHit return false,  innerTargets & finalResult is empty
     */
    eventHub = nullptr;
    TouchRestrict touchRestrict;
    TouchTestResult innerTargets;
    TouchTestResult finalResult;
    auto flag = gestureEventHub->ProcessTouchTestHit(
        COORDINATE_OFFSET, touchRestrict, innerTargets, finalResult, TOUCH_ID, PointF(), nullptr);
    EXPECT_FALSE(flag);
    auto sizeOfInnerTargets = static_cast<int32_t>(innerTargets.size());
    auto sizeOfFinalResult = static_cast<int32_t>(finalResult.size());
    EXPECT_EQ(sizeOfInnerTargets, 0);
    EXPECT_EQ(sizeOfFinalResult, 0);

    /**
     * @tc.steps: step3. construct touchEventActuator_
     *                   then set it to gestureEventHub
     */
    // reconstruct a gestureEventHub
    eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto framenode = FrameNode::CreateFrameNode("test", 1, AceType::MakeRefPtr<Pattern>(), false);
    EXPECT_NE(framenode, nullptr);
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(framenode));
    gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);
    // set touchEventActuator_
    auto touchCallback = [](TouchEventInfo& info) {};
    auto touchEvent = AceType::MakeRefPtr<TouchEventImpl>(std::move(touchCallback));
    gestureEventHub->AddTouchEvent(touchEvent);

    /**
     * @tc.steps: step4. Test ProcessTouchTestHit
     *            case: eventHub is not null && touchEventActuator_ is not null
     * @tc.expected: ProcessTouchTestHit return false,  innerTargets & finalResult have one element
     */
    flag = gestureEventHub->ProcessTouchTestHit(
        COORDINATE_OFFSET, touchRestrict, innerTargets, finalResult, TOUCH_ID, PointF(), nullptr);
    EXPECT_FALSE(flag);
    sizeOfInnerTargets = static_cast<int32_t>(innerTargets.size());
    sizeOfFinalResult = static_cast<int32_t>(finalResult.size());
    EXPECT_EQ(sizeOfInnerTargets, 1);
    EXPECT_EQ(sizeOfFinalResult, 1);
}

/**
 * @tc.name: GestureEventHubTest003
 * @tc.desc: Test ProcessTouchTestHit part2
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. construct scrollableActuator_ and other actuators
     *                   then set them to gestureEventHub
     */

    // set touchEventActuator_
    auto touchCallback = [](TouchEventInfo& info) {};
    auto touchEvent = AceType::MakeRefPtr<TouchEventImpl>(std::move(touchCallback));
    gestureEventHub->AddTouchEvent(touchEvent);

    // set scrollableActuator_
    auto scrollableEvent = AceType::MakeRefPtr<ScrollableEvent>(AXIS_VERTICAL);
    gestureEventHub->AddScrollableEvent(scrollableEvent);

    // set clickEventActuator_
    auto clickCallback = [](GestureEvent& info) {};
    auto clickEvent = AceType::MakeRefPtr<ClickEvent>(std::move(clickCallback));
    gestureEventHub->AddClickEvent(clickEvent);

    // set panEventActuator_
    auto panActionStart = [](GestureEvent& info) {};
    auto panActionUpdate = [](GestureEvent& info) {};
    auto panActionEnd = [](GestureEvent& info) {};
    auto panActionCancel = []() {};
    auto panEvent = AceType::MakeRefPtr<PanEvent>(
        std::move(panActionStart), std::move(panActionUpdate), std::move(panActionEnd), std::move(panActionCancel));
    gestureEventHub->AddPanEvent(panEvent, PAN_DIRECTION_ALL, FINGERS, DISTANCE);

    // set longPressEventActuator_
    auto longPressCallback = [](GestureEvent& info) {};
    auto longPressEvent = AceType::MakeRefPtr<LongPressEvent>(longPressCallback);
    gestureEventHub->SetLongPressEvent(longPressEvent);

    // set dragEventActuator_
    auto dragActionStart = [](GestureEvent& info) {};
    auto dragActionUpdate = [](GestureEvent& info) {};
    auto dragActionEnd = [](GestureEvent& info) {};
    auto dragActionCancel = []() {};
    auto dragEvent = AceType::MakeRefPtr<DragEvent>(
        std::move(dragActionStart), std::move(dragActionUpdate), std::move(dragActionEnd), std::move(dragActionCancel));
    gestureEventHub->SetCustomDragEvent(dragEvent, PAN_DIRECTION_ALL, FINGERS, DISTANCE);
}

/**
 * @tc.name: GestureEventHubTest004
 * @tc.desc: Test AddClickEvent, SetUserOnClick, ActClick & SetFocusClickEvent
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. call ActClick
     *            case: clickEventActuator_ is null
     * @tc.expected: flag is false
     */
    auto flag = gestureEventHub->ActClick();
    EXPECT_FALSE(flag);

    /**
     * @tc.steps: step3. call ActClick
     *            case: clickEventActuator_ is null, clickRecognizer fingers is 2, count is 1
     * @tc.expected: flag is false
     */
    auto clickRecognizer = AceType::MakeRefPtr<ClickRecognizer>(DOUBLE_FINGERS, 1);
    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer);
    EXPECT_FALSE(gestureEventHub->ActClick());
    gestureEventHub->gestureHierarchy_.clear();

    /**
     * @tc.steps: step4. call ActClick
     *            case: clickEventActuator_ is null, clickRecognizer fingers is 1, count is 1
     * @tc.expected: flag is true
     */
    clickRecognizer = AceType::MakeRefPtr<ClickRecognizer>(1, 1);
    clickRecognizer->SetOnAction([](GestureEvent& info) {});
    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer);
    EXPECT_TRUE(gestureEventHub->ActClick());
    gestureEventHub->gestureHierarchy_.clear();

    /**
     * @tc.steps: step5. construct two clickCallback
     *            one is for SetUserOnClick, the other is for AddClickEvent
     */
    std::string msg1;
    auto clickCallback = [&msg1](GestureEvent& /* info */) { msg1 = CHECK_TAG_1; };
    gestureEventHub->SetUserOnClick(clickCallback);
    std::string msg2;
    auto clickCallback2 = [&msg2](GestureEvent& /* info */) { msg2 = CHECK_TAG_2; };
    auto clickEvent = AceType::MakeRefPtr<ClickEvent>(std::move(clickCallback2));
    gestureEventHub->AddClickEvent(clickEvent);

    /**
     * @tc.steps: step6. call ActClick
     *                   case: clickEventActuator_ is not null
     * @tc.expected: flag is true & clickCallback & clickCallback2 has be called
     */
    flag = gestureEventHub->ActClick();
    EXPECT_TRUE(flag);
    EXPECT_EQ(msg1, CHECK_TAG_1);
    EXPECT_EQ(msg2, CHECK_TAG_2);

    /**
     * @tc.steps: step7. call eventHub's GetOrCreateFocusHub
     * @tc.expected: return is not null
     */
    auto focusHub = eventHub->GetOrCreateFocusHub();
    EXPECT_TRUE(focusHub);

    /**
     * @tc.steps: step8. call SetFocusClickEvent
     * @tc.expected: no fatal error occur
     */
    msg1 = "";
    auto clickCallback3 = [&msg1](GestureEvent& /* info */) { msg1 = CHECK_TAG_1; };
    gestureEventHub->SetFocusClickEvent(clickCallback3);
}

/**
 * @tc.name: GestureEventHubTest005
 * @tc.desc: Test ActLongClick
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. call ActLongClick
     *            case: longPressEventActuator_ is null
     * @tc.expected: flag is false
     */
    auto flag = gestureEventHub->ActLongClick();
    EXPECT_FALSE(flag);

    /**
     * @tc.steps: step3. call ActLongClick
     *            case: longPressEventActuator_ is null, longPressRecognizer fingers is 2
     * @tc.expected: flag is false
     */
    auto longPressRecognizer = AceType::MakeRefPtr<LongPressRecognizer>(1, DOUBLE_FINGERS, false);
    gestureEventHub->gestureHierarchy_.emplace_back(longPressRecognizer);
    EXPECT_FALSE(gestureEventHub->ActLongClick());
    gestureEventHub->gestureHierarchy_.clear();

    /**
     * @tc.steps: step4. call ActLongClick
     *            case: longPressEventActuator_ is null, longPressRecognizer fingers is 1
     * @tc.expected: flag is true
     */
    longPressRecognizer = AceType::MakeRefPtr<LongPressRecognizer>(1, 1, false);
    longPressRecognizer->SetOnAction([](GestureEvent& info) {});
    gestureEventHub->gestureHierarchy_.emplace_back(longPressRecognizer);
    EXPECT_TRUE(gestureEventHub->ActLongClick());
    gestureEventHub->gestureHierarchy_.clear();

    /**
     * @tc.steps: step5. construct a longPressCallback
     */
    std::string msg1;
    auto longPressCallback = [&msg1](GestureEvent& /* info */) { msg1 = CHECK_TAG_1; };
    auto longPressEvent = AceType::MakeRefPtr<LongPressEvent>(longPressCallback);
    gestureEventHub->SetLongPressEvent(longPressEvent);

    /**
     * @tc.steps: step6. call ActLongClick
     *                   case: longPressEventActuator_ is not null
     * @tc.expected: flag is true & longPressCallback will be called
     */
    flag = gestureEventHub->ActLongClick();
    EXPECT_TRUE(flag);
    EXPECT_EQ(msg1, CHECK_TAG_1);

    /**
     * @tc.steps: step7. call eventHub's GetOrCreateFocusHub
     * @tc.expected: return is not null
     */
    auto focusHub = eventHub->GetOrCreateFocusHub();
    EXPECT_TRUE(focusHub);

    /**
     * @tc.steps: step8. call SetFocusClickEvent
     * @tc.expected: no fatal error occur
     */
    msg1 = "";
    auto clickCallback3 = [&msg1](GestureEvent& /* info */) { msg1 = CHECK_TAG_1; };
    gestureEventHub->SetFocusClickEvent(clickCallback3);
}

/**
 * @tc.name: GestureEventHubTest006
 * @tc.desc: Test CombineIntoExclusiveRecognizer
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. call CombineIntoExclusiveRecognizer
     *            case: result is empty
     * @tc.expected: result is empty
     */
    TouchTestResult result;
    gestureEventHub->CombineIntoExclusiveRecognizer(GLOBAL_POINT, LOCAL_POINT, result, TOUCH_ID);
    auto size = static_cast<int32_t>(result.size());
    EXPECT_EQ(size, 0);

    /**
     * @tc.steps: step3. insert element to result
     * @tc.expected: result'size is 3
     */

    // new TouchEventActuator
    auto touchEventActuator = AceType::MakeRefPtr<TouchEventActuator>();

    // new LongPressRecognizer (extends NGGestureRecognizer)
    auto longPressRecognizer = AceType::MakeRefPtr<LongPressRecognizer>(false, false);

    // new ClickRecognizer (extends NGGestureRecognizer)
    auto clickRecognizer = AceType::MakeRefPtr<ClickRecognizer>();

    result.emplace_back(touchEventActuator);
    result.emplace_back(longPressRecognizer);
    result.emplace_back(clickRecognizer);
    size = static_cast<int32_t>(result.size());
    EXPECT_EQ(size, 3);

    /**
     * @tc.steps: step4. call CombineIntoExclusiveRecognizer
     *            case: recognizers'size > 1
     * @tc.expected: result'size is 2. One is touchEventActuator, the other is a exclusiveRecognizer created by
     *               longPressRecognizer and clickRecognizer
     */
    gestureEventHub->CombineIntoExclusiveRecognizer(GLOBAL_POINT, LOCAL_POINT, result, TOUCH_ID);
    size = static_cast<int32_t>(result.size());
    EXPECT_EQ(size, 2);

    /**
     * @tc.steps: step5. call CombineIntoExclusiveRecognizer
     *            case: recognizers'size = 1
     * @tc.expected: result2'size is 2. One is touchEventActuator, the other is longPressRecognizer
     */
    TouchTestResult result2;
    result2.emplace_back(touchEventActuator);
    result2.emplace_back(longPressRecognizer);
    gestureEventHub->CombineIntoExclusiveRecognizer(GLOBAL_POINT, LOCAL_POINT, result2, TOUCH_ID);
    size = static_cast<int32_t>(result2.size());
    EXPECT_EQ(size, 2);
}

/**
 * @tc.name: GestureEventHubTest007
 * @tc.desc: Test InitDragDropEvent
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. Call InitDragDropEvent.
     * @tc.expected: dragEventActuator_ is not null.
     */
    gestureEventHub->InitDragDropEvent();
    EXPECT_TRUE(gestureEventHub->dragEventActuator_);
}

/**
 * @tc.name: GestureEventHubTest008
 * @tc.desc: Test Functions related with drag
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. call HandleOnDragStart
     *            case: eventHub->HasOnDragStart() is null
     * @tc.expected: dragDropProxy_ is null.
     */
    GestureEvent info;
    gestureEventHub->HandleOnDragStart(info);
    EXPECT_FALSE(gestureEventHub->dragDropProxy_);

    /**
     * @tc.steps: step3. set OnDragStart for eventHub
     *            after that eventHub->HasOnDragStart() is not null
     *            case: dragDropInfo.customNode is not null
     */
    RefPtr<UINode> customNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    auto onDragStart = [&customNode](
                           const RefPtr<OHOS::Ace::DragEvent>& /* dragEvent */, const std::string& /* param */) {
        DragDropInfo dragDropInfo;
        dragDropInfo.customNode = customNode;
        return dragDropInfo;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));

    /**
     * @tc.steps: step4. call HandleOnDragStart
     *            case: dragDropInfo.customNode is not null
     * @tc.expected: dragDropProxy_ is not null.
     */
    gestureEventHub->HandleOnDragStart(info);
    EXPECT_FALSE(gestureEventHub->dragDropProxy_);

    /**
     * @tc.steps: step5. set OnDragStart for eventHub2
     *            after that eventHub->HasOnDragStart() is not null
     *            case: dragDropInfo.pixelMap is not null
     */
    void* voidPtr = static_cast<void*>(new char[0]);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    auto onDragStart2 = [&pixelMap](
                            const RefPtr<OHOS::Ace::DragEvent>& /* dragEvent */, const std::string& /* param */) {
        DragDropInfo dragDropInfo;
        dragDropInfo.pixelMap = pixelMap;
        return dragDropInfo;
    };
    auto eventHub2 = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub2);
    eventHub2->SetOnDragStart(std::move(onDragStart2));
    auto gestureEventHub2 = AceType::MakeRefPtr<GestureEventHub>(eventHub2);
    EXPECT_TRUE(gestureEventHub2);

    /**
     * @tc.steps: step6. call HandleOnDragStart
     *            case: dragDropInfo.pixelMap is not null
     * @tc.expected: dragDropProxy_ is not null.
     */
    gestureEventHub2->HandleOnDragStart(info);
    EXPECT_FALSE(gestureEventHub2->dragDropProxy_);

    /**
     * @tc.steps: step7. call HandleOnDragStart again
     *            case: dragDropProxy_ need to reset
     * @tc.expected: dragDropProxy_ is not null.
     */
    gestureEventHub2->HandleOnDragStart(info);
    EXPECT_FALSE(gestureEventHub2->dragDropProxy_);

    /**
     * @tc.steps: step8. call HandleOnDragUpdate
     * @tc.expected: dragDropProxy_ is not null.
     */
    gestureEventHub2->HandleOnDragUpdate(info);
    EXPECT_FALSE(gestureEventHub2->dragDropProxy_);

    /**
     * @tc.steps: step9. call HandleOnDragCancel
     * @tc.expected: dragDropProxy_ is null.
     */
    gestureEventHub2->HandleOnDragCancel();
    EXPECT_FALSE(gestureEventHub2->dragDropProxy_);

    /**
     * @tc.steps: step10. call HandleOnDragEnd
     *            case: eventHub->HasOnDrop() is false
     * @tc.expected: dragDropProxy_ is null.
     */
    gestureEventHub->HandleOnDragEnd(info);
    EXPECT_FALSE(gestureEventHub->dragDropProxy_);

    /**
     * @tc.steps: step10. call HandleOnDragEnd
     *            case: eventHub->HasOnDrop() is true
     * @tc.expected: dragDropProxy_ is null
     *               onDrop has been called, msg1 = CHECK_TAG_1
     */
    std::string msg1;
    auto onDrop = [&msg1](const RefPtr<OHOS::Ace::DragEvent>& /* dragEvent */, const std::string& /* param */) {
        msg1 = CHECK_TAG_1;
    };
    eventHub->SetOnDrop(std::move(onDrop));
    gestureEventHub->HandleOnDragStart(info);
    gestureEventHub->HandleOnDragEnd(info);
    EXPECT_FALSE(gestureEventHub->dragDropProxy_);
    EXPECT_EQ(msg1, "");
}

/**
 * @tc.name: GestureEventHubTest009
 * @tc.desc: Test ModifyDone & UpdateGestureHierarchy
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. call OnModifyDone
     *            case: recreateGesture_ is true & gestures_.size() == gestureHierarchy_.size() == 0
     * @tc.expected: recreateGesture_ = false
     */
    gestureEventHub->OnModifyDone();
    EXPECT_FALSE(gestureEventHub->recreateGesture_);

    /**
     * @tc.steps: step3. call OnModifyDone
     *            case: recreateGesture_ is false
     * @tc.expected: recreateGesture_ = false
     */
    gestureEventHub->OnModifyDone();
    EXPECT_FALSE(gestureEventHub->recreateGesture_);

    /**
     * @tc.steps: step4. call OnModifyDone
     *            case: recreateGesture_ is true & gestures_.size() != gestureHierarchy_.size()
     * @tc.expected: recreateGesture_ = false
     *               gestures_ has cleared & gestureHierarchy_ has one element
     */
    gestureEventHub->recreateGesture_ = true;
    auto longPressGesture = AceType::MakeRefPtr<LongPressGesture>(FINGERS, false, 1);
    gestureEventHub->gestures_.emplace_back(longPressGesture);
    gestureEventHub->OnModifyDone();
    EXPECT_FALSE(gestureEventHub->recreateGesture_);
    auto sizeGestures = static_cast<int32_t>(gestureEventHub->gestures_.size());
    auto sizeGestureHierarchy = static_cast<int32_t>(gestureEventHub->gestureHierarchy_.size());
    EXPECT_EQ(sizeGestures, 0);
    EXPECT_EQ(sizeGestureHierarchy, 1);
}

/**
 * @tc.name: GestureEventHubTest010
 * @tc.desc: Test ProcessTouchTestHierarchy
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. call ProcessTouchTestHierarchy
     *            case: innerRecognizers & gestureHierarchy_ is empty, current is null
     * @tc.expected: finalResult is empty
     */
    TouchRestrict touchRestrict;
    std::list<RefPtr<NGGestureRecognizer>> innerTargets;
    TouchTestResult finalResult;

    std::vector<RefPtr<NGGestureRecognizer>> vc;
    vc.push_back(AceType::MakeRefPtr<ClickRecognizer>());
    auto exclusiveRecognizer = AceType::MakeRefPtr<ExclusiveRecognizer>(vc);
    std::vector<RefPtr<NGGestureRecognizer>> parallelVc;
    parallelVc.push_back(AceType::MakeRefPtr<ClickRecognizer>());
    auto parallelRecognizer = AceType::MakeRefPtr<ParallelRecognizer>(parallelVc);

    gestureEventHub->externalExclusiveRecognizer_.push_back(exclusiveRecognizer);
    gestureEventHub->externalParallelRecognizer_.push_back(parallelRecognizer);
    EXPECT_EQ(static_cast<int32_t>(gestureEventHub->externalExclusiveRecognizer_.size()), 1);
    EXPECT_EQ(static_cast<int32_t>(gestureEventHub->externalParallelRecognizer_.size()), 1);

    gestureEventHub->ProcessTouchTestHierarchy(
        COORDINATE_OFFSET, touchRestrict, innerTargets, finalResult, TOUCH_ID, nullptr);
    EXPECT_TRUE(finalResult.empty());

    /**
     * @tc.steps: step3. call ProcessTouchTestHierarchy several
     *            case: innerRecognizers & gestureHierarchy_ is not all empty
     * @tc.expected: finalResult's size has increased by 1 per call
     */
    auto clickRecognizer = AceType::MakeRefPtr<ClickRecognizer>(FINGERS, 1); // priority == GesturePriority::Low
    innerTargets.emplace_back(clickRecognizer);

    gestureEventHub->gestureHierarchy_.emplace_back(nullptr);
    auto clickRecognizer2 = AceType::MakeRefPtr<ClickRecognizer>(FINGERS, 1);
    clickRecognizer2->SetPriorityMask(GestureMask::IgnoreInternal);           // current will assigned to this
    auto clickRecognizer3 = AceType::MakeRefPtr<ClickRecognizer>(FINGERS, 1); // priority == GesturePriority::High
    clickRecognizer3->SetPriority(GesturePriority::High);
    auto clickRecognizer4 = AceType::MakeRefPtr<ClickRecognizer>(FINGERS, 1); // priority == GesturePriority::Parallel
    clickRecognizer4->SetPriority(GesturePriority::Parallel);
    auto clickRecognizer5 = AceType::MakeRefPtr<ClickRecognizer>(FINGERS, 1); // priority == GesturePriority::Parallel
    clickRecognizer5->SetPriority(GesturePriority::Parallel);

    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer);
    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer4);
    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer2);
    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer3);
    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer5);

    gestureEventHub->ProcessTouchTestHierarchy(
        COORDINATE_OFFSET, touchRestrict, innerTargets, finalResult, TOUCH_ID, nullptr);
    auto sizeOfFinalResult = static_cast<int32_t>(finalResult.size());
    EXPECT_EQ(sizeOfFinalResult, 1);

    auto clickRecognizer6 = AceType::MakeRefPtr<ClickRecognizer>(FINGERS, 1); // priority == GesturePriority::Low
    std::list<RefPtr<NGGestureRecognizer>> innerTargets2;
    innerTargets2.emplace_back(clickRecognizer);
    innerTargets2.emplace_back(clickRecognizer6);
    gestureEventHub->ProcessTouchTestHierarchy(
        COORDINATE_OFFSET, touchRestrict, innerTargets2, finalResult, TOUCH_ID, nullptr);
    sizeOfFinalResult = static_cast<int32_t>(finalResult.size());
    EXPECT_EQ(sizeOfFinalResult, 2);

    std::list<RefPtr<NGGestureRecognizer>> innerTargets3;
    innerTargets3.emplace_back(clickRecognizer);
    innerTargets3.emplace_back(clickRecognizer6);
    gestureEventHub->ProcessTouchTestHierarchy(
        COORDINATE_OFFSET, touchRestrict, innerTargets3, finalResult, TOUCH_ID, nullptr);
    sizeOfFinalResult = static_cast<int32_t>(finalResult.size());
    EXPECT_EQ(sizeOfFinalResult, 3);

    std::list<RefPtr<NGGestureRecognizer>> innerTargets4;
    gestureEventHub->gestureHierarchy_.clear();
    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer4);
    gestureEventHub->ProcessTouchTestHierarchy(
        COORDINATE_OFFSET, touchRestrict, innerTargets4, finalResult, TOUCH_ID, nullptr);
    sizeOfFinalResult = static_cast<int32_t>(finalResult.size());
    EXPECT_EQ(sizeOfFinalResult, 4);

    std::list<RefPtr<NGGestureRecognizer>> innerTargets5;
    gestureEventHub->gestureHierarchy_.clear();
    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer);
    gestureEventHub->ProcessTouchTestHierarchy(
        COORDINATE_OFFSET, touchRestrict, innerTargets5, finalResult, TOUCH_ID, nullptr);
    sizeOfFinalResult = static_cast<int32_t>(finalResult.size());
    EXPECT_EQ(sizeOfFinalResult, 5);
}

/**
 * @tc.name: GestureEventHubTest011
 * @tc.desc: Test IsAccessibilityClickable and IsAccessibiityLongClickable
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    ASSERT_NE(gestureEventHub, nullptr);

    /**
     * @tc.steps: step2. gestureHierarchy_ has ClickRecognizer, the number of fingers is two or click count is two
     * @tc.expected: IsAccessibilityClickable is false
     */
    EXPECT_FALSE(gestureEventHub->IsAccessibilityClickable());

    auto clickRecognizer = AceType::MakeRefPtr<ClickRecognizer>(DOUBLE_FINGERS, 1);
    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer);
    EXPECT_FALSE(gestureEventHub->IsAccessibilityClickable());
    gestureEventHub->gestureHierarchy_.clear();
    clickRecognizer = AceType::MakeRefPtr<ClickRecognizer>(1, CLICK_COUNTS);
    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer);
    EXPECT_FALSE(gestureEventHub->IsAccessibilityClickable());
    gestureEventHub->gestureHierarchy_.clear();

    /**
     * @tc.steps: step3. gestureHierarchy_ has ClickRecognizer, the number of fingers is one
     * @tc.expected: IsAccessibilityClickable is true
     */
    clickRecognizer = AceType::MakeRefPtr<ClickRecognizer>(FINGERS, 1);
    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer);
    EXPECT_TRUE(gestureEventHub->IsAccessibilityClickable());
    gestureEventHub->gestureHierarchy_.clear();

    /**
     * @tc.steps: step4. call AddClickEvent
     * @tc.expected: IsAccessibilityClickable is true
     */
    auto clickCallback = [](GestureEvent& info) {};
    auto clickEvent = AceType::MakeRefPtr<ClickEvent>(std::move(clickCallback));
    gestureEventHub->AddClickEvent(clickEvent);
    EXPECT_TRUE(gestureEventHub->IsAccessibilityClickable());

    /**
     * @tc.steps: step5. gestureHierarchy_ has LongPressRecognizer, the number of fingers is two
     * @tc.expected: IsAccessibilityLongClickable is false
     */
    EXPECT_FALSE(gestureEventHub->IsAccessibilityLongClickable());

    auto longPressRecognizer = AceType::MakeRefPtr<LongPressRecognizer>(1, DOUBLE_FINGERS, false);
    gestureEventHub->gestureHierarchy_.emplace_back(longPressRecognizer);
    EXPECT_FALSE(gestureEventHub->IsAccessibilityLongClickable());
    gestureEventHub->gestureHierarchy_.clear();

    /**
     * @tc.steps: step6. gestureHierarchy_ has LongPressRecognizer, the number of fingers is one
     * @tc.expected: IsAccessibilityLongClickable is false
     */
    longPressRecognizer = AceType::MakeRefPtr<LongPressRecognizer>(1, 1, false);
    gestureEventHub->gestureHierarchy_.emplace_back(longPressRecognizer);
    EXPECT_TRUE(gestureEventHub->IsAccessibilityLongClickable());
    gestureEventHub->gestureHierarchy_.clear();

    /**
     * @tc.steps: step7. call SetLongPressEvent
     * @tc.expected: IsAccessibilityLongClickable is true
     */
    auto longPressCallback = [](GestureEvent& info) {};
    auto longPressEvent = AceType::MakeRefPtr<LongPressEvent>(longPressCallback);
    gestureEventHub->SetLongPressEvent(longPressEvent);
    EXPECT_TRUE(gestureEventHub->IsAccessibilityLongClickable());
}

/**
 * @tc.name: GestureEventHubTest012
 * @tc.desc: Test UpdateGestureHierarchy
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    ASSERT_NE(gestureEventHub, nullptr);

    /**
     * @tc.steps: step2. call OnModifyDone
     * @tc.expected: gestureHierarchy_ has two elements
     */
    gestureEventHub->recreateGesture_ = true;
    auto tapGesture = AceType::MakeRefPtr<TapGesture>(FINGERS, 1);
    gestureEventHub->gestures_.emplace_back(tapGesture);
    auto longPressGesture = AceType::MakeRefPtr<LongPressGesture>(FINGERS, false, 1);
    gestureEventHub->gestures_.emplace_back(longPressGesture);
    auto onAccessibilityEvent = gestureEventHub->GetOnAccessibilityEventFunc();
    ASSERT_NE(onAccessibilityEvent, nullptr);
    onAccessibilityEvent(AccessibilityEventType::CLICK);
    gestureEventHub->OnModifyDone();

    auto sizeGestureHierarchy = static_cast<int32_t>(gestureEventHub->gestureHierarchy_.size());
    EXPECT_EQ(sizeGestureHierarchy, GESTURES_COUNTS);
}

/**
 * @tc.name: GestureEventHubTest013
 * @tc.desc: Test ProcessTouchTestHit
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto frameNode = FrameNode::CreateFrameNode("myButton", 100, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(guestureEventHub, nullptr);
    OffsetF coordinateOffset;
    TouchRestrict touchRestrict;
    TouchTestResult innerTargets;
    TouchTestResult finalResult;
    PointF localPoint;

    PanDirection panDirection;
    /**
     * @tc.steps: step2. call ProcessTouchTestHit
     * @tc.expected: result is true
     */
    guestureEventHub->scrollableActuator_ =
        AceType::MakeRefPtr<ScrollableActuator>(AceType::WeakClaim(AceType::RawPtr(guestureEventHub)));
    guestureEventHub->touchEventActuator_ = AceType::MakeRefPtr<TouchEventActuator>();
    guestureEventHub->clickEventActuator_ =
        AceType::MakeRefPtr<ClickEventActuator>(AceType::WeakClaim(AceType::RawPtr(guestureEventHub)));
    guestureEventHub->panEventActuator_ = AceType::MakeRefPtr<PanEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(guestureEventHub)), panDirection, 1, 50.0f);
    guestureEventHub->longPressEventActuator_ =
        AceType::MakeRefPtr<LongPressEventActuator>(AceType::WeakClaim(AceType::RawPtr(guestureEventHub)));
    guestureEventHub->dragEventActuator_ = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(guestureEventHub)), panDirection, 1, 50.0f);
    auto result = guestureEventHub->ProcessTouchTestHit(
        coordinateOffset, touchRestrict, innerTargets, finalResult, 2, localPoint, nullptr);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GestureEventHubTest014
 * @tc.desc: Test IsAllowedDrag
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto frameNode = FrameNode::CreateFrameNode("myButton", 101, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(guestureEventHub, nullptr);

    auto eventHub = guestureEventHub->eventHub_.Upgrade();

    auto event = guestureEventHub->eventHub_.Upgrade();
    event->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));
    auto result = guestureEventHub->IsAllowedDrag(eventHub);
    ASSERT_FALSE(result);
    /**
     * @tc.steps: step2. call IsAllowedDrag
     * @tc.expected: result is correct
     */
    frameNode->userSet_ = true;
    result = guestureEventHub->IsAllowedDrag(eventHub);
    ASSERT_FALSE(result);

    frameNode->userSet_ = false;
    auto func = [](const RefPtr<OHOS::Ace::DragEvent>&, const std::string&) { return DragDropInfo(); };
    eventHub->onDragStart_ = func;
    result = guestureEventHub->IsAllowedDrag(eventHub);
    ASSERT_TRUE(result);

    guestureEventHub->HandleOnDragStart(GestureEvent());

    frameNode->draggable_ = true;
    result = guestureEventHub->IsAllowedDrag(eventHub);
    ASSERT_TRUE(result);

    frameNode->draggable_ = true;
    eventHub->onDragStart_ = nullptr;
    result = guestureEventHub->IsAllowedDrag(eventHub);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: GestureEventHubTest015
 * @tc.desc: Test StartDragTaskForWeb HandleNotallowDrag
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest015, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto frameNode = FrameNode::CreateFrameNode("myButton", 102, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(guestureEventHub, nullptr);

    auto event = guestureEventHub->eventHub_.Upgrade();
    event->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));

    guestureEventHub->StartDragTaskForWeb();

    guestureEventHub->isReceivedDragGestureInfo_ = true;
    guestureEventHub->StartDragTaskForWeb();
    ASSERT_FALSE(guestureEventHub->isReceivedDragGestureInfo_);

    guestureEventHub->HandleNotallowDrag(GestureEvent());

    frameNode = FrameNode::CreateFrameNode("Web", 102, AceType::MakeRefPtr<Pattern>());
    guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    event = guestureEventHub->eventHub_.Upgrade();
    event->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));

    frameNode->userSet_ = false;
    auto func = [](const RefPtr<OHOS::Ace::DragEvent>&, const std::string&) { return DragDropInfo(); };
    event->onDragStart_ = func;
    guestureEventHub->HandleOnDragStart(GestureEvent());

    guestureEventHub->HandleNotallowDrag(GestureEvent());
    ASSERT_TRUE(guestureEventHub->isReceivedDragGestureInfo_);
}

/**
 * @tc.name: GestureEventHubTest016
 * @tc.desc: Test BindMenu
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest016, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto frameNode = FrameNode::CreateFrameNode("myButton", 102, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(guestureEventHub, nullptr);

    auto eventHub = guestureEventHub->eventHub_.Upgrade();
    eventHub->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));

    auto pipline = PipelineContext::GetCurrentContext();
    auto func = [](GestureEvent& info) {};
    guestureEventHub->BindMenu(func);

    guestureEventHub->showMenu_ = AceType::MakeRefPtr<ClickEvent>([](GestureEvent& info) {});
    guestureEventHub->BindMenu(func);

    guestureEventHub->clickEventActuator_ = nullptr;
    guestureEventHub->ClearUserOnClick();
    guestureEventHub->ClearUserOnTouch();

    guestureEventHub->clickEventActuator_ =
        AceType::MakeRefPtr<ClickEventActuator>(AceType::WeakClaim(AceType::RawPtr(guestureEventHub)));
    guestureEventHub->touchEventActuator_ = AceType::MakeRefPtr<TouchEventActuator>();
    guestureEventHub->ClearUserOnClick();
    guestureEventHub->ClearUserOnTouch();
    ASSERT_FALSE(guestureEventHub->clickEventActuator_->userCallback_);
}

/**
 * @tc.name: GestureEventHubTest017
 * @tc.desc: Test ProcessTouchTestHit
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest017, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto frameNode = FrameNode::CreateFrameNode("myButton", 100, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(guestureEventHub, nullptr);
    OffsetF coordinateOffset;
    TouchRestrict touchRestrict;
    TouchTestResult innerTargets;
    TouchTestResult finalResult;
    PointF localPoint;

    PanDirection panDirection;
    guestureEventHub->panEventActuator_ = AceType::MakeRefPtr<PanEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(guestureEventHub)), panDirection, 1, 50.0f);
    /**
     * @tc.steps: step2. call ProcessTouchTestHit , recognizer is not instance of recognizer group
     * @tc.expected: result is false
     */
    guestureEventHub->longPressEventActuator_ =
        AceType::MakeRefPtr<LongPressEventActuator>(AceType::WeakClaim(AceType::RawPtr(guestureEventHub)));
    guestureEventHub->dragEventActuator_ = nullptr;

    auto result = guestureEventHub->ProcessTouchTestHit(
        coordinateOffset, touchRestrict, innerTargets, finalResult, 2, localPoint, nullptr);
    EXPECT_FALSE(result);
    /**
     * @tc.steps: step3. call ProcessTouchTestHit , recognizer is instance of recognizer group.
     * @tc.expected: result is false
     */
    GestureEventFunc callback = [](GestureEvent& info) {};
    auto longPressEvent = AceType::MakeRefPtr<LongPressEvent>(std::move(callback));
    guestureEventHub->longPressEventActuator_->SetLongPressEvent(longPressEvent);

    guestureEventHub->dragEventActuator_ = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(guestureEventHub)), panDirection, 1, 50.0f);

    auto dragActionStart = [](GestureEvent& info) {};
    auto dragActionUpdate = [](GestureEvent& info) {};
    auto dragActionEnd = [](GestureEvent& info) {};
    auto dragActionCancel = []() {};
    auto dragEvent = AceType::MakeRefPtr<DragEvent>(
        std::move(dragActionStart), std::move(dragActionUpdate), std::move(dragActionEnd), std::move(dragActionCancel));
    guestureEventHub->dragEventActuator_->userCallback_ = dragEvent;
    result = guestureEventHub->ProcessTouchTestHit(
        coordinateOffset, touchRestrict, innerTargets, finalResult, 2, localPoint, nullptr);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GestureEventHubTest018
 * @tc.desc: Test HandleOnDragStart
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest018, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. update the input parameter attributes.
     */
    RefPtr<UINode> customNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    auto onDragStart = [&customNode](
                           const RefPtr<OHOS::Ace::DragEvent>& /* dragEvent */, const std::string& /* param */) {
        DragDropInfo dragDropInfo;
        dragDropInfo.customNode = customNode;
        return dragDropInfo;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));

    auto retFlag = gestureEventHub->IsAllowedDrag(eventHub);
    EXPECT_TRUE(retFlag);

    auto eventManager = AceType::MakeRefPtr<EventManager>();
    EXPECT_TRUE(eventManager);
    eventManager->SetLastMoveBeforeUp(true);

    auto pipeline = PipelineContext::GetCurrentContext();
    pipeline->SetEventManager(eventManager);

    GestureEvent gestureEvent;
    gestureEvent.SetInputEventType(InputEventType::MOUSE_BUTTON);
    gestureEventHub->HandleOnDragStart(gestureEvent);
    EXPECT_TRUE(pipeline->GetEventManager());
    eventManager->SetLastMoveBeforeUp(false);
    gestureEvent.SetInputEventType(InputEventType::TOUCH_SCREEN);

    /**
     * @tc.steps: step3. call HandleOnDragStart with SourceType::MOUSE_BUTTON and InputEventType::TOUCH_SCREEN.
     * @tc.expected: eventManager->IsLastMoveBeforeUp() the return value is false.
     */
    gestureEventHub->HandleOnDragStart(gestureEvent);
    EXPECT_FALSE(eventManager->IsLastMoveBeforeUp());
}

/**
 * @tc.name: GestureEventHubTest0181
 * @tc.desc: Test HandleOnDragStart
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest0181, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::WEB_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());

    /**
     * @tc.steps: step2. update the input parameter attributes.
     */
    RefPtr<UINode> customNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    DragDropInfo dragDropInfo;
    dragDropInfo.customNode = customNode;
    frameNode->SetDragPreview(dragDropInfo);

    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    auto onDragStart = [&customNode](
                           const RefPtr<OHOS::Ace::DragEvent>& /* dragEvent */, const std::string& /* param */) {
        DragDropInfo dragDropInfo;
        dragDropInfo.customNode = customNode;
        return dragDropInfo;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));

    auto eventManager = AceType::MakeRefPtr<EventManager>();
    EXPECT_TRUE(eventManager);

    auto pipeline = PipelineContext::GetCurrentContext();
    pipeline->SetEventManager(eventManager);
    eventManager->SetLastMoveBeforeUp(false);

    /**
     * @tc.steps: step3. call HandleOnDragStart with SourceType::MOUSE and InputEventType::TOUCH_SCREEN.
     * @tc.expected: eventManager->IsLastMoveBeforeUp() the return value is false.
     */
    GestureEvent gestureEvent;
    gestureEvent.SetSourceDevice(SourceType::MOUSE);
    gestureEvent.SetInputEventType(InputEventType::TOUCH_SCREEN);
    gestureEventHub->HandleOnDragStart(gestureEvent);
    EXPECT_FALSE(eventManager->IsLastMoveBeforeUp());
}

/**
 * @tc.name: GestureEventHubTest0183
 * @tc.desc: Test HandleOnDragStart
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest0183, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXTINPUT_ETS_TAG, -1, AceType::MakeRefPtr<TextPattern>());

    /**
     * @tc.steps: step2. update the input parameter attributes.
     */
    void* voidPtr = static_cast<void*>(new char[0]);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    RefPtr<UINode> customNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    DragDropInfo dragDropInfo;
    dragDropInfo.customNode = customNode;
    dragDropInfo.pixelMap = pixelMap;
    frameNode->SetDragPreview(dragDropInfo);

    auto hostPattern = frameNode->GetPattern<TextDragBase>();
    EXPECT_TRUE(hostPattern);

    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    auto onDragStart = [&customNode](
                           const RefPtr<OHOS::Ace::DragEvent>& /* dragEvent */, const std::string& /* param */) {
        DragDropInfo dragDropInfo;
        dragDropInfo.customNode = customNode;
        return dragDropInfo;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));

    auto eventManager = AceType::MakeRefPtr<EventManager>();
    EXPECT_TRUE(eventManager);

    auto pipeline = PipelineContext::GetCurrentContext();
    pipeline->SetEventManager(eventManager);
    eventManager->SetLastMoveBeforeUp(false);

    /**
     * @tc.steps: step3. call HandleOnDragStart with SourceType::NONE and InputEventType::TOUCH_SCREEN.
     * @tc.expected: eventManager->IsLastMoveBeforeUp() the return value is false.
     */
    GestureEvent gestureEvent;
    gestureEvent.SetSourceDevice(SourceType::NONE);
    gestureEvent.SetInputEventType(InputEventType::TOUCH_SCREEN);
    gestureEventHub->HandleOnDragStart(gestureEvent);
    EXPECT_FALSE(eventManager->IsLastMoveBeforeUp());
}

/**
 * @tc.name: GestureEventHubTest0184
 * @tc.desc: Test HandleOnDragStart
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest0184, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::SEARCH_Field_ETS_TAG, -1, AceType::MakeRefPtr<TextPattern>());

    /**
     * @tc.steps: step2. update the input parameter attributes.
     */
    DragDropInfo dragDropInfo;
    dragDropInfo.customNode = nullptr;
    dragDropInfo.pixelMap = nullptr;
    frameNode->SetDragPreview(dragDropInfo);

    auto hostPattern = frameNode->GetPattern<TextDragBase>();
    EXPECT_TRUE(hostPattern);

    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    RefPtr<UINode> customNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    auto onDragStart = [&customNode](
                           const RefPtr<OHOS::Ace::DragEvent>& /* dragEvent */, const std::string& /* param */) {
        DragDropInfo dragDropInfo;
        dragDropInfo.customNode = customNode;
        return dragDropInfo;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));

    auto eventManager = AceType::MakeRefPtr<EventManager>();
    EXPECT_TRUE(eventManager);
    auto pipeline = PipelineContext::GetCurrentContext();
    pipeline->SetEventManager(eventManager);
    eventManager->SetLastMoveBeforeUp(false);

    /**
     * @tc.steps: step3. call HandleOnDragStart with SourceType::MOUSE and InputEventType::TOUCH_SCREEN.
     * @tc.expected: eventManager->IsLastMoveBeforeUp() the return value is false.
     */
    GestureEvent gestureEvent;
    gestureEvent.SetSourceDevice(SourceType::MOUSE);
    gestureEvent.SetInputEventType(InputEventType::TOUCH_SCREEN);
    gestureEventHub->HandleOnDragStart(gestureEvent);
    EXPECT_FALSE(eventManager->IsLastMoveBeforeUp());
}

/**
 * @tc.name: GestureEventHubTest0185
 * @tc.desc: Test HandleOnDragStart
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest0185, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<TextPattern>());

    /**
     * @tc.steps: step2. Build PixelMap and UINode, and update the properties of DragDropInfo.
     */
    void* voidPtr = static_cast<void*>(new char[0]);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    RefPtr<UINode> customNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    DragDropInfo dragDropInfo;
    dragDropInfo.customNode = customNode;
    dragDropInfo.pixelMap = pixelMap;
    frameNode->SetDragPreview(dragDropInfo);

    auto hostPattern = frameNode->GetPattern<TextDragBase>();
    EXPECT_TRUE(hostPattern);

    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    auto onDragStart = [&customNode](
                           const RefPtr<OHOS::Ace::DragEvent>& /* dragEvent */, const std::string& /* param */) {
        DragDropInfo dragDropInfo;
        dragDropInfo.customNode = customNode;
        return dragDropInfo;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));

    auto eventManager = AceType::MakeRefPtr<EventManager>();
    EXPECT_TRUE(eventManager);
    auto pipeline = PipelineContext::GetCurrentContext();
    pipeline->SetEventManager(eventManager);
    eventManager->SetLastMoveBeforeUp(false);

    /**
     * @tc.steps: step3. call HandleOnDragStart with SourceType::NONE and InputEventType::TOUCH_SCREEN.
     * @tc.expected: eventManager->IsLastMoveBeforeUp() the return value is false.
     */
    GestureEvent gestureEvent;
    gestureEvent.SetSourceDevice(SourceType::NONE);
    gestureEvent.SetInputEventType(InputEventType::TOUCH_SCREEN);
    gestureEventHub->HandleOnDragStart(gestureEvent);
    EXPECT_FALSE(eventManager->IsLastMoveBeforeUp());
}

/**
 * @tc.name: GestureEventHubTest019
 * @tc.desc: Test KeyBoardShortCutClick
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest019, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. construct a clickEventActuator and update gestureEventHub attributes.
     */
    auto clickEventActuator = AceType::MakeRefPtr<ClickEventActuator>(WeakPtr<NG::GestureEventHub>(gestureEventHub));
    gestureEventHub->clickEventActuator_ = clickEventActuator;

    /**
     * @tc.steps: step3. call KeyBoardShortCutClick with event and frameNode.
     * @tc.expected: retFlag is false.
     */
    KeyEvent event;
    bool retFlag = gestureEventHub->KeyBoardShortCutClick(event, WeakPtr<NG::FrameNode>(frameNode));
    EXPECT_TRUE(retFlag);
}

/**
 * @tc.name: GestureEventHubTest020
 * @tc.desc: taskScheduler of Test StartDragTaskForWeb were true branches
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest020, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto frameNode = FrameNode::CreateFrameNode("myButton", 102, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(guestureEventHub, nullptr);

    /**
     * @tc.steps: step2. updates event and pipeline attributes.
     */
    auto event = guestureEventHub->eventHub_.Upgrade();
    event->host_ = AceType::WeakClaim(AceType::RawPtr(frameNode));

    auto pipeline = PipelineContext::GetCurrentContext();
    EXPECT_TRUE(pipeline);

    /**
     * @tc.steps: step3. call StartDragTaskForWeb.
     * @tc.expected: taskScheduler is true.
     */
    guestureEventHub->isReceivedDragGestureInfo_ = true;
    guestureEventHub->StartDragTaskForWeb();
    auto taskScheduler = pipeline->GetTaskExecutor();
    EXPECT_FALSE(taskScheduler);
}

/**
 * @tc.name: GestureEventHubTest021
 * @tc.desc: Test StartLongPressActionForWeb
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest021, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. construct a PipelineContext and update pipeline attributes.
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    EXPECT_TRUE(pipeline);

    /**
     * @tc.steps: step3. call StartLongPressActionForWeb.
     * @tc.expected: taskScheduler is false.
     */
    gestureEventHub->StartLongPressActionForWeb();
    auto taskScheduler = pipeline->GetTaskExecutor();
    EXPECT_FALSE(taskScheduler);
}

/**
 * @tc.name: GestureEventHubTest022
 * @tc.desc: Test HandleOnDragUpdate
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest022, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. construct a dragDropProxy and update gestureEventHub attributes.
     */
    constexpr int64_t PROXY_ID_NOT_FIT = 101;
    auto dragDropProxy = AceType::MakeRefPtr<DragDropProxy>(PROXY_ID_NOT_FIT);
    gestureEventHub->dragDropProxy_ = dragDropProxy;

    /**
     * @tc.steps: step3. call HandleOnDragUpdate with GestureEvent.
     * @tc.expected: gestureEventHub->dragDropProxy_ is true.
     */
    GestureEvent info;
    gestureEventHub->HandleOnDragUpdate(info);
    EXPECT_TRUE(gestureEventHub->dragDropProxy_);

    /**
     * @tc.steps: step4. call HandleOnDragUpdate with GestureEvent.
     * @tc.expected: gestureEventHub->IsDragged() the return value is false.
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    auto dragDropManager = pipeline->GetDragDropManager();
    dragDropManager->SetIsDragged(false);
    gestureEventHub->HandleOnDragUpdate(info);
    EXPECT_FALSE(dragDropManager->IsDragged());
}

/**
 * @tc.name: GestureEventHubTest024
 * @tc.desc: Test HandleOnDragEnd
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest024, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);

    /**
     * @tc.steps: step2. construct a func and update eventHub attributes.
     */
    auto func = [](const RefPtr<OHOS::Ace::DragEvent>&, const std::string&) { return DragDropInfo(); };
    eventHub->SetOnDrop(func);
    eventHub->customerOnDrop_ = func;

    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    auto pipeline = PipelineContext::GetCurrentContext();
    pipeline->SetMinPlatformVersion(12);

    /**
     * @tc.steps: step3. call HandleOnDragEnd with GestureEvent.
     * @tc.expected: The return value of gestureEventHub->GetFrameNode() is true.
     */
    GestureEvent info;
    gestureEventHub->HandleOnDragEnd(info);
    EXPECT_TRUE(gestureEventHub->GetFrameNode());

    constexpr int64_t PROXY_ID_NOT_FIT = 101;
    auto dragDropProxy = AceType::MakeRefPtr<DragDropProxy>(PROXY_ID_NOT_FIT);
    gestureEventHub->dragDropProxy_ = dragDropProxy;
    pipeline->SetMinPlatformVersion(9);

    /**
     * @tc.steps: step4. call HandleOnDragEnd with GestureEvent.
     * @tc.expected: The return value of gestureEventHub->dragDropProxy_ is false.
     */
    gestureEventHub->HandleOnDragEnd(info);
    EXPECT_FALSE(gestureEventHub->dragDropProxy_);
}

/**
 * @tc.name: GestureEventHubTest025
 * @tc.desc: Test HandleOnDragEnd
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest025, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::WEB_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);

    /**
     * @tc.steps: step2. construct a func and update eventHub attributes.
     */
    auto func = [](const RefPtr<OHOS::Ace::DragEvent>&, const std::string&) { return DragDropInfo(); };
    eventHub->SetOnDrop(func);
    eventHub->customerOnDrop_ = func;

    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    auto pipeline = PipelineContext::GetCurrentContext();
    pipeline->SetMinPlatformVersion(9);

    constexpr int64_t PROXY_ID_NOT_FIT = 101;
    auto dragDropProxy = AceType::MakeRefPtr<DragDropProxy>(PROXY_ID_NOT_FIT);
    gestureEventHub->dragDropProxy_ = dragDropProxy;

    /**
     * @tc.steps: step3. call HandleOnDragEnd with GestureEvent.
     * @tc.expected: gestureEventHub->dragDropProxy_ is null.
     */
    GestureEvent info;
    gestureEventHub->HandleOnDragEnd(info);
    EXPECT_FALSE(gestureEventHub->dragDropProxy_);

    eventHub->onDrop_ = nullptr;
    eventHub->customerOnDrop_ = nullptr;
    gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);

    /**
     * @tc.steps: step4. call HandleOnDragEnd with GestureEvent.
     * @tc.expected: The return value of eventHub->HasOnDrop() is false.
     */
    gestureEventHub->HandleOnDragEnd(info);
    EXPECT_FALSE(eventHub->HasOnDrop());
}

/**
 * @tc.name: GestureEventHubTest026
 * @tc.desc: Test HandleOnDragCancel
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest026, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. construct a dragDropProxy and update gestureEventHub attribute.
     */
    constexpr int64_t PROXY_ID_NOT_FIT = 101;
    auto dragDropProxy = AceType::MakeRefPtr<DragDropProxy>(PROXY_ID_NOT_FIT);
    gestureEventHub->dragDropProxy_ = dragDropProxy;

    /**
     * @tc.steps: step3. call HandleOnDragCancel.
     * @tc.expected: gestureEventHub->dragDropProxy_ is null.
     */
    gestureEventHub->HandleOnDragCancel();
    EXPECT_FALSE(gestureEventHub->dragDropProxy_);
}

/**
 * @tc.name: GestureEventHubTest027
 * @tc.desc: Test SetOnGestureJudgeBegin and SetOnGestureJudgeNativeBegin
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest027, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. construct a gestureJudgeFunc.
     */
    auto gestureJudgeFunc = [](const RefPtr<GestureInfo>& gestureInfo,
                                const std::shared_ptr<BaseGestureEvent>&) -> GestureJudgeResult {
        return GestureJudgeResult(1);
    };

    /**
     * @tc.steps: step3. call SetOnGestureJudgeBegin with gestureJudgeFunc.
     * @tc.expected: gestureEventHub->gestureJudgeFunc_ is not null.
     */
    gestureEventHub->SetOnGestureJudgeBegin(gestureJudgeFunc);
    EXPECT_TRUE(gestureEventHub->gestureJudgeFunc_);

    /**
     * @tc.steps: step4. call SetOnGestureJudgeNativeBegin with gestureJudgeFunc.
     * @tc.expected: gestureEventHub->gestureJudgeNativeFunc_ is not null.
     */
    gestureEventHub->SetOnGestureJudgeNativeBegin(gestureJudgeFunc);
    EXPECT_TRUE(gestureEventHub->gestureJudgeNativeFunc_);
}

/**
 * @tc.name: GestureEventHubTest028
 * @tc.desc: Test IsAccessibilityClickable and IsAccessibilityLongClickable
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest028, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. Updated properties for gestureEventHub.
     */
    auto clickRecognizer = AceType::MakeRefPtr<ClickRecognizer>(DOUBLE_FINGERS, FINGERS);
    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer);

    /**
     * @tc.steps: step3. call IsAccessibilityClickable.
     * @tc.expected: retFlag is false.
     */
    bool retFlag = gestureEventHub->IsAccessibilityClickable();
    EXPECT_FALSE(retFlag);

    /**
     * @tc.steps: step3. call IsAccessibilityLongClickable.
     * @tc.expected: retFlag is false.
     */
    retFlag = gestureEventHub->IsAccessibilityLongClickable();
    EXPECT_FALSE(retFlag);
}

/**
 * @tc.name: GestureEventHubTest029
 * @tc.desc: Test GetHitTestModeStr
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest029, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. call GetHitTestModeStr with hitTestMode_ == -1.
     * @tc.expected: retStr is equal to "HitTestMode.Default".
     */
    gestureEventHub->SetHitTestMode(HitTestMode(-1));
    std::string retStr = gestureEventHub->GetHitTestModeStr();
    EXPECT_EQ(retStr, "HitTestMode.Default");

    /**
     * @tc.steps: step3. call GetHitTestModeStr with hitTestMode_ == 4.
     * @tc.expected: retStr is equal to "HitTestMode.Default".
     */
    gestureEventHub->SetHitTestMode(HitTestMode(4));
    retStr = gestureEventHub->GetHitTestModeStr();
    EXPECT_EQ(retStr, "HitTestMode.Default");
}

/**
 * @tc.name: GestureEventHubTest030
 * @tc.desc: Test ActClick
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest030, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. Updated properties for gestureEventHub.
     */
    auto clickRecognizer = AceType::MakeRefPtr<ClickRecognizer>(DOUBLE_FINGERS, CLICK_COUNTS);
    gestureEventHub->gestureHierarchy_.emplace_back(clickRecognizer);

    /**
     * @tc.steps: step3. call ActClick.
     * @tc.expected: retFlag is false.
     */
    bool retFlag = gestureEventHub->ActClick();
    EXPECT_FALSE(retFlag);
}

/**
 * @tc.name: GestureEventHubTest031
 * @tc.desc: Test UpdateGestureHierarchy
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest031, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    ASSERT_NE(gestureEventHub, nullptr);

    /**
     * @tc.steps: step2. Updated properties for gestureEventHub.
     */
    gestureEventHub->recreateGesture_ = true;
    auto tapGesture = AceType::MakeRefPtr<TapGesture>(FINGERS, 1);
    gestureEventHub->gestures_.emplace_back(tapGesture);
    auto longPressRecognizer = AceType::MakeRefPtr<LongPressRecognizer>(1, 1, false);
    gestureEventHub->gestureHierarchy_.emplace_back(longPressRecognizer);
    gestureEventHub->UpdateGestureHierarchy();

    /**
     * @tc.steps: step3. call UpdateGestureHierarchy.
     * @tc.expected: gestureHierarchy_ has one elements.
     */
    auto sizeGestureHierarchy = static_cast<int32_t>(gestureEventHub->gestureHierarchy_.size());
    EXPECT_EQ(sizeGestureHierarchy, 1);
}

/**
 * @tc.name: ResetDragActionForWeb001
 * @tc.desc: Test ResetDragActionForWeb
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, ResetDragActionForWeb001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto frameNode = FrameNode::CreateFrameNode("myButton", 102, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(guestureEventHub, nullptr);

    /**
     * @tc.steps: step1. Calling the ResetDragActionForWeb interface
     * @tc.expected: IsReceivedDragGestureInfo_ Equal to false.
     */
    guestureEventHub->ResetDragActionForWeb();
    ASSERT_EQ(guestureEventHub->isReceivedDragGestureInfo_, false);
}

/**
 * @tc.name: ResetDragActionForWeb001
 * @tc.desc: Test ResetDragActionForWeb
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, OnDragStart001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto frameNode = FrameNode::CreateFrameNode("myButton", 101, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(guestureEventHub, nullptr);
    auto eventHub = guestureEventHub->eventHub_.Upgrade();

    GestureEvent info;
    auto pipline = PipelineContext::GetCurrentContext();

    auto EventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(EventHub);

    auto frameNodeOfEvent = EventHub->GetFrameNode();
    EXPECT_TRUE(frameNodeOfEvent);
    RefPtr<OHOS::Ace::DragEvent> event = AceType::MakeRefPtr<OHOS::Ace::DragEvent>();

    RefPtr<UINode> customNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    DragDropInfo dragDropInfo;
    dragDropInfo.customNode = customNode;

    /**
     * @tc.steps: step1. Calling the ResetDragActionForWeb interface
     * @tc.expected: dragDropProxy_ Equal to false.
     */
    guestureEventHub->OnDragStart(info, pipline, frameNode, dragDropInfo, event);
    EXPECT_TRUE(EventHub->dragDropProxy_ == false);
}

/**
 * @tc.name: GetHitTestModeStr
 * @tc.desc: Test GetHitTestModeStr001
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GetHitTestModeStr001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto frameNode = FrameNode::CreateFrameNode("myButton", 101, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(guestureEventHub, nullptr);
    auto eventHub = guestureEventHub->eventHub_.Upgrade();
    auto pipline = PipelineContext::GetCurrentContext();
    auto EventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EventHub->CancelDragForWeb();
    string testModeStr;
    /**
     * @tc.steps: step1. Calling the GetHitTestModeStr interface
     * @tc.expected: EventHub ->GetHitTestModeStr() is not equal to nullptr
     */
    EXPECT_TRUE(testModeStr != EventHub->GetHitTestModeStr());
}

/**
 * @tc.name: HandleOnDragUpdate001
 * @tc.desc: Test Functions related with drag
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, HandleOnDragUpdate001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create guestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto frameNode = FrameNode::CreateFrameNode("MyButton", 102, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(guestureEventHub, nullptr);
    auto eventHub = guestureEventHub->eventHub_.Upgrade();
    eventHub->AttachHost(frameNode);
    ASSERT_NE(eventHub, nullptr);

    auto pipline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipline, nullptr);

    /**
     * @tc.steps: step2. call HandleOnDragStart
     *            case: eventHub->HasOnDragStart() is null
     * @tc.expected: dragDropProxy_ is null.
     */
    GestureEvent info;
    info.SetSourceDevice(SourceType::MOUSE);
    guestureEventHub->HandleOnDragStart(info);
    EXPECT_EQ(guestureEventHub->dragDropProxy_, nullptr);
    /**
     * @tc.steps: step3. set OnDragStart for eventHub
     *            after that eventHub->HasOnDragStart() is not null
     *            case: dragDropInfo.customNode is not null
     */
    RefPtr<UINode> customNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    auto onDragStart = [&customNode](const RefPtr<OHOS::Ace::DragEvent>&, const std::string&) {
        DragDropInfo dragDropInfo;
        dragDropInfo.customNode = customNode;
        return dragDropInfo;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));

    /**
     * @tc.steps: step4. call HandleOnDragStart
     *            case: dragDropInfo.customNode is not null
     * @tc.expected: dragDropProxy_ is null.
     */
    guestureEventHub->HandleOnDragStart(info);
    EXPECT_EQ(guestureEventHub->dragDropProxy_, nullptr);

    /**
     * @tc.steps: step5. call HandleOnDragEnd
     *            case: eventHub->HasOnDrop() is true
     * @tc.expected: dragDropProxy_ is null
     *               onDrop has been called, msg1 = CHECK_TAG_1
     */
    std::string msg1;
    auto onDrop = [&msg1](const RefPtr<OHOS::Ace::DragEvent>& /* dragEvent */, const std::string& /* param */) {
        msg1 = CHECK_TAG_1;
    };
    eventHub->SetOnDrop(std::move(onDrop));
    guestureEventHub->HandleOnDragStart(info);
    guestureEventHub->HandleOnDragEnd(info);
    EXPECT_EQ(guestureEventHub->dragDropProxy_, nullptr);
    EXPECT_EQ(msg1, "HELLO");
}

/**
 * @tc.name: HandleOnDragUpdate002
 * @tc.desc: Test Functions related with drag
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, HandleOnDragUpdate002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::RICH_EDITOR_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());

    /**
     * @tc.steps: step2. update the input parameter attributes.
     */
    RefPtr<UINode> customNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    DragDropInfo dragDropInfo;
    dragDropInfo.customNode = customNode;
    frameNode->SetDragPreview(dragDropInfo);

    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    ASSERT_NE(gestureEventHub, nullptr);

    auto onDragStart = [&customNode](const RefPtr<OHOS::Ace::DragEvent>&, const std::string&) {
        DragDropInfo dragDropInfo;
        dragDropInfo.customNode = customNode;
        return dragDropInfo;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));

    auto eventManager = AceType::MakeRefPtr<EventManager>();
    ASSERT_NE(eventManager, nullptr);

    auto pipeline = PipelineContext::GetCurrentContext();
    pipeline->SetEventManager(eventManager);
    eventManager->SetLastMoveBeforeUp(false);

    /**
     * @tc.steps: step3. call HandleOnDragStart with SourceType::MOUSE and InputEventType::TOUCH_SCREEN.
     * @tc.expected: eventManager->IsLastMoveBeforeUp() the return value is false.
     */
    GestureEvent gestureEvent;
    gestureEvent.SetSourceDevice(SourceType::MOUSE);
    gestureEvent.SetInputEventType(InputEventType::MOUSE_BUTTON);
    gestureEventHub->HandleOnDragStart(gestureEvent);
    EXPECT_FALSE(eventManager->IsLastMoveBeforeUp());
}

/**
 * @tc.name: HandleOnDragUpdate003
 * @tc.desc: Test Functions related with drag
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, HandleOnDragUpdate003, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::WEB_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(guestureEventHub, nullptr);
    auto eventHub = guestureEventHub->eventHub_.Upgrade();
    eventHub->AttachHost(frameNode);
    ASSERT_NE(eventHub, nullptr);

    auto pipline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipline, nullptr);

    pipline->SetupRootElement();
    pipline->onVisibleAreaChangeNodeIds_.clear();
    pipline->AddOnAreaChangeNode(frameNode->GetId());

    RefPtr<UINode> customNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    auto onDragStart = [&customNode](const RefPtr<OHOS::Ace::DragEvent>&, const std::string&) {
        void* voidPtr = static_cast<void*>(new char[0]);
        RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
        DragDropInfo dragDropInfo;
        dragDropInfo.customNode = customNode;
        return dragDropInfo;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));

    GestureEvent info;
    info.SetSourceDevice(SourceType::MOUSE);
    info.SetInputEventType(InputEventType::TOUCH_SCREEN);
    guestureEventHub->HandleOnDragStart(info);
    EXPECT_EQ(guestureEventHub->dragDropProxy_, nullptr);
}

/**
 * @tc.name: GestureEventHubTest032
 * @tc.desc: Test ProcessTouchTestHierarchy
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubTest032, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. call ProcessTouchTestHierarchy
     *            case: innerRecognizers & gestureHierarchy_ is empty, current is null
     * @tc.expected: finalResult is empty
     */
    TouchRestrict touchRestrict;
    std::list<RefPtr<NGGestureRecognizer>> innerTargets;
    TouchTestResult finalResult;

    /**
     * @tc.steps: step3. call externalExclusiveRecognizer_
     *            case: innerRecognizers & gestureHierarchy_ is empty, current is null
     * @tc.expected: finalResult is empty
     */
    gestureEventHub->externalExclusiveRecognizer_.push_back(nullptr);
    gestureEventHub->externalParallelRecognizer_.push_back(nullptr);
    EXPECT_EQ(static_cast<int32_t>(gestureEventHub->externalExclusiveRecognizer_.size()), 1);
    EXPECT_EQ(static_cast<int32_t>(gestureEventHub->externalParallelRecognizer_.size()), 1);

    /**
     * @tc.steps: step4. call ProcessTouchTestHierarchy
     *            case: innerRecognizers & gestureHierarchy_ is empty, current is null
     * @tc.expected: finalResult is empty
     */
    gestureEventHub->ProcessTouchTestHierarchy(
        COORDINATE_OFFSET, touchRestrict, innerTargets, finalResult, TOUCH_ID, nullptr);
    EXPECT_TRUE(finalResult.empty());
}

/**
 * @tc.name: GetPixelMapScale001
 * @tc.desc: Test GetPixelMapScale
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GetPixelMapScale001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);

    /**
     * @tc.steps: step2. construct a dragDropProxy and update gestureEventHub attributes.
     */
    constexpr int64_t PROXY_ID_NOT_FIT = 101;
    auto dragDropProxy = AceType::MakeRefPtr<DragDropProxy>(PROXY_ID_NOT_FIT);
    gestureEventHub->dragDropProxy_ = dragDropProxy;

    /**
     * @tc.steps: step3. call HandleOnDragUpdate with GestureEvent.
     * @tc.expected: gestureEventHub->dragDropProxy_ is true.
     */
    GestureEvent info;
    gestureEventHub->HandleOnDragUpdate(info);

    /**
     * @tc.steps: step4. call GetPixelMapScale.
     * @tc.expected: scale is true.
     */
    const int32_t height = 600.0f;
    const int32_t width = 500.0f;
    float scale = gestureEventHub->GetPixelMapScale(height, width);
    EXPECT_TRUE(scale);
}

/**
 * @tc.name: IsPixelMapNeedScale001
 * @tc.desc: Test IsPixelMapNeedScale
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, IsPixelMapNeedScale001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create guestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto frameNode = FrameNode::CreateFrameNode("mybutton", -1, AceType::MakeRefPtr<Pattern>());
    auto guestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(guestureEventHub, nullptr);
    auto eventHub = guestureEventHub->eventHub_.Upgrade();
    eventHub->AttachHost(frameNode);
    ASSERT_NE(eventHub, nullptr);

    auto pipline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipline, nullptr);

    void* voidPtr2 = static_cast<void*>(new char[0]);
    RefPtr<PixelMap> pixelMap2 = PixelMap::CreatePixelMap(voidPtr2);
    guestureEventHub->SetPixelMap(pixelMap2);

    bool result = guestureEventHub->IsPixelMapNeedScale();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetDragDropInfo001
 * @tc.desc: Test GetDragDropInfo function.
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GetDragDropInfo001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. set defaultOnDragStart for eventHub
     */
    auto defaultOnDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& dragEvent, const std::string& /* param */) {
        DragDropInfo dragDropInfo;
        auto unifiedData = AceType::MakeRefPtr<MockUnifiedData>();
        dragEvent->SetData(unifiedData);
        dragDropInfo.extraInfo = "default extraInfo";
        return dragDropInfo;
    };
    eventHub->SetDefaultOnDragStart(std::move(defaultOnDragStart));

    /**
     * @tc.steps: step3. call GetDragDropInfo function
     *            case: textDraggable is false, and component is text
     * @tc.expected: customNode is null, extraInfo is 'default extraInfo'.
     */
    auto frameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(frameNode, nullptr);
    GestureEvent info;
    DragDropInfo dragPreviewInfo;
    RefPtr<OHOS::Ace::DragEvent> dragEvent = AceType::MakeRefPtr<OHOS::Ace::DragEvent>();
    auto dragDropInfo = gestureEventHub->GetDragDropInfo(info, frameNode, dragPreviewInfo, dragEvent);
    EXPECT_FALSE(dragDropInfo.customNode);
    EXPECT_EQ(dragDropInfo.extraInfo, "default extraInfo");

    /**
     * @tc.steps: step4. call GetDragDropInfo function
     *            case: textDraggable is true, and component is text
     * @tc.expected: customNode is null, extraInfo is 'default extraInfo'.
     */
    gestureEventHub->SetTextDraggable(true);
    dragDropInfo = gestureEventHub->GetDragDropInfo(info, frameNode, dragPreviewInfo, dragEvent);
    EXPECT_FALSE(dragDropInfo.customNode);
    EXPECT_EQ(dragDropInfo.extraInfo, "default extraInfo");

    /**
     * @tc.steps: step5. set onDragStart for eventHub
     */
    auto onDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& dragEvent, const std::string& /* param */) {
        DragDropInfo dragDropInfo;
        auto unifiedData = AceType::MakeRefPtr<MockUnifiedData>();
        dragEvent->SetData(unifiedData);
        auto customNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
        dragDropInfo.customNode = customNode;
        dragDropInfo.extraInfo = "user set extraInfo";
        return dragDropInfo;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));

    /**
     * @tc.steps: step6. call GetDragDropInfo function
     *            case: textDraggable is true, and component is text
     * @tc.expected: customNode is null, extraInfo is 'user set extraInfo'.
     */
    dragDropInfo = gestureEventHub->GetDragDropInfo(info, frameNode, dragPreviewInfo, dragEvent);
    EXPECT_FALSE(dragDropInfo.customNode);
    EXPECT_EQ(dragDropInfo.extraInfo, "user set extraInfo");
}

/**
 * @tc.name: GetDragDropInfo002
 * @tc.desc: Test GetDragDropInfo function.
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GetDragDropInfo002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. set onDragStart for eventHub
     */
    auto onDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& dragEvent, const std::string& /* param */) {
        DragDropInfo dragDropInfo;
        auto unifiedData = AceType::MakeRefPtr<MockUnifiedData>();
        dragEvent->SetData(unifiedData);
        auto customNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
        dragDropInfo.customNode = customNode;
        dragDropInfo.extraInfo = "user set extraInfo";
        return dragDropInfo;
    };
    eventHub->SetOnDragStart(std::move(onDragStart));;

    /**
     * @tc.steps: step3. set dragPreview for frameNode
     */
    auto frameNode = FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, -1, AceType::MakeRefPtr<ImagePattern>());
    ASSERT_NE(frameNode, nullptr);
    DragDropInfo dragPreviewDropInfo;
    dragPreviewDropInfo.extraInfo = "drag preview extraInfo";
    auto customNode = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    dragPreviewDropInfo.customNode = customNode;
    frameNode->SetDragPreview(dragPreviewDropInfo);

    /**
     * @tc.steps: step4. call GetDragDropInfo function
     *            case: textDraggable is false, and component is image
     * @tc.expected: dragPreviewInfo.customNode is not null, extraInfo is 'drag preview extraInfo'.
     */
    GestureEvent info;
    DragDropInfo dragPreviewInfo;
    RefPtr<OHOS::Ace::DragEvent> dragEvent = AceType::MakeRefPtr<OHOS::Ace::DragEvent>();
    auto dragDropInfo = gestureEventHub->GetDragDropInfo(info, frameNode, dragPreviewInfo, dragEvent);
    EXPECT_TRUE(dragDropInfo.customNode);
    EXPECT_EQ(dragDropInfo.extraInfo, "user set extraInfo");
    EXPECT_TRUE(dragPreviewInfo.customNode);
    EXPECT_EQ(dragPreviewInfo.extraInfo, "drag preview extraInfo");
}

/**
 * @tc.name: GetUnifiedData001
 * @tc.desc: Test GetUnifiedData function when user has not set the onDragStart callback.
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GetUnifiedData001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. set OnDragStart for eventHub
     *            case: user not set onDragStart callback function
     */
    RefPtr<OHOS::Ace::DragEvent> dragEvent = AceType::MakeRefPtr<OHOS::Ace::DragEvent>();
    auto defaultOnDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& dragEvent, const std::string& /* param */) {
        DragDropInfo dragDropInfo;
        auto unifiedData = AceType::MakeRefPtr<MockUnifiedData>();
        dragEvent->SetData(unifiedData);
        dragDropInfo.extraInfo = "default extraInfo";
        return dragDropInfo;
    };
    eventHub->SetDefaultOnDragStart(std::move(defaultOnDragStart));
    EXPECT_TRUE(eventHub->GetDefaultOnDragStart());

    /**
     * @tc.steps: step3. Call GetUnifiedData function
     *            case: Do not set default onDragStart function
     * @tc.expected: unifiedData is not null, extraInfo is not empty.
     */
    DragDropInfo dragDropInfo;
    gestureEventHub->GetUnifiedData("", dragDropInfo, dragEvent);
    EXPECT_TRUE(dragEvent->GetData());
    EXPECT_EQ(dragDropInfo.extraInfo, "default extraInfo");

    /**
     * @tc.steps: step4. Call GetUnifiedData function
     *            case: Do not set default onDragStart function
     * @tc.expected: unifiedData is not null, extraInfo is not empty.
     */
    dragEvent->SetData(nullptr);
    dragDropInfo.extraInfo = "";
    eventHub->SetDefaultOnDragStart(nullptr);
    gestureEventHub->GetUnifiedData("", dragDropInfo, dragEvent);
    EXPECT_EQ(dragEvent->GetData(), nullptr);
    EXPECT_EQ(dragDropInfo.extraInfo, "");
}

/**
 * @tc.name: GetUnifiedData002
 * @tc.desc: Test GetUnifiedData function when user set the onDragStart callback.
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GetUnifiedData002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. set OnDragStart for eventHub
     *            case: set user set onDragStart and defaultOnDragStart
     */
    RefPtr<OHOS::Ace::DragEvent> dragEvent = AceType::MakeRefPtr<OHOS::Ace::DragEvent>();
    auto defaultOnDragStart = [](const RefPtr<OHOS::Ace::DragEvent>& dragEvent, const std::string& /* param */) {
        DragDropInfo dragDropInfo;
        auto unifiedData = AceType::MakeRefPtr<MockUnifiedData>();
        dragEvent->SetData(unifiedData);
        dragDropInfo.extraInfo = "default extraInfo";
        return dragDropInfo;
    };
    eventHub->SetDefaultOnDragStart(std::move(defaultOnDragStart));
    EXPECT_TRUE(eventHub->GetDefaultOnDragStart());

    /**
     * @tc.steps: step3. Call GetUnifiedData function
     *            case: user do not set unifiedData and extraInfo
     * @tc.expected: unifiedData is not null, extraInfo is not empty.
     */
    DragDropInfo dragDropInfo;
    gestureEventHub->GetUnifiedData("", dragDropInfo, dragEvent);
    EXPECT_TRUE(dragEvent->GetData());
    EXPECT_EQ(dragDropInfo.extraInfo, "default extraInfo");

    /**
     * @tc.steps: step4. set OnDragStart for eventHub
     *            case: user set onDragStart function
     */
    auto unifiedData = AceType::MakeRefPtr<MockUnifiedData>();
    dragEvent->SetData(unifiedData);
    dragDropInfo.extraInfo = "user set extraInfo";
    eventHub->SetDefaultOnDragStart(std::move(defaultOnDragStart));
    EXPECT_TRUE(eventHub->GetDefaultOnDragStart());

    /**
     * @tc.steps: step5. Call GetUnifiedData function
     *            case: user set unifiedData and extraInfo
     * @tc.expected: unifiedData is not null, extraInfo is not empty.
     */
    gestureEventHub->GetUnifiedData("", dragDropInfo, dragEvent);
    EXPECT_TRUE(dragEvent->GetData());
    EXPECT_EQ(dragDropInfo.extraInfo, "user set extraInfo");
}

/**
 * @tc.name: GestureEventHubNodeTest001
 * @tc.desc: Test SetJSFrameNodeOnClick and ClearJSFrameNodeOnClick.
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubNodeTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. Create GestureEventFunc and call SetJSFrameNodeOnClick.
     * @tc.expected: ClickEventActuator_ is not nullptr.
     */
    GestureEventFunc gestureEventFunc = [](GestureEvent& info) {};
    gestureEventHub->SetJSFrameNodeOnClick(std::move(gestureEventFunc));
    EXPECT_NE(gestureEventHub->clickEventActuator_, nullptr);

    /**
     * @tc.steps: step3. Set parallelCombineClick and call SetJSFrameNodeOnClick.
     * @tc.expected: userParallelClickEventActuator_ is not nullptr.
     */
    gestureEventHub->parallelCombineClick = true;
    gestureEventHub->SetJSFrameNodeOnClick(std::move(gestureEventFunc));
    EXPECT_NE(gestureEventHub->userParallelClickEventActuator_, nullptr);

    /**
     * @tc.steps: step4. Call ClearJSFrameNodeOnClick.
     * @tc.expected: jsFrameNodeCallback_ is nullptr.
     */
    gestureEventHub->ClearJSFrameNodeOnClick();
    EXPECT_EQ(gestureEventHub->clickEventActuator_->jsFrameNodeCallback_, nullptr);
}

/**
 * @tc.name: GestureEventHubNodeTest002
 * @tc.desc: Test SetOnTouchEvent, SetJSFrameNodeOnTouchEvent and ClearJSFrameNodeOnClick.
 * @tc.type: FUNC
 */
HWTEST_F(GestureEventHubTestNg, GestureEventHubNodeTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventHub.
     * @tc.expected: gestureEventHub is not null.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    EXPECT_TRUE(eventHub);
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(eventHub);
    EXPECT_TRUE(gestureEventHub);

    /**
     * @tc.steps: step2. Create touchEventFunc and call SetOnTouchEvent.
     * @tc.expected: ouchEventActuator_ is not nullptr.
     */
    TouchEventFunc touchEventFunc = [](TouchEventInfo& info) {};
    gestureEventHub->SetOnTouchEvent(std::move(touchEventFunc));
    gestureEventHub->SetJSFrameNodeOnTouchEvent(std::move(touchEventFunc));
    EXPECT_NE(gestureEventHub->touchEventActuator_, nullptr);

    /**
     * @tc.steps: step3. Call ClearJSFrameNodeOnTouch.
     * @tc.expected: commonTouchEventCallback_ is nullptr.
     */
    gestureEventHub->ClearJSFrameNodeOnTouch();
    EXPECT_EQ(gestureEventHub->touchEventActuator_->commonTouchEventCallback_, nullptr);
}
} // namespace OHOS::Ace::NG
