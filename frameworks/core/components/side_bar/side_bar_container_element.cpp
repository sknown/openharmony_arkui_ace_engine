/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "core/components/side_bar/side_bar_container_element.h"

#include "core/components/side_bar/side_bar_container_component.h"

namespace OHOS::Ace {
void SideBarContainerElement::PerformBuild()
{
    auto sidebarContainer = AceType::DynamicCast<SideBarContainerComponent>(component_);
    if (!sidebarContainer) {
        return;
    }
    sidebarContainer->Build();
    StackElement::PerformBuild();
}
}