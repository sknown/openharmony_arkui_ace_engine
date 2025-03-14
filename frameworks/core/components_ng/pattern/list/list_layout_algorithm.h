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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_LAYOUT_ALGORITHM_H

#include <cstdint>
#include <map>
#include <optional>

#include "base/geometry/axis.h"
#include "base/memory/referenced.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/list/list_layout_property.h"
#include "core/components_ng/pattern/list/list_position_map.h"
#include "core/components_v2/list/list_component.h"
#include "core/components_v2/list/list_properties.h"

namespace OHOS::Ace::NG {
class PipelineContext;
class ListPositionMap;

struct ListItemInfo {
    int32_t id;
    float startPos;
    float endPos;
    bool isGroup;
    bool isPressed = false;
};

struct ListPredictLayoutParam {
    std::list<int32_t> items;
    LayoutConstraintF layoutConstraint;
};

struct PredictLayoutItem {
    int32_t index;
    bool forward;
    int32_t cached;
};

struct ListPredictLayoutParamV2 {
    std::list<PredictLayoutItem> items;
    LayoutConstraintF layoutConstraint;
    LayoutConstraintF groupLayoutConstraint;
};

enum class ScrollAutoType {
    NOT_CHANGE = 0,
    START,
    END,
};

// TextLayoutAlgorithm acts as the underlying text layout.
class ACE_EXPORT ListLayoutAlgorithm : public LayoutAlgorithm {
    DECLARE_ACE_TYPE(ListLayoutAlgorithm, LayoutAlgorithm);

public:
    using PositionMap = std::map<int32_t, ListItemInfo>;
    static constexpr int32_t LAST_ITEM = -1;

    ListLayoutAlgorithm() = default;

    ~ListLayoutAlgorithm() override = default;

    void OnReset() override {}

    const PositionMap& GetItemPosition() const
    {
        return itemPosition_;
    }

    void SetItemsPosition(const PositionMap& itemPosition)
    {
        itemPosition_ = itemPosition;
    }

    const PositionMap& GetRecycledItemPosition() const
    {
        return recycledItemPosition_;
    }

    void ClearAllItemPosition(LayoutWrapper* layoutWrapper);

    void SetOverScrollFeature()
    {
        overScrollFeature_ = true;
    }

    void SetCanOverScroll(bool canOverScroll)
    {
        canOverScroll_ = canOverScroll;
    }

    void SetIsSpringEffect(bool isSpringEffect)
    {
        isSpringEffect_ = isSpringEffect;
    }

    void SetIndex(int32_t index)
    {
        jumpIndex_ = index;
    }

    void SetTargetIndex(int32_t index)
    {
        targetIndex_ = index;
    }

    std::optional<int32_t> GetTargetIndex() const
    {
        return targetIndexStaged_;
    }

    void SetPredictSnapOffset(float predictSnapOffset)
    {
        predictSnapOffset_ = predictSnapOffset;
    }

    std::optional<float> GetPredictSnapOffset() const
    {
        return predictSnapOffset_;
    }

    void SetPredictSnapEndPosition(float predictSnapEndPos)
    {
        predictSnapEndPos_ = predictSnapEndPos;
    }

    std::optional<float> GetPredictSnapEndPosition()
    {
        return predictSnapEndPos_;
    }

    void SetIndexInGroup(int32_t index)
    {
        jumpIndexInGroup_ = index;
    }

    void SetIndexAlignment(ScrollAlign align)
    {
        scrollAlign_ = align;
    }

    void SetCurrentDelta(float offset)
    {
        currentDelta_ = offset;
        currentOffset_ = offset;
    }

    float GetCurrentOffset() const
    {
        return currentOffset_ - adjustOffset_;
    }

    void SetIsNeedCheckOffset(bool isNeedCheckOffset)
    {
        isNeedCheckOffset_ = isNeedCheckOffset;
    }

    void SetTotalOffset(float totalOffset)
    {
        totalOffset_ = totalOffset;
    }

    float GetContentMainSize() const
    {
        return contentMainSize_;
    }

    void SetPrevContentMainSize(float mainSize)
    {
        prevContentMainSize_ = mainSize;
    }

    int32_t GetStartIndex() const
    {
        return itemPosition_.empty() ? -1 : itemPosition_.begin()->first;
    }

    int32_t GetEndIndex() const
    {
        return itemPosition_.empty() ? -1 : itemPosition_.rbegin()->first;
    }

    int32_t GetMidIndex(LayoutWrapper* layoutWrapper, bool usePreContentMainSize = false);

    int32_t GetMaxListItemIndex() const
    {
        return totalItemCount_ - 1;
    }

    void SetSpaceWidth(float spaceWidth)
    {
        spaceWidth_ = spaceWidth;
    }

    float GetSpaceWidth() const
    {
        return spaceWidth_;
    }

    bool NeedEstimateOffset() const
    {
        return needEstimateOffset_;
    }

    void SetContentStartOffset(float startOffset)
    {
        contentStartOffset_ = startOffset;
    }

    void SetContentEndOffset(float endOffset)
    {
        contentEndOffset_ = endOffset;
    }

    float GetContentStartOffset() const
    {
        return contentStartOffset_;
    }

    float GetContentEndOffset() const
    {
        return contentEndOffset_;
    }

    void SetPrevContentStartOffset(float prevContentStartOffset)
    {
        prevContentStartOffset_ = prevContentStartOffset;
    }

    void SetPrevContentEndOffset(float prevContentEndOffset)
    {
        prevContentEndOffset_ = prevContentEndOffset;
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

    void SetChainOffsetCallback(std::function<float(int32_t)> func)
    {
        chainOffsetFunc_ = std::move(func);
    }

    void SetChainInterval(float interval)
    {
        chainInterval_ = interval;
    }

    bool IsCrossMatchChild() const
    {
        return crossMatchChild_;
    }

    float GetChildMaxCrossSize(LayoutWrapper* layoutWrapper, Axis axis) const;

    void Measure(LayoutWrapper* layoutWrapper) override;

    void Layout(LayoutWrapper* layoutWrapper) override;

    void LayoutForward(LayoutWrapper* layoutWrapper, int32_t startIndex, float startPos);
    void LayoutBackward(LayoutWrapper* layoutWrapper, int32_t endIndex, float endPos);

    void BeginLayoutForward(float startPos, LayoutWrapper* layoutWrapper);

    void BeginLayoutBackward(float startPos, LayoutWrapper* layoutWrapper);

    void HandleJumpAuto(LayoutWrapper* layoutWrapper, int32_t startIndex, int32_t endIndex);

    void HandleJumpCenter(LayoutWrapper* layoutWrapper);

    void HandleJumpStart(LayoutWrapper* layoutWrapper);

    void HandleJumpEnd(LayoutWrapper* layoutWrapper);

    bool NoNeedJump(LayoutWrapper* layoutWrapper, float startPos, float endPos,
        int32_t startIndex, int32_t endIndex, int32_t jumpIndex, float jumpIndexStartPos);

    bool CheckNoNeedJumpListItem(LayoutWrapper* layoutWrapper, float startPos, float endPos,
        int32_t startIndex, int32_t endIndex, int32_t jumpIndex);

    bool CheckNoNeedJumpListItemGroup(LayoutWrapper* layoutWrapper, int32_t startIndex, int32_t endIndex,
        int32_t jumpIndex, float jumpIndexStartPos);

    virtual float MeasureAndGetChildHeight(LayoutWrapper* layoutWrapper, int32_t childIndex,
        bool groupLayoutAll = true);

    virtual float GetChildHeight(LayoutWrapper* layoutWrapper, int32_t childIndex)
    {
        return childrenSize_->GetChildSize(childIndex);
    }

    virtual int32_t GetLanes() const
    {
        return 1;
    }

    void SetLaneGutter(float laneGutter)
    {
        laneGutter_ = laneGutter;
    }

    float GetLaneGutter() const
    {
        return laneGutter_;
    }

    void OffScreenLayoutDirection();

    ScrollAutoType GetScrollAutoType() const
    {
        return scrollAutoType_;
    }

    bool CheckJumpValid(LayoutWrapper* layoutWrapper);

    float GetListGroupItemHeight(const RefPtr<LayoutWrapper>& layoutWrapper, int32_t index);

    bool JudgeInOfScreenScrollAutoType(const RefPtr<LayoutWrapper>& layoutWrapper,
        const RefPtr<ListLayoutProperty>& layoutProperty, float topPos, float bottomPos);

    void JudgeOutOfScreenScrollAutoType(const RefPtr<LayoutWrapper>& layoutWrapper, int32_t index,
        const RefPtr<ListLayoutProperty>& layoutProperty, int32_t indexInGroup, int32_t judgeIndex,
        int32_t startIndex, int32_t endIndex);

    virtual LayoutConstraintF& GetGroupLayoutConstraint()
    {
        return childLayoutConstraint_;
    }

    void OnItemPositionAddOrUpdate(LayoutWrapper* layoutWrapper, uint32_t index);

    void SetListChildrenMainSize(const RefPtr<ListChildrenMainSize>& childrenMainSize)
    {
        childrenSize_ = childrenMainSize;
    }

    void SetListPositionMap(const RefPtr<ListPositionMap>& posMap)
    {
        posMap_ = posMap;
    }

    std::pair<int32_t, float> GetSnapStartIndexAndPos();

    std::pair<int32_t, float> GetSnapEndIndexAndPos();

protected:
    virtual void UpdateListItemConstraint(
        Axis axis, const OptionalSizeF& selfIdealSize, LayoutConstraintF& contentConstraint);
    virtual int32_t LayoutALineForward(
        LayoutWrapper* layoutWrapper, int32_t& currentIndex, float startPos, float& endPos);
    virtual int32_t LayoutALineBackward(
        LayoutWrapper* layoutWrapper, int32_t& currentIndex, float endPos, float& startPos);
    virtual float CalculateLaneCrossOffset(float crossSize, float childCrossSize);
    virtual void CalculateLanes(const RefPtr<ListLayoutProperty>& layoutProperty,
        const LayoutConstraintF& layoutConstraint, std::optional<float> crossSizeOptional, Axis axis) {};
    virtual int32_t GetLanesFloor(LayoutWrapper* layoutWrapper, int32_t index)
    {
        return index;
    }
    virtual int32_t GetLanesCeil(LayoutWrapper* layoutWrapper, int32_t index)
    {
        return index;
    }
    virtual void SetCacheCount(LayoutWrapper* layoutWrapper, int32_t cacheCount);
    virtual void SetActiveChildRange(LayoutWrapper* layoutWrapper, int32_t cacheCount);

    void SetListItemGroupParam(const RefPtr<LayoutWrapper>& layoutWrapper, int32_t index, float referencePos,
        bool forwardLayout, const RefPtr<ListLayoutProperty>& layoutProperty, bool groupNeedAllLayout,
        bool needAdjustRefPos = false);
    static void SetListItemIndex(const RefPtr<LayoutWrapper>& layoutWrapper, int32_t index);
    void ReMeasureListItemGroup(LayoutWrapper* layoutWrapper, bool forwardLayout);
    void CheckListItemGroupRecycle(
        LayoutWrapper* layoutWrapper, int32_t index, float referencePos, bool forwardLayout) const;
    void AdjustPostionForListItemGroup(LayoutWrapper* layoutWrapper, Axis axis, int32_t index, bool forwardLayout);
    void SetItemInfo(int32_t index, ListItemInfo&& info)
    {
        itemPosition_[index] = info;
    }
    void LayoutItem(RefPtr<LayoutWrapper>& layoutWrapper, int32_t index, const ListItemInfo& pos,
        int32_t& startIndex, float crossSize);
    static void SyncGeometry(RefPtr<LayoutWrapper>& wrapper);
    ListItemInfo GetListItemGroupPosition(const RefPtr<LayoutWrapper>& layoutWrapper, int32_t index);
    bool CheckNeedMeasure(const RefPtr<LayoutWrapper>& layoutWrapper) const;
    void ReviseSpace(const RefPtr<ListLayoutProperty>& listLayoutProperty);
    std::pair<int32_t, int32_t> GetLayoutGroupCachedCount(
        const RefPtr<LayoutWrapper>& wrapper, bool forward, int32_t cacheCount);

    Axis axis_ = Axis::VERTICAL;
    LayoutConstraintF childLayoutConstraint_;
    RefPtr<ListChildrenMainSize> childrenSize_;
    RefPtr<ListPositionMap> posMap_;
    std::optional<std::pair<int32_t, ListItemInfo>> firstItemInfo_;
private:
    void MeasureList(LayoutWrapper* layoutWrapper);
    void RecycleGroupItem(LayoutWrapper* layoutWrapper) const;
    void CheckJumpToIndex();
    void CheckAndMeasureStartItem(LayoutWrapper* layoutWrapper, int32_t startIndex,
        float& startPos, bool isGroup, bool forwardLayout);

    std::pair<int32_t, float> RequestNewItemsForward(LayoutWrapper* layoutWrapper,
        const LayoutConstraintF& layoutConstraint, int32_t startIndex, float startPos, Axis axis);

    std::pair<int32_t, float> RequestNewItemsBackward(LayoutWrapper* layoutWrapper,
        const LayoutConstraintF& layoutConstraint, int32_t startIndex, float startPos, Axis axis);

    void OnSurfaceChanged(LayoutWrapper* layoutWrapper);

    void FixPredictSnapOffset(const RefPtr<ListLayoutProperty>& listLayoutProperty);
    void FixPredictSnapOffsetAlignStart();
    void FixPredictSnapOffsetAlignCenter();
    void FixPredictSnapOffsetAlignEnd();
    bool IsScrollSnapAlignCenter(LayoutWrapper* layoutWrapper);
    bool LayoutCachedALine(LayoutWrapper* layoutWrapper, int32_t index, bool forward, float &currPos, float crossSize);
    virtual std::list<int32_t> LayoutCachedItem(LayoutWrapper* layoutWrapper, int32_t cacheCount);
    static void PostIdleTask(RefPtr<FrameNode> frameNode, const ListPredictLayoutParam& param);
    static bool PredictBuildItem(RefPtr<LayoutWrapper> wrapper, const LayoutConstraintF& constraint);

    virtual int32_t LayoutCachedForward(LayoutWrapper* layoutWrapper, int32_t cacheCount, int32_t cached);
    virtual int32_t LayoutCachedBackward(LayoutWrapper* layoutWrapper, int32_t cacheCount, int32_t cached);
    std::list<PredictLayoutItem> LayoutCachedItemV2(LayoutWrapper* layoutWrapper, int32_t cacheCount);
    static bool PredictBuildGroup(RefPtr<LayoutWrapper> wrapper,
        const LayoutConstraintF& constraint, bool forward, int64_t deadline, int32_t cached);
    static void PostIdleTaskV2(RefPtr<FrameNode> frameNode, const ListPredictLayoutParamV2& param);
    static void PredictBuildV2(RefPtr<FrameNode> frameNode, int64_t deadline);

    float GetStopOnScreenOffset(V2::ScrollSnapAlign scrollSnapAlign);
    int32_t FindPredictSnapEndIndexInItemPositions(float predictEndPos, V2::ScrollSnapAlign scrollSnapAlign);
    bool IsUniformHeightProbably();
    float CalculatePredictSnapEndPositionByIndex(uint32_t index, V2::ScrollSnapAlign scrollSnapAlign);
    void UpdateSnapCenterContentOffset(LayoutWrapper* layoutWrapper);

    std::optional<int32_t> jumpIndex_;
    std::optional<int32_t> jumpIndexInGroup_;
    std::optional<int32_t> targetIndex_;
    std::optional<int32_t> targetIndexStaged_;
    std::optional<float> predictSnapOffset_;
    std::optional<float> predictSnapEndPos_;
    ScrollAlign scrollAlign_ = ScrollAlign::START;
    ScrollAutoType scrollAutoType_ = ScrollAutoType::NOT_CHANGE;

    PositionMap itemPosition_;
    PositionMap recycledItemPosition_;
    float currentOffset_ = 0.0f;
    float adjustOffset_ = 0.0f;
    float totalOffset_ = 0.0f;
    float currentDelta_ = 0.0f;
    float startMainPos_ = 0.0f;
    float endMainPos_ = 0.0f;
    float contentStartOffset_ = 0.0f;
    float contentEndOffset_ = 0.0f;
    float prevContentStartOffset_ = 0.0f;
    float prevContentEndOffset_ = 0.0f;
    float spaceWidth_ = 0.0f;
    bool overScrollFeature_ = false;
    bool canOverScroll_ = false;
    bool isSpringEffect_ = false;
    bool forwardFeature_ = false;
    bool backwardFeature_ = false;
    bool isNeedCheckOffset_ = false;
    bool expandSafeArea_ = false;

    int32_t totalItemCount_ = 0;

    V2::ListItemAlign listItemAlign_ = V2::ListItemAlign::START;

    bool needEstimateOffset_ = false;

    bool mainSizeIsDefined_ = false;
    bool crossMatchChild_ = false;
    bool isSnapCenter_ = false;
    float contentMainSize_ = 0.0f;
    float prevContentMainSize_ = 0.0f;
    float paddingBeforeContent_ = 0.0f;
    float paddingAfterContent_ = 0.0f;
    float laneGutter_ = 0.0f;
    OffsetF paddingOffset_;

    V2::StickyStyle stickyStyle_ = V2::StickyStyle::NONE;

    std::function<float(int32_t)> chainOffsetFunc_;
    float chainInterval_ = 0.0f;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_LAYOUT_ALGORITHM_H
