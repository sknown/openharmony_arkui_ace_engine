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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVIGATION_BAR_ITEM_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVIGATION_BAR_ITEM_PATTERN_H

#include "base/memory/referenced.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/navigation/bar_item_accessibility_property.h"
#include "core/components_ng/pattern/navigation/bar_item_event_hub.h"
#include "core/components_ng/pattern/navigation/bar_item_layout_algorithm.h"
#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace::NG {

class BarItemPattern : public Pattern {
    DECLARE_ACE_TYPE(BarItemPattern, Pattern);

public:
    BarItemPattern() = default;
    ~BarItemPattern() override = default;

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<LayoutProperty>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<BarItemAccessibilityProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<BarItemLayoutAlgorithm>();
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<BarItemEventHub>();
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    void OnModifyDone() override;

private:
    RefPtr<ClickEvent> clickListener_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVIGATION_BAR_ITEM_PATTERN_H
