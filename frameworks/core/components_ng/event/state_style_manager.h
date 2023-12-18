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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_STATE_STYLE_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_STATE_STYLE_MANAGER_H

#include <set>

#include "base/geometry/ng/point_t.h"
#include "base/geometry/offset.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/thread/cancelable_callback.h"

namespace OHOS::Ace::NG {

class FrameNode;
class TouchEventImpl;

using UIState = uint64_t;
inline constexpr UIState UI_STATE_NORMAL = 0;
inline constexpr UIState UI_STATE_PRESSED = 1;
inline constexpr UIState UI_STATE_FOCUSED = 1 << 1;
inline constexpr UIState UI_STATE_DISABLED = 1 << 2;
// used for radio, checkbox, switch.
inline constexpr UIState UI_STATE_SELECTED = 1 << 3;

// StateStyleManager is mainly used to manage the setting and refresh of state styles.
class StateStyleManager : public virtual AceType {
    DECLARE_ACE_TYPE(StateStyleManager, AceType)

public:
    explicit StateStyleManager(WeakPtr<FrameNode> frameNode);
    ~StateStyleManager() override;

    bool HasStateStyle(UIState state) const
    {
        return (supportedStates_ & state) == state;
    }

    UIState GetCurrentUIState() const
    {
        return currentState_;
    }

    void AddSupportedState(UIState state)
    {
        supportedStates_ = supportedStates_ | state;
    }

    void SetSupportedStates(UIState state)
    {
        supportedStates_ = state;
    }

    bool IsCurrentStateOn(UIState state) const
    {
        if (state == UI_STATE_NORMAL) {
            return currentState_ == state;
        }
        return (currentState_ & state) == state;
    }

    void SetCurrentUIState(UIState state, bool flag)
    {
        if (flag) {
            currentState_ |= state;
        } else {
            currentState_ &= ~state;
        }
    }

    void UpdateCurrentUIState(UIState state)
    {
        if (!HasStateStyle(state)) {
            return;
        }
        auto temp = currentState_ | state;
        if (temp != currentState_) {
            currentState_ = temp;
            FireStateFunc();
        }
    }

    void ResetCurrentUIState(UIState state)
    {
        if (!HasStateStyle(state)) {
            return;
        }
        if ((currentState_ & state) != state) {
            return;
        }
        auto temp = currentState_ ^ state;
        if (temp != currentState_) {
            currentState_ = temp;
            FireStateFunc();
        }
    }

    const RefPtr<TouchEventImpl>& GetPressedListener();

private:
    void FireStateFunc();

    void PostPressStyleTask(uint32_t delayTime);

    bool HandleScrollingParent();

    void CancelPressStyleTask()
    {
        if (pressStyleTask_) {
            pressStyleTask_.Cancel();
        }
    }

    bool IsPressedStatePending()
    {
        return pressedPendingState_;
    }

    void ResetPressedPendingState()
    {
        pressedPendingState_ = false;
    }

    void PendingPressedState()
    {
        pressedPendingState_ = true;
    }

    void ResetPressedState()
    {
        ResetCurrentUIState(UI_STATE_PRESSED);
        CancelPressStyleTask();
        ResetPressedPendingState();
    }

    bool IsOutOfPressedRegion(int32_t sourceType, const Offset& location) const;
    void Transform(PointF& localPointF, const WeakPtr<FrameNode>& node) const;

    WeakPtr<FrameNode> host_;
    RefPtr<TouchEventImpl> pressedFunc_;

    UIState supportedStates_ = UI_STATE_NORMAL;
    UIState currentState_ = UI_STATE_NORMAL;

    std::set<int32_t> pointerId_;
    CancelableCallback<void()> pressStyleTask_;
    bool pressedPendingState_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(StateStyleManager);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_STATE_STYLE_MANAGER_H