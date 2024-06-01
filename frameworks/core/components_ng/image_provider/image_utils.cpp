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
#include "core/components_ng/image_provider/image_utils.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"

namespace OHOS::Ace::NG {
void ImageUtils::PostTask(
    std::function<void()>&& task, TaskExecutor::TaskType taskType, const char* taskTypeName, PriorityType priorityType)
{
    auto taskExecutor = Container::CurrentTaskExecutor();
    if (!taskExecutor) {
        LOGE("taskExecutor is null when try post task to %{public}s", taskTypeName);
        return;
    }
    taskExecutor->PostTask(
        [task, id = Container::CurrentId()] {
            ContainerScope scope(id);
            CHECK_NULL_VOID(task);
            task();
        },
        taskType, std::string(taskTypeName), priorityType);
}

void ImageUtils::PostToUI(
    std::function<void()>&& task, const std::string& name, const int32_t containerId, PriorityType priorityType)
{
    ContainerScope scope(containerId);

    CHECK_NULL_VOID(task);
    ImageUtils::PostTask(std::move(task), TaskExecutor::TaskType::UI, name.c_str(), priorityType);
}

void ImageUtils::PostToBg(
    std::function<void()>&& task, const std::string& name, const int32_t containerId, PriorityType priorityType)
{
    ContainerScope scope(containerId);

    CHECK_NULL_VOID(task);
    ImageUtils::PostTask(std::move(task), TaskExecutor::TaskType::BACKGROUND, name.c_str(), priorityType);
}
} // namespace OHOS::Ace::NG
