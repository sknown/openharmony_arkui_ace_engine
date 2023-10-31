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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_LAYOUT_BASE_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_LAYOUT_BASE_ALGORITHM_H

#include <utility>

#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/pattern/grid/grid_item_layout_property.h"
#include "core/components_ng/pattern/grid/grid_item_pattern.h"
#include "core/components_ng/pattern/grid/grid_layout_info.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT GridLayoutBaseAlgorithm : public LayoutAlgorithm {
    DECLARE_ACE_TYPE(GridLayoutBaseAlgorithm, LayoutAlgorithm);

public:
    explicit GridLayoutBaseAlgorithm(GridLayoutInfo gridLayoutInfo) : gridLayoutInfo_(std::move(gridLayoutInfo)) {};
    ~GridLayoutBaseAlgorithm() override = default;

    const GridLayoutInfo& GetGridLayoutInfo()
    {
        return std::move(gridLayoutInfo_);
    }

    virtual void UpdateRealGridItemPositionInfo(
        const RefPtr<LayoutWrapper>& itemLayoutWrapper, int32_t mainIndex, int32_t crossIndex)
    {
        auto gridItemLayoutProperty =
            AceType::DynamicCast<GridItemLayoutProperty>(itemLayoutWrapper->GetLayoutProperty());
        CHECK_NULL_VOID(gridItemLayoutProperty);
        bool isItemAtExpectedPosition =
            gridItemLayoutProperty->CheckWhetherCurrentItemAtExpectedPosition(gridLayoutInfo_.axis_);
        auto gridItemNode = itemLayoutWrapper->GetHostNode();
        CHECK_NULL_VOID(gridItemNode);
        auto gridItemPattern = gridItemNode->GetPattern<GridItemPattern>();
        CHECK_NULL_VOID(gridItemPattern);
        if (isItemAtExpectedPosition) {
            gridItemPattern->ResetGridItemInfo();
        }
        if (!isItemAtExpectedPosition && gridLayoutInfo_.hasBigItem_) {
            GridItemIndexInfo itemInfo;
            itemInfo.mainIndex = mainIndex;
            itemInfo.crossIndex = crossIndex;
            itemInfo.mainSpan = gridItemLayoutProperty->GetRealMainSpan(gridLayoutInfo_.axis_);
            itemInfo.crossSpan = gridItemLayoutProperty->GetRealCrossSpan(gridLayoutInfo_.axis_);
            itemInfo.mainStart = mainIndex - itemInfo.mainSpan + 1;
            itemInfo.mainEnd = mainIndex;
            itemInfo.crossStart = crossIndex;
            itemInfo.crossEnd = crossIndex + itemInfo.crossSpan - 1;
            gridItemPattern->ResetGridItemInfo();
            gridItemPattern->SetIrregularItemInfo(itemInfo);
        }
    }

protected:
    GridLayoutInfo gridLayoutInfo_;

    ACE_DISALLOW_COPY_AND_MOVE(GridLayoutBaseAlgorithm);
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_LAYOUT_BASE_ALGORITHM_H
