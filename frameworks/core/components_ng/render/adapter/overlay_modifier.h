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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_OVERLAY_MODIFIER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_OVERLAY_MODIFIER_H

#include <memory>
#include <string>

#include "modifier/rs_animatable_arithmetic.h"
#include "render_service_client/core/modifier/rs_extended_modifier.h"
#include "render_service_client/core/modifier/rs_property.h"

#include "base/geometry/ng/offset_t.h"
#include "base/i18n/localization.h"
#include "base/memory/referenced.h"
#include "core/components/common/properties/alignment.h"
#include "core/components_ng/property/overlay_property.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/components_ng/render/font_collection.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {

class OverlayTextData : public Rosen::RSArithmetic<OverlayTextData> {
public:
    OverlayTextData() = default;
    explicit OverlayTextData(const OverlayOptions& overlay) : overlay_(overlay) {}
    ~OverlayTextData() override = default;

    bool IsEqual(const OverlayTextData& other) const override
    {
        return overlay_ == other.overlay_;
    }

    OverlayOptions GetOverlayOptions()
    {
        return overlay_;
    }

private:
    OverlayOptions overlay_;
};

class OverlayTextModifier : public Rosen::RSOverlayStyleModifier {
public:
    OverlayTextModifier() = default;
    ~OverlayTextModifier() override = default;

    void Draw(Rosen::RSDrawingContext& context) const override
    {
        if (property_) {
            const Dimension fontSize = Dimension(40);
            auto overlayOptions = property_->Get().GetOverlayOptions();
            // create paragraph
            TextStyle textStyle;
            textStyle.SetFontSize(fontSize);
            RSParagraphStyle paraStyle;
            paraStyle.textAlign_ = ToRSTextAlign(textStyle.GetTextAlign());
            paraStyle.maxLines_ = textStyle.GetMaxLines();
            paraStyle.locale_ = Localization::GetInstance()->GetFontLocale();
            paraStyle.wordBreakType_ = ToRSWordBreakType(textStyle.GetWordBreak());
            paraStyle.fontSize_ = fontSize.Value();
            auto builder = RSParagraphBuilder::CreateRosenBuilder(paraStyle, RSFontCollection::GetInstance(false));
            CHECK_NULL_VOID(builder);
            auto pipelineContext = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID(pipelineContext);
            builder->PushStyle(ToRSTextStyle(pipelineContext, textStyle));
            builder->AddText(StringUtils::Str8ToStr16(overlayOptions.content));
            builder->Pop();
            auto paragraph = builder->Build();
            CHECK_NULL_VOID(paragraph);
            paragraph->Layout(context.width);
            OffsetF offset = OverlayTextModifier::GetTextPosition(SizeF(context.width, context.height),
                SizeF(paragraph->GetMinIntrinsicWidth(), paragraph->GetHeight()), overlayOptions);
            std::shared_ptr<SkCanvas> skCanvas { context.canvas, [](SkCanvas*) {} };
            RSCanvas canvas(&skCanvas);
            CHECK_NULL_VOID(&canvas);
            paragraph->Paint(&canvas, offset.GetX(), offset.GetY());
        } else {
            LOGE("property is null");
        }
    }

    void SetCustomData(const OverlayTextData& data)
    {
        if (!property_) {
            property_ = std::make_shared<Rosen::RSProperty<OverlayTextData>>(data);
            AttachProperty(property_);
        } else {
            property_->Set(data);
        }
    }

private:
    static OffsetF GetTextPosition(const SizeF& parentSize, const SizeF& childSize, OverlayOptions& overlay)
    {
        const double dx = overlay.x.ConvertToPx();
        const double dy = overlay.y.ConvertToPx();
        const Alignment align = overlay.align;
        OffsetF const offset = Alignment::GetAlignPosition(parentSize, childSize, align);
        const float fx = static_cast<float>(dx) + offset.GetX();
        const float fy = static_cast<float>(dy) + offset.GetY();
        return { fx, fy };
    }

    std::shared_ptr<Rosen::RSProperty<OverlayTextData>> property_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_OVERLAY_MODIFIER_H