/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_OHOS_OSAL_DRAWING_COLOR_FILTER_PREVIEW_H
#define FOUNDATION_ACE_ADAPTER_OHOS_OSAL_DRAWING_COLOR_FILTER_PREVIEW_H

#ifdef USE_ROSEN_DRAWING
#include "effect/color_filter.h"
#endif

#include "base/image/drawing_color_filter.h"

namespace OHOS::Ace {
class DrawingColorFilterPreview : public DrawingColorFilter {
public:
    explicit DrawingColorFilterPreview(std::shared_ptr<Rosen::Drawing::ColorFilter> colorFilter)
        : colorFilter_(std::move(colorFilter)) {}
    ~DrawingColorFilterPreview() override = default;
    void* GetDrawingColorFilterSptrAddr() override;

private:
    std::shared_ptr<Rosen::Drawing::ColorFilter> colorFilter_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_OHOS_OSAL_DRAWING_COLOR_FILTER_PREVIEW_H