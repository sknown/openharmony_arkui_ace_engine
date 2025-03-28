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

#include "core/components_ng/event/touch_event.h"

#include "core/components_ng/event/response_ctrl.h"
#include "core/components_ng/gestures/recognizers/gesture_recognizer.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

bool TouchEventActuator::DispatchEvent(const TouchEvent& point)
{
    return true;
}

void TouchEventActuator::OnFlushTouchEventsBegin()
{
    isFlushTouchEventsEnd_ = false;
}

void TouchEventActuator::OnFlushTouchEventsEnd()
{
    isFlushTouchEventsEnd_ = true;
}

bool TouchEventActuator::HandleEvent(const TouchEvent& point)
{
    // if current node is forbidden by monopolize, upper nodes should not response either
    if (!ShouldResponse()) {
        return false;
    }
    return TriggerTouchCallBack(point);
}

bool TouchEventActuator::TriggerTouchCallBack(const TouchEvent& point)
{
    if (point.type == TouchType::DOWN &&
        firstInputTimeWithId_.find(point.id) == firstInputTimeWithId_.end()) {
        firstInputTimeWithId_[point.id] = point.time;
    }
    if (point.type == TouchType::UP &&
        firstInputTimeWithId_.find(point.id) != firstInputTimeWithId_.end()) {
        int64_t overTime = GetSysTimestamp();
        int64_t inputTime = static_cast<int64_t>(firstInputTimeWithId_[point.id].time_since_epoch().count());
        if (SystemProperties::GetTraceInputEventEnabled()) {
            ACE_SCOPED_TRACE("UserEvent InputTime:%lld OverTime:%lld InputType:TouchEvent",
                static_cast<long long>(inputTime), static_cast<long long>(overTime));
        }
        if (SystemProperties::GetTraceInputEventEnabled()) {
            ACE_SCOPED_TRACE("UserEvent InputTime:%lld AcceptTime:%lld InputType:TouchEvent",
                static_cast<long long>(inputTime), static_cast<long long>(overTime));
        }
        firstInputTimeWithId_.erase(point.id);
    }

    if (touchEvents_.empty() && !touchAfterEvents_ && !userCallback_ && !onTouchEventCallback_ &&
        !commonTouchEventCallback_) {
        return true;
    }
    TouchEvent lastPoint;
    if (point.isInterpolated) {
        lastPoint = point;
    } else {
        lastPoint = !point.history.empty() ? point.history.back() : point;
    }
    TouchEventInfo event("touchEvent");
    event.SetTimeStamp(lastPoint.time);
    event.SetPointerEvent(lastPoint.pointerEvent);
    TouchLocationInfo changedInfo("onTouch", lastPoint.originalId);
    PointF lastLocalPoint(lastPoint.x, lastPoint.y);
    NGGestureRecognizer::Transform(lastLocalPoint, GetAttachedNode(), false,
        isPostEventResult_, point.postEventNodeId);
    auto localX = static_cast<float>(lastLocalPoint.GetX());
    auto localY = static_cast<float>(lastLocalPoint.GetY());
    changedInfo.SetLocalLocation(Offset(localX, localY));
    changedInfo.SetGlobalLocation(Offset(lastPoint.x, lastPoint.y));
    changedInfo.SetScreenLocation(Offset(lastPoint.screenX, lastPoint.screenY));
    changedInfo.SetTouchType(lastPoint.type);
    changedInfo.SetForce(lastPoint.force);
    if (lastPoint.tiltX.has_value()) {
        changedInfo.SetTiltX(lastPoint.tiltX.value());
    }
    if (lastPoint.tiltY.has_value()) {
        changedInfo.SetTiltY(lastPoint.tiltY.value());
    }
    changedInfo.SetSourceTool(lastPoint.sourceTool);
    event.AddChangedTouchLocationInfo(std::move(changedInfo));
    event.SetTarget(GetEventTarget().value_or(EventTarget()));
    auto frameNode = GetAttachedNode().Upgrade();
    std::string patternName = "";
    if (frameNode) {
        patternName = frameNode->GetTag();
    }
    event.SetPatternName(patternName.c_str());

    // all fingers collection
    for (const auto& item : lastPoint.pointers) {
        float globalX = item.x;
        float globalY = item.y;
        float screenX = item.screenX;
        float screenY = item.screenY;
        PointF localPoint(globalX, globalY);
        NGGestureRecognizer::Transform(localPoint, GetAttachedNode(),
            false, isPostEventResult_, point.postEventNodeId);
        auto localX = static_cast<float>(localPoint.GetX());
        auto localY = static_cast<float>(localPoint.GetY());
        TouchLocationInfo info("onTouch", item.originalId);
        info.SetGlobalLocation(Offset(globalX, globalY));
        info.SetLocalLocation(Offset(localX, localY));
        info.SetScreenLocation(Offset(screenX, screenY));
        info.SetTouchType(lastPoint.type);
        info.SetForce(item.force);
        if (item.tiltX.has_value()) {
            info.SetTiltX(item.tiltX.value());
        }
        if (item.tiltY.has_value()) {
            info.SetTiltY(item.tiltY.value());
        }
        info.SetSourceTool(item.sourceTool);
        event.AddTouchLocationInfo(std::move(info));
    }
    event.SetSourceDevice(lastPoint.sourceType);
    event.SetForce(lastPoint.force);
    for (const auto& item : point.history) {
        float globalX = item.x;
        float globalY = item.y;
        float screenX = item.screenX;
        float screenY = item.screenY;
        PointF localPoint(globalX, globalY);
        NGGestureRecognizer::Transform(localPoint, GetAttachedNode(), false,
            isPostEventResult_, point.postEventNodeId);
        auto localX = static_cast<float>(localPoint.GetX());
        auto localY = static_cast<float>(localPoint.GetY());
        TouchLocationInfo historyInfo("onTouch", item.originalId);
        historyInfo.SetTimeStamp(item.time);
        historyInfo.SetGlobalLocation(Offset(globalX, globalY));
        historyInfo.SetLocalLocation(Offset(localX, localY));
        historyInfo.SetScreenLocation(Offset(screenX, screenY));
        historyInfo.SetTouchType(item.type);
        historyInfo.SetForce(item.force);
        if (item.tiltX.has_value()) {
            historyInfo.SetTiltX(item.tiltX.value());
        }
        if (item.tiltY.has_value()) {
            historyInfo.SetTiltY(item.tiltY.value());
        }
        historyInfo.SetSourceTool(item.sourceTool);
        event.AddHistoryLocationInfo(std::move(historyInfo));
        event.AddHistoryPointerEvent(item.pointerEvent);
    }
    if (lastPoint.tiltX.has_value()) {
        event.SetTiltX(lastPoint.tiltX.value());
    }
    if (lastPoint.tiltY.has_value()) {
        event.SetTiltY(lastPoint.tiltY.value());
    }
    event.SetSourceTool(lastPoint.sourceTool);
    event.SetPressedKeyCodes(lastPoint.pressedKeyCodes_);
    if (isFlushTouchEventsEnd_) {
        // trigger callback of the last touch event during one vsync period
        event.SetTouchEventsEnd(true);
        isFlushTouchEventsEnd_ = false;
    }
    for (auto& impl : touchEvents_) {
        if (impl) {
            (*impl)(event);
        }
    }
    if (userCallback_) {
        // actuator->userCallback_ may be overwritten in its invoke so we copy it first
        auto userCallback = userCallback_;
        (*userCallback)(event);
    }
    if (touchAfterEvents_) {
        auto touchAfterEvents = touchAfterEvents_;
        (*touchAfterEvents)(event);
    }
    if (onTouchEventCallback_) {
        // actuator->onTouchEventCallback_ may be overwritten in its invoke so we copy it first
        auto onTouchEventCallback = onTouchEventCallback_;
        (*onTouchEventCallback)(event);
    }
    if (commonTouchEventCallback_) {
        // actuator->commonTouchEventCallback_ may be overwritten in its invoke so we copy it first
        auto commonTouchEventCallback = commonTouchEventCallback_;
        (*commonTouchEventCallback)(event);
    }
    return !event.IsStopPropagation();
}

bool TouchEventActuator::ShouldResponse()
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, true);

    auto eventManager = context->GetEventManager();
    CHECK_NULL_RETURN(eventManager, true);

    auto frameNode = GetAttachedNode();
    auto ctrl = eventManager->GetResponseCtrl();
    CHECK_NULL_RETURN(ctrl, true);
    if (!ctrl->ShouldResponse(frameNode)) {
        return false;
    }
    ctrl->TrySetFirstResponse(frameNode);
    return true;
}

} // namespace OHOS::Ace::NG