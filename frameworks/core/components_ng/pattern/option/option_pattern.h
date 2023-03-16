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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OPTION_OPTION_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OPTION_OPTION_PATTERN_H

#include <optional>

#include "core/components/select/select_theme.h"
#include "core/components/text/text_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/option/option_accessibility_property.h"
#include "core/components_ng/pattern/option/option_event_hub.h"
#include "core/components_ng/pattern/option/option_layout_algorithm.h"
#include "core/components_ng/pattern/option/option_paint_method.h"
#include "core/components_ng/pattern/option/option_paint_property.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/render/paint_property.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {
class OptionPattern : public Pattern {
    DECLARE_ACE_TYPE(OptionPattern, Pattern);

public:
    explicit OptionPattern(int index) : index_(index) {}
    ~OptionPattern() override = default;

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        return MakeRefPtr<OptionPaintMethod>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<OptionPaintProperty>();
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<OptionEventHub>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<OptionAccessibilityProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<OptionLayoutAlgorithm>();
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    void SetTextNode(const RefPtr<FrameNode>& text)
    {
        text_ = text;
    }

    void SetIconNode(const RefPtr<FrameNode>& icon)
    {
        icon_ = icon;
    }

    void SetBgColor(const Color& color);
    // set font props
    void SetFontSize(const Dimension& value);
    void SetItalicFontStyle(const Ace::FontStyle& value);
    void SetFontWeight(const FontWeight& value);
    void SetFontFamily(const std::vector<std::string>& value);
    void SetFontColor(const Color& color);

    Color GetBgColor();
    // get font props
    Dimension GetFontSize();
    Ace::FontStyle GetItalicFontStyle();
    FontWeight GetFontWeight();
    std::vector<std::string> GetFontFamily();
    Color GetFontColor();

    const std::string& GetText();

    // XTS inspector functions
    std::string InspectorGetFont();

    void SetIcon(const std::string& src)
    {
        iconSrc_ = src;
    }

    const std::string& GetIcon()
    {
        return iconSrc_;
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, true, FocusStyleType::INNER_BORDER };
    }

    void UpdateNextNodeDivider(bool needDivider);

    void SetBgBlendColor(const Color& color)
    {
        bgBlendColor_ = color;
    }

    Color GetBgBlendColor() const
    {
        return bgBlendColor_;
    }

    void SetIsHover(bool isHover)
    {
        isHover_ = isHover;
    }

    bool IsHover() const
    {
        return isHover_;
    }

    void PlayBgColorAnimation(bool isHoverChange = true);

    void UpdateText(const std::string& content);
    void UpdateIcon(const std::string& src);

private:
    void OnModifyDone() override;

    // register option's callback
    void RegisterOnClick();

    void RegisterOnTouch();
    void RegisterOnHover();
    void RegisterOnKeyEvent(const RefPtr<FocusHub>& focusHub);

    // change option paint props on press
    void OnPress(const TouchEventInfo& info);
    void OnHover(bool isHover);
    bool OnKeyEvent(const KeyEvent& event);

    void OnSelectProcess();

    std::optional<Color> bgColor_;

    // src of icon image, used in XTS inspector
    std::string iconSrc_;
    RefPtr<FrameNode> text_;
    RefPtr<FrameNode> icon_;
    RefPtr<TextTheme> textTheme_;
    RefPtr<SelectTheme> selectTheme_;
    // this option node's index in the menu
    int index_ = -1;

    Color bgBlendColor_ = Color::TRANSPARENT;
    bool isHover_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(OptionPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OPTION_OPTION_PATTERN_H
