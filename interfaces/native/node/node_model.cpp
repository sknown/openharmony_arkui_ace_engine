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

#include "node_model.h"

#include <cstdint>
#include <set>
#include <unordered_map>

#include "event_converter.h"
#include "interfaces/native/event/ui_input_event_impl.h"
#include "native_node.h"
#include "native_type.h"
#include "style_modifier.h"

#include "base/error/error_code.h"
#include "base/log/log_wrapper.h"
#include "base/utils/utils.h"
#include "core/interfaces/arkoala/arkoala_api.h"

namespace OHOS::Ace::NodeModel {
namespace {
#if defined(WINDOWS_PLATFORM)
#include <windows.h>
// Here we need to find module where GetArkUINodeAPI()
// function is implemented.
void* FindModule()
{
    // To find from main exe
    HMODULE result = nullptr;
    const char libname[] = "libace_compatible.dll";
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, libname, &result);
    if (result) {
        return result;
    }
    TAG_LOGE(AceLogTag::ACE_NATIVE_NODE, "Cannot find module!");
    return nullptr;
}
void* FindFunction(void* library, const char* name)
{
    return (void*)GetProcAddress(reinterpret_cast<HMODULE>(library), name);
}
#else
#include <dlfcn.h>
void* FindModule()
{
    const char libname[] = "libace_compatible.z.so";
    void* result = dlopen(libname, RTLD_LAZY | RTLD_LOCAL);
    if (result) {
        return result;
    }
    TAG_LOGE(AceLogTag::ACE_NATIVE_NODE, "Cannot load libace: %{public}s", dlerror());
    return nullptr;
}

void* FindFunction(void* library, const char* name)
{
    return dlsym(library, name);
}
#endif

ArkUIFullNodeAPI* impl = nullptr;

ArkUIFullNodeAPI* GetAnyFullNodeImpl(int version)
{
    if (!impl) {
        typedef ArkUIAnyAPI* (*GetAPI_t)(int);
        GetAPI_t getAPI = nullptr;
        void* module = FindModule();
        if (module == nullptr) {
            TAG_LOGE(AceLogTag::ACE_NATIVE_NODE, "fail to get module");
            return nullptr;
        }
        // Note, that RTLD_DEFAULT is ((void *) 0).
        getAPI = reinterpret_cast<GetAPI_t>(FindFunction(module, "GetArkUIAnyFullNodeAPI"));
        if (!getAPI) {
            TAG_LOGE(AceLogTag::ACE_NATIVE_NODE, "Cannot find GetArkUIAnyFullNodeAPI()");
            return nullptr;
        }

        impl = reinterpret_cast<ArkUIFullNodeAPI*>((*getAPI)(version));
        if (!impl) {
            TAG_LOGE(AceLogTag::ACE_NATIVE_NODE, "getAPI() returned null");
            return nullptr;
        }

        if (impl->version != version) {
            TAG_LOGE(AceLogTag::ACE_NATIVE_NODE,
                "API version mismatch: expected %{public}d, but get the version %{public}d", version, impl->version);
            return nullptr;
        }
    }

    impl->getBasicAPI()->registerNodeAsyncEventReceiver(OHOS::Ace::NodeModel::HandleInnerEvent);
    return impl;
}
} // namespace

ArkUIFullNodeAPI* GetFullImpl()
{
    return GetAnyFullNodeImpl(ARKUI_NODE_API_VERSION);
}

struct InnerEventExtraParam {
    int32_t targetId;
    ArkUI_NodeHandle nodePtr;
    void* userData;
};

struct ExtraData {
    std::unordered_map<int64_t, InnerEventExtraParam*> eventMap;
};

std::set<ArkUI_NodeHandle> g_nodeSet;

ArkUI_NodeHandle CreateNode(ArkUI_NodeType type)
{
    static const ArkUINodeType nodes[] = { ARKUI_CUSTOM, ARKUI_TEXT, ARKUI_SPAN, ARKUI_IMAGE_SPAN, ARKUI_IMAGE,
        ARKUI_TOGGLE, ARKUI_LOADING_PROGRESS, ARKUI_TEXT_INPUT, ARKUI_TEXTAREA, ARKUI_BUTTON, ARKUI_PROGRESS,
        ARKUI_CHECKBOX, ARKUI_XCOMPONENT, ARKUI_DATE_PICKER, ARKUI_TIME_PICKER, ARKUI_TEXT_PICKER,
        ARKUI_CALENDAR_PICKER, ARKUI_SLIDER, ARKUI_STACK, ARKUI_SWIPER, ARKUI_SCROLL, ARKUI_LIST, ARKUI_LIST_ITEM,
        ARKUI_LIST_ITEM_GROUP, ARKUI_COLUMN, ARKUI_ROW, ARKUI_FLEX, ARKUI_REFRESH, ARKUI_WATER_FLOW, ARKUI_FLOW_ITEM };
    // already check in entry point.
    int32_t nodeType = type < MAX_NODE_SCOPE_NUM ? type : (type - MAX_NODE_SCOPE_NUM + BASIC_COMPONENT_NUM);
    auto* impl = GetFullImpl();
    if (nodeType > sizeof(nodes) / sizeof(ArkUINodeType)) {
        TAG_LOGE(AceLogTag::ACE_NATIVE_NODE, "node type: %{public}d NOT IMPLEMENT", type);
        return nullptr;
    }

    ArkUI_Int32 id = ARKUI_AUTO_GENERATE_NODE_ID;
    auto* uiNode = impl->getBasicAPI()->createNode(nodes[nodeType], id, 0);
    if (!uiNode) {
        TAG_LOGE(AceLogTag::ACE_NATIVE_NODE, "node type: %{public}d can not find in full impl", type);
        return nullptr;
    }
    impl->getBasicAPI()->markDirty(uiNode, ARKUI_DIRTY_FLAG_ATTRIBUTE_DIFF);
    ArkUI_Node* arkUINode = new ArkUI_Node({ type, uiNode });
    impl->getExtendedAPI()->setAttachNodePtr(uiNode, reinterpret_cast<void*>(arkUINode));
    g_nodeSet.emplace(arkUINode);
    return arkUINode;
}

void DisposeNode(ArkUI_NodeHandle nativePtr)
{
    CHECK_NULL_VOID(nativePtr);
    // already check in entry point.
    auto* impl = GetFullImpl();
    impl->getBasicAPI()->disposeNode(nativePtr->uiNodeHandle);
    g_nodeSet.erase(nativePtr);
    delete nativePtr;
}

int32_t AddChild(ArkUI_NodeHandle parentNode, ArkUI_NodeHandle childNode)
{
    CHECK_NULL_RETURN(parentNode, ERROR_CODE_PARAM_INVALID);
    CHECK_NULL_RETURN(childNode, ERROR_CODE_PARAM_INVALID);
    if (parentNode->type == -1) {
        return ERROR_CODE_NATIVE_IMPL_BUILDER_NODE_ERROR;
    }
    auto* impl = GetFullImpl();
    // already check in entry point.
    impl->getBasicAPI()->addChild(parentNode->uiNodeHandle, childNode->uiNodeHandle);
    impl->getBasicAPI()->markDirty(parentNode->uiNodeHandle, ARKUI_DIRTY_FLAG_MEASURE_BY_CHILD_REQUEST);
    return ERROR_CODE_NO_ERROR;
}

int32_t RemoveChild(ArkUI_NodeHandle parentNode, ArkUI_NodeHandle childNode)
{
    CHECK_NULL_RETURN(parentNode, ERROR_CODE_PARAM_INVALID);
    CHECK_NULL_RETURN(childNode, ERROR_CODE_PARAM_INVALID);
    // already check in entry point.
    if (parentNode->type == -1) {
        return ERROR_CODE_NATIVE_IMPL_BUILDER_NODE_ERROR;
    }
    auto* impl = GetFullImpl();
    impl->getBasicAPI()->removeChild(parentNode->uiNodeHandle, childNode->uiNodeHandle);
    impl->getBasicAPI()->markDirty(parentNode->uiNodeHandle, ARKUI_DIRTY_FLAG_MEASURE_BY_CHILD_REQUEST);
    return ERROR_CODE_NO_ERROR;
}

int32_t InsertChildAfter(ArkUI_NodeHandle parentNode, ArkUI_NodeHandle childNode, ArkUI_NodeHandle siblingNode)
{
    CHECK_NULL_RETURN(parentNode, ERROR_CODE_PARAM_INVALID);
    CHECK_NULL_RETURN(childNode, ERROR_CODE_PARAM_INVALID);
    // already check in entry point.
    if (parentNode->type == -1) {
        return ERROR_CODE_NATIVE_IMPL_BUILDER_NODE_ERROR;
    }
    auto* impl = GetFullImpl();
    impl->getBasicAPI()->insertChildAfter(
        parentNode->uiNodeHandle, childNode->uiNodeHandle, siblingNode ? siblingNode->uiNodeHandle : nullptr);
    impl->getBasicAPI()->markDirty(parentNode->uiNodeHandle, ARKUI_DIRTY_FLAG_MEASURE_BY_CHILD_REQUEST);
    return ERROR_CODE_NO_ERROR;
}

int32_t InsertChildBefore(ArkUI_NodeHandle parentNode, ArkUI_NodeHandle childNode, ArkUI_NodeHandle siblingNode)
{
    CHECK_NULL_RETURN(parentNode, ERROR_CODE_PARAM_INVALID);
    CHECK_NULL_RETURN(childNode, ERROR_CODE_PARAM_INVALID);
    // already check in entry point.
    if (parentNode->type == -1) {
        return ERROR_CODE_NATIVE_IMPL_BUILDER_NODE_ERROR;
    }
    auto* impl = GetFullImpl();
    impl->getBasicAPI()->insertChildBefore(
        parentNode->uiNodeHandle, childNode->uiNodeHandle, siblingNode ? siblingNode->uiNodeHandle : nullptr);
    impl->getBasicAPI()->markDirty(parentNode->uiNodeHandle, ARKUI_DIRTY_FLAG_MEASURE_BY_CHILD_REQUEST);
    return ERROR_CODE_NO_ERROR;
}

int32_t InsertChildAt(ArkUI_NodeHandle parentNode, ArkUI_NodeHandle childNode, int32_t position)
{
    CHECK_NULL_RETURN(parentNode, ERROR_CODE_PARAM_INVALID);
    CHECK_NULL_RETURN(childNode, ERROR_CODE_PARAM_INVALID);
    // already check in entry point.
    if (parentNode->type == -1) {
        return ERROR_CODE_NATIVE_IMPL_BUILDER_NODE_ERROR;
    }
    auto* impl = GetFullImpl();
    impl->getBasicAPI()->insertChildAt(parentNode->uiNodeHandle, childNode->uiNodeHandle, position);
    impl->getBasicAPI()->markDirty(parentNode->uiNodeHandle, ARKUI_DIRTY_FLAG_MEASURE_BY_CHILD_REQUEST);
    return ERROR_CODE_NO_ERROR;
}

void SetAttribute(ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute, const char* value)
{
    SetNodeAttribute(node, attribute, value);
}

int32_t SetAttribute(ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute, const ArkUI_AttributeItem* value)
{
    if (node->type == -1) {
        return ERROR_CODE_NATIVE_IMPL_BUILDER_NODE_ERROR;
    }
    return SetNodeAttribute(node, attribute, value);
}

int32_t ResetAttribute(ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute)
{
    if (node->type == -1) {
        return ERROR_CODE_NATIVE_IMPL_BUILDER_NODE_ERROR;
    }
    return ResetNodeAttribute(node, attribute);
}

const ArkUI_AttributeItem* GetAttribute(ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute)
{
    return GetNodeAttribute(node, attribute);
}

int32_t RegisterNodeEvent(ArkUI_NodeHandle nodePtr, ArkUI_NodeEventType eventType, int32_t targetId)
{
    return RegisterNodeEvent(nodePtr, eventType, targetId, nullptr);
}

int32_t RegisterNodeEvent(ArkUI_NodeHandle nodePtr, ArkUI_NodeEventType eventType, int32_t targetId, void* userData)
{
    auto originEventType = ConvertOriginEventType(eventType, nodePtr->type);
    if (originEventType < 0) {
        TAG_LOGE(AceLogTag::ACE_NATIVE_NODE, "event is not supported %{public}d", eventType);
        return ERROR_CODE_NATIVE_IMPL_TYPE_NOT_SUPPORTED;
    }
    // already check in entry point.
    if (nodePtr->type == -1) {
        return ERROR_CODE_NATIVE_IMPL_BUILDER_NODE_ERROR;
    }
    auto* impl = GetFullImpl();
    auto* extraParam = new InnerEventExtraParam({ targetId, nodePtr, userData });
    if (nodePtr->extraData) {
        auto* extraData = reinterpret_cast<ExtraData*>(nodePtr->extraData);
        auto result = extraData->eventMap.try_emplace(eventType, extraParam);
        if (!result.second) {
            result.first->second->targetId = targetId;
            result.first->second->userData = userData;
            delete extraParam;
        }
    } else {
        nodePtr->extraData = new ExtraData();
        auto* extraData = reinterpret_cast<ExtraData*>(nodePtr->extraData);
        extraData->eventMap[eventType] = extraParam;
    }
    impl->getBasicAPI()->registerNodeAsyncEvent(
        nodePtr->uiNodeHandle, static_cast<ArkUIEventSubKind>(originEventType), reinterpret_cast<int64_t>(nodePtr));
    return ERROR_CODE_NO_ERROR;
}

void UnregisterNodeEvent(ArkUI_NodeHandle nodePtr, ArkUI_NodeEventType eventType)
{
    if (!nodePtr->extraData) {
        return;
    }
    if (nodePtr->type == -1) {
        return;
    }
    auto* extraData = reinterpret_cast<ExtraData*>(nodePtr->extraData);
    auto& eventMap = extraData->eventMap;
    auto innerEventExtraParam = eventMap.find(eventType);
    if (innerEventExtraParam == eventMap.end()) {
        return;
    }
    delete innerEventExtraParam->second;
    eventMap.erase(innerEventExtraParam);
    if (eventMap.empty()) {
        delete extraData;
        nodePtr->extraData = nullptr;
    }
}

void (*g_compatibleEventReceiver)(ArkUI_CompatibleNodeEvent* event) = nullptr;
void RegisterOnEvent(void (*eventReceiver)(ArkUI_CompatibleNodeEvent* event))
{
    g_compatibleEventReceiver = eventReceiver;
}

void (*g_eventReceiver)(ArkUI_NodeEvent* event) = nullptr;
void RegisterOnEvent(void (*eventReceiver)(ArkUI_NodeEvent* event))
{
    g_eventReceiver = eventReceiver;
}

void UnregisterOnEvent()
{
    g_eventReceiver = nullptr;
}

void HandleInnerNodeEvent(ArkUINodeEvent* innerEvent)
{
    if (!innerEvent) {
        return;
    }
    if (!g_eventReceiver) {
        TAG_LOGE(AceLogTag::ACE_NATIVE_NODE, "event receiver is not register");
        return;
    }

    ArkUI_NodeEvent event;
    auto* nodePtr = reinterpret_cast<ArkUI_NodeHandle>(innerEvent->extraParam);
    if (g_nodeSet.count(nodePtr) == 0) {
        return;
    }
    if (!nodePtr->extraData) {
        return;
    }
    auto extraData = reinterpret_cast<ExtraData*>(nodePtr->extraData);
    ArkUIEventSubKind subKind = static_cast<ArkUIEventSubKind>(-1);
    switch (innerEvent->kind) {
        case COMPONENT_ASYNC_EVENT:
            subKind = static_cast<ArkUIEventSubKind>(innerEvent->componentAsyncEvent.subKind);
            break;
        case TEXT_INPUT:
            subKind = static_cast<ArkUIEventSubKind>(innerEvent->textInputEvent.subKind);
            break;
        case TOUCH_EVENT:
            subKind = static_cast<ArkUIEventSubKind>(innerEvent->touchEvent.subKind);
            break;
        default:
            break; /* Empty */
    }
    ArkUI_NodeEventType eventType = static_cast<ArkUI_NodeEventType>(ConvertToNodeEventType(subKind));
    auto innerEventExtraParam = extraData->eventMap.find(eventType);
    if (innerEventExtraParam == extraData->eventMap.end()) {
        TAG_LOGE(AceLogTag::ACE_NATIVE_NODE, "the event of %{public}d is not register", eventType);
        return;
    }
    event.node = nodePtr;
    event.eventId = innerEventExtraParam->second->targetId;
    event.userData = innerEventExtraParam->second->userData;
    if (ConvertEvent(innerEvent, &event)) {
        event.targetId = innerEvent->nodeId;
        ArkUI_UIInputEvent uiEvent;
        if (eventType == NODE_TOUCH_EVENT || eventType == NODE_ON_TOUCH_INTERCEPT) {
            uiEvent.inputType = ARKUI_UIINPUTEVENT_TYPE_TOUCH;
            uiEvent.eventTypeId = C_TOUCH_EVENT_ID;
            uiEvent.inputEvent = &(innerEvent->touchEvent);
            event.origin = &uiEvent;
        } else {
            event.origin = innerEvent;
        }
        g_eventReceiver(&event);
    }
    if (g_compatibleEventReceiver) {
        ArkUI_CompatibleNodeEvent event;
        event.node = nodePtr;
        event.eventId = innerEventExtraParam->second->targetId;
        if (ConvertEvent(innerEvent, &event)) {
            g_compatibleEventReceiver(&event);
            ConvertEventResult(&event, innerEvent);
        }
    }
}

int32_t CheckEvent(ArkUI_NodeEvent* event)
{
    // TODO.
    return 0;
}


int32_t SetLengthMetricUnit(ArkUI_NodeHandle nodePtr, ArkUI_LengthMetricUnit unit)
{
    if (!nodePtr) {
        return ERROR_CODE_PARAM_INVALID;
    }
    if (!InRegion(static_cast<int32_t>(ARKUI_LENGTH_METRIC_UNIT_DEFAULT),
        static_cast<int32_t>(ARKUI_LENGTH_METRIC_UNIT_FP), static_cast<int32_t>(unit))) {
        return ERROR_CODE_PARAM_INVALID;
    }
    nodePtr->lengthMetricUnit = unit;
    return ERROR_CODE_NO_ERROR;
}

void ApplyModifierFinish(ArkUI_NodeHandle nodePtr)
{
    // already check in entry point.
    if (!nodePtr) {
        return;
    }
    auto* impl = GetFullImpl();
    impl->getBasicAPI()->applyModifierFinish(nodePtr->uiNodeHandle);
}

void MarkDirty(ArkUI_NodeHandle nodePtr, ArkUI_NodeDirtyFlag dirtyFlag)
{
    // spanNode inherited from UINode
    if (!nodePtr) {
        return;
    }
    ArkUIDirtyFlag flag = ARKUI_DIRTY_FLAG_MEASURE;
    switch (dirtyFlag) {
        case NODE_NEED_MEASURE: {
            flag = ARKUI_DIRTY_FLAG_MEASURE_SELF_AND_PARENT;
            break;
        }
        case NODE_NEED_LAYOUT: {
            flag = ARKUI_DIRTY_FLAG_LAYOUT;
            break;
        }
        case NODE_NEED_RENDER: {
            flag = ARKUI_DIRTY_FLAG_RENDER;
            break;
        }
        default: {
            flag = ARKUI_DIRTY_FLAG_MEASURE;
        }
    }
    // already check in entry point.
    auto* impl = GetFullImpl();
    impl->getBasicAPI()->markDirty(nodePtr->uiNodeHandle, flag);
}

} // namespace OHOS::Ace::NodeModel
