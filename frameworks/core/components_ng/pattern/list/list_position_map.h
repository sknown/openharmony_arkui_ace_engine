/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_POSITION_MAP_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_POSITION_MAP_H


#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <tuple>
#include <queue>

#include "base/geometry/dimension.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"
#include "core/components_ng/pattern/list/list_children_main_size.h"
#include "core/components_ng/pattern/list/list_item_group_pattern.h"
#include "core/components_ng/pattern/list/list_layout_algorithm.h"
#include "core/components_ng/property/measure_property.h"


namespace OHOS::Ace::NG {
class ListItemGroupPattern;
namespace {

struct PositionInfo {
    float mainPos;
    float mainSize;
};

enum class ListPosMapUpdate {
    NO_CHANGE = 0,
    UPDATE_ALL_SIZE,
    RE_CALCULATE,
};
}

class ListPositionMap : public virtual AceType {
    DECLARE_ACE_TYPE(ListPositionMap, AceType)
public:
    ListPositionMap() = default;
    ~ListPositionMap() override = default;

    void UpdatePos(int32_t index, PositionInfo posInfo)
    {
        posMap_[index] = { posInfo.mainPos, posInfo.mainSize };
    }

    void ClearPosMap()
    {
        posMap_.clear();
    }

    void MarkDirty(ListChangeFlag flag)
    {
        dirty_ = dirty_ | flag;
    }

    void ClearDirty()
    {
        dirty_ = LIST_NO_CHANGE;
    }

    float GetTotalHeight()
    {
        return totalHeight_;
    }

    ListPosMapUpdate CheckPosMapUpdateRule()
    {
        ListPosMapUpdate flag;
        if (dirty_ == LIST_NO_CHANGE) {
            flag = ListPosMapUpdate::NO_CHANGE;
        } else if (0 == (dirty_ & (LIST_UPDATE_CHILD_SIZE | LIST_UPDATE_LANES | LIST_GROUP_UPDATE_HEADER_FOOTER))) {
            flag = ListPosMapUpdate::UPDATE_ALL_SIZE;
        } else {
            flag = ListPosMapUpdate::RE_CALCULATE;
        }
        return flag;
    }

    void UpdatePosMapStart(float delta, float& listCurrentPos, float space, int32_t startIndex, float startPos)
    {
        listCurrentPos += delta;
        if (startIndex == 0) {
            listCurrentPos = -startPos;
            return;
        }
        auto it = posMap_.find(startIndex);
        if (it == posMap_.begin() || it == posMap_.end()) {
            return;
        }
        it--;
        float prevPos = it->second.mainPos + it->second.mainSize + space;
        int32_t prevIndex = it->first;
        if (prevIndex + 1 >= startIndex) {
            if (NearEqual(prevPos, startPos)) {
                return;
            }
        } else {
            if (LessNotEqual(prevPos, startPos)) {
                return;
            }
        }
        listCurrentPos += prevPos - startPos;
    }

    void UpdatePosMapEnd(int32_t prevEndIndex, float space)
    {
        auto it = posMap_.find(prevEndIndex);
        if (it == posMap_.end()) {
            return;
        }
        float prevPos = it->second.mainPos + it->second.mainSize + space;
        it++;
        if (it == posMap_.end()) {
            return;
        }
        if (prevEndIndex + 1 >= it->first) {
            if (NearEqual(prevPos, it->second.mainPos)) {
                return;
            }
        } else {
            if (LessNotEqual(prevPos, it->second.mainPos)) {
                return;
            }
        }
        float delta = prevPos - it->second.mainPos;
        while (it != posMap_.end()) {
            it->second.mainPos += delta;
            it++;
        }
    }

    void CalculateUINode(RefPtr<UINode> node)
    {
        CHECK_NULL_VOID(node);
        auto children = node->GetChildren();
        for (const auto& child : children) {
            if (AceType::InstanceOf<FrameNode>(child)) {
                auto frameNode = AceType::DynamicCast<FrameNode>(child);
                CalculateFrameNode(frameNode);
            } else if (AceType::InstanceOf<LazyForEachNode>(child)) {
                auto lazyForEach = AceType::DynamicCast<LazyForEachNode>(child);
                // Rules: only one type node(ListItem or ListItemGroup) can exist in LazyForEach.
                CalculateLazyForEachNode(lazyForEach);
            } else {
                CalculateUINode(child);
            }
        }
    }

    RefPtr<FrameNode> GetListFrameNode(RefPtr<LazyForEachNode> node) const
    {
        auto parent = node->GetParent();
        RefPtr<FrameNode> frameNode = AceType::DynamicCast<FrameNode>(parent);
        while (parent && (!frameNode)) {
            parent = parent->GetParent();
            frameNode = AceType::DynamicCast<FrameNode>(parent);
        }
        return frameNode;
    }

    std::optional<bool> GetLazyForEachChildIsGroup(RefPtr<LazyForEachNode> node)
    {
        std::optional<bool> isGroup;
        auto children = node->GetChildren();
        if (children.size() > 0) {
            auto child = children.begin();
            while (child != children.end() && !((*child)->GetFrameChildByIndex(0, false))) {
                child++;
            }
            auto frameNode = AceType::DynamicCast<FrameNode>((*child)->GetFrameChildByIndex(0, false));
            if (frameNode) {
                isGroup = frameNode->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
            }
        }
        if (!(isGroup.has_value())) {
            auto listNode = GetListFrameNode(node);
            CHECK_NULL_RETURN(listNode, isGroup);
            auto wrapper = listNode->GetOrCreateChildByIndex(curIndex_);
            CHECK_NULL_RETURN(wrapper, isGroup);
            isGroup = wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
        }
        return isGroup;
    }

    void CalculateLazyForEachNode(RefPtr<LazyForEachNode> node)
    {
        int32_t count = node->FrameCount();
        if (count <= 0) {
            return;
        }
        std::optional<bool> isGroup = GetLazyForEachChildIsGroup(node);
        if (!(isGroup.has_value())) {
            TAG_LOGW(AceLogTag::ACE_LIST,
                "ListPositionMap Conflict: LazyForEach FrameCount > 0, but get child type failed.");
            isGroup = false;
        }
        while (count--) {
            isGroup.value() ? CalculateGroupNode() : CalculateListItemNode();
        }
        return;
    }

    void CalculateFrameNode(RefPtr<FrameNode> frameNode)
    {
        CHECK_NULL_VOID(frameNode);
        auto listItemGroupPatten = frameNode->GetPattern<ListItemGroupPattern>();
        if (listItemGroupPatten) {
            CalculateGroupNode();
        } else {
            CalculateListItemNode();
        }
    }

    void CalculateListItemNode()
    {
        curRowHeight_ = std::max(curRowHeight_, childrenSize_->GetChildSize(curIndex_));
        curLine_++;
        if (curLine_ == lanes_ || curIndex_ == totalItemCount_ - 1) {
            while (curLine_) {
                curLine_--;
                posMap_[curIndex_ - curLine_] = { totalHeight_, curRowHeight_ };
            }
            totalHeight_ += (curRowHeight_ + space_);
            curRowHeight_ = 0.0f;
        }
        curIndex_++;
    }

    void CalculateGroupNode()
    {
        if (curLine_ > 0) {
            while (curLine_) {
                curLine_--;
                posMap_[curIndex_ - 1 - curLine_] = { totalHeight_, curRowHeight_ };
            }
            totalHeight_ += (curRowHeight_ + space_);
            curRowHeight_ = 0.0f;
        }
        curRowHeight_ = childrenSize_->GetChildSize(curIndex_);
        posMap_[curIndex_] = { totalHeight_, curRowHeight_ };
        totalHeight_ += (curRowHeight_ + space_);
        curLine_ = 0;
        curRowHeight_ = 0.0f;
        curIndex_++;
    }

    void PosMapRecalculate(LayoutWrapper* layoutWrapper)
    {
        curIndex_ = 0;
        curLine_ = 0;
        totalHeight_ = 0.0f;
        curRowHeight_ = 0.0f;
        if (lanes_ == 1) {
            for (int32_t index = 0; index < totalItemCount_; index++) {
                curRowHeight_ = childrenSize_->GetChildSize(index);
                posMap_[index] = { totalHeight_, curRowHeight_ };
                totalHeight_ += (curRowHeight_ + space_);
            }
            totalHeight_ -= space_;
        } else {
            auto listNode = layoutWrapper->GetHostNode();
            CHECK_NULL_VOID(listNode);
            CalculateUINode(listNode);
            totalHeight_ -= space_;
        }
    }

    void GroupPosMapRecalculate()
    {
        curIndex_ = 0;
        totalHeight_ = headerSize_;
        curRowHeight_ = 0.0f;
        curLine_ = 0;

        for (int32_t index = 0; index < totalItemCount_;) {
            while (curLine_ < lanes_) {
                curRowHeight_ = std::max(curRowHeight_, childrenSize_->GetChildSize(index + curLine_));
                curLine_++;
            }
            curLine_ = 0;
            int32_t curRowEndIndex = std::min(index + lanes_ - 1, totalItemCount_ - 1);
            while (index <= curRowEndIndex) {
                posMap_[index++] = { totalHeight_, curRowHeight_ };
            }
            totalHeight_ += (curRowHeight_ + space_);
            curRowHeight_ = 0.0f;
        }
        totalHeight_ = totalHeight_ - space_ + footerSize_;
    }

    void PosMapUpdateAllSize()
    {
        float curPos = 0.0f;
        for (int32_t index = 0; index < totalItemCount_; index++) {
            posMap_[index] = { curPos, curRowHeight_ };
        }
    }

    void UpdatePosMap(LayoutWrapper* layoutWrapper, int32_t lanes, float space,
        RefPtr<ListChildrenMainSize>& childrenSize)
    {
        totalItemCount_ = layoutWrapper->GetTotalChildCount();
        childrenSize_ = childrenSize;
        if (lanes != lanes_) {
            dirty_ |= LIST_UPDATE_LANES;
            lanes_ = lanes;
        }
        if (!NearEqual(space, space_)) {
            dirty_ |= LIST_UPDATE_SPACE;
            space_ = space;
        }
        switch (CheckPosMapUpdateRule()) {
            case ListPosMapUpdate::NO_CHANGE:
                break;
            case ListPosMapUpdate::UPDATE_ALL_SIZE:
                PosMapRecalculate(layoutWrapper);
                break;
            case ListPosMapUpdate::RE_CALCULATE:
            default:
                PosMapRecalculate(layoutWrapper);
        }
        ClearDirty();
    }

    void UpdateGroupPosMap(int32_t totalCount, int32_t lanes, float space,
        RefPtr<ListChildrenMainSize>& childrenSize, float headerSize, float footerSize)
    {
        totalItemCount_ = totalCount;
        childrenSize_ = childrenSize;
        if (lanes != lanes_) {
            dirty_ |= LIST_UPDATE_LANES;
            lanes_ = lanes;
        }
        if (!NearEqual(space, space_)) {
            dirty_ |= LIST_UPDATE_SPACE;
            space_ = space;
        }
        if (!NearEqual(headerSize, headerSize_) || !NearEqual(footerSize, footerSize_)) {
            dirty_ |= LIST_GROUP_UPDATE_HEADER_FOOTER;
            headerSize_ = headerSize;
            footerSize_ = footerSize;
        }
        if (totalItemCount_ <= 0) {
            totalHeight_ = 0;
            ClearPosMap();
            ClearDirty();
            return;
        }
        switch (CheckPosMapUpdateRule()) {
            case ListPosMapUpdate::NO_CHANGE:
                break;
            case ListPosMapUpdate::UPDATE_ALL_SIZE:
                GroupPosMapRecalculate();
                break;
            case ListPosMapUpdate::RE_CALCULATE:
            default:
                GroupPosMapRecalculate();
        }
        ClearDirty();
    }

    float GetPos(int32_t startIndex, float startPos)
    {
        return posMap_[startIndex].mainPos - startPos;
    }

    float GetGroupLayoutOffset(int32_t startIndex, float startPos)
    {
        return posMap_[startIndex].mainPos - startPos;
    }

    void OptimizeBeforeMeasure(int32_t& beginIndex, float& beginPos, const float offset, const float contentSize)
    {
        if (NearZero(offset) || GreatOrEqual(contentSize, totalHeight_)) {
            return;
        }
        float chainOffset = chainOffsetFunc_ ? chainOffsetFunc_(beginIndex) : 0.0f;
        if (Positive(offset)) {
            float criticalPos = offset;
            std::pair<int32_t, float> rowInfo = GetRowEndIndexAndHeight(beginIndex);
            while (!NearEqual(posMap_[beginIndex].mainPos + rowInfo.second, totalHeight_) &&
                LessNotEqual(beginPos + rowInfo.second + chainOffset, criticalPos)) {
                beginIndex = rowInfo.first + 1;
                beginPos += (rowInfo.second + space_);
                rowInfo = GetRowEndIndexAndHeight(beginIndex);
                chainOffset = chainOffsetFunc_ ? chainOffsetFunc_(beginIndex) : 0.0f;
            }
        } else {
            float criticalPos = offset + contentSize;
            std::pair<int32_t, float> rowInfo = GetRowEndIndexAndHeight(beginIndex);
            while (Positive(posMap_[beginIndex].mainPos) &&
                GreatNotEqual(beginPos - rowInfo.second + chainOffset, criticalPos)) {
                beginIndex = GetRowStartIndex(beginIndex) - 1;
                beginPos -= (rowInfo.second + space_);
                rowInfo = GetRowEndIndexAndHeight(beginIndex);
                chainOffset = chainOffsetFunc_ ? chainOffsetFunc_(beginIndex) : 0.0f;
            }
        }
    }

    void SetChainOffsetCallback(std::function<float(int32_t)> func)
    {
        chainOffsetFunc_ = std::move(func);
    }

    int32_t GetRowStartIndex(const int32_t input)
    {
        int32_t startIndex = input;
        while (startIndex > 0 && NearEqual(posMap_[startIndex].mainPos, posMap_[startIndex - 1].mainPos)) {
            startIndex--;
        }
        return startIndex;
    }

    std::pair<int32_t, float> GetRowEndIndexAndHeight(const int32_t input)
    {
        int32_t endIndex = input;
        float rowHeight = 0.0f;
        while (endIndex < (totalItemCount_ - 1) &&
            NearEqual(posMap_[endIndex].mainPos, posMap_[endIndex + 1].mainPos)) {
            endIndex++;
        }
        if (endIndex == totalItemCount_ - 1) {
            rowHeight = totalHeight_ - posMap_[endIndex].mainPos;
        } else {
            rowHeight = posMap_[endIndex + 1].mainPos  - posMap_[endIndex].mainPos - space_;
        }
        return { endIndex, rowHeight };
    }
private:
    std::map<int32_t, PositionInfo> posMap_;
    RefPtr<ListChildrenMainSize> childrenSize_;
    std::function<float(int32_t)> chainOffsetFunc_;
    ListChangeFlag dirty_ = LIST_NO_CHANGE;
    int32_t totalItemCount_ = 0;
    int32_t lanes_ = -1;
    int32_t curLine_ = 0;
    int32_t curIndex_ = 0;
    float totalHeight_ = 0.0f;
    float curRowHeight_ = 0.0f;
    float space_ = 0.0f;
    float headerSize_ = 0.0f;
    float footerSize_ = 0.0f;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_POSITION_MAP_H