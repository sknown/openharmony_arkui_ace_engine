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

#include "core/components_ng/pattern/grid/grid_paint_method.h"

#include "core/components_ng/pattern/scroll/inner/scroll_bar_overlay_modifier.h"
#include "core/components_ng/pattern/scroll/inner/scroll_bar_painter.h"

namespace OHOS::Ace::NG {
void GridPaintMethod::PaintEdgeEffect(PaintWrapper* paintWrapper, RSCanvas& canvas)
{
    auto edgeEffect = edgeEffect_.Upgrade();
    CHECK_NULL_VOID_NOLOG(edgeEffect);
    CHECK_NULL_VOID(paintWrapper);
    auto frameSize = paintWrapper->GetGeometryNode()->GetFrameSize();
    edgeEffect->Paint(canvas, frameSize, { 0.0f, 0.0f });
}

CanvasDrawFunction GridPaintMethod::GetForegroundDrawFunction(PaintWrapper* paintWrapper)
{
    return [weak = WeakClaim(this), paintWrapper, weakScrollBar = scrollBar_](RSCanvas& canvas) {
        auto painter = weak.Upgrade();
        CHECK_NULL_VOID(painter);
        painter->PaintEdgeEffect(paintWrapper, canvas);
    };
}

void GridPaintMethod::UpdateOverlayModifier(PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID_NOLOG(paintWrapper);
    auto scrollBarOverlayModifier = scrollBarOverlayModifier_.Upgrade();
    CHECK_NULL_VOID_NOLOG(scrollBarOverlayModifier);
    auto scrollBar = scrollBar_.Upgrade();
    if (!scrollBar || !scrollBar->NeedPaint()) {
        LOGD("no need paint scroll bar.");
        return;
    }
    OffsetF fgOffset(scrollBar->GetActiveRect().Left(), scrollBar->GetActiveRect().Top());
    OffsetF bgOffset(scrollBar->GetBarRect().Left(), scrollBar->GetBarRect().Top());
    scrollBarOverlayModifier->SetRect(SizeF(scrollBar->GetActiveRect().Width(), scrollBar->GetActiveRect().Height()),
        SizeF(scrollBar->GetBarRect().Width(), scrollBar->GetBarRect().Height()), fgOffset, bgOffset,
        scrollBar->GetHoverAnimationType());
    scrollBarOverlayModifier->SetOffset(fgOffset, bgOffset);
    scrollBar->SetHoverAnimationType(HoverAnimationType::NONE);
    scrollBarOverlayModifier->SetFgColor(scrollBar->GetForegroundColor());
    scrollBarOverlayModifier->SetBgColor(scrollBar->GetBackgroundColor());
    scrollBarOverlayModifier->StartOpacityAnimation(scrollBar->GetOpacityAnimationType());
    scrollBar->SetOpacityAnimationType(OpacityAnimationType::NONE);
}
} // namespace OHOS::Ace::NG
