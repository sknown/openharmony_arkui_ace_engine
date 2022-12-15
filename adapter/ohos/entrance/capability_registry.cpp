/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "adapter/ohos/entrance/capability_registry.h"

#include "adapter/ohos/capability/clipboard/clipboard_impl.h"
#include "core/common/clipboard/clipboard_proxy.h"
#include "core/common/storage/storage_proxy.h"
#include "adapter/ohos/capability/preference/storage_impl.h"

namespace OHOS::Ace {

void CapabilityRegistry::Register()
{
    ClipboardProxy::GetInstance()->SetDelegate(std::make_unique<ClipboardProxyImpl>());
    StorageProxy::GetInstance()->SetDelegate(std::make_unique<StorageProxyImpl>());
}

} // namespace OHOS::Ace
