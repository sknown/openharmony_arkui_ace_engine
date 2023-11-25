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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_GESTURES_RECOGNIZERS_CLICK_RECOGNIZER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_GESTURES_RECOGNIZERS_CLICK_RECOGNIZER_H

#include <functional>

#include "base/geometry/ng/rect_t.h"
#include "base/geometry/ng/point_t.h"
#include "base/thread/cancelable_callback.h"
#include "core/accessibility/accessibility_utils.h"
#include "core/components_ng/gestures/recognizers/multi_fingers_recognizer.h"
#include "core/gestures/click_info.h"

namespace OHOS::Ace::NG {
using OnAccessibilityEventFunc = std::function<void(AccessibilityEventType)>;

class ClickRecognizer : public MultiFingersRecognizer {
    DECLARE_ACE_TYPE(ClickRecognizer, MultiFingersRecognizer);

public:
    ClickRecognizer() = default;
    ClickRecognizer(int32_t fingers, int32_t count);

    ~ClickRecognizer() override = default;

    void OnAccepted() override;
    void OnRejected() override;

    void SetOnClick(const ClickCallback& onClick)
    {
        onClick_ = onClick;
    }

    void SetRemoteMessage(const ClickCallback& remoteMessage)
    {
        remoteMessage_ = remoteMessage;
    }

    void SetUseCatchMode(bool useCatchMode)
    {
        useCatchMode_ = useCatchMode;
    }

    void SetOnAccessibility(OnAccessibilityEventFunc onAccessibilityEvent)
    {
        onAccessibilityEventFunc_ = std::move(onAccessibilityEvent);
    }

    int GetCount()
    {
        return count_;
    }

    GestureEventFunc GetTapActionFunc()
    {
        auto callback = [weak = WeakClaim(this)](GestureEvent& info) {
            auto clickRecognizer = weak.Upgrade();
            CHECK_NULL_VOID(clickRecognizer);
            if (clickRecognizer->onAction_) {
                (*(clickRecognizer->onAction_))(info);
            }
        };
        return callback;
    }

    virtual RefPtr<GestureSnapshot> Dump() const override;

private:
    // Recognize whether MOVE/UP event is in response region.
    void IsPointInRegion(const TouchEvent& event);
    void HandleTouchDownEvent(const TouchEvent& event) override;
    void HandleTouchUpEvent(const TouchEvent& event) override;
    void HandleTouchMoveEvent(const TouchEvent& event) override;
    void HandleTouchCancelEvent(const TouchEvent& event) override;
    bool ReconcileFrom(const RefPtr<NGGestureRecognizer>& recognizer) override;

    void OnResetStatus() override
    {
        MultiFingersRecognizer::OnResetStatus();
        tappedCount_ = 0;
        equalsToFingers_ = false;
        focusPoint_ = {};
        fingerDeadlineTimer_.Cancel();
        tapDeadlineTimer_.Cancel();
        currentTouchPointsNum_ = 0;
    }

    void HandleOverdueDeadline();
    void DeadlineTimer(CancelableCallback<void()>& deadlineTimer, int32_t time);
    Offset ComputeFocusPoint();

    void SendCallbackMsg(const std::unique_ptr<GestureEventFunc>& callback);
    GestureJudgeResult TriggerGestureJudgeCallback();
    bool ExceedSlop();
    void InitGlobalValue(SourceType deviceId);

    bool CheckNeedReceiveEvent();

    int32_t count_ = 1;

    // number of tap action.
    int32_t tappedCount_ = 0;

    // Check whether the touch point num has reached the configured value
    bool equalsToFingers_ = false;
    // the time when gesture recognition is successful
    TimeStamp time_;
    Offset focusPoint_;
    TimeStamp touchDownTime_;

    ClickCallback onClick_;
    ClickCallback remoteMessage_;
    bool useCatchMode_ = true;
    CancelableCallback<void()> fingerDeadlineTimer_;
    CancelableCallback<void()> tapDeadlineTimer_;
    std::vector<RectF> responseRegionBuffer_;

    int32_t currentTouchPointsNum_ = 0;

    OnAccessibilityEventFunc onAccessibilityEventFunc_ = nullptr;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_GESTURES_RECOGNIZERS_CLICK_RECOGNIZER_H
