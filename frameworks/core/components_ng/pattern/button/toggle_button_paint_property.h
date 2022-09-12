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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_BUTTON_TOGGLE_BUTTON_PAINT_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_BUTTON_TOGGLE_BUTTON_PAINT_PROPERTY_H

#include "core/components/common/properties/color.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/paint_property.h"

namespace OHOS::Ace::NG {

class ToggleButtonPaintProperty : public PaintProperty {
    DECLARE_ACE_TYPE(ToggleButtonPaintProperty, PaintProperty)

public:
    ToggleButtonPaintProperty() = default;
    ~ToggleButtonPaintProperty() override = default;
    RefPtr<PaintProperty> Clone() const override
    {
        auto value = MakeRefPtr<ToggleButtonPaintProperty>();
        value->PaintProperty::UpdatePaintProperty(DynamicCast<PaintProperty>(this));
        value->propIsOn_ = CloneIsOn();
        value->propSelectedColor_ = CloneSelectedColor();
        return value;
    }

    void Reset() override
    {
        PaintProperty::Reset();
        ResetIsOn();
        ResetSelectedColor();
    }

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(IsOn, bool, PROPERTY_UPDATE_RENDER);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(SelectedColor, Color, PROPERTY_UPDATE_RENDER);
    ACE_DISALLOW_COPY_AND_MOVE(ToggleButtonPaintProperty);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_BUTTON_TOGGLE_BUTTON_PAINT_PROPERTY_H