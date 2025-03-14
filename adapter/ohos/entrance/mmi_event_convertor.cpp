/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "adapter/ohos/entrance/mmi_event_convertor.h"

#include <memory>

#include "pointer_event.h"

#include "adapter/ohos/entrance/ace_container.h"
#include "adapter/ohos/entrance/ace_extra_input_data.h"
#include "base/utils/time_util.h"
#include "base/utils/utils.h"
#include "core/event/ace_events.h"
#include "core/event/key_event.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::Platform {
namespace {
constexpr int32_t ANGLE_0 = 0;
constexpr int32_t ANGLE_90 = 90;
constexpr int32_t ANGLE_180 = 180;
constexpr int32_t ANGLE_270 = 270;
constexpr double SIZE_DIVIDE = 2.0;
} // namespace

SourceTool GetSourceTool(int32_t orgToolType)
{
    switch (orgToolType) {
        case OHOS::MMI::PointerEvent::TOOL_TYPE_FINGER:
            return SourceTool::FINGER;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_PEN:
            return SourceTool::PEN;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_RUBBER:
            return SourceTool::RUBBER;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_BRUSH:
            return SourceTool::BRUSH;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_PENCIL:
            return SourceTool::PENCIL;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_AIRBRUSH:
            return SourceTool::AIRBRUSH;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_MOUSE:
            return SourceTool::MOUSE;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_LENS:
            return SourceTool::LENS;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_TOUCHPAD:
            return SourceTool::TOUCHPAD;
        default:
            LOGW("unknown tool type");
            return SourceTool::UNKNOWN;
    }
}

TouchPoint ConvertTouchPoint(const MMI::PointerEvent::PointerItem& pointerItem)
{
    TouchPoint touchPoint;
    // just get the max of width and height
    touchPoint.size = std::max(pointerItem.GetWidth(), pointerItem.GetHeight()) / 2.0;
    touchPoint.id = pointerItem.GetPointerId();
    touchPoint.downTime = TimeStamp(std::chrono::microseconds(pointerItem.GetDownTime()));
    touchPoint.x = pointerItem.GetWindowX();
    touchPoint.y = pointerItem.GetWindowY();
    touchPoint.screenX = pointerItem.GetDisplayX();
    touchPoint.screenY = pointerItem.GetDisplayY();
    touchPoint.isPressed = pointerItem.IsPressed();
    touchPoint.force = static_cast<float>(pointerItem.GetPressure());
    touchPoint.tiltX = pointerItem.GetTiltX();
    touchPoint.tiltY = pointerItem.GetTiltY();
    touchPoint.sourceTool = GetSourceTool(pointerItem.GetToolType());
    touchPoint.originalId = pointerItem.GetOriginPointerId();
    return touchPoint;
}

void UpdateTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, TouchEvent& touchEvent)
{
    auto ids = pointerEvent->GetPointerIds();
    for (auto&& id : ids) {
        MMI::PointerEvent::PointerItem item;
        bool ret = pointerEvent->GetPointerItem(id, item);
        if (!ret) {
            LOGE("get pointer item failed.");
            continue;
        }
        auto touchPoint = ConvertTouchPoint(item);
        touchEvent.pointers.emplace_back(std::move(touchPoint));
    }
    touchEvent.CovertId();
}

Offset GetTouchEventOriginOffset(const TouchEvent& event)
{
    auto pointerEvent = event.pointerEvent;
    if (!pointerEvent) {
        return Offset();
    }
    int32_t pointerID = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem item;
    bool ret = pointerEvent->GetPointerItem(pointerID, item);
    if (!ret) {
        return Offset();
    } else {
        return Offset(item.GetWindowX(), item.GetWindowY());
    }
}

TouchEvent ConvertTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    int32_t pointerID = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem item;
    bool ret = pointerEvent->GetPointerItem(pointerID, item);
    if (!ret) {
        LOGE("get pointer item failed.");
        return TouchEvent();
    }
    auto touchPoint = ConvertTouchPoint(item);
    std::chrono::microseconds microseconds(pointerEvent->GetActionTime());
    TimeStamp time(microseconds);
    TouchEvent event;
    event.SetId(touchPoint.id)
        .SetX(touchPoint.x)
        .SetY(touchPoint.y)
        .SetScreenX(touchPoint.screenX)
        .SetScreenY(touchPoint.screenY)
        .SetType(TouchType::UNKNOWN)
        .SetPullType(TouchType::UNKNOWN)
        .SetTime(time)
        .SetSize(touchPoint.size)
        .SetForce(touchPoint.force)
        .SetTiltX(touchPoint.tiltX)
        .SetTiltY(touchPoint.tiltY)
        .SetDeviceId(pointerEvent->GetDeviceId())
        .SetTargetDisplayId(pointerEvent->GetTargetDisplayId())
        .SetSourceType(SourceType::NONE)
        .SetSourceTool(touchPoint.sourceTool)
        .SetTouchEventId(pointerEvent->GetId())
        .SetOriginalId(touchPoint.originalId);
    AceExtraInputData::ReadToTouchEvent(pointerEvent, event);
    event.pointerEvent = pointerEvent;
    int32_t orgDevice = pointerEvent->GetSourceType();
    GetEventDevice(orgDevice, event);
    int32_t orgAction = pointerEvent->GetPointerAction();
    SetTouchEventType(orgAction, event);
    UpdateTouchEvent(pointerEvent, event);
    if (event.sourceType == SourceType::TOUCH && event.sourceTool == SourceTool::PEN) {
        // Pen use type double XY position.
        event.x = item.GetWindowXPos();
        event.y = item.GetWindowYPos();
        event.screenX = item.GetDisplayXPos();
        event.screenY = item.GetDisplayYPos();
    }
    event.pressedKeyCodes_.clear();
    for (const auto& curCode : pointerEvent->GetPressedKeys()) {
        event.pressedKeyCodes_.emplace_back(static_cast<KeyCode>(curCode));
    }
    return event;
}

void SetTouchEventType(int32_t orgAction, TouchEvent& event)
{
    switch (orgAction) {
        case OHOS::MMI::PointerEvent::POINTER_ACTION_CANCEL:
            event.type = TouchType::CANCEL;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_DOWN:
            event.type = TouchType::DOWN;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_MOVE:
            event.type = TouchType::MOVE;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_UP:
            event.type = TouchType::UP;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_DOWN:
            event.type = TouchType::PULL_DOWN;
            event.pullType = TouchType::PULL_DOWN;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_MOVE:
            event.type = TouchType::PULL_MOVE;
            event.pullType = TouchType::PULL_MOVE;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_UP:
            event.type = TouchType::PULL_UP;
            event.pullType = TouchType::PULL_UP;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_IN_WINDOW:
            event.type = TouchType::PULL_IN_WINDOW;
            event.pullType = TouchType::PULL_IN_WINDOW;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_OUT_WINDOW:
            event.type = TouchType::PULL_OUT_WINDOW;
            event.pullType = TouchType::PULL_OUT_WINDOW;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER:
            event.type = TouchType::HOVER_ENTER;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_HOVER_MOVE:
            event.type = TouchType::HOVER_MOVE;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_HOVER_EXIT:
            event.type = TouchType::HOVER_EXIT;
            return;
        default:
            LOGW("unknown type");
            return;
    }
}

void GetMouseEventAction(int32_t action, MouseEvent& events, bool isScenceBoardWindow)
{
    switch (action) {
        case OHOS::MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN:
            events.action = MouseAction::PRESS;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_BUTTON_UP:
            events.action = MouseAction::RELEASE;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_ENTER_WINDOW:
            events.action = MouseAction::WINDOW_ENTER;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_LEAVE_WINDOW:
            events.action = MouseAction::WINDOW_LEAVE;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_MOVE:
            events.action = MouseAction::MOVE;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_DOWN:
            events.action = MouseAction::PRESS;
            events.pullAction = MouseAction::PULL_DOWN;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_MOVE:
            events.action = MouseAction::MOVE;
            events.pullAction = MouseAction::PULL_MOVE;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_UP:
            events.action = MouseAction::RELEASE;
            events.pullAction = MouseAction::PULL_UP;
            break;
        default:
            events.action = MouseAction::NONE;
            break;
    }
}

void GetMouseEventButton(int32_t button, MouseEvent& events)
{
    switch (button) {
        case OHOS::MMI::PointerEvent::MOUSE_BUTTON_LEFT:
            events.button = MouseButton::LEFT_BUTTON;
            break;
        case OHOS::MMI::PointerEvent::MOUSE_BUTTON_RIGHT:
            events.button = MouseButton::RIGHT_BUTTON;
            break;
        case OHOS::MMI::PointerEvent::MOUSE_BUTTON_MIDDLE:
            events.button = MouseButton::MIDDLE_BUTTON;
            break;
        case OHOS::MMI::PointerEvent::MOUSE_BUTTON_SIDE:
            events.button = MouseButton::BACK_BUTTON;
            break;
        case OHOS::MMI::PointerEvent::MOUSE_BUTTON_EXTRA:
            events.button = MouseButton::FORWARD_BUTTON;
            break;
        default:
            events.button = MouseButton::NONE_BUTTON;
            break;
    }
}

void ConvertMouseEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    MouseEvent& events, bool isScenceBoardWindow)
{
    int32_t pointerID = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem item;
    bool ret = pointerEvent->GetPointerItem(pointerID, item);
    if (!ret) {
        LOGE("get pointer: %{public}d item failed.", pointerID);
        return;
    }
    events.id = pointerID;
    events.x = item.GetWindowX();
    events.y = item.GetWindowY();
    events.screenX = item.GetDisplayX();
    events.screenY = item.GetDisplayY();
    int32_t orgAction = pointerEvent->GetPointerAction();
    GetMouseEventAction(orgAction, events, isScenceBoardWindow);
    int32_t orgButton = pointerEvent->GetButtonId();
    GetMouseEventButton(orgButton, events);
    int32_t orgDevice = pointerEvent->GetSourceType();
    GetEventDevice(orgDevice, events);
    events.targetDisplayId = pointerEvent->GetTargetDisplayId();
    events.originalId = item.GetOriginPointerId();

    std::set<int32_t> pressedSet = pointerEvent->GetPressedButtons();
    uint32_t pressedButtons = 0;
    if (pressedSet.find(OHOS::MMI::PointerEvent::MOUSE_BUTTON_LEFT) != pressedSet.end()) {
        pressedButtons &= static_cast<uint32_t>(MouseButton::LEFT_BUTTON);
    }
    if (pressedSet.find(OHOS::MMI::PointerEvent::MOUSE_BUTTON_RIGHT) != pressedSet.end()) {
        pressedButtons &= static_cast<uint32_t>(MouseButton::RIGHT_BUTTON);
    }
    if (pressedSet.find(OHOS::MMI::PointerEvent::MOUSE_BUTTON_MIDDLE) != pressedSet.end()) {
        pressedButtons &= static_cast<uint32_t>(MouseButton::MIDDLE_BUTTON);
    }
    events.pressedButtons = static_cast<int32_t>(pressedButtons);

    std::chrono::microseconds microseconds(pointerEvent->GetActionTime());
    TimeStamp time(microseconds);
    events.time = time;
    events.pointerEvent = pointerEvent;
    events.sourceTool = GetSourceTool(item.GetToolType());
    if (events.sourceType == SourceType::TOUCH && events.sourceTool == SourceTool::PEN) {
        events.id = TOUCH_TOOL_BASE_ID + static_cast<int32_t>(events.sourceTool);
        // Pen use type double XY position.
        events.x = item.GetWindowXPos();
        events.y = item.GetWindowYPos();
        events.screenX = item.GetDisplayXPos();
        events.screenY = item.GetDisplayYPos();
        events.originalId = events.id;
    }
    events.touchEventId = pointerEvent->GetId();
    events.pressedKeyCodes_.clear();
    for (const auto& curCode : pointerEvent->GetPressedKeys()) {
        events.pressedKeyCodes_.emplace_back(static_cast<KeyCode>(curCode));
    }
}

void GetAxisEventAction(int32_t action, AxisEvent& event)
{
    switch (action) {
        case OHOS::MMI::PointerEvent::POINTER_ACTION_AXIS_BEGIN:
        case OHOS::MMI::PointerEvent::POINTER_ACTION_ROTATE_BEGIN:
            event.action = AxisAction::BEGIN;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_AXIS_UPDATE:
        case OHOS::MMI::PointerEvent::POINTER_ACTION_ROTATE_UPDATE:
            event.action = AxisAction::UPDATE;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_AXIS_END:
        case OHOS::MMI::PointerEvent::POINTER_ACTION_ROTATE_END:
            event.action = AxisAction::END;
            break;
        default:
            event.action = AxisAction::NONE;
            break;
    }
}

void ConvertAxisEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, AxisEvent& event)
{
    int32_t pointerID = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem item;
    bool ret = pointerEvent->GetPointerItem(pointerID, item);
    if (!ret) {
        LOGE("get pointer: %{public}d item failed.", pointerID);
        return;
    }

    event.id = item.GetPointerId();
    event.x = static_cast<float>(item.GetWindowX());
    event.y = static_cast<float>(item.GetWindowY());
    event.screenX = static_cast<float>(item.GetDisplayX());
    event.screenY = static_cast<float>(item.GetDisplayY());
    event.horizontalAxis = pointerEvent->GetAxisValue(OHOS::MMI::PointerEvent::AxisType::AXIS_TYPE_SCROLL_HORIZONTAL);
    event.verticalAxis = pointerEvent->GetAxisValue(OHOS::MMI::PointerEvent::AxisType::AXIS_TYPE_SCROLL_VERTICAL);
    event.pinchAxisScale = pointerEvent->GetAxisValue(OHOS::MMI::PointerEvent::AxisType::AXIS_TYPE_PINCH);
    event.rotateAxisAngle = pointerEvent->GetAxisValue(OHOS::MMI::PointerEvent::AxisType::AXIS_TYPE_ROTATE);
    int32_t orgAction = pointerEvent->GetPointerAction();
    GetAxisEventAction(orgAction, event);
    event.isRotationEvent = (orgAction >= MMI::PointerEvent::POINTER_ACTION_ROTATE_BEGIN) &&
                            (orgAction <= MMI::PointerEvent::POINTER_ACTION_ROTATE_END);
    int32_t orgDevice = pointerEvent->GetSourceType();
    GetEventDevice(orgDevice, event);
    event.sourceTool = GetSourceTool(item.GetToolType());
    event.pointerEvent = pointerEvent;
    event.originalId = item.GetOriginPointerId();

    std::chrono::microseconds microseconds(pointerEvent->GetActionTime());
    TimeStamp time(microseconds);
    event.time = time;
    event.touchEventId = pointerEvent->GetId();
    event.targetDisplayId = pointerEvent->GetTargetDisplayId();
    event.pressedCodes.clear();
    for (const auto& curCode : pointerEvent->GetPressedKeys()) {
        event.pressedCodes.emplace_back(static_cast<KeyCode>(curCode));
    }
}

void ConvertKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent, KeyEvent& event)
{
    event.rawKeyEvent = keyEvent;
    event.code = static_cast<KeyCode>(keyEvent->GetKeyCode());
    event.keyIntention = static_cast<KeyIntention>(keyEvent->GetKeyIntention());
    if (keyEvent->GetKeyAction() == OHOS::MMI::KeyEvent::KEY_ACTION_UP) {
        event.action = KeyAction::UP;
    } else if (keyEvent->GetKeyAction() == OHOS::MMI::KeyEvent::KEY_ACTION_DOWN) {
        event.action = KeyAction::DOWN;
    } else {
        event.action = KeyAction::UNKNOWN;
    }
    std::chrono::microseconds microseconds(keyEvent->GetActionTime());
    TimeStamp time(microseconds);
    event.timeStamp = time;
    event.key = MMI::KeyEvent::KeyCodeToString(keyEvent->GetKeyCode());
    event.deviceId = keyEvent->GetDeviceId();
    event.sourceType = SourceType::KEYBOARD;
#ifdef SECURITY_COMPONENT_ENABLE
    event.enhanceData = keyEvent->GetEnhanceData();
#endif
    event.pressedCodes.clear();
    for (const auto& curCode : keyEvent->GetPressedKeys()) {
        event.pressedCodes.emplace_back(static_cast<KeyCode>(curCode));
    }
}

void ConvertPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, PointerEvent& event)
{
    event.rawPointerEvent = pointerEvent;
    event.pointerEventId = pointerEvent->GetId();
    event.pointerId = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    event.pressed = pointerItem.IsPressed();
    event.windowX = pointerItem.GetWindowX();
    event.windowY = pointerItem.GetWindowY();
    event.displayX = pointerItem.GetDisplayX();
    event.displayY = pointerItem.GetDisplayY();
    event.size = std::max(pointerItem.GetWidth(), pointerItem.GetHeight()) / SIZE_DIVIDE;
    event.force = static_cast<float>(pointerItem.GetPressure());
    event.deviceId = pointerItem.GetDeviceId();
    event.downTime = TimeStamp(std::chrono::microseconds(pointerItem.GetDownTime()));
    event.time = TimeStamp(std::chrono::microseconds(pointerEvent->GetActionTime()));
    event.sourceTool = GetSourceTool(pointerItem.GetToolType());
    event.targetWindowId = pointerItem.GetTargetWindowId();
    event.x = event.windowX;
    event.y = event.windowY;
    event.pressedKeyCodes_.clear();
    for (const auto& curCode : pointerEvent->GetPressedKeys()) {
        event.pressedKeyCodes_.emplace_back(static_cast<KeyCode>(curCode));
    }
}

void LogPointInfo(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, int32_t instanceId)
{
    if (pointerEvent->GetPointerAction() == OHOS::MMI::PointerEvent::POINTER_ACTION_DOWN) {
        auto container = Platform::AceContainer::GetContainer(instanceId);
        if (container) {
            auto pipelineContext = container->GetPipelineContext();
            if (pipelineContext) {
                uint32_t windowId = pipelineContext->GetWindowId();
                LOGI("pointdown windowId: %{public}u", windowId);
            }
        }
    }
    if (SystemProperties::GetDebugEnabled()) {
        LOGI("point source: %{public}d", pointerEvent->GetSourceType());
        auto actionId = pointerEvent->GetPointerId();
        MMI::PointerEvent::PointerItem item;
        if (pointerEvent->GetPointerItem(actionId, item)) {
            LOGI("action point info: id: %{public}d, pointerId: %{public}d, x: %{public}d, y: %{public}d, action: "
                "%{public}d, pressure: %{public}f, tiltX: %{public}f, tiltY: %{public}f",
                pointerEvent->GetId(), actionId, item.GetWindowX(), item.GetWindowY(), pointerEvent->GetPointerAction(),
                item.GetPressure(), item.GetTiltX(), item.GetTiltY());
        }
        auto ids = pointerEvent->GetPointerIds();
        for (auto&& id : ids) {
            MMI::PointerEvent::PointerItem item;
            if (pointerEvent->GetPointerItem(id, item)) {
                LOGI("all point info: id: %{public}d, x: %{public}d, y: %{public}d, isPressed: %{public}d, pressure: "
                     "%{public}f, tiltX: %{public}f, tiltY: %{public}f",
                    actionId, item.GetWindowX(), item.GetWindowY(), item.IsPressed(), item.GetPressure(),
                    item.GetTiltX(), item.GetTiltY());
            }
        }
    }
}

void CalculatePointerEvent(const NG::OffsetF& offsetF, const std::shared_ptr<MMI::PointerEvent>& point,
    const NG::VectorF& scale, int32_t udegree)
{
    CHECK_NULL_VOID(point);
    int32_t pointerId = point->GetPointerId();
    MMI::PointerEvent::PointerItem item;
    bool ret = point->GetPointerItem(pointerId, item);
    if (ret) {
        float xRelative = item.GetWindowX();
        float yRelative = item.GetWindowY();
        if (point->GetSourceType() == OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN &&
            item.GetToolType() == OHOS::MMI::PointerEvent::TOOL_TYPE_PEN) {
            xRelative = item.GetWindowXPos();
            yRelative = item.GetWindowYPos();
        }
        auto windowX = xRelative;
        auto windowY = yRelative;
        auto pipelineContext = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        auto displayWindowRect = pipelineContext->GetDisplayWindowRectInfo();
        auto windowWidth = displayWindowRect.Width();
        auto windowHeight = displayWindowRect.Height();
        switch (udegree) {
            case ANGLE_0:
                windowX = xRelative - offsetF.GetX();
                windowY = yRelative - offsetF.GetY();
                break;
            case ANGLE_90:
                windowX = yRelative - offsetF.GetX();
                windowY = windowWidth - offsetF.GetY() - xRelative;
                break;
            case ANGLE_180:
                windowX = windowWidth - offsetF.GetX() - xRelative;
                windowY = windowHeight - offsetF.GetY() - yRelative;
                break;
            case ANGLE_270:
                windowX = windowHeight - offsetF.GetX() - yRelative;
                windowY = xRelative - offsetF.GetY();
                break;
            default:
                break;
        }
        windowX = NearZero(scale.x) ? windowX : windowX / scale.x;
        windowY = NearZero(scale.y) ? windowY : windowY / scale.y;

        item.SetWindowX(static_cast<int32_t>(windowX));
        item.SetWindowY(static_cast<int32_t>(windowY));
        item.SetWindowXPos(windowX);
        item.SetWindowYPos(windowY);
        point->UpdatePointerItem(pointerId, item);
    }
}

void CalculateWindowCoordinate(const NG::OffsetF& offsetF, const std::shared_ptr<MMI::PointerEvent>& point,
    const NG::VectorF& scale, const int32_t udegree)
{
    CHECK_NULL_VOID(point);
    auto ids = point->GetPointerIds();
    for (auto&& id : ids) {
        MMI::PointerEvent::PointerItem item;
        bool ret = point->GetPointerItem(id, item);
        if (!ret) {
            LOGE("get pointer:%{public}d item failed", id);
            continue;
        }
        float xRelative = item.GetDisplayX();
        float yRelative = item.GetDisplayY();
        if (point->GetSourceType() == OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN &&
            item.GetToolType() == OHOS::MMI::PointerEvent::TOOL_TYPE_PEN) {
            xRelative = item.GetDisplayXPos();
            yRelative = item.GetDisplayYPos();
        }
        float windowX = xRelative;
        float windowY = yRelative;
        int32_t deviceWidth = SystemProperties::GetDevicePhysicalWidth();
        int32_t deviceHeight = SystemProperties::GetDevicePhysicalHeight();

        if (udegree == ANGLE_0) {
            windowX = xRelative - offsetF.GetX();
            windowY = yRelative - offsetF.GetY();
        }
        if (udegree == ANGLE_90) {
            windowX = yRelative - offsetF.GetX();
            windowY = deviceWidth - offsetF.GetY() - xRelative;
        }
        if (udegree == ANGLE_180) {
            windowX = deviceWidth - offsetF.GetX() - xRelative;
            windowY = deviceHeight - offsetF.GetY() - yRelative;
        }
        if (udegree == ANGLE_270) {
            windowX = deviceHeight - offsetF.GetX() - yRelative;
            windowY = xRelative - offsetF.GetY();
        }

        windowX = NearZero(scale.x) ? windowX : windowX / scale.x;
        windowY = NearZero(scale.y) ? windowY : windowY / scale.y;

        item.SetWindowX(static_cast<int32_t>(windowX));
        item.SetWindowY(static_cast<int32_t>(windowY));
        item.SetWindowXPos(windowX);
        item.SetWindowYPos(windowY);
        point->UpdatePointerItem(id, item);
    }
}
} // namespace OHOS::Ace::Platform
