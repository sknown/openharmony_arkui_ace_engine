/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/gestures/recognizers/pinch_recognizer.h"

#include "base/geometry/offset.h"
#include "base/log/log.h"
#include "base/ressched/ressched_report.h"
#include "core/components_ng/gestures/base_gesture_event.h"
#include "core/components_ng/gestures/gesture_referee.h"
#include "core/components_ng/gestures/recognizers/gesture_recognizer.h"
#include "core/components_ng/gestures/recognizers/multi_fingers_recognizer.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

namespace {

constexpr int32_t MAX_PINCH_FINGERS = 5;
constexpr int32_t DEFAULT_PINCH_FINGERS = 2;
constexpr Dimension DEFAULT_PINCH_DISTANCE = Dimension(5.0, DimensionUnit::VP);
constexpr double SCALE_PER_AXIS_EVENT = 0.1f;

} // namespace

PinchRecognizer::PinchRecognizer(int32_t fingers, double distance)
    : MultiFingersRecognizer(fingers), distance_(distance)
{
    if (fingers_ > MAX_PINCH_FINGERS || fingers_ < DEFAULT_PINCH_FINGERS) {
        fingers_ = DEFAULT_PINCH_FINGERS;
    }
    if (distance_ <= 0) {
        distance_ = DEFAULT_PINCH_DISTANCE.ConvertToPx();
    }
}

void PinchRecognizer::OnAccepted()
{
    int64_t acceptTime = GetSysTimestamp();
    int64_t inputTime = acceptTime;
    if (firstInputTime_.has_value()) {
        inputTime = static_cast<int64_t>(firstInputTime_.value().time_since_epoch().count());
    }
    if (SystemProperties::GetTraceInputEventEnabled()) {
        ACE_SCOPED_TRACE("UserEvent InputTime:%lld AcceptTime:%lld InputType:PinchGesture",
            static_cast<long long>(inputTime), static_cast<long long>(acceptTime));
    }
    
    auto node = GetAttachedNode().Upgrade();
    TAG_LOGI(AceLogTag::ACE_GESTURE, "Pinch accepted, tag = %{public}s, id = %{public}s",
        node ? node->GetTag().c_str() : "null", node ? std::to_string(node->GetId()).c_str() : "invalid");
    ResSchedReport::GetInstance().ResSchedDataReport("click");
    refereeState_ = RefereeState::SUCCEED;
    SendCallbackMsg(onActionStart_);
}

void PinchRecognizer::OnRejected()
{
    if (refereeState_ == RefereeState::SUCCEED) {
        return;
    }
    refereeState_ = RefereeState::FAIL;
    firstInputTime_.reset();
}

bool PinchRecognizer::IsCtrlBeingPressed()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    return pipeline->IsKeyInPressed(KeyCode::KEY_CTRL_LEFT) || pipeline->IsKeyInPressed(KeyCode::KEY_CTRL_RIGHT);
}

void PinchRecognizer::HandleTouchDownEvent(const TouchEvent& event)
{
    TAG_LOGI(AceLogTag::ACE_GESTURE, "Id:%{public}d, pinch %{public}d down, begin to detect pinch, state: %{public}d",
        event.touchEventId, event.id, refereeState_);
    if (!firstInputTime_.has_value()) {
        firstInputTime_ = event.time;
    }

    if (static_cast<int32_t>(activeFingers_.size()) >= fingers_) {
        return;
    }
    if (!IsInAttachedNode(event)) {
        Adjudicate(Claim(this), GestureDisposal::REJECT);
        return;
    }

    if (fingersId_.find(event.id) == fingersId_.end()) {
        fingersId_.insert(event.id);
    }
    activeFingers_.emplace_back(event.id);
    touchPoints_[event.id] = event;
    lastTouchEvent_ = event;

    if (static_cast<int32_t>(activeFingers_.size()) >= fingers_ && refereeState_ != RefereeState::FAIL) {
        initialDev_ = ComputeAverageDeviation();
        pinchCenter_ = ComputePinchCenter();
        refereeState_ = RefereeState::DETECTING;
    }
}

void PinchRecognizer::HandleTouchDownEvent(const AxisEvent& event)
{
    if (!firstInputTime_.has_value()) {
        firstInputTime_ = event.time;
    }
    if (event.isRotationEvent) {
        return;
    }
    touchPoints_[event.id] = TouchEvent();
    UpdateTouchPointWithAxisEvent(event);
    lastAxisEvent_ = event;
    if (IsRefereeFinished()) {
        return;
    }
    TAG_LOGI(AceLogTag::ACE_GESTURE, "Id:%{public}d, pinch axis start, state: %{public}d", event.touchEventId,
        refereeState_);
    if (refereeState_ == RefereeState::READY && (NearEqual(event.pinchAxisScale, 1.0) || IsCtrlBeingPressed())) {
        scale_ = 1.0f;
        pinchCenter_ = Offset(event.x, event.y);
        refereeState_ = RefereeState::DETECTING;
    }
}

void PinchRecognizer::HandleTouchUpEvent(const TouchEvent& event)
{
    if (fingersId_.find(event.id) != fingersId_.end()) {
        fingersId_.erase(event.id);
    }
    if (!IsActiveFinger(event.id)) {
        return;
    }

    TAG_LOGI(AceLogTag::ACE_GESTURE, "Id:%{public}d, pinch %{public}d up, state: %{public}d", event.touchEventId,
        event.id, refereeState_);
    if (static_cast<int32_t>(activeFingers_.size()) < fingers_ && refereeState_ != RefereeState::SUCCEED) {
        Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
        return;
    }

    lastTouchEvent_ = event;
    if ((refereeState_ != RefereeState::SUCCEED) && (refereeState_ != RefereeState::FAIL)) {
        Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
        return;
    }

    if (refereeState_ == RefereeState::SUCCEED && static_cast<int32_t>(activeFingers_.size()) == fingers_) {
        SendCallbackMsg(onActionEnd_);
        int64_t overTime = GetSysTimestamp();
        int64_t inputTime = overTime;
        if (firstInputTime_.has_value()) {
            inputTime = static_cast<int64_t>(firstInputTime_.value().time_since_epoch().count());
        }
        if (SystemProperties::GetTraceInputEventEnabled()) {
            ACE_SCOPED_TRACE("UserEvent InputTime:%lld OverTime:%lld InputType:PinchGesture",
                static_cast<long long>(inputTime), static_cast<long long>(overTime));
        }
        firstInputTime_.reset();
    }
    activeFingers_.remove(event.id);
}

void PinchRecognizer::HandleTouchUpEvent(const AxisEvent& event)
{
    TAG_LOGI(
        AceLogTag::ACE_GESTURE, "Id:%{public}d, pinch axis end, state: %{public}d", event.touchEventId, refereeState_);
    // if axisEvent received rotateEvent, no need to active Pinch recognizer.
    if (isPinchEnd_ || event.isRotationEvent) {
        return;
    }
    lastAxisEvent_ = event;
    if ((refereeState_ != RefereeState::SUCCEED) && (refereeState_ != RefereeState::FAIL)) {
        Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
        return;
    }
    if (refereeState_ == RefereeState::SUCCEED) {
        SendCallbackMsg(onActionEnd_);
        isPinchEnd_ = true;
        int64_t overTime = GetSysTimestamp();
        int64_t inputTime = overTime;
        if (firstInputTime_.has_value()) {
            inputTime = static_cast<int64_t>(firstInputTime_.value().time_since_epoch().count());
        }
        if (SystemProperties::GetTraceInputEventEnabled()) {
            ACE_SCOPED_TRACE("UserEvent InputTime:%lld OverTime:%lld InputType:PinchGesture",
                static_cast<long long>(inputTime), static_cast<long long>(overTime));
        }
        firstInputTime_.reset();
    }
}

void PinchRecognizer::HandleTouchMoveEvent(const TouchEvent& event)
{
    if (!IsActiveFinger(event.id)) {
        return;
    }

    touchPoints_[event.id] = event;
    lastTouchEvent_ = event;
    currentDev_ = ComputeAverageDeviation();
    time_ = event.time;

    if (static_cast<int32_t>(touchPoints_.size()) < fingers_ ||
        static_cast<int32_t>(activeFingers_.size()) < fingers_) {
        return;
    }
    if (refereeState_ == RefereeState::DETECTING) {
        if (GreatOrEqual(fabs(currentDev_ - initialDev_), distance_)) {
            scale_ = currentDev_ / initialDev_;
            auto onGestureJudgeBeginResult = TriggerGestureJudgeCallback();
            if (onGestureJudgeBeginResult == GestureJudgeResult::REJECT) {
                Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
                return;
            }
            Adjudicate(AceType::Claim(this), GestureDisposal::ACCEPT);
        }
    } else if (refereeState_ == RefereeState::SUCCEED) {
        scale_ = currentDev_ / initialDev_;
        if (isFlushTouchEventsEnd_) {
            SendCallbackMsg(onActionUpdate_);
        }
    }
}

void PinchRecognizer::OnFlushTouchEventsBegin()
{
    isFlushTouchEventsEnd_ = false;
}

void PinchRecognizer::OnFlushTouchEventsEnd()
{
    isFlushTouchEventsEnd_ = true;
}

void PinchRecognizer::HandleTouchMoveEvent(const AxisEvent& event)
{
    if (event.isRotationEvent || isPinchEnd_) {
        return;
    }
    if (NearZero(event.pinchAxisScale) && !IsCtrlBeingPressed()) {
        if (refereeState_ == RefereeState::DETECTING) {
            Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
            return;
        }
        if (refereeState_ == RefereeState::SUCCEED) {
            refereeState_ = RefereeState::READY;
            SendCallbackMsg(onActionEnd_);
            isPinchEnd_ = true;
            return;
        }
    }
    UpdateTouchPointWithAxisEvent(event);
    lastTouchEvent_ = touchPoints_[event.id];
    time_ = event.time;
    lastAxisEvent_ = event;
    if (refereeState_ == RefereeState::DETECTING || refereeState_ == RefereeState::SUCCEED) {
        if (event.pinchAxisScale != 0.0) {
            scale_ = event.pinchAxisScale;
        } else {
            if (GreatNotEqual(event.verticalAxis, 0.0) || GreatNotEqual(event.horizontalAxis, 0.0)) {
                scale_ -= SCALE_PER_AXIS_EVENT;
            } else if (LessNotEqual(event.verticalAxis, 0.0) || LessNotEqual(event.horizontalAxis, 0)) {
                scale_ += SCALE_PER_AXIS_EVENT;
            }
            if (LessNotEqual(scale_, 0.0)) {
                scale_ = 0.0;
            }
        }
        if (refereeState_ == RefereeState::DETECTING) {
            auto onGestureJudgeBeginResult = TriggerGestureJudgeCallback();
            if (onGestureJudgeBeginResult == GestureJudgeResult::REJECT) {
                Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
                return;
            }
            Adjudicate(AceType::Claim(this), GestureDisposal::ACCEPT);
        }
        SendCallbackMsg(onActionUpdate_);
    }
}

void PinchRecognizer::HandleTouchCancelEvent(const TouchEvent& event)
{
    if (!IsActiveFinger(event.id)) {
        return;
    }
    TAG_LOGI(AceLogTag::ACE_GESTURE, "Id:%{public}d, pinch %{public}d cancel", event.touchEventId, event.id);
    if ((refereeState_ != RefereeState::SUCCEED) && (refereeState_ != RefereeState::FAIL)) {
        Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
        return;
    }

    if (refereeState_ == RefereeState::SUCCEED && static_cast<int32_t>(activeFingers_.size()) == fingers_) {
        SendCancelMsg();
        refereeState_ = RefereeState::READY;
    }
}

void PinchRecognizer::HandleTouchCancelEvent(const AxisEvent& event)
{
    TAG_LOGI(AceLogTag::ACE_GESTURE, "Id:%{public}d, pinch axis cancel", event.touchEventId);
    if ((refereeState_ != RefereeState::SUCCEED) && (refereeState_ != RefereeState::FAIL)) {
        Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
        return;
    }

    if (refereeState_ == RefereeState::SUCCEED) {
        SendCancelMsg();
    }
}

double PinchRecognizer::ComputeAverageDeviation()
{
    // compute the coordinate of focal point
    double sumOfX = 0.0;
    double sumOfY = 0.0;
    for (auto& id : activeFingers_) {
        sumOfX = sumOfX + touchPoints_[id].x;
        sumOfY = sumOfY + touchPoints_[id].y;
    }
    double focalX = sumOfX / fingers_;
    double focalY = sumOfY / fingers_;

    // compute average deviation
    double devX = 0.0;
    double devY = 0.0;
    for (auto& id : activeFingers_) {
        devX = devX + fabs(touchPoints_[id].x - focalX);
        devY = devY + fabs(touchPoints_[id].y - focalY);
    }
    double aveDevX = devX / fingers_;
    double aveDevY = devY / fingers_;

    // compute zoom distance
    double zoomDistance = sqrt(pow(aveDevX, 2) + pow(aveDevY, 2));

    return zoomDistance;
}

Offset PinchRecognizer::ComputePinchCenter()
{
    if (touchPoints_.empty()) {
        return Offset();
    }
    double sumOfX = 0.0;
    double sumOfY = 0.0;
    for (auto& id : activeFingers_) {
        sumOfX = sumOfX + touchPoints_[id].x;
        sumOfY = sumOfY + touchPoints_[id].y;
    }
    double focalX = sumOfX / fingers_;
    double focalY = sumOfY / fingers_;

    PointF localPoint(focalX, focalY);
    NGGestureRecognizer::Transform(localPoint, GetAttachedNode(), false,
        isPostEventResult_, touchPoints_[touchPoints_.begin()->first].postEventNodeId);
    Offset pinchCenter = Offset(localPoint.GetX(), localPoint.GetY());

    return pinchCenter;
}

void PinchRecognizer::OnResetStatus()
{
    MultiFingersRecognizer::OnResetStatus();
    isPinchEnd_ = false;
}

void PinchRecognizer::SendCallbackMsg(const std::unique_ptr<GestureEventFunc>& callback)
{
    if (callback && *callback) {
        GestureEvent info;
        info.SetTimeStamp(time_);
        UpdateFingerListInfo();
        info.SetFingerList(fingerList_);
        info.SetScale(scale_);
        info.SetPinchCenter(pinchCenter_);
        info.SetDeviceId(deviceId_);
        info.SetSourceDevice(deviceType_);
        info.SetTarget(GetEventTarget().value_or(EventTarget()));
        info.SetForce(lastTouchEvent_.force);
        if (lastTouchEvent_.tiltX.has_value()) {
            info.SetTiltX(lastTouchEvent_.tiltX.value());
        }
        if (lastTouchEvent_.tiltY.has_value()) {
            info.SetTiltY(lastTouchEvent_.tiltY.value());
        }
        if (inputEventType_ == InputEventType::AXIS) {
            info.SetVerticalAxis(lastAxisEvent_.verticalAxis);
            info.SetHorizontalAxis(lastAxisEvent_.horizontalAxis);
            info.SetSourceTool(lastAxisEvent_.sourceTool);
        } else {
            info.SetSourceTool(lastTouchEvent_.sourceTool);
        }
        info.SetPointerEvent(lastPointEvent_);
        info.SetPressedKeyCodes(lastTouchEvent_.pressedKeyCodes_);
        // callback may be overwritten in its invoke so we copy it first
        auto callbackFunction = *callback;
        callbackFunction(info);
    }
}

GestureJudgeResult PinchRecognizer::TriggerGestureJudgeCallback()
{
    auto targetComponent = GetTargetComponent();
    CHECK_NULL_RETURN(targetComponent, GestureJudgeResult::CONTINUE);
    auto gestureRecognizerJudgeFunc = targetComponent->GetOnGestureRecognizerJudgeBegin();
    auto callback = targetComponent->GetOnGestureJudgeBeginCallback();
    if (!callback && !gestureRecognizerJudgeFunc) {
        return GestureJudgeResult::CONTINUE;
    }
    auto info = std::make_shared<PinchGestureEvent>();
    info->SetTimeStamp(time_);
    UpdateFingerListInfo();
    info->SetFingerList(fingerList_);
    info->SetScale(scale_);
    info->SetPinchCenter(pinchCenter_);
    info->SetSourceDevice(deviceType_);
    info->SetTarget(GetEventTarget().value_or(EventTarget()));
    info->SetForce(lastTouchEvent_.force);
    if (lastTouchEvent_.tiltX.has_value()) {
        info->SetTiltX(lastTouchEvent_.tiltX.value());
    }
    if (lastTouchEvent_.tiltY.has_value()) {
        info->SetTiltY(lastTouchEvent_.tiltY.value());
    }
    info->SetSourceTool(lastTouchEvent_.sourceTool);
    if (gestureRecognizerJudgeFunc) {
        return gestureRecognizerJudgeFunc(info, Claim(this), responseLinkRecognizer_);
    }
    return callback(gestureInfo_, info);
}

bool PinchRecognizer::ReconcileFrom(const RefPtr<NGGestureRecognizer>& recognizer)
{
    RefPtr<PinchRecognizer> curr = AceType::DynamicCast<PinchRecognizer>(recognizer);
    if (!curr) {
        ResetStatus();
        return false;
    }

    if (curr->fingers_ != fingers_ || curr->distance_ != distance_ || curr->priorityMask_ != priorityMask_) {
        ResetStatus();
        return false;
    }

    onActionStart_ = std::move(curr->onActionStart_);
    onActionUpdate_ = std::move(curr->onActionUpdate_);
    onActionEnd_ = std::move(curr->onActionEnd_);
    onActionCancel_ = std::move(curr->onActionCancel_);

    return true;
}

} // namespace OHOS::Ace::NG
