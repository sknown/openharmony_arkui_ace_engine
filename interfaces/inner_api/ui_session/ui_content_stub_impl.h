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

#ifndef FOUNDATION_ACE_INTERFACE_UI_CONTENT_STUB_IMPL_H
#define FOUNDATION_ACE_INTERFACE_UI_CONTENT_STUB_IMPL_H

#include <singleton.h>

#include "iremote_object.h"
#include "ui_content_errors.h"
#include "ui_content_stub.h"

#include "base/utils/macros.h"

namespace OHOS::Ace {
class ACE_FORCE_EXPORT UIContentServiceStubImpl : public UiContentStub {
public:
    int32_t Connect() override
    {
        return 0;
    }
    int32_t GetInspectorTree() override;
    int32_t RegisterClickEventCallback(EventCallback eventCallback) override;
    int32_t RegisterRouterChangeEventCallback(EventCallback eventCallback) override;
    int32_t RegisterSearchEventCallback(EventCallback eventCallback) override;
    int32_t RegisterComponentChangeEventCallback(EventCallback eventCallback) override;
    int32_t UnregisterClickEventCallback() override;
    int32_t UnregisterSearchEventCallback() override;
    int32_t UnregisterRouterChangeEventCallback() override;
    int32_t UnregisterComponentChangeEventCallback() override;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_INTERFACE_UI_CONTENT_STUB_IMPL_H

