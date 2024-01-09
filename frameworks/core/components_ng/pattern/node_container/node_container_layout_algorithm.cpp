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

#include "core/components_ng/pattern/node_container/node_container_layout_algorithm.h"

#include "core/components_ng/layout/layout_wrapper.h"

namespace OHOS::Ace::NG {
void NodeContainerLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto layoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    for (auto&& child : layoutWrapper->GetAllChildrenWithBuild()) {
        if (child->GetHostTag() == "RenderNode") {
            child->Measure(child->GetLayoutProperty()->GetLayoutConstraint());
        } else {
            child->Measure(layoutConstraint);
        }
    }
    PerformMeasureSelf(layoutWrapper);
}
} // namespace OHOS::Ace::NG
