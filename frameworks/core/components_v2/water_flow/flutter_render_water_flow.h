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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_WATER_FLOW_FLUTTER_RENDER_WATER_FLOW_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_WATER_FLOW_FLUTTER_RENDER_WATER_FLOW_H

#include "core/components_v2/water_flow/render_water_flow.h"
#include "core/pipeline/layers/clip_layer.h"

namespace OHOS::Ace::V2 {
class FlutterRenderWaterFlow : public RenderWaterFlow {
    DECLARE_ACE_TYPE(FlutterRenderWaterFlow, RenderWaterFlow);

public:
    FlutterRenderWaterFlow() = default;
    ~FlutterRenderWaterFlow() override = default;

    RenderLayer GetRenderLayer() override;
    void Paint(RenderContext& context, const Offset& offset) override;

private:
    RefPtr<Flutter::ClipLayer> layer_;
    ACE_DISALLOW_COPY_AND_MOVE(FlutterRenderWaterFlow);
};
} // namespace OHOS::Ace::V2
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_WATER_FLOW_FLUTTER_RENDER_WATER_FLOW_H
