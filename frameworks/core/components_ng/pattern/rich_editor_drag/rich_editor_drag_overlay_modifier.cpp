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

#include "core/components_ng/pattern/rich_editor_drag/rich_editor_drag_overlay_modifier.h"

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/rect.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#include "core/components_ng/pattern/rich_editor_drag/rich_editor_drag_pattern.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/render/adapter/pixelmap_image.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing_prop_convertor.h"

namespace OHOS::Ace::NG {
constexpr int32_t HANDLE_RING_DEGREE = 360;
constexpr int32_t FLOATING_ANIMATE_HANDLE_OPACITY_DURATION = 150;
constexpr int32_t FLOATING_ANIMATE_BACKGROUND_CHANGE_DURATION = 250;
constexpr int32_t FLOATING_CANCEL_ANIMATE_TEXT_RECOVERY_DELAY_DURATION = 100;
constexpr int32_t FLOATING_CANCEL_ANIMATE_TEXT_RECOVERY_DURATION = 200;
void RichEditorDragOverlayModifier::onDraw(DrawingContext& context)
{
    auto pattern = DynamicCast<RichEditorDragPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(pattern);
    auto& canvas = context.canvas;
    auto textDragPattern = DynamicCast<TextDragPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textDragPattern);
    auto hostPattern = hostPattern_.Upgrade();
    auto richEditor = DynamicCast<RichEditorPattern>(hostPattern);
    PaintBackground(canvas, textDragPattern, richEditor);
    PaintSelBackground(canvas, textDragPattern, richEditor);
    CHECK_NULL_VOID(hostPattern);

    canvas.Save();
    canvas.ClipPath(*pattern->GetClipPath(), RSClipOp::INTERSECT, true);
    if (richEditor) {
        OffsetF offset = { pattern->GetTextRect().GetX(), pattern->GetTextRect().GetY() };
        for (auto&& info : richEditor->GetParagraphs()) {
            info.paragraph->Paint(canvas, offset.GetX(), offset.GetY());
            offset.AddY(info.paragraph->GetHeight());
        }
    } else {
        auto&& paragraph = hostPattern->GetParagraph();
        paragraph->Paint(canvas, pattern->GetTextRect().GetX(), pattern->GetTextRect().GetY());
    }
    canvas.Restore();

    PaintImage(context);
    if (firstHandle_) {
        auto selectPosition = pattern->GetSelectPosition();
        auto rect = firstHandle_->Get();
        auto startY = rect.Top() - selectPosition.globalY_;
        PaintHandle(canvas, firstHandle_->Get(), true, selectPosition.startX_, startY);
    }
    if (secondHandle_) {
        auto selectPosition = pattern->GetSelectPosition();
        auto rect = secondHandle_->Get();
        auto startY = rect.Bottom() - selectPosition.globalY_;
        PaintHandle(canvas, secondHandle_->Get(), false, selectPosition.endX_, startY);
    }
}

void RichEditorDragOverlayModifier::PaintImage(DrawingContext& context)
{
    auto pattern = DynamicCast<RichEditorDragPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(pattern);
    auto& canvas = context.canvas;
    size_t index = 0;
    auto imageChildren = pattern->GetImageChildren();
    auto rectsForPlaceholders = pattern->GetRectsForPlaceholders();
    for (const auto& child : imageChildren) {
        auto rect = rectsForPlaceholders.at(index);
        auto offset = OffsetF(rect.Left(), rect.Top()) + pattern->GetTextRect().GetOffset();
        auto imageChild = DynamicCast<ImagePattern>(child->GetPattern());
        if (imageChild) {
            auto pixelMap = child->GetRenderContext()->GetThumbnailPixelMap();
            CHECK_NULL_VOID(pixelMap);
            auto canvasImage = CanvasImage::Create(pixelMap);
            CHECK_NULL_VOID(canvasImage);
            auto layoutProperty = imageChild->GetLayoutProperty<ImageLayoutProperty>();
            CHECK_NULL_VOID(layoutProperty);
            const auto& marginProperty = layoutProperty->GetMarginProperty();
            float marginTop = 0.0f;
            float marginLeft = 0.0f;
            if (marginProperty) {
                marginLeft =
                    marginProperty->left.has_value() ? marginProperty->left->GetDimension().ConvertToPx() : 0.0f;
                marginTop = marginProperty->top.has_value() ? marginProperty->top->GetDimension().ConvertToPx() : 0.0f;
            }
            RectF imageRect(
                offset.GetX() + marginLeft, offset.GetY() + marginTop, pixelMap->GetWidth(), pixelMap->GetHeight());
            CHECK_NULL_VOID(canvasImage);
            auto pixelMapImage = DynamicCast<PixelMapImage>(canvasImage);
            CHECK_NULL_VOID(pixelMapImage);
            pixelMapImage->DrawRect(canvas, ToRSRect(imageRect));
        }
        ++index;
    }
    canvas.DetachBrush();
}

void RichEditorDragOverlayModifier::PaintBackground(RSCanvas& canvas, RefPtr<TextDragPattern> textDragPattern,
    RefPtr<RichEditorPattern> richEditorPattern)
{
    std::shared_ptr<RSPath> path;
    if (isAnimating_) {
        path = textDragPattern->GenerateBackgroundPath(backgroundOffset_->Get());
    } else {
        path = textDragPattern->GetBackgroundPath();
    }
    Color color(TEXT_DRAG_COLOR_BG);
    if (richEditorPattern && type_ != DragAnimType::DEFAULT) {
        color = color.BlendOpacity(1 - selectedBackgroundOpacity_->Get());
    }
    RSBrush brush;
    brush.SetColor(ToRSColor(color));
    brush.SetAntiAlias(true);
    canvas.AttachBrush(brush);
    canvas.DrawPath(*path);
}

void RichEditorDragOverlayModifier::PaintSelBackground(RSCanvas& canvas, RefPtr<TextDragPattern> textDragPattern,
    RefPtr<RichEditorPattern> richEditorPattern)
{
    CHECK_NULL_VOID(richEditorPattern);
    if (type_ == DragAnimType::DEFAULT || NearZero(selectedBackgroundOpacity_->Get())) {
        return;
    }
    std::shared_ptr<RSPath> path;
    if (isAnimating_) {
        path = textDragPattern->GenerateSelBackgroundPath(backgroundOffset_->Get());
    } else {
        path = textDragPattern->GetSelBackgroundPath();
    }
    RSBrush selBrush;
    Color selColor = Color::WHITE;
    selBrush.SetColor(ToRSColor(selColor));
    selBrush.SetAntiAlias(true);
    canvas.AttachBrush(selBrush);
    canvas.DrawPath(*path);
    canvas.DetachBrush();

    selColor = Color(selectedColor_->Get());
    selColor = selColor.BlendOpacity(selectedBackgroundOpacity_->Get());
    selBrush.SetColor(ToRSColor(selColor));
    selBrush.SetAntiAlias(true);
    canvas.AttachBrush(selBrush);
    canvas.DrawPath(*path);
    canvas.DetachBrush();
}

void RichEditorDragOverlayModifier::PaintHandle(RSCanvas& canvas, const RectF& handleRect, bool isFirstHandle,
    float startX, float startY)
{
    if (!isAnimating_ || type_ == DragAnimType::DEFAULT) {
        return;
    }
    if (NearZero(handleOpacity_->Get()) || NearZero(handleRect.Width())) {
        return;
    }
    auto rectTopX = (handleRect.Right() - handleRect.Left()) / 2.0f + startX;
    auto centerOffset = OffsetF(rectTopX, 0.0);
    OffsetF startPoint(0.0, 0.0);
    OffsetF endPoint(0.0, 0.0);
    float offset = type_ == DragAnimType::FLOATING ? backgroundOffset_->Get() : 0.0;
    auto outerHandleRadius = handleRadius_->Get();
    auto handleRectHeight = handleRect.Height();
    if (isFirstHandle) {
        centerOffset.SetX(centerOffset.GetX() - offset);
        centerOffset.SetY(startY - outerHandleRadius);
        startPoint.SetY(outerHandleRadius - 1.0);
        endPoint.SetY(outerHandleRadius + handleRectHeight);
    } else {
        centerOffset.SetX(centerOffset.GetX() + offset);
        centerOffset.SetY(startY + outerHandleRadius);
        startPoint.SetY(-outerHandleRadius + 1.0);
        endPoint.SetY(-outerHandleRadius - handleRectHeight);
    }
    canvas.Save();
    canvas.Translate(centerOffset.GetX(), centerOffset.GetY());
    PaintHandleRing(canvas);
    PaintHandleHold(canvas, handleRect, startPoint, endPoint);
    canvas.Restore();
}

void RichEditorDragOverlayModifier::PaintHandleRing(RSCanvas& canvas)
{
    RSPen pen;
    pen.SetAntiAlias(true);
    Color handleColor = handleColor_->Get();
    auto handleOpacity = handleOpacity_->Get();
    handleColor = handleColor.BlendOpacity(handleOpacity);
    pen.SetColor(handleColor.GetValue());
    auto outerHandleRadius = handleRadius_->Get();
    auto innerHandleRadius = innerHandleRadius_->Get();
    pen.SetWidth(outerHandleRadius - innerHandleRadius);
    pen.SetCapStyle(RSPen::CapStyle::ROUND_CAP);
    canvas.AttachPen(pen);
    auto radius = (innerHandleRadius + outerHandleRadius) / 2;
    RSPath ringPath;
    ringPath.AddArc({ -radius, -radius, radius, radius }, 0, HANDLE_RING_DEGREE);
    canvas.DrawPath(ringPath);
    canvas.DetachPen();
    Color innerHandleColor = innerHandleColor_->Get();
    innerHandleColor = innerHandleColor.BlendOpacity(handleOpacity);
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetColor(innerHandleColor.GetValue());
    canvas.AttachBrush(brush);
    canvas.DrawCircle(RSPoint(0.0, 0.0), innerHandleRadius);
    canvas.DetachBrush();
}

void RichEditorDragOverlayModifier::PaintHandleHold(RSCanvas& canvas, const RectF& handleRect,
    const OffsetF& startPoint, const OffsetF& endPoint)
{
    Color handleColor = handleColor_->Get();
    auto handleOpacity = handleOpacity_->Get();
    handleColor = handleColor.BlendOpacity(handleOpacity);
    RSPen linePen;
    linePen.SetAntiAlias(true);
    linePen.SetColor(handleColor.GetValue());
    linePen.SetWidth(handleRect.Width());
    linePen.SetCapStyle(RSPen::CapStyle::ROUND_CAP);
    canvas.AttachPen(linePen);
    canvas.DrawLine(RSPoint(startPoint.GetX(), startPoint.GetY()), RSPoint(endPoint.GetX(), endPoint.GetY()));
    canvas.DetachPen();
}

void RichEditorDragOverlayModifier::StartFloatingAnimate()
{
    auto pattern = DynamicCast<RichEditorPattern>(hostPattern_.Upgrade());
    if (!pattern || !IsHandlesShow()) {
        type_ = DragAnimType::DEFAULT;
        SetSelectedBackgroundOpacity(0.0);
        SetHandleOpacity(0.0);
        TextDragOverlayModifier::StartFloatingAnimate();
        return;
    }
    type_ = DragAnimType::FLOATING;
    isAnimating_ = true;

    SetHandleOpacity(1.0);
    AnimationOption handleOption;
    handleOption.SetDuration(FLOATING_ANIMATE_HANDLE_OPACITY_DURATION);
    handleOption.SetCurve(Curves::LINEAR);
    handleOption.SetDelay(0);
    handleOption.SetFillMode(FillMode::FORWARDS);
    auto handlePropertyCallback = [weakModifier = WeakClaim(this)]() {
        auto modifier = weakModifier.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->SetHandleOpacity(0.0);
    };
    AnimationUtils::Animate(handleOption, handlePropertyCallback, nullptr);
    StartFloatingSelBackgroundAnimate();
}

void RichEditorDragOverlayModifier::StartFloatingSelBackgroundAnimate()
{
    SetBackgroundOffset(0);
    SetSelectedBackgroundOpacity(1.0);
    AnimationOption backgroundOption;
    backgroundOption.SetDuration(FLOATING_ANIMATE_BACKGROUND_CHANGE_DURATION);
    backgroundOption.SetCurve(Curves::FRICTION);
    backgroundOption.SetDelay(0);
    auto backgroundAnimFinishFuc = [weakModifier = WeakClaim(this)]() {
        auto modifier = weakModifier.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->SetAnimateFlag(false);
    };
    backgroundOption.SetOnFinishEvent(backgroundAnimFinishFuc);
    backgroundOption.SetFillMode(FillMode::FORWARDS);
    auto backgroundPropertyCallback = [weakModifier = WeakClaim(this)]() {
        auto modifier = weakModifier.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->SetBackgroundOffset(TEXT_DRAG_OFFSET.ConvertToPx());
        modifier->SetSelectedBackgroundOpacity(0.0);
    };
    AnimationUtils::Animate(backgroundOption, backgroundPropertyCallback, backgroundOption.GetOnFinishEvent());
}

void RichEditorDragOverlayModifier::StartFloatingCancelAnimate()
{
    auto pattern = DynamicCast<RichEditorPattern>(hostPattern_.Upgrade());
    if (!pattern || !IsHandlesShow()) {
        type_ = DragAnimType::DEFAULT;
        TextDragOverlayModifier::StartFloatingCancelAnimate();
        return;
    }
    type_ = DragAnimType::FLOATING_CANCEL;
    isAnimating_ = true;

    SetBackgroundOffset(TEXT_DRAG_OFFSET.ConvertToPx());
    AnimationOption backgroundOption;
    backgroundOption.SetDuration(FLOATING_CANCEL_ANIMATE_TEXT_RECOVERY_DURATION);
    backgroundOption.SetCurve(Curves::SHARP);
    backgroundOption.SetDelay(0);
    backgroundOption.SetFillMode(FillMode::FORWARDS);
    auto backgroundPropertyCallback = [weakModifier = WeakClaim(this)]() {
        auto modifier = weakModifier.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->SetBackgroundOffset(0.0);
    };
    AnimationUtils::Animate(backgroundOption, backgroundPropertyCallback, nullptr);
    StartSelBackgroundCancelAnimate();
}

void RichEditorDragOverlayModifier::StartSelBackgroundCancelAnimate()
{
    auto pattern = DynamicCast<RichEditorPattern>(hostPattern_.Upgrade());
    SetHandleOpacity(0.0);
    SetSelectedBackgroundOpacity(0.0);
    AnimationOption selOption;
    selOption.SetDuration(FLOATING_CANCEL_ANIMATE_TEXT_RECOVERY_DURATION);
    selOption.SetCurve(Curves::LINEAR);
    selOption.SetDelay(FLOATING_CANCEL_ANIMATE_TEXT_RECOVERY_DELAY_DURATION);
    selOption.SetFillMode(FillMode::FORWARDS);
    auto selectAnimFinishFuc = [weakModifier = WeakClaim(this),
        weakPattern = WeakPtr<RichEditorPattern>(pattern)]() {
        auto modifier = weakModifier.Upgrade();
        CHECK_NULL_VOID(modifier);
        auto pattern = weakPattern.Upgrade();
        CHECK_NULL_VOID(pattern);
        modifier->SetAnimateFlag(false);
        pattern->ShowHandles();
    };
    selOption.SetOnFinishEvent(selectAnimFinishFuc);
    auto selPropertyCallback = [weakModifier = WeakClaim(this)]() {
        auto modifier = weakModifier.Upgrade();
        CHECK_NULL_VOID(modifier);
        modifier->SetHandleOpacity(1.0);
        modifier->SetSelectedBackgroundOpacity(1.0);
    };
    AnimationUtils::Animate(selOption, selPropertyCallback, selOption.GetOnFinishEvent());
}
} // namespace OHOS::Ace::NG
