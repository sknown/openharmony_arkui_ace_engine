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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SLIDER_SLIDER_TIP_MODIFIER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SLIDER_SLIDER_TIP_MODIFIER_H

#include "core/components_ng/base/modifier.h"
#include "core/components_ng/pattern/slider/slider_content_modifier.h"
#include "core/components_ng/render/paragraph.h"

namespace OHOS::Ace::NG {
class SliderTipModifier : public OverlayModifier {
    DECLARE_ACE_TYPE(SliderTipModifier, OverlayModifier);

public:
    explicit SliderTipModifier(
        std::function<OffsetF()> getBlockCenterFunc);
    ~SliderTipModifier() override;

    void PaintTip(DrawingContext& context);
    void PaintBubble(DrawingContext& context);

    void onDraw(DrawingContext& context) override;

    void SetParagraph(const RefPtr<NG::Paragraph>& paragraph)
    {
        paragraph_ = paragraph;
    }

    void SetCircleCenter(const OffsetF& center)
    {
        if (blockCenter_) {
            blockCenter_->Set(center);
        }
    }

    void SetTextFont(const Dimension& fontSize)
    {
        textFontSize_ = fontSize;
    }

    void SetContent(const std::string& content)
    {
        if (content_) {
            content_->Set(content);
        }
    }

    void SetTextColor(const Color& textColor)
    {
        textColor_ = textColor;
    }

    void SetDirection(const Axis& axis)
    {
        axis_ = axis;
    }

    void SetTipColor(const Color& color)
    {
        tipColor_ = color;
    }

    void SetTipFlag(bool flag);

    void SetContentOffset(const OffsetF& contentOffset)
    {
        if (contentOffset_) {
            contentOffset_->Set(contentOffset);
        }
    }

    void SetContentSize(const SizeF& contentSize)
    {
        if (contentSize_) {
            contentSize_->Set(contentSize);
        }
    }

    void SetBubbleSize(const SizeF& bubbleSize)
    {
        bubbleSize_ = bubbleSize;
    }

    void SetBubbleOffset(const OffsetF& bubbleOffset)
    {
        bubbleOffset_ = bubbleOffset;
    }

    void SetTextOffset(const OffsetF& textOffset)
    {
        textOffset_ = textOffset;
    }

private:
    void PaintBezier(bool isLeft, Axis axis, RSPath& path, const OffsetF& arrowCenter, const OffsetF& arrowEdge);
    void SetBubbleDisplayAnimation();
    void SetBubbleDisappearAnimation();
    void BuildParagraph();
    void CreateParagraphAndLayout(
        const TextStyle& textStyle, const std::string& content);
    bool CreateParagraph(const TextStyle& textStyle, std::string content);
    OffsetF GetBubbleVertex();
    void UpdateBubbleSize();
    RectF UpdateOverlayRect();

private:
    RefPtr<PropertyBool> tipFlag_;
    RefPtr<PropertyOffsetF> contentOffset_;
    RefPtr<PropertySizeF> contentSize_;
    RefPtr<NG::Paragraph> paragraph_;

    RefPtr<AnimatablePropertyFloat> sizeScale_;
    RefPtr<AnimatablePropertyFloat> opacityScale_;
    RefPtr<PropertyString> content_;
    RefPtr<PropertyOffsetF> blockCenter_;

    SizeF bubbleSize_;
    OffsetF bubbleOffset_;
    OffsetF textOffset_;
    Axis axis_ = Axis::HORIZONTAL;
    Color tipColor_ = Color::BLACK;
    Color textColor_ = Color::TRANSPARENT;
    Dimension textFontSize_;
    std::function<OffsetF()> getBlockCenterFunc_;

    ACE_DISALLOW_COPY_AND_MOVE(SliderTipModifier);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SLIDER_TIP_MODIFIER_H
