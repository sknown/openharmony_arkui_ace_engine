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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_ACCESSIBILITY_ACCESSIBILITY_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_ACCESSIBILITY_ACCESSIBILITY_MANAGER_H

#include "base/memory/ace_type.h"
#include "core/accessibility/accessibility_node.h"
#include "core/accessibility/accessibility_utils.h"
#include "core/pipeline/base/base_composed_component.h"

namespace OHOS::Accessibility {
class AccessibilityElementInfo;
class AccessibilityEventInfo;
} // namespace OHOS::Accessibility

namespace OHOS::Ace {

struct AccessibilityEvent {
    int64_t nodeId = 0;
    uint32_t windowId = 0;
    WindowsContentChangeTypes windowContentChangeTypes = CONTENT_CHANGE_TYPE_INVALID;
    WindowUpdateType windowChangeTypes = WINDOW_UPDATE_INVALID;
    std::string eventType;
    std::string componentType;
    std::string beforeText;
    std::string latestContent;
    double currentItemIndex = 0.0;
    double itemCount = 0.0;
    AccessibilityEventType type = AccessibilityEventType::UNKNOWN;
};

enum class AccessibilityVersion {
    JS_VERSION = 1,
    JS_DECLARATIVE_VERSION,
};

class AccessibilityChildTreeCallback {
public:
    AccessibilityChildTreeCallback() = default;
    virtual ~AccessibilityChildTreeCallback() = default;
    virtual bool OnRegister(uint32_t windowId, int32_t treeId) = 0;
    virtual bool OnDeregister() = 0;
    virtual bool OnSetChildTree(int32_t childWindowId, int32_t childTreeId) = 0;
    virtual bool OnDumpChildInfo(const std::vector<std::string>& params, std::vector<std::string>& info) = 0;
    int32_t GetChildTreeId() const
    {
        return childTreeId_;
    }
    void SetChildTreeId(int32_t childTreeId)
    {
        childTreeId_ = childTreeId;
    }

private:
    int32_t childTreeId_ = 0;
};

using VisibleRatioCallback = std::function<void(bool, double)>;
class AccessibilityManager : public AceType {
    DECLARE_ACE_TYPE(AccessibilityManager, AceType);

public:
    AccessibilityManager() = default;
    ~AccessibilityManager() override = default;

    virtual void SendAccessibilityAsyncEvent(const AccessibilityEvent& accessibilityEvent) = 0;
    virtual int64_t GenerateNextAccessibilityId() = 0;
    virtual RefPtr<AccessibilityNode> CreateSpecializedNode(
        const std::string& tag, int32_t nodeId, int32_t parentNodeId) = 0;
    virtual RefPtr<AccessibilityNode> CreateAccessibilityNode(
        const std::string& tag, int32_t nodeId, int32_t parentNodeId, int32_t itemIndex) = 0;
    virtual RefPtr<AccessibilityNode> GetAccessibilityNodeById(NodeId nodeId) const = 0;
    virtual std::string GetInspectorNodeById(NodeId nodeId) const = 0;
    virtual void RemoveAccessibilityNodes(RefPtr<AccessibilityNode>& node) = 0;
    virtual void RemoveAccessibilityNodeById(NodeId nodeId) = 0;
    virtual void ClearPageAccessibilityNodes(int32_t pageId) = 0;
    virtual void SetRootNodeId(int32_t nodeId) = 0;
    virtual void TrySaveTargetAndIdNode(
        const std::string& id, const std::string& target, const RefPtr<AccessibilityNode>& node) = 0;
    virtual void HandleComponentPostBinding() = 0;
    virtual void OnDumpInfo(const std::vector<std::string>& params) = 0;
    virtual void OnDumpInfoNG(const std::vector<std::string>& params, uint32_t windowId) = 0;
    virtual void SetCardViewPosition(int id, float offsetX, float offsetY) = 0;
    virtual void SetCardViewParams(const std::string& key, bool focus) = 0;
    virtual void SetSupportAction(uint32_t action, bool isEnable) = 0;
    virtual void ClearNodeRectInfo(RefPtr<AccessibilityNode>& node, bool isPopDialog) = 0;
    virtual void AddComposedElement(const std::string& key, const RefPtr<ComposedElement>& node) = 0;
    virtual void RemoveComposedElementById(const std::string& key) = 0;
    virtual WeakPtr<ComposedElement> GetComposedElementFromPage(NodeId nodeId) = 0;
    virtual void TriggerVisibleChangeEvent() = 0;
    virtual void AddVisibleChangeNode(NodeId nodeId, double ratio, VisibleRatioCallback callback) = 0;
    virtual void RemoveVisibleChangeNode(NodeId nodeId) = 0;
    virtual bool IsVisibleChangeNodeExists(NodeId nodeId) = 0;
    virtual void UpdateEventTarget(NodeId id, BaseEventInfo& info) = 0;
    virtual void SetWindowPos(int32_t left, int32_t top, int32_t windowId) = 0;
#ifdef WINDOW_SCENE_SUPPORTED
    virtual void SearchElementInfoByAccessibilityIdNG(int64_t elementId, int32_t mode,
        std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
        int64_t uiExtensionOffset) = 0;
    virtual void SearchElementInfosByTextNG(int64_t elementId, const std::string& text,
        std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
        const int64_t uiExtensionOffset = 0) = 0;
    virtual void FindFocusedElementInfoNG(int64_t elementId, int32_t focusType,
        Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
        const int64_t uiExtensionOffset = 0) = 0;
    virtual void FocusMoveSearchNG(int64_t elementId, int32_t direction, Accessibility::AccessibilityElementInfo& info,
        const RefPtr<PipelineBase>& context, const int64_t uiExtensionOffset = 0) = 0;
    virtual bool ExecuteExtensionActionNG(int64_t elementId, const std::map<std::string, std::string>& actionArguments,
        int32_t action, const RefPtr<PipelineBase>& context, int64_t uiExtensionOffset) = 0;
    virtual bool TransferAccessibilityAsyncEvent(
        const Accessibility::AccessibilityEventInfo& eventInfo, int64_t uiExtensionOffset)
    {
        return false;
    }
    virtual void SendExtensionAccessibilityEvent(
        const Accessibility::AccessibilityEventInfo& eventInfo, int64_t uiExtensionOffset) {}
#endif
#ifdef WEB_SUPPORTED
    virtual void UpdateAccessibilityFocusId(const RefPtr<PipelineBase>& context, int64_t accessibilityId,
        bool isFocus) {}
    virtual int64_t GetAccessibilityFocusId() const
    {
        return -1;
    }
#endif
    void SetVersion(AccessibilityVersion version)
    {
        version_ = version;
    }
    AccessibilityVersion GetVersion()
    {
        return version_;
    }

    virtual void RegisterAccessibilityChildTreeCallback(
        int64_t elementId, const std::shared_ptr<AccessibilityChildTreeCallback> &callback) {};

    virtual void DeregisterAccessibilityChildTreeCallback(int64_t elementId) {};

    virtual void RegisterInteractionOperationAsChildTree(
        uint32_t parentWindowId, int32_t parentTreeId, int64_t parentElementId) {};
    virtual void SetAccessibilityGetParentRectHandler(std::function<void(int32_t &, int32_t &)> &&callback) {};
    virtual void DeregisterInteractionOperationAsChildTree() {};
    virtual void SendEventToAccessibilityWithNode(const AccessibilityEvent& accessibilityEvent,
        const RefPtr<AceType>& node, const RefPtr<PipelineBase>& context) {};
    bool IsRegister()
    {
        return isReg_;
    }

    void Register(bool state)
    {
        isReg_ = state;
    }

private:
    AccessibilityVersion version_ = AccessibilityVersion::JS_VERSION;
    bool isReg_ = false;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_ACCESSIBILITY_ACCESSIBILITY_MANAGER_H
