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

#include "cj_router_ffi.h"

#include "bridge/cj_frontend/frontend/cj_frontend_abstract.h"
#include "bridge/cj_frontend/frontend/cj_page_router_abstract.h"
#include "bridge/cj_frontend/interfaces/cj_ffi/utils.h"
#include "core/common/container.h"
#include "core/components/page/page_target.h"

using namespace OHOS::Ace;
using namespace OHOS::Ace::Framework;

extern "C" {
void FfiOHOSAceFrameworkRouterPush(const char* url, const char* param)
{
    if (Container::CurrentId() < 0) {
        LOGE("RouterPush fail, no current container");
        return;
    }
    auto frontend = Utils::GetCurrentFrontend();
    if (!frontend) {
        LOGE("can not get frontend.");
        return;
    }
    frontend->PushPage(url, param);
}

void FfiOHOSAceFrameworkRouterBack(const char* url, const char* param)
{
    if (Container::CurrentId() < 0) {
        LOGE("RouterBack fail, no current container");
        return;
    }
    auto frontend = AceType::DynamicCast<CJFrontendAbstract>(Utils::GetCurrentFrontend());
    if (!frontend) {
        LOGE("can not get frontend.");
        return;
    }
    frontend->Back(url, param);
}

ExternalString FfiOHOSAceFrameworkRouterGetParams()
{
    if (Container::CurrentId() < 0) {
        LOGE("RouterGetParams fail, no current container");
        return {};
    }

    auto frontend = AceType::DynamicCast<CJFrontendAbstract>(Utils::GetCurrentFrontend());
    if (!frontend) {
        LOGE("can not get frontend.");
        return {};
    }

    return Utils::MallocCString(frontend->GetParams());
}
}
