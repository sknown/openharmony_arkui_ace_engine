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

#include "core/components_ng/pattern/text_drag/text_drag_overlay_modifier.h"

#include <variant>

#include "base/geometry/rect.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/text_drag/text_drag_pattern.h"
#include "core/components_ng/render/adapter/pixelmap_image.h"
#include "core/components_ng/render/drawing_prop_convertor.h"

namespace OHOS::Ace::NG {
TextDragOverlayModifier::TextDragOverlayModifier(const WeakPtr<OHOS::Ace::NG::Pattern>& pattern) : pattern_(pattern)
{
    backgroundOffset_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(TEXT_DRAG_OFFSET.ConvertToPx());
    selectedBackgroundOpacity_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0);
    AttachProperty(backgroundOffset_);
    AttachProperty(selectedBackgroundOpacity_);
}

void TextDragOverlayModifier::onDraw(DrawingContext& context)
{
    auto pattern = DynamicCast<TextDragPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(pattern);
    auto& canvas = context.canvas;
    Color color(TEXT_DRAG_COLOR_BG);
    RSBrush brush;
    brush.SetColor(ToRSColor(color));
    brush.SetAntiAlias(true);
    canvas.AttachBrush(brush);
    if (!isAnimating_) {
        canvas.DrawPath(*pattern->GetBackgroundPath());
    } else {
        canvas.DrawPath(*pattern->GenerateBackgroundPath(backgroundOffset_->Get()));
    }
    canvas.DetachBrush();
    canvas.ClipPath(*pattern->GetClipPath(), RSClipOp::INTERSECT, true);
    auto paragraph = pattern->GetParagraph().Upgrade();
    if (paragraph) {
        paragraph->Paint(canvas, pattern->GetTextRect().GetX(), pattern->GetTextRect().GetY());
    }

    size_t index = 0;
    auto contentOffset = pattern->GetContentOffset();
    auto imageChildren = pattern->GetImageChildren();
    auto rectsForPlaceholders = pattern->GetRectsForPlaceholders();
    for (const auto& child : imageChildren) {
        auto rect = rectsForPlaceholders.at(index);
        auto offset = OffsetF(rect.Left(), rect.Top()) - contentOffset;
        auto imageChild = DynamicCast<ImagePattern>(child->GetPattern());
        if (imageChild) {
            RectF imageRect(offset.GetX(), offset.GetY(), rect.Width(), rect.Height());
            auto canvasImage = imageChild->GetCanvasImage();
            CHECK_NULL_VOID(canvasImage);
            auto pixelMapImage = DynamicCast<PixelMapImage>(canvasImage);
            CHECK_NULL_VOID(pixelMapImage);
            pixelMapImage->DrawRect(canvas, ToRSRect(imageRect));
        }
        ++index;
    }
}

void TextDragOverlayModifier::SetBackgroundOffset(float offset)
{
    backgroundOffset_->Set(offset);
}

void TextDragOverlayModifier::SetSelectedBackgroundOpacity(float offset)
{
    selectedBackgroundOpacity_->Set(offset);
}
} // namespace OHOS::Ace::NG
