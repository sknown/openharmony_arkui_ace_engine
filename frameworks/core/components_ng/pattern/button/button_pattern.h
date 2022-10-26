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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_BUTTON_BUTTON_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_BUTTON_BUTTON_PATTERN_H

#include <optional>

#include "core/components/common/layout/constants.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/button/button_accessibility_property.h"
#include "core/components_ng/pattern/button/button_event_hub.h"
#include "core/components_ng/pattern/button/button_layout_algorithm.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace::NG {
class ButtonPattern : public Pattern {
    DECLARE_ACE_TYPE(ButtonPattern, Pattern);

public:
    ButtonPattern() = default;

    ~ButtonPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<ButtonEventHub>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<ButtonLayoutAlgorithm>();
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<ButtonLayoutProperty>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<ButtonAccessibilityProperty>();
    }

    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, true };
    }

    void SetClickedColor(const Color& color)
    {
        clickedColor_ = color;
        isSetClickedColor_ = true;
    }

protected:
    void OnModifyDone() override;
    void OnAttachToFrameNode() override;

private:
    void OnTouchDown();
    void OnTouchUp();
    Color clickedColor_;
    Color backgroundColor_;
    bool isSetClickedColor_ = false;

    RefPtr<TouchEventImpl> touchListener_;

    ACE_DISALLOW_COPY_AND_MOVE(ButtonPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_BUTTON_BUTTON_PATTERN_H
