/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define private public
#include "core/components_ng/pattern/navigation/nav_bar_pattern.h"

namespace OHOS::Ace::NG {
bool NavBarPattern::GetDraggedDown()
{
    return true;
}

void NavBarPattern::OnCoordScrollStart()
{
}

void NavBarPattern::OnCoordScrollUpdate(float offset)
{
}

void NavBarPattern::OnCoordScrollEnd()
{
}

bool NavBarPattern::GetFullStatus()
{
    return true;
}

bool NavBarPattern::GetIsMinTitle() const
{
    return true;
}

void NavBarPattern::ResetAssociatedScroll()
{
}

void NavBarPattern::UpdateAssociatedScrollOffset(float offset)
{
}

bool NavBarPattern::IsTitleModeFree()
{
    return true;
}
}; // namespace OHOS::Ace::NG