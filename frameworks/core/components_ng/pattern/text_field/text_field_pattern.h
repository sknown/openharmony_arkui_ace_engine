/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_TEXT_FIELD_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_TEXT_FIELD_PATTERN_H

#include <cstdint>
#include <optional>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/rect.h"
#include "base/memory/referenced.h"
#include "base/mousestyle/mouse_style.h"
#include "base/view_data/view_data_wrap.h"
#include "core/common/autofill/auto_fill_trigger_state_holder.h"
#include "core/common/clipboard/clipboard.h"
#include "core/common/ime/text_edit_controller.h"
#include "core/common/ime/text_input_action.h"
#include "core/common/ime/text_input_client.h"
#include "core/common/ime/text_input_configuration.h"
#include "core/common/ime/text_input_connection.h"
#include "core/common/ime/text_input_formatter.h"
#include "core/common/ime/text_input_proxy.h"
#include "core/common/ime/text_input_type.h"
#include "core/common/ime/text_selection.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/image_provider/image_loading_context.h"
#include "core/components_ng/pattern/overlay/keyboard_base_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/scroll/inner/scroll_bar.h"
#include "core/components_ng/pattern/scroll_bar/proxy/scroll_bar_proxy.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/pattern/select_overlay/magnifier.h"
#include "core/components_ng/pattern/select_overlay/magnifier_controller.h"
#include "core/components_ng/pattern/text/text_base.h"
#include "core/components_ng/pattern/text/text_menu_extension.h"
#include "core/components_ng/pattern/text_area/text_area_layout_algorithm.h"
#include "core/components_ng/pattern/text_drag/text_drag_base.h"
#include "core/components_ng/pattern/text_field/content_controller.h"
#include "core/components_ng/pattern/text_field/text_editing_value_ng.h"
#include "core/components_ng/pattern/text_field/text_content_type.h"
#include "core/components_ng/pattern/text_field/text_field_accessibility_property.h"
#include "core/components_ng/pattern/text_field/text_field_controller.h"
#include "core/components_ng/pattern/text_field/text_field_event_hub.h"
#include "core/components_ng/pattern/text_field/text_field_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_paint_method.h"
#include "core/components_ng/pattern/text_field/text_field_paint_property.h"
#include "core/components_ng/pattern/text_field/text_field_select_overlay.h"
#include "core/components_ng/pattern/text_field/text_input_response_area.h"
#include "core/components_ng/pattern/text_field/text_select_controller.h"
#include "core/components_ng/pattern/text_field/text_selector.h"
#include "core/components_ng/pattern/text_input/text_input_layout_algorithm.h"
#include "core/components_ng/property/property.h"

#if not defined(ACE_UNITTEST)
#if defined(ENABLE_STANDARD_INPUT)
#include "commonlibrary/c_utils/base/include/refbase.h"

namespace OHOS::MiscServices {
class InspectorFilter;
class OnTextChangedListener;

struct TextConfig;
} // namespace OHOS::MiscServices
#endif
#endif

namespace OHOS::Ace::NG {

enum class FocuseIndex { TEXT = 0, CANCEL, UNIT };

enum class SelectionMode { SELECT, SELECT_ALL, NONE };

enum class DragStatus { DRAGGING, ON_DROP, NONE };

enum class CaretStatus { SHOW, HIDE, NONE };

enum class InputOperation {
    INSERT,
    DELETE_BACKWARD,
    DELETE_FORWARD,
    CURSOR_UP,
    CURSOR_DOWN,
    CURSOR_LEFT,
    CURSOR_RIGHT,
    SET_PREVIEW_TEXT,
    SET_PREVIEW_FINISH,
};

enum {
    ACTION_SELECT_ALL, // Smallest code unit.
    ACTION_UNDO,
    ACTION_REDO,
    ACTION_CUT,
    ACTION_COPY,
    ACTION_PASTE,
    ACTION_SHARE,
    ACTION_PASTE_AS_PLAIN_TEXT,
    ACTION_REPLACE,
    ACTION_ASSIST,
    ACTION_AUTOFILL,
};

struct PasswordModeStyle {
    Color bgColor;
    Color textColor;
    BorderWidthProperty borderwidth;
    BorderColorProperty borderColor;
    BorderRadiusProperty radius;
    PaddingProperty padding;
    MarginProperty margin;
};

struct PreState {
    Color textColor;
    Color bgColor;
    BorderRadiusProperty radius;
    BorderWidthProperty borderWidth;
    BorderColorProperty borderColor;
    PaddingProperty padding;
    MarginProperty margin;
    RectF frameRect;
    bool setHeight = false;
    bool saveState = false;
    bool hasBorderColor = false;
};

struct PreviewTextInfo {
    std::string text;
    PreviewRange range;
};

struct SourceAndValueInfo {
    std::string insertValue;
    bool isIME = false;
};

class TextFieldPattern : public ScrollablePattern,
                         public TextDragBase,
                         public ValueChangeObserver,
                         public TextInputClient,
                         public TextBase,
                         public Magnifier {
    DECLARE_ACE_TYPE(
        TextFieldPattern, ScrollablePattern, TextDragBase, ValueChangeObserver, TextInputClient, TextBase, Magnifier);

public:
    TextFieldPattern();
    ~TextFieldPattern() override;

    int32_t GetInstanceId() const override
    {
        return GetHostInstanceId();
    }

    // TextField needs softkeyboard, override function.
    bool NeedSoftKeyboard() const override
    {
        return needToRequestKeyboardOnFocus_;
    }
    void SetBlurOnSubmit(bool blurOnSubmit)
    {
        textInputBlurOnSubmit_ = blurOnSubmit;
        textAreaBlurOnSubmit_ = blurOnSubmit;
    }
    bool GetBlurOnSubmit()
    {
        return IsTextArea() ? textAreaBlurOnSubmit_ : textInputBlurOnSubmit_;
    }
    bool GetNeedToRequestKeyboardOnFocus() const
    {
        return needToRequestKeyboardOnFocus_;
    }

    bool CheckBlurReason();

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override;

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<TextFieldLayoutProperty>();
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<TextFieldEventHub>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<TextFieldPaintProperty>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<TextFieldAccessibilityProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        if (IsTextArea()) {
            return MakeRefPtr<TextAreaLayoutAlgorithm>();
        }
        return MakeRefPtr<TextInputLayoutAlgorithm>();
    }

    void OnModifyDone() override;
    void ProcessUnderlineColorOnModifierDone();
    void UpdateSelectionOffset();
    void CalcCaretMetricsByPosition(
        int32_t extent, CaretMetricsF& caretCaretMetric, TextAffinity textAffinity = TextAffinity::DOWNSTREAM);
    int32_t ConvertTouchOffsetToCaretPosition(const Offset& localOffset);
    int32_t ConvertTouchOffsetToCaretPositionNG(const Offset& localOffset);

    // Obtain the systemWindowsId when switching between windows
    uint32_t GetSCBSystemWindowId();

    void InsertValue(const std::string& insertValue, bool isIME = false) override;
    void InsertValueOperation(const SourceAndValueInfo& info);
    void UpdateObscure(const std::string& insertValue, bool hasInsertValue);
    void UpdateCounterMargin();
    void CleanCounterNode();
    void UltralimitShake();
    void UpdateAreaBorderStyle(BorderWidthProperty& currentBorderWidth, BorderWidthProperty& overCountBorderWidth,
        BorderColorProperty& overCountBorderColor, BorderColorProperty& currentBorderColor);
    void DeleteBackward(int32_t length) override;
    void DeleteBackwardOperation(int32_t length);
    void DeleteForward(int32_t length) override;
    void DeleteForwardOperation(int32_t length);
    void HandleOnDelete(bool backward) override;
    void CreateHandles() override;

    int32_t SetPreviewText(const std::string& previewValue, const PreviewRange range) override;
    void FinishTextPreview() override;
    void SetPreviewTextOperation(PreviewTextInfo info);
    void FinishTextPreviewOperation();

    WeakPtr<LayoutWrapper> GetCounterNode()
    {
        return counterTextNode_;
    }

    bool GetShowCounterStyleValue() const
    {
        return showCountBorderStyle_;
    }

    void SetCounterState(bool counterChange)
    {
        counterChange_ = counterChange;
    }

    float GetTextOrPlaceHolderFontSize();

    void SetTextFieldController(const RefPtr<TextFieldController>& controller)
    {
        textFieldController_ = controller;
    }

    const RefPtr<TextFieldController>& GetTextFieldController()
    {
        return textFieldController_;
    }

    void SetTextEditController(const RefPtr<TextEditController>& textEditController)
    {
        textEditingController_ = textEditController;
    }

    std::string GetTextValue() const
    {
        return contentController_->GetTextValue();
    }

#if defined(IOS_PLATFORM)
    const TextEditingValue& GetInputEditingValue() const override
    {
        static TextEditingValue value;
        value.text = contentController_->GetTextValue();
        value.hint = GetPlaceHolder();
        value.selection.Update(selectController_->GetStartIndex(), selectController_->GetEndIndex());
        return value;
    };
    Offset GetGlobalOffset() const;
    double GetEditingBoxY() const override;
    double GetEditingBoxTopY() const override;
    bool GetEditingBoxModel() const override;
#endif

    bool ShouldDelayChildPressedState() const override
    {
        return false;
    }

    void UpdateEditingValue(const std::string& value, int32_t caretPosition)
    {
        contentController_->SetTextValue(value);
        selectController_->UpdateCaretIndex(caretPosition);
    }
    void UpdateCaretPositionByTouch(const Offset& offset);
    bool IsReachedBoundary(float offset);

    virtual TextInputAction GetDefaultTextInputAction() const;
    bool RequestKeyboardCrossPlatForm(bool isFocusViewChanged);
    bool RequestKeyboard(bool isFocusViewChanged, bool needStartTwinkling, bool needShowSoftKeyboard);
    bool CloseKeyboard(bool forceClose) override;
    bool CloseKeyboard(bool forceClose, bool isStopTwinkling);

    FocusPattern GetFocusPattern() const override
    {
        FocusPattern focusPattern = { FocusType::NODE, true, FocusStyleType::FORCE_NONE };
        focusPattern.SetIsFocusActiveWhenFocused(true);
        return focusPattern;
    }

    void PerformAction(TextInputAction action, bool forceCloseKeyboard = false) override;
    void UpdateEditingValue(const std::shared_ptr<TextEditingValue>& value, bool needFireChangeEvent = true) override;
    void UpdateInputFilterErrorText(const std::string& errorText) override;

    void OnValueChanged(bool needFireChangeEvent = true, bool needFireSelectChangeEvent = true) override;

    void OnAreaChangedInner() override;
    void OnHandleAreaChanged() override;
    void OnVisibleChange(bool isVisible) override;
    void HandleCounterBorder();
    std::wstring GetWideText()
    {
        return contentController_->GetWideText();
    }

    int32_t GetCaretIndex() const override
    {
        return selectController_->GetCaretIndex();
    }

    OffsetF GetFirstHandleOffset() const override
    {
        return selectController_->GetFirstHandleOffset();
    }

    OffsetF GetSecondHandleOffset() const override
    {
        return selectController_->GetSecondHandleOffset();
    }

    ACE_DEFINE_PROPERTY_ITEM_FUNC_WITHOUT_GROUP(TextInputAction, TextInputAction)

    const RefPtr<Paragraph>& GetParagraph() const
    {
        return paragraph_;
    }

    const RefPtr<Paragraph>& GetErrorParagraph() const
    {
        return errorParagraph_;
    }

    bool GetCursorVisible() const
    {
        return cursorVisible_;
    }

#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    bool GetImeAttached() const
    {
        return imeAttached_;
    }
#endif

    const OffsetF& GetLastTouchOffset()
    {
        return lastTouchOffset_;
    }

    OffsetF GetCaretOffset() const override
    {
        return movingCaretOffset_;
    }

    void SetMovingCaretOffset(const OffsetF& offset)
    {
        movingCaretOffset_ = offset;
    }

    CaretUpdateType GetCaretUpdateType() const
    {
        return caretUpdateType_;
    }

    void SetCaretUpdateType(CaretUpdateType type)
    {
        caretUpdateType_ = type;
    }

    float GetPaddingTop() const
    {
        return utilPadding_.top.value_or(0.0f);
    }

    float GetPaddingBottom() const
    {
        return utilPadding_.bottom.value_or(0.0f);
    }

    float GetPaddingLeft() const
    {
        return utilPadding_.left.value_or(0.0f);
    }

    float GetPaddingRight() const
    {
        return utilPadding_.right.value_or(0.0f);
    }

    const PaddingPropertyF& GetUtilPadding() const
    {
        return utilPadding_;
    }

    float GetHorizontalPaddingAndBorderSum() const
    {
        return utilPadding_.left.value_or(0.0f) + utilPadding_.right.value_or(0.0f) + GetBorderLeft() +
               GetBorderRight();
    }

    float GetVerticalPaddingAndBorderSum() const
    {
        return utilPadding_.top.value_or(0.0f) + utilPadding_.bottom.value_or(0.0f) + GetBorderTop() +
               GetBorderBottom();
    }

    float GetBorderLeft() const
    {
        return lastBorderWidth_.leftDimen.value_or(Dimension(0.0f)).ConvertToPx();
    }

    float GetBorderTop() const
    {
        return lastBorderWidth_.topDimen.value_or(Dimension(0.0f)).ConvertToPx();
    }

    float GetBorderBottom() const
    {
        return lastBorderWidth_.bottomDimen.value_or(Dimension(0.0f)).ConvertToPx();
    }

    float GetBorderRight() const
    {
        return lastBorderWidth_.rightDimen.value_or(Dimension(0.0f)).ConvertToPx();
    }

    const RectF& GetTextRect() override
    {
        return textRect_;
    }

    void SetTextRect(const RectF& textRect)
    {
        textRect_ = textRect;
    }

    const RectF& GetFrameRect() const
    {
        return frameRect_;
    }

    float GetCountHeight() const
    {
        return countHeight_;
    }

    const RefPtr<TextSelectController>& GetTextSelectController()
    {
        return selectController_;
    }

    const RefPtr<ContentController>& GetTextContentController()
    {
        return contentController_;
    }

    void SetInSelectMode(SelectionMode selectionMode)
    {
        selectionMode_ = selectionMode;
    }

    SelectionMode GetSelectMode() const
    {
        return selectionMode_;
    }

    bool IsSelected() const override
    {
        return selectController_->IsSelected();
    }

    bool IsUsingMouse() const
    {
        return selectOverlay_->IsUsingMouse();
    }
    int32_t GetWordLength(int32_t originCaretPosition, int32_t directionalMove);
    int32_t GetLineBeginPosition(int32_t originCaretPosition, bool needToCheckLineChanged = true);
    int32_t GetLineEndPosition(int32_t originCaretPosition, bool needToCheckLineChanged = true);
    bool IsOperation() const
    {
        return !contentController_->IsEmpty();
    }

    void CursorMove(CaretMoveIntent direction) override;
    bool CursorMoveLeft();
    bool CursorMoveLeftOperation();
    bool CursorMoveLeftWord();
    bool CursorMoveLineBegin();
    bool CursorMoveToParagraphBegin();
    bool CursorMoveHome();
    bool CursorMoveRight();
    bool CursorMoveRightOperation();
    bool CursorMoveRightWord();
    bool CursorMoveLineEnd();
    bool CursorMoveToParagraphEnd();
    bool CursorMoveEnd();
    bool CursorMoveUp();
    bool CursorMoveDown();
    bool CursorMoveUpOperation();
    bool CursorMoveDownOperation();
    void SetCaretPosition(int32_t position);
    void HandleSetSelection(int32_t start, int32_t end, bool showHandle = true) override;
    void HandleExtendAction(int32_t action) override;
    void HandleSelect(CaretMoveIntent direction) override;
    OffsetF GetDragUpperLeftCoordinates() override;

    std::vector<RectF> GetTextBoxes() override
    {
        return selectController_->GetSelectedRects();
    }
    void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const override;
    void ToJsonValueForOption(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const;
    void FromJson(const std::unique_ptr<JsonValue>& json) override;
    void InitEditingValueText(std::string content);
    void InitValueText(std::string content);

    void CloseSelectOverlay() override;
    void CloseSelectOverlay(bool animation);
    void SetInputMethodStatus(bool keyboardShown) override
    {
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
        imeShown_ = keyboardShown;
#endif
    }
    void NotifyKeyboardClosedByUser() override
    {
        isKeyboardClosedByUser_ = true;
        FocusHub::LostFocusToViewRoot();
        isKeyboardClosedByUser_ = false;
    }
    std::u16string GetLeftTextOfCursor(int32_t number) override;
    std::u16string GetRightTextOfCursor(int32_t number) override;
    int32_t GetTextIndexAtCursor() override;

    bool HasConnection() const
    {
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
        return imeShown_;
#else
        return connection_;
#endif
    }
    float PreferredLineHeight(bool isAlgorithmMeasure = false);

    void SearchRequestKeyboard();

    bool GetTextObscured() const
    {
        return textObscured_;
    }

    static std::u16string CreateObscuredText(int32_t len);
    static std::u16string CreateDisplayText(
        const std::string& content, int32_t nakedCharPosition, bool needObscureText, bool showPasswordDirectly);
    bool IsTextArea() const override;

    const RefPtr<TouchEventImpl>& GetTouchListener()
    {
        return touchListener_;
    }

    bool NeedShowPasswordIcon()
    {
        auto layoutProperty = GetLayoutProperty<TextFieldLayoutProperty>();
        CHECK_NULL_RETURN(layoutProperty, false);
        return IsInPasswordMode() && layoutProperty->GetShowPasswordIconValue(true);
    }

    void SetEnableTouchAndHoverEffect(bool enable)
    {
        enableTouchAndHoverEffect_ = enable;
    }

    RectF GetCaretRect() const override
    {
        return selectController_->GetCaretRect();
    }

    void HandleSurfaceChanged(int32_t newWidth, int32_t newHeight, int32_t prevWidth, int32_t prevHeight);
    void HandleSurfacePositionChanged(int32_t posX, int32_t posY);

    void InitSurfaceChangedCallback();
    void InitSurfacePositionChangedCallback();

    bool HasSurfaceChangedCallback()
    {
        return surfaceChangedCallbackId_.has_value();
    }
    void UpdateSurfaceChangedCallbackId(int32_t id)
    {
        surfaceChangedCallbackId_ = id;
    }

    bool HasSurfacePositionChangedCallback()
    {
        return surfacePositionChangedCallbackId_.has_value();
    }
    void UpdateSurfacePositionChangedCallbackId(int32_t id)
    {
        surfacePositionChangedCallbackId_ = id;
    }

    void ProcessInnerPadding();
    void ProcessNumberOfLines();
    void OnCursorMoveDone(
        TextAffinity textAffinity = TextAffinity::UPSTREAM, std::optional<Offset> offset = std::nullopt);
    bool IsDisabled();
    bool AllowCopy();

    bool GetIsMousePressed() const
    {
        return isMousePressed_;
    }

    MouseStatus GetMouseStatus() const
    {
        return mouseStatus_;
    }

    void SetMenuOptionItems(std::vector<MenuOptionsParam>&& menuOptionItems)
    {
        menuOptionItems_ = std::move(menuOptionItems);
    }

    const std::vector<MenuOptionsParam>&& GetMenuOptionItems() const
    {
        return std::move(menuOptionItems_);
    }

    void UpdateEditingValueToRecord();

    void UpdateScrollBarOffset() override;

    bool UpdateCurrentOffset(float offset, int32_t source) override
    {
        OnScrollCallback(offset, source);
        return true;
    }

    void PlayScrollBarAppearAnimation();

    void ScheduleDisappearDelayTask();

    bool IsAtTop() const override
    {
        return contentRect_.GetY() == textRect_.GetY();
    }

    bool IsAtBottom() const override
    {
        return contentRect_.GetY() + contentRect_.Height() == textRect_.GetY() + textRect_.Height();
    }

    bool IsScrollable() const override
    {
        return scrollable_;
    }

    bool IsAtomicNode() const override
    {
        return true;
    }

    float GetCurrentOffset() const
    {
        return currentOffset_;
    }

    RefPtr<TextFieldContentModifier> GetContentModifier()
    {
        return textFieldContentModifier_;
    }

    double GetScrollBarWidth();

    float GetLineHeight() const override
    {
        return selectController_->GetCaretRect().Height();
    }

    OffsetF GetParentGlobalOffset() const override
    {
        return parentGlobalOffset_;
    }

    RectF GetTextContentRect(bool isActualText = false) const override
    {
        return contentRect_;
    }

    const RefPtr<Paragraph>& GetDragParagraph() const override
    {
        return paragraph_;
    }

    const RefPtr<FrameNode>& MoveDragNode() override
    {
        return dragNode_;
    }

    const std::vector<std::string>& GetDragContents() const
    {
        return dragContents_;
    }

    void AddDragFrameNodeToManager(const RefPtr<FrameNode>& frameNode)
    {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        auto dragDropManager = context->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        dragDropManager->AddDragFrameNode(frameNode->GetId(), AceType::WeakClaim(AceType::RawPtr(frameNode)));
    }

    void RemoveDragFrameNodeFromManager(const RefPtr<FrameNode>& frameNode)
    {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        auto dragDropManager = context->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        dragDropManager->RemoveDragFrameNode(frameNode->GetId());
    }

    bool IsDragging() const
    {
        return dragStatus_ == DragStatus::DRAGGING;
    }

    bool BetweenSelectedPosition(const Offset& globalOffset) override
    {
        if (!IsSelected()) {
            return false;
        }
        auto localOffset = ConvertGlobalToLocalOffset(globalOffset);
        auto offsetX = IsTextArea() ? contentRect_.GetX() : textRect_.GetX();
        auto offsetY = IsTextArea() ? textRect_.GetY() : contentRect_.GetY();
        Offset offset = localOffset - Offset(offsetX, offsetY);
        for (const auto& rect : selectController_->GetSelectedRects()) {
            bool isInRange = rect.IsInRegion({ offset.GetX(), offset.GetY() });
            if (isInRange) {
                return true;
            }
        }
        return false;
    }

    bool RequestCustomKeyboard();
    bool CloseCustomKeyboard();

    // xts
    std::string TextInputTypeToString() const;
    std::string TextInputActionToString() const;
    std::string TextContentTypeToString() const;
    std::string GetPlaceholderFont() const;
    RefPtr<TextFieldTheme> GetTheme() const;
    std::string GetTextColor() const;
    std::string GetCaretColor() const;
    std::string GetPlaceholderColor() const;
    std::string GetFontSize() const;
    Ace::FontStyle GetItalicFontStyle() const;
    FontWeight GetFontWeight() const;
    std::string GetFontFamily() const;
    TextAlign GetTextAlign() const;
    std::string GetPlaceHolder() const;
    uint32_t GetMaxLength() const;
    uint32_t GetMaxLines() const;
    std::string GetInputFilter() const;
    std::string GetCopyOptionString() const;
    std::string GetInputStyleString() const;
    std::string GetErrorTextString() const;
    std::string GetBarStateString() const;
    bool GetErrorTextState() const;
    std::string GetShowPasswordIconString() const;
    int32_t GetNakedCharPosition() const;
    void SetSelectionFlag(int32_t selectionStart, int32_t selectionEnd,
        const std::optional<SelectionOptions>& options = std::nullopt, bool isForward = false);
    void HandleBlurEvent();
    void HandleFocusEvent();
    void ProcessFocusStyle();
    void SetFocusStyle();
    void ClearFocusStyle();
    void AddIsFocusActiveUpdateEvent();
    void RemoveIsFocusActiveUpdateEvent();
    void OnIsFocusActiveUpdate(bool isFocusAcitve);
    bool OnBackPressed() override;
    void CheckScrollable();
    void HandleClickEvent(GestureEvent& info);
    int32_t CheckClickLocation(GestureEvent& info);
    void HandleDoubleClickEvent(GestureEvent& info);
    void HandleTripleClickEvent(GestureEvent& info);
    void HandleSingleClickEvent(GestureEvent& info);

    void HandleSelectionUp();
    void HandleSelectionDown();
    void HandleSelectionLeft();
    void HandleSelectionLeftWord();
    void HandleSelectionLineBegin();
    void HandleSelectionHome();
    void HandleSelectionRight();
    void HandleSelectionRightWord();
    void HandleSelectionLineEnd();
    void HandleSelectionEnd();
    bool HandleOnEscape() override;
    bool HandleOnTab(bool backward) override;
    void HandleOnEnter() override
    {
        PerformAction(GetTextInputActionValue(GetDefaultTextInputAction()), false);
    }
    void HandleOnUndoAction() override;
    void HandleOnRedoAction() override;
    void HandleOnSelectAll(bool isKeyEvent, bool inlineStyle = false, bool showMenu = false);
    void HandleOnSelectAll() override
    {
        HandleOnSelectAll(true);
    }
    void HandleOnCopy(bool isUsingExternalKeyboard = false) override;
    void HandleOnPaste() override;
    void HandleOnCut() override;
    void HandleOnCameraInput();
    void UpdateShowCountBorderStyle();
    void StripNextLine(std::wstring& data);
    bool IsShowHandle();
    std::string GetCancelButton();
    bool OnKeyEvent(const KeyEvent& event);
    int32_t GetLineCount() const;
    TextInputType GetKeyboard()
    {
        return keyboard_;
    }
    TextInputAction GetAction()
    {
        return action_;
    }

    void SetNeedToRequestKeyboardOnFocus(bool needToRequest)
    {
        needToRequestKeyboardOnFocus_ = needToRequest;
    }
    void SetUnitNode(const RefPtr<NG::UINode>& unitNode)
    {
        unitNode_ = unitNode;
    }
    void AddCounterNode();
    void ClearCounterNode();
    void SetShowError();

    float GetUnderlineWidth() const
    {
        return static_cast<float>(underlineWidth_.Value());
    }

    const Color& GetUnderlineColor() const
    {
        return underlineColor_;
    }

    float GetMarginBottom() const;

    void SetUnderlineColor(Color underlineColor)
    {
        underlineColor_ = underlineColor;
    }

    void SetNormalUnderlineColor(const Color& normalColor)
    {
        userUnderlineColor_.normal = normalColor;
    }

    void SetUserUnderlineColor(UserUnderlineColor userUnderlineColor)
    {
        userUnderlineColor_ = userUnderlineColor;
    }

    UserUnderlineColor GetUserUnderlineColor()
    {
        return userUnderlineColor_;
    }

    void SetUnderlineWidth(Dimension underlineWidth)
    {
        underlineWidth_ = underlineWidth;
    }

    bool IsSelectAll()
    {
        return abs(selectController_->GetStartIndex() - selectController_->GetEndIndex()) >=
               static_cast<int32_t>(contentController_->GetWideText().length());
    }

    void StopEditing();

    void MarkContentChange()
    {
        contChange_ = true;
    }

    void ResetContChange()
    {
        contChange_ = false;
    }

    bool GetContChange() const
    {
        return contChange_;
    }
    std::string GetShowResultImageSrc() const;
    std::string GetHideResultImageSrc() const;
    std::string GetNormalUnderlineColorStr() const;
    std::string GetTypingUnderlineColorStr() const;
    std::string GetDisableUnderlineColorStr() const;
    std::string GetErrorUnderlineColorStr() const;
    void OnAttachToFrameNode() override;

    bool GetTextInputFlag() const
    {
        return isTextInput_;
    }

    void SetTextInputFlag(bool isTextInput)
    {
        isTextInput_ = isTextInput;
        SetTextFadeoutCapacity(isTextInput_);
    }

    void SetSingleLineHeight(float height)
    {
        inlineSingleLineHeight_ = height;
    }

    float GetSingleLineHeight() const
    {
        return inlineSingleLineHeight_;
    }

    float GetInlinePadding() const
    {
        return inlinePadding_;
    }

    bool GetScrollBarVisible() const
    {
        return scrollBarVisible_;
    }

    void SetFillRequestFinish(bool success)
    {
        isFillRequestFinish_ = success;
    }

    bool IsFillRequestFinish()
    {
        return isFillRequestFinish_;
    }

    bool IsNormalInlineState() const;
    bool IsUnspecifiedOrTextType() const;
    void TextIsEmptyRect(RectF& rect);
    void TextAreaInputRectUpdate(RectF& rect);
    void UpdateRectByTextAlign(RectF& rect);

    void EditingValueFilterChange();

    void SetCustomKeyboard(const std::function<void()>&& keyboardBuilder)
    {
        if (customKeyboardBuilder_ && isCustomKeyboardAttached_ && !keyboardBuilder) {
            // close customKeyboard and request system keyboard
            CloseCustomKeyboard();
            customKeyboardBuilder_ = keyboardBuilder; // refresh current keyboard
            RequestKeyboard(false, true, true);
            StartTwinkling();
            return;
        }
        if (!customKeyboardBuilder_ && keyboardBuilder) {
            // close system keyboard and request custom keyboard
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
            if (imeShown_) {
                CloseKeyboard(true);
                customKeyboardBuilder_ = keyboardBuilder; // refresh current keyboard
                RequestKeyboard(false, true, true);
                StartTwinkling();
                return;
            }
#endif
        }
        customKeyboardBuilder_ = keyboardBuilder;
    }

    void SetCustomKeyboardWithNode(const RefPtr<UINode>& keyboardBuilder)
    {
        if (customKeyboard_ && isCustomKeyboardAttached_ && !keyboardBuilder) {
            // close customKeyboard and request system keyboard
            CloseCustomKeyboard();
            customKeyboard_ = keyboardBuilder; // refresh current keyboard
            RequestKeyboard(false, true, true);
            StartTwinkling();
            return;
        }
        if (!customKeyboard_ && keyboardBuilder) {
            // close system keyboard and request custom keyboard
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
            if (imeShown_) {
                CloseKeyboard(true);
                customKeyboard_ = keyboardBuilder; // refresh current keyboard
                RequestKeyboard(false, true, true);
                StartTwinkling();
                return;
            }
#endif
        }
        customKeyboard_ = keyboardBuilder;
    }

    bool HasCustomKeyboard()
    {
        return customKeyboard_ != nullptr || customKeyboardBuilder_ != nullptr;
    }

    void DumpInfo() override;
    void DumpAdvanceInfo() override;
    void DumpViewDataPageNode(RefPtr<ViewDataWrap> viewDataWrap) override;
    void NotifyFillRequestSuccess(RefPtr<ViewDataWrap> viewDataWrap,
        RefPtr<PageNodeInfoWrap> nodeWrap, AceAutoFillType autoFillType) override;
    void NotifyFillRequestFailed(int32_t errCode, const std::string& fillContent = "", bool isPopup = false) override;
    bool CheckAutoSave() override;
    void OnColorConfigurationUpdate() override;
    bool NeedPaintSelect();
    void SetCustomKeyboardOption(bool supportAvoidance);

    void SetIsCustomFont(bool isCustomFont)
    {
        isCustomFont_ = isCustomFont;
    }

    bool GetIsCustomFont() const
    {
        return isCustomFont_;
    }

    void SetIsCounterIdealHeight(bool isIdealHeight)
    {
        isCounterIdealheight_ = isIdealHeight;
    }

    bool GetIsCounterIdealHeight() const
    {
        return isCounterIdealheight_;
    }

    virtual RefPtr<FocusHub> GetFocusHub() const;
    void UpdateCaretInfoToController();
    void OnObscuredChanged(bool isObscured);
    const RefPtr<TextInputResponseArea>& GetResponseArea()
    {
        return responseArea_;
    }

    const RefPtr<TextInputResponseArea>& GetCleanNodeResponseArea()
    {
        return cleanNodeResponseArea_;
    }

    bool IsShowUnit() const;
    bool IsShowPasswordIcon() const;
    std::optional<bool> IsShowPasswordText() const;
    bool IsInPasswordMode() const;
    bool IsShowCancelButtonMode() const;
    void CheckPasswordAreaState();

    bool GetShowSelect() const
    {
        return showSelect_;
    }

    void ShowSelect()
    {
        showSelect_ = true;
    }

    bool UpdateFocusForward();

    bool UpdateFocusBackward();

    bool HandleSpaceEvent();

    virtual void ApplyNormalTheme();
    void ApplyUnderlineTheme();
    void ApplyInlineTheme();

    int32_t GetContentWideTextLength() override
    {
        return static_cast<int32_t>(contentController_->GetWideText().length());
    }

    void HandleOnShowMenu() override
    {
        selectOverlay_->HandleOnShowMenu();
    }
    bool HasFocus() const;
    void StopTwinkling();
    void StartTwinkling();

    bool IsModifyDone()
    {
        return isModifyDone_;
    }
    void SetModifyDoneStatus(bool value)
    {
        isModifyDone_ = value;
    }

    const TimeStamp& GetLastClickTime()
    {
        return lastClickTimeStamp_;
    }

    void CheckTextAlignByDirection(TextAlign& textAlign, TextDirection direction);

    void HandleOnDragStatusCallback(
        const DragEventType& dragEventType, const RefPtr<NotifyDragEvent>& notifyDragEvent) override;

    void GetCaretMetrics(CaretMetricsF& caretCaretMetric) override;

    OffsetF GetTextPaintOffset() const override;

    OffsetF GetPaintRectGlobalOffset() const;

    void NeedRequestKeyboard()
    {
        needToRequestKeyboardInner_ = true;
    }

    void CleanNodeResponseKeyEvent();

    void ScrollPage(bool reverse, bool smooth = false) override;
    void InitScrollBarClickEvent() override {}
    bool IsUnderlineMode();
    bool IsInlineMode();
    bool IsShowError();
    bool IsShowCount();
    void ResetContextAttr();
    void RestoreDefaultMouseState();

    void RegisterWindowSizeCallback();
    void OnWindowSizeChanged(int32_t width, int32_t height, WindowSizeChangeReason type) override;

    bool IsTransparent()
    {
        return isTransparent_;
    }

    RefPtr<Clipboard> GetClipboard() override
    {
        return clipboard_;
    }

    const Dimension& GetAvoidSoftKeyboardOffset() const override;

    RectF GetPaintContentRect() override
    {
        auto transformContentRect = contentRect_;
        selectOverlay_->GetLocalRectWithTransform(transformContentRect);
        return transformContentRect;
    }

    bool ProcessAutoFill(bool& isPopup, bool isFromKeyBoard = false, bool isNewPassWord = false);
    void SetAutoFillUserName(const std::string& userName)
    {
        autoFillUserName_ = userName;
    }

    std::string GetAutoFillUserName()
    {
        return autoFillUserName_;
    }

    std::string GetAutoFillNewPassword()
    {
        return autoFillNewPassword_;
    }

    void SetAutoFillNewPassword(const std::string& newPassword)
    {
        autoFillNewPassword_ = newPassword;
    }
    void SetAutoFillOtherAccount(bool otherAccount)
    {
        autoFillOtherAccount_ = otherAccount;
    }

    std::vector<RectF> GetPreviewTextRects() const;

    bool GetIsPreviewText() const
    {
        return hasPreviewText_;
    }

    const Color& GetPreviewDecorationColor() const
    {
        auto theme = GetTheme();
        CHECK_NULL_RETURN(theme, Color::TRANSPARENT);
        return theme->GetPreviewUnderlineColor();
    }

    bool NeedDrawPreviewText();

    float GetPreviewUnderlineWidth() const
    {
        return static_cast<float>(previewUnderlineWidth_.ConvertToPx());
    }

    void ReceivePreviewTextStyle(const std::string& style) override;

    PreviewTextStyle GetPreviewTextStyle() const;

    RefPtr<UINode> GetCustomKeyboard()
    {
        return customKeyboard_;
    }

    bool GetCustomKeyboardOption()
    {
        return keyboardAvoidance_;
    }

    void SetTextFadeoutCapacity(bool enabled)
    {
        haveTextFadeoutCapacity_ = enabled;
    }
    bool GetTextFadeoutCapacity()
    {
        return haveTextFadeoutCapacity_;
    }

    void SetShowKeyBoardOnFocus(bool value);
    bool GetShowKeyBoardOnFocus()
    {
        return showKeyBoardOnFocus_;
    }

    void OnSelectionMenuOptionsUpdate(const std::vector<MenuOptionsParam> && menuOptionsItems);

    void SetSupportPreviewText(bool isSupported)
    {
        hasSupportedPreviewText_ = isSupported;
    }

    void OnTouchTestHit(SourceType hitTestType) override
    {
        selectOverlay_->OnTouchTestHit(hitTestType);
    }

    int32_t GetPreviewTextStart() const
    {
        return hasPreviewText_ ? previewTextStart_ : selectController_->GetCaretIndex();
    }

    int32_t GetPreviewTextEnd() const
    {
        return hasPreviewText_ ? previewTextEnd_ : selectController_->GetCaretIndex();
    }

    bool IsPressSelectedBox()
    {
        return isPressSelectedBox_;
    }

    int32_t CheckPreviewTextValidate(const std::string& previewValue, const PreviewRange range) override;
    void HiddenMenu();

protected:
    virtual void InitDragEvent();
    void OnAttachToMainTree() override
    {
        isDetachFromMainTree_ = false;
    }

    void OnDetachFromMainTree() override
    {
        isDetachFromMainTree_ = true;
    }

private:
    void GetTextSelectRectsInRangeAndWillChange();
    bool BeforeIMEInsertValue(const std::string& insertValue, int32_t offset);
    void AfterIMEInsertValue(const std::string& insertValue);
    bool BeforeIMEDeleteValue(const std::string& deleteValue, TextDeleteDirection direction, int32_t offset);
    void AfterIMEDeleteValue(const std::string& deleteValue, TextDeleteDirection direction);
    void OnAfterModifyDone() override;
    void HandleTouchEvent(const TouchEventInfo& info);
    void HandleTouchDown(const Offset& offset);
    void HandleTouchUp();
    void HandleTouchMove(const TouchEventInfo& info);
    void UpdateCaretByTouchMove(const TouchEventInfo& info);
    void InitDisableColor();
    void InitFocusEvent();
    void InitTouchEvent();
    void InitLongPressEvent();
    void InitClickEvent();
    void InitDragDropEvent();
    std::function<DragDropInfo(const RefPtr<OHOS::Ace::DragEvent>&, const std::string&)> OnDragStart();
    std::function<void(const RefPtr<OHOS::Ace::DragEvent>&, const std::string&)> OnDragDrop();
    void ShowSelectAfterDragEvent();
    void ClearDragDropEvent();
    std::function<void(Offset)> GetThumbnailCallback();
    void HandleCursorOnDragMoved(const RefPtr<NotifyDragEvent>& notifyDragEvent);
    void HandleCursorOnDragLeaved(const RefPtr<NotifyDragEvent>& notifyDragEvent);
    void HandleCursorOnDragEnded(const RefPtr<NotifyDragEvent>& notifyDragEvent);
    bool HasStateStyle(UIState state) const;

    void OnTextInputScroll(float offset);
    void OnTextAreaScroll(float offset);
    bool OnScrollCallback(float offset, int32_t source) override;
    void OnScrollEndCallback() override;
    bool CheckSelectAreaVisible();
    void InitMouseEvent();
    void HandleHoverEffect(MouseInfo& info, bool isHover);
    void OnHover(bool isHover);
    void ChangeMouseState(
        const Offset location, const RefPtr<PipelineContext>& pipeline, int32_t frameId, bool isByPass = false);
    void HandleMouseEvent(MouseInfo& info);
    void FocusAndUpdateCaretByMouse(MouseInfo& info);
    void HandleRightMouseEvent(MouseInfo& info);
    void HandleRightMousePressEvent(MouseInfo& info);
    void HandleRightMouseReleaseEvent(MouseInfo& info);
    void HandleLeftMouseEvent(MouseInfo& info);
    void HandleLeftMousePressEvent(MouseInfo& info);
    void HandleLeftMouseMoveEvent(MouseInfo& info);
    void HandleLeftMouseReleaseEvent(MouseInfo& info);
    void HandleLongPress(GestureEvent& info);
    void UpdateCaretPositionWithClamp(const int32_t& pos);
    void CursorMoveOnClick(const Offset& offset);

    void DelayProcessOverlay(const OverlayRequest& request = OverlayRequest());
    void ProcessOverlayAfterLayout(bool isGlobalAreaChanged);
    void ProcessOverlay(const OverlayRequest& request = OverlayRequest());

    bool SelectOverlayIsOn()
    {
        return selectOverlay_->SelectOverlayIsOn();
    }

    // when moving one handle causes shift of textRect, update x position of the other handle
    void SetHandlerOnMoveDone();
    void OnDetachFromFrameNode(FrameNode* node) override;
    void OnAttachContext(PipelineContext *context) override;
    void OnDetachContext(PipelineContext *context) override;
    void UpdateSelectionByMouseDoubleClick();

    void AfterSelection();

    void AutoFillValueChanged();
    void FireEventHubOnChange(const std::string& text);
    // The return value represents whether the editor content has change.
    bool FireOnTextChangeEvent();
    void AddTextFireOnChange();

    void FilterInitializeText();

    void UpdateSelection(int32_t both);
    void UpdateSelection(int32_t start, int32_t end);
    void UpdateCaretPositionByLastTouchOffset();
    bool UpdateCaretPosition();
    void UpdateCaretRect(bool isEditorValueChanged);
    void AdjustTextInReasonableArea();
    bool CharLineChanged(int32_t caretPosition);

    void ScheduleCursorTwinkling();
    void OnCursorTwinkling();
    void CheckIfNeedToResetKeyboard();

    float PreferredTextHeight(bool isPlaceholder, bool isAlgorithmMeasure = false);

    void SetCaretOffsetForEmptyTextOrPositionZero();
    void UpdateTextFieldManager(const Offset& offset, float height);
    void OnTextInputActionUpdate(TextInputAction value);

    void Delete(int32_t start, int32_t end);
    void BeforeCreateLayoutWrapper() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    bool CursorInContentRegion();
    bool OffsetInContentRegion(const Offset& offset);
    void SetDisabledStyle();

    void CalculateDefaultCursor();
    void RequestKeyboardOnFocus();
    bool IsModalCovered();
    void SetNeedToRequestKeyboardOnFocus();
    void SetAccessibilityAction();
    void SetAccessibilityActionGetAndSetCaretPosition();
    void SetAccessibilityMoveTextAction();
    void SetAccessibilityScrollAction();
    void SetAccessibilityDeleteAction();

    void UpdateCopyAllStatus();
    void RestorePreInlineStates();
    void ProcessRectPadding();
    void CalcInlineScrollRect(Rect& inlineScrollRect);

    bool ResetObscureTickCountDown();

    bool IsOnUnitByPosition(const Offset& globalOffset);
    bool IsTouchAtLeftOffset(float currentOffsetX);
    void FilterExistText();
    void CreateErrorParagraph(const std::string& content);
    void UpdateErrorTextMargin();
    void UpdateSelectController();
    void UpdateHandlesOffsetOnScroll(float offset);
    void CloseHandleAndSelect() override;
    bool RepeatClickCaret(const Offset& offset, int32_t lastCaretIndex, const RectF& lastCaretRect);
    void PaintTextRect();
    void GetIconPaintRect(const RefPtr<TextInputResponseArea>& responseArea, RoundRect& paintRect);
    void GetInnerFocusPaintRect(RoundRect& paintRect);
    void PaintResponseAreaRect();
    void PaintCancelRect();
    void PaintUnitRect();
    void PaintPasswordRect();
    bool CancelNodeIsShow()
    {
        auto cleanNodeArea = AceType::DynamicCast<CleanNodeResponseArea>(cleanNodeResponseArea_);
        CHECK_NULL_RETURN(cleanNodeArea, false);
        return cleanNodeArea->IsShow();
    }

    void InitPanEvent();

    void PasswordResponseKeyEvent();
    void UnitResponseKeyEvent();
    void ProcNormalInlineStateInBlurEvent();
    bool IsMouseOverScrollBar(const GestureEvent& info);

#if defined(ENABLE_STANDARD_INPUT)
    std::optional<MiscServices::TextConfig> GetMiscTextConfig() const;
    void GetInlinePositionYAndHeight(double& positionY, double& height) const;
#endif
    void SetIsSingleHandle(bool isSingleHandle)
    {
        selectOverlay_->SetIsSingleHandle(isSingleHandle);
    }
    void NotifyOnEditChanged(bool isChanged);
    void ProcessResponseArea();
    bool HasInputOperation();
    AceAutoFillType ConvertToAceAutoFillType(TextInputType type);
    bool CheckAutoFill(bool isFromKeyBoard = false);
    void ScrollToSafeArea() const override;
    void RecordSubmitEvent() const;
    void UpdateCancelNode();
    void RequestKeyboardAfterLongPress();
    void UpdatePasswordModeState();
    void InitDragDropCallBack();
    void InitDragDropEventWithOutDragStart();
    void UpdateBlurReason();
    AceAutoFillType TextContentTypeToAceAutoFillType(const TextContentType& type);
    bool CheckAutoFillType(const AceAutoFillType& aceAutoFillAllType, bool isFromKeyBoard = false);
    bool GetAutoFillTriggeredStateByType(const AceAutoFillType& autoFillType);
    void SetAutoFillTriggeredStateByType(const AceAutoFillType& autoFillType);
    AceAutoFillType GetAutoFillType();
    bool IsAutoFillPasswordType(const AceAutoFillType& autoFillType);
    void DoProcessAutoFill();
    void KeyboardContentTypeToInputType();
    void ProcessScroll();
    void ProcessCounter();
    void HandleParentGlobalOffsetChange();
    AceAutoFillType GetHintType();
    void SetThemeAttr();
    void SetThemeBorderAttr();
    void ProcessInlinePaddingAndMargin();
    Offset ConvertGlobalToLocalOffset(const Offset& globalOffset);
    void HandleCountStyle();
    void HandleDeleteOnCounterScene();
    bool ParseFillContentJsonValue(const std::unique_ptr<JsonValue>& jsonObject,
        std::unordered_map<std::string, std::variant<std::string, bool, int32_t>>& map);
    void HandleContentSizeChange(const RectF& textRect);
    void UpdatePreviewIndex(int32_t start, int32_t end)
    {
        previewTextStart_ = start;
        previewTextEnd_ = end;
    }

    void CalculatePreviewingTextMovingLimit(const Offset& touchOffset, double& limitL, double& limitR);
    void UpdateParam(GestureEvent& info, bool shouldProcessOverlayAfterLayout);
    void ShowCaretAndStopTwinkling();
    void OnCaretMoveDone(const TouchEventInfo& info);

    void TwinklingByFocus();

    bool FinishTextPreviewByPreview(const std::string& insertValue);

    bool GetTouchInnerPreviewText(const Offset& offset) const;

    RectF frameRect_;
    RectF textRect_;
    RefPtr<Paragraph> paragraph_;
    RefPtr<Paragraph> errorParagraph_;
    RefPtr<Paragraph> dragParagraph_;
    InlineMeasureItem inlineMeasureItem_;
    TextStyle nextLineUtilTextStyle_;

    RefPtr<ClickEvent> clickListener_;
    RefPtr<TouchEventImpl> touchListener_;
    RefPtr<ScrollableEvent> scrollableEvent_;
    RefPtr<InputEvent> mouseEvent_;
    RefPtr<InputEvent> hoverEvent_;
    RefPtr<LongPressEvent> longPressEvent_;
    CursorPositionType cursorPositionType_ = CursorPositionType::NORMAL;

    // What the keyboard should appears.
    TextInputType keyboard_ = TextInputType::UNSPECIFIED;
    // Action when "enter" pressed.
    TextInputAction action_ = TextInputAction::UNSPECIFIED;
    TextDirection textDirection_ = TextDirection::LTR;

    OffsetF parentGlobalOffset_;
    OffsetF lastTouchOffset_;
    PaddingPropertyF utilPadding_;

    BorderWidthProperty lastBorderWidth_;

    bool setBorderFlag_ = true;
    BorderWidthProperty lastDiffBorderWidth_;
    BorderColorProperty lastDiffBorderColor_;

    HandleMoveStatus handleMoveStatus_;
    bool cursorVisible_ = false;
    bool focusEventInitialized_ = false;
    bool isMousePressed_ = false;
    bool textObscured_ = true;
    bool enableTouchAndHoverEffect_ = true;
    bool isOnHover_ = false;
    bool needToRefreshSelectOverlay_ = false;
    bool needToRequestKeyboardInner_ = false;
    bool needToRequestKeyboardOnFocus_ = false;
    bool isTransparent_ = false;
    bool contChange_ = false;
    bool counterChange_ = false;
    WeakPtr<LayoutWrapper> counterTextNode_;
    bool isCursorAlwaysDisplayed_ = false;
    std::optional<int32_t> surfaceChangedCallbackId_;
    std::optional<int32_t> surfacePositionChangedCallbackId_;

    SelectionMode selectionMode_ = SelectionMode::NONE;
    CaretUpdateType caretUpdateType_ = CaretUpdateType::NONE;
    bool scrollable_ = true;
    bool blockPress_ = false;
    bool isPressSelectedBox_ = false;
    float previewWidth_ = 0.0f;
    float lastTextRectY_ = 0.0f;
    std::optional<DisplayMode> barState_;

    uint32_t twinklingInterval_ = 0;
    int32_t obscureTickCountDown_ = 0;
    int32_t nakedCharPosition_ = -1;
    bool obscuredChange_ = false;
    float currentOffset_ = 0.0f;
    float countHeight_ = 0.0f;
    Dimension underlineWidth_ = 1.0_px;
    Color underlineColor_;
    UserUnderlineColor userUnderlineColor_ = UserUnderlineColor();
    bool scrollBarVisible_ = false;
    bool isCounterIdealheight_ = false;
    float maxFrameOffsetY_ = 0.0f;
    float maxFrameHeight_ = 0.0f;

    CancelableCallback<void()> cursorTwinklingTask_;

    std::list<std::unique_ptr<TextInputFormatter>> textInputFormatters_;

    RefPtr<TextFieldController> textFieldController_;
    RefPtr<TextEditController> textEditingController_;
    TextEditingValueNG textEditingValue_;
    // controls redraw of overlay modifier, update when need to redraw
    bool changeSelectedRects_ = false;
    RefPtr<TextFieldOverlayModifier> textFieldOverlayModifier_;
    RefPtr<TextFieldContentModifier> textFieldContentModifier_;
    ACE_DISALLOW_COPY_AND_MOVE(TextFieldPattern);

    int32_t dragTextStart_ = 0;
    int32_t dragTextEnd_ = 0;
    std::string dragValue_;
    RefPtr<FrameNode> dragNode_;
    DragStatus dragStatus_ = DragStatus::NONE; // The status of the dragged initiator
    DragStatus dragRecipientStatus_ = DragStatus::NONE; // Drag the recipient's state
    RefPtr<Clipboard> clipboard_;
    std::vector<TextEditingValueNG> operationRecords_;
    std::vector<TextEditingValueNG> redoOperationRecords_;
    std::vector<MenuOptionsParam> menuOptionItems_;
    BorderRadiusProperty borderRadius_;
    PasswordModeStyle passwordModeStyle_;
    SelectMenuInfo selectMenuInfo_;

    RefPtr<PanEvent> boxSelectPanEvent_;

    // inline
    bool isTextInput_ = false;
    bool inlineSelectAllFlag_ = false;
    bool inlineFocusState_ = false;
    float inlineSingleLineHeight_ = 0.0f;
    float inlinePadding_ = 0.0f;

    bool isOritationListenerRegisted_ = false;

#if defined(ENABLE_STANDARD_INPUT)
    sptr<OHOS::MiscServices::OnTextChangedListener> textChangeListener_;
#else
    RefPtr<TextInputConnection> connection_;
#endif
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    bool imeAttached_ = false;
    bool imeShown_ = false;
#endif
    BlurReason blurReason_ = BlurReason::FOCUS_SWITCH;
    bool isFocusedBeforeClick_ = false;
    bool isCustomKeyboardAttached_ = false;
    std::function<void()> customKeyboardBuilder_;
    RefPtr<UINode> customKeyboard_;
    RefPtr<OverlayManager> keyboardOverlay_;
    bool isCustomFont_ = false;
    bool hasClicked_ = false;
    bool isDoubleClick_ = false;
    TimeStamp lastClickTimeStamp_;
    TimeStamp penultimateClickTimeStamp_;
    float paragraphWidth_ = 0.0f;

    std::queue<int32_t> deleteBackwardOperations_;
    std::queue<int32_t> deleteForwardOperations_;
    std::queue<SourceAndValueInfo> insertValueOperations_;
    std::queue<InputOperation> inputOperations_;
    bool leftMouseCanMove_ = false;
    bool isLongPress_ = false;
    bool isEdit_ = false;
    RefPtr<ContentController> contentController_;
    RefPtr<TextSelectController> selectController_;
    RefPtr<NG::UINode> unitNode_;
    RefPtr<TextInputResponseArea> responseArea_;
    RefPtr<TextInputResponseArea> cleanNodeResponseArea_;
    std::string lastAutoFillPasswordTextValue_;
    bool isSupportCameraInput_ = false;
    std::function<void()> processOverlayDelayTask_;
    std::function<void(bool)> isFocusActiveUpdateEvent_;
    FocuseIndex focusIndex_ = FocuseIndex::TEXT;
    bool isTouchCaret_ = false;
    bool needSelectAll_ = false;
    bool isModifyDone_ = false;
    bool initTextRect_ = false;
    bool colorModeChange_ = false;
    Offset clickLocation_;
    bool isKeyboardClosedByUser_ = false;
    bool isFillRequestFinish_ = false;
    bool keyboardAvoidance_ = false;
    bool hasMousePressed_ = false;
    bool showCountBorderStyle_ = false;
    RefPtr<TextFieldSelectOverlay> selectOverlay_;
    OffsetF movingCaretOffset_;
    std::string autoFillUserName_;
    std::string autoFillNewPassword_;
    bool autoFillOtherAccount_ = false;
    std::unordered_map<std::string, std::variant<std::string, bool, int32_t>> fillContentMap_;

    bool textInputBlurOnSubmit_ = true;
    bool textAreaBlurOnSubmit_ = false;
    bool isDetachFromMainTree_ = false;

    bool haveTextFadeoutCapacity_ = false;

    bool isFocusBGColorSet_ = false;
    bool isFocusTextColorSet_ = false;
    bool isFocusPlaceholderColorSet_ = false;
    Dimension previewUnderlineWidth_ = 2.0_vp;
    bool hasSupportedPreviewText_ = false;
    bool hasPreviewText_ = false;
    std::queue<PreviewTextInfo> previewTextOperation_;
    int32_t previewTextStart_ = -1;
    int32_t previewTextEnd_ = -1;
    PreviewRange lastCursorRange_ = {};
    bool showKeyBoardOnFocus_ = true;
    bool isTextSelectionMenuShow_ = true;
    bool isMoveCaretAnywhere_ = false;
    bool isTouchPreviewText_ = false;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_TEXT_FIELD_PATTERN_H
