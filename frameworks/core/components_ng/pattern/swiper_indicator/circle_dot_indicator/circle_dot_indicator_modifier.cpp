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

#include "core/components_ng/pattern/swiper_indicator/circle_dot_indicator/circle_dot_indicator_modifier.h"

#include <cmath>

#include "base/utils/utils.h"
#include "core/animation/spring_curve.h"
#include "core/components_ng/render/animation_utils.h"
#include "core/components_ng/render/drawing.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t COMPONENT_DILATE_ANIMATION_DURATION = 250;
constexpr int32_t COMPONENT_SHRINK_ANIMATION_DURATION = 300;

constexpr float BLACK_POINT_CENTER_BEZIER_CURVE_VELOCITY = 0.4f;
constexpr float HEAD_POINT_CENTER_BEZIER_CURVE_VELOCITY = 1.0f;
constexpr float TAIL_POINT_CENTER_BEZIER_CURVE_VELOCITY = 0.2f;
constexpr float CENTER_BEZIER_CURVE_MASS = 0.0f;
constexpr float CENTER_BEZIER_CURVE_STIFFNESS = 1.0f;
constexpr float CENTER_BEZIER_CURVE_DAMPING = 1.0f;

constexpr int32_t BLACK_POINT_DURATION = 400;
constexpr int32_t LONG_POINT_DURATION = 400;

constexpr uint32_t ITEM_RADIUS = 0;
constexpr uint32_t SELECTED_ITEM_RADIUS = 1;
constexpr uint32_t CONTAINER_RADIUS = 2;
constexpr uint32_t ITEM_PADDING = 3;
constexpr uint32_t SELECTED_ITEM_PADDING = 4;
constexpr uint32_t ACTIVE_ITEM_ANGLE = 5;
constexpr Dimension MASK_HEIGHT = 36.0_vp;
constexpr double QUARTER_CIRCLE_ANGLE = 90.0;
constexpr double HALF_CIRCLE_ANGLE = 180.0;
constexpr int32_t ITEM_TWO_NUM = 2;
constexpr int32_t ITEM_THREE_NUM = 3;

constexpr float DEFAULT_DOT_PADDING_ANGLE = 5.0f;
constexpr float DEFAULT_DOT_ACTIVE_PADDING_ANGLE = 7.0f;
constexpr float DEFAULT_DOT_ACTIVE_ANGLE = 4.0f;
constexpr Dimension DEFAULT_CONTAINER_BORDER_WIDTH = 16.0_vp;
constexpr Dimension DEFAULT_ITEM_RADIUS = 2.5_vp;
constexpr Dimension DEFAULT_ITEM_SELECT_WIDTH = 5.0_vp;
} // namespace

CircleDotIndicatorModifier::CircleDotIndicatorModifier()
    : backgroundColor_(AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor::TRANSPARENT)),
      vectorBlackPointAngle_(AceType::MakeRefPtr<AnimatablePropertyVectorFloat>(LinearVector<float>(0))),
      unselectedColor_(AceType::MakeRefPtr<PropertyColor>(Color::TRANSPARENT)),
      selectedColor_(AceType::MakeRefPtr<PropertyColor>(Color::TRANSPARENT)),
      dotPaddingAngle_(AceType::MakeRefPtr<AnimatablePropertyFloat>(DEFAULT_DOT_PADDING_ANGLE)),
      dotActiveAngle_(AceType::MakeRefPtr<AnimatablePropertyFloat>(DEFAULT_DOT_ACTIVE_ANGLE)),
      dotActiveStartAngle_(AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0)),
      dotActiveEndAngle_(AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0)),
      dotActivePaddingAngle_(AceType::MakeRefPtr<AnimatablePropertyFloat>(DEFAULT_DOT_ACTIVE_PADDING_ANGLE)),
      containerBorderWidth_(AceType::MakeRefPtr<AnimatablePropertyFloat>(DEFAULT_CONTAINER_BORDER_WIDTH.ConvertToPx())),
      itemRadius_(AceType::MakeRefPtr<AnimatablePropertyFloat>(DEFAULT_ITEM_RADIUS.ConvertToPx())),
      itemSelectWidth_(AceType::MakeRefPtr<AnimatablePropertyFloat>(DEFAULT_ITEM_SELECT_WIDTH.ConvertToPx())),
      updateMaskColor_(AceType::MakeRefPtr<PropertyBool>(false))
{
    AttachProperty(backgroundColor_);
    AttachProperty(vectorBlackPointAngle_);
    AttachProperty(unselectedColor_);
    AttachProperty(selectedColor_);
    AttachProperty(dotPaddingAngle_);
    AttachProperty(dotActiveAngle_);
    AttachProperty(dotActiveStartAngle_);
    AttachProperty(dotActiveEndAngle_);
    AttachProperty(dotActivePaddingAngle_);
    AttachProperty(containerBorderWidth_);
    AttachProperty(itemRadius_);
    AttachProperty(itemSelectWidth_);
    AttachProperty(updateMaskColor_);
}

void CircleDotIndicatorModifier::onDraw(DrawingContext& context)
{
    ContentProperty contentProperty;
    contentProperty.backgroundColor = backgroundColor_->Get().ToColor();
    contentProperty.vectorBlackPointAngle = vectorBlackPointAngle_->Get();
    contentProperty.dotPaddingAngle = dotPaddingAngle_->Get();
    contentProperty.dotActiveAngle = dotActiveAngle_->Get();
    contentProperty.dotActiveStartAngle = dotActiveStartAngle_->Get();
    contentProperty.dotActiveEndAngle = dotActiveEndAngle_->Get();
    contentProperty.dotActivePaddingAngle = dotActivePaddingAngle_->Get();
    contentProperty.containerBorderWidth = containerBorderWidth_->Get();
    contentProperty.itemRadius = itemRadius_->Get();
    contentProperty.itemSelectWidth = itemSelectWidth_->Get();

    PaintIndicatorMask(context, contentProperty);
    PaintBackground(context, contentProperty);
    PaintContent(context, contentProperty);
}

void CircleDotIndicatorModifier::PaintBackground(DrawingContext& context, const ContentProperty& contentProperty)
{
    RSCanvas& canvas = context.canvas;
    CHECK_NULL_VOID(contentProperty.backgroundColor.GetAlpha());
    float dotPaddingAngle = contentProperty.dotPaddingAngle;
    float dotActivePaddingAngle = contentProperty.dotActivePaddingAngle;
    float containerBorderWidth = contentProperty.containerBorderWidth;
    float dotActiveAngle = contentProperty.dotActiveAngle;
    auto itemSize = contentProperty.vectorBlackPointAngle.size();
    float allPointArcAngle = 0;
    // The number 2 represents multiplying by 2 times or dividing equally
    if (currentIndex_ == 0 || currentIndex_ == itemSize - 1) {
        if (itemSize >= ITEM_TWO_NUM) {
            allPointArcAngle = (itemSize - ITEM_TWO_NUM) * dotPaddingAngle + dotActivePaddingAngle + dotActiveAngle / 2;
        } else {
            allPointArcAngle = dotActiveAngle;
        }
    } else {
        allPointArcAngle = (itemSize - ITEM_THREE_NUM) * dotPaddingAngle + dotActivePaddingAngle * 2;
    }
    RSPen pen;
    pen.SetAntiAlias(true);
    pen.SetWidth(containerBorderWidth);
    pen.SetColor(contentProperty.backgroundColor.GetValue());
    pen.SetCapStyle(ToRSCapStyle(LineCap::ROUND));
    float startAngle = 0;
    PointF startPoint;
    PointF endPoint;
    // The number 2 represents multiplying by 2 times or dividing equally
    if (axis_ == Axis::HORIZONTAL) {
        startPoint.SetX(centerX_ - circleRadius_ + containerBorderWidth / 2);
        startPoint.SetY(centerY_ - circleRadius_ + containerBorderWidth / 2);
        endPoint.SetX(centerX_ + circleRadius_ - (containerBorderWidth / 2));
        endPoint.SetY(centerY_ + circleRadius_ - (containerBorderWidth / 2));
    } else {
        startPoint.SetX(centerY_ - circleRadius_ + containerBorderWidth / 2);
        startPoint.SetY(centerX_ - circleRadius_ + containerBorderWidth / 2);
        endPoint.SetX(centerY_ + circleRadius_ - (containerBorderWidth / 2));
        endPoint.SetY(centerX_ + circleRadius_ - (containerBorderWidth / 2));
    }
    if (arcDirection_ == SwiperArcDirection::SIX_CLOCK_DIRECTION) {
        startAngle = QUARTER_CIRCLE_ANGLE + (allPointArcAngle / 2);
    } else if (arcDirection_ == SwiperArcDirection::THREE_CLOCK_DIRECTION) {
        startAngle = allPointArcAngle / 2;
    } else {
        startAngle = HALF_CIRCLE_ANGLE + (allPointArcAngle / 2);
    }
    canvas.AttachPen(pen);
    canvas.DrawArc(
        { startPoint.GetX(), startPoint.GetY(), endPoint.GetX(), endPoint.GetY() }, startAngle, -allPointArcAngle);
    canvas.DetachPen();
    canvas.Restore();
}

std::vector<GradientColor> CircleDotIndicatorModifier::GetMaskColor() const
{
    Gradient gradient = SortGradientColorsByOffset(maskColor_);
    std::vector<GradientColor> gradientColors = gradient.GetColors();
    if (gradientColors.empty()) {
        auto pipeline = PipelineBase::GetCurrentContextSafely();
        CHECK_NULL_RETURN(pipeline, gradientColors);
        auto theme = pipeline->GetTheme<SwiperIndicatorTheme>();
        CHECK_NULL_RETURN(theme, gradientColors);
        gradientColors = theme->GetArcMaskColor().GetColors();
    }
    return gradientColors;
}

Gradient CircleDotIndicatorModifier::SortGradientColorsByOffset(const Gradient& gradient) const
{
    auto srcGradientColors = gradient.GetColors();
    std::sort(srcGradientColors.begin(), srcGradientColors.end(),
        [](const GradientColor& left, const GradientColor& right) {
            return left.GetDimension().Value() < right.GetDimension().Value();
        });

    Gradient sortedGradient;
    for (const auto& item : srcGradientColors) {
        sortedGradient.AddColor(item);
    }

    return sortedGradient;
}

void CircleDotIndicatorModifier::PaintIndicatorMask(DrawingContext& context, const ContentProperty& contentProperty)
{
    RSCanvas& canvas = context.canvas;
    RSBrush brush;
    brush.SetAntiAlias(true);

    std::vector<GradientColor> gradientColors = GetMaskColor();
    std::vector<RSColorQuad> colors;
    std::vector<float> pos;
    for (size_t i = 0; i < gradientColors.size(); i++) {
        colors.emplace_back(gradientColors[i].GetLinearColor().GetValue());
        pos.emplace_back(gradientColors[i].GetDimension().Value());
    }

    float centerX = axis_ == Axis::HORIZONTAL ? centerX_ : centerY_;
    float centerY = axis_ == Axis::HORIZONTAL ? centerY_ : centerX_;
    RSPoint startPt = { 0.0, 0.0 };
    RSPoint endPt = { 0.0, 0.0 };
    RSPoint gradientStartPt = { 0.0, 0.0 };
    RSPoint gradientendPt = { 0.0, 0.0 };
    if (arcDirection_ == SwiperArcDirection::SIX_CLOCK_DIRECTION) {
        startPt = RSPoint(
            centerX - circleRadius_, centerY + circleRadius_ - static_cast<float>(MASK_HEIGHT.ConvertToPx()));
        endPt = RSPoint(centerX + circleRadius_, centerY + circleRadius_);
        gradientStartPt = RSPoint(centerX, centerY + circleRadius_ - static_cast<float>(MASK_HEIGHT.ConvertToPx()));
        gradientendPt = RSPoint(centerX, centerY + circleRadius_);
    } else if (arcDirection_ == SwiperArcDirection::THREE_CLOCK_DIRECTION) {
        startPt = RSPoint(
            centerX + circleRadius_ - static_cast<float>(MASK_HEIGHT.ConvertToPx()), centerY - circleRadius_);
        endPt = RSPoint(centerX + circleRadius_, centerY + circleRadius_);
        gradientStartPt = RSPoint(centerX + circleRadius_ - static_cast<float>(MASK_HEIGHT.ConvertToPx()), centerY);
        gradientendPt = RSPoint(centerX + circleRadius_, centerY);
    } else {
        startPt = RSPoint(centerX - circleRadius_, centerY - circleRadius_);
        endPt = RSPoint(
            centerX - circleRadius_ + static_cast<float>(MASK_HEIGHT.ConvertToPx()), centerY + circleRadius_);
        gradientStartPt = RSPoint(centerX - circleRadius_ + static_cast<float>(MASK_HEIGHT.ConvertToPx()), centerY);
        gradientendPt = RSPoint(centerX - circleRadius_, centerY);
    }

    brush.SetShaderEffect(
        RSShaderEffect::CreateLinearGradient(gradientStartPt, gradientendPt, colors, pos, RSTileMode::CLAMP));
    canvas.AttachBrush(brush);
    canvas.DrawRect({ startPt.GetX(), startPt.GetY(), endPt.GetX(), endPt.GetY() });
    canvas.DetachBrush();
    canvas.Restore();
}

void CircleDotIndicatorModifier::PaintContent(DrawingContext& context, ContentProperty& contentProperty)
{
    RSCanvas& canvas = context.canvas;
    auto totalCount = contentProperty.vectorBlackPointAngle.size();
    for (size_t i = 0; i < totalCount; ++i) {
        float itemAngle = contentProperty.vectorBlackPointAngle[i];
        PaintUnselectedIndicator(canvas, itemAngle, contentProperty, LinearColor(unselectedColor_->Get()));
    }

    PaintSelectedIndicator(canvas, contentProperty);
}

void CircleDotIndicatorModifier::PaintUnselectedIndicator(
    RSCanvas& canvas, float itemAngle, ContentProperty& contentProperty, const LinearColor& indicatorColor)
{
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetColor(ToRSColor(indicatorColor));
    canvas.AttachBrush(brush);

    float itemRadius = contentProperty.itemRadius;
    float containerBorderWidth = contentProperty.containerBorderWidth;

    float itemCenterX = 0.0;
    float itemCenterY = 0.0;
    float centerX = axis_ == Axis::HORIZONTAL ? centerX_ : centerY_;
    float centerY = axis_ == Axis::HORIZONTAL ? centerY_ : centerX_;
    double radians = std::abs(itemAngle) * M_PI / HALF_CIRCLE_ANGLE;
    // The number 2 represents multiplying by 2 times or dividing equally
    if (GreatOrEqualCustomPrecision(itemAngle, 0.0f)) {
        if (arcDirection_ == SwiperArcDirection::SIX_CLOCK_DIRECTION) {
            itemCenterX = centerX - (circleRadius_ - (containerBorderWidth / 2)) * sin(radians);
            itemCenterY = centerY + (circleRadius_ - (containerBorderWidth / 2)) * cos(radians);
        } else if (arcDirection_ == SwiperArcDirection::THREE_CLOCK_DIRECTION) {
            itemCenterX = centerX + (circleRadius_ - (containerBorderWidth / 2)) * cos(radians);
            itemCenterY = centerY + (circleRadius_ - (containerBorderWidth / 2)) * sin(radians);
        } else {
            itemCenterX = centerX - (circleRadius_ - (containerBorderWidth / 2)) * cos(radians);
            itemCenterY = centerY - (circleRadius_ - (containerBorderWidth / 2)) * sin(radians);
        }
    } else {
        if (arcDirection_ == SwiperArcDirection::SIX_CLOCK_DIRECTION) {
            itemCenterX = centerX + (circleRadius_ - (containerBorderWidth / 2)) * sin(radians);
            itemCenterY = centerY + (circleRadius_ - (containerBorderWidth / 2)) * cos(radians);
        } else if (arcDirection_ == SwiperArcDirection::THREE_CLOCK_DIRECTION) {
            itemCenterX = centerX + (circleRadius_ - (containerBorderWidth / 2)) * cos(radians);
            itemCenterY = centerY - (circleRadius_ - (containerBorderWidth / 2)) * sin(radians);
        } else {
            itemCenterX = centerX - (circleRadius_ - (containerBorderWidth / 2)) * cos(radians);
            itemCenterY = centerY + (circleRadius_ - (containerBorderWidth / 2)) * sin(radians);
        }
    }

    canvas.DrawCircle({ itemCenterX, itemCenterY }, itemRadius);
    canvas.DetachBrush();
}

void CircleDotIndicatorModifier::PaintSelectedIndicator(RSCanvas& canvas, ContentProperty& contentProperty)
{
    auto totalCount = contentProperty.vectorBlackPointAngle.size();
    if (totalCount == 0) {
        return;
    }

    float containerBorderWidth = contentProperty.containerBorderWidth;
    float itemSelectWidth = contentProperty.itemSelectWidth;
    float dotActiveStartAngle = contentProperty.dotActiveStartAngle;
    float dotActiveEndAngle = contentProperty.dotActiveEndAngle;

    RSPen pen;
    pen.SetAntiAlias(true);
    pen.SetWidth(itemSelectWidth);
    pen.SetColor(selectedColor_->Get().GetValue());
    pen.SetCapStyle(ToRSCapStyle(LineCap::ROUND));

    PointF startPoint;
    PointF endPoint;
    if (axis_ == Axis::HORIZONTAL) {
        startPoint.SetX(centerX_ - circleRadius_ + containerBorderWidth / 2);
        startPoint.SetY(centerY_ - circleRadius_ + containerBorderWidth / 2);
        endPoint.SetX(centerX_ + circleRadius_ - (containerBorderWidth / 2));
        endPoint.SetY(centerY_ + circleRadius_ - (containerBorderWidth / 2));
    } else {
        startPoint.SetX(centerY_ - circleRadius_ + containerBorderWidth / 2);
        startPoint.SetY(centerX_ - circleRadius_ + containerBorderWidth / 2);
        endPoint.SetX(centerY_ + circleRadius_ - (containerBorderWidth / 2));
        endPoint.SetY(centerX_ + circleRadius_ - (containerBorderWidth / 2));
    }

    canvas.AttachPen(pen);
    canvas.DrawArc(
        { startPoint.GetX(), startPoint.GetY(), endPoint.GetX(), endPoint.GetY() },
        dotActiveStartAngle, dotActiveEndAngle - dotActiveStartAngle);
    canvas.DetachPen();
    canvas.Restore();
}

void CircleDotIndicatorModifier::UpdateShrinkPaintProperty(const LinearVector<float>& normalItemSizes,
    const LinearVector<float>& vectorBlackPointAngle, const std::pair<float, float>& longPointAngle)
{
    dotPaddingAngle_->Set(normalItemSizes[ITEM_PADDING]);
    dotActiveAngle_->Set(normalItemSizes[ACTIVE_ITEM_ANGLE]);
    dotActivePaddingAngle_->Set(normalItemSizes[SELECTED_ITEM_PADDING]);
    containerBorderWidth_->Set(normalItemSizes[CONTAINER_RADIUS] * 2);
    itemRadius_->Set(normalItemSizes[ITEM_RADIUS]);
    itemSelectWidth_->Set(normalItemSizes[SELECTED_ITEM_RADIUS] * 2);
    if (longPointLeftAnimEnd_ && longPointRightAnimEnd_) {
        dotActiveStartAngle_->Set(longPointAngle.first);
        dotActiveEndAngle_->Set(longPointAngle.second);
    }
    vectorBlackPointAngle_->Set(vectorBlackPointAngle);
}

void CircleDotIndicatorModifier::UpdateDilatePaintProperty(const LinearVector<float>& hoverItemSizes,
    const LinearVector<float>& vectorBlackPointAngle, const std::pair<float, float>& longPointAngle)
{
    dotPaddingAngle_->Set(hoverItemSizes[ITEM_PADDING]);
    dotActiveAngle_->Set(hoverItemSizes[ACTIVE_ITEM_ANGLE]);
    dotActivePaddingAngle_->Set(hoverItemSizes[SELECTED_ITEM_PADDING]);
    containerBorderWidth_->Set(hoverItemSizes[CONTAINER_RADIUS] * 2);
    itemRadius_->Set(hoverItemSizes[ITEM_RADIUS]);
    itemSelectWidth_->Set(hoverItemSizes[SELECTED_ITEM_RADIUS] * 2);
    if (longPointLeftAnimEnd_ && longPointRightAnimEnd_) {
        dotActiveStartAngle_->Set(longPointAngle.first);
        dotActiveEndAngle_->Set(longPointAngle.second);
    }
    vectorBlackPointAngle_->Set(vectorBlackPointAngle);
}

void CircleDotIndicatorModifier::UpdateBackgroundColor(const Color& backgroundColor)
{
    backgroundColor_->Set(LinearColor(backgroundColor));
}

void CircleDotIndicatorModifier::UpdateNormalPaintProperty(const LinearVector<float>& normalItemSizes,
    const LinearVector<float>& vectorBlackPointAngle, const std::pair<float, float>& longPointAngle)
{
    UpdateShrinkPaintProperty(normalItemSizes, vectorBlackPointAngle, longPointAngle);
    UpdateBackgroundColor(containerColor_.ChangeOpacity(0));
}

void CircleDotIndicatorModifier::UpdatePressPaintProperty(const LinearVector<float>& hoverItemSizes,
    const LinearVector<float>& vectorBlackPointAngle, const std::pair<float, float>& longPointAngle)
{
    UpdateDilatePaintProperty(hoverItemSizes, vectorBlackPointAngle, longPointAngle);
    UpdateBackgroundColor(containerColor_);
}

void CircleDotIndicatorModifier::UpdateNormalToPressPaintProperty(const LinearVector<float>& hoverItemSizes,
    const LinearVector<float>& vectorBlackPointAngle, const std::pair<float, float>& longPointAngle)
{
    AnimationOption option;
    option.SetDuration(COMPONENT_DILATE_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    longPointLeftAnimEnd_ = true;
    longPointRightAnimEnd_ = true;
    AnimationUtils::Animate(option, [weak = WeakClaim(this), hoverItemSizes, vectorBlackPointAngle, longPointAngle]() {
        auto modifier = weak.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->UpdatePressPaintProperty(hoverItemSizes, vectorBlackPointAngle, longPointAngle);
    });
}

void CircleDotIndicatorModifier::UpdatePressToNormalPaintProperty(const LinearVector<float>& normalItemSizes,
    const LinearVector<float>& vectorBlackPointAngle, const std::pair<float, float>& longPointAngle)
{
    StopAnimation(true);
    AnimationOption option;
    option.SetDuration(COMPONENT_SHRINK_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    AnimationUtils::Animate(option, [weak = WeakClaim(this), normalItemSizes, vectorBlackPointAngle, longPointAngle]() {
        auto modifier = weak.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->UpdateNormalPaintProperty(normalItemSizes, vectorBlackPointAngle, longPointAngle);
    });
}

void CircleDotIndicatorModifier::PlayBlackPointsAnimation(const LinearVector<float>& vectorBlackPointAngle)
{
    auto curve = AceType::MakeRefPtr<CubicCurve>(BLACK_POINT_CENTER_BEZIER_CURVE_VELOCITY, CENTER_BEZIER_CURVE_MASS,
        CENTER_BEZIER_CURVE_STIFFNESS, CENTER_BEZIER_CURVE_DAMPING);
    AnimationOption option;
    option.SetCurve(curve);
    option.SetDuration(BLACK_POINT_DURATION);
    blackPointsAnimation_ =
        AnimationUtils::StartAnimation(option, [&]() { vectorBlackPointAngle_->Set(vectorBlackPointAngle); });
}

void CircleDotIndicatorModifier::PlayLongPointAnimation(const std::pair<float, float>& longPointAngle,
    GestureState gestureState, const LinearVector<float>& vectorBlackPointAngle)
{
    AnimationOption optionHead;
    optionHead.SetDuration(LONG_POINT_DURATION);
    optionHead.SetCurve(AceType::MakeRefPtr<CubicCurve>(HEAD_POINT_CENTER_BEZIER_CURVE_VELOCITY,
        CENTER_BEZIER_CURVE_MASS, CENTER_BEZIER_CURVE_STIFFNESS, CENTER_BEZIER_CURVE_DAMPING));

    AnimationOption optionTail;
    optionTail.SetDuration(LONG_POINT_DURATION);
    optionHead.SetCurve(AceType::MakeRefPtr<CubicCurve>(TAIL_POINT_CENTER_BEZIER_CURVE_VELOCITY,
        CENTER_BEZIER_CURVE_MASS, CENTER_BEZIER_CURVE_STIFFNESS, CENTER_BEZIER_CURVE_DAMPING));

    AnimationOption optionLeft = optionTail;
    AnimationOption optionRight = optionHead;

    if (gestureState == GestureState::GESTURE_STATE_RELEASE_LEFT) {
        optionLeft = optionHead;
        optionRight = optionTail;
    }

    if (longPointLeftAnimEnd_ && longPointRightAnimEnd_) {
        longPointLeftAnimEnd_ = false;
        longPointRightAnimEnd_ = false;
        auto weak = WeakClaim(this);
        AnimationUtils::StartAnimation(optionLeft, [weak, longPointAngle]() {
                auto modifier = weak.Upgrade();
                CHECK_NULL_VOID(modifier);
                modifier->dotActiveStartAngle_->Set(longPointAngle.first);
            }, [weak]() {
                auto modifier = weak.Upgrade();
                CHECK_NULL_VOID(modifier);
                modifier->longPointLeftAnimEnd_ = true;
            });

        AnimationUtils::StartAnimation(optionRight, [weak, longPointAngle]() {
                auto modifier = weak.Upgrade();
                CHECK_NULL_VOID(modifier);
                modifier->dotActiveEndAngle_->Set(longPointAngle.second);
            }, [weak]() {
                auto modifier = weak.Upgrade();
                CHECK_NULL_VOID(modifier);
                modifier->longPointRightAnimEnd_ = true;
            });
    }
}

void CircleDotIndicatorModifier::PlayIndicatorAnimation(const LinearVector<float>& vectorBlackPointAngle,
    const std::pair<float, float>& longPointAngle, GestureState gestureState)
{
    StopAnimation();
    PlayBlackPointsAnimation(vectorBlackPointAngle);
    PlayLongPointAnimation(longPointAngle, gestureState, vectorBlackPointAngle);
}

void CircleDotIndicatorModifier::StopAnimation(bool ifImmediately)
{
    if (ifImmediately) {
        AnimationOption option;
        option.SetDuration(0);
        option.SetCurve(Curves::LINEAR);
        AnimationUtils::StartAnimation(option, [weak = WeakClaim(this)]() {
            auto modifier = weak.Upgrade();
            CHECK_NULL_VOID(modifier);
            modifier->dotActiveStartAngle_->Set(modifier->dotActiveStartAngle_->Get());
            modifier->dotActiveEndAngle_->Set(modifier->dotActiveEndAngle_->Get());
        });
    }
    AnimationUtils::StopAnimation(blackPointsAnimation_);
    longPointLeftAnimEnd_ = true;
    longPointRightAnimEnd_ = true;
}
} // namespace OHOS::Ace::NG
