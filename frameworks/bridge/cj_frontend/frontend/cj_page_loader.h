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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BRIDGE_CJ_FRONTEND_CJ_PAGE_LOADER_H
#define FOUNDATION_ACE_FRAMEWORKS_BRIDGE_CJ_FRONTEND_CJ_PAGE_LOADER_H

#include "bridge/cj_frontend/cppview/native_view.h"

namespace OHOS::Ace::Framework {
bool LoadNativeViewNG(NativeView* view);
bool LoadNativeViewClassic(NativeView* view);
}
#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_CJ_FRONTEND_CJ_PAGE_LOADER_H
