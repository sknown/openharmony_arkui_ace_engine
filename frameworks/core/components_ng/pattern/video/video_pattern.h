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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_VIDEO_VIDEO_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_VIDEO_VIDEO_PATTERN_H
#include "base/geometry/dimension.h"
#include "base/geometry/size.h"
#include "base/memory/referenced.h"
#include "base/utils/noncopyable.h"
#include "core/components/video/video_controller_v2.h"
#include "core/components_ng/image_provider/image_loading_context.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/video/video_accessibility_property.h"
#include "core/components_ng/pattern/video/video_event_hub.h"
#include "core/components_ng/pattern/video/video_layout_algorithm.h"
#include "core/components_ng/pattern/video/video_layout_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/media_player.h"
#include "core/components_ng/render/render_surface.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/base/geometry/rect.h"

namespace OHOS::Ace {
class ImageAnalyzerManager;
}
namespace OHOS::Ace::NG {
class VideoPattern : public Pattern {
    DECLARE_ACE_TYPE(VideoPattern, Pattern);

public:
    using HiddenChangeEvent = std::function<void(bool)>;

    VideoPattern() = delete;
    explicit VideoPattern(const RefPtr<VideoControllerV2>& videoController);
    ~VideoPattern() override;

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<VideoEventHub>();
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<VideoLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<VideoLayoutAlgorithm>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<VideoAccessibilityProperty>();
    }

    bool DefaultSupportDrag() override
    {
        return true;
    }

    bool IsSupportDrawModifier() const override
    {
        return false;
    }

    void UpdateMuted(bool muted)
    {
        muted_ = muted;
    }

    bool GetMuted() const
    {
        return muted_;
    }

    void UpdateAutoPlay(bool autoPlay)
    {
        autoPlay_ = autoPlay;
    }

    bool GetAutoPlay() const
    {
        return autoPlay_;
    }

    void UpdateLoop(bool loop)
    {
        loop_ = loop;
    }

    bool GetLoop() const
    {
        return loop_;
    }

    virtual bool IsFullScreen() const;

    void OnColorConfigurationUpdate() override;
    void UpdateProgressRate(double progressRate)
    {
        progressRate_ = progressRate;
    }

    double GetProgressRate() const
    {
        return progressRate_;
    }

    FocusPattern GetFocusPattern() const override
    {
        // Video focus type is scope, it is a container, inner focus is on slider now.
        return { FocusType::SCOPE, true };
    }

    RefPtr<FrameNode> CreateControlBar(int32_t nodeId = -1);

    void SetHiddenChangeEvent(HiddenChangeEvent&& hiddenChangeEvent)
    {
        hiddenChangeEvent_ = std::move(hiddenChangeEvent);
    }

    uint32_t GetCurrentPos() const
    {
        return currentPos_;
    }

    uint32_t GetDuration() const
    {
        return duration_;
    }

    bool GetInitialState() const
    {
        return isInitialState_;
    }

    virtual bool OnBackPressed()
    {
        return false;
    }

    void OnVisibleChange(bool isVisible) override;

    void OnAreaChangedInner() override;

    // It is used to init mediaplayer on background.
    void UpdateMediaPlayerOnBg();
    void ResetMediaPlayer();

    void EnableDrag();
    void SetIsStop(bool isStop)
    {
        isStop_ = isStop;
    }

    bool GetIsStop() const
    {
        return isStop_;
    }

    void SetIsDrag(bool isDrag)
    {
        isDrag_ = isDrag;
    }

    bool IsInitialState() const
    {
        return isInitialState_;
    }

    void SetIsDragEndAutoPlay(bool isDragEndAutoPlay)
    {
        dragEndAutoPlay_ = isDragEndAutoPlay;
    }

    const std::string& GetSrc() const
    {
        return src_;
    }

    void UpdateMediaParam(const RefPtr<MediaPlayer>& mediaPlayer, const RefPtr<RenderSurface>& renderSurface,
        const RefPtr<RenderContext>& renderContext)
    {
        mediaPlayer_ = AceType::Claim(AceType::RawPtr(mediaPlayer));
        renderSurface_ = AceType::Claim(AceType::RawPtr(renderSurface));
        renderContextForMediaPlayer_ = AceType::Claim(AceType::RawPtr(renderContext));
    }

    void ResetMediaParam()
    {
        mediaPlayer_.Reset();
        renderSurface_.Reset();
        renderContextForMediaPlayer_.Reset();
    }

    void OnFullScreenChange(bool isFullScreen);

    void RecoverState(const RefPtr<VideoPattern>& videoPattern);

    bool NeedLift() const;

    RefPtr<FrameNode> GetFullScreenNode() const
    {
        if (!fullScreenNodeId_.has_value()) {
            return nullptr;
        }
        return FrameNode::GetFrameNode(V2::VIDEO_ETS_TAG, fullScreenNodeId_.value());
    }

    void OnPlayerStatus(PlaybackStatus status);

    void OnCurrentTimeChange(uint32_t currentPos);

    void OnError(const std::string& errorId);

    void OnResolutionChange() const;

    void OnStartRenderFrameCb() const;

    void ResetLastBoundsRect()
    {
        lastBoundsRect_.SetRect(0.0f, 0.0f, 0.0f, 0.0f);
    }

    RefPtr<VideoPattern> GetTargetVideoPattern();
    void EnableAnalyzer(bool enable);
    void SetImageAnalyzerConfig(void* config);
    void SetImageAIOptions(void* options);
    bool GetAnalyzerState();
    void UpdateAnalyzerState(bool isCreated)
    {
        isAnalyzerCreated_ = isCreated;
    }

#ifdef RENDER_EXTRACT_SUPPORTED
    void OnTextureRefresh(void* surface);
#endif

protected:
    void OnUpdateTime(uint32_t time, int pos) const;
    void RegisterMediaPlayerEvent();

    RefPtr<MediaPlayer> mediaPlayer_ = MediaPlayer::Create();
    RefPtr<RenderSurface> renderSurface_ = RenderSurface::Create();
    RefPtr<RenderContext> renderContextForMediaPlayer_ = RenderContext::Create();

    int32_t instanceId_;

#if defined(RENDER_EXTRACT_SUPPORTED) && defined(ENABLE_ROSEN_BACKEND)
    WeakPtr<RenderSurface> renderSurfaceWeakPtr_;
    WeakPtr<RenderContext> renderContextForMediaPlayerWeakPtr_;
#endif

private:
    void OnAttachToFrameNode() override;
    void OnDetachFromFrameNode(FrameNode* frameNode) override;
    void OnDetachFromMainTree() override;
    void OnModifyDone() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    void OnRebuildFrame() override;
    void OnWindowHide() override;

    // Set properties for media player.
    void PrepareMediaPlayer();
    void SetMethodCall();

    bool SetSourceForMediaPlayer();
    void UpdateLooping();
    void UpdateSpeed();
    void UpdateMuted();
    void PrepareSurface();

    bool HasPlayer() const;

    // Functions for the video controller.
    void Start();
    void Pause();
    void Stop();
    void FullScreen();

    void SetCurrentTime(float currentPos, SeekMode seekMode = SeekMode::SEEK_PREVIOUS_SYNC);
    void SetFullScreenButtonCallBack(RefPtr<FrameNode>& fullScreenBtn);

    void OnPrepared(double width, double height, uint32_t duration, uint32_t currentPos, bool needFireEvent);
    void OnCompletion();
    void OnSliderChange(float posTime, int32_t mode);

    void UpdatePreviewImage();
    void UpdateControllerBar();
    void UpdateVideoProperty();

    static RefPtr<FrameNode> CreateSVG();
    RefPtr<FrameNode> CreateText(uint32_t time);
    RefPtr<FrameNode> CreateSlider();
    void ChangePlayButtonTag();
    void ChangePlayButtonTag(RefPtr<FrameNode>& playBtn);

    void ChangeFullScreenButtonTag(bool isFullScreen, RefPtr<FrameNode>& fullScreenBtn);
    void ResetStatus();
    void HiddenChange(bool hidden);
    void PrintPlayerStatus(PlaybackStatus status);

    void UpdateFsState();
    void checkNeedAutoPlay();

    // Fire error manually, eg. src is not existed. It must run on ui.
    void FireError();

    HiddenChangeEvent CreateHiddenChangeEvent();

    void SetMediaFullScreen(bool isFullScreen)
    {
        mediaPlayer_->FullScreenChange(isFullScreen);
        if (SystemProperties::GetExtSurfaceEnabled()) {
            renderSurface_->SetIsFullScreen(isFullScreen);
        }
    }

#ifdef RENDER_EXTRACT_SUPPORTED
    void* GetNativeWindow(int32_t instanceId, int64_t textureId);
#endif

    void RegisterRenderContextCallBack();
    void ChangePlayerStatus(bool isPlaying, const PlaybackStatus& status);

    bool IsSupportImageAnalyzer();
    void StartImageAnalyzer();
    void CreateAnalyzerOverlay();
    void DestroyAnalyzerOverlay();
    void UpdateAnalyzerOverlay();
    void UpdateAnalyzerUIConfig(const RefPtr<NG::GeometryNode>& geometryNode);

    RefPtr<VideoControllerV2> videoControllerV2_;
    RefPtr<FrameNode> controlBar_;

    GestureEventFunc playBtnCallBack_;
    GestureEventFunc pauseBtnCallBack_;
    HiddenChangeEvent hiddenChangeEvent_;

    // Video src.
    std::string src_;
    bool isInitialState_ = true; // Initial state is true. Play or seek will set it to false.
    bool isPlaying_ = false;

    bool isStop_ = false;
    bool isDrag_ = false;

    bool muted_ = false;
    bool autoPlay_ = false;
    bool loop_ = false;

    bool pastPlayingStatus_ = false;

    bool dragEndAutoPlay_ = false;
    bool isEnableAnalyzer_ = false;
    bool isAnalyzerCreated_ = false;
    bool isPaused_ = false;

    uint32_t currentPos_ = 0;
    uint32_t duration_ = 0;

    // full screen node id
    std::optional<int32_t> fullScreenNodeId_;

    // Video playback speed.
    double progressRate_ = 1.0;

    Rect lastBoundsRect_;
    Rect contentRect_;
    std::shared_ptr<ImageAnalyzerManager> imageAnalyzerManager_;

    ACE_DISALLOW_COPY_AND_MOVE(VideoPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_VIDEO_VIDEO_PATTERN_H
