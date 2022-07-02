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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NAVIGATOR_NAVIGATOR_ELEMENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NAVIGATOR_NAVIGATOR_ELEMENT_H

#include "core/components/navigator/navigator_component.h"
#include "core/focus/focus_node.h"

namespace OHOS::Ace {

class NavigatorElement : public SoleChildElement, public FocusNode {
    DECLARE_ACE_TYPE(NavigatorElement, SoleChildElement, FocusNode);

public:
    void PerformBuild() override;
    void OnClick() override;
    void SetTargetContainer(const WeakPtr<StageElement>& targetContainer, bool useSubStage = false);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NAVIGATOR_NAVIGATOR_ELEMENT_H
