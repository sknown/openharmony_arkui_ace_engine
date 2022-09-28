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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_ITEM_LAYOUT_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_ITEM_LAYOUT_PROPERTY_H

#include "core/components_ng/layout/layout_property.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT GridItemLayoutProperty : public LayoutProperty {
    DECLARE_ACE_TYPE(GridItemLayoutProperty, LayoutProperty);

public:
    GridItemLayoutProperty() = default;
    ~GridItemLayoutProperty() override = default;

    RefPtr<LayoutProperty> Clone() const override
    {
        auto value = MakeRefPtr<GridItemLayoutProperty>();
        value->LayoutProperty::UpdateLayoutProperty(DynamicCast<LayoutProperty>(this));
        value->propRowStart_ = CloneRowStart();
        value->propRowEnd_ = CloneRowEnd();
        value->propColumnStart_ = CloneColumnStart();
        value->propColumnEnd_ = CloneColumnEnd();
        return value;
    }

    void Reset() override
    {
        LayoutProperty::Reset();
        ResetRowStart();
        ResetRowEnd();
        ResetColumnStart();
        ResetColumnEnd();
    }

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(RowStart, int32_t, PROPERTY_UPDATE_LAYOUT);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(RowEnd, int32_t, PROPERTY_UPDATE_LAYOUT);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(ColumnStart, int32_t, PROPERTY_UPDATE_LAYOUT);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(ColumnEnd, int32_t, PROPERTY_UPDATE_LAYOUT);

private:
    ACE_DISALLOW_COPY_AND_MOVE(GridItemLayoutProperty);
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_ITEM_LAYOUT_PROPERTY_H
