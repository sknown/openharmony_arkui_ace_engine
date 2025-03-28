/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/grid/grid_scroll/grid_scroll_layout_algorithm.h"

#include "base/geometry/axis.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/alignment.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/pattern/grid/grid_item_layout_property.h"
#include "core/components_ng/pattern/grid/grid_pattern.h"
#include "core/components_ng/pattern/grid/grid_utils.h"
#include "core/components_ng/pattern/scrollable/scrollable_properties.h"
#include "core/components_ng/pattern/scrollable/scrollable_utils.h"
#include "core/components_ng/pattern/text_field/text_field_manager.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/property/templates_parser.h"
#include "core/pipeline_ng/pipeline_context.h"
namespace OHOS::Ace::NG {
void GridScrollLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto gridLayoutProperty = AceType::DynamicCast<GridLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(gridLayoutProperty);

    // Pre-recycle
    ScrollableUtils::RecycleItemsOutOfBoundary(gridLayoutInfo_.axis_,
        gridLayoutInfo_.currentOffset_ - gridLayoutInfo_.prevOffset_, gridLayoutInfo_.startIndex_,
        gridLayoutInfo_.endIndex_, layoutWrapper);

    // Step1: Decide size of Grid
    Axis axis = gridLayoutInfo_.axis_;
    auto idealSize = CreateIdealSize(
        gridLayoutProperty->GetLayoutConstraint().value(), axis, gridLayoutProperty->GetMeasureType(), true);
    if (NearZero(GetMainAxisSize(idealSize, axis))) {
        TAG_LOGW(AceLogTag::ACE_GRID, "size of main axis value is 0, please check");
        return;
    }
    bool matchChildren = GreaterOrEqualToInfinity(GetMainAxisSize(idealSize, axis));
    layoutWrapper->GetGeometryNode()->SetFrameSize(idealSize);
    MinusPaddingToSize(gridLayoutProperty->CreatePaddingAndBorder(), idealSize);
    gridLayoutInfo_.contentEndPadding_ = ScrollableUtils::CheckHeightExpansion(gridLayoutProperty, axis);
    idealSize.AddHeight(gridLayoutInfo_.contentEndPadding_);
    auto&& safeAreaOpts = gridLayoutProperty->GetSafeAreaExpandOpts();
    expandSafeArea_ = safeAreaOpts && safeAreaOpts->Expansive();

    InitialItemsCrossSize(gridLayoutProperty, idealSize, gridLayoutInfo_.childrenCount_);

    // Step2: Measure children that can be displayed in viewport of Grid
    float mainSize = GetMainAxisSize(idealSize, axis);
    float crossSize = GetCrossAxisSize(idealSize, axis);
    if (!NearEqual(mainSize, gridLayoutInfo_.lastMainSize_)) {
        gridLayoutInfo_.ResetPositionFlags();
        UpdateOffsetOnVirtualKeyboardHeightChange(layoutWrapper, mainSize);
        UpdateOffsetOnHeightChangeDuringAnimation(layoutWrapper, mainSize);
    }
    FillGridViewportAndMeasureChildren(mainSize, crossSize, layoutWrapper);

    // update cache info.
    layoutWrapper->SetCacheCount(static_cast<int32_t>(gridLayoutProperty->GetCachedCountValue(1) * crossCount_));

    gridLayoutInfo_.lastMainSize_ = mainSize;
    gridLayoutInfo_.lastCrossSize_ = crossSize;
    AdaptToChildMainSize(layoutWrapper, gridLayoutProperty, mainSize, idealSize, matchChildren);

    // reset offsetEnd after scroll to moveToEndLineIndex_
    gridLayoutInfo_.offsetEnd_ = moveToEndLineIndex_ > 0
                                     ? (gridLayoutInfo_.endIndex_ + 1 >= gridLayoutInfo_.childrenCount_)
                                     : gridLayoutInfo_.offsetEnd_;

    if (SystemProperties::GetGridCacheEnabled()) {
        FillCacheLineAtEnd(mainSize, crossSize, layoutWrapper);
        if (!predictBuildList_.empty()) {
            PostIdleTask(layoutWrapper->GetHostNode(),
                { predictBuildList_, cachedChildConstraint_, itemsCrossSize_, crossGap_ });
            predictBuildList_.clear();
        }
    }
}

void GridScrollLayoutAlgorithm::UpdateOffsetOnVirtualKeyboardHeightChange(LayoutWrapper* layoutWrapper, float mainSize)
{
    if (GreatOrEqual(mainSize, gridLayoutInfo_.lastMainSize_)) {
        return;
    }
    // only need to offset vertical grid
    if (gridLayoutInfo_.axis_ != Axis::VERTICAL) {
        return;
    }

    auto grid = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(grid);
    auto focusHub = grid->GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    // textField not in Grid
    if (!focusHub->IsCurrentFocus()) {
        return;
    }

    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto textFieldManager = AceType::DynamicCast<TextFieldManagerNG>(context->GetTextFieldManager());
    CHECK_NULL_VOID(textFieldManager);
    // only when textField is onFocus
    auto focused = textFieldManager->GetOnFocusTextField().Upgrade();
    CHECK_NULL_VOID(focused);
    auto position = textFieldManager->GetClickPosition().GetY();
    auto gridOffset = grid->GetTransformRelativeOffset();
    auto offset = mainSize + gridOffset.GetY() - position;
    if (LessOrEqual(offset, 0.0)) {
        // negative offset to scroll down
        auto lineHeight = gridLayoutInfo_.GetAverageLineHeight();
        if (GreatNotEqual(lineHeight, 0)) {
            offset = floor(offset / lineHeight) * lineHeight;
        }
        gridLayoutInfo_.currentOffset_ += offset;
        TAG_LOGI(AceLogTag::ACE_GRID, "update offset on virtual keyboard height change, %{public}f", offset);
    }
}

void GridScrollLayoutAlgorithm::AdaptToChildMainSize(LayoutWrapper* layoutWrapper,
    RefPtr<GridLayoutProperty>& gridLayoutProperty, float mainSize, SizeF idealSize, bool matchChildren)
{
    if (!matchChildren) {
        // grid with columnsTemplate/rowsTemplate and maxCount
        if (!gridLayoutProperty->HasMaxCount()) {
            return;
        }
        std::optional<CalcLength> mainAxisIdealSize;
        const auto& selfLayoutConstraint = gridLayoutProperty->GetCalcLayoutConstraint();
        if (selfLayoutConstraint && selfLayoutConstraint->selfIdealSize.has_value()) {
            mainAxisIdealSize = axis_ == Axis::HORIZONTAL ? selfLayoutConstraint->selfIdealSize->Width()
                                                          : selfLayoutConstraint->selfIdealSize->Height();
        }

        if (mainAxisIdealSize.has_value()) {
            return;
        }
    }

    auto lengthOfItemsInViewport = gridLayoutInfo_.GetTotalHeightOfItemsInView(mainGap_);
    auto gridMainSize = std::min(lengthOfItemsInViewport, mainSize);
    gridMainSize = std::max(gridMainSize, GetMainAxisSize(gridLayoutProperty->GetLayoutConstraint()->minSize, axis_));
    idealSize.SetMainSize(gridMainSize, axis_);
    AddPaddingToSize(gridLayoutProperty->CreatePaddingAndBorder(), idealSize);
    layoutWrapper->GetGeometryNode()->SetFrameSize(idealSize);
    gridLayoutInfo_.lastMainSize_ = gridMainSize;
    TAG_LOGI(AceLogTag::ACE_GRID, "gridMainSize:%{public}f", gridMainSize);
}

void GridScrollLayoutAlgorithm::UpdateOffsetOnHeightChangeDuringAnimation(LayoutWrapper* layoutWrapper, float mainSize)
{
    // If only the height of the Grid is changed, keep the prevOffset_ and currentOffset_ equal.
    if (scrollSource_ == SCROLL_FROM_NONE) {
        gridLayoutInfo_.prevOffset_ = gridLayoutInfo_.currentOffset_;
    }
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto pattern = host->GetPattern<GridPattern>();
    CHECK_NULL_VOID(pattern);
    if (pattern->IsScrollableSpringMotionRunning()) {
        gridLayoutInfo_.currentOffset_ += (mainSize - gridLayoutInfo_.lastMainSize_);
    }
}

void GridScrollLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    auto gridLayoutProperty = AceType::DynamicCast<GridLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(gridLayoutProperty);
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    childFrameOffset_ = OffsetF(0.0f, padding.top.value_or(0.0f));
    childFrameOffset_ += OffsetF(0.0f, gridLayoutInfo_.currentOffset_, axis_);
    bool isRtl = layoutWrapper->GetLayoutProperty()->GetNonAutoLayoutDirection() == TextDirection::RTL;
    float prevLineHeight = 0.0f;
    int32_t startIndex = -1;
    int32_t endIndex = -1;
    if (gridLayoutInfo_.hasMultiLineItem_) {
        layoutWrapper->RemoveAllChildInRenderTree();
    }
    LargeItemForwardLineHeight(gridLayoutInfo_.startMainLineIndex_, layoutWrapper);
    for (auto i = gridLayoutInfo_.startMainLineIndex_; i <= gridLayoutInfo_.endMainLineIndex_; i++) {
        const auto& line = gridLayoutInfo_.gridMatrix_.find(i);
        if (line == gridLayoutInfo_.gridMatrix_.end()) {
            continue;
        }

        auto prevLineOffset = OffsetF(0.0f, prevLineHeight, axis_);
        if (line->second.empty()) {
            TAG_LOGW(AceLogTag::ACE_GRID, "line %{public}d should not be empty, please check.", line->first);
            break;
        }
        int32_t itemIdex = -1;
        float lineHeight = gridLayoutInfo_.lineHeightMap_[line->first];
        Alignment align = axis_ == Axis::VERTICAL ? Alignment::TOP_CENTER : Alignment::CENTER_LEFT;
        if (gridLayoutProperty->GetPositionProperty()) {
            align = gridLayoutProperty->GetPositionProperty()->GetAlignment().value_or(align);
        }
        for (auto iter = line->second.begin(); iter != line->second.end(); iter++) {
            // If item index is the same, must be the same GridItem, need't layout again.
            if (itemIdex == iter->second) {
                continue;
            }
            itemIdex = iter->second;
            auto crossIter = itemsCrossPosition_.find(itemIdex);
            if (crossIter == itemsCrossPosition_.end()) {
                TAG_LOGI(AceLogTag::ACE_GRID, "item %{public}d not in cross position", itemIdex);
                continue;
            }
            auto crossOffset = crossIter->second;
            auto offset = childFrameOffset_ + prevLineOffset;
            offset = CalculateLargeItemOffset(offset, itemIdex, i, iter->first);
            if (axis_ == Axis::VERTICAL) {
                offset.SetX(crossOffset);
            } else {
                offset.SetY(crossOffset);
            }
            auto wrapper = layoutWrapper->GetOrCreateChildByIndex(itemIdex);
            if (!wrapper) {
                continue;
            }
            startIndex = startIndex == -1 ? itemIdex : std::min(startIndex, itemIdex);
            endIndex = std::max(itemIdex, endIndex);
            auto frSize = itemsCrossSize_.at(iter->first);
            SizeF blockSize = SizeF(frSize, lineHeight, axis_);
            auto translate = OffsetF(0.0f, 0.0f);
            auto childSize = wrapper->GetGeometryNode()->GetMarginFrameSize();
            translate = Alignment::GetAlignPosition(blockSize, childSize, align);

            if (isRtl) {
                offset.SetX(size.Width() - offset.GetX() - childSize.Width());
            }
            offset += OffsetF(padding.left.value_or(0.0f), 0.0f);
            wrapper->GetGeometryNode()->SetMarginFrameOffset(offset + translate);
            if (gridLayoutInfo_.hasMultiLineItem_ || expandSafeArea_ || wrapper->CheckNeedForceMeasureAndLayout()) {
                wrapper->Layout();
            } else {
                SyncGeometry(wrapper);
            }
            auto layoutProperty = wrapper->GetLayoutProperty();
            CHECK_NULL_VOID(layoutProperty);
            auto gridItemLayoutProperty = AceType::DynamicCast<GridItemLayoutProperty>(layoutProperty);
            CHECK_NULL_VOID(gridItemLayoutProperty);
            gridItemLayoutProperty->UpdateMainIndex(line->first);
            gridItemLayoutProperty->UpdateCrossIndex(iter->first);
            UpdateRealGridItemPositionInfo(wrapper, line->first, iter->first);
            auto frameNode = AceType::DynamicCast<FrameNode>(wrapper);
            if (frameNode) {
                frameNode->MarkAndCheckNewOpIncNode();
            }
        }
        prevLineHeight += gridLayoutInfo_.lineHeightMap_[line->first] + mainGap_;
    }
    auto cacheCount = gridLayoutProperty->GetCachedCountValue(1);
    if (!gridLayoutInfo_.hasMultiLineItem_) {
        layoutWrapper->SetActiveChildRange(startIndex, endIndex, cacheCount * crossCount_, cacheCount * crossCount_);
    }
    gridLayoutInfo_.totalHeightOfItemsInView_ = gridLayoutInfo_.GetTotalHeightOfItemsInView(mainGap_);

    if (SystemProperties::GetGridCacheEnabled()) {
        if (!itemsCrossPosition_.empty() && cacheCount > 0) {
            LayoutCachedItem(layoutWrapper, cacheCount);
        }
    }
}

void GridScrollLayoutAlgorithm::LayoutCachedItem(LayoutWrapper* layoutWrapper, int32_t cacheCount)
{
    if (!gridLayoutInfo_.reachEnd_) {
        LayoutBackwardCachedLine(layoutWrapper, cacheCount);
    }
    if (!gridLayoutInfo_.reachStart_) {
        LayoutForwardCachedLine(layoutWrapper, cacheCount);
    }
}
void GridScrollLayoutAlgorithm::LayoutBackwardCachedLine(LayoutWrapper* layoutWrapper, int32_t cacheCount)
{
    auto gridLayoutProperty = AceType::DynamicCast<GridLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(gridLayoutProperty);
    auto prevLineHeight = gridLayoutInfo_.totalHeightOfItemsInView_;
    auto prevLineIter = gridLayoutInfo_.gridMatrix_.find(gridLayoutInfo_.endMainLineIndex_);
    std::map<int32_t, int32_t> prevLine;
    if (prevLineIter != gridLayoutInfo_.gridMatrix_.end()) {
        prevLine = prevLineIter->second;
    }
    for (int32_t i = 1; i <= cacheCount; i++) {
        auto currentLine = gridLayoutInfo_.endMainLineIndex_ + i;
        const auto& line = gridLayoutInfo_.gridMatrix_.find(currentLine);
        if (line == gridLayoutInfo_.gridMatrix_.end()) {
            break;
        }

        auto prevLineOffset = axis_ == Axis::VERTICAL ? OffsetF(0.0, prevLineHeight) : OffsetF(prevLineHeight, 0.0);
        if (line->second.empty()) {
            break;
        }
        int32_t itemIdex = -1;
        auto lineHeightIter = gridLayoutInfo_.lineHeightMap_.find(line->first);
        if (lineHeightIter == gridLayoutInfo_.lineHeightMap_.end()) {
            return;
        }
        float lineHeight = lineHeightIter->second;
        Alignment align = axis_ == Axis::VERTICAL ? Alignment::TOP_CENTER : Alignment::CENTER_LEFT;
        if (gridLayoutProperty->GetPositionProperty()) {
            align = gridLayoutProperty->GetPositionProperty()->GetAlignment().value_or(align);
        }
        for (auto iter = line->second.begin(); iter != line->second.end(); iter++) {
            // If item index is the same, must be the same GridItem, need't layout again.
            if (itemIdex == iter->second) {
                continue;
            }
            itemIdex = iter->second;
            auto crossIter = itemsCrossPosition_.find(itemIdex);
            if (crossIter == itemsCrossPosition_.end()) {
                continue;
            }
            bool prevLineNotContains =
                std::none_of(prevLine.begin(), prevLine.end(), [itemIdex](auto ite) { return ite.second == itemIdex; });
            if (!prevLineNotContains) {
                continue;
            }
            auto crossOffset = crossIter->second;
            auto offset = childFrameOffset_ + prevLineOffset;
            offset = CalculateLargeItemOffset(offset, itemIdex, currentLine, iter->first);
            if (axis_ == Axis::VERTICAL) {
                offset.SetX(crossOffset);
            } else {
                offset.SetY(crossOffset);
            }
            auto wrapper = layoutWrapper->GetChildByIndex(itemIdex, true);
            if (!wrapper || wrapper->CheckNeedForceMeasureAndLayout()) {
                continue;
            }
            auto frSize = itemsCrossSize_.at(iter->first);
            SizeF blockSize = gridLayoutProperty->IsVertical() ? SizeF(frSize, lineHeight) : SizeF(lineHeight, frSize);
            auto translate = OffsetF(0.0f, 0.0f);
            translate = Alignment::GetAlignPosition(blockSize, wrapper->GetGeometryNode()->GetMarginFrameSize(), align);

            wrapper->GetGeometryNode()->SetMarginFrameOffset(offset + translate);
            SyncGeometry(wrapper);
            wrapper->SetActive(false);
        }
        prevLineHeight += gridLayoutInfo_.lineHeightMap_[line->first] + mainGap_;
    }
}

void GridScrollLayoutAlgorithm::LayoutForwardCachedLine(LayoutWrapper* layoutWrapper, int32_t cacheCount)
{
    auto gridLayoutProperty = AceType::DynamicCast<GridLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(gridLayoutProperty);
    // if in spring animation and all nodes are located above the page,
    // startMainLineIndex will greater than endMainLineIndex
    auto currentLine = std::min(gridLayoutInfo_.startMainLineIndex_, gridLayoutInfo_.endMainLineIndex_);
    auto nextLineHeight = gridLayoutInfo_.lineHeightMap_[currentLine] + mainGap_;
    auto nextLineIter = gridLayoutInfo_.gridMatrix_.find(gridLayoutInfo_.startMainLineIndex_);
    std::map<int32_t, int32_t> nextLine;
    if (nextLineIter != gridLayoutInfo_.gridMatrix_.end()) {
        nextLine = nextLineIter->second;
    }
    for (int32_t i = 1; i <= cacheCount && currentLine - i >= 0; i++) {
        int32_t currLine = currentLine - i;
        const auto& line = gridLayoutInfo_.gridMatrix_.find(currLine);
        if (line == gridLayoutInfo_.gridMatrix_.end() || line->second.empty()) {
            break;
        }
        auto nextLineOffset = axis_ == Axis::VERTICAL ? OffsetF(0.0, nextLineHeight) : OffsetF(nextLineHeight, 0.0);
        int32_t itemIdex = -1;
        auto lineHeightIter = gridLayoutInfo_.lineHeightMap_.find(line->first);
        if (lineHeightIter == gridLayoutInfo_.lineHeightMap_.end()) {
            return;
        }
        float lineHeight = lineHeightIter->second;
        Alignment align = axis_ == Axis::VERTICAL ? Alignment::TOP_CENTER : Alignment::CENTER_LEFT;
        if (gridLayoutProperty->GetPositionProperty()) {
            align = gridLayoutProperty->GetPositionProperty()->GetAlignment().value_or(align);
        }
        for (auto iter = line->second.begin(); iter != line->second.end(); iter++) {
            // If item index is the same, must be the same GridItem, need't layout again.
            if (itemIdex == iter->second) {
                continue;
            }
            itemIdex = iter->second;
            auto crossIter = itemsCrossPosition_.find(itemIdex);
            if (crossIter == itemsCrossPosition_.end()) {
                continue;
            }
            bool nextLineNotContains =
                std::none_of(nextLine.begin(), nextLine.end(), [itemIdex](auto ite) { return ite.second == itemIdex; });
            if (!nextLineNotContains) {
                continue;
            }
            auto crossOffset = crossIter->second;
            auto offset = childFrameOffset_ - nextLineOffset;
            offset = CalculateLargeItemOffset(offset, itemIdex, currLine, iter->first);
            if (axis_ == Axis::VERTICAL) {
                offset.SetX(crossOffset);
            } else {
                offset.SetY(crossOffset);
            }
            auto wrapper = layoutWrapper->GetChildByIndex(itemIdex, true);
            if (!wrapper || wrapper->CheckNeedForceMeasureAndLayout()) {
                continue;
            }
            auto frSize = itemsCrossSize_.at(iter->first);
            SizeF blockSize = gridLayoutProperty->IsVertical() ? SizeF(frSize, lineHeight) : SizeF(lineHeight, frSize);
            auto translate = OffsetF(0.0f, 0.0f);
            translate = Alignment::GetAlignPosition(blockSize, wrapper->GetGeometryNode()->GetMarginFrameSize(), align);

            wrapper->GetGeometryNode()->SetMarginFrameOffset(offset + translate);
            SyncGeometry(wrapper);
            wrapper->SetActive(false);
        }
        nextLineHeight += gridLayoutInfo_.lineHeightMap_[currLine] + mainGap_;
    }
}

void GridScrollLayoutAlgorithm::SyncGeometry(RefPtr<LayoutWrapper>& wrapper)
{
    CHECK_NULL_VOID(wrapper);
    auto host = wrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    host->ForceSyncGeometryNode();
}

void GridScrollLayoutAlgorithm::InitialItemsCrossSize(
    const RefPtr<GridLayoutProperty>& layoutProperty, const SizeF& frameSize, int32_t childrenCount)
{
    itemsCrossSize_.clear();
    auto rowsTemplate = layoutProperty->GetRowsTemplate().value_or("");
    auto columnsTemplate = layoutProperty->GetColumnsTemplate().value_or("");
    axis_ = columnsTemplate.empty() ? Axis::HORIZONTAL : Axis::VERTICAL;
    auto scale = layoutProperty->GetLayoutConstraint()->scaleProperty;
    auto rowsGap = ConvertToPx(layoutProperty->GetRowsGap().value_or(0.0_vp), scale, frameSize.Height()).value_or(0);
    auto columnsGap =
        ConvertToPx(layoutProperty->GetColumnsGap().value_or(0.0_vp), scale, frameSize.Width()).value_or(0);
    mainGap_ = axis_ == Axis::HORIZONTAL ? columnsGap : rowsGap;
    crossGap_ = axis_ == Axis::VERTICAL ? columnsGap : rowsGap;
    auto padding = layoutProperty->CreatePaddingAndBorder();
    crossPaddingOffset_ = axis_ == Axis::HORIZONTAL ? padding.top.value_or(0) : 0.0f;

    auto crossSize = frameSize.CrossSize(axis_);
    std::vector<double> crossLens;
    std::pair<std::vector<double>, double> cross;
    if (!rowsTemplate.empty()) {
        cross = ParseTemplateArgs(GridUtils::ParseArgs(rowsTemplate), crossSize, crossGap_, childrenCount);
    } else {
        cross = ParseTemplateArgs(GridUtils::ParseArgs(columnsTemplate), crossSize, crossGap_, childrenCount);
    }
    crossLens = cross.first;
    crossGap_ = cross.second;

    if (crossLens.empty()) {
        crossLens.push_back(crossSize);
    }

    if (crossCount_ != crossLens.size()) {
        crossCount_ = crossLens.size();
        gridLayoutInfo_.crossCount_ = static_cast<int32_t>(crossCount_);
    }

    int32_t index = 0;
    for (const auto& len : crossLens) {
        itemsCrossSize_.try_emplace(index, len);
        ++index;
    }
}

void GridScrollLayoutAlgorithm::FillGridViewportAndMeasureChildren(
    float mainSize, float crossSize, LayoutWrapper* layoutWrapper)
{
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto gridPattern = host->GetPattern<GridPattern>();
    CHECK_NULL_VOID(gridPattern);
    itemsCrossPosition_.clear();
    UpdateGridLayoutInfo(layoutWrapper, mainSize);
    if (gridLayoutInfo_.targetIndex_.has_value()) {
        // Complete the gridLayoutInfo to get a complete set of data from 0 to targetIndex for the GridView. Make sure
        // that the index of the matrix_ and heightMap_ is incremented from 0 to targetIndex and sequentially
        SupplyAllData2ZeroIndex(mainSize, crossSize, layoutWrapper);
        gridLayoutInfo_.targetIndex_.reset();
    }
    if (enableSkipping_) {
        SkipLargeOffset(mainSize, layoutWrapper);
    }

    if (!gridLayoutInfo_.lastCrossCount_) {
        gridLayoutInfo_.lastCrossCount_ = crossCount_;
    }

    CheckReset(mainSize, crossSize, layoutWrapper);

    UpdateCurrentOffsetForJumpTo(mainSize);
    gridLayoutInfo_.jumpIndex_ = EMPTY_JUMP_INDEX;
    gridLayoutInfo_.scrollAlign_ = ScrollAlign::AUTO;

    // Step1: Measure [GridItem] that has been recorded to [gridMatrix_]
    float mainLength = MeasureRecordedItems(mainSize, crossSize, layoutWrapper);

    // Step2: When done measure items in record, request new items to fill blank at end
    FillBlankAtEnd(mainSize, crossSize, layoutWrapper, mainLength);
    if (gridLayoutInfo_.reachEnd_) { // If it reaches end when [FillBlankAtEnd], modify [currentOffset_]
        ModifyCurrentOffsetWhenReachEnd(mainSize, layoutWrapper);
    }

    // Step3: Check if need to fill blank at start (in situation of grid items moving down)
    auto haveNewLineAtStart = FillBlankAtStart(mainSize, crossSize, layoutWrapper);
    if (gridLayoutInfo_.reachStart_) {
        auto offset = gridLayoutInfo_.currentOffset_;
        if (!canOverScroll_) {
            gridLayoutInfo_.currentOffset_ = 0.0;
            gridLayoutInfo_.prevOffset_ = 0.0;
        }
        if (!haveNewLineAtStart) {
            if (canOverScroll_) {
                gridLayoutInfo_.UpdateEndIndex(offset, mainSize, mainGap_);
            }
            layoutWrapper->GetHostNode()->ChildrenUpdatedFrom(-1);
            return;
        }
        // we need lastline if blank at start is not fully filled when start line is shorter
        mainLength -= offset;
        currentMainLineIndex_ = gridLayoutInfo_.endMainLineIndex_;
        if (UseCurrentLines(mainSize, crossSize, layoutWrapper, mainLength)) {
            FillBlankAtEnd(mainSize, crossSize, layoutWrapper, mainLength);
            if (gridLayoutInfo_.reachEnd_) {
                ModifyCurrentOffsetWhenReachEnd(mainSize, layoutWrapper);
            }
        }
    }
    layoutWrapper->GetHostNode()->ChildrenUpdatedFrom(-1);
}

void GridScrollLayoutAlgorithm::ReloadToStartIndex(float mainSize, float crossSize, LayoutWrapper* layoutWrapper)
{
    const int32_t currentItemIndex = gridLayoutInfo_.startIndex_;
    // adjust startMainLine based on the new cross count
    UpdateMainLineOnReload(currentItemIndex);
    auto firstItem = GetStartingItem(layoutWrapper, currentItemIndex);
    gridLayoutInfo_.startIndex_ = firstItem;
    currentMainLineIndex_ = (firstItem == 0 ? 0 : gridLayoutInfo_.startMainLineIndex_) - 1;
    gridLayoutInfo_.endIndex_ = firstItem - 1;
    TAG_LOGI(AceLogTag::ACE_GRID, "data reload begin, firstItem:%{public}d, currentItemIndex:%{public}d", firstItem,
        currentItemIndex);
    while (gridLayoutInfo_.endIndex_ < currentItemIndex) {
        auto lineHeight = FillNewLineBackward(crossSize, mainSize, layoutWrapper, false);
        if (LessNotEqual(lineHeight, 0.0)) {
            gridLayoutInfo_.reachEnd_ = true;
            break;
        }
    }
    gridLayoutInfo_.startMainLineIndex_ = currentMainLineIndex_;
    gridLayoutInfo_.UpdateStartIndexByStartLine();
    // FillNewLineBackward sometimes make startIndex_ > currentItemIndex
    while (gridLayoutInfo_.startIndex_ > currentItemIndex &&
           gridLayoutInfo_.gridMatrix_.find(gridLayoutInfo_.startMainLineIndex_) != gridLayoutInfo_.gridMatrix_.end()) {
        gridLayoutInfo_.startMainLineIndex_--;
        gridLayoutInfo_.UpdateStartIndexByStartLine();
    }
    TAG_LOGI(AceLogTag::ACE_GRID, "data reload end, startIndex_:%{public}d, startMainLineIndex_:%{public}d",
        gridLayoutInfo_.startIndex_, gridLayoutInfo_.startMainLineIndex_);
}

void GridScrollLayoutAlgorithm::ReloadFromUpdateIdxToStartIndex(
    float mainSize, float crossSize, int32_t updateLineIndex, LayoutWrapper* layoutWrapper)
{
    const int32_t currentItemIndex = gridLayoutInfo_.startIndex_;
    auto firstItem = layoutWrapper->GetHostNode()->GetChildrenUpdated();
    gridLayoutInfo_.startIndex_ = firstItem;
    //first "-1" means trying to fill from last line;second "-1" because it will fill next line in FillNewLineBackward
    currentMainLineIndex_ = std::max(updateLineIndex - 1, 0) - 1;
    gridLayoutInfo_.endIndex_ = firstItem - 1;

    while (gridLayoutInfo_.endIndex_ < currentItemIndex) {
        auto lineHeight = FillNewLineBackward(crossSize, mainSize, layoutWrapper, false);
        if (LessNotEqual(lineHeight, 0.0)) {
            gridLayoutInfo_.reachEnd_ = true;
            break;
        }
    }
    gridLayoutInfo_.startMainLineIndex_ = currentMainLineIndex_;
    gridLayoutInfo_.UpdateStartIndexByStartLine();
    // FillNewLineBackward sometimes make startIndex_ > currentItemIndex
    while (gridLayoutInfo_.startIndex_ > currentItemIndex &&
           gridLayoutInfo_.gridMatrix_.find(gridLayoutInfo_.startMainLineIndex_) != gridLayoutInfo_.gridMatrix_.end()) {
        gridLayoutInfo_.startMainLineIndex_--;
        gridLayoutInfo_.UpdateStartIndexByStartLine();
    }
    TAG_LOGI(AceLogTag::ACE_GRID, "data reload end, startIndex_:%{public}d, startMainLineIndex_:%{public}d",
        gridLayoutInfo_.startIndex_, gridLayoutInfo_.startMainLineIndex_);
}

bool GridScrollLayoutAlgorithm::FillBlankAtStart(float mainSize, float crossSize, LayoutWrapper* layoutWrapper)
{
    bool fillNewLine = false;
    // If [currentOffset_] is non-positive, it means no blank at start
    if (LessOrEqual(gridLayoutInfo_.currentOffset_, 0.0)) {
        return fillNewLine;
    }
    auto blankAtStart = gridLayoutInfo_.currentOffset_;
    while (GreatNotEqual(blankAtStart, 0.0) || gridLayoutInfo_.startIndex_ > gridLayoutInfo_.childrenCount_ - 1) {
        float lineHeight = FillNewLineForward(crossSize, mainSize, layoutWrapper);
        if (GreatOrEqual(lineHeight, 0.0)) {
            gridLayoutInfo_.lineHeightMap_[gridLayoutInfo_.startMainLineIndex_] = lineHeight;
            blankAtStart -= (lineHeight + mainGap_);
            fillNewLine = true;
            continue;
        }
        gridLayoutInfo_.reachStart_ = true;
        break;
    }
    gridLayoutInfo_.currentOffset_ = blankAtStart;
    gridLayoutInfo_.prevOffset_ = gridLayoutInfo_.currentOffset_;
    return fillNewLine;
}

// When a moving up event comes, the [currentOffset_] may have been reduced too much than the items really need to
// be moved up, so we need to modify [currentOffset_] according to previous position.
void GridScrollLayoutAlgorithm::ModifyCurrentOffsetWhenReachEnd(float mainSize, LayoutWrapper* layoutWrapper)
{
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto gridPattern = host->GetPattern<GridPattern>();
    CHECK_NULL_VOID(gridPattern);
    // use original size in order to add end spacing
    mainSize -= gridLayoutInfo_.contentEndPadding_;
    // Step1. Calculate total length of all items with main gap in viewport.
    // [lengthOfItemsInViewport] must be greater than or equal to viewport height
    float lengthOfItemsInViewport = gridLayoutInfo_.GetTotalHeightOfItemsInView(mainGap_);
    // scroll forward
    if (LessNotEqual(gridLayoutInfo_.prevOffset_, gridLayoutInfo_.currentOffset_)) {
        if (!canOverScroll_) {
            gridLayoutInfo_.reachEnd_ = false;
            return;
        } else if (!isChildrenUpdated_) {
            if (LessNotEqual(lengthOfItemsInViewport, mainSize)) {
                return;
            }
        }
    }
    // Step2. Calculate real offset that items can only be moved up by.
    // Hint: [prevOffset_] is a non-positive value
    if (LessNotEqual(lengthOfItemsInViewport, mainSize) && gridLayoutInfo_.startIndex_ == 0) {
        if (!canOverScroll_ || isChildrenUpdated_) {
            gridLayoutInfo_.currentOffset_ = 0;
            gridLayoutInfo_.prevOffset_ = 0;
        }
        gridLayoutInfo_.reachStart_ = true;
        gridLayoutInfo_.offsetEnd_ = true;
        return;
    }

    // last grid item is not fully showed
    if (GreatNotEqual(gridLayoutInfo_.currentOffset_ + lengthOfItemsInViewport, mainSize)) {
        gridLayoutInfo_.offsetEnd_ = false;
        return;
    }

    if (gridLayoutInfo_.hasMultiLineItem_ && gridLayoutInfo_.endIndex_ == gridLayoutInfo_.childrenCount_ - 1) {
        if (!CheckLastLineItemFullyShowed(layoutWrapper)) {
            gridLayoutInfo_.offsetEnd_ = false;
            return;
        }
    }

    // Step3. modify [currentOffset_]
    if (!canOverScroll_) {
        float realOffsetToMoveUp = lengthOfItemsInViewport - mainSize + gridLayoutInfo_.prevOffset_;
        gridLayoutInfo_.currentOffset_ = gridLayoutInfo_.prevOffset_ - realOffsetToMoveUp;
        gridLayoutInfo_.prevOffset_ = gridLayoutInfo_.currentOffset_;
    }
    gridLayoutInfo_.offsetEnd_ = true;
}

void GridScrollLayoutAlgorithm::FillBlankAtEnd(
    float mainSize, float crossSize, LayoutWrapper* layoutWrapper, float& mainLength)
{
    // fill current line first
    auto mainIter = gridLayoutInfo_.gridMatrix_.find(currentMainLineIndex_);
    auto nextMain = gridLayoutInfo_.gridMatrix_.find(currentMainLineIndex_ + 1);
    if (mainIter != gridLayoutInfo_.gridMatrix_.end() && mainIter->second.size() < crossCount_ &&
        nextMain == gridLayoutInfo_.gridMatrix_.end()) {
        auto currentIndex = gridLayoutInfo_.endIndex_ + 1;
        cellAveLength_ = -1.0f;
        bool hasNormalItem = false;
        lastCross_ = 0;
        for (uint32_t i = (mainIter->second.empty() ? 0 : mainIter->second.rbegin()->first); i < crossCount_; i++) {
            // Step1. Get wrapper of [GridItem]
            auto itemWrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex);
            if (!itemWrapper) {
                break;
            }
            // Step2. Measure child
            auto frameSize = axis_ == Axis::VERTICAL ? SizeF(crossSize, mainSize) : SizeF(mainSize, crossSize);
            auto childState = MeasureNewChild(frameSize, currentIndex, layoutWrapper, itemWrapper, false);
            if (childState == -1) {
                cellAveLength_ = LessNotEqual(cellAveLength_, 0.0)
                                     ? gridLayoutInfo_.lineHeightMap_.find(currentMainLineIndex_ - 1)->second
                                     : cellAveLength_;
                --currentIndex;
                break;
            }
            i += static_cast<uint32_t>(childState) - 1;
            // Step3. Measure [GridItem]
            LargeItemLineHeight(itemWrapper, hasNormalItem);
            gridLayoutInfo_.endIndex_ = currentIndex;
            currentIndex++;
        }
    }

    if (GreatNotEqual(mainLength, mainSize)) {
        if (IsScrollToEndLine()) {
            TAG_LOGI(AceLogTag::ACE_GRID, "scroll to end line with index:%{public}d", moveToEndLineIndex_);
            // scrollToIndex(AUTO) on first layout
            moveToEndLineIndex_ = -1;
        }
        return;
    }
    // When [mainLength] is still less than [mainSize], do [FillNewLineBackward] repeatedly until filling up the lower
    // part of the viewport
    while (LessNotEqual(mainLength, mainSize)) {
        float lineHeight = FillNewLineBackward(crossSize, mainSize, layoutWrapper, false);
        if (GreatOrEqual(lineHeight, 0.0)) {
            mainLength += (lineHeight + mainGap_);
            continue;
        }
        gridLayoutInfo_.reachEnd_ = true;
        return;
    };
    // last line make LessNotEqual(mainLength, mainSize) and continue is reach end too
    gridLayoutInfo_.reachEnd_ = gridLayoutInfo_.endIndex_ == gridLayoutInfo_.childrenCount_ - 1;
}

OffsetF GridScrollLayoutAlgorithm::CalculateLargeItemOffset(
    OffsetF currOffset, int32_t itemIndex, int32_t currLineIndex, int32_t currentCrossIndex)
{
    OffsetF offset = currOffset;
    for (int32_t lastCrossIndex = currLineIndex - 1; lastCrossIndex >= 0; lastCrossIndex--) {
        auto lastGridMatrixIter = gridLayoutInfo_.gridMatrix_.find(lastCrossIndex);
        if (lastGridMatrixIter == gridLayoutInfo_.gridMatrix_.end()) {
            continue;
        }
        const auto& lastGridItemRecord = lastGridMatrixIter->second;
        auto lastLineCrossItem = lastGridItemRecord.find(currentCrossIndex);
        if (lastLineCrossItem == lastGridItemRecord.end()) {
            continue;
        }
        if (lastLineCrossItem->second == itemIndex) {
            offset -= axis_ == Axis::VERTICAL ? OffsetF(0, gridLayoutInfo_.lineHeightMap_[lastCrossIndex] + mainGap_)
                                              : OffsetF(gridLayoutInfo_.lineHeightMap_[lastCrossIndex] + mainGap_, 0.0);
        } else {
            break;
        }
    }

    return offset;
}

bool GridScrollLayoutAlgorithm::NeedAdjust(const RefPtr<GridItemLayoutProperty>& itemLayoutProperty)
{
    bool needAdjust = false;
    auto main = axis_ == Axis::VERTICAL ? mainCount_ : crossCount_;
    auto cross = axis_ == Axis::VERTICAL ? crossCount_ : mainCount_;
    if (itemLayoutProperty->GetRowStart().has_value()) {
        currentItemRowStart_ = itemLayoutProperty->GetRowStart().value_or(-1);
        if ((currentItemRowStart_ < 0) || (currentItemRowStart_ >= static_cast<int32_t>(main))) {
            needAdjust = true;
        }
    }
    if (itemLayoutProperty->GetRowEnd().has_value()) {
        currentItemRowEnd_ = itemLayoutProperty->GetRowEnd().value_or(-1);
        if ((currentItemRowEnd_ < 0) || (currentItemRowEnd_ >= static_cast<int32_t>(main))) {
            needAdjust = true;
        }
    }
    if (itemLayoutProperty->GetColumnStart().has_value()) {
        currentItemColStart_ = itemLayoutProperty->GetColumnStart().value_or(-1);
        if ((currentItemColStart_ < 0) || (currentItemColStart_ >= static_cast<int32_t>(cross))) {
            needAdjust = true;
        }
    }
    if (itemLayoutProperty->GetColumnEnd().has_value()) {
        currentItemColEnd_ = itemLayoutProperty->GetColumnEnd().value_or(-1);
        if ((currentItemColEnd_ < 0) || (currentItemColEnd_ >= static_cast<int32_t>(cross))) {
            needAdjust = true;
        }
    }
    return needAdjust;
}

void GridScrollLayoutAlgorithm::AdjustRowColSpan(
    const RefPtr<LayoutWrapper>& itemLayoutWrapper, LayoutWrapper* /* layoutWrapper */, int32_t /* itemIndex */)
{
    auto itemLayoutProperty = DynamicCast<GridItemLayoutProperty>(itemLayoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(itemLayoutProperty);
    bool needAdjust = false;
    currentItemRowSpan_ = 1;
    currentItemColSpan_ = 1;
    currentItemRowStart_ = -1;
    currentItemColStart_ = -1;
    currentItemColEnd_ = -1;
    currentItemRowEnd_ = -1;
    needAdjust = NeedAdjust(itemLayoutProperty);
    if (!needAdjust) {
        currentItemRowSpan_ = std::max(currentItemRowEnd_ - currentItemRowStart_ + 1, 1);
        currentItemColSpan_ = std::max(currentItemColEnd_ - currentItemColStart_ + 1, 1);
    } else {
        currentItemRowStart_ = -1;
        currentItemColStart_ = -1;
        currentItemColEnd_ = -1;
        currentItemRowEnd_ = -1;
    }
    if ((currentItemRowStart_ == -1 && currentItemRowEnd_ != -1) ||
        (currentItemRowEnd_ == -1 && currentItemRowStart_ != -1) ||
        (currentItemColStart_ == -1 && currentItemColEnd_ != -1) ||
        (currentItemColEnd_ == -1 && currentItemColStart_ != -1)) {
        currentItemRowSpan_ = 1;
        currentItemColSpan_ = 1;
        currentItemRowStart_ = -1;
        currentItemColStart_ = -1;
        currentItemColEnd_ = -1;
        currentItemRowEnd_ = -1;
    }
    if (currentItemRowSpan_ > 1 || currentItemColSpan_ > 1) {
        gridLayoutInfo_.hasBigItem_ = true;
    }

    itemLayoutProperty->UpdateRealRowSpan(currentItemRowSpan_);
    itemLayoutProperty->UpdateRealColumnSpan(currentItemColSpan_);
}

void GridScrollLayoutAlgorithm::LargeItemLineHeight(const RefPtr<LayoutWrapper>& itemWrapper, bool& hasNormalItem)
{
    AdjustRowColSpan(itemWrapper, nullptr, 0);
    auto mainSpan = axis_ == Axis::VERTICAL ? currentItemRowSpan_ : currentItemColSpan_;
    auto itemSize = itemWrapper->GetGeometryNode()->GetMarginFrameSize();
    if (mainSpan == 1) {
        cellAveLength_ = std::max(GetMainAxisSize(itemSize, gridLayoutInfo_.axis_), cellAveLength_);
        hasNormalItem = true;
    }

    if ((mainSpan > 1) && !hasNormalItem) {
        cellAveLength_ =
            std::max((GetMainAxisSize(itemSize, gridLayoutInfo_.axis_) - (mainGap_ * (mainSpan - 1))) / mainSpan,
                cellAveLength_);
    }
}

bool GridScrollLayoutAlgorithm::IsIndexInMatrix(int32_t index, int32_t& startLine)
{
    auto iter = std::find_if(gridLayoutInfo_.gridMatrix_.begin(), gridLayoutInfo_.gridMatrix_.end(),
        [index, &startLine](const std::pair<int32_t, std::map<int32_t, int32_t>>& item) {
            for (auto& subitem : item.second) {
                if (subitem.second == index) {
                    startLine = item.first;
                    return true;
                }
            }
            return false;
        });
    return (iter != gridLayoutInfo_.gridMatrix_.end());
}

void GridScrollLayoutAlgorithm::GetTargetIndexInfoWithBenchMark(
    LayoutWrapper* layoutWrapper, bool isTargetBackward, int32_t targetIndex)
{
    int32_t benchmarkIndex = (isTargetBackward && !gridLayoutInfo_.gridMatrix_.empty())
                                 ? gridLayoutInfo_.gridMatrix_.rbegin()->second.rbegin()->second + 1
                                 : 0;
    int32_t mainStartIndex = (isTargetBackward && !gridLayoutInfo_.gridMatrix_.empty())
                                 ? gridLayoutInfo_.gridMatrix_.rbegin()->first + 1
                                 : 0;
    int32_t currentIndex = benchmarkIndex;
    int32_t headOfMainStartLine = currentIndex;

    while (currentIndex < targetIndex) {
        int32_t crossGridReserve = gridLayoutInfo_.crossCount_;
        /* go through a new line */
        while ((crossGridReserve > 0) && (currentIndex <= targetIndex)) {
            auto currentWrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex, false);
            CHECK_NULL_VOID(currentWrapper);
            auto layoutProperty = DynamicCast<GridItemLayoutProperty>(currentWrapper->GetLayoutProperty());
            CHECK_NULL_VOID(layoutProperty);
            auto gridSpan = layoutProperty->GetCrossSpan(gridLayoutInfo_.axis_);
            if (crossGridReserve >= gridSpan) {
                crossGridReserve -= gridSpan;
            } else if (gridLayoutInfo_.crossCount_ >= gridSpan) {
                ++mainStartIndex;
                headOfMainStartLine = currentIndex;
                crossGridReserve = gridLayoutInfo_.crossCount_ - gridSpan;
            }
            ++currentIndex;
        }
        if (currentIndex > targetIndex) {
            break;
        }
        ++mainStartIndex;
        headOfMainStartLine = currentIndex;
    }
    gridLayoutInfo_.startMainLineIndex_ = mainStartIndex;
    gridLayoutInfo_.startIndex_ = headOfMainStartLine;
    gridLayoutInfo_.endIndex_ = headOfMainStartLine - 1;
    gridLayoutInfo_.prevOffset_ = 0;
    gridLayoutInfo_.currentOffset_ = 0;
    gridLayoutInfo_.ResetPositionFlags();
    gridLayoutInfo_.gridMatrix_.clear();
    gridLayoutInfo_.lineHeightMap_.clear();
    gridLayoutInfo_.irregularItemsPosition_.clear();
}

void GridScrollLayoutAlgorithm::UpdateGridLayoutInfo(LayoutWrapper* layoutWrapper, float mainSize)
{
    /* 1. Have gotten gridLayoutInfo_.startMainLineIndex_ and directly jump to it */
    if (gridLayoutInfo_.jumpIndex_ < 0 && gridLayoutInfo_.jumpIndex_ != LAST_ITEM) {
        return;
    }
    if (gridLayoutInfo_.jumpIndex_ == LAST_ITEM) {
        gridLayoutInfo_.jumpIndex_ = gridLayoutInfo_.childrenCount_ - 1;
    }
    /* 2. Need to find out the startMainLineIndex according to startIndex */
    int32_t targetIndex = gridLayoutInfo_.jumpIndex_;
    /* 2.1 invalid targetIndex */
    if (gridLayoutInfo_.childrenCount_ <= targetIndex) {
        return;
    }

    switch (gridLayoutInfo_.scrollAlign_) {
        case ScrollAlign::START:
        case ScrollAlign::END:
        case ScrollAlign::CENTER:
            ScrollToIndexStart(layoutWrapper, targetIndex);
            return;
        default:
            ScrollToIndexAuto(layoutWrapper, mainSize, targetIndex);
    }
}

bool GridScrollLayoutAlgorithm::IsScrollToEndLine() const
{
    return moveToEndLineIndex_ > 0 && gridLayoutInfo_.endIndex_ >= moveToEndLineIndex_;
}

bool GridScrollLayoutAlgorithm::IsEndLineInScreenWithGap(
    int32_t targetLine, float totalViewHeight, float mainSize) const
{
    return targetLine == gridLayoutInfo_.endMainLineIndex_ &&
           LessOrEqual(totalViewHeight + gridLayoutInfo_.currentOffset_, mainSize);
}

void GridScrollLayoutAlgorithm::ScrollToIndexAuto(LayoutWrapper* layoutWrapper, float mainSize, int32_t targetIndex)
{
    int32_t startLine = 0;
    if (IsIndexInMatrix(targetIndex, startLine)) {
        auto& info = gridLayoutInfo_;
        if (startLine == info.startMainLineIndex_ && info.startMainLineIndex_ == info.endMainLineIndex_) {
            // startLine occupies the whole viewport
            return;
        }
        if (startLine < gridLayoutInfo_.endMainLineIndex_ && startLine > gridLayoutInfo_.startMainLineIndex_) {
            return;
        }

        if (startLine >= gridLayoutInfo_.endMainLineIndex_) {
            auto totalViewHeight = gridLayoutInfo_.GetTotalHeightOfItemsInView(mainGap_);
            if (IsEndLineInScreenWithGap(startLine, totalViewHeight, mainSize)) {
                return;
            }
            gridLayoutInfo_.prevOffset_ = gridLayoutInfo_.currentOffset_;
            gridLayoutInfo_.currentOffset_ -= (totalViewHeight - mainSize + gridLayoutInfo_.currentOffset_);
            for (int32_t i = gridLayoutInfo_.endMainLineIndex_ + 1; i <= startLine; ++i) {
                gridLayoutInfo_.currentOffset_ -= (mainGap_ + gridLayoutInfo_.lineHeightMap_[i]);
            }
            gridLayoutInfo_.ResetPositionFlags();
            return;
        }

        // startLine <= gridLayoutInfo_.startMainLineIndex_
        gridLayoutInfo_.startMainLineIndex_ = startLine;
        gridLayoutInfo_.UpdateStartIndexByStartLine();
        gridLayoutInfo_.prevOffset_ = 0;
        gridLayoutInfo_.currentOffset_ = 0;
        gridLayoutInfo_.ResetPositionFlags();
        return;
    }

    /* 2.3 targetIndex is out of the matrix */
    bool isTargetBackward = true;
    if (!gridLayoutInfo_.gridMatrix_.empty()) {
        if (targetIndex < gridLayoutInfo_.gridMatrix_.begin()->second.begin()->second) {
            isTargetBackward = false;
        } else if (targetIndex > gridLayoutInfo_.gridMatrix_.rbegin()->second.rbegin()->second) {
            isTargetBackward = true;
        } else {
            return;
        }
    }
    auto grid = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(grid);
    grid->ChildrenUpdatedFrom(0);
    GetTargetIndexInfoWithBenchMark(layoutWrapper, isTargetBackward, targetIndex);
    moveToEndLineIndex_ = isTargetBackward ? targetIndex : moveToEndLineIndex_;
}

void GridScrollLayoutAlgorithm::ScrollToIndexStart(LayoutWrapper* layoutWrapper, int32_t targetIndex)
{
    int32_t startLine = 0;
    /* targetIndex is in the matrix */
    if (IsIndexInMatrix(targetIndex, startLine)) {
        if (startLine == gridLayoutInfo_.startMainLineIndex_) {
            gridLayoutInfo_.prevOffset_ = gridLayoutInfo_.currentOffset_;
            gridLayoutInfo_.currentOffset_ = 0;
            gridLayoutInfo_.ResetPositionFlags();
            return;
        }

        gridLayoutInfo_.startMainLineIndex_ = startLine;
        gridLayoutInfo_.UpdateStartIndexByStartLine();
        gridLayoutInfo_.prevOffset_ = 0;
        gridLayoutInfo_.currentOffset_ = 0;
        gridLayoutInfo_.ResetPositionFlags();
        return;
    }
    /* targetIndex is out of the matrix */
    bool isTargetBackward = true;
    if (!gridLayoutInfo_.gridMatrix_.empty()) {
        if (targetIndex < gridLayoutInfo_.gridMatrix_.begin()->second.begin()->second) {
            isTargetBackward = false;
        } else if (targetIndex > gridLayoutInfo_.gridMatrix_.rbegin()->second.rbegin()->second) {
            isTargetBackward = true;
        } else {
            return;
        }
    }
    auto grid = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(grid);
    grid->ChildrenUpdatedFrom(0);
    GetTargetIndexInfoWithBenchMark(layoutWrapper, isTargetBackward, targetIndex);
}

void GridScrollLayoutAlgorithm::UpdateCurrentOffsetForJumpTo(float mainSize)
{
    if (gridLayoutInfo_.scrollAlign_ == ScrollAlign::CENTER || gridLayoutInfo_.scrollAlign_ == ScrollAlign::END) {
        int32_t startLine = 0;
        /* targetIndex is in the matrix */
        if (IsIndexInMatrix(gridLayoutInfo_.jumpIndex_, startLine)) {
            // scroll to end of the screen
            gridLayoutInfo_.currentOffset_ =
                mainSize - gridLayoutInfo_.lineHeightMap_[startLine] - gridLayoutInfo_.contentEndPadding_;
            // scroll to center of the screen
            if (gridLayoutInfo_.scrollAlign_ == ScrollAlign::CENTER) {
                gridLayoutInfo_.currentOffset_ /= 2;
            }
            gridLayoutInfo_.prevOffset_ = gridLayoutInfo_.currentOffset_;
        } else {
            /* targetIndex is out of the matrix */
            TAG_LOGW(
                AceLogTag::ACE_GRID, "can not find jumpIndex in Grid Matrix :%{public}d", gridLayoutInfo_.jumpIndex_);
        }
    }
    if (gridLayoutInfo_.extraOffset_.has_value()) {
        gridLayoutInfo_.currentOffset_ += gridLayoutInfo_.extraOffset_.value();
        gridLayoutInfo_.extraOffset_.reset();
    }
}

float GridScrollLayoutAlgorithm::MeasureRecordedItems(float mainSize, float crossSize, LayoutWrapper* layoutWrapper)
{
    currentMainLineIndex_ = gridLayoutInfo_.startMainLineIndex_ - 1;
    float mainLength = gridLayoutInfo_.currentOffset_;
    // already at start line, do not use offset for mainLength
    if (gridLayoutInfo_.startMainLineIndex_ == 0 && GreatNotEqual(mainLength, 0)) {
        mainLength = 0;
    }
    UseCurrentLines(mainSize, crossSize, layoutWrapper, mainLength);
    return mainLength;
}

bool GridScrollLayoutAlgorithm::UseCurrentLines(
    float mainSize, float crossSize, LayoutWrapper* layoutWrapper, float& mainLength)
{
    auto& info = gridLayoutInfo_;
    bool cacheValid = true;
    bool runOutOfRecord = false;
    // Measure grid items row by row
    int32_t tempEndIndex = -1;
    while (LessNotEqual(mainLength, mainSize)) {
        // If [gridMatrix_] does not contain record of line [currentMainLineIndex_], do [FillNewLineBackward]
        auto gridMatrixIter = gridLayoutInfo_.gridMatrix_.find(++currentMainLineIndex_);
        if ((gridMatrixIter == gridLayoutInfo_.gridMatrix_.end()) ||
            (gridLayoutInfo_.lineHeightMap_.find(currentMainLineIndex_) == gridLayoutInfo_.lineHeightMap_.end())) {
            runOutOfRecord = true;
            break;
        }
        int32_t currentIndex = -1;
        bool hasNormalItem = false;
        cellAveLength_ = -1.0f;
        for (const auto& gridItemRecord : gridMatrixIter->second) {
            if (currentIndex == gridItemRecord.second) {
                continue;
            }
            currentIndex = gridItemRecord.second;
            if (currentIndex == -1) {
                // move from another grid
                continue;
            }
            auto itemWrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex);
            if (!itemWrapper) {
                break;
            }
            auto frameSize = axis_ == Axis::VERTICAL ? SizeF(crossSize, mainSize) : SizeF(mainSize, crossSize);
            AdjustRowColSpan(itemWrapper, layoutWrapper, currentIndex);
            auto crossStart = axis_ == Axis::VERTICAL ? currentItemColStart_ : currentItemRowStart_;
            if (crossStart == -1) {
                MeasureChildPlaced(frameSize, currentIndex, gridItemRecord.first, layoutWrapper, itemWrapper);
            } else {
                MeasureChildPlaced(frameSize, currentIndex, crossStart, layoutWrapper, itemWrapper);
            }
            // Record end index. When fill new line, the [endIndex_] will be the first item index to request
            LargeItemLineHeight(itemWrapper, hasNormalItem);
            tempEndIndex = std::max(currentIndex, tempEndIndex);
            gridLayoutInfo_.endIndex_ = tempEndIndex;
        }

        if (GreatOrEqual(cellAveLength_, 0.0)) { // Means at least one item has been measured
            auto it = info.lineHeightMap_.find(currentMainLineIndex_);
            if (it != info.lineHeightMap_.end() && it->second != cellAveLength_) {
                // Invalidate cache when item height changes, so that a future line jump would correctly
                // recalculate lineHeights instead of using bad cache values.
                cacheValid = false;
            }
            info.lineHeightMap_[currentMainLineIndex_] = cellAveLength_;
            mainLength += (cellAveLength_ + mainGap_);
        }
        // If a line moves up out of viewport, update [startIndex_], [currentOffset_] and [startMainLineIndex_]
        if (LessNotEqual(mainLength, 0.0)) {
            gridLayoutInfo_.currentOffset_ = mainLength;
            gridLayoutInfo_.prevOffset_ = gridLayoutInfo_.currentOffset_;
            gridLayoutInfo_.startMainLineIndex_ = currentMainLineIndex_ + 1;
            gridLayoutInfo_.UpdateStartIndexByStartLine();
        }
    }
    // Case 1. if this while-loop breaks due to running out of records, the [currentMainLineIndex_] is larger by 1 than
    // real main line index, so reduce 1.
    // Case 2. if this while-loop stops due to false result of [LessNotEqual(mainLength, mainSize)], the
    // [currentMainLineIndex_] is exactly the real main line index. Update [endMainLineIndex_] when the recorded items
    // are done measured.
    gridLayoutInfo_.endMainLineIndex_ = runOutOfRecord ? --currentMainLineIndex_ : currentMainLineIndex_;
    // reset reachEnd_ if any line at bottom is out of viewport
    // last line make LessNotEqual(mainLength, mainSize) and continue is reach end too
    gridLayoutInfo_.reachEnd_ = gridLayoutInfo_.endIndex_ == gridLayoutInfo_.childrenCount_ - 1;
    if (!gridLayoutInfo_.reachEnd_) {
        gridLayoutInfo_.offsetEnd_ = false;
    }
    if (!cacheValid) {
        info.ClearMapsToEnd(info.endMainLineIndex_ + 1);
        // run out of reord, startMainLineIndex is larger by 1 than real start main line index, so reduce 1
        info.ClearMapsFromStart(info.startMainLineIndex_ > info.endMainLineIndex_ ? info.startMainLineIndex_ - 1
                                                                                  : info.startMainLineIndex_);
    }
    return runOutOfRecord;
}

void GridScrollLayoutAlgorithm::SkipLargeOffset(float mainSize, LayoutWrapper* layoutWrapper)
{
    auto gridLayoutProperty = AceType::DynamicCast<GridLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(gridLayoutProperty);
    auto cacheCount = gridLayoutProperty->GetCachedCountValue(1);
    cacheCount = std::max(cacheCount, 1);
    SkipForwardLines(cacheCount * mainSize, layoutWrapper);
    SkipBackwardLines(cacheCount * mainSize, layoutWrapper);
}

void GridScrollLayoutAlgorithm::SkipForwardLines(float mainSize, LayoutWrapper* layoutWrapper)
{
    if (!GreatOrEqual(gridLayoutInfo_.currentOffset_ - gridLayoutInfo_.prevOffset_, mainSize)) {
        return;
    }
    // skip lines in matrix
    while (GreatOrEqual(gridLayoutInfo_.currentOffset_, mainSize)) {
        auto line = gridLayoutInfo_.gridMatrix_.find(gridLayoutInfo_.startMainLineIndex_ - 1);
        if (line == gridLayoutInfo_.gridMatrix_.end()) {
            break;
        }
        auto lineHeight = gridLayoutInfo_.lineHeightMap_.find(gridLayoutInfo_.startMainLineIndex_ - 1);
        if (lineHeight == gridLayoutInfo_.lineHeightMap_.end()) {
            break;
        }
        gridLayoutInfo_.startMainLineIndex_--;
        gridLayoutInfo_.startIndex_ = line->second.begin()->second;
        gridLayoutInfo_.currentOffset_ -= lineHeight->second + mainGap_;
    }

    // skip lines not in matrix
    if (GreatOrEqual(gridLayoutInfo_.currentOffset_, mainSize) && gridLayoutInfo_.startIndex_ > 0) {
        if (!gridLayoutInfo_.hasBigItem_) {
            SkipRegularLines(true);
        } else {
            SkipIrregularLines(layoutWrapper, true);
        }
        TAG_LOGI(AceLogTag::ACE_GRID, "estimatedIndex:%{public}d", gridLayoutInfo_.startIndex_);
        auto grid = layoutWrapper->GetHostNode();
        CHECK_NULL_VOID(grid);
        grid->ChildrenUpdatedFrom(0);
    }
}

void GridScrollLayoutAlgorithm::SkipBackwardLines(float mainSize, LayoutWrapper* layoutWrapper)
{
    if (!GreatOrEqual(gridLayoutInfo_.prevOffset_ - gridLayoutInfo_.currentOffset_, mainSize)) {
        return;
    }

    auto totalViewHeight = gridLayoutInfo_.GetTotalHeightOfItemsInView(mainGap_, true);
    auto needSkipHeight = totalViewHeight + gridLayoutInfo_.prevOffset_ + mainGap_;
    if (GreatOrEqual(needSkipHeight, -gridLayoutInfo_.currentOffset_)) {
        return;
    }

    auto endLine = gridLayoutInfo_.gridMatrix_.find(gridLayoutInfo_.endMainLineIndex_ + 1);
    if (endLine != gridLayoutInfo_.gridMatrix_.end() && !endLine->second.empty()) {
        gridLayoutInfo_.currentOffset_ += needSkipHeight;
        gridLayoutInfo_.endMainLineIndex_++;
        gridLayoutInfo_.startMainLineIndex_ = gridLayoutInfo_.endMainLineIndex_;
    }

    // grid size change from big to small
    gridLayoutInfo_.UpdateEndLine(mainSize, mainGap_);

    // skip lines in matrix
    while (GreatOrEqual(-gridLayoutInfo_.currentOffset_, mainSize)) {
        auto line = gridLayoutInfo_.gridMatrix_.find(gridLayoutInfo_.endMainLineIndex_ + 1);
        if (line == gridLayoutInfo_.gridMatrix_.end()) {
            break;
        }
        auto lineHeight = gridLayoutInfo_.lineHeightMap_.find(gridLayoutInfo_.endMainLineIndex_ + 1);
        if (lineHeight == gridLayoutInfo_.lineHeightMap_.end()) {
            break;
        }
        gridLayoutInfo_.startMainLineIndex_++;
        gridLayoutInfo_.endMainLineIndex_++;
        gridLayoutInfo_.currentOffset_ += lineHeight->second + mainGap_;
    }
    gridLayoutInfo_.UpdateStartIndexByStartLine();

    // skip lines not in matrix
    if (GreatOrEqual(-gridLayoutInfo_.currentOffset_, mainSize)) {
        if (!gridLayoutInfo_.hasBigItem_) {
            SkipRegularLines(false);
        } else {
            SkipIrregularLines(layoutWrapper, false);
        }
        TAG_LOGI(AceLogTag::ACE_GRID, "estimatedIndex:%{public}d, currentOffset:%{public}f",
            gridLayoutInfo_.startIndex_, gridLayoutInfo_.currentOffset_);
        auto grid = layoutWrapper->GetHostNode();
        CHECK_NULL_VOID(grid);
        grid->ChildrenUpdatedFrom(0);
    }
}

void GridScrollLayoutAlgorithm::SkipRegularLines(bool forward)
{
    auto lineHeight = gridLayoutInfo_.GetAverageLineHeight() + mainGap_;
    if (LessOrEqual(lineHeight, 0.0)) {
        return;
    }
    int32_t estimatedLines = gridLayoutInfo_.currentOffset_ / lineHeight;
    if (forward && gridLayoutInfo_.startIndex_ < estimatedLines * static_cast<int32_t>(crossCount_)) {
        gridLayoutInfo_.startIndex_ = 0;
        gridLayoutInfo_.currentOffset_ = 0;
    } else {
        gridLayoutInfo_.startIndex_ -= estimatedLines * static_cast<int32_t>(crossCount_);
        gridLayoutInfo_.currentOffset_ -= lineHeight * estimatedLines;
    }
}

void GridScrollLayoutAlgorithm::SkipIrregularLines(LayoutWrapper* layoutWrapper, bool forward)
{
    auto grid = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(grid);
    auto pattern = grid->GetPattern<GridPattern>();
    CHECK_NULL_VOID(pattern);
    auto averageHeight = pattern->GetAverageHeight();
    if (LessOrEqual(averageHeight, 0.0)) {
        return;
    }
    int32_t estimatedIndex = (gridLayoutInfo_.currentOffset_) / averageHeight;
    gridLayoutInfo_.startIndex_ =
        std::min(gridLayoutInfo_.startIndex_ - estimatedIndex, gridLayoutInfo_.childrenCount_);
    gridLayoutInfo_.currentOffset_ = gridLayoutInfo_.prevOffset_;
}

float GridScrollLayoutAlgorithm::FillNewLineForward(float crossSize, float mainSize, LayoutWrapper* layoutWrapper)
{
    // To make the code more convenient to read, we name a param in situation of vertical, for example:
    // 1. [lineHight] means height of a row when the Grid is vertical;
    // 2. [lineHight] means width of a column when the Grid is horizontal;
    // Other params are also named according to this principle.
    cellAveLength_ = -1.0f;
    auto currentIndex = gridLayoutInfo_.startIndex_;
    bool hasNormalItem = false;
    if (gridLayoutInfo_.startMainLineIndex_ - 1 < 0) {
        if (currentIndex == 0) {
            return cellAveLength_;
        }
        // add more than one line
        UpdateMatrixForAddedItems();
    }
    gridLayoutInfo_.startMainLineIndex_--;
    bool doneCreateNewLine = false;
    auto gridMatrixIter = gridLayoutInfo_.gridMatrix_.find(gridLayoutInfo_.startMainLineIndex_);
    if (gridMatrixIter == gridLayoutInfo_.gridMatrix_.end()) {
        AddForwardLines(currentIndex, crossSize, mainSize, layoutWrapper);
    }
    gridMatrixIter = gridLayoutInfo_.gridMatrix_.find(gridLayoutInfo_.startMainLineIndex_);
    if (gridMatrixIter == gridLayoutInfo_.gridMatrix_.end()) {
        return cellAveLength_;
    }

    // need to obtain the item node in order and by step one in LazyLayoutWrapperBuilder::OnGetOrCreateWrapperByIndex
    for (auto itemIter = gridMatrixIter->second.rbegin(); itemIter != gridMatrixIter->second.rend(); ++itemIter) {
        currentIndex = itemIter->second;

        // Step1. Get wrapper of [GridItem]
        auto itemWrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex);
        if (!itemWrapper) {
            break;
        }
        // Step2. Measure child
        auto frameSize = axis_ == Axis::VERTICAL ? SizeF(crossSize, mainSize) : SizeF(mainSize, crossSize);
        AdjustRowColSpan(itemWrapper, layoutWrapper, currentIndex);
        auto crossStart = axis_ == Axis::VERTICAL ? currentItemColStart_ : currentItemRowStart_;
        if (crossStart == -1) {
            MeasureChildPlaced(frameSize, currentIndex, itemIter->first, layoutWrapper, itemWrapper);
        } else {
            MeasureChildPlaced(frameSize, currentIndex, crossStart, layoutWrapper, itemWrapper);
        }
        // Step3. Measure [GridItem]
        LargeItemLineHeight(itemWrapper, hasNormalItem);
        gridLayoutInfo_.startIndex_ = currentIndex;
    }

    doneCreateNewLine = GreatOrEqual(cellAveLength_, 0.0);
    // If it fails to create new line when [FillNewLineForward] is called, it means that it reaches start
    gridLayoutInfo_.reachStart_ = !doneCreateNewLine;

    return cellAveLength_;
}

void GridScrollLayoutAlgorithm::UpdateMatrixForAddedItems()
{
    decltype(gridLayoutInfo_.lineHeightMap_) gridLineHeightMap(std::move(gridLayoutInfo_.lineHeightMap_));
    decltype(gridLayoutInfo_.gridMatrix_) gridMatrix(std::move(gridLayoutInfo_.gridMatrix_));
    for (const auto& item : gridMatrix) {
        gridLayoutInfo_.gridMatrix_[item.first + 1] = item.second;
    }
    for (const auto& item : gridLineHeightMap) {
        gridLayoutInfo_.lineHeightMap_[item.first + 1] = item.second;
    }
    gridLayoutInfo_.startMainLineIndex_ = gridLayoutInfo_.startMainLineIndex_ + 1;
    gridLayoutInfo_.endMainLineIndex_ = gridLayoutInfo_.endMainLineIndex_ + 1;
    TAG_LOGI(AceLogTag::ACE_GRID, "add more than one line startMainLineIndex_:%{public}d",
        gridLayoutInfo_.startMainLineIndex_);
}

void GridScrollLayoutAlgorithm::AddForwardLines(
    int32_t currentIndex, float crossSize, float mainSize, LayoutWrapper* layoutWrapper)
{
    auto endMainLineIndex = gridLayoutInfo_.endMainLineIndex_;
    auto endIndex = gridLayoutInfo_.endIndex_;
    auto firstItem = GetStartingItem(layoutWrapper, currentIndex - 1);
    auto itemWrapper = layoutWrapper->GetOrCreateChildByIndex(firstItem);
    CHECK_NULL_VOID(itemWrapper);
    AdjustRowColSpan(itemWrapper, layoutWrapper, firstItem);
    auto mainSpan = axis_ == Axis::VERTICAL ? currentItemRowSpan_ : currentItemColSpan_;
    if (mainSpan > 1) {
        gridLayoutInfo_.hasMultiLineItem_ = true;
    }
    auto measureNumber = 0;
    currentMainLineIndex_ = (firstItem == 0 ? 0 : gridLayoutInfo_.startMainLineIndex_) - 1;
    gridLayoutInfo_.endIndex_ = firstItem - 1;
    // firstItem may be more than one line ahead, use new matrix to save and merge to old matrix
    decltype(gridLayoutInfo_.lineHeightMap_) gridLineHeightMap(std::move(gridLayoutInfo_.lineHeightMap_));
    decltype(gridLayoutInfo_.gridMatrix_) gridMatrix(std::move(gridLayoutInfo_.gridMatrix_));
    bool addLine = false;
    while (gridLayoutInfo_.endIndex_ < currentIndex - 1 || mainSpan > measureNumber) {
        auto newLineHeight = FillNewLineBackward(crossSize, mainSize, layoutWrapper, true);
        measureNumber++;
        if (LessNotEqual(newLineHeight, 0.0)) {
            gridLayoutInfo_.reachEnd_ = true;
            break;
        }
        addLine = true;
    }
    if (!addLine) {
        return;
    }
    // merge matrix
    auto forwardLines = gridLayoutInfo_.endMainLineIndex_ - gridLayoutInfo_.startMainLineIndex_;
    if (forwardLines >= 0) {
        auto begin = gridLayoutInfo_.gridMatrix_.begin()->first;
        if (gridLayoutInfo_.endMainLineIndex_ - begin <= begin) {
            for (auto i = begin; i <= gridLayoutInfo_.endMainLineIndex_; i++) {
                gridMatrix.emplace(i - forwardLines, std::move(gridLayoutInfo_.gridMatrix_[i]));
                gridLineHeightMap.emplace(i - forwardLines, gridLayoutInfo_.lineHeightMap_[i]);
            }
            gridMatrix.swap(gridLayoutInfo_.gridMatrix_);
            gridLineHeightMap.swap(gridLayoutInfo_.lineHeightMap_);
        } else {
            for (auto i = gridLayoutInfo_.startMainLineIndex_ + 1; i <= gridMatrix.rbegin()->first; i++) {
                gridLayoutInfo_.gridMatrix_.emplace(forwardLines + i, std::move(gridMatrix[i]));
                gridLayoutInfo_.lineHeightMap_.emplace(forwardLines + i, gridLineHeightMap[i]);
            }
        }
    } else {
        // delete more than one line items
        for (auto i = gridLayoutInfo_.startMainLineIndex_ + 1; i <= gridMatrix.rbegin()->first; i++) {
            gridLayoutInfo_.gridMatrix_.emplace(forwardLines + i, std::move(gridMatrix[i]));
            gridLayoutInfo_.lineHeightMap_.emplace(forwardLines + i, gridLineHeightMap[i]);
        }
    }

    gridLayoutInfo_.startMainLineIndex_ = gridLayoutInfo_.endMainLineIndex_ - (forwardLines > 0 ? forwardLines : 0);
    gridLayoutInfo_.endMainLineIndex_ = endMainLineIndex + (forwardLines < 0 ? forwardLines : 0);
    gridLayoutInfo_.endIndex_ = endIndex;
    TAG_LOGI(AceLogTag::ACE_GRID, "after load forward:start main line %{public}d end main line %{public}d",
        gridLayoutInfo_.startMainLineIndex_, gridLayoutInfo_.endMainLineIndex_);
}

float GridScrollLayoutAlgorithm::FillNewLineBackward(
    float crossSize, float mainSize, LayoutWrapper* layoutWrapper, bool reverse)
{
    // To make the code more convenient to read, we name a param in situation of vertical, for example:
    // 1. [lineHight] means height of a row when the Grid is vertical;
    // 2. [lineHight] means width of a column when the Grid is horizontal;
    // Other params are also named according to this principle.
    cellAveLength_ = -1.0f;
    if (IsScrollToEndLine()) {
        TAG_LOGI(AceLogTag::ACE_GRID, "scroll to end line with index:%{public}d", moveToEndLineIndex_);
        // scrollToIndex(AUTO) on first layout
        moveToEndLineIndex_ = -1;
        return cellAveLength_;
    }
    auto currentIndex = gridLayoutInfo_.endIndex_ + 1;
    currentMainLineIndex_++; // if it fails to fill a new line backward, do [currentMainLineIndex_--]
    if (gridLayoutInfo_.gridMatrix_.find(currentMainLineIndex_) != gridLayoutInfo_.gridMatrix_.end()) {
        cellAveLength_ = gridLayoutInfo_.lineHeightMap_.find(currentMainLineIndex_ - 1)->second;
    }
    lastCross_ = 0;
    bool hasNormalItem = false;
    bool doneFillLine = false;

    for (uint32_t i = 0; i < crossCount_; i++) {
        // already finish first line forward
        if (reverse && currentIndex >= gridLayoutInfo_.startIndex_) {
            break;
        }
        // Step1. Get wrapper of [GridItem]
        auto itemWrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex);
        if (!itemWrapper) {
            LargeItemNextLineHeight(currentMainLineIndex_, layoutWrapper);
            break;
        }
        // Step2. Measure child
        auto frameSize = axis_ == Axis::VERTICAL ? SizeF(crossSize, mainSize) : SizeF(mainSize, crossSize);
        auto crossSpan = MeasureNewChild(frameSize, currentIndex, layoutWrapper, itemWrapper, false);
        if (crossSpan < 0) {
            cellAveLength_ = LessNotEqual(cellAveLength_, 0.0)
                                 ? gridLayoutInfo_.lineHeightMap_.find(currentMainLineIndex_ - 1)->second
                                 : cellAveLength_;
            --currentIndex;
            break;
        }
        i = lastCross_ - 1;
        // Step3. Measure [GridItem]
        LargeItemLineHeight(itemWrapper, hasNormalItem);

        gridLayoutInfo_.endIndex_ = currentIndex;
        currentIndex++;
        doneFillLine = true;
    }

    if (doneFillLine || gridLayoutInfo_.gridMatrix_.find(currentMainLineIndex_) != gridLayoutInfo_.gridMatrix_.end()) {
        gridLayoutInfo_.lineHeightMap_[currentMainLineIndex_] = cellAveLength_;
        gridLayoutInfo_.endMainLineIndex_ = currentMainLineIndex_;
    } else {
        currentMainLineIndex_--;
    }
    return cellAveLength_;
}

void GridScrollLayoutAlgorithm::LargeItemNextLineHeight(int32_t currentLineIndex, LayoutWrapper* layoutWrapper)
{
    auto gridMatrixIter = gridLayoutInfo_.gridMatrix_.find(currentLineIndex);
    bool hasNormalItem = false;
    auto currentIndex = 0;
    if (gridMatrixIter != gridLayoutInfo_.gridMatrix_.end()) {
        for (auto itemIter = gridMatrixIter->second.rbegin(); itemIter != gridMatrixIter->second.rend(); ++itemIter) {
            currentIndex = itemIter->second;
            auto itemWrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex);
            if (!itemWrapper) {
                break;
            }
            LargeItemLineHeight(itemWrapper, hasNormalItem);
        }
    }
}

void GridScrollLayoutAlgorithm::LargeItemForwardLineHeight(int32_t currentLineIndex, LayoutWrapper* layoutWrapper)
{
    auto lineIndex = currentLineIndex;
    auto gridMatrixIter = gridLayoutInfo_.gridMatrix_.find(lineIndex);
    if (gridMatrixIter == gridLayoutInfo_.gridMatrix_.end()) {
        return;
    }
    auto currentIndex = -1;

    lineIndex = CalculateLineIndexForLargeItem(gridMatrixIter, currentIndex, lineIndex, layoutWrapper);
    CalculateLineHeightForLargeItem(lineIndex, currentLineIndex, gridMatrixIter, layoutWrapper);
}

int32_t GridScrollLayoutAlgorithm::CalculateLineIndexForLargeItem(
    std::map<int32_t, std::map<int32_t, int32_t>>::iterator gridMatrixIter, int32_t currentIndex, int32_t lineIndex,
    LayoutWrapper* layoutWrapper)
{
    for (const auto& gridItemRecord : gridMatrixIter->second) {
        if (currentIndex == gridItemRecord.second || gridItemRecord.second == -1) {
            continue;
        }
        currentIndex = gridItemRecord.second;
        auto itemWrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex);
        if (!itemWrapper) {
            break;
        }
        AdjustRowColSpan(itemWrapper, layoutWrapper, currentIndex);
        for (int32_t lastCrossIndex = lineIndex - 1; lastCrossIndex >= 0; lastCrossIndex--) {
            auto lastGridMatrixIter = gridLayoutInfo_.gridMatrix_.find(lastCrossIndex);
            if (lastGridMatrixIter == gridLayoutInfo_.gridMatrix_.end()) {
                continue;
            }
            auto lastGridItemRecord = lastGridMatrixIter->second;
            auto lastLineCrossItem = lastGridItemRecord.find(gridItemRecord.first);
            if (lastLineCrossItem == lastGridItemRecord.end()) {
                continue;
            }
            if (lastLineCrossItem->second == currentIndex) {
                lineIndex--;
            } else {
                break;
            }
        }
    }
    return lineIndex;
}

void GridScrollLayoutAlgorithm::CalculateLineHeightForLargeItem(int32_t lineIndex, int32_t currentLineIndex,
    std::map<int32_t, std::map<int32_t, int32_t>>::iterator gridMatrixIter, LayoutWrapper* layoutWrapper)
{
    for (int32_t i = lineIndex; i <= currentLineIndex; i++) {
        auto currentGridMatrixIter = gridLayoutInfo_.gridMatrix_.find(i);
        if (currentGridMatrixIter == gridLayoutInfo_.gridMatrix_.end()) {
            continue;
        }
        bool hasNormalItem = false;
        auto currentIndex = 0;
        cellAveLength_ = -1.0f;
        for (auto itemIter = gridMatrixIter->second.rbegin(); itemIter != gridMatrixIter->second.rend(); ++itemIter) {
            if (currentIndex == itemIter->second) {
                continue;
            }
            currentIndex = itemIter->second;
            auto itemWrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex);
            if (!itemWrapper) {
                break;
            }
            LargeItemLineHeight(itemWrapper, hasNormalItem);
            auto line = gridLayoutInfo_.lineHeightMap_.find(i);
            if (line == gridLayoutInfo_.lineHeightMap_.end() || line->second < cellAveLength_) {
                gridLayoutInfo_.lineHeightMap_[i] = cellAveLength_;
            }
        }
    }
}

LayoutConstraintF GridScrollLayoutAlgorithm::CreateChildConstraint(float mainSize, float crossSize,
    const RefPtr<GridLayoutProperty>& gridLayoutProperty, int32_t crossStart, int32_t crossSpan) const
{
    float itemMainSize =
        gridLayoutProperty->IsConfiguredScrollable() ? Infinity<float>() : mainSize / static_cast<float>(mainCount_);

    auto frameSize = axis_ == Axis::VERTICAL ? SizeF(crossSize, mainSize) : SizeF(mainSize, crossSize);
    float itemCrossSize = GridUtils::GetCrossGap(gridLayoutProperty, frameSize, axis_) * (crossSpan - 1);
    for (int32_t index = 0; index < crossSpan; ++index) {
        int32_t crossIndex = (crossStart + index) % static_cast<int32_t>(crossCount_);
        if (crossIndex >= 0 && crossIndex < static_cast<int32_t>(itemsCrossSize_.size())) {
            itemCrossSize += itemsCrossSize_.at(crossIndex);
        }
    }

    SizeF itemIdealSize =
        gridLayoutProperty->IsVertical() ? SizeF(itemCrossSize, itemMainSize) : SizeF(itemMainSize, itemCrossSize);
    auto itemConstraint = gridLayoutProperty->CreateChildConstraint();

    // The percent size of GridItem is based on the fraction size, for e.g., if a GridItem has width of "50%" in Grid
    // configured with columnsTemplate = "1fr 1fr", rowsTemplate = "1fr 1fr",
    // then the GridItem width = [width of 1fr] * 50%,
    // [itemFractionCount] is now only in direction of cross axis
    float widthPercentBase =
        GreatOrEqual(crossCount_, Infinity<uint32_t>()) ? itemConstraint.percentReference.Width() : itemCrossSize;
    float heightPercentBase = GreatOrEqual(mainCount_, Infinity<uint32_t>())
                                  ? itemConstraint.percentReference.Height()
                                  : itemConstraint.percentReference.Height() / static_cast<float>(mainCount_);
    if (axis_ == Axis::VERTICAL) {
        itemConstraint.percentReference = SizeF(widthPercentBase, itemConstraint.percentReference.Height());
    } else {
        itemConstraint.percentReference = SizeF(itemConstraint.percentReference.Width(), heightPercentBase);
    }
    itemConstraint.maxSize = itemIdealSize;
    itemConstraint.UpdateIllegalSelfMarginSizeWithCheck(axis_ == Axis::VERTICAL
                                                            ? OptionalSizeF(itemCrossSize, std::nullopt)
                                                            : OptionalSizeF(std::nullopt, itemCrossSize));
    return itemConstraint;
}

bool GridScrollLayoutAlgorithm::GetNextGrid(int32_t& curMain, int32_t& curCross, bool reverse) const
{
    if (!reverse) {
        ++curCross;
        if (curCross >= static_cast<int32_t>(crossCount_)) {
            return false;
        }
        return true;
    }

    --curCross;
    if (curCross < 0) {
        return false;
    }
    return true;
}

int32_t GridScrollLayoutAlgorithm::MeasureNewChild(const SizeF& frameSize, int32_t itemIndex,
    LayoutWrapper* layoutWrapper, const RefPtr<LayoutWrapper>& childLayoutWrapper, bool reverse)
{
    auto mainCount = static_cast<int32_t>(mainCount_);
    auto crossCount = static_cast<int32_t>(crossCount_);
    AdjustRowColSpan(childLayoutWrapper, layoutWrapper, itemIndex);
    auto mainSpan = axis_ == Axis::VERTICAL ? currentItemRowSpan_ : currentItemColSpan_;
    if (mainSpan > 1) {
        gridLayoutInfo_.hasMultiLineItem_ = true;
    }
    auto crossSpan = axis_ == Axis::VERTICAL ? currentItemColSpan_ : currentItemRowSpan_;
    auto crossStart = axis_ == Axis::VERTICAL ? currentItemColStart_ : currentItemRowStart_;
    if (crossSpan > crossCount) {
        TAG_LOGW(AceLogTag::ACE_GRID,
            "item %{public}d can not be placed in grid: cross count:%{public}d, cross span:%{public}d", itemIndex,
            crossCount, crossSpan);
        return crossSpan;
    }
    int32_t mainIndex = currentMainLineIndex_;

    if (crossStart >= 0 && crossStart < crossCount) {
        if (crossStart < lastCross_) {
            return -1;
        } else if (CheckGridPlaced(itemIndex, mainIndex, crossStart, mainSpan, crossSpan)) {
            MeasureChild(layoutWrapper, frameSize, childLayoutWrapper, crossStart, crossSpan);
            itemsCrossPosition_.try_emplace(itemIndex, ComputeItemCrossPosition(layoutWrapper, crossStart));
        } else {
            return -1;
        }
    } else {
        int32_t crossIndex = crossStart >= 0 ? crossStart : lastCross_;

        while (!CheckGridPlaced(itemIndex, mainIndex, crossIndex, mainSpan, crossSpan)) {
            if (GetNextGrid(mainIndex, crossIndex, reverse) == false) {
                return -1;
            }
            if (mainIndex >= mainCount || crossIndex >= crossCount) {
                break;
            }
        }

        MeasureChild(layoutWrapper, frameSize, childLayoutWrapper, crossIndex, crossSpan);
        itemsCrossPosition_.try_emplace(itemIndex, ComputeItemCrossPosition(layoutWrapper, crossIndex));
    }
    return crossSpan;
}

int32_t GridScrollLayoutAlgorithm::MeasureChildPlaced(const SizeF& frameSize, int32_t itemIndex, int32_t crossStart,
    LayoutWrapper* layoutWrapper, const RefPtr<LayoutWrapper>& childLayoutWrapper)
{
    AdjustRowColSpan(childLayoutWrapper, layoutWrapper, itemIndex);
    auto crossSpan = axis_ == Axis::VERTICAL ? currentItemColSpan_ : currentItemRowSpan_;
    if (static_cast<uint32_t>(crossStart + crossSpan) > crossCount_) {
        TAG_LOGI(AceLogTag::ACE_GRID, "item %{public}d cross not enough, start:%{public}d, span:%{public}d", itemIndex,
            crossStart, crossSpan);
        return 0;
    }

    MeasureChild(layoutWrapper, frameSize, childLayoutWrapper, crossStart, crossSpan);
    itemsCrossPosition_.try_emplace(itemIndex, ComputeItemCrossPosition(layoutWrapper, crossStart));
    return crossSpan;
}

bool GridScrollLayoutAlgorithm::CheckNeedMeasure(
    const RefPtr<LayoutWrapper>& layoutWrapper, const LayoutConstraintF& layoutConstraint) const
{
    if (expandSafeArea_ || layoutWrapper->CheckNeedForceMeasureAndLayout()) {
        return true;
    }
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, true);
    auto constraint = geometryNode->GetParentLayoutConstraint();
    CHECK_NULL_RETURN(constraint, true);
    return constraint.value() != layoutConstraint;
}

void GridScrollLayoutAlgorithm::MeasureChild(LayoutWrapper* layoutWrapper, const SizeF& frameSize,
    const RefPtr<LayoutWrapper>& childLayoutWrapper, int32_t crossStart, int32_t crossSpan)
{
    auto gridLayoutProperty = DynamicCast<GridLayoutProperty>(layoutWrapper->GetLayoutProperty());
    auto mainSize = GetMainAxisSize(frameSize, gridLayoutInfo_.axis_);
    auto crossSize = GetCrossAxisSize(frameSize, gridLayoutInfo_.axis_);
    auto childConstraint = CreateChildConstraint(mainSize, crossSize, gridLayoutProperty, crossStart, crossSpan);
    if (!CheckNeedMeasure(childLayoutWrapper, childConstraint)) {
        return;
    }
    auto childLayoutProperty = childLayoutWrapper->GetLayoutProperty();
    if (!childLayoutProperty) {
        childLayoutWrapper->Measure(childConstraint);
        return;
    }
    auto oldConstraint = childLayoutProperty->GetLayoutConstraint();
    if (oldConstraint.has_value() && !NearEqual(GetCrossAxisSize(oldConstraint.value().maxSize, axis_),
                                         GetCrossAxisSize(childConstraint.maxSize, axis_))) {
        auto layoutAlgorithmWrapper = childLayoutWrapper->GetLayoutAlgorithm();
        if (layoutAlgorithmWrapper->SkipMeasure()) {
            layoutAlgorithmWrapper->SetNeedMeasure();
            if (layoutAlgorithmWrapper->GetLayoutAlgorithm() == nullptr) {
                layoutAlgorithmWrapper->SetLayoutAlgorithm(
                    childLayoutWrapper->GetHostNode()->GetPattern()->CreateLayoutAlgorithm());
            }
        }
    }
    childLayoutWrapper->Measure(childConstraint);
}

bool GridScrollLayoutAlgorithm::CheckGridPlaced(
    int32_t index, int32_t main, int32_t cross, int32_t mainSpan, int32_t crossSpan)
{
    // If start position is already exist in gridMatrix, place grid item fail.
    auto mainIter = gridLayoutInfo_.gridMatrix_.find(main);
    if (mainIter != gridLayoutInfo_.gridMatrix_.end()) {
        auto crossIter = mainIter->second.find(cross);
        if (crossIter != mainIter->second.end()) {
            return false;
        }
    }

    // If cross length of grid item if out of range,  place grid item fail.
    if (cross + crossSpan > static_cast<int32_t>(crossCount_)) {
        return false;
    }

    // If any grid item is already exist in gridMatrix, place grid item fail.
    for (int32_t i = 0; i < mainSpan; i++) {
        mainIter = gridLayoutInfo_.gridMatrix_.find(i + main);
        if (mainIter == gridLayoutInfo_.gridMatrix_.end()) {
            continue;
        }
        for (int32_t j = 0; j < crossSpan; j++) {
            auto crossIter = mainIter->second.find(j + cross);
            if (crossIter != mainIter->second.end()) {
                return false;
            }
        }
    }

    // Padding grid matrix for grid item's range.
    for (int32_t i = main; i < main + mainSpan; ++i) {
        std::map<int32_t, int32_t> mainMap;
        auto iter = gridLayoutInfo_.gridMatrix_.find(i);
        if (iter != gridLayoutInfo_.gridMatrix_.end()) {
            mainMap = iter->second;
        }
        for (int32_t j = cross; j < cross + crossSpan; ++j) {
            mainMap.emplace(std::make_pair(j, index));
        }
        gridLayoutInfo_.gridMatrix_[i] = mainMap;
    }
    lastCross_ = cross + crossSpan;

    return true;
}

float GridScrollLayoutAlgorithm::ComputeItemCrossPosition(LayoutWrapper* layoutWrapper, int32_t crossStart) const
{
    float position = 0.0f;
    for (int32_t index = 0; index < crossStart; ++index) {
        if (index >= 0 && index < static_cast<int32_t>(itemsCrossSize_.size())) {
            position += itemsCrossSize_.at(index);
        }
    }
    position += crossStart * crossGap_ + crossPaddingOffset_;
    return position;
}

int32_t GridScrollLayoutAlgorithm::GetStartingItem(LayoutWrapper* layoutWrapper, int32_t currentIndex)
{
    int32_t firstIndex = 0;
    currentIndex = currentIndex < gridLayoutInfo_.childrenCount_ ? currentIndex : gridLayoutInfo_.childrenCount_ - 1;
    auto index = currentIndex;
    if (gridLayoutInfo_.hasBigItem_) {
        while (index > 0) {
            auto childLayoutWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
            if (!childLayoutWrapper) {
                TAG_LOGW(AceLogTag::ACE_GRID, "item [%{public}d] does not exist, reload to [0]", index);
                break;
            }

            AdjustRowColSpan(childLayoutWrapper, layoutWrapper, index);
            auto crossIndex = axis_ == Axis::VERTICAL ? currentItemColStart_ : currentItemRowStart_;
            if (crossIndex == 0) {
                firstIndex = index;
                break;
            }
            --index;
        }
    } else {
        while (index > 0) {
            // need to obtain the item node in order and by step one
            auto childLayoutWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
            if (!childLayoutWrapper) {
                TAG_LOGW(AceLogTag::ACE_GRID, "item [%{public}d] does not exist, reload to [0]", index);
                break;
            }
            AdjustRowColSpan(childLayoutWrapper, layoutWrapper, index);
            auto crossIndex = axis_ == Axis::VERTICAL ? currentItemColStart_ : currentItemRowStart_;
            // Grid may change from no big item to has big item
            if (crossIndex >= 0) {
                gridLayoutInfo_.hasBigItem_ = true;
                return GetStartingItem(layoutWrapper, currentIndex);
            }
            if (index % gridLayoutInfo_.crossCount_ == 0) {
                firstIndex = index;
                break;
            }
            --index;
        }
    }
    return firstIndex;
}

void GridScrollLayoutAlgorithm::SupplyAllData2ZeroIndex(float mainSize, float crossSize, LayoutWrapper* layoutWrapper)
{
    // Save the global variable at this moment.
    auto tempGridLayoutInfo = gridLayoutInfo_;

    // When the data is supplied again, there is an update of the original global variable gridLayoutInfo_. Therefore,
    // each time you supply the data, you must re-complete the data based on the current screen data
    auto startLineIndex = tempGridLayoutInfo.startMainLineIndex_;
    auto startIndex = tempGridLayoutInfo.startIndex_;
    auto endLineIndex = tempGridLayoutInfo.endMainLineIndex_;
    auto endIndex = tempGridLayoutInfo.endIndex_;
    auto targetIndex = tempGridLayoutInfo.targetIndex_;
    // Remove redundant data that is visible off-screen. This is the key to completing data accurately
    DeleteItemsOutOfScope(gridLayoutInfo_.lineHeightMap_, startLineIndex, endLineIndex);
    DeleteItemsOutOfScope(gridLayoutInfo_.gridMatrix_, startLineIndex, endLineIndex);

    // The continuous grid information is saved and used in the GridPattern to calculate the scroll distance
    // Complete all data with indexes from startIndex to 0
    if (startIndex > 0) {
        // The start line when completing the data
        currentMainLineIndex_ = startLineIndex;
        float lineHeight = 0.0f;
        do {
            lineHeight = FillNewLineForward(crossSize, mainSize, layoutWrapper);
        } while (GreatOrEqual(lineHeight, 0.0));
    }

    // Complete the data from endIndex+1 to targetIndex_
    if (endIndex < targetIndex) {
        // The start line when completing the data
        currentMainLineIndex_ = endLineIndex;
        float lineHeight = 0.0f;
        int32_t targetLineIndex = 0;
        do {
            lineHeight = FillNewLineBackward(crossSize, mainSize, layoutWrapper, false);
        } while (!(LessNotEqual(lineHeight, 0.0) || IsIndexInMatrix(targetIndex.value(), targetLineIndex)));
    }

    // Once the data is completed, the global variables need to be returned
    scrollGridLayoutInfo_ = gridLayoutInfo_;
    gridLayoutInfo_ = tempGridLayoutInfo;
}

void GridScrollLayoutAlgorithm::FillCacheLineAtEnd(float mainSize, float crossSize, LayoutWrapper* layoutWrapper)
{
    auto gridLayoutProperty = DynamicCast<GridLayoutProperty>(layoutWrapper->GetLayoutProperty());
    auto cacheCount = gridLayoutProperty->GetCachedCountValue(1);
    if (gridLayoutInfo_.reachEnd_ || cacheCount == 0) {
        return;
    }
    auto tempEndIndex = gridLayoutInfo_.endIndex_;
    auto tempEndMainLineIndex = gridLayoutInfo_.endMainLineIndex_;
    auto tempCurrentMainLineIndex = currentMainLineIndex_;
    currentMainLineIndex_ = gridLayoutInfo_.endMainLineIndex_;

    for (; currentMainLineIndex_ <= tempEndMainLineIndex + cacheCount; currentMainLineIndex_++) {
        float lineHeight = FillNewCacheLineBackward(crossSize, mainSize, layoutWrapper, currentMainLineIndex_);
        if (LessNotEqual(lineHeight, 0.0)) {
            break;
        }
    }
    gridLayoutInfo_.endIndex_ = tempEndIndex;
    gridLayoutInfo_.endMainLineIndex_ = tempEndMainLineIndex;
    currentMainLineIndex_ = tempCurrentMainLineIndex;

    if (!predictBuildList_.empty()) {
        CreateCachedChildConstraint(layoutWrapper, mainSize, crossSize);
    }
}

float GridScrollLayoutAlgorithm::FillNewCacheLineBackward(
    float crossSize, float mainSize, LayoutWrapper* layoutWrapper, int32_t currentLine)
{
    // To make the code more convenient to read, we name a param in situation of vertical, for example:
    // 1. [lineHight] means height of a row when the Grid is vertical;
    // 2. [lineHight] means width of a column when the Grid is horizontal;
    // Other params are also named according to this principle.
    cellAveLength_ = -1.0f;
    auto currentIndex = gridLayoutInfo_.endIndex_ + 1;

    // if it fails to fill a new line backward, do [currentLine--]
    auto line = gridLayoutInfo_.gridMatrix_.find(currentLine);
    if (gridLayoutInfo_.gridMatrix_.find(currentLine) != gridLayoutInfo_.gridMatrix_.end()) {
        auto nextMain = gridLayoutInfo_.gridMatrix_.find(currentLine + 1);
        if (line->second.size() < crossCount_ && nextMain == gridLayoutInfo_.gridMatrix_.end()) {
            bool hasNormalItem = false;
            lastCross_ = 0;
            for (const auto& elem : line->second) {
                if (elem.second > gridLayoutInfo_.endIndex_) {
                    gridLayoutInfo_.endIndex_ = elem.second;
                }
            }
            auto currentIndex = gridLayoutInfo_.endIndex_ + 1;
            for (uint32_t i = (line->second.empty() ? 0 : line->second.rbegin()->first); i < crossCount_ - 1; i++) {
                // Step1. Get wrapper of [GridItem]
                auto itemWrapper = layoutWrapper->GetChildByIndex(currentIndex, true);
                if (!itemWrapper || itemWrapper->CheckNeedForceMeasureAndLayout()) {
                    for (uint32_t y = i; y < crossCount_ - 1; y++) {
                        predictBuildList_.emplace_back(currentIndex++);
                    }
                    if (GreatOrEqual(cellAveLength_, 0.0f) &&
                        gridLayoutInfo_.lineHeightMap_.find(currentLine) == gridLayoutInfo_.lineHeightMap_.end()) {
                        gridLayoutInfo_.lineHeightMap_[currentLine] = cellAveLength_;
                    }
                    return -1.0f;
                }
                // Step2. Measure child
                auto frameSize = axis_ == Axis::VERTICAL ? SizeF(crossSize, mainSize) : SizeF(mainSize, crossSize);
                auto childState = MeasureCachedChild(frameSize, currentIndex, layoutWrapper, itemWrapper);
                if (childState == -1) {
                    cellAveLength_ = LessNotEqual(cellAveLength_, 0.0)
                                         ? gridLayoutInfo_.lineHeightMap_.find(currentLine - 1)->second
                                         : cellAveLength_;
                    --currentIndex;
                    break;
                }
                i += static_cast<uint32_t>(childState) - 1;
                // Step3. Measure [GridItem]
                LargeItemLineHeight(itemWrapper, hasNormalItem);
                gridLayoutInfo_.endIndex_ = currentIndex;
                currentIndex++;
            }
        }
        CompleteItemCrossPosition(layoutWrapper, line->second);
        for (const auto& elem : line->second) {
            if (elem.second > gridLayoutInfo_.endIndex_) {
                gridLayoutInfo_.endIndex_ = elem.second;
            }
        }
        return gridLayoutInfo_.lineHeightMap_.find(currentLine)->second;
    }

    lastCross_ = 0;
    bool hasNormalItem = false;
    bool doneFillLine = false;

    for (uint32_t i = 0; i < crossCount_; i++) {
        if (currentIndex >= gridLayoutInfo_.childrenCount_) {
            break;
        }
        // Step1. Get wrapper of [GridItem]
        auto itemWrapper = layoutWrapper->GetChildByIndex(currentIndex, true);
        if (!itemWrapper || itemWrapper->CheckNeedForceMeasureAndLayout()) {
            for (uint32_t x = i; x < crossCount_; x++) {
                predictBuildList_.emplace_back(currentIndex++);
            }
            if (GreatOrEqual(cellAveLength_, 0.0f) &&
                gridLayoutInfo_.lineHeightMap_.find(currentLine) == gridLayoutInfo_.lineHeightMap_.end()) {
                gridLayoutInfo_.lineHeightMap_[currentLine] = cellAveLength_;
            }
            return -1.0f;
        }
        // // Step2. Measure child
        auto frameSize = axis_ == Axis::VERTICAL ? SizeF(crossSize, mainSize) : SizeF(mainSize, crossSize);
        auto crossSpan = MeasureCachedChild(frameSize, currentIndex, layoutWrapper, itemWrapper);
        if (crossSpan < 0) {
            cellAveLength_ = LessNotEqual(cellAveLength_, 0.0)
                                 ? gridLayoutInfo_.lineHeightMap_.find(currentLine - 1)->second
                                 : cellAveLength_;
            --currentIndex;
            break;
        }
        i = static_cast<uint32_t>(lastCross_ - 1);
        // // Step3. Measure [GridItem]
        LargeItemLineHeight(itemWrapper, hasNormalItem);

        gridLayoutInfo_.endIndex_ = currentIndex;
        currentIndex++;
        doneFillLine = true;
    }

    if (doneFillLine || gridLayoutInfo_.gridMatrix_.find(currentLine) != gridLayoutInfo_.gridMatrix_.end()) {
        gridLayoutInfo_.lineHeightMap_[currentLine] = cellAveLength_;
        gridLayoutInfo_.endMainLineIndex_ = currentLine;
    } else {
        return -1.0f;
    }
    return cellAveLength_;
}

int32_t GridScrollLayoutAlgorithm::MeasureCachedChild(const SizeF& frameSize, int32_t itemIndex,
    LayoutWrapper* layoutWrapper, const RefPtr<LayoutWrapper>& childLayoutWrapper)
{
    auto mainCount = static_cast<int32_t>(mainCount_);
    auto crossCount = static_cast<int32_t>(crossCount_);
    AdjustRowColSpan(childLayoutWrapper, layoutWrapper, itemIndex);
    auto mainSpan = axis_ == Axis::VERTICAL ? currentItemRowSpan_ : currentItemColSpan_;
    auto crossSpan = axis_ == Axis::VERTICAL ? currentItemColSpan_ : currentItemRowSpan_;
    auto crossStart = axis_ == Axis::VERTICAL ? currentItemColStart_ : currentItemRowStart_;
    if (crossSpan > crossCount) {
        return crossSpan;
    }
    int32_t mainIndex = currentMainLineIndex_;

    if (crossStart >= 0 && crossStart < crossCount) {
        if (crossStart < lastCross_) {
            return -1;
        } else if (CheckGridPlaced(itemIndex, mainIndex, crossStart, mainSpan, crossSpan)) {
            itemsCrossPosition_.try_emplace(itemIndex, ComputeItemCrossPosition(layoutWrapper, crossStart));
        } else {
            return -1;
        }
    } else {
        int32_t crossIndex = crossStart >= 0 ? crossStart : lastCross_;

        while (!CheckGridPlaced(itemIndex, mainIndex, crossIndex, mainSpan, crossSpan)) {
            if (GetNextGrid(mainIndex, crossIndex, false) == false) {
                return -1;
            }
            if (mainIndex >= mainCount || crossIndex >= crossCount) {
                break;
            }
        }

        itemsCrossPosition_.try_emplace(itemIndex, ComputeItemCrossPosition(layoutWrapper, crossIndex));
    }
    return crossSpan;
}

void GridScrollLayoutAlgorithm::CompleteItemCrossPosition(
    LayoutWrapper* layoutWrapper, std::map<int32_t, int32_t> items)
{
    for (auto item : items) {
        auto currentIndex = item.second;
        auto itemWrapper = layoutWrapper->GetChildByIndex(currentIndex, true);
        if (!itemWrapper) {
            if (predictBuildList_.back() < currentIndex) {
                predictBuildList_.push_front(currentIndex);
            } else if (predictBuildList_.front() > currentIndex) {
                predictBuildList_.emplace_back(currentIndex);
            }
        }
        itemsCrossPosition_.try_emplace(currentIndex, ComputeItemCrossPosition(layoutWrapper, item.first));
    }
}

void GridScrollLayoutAlgorithm::PostIdleTask(RefPtr<FrameNode> frameNode, const GridPredictLayoutParam& param)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<GridPattern>();
    CHECK_NULL_VOID(pattern);
    if (pattern->GetPredictLayoutParam()) {
        pattern->SetPredictLayoutParam(param);
        return;
    }
    pattern->SetPredictLayoutParam(param);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    context->AddPredictTask([weak = WeakClaim(RawPtr(frameNode))](int64_t deadline, bool canUseLongPredictTask) {
        ACE_SCOPED_TRACE("Grid predict");
        auto frameNode = weak.Upgrade();
        CHECK_NULL_VOID(frameNode);
        auto pattern = frameNode->GetPattern<GridPattern>();
        CHECK_NULL_VOID(pattern);
        if (!pattern->GetPredictLayoutParam().has_value()) {
            return;
        }
        Axis axis = pattern->GetAxis();
        bool needMarkDirty = false;
        auto param = pattern->GetPredictLayoutParam().value();
        auto firstItem = param.items.begin();
        if (pattern->IsPredictOutOfRange(*firstItem)) {
            param.items.clear();
        }
        for (auto it = param.items.begin(); it != param.items.end();) {
            if (GetSysTimestamp() > deadline) {
                break;
            }
            auto wrapper = frameNode->GetOrCreateChildByIndex(*it, false, true);
            CHECK_NULL_BREAK(wrapper);
            auto itemProperty = DynamicCast<GridItemLayoutProperty>(wrapper->GetLayoutProperty());
            CHECK_NULL_BREAK(itemProperty);
            int32_t crossSpan = itemProperty->GetCrossSpan(axis);
            int32_t crossStart = itemProperty->GetCrossStart(axis);
            auto constraint = param.layoutConstraint;
            if (crossSpan > 1) {
                float itemCrossSize = param.crossGap * (crossSpan - 1);
                for (int32_t index = 0; index < crossSpan; ++index) {
                    int32_t crossIndex = (crossStart + index) % static_cast<int32_t>(param.itemsCrossSizes.size());
                    if (crossIndex >= 0 && crossIndex < static_cast<int32_t>(param.itemsCrossSizes.size())) {
                        itemCrossSize += param.itemsCrossSizes.at(crossIndex);
                    }
                }
                constraint.maxSize.SetCrossSize(itemCrossSize, axis);
                constraint.selfIdealSize.SetCrossSize(itemCrossSize, axis);
            }
            needMarkDirty = PredictBuildItem(wrapper, constraint) || needMarkDirty;
            param.items.erase(it++);
        }
        if (needMarkDirty) {
            frameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        }
        pattern->SetPredictLayoutParam(std::nullopt);
        if (!param.items.empty() && !needMarkDirty) {
            GridScrollLayoutAlgorithm::PostIdleTask(frameNode, param);
            pattern->SetPredictLayoutParam(param);
        }
    });
}

bool GridScrollLayoutAlgorithm::PredictBuildItem(RefPtr<LayoutWrapper> wrapper, const LayoutConstraintF& constraint)
{
    CHECK_NULL_RETURN(wrapper, false);
    wrapper->SetActive(false);
    auto frameNode = wrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, false);
    frameNode->GetGeometryNode()->SetParentLayoutConstraint(constraint);
    FrameNode::ProcessOffscreenNode(frameNode);
    return true;
}

void GridScrollLayoutAlgorithm::CreateCachedChildConstraint(
    LayoutWrapper* layoutWrapper, float mainSize, float crossSize)
{
    auto gridLayoutProperty = DynamicCast<GridLayoutProperty>(layoutWrapper->GetLayoutProperty());
    cachedChildConstraint_ = CreateChildConstraint(mainSize, crossSize, gridLayoutProperty, 0, 1);
}

void GridScrollLayoutAlgorithm::UpdateMainLineOnReload(int32_t startIdx)
{
    auto& info = gridLayoutInfo_;
    if (!info.hasBigItem_) {
        info.startMainLineIndex_ = startIdx / info.crossCount_;
    }
}

std::pair<bool, bool> GridScrollLayoutAlgorithm::GetResetMode(LayoutWrapper* layoutWrapper, int32_t updateIdx)
{
    if (updateIdx == -1) {
        return { 0, 0 };
    }
    bool outOfMatrix = false;
    if (updateIdx != -1 && updateIdx < gridLayoutInfo_.startIndex_) {
        int32_t startLine = 0;
        outOfMatrix = !IsIndexInMatrix(updateIdx, startLine);
    }
    auto gridLayoutProperty = AceType::DynamicCast<GridLayoutProperty>(layoutWrapper->GetLayoutProperty());
    bool hasOptions = gridLayoutProperty->GetLayoutOptions().has_value();
    return { !gridLayoutInfo_.hasBigItem_ || outOfMatrix || hasOptions,
        gridLayoutInfo_.hasBigItem_ && !outOfMatrix && !hasOptions };
}

void GridScrollLayoutAlgorithm::CheckReset(float mainSize, float crossSize, LayoutWrapper* layoutWrapper)
{
    int32_t updateIdx = layoutWrapper->GetHostNode()->GetChildrenUpdated();
    // [resetFromStart,resetFromUpdate]
    std::pair<bool, bool> resetMode = GetResetMode(layoutWrapper, updateIdx);
    if (gridLayoutInfo_.lastCrossCount_ != crossCount_ || resetMode.first || gridLayoutInfo_.IsResetted()) {
        gridLayoutInfo_.lastCrossCount_ = crossCount_;
        gridLayoutInfo_.lineHeightMap_.clear();
        gridLayoutInfo_.gridMatrix_.clear();
        gridLayoutInfo_.irregularItemsPosition_.clear();
        gridLayoutInfo_.endIndex_ = -1;
        gridLayoutInfo_.endMainLineIndex_ = 0;
        gridLayoutInfo_.prevOffset_ = gridLayoutInfo_.currentOffset_;
        gridLayoutInfo_.ResetPositionFlags();
        isChildrenUpdated_ = true;
        if (gridLayoutInfo_.childrenCount_ > 0) {
            ReloadToStartIndex(mainSize, crossSize, layoutWrapper);
        }
        if (IsScrollToEndLine()) {
            gridLayoutInfo_.currentOffset_ =
                mainSize - gridLayoutInfo_.lineHeightMap_[gridLayoutInfo_.endMainLineIndex_];
            gridLayoutInfo_.prevOffset_ = gridLayoutInfo_.currentOffset_;
        }
    } else if (resetMode.second) {
        isChildrenUpdated_ = true;
        gridLayoutInfo_.irregularItemsPosition_.clear();
        gridLayoutInfo_.ResetPositionFlags();
        gridLayoutInfo_.prevOffset_ = gridLayoutInfo_.currentOffset_;
        auto it = gridLayoutInfo_.FindInMatrix(updateIdx);
        it = gridLayoutInfo_.FindStartLineInMatrix(it, updateIdx);
        if (it != gridLayoutInfo_.gridMatrix_.end()) {
            gridLayoutInfo_.ClearMatrixToEnd(updateIdx, it->first);
            gridLayoutInfo_.ClearHeightsFromMatrix(it->first);
            if (updateIdx <= gridLayoutInfo_.startIndex_) {
                ReloadFromUpdateIdxToStartIndex(mainSize, crossSize, it->first, layoutWrapper);
            }
        }
    }
}

bool GridScrollLayoutAlgorithm::CheckLastLineItemFullyShowed(LayoutWrapper* layoutWrapper)
{
    auto lastLine = gridLayoutInfo_.gridMatrix_.find(gridLayoutInfo_.endMainLineIndex_);
    if (lastLine != gridLayoutInfo_.gridMatrix_.end()) {
        for (const auto [corssIndex, itemIndex] : lastLine->second) {
            auto itemWrapper = layoutWrapper->GetChildByIndex(itemIndex);
            if (!itemWrapper) {
                continue;
            }
            auto itemLayoutProperty = DynamicCast<GridItemLayoutProperty>(itemWrapper->GetLayoutProperty());
            if (!itemLayoutProperty) {
                continue;
            }
            if (itemLayoutProperty->GetMainSpan(axis_) == 1) {
                continue;
            }
            auto it = gridLayoutInfo_.FindStartLineInMatrix(lastLine, itemIndex);
            if (it == gridLayoutInfo_.gridMatrix_.end()) {
                continue;
            }
            int32_t startLine = it->first;
            if (startLine + itemLayoutProperty->GetMainSpan(axis_) > gridLayoutInfo_.endMainLineIndex_ + 1) {
                return false;
            }
        }
    }
    return true;
}
} // namespace OHOS::Ace::NG
