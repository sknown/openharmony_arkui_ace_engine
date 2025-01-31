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

#include "core/components_ng/pattern/security_component/security_component_layout_algorithm.h"

#include "base/log/ace_scoring_log.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_render_property.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "unicode/uchar.h"

namespace {
constexpr float HALF = 2.0f;
}

namespace OHOS::Ace::NG {
RefPtr<LayoutWrapper> SecurityComponentLayoutAlgorithm::GetChildWrapper(LayoutWrapper* layoutWrapper,
    const std::string& tag)
{
    int32_t count = layoutWrapper->GetTotalChildCount();
    for (int32_t i = 0; i < count; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(i);
        if (childWrapper == nullptr) {
            continue;
        }
        if (childWrapper->GetHostTag() == tag) {
            return childWrapper;
        }
    }
    return nullptr;
}

void SecurityComponentLayoutAlgorithm::UpdateChildPosition(LayoutWrapper* layoutWrapper, const std::string& tag,
    OffsetF& offset)
{
    auto childWrapper = GetChildWrapper(layoutWrapper, tag);
    CHECK_NULL_VOID(childWrapper);
    auto childNode = childWrapper->GetHostNode();
    CHECK_NULL_VOID(childNode);
    childNode->GetGeometryNode()->SetMarginFrameOffset(
        OffsetF(std::round(offset.GetX()), std::round(offset.GetY())));
}

static LayoutConstraintF CreateDefaultChildConstraint(
    RefPtr<SecurityComponentLayoutProperty>& securityComponentProperty)
{
    auto constraint = securityComponentProperty->CreateChildConstraint();
    SizeT<float> maxSize { Infinity<float>(), Infinity<float>() };
    constraint.maxSize = maxSize;
    return constraint;
}

void SecurityComponentLayoutAlgorithm::MeasureButton(LayoutWrapper* layoutWrapper,
    RefPtr<SecurityComponentLayoutProperty>& securityComponentProperty)
{
    auto buttonWrapper = GetChildWrapper(layoutWrapper, V2::BUTTON_ETS_TAG);
    CHECK_NULL_VOID(buttonWrapper);
    auto buttonLayoutProperty = DynamicCast<ButtonLayoutProperty>(buttonWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(buttonLayoutProperty);
    auto buttonConstraint = CreateDefaultChildConstraint(securityComponentProperty);
    if (securityComponentProperty->GetBackgroundType() == static_cast<int32_t>(ButtonType::CIRCLE)) {
        buttonConstraint.selfIdealSize.SetSize(SizeF(std::max(componentWidth_, componentHeight_),
            std::max(componentWidth_, componentHeight_)));
        if (GreatNotEqual(componentWidth_, componentHeight_)) {
            top_.EnlargeHeight((componentWidth_ / HALF) - (componentHeight_ / HALF));
        } else if (GreatNotEqual(componentHeight_, componentWidth_)) {
            left_.EnlargeWidth((componentHeight_ / HALF) - (componentWidth_ / HALF));
        }
        componentWidth_ = componentHeight_ = std::max(componentWidth_, componentHeight_);
    } else {
        buttonConstraint.selfIdealSize.SetSize(SizeF(componentWidth_, componentHeight_));
    }

    buttonWrapper->Measure(std::optional<LayoutConstraintF>(buttonConstraint));
}

void SecurityComponentLayoutAlgorithm::InitPadding(RefPtr<SecurityComponentLayoutProperty>& property)
{
    auto theme = PipelineContext::GetCurrentContext()->GetTheme<SecurityComponentTheme>();
    CHECK_NULL_VOID(theme);

    double borderWidth = property->GetBackgroundBorderWidth().value_or(Dimension(0.0)).ConvertToPx();
    double size = property->GetBackgroundLeftPadding().value_or(theme->GetBackgroundLeftPadding()).ConvertToPx() +
        borderWidth;
    left_.Init(false,
        property->GetBackgroundLeftPadding().has_value(), size, borderWidth);

    size = property->GetBackgroundTopPadding().value_or(theme->GetBackgroundTopPadding()).ConvertToPx() +
        borderWidth;
    top_.Init(true,
        property->GetBackgroundTopPadding().has_value(), size, borderWidth);

    size = property->GetBackgroundRightPadding().value_or(theme->GetBackgroundRightPadding()).ConvertToPx() +
        borderWidth;
    right_.Init(false,
        property->GetBackgroundRightPadding().has_value(), size, borderWidth);

    size = property->GetBackgroundBottomPadding().value_or(theme->GetBackgroundBottomPadding()).ConvertToPx() +
        borderWidth;
    bottom_.Init(true,
        property->GetBackgroundBottomPadding().has_value(), size, borderWidth);

    size = property->GetTextIconSpace().value_or(theme->GetTextIconSpace()).ConvertToPx();
    middle_.Init(isVertical_, property->GetTextIconSpace().has_value(), size, 0.0);
}

double SecurityComponentLayoutAlgorithm::ShrinkWidth(double diff)
{
    // first shrink left and right padding
    double remain = left_.ShrinkWidth(diff / HALF);
    remain = right_.ShrinkWidth(remain + (diff / HALF));
    remain = left_.ShrinkWidth(remain);
    if (NearEqual(remain, 0.0)) {
        MeasureIntegralSize();
        return componentWidth_;
    }

    // if horizontal shrink IconTextSpace
    remain = middle_.ShrinkWidth(remain);
    if (NearEqual(remain, 0.0)) {
        MeasureIntegralSize();
        return componentWidth_;
    }

    double iconWidth = icon_.width_;
    double textWidth = text_.width_;
    if (isVertical_) {
        // Shrink max width, then shrink another proportionally if vertical
        if (GreatNotEqual(textWidth, iconWidth)) {
            double textRemain = text_.ShrinkWidth(remain);
            double iconRemain = (remain - textRemain) * iconWidth / textWidth;
            icon_.ShrinkWidth(iconRemain);
        } else {
            double iconRemain = icon_.ShrinkWidth(remain);
            double textRemain = (remain - iconRemain) * textWidth / iconWidth;
            text_.ShrinkWidth(textRemain);
        }
    } else {
        // Shrink proportional text and icon if horizontal
        double iconRemain = iconWidth * remain / (iconWidth + textWidth);
        double textRemain = textWidth * remain / (iconWidth + textWidth);
        double resIcon = icon_.ShrinkWidth(iconRemain);
        double resText = text_.ShrinkWidth(textRemain);
        if (!NearEqual(resIcon, 0.0)) {
            text_.ShrinkWidth(resIcon);
        } else if (!NearEqual(resText, 0.0)) {
            icon_.ShrinkWidth(resText);
        }
    }
    MeasureIntegralSize();
    return componentWidth_;
}

double SecurityComponentLayoutAlgorithm::EnlargeWidth(double diff)
{
    double remain = left_.EnlargeWidth(diff / HALF);
    remain = right_.EnlargeWidth(remain + (diff / HALF));
    remain = left_.EnlargeWidth(remain);
    if (GreatNotEqual(remain, 0.0) && !isVertical_) {
        middle_.EnlargeWidth(remain);
    }
    MeasureIntegralSize();
    return componentWidth_;
}

double SecurityComponentLayoutAlgorithm::ShrinkHeight(double diff)
{
    // first shrink left and right padding
    double remain = top_.ShrinkHeight(diff / HALF);
    remain = bottom_.ShrinkHeight(remain + (diff / HALF));
    remain = top_.ShrinkHeight(remain);
    if (NearEqual(remain, 0.0)) {
        MeasureIntegralSize();
        return componentHeight_;
    }

    // if vertical shrink IconTextSpace
    remain = middle_.ShrinkHeight(remain);
    if (NearEqual(remain, 0.0)) {
        MeasureIntegralSize();
        return componentHeight_;
    }

    double iconHeight = icon_.height_;
    double textHeight = text_.height_;
    if (!isVertical_) {
         // Shrink max width, then shrink another proportionally if horizontal
        if (GreatNotEqual(textHeight, iconHeight)) {
            double textRemain = text_.ShrinkHeight(remain);
            double iconRemain = (remain - textRemain) * iconHeight / textHeight;
            icon_.ShrinkHeight(iconRemain);
        } else {
            double iconRemain = icon_.ShrinkHeight(remain);
            double textRemain = (remain - iconRemain) * textHeight / iconHeight;
            text_.ShrinkHeight(textRemain);
        }
    } else {
        double iconRemain = iconHeight * remain / (iconHeight + textHeight);
        double textRemain = textHeight * remain / (iconHeight + textHeight);
        double resIcon = icon_.ShrinkHeight(iconRemain);
        double resText = text_.ShrinkHeight(textRemain);
        if (!NearEqual(resIcon, 0.0)) {
            text_.ShrinkHeight(resIcon);
        } else if (!NearEqual(resText, 0.0)) {
            icon_.ShrinkHeight(resText);
        }
    }
    isNeedReadaptWidth_ = true;
    MeasureIntegralSize();
    return componentHeight_;
}

double SecurityComponentLayoutAlgorithm::EnlargeHeight(double diff)
{
    double remain = top_.EnlargeHeight(diff / HALF);
    remain = bottom_.EnlargeHeight(remain + (diff / HALF));
    remain = top_.EnlargeHeight(remain);
    if (GreatNotEqual(remain, 0.0) && isVertical_) {
        middle_.EnlargeHeight(remain);
    }
    MeasureIntegralSize();
    return componentHeight_;
}

void SecurityComponentLayoutAlgorithm::AdaptWidth()
{
    if (idealWidth_ != 0.0) {
        if (componentWidth_ > idealWidth_) {
            ShrinkWidth(componentWidth_ - idealWidth_);
        } else if (componentWidth_ < idealWidth_) {
            EnlargeWidth(idealWidth_ - componentWidth_);
        }
        return;
    }

    if (componentWidth_ > maxWidth_) {
        ShrinkWidth(componentWidth_ - maxWidth_);
    } else if (componentWidth_ < minWidth_) {
        EnlargeWidth(minWidth_ - componentWidth_);
    }
}

void SecurityComponentLayoutAlgorithm::AdaptHeight()
{
    if (idealHeight_ != 0.0) {
        if (componentHeight_ > idealHeight_) {
            ShrinkHeight(componentHeight_ - idealHeight_);
        } else if (componentHeight_ < idealHeight_) {
            EnlargeHeight(idealHeight_ - componentHeight_);
        }
        return;
    }
    if (componentHeight_ > maxHeight_) {
        ShrinkHeight(componentHeight_ - maxHeight_);
    } else if (componentHeight_ < minHeight_) {
        EnlargeHeight(minHeight_ - componentHeight_);
    }
}

void SecurityComponentLayoutAlgorithm::MeasureIntegralSize()
{
    if (isVertical_) {
        double contextWidth = std::max(text_.width_, icon_.width_);
        componentHeight_ = top_.height_ + text_.height_ +
            middle_.height_ + icon_.height_ + bottom_.height_;
        componentWidth_ = left_.width_ + contextWidth + right_.width_;
    } else {
        double contextHeight = std::max(text_.height_, icon_.height_);
        componentHeight_ = top_.height_ + contextHeight + bottom_.height_;
        componentWidth_ = left_.width_ + icon_.width_ +
            middle_.width_ + text_.width_ + right_.width_;
    }
}

void SecurityComponentLayoutAlgorithm::UpdateVerticalOffset(OffsetF& offsetIcon,
    OffsetF& offsetText)
{
    offsetText = offsetIcon + OffsetF(0.0, icon_.height_ + middle_.height_);
    if (icon_.width_ > text_.width_) {
        offsetText += OffsetF((icon_.width_ - text_.width_) / HALF, 0.0);
    } else {
        offsetIcon += OffsetF((text_.width_ - icon_.width_) / HALF, 0.0);
    }
}

void SecurityComponentLayoutAlgorithm::UpdateHorizontalOffset(LayoutWrapper* layoutWrapper,
    OffsetF& offsetIcon, OffsetF& offsetText)
{
    if (GetTextDirection(layoutWrapper) == TextDirection::RTL) {
        offsetIcon = offsetText +
            OffsetF(text_.width_ + middle_.width_, 0.0);
    } else {
        offsetText = offsetIcon +
            OffsetF(icon_.width_ + middle_.width_, 0.0);
    }
    if (icon_.height_ > text_.height_) {
        offsetText +=
            OffsetF(0.0, (icon_.height_ - text_.height_) / HALF);
    } else {
        offsetIcon +=
            OffsetF(0.0, (text_.height_ - icon_.height_) / HALF);
    }
}

void SecurityComponentLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    OffsetF offsetIcon = OffsetF(left_.width_, top_.height_);
    OffsetF offsetText = OffsetF(left_.width_, top_.height_);
    if (isVertical_) {
        UpdateVerticalOffset(offsetIcon, offsetText);
    } else {
        UpdateHorizontalOffset(layoutWrapper, offsetIcon, offsetText);
    }

    UpdateChildPosition(layoutWrapper, V2::IMAGE_ETS_TAG, offsetIcon);
    UpdateChildPosition(layoutWrapper, V2::TEXT_ETS_TAG, offsetText);

    for (auto&& child : layoutWrapper->GetAllChildrenWithBuild()) {
        child->Layout();
    }
}

void SecurityComponentLayoutAlgorithm::UpdateCircleButtonConstraint()
{
    double circleIdealSize = std::max(componentWidth_, componentHeight_);
    if ((idealWidth_ != 0.0) && (idealHeight_ != 0.0)) {
        circleIdealSize = std::min(idealWidth_, idealHeight_);
    } else if (idealWidth_ != 0.0) {
        circleIdealSize = idealWidth_;
    } else if (idealHeight_ != 0.0) {
        circleIdealSize = idealHeight_;
    } else {
        if ((componentWidth_ < minWidth_) || (componentHeight_ < minHeight_)) {
            circleIdealSize = std::max(minWidth_, minHeight_);
        } else if ((componentWidth_ > maxWidth_) || (componentHeight_ > maxHeight_)) {
            circleIdealSize = std::min(maxWidth_, maxHeight_);
        }
    }
    idealWidth_ = idealHeight_ = circleIdealSize;
}

void SecurityComponentLayoutAlgorithm::FillBlank()
{
    if (isNobg_) {
        return;
    }
    if (GreatNotEqual(idealWidth_, componentWidth_)) {
        left_.width_ += ((idealWidth_ - componentWidth_) / HALF);
        right_.width_ += ((idealWidth_ - componentWidth_) / HALF);
    } else if (GreatNotEqual(minWidth_, componentWidth_)) {
        left_.width_ += ((minWidth_ - componentWidth_) / HALF);
        right_.width_ += ((minWidth_ - componentWidth_) / HALF);
    }
    if (GreatNotEqual(idealHeight_, componentHeight_)) {
        top_.height_ += ((idealHeight_ - componentHeight_) / HALF);
        bottom_.height_ += ((idealHeight_ - componentHeight_) / HALF);
    } else if (GreatNotEqual(minHeight_, componentHeight_)) {
        top_.height_ += ((minHeight_ - componentHeight_) / HALF);
        bottom_.height_ += ((minHeight_ - componentHeight_) / HALF);
    }
    MeasureIntegralSize();
}

void SecurityComponentLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto securityComponentLayoutProperty =
        AceType::DynamicCast<SecurityComponentLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(securityComponentLayoutProperty);

    auto iconWrapper = GetChildWrapper(layoutWrapper, V2::IMAGE_ETS_TAG);
    icon_.Init(securityComponentLayoutProperty, iconWrapper);

    auto textWrapper = GetChildWrapper(layoutWrapper, V2::TEXT_ETS_TAG);
    text_.Init(securityComponentLayoutProperty, textWrapper);

    constraint_ = securityComponentLayoutProperty->GetContentLayoutConstraint();
    CHECK_NULL_VOID(constraint_);
    isVertical_ = (securityComponentLayoutProperty->GetTextIconLayoutDirection().value() ==
        SecurityComponentLayoutDirection::VERTICAL);
    isNobg_ = (securityComponentLayoutProperty->GetBackgroundType().value() == BUTTON_TYPE_NULL);
    idealWidth_ = constraint_->selfIdealSize.Width().value_or(0.0);
    idealHeight_ = constraint_->selfIdealSize.Height().value_or(0.0);
    minWidth_ = constraint_->minSize.Width();
    minHeight_ = constraint_->minSize.Height();
    maxWidth_ = constraint_->maxSize.Width();
    maxHeight_ = constraint_->maxSize.Height();
    InitPadding(securityComponentLayoutProperty);

    MeasureIntegralSize();

    if (securityComponentLayoutProperty->GetBackgroundType() == static_cast<int32_t>(ButtonType::CIRCLE)) {
        UpdateCircleButtonConstraint();
    }
    AdaptWidth();
    AdaptHeight();
    if (isNeedReadaptWidth_) {
        AdaptWidth();
    }
    // fill blank when all paddings can not be enlarged because it has been set
    FillBlank();

    icon_.DoMeasure();
    MeasureButton(layoutWrapper, securityComponentLayoutProperty);
    layoutWrapper->GetGeometryNode()->SetFrameSize(SizeF(componentWidth_, componentHeight_));
}

TextDirection SecurityComponentLayoutAlgorithm::GetTextDirection(LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    // default return LTR
    CHECK_NULL_RETURN(frameNode, TextDirection::LTR);
    std::string text = "";
    // get button string
    for (const auto& child : frameNode->GetChildren()) {
        auto node = AceType::DynamicCast<FrameNode, UINode>(child);
        if (node == nullptr) {
            continue;
        }
        if (node->GetTag() == V2::TEXT_ETS_TAG) {
            auto textLayoutProperty = node->GetLayoutProperty<TextLayoutProperty>();
            if (textLayoutProperty == nullptr) {
                continue;
            }
            text = textLayoutProperty->GetContentValue(text);
            break;
        }
    }
    if (text.empty()) {
        return TextDirection::LTR;
    }
    auto wString = StringUtils::ToWstring(text);
    for (const auto& charInStr : wString) {
        auto direction = u_charDirection(charInStr);
        if (direction == UCharDirection::U_LEFT_TO_RIGHT) {
            return TextDirection::LTR;
        }
        if (direction == UCharDirection::U_RIGHT_TO_LEFT || direction == UCharDirection::U_RIGHT_TO_LEFT_ARABIC) {
            return TextDirection::RTL;
        }
    }
    return TextDirection::LTR;
}
} // namespace OHOS::Ace::NG
