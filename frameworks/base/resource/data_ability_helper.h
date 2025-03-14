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

#ifndef FOUNDATION_ACE_ACE_ENGINE_FRAMEWORKS_BASE_RESOURCE_DATA_ABILITY_HELPER_H
#define FOUNDATION_ACE_ACE_ENGINE_FRAMEWORKS_BASE_RESOURCE_DATA_ABILITY_HELPER_H

#include "base/memory/ace_type.h"

namespace OHOS::Ace {

class DataAbilityHelper : public AceType {
    DECLARE_ACE_TYPE(DataAbilityHelper, AceType)

public:
    DataAbilityHelper() = default;
    ~DataAbilityHelper() override = default;

    virtual int32_t OpenFile(const std::string& uriStr, const std::string& mode) = 0;
    virtual void* QueryThumbnailResFromDataAbility(const std::string& uri) = 0;
    virtual int32_t ReadMovingPhotoVideo(const std::string &uri) { return -1; }
    virtual std::string GetMovingPhotoImageUri(const std::string& uri) { return ""; }
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ACE_ENGINE_FRAMEWORKS_BASE_RESOURCE_DATA_ABILITY_HELPER_H
