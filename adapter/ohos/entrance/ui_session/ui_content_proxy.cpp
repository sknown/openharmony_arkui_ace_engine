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

#include "interfaces/inner_api/ui_session/ui_content_proxy.h"

#include "ipc_skeleton.h"

#include "adapter/ohos/entrance/ui_session/include/ui_service_hilog.h"

namespace OHOS::Ace {

int32_t UIContentServiceProxy::GetInspectorTree()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGW("GetInspectorTree write interface token failed");
        return FAILED;
    }
    if (Remote()->SendRequest(UI_CONTENT_SERVICE_GET_TREE, data, reply, option) != ERR_NONE) {
        LOGW("GetInspectorTree send request failed");
        return REPLY_ERROR;
    }
    return NO_ERROR;
}

int32_t UIContentServiceProxy::Connect()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGW("connect write interface token failed");
        return FAILED;
    }
    report_ = new (std::nothrow) UiReportStub();
    processId_ = IPCSkeleton::GetCallingRealPid();
    if (report_ == nullptr) {
        LOGW("connect failed,create reportStub failed");
        return FAILED;
    }
    if (!data.WriteRemoteObject(report_)) {
        LOGW("write reportStub failed");
        return FAILED;
    }
    if (!data.WriteInt32(processId_)) {
        LOGW("write processId failed");
        return FAILED;
    }
    if (Remote()->SendRequest(UI_CONTENT_CONNECT, data, reply, option) != ERR_NONE) {
        LOGW("connect send request failed");
        return REPLY_ERROR;
    }
    return NO_ERROR;
}

int32_t UIContentServiceProxy::RegisterClickEventCallback(EventCallback eventCallback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGW("RegisterClickEventCallback write interface token failed");
        return FAILED;
    }
    if (report_ == nullptr) {
        LOGW("reportStub is nullptr,connect is not execute");
    }
    report_->RegisterClickEventCallback(eventCallback);
    if (Remote()->SendRequest(REGISTER_CLICK_EVENT, data, reply, option) != ERR_NONE) {
        LOGW("RegisterClickEventCallback send request failed");
        return REPLY_ERROR;
    }
    return NO_ERROR;
}

int32_t UIContentServiceProxy::RegisterSearchEventCallback(EventCallback eventCallback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGW("RegisterSearchEventCallback write interface token failed");
        return FAILED;
    }
    if (report_ == nullptr) {
        LOGW("reportStub is nullptr,connect is not execute");
    }
    report_->RegisterSearchEventCallback(eventCallback);
    if (Remote()->SendRequest(REGISTER_SEARCH_EVENT, data, reply, option) != ERR_NONE) {
        LOGW("RegisterSearchEventCallback send request failed");
        return REPLY_ERROR;
    }
    return NO_ERROR;
}

int32_t UIContentServiceProxy::RegisterRouterChangeEventCallback(EventCallback eventCallback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGW("RegisterRouterChangeEventCallback write interface token failed");
        return FAILED;
    }
    if (report_ == nullptr) {
        LOGW("reportStub is nullptr,connect is not execute");
    }
    report_->RegisterRouterChangeEventCallback(eventCallback);
    if (Remote()->SendRequest(UNREGISTER_ROUTER_CHANGE_EVENT, data, reply, option) != ERR_NONE) {
        LOGW("RegisterRouterChangeEventCallback send request failed");
        return REPLY_ERROR;
    }
    return NO_ERROR;
}

int32_t UIContentServiceProxy::RegisterComponentChangeEventCallback(EventCallback eventCallback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGW("RegisterComponentChangeEventCallback write interface token failed");
        return FAILED;
    }
    if (report_ == nullptr) {
        LOGW("reportStub is nullptr,connect is not execute");
    }
    report_->RegisterComponentChangeEventCallback(eventCallback);
    if (Remote()->SendRequest(REGISTER_COMPONENT_EVENT, data, reply, option) != ERR_NONE) {
        LOGW("RegisterComponentChangeEventCallback send request failed");
        return REPLY_ERROR;
    }
    return NO_ERROR;
}

int32_t UIContentServiceProxy::UnregisterClickEventCallback()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGW("UnregisterClickEventCallback write interface token failed");
        return FAILED;
    }
    if (report_ == nullptr) {
        LOGW("reportStub is nullptr,connect is not execute");
    }
    report_->UnregisterClickEventCallback();
    if (Remote()->SendRequest(UNREGISTER_CLICK_EVENT, data, reply, option) != ERR_NONE) {
        LOGW("UnregisterClickEventCallback send request failed");
        return REPLY_ERROR;
    }
    return NO_ERROR;
}

int32_t UIContentServiceProxy::UnregisterSearchEventCallback()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGW("UnregisterSearchEventCallback write interface token failed");
        return FAILED;
    }
    if (report_ == nullptr) {
        LOGW("reportStub is nullptr,connect is not execute");
    }
    report_->UnregisterSearchEventCallback();
    if (Remote()->SendRequest(UNREGISTER_SEARCH_EVENT, data, reply, option) != ERR_NONE) {
        LOGW("UnregisterSearchEventCallback send request failed");
        return REPLY_ERROR;
    }
    return NO_ERROR;
}

int32_t UIContentServiceProxy::UnregisterRouterChangeEventCallback()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGW("UnregisterRouterChangeEventCallback write interface token failed");
        return FAILED;
    }
    if (report_ == nullptr) {
        LOGW("reportStub is nullptr,connect is not execute");
    }
    report_->UnregisterRouterChangeEventCallback();
    if (Remote()->SendRequest(UNREGISTER_ROUTER_CHANGE_EVENT, data, reply, option) != ERR_NONE) {
        LOGW("UnregisterRouterChangeEventCallback send request failed");
        return REPLY_ERROR;
    }
    return NO_ERROR;
}

int32_t UIContentServiceProxy::UnregisterComponentChangeEventCallback()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGW("UnregisterComponentChangeEventCallback write interface token failed");
        return FAILED;
    }
    if (report_ == nullptr) {
        LOGW("reportStub is nullptr,connect is not execute");
    }
    report_->UnregisterComponentChangeEventCallback();
    if (Remote()->SendRequest(UNREGISTER_COMPONENT_EVENT, data, reply, option) != ERR_NONE) {
        LOGW("UnregisterComponentChangeEventCallback send request failed");
        return REPLY_ERROR;
    }
    return NO_ERROR;
}
} // namespace OHOS::Ace
