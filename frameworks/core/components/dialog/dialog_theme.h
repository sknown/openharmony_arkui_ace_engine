/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_DIALOG_DIALOG_THEME_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_DIALOG_DIALOG_THEME_H

#include "base/utils/system_properties.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/edge.h"
#include "core/components/common/properties/radius.h"
#include "core/components/common/properties/text_style.h"
#include "core/components/theme/theme.h"
#include "core/components/theme/theme_constants.h"
#include "core/components/theme/theme_constants_defines.h"

namespace OHOS::Ace {
namespace {
constexpr double PRIMARY_RGBA_OPACITY = 0.9f;
constexpr double SECONDARY_RGBA_OPACITY = 0.6f;
} // namespace
/**
 * DialogTheme defines color and styles of DialogComponent. DialogTheme should be built
 * using DialogTheme::Builder.
 */
class DialogTheme : public virtual Theme {
    DECLARE_ACE_TYPE(DialogTheme, Theme);

public:
    class Builder {
    public:
        Builder() = default;
        ~Builder() = default;

        RefPtr<DialogTheme> Build(const RefPtr<ThemeConstants>& themeConstants) const
        {
            RefPtr<DialogTheme> theme = AceType::Claim(new DialogTheme());
            if (!themeConstants) {
                return theme;
            }
            // init theme from global data
            theme->backgroundColor_ = themeConstants->GetColor(THEME_DIALOG_BACKGROUND_COLOR);
            theme->titleTextStyle_.SetTextColor(themeConstants->GetColor(THEME_DIALOG_TITLE_TEXT_COLOR));
            theme->titleTextStyle_.SetFontSize(themeConstants->GetDimension(THEME_DIALOG_TITLE_TEXT_FONTSIZE));
            theme->titleTextStyle_.SetFontWeight(
                FontWeight(themeConstants->GetInt(THEME_DIALOG_TITLE_TEXT_FONTWEIGHT)));
            theme->subtitleTextStyle_.SetTextColor(themeConstants->GetColor(THEME_DIALOG_TITLE_TEXT_COLOR));
            theme->subtitleTextStyle_.SetFontSize(themeConstants->GetDimension(THEME_OHOS_TEXT_SIZE_SUBTITLE3));
            theme->titleMinFontSize_ = themeConstants->GetDimension(THEME_DIALOG_TITLE_TEXT_FONTSIZE_MIN);
            theme->contentMinFontSize_ = themeConstants->GetDimension(THEME_DIALOG_CONTENT_TEXT_FONTSIZE_MIN);
            auto titleMaxLines = themeConstants->GetInt(THEME_DIALOG_TITLE_TEXT_MAX_LINES);
            theme->titleMaxLines_ = titleMaxLines < 0 ? theme->titleMaxLines_ : static_cast<uint32_t>(titleMaxLines);
            theme->contentTextStyle_.SetTextColor(themeConstants->GetColor(THEME_DIALOG_CONTENT_TEXT_COLOR));
            theme->contentTextStyle_.SetFontSize(themeConstants->GetDimension(THEME_DIALOG_CONTENT_TEXT_FONTSIZE));
            theme->defaultPadding_ = Edge(themeConstants->GetDimension(THEME_DIALOG_PADDING_LEFT),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_TOP),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_RIGHT),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_BOTTOM));
            theme->adjustPadding_ = Edge(themeConstants->GetDimension(THEME_DIALOG_PADDING_LEFT_ADJUST),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_TOP_ADJUST),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_RIGHT_ADJUST),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_BOTTOM_ADJUST));
            theme->titleDefaultPadding_ = Edge(themeConstants->GetDimension(THEME_DIALOG_PADDING_LEFT),
                themeConstants->GetDimension(THEME_DIALOG_TITLE_PADDING_VERTICAL),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_RIGHT),
                themeConstants->GetDimension(THEME_DIALOG_TITLE_PADDING_VERTICAL));
            theme->titleAdjustPadding_ = Edge(themeConstants->GetDimension(THEME_DIALOG_PADDING_LEFT_ADJUST),
                themeConstants->GetDimension(THEME_DIALOG_TITLE_PADDING_VERTICAL),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_RIGHT_ADJUST),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_BOTTOM_ADJUST));
            theme->contentDefaultPadding_ = Edge(themeConstants->GetDimension(THEME_DIALOG_PADDING_LEFT),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_BOTTOM),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_RIGHT),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_BOTTOM));
            theme->contentAdjustPadding_ = Edge(themeConstants->GetDimension(THEME_DIALOG_PADDING_LEFT_ADJUST),
                themeConstants->GetDimension(THEME_DIALOG_TITLE_PADDING_VERTICAL),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_RIGHT_ADJUST),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_BOTTOM_ADJUST));
            theme->actionsPadding_ = Edge(themeConstants->GetDimension(THEME_DIALOG_PADDING_ACTIONS_LEFT),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_ACTIONS_TOP),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_ACTIONS_RIGHT),
                themeConstants->GetDimension(THEME_DIALOG_PADDING_ACTIONS_BOTTOM));
            theme->buttonPaddingLeft_ =
                Edge(themeConstants->GetDimension(THEME_DIALOG_PADDING_MIN).Value(), 0.0, 0.0, 0.0, DimensionUnit::VP);
            theme->buttonPaddingRight_ =
                Edge(0.0, 0.0, themeConstants->GetDimension(THEME_DIALOG_PADDING_MIN).Value(), 0.0, DimensionUnit::VP);
            theme->buttonPaddingCenter_ = Edge(0.0, themeConstants->GetDimension(THEME_DIALOG_PADDING_MID).Value(), 0.0,
                themeConstants->GetDimension(THEME_DIALOG_PADDING_MID).Value(), DimensionUnit::VP);
            theme->buttonSpacingHorizontal_ = themeConstants->GetDimension(THEME_DIALOG_BUTTON_SPACING_HORIZONTAL);
            theme->buttonSpacingVertical_ = themeConstants->GetDimension(THEME_DIALOG_BUTTON_SPACING_VERTICAL);
            theme->buttonBackgroundColor_ = themeConstants->GetColor(THEME_DIALOG_BUTTON_BG_COLOR);
            theme->buttonClickedColor_ = themeConstants->GetColor(THEME_DIALOG_BUTTON_CLICKED_COLOR);
            theme->frameStart_ = themeConstants->GetDouble(THEME_DIALOG_FRAME_START);
            theme->frameEnd_ = themeConstants->GetDouble(THEME_DIALOG_FRAME_END);
            theme->scaleStart_ = themeConstants->GetDouble(THEME_DIALOG_SCALE_START);
            theme->scaleEnd_ = themeConstants->GetDouble(THEME_DIALOG_SCALE_END);
            theme->opacityStart_ = themeConstants->GetDouble(THEME_DIALOG_OPACITY_START);
            theme->opacityEnd_ = themeConstants->GetDouble(THEME_DIALOG_OPACITY_END);
            theme->maskColorStart_ = themeConstants->GetColor(THEME_DIALOG_MASK_COLOR_START);
            theme->maskColorEnd_ = themeConstants->GetColor(THEME_DIALOG_MASK_COLOR_END);
            theme->animationDurationIn_ = themeConstants->GetInt(THEME_DIALOG_ANIMATION_DURATION_IN);
            theme->animationDurationOut_ = themeConstants->GetInt(THEME_DIALOG_ANIMATION_DURATION_OUT);
            theme->translateValue_ = Dimension(themeConstants->GetDouble(THEME_DIALOG_TRANSLATE), DimensionUnit::PX);
            theme->dividerColor_ = themeConstants->GetColor(THEME_DIALOG_DIVIDER_COLOR);
            theme->dividerWidth_ = themeConstants->GetDimension(THEME_DIALOG_DIVIDER_WIDTH);
            theme->dividerHeight_ = themeConstants->GetDimension(THEME_DIALOG_DIVIDER_HEIGHT);
            theme->dividerPadding_ = Edge(themeConstants->GetDimension(THEME_DIALOG_DIVIDER_PADDING_HORIZON),
                themeConstants->GetDimension(THEME_DIALOG_DIVIDER_PADDING_VERTICAL),
                themeConstants->GetDimension(THEME_DIALOG_DIVIDER_PADDING_HORIZON),
                themeConstants->GetDimension(THEME_DIALOG_DIVIDER_PADDING_VERTICAL));
            theme->marginBottom_ = themeConstants->GetDimension(THEME_OHOS_DIMENS_DIALOG_BOTTOM);
            theme->marginLeft_ = themeConstants->GetDimension(THEME_OHOS_DIMENS_DIALOG_START);
            theme->marginRight_ = themeConstants->GetDimension(THEME_OHOS_DIMENS_DIALOG_END);
            ParsePattern(themeConstants->GetThemeStyle(), theme);
            return theme;
        }

        void ParsePattern(const RefPtr<ThemeStyle>& themeStyle, const RefPtr<DialogTheme>& theme) const
        {
            if (!themeStyle) {
                LOGI("progress theme style is null");
                return;
            }
            auto dialogPattern = themeStyle->GetAttr<RefPtr<ThemeStyle>>(THEME_PATTERN_DIALOG, nullptr);
            if (!dialogPattern) {
                return;
            }
            theme->backgroundColor_ = dialogPattern->GetAttr<Color>(PATTERN_BG_COLOR, Color(0xd9ffffff));
            auto textColor = dialogPattern->GetAttr<Color>("title_text_color", Color::BLACK);
            auto textOpacity = dialogPattern->GetAttr<double>("attribute_alpha_content_primary", PRIMARY_RGBA_OPACITY);
            theme->titleTextStyle_.SetTextColor(textColor.BlendOpacity(textOpacity));
            theme->titleTextStyle_.SetFontSize(dialogPattern->GetAttr<Dimension>("title_text_font_size", 20.0_fp));
            textOpacity = dialogPattern->GetAttr<double>("attribute_alpha_content_secondary", SECONDARY_RGBA_OPACITY);
            theme->subtitleTextStyle_.SetTextColor(textColor.BlendOpacity(textOpacity));
            theme->subtitleTextStyle_.SetFontSize(
                dialogPattern->GetAttr<Dimension>("subtitle_text_font_size", 14.0_fp));
            theme->contentTextStyle_.SetTextColor(dialogPattern->GetAttr<Color>("content_text_color", Color::BLACK));
            theme->contentTextStyle_.SetFontSize(dialogPattern->GetAttr<Dimension>("content_text_font_size", 16.0_fp));
            theme->buttonBackgroundColor_ = dialogPattern->GetAttr<Color>("button_bg_color", Color::BLACK);
            theme->radius_ = Radius(dialogPattern->GetAttr<Dimension>("radius", 24.0_vp));
            theme->dividerLength_ = dialogPattern->GetAttr<Dimension>(DIALOG_DIVIDER_LENGTH, 24.0_vp);
            theme->dividerBetweenButtonWidth_ =
                dialogPattern->GetAttr<Dimension>(DIALOG_DIVIDER_BETWEEN_BUTTON_WIDTH, 2.0_px);

            auto defaultPadding = dialogPattern->GetAttr<Dimension>(DIALOG_CONTENT_TOP_PADDING, 24.0_vp);
            theme->contentAdjustPadding_ = Edge(defaultPadding, defaultPadding, defaultPadding, 0.0_vp);
            theme->defaultPaddingBottomFixed_ =
                dialogPattern->GetAttr<Dimension>("default_padding_bottom_fixed", 24.0_vp);
            theme->buttonHighlightBgColor_ =
                dialogPattern->GetAttr<Color>("button_bg_highlight_color", Color(0xff007dff));
            theme->buttonHighlightFontColor_ = dialogPattern->GetAttr<Color>("first_button_text_color", Color::WHITE);
            theme->buttonDefaultBgColor_ = dialogPattern->GetAttr<Color>("button_default_bg_color", Color::TRANSPARENT);
            theme->buttonDefaultFontColor_ =
                dialogPattern->GetAttr<Color>("button_default_font_color", Color(0xff007dff));

            if (SystemProperties::GetDeviceType() != DeviceType::CAR) {
                return;
            }
            auto titlePadding = dialogPattern->GetAttr<Dimension>(DIALOG_TITLE_TOP_PADDING, 0.0_vp);
            auto actionsTopPadding = dialogPattern->GetAttr<Dimension>(DIALOG_ACTIONS_TOP_PADDING, 0.0_vp);
            theme->titleAdjustPadding_ = Edge(defaultPadding, titlePadding, defaultPadding, titlePadding);
            theme->titleDefaultPadding_ = Edge(defaultPadding, titlePadding, defaultPadding, titlePadding);
            theme->defaultPadding_ = Edge(defaultPadding, defaultPadding, defaultPadding, defaultPadding);
            theme->adjustPadding_ = Edge(defaultPadding, defaultPadding, defaultPadding, 0.0_vp);
            theme->contentDefaultPadding_ = Edge(defaultPadding, 0.0_vp, defaultPadding, defaultPadding);
            theme->contentAdjustPadding_ = Edge(defaultPadding, 0.0_vp, defaultPadding, 0.0_vp);
            theme->actionsPadding_ = Edge(defaultPadding, actionsTopPadding, defaultPadding, actionsTopPadding);
            theme->buttonHeight_ = dialogPattern->GetAttr<Dimension>(DIALOG_BUTTON_HEIGHT, 0.0_vp);
            theme->titleMaxLines_ = static_cast<uint32_t>(dialogPattern->GetAttr<int32_t>(DIALOG_TITLE_MAX_LINES, 2));
            theme->buttonSpacingHorizontal_ = actionsTopPadding;
            theme->commonButtonTextColor_ =
                dialogPattern->GetAttr<Color>(DIALOG_COMMON_BUTTON_TEXT_COLOR, Color::WHITE);
            theme->buttonMinTextSize_ = dialogPattern->GetAttr<Dimension>(DIALOG_MIN_BUTTON_TEXT_SIZE, 10.0_vp);
            theme->minButtonWidth_ = dialogPattern->GetAttr<Dimension>(DIALOG_MIN_BUTTON_WIDTH, 104.0_vp);
            theme->maxButtonWidth_ = dialogPattern->GetAttr<Dimension>(DIALOG_MAX_BUTTON_WIDTH, 260.0_vp);
            theme->maskColorEnd_ = dialogPattern->GetAttr<Color>(DIALOG_MASK_COLOR_END, Color::WHITE);
            // pattern config
            theme->titleTextStyle_.SetFontSize(dialogPattern->GetAttr<Dimension>("title_text_font_size", 20.0_vp));
            theme->titleMinFontSize_ = dialogPattern->GetAttr<Dimension>("title_text_font_size_min", 20.0_vp);
            theme->commonButtonBgColor_ = dialogPattern->GetAttr<Color>("common_button_bg_color", Color::GRAY);
            theme->emphasizeButtonBgColor_ = dialogPattern->GetAttr<Color>("first_button_bg_color", Color::BLACK);
            theme->emphasizeButtonTextColor_ = dialogPattern->GetAttr<Color>("first_button_text_color", Color::WHITE);
            theme->buttonTextSize_ = dialogPattern->GetAttr<Dimension>("button_text_font_size", 16.0_vp);
            theme->buttonClickedColor_ = dialogPattern->GetAttr<Color>("button_bg_color_clicked", Color::BLACK);
            theme->contentTextStyle_.SetFontSize(themeStyle->GetAttr<Dimension>("content_text_font_size", 16.0_vp));
            theme->contentMinFontSize_ = themeStyle->GetAttr<Dimension>("content_text_font_size_min", 16.0_vp);
        }
    };

    ~DialogTheme() override = default;

    const Radius& GetRadius() const
    {
        return radius_;
    }

    const Color& GetBackgroundColor() const
    {
        return backgroundColor_;
    }

    const Color& GetCommonButtonBgColor() const
    {
        return commonButtonBgColor_;
    }

    const Color& GetEmphasizeButtonBgColor() const
    {
        return emphasizeButtonBgColor_;
    }

    const TextStyle& GetTitleTextStyle() const
    {
        return titleTextStyle_;
    }

    const TextStyle& GetSubTitleTextStyle() const
    {
        return subtitleTextStyle_;
    }

    const Dimension& GetTitleMinFontSize() const
    {
        return titleMinFontSize_;
    }

    const Dimension& GetContentMinFontSize() const
    {
        return contentMinFontSize_;
    }

    uint32_t GetTitleMaxLines() const
    {
        return titleMaxLines_;
    }

    const TextStyle& GetContentTextStyle() const
    {
        return contentTextStyle_;
    }

    const Edge& GetDefaultPadding() const
    {
        return defaultPadding_;
    }

    const Edge& GetAdjustPadding() const
    {
        return adjustPadding_;
    }

    const Edge& GetTitleDefaultPadding() const
    {
        return titleDefaultPadding_;
    }

    const Edge& GetTitleAdjustPadding() const
    {
        return titleAdjustPadding_;
    }

    const Edge& GetContentDefaultPadding() const
    {
        return contentDefaultPadding_;
    }

    const Edge& GetContentAdjustPadding() const
    {
        return contentAdjustPadding_;
    }

    const Edge& GetActionsPadding() const
    {
        return actionsPadding_;
    }

    const Edge& GetButtonPaddingLeft() const
    {
        return buttonPaddingLeft_;
    }

    const Edge& GetButtonPaddingRight() const
    {
        return buttonPaddingRight_;
    }

    const Edge& GetButtonPaddingCenter() const
    {
        return buttonPaddingCenter_;
    }

    const Dimension& GetButtonSpacingHorizontal() const
    {
        return buttonSpacingHorizontal_;
    }

    const Dimension& GetButtonSpacingVertical() const
    {
        return buttonSpacingVertical_;
    }

    const Dimension& GetDividerLength() const
    {
        return dividerLength_;
    }

    const Dimension& GetDividerBetweenButtonWidth_() const
    {
        return dividerBetweenButtonWidth_;
    }

    const Color& GetButtonBackgroundColor() const
    {
        return buttonBackgroundColor_;
    }

    const Color& GetButtonClickedColor() const
    {
        return buttonClickedColor_;
    }

    const Color& GetButtonHighlightBgColor() const
    {
        return buttonHighlightBgColor_;
    }

    const Color& GetButtonHighlightFontColor() const
    {
        return buttonHighlightFontColor_;
    }

    const Color& GetButtonDefaultBgColor() const
    {
        return buttonDefaultBgColor_;
    }

    const Color& GetButtonDefaultFontColor() const
    {
        return buttonDefaultFontColor_;
    }

    double GetFrameStart() const
    {
        return frameStart_;
    }

    double GetFrameEnd() const
    {
        return frameEnd_;
    }

    double GetScaleStart() const
    {
        return scaleStart_;
    }

    double GetScaleEnd() const
    {
        return scaleEnd_;
    }

    double GetOpacityStart() const
    {
        return opacityStart_;
    }

    double GetOpacityEnd() const
    {
        return opacityEnd_;
    }

    const Dimension& GetTranslateValue() const
    {
        return translateValue_;
    }

    const Color& GetMaskColorStart() const
    {
        return maskColorStart_;
    }

    const Color& GetMaskColorEnd() const
    {
        return maskColorEnd_;
    }

    const Color& GetCommonButtonTextColor() const
    {
        return commonButtonTextColor_;
    }

    const Color& GetEmphasizeButtonTextColor() const
    {
        return emphasizeButtonTextColor_;
    }

    int32_t GetOpacityAnimationDurIn() const
    {
        return opacityAnimationDurIn_;
    }

    int32_t GetAnimationDurationIn() const
    {
        return animationDurationIn_;
    }

    int32_t GetAnimationDurationOut() const
    {
        return animationDurationOut_;
    }

    const Color& GetDividerColor()
    {
        return dividerColor_;
    }

    const Dimension& GetDividerWidth()
    {
        return dividerWidth_;
    }

    const Dimension& GetDividerHeight()
    {
        return dividerHeight_;
    }

    const Edge& GetDividerPadding()
    {
        return dividerPadding_;
    }

    const Dimension& GetMarginBottom() const
    {
        return marginBottom_;
    }

    const Dimension& GetMarginLeft() const
    {
        return marginLeft_;
    }

    const Dimension& GetMarginRight() const
    {
        return marginRight_;
    }

    const Dimension& GetButtonHeight() const
    {
        return buttonHeight_;
    }

    const Dimension& GetButtonTextSize() const
    {
        return buttonTextSize_;
    }

    const Dimension& GetMinButtonTextSize() const
    {
        return buttonMinTextSize_;
    }

    const Dimension& GetDefaultPaddingBottomFixed()
    {
        return defaultPaddingBottomFixed_;
    }

protected:
    DialogTheme() = default;

private:
    Radius radius_;
    Color backgroundColor_;
    TextStyle titleTextStyle_;
    TextStyle subtitleTextStyle_;
    TextStyle contentTextStyle_;
    Dimension titleMinFontSize_;
    Dimension contentMinFontSize_;
    uint32_t titleMaxLines_ = 1;
    Edge defaultPadding_;
    Edge adjustPadding_;
    Edge titleDefaultPadding_;
    Edge titleAdjustPadding_;
    Edge contentDefaultPadding_;
    Edge contentAdjustPadding_;
    Edge actionsPadding_;
    Edge buttonPaddingLeft_;
    Edge buttonPaddingRight_;
    Edge buttonPaddingCenter_;
    Dimension buttonSpacingHorizontal_;
    Dimension buttonSpacingVertical_;
    Dimension dividerLength_;
    Dimension dividerBetweenButtonWidth_;
    Color buttonBackgroundColor_;
    Color buttonClickedColor_;
    Color buttonHighlightBgColor_;
    Color buttonHighlightFontColor_;
    Color buttonDefaultBgColor_;
    Color buttonDefaultFontColor_;
    Color emphasizeButtonTextColor_;
    Dimension translateValue_;
    double frameStart_ = 0.0;
    double frameEnd_ = 1.0;
    double scaleStart_ = 0.0;
    double scaleEnd_ = 1.0;
    double opacityStart_ = 0.0;
    double opacityEnd_ = 1.0;
    int32_t animationDurationIn_ = 250;
    int32_t opacityAnimationDurIn_ = 150;
    int32_t animationDurationOut_ = 250;
    Color maskColorStart_;
    Color maskColorEnd_;
    Color dividerColor_;
    Color commonButtonBgColor_;
    Color commonButtonTextColor_;
    Color emphasizeButtonBgColor_;
    Dimension dividerWidth_;
    Dimension dividerHeight_;
    Edge dividerPadding_;
    Dimension marginLeft_;
    Dimension marginRight_;
    Dimension marginBottom_;
    Dimension buttonHeight_;
    Dimension buttonTextSize_;
    Dimension buttonMinTextSize_;
    Dimension minButtonWidth_;
    Dimension maxButtonWidth_;
    Dimension defaultPaddingBottomFixed_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_DIALOG_DIALOG_THEME_H
