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

#include "core/components_ng/pattern/swiper_indicator/dot_indicator/dot_indicator_modifier.h"

#include "base/utils/utils.h"
#include "core/animation/spring_curve.h"
#include "core/components_ng/render/animation_utils.h"
#include "core/components_ng/render/drawing.h"

namespace OHOS::Ace::NG {
namespace {
constexpr Dimension INDICATOR_ITEM_SPACE = 8.0_vp;
constexpr Dimension INDICATOR_PADDING_DEFAULT = 12.0_vp;
constexpr Dimension INDICATOR_PADDING_HOVER = 12.0_vp;
constexpr float INDICATOR_ZOOM_IN_SCALE = 1.33f;
constexpr int32_t POINT_HOVER_ANIMATION_DURATION = 100;
constexpr int32_t COMPONENT_DILATE_ANIMATION_DURATION = 250;
constexpr int32_t COMPONENT_SHRINK_ANIMATION_DURATION = 300;
constexpr int32_t MOUSE_PRESS_ANIMATION_DURATION = 250;

constexpr float BLACK_POINT_CENTER_BEZIER_CURVE_VELOCITY = 0.4f;
constexpr float CENTER_BEZIER_CURVE_MASS = 0.0f;
constexpr float CENTER_BEZIER_CURVE_STIFFNESS = 1.0f;
constexpr float CENTER_BEZIER_CURVE_DAMPING = 1.0f;
constexpr uint32_t ITEM_HALF_WIDTH = 0;
constexpr uint32_t ITEM_HALF_HEIGHT = 1;
constexpr uint32_t SELECTED_ITEM_HALF_WIDTH = 2;
constexpr uint32_t SELECTED_ITEM_HALF_HEIGHT = 3;
constexpr float TOUCH_BOTTOM_CURVE_VELOCITY = 0.1f;
constexpr float TOUCH_BOTTOM_CURVE_MASS = 0.2f;
constexpr float TOUCH_BOTTOM_CURVE_STIFFNESS = 0.48f;
constexpr float TOUCH_BOTTOM_CURVE_DAMPING = 1.0f;
constexpr float TOUCH_BOTTOM_BACKGROUND_WIDTH_MULTIPLE = 1.225f;
constexpr float TOUCH_BOTTOM_BACKGROUND_HEIGHT_MULTIPLE = 0.8f;
constexpr float TOUCH_BOTTOM_DOT_WIDTH_MULTIPLE = 0.0125f;
constexpr int32_t TOUCH_BOTTOM_ANIMATION_DURATION = 200;
} // namespace

void DotIndicatorModifier::onDraw(DrawingContext& context)
{
    ContentProperty contentProperty;
    contentProperty.backgroundColor = backgroundColor_->Get().ToColor();
    contentProperty.vectorBlackPointCenterX = vectorBlackPointCenterX_->Get();
    contentProperty.longPointLeftCenterX = longPointLeftCenterX_->Get();
    contentProperty.longPointRightCenterX = longPointRightCenterX_->Get();
    contentProperty.normalToHoverPointDilateRatio = normalToHoverPointDilateRatio_->Get();
    contentProperty.hoverToNormalPointDilateRatio = hoverToNormalPointDilateRatio_->Get();
    contentProperty.longPointDilateRatio = longPointDilateRatio_->Get();
    contentProperty.indicatorPadding = indicatorPadding_->Get();
    contentProperty.indicatorMargin = indicatorMargin_->Get();
    contentProperty.itemHalfSizes = itemHalfSizes_->Get();
    PaintBackground(context, contentProperty);
    PaintContent(context, contentProperty);
}

void DotIndicatorModifier::PaintBackground(DrawingContext& context, const ContentProperty& contentProperty)
{
    CHECK_NULL_VOID(contentProperty.backgroundColor.GetAlpha());
    auto itemWidth = contentProperty.itemHalfSizes[ITEM_HALF_WIDTH] * 2;
    auto itemHeight = contentProperty.itemHalfSizes[ITEM_HALF_HEIGHT] * 2;
    auto selectedItemWidth = contentProperty.itemHalfSizes[SELECTED_ITEM_HALF_WIDTH] * 2;
    auto selectedItemHeight = contentProperty.itemHalfSizes[SELECTED_ITEM_HALF_HEIGHT] * 2;
    auto pointNumber = static_cast<float>(contentProperty.vectorBlackPointCenterX.size());
    float allPointDiameterSum = itemWidth * static_cast<float>(pointNumber + 1);
    if (isCustomSize_) {
        allPointDiameterSum = itemWidth * static_cast<float>(pointNumber - 1) + selectedItemWidth;
    }
    float allPointSpaceSum = static_cast<float>(INDICATOR_ITEM_SPACE.ConvertToPx()) * (pointNumber - 1);

    // Background necessary property
    float rectWidth =
        contentProperty.indicatorPadding + allPointDiameterSum + allPointSpaceSum + contentProperty.indicatorPadding;
    float rectHeight = contentProperty.indicatorPadding + itemHeight + contentProperty.indicatorPadding;
    if (selectedItemHeight > itemHeight) {
        rectHeight = contentProperty.indicatorPadding + selectedItemHeight + contentProperty.indicatorPadding;
    }

    auto widthChangeValue = (backgroundWidthDilateRatio_->Get() - 1.0f) * rectWidth;
    auto heightChangeValue = (1.0f - backgroundHeightDilateRatio_->Get()) * rectHeight;
    if (axis_ == Axis::VERTICAL) {
        std::swap(widthChangeValue, heightChangeValue);
    }
    // Property to get the rectangle offset
    float rectLeft =
        axis_ == Axis::HORIZONTAL ? contentProperty.indicatorMargin.GetX() : contentProperty.indicatorMargin.GetY();
    float rectTop =
        axis_ == Axis::HORIZONTAL ? contentProperty.indicatorMargin.GetY() : contentProperty.indicatorMargin.GetX();
    // Adapter circle and rect
    float rectRight = rectLeft + (axis_ == Axis::HORIZONTAL ? rectWidth : rectHeight);
    float rectBottom = rectTop + (axis_ == Axis::HORIZONTAL ? rectHeight : rectWidth);

    if (axis_ == Axis::HORIZONTAL) {
        if (touchBottomType_ == TouchBottomType::START) {
            rectLeft -= widthChangeValue;
        }
        if (touchBottomType_ == TouchBottomType::END) {
            rectRight += widthChangeValue;
        }
        rectTop = rectTop + heightChangeValue * 0.5f;
        rectBottom = rectBottom - heightChangeValue * 0.5f;
        rectHeight -= heightChangeValue;
    } else {
        if (touchBottomType_ == TouchBottomType::START) {
            rectTop -= heightChangeValue;
        }
        if (touchBottomType_ == TouchBottomType::END) {
            rectBottom += heightChangeValue;
        }
        rectLeft = rectLeft + widthChangeValue * 0.5f;
        rectRight = rectRight - widthChangeValue * 0.5f;
        rectWidth -= widthChangeValue;
    }
    // Paint background
    RSCanvas& canvas = context.canvas;
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetColor(ToRSColor(contentProperty.backgroundColor));
    canvas.AttachBrush(brush);
    auto radius = axis_ == Axis::HORIZONTAL ? rectHeight : rectWidth;
    canvas.DrawRoundRect({ { rectLeft, rectTop, rectRight, rectBottom }, radius, radius });
    canvas.DetachBrush();
}

void DotIndicatorModifier::PaintContent(DrawingContext& context, ContentProperty& contentProperty)
{
    RSCanvas& canvas = context.canvas;
    OffsetF selectedCenter = {};
    auto totalCount = contentProperty.vectorBlackPointCenterX.size();
    for (size_t i = 0; i < totalCount; ++i) {
        LinearVector<float> itemHalfSizes = GetItemHalfSizes(i, contentProperty);
        OffsetF center = { contentProperty.vectorBlackPointCenterX[i], centerY_ };
        if (i != currentIndex_) {
            PaintUnselectedIndicator(canvas, center, itemHalfSizes, false, LinearColor(unselectedColor_->Get()));
        } else {
            selectedCenter = center;
            PaintUnselectedIndicator(canvas, center, itemHalfSizes, false, LinearColor(unselectedColor_->Get()));
        }
    }

    OffsetF leftCenter = { contentProperty.longPointLeftCenterX, centerY_ };
    OffsetF rightCenter = { contentProperty.longPointRightCenterX, centerY_ };
    OffsetF centerDistance = rightCenter - leftCenter;
    OffsetF centerDilateDistance = centerDistance * contentProperty.longPointDilateRatio;
    leftCenter -= (centerDilateDistance - centerDistance) * 0.5;
    rightCenter += (centerDilateDistance - centerDistance) * 0.5;
    PaintSelectedIndicator(canvas, selectedCenter, leftCenter, rightCenter,
        contentProperty.itemHalfSizes * contentProperty.longPointDilateRatio);

    bool isLeftTouchBottom = (currentIndex_ == totalCount - 1);
    bool isRightTouchBottom = (currentIndex_ == 0);
    bool isTouchBottom = (isLeftTouchBottom || isRightTouchBottom);
    if (!isTouchBottom || totalCount == 0 || !isTouchBottomLoop_) {
        return;
    }

    size_t index = 0;
    if (isRightTouchBottom) {
        index = totalCount - 1;
    }
    LinearVector<float> itemHalfSizes = GetItemHalfSizes(index, contentProperty);
    OffsetF center = { contentProperty.vectorBlackPointCenterX[index], centerY_ };
    PaintUnselectedIndicator(canvas, center, itemHalfSizes, false, touchBottomPointColor_->Get());
}

LinearVector<float> DotIndicatorModifier::GetItemHalfSizes(size_t index, ContentProperty& contentProperty)
{
    if (normalToHoverIndex_.has_value() && normalToHoverIndex_ == index) {
        return contentProperty.itemHalfSizes * contentProperty.normalToHoverPointDilateRatio;
    }
    if (hoverToNormalIndex_.has_value() && hoverToNormalIndex_ == index) {
        return contentProperty.itemHalfSizes * contentProperty.hoverToNormalPointDilateRatio;
    }
    return contentProperty.itemHalfSizes;
}

void DotIndicatorModifier::PaintUnselectedIndicator(RSCanvas& canvas, const OffsetF& center,
    const LinearVector<float>& itemHalfSizes, bool currentIndexFlag, const LinearColor& indicatorColor)
{
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetColor(ToRSColor(indicatorColor));
    canvas.AttachBrush(brush);
    if (!NearEqual(itemHalfSizes[ITEM_HALF_WIDTH], itemHalfSizes[ITEM_HALF_HEIGHT]) || currentIndexFlag ||
        !isCustomSize_) {
        float rectItemWidth = itemHalfSizes[ITEM_HALF_WIDTH] * 2;
        float rectItemHeight = itemHalfSizes[ITEM_HALF_HEIGHT] * 2;
        if (currentIndexFlag) {
            rectItemWidth = itemHalfSizes[SELECTED_ITEM_HALF_WIDTH] * 2;
            rectItemHeight = itemHalfSizes[SELECTED_ITEM_HALF_HEIGHT] * 2;
        }
        float rectLeft =
            (axis_ == Axis::HORIZONTAL ? center.GetX() - rectItemWidth * 0.5 : center.GetY() - rectItemHeight * 0.5);
        float rectTop =
            (axis_ == Axis::HORIZONTAL ? center.GetY() - rectItemHeight * 0.5 : center.GetX() - rectItemWidth * 0.5);
        float rectRight =
            (axis_ == Axis::HORIZONTAL ? center.GetX() + rectItemWidth * 0.5 : center.GetY() + rectItemHeight * 0.5);
        float rectBottom =
            (axis_ == Axis::HORIZONTAL ? center.GetY() + rectItemHeight * 0.5 : center.GetX() + rectItemWidth * 0.5);

        if (rectItemHeight > rectItemWidth || !isCustomSize_) {
            canvas.DrawRoundRect({ { rectLeft, rectTop, rectRight, rectBottom }, rectItemWidth, rectItemWidth });
        } else if (rectItemHeight < rectItemWidth) {
            canvas.DrawRoundRect({ { rectLeft, rectTop, rectRight, rectBottom }, rectItemHeight, rectItemHeight });
        } else {
            float customPointX = axis_ == Axis::HORIZONTAL ? center.GetX() : center.GetY();
            float customPointY = axis_ == Axis::HORIZONTAL ? center.GetY() : center.GetX();
            canvas.DrawCircle({ customPointX, customPointY }, rectItemHeight * 0.5);
        }
    } else {
        float pointX = axis_ == Axis::HORIZONTAL ? center.GetX() : center.GetY();
        float pointY = axis_ == Axis::HORIZONTAL ? center.GetY() : center.GetX();
        canvas.DrawCircle({ pointX, pointY }, itemHalfSizes[ITEM_HALF_HEIGHT]);
    }
    canvas.DetachBrush();
}

void DotIndicatorModifier::PaintSelectedIndicator(RSCanvas& canvas, const OffsetF& center, const OffsetF& leftCenter,
    const OffsetF& rightCenter, const LinearVector<float>& itemHalfSizes)
{
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetColor(ToRSColor(selectedColor_->Get()));
    canvas.AttachBrush(brush);

    float rectLeft = (axis_ == Axis::HORIZONTAL ? leftCenter.GetX() - itemHalfSizes[SELECTED_ITEM_HALF_WIDTH]
                                                : leftCenter.GetY() - itemHalfSizes[SELECTED_ITEM_HALF_HEIGHT]);

    float rectTop = (axis_ == Axis::HORIZONTAL ? leftCenter.GetY() - itemHalfSizes[SELECTED_ITEM_HALF_HEIGHT]
                                               : leftCenter.GetX() - itemHalfSizes[SELECTED_ITEM_HALF_WIDTH]);
    float rectRight = (axis_ == Axis::HORIZONTAL ? rightCenter.GetX() + itemHalfSizes[SELECTED_ITEM_HALF_WIDTH]
                                                 : rightCenter.GetY() + itemHalfSizes[SELECTED_ITEM_HALF_HEIGHT]);

    float rectBottom = (axis_ == Axis::HORIZONTAL ? rightCenter.GetY() + itemHalfSizes[SELECTED_ITEM_HALF_HEIGHT]
                                                  : rightCenter.GetX() + itemHalfSizes[SELECTED_ITEM_HALF_WIDTH]);

    float rectSelectedItemWidth = itemHalfSizes[SELECTED_ITEM_HALF_WIDTH] * 2;
    float rectSelectedItemHeight = itemHalfSizes[SELECTED_ITEM_HALF_HEIGHT] * 2;

    if (rectSelectedItemHeight > rectSelectedItemWidth && !isCustomSize_) {
        canvas.DrawRoundRect(
            { { rectLeft, rectTop, rectRight, rectBottom }, rectSelectedItemWidth, rectSelectedItemWidth });
    } else {
        canvas.DrawRoundRect(
            { { rectLeft, rectTop, rectRight, rectBottom }, rectSelectedItemHeight, rectSelectedItemHeight });
    }
    canvas.DetachBrush();
}

void DotIndicatorModifier::PaintMask(DrawingContext& context)
{
    RSCanvas& canvas = context.canvas;

    RSBrush brush;
    brush.SetAntiAlias(true);
    canvas.Save();

    std::vector<RSColorQuad> colors;
    colors.push_back(0x00000000);
    colors.push_back(0xff000000);
    colors.push_back(0xff000000);

    RSPoint startPt = { offset_.GetX(), offset_.GetY() };
    RSPoint endPt = { offset_.GetX(), offset_.GetY() };
    startPt -= axis_ == Axis::HORIZONTAL ? RSPoint(0, (9.0_vp).ConvertToPx()) : RSPoint((9.0_vp).ConvertToPx(), 0);
    endPt += axis_ == Axis::HORIZONTAL ? RSPoint(0, (15.0_vp).ConvertToPx()) : RSPoint((15.0_vp).ConvertToPx(), 0);

    std::vector<float> pos = { 0.0f, 0.75f, 1.0f };

    brush.SetShaderEffect(RSShaderEffect::CreateLinearGradient(startPt, endPt, colors, pos, RSTileMode::CLAMP));
    canvas.DrawRect({ startPt.GetX(), startPt.GetY(), endPt.GetX(), endPt.GetY() });
}

void DotIndicatorModifier::UpdateShrinkPaintProperty(
    const OffsetF& margin, const LinearVector<float>& normalItemHalfSizes,
    const LinearVector<float>& vectorBlackPointCenterX, const std::pair<float, float>& longPointCenterX)
{
    indicatorMargin_->Set(margin);
    indicatorPadding_->Set(static_cast<float>(INDICATOR_PADDING_DEFAULT.ConvertToPx()));

    vectorBlackPointCenterX_->Set(vectorBlackPointCenterX);
    if (longPointLeftAnimEnd_ && longPointRightAnimEnd_) {
        longPointLeftCenterX_->Set(longPointCenterX.first);
        longPointRightCenterX_->Set(longPointCenterX.second);
    }

    itemHalfSizes_->Set(normalItemHalfSizes);
    normalToHoverPointDilateRatio_->Set(1.0f);
    hoverToNormalPointDilateRatio_->Set(1.0f);
    longPointDilateRatio_->Set(1.0f);
    backgroundWidthDilateRatio_->Set(1.0f);
    backgroundHeightDilateRatio_->Set(1.0f);
}

void DotIndicatorModifier::UpdateDilatePaintProperty(
    const LinearVector<float>& hoverItemHalfSizes, const LinearVector<float>& vectorBlackPointCenterX,
    const std::pair<float, float>& longPointCenterX)
{
    indicatorMargin_->Set({ 0, 0 });
    indicatorPadding_->Set(static_cast<float>(INDICATOR_PADDING_HOVER.ConvertToPx()));

    vectorBlackPointCenterX_->Set(vectorBlackPointCenterX);
    if (longPointLeftAnimEnd_ && longPointRightAnimEnd_) {
        longPointLeftCenterX_->Set(longPointCenterX.first);
        longPointRightCenterX_->Set(longPointCenterX.second);
    }
    itemHalfSizes_->Set(hoverItemHalfSizes);
    backgroundWidthDilateRatio_->Set(1.0f);
    backgroundHeightDilateRatio_->Set(1.0f);
}

void DotIndicatorModifier::UpdateBackgroundColor(const Color& backgroundColor)
{
    backgroundColor_->Set(LinearColor(backgroundColor));
}

void DotIndicatorModifier::UpdateNormalPaintProperty(
    const OffsetF& margin, const LinearVector<float>& normalItemHalfSizes,
    const LinearVector<float>& vectorBlackPointCenterX, const std::pair<float, float>& longPointCenterX)
{
    auto swiperTheme = GetSwiperIndicatorTheme();
    CHECK_NULL_VOID(swiperTheme);
    auto backgroundColor = indicatorMask_ ?
        swiperTheme->GetPressedColor() :
        swiperTheme->GetHoverColor().ChangeOpacity(0);
    UpdateShrinkPaintProperty(margin, normalItemHalfSizes, vectorBlackPointCenterX, longPointCenterX);
    UpdateBackgroundColor(backgroundColor);
}

void DotIndicatorModifier::UpdateHoverPaintProperty(
    const LinearVector<float>& hoverItemHalfSizes,
    const LinearVector<float>& vectorBlackPointCenterX, const std::pair<float, float>& longPointCenterX)
{
    auto swiperTheme = GetSwiperIndicatorTheme();
    CHECK_NULL_VOID(swiperTheme);
    auto backgroundColor = swiperTheme->GetHoverColor();
    UpdateDilatePaintProperty(hoverItemHalfSizes, vectorBlackPointCenterX, longPointCenterX);
    UpdateBackgroundColor(backgroundColor);
}

void DotIndicatorModifier::UpdatePressPaintProperty(
    const LinearVector<float>& hoverItemHalfSizes,
    const LinearVector<float>& vectorBlackPointCenterX, const std::pair<float, float>& longPointCenterX)
{
    auto swiperTheme = GetSwiperIndicatorTheme();
    CHECK_NULL_VOID(swiperTheme);
    auto backgroundColor = swiperTheme->GetPressedColor();
    UpdateDilatePaintProperty(hoverItemHalfSizes, vectorBlackPointCenterX, longPointCenterX);
    UpdateBackgroundColor(backgroundColor);
}

void DotIndicatorModifier::UpdateNormalToHoverPaintProperty(
    const LinearVector<float>& hoverItemHalfSizes, const LinearVector<float>& vectorBlackPointCenterX,
    const std::pair<float, float>& longPointCenterX)
{
    AnimationOption option;
    option.SetDuration(COMPONENT_DILATE_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    AnimationUtils::Animate(option, [weak = WeakClaim(this), hoverItemHalfSizes, vectorBlackPointCenterX,
        longPointCenterX]() {
        auto modifier = weak.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->UpdateHoverPaintProperty(hoverItemHalfSizes, vectorBlackPointCenterX, longPointCenterX);
    });
}

void DotIndicatorModifier::UpdateHoverToNormalPaintProperty(
    const OffsetF& margin, const LinearVector<float>& normalItemHalfSizes,
    const LinearVector<float>& vectorBlackPointCenterX, const std::pair<float, float>& longPointCenterX)
{
    AnimationOption option;
    option.SetDuration(COMPONENT_SHRINK_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    AnimationUtils::Animate(option, [weak = WeakClaim(this), margin, normalItemHalfSizes, vectorBlackPointCenterX,
        longPointCenterX]() {
        auto modifier = weak.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->UpdateNormalPaintProperty(margin, normalItemHalfSizes, vectorBlackPointCenterX, longPointCenterX);
    });
}

void DotIndicatorModifier::UpdateNormalToPressPaintProperty(
    const LinearVector<float>& hoverItemHalfSizes, const LinearVector<float>& vectorBlackPointCenterX,
    const std::pair<float, float>& longPointCenterX)
{
    AnimationOption option;
    option.SetDuration(COMPONENT_DILATE_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    AnimationUtils::Animate(option, [weak = WeakClaim(this), hoverItemHalfSizes, vectorBlackPointCenterX,
        longPointCenterX]() {
        auto modifier = weak.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->UpdatePressPaintProperty(hoverItemHalfSizes, vectorBlackPointCenterX, longPointCenterX);
    });
}

void DotIndicatorModifier::UpdatePressToNormalPaintProperty(
    const OffsetF& margin, const LinearVector<float>& normalItemHalfSizes,
    const LinearVector<float>& vectorBlackPointCenterX, const std::pair<float, float>& longPointCenterX)
{
    AnimationOption option;
    option.SetDuration(COMPONENT_SHRINK_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    AnimationUtils::Animate(option, [weak = WeakClaim(this), margin, normalItemHalfSizes, vectorBlackPointCenterX,
        longPointCenterX]() {
        auto modifier = weak.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->UpdateNormalPaintProperty(margin, normalItemHalfSizes, vectorBlackPointCenterX, longPointCenterX);
    });
}

void DotIndicatorModifier::UpdateHoverAndPressConversionPaintProperty()
{
    auto swiperTheme = GetSwiperIndicatorTheme();
    CHECK_NULL_VOID(swiperTheme);
    Color backgroundColor = isPressed_ ? swiperTheme->GetPressedColor() : swiperTheme->GetHoverColor();
    AnimationOption option;
    option.SetDuration(MOUSE_PRESS_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    AnimationUtils::Animate(option, [weak = WeakClaim(this), backgroundColor]() {
        auto modifier = weak.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->UpdateBackgroundColor(backgroundColor);
    });
}

void DotIndicatorModifier::UpdateNormalToHoverPointDilateRatio()
{
    normalToHoverPointDilateRatio_->Set(1.0f);
    AnimationOption option;
    option.SetDuration(POINT_HOVER_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    AnimationUtils::Animate(option, [&]() { normalToHoverPointDilateRatio_->Set(INDICATOR_ZOOM_IN_SCALE); });
}

void DotIndicatorModifier::UpdateHoverToNormalPointDilateRatio()
{
    hoverToNormalPointDilateRatio_->Set(normalToHoverPointDilateRatio_->Get());
    AnimationOption option;
    option.SetDuration(POINT_HOVER_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    AnimationUtils::Animate(option, [&]() { hoverToNormalPointDilateRatio_->Set(1.0f); });
}

void DotIndicatorModifier::UpdateLongPointDilateRatio()
{
    AnimationOption option;
    option.SetDuration(POINT_HOVER_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    if (longPointIsHover_) {
        AnimationUtils::Animate(option, [&]() { longPointDilateRatio_->Set(INDICATOR_ZOOM_IN_SCALE); });
    } else {
        AnimationUtils::Animate(option, [&]() { longPointDilateRatio_->Set(1.0f); });
    }
}

void DotIndicatorModifier::UpdateAllPointCenterXAnimation(GestureState gestureState,
    const LinearVector<float>& vectorBlackPointCenterX, const std::pair<float, float>& longPointCenterX)
{
    AnimationOption blackPointOption;
    blackPointOption.SetDuration(animationDuration_);
    blackPointOption.SetCurve(AceType::MakeRefPtr<CubicCurve>(BLACK_POINT_CENTER_BEZIER_CURVE_VELOCITY,
        CENTER_BEZIER_CURVE_MASS, CENTER_BEZIER_CURVE_STIFFNESS, CENTER_BEZIER_CURVE_DAMPING));
    AnimationUtils::Animate(blackPointOption, [&]() { vectorBlackPointCenterX_->Set(vectorBlackPointCenterX); });

    // normal page turning
    AnimationOption optionHead;
    RefPtr<Curve> curve = headCurve_;
    if (InstanceOf<LinearCurve>(curve)) {
        // mass:1, stiffness:228, damping:30
        curve = AceType::MakeRefPtr<InterpolatingSpring>(motionVelocity_, 1, 228, 30);
    }
    optionHead.SetCurve(curve);
    optionHead.SetDuration(animationDuration_);

    AnimationOption optionTail;
    // velocity:0, mass:1, stiffness:81, damping:11
    optionTail.SetCurve(AceType::MakeRefPtr<InterpolatingSpring>(0, 1, 81, 11));
    optionTail.SetDuration(animationDuration_);
    AnimationOption optionLeft = optionTail;
    AnimationOption optionRight = optionHead;

    if (gestureState == GestureState::GESTURE_STATE_RELEASE_LEFT) {
        optionLeft = optionHead;
        optionRight = optionTail;
    }

    if (longPointLeftAnimEnd_ && longPointRightAnimEnd_) {
        longPointLeftAnimEnd_ = false;
        longPointRightAnimEnd_ = false;

        longPointLeftAnimation_ = AnimationUtils::StartAnimation(
            optionLeft, [&]() { longPointLeftCenterX_->Set(longPointCenterX.first); },
            [&]() { longPointLeftAnimEnd_ = true; });

        longPointRightAnimation_ = AnimationUtils::StartAnimation(
            optionRight, [&]() { longPointRightCenterX_->Set(longPointCenterX.second); },
            [&]() { longPointRightAnimEnd_ = true; });
    }
}

void DotIndicatorModifier::UpdateTouchBottomAnimation(TouchBottomType touchBottomType,
    const LinearVector<float>& vectorBlackPointCenterX, const std::pair<float, float>& longPointCenterX,
    float touchBottomRate)
{
    AnimationOption option;
    option.SetDuration(POINT_HOVER_ANIMATION_DURATION);
    option.SetCurve(AceType::MakeRefPtr<CubicCurve>(TOUCH_BOTTOM_CURVE_VELOCITY, TOUCH_BOTTOM_CURVE_MASS,
        TOUCH_BOTTOM_CURVE_STIFFNESS, TOUCH_BOTTOM_CURVE_DAMPING));

    auto backgroundWidthDilateRatio = 1.0f;
    auto backgroundHeightDilateRatio = 1.0f;

    if (touchBottomType != TouchBottomType::NONE) {
        backgroundWidthDilateRatio = TOUCH_BOTTOM_BACKGROUND_WIDTH_MULTIPLE -
                                     TOUCH_BOTTOM_DOT_WIDTH_MULTIPLE * vectorBlackPointCenterX_->Get().size();
        backgroundHeightDilateRatio = TOUCH_BOTTOM_BACKGROUND_HEIGHT_MULTIPLE;
        backgroundWidthDilateRatio = (backgroundWidthDilateRatio - 1.0f) * touchBottomRate + 1.0f;
        backgroundHeightDilateRatio = (backgroundHeightDilateRatio - 1.0f) * touchBottomRate + 1.0f;
    }
    touchBottomType_ = touchBottomType;
    AnimationUtils::Animate(option, [weak = WeakClaim(this), backgroundWidthDilateRatio, backgroundHeightDilateRatio,
                                        vectorBlackPointCenterX, longPointCenterX]() {
        auto modifier = weak.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->backgroundWidthDilateRatio_->Set(backgroundWidthDilateRatio);
        modifier->backgroundHeightDilateRatio_->Set(backgroundHeightDilateRatio);
        modifier->vectorBlackPointCenterX_->Set(vectorBlackPointCenterX);
        if (modifier->longPointLeftAnimEnd_) {
            modifier->longPointLeftCenterX_->Set(longPointCenterX.first);
        }
        if (modifier->longPointRightAnimEnd_) {
            modifier->longPointRightCenterX_->Set(longPointCenterX.second);
        }
    });
}

void DotIndicatorModifier::PlayBlackPointsAnimation(const LinearVector<float>& vectorBlackPointCenterX)
{
    auto curve = AceType::MakeRefPtr<CubicCurve>(BLACK_POINT_CENTER_BEZIER_CURVE_VELOCITY, CENTER_BEZIER_CURVE_MASS,
        CENTER_BEZIER_CURVE_STIFFNESS, CENTER_BEZIER_CURVE_DAMPING);
    AnimationOption option;
    option.SetCurve(curve);
    option.SetDuration(animationDuration_);
    blackPointsAnimation_ =
        AnimationUtils::StartAnimation(option, [&]() { vectorBlackPointCenterX_->Set(vectorBlackPointCenterX); });
}

void DotIndicatorModifier::PlayTouchBottomAnimation(const std::vector<std::pair<float, float>>& longPointCenterX,
    TouchBottomTypeLoop touchBottomTypeLoop, const LinearVector<float>& vectorBlackPointCenterX)
{
    if (vectorBlackPointCenterX.empty()) {
        return;
    }

    AnimationOption optionBottom;
    // x0:0.33, y0:0, x1:0.67, y1:1
    optionBottom.SetCurve(AceType::MakeRefPtr<CubicCurve>(0.33, 0, 0.67, 1));
    optionBottom.SetDuration(TOUCH_BOTTOM_ANIMATION_DURATION);

    FinishCallback bottomFinishCallback = [&, optionBottom, longPointCenterX, vectorBlackPointCenterX,
                                              touchBottomTypeLoop]() {
        if (!ifNeedFinishCallback_) {
            return;
        }
        if (touchBottomTypeLoop == TouchBottomTypeLoop::TOUCH_BOTTOM_TYPE_LOOP_LEFT) {
            longPointLeftCenterX_->Set(vectorBlackPointCenterX[vectorBlackPointCenterX.size() - 1]);
            longPointRightCenterX_->Set(vectorBlackPointCenterX[vectorBlackPointCenterX.size() - 1]);
        } else if (touchBottomTypeLoop == TouchBottomTypeLoop::TOUCH_BOTTOM_TYPE_LOOP_RIGHT) {
            longPointLeftCenterX_->Set(vectorBlackPointCenterX[0]);
            longPointRightCenterX_->Set(vectorBlackPointCenterX[0]);
        }

        AnimationOption optionOpacity;
        // x0:0.33, y0:0, x1:0.67, y1:1
        optionOpacity.SetCurve(AceType::MakeRefPtr<CubicCurve>(0.33, 0, 0.67, 1));
        optionOpacity.SetDuration(100);
        isSelectedColorAnimEnd_ = false;
        isTouchBottomLoop_ = true;
        AnimationUtils::StartAnimation(optionOpacity, [&]() {
            selectedColor_->Set(LinearColor(selectedColor_->Get().BlendOpacity(1.0)));
            touchBottomPointColor_->Set(LinearColor(touchBottomPointColor_->Get().BlendOpacity(0)));
        }, [&]() {
            touchBottomPointColor_->Set(LinearColor(unselectedColor_->Get()));
            isTouchBottomLoop_ = false;
            isSelectedColorAnimEnd_ = true;
        });

        AnimationUtils::StartAnimation(optionBottom, [&, longPointCenterX]() {
            longPointLeftCenterX_->Set(longPointCenterX[1].first);
            longPointRightCenterX_->Set(longPointCenterX[1].second);
        }, [&]() {
            longPointLeftAnimEnd_ = true;
            longPointRightAnimEnd_ = true;
        });
    };
    if (longPointLeftAnimEnd_ && longPointRightAnimEnd_) {
        longPointLeftAnimEnd_ = false;
        longPointRightAnimEnd_ = false;
        ifNeedFinishCallback_ = true;
        touchBottomPointColor_->Set(LinearColor(selectedColor_->Get()));
        AnimationUtils::StartAnimation(optionBottom, [&, longPointCenterX]() {
            longPointLeftCenterX_->Set(longPointCenterX[0].first);
            longPointRightCenterX_->Set(longPointCenterX[0].second);
        }, bottomFinishCallback);
    }
}

void DotIndicatorModifier::PlayLongPointAnimation(const std::vector<std::pair<float, float>>& longPointCenterX,
    GestureState gestureState, TouchBottomTypeLoop touchBottomTypeLoop,
    const LinearVector<float>& vectorBlackPointCenterX)
{
    if (longPointCenterX.empty()) {
        return;
    }
    // touch bottom
    if (longPointCenterX.size() > 1) {
        PlayTouchBottomAnimation(longPointCenterX, touchBottomTypeLoop, vectorBlackPointCenterX);
        return;
    }
    // normal page turning
    AnimationOption optionHead;
    RefPtr<Curve> curve = headCurve_;
    if (InstanceOf<LinearCurve>(curve)) {
        // mass:1, stiffness:228, damping:30
        curve = AceType::MakeRefPtr<InterpolatingSpring>(motionVelocity_, 1, 228, 30);
    }
    optionHead.SetCurve(curve);
    optionHead.SetDuration(animationDuration_);

    AnimationOption optionTail;
    // velocity:0, mass:1, stiffness:81, damping:11
    optionTail.SetCurve(AceType::MakeRefPtr<InterpolatingSpring>(0, 1, 81, 11));
    optionTail.SetDuration(animationDuration_);
    AnimationOption optionLeft = optionTail;
    AnimationOption optionRight = optionHead;

    if (gestureState == GestureState::GESTURE_STATE_RELEASE_LEFT) {
        optionLeft = optionHead;
        optionRight = optionTail;
    }

    if (longPointLeftAnimEnd_ && longPointRightAnimEnd_) {
        longPointLeftAnimEnd_ = false;
        longPointRightAnimEnd_ = false;
        longPointLeftAnimation_ = AnimationUtils::StartAnimation(optionLeft, [&, longPointCenterX]() {
                longPointLeftCenterX_->Set(longPointCenterX[0].first);
            }, [&]() { longPointLeftAnimEnd_ = true; });

        longPointRightAnimation_ = AnimationUtils::StartAnimation(optionRight, [&, longPointCenterX]() {
                longPointRightCenterX_->Set(longPointCenterX[0].second);
            }, [&]() { longPointRightAnimEnd_ = true; });
    }
}

void DotIndicatorModifier::PlayIndicatorAnimation(const LinearVector<float>& vectorBlackPointCenterX,
    const std::vector<std::pair<float, float>>& longPointCenterX, GestureState gestureState,
    TouchBottomTypeLoop touchBottomTypeLoop)
{
    isTouchBottomLoop_ = false;
    PlayBlackPointsAnimation(vectorBlackPointCenterX);
    PlayLongPointAnimation(longPointCenterX, gestureState, touchBottomTypeLoop, vectorBlackPointCenterX);
}

void DotIndicatorModifier::StopAnimation()
{
    AnimationUtils::StopAnimation(blackPointsAnimation_);
    AnimationUtils::StopAnimation(longPointLeftAnimation_);
    longPointLeftAnimEnd_ = true;
    AnimationUtils::StopAnimation(longPointRightAnimation_);
    longPointRightAnimEnd_ = true;
    ifNeedFinishCallback_ = false;
}
} // namespace OHOS::Ace::NG
