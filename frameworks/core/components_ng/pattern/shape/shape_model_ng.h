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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SHAPE_MODEL_NG_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SHAPE_MODEL_NG_H

#include "base/geometry/dimension.h"
#include "base/utils/macros.h"
#include "core/components_ng/pattern/shape/shape_model.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT ShapeModelNG : public ShapeModel {
public:
    void Create() override;
    void SetBitmapMesh(std::vector<double>& mesh, int32_t column, int32_t row) override;
    void SetViewPort(const Dimension& dimLeft, const Dimension& dimTop, const Dimension& dimWidth,
        const Dimension& dimHeight) override;
    void InitBox(RefPtr<PixelMap>& pixMap) override {}
    void SetWidth(Dimension& width) override;
    void SetHeight(Dimension& height) override;
};

} // namespace OHOS::Ace::NG

#endif