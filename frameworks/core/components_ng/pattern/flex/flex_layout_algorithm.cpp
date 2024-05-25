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

#include "core/components_ng/pattern/flex/flex_layout_algorithm.h"

#include <algorithm>
#include <functional>
#include <iterator>

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/log/ace_trace.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/common/container.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/blank/blank_layout_property.h"
#include "core/components_ng/pattern/flex/flex_layout_property.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/components_ng/pattern/navigation/navigation_group_node.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

namespace {
/**
 * Get the main axis direction based on direction.
 */
FlexDirection FlipAxis(FlexDirection direction)
{
    if (direction == FlexDirection::ROW || direction == FlexDirection::ROW_REVERSE) {
        return FlexDirection::COLUMN;
    }
    return FlexDirection::ROW;
}

/**
 * Determine whether to start the layout from the upper left corner
 */
bool IsStartTopLeft(FlexDirection direction, TextDirection textDirection)
{
    switch (direction) {
        case FlexDirection::ROW:
            return textDirection == TextDirection::LTR;
        case FlexDirection::ROW_REVERSE:
            return textDirection == TextDirection::RTL;
        case FlexDirection::COLUMN:
            return true;
        case FlexDirection::COLUMN_REVERSE:
            return false;
        default:
            return true;
    }
}

float GetCrossAxisSizeHelper(const SizeF& size, FlexDirection direction)
{
    if (direction == FlexDirection::ROW || direction == FlexDirection::ROW_REVERSE) {
        return size.Height();
    }
    return size.Width();
}

float GetMainAxisSizeHelper(const SizeF& size, FlexDirection direction)
{
    if (direction == FlexDirection::ROW || direction == FlexDirection::ROW_REVERSE) {
        return size.Width();
    }
    return size.Height();
}

OptionalSizeF GetCalcSizeHelper(float mainAxisSize, float crossAxisSize, FlexDirection direction)
{
    OptionalSizeF size;
    if (direction == FlexDirection::ROW || direction == FlexDirection::ROW_REVERSE) {
        size.SetWidth(mainAxisSize);
        size.SetHeight(crossAxisSize);
    } else {
        size.SetHeight(mainAxisSize);
        size.SetWidth(crossAxisSize);
    }
    return size;
}

bool IsHorizontal(FlexDirection direction)
{
    return direction == FlexDirection::ROW || direction == FlexDirection::ROW_REVERSE;
}

void UpdateChildLayoutConstrainByFlexBasis(
    FlexDirection direction, const RefPtr<LayoutWrapper>& child, LayoutConstraintF& layoutConstraint)
{
    const auto& flexItemProperty = child->GetLayoutProperty()->GetFlexItemProperty();
    CHECK_NULL_VOID(flexItemProperty);
    const auto& flexBasis = flexItemProperty->GetFlexBasis();
    CHECK_NULL_VOID(flexBasis);
    if (flexBasis->Unit() == DimensionUnit::AUTO || !flexBasis->IsValid()) {
        return;
    }
    if (child->GetLayoutProperty()->GetCalcLayoutConstraint()) {
        auto selfIdealSize = child->GetLayoutProperty()->GetCalcLayoutConstraint()->selfIdealSize;
        if (child->GetHostTag() == V2::BLANK_ETS_TAG && selfIdealSize.has_value()) {
            if (IsHorizontal(direction) && selfIdealSize->Width().has_value() &&
                selfIdealSize->Width()->GetDimension().ConvertToPx() > flexBasis->ConvertToPx()) {
                return;
            } else if (!IsHorizontal(direction) && selfIdealSize->Height().has_value() &&
                       selfIdealSize->Height()->GetDimension().ConvertToPx() > flexBasis->ConvertToPx()) {
                return;
            }
        }
    }
    if (direction == FlexDirection::ROW || direction == FlexDirection::ROW_REVERSE) {
        layoutConstraint.selfIdealSize.SetWidth(flexBasis->ConvertToPx());
    } else {
        layoutConstraint.selfIdealSize.SetHeight(flexBasis->ConvertToPx());
    }
}

float GetMainAxisMargin(const RefPtr<LayoutWrapper>& child, FlexDirection direction)
{
    float childMainAxisMargin = 0.0f;
    if (child && child->GetGeometryNode() && child->GetGeometryNode()->GetMargin()) {
        childMainAxisMargin = GetMainAxisSizeHelper(child->GetGeometryNode()->GetMargin()->Size(), direction);
    }
    return childMainAxisMargin;
}

} // namespace

float FlexLayoutAlgorithm::GetChildMainAxisSize(const RefPtr<LayoutWrapper>& layoutWrapper) const
{
    float size = 0.0f;
    CHECK_NULL_RETURN(layoutWrapper, size);
    return GetMainAxisSizeHelper(layoutWrapper->GetGeometryNode()->GetMarginFrameSize(), direction_);
}

float FlexLayoutAlgorithm::GetChildCrossAxisSize(const RefPtr<LayoutWrapper>& layoutWrapper) const
{
    CHECK_NULL_RETURN(layoutWrapper, 0.0f);
    return GetCrossAxisSizeHelper(layoutWrapper->GetGeometryNode()->GetMarginFrameSize(), direction_);
}

float FlexLayoutAlgorithm::GetSelfCrossAxisSize(const RefPtr<LayoutWrapper>& layoutWrapper) const
{
    CHECK_NULL_RETURN(layoutWrapper, 0.0f);
    return GetCrossAxisSizeHelper(layoutWrapper->GetGeometryNode()->GetFrameSize(), direction_);
}

void FlexLayoutAlgorithm::CheckSizeValidity(const RefPtr<LayoutWrapper>& layoutWrapper)
{
    if (layoutWrapper && layoutWrapper->GetHostNode() &&
        layoutWrapper->GetHostNode()->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE) ==
            VisibleType::GONE) {
        return;
    }
    ++validSizeCount_;
}

/**
 * Check and record baseline distance.
 */
void FlexLayoutAlgorithm::CheckBaselineProperties(const RefPtr<LayoutWrapper>& layoutWrapper)
{
    if (crossAxisAlign_ != FlexAlign::BASELINE && !childrenHasAlignSelfBaseLine_) {
        return;
    }
    float distance = layoutWrapper->GetBaselineDistance();
    baselineProperties_.maxBaselineDistance = std::max(baselineProperties_.maxBaselineDistance, distance);
    baselineProperties_.maxDistanceAboveBaseline = std::max(baselineProperties_.maxDistanceAboveBaseline, distance);
    baselineProperties_.maxDistanceBelowBaseline =
        std::max(baselineProperties_.maxDistanceBelowBaseline, GetSelfCrossAxisSize(layoutWrapper) - distance);
    if (crossAxisAlign_ == FlexAlign::BASELINE) {
        crossAxisSize_ = baselineProperties_.maxDistanceAboveBaseline + baselineProperties_.maxDistanceBelowBaseline;
    }
}

/**
 * Initialize the FlexLayoutAlgorithm property.
 */
void FlexLayoutAlgorithm::InitFlexProperties(LayoutWrapper* layoutWrapper)
{
    mainAxisSize_ = 0.0f;
    crossAxisSize_ = 0.0f;
    allocatedSize_ = 0.0f;
    selfIdealCrossAxisSize_ = -1.0f;
    validSizeCount_ = 0;
    realSize_.Reset();
    isInfiniteLayout_ = false;
    auto layoutProperty = AceType::DynamicCast<FlexLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    space_ = static_cast<float>(layoutProperty->GetSpaceValue({}).ConvertToPx());
    direction_ = layoutProperty->GetFlexDirection().value_or(FlexDirection::ROW);
    mainAxisAlign_ = layoutProperty->GetMainAxisAlignValue(FlexAlign::FLEX_START);
    secondaryMeasureList_.clear();
    crossAxisAlign_ =
        layoutProperty->GetCrossAxisAlignValue(isLinearLayoutFeature_ ? FlexAlign::CENTER : FlexAlign::FLEX_START);
    baselineProperties_.Reset();
    textDir_ = layoutProperty->GetLayoutDirection();
    if (textDir_ == TextDirection::AUTO) {
        textDir_ = AceApplicationInfo::GetInstance().IsRightToLeft() ? TextDirection::RTL : TextDirection::LTR;
    }
    /**
     * FlexLayoutAlgorithm, as the parent class, should not handle the special logic of the subclass
     * LinearLayout.
     */
    if (isLinearLayoutFeature_) {
        bool isVertical = DynamicCast<LinearLayoutProperty>(layoutWrapper->GetLayoutProperty())->IsVertical();
        direction_ = isVertical ? FlexDirection::COLUMN : FlexDirection::ROW;
    }
}

void FlexLayoutAlgorithm::TravelChildrenFlexProps(LayoutWrapper* layoutWrapper, const SizeF& realSize)
{
    maxDisplayPriority_ = 0;
    totalFlexWeight_ = 0.0f;
    outOfLayoutChildren_.clear();
    magicNodes_.clear();
    magicNodeWeights_.clear();
    childrenHasAlignSelfBaseLine_ = false;
    const auto& layoutProperty = layoutWrapper->GetLayoutProperty();
    const auto& children = layoutWrapper->GetAllChildrenWithBuild();
    auto childLayoutConstraint = layoutProperty->CreateChildConstraint();
    for (const auto& child : children) {
        if (child->IsOutOfLayout()) {
            outOfLayoutChildren_.emplace_back(child);
            continue;
        }
        const auto& childLayoutProperty = child->GetLayoutProperty();
        const auto& childMagicItemProperty = childLayoutProperty->GetMagicItemProperty();
        const auto& childFlexItemProperty = childLayoutProperty->GetFlexItemProperty();
        MagicLayoutNode node;
        node.layoutWrapper = child;
        node.layoutConstraint = childLayoutConstraint;

        bool childGone =
            child && child->GetHostNode() && child->GetHostNode()->GetLayoutProperty() &&
            child->GetHostNode()->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE) == VisibleType::GONE;
        int32_t childDisplayPriority = 1;
        float childLayoutWeight = 0.0f;
        if (!childGone) {
            childLayoutWeight = childMagicItemProperty.GetLayoutWeight().value_or(0.0f);
            if (childFlexItemProperty) {
                childDisplayPriority = childFlexItemProperty->GetDisplayIndex().value_or(1);
                if (!childrenHasAlignSelfBaseLine_ &&
                    childFlexItemProperty->GetAlignSelf().value_or(FlexAlign::FLEX_START) == FlexAlign::BASELINE) {
                    childrenHasAlignSelfBaseLine_ = true;
                }
            }
        }

        auto iter = magicNodes_.find(childDisplayPriority);
        if (iter == magicNodes_.end()) {
            magicNodes_.insert(
                std::map<int32_t, std::list<MagicLayoutNode>>::value_type(childDisplayPriority, { node }));
            if (GreatNotEqual(childLayoutWeight, 0.0f)) {
                magicNodeWeights_.insert(std::map<int32_t, float>::value_type(childDisplayPriority, childLayoutWeight));
            }
        } else {
            iter->second.emplace_back(node);
            if (GreatNotEqual(childLayoutWeight, 0.0f)) {
                magicNodeWeights_[childDisplayPriority] += childLayoutWeight;
            }
        }
        if (!childGone) {
            totalFlexWeight_ += GreatNotEqual(childLayoutWeight, 0.0f) ? childLayoutWeight : 0.0f;
            maxDisplayPriority_ = std::max(childDisplayPriority, maxDisplayPriority_);
        }
    }
}

void FlexLayoutAlgorithm::UpdateAllocatedSize(const RefPtr<LayoutWrapper>& childLayoutWrapper, float& crossAxisSize)
{
    float mainAxisSize = GetChildMainAxisSize(childLayoutWrapper);
    if (GreaterOrEqualToInfinity(mainAxisSize)) {
        mainAxisSize = 0.0f;
    }
    crossAxisSize = std::max(crossAxisSize, GetChildCrossAxisSize(childLayoutWrapper));
    allocatedSize_ += mainAxisSize;
    allocatedSize_ += space_;
}

void FlexLayoutAlgorithm::MeasureOutOfLayoutChildren(LayoutWrapper* layoutWrapper)
{
    const auto& layoutConstrain = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    for (const auto& child : outOfLayoutChildren_) {
        child->Measure(layoutConstrain);
    }
}

void FlexLayoutAlgorithm::MeasureAndCleanMagicNodes(
    LayoutWrapper* containerLayoutWrapper, FlexItemProperties& flexItemProperties)
{
    if (GreatNotEqual(totalFlexWeight_, 0.0f)) {
        auto newTotalFlexWeight = totalFlexWeight_;
        /**
         * The child elements with layoutWeight=0 are measured first.
         * Then, measure the sub elements of layoutWeight>1 based on the remaining space.
         * If the total main axis size of the element is larger than the main axis size of Flex, the lower priority
         * element will be deleted.
         */
        auto firstLoopIter = magicNodes_.rbegin();
        auto loopIter = firstLoopIter;
        bool outOfDisplay = false;
        while (loopIter != magicNodes_.rend()) {
            auto& childList = loopIter->second;
            float crossAxisSize = crossAxisSize_;
            for (auto& child : childList) {
                if (!outOfDisplay) {
                    const auto& childLayoutWrapper = child.layoutWrapper;
                    float childLayoutWeight = 0.0f;
                    const auto& childMagicItemProperty =
                        childLayoutWrapper->GetLayoutProperty()->GetMagicItemProperty();
                    childLayoutWeight = childMagicItemProperty.GetLayoutWeight().value_or(0.0f);
                    if (LessOrEqual(childLayoutWeight, 0.0f)) {
                        if (child.layoutWrapper && child.layoutWrapper->GetHostNode() &&
                            child.layoutWrapper->GetHostNode()->GetLayoutProperty() &&
                            child.layoutWrapper->GetHostNode()->GetLayoutProperty()->GetVisibilityValue(
                                VisibleType::VISIBLE) == VisibleType::GONE) {
                            continue;
                        }
                        childLayoutWrapper->Measure(child.layoutConstraint);
                        UpdateAllocatedSize(childLayoutWrapper, crossAxisSize);
                        CheckSizeValidity(childLayoutWrapper);
                        CheckBaselineProperties(childLayoutWrapper);
                    } else {
                        allocatedSize_ += space_;
                    }
                } else {
                    child.layoutWrapper->SetActive(false);
                    child.layoutWrapper->GetGeometryNode()->SetFrameSize(SizeF());
                }
            }
            if (outOfDisplay) {
                ++loopIter;
                continue;
            }
            /**
             * The main axis size of the element with layoutWeight of 0 is larger than the Flex main axis size
             */
            if (allocatedSize_ - space_ > mainAxisSize_ && magicNodes_.size() > 1) {
                for (const auto& child : childList) {
                    allocatedSize_ -= GetChildMainAxisSize(child.layoutWrapper);
                    allocatedSize_ -= space_;
                    // TODO: reset size validity and baseline properties.
                    child.layoutWrapper->SetActive(false);
                    child.layoutWrapper->GetGeometryNode()->SetFrameSize(SizeF());
                }
                outOfDisplay = true;
                firstLoopIter = loopIter++;
            } else {
                crossAxisSize_ = crossAxisSize;
                firstLoopIter = ++loopIter;
            }
        }
        allocatedSize_ -= space_;
        auto remainedMainAxisSize = mainAxisSize_ - allocatedSize_;
        auto spacePerWeight = remainedMainAxisSize / newTotalFlexWeight;
        auto secondIterLoop = magicNodes_.rbegin();
        while (secondIterLoop != firstLoopIter) {
            auto& childList = secondIterLoop->second;
            bool isExceed = false;
            for (auto& child : childList) {
                auto childLayoutWrapper = child.layoutWrapper;
                auto& childConstraint = child.layoutConstraint;
                float childLayoutWeight = 0.0f;
                const auto& childMagicItemProperty = childLayoutWrapper->GetLayoutProperty()->GetMagicItemProperty();
                childLayoutWeight = childMagicItemProperty.GetLayoutWeight().value_or(0.0f);
                if (LessOrEqual(childLayoutWeight, 0.0)) {
                    continue;
                }
                float childCalcSize = std::max(spacePerWeight * childLayoutWeight, 0.0f);
                if (GetMainAxisSizeHelper(childConstraint.minSize, direction_) > childCalcSize) {
                    isExceed = true;
                }
                UpdateLayoutConstraintOnMainAxis(childConstraint, childCalcSize);
            }
            if (isExceed) {
                if (magicNodes_.size() <= 1) {
                    break;
                }
                isExceed = true;
                auto& lowPriorityChildList = magicNodes_.begin()->second;
                for (const auto& child : lowPriorityChildList) {
                    allocatedSize_ -= GetChildMainAxisSize(child.layoutWrapper);
                    allocatedSize_ -= space_;
                    child.layoutWrapper->SetActive(false);
                    child.layoutWrapper->GetGeometryNode()->SetFrameSize(SizeF());
                }
                newTotalFlexWeight -= magicNodeWeights_[magicNodes_.begin()->first];
                remainedMainAxisSize = mainAxisSize_ - allocatedSize_;
                spacePerWeight = remainedMainAxisSize / newTotalFlexWeight;
                isExceed = false;
                magicNodes_.erase(magicNodes_.begin());
                secondIterLoop = magicNodes_.rbegin();
            } else {
                secondIterLoop++;
            }
        }
        auto iter = magicNodes_.rbegin();
        while (iter != magicNodes_.rend()) {
            auto& childList = iter->second;
            for (auto& child : childList) {
                auto childLayoutWrapper = child.layoutWrapper;
                if (!childLayoutWrapper->IsActive()) {
                    continue;
                }
                float childLayoutWeight = 0.0f;
                const auto& childMagicItemProperty = childLayoutWrapper->GetLayoutProperty()->GetMagicItemProperty();
                childLayoutWeight = childMagicItemProperty.GetLayoutWeight().value_or(0.0f);
                secondaryMeasureList_.emplace_back(child);
                if (LessOrEqual(childLayoutWeight, 0.0)) {
                    continue;
                }
                childLayoutWrapper->Measure(child.layoutConstraint);
                UpdateAllocatedSize(childLayoutWrapper, crossAxisSize_);
                CheckSizeValidity(childLayoutWrapper);
                CheckBaselineProperties(childLayoutWrapper);
            }
            iter++;
        }
    } else if (GreatNotEqual(maxDisplayPriority_, 1) && !isInfiniteLayout_) {
        bool outOfDisplay = false;
        auto iter = magicNodes_.rbegin();
        while (iter != magicNodes_.rend()) {
            auto childList = iter->second;
            if (outOfDisplay) {
                for (auto& child : childList) {
                    child.layoutWrapper->SetActive(false);
                    child.layoutWrapper->GetGeometryNode()->SetFrameSize(SizeF());
                }
                ++iter;
                continue;
            }
            float crossAxisSize = crossAxisSize_;
            for (auto& child : childList) {
                const auto& childLayoutWrapper = child.layoutWrapper;
                auto childLayoutConstraint = child.layoutConstraint;
                UpdateChildLayoutConstrainByFlexBasis(direction_, childLayoutWrapper, childLayoutConstraint);
                childLayoutWrapper->Measure(childLayoutConstraint);
                UpdateAllocatedSize(childLayoutWrapper, crossAxisSize);
                CheckSizeValidity(childLayoutWrapper);
                CheckBaselineProperties(childLayoutWrapper);
                const auto& flexItemProperty = childLayoutWrapper->GetLayoutProperty()->GetFlexItemProperty();
                if (flexItemProperty && GreatNotEqual(flexItemProperty->GetFlexGrow().value_or(0.0f), 0.0f)) {
                    flexItemProperties.totalGrow += flexItemProperty->GetFlexGrow().value_or(0.0f);
                }
                secondaryMeasureList_.emplace_back(child);
            }
            if (!LessOrEqual(allocatedSize_ - space_, mainAxisSize_)) {
                outOfDisplay = true;
                for (auto& child : childList) {
                    allocatedSize_ -= GetChildMainAxisSize(child.layoutWrapper);
                    allocatedSize_ -= space_;
                    child.layoutWrapper->SetActive(false);
                    --validSizeCount_;
                    child.layoutWrapper->GetGeometryNode()->SetFrameSize(SizeF());
                    const auto& flexItemProperty = child.layoutWrapper->GetLayoutProperty()->GetFlexItemProperty();
                    if (flexItemProperty && GreatNotEqual(flexItemProperty->GetFlexGrow().value_or(0.0f), 0.0f)) {
                        flexItemProperties.totalGrow -= flexItemProperty->GetFlexGrow().value_or(0.0f);
                    }
                    secondaryMeasureList_.pop_back();
                }
            } else {
                crossAxisSize_ = crossAxisSize;
            }
            ++iter;
        }
    } else {
        auto iter = magicNodes_.rbegin();
        while (iter != magicNodes_.rend()) {
            auto childList = iter->second;
            for (auto& child : childList) {
                if (HandleBlankFirstTimeMeasure(child, flexItemProperties)) {
                    continue;
                }
                const auto& childLayoutWrapper = child.layoutWrapper;
                UpdateChildLayoutConstrainByFlexBasis(direction_, childLayoutWrapper, child.layoutConstraint);
                childLayoutWrapper->Measure(child.layoutConstraint);
                if (child.layoutWrapper && child.layoutWrapper->GetHostNode() &&
                    child.layoutWrapper->GetHostNode()->GetLayoutProperty() &&
                    child.layoutWrapper->GetHostNode()->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE) ==
                        VisibleType::GONE) {
                    continue;
                }
                UpdateAllocatedSize(childLayoutWrapper, crossAxisSize_);
                CheckSizeValidity(childLayoutWrapper);
                CheckBaselineProperties(childLayoutWrapper);
                if (!isInfiniteLayout_ || GreatNotEqual(MainAxisMinValue(containerLayoutWrapper), 0.0f)) {
                    UpdateFlexProperties(flexItemProperties, childLayoutWrapper);
                }
                secondaryMeasureList_.emplace_back(child);
            }
            ++iter;
        }
        allocatedSize_ -= space_;
    }
}

bool FlexLayoutAlgorithm::HandleBlankFirstTimeMeasure(
    const MagicLayoutNode& child, FlexItemProperties& flexItemProperties)
{
    const auto& childLayoutWrapper = child.layoutWrapper;
    if (!(childLayoutWrapper->GetHostTag() == V2::BLANK_ETS_TAG &&
            Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN))) {
        return false;
    }

    // if constainer is self adaptive, secondaryMeasure won't happen, blank can call Measure directly
    if (selfAdaptive_ || isInfiniteLayout_) {
        childLayoutWrapper->Measure(child.layoutConstraint);
        UpdateAllocatedSize(childLayoutWrapper, crossAxisSize_);
        CheckSizeValidity(childLayoutWrapper);
        if (!isInfiniteLayout_) {
            UpdateFlexProperties(flexItemProperties, childLayoutWrapper);
        }
        secondaryMeasureList_.emplace_back(child);
        return true;
    }
    // to make blank components splilt remain space(not selfAdaptive)
    // min size should not participate in the first measure of blank
    auto mainAxisSize = 0.0f;
    auto crossAxisSize = 0.0f;
    auto blankLayoutProperty = childLayoutWrapper->GetLayoutProperty();
    childLayoutWrapper->GetHostNode()->GetPattern()->BeforeCreateLayoutWrapper();
    if (blankLayoutProperty) {
        const auto& calcConstraint = blankLayoutProperty->GetCalcLayoutConstraint();
        if (calcConstraint && calcConstraint->selfIdealSize.has_value()) {
            auto size = ConvertToSize(calcConstraint->selfIdealSize.value(), child.layoutConstraint.scaleProperty,
                child.layoutConstraint.percentReference);
            mainAxisSize = std::max(IsHorizontal(direction_) ? size.Width() : size.Height(), 0.0f);
            crossAxisSize = std::max(IsHorizontal(direction_) ? size.Height() : size.Width(), 0.0f);
        }
    }
    childLayoutWrapper->GetGeometryNode()->SetFrameSize(
        IsHorizontal(direction_) ? SizeF(mainAxisSize, crossAxisSize) : SizeF(crossAxisSize, mainAxisSize));
    secondaryMeasureList_.emplace_back(child);
    UpdateAllocatedSize(childLayoutWrapper, crossAxisSize_);
    CheckSizeValidity(childLayoutWrapper);
    UpdateFlexProperties(flexItemProperties, childLayoutWrapper);
    return true;
}

void FlexLayoutAlgorithm::UpdateFlexProperties(
    FlexItemProperties& flexItemProperties, const RefPtr<LayoutWrapper>& layoutWrapper)
{
    const auto& flexItemProperty = layoutWrapper->GetLayoutProperty()->GetFlexItemProperty();
    float flexShrink = isLinearLayoutFeature_ ? 0.0f : 1.0f;
    float flexGrow = 0.0f;
    if (flexItemProperty) {
        flexShrink = flexItemProperty->GetFlexShrink().value_or(flexShrink);
        flexGrow = flexItemProperty->GetFlexGrow().value_or(flexGrow);
    }
    flexItemProperties.totalGrow += flexGrow;
    flexItemProperties.totalShrink +=
        (flexShrink * (GetChildMainAxisSize(layoutWrapper) - GetMainAxisMargin(layoutWrapper, direction_)));
}

void FlexLayoutAlgorithm::SecondaryMeasureByProperty(
    FlexItemProperties& flexItemProperties, LayoutWrapper* layoutWrapper)
{
    float remainSpace = mainAxisSize_ - allocatedSize_;
    float spacePerFlex = 0;
    float allocatedFlexSpace = 0;
    std::function<float(const RefPtr<LayoutWrapper>&)> getFlex;
    RefPtr<LayoutWrapper> lastChild;
    /**
     * get the real cross axis size.
     */
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    auto paddingLeft = padding.left.value_or(0.0f);
    auto paddingRight = padding.right.value_or(0.0f);
    auto paddingTop = padding.top.value_or(0.0f);
    auto paddingBottom = padding.bottom.value_or(0.0f);
    auto crossAxisSize = crossAxisSize_;
    if (NonNegative(selfIdealCrossAxisSize_)) {
        if (direction_ == FlexDirection::ROW || direction_ == FlexDirection::ROW_REVERSE) {
            crossAxisSize = selfIdealCrossAxisSize_ - paddingTop - paddingBottom;
        } else {
            crossAxisSize = selfIdealCrossAxisSize_ - paddingLeft - paddingRight;
        }
    }
    if (Negative(crossAxisSize)) {
        crossAxisSize = 0.0f;
    }
    // calculate child
    auto iter = secondaryMeasureList_.rbegin();
    bool needSecondMeasure = true;
    float reserveMainAxisSize = 0.0f;
    while (needSecondMeasure) {
        needSecondMeasure = false;
        // when a child's flexSize equal 0, allocatedSize need to minus its MainAxisSize
        allocatedSize_ -= reserveMainAxisSize;
        reserveMainAxisSize = 0.0f;
        remainSpace = mainAxisSize_ - allocatedSize_;

        iter = secondaryMeasureList_.rbegin();
        while (iter != secondaryMeasureList_.rend()) {
            if (!(*iter).layoutWrapper->IsActive()) {
                remainSpace += space_;
            }
            ++iter;
        }
        CheckIsGrowOrShrink(getFlex, remainSpace, spacePerFlex, flexItemProperties, lastChild);
        iter = secondaryMeasureList_.rbegin();
        while (iter != secondaryMeasureList_.rend()) {
            auto& child = *iter;
            auto childLayoutWrapper = child.layoutWrapper;
            if (!childLayoutWrapper) {
                continue;
            }
            if (GetSelfAlign(childLayoutWrapper) == FlexAlign::STRETCH) {
                UpdateLayoutConstraintOnCrossAxis(child.layoutConstraint, crossAxisSize);
                child.needSecondMeasure = true;
            }
            if (LessOrEqual(totalFlexWeight_, 0.0f) &&
                (!isInfiniteLayout_ || GreatNotEqual(MainAxisMinValue(layoutWrapper), 0.0f) ||
                    (childLayoutWrapper->GetHostTag() == V2::BLANK_ETS_TAG && !selfAdaptive_ &&
                        Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)))) {
                if (child.needKeepMinCalcSize) {
                    ++iter;
                    continue;
                }
                float childMainAxisMargin = GetMainAxisMargin(childLayoutWrapper, direction_);
                float itemFlex = getFlex(child.layoutWrapper);
                float flexSize =
                    (child.layoutWrapper == lastChild) ? (remainSpace - allocatedFlexSpace)
                    : GreatOrEqual(remainSpace, 0.0f) || GreatNotEqual(maxDisplayPriority_, 1)
                        ? spacePerFlex * itemFlex
                        : spacePerFlex * itemFlex * (GetChildMainAxisSize(childLayoutWrapper) - childMainAxisMargin);
                if (!NearZero(flexSize) && childLayoutWrapper->IsActive()) {
                    flexSize += GetChildMainAxisSize(childLayoutWrapper);
                    child.needSecondMeasure = true;
                    CheckBlankAndKeepMin(childLayoutWrapper, flexSize);
                    if (LessOrEqual(flexSize, 0.0f)) {
                        child.layoutWrapper->SetActive(false);
                        flexItemProperties.totalShrink -=
                            itemFlex * (GetChildMainAxisSize(childLayoutWrapper) - childMainAxisMargin);
                        reserveMainAxisSize += GetChildMainAxisSize(childLayoutWrapper);
                        needSecondMeasure = true;
                        UpdateLayoutConstraintOnMainAxis(child.layoutConstraint, 0.0f);
                        break;
                    }
                    if (IsKeepMinSize(childLayoutWrapper, flexSize)) {
                        needSecondMeasure = true;
                        auto shrinkSize = itemFlex * (GetChildMainAxisSize(childLayoutWrapper) - childMainAxisMargin);
                        reserveMainAxisSize -= (flexSize - shrinkSize);
                        child.needKeepMinCalcSize = true;
                        flexItemProperties.totalShrink -= shrinkSize;
                    }
                    UpdateLayoutConstraintOnMainAxis(child.layoutConstraint, flexSize);
                } else if (childLayoutWrapper->GetHostTag() == V2::BLANK_ETS_TAG && NearZero(flexSize) &&
                           childLayoutWrapper->IsActive()) {
                    child.needSecondMeasure = true;
                }
            }
            ++iter;
        }
    }

    // child need to second show
    int32_t childMeasureCount = 0;
    iter = secondaryMeasureList_.rbegin();
    while (iter != secondaryMeasureList_.rend()) {
        auto child = *iter;
        auto childLayoutWrapper = child.layoutWrapper;
        if (!child.needSecondMeasure || !childLayoutWrapper->IsActive()) {
            ++iter;
            continue;
        }
        childLayoutWrapper->Measure(child.layoutConstraint);
        crossAxisSize_ = std::max(crossAxisSize_, GetChildCrossAxisSize(childLayoutWrapper));
        CheckBaselineProperties(child.layoutWrapper);
        ++iter;
        ++childMeasureCount;
    }
    // if child has secondary measure, calculate crossAxis again
    if (childMeasureCount) {
        float chilMaxHeight = -1.0f;
        iter = secondaryMeasureList_.rbegin();
        while (iter != secondaryMeasureList_.rend()) {
            auto child = *iter;
            auto childLayoutWrapper = child.layoutWrapper;
            chilMaxHeight = std::max(GetChildCrossAxisSize(childLayoutWrapper), chilMaxHeight);
            ++iter;
        }
        if (GreatNotEqual(chilMaxHeight, 0.0f)) {
            crossAxisSize_ = chilMaxHeight;
        }
    }
}

void FlexLayoutAlgorithm::CheckIsGrowOrShrink(std::function<float(const RefPtr<LayoutWrapper>&)>& getFlex,
    float remainSpace, float& spacePerFlex, FlexItemProperties& flexItemProperties, RefPtr<LayoutWrapper>& lastChild)
{
    if (GreatOrEqual(remainSpace, 0.0f) || GreatNotEqual(maxDisplayPriority_, 1)) {
        getFlex = [](const RefPtr<LayoutWrapper>& item) -> float {
            const auto& flexItemProperty = item->GetLayoutProperty()->GetFlexItemProperty();
            float ret = 0.0f;
            if (flexItemProperty) {
                ret = flexItemProperty->GetFlexGrow().value_or(ret);
                /**
                 * handle non positive flex grow.
                 */
                if (NonPositive(ret)) {
                    ret = 0.0f;
                }
            }
            return ret;
        };
        spacePerFlex = NearZero(flexItemProperties.totalGrow) ? 0.0f : remainSpace / flexItemProperties.totalGrow;
        lastChild = flexItemProperties.lastGrowChild;
    } else {
        getFlex = [isLinearLayoutFeature = isLinearLayoutFeature_](const RefPtr<LayoutWrapper>& item) -> float {
            const auto& flexItemProperty = item->GetLayoutProperty()->GetFlexItemProperty();
            float ret = isLinearLayoutFeature ? 0.0f : 1.0f;
            if (flexItemProperty) {
                ret = flexItemProperty->GetFlexShrink().value_or(ret);
                /**
                 * handle non positive flex shrink.
                 */
                if (NonPositive(ret)) {
                    ret = 0.0f;
                }
            }
            return ret;
        };
        spacePerFlex = NearZero(flexItemProperties.totalShrink) ? 0.0f : remainSpace / flexItemProperties.totalShrink;
        lastChild = flexItemProperties.lastShrinkChild;
    }
}

void FlexLayoutAlgorithm::CheckBlankAndKeepMin(const RefPtr<LayoutWrapper>& childLayoutWrapper, float& flexSize)
{
    auto child = childLayoutWrapper->GetHostNode();
    if (!child) {
        return;
    }
    if (child->GetTag() != V2::BLANK_ETS_TAG) {
        return;
    }
    auto blankProperty = child->GetLayoutProperty<BlankLayoutProperty>();
    CHECK_NULL_VOID(blankProperty);
    auto blankMin = blankProperty->GetMinSize();
    if (GreatOrEqual(blankMin->ConvertToPx(), flexSize)) {
        flexSize = blankMin->ConvertToPx();
    }
}

bool FlexLayoutAlgorithm::IsKeepMinSize(const RefPtr<LayoutWrapper>& childLayoutWrapper, float& flexSize)
{
    auto child = childLayoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(child, false);
    auto minSize = MainAxisMinValue(AceType::RawPtr(childLayoutWrapper));
    if (GreatOrEqual(minSize, flexSize)) {
        flexSize = minSize;
        return true;
    }
    return false;
}

void FlexLayoutAlgorithm::UpdateLayoutConstraintOnMainAxis(LayoutConstraintF& layoutConstraint, float size)
{
    if (direction_ == FlexDirection::ROW || direction_ == FlexDirection::ROW_REVERSE) {
        layoutConstraint.selfIdealSize.SetWidth(size);
    } else {
        layoutConstraint.selfIdealSize.SetHeight(size);
    }
}

void FlexLayoutAlgorithm::UpdateLayoutConstraintOnCrossAxis(LayoutConstraintF& layoutConstraint, float size)
{
    OptionalSizeF& selfIdealSize = layoutConstraint.selfIdealSize;
    if (direction_ == FlexDirection::ROW || direction_ == FlexDirection::ROW_REVERSE) {
        selfIdealSize.SetHeight(size);
    } else {
        selfIdealSize.SetWidth(size);
    }
}

float FlexLayoutAlgorithm::MainAxisMinValue(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_RETURN(layoutWrapper, 0.0f);
    CHECK_NULL_RETURN(layoutWrapper->GetLayoutProperty(), 0.0f);
    auto layoutConstraint = layoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
    CHECK_NULL_RETURN(layoutConstraint, 0.0f);
    return IsHorizontal(direction_) ? layoutConstraint->minSize.Width() : layoutConstraint->minSize.Height();
}

bool FlexLayoutAlgorithm::MarginOnMainAxisNegative(LayoutWrapper* layoutWrapper)
{
    const auto& margin = layoutWrapper->GetGeometryNode()->GetMargin();
    CHECK_NULL_RETURN(margin, false);
    if (IsHorizontal(direction_)) {
        return LessNotEqual(margin->left.value_or(0.0f) + margin->right.value_or(0.0f), 0.0f);
    }
    return LessNotEqual(margin->top.value_or(0.0f) + margin->bottom.value_or(0.0f), 0.0f);
}

bool FlexLayoutAlgorithm::CheckSetConstraint(const std::unique_ptr<MeasureProperty>& propertyPtr)
{
    return propertyPtr && (propertyPtr->minSize || propertyPtr->maxSize);
}

void FlexLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    const auto& children = layoutWrapper->GetAllChildrenWithBuild();
    /**
     * Obtain the main axis size and cross axis size based on user setting.
     */
    const auto& layoutConstraint = layoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
    const auto& rawConstraint = layoutWrapper->GetLayoutProperty()->GetCalcLayoutConstraint();
    bool needToConstraint = CheckSetConstraint(rawConstraint) && children.empty();
    const auto& measureType = layoutWrapper->GetLayoutProperty()->GetMeasureType();
    InitFlexProperties(layoutWrapper);
    Axis axis = (direction_ == FlexDirection::ROW || direction_ == FlexDirection::ROW_REVERSE) ? Axis::HORIZONTAL
                                                                                               : Axis::VERTICAL;
    auto realSize = CreateIdealSizeByPercentRef(layoutConstraint.value(), axis, measureType, needToConstraint,
        rawConstraint).ConvertToSizeT();
    if (children.empty()) {
        layoutWrapper->GetGeometryNode()->SetFrameSize(realSize);
        return;
    }
    mainAxisSize_ = GetMainAxisSizeHelper(realSize, direction_);
    /**
     * The user has not set the main axis size
     */
    isInfiniteLayout_ = false;
    selfAdaptive_ = false;
    bool mainAxisInf =
        GreaterOrEqualToInfinity(direction_ == FlexDirection::ROW || direction_ == FlexDirection::ROW_REVERSE
                                     ? layoutConstraint->maxSize.Width()
                                     : layoutConstraint->maxSize.Height()) &&
        NearEqual(mainAxisSize_, -1.0f);
    if (NearEqual(mainAxisSize_, -1.0f)) {
        auto marginOnMainAxisNegative = MarginOnMainAxisNegative(layoutWrapper);
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            mainAxisSize_ = direction_ == FlexDirection::ROW || direction_ == FlexDirection::ROW_REVERSE
                                ? layoutConstraint->maxSize.Width()
                                : layoutConstraint->maxSize.Height();
        } else if (isLinearLayoutFeature_ && IsHorizontal(direction_) && !NearZero(layoutConstraint->minSize.Width()) &&
                   !marginOnMainAxisNegative) {
            mainAxisSize_ = layoutConstraint->minSize.Width();
        } else if (isLinearLayoutFeature_ && !IsHorizontal(direction_) &&
                   !NearZero(layoutConstraint->minSize.Height()) && !marginOnMainAxisNegative) {
            mainAxisSize_ = layoutConstraint->minSize.Height();
        } else {
            mainAxisSize_ =
                direction_ == FlexDirection::ROW || direction_ == FlexDirection::ROW_REVERSE
                    ? (mainAxisInf ? layoutConstraint->percentReference.Width()
                                   : std::max(layoutConstraint->minSize.Width(), layoutConstraint->maxSize.Width()))
                    : (mainAxisInf ? layoutConstraint->percentReference.Height()
                                   : std::max(layoutConstraint->minSize.Height(), layoutConstraint->maxSize.Height()));
        }
        isInfiniteLayout_ = isLinearLayoutFeature_;
    }
    selfAdaptive_ = isLinearLayoutFeature_;
    if (!isInfiniteLayout_) {
        isInfiniteLayout_ = mainAxisInf;
    }
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    auto horizontalPadding = padding.left.value_or(0.0f) + padding.right.value_or(0.0f);
    auto verticalPadding = padding.top.value_or(0.0f) + padding.bottom.value_or(0.0f);
    if (direction_ == FlexDirection::ROW || direction_ == FlexDirection::ROW_REVERSE) {
        mainAxisSize_ -= horizontalPadding;
    } else {
        mainAxisSize_ -= verticalPadding;
    }
    if (Negative(mainAxisSize_)) {
        mainAxisSize_ = 0.0f;
    }
    TravelChildrenFlexProps(layoutWrapper, realSize);
    selfIdealCrossAxisSize_ = GetCrossAxisSizeHelper(realSize, direction_);
    FlexItemProperties flexItemProperties;

    /**
     * first measure
     */
    MeasureAndCleanMagicNodes(layoutWrapper, flexItemProperties);

    /**
     * secondary measure
     */
    SecondaryMeasureByProperty(flexItemProperties, layoutWrapper);

    /**
     *  position property measure.
     */
    MeasureOutOfLayoutChildren(layoutWrapper);

    AdjustTotalAllocatedSize(layoutWrapper);

    /**
     * For Row and Column, the main axis size is wrapContent.
     * And, FlexLayoutAlgorithm, as the parent class, should not handle the special logic of the subclass LinearLayout.
     */
    if (isInfiniteLayout_) {
        mainAxisSize_ = allocatedSize_;
    }

    auto finalMainAxisSize = mainAxisSize_;
    auto finalCrossAxisSize = crossAxisSize_;
    if (direction_ == FlexDirection::ROW || direction_ == FlexDirection::ROW_REVERSE) {
        finalCrossAxisSize += verticalPadding;
        finalMainAxisSize += horizontalPadding;
    } else {
        finalCrossAxisSize += horizontalPadding;
        finalMainAxisSize += verticalPadding;
    }
    auto mainAxisSizeMin = GetMainAxisSizeHelper(layoutConstraint->minSize, direction_);
    auto mainAxisSizeMax = GetMainAxisSizeHelper(layoutConstraint->maxSize, direction_);
    auto crossAxisSizeMin = GetCrossAxisSizeHelper(layoutConstraint->minSize, direction_);
    auto crossAxisSizeMax = GetCrossAxisSizeHelper(layoutConstraint->maxSize, direction_);
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        finalMainAxisSize = std::max(mainAxisSizeMin, std::min(finalMainAxisSize, mainAxisSizeMax));
        finalCrossAxisSize = std::max(crossAxisSizeMin, std::min(finalCrossAxisSize, crossAxisSizeMax));
    } else {
        finalMainAxisSize = std::clamp(
            finalMainAxisSize, std::min(mainAxisSizeMin, mainAxisSizeMax), std::max(mainAxisSizeMin, mainAxisSizeMax));
        finalCrossAxisSize = std::clamp(finalCrossAxisSize, std::min(crossAxisSizeMin, crossAxisSizeMax),
            std::max(crossAxisSizeMin, crossAxisSizeMax));
    }

    realSize.UpdateIllegalSizeWithCheck(
        GetCalcSizeHelper(finalMainAxisSize, finalCrossAxisSize, direction_).ConvertToSizeT());
    layoutWrapper->GetGeometryNode()->SetFrameSize(realSize);
}

void FlexLayoutAlgorithm::AdjustTotalAllocatedSize(LayoutWrapper* layoutWrapper)
{
    const auto& children = layoutWrapper->GetAllChildrenWithBuild(false);
    if (children.empty()) {
        allocatedSize_ = 0.0f;
        return;
    }
    allocatedSize_ = 0.0f;
    allocatedSize_ += space_ * (validSizeCount_ - 1);
    // space is not valid when mainAxisAlign is SPACE_AROUND/SPACE_BETWEEN/SPACE_EVENLY
    if (mainAxisAlign_ == FlexAlign::SPACE_AROUND || mainAxisAlign_ == FlexAlign::SPACE_BETWEEN ||
        mainAxisAlign_ == FlexAlign::SPACE_EVENLY) {
        allocatedSize_ = 0.0;
    }
    for (const auto& child : children) {
        if (child->IsOutOfLayout() ||
            (layoutWrapper && layoutWrapper->GetHostNode() && layoutWrapper->GetHostNode()->GetLayoutProperty() &&
                child->GetHostNode()->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE) ==
                    VisibleType::GONE)) {
            continue;
        }
        allocatedSize_ += GetChildMainAxisSize(child);
    }
}

/**
 * Get cross axis size in stretch.
 * At this time, the cross axis size has been determined.
 */
float FlexLayoutAlgorithm::GetStretchCrossAxisLimit() const
{
    float crossAxisLimit = GreatNotEqual(selfIdealCrossAxisSize_, -1.0f) ? selfIdealCrossAxisSize_ : crossAxisSize_;
    return crossAxisLimit;
}

void FlexLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    const auto& children = layoutWrapper->GetAllChildrenWithBuild(false);
    if (children.empty()) {
        return;
    }
    auto layoutProperty = AceType::DynamicCast<FlexLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    space_ = static_cast<float>(layoutProperty->GetSpaceValue({}).ConvertToPx());
    direction_ = layoutProperty->GetFlexDirection().value_or(FlexDirection::ROW);
    mainAxisAlign_ = layoutProperty->GetMainAxisAlignValue(FlexAlign::FLEX_START);
    crossAxisAlign_ =
        layoutProperty->GetCrossAxisAlignValue(isLinearLayoutFeature_ ? FlexAlign::CENTER : FlexAlign::FLEX_START);
    textDir_ = layoutProperty->GetLayoutDirection();
    if (textDir_ == TextDirection::AUTO) {
        textDir_ = AceApplicationInfo::GetInstance().IsRightToLeft() ? TextDirection::RTL : TextDirection::LTR;
    }
    auto contentSize = layoutWrapper->GetGeometryNode()->GetFrameSize();
    const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)
        ? MinusPaddingToNonNegativeSize(padding, contentSize)
        : MinusPaddingToSize(padding, contentSize);
    mainAxisSize_ = GetMainAxisSizeHelper(contentSize, direction_);
    crossAxisSize_ = GetCrossAxisSizeHelper(contentSize, direction_);
    auto paddingOffset = OffsetF(padding.left.value_or(0.0f), padding.top.value_or(0.0f));
    float remainSpace = std::max(mainAxisSize_ - allocatedSize_, 0.0f);
    float frontSpace = 0.0f;
    float betweenSpace = 0.0f;
    CalculateSpace(remainSpace, frontSpace, betweenSpace);
    PlaceChildren(layoutWrapper, frontSpace, betweenSpace, paddingOffset);

    for (auto&& child : children) {
        if (!child->IsOutOfLayout() && child->IsActive()) {
            child->Layout();
        }
    }
}

void FlexLayoutAlgorithm::CalculateSpace(float remainSpace, float& frontSpace, float& betweenSpace) const
{
    switch (mainAxisAlign_) {
        case FlexAlign::FLEX_START:
            frontSpace = 0.0f;
            betweenSpace = space_;
            break;
        case FlexAlign::FLEX_END:
            frontSpace = remainSpace;
            betweenSpace = space_;
            break;
        case FlexAlign::CENTER:
            frontSpace = remainSpace / 2;
            betweenSpace = space_;
            break;
        case FlexAlign::SPACE_BETWEEN:
            frontSpace = 0.0f;
            betweenSpace = validSizeCount_ > 1 ? remainSpace / static_cast<float>(validSizeCount_ - 1) : 0.0f;
            break;
        case FlexAlign::SPACE_AROUND:
            betweenSpace = validSizeCount_ > 0 ? remainSpace / static_cast<float>(validSizeCount_) : 0.0f;
            frontSpace = betweenSpace / 2;
            break;
        case FlexAlign::SPACE_EVENLY:
            betweenSpace = validSizeCount_ > 0 ? remainSpace / static_cast<float>(validSizeCount_ + 1) : 0.0f;
            frontSpace = betweenSpace;
            break;
        default:
            break;
    }
}

void FlexLayoutAlgorithm::PlaceChildren(
    LayoutWrapper* layoutWrapper, float frontSpace, float betweenSpace, const OffsetF& paddingOffset)
{
    float childMainPos = IsStartTopLeft(direction_, textDir_) ? frontSpace : mainAxisSize_ - frontSpace;
    float childCrossPos = 0.0f;
    const auto& children = layoutWrapper->GetAllChildrenWithBuild(false);
    for (const auto& child : children) {
        if (child->IsOutOfLayout() || !child->IsActive()) {
            // adjust by postion property.
            child->GetGeometryNode()->SetMarginFrameOffset({});
            child->Layout();
            continue;
        }
        if (layoutWrapper && layoutWrapper->GetHostNode() && layoutWrapper->GetHostNode()->GetLayoutProperty() &&
            child->GetHostNode()->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE) == VisibleType::GONE) {
            continue;
        }
        auto alignItem = GetSelfAlign(child);
        auto crossDirection = FlipAxis(direction_);
        switch (alignItem) {
            case FlexAlign::FLEX_START:
            case FlexAlign::FLEX_END:
                childCrossPos = (IsStartTopLeft(crossDirection, textDir_) == (alignItem == FlexAlign::FLEX_START))
                                    ? 0.0f
                                    : crossAxisSize_ - GetChildCrossAxisSize(child);
                break;
            case FlexAlign::CENTER:
                childCrossPos = crossAxisSize_ / 2 - GetChildCrossAxisSize(child) / 2;
                break;
            case FlexAlign::STRETCH:
                childCrossPos =
                    IsStartTopLeft(crossDirection, textDir_) ? 0.0f : crossAxisSize_ - GetChildCrossAxisSize(child);
                break;
            case FlexAlign::BASELINE:
                childCrossPos = 0.0;
                if (direction_ == FlexDirection::ROW || direction_ == FlexDirection::ROW_REVERSE) {
                    float distance = child->GetBaselineDistance();
                    childCrossPos = baselineProperties_.maxBaselineDistance - distance;
                }
                break;
            default:
                break;
        }
        OffsetF offset;
        if (direction_ == FlexDirection::ROW || direction_ == FlexDirection::ROW_REVERSE) {
            offset = OffsetF(childMainPos, childCrossPos);
        } else {
            offset = OffsetF(childCrossPos, childMainPos);
        }

        if (!IsStartTopLeft(direction_, textDir_)) {
            if (direction_ != FlexDirection::COLUMN_REVERSE) {
                offset.SetX(offset.GetX() - GetChildMainAxisSize(child));
            } else {
                offset.SetY(offset.GetY() - GetChildMainAxisSize(child));
            }
            child->GetGeometryNode()->SetMarginFrameOffset(offset + paddingOffset);
            childMainPos -= GetChildMainAxisSize(child) + betweenSpace;
        } else {
            child->GetGeometryNode()->SetMarginFrameOffset(offset + paddingOffset);
            childMainPos += GetChildMainAxisSize(child) + betweenSpace;
        }
    }
}

FlexAlign FlexLayoutAlgorithm::GetSelfAlign(const RefPtr<LayoutWrapper>& layoutWrapper) const
{
    const auto& flexItemProperty = layoutWrapper->GetLayoutProperty()->GetFlexItemProperty();
    FlexAlign crossAxisAlign = (crossAxisAlign_ == FlexAlign::AUTO) ? FlexAlign::FLEX_START : crossAxisAlign_;
    if (!flexItemProperty || !flexItemProperty->GetAlignSelf().has_value() ||
        flexItemProperty->GetAlignSelf().value_or(crossAxisAlign_) == FlexAlign::AUTO) {
        return crossAxisAlign;
    }
    return flexItemProperty->GetAlignSelf().value_or(crossAxisAlign);
}

} // namespace OHOS::Ace::NG
