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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_RENDER_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_RENDER_PROPERTY_H

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/vector.h"
#include "core/components/common/properties/clip_path.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/decoration.h"
#include "core/components/common/properties/shadow.h"
#include "core/components/common/properties/blend_mode.h"
#include "core/components_ng/property/border_property.h"
#include "core/components_ng/property/gradient_property.h"
#include "core/components_ng/property/overlay_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/property/transition_property.h"
#include "core/image/image_source_info.h"

namespace OHOS::Ace {
enum class BlurStyle;
} // namespace OHOS::Ace

namespace OHOS::Ace::NG {

struct BackgroundProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackgroundImage, ImageSourceInfo);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackgroundImageRepeat, ImageRepeat);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackgroundImageSize, BackgroundImageSize);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackgroundImagePosition, BackgroundImagePosition);
    bool CheckBlurStyleOption(const std::optional<BlurStyleOption>& option) const
    {
        if (!option.has_value()) {
            return false;
        }
        if (!propBlurStyleOption.has_value()) {
            return false;
        }
        return NearEqual(propBlurStyleOption.value(), option.value());
    }
    bool CheckBlurRadius(const Dimension& radius) const
    {
        if (!propBlurRadius.has_value()) {
            return false;
        }
        return NearEqual(propBlurRadius.value(), radius);
    }
    bool CheckEffectOption(const std::optional<EffectOption>& effectOption) const
    {
        if (!effectOption.has_value()) {
            return false;
        }
        if (!propEffectOption.has_value()) {
            return false;
        }
        return NearEqual(propEffectOption.value(), effectOption.value());
    }
    std::optional<BlurStyleOption> propBlurStyleOption;
    std::optional<Dimension> propBlurRadius;
    std::optional<EffectOption> propEffectOption;
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct CustomBackgroundProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackgroundPixelMap, RefPtr<PixelMap>);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackgroundAlign, Alignment);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct ForegroundProperty {
    bool CheckBlurStyleOption(const std::optional<BlurStyleOption>& option) const
    {
        if (!option.has_value()) {
            return false;
        }
        if (!propBlurStyleOption.has_value()) {
            return false;
        }
        return NearEqual(propBlurStyleOption.value(), option.value());
    }
    bool CheckBlurRadius(const Dimension& radius) const
    {
        if (!propBlurRadius.has_value()) {
            return false;
        }
        return NearEqual(propBlurRadius.value(), radius);
    }
    std::optional<BlurStyleOption> propBlurStyleOption;
    std::optional<Dimension> propBlurRadius;
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct BorderImageProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderImage, RefPtr<BorderImage>);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderImageSource, ImageSourceInfo);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(HasBorderImageSlice, bool);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(HasBorderImageWidth, bool);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(HasBorderImageOutset, bool);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(HasBorderImageRepeat, bool);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderImageGradient, Gradient);

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const
    {
        static const char* REPEAT_MODE[] = {
            "RepeatMode.Space",
            "RepeatMode.Stretch",
            "RepeatMode.Repeat",
            "RepeatMode.Round",
        };
        auto jsonBorderImage = JsonUtil::Create(true);
        jsonBorderImage->Put("source", propBorderImage.value_or(AceType::MakeRefPtr<BorderImage>())->GetSrc().c_str());
        jsonBorderImage->Put("slice", propBorderImage.value_or(AceType::MakeRefPtr<BorderImage>())
            ->GetBorderImageEdge(BorderImageDirection::LEFT).GetBorderImageSlice().ToString().c_str());
        jsonBorderImage->Put("width", propBorderImage.value_or(AceType::MakeRefPtr<BorderImage>())
            ->GetBorderImageEdge(BorderImageDirection::LEFT).GetBorderImageWidth().ToString().c_str());
        jsonBorderImage->Put("outset", propBorderImage.value_or(AceType::MakeRefPtr<BorderImage>())
            ->GetBorderImageEdge(BorderImageDirection::LEFT).GetBorderImageOutset().ToString().c_str());
        jsonBorderImage->Put("repeat", REPEAT_MODE[static_cast<int>(
            propBorderImage.value_or(AceType::MakeRefPtr<BorderImage>())->GetRepeatMode())]);
        jsonBorderImage->Put("fill", propBorderImage.value_or(AceType::MakeRefPtr<BorderImage>())
            ->GetNeedFillCenter() ? "true" : "false");
        json->Put("borderImage", jsonBorderImage->ToString().c_str());
    }
};

struct BorderProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderRadius, BorderRadiusProperty);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderColor, BorderColorProperty);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderStyle, BorderStyleProperty);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderWidth, BorderWidthProperty);

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct PointLightProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(LightPosition, TranslateOptions);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(LightIntensity, float);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(LightIlluminated, uint32_t);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Bloom, float);

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct TransformProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TransformScale, VectorF);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TransformCenter, DimensionOffset);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TransformTranslate, TranslateOptions);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TransformRotate, Vector5F);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct GraphicsProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontBrightness, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontGrayScale, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontContrast, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontSaturate, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontSepia, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontInvert, InvertVariant);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontHueRotate, float);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontColorBlend, Color);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontBlurRadius, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(LinearGradientBlur, NG::LinearGradientBlurPara);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(DynamicLightUpRate, float);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(DynamicLightUpDegree, float);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackShadow, Shadow);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackBlendMode, BlendMode);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct RenderPositionProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Position, OffsetT<Dimension>);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Offset, OffsetT<Dimension>);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Anchor, OffsetT<Dimension>);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct ClipProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ClipShape, RefPtr<BasicShape>);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ClipEdge, bool);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ClipMask, RefPtr<BasicShape>);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct GradientProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(LinearGradient, NG::Gradient);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(SweepGradient, NG::Gradient);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(RadialGradient, NG::Gradient);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct OverlayProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(OverlayText, OverlayOptions);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const
    {
        propOverlayText.value_or(OverlayOptions()).ToJsonValue(json);
    }
};

struct MotionPathProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(MotionPath, MotionPathOption);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_RENDER_PROPERTY_H
