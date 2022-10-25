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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_ADAPTER_MODIFIER_ADAPTER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_ADAPTER_MODIFIER_ADAPTER_H

#include <functional>
#include <memory>
#include <vector>

#include "include/core/SkCanvas.h"
#include "modifier/rs_property.h"
#include "render_service_client/core/modifier/rs_extended_modifier.h"
#include "render_service_client/core/modifier/rs_modifier.h"
#include "render_service_client/core/ui/rs_node.h"

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/modifier.h"
#include "core/components_ng/render/drawing.h"

namespace OHOS::Ace::NG {

using RSModifier = Rosen::RSModifier;
using RSNode = Rosen::RSNode;
using RSAnimationTimingProtocol = Rosen::RSAnimationTimingProtocol;
using RSAnimationTimingCurve = Rosen::RSAnimationTimingCurve;
template<typename T>
using RSAnimatableProperty = Rosen::RSAnimatableProperty<T>;
template<typename T>
using RSAnimatableArithmetic = Rosen::RSAnimatableArithmetic<T>;
using RSContentStyleModifier = Rosen::RSContentStyleModifier;
using RSOverlayStyleModifier = Rosen::RSOverlayStyleModifier;
using RSDrawingContext = Rosen::RSDrawingContext;
using RSPropertyBase = Rosen::RSPropertyBase;

class ContentModifierAdapter : public RSContentStyleModifier {
public:
    ContentModifierAdapter() = default;
    explicit ContentModifierAdapter(const RefPtr<Modifier>& modifier)
        : modifier_(AceType::DynamicCast<ContentModifier>(modifier))
    {}
    ~ContentModifierAdapter() override = default;

    void Draw(RSDrawingContext& context) const override;

    void AttachProperties();

private:
    RefPtr<ContentModifier> modifier_;
    std::vector<std::shared_ptr<RSPropertyBase>> attachedProperties_;

    ACE_DISALLOW_COPY_AND_MOVE(ContentModifierAdapter);
};

std::shared_ptr<RSModifier> ConvertModifier(const RefPtr<Modifier>& modifier);

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_ADAPTER_MODIFIER_ADAPTER_H
