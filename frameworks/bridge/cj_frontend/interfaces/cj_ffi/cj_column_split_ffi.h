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

#ifndef OHOS_ACE_FRAMEWORK_FFI_CJ_COLUMN_SPLIT_H
#define OHOS_ACE_FRAMEWORK_FFI_CJ_COLUMN_SPLIT_H

#include <cstdint>

#include "bridge/cj_frontend/interfaces/cj_ffi/cj_macro.h"

extern "C" {
struct DividerParam {
    double startMargin;
    int32_t startMarginUnit;
    double endMargin;
    int32_t endMarginUnit;
};

CJ_EXPORT void FfiOHOSAceFrameworkColumnSplitCreate();
CJ_EXPORT void FfiOHOSAceFrameworkColumnSplitResizeable(bool resizeable);
CJ_EXPORT void FfiOHOSAceFrameworkColumnSplitDivider(DividerParam params);
CJ_EXPORT void FfiOHOSAceFrameworkColumnSplitClip(bool isClip);
CJ_EXPORT void FfiOHOSAceFrameworkColumnSplitClipByShape(int64_t shapeId);
}

#endif // OHOS_ACE_FRAMEWORK_FFI_CJ_COLUMN_SPLIT_H
