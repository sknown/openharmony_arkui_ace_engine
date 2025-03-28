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

#include "core/components_ng/pattern/button/toggle_button_pattern.h"

#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/common/recorder/node_data_cache.h"
#include "core/components/button/button_theme.h"
#include "core/components/common/properties/color.h"
#include "core/components/toggle/toggle_theme.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/button/toggle_button_paint_property.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/property/property.h"
#include "core/pipeline/pipeline_base.h"
#include "core/components_ng/pattern/toggle/toggle_model.h"

namespace OHOS::Ace::NG {
namespace {
const Color ITEM_FILL_COLOR = Color::TRANSPARENT;
constexpr int32_t TOUCH_DURATION = 100;
constexpr int32_t TYPE_TOUCH = 0;
constexpr int32_t TYPE_HOVER = 1;
constexpr int32_t TYPE_CANCEL = 2;
}

void ToggleButtonPattern::OnAttachToFrameNode()
{
    InitParameters();
}

void ToggleButtonPattern::InitParameters()
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto toggleTheme = pipeline->GetTheme<ToggleTheme>();
    CHECK_NULL_VOID(toggleTheme);
    checkedColor_ = toggleTheme->GetCheckedColor();
    unCheckedColor_ = toggleTheme->GetBackgroundColor();
    textMargin_ = toggleTheme->GetTextMargin();
    buttonMargin_ = toggleTheme->GetButtonMargin();
    buttonHeight_ = toggleTheme->GetButtonHeight();
    buttonRadius_ = toggleTheme->GetButtonRadius();
    textFontSize_ = toggleTheme->GetTextFontSize();
    textColor_ = toggleTheme->GetTextColor();
    disabledAlpha_ = toggleTheme->GetDisabledAlpha();
    auto buttonTheme = pipeline->GetTheme<ButtonTheme>();
    CHECK_NULL_VOID(buttonTheme);
    clickedColor_ = buttonTheme->GetClickedColor();
}

void ToggleButtonPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    auto layoutProperty = host->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    if (layoutProperty->GetPositionProperty()) {
        layoutProperty->UpdateAlignment(
            layoutProperty->GetPositionProperty()->GetAlignment().value_or(Alignment::CENTER));
    } else {
        layoutProperty->UpdateAlignment(Alignment::CENTER);
    }

    auto buttonPaintProperty = GetPaintProperty<ToggleButtonPaintProperty>();
    CHECK_NULL_VOID(buttonPaintProperty);
    if (!isOn_.has_value()) {
        isOn_ = buttonPaintProperty->GetIsOnValue();
    }
    bool changed = false;
    if (buttonPaintProperty->HasIsOn()) {
        bool isOn = buttonPaintProperty->GetIsOnValue();
        changed = isOn ^ isOn_.value();
        isOn_ = isOn;
    }
    const auto& renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);

    if (!UseContentModifier()) {
        if (isOn_.value()) {
            auto selectedColor = buttonPaintProperty->GetSelectedColor().value_or(checkedColor_);
            renderContext->UpdateBackgroundColor(selectedColor);
        } else {
            auto bgColor = buttonPaintProperty->GetBackgroundColor().value_or(unCheckedColor_);
            renderContext->UpdateBackgroundColor(bgColor);
        }
    }

    if (changed) {
        auto toggleButtonEventHub = GetEventHub<ToggleButtonEventHub>();
        CHECK_NULL_VOID(toggleButtonEventHub);
        toggleButtonEventHub->UpdateChangeEvent(isOn_.value());
    }
    FireBuilder();
    InitButtonAndText();
    HandleEnabled();
    HandleBorderColorAndWidth();
    InitClickEvent();
    InitTouchEvent();
    InitHoverEvent();
    InitOnKeyEvent();
    InitFocusEvent();
    SetAccessibilityAction();
}

void ToggleButtonPattern::HandleBorderColorAndWidth()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto toggleTheme = pipeline->GetTheme<ToggleTheme>();
    CHECK_NULL_VOID(toggleTheme);
    auto paintProperty = host->GetPaintProperty<ToggleButtonPaintProperty>();
    CHECK_NULL_VOID(paintProperty);

    BorderColorProperty borderColor;
    BorderWidthProperty borderWidth;
    Dimension width = toggleTheme->GetBorderWidth();
    Color color = paintProperty->GetIsOnValue(false) ?
        toggleTheme->GetBorderColorChecked() : toggleTheme->GetBorderColorUnchecked();
    borderColor.SetColor(color);
    borderWidth.SetBorderWidth(width);

    auto layoutProperty = GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    if (!layoutProperty->GetBorderWidthProperty()) {
        if (!renderContext->HasBorderWidth()) {
            layoutProperty->UpdateBorderWidth(borderWidth);
            renderContext->UpdateBorderWidth(borderWidth);
        }
        if (!renderContext->HasBorderColor()) {
            renderContext->UpdateBorderColor(borderColor);
        }
    }
}

void ToggleButtonPattern::InitFocusEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto toggleTheme = pipeline->GetTheme<ToggleTheme>();
    CHECK_NULL_VOID(toggleTheme);
    auto textNode = DynamicCast<FrameNode>(host->GetFirstChild());
    CHECK_NULL_VOID(textNode);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    auto paintProperty = host->GetPaintProperty<ToggleButtonPaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    auto focusTask = [weak = WeakClaim(this), render = renderContext, theme = toggleTheme, node = textNode,
        textLayout = textLayoutProperty, paint = paintProperty]() {
        TAG_LOGD(AceLogTag::ACE_SELECT_COMPONENT, "status button handle focus event");
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleFocusEvent(render, theme, node, textLayout, paint);
    };
    focusHub->SetOnFocusInternal(focusTask);
    auto blurTask = [weak = WeakClaim(this), render = renderContext, theme = toggleTheme, node = textNode,
        textLayout = textLayoutProperty, paint = paintProperty]() {
        TAG_LOGD(AceLogTag::ACE_SELECT_COMPONENT, "status button handle blur event");
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleBlurEvent(render, theme, node, textLayout, paint);
    };
    focusHub->SetOnBlurInternal(blurTask);
}

void ToggleButtonPattern::HandleBlurEvent(RefPtr<RenderContext> renderContext, RefPtr<ToggleTheme> toggleTheme,
    RefPtr<FrameNode> textNode, RefPtr<TextLayoutProperty> textLayoutProperty,
    RefPtr<ToggleButtonPaintProperty> paintProperty)
{
    isFocus_ = false;
    if (isCheckedShadow_ && isOn_.value()) {
        isCheckedShadow_ = false;
        ShadowStyle shadowStyle = static_cast<ShadowStyle>(toggleTheme->GetShadowNormal());
        renderContext->UpdateBackShadow(Shadow::CreateShadow(shadowStyle));
    }

    if (isShadow_ && !isOn_.value()) {
        isShadow_ = false;
        renderContext->UpdateBackShadow(Shadow::CreateShadow(ShadowStyle::None));
    }
    if (isScale_) {
        isScale_ = false;
        renderContext->SetScale(1.0, 1.0);
    }
    if (isbgColorFocus_) {
        Color color =
            paintProperty->GetIsOnValue(false) ? toggleTheme->GetCheckedColor() : toggleTheme->GetBackgroundColor();
        renderContext->UpdateBackgroundColor(color);
    }
    if (isTextColor_) {
        isTextColor_ = false;
        textLayoutProperty->UpdateTextColor(toggleTheme->GetTextColor());
        textNode->MarkModifyDone();
        textNode->MarkDirtyNode();
    }
}

void ToggleButtonPattern::HandleFocusEvent(RefPtr<RenderContext> renderContext, RefPtr<ToggleTheme> toggleTheme,
    RefPtr<FrameNode> textNode, RefPtr<TextLayoutProperty> textLayoutProperty,
    RefPtr<ToggleButtonPaintProperty> paintProperty)
{
    isFocus_ = true;
    auto && graphics = renderContext->GetOrCreateGraphics();
    CHECK_NULL_VOID(graphics);
    auto && transform = renderContext->GetOrCreateTransform();
    CHECK_NULL_VOID(transform);
    float sacleFocus = toggleTheme->GetScaleFocus();
    VectorF scale(sacleFocus, sacleFocus);
    isTextColor_ = textLayoutProperty->GetTextColor() == toggleTheme->GetTextColor();
    if (isTextColor_) {
        textLayoutProperty->UpdateTextColor(toggleTheme->GetTextColorFocus());
        textNode->MarkModifyDone();
        textNode->MarkDirtyNode();
    }
    if (!transform->HasTransformScale() || transform->GetTransformScaleValue() == scale) {
        isScale_ = true;
        renderContext->SetScale(sacleFocus, sacleFocus);
    }

    ShadowStyle focusShadowStyle = static_cast<ShadowStyle>(toggleTheme->GetShadowFocus());
    Shadow normalshadow = Shadow::CreateShadow(static_cast<ShadowStyle>(toggleTheme->GetShadowNormal()));
    if (paintProperty->GetIsOnValue(false)) {
        if (!graphics->HasBackShadow() || graphics->GetBackShadowValue() == normalshadow) {
            isCheckedShadow_ = true;
            renderContext->UpdateBackShadow(Shadow::CreateShadow(focusShadowStyle));
        }
        isbgColorFocus_ = renderContext->GetBackgroundColor() == toggleTheme->GetCheckedColor();
        if (isbgColorFocus_) {
            renderContext->UpdateBackgroundColor(toggleTheme->GetBackgroundColorFocusChecked());
        }
        return;
    }
    if (!graphics->HasBackShadow() || graphics->GetBackShadowValue() == Shadow::CreateShadow(ShadowStyle::None)) {
        isShadow_ = true;
        renderContext->UpdateBackShadow(Shadow::CreateShadow(focusShadowStyle));
    }
    isbgColorFocus_ = renderContext->GetBackgroundColor() == toggleTheme->GetBackgroundColor();
    if (isbgColorFocus_) {
        renderContext->UpdateBackgroundColor(toggleTheme->GetBackgroundColorFocusUnchecked());
    }
}

void ToggleButtonPattern::SetAccessibilityAction()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto accessibilityProperty = host->GetAccessibilityProperty<AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    accessibilityProperty->SetActionSelect([weakPtr = WeakClaim(this)]() {
        const auto& pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->UpdateSelectStatus(true);
    });

    accessibilityProperty->SetActionClearSelection([weakPtr = WeakClaim(this)]() {
        const auto& pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->UpdateSelectStatus(false);
    });
}

void ToggleButtonPattern::UpdateSelectStatus(bool isSelected)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetRenderContext();
    CHECK_NULL_VOID(context);
    MarkIsSelected(isSelected);
    context->OnMouseSelectUpdate(isSelected, ITEM_FILL_COLOR, ITEM_FILL_COLOR);
}

void ToggleButtonPattern::MarkIsSelected(bool isSelected)
{
    if (isOn_ == isSelected) {
        return;
    }
    isOn_ = isSelected;
    auto eventHub = GetEventHub<ToggleButtonEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->UpdateChangeEvent(isSelected);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (isSelected) {
        eventHub->SetCurrentUIState(UI_STATE_SELECTED, isSelected);
        host->OnAccessibilityEvent(AccessibilityEventType::SELECTED);
    } else {
        eventHub->SetCurrentUIState(UI_STATE_SELECTED, isSelected);
        host->OnAccessibilityEvent(AccessibilityEventType::CHANGE);
    }
}

void ToggleButtonPattern::OnAfterModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto inspectorId = host->GetInspectorId().value_or("");
    if (!inspectorId.empty()) {
        Recorder::NodeDataCache::Get().PutBool(host, inspectorId, isOn_.value_or(false));
    }
}

void ToggleButtonPattern::HandleEnabled()
{
    if (UseContentModifier()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto enabled = eventHub->IsEnabled();
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<ToggleTheme>();
    CHECK_NULL_VOID(theme);
    auto backgroundColor = renderContext->GetBackgroundColor().value_or(theme->GetCheckedColor());
    if (!enabled) {
        if (host->GetFirstChild()) {
            auto textNode = DynamicCast<FrameNode>(host->GetFirstChild());
            CHECK_NULL_VOID(textNode);
            auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
            CHECK_NULL_VOID(textLayoutProperty);
            auto color = textLayoutProperty->GetTextColorValue(textColor_);
            textLayoutProperty->UpdateTextColor(color.BlendOpacity(disabledAlpha_));
        }
        renderContext->OnBackgroundColorUpdate(backgroundColor.BlendOpacity(disabledAlpha_));
    } else {
        renderContext->OnBackgroundColorUpdate(backgroundColor);
    }
}

void ToggleButtonPattern::InitTouchEvent()
{
    if (touchListener_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gesture = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gesture);
    auto touchCallback = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto buttonPattern = weak.Upgrade();
        CHECK_NULL_VOID(buttonPattern);
        if (info.GetSourceDevice() == SourceType::TOUCH && info.IsPreventDefault()) {
            buttonPattern->isTouchPreventDefault_ = info.IsPreventDefault();
        }
        if (info.GetTouches().front().GetTouchType() == TouchType::DOWN) {
            TAG_LOGD(AceLogTag::ACE_SELECT_COMPONENT, "button touch down");
            buttonPattern->OnTouchDown();
        }
        if (info.GetTouches().front().GetTouchType() == TouchType::UP ||
            info.GetTouches().front().GetTouchType() == TouchType::CANCEL) {
            TAG_LOGD(AceLogTag::ACE_SELECT_COMPONENT, "button touch up");
            buttonPattern->OnTouchUp();
        }
    };
    touchListener_ = MakeRefPtr<TouchEventImpl>(std::move(touchCallback));
    gesture->AddTouchAfterEvent(touchListener_);
}

void ToggleButtonPattern::OnTouchDown()
{
    isPress_ = true;
    FireBuilder();
    if (UseContentModifier()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto buttonEventHub = GetEventHub<ButtonEventHub>();
    CHECK_NULL_VOID(buttonEventHub);
    if (buttonEventHub->GetStateEffect()) {
        auto renderContext = host->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        backgroundColor_ = renderContext->GetBackgroundColor().value_or(Color::TRANSPARENT);
        if (isSetClickedColor_) {
            // for user self-defined
            renderContext->UpdateBackgroundColor(clickedColor_);
            return;
        }
        // for system default
        auto isNeedToHandleHoverOpacity = false;
        AnimateTouchAndHover(renderContext, isNeedToHandleHoverOpacity ? TYPE_HOVER : TYPE_CANCEL, TYPE_TOUCH,
            TOUCH_DURATION, isNeedToHandleHoverOpacity ? Curves::SHARP : Curves::FRICTION);
    }
}

void ToggleButtonPattern::OnTouchUp()
{
    isPress_ = false;
    FireBuilder();
    if (UseContentModifier()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto buttonEventHub = GetEventHub<ButtonEventHub>();
    CHECK_NULL_VOID(buttonEventHub);
    if (buttonEventHub->GetStateEffect()) {
        auto renderContext = host->GetRenderContext();
        if (isSetClickedColor_) {
            renderContext->UpdateBackgroundColor(backgroundColor_);
            return;
        }
        if (buttonEventHub->IsEnabled()) {
            auto isNeedToHandleHoverOpacity = false;
            AnimateTouchAndHover(renderContext, TYPE_TOUCH, isNeedToHandleHoverOpacity ? TYPE_HOVER : TYPE_CANCEL,
                TOUCH_DURATION, isNeedToHandleHoverOpacity ? Curves::SHARP : Curves::FRICTION);
        } else {
            AnimateTouchAndHover(renderContext, TYPE_TOUCH, TYPE_CANCEL, TOUCH_DURATION, Curves::FRICTION);
        }
    }
}

void ToggleButtonPattern::InitClickEvent()
{
    if (clickListener_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gesture = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gesture);
    auto clickCallback = [weak = WeakClaim(this)](GestureEvent& info) {
        auto buttonPattern = weak.Upgrade();
        CHECK_NULL_VOID(buttonPattern);
        if (info.GetSourceDevice() == SourceType::TOUCH &&
            (info.IsPreventDefault() || buttonPattern->isTouchPreventDefault_)) {
            TAG_LOGD(AceLogTag::ACE_SELECT_COMPONENT, "toggle button preventDefault successfully");
            buttonPattern->isTouchPreventDefault_ = false;
            return;
        }
        buttonPattern->OnClick();
    };
    clickListener_ = MakeRefPtr<ClickEvent>(std::move(clickCallback));
    gesture->AddClickAfterEvent(clickListener_);
}

void ToggleButtonPattern::OnClick()
{
    if (UseContentModifier()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto paintProperty = host->GetPaintProperty<ToggleButtonPaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    bool isLastSelected = false;
    if (paintProperty->HasIsOn()) {
        isLastSelected = paintProperty->GetIsOnValue();
    }
    const auto& renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    Color selectedColor;
    auto buttonPaintProperty = host->GetPaintProperty<ToggleButtonPaintProperty>();
    CHECK_NULL_VOID(buttonPaintProperty);
    if (isLastSelected) {
        selectedColor = buttonPaintProperty->GetBackgroundColor().value_or(unCheckedColor_);
    } else {
        selectedColor = buttonPaintProperty->GetSelectedColor().value_or(checkedColor_);
    }
    paintProperty->UpdateIsOn(!isLastSelected);
    isOn_ = !isLastSelected;
    renderContext->UpdateBackgroundColor(selectedColor);
    auto buttonEventHub = GetEventHub<ToggleButtonEventHub>();
    CHECK_NULL_VOID(buttonEventHub);
    buttonEventHub->UpdateChangeEvent(!isLastSelected);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    HandleOnOffStyle(!isOn_.value(), isFocus_);
}

void ToggleButtonPattern::HandleOnOffStyle(bool isOnToOff, bool isFocus)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    const auto& renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto toggleTheme = pipeline->GetTheme<ToggleTheme>();
    CHECK_NULL_VOID(toggleTheme);
    auto && graphics = renderContext->GetOrCreateGraphics();
    CHECK_NULL_VOID(graphics);
    if (isFocus) {
        ShadowStyle focusShadowStyle = static_cast<ShadowStyle>(toggleTheme->GetShadowFocus());
        Shadow shadow = Shadow::CreateShadow(focusShadowStyle);
        auto isShadow = !graphics->HasBackShadow() || graphics->GetBackShadowValue() == shadow;
        isOnToOff ? isShadow_ = isShadow : isCheckedShadow_ = isShadow;
        if (isShadow) {
            renderContext->UpdateBackShadow(shadow);
        }
        if (isbgColorFocus_) {
            renderContext->UpdateBackgroundColor(isOnToOff ? toggleTheme->GetBackgroundColorFocusUnchecked() :
                toggleTheme->GetBackgroundColorFocusChecked());
        }
    } else {
        Shadow shadowNone = Shadow::CreateShadow(ShadowStyle::None);
        Shadow normalShadow = Shadow::CreateShadow(static_cast<ShadowStyle>(toggleTheme->GetShadowNormal()));
        if (!graphics->HasBackShadow() || graphics->GetBackShadowValue() == (isOnToOff ? normalShadow : shadowNone)) {
            renderContext->UpdateBackShadow(isOnToOff ? shadowNone : normalShadow);
        }
    }
    BorderColorProperty borderColor;
    borderColor.SetColor(isOnToOff ? toggleTheme->GetBorderColorChecked() : toggleTheme->GetBorderColorUnchecked());
    if (!renderContext->HasBorderColor() || renderContext->GetBorderColor() == borderColor) {
        BorderColorProperty color;
        color.SetColor(isOnToOff ? toggleTheme->GetBorderColorUnchecked() : toggleTheme->GetBorderColorChecked());
        renderContext->UpdateBorderColor(color);
    }
}

void ToggleButtonPattern::InitButtonShadow()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto toggleTheme = pipeline->GetTheme<ToggleTheme>();
    CHECK_NULL_VOID(toggleTheme);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto && graphics = renderContext->GetOrCreateGraphics();
    CHECK_NULL_VOID(graphics);
    auto paintProperty = host->GetPaintProperty<ToggleButtonPaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    CHECK_NULL_VOID(paintProperty->GetIsOn());
    if (!graphics->HasBackShadow() && paintProperty->GetIsOnValue(false)) {
        ShadowStyle shadowStyle = static_cast<ShadowStyle>(toggleTheme->GetShadowNormal());
        Shadow shadow = Shadow::CreateShadow(shadowStyle);
        renderContext->UpdateBackShadow(shadow);
    } else if (!graphics->HasBackShadow() && !paintProperty->GetIsOnValue(false)) {
        renderContext->UpdateBackShadow(Shadow::CreateShadow(ShadowStyle::None));
    }
}

void ToggleButtonPattern::InitButtonAndText()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);

    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    if (!renderContext->HasBorderRadius()) {
        renderContext->UpdateBorderRadius({ buttonRadius_, buttonRadius_, buttonRadius_, buttonRadius_ });
    }
    InitButtonShadow();
    if (!host->GetFirstChild()) {
        return;
    }
    auto textNode = DynamicCast<FrameNode>(host->GetFirstChild());
    CHECK_NULL_VOID(textNode);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    if (textLayoutProperty->HasFontSize()) {
        layoutProperty->UpdateFontSize(textLayoutProperty->GetFontSizeValue(textFontSize_));
    }
    layoutProperty->UpdateLabel(textLayoutProperty->GetContentValue(""));
    if (!textLayoutProperty->GetTextColor().has_value()) {
        textLayoutProperty->UpdateTextColor(textColor_);
    }

    if (!textLayoutProperty->GetMarginProperty()) {
        MarginProperty margin;
        margin.left = CalcLength(textMargin_.ConvertToPx());
        margin.right = CalcLength(textMargin_.ConvertToPx());
        textLayoutProperty->UpdateMargin(margin);
    }
    textNode->MarkModifyDone();
    textNode->MarkDirtyNode();
}

void ToggleButtonPattern::InitOnKeyEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto focusHub = host->GetOrCreateFocusHub();
    auto onKeyEvent = [wp = WeakClaim(this)](const KeyEvent& event) -> bool {
        auto pattern = wp.Upgrade();
        if (!pattern) {
            return false;
        }
        return pattern->OnKeyEvent(event);
    };
    focusHub->SetOnKeyEventInternal(std::move(onKeyEvent));
}

bool ToggleButtonPattern::OnKeyEvent(const KeyEvent& event)
{
    if (event.action != KeyAction::DOWN) {
        return false;
    }
    if (event.code == KeyCode::KEY_SPACE || event.code == KeyCode::KEY_ENTER) {
        OnClick();
        return true;
    }
    return false;
}

std::string ToggleButtonPattern::ProvideRestoreInfo()
{
    auto jsonObj = JsonUtil::Create(true);
    jsonObj->Put("IsOn", isOn_.value_or(false));
    return jsonObj->ToString();
}

void ToggleButtonPattern::OnRestoreInfo(const std::string& restoreInfo)
{
    auto toggleButtonPaintProperty = GetPaintProperty<ToggleButtonPaintProperty>();
    CHECK_NULL_VOID(toggleButtonPaintProperty);
    auto info = JsonUtil::ParseJsonString(restoreInfo);
    if (!info->IsValid() || !info->IsObject()) {
        return;
    }
    auto jsonIsOn = info->GetValue("IsOn");
    toggleButtonPaintProperty->UpdateIsOn(jsonIsOn->GetBool());
    OnModifyDone();
}

void ToggleButtonPattern::OnColorConfigurationUpdate()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto toggleTheme = pipeline->GetTheme<ToggleTheme>();
    CHECK_NULL_VOID(toggleTheme);
    checkedColor_ = toggleTheme->GetCheckedColor();
    unCheckedColor_ = toggleTheme->GetBackgroundColor();
    OnModifyDone();
}

void ToggleButtonPattern::SetButtonPress(bool isSelected)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto enabled = eventHub->IsEnabled();
    if (!enabled) {
        return;
    }
    auto paintProperty = host->GetPaintProperty<ToggleButtonPaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    paintProperty->UpdateIsOn(isSelected);
    OnModifyDone();
}

void ToggleButtonPattern::FireBuilder()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (!toggleMakeFunc_.has_value()) {
        auto children = host->GetChildren();
        for (const auto& child : children) {
            if (child->GetId() == nodeId_) {
                host->RemoveChildAndReturnIndex(child);
                host->MarkNeedFrameFlushDirty(PROPERTY_UPDATE_MEASURE);
                break;
            }
        }
        return;
    }
    auto node = BuildContentModifierNode();
    if (contentModifierNode_ == node) {
        return;
    }
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateBackgroundColor(Color::TRANSPARENT);
    host->RemoveChildAndReturnIndex(contentModifierNode_);
    contentModifierNode_ = node;
    CHECK_NULL_VOID(contentModifierNode_);
    nodeId_ = contentModifierNode_->GetId();
    host->AddChild(contentModifierNode_, 0);
    host->MarkNeedFrameFlushDirty(PROPERTY_UPDATE_MEASURE);
}

RefPtr<FrameNode> ToggleButtonPattern::BuildContentModifierNode()
{
    if (!toggleMakeFunc_.has_value()) {
        return nullptr;
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(eventHub, nullptr);
    auto enabled = eventHub->IsEnabled();
    auto paintProperty = host->GetPaintProperty<ToggleButtonPaintProperty>();
    CHECK_NULL_RETURN(paintProperty, nullptr);
    bool isSelected = false;
    if (paintProperty->HasIsOn()) {
        isSelected = paintProperty->GetIsOnValue();
    } else {
        isSelected = false;
    }
    return (toggleMakeFunc_.value())(ToggleConfiguration(enabled, isSelected));
}

void ToggleButtonPattern::SetToggleBuilderFunc(SwitchMakeCallback&& toggleMakeFunc)
{
    if (toggleMakeFunc == nullptr) {
        toggleMakeFunc_ = std::nullopt;
        contentModifierNode_ = nullptr;
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        for (auto child : host->GetChildren()) {
            auto childNode = DynamicCast<FrameNode>(child);
            if (childNode) {
                childNode->GetLayoutProperty()->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
            }
        }
        OnModifyDone();
        return;
    }
    toggleMakeFunc_ = std::move(toggleMakeFunc);
}
} // namespace OHOS::Ace::NG
