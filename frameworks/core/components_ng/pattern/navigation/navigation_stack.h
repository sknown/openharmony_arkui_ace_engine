
/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVIGATION_NAVIGATION_STACK_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVIGATION_NAVIGATION_STACK_H

#include <functional>
#include <optional>

#include "base/memory/referenced.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/pattern/navrouter/navdestination_context.h"

namespace OHOS::Ace::NG {
using NavPathList = std::vector<std::pair<std::string, RefPtr<UINode>>>;
class NavDestinationContext;
class RouteInfo : public virtual AceType {
    DECLARE_ACE_TYPE(NG::RouteInfo, AceType)
public:
    RouteInfo() = default;
    virtual ~RouteInfo() = default;

    virtual std::string GetName()
    {
        return "";
    }
};

class NavigationStack : public virtual AceType {
    DECLARE_ACE_TYPE(NG::NavigationStack, AceType)
public:
    NavigationStack() = default;
    ~NavigationStack() override = default;

    virtual void UpdateStackInfo(const RefPtr<NavigationStack>& newStack) {}

    NavPathList& GetAllNavDestinationNodes()
    {
        return navPathList_;
    }

    virtual void SetOnStateChangedCallback(std::function<void()> callback) {}

    void SetNavPathList(const NavPathList& navPathList)
    {
        preNavPathList_ = navPathList;
        navPathList_ = navPathList;
    }

    bool Empty() const
    {
        return navPathList_.empty();
    }

    int32_t Size() const
    {
        return static_cast<int32_t>(navPathList_.size());
    }

    int32_t PreSize() const
    {
        return static_cast<int32_t>(preNavPathList_.size());
    }

    std::optional<std::pair<std::string, RefPtr<UINode>>> GetTopNavPath() const
    {
        if (navPathList_.empty()) {
            return std::nullopt;
        }
        return navPathList_.back();
    }

    std::optional<std::pair<std::string, RefPtr<UINode>>> GetPreTopNavPath() const
    {
        if (preNavPathList_.empty()) {
            return std::nullopt;
        }
        return preNavPathList_.back();
    }

    void RemoveStack()
    {
        navPathList_.clear();
    }

    NavPathList GetAllCacheNodes();
    void AddCacheNode(const std::string& name, const RefPtr<UINode>& uiNode);
    RefPtr<UINode> GetFromCacheNode(NavPathList& cacheNodes, const std::string& name);
    RefPtr<UINode> GetFromCacheNode(const std::string& name);
    std::optional<std::pair<std::string, RefPtr<UINode>>> GetFromCacheNode(int32_t handle);
    void RemoveCacheNode(
        NavPathList& cacheNodes, const std::string& name, const RefPtr<UINode>& navDestinationNode);
    void RemoveCacheNode(int32_t handle);
    void ReOrderCache(const std::string& name, const RefPtr<UINode>& navDestinationNode);

    void Remove();
    void Remove(const std::string& name);
    void Remove(const std::string& name, const RefPtr<UINode>& navDestinationNode);
    int32_t RemoveInNavPathList(const std::string& name, const RefPtr<UINode>& navDestinationNode);
    int32_t RemoveInPreNavPathList(const std::string& name, const RefPtr<UINode>& navDestinationNode);
    void RemoveAll();
    void Add(const std::string& name, const RefPtr<UINode>& navDestinationNode,
        const RefPtr<RouteInfo>& routeInfo = nullptr);
    void Add(const std::string& name, const RefPtr<UINode>& navDestinationNode, NavRouteMode mode,
        const RefPtr<RouteInfo>& routeInfo = nullptr);
    RefPtr<UINode> Get();
    bool Get(const std::string& name, RefPtr<UINode>& navDestinationNode, int32_t& index);
    bool GetFromPreBackup(const std::string& name, RefPtr<UINode>& navDestinationNode, int32_t& index);
    RefPtr<UINode> Get(int32_t index);
    RefPtr<UINode> GetPre(const std::string& name, const RefPtr<UINode>& navDestinationNode);
    virtual bool IsEmpty();
    virtual std::vector<std::string> GetAllPathName();
    virtual std::vector<int32_t> GetAllPathIndex();
    virtual void InitNavPathIndex(const std::vector<std::string>& pathNames) {}
    virtual void Pop();
    virtual void Push(const std::string& name, const RefPtr<RouteInfo>& routeInfo = nullptr);
    virtual void Push(const std::string& name, int32_t index);
    virtual void RemoveName(const std::string& name);
    virtual void RemoveIndex(int32_t index);
    virtual void Clear();
    virtual void UpdateReplaceValue(int32_t replaceValue) const;
    virtual int32_t GetReplaceValue() const;
    virtual RefPtr<UINode> CreateNodeByIndex(int32_t index, const WeakPtr<UINode>& customNode);
    virtual RefPtr<UINode> CreateNodeByRouteInfo(const RefPtr<RouteInfo>& routeInfo, const WeakPtr<UINode>& node);
    virtual bool GetDisableAnimation() const
    {
        return false;
    }
    virtual bool GetAnimatedValue() const
    {
        return animated_;
    }
    virtual void UpdateAnimatedValue(bool animated)
    {
        animated_ = animated;
    }
    int32_t FindIndex(const std::string& name, const RefPtr<UINode>& navDestinationNode, bool isNavPathList);
    virtual std::string GetRouteParam() const
    {
        return "";
    }

    virtual void FireNavigationInterception(bool isBefore, const RefPtr<NG::NavDestinationContext>& from,
        const RefPtr<NG::NavDestinationContext>& to, NavigationOperation operation, bool isAnimated) {}

    virtual void FireNavigationModeChange(NavigationMode mode) {}

    virtual void OnAttachToParent(RefPtr<NavigationStack> parent) {}
    virtual void OnDetachFromParent() {}
    virtual void ClearPreBuildNodeList() {}

    virtual std::vector<std::string> DumpStackInfo() const;

    virtual int32_t GetJsIndexFromNativeIndex(int32_t index) { return -1; }
    virtual void MoveIndexToTop(int32_t index) {}

    const WeakPtr<UINode>& GetNavigationNode()
    {
        return navigationNode_;
    }

    void SetNavigationNode(const WeakPtr<UINode>& navigationNode)
    {
        navigationNode_ = navigationNode;
    }

    virtual void UpdatePathInfoIfNeeded(RefPtr<UINode>& uiNode, int32_t index) {}
    virtual void RecoveryNavigationStack() {}

    void UpdateRecoveryList()
    {
        recoveryList_ = navPathList_;
    }

    void ClearRecoveryList()
    {
        recoveryList_.clear();
    }

    NavPathList GetRecoveryList()
    {
        return recoveryList_;
    }

protected:
    void MoveToTop(const std::string& name, const RefPtr<UINode>& navDestinationNode);
    void AddForDefault(const std::string& name, const RefPtr<UINode>& navDestinationNode,
        const RefPtr<RouteInfo>& routeInfo = nullptr);
    void AddForReplace(const std::string& name, const RefPtr<UINode>& navDestinationNode,
        const RefPtr<RouteInfo>& routeInfo = nullptr);

    NavPathList navPathList_;
    // prev backup NavPathList
    NavPathList preNavPathList_;
    // recovery NavPathList
    NavPathList recoveryList_;
    NavPathList cacheNodes_;
    bool animated_ = true;
    WeakPtr<UINode> navigationNode_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVIGATION_NAVIGATION_STACK_H
