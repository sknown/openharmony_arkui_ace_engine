/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/menu/menu_item/menu_item_layout_algorithm.h"

#include "base/geometry/ng/size_t.h"
#include "base/utils/utils.h"
#include "core/components/select/select_theme.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {
namespace {
    constexpr Dimension MENU_ITEM_CHILD_MIN_HEIGHT = 32.0_vp;
    constexpr Dimension MENU_ITEM_VERTICAL_PADDING = 8.0_vp;
}
void MenuItemLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    horInterval_ = static_cast<float>(theme->GetMenuIconPadding().ConvertToPx()) -
                   static_cast<float>(theme->GetOutPadding().ConvertToPx());
    middleSpace_ = static_cast<float>(theme->GetIconContentPadding().ConvertToPx());
    auto props = layoutWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(props);

    auto layoutConstraint = props->GetLayoutConstraint();
    CHECK_NULL_VOID(layoutConstraint);
    const auto& padding = props->CreatePaddingAndBorderWithDefault(horInterval_, 0.0f, 0.0f, 0.0f);
    maxRowWidth_ = layoutConstraint->maxSize.Width() - padding.Width();
    // update ideal width if user defined
    const auto& calcConstraint = props->GetCalcLayoutConstraint();
    if (calcConstraint && calcConstraint->selfIdealSize.has_value() &&
        calcConstraint->selfIdealSize.value().Width().has_value()) {
        ScaleProperty scaleProperty;
        if (layoutWrapper->GetGeometryNode() && layoutWrapper->GetGeometryNode()->GetParentLayoutConstraint()) {
            scaleProperty = layoutWrapper->GetGeometryNode()->GetParentLayoutConstraint()->scaleProperty;
        } else {
            scaleProperty = layoutConstraint->scaleProperty;
        }
        layoutConstraint->selfIdealSize.SetWidth(
            ConvertToPx(calcConstraint->selfIdealSize.value().Width()->GetDimension(), scaleProperty,
                layoutConstraint->percentReference.Width()));
    }
    if (layoutConstraint->selfIdealSize.Width().has_value()) {
        maxRowWidth_ =
            std::max(layoutConstraint->minSize.Width(),
                std::min(layoutConstraint->maxSize.Width(), layoutConstraint->selfIdealSize.Width().value())) -
            padding.Width();
    }
    CheckNeedMatchParent(layoutWrapper, layoutConstraint);
    minRowWidth_ = layoutConstraint->minSize.Width();

    auto childConstraint = props->CreateChildConstraint();
    minItemHeight_ = static_cast<float>(theme->GetOptionMinHeight().ConvertToPx());
    // set item min height
    childConstraint.minSize.SetHeight(Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TWELVE) ?
        MENU_ITEM_CHILD_MIN_HEIGHT.ConvertToPx() : minItemHeight_);

    iconSize_ = theme->GetIconSideLength().ConvertToPx();
    MeasureItemViews(childConstraint, layoutConstraint, padding, layoutWrapper);

    auto clickableArea = layoutWrapper->GetOrCreateChildByIndex(CLICKABLE_AREA_VIEW_INDEX);
    if (GreatNotEqual(idealWidth_, 0.0f)) {
        layoutWrapper->GetGeometryNode()->SetFrameWidth(idealWidth_);
        if (clickableArea) {
            clickableArea->GetGeometryNode()->SetFrameWidth(idealWidth_);
        }
    }

    CheckNeedExpandContent(layoutWrapper, childConstraint);
}

void MenuItemLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    float verticalInteval = 0.0f;
    if (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
        verticalInteval = static_cast<float>(MENU_ITEM_VERTICAL_PADDING.ConvertToPx());
    }
    const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorderWithDefault(horInterval_,
        verticalInteval, 0.0f, 0.0f);

    auto layoutDirection = layoutWrapper->GetLayoutProperty()->GetNonAutoLayoutDirection();
    auto leftRow = layoutWrapper->GetOrCreateChildByIndex(0);
    auto leftRowSize = leftRow ? leftRow->GetGeometryNode()->GetFrameSize() : SizeT(0.0f, 0.0f);
    auto rightRow = layoutWrapper->GetOrCreateChildByIndex(1);
    auto rightRowSize = rightRow ? rightRow->GetGeometryNode()->GetFrameSize() : SizeT(0.0f, 0.0f);

    auto itemHeight = std::max(leftRowSize.Height(), rightRowSize.Height());
    CHECK_NULL_VOID(leftRow);
    float topSpace = (itemHeight - leftRowSize.Height()) / 2.0f;
    if (padding.top.has_value() && Positive(padding.top.value()) && padding.top.value() > topSpace) {
        topSpace = padding.top.value();
    }
    leftRow->GetLayoutProperty()->UpdatePropertyChangeFlag(PROPERTY_UPDATE_LAYOUT);
    leftRow->GetGeometryNode()->SetMarginFrameOffset(OffsetF(padding.left.value_or(horInterval_), topSpace));
    if (layoutDirection == TextDirection::RTL) {
        leftRow->GetGeometryNode()->SetMarginFrameOffset(
            OffsetF(layoutWrapper->GetGeometryNode()->GetFrameSize().Width() - padding.right.value_or(horInterval_) -
                    leftRow->GetGeometryNode()->GetFrameSize().Width(),
            topSpace));
    }
    leftRow->Layout();

    CHECK_NULL_VOID(rightRow);
    topSpace = (itemHeight - rightRowSize.Height()) / 2.0f;
    if (padding.top.has_value() && Positive(padding.top.value()) && padding.top.value() > topSpace) {
        topSpace = padding.top.value();
    }
    rightRow->GetGeometryNode()->SetMarginFrameOffset(
        OffsetF(layoutWrapper->GetGeometryNode()->GetFrameSize().Width() - padding.right.value_or(horInterval_) -
            rightRow->GetGeometryNode()->GetFrameSize().Width(), topSpace));
    if (layoutDirection == TextDirection::RTL) {
        rightRow->GetGeometryNode()->SetMarginFrameOffset(OffsetF(padding.left.value_or(horInterval_), topSpace));
    }
    rightRow->Layout();

    auto clickableArea = layoutWrapper->GetOrCreateChildByIndex(CLICKABLE_AREA_VIEW_INDEX);
    CHECK_NULL_VOID(clickableArea);
    clickableArea->Layout();

    auto expandableArea = layoutWrapper->GetOrCreateChildByIndex(EXPANDABLE_AREA_VIEW_INDEX);
    CHECK_NULL_VOID(expandableArea);
    expandableArea->GetLayoutProperty()->UpdatePropertyChangeFlag(PROPERTY_UPDATE_LAYOUT);
    expandableArea->GetGeometryNode()->SetMarginFrameOffset(OffsetF(
        padding.left.value_or(horInterval_), itemHeight + padding.Height()));
    expandableArea->Layout();
}

void MenuItemLayoutAlgorithm::CheckNeedMatchParent(LayoutWrapper* layoutWrapper,
    std::optional<LayoutConstraintF>& layoutConstraint)
{
    auto menuNode = layoutWrapper->GetHostNode();
    auto menuItemPattern = menuNode ? menuNode->GetPattern<MenuItemPattern>() : nullptr;
    auto expandingMode = menuItemPattern ? menuItemPattern->GetExpandingMode() : SubMenuExpandingMode::STACK;
    auto isSubMenu = menuItemPattern ? menuItemPattern->IsSubMenu() : false;
    auto isEmbedded = menuItemPattern ? menuItemPattern->IsEmbedded() : false;
    bool matchParent = (expandingMode == SubMenuExpandingMode::STACK && isSubMenu) ||
                       (expandingMode == SubMenuExpandingMode::EMBEDDED && isEmbedded);
    if (matchParent) {
        auto width = layoutConstraint->maxSize.Width();
        layoutConstraint->minSize.SetWidth(width);
    }
}

void MenuItemLayoutAlgorithm::MeasureItemViews(LayoutConstraintF& childConstraint,
    std::optional<LayoutConstraintF>& layoutConstraint, PaddingPropertyF padding, LayoutWrapper* layoutWrapper)
{
    auto leftRow = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_VOID(leftRow);
    childConstraint.maxSize.SetWidth(leftRow->GetGeometryNode()->GetFrameSize().Width()
        // Cannot cover left icon
        ? maxRowWidth_ - middleSpace_ - static_cast<float>(iconSize_)
        : maxRowWidth_);
    // measure right row
    float rightRowWidth = 0.0f;
    float rightRowHeight = 0.0f;
    auto rightRow = layoutWrapper->GetOrCreateChildByIndex(1);
    if (rightRow) {
        rightRow->Measure(childConstraint);
        rightRowWidth = rightRow->GetGeometryNode()->GetMarginFrameSize().Width();
        rightRowHeight = rightRow->GetGeometryNode()->GetMarginFrameSize().Height();
    }
    // measure left row
    auto maxWidth = maxRowWidth_ - rightRowWidth + middleSpace_;
    childConstraint.maxSize.SetWidth(maxWidth);
    MeasureRow(leftRow, childConstraint);
    float leftRowWidth = leftRow->GetGeometryNode()->GetMarginFrameSize().Width();
    float leftRowHeight = leftRow->GetGeometryNode()->GetMarginFrameSize().Height();
    float contentWidth = leftRowWidth + rightRowWidth + padding.Width() + middleSpace_;

    auto itemHeight = std::max(leftRowHeight, rightRowHeight);
    auto width = std::max(minRowWidth_, contentWidth);

    needExpandContent_ = false;
    emptyWidth_ = 0.0f;
    if (contentWidth < minRowWidth_) {
        emptyWidth_ = minRowWidth_ - contentWidth;
        needExpandContent_ = true;
    }

    idealWidth_ = 0.0f;
    if (layoutConstraint->selfIdealSize.Width().has_value()) {
        idealWidth_ = std::max(layoutConstraint->minSize.Width(),
            std::min(layoutConstraint->maxSize.Width(), layoutConstraint->selfIdealSize.Width().value()));

        float newLeftRowWidth = idealWidth_ - rightRowWidth - padding.Width() - middleSpace_;
        if (newLeftRowWidth > leftRowWidth) {
            emptyWidth_ = newLeftRowWidth - leftRowWidth;
            needExpandContent_ = true;
        }
    }
    
    auto actualWidth = GreatNotEqual(idealWidth_, 0.0f) ? idealWidth_ : width;
    childConstraint.minSize.SetWidth(actualWidth - padding.Width());
    childConstraint.maxSize.SetWidth(actualWidth - padding.Width());

    auto expandableHeight = 0.0f;
    auto expandableArea = layoutWrapper->GetOrCreateChildByIndex(EXPANDABLE_AREA_VIEW_INDEX);
    if (expandableArea) {
        expandableArea->Measure(childConstraint);
        expandableHeight = expandableArea->GetGeometryNode()->GetMarginFrameSize().Height();
    }

    UpdateSelfSize(layoutWrapper, actualWidth, itemHeight, expandableHeight);
}

void MenuItemLayoutAlgorithm::MeasureRow(const RefPtr<LayoutWrapper>& row, const LayoutConstraintF& constraint)
{
    auto children = row->GetAllChildrenWithBuild();
    CHECK_NULL_VOID(!children.empty());
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    auto iconContentPadding = static_cast<float>(theme->GetIconContentPadding().ConvertToPx());

    float spaceWidth = constraint.maxSize.Width();
    float rowWidth = 0.0f;
    float rowHeight = 0.0f;
    for (const auto& child : children) {
        if (child != children.back()) {
            // not content node
            child->Measure(constraint);
        } else {
            // content node update constraint max width
            auto contentConstraint = constraint;
            contentConstraint.maxSize.SetWidth(spaceWidth);
            child->Measure(contentConstraint);
            auto frameNode  = child->GetHostNode();
            UpdateOverlay(frameNode);
        }
        auto childSize = child->GetGeometryNode()->GetMarginFrameSize();
        spaceWidth -= childSize.Width() + iconContentPadding;
        rowWidth += childSize.Width() + iconContentPadding;
        rowHeight = std::max(rowHeight, childSize.Height());
    }
    rowWidth -= iconContentPadding;
    row->GetGeometryNode()->SetFrameSize(SizeF(rowWidth, rowHeight));
}

void MenuItemLayoutAlgorithm::UpdateOverlay(RefPtr<FrameNode> frameNode)
{
    CHECK_NULL_VOID(frameNode);
    auto overlayNode = frameNode ->GetOverlayNode();
    CHECK_NULL_VOID(overlayNode);
    auto geometryNode = frameNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto textFrameSize_ = geometryNode->GetMarginFrameSize();
    auto overlayGeometryNode = overlayNode->GetGeometryNode();
    CHECK_NULL_VOID(overlayGeometryNode);
    overlayGeometryNode->SetFrameSize(textFrameSize_);
    overlayNode->Layout();
}

void MenuItemLayoutAlgorithm::CheckNeedExpandContent(LayoutWrapper* layoutWrapper,
    LayoutConstraintF& childConstraint)
{
    if (needExpandContent_) {
        auto menuItemNode = layoutWrapper->GetHostNode();
        CHECK_NULL_VOID(menuItemNode);
        auto pattern = menuItemNode->GetPattern<MenuItemPattern>();
        CHECK_NULL_VOID(pattern);
        auto contentNode = pattern->GetContentNode();
        CHECK_NULL_VOID(contentNode);

        auto leftRow = layoutWrapper->GetChildByIndex(0);
        CHECK_NULL_VOID(leftRow);
        auto newRowSize = leftRow->GetGeometryNode()->GetFrameSize();
        newRowSize.SetWidth(emptyWidth_ + newRowSize.Width());
        leftRow->GetGeometryNode()->SetFrameSize(newRowSize);

        auto oldTextSize = contentNode->GetGeometryNode()->GetFrameSize();
        float newTextWidth = emptyWidth_ + oldTextSize.Width();
        childConstraint.minSize.SetWidth(newTextWidth);
        contentNode->Measure(childConstraint);
    }
}

void MenuItemLayoutAlgorithm::UpdateSelfSize(LayoutWrapper* layoutWrapper,
    float width, float itemHeight, float expandableHeight)
{
    itemHeight += GetDividerStroke(layoutWrapper);
    auto clickableArea = layoutWrapper->GetOrCreateChildByIndex(CLICKABLE_AREA_VIEW_INDEX);
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        auto height = std::max(itemHeight, minItemHeight_);
        layoutWrapper->GetGeometryNode()->SetContentSize(SizeF(width, height + expandableHeight));
        if (clickableArea) {
            clickableArea->GetGeometryNode()->SetFrameSize(SizeF(width, height));
        }
    } else {
        layoutWrapper->GetGeometryNode()->SetContentSize(SizeF(width, itemHeight));
    }
    BoxLayoutAlgorithm::PerformMeasureSelf(layoutWrapper);
}

float MenuItemLayoutAlgorithm::GetDividerStroke(LayoutWrapper* layoutWrapper)
{
    auto menuItemNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(menuItemNode, 0.0f);
    auto pattern = menuItemNode->GetPattern<MenuItemPattern>();
    CHECK_NULL_RETURN(pattern, 0.0f);
    return pattern->GetDividerStroke();
}
} // namespace OHOS::Ace::NG
