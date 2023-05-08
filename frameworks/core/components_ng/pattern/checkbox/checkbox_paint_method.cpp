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
#include "core/components_ng/pattern/checkbox/checkbox_paint_method.h"

#include <optional>

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/rect.h"
#include "base/geometry/rrect.h"
#include "base/utils/utils.h"
#include "core/components/checkable/checkable_theme.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/common/properties/color.h"
#include "core/components/theme/theme_manager.h"
#include "core/components_ng/pattern/checkbox/checkbox_paint_property.h"
#include "core/components_ng/pattern/checkbox/checkbox_pattern.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"

namespace OHOS::Ace::NG {
namespace {
constexpr uint8_t ENABLED_ALPHA = 255;
constexpr uint8_t DISABLED_ALPHA = 102;
constexpr float CHECK_MARK_START_X_POSITION = 0.25f;
constexpr float CHECK_MARK_START_Y_POSITION = 0.49f;
constexpr float CHECK_MARK_MIDDLE_X_POSITION = 0.44f;
constexpr float CHECK_MARK_MIDDLE_Y_POSITION = 0.68f;
constexpr float CHECK_MARK_END_X_POSITION = 0.76f;
constexpr float CHECK_MARK_END_Y_POSITION = 0.33f;
constexpr float CHECKBOX_DOUBLE_RATIO = 2.0f;
constexpr float CHECKBOX_LENGTH_ZERO = 0.0f;
} // namespace

CheckBoxModifier::CheckBoxModifier(
    bool isSelect, const Color& boardColor, const Color& checkColor, const Color& borderColor, const Color& shadowColor)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto checkBoxTheme = pipeline->GetTheme<CheckboxTheme>();
    CHECK_NULL_VOID(checkBoxTheme);

    animatableBoardColor_ = AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(boardColor));
    animatableCheckColor_ = AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(checkColor));
    animatableBorderColor_ = AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(borderColor));
    animatableShadowColor_ = AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(shadowColor));
    checkStroke_ =
        AceType::MakeRefPtr<AnimatablePropertyFloat>(static_cast<float>(checkBoxTheme->GetCheckStroke().ConvertToPx()));
    strokeSize_ =
        AceType::MakeRefPtr<AnimatablePropertyFloat>(static_cast<float>(checkBoxTheme->GetWidth().ConvertToPx()));
    animateTouchHoverColor_ = AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(Color::TRANSPARENT));
    isSelect_ = AceType::MakeRefPtr<PropertyBool>(isSelect);
    offset_ = AceType::MakeRefPtr<AnimatablePropertyOffsetF>(OffsetF());
    size_ = AceType::MakeRefPtr<AnimatablePropertySizeF>(SizeF());
    enabled_ = AceType::MakeRefPtr<PropertyBool>(true);

    AttachProperty(animatableBoardColor_);
    AttachProperty(animatableCheckColor_);
    AttachProperty(animatableBorderColor_);
    AttachProperty(animatableShadowColor_);
    AttachProperty(animateTouchHoverColor_);
    AttachProperty(checkStroke_);
    AttachProperty(strokeSize_);
    AttachProperty(isSelect_);
    AttachProperty(offset_);
    AttachProperty(size_);
    AttachProperty(enabled_);
}

void CheckBoxModifier::InitializeParam()
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto checkBoxTheme = pipeline->GetTheme<CheckboxTheme>();
    CHECK_NULL_VOID(checkBoxTheme);
    borderWidth_ = checkBoxTheme->GetBorderWidth().ConvertToPx();
    borderRadius_ = checkBoxTheme->GetBorderRadius().ConvertToPx();
    pointColor_ = checkBoxTheme->GetPointColor();
    activeColor_ = checkBoxTheme->GetActiveColor();
    inactiveColor_ = checkBoxTheme->GetInactiveColor();
    inactivePointColor_ = checkBoxTheme->GetInactivePointColor();
    shadowColor_ = checkBoxTheme->GetShadowColor();
    clickEffectColor_ = checkBoxTheme->GetClickEffectColor();
    hoverColor_ = checkBoxTheme->GetHoverColor();
    hoverRadius_ = checkBoxTheme->GetHoverRadius();
    hotZoneHorizontalPadding_ = checkBoxTheme->GetHotZoneHorizontalPadding();
    hotZoneVerticalPadding_ = checkBoxTheme->GetHotZoneVerticalPadding();
    shadowWidth_ = checkBoxTheme->GetShadowWidth();
    userActiveColor_ = activeColor_;
    hoverDuration_ = checkBoxTheme->GetHoverDuration();
    hoverToTouchDuration_ = checkBoxTheme->GetHoverToTouchDuration();
    touchDuration_ = checkBoxTheme->GetTouchDuration();
    colorAnimationDuration_ = checkBoxTheme->GetColorAnimationDuration();
}

void CheckBoxModifier::PaintCheckBox(RSCanvas& canvas, const OffsetF& paintOffset, const SizeF& contentSize) const
{
    RSPen pen;
    RSBrush brush;
    pen.SetWidth(borderWidth_);
    pen.SetAntiAlias(true);
    DrawTouchAndHoverBoard(canvas, contentSize, paintOffset);
    RSPen shadowPen = RSPen(pen);
    brush.SetColor(ToRSColor(animatableBoardColor_->Get()));
    brush.SetAntiAlias(true);
    if (!enabled_->Get()) {
        brush.SetColor(
            ToRSColor(animatableBoardColor_->Get().BlendOpacity(static_cast<float>(DISABLED_ALPHA) / ENABLED_ALPHA)));
    }
    DrawBackboard(canvas, paintOffset, brush, contentSize);
    pen.SetColor(ToRSColor(animatableBorderColor_->Get()));
    if (!enabled_->Get()) {
        pen.SetColor(
            ToRSColor(animatableBorderColor_->Get().BlendOpacity(static_cast<float>(DISABLED_ALPHA) / ENABLED_ALPHA)));
    }
    DrawBorder(canvas, paintOffset, pen, contentSize);
    pen.SetColor(ToRSColor(animatableCheckColor_->Get()));
    if (!enabled_->Get()) {
        pen.SetColor(
            ToRSColor(animatableCheckColor_->Get().BlendOpacity(static_cast<float>(DISABLED_ALPHA) / ENABLED_ALPHA)));
    }
    shadowPen.SetColor(ToRSColor(animatableShadowColor_->Get()));
    DrawCheck(canvas, paintOffset, pen, shadowPen, contentSize);
}

void CheckBoxModifier::DrawTouchAndHoverBoard(RSCanvas& canvas, const SizeF& size, const OffsetF& offset) const
{
    RSBrush brush;
    brush.SetColor(ToRSColor(animateTouchHoverColor_->Get()));
    brush.SetAntiAlias(true);
    float originX = offset.GetX() - hotZoneHorizontalPadding_.ConvertToPx();
    float originY = offset.GetY() - hotZoneVerticalPadding_.ConvertToPx();
    float endX = size.Width() + originX + CHECKBOX_DOUBLE_RATIO * hotZoneHorizontalPadding_.ConvertToPx();
    float endY = size.Height() + originY + CHECKBOX_DOUBLE_RATIO * hotZoneVerticalPadding_.ConvertToPx();
    auto rrect = RSRoundRect({ originX, originY, endX, endY }, hoverRadius_.ConvertToPx(), hoverRadius_.ConvertToPx());
    canvas.AttachBrush(brush);
    canvas.DrawRoundRect(rrect);
}

void CheckBoxModifier::DrawBorder(RSCanvas& canvas, const OffsetF& origin, RSPen& pen, const SizeF& paintSize) const
{
    float originX = origin.GetX() + borderWidth_ / CHECKBOX_DOUBLE_RATIO;
    float originY = origin.GetY() + borderWidth_ / CHECKBOX_DOUBLE_RATIO;
    float endX = originX + paintSize.Width() - borderWidth_;
    float endY = originY + paintSize.Height() - borderWidth_;
    auto rrect = RSRoundRect({ originX, originY, endX, endY }, borderRadius_, borderRadius_);
    canvas.AttachPen(pen);
    canvas.DrawRoundRect(rrect);
}

void CheckBoxModifier::DrawBackboard(
    RSCanvas& canvas, const OffsetF& origin, RSBrush& brush, const SizeF& paintSize) const
{
    float originX = origin.GetX();
    float originY = origin.GetY();
    float endX = originX + paintSize.Width();
    float endY = originY + paintSize.Height();
    auto rrect = RSRoundRect({ originX, originY, endX, endY }, borderRadius_, borderRadius_);
    canvas.AttachBrush(brush);
    canvas.DrawRoundRect(rrect);
}

void CheckBoxModifier::DrawCheck(
    RSCanvas& canvas, const OffsetF& origin, RSPen& pen, RSPen& shadowPen, const SizeF& paintSize) const
{
    if (strokeSize_->Get() == CHECKBOX_LENGTH_ZERO || checkStroke_->Get() == CHECKBOX_LENGTH_ZERO) {
        return;
    }
    RSPath path;
    float originX = origin.GetX();
    float originY = origin.GetY();
    float strokeSize = strokeSize_->Get();
    const Offset start = Offset(strokeSize * CHECK_MARK_START_X_POSITION, strokeSize * CHECK_MARK_START_Y_POSITION);
    const Offset middle = Offset(strokeSize * CHECK_MARK_MIDDLE_X_POSITION, strokeSize * CHECK_MARK_MIDDLE_Y_POSITION);
    const Offset end = Offset(strokeSize * CHECK_MARK_END_X_POSITION, strokeSize * CHECK_MARK_END_Y_POSITION);
    path.MoveTo(originX + start.GetX() + (paintSize.Width() - strokeSize) / CHECKBOX_DOUBLE_RATIO,
        originY + start.GetY() + (paintSize.Height() - strokeSize) / CHECKBOX_DOUBLE_RATIO);
    path.LineTo(originX + middle.GetX() + (paintSize.Width() - strokeSize) / CHECKBOX_DOUBLE_RATIO,
        originY + middle.GetY() + (paintSize.Height() - strokeSize) / CHECKBOX_DOUBLE_RATIO);
    path.MoveTo(originX + middle.GetX() + (paintSize.Width() - strokeSize) / CHECKBOX_DOUBLE_RATIO,
        originY + middle.GetY() + (paintSize.Height() - strokeSize) / CHECKBOX_DOUBLE_RATIO);
    path.LineTo(originX + end.GetX() + (paintSize.Width() - strokeSize) / CHECKBOX_DOUBLE_RATIO,
        originY + end.GetY() + (paintSize.Height() - strokeSize) / CHECKBOX_DOUBLE_RATIO);
    shadowPen.SetCapStyle(RSPen::CapStyle::ROUND_CAP);
    shadowPen.SetWidth(checkStroke_->Get() + shadowWidth_.ConvertToPx() * CHECKBOX_DOUBLE_RATIO);
    pen.SetWidth(checkStroke_->Get());
    pen.SetCapStyle(RSPen::CapStyle::ROUND_CAP);
    canvas.AttachPen(shadowPen);
    canvas.DrawPath(path);
    canvas.AttachPen(pen);
    canvas.DrawPath(path);
}

} // namespace OHOS::Ace::NG
