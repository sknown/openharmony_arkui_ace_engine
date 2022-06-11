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

#include "core/components_ng/property/property.h"

namespace OHOS::Ace::NG {
bool CheckNeedRender(PropertyChangeFlag propertyChangeFlag)
{
    return ((propertyChangeFlag & PROPERTY_UPDATE_RENDER) == PROPERTY_UPDATE_RENDER);
}

bool CheckNeedRequestMeasureAndLayout(PropertyChangeFlag propertyChangeFlag)
{
    return ((propertyChangeFlag & PROPERTY_UPDATE_MEASURE) == PROPERTY_UPDATE_MEASURE) ||
           ((propertyChangeFlag & PROPERTY_UPDATE_LAYOUT) == PROPERTY_UPDATE_LAYOUT) ||
           ((propertyChangeFlag & PROPERTY_REQUEST_NEW_CHILD_NODE) == PROPERTY_REQUEST_NEW_CHILD_NODE) ||
           ((propertyChangeFlag & PROPERTY_UPDATE_BY_CHILD_REQUEST) == PROPERTY_UPDATE_BY_CHILD_REQUEST) ||
           ((propertyChangeFlag & PROPERTY_UPDATE_POSITION) == PROPERTY_UPDATE_POSITION);
}

bool CheckNeedRequestParent(PropertyChangeFlag propertyChangeFlag)
{
    return CheckMeasureFlag(propertyChangeFlag) || CheckPositionFlag(propertyChangeFlag);
}

bool CheckMeasureFlag(PropertyChangeFlag propertyChangeFlag)
{
    return ((propertyChangeFlag & PROPERTY_UPDATE_MEASURE) == PROPERTY_UPDATE_MEASURE) ||
           ((propertyChangeFlag & PROPERTY_UPDATE_BY_CHILD_REQUEST) == PROPERTY_UPDATE_BY_CHILD_REQUEST);
}

bool CheckLayoutFlag(PropertyChangeFlag propertyChangeFlag)
{
    return ((propertyChangeFlag & PROPERTY_UPDATE_LAYOUT) == PROPERTY_UPDATE_LAYOUT);
}

bool CheckPositionFlag(PropertyChangeFlag propertyChangeFlag)
{
    return ((propertyChangeFlag & PROPERTY_UPDATE_POSITION) == PROPERTY_UPDATE_POSITION);
}

bool CheckRequestNewChildNodeFlag(PropertyChangeFlag propertyChangeFlag)
{
    return ((propertyChangeFlag & PROPERTY_REQUEST_NEW_CHILD_NODE) == PROPERTY_REQUEST_NEW_CHILD_NODE);
}

bool CheckNoChanged(PropertyChangeFlag propertyChangeFlag)
{
    return (propertyChangeFlag == PROPERTY_UPDATE_NORMAL);
}

} // namespace OHOS::Ace::NG
