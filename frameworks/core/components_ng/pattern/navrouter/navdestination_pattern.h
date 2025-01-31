/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVROUTER_NAVDESTINATION_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVROUTER_NAVDESTINATION_PATTERN_H

#include "base/memory/referenced.h"
#include "base/system_bar/system_bar_style.h"
#include "base/utils/utils.h"
#include "core/common/autofill/auto_fill_trigger_state_holder.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/navigation/navigation_event_hub.h"
#include "core/components_ng/pattern/navigation/navigation_stack.h"
#include "core/components_ng/pattern/navrouter/navdestination_context.h"
#include "core/components_ng/pattern/navrouter/navdestination_event_hub.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_algorithm.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_property.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/syntax/shallow_builder.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

class NavDestinationPattern : public Pattern, public FocusView, public AutoFillTriggerStateHolder {
    DECLARE_ACE_TYPE(NavDestinationPattern, Pattern, FocusView, AutoFillTriggerStateHolder);

public:
    explicit NavDestinationPattern(const RefPtr<ShallowBuilder>& shallowBuilder);
    NavDestinationPattern();
    ~NavDestinationPattern() override;

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<NavDestinationLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        auto layout = MakeRefPtr<NavDestinationLayoutAlgorithm>();
        layout->SetIsShown(isOnShow_);
        return layout;
    }
    
    bool CheckCustomAvoidKeyboard() const override
    {
        return !NearZero(avoidKeyboardOffset_);
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<NavDestinationEventHub>();
    }

    void OnActive() override;

    void OnModifyDone() override;

    const RefPtr<ShallowBuilder>& GetShallowBuilder() const
    {
        return shallowBuilder_;
    }

    void SetName(const std::string& name)
    {
        name_ = name;
        auto eventHub = GetEventHub<NavDestinationEventHub>();
        CHECK_NULL_VOID(eventHub);
        eventHub->SetName(name);
    }

    const std::string& GetName()
    {
        return name_;
    }

    void SetNavPathInfo(const RefPtr<NavPathInfo>& pathInfo)
    {
        if (navDestinationContext_) {
            navDestinationContext_->SetNavPathInfo(pathInfo);
        }
    }

    RefPtr<NavPathInfo> GetNavPathInfo() const
    {
        return navDestinationContext_ ? navDestinationContext_->GetNavPathInfo() : nullptr;
    }

    void SetNavigationStack(const WeakPtr<NavigationStack>& stack)
    {
        if (navDestinationContext_) {
            navDestinationContext_->SetNavigationStack(stack);
        }
    }

    WeakPtr<NavigationStack> GetNavigationStack() const
    {
        return navDestinationContext_ ? navDestinationContext_->GetNavigationStack() : nullptr;
    }

    void SetIndex(int32_t index)
    {
        if (navDestinationContext_) {
            navDestinationContext_->SetIndex(index);
        }
    }

    void SetNavDestinationContext(const RefPtr<NavDestinationContext>& context)
    {
        navDestinationContext_ = context;
        if (navDestinationContext_) {
            navDestinationContext_->SetNavDestinationId(navDestinationId_);
        }
    }

    RefPtr<NavDestinationContext> GetNavDestinationContext() const
    {
        return navDestinationContext_;
    }

    void SetCustomNode(const RefPtr<UINode>& customNode)
    {
        customNode_ = customNode;
    }

    RefPtr<UINode> GetCustomNode()
    {
        return customNode_;
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::SCOPE, true };
    }

    std::list<int32_t> GetRouteOfFirstScope() override
    {
        return {};
    }

    bool IsEntryFocusView() override
    {
        return false;
    }

    void SetIsOnShow(bool isOnShow)
    {
        isOnShow_ = isOnShow;
    }

    bool GetIsOnShow()
    {
        return isOnShow_;
    }

    bool GetBackButtonState();

    RefPtr<UINode> GetNavigationNode()
    {
        return navigationNode_.Upgrade();
    }

    NavDestinationState GetNavDestinationState() const
    {
        auto eventHub = GetEventHub<NavDestinationEventHub>();
        CHECK_NULL_RETURN(eventHub, NavDestinationState::NONE);
        auto state = eventHub->GetState();
        return state;
    }

    void DumpInfo() override;

    uint64_t GetNavDestinationId() const
    {
        return navDestinationId_;
    }

    void SetNavigationNode(const RefPtr<UINode>& navigationNode)
    {
        navigationNode_ = AceType::WeakClaim(RawPtr(navigationNode));
    }

    void OnDetachFromMainTree() override
    {
        backupStyle_.reset();
        currStyle_.reset();
    }

    bool OverlayOnBackPressed();

    void CreateOverlayManager(bool isShow)
    {
        if (!overlayManager_ && isShow) {
            overlayManager_ = MakeRefPtr<OverlayManager>(GetHost());
        }
    }

    const RefPtr<OverlayManager>& GetOverlayManager()
    {
        return overlayManager_;
    }

    void DeleteOverlayManager()
    {
        overlayManager_.Reset();
    }

    void SetNavigationId(const std::string& id)
    {
        inspectorId_ = id;
    }

    std::string GetNavigationId() const
    {
        return inspectorId_;
    }

    void SetIsUserDefinedBgColor(bool isUserDefinedBgColor)
    {
        isUserDefinedBgColor_ = isUserDefinedBgColor;
    }

    bool IsUserDefinedBgColor() const
    {
        return isUserDefinedBgColor_;
    }

    void OnLanguageConfigurationUpdate() override;
    void SetAvoidKeyboardOffset(float avoidKeyboardOffset)
    {
        avoidKeyboardOffset_ = avoidKeyboardOffset;
    }

    float GetAvoidKeyboardOffset()
    {
        return avoidKeyboardOffset_;
    }

    bool NeedIgnoreKeyboard();

    void SetSystemBarStyle(const RefPtr<SystemBarStyle>& style);
    const std::optional<RefPtr<SystemBarStyle>>& GetBackupStyle() const
    {
        return backupStyle_;
    }
    const std::optional<RefPtr<SystemBarStyle>>& GetCurrentStyle() const
    {
        return currStyle_;
    }

private:
    void UpdateNameIfNeeded(RefPtr<NavDestinationGroupNode>& hostNode);
    void UpdateBackgroundColorIfNeeded(RefPtr<NavDestinationGroupNode>& hostNode);
    void UpdateTitlebarVisibility(RefPtr<NavDestinationGroupNode>& hostNode);
    void InitBackButtonLongPressEvent(RefPtr<NavDestinationGroupNode>& hostNode);
    void HandleLongPress();
    void HandleLongPressActionEnd();

    RefPtr<ShallowBuilder> shallowBuilder_;
    std::string name_;
    std::string inspectorId_;
    RefPtr<NavDestinationContext> navDestinationContext_;
    RefPtr<UINode> customNode_;
    WeakPtr<UINode> navigationNode_;
    RefPtr<OverlayManager> overlayManager_;
    bool isOnShow_ = false;
    bool isUserDefinedBgColor_ = false;
    bool isRightToLeft_ = false;
    uint64_t navDestinationId_ = 0;
    void OnAttachToFrameNode() override;
    float avoidKeyboardOffset_ = 0.0f;

    RefPtr<LongPressEvent> longPressEvent_;
    RefPtr<FrameNode> dialogNode_;

    std::optional<RefPtr<SystemBarStyle>> backupStyle_;
    std::optional<RefPtr<SystemBarStyle>> currStyle_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVROUTER_NAVDESTINATION_PATTERN_H
