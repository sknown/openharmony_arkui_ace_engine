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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_FIELD_TEXT_FIELD_EVENT_HUB_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_FIELD_TEXT_FIELD_EVENT_HUB_H

#include <cstdint>
#include <utility>

#include "base/memory/ace_type.h"
#include "base/utils/noncopyable.h"
#include "core/components_ng/event/event_hub.h"

namespace OHOS::Ace::NG {

using ChangeEvent = std::function<void(const std::string, const std::string)>;

class TextFieldEventHub : public EventHub {
    DECLARE_ACE_TYPE(TextFieldEventHub, EventHub)

public:
    TextFieldEventHub() = default;
    ~TextFieldEventHub() override = default;

    void SetOnInputFilterError(const std::function<void(const std::string&)>& onInputFilterError)
    {
        onInputFilterError_ = onInputFilterError;
    }

    void FireOnInputFilterError(const std::string& value) const
    {
        if (onInputFilterError_) {
            onInputFilterError_(value);
        }
    }

    void SetOnEditChanged(std::function<void(bool)>&& func)
    {
        onEditChanged_ = std::move(func);
    }

    void FireOnEditChanged(bool value)
    {
        if (onEditChanged_) {
            onEditChanged_(value);
        }
    }

    void SetOnSubmit(std::function<void(int32_t)>&& func)
    {
        onSubmit_ = std::move(func);
    }

    void FireOnSubmit(int32_t value)
    {
        if (onSubmit_) {
            onSubmit_(value);
        }
    }

    void SetOnChange(std::function<void(const std::string&)>&& func)
    {
        onChange_ = std::move(func);
    }

    void FireOnChange(const std::string& value)
    {
        if (onChange_) {
            onChange_(value);
        }
    }

private:
    std::function<void(const std::string&)> onInputFilterError_;
    std::function<void(bool)> onEditChanged_;
    std::function<void(int32_t)> onSubmit_;
    std::function<void(const std::string&)> onChange_;

    ACE_DISALLOW_COPY_AND_MOVE(TextFieldEventHub);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_FIELD_TEXT_FIELD_EVENT_HUB_H