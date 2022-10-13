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

#include "core/components_v2/inspector/water_flow_item_composed_element.h"

#include "base/log/dump_log.h"
#include "core/components/box/box_element.h"
#include "core/components/common/layout/constants.h"
#include "core/components_v2/inspector/utils.h"
#include "core/components_v2/water_flow/render_water_flow_item.h"

namespace OHOS::Ace::V2 {
namespace {

const std::unordered_map<std::string, 
std::function<std::string(const WaterFlowItemComposedElement&)>> CREATE_JSON_MAP {
};

} // namespace

void WaterFlowItemComposedElement::Dump()
{
    InspectorComposedElement::Dump();
}

std::unique_ptr<JsonValue> WaterFlowItemComposedElement::ToJsonObject() const
{
    auto resultJson = InspectorComposedElement::ToJsonObject();
    for (const auto& value : CREATE_JSON_MAP) {
        resultJson->Put(value.first.c_str(), value.second(*this).c_str());
    }
    return resultJson;
}

void WaterFlowItemComposedElement::AddChildWithSlot(int32_t slot, const RefPtr<Component>& newComponent)
{
    auto waterFlowItem = GetContentElement<BoxElement>(BoxElement::TypeId());
    if (!waterFlowItem) {
        LOGE("get GetWaterFlowItemElement failed");
        return;
    }
    waterFlowItem->UpdateChildWithSlot(nullptr, newComponent, slot, slot);
    waterFlowItem->MarkDirty();
}

void WaterFlowItemComposedElement::UpdateChildWithSlot(int32_t slot, const RefPtr<Component>& newComponent)
{
    auto waterFlowItem = GetContentElement<BoxElement>(BoxElement::TypeId());
    if (!waterFlowItem) {
        LOGE("get GetWaterFlowItemElement failed");
        return;
    }
    auto child = GetElementChildBySlot(waterFlowItem, slot);
    if (!child) {
        LOGE("waterFlowItem get GetChildBySlot failed");
        return;
    }
    waterFlowItem->UpdateChildWithSlot(child, newComponent, slot, slot);
    waterFlowItem->MarkDirty();
}

void WaterFlowItemComposedElement::DeleteChildWithSlot(int32_t slot)
{
    auto waterFlowItem = GetContentElement<BoxElement>(BoxElement::TypeId());
    if (!waterFlowItem) {
        LOGE("get GetWaterFlowItemElement failed");
        return;
    }
    auto child = GetElementChildBySlot(waterFlowItem, slot);
    if (!child) {
        LOGE("waterFlowItem get GetChildBySlot failed");
        return;
    }
    waterFlowItem->UpdateChildWithSlot(child, nullptr, slot, slot);
    waterFlowItem->MarkDirty();
}

} // namespace OHOS::Ace::V2