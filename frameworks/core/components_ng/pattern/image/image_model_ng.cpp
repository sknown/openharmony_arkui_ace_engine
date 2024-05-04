/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_IMAGE_IMAGE_MODEL_NG_CPP
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_IMAGE_IMAGE_MODEL_NG_CPP

#include "core/components_ng/pattern/image/image_model_ng.h"

#include "core/components/common/layout/constants.h"
#include "core/components/image/image_theme.h"
#include "core/components_ng/base/frame_node.h"
#ifndef ACE_UNITTEST
#include "core/components_ng/base/view_abstract.h"
#endif
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
const std::vector<float> DEFAULT_COLOR_FILTER = { 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0 };
ImageSourceInfo CreateSourceInfo(const std::string &src, RefPtr<PixelMap> &pixmap, const std::string &bundleName,
    const std::string &moduleName)
{
#if defined(PIXEL_MAP_SUPPORTED)
    if (pixmap) {
        return ImageSourceInfo(pixmap);
    }
#endif
    return { src, bundleName, moduleName };
}
} // namespace

void ImageModelNG::Create(const std::string &src, RefPtr<PixelMap> &pixMap, const std::string &bundleName,
    const std::string &moduleName, bool isUriPureNumber)
{
    auto *stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d] [src:%s]", V2::IMAGE_ETS_TAG, nodeId, src.c_str());
    auto frameNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG, nodeId,
        []() { return AceType::MakeRefPtr<ImagePattern>(); });
    stack->Push(frameNode);

    // set draggable for framenode
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto draggable = pipeline->GetDraggable<ImageTheme>();
    if (draggable && !frameNode->IsDraggable()) {
        auto gestureHub = frameNode->GetOrCreateGestureEventHub();
        CHECK_NULL_VOID(gestureHub);
        gestureHub->InitDragDropEvent();
    }
    frameNode->SetDraggable(draggable);
    auto srcInfo = CreateSourceInfo(src, pixMap, bundleName, moduleName);
    srcInfo.SetIsUriPureNumber(isUriPureNumber);

    auto pattern = GetImagePattern();
    CHECK_NULL_VOID(pattern);
    if (pattern->GetIsAnimation()) {
        if (pattern->GetHasSizeChanged()) {
            pattern->ResetPictureSize();
        }
        pattern->StopAnimation();
        pattern->ResetImages();
        if (!frameNode->GetChildren().empty()) {
            auto imageFrameNode = AceType::DynamicCast<FrameNode>(frameNode->GetChildren().front());
            ACE_RESET_NODE_LAYOUT_PROPERTY(ImageLayoutProperty, ImageSourceInfo, imageFrameNode);
            frameNode->RemoveChild(imageFrameNode);
        }
    }
    pattern->SetIsAnimation(false);

    ACE_UPDATE_LAYOUT_PROPERTY(ImageLayoutProperty, ImageSourceInfo, srcInfo);
}

void ImageModelNG::SetInitialSrc(FrameNode *frameNode, const std::string &src, const std::string &bundleName,
    const std::string &moduleName, bool isUriPureNumber)
{
    auto srcInfo = ImageSourceInfo{ src, bundleName, moduleName };
    srcInfo.SetIsUriPureNumber(isUriPureNumber);

    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ImageLayoutProperty, ImageSourceInfo, srcInfo, frameNode);
}

void ImageModelNG::SetInitialPixelMap(FrameNode* frameNode, RefPtr<PixelMap>& pixMap)
{
    auto srcInfo = CreateSourceInfo("", pixMap, "", "");
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ImageLayoutProperty, ImageSourceInfo, srcInfo, frameNode);
}

void ImageModelNG::CreateAnimation(const std::vector<ImageProperties>& imageList, int32_t duration, int32_t iteration)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::IMAGE_ETS_TAG, nodeId);
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::IMAGE_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<ImagePattern>(); });
    CHECK_NULL_VOID(frameNode);
    if (frameNode->GetChildren().empty()) {
        auto imageNode = FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, -1, AceType::MakeRefPtr<ImagePattern>());
        CHECK_NULL_VOID(imageNode);
        auto imageLayoutProperty = AceType::DynamicCast<ImageLayoutProperty>(imageNode->GetLayoutProperty());
        CHECK_NULL_VOID(imageLayoutProperty);
        imageLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);
        frameNode->GetLayoutProperty()->UpdateAlignment(Alignment::TOP_LEFT);
        frameNode->AddChild(imageNode);
    }
    stack->Push(frameNode);

    auto pattern = GetImagePattern();
    CHECK_NULL_VOID(pattern);
    if (!pattern->GetIsAnimation()) {
        auto castImageLayoutProperty = frameNode->GetLayoutPropertyPtr<ImageLayoutProperty>();
        CHECK_NULL_VOID(castImageLayoutProperty);
        castImageLayoutProperty->Reset();
        auto castImageRenderProperty = frameNode->GetPaintPropertyPtr<ImageRenderProperty>();
        CHECK_NULL_VOID(castImageRenderProperty);
        castImageRenderProperty->Reset();
        pattern->ResetImageAndAlt();
        pattern->ResetImageProperties();
    }
    // set draggable for framenode
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto draggable = pipeline->GetDraggable<ImageTheme>();
    if (draggable && !frameNode->IsDraggable()) {
        auto gestureHub = frameNode->GetOrCreateGestureEventHub();
        CHECK_NULL_VOID(gestureHub);
        gestureHub->InitDragDropEvent();
    }
    frameNode->SetDraggable(draggable);
    pattern->StopAnimation();
    pattern->SetIsAnimation(true);
    std::vector<ImageProperties> images = imageList;
    pattern->SetImages(std::move(images));
    pattern->SetDuration(duration);
    pattern->SetIteration(iteration);
    pattern->StartAnimation();
}

RefPtr<FrameNode> ImageModelNG::CreateFrameNode(int32_t nodeId, const std::string& src, RefPtr<PixelMap>& pixMap,
    const std::string& bundleName, const std::string& moduleName, bool isUriPureNumber)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, nodeId, AceType::MakeRefPtr<ImagePattern>());
    CHECK_NULL_RETURN(frameNode, nullptr);
    // set draggable for framenode
    auto pipeline = PipelineContext::GetCurrentContextSafely();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto draggable = pipeline->GetDraggable<ImageTheme>();
    if (draggable && !frameNode->IsDraggable()) {
        auto gestureHub = frameNode->GetOrCreateGestureEventHub();
        CHECK_NULL_RETURN(gestureHub, nullptr);
        gestureHub->InitDragDropEvent();
    }
    frameNode->SetDraggable(draggable);
    auto srcInfo = CreateSourceInfo(src, pixMap, bundleName, moduleName);
    srcInfo.SetIsUriPureNumber(isUriPureNumber);
    auto layoutProperty = frameNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, nullptr);
    layoutProperty->UpdateImageSourceInfo(srcInfo);
    return frameNode;
}

void ImageModelNG::SetAlt(const ImageSourceInfo &src)
{
    ACE_UPDATE_LAYOUT_PROPERTY(ImageLayoutProperty, Alt, src);
}

void ImageModelNG::SetSmoothEdge(float value)
{
    ACE_UPDATE_PAINT_PROPERTY(ImageRenderProperty, SmoothEdge, value);
}

void ImageModelNG::SetSmoothEdge(FrameNode *frameNode, float value)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(ImageRenderProperty, SmoothEdge, value, frameNode);
}

void ImageModelNG::SetDynamicRangeMode(DynamicRangeMode dynamicRangeMode)
{
    ACE_UPDATE_PAINT_PROPERTY(ImageRenderProperty, DynamicMode, dynamicRangeMode);
    ACE_UPDATE_RENDER_CONTEXT(DynamicRangeMode, dynamicRangeMode);
}

void ImageModelNG::SetDynamicRangeMode(FrameNode* frameNode, DynamicRangeMode dynamicRangeMode)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(ImageRenderProperty, DynamicMode, dynamicRangeMode, frameNode);
    ACE_UPDATE_NODE_RENDER_CONTEXT(DynamicRangeMode, dynamicRangeMode, frameNode);
}

void ImageModelNG::SetEnhancedImageQuality(AIImageQuality imageQuality)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<ImagePattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetImageQuality(imageQuality);
}

void ImageModelNG::SetEnhancedImageQuality(FrameNode* frameNode, AIImageQuality imageQuality)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<ImagePattern>(frameNode);
    CHECK_NULL_VOID(pattern);
    pattern->SetImageQuality(imageQuality);
}

void ImageModelNG::SetBorder(const Border &border) {}

void ImageModelNG::SetBackBorder()
{
    ACE_UPDATE_PAINT_PROPERTY(ImageRenderProperty, NeedBorderRadius, true);
}

void ImageModelNG::SetBackBorder(FrameNode *frameNode)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(ImageRenderProperty, NeedBorderRadius, true, frameNode);
}

void ImageModelNG::SetBlur(double blur) {}

void ImageModelNG::SetImageFit(ImageFit value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(ImageLayoutProperty, ImageFit, value);
    ACE_UPDATE_PAINT_PROPERTY(ImageRenderProperty, ImageFit, value);
}

void ImageModelNG::SetMatchTextDirection(bool value)
{
    ACE_UPDATE_PAINT_PROPERTY(ImageRenderProperty, MatchTextDirection, value);
}

void ImageModelNG::SetFitOriginSize(bool value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(ImageLayoutProperty, FitOriginalSize, value);
}

void ImageModelNG::SetOnComplete(std::function<void(const LoadImageSuccessEvent &info)> &&callback)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<ImageEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnComplete(std::move(callback));
}

void ImageModelNG::SetOnError(std::function<void(const LoadImageFailEvent &info)> &&callback)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<ImageEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnError(std::move(callback));
}

void ImageModelNG::SetSvgAnimatorFinishEvent(std::function<void()>&& callback)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<ImageEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnFinish(std::move(callback));
}

void ImageModelNG::SetImageSourceSize(const std::pair<Dimension, Dimension> &size)
{
    SizeF sourceSize =
        SizeF(static_cast<float>(size.first.ConvertToPx()), static_cast<float>(size.second.ConvertToPx()));
    ACE_UPDATE_LAYOUT_PROPERTY(ImageLayoutProperty, SourceSize, sourceSize);
}

void ImageModelNG::SetImageFill(const Color &color)
{
    ACE_UPDATE_PAINT_PROPERTY(ImageRenderProperty, SvgFillColor, color);
    ACE_UPDATE_RENDER_CONTEXT(ForegroundColor, color);
}

void ImageModelNG::SetImageInterpolation(ImageInterpolation interpolation)
{
    ACE_UPDATE_PAINT_PROPERTY(ImageRenderProperty, ImageInterpolation, interpolation);
}

void ImageModelNG::SetImageRepeat(ImageRepeat imageRepeat)
{
    ACE_UPDATE_PAINT_PROPERTY(ImageRenderProperty, ImageRepeat, imageRepeat);
}

void ImageModelNG::SetImageRenderMode(ImageRenderMode imageRenderMode)
{
    ACE_UPDATE_PAINT_PROPERTY(ImageRenderProperty, ImageRenderMode, imageRenderMode);
}

bool ImageModelNG::IsSrcSvgImage()
{
    return false;
}

void ImageModelNG::SetAutoResize(bool autoResize)
{
    ACE_UPDATE_LAYOUT_PROPERTY(ImageLayoutProperty, AutoResize, autoResize);
}

void ImageModelNG::SetSyncMode(bool syncMode)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<ImagePattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSyncLoad(syncMode);
}

void ImageModelNG::SetColorFilterMatrix(const std::vector<float> &matrix)
{
    ACE_UPDATE_PAINT_PROPERTY(ImageRenderProperty, ColorFilter, matrix);
    ACE_RESET_PAINT_PROPERTY(ImageRenderProperty, DrawingColorFilter);
}

void ImageModelNG::SetDrawingColorFilter(RefPtr<DrawingColorFilter> &colorFilter)
{
    ACE_UPDATE_PAINT_PROPERTY(ImageRenderProperty, DrawingColorFilter, colorFilter);
    ACE_RESET_PAINT_PROPERTY(ImageRenderProperty, ColorFilter);
}

void ImageModelNG::SetDraggable(bool draggable)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto gestureHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    if (draggable) {
        if (!frameNode->IsDraggable()) {
            gestureHub->InitDragDropEvent();
        }
    } else {
        gestureHub->RemoveDragEvent();
    }
    CHECK_NULL_VOID(frameNode);
    frameNode->SetCustomerDraggable(draggable);
}

void ImageModelNG::SetOnDragStart(OnDragStartFunc &&onDragStart)
{
#ifndef ACE_UNITTEST
    auto dragStart = [dragStartFunc = std::move(onDragStart)](const RefPtr<OHOS::Ace::DragEvent> &event,
        const std::string &extraParams) -> DragDropInfo {
        auto dragInfo = dragStartFunc(event, extraParams);
        DragDropInfo info;
        info.extraInfo = dragInfo.extraInfo;
        info.pixelMap = dragInfo.pixelMap;
        info.customNode = AceType::DynamicCast<UINode>(dragInfo.node);
        return info;
    };
    ViewAbstract::SetOnDragStart(std::move(dragStart));
#endif
}

void ImageModelNG::SetOnDragEnter(OnDragDropFunc &&onDragEnter) {}

void ImageModelNG::SetOnDragLeave(OnDragDropFunc &&onDragLeave) {}

void ImageModelNG::SetOnDragMove(OnDragDropFunc &&onDragMove) {}

void ImageModelNG::SetOnDrop(OnDragDropFunc &&onDrop) {}

void ImageModelNG::SetCopyOption(const CopyOptions &copyOption)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<ImagePattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetCopyOption(copyOption);
}

bool ImageModelNG::UpdateDragItemInfo(DragItemInfo &itemInfo)
{
    return false;
}

void ImageModelNG::InitImage(FrameNode *frameNode, std::string& src)
{
    std::string bundleName;
    std::string moduleName;
    RefPtr<OHOS::Ace::PixelMap> pixMapPtr;
    auto srcInfo = CreateSourceInfo(src, pixMapPtr, bundleName, moduleName);
    srcInfo.SetIsUriPureNumber(false);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ImageLayoutProperty, ImageSourceInfo, srcInfo, frameNode);
}

void ImageModelNG::SetDrawingColorFilter(FrameNode *frameNode, RefPtr<DrawingColorFilter> &colorFilter)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(ImageRenderProperty, DrawingColorFilter, colorFilter, frameNode);
    ACE_RESET_NODE_PAINT_PROPERTY(ImageRenderProperty, ColorFilter, frameNode);
}

void ImageModelNG::SetCopyOption(FrameNode *frameNode, CopyOptions copyOption)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<ImagePattern>(frameNode);
    CHECK_NULL_VOID(pattern);
    pattern->SetCopyOption(copyOption);
}

void ImageModelNG::SetAutoResize(FrameNode *frameNode, bool autoResize)
{
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ImageLayoutProperty, AutoResize, autoResize, frameNode);
}

void ImageModelNG::SetResizableSlice(const ImageResizableSlice& slice)
{
    ACE_UPDATE_PAINT_PROPERTY(ImageRenderProperty, ImageResizableSlice, slice);
}

void ImageModelNG::SetResizableSlice(FrameNode *frameNode, const ImageResizableSlice& slice)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(ImageRenderProperty, ImageResizableSlice, slice, frameNode);
}

void ImageModelNG::SetImageRepeat(FrameNode *frameNode, ImageRepeat imageRepeat)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(ImageRenderProperty, ImageRepeat, imageRepeat, frameNode);
}

void ImageModelNG::SetImageRenderMode(FrameNode *frameNode, ImageRenderMode imageRenderMode)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(ImageRenderProperty, ImageRenderMode, imageRenderMode, frameNode);
}

void ImageModelNG::SetImageFit(FrameNode *frameNode, ImageFit value)
{
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ImageLayoutProperty, ImageFit, value, frameNode);
    ACE_UPDATE_NODE_PAINT_PROPERTY(ImageRenderProperty, ImageFit, value, frameNode);
}

void ImageModelNG::SetFitOriginSize(FrameNode *frameNode, bool value)
{
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ImageLayoutProperty, FitOriginalSize, value, frameNode);
}

void ImageModelNG::SetSyncMode(FrameNode *frameNode, bool syncMode)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<ImagePattern>(frameNode);
    CHECK_NULL_VOID(pattern);
    pattern->SetSyncLoad(syncMode);
}

void ImageModelNG::SetImageSourceSize(FrameNode *frameNode, const std::pair<Dimension, Dimension> &size)
{
    SizeF sourceSize =
        SizeF(static_cast<float>(size.first.ConvertToPx()), static_cast<float>(size.second.ConvertToPx()));
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ImageLayoutProperty, SourceSize, sourceSize, frameNode);
}

void ImageModelNG::SetMatchTextDirection(FrameNode *frameNode, bool value)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(ImageRenderProperty, MatchTextDirection, value, frameNode);
}

void ImageModelNG::SetImageFill(FrameNode *frameNode, const Color &color)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(ImageRenderProperty, SvgFillColor, color, frameNode);
    ACE_UPDATE_NODE_RENDER_CONTEXT(ForegroundColor, color, frameNode);
}

void ImageModelNG::SetAlt(FrameNode *frameNode, const ImageSourceInfo &src)
{
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ImageLayoutProperty, Alt, src, frameNode);
}

void ImageModelNG::SetImageInterpolation(FrameNode *frameNode, ImageInterpolation interpolation)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(ImageRenderProperty, ImageInterpolation, interpolation, frameNode);
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<ImagePattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetImageInterpolation(interpolation);
}

void ImageModelNG::SetColorFilterMatrix(FrameNode *frameNode, const std::vector<float> &matrix)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(ImageRenderProperty, ColorFilter, matrix, frameNode);
    ACE_RESET_NODE_PAINT_PROPERTY(ImageRenderProperty, DrawingColorFilter, frameNode);
}

void ImageModelNG::SetDraggable(FrameNode *frameNode, bool draggable)
{
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    if (draggable) {
        if (!frameNode->IsDraggable()) {
            gestureHub->InitDragDropEvent();
        }
    } else {
        gestureHub->RemoveDragEvent();
    }
    frameNode->SetCustomerDraggable(draggable);
}

void ImageModelNG::EnableAnalyzer(bool isEnableAnalyzer)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<ImagePattern>();
    CHECK_NULL_VOID(pattern);
    pattern->EnableAnalyzer(isEnableAnalyzer);
}

void ImageModelNG::SetImageAnalyzerConfig(const ImageAnalyzerConfig& config)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<ImagePattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetImageAnalyzerConfig(config);
}

void ImageModelNG::SetImageAnalyzerConfig(void* config)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<ImagePattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetImageAnalyzerConfig(config);
}

bool ImageModelNG::IsSrcSvgImage(FrameNode* frameNode)
{
    return false;
}

void ImageModelNG::SetOnComplete(
    FrameNode* frameNode, std::function<void(const LoadImageSuccessEvent& info)>&& callback)
{
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<ImageEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnComplete(std::move(callback));
}

void ImageModelNG::SetOnError(FrameNode* frameNode, std::function<void(const LoadImageFailEvent& info)>&& callback)
{
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<ImageEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnError(std::move(callback));
}

void ImageModelNG::SetOnSvgPlayFinish(FrameNode* frameNode, std::function<void()>&& callback)
{
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<ImageEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnFinish(std::move(callback));
}

ImageSourceInfo ImageModelNG::GetSrc(FrameNode* frameNode)
{
    ImageSourceInfo defaultImageSourceInfo;
    CHECK_NULL_RETURN(frameNode, defaultImageSourceInfo);
    auto layoutProperty = frameNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, defaultImageSourceInfo);
    return layoutProperty->GetImageSourceInfo().value_or(defaultImageSourceInfo);
}

ImageFit ImageModelNG::GetObjectFit(FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, ImageFit::COVER);
    auto layoutProperty = frameNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, ImageFit::COVER);
    return layoutProperty->GetImageFit().value_or(ImageFit::COVER);
}

ImageInterpolation ImageModelNG::GetInterpolation(FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, ImageInterpolation::NONE);
    auto paintProperty = frameNode->GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_RETURN(paintProperty, ImageInterpolation::NONE);
    CHECK_NULL_RETURN(paintProperty->GetImagePaintStyle(), ImageInterpolation::NONE);
    return paintProperty->GetImagePaintStyle()->GetImageInterpolation().value_or(ImageInterpolation::NONE);
}

ImageRepeat ImageModelNG::GetObjectRepeat(FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, ImageRepeat::NO_REPEAT);
    auto paintProperty = frameNode->GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_RETURN(paintProperty, ImageRepeat::NO_REPEAT);
    CHECK_NULL_RETURN(paintProperty->GetImagePaintStyle(), ImageRepeat::NO_REPEAT);
    return paintProperty->GetImagePaintStyle()->GetImageRepeat().value_or(ImageRepeat::NO_REPEAT);
}

std::vector<float> ImageModelNG::GetColorFilter(FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, DEFAULT_COLOR_FILTER);
    auto paintProperty = frameNode->GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_RETURN(paintProperty, DEFAULT_COLOR_FILTER);
    CHECK_NULL_RETURN(paintProperty->GetImagePaintStyle(), DEFAULT_COLOR_FILTER);
    return paintProperty->GetImagePaintStyle()->GetColorFilter().value_or(DEFAULT_COLOR_FILTER);
}

bool ImageModelNG::GetAutoResize(FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, true);
    auto layoutProperty = frameNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, true);
    CHECK_NULL_RETURN(layoutProperty->GetImageSizeStyle(), true);
    return layoutProperty->GetImageSizeStyle()->GetAutoResize().value_or(true);
}

ImageSourceInfo ImageModelNG::GetAlt(FrameNode* frameNode)
{
    ImageSourceInfo defaultImageSourceInfo;
    CHECK_NULL_RETURN(frameNode, defaultImageSourceInfo);
    auto layoutProperty = frameNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, defaultImageSourceInfo);
    return layoutProperty->GetAlt().value_or(defaultImageSourceInfo);
}

bool ImageModelNG::GetFitOriginalSize(FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto layoutProperty = frameNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, false);
    return layoutProperty->GetImageSizeStyle()->GetFitOriginalSize().value_or(false);
}

uint32_t ImageModelNG::GetFillColor(FrameNode *frameNode)
{
    uint32_t errorCode = -1;
    CHECK_NULL_RETURN(frameNode, errorCode);
    auto paintProperty = frameNode->GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_RETURN(paintProperty, errorCode);
    CHECK_NULL_RETURN(paintProperty->GetImagePaintStyle(), errorCode);
    Color color = paintProperty->GetImagePaintStyle()->GetSvgFillColor().value_or(Color());
    return color.GetValue();
}

bool ImageModelNG::GetDraggable(FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, false);
    return frameNode->IsDraggable();
}

ImageRenderMode ImageModelNG::GetImageRenderMode(FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, ImageRenderMode::ORIGINAL);
    auto paintProperty = frameNode->GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_RETURN(paintProperty, ImageRenderMode::ORIGINAL);
    CHECK_NULL_RETURN(paintProperty->GetImagePaintStyle(), ImageRenderMode::ORIGINAL);
    return paintProperty->GetImagePaintStyle()->GetImageRenderMode().value_or(ImageRenderMode::ORIGINAL);
}

bool ImageModelNG::GetIsAnimation()
{
    return GetImagePattern()->GetIsAnimation();
}

RefPtr<ImagePattern> ImageModelNG::GetImagePattern()
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_RETURN(frameNode, nullptr);
    return AceType::DynamicCast<ImagePattern>(frameNode->GetPattern());
}

ImageResizableSlice ImageModelNG::GetResizableSlice(FrameNode *frameNode)
{
    ImageResizableSlice resizable;
    CHECK_NULL_RETURN(frameNode, resizable);
    auto renderProperty = frameNode->GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_RETURN(renderProperty, resizable);
    return renderProperty->GetImageResizableSlice().value_or(resizable);
}
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_IMAGE_IMAGE_MODEL_NG_CPP
