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

#include "bridge/cj_frontend/interfaces/cj_ffi/cj_common_ffi.h"

#include "core/common/container.h"
#include "bridge/cj_frontend/runtime/cj_runtime_delegate.h"
#include "bridge/cj_frontend/interfaces/cj_ffi/utils.h"

using namespace OHOS::Ace;
using namespace OHOS::Ace::Framework;

extern "C" {
void FfiOHOSAceFrameworkRegisterCJFuncs(AtCPackage cjFuncs)
{
    CJRuntimeDelegate::GetInstance()->RegisterCJFuncs(cjFuncs);
}

int64_t FfiGeneralSizeOfPointer()
{
    return sizeof(void*);
}
}

namespace OHOS::Ace {
void TransformNativeTouchLocationInfo(
    CJTouchInfo* sources, const std::list<OHOS::Ace::TouchLocationInfo>& touchLocationInfoList)
{
    int id = 0;
    for (const OHOS::Ace::TouchLocationInfo& touchLocationInfo : touchLocationInfoList) {
        auto& ffiTouches = sources[id];
        ffiTouches.type = static_cast<uint8_t>(touchLocationInfo.GetTouchType());
        ffiTouches.fingerId = touchLocationInfo.GetFingerId();
        auto& globalLoc = touchLocationInfo.GetGlobalLocation();
        ffiTouches.globalX = PipelineBase::Px2VpWithCurrentDensity(globalLoc.GetX());
        ffiTouches.globalY = PipelineBase::Px2VpWithCurrentDensity(globalLoc.GetY());
        auto& localLoc = touchLocationInfo.GetLocalLocation();
        ffiTouches.localX = PipelineBase::Px2VpWithCurrentDensity(localLoc.GetX());
        ffiTouches.localY = PipelineBase::Px2VpWithCurrentDensity(localLoc.GetY());
        id++;
    }
}

void TransformNativeCJFingerInfo(CJFingerInfo* sources, const std::list<OHOS::Ace::FingerInfo>& fingerInfoList)
{
    int id = 0;
    for (const OHOS::Ace::FingerInfo& fingerInfo : fingerInfoList) {
        auto& ffiFingers = sources[id];
        ffiFingers.id = fingerInfo.fingerId_;
        auto& globalLoc = fingerInfo.globalLocation_;
        ffiFingers.globalX = PipelineBase::Px2VpWithCurrentDensity(globalLoc.GetX());
        ffiFingers.globalY = PipelineBase::Px2VpWithCurrentDensity(globalLoc.GetY());
        auto& localLoc = fingerInfo.localLocation_;
        ffiFingers.localX = PipelineBase::Px2VpWithCurrentDensity(localLoc.GetX());
        ffiFingers.localY = PipelineBase::Px2VpWithCurrentDensity(localLoc.GetY());
        id++;
    }
}
} // namespace OHOS::Ace
