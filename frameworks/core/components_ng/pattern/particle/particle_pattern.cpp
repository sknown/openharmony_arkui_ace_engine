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

#include "core/components_ng/pattern/particle/particle_pattern.h"

#include "core/components_ng/render/adapter/rosen_particle_context.h"

namespace OHOS::Ace::NG {
class RosenRenderParticle;
void ParticlePattern::OnVisibleChange(bool isVisible)
{
    if (HaveUnVisibleParent() == !isVisible) {
        return;
    }
    SetHaveUnVisibleParent(!isVisible);
    if (isVisible) {
        auto host = GetHost();
        auto context = host->GetRenderContext();
        context->OnParticleOptionArrayUpdate(context->GetParticleOptionArray().value());
    }
}

void ParticlePattern::OnAttachToMainTree()
{
    auto host = GetHost();
    auto parent = host->GetParent();
    while (parent) {
        if (InstanceOf<FrameNode>(parent)) {
            auto frameNode = DynamicCast<FrameNode>(parent);
            if (!frameNode->IsVisible()) {
                SetHaveUnVisibleParent(true);
                return;
            }
        }
        parent = parent->GetParent();
    }
}

void ParticlePattern::ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    auto array = JsonUtil::CreateArray(true);
    auto props = GetEmitterProperty();
    for (auto i = 0; i < props.size(); i++) {
        auto object = JsonUtil::Create(true);
        object->Put("index", std::to_string(props[i].index).c_str());
        if (props[i].emitRate.has_value()) {
            object->Put("emitRate", std::to_string(*props[i].emitRate).c_str());
        }
        if (props[i].position.has_value()) {
            auto positionObj = JsonUtil::Create(true);
            positionObj->Put("x", std::to_string(props[i].position->x).c_str());
            positionObj->Put("y", std::to_string(props[i].position->y).c_str());
            object->Put("position", positionObj);
        }
        if (props[i].size.has_value()) {
            auto sizeObj = JsonUtil::Create(true);
            sizeObj->Put("x", std::to_string(props[i].size->x).c_str());
            sizeObj->Put("y", std::to_string(props[i].size->y).c_str());
            object->Put("size", sizeObj);
        }
        array->Put(std::to_string(i).c_str(), object);
    }
    json->Put("emitter", array);
}

void ParticlePattern::UpdateDisturbance(const std::vector<ParticleDisturbance>& disturbanceArray)
{
    if (disturbanceArray.size() == 0) {
        return;
    }
    const std::vector<ParticleDisturbance>& disturbance = GetDisturbance();
    if (disturbance.size() != disturbanceArray.size()) {
        SetDisturbance(disturbanceArray);
        auto frameNode = GetHost();
        RosenRenderParticle::UpdateDisturbance(frameNode, disturbanceArray);
        return;
    }
    bool equal = true;
    for (size_t i = 0; i < disturbance.size(); i++) {
        ParticleDisturbance src = disturbance[i];
        ParticleDisturbance dst = disturbanceArray[i];
        if (src != dst) {
            equal = false;
            break;
        }
    }
    if (equal) {
        return;
    }
    SetDisturbance(disturbanceArray);
    auto frameNode = GetHost();
    RosenRenderParticle::UpdateDisturbance(frameNode, disturbanceArray);
}

void ParticlePattern::updateEmitterPosition(std::vector<EmitterProperty>& props)
{
    auto frameNode = GetHost();
    for (EmitterProperty& prop : props) {
        prop.index = prop.index >= GetEmitterCount() ? 0 : prop.index;
    }
    RosenRenderParticle::updateEmitterPosition(frameNode, props);
}
} // namespace OHOS::Ace::NG