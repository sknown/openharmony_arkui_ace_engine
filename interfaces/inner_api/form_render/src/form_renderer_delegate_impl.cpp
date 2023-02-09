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
#include "form_renderer_delegate_impl.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace Ace {
int32_t FormRendererDelegateImpl::OnSurfaceCreate(
    const std::shared_ptr<Rosen::RSSurfaceNode>& surfaceNode,
    const OHOS::AppExecFwk::FormJsInfo& formJsInfo,
    const AAFwk::Want& want)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (!surfaceNode) {
        HILOG_ERROR("surface is invalid");
        return ERR_NULL_OBJECT;
    }
    int64_t formId = formJsInfo.formId;
    if (formId < 0) {
        HILOG_ERROR("%{public}s error, the passed form id can't be negative.", __func__);
        return ERR_INVALID_DATA;
    }

    if (!surfaceCreateEventHandler_) {
        HILOG_ERROR("surfaceCreateEventHandler_ is null");
        return ERR_INVALID_DATA;
    }
    surfaceCreateEventHandler_(surfaceNode, formJsInfo, want);
    return ERR_OK;
}

int32_t FormRendererDelegateImpl::OnActionEvent(const std::string& action)
{
    HILOG_INFO("OnActionEvent %{public}s", action.c_str());
    if (!actionEventHandler_) {
        HILOG_ERROR("actionEventHandler_ is null,  %{public}s", action.c_str());
        return ERR_INVALID_DATA;
    }

    actionEventHandler_(action);
    return ERR_OK;
}

int32_t FormRendererDelegateImpl::OnError(const std::string& code, const std::string& msg)
{
    HILOG_INFO("OnError code: %{public}s, msg: %{public}s", code.c_str(), msg.c_str());
    if (!errorEventHandler_) {
        HILOG_ERROR("errorEventHandler_ is null");
        return ERR_INVALID_DATA;
    }

    errorEventHandler_(code, msg);
    return ERR_OK;
}

void FormRendererDelegateImpl::SetSurfaceCreateEventHandler(
    std::function<void(const std::shared_ptr<Rosen::RSSurfaceNode>&, const OHOS::AppExecFwk::FormJsInfo&,
        const AAFwk::Want&)>&& listener)
{
    surfaceCreateEventHandler_ = std::move(listener);
}

void FormRendererDelegateImpl::SetActionEventHandler(
    std::function<void(const std::string&)>&& listener)
{
    actionEventHandler_ = std::move(listener);
}

void FormRendererDelegateImpl::SetErrorEventHandler(
    std::function<void(const std::string&, const std::string&)>&& listener)
{
    errorEventHandler_ = std::move(listener);
}
} // namespace Ace
} // namespace OHOS
