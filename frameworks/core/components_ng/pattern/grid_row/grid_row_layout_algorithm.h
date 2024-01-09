
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_ROW_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_ROW_LAYOUT_ALGORITHM_H

#include "grid_row_layout_property.h"

#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/pattern/grid_col/grid_col_layout_property.h"
#include "core/components_ng/property/measure_utils.h"

namespace OHOS::Ace::NG {
struct NewLineOffset {
    int32_t newLineCount = 0;
    int32_t offset = 0;
    int32_t span = 0;
    float offsetY = 0;
    std::string ToString()
    {
        std::string result = "new line count: ";
        result += std::to_string(newLineCount);
        result += " , offset: ";
        result += std::to_string(offset);
        result += " , span: ";
        result += std::to_string(span);
        result += " , offsetY: ";
        result += std::to_string(offsetY);
        return result;
    }
};

using ChildrenRow = std::list<std::pair<RefPtr<LayoutWrapper>, NewLineOffset>>;
class ACE_EXPORT GridRowLayoutAlgorithm : public LayoutAlgorithm {
    DECLARE_ACE_TYPE(GridRowLayoutAlgorithm, LayoutAlgorithm);

public:
    void Measure(LayoutWrapper* layoutWrapper) override;
    void Layout(LayoutWrapper* layoutWrapper) override;

private:
    void MeasureSelf(LayoutWrapper* layoutWrapper, float childHeight, float selfHeight);
    float MeasureChildren(LayoutWrapper* layoutWrapper, double columnUnitWidth, double childHeightLimit,
        std::pair<double, double>& gutter, V2::GridSizeType sizeType, int32_t columnNum);
    void CalcCrossAxisAlignment(LayoutWrapper* layoutWrapper,
        std::list<std::pair<RefPtr<LayoutWrapper>, NewLineOffset>>& row, float currentRowHeight);
    std::pair<double, double> gutterInDouble_ { 0, 0 };
    double columnUnitWidth_ = 0;
    std::list<ChildrenRow> gridColChildrenRows_ {};
    ChildrenRow gridColChildrenOfOneRow_ {};
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_ROW_LAYOUT_ALGORITHM_H
