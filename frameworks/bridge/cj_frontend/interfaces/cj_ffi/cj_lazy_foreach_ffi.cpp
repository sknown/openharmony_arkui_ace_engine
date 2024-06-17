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

#include "bridge/cj_frontend/interfaces/cj_ffi/cj_lazy_foreach_ffi.h"

#include "bridge/cj_frontend/cppview/native_view.h"
#include "bridge/cj_frontend/interfaces/cj_ffi/cj_native_view_ffi.h"
#include "bridge/cj_frontend/cppview/lazy_foreach.h"
#include "bridge/cj_frontend/cppview/lazy_foreach_func.h"
#include "ffi_remote_data.h"
#include "core/common/container.h"
#include "core/components_ng/base/view_stack_model.h"
#include "core/components_ng/syntax/lazy_for_each_model.h"

using namespace OHOS::Ace::Framework;
using namespace OHOS::Ace;
using namespace OHOS::FFI;

extern "C" {
void FfiOHOSAceFrameworkLazyForEachCreate(int64_t viewID, int64_t parentViewID, int64_t lazyForEachFuncsID)
{
    auto nativeParentView = FFIData::GetData<NativeView>(parentViewID);
    auto lazyForeachFunc = LazyForEachFuncs::Create<LazyForEachFuncs>(lazyForEachFuncsID);
    ViewStackModel::GetInstance()->ProcessViewId(std::to_string(viewID));
    auto builder = AceType::MakeRefPtr<CJLazyForEachBuilder>(std::move(lazyForeachFunc));
    LazyForEachModel::GetInstance()->Create(builder);
}

void FfiOHOSAceFrameworkLazyForEachPop()
{
    ViewStackModel::GetInstance()->PopContainer();
}
}
