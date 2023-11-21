/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "core/common/ai/data_detector_mgr.h"
#include "core/common/ai/data_detector_default.h"
namespace OHOS::Ace {
namespace {
#ifdef __aarch64__
const std::string AI_ADAPTER_SO_PATH = "system/lib64/libai_text_analyzer_innerapi.z.so";
#else
const std::string AI_ADAPTER_SO_PATH = "system/lib/libai_text_analyzer_innerapi.z.so";
#endif
}

DataDetectorMgr& DataDetectorMgr::GetInstance()
{
    static DataDetectorMgr instance;
    return instance;
}

DataDetectorMgr::DataDetectorMgr()
{
    auto lib = DataDetectorLoader::Load(AI_ADAPTER_SO_PATH);
    if (lib == nullptr || (engine_ = lib->CreateDataDetector()) == nullptr) {
        engine_ = DataDetectorInstance(new DataDetectorDefault, [](DataDetectorInterface* e) {
            auto *p = reinterpret_cast<DataDetectorDefault*>(e);
            delete p;
        });
    }
}

bool DataDetectorMgr::IsDataDetectorSupported()
{
    if (engine_) {
        return engine_->IsDataDetectorSupported();
    }
    return false;
}

void DataDetectorMgr::DataDetect(const TextDataDetectInfo& info, const TextDetectResultFunc& resultFunc)
{
    if (engine_) {
        engine_->DataDetect(info, resultFunc);
    }
}
} // namespace OHOS::Ace
