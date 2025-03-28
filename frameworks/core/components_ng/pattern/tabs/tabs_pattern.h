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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TABS_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TABS_PATTERN_H

#include <optional>

#include "base/geometry/axis.h"
#include "base/memory/referenced.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/swiper/swiper_model.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_layout_algorithm.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"

namespace OHOS::Ace::NG {

class TabsPattern : public Pattern {
    DECLARE_ACE_TYPE(TabsPattern, Pattern);

public:
    using ChangeEvent = std::function<void(int32_t)>;
    using ChangeEventPtr = std::shared_ptr<ChangeEvent>;

    TabsPattern() = default;
    ~TabsPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<TabsLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<TabsLayoutAlgorithm>();
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::SCOPE, true };
    }

    ScopeFocusAlgorithm GetScopeFocusAlgorithm() override;

    void SetOnChangeEvent(std::function<void(const BaseEventInfo*)>&& event);

    ChangeEventPtr GetChangeEvent()
    {
        return onChangeEvent_;
    }

    void SetOnTabBarClickEvent(std::function<void(const BaseEventInfo*)>&& event);

    void SetAnimationStartEvent(AnimationStartEvent&& event);

    void SetAnimationEndEvent(AnimationEndEvent&& event);

    ChangeEventPtr GetTabBarClickEvent()
    {
        return onTabBarClickEvent_;
    }

    void OnModifyDone() override;
	
    std::string ProvideRestoreInfo() override;
    
    void OnRestoreInfo(const std::string& restoreInfo) override;
	
    void SetOnIndexChangeEvent(std::function<void(const BaseEventInfo*)>&& event);

    ChangeEventPtr GetIndexChangeEvent()
    {
        return onIndexChangeEvent_;
    }

    bool GetIsCustomAnimation() const
    {
        return isCustomAnimation_;
    }

    void SetIsCustomAnimation(bool isCustomAnimation)
    {
        isCustomAnimation_ = isCustomAnimation;
    }

    void SetIsDisableSwipe(bool isDisableSwipe)
    {
        isDisableSwipe_ = isDisableSwipe;
    }

    void SetOnContentWillChange(std::function<bool(int32_t, int32_t)>&& callback)
    {
        callback_ = callback;
    }

    std::optional<bool> OnContentWillChange(int32_t preIndex, int32_t index) const
    {
        std::optional<bool> ret;
        if (callback_) {
            ret = callback_(preIndex, index);
        }
        return ret;
    }

    void SetInterceptStatus(bool status)
    {
        interceptStatus_ = status;
    }

    bool GetInterceptStatus() const
    {
        return interceptStatus_;
    }

    void SetAnimateMode(TabAnimateMode mode)
    {
        animateMode_ = mode;
    }

    TabAnimateMode GetAnimateMode() const
    {
        return animateMode_;
    }

    void HandleChildrenUpdated(const RefPtr<FrameNode>& swiperNode, const RefPtr<FrameNode>& tabBarNode);

    void HandleMaskAnimationByCreate(const RefPtr<FrameNode>& tabBarNode, const RefPtr<FrameNode>& swiperNode,
        const RefPtr<TabBarPattern>& tabBarPattern, const RefPtr<TabsLayoutProperty>& tabsLayoutProperty, int index);

private:
    void OnAttachToFrameNode() override;
    void OnAfterModifyDone() override;
    void OnUpdateShowDivider();
    WeakPtr<FocusHub> GetNextFocusNode(FocusStep step, const WeakPtr<FocusHub>& currentFocusNode);
    void BeforeCreateLayoutWrapper() override;
    std::string GetTabBarTextByIndex(int32_t index) const;
    void UpdateSwiperDisableSwipe(bool disableSwipe);
    void SetSwiperPaddingAndBorder();
    void RecordChangeEvent(int32_t index);

    bool isCustomAnimation_ = false;
    bool isDisableSwipe_ = false;
    bool isInit_ = true;

    TabAnimateMode animateMode_ = TabAnimateMode::CONTENT_FIRST;
    ChangeEventPtr onChangeEvent_;
    ChangeEventPtr onTabBarClickEvent_;
    ChangeEventPtr onIndexChangeEvent_;
    AnimationStartEventPtr animationStartEvent_;
    AnimationEndEventPtr animationEndEvent_;
    std::function<bool(int32_t, int32_t)> callback_;
    bool interceptStatus_ = false;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TABS_PATTERN_H
