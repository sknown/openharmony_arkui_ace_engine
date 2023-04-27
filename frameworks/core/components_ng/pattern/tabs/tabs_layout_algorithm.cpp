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

#include "core/components_ng/pattern/tabs/tabs_layout_algorithm.h"

#include "base/geometry/axis.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/log/ace_trace.h"
#include "base/utils/utils.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t SWIPER_INDEX = 0;
constexpr int32_t DIVIDER_INDEX = 1;
constexpr int32_t TAB_BAR_INDEX = 2;
} // namespace

void TabsLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto layoutProperty = AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto axis = layoutProperty->GetAxis().value_or(Axis::HORIZONTAL);
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto idealSize = CreateIdealSize(constraint.value(), Axis::HORIZONTAL, layoutProperty->GetMeasureType(), true);
    if (GreaterOrEqualToInfinity(idealSize.Width()) || GreaterOrEqualToInfinity(idealSize.Height())) {
        LOGW("Size is infinity.");
        geometryNode->SetFrameSize(SizeF());
        return;
    }
    geometryNode->SetFrameSize(idealSize);
    MinusPaddingToSize(layoutProperty->CreatePaddingAndBorder(), idealSize);
    auto childLayoutConstraint = layoutProperty->CreateChildConstraint();
    childLayoutConstraint.parentIdealSize = OptionalSizeF(idealSize);
    float dividerStrokeWidth = 0.0f;

    // Measure tab bar.
    auto tabBarWrapper = layoutWrapper->GetOrCreateChildByIndex(TAB_BAR_INDEX);
    SizeF tabBarSize;
    if (tabBarWrapper) {
        tabBarWrapper->Measure(childLayoutConstraint);
        tabBarSize = tabBarWrapper->GetGeometryNode()->GetMarginFrameSize();
    }

    // Measure divider.
    auto dividerWrapper = layoutWrapper->GetOrCreateChildByIndex(DIVIDER_INDEX);
    if (dividerWrapper) {
        dividerStrokeWidth = MeasureDivider(layoutProperty, dividerWrapper, idealSize);
    }

    // Measure swiper.
    auto swiperWrapper = layoutWrapper->GetOrCreateChildByIndex(SWIPER_INDEX);
    if (swiperWrapper) {
        SizeF parentIdealSize = idealSize;
        if (axis == Axis::HORIZONTAL) {
            childLayoutConstraint.selfIdealSize.SetHeight(
                childLayoutConstraint.maxSize.Height() - tabBarSize.Height() - dividerStrokeWidth);
            childLayoutConstraint.selfIdealSize.SetWidth(childLayoutConstraint.maxSize.Width());
            parentIdealSize.SetHeight(idealSize.Height() - tabBarSize.Height() - dividerStrokeWidth);
        } else if (axis == Axis::VERTICAL) {
            childLayoutConstraint.selfIdealSize.SetWidth(
                childLayoutConstraint.maxSize.Width() - tabBarSize.Width() - dividerStrokeWidth);
            childLayoutConstraint.selfIdealSize.SetHeight(childLayoutConstraint.maxSize.Height());
            parentIdealSize.SetWidth(idealSize.Width() - tabBarSize.Width() - dividerStrokeWidth);
        }
        childLayoutConstraint.parentIdealSize = OptionalSizeF(parentIdealSize);
        swiperWrapper->Measure(childLayoutConstraint);
    }
}

void TabsLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto frameSize = geometryNode->GetFrameSize();
    if (!frameSize.IsPositive()) {
        LOGW("Frame size is not positive.");
        return;
    }

    auto tabBarWrapper = layoutWrapper->GetOrCreateChildByIndex(TAB_BAR_INDEX);
    auto dividerWrapper = layoutWrapper->GetOrCreateChildByIndex(DIVIDER_INDEX);
    auto swiperWrapper = layoutWrapper->GetOrCreateChildByIndex(SWIPER_INDEX);
    if (!tabBarWrapper || !dividerWrapper || !swiperWrapper) {
        return;
    }

    auto offsetList = LayoutOffsetList(layoutWrapper, tabBarWrapper, frameSize);

    swiperWrapper->GetGeometryNode()->SetMarginFrameOffset(offsetList[SWIPER_INDEX]);
    swiperWrapper->Layout();

    dividerWrapper->GetGeometryNode()->SetMarginFrameOffset(offsetList[DIVIDER_INDEX]);
    dividerWrapper->Layout();
	
    tabBarWrapper->GetGeometryNode()->SetMarginFrameOffset(offsetList[TAB_BAR_INDEX]);
    tabBarWrapper->Layout();
}

std::vector<OffsetF> TabsLayoutAlgorithm::LayoutOffsetList(LayoutWrapper* layoutWrapper,
    const RefPtr<LayoutWrapper>& tabBarWrapper, const SizeF& frameSize) const
{
    std::vector<OffsetF> offsetList;
    OffsetF tabBarOffset;
    OffsetF dividerOffset;
    OffsetF swiperOffset;
    auto axis = GetAxis(layoutWrapper);
    auto barPosition = GetBarPosition(layoutWrapper);
    auto divider = GetDivider(layoutWrapper);
    auto tabBarGeometryNode = tabBarWrapper->GetGeometryNode();
    CHECK_NULL_RETURN(tabBarGeometryNode, offsetList);
    auto tabBarFrameSize = tabBarGeometryNode->GetMarginFrameSize();
    auto dividerStrokeWidth = divider.strokeWidth.ConvertToPx();
    auto dividerStartMargin = divider.startMargin.ConvertToPx();
    auto layoutProperty = DynamicCast<TabsLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, offsetList);
    auto padding = layoutProperty->CreatePaddingAndBorder();

    if (axis == Axis::HORIZONTAL) {
        float barPosX = (frameSize.MainSize(Axis::HORIZONTAL) - tabBarFrameSize.MainSize(Axis::HORIZONTAL)) / 2;
        if (barPosition == BarPosition::START) {
            tabBarOffset = OffsetF(barPosX, padding.Offset().GetY());
            dividerOffset = OffsetF(dividerStartMargin,
                tabBarFrameSize.MainSize(Axis::VERTICAL) + padding.Offset().GetY());
            swiperOffset = OffsetF(padding.Offset().GetX(),
                tabBarFrameSize.MainSize(Axis::VERTICAL) + dividerStrokeWidth + padding.Offset().GetY());
        } else {
            tabBarOffset = OffsetF(barPosX, frameSize.MainSize(Axis::VERTICAL) -
                tabBarFrameSize.MainSize(Axis::VERTICAL) - padding.Offset().GetY());
            dividerOffset = OffsetF(dividerStartMargin, frameSize.MainSize(Axis::VERTICAL) -
                tabBarFrameSize.MainSize(Axis::VERTICAL) - padding.Offset().GetY() - dividerStrokeWidth);
            swiperOffset = padding.Offset();
        }
    } else {
        float barPosY = (frameSize.MainSize(Axis::VERTICAL) - tabBarFrameSize.MainSize(Axis::VERTICAL)) / 2;
        if (barPosition == BarPosition::START) {
            tabBarOffset = OffsetF(padding.Offset().GetX(), barPosY);
            dividerOffset = OffsetF(tabBarFrameSize.MainSize(Axis::HORIZONTAL) + padding.Offset().GetX(),
                dividerStartMargin);
            swiperOffset = OffsetF(tabBarFrameSize.MainSize(Axis::HORIZONTAL) +
                padding.Offset().GetX() + dividerStrokeWidth, padding.Offset().GetY());
        } else {
            tabBarOffset = OffsetF(frameSize.MainSize(Axis::HORIZONTAL) - tabBarFrameSize.MainSize(Axis::HORIZONTAL) -
                padding.Offset().GetX(), barPosY);
            dividerOffset = OffsetF(frameSize.MainSize(Axis::HORIZONTAL) - tabBarFrameSize.MainSize(Axis::HORIZONTAL) -
                padding.Offset().GetX() - dividerStrokeWidth, dividerStartMargin);
            swiperOffset = padding.Offset();
        }
    }
    offsetList.emplace_back(swiperOffset);
    offsetList.emplace_back(dividerOffset);
    offsetList.emplace_back(tabBarOffset);
    return offsetList;
}

BarPosition TabsLayoutAlgorithm::GetBarPosition(LayoutWrapper* layoutWrapper) const
{
    auto layoutProperty = AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, BarPosition::START);
    return layoutProperty->GetTabBarPosition().value_or(BarPosition::START);
}

Axis TabsLayoutAlgorithm::GetAxis(LayoutWrapper* layoutWrapper) const
{
    auto layoutProperty = AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, Axis::HORIZONTAL);
    return layoutProperty->GetAxis().value_or(Axis::HORIZONTAL);
}

TabsItemDivider TabsLayoutAlgorithm::GetDivider(LayoutWrapper* layoutWrapper) const
{
    auto layoutProperty = AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper->GetLayoutProperty());
    TabsItemDivider divider;
    CHECK_NULL_RETURN(layoutProperty, divider);
    return layoutProperty->GetDivider().value_or(divider);
}

float TabsLayoutAlgorithm::MeasureDivider(const RefPtr<TabsLayoutProperty>& layoutProperty,
    const RefPtr<LayoutWrapper>& dividerWrapper, const SizeF& idealSize)
{
    auto constraint = layoutProperty->GetLayoutConstraint();
    
    auto dividerIdealSize = CreateIdealSize(
        constraint.value(), Axis::HORIZONTAL, layoutProperty->GetMeasureType(MeasureType::MATCH_PARENT), true);
    TabsItemDivider defaultDivider;
    auto axis = layoutProperty->GetAxis().value_or(Axis::HORIZONTAL);
    auto parentWidth = idealSize.Width();
    auto parentHeight = idealSize.Height();
    auto divider = layoutProperty->GetDivider().value_or(defaultDivider);
    auto dividerStrokeWidth = divider.strokeWidth.ConvertToPx();
    auto dividerStartMargin = divider.startMargin.ConvertToPx();
    auto dividerEndMargin = divider.endMargin.ConvertToPx();

    if (axis == Axis::VERTICAL) {
        dividerIdealSize.SetWidth(dividerStrokeWidth);
        dividerIdealSize.SetHeight(parentHeight - dividerStartMargin - dividerEndMargin);
    } else if (axis == Axis::HORIZONTAL) {
        dividerIdealSize.SetWidth(parentWidth - dividerStartMargin - dividerEndMargin);
        dividerIdealSize.SetHeight(dividerStrokeWidth);
    }

    auto dividerLayoutConstraint = layoutProperty->CreateChildConstraint();
    dividerLayoutConstraint.selfIdealSize = OptionalSizeF(dividerIdealSize);
    dividerWrapper->Measure(dividerLayoutConstraint);

    return dividerStrokeWidth;
}
} // namespace OHOS::Ace::NG
