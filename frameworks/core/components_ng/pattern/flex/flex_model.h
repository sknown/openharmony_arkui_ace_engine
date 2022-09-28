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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_FLEX_FLEX_MODEL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_FLEX_FLEX_MODEL_H

#include <memory>

#include "base/utils/macros.h"

namespace OHOS::Ace {

class ACE_EXPORT FlexModel {
public:
    static FlexModel* GetInstance();
    virtual ~FlexModel() = default;

    virtual void SetFillParent() = 0;

    virtual void SetWrapContent() = 0;

    virtual void SetJustifyContent(int32_t value) = 0;

    virtual void SetAlignItems(int32_t value) = 0;

    virtual void SetAlignContent(int32_t value) = 0;

    virtual void SetHasHeight() = 0;

    virtual void SetHasWidth() = 0;

private:
    static std::unique_ptr<FlexModel> instance_;
};

} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_FLEX_FLEX_MODEL_H
