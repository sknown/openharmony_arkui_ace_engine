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

#include "core/components_ng/pattern/linear_split/linear_split_layout_algorithm.h"

#include <algorithm>

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/utils/utils.h"
#include "core/components/text/text_theme.h"
#include "core/components_ng/pattern/linear_split/linear_split_layout_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
    constexpr double SPLIT_HEIGHT_RATE = 2.0;
} // namespace

void LinearSplitLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    if (PipelineBase::GetCurrentContext() && PipelineBase::GetCurrentContext()->GetMinPlatformVersion() < 10) {
        MeasureBeforeAPI10(layoutWrapper);
        return;
    }

    const auto& layoutConstraint = layoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
    const auto& minSize = layoutConstraint->minSize;
    const auto& maxSize = layoutConstraint->maxSize;
    OptionalSizeF realSize;
    do {
        // Use idea size first if it is valid.
        realSize.UpdateSizeWithCheck(layoutConstraint->selfIdealSize);
        if (realSize.IsValid()) {
            break;
        }

        if (layoutWrapper->GetLayoutProperty()->GetMeasureType() == MeasureType::MATCH_PARENT) {
            realSize.UpdateIllegalSizeWithCheck(layoutConstraint->parentIdealSize);
        }
    } while (false);

    if (!childrenDragPos_.empty() && childrenDragPos_.size() != layoutWrapper->GetTotalChildCount() + 1) {
        childrenDragPos_.clear();
    }

    // measure self size.
    auto [childTotalSize, childMaxSize] = MeasureChildren(layoutWrapper);
    if (splitType_ == SplitType::ROW_SPLIT) {
        if (!layoutConstraint->selfIdealSize.Width()) {
            float width = std::max(minSize.Width(), childTotalSize.Width());
            if (maxSize.Width() > 0) {
                width = std::min(maxSize.Width(), width);
            }
            realSize.SetWidth(width);
        }
        if (!layoutConstraint->selfIdealSize.Height()) {
            realSize.SetHeight(std::min(maxSize.Height(), childMaxSize.Height()));
        }
        layoutWrapper->GetGeometryNode()->SetFrameSize((realSize.ConvertToSizeT()));
    } else if (splitType_ == SplitType::COLUMN_SPLIT) {
        if (!layoutConstraint->selfIdealSize.Height()) {
            float height = std::min(maxSize.Height(), childTotalSize.Height());
            realSize.SetHeight(height);
        }
        if (!layoutConstraint->selfIdealSize.Width()) {
            realSize.SetWidth(std::min(maxSize.Width(), childMaxSize.Width()));
        }
        layoutWrapper->GetGeometryNode()->SetFrameSize((realSize.ConvertToSizeT()));
    }
}

void LinearSplitLayoutAlgorithm::MeasureBeforeAPI10(LayoutWrapper* layoutWrapper)
{
    const auto& layoutConstraint = layoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
    const auto& minSize = layoutConstraint->minSize;
    const auto& maxSize = layoutConstraint->maxSize;
    const auto& parentIdeaSize = layoutConstraint->parentIdealSize;
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    auto measureType = layoutWrapper->GetLayoutProperty()->GetMeasureType();
    OptionalSizeF realSize;
    do {
        // Use idea size first if it is valid.
        realSize.UpdateSizeWithCheck(layoutConstraint->selfIdealSize);
        if (realSize.IsValid()) {
            break;
        }

        if (measureType == MeasureType::MATCH_PARENT) {
            realSize.UpdateIllegalSizeWithCheck(parentIdeaSize);
        }
    } while (false);

    // Get Max Size for children.
    OptionalSizeF optionalMaxSize;
    optionalMaxSize.UpdateIllegalSizeWithCheck(maxSize);
    auto maxSizeT = optionalMaxSize.ConvertToSizeT();
    MinusPaddingToSize(padding, maxSizeT);

    const auto& childrenWrappers = layoutWrapper->GetAllChildrenWithBuild();

    auto childConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childConstraint.maxSize = layoutConstraint->selfIdealSize.ConvertToSizeT();
    if (childConstraint.maxSize.Height() < 0.0) {
        childConstraint.maxSize.SetHeight(maxSize.Height());
    }
    if (childConstraint.maxSize.Width() < 0.0) {
        childConstraint.maxSize.SetWidth(maxSize.Width());
    }

    float allocatedSize = 0.0f;
    float crossSize = 0.0f;
    // measure normal node.
    for (const auto& child : childrenWrappers) {
        child->Measure(childConstraint);
        allocatedSize += child->GetGeometryNode()->GetMarginFrameSize().Width();
        crossSize += child->GetGeometryNode()->GetMarginFrameSize().Height();
    }

    // measure self size.
    SizeF childTotalSize = { allocatedSize, crossSize };
    AddPaddingToSize(padding, childTotalSize);
    if (splitType_ == SplitType::ROW_SPLIT) {
        if (!layoutConstraint->selfIdealSize.Width()) {
            float width = std::max(minSize.Width(), childTotalSize.Width());
            if (maxSize.Width() > 0) {
                width = std::min(maxSize.Width(), width);
            }
            realSize.SetWidth(width);
        }
        if (!layoutConstraint->selfIdealSize.Height()) {
            realSize.SetHeight(maxSize.Height());
        }
        layoutWrapper->GetGeometryNode()->SetFrameSize((realSize.ConvertToSizeT()));
    } else if (splitType_ == SplitType::COLUMN_SPLIT) {
        if (!layoutConstraint->selfIdealSize.Height()) {
            float height = std::min(maxSize.Height(), childTotalSize.Height());
            realSize.SetHeight(height);
        }
        if (!layoutConstraint->selfIdealSize.Width()) {
            realSize.SetWidth(maxSize.Width());
        }
        layoutWrapper->GetGeometryNode()->SetFrameSize((realSize.ConvertToSizeT()));
    }
}

std::pair<SizeF, SizeF> LinearSplitLayoutAlgorithm::MeasureChildren(LayoutWrapper* layoutWrapper)
{
    const auto& layoutConstraint = layoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
    const auto& maxSize = layoutConstraint->maxSize;
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();

    // Get Max Size for children.
    OptionalSizeF optionalMaxSize;
    optionalMaxSize.UpdateIllegalSizeWithCheck(maxSize);
    auto maxSizeT = optionalMaxSize.ConvertToSizeT();
    MinusPaddingToSize(padding, maxSizeT);

    auto childConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childConstraint.maxSize = layoutConstraint->selfIdealSize.ConvertToSizeT();
    if (childConstraint.maxSize.Height() < 0.0) {
        childConstraint.maxSize.SetHeight(maxSizeT.Height());
    }
    if (childConstraint.maxSize.Width() < 0.0) {
        childConstraint.maxSize.SetWidth(maxSizeT.Width());
    }

    float allocatedSize = 0.0f;
    float crossSize = 0.0f;
    float childMaxWidth = 0.0f;
    float childMaxHeight = 0.0f;
    const auto [startMargin, endMargin] = GetDividerMargin(layoutWrapper);
    int32_t childCount = layoutWrapper->GetTotalChildCount();

    // measure normal node.
    int32_t index = 0;
    for (const auto& child : layoutWrapper->GetAllChildrenWithBuild()) {
        child->Measure(GetChildConstrain(layoutWrapper, childConstraint, index));
        float childWidth = child->GetGeometryNode()->GetMarginFrameSize().Width();
        float childHeight = child->GetGeometryNode()->GetMarginFrameSize().Height();
        allocatedSize += childWidth;
        crossSize += childHeight;
        childMaxWidth = childWidth > childMaxWidth? childWidth: childMaxWidth;
        childMaxHeight = childHeight > childMaxHeight? childHeight: childMaxHeight;
        index++;
    }

    const auto splitHeightFloat = static_cast<float>(DEFAULT_SPLIT_HEIGHT);
    if (splitType_ == SplitType::ROW_SPLIT) {
        allocatedSize += splitHeightFloat * std::max(0.f, static_cast<float>(childCount - 1));
    } else {
        crossSize += (startMargin + splitHeightFloat + endMargin) * std::max(0.f, static_cast<float>(childCount - 1));
    }

    SizeF childTotalSize = { allocatedSize, crossSize };
    AddPaddingToSize(padding, childTotalSize);
    SizeF childMaxSize = { childMaxWidth, childMaxHeight };
    AddPaddingToSize(padding, childMaxSize);
    return { childTotalSize, childMaxSize };
}

LayoutConstraintF LinearSplitLayoutAlgorithm::GetChildConstrain(LayoutWrapper* layoutWrapper,
    LayoutConstraintF childConstrain, int32_t index)
{
    const auto [startMargin, endMargin] = GetDividerMargin(layoutWrapper);
    int32_t childCount = layoutWrapper->GetTotalChildCount();
    auto constrain = childConstrain;
    if (!childrenDragPos_.empty() && (index < childCount)) {
        float childMaxSize =
                childrenDragPos_[index + 1] - childrenDragPos_[index] - static_cast<float>(DEFAULT_SPLIT_HEIGHT);
        if (splitType_ == SplitType::ROW_SPLIT) {
            if (index == childCount - 1) {
                childMaxSize += static_cast<float>(DEFAULT_SPLIT_HEIGHT);
            }
            constrain.selfIdealSize.SetWidth(childMaxSize);
        } else {
            if (index == 0) {
                childMaxSize -= startMargin;
            } else if (index == childCount - 1) {
                childMaxSize = childMaxSize - endMargin + static_cast<float>(DEFAULT_SPLIT_HEIGHT);
            } else {
                childMaxSize -= startMargin + endMargin;
            }
            constrain.selfIdealSize.SetHeight(childMaxSize);
        }
    }
    return constrain;
}

void LinearSplitLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    if (PipelineBase::GetCurrentContext() && PipelineBase::GetCurrentContext()->GetMinPlatformVersion() < 10) {
        LayoutBeforeAPI10(layoutWrapper);
        return;
    }
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    float childTotalWidth = 0.0f;
    float childTotalHeight = 0.0f;
    int32_t childCount = layoutWrapper->GetTotalChildCount();

    childrenConstrains_ = std::vector<float>(childCount, 0.0f);
    for (const auto& item : layoutWrapper->GetAllChildrenWithBuild()) {
        childTotalWidth += item->GetGeometryNode()->GetMarginFrameSize().Width();
        childTotalHeight += item->GetGeometryNode()->GetMarginFrameSize().Height();
    }

    const auto splitHeightFloat = static_cast<float>(DEFAULT_SPLIT_HEIGHT);
    if (splitType_ == SplitType::ROW_SPLIT) {
        childTotalWidth += splitHeightFloat * std::max(0.f, static_cast<float>(childCount - 1));
    } else {
        const auto [startMargin, endMargin] = GetDividerMargin(layoutWrapper);
        childTotalHeight +=
                (startMargin + splitHeightFloat + endMargin) * std::max(0.f, static_cast<float>(childCount - 1));
    }

    auto parentWidth = layoutWrapper->GetGeometryNode()->GetFrameSize().Width() - padding.Width();
    auto parentHeight = layoutWrapper->GetGeometryNode()->GetFrameSize().Height() - padding.Height();

    auto startPointX = (parentWidth - childTotalWidth) / 2 + padding.left.value_or(0.0f);
    auto startPointY = (parentHeight - childTotalHeight) / 2 + padding.top.value_or(0.0f);
    float childOffsetMain = startPointX;
    float childOffsetCross = startPointY;
    if (splitType_ == SplitType::ROW_SPLIT) {
        LayoutRowSplit(layoutWrapper, childOffsetMain, childOffsetCross);
    } else if (splitType_ == SplitType::COLUMN_SPLIT) {
        LayoutColumnSplit(layoutWrapper, childOffsetMain, childOffsetCross);
    }
}

void LinearSplitLayoutAlgorithm::LayoutBeforeAPI10(LayoutWrapper* layoutWrapper)
{
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    int32_t index = 0;
    float childOffsetMain = 0.0f;
    float childOffsetCross = 0.0f;
    float childTotalWidth = 0.0f;
    float childTotalHeight = 0.0f;
    float childTotalOffsetMain = 0.0f;
    float childTotalOffsetCross = 0.0f;
    if (dragSplitOffset_.empty()) {
        dragSplitOffset_ = std::vector<float>(layoutWrapper->GetTotalChildCount() - 1, 0.0);
    }
    for (const auto& item : layoutWrapper->GetAllChildrenWithBuild()) {
        childTotalWidth += item->GetGeometryNode()->GetFrameSize().Width();
        childTotalHeight += item->GetGeometryNode()->GetFrameSize().Height();
        childTotalOffsetMain += item->GetGeometryNode()->GetFrameSize().Width();
        childTotalOffsetCross += item->GetGeometryNode()->GetFrameSize().Height();
        if (index < layoutWrapper->GetTotalChildCount() - 1) {
            if (splitType_ == SplitType::ROW_SPLIT) {
                childTotalOffsetMain += dragSplitOffset_[index] + DEFAULT_SPLIT_HEIGHT;
            } else {
                childTotalOffsetCross += dragSplitOffset_[index] + DEFAULT_SPLIT_HEIGHT;
            }
        }
        index++;
    }
    index = 0;
    auto parentWidth = layoutWrapper->GetGeometryNode()->GetFrameSize().Width() - padding.Width();
    auto parentHeight = layoutWrapper->GetGeometryNode()->GetFrameSize().Height() - padding.Height();

    parentOffset_ = layoutWrapper->GetGeometryNode()->GetFrameOffset();
    auto startPointX = (parentWidth - childTotalWidth) / 2;
    startPointX = startPointX < 0 ? 0.0f : startPointX;
    startPointX += padding.left.value_or(0.0f);
    auto startPointY = (parentHeight - childTotalHeight) / 2;
    startPointY = startPointY < 0 ? 0.0f : startPointY;
    startPointY += padding.top.value_or(0.0f);
    childOffsetMain += startPointX;
    childOffsetCross += startPointY;
    childTotalOffsetMain += startPointX;
    childTotalOffsetCross += startPointY;
    if (splitType_ == SplitType::ROW_SPLIT) {
        splitLength_ = parentHeight;
        if (GreatOrEqual(childTotalOffsetMain, parentWidth)) {
            isOverParent_ = true;
        }
        for (const auto& item : layoutWrapper->GetAllChildrenWithBuild()) {
            if (padding.top.has_value()) {
                childOffsetCross = padding.top.value();
            }
            OffsetF offset = OffsetF(childOffsetMain, childOffsetCross);
            item->GetGeometryNode()->SetMarginFrameOffset(offset);
            childOffsetMain += item->GetGeometryNode()->GetFrameSize().Width();
            if (index < layoutWrapper->GetTotalChildCount() - 1) {
                childOffsetMain += dragSplitOffset_[index];
                childrenOffset_.emplace_back(OffsetF(childOffsetMain, childOffsetCross));
                splitRects_.emplace_back(
                        Rect(childOffsetMain - DEFAULT_SPLIT_HEIGHT, 0, DEFAULT_SPLIT_HEIGHT * 2, parentHeight));
            }
            childOffsetMain += static_cast<float>(DEFAULT_SPLIT_HEIGHT);
            index++;
            item->Layout();
        }
    } else if (splitType_ == SplitType::COLUMN_SPLIT) {
        splitLength_ = parentWidth;
        if (GreatOrEqual(childTotalOffsetCross, parentHeight)) {
            isOverParent_ = true;
        }
        for (const auto& item : layoutWrapper->GetAllChildrenWithBuild()) {
            if (padding.left.has_value()) {
                childOffsetMain = padding.left.value();
            }
            OffsetF offset = OffsetF(childOffsetMain, childOffsetCross);
            item->GetGeometryNode()->SetMarginFrameOffset(offset);
            childOffsetCross += item->GetGeometryNode()->GetFrameSize().Height();
            if (index < layoutWrapper->GetTotalChildCount() - 1) {
                childOffsetCross += dragSplitOffset_[index];
                childrenOffset_.emplace_back(OffsetF(childOffsetMain, childOffsetCross));
                splitRects_.emplace_back(
                        Rect(0, childOffsetCross - DEFAULT_SPLIT_HEIGHT, parentWidth, DEFAULT_SPLIT_HEIGHT * 2));
            }
            childOffsetCross += static_cast<float>(DEFAULT_SPLIT_HEIGHT);
            index++;
            item->Layout();
        }
    }
}

void LinearSplitLayoutAlgorithm::LayoutRowSplit(LayoutWrapper* layoutWrapper, float childOffsetMain,
    float childOffsetCross)
{
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    auto parentHeight = layoutWrapper->GetGeometryNode()->GetFrameSize().Height() - padding.Height();
    int32_t index = 0;
    bool isFirstSetPos = false;
    if (childrenDragPos_.empty()) {
        childrenDragPos_ = std::vector<float>(layoutWrapper->GetTotalChildCount() + 1, 0.0f);
        isFirstSetPos = true;
    }

    splitLength_ = parentHeight;
    for (const auto& item : layoutWrapper->GetAllChildrenWithBuild()) {
        if (padding.top.has_value()) {
            childOffsetCross = padding.top.value();
        }
        if (isFirstSetPos) {
            childrenDragPos_[index] = childOffsetMain;
        } else {
            childOffsetMain = childrenDragPos_[index];
        }
        auto& constraint = item->GetLayoutProperty()->GetCalcLayoutConstraint();
        auto childMargin = item->GetLayoutProperty()->CreateMargin();
        float marginWidth = childMargin.left.value_or(0.f) + childMargin.right.value_or(0.f);
        if (constraint && constraint->minSize.has_value()) {
            childrenConstrains_[index] =
                    item->GetLayoutProperty()->GetLayoutConstraint()->minSize.Width() + marginWidth;
        } else {
            childrenConstrains_[index] = GetLinearSplitChildMinSize(layoutWrapper) + marginWidth;
        }
        item->GetGeometryNode()->SetMarginFrameOffset(OffsetF(childOffsetMain, childOffsetCross));
        childOffsetMain += item->GetGeometryNode()->GetMarginFrameSize().Width();
        if (index < layoutWrapper->GetTotalChildCount() - 1) {
            if (!isFirstSetPos) {
                childOffsetMain = childrenDragPos_[index + 1] - static_cast<float>(DEFAULT_SPLIT_HEIGHT);
            }
            childrenOffset_.emplace_back(childOffsetMain, childOffsetCross);
            splitRects_.emplace_back(childOffsetMain - DEFAULT_SPLIT_HEIGHT, childOffsetCross,
                                     DEFAULT_SPLIT_HEIGHT * SPLIT_HEIGHT_RATE, parentHeight);
        }
        childOffsetMain += static_cast<float>(DEFAULT_SPLIT_HEIGHT);
        index++;
        item->Layout();
    }
    if (isFirstSetPos) {
        childrenDragPos_[index] = childOffsetMain - static_cast<float>(DEFAULT_SPLIT_HEIGHT);
    }
}

void LinearSplitLayoutAlgorithm::LayoutColumnSplit(LayoutWrapper* layoutWrapper, float childOffsetMain,
    float childOffsetCross)
{
    const auto [startMargin, endMargin] = GetDividerMargin(layoutWrapper);
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    auto parentWidth = layoutWrapper->GetGeometryNode()->GetFrameSize().Width() - padding.Width();
    bool isFirstSetPos = false;
    if (!childrenDragPos_.empty() && childrenDragPos_.size() != layoutWrapper->GetTotalChildCount() + 1) {
        childrenDragPos_.clear();
    }
    if (childrenDragPos_.empty()) {
        childrenDragPos_ = std::vector<float>(layoutWrapper->GetTotalChildCount() + 1, 0.0f);
        isFirstSetPos = true;
    }
    splitLength_ = parentWidth;
    int32_t index = 0;
    for (const auto& item : layoutWrapper->GetAllChildrenWithBuild()) {
        if (padding.left.has_value()) {
            childOffsetMain = padding.left.value();
        }
        if (isFirstSetPos) {
            childrenDragPos_[index] = childOffsetCross;
            if (index != 0) {
                childrenDragPos_[index] -= endMargin;
            }
        } else {
            childOffsetCross = childrenDragPos_[index];
            if (index != 0) {
                childOffsetCross += endMargin;
            }
        }

        ColumnSplitChildConstrain(layoutWrapper, item, index);
        item->GetGeometryNode()->SetMarginFrameOffset(OffsetF(childOffsetMain, childOffsetCross));
        childOffsetCross += item->GetGeometryNode()->GetMarginFrameSize().Height();
        if (index < layoutWrapper->GetTotalChildCount() - 1) {
            childOffsetCross += startMargin;
            if (!isFirstSetPos) {
                childOffsetCross = childrenDragPos_[index + 1] - static_cast<float>(DEFAULT_SPLIT_HEIGHT);
            }
            childrenOffset_.emplace_back(childOffsetMain, childOffsetCross);
            splitRects_.emplace_back(childOffsetMain, childOffsetCross - DEFAULT_SPLIT_HEIGHT, parentWidth,
                                     DEFAULT_SPLIT_HEIGHT * SPLIT_HEIGHT_RATE);
        }
        childOffsetCross += static_cast<float>(DEFAULT_SPLIT_HEIGHT);
        childOffsetCross += endMargin;
        index++;
        item->Layout();
    }
    if (isFirstSetPos) {
        childrenDragPos_[index] = childOffsetCross - static_cast<float>(DEFAULT_SPLIT_HEIGHT) - endMargin;
    }
}

void LinearSplitLayoutAlgorithm::ColumnSplitChildConstrain(LayoutWrapper* layoutWrapper,
    const RefPtr<LayoutWrapper>& item, int32_t index)
{
    if (index >= childrenConstrains_.size()) {
        return;
    }
    const auto [startMargin, endMargin] = GetDividerMargin(layoutWrapper);
    auto& constraint = item->GetLayoutProperty()->GetCalcLayoutConstraint();
    auto childMargin = item->GetLayoutProperty()->CreateMargin();
    float marginHeight = childMargin.top.value_or(0.f) + childMargin.bottom.value_or(0.f);
    if (constraint && constraint->minSize.has_value()) {
        childrenConstrains_[index] = item->GetLayoutProperty()->GetLayoutConstraint()->minSize.Height() + marginHeight;
    } else {
        childrenConstrains_[index] = GetLinearSplitChildMinSize(layoutWrapper) + marginHeight;
    }
    if (index == 0) {
        childrenConstrains_[index] += startMargin;
    } else if (index == layoutWrapper->GetTotalChildCount()) {
        childrenConstrains_[index] += endMargin;
    } else {
        childrenConstrains_[index] += startMargin + endMargin;
    }
}

std::pair<float, float> LinearSplitLayoutAlgorithm::GetDividerMargin(LayoutWrapper* layoutWrapper)
{
    auto listLayoutProperty = AceType::DynamicCast<LinearSplitLayoutProperty>(layoutWrapper->GetLayoutProperty());
    ItemDivider divider;
    if (listLayoutProperty->HasDivider()) {
        divider = listLayoutProperty->GetDivider().value();
    }
    auto startMargin = static_cast<float>(divider.startMargin.ConvertToPx());
    auto endMargin = static_cast<float>(divider.endMargin.ConvertToPx());
    return {startMargin, endMargin};
}

float LinearSplitLayoutAlgorithm::GetLinearSplitChildMinSize(LayoutWrapper* layoutWrapper)
{
    constexpr float DEFAULT_MIN_CHILD_SIZE = 20.f;
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, DEFAULT_MIN_CHILD_SIZE);
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_RETURN(pipeline, DEFAULT_MIN_CHILD_SIZE);
    auto theme = pipeline->GetTheme<TextTheme>();
    CHECK_NULL_RETURN(theme, DEFAULT_MIN_CHILD_SIZE);
    auto size = static_cast<float>(theme->GetLinearSplitChildMinSize());
    return size;
}

} // namespace OHOS::Ace::NG