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

#ifndef FOUNDATION_ACE_FRAMEWORKS_COMMON_THREAD_CONTAINER_H
#define FOUNDATION_ACE_FRAMEWORKS_COMMON_THREAD_CONTAINER_H

#include "core/common/task_runner_adapter.h"

namespace OHOS::Ace {
class ThreadContainer {
public:
    enum ThreadType {
        Platform = 1 << 0,
        UI = 1 << 1,
        GPU = 1 << 2,
        IO = 1 << 3,
    };

    ThreadContainer() = default;
    ThreadContainer(ThreadContainer&&) = default;
    ThreadContainer(std::string namePrefix, uint64_t typeMask);
    ~ThreadContainer() = default;
    ThreadContainer& operator=(ThreadContainer&&) = default;

    RefPtr<TaskRunnerAdapter> platformThread;
    RefPtr<TaskRunnerAdapter> uiThread;
    RefPtr<TaskRunnerAdapter> gpuThread;
    RefPtr<TaskRunnerAdapter> ioThread;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_COMMON_THREAD_CONTAINER_H
