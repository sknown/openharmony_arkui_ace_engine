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

#include "core/components_ng/pattern/button/button_layout_algorithm.h"

#include "base/utils/utils.h"
#include "core/components/button/button_theme.h"
#include "core/components/toggle/toggle_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/components_ng/pattern/button/button_pattern.h"

namespace OHOS::Ace::NG {

void ButtonLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto layoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    HandleChildLayoutConstraint(layoutWrapper, layoutConstraint);
    auto buttonLayoutProperty = DynamicCast<ButtonLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(buttonLayoutProperty);
    if (buttonLayoutProperty->HasLabel()) {
        // If the button has label, according to whether the font size is set to do the corresponding expansion button,
        // font reduction, truncation and other operations.
        HandleAdaptiveText(layoutWrapper, layoutConstraint);
    } else {
        // If the button has not label, measure the child directly.
        for (auto&& child : layoutWrapper->GetAllChildrenWithBuild()) {
            child->Measure(layoutConstraint);
        }
    }
    PerformMeasureSelf(layoutWrapper);
    MarkNeedFlushMouseEvent(layoutWrapper);
}

void ButtonLayoutAlgorithm::HandleChildLayoutConstraint(
    LayoutWrapper* layoutWrapper, LayoutConstraintF& layoutConstraint)
{
    auto buttonLayoutProperty = DynamicCast<ButtonLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(buttonLayoutProperty);
    if (!buttonLayoutProperty->HasLabel()) {
        return;
    }
    if (buttonLayoutProperty->GetType().value_or(ButtonType::CAPSULE) == ButtonType::CIRCLE) {
        layoutConstraint.maxSize = HandleLabelCircleButtonConstraint(layoutWrapper).value_or(SizeF());
        return;
    }
    const auto& selfLayoutConstraint = layoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
    // If height is not set, apply the default height.
    if (selfLayoutConstraint && !selfLayoutConstraint->selfIdealSize.Height().has_value()) {
        auto buttonTheme = PipelineBase::GetCurrentContext()->GetTheme<ButtonTheme>();
        CHECK_NULL_VOID(buttonTheme);
        auto defaultHeight = GetDefaultHeight(layoutWrapper);
        auto maxHeight = selfLayoutConstraint->maxSize.Height();
        layoutConstraint.maxSize.SetHeight(maxHeight > defaultHeight ? defaultHeight : maxHeight);
    }
}

// If the ButtonType is CIRCLE, then omit text by the smaller edge.
std::optional<SizeF> ButtonLayoutAlgorithm::HandleLabelCircleButtonConstraint(LayoutWrapper* layoutWrapper)
{
    SizeF constraintSize;
    auto buttonLayoutProperty = DynamicCast<ButtonLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(buttonLayoutProperty, constraintSize);
    const auto& selfLayoutConstraint = layoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
    auto buttonTheme = PipelineBase::GetCurrentContext()->GetTheme<ButtonTheme>();
    CHECK_NULL_RETURN(buttonTheme, constraintSize);
    const auto& padding = buttonLayoutProperty->CreatePaddingAndBorder();
    auto defaultHeight = GetDefaultHeight(layoutWrapper);
    float minLength = 0.0f;
    if (selfLayoutConstraint->selfIdealSize.IsNull()) {
        // Width and height are not set.
        minLength = defaultHeight;
    } else if (selfLayoutConstraint->selfIdealSize.Width().has_value() &&
               !selfLayoutConstraint->selfIdealSize.Height().has_value()) {
        // Only width is set.
        minLength = selfLayoutConstraint->selfIdealSize.Width().value();
    } else if (selfLayoutConstraint->selfIdealSize.Height().has_value() &&
               !selfLayoutConstraint->selfIdealSize.Width().has_value()) {
        // Only height is set.
        minLength = selfLayoutConstraint->selfIdealSize.Height().value();
    } else {
        // Both width and height are set.
        auto buttonWidth = selfLayoutConstraint->selfIdealSize.Width().value();
        auto buttonHeight = selfLayoutConstraint->selfIdealSize.Height().value();
        minLength = std::min(buttonWidth, buttonHeight);
    }
    if (buttonLayoutProperty->HasBorderRadius() && selfLayoutConstraint->selfIdealSize.IsNull()) {
        auto radius =
            static_cast<float>(GetFirstValidRadius(buttonLayoutProperty->GetBorderRadius().value()).ConvertToPx());
        minLength = 2 * radius;
    }
    constraintSize.SetSizeT(SizeF(minLength, minLength));
    MinusPaddingToSize(padding, constraintSize);
    return ConstrainSize(constraintSize, selfLayoutConstraint->minSize, selfLayoutConstraint->maxSize);
}

void ButtonLayoutAlgorithm::HandleAdaptiveText(LayoutWrapper* layoutWrapper, LayoutConstraintF& layoutConstraint)
{
    auto buttonLayoutProperty = DynamicCast<ButtonLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(buttonLayoutProperty);
    auto buttonTheme = PipelineBase::GetCurrentContext()->GetTheme<ButtonTheme>();
    CHECK_NULL_VOID(buttonTheme);
    auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_VOID(childWrapper);
    auto childConstraint = layoutWrapper->GetLayoutProperty()->GetContentLayoutConstraint();
    childWrapper->Measure(childConstraint);
    auto textSize = childWrapper->GetGeometryNode()->GetContentSize();
    if (buttonLayoutProperty->HasFontSize() || buttonLayoutProperty->HasControlSize()) {
        // Fonsize is set. When the font height is larger than the button height, make the button fit the font
        // height.
        if (GreatOrEqual(textSize.Height(), layoutConstraint.maxSize.Height())) {
            layoutConstraint.maxSize.SetHeight(textSize.Height());
        }
    } else {
        // Fonsize is not set. When the font width is greater than the button width, dynamically change the font
        // size to no less than 9sp.
        auto textLayoutProperty = DynamicCast<TextLayoutProperty>(childWrapper->GetLayoutProperty());
        textLayoutProperty->UpdateAdaptMaxFontSize(
            buttonLayoutProperty->GetMaxFontSize().value_or(buttonTheme->GetMaxFontSize()));
        textLayoutProperty->UpdateAdaptMinFontSize(
            buttonLayoutProperty->GetMinFontSize().value_or(buttonTheme->GetMinFontSize()));
    }
    childWrapper->Measure(layoutConstraint);
    childSize_ = childWrapper->GetGeometryNode()->GetContentSize();
}

void ButtonLayoutAlgorithm::HandleBorderRadius(LayoutWrapper* layoutWrapper)
{
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto buttonLayoutProperty = DynamicCast<ButtonLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(buttonLayoutProperty);
    auto frameSize = layoutWrapper->GetGeometryNode()->GetFrameSize();
    auto renderContext = host->GetRenderContext();
    if (buttonLayoutProperty->GetType().value_or(ButtonType::CAPSULE) == ButtonType::CIRCLE) {
        auto minSize = std::min(frameSize.Height(), frameSize.Width());
        auto layoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
        if (buttonLayoutProperty->HasBorderRadius() && layoutConstraint.parentIdealSize.IsNull()) {
            auto borderRadius = buttonLayoutProperty->GetBorderRadius().value_or(NG::BorderRadiusProperty());
            minSize = static_cast<float>(GetFirstValidRadius(borderRadius).ConvertToPx() * 2);
        }
        renderContext->UpdateBorderRadius(BorderRadiusProperty(Dimension(minSize / 2)));
        MeasureCircleButton(layoutWrapper);
    } else if (buttonLayoutProperty->GetType().value_or(ButtonType::CAPSULE) == ButtonType::CAPSULE) {
        renderContext->UpdateBorderRadius(BorderRadiusProperty(Dimension(frameSize.Height() / 2)));
    } else {
        auto normalRadius =
            buttonLayoutProperty->GetBorderRadiusValue(BorderRadiusProperty({ 0.0_vp, 0.0_vp, 0.0_vp, 0.0_vp }));
        renderContext->UpdateBorderRadius(normalRadius);
    }
}

// Called to perform measure current render node.
void ButtonLayoutAlgorithm::PerformMeasureSelf(LayoutWrapper* layoutWrapper)
{
    auto buttonLayoutProperty = DynamicCast<ButtonLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(buttonLayoutProperty);
    BoxLayoutAlgorithm::PerformMeasureSelf(layoutWrapper);
    if (buttonLayoutProperty->HasLabel()) {
        auto frameSize = layoutWrapper->GetGeometryNode()->GetFrameSize();
        auto layoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
        const auto& selfLayoutConstraint = layoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
        auto padding = buttonLayoutProperty->CreatePaddingAndBorder();
        auto topPadding = padding.top.value_or(0.0);
        auto bottomPadding = padding.bottom.value_or(0.0);
        auto buttonTheme = PipelineBase::GetCurrentContext()->GetTheme<ButtonTheme>();
        CHECK_NULL_VOID(buttonTheme);

        if(buttonLayoutProperty->GetButtonStyle().value_or(ButtonStyleMode::EMPHASIZE) == ButtonStyleMode::TEXT) {
            if(buttonLayoutProperty->GetControlSize().value_or(ControlSize::NORMAL) == ControlSize::SMALL
                || buttonLayoutProperty->GetControlSize().value_or(ControlSize::NORMAL) == ControlSize::NORMAL) {
                padding.left = buttonTheme->GetPaddingText().ConvertToPx();
                padding.right = buttonTheme->GetPaddingText().ConvertToPx();
            }
            PaddingProperty defaultPadding = { CalcLength(padding.left.value_or(0)), CalcLength(padding.right.value_or(0)),
            CalcLength(padding.top.value_or(0)), CalcLength(padding.bottom.value_or(0)) };
            layoutWrapper->GetLayoutProperty()->UpdatePadding(defaultPadding);
        }
        else {
            if(buttonLayoutProperty->GetControlSize().value_or(ControlSize::NORMAL) == ControlSize::SMALL) {
                padding.left = buttonTheme->GetPadding(ControlSize::SMALL).Left().ConvertToPx();
                padding.right = buttonTheme->GetPadding(ControlSize::SMALL).Right().ConvertToPx();
            }
            else {
                padding.left = buttonTheme->GetPadding(ControlSize::NORMAL).Left().ConvertToPx();
                padding.right = buttonTheme->GetPadding(ControlSize::NORMAL).Right().ConvertToPx();
            }
            PaddingProperty defaultPadding = { CalcLength(padding.left.value_or(0)), CalcLength(padding.right.value_or(0)),
            CalcLength(padding.top.value_or(0)), CalcLength(padding.bottom.value_or(0)) };
            layoutWrapper->GetLayoutProperty()->UpdatePadding(defaultPadding);
        }

        auto defaultHeight = GetDefaultHeight(layoutWrapper);
        if (buttonLayoutProperty->GetType().value_or(ButtonType::CAPSULE) == ButtonType::CIRCLE) {
            HandleLabelCircleButtonFrameSize(layoutConstraint, frameSize, defaultHeight);
        } else {
            if (selfLayoutConstraint && !selfLayoutConstraint->selfIdealSize.Height().has_value()) {
                auto layoutContraint = buttonLayoutProperty->GetLayoutConstraint();
                CHECK_NULL_VOID(layoutContraint);
                auto maxHeight = layoutContraint->maxSize.Height();
                auto actualHeight = static_cast<float>(childSize_.Height() + topPadding + bottomPadding);
                actualHeight = std::min(actualHeight, maxHeight);
                frameSize.SetHeight(maxHeight > defaultHeight ? std::max(defaultHeight, actualHeight) : maxHeight);
            }
        }
        // Determine if the button needs to fit the font size.
        if (buttonLayoutProperty->HasFontSize()) {
            if (GreatOrEqual(childSize_.Height() + topPadding + bottomPadding, frameSize.Height())) {
                frameSize = SizeF(frameSize.Width(), childSize_.Height() + topPadding + bottomPadding);
            }
        }
        layoutWrapper->GetGeometryNode()->SetFrameSize(frameSize);
    }
    HandleBorderRadius(layoutWrapper);
}

void ButtonLayoutAlgorithm::HandleLabelCircleButtonFrameSize(
    const LayoutConstraintF& layoutConstraint, SizeF& frameSize, const float& defaultHeight)
{
    float minLength = 0.0f;
    if (layoutConstraint.parentIdealSize.IsNull()) {
        minLength = static_cast<float>(defaultHeight);
    } else if (layoutConstraint.parentIdealSize.Width().has_value() &&
               !layoutConstraint.parentIdealSize.Height().has_value()) {
        minLength = frameSize.Width();
    } else if (layoutConstraint.parentIdealSize.Height().has_value() &&
               !layoutConstraint.parentIdealSize.Width().has_value()) {
        minLength = frameSize.Height();
    } else {
        minLength = std::min(frameSize.Width(), frameSize.Height());
    }
    frameSize.SetWidth(minLength);
    frameSize.SetHeight(minLength);
}

void ButtonLayoutAlgorithm::MeasureCircleButton(LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    const auto& radius = frameNode->GetRenderContext()->GetBorderRadius();
    SizeF frameSize = { -1, -1 };
    if (radius.has_value()) {
        auto radiusTopMax = std::max(radius->radiusTopLeft, radius->radiusTopRight);
        auto radiusBottomMax = std::max(radius->radiusBottomLeft, radius->radiusBottomRight);
        auto radiusMax = std::max(radiusTopMax, radiusBottomMax);
        auto rrectRadius = radiusMax.value_or(0.0_vp).ConvertToPx();
        frameSize.SetSizeT(SizeF { static_cast<float>(rrectRadius * 2), static_cast<float>(rrectRadius * 2) });
    }
    frameSize.UpdateIllegalSizeWithCheck(SizeF { 0.0f, 0.0f });
    layoutWrapper->GetGeometryNode()->SetFrameSize(frameSize);
}

Dimension ButtonLayoutAlgorithm::GetFirstValidRadius(const BorderRadiusProperty& borderRadius)
{
    if (borderRadius.radiusTopLeft.has_value()) {
        return borderRadius.radiusTopLeft.value();
    }
    if (borderRadius.radiusTopRight.has_value()) {
        return borderRadius.radiusTopRight.value();
    }
    if (borderRadius.radiusBottomLeft.has_value()) {
        return borderRadius.radiusBottomLeft.value();
    }
    if (borderRadius.radiusBottomRight.has_value()) {
        return borderRadius.radiusBottomRight.value();
    }
    return 0.0_vp;
}

float ButtonLayoutAlgorithm::GetDefaultHeight(LayoutWrapper* layoutWrapper)
{
    auto layoutProperty = DynamicCast<ButtonLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, 0.0);
    auto buttonTheme = PipelineBase::GetCurrentContext()->GetTheme<ButtonTheme>();
    CHECK_NULL_RETURN(buttonTheme, 0.0);
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, 0.0);
    if (frameNode->GetTag() == V2::TOGGLE_ETS_TAG) {
        auto toggleTheme = PipelineBase::GetCurrentContext()->GetTheme<ToggleTheme>();
        CHECK_NULL_RETURN(toggleTheme, 0.0);
        return static_cast<float>(toggleTheme->GetButtonHeight().ConvertToPx());
    }
    ControlSize controlSize = layoutProperty->GetControlSize().value_or(ControlSize::NORMAL);
    return static_cast<float>(buttonTheme->GetHeight(controlSize).ConvertToPx());
}

void ButtonLayoutAlgorithm::MarkNeedFlushMouseEvent(LayoutWrapper* layoutWrapper)
{
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto pattern = host->GetPattern<ButtonPattern>();
    CHECK_NULL_VOID(pattern);
    auto frameSize = layoutWrapper->GetGeometryNode()->GetFrameSize();
    if (frameSize != pattern->GetPreFrameSize()) {
        pattern->SetPreFrameSize(frameSize);
        auto context = PipelineContext::GetCurrentContext();
        context->MarkNeedFlushMouseEvent();
    }
}
} // namespace OHOS::Ace::NG
