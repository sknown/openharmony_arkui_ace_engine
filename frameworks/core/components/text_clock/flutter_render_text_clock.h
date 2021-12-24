/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_CLOCK_FLUTTER_RENDER_TEXT_CLOCK_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_CLOCK_FLUTTER_RENDER_TEXT_CLOCK_H

#include "core/components/text_clock/render_text_clock.h"
#include "core/pipeline/layers/offset_layer.h"

namespace OHOS::Ace {
class FlutterRenderTextClock final : public RenderTextClock {
    DECLARE_ACE_TYPE(FlutterRenderTextClock, RenderTextClock);

public:
    FlutterRenderTextClock() = default;
    ~FlutterRenderTextClock() override = default;

    void Paint(RenderContext& context, const Offset& offset) override;
    void RequestRenderForNextSecond();

    bool IsRepaintBoundary() const override
    {
        return true;
    }
    RenderLayer GetRenderLayer() override
    {
        if (IsRepaintBoundary()) {
            if (!layer_) {
                layer_ = AceType::MakeRefPtr<Flutter::OffsetLayer>();
            }
        } else {
            layer_ = nullptr;
        }
        return AceType::RawPtr(layer_);
    }

private:
    RefPtr<Flutter::OffsetLayer> layer_;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_CLOCK_FLUTTER_RENDER_TEXT_CLOCK_H
