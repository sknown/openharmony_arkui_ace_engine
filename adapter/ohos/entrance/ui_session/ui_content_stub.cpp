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

#include "interfaces/inner_api/ui_session/ui_content_stub.h"

#include "interfaces/inner_api/ui_session/ui_session_manager.h"
#include "ui_content_errors.h"

#include "adapter/ohos/entrance/ui_session/include/ui_service_hilog.h"

namespace OHOS::Ace {
int32_t UiContentStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LOGW("ui_session InterfaceToken check failed");
        return -1;
    }
    switch (code) {
        case UI_CONTENT_SERVICE_GET_TREE: {
            GetInspectorTreeInner(data, reply, option);
            break;
        }
        case UI_CONTENT_CONNECT: {
            ConnectInner(data, reply, option);
            break;
        }
        case REGISTER_CLICK_EVENT: {
            RegisterClickEventCallbackInner(data, reply, option);
            break;
        }
        case REGISTER_SEARCH_EVENT: {
            RegisterRouterChangeEventCallbackInner(data, reply, option);
            break;
        }
        case REGISTER_ROUTER_CHANGE_EVENT: {
            RegisterSearchEventCallbackInner(data, reply, option);
            break;
        }
        case REGISTER_COMPONENT_EVENT: {
            RegisterComponentChangeEventCallbackInner(data, reply, option);
            break;
        }
        case UNREGISTER_CLICK_EVENT: {
            UnregisterClickEventCallbackInner(data, reply, option);
            break;
        }
        case UNREGISTER_SEARCH_EVENT: {
            UnregisterSearchEventCallbackInner(data, reply, option);
            break;
        }
        case UNREGISTER_ROUTER_CHANGE_EVENT: {
            UnregisterRouterChangeEventCallbackInner(data, reply, option);
            break;
        }
        case UNREGISTER_COMPONENT_EVENT: {
            UnregisterComponentChangeEventCallbackInner(data, reply, option);
            break;
        }
        default: {
            LOGI("ui_session unknown transaction code %{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
    return 0;
}

int32_t UiContentStub::GetInspectorTreeInner(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    return NO_ERROR;
}

int32_t UiContentStub::ConnectInner(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    sptr<IRemoteObject> report = data.ReadRemoteObject();
    if (report == nullptr) {
        LOGW("read reportStub object is nullptr,connect failed");
        return FAILED;
    }
    int32_t processId = data.ReadInt32();
    UiSessionManager::GetInstance().SaveReportStub(report, processId);
    return NO_ERROR;
}

int32_t UiContentStub::RegisterClickEventCallbackInner(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    reply.WriteInt32(RegisterClickEventCallback(nullptr));
    return NO_ERROR;
}

int32_t UiContentStub::RegisterRouterChangeEventCallbackInner(
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    reply.WriteInt32(RegisterRouterChangeEventCallback(nullptr));
    return NO_ERROR;
}

int32_t UiContentStub::RegisterSearchEventCallbackInner(
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    reply.WriteInt32(RegisterSearchEventCallback(nullptr));
    return NO_ERROR;
}

int32_t UiContentStub::RegisterComponentChangeEventCallbackInner(
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    reply.WriteInt32(RegisterRouterChangeEventCallback(nullptr));
    return NO_ERROR;
}

int32_t UiContentStub::UnregisterClickEventCallbackInner(
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    reply.WriteInt32(UnregisterClickEventCallback());
    return NO_ERROR;
}

int32_t UiContentStub::UnregisterSearchEventCallbackInner(
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    reply.WriteInt32(UnregisterSearchEventCallback());
    return NO_ERROR;
}

int32_t UiContentStub::UnregisterRouterChangeEventCallbackInner(
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    reply.WriteInt32(UnregisterRouterChangeEventCallback());
    return NO_ERROR;
}

int32_t UiContentStub::UnregisterComponentChangeEventCallbackInner(
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    reply.WriteInt32(UnregisterComponentChangeEventCallback());
    return NO_ERROR;
}
} // namespace OHOS::Ace
