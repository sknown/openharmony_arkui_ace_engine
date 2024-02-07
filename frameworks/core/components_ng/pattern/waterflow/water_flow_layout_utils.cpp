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
#include "frameworks/core/components_ng/pattern/waterflow/water_flow_layout_utils.h"

#include "base/utils/string_utils.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/waterflow/water_flow_item_layout_property.h"
namespace OHOS::Ace::NG {
namespace {
const std::string UNIT_AUTO = "auto";
}
std::string WaterFlowLayoutUtils::PreParseArgs(const std::string& args)
{
    if (args.empty() || args.find(UNIT_AUTO) == std::string::npos) {
        return args;
    }
    std::string rowsArgs;
    std::vector<std::string> strs;
    StringUtils::StringSplitter(args, ' ', strs);
    std::string current;
    size_t rowArgSize = strs.size();
    for (size_t i = 0; i < rowArgSize; ++i) {
        current = strs[i];
        // "auto" means 1fr in waterflow
        if (strs[i] == std::string(UNIT_AUTO)) {
            current = "1fr";
        }
        rowsArgs += ' ' + current;
    }
    return rowsArgs;
}

FlowItemPosition WaterFlowLayoutUtils::GetItemPosition(const WaterFlowLayoutInfo& info, int32_t index, float mainGap)
{
    auto crossIndex = info.GetCrossIndex(index);
    // already in layoutInfo
    if (crossIndex != -1) {
        return { crossIndex, info.GetStartMainPos(crossIndex, index) };
    }
    auto itemIndex = info.GetCrossIndexForNextItem(info.GetSegment(index));
    if (itemIndex.lastItemIndex < 0) {
        return { itemIndex.crossIndex, 0.0f };
    }
    auto mainHeight = info.GetMainHeight(itemIndex.crossIndex, itemIndex.lastItemIndex);
    return { itemIndex.crossIndex, mainHeight + mainGap };
}

LayoutConstraintF WaterFlowLayoutUtils::CreateChildConstraint(
    const ConstraintParams& params, const RefPtr<WaterFlowLayoutProperty>& props, const RefPtr<LayoutWrapper>& child)
{
    auto itemConstraint = props->CreateChildConstraint();
    auto itemMainSize = params.mainSize;
    auto itemIdealSize =
        params.axis == Axis::VERTICAL ? SizeF(params.crossSize, itemMainSize) : SizeF(itemMainSize, params.crossSize);

    itemConstraint.maxSize = itemIdealSize;
    itemConstraint.maxSize.SetMainSize(Infinity<float>(), params.axis);
    itemConstraint.percentReference = itemIdealSize;

    CHECK_NULL_RETURN(props->HasItemLayoutConstraint(), itemConstraint);

    OptionalSizeF childMinSize;
    OptionalSizeF childMaxSize;
    // Waterflow ItemLayoutConstraint
    auto itemMinSize = props->GetItemMinSize();
    if (itemMinSize.has_value()) {
        childMinSize =
            ConvertToOptionalSize(itemMinSize.value(), props->GetLayoutConstraint()->scaleProperty, itemIdealSize);
    }
    auto itemMaxSize = props->GetItemMaxSize();
    if (itemMaxSize.has_value()) {
        childMaxSize =
            ConvertToOptionalSize(itemMaxSize.value(), props->GetLayoutConstraint()->scaleProperty, itemIdealSize);
    }

    if (childMaxSize.AtLeastOneValid()) {
        itemConstraint.maxSize.UpdateSizeWhenSmaller(childMaxSize.ConvertToSizeT());
    }
    if (childMinSize.AtLeastOneValid()) {
        itemConstraint.minSize.UpdateSizeWhenLarger(childMinSize.ConvertToSizeT());
    }

    // FlowItem layoutConstraint
    CHECK_NULL_RETURN(child, itemConstraint);
    auto childLayoutProperty = AceType::DynamicCast<WaterFlowItemLayoutProperty>(child->GetLayoutProperty());
    CHECK_NULL_RETURN(childLayoutProperty, itemConstraint);
    if (childLayoutProperty->HasLayoutConstraint()) {
        if (childLayoutProperty->GetMaxSize().has_value()) {
            itemConstraint.UpdateMaxSizeWithCheck(ConvertToSize(childLayoutProperty->GetMaxSize().value(),
                itemConstraint.scaleProperty, itemConstraint.percentReference));
        }
        if (childLayoutProperty->GetMinSize().has_value()) {
            itemConstraint.UpdateMinSizeWithCheck(ConvertToSize(childLayoutProperty->GetMinSize().value(),
                itemConstraint.scaleProperty, itemConstraint.percentReference));
        }
    }

    childLayoutProperty->ResetCalcMinSize();
    childLayoutProperty->ResetCalcMaxSize();

    childLayoutProperty->UpdateItemCalcMaxSize(CalcSize(CalcLength(itemConstraint.maxSize.Width(), DimensionUnit::PX),
        CalcLength(itemConstraint.maxSize.Height(), DimensionUnit::PX)));
    childLayoutProperty->UpdateItemCalcMinSize(CalcSize(CalcLength(itemConstraint.minSize.Width(), DimensionUnit::PX),
        CalcLength(itemConstraint.minSize.Height(), DimensionUnit::PX)));

    return itemConstraint;
}
} // namespace OHOS::Ace::NG
