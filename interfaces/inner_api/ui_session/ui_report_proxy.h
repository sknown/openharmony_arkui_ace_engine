/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_INTERFACE_UI_REPORT_PROXY_H
#define FOUNDATION_ACE_INTERFACE_UI_REPORT_PROXY_H

#include "iremote_proxy.h"
#include "ui_content_errors.h"
#include "ui_content_service_interface.h"

#include "base/utils/macros.h"

namespace OHOS::Ace {
class ACE_FORCE_EXPORT UiReportProxy : public IRemoteProxy<ReportService> {
public:
    explicit UiReportProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<ReportService>(impl) {};

    /**
     * @description: notify stub side to execute click callback
     */
    void ReportClickEvent(std::string data) override;

    /**
     * @description: notify stub side to execute switch callback
     */
    void ReportRouterChangeEvent(std::string data) override;

    /**
     * @description: notify stub side to execute component callback
     */
    void ReportComponentChangeEvent(std::string data) override;

    /**
     * @description: notify stub side to execute search callback
     */
    void ReportSearchEvent(std::string data) override;

private:
    static inline BrokerDelegator<UiReportProxy> delegator_;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_INTERFACE_UI_CONTENT_PROXY_H
