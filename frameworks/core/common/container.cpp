/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "core/common/container.h"

#include "base/utils/utils.h"
#include "core/common/ace_engine.h"
#include "core/common/container_scope.h"
#ifdef PLUGIN_COMPONENT_SUPPORTED
#include "core/common/plugin_manager.h"
#endif

namespace OHOS::Ace {

int32_t Container::CurrentId()
{
    return ContainerScope::CurrentId();
}

int32_t Container::SafelyId()
{
    uint32_t containerCount = ContainerScope::ContainerCount();
    if (containerCount == 0) {
        return INSTANCE_ID_UNDEFINED;
    }
    if (containerCount == 1) {
        return ContainerScope::SingletonId();
    }
    int32_t currentId = ContainerScope::RecentActiveId();
    if (currentId >= 0) {
        return currentId;
    }
    currentId = ContainerScope::RecentForegroundId();
    if (currentId >= 0) {
        return currentId;
    }
    return ContainerScope::DefaultId();
}

int32_t Container::CurrentIdSafely()
{
    int32_t currentId = ContainerScope::CurrentId();
    if (currentId >= 0) {
        return currentId;
    }
    return SafelyId();
}

RefPtr<Container> Container::Current()
{
    return AceEngine::Get().GetContainer(ContainerScope::CurrentId());
}

RefPtr<Container> Container::CurrentSafely()
{
    return AceEngine::Get().GetContainer(Container::CurrentIdSafely());
}

RefPtr<Container> Container::CurrentSafelyWithCheck()
{
    int32_t currentId = CurrentId();
    if (currentId >= 0) {
        auto container = GetContainer(currentId);
        if (container) {
            return container;
        }
    }
    currentId = SafelyId();
    return GetContainer(currentId);
}

RefPtr<Container> Container::GetContainer(int32_t containerId)
{
    return AceEngine::Get().GetContainer(containerId);
}

RefPtr<Container> Container::GetActive()
{
    RefPtr<Container> activeContainer;
    AceEngine::Get().NotifyContainers([&activeContainer](const RefPtr<Container>& container) {
        auto front = container->GetFrontend();
        if (front && front->IsForeground()) {
            activeContainer = container;
        }
    });
    return activeContainer;
}

RefPtr<Container> Container::GetDefault()
{
    RefPtr<Container> defaultContainer;
    AceEngine::Get().NotifyContainers([&defaultContainer](const RefPtr<Container>& container) {
        auto front = container->GetFrontend();
        if (front) {
            defaultContainer = container;
        }
    });
    return defaultContainer;
}

RefPtr<Container> Container::GetFoucsed()
{
    RefPtr<Container> foucsContainer;
    AceEngine::Get().NotifyContainers([&foucsContainer](const RefPtr<Container>& container) {
        auto pipeline = container->GetPipelineContext();
        if (pipeline && pipeline->GetOnFoucs()) {
            foucsContainer = container;
        }
    });
    return foucsContainer;
}

RefPtr<TaskExecutor> Container::CurrentTaskExecutor()
{
    auto curContainer = Current();
    CHECK_NULL_RETURN(curContainer, nullptr);
    return curContainer->GetTaskExecutor();
}

RefPtr<TaskExecutor> Container::CurrentTaskExecutorSafely()
{
    auto curContainer = CurrentSafely();
    CHECK_NULL_RETURN(curContainer, nullptr);
    return curContainer->GetTaskExecutor();
}

RefPtr<TaskExecutor> Container::CurrentTaskExecutorSafelyWithCheck()
{
    auto curContainer = CurrentSafelyWithCheck();
    CHECK_NULL_RETURN(curContainer, nullptr);
    return curContainer->GetTaskExecutor();
}

void Container::UpdateCurrent(int32_t id)
{
    ContainerScope::UpdateCurrent(id);
}

bool Container::UpdateState(const Frontend::State& state)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (state_ == state) {
        return false;
    }
    state_ = state;
    return true;
}

bool Container::Dump(const std::vector<std::string>& params, std::vector<std::string>& info)
{
    std::string tip("container not support, type:");
    tip.append(AceType::TypeName(this));
    info.emplace_back(tip);
    return true;
}

bool Container::IsIdAvailable(int32_t id)
{
    return !AceEngine::Get().GetContainer(id);
}

template<>
int32_t Container::GenerateId<PLUGIN_SUBCONTAINER>()
{
#ifdef PLUGIN_COMPONENT_SUPPORTED
    return PluginManager::GetInstance().GetPluginSubContainerId();
#else
    return INSTANCE_ID_UNDEFINED;
#endif
}

} // namespace OHOS::Ace
