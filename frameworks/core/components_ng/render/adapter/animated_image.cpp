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

#include "core/components_ng/render/adapter/animated_image.h"

#include <mutex>

#ifdef USE_ROSEN_DRAWING
#include "drawing/engine_adapter/skia_adapter/skia_bitmap.h"
#include "drawing/engine_adapter/skia_adapter/skia_data.h"
#include "drawing/engine_adapter/skia_adapter/skia_image_info.h"
#endif

#include "core/animation/animator.h"
#include "core/animation/picture_animation.h"
#ifndef USE_ROSEN_DRAWING
#include "core/components_ng/image_provider/adapter/skia_image_data.h"
#else
#include "core/components_ng/image_provider/adapter/rosen/drawing_image_data.h"
#endif
#include "core/components_ng/image_provider/image_utils.h"
#include "core/image/sk_image_cache.h"
#include "core/pipeline_ng/pipeline_context.h"
namespace OHOS::Ace::NG {
namespace {
constexpr int32_t STANDARD_FRAME_DURATION = 100;
constexpr int32_t FORM_REPEAT_COUNT = 1;
constexpr float RESIZE_THRESHOLD = 0.7f;
} // namespace

#ifndef USE_ROSEN_DRAWING
RefPtr<CanvasImage> AnimatedImage::Create(
    const RefPtr<SkiaImageData>& data, const ResizeParam& size, const std::string& url)
{
    CHECK_NULL_RETURN(data, nullptr);
    auto skData = data->GetSkData();
    CHECK_NULL_RETURN(skData, nullptr);
    auto codec = SkCodec::MakeFromData(skData);
    CHECK_NULL_RETURN(codec, nullptr);
    if (SystemProperties::GetImageFrameworkEnabled()) {
        auto src = ImageSource::Create(skData->bytes(), skData->size());
        CHECK_NULL_RETURN(src, nullptr);
        return MakeRefPtr<AnimatedPixmap>(codec, src, size, url);
    }
    return MakeRefPtr<AnimatedSkImage>(std::move(codec), url);
}
#else
RefPtr<CanvasImage> AnimatedImage::Create(
    const RefPtr<DrawingImageData>& data, const ResizeParam& size, const std::string& url)
{
    CHECK_NULL_RETURN(data, nullptr);
    auto rsData = data->GetRSData();
    CHECK_NULL_RETURN(rsData, nullptr);
    auto skData = rsData->GetImpl<Rosen::Drawing::SkiaData>()->GetSkData();
    auto codec = SkCodec::MakeFromData(skData);
    CHECK_NULL_RETURN(codec, nullptr);
    if (SystemProperties::GetImageFrameworkEnabled()) {
        auto src = ImageSource::Create(static_cast<const uint8_t*>(rsData->GetData()), rsData->GetSize());
        CHECK_NULL_RETURN(src, nullptr);
        return MakeRefPtr<AnimatedPixmap>(codec, src, size, url);
    }
    return MakeRefPtr<AnimatedRSImage>(std::move(codec), url);
}
#endif

AnimatedImage::AnimatedImage(const std::unique_ptr<SkCodec>& codec, std::string url) : cacheKey_(std::move(url))
{
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    animator_ = CREATE_ANIMATOR(pipelineContext);

    // set up animator
    int32_t totalDuration = 0;
    auto info = codec->getFrameInfo();
    for (int32_t i = 0; i < codec->getFrameCount(); ++i) {
        if (info[i].fDuration <= 0) {
            info[i].fDuration = STANDARD_FRAME_DURATION;
        }
        totalDuration += info[i].fDuration;
    }
    animator_->SetDuration(totalDuration);
    // repetition is 0 => play only once
    auto iteration = codec->getRepetitionCount() + 1;
    if (iteration == 0) {
        iteration = ANIMATION_REPEAT_INFINITE;
    }
    animator_->SetIteration(iteration);
    if (pipelineContext->IsFormRender() && animator_->GetIteration() != 0) {
        animator_->SetIteration(FORM_REPEAT_COUNT);
    }

    // initialize PictureAnimation interpolator
    auto picAnimation = MakeRefPtr<PictureAnimation<uint32_t>>();
    CHECK_NULL_VOID(picAnimation);
    for (int32_t i = 0; i < codec->getFrameCount(); ++i) {
        picAnimation->AddPicture(static_cast<float>(info[i].fDuration) / totalDuration, i);
    }
    picAnimation->AddListener([weak = WeakClaim(this)](const uint32_t idx) {
        auto self = weak.Upgrade();
        CHECK_NULL_VOID(self);
        self->RenderFrame(idx);
    });
    animator_->AddInterpolator(picAnimation);

    LOGD("animated image setup: duration = %{public}d", totalDuration);
    animator_->Play();
}

AnimatedImage::~AnimatedImage()
{
    // animator has to destruct on UI thread
    ImageUtils::PostToUI([animator = animator_]() mutable { animator.Reset(); });
}

void AnimatedImage::ControlAnimation(bool play)
{
    (play) ? animator_->Play() : animator_->Pause();
}

void AnimatedImage::RenderFrame(uint32_t idx)
{
    if (GetCachedFrame(idx)) {
        return;
    }
    ImageUtils::PostToBg([weak = WeakClaim(this), idx] {
        auto self = weak.Upgrade();
        CHECK_NULL_VOID(self);
        self->DecodeFrame(idx);
    });
}

// runs on Background threads
void AnimatedImage::DecodeFrame(uint32_t idx)
{
    // max number of decoding thread = 2
    if (queueSize_ >= 2) {
        // skip frame
        return;
    }
    ++queueSize_;

    ACE_SCOPED_TRACE("decode %s frame %d", cacheKey_.c_str(), idx);
    std::scoped_lock<std::mutex> lock(decodeMtx_);
    DecodeImpl(idx);

    ImageUtils::PostToUI([weak = WeakClaim(this)] {
        auto self = weak.Upgrade();
        CHECK_NULL_VOID(self && self->redraw_);
        self->redraw_();
    });

    CacheFrame(cacheKey_ + std::to_string(idx));
    --queueSize_;
}

bool AnimatedImage::GetCachedFrame(uint32_t idx)
{
    auto image = GetCachedFrameImpl(cacheKey_ + std::to_string(idx));
    CHECK_NULL_RETURN(image, false);

    if (!decodeMtx_.try_lock()) {
        // last frame still decoding, skip this frame to avoid blocking UI thread
        return true;
    }
    UseCachedFrame(std::move(image));

    decodeMtx_.unlock();
    LOGD("frame cache found src = %{public}s, frame = %{public}d", cacheKey_.c_str(), idx);

    if (redraw_) {
        redraw_();
    }
    return true;
}

// ----------------------------------------------------------
// AnimatedSkImage implementation
// ----------------------------------------------------------

#ifndef USE_ROSEN_DRAWING
sk_sp<SkImage> AnimatedSkImage::GetImage() const
#else
std::shared_ptr<RSImage> AnimatedRSImage::GetImage() const
#endif
{
    std::scoped_lock<std::mutex> lock(frameMtx_);
    return currentFrame_;
}

#ifndef USE_ROSEN_DRAWING
void AnimatedSkImage::DecodeImpl(uint32_t idx)
#else
void AnimatedRSImage::DecodeImpl(uint32_t idx)
#endif
{
    SkImageInfo imageInfo = codec_->getInfo();
#ifndef USE_ROSEN_DRAWING
    SkBitmap bitmap;
#else
    RSBitmap bitmap;
#endif

    SkCodec::Options options;
    options.fFrameIndex = idx;

    SkCodec::FrameInfo info {};
    codec_->getFrameInfo(idx, &info);
    if (info.fRequiredFrame != SkCodec::kNoFrame) {
        // frame requires a previous frame as background layer
        options.fPriorFrame = info.fRequiredFrame;
        bitmap = requiredFrame_;
    } else {
        // create from empty layer
#ifndef USE_ROSEN_DRAWING
        bitmap.allocPixels(imageInfo);
#else
        auto info = Rosen::Drawing::ConvertToRSImageInfo(imageInfo);
        bitmap.Build(info);
#endif
    }

    // decode pixels from codec
#ifndef USE_ROSEN_DRAWING
    auto res = codec_->getPixels(imageInfo, bitmap.getPixels(), bitmap.rowBytes(), &options);
#else
    auto res = codec_->getPixels(imageInfo, bitmap.GetPixels(), bitmap.GetRowBytes(), &options);
#endif
    CHECK_NULL_VOID(res == SkCodec::kSuccess);

    // next frame will be drawn on top of this one
    if (info.fDisposalMethod != SkCodecAnimation::DisposalMethod::kRestorePrevious) {
        requiredFrame_ = bitmap;
    }

    // save current frame, notify redraw
    {
        std::scoped_lock<std::mutex> lock(frameMtx_);
#ifndef USE_ROSEN_DRAWING
        currentFrame_ = SkImage::MakeFromBitmap(bitmap);
#else
        currentFrame_ = std::shared_ptr<RSImage>();
        currentFrame_->BuildFromBitmap(bitmap);
#endif
    }
}

#ifndef USE_ROSEN_DRAWING
void AnimatedSkImage::CacheFrame(const std::string& key)
#else
void AnimatedRSImage::CacheFrame(const std::string& key)
#endif
{
    auto ctx = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(ctx);
    auto cache = ctx->GetImageCache();
    CHECK_NULL_VOID(cache);
    std::shared_ptr<CachedImage> cacheNode;
    {
        std::scoped_lock<std::mutex> lock(frameMtx_);
        cacheNode = std::make_shared<CachedImage>(currentFrame_);
    }
    cache->CacheImage(key, cacheNode);
}

#ifndef USE_ROSEN_DRAWING
RefPtr<CanvasImage> AnimatedSkImage::GetCachedFrameImpl(const std::string& key)
{
    return SkiaImage::QueryFromCache(key);
}
#else
RefPtr<CanvasImage> AnimatedRSImage::GetCachedFrameImpl(const std::string& key)
{
    return DrawingImage::QueryFromCache(key);
}
#endif

#ifndef USE_ROSEN_DRAWING
void AnimatedSkImage::UseCachedFrame(RefPtr<CanvasImage>&& image)
{
    std::scoped_lock<std::mutex> lock(frameMtx_);
    currentFrame_ = DynamicCast<SkiaImage>(image)->GetImage();
}
#else
void AnimatedRSImage::UseCachedFrame(RefPtr<CanvasImage>&& image)
{
    std::scoped_lock<std::mutex> lock(frameMtx_);
    currentFrame_ = DynamicCast<DrawingImage>(image)->GetImage();
}
#endif

// ----------------------------------------------------------
// AnimatedPixmap implementation
// ----------------------------------------------------------
AnimatedPixmap::AnimatedPixmap(
    const std::unique_ptr<SkCodec>& codec, const RefPtr<ImageSource>& src, const ResizeParam& size, std::string url)
    : AnimatedImage(codec, std::move(url)), size_(size), src_(src)
{
    // resizing to a size >= 0.7 [~= sqrt(2) / 2] intrinsic size takes 2x longer to decode while memory usage is 1/2.
    // 0.7 is the balance point.
    auto intrSize = src_->GetImageSize();
    if (intrSize.first * RESIZE_THRESHOLD >= size_.width || intrSize.second * RESIZE_THRESHOLD >= size_.height) {
        size_.forceResize = true;
    }
}

RefPtr<PixelMap> AnimatedPixmap::GetPixelMap() const
{
    std::scoped_lock<std::mutex> lock(frameMtx_);
    return currentFrame_;
}

void AnimatedPixmap::DecodeImpl(uint32_t idx)
{
    RefPtr<PixelMap> frame;
    if (size_.forceResize) {
        frame = src_->CreatePixelMap(idx, { size_.width, size_.height });
    } else {
        // decode to intrinsic size
        frame = src_->CreatePixelMap(idx, { -1, -1 });
    }
    std::scoped_lock<std::mutex> lock(frameMtx_);
    currentFrame_ = frame;
}

void AnimatedPixmap::CacheFrame(const std::string& key)
{
    auto ctx = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(ctx);
    auto cache = ctx->GetImageCache();
    CHECK_NULL_VOID(cache);

    std::scoped_lock<std::mutex> lock(frameMtx_);
    cache->CacheImageData(key, MakeRefPtr<PixmapData>(currentFrame_));
}

RefPtr<CanvasImage> AnimatedPixmap::GetCachedFrameImpl(const std::string& key)
{
    return PixelMapImage::QueryFromCache(key);
}

void AnimatedPixmap::UseCachedFrame(RefPtr<CanvasImage>&& image)
{
    std::scoped_lock<std::mutex> lock(frameMtx_);
    currentFrame_ = DynamicCast<PixelMapImage>(image)->GetPixelMap();
}
} // namespace OHOS::Ace::NG
