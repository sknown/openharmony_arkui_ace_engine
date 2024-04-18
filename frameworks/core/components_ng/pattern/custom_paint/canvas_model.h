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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CUSTOM_PAINT_CANVAS_MODEL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CUSTOM_PAINT_CANVAS_MODEL_H

#include <mutex>

#include "core/components_ng/pattern/custom_paint/custom_paint_pattern.h"

namespace OHOS::Ace {
class CanvasModel {
public:
    static CanvasModel* GetInstance();
    static CanvasModel* GetInstanceNG();
    virtual ~CanvasModel() = default;

    virtual RefPtr<AceType> Create() = 0;
    virtual RefPtr<AceType> GetTaskPool(RefPtr<AceType>& pattern) = 0;
    virtual void PushCanvasPattern(RefPtr<AceType>& canvasPattern) {};
    virtual void SetOnReady(std::function<void(uint32_t)>&& onReady) {};
    virtual void SetOnReady(std::function<void()>&& onReady) {};
    virtual void EnableAnalyzer(bool enable) {};

private:
    static std::unique_ptr<CanvasModel> instance_;
    static std::mutex mutex_;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CUSTOM_PAINT_CANVAS_MODEL_H
