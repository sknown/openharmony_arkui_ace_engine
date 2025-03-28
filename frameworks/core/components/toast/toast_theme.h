/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TOAST_TOAST_THEME_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TOAST_TOAST_THEME_H

#include "core/common/container.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/edge.h"
#include "core/components/common/properties/text_style.h"
#include "core/components/theme/theme.h"
#include "core/components/theme/theme_constants.h"
#include "core/components/theme/theme_constants_defines.h"

namespace OHOS::Ace {

/**
 * ToastTheme defines color and styles of Toast. ToastTheme should be built
 * using ToastTheme::Builder.
 */
class ToastTheme : public virtual Theme {
    DECLARE_ACE_TYPE(ToastTheme, Theme);

public:
    class Builder {
    public:
        Builder() = default;
        ~Builder() = default;

        RefPtr<ToastTheme> Build(const RefPtr<ThemeConstants>& themeConstants) const
        {
            RefPtr<ToastTheme> theme = AceType::Claim(new ToastTheme());
            if (!themeConstants) {
                return theme;
            }
            ParsePattern(themeConstants, theme);
            return theme;
        }
    private:
        void ParsePattern(const RefPtr<ThemeConstants>& themeConstants, const RefPtr<ToastTheme>& theme) const
        {
            RefPtr<ThemeStyle> toastPattern = themeConstants->GetPatternByName(THEME_PATTERN_TOAST);
            if (!toastPattern) {
                return;
            }

            theme->minWidth_ = toastPattern->GetAttr<Dimension>("toast_content_min_width", 0.0_vp);
            theme->minHeight_ = toastPattern->GetAttr<Dimension>("toast_content_min_height", 0.0_vp);
            theme->bottom_ = toastPattern->GetAttr<Dimension>("toast_bottom", 0.0_vp);
            theme->minFontSize_ = toastPattern->GetAttr<Dimension>("toast_text_min_font_size", 0.0_vp);
            auto textMaxLines = static_cast<int32_t>(toastPattern->GetAttr<double>("toast_text_max_lines", 0.0));
            theme->textMaxLines_ = textMaxLines < 0 ? theme->textMaxLines_ : static_cast<uint32_t>(textMaxLines);
            theme->backgroundColor_ = toastPattern->GetAttr<Color>(PATTERN_BG_COLOR, Color());
            theme->blurStyleTextColor_ = toastPattern->GetAttr<Color>(PATTERN_TEXT_COLOR_BLUR, Color());

            if (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
                theme->padding_ = Edge(toastPattern->GetAttr<Dimension>("toast_padding_level8", 0.0_vp).Value(),
                    toastPattern->GetAttr<Dimension>("toast_padding_level4", 0.0_vp).Value(),
                    toastPattern->GetAttr<Dimension>("toast_padding_level8", 0.0_vp).Value(),
                    toastPattern->GetAttr<Dimension>("toast_padding_level4", 0.0_vp).Value(),
                    toastPattern->GetAttr<Dimension>("toast_padding_level4", 0.0_vp).Unit());
                theme->marging_ = Edge(toastPattern->GetAttr<Dimension>("toast_margin_left", 0.0_vp).Value(), 0.0,
                    toastPattern->GetAttr<Dimension>("toast_margin_right", 0.0_vp).Value(), 0.0,
                    toastPattern->GetAttr<Dimension>("toast_margin_right", 0.0_vp).Unit());
                theme->maxWidth_ = toastPattern->GetAttr<Dimension>("toast_content_limit_max_width", 400.0_vp);
                theme->textStyle_.SetTextColor(toastPattern->GetAttr<Color>("toast_font_primary", Color()));
                theme->textStyle_.SetFontSize(toastPattern->GetAttr<Dimension>("toast_Body_M", 0.0_vp));
                theme->textStyle_.SetFontWeight(
                    FontWeight(static_cast<int32_t>(toastPattern->GetAttr<double>("toast_font_weight_regular", 0.0))));
                theme->radius_ = Radius(toastPattern->GetAttr<Dimension>("toast_border_radius_level9", 24.0_vp),
                    toastPattern->GetAttr<Dimension>("toast_border_radius_level9", 24.0_vp));
            } else {
                theme->padding_ = Edge(toastPattern->GetAttr<Dimension>("toast_padding_horizontal", 0.0_vp).Value(),
                    toastPattern->GetAttr<Dimension>("toast_padding_vertical", 0.0_vp).Value(),
                    toastPattern->GetAttr<Dimension>("toast_padding_horizontal", 0.0_vp).Value(),
                    toastPattern->GetAttr<Dimension>("toast_padding_vertical", 0.0_vp).Value(),
                    toastPattern->GetAttr<Dimension>("toast_padding_vertical", 0.0_vp).Unit());
                theme->marging_ = Edge(toastPattern->GetAttr<Dimension>("toast_margin", 0.0_vp).Value(), 0.0,
                    toastPattern->GetAttr<Dimension>("toast_margin", 0.0_vp).Value(), 0.0,
                    toastPattern->GetAttr<Dimension>("toast_margin", 0.0_vp).Unit());
                theme->maxWidth_ = toastPattern->GetAttr<Dimension>("toast_content_max_width", 0.0_vp);
                theme->textStyle_.SetTextColor(toastPattern->GetAttr<Color>(PATTERN_TEXT_COLOR, Color()));
                theme->textStyle_.SetFontSize(toastPattern->GetAttr<Dimension>(PATTERN_TEXT_SIZE, 0.0_vp));
                theme->textStyle_.SetFontWeight(
                    FontWeight(static_cast<int32_t>(toastPattern->GetAttr<double>("toast_text_font_weight", 0.0))));
                theme->radius_ = Radius(toastPattern->GetAttr<Dimension>("toast_border_radius", 24.0_vp),
                    toastPattern->GetAttr<Dimension>("toast_border_radius", 24.0_vp));
            }
        }
    };

    ~ToastTheme() override = default;

    const Edge& GetPadding() const
    {
        return padding_;
    }

    const Dimension& GetMaxWidth() const
    {
        return maxWidth_;
    }

    const Dimension& GetMinWidth() const
    {
        return minWidth_;
    }

    const Dimension& GetMinHeight() const
    {
        return minHeight_;
    }

    const Color& GetBackgroundColor() const
    {
        return backgroundColor_;
    }

    const TextStyle& GetTextStyle() const
    {
        return textStyle_;
    }

    const Radius& GetRadius() const
    {
        return radius_;
    }

    const Dimension& GetBottom() const
    {
        return bottom_;
    }

    const Dimension& GetMinFontSize() const
    {
        return minFontSize_;
    }

    uint32_t GetTextMaxLines() const
    {
        return textMaxLines_;
    }

    const Edge& GetMarging() const
    {
        return marging_;
    }

    const Color& GetBlurStyleTextColor() const
    {
        return blurStyleTextColor_;
    }

protected:
    ToastTheme() = default;

private:
    Edge padding_;
    Dimension maxWidth_;
    Dimension minWidth_;
    Dimension minHeight_;
    Color backgroundColor_;
    TextStyle textStyle_;
    Radius radius_;
    Dimension bottom_;
    Dimension minFontSize_;
    uint32_t textMaxLines_ = 1;
    Edge marging_;
    Color blurStyleTextColor_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TOAST_TOAST_THEME_H
