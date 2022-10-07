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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_WATER_FLOW_WATER_FLOW_ITEM_COMPONENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_WATER_FLOW_WATER_FLOW_ITEM_COMPONENT_H

#include "core/pipeline/base/sole_child_component.h"

namespace OHOS::Ace::V2 {
using OnSelectFunc = std::function<void(bool)>;

class ACE_EXPORT WaterFlowItemComponent : public SoleChildComponent {
    DECLARE_ACE_TYPE(WaterFlowItemComponent, SoleChildComponent);

public:
    WaterFlowItemComponent() = default;
    explicit WaterFlowItemComponent(const RefPtr<Component>& child) : SoleChildComponent(child) {}
    ~WaterFlowItemComponent() override = default;

    RefPtr<RenderNode> CreateRenderNode() override;
    RefPtr<Element> CreateElement() override;
};
} // namespace OHOS::Ace::V2
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_WATER_FLOW_WATER_FLOW_ITEM_COMPONENT_H
