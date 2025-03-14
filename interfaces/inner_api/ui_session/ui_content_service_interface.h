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

#ifndef FOUNDATION_ACE_INTERFACE_UI_CONTENT_SERVICE_INTERFACE_H
#define FOUNDATION_ACE_INTERFACE_UI_CONTENT_SERVICE_INTERFACE_H
#include <iremote_broker.h>

#include "base/utils/macros.h"

namespace OHOS::Ace {
class ACE_FORCE_EXPORT IUiContentService : public OHOS::IRemoteBroker {
public:
    using EventCallback = std::function<void(std::string)>;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ace.UIContentService");
    IUiContentService() = default;
    ~IUiContentService() override = default;
    enum {
        UI_CONTENT_SERVICE_GET_TREE,
        UI_CONTENT_CONNECT,
        REGISTER_CLICK_EVENT,
        REGISTER_SEARCH_EVENT,
        REGISTER_ROUTER_CHANGE_EVENT,
        REGISTER_COMPONENT_EVENT,
        UNREGISTER_CLICK_EVENT,
        UNREGISTER_SEARCH_EVENT,
        UNREGISTER_ROUTER_CHANGE_EVENT,
        UNREGISTER_COMPONENT_EVENT,

    };

    /**
     * @description: define get the current page inspector tree info interface
     * @return: result number
     */
    virtual int32_t GetInspectorTree() = 0;

    /**
     * @description: define SA process and current process connect interface
     * @return: result number
     */
    virtual int32_t Connect() = 0;

    /**
     * @description: define register a callback on click event occur to execute interface
     * @return: result number
     */
    virtual int32_t RegisterClickEventCallback(EventCallback eventCallback) = 0;

    /**
     * @description: define register a callback on switch event occur to execute interface
     * @return: result number
     */
    virtual int32_t RegisterRouterChangeEventCallback(EventCallback eventCallback) = 0;

    /**
     * @description: define register a callback on search event occur to execute interface
     * @return: result number
     */
    virtual int32_t RegisterSearchEventCallback(EventCallback eventCallback) = 0;

    /**
     * @description: define register a callback on component event occur to execute interface
     * @return: result number
     */
    virtual int32_t RegisterComponentChangeEventCallback(EventCallback eventCallback) = 0;

    /**
     * @description: define unregister the click event occur callback last register interface
     * @return: result number
     */
    virtual int32_t UnregisterClickEventCallback() = 0;

    /**
     * @description: define unregister the search event occur callback last register interface
     * @return: result number
     */
    virtual int32_t UnregisterSearchEventCallback() = 0;

    /**
     * @description: define unregister the switch event occur callback last register interface
     * @return: result number
     */
    virtual int32_t UnregisterRouterChangeEventCallback() = 0;

    /**
     * @description: define unregister the component event occur callback last register interface
     * @return: result number
     */
    virtual int32_t UnregisterComponentChangeEventCallback() = 0;
};
class ACE_FORCE_EXPORT ReportService : public OHOS::IRemoteBroker {
public:
    using EventCallback = std::function<void(std::string)>;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ace.ReportService");
    ReportService() = default;
    ~ReportService() override = default;
    enum { REPORT_CLICK_EVENT, REPORT_SWITCH_EVENT, REPORT_COMPONENT_EVENT, REPORT_SEARCH_EVENT };

    /**
     * @description: define reports the click event to the proxy interface
     */
    virtual void ReportClickEvent(std::string data) = 0;

    /**
     * @description: define reports the switch event to the proxy interface
     */
    virtual void ReportRouterChangeEvent(std::string data) = 0;

    /**
     * @description: define reports the component change event to the proxy interface
     */
    virtual void ReportComponentChangeEvent(std::string data) = 0;

    /**
     * @description: define reports the search event to the proxy interface
     */
    virtual void ReportSearchEvent(std::string data) = 0;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_INTERFACE_UI_CONTENT_SERVICE_INTERFACE_H
