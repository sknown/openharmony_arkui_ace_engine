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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RADIO_RADIO_MODEL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RADIO_RADIO_MODEL_H

#include "core/components_ng/pattern/radio/radio_event_hub.h"

namespace OHOS::Ace {

class RadioModel {
public:
    static RadioModel* GetInstance();
    virtual ~RadioModel() = default;

    virtual void Create(const std::optional<std::string>& value, const std::optional<std::string>& group);
    virtual void SetChecked(bool isChecked);
    virtual void SetOnChange(NG::ChangeEvent&& onChange);
    virtual void SetWidth(const Dimension& width);
    virtual void SetHeight(const Dimension& height);
    virtual void SetPadding(const NG::PaddingPropertyF& args);

private:
    static std::unique_ptr<RadioModel> instance_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RADIO_RADIO_MODEL_H
