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

#include "core/components_ng/image_provider/image_provider.h"

#include <cstdint>
#include <mutex>

#include "base/log/ace_trace.h"
#include "base/memory/referenced.h"
#include "base/subwindow/subwindow_manager.h"
#include "core/components_ng/image_provider/adapter/image_decoder.h"
#ifndef USE_ROSEN_DRAWING
#include "core/components_ng/image_provider/adapter/skia_image_data.h"
#else
#include "core/components_ng/image_provider/adapter/rosen/drawing_image_data.h"
#endif
#include "core/components_ng/image_provider/animated_image_object.h"
#include "core/components_ng/image_provider/image_loading_context.h"
#include "core/components_ng/image_provider/image_object.h"
#include "core/components_ng/image_provider/image_utils.h"
#include "core/components_ng/image_provider/pixel_map_image_object.h"
#include "core/components_ng/image_provider/static_image_object.h"
#include "core/components_ng/image_provider/svg_image_object.h"
#ifndef USE_ROSEN_DRAWING
#include "core/components_ng/render/adapter/skia_image.h"
#else
#include "core/components_ng/render/adapter/rosen/drawing_image.h"
#endif
#include "core/image/image_loader.h"
#include "core/image/sk_image_cache.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

void ImageProvider::CacheImageObject(const RefPtr<ImageObject>& obj)
{
    CHECK_NULL_VOID(obj);
    auto pipelineCtx = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineCtx);
    auto cache = pipelineCtx->GetImageCache();
    CHECK_NULL_VOID(cache);
    if (cache && obj->IsSupportCache()) {
        cache->CacheImgObjNG(obj->GetSourceInfo().GetKey(), obj);
    }
}

std::mutex ImageProvider::taskMtx_;
std::unordered_map<std::string, ImageProvider::Task> ImageProvider::tasks_;

bool ImageProvider::PrepareImageData(const RefPtr<ImageObject>& imageObj)
{
    CHECK_NULL_RETURN(imageObj, false);
    // data already loaded
    if (imageObj->GetData()) {
        return true;
    }
    // if image object has no skData, reload data.
    auto imageLoader = ImageLoader::CreateImageLoader(imageObj->GetSourceInfo());
    CHECK_NULL_RETURN(imageLoader, false);

    auto container = Container::Current();
    if (container && container->IsSubContainer()) {
        TAG_LOGI(AceLogTag::ACE_IMAGE, "subContainer's pipeline's dataProviderManager is null, cannot load image "
                                       "source, need to switch pipeline in parentContainer.");
        auto currentId = SubwindowManager::GetInstance()->GetParentContainerId(Container::CurrentId());
        container = Container::GetContainer(currentId);
    }
    CHECK_NULL_RETURN(container, false);
    auto pipeline = container->GetPipelineContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto newLoadedData = imageLoader->GetImageData(imageObj->GetSourceInfo(), WeakClaim(RawPtr(pipeline)));
    CHECK_NULL_RETURN(newLoadedData, false);
    // load data success
    imageObj->SetData(newLoadedData);
    return true;
}

RefPtr<ImageObject> ImageProvider::QueryThumbnailCache(const ImageSourceInfo& src)
{
    // query thumbnail from cache
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto cache = pipeline->GetImageCache();
    CHECK_NULL_RETURN(cache, nullptr);
    auto data = DynamicCast<PixmapData>(cache->GetCacheImageData(src.GetKey()));
    if (data) {
        return PixelMapImageObject::Create(src, data);
    }
    return nullptr;
}

RefPtr<ImageObject> ImageProvider::QueryImageObjectFromCache(const ImageSourceInfo& src)
{
    if (src.GetSrcType() == SrcType::DATA_ABILITY_DECODED) {
        return QueryThumbnailCache(src);
    }
    if (!src.SupportObjCache()) {
        return nullptr;
    }
    auto pipelineCtx = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineCtx, nullptr);
    auto imageCache = pipelineCtx->GetImageCache();
    CHECK_NULL_RETURN(imageCache, nullptr);
    RefPtr<ImageObject> imageObj = imageCache->GetCacheImgObjNG(src.GetKey());
    return imageObj;
}

void ImageProvider::FailCallback(const std::string& key, const std::string& errorMsg, bool sync)
{
    auto ctxs = EndTask(key);
    for (auto&& it : ctxs) {
        auto ctx = it.Upgrade();
        if (!ctx) {
            continue;
        }

        if (sync) {
            ctx->FailCallback(errorMsg);
        } else {
            // NOTE: contexts may belong to different arkui pipelines
            auto notifyLoadFailTask = [ctx, errorMsg] {
                ctx->FailCallback(errorMsg);
            };

            ImageUtils::PostToUI(std::move(notifyLoadFailTask), "ArkUIImageProviderFail", ctx->GetContainerId());
        }
    }
}

void ImageProvider::SuccessCallback(
    const RefPtr<CanvasImage>& canvasImage, const std::string& key, bool sync, bool loadInVipChannel)
{
    canvasImage->Cache(key);
    auto ctxs = EndTask(key);
    // when upload success, pass back canvasImage to LoadingContext
    for (auto&& it : ctxs) {
        auto ctx = it.Upgrade();
        if (!ctx) {
            continue;
        }
        if (sync) {
            ctx->SuccessCallback(canvasImage->Clone());
        } else {
            // NOTE: contexts may belong to different arkui pipelines
            auto notifyLoadSuccess = [ctx, canvasImage] { ctx->SuccessCallback(canvasImage->Clone()); };
            ImageUtils::PostToUI(std::move(notifyLoadSuccess), "ArkUIImageProviderSuccess", ctx->GetContainerId(),
                loadInVipChannel ? PriorityType::VIP : PriorityType::LOW);
        }
    }
}

void ImageProvider::CreateImageObjHelper(const ImageSourceInfo& src, bool sync)
{
    ACE_SCOPED_TRACE("CreateImageObj %s", src.ToString().c_str());
    // load image data
    auto imageLoader = ImageLoader::CreateImageLoader(src);
    if (!imageLoader) {
        std::string errorMessage("Failed to create image loader, Image source type not supported");
        FailCallback(src.GetKey(), src.ToString() + errorMessage, sync);
        return;
    }
    auto pipeline = PipelineContext::GetCurrentContext();
    RefPtr<ImageData> data = imageLoader->GetImageData(src, WeakClaim(RawPtr(pipeline)));
    if (!data) {
        FailCallback(src.GetKey(), "Failed to load image data", sync);
        return;
    }

    // build ImageObject
    RefPtr<ImageObject> imageObj = ImageProvider::BuildImageObject(src, data);
    if (!imageObj) {
        FailCallback(src.GetKey(), "Failed to build image object", sync);
        return;
    }

    auto cloneImageObj = imageObj->Clone();

    // ImageObject cache is only for saving image size info, clear data to save memory
    cloneImageObj->ClearData();

    CacheImageObject(cloneImageObj);

    auto ctxs = EndTask(src.GetKey());
    // callback to LoadingContext
    for (auto&& it : ctxs) {
        auto ctx = it.Upgrade();
        if (!ctx) {
            continue;
        }
        if (sync) {
            ctx->DataReadyCallback(imageObj);
        } else {
            // NOTE: contexts may belong to different arkui pipelines
            auto notifyDataReadyTask = [ctx, imageObj, src] {
                ctx->DataReadyCallback(imageObj);
            };
            ImageUtils::PostToUI(std::move(notifyDataReadyTask), "ArkUIImageProviderDataReady", ctx->GetContainerId());
        }
    }
}

bool ImageProvider::RegisterTask(const std::string& key, const WeakPtr<ImageLoadingContext>& ctx)
{
    std::scoped_lock<std::mutex> lock(taskMtx_);
    // key exists -> task is running
    auto it = tasks_.find(key);
    if (it != tasks_.end()) {
        it->second.ctxs_.insert(ctx);
        return false;
    }
    tasks_[key].ctxs_.insert(ctx);
    return true;
}

std::set<WeakPtr<ImageLoadingContext>> ImageProvider::EndTask(const std::string& key)
{
    std::scoped_lock<std::mutex> lock(taskMtx_);
    auto it = tasks_.find(key);
    if (it == tasks_.end()) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, "task not found in map %{private}s", key.c_str());
        return {};
    }
    auto ctxs = it->second.ctxs_;
    if (ctxs.empty()) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, "registered task has empty context %{public}s", key.c_str());
    }
    tasks_.erase(it);
    return ctxs;
}

void ImageProvider::CancelTask(const std::string& key, const WeakPtr<ImageLoadingContext>& ctx)
{
    std::scoped_lock<std::mutex> lock(taskMtx_);
    auto it = tasks_.find(key);
    CHECK_NULL_VOID(it != tasks_.end());
    CHECK_NULL_VOID(it->second.ctxs_.find(ctx) != it->second.ctxs_.end());
    // only one LoadingContext waiting for this task, can just cancel
    if (it->second.ctxs_.size() == 1) {
        bool canceled = it->second.bgTask_.Cancel();
        if (canceled) {
            tasks_.erase(it);
            return;
        }
    }
    // other LoadingContext still waiting for this task, remove ctx from set
    it->second.ctxs_.erase(ctx);
}

void ImageProvider::CreateImageObject(const ImageSourceInfo& src, const WeakPtr<ImageLoadingContext>& ctxWp, bool sync)
{
    if (!RegisterTask(src.GetKey(), ctxWp)) {
        // task is already running, only register callbacks
        return;
    }
    if (sync) {
        CreateImageObjHelper(src, true);
    } else {
        std::scoped_lock<std::mutex> lock(taskMtx_);
        // wrap with [CancelableCallback] and record in [tasks_] map
        CancelableCallback<void()> task;
        task.Reset([src, ctxWp] { ImageProvider::CreateImageObjHelper(src); });
        tasks_[src.GetKey()].bgTask_ = task;
        auto ctx = ctxWp.Upgrade();
        CHECK_NULL_VOID(ctx);
        ImageUtils::PostToBg(task, "ArkUIImageProviderCreateImageObject", ctx->GetContainerId());
    }
}

RefPtr<ImageObject> ImageProvider::BuildImageObject(const ImageSourceInfo& src, const RefPtr<ImageData>& data)
{
    if (!data) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, "data is null when try ParseImageObjectType, src: %{public}s",
            src.ToString().c_str());
        return nullptr;
    }
    if (src.IsSvg()) {
        // SVG object needs to make SVG dom during creation
        return SvgImageObject::Create(src, data);
    }
    if (src.IsPixmap()) {
        return PixelMapImageObject::Create(src, data);
    }

#ifndef USE_ROSEN_DRAWING
    auto skiaImageData = DynamicCast<SkiaImageData>(data);
    CHECK_NULL_RETURN(skiaImageData, nullptr);
    auto [size, frameCount] = skiaImageData->Parse();
#else
    auto rosenImageData = DynamicCast<DrawingImageData>(data);
    CHECK_NULL_RETURN(rosenImageData, nullptr);
    auto [size, frameCount] = rosenImageData->Parse();
#endif
    if (!size.IsPositive()) {
        TAG_LOGW(AceLogTag::ACE_IMAGE,
            "Image of src: %{public}s, imageData's size is invalid %{public}s, frameCount is %{public}d",
            src.ToString().c_str(), size.ToString().c_str(), frameCount);
        return nullptr;
    }
    if (frameCount > 1) {
        auto imageObject = MakeRefPtr<AnimatedImageObject>(src, size, data);
        imageObject->SetFrameCount(frameCount);
        return imageObject;
    }
    return MakeRefPtr<StaticImageObject>(src, size, data);
}

void ImageProvider::MakeCanvasImage(const RefPtr<ImageObject>& obj, const WeakPtr<ImageLoadingContext>& ctxWp,
    const SizeF& size, const ImageDecoderOptions& imageDecoderOptions)
{
    auto key = ImageUtils::GenerateImageKey(obj->GetSourceInfo(), size);
    // check if same task is already executing
    if (!RegisterTask(key, ctxWp)) {
        return;
    }
    if (imageDecoderOptions.sync) {
        MakeCanvasImageHelper(obj, size, key, imageDecoderOptions);
    } else {
        std::scoped_lock<std::mutex> lock(taskMtx_);
        // wrap with [CancelableCallback] and record in [tasks_] map
        CancelableCallback<void()> task;
        task.Reset([key, obj, size, imageDecoderOptions] {
            MakeCanvasImageHelper(obj, size, key, imageDecoderOptions);
        });
        tasks_[key].bgTask_ = task;
        auto ctx = ctxWp.Upgrade();
        CHECK_NULL_VOID(ctx);
        ImageUtils::PostToBg(task, "ArkUIImageProviderMakeCanvasImage", ctx->GetContainerId());
    }
}

void ImageProvider::MakeCanvasImageHelper(const RefPtr<ImageObject>& obj, const SizeF& size, const std::string& key,
    const ImageDecoderOptions& imageDecoderOptions)
{
    ImageDecoder decoder(obj, size, imageDecoderOptions.forceResize);
    RefPtr<CanvasImage> image;
    if (SystemProperties::GetImageFrameworkEnabled()) {
        image = decoder.MakePixmapImage(imageDecoderOptions.imageQuality, imageDecoderOptions.isHdrDecoderNeed);
    } else {
#ifndef USE_ROSEN_DRAWING
        image = decoder.MakeSkiaImage();
#else
        image = decoder.MakeDrawingImage();
#endif
    }

    if (image) {
        SuccessCallback(image, key, imageDecoderOptions.sync, imageDecoderOptions.loadInVipChannel);
    } else {
        FailCallback(key, "Failed to decode image");
    }
}
} // namespace OHOS::Ace::NG
