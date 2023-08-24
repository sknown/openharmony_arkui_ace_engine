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

#include "core/components_ng/pattern/menu/menu_model_ng.h"

#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_stack_processor.h"

namespace OHOS::Ace::NG {
void MenuModelNG::Create()
{
    LOGI("MenuView::Create");
    auto* stack = ViewStackProcessor::GetInstance();
    int32_t nodeId = (stack == nullptr ? 0 : stack->ClaimNodeId());
    auto menuNode = FrameNode::GetOrCreateFrameNode(V2::MENU_ETS_TAG, nodeId,
        []() { return AceType::MakeRefPtr<InnerMenuPattern>(-1, V2::MENU_ETS_TAG, MenuType::MULTI_MENU); });
    CHECK_NULL_VOID(menuNode);
    ViewStackProcessor::GetInstance()->Push(menuNode);

    auto layoutProps = menuNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProps);
    // default min width
    layoutProps->UpdateCalcMinSize(CalcSize(CalcLength(MIN_MENU_WIDTH), std::nullopt));
}

void MenuModelNG::SetFontSize(const Dimension& fontSize)
{
    if (fontSize.IsValid()) {
        ACE_UPDATE_LAYOUT_PROPERTY(MenuLayoutProperty, FontSize, fontSize);
    } else {
        LOGW("FontSize value is not valid");
        ACE_RESET_LAYOUT_PROPERTY(MenuLayoutProperty, FontSize);
    }
}

void MenuModelNG::SetFontWeight(FontWeight weight)
{
    ACE_UPDATE_LAYOUT_PROPERTY(MenuLayoutProperty, FontWeight, weight);
}

void MenuModelNG::SetFontStyle(Ace::FontStyle style)
{
    ACE_UPDATE_LAYOUT_PROPERTY(MenuLayoutProperty, ItalicFontStyle, style);
}

void MenuModelNG::SetFontColor(const std::optional<Color>& color)
{
    if (color.has_value()) {
        ACE_UPDATE_LAYOUT_PROPERTY(MenuLayoutProperty, FontColor, color.value());
    } else {
        ACE_RESET_LAYOUT_PROPERTY(MenuLayoutProperty, FontColor);
    }
}

void MenuModelNG::SetBorderRadius(const Dimension& radius)
{
    NG::BorderRadiusProperty borderRadius;
    borderRadius.radiusTopLeft = radius;
    borderRadius.radiusTopRight = radius;
    borderRadius.radiusBottomLeft = radius;
    borderRadius.radiusBottomRight = radius;
    borderRadius.multiValued = true;
    ACE_UPDATE_LAYOUT_PROPERTY(MenuLayoutProperty, BorderRadius, borderRadius);
}

void MenuModelNG::SetBorderRadius(const std::optional<Dimension>& radiusTopLeft,
    const std::optional<Dimension>& radiusTopRight, const std::optional<Dimension>& radiusBottomLeft,
    const std::optional<Dimension>& radiusBottomRight)
{
    NG::BorderRadiusProperty borderRadius;
    borderRadius.radiusTopLeft = radiusTopLeft;
    borderRadius.radiusTopRight = radiusTopRight;
    borderRadius.radiusBottomLeft = radiusBottomLeft;
    borderRadius.radiusBottomRight = radiusBottomRight;
    borderRadius.multiValued = true;
    ACE_UPDATE_LAYOUT_PROPERTY(MenuLayoutProperty, BorderRadius, borderRadius);
}

void MenuModelNG::SetWidth(const Dimension& width)
{
    ACE_UPDATE_LAYOUT_PROPERTY(MenuLayoutProperty, MenuWidth, width);
    ViewAbstract::SetWidth(NG::CalcLength(width));
}

void MenuModelNG::SetFontFamily(const std::vector<std::string>& families)
{
    ACE_UPDATE_LAYOUT_PROPERTY(MenuLayoutProperty, FontFamily, families);
}
} // namespace OHOS::Ace::NG
