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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OPTION_OPTION_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OPTION_OPTION_PATTERN_H

#include "core/components_ng/pattern/option/option_event_hub.h"
#include "core/components_ng/pattern/option/option_layout_algorithm.h"
#include "core/components_ng/pattern/option/option_paint_method.h"
#include "core/components_ng/pattern/option/option_paint_property.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/render/paint_property.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {
class OptionPattern : public Pattern {
    DECLARE_ACE_TYPE(OptionPattern, Pattern);

public:
    OptionPattern(int32_t targetId, int index) : targetId_(targetId), index_(index) {}
    ~OptionPattern() override = default;

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        return MakeRefPtr<OptionPaintMethod>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<OptionPaintProperty>();
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<OptionEventHub>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<OptionLayoutAlgorithm>();
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    void OnModifyDone() override;

private:
    void UpdateNextNodeDivider(bool needDivider);

    // register option's JS callback
    void RegisterOnClick(const RefPtr<GestureEventHub>& hub, const std::function<void()>& action);
    // change option background color
    void RegisterOnHover(const RefPtr<GestureEventHub>& hub);
    
    // to hide menu through OverlayManager when option is clicked
    int32_t targetId_ = -1;
    // this option node's index in the menu
    int index_ = -1;

    ACE_DISALLOW_COPY_AND_MOVE(OptionPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OPTION_OPTION_PATTERN_H
