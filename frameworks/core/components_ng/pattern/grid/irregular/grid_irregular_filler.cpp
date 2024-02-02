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

#include "core/components_ng/pattern/grid/irregular/grid_irregular_filler.h"

#include "base/geometry/ng/size_t.h"
#include "core/components_ng/pattern/grid/grid_layout_property.h"
#include "core/components_ng/pattern/grid/irregular/grid_layout_utils.h"

namespace OHOS::Ace::NG {
GridIrregularFiller::GridIrregularFiller(GridLayoutInfo* info, LayoutWrapper* wrapper) : info_(info), wrapper_(wrapper)
{}

int32_t GridIrregularFiller::InitPos(int32_t lineIdx)
{
    // to land on the first item after advancing once
    posX_ = -1;
    posY_ = lineIdx;

    const auto& row = info_->gridMatrix_.find(lineIdx);
    if (row == info_->gridMatrix_.end()) {
        // implies empty matrix
        return -1;
    }
    return row->second.at(0) - 1;
}

using Result = GridIrregularFiller::FillResult;
Result GridIrregularFiller::Fill(const FillParameters& params, float targetLen, int32_t startingLine)
{
    int32_t idx = InitPos(startingLine);
    // no gap on first row
    float len = -params.mainGap;
    while (idx < info_->childrenCount_ - 1) {
        int32_t prevRow = posY_;
        if (!FindNextItem(++idx)) {
            FillOne(idx);
        }
        // (posY_ > prevRow) implies that the previous row has been filled

        int32_t row = prevRow;
        if (UpdateLength(len, targetLen, row, posY_, params.mainGap)) {
            return { len, row, idx - 1 };
        }

        MeasureItem(params, idx, posX_, posY_);
    }

    if (info_->lineHeightMap_.empty()) {
        return {};
    }
    // last child reached
    int32_t lastRow = info_->lineHeightMap_.rbegin()->first;
    if (UpdateLength(len, targetLen, posY_, lastRow + 1, params.mainGap)) {
        return { len, posY_, idx };
    }
    return { len, lastRow, idx };
}

void GridIrregularFiller::FillToTarget(const FillParameters& params, int32_t targetIdx, int32_t startingLine)
{
    if (targetIdx >= info_->childrenCount_) {
        targetIdx = info_->childrenCount_ - 1;
    }
    int32_t idx = InitPos(startingLine);
    while (idx < targetIdx) {
        if (!FindNextItem(++idx)) {
            FillOne(idx);
        }
        MeasureItem(params, idx, posX_, posY_);
    }
}

int32_t GridIrregularFiller::FitItem(const decltype(GridLayoutInfo::gridMatrix_)::iterator& it, int32_t itemWidth)
{
    if (it == info_->gridMatrix_.end()) {
        // empty row, can fit
        return 0;
    }

    if (it->second.size() + itemWidth > info_->crossCount_) {
        // not enough space
        return -1;
    }

    for (int i = 0; i <= info_->crossCount_ - itemWidth; ++i) {
        bool flag = true;
        for (int j = 0; j < itemWidth; ++j) {
            if (it->second.find(i + j) != it->second.end()) {
                // spot already filled
                flag = false;
                break;
            }
        }

        if (flag) {
            return i;
        }
    }
    return -1;
}

void GridIrregularFiller::FillOne(const int32_t idx)
{
    int32_t row = posY_;

    auto size = GridLayoutUtils::GetItemSize(info_, wrapper_, idx);

    auto it = info_->gridMatrix_.find(row);
    int32_t col = FitItem(it, size.columns);
    while (col == -1) {
        // can't fill at end, find the next available line
        it = info_->gridMatrix_.find(++row);
        col = FitItem(it, size.columns);
    }

    if (it == info_->gridMatrix_.end()) {
        // create a new line
        info_->gridMatrix_[row] = {};
    }

    // top left square should be set to [idx], the rest to -[idx]
    for (int32_t r = 0; r < size.rows; ++r) {
        for (int32_t c = 0; c < size.columns; ++c) {
            info_->gridMatrix_[row + r][col + c] = -idx;
        }
    }

    info_->gridMatrix_[row][col] = idx;

    posY_ = row;
    posX_ = col;
}

bool GridIrregularFiller::FindNextItem(int32_t target)
{
    const auto& mat = info_->gridMatrix_;
    while (AdvancePos()) {
        if (mat.at(posY_).at(posX_) == target) {
            return true;
        }
    }
    // to handle empty tiles in the middle of matrix, check next row
    auto nextRow = mat.find(posY_ + 1);
    if (nextRow != mat.end()) {
        for (const auto [col, item] : nextRow->second) {
            if (item == target) {
                ++posY_;
                posX_ = col;
                return true;
            }
        }
    }
    return false;
}

bool GridIrregularFiller::AdvancePos()
{
    ++posX_;
    if (posX_ == info_->crossCount_) {
        // go to next row
        ++posY_;
        posX_ = 0;
    }

    const auto& mat = info_->gridMatrix_;
    if (mat.find(posY_) == mat.end()) {
        return false;
    }

    const auto& row = mat.at(posY_);
    return row.find(posX_) != row.end();
}

bool GridIrregularFiller::UpdateLength(float& len, float targetLen, int32_t& row, int32_t rowBound, float mainGap) const
{
    for (; row < rowBound; ++row) {
        len += info_->lineHeightMap_.at(row) + mainGap;
        if (GreatOrEqual(len, targetLen)) {
            return true;
        }
    }
    return false;
}

void GridIrregularFiller::MeasureItem(const FillParameters& params, int32_t itemIdx, int32_t col, int32_t row)
{
    auto child = wrapper_->GetOrCreateChildByIndex(itemIdx);
    auto props = AceType::DynamicCast<GridLayoutProperty>(wrapper_->GetLayoutProperty());
    auto constraint = props->CreateChildConstraint();

    auto itemSize = GridLayoutUtils::GetItemSize(info_, wrapper_, itemIdx);
    // should cache child constraint result
    float crossLen = 0.0f;
    for (int32_t i = 0; i < itemSize.columns; ++i) {
        crossLen += params.crossLens[i + col];
    }
    crossLen += params.crossGap * (itemSize.columns - 1);

    constraint.percentReference.SetCrossSize(crossLen, info_->axis_);
    if (info_->axis_ == Axis::VERTICAL) {
        constraint.maxSize = SizeF { crossLen, Infinity<float>() };
        constraint.selfIdealSize = OptionalSizeF(crossLen, std::nullopt);
    } else {
        constraint.maxSize = SizeF { Infinity<float>(), crossLen };
        constraint.selfIdealSize = OptionalSizeF(std::nullopt, crossLen);
    }

    child->Measure(constraint);

    float childHeight = child->GetGeometryNode()->GetMarginFrameSize().MainSize(info_->axis_);
    // spread height to each row. May be buggy?
    float heightPerRow = (childHeight - (params.mainGap * (itemSize.rows - 1))) / itemSize.rows;
    for (int32_t i = 0; i < itemSize.rows; ++i) {
        info_->lineHeightMap_[row + i] = std::max(info_->lineHeightMap_[row + i], heightPerRow);
    }
}

int32_t GridIrregularFiller::InitPosToLastItem(int32_t lineIdx)
{
    auto res = info_->FindEndIdx(lineIdx);
    posX_ = res.x;
    posY_ = std::max(0, res.y);
    return res.itemIdx;
}

int32_t GridIrregularFiller::FillMatrixOnly(int32_t targetIdx)
{
    if (targetIdx >= info_->childrenCount_) {
        targetIdx = info_->childrenCount_ - 1;
    }
    int32_t idx = InitPosToLastItem(info_->gridMatrix_.size() - 1);
    while (idx < targetIdx) {
        if (!FindNextItem(++idx)) {
            FillOne(idx);
        }
    }
    return posY_;
}

int32_t GridIrregularFiller::FillMatrixByLine(int32_t startingLine, int32_t targetLine)
{
    int32_t idx = InitPosToLastItem(startingLine);
    while (posY_ < targetLine && idx < info_->childrenCount_ - 1) {
        if (!FindNextItem(++idx)) {
            FillOne(idx);
        }
    }
    return idx;
}

float GridIrregularFiller::MeasureBackward(const FillParameters& params, float targetLen, int32_t startingLine)
{
    float len = 0.0f;
    posY_ = startingLine;
    // only need to record irregular items
    std::unordered_set<int32_t> measured;

    for (; posY_ >= 0 && len < targetLen; --posY_) {
        BackwardImpl(measured, params);
        len += params.mainGap + info_->lineHeightMap_.at(posY_);
    }
    return len;
}

void GridIrregularFiller::MeasureBackwardToTarget(
    const FillParameters& params, int32_t targetLine, int32_t startingLine)
{
    if (targetLine < 0) {
        return;
    }
    posY_ = startingLine;

    std::unordered_set<int32_t> measured;
    for (; posY_ >= targetLine; --posY_) {
        BackwardImpl(measured, params);
    }
}

void GridIrregularFiller::BackwardImpl(std::unordered_set<int32_t>& measured, const FillParameters& params)
{
    const auto& row = info_->gridMatrix_.find(posY_)->second;
    for (int c = 0; c < info_->crossCount_; ++c) {
        if (row.find(c) == row.end()) {
            continue;
        }

        if (measured.find(row.at(c)) != measured.end()) {
            // skip all columns of a measured irregular item
            c += GridLayoutUtils::GetItemSize(info_, wrapper_, row.at(c)).columns - 1;
            continue;
        }

        int32_t topRow = FindItemTopRow(posY_, c);
        int32_t itemIdx = info_->gridMatrix_.at(topRow).at(c);
        MeasureItem(params, itemIdx, c, topRow);
        if (topRow < posY_) {
            // measure irregular items only once from the bottom-left tile
            measured.insert(itemIdx);
        }
        // skip all columns of this item
        c += GridLayoutUtils::GetItemSize(info_, wrapper_, itemIdx).columns - 1;
    }
}

int32_t GridIrregularFiller::FindItemTopRow(int32_t row, int32_t col) const
{
    if (info_->gridMatrix_.at(row).at(col) == 0) {
        return 0;
    }

    while (info_->gridMatrix_.at(row).at(col) < 0) {
        --row;
    }
    return row;
}
} // namespace OHOS::Ace::NG
