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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_SWIPER_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_SWIPER_PATTERN_H

#include "base/geometry/axis.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/swiper/swiper_layout_algorithm.h"
#include "core/components_ng/pattern/swiper/swiper_layout_property.h"
#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace::NG {

class SwiperPattern : public Pattern {
    DECLARE_ACE_TYPE(SwiperPattern, Pattern);

public:
    SwiperPattern() = default;
    ~SwiperPattern() override = default;

    bool IsAtomicNode() const override 
    {
        return false;
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<SwiperLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        LOGE("CCCC CreateLayoutAlgorithm: %{public}lf", currentOffset_);
        auto layoutAlgorithm = MakeRefPtr<SwiperLayoutAlgorithm>(currentIndex_, startIndex_, endIndex_);
        layoutAlgorithm->SetCurrentOffset(currentOffset_);
        return layoutAlgorithm;
    }

    void UpdateCurrentOffset(float offset);

private:
    void OnModifyDone() override;
    void OnAttachToFrameNode() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, bool skipMeasure, bool skipLayout) override;

    // Init pan recognizer to move items when drag update, play translate animation when drag end.
    void InitPanEvent(const RefPtr<GestureEventHub>& gestureHub);

    // Init touch event, stop animation when touch down.
    void InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub);

    void HandleDragStart();
    void HandleDragEnd(double dragVelocity);
    void HandleTouchEvent(const TouchEventInfo& info);

    void PlayTranslateAnimation(float startPos, float endPos, int32_t nextIndex);

    Axis GetDirection() const;

    RefPtr<PanEvent> panEvent_;
    RefPtr<TouchEventImpl> touchEvent_;

    RefPtr<Animator> controller_;

    int32_t startIndex_ = 0;
    int32_t endIndex_ = 0;
    int32_t currentIndex_ = 0;

    float currentOffset_ = 0.0;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_SWIPER_PATTERN_H
