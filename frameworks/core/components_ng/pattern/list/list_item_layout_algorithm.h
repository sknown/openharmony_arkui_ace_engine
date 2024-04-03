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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_ITEM_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_ITEM_LAYOUT_ALGORITHM_H

#include "base/geometry/axis.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_v2/list/list_properties.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT ListItemLayoutAlgorithm : public BoxLayoutAlgorithm {
    DECLARE_ACE_TYPE(ListItemLayoutAlgorithm, BoxLayoutAlgorithm);
public:
    ListItemLayoutAlgorithm(int32_t startNodeIndex, int32_t endNodeIndex, int32_t childNodeIndex)
        :startNodeIndex_(startNodeIndex), endNodeIndex_(endNodeIndex), childNodeIndex_(childNodeIndex) {}

    void Measure(LayoutWrapper* layoutWrapper) override;

    void Layout(LayoutWrapper* layoutWrapper) override;

    void PerformMeasureSelf(LayoutWrapper* layoutWrapper) const;

    void SetAxis(Axis axis)
    {
        axis_ = axis;
    }

    void SetCurOffset(float curOffset)
    {
        curOffset_ = curOffset;
    }

    void SetStartNodeSize(float startNodeSize)
    {
        startNodeSize_ = startNodeSize;
    }

    float GetStartNodeSize() const
    {
        return startNodeSize_;
    }

    void SetEndNodeSize(float endNodeSize)
    {
        endNodeSize_ = endNodeSize;
    }

    float GetEndNodeSize() const
    {
        return endNodeSize_;
    }

    void SetIndexInList(int32_t index)
    {
        indexInList_ = index;
    }

    void SetIndexInListItemGroup(int32_t indexInGroup)
    {
        indexInListItemGroup_ = indexInGroup;
    }

    int32_t GetIndexInList() const
    {
        return indexInList_;
    }

    int32_t GetIndexInListItemGroup() const
    {
        return indexInListItemGroup_;
    }

    void Initialize(LayoutWrapper* layoutWrapper);

private:
    int32_t startNodeIndex_;
    int32_t endNodeIndex_;
    int32_t childNodeIndex_;

    int32_t indexInList_ = 0;
    int32_t indexInListItemGroup_ = -1;

    float curOffset_ = 0.0f;
    float startNodeSize_ = 0.0f;
    float endNodeSize_ = 0.0f;

    Axis axis_ = Axis::VERTICAL;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_LAYOUT_ALGORITHM_H