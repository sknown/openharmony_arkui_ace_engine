/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_ITEM_GROUP_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_ITEM_GROUP_LAYOUT_ALGORITHM_H

#include <optional>
#include "base/geometry/axis.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/list/list_layout_property.h"
#include "core/components_v2/list/list_properties.h"

namespace OHOS::Ace::NG {
class ListPositionMap;
class ListChildrenMainSize;
struct LayoutedItemInfo {
    int32_t startIndex = 0;
    float startPos = 0.0f;
    int32_t endIndex = 0;
    float endPos = 0.0f;
};

struct ListItemGroupInfo {
    int32_t id = -1;
    float startPos = 0.0f;
    float endPos = 0.0f;
    bool isPressed = false;
};

// TextLayoutAlgorithm acts as the underlying text layout.
class ACE_EXPORT ListItemGroupLayoutAlgorithm : public LayoutAlgorithm {
    DECLARE_ACE_TYPE(ListItemGroupLayoutAlgorithm, LayoutAlgorithm);
public:
    using PositionMap = std::map<int32_t, ListItemGroupInfo>;

    static const int32_t LAST_ITEM = -1;

    ListItemGroupLayoutAlgorithm(int32_t headerIndex, int32_t footerIndex, int32_t itemStartIndex)
        :headerIndex_(headerIndex), footerIndex_(footerIndex), itemStartIndex_(itemStartIndex) {}

    void Measure(LayoutWrapper* layoutWrapper) override;

    void Layout(LayoutWrapper* layoutWrapper) override;

    const PositionMap& GetItemPosition() const
    {
        return itemPosition_;
    }

    void SetItemsPosition(const PositionMap& itemPosition)
    {
        itemPosition_ = itemPosition;
    }

    void ClearItemPosition(LayoutWrapper* layoutWrapper);

    float GetSpaceWidth() const
    {
        return spaceWidth_;
    }

    Axis GetAxis() const
    {
        return axis_;
    }

    TextDirection GetLayoutDirection() const
    {
        return layoutDirection_;
    }

    float GetMainSize() const
    {
        return totalMainSize_;
    }

    int32_t GetLanes() const
    {
        return lanes_;
    }

    float GetLaneGutter() const
    {
        return laneGutter_;
    }

    int32_t GetLanesFloor(int32_t index) const
    {
        if (lanes_ <= 1) {
            return index;
        }
        return index - index % lanes_;
    }

    int32_t GetLanesCeil(int32_t index) const
    {
        int32_t tmpIndex = (lanes_ <= 1) ? index : (index - index % lanes_ + lanes_ - 1);
        tmpIndex = tmpIndex >= totalItemCount_ ? totalItemCount_ - 1 : tmpIndex;
        return tmpIndex;
    }

    void SetListMainSize(float startPos, float endPos, float referencePos, float prevContentSize, bool forwardLayout)
    {
        startPos_ = startPos;
        endPos_ = endPos;
        referencePos_ = referencePos;
        forwardLayout_ = forwardLayout;
        refPos_ = referencePos;
        prevContentMainSize_ = prevContentSize;
    }

    void ModifyReferencePos(int32_t index, float pos);

    void SetNeedAdjustRefPos(bool needAdjust)
    {
        needAdjustRefPos_ = needAdjust;
    }

    float GetRefPos() const
    {
        return refPos_;
    }

    void SetContentOffset(float contentStartOffset, float contentEndOffset)
    {
        contentStartOffset_ = contentStartOffset;
        contentEndOffset_ = contentEndOffset;
    }

    void SetListLayoutProperty(RefPtr<ListLayoutProperty> layoutProperty)
    {
        listLayoutProperty_ = std::move(layoutProperty);
    }

    void SetJumpIndex(int32_t index)
    {
        jumpIndex_ = index;
    }

    void SetTargetIndex(int32_t index)
    {
        targetIndex_ = index;
    }

    int32_t GetStartIndex() const
    {
        return itemPosition_.empty() ? 0 : itemPosition_.begin()->first;
    }

    int32_t GetEndIndex() const
    {
        return itemPosition_.empty() ? 0 : itemPosition_.rbegin()->first;
    }

    float GetStartPosition() const
    {
        if (itemPosition_.empty()) {
            return 0.0f;
        }
        if (GetStartIndex() == 0) {
            return itemPosition_.begin()->second.startPos;
        }
        return itemPosition_.begin()->second.startPos - spaceWidth_;
    }

    float GetEndPosition() const
    {
        if (itemPosition_.empty()) {
            return 0.0f;
        }
        if (GetEndIndex() == totalItemCount_ - 1) {
            return itemPosition_.rbegin()->second.endPos;
        }
        return itemPosition_.rbegin()->second.endPos + spaceWidth_;
    }

    int32_t GetTotalItemCount() const
    {
        return totalItemCount_;
    }

    float GetChildMaxCrossSize(LayoutWrapper* layoutWrapper, Axis axis);

    void CheckRecycle(const RefPtr<LayoutWrapper>& layoutWrapper, float startPos, float endPos, float referencePos,
        bool forwardLayout);

    void SetNeedAllLayout()
    {
        needAllLayout_ = true;
    }

    void SetScrollAlign(ScrollAlign align)
    {
        scrollAlign_ = align;
    }

    std::pair<float, float> GetItemGroupPosition(int32_t index);

    float GetHeaderMainSize() const
    {
        return headerMainSize_;
    }

    float GetFooterMainSize() const
    {
        return footerMainSize_;
    }

    float GetItemHeight(int32_t index);

    int32_t GetItemStartIndex()
    {
        return itemStartIndex_;
    }

    void SetLayoutedItemInfo(const std::optional<LayoutedItemInfo>& itemInfo)
    {
        layoutedItemInfo_ = itemInfo;
    }

    std::optional<LayoutedItemInfo> GetLayoutedItemInfo() const
    {
        return layoutedItemInfo_;
    }

    void SetListChildrenMainSize(const RefPtr<ListChildrenMainSize>& childrenMainSize)
    {
        childrenSize_ = childrenMainSize;
    }

    void SetListPositionMap(const RefPtr<ListPositionMap>& posMap)
    {
        posMap_ = posMap;
    }

    void AdjustByPosMap();

    static void SyncGeometry(RefPtr<LayoutWrapper>& wrapper);

    float GetStartHeaderPos() const
    {
        return startHeaderPos_;
    }

    float GetEndFooterPos() const
    {
        return endFooterPos_;
    }

private:
    float CalculateLaneCrossOffset(float crossSize, float childCrossSize);
    void UpdateListItemConstraint(const OptionalSizeF& selfIdealSize, LayoutConstraintF& contentConstraint);
    void LayoutListItem(LayoutWrapper* layoutWrapper, const OffsetF& paddingOffset, float crossSize);
    void LayoutListItemAll(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, float startPos);
    void LayoutHeaderFooterRTL(LayoutWrapper* layoutWrapper, const OffsetF& paddingOffset, float crossSize);
    void LayoutHeaderFooterLTR(LayoutWrapper* layoutWrapper, const OffsetF& paddingOffset, float crossSize);
    void UpdateZIndex(const RefPtr<LayoutWrapper>& layoutWrapper);
    void LayoutIndex(const RefPtr<LayoutWrapper>& wrapper, const OffsetF& paddingOffset,
        float crossSize, float startPos);
    inline RefPtr<LayoutWrapper> GetListItem(LayoutWrapper* layoutWrapper, int32_t index) const
    {
        return layoutWrapper->GetOrCreateChildByIndex(index + itemStartIndex_);
    }
    void CalculateLanes(const RefPtr<ListLayoutProperty>& layoutProperty,
        const LayoutConstraintF& layoutConstraint, std::optional<float> crossSizeOptional, Axis axis);

    void MeasureListItem(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint);
    int32_t MeasureALineForward(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
        int32_t& currentIndex, float startPos, float& endPos);
    int32_t MeasureALineBackward(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
        int32_t& currentIndex, float endPos, float& startPos);
    int32_t MeasureALineCenter(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
        int32_t currentIndex);
    int32_t MeasureALineAuto(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
        int32_t currentIndex);
    void MeasureForward(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
        int32_t startIndex, float startPos);
    void MeasureBackward(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
        int32_t endIndex, float endPos);
    void MeasureJumpToItemForward(LayoutWrapper* layoutWrapper,
        const LayoutConstraintF& layoutConstraint, int32_t startIndex, float startPos);
    void MeasureJumpToItemBackward(LayoutWrapper* layoutWrapper,
        const LayoutConstraintF& layoutConstraint, int32_t endIndex, float endPos);
    void MeasureCenter(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, int32_t startIndex);
    void MeasureStart(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, int32_t startIndex);
    void MeasureEnd(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, int32_t startIndex);
    void MeasureAuto(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, int32_t startIndex);
    void MeasureHeaderFooter(LayoutWrapper* layoutWrapper);
    void SetActiveChildRange(LayoutWrapper* layoutWrapper, int32_t cacheCount);
    float UpdateReferencePos(RefPtr<LayoutProperty> layoutProperty, bool forwardLayout, float referencePos);
    bool NeedMeasureItem(LayoutWrapper* layoutWrapper);
    static void SetListItemIndex(const LayoutWrapper* groupLayoutWrapper,
        const RefPtr<LayoutWrapper>& itemLayoutWrapper, int32_t indexInGroup);
    bool IsCardStyleForListItemGroup(const LayoutWrapper* groupLayoutWrapper);
    float GetListItemGroupMaxWidth(const OptionalSizeF& parentIdealSize, RefPtr<LayoutProperty> layoutProperty);
    void AdjustItemPosition();
    bool CheckNeedMeasure(const RefPtr<LayoutWrapper>& layoutWrapper) const;

    bool isCardStyle_ = false;
    int32_t headerIndex_;
    int32_t footerIndex_;
    int32_t itemStartIndex_;
    RefPtr<ListLayoutProperty> listLayoutProperty_;
    float paddingBeforeContent_ = 0.0f;
    float paddingAfterContent_ = 0.0f;

    PositionMap itemPosition_;
    RefPtr<ListChildrenMainSize> childrenSize_;
    RefPtr<ListPositionMap> posMap_;
    Axis axis_ = Axis::VERTICAL;
    int32_t lanes_ = 1;
    float laneGutter_ = 0.0f;
    std::optional<float> minLaneLength_;
    std::optional<float> maxLaneLength_;
    V2::ListItemAlign itemAlign_ = V2::ListItemAlign::START;
    float spaceWidth_ = 0.0f;

    std::optional<int32_t> jumpIndex_;
    std::optional<int32_t> targetIndex_;
    ScrollAlign scrollAlign_ = ScrollAlign::NONE;
    int32_t totalItemCount_ = 0;
    float totalMainSize_ = 0.0f;
    float headerMainSize_ = 0.0f;
    float footerMainSize_ = 0.0f;
    float startPos_ = 0.0f;
    float startHeaderPos_ = 0.0f;
    float endFooterPos_ = 0.0f;
    float prevStartPos_ = 0.0f;
    float prevEndPos_ = 0.0f;
    float endPos_ = 0.0f;
    float referencePos_ = 0.0f;
    float refPos_ = 0.0f;
    float prevContentMainSize_ = 0.0f;
    float contentStartOffset_ = 0.0f;
    float contentEndOffset_ = 0.0f;
    bool forwardLayout_ = true;
    bool needAllLayout_ = false;
    bool needAdjustRefPos_ = false;

    std::optional<LayoutedItemInfo> layoutedItemInfo_;
    LayoutConstraintF childLayoutConstraint_;
    TextDirection layoutDirection_ = TextDirection::LTR;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_LAYOUT_ALGORITHM_H
