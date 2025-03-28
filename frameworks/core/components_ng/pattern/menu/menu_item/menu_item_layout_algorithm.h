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
#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MENU_MENU_ITEM_MENU_ITEM_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MENU_MENU_ITEM_MENU_ITEM_LAYOUT_ALGORITHM_H

#include "base/memory/referenced.h"
#include "core/components_ng/layout/box_layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"

namespace OHOS::Ace::NG {
namespace {
    constexpr static int32_t CLICKABLE_AREA_VIEW_INDEX = 2;
    constexpr static int32_t EXPANDABLE_AREA_VIEW_INDEX = 3;
}
class ACE_EXPORT MenuItemLayoutAlgorithm : public BoxLayoutAlgorithm {
    DECLARE_ACE_TYPE(MenuItemLayoutAlgorithm, BoxLayoutAlgorithm);

public:
    MenuItemLayoutAlgorithm() = default;
    ~MenuItemLayoutAlgorithm() override = default;

    void Measure(LayoutWrapper* layoutWrapper) override;
    void Layout(LayoutWrapper* layoutWrapper) override;

private:
    void CheckNeedMatchParent(LayoutWrapper* layoutWrapper,
        std::optional<LayoutConstraintF>& layoutConstraint);
    void MeasureItemViews(LayoutConstraintF& childConstraint,
        std::optional<LayoutConstraintF>& layoutConstraint,
        PaddingPropertyF padding, LayoutWrapper* layoutWrapper);
    void MeasureRow(const RefPtr<LayoutWrapper>& row, const LayoutConstraintF& constraint);
    void CheckNeedExpandContent(LayoutWrapper* layoutWrapper, LayoutConstraintF& childConstraint);
    void UpdateSelfSize(LayoutWrapper* layoutWrapper, float width, float itemHeight, float expandableHeight);
    void UpdateOverlay(RefPtr<FrameNode> frameNode);
    float GetDividerStroke(LayoutWrapper* layoutWrapper);

    float horInterval_ = 0.0f;
    float idealWidth_ = 0.0f;
    float emptyWidth_ = 0.0f;
    float maxRowWidth_ = 0.0f;
    float middleSpace_ = 0.0f;
    float minRowWidth_ = 0.0f;
    float minItemHeight_ = 0.0f;
    double iconSize_ = 0.0f;
    bool needExpandContent_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(MenuItemLayoutAlgorithm);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MENU_MENU_ITEM_MENU_ITEM_LAYOUT_ALGORITHM_H
