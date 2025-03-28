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

#include "js_accessibility_manager.h"

#include <algorithm>
#include <variant>

#include "accessibility_constants.h"
#include "accessibility_event_info.h"
#include "accessibility_system_ability_client.h"

#include "adapter/ohos/entrance/ace_application_info.h"
#include "adapter/ohos/entrance/ace_container.h"
#include "base/log/ace_trace.h"
#include "base/log/dump_log.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/utils/linear_map.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/inspector.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/pipeline_context.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/bridge/common/dom/dom_type.h"
#include "frameworks/core/components_ng/pattern/web/web_pattern.h"
#include "nlohmann/json.hpp"

using namespace OHOS::Accessibility;
using namespace OHOS::AccessibilityConfig;
using namespace std;

namespace OHOS::Ace::Framework {
namespace {
const char DUMP_ORDER[] = "-accessibility";
const char DUMP_INSPECTOR[] = "-inspector";
const char ACCESSIBILITY_FOCUSED_EVENT[] = "accessibilityfocus";
const char ACCESSIBILITY_CLEAR_FOCUS_EVENT[] = "accessibilityclearfocus";
const char TEXT_CHANGE_EVENT[] = "textchange";
const char PAGE_CHANGE_EVENT[] = "pagechange";
const char SCROLL_END_EVENT[] = "scrollend";
const char SCROLL_START_EVENT[] = "scrollstart";
const char MOUSE_HOVER_ENTER[] = "mousehoverenter";
const char MOUSE_HOVER_EXIT[] = "mousehoverexit";
const char LIST_TAG[] = "List";
const char SIDEBARCONTAINER_TAG[] = "SideBarContainer";
const char STRING_DIR_FORWARD[] = "forward";
const char STRING_DIR_BACKWARD[] = "backward";
constexpr int32_t INVALID_PARENT_ID = -2100000;
constexpr int32_t DEFAULT_PARENT_ID = 2100000;
constexpr int32_t ROOT_STACK_BASE = 1100000;
constexpr int32_t ROOT_DECOR_BASE = 3100000;
constexpr int32_t CARD_NODE_ID_RATION = 10000;
constexpr int32_t CARD_ROOT_NODE_ID_RATION = 1000;
constexpr int32_t CARD_BASE = 100000;

struct ActionTable {
    AceAction aceAction;
    ActionType action;
};

struct AccessibilityActionParam {
    RefPtr<NG::AccessibilityProperty> accessibilityProperty;
    std::string setTextArgument = "";
    int32_t setSelectionStart = -1;
    int32_t setSelectionEnd = -1;
    bool setSelectionDir = false;
    int32_t setCursorIndex = -1;
    TextMoveUnit moveUnit = TextMoveUnit::STEP_CHARACTER;
};

const std::map<Accessibility::ActionType, std::function<bool(const AccessibilityActionParam& param)>> ACTIONS = {
    { ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionScrollForward(); } },
    { ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionScrollBackward(); } },
    { ActionType::ACCESSIBILITY_ACTION_SET_TEXT,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionSetText(param.setTextArgument);
        } },
    { ActionType::ACCESSIBILITY_ACTION_SET_SELECTION,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionSetSelection(param.setSelectionStart,
                                                                      param.setSelectionEnd, param.setSelectionDir);
        } },
    { ActionType::ACCESSIBILITY_ACTION_COPY,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionCopy(); } },
    { ActionType::ACCESSIBILITY_ACTION_CUT,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionCut(); } },
    { ActionType::ACCESSIBILITY_ACTION_PASTE,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionPaste(); } },
    { ActionType::ACCESSIBILITY_ACTION_CLICK,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionClick(); } },
    { ActionType::ACCESSIBILITY_ACTION_LONG_CLICK,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionLongClick(); } },
    { ActionType::ACCESSIBILITY_ACTION_SELECT,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionSelect(); } },
    { ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionClearSelection(); } },
    { ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionMoveText(static_cast<int32_t>(param.moveUnit), true);
        } },
    { ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionMoveText(static_cast<int32_t>(param.moveUnit), false);
        } },
    { ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionSetIndex(static_cast<int32_t>(param.setCursorIndex));
        } },
};

bool IsExtensionComponent(const RefPtr<NG::UINode>& node)
{
    return node && (node->GetTag() == V2::UI_EXTENSION_COMPONENT_ETS_TAG
        || node->GetTag() == V2::EMBEDDED_COMPONENT_ETS_TAG
        || node->GetTag() == V2::ISOLATED_COMPONENT_ETS_TAG);
}

bool IsUIExtensionShowPlaceholder(const RefPtr<NG::UINode>& node)
{
    CHECK_NULL_RETURN(node, true);
    if (node->GetTag() == V2::ISOLATED_COMPONENT_ETS_TAG) {
        return false;
    }
#ifdef WINDOW_SCENE_SUPPORTED
    auto pipeline = node->GetContextRefPtr();
    CHECK_NULL_RETURN(pipeline, true);
    auto manager = pipeline->GetUIExtensionManager();
    CHECK_NULL_RETURN(manager, true);
    return manager->IsShowPlaceholder(node->GetId());
#endif
    return true;
}

Accessibility::EventType ConvertStrToEventType(const std::string& type)
{
    // static linear map must be sorted by key.
    static const LinearMapNode<Accessibility::EventType> eventTypeMap[] = {
        { ACCESSIBILITY_CLEAR_FOCUS_EVENT, Accessibility::EventType::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED_EVENT },
        { ACCESSIBILITY_FOCUSED_EVENT, Accessibility::EventType::TYPE_VIEW_ACCESSIBILITY_FOCUSED_EVENT },
        { DOM_CLICK, Accessibility::EventType::TYPE_VIEW_CLICKED_EVENT },
        { DOM_FOCUS, Accessibility::EventType::TYPE_VIEW_FOCUSED_EVENT },
        { DOM_LONG_PRESS, Accessibility::EventType::TYPE_VIEW_LONG_CLICKED_EVENT },
        { MOUSE_HOVER_ENTER, Accessibility::EventType::TYPE_VIEW_HOVER_ENTER_EVENT },
        { MOUSE_HOVER_EXIT, Accessibility::EventType::TYPE_VIEW_HOVER_EXIT_EVENT },
        { PAGE_CHANGE_EVENT, Accessibility::EventType::TYPE_PAGE_STATE_UPDATE },
        { SCROLL_END_EVENT, Accessibility::EventType::TYPE_VIEW_SCROLLED_EVENT },
        { SCROLL_START_EVENT, Accessibility::EventType::TYPE_VIEW_SCROLLED_START },
        { DOM_SELECTED, Accessibility::EventType::TYPE_VIEW_SELECTED_EVENT },
        { TEXT_CHANGE_EVENT, Accessibility::EventType::TYPE_VIEW_TEXT_UPDATE_EVENT },
        { DOM_TOUCH_END, Accessibility::EventType::TYPE_TOUCH_END },
        { DOM_TOUCH_START, Accessibility::EventType::TYPE_TOUCH_BEGIN },
    };
    Accessibility::EventType eventType = Accessibility::EventType::TYPE_VIEW_INVALID;
    int64_t idx = BinarySearchFindIndex(eventTypeMap, ArraySize(eventTypeMap), type.c_str());
    if (idx >= 0) {
        eventType = eventTypeMap[idx].value;
    }
    return eventType;
}

Accessibility::EventType ConvertAceEventType(AccessibilityEventType type)
{
    static const LinearEnumMapNode<AccessibilityEventType, Accessibility::EventType> eventTypeMap[] = {
        { AccessibilityEventType::CLICK, Accessibility::EventType::TYPE_VIEW_CLICKED_EVENT },
        { AccessibilityEventType::LONG_PRESS, Accessibility::EventType::TYPE_VIEW_LONG_CLICKED_EVENT },
        { AccessibilityEventType::SELECTED, Accessibility::EventType::TYPE_VIEW_SELECTED_EVENT },
        { AccessibilityEventType::FOCUS, Accessibility::EventType::TYPE_VIEW_FOCUSED_EVENT },
        { AccessibilityEventType::TEXT_CHANGE, Accessibility::EventType::TYPE_VIEW_TEXT_UPDATE_EVENT },
        { AccessibilityEventType::HOVER_ENTER_EVENT, Accessibility::EventType::TYPE_VIEW_HOVER_ENTER_EVENT },
        { AccessibilityEventType::PAGE_CHANGE, Accessibility::EventType::TYPE_PAGE_STATE_UPDATE },
        { AccessibilityEventType::HOVER_EXIT_EVENT, Accessibility::EventType::TYPE_VIEW_HOVER_EXIT_EVENT },
        { AccessibilityEventType::CHANGE, Accessibility::EventType::TYPE_PAGE_CONTENT_UPDATE },
        { AccessibilityEventType::COMPONENT_CHANGE, Accessibility::EventType::TYPE_VIEW_TEXT_UPDATE_EVENT },
        { AccessibilityEventType::SCROLL_END, Accessibility::EventType::TYPE_VIEW_SCROLLED_EVENT },
        { AccessibilityEventType::TEXT_SELECTION_UPDATE,
            Accessibility::EventType::TYPE_VIEW_TEXT_SELECTION_UPDATE_EVENT },
        { AccessibilityEventType::ACCESSIBILITY_FOCUSED,
            Accessibility::EventType::TYPE_VIEW_ACCESSIBILITY_FOCUSED_EVENT },
        { AccessibilityEventType::ACCESSIBILITY_FOCUS_CLEARED,
            Accessibility::EventType::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED_EVENT },
        { AccessibilityEventType::TEXT_MOVE_UNIT, Accessibility::EventType::TYPE_VIEW_TEXT_MOVE_UNIT_EVENT },
        { AccessibilityEventType::SCROLL_START, Accessibility::EventType::TYPE_VIEW_SCROLLED_START },
    };
    Accessibility::EventType eventType = Accessibility::EventType::TYPE_VIEW_INVALID;
    int64_t idx = BinarySearchFindIndex(eventTypeMap, ArraySize(eventTypeMap), type);
    if (idx >= 0) {
        eventType = eventTypeMap[idx].value;
    }
    return eventType;
}

ActionType ConvertAceAction(AceAction aceAction)
{
    static const ActionTable actionTable[] = {
        { AceAction::ACTION_CLICK, ActionType::ACCESSIBILITY_ACTION_CLICK },
        { AceAction::ACTION_LONG_CLICK, ActionType::ACCESSIBILITY_ACTION_LONG_CLICK },
        { AceAction::ACTION_SCROLL_FORWARD, ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD },
        { AceAction::ACTION_SCROLL_BACKWARD, ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD },
        { AceAction::ACTION_FOCUS, ActionType::ACCESSIBILITY_ACTION_FOCUS },
        { AceAction::ACTION_CLEAR_FOCUS, ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS },
        { AceAction::ACTION_ACCESSIBILITY_FOCUS, ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS },
        { AceAction::ACTION_CLEAR_ACCESSIBILITY_FOCUS, ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS },
        { AceAction::ACTION_NEXT_AT_MOVEMENT_GRANULARITY, ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT },
        { AceAction::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY, ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT },
        { AceAction::ACTION_SET_TEXT, ActionType::ACCESSIBILITY_ACTION_SET_TEXT },
        { AceAction::ACTION_COPY, ActionType::ACCESSIBILITY_ACTION_COPY },
        { AceAction::ACTION_PASTE, ActionType::ACCESSIBILITY_ACTION_PASTE },
        { AceAction::ACTION_CUT, ActionType::ACCESSIBILITY_ACTION_CUT },
        { AceAction::ACTION_SELECT, ActionType::ACCESSIBILITY_ACTION_SELECT },
        { AceAction::ACTION_CLEAR_SELECTION, ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION },
        { AceAction::ACTION_SET_SELECTION, ActionType::ACCESSIBILITY_ACTION_SET_SELECTION },
        { AceAction::ACTION_SET_CURSOR_POSITION, ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION },
    };
    for (const auto& item : actionTable) {
        if (aceAction == item.aceAction) {
            return item.action;
        }
    }
    return ActionType::ACCESSIBILITY_ACTION_INVALID;
}

inline RangeInfo ConvertAccessibilityValue(const AccessibilityValue& value)
{
    return RangeInfo(static_cast<int>(value.min), static_cast<int>(value.max), static_cast<int>(value.current));
}

int64_t ConvertToCardAccessibilityId(int64_t nodeId, int64_t cardId, int64_t rootNodeId)
{
    // result is integer total ten digits, top five for agp virtualViewId, end five for ace nodeId,
    // for example agp virtualViewId is 32, ace nodeId is 1000001, convert to result is 00032 10001.
    int64_t result = 0;
    if (nodeId == rootNodeId + ROOT_STACK_BASE) {
        // for example agp virtualViewId is 32 root node is 2100000, convert to result is 00032 21000.
        result = cardId * CARD_BASE + (static_cast<int64_t>(nodeId / CARD_BASE)) * CARD_ROOT_NODE_ID_RATION +
                 nodeId % CARD_BASE;
    } else {
        result = cardId * CARD_BASE + (static_cast<int64_t>(nodeId / DOM_ROOT_NODE_ID_BASE)) * CARD_NODE_ID_RATION +
                 nodeId % DOM_ROOT_NODE_ID_BASE;
    }
    return result;
}

void UpdateAccessibilityNodeInfo(const RefPtr<AccessibilityNode>& node, AccessibilityElementInfo& nodeInfo,
    const RefPtr<JsAccessibilityManager>& manager, int windowId)
{
    int leftTopX = static_cast<int>(node->GetLeft()) + manager->GetWindowLeft(node->GetWindowId());
    int leftTopY = static_cast<int>(node->GetTop()) + manager->GetWindowTop(node->GetWindowId());
    int rightBottomX = leftTopX + static_cast<int>(node->GetWidth());
    int rightBottomY = leftTopY + static_cast<int>(node->GetHeight());
    if (manager->isOhosHostCard()) {
        int64_t id = ConvertToCardAccessibilityId(node->GetNodeId(), manager->GetCardId(), manager->GetRootNodeId());
        nodeInfo.SetAccessibilityId(id);
        if (node->GetParentId() == -1) {
            nodeInfo.SetParent(-1);
        } else {
            nodeInfo.SetParent(
                ConvertToCardAccessibilityId(node->GetParentId(), manager->GetCardId(), manager->GetRootNodeId()));
        }
        leftTopX = static_cast<int>(node->GetLeft() + manager->GetCardOffset().GetX());
        leftTopY = static_cast<int>(node->GetTop() + manager->GetCardOffset().GetY());
        rightBottomX = leftTopX + static_cast<int>(node->GetWidth());
        rightBottomY = leftTopY + static_cast<int>(node->GetHeight());
        Accessibility::Rect bounds(leftTopX, leftTopY, rightBottomX, rightBottomY);
        nodeInfo.SetRectInScreen(bounds);
    } else {
        if (node->GetTag() == SIDEBARCONTAINER_TAG) {
            Rect sideBarRect = node->GetRect();
            for (const auto& childNode : node->GetChildList()) {
                sideBarRect = sideBarRect.CombineRect(childNode->GetRect());
            }
            leftTopX = static_cast<int>(sideBarRect.Left()) + manager->GetWindowLeft(node->GetWindowId());
            leftTopY = static_cast<int>(sideBarRect.Top()) + manager->GetWindowTop(node->GetWindowId());
            rightBottomX = static_cast<int>(sideBarRect.Right()) + manager->GetWindowLeft(node->GetWindowId());
            rightBottomY = static_cast<int>(sideBarRect.Bottom()) + manager->GetWindowTop(node->GetWindowId());
        }
        Accessibility::Rect bounds(leftTopX, leftTopY, rightBottomX, rightBottomY);
        nodeInfo.SetRectInScreen(bounds);
        nodeInfo.SetComponentId(static_cast<int64_t>(node->GetNodeId()));
        nodeInfo.SetParent(static_cast<int64_t>(node->GetParentId()));
    }

    if (node->GetParentId() == -1) {
        const auto& children = node->GetChildList();
        if (!children.empty()) {
            auto lastChildNode = manager->GetAccessibilityNodeById(children.back()->GetNodeId());
            if (lastChildNode) {
                rightBottomX = leftTopX + static_cast<int>(lastChildNode->GetWidth());
                rightBottomY = leftTopY + static_cast<int>(lastChildNode->GetHeight());
                Accessibility::Rect bounds(leftTopX, leftTopY, rightBottomX, rightBottomY);
                nodeInfo.SetRectInScreen(bounds);
            }
        }
        nodeInfo.SetParent(INVALID_PARENT_ID);
    }
    if (node->GetNodeId() == 0) {
        nodeInfo.SetParent(INVALID_PARENT_ID);
    }
    nodeInfo.SetPagePath(manager->GetPagePath());
    nodeInfo.SetWindowId(windowId);
    nodeInfo.SetChecked(node->GetCheckedState());
    nodeInfo.SetEnabled(node->GetEnabledState());
    nodeInfo.SetFocused(node->GetFocusedState());
    nodeInfo.SetSelected(node->GetSelectedState());
    nodeInfo.SetCheckable(node->GetCheckableState());
    nodeInfo.SetClickable(node->GetClickableState());
    nodeInfo.SetFocusable(node->GetFocusableState());
    nodeInfo.SetScrollable(node->GetScrollableState());
    nodeInfo.SetLongClickable(node->GetLongClickableState());
    nodeInfo.SetEditable(node->GetEditable());
    nodeInfo.SetPluraLineSupported(node->GetIsMultiLine());
    nodeInfo.SetPassword(node->GetIsPassword());
    nodeInfo.SetTextLengthLimit(node->GetMaxTextLength());
    nodeInfo.SetSelectedBegin(node->GetTextSelectionStart());
    nodeInfo.SetSelectedEnd(node->GetTextSelectionEnd());
    nodeInfo.SetVisible(node->GetShown() && node->GetVisible());
    nodeInfo.SetHint(node->GetHintText());
    std::string accessibilityLabel = node->GetAccessibilityLabel();
    nodeInfo.SetLabeled(atol(accessibilityLabel.c_str()));
    nodeInfo.SetError(node->GetErrorText());
    nodeInfo.SetComponentResourceId(node->GetJsComponentId());
    nodeInfo.SetInspectorKey(node->GetJsComponentId());
    RangeInfo rangeInfo = ConvertAccessibilityValue(node->GetAccessibilityValue());
    nodeInfo.SetRange(rangeInfo);
    nodeInfo.SetInputType(static_cast<int>(node->GetTextInputType()));
    nodeInfo.SetComponentType(node->GetTag());
    GridInfo gridInfo(
        node->GetCollectionInfo().rows, node->GetCollectionInfo().columns, (nodeInfo.IsPluraLineSupported() ? 0 : 1));
    nodeInfo.SetGrid(gridInfo);
    nodeInfo.SetAccessibilityFocus(node->GetAccessibilityFocusedState());
    nodeInfo.SetPageId(node->GetPageId());

    int32_t row = node->GetCollectionItemInfo().row;
    int32_t column = node->GetCollectionItemInfo().column;
    GridItemInfo gridItemInfo(row, row, column, column, false, nodeInfo.IsSelected());
    nodeInfo.SetGridItem(gridItemInfo);
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    if (node->GetTag() == LIST_TAG) {
        nodeInfo.SetItemCounts(node->GetListItemCounts());
        nodeInfo.SetBeginIndex(node->GetListBeginIndex());
        nodeInfo.SetEndIndex(node->GetListEndIndex());
    }
    if (node->GetIsPassword()) {
        std::string strStar(node->GetText().size(), '*');
        nodeInfo.SetContent(strStar);
    } else {
        nodeInfo.SetContent(node->GetText());
    }

    auto supportAceActions = node->GetSupportAction();
    std::vector<ActionType> actions(supportAceActions.size());

    for (auto it = supportAceActions.begin(); it != supportAceActions.end(); ++it) {
        AccessibleAction action(ConvertAceAction(*it), "ace");
        nodeInfo.AddAction(action);
    }

    if (node->GetImportantForAccessibility() == NG::AccessibilityProperty::Level::YES) {
        actions.emplace_back(ActionType::ACCESSIBILITY_ACTION_FOCUS);
        nodeInfo.SetCheckable(true);
    } else if (node->GetImportantForAccessibility() == NG::AccessibilityProperty::Level::NO ||
               node->GetImportantForAccessibility() == NG::AccessibilityProperty::Level::NO_HIDE_DESCENDANTS) {
        nodeInfo.SetVisible(false);
    }

    manager->UpdateNodeChildIds(node);
    for (const auto& child : node->GetChildIds()) {
        nodeInfo.AddChild(child);
    }

#ifdef ACE_DEBUG
    std::string actionForLog;
    for (const auto& action : supportAceActions) {
        if (!actionForLog.empty()) {
            actionForLog.append(",");
        }
        actionForLog.append(std::to_string(static_cast<int32_t>(action)));
    }
#endif
}

void UpdateCacheInfo(std::list<AccessibilityElementInfo>& infos, uint32_t mode, const RefPtr<AccessibilityNode>& node,
    const RefPtr<JsAccessibilityManager>& jsAccessibilityManager, int windowId)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "elementId: %{public}d, mode: %{public}d", node->GetNodeId(), mode);
    // parent
    uint32_t umode = mode;
    if (umode & static_cast<uint32_t>(PREFETCH_PREDECESSORS)) {
        if (node->GetParentId() != -1 && node->GetParentId() != DEFAULT_PARENT_ID) {
            AccessibilityElementInfo parentNodeInfo;
            UpdateAccessibilityNodeInfo(node->GetParentNode(), parentNodeInfo, jsAccessibilityManager, windowId);
            infos.emplace_back(parentNodeInfo);
        }
    }
    // sister/brothers
    if (umode & static_cast<uint32_t>(PREFETCH_SIBLINGS)) {
        if (node->GetParentId() != -1 && node->GetParentId() != DEFAULT_PARENT_ID) {
            for (const auto& item : node->GetParentNode()->GetChildList()) {
                if (node->GetNodeId() != item->GetNodeId()) {
                    AccessibilityElementInfo siblingNodeInfo;
                    UpdateAccessibilityNodeInfo(item, siblingNodeInfo, jsAccessibilityManager, windowId);
                    infos.emplace_back(siblingNodeInfo);
                }
            }
        }
    }
    // children
    if (umode & static_cast<uint32_t>(PREFETCH_CHILDREN)) {
        for (const auto& item : node->GetChildList()) {
            AccessibilityElementInfo childNodeInfo;
            UpdateAccessibilityNodeInfo(item, childNodeInfo, jsAccessibilityManager, windowId);
            infos.emplace_back(childNodeInfo);
        }
    }

    // get all children
    if (umode & static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN)) {
        std::list<RefPtr<AccessibilityNode>> children;
        for (const auto& item : node->GetChildList()) {
            children.emplace_back(item);
        }

        while (!children.empty()) {
            auto parent = children.front();
            children.pop_front();
            AccessibilityElementInfo childNodeInfo;
            UpdateAccessibilityNodeInfo(parent, childNodeInfo, jsAccessibilityManager, windowId);
            infos.push_back(childNodeInfo);
            for (const auto& item : parent->GetChildList()) {
                children.emplace_back(item);
            }
        }
    }
}

void SortAccessibilityInfosByBreadth(std::list<AccessibilityElementInfo>& infos,
    std::list<int64_t>& accessibilityIdQueue, std::list<AccessibilityElementInfo>& output)
{
    while (!accessibilityIdQueue.empty()) {
        auto accessibilityId = accessibilityIdQueue.front();
        accessibilityIdQueue.pop_front();
        for (std::list<AccessibilityElementInfo>::iterator info = infos.begin(); info != infos.end(); ++info) {
            if (accessibilityId != info->GetAccessibilityId()) {
                continue;
            }
            for (auto& child : info->GetChildIds()) {
                accessibilityIdQueue.emplace_back(child);
            }
            output.emplace_back(*info);
            infos.erase(info);
            break;
        }
        SortAccessibilityInfosByBreadth(infos, accessibilityIdQueue, output);
    }
}

void SortExtensionAccessibilityInfo(std::list<AccessibilityElementInfo>& infos, int64_t rootAccessibilityId)
{
    auto input = infos;
    infos.clear();
    std::list<int64_t> accessibilityIdQueue;
    accessibilityIdQueue.emplace_back(rootAccessibilityId);
    SortAccessibilityInfosByBreadth(input, accessibilityIdQueue, infos);
}

void ConvertExtensionAccessibilityId(AccessibilityElementInfo& info, const RefPtr<NG::FrameNode>& extensionNode,
    int64_t uiExtensionOffset, AccessibilityElementInfo& parentInfo)
{
    auto extensionAbilityId =
            extensionNode->WrapExtensionAbilityId(uiExtensionOffset, info.GetAccessibilityId());
    info.SetAccessibilityId(extensionAbilityId);
    auto parentNodeId =
        extensionNode->WrapExtensionAbilityId(uiExtensionOffset, info.GetParentNodeId());
    info.SetParent(parentNodeId);
    auto childIds = info.GetChildIds();
    for (auto& child : childIds) {
        info.RemoveChild(child);
        info.AddChild(extensionNode->WrapExtensionAbilityId(uiExtensionOffset, child));
    }
    if (V2::ROOT_ETS_TAG == info.GetComponentType()) {
        for (auto& child : info.GetChildIds()) {
            parentInfo.AddChild(child);
        }
    }
}

void ConvertExtensionAccessibilityNodeId(std::list<AccessibilityElementInfo>& infos,
    const RefPtr<NG::FrameNode>& extensionNode, int64_t uiExtensionOffset,
    AccessibilityElementInfo& parentInfo)
{
    CHECK_NULL_VOID(extensionNode);
    for (auto& accessibilityElementInfo : infos) {
        ConvertExtensionAccessibilityId(accessibilityElementInfo, extensionNode, uiExtensionOffset, parentInfo);
    }
    for (auto& accessibilityElementInfo : infos) {
        if (std::find(parentInfo.GetChildIds().begin(), parentInfo.GetChildIds().end(),
            accessibilityElementInfo.GetAccessibilityId()) != parentInfo.GetChildIds().end()) {
            accessibilityElementInfo.SetParent(extensionNode->GetAccessibilityId());
        }
    }
}

inline std::string BoolToString(bool tag)
{
    return tag ? "true" : "false";
}

std::string ConvertInputTypeToString(AceTextCategory type)
{
    switch (type) {
        case AceTextCategory::INPUT_TYPE_DEFAULT:
            return "INPUT_TYPE_DEFAULT";
        case AceTextCategory::INPUT_TYPE_TEXT:
            return "INPUT_TYPE_TEXT";
        case AceTextCategory::INPUT_TYPE_EMAIL:
            return "INPUT_TYPE_EMAIL";
        case AceTextCategory::INPUT_TYPE_DATE:
            return "INPUT_TYPE_DATE";
        case AceTextCategory::INPUT_TYPE_TIME:
            return "INPUT_TYPE_TIME";
        case AceTextCategory::INPUT_TYPE_NUMBER:
            return "INPUT_TYPE_NUMBER";
        case AceTextCategory::INPUT_TYPE_PASSWORD:
            return "INPUT_TYPE_PASSWORD";
        case AceTextCategory::INPUT_TYPE_PHONENUMBER:
            return "INPUT_TYPE_PHONENUMBER";
        case AceTextCategory::INPUT_TYPE_USER_NAME:
            return "INPUT_TYPE_USER_NAME";
        case AceTextCategory::INPUT_TYPE_NEW_PASSWORD:
            return "INPUT_TYPE_NEW_PASSWORD";
        default:
            return "illegal input type";
    }
}

bool FindAccessibilityFocus(const RefPtr<AccessibilityNode>& node, RefPtr<AccessibilityNode>& resultNode)
{
    CHECK_NULL_RETURN(node, false);
    if (node->GetAccessibilityFocusedState()) {
        resultNode = node;
        return true;
    }
    if (!node->GetChildList().empty()) {
        for (const auto& item : node->GetChildList()) {
            if (resultNode != nullptr) {
                return true;
            }
            if (FindAccessibilityFocus(item, resultNode)) {
                return true;
            }
        }
    }

    return false;
}

void FindFocusedExtensionElementInfoNG(const SearchParameter& searchParam,
    const RefPtr<NG::FrameNode>& node, Accessibility::AccessibilityElementInfo& info)
{
    if (NG::UI_EXTENSION_OFFSET_MIN < (searchParam.uiExtensionOffset + 1)) {
        node->FindFocusedExtensionElementInfoNG(searchParam.nodeId, searchParam.mode,
            searchParam.uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR, info);
    } else {
        info.SetValidElement(false);
    }
}

#ifdef WEB_SUPPORTED
RefPtr<NG::FrameNode> FindWebAccessibilityFocus(const RefPtr<NG::FrameNode>& frameNode)
{
    if (frameNode->GetTag() == V2::WEB_ETS_TAG) {
        if (!frameNode->GetRenderContext()->GetAccessibilityFocus().value_or(false)) {
            return nullptr;
        }
        auto webPattern = frameNode->GetPattern<NG::WebPattern>();
        CHECK_NULL_RETURN(webPattern, nullptr);
        auto result = webPattern->GetFocusedAccessibilityNode(-1, true);
        if (result) {
            return result;
        }
    } else if (frameNode->GetTag() == V2::WEB_CORE_TAG) {
        auto webAccessibilityNode = AceType::DynamicCast<NG::WebAccessibilityNode>(frameNode);
        CHECK_NULL_RETURN(webAccessibilityNode, nullptr);
        auto webNode = webAccessibilityNode->GetWebNode();
        CHECK_NULL_RETURN(webNode, nullptr);
        auto webPattern = webNode->GetPattern<NG::WebPattern>();
        CHECK_NULL_RETURN(webPattern, nullptr);
        auto result = webPattern->GetFocusedAccessibilityNode(
            webAccessibilityNode->GetAccessibilityId(), true);
        if (result) {
            return result;
        }
    }
    return nullptr;
}
#endif

void SetUiExtensionAbilityParentIdForFocus(const RefPtr<NG::UINode>& uiExtensionNode,
    const int64_t uiExtensionOffset, Accessibility::AccessibilityElementInfo& info)
{
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiExtensionNode);
    if (frameNode) {
        auto parentId = info.GetParentNodeId();
        AccessibilityElementInfo parentInfo;
        ConvertExtensionAccessibilityId(info, frameNode, uiExtensionOffset, parentInfo);
        if (parentId == NG::UI_EXTENSION_ROOT_ID) {
            info.SetParent(frameNode->GetAccessibilityId());
        }
    }
}

RefPtr<NG::FrameNode> FindAccessibilityFocus(const RefPtr<NG::UINode>& node,
    int32_t focusType, Accessibility::AccessibilityElementInfo& info,
    const int64_t uiExtensionOffset, const RefPtr<PipelineBase>& context)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    if (frameNode) {
#ifdef WEB_SUPPORTED
        auto result = FindWebAccessibilityFocus(frameNode);
        if (result) {
            return result;
        }
#endif
        if (frameNode->GetRenderContext()->GetAccessibilityFocus().value_or(false)) {
            return frameNode;
        }
    }
    if (node->GetChildren().empty()) {
        return nullptr;
    }
    for (const auto& child : node->GetChildren()) {
        auto extensionNode = AceType::DynamicCast<NG::FrameNode>(child);
        if (IsExtensionComponent(child) && extensionNode &&
            !IsUIExtensionShowPlaceholder(extensionNode) &&
            (extensionNode->GetUiExtensionId() > NG::UI_EXTENSION_UNKNOW_ID) &&
            (((extensionNode->GetUiExtensionId() <= NG::UI_EXTENSION_ID_FIRST_MAX) &&
            (NG::UI_EXTENSION_OFFSET_MAX == uiExtensionOffset)) ||
            (extensionNode->GetUiExtensionId() <= NG::UI_EXTENSION_ID_OTHER_MAX))) {
            SearchParameter transferSearchParam {NG::UI_EXTENSION_ROOT_ID, "", focusType, uiExtensionOffset};
            OHOS::Ace::Framework::FindFocusedExtensionElementInfoNG(
                transferSearchParam, extensionNode, info);
            if (info.GetAccessibilityId() < 0) {
                continue;
            }
            SetUiExtensionAbilityParentIdForFocus(extensionNode, uiExtensionOffset, info);
            return extensionNode;
        }
        auto result = FindAccessibilityFocus(child, focusType, info, uiExtensionOffset, context);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

bool FindInputFocus(const RefPtr<AccessibilityNode>& node, RefPtr<AccessibilityNode>& resultNode)
{
    CHECK_NULL_RETURN(node, false);
    if (!node->GetFocusedState() && (node->GetParentId() != -1)) {
        return false;
    }
    if (node->GetFocusedState()) {
        resultNode = node;
    }
    if (!node->GetChildList().empty()) {
        for (const auto& item : node->GetChildList()) {
            if (FindInputFocus(item, resultNode)) {
                return true;
            }
        }
    }
    return node->GetFocusedState();
}

#ifdef WEB_SUPPORTED
RefPtr<NG::FrameNode> FindWebInputFocus(const RefPtr<NG::FrameNode>& frameNode)
{
    if (frameNode->GetTag() == V2::WEB_ETS_TAG) {
        if (!(frameNode->GetFocusHub() ? frameNode->GetFocusHub()->IsCurrentFocus() : false)) {
            return nullptr;
        }
        auto webPattern = frameNode->GetPattern<NG::WebPattern>();
        CHECK_NULL_RETURN(webPattern, nullptr);
        auto result = webPattern->GetFocusedAccessibilityNode(-1, false);
        if (result) {
            return result;
        }
    } else if (frameNode->GetTag() == V2::WEB_CORE_TAG) {
        auto webAccessibilityNode = AceType::DynamicCast<NG::WebAccessibilityNode>(frameNode);
        CHECK_NULL_RETURN(webAccessibilityNode, nullptr);
        auto webNode = webAccessibilityNode->GetWebNode();
        CHECK_NULL_RETURN(webNode, nullptr);
        auto webPattern = webNode->GetPattern<NG::WebPattern>();
        CHECK_NULL_RETURN(webPattern, nullptr);
        auto result = webPattern->GetFocusedAccessibilityNode(
            webAccessibilityNode->GetAccessibilityId(), false);
        if (result) {
            return result;
        }
    }
    return nullptr;
}
#endif

RefPtr<NG::FrameNode> FindInputFocus(const RefPtr<NG::UINode>& node, int32_t focusType,
    Accessibility::AccessibilityElementInfo& info, const int64_t uiExtensionOffset,
    const RefPtr<PipelineBase>& context)
{
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    CHECK_NULL_RETURN(frameNode, nullptr);
#ifdef WEB_SUPPORTED
    auto result = FindWebInputFocus(frameNode);
    if (result) {
        return result;
    }
#endif
    if (!(frameNode->GetFocusHub() ? frameNode->GetFocusHub()->IsCurrentFocus() : false)) {
        return nullptr;
    }
    if (frameNode->GetFocusHub()->IsChild()) {
        if (frameNode->IsInternal()) {
            return frameNode->GetFocusParent();
        }
        return frameNode;
    }
    auto focusHub = frameNode->GetFocusHub();
    auto focusChildren = focusHub->GetChildren();
    for (const auto& focusChild : focusChildren) {
        auto extensionNode = focusChild->GetFrameNode();
        if ((extensionNode && IsExtensionComponent(extensionNode)) &&
            !IsUIExtensionShowPlaceholder(extensionNode) &&
            (extensionNode->GetUiExtensionId() > NG::UI_EXTENSION_UNKNOW_ID) &&
            (((extensionNode->GetUiExtensionId() <= NG::UI_EXTENSION_ID_FIRST_MAX) &&
            (NG::UI_EXTENSION_OFFSET_MAX == uiExtensionOffset)) ||
            (extensionNode->GetUiExtensionId() <= NG::UI_EXTENSION_ID_OTHER_MAX))) {
            SearchParameter transferSearchParam {NG::UI_EXTENSION_ROOT_ID, "", focusType, uiExtensionOffset};
            OHOS::Ace::Framework::FindFocusedExtensionElementInfoNG(
                transferSearchParam, extensionNode, info);
            if (info.GetAccessibilityId() < 0) {
                continue;
            }
            SetUiExtensionAbilityParentIdForFocus(extensionNode, uiExtensionOffset, info);
            return extensionNode;
        }
        auto childNode = FindInputFocus(focusChild->GetFrameNode(), focusType, info, uiExtensionOffset, context);
        if (childNode) {
            return childNode;
        }
    }
    return nullptr;
}

void FindText(
    const RefPtr<AccessibilityNode>& node, const std::string& text, std::list<RefPtr<AccessibilityNode>>& nodeList)
{
    CHECK_NULL_VOID(node);
    if (node->GetText().find(text) != std::string::npos) {
        nodeList.push_back(node);
    }
    if (!node->GetChildList().empty()) {
        for (const auto& child : node->GetChildList()) {
            FindText(child, text, nodeList);
        }
    }
}

void FindText(const RefPtr<NG::UINode>& node, const std::string& text, std::list<RefPtr<NG::FrameNode>>& nodeList)
{
    CHECK_NULL_VOID(node);

    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    if (frameNode && !frameNode->IsInternal()) {
        if (frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>()->GetGroupText().find(text) !=
            std::string::npos) {
            nodeList.push_back(frameNode);
        }
    }

    if (!node->GetChildren().empty()) {
        for (const auto& child : node->GetChildren()) {
            FindText(child, text, nodeList);
        }
    }
}

#ifdef WEB_SUPPORTED
std::vector<RefPtr<NG::FrameNode>> GetWebs(const RefPtr<NG::FrameNode>& root)
{
    std::vector<RefPtr<NG::FrameNode>> results;
    CHECK_NULL_RETURN(root, results);
    std::queue<RefPtr<NG::UINode>> nodes;
    nodes.push(root);
    RefPtr<NG::FrameNode> frameNode;
    while (!nodes.empty()) {
        auto current = nodes.front();
        nodes.pop();
        frameNode = AceType::DynamicCast<NG::FrameNode>(current);
        if (frameNode != nullptr && frameNode->GetTag() == V2::WEB_ETS_TAG) {
            results.emplace_back(frameNode);
        }
        if (current != nullptr) {
            const auto& children = current->GetChildren();
            for (const auto& child : children) {
                nodes.push(child);
            }
        }
    }
    return results;
}

RefPtr<NG::FrameNode> GetWebCoreNodeById(const NG::FrameNode* frameNode, int64_t id)
{
    if (frameNode->IsNodeHasFlag(NG::NodeFlag::WEB_TAG)) {
        auto webPattern = frameNode->GetPattern<NG::WebPattern>();
        CHECK_NULL_RETURN(webPattern, nullptr);
        auto webAccessibilityNode = AceType::DynamicCast<NG::FrameNode>(webPattern->GetAccessibilityNodeById(id));
        if (webAccessibilityNode != nullptr) {
            return webAccessibilityNode;
        }
    } else if (frameNode->IsNodeHasFlag(NG::NodeFlag::WEB_CORE_TAG)) {
        auto webAccessibilityNode = AceType::DynamicCast<NG::WebAccessibilityNode>(frameNode);
        CHECK_NULL_RETURN(webAccessibilityNode, nullptr);
        auto webNode = webAccessibilityNode->GetWebNode();
        CHECK_NULL_RETURN(webNode, nullptr);
        auto webPattern = webNode->GetPattern<NG::WebPattern>();
        CHECK_NULL_RETURN(webPattern, nullptr);
        auto retWebAccessibilityNode = AceType::DynamicCast<NG::FrameNode>(webPattern->GetAccessibilityNodeById(id));
        if (retWebAccessibilityNode != nullptr) {
            return retWebAccessibilityNode;
        }
    }
    return nullptr;
}
#endif

RefPtr<NG::FrameNode> GetFramenodeByAccessibilityId(const RefPtr<NG::FrameNode>& root, int64_t id)
{
    CHECK_NULL_RETURN(root, nullptr);
    if (root->GetAccessibilityId() == id) {
        return root;
    }
#ifdef WEB_SUPPORTED
    auto result = GetWebCoreNodeById(Referenced::RawPtr(root), id);
    if (result) {
        return result;
    }
#endif
    std::queue<NG::UINode*> nodes;
    nodes.push(Referenced::RawPtr(root));
    NG::FrameNode* frameNode = nullptr;

    while (!nodes.empty()) {
        auto current = nodes.front();
        nodes.pop();
        const auto& children = current->GetChildren();
        for (const auto& child : children) {
            frameNode = AceType::DynamicCast<NG::FrameNode>(Referenced::RawPtr(child));
            if (frameNode != nullptr && !frameNode->CheckAccessibilityLevelNo()) {
                if (frameNode->GetAccessibilityId() == id) {
                    return AceType::DynamicCast<NG::FrameNode>(child);
                }
#ifdef WEB_SUPPORTED
                auto result = GetWebCoreNodeById(frameNode, id);
                if (result) {
                    return result;
                }
#endif
            }

            if (frameNode != nullptr && frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>() != nullptr) {
                auto property = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
                if (property->GetAccessibilityLevel() != NG::AccessibilityProperty::Level::NO_HIDE_DESCENDANTS &&
                !property->IsAccessibilityGroup()) {
                    nodes.push(Referenced::RawPtr(child));
                }
            } else {
                nodes.push(Referenced::RawPtr(child));
            }
        }
    }
    return nullptr;
}

void GetFrameNodeParent(const RefPtr<NG::UINode>& uiNode, RefPtr<NG::FrameNode>& parent)
{
    if (AceType::InstanceOf<NG::FrameNode>(uiNode)) {
        auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
        if (!frameNode->IsInternal()) {
            parent = frameNode;
            return;
        }
    }
    CHECK_NULL_VOID(uiNode);
    auto parentNode = uiNode->GetParent();
    GetFrameNodeParent(parentNode, parent);
}

bool CheckFrameNodeByAccessibilityLevel(const RefPtr<NG::FrameNode>& frameNode, bool isParent)
{
    bool ret = false;
    CHECK_NULL_RETURN(frameNode, false);
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, false);
    auto uiNode = frameNode->GetParent();
    RefPtr<NG::FrameNode> parentNode;
    if (uiNode != nullptr) {
        GetFrameNodeParent(uiNode, parentNode);
    }

    auto accessibilityLevel = accessibilityProperty->GetAccessibilityLevel();
    if (isParent) {
        if (accessibilityLevel == NG::AccessibilityProperty::Level::NO_HIDE_DESCENDANTS) {
            ret = false;
            return ret;
        }
        if (!parentNode) {
            if (accessibilityProperty->IsAccessibilityGroup()) {
                ret = false;
            } else {
                ret = true;
            }
        } else {
            if (accessibilityProperty->IsAccessibilityGroup()) {
                ret = false;
            } else {
                ret = CheckFrameNodeByAccessibilityLevel(parentNode, true);
            }
        }
    } else {
        if (accessibilityLevel == NG::AccessibilityProperty::Level::AUTO ||
            accessibilityLevel == NG::AccessibilityProperty::Level::YES) {
            ret = true;
            if (!parentNode) {
                return ret;
            }
            auto parentAccessibilityProperty = parentNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
            if (parentAccessibilityProperty->IsAccessibilityGroup()) {
                ret = false;
            } else {
                ret = CheckFrameNodeByAccessibilityLevel(parentNode, true);
            }
        } else {
            ret = false;
        }
    }
    return ret;
}

void GetFrameNodeChildren(const RefPtr<NG::UINode>& uiNode, std::vector<int64_t>& children, int32_t pageId)
{
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
    if (AceType::InstanceOf<NG::FrameNode>(uiNode)) {
        CHECK_NULL_VOID(frameNode->IsActive());
        if (uiNode->GetTag() == "stage") {
        } else if (uiNode->GetTag() == "page") {
            if (uiNode->GetPageId() != pageId) {
                return;
            }
        } else if (!frameNode->IsInternal()) {
            if (CheckFrameNodeByAccessibilityLevel(frameNode, false)) {
                children.emplace_back(uiNode->GetAccessibilityId());
                return;
            }
        }
    }

    for (const auto& frameChild : uiNode->GetChildren()) {
        GetFrameNodeChildren(frameChild, children, pageId);
    }
}

void GetFrameNodeChildren(
    const RefPtr<NG::UINode>& uiNode, std::list<RefPtr<NG::FrameNode>>& children, int32_t pageId = -1)
{
    if (AceType::InstanceOf<NG::FrameNode>(uiNode)) {
        auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
        CHECK_NULL_VOID(frameNode->IsActive());
        if (uiNode->GetTag() == "page") {
            if (pageId != -1 && uiNode->GetPageId() != pageId) {
                return;
            }
        } else if (!frameNode->IsInternal() && uiNode->GetTag() != "stage") {
            if (CheckFrameNodeByAccessibilityLevel(frameNode, false)) {
                children.emplace_back(frameNode);
                return;
            }
        }
    }
    for (const auto& frameChild : uiNode->GetChildren()) {
        GetFrameNodeChildren(frameChild, children, pageId);
    }
}

int64_t GetParentId(const RefPtr<NG::UINode>& uiNode)
{
    auto parent = uiNode->GetParent();
    while (parent) {
        if (AceType::InstanceOf<NG::FrameNode>(parent)) {
            if ((parent->GetTag() == V2::PAGE_ETS_TAG) || (parent->GetTag() == V2::STAGE_ETS_TAG) ||
                AceType::DynamicCast<NG::FrameNode>(parent)->CheckAccessibilityLevelNo()) {
                parent = parent->GetParent();
                continue;
            }
            return parent->GetAccessibilityId();
        }
        parent = parent->GetParent();
    }
    return INVALID_PARENT_ID;
}

void FillElementInfo(int64_t elementId, AccessibilityElementInfo& elementInfo,
    const RefPtr<PipelineBase>& context, const RefPtr<JsAccessibilityManager>& jsAccessibilityManager)
{
    std::list<AccessibilityElementInfo> elementInfos;
    int32_t mode = 0;
    CHECK_NULL_VOID(jsAccessibilityManager);
    jsAccessibilityManager->SearchElementInfoByAccessibilityIdNG(
        elementId, mode, elementInfos, context, NG::UI_EXTENSION_OFFSET_MAX);
    if (elementInfos.empty()) {
        LOGE("Element infos is empty. Find element infos failed.");
        return;
    }
    elementInfo = elementInfos.front();
}

void FillEventInfo(const RefPtr<NG::FrameNode>& node,
                   AccessibilityEventInfo& eventInfo,
                   const RefPtr<PipelineBase>& context,
                   int64_t elementId,
                   const RefPtr<JsAccessibilityManager>& jsAccessibilityManager)
{
    CHECK_NULL_VOID(node);
    if (node->GetTag() == V2::WEB_CORE_TAG) {
        auto webAccessibilityNode = AceType::DynamicCast<NG::WebAccessibilityNode>(node);
        CHECK_NULL_VOID(webAccessibilityNode);
        eventInfo.SetComponentType(webAccessibilityNode->GetComponentType());
        eventInfo.SetPageId(webAccessibilityNode->GetPageId());
        eventInfo.AddContent(webAccessibilityNode->GetContent());
        AccessibilityElementInfo elementInfo;
        FillElementInfo(elementId, elementInfo, context, jsAccessibilityManager);
        eventInfo.SetElementInfo(elementInfo);
        return;
    }
    eventInfo.SetComponentType(node->GetTag());
    eventInfo.SetPageId(node->GetPageId());
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    eventInfo.AddContent(accessibilityProperty->GetGroupText());
    eventInfo.SetItemCounts(accessibilityProperty->GetCollectionItemCounts());
    eventInfo.SetBeginIndex(accessibilityProperty->GetBeginIndex());
    eventInfo.SetEndIndex(accessibilityProperty->GetEndIndex());
    AccessibilityElementInfo elementInfo;
    FillElementInfo(elementId, elementInfo, context, jsAccessibilityManager);
    eventInfo.SetElementInfo(elementInfo);
}

void FillEventInfo(const RefPtr<AccessibilityNode>& node, AccessibilityEventInfo& eventInfo)
{
    eventInfo.SetComponentType(node->GetTag());
    if (node->GetTag() == LIST_TAG) {
        eventInfo.SetItemCounts(node->GetListItemCounts());
        eventInfo.SetBeginIndex(node->GetListBeginIndex());
        eventInfo.SetEndIndex(node->GetListEndIndex());
    }
    eventInfo.SetPageId(node->GetPageId());
    eventInfo.AddContent(node->GetText());
    eventInfo.SetLatestContent(node->GetText());
}

inline bool IsPopupSupported(const RefPtr<NG::PipelineContext>& pipeline, int64_t nodeId)
{
    CHECK_NULL_RETURN(pipeline, false);
    auto overlayManager = pipeline->GetOverlayManager();
    if (overlayManager) {
        return overlayManager->HasPopupInfo(nodeId);
    }
    return false;
}

void SetAccessibilityFocusAction(AccessibilityElementInfo& nodeInfo, const char* tag)
{
    if (nodeInfo.HasAccessibilityFocus()) {
        AccessibleAction action(ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS, tag);
        nodeInfo.AddAction(action);
    } else {
        AccessibleAction action(ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS, tag);
        nodeInfo.AddAction(action);
    }
}

void UpdateSupportAction(const RefPtr<NG::FrameNode>& node, AccessibilityElementInfo& nodeInfo)
{
    CHECK_NULL_VOID(node);
    if (nodeInfo.IsFocusable()) {
        if (nodeInfo.IsFocused()) {
            AccessibleAction action(ACCESSIBILITY_ACTION_CLEAR_FOCUS, "ace");
            nodeInfo.AddAction(action);
        } else {
            AccessibleAction action(ACCESSIBILITY_ACTION_FOCUS, "ace");
            nodeInfo.AddAction(action);
        }
    }

    auto eventHub = node->GetEventHub<NG::EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto gestureEventHub = eventHub->GetGestureEventHub();
    CHECK_NULL_VOID(gestureEventHub);
    nodeInfo.SetClickable(gestureEventHub->IsAccessibilityClickable());
    if (gestureEventHub->IsAccessibilityClickable()) {
        AccessibleAction action(ACCESSIBILITY_ACTION_CLICK, "ace");
        nodeInfo.AddAction(action);
    }
    nodeInfo.SetLongClickable(gestureEventHub->IsAccessibilityLongClickable());
    if (gestureEventHub->IsAccessibilityLongClickable()) {
        AccessibleAction action(ACCESSIBILITY_ACTION_LONG_CLICK, "ace");
        nodeInfo.AddAction(action);
    }
}
}

void JsAccessibilityManager::UpdateAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& node, AccessibilityElementInfo& nodeInfo)
{
    CHECK_NULL_VOID(node);
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    if (accessibilityProperty->HasAccessibilityRole()) {
        nodeInfo.SetComponentType(accessibilityProperty->GetAccessibilityRole());
    }

    if (accessibilityProperty->HasUserTextValue()) {
        nodeInfo.SetContent(accessibilityProperty->GetUserTextValue());
    } else {
        nodeInfo.SetContent(accessibilityProperty->GetGroupText());
    }
    nodeInfo.SetAccessibilityText(accessibilityProperty->GetAccessibilityText());
    if (accessibilityProperty->HasRange()) {
        RangeInfo rangeInfo = ConvertAccessibilityValue(accessibilityProperty->GetAccessibilityValue());
        nodeInfo.SetRange(rangeInfo);
    }
    nodeInfo.SetHint(accessibilityProperty->GetHintText());
    nodeInfo.SetTextType(accessibilityProperty->GetTextType());
    nodeInfo.SetTextLengthLimit(accessibilityProperty->GetTextLengthLimit());
    nodeInfo.SetOffset(accessibilityProperty->GetScrollOffSet());
    if (accessibilityProperty->HasUserDisabled()) {
        nodeInfo.SetEnabled(!accessibilityProperty->IsUserDisabled());
    }
    if (accessibilityProperty->HasUserCheckedType()) {
        nodeInfo.SetChecked(accessibilityProperty->GetUserCheckedType());
    } else {
        nodeInfo.SetChecked(accessibilityProperty->IsChecked());
    }
    if (accessibilityProperty->HasUserSelected()) {
        nodeInfo.SetSelected(accessibilityProperty->IsUserSelected());
    } else {
        nodeInfo.SetSelected(accessibilityProperty->IsSelected());
    }
    nodeInfo.SetPassword(accessibilityProperty->IsPassword());
    nodeInfo.SetPluraLineSupported(accessibilityProperty->IsMultiLine());
    nodeInfo.SetHinting(accessibilityProperty->IsHint());
    nodeInfo.SetDescriptionInfo(accessibilityProperty->GetAccessibilityDescription());
    if (accessibilityProperty->HasUserCurrentValue()) {
        nodeInfo.SetCurrentIndex(accessibilityProperty->GetUserCurrentValue());
    } else {
        nodeInfo.SetCurrentIndex(accessibilityProperty->GetCurrentIndex());
    }
    if (accessibilityProperty->HasUserMinValue()) {
        nodeInfo.SetBeginIndex(accessibilityProperty->GetUserMinValue());
    } else {
        nodeInfo.SetBeginIndex(accessibilityProperty->GetBeginIndex());
    }
    if (accessibilityProperty->HasUserMaxValue()) {
        nodeInfo.SetEndIndex(accessibilityProperty->GetUserMaxValue());
    } else {
        nodeInfo.SetEndIndex(accessibilityProperty->GetEndIndex());
    }
    auto tag = node->GetTag();
    if (tag == V2::TOAST_ETS_TAG || tag == V2::POPUP_ETS_TAG || tag == V2::DIALOG_ETS_TAG ||
        tag == V2::ACTION_SHEET_DIALOG_ETS_TAG || tag == V2::ALERT_DIALOG_ETS_TAG || tag == V2::MENU_ETS_TAG ||
        tag == "SelectMenu") {
        nodeInfo.SetLiveRegion(1);
    }
    nodeInfo.SetContentInvalid(accessibilityProperty->GetContentInvalid());
    nodeInfo.SetError(accessibilityProperty->GetErrorText());
    nodeInfo.SetSelectedBegin(accessibilityProperty->GetTextSelectionStart());
    nodeInfo.SetSelectedEnd(accessibilityProperty->GetTextSelectionEnd());
    nodeInfo.SetInputType(static_cast<int>(accessibilityProperty->GetTextInputType()));
    nodeInfo.SetItemCounts(accessibilityProperty->GetCollectionItemCounts());
    nodeInfo.SetChildTreeIdAndWinId(
        accessibilityProperty->GetChildTreeId(), accessibilityProperty->GetChildWindowId());
    if (nodeInfo.GetComponentType() == "FormComponent") {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "elementId: %{public}" PRId64 ", Set: %{public}d %{public}d",
            nodeInfo.GetAccessibilityId(), accessibilityProperty->GetChildTreeId(),
            accessibilityProperty->GetChildWindowId());
    }
    nodeInfo.SetBelongTreeId(treeId_);
    nodeInfo.SetParentWindowId(parentWindowId_);

    GridInfo gridInfo(accessibilityProperty->GetCollectionInfo().rows,
        accessibilityProperty->GetCollectionInfo().columns, accessibilityProperty->GetCollectionInfo().selectMode);
    nodeInfo.SetGrid(gridInfo);

    int32_t row = accessibilityProperty->GetCollectionItemInfo().row;
    int32_t column = accessibilityProperty->GetCollectionItemInfo().column;
    int32_t rowSpan = accessibilityProperty->GetCollectionItemInfo().rowSpan;
    int32_t columnSpan = accessibilityProperty->GetCollectionItemInfo().columnSpan;
    bool heading = accessibilityProperty->GetCollectionItemInfo().heading;
    GridItemInfo gridItemInfo(row, rowSpan, column, columnSpan, heading, nodeInfo.IsSelected());
    nodeInfo.SetGridItem(gridItemInfo);

    SetAccessibilityFocusAction(nodeInfo, "ace");
    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetCheckable(accessibilityProperty->IsCheckable());
        nodeInfo.SetScrollable(accessibilityProperty->IsScrollable());
        nodeInfo.SetEditable(accessibilityProperty->IsEditable());
        nodeInfo.SetDeletable(accessibilityProperty->IsDeletable());
        UpdateSupportAction(node, nodeInfo);
        accessibilityProperty->ResetSupportAction();
        auto supportAceActions = accessibilityProperty->GetSupportAction();
        for (auto it = supportAceActions.begin(); it != supportAceActions.end(); ++it) {
            AccessibleAction action(ConvertAceAction(*it), "ace");
            nodeInfo.AddAction(action);
        }
    }
}

namespace {
#ifdef WEB_SUPPORTED
static void UpdateWebAccessibilityElementInfo(
    RefPtr<NG::WebAccessibilityNode> node, AccessibilityElementInfo& nodeInfo)
{
    nodeInfo.SetContent(node->GetContent());
    nodeInfo.SetAccessibilityText(node->GetContent());
    RangeInfo rangeInfo(static_cast<int32_t>(node->GetRangeInfoMin()), static_cast<int32_t>(node->GetRangeInfoMax()),
        static_cast<int32_t>(node->GetRangeInfoCurrent()));
    nodeInfo.SetRange(rangeInfo);
    nodeInfo.SetHint(node->GetHint());
    nodeInfo.SetHinting(node->GetIsHinting());
    nodeInfo.SetDescriptionInfo(node->GetDescriptionInfo());
    nodeInfo.SetChecked(node->GetIsChecked());
    nodeInfo.SetSelected(node->GetIsSelected());
    nodeInfo.SetPassword(node->GetIsPassword());
    nodeInfo.SetPluraLineSupported(node->GetIsPluralLineSupported());
    nodeInfo.SetLiveRegion(node->GetLiveRegion());
    nodeInfo.SetContentInvalid(node->GetIsContentInvalid());
    nodeInfo.SetError(node->GetError());
    nodeInfo.SetSelectedBegin(node->GetSelectionStart());
    nodeInfo.SetSelectedEnd(node->GetSelectionEnd());
    nodeInfo.SetInputType(node->GetInputType());
    nodeInfo.SetItemCounts(node->GetItemCounts());

    GridInfo gridInfo(node->GetGridRows(), node->GetGridColumns(), node->GetGridSelectedMode());
    nodeInfo.SetGrid(gridInfo);

    int32_t row = node->GetGridItemRow();
    int32_t column = node->GetGridItemColumn();
    int32_t rowSpan = node->GetGridItemRowSpan();
    int32_t columnSpan = node->GetGridItemColumnSpan();
    bool heading = node->GetIsHeading();
    GridItemInfo gridItemInfo(row, rowSpan, column, columnSpan, heading, nodeInfo.IsSelected());
    nodeInfo.SetGridItem(gridItemInfo);

    SetAccessibilityFocusAction(nodeInfo, "web");
    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetCheckable(node->GetIsCheckable());
        nodeInfo.SetScrollable(node->GetIsScrollable());
        nodeInfo.SetEditable(node->GetIsEditable());
        nodeInfo.SetDeletable(node->GetIsDeletable());
        nodeInfo.SetClickable(node->GetIsClickable());
        auto supportAceActions = node->GetActions();
        for (auto it = supportAceActions.begin(); it != supportAceActions.end(); ++it) {
            AccessibleAction action(ConvertAceAction(static_cast<AceAction>(*it)), "web");
            nodeInfo.AddAction(action);
        }
    }
}

void UpdateWebAccessibilityElementInfo(RefPtr<NG::WebAccessibilityNode> node,
    const CommonProperty& commonProperty, AccessibilityElementInfo& nodeInfo)
{
    nodeInfo.SetParent(node->GetParentId());
    for (const auto& child : node->GetChildIds()) {
        nodeInfo.AddChild(child);
    }

    nodeInfo.SetAccessibilityId(node->GetAccessibilityId());
    nodeInfo.SetComponentType(node->GetComponentType());
    nodeInfo.SetEnabled(node->GetIsEnabled());
    nodeInfo.SetFocused(node->GetIsFocused());
    nodeInfo.SetAccessibilityFocus(node->GetIsAccessibilityFocus());
    nodeInfo.SetVisible(node->GetIsVisible());
    const RefPtr<NG::FrameNode>& webNode = node->GetWebNode();
    if (node->GetIsVisible() && webNode) {
        auto webRect = webNode->GetTransformRectRelativeToWindow();
        auto left = webRect.Left() + node->GetRectX() + commonProperty.windowLeft;
        auto top = webRect.Top() + node->GetRectY() + commonProperty.windowTop;
        auto right = webRect.Left() + node->GetRectX() + node->GetRectWidth() + commonProperty.windowLeft;
        auto bottom = webRect.Top() + node->GetRectY() + node->GetRectHeight() + commonProperty.windowTop;
        Accessibility::Rect bounds { left, top, right, bottom };
        nodeInfo.SetRectInScreen(bounds);
    }

    nodeInfo.SetWindowId(commonProperty.windowId);
    nodeInfo.SetPageId(node->GetPageId());
    nodeInfo.SetPagePath(commonProperty.pagePath);
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetFocusable(node->GetIsFocusable());
        nodeInfo.SetPopupSupported(node->GetIsPopupSupported());
    }
    UpdateWebAccessibilityElementInfo(node, nodeInfo);
}
#endif

void UpdateChildrenOfAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty, AccessibilityElementInfo& nodeInfo)
{
    if (!IsExtensionComponent(node) || IsUIExtensionShowPlaceholder(node)) {
        std::vector<int64_t> children;
        for (const auto& item : node->GetChildren()) {
            GetFrameNodeChildren(item, children, commonProperty.pageId);
        }
#ifdef WEB_SUPPORTED
        if (node->GetTag() == V2::WEB_ETS_TAG && AceApplicationInfo::GetInstance().IsAccessibilityEnabled()) {
            auto webPattern = node->GetPattern<NG::WebPattern>();
            CHECK_NULL_VOID(webPattern);
            auto webAccessibilityNode = webPattern->GetAccessibilityNodeById(-1);
            CHECK_NULL_VOID(webAccessibilityNode);
            for (auto& childId : webAccessibilityNode->GetChildIds()) {
                children.emplace_back(childId);
            }
        }
#endif
        for (const auto& child : children) {
            nodeInfo.AddChild(child);
        }
    }
}
}

void JsAccessibilityManager::UpdateVirtualNodeChildAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
    AccessibilityElementInfo& nodeParentInfo, AccessibilityElementInfo& nodeInfo,
    const RefPtr<NG::PipelineContext>& ngPipeline)
{
    CHECK_NULL_VOID(node);
#ifdef WEB_SUPPORTED
    if (node->GetTag() == V2::WEB_CORE_TAG) {
        auto webAccessibilityNode = AceType::DynamicCast<NG::WebAccessibilityNode>(node);
        CHECK_NULL_VOID(webAccessibilityNode);
        UpdateWebAccessibilityElementInfo(webAccessibilityNode, commonProperty, nodeInfo);
        return;
    }
#endif
    nodeInfo.SetParent(GetParentId(node));
    UpdateChildrenOfAccessibilityElementInfo(node, commonProperty, nodeInfo);

    nodeInfo.SetAccessibilityId(node->GetAccessibilityId());
    nodeInfo.SetComponentType(node->GetTag());

    nodeInfo.SetEnabled(node->GetFocusHub() ? node->GetFocusHub()->IsEnabled() : true);
    nodeInfo.SetFocused(node->GetFocusHub() ? node->GetFocusHub()->IsCurrentFocus() : false);
    nodeInfo.SetAccessibilityFocus(node->GetRenderContext()->GetAccessibilityFocus().value_or(false));
    nodeInfo.SetInspectorKey(node->GetInspectorId().value_or(""));
    nodeInfo.SetVisible(node->IsVisible());
    if (node->IsVisible()) {
        auto virtualNodeRect = node->GetTransformRectRelativeToWindow();
        int32_t left = nodeParentInfo.GetRectInScreen().GetLeftTopXScreenPostion() + virtualNodeRect.GetX();
        int32_t top = nodeParentInfo.GetRectInScreen().GetLeftTopYScreenPostion() + virtualNodeRect.GetY();
        int32_t right = nodeParentInfo.GetRectInScreen().GetLeftTopXScreenPostion() + virtualNodeRect.GetX() +
            virtualNodeRect.Width();
        int32_t bottom = nodeParentInfo.GetRectInScreen().GetLeftTopYScreenPostion() + virtualNodeRect.GetY() +
            virtualNodeRect.Height();
        Accessibility::Rect bounds { left, top, right, bottom };
        nodeInfo.SetRectInScreen(bounds);
    }
    nodeInfo.SetWindowId(commonProperty.windowId);
    nodeInfo.SetPageId(node->GetPageId());
    nodeInfo.SetPagePath(commonProperty.pagePath);
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetFocusable(node->GetFocusHub() ? node->GetFocusHub()->IsFocusable() : false);
        nodeInfo.SetPopupSupported(IsPopupSupported(ngPipeline, node->GetId()));
    }
    nodeInfo.SetComponentResourceId(node->GetInspectorId().value_or(""));
    UpdateAccessibilityElementInfo(node, nodeInfo);
}

void JsAccessibilityManager::UpdateVirtualNodeAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& parent, const RefPtr<NG::FrameNode>& node,
    const CommonProperty& commonProperty, AccessibilityElementInfo& nodeInfo,
    const RefPtr<NG::PipelineContext>& ngPipeline)
{
    CHECK_NULL_VOID(parent);
    CHECK_NULL_VOID(node);
#ifdef WEB_SUPPORTED
    if (node->GetTag() == V2::WEB_CORE_TAG) {
        auto webAccessibilityNode = AceType::DynamicCast<NG::WebAccessibilityNode>(node);
        CHECK_NULL_VOID(webAccessibilityNode);
        UpdateWebAccessibilityElementInfo(webAccessibilityNode, commonProperty, nodeInfo);
        return;
    }
#endif
    nodeInfo.SetParent(GetParentId(node));
    UpdateChildrenOfAccessibilityElementInfo(node, commonProperty, nodeInfo);

    nodeInfo.SetAccessibilityId(node->GetAccessibilityId());
    nodeInfo.SetComponentType(node->GetTag());

    nodeInfo.SetEnabled(node->GetFocusHub() ? node->GetFocusHub()->IsEnabled() : true);
    nodeInfo.SetFocused(node->GetFocusHub() ? node->GetFocusHub()->IsCurrentFocus() : false);
    nodeInfo.SetAccessibilityFocus(node->GetRenderContext()->GetAccessibilityFocus().value_or(false));
    nodeInfo.SetInspectorKey(node->GetInspectorId().value_or(""));
    nodeInfo.SetVisible(node->IsVisible());
    if (node->IsVisible()) {
        auto virtualNodeRect = node->GetTransformRectRelativeToWindow();
        auto parentRect = parent->GetTransformRectRelativeToWindow();
        auto left = parentRect.Left() + commonProperty.windowLeft;
        auto top = parentRect.Top() + commonProperty.windowTop;
        auto right = parentRect.Left() + virtualNodeRect.Width() + commonProperty.windowLeft;
        if (virtualNodeRect.Width() > (parentRect.Right() - parentRect.Left())) {
            right = parentRect.Right() + commonProperty.windowLeft;
        }
        auto bottom = parentRect.Top() + virtualNodeRect.Height() + commonProperty.windowTop;
        if (virtualNodeRect.Height() > (parentRect.Bottom() - parentRect.Top())) {
            bottom = parentRect.Bottom() + commonProperty.windowTop;
        }
        Accessibility::Rect bounds { left, top, right, bottom };
        nodeInfo.SetRectInScreen(bounds);
    }
    nodeInfo.SetWindowId(commonProperty.windowId);
    nodeInfo.SetPageId(node->GetPageId());
    nodeInfo.SetPagePath(commonProperty.pagePath);
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetFocusable(node->GetFocusHub() ? node->GetFocusHub()->IsFocusable() : false);
        nodeInfo.SetPopupSupported(IsPopupSupported(ngPipeline, node->GetId()));
    }
    nodeInfo.SetComponentResourceId(node->GetInspectorId().value_or(""));
    UpdateAccessibilityElementInfo(node, nodeInfo);
}

void JsAccessibilityManager::UpdateAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
    AccessibilityElementInfo& nodeInfo, const RefPtr<NG::PipelineContext>& ngPipeline)
{
    CHECK_NULL_VOID(node);
#ifdef WEB_SUPPORTED
    if (node->GetTag() == V2::WEB_CORE_TAG) {
        auto webAccessibilityNode = AceType::DynamicCast<NG::WebAccessibilityNode>(node);
        CHECK_NULL_VOID(webAccessibilityNode);
        UpdateWebAccessibilityElementInfo(webAccessibilityNode, commonProperty, nodeInfo);
        return;
    }
#endif
    nodeInfo.SetParent(GetParentId(node));
    UpdateChildrenOfAccessibilityElementInfo(node, commonProperty, nodeInfo);

    nodeInfo.SetAccessibilityId(node->GetAccessibilityId());
    nodeInfo.SetComponentType(node->GetTag());

    nodeInfo.SetEnabled(node->GetFocusHub() ? node->GetFocusHub()->IsEnabled() : true);
    nodeInfo.SetFocused(node->GetFocusHub() ? node->GetFocusHub()->IsCurrentFocus() : false);
    nodeInfo.SetAccessibilityFocus(node->GetRenderContext()->GetAccessibilityFocus().value_or(false));
    nodeInfo.SetInspectorKey(node->GetInspectorId().value_or(""));
    nodeInfo.SetVisible(node->IsVisible());
    if (node->IsVisible()) {
        auto rect = node->GetTransformRectRelativeToWindow();
        auto left = rect.Left() + commonProperty.windowLeft;
        auto top = rect.Top() + commonProperty.windowTop;
        auto right = rect.Right() + commonProperty.windowLeft;
        auto bottom = rect.Bottom() + commonProperty.windowTop;
        Accessibility::Rect bounds { left, top, right, bottom };
        nodeInfo.SetRectInScreen(bounds);
    }
    nodeInfo.SetWindowId(commonProperty.windowId);
    nodeInfo.SetPageId(node->GetPageId());
    nodeInfo.SetPagePath(commonProperty.pagePath);
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetFocusable(node->GetFocusHub() ? node->GetFocusHub()->IsFocusable() : false);
        nodeInfo.SetPopupSupported(IsPopupSupported(ngPipeline, node->GetId()));
    }
    nodeInfo.SetComponentResourceId(node->GetInspectorId().value_or(""));
    UpdateAccessibilityElementInfo(node, nodeInfo);
}

namespace {
std::list<AccessibilityElementInfo> SearchExtensionElementInfoByAccessibilityIdNG(
    int64_t elementId, int32_t mode, const RefPtr<NG::FrameNode>& node,  int64_t offset)
{
    std::list<AccessibilityElementInfo> extensionElementInfo;
    if (NG::UI_EXTENSION_OFFSET_MIN < (offset + 1)) {
        node->SearchExtensionElementInfoByAccessibilityIdNG(elementId, mode,
            offset / NG::UI_EXTENSION_ID_FACTOR, extensionElementInfo);
    }
    return extensionElementInfo;
}

void UpdateUiExtensionParentIdForFocus(const RefPtr<NG::FrameNode>& rootNode, const int64_t uiExtensionOffset,
    Accessibility::AccessibilityElementInfo& info)
{
    if ((uiExtensionOffset != NG::UI_EXTENSION_OFFSET_MAX) && (info.GetComponentType() != V2::ROOT_ETS_TAG) &&
        (info.GetParentNodeId() == rootNode->GetAccessibilityId())) {
            info.SetParent(NG::UI_EXTENSION_ROOT_ID);
    }
}

void GetChildrenFromWebNode(
    const RefPtr<NG::FrameNode>& node, std::list<std::variant<RefPtr<NG::FrameNode>, int64_t>>& children,
    const RefPtr<NG::PipelineContext>& ngPipeline)
{
#ifdef WEB_SUPPORTED
    std::list<int64_t> webNodeChildren;
    if (AceApplicationInfo::GetInstance().IsAccessibilityEnabled()) {
        if (node->GetTag() == V2::WEB_ETS_TAG) {
            if (!ngPipeline->GetOnFoucs()) {
                return;
            }
            auto webPattern = node->GetPattern<NG::WebPattern>();
            CHECK_NULL_VOID(webPattern);
            auto webAccessibilityNode = webPattern->GetAccessibilityNodeById(-1);
            CHECK_NULL_VOID(webAccessibilityNode);
            for (auto& childId : webAccessibilityNode->GetChildIds()) {
                webNodeChildren.emplace_back(childId);
            }
        } else if (node->GetTag() == V2::WEB_CORE_TAG) {
            auto webAccessibilityNode = AceType::DynamicCast<NG::WebAccessibilityNode>(node);
            CHECK_NULL_VOID(webAccessibilityNode);
            for (auto& childId : webAccessibilityNode->GetChildIds()) {
                webNodeChildren.emplace_back(childId);
            }
        }
    }
    while (!webNodeChildren.empty()) {
        children.emplace_back(webNodeChildren.front());
        webNodeChildren.pop_front();
    }
#endif
}

void GetChildrenFromFrameNode(const RefPtr<NG::FrameNode>& node,
    std::list<std::variant<RefPtr<NG::FrameNode>, int64_t>>& children, int32_t pageId)
{
    std::list<RefPtr<NG::FrameNode>> frameNodeChildren;
    for (const auto& item : node->GetChildren()) {
        GetFrameNodeChildren(item, frameNodeChildren, pageId);
    }
    while (!frameNodeChildren.empty()) {
        children.emplace_back(frameNodeChildren.front());
        frameNodeChildren.pop_front();
    }
}
}

void JsAccessibilityManager::UpdateVirtualNodeInfo(std::list<AccessibilityElementInfo>& infos,
    AccessibilityElementInfo& nodeInfo, const RefPtr<NG::UINode>& uiVirtualNode, const CommonProperty& commonProperty,
    const RefPtr<NG::PipelineContext>& ngPipeline)
{
    AccessibilityElementInfo virtualInfo;
    for (const auto& item : uiVirtualNode->GetChildren()) {
        auto frameNodeChild = AceType::DynamicCast<NG::FrameNode>(item);
        if (frameNodeChild == nullptr) {
            continue;
        }
        UpdateVirtualNodeChildAccessibilityElementInfo(frameNodeChild, commonProperty,
            nodeInfo, virtualInfo, ngPipeline);
        virtualInfo.SetParent(uiVirtualNode->GetAccessibilityId());
        nodeInfo.AddChild(frameNodeChild->GetAccessibilityId());
        UpdateVirtualNodeInfo(infos, virtualInfo, item, commonProperty, ngPipeline);
        infos.push_back(virtualInfo);
    }
}

namespace {
void SearchExtensionElementInfoNG(const SearchParameter& searchParam,
    const RefPtr<NG::FrameNode>& node, std::list<Accessibility::AccessibilityElementInfo>& infos,
    Accessibility::AccessibilityElementInfo& parentInfo)
{
    auto extensionElementInfos = SearchExtensionElementInfoByAccessibilityIdNG(
        searchParam.nodeId, searchParam.mode, node, searchParam.uiExtensionOffset);
    if (extensionElementInfos.size() > 0) {
        auto rootParentId = extensionElementInfos.front().GetParentNodeId();
        ConvertExtensionAccessibilityNodeId(extensionElementInfos, node, searchParam.uiExtensionOffset, parentInfo);
        if (rootParentId == NG::UI_EXTENSION_ROOT_ID) {
            extensionElementInfos.front().SetParent(node->GetAccessibilityId());
        }
        if (parentInfo.GetComponentType() == V2::ISOLATED_COMPONENT_ETS_TAG) {
            auto windowId = parentInfo.GetWindowId();
            for (auto& info : extensionElementInfos) {
                info.SetWindowId(windowId);
            }
        }
        for (auto& info : extensionElementInfos) {
            infos.push_back(info);
        }
    }
}

bool IsNodeInRoot(const RefPtr<NG::FrameNode>& node, const RefPtr<NG::PipelineContext>& ngPipeline)
{
    CHECK_NULL_RETURN(node, false);
    CHECK_NULL_RETURN(ngPipeline, false);
    auto rect = node->GetTransformRectRelativeToWindow();
    auto root = ngPipeline->GetRootElement();
    CHECK_NULL_RETURN(root, false);
    auto rootRect = root->GetTransformRectRelativeToWindow();
    return LessNotEqual(rect.GetX(), rootRect.GetX() + rootRect.Width());
}

}

void JsAccessibilityManager::UpdateCacheInfoNG(std::list<AccessibilityElementInfo>& infos,
    const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
    const RefPtr<NG::PipelineContext>& ngPipeline, const SearchParameter& searchParam)
{
    uint32_t umode = searchParam.mode;
    std::list<std::variant<RefPtr<NG::FrameNode>, int64_t>> children;
    // get all children
    if (!(umode & static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN))) {
        return;
    }
    GetChildrenFromFrameNode(node, children, commonProperty.pageId);
    GetChildrenFromWebNode(node, children, ngPipeline);
    while (!children.empty()) {
        std::variant<RefPtr<NG::FrameNode>, int64_t> parent = children.front();
        children.pop_front();
        AccessibilityElementInfo nodeInfo;
        if (parent.index() == 0) {
            // Handle the parent when its type is FrameNode
            RefPtr<NG::FrameNode> frameNodeParent = std::get<0>(parent);
            auto accessibilityProperty = frameNodeParent->GetAccessibilityProperty<NG::AccessibilityProperty>();
            auto uiVirtualNode = accessibilityProperty->GetAccessibilityVirtualNode();
            UpdateAccessibilityElementInfo(frameNodeParent, commonProperty, nodeInfo, ngPipeline);
            if (nodeInfo.GetComponentType() == V2::WEB_ETS_TAG && !IsNodeInRoot(frameNodeParent, ngPipeline)) {
                continue;
            }
            if (uiVirtualNode != nullptr) {
                auto virtualNode = AceType::DynamicCast<NG::FrameNode>(uiVirtualNode);
                if (virtualNode == nullptr) {
                    continue;
                }
                AccessibilityElementInfo virtualInfo;
                UpdateVirtualNodeAccessibilityElementInfo(frameNodeParent, virtualNode,
                    commonProperty, virtualInfo, ngPipeline);
                virtualInfo.SetParent(frameNodeParent->GetAccessibilityId());
                auto childIds = nodeInfo.GetChildIds();
                for (auto& child : childIds) {
                    nodeInfo.RemoveChild(child);
                }
                nodeInfo.AddChild(virtualNode->GetAccessibilityId());
                auto uiParentNode = AceType::DynamicCast<NG::UINode>(frameNodeParent);
                if (!uiVirtualNode->GetChildren().empty()) {
                    UpdateVirtualNodeInfo(infos, virtualInfo, uiVirtualNode, commonProperty, ngPipeline);
                }
                infos.push_back(virtualInfo);
                infos.push_back(nodeInfo);
                continue;
            }
            if (!IsExtensionComponent(frameNodeParent) || IsUIExtensionShowPlaceholder(frameNodeParent)) {
                infos.push_back(nodeInfo);
                GetChildrenFromFrameNode(frameNodeParent, children, commonProperty.pageId);
                GetChildrenFromWebNode(frameNodeParent, children, ngPipeline);
                continue;
            }
            if (!((frameNodeParent->GetUiExtensionId() > NG::UI_EXTENSION_UNKNOW_ID) &&
                (((frameNodeParent->GetUiExtensionId() <= NG::UI_EXTENSION_ID_FIRST_MAX) &&
                (NG::UI_EXTENSION_OFFSET_MAX == searchParam.uiExtensionOffset)) ||
                (frameNodeParent->GetUiExtensionId() <= NG::UI_EXTENSION_ID_OTHER_MAX)))) {
                continue;
            }
            auto transferParam = searchParam;
            transferParam.nodeId = NG::UI_EXTENSION_ROOT_ID;
            SearchExtensionElementInfoNG(transferParam, frameNodeParent, infos, nodeInfo);
        } else if (parent.index() == 1) {
            // Handle the parent when its type is WebNode
            int64_t intParent = std::get<1>(parent);
            auto frameNode = GetFramenodeByAccessibilityId(node, intParent);
            GetChildrenFromWebNode(frameNode, children, ngPipeline);
            UpdateAccessibilityElementInfo(frameNode, commonProperty, nodeInfo, ngPipeline);
        }
        infos.push_back(nodeInfo);
    }
}

namespace {
bool CanAccessibilityFocusedNG(const RefPtr<NG::FrameNode>& node)
{
    CHECK_NULL_RETURN(node, false);
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, false);
    auto level = accessibilityProperty->GetAccessibilityLevel();
    return !node->IsRootNode() &&
           node->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE) == VisibleType::VISIBLE &&
           level != NG::AccessibilityProperty::Level::NO &&
           level != NG::AccessibilityProperty::Level::NO_HIDE_DESCENDANTS;
}
// focus move search
void AddFocusableNode(std::list<RefPtr<NG::FrameNode>>& nodeList, const RefPtr<NG::FrameNode>& node)
{
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    auto level = accessibilityProperty->GetAccessibilityLevel();
    if (CanAccessibilityFocusedNG(node)) {
        nodeList.emplace_back(node);
    }

    if (!accessibilityProperty->IsAccessibilityGroup() &&
        level != NG::AccessibilityProperty::Level::NO_HIDE_DESCENDANTS) {
        std::list<RefPtr<NG::FrameNode>> children;
        for (const auto& child : node->GetChildren()) {
            GetFrameNodeChildren(child, children);
        }

        for (const auto& child : children) {
            AddFocusableNode(nodeList, child);
        }
    }
}

// execute action
bool RequestFocus(RefPtr<NG::FrameNode>& frameNode)
{
    auto focusHub = frameNode->GetFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    return focusHub->RequestFocusImmediately();
}

bool LostFocus(const RefPtr<NG::FrameNode>& frameNode)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto focusHub = frameNode->GetFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    focusHub->LostFocus();
    return true;
}

bool ActClick(RefPtr<NG::FrameNode>& frameNode)
{
    auto gesture = frameNode->GetEventHub<NG::EventHub>()->GetGestureEventHub();
    CHECK_NULL_RETURN(gesture, false);
    bool result = gesture->ActClick();
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, result);
    auto accessibilityAction = ACTIONS.find(ACCESSIBILITY_ACTION_CLICK);
    if (accessibilityAction == ACTIONS.end()) {
        return result;
    }
    AccessibilityActionParam param;
    param.accessibilityProperty = accessibilityProperty;
    result |= accessibilityAction->second(param);
    return result;
}

bool ActLongClick(RefPtr<NG::FrameNode>& frameNode)
{
    auto gesture = frameNode->GetEventHub<NG::EventHub>()->GetGestureEventHub();
    CHECK_NULL_RETURN(gesture, false);
    return gesture->ActLongClick();
}

void ClearAccessibilityFocus(const RefPtr<NG::FrameNode>& root, int64_t focusNodeId)
{
    auto oldFocusNode = GetFramenodeByAccessibilityId(root, focusNodeId);
    CHECK_NULL_VOID(oldFocusNode);
    if (oldFocusNode->GetTag() != V2::WEB_CORE_TAG) {
        oldFocusNode->GetRenderContext()->UpdateAccessibilityFocus(false);
    }
}

bool ActAccessibilityFocus(int64_t elementId, RefPtr<NG::FrameNode>& frameNode, RefPtr<NG::PipelineContext>& context,
    int64_t& currentFocusNodeId, bool isNeedClear)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto renderContext = frameNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, false);
    if (isNeedClear) {
        if (elementId != currentFocusNodeId) {
            return false;
        }
        renderContext->UpdateAccessibilityFocus(false);
        currentFocusNodeId = -1;
        return true;
    }
    if (elementId == currentFocusNodeId) {
        return false;
    }
    Framework::ClearAccessibilityFocus(context->GetRootElement(), currentFocusNodeId);
    renderContext->UpdateAccessibilityFocus(true);
    currentFocusNodeId = frameNode->GetAccessibilityId();
    return true;
}

inline string GetSupportAction(const std::unordered_set<AceAction>& supportAceActions)
{
    std::string actionForDump;
    for (const auto& action : supportAceActions) {
        if (!actionForDump.empty()) {
            actionForDump.append(",");
        }
        actionForDump.append(std::to_string(static_cast<int32_t>(action)));
    }
    return actionForDump;
}

static std::string ConvertActionTypeToString(ActionType action)
{
    switch (action) {
        case ActionType::ACCESSIBILITY_ACTION_FOCUS:
            return "ACCESSIBILITY_ACTION_FOCUS";
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS:
            return "ACCESSIBILITY_ACTION_CLEAR_FOCUS";
        case ActionType::ACCESSIBILITY_ACTION_SELECT:
            return "ACCESSIBILITY_ACTION_SELECT";
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION:
            return "ACCESSIBILITY_ACTION_CLEAR_SELECTION";
        case ActionType::ACCESSIBILITY_ACTION_CLICK:
            return "ACCESSIBILITY_ACTION_CLICK";
        case ActionType::ACCESSIBILITY_ACTION_LONG_CLICK:
            return "ACCESSIBILITY_ACTION_LONG_CLICK";
        case ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS:
            return "ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS";
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS:
            return "ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS";
        case ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD:
            return "ACCESSIBILITY_ACTION_SCROLL_FORWARD";
        case ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD:
            return "ACCESSIBILITY_ACTION_SCROLL_BACKWARD";
        case ActionType::ACCESSIBILITY_ACTION_COPY:
            return "ACCESSIBILITY_ACTION_COPY";
        case ActionType::ACCESSIBILITY_ACTION_PASTE:
            return "ACCESSIBILITY_ACTION_PASTE";
        case ActionType::ACCESSIBILITY_ACTION_CUT:
            return "ACCESSIBILITY_ACTION_CUT";
        case ActionType::ACCESSIBILITY_ACTION_SET_SELECTION:
            return "ACCESSIBILITY_ACTION_SET_SELECTION";
        case ActionType::ACCESSIBILITY_ACTION_SET_TEXT:
            return "ACCESSIBILITY_ACTION_SET_TEXT";
        case ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT:
            return "ACCESSIBILITY_ACTION_NEXT_TEXT";
        case ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT:
            return "ACCESSIBILITY_ACTION_PREVIOUS_TEXT";
        case ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION:
            return "ACCESSIBILITY_ACTION_SET_CURSOR_POSITION";
        default:
            return "ACCESSIBILITY_ACTION_INVALID";
    }
}

static AceAction ConvertAccessibilityAction(ActionType accessibilityAction)
{
    static const ActionTable actionTable[] = {
        { AceAction::ACTION_CLICK, ActionType::ACCESSIBILITY_ACTION_CLICK },
        { AceAction::ACTION_LONG_CLICK, ActionType::ACCESSIBILITY_ACTION_LONG_CLICK },
        { AceAction::ACTION_SCROLL_FORWARD, ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD },
        { AceAction::ACTION_SCROLL_BACKWARD, ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD },
        { AceAction::ACTION_FOCUS, ActionType::ACCESSIBILITY_ACTION_FOCUS },
        { AceAction::ACTION_CLEAR_FOCUS, ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS },
        { AceAction::ACTION_ACCESSIBILITY_FOCUS, ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS },
        { AceAction::ACTION_CLEAR_ACCESSIBILITY_FOCUS, ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS },
        { AceAction::ACTION_NEXT_AT_MOVEMENT_GRANULARITY, ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT },
        { AceAction::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY, ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT },
        { AceAction::ACTION_SET_TEXT, ActionType::ACCESSIBILITY_ACTION_SET_TEXT },
        { AceAction::ACTION_COPY, ActionType::ACCESSIBILITY_ACTION_COPY },
        { AceAction::ACTION_PASTE, ActionType::ACCESSIBILITY_ACTION_PASTE },
        { AceAction::ACTION_CUT, ActionType::ACCESSIBILITY_ACTION_CUT },
        { AceAction::ACTION_SELECT, ActionType::ACCESSIBILITY_ACTION_SELECT },
        { AceAction::ACTION_CLEAR_SELECTION, ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION },
        { AceAction::ACTION_SET_SELECTION, ActionType::ACCESSIBILITY_ACTION_SET_SELECTION },
        { AceAction::ACTION_SET_CURSOR_POSITION, ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION },
    };
    for (const auto& item : actionTable) {
        if (accessibilityAction == item.action) {
            return item.aceAction;
        }
    }
    return AceAction::ACTION_NONE;
}

static void DumpSupportActionNG(const AccessibilityElementInfo& nodeInfo)
{
    DumpLog::GetInstance().AddDesc(
        "support action instructions: use command to make application components perform corresponding action");
    DumpLog::GetInstance().AddDesc(
        "use support action command: aa dump -i [AbilityRecord] -c -inspector [AccessibilityId] [AceAction]");
    std::string actionForDump;
    for (const auto& action : nodeInfo.GetActionList()) {
        if (!actionForDump.empty()) {
            actionForDump.append(",");
        }
        actionForDump.append(ConvertActionTypeToString(action.GetActionType()));
        actionForDump.append(": ");
        actionForDump.append(std::to_string(static_cast<int32_t>(ConvertAccessibilityAction(action.GetActionType()))));
    }
    DumpLog::GetInstance().AddDesc("support action: ", actionForDump);
}

inline void DumpContentListNG(const AccessibilityElementInfo& nodeInfo)
{
    std::vector<std::string> contentList;
    nodeInfo.GetContentList(contentList);
    std::string contents;
    for (auto content : contentList) {
        if (!contents.empty()) {
            contents.append(",");
        }
        contents.append(content);
    }
    DumpLog::GetInstance().AddDesc("content list: ", contents);
}

static void DumpAccessibilityPropertyNG(const AccessibilityElementInfo& nodeInfo)
{
    DumpLog::GetInstance().AddDesc("checked: ", BoolToString(nodeInfo.IsChecked()));
    DumpLog::GetInstance().AddDesc("selected: ", BoolToString(nodeInfo.IsSelected()));
    DumpLog::GetInstance().AddDesc("checkable: ", BoolToString(nodeInfo.IsCheckable()));
    DumpLog::GetInstance().AddDesc("scrollable: ", BoolToString(nodeInfo.IsScrollable()));
    DumpLog::GetInstance().AddDesc("accessibility hint: ", BoolToString(nodeInfo.IsGivingHint()));
    DumpLog::GetInstance().AddDesc("hint text: ", nodeInfo.GetHint());
    DumpLog::GetInstance().AddDesc("error text: ", nodeInfo.GetError());
    DumpLog::GetInstance().AddDesc("max text length: ", nodeInfo.GetTextLengthLimit());
    DumpLog::GetInstance().AddDesc("scroll offset: ", nodeInfo.GetOffset());
    DumpLog::GetInstance().AddDesc("text selection start: ", nodeInfo.GetSelectedBegin());
    DumpLog::GetInstance().AddDesc("text selection end: ", nodeInfo.GetSelectedEnd());
    DumpLog::GetInstance().AddDesc("is multi line: ", BoolToString(nodeInfo.IsPluraLineSupported()));
    DumpLog::GetInstance().AddDesc("is password: ", BoolToString(nodeInfo.IsPassword()));
    DumpLog::GetInstance().AddDesc(
        "text input type: ", ConvertInputTypeToString(static_cast<AceTextCategory>(nodeInfo.GetInputType())));

    DumpLog::GetInstance().AddDesc("min value: ", nodeInfo.GetRange().GetMin());
    DumpLog::GetInstance().AddDesc("max value: ", nodeInfo.GetRange().GetMax());
    DumpLog::GetInstance().AddDesc("current value: ", nodeInfo.GetRange().GetCurrent());
    DumpLog::GetInstance().AddDesc("gird info rows: ", nodeInfo.GetGrid().GetRowCount());
    DumpLog::GetInstance().AddDesc("gird info columns: ", nodeInfo.GetGrid().GetColumnCount());
    DumpLog::GetInstance().AddDesc("gird info select mode: ", nodeInfo.GetGrid().GetSelectionMode());
    DumpLog::GetInstance().AddDesc("gird item info, row: ", nodeInfo.GetGridItem().GetRowIndex());
    DumpLog::GetInstance().AddDesc("gird item info, column: ", nodeInfo.GetGridItem().GetColumnIndex());
    DumpLog::GetInstance().AddDesc("gird item info, rowSpan: ", nodeInfo.GetGridItem().GetRowSpan());
    DumpLog::GetInstance().AddDesc("gird item info, columnSpan: ", nodeInfo.GetGridItem().GetColumnSpan());
    DumpLog::GetInstance().AddDesc("gird item info, is heading: ", nodeInfo.GetGridItem().IsHeading());
    DumpLog::GetInstance().AddDesc("gird item info, selected: ", nodeInfo.GetGridItem().IsSelected());
    DumpLog::GetInstance().AddDesc("current index: ", nodeInfo.GetCurrentIndex());
    DumpLog::GetInstance().AddDesc("begin index: ", nodeInfo.GetBeginIndex());
    DumpLog::GetInstance().AddDesc("end index: ", nodeInfo.GetEndIndex());
    DumpLog::GetInstance().AddDesc("collection item counts: ", nodeInfo.GetItemCounts());
    DumpLog::GetInstance().AddDesc("editable: ", BoolToString(nodeInfo.IsEditable()));
    DumpLog::GetInstance().AddDesc("is essential: ", BoolToString(nodeInfo.IsEssential()));
    DumpLog::GetInstance().AddDesc("deletable: ", nodeInfo.IsDeletable());
    DumpLog::GetInstance().AddDesc("live region: ", nodeInfo.GetLiveRegion());
    DumpLog::GetInstance().AddDesc("content description: ", nodeInfo.GetDescriptionInfo());
    DumpLog::GetInstance().AddDesc("content invalid: ", BoolToString(nodeInfo.GetContentInvalid()));
    DumpLog::GetInstance().AddDesc("accessibility label: ", nodeInfo.GetLabeledAccessibilityId());
    DumpLog::GetInstance().AddDesc(
        "trigger action: ", static_cast<int32_t>(ConvertAccessibilityAction(nodeInfo.GetTriggerAction())));
    DumpLog::GetInstance().AddDesc("text move step: " + std::to_string(nodeInfo.GetTextMovementStep()));
    DumpSupportActionNG(nodeInfo);
    DumpContentListNG(nodeInfo);
    DumpLog::GetInstance().AddDesc("latest content: ", nodeInfo.GetLatestContent());
}

inline string ChildernToString(const vector<int64_t>& children, int32_t treeId)
{
    std::string ids;
    for (auto child : children) {
        if (!ids.empty()) {
            ids.append(",");
        }
        int64_t childId = child;
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId, childId);
        ids.append(std::to_string(childId));
    }
    return ids;
}

inline void DumpRectNG(const Accessibility::Rect& rect)
{
    DumpLog::GetInstance().AddDesc(
        "width: ", std::to_string(rect.GetRightBottomXScreenPostion() - rect.GetLeftTopXScreenPostion()));
    DumpLog::GetInstance().AddDesc(
        "height: ", std::to_string(rect.GetRightBottomYScreenPostion() - rect.GetLeftTopYScreenPostion()));
    DumpLog::GetInstance().AddDesc("left: ", std::to_string(rect.GetLeftTopXScreenPostion()));
    DumpLog::GetInstance().AddDesc("top: ", std::to_string(rect.GetLeftTopYScreenPostion()));
    DumpLog::GetInstance().AddDesc("right: ", std::to_string(rect.GetRightBottomXScreenPostion()));
    DumpLog::GetInstance().AddDesc("bottom: ", std::to_string(rect.GetRightBottomYScreenPostion()));
}

static void DumpCommonPropertyNG(const AccessibilityElementInfo& nodeInfo, int32_t treeId)
{
    int64_t elementId = nodeInfo.GetAccessibilityId();
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId, elementId);
    DumpLog::GetInstance().AddDesc("ID: ", elementId);
    int64_t parentId = nodeInfo.GetParentNodeId();
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId, parentId);
    DumpLog::GetInstance().AddDesc("parent ID: ", parentId);
    DumpLog::GetInstance().AddDesc("child IDs: ", ChildernToString(nodeInfo.GetChildIds(), treeId));
    DumpLog::GetInstance().AddDesc("component type: ", nodeInfo.GetComponentType());
    DumpLog::GetInstance().AddDesc("text: ", nodeInfo.GetContent());
    DumpLog::GetInstance().AddDesc("window id: " + std::to_string(nodeInfo.GetWindowId()));
    DumpRectNG(nodeInfo.GetRectInScreen());

    DumpLog::GetInstance().AddDesc("enabled: ", BoolToString(nodeInfo.IsEnabled()));
    DumpLog::GetInstance().AddDesc("focusable: ", BoolToString(nodeInfo.IsFocusable()));
    DumpLog::GetInstance().AddDesc("focused: ", BoolToString(nodeInfo.IsFocused()));
    DumpLog::GetInstance().AddDesc("visible: ", BoolToString(nodeInfo.IsVisible()));
    DumpLog::GetInstance().AddDesc("accessibility focused: ", BoolToString(nodeInfo.HasAccessibilityFocus()));
    DumpLog::GetInstance().AddDesc("inspector key: ", nodeInfo.GetInspectorKey());
    DumpLog::GetInstance().AddDesc("bundle name: ", nodeInfo.GetBundleName());
    DumpLog::GetInstance().AddDesc("page id: " + std::to_string(nodeInfo.GetPageId()));
    DumpLog::GetInstance().AddDesc("page path: ", nodeInfo.GetPagePath());
    DumpLog::GetInstance().AddDesc("is valid element: ", BoolToString(nodeInfo.IsValidElement()));
    DumpLog::GetInstance().AddDesc("resource name: ", nodeInfo.GetComponentResourceId());

    DumpLog::GetInstance().AddDesc("clickable: ", BoolToString(nodeInfo.IsClickable()));
    DumpLog::GetInstance().AddDesc("long clickable: ", BoolToString(nodeInfo.IsLongClickable()));
    DumpLog::GetInstance().AddDesc("popup supported: ", BoolToString(nodeInfo.IsPopupSupported()));
}

bool IsExtensionSendAccessibilitySyncEvent(RefPtr<NG::PipelineContext>& pipeline)
{
    CHECK_NULL_RETURN(pipeline, false);
    auto container = Platform::AceContainer::GetContainer(pipeline->GetInstanceId());
    CHECK_NULL_RETURN(container, false);
    return container->IsUIExtensionWindow();
}

void GenerateAccessibilityEventInfo(const AccessibilityEvent& accessibilityEvent, AccessibilityEventInfo& eventInfo)
{
    Accessibility::EventType type = Accessibility::EventType::TYPE_VIEW_INVALID;
    if (accessibilityEvent.type != AccessibilityEventType::UNKNOWN) {
        type = ConvertAceEventType(accessibilityEvent.type);
    } else {
        type = ConvertStrToEventType(accessibilityEvent.eventType);
    }

    if (type == Accessibility::EventType::TYPE_VIEW_INVALID) {
        return;
    }

    eventInfo.SetTimeStamp(GetMicroTickCount());
    eventInfo.SetBeforeText(accessibilityEvent.beforeText);
    eventInfo.SetLatestContent(accessibilityEvent.latestContent);
    eventInfo.SetWindowChangeTypes(static_cast<Accessibility::WindowUpdateType>(accessibilityEvent.windowChangeTypes));
    eventInfo.SetWindowContentChangeTypes(
        static_cast<Accessibility::WindowsContentChangeTypes>(accessibilityEvent.windowContentChangeTypes));
    eventInfo.SetSource(accessibilityEvent.nodeId);
    eventInfo.SetEventType(type);
    eventInfo.SetCurrentIndex(static_cast<int>(accessibilityEvent.currentItemIndex));
    eventInfo.SetItemCounts(static_cast<int>(accessibilityEvent.itemCount));
    eventInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());
}

} // namespace

JsAccessibilityManager::~JsAccessibilityManager()
{
    auto eventType = AccessibilityStateEventType::EVENT_ACCESSIBILITY_STATE_CHANGED;

    UnsubscribeStateObserver(eventType);
    UnsubscribeToastObserver();

    DeregisterInteractionOperation();
}
void JsAccessibilityManager::ToastAccessibilityConfigObserver::OnConfigChanged(
    const AccessibilityConfig::CONFIG_ID id, const AccessibilityConfig::ConfigValue& value)
{
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "accessibility content timeout changed:%{public}u", value.contentTimeout);
    AceApplicationInfo::GetInstance().SetBarrierfreeDuration((int32_t)value.contentTimeout);
}

bool JsAccessibilityManager::SubscribeToastObserver()
{
    if (!toastObserver_) {
        toastObserver_ = std::make_shared<ToastAccessibilityConfigObserver>();
    }
    CHECK_NULL_RETURN(toastObserver_, false);
    auto& config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
    bool isSuccess = config.InitializeContext();
    if (!isSuccess) {
        return false;
    }
    config.SubscribeConfigObserver(CONFIG_CONTENT_TIMEOUT, toastObserver_);
    return true;
}

bool JsAccessibilityManager::UnsubscribeToastObserver()
{
    CHECK_NULL_RETURN(toastObserver_, false);
    auto& config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
    bool isSuccess = config.InitializeContext();
    if (!isSuccess) {
        return false;
    }
    config.UnsubscribeConfigObserver(CONFIG_CONTENT_TIMEOUT, toastObserver_);
    return true;
}

bool JsAccessibilityManager::SubscribeStateObserver(int eventType)
{
    if (!stateObserver_) {
        stateObserver_ = std::make_shared<JsAccessibilityStateObserver>();
    }

    stateObserver_->SetAccessibilityManager(WeakClaim(this));
    stateObserver_->SetPipeline(context_);

    auto instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(instance, false);
    Accessibility::RetError ret = instance->SubscribeStateObserver(stateObserver_, eventType);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, " the result of SubscribeStateObserver:%{public}d", ret);
    return ret == RET_OK;
}

bool JsAccessibilityManager::UnsubscribeStateObserver(int eventType)
{
    CHECK_NULL_RETURN(stateObserver_, false);
    std::shared_ptr<AccessibilitySystemAbilityClient> instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(instance, false);
    Accessibility::RetError ret = instance->UnsubscribeStateObserver(stateObserver_, eventType);
    return ret == RET_OK;
}

void JsAccessibilityManager::InitializeCallback()
{
    if (IsRegister()) {
        return;
    }

    auto pipelineContext = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(pipelineContext);
    windowId_ = pipelineContext->GetWindowId();

    auto client = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_VOID(client);
    bool isEnabled = false;
    client->IsEnabled(isEnabled);
    AceApplicationInfo::GetInstance().SetAccessibilityEnabled(isEnabled);

    SubscribeToastObserver();

    if (pipelineContext->IsFormRender() || pipelineContext->IsJsCard() || pipelineContext->IsJsPlugin()) {
        return;
    }
    SubscribeStateObserver(AccessibilityStateEventType::EVENT_ACCESSIBILITY_STATE_CHANGED);
    if (isEnabled) {
        RegisterInteractionOperation(windowId_);
    }
}

bool JsAccessibilityManager::SendExtensionAccessibilitySyncEvent(
    const AccessibilityEvent& accessibilityEvent, const Accessibility::AccessibilityEventInfo& eventInfo)
{
#ifdef WINDOW_SCENE_SUPPORTED
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context_.Upgrade());
    CHECK_NULL_RETURN(ngPipeline, false);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_RETURN(uiExtensionManager, false);
    return uiExtensionManager->SendAccessibilityEventInfo(eventInfo, NG::UI_EXTENSION_UNKNOW_ID, ngPipeline);
#endif
    return false;
}

bool JsAccessibilityManager::SendAccessibilitySyncEvent(
    const AccessibilityEvent& accessibilityEvent, AccessibilityEventInfo eventInfo)
{
    if (!IsRegister()) {
        return false;
    }
    auto client = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(client, false);
    bool isEnabled = false;
    client->IsEnabled(isEnabled);
    if (!isEnabled) {
        return false;
    }
    int64_t elementId = eventInfo.GetAccessibilityId();
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId_, elementId);
    eventInfo.SetSource(elementId);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "send accessibility event:%{public}d accessibilityId:%{public}" PRId64,
        eventInfo.GetEventType(), eventInfo.GetAccessibilityId());
    return client->SendEvent(eventInfo);
}

bool JsAccessibilityManager::TransferAccessibilityAsyncEvent(
    const AccessibilityEventInfo& eventInfo, int64_t uiExtensionOffset)
{
#ifdef WINDOW_SCENE_SUPPORTED
    auto client = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(client, false);
    bool isEnabled = false;
    client->IsEnabled(isEnabled);
    if (!isEnabled) {
        return false;
    }

    auto pipeline = context_.Upgrade();
    CHECK_NULL_RETURN(pipeline, false);
    RefPtr<NG::PipelineContext> ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    CHECK_NULL_RETURN(ngPipeline, false);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_RETURN(uiExtensionManager, false);
    auto container = Container::GetContainer(ngPipeline->GetInstanceId());
    if (container && container->IsUIExtensionWindow()) {
        return uiExtensionManager->SendAccessibilityEventInfo(eventInfo,
            uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR, pipeline);
    }
    if (!IsRegister() && !(container && container->IsDynamicRender())) {
        return false;
    }
    AccessibilityEventInfo eventInfoNew = eventInfo;
    eventInfoNew.SetSource(uiExtensionOffset + eventInfo.GetViewId());
    AccessibilityElementInfo elementInfo;
    FillElementInfo(eventInfoNew.GetAccessibilityId(), elementInfo, pipeline, Claim(this));
    eventInfoNew.SetElementInfo(elementInfo);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "send accessibility event:%{public}d accessibilityId:%{public}" PRId64,
        eventInfoNew.GetEventType(), eventInfoNew.GetAccessibilityId());
    return client->SendEvent(eventInfoNew);
#endif
    return false;
}

void JsAccessibilityManager::SendExtensionAccessibilityEvent(
    const AccessibilityEventInfo& eventInfo, int64_t uiExtensionOffset)
{
    TransferAccessibilityAsyncEvent(eventInfo, uiExtensionOffset);
}

void JsAccessibilityManager::FillEventInfoWithNode(
    const RefPtr<NG::FrameNode>& node,
    AccessibilityEventInfo& eventInfo,
    const RefPtr<NG::PipelineContext>& context,
    int64_t elementId)
{
    CHECK_NULL_VOID(node);
    if (node->GetTag() == V2::WEB_CORE_TAG) {
        FillEventInfo(node, eventInfo, context, elementId, Claim(this));
        return;
    }
    eventInfo.SetComponentType(node->GetTag());
    eventInfo.SetPageId(node->GetPageId());
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    eventInfo.AddContent(accessibilityProperty->GetGroupText());
    eventInfo.SetItemCounts(accessibilityProperty->GetCollectionItemCounts());
    eventInfo.SetBeginIndex(accessibilityProperty->GetBeginIndex());
    eventInfo.SetEndIndex(accessibilityProperty->GetEndIndex());
    AccessibilityElementInfo elementInfo;

    CommonProperty commonProperty;
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    GenerateCommonProperty(context, commonProperty, mainContext);
    UpdateAccessibilityElementInfo(node, commonProperty, elementInfo, context);
    eventInfo.SetElementInfo(elementInfo);
}

void JsAccessibilityManager::SendEventToAccessibilityWithNode(
    const AccessibilityEvent& accessibilityEvent, const RefPtr<AceType>& node, const RefPtr<PipelineBase>& context)
{
    ACE_ACCESS_SCOPED_TRACE("SendAccessibilityAsyncEvent");
    CHECK_NULL_VOID(node);
    CHECK_NULL_VOID(context);
    int32_t windowId = static_cast<int32_t>(context->GetFocusWindowId());
    if (windowId == 0) {
        return;
    }
    if (!AceType::InstanceOf<NG::FrameNode>(node)) {
        return;
    }
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    CHECK_NULL_VOID(frameNode);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);

    if ((!frameNode->IsActive()) || frameNode->CheckAccessibilityLevelNo()) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "node: %{public}" PRId64 ", is not active or level is no",
            frameNode->GetAccessibilityId());
        return;
    }

    AccessibilityEventInfo eventInfo;
    FillEventInfoWithNode(frameNode, eventInfo, ngPipeline, accessibilityEvent.nodeId);

    if (accessibilityEvent.type != AccessibilityEventType::PAGE_CHANGE || accessibilityEvent.windowId == 0) {
        eventInfo.SetWindowId(windowId);
    } else {
        eventInfo.SetWindowId(accessibilityEvent.windowId);
    }
    GenerateAccessibilityEventInfo(accessibilityEvent, eventInfo);

    auto container = Container::GetContainer(context->GetInstanceId());
    if (IsExtensionSendAccessibilitySyncEvent(ngPipeline)) {
        SendExtensionAccessibilitySyncEvent(accessibilityEvent, eventInfo);
    } else if (container && container->IsDynamicRender()) {
        SendExtensionAccessibilityEvent(eventInfo, NG::UI_EXTENSION_UNKNOW_ID);
    } else {
        context->GetTaskExecutor()->PostTask(
            [weak = WeakClaim(this), accessibilityEvent, eventInfo] {
                auto jsAccessibilityManager = weak.Upgrade();
                CHECK_NULL_VOID(jsAccessibilityManager);
                jsAccessibilityManager->SendAccessibilitySyncEvent(accessibilityEvent, eventInfo);
            },
            TaskExecutor::TaskType::BACKGROUND, "ArkUIAccessibilitySendSyncEvent");
    }
}

void JsAccessibilityManager::SendAccessibilityAsyncEvent(const AccessibilityEvent& accessibilityEvent)
{
    ACE_ACCESS_SCOPED_TRACE("SendAccessibilityAsyncEvent");
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    int32_t windowId = static_cast<int32_t>(context->GetFocusWindowId());
    if (windowId == 0) {
        return;
    }

    AccessibilityEventInfo eventInfo;
    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        RefPtr<NG::FrameNode> node;
        auto ngPipeline = FindPipelineByElementId(accessibilityEvent.nodeId, node);
        CHECK_NULL_VOID(ngPipeline);
        CHECK_NULL_VOID(node);
        FillEventInfo(node, eventInfo, ngPipeline, accessibilityEvent.nodeId, Claim(this));
        eventInfo.SetWindowId(ngPipeline->GetFocusWindowId());
    } else {
        auto node = GetAccessibilityNodeFromPage(accessibilityEvent.nodeId);
        CHECK_NULL_VOID(node);
        FillEventInfo(node, eventInfo);
        eventInfo.SetWindowId(windowId);
    }
    if (accessibilityEvent.type == AccessibilityEventType::PAGE_CHANGE && accessibilityEvent.windowId != 0) {
        eventInfo.SetWindowId(accessibilityEvent.windowId);
    }

    GenerateAccessibilityEventInfo(accessibilityEvent, eventInfo);

    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    auto container = Container::GetContainer(context->GetInstanceId());
    if (IsExtensionSendAccessibilitySyncEvent(ngPipeline)) {
        SendExtensionAccessibilitySyncEvent(accessibilityEvent, eventInfo);
    } else if (container && container->IsDynamicRender()) {
        SendExtensionAccessibilityEvent(eventInfo, NG::UI_EXTENSION_UNKNOW_ID);
    } else {
        context->GetTaskExecutor()->PostTask(
            [weak = WeakClaim(this), accessibilityEvent, eventInfo] {
                auto jsAccessibilityManager = weak.Upgrade();
                CHECK_NULL_VOID(jsAccessibilityManager);
                jsAccessibilityManager->SendAccessibilitySyncEvent(accessibilityEvent, eventInfo);
            },
            TaskExecutor::TaskType::BACKGROUND, "ArkUIAccessibilitySendSyncEvent");
    }
}

void JsAccessibilityManager::UpdateNodeChildIds(const RefPtr<AccessibilityNode>& node)
{
    CHECK_NULL_VOID(node);
    node->ActionUpdateIds();
    const auto& children = node->GetChildList();
    std::vector<int32_t> childrenVec;
    auto cardId = GetCardId();
    auto rootNodeId = GetRootNodeId();

    // get last stack children to barrier free service.
    if ((node->GetNodeId() == GetRootNodeId() + ROOT_STACK_BASE) && !children.empty() && !IsDeclarative()) {
        auto lastChildNodeId = children.back()->GetNodeId();
        if (isOhosHostCard()) {
            childrenVec.emplace_back(ConvertToCardAccessibilityId(lastChildNodeId, cardId, rootNodeId));
        } else {
            childrenVec.emplace_back(lastChildNodeId);
            for (const auto& child : children) {
                if (child->GetNodeId() == ROOT_DECOR_BASE - 1) {
                    childrenVec.emplace_back(child->GetNodeId());
                    break;
                }
            }
        }
    } else {
        childrenVec.resize(children.size());
        if (isOhosHostCard()) {
            std::transform(children.begin(), children.end(), childrenVec.begin(),
                [cardId, rootNodeId](const RefPtr<AccessibilityNode>& child) {
                    return ConvertToCardAccessibilityId(child->GetNodeId(), cardId, rootNodeId);
                });
        } else {
            std::transform(children.begin(), children.end(), childrenVec.begin(),
                [](const RefPtr<AccessibilityNode>& child) { return child->GetNodeId(); });
        }
    }
    node->SetChildIds(childrenVec);
}

void JsAccessibilityManager::ProcessParameters(
    ActionType op, const std::vector<std::string>& params, std::map<std::string, std::string>& paramsMap)
{
    if (op == ActionType::ACCESSIBILITY_ACTION_SET_TEXT) {
        if (params.size() == EVENT_DUMP_PARAM_LENGTH_UPPER) {
            paramsMap = { { ACTION_ARGU_SET_TEXT, params[EVENT_DUMP_ACTION_PARAM_INDEX] } };
        }
    }

    if (op == ActionType::ACCESSIBILITY_ACTION_SET_SELECTION) {
        paramsMap[ACTION_ARGU_SELECT_TEXT_START] = "-1";
        paramsMap[ACTION_ARGU_SELECT_TEXT_END] = "-1";
        paramsMap[ACTION_ARGU_SELECT_TEXT_INFORWARD] = STRING_DIR_BACKWARD;
        if (params.size() > EVENT_DUMP_PARAM_LENGTH_LOWER) {
            paramsMap[ACTION_ARGU_SELECT_TEXT_START] = params[EVENT_DUMP_ACTION_PARAM_INDEX];
        }
        if (params.size() > EVENT_DUMP_PARAM_LENGTH_LOWER + 1) {
            paramsMap[ACTION_ARGU_SELECT_TEXT_END] = params[EVENT_DUMP_ACTION_PARAM_INDEX + 1];
        }
        // 2 means params number Offset
        if (params.size() > EVENT_DUMP_PARAM_LENGTH_LOWER + 2) {
            // 2 means params number Offset
            paramsMap[ACTION_ARGU_SELECT_TEXT_INFORWARD] = params[EVENT_DUMP_ACTION_PARAM_INDEX + 2];
        }
    }

    if (op == ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT || op == ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT) {
        if (params.size() == EVENT_DUMP_PARAM_LENGTH_UPPER) {
            paramsMap[ACTION_ARGU_MOVE_UNIT] = std::to_string(TextMoveUnit::STEP_CHARACTER);
        }
        paramsMap[ACTION_ARGU_MOVE_UNIT] = std::to_string(TextMoveUnit::STEP_CHARACTER);
    }

    if (op == ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION) {
        paramsMap[ACTION_ARGU_SET_OFFSET] = params[EVENT_DUMP_ACTION_PARAM_INDEX];
    }
}

bool TransferExecuteAction(int64_t elementId, const RefPtr<NG::FrameNode>& node,
    const std::map<std::string, std::string>& actionArguments,
    ActionType action, int64_t uiExtensionOffset)
{
    bool isExecuted = false;
    if ((uiExtensionOffset + 1) > NG::UI_EXTENSION_OFFSET_MIN) {
        isExecuted = node->TransferExecuteAction(
            elementId, actionArguments, static_cast<int>(action),
            uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR);
    }
    return isExecuted;
}

RefPtr<NG::PipelineContext> JsAccessibilityManager::GetPipelineByWindowId(uint32_t windowId)
{
    auto mainPipeline = AceType::DynamicCast<NG::PipelineContext>(context_.Upgrade());
    if (mainPipeline!= nullptr && mainPipeline->GetWindowId() == windowId) {
        return mainPipeline;
    }
    for (auto subPipelineWeak : GetSubPipelineContexts()) {
        auto subContextNG = AceType::DynamicCast<NG::PipelineContext>(subPipelineWeak.Upgrade());
        if (subContextNG!= nullptr && subContextNG->GetWindowId() == windowId) {
            return subContextNG;
        }
    }
    return nullptr;
}

void JsAccessibilityManager::DumpTreeNG(bool useWindowId, uint32_t windowId, int64_t rootId)
{
    if (!useWindowId && rootId == -1) {
        // used to adapt old function
        DumpTree(0, 0);
        return;
    }

    auto pipeline = GetPipelineByWindowId(windowId);
    if (pipeline == nullptr) {
        DumpLog::GetInstance().Print("Error: pipeline is not found!");
        return;
    }
    auto rootNode = pipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    CommonProperty commonProperty;
    auto mainPipeline = context_.Upgrade();
    CHECK_NULL_VOID(mainPipeline);
    GenerateCommonProperty(pipeline, commonProperty, mainPipeline);
    auto nodeId = rootId == -1 ? rootNode->GetAccessibilityId() : rootId;
    DumpTreeNG(rootNode, 0, nodeId, commonProperty);
}

void JsAccessibilityManager::DumpHoverTestNG(uint32_t windowId, int64_t rootId, int32_t x, int32_t y, bool verbose)
{
    auto pipeline = GetPipelineByWindowId(windowId);
    CHECK_NULL_VOID(pipeline);
    auto accessibilityManagerNG = pipeline->GetAccessibilityManagerNG();
    CHECK_NULL_VOID(accessibilityManagerNG);
    auto pipelineRoot = pipeline->GetRootElement();
    RefPtr<NG::FrameNode> root = nullptr;
    if (rootId == -1) {
        root = pipelineRoot;
    } else {
        root = GetFramenodeByAccessibilityId(pipelineRoot, rootId);
    }
    CHECK_NULL_VOID(root);

    DumpLog::GetInstance().Print("Window ID: " + std::to_string(windowId));
    DumpLog::GetInstance().Print("Root ID: " + std::to_string(root->GetAccessibilityId()));
    NG::PointF hoverPoint(x, y);
    DumpLog::GetInstance().Print("Hover Point: " + hoverPoint.ToString());

    std::string summary;
    std::string detail;
    accessibilityManagerNG->HoverTestDebug(root, hoverPoint, summary, detail);
    DumpLog::GetInstance().Print(summary);
    if (verbose) {
        DumpLog::GetInstance().Print(detail);
    }
}

void JsAccessibilityManager::OnDumpInfoNG(const std::vector<std::string>& params, uint32_t windowId)
{
    if (params.size() < 1 || (params[0] != "-inspector" && params[0] != "-accessibility")) {
        DumpLog::GetInstance().Print("Error: invalid arguments!");
        return;
    }
    enum class DumpMode {
        TREE,
        NODE,
        HANDLE_EVENT,
        HOVER_TEST
    };
    bool useWindowId = false;
    DumpMode mode = DumpMode::TREE;
    bool verbose = false;
    int64_t rootId = -1;
    int32_t pointX = 0;
    int32_t pointY = 0;
    int64_t nodeId = -1;
    int32_t action = 0;
    for (auto arg = params.begin() + 1; arg != params.end(); ++arg) {
        if (*arg == "-w") {
            useWindowId = true;
        } else if (*arg == "--root") {
            ++arg;
            if (arg == params.end()) {
                DumpLog::GetInstance().Print(std::string("Error: --root is used to set the root node, ") +
                    "e.g. '--root ${AccessibilityId}'!");
                return;
            }
            rootId = StringUtils::StringToLongInt(*arg);
        } else if (*arg == "--hover-test") {
            mode = DumpMode::HOVER_TEST;
            static constexpr int32_t NUM_POINT_DIMENSION = 2;
            if (std::distance(arg, params.end()) <= NUM_POINT_DIMENSION) {
                DumpLog::GetInstance().Print(std::string("Error: --hover-test is used to get nodes at a point ") +
                    "relative to the root node, e.g. '--hover-test ${x} ${y}'!");
                return;
            }
            ++arg;
            pointX = StringUtils::StringToInt(*arg);
            ++arg;
            pointY = StringUtils::StringToInt(*arg);
        } else if (*arg == "-v") {
            verbose = true;
        } else {
            if (mode == DumpMode::NODE) {
                mode = DumpMode::HANDLE_EVENT;
                action = StringUtils::StringToInt(*arg);
                break;
            } else {
                mode = DumpMode::NODE;
                nodeId = StringUtils::StringToLongInt(*arg);
            }
        }
    }
    std::vector<std::string> info;
    bool isChildElement = CheckIsChildElement(nodeId, params, info);
    if (isChildElement) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "dump child element: %{public}" PRId64, nodeId);
        return;
    }
    switch (mode) {
        case DumpMode::TREE:
            DumpTreeNG(useWindowId, windowId, rootId);
            break;
        case DumpMode::NODE:
            DumpPropertyNG(nodeId);
            break;
        case DumpMode::HANDLE_EVENT:
            DumpHandleEvent(params);
            break;
        case DumpMode::HOVER_TEST:
            DumpHoverTestNG(windowId, rootId, pointX, pointY, verbose);
            break;
        default:
            DumpLog::GetInstance().Print("Error: invalid arguments!");
            break;
    }
}

void JsAccessibilityManager::DumpHandleEvent(const std::vector<std::string>& params)
{
    if (params.size() > EVENT_DUMP_PARAM_LENGTH_UPPER + 1) {
        return DumpLog::GetInstance().Print("Error: params length is illegal!");
    }
    if (params[EVENT_DUMP_ORDER_INDEX] != DUMP_ORDER && params[EVENT_DUMP_ORDER_INDEX] != DUMP_INSPECTOR) {
        return DumpLog::GetInstance().Print("Error: not accessibility dump order!");
    }
    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    int64_t nodeId = StringUtils::StringToLongInt(params[EVENT_DUMP_ID_INDEX]);

    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(nodeId, splitElementId, splitTreeId);
    nodeId = splitElementId;

    auto action = static_cast<AceAction>(StringUtils::StringToInt(params[EVENT_DUMP_ACTION_INDEX]));
    auto op = ConvertAceAction(action);
    if ((op != ActionType::ACCESSIBILITY_ACTION_SET_SELECTION) && (params.size() > EVENT_DUMP_PARAM_LENGTH_UPPER + 1)) {
        return DumpLog::GetInstance().Print("Error: params is illegal!");
    }
    std::map<std::string, std::string> paramsMap;
    ProcessParameters(op, params, paramsMap);
    if (AceType::InstanceOf<NG::PipelineContext>(pipeline)) {
        RefPtr<NG::FrameNode> node;
#ifdef WINDOW_SCENE_SUPPORTED
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
        auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
        CHECK_NULL_VOID(uiExtensionManager);
        if (uiExtensionManager->IsWrapExtensionAbilityId(nodeId)) {
            ExecuteActionNG(nodeId, paramsMap, op, ngPipeline, NG::UI_EXTENSION_OFFSET_MAX);
            return;
        }
#endif
        pipeline = FindPipelineByElementId(nodeId, node);
        CHECK_NULL_VOID(pipeline);
        pipeline->GetTaskExecutor()->PostTask(
            [weak = WeakClaim(this), op, nodeId, paramsMap, pipeline]() {
                auto jsAccessibilityManager = weak.Upgrade();
                CHECK_NULL_VOID(jsAccessibilityManager);
                jsAccessibilityManager->ExecuteActionNG(nodeId, paramsMap, op, pipeline, NG::UI_EXTENSION_OFFSET_MAX);
            },
            TaskExecutor::TaskType::UI, "ArkUIAccessibilityExecuteAction");
        return;
    }
    auto node = GetAccessibilityNodeFromPage(nodeId);
    CHECK_NULL_VOID(node);
    pipeline->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), op, node, paramsMap, pipeline]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            jsAccessibilityManager->AccessibilityActionEvent(
                op, paramsMap, node, AceType::DynamicCast<PipelineContext>(pipeline));
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityActionEvent");
}

void JsAccessibilityManager::DumpProperty(const RefPtr<AccessibilityNode>& node)
{
    const auto& supportAceActions = node->GetSupportAction();
    const auto& charValue = node->GetChartValue();

    DumpLog::GetInstance().AddDesc("ID: ", node->GetNodeId());
    DumpLog::GetInstance().AddDesc("parent ID: ", node->GetParentId());
    DumpLog::GetInstance().AddDesc("child IDs: ", GetNodeChildIds(node));
    DumpLog::GetInstance().AddDesc("component type: ", node->GetTag());
    DumpLog::GetInstance().AddDesc("input type: ", node->GetInputType());
    DumpLog::GetInstance().AddDesc("text: ", node->GetText());
    DumpLog::GetInstance().AddDesc("width: ", node->GetWidth());
    DumpLog::GetInstance().AddDesc("height: ", node->GetHeight());
    DumpLog::GetInstance().AddDesc("left: ", node->GetLeft() + GetCardOffset().GetX());
    DumpLog::GetInstance().AddDesc("top: ", node->GetTop() + GetCardOffset().GetY());
    DumpLog::GetInstance().AddDesc("enabled: ", BoolToString(node->GetEnabledState()));
    DumpLog::GetInstance().AddDesc("checked: ", BoolToString(node->GetCheckedState()));
    DumpLog::GetInstance().AddDesc("selected: ", BoolToString(node->GetSelectedState()));
    DumpLog::GetInstance().AddDesc("focusable: ", BoolToString(node->GetFocusableState()));
    DumpLog::GetInstance().AddDesc("focused: ", BoolToString(node->GetFocusedState()));
    DumpLog::GetInstance().AddDesc("checkable: ", BoolToString(node->GetCheckableState()));
    DumpLog::GetInstance().AddDesc("clickable: ", BoolToString(node->GetClickableState()));
    DumpLog::GetInstance().AddDesc("long clickable: ", BoolToString(node->GetLongClickableState()));
    DumpLog::GetInstance().AddDesc("scrollable: ", BoolToString(node->GetScrollableState()));
    DumpLog::GetInstance().AddDesc("editable: ", BoolToString(node->GetEditable()));
    DumpLog::GetInstance().AddDesc("hint text: ", node->GetHintText());
    DumpLog::GetInstance().AddDesc("error text: ", node->GetErrorText());
    DumpLog::GetInstance().AddDesc("js component id: ", node->GetJsComponentId());
    DumpLog::GetInstance().AddDesc("accessibility label: ", node->GetAccessibilityLabel());
    DumpLog::GetInstance().AddDesc("accessibility hint: ", node->GetAccessibilityHint());
    DumpLog::GetInstance().AddDesc("max text length: ", node->GetMaxTextLength());
    DumpLog::GetInstance().AddDesc("text selection start: ", node->GetTextSelectionStart());
    DumpLog::GetInstance().AddDesc("text selection end: ", node->GetTextSelectionEnd());
    DumpLog::GetInstance().AddDesc("is multi line: ", BoolToString(node->GetIsMultiLine()));
    DumpLog::GetInstance().AddDesc("is password", BoolToString(node->GetIsPassword()));
    DumpLog::GetInstance().AddDesc("text input type: ", ConvertInputTypeToString(node->GetTextInputType()));
    DumpLog::GetInstance().AddDesc("min value: ", node->GetAccessibilityValue().min);
    DumpLog::GetInstance().AddDesc("max value: ", node->GetAccessibilityValue().max);
    DumpLog::GetInstance().AddDesc("current value: ", node->GetAccessibilityValue().current);
    DumpLog::GetInstance().AddDesc("collection info rows: ", node->GetCollectionInfo().rows);
    DumpLog::GetInstance().AddDesc("collection info columns: ", node->GetCollectionInfo().columns);
    DumpLog::GetInstance().AddDesc("collection item info, row: ", node->GetCollectionItemInfo().row);
    DumpLog::GetInstance().AddDesc("collection item info, column: ", node->GetCollectionItemInfo().column);
    DumpLog::GetInstance().AddDesc("chart has value: ", BoolToString(charValue && !charValue->empty()));
    DumpLog::GetInstance().AddDesc("accessibilityGroup: ", BoolToString(node->GetAccessible()));
    DumpLog::GetInstance().AddDesc("accessibilityImportance: ", node->GetImportantForAccessibility());
    DumpLog::GetInstance().AddDesc("support action: ", GetSupportAction(supportAceActions));
    DumpLog::GetInstance().Print(0, node->GetTag(), node->GetChildList().size());
}

void JsAccessibilityManager::DumpPropertyNG(int64_t nodeID)
{
    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    RefPtr<NG::PipelineContext> ngPipeline;

#ifdef WINDOW_SCENE_SUPPORTED
    ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    std::list<AccessibilityElementInfo> extensionElementInfos;
    if (uiExtensionManager->IsWrapExtensionAbilityId(nodeID)) {
        SearchElementInfoByAccessibilityIdNG(
            nodeID, PREFETCH_RECURSIVE_CHILDREN, extensionElementInfos, ngPipeline, NG::UI_EXTENSION_OFFSET_MAX);
        for (auto& extensionElementInfo : extensionElementInfos) {
            if (nodeID == extensionElementInfo.GetAccessibilityId()) {
                DumpCommonPropertyNG(extensionElementInfo, treeId_);
                DumpAccessibilityPropertyNG(extensionElementInfo);
                DumpLog::GetInstance().Print(
                    0, extensionElementInfo.GetComponentType(), extensionElementInfo.GetChildCount());
                return;
            }
        }
    }
#endif

    RefPtr<NG::FrameNode> frameNode;
    ngPipeline = FindPipelineByElementId(nodeID, frameNode);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(frameNode);

    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, pipeline);
    AccessibilityElementInfo nodeInfo;
    UpdateAccessibilityElementInfo(frameNode, commonProperty, nodeInfo, ngPipeline);
    if (IsExtensionComponent(frameNode) && !IsUIExtensionShowPlaceholder(frameNode)) {
        SearchParameter param {-1, "", PREFETCH_RECURSIVE_CHILDREN, NG::UI_EXTENSION_OFFSET_MAX};
        std::list<AccessibilityElementInfo> extensionElementInfos;
        SearchExtensionElementInfoNG(param, frameNode, extensionElementInfos, nodeInfo);
    }
    DumpCommonPropertyNG(nodeInfo, treeId_);
    DumpAccessibilityPropertyNG(nodeInfo);
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    if (accessibilityProperty) {
        DumpLog::GetInstance().AddDesc("offset: ", accessibilityProperty->GetScrollOffSet());
    }
    DumpLog::GetInstance().Print(0, nodeInfo.GetComponentType(), nodeInfo.GetChildCount());
}

void JsAccessibilityManager::DumpProperty(const std::vector<std::string>& params)
{
    CHECK_NULL_VOID(DumpLog::GetInstance().GetDumpFile());
    if (params.empty()) {
        DumpLog::GetInstance().Print("Error: params cannot be empty!");
        return;
    }
    if (params.size() != PROPERTY_DUMP_PARAM_LENGTH) {
        DumpLog::GetInstance().Print("Error: params length is illegal!");
        return;
    }
    if (params[0] != DUMP_ORDER && params[0] != DUMP_INSPECTOR) {
        DumpLog::GetInstance().Print("Error: not accessibility dump order!");
        return;
    }

    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);

    if (!AceType::InstanceOf<NG::PipelineContext>(pipeline)) {
        auto node = GetAccessibilityNodeFromPage(StringUtils::StringToLongInt(params[1]));
        if (!node) {
            DumpLog::GetInstance().Print("Error: can't find node with ID " + params[1]);
            return;
        }
        DumpProperty(node);
    } else {
        DumpPropertyNG(StringUtils::StringToLongInt(params[1]));
    }
}

static void DumpAccessibilityElementInfosTreeNG(
    std::list<AccessibilityElementInfo>& infos, int32_t depth, int64_t accessibilityId, bool isRoot)
{
    AccessibilityElementInfo accessibilityInfo;
    for (auto& info : infos) {
        if (accessibilityId == info.GetAccessibilityId()) {
            accessibilityInfo = info;
            break;
        }
    }
    if (!isRoot) {
        DumpLog::GetInstance().AddDesc("ID: " + std::to_string(accessibilityInfo.GetAccessibilityId()));
        DumpLog::GetInstance().AddDesc("compid: " + accessibilityInfo.GetInspectorKey());
        DumpLog::GetInstance().AddDesc("text: " + accessibilityInfo.GetContent());
        DumpLog::GetInstance().AddDesc("accessibilityText: " + accessibilityInfo.GetContent());
        DumpLog::GetInstance().AddDesc("accessibilityGroup: ");
        DumpLog::GetInstance().AddDesc("accessibilityLevel: ");
        DumpLog::GetInstance().AddDesc("top: " +
            std::to_string(accessibilityInfo.GetRectInScreen().GetLeftTopYScreenPostion()));
        DumpLog::GetInstance().AddDesc("left: " +
            std::to_string(accessibilityInfo.GetRectInScreen().GetLeftTopXScreenPostion()));
        DumpLog::GetInstance().AddDesc("width: " + std::to_string(
            accessibilityInfo.GetRectInScreen().GetRightBottomXScreenPostion() -
            accessibilityInfo.GetRectInScreen().GetLeftTopXScreenPostion()));
        DumpLog::GetInstance().AddDesc("height: " + std::to_string(
            accessibilityInfo.GetRectInScreen().GetRightBottomYScreenPostion() -
            accessibilityInfo.GetRectInScreen().GetLeftTopYScreenPostion()));
        DumpLog::GetInstance().AddDesc("visible: " + std::to_string(accessibilityInfo.IsVisible()));
        DumpLog::GetInstance().AddDesc(
            "clickable: " + std::to_string(accessibilityInfo.IsClickable()));
        DumpLog::GetInstance().AddDesc("longclickable: " +
            std::to_string(accessibilityInfo.IsLongClickable()));
        DumpLog::GetInstance().AddDesc("checkable: " + std::to_string(accessibilityInfo.IsCheckable()));
        DumpLog::GetInstance().AddDesc("scrollable: " + std::to_string(accessibilityInfo.IsScrollable()));
        DumpLog::GetInstance().AddDesc("checked: " + std::to_string(accessibilityInfo.IsCheckable()));
        DumpLog::GetInstance().AddDesc("hint: " + accessibilityInfo.GetHint());
        DumpLog::GetInstance().Print(depth, accessibilityInfo.GetComponentType(), accessibilityInfo.GetChildCount());
        depth ++;
    }
    for (auto child : accessibilityInfo.GetChildIds()) {
        DumpAccessibilityElementInfosTreeNG(infos, depth, child, false);
    }
}

static void DumpTreeNodeInfoNG(
    const RefPtr<NG::FrameNode>& node, int32_t depth, const CommonProperty& commonProperty, int32_t childSize)
{
    NG::RectF rect = node->GetTransformRectRelativeToWindow();
    DumpLog::GetInstance().AddDesc("ID: " + std::to_string(node->GetAccessibilityId()));
    DumpLog::GetInstance().AddDesc("compid: " + node->GetInspectorId().value_or(""));
    DumpLog::GetInstance().AddDesc("text: " +
        node->GetAccessibilityProperty<NG::AccessibilityProperty>()->GetGroupText());
    DumpLog::GetInstance().AddDesc(
        "accessibilityText: " + node->GetAccessibilityProperty<NG::AccessibilityProperty>()->GetAccessibilityText());
    DumpLog::GetInstance().AddDesc("accessibilityGroup: " +
        std::to_string(node->GetAccessibilityProperty<NG::AccessibilityProperty>()->IsAccessibilityGroup()));
    DumpLog::GetInstance().AddDesc(
        "accessibilityLevel: " + node->GetAccessibilityProperty<NG::AccessibilityProperty>()->GetAccessibilityLevel());
    DumpLog::GetInstance().AddDesc("top: " + std::to_string(rect.Top() + commonProperty.windowTop));
    DumpLog::GetInstance().AddDesc("left: " + std::to_string(rect.Left() + commonProperty.windowLeft));
    DumpLog::GetInstance().AddDesc("width: " + std::to_string(rect.Width()));
    DumpLog::GetInstance().AddDesc("height: " + std::to_string(rect.Height()));
    DumpLog::GetInstance().AddDesc("visible: " + std::to_string(node->IsVisible()));
    auto eventHub = node->GetEventHub<NG::EventHub>();
    if (eventHub) {
        auto gestureEventHub = eventHub->GetGestureEventHub();
        DumpLog::GetInstance().AddDesc(
            "clickable: " + std::to_string(gestureEventHub ? gestureEventHub->IsAccessibilityClickable() : false));
        DumpLog::GetInstance().AddDesc("longclickable: " +
            std::to_string(gestureEventHub ? gestureEventHub->IsAccessibilityLongClickable() : false));
    }
    DumpLog::GetInstance().AddDesc(
        "checkable: " + std::to_string(node->GetAccessibilityProperty<NG::AccessibilityProperty>()->IsCheckable()));
    DumpLog::GetInstance().AddDesc(
        "scrollable: " + std::to_string(node->GetAccessibilityProperty<NG::AccessibilityProperty>()->IsScrollable()));
    DumpLog::GetInstance().AddDesc(
        "checked: " + std::to_string(node->GetAccessibilityProperty<NG::AccessibilityProperty>()->IsChecked()));
    DumpLog::GetInstance().AddDesc(
        "hint: " + node->GetAccessibilityProperty<NG::AccessibilityProperty>()->GetHintText());
    DumpLog::GetInstance().AddDesc(
        "childTree: " + std::to_string(node->GetAccessibilityProperty<NG::AccessibilityProperty>()->GetChildTreeId()));
    DumpLog::GetInstance().Print(depth, node->GetTag(), childSize);
}

void JsAccessibilityManager::DumpTreeNG(const RefPtr<NG::FrameNode>& parent, int32_t depth,
    int64_t nodeID, const CommonProperty& commonProperty)
{
    auto node = GetFramenodeByAccessibilityId(parent, nodeID);
    if (!node) {
        DumpLog::GetInstance().Print("Error: failed to get accessibility node with ID " + std::to_string(nodeID));
        return;
    }
    if (!node->IsActive()) {
        return;
    }
    std::vector<int64_t> children;
    for (const auto& item : node->GetChildren()) {
        GetFrameNodeChildren(item, children, commonProperty.pageId);
    }
    DumpTreeNodeInfoNG(node, depth, commonProperty, children.size());
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    auto uiVirtualNode = accessibilityProperty->GetAccessibilityVirtualNode();
    bool hasVirtualNode = false;
    if (uiVirtualNode != nullptr) {
        auto virtualNode = AceType::DynamicCast<NG::FrameNode>(uiVirtualNode);
        CHECK_NULL_VOID(virtualNode);
        hasVirtualNode = true;
        DumpTreeNodeInfoNG(virtualNode, depth + 1, commonProperty, children.size());
        for (const auto& item : uiVirtualNode->GetChildren()) {
            auto frameChild = AceType::DynamicCast<NG::FrameNode>(item);
            DumpTreeNodeInfoNG(frameChild, depth + 1, commonProperty, children.size());
        }
    }
    if (IsExtensionComponent(node) && !IsUIExtensionShowPlaceholder(node)) {
        std::list<AccessibilityElementInfo> extensionElementInfos;
        auto pipeline = context_.Upgrade();
        CHECK_NULL_VOID(pipeline);
        SearchElementInfoByAccessibilityIdNG(
            node->GetAccessibilityId(), PREFETCH_RECURSIVE_CHILDREN, extensionElementInfos,
            pipeline, NG::UI_EXTENSION_OFFSET_MAX);
        if (!extensionElementInfos.empty()) {
            DumpAccessibilityElementInfosTreeNG(extensionElementInfos, depth + 1, node->GetAccessibilityId(), true);
        }
    }
    if (!hasVirtualNode) {
        for (auto childId : children) {
            DumpTreeNG(node, depth + 1, childId, commonProperty);
        }
    }
}

void JsAccessibilityManager::DumpTree(int32_t depth, int64_t nodeID)
{
    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    if (!AceType::InstanceOf<NG::PipelineContext>(pipeline)) {
        AccessibilityNodeManager::DumpTree(depth, nodeID);
    } else {
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
        auto rootNode = ngPipeline->GetRootElement();
        CHECK_NULL_VOID(rootNode);
        nodeID = rootNode->GetAccessibilityId();
        CommonProperty commonProperty;
        GenerateCommonProperty(ngPipeline, commonProperty, pipeline);
        DumpTreeNG(rootNode, depth, nodeID, commonProperty);
        for (auto subContext : GetSubPipelineContexts()) {
            auto subPipeline = subContext.Upgrade();
            ngPipeline = AceType::DynamicCast<NG::PipelineContext>(subPipeline);
            CHECK_NULL_VOID(ngPipeline);
            rootNode = ngPipeline->GetRootElement();
            CHECK_NULL_VOID(rootNode);
            nodeID = rootNode->GetAccessibilityId();
            commonProperty.windowId = ngPipeline->GetWindowId();
            commonProperty.windowLeft = GetWindowLeft(ngPipeline->GetWindowId());
            commonProperty.windowTop = GetWindowTop(ngPipeline->GetWindowId());
            commonProperty.pageId = 0;
            commonProperty.pagePath = "";
            DumpTreeNG(rootNode, depth, nodeID, commonProperty);
        }
    }
}

void JsAccessibilityManager::SetCardViewParams(const std::string& key, bool focus)
{
    callbackKey_ = key;
    if (!callbackKey_.empty()) {
        InitializeCallback();
    }
}

void JsAccessibilityManager::UpdateViewScale()
{
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    float scaleX = 1.0;
    float scaleY = 1.0;
    if (context->GetViewScale(scaleX, scaleY)) {
        scaleX_ = scaleX;
        scaleY_ = scaleY;
    }
}

void JsAccessibilityManager::HandleComponentPostBinding()
{
    for (auto targetIter = nodeWithTargetMap_.begin(); targetIter != nodeWithTargetMap_.end();) {
        auto nodeWithTarget = targetIter->second.Upgrade();
        if (nodeWithTarget) {
            if (nodeWithTarget->GetTag() == ACCESSIBILITY_TAG_POPUP) {
                auto idNodeIter = nodeWithIdMap_.find(targetIter->first);
                if (idNodeIter != nodeWithIdMap_.end()) {
                    auto nodeWithId = idNodeIter->second.Upgrade();
                    if (nodeWithId) {
                        nodeWithId->SetAccessibilityHint(nodeWithTarget->GetText());
                    } else {
                        nodeWithIdMap_.erase(idNodeIter);
                    }
                }
            }
            ++targetIter;
        } else {
            // clear the disabled node in the maps
            nodeWithTargetMap_.erase(targetIter++);
        }
    }

    // clear the disabled node in the maps
    for (auto idItem = nodeWithIdMap_.begin(); idItem != nodeWithIdMap_.end();) {
        if (!idItem->second.Upgrade()) {
            nodeWithIdMap_.erase(idItem++);
        } else {
            ++idItem;
        }
    }
}

RefPtr<AccessibilityNodeManager> AccessibilityNodeManager::Create()
{
    return AceType::MakeRefPtr<JsAccessibilityManager>();
}

RefPtr<PipelineBase> JsAccessibilityManager::GetPipelineByWindowId(const int32_t windowId)
{
    auto context = context_.Upgrade();
    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        CHECK_NULL_RETURN(context, nullptr);
        if (context->GetWindowId() == static_cast<uint32_t>(windowId)) {
            return context;
        }
        for (auto& subContext : GetSubPipelineContexts()) {
            context = subContext.Upgrade();
            CHECK_NULL_RETURN(context, nullptr);
            if (context->GetWindowId() == static_cast<uint32_t>(windowId)) {
                return context;
            }
        }
        return nullptr;
    } else {
        return context;
    }
}

void JsAccessibilityManager::JsInteractionOperation::SearchElementInfoByAccessibilityId(const int64_t elementId,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback, const int32_t mode)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto windowId = windowId_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, requestId, &callback, mode, windowId]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            ACE_SCOPED_TRACE("SearchElementInfoByAccessibilityId");
            jsAccessibilityManager->SearchElementInfoByAccessibilityId(
                splitElementId, requestId, callback, mode, windowId);
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilitySearchElementInfoById");
}

void JsAccessibilityManager::SearchElementInfoByAccessibilityId(const int64_t elementId, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const int32_t mode, const int32_t windowId)
{
    std::list<AccessibilityElementInfo> infos;

    auto pipeline = GetPipelineByWindowId(windowId);
    if (pipeline) {
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
        if (ngPipeline) {
            SearchElementInfoByAccessibilityIdNG(elementId, mode, infos, pipeline, NG::UI_EXTENSION_OFFSET_MAX);
            SetSearchElementInfoByAccessibilityIdResult(callback, std::move(infos), requestId);
            return;
        }
    }

    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = 0;
    }
    auto weak = WeakClaim(this);
    auto jsAccessibilityManager = weak.Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto node = jsAccessibilityManager->GetAccessibilityNodeFromPage(nodeId);
    if (!node) {
        SetSearchElementInfoByAccessibilityIdResult(callback, std::move(infos), requestId);
        return;
    }

    AccessibilityElementInfo nodeInfo;
    UpdateAccessibilityNodeInfo(node, nodeInfo, jsAccessibilityManager, jsAccessibilityManager->windowId_);
    infos.push_back(nodeInfo);
    // cache parent/siblings/children infos
    UpdateCacheInfo(infos, mode, node, jsAccessibilityManager, jsAccessibilityManager->windowId_);

    SetSearchElementInfoByAccessibilityIdResult(callback, std::move(infos), requestId);
}

void JsAccessibilityManager::SearchElementInfoByAccessibilityIdNG(int64_t elementId, int32_t mode,
    std::list<AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context, int64_t uiExtensionOffset)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "elementId: %{public}" PRId64 ", treeId: %{public}d, mode: %{public}d",
        elementId, treeId_, mode);
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);

    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);

    AccessibilityElementInfo nodeInfo;
    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = rootNode->GetAccessibilityId();
    }

#ifdef WINDOW_SCENE_SUPPORTED
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    if (uiExtensionManager->IsWrapExtensionAbilityId(nodeId)) {
        SearchParameter param {nodeId, "", mode, uiExtensionOffset};
        return SearchExtensionElementInfoByAccessibilityIdNG(param, rootNode, infos, context, ngPipeline);
    }
#endif

    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext);
    auto node = GetFramenodeByAccessibilityId(rootNode, nodeId);
    CHECK_NULL_VOID(node);
    UpdateAccessibilityElementInfo(node, commonProperty, nodeInfo, ngPipeline);
    if (IsExtensionComponent(node) && !IsUIExtensionShowPlaceholder(node)) {
        SearchParameter param {-1, "", mode, uiExtensionOffset};
        SearchExtensionElementInfoNG(param, node, infos, nodeInfo);
    }
    infos.push_back(nodeInfo);
    SearchParameter param {nodeId, "", mode, uiExtensionOffset};
    UpdateCacheInfoNG(infos, node, commonProperty, ngPipeline, param);
    SortExtensionAccessibilityInfo(infos, nodeInfo.GetAccessibilityId());
    if ((infos.size() > 0) && (uiExtensionOffset != NG::UI_EXTENSION_OFFSET_MAX) &&
        (infos.front().GetComponentType() != V2::ROOT_ETS_TAG) &&
        (infos.front().GetParentNodeId() == rootNode->GetAccessibilityId())) {
            infos.front().SetParent(NG::UI_EXTENSION_ROOT_ID);
    }
}

void JsAccessibilityManager::SearchExtensionElementInfoByAccessibilityIdNG(const SearchParameter& searchParam,
    const RefPtr<NG::FrameNode>& node, std::list<Accessibility::AccessibilityElementInfo>& infos,
    const RefPtr<PipelineBase>& context, const RefPtr<NG::PipelineContext>& ngPipeline)
{
#ifdef WINDOW_SCENE_SUPPORTED
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    auto unWrapIdPair = uiExtensionManager->UnWrapExtensionAbilityId(searchParam.uiExtensionOffset, searchParam.nodeId);
    int64_t childWrapId = unWrapIdPair.second;
    int64_t uiExtensionId = unWrapIdPair.first;
    auto uiExtensionNode = FindNodeFromRootByExtensionId(node, uiExtensionId);
    CHECK_NULL_VOID(uiExtensionNode);
    SearchParameter param {childWrapId, "", searchParam.mode, searchParam.uiExtensionOffset};
    AccessibilityElementInfo nodeInfo;
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext);
    UpdateAccessibilityElementInfo(uiExtensionNode, commonProperty, nodeInfo, ngPipeline);
    SearchExtensionElementInfoNG(param, uiExtensionNode, infos, nodeInfo);
#endif
}

std::list<AccessibilityElementInfo> JsAccessibilityManager::SearchElementInfosByTextNG(
    int64_t elementId, const std::string& text, const RefPtr<NG::FrameNode>& node, int64_t offset)
{
    std::list<AccessibilityElementInfo> extensionElementInfo;
    if (NG::UI_EXTENSION_OFFSET_MIN < (offset + 1)) {
        node->SearchElementInfosByTextNG(elementId, text, offset, extensionElementInfo);
    }
    return extensionElementInfo;
}

void JsAccessibilityManager::SearchElementInfosByTextNG(const SearchParameter& searchParam,
    const RefPtr<NG::FrameNode>& node, std::list<Accessibility::AccessibilityElementInfo>& infos,
    const RefPtr<PipelineBase>& context, const RefPtr<NG::PipelineContext>& ngPipeline)
{
#ifdef WINDOW_SCENE_SUPPORTED
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    auto unWrapIdPair = uiExtensionManager->UnWrapExtensionAbilityId(searchParam.uiExtensionOffset, searchParam.nodeId);
    int64_t childWrapId = unWrapIdPair.second;
    int64_t uiExtensionId = unWrapIdPair.first;
    std::list<AccessibilityElementInfo> extensionElementInfos;
    AccessibilityElementInfo nodeInfo;
    auto uiExtensionNode = FindNodeFromRootByExtensionId(node, uiExtensionId);
    CHECK_NULL_VOID(uiExtensionNode);

    extensionElementInfos = SearchElementInfosByTextNG(
        childWrapId, searchParam.text, uiExtensionNode, searchParam.uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR);
    if (extensionElementInfos.empty()) {
        return;
    }
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext);
    UpdateAccessibilityElementInfo(uiExtensionNode, commonProperty, nodeInfo, ngPipeline);
    ConvertExtensionAccessibilityNodeId(extensionElementInfos, uiExtensionNode,
        searchParam.uiExtensionOffset, nodeInfo);
    for (auto& info : extensionElementInfos) {
        infos.emplace_back(info);
    }
#endif
}

RefPtr<NG::FrameNode> JsAccessibilityManager::FindNodeFromRootByExtensionId(
    const RefPtr<NG::FrameNode>& root, const int64_t uiExtensionId)
{
    CHECK_NULL_RETURN(root, nullptr);
    std::queue<RefPtr<NG::UINode>> nodes;
    nodes.push(root);
    RefPtr<NG::FrameNode> frameNode;
    while (!nodes.empty()) {
        auto current = nodes.front();
        nodes.pop();
        frameNode = AceType::DynamicCast<NG::FrameNode>(current);
        if (IsExtensionComponent(frameNode) && !IsUIExtensionShowPlaceholder(frameNode) &&
            (uiExtensionId == frameNode->GetUiExtensionId())) {
            return frameNode;
        }
        const auto& children = current->GetChildren();
        for (const auto& child : children) {
            nodes.push(child);
        }
    }
    return nullptr;
}

void JsAccessibilityManager::SearchElementInfosByTextNG(int64_t elementId, const std::string& text,
    std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
    int64_t uiExtensionOffset)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
#ifdef WINDOW_SCENE_SUPPORTED
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    if (uiExtensionManager->IsWrapExtensionAbilityId(elementId)) {
        SearchParameter param {elementId, text, 0, uiExtensionOffset};
        SearchElementInfosByTextNG(param, rootNode, infos, context, ngPipeline);
        return;
    }
#endif
    if (elementId == NG::UI_EXTENSION_ROOT_ID) {
        elementId = rootNode->GetAccessibilityId();
    }
    auto node = GetFramenodeByAccessibilityId(rootNode, elementId);
    CHECK_NULL_VOID(node);
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext);
    nlohmann::json textJson = nlohmann::json::parse(text, nullptr, false);
    if (textJson.is_null() || textJson.is_discarded() || !textJson.contains("type")) {
        return;
    }
    if (textJson["type"] == "textType") {
        SearchParameter param {0, text, 0, uiExtensionOffset};
        FindTextByTextHint(node, infos, ngPipeline, commonProperty, param);
        return;
    }
    if (!textJson.contains("value")) {
        return;
    }
    SearchParameter param {0, textJson["value"], 0, uiExtensionOffset};
    FindText(node, infos, ngPipeline, commonProperty, param);
}

void JsAccessibilityManager::JsInteractionOperation::SearchElementInfosByText(const int64_t elementId,
    const std::string& text, const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    if (text.empty()) {
        return;
    }
    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto windowId = windowId_;
    if (context) {
        context->GetTaskExecutor()->PostTask(
            [weak = GetHandler(), splitElementId, text, requestId, &callback, windowId]() {
                auto jsAccessibilityManager = weak.Upgrade();
                CHECK_NULL_VOID(jsAccessibilityManager);
                ACE_SCOPED_TRACE("SearchElementInfosByText");
                jsAccessibilityManager->SearchElementInfosByText(
                    splitElementId, text, requestId, callback, windowId);
            },
            TaskExecutor::TaskType::UI, "ArkUIAccessibilitySearchElementInfoByText");
    }
}

void JsAccessibilityManager::SearchElementInfosByText(const int64_t elementId, const std::string& text,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback, const int32_t windowId)
{
    if (text.empty()) {
        return;
    }

    if (elementId == -1) {
        return;
    }

    std::list<AccessibilityElementInfo> infos;

    auto pipeline = GetPipelineByWindowId(windowId);
    if (pipeline) {
        if (AceType::InstanceOf<NG::PipelineContext>(pipeline)) {
            SearchElementInfosByTextNG(elementId, text, infos, pipeline, NG::UI_EXTENSION_OFFSET_MAX);
            SetSearchElementInfoByTextResult(callback, std::move(infos), requestId);
            return;
        }
    }

    auto weak = WeakClaim(this);
    auto jsAccessibilityManager = weak.Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    int64_t nodeId = elementId;
    auto node = jsAccessibilityManager->GetAccessibilityNodeFromPage(nodeId);
    CHECK_NULL_VOID(node);
    std::list<RefPtr<AccessibilityNode>> nodeList;
    OHOS::Ace::Framework::FindText(node, text, nodeList);
    if (!nodeList.empty()) {
        for (const auto& node : nodeList) {
            AccessibilityElementInfo nodeInfo;
            UpdateAccessibilityNodeInfo(node, nodeInfo, jsAccessibilityManager, jsAccessibilityManager->windowId_);
            infos.emplace_back(nodeInfo);
        }
    }

    SetSearchElementInfoByTextResult(callback, std::move(infos), requestId);
}

void JsAccessibilityManager::JsInteractionOperation::FindFocusedElementInfo(const int64_t elementId,
    const int32_t focusType, const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto windowId = windowId_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, focusType, requestId, &callback, windowId]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            ACE_SCOPED_TRACE("FindFocusedElementInfo");
            jsAccessibilityManager->FindFocusedElementInfo(splitElementId, focusType, requestId, callback, windowId);
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityFindFocusedElementInfo");
}

void JsAccessibilityManager::FindFocusedElementInfo(const int64_t elementId, const int32_t focusType,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback, const int32_t windowId)
{
    AccessibilityElementInfo nodeInfo;
    if (focusType != FOCUS_TYPE_INPUT && focusType != FOCUS_TYPE_ACCESSIBILITY) {
        nodeInfo.SetValidElement(false);
        SetFindFocusedElementInfoResult(callback, nodeInfo, requestId);
        return;
    }

    auto context = GetPipelineByWindowId(windowId);
    if (!context) {
        SetFindFocusedElementInfoResult(callback, nodeInfo, requestId);
        return;
    }

    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        FindFocusedElementInfoNG(elementId, focusType, nodeInfo, context, NG::UI_EXTENSION_OFFSET_MAX);
        SetFindFocusedElementInfoResult(callback, nodeInfo, requestId);
        return;
    }

    int64_t nodeId = static_cast<int64_t>(elementId);
    if (elementId == -1) {
        nodeId = 0;
    }

    auto node = GetAccessibilityNodeFromPage(nodeId);
    if (!node) {
        nodeInfo.SetValidElement(false);
        SetFindFocusedElementInfoResult(callback, nodeInfo, requestId);
        return;
    }

    RefPtr<AccessibilityNode> resultNode = nullptr;
    bool status = false;
    if (focusType == FOCUS_TYPE_ACCESSIBILITY) {
        status = FindAccessibilityFocus(node, resultNode);
    }
    if (focusType == FOCUS_TYPE_INPUT) {
        status = FindInputFocus(node, resultNode);
    }

    if ((status) && (resultNode != nullptr)) {
        UpdateAccessibilityNodeInfo(resultNode, nodeInfo, Claim(this), windowId_);
    }

    SetFindFocusedElementInfoResult(callback, nodeInfo, requestId);
}

void JsAccessibilityManager::FindFocusedElementInfoNG(int64_t elementId, int32_t focusType,
    Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
    const int64_t uiExtensionOffset)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
#ifdef WINDOW_SCENE_SUPPORTED
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    if (uiExtensionManager->IsWrapExtensionAbilityId(elementId)) {
        SearchParameter param {elementId, "", focusType, uiExtensionOffset};
        return FindFocusedExtensionElementInfoNG(param, info, context, rootNode);
    }
#endif
    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = rootNode->GetAccessibilityId();
    }
    auto node = GetFramenodeByAccessibilityId(rootNode, nodeId);
    if (!node) {
        return info.SetValidElement(false);
    }
    if (IsExtensionComponent(node) && !IsUIExtensionShowPlaceholder(node)) {
        SearchParameter transferSearchParam {NG::UI_EXTENSION_ROOT_ID, "", focusType, uiExtensionOffset};
        OHOS::Ace::Framework::FindFocusedExtensionElementInfoNG(transferSearchParam, node, info);
        return SetUiExtensionAbilityParentIdForFocus(node, uiExtensionOffset, info);
    }
    RefPtr<NG::FrameNode> resultNode;
    if (focusType == FOCUS_TYPE_ACCESSIBILITY) {
        resultNode = FindAccessibilityFocus(node, focusType, info, uiExtensionOffset, context);
    }
    if (focusType == FOCUS_TYPE_INPUT) {
        resultNode = FindInputFocus(node, focusType, info, uiExtensionOffset, context);
    }
    if ((!resultNode) || (IsExtensionComponent(resultNode) && !IsUIExtensionShowPlaceholder(resultNode))) {
        return;
    }
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext);
    UpdateAccessibilityElementInfo(resultNode, commonProperty, info, ngPipeline);
    UpdateUiExtensionParentIdForFocus(rootNode, uiExtensionOffset, info);
}

void JsAccessibilityManager::FindFocusedExtensionElementInfoNG(const SearchParameter& searchParam,
    Accessibility::AccessibilityElementInfo& info,
    const RefPtr<PipelineBase>& context, const RefPtr<NG::FrameNode>& root)
{
#ifdef WINDOW_SCENE_SUPPORTED
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    auto elementIdPair = uiExtensionManager->UnWrapExtensionAbilityId(searchParam.uiExtensionOffset,
        searchParam.nodeId);
    auto uiExtensionNode = uiExtensionManager->GetFocusUiExtensionNode();
    CHECK_NULL_VOID(uiExtensionNode);
    SearchParameter transferSearchParam {elementIdPair.second, "",
        searchParam.mode, searchParam.uiExtensionOffset};
    OHOS::Ace::Framework::FindFocusedExtensionElementInfoNG(transferSearchParam, uiExtensionNode, info);
    SetUiExtensionAbilityParentIdForFocus(uiExtensionNode, searchParam.uiExtensionOffset, info);
#endif
}

RefPtr<NG::FrameNode> JsAccessibilityManager::FindNodeFromPipeline(
    const WeakPtr<PipelineBase>& context, const int64_t elementId)
{
    auto pipeline = context.Upgrade();
    CHECK_NULL_RETURN(pipeline, nullptr);

    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_RETURN(rootNode, nullptr);

    int64_t nodeId = elementId;
    // accessibility use -1 for first search to get root node
    if (elementId == -1) {
        nodeId = rootNode->GetAccessibilityId();
    }

    auto node = GetFramenodeByAccessibilityId(rootNode, nodeId);
    if (node) {
        return node;
    }
    return nullptr;
}

RefPtr<NG::PipelineContext> JsAccessibilityManager::FindPipelineByElementId(
    const int64_t elementId, RefPtr<NG::FrameNode>& node)
{
    node = FindNodeFromPipeline(context_, elementId);
    if (node) {
        auto context = AceType::DynamicCast<NG::PipelineContext>(context_.Upgrade());
        return context;
    }
    for (auto subContext : GetSubPipelineContexts()) {
        node = FindNodeFromPipeline(subContext, elementId);
        if (node) {
            auto context = AceType::DynamicCast<NG::PipelineContext>(subContext.Upgrade());
            return context;
        }
    }
    return nullptr;
}

void JsAccessibilityManager::JsInteractionOperation::ExecuteAction(const int64_t elementId, const int32_t action,
    const std::map<std::string, std::string>& actionArguments, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "elementId: %{public}" PRId64 ", action: %{public}d", elementId, action);
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto actionInfo = static_cast<ActionType>(action);
    ActionParam param { actionInfo, actionArguments };
    auto windowId = windowId_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, param, requestId, &callback, windowId] {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            ACE_SCOPED_TRACE("ExecuteAction");
            jsAccessibilityManager->ExecuteAction(splitElementId, param, requestId, callback, windowId);
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityExecuteAction");
}

bool JsAccessibilityManager::AccessibilityActionEvent(const ActionType& action,
    const std::map<std::string, std::string>& actionArguments, const RefPtr<AccessibilityNode>& node,
    const RefPtr<PipelineContext>& context)
{
    if (!node || !context) {
        return false;
    }
    ContainerScope scope(context->GetInstanceId());
    switch (action) {
        case ActionType::ACCESSIBILITY_ACTION_CLICK: {
            node->SetClicked(true);
            if (!node->GetClickEventMarker().IsEmpty()) {
#ifndef NG_BUILD
                context->SendEventToFrontend(node->GetClickEventMarker());
#endif
                node->ActionClick();
                return true;
            }
            return node->ActionClick();
        }
        case ActionType::ACCESSIBILITY_ACTION_LONG_CLICK: {
            if (!node->GetLongPressEventMarker().IsEmpty()) {
#ifndef NG_BUILD
                context->SendEventToFrontend(node->GetLongPressEventMarker());
#endif
                node->ActionLongClick();
                return true;
            }
            return node->ActionLongClick();
        }
        case ActionType::ACCESSIBILITY_ACTION_SET_TEXT: {
            if (!node->GetSetTextEventMarker().IsEmpty()) {
#ifndef NG_BUILD
                context->SendEventToFrontend(node->GetSetTextEventMarker());
#endif
                node->ActionSetText(actionArguments.find(ACTION_ARGU_SET_TEXT)->second);
                return true;
            }
            return node->ActionSetText(actionArguments.find(ACTION_ARGU_SET_TEXT)->second);
        }
        case ActionType::ACCESSIBILITY_ACTION_FOCUS: {
#ifndef NG_BUILD
            context->AccessibilityRequestFocus(std::to_string(node->GetNodeId()));
#endif
            if (!node->GetFocusEventMarker().IsEmpty()) {
#ifndef NG_BUILD
                context->SendEventToFrontend(node->GetFocusEventMarker());
#endif
                node->ActionFocus();
                return true;
            }
            return node->ActionFocus();
        }
        case ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS: {
            return RequestAccessibilityFocus(node);
        }
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS: {
            return ClearAccessibilityFocus(node);
        }
        case ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD:
            return node->ActionScrollForward();
        case ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD:
            return node->ActionScrollBackward();
        default:
            return false;
    }
}

void JsAccessibilityManager::SendActionEvent(const Accessibility::ActionType& action, int64_t nodeId)
{
    static std::unordered_map<Accessibility::ActionType, std::string> actionToStr {
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_CLICK, DOM_CLICK },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_LONG_CLICK, DOM_LONG_PRESS },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_FOCUS, DOM_FOCUS },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS, ACCESSIBILITY_FOCUSED_EVENT },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS, ACCESSIBILITY_CLEAR_FOCUS_EVENT },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD, SCROLL_END_EVENT },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD, SCROLL_END_EVENT },
    };
    if (actionToStr.find(action) == actionToStr.end()) {
        return;
    }
    AccessibilityEvent accessibilityEvent;
    accessibilityEvent.eventType = actionToStr[action];
    accessibilityEvent.nodeId = static_cast<int64_t>(nodeId);
    SendAccessibilityAsyncEvent(accessibilityEvent);
}

void stringToLower(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char &c) {
        return std::tolower(c);
    });
}

bool conversionDirection(std::string dir)
{
    stringToLower(dir);
    if (dir.compare(STRING_DIR_FORWARD) == 0) {
        return true;
    }

    return false;
}

bool ActAccessibilityAction(Accessibility::ActionType action, const std::map<std::string, std::string> actionArguments,
    RefPtr<NG::AccessibilityProperty> accessibilityProperty)
{
    AccessibilityActionParam param;
    if (action == ActionType::ACCESSIBILITY_ACTION_SET_SELECTION) {
        int start = -1;
        int end = -1;
        std::string dir = STRING_DIR_BACKWARD;
        auto iter = actionArguments.find(ACTION_ARGU_SELECT_TEXT_START);
        if (iter != actionArguments.end()) {
            std::stringstream str_start;
            str_start << iter->second;
            str_start >> start;
        }
        iter = actionArguments.find(ACTION_ARGU_SELECT_TEXT_END);
        if (iter != actionArguments.end()) {
            std::stringstream str_end;
            str_end << iter->second;
            str_end >> end;
        }
        iter = actionArguments.find(ACTION_ARGU_SELECT_TEXT_INFORWARD);
        if (iter != actionArguments.end()) {
            dir = iter->second;
        }
        param.setSelectionStart = start;
        param.setSelectionEnd = end;
        param.setSelectionDir = conversionDirection(dir);
    }
    if (action == ActionType::ACCESSIBILITY_ACTION_SET_TEXT) {
        auto iter = actionArguments.find(ACTION_ARGU_SET_TEXT);
        if (iter != actionArguments.end()) {
            param.setTextArgument = iter->second;
        }
    }
    if (action == ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT ||
        action == ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT) {
        int moveUnit = TextMoveUnit::STEP_CHARACTER;
        auto iter = actionArguments.find(ACTION_ARGU_MOVE_UNIT);
        if (iter != actionArguments.end()) {
            std::stringstream str_moveUnit;
            str_moveUnit << iter->second;
            str_moveUnit >> moveUnit;
        }
        param.moveUnit = static_cast<TextMoveUnit>(moveUnit);
    }
    if (action == ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION) {
        auto iter = actionArguments.find(ACTION_ARGU_SET_OFFSET);
        int32_t position = -1;
        if (iter != actionArguments.end()) {
            std::stringstream strPosition;
            strPosition << iter->second;
            strPosition >> position;
        }
        param.setCursorIndex = position;
    }
    auto accessibiltyAction = ACTIONS.find(action);
    if (accessibiltyAction != ACTIONS.end()) {
        param.accessibilityProperty = accessibilityProperty;
        return accessibiltyAction->second(param);
    }
    return false;
}

bool JsAccessibilityManager::ExecuteExtensionActionNG(int64_t elementId,
    const std::map<std::string, std::string>& actionArguments, int32_t action, const RefPtr<PipelineBase>& context,
    int64_t uiExtensionOffset)
{
    return ExecuteActionNG(
        elementId, actionArguments, static_cast<ActionType>(action), context, uiExtensionOffset);
}

#ifdef WEB_SUPPORTED
bool JsAccessibilityManager::ExecuteWebActionNG(int64_t elementId, ActionType action,
    const RefPtr<NG::FrameNode>& frameNode, const RefPtr<NG::PipelineContext>& ngPipeline,
    const std::map<std::string, std::string>& actionArguments)
{
    auto webAccessibilityNode = AceType::DynamicCast<NG::WebAccessibilityNode>(frameNode);
    CHECK_NULL_RETURN(webAccessibilityNode, false);
    auto webNode = webAccessibilityNode->GetWebNode();
    CHECK_NULL_RETURN(webNode, false);
    auto webPattern = webNode->GetPattern<NG::WebPattern>();
    CHECK_NULL_RETURN(webPattern, false);
    webPattern->ExecuteAction(elementId, ConvertAccessibilityAction(action), actionArguments);
    return true;
}

int64_t JsAccessibilityManager::GetAccessibilityFocusId() const
{
    return currentFocusNodeId_;
}

void JsAccessibilityManager::UpdateAccessibilityFocusId(const RefPtr<PipelineBase>& context, int64_t accessibilityId,
    bool isFocus)
{
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    if (isFocus) {
        Framework::ClearAccessibilityFocus(ngPipeline->GetRootElement(), currentFocusNodeId_);
        currentFocusNodeId_ = accessibilityId;
    } else {
        currentFocusNodeId_ = -1;
    }
}
#endif

bool JsAccessibilityManager::ExecuteActionNG(int64_t elementId,
    const std::map<std::string, std::string>& actionArguments, ActionType action, const RefPtr<PipelineBase>& context,
    int64_t uiExtensionOffset)
{
    bool result = false;
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_RETURN(ngPipeline, result);
#ifdef WINDOW_SCENE_SUPPORTED
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_RETURN(uiExtensionManager, result);
    if (uiExtensionManager->IsWrapExtensionAbilityId(elementId)) {
        auto unWrapIdPair = uiExtensionManager->UnWrapExtensionAbilityId(uiExtensionOffset, elementId);
        int64_t childWrapId = unWrapIdPair.second;
        int64_t uiExtensionId = unWrapIdPair.first;
        auto rootNode = ngPipeline->GetRootElement();
        CHECK_NULL_RETURN(rootNode, result);
        auto uiExtensionNode = FindNodeFromRootByExtensionId(rootNode, uiExtensionId);
        CHECK_NULL_RETURN(uiExtensionNode, result);
        return OHOS::Ace::Framework::TransferExecuteAction(
            childWrapId, uiExtensionNode, actionArguments, action, uiExtensionOffset);
    }
#endif
    ContainerScope instance(ngPipeline->GetInstanceId());
    auto frameNode = GetFramenodeByAccessibilityId(ngPipeline->GetRootElement(), elementId);

    if (!frameNode && elementId == lastElementId_) {
        frameNode = lastFrameNode_.Upgrade();
    }


    CHECK_NULL_RETURN(frameNode, result);

#ifdef WEB_SUPPORTED
    if (frameNode->GetTag() == V2::WEB_CORE_TAG) {
        return ExecuteWebActionNG(elementId, action, frameNode, ngPipeline, actionArguments);
    }
#endif
    auto enabled = frameNode->GetFocusHub() ? frameNode->GetFocusHub()->IsEnabled() : true;
    if (!enabled && action != ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS &&
        action != ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS) {
        return result;
    }
    result = ConvertActionTypeToBoolen(action, frameNode, elementId, ngPipeline);
    if (!result) {
        auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
        CHECK_NULL_RETURN(accessibilityProperty, false);
        result = ActAccessibilityAction(action, actionArguments, accessibilityProperty);
    }
    return result;
}

bool JsAccessibilityManager::ConvertActionTypeToBoolen(ActionType action, RefPtr<NG::FrameNode>& frameNode,
    int64_t elementId, RefPtr<NG::PipelineContext>& context)
{
    bool result = false;
    switch (action) {
        case ActionType::ACCESSIBILITY_ACTION_FOCUS: {
            result = RequestFocus(frameNode);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS: {
            result = LostFocus(frameNode);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_CLICK: {
            result = ActClick(frameNode);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_LONG_CLICK: {
            result = ActLongClick(frameNode);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS: {
            SaveLast(elementId, frameNode);
            result = ActAccessibilityFocus(elementId, frameNode, context, currentFocusNodeId_, false);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS: {
            SaveLast(elementId, frameNode);
            result = ActAccessibilityFocus(elementId, frameNode, context, currentFocusNodeId_, true);
            break;
        }
        default:
            break;
    }
    return result;
}

void JsAccessibilityManager::ExecuteAction(const int64_t elementId, const ActionParam& param, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const int32_t windowId)
{
    auto action = param.action;
    auto actionArguments = param.actionArguments;

    bool actionResult = false;
    auto context = GetPipelineByWindowId(windowId);
    if (!context) {
        SetExecuteActionResult(callback, actionResult, requestId);
        return;
    }

    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        actionResult = ExecuteActionNG(elementId, actionArguments, action, context, NG::UI_EXTENSION_OFFSET_MAX);
    } else {
        auto node = GetAccessibilityNodeFromPage(elementId);
        if (!node) {
            SetExecuteActionResult(callback, false, requestId);
            return;
        }

        actionResult =
            AccessibilityActionEvent(action, actionArguments, node, AceType::DynamicCast<PipelineContext>(context));
    }
    SetExecuteActionResult(callback, actionResult, requestId);
    if (actionResult && AceType::InstanceOf<PipelineContext>(context)) {
        SendActionEvent(action, elementId);
    }
}

void JsAccessibilityManager::JsInteractionOperation::GetCursorPosition(const int64_t elementId,
    const int32_t requestId, AccessibilityElementOperatorCallback &callback)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto frameNode = GetFramenodeByAccessibilityId(ngPipeline->GetRootElement(), splitElementId);
    CHECK_NULL_VOID(frameNode);
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    auto callNumber = accessibilityProperty->ActActionGetIndex();
    callback.SetCursorPositionResult(callNumber, requestId);
}

void JsAccessibilityManager::JsInteractionOperation::ClearFocus()
{
    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler()] {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            ACE_SCOPED_TRACE("ClearCurrentFocus");
            jsAccessibilityManager->ClearCurrentFocus();
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityClearCurrentFocus");
}

void JsAccessibilityManager::JsInteractionOperation::OutsideTouch() {}

int JsAccessibilityManager::RegisterInteractionOperation(int windowId)
{
    if (IsRegister()) {
        return 0;
    }

    std::shared_ptr<AccessibilitySystemAbilityClient> instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(instance, -1);
    auto interactionOperation = std::make_shared<JsInteractionOperation>(windowId);
    interactionOperation->SetHandler(WeakClaim(this));
    Accessibility::RetError retReg = instance->RegisterElementOperator(windowId, interactionOperation);
    RefPtr<PipelineBase> context;
    for (auto subContext : GetSubPipelineContexts()) {
        context = subContext.Upgrade();
        CHECK_NULL_RETURN(context, -1);
        interactionOperation = std::make_shared<JsInteractionOperation>(context->GetWindowId());
        interactionOperation->SetHandler(WeakClaim(this));
        retReg = instance->RegisterElementOperator(context->GetWindowId(), interactionOperation);
    }
    Register(retReg == RET_OK);
    if (retReg == RET_OK) {
        NotifyChildTreeOnRegister(treeId_);
    }

    return retReg;
}

void JsAccessibilityManager::RegisterSubWindowInteractionOperation(int windowId)
{
    if (!AceApplicationInfo::GetInstance().IsAccessibilityEnabled() || !IsRegister()) {
        return;
    }

    std::shared_ptr<AccessibilitySystemAbilityClient> instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_VOID(instance);
    auto interactionOperation = std::make_shared<JsInteractionOperation>(windowId);
    interactionOperation->SetHandler(WeakClaim(this));
    instance->RegisterElementOperator(windowId, interactionOperation);
}

void JsAccessibilityManager::DeregisterInteractionOperation()
{
    if (!IsRegister()) {
        return;
    }
    int windowId = static_cast<int>(GetWindowId());

    auto instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_VOID(instance);
    Register(false);
    if (currentFocusNodeId_ != -1 && lastElementId_ != -1) {
        auto focusNode = lastFrameNode_.Upgrade();
        if (focusNode != nullptr && focusNode->GetTag() != V2::WEB_CORE_TAG) {
            focusNode->GetRenderContext()->UpdateAccessibilityFocus(false);
        }
    }
    lastFrameNode_.Reset();
    lastElementId_ = -1;
    currentFocusNodeId_ = -1;
    instance->DeregisterElementOperator(windowId);
    RefPtr<PipelineBase> context;
    for (auto subContext : GetSubPipelineContexts()) {
        context = subContext.Upgrade();
        CHECK_NULL_VOID(context);
        instance->DeregisterElementOperator(context->GetWindowId());
    }
    NotifyChildTreeOnDeregister();
}

void JsAccessibilityManager::RegisterAccessibilityChildTreeCallback(
    int64_t elementId, const std::shared_ptr<AccessibilityChildTreeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
    childTreeCallbackMap_[elementId] = callback;
}

void JsAccessibilityManager::DeregisterAccessibilityChildTreeCallback(int64_t elementId)
{
    std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
    childTreeCallbackMap_.erase(elementId);
}

void JsAccessibilityManager::NotifyChildTreeOnRegister(int32_t treeId)
{
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "NotifyChildTreeOnRegister size: %{public}zu", childTreeCallbackMap_.size());
    std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
    for (auto &item : childTreeCallbackMap_) {
        if (item.second == nullptr) {
            continue;
        }
        item.second->OnRegister(GetWindowId(), treeId);
    }
}

void JsAccessibilityManager::NotifyChildTreeOnDeregister()
{
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "NotifyChildTreeOnDeregister size: %{public}zu",
        childTreeCallbackMap_.size());
    std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
    for (auto &item : childTreeCallbackMap_) {
        if (item.second == nullptr) {
            continue;
        }
        item.second->OnDeregister();
    }
}

void JsAccessibilityManager::NotifySetChildTreeIdAndWinId(
    int64_t elementId, const int32_t treeId, const int32_t childWindowId)
{
    std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
    auto findResult = childTreeCallbackMap_.find(elementId);
    if (findResult == childTreeCallbackMap_.end()) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "node: %{public}" PRId64 " not found, treeId: %{public}d", elementId,
            treeId);
        return;
    }
    auto callback = findResult->second;
    CHECK_NULL_VOID(callback);
    callback->SetChildTreeId(treeId);
    callback->OnSetChildTree(childWindowId, treeId);
}

bool JsAccessibilityManager::CheckIsChildElement(
    int64_t &elementId, const std::vector<std::string> &params, std::vector<std::string> &info)
{
    if (elementId <= 0) {
        return false;
    }

    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);
    if (splitTreeId <= 0 || splitTreeId == treeId_) {
        elementId = splitElementId;
        return false;
    }
    std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
    for (const auto &item : childTreeCallbackMap_) {
        if (item.second == nullptr) {
            continue;
        }
        if (item.second->GetChildTreeId() != splitTreeId) {
            continue;
        }
        item.second->OnDumpChildInfo(params, info);
        for (const auto &childInfo : info) {
            DumpLog::GetInstance().Print(childInfo.c_str());
        }
        return true;
    }
    return false;
}

void JsAccessibilityManager::RegisterInteractionOperationAsChildTree(
    uint32_t parentWindowId, int32_t parentTreeId, int64_t parentElementId)
{
    if (IsRegister()) {
        return;
    }
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "parentTreeId: %{public}d, %{public}" PRId64, parentTreeId, parentElementId);

    std::shared_ptr<AccessibilitySystemAbilityClient> instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_VOID(instance);
    uint32_t windowId = GetWindowId();
    auto interactionOperation = std::make_shared<JsInteractionOperation>(windowId);
    interactionOperation->SetHandler(WeakClaim(this));
    Accessibility::Registration registration {
        .windowId = windowId,
        .parentWindowId = parentWindowId,
        .parentTreeId = parentTreeId,
        .elementId = parentElementId,
    };
    Accessibility::RetError retReg = instance->RegisterElementOperator(registration, interactionOperation);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "RegisterElementOperator result: %{public}d", retReg);
    Register(retReg == RET_OK);
    AceApplicationInfo::GetInstance().SetAccessibilityEnabled(retReg == RET_OK);
    parentElementId_ = parentElementId;
    parentWindowId_ = parentWindowId;
}

void JsAccessibilityManager::SetAccessibilityGetParentRectHandler(std::function<void(int32_t &, int32_t &)> &&callback)
{
    getParentRectHandler_ = std::move(callback);
}

void JsAccessibilityManager::DeregisterInteractionOperationAsChildTree()
{
    if (!IsRegister()) {
        return;
    }
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "deregister accessibility childTree");

    std::shared_ptr<AccessibilitySystemAbilityClient> instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_VOID(instance);
    uint32_t windowId = GetWindowId();
    Register(false);
    currentFocusNodeId_ = -1;
    instance->DeregisterElementOperator(windowId);
    AceApplicationInfo::GetInstance().SetAccessibilityEnabled(false);
    parentElementId_ = INVALID_PARENT_ID;
    parentWindowId_ = 0;
}

void JsAccessibilityManager::JsInteractionOperation::SetChildTreeIdAndWinId(
    const int64_t nodeId, const int32_t treeId, const int32_t childWindowId)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(nodeId, splitElementId, splitTreeId);
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "SetChildTreeId node: %{public}" PRId64 " treeId: %{public}d",
        splitElementId, treeId);
    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    jsAccessibilityManager->NotifySetChildTreeIdAndWinId(splitElementId, treeId, childWindowId);
}

void JsAccessibilityManager::JsInteractionOperation::SetBelongTreeId(const int32_t treeId)
{
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "SetBelongTreeId treeId: %{public}d", treeId);
    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    jsAccessibilityManager->treeId_ = treeId;
}

void JsAccessibilityManager::UpdateElementInfoTreeId(Accessibility::AccessibilityElementInfo& info)
{
    if (treeId_ == 0) {
        return;
    }

    int64_t elementId = info.GetAccessibilityId();
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId_, elementId);
    info.SetAccessibilityId(elementId);

    int64_t parentId = info.GetParentNodeId();
    if (parentId != INVALID_PARENT_ID) {
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId_, parentId);
        info.SetParent(parentId);
    }

    std::vector<int64_t> childIds = info.GetChildIds();
    for (int64_t child : childIds) {
        info.RemoveChild(child);
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId_, child);
        info.AddChild(child);
    }
}

void JsAccessibilityManager::UpdateElementInfosTreeId(std::list<Accessibility::AccessibilityElementInfo>& infos)
{
    if (treeId_ == 0) {
        return;
    }

    for (auto &item : infos) {
        int64_t elementId = item.GetAccessibilityId();
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId_, elementId);
        item.SetAccessibilityId(elementId);

        int64_t parentId = item.GetParentNodeId();
        if (parentId != INVALID_PARENT_ID) {
            AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId_, parentId);
            item.SetParent(parentId);
        }

        std::vector<int64_t> childIds = item.GetChildIds();
        for (int64_t child : childIds) {
            item.RemoveChild(child);
            AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId_, child);
            item.AddChild(child);
        }
    }
}

void JsAccessibilityManager::SetPipelineContext(const RefPtr<PipelineBase>& context)
{
    context_ = context;
    if (stateObserver_ != nullptr) {
        stateObserver_->SetPipeline(context_);
    }
}

#ifdef WEB_SUPPORTED
void JsAccessibilityManager::SetWebAccessibilityState(bool state)
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    auto webs = GetWebs(rootNode);
    for (auto& web : webs) {
        CHECK_NULL_VOID(web);
        auto webPattern = web->GetPattern<NG::WebPattern>();
        CHECK_NULL_VOID(webPattern);
        webPattern->SetAccessibilityState(state);
    }
}
#endif

void JsAccessibilityManager::JsAccessibilityStateObserver::OnStateChanged(const bool state)
{
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "accessibility state changed:%{public}d", state);
    // Do not upgrade jsAccessibilityManager on async thread, destructor will cause freeze
    auto pipelineRef = pipeline_.Upgrade();
    CHECK_NULL_VOID(pipelineRef);
    pipelineRef->GetTaskExecutor()->PostTask(
        [weak = accessibilityManager_, state]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            if (state) {
                jsAccessibilityManager->RegisterInteractionOperation(jsAccessibilityManager->GetWindowId());
            } else {
                jsAccessibilityManager->DeregisterInteractionOperation();
            }
            AceApplicationInfo::GetInstance().SetAccessibilityEnabled(state);
#ifdef WEB_SUPPORTED
            jsAccessibilityManager->SetWebAccessibilityState(state);
#endif
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityStateChanged");
}

void JsAccessibilityManager::JsInteractionOperation::FocusMoveSearch(
    int64_t elementId, const int32_t direction, const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto windowId = windowId_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, direction, requestId, &callback, windowId] {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            ACE_SCOPED_TRACE("FocusMoveSearch");
            jsAccessibilityManager->FocusMoveSearch(splitElementId, direction, requestId, callback, windowId);
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityFocusMoveSearch");
}

void JsAccessibilityManager::FocusMoveSearch(const int64_t elementId, const int32_t direction, const int32_t requestId,
    Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t windowId)
{
    AccessibilityElementInfo nodeInfo;
    auto context = GetPipelineByWindowId(windowId);
    if (!context) {
        nodeInfo.SetValidElement(false);
        SetFocusMoveSearchResult(callback, nodeInfo, requestId);
        return;
    }

    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        FocusMoveSearchNG(elementId, direction, nodeInfo, context, NG::UI_EXTENSION_OFFSET_MAX);
        SetFocusMoveSearchResult(callback, nodeInfo, requestId);
        return;
    }

    auto node = GetAccessibilityNodeFromPage(elementId);
    if (!node) {
        nodeInfo.SetValidElement(false);
        SetFocusMoveSearchResult(callback, nodeInfo, requestId);
        return;
    }

    // get root node.
    auto rootNode = node;
    while (rootNode->GetParentNode()) {
        rootNode = rootNode->GetParentNode();
        if (!rootNode->GetParentNode()) {
            break;
        }
    }

    std::list<RefPtr<AccessibilityNode>> nodeList;
    AddFocusableNode(nodeList, rootNode);
    RefPtr<AccessibilityNode> resultNode;

    switch (direction) {
        case FocusMoveDirection::FORWARD:
        case FocusMoveDirection::BACKWARD:
            // forward and backward
            resultNode = FindNodeInRelativeDirection(nodeList, node, direction);
            break;
        case FocusMoveDirection::UP:
        case FocusMoveDirection::DOWN:
        case FocusMoveDirection::LEFT:
        case FocusMoveDirection::RIGHT:
            // up, down, left and right
            resultNode = FindNodeInAbsoluteDirection(nodeList, node, direction);
            break;
        default:
            break;
    }

    if (resultNode) {
        auto jsAccessibilityManager = Claim(this);
        UpdateAccessibilityNodeInfo(resultNode, nodeInfo, jsAccessibilityManager, windowId_);
    }

    SetFocusMoveSearchResult(callback, nodeInfo, requestId);
}

void JsAccessibilityManager::AddFocusableNode(
    std::list<RefPtr<AccessibilityNode>>& nodeList, const RefPtr<AccessibilityNode>& node)
{
    const std::string importance = node->GetImportantForAccessibility();
    if (CanAccessibilityFocused(node)) {
        nodeList.push_back(node);
    }
    if (!node->GetAccessible() && importance != "no-hide-descendants") {
        for (auto& child : node->GetChildList()) {
            AddFocusableNode(nodeList, child);
        }
    }
}

bool JsAccessibilityManager::CanAccessibilityFocused(const RefPtr<AccessibilityNode>& node)
{
    return node != nullptr && !node->IsRootNode() && node->GetVisible() &&
           node->GetImportantForAccessibility() != "no" &&
           node->GetImportantForAccessibility() != "no-hide-descendants";
}

RefPtr<AccessibilityNode> JsAccessibilityManager::FindNodeInRelativeDirection(
    const std::list<RefPtr<AccessibilityNode>>& nodeList, RefPtr<AccessibilityNode>& node, const int direction)
{
    switch (direction) {
        case FocusMoveDirection::FORWARD:
            return GetNextFocusableNode(nodeList, node);
        case FocusMoveDirection::BACKWARD:
            return GetPreviousFocusableNode(nodeList, node);
        default:
            break;
    }

    return nullptr;
}

RefPtr<AccessibilityNode> JsAccessibilityManager::FindNodeInAbsoluteDirection(
    const std::list<RefPtr<AccessibilityNode>>& nodeList, RefPtr<AccessibilityNode>& node, const int direction)
{
    auto tempBest = node->GetRect();
    auto nodeRect = node->GetRect();

    switch (direction) {
        case FocusMoveDirection::LEFT:
            tempBest.SetLeft(node->GetLeft() + node->GetWidth() + 1);
            break;
        case FocusMoveDirection::RIGHT:
            tempBest.SetLeft(node->GetLeft() - node->GetWidth() - 1);
            break;
        case FocusMoveDirection::UP:
            tempBest.SetTop(node->GetTop() + node->GetHeight() + 1);
            break;
        case FocusMoveDirection::DOWN:
            tempBest.SetTop(node->GetTop() - node->GetHeight() - 1);
            break;
        default:
            break;
    }

    RefPtr<AccessibilityNode> nearestNode = nullptr;
    for (auto nodeItem = nodeList.begin(); nodeItem != nodeList.end(); nodeItem++) {
        if ((*nodeItem)->GetNodeId() == node->GetNodeId() || (*nodeItem)->IsRootNode()) {
            continue;
        }
        auto itemRect = (*nodeItem)->GetRect();
        if (CheckBetterRect(nodeRect, direction, itemRect, tempBest)) {
            tempBest = itemRect;
            nearestNode = (*nodeItem);
        }
    }

    return nearestNode;
}

RefPtr<AccessibilityNode> JsAccessibilityManager::GetNextFocusableNode(
    const std::list<RefPtr<AccessibilityNode>>& nodeList, RefPtr<AccessibilityNode>& node)
{
    auto nodeItem = nodeList.begin();
    for (; nodeItem != nodeList.end(); nodeItem++) {
        if ((*nodeItem)->GetNodeId() == node->GetNodeId()) {
            break;
        }
    }

    if (nodeItem != nodeList.end() && ++nodeItem != nodeList.end()) {
        return (*nodeItem);
    }
    if (!nodeList.empty()) {
        return (*nodeList.begin());
    }

    return nullptr;
}

RefPtr<AccessibilityNode> JsAccessibilityManager::GetPreviousFocusableNode(
    const std::list<RefPtr<AccessibilityNode>>& nodeList, RefPtr<AccessibilityNode>& node)
{
    auto nodeItem = nodeList.rbegin();
    for (; nodeItem != nodeList.rend(); nodeItem++) {
        if ((*nodeItem)->GetNodeId() == node->GetNodeId()) {
            break;
        }
    }

    if (nodeItem != nodeList.rend() && ++nodeItem != nodeList.rend()) {
        return (*nodeItem);
    }

    if (!nodeList.empty()) {
        return (*nodeList.rbegin());
    }
    return nullptr;
}

bool JsAccessibilityManager::RequestAccessibilityFocus(const RefPtr<AccessibilityNode>& node)
{
    auto requestNodeId = node->GetNodeId();
    if (currentFocusNodeId_ == requestNodeId) {
        return false;
    }

    ClearCurrentFocus();
    currentFocusNodeId_ = requestNodeId;
    node->SetAccessibilityFocusedState(true);
    return node->ActionAccessibilityFocus(true);
}

bool JsAccessibilityManager::ClearAccessibilityFocus(const RefPtr<AccessibilityNode>& node)
{
    auto requestNodeId = node->GetNodeId();
    if (currentFocusNodeId_ != requestNodeId) {
        return false;
    }

    currentFocusNodeId_ = -1;
    node->SetAccessibilityFocusedState(false);
    return node->ActionAccessibilityFocus(false);
}

bool JsAccessibilityManager::ClearCurrentFocus()
{
    auto currentFocusNode = GetAccessibilityNodeFromPage(currentFocusNodeId_);
    CHECK_NULL_RETURN(currentFocusNode, false);
    currentFocusNodeId_ = -1;
    currentFocusNode->SetFocusedState(false);
    currentFocusNode->SetAccessibilityFocusedState(false);
    return currentFocusNode->ActionAccessibilityFocus(false);
}

void FocusExtensionElementMoveSearchNG(const SearchParameter& searchParam,
    const RefPtr<NG::FrameNode>& node, Accessibility::AccessibilityElementInfo& info)
{
    if (NG::UI_EXTENSION_OFFSET_MIN < (searchParam.uiExtensionOffset + 1)) {
        node->FocusMoveSearchNG(searchParam.nodeId, searchParam.mode,
            searchParam.uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR, info);
    } else {
        info.SetValidElement(false);
    }
}

RefPtr<NG::FrameNode> GetResultOfFocusMoveSearchNGForWeb(RefPtr<NG::FrameNode>& node,
    const RefPtr<NG::FrameNode>& rootNode, int64_t elementId, int32_t direction)
{
    RefPtr<NG::FrameNode> resultNode = nullptr;
    if (node->GetTag() == V2::WEB_CORE_TAG) {
#ifdef WEB_SUPPORTED
        auto webAccessibilityNode = AceType::DynamicCast<NG::WebAccessibilityNode>(node);
        CHECK_NULL_RETURN(webAccessibilityNode, nullptr);
        auto webNode = webAccessibilityNode->GetWebNode();
        CHECK_NULL_RETURN(webNode, nullptr);
        auto webPattern = webNode->GetPattern<NG::WebPattern>();
        CHECK_NULL_RETURN(webPattern, nullptr);
        resultNode = webPattern->GetAccessibilityNodeByFocusMove(elementId, direction);
#endif
    }
    return resultNode;
}

void GetExtensionNextFocusableNode(const AccessibilityElementInfo& focusElement,
    const std::list<AccessibilityElementInfo>& nodeList, AccessibilityElementInfo& node)
{
    auto nodeItem = nodeList.begin();
    for (; nodeItem != nodeList.end(); nodeItem++) {
        if ((*nodeItem).GetAccessibilityId() == focusElement.GetAccessibilityId()) {
            break;
        }
    }

    if (nodeItem != nodeList.end() && ++nodeItem != nodeList.end()) {
        node = (*nodeItem);
    }
    if (!nodeList.empty()) {
        node = (*nodeList.begin());
    }
}

void GetExtensionPreviousFocusableNode(const AccessibilityElementInfo& focusElement,
    const std::list<AccessibilityElementInfo>& nodeList, AccessibilityElementInfo& node)
{
    auto nodeItem = nodeList.rbegin();
    for (; nodeItem != nodeList.rend(); nodeItem++) {
        if ((*nodeItem).GetAccessibilityId() == focusElement.GetAccessibilityId()) {
            break;
        }
    }

    if (nodeItem != nodeList.rend() && ++nodeItem != nodeList.rend()) {
        node = (*nodeItem);
    }

    if (!nodeList.empty()) {
        node = (*nodeList.rbegin());
    }
}

void FindExtensionNodeInAbsoluteDirection(const AccessibilityElementInfo& focusElement,
    const std::list<AccessibilityElementInfo>& nodeList, AccessibilityElementInfo& node, int32_t direction)
{
    auto left = focusElement.GetRectInScreen().GetLeftTopXScreenPostion();
    auto top = focusElement.GetRectInScreen().GetLeftTopYScreenPostion();
    auto width = focusElement.GetRectInScreen().GetRightBottomXScreenPostion() -
        focusElement.GetRectInScreen().GetLeftTopXScreenPostion();
    auto height = focusElement.GetRectInScreen().GetRightBottomYScreenPostion() -
        focusElement.GetRectInScreen().GetLeftTopYScreenPostion();
    Rect rect(left, top, width, height);
    Rect tempBest(left, top, width, height);
    auto nodeRect = tempBest;
    switch (direction) {
        case FocusMoveDirection::LEFT:
            tempBest.SetLeft(left + width + 1);
            break;
        case FocusMoveDirection::RIGHT:
            tempBest.SetLeft(left - width - 1);
            break;
        case FocusMoveDirection::UP:
            tempBest.SetTop(top + height + 1);
            break;
        case FocusMoveDirection::DOWN:
            tempBest.SetTop(top - height - 1);
            break;
        default:
            break;
    }
    for (const auto& nodeItem : nodeList) {
        if (nodeItem.GetAccessibilityId() == focusElement.GetAccessibilityId() ||
            V2::ROOT_ETS_TAG == nodeItem.GetComponentType()) {
            continue;
        }
        left = nodeItem.GetRectInScreen().GetLeftTopXScreenPostion();
        top = nodeItem.GetRectInScreen().GetLeftTopYScreenPostion();
        width = nodeItem.GetRectInScreen().GetRightBottomXScreenPostion() -
            nodeItem.GetRectInScreen().GetLeftTopXScreenPostion();
        height = nodeItem.GetRectInScreen().GetRightBottomYScreenPostion() -
            nodeItem.GetRectInScreen().GetLeftTopYScreenPostion();
        Rect itemRect(left, top, width, height);
        if (CheckBetterRect(nodeRect, direction, itemRect, tempBest)) {
            tempBest = itemRect;
            node = nodeItem;
        }
    }
}

void FindExtensionNodeInRelativeDirection(const AccessibilityElementInfo& focusElement,
    const std::list<AccessibilityElementInfo>& nodeList, AccessibilityElementInfo& node, int direction)
{
    switch (direction) {
        case FocusMoveDirection::FORWARD:
            GetExtensionNextFocusableNode(focusElement, nodeList, node);
            break;
        case FocusMoveDirection::BACKWARD:
            GetExtensionPreviousFocusableNode(focusElement, nodeList, node);
            break;
        default:
            break;
    }
}

void FilterAccessibilityElementByFocusable(std::list<AccessibilityElementInfo>& elementList,
    AccessibilityElementInfo& focusElement, int64_t elementId)
{
    auto input = elementList;
    elementList.clear();
    std::set<int64_t> filterIds;
    for (auto& element : input) {
        if (filterIds.find(element.GetParentNodeId()) != filterIds.end()) {
            filterIds.insert(element.GetAccessibilityId());
            continue;
        }
        auto width = element.GetRectInScreen().GetRightBottomXScreenPostion() -
            element.GetRectInScreen().GetLeftTopXScreenPostion();
        auto height = element.GetRectInScreen().GetRightBottomYScreenPostion() -
            element.GetRectInScreen().GetLeftTopYScreenPostion();
        if (width == 0 || height == 0) {
            filterIds.insert(element.GetAccessibilityId());
            continue;
        }
        elementList.emplace_back(element);
        if (element.GetAccessibilityId() == elementId) {
            focusElement = element;
        }
    }
}

void JsAccessibilityManager::GetResultOfFocusMoveSearchNG(
    int64_t elementId, int32_t direction, AccessibilityElementInfo& info)
{
    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    std::list<AccessibilityElementInfo> nodeList;
    SearchElementInfoByAccessibilityIdNG(NG::UI_EXTENSION_ROOT_ID,
        PREFETCH_RECURSIVE_CHILDREN, nodeList, pipeline, NG::UI_EXTENSION_OFFSET_MAX);
    AccessibilityElementInfo focusElement;
    FilterAccessibilityElementByFocusable(nodeList, focusElement, elementId);
    switch (direction) {
        case FocusMoveDirection::FORWARD:
        case FocusMoveDirection::BACKWARD:
            Framework::FindExtensionNodeInRelativeDirection(focusElement, nodeList, info, direction);
            break;
        case FocusMoveDirection::UP:
        case FocusMoveDirection::DOWN:
        case FocusMoveDirection::LEFT:
        case FocusMoveDirection::RIGHT:
            Framework::FindExtensionNodeInAbsoluteDirection(focusElement, nodeList, info, direction);
            break;
        default:
            break;
    }
}

void JsAccessibilityManager::FocusMoveSearchNG(int64_t elementId, int32_t direction,
    Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
    const int64_t uiExtensionOffset)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    auto node = GetFramenodeByAccessibilityId(rootNode, elementId);
    if (node && node->GetTag() == V2::WEB_CORE_TAG) {
        auto resultNode = GetResultOfFocusMoveSearchNGForWeb(node, rootNode, elementId, direction);
        CHECK_NULL_VOID(resultNode);
        CommonProperty commonProperty;
        GenerateCommonProperty(ngPipeline, commonProperty, mainContext);
        UpdateAccessibilityElementInfo(resultNode, commonProperty, info, ngPipeline);
    } else {
        info.SetValidElement(false);
        GetResultOfFocusMoveSearchNG(elementId, direction, info);
    }
}

void JsAccessibilityManager::FocusExtensionElementMoveSearchNG(const SearchParameter& searchParam,
    Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
    const RefPtr<NG::FrameNode>& root, RefPtr<NG::FrameNode>& outputExtensionNode)
{
#ifdef WINDOW_SCENE_SUPPORTED
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    auto elementIdPair =
        uiExtensionManager->UnWrapExtensionAbilityId(searchParam.uiExtensionOffset, searchParam.nodeId);
    outputExtensionNode = uiExtensionManager->GetFocusUiExtensionNode();
    CHECK_NULL_VOID(outputExtensionNode);
    SearchParameter transferSearchParam {elementIdPair.second, "",
        searchParam.mode, searchParam.uiExtensionOffset};
    OHOS::Ace::Framework::FocusExtensionElementMoveSearchNG(transferSearchParam, outputExtensionNode, info);
    AccessibilityElementInfo parentInfo;
    ConvertExtensionAccessibilityId(info, outputExtensionNode, searchParam.uiExtensionOffset, parentInfo);
#endif
}

// AccessibilitySystemAbilityClient will release callback after DeregisterElementOperator
void JsAccessibilityManager::SetSearchElementInfoByAccessibilityIdResult(AccessibilityElementOperatorCallback& callback,
    std::list<AccessibilityElementInfo>&& infos, const int32_t requestId)
{
    if (!IsRegister()) {
        return;
    }
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), infos = std::move(infos), &callback, requestId] () mutable {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "winId: %{public}d, treeId: %{public}d, reqId: %{public}d",
                jsAccessibilityManager->windowId_, jsAccessibilityManager->treeId_, requestId);
            if (!jsAccessibilityManager->IsRegister()) {
                return;
            }
            jsAccessibilityManager->UpdateElementInfosTreeId(infos);
            callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
        }, TaskExecutor::TaskType::BACKGROUND, "ArkUIAccessibilitySetSearchElementInfoById");
}

void JsAccessibilityManager::SetSearchElementInfoByTextResult(AccessibilityElementOperatorCallback& callback,
    std::list<AccessibilityElementInfo>&& infos, const int32_t requestId)
{
    if (!IsRegister()) {
        return;
    }
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), infos = std::move(infos), &callback, requestId] () mutable {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            if (!jsAccessibilityManager->IsRegister()) {
                return;
            }
            jsAccessibilityManager->UpdateElementInfosTreeId(infos);
            callback.SetSearchElementInfoByTextResult(infos, requestId);
        }, TaskExecutor::TaskType::BACKGROUND, "ArkUIAccessibilitySetSearchElementInfoByText");
}

void JsAccessibilityManager::SetFindFocusedElementInfoResult(
    AccessibilityElementOperatorCallback& callback, AccessibilityElementInfo& info, const int32_t requestId)
{
    if (IsRegister()) {
        UpdateElementInfoTreeId(info);
        callback.SetFindFocusedElementInfoResult(info, requestId);
    }
}

void JsAccessibilityManager::SetFocusMoveSearchResult(
    AccessibilityElementOperatorCallback& callback, AccessibilityElementInfo& info, const int32_t requestId)
{
    if (IsRegister()) {
        UpdateElementInfoTreeId(info);
        callback.SetFocusMoveSearchResult(info, requestId);
    }
}

void JsAccessibilityManager::SetExecuteActionResult(
    AccessibilityElementOperatorCallback& callback, const bool succeeded, const int32_t requestId)
{
    if (IsRegister()) {
        callback.SetExecuteActionResult(succeeded, requestId);
    }
}

std::string JsAccessibilityManager::GetPagePath()
{
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, "");
    auto frontend = context->GetFrontend();
    CHECK_NULL_RETURN(frontend, "");
    ContainerScope scope(context->GetInstanceId());
    return frontend->GetPagePath();
}

void JsAccessibilityManager::GenerateCommonProperty(const RefPtr<PipelineBase>& context, CommonProperty& output,
    const RefPtr<PipelineBase>& mainContext)
{
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto stageManager = ngPipeline->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    output.windowId = static_cast<int32_t>(ngPipeline->GetFocusWindowId());
    if (getParentRectHandler_) {
        getParentRectHandler_(output.windowTop, output.windowLeft);
    } else {
        output.windowLeft = GetWindowLeft(ngPipeline->GetWindowId());
        output.windowTop = GetWindowTop(ngPipeline->GetWindowId());
    }
    auto page = stageManager->GetLastPage();
    if (page != nullptr) {
        output.pageId = page->GetPageId();
        output.pagePath = GetPagePath();
    }
    if (context->GetWindowId() != mainContext->GetWindowId()) {
        output.pageId = 0;
        output.pagePath = "";
    }
}

void JsAccessibilityManager::FindText(const RefPtr<NG::UINode>& node,
    std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<NG::PipelineContext>& context,
    const CommonProperty& commonProperty, const SearchParameter& searchParam)
{
    CHECK_NULL_VOID(node);
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    if (frameNode && !frameNode->IsInternal()) {
        if (frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>()->GetGroupText().find(
            searchParam.text) != std::string::npos) {
            AccessibilityElementInfo nodeInfo;
            UpdateAccessibilityElementInfo(frameNode, commonProperty, nodeInfo, context);
            infos.emplace_back(nodeInfo);
        }
    }
    if (IsExtensionComponent(frameNode) && !IsUIExtensionShowPlaceholder(frameNode)) {
        auto infosByIPC = SearchElementInfosByTextNG(NG::UI_EXTENSION_ROOT_ID, searchParam.text,
            frameNode, searchParam.uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR);
        if (!infosByIPC.empty()) {
            AccessibilityElementInfo nodeInfo;
            UpdateAccessibilityElementInfo(frameNode, commonProperty, nodeInfo, context);
            ConvertExtensionAccessibilityNodeId(infosByIPC, frameNode, searchParam.uiExtensionOffset, nodeInfo);
            for (auto& info : infosByIPC) {
                infos.emplace_back(info);
            }
        }
    }
    if (!node->GetChildren().empty()) {
        for (const auto& child : node->GetChildren()) {
            FindText(child, infos, context, commonProperty, searchParam);
        }
    }
}

void JsAccessibilityManager::FindTextByTextHint(const RefPtr<NG::UINode>& node,
    std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<NG::PipelineContext>& context,
    const CommonProperty& commonProperty, const SearchParameter& searchParam)
{
    CHECK_NULL_VOID(node);
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    if (frameNode && !frameNode->IsInternal()) {
        std::string text = searchParam.text;
        nlohmann::json textJson = nlohmann::json::parse(text, nullptr, false);
        std::string value = "";
        if (!textJson.is_null() && !textJson.is_discarded() && textJson.contains("value")) {
            value = textJson["value"];
        }
        std::string textType = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>()->GetTextType();
        nlohmann::json textTypeJson = nlohmann::json::parse(textType, nullptr, false);
        if (!textTypeJson.is_null() && textTypeJson.contains("type") && textTypeJson["type"] == value) {
            AccessibilityElementInfo nodeInfo;
            UpdateAccessibilityElementInfo(frameNode, commonProperty, nodeInfo, context);
            infos.emplace_back(nodeInfo);
        }
    }
    if (IsExtensionComponent(frameNode)) {
        auto infosByIPC = SearchElementInfosByTextNG(NG::UI_EXTENSION_ROOT_ID, searchParam.text,
            frameNode, searchParam.uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR);
        if (!infosByIPC.empty()) {
            AccessibilityElementInfo nodeInfo;
            UpdateAccessibilityElementInfo(frameNode, commonProperty, nodeInfo, context);
            ConvertExtensionAccessibilityNodeId(infosByIPC, frameNode, searchParam.uiExtensionOffset, nodeInfo);
            for (auto& info : infosByIPC) {
                infos.emplace_back(info);
            }
        }
    }
    if (!node->GetChildren().empty()) {
        for (const auto& child : node->GetChildren()) {
            FindTextByTextHint(child, infos, context, commonProperty, searchParam);
        }
    }
}

} // namespace OHOS::Ace::Framework
