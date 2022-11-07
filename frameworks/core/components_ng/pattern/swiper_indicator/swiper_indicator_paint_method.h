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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SWIPER_INDICATOR_SWIPER_INDICATOR_PAINT_METHOD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SWIPER_INDICATOR_SWIPER_INDICATOR_PAINT_METHOD_H

#include "core/components/common/properties/swiper_indicator.h"
#include "core/components_ng/pattern/swiper_indicator/swiper_indicator_paint_property.h"
#include "core/components_ng/render/canvas.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/node_paint_method.h"
#include "core/components_ng/render/paint_wrapper.h"
#include "core/components_ng/render/render_context.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT SwiperIndicatorPaintMethod : public NodePaintMethod {
    DECLARE_ACE_TYPE(SwiperIndicatorPaintMethod, NodePaintMethod)
public:
    SwiperIndicatorPaintMethod(float mainDelta, Axis axis, int32_t currentIndex, int32_t itemCount, bool showIndicator)
        : mainDelta_(mainDelta), axis_(axis), currentIndex_(currentIndex), itemCount_(itemCount),
          showIndicator_(showIndicator) {};
    ~SwiperIndicatorPaintMethod() override = default;

    CanvasDrawFunction GetContentDrawFunction(PaintWrapper* paintWrapper) override;

    void SetCurrentIndex(int32_t index)
    {
        currentIndex_ = index;
    }

    int32_t GetCurrentIndex() const
    {
        return currentIndex_;
    }

    void SetItemCount(int32_t itemCount)
    {
        itemCount_ = itemCount;
    }

    void SetMainDelta(float mainDelta)
    {
        mainDelta_ = mainDelta;
    }

    float GetMainDelta() const
    {
        return mainDelta_;
    }

    void SetAxis(Axis axis)
    {
        axis_ = axis;
    }

    Axis GetAxis() const
    {
        return axis_;
    }

    void SetShowIndicator(bool showIndicator)
    {
        showIndicator_ = showIndicator;
    }

private:
    void PaintContent(RSCanvas& canvas, const RefPtr<SwiperIndicatorPaintProperty>& paintProperty, SizeF contentSize);
    void PaintMask(
        RSCanvas& canvas, RefPtr<SwiperIndicatorPaintProperty> paintProperty, SizeF contentSize, OffsetF contentOffset);
    float mainDelta_ = 0.0f;
    Axis axis_ = Axis::HORIZONTAL;
    RefPtr<OHOS::Ace::SwiperIndicator> indicator_;
    Offset indicatorPosition_;
    int32_t currentIndex_ = 0;
    int32_t itemCount_ = 1;
    bool showIndicator_ = true;

    ACE_DISALLOW_COPY_AND_MOVE(SwiperIndicatorPaintMethod);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SWIPER_INDICATOR_SWIPER_INDICATOR_PAINT_METHOD_H