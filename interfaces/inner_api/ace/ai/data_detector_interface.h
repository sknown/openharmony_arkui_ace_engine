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

#ifndef FOUNDATION_ACE_INTERFACE_INNERKITS_DATA_DETECTOR_INTERFACE_H
#define FOUNDATION_ACE_INTERFACE_INNERKITS_DATA_DETECTOR_INTERFACE_H

#include <string>
#include <functional>

namespace OHOS::Ace {
struct TextDataDetectInfo {
    std::string text;
    std::string module;
    bool isWordPosEnabled = false;
};

struct TextDataDetectResult {
    int32_t code = 0;
    std::string entity;
    std::string wordPos;
    std::string menuOption;
};

using TextDetectResultFunc = std::function<void(const TextDataDetectResult)>;

class DataDetectorInterface {
public:
    virtual bool IsDataDetectorSupported() = 0;
    virtual void DataDetect(const TextDataDetectInfo& info, const TextDetectResultFunc& resultFunc) = 0;
protected:
    virtual ~DataDetectorInterface() {}
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_INTERFACE_INNERKITS_DATA_DETECTOR_INTERFACE_H
