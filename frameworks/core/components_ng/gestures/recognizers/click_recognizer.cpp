/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/gestures/recognizers/click_recognizer.h"

#include "base/geometry/offset.h"
#include "base/log/log.h"
#include "base/ressched/ressched_report.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/gestures/base_gesture_event.h"
#include "core/components_ng/gestures/gesture_referee.h"
#include "core/components_ng/gestures/recognizers/gesture_recognizer.h"
#include "core/components_ng/gestures/recognizers/multi_fingers_recognizer.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {

int32_t MULTI_FINGER_TIMEOUT = 300;
constexpr int32_t MULTI_FINGER_TIMEOUT_TOUCH = 300;
constexpr int32_t MULTI_FINGER_TIMEOUT_MOUSE = 300;
int32_t MULTI_TAP_TIMEOUT = 300;
constexpr int32_t MULTI_TAP_TIMEOUT_TOUCH = 300;
constexpr int32_t MULTI_TAP_TIMEOUT_MOUSE = 300;
constexpr int32_t MAX_THRESHOLD_MANYTAP = 60;
constexpr int32_t MAX_TAP_FINGERS = 10;
constexpr double MAX_THRESHOLD = 20.0;
constexpr int32_t DEFAULT_TAP_FINGERS = 1;
constexpr int32_t DEFAULT_LONGPRESS_DURATION = 800000000;

} // namespace

void ClickRecognizer::ForceCleanRecognizer()
{
    MultiFingersRecognizer::ForceCleanRecognizer();
    OnResetStatus();
}

bool ClickRecognizer::IsPointInRegion(const TouchEvent& event)
{
    PointF localPoint(event.x, event.y);
    auto frameNode = GetAttachedNode();
    if (!frameNode.Invalid()) {
        auto host = frameNode.Upgrade();
        CHECK_NULL_RETURN(host, false);
        NGGestureRecognizer::Transform(localPoint, frameNode, false, isPostEventResult_, event.postEventNodeId);
        auto renderContext = host->GetRenderContext();
        CHECK_NULL_RETURN(renderContext, false);
        auto paintRect = renderContext->GetPaintRectWithoutTransform();
        localPoint = localPoint + paintRect.GetOffset();
        if (!host->InResponseRegionList(localPoint, responseRegionBuffer_)) {
            TAG_LOGI(AceLogTag::ACE_GESTURE,
                "InputTracking id:%{public}d, this MOVE/UP event is out of region, try to reject click gesture",
                event.touchEventId);
            Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
            return false;
        }
    }
    return true;
}

ClickRecognizer::ClickRecognizer(int32_t fingers, int32_t count) : MultiFingersRecognizer(fingers), count_(count)
{
    if (fingers_ > MAX_TAP_FINGERS || fingers_ < DEFAULT_TAP_FINGERS) {
        fingers_ = DEFAULT_TAP_FINGERS;
    }
}

void ClickRecognizer::InitGlobalValue(SourceType sourceType)
{
    switch (sourceType) {
        case SourceType::TOUCH:
            MULTI_FINGER_TIMEOUT = MULTI_FINGER_TIMEOUT_TOUCH;
            MULTI_TAP_TIMEOUT = MULTI_TAP_TIMEOUT_TOUCH;
            break;
        case SourceType::MOUSE:
        case SourceType::TOUCH_PAD:
            MULTI_FINGER_TIMEOUT = MULTI_FINGER_TIMEOUT_MOUSE;
            MULTI_TAP_TIMEOUT = MULTI_TAP_TIMEOUT_MOUSE;
            break;
        default:
            break;
    }
}

ClickInfo ClickRecognizer::GetClickInfo()
{
    TouchEvent touchPoint = {};
    if (!touchPoints_.empty()) {
        touchPoint = touchPoints_.begin()->second;
    }
    ClickInfo info(touchPoint.id);
    PointF localPoint(touchPoint.GetOffset().GetX(), touchPoint.GetOffset().GetY());
    NGGestureRecognizer::Transform(localPoint, GetAttachedNode(), false,
        isPostEventResult_, touchPoint.postEventNodeId);
    Offset localOffset(localPoint.GetX(), localPoint.GetY());
    info.SetTimeStamp(touchPoint.time);
    info.SetScreenLocation(touchPoint.GetScreenOffset());
    info.SetGlobalLocation(touchPoint.GetOffset()).SetLocalLocation(localOffset);
    info.SetSourceDevice(deviceType_);
    info.SetDeviceId(deviceId_);
    info.SetTarget(GetEventTarget().value_or(EventTarget()));
    info.SetForce(touchPoint.force);
    auto frameNode = GetAttachedNode().Upgrade();
    std::string patternName = "";
    if (frameNode) {
        patternName = frameNode->GetTag();
    }
    info.SetPatternName(patternName.c_str());
    if (touchPoint.tiltX.has_value()) {
        info.SetTiltX(touchPoint.tiltX.value());
    }
    if (touchPoint.tiltY.has_value()) {
        info.SetTiltY(touchPoint.tiltY.value());
    }
    info.SetSourceTool(touchPoint.sourceTool);
    return info;
}

void ClickRecognizer::OnAccepted()
{
    int64_t acceptTime = GetSysTimestamp();
    int64_t inputTime = acceptTime;
    if (firstInputTime_.has_value()) {
        inputTime = static_cast<int64_t>(firstInputTime_.value().time_since_epoch().count());
    }
    if (SystemProperties::GetTraceInputEventEnabled()) {
        ACE_SCOPED_TRACE("UserEvent InputTime:%lld AcceptTime:%lld InputType:ClickGesture",
            static_cast<long long>(inputTime), static_cast<long long>(acceptTime));
    }
    firstInputTime_.reset();

    auto node = GetAttachedNode().Upgrade();
    TAG_LOGI(AceLogTag::ACE_GESTURE, "Click accepted, tag: %{public}s, id: %{public}s",
        node ? node->GetTag().c_str() : "null", node ? std::to_string(node->GetId()).c_str() : "invalid");
    if (onAccessibilityEventFunc_) {
        onAccessibilityEventFunc_(AccessibilityEventType::CLICK);
    }
    refereeState_ = RefereeState::SUCCEED;
    ResSchedReport::GetInstance().ResSchedDataReport("click");
    TouchEvent touchPoint = {};
    if (!touchPoints_.empty()) {
        touchPoint = touchPoints_.begin()->second;
    }
    PointF localPoint(touchPoint.GetOffset().GetX(), touchPoint.GetOffset().GetY());
    NGGestureRecognizer::Transform(localPoint, GetAttachedNode(), false,
        isPostEventResult_, touchPoint.postEventNodeId);
    Offset localOffset(localPoint.GetX(), localPoint.GetY());
    if (onClick_) {
        ClickInfo info = GetClickInfo();
        onClick_(info);
    }

    if (remoteMessage_) {
        ClickInfo info = GetClickInfo();
        info.SetTimeStamp(touchPoint.time);
        info.SetGlobalLocation(touchPoint.GetOffset()).SetLocalLocation(localOffset);
        remoteMessage_(info);
    }
    UpdateFingerListInfo();
    SendCallbackMsg(onAction_);

    int64_t overTime = GetSysTimestamp();
    if (SystemProperties::GetTraceInputEventEnabled()) {
        ACE_SCOPED_TRACE("UserEvent InputTime:%lld OverTime:%lld InputType:ClickGesture",
            static_cast<long long>(inputTime), static_cast<long long>(overTime));
    }
    firstInputTime_.reset();
}

void ClickRecognizer::OnRejected()
{
    refereeState_ = RefereeState::FAIL;
    firstInputTime_.reset();
}

void ClickRecognizer::HandleTouchDownEvent(const TouchEvent& event)
{
    TAG_LOGI(AceLogTag::ACE_GESTURE,
        "Id:%{public}d, click %{public}d down, ETF: %{public}d, CTP: %{public}d, state: %{public}d",
        event.touchEventId, event.id, equalsToFingers_, currentTouchPointsNum_, refereeState_);
    if (!firstInputTime_.has_value()) {
        firstInputTime_ = event.time;
    }

    auto pipeline = PipelineBase::GetCurrentContext();
    if (pipeline && pipeline->IsFormRender()) {
        touchDownTime_ = event.time;
    }
    if (IsRefereeFinished()) {
        auto node = GetAttachedNode().Upgrade();
        TAG_LOGI(AceLogTag::ACE_GESTURE,
            "Click recognizer handle touch down event refereeState is %{public}d, node tag = %{public}s, id = "
            "%{public}s",
            refereeState_, node ? node->GetTag().c_str() : "null",
            node ? std::to_string(node->GetId()).c_str() : "invalid");
        return;
    }
    InitGlobalValue(event.sourceType);
    if (!IsInAttachedNode(event, false)) {
        Adjudicate(Claim(this), GestureDisposal::REJECT);
        return;
    }
    // The last recognition sequence has been completed, reset the timer.
    if (tappedCount_ > 0 && currentTouchPointsNum_ == 0) {
        responseRegionBuffer_.clear();
        tapDeadlineTimer_.Cancel();
    }
    if (currentTouchPointsNum_ == 0) {
        auto frameNode = GetAttachedNode();
        if (!frameNode.Invalid()) {
            auto host = frameNode.Upgrade();
            responseRegionBuffer_ = host->GetResponseRegionListForRecognizer(static_cast<int32_t>(event.sourceType));
        }
    }
    if (fingersId_.find(event.id) == fingersId_.end()) {
        fingersId_.insert(event.id);
        ++currentTouchPointsNum_;
        touchPoints_[event.id] = event;
    }
    UpdateFingerListInfo();
    if (fingers_ > currentTouchPointsNum_) {
        // waiting for multi-finger press
        DeadlineTimer(fingerDeadlineTimer_, MULTI_FINGER_TIMEOUT);
    } else {
        // Turn off the multi-finger press deadline timer
        fingerDeadlineTimer_.Cancel();
        equalsToFingers_ = true;
        if (ExceedSlop()) {
            TAG_LOGW(AceLogTag::ACE_GESTURE, "Fail to detect multi finger tap due to offset is out of slop");
            Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
        }
    }
    if (currentTouchPointsNum_ == fingers_) {
        focusPoint_ = ComputeFocusPoint();
    }
}

void ClickRecognizer::HandleTouchUpEvent(const TouchEvent& event)
{
    TAG_LOGI(AceLogTag::ACE_GESTURE, "Id:%{public}d, click %{public}d up, state: %{public}d", event.touchEventId,
        event.id, refereeState_);
    auto pipeline = PipelineBase::GetCurrentContext();
    // In a card scenario, determine the interval between finger pressing and finger lifting. Delete this section of
    // logic when the formal scenario is complete.
    if (pipeline && pipeline->IsFormRender()) {
        Offset offset = event.GetScreenOffset() - touchPoints_[event.id].GetScreenOffset();
        if (event.time.time_since_epoch().count() - touchDownTime_.time_since_epoch().count() >
            DEFAULT_LONGPRESS_DURATION || offset.GetDistance() > MAX_THRESHOLD) {
            TAG_LOGI(AceLogTag::ACE_GESTURE, "reject click when up, offset is %{public}f",
                static_cast<float>(offset.GetDistance()));
            Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
            return;
        }
    }
    if (IsRefereeFinished()) {
        return;
    }
    InitGlobalValue(event.sourceType);
    touchPoints_[event.id] = event;
    UpdateFingerListInfo();
    auto isUpInRegion = IsPointInRegion(event);
    if (fingersId_.find(event.id) != fingersId_.end()) {
        fingersId_.erase(event.id);
        --currentTouchPointsNum_;
    }
    if (currentTouchPointsNum_ == 0) {
        responseRegionBuffer_.clear();
    }
    // Check whether multi-finger taps are completed in count_ times
    if (equalsToFingers_ && (currentTouchPointsNum_ == 0) && isUpInRegion) {
        // Turn off the multi-finger lift deadline timer
        fingerDeadlineTimer_.Cancel();
        tappedCount_++;

        if (tappedCount_ == count_) {
            TAG_LOGI(AceLogTag::ACE_GESTURE, "Click try accept");
            time_ = event.time;
            if (!useCatchMode_) {
                OnAccepted();
                return;
            }
            auto onGestureJudgeBeginResult = TriggerGestureJudgeCallback();
            if (onGestureJudgeBeginResult == GestureJudgeResult::REJECT) {
                Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
                return;
            }
            Adjudicate(AceType::Claim(this), GestureDisposal::ACCEPT);
            return;
        }
        equalsToFingers_ = false;
        // waiting for multi-finger lift
        DeadlineTimer(tapDeadlineTimer_, MULTI_TAP_TIMEOUT);
    }

    if (refereeState_ != RefereeState::PENDING && refereeState_ != RefereeState::FAIL) {
        Adjudicate(AceType::Claim(this), GestureDisposal::PENDING);
    }

    if (currentTouchPointsNum_ < fingers_ && equalsToFingers_) {
        DeadlineTimer(fingerDeadlineTimer_, MULTI_FINGER_TIMEOUT);
    }
}

void ClickRecognizer::HandleTouchMoveEvent(const TouchEvent& event)
{
    if (currentFingers_ < fingers_) {
        return;
    }
    if (IsRefereeFinished()) {
        return;
    }
    InitGlobalValue(event.sourceType);
    // In form scenario, if move more than 20vp, reject click gesture.
    // Remove form scenario when formal solution is completed.
    auto pipeline = PipelineBase::GetCurrentContext();
    if (pipeline && pipeline->IsFormRender()) {
        Offset offset = event.GetScreenOffset() - touchPoints_[event.id].GetScreenOffset();
        if (offset.GetDistance() > MAX_THRESHOLD) {
            TAG_LOGI(AceLogTag::ACE_GESTURE, "This gesture is out of offset, try to reject it");
            Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
        }
    }
    IsPointInRegion(event);
    UpdateFingerListInfo();
}

void ClickRecognizer::HandleTouchCancelEvent(const TouchEvent& event)
{
    TAG_LOGI(AceLogTag::ACE_GESTURE, "Id:%{public}d, click %{public}d cancel", event.touchEventId, event.id);
    if (IsRefereeFinished()) {
        return;
    }
    InitGlobalValue(event.sourceType);
    Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
}

void ClickRecognizer::HandleOverdueDeadline()
{
    if (currentTouchPointsNum_ < fingers_ || tappedCount_ < count_) {
        Adjudicate(AceType::Claim(this), GestureDisposal::REJECT);
    }
}

void ClickRecognizer::DeadlineTimer(CancelableCallback<void()>& deadlineTimer, int32_t time)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);

    auto&& callback = [weakPtr = AceType::WeakClaim(this)]() {
        auto refPtr = weakPtr.Upgrade();
        if (refPtr) {
            refPtr->HandleOverdueDeadline();
        }
    };

    deadlineTimer.Reset(callback);
    auto taskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    taskExecutor.PostDelayedTask(deadlineTimer, time, "ArkUIGestureClickDeadlineTimer");
}

Offset ClickRecognizer::ComputeFocusPoint()
{
    Offset sumOfPoints;
    int32_t count = 0;
    for (auto& element : touchPoints_) {
        if (count >= fingers_) {
            break;
        }
        sumOfPoints = sumOfPoints + element.second.GetOffset();
        count++;
    }
    Offset focusPoint = sumOfPoints / count;
    return focusPoint;
}

bool ClickRecognizer::ExceedSlop()
{
    if (tappedCount_ > 0 && tappedCount_ < count_) {
        Offset currentFocusPoint = ComputeFocusPoint();
        Offset slop = currentFocusPoint - focusPoint_;
        if (GreatOrEqual(PipelineBase::Px2VpWithCurrentDensity(slop.GetDistance()), MAX_THRESHOLD_MANYTAP)) {
            return true;
        }
    }
    return false;
}

GestureEvent ClickRecognizer::GetGestureEventInfo()
{
    GestureEvent info;
    info.SetTimeStamp(time_);
    info.SetFingerList(fingerList_);
    TouchEvent touchPoint = {};
    for (const auto& pointKeyVal : touchPoints_) {
        auto pointVal = pointKeyVal.second;
        if (pointVal.sourceType != SourceType::NONE) {
            touchPoint = pointVal;
            break;
        }
    }
    PointF localPoint(touchPoint.GetOffset().GetX(), touchPoint.GetOffset().GetY());
    NGGestureRecognizer::Transform(localPoint, GetAttachedNode(), false,
        isPostEventResult_, touchPoint.postEventNodeId);
    info.SetTimeStamp(touchPoint.time);
    info.SetScreenLocation(touchPoint.GetScreenOffset());
    info.SetGlobalLocation(touchPoint.GetOffset()).SetLocalLocation(Offset(localPoint.GetX(), localPoint.GetY()));
    info.SetSourceDevice(deviceType_);
    info.SetDeviceId(deviceId_);
    info.SetTarget(GetEventTarget().value_or(EventTarget()));
    info.SetForce(touchPoint.force);
    auto frameNode = GetAttachedNode().Upgrade();
    std::string patternName = "";
    if (frameNode) {
        patternName = frameNode->GetTag();
    }
    info.SetPatternName(patternName.c_str());
    
    if (touchPoint.tiltX.has_value()) {
        info.SetTiltX(touchPoint.tiltX.value());
    }
    if (touchPoint.tiltY.has_value()) {
        info.SetTiltY(touchPoint.tiltY.value());
    }
    info.SetSourceTool(touchPoint.sourceTool);
#ifdef SECURITY_COMPONENT_ENABLE
    info.SetDisplayX(touchPoint.screenX);
    info.SetDisplayY(touchPoint.screenY);
#endif
    info.SetPointerEvent(lastPointEvent_);
    info.SetPressedKeyCodes(touchPoint.pressedKeyCodes_);
    return info;
}

void ClickRecognizer::SendCallbackMsg(const std::unique_ptr<GestureEventFunc>& onAction)
{
    if (onAction && *onAction) {
        GestureEvent info = GetGestureEventInfo();
        // onAction may be overwritten in its invoke so we copy it first
        auto onActionFunction = *onAction;
        onActionFunction(info);
    }
}

GestureJudgeResult ClickRecognizer::TriggerGestureJudgeCallback()
{
    auto targetComponent = GetTargetComponent();
    CHECK_NULL_RETURN(targetComponent, GestureJudgeResult::CONTINUE);
    auto gestureRecognizerJudgeFunc = targetComponent->GetOnGestureRecognizerJudgeBegin();
    auto callback = targetComponent->GetOnGestureJudgeBeginCallback();
    if (!callback && !sysJudge_ && !gestureRecognizerJudgeFunc) {
        return GestureJudgeResult::CONTINUE;
    }
    auto info = std::make_shared<TapGestureEvent>();
    info->SetTimeStamp(time_);
    info->SetFingerList(fingerList_);
    TouchEvent touchPoint = {};
    if (!touchPoints_.empty()) {
        touchPoint = touchPoints_.begin()->second;
    }
    info->SetSourceDevice(deviceType_);
    info->SetTarget(GetEventTarget().value_or(EventTarget()));
    info->SetForce(touchPoint.force);
    if (touchPoint.tiltX.has_value()) {
        info->SetTiltX(touchPoint.tiltX.value());
    }
    if (touchPoint.tiltY.has_value()) {
        info->SetTiltY(touchPoint.tiltY.value());
    }
    info->SetSourceTool(touchPoint.sourceTool);
    if (sysJudge_) {
        return sysJudge_(gestureInfo_, info);
    }
    if (gestureRecognizerJudgeFunc) {
        return gestureRecognizerJudgeFunc(info, Claim(this), responseLinkRecognizer_);
    }
    return callback(gestureInfo_, info);
}

bool ClickRecognizer::ReconcileFrom(const RefPtr<NGGestureRecognizer>& recognizer)
{
    RefPtr<ClickRecognizer> curr = AceType::DynamicCast<ClickRecognizer>(recognizer);
    if (!curr) {
        ResetStatus();
        return false;
    }

    if (curr->count_ != count_ || curr->fingers_ != fingers_ || curr->priorityMask_ != priorityMask_) {
        ResetStatus();
        return false;
    }

    onAction_ = std::move(curr->onAction_);
    return true;
}

RefPtr<GestureSnapshot> ClickRecognizer::Dump() const
{
    RefPtr<GestureSnapshot> info = NGGestureRecognizer::Dump();
    std::stringstream oss;
    oss << "count: " << count_ << ", "
        << "fingers: " << fingers_;
    info->customInfo = oss.str();
    return info;
}

RefPtr<Gesture> ClickRecognizer::CreateGestureFromRecognizer() const
{
    return AceType::MakeRefPtr<TapGesture>(count_, fingers_);
}

void ClickRecognizer::CleanRecognizerState()
{
    if ((refereeState_ == RefereeState::SUCCEED ||
        refereeState_ == RefereeState::FAIL ||
        refereeState_ == RefereeState::DETECTING) &&
        currentFingers_ == 0) {
        tappedCount_ = 0;
        refereeState_ = RefereeState::READY;
        disposal_ = GestureDisposal::NONE;
    }
}
} // namespace OHOS::Ace::NG
