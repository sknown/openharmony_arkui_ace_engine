/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/components/video/resource/ext_surface.h"

#include <sstream>

#include "base/log/log.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"

namespace OHOS::Ace {

const char SURFACE_ERRORCODE_CREATEFAIL[] = "error_video_000001";
const char SURFACE_ERRORMSG_CREATEFAIL[] = "Unable to initialize video player.";

const char SURFACE_METHOD_ONCREATE[] = "onCreate";

const char SET_SURFACE_BOUNDS[] = "setSurfaceBounds";
const char SURFACE_ID[] = "surfaceId";
const char SURFACE_LEFT[] = "surfaceLeft";
const char SURFACE_TOP[] = "surfaceTop";
const char SURFACE_HEIGHT[] = "surfaceHeight";
const char SURFACE_WIDTH[] = "surfaceWidth";

ExtSurface::~ExtSurface()
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto resRegister = context->GetPlatformResRegister();
    CHECK_NULL_VOID(resRegister);
    auto platformTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::PLATFORM);
    if (platformTaskExecutor.IsRunOnCurrentThread()) {
        resRegister->UnregisterEvent(MakeEventHash(SURFACE_METHOD_ONCREATE));
    } else {
        WeakPtr<PlatformResRegister> weak = resRegister;
        platformTaskExecutor.PostTask([eventHash = MakeEventHash(SURFACE_METHOD_ONCREATE), weak] {
            auto resRegister = weak.Upgrade();
            CHECK_NULL_VOID(resRegister);
            resRegister->UnregisterEvent(eventHash);
        });
    }
}

void ExtSurface::Create(const std::function<void(int64_t)>& onCreate)
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);

    auto platformTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::PLATFORM);
    platformTaskExecutor.PostTask([weak = WeakClaim(this), onCreate] {
        auto surface = weak.Upgrade();
        if (surface) {
            surface->CreateExtSurface(onCreate);
        }
    });
}

void ExtSurface::CreateExtSurface(const std::function<void(int64_t)>& onCreate)
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto resRegister = context->GetPlatformResRegister();
    CHECK_NULL_VOID(resRegister);
    id_ = resRegister->CreateResource(type_, PARAM_NONE);
    if (id_ == INVALID_ID) {
        if (onError_) {
            onError_(SURFACE_ERRORCODE_CREATEFAIL, SURFACE_ERRORMSG_CREATEFAIL);
        }
        return;
    }
    hash_ = MakeResourceHash();
    resRegister->RegisterEvent(
        MakeEventHash(SURFACE_METHOD_ONCREATE), [weak = WeakClaim(this)](const std::string& param) {
            auto surface = weak.Upgrade();
            if (surface) {
                surface->OnSurfaceCreated();
            }
        });

    if (onCreate) {
        onCreate(id_);
    }
#if defined(IOS_PLATFORM)
    CallResRegisterMethod(MakeMethodHash(SURFACE_METHOD_ONCREATE), "");
#endif
}

void ExtSurface::SetBounds(int64_t surfaceId, int32_t left, int32_t top, int32_t width, int32_t height)
{
    std::stringstream paramStream;
    paramStream << SURFACE_ID << PARAM_EQUALS << surfaceId << PARAM_AND << SURFACE_LEFT << PARAM_EQUALS << left
                << PARAM_AND << SURFACE_TOP << PARAM_EQUALS << top << PARAM_AND << SURFACE_WIDTH << PARAM_EQUALS
                << width << PARAM_AND << SURFACE_HEIGHT << PARAM_EQUALS << height;
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(SET_SURFACE_BOUNDS), param);
}

void ExtSurface::OnSurfaceCreated()
{
    if (onSurfaceCreated_) {
        onSurfaceCreated_();
    }
}

} // namespace OHOS::Ace