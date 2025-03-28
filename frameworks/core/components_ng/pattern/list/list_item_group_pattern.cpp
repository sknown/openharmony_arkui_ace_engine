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

#include "core/components_ng/pattern/list/list_item_group_pattern.h"

#include "base/log/dump_log.h"
#include "core/components/list/list_item_theme.h"
#include "core/components_ng/pattern/list/list_item_group_layout_algorithm.h"
#include "core/components_ng/pattern/list/list_item_group_paint_method.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

void ListItemGroupPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (listItemGroupStyle_ == V2::ListItemGroupStyle::CARD) {
        SetListItemGroupDefaultAttributes(host);
    }
}

void ListItemGroupPattern::OnColorConfigurationUpdate()
{
    if (listItemGroupStyle_ != V2::ListItemGroupStyle::CARD) {
        return;
    }
    auto itemGroupNode = GetHost();
    CHECK_NULL_VOID(itemGroupNode);
    auto renderContext = itemGroupNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto pipeline = itemGroupNode->GetContext();
    CHECK_NULL_VOID(pipeline);
    auto listItemGroupTheme = pipeline->GetTheme<ListItemTheme>();
    CHECK_NULL_VOID(listItemGroupTheme);

    renderContext->UpdateBackgroundColor(listItemGroupTheme->GetItemGroupDefaultColor());
}

void ListItemGroupPattern::SetListItemGroupDefaultAttributes(const RefPtr<FrameNode>& itemGroupNode)
{
    auto renderContext = itemGroupNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto layoutProperty = itemGroupNode->GetLayoutProperty<ListItemGroupLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);

    auto pipeline = GetContext();
    CHECK_NULL_VOID(pipeline);
    auto listItemGroupTheme = pipeline->GetTheme<ListItemTheme>();
    CHECK_NULL_VOID(listItemGroupTheme);

    renderContext->UpdateBackgroundColor(listItemGroupTheme->GetItemGroupDefaultColor());

    MarginProperty itemGroupMargin;
    itemGroupMargin.left = CalcLength(listItemGroupTheme->GetItemGroupDefaultLeftMargin());
    itemGroupMargin.right = CalcLength(listItemGroupTheme->GetItemGroupDefaultRightMargin());
    layoutProperty->UpdateMargin(itemGroupMargin);

    PaddingProperty itemGroupPadding;
    itemGroupPadding.left = CalcLength(listItemGroupTheme->GetItemGroupDefaultPadding().Left());
    itemGroupPadding.right = CalcLength(listItemGroupTheme->GetItemGroupDefaultPadding().Right());
    itemGroupPadding.top = CalcLength(listItemGroupTheme->GetItemGroupDefaultPadding().Top());
    itemGroupPadding.bottom = CalcLength(listItemGroupTheme->GetItemGroupDefaultPadding().Bottom());
    layoutProperty->UpdatePadding(itemGroupPadding);

    renderContext->UpdateBorderRadius(listItemGroupTheme->GetItemGroupDefaultBorderRadius());
}

void ListItemGroupPattern::DumpAdvanceInfo()
{
    DumpLog::GetInstance().AddDesc("itemStartIndex:" + std::to_string(itemStartIndex_));
    DumpLog::GetInstance().AddDesc("itemTotalCount:" + std::to_string(itemTotalCount_));
    DumpLog::GetInstance().AddDesc("itemDisplayEndIndex:" + std::to_string(itemDisplayEndIndex_));
    DumpLog::GetInstance().AddDesc("itemDisplayStartIndex:" + std::to_string(itemDisplayStartIndex_));
    DumpLog::GetInstance().AddDesc("headerMainSize:" + std::to_string(headerMainSize_));
    DumpLog::GetInstance().AddDesc("footerMainSize:" + std::to_string(footerMainSize_));
    DumpLog::GetInstance().AddDesc("spaceWidth:" + std::to_string(spaceWidth_));
    DumpLog::GetInstance().AddDesc("lanes:" + std::to_string(lanes_));
    DumpLog::GetInstance().AddDesc("laneGutter:" + std::to_string(laneGutter_));
    DumpLog::GetInstance().AddDesc("startHeaderPos:" + std::to_string(startHeaderPos_));
    DumpLog::GetInstance().AddDesc("endFooterPos:" + std::to_string(endFooterPos_));
}

RefPtr<LayoutAlgorithm> ListItemGroupPattern::CreateLayoutAlgorithm()
{
    CalculateItemStartIndex();
    auto layoutAlgorithm = MakeRefPtr<ListItemGroupLayoutAlgorithm>(headerIndex_, footerIndex_, itemStartIndex_);
    layoutAlgorithm->SetItemsPosition(itemPosition_);
    layoutAlgorithm->SetLayoutedItemInfo(layoutedItemInfo_);
    if (childrenSize_ && ListChildrenSizeExist()) {
        if (!posMap_) {
            posMap_ = MakeRefPtr<ListPositionMap>();
        }
        layoutAlgorithm->SetListChildrenMainSize(childrenSize_);
        layoutAlgorithm->SetListPositionMap(posMap_);
    }
    return layoutAlgorithm;
}

RefPtr<NodePaintMethod> ListItemGroupPattern::CreateNodePaintMethod()
{
    auto layoutProperty = GetLayoutProperty<ListItemGroupLayoutProperty>();
    V2::ItemDivider itemDivider;
    auto divider = layoutProperty->GetDivider().value_or(itemDivider);
    auto drawVertical = (axis_ == Axis::HORIZONTAL);
    ListItemGroupPaintInfo listItemGroupPaintInfo { layoutDirection_, mainSize_, drawVertical, lanes_,
        spaceWidth_, laneGutter_, itemTotalCount_ };
    return MakeRefPtr<ListItemGroupPaintMethod>(divider, listItemGroupPaintInfo, itemPosition_, pressedItem_);
}

bool ListItemGroupPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto layoutAlgorithm = DynamicCast<ListItemGroupLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithm, false);
    itemPosition_ = layoutAlgorithm->GetItemPosition();
    spaceWidth_ = layoutAlgorithm->GetSpaceWidth();
    lanes_ = layoutAlgorithm->GetLanes();
    axis_ = layoutAlgorithm->GetAxis();
    layoutDirection_ = layoutAlgorithm->GetLayoutDirection();
    mainSize_ = layoutAlgorithm->GetMainSize();
    laneGutter_ = layoutAlgorithm->GetLaneGutter();
    itemDisplayEndIndex_ = layoutAlgorithm->GetEndIndex();
    itemDisplayStartIndex_ = layoutAlgorithm->GetStartIndex();
    itemTotalCount_ = layoutAlgorithm->GetTotalItemCount();
    headerMainSize_ = layoutAlgorithm->GetHeaderMainSize();
    footerMainSize_ = layoutAlgorithm->GetFooterMainSize();
    layoutedItemInfo_ = layoutAlgorithm->GetLayoutedItemInfo();
    startHeaderPos_ = layoutAlgorithm->GetStartHeaderPos();
    endFooterPos_ = layoutAlgorithm->GetEndFooterPos();
    layouted_ = true;
    CheckListDirectionInCardStyle();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto accessibilityProperty = host->GetAccessibilityProperty<ListItemGroupAccessibilityProperty>();
    if (accessibilityProperty != nullptr) {
        accessibilityProperty->SetCollectionItemCounts(layoutAlgorithm->GetTotalItemCount());
    }
    auto listLayoutProperty = host->GetLayoutProperty<ListItemGroupLayoutProperty>();
    return listLayoutProperty && listLayoutProperty->GetDivider().has_value() && !itemPosition_.empty();
}

float ListItemGroupPattern::GetEstimateOffset(float height, const std::pair<float, float>& targetPos) const
{
    if (layoutedItemInfo_.has_value() && layoutedItemInfo_.value().startIndex > 0) {
        float averageHeight = 0.0f;
        float estimateHeight = GetEstimateHeight(averageHeight);
        return height + estimateHeight - targetPos.second;
    }
    return height - targetPos.first;
}

float ListItemGroupPattern::GetEstimateHeight(float& averageHeight) const
{
    auto layoutProperty = GetLayoutProperty<ListItemGroupLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, 0.0f);
    auto visible = layoutProperty->GetVisibility().value_or(VisibleType::VISIBLE);
    if (visible == VisibleType::GONE) {
        return 0.0f;
    }
    if (layoutedItemInfo_.has_value()) {
        auto totalHeight = (layoutedItemInfo_.value().endPos - layoutedItemInfo_.value().startPos + spaceWidth_);
        auto itemCount = layoutedItemInfo_.value().endIndex - layoutedItemInfo_.value().startIndex + 1;
        averageHeight = totalHeight / itemCount;
    }
    if (layouted_) {
        if (itemTotalCount_ > 0) {
            return itemTotalCount_ * averageHeight + headerMainSize_ + footerMainSize_ - spaceWidth_;
        } else {
            return headerMainSize_ + footerMainSize_;
        }
    }
    auto host = GetHost();
    auto totalItem = host->GetTotalChildCount();
    return averageHeight * totalItem;
}

void ListItemGroupPattern::CheckListDirectionInCardStyle()
{
    if (axis_ == Axis::HORIZONTAL && listItemGroupStyle_ == V2::ListItemGroupStyle::CARD) {
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        RefPtr<FrameNode> listNode = AceType::DynamicCast<FrameNode>(host->GetParent());
        CHECK_NULL_VOID(listNode);
        auto listPattern = listNode->GetPattern<ListPattern>();
        CHECK_NULL_VOID(listPattern);
        listPattern->SetNeedToUpdateListDirectionInCardStyle(true);
    }
}

RefPtr<FrameNode> ListItemGroupPattern::GetListFrameNode() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto parent = host->GetParent();
    RefPtr<FrameNode> frameNode = AceType::DynamicCast<FrameNode>(parent);
    while (parent && !frameNode) {
        parent = parent->GetParent();
        frameNode = AceType::DynamicCast<FrameNode>(parent);
    }
    return frameNode;
}

bool ListItemGroupPattern::ListChildrenSizeExist()
{
    RefPtr<FrameNode> listNode = GetListFrameNode();
    CHECK_NULL_RETURN(listNode, false);
    auto listPattern = listNode->GetPattern<ListPattern>();
    CHECK_NULL_RETURN(listPattern, false);
    return listPattern->ListChildrenSizeExist();
}

RefPtr<ListChildrenMainSize> ListItemGroupPattern::GetOrCreateListChildrenMainSize()
{
    if (childrenSize_) {
        return childrenSize_;
    }
    childrenSize_ = AceType::MakeRefPtr<ListChildrenMainSize>();
    auto callback = [weakPattern = WeakClaim(this)](std::tuple<int32_t, int32_t, int32_t> change, ListChangeFlag flag) {
        auto pattern = weakPattern.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        context->AddBuildFinishCallBack([weakPattern, change, flag]() {
            auto pattern = weakPattern.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->OnChildrenSizeChanged(change, flag);
        });
        context->RequestFrame();
    };
    childrenSize_->SetOnDataChange(callback);
    return childrenSize_;
}

void ListItemGroupPattern::SetListChildrenMainSize(
    float defaultSize, const std::vector<float>& mainSize)
{
    childrenSize_ = AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, defaultSize);
    OnChildrenSizeChanged({ -1, -1, -1 }, LIST_UPDATE_CHILD_SIZE);
}

void ListItemGroupPattern::OnChildrenSizeChanged(std::tuple<int32_t, int32_t, int32_t> change, ListChangeFlag flag)
{
    if (!posMap_) {
        posMap_ = MakeRefPtr<ListPositionMap>();
    }
    posMap_->MarkDirty(flag);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
}

VisibleContentInfo ListItemGroupPattern::GetStartListItemIndex()
{
    bool isHeader = false;
    auto startHeaderMainSize = GetHeaderMainSize();
    auto startFooterMainSize = GetFooterMainSize();
    if (GetDisplayStartIndexInGroup() == 0) {
        auto startHeaderPos = startHeaderPos_;
        isHeader = (startHeaderPos + startHeaderMainSize) > 0 ? true : false;
    }
    auto startPositionSize = GetItemPosition().size();
    auto startItemIndexInGroup = GetDisplayStartIndexInGroup();
    auto startArea = ListItemGroupArea::IN_LIST_ITEM_AREA;
    if (startPositionSize == 0 && startFooterMainSize > 0) {
        startArea = ListItemGroupArea::IN_FOOTER_AREA;
        startItemIndexInGroup = -1;
    }
    if (GetDisplayStartIndexInGroup() == 0 && isHeader && startHeaderMainSize > 0) {
        startArea = ListItemGroupArea::IN_HEADER_AREA;
        startItemIndexInGroup = -1;
    }
    if (startHeaderMainSize == 0 && startFooterMainSize == 0 && GetTotalItemCount() == 0) {
        startArea = ListItemGroupArea::NONE_AREA;
    }
    VisibleContentInfo startInfo = { startArea, startItemIndexInGroup };
    return startInfo;
}

VisibleContentInfo ListItemGroupPattern::GetEndListItemIndex()
{
    bool isFooter = endFooterPos_ < 0 ? true : false;
    auto endHeaderMainSize = GetHeaderMainSize();
    auto endFooterMainSize = GetFooterMainSize();
    auto endPositionSize = GetItemPosition().size();
    auto endItemIndexInGroup = GetDisplayEndIndexInGroup();
    auto endArea = ListItemGroupArea::IN_LIST_ITEM_AREA;
    if (endPositionSize == 0 && endHeaderMainSize > 0) {
        endArea = ListItemGroupArea::IN_HEADER_AREA;
        endItemIndexInGroup = -1;
    }
    if (isFooter && endFooterMainSize > 0) {
        endArea = ListItemGroupArea::IN_FOOTER_AREA;
        endItemIndexInGroup = -1;
    }
    if (endHeaderMainSize == 0 && endFooterMainSize == 0 && GetTotalItemCount() == 0) {
        endArea = ListItemGroupArea::NONE_AREA;
    }
    VisibleContentInfo endInfo = { endArea, endItemIndexInGroup };
    return endInfo;
}

void ListItemGroupPattern::ResetChildrenSize()
{
    if (childrenSize_) {
        childrenSize_ = nullptr;
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        host->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
        OnChildrenSizeChanged({ -1, -1, -1 }, LIST_UPDATE_CHILD_SIZE);
    }
}

void ListItemGroupPattern::CalculateItemStartIndex()
{
    int32_t headerIndex = -1;
    int32_t footerIndex = -1;
    int32_t itemStartIndex = 0;
    auto header = header_.Upgrade();
    if (header) {
        auto count = header->FrameCount();
        if (count > 0) {
            headerIndex = itemStartIndex;
            itemStartIndex += count;
        }
    }
    auto footer = footer_.Upgrade();
    if (footer) {
        int32_t count = footer->FrameCount();
        if (count > 0) {
            footerIndex = itemStartIndex;
            itemStartIndex += count;
        }
    }
    headerIndex_ = headerIndex;
    footerIndex_ = footerIndex;
    itemStartIndex_ = itemStartIndex;
}

int32_t ListItemGroupPattern::GetForwardCachedIndex(int32_t cacheCount)
{
    int32_t endIndex = itemPosition_.empty() ? -1 : itemPosition_.rbegin()->first;
    int32_t limit = std::min(endIndex + cacheCount, itemTotalCount_ - 1);
    forwardCachedIndex_ = std::clamp(forwardCachedIndex_, endIndex, limit);
    return forwardCachedIndex_;
}

int32_t ListItemGroupPattern::GetBackwardCachedIndex(int32_t cacheCount)
{
    int32_t startIndex = itemPosition_.empty() ? itemTotalCount_ : itemPosition_.begin()->first;
    int32_t limit = std::max(startIndex - cacheCount, 0);
    backwardCachedIndex_ = std::clamp(backwardCachedIndex_, limit, startIndex);
    return backwardCachedIndex_;
}

void ListItemGroupPattern::LayoutCache(const LayoutConstraintF& constraint,
    bool forward, int64_t deadline, int32_t cached)
{
    ACE_SCOPED_TRACE("Group LayoutCache:%d,%d", forward, cached);
    auto listNode = GetListFrameNode();
    CHECK_NULL_VOID(listNode);
    auto listLayoutProperty = listNode->GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_VOID(listLayoutProperty);
    auto cacheCount = listLayoutProperty->GetCachedCountValue(1) - cached;
    if (cacheCount < 1) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    CalculateItemStartIndex();
    itemTotalCount_ = host->GetTotalChildCount() - itemStartIndex_;
    if (itemTotalCount_ == 0) {
        return;
    }
    if (forward) {
        int32_t endIndex = itemPosition_.empty() ? -1 : itemPosition_.rbegin()->first;
        int32_t limit = std::min(endIndex + cacheCount, itemTotalCount_ - 1);
        int32_t currentIndex = GetForwardCachedIndex(cacheCount) + 1;
        for (; currentIndex <= limit; currentIndex++) {
            auto item = host->GetOrCreateChildByIndex(currentIndex + itemStartIndex_, false, true);
            if (!item) {
                break;
            }
        }
        forwardCachedIndex_ = std::min(currentIndex - 1, limit);
    } else {
        int32_t startIndex = itemPosition_.empty() ? itemTotalCount_ : itemPosition_.begin()->first;
        int32_t limit = std::max(startIndex - cacheCount, 0);
        int32_t currentIndex = GetBackwardCachedIndex(cacheCount) - 1;
        for (; currentIndex >= limit; currentIndex--) {
            auto item = host->GetOrCreateChildByIndex(currentIndex + itemStartIndex_, false, true);
            if (!item) {
                break;
            }
        }
        backwardCachedIndex_ = std::max(currentIndex + 1, limit);
    }
}
} // namespace OHOS::Ace::NG
