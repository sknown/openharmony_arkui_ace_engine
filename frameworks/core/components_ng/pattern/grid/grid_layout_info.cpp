/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "core/components_ng/pattern/grid/grid_layout_info.h"

#include <numeric>

#include "base/utils/utils.h"
#include "core/components_ng/pattern/scrollable/scrollable_properties.h"

namespace OHOS::Ace::NG {
int32_t GridLayoutInfo::GetItemIndexByPosition(int32_t position)
{
    auto iter = positionItemIndexMap_.find(position);
    if (iter != positionItemIndexMap_.end()) {
        return iter->second;
    }
    return position;
}

int32_t GridLayoutInfo::GetPositionByItemIndex(int32_t itemIndex)
{
    auto position = itemIndex;
    auto find = std::find_if(positionItemIndexMap_.begin(), positionItemIndexMap_.end(),
        [itemIndex](const std::pair<int32_t, int32_t>& item) { return item.second == itemIndex; });
    if (find != positionItemIndexMap_.end()) {
        position = find->first;
    }

    return position;
}

int32_t GridLayoutInfo::GetOriginalIndex() const
{
    return currentMovingItemPosition_;
}

void GridLayoutInfo::ClearDragState()
{
    positionItemIndexMap_.clear();
    currentMovingItemPosition_ = -1;
    currentRect_.Reset();
}

void GridLayoutInfo::MoveItemsBack(int32_t from, int32_t to, int32_t itemIndex)
{
    auto lastItemIndex = itemIndex;
    for (int32_t i = from; i <= to; ++i) {
        int32_t mainIndex = (i - startIndex_) / crossCount_ + startMainLineIndex_;
        int32_t crossIndex = (i - startIndex_) % crossCount_;
        if (i == from) {
            gridMatrix_[mainIndex][crossIndex] = itemIndex;
        } else {
            auto index = GetItemIndexByPosition(i - 1);
            gridMatrix_[mainIndex][crossIndex] = index;
            positionItemIndexMap_[i - 1] = lastItemIndex;
            lastItemIndex = index;
        }
    }
    positionItemIndexMap_[from] = itemIndex;
    positionItemIndexMap_[to] = lastItemIndex;
    currentMovingItemPosition_ = from;
}

void GridLayoutInfo::MoveItemsForward(int32_t from, int32_t to, int32_t itemIndex)
{
    for (int32_t i = from; i <= to; ++i) {
        int32_t mainIndex = (i - startIndex_) / crossCount_ + startMainLineIndex_;
        int32_t crossIndex = (i - startIndex_) % crossCount_;
        if (i == to) {
            gridMatrix_[mainIndex][crossIndex] = itemIndex;
        } else {
            auto index = GetItemIndexByPosition(i + 1);
            gridMatrix_[mainIndex][crossIndex] = index;
            positionItemIndexMap_[i] = index;
        }
    }
    positionItemIndexMap_[to] = itemIndex;
    currentMovingItemPosition_ = to;
}

void GridLayoutInfo::SwapItems(int32_t itemIndex, int32_t insertIndex)
{
    currentMovingItemPosition_ = currentMovingItemPosition_ == -1 ? itemIndex : currentMovingItemPosition_;
    auto insertPosition = insertIndex;
    // drag from another grid
    if (itemIndex == -1) {
        if (currentMovingItemPosition_ == -1) {
            MoveItemsBack(insertPosition, childrenCount_, itemIndex);
            return;
        }
    } else {
        insertPosition = GetPositionByItemIndex(insertIndex);
    }

    if (currentMovingItemPosition_ > insertPosition) {
        MoveItemsBack(insertPosition, currentMovingItemPosition_, itemIndex);
        return;
    }

    if (insertPosition > currentMovingItemPosition_) {
        MoveItemsForward(currentMovingItemPosition_, insertPosition, itemIndex);
    }
}

void GridLayoutInfo::UpdateEndLine(float mainSize, float mainGap)
{
    if (mainSize >= lastMainSize_) {
        return;
    }
    for (auto i = startMainLineIndex_; i < endMainLineIndex_; ++i) {
        mainSize -= (lineHeightMap_[i] + mainGap);
        if (LessOrEqual(mainSize + mainGap, 0)) {
            endMainLineIndex_ = i;
            break;
        }
    }
}

void GridLayoutInfo::UpdateEndIndex(float overScrollOffset, float mainSize, float mainGap)
{
    auto remainSize = mainSize - overScrollOffset;
    for (auto i = startMainLineIndex_; i < endMainLineIndex_; ++i) {
        remainSize -= (lineHeightMap_[i] + mainGap);
        if (LessOrEqual(remainSize + mainGap, 0)) {
            auto endLine = gridMatrix_.find(i);
            CHECK_NULL_VOID(endLine != gridMatrix_.end());
            CHECK_NULL_VOID(!endLine->second.empty());
            endIndex_ = endLine->second.rbegin()->second;
            break;
        }
    }
}

bool GridLayoutInfo::IsOutOfStart() const
{
    return reachStart_ && Positive(currentOffset_);
}

bool GridLayoutInfo::IsOutOfEnd() const
{
    auto atOrOutofStart = reachStart_ && NonNegative(currentOffset_);
    auto endPos = currentOffset_ + totalHeightOfItemsInView_;
    return !atOrOutofStart && (endIndex_ == childrenCount_ - 1) &&
           LessNotEqual(endPos, lastMainSize_ - contentEndPadding_);
}

float GridLayoutInfo::GetCurrentOffsetOfRegularGrid(float mainGap) const
{
    float defaultHeight = GetCurrentLineHeight();
    auto lines = startIndex_ / crossCount_;
    float res = 0.0f;
    for (int i = 0; i < lines; ++i) {
        auto it = lineHeightMap_.find(i);
        res += (it != lineHeightMap_.end() ? it->second : defaultHeight) + mainGap;
    }
    return res - currentOffset_;
}

float GridLayoutInfo::GetContentOffset(float mainGap) const
{
    if (lineHeightMap_.empty()) {
        return 0.0f;
    }
    if (!hasBigItem_) {
        return GetCurrentOffsetOfRegularGrid(mainGap);
    }
    // assume lineHeightMap is continuous in range [begin, rbegin].
    int32_t itemCount = FindItemCount(lineHeightMap_.begin()->first, lineHeightMap_.rbegin()->first);
    if (itemCount == 0) {
        return 0.0f;
    }
    if (itemCount == childrenCount_ || (lineHeightMap_.begin()->first == 0 && itemCount >= startIndex_)) {
        return GetStartLineOffset(mainGap);
    }
    // begin estimation
    float heightSum = GetTotalLineHeight(mainGap, false);
    if (itemCount == 0) {
        return 0.0f;
    }
    auto averageHeight = heightSum / itemCount;
    return startIndex_ * averageHeight - currentOffset_;
}

int32_t GridLayoutInfo::FindItemCount(int32_t startLine, int32_t endLine) const
{
    auto firstLine = gridMatrix_.find(startLine);
    auto lastLine = gridMatrix_.find(endLine);
    if (firstLine == gridMatrix_.end() || firstLine->second.empty()) {
        for (auto i = startLine; i <= endLine; ++i) {
            auto it = gridMatrix_.find(i);
            if (it != gridMatrix_.end()) {
                firstLine = it;
                break;
            }
        }
        if (firstLine == gridMatrix_.end() || firstLine->second.empty()) {
            return 0;
        }
    }
    if (lastLine == gridMatrix_.end() || lastLine->second.empty()) {
        for (auto i = endLine; i >= startLine; --i) {
            auto it = gridMatrix_.find(i);
            if (it != gridMatrix_.end()) {
                lastLine = it;
                break;
            }
        }
        if (lastLine == gridMatrix_.end() || lastLine->second.empty()) {
            return 0;
        }
    }

    int32_t minIdx = firstLine->second.begin()->second;

    int32_t maxIdx = 0;
    // maxIdx might not be in the last position if hasBigItem_
    for (const auto& it : lastLine->second) {
        maxIdx = std::max(maxIdx, it.second);
    }
    maxIdx = std::max(maxIdx, FindEndIdx(endLine).itemIdx);
    return maxIdx - minIdx + 1;
}

float GridLayoutInfo::GetContentHeightOfRegularGrid(float mainGap) const
{
    float lineHeight = GetCurrentLineHeight();
    float res = 0.0f;
    auto lines = (childrenCount_) / crossCount_;
    for (int i = 0; i < lines; ++i) {
        auto it = lineHeightMap_.find(i);
        res += (it != lineHeightMap_.end() ? it->second : lineHeight) + mainGap;
    }
    if (childrenCount_ % crossCount_ == 0) {
        return res - mainGap;
    }
    auto lastLine = lineHeightMap_.find(lines);
    return res + (lastLine != lineHeightMap_.end() ? lastLine->second : lineHeight);
}

float GridLayoutInfo::GetContentHeight(float mainGap) const
{
    if (!hasBigItem_) {
        return GetContentHeightOfRegularGrid(mainGap);
    }
    if (lineHeightMap_.empty()) {
        return 0.0f;
    }
    float heightSum = GetTotalLineHeight(mainGap, false);
    // assume lineHeightMap is continuous in range [begin, rbegin]
    int32_t itemCount = FindItemCount(lineHeightMap_.begin()->first, lineHeightMap_.rbegin()->first);
    if (itemCount == 0) {
        return 0.0f;
    }
    float averageHeight = heightSum / itemCount;

    if (itemCount == childrenCount_) {
        return heightSum - mainGap;
    }
    return heightSum + (childrenCount_ - itemCount) * averageHeight;
}

float GridLayoutInfo::GetContentOffset(const GridLayoutOptions& options, float mainGap) const
{
    if (startIndex_ == 0) {
        return -currentOffset_;
    }
    if (options.irregularIndexes.empty() || startIndex_ < *(options.irregularIndexes.begin())) {
        return GetCurrentOffsetOfRegularGrid(mainGap);
    }
    if (options.getSizeByIndex) {
        return GetContentOffset(mainGap);
    }
    float prevHeight = GetContentHeight(options, startIndex_, mainGap) + mainGap;
    return prevHeight - currentOffset_;
}

namespace {
// prevIdx and idx are indices of two irregular items that take up a whole line
inline float AddLinesInBetween(int32_t prevIdx, int32_t idx, int32_t crossCount, float lineHeight)
{
    if (crossCount == 0) {
        return 0.0f;
    }
    return (idx - prevIdx) > 1 ? ((idx - 2 - prevIdx) / crossCount + 1) * lineHeight : 0.0f;
}
} // namespace

void GridLayoutInfo::GetLineHeights(
    const GridLayoutOptions& options, float mainGap, float& regularHeight, float& irregularHeight) const
{
    for (const auto& item : lineHeightMap_) {
        auto line = gridMatrix_.find(item.first);
        if (line == gridMatrix_.end()) {
            continue;
        }
        if (line->second.empty() || LessOrEqual(item.second, 0.0f)) {
            continue;
        }
        auto lineStart = line->second.begin()->second;
        if (options.irregularIndexes.find(lineStart) != options.irregularIndexes.end()) {
            irregularHeight = item.second + mainGap;
        } else {
            if (NearZero(regularHeight)) {
                regularHeight = item.second + mainGap;
            }
        }
        if (!(NearZero(irregularHeight) || NearZero(regularHeight))) {
            break;
        }
    }
}

float GridLayoutInfo::GetContentHeight(const GridLayoutOptions& options, int32_t endIdx, float mainGap) const
{
    if (options.irregularIndexes.empty()) {
        return GetContentHeightOfRegularGrid(mainGap);
    }
    if (options.getSizeByIndex) {
        return GetContentHeight(mainGap);
    }

    float irregularHeight = 0.0f;
    float regularHeight = 0.0f;
    GetLineHeights(options, mainGap, regularHeight, irregularHeight);
    if (NearZero(irregularHeight)) {
        irregularHeight = lastIrregularMainSize_;
    }
    if (NearZero(regularHeight)) {
        regularHeight = lastRegularMainSize_;
    }

    // get line count
    auto firstIrregularIndex = *(options.irregularIndexes.begin());
    float totalHeight = AddLinesInBetween(-1, firstIrregularIndex, crossCount_, regularHeight);
    auto lastIndex = firstIrregularIndex;
    for (int32_t idx : options.irregularIndexes) {
        if (idx >= endIdx) {
            break;
        }
        totalHeight += irregularHeight;
        totalHeight += AddLinesInBetween(lastIndex, idx, crossCount_, regularHeight);
        lastIndex = idx;
    }

    totalHeight += AddLinesInBetween(lastIndex, endIdx, crossCount_, regularHeight);
    totalHeight -= mainGap;
    return totalHeight;
}

float GridLayoutInfo::GetIrregularOffset(float mainGap) const
{
    // need to calculate total line height before startMainLine_
    // gridMatrix ready up to endLine, so lineCnt is known.
    // get sum of existing lines
    // use average to estimate unknown lines
    if (lineHeightMap_.empty() || childrenCount_ == 0) {
        return 0.0f;
    }

    auto it = lineHeightMap_.lower_bound(startMainLineIndex_);
    auto knownLineCnt = static_cast<float>(std::distance(lineHeightMap_.begin(), it));
    float knownHeight = GetHeightInRange(lineHeightMap_.begin()->first, startMainLineIndex_, 0.0f);
    float avgHeight = synced_ ? avgLineHeight_ : GetTotalLineHeight(0.0f) / static_cast<float>(lineHeightMap_.size());

    auto startLine = static_cast<float>(startMainLineIndex_);
    float estTotal = knownHeight + avgHeight * (startLine - knownLineCnt);
    return estTotal + startLine * mainGap - currentOffset_;
}

float GridLayoutInfo::GetIrregularHeight(float mainGap) const
{
    // count current number of lines
    // estimate total number of lines based on {known item / total item}
    if (lineHeightMap_.empty() || childrenCount_ == 0) {
        return 0.0f;
    }
    int32_t lastKnownLine = lineHeightMap_.rbegin()->first;
    float itemRatio = static_cast<float>(FindEndIdx(lastKnownLine).itemIdx + 1) / static_cast<float>(childrenCount_);
    float estTotalLines = std::round(static_cast<float>(lastKnownLine + 1) / itemRatio);

    auto knownLineCnt = static_cast<float>(lineHeightMap_.size());
    float knownHeight = synced_ ? avgLineHeight_ * knownLineCnt : GetTotalLineHeight(0.0f);
    float avgHeight = synced_ ? avgLineHeight_ : knownHeight / knownLineCnt;
    return knownHeight + (estTotalLines - knownLineCnt) * avgHeight + (estTotalLines - 1) * mainGap;
}

void GridLayoutInfo::SkipStartIndexByOffset(const GridLayoutOptions& options, float mainGap)
{
    auto targetContent = currentHeight_ - (currentOffset_ - prevOffset_);
    if (LessOrEqual(targetContent, 0.0)) {
        currentOffset_ = 0.0f;
        startIndex_ = 0;
        return;
    }

    float irregularHeight = 0.0f;
    float regularHeight = 0.0f;
    GetLineHeights(options, mainGap, regularHeight, irregularHeight);
    if (NearZero(irregularHeight)) {
        irregularHeight = lastIrregularMainSize_;
    } else {
        lastIrregularMainSize_ = irregularHeight;
    }
    if (NearZero(regularHeight)) {
        regularHeight = lastRegularMainSize_;
    } else {
        lastRegularMainSize_ = regularHeight;
    }

    auto firstIrregularIndex = *(options.irregularIndexes.begin());
    float totalHeight = AddLinesInBetween(-1, firstIrregularIndex, crossCount_, regularHeight);
    auto lastIndex = firstIrregularIndex;
    auto lastHeight = 0.0f;

    for (auto idx : options.irregularIndexes) {
        if (GreatOrEqual(totalHeight, targetContent)) {
            break;
        }
        lastHeight = totalHeight;
        totalHeight += irregularHeight;
        totalHeight += AddLinesInBetween(lastIndex, idx, crossCount_, regularHeight);
        lastIndex = idx;
    }
    auto lines = static_cast<int32_t>(std::floor((targetContent - lastHeight) / regularHeight));
    currentOffset_ = lastHeight + lines * regularHeight - targetContent;
    auto startIdx = lines * crossCount_ + lastIndex;
    startIndex_ = std::min(startIdx, childrenCount_ - 1);
}

float GridLayoutInfo::GetCurrentLineHeight() const
{
    auto currentLineHeight = lineHeightMap_.find(startMainLineIndex_);
    auto currentLineMatrix = gridMatrix_.find(startMainLineIndex_);
    // if current line exist, find it
    if (currentLineHeight != lineHeightMap_.end() && currentLineMatrix != gridMatrix_.end() &&
        !currentLineMatrix->second.empty()) {
        return currentLineHeight->second;
    }

    // otherwise return the first line in cache
    for (const auto& item : lineHeightMap_) {
        auto line = gridMatrix_.find(item.first);
        if (line == gridMatrix_.end()) {
            continue;
        }
        if (line->second.empty()) {
            continue;
        }
        return item.second;
    }
    return 0.0f;
}

std::pair<int32_t, int32_t> GridLayoutInfo::FindItemInRange(int32_t target) const
{
    if (gridMatrix_.empty()) {
        return { -1, -1 };
    }
    for (int r = startMainLineIndex_; r <= endMainLineIndex_; ++r) {
        const auto& row = gridMatrix_.at(r);
        for (const auto& it : row) {
            if (it.second == target) {
                return { r, it.first };
            }
        }
    }
    return { -1, -1 };
}

// Use the index to get the line number where the item is located
bool GridLayoutInfo::GetLineIndexByIndex(int32_t targetIndex, int32_t& targetLineIndex) const
{
    for (auto [lineIndex, lineMap] : gridMatrix_) {
        for (auto [crossIndex, index] : lineMap) {
            if (index == targetIndex) {
                targetLineIndex = lineIndex;
                return true;
            }
        }
    }
    return false;
}

// get the total height of all rows from zero before the targetLineIndex
float GridLayoutInfo::GetTotalHeightFromZeroIndex(int32_t targetLineIndex, float mainGap) const
{
    auto targetPos = 0.f;
    for (auto [lineIndex, lineHeight] : lineHeightMap_) {
        if (targetLineIndex > lineIndex) {
            targetPos += lineHeight + mainGap;
        } else {
            break;
        }
    }
    return targetPos;
}

ScrollAlign GridLayoutInfo::TransformAutoScrollAlign(
    int32_t itemIdx, int32_t height, float mainSize, float mainGap) const
{
    if (itemIdx >= startIndex_ && itemIdx <= endIndex_) {
        auto [line, _] = FindItemInRange(itemIdx);
        float topPos = GetItemTopPos(line, mainGap);
        float botPos = GetItemBottomPos(line, height, mainGap);
        if (NonPositive(topPos) && GreatOrEqual(botPos, mainSize)) {
            // item occupies the whole viewport
            return ScrollAlign::NONE;
        }
        // scrollAlign start / end if the item is not fully in viewport
        if (Negative(topPos)) {
            return ScrollAlign::START;
        }
        if (GreatNotEqual(botPos, mainSize)) {
            return ScrollAlign::END;
        }
        return ScrollAlign::NONE;
    }
    if (itemIdx > endIndex_) {
        return ScrollAlign::END;
    }
    return ScrollAlign::START;
}

float GridLayoutInfo::GetAnimatePosIrregular(int32_t targetIdx, int32_t height, ScrollAlign align, float mainGap) const
{
    auto it = FindInMatrix(targetIdx);
    if (it == gridMatrix_.end()) {
        return -1.0f;
    }
    if (align == ScrollAlign::AUTO) {
        align = TransformAutoScrollAlign(targetIdx, height, lastMainSize_, mainGap);
    }
    switch (align) {
        case ScrollAlign::START:
            return GetTotalHeightFromZeroIndex(it->first, mainGap);
        case ScrollAlign::CENTER: {
            auto [center, offset] = FindItemCenter(it->first, height, mainGap);
            float res = GetTotalHeightFromZeroIndex(center, mainGap) + offset - lastMainSize_ / 2.0f;
            return std::max(res, 0.0f);
        }
        case ScrollAlign::END: {
            float res = GetTotalHeightFromZeroIndex(it->first + height, mainGap) - mainGap - lastMainSize_;
            return std::max(res, 0.0f);
        }
        default:
            return -1.0f;
    }
}

// Based on the index from zero and align, gets the position to scroll to
bool GridLayoutInfo::GetGridItemAnimatePos(const GridLayoutInfo& currentGridLayoutInfo, int32_t targetIndex,
    ScrollAlign align, float mainGap, float& targetPos)
{
    int32_t startMainLineIndex = currentGridLayoutInfo.startMainLineIndex_;
    int32_t endMainLineIndex = currentGridLayoutInfo.endMainLineIndex_;
    float lastMainSize = currentGridLayoutInfo.lastMainSize_;
    int32_t targetLineIndex = -1;
    // Pre-check
    // Get the line number where the index is located. If targetIndex does not exist, it is returned.
    CHECK_NULL_RETURN(GetLineIndexByIndex(targetIndex, targetLineIndex), false);

    // Get the total height of the targetPos from row 0 to targetLineIndex-1.
    targetPos = GetTotalHeightFromZeroIndex(targetLineIndex, mainGap);

    // Find the target row and get the altitude information
    auto targetItem = lineHeightMap_.find(targetLineIndex);

    // Make sure that the target line has height information
    CHECK_NULL_RETURN(targetItem != lineHeightMap_.end(), false);
    auto targetLineHeight = targetItem->second;

    // Depending on align, calculate where you need to scroll to
    switch (align) {
        case ScrollAlign::START:
        case ScrollAlign::NONE:
            break;
        case ScrollAlign::CENTER: {
            targetPos -= ((lastMainSize - targetLineHeight) * HALF);
            break;
        }
        case ScrollAlign::END: {
            targetPos -= (lastMainSize - targetLineHeight);
            break;
        }
        case ScrollAlign::AUTO: {
            GetLineIndexByIndex(currentGridLayoutInfo.startIndex_, startMainLineIndex);
            GetLineIndexByIndex(currentGridLayoutInfo.endIndex_, endMainLineIndex);
            auto targetPosBeforeStartIndex = GetTotalHeightFromZeroIndex(startMainLineIndex, mainGap);
            // targetPos - targetPosBeforeStartIndex:The distance between the top of the startLine row and the top of
            // the targetLine row
            // The distance of the targetLine row from the top of the screen
            auto height2Top = targetPos - targetPosBeforeStartIndex - std::abs(currentGridLayoutInfo.currentOffset_);
            // The distance of the targetLine row from the bottom of the screen
            auto height2Bottom = std::abs(currentGridLayoutInfo.currentOffset_) + lastMainSize - targetPos +
                                 targetPosBeforeStartIndex - targetLineHeight;
            // This is handled when the targetLine line is the same as the endLine line. As for the period between
            // startLine and endLine, follow the following process
            if (GreatOrEqual(height2Top, 0.f) && GreatOrEqual(height2Bottom, 0.f)) {
                return false;
            }
            // When the row height is greater than the screen height and occupies the entire screen height, do nothing
            if ((startMainLineIndex == targetLineIndex) && (endMainLineIndex == targetLineIndex)) {
                if ((std::abs(currentGridLayoutInfo.currentOffset_) + lastMainSize - targetLineHeight) <= 0) {
                    return false;
                }
            }
            if (startMainLineIndex >= targetLineIndex) {
            } else if (targetLineIndex >= endMainLineIndex) {
                targetPos -= (lastMainSize - targetLineHeight);
            } else {
                return false;
            }
            break;
        }
    }
    return true;
}

namespace {
bool CheckRow(int32_t& maxV, const std::map<int, int>& row, int32_t target)
{
    for (auto [_, item] : row) {
        maxV = std::max(maxV, std::abs(item));
        if (item == target) {
            return true;
        }
    }
    return false;
}

using MatIter = decltype(GridLayoutInfo::gridMatrix_)::const_iterator;
} // namespace

MatIter GridLayoutInfo::FindInMatrix(int32_t index) const
{
    if (index == 0) {
        return gridMatrix_.begin();
    }
    size_t count = gridMatrix_.size();
    size_t step = 0;
    auto left = gridMatrix_.begin();
    auto it = left;
    while (count > 0) {
        it = left;
        step = count / 2;
        std::advance(it, step);

        // with irregular items, only the max index on each row is guaranteed to be in order.
        int32_t maxV = -1;
        if (CheckRow(maxV, it->second, index)) {
            return it;
        }

        if (index <= maxV) {
            count = step;
        } else {
            // index on the right side of current row
            left = ++it;
            count -= step + 1;
        }
    }
    return gridMatrix_.end();
}

GridLayoutInfo::EndIndexInfo GridLayoutInfo::FindEndIdx(int32_t endLine) const
{
    if (gridMatrix_.find(endLine) == gridMatrix_.end()) {
        return {};
    }
    for (int32_t rowIdx = endLine; rowIdx >= 0; --rowIdx) {
        const auto& row = gridMatrix_.at(rowIdx);
        for (auto it = row.rbegin(); it != row.rend(); ++it) {
            if (it->second > 0) {
                return { .itemIdx = it->second, .y = rowIdx, .x = it->first };
            }
        }
    }
    return { .itemIdx = 0, .y = 0, .x = 0 };
}

void GridLayoutInfo::ClearMapsToEnd(int32_t idx)
{
    if (hasMultiLineItem_) {
        ClearMapsToEndContainsMultiLineItem(idx - 1);
        return;
    }
    auto gridIt = gridMatrix_.lower_bound(idx);
    gridMatrix_.erase(gridIt, gridMatrix_.end());
    ClearHeightsToEnd(idx);
}

void GridLayoutInfo::ClearMapsToEndContainsMultiLineItem(int32_t idx)
{
    int32_t maxIndex = INT_MIN;
    for (const auto& col : gridMatrix_[idx]) {
        maxIndex = std::max(maxIndex, col.second);
    }

    int targetLine = idx;
    while (targetLine < gridMatrix_.rbegin()->first) {
        int32_t minIndex = INT_MAX;
        for (const auto& col : gridMatrix_[targetLine + 1]) {
            minIndex = std::min(minIndex, col.second);
        }
        if (maxIndex < minIndex) {
            break;
        }
        targetLine++;
    }
    gridMatrix_.erase(gridMatrix_.find(targetLine + 1), gridMatrix_.end());

    auto lineIt = lineHeightMap_.find(targetLine + 1);
    if (lineIt != lineHeightMap_.end()) {
        lineHeightMap_.erase(lineIt, lineHeightMap_.end());
    }
}

void GridLayoutInfo::ClearMapsFromStart(int32_t idx)
{
    if (hasMultiLineItem_) {
        ClearMapsFromStartContainsMultiLineItem(idx);
        return;
    }
    auto gridIt = gridMatrix_.lower_bound(idx);
    gridMatrix_.erase(gridMatrix_.begin(), gridIt);
    auto lineIt = lineHeightMap_.lower_bound(idx);
    lineHeightMap_.erase(lineHeightMap_.begin(), lineIt);
}

void GridLayoutInfo::ClearMapsFromStartContainsMultiLineItem(int32_t idx)
{
    int32_t minIndex = INT_MAX;
    for (const auto& col : gridMatrix_[idx]) {
        minIndex = std::min(minIndex, col.second);
    }

    auto iter = gridMatrix_.begin();
    int targetLine = idx;
    while (targetLine > iter->first) {
        int32_t maxIndex = INT_MIN;
        for (const auto& col : gridMatrix_[targetLine - 1]) {
            maxIndex = std::max(maxIndex, col.second);
        }
        if (maxIndex < minIndex) {
            break;
        }
        targetLine--;
    }
    gridMatrix_.erase(gridMatrix_.begin(), gridMatrix_.find(targetLine));

    auto lineIt = lineHeightMap_.find(targetLine);
    if (lineIt != lineHeightMap_.end()) {
        lineHeightMap_.erase(lineHeightMap_.begin(), lineIt);
    }
}

void GridLayoutInfo::ClearHeightsToEnd(int32_t idx)
{
    auto lineIt = lineHeightMap_.lower_bound(idx);
    lineHeightMap_.erase(lineIt, lineHeightMap_.end());
}

void GridLayoutInfo::ClearMatrixToEnd(int32_t idx, int32_t lineIdx)
{
    auto it = gridMatrix_.find(lineIdx);
    for (; it != gridMatrix_.end(); ++it) {
        for (auto itemIt = it->second.begin(); itemIt != it->second.end();) {
            if (std::abs(itemIt->second) < idx) {
                ++itemIt;
                continue;
            }
            itemIt = it->second.erase(itemIt);
        }
        if (it->second.empty()) {
            break;
        }
    }
    gridMatrix_.erase(it, gridMatrix_.end());
}

float GridLayoutInfo::GetTotalHeightOfItemsInView(float mainGap, bool regular) const
{
    float len = 0.0f;
    auto it = lineHeightMap_.find(startMainLineIndex_);
    if (!regular) {
        it = SkipLinesAboveView(mainGap).first;
    }
    if (it == lineHeightMap_.end()) {
        return -mainGap;
    }
    if (startMainLineIndex_ > endMainLineIndex_ || it->first > endMainLineIndex_) {
        return -mainGap;
    }
    auto endIt = lineHeightMap_.find(endMainLineIndex_ + 1);
    for (; it != endIt; ++it) {
        len += it->second + mainGap;
    }
    return len - mainGap;
}

std::pair<GridLayoutInfo::HeightMapIt, float> GridLayoutInfo::SkipLinesAboveView(float mainGap) const
{
    auto it = lineHeightMap_.find(startMainLineIndex_);
    float offset = currentOffset_;
    while (it != lineHeightMap_.end() && Negative(it->second + offset + mainGap)) {
        offset += it->second + mainGap;
        ++it;
    }
    return { it, offset };
}

float GridLayoutInfo::GetDistanceToBottom(float mainSize, float heightInView, float mainGap) const
{
    if (lineHeightMap_.empty() || endIndex_ < childrenCount_ - 1 ||
        endMainLineIndex_ < lineHeightMap_.rbegin()->first) {
        return Infinity<float>();
    }

    float offset = currentOffset_;
    // currentOffset_ is relative to startMainLine, which might be entirely above viewport
    auto it = lineHeightMap_.find(startMainLineIndex_);
    while (it != lineHeightMap_.end() && Negative(offset + it->second + mainGap)) {
        offset += it->second + mainGap;
        ++it;
    }
    float bottomPos = offset + heightInView;
    return bottomPos - mainSize;
}

void GridLayoutInfo::ClearHeightsFromMatrix(int32_t lineIdx)
{
    auto lineIt = lineHeightMap_.find(lineIdx);
    if (lineIt == lineHeightMap_.end()) {
        return;
    }
    if (gridMatrix_.find(lineIdx) != gridMatrix_.end()) {
        lineIt++;
    }
    lineHeightMap_.erase(lineIt, lineHeightMap_.end());
}

MatIter GridLayoutInfo::FindStartLineInMatrix(MatIter iter, int32_t index) const
{
    if (iter == gridMatrix_.end() || iter == gridMatrix_.begin()) {
        return iter;
    }

    --iter;
    int32_t maxValue = 0;
    while (CheckRow(maxValue, iter->second, index)) {
        if (iter == gridMatrix_.begin()) {
            return iter;
        }
        --iter;
    }
    return ++iter;
}

float GridLayoutInfo::GetHeightInRange(int32_t startLine, int32_t endLine, float mainGap) const
{
    if (endLine <= startLine) {
        return 0.0f;
    }
    auto endIt = lineHeightMap_.find(endLine);
    auto it = lineHeightMap_.find(startLine);
    if (it == lineHeightMap_.end()) {
        return 0.0f;
    }
    float totalHeight = 0.0f;
    for (; it != lineHeightMap_.end() && it != endIt; ++it) {
        totalHeight += it->second + mainGap;
    }
    return totalHeight;
}

bool GridLayoutInfo::HeightSumSmaller(float other, float mainGap) const
{
    other += mainGap;
    for (const auto& it : lineHeightMap_) {
        other -= it.second + mainGap;
        if (NonPositive(other)) {
            return false;
        }
    }
    return true;
}

std::pair<int32_t, float> GridLayoutInfo::FindItemCenter(int32_t startLine, int32_t lineCnt, float mainGap) const
{
    float halfLen = (GetHeightInRange(startLine, startLine + lineCnt, mainGap) - mainGap) / 2.0f;
    auto it = lineHeightMap_.find(startLine);
    float len = 0.0f;
    while (it != lineHeightMap_.end() && LessNotEqual(len + it->second + mainGap, halfLen)) {
        len += it->second + mainGap;
        ++it;
    }
    return { it->first, halfLen - len };
}

void GridLayoutInfo::PrepareJumpToBottom()
{
    if (gridMatrix_.empty() || gridMatrix_.rbegin()->second.empty()) {
        TAG_LOGW(ACE_GRID, "Matrix setup is incorrect");
        jumpIndex_ = LAST_ITEM;
    } else {
        jumpIndex_ = std::abs(gridMatrix_.rbegin()->second.begin()->second);
    }
    scrollAlign_ = ScrollAlign::END;
}
} // namespace OHOS::Ace::NG
