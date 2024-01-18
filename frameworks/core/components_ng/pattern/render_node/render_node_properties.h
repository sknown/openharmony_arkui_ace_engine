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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RENDER_HODE_RENDER_NODE_PROPERTIES_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RENDER_HODE_RENDER_NODE_PROPERTIES_H

#include <cstdint>

namespace OHOS::Ace::NG {
const uint32_t SHAPE_MASK_DEFAULT_COLOR = 0xFF000000;

struct ShapeMaskProperty {
    uint32_t fillColor = SHAPE_MASK_DEFAULT_COLOR;
    uint32_t strokeColor = SHAPE_MASK_DEFAULT_COLOR;
    float strokeWidth = 0.0f;
};
} // namespace OHOS::Ace::NG

#endif
