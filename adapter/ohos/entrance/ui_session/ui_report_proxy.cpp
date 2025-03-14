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

#include "interfaces/inner_api/ui_session/ui_report_proxy.h"

#include "adapter/ohos/entrance/ui_session/include/ui_service_hilog.h"

namespace OHOS::Ace {
void UiReportProxy::ReportClickEvent(std::string data)
{
    MessageParcel messageData;
    MessageParcel reply;
    MessageOption option;
    if (!messageData.WriteInterfaceToken(GetDescriptor())) {
        LOGW("ReportClickEvent write interface token failed");
    }
    if (!messageData.WriteString(data)) {
        LOGW("ReportClickEvent write data failed");
    }
    if (Remote()->SendRequest(REPORT_CLICK_EVENT, messageData, reply, option) != ERR_NONE) {
        LOGW("ReportClickEvent send request failed");
    }
}

void UiReportProxy::ReportRouterChangeEvent(std::string data)
{
    MessageParcel messageData;
    MessageParcel reply;
    MessageOption option;
    if (!messageData.WriteInterfaceToken(GetDescriptor())) {
        LOGW("ReportRouterChangeEvent write interface token failed");
    }
    if (!messageData.WriteString(data)) {
        LOGW("ReportRouterChangeEvent write data failed");
    }
    if (Remote()->SendRequest(REPORT_SWITCH_EVENT, messageData, reply, option) != ERR_NONE) {
        LOGW("ReportRouterChangeEvent send request failed");
    }
}

void UiReportProxy::ReportComponentChangeEvent(std::string data)
{
    MessageParcel messageData;
    MessageParcel reply;
    MessageOption option;
    if (!messageData.WriteInterfaceToken(GetDescriptor())) {
        LOGW("ReportComponentChangeEvent write interface token failed");
    }
    if (!messageData.WriteString(data)) {
        LOGW("ReportComponentChangeEvent write data failed");
    }
    if (Remote()->SendRequest(REPORT_COMPONENT_EVENT, messageData, reply, option) != ERR_NONE) {
        LOGW("ReportComponentChangeEvent send request failed");
    }
}

void UiReportProxy::ReportSearchEvent(std::string data)
{
    MessageParcel messageData;
    MessageParcel reply;
    MessageOption option;
    if (!messageData.WriteInterfaceToken(GetDescriptor())) {
        LOGW("ReportSearchEvent write interface token failed");
    }
    if (!messageData.WriteString(data)) {
        LOGW("ReportSearchEvent write data failed");
    }
    if (Remote()->SendRequest(REPORT_SEARCH_EVENT, messageData, reply, option) != ERR_NONE) {
        LOGW("ReportSearchEvent send request failed");
    }
}
} // namespace OHOS::Ace
