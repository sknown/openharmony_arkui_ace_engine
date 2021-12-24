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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_CHECKABLE_RENDER_CHECKBOX_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_CHECKABLE_RENDER_CHECKBOX_H

#include "core/animation/animator.h"
#include "core/animation/curve_animation.h"
#include "core/components/checkable/checkable_component.h"
#include "core/components/checkable/render_checkable.h"

namespace OHOS::Ace {

class RenderCheckbox : public RenderCheckable {
    DECLARE_ACE_TYPE(RenderCheckbox, RenderCheckable);

public:
    RenderCheckbox() = default;
    ~RenderCheckbox() override = default;

    static RefPtr<RenderNode> Create();
    void Update(const RefPtr<Component>& component) override;
    void HandleClick() override;
    bool UpdateGroupValue(const bool groupValue);
    void SetChecked(bool checked)
    {
        checked_ = checked;
    }

protected:
    void UpdateAnimation();
    void OnAnimationStop();
    void UpdateCheckBoxShape(double value);
    void UpdateAccessibilityAttr();

    // animation control
    RefPtr<Animator> controller_;
    RefPtr<CurveAnimation<double>> translate_;

    double shapeScale_ = 1.0;
    std::string checkboxGroupName_ = "";
    RefPtr<CheckboxComponent> component_;
    bool isGroup_ = false;
    bool selectAll_ = false;
    std::function<void(bool)> groupValueChangedListener_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_CHECKABLE_RENDER_CHECKBOX_H
