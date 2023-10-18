/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "core/components/select_popup/flutter_render_select_popup.h"

#include "include/core/SkMaskFilter.h"
#include "include/effects/SkGradientShader.h"

#include "core/components/common/painter/flutter_decoration_painter.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/pipeline/base/scoped_canvas_state.h"

namespace OHOS::Ace {
namespace {

const Dimension GRADIENT_HEIGHT = Dimension(50, DimensionUnit::VP);
const uint8_t GRADIENT_END_GRADIENT = 255;

} // namespace

double FlutterRenderSelectPopup::GetOptionLeft() const
{
    if (renderOptions_.empty()) {
        return 0.0;
    }

    auto option = renderOptions_[0];
    if (!option) {
        return 0.0;
    }

    return option->GetGlobalOffset().GetX();
}

double FlutterRenderSelectPopup::GetOptionTop() const
{
    auto scroll = renderScroll_;
    if (!scroll) {
        return 0.0;
    }

    return scroll->GetGlobalOffset().GetY();
}

double FlutterRenderSelectPopup::GetOptionBottom() const
{
    auto scroll = renderScroll_;
    if (!scroll) {
        return 0.0;
    }

    return scroll->GetGlobalOffset().GetY() + scroll->GetLayoutSize().Height();
}

double FlutterRenderSelectPopup::GetOptionWidth() const
{
    if (renderOptions_.empty()) {
        return 0.0;
    }

    auto option = renderOptions_[0];
    if (!option) {
        return 0.0;
    }

    return option->GetLayoutSize().Width();
}

void FlutterRenderSelectPopup::PaintGradient(RenderContext& context, bool isTop)
{
    auto canvas = ScopedCanvas::Create(context);
    if (!canvas) {
        LOGE("Paint canvas is null");
        return;
    }
    flutter::Paint paintGradient;
    flutter::PaintData paintDataGradient;
    Rect gradientRect;
    auto interval = NormalizeToPx(optionInterval_);
    auto yPos = isTop ? GetOptionTop() : GetOptionBottom() - NormalizeToPx(GRADIENT_HEIGHT);
    gradientRect.SetOffset(Offset(GetOptionLeft() + interval, yPos));
    gradientRect.SetHeight(NormalizeToPx(GRADIENT_HEIGHT));
    gradientRect.SetWidth(GetOptionWidth() - 2 * interval);
    SkPoint beginPoint = SkPoint::Make(SkDoubleToScalar(gradientRect.Left()), SkDoubleToScalar(gradientRect.Top()));
    SkPoint endPoint = SkPoint::Make(SkDoubleToScalar(gradientRect.Left()), SkDoubleToScalar(gradientRect.Bottom()));
    SkPoint points[2] = { beginPoint, endPoint };
    // color with red 13, green 13, and blue 13 is used for color stop (0.85) in gradient
    SkColor colors[2] = { tvBackColor_.ChangeAlpha(0).GetValue(),
        Color::FromRGB(13, 13, 13).ChangeAlpha(GRADIENT_END_GRADIENT).GetValue() };
    if (isTop) {
        colors[0] = Color::FromRGB(13, 13, 13).ChangeAlpha(GRADIENT_END_GRADIENT).GetValue();
        colors[1] = tvBackColor_.ChangeAlpha(0).GetValue();
    }
    const float stopPositions[2] = { 0.0f, 0.85f };

    paintGradient.paint()->setShader(
        SkGradientShader::MakeLinear(points, colors, stopPositions, std::size(colors), SkTileMode::kMirror));
    canvas->drawRect(gradientRect.Left(), gradientRect.Top(), gradientRect.Right(), gradientRect.Bottom(),
        paintGradient, paintDataGradient);
}

void FlutterRenderSelectPopup::Paint(RenderContext& context, const Offset& offset)
{
    if (!isFullScreen_ && isWattingForAnimationStart_) {
        return;
    }

    {
        auto canvas = ScopedCanvas::Create(context);
        if (!canvas) {
            LOGE("Paint canvas is null");
            return;
        }
        flutter::Paint paint;
        paint.paint()->setARGB(
            tvBackColor_.GetAlpha(), tvBackColor_.GetRed(), tvBackColor_.GetGreen(), tvBackColor_.GetBlue());
        flutter::PaintData paintData;
        auto size = GetLayoutSize();
        canvas->drawRect(0, 0, size.Width(), size.Height(), paint, paintData);
    }
    RenderNode::Paint(context, offset);

    if (!renderScroll_ || SystemProperties::GetDeviceType() != DeviceType::TV) {
        return;
    }
    if (!renderScroll_->IsAtBottom()) {
        PaintGradient(context, false);
    }

    if (!renderScroll_->IsAtTop()) {
        PaintGradient(context, true);
    }
}

} // namespace OHOS::Ace
