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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_FLEX_FLEX_LAYOUT_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_FLEX_FLEX_LAYOUT_PATTERN_H

#include "core/components_ng/pattern/flex/flex_layout_algorithm.h"
#include "core/components_ng/pattern/flex/flex_layout_property.h"
#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace::NG {
class FlexLayoutPattern : public Pattern {
    DECLARE_ACE_TYPE(FlexLayoutPattern, Pattern);

public:
    FlexLayoutPattern() = default;
    ~FlexLayoutPattern() override = default;

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<FlexLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<FlexLayoutAlgorithm>();
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    FocusType GetFocusType() override
    {
        return FocusType::SCOPE;
    }
    bool GetFocusable() override
    {
        return true;
    }

private:
    ACE_DISALLOW_COPY_AND_MOVE(FlexLayoutPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_FLEX_FLEX_LAYOUT_PATTERN_H
