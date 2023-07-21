/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SWIPER_SWIPER_CONTROLLER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SWIPER_SWIPER_CONTROLLER_H

#include <functional>

#include "base/memory/ace_type.h"

namespace OHOS::Ace {

using CommonFunc = std::function<void()>;
using SwipeToImpl = std::function<void(const int32_t, bool)>;
using SwipeToWithoutAnimationImpl = std::function<void(const int32_t)>;
using TurnPageRateFunc = std::function<void(const int32_t, float)>;

class SwiperController : public virtual AceType {
    DECLARE_ACE_TYPE(SwiperController, AceType);

public:
    void SwipeTo(int32_t index, bool reverse = false)
    {
        if (swipeToImpl_) {
            swipeToImpl_(index, reverse);
        }
    }

    void SetSwipeToImpl(const SwipeToImpl& swipeToImpl)
    {
        swipeToImpl_ = swipeToImpl;
    }

    void SwipeToWithoutAnimation(int32_t index)
    {
        if (swipeToWithoutAnimationImpl_) {
            swipeToWithoutAnimationImpl_(index);
        }
    }

    void SetSwipeToWithoutAnimationImpl(const SwipeToWithoutAnimationImpl& swipeToWithoutAnimationImpl)
    {
        swipeToWithoutAnimationImpl_ = swipeToWithoutAnimationImpl;
    }

    void ShowPrevious()
    {
        if (showPrevImpl_) {
            showPrevImpl_();
        }
    }

    void SetShowPrevImpl(const CommonFunc& showPrevImpl)
    {
        showPrevImpl_ = showPrevImpl;
    }

    void ShowNext()
    {
        if (showNextImpl_) {
            showNextImpl_();
        }
    }

    void SetShowNextImpl(const CommonFunc& showNextImpl)
    {
        showNextImpl_ = showNextImpl;
    }

    void FinishAnimation() const
    {
        if (finishImpl_) {
            finishImpl_();
        }
    }

    void SetFinishImpl(const CommonFunc& finishImpl)
    {
        finishImpl_ = finishImpl;
    }

    void SetFinishCallback(const CommonFunc& onFinish)
    {
        finishCallback_ = onFinish;
    }

    const CommonFunc& GetFinishCallback() const
    {
        return finishCallback_;
    }

    bool HasInitialized() const
    {
        return showPrevImpl_ && showNextImpl_ && finishImpl_;
    }

    void SetTabBarFinishCallback(const CommonFunc& onTabBarFinish)
    {
        tabBarFinishCallback_ = onTabBarFinish;
    }

    const CommonFunc& GetTabBarFinishCallback() const
    {
        return tabBarFinishCallback_;
    }

    void SetRemoveTabBarEventCallback(const CommonFunc& removeTabBarEventCallback)
    {
        removeTabBarEventCallback_ = removeTabBarEventCallback;
    }

    const CommonFunc& GetRemoveTabBarEventCallback() const
    {
        return removeTabBarEventCallback_;
    }

    void SetAddTabBarEventCallback(const CommonFunc& addTabBarEventCallback)
    {
        addTabBarEventCallback_ = addTabBarEventCallback;
    }

    const CommonFunc& GetAddTabBarEventCallback() const
    {
        return addTabBarEventCallback_;
    }

    void SetRemoveSwiperEventCallback(const CommonFunc& removeSwiperEventCallback)
    {
        removeSwiperEventCallback_ = removeSwiperEventCallback;
    }

    const CommonFunc& GetRemoveSwiperEventCallback() const
    {
        return removeSwiperEventCallback_;
    }

    void SetAddSwiperEventCallback(const CommonFunc& addSwiperEventCallback)
    {
        addSwiperEventCallback_ = addSwiperEventCallback;
    }

    const CommonFunc& GetAddSwiperEventCallback() const
    {
        return addSwiperEventCallback_;
    }

    void SetTurnPageRateCallback(const TurnPageRateFunc& turnPageRateCallback)
    {
        turnPageRateCallback_ = turnPageRateCallback;
    }

    const TurnPageRateFunc& GetTurnPageRateCallback() const
    {
        return turnPageRateCallback_;
    }

private:
    SwipeToImpl swipeToImpl_;
    SwipeToWithoutAnimationImpl swipeToWithoutAnimationImpl_;
    CommonFunc showPrevImpl_;
    CommonFunc showNextImpl_;
    CommonFunc finishImpl_;
    CommonFunc finishCallback_;
    CommonFunc tabBarFinishCallback_;
    CommonFunc removeTabBarEventCallback_;
    CommonFunc addTabBarEventCallback_;
    CommonFunc removeSwiperEventCallback_;
    CommonFunc addSwiperEventCallback_;
    TurnPageRateFunc turnPageRateCallback_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SWIPER_SWIPER_CONTROLLER_H
