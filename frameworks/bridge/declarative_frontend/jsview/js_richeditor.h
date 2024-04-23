/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_RICHEDITOR_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_RICHEDITOR_H

#include "core/components_ng/pattern/rich_editor/rich_editor_event_hub.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_model.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_container_base.h"

namespace OHOS::Ace::Framework {
class JSRichEditor : public JSContainerBase {
public:
    static void Create(const JSCallbackInfo& info);
    static void JSBind(BindingTarget globalObj);
    static void SetOnReady(const JSCallbackInfo& args);
    static void SetOnSelect(const JSCallbackInfo& args);
    static void SetOnSelectionChange(const JSCallbackInfo& args);
    static void SetAboutToIMEInput(const JSCallbackInfo& args);
    static void SetOnIMEInputComplete(const JSCallbackInfo& args);
    static void SetAboutToDelete(const JSCallbackInfo& args);
    static void SetOnDeleteComplete(const JSCallbackInfo& args);
    static void SetCustomKeyboard(const JSCallbackInfo& args);
    static JSRef<JSVal> CreateJsAboutToIMEInputObj(const NG::RichEditorInsertValue& insertValue);
    static JSRef<JSVal> CreateJsOnIMEInputComplete(const NG::RichEditorAbstractSpanResult& textSpanResult);
    static JSRef<JSVal> CreateJsAboutToDelet(const NG::RichEditorDeleteValue& deleteValue);
    static void SetChangeTextSpans(
        JSRef<JSArray>& jsArray, const std::list<NG::RichEditorAbstractSpanResult>& spanList);
    static JSRef<JSVal> CreateJsOnWillChange(const NG::RichEditorChangeValue& changeValue);
    static JSRef<JSVal> CreateJsOnDidChange(const std::list<NG::RichEditorAbstractSpanResult>& spanList);
    static void JsClip(const JSCallbackInfo& info);
    static void JsFocusable(const JSCallbackInfo& info);
    static void SetCopyOptions(const JSCallbackInfo& info);
    static void BindSelectionMenu(const JSCallbackInfo& info);
    static void SetOnPaste(const JSCallbackInfo& info);
    static void SetPlaceholder(const JSCallbackInfo& info);
    static void JsEnableDataDetector(const JSCallbackInfo& info);
    static void JsDataDetectorConfig(const JSCallbackInfo& info);
    static void SetOnWillChange(const JSCallbackInfo& info);
    static void SetOnDidChange(const JSCallbackInfo& info);
    static void SetOnCut(const JSCallbackInfo& info);
    static void SetOnCopy(const JSCallbackInfo& info);
    static JSRef<JSVal> CreateJSTextCommonEvent(NG::TextCommonEvent& event);
    static JSRef<JSObject> CreateJSSpanResultObject(const ResultObject& resultObject);
    static JSRef<JSVal> CreateJSSelection(const SelectionInfo& selectInfo);
    static JSRef<JSVal> CreateJSSelectionRange(const SelectionRangeInfo& selectRange);
    static JSRef<JSObject> CreateJSTextStyleResult(const TextStyleResult& textStyleResult);
    static JSRef<JSObject> CreateJSParagraphStyle(const TextStyleResult& textStyleResult);
    static JSRef<JSObject> CreateJSSymbolSpanStyleResult(const SymbolSpanStyle& symbolSpanStyle);
    static JSRef<JSObject> CreateJSValueResource(const RefPtr<ResourceObject>& valueResource);
    static JSRef<JSObject> CreateJSLayoutStyle(const ImageStyleResult& imageStyleResult);
    static JSRef<JSObject> CreateJSImageStyleResult(const ImageStyleResult& imageStyleResult);
    static JSRef<JSObject> CreateParagraphStyleResult(const ParagraphInfo& info);
    static void SetCaretColor(const JSCallbackInfo& info);
    static void SetSelectedBackgroundColor(const JSCallbackInfo& info);
    static void SetOnEditingChange(const JSCallbackInfo& args);
    static void SetEnterKeyType(const JSCallbackInfo& info);
    static void CreateJsRichEditorCommonEvent(const JSCallbackInfo& info);
    static void SetOnSubmit(const JSCallbackInfo& info);
    static Local<JSValueRef> JsKeepEditableState(panda::JsiRuntimeCallInfo* info);
    static std::optional<NG::BorderRadiusProperty> ParseBorderRadiusAttr(JsiRef<JSVal> args);
    static std::optional<NG::MarginProperty> ParseMarginAttr(JsiRef<JSVal> marginAttr);
    static CalcDimension ParseLengthMetrics(const JSRef<JSObject>& obj);
private:
    static void CreateTextStyleObj(JSRef<JSObject>& textStyleObj, const NG::RichEditorAbstractSpanResult& spanResult);
    static void CreateImageStyleObj(JSRef<JSObject>& imageStyleObj, JSRef<JSObject>& spanResultObj,
        const NG::RichEditorAbstractSpanResult& spanResult);
    static void ParseUserGesture(
        const JSCallbackInfo& args, UserGestureOptions& gestureOption, const std::string& spanType);
    static void ParseJsFont(const JSRef<JSObject>& fontObject, Font& font);
};

class JSRichEditorController final : public Referenced {
public:
    JSRichEditorController() = default;
    ~JSRichEditorController() override = default;

    static void JSBind(BindingTarget globalObj);

    static void Constructor(const JSCallbackInfo& args)
    {
        auto controller = Referenced::MakeRefPtr<JSRichEditorController>();
        controller->IncRefCount();
        args.SetReturnValue(Referenced::RawPtr(controller));
    }

    static void Destructor(JSRichEditorController* controller)
    {
        if (controller != nullptr) {
            controller->DecRefCount();
        }
    }

    void SetController(const RefPtr<RichEditorControllerBase>& controller)
    {
        controllerWeak_ = controller;
    }
    void AddImageSpan(const JSCallbackInfo& args);
    void AddTextSpan(const JSCallbackInfo& args);
    void AddSymbolSpan(const JSCallbackInfo& args);
    void AddPlaceholderSpan(const JSCallbackInfo& args);
    void ParseOptions(const JSCallbackInfo& args, SpanOptionBase& placeholderSpan);
    void DeleteSpans(const JSCallbackInfo& args);
    ImageSpanAttribute ParseJsImageSpanAttribute(JSRef<JSObject> imageAttribute);
    void ParseJsTextStyle(
        const JSRef<JSObject>& styleObject, TextStyle& style, struct UpdateSpanStyle& updateSpanStyle);
    void ParseJsLineHeightLetterSpacingTextStyle(const JSRef<JSObject>& styleObject, TextStyle& style,
        struct UpdateSpanStyle& updateSpanStyle, bool isSupportPercent = true);
    void ParseJsFontFeatureTextStyle(const JSRef<JSObject>& styleObject, TextStyle& style,
        struct UpdateSpanStyle& updateSpanStyle);
    void ParseJsSymbolSpanStyle(
        const JSRef<JSObject>& styleObject, TextStyle& style, struct UpdateSpanStyle& updateSpanStyle);
    ImageSpanOptions CreateJsImageOptions(const JSCallbackInfo& args);
    void SetCaretOffset(const JSCallbackInfo& args);
    void GetCaretOffset(const JSCallbackInfo& args);
    void UpdateSpanStyle(const JSCallbackInfo& info);
    void UpdateParagraphStyle(const JSCallbackInfo& info);
    void GetSpansInfo(const JSCallbackInfo& args);
    void GetParagraphsInfo(const JSCallbackInfo& args);
    void GetTypingStyle(const JSCallbackInfo& info);
    void SetTypingStyle(const JSCallbackInfo& info);
    void CloseSelectionMenu();
    void SetSelection(const JSCallbackInfo& args);
    void GetSelection(const JSCallbackInfo& args);
    void IsEditing(const JSCallbackInfo& args);
    void SetInstanceId(int32_t id)
    {
        instanceId_ = id;
    }
    void StopEditing();

private:
    int32_t instanceId_ = INSTANCE_ID_UNDEFINED;
    bool ParseParagraphStyle(const JSRef<JSObject>& styleObject, struct UpdateParagraphStyle& style);
    bool IsPixelMap(const JSRef<JSVal>& jsValue);
    bool IsDrawable(const JSRef<JSVal>& jsValue);
    void ParseTextDecoration(
        const JSRef<JSObject>& styleObject, TextStyle& style, struct UpdateSpanStyle& updateSpanStyle);
    void ParseTextShadow(
        const JSRef<JSObject>& styleObject, TextStyle& style, struct UpdateSpanStyle& updateSpanStyle);
    void ParseJsSelectionOptions(const JSCallbackInfo& args, std::optional<SelectionOptions>& options);
    static JSRef<JSVal> CreateJSSpansInfo(const SelectionInfo& info);
    static JSRef<JSVal> CreateJSParagraphsInfo(const std::vector<ParagraphInfo>& info);
    static JSRef<JSObject> CreateTypingStyleResult(const struct UpdateSpanStyle& typingStyle);
    static void ParseWordBreakParagraphStyle(const JSRef<JSObject>& styleObject, struct UpdateParagraphStyle& style);
    static void ParseTextAlignParagraphStyle(const JSRef<JSObject>& styleObject, struct UpdateParagraphStyle& style);
    bool CheckImageSource(std::string assetSrc);

    WeakPtr<RichEditorControllerBase> controllerWeak_;
    ACE_DISALLOW_COPY_AND_MOVE(JSRichEditorController);
    struct UpdateSpanStyle updateSpanStyle_;
    struct UpdateSpanStyle typingStyle_;
    static void ParseLineBreakStrategyParagraphStyle(
        const JSRef<JSObject>& styleObject, struct UpdateParagraphStyle& style);
};
} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_RICHEDITOR_H
