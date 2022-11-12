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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RATING_RATING_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RATING_RATING_PATTERN_H

#include <cstdint>

#include "core/components/rating/rating_theme.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/rating/rating_event_hub.h"
#include "core/components_ng/pattern/rating/rating_layout_algorithm.h"
#include "core/components_ng/pattern/rating/rating_layout_property.h"
#include "core/components_ng/pattern/rating/rating_render_property.h"
#include "core/components_ng/render/canvas_image.h"

namespace OHOS::Ace::NG {

#define ACE_DEFINE_RATING_GET_PROPERTY_FROM_THEME(name, type)            \
    static std::optional<type> Get##name##FromTheme()                    \
    {                                                                    \
        do {                                                             \
            auto pipelineContext = PipelineContext::GetCurrentContext(); \
            CHECK_NULL_RETURN(pipelineContext, std::nullopt);            \
            auto ratingTheme = pipelineContext->GetTheme<RatingTheme>(); \
            CHECK_NULL_RETURN(ratingTheme, std::nullopt);                \
            return ratingTheme->Get##name();                             \
        } while (false);                                                 \
    }

class RatingPattern : public Pattern {
    DECLARE_ACE_TYPE(RatingPattern, Pattern);

public:
    RatingPattern() = default;
    ~RatingPattern() override = default;

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override;

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<RatingLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<RatingLayoutAlgorithm>(
            foregroundImageLoadingCtx_, secondaryImageLoadingCtx_, backgroundImageLoadingCtx_);
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<RatingRenderProperty>();
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<RatingEventHub>();
    }

    // Called on main thread to check if need rerender of the content.
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    ACE_DEFINE_RATING_GET_PROPERTY_FROM_THEME(RatingScore, double);
    ACE_DEFINE_RATING_GET_PROPERTY_FROM_THEME(StepSize, double);
    ACE_DEFINE_RATING_GET_PROPERTY_FROM_THEME(StarNum, int32_t);

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, true, FocusStyle::INNER_BORDER };
    }

private:
    void OnModifyDone() override;

    void ConstrainsRatingScore();
    void OnImageDataReady(int32_t imageFlag);
    void OnImageLoadSuccess(int32_t imageFlag);
    void CheckImageInfoHasChangedOrNot(
        int32_t imageFlag, const ImageSourceInfo& sourceInfo, const std::string& lifeCycleTag);
    static ImageSourceInfo GetImageSourceInfoFromTheme(int32_t imageFlag);

    // Init pan recognizer to update render when drag updates, fire change event when drag ends.
    void InitPanEvent(const RefPtr<GestureEventHub>& gestureHub);

    // Init touch event, show press effect when touch down, update render when touch up.
    void InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub);

    // Init touch event, update render when click.
    void InitClickEvent(const RefPtr<GestureEventHub>& gestureHub);

    // Init on key event
    void InitOnKeyEvent(const RefPtr<FocusHub>& focusHub);
    bool OnKeyEvent(const KeyEvent& event);

    void HandleDragUpdate(const GestureEvent& info);
    void HandleDragEnd();
    void HandleTouchDown(const Offset& localPosition);
    void HandleTouchUp();
    void HandleClick(const GestureEvent& info);
    void FireChangeEvent() const;
    void RecalculatedRatingScoreBasedOnEventPoint(double eventPointX);
    bool IsIndicator();
    void UpdateInternalResource(ImageSourceInfo& sourceInfo, int32_t imageFlag);

    RefPtr<PanEvent> panEvent_;
    RefPtr<TouchEventImpl> touchEvent_;
    RefPtr<ClickEvent> clickEvent_;

    DataReadyNotifyTask CreateDataReadyCallback(int32_t imageFlag);
    LoadSuccessNotifyTask CreateLoadSuccessCallback(int32_t imageFlag);
    LoadFailNotifyTask CreateLoadFailCallback(int32_t imageFlag);

    RefPtr<ImageLoadingContext> foregroundImageLoadingCtx_;
    RefPtr<ImageLoadingContext> secondaryImageLoadingCtx_;
    RefPtr<ImageLoadingContext> backgroundImageLoadingCtx_;

    RefPtr<CanvasImage> foregroundImageCanvas_;
    RefPtr<CanvasImage> secondaryImageCanvas_;
    RefPtr<CanvasImage> backgroundImageCanvas_;
    ImagePaintConfig singleStarImagePaintConfig_;
    RectF singleStarDstRect_;
    RectF singleStarRect_;
    int32_t imageReadyStateCode_ = 0;
    int32_t imageSuccessStateCode_ = 0;
    bool hasInit_ = false;
    double lastRatingScore_ = 0.0;

    bool isForegroundImageInfoFromTheme_ = false;
    bool isSecondaryImageInfoFromTheme_ = false;
    bool isBackgroundImageInfoFromTheme_ = false;
    // get XTS inspector value
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override;
    ACE_DISALLOW_COPY_AND_MOVE(RatingPattern);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RATING_RATING_PATTERN_H