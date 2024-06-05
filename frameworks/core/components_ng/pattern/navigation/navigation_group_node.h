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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVIGATION_GROUP_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVIGATION_GROUP_NODE_H

#include <cstdint>
#include <list>

#include "base/memory/referenced.h"
#include "core/animation/page_transition_common.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/group_node.h"
#include "core/components_ng/pattern/navigation/bar_item_node.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/pattern/navigation/navigation_stack.h"
#include "core/components_ng/pattern/navigation/title_bar_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/navrouter/navrouter_pattern.h"
#include "core/components_ng/property/property.h"

namespace OHOS::Ace::NG {
class InspectorFilter;

class ACE_EXPORT NavigationGroupNode : public GroupNode {
    DECLARE_ACE_TYPE(NavigationGroupNode, GroupNode)
public:
    NavigationGroupNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
        : GroupNode(tag, nodeId, pattern)
    {}

    ~NavigationGroupNode() override;

    using AnimationFinishCallback = std::function<void()>;

    void AddChildToGroup(const RefPtr<UINode>& child, int32_t slot = DEFAULT_NODE_SLOT) override;

    // remain child needs to keep to use pop animation
    void UpdateNavDestinationNodeWithoutMarkDirty(const RefPtr<UINode>& remainChild, bool modeChange = false);
    static RefPtr<NavigationGroupNode> GetOrCreateGroupNode(
        const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator);

    bool IsAtomicNode() const override
    {
        return false;
    }

    void SetNavBarNode(const RefPtr<UINode>& navBarNode)
    {
        navBarNode_ = navBarNode;
    }

    const RefPtr<UINode>& GetNavBarNode() const
    {
        return navBarNode_;
    }

    void SetContentNode(const RefPtr<UINode>& contentNode)
    {
        contentNode_ = contentNode;
    }

    const RefPtr<UINode>& GetContentNode() const
    {
        return contentNode_;
    }

    void SetDividerNode(const RefPtr<UINode>& dividerNode)
    {
        dividerNode_ = dividerNode;
    }

    const RefPtr<UINode>& GetDividerNode() const
    {
        return dividerNode_;
    }

    const std::string& GetCurId() const
    {
        return curId_;
    }

    bool GetIsModeChange() const
    {
        return isModeChange_;
    }

    void SetIsModeChange(bool isModeChange)
    {
        isModeChange_ = isModeChange;
    }

    bool GetNeedSetInvisible() const
    {
        return needSetInvisible_;
    }

    void SetNeedSetInvisible(bool needSetInvisible)
    {
        needSetInvisible_ = needSetInvisible;
    }

    bool IsOnModeSwitchAnimation()
    {
        return isOnModeSwitchAnimation_;
    }

    void SetDoingModeSwitchAnimationFlag(bool isOnAnimation)
    {
        isOnModeSwitchAnimation_ = isOnAnimation;
    }

    std::list<std::shared_ptr<AnimationUtils::Animation>>& GetPushAnimations()
    {
        return pushAnimations_;
    }

    std::list<std::shared_ptr<AnimationUtils::Animation>>& GetPopAnimations()
    {
        return popAnimations_;
    }

    void CleanPushAnimations()
    {
        pushAnimations_.clear();
    }

    void CleanPopAnimations()
    {
        popAnimations_.clear();
    }

    bool CheckCanHandleBack();

    void OnInspectorIdUpdate(const std::string& id) override;

    bool HandleBack(const RefPtr<FrameNode>& node, bool isLastChild, bool isOverride);

    void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const override;
    static RefPtr<UINode> GetNavDestinationNode(RefPtr<UINode> uiNode);
    void SetBackButtonEvent(const RefPtr<NavDestinationGroupNode>& navDestination);

    void TransitionWithPop(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode, bool isNavBar = false);
    void TransitionWithPush(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode, bool isNavBar = false);

    std::shared_ptr<AnimationUtils::Animation> BackButtonAnimation(
        const RefPtr<FrameNode>& backButtonNode, bool isTransitionIn);
    std::shared_ptr<AnimationUtils::Animation> MaskAnimation(const RefPtr<RenderContext>& transitionOutNodeContext);
    std::shared_ptr<AnimationUtils::Animation> TitleOpacityAnimation(
        const RefPtr<FrameNode>& node, bool isTransitionOut);
    void TransitionWithReplace(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode, bool isNavBar);
    void DealNavigationExit(const RefPtr<FrameNode>& preNode, bool isNavBar, bool isAnimated = true);
    void NotifyPageHide();
    void UpdateLastStandardIndex();
    int32_t GetLastStandardIndex() const
    {
        return lastStandardIndex_;
    }
    AnimationOption CreateAnimationOption(const RefPtr<Curve>& curve, FillMode mode,
        int32_t duration, const AnimationFinishCallback& callback);
    NavigationMode GetNavigationMode();

    void SetIsOnAnimation(bool isOnAnimation)
    {
        isOnAnimation_ = isOnAnimation;
    }

    void OnDetachFromMainTree(bool recursive) override;
    void OnAttachToMainTree(bool recursive) override;

    void FireHideNodeChange(NavDestinationLifecycle lifecycle);

    float CheckLanguageDirection();

    void RemoveDialogDestination();

    void SetNavigationPathInfo(const std::string& moduleName, const std::string& pagePath)
    {
        navigationPathInfo_ = pagePath;
        navigationModuleName_ = moduleName;
    }

    const std::string& GetNavigationPathInfo() const
    {
        return navigationPathInfo_;
    }

    void CleanHideNodes()
    {
        hideNodes_.clear();
    }

private:
    bool UpdateNavDestinationVisibility(const RefPtr<NavDestinationGroupNode>& navDestination,
        const RefPtr<UINode>& remainChild, int32_t index, size_t destinationSize);
    bool ReorderNavDestination(
        const std::vector<std::pair<std::string, RefPtr<UINode>>>& navDestinationNodes,
        RefPtr<FrameNode>& navigationContentNode, int32_t& slot, bool& hasChanged);
    void RemoveRedundantNavDestination(RefPtr<FrameNode>& navigationContentNode,
        const RefPtr<UINode>& remainChild, size_t slot, bool& hasChanged);
    bool FindNavigationParent(const std::string& parentName);
    bool GetCurTitleBarNode(RefPtr<TitleBarNode>& curTitleBarNode, const RefPtr<FrameNode>& curNode,
        bool isNavBar);

    void DealRemoveDestination(const RefPtr<NavDestinationGroupNode>& destination);

    RefPtr<UINode> navBarNode_;
    RefPtr<UINode> contentNode_;
    RefPtr<UINode> dividerNode_;
    // dialog hideNodes, if is true, nodes need remove
    std::vector<std::pair<RefPtr<NavDestinationGroupNode>, bool>> hideNodes_;
    std::vector<RefPtr<NavDestinationGroupNode>> showNodes_;
    int32_t lastStandardIndex_ = -1;
    bool isOnAnimation_ { false };
    bool isModeChange_ { false };
    bool needSetInvisible_ { false };
    bool isOnModeSwitchAnimation_ { false };
    std::string curId_;
    std::list<std::shared_ptr<AnimationUtils::Animation>> pushAnimations_;
    std::list<std::shared_ptr<AnimationUtils::Animation>> popAnimations_;
    std::string navigationPathInfo_;
    std::string navigationModuleName_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVIGATION_GROUP_NODE_H
