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
    uint64_t timeStamp = 0;
    for (const auto& point : touchPoints_) {
        PointF localPoint(point.second.x, point.second.y);
        NGGestureRecognizer::Transform(localPoint, GetAttachedNode(), false,
            isPostEventResult_, point.second.postEventNodeId);
        FingerInfo fingerInfo = { point.first, point.second.GetOffset(), Offset(localPoint.GetX(), localPoint.GetY()),
            point.second.sourceType, point.second.sourceTool, point.second.originalId };
        fingerList_.emplace_back(fingerInfo);
        uint64_t currentTimeStamp = point.second.GetTimeStamp().time_since_epoch().count();
        if (timeStamp < currentTimeStamp) {
            lastPointEvent_ = point.second.pointerEvent;
            timeStamp = currentTimeStamp;
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
    if ((refereeState_ == RefereeState::SUCCEED || refereeState_ == RefereeState::FAIL) &&
        currentFingers_ == 0) {
        refereeState_ = RefereeState::READY;
        disposal_ = GestureDisposal::NONE;
    }
}
} // namespace OHOS::Ace::NG
