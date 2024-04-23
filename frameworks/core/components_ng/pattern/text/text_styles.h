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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_TEXT_STYLES_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_TEXT_STYLES_H

#include "core/components/common/properties/text_style.h"
#include "core/components/text/text_theme.h"
#include "core/components_ng/pattern/symbol/symbol_effect_options.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/paragraph.h"
#include "core/components_v2/inspector/utils.h"

namespace OHOS::Ace {
struct UserGestureOptions {
    GestureEventFunc onClick;
    GestureEventFunc onLongPress;
};

struct ImageSpanSize {
    CalcDimension width;
    CalcDimension height;

    bool operator==(const ImageSpanSize& other) const
    {
        return width == other.width && height == other.height;
    }

    std::string ToString() const
    {
        auto jsonValue = JsonUtil::Create(true);
        JSON_STRING_PUT_STRINGABLE(jsonValue, width);
        JSON_STRING_PUT_STRINGABLE(jsonValue, height);
        return jsonValue->ToString();
    }
};

struct ImageSpanAttribute {
    std::optional<ImageSpanSize> size;
    std::optional<VerticalAlign> verticalAlign;
    std::optional<ImageFit> objectFit;
    std::optional<OHOS::Ace::NG::MarginProperty> marginProp;
    std::optional<OHOS::Ace::NG::BorderRadiusProperty> borderRadius;
    std::optional<OHOS::Ace::NG::PaddingProperty> paddingProp;

    bool operator==(const ImageSpanAttribute& attribute) const
    {
        return size == attribute.size && verticalAlign == attribute.verticalAlign && objectFit == attribute.objectFit &&
               marginProp == attribute.marginProp && borderRadius == attribute.borderRadius &&
               paddingProp == attribute.paddingProp;
    }

    std::string ToString() const
    {
        auto jsonValue = JsonUtil::Create(true);
        JSON_STRING_PUT_OPTIONAL_STRINGABLE(jsonValue, size);
        JSON_STRING_PUT_OPTIONAL_INT(jsonValue, verticalAlign);
        JSON_STRING_PUT_OPTIONAL_INT(jsonValue, objectFit);
        JSON_STRING_PUT_OPTIONAL_STRINGABLE(jsonValue, marginProp);
        JSON_STRING_PUT_OPTIONAL_STRINGABLE(jsonValue, borderRadius);
        return jsonValue->ToString();
    }
};

struct SpanOptionBase {
    std::optional<int32_t> offset;
    UserGestureOptions userGestureOption;

    std::string ToString() const
    {
        auto jsonValue = JsonUtil::Create(true);
        JSON_STRING_PUT_OPTIONAL_INT(jsonValue, offset);
        return jsonValue->ToString();
    }
};

struct ImageSpanOptions : SpanOptionBase {
    std::optional<int32_t> offset;
    std::optional<std::string> image;
    std::optional<std::string> bundleName;
    std::optional<std::string> moduleName;
    std::optional<RefPtr<PixelMap>> imagePixelMap;
    std::optional<ImageSpanAttribute> imageAttribute;

    std::string ToString() const
    {
        auto jsonValue = JsonUtil::Create(true);
        JSON_STRING_PUT_OPTIONAL_INT(jsonValue, offset);
        JSON_STRING_PUT_OPTIONAL_STRING(jsonValue, image);
        JSON_STRING_PUT_OPTIONAL_STRING(jsonValue, bundleName);
        JSON_STRING_PUT_OPTIONAL_STRING(jsonValue, moduleName);
        JSON_STRING_PUT_OPTIONAL_STRING(jsonValue, image);
        if (imagePixelMap && *imagePixelMap) {
            std::string pixSize = "[";
            pixSize += std::to_string((*imagePixelMap)->GetWidth());
            pixSize += "*";
            pixSize += std::to_string((*imagePixelMap)->GetHeight());
            pixSize += "]";
            jsonValue->Put("pixelMapSize", pixSize.c_str());
        }
        JSON_STRING_PUT_OPTIONAL_STRINGABLE(jsonValue, imageAttribute);
        return jsonValue->ToString();
    }
};
} // namespace OHOS::Ace
namespace OHOS::Ace::NG {
constexpr Dimension TEXT_DEFAULT_FONT_SIZE = 16.0_fp;
using FONT_FEATURES_LIST = std::list<std::pair<std::string, int32_t>>;
struct FontStyle {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FontSize, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TextColor, Color);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TextShadow, std::vector<Shadow>);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ItalicFontStyle, Ace::FontStyle);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FontWeight, FontWeight);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FontFamily, std::vector<std::string>);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FontFeature, FONT_FEATURES_LIST);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TextDecoration, TextDecoration);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TextDecorationColor, Color);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TextDecorationStyle, TextDecorationStyle);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TextCase, TextCase);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(AdaptMinFontSize, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(AdaptMaxFontSize, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(LetterSpacing, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ForegroundColor, Color);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(SymbolColorList, std::vector<Color>);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(SymbolRenderingStrategy, uint32_t);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(SymbolEffectStrategy, uint32_t);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(SymbolEffectOptions, SymbolEffectOptions);
};

struct TextLineStyle {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(LineHeight, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TextBaseline, TextBaseline);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BaselineOffset, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TextOverflow, TextOverflow);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TextAlign, TextAlign);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(MaxLength, uint32_t);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(MaxLines, uint32_t);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(HeightAdaptivePolicy, TextHeightAdaptivePolicy);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TextIndent, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(LeadingMargin, LeadingMargin);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(WordBreak, WordBreak);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(EllipsisMode, EllipsisMode);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(LineSpacing, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(LineBreakStrategy, LineBreakStrategy);
};

struct HandleInfoNG {
    void UpdateOffset(const OffsetF& offset)
    {
        rect.SetOffset(offset);
    }

    void AddOffset(float x, float y)
    {
        auto offset = rect.GetOffset();
        offset.AddX(x);
        offset.AddY(y);
        UpdateOffset(offset);
    }

    bool operator==(const HandleInfoNG& handleInfo) const
    {

        return rect == handleInfo.rect && index == handleInfo.index;
    }

    bool operator!=(const HandleInfoNG& handleInfo) const
    {
        return !operator==(handleInfo);
    }

    int32_t index = 0;
    RectF rect;
};

TextStyle CreateTextStyleUsingTheme(const std::unique_ptr<FontStyle>& fontStyle,
    const std::unique_ptr<TextLineStyle>& textLineStyle, const RefPtr<TextTheme>& textTheme);

TextStyle CreateTextStyleUsingThemeWithText(const RefPtr<FrameNode> frameNode,
    const std::unique_ptr<FontStyle>& fontStyle, const std::unique_ptr<TextLineStyle>& textLineStyle,
    const RefPtr<TextTheme>& textTheme);

void UseSelfStyle(const std::unique_ptr<FontStyle>& fontStyle,
    const std::unique_ptr<TextLineStyle>& textLineStyle, TextStyle& textStyle);

std::string GetFontFamilyInJson(const std::optional<std::vector<std::string>>& value);
std::string GetFontStyleInJson(const std::optional<Ace::FontStyle>& value);
std::string GetFontWeightInJson(const std::optional<FontWeight>& value);
std::string GetFontSizeInJson(const std::optional<Dimension>& value);
std::string GetSymbolRenderingStrategyInJson(const std::optional<uint32_t>& value);
std::string GetSymbolEffectStrategyInJson(const std::optional<uint32_t>& value);
std::string GetLineBreakStrategyInJson(const std::optional<Ace::LineBreakStrategy>& value);
std::string GetSymbolEffectOptionsInJson(const std::optional<SymbolEffectOptions>& value);
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_TEXT_STYLES_H
