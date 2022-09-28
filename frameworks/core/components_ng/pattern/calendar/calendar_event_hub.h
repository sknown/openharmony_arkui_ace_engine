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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CALENDAR_CALENDAR_EVENT_HUB_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CALENDAR_CALENDAR_EVENT_HUB_H

#include <string>

#include "base/memory/ace_type.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/gesture_event_hub.h"

namespace OHOS::Ace::NG {

using SelectedChangeEvent = std::function<void(const std::string&)>;
using RequestDataEvent = std::function<void(const std::string&)>;

class CalendarEventHub : public EventHub {
    DECLARE_ACE_TYPE(CalendarEventHub, EventHub)

public:
    CalendarEventHub() = default;
    ~CalendarEventHub() override = default;

    void SetSelectedChangeEvent(SelectedChangeEvent&& selectedChangeEvent)
    {
        changeEvent_ = std::move(selectedChangeEvent);
    }

    void UpdateSelectedChangeEvent(const std::string& info) const
    {
        if (changeEvent_) {
            changeEvent_(info);
        }
    }
    void SetOnRequestDataEvent(RequestDataEvent&& requestDataEvent)
    {
        requestDataEvent_ = std::move(requestDataEvent);
    }

    const RequestDataEvent& GetOnRequestDataEvent()
    {
        return requestDataEvent_;
    }

    void UpdateRequestDataEvent(const std::string& info) const
    {
        if (requestDataEvent_) {
            requestDataEvent_(info);
        }
    }

private:
    SelectedChangeEvent changeEvent_;
    RequestDataEvent requestDataEvent_;

    ACE_DISALLOW_COPY_AND_MOVE(CalendarEventHub);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CALENDAR_CALENDAR_EVENT_HUB_H