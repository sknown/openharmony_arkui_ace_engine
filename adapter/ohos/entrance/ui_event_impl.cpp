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

#include "adapter/ohos/entrance/ui_event_impl.h"

#include <dlfcn.h>
#include <mutex>
#include <string>
#include <unordered_map>

#include "interfaces/inner_api/ace/ui_event_observer.h"

#include "base/log/log.h"
#include "base/thread/background_task_executor.h"
#include "core/common/container.h"
#include "core/common/recorder/event_controller.h"
#include "core/common/recorder/event_recorder.h"
#include "core/common/recorder/node_data_cache.h"
#include "core/components_ng/base/inspector.h"

namespace OHOS::Ace {
extern "C" ACE_FORCE_EXPORT void OHOS_ACE_RegisterUIEventObserver(
    const std::string& config, const std::shared_ptr<UIEventObserver>& observer)
{
    TAG_LOGI(AceLogTag::ACE_UIEVENT, "RegisterUIEventObserver");
    Recorder::EventController::Get().Register(config, observer);
}

extern "C" ACE_FORCE_EXPORT void OHOS_ACE_UnregisterUIEventObserver(const std::shared_ptr<UIEventObserver>& observer)
{
    TAG_LOGI(AceLogTag::ACE_UIEVENT, "UnregisterUIEventObserver.");
    Recorder::EventController::Get().Unregister(observer);
}

extern "C" ACE_FORCE_EXPORT void OHOS_ACE_GetNodeProperty(
    const std::string& pageUrl, std::unordered_map<std::string, std::string>& nodeProperties)
{
    Recorder::NodeDataCache::Get().GetNodeData(pageUrl, nodeProperties);
}

extern "C" ACE_FORCE_EXPORT void OHOS_ACE_GetSimplifiedInspectorTree(std::string& tree)
{
    auto containerId = Recorder::EventRecorder::Get().GetContainerId();
    auto container = Container::GetContainer(containerId);
    if (!container) {
        return;
    }
    if (container->IsUseNewPipeline()) {
        tree = NG::Inspector::GetSimplifiedInspector(containerId);
    }
}

namespace Recorder {
constexpr char HA_CLIENT_SO_PATH[] = "libha_ace_engine.z.so";

static bool g_loaded = false;
static void* g_handle = nullptr;
static std::once_flag g_loadFlag;

void InitHandler()
{
    if (g_handle) {
        return;
    }
    TAG_LOGI(AceLogTag::ACE_UIEVENT, "report ace loaded");
    auto handle = dlopen(HA_CLIENT_SO_PATH, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGI(AceLogTag::ACE_UIEVENT, "Failed to open shared library %{public}s, reason: %{public}sn",
            HA_CLIENT_SO_PATH, dlerror());
        return;
    }
    g_handle = handle;
}

void Init()
{
    if (g_loaded) {
        return;
    }
    std::call_once(g_loadFlag, [] { InitHandler(); });
    g_loaded = true;
}

void DeInit()
{
    if (g_handle) {
        dlclose(g_handle);
        g_handle = nullptr;
        g_loaded = false;
    }
}
} // namespace OHOS::Ace::Recorder
} // namespace OHOS::Ace
