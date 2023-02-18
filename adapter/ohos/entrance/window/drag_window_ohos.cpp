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

#include "drag_window_ohos.h"

#include "flutter/fml/memory/ref_counted.h"
#include "flutter/lib/ui/painting/image.h"
#include "flutter/third_party/txt/src/txt/paragraph_txt.h"
#include "fml/memory/ref_ptr.h"

#include "base/geometry/ng/rect_t.h"
#include "base/geometry/offset.h"
#include "base/image/pixel_map.h"
#include "base/log/log_wrapper.h"
#include "base/utils/utils.h"
#include "core/components/text/render_text.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/render/adapter/rosen_render_context.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace {
#ifdef ENABLE_ROSEN_BACKEND
namespace {
// Adapt text dragging background shadows to expand the width of dargwindow
const Dimension Window_EXTERN = 10.0_vp;
sk_sp<SkColorSpace> ColorSpaceToSkColorSpace(const RefPtr<PixelMap>& pixmap)
{
    return SkColorSpace::MakeSRGB(); // Media::PixelMap has not support wide gamut yet.
}

SkAlphaType AlphaTypeToSkAlphaType(const RefPtr<PixelMap>& pixmap)
{
    switch (pixmap->GetAlphaType()) {
        case AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN:
            return SkAlphaType::kUnknown_SkAlphaType;
        case AlphaType::IMAGE_ALPHA_TYPE_OPAQUE:
            return SkAlphaType::kOpaque_SkAlphaType;
        case AlphaType::IMAGE_ALPHA_TYPE_PREMUL:
            return SkAlphaType::kPremul_SkAlphaType;
        case AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL:
            return SkAlphaType::kUnpremul_SkAlphaType;
        default:
            return SkAlphaType::kUnknown_SkAlphaType;
    }
}

SkColorType PixelFormatToSkColorType(const RefPtr<PixelMap>& pixmap)
{
    switch (pixmap->GetPixelFormat()) {
        case PixelFormat::RGB_565:
            return SkColorType::kRGB_565_SkColorType;
        case PixelFormat::RGBA_8888:
            return SkColorType::kRGBA_8888_SkColorType;
        case PixelFormat::BGRA_8888:
            return SkColorType::kBGRA_8888_SkColorType;
        case PixelFormat::ALPHA_8:
            return SkColorType::kAlpha_8_SkColorType;
        case PixelFormat::RGBA_F16:
            return SkColorType::kRGBA_F16_SkColorType;
        case PixelFormat::UNKNOWN:
        case PixelFormat::ARGB_8888:
        case PixelFormat::RGB_888:
        case PixelFormat::NV21:
        case PixelFormat::NV12:
        case PixelFormat::CMYK:
        default:
            return SkColorType::kUnknown_SkColorType;
    }
}

SkImageInfo MakeSkImageInfoFromPixelMap(const RefPtr<PixelMap>& pixmap)
{
    SkColorType colorType = PixelFormatToSkColorType(pixmap);
    SkAlphaType alphaType = AlphaTypeToSkAlphaType(pixmap);
    sk_sp<SkColorSpace> colorSpace = ColorSpaceToSkColorSpace(pixmap);
    return SkImageInfo::Make(pixmap->GetWidth(), pixmap->GetHeight(), colorType, alphaType, colorSpace);
}

void DrawSkImage(SkCanvas* canvas, const RefPtr<PixelMap>& pixmap, int32_t width, int32_t height)
{
    // Step1: Create SkPixmap
    auto imageInfo = MakeSkImageInfoFromPixelMap(pixmap);
    SkPixmap imagePixmap(imageInfo, reinterpret_cast<const void*>(pixmap->GetPixels()), pixmap->GetRowBytes());

    // Step2: Create SkImage and draw it
    sk_sp<SkImage> skImage =
        SkImage::MakeFromRaster(imagePixmap, &PixelMap::ReleaseProc, PixelMap::GetReleaseContext(pixmap));
    CHECK_NULL_VOID(skImage);
    SkPaint paint;
    sk_sp<SkColorSpace> colorSpace = skImage->refColorSpace();
#ifdef USE_SYSTEM_SKIA
    paint.setColor4f(paint.getColor4f(), colorSpace.get());
#else
    paint.setColor(paint.getColor4f(), colorSpace.get());
#endif
    auto skSrcRect = SkRect::MakeXYWH(0, 0, pixmap->GetWidth(), pixmap->GetHeight());
    auto skDstRect = SkRect::MakeXYWH(0, 0, width, height);
    canvas->drawImageRect(skImage, skSrcRect, skDstRect, &paint);
}

void DrawSkImage(SkCanvas* canvas, const sk_sp<SkImage>& skImage, int32_t width, int32_t height)
{
    CHECK_NULL_VOID(skImage);
    SkPaint paint;
    sk_sp<SkColorSpace> colorSpace = skImage->refColorSpace();
#ifdef USE_SYSTEM_SKIA
    paint.setColor4f(paint.getColor4f(), colorSpace.get());
#else
    paint.setColor(paint.getColor4f(), colorSpace.get());
#endif
    auto skSrcRect = SkRect::MakeXYWH(0, 0, skImage->width(), skImage->height());
    auto skDstRect = SkRect::MakeXYWH(0, 0, width, height);
    canvas->drawImageRect(skImage, skSrcRect, skDstRect, &paint);
}
} // namespace
#endif

RefPtr<DragWindow> DragWindow::CreateDragWindow(
    const std::string& windowName, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    int32_t halfWidth = static_cast<int32_t>(width) / 2;
    int32_t halfHeight = static_cast<int32_t>(height) / 2;

    OHOS::sptr<OHOS::Rosen::WindowOption> option = new OHOS::Rosen::WindowOption();
    option->SetWindowRect({ x - halfWidth, y - halfHeight, width, height });
    option->SetHitOffset(halfWidth, halfHeight);
    option->SetWindowType(OHOS::Rosen::WindowType::WINDOW_TYPE_DRAGGING_EFFECT);
    option->SetWindowMode(OHOS::Rosen::WindowMode::WINDOW_MODE_FLOATING);
    option->SetFocusable(false);
    OHOS::sptr<OHOS::Rosen::Window> dragWindow = OHOS::Rosen::Window::Create(windowName, option);
    CHECK_NULL_RETURN(dragWindow, nullptr);

    OHOS::Rosen::WMError ret = dragWindow->Show();
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        LOGE("DragWindow::CreateDragWindow, drag window Show() failed, ret: %d", ret);
    }

    auto window = AceType::MakeRefPtr<DragWindowOhos>(dragWindow);
    window->SetSize(width, height);
    return window;
}

RefPtr<DragWindow> DragWindow::CreateTextDragWindow(
    const std::string& windowName, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    int32_t halfWidth = static_cast<int32_t>(width + Window_EXTERN.ConvertToPx() * 2) / 2;
    int32_t halfHeight = static_cast<int32_t>(height + Window_EXTERN.ConvertToPx() * 2) / 2;

    OHOS::sptr<OHOS::Rosen::WindowOption> option = new OHOS::Rosen::WindowOption();
    option->SetWindowRect({ x - Window_EXTERN.ConvertToPx(), y - Window_EXTERN.ConvertToPx(),
        width + Window_EXTERN.ConvertToPx() * 2, height + Window_EXTERN.ConvertToPx() * 2 });
    option->SetHitOffset(halfWidth, halfHeight);
    option->SetWindowType(OHOS::Rosen::WindowType::WINDOW_TYPE_DRAGGING_EFFECT);
    option->SetWindowMode(OHOS::Rosen::WindowMode::WINDOW_MODE_FLOATING);
    option->SetFocusable(false);
    OHOS::sptr<OHOS::Rosen::Window> dragWindow = OHOS::Rosen::Window::Create(windowName, option);
    CHECK_NULL_RETURN(dragWindow, nullptr);

    OHOS::Rosen::WMError ret = dragWindow->Show();
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        LOGE("DragWindow::CreateTextDragWindow, drag window Show() failed, ret: %d", ret);
    }

    auto window = AceType::MakeRefPtr<DragWindowOhos>(dragWindow);
    window->SetSize(width + Window_EXTERN.ConvertToPx() * 2, height + Window_EXTERN.ConvertToPx() * 2);
    return window;
}

void DragWindowOhos::MoveTo(int32_t x, int32_t y) const
{
    CHECK_NULL_VOID(dragWindow_);

    OHOS::Rosen::WMError ret = dragWindow_->MoveTo(x + offsetX_ - width_ / 2, y + offsetY_ - height_ / 2);
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        LOGE("DragWindow::MoveTo, drag window move failed, ret: %d", ret);
        return;
    }

    ret = dragWindow_->Show();
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        LOGE("DragWindow::CreateDragWindow, drag window Show() failed, ret: %d", ret);
    }
}

void DragWindowOhos::TextDragWindowMove(double x, double y) const
{
    CHECK_NULL_VOID(dragWindow_);
    OHOS::Rosen::WMError ret =
        dragWindow_->MoveTo(x - Window_EXTERN.ConvertToPx() + offsetX_, y + offsetY_ - Window_EXTERN.ConvertToPx());
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        LOGE("DragWindow::TextDragWindowMove, drag window move failed, ret: %d", ret);
        return;
    }

    ret = dragWindow_->Show();
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        LOGE("DragWindow::TextDragWindowMove, drag window Show() failed, ret: %d", ret);
    }
}

void DragWindowOhos::Destroy() const
{
    CHECK_NULL_VOID(dragWindow_);

    OHOS::Rosen::WMError ret = dragWindow_->Destroy();
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        LOGE("DragWindow::Destroy, drag window destroy failed, ret: %d", ret);
    }
}

void DragWindowOhos::DrawFrameNode(const RefPtr<NG::FrameNode>& rootNode)
{
#ifdef ENABLE_ROSEN_BACKEND
    CHECK_NULL_VOID(rootNode);

    auto surfaceNode = dragWindow_->GetSurfaceNode();
    rsUiDirector_ = Rosen::RSUIDirector::Create();
    rsUiDirector_->Init();
    auto transactionProxy = Rosen::RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        transactionProxy->FlushImplicitTransaction();
    }
    rsUiDirector_->SetRSSurfaceNode(surfaceNode);

    auto renderContext = AceType::DynamicCast<NG::RosenRenderContext>(rootNode->GetRenderContext());
    CHECK_NULL_VOID(renderContext);
    auto rsNode = renderContext->GetRSNode();
    CHECK_NULL_VOID(rsNode);

    rsUiDirector_->SetRoot(rsNode->GetId());
    rsUiDirector_->SendMessages();
#endif
}

void DragWindowOhos::DrawPixelMap(const RefPtr<PixelMap>& pixelmap)
{
#ifdef ENABLE_ROSEN_BACKEND
    CHECK_NULL_VOID(pixelmap);
    auto surfaceNode = dragWindow_->GetSurfaceNode();
    rsUiDirector_ = Rosen::RSUIDirector::Create();
    rsUiDirector_->Init();
    auto transactionProxy = Rosen::RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        transactionProxy->FlushImplicitTransaction();
    }
    rsUiDirector_->SetRSSurfaceNode(surfaceNode);
    rootNode_ = Rosen::RSRootNode::Create();
    rootNode_->SetBounds(0, 0, static_cast<float>(width_), static_cast<float>(height_));
    rootNode_->SetFrame(0, 0, static_cast<float>(width_), static_cast<float>(height_));
    rsUiDirector_->SetRoot(rootNode_->GetId());
    auto canvasNode = std::static_pointer_cast<Rosen::RSCanvasNode>(rootNode_);
    auto skia = canvasNode->BeginRecording(width_, height_);
    DrawSkImage(skia, pixelmap, width_, height_);
    canvasNode->FinishRecording();
    rsUiDirector_->SendMessages();
#endif
}

void DragWindowOhos::DrawImage(void* skImage)
{
#ifdef ENABLE_ROSEN_BACKEND
    CHECK_NULL_VOID(skImage);
    fml::RefPtr<flutter::CanvasImage>* canvasImagePtr = reinterpret_cast<fml::RefPtr<flutter::CanvasImage>*>(skImage);
    CHECK_NULL_VOID(canvasImagePtr);
    fml::RefPtr<flutter::CanvasImage> canvasImage = *canvasImagePtr;
    CHECK_NULL_VOID(canvasImage);
    auto surfaceNode = dragWindow_->GetSurfaceNode();
    rsUiDirector_ = Rosen::RSUIDirector::Create();
    rsUiDirector_->Init();
    auto transactionProxy = Rosen::RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        transactionProxy->FlushImplicitTransaction();
    }
    rsUiDirector_->SetRSSurfaceNode(surfaceNode);
    rootNode_ = Rosen::RSRootNode::Create();
    rootNode_->SetBounds(0, 0, static_cast<float>(width_), static_cast<float>(height_));
    rootNode_->SetFrame(0, 0, static_cast<float>(width_), static_cast<float>(height_));
    rsUiDirector_->SetRoot(rootNode_->GetId());
    auto canvasNode = std::static_pointer_cast<Rosen::RSCanvasNode>(rootNode_);
    auto skia = canvasNode->BeginRecording(width_, height_);
    DrawSkImage(skia, canvasImage->image(), width_, height_);
    canvasNode->FinishRecording();
    rsUiDirector_->SendMessages();
#endif
}

void DragWindowOhos::DrawText(
    std::shared_ptr<txt::Paragraph> paragraph, const Offset& offset, const RefPtr<RenderText>& renderText)
{
#ifdef ENABLE_ROSEN_BACKEND
    CHECK_NULL_VOID(paragraph);
    auto surfaceNode = dragWindow_->GetSurfaceNode();
    rsUiDirector_ = Rosen::RSUIDirector::Create();
    rsUiDirector_->Init();
    auto transactionProxy = Rosen::RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        transactionProxy->FlushImplicitTransaction();
    }
    rsUiDirector_->SetRSSurfaceNode(surfaceNode);
    rootNode_ = Rosen::RSRootNode::Create();
    rootNode_->SetBounds(0, 0, static_cast<float>(width_), static_cast<float>(height_));
    rootNode_->SetFrame(0, 0, static_cast<float>(width_), static_cast<float>(height_));
    rsUiDirector_->SetRoot(rootNode_->GetId());
    auto canvasNode = std::static_pointer_cast<Rosen::RSCanvasNode>(rootNode_);
    SkPath path;
    if (renderText->GetStartOffset().GetY() == renderText->GetEndOffset().GetY()) {
        path.moveTo(renderText->GetStartOffset().GetX() - renderText->GetGlobalOffset().GetX(),
            renderText->GetStartOffset().GetY() - renderText->GetGlobalOffset().GetY());
        path.lineTo(renderText->GetEndOffset().GetX() - renderText->GetGlobalOffset().GetX(),
            renderText->GetEndOffset().GetY() - renderText->GetGlobalOffset().GetY());
        path.lineTo(renderText->GetEndOffset().GetX() - renderText->GetGlobalOffset().GetX(),
            renderText->GetEndOffset().GetY() - renderText->GetSelectHeight() - renderText->GetGlobalOffset().GetY());
        path.lineTo(renderText->GetStartOffset().GetX() - renderText->GetGlobalOffset().GetX(),
            renderText->GetStartOffset().GetY() - renderText->GetSelectHeight() - renderText->GetGlobalOffset().GetY());
        path.lineTo(renderText->GetStartOffset().GetX() - renderText->GetGlobalOffset().GetX(),
            renderText->GetStartOffset().GetY() - renderText->GetGlobalOffset().GetY());
    } else {
        path.moveTo(renderText->GetStartOffset().GetX() - renderText->GetGlobalOffset().GetX(),
            renderText->GetStartOffset().GetY() - renderText->GetGlobalOffset().GetY());
        path.lineTo(renderText->GetStartOffset().GetX() - renderText->GetGlobalOffset().GetX(),
            renderText->GetStartOffset().GetY() - renderText->GetSelectHeight() - renderText->GetGlobalOffset().GetY());
        path.lineTo(renderText->GetPaintRect().Width(),
            renderText->GetStartOffset().GetY() - renderText->GetSelectHeight() - renderText->GetGlobalOffset().GetY());
        path.lineTo(renderText->GetPaintRect().Width(),
            renderText->GetEndOffset().GetY() - renderText->GetSelectHeight() - renderText->GetGlobalOffset().GetY());
        path.lineTo(renderText->GetEndOffset().GetX() - renderText->GetGlobalOffset().GetX(),
            renderText->GetEndOffset().GetY() - renderText->GetSelectHeight() - renderText->GetGlobalOffset().GetY());
        path.lineTo(renderText->GetEndOffset().GetX() - renderText->GetGlobalOffset().GetX(),
            renderText->GetEndOffset().GetY() - renderText->GetGlobalOffset().GetY());
        path.lineTo(renderText->GetPaintRect().Left() - renderText->GetGlobalOffset().GetX(),
            renderText->GetEndOffset().GetY() - renderText->GetGlobalOffset().GetY());
        path.lineTo(renderText->GetPaintRect().Left() - renderText->GetGlobalOffset().GetX(),
            renderText->GetStartOffset().GetY() - renderText->GetGlobalOffset().GetY());
        path.lineTo(renderText->GetStartOffset().GetX() - renderText->GetGlobalOffset().GetX(),
            renderText->GetStartOffset().GetY() - renderText->GetGlobalOffset().GetY());
    }
    rootNode_->SetClipToBounds(true);
    rootNode_->SetClipBounds(Rosen::RSPath::CreateRSPath(path));
    auto skia = canvasNode->BeginRecording(width_, height_);
    paragraph->Paint(skia, 0, 0);
    canvasNode->FinishRecording();
    rsUiDirector_->SendMessages();
#endif
}

void DragWindowOhos::DrawTextNG(const RefPtr<NG::Paragraph>& paragraph, const RefPtr<NG::TextPattern>& textPattern)
{
#ifdef ENABLE_ROSEN_BACKEND
    CHECK_NULL_VOID(paragraph);
    auto surfaceNode = dragWindow_->GetSurfaceNode();
    rsUiDirector_ = Rosen::RSUIDirector::Create();
    CHECK_NULL_VOID(rsUiDirector_);
    rsUiDirector_->Init();
    auto transactionProxy = Rosen::RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        transactionProxy->FlushImplicitTransaction();
    }
    rsUiDirector_->SetRSSurfaceNode(surfaceNode);

    rootNode_ = Rosen::RSRootNode::Create();
    CHECK_NULL_VOID(rootNode_);
    rootNode_->SetBounds(Window_EXTERN.ConvertToPx(), Window_EXTERN.ConvertToPx(), static_cast<float>(width_),
        static_cast<float>(height_));
    rootNode_->SetFrame(Window_EXTERN.ConvertToPx(), Window_EXTERN.ConvertToPx(), static_cast<float>(width_),
        static_cast<float>(height_));
    rsUiDirector_->SetRoot(rootNode_->GetId());
    auto canvasNode = std::static_pointer_cast<Rosen::RSCanvasNode>(rootNode_);
    CHECK_NULL_VOID(canvasNode);
    Offset globalOffset;
    textPattern->GetGlobalOffset(globalOffset);
    SkPath path;
    if (textPattern->GetStartOffset().GetY() == textPattern->GetEndOffset().GetY()) {
        path.moveTo(textPattern->GetStartOffset().GetX() - globalOffset.GetX(),
            textPattern->GetStartOffset().GetY() - globalOffset.GetY());
        path.lineTo(textPattern->GetEndOffset().GetX() - globalOffset.GetX(),
            textPattern->GetEndOffset().GetY() - globalOffset.GetY());
        path.lineTo(textPattern->GetEndOffset().GetX() - globalOffset.GetX(),
            textPattern->GetEndOffset().GetY() + textPattern->GetSelectHeight() - globalOffset.GetY());
        path.lineTo(textPattern->GetStartOffset().GetX() - globalOffset.GetX(),
            textPattern->GetStartOffset().GetY() + textPattern->GetSelectHeight() - globalOffset.GetY());
        path.lineTo(textPattern->GetStartOffset().GetX() - globalOffset.GetX(),
            textPattern->GetStartOffset().GetY() - globalOffset.GetY());
    } else {
        path.moveTo(textPattern->GetStartOffset().GetX() - globalOffset.GetX(),
            textPattern->GetStartOffset().GetY() - globalOffset.GetY());
        path.lineTo(
            textPattern->GetTextContentRect().Width(), textPattern->GetStartOffset().GetY() - globalOffset.GetY());
        path.lineTo(
            textPattern->GetTextContentRect().Width(), textPattern->GetEndOffset().GetY() - globalOffset.GetY());
        path.lineTo(textPattern->GetEndOffset().GetX() - globalOffset.GetX(),
            textPattern->GetEndOffset().GetY() - globalOffset.GetY());
        path.lineTo(textPattern->GetEndOffset().GetX() - globalOffset.GetX(),
            textPattern->GetEndOffset().GetY() + textPattern->GetSelectHeight() - globalOffset.GetY());
        path.lineTo(textPattern->GetTextContentRect().GetX(),
            textPattern->GetEndOffset().GetY() + textPattern->GetSelectHeight() - globalOffset.GetY());
        path.lineTo(textPattern->GetTextContentRect().GetX(),
            textPattern->GetStartOffset().GetY() + textPattern->GetSelectHeight() - globalOffset.GetY());
        path.lineTo(textPattern->GetStartOffset().GetX() - globalOffset.GetX(),
            textPattern->GetStartOffset().GetY() + textPattern->GetSelectHeight() - globalOffset.GetY());
        path.lineTo(textPattern->GetStartOffset().GetX() - globalOffset.GetX(),
            textPattern->GetStartOffset().GetY() - globalOffset.GetY());
    }
    rootNode_->SetClipToBounds(true);
    rootNode_->SetClipBounds(Rosen::RSPath::CreateRSPath(path));

    auto skia = canvasNode->BeginRecording(width_, height_);
    paragraph->Paint(skia, textPattern->GetTextContentRect().GetX(),
        textPattern->GetTextContentRect().GetY() - std::min(textPattern->GetBaselineOffset(), 0.0f));
    canvasNode->FinishRecording();
    rsUiDirector_->SendMessages();

    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    context->RequestFrame();
#endif
}
} // namespace OHOS::Ace