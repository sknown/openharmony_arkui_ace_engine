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

#include "core/components_ng/pattern/swiper_indicator/dot_indicator/overlength_dot_indicator_modifier.h"
#include "base/utils/utils.h"
#include "core/components/swiper/swiper_indicator_theme.h"
#include "core/components_ng/render/animation_utils.h"
#include "core/components_ng/render/drawing.h"

namespace OHOS::Ace::NG {
namespace {
constexpr Dimension INDICATOR_PADDING_DEFAULT = 12.0_vp;
constexpr Dimension INDICATOR_ITEM_SPACE = 8.0_vp;
constexpr float BLACK_POINT_CENTER_BEZIER_CURVE_VELOCITY = 0.2f;
constexpr float CENTER_BEZIER_CURVE_MASS = 0.0f;
constexpr float CENTER_BEZIER_CURVE_STIFFNESS = 0.2f;
constexpr float CENTER_BEZIER_CURVE_DAMPING = 1.0f;
constexpr float SMALLEST_POINT_RATIO = 1.0f / 3.0f;
constexpr float SECOND_SMALLEST_POINT_RATIO = 2.0f / 3.0f;
constexpr float NORMAL_FADING_RATIO = 1.0f;
constexpr double FULL_ALPHA = 255.0;
constexpr int32_t BLACK_POINT_DURATION = 400;
constexpr int32_t NUM_6 = 6;
constexpr int32_t LEFT_FIRST_POINT_INDEX = 0;
constexpr int32_t SECOND_POINT_INDEX = 1;
constexpr int32_t THIRD_POINT_INDEX = 2;
constexpr uint32_t ITEM_HALF_WIDTH = 0;
constexpr uint32_t SELECTED_ITEM_HALF_WIDTH = 2;
constexpr float HALF_FLOAT = 0.5f;
} // namespace

void OverlengthDotIndicatorModifier::onDraw(DrawingContext& context)
{
    if (maxDisplayCount_ <= 0) {
        return;
    }

    ContentProperty contentProperty;
    contentProperty.backgroundColor = backgroundColor_->Get().ToColor();
    contentProperty.unselectedIndicatorWidth = unselectedIndicatorWidth_->Get();
    contentProperty.unselectedIndicatorHeight = unselectedIndicatorHeight_->Get();
    contentProperty.vectorBlackPointCenterX = vectorBlackPointCenterX_->Get();
    contentProperty.longPointLeftCenterX = longPointLeftCenterX_->Get();
    contentProperty.longPointRightCenterX = longPointRightCenterX_->Get();
    contentProperty.normalToHoverPointDilateRatio = normalToHoverPointDilateRatio_->Get();
    contentProperty.hoverToNormalPointDilateRatio = hoverToNormalPointDilateRatio_->Get();
    contentProperty.longPointDilateRatio = longPointDilateRatio_->Get();
    contentProperty.indicatorPadding = indicatorPadding_->Get();
    contentProperty.indicatorMargin = indicatorMargin_->Get();
    contentProperty.itemHalfSizes = itemHalfSizes_->Get();
    contentProperty.firstPointOpacity = firstPointOpacity_->Get();
    contentProperty.newPointOpacity = newPointOpacity_->Get();

    SetFocusedAndSelectedColor(contentProperty);
    PaintBackground(context, contentProperty);
    PaintContent(context, contentProperty);
}

void OverlengthDotIndicatorModifier::PaintContent(DrawingContext& context, ContentProperty& contentProperty)
{
    PaintBlackPoint(context, contentProperty);
    RSCanvas& canvas = context.canvas;
    auto [leftCenterX, rightCenterX] = GetTouchBottomCenterX(contentProperty);

    OffsetF leftCenter = { leftCenterX, centerY_ };
    OffsetF rightCenter = { rightCenterX, centerY_ };
    OffsetF centerDistance = rightCenter - leftCenter;
    OffsetF centerDilateDistance = centerDistance * contentProperty.longPointDilateRatio;
    leftCenter -= (centerDilateDistance - centerDistance) * HALF_FLOAT;
    rightCenter += (centerDilateDistance - centerDistance) * HALF_FLOAT;
    PaintSelectedIndicator(
        canvas, leftCenter, rightCenter, contentProperty.itemHalfSizes * contentProperty.longPointDilateRatio);
}

void OverlengthDotIndicatorModifier::PaintBlackPoint(DrawingContext& context, ContentProperty& contentProperty)
{
    RSCanvas& canvas = context.canvas;
    auto totalCount = contentProperty.vectorBlackPointCenterX.size();
    for (size_t i = 0; i < totalCount; ++i) {
        OffsetF center = { contentProperty.vectorBlackPointCenterX[i], centerY_ };
        float width = contentProperty.unselectedIndicatorWidth[i];
        float height = contentProperty.unselectedIndicatorHeight[i];

        auto paintColor = unselectedColor_->Get();
        bool isFirstPoint = (i == 0 && moveDirection_ == OverlongIndicatorMove::MOVE_BACKWARD) ||
                            (i == totalCount - 2 && moveDirection_ == OverlongIndicatorMove::MOVE_FORWARD);
        if (isFirstPoint) {
            // first point color
            paintColor = paintColor.BlendOpacity(contentProperty.firstPointOpacity / FULL_ALPHA);
        } else if (i == totalCount - 1 && moveDirection_ != OverlongIndicatorMove::NONE) {
            // new point color
            paintColor = paintColor.BlendOpacity(contentProperty.newPointOpacity / FULL_ALPHA);
        }

        PaintUnselectedIndicator(canvas, center, width, height, LinearColor(paintColor));
    }
}

void OverlengthDotIndicatorModifier::PaintUnselectedIndicator(
    RSCanvas& canvas, const OffsetF& center, float width, float height, const LinearColor& indicatorColor)
{
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetColor(ToRSColor(indicatorColor));
    canvas.AttachBrush(brush);
    if (!NearEqual(width, height) || !isCustomSize_) {
        float rectLeft =
            (axis_ == Axis::HORIZONTAL ? center.GetX() - width * HALF_FLOAT : center.GetY() - height * HALF_FLOAT);
        float rectTop =
            (axis_ == Axis::HORIZONTAL ? center.GetY() - height * HALF_FLOAT : center.GetX() - width * HALF_FLOAT);
        float rectRight =
            (axis_ == Axis::HORIZONTAL ? center.GetX() + width * HALF_FLOAT : center.GetY() + height * HALF_FLOAT);
        float rectBottom =
            (axis_ == Axis::HORIZONTAL ? center.GetY() + height * HALF_FLOAT : center.GetX() + width * HALF_FLOAT);

        if (height > width || !isCustomSize_) {
            canvas.DrawRoundRect({ { rectLeft, rectTop, rectRight, rectBottom }, width, width });
        } else if (height < width) {
            canvas.DrawRoundRect({ { rectLeft, rectTop, rectRight, rectBottom }, height, height });
        } else {
            float customPointX = axis_ == Axis::HORIZONTAL ? center.GetX() : center.GetY();
            float customPointY = axis_ == Axis::HORIZONTAL ? center.GetY() : center.GetX();
            canvas.DrawCircle({ customPointX, customPointY }, height * HALF_FLOAT);
        }
    } else {
        float pointX = axis_ == Axis::HORIZONTAL ? center.GetX() : center.GetY();
        float pointY = axis_ == Axis::HORIZONTAL ? center.GetY() : center.GetX();
        canvas.DrawCircle({ pointX, pointY }, width * HALF_FLOAT);
    }
    canvas.DetachBrush();
}

void OverlengthDotIndicatorModifier::UpdateShrinkPaintProperty(const OffsetF& margin,
    const LinearVector<float>& normalItemHalfSizes, const std::pair<float, float>& longPointCenterX)
{
    indicatorMargin_->Set(margin);
    indicatorPadding_->Set(static_cast<float>(INDICATOR_PADDING_DEFAULT.ConvertToPx()));

    if (longPointLeftAnimEnd_ && longPointRightAnimEnd_) {
        vectorBlackPointCenterX_->Set(animationEndCenterX_);
        longPointLeftCenterX_->Set(longPointCenterX.first);
        longPointRightCenterX_->Set(longPointCenterX.second);
    }
    unselectedIndicatorWidth_->Set(animationEndIndicatorWidth_);
    unselectedIndicatorHeight_->Set(animationEndIndicatorHeight_);
    itemHalfSizes_->Set(normalItemHalfSizes);
    normalToHoverPointDilateRatio_->Set(NORMAL_FADING_RATIO);
    hoverToNormalPointDilateRatio_->Set(NORMAL_FADING_RATIO);
    longPointDilateRatio_->Set(NORMAL_FADING_RATIO);
    backgroundWidthDilateRatio_->Set(NORMAL_FADING_RATIO);
    backgroundHeightDilateRatio_->Set(NORMAL_FADING_RATIO);
    longPointWithAnimation_ = true;
}

void OverlengthDotIndicatorModifier::UpdateNormalPaintProperty(const OffsetF& margin,
    const LinearVector<float>& normalItemHalfSizes, const std::pair<float, float>& longPointCenterX)
{
    normalMargin_ = margin;
    CalcAnimationEndCenterX(normalItemHalfSizes);
    auto swiperTheme = GetSwiperIndicatorTheme();
    CHECK_NULL_VOID(swiperTheme);
    auto backgroundColor =
        indicatorMask_ ? swiperTheme->GetPressedColor() : swiperTheme->GetHoverColor().ChangeOpacity(0);
    UpdateShrinkPaintProperty(margin, normalItemHalfSizes, overlongSelectedEndCenterX_);
    UpdateBackgroundColor(backgroundColor);
}

void OverlengthDotIndicatorModifier::PlayBlackPointsAnimation(const LinearVector<float>& itemHalfSizes)
{
    if (maxDisplayCount_ < NUM_6) {
        return;
    }

    AnimationOption blackPointOption;
    auto pointMoveCurve = AceType::MakeRefPtr<CubicCurve>(BLACK_POINT_CENTER_BEZIER_CURVE_VELOCITY,
        CENTER_BEZIER_CURVE_MASS, CENTER_BEZIER_CURVE_STIFFNESS, CENTER_BEZIER_CURVE_DAMPING);
    blackPointOption.SetCurve(pointMoveCurve);
    blackPointOption.SetDuration(BLACK_POINT_DURATION);
    
    vectorBlackPointCenterX_->Set(animationStartCenterX_);
    unselectedIndicatorWidth_->Set(animationStartIndicatorWidth_);
    unselectedIndicatorHeight_->Set(animationStartIndicatorHeight_);
    firstPointOpacity_->Set(UINT8_MAX);
    newPointOpacity_->Set(0);
    isSelectedColorAnimEnd_ = false;
    isTouchBottomLoop_ = true;
    AnimationUtils::StartAnimation(blackPointOption, [&]() {
        vectorBlackPointCenterX_->Set(animationEndCenterX_);
        unselectedIndicatorWidth_->Set(animationEndIndicatorWidth_);
        unselectedIndicatorHeight_->Set(animationEndIndicatorHeight_);

        if (moveDirection_ != OverlongIndicatorMove::NONE) {
            firstPointOpacity_->Set(0);
            newPointOpacity_->Set(UINT8_MAX);
        }
    }, [weak = WeakClaim(this)]() {
        auto modifier = weak.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->SetMoveDirection(OverlongIndicatorMove::NONE);
        modifier->currentSelectedIndex_ = modifier->targetSelectedIndex_;
        modifier->currentOverlongType_ = modifier->targetOverlongType_;
    });
}

LinearVector<float> OverlengthDotIndicatorModifier::CalcIndicatorSize(const LinearVector<float>& itemHalfSizes,
    int32_t selectedIndex, OverlongType overlongType, int32_t pageIndex, bool isWidth)
{
    auto unselectedIndicatorRadius = isWidth ? itemHalfSizes[0] : itemHalfSizes[1];
    LinearVector<float> indicatorSize(maxDisplayCount_ + 1);

    auto secondSmallestRadius = unselectedIndicatorRadius * SECOND_SMALLEST_POINT_RATIO;
    auto smallestRadius = unselectedIndicatorRadius * SMALLEST_POINT_RATIO;

    for (int32_t i = 0; i < maxDisplayCount_; i++) {
        if (i == LEFT_FIRST_POINT_INDEX) {
            if (overlongType == OverlongType::LEFT_NORMAL_RIGHT_FADEOUT) {
                indicatorSize[i] = unselectedIndicatorRadius * 2;
            } else {
                indicatorSize[i] = smallestRadius * 2;
            }
            continue;
        }

        if (i == SECOND_POINT_INDEX) {
            if (overlongType == OverlongType::LEFT_NORMAL_RIGHT_FADEOUT) {
                indicatorSize[i] = unselectedIndicatorRadius * 2;
            } else {
                indicatorSize[i] = secondSmallestRadius * 2;
            }
            continue;
        }

        if (i >= THIRD_POINT_INDEX && i <= maxDisplayCount_ - 1 - THIRD_POINT_INDEX) {
            indicatorSize[i] = unselectedIndicatorRadius * 2;
            continue;
        }

        if (i == maxDisplayCount_ - 1 - SECOND_POINT_INDEX) {
            if (overlongType == OverlongType::LEFT_FADEOUT_RIGHT_NORMAL) {
                indicatorSize[i] = unselectedIndicatorRadius * 2;
            } else {
                indicatorSize[i] = secondSmallestRadius * 2;
            }
            continue;
        }

        if (i == maxDisplayCount_ - 1) {
            if (overlongType == OverlongType::LEFT_FADEOUT_RIGHT_NORMAL) {
                indicatorSize[i] = unselectedIndicatorRadius * 2;
            } else {
                indicatorSize[i] = smallestRadius * 2;
            }
            continue;
        }
    }

    return indicatorSize;
}

void OverlengthDotIndicatorModifier::UpdateSelectedCenterXOnDrag()
{
    if (gestureState_ != GestureState::GESTURE_STATE_FOLLOW_LEFT &&
        gestureState_ != GestureState::GESTURE_STATE_FOLLOW_RIGHT) {
        return;
    }

    auto leftMoveRate = longPointLeftCenterMoveRate_;
    auto rightMoveRate = longPointRightCenterMoveRate_;
    auto blackPointMoveRate = blackPointCenterMoveRate_;
    if (gestureState_ == GestureState::GESTURE_STATE_FOLLOW_LEFT &&
        touchBottomTypeLoop_ == TouchBottomTypeLoop::TOUCH_BOTTOM_TYPE_LOOP_NONE) {
        leftMoveRate = 1.0f - longPointLeftCenterMoveRate_;
        rightMoveRate = 1.0f - longPointRightCenterMoveRate_;
        blackPointMoveRate = 1.0f - blackPointCenterMoveRate_;
    }

    if (touchBottomTypeLoop_ != TouchBottomTypeLoop::TOUCH_BOTTOM_TYPE_LOOP_NONE) {
        overlongSelectedEndCenterX_.first =
            overlongSelectedStartCenterX_.first +
            (overlongSelectedStartCenterX_.second - overlongSelectedStartCenterX_.first) * leftMoveRate;

        overlongSelectedEndCenterX_.second =
            overlongSelectedStartCenterX_.first +
            (overlongSelectedStartCenterX_.second - overlongSelectedStartCenterX_.first) * (1.0f - rightMoveRate);
    } else {
        overlongSelectedEndCenterX_.first =
            overlongSelectedStartCenterX_.first +
            (overlongSelectedEndCenterX_.first - overlongSelectedStartCenterX_.first) * leftMoveRate;

        overlongSelectedEndCenterX_.second =
            overlongSelectedStartCenterX_.second +
            (overlongSelectedEndCenterX_.second - overlongSelectedStartCenterX_.second) * rightMoveRate;
    }

    animationEndCenterX_[currentSelectedIndex_] =
        animationStartCenterX_[currentSelectedIndex_] +
        (animationEndCenterX_[currentSelectedIndex_] - animationStartCenterX_[currentSelectedIndex_]) *
            blackPointMoveRate;

    animationEndCenterX_[targetSelectedIndex_] =
        animationStartCenterX_[targetSelectedIndex_] +
        (animationEndCenterX_[targetSelectedIndex_] - animationStartCenterX_[targetSelectedIndex_]) *
            blackPointMoveRate;
    targetSelectedIndex_ = currentSelectedIndex_;
    targetOverlongType_ = currentOverlongType_;
}

void OverlengthDotIndicatorModifier::UpdateUnselectedCenterXOnDrag()
{
    if (gestureState_ != GestureState::GESTURE_STATE_FOLLOW_LEFT &&
        gestureState_ != GestureState::GESTURE_STATE_FOLLOW_RIGHT) {
        return;
    }

    for (size_t i = 0; i < animationEndCenterX_.size(); i++) {
        animationEndCenterX_[i] = animationStartCenterX_[i] +
                                  (animationEndCenterX_[i] - animationStartCenterX_[i]) * blackPointCenterMoveRate_;
    }

    targetSelectedIndex_ = currentSelectedIndex_;
    targetOverlongType_ = currentOverlongType_;
}

int32_t OverlengthDotIndicatorModifier::CalcTargetIndexOnDrag() const
{
    if (NearEqual(turnPageRate_, 0.0f)) {
        return animationEndIndex_;
    }

    if (animationStartIndex_ == animationEndIndex_) {
        if (animationStartIndex_ == realItemCount_ - 1) {
            return animationStartIndex_;
        }
        return animationStartIndex_ + 1;
    }

    if (animationStartIndex_ == 0 && animationEndIndex_ == realItemCount_ - 1) {
        return animationStartIndex_;
    }

    return animationEndIndex_;
}

void OverlengthDotIndicatorModifier::CalcTargetStatusOnLongPointMove(const LinearVector<float>& itemHalfSizes)
{
    auto endCenterX =
        CalcIndicatorCenterX(itemHalfSizes, targetSelectedIndex_, targetOverlongType_, animationEndIndex_);
    animationEndCenterX_ = endCenterX.first;
    overlongSelectedEndCenterX_ = endCenterX.second;
    animationStartIndicatorWidth_ =
        CalcIndicatorSize(itemHalfSizes, currentSelectedIndex_, currentOverlongType_, animationStartIndex_, true);
    animationStartIndicatorHeight_ =
        CalcIndicatorSize(itemHalfSizes, currentSelectedIndex_, currentOverlongType_, animationStartIndex_, false);

    animationEndIndicatorWidth_ =
        CalcIndicatorSize(itemHalfSizes, targetSelectedIndex_, targetOverlongType_, animationEndIndex_, true);
    animationEndIndicatorHeight_ =
        CalcIndicatorSize(itemHalfSizes, targetSelectedIndex_, targetOverlongType_, animationEndIndex_, false);

    animationStartCenterX_.resize(maxDisplayCount_);
    animationEndCenterX_.resize(maxDisplayCount_);
    animationStartIndicatorWidth_.resize(maxDisplayCount_);
    animationStartIndicatorHeight_.resize(maxDisplayCount_);
    animationEndIndicatorWidth_.resize(maxDisplayCount_);
    animationEndIndicatorHeight_.resize(maxDisplayCount_);

    UpdateSelectedCenterXOnDrag();
}

void OverlengthDotIndicatorModifier::CalcTargetStatusOnAllPointMoveForward(const LinearVector<float>& itemHalfSizes)
{
    auto targetCenterX =
        CalcIndicatorCenterX(itemHalfSizes, targetSelectedIndex_, targetOverlongType_, animationEndIndex_);
    overlongSelectedEndCenterX_ = targetCenterX.second;
    auto targetIndicatorWidth =
        CalcIndicatorSize(itemHalfSizes, targetSelectedIndex_, targetOverlongType_, animationEndIndex_, true);
    auto targetIndicatorHeight =
        CalcIndicatorSize(itemHalfSizes, targetSelectedIndex_, targetOverlongType_, animationEndIndex_, false);

    if (currentOverlongType_ != targetOverlongType_) {
        longPointWithAnimation_ = false;
    }

    float itemSpacePx = static_cast<float>(INDICATOR_ITEM_SPACE.ConvertToPx());
    // calc new point current position
    animationStartCenterX_[maxDisplayCount_] =
        animationStartCenterX_[0] - animationStartIndicatorWidth_[0] - itemSpacePx;
    animationStartIndicatorWidth_[maxDisplayCount_] = animationStartIndicatorWidth_[0];
    animationStartIndicatorHeight_[maxDisplayCount_] = animationStartIndicatorHeight_[0];

    // calc new point target position
    animationEndCenterX_[maxDisplayCount_] = targetCenterX.first[0];
    animationEndIndicatorWidth_[maxDisplayCount_] = targetIndicatorWidth[0];
    animationEndIndicatorHeight_[maxDisplayCount_] = targetIndicatorHeight[0];

    for (int32_t i = 0; i < maxDisplayCount_ - 1; i++) {
        animationEndCenterX_[i] = targetCenterX.first[i + 1];
        animationEndIndicatorWidth_[i] = targetIndicatorWidth[i + 1];
        animationEndIndicatorHeight_[i] = targetIndicatorHeight[i + 1];
    }

    animationEndCenterX_[maxDisplayCount_ - 1] =
        targetCenterX.first[maxDisplayCount_ - 1] + targetIndicatorWidth[maxDisplayCount_ - 1] + itemSpacePx;
    animationEndIndicatorWidth_[maxDisplayCount_ - 1] = targetIndicatorWidth[maxDisplayCount_ - 1];
    animationEndIndicatorHeight_[maxDisplayCount_ - 1] = targetIndicatorHeight[maxDisplayCount_ - 1];

    UpdateUnselectedCenterXOnDrag();
}

void OverlengthDotIndicatorModifier::CalcTargetStatusOnAllPointMoveBackward(const LinearVector<float>& itemHalfSizes)
{
    auto targetCenterX =
        CalcIndicatorCenterX(itemHalfSizes, targetSelectedIndex_, targetOverlongType_, animationEndIndex_);
    overlongSelectedEndCenterX_ = targetCenterX.second;
    auto targetIndicatorWidth =
        CalcIndicatorSize(itemHalfSizes, targetSelectedIndex_, targetOverlongType_, animationEndIndex_, true);
    auto targetIndicatorHeight =
        CalcIndicatorSize(itemHalfSizes, targetSelectedIndex_, targetOverlongType_, animationEndIndex_, false);

    float itemSpacePx = static_cast<float>(INDICATOR_ITEM_SPACE.ConvertToPx());
    // calc new point current position
    animationStartCenterX_[maxDisplayCount_] = animationStartCenterX_[maxDisplayCount_ - 1] +
                                               animationStartIndicatorWidth_[maxDisplayCount_ - 1] + itemSpacePx;
    animationStartIndicatorWidth_[maxDisplayCount_] = animationStartIndicatorWidth_[maxDisplayCount_ - 1];
    animationStartIndicatorHeight_[maxDisplayCount_] = animationStartIndicatorHeight_[maxDisplayCount_ - 1];

    // calc first point target position
    auto distance = std::abs(targetCenterX.first[1] - targetCenterX.first[0]);
    animationEndCenterX_[0] = targetCenterX.first[0] - distance;
    animationEndIndicatorWidth_[0] = targetIndicatorWidth[0];
    animationEndIndicatorHeight_[0] = targetIndicatorHeight[0];

    for (int32_t i = 1; i <= maxDisplayCount_; i++) {
        animationEndCenterX_[i] = targetCenterX.first[i - 1];
        animationEndIndicatorWidth_[i] = targetIndicatorWidth[i - 1];
        animationEndIndicatorHeight_[i] = targetIndicatorHeight[i - 1];
    }

    UpdateUnselectedCenterXOnDrag();
}

void OverlengthDotIndicatorModifier::CalcAnimationEndCenterX(const LinearVector<float>& itemHalfSizes)
{
    if (gestureState_ == GestureState::GESTURE_STATE_FOLLOW_LEFT ||
        gestureState_ == GestureState::GESTURE_STATE_FOLLOW_RIGHT) {
        animationEndIndex_ = CalcTargetIndexOnDrag();
    }

    CalcTargetOverlongStatus(animationStartIndex_, animationEndIndex_);

    auto startCenterX =
        CalcIndicatorCenterX(itemHalfSizes, currentSelectedIndex_, currentOverlongType_, animationStartIndex_);
    animationStartCenterX_ = startCenterX.first;
    overlongSelectedStartCenterX_ = startCenterX.second;

    // long point move or no move
    if (currentSelectedIndex_ != targetSelectedIndex_ || animationStartIndex_ == animationEndIndex_) {
        moveDirection_ = OverlongIndicatorMove::NONE;
        CalcTargetStatusOnLongPointMove(itemHalfSizes);
        return;
    }

    animationStartIndicatorWidth_ =
        CalcIndicatorSize(itemHalfSizes, currentSelectedIndex_, currentOverlongType_, animationStartIndex_, true);
    animationStartIndicatorHeight_ =
        CalcIndicatorSize(itemHalfSizes, currentSelectedIndex_, currentOverlongType_, animationStartIndex_, false);
    animationEndCenterX_.resize(maxDisplayCount_ + 1);
    animationEndIndicatorWidth_.resize(maxDisplayCount_ + 1);
    animationEndIndicatorHeight_.resize(maxDisplayCount_ + 1);

    if (animationStartIndex_ < animationEndIndex_) {
        moveDirection_ = OverlongIndicatorMove::MOVE_BACKWARD;
        CalcTargetStatusOnAllPointMoveBackward(itemHalfSizes);
        return;
    }

    moveDirection_ = OverlongIndicatorMove::MOVE_FORWARD;
    CalcTargetStatusOnAllPointMoveForward(itemHalfSizes);
}

void OverlengthDotIndicatorModifier::PlayIndicatorAnimation(const OffsetF& margin,
    const LinearVector<float>& itemHalfSizes, GestureState gestureState, TouchBottomTypeLoop touchBottomTypeLoop)
{
    StopAnimation(false);
    isTouchBottomLoop_ = false;
    animationState_ = TouchBottomAnimationStage::STAGE_NONE;
    normalMargin_ = margin;
    CalcAnimationEndCenterX(itemHalfSizes);
    PlayBlackPointsAnimation(itemHalfSizes);

    std::vector<std::pair<float, float>> pointCenterX;
    if ((currentSelectedIndex_ == 0 && targetSelectedIndex_ == maxDisplayCount_ - 1) ||
        (currentSelectedIndex_ == maxDisplayCount_ - 1 && targetSelectedIndex_ == 0)) {
        overlongSelectedStartCenterX_.first = animationEndCenterX_[currentSelectedIndex_];
        overlongSelectedStartCenterX_.second = animationEndCenterX_[currentSelectedIndex_];
        pointCenterX.emplace_back(overlongSelectedStartCenterX_);
        pointCenterX.emplace_back(overlongSelectedEndCenterX_);
    } else {
        pointCenterX.emplace_back(overlongSelectedEndCenterX_);
    }

    if (longPointWithAnimation_) {
        PlayLongPointAnimation(pointCenterX, gestureState, touchBottomTypeLoop, animationEndCenterX_);
    }
    longPointWithAnimation_ = true;
}

void OverlengthDotIndicatorModifier::StopAnimation(bool ifImmediately)
{
    if (ifImmediately) {
        AnimationOption option;
        option.SetDuration(0);
        option.SetCurve(Curves::LINEAR);
        AnimationUtils::StartAnimation(option, [weak = WeakClaim(this)]() {
            auto modifier = weak.Upgrade();
            CHECK_NULL_VOID(modifier);
            modifier->longPointLeftCenterX_->Set(modifier->longPointLeftCenterX_->Get());
            modifier->longPointRightCenterX_->Set(modifier->longPointRightCenterX_->Get());
        });
    }

    AnimationOption option;
    option.SetDuration(0);
    option.SetCurve(Curves::LINEAR);
    AnimationUtils::StartAnimation(option, [weak = WeakClaim(this)]() {
        auto modifier = weak.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->vectorBlackPointCenterX_->Set(modifier->vectorBlackPointCenterX_->Get());
        modifier->firstPointOpacity_->Set(modifier->firstPointOpacity_->Get());
        modifier->newPointOpacity_->Set(modifier->newPointOpacity_->Get());
        modifier->unselectedIndicatorWidth_->Set(modifier->unselectedIndicatorWidth_->Get());
        modifier->unselectedIndicatorHeight_->Set(modifier->unselectedIndicatorHeight_->Get());
    });

    longPointLeftAnimEnd_ = true;
    longPointRightAnimEnd_ = true;
    ifNeedFinishCallback_ = false;
    currentSelectedIndex_ = targetSelectedIndex_;
    currentOverlongType_ = targetOverlongType_;
}

void OverlengthDotIndicatorModifier::InitOverlongStatus(int32_t pageIndex)
{
    if (pageIndex < maxDisplayCount_ - 1 - THIRD_POINT_INDEX) {
        currentSelectedIndex_ = pageIndex;
        currentOverlongType_ = OverlongType::LEFT_NORMAL_RIGHT_FADEOUT;

        targetSelectedIndex_ = currentSelectedIndex_;
        targetOverlongType_ = currentOverlongType_;
        return;
    }

    if (pageIndex >= maxDisplayCount_ - 1 - THIRD_POINT_INDEX && pageIndex < realItemCount_ - 1 - THIRD_POINT_INDEX) {
        currentSelectedIndex_ = maxDisplayCount_ - 1 - THIRD_POINT_INDEX;
        currentOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_FADEOUT;

        targetSelectedIndex_ = currentSelectedIndex_;
        targetOverlongType_ = currentOverlongType_;
        return;
    }

    if (pageIndex >= maxDisplayCount_ - 1 - THIRD_POINT_INDEX && pageIndex == realItemCount_ - 1 - THIRD_POINT_INDEX) {
        currentSelectedIndex_ = maxDisplayCount_ - 1 - THIRD_POINT_INDEX;
        currentOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_NORMAL;

        targetSelectedIndex_ = currentSelectedIndex_;
        targetOverlongType_ = currentOverlongType_;
        return;
    }

    if (pageIndex >= maxDisplayCount_ - 1 - THIRD_POINT_INDEX && pageIndex == realItemCount_ - 1 - SECOND_POINT_INDEX) {
        currentSelectedIndex_ = maxDisplayCount_ - 1 - SECOND_POINT_INDEX;
        currentOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_NORMAL;

        targetSelectedIndex_ = currentSelectedIndex_;
        targetOverlongType_ = currentOverlongType_;
        return;
    }

    if (pageIndex >= maxDisplayCount_ - 1 - THIRD_POINT_INDEX && pageIndex == realItemCount_ - 1) {
        currentSelectedIndex_ = maxDisplayCount_ - 1;
        currentOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_NORMAL;

        targetSelectedIndex_ = currentSelectedIndex_;
        targetOverlongType_ = currentOverlongType_;
        return;
    }
}

void OverlengthDotIndicatorModifier::CalcTargetSelectedIndex(int32_t currentPageIndex, int32_t targetPageIndex)
{
    if (currentPageIndex == targetPageIndex) {
        return;
    }

    auto step = std::abs(targetPageIndex - currentPageIndex);
    auto rightThirdIndicatorIndex = maxDisplayCount_ - 1 - THIRD_POINT_INDEX;
    auto rightSecondPageIndex = realItemCount_ - 1 - SECOND_POINT_INDEX;
    if (currentPageIndex < targetPageIndex) {
        if (currentSelectedIndex_ == rightThirdIndicatorIndex) {
            if (targetPageIndex < rightSecondPageIndex) {
                step = 0;
            } else {
                step = targetPageIndex - currentPageIndex;
            }
        } else if (currentSelectedIndex_ < rightThirdIndicatorIndex) {
            if (targetPageIndex < rightSecondPageIndex) {
                step = std::min(targetPageIndex - currentPageIndex, rightThirdIndicatorIndex - currentSelectedIndex_);
            } else if (targetPageIndex == rightSecondPageIndex) {
                step = rightThirdIndicatorIndex - currentSelectedIndex_ + 1;
            } else {
                step = rightThirdIndicatorIndex - currentSelectedIndex_ + THIRD_POINT_INDEX;
            }
        } else {
            step = targetPageIndex - currentPageIndex;
        }

        targetSelectedIndex_ = currentSelectedIndex_ + step;
        return;
    }

    if (currentSelectedIndex_ > THIRD_POINT_INDEX) {
        if (targetPageIndex > SECOND_POINT_INDEX) {
            step = std::min(currentPageIndex - targetPageIndex, currentSelectedIndex_ - THIRD_POINT_INDEX);
        } else if (targetPageIndex == SECOND_POINT_INDEX) {
            step = currentSelectedIndex_ - SECOND_POINT_INDEX;
        } else {
            step = currentSelectedIndex_ - LEFT_FIRST_POINT_INDEX;
        }
    } else if (currentSelectedIndex_ == THIRD_POINT_INDEX) {
        if (targetPageIndex > SECOND_POINT_INDEX) {
            step = 0;
        } else {
            step = currentPageIndex - targetPageIndex;
        }
    } else {
        step = currentPageIndex - targetPageIndex;
    }

    targetSelectedIndex_ = currentSelectedIndex_ - step;
}

void OverlengthDotIndicatorModifier::CalcTargetOverlongStatus(int32_t currentPageIndex, int32_t targetPageIndex)
{
    if (currentPageIndex == targetPageIndex || currentOverlongType_ == OverlongType::NONE) {
        return;
    }

    if (currentPageIndex == realItemCount_ - 1 && targetPageIndex == 0) {
        targetSelectedIndex_ = 0;
        targetOverlongType_ = OverlongType::LEFT_NORMAL_RIGHT_FADEOUT;
        return;
    }

    if (currentPageIndex == 0 && targetPageIndex == realItemCount_ - 1) {
        targetSelectedIndex_ = maxDisplayCount_ - 1;
        targetOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_NORMAL;
        return;
    }

    CalcTargetSelectedIndex(currentPageIndex, targetPageIndex);

    if (currentPageIndex < targetPageIndex) {
        if (currentOverlongType_ == OverlongType::LEFT_NORMAL_RIGHT_FADEOUT) {
            if (targetSelectedIndex_ < maxDisplayCount_ - 1 - THIRD_POINT_INDEX) {
                targetOverlongType_ = OverlongType::LEFT_NORMAL_RIGHT_FADEOUT;
            } else if (targetSelectedIndex_ == maxDisplayCount_ - 1 - THIRD_POINT_INDEX) {
                targetOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_FADEOUT;
            }
        } else if (currentOverlongType_ == OverlongType::LEFT_FADEOUT_RIGHT_NORMAL) {
            targetOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_NORMAL;
        } else {
            if (targetSelectedIndex_ < maxDisplayCount_ - 1 - THIRD_POINT_INDEX) {
                targetOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_FADEOUT;
            } else if (targetSelectedIndex_ == maxDisplayCount_ - 1 - THIRD_POINT_INDEX) {
                if (targetPageIndex < realItemCount_ - 1 - THIRD_POINT_INDEX) {
                    targetOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_FADEOUT;
                } else {
                    targetOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_NORMAL;
                }
            } else {
                targetOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_NORMAL;
            }
        }

        return;
    }

    if (currentOverlongType_ == OverlongType::LEFT_NORMAL_RIGHT_FADEOUT) {
        targetOverlongType_ = OverlongType::LEFT_NORMAL_RIGHT_FADEOUT;
    } else if (currentOverlongType_ == OverlongType::LEFT_FADEOUT_RIGHT_NORMAL) {
        if (targetSelectedIndex_ > THIRD_POINT_INDEX) {
            targetOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_NORMAL;
        } else if (targetSelectedIndex_ == THIRD_POINT_INDEX) {
            targetOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_FADEOUT;
        }
    } else {
        if (targetSelectedIndex_ > THIRD_POINT_INDEX) {
            targetOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_FADEOUT;
        } else if (targetSelectedIndex_ == THIRD_POINT_INDEX) {
            if (targetPageIndex > THIRD_POINT_INDEX) {
                targetOverlongType_ = OverlongType::LEFT_FADEOUT_RIGHT_FADEOUT;
            } else {
                targetOverlongType_ = OverlongType::LEFT_NORMAL_RIGHT_FADEOUT;
            }
        } else {
            targetOverlongType_ = OverlongType::LEFT_NORMAL_RIGHT_FADEOUT;
        }
    }
}

std::pair<LinearVector<float>, std::pair<float, float>> OverlengthDotIndicatorModifier::CalcIndicatorCenterX(
    const LinearVector<float>& itemHalfSizes, int32_t selectedIndex, OverlongType overlongType, int32_t pageIndex)
{
    auto unselectedIndicatorRadius = itemHalfSizes[ITEM_HALF_WIDTH];
    auto selectedIndicatorRadius = itemHalfSizes[SELECTED_ITEM_HALF_WIDTH];
    if (!isCustomSizeValue_) {
        selectedIndicatorRadius *= 2.0f;
    }

    LinearVector<float> indicatorCenterX(maxDisplayCount_ + 1);
    std::pair<float, float> longPointCenterX;
    float itemSpacePx = static_cast<float>(INDICATOR_ITEM_SPACE.ConvertToPx());
    auto leftFirstRadius = unselectedIndicatorRadius * SMALLEST_POINT_RATIO;
    auto leftSecondRadius = unselectedIndicatorRadius * SECOND_SMALLEST_POINT_RATIO;
    auto rightFirstRadius = unselectedIndicatorRadius * SMALLEST_POINT_RATIO;
    auto rightSecondRadius = unselectedIndicatorRadius * SECOND_SMALLEST_POINT_RATIO;

    auto startIndicatorCenterX = normalMargin_.GetX() + static_cast<float>(INDICATOR_PADDING_DEFAULT.ConvertToPx());
    for (int32_t i = 0; i < maxDisplayCount_; i++) {
        if (i == LEFT_FIRST_POINT_INDEX) {
            if (i == selectedIndex) {
                startIndicatorCenterX += selectedIndicatorRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += selectedIndicatorRadius;

                longPointCenterX.first =
                    indicatorCenterX[i] - (isCustomSizeValue_ ? 0.0f : selectedIndicatorRadius * 0.5f);
                longPointCenterX.second =
                    indicatorCenterX[i] + (isCustomSizeValue_ ? 0.0f : selectedIndicatorRadius * 0.5f);
            } else if (overlongType == OverlongType::LEFT_NORMAL_RIGHT_FADEOUT) {
                startIndicatorCenterX += unselectedIndicatorRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += unselectedIndicatorRadius;
            } else {
                startIndicatorCenterX += leftFirstRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += leftFirstRadius;
            }
            continue;
        }

        if (i == SECOND_POINT_INDEX) {
            if (i == selectedIndex) {
                startIndicatorCenterX += itemSpacePx + selectedIndicatorRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += selectedIndicatorRadius;

                longPointCenterX.first =
                    indicatorCenterX[i] - (isCustomSizeValue_ ? 0.0f : selectedIndicatorRadius * 0.5f);
                longPointCenterX.second =
                    indicatorCenterX[i] + (isCustomSizeValue_ ? 0.0f : selectedIndicatorRadius * 0.5f);
            } else if (overlongType == OverlongType::LEFT_NORMAL_RIGHT_FADEOUT) {
                startIndicatorCenterX += itemSpacePx + unselectedIndicatorRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += unselectedIndicatorRadius;
            } else {
                startIndicatorCenterX += itemSpacePx + leftSecondRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += leftSecondRadius;
            }
            continue;
        }

        if (i >= THIRD_POINT_INDEX && i <= maxDisplayCount_ - 1 - THIRD_POINT_INDEX) {
            if (i == selectedIndex) {
                startIndicatorCenterX += itemSpacePx + selectedIndicatorRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += selectedIndicatorRadius;

                longPointCenterX.first =
                    indicatorCenterX[i] - (isCustomSizeValue_ ? 0.0f : selectedIndicatorRadius * 0.5f);
                longPointCenterX.second =
                    indicatorCenterX[i] + (isCustomSizeValue_ ? 0.0f : selectedIndicatorRadius * 0.5f);
            } else {
                startIndicatorCenterX += itemSpacePx + unselectedIndicatorRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += unselectedIndicatorRadius;
            }
            continue;
        }

        if (i == maxDisplayCount_ - 1 - SECOND_POINT_INDEX) {
            if (i == selectedIndex) {
                startIndicatorCenterX += itemSpacePx + selectedIndicatorRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += selectedIndicatorRadius;

                longPointCenterX.first =
                    indicatorCenterX[i] - (isCustomSizeValue_ ? 0.0f : selectedIndicatorRadius * 0.5f);
                longPointCenterX.second =
                    indicatorCenterX[i] + (isCustomSizeValue_ ? 0.0f : selectedIndicatorRadius * 0.5f);
            } else if (overlongType == OverlongType::LEFT_FADEOUT_RIGHT_NORMAL) {
                startIndicatorCenterX += itemSpacePx + unselectedIndicatorRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += unselectedIndicatorRadius;
            } else {
                startIndicatorCenterX += itemSpacePx + rightSecondRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += rightSecondRadius;
            }
            continue;
        }

        if (i == maxDisplayCount_ - 1) {
            if (i == selectedIndex) {
                startIndicatorCenterX += itemSpacePx + selectedIndicatorRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += selectedIndicatorRadius;

                longPointCenterX.first =
                    indicatorCenterX[i] - (isCustomSizeValue_ ? 0.0f : selectedIndicatorRadius * 0.5f);
                longPointCenterX.second =
                    indicatorCenterX[i] + (isCustomSizeValue_ ? 0.0f : selectedIndicatorRadius * 0.5f);
            } else if (overlongType == OverlongType::LEFT_FADEOUT_RIGHT_NORMAL) {
                startIndicatorCenterX += itemSpacePx + unselectedIndicatorRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += unselectedIndicatorRadius;
            } else {
                startIndicatorCenterX += itemSpacePx + rightFirstRadius;
                indicatorCenterX[i] = startIndicatorCenterX;
                startIndicatorCenterX += rightFirstRadius;
            }
            continue;
        }
    }

    return std::make_pair(indicatorCenterX, longPointCenterX);
}

} // namespace OHOS::Ace::NG
