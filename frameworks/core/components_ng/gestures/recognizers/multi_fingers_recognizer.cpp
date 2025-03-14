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

#include "core/components_ng/gestures/recognizers/multi_fingers_recognizer.h"

#include "base/memory/ace_type.h"
#include "core/components_ng/gestures/recognizers/recognizer_group.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t DEFAULT_MAX_FINGERS = 10;
} // namespace

MultiFingersRecognizer::MultiFingersRecognizer(int32_t fingers)
{
    if (fingers > DEFAULT_MAX_FINGERS || fingers <= 0) {
        fingers_ = 1;
    } else {
        fingers_ = fingers;
    }
}

void MultiFingersRecognizer::UpdateFingerListInfo()
{
    fingerList_.clear();
    lastPointEvent_.reset();
    auto maxTimeStamp = TimeStamp::min().time_since_epoch().count();
    for (const auto& point : touchPoints_) {
        PointF localPoint(point.second.x, point.second.y);
        NGGestureRecognizer::Transform(
            localPoint, GetAttachedNode(), false, isPostEventResult_, point.second.postEventNodeId);
        FingerInfo fingerInfo = { point.second.originalId, point.second.GetOffset(),
            Offset(localPoint.GetX(), localPoint.GetY()), point.second.GetScreenOffset(), point.second.sourceType,
            point.second.sourceTool };
        fingerList_.emplace_back(fingerInfo);
        if (maxTimeStamp <= point.second.GetTimeStamp().time_since_epoch().count()
            && point.second.pointers.size() >= touchPoints_.size()) {
            lastPointEvent_ = point.second.pointerEvent;
            maxTimeStamp = point.second.GetTimeStamp().time_since_epoch().count();
        }
    }
}

bool MultiFingersRecognizer::IsNeedResetStatus()
{
    if (!touchPoints_.empty()) {
        return false;
    }

    auto ref = AceType::Claim(this);
    auto group = AceType::DynamicCast<RecognizerGroup>(ref);
    if (!group) {
        return true;
    }

    auto groupList = group->GetGroupRecognizer();
    for (auto &recognizer : groupList) {
        auto multiFingersRecognizer = AceType::DynamicCast<MultiFingersRecognizer>(recognizer);
        if (!multiFingersRecognizer) {
            continue;
        }

        if (!multiFingersRecognizer->IsNeedResetStatus()) {
            return false;
        }
    }

    return true;
}

void MultiFingersRecognizer::OnFinishGestureReferee(int32_t touchId, bool isBlocked)
{
    touchPoints_.erase(touchId);
    activeFingers_.remove(touchId);
    if (IsNeedResetStatus()) {
        ResetStatusOnFinish(isBlocked);
    }
}

void MultiFingersRecognizer::CleanRecognizerState()
{
    if ((refereeState_ == RefereeState::SUCCEED ||
        refereeState_ == RefereeState::FAIL ||
        refereeState_ == RefereeState::DETECTING) &&
        currentFingers_ == 0) {
        refereeState_ = RefereeState::READY;
        disposal_ = GestureDisposal::NONE;
    }
}

void MultiFingersRecognizer::UpdateTouchPointWithAxisEvent(const AxisEvent& event)
{
    // Update touchPointInfo with axisEvent.
    touchPoints_[event.id].id = event.id;
    touchPoints_[event.id].x = event.x;
    touchPoints_[event.id].y = event.y;
    touchPoints_[event.id].screenX = event.screenX;
    touchPoints_[event.id].screenY = event.screenY;
    touchPoints_[event.id].sourceType = event.sourceType;
    touchPoints_[event.id].sourceTool = event.sourceTool;
    touchPoints_[event.id].originalId = event.originalId;
}
} // namespace OHOS::Ace::NG
