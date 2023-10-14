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

#include "navigator_event_hub.h"

#include "base/utils/utils.h"
#include "core/pipeline/pipeline_base.h"
#include "frameworks/bridge/common/utils/engine_helper.h"

namespace OHOS::Ace::NG {

void NavigatorEventHub::NavigatePage()
{
    auto delegate = EngineHelper::GetCurrentDelegate();
    CHECK_NULL_VOID(delegate);
    switch (type_) {
        case NavigatorType::PUSH:
            delegate->Push(url_, params_);
            break;
        case NavigatorType::REPLACE:
            delegate->Replace(url_, params_);
            break;
        case NavigatorType::BACK:
            delegate->Back(url_, params_);
            break;
        default:
            LOGE("Navigator type is invalid!");
    }
    LOGD("navigate success");
}

void NavigatorEventHub::SetActive(bool active)
{
    if (active) {
        auto pipelineContext = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        pipelineContext->GetTaskExecutor()->PostTask(
            [weak = WeakClaim(this)] {
                auto eventHub = weak.Upgrade();
                CHECK_NULL_VOID(eventHub);
                eventHub->NavigatePage();
            },
            TaskExecutor::TaskType::JS);
    }
    active_ = active;
}

std::string NavigatorEventHub::GetNavigatorType() const
{
    switch (type_) {
        case NavigatorType::PUSH:
            return "NavigationType.Push";
        case NavigatorType::BACK:
            return "NavigationType.Back";
        case NavigatorType::DEFAULT:
            return "NavigationType.Default";
        case NavigatorType::REPLACE:
            return "NavigationType.Replace";
        default:
            return "NavigationType.Push";
    }
}

void NavigatorEventHub::ToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    json->Put("active", active_ ? "true" : "false");
    json->Put("target", url_.c_str());
    json->Put("type", GetNavigatorType().c_str());
    json->Put("params", params_.c_str());
}

} // namespace OHOS::Ace::NG
