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

#include "core/components_ng/pattern/indexer/indexer_layout_algorithm.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/size_t.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/layout_param.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/indexer/indexer_pattern.h"
#include "core/components_ng/pattern/indexer/indexer_theme.h"
#include "core/components_ng/pattern/text/text_model.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"

namespace OHOS::Ace::NG {

void IndexerLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto indexerLayoutProperty = AceType::DynamicCast<IndexerLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(indexerLayoutProperty);
    LayoutConstraintF layoutConstraint;
    if (indexerLayoutProperty->GetLayoutConstraint().has_value()) {
        layoutConstraint = indexerLayoutProperty->GetLayoutConstraint().value();
    }
    auto itemSize = indexerLayoutProperty->GetItemSize().value_or(Dimension(INDEXER_ITEM_SIZE, DimensionUnit::VP));
    itemSize_ = ConvertToPx(itemSize, layoutConstraint.scaleProperty, layoutConstraint.maxSize.Height()).value();
    auto defaultHorizontalPadding = Dimension(INDEXER_PADDING_LEFT, DimensionUnit::VP).ConvertToPx();
    auto defaultVerticalPadding = Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TWELVE)
                                      ? Dimension(INDEXER_PADDING_TOP_API_TWELVE, DimensionUnit::VP).ConvertToPx()
                                      : Dimension(INDEXER_PADDING_TOP, DimensionUnit::VP).ConvertToPx();
    const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorderWithDefault(
        static_cast<float>(defaultHorizontalPadding), static_cast<float>(defaultVerticalPadding), 0, 0);
    auto verticalPadding = (padding.top.value_or(0) + padding.bottom.value_or(0));
    auto horizontalPadding = padding.left.value_or(0.0f) + padding.right.value_or(0.0f);
    auto contentWidth = itemSize_ + horizontalPadding;
    auto contentHeight = itemCount_ * itemSize_ + verticalPadding;
    auto selfIdealSize = layoutConstraint.selfIdealSize;
    auto actualWidth = selfIdealSize.Width().has_value()
                           ? selfIdealSize.Width().value()
                           : std::clamp(contentWidth, 0.0f, layoutConstraint.maxSize.Width());
    actualHeight_ = selfIdealSize.Height().has_value()
                            ? selfIdealSize.Height().value()
                            : std::clamp(contentHeight, 0.0f, layoutConstraint.maxSize.Height());
    contentHeight = GreatNotEqual(contentHeight, actualHeight_) ? actualHeight_ : contentHeight;
    itemWidth_ = GreatOrEqual(actualWidth - horizontalPadding, 0.0f) ? actualWidth - horizontalPadding : 0.0f;
    itemSizeRender_ =
        GreatOrEqual(contentHeight - verticalPadding, 0.0f) ? (contentHeight - verticalPadding) / itemCount_ : 0.0f;
    auto childLayoutConstraint = indexerLayoutProperty->CreateChildConstraint();
    for (int32_t index = 0; index < itemCount_; index++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        CHECK_NULL_VOID(childWrapper);
        childLayoutConstraint.UpdateSelfMarginSizeWithCheck(OptionalSizeF(itemWidth_, itemSizeRender_));
        childWrapper->Measure(childLayoutConstraint);
    }
    if (indexerLayoutProperty->GetIsPopupValue(false)) {
        auto childCount = layoutWrapper->GetTotalChildCount();
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(childCount - 1);
        CHECK_NULL_VOID(childWrapper);
        auto childLayoutProperty = AceType::DynamicCast<LinearLayoutProperty>(childWrapper->GetLayoutProperty());
        CHECK_NULL_VOID(childLayoutProperty);
        auto layoutConstraint = childLayoutProperty->GetLayoutConstraint();
        layoutConstraint->Reset();
        childWrapper->Measure(layoutConstraint);
    }
    
    layoutWrapper->GetGeometryNode()->SetFrameSize(SizeF(actualWidth, actualHeight_));
}

void IndexerLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto layoutProperty = DynamicCast<IndexerLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    auto defaultHorizontalPadding = Dimension(INDEXER_PADDING_LEFT, DimensionUnit::VP).ConvertToPx();
    auto defaultVerticalPadding = Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TWELVE)
                                      ? Dimension(INDEXER_PADDING_TOP_API_TWELVE, DimensionUnit::VP).ConvertToPx()
                                      : Dimension(INDEXER_PADDING_TOP, DimensionUnit::VP).ConvertToPx();
    const auto& padding = layoutProperty->CreatePaddingAndBorderWithDefault(
        static_cast<float>(defaultHorizontalPadding), static_cast<float>(defaultVerticalPadding), 0, 0);
    MinusPaddingToSize(padding, size);
    auto left = padding.left.value_or(0);
    auto top = padding.top.value_or(0);
    auto paddingOffset = OffsetF(left, top);
    auto align = Alignment::CENTER;
    if (layoutProperty->GetPositionProperty()) {
        align = layoutProperty->GetPositionProperty()->GetAlignment().value_or(align);
    }
    SizeF contentSize;
    auto childCount = layoutWrapper->GetTotalChildCount();
    if (layoutProperty->GetIsPopupValue(false)) {
        const auto& child = layoutWrapper->GetChildByIndex(childCount - 1);
        auto offset = GetPositionOfPopupNode(layoutProperty, size.Width());
        child->GetHostNode()->GetRenderContext()->UpdatePosition(offset);
        child->Layout();
        childCount -= 1;
    }
    for (int32_t i = 0; i < childCount; i++) {
        const auto& child = layoutWrapper->GetChildByIndex(i);
        SizeF childSize = child->GetGeometryNode()->GetMarginFrameSize();
        contentSize += childSize;
    }
    for (int32_t i = 0; i < childCount; i++) {
        const auto& child = layoutWrapper->GetChildByIndex(i);
        auto translate = Alignment::GetAlignPosition(size, contentSize, align);
        child->GetGeometryNode()->SetMarginFrameOffset(
            translate + paddingOffset + OffsetF(0, itemSizeRender_ * i));
        child->Layout();
    }
}

OffsetT<Dimension> IndexerLayoutAlgorithm::GetPositionOfPopupNode(
    const RefPtr<IndexerLayoutProperty>& layoutProperty, float indexerWidth)
{
    auto alignMent = layoutProperty->GetAlignStyle().value_or(NG::AlignStyle::RIGHT);
    auto userDefinePositionX =
        layoutProperty->GetPopupPositionX().value_or(Dimension(NG::BUBBLE_POSITION_X, DimensionUnit::VP)).ConvertToPx();
    auto userDefinePositionY =
        layoutProperty->GetPopupPositionY().value_or(Dimension(NG::BUBBLE_POSITION_Y, DimensionUnit::VP)).ConvertToPx();
    auto userDefineSpace = layoutProperty->GetPopupHorizontalSpace();

    if (alignMent == NG::AlignStyle::LEFT) {
        auto xPos = userDefineSpace ? userDefineSpace.value().ConvertToPx() + indexerWidth
                                    : userDefinePositionX + indexerWidth / 2;
        return OffsetT<Dimension>(Dimension(xPos), Dimension(userDefinePositionY));
    } else {
        auto bubbleSize = Dimension(BUBBLE_BOX_SIZE, DimensionUnit::VP).ConvertToPx();
        auto xPos = (userDefineSpace ? -userDefineSpace.value().ConvertToPx()
                                    : -userDefinePositionX + indexerWidth / 2) - bubbleSize;
        return OffsetT<Dimension>(Dimension(xPos), Dimension(userDefinePositionY));
    }
}
} // namespace OHOS::Ace::NG
