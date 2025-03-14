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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_WEB_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_WEB_PATTERN_H

#include <optional>
#include <string>
#include <tuple>
#include <utility>

#include "base/memory/referenced.h"
#include "base/thread/cancelable_callback.h"
#include "base/utils/utils.h"
#include "base/geometry/axis.h"
#include "base/web/webview/ohos_nweb/include/nweb_handler.h"
#include "core/common/udmf/unified_data.h"
#include "core/components/dialog/dialog_properties.h"
#include "core/components/dialog/dialog_theme.h"
#include "core/components/web/web_event.h"
#include "core/components/web/web_property.h"
#include "core/components_ng/gestures/recognizers/pan_recognizer.h"
#include "core/components_ng/manager/select_overlay/select_overlay_manager.h"
#include "core/components_ng/manager/select_overlay/select_overlay_proxy.h"
#include "core/components_ng/manager/select_overlay/selection_host.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/scrollable/nestable_scroll_container.h"
#include "core/components_ng/pattern/web/touch_event_listener.h"
#include "core/components_ng/pattern/web/web_accessibility_node.h"
#include "core/components_ng/pattern/web/web_accessibility_property.h"
#include "core/components_ng/pattern/web/web_context_select_overlay.h"
#include "core/components_ng/pattern/web/web_event_hub.h"
#include "core/components_ng/pattern/web/web_layout_algorithm.h"
#include "core/components_ng/pattern/web/web_paint_property.h"
#include "core/components_ng/pattern/web/web_pattern_property.h"
#include "core/components_ng/pattern/web/web_paint_method.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/adapter/rosen_render_context.h"
#include "core/components_ng/render/render_surface.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/gestures/pinch_gesture.h"

namespace OHOS::Ace {
class WebDelegateObserver;
class ImageAnalyzerManager;
}

namespace OHOS::Ace::NG {
namespace {

struct MouseClickInfo {
    double x = -1;
    double y = -1;
    TimeStamp start;
};

struct ReachEdge {
    bool atStart = false;
    bool atEnd = false;
};

#ifdef OHOS_STANDARD_SYSTEM
struct TouchInfo {
    double x = -1;
    double y = -1;
    int32_t id = -1;
};

struct TouchHandleState {
    int32_t id = -1;
    int32_t x = -1;
    int32_t y = -1;
    int32_t edge_height = 0;
};

enum WebOverlayType { INSERT_OVERLAY, SELECTION_OVERLAY, INVALID_OVERLAY };
#endif
} // namespace

enum class WebInfoType : int32_t {
    TYPE_MOBILE,
    TYPE_TABLET,
    TYPE_2IN1,
    TYPE_UNKNOWN
};

class WebPattern : public NestableScrollContainer, public TextBase {
    DECLARE_ACE_TYPE(WebPattern, NestableScrollContainer, TextBase);

public:
    using SetWebIdCallback = std::function<void(int32_t)>;
    using SetHapPathCallback = std::function<void(const std::string&)>;
    using JsProxyCallback = std::function<void()>;
    using OnControllerAttachedCallback = std::function<void()>;
    using PermissionClipboardCallback = std::function<void(const std::shared_ptr<BaseEventInfo>&)>;
    using OnOpenAppLinkCallback = std::function<void(const std::shared_ptr<BaseEventInfo>&)>;
    using DefaultFileSelectorShowCallback = std::function<void(const std::shared_ptr<BaseEventInfo>&)>;
    WebPattern();
    WebPattern(const std::string& webSrc, const RefPtr<WebController>& webController,
               RenderMode type = RenderMode::ASYNC_RENDER, bool incognitoMode = false);
    WebPattern(const std::string& webSrc, const SetWebIdCallback& setWebIdCallback,
               RenderMode type = RenderMode::ASYNC_RENDER, bool incognitoMode = false);

    ~WebPattern() override;

    enum class VkState {
        VK_NONE,
        VK_SHOW,
        VK_HIDE
    };

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override;

    bool IsAtomicNode() const override
    {
        return true;
    }

    bool NeedSoftKeyboard() const override;

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<WebEventHub>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<WebAccessibilityProperty>();
    }


    void OnModifyDone() override;

    void SetWebSrc(const std::string& webSrc)
    {
        if (webSrc_ != webSrc_) {
            OnWebSrcUpdate();
            webSrc_ = webSrc;
        }
        if (webPaintProperty_) {
            webPaintProperty_->SetWebPaintData(webSrc);
        }
    }

    const std::optional<std::string>& GetWebSrc() const
    {
        return webSrc_;
    }

    void SetPopup(bool popup)
    {
        isPopup_ = popup;
    }

    void SetParentNWebId(int32_t parentNWebId)
    {
        parentNWebId_ = parentNWebId;
    }

    void SetWebData(const std::string& webData)
    {
        if (webData_ != webData) {
            webData_ = webData;
            OnWebDataUpdate();
        }
        if (webPaintProperty_) {
            webPaintProperty_->SetWebPaintData(webData);
        }
    }

    const std::optional<std::string>& GetWebData() const
    {
        return webData_;
    }

    void SetCustomScheme(const std::string& scheme)
    {
        customScheme_ = scheme;
    }

    const std::optional<std::string>& GetCustomScheme() const
    {
        return customScheme_;
    }

    void SetWebController(const RefPtr<WebController>& webController)
    {
        webController_ = webController;
    }

    RefPtr<WebController> GetWebController() const
    {
        return webController_;
    }

    void SetSetWebIdCallback(SetWebIdCallback&& SetIdCallback)
    {
        setWebIdCallback_ = std::move(SetIdCallback);
    }

    SetWebIdCallback GetSetWebIdCallback() const
    {
        return setWebIdCallback_;
    }

    void SetPermissionClipboardCallback(PermissionClipboardCallback&& Callback)
    {
        permissionClipboardCallback_ = std::move(Callback);
    }

    void SetDefaultFileSelectorShowCallback(DefaultFileSelectorShowCallback&& Callback)
    {
        defaultFileSelectorShowCallback_ = std::move(Callback);
    }

    DefaultFileSelectorShowCallback GetDefaultFileSelectorShowCallback()
    {
        return defaultFileSelectorShowCallback_;
    }

    PermissionClipboardCallback GetPermissionClipboardCallback() const
    {
        return permissionClipboardCallback_;
    }

    void SetOnOpenAppLinkCallback(OnOpenAppLinkCallback&& callback)
    {
        onOpenAppLinkCallback_ = std::move(callback);
    }

    OnOpenAppLinkCallback GetOnOpenAppLinkCallback() const
    {
        return onOpenAppLinkCallback_;
    }

    void SetRenderMode(RenderMode renderMode)
    {
        renderMode_ = renderMode;
    }

    RenderMode GetRenderMode()
    {
        return renderMode_;
    }

    void SetIncognitoMode(bool incognitoMode)
    {
        incognitoMode_ = incognitoMode;
    }

    bool GetIncognitoMode() const
    {
        return incognitoMode_;
    }

    void SetOnControllerAttachedCallback(OnControllerAttachedCallback&& callback)
    {
        onControllerAttachedCallback_ = std::move(callback);
    }

    OnControllerAttachedCallback GetOnControllerAttachedCallback()
    {
        return onControllerAttachedCallback_;
    }

    void SetSetHapPathCallback(SetHapPathCallback&& callback)
    {
        setHapPathCallback_ = std::move(callback);
    }

    SetHapPathCallback GetSetHapPathCallback() const
    {
        return setHapPathCallback_;
    }

    void SetJsProxyCallback(JsProxyCallback&& jsProxyCallback)
    {
        jsProxyCallback_ = std::move(jsProxyCallback);
    }

    void CallJsProxyCallback()
    {
        if (jsProxyCallback_) {
            jsProxyCallback_();
        }
    }

    RefPtr<WebEventHub> GetWebEventHub()
    {
        return GetEventHub<WebEventHub>();
    }

    FocusPattern GetFocusPattern() const override
    {
        FocusPattern focusPattern = { FocusType::NODE, true, FocusStyleType::FORCE_NONE };
        focusPattern.SetIsFocusActiveWhenFocused(true);
        return focusPattern;
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        if (!webPaintProperty_) {
            webPaintProperty_ = MakeRefPtr<WebPaintProperty>();
            if (!webPaintProperty_) {
            }
        }
        return webPaintProperty_;
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<WebLayoutAlgorithm>();
    }

    bool BetweenSelectedPosition(const Offset& globalOffset) override
    {
        return false;
    }

    int32_t GetDragRecordSize() override
    {
        return 1;
    }

    /**
     *  NestableScrollContainer implementations
     */
    Axis GetAxis() const override
    {
        return axis_;
    }
    ScrollResult HandleScroll(float offset, int32_t source, NestedState state, float velocity = 0.f) override;
    ScrollResult HandleScroll(RefPtr<NestableScrollContainer> parent, float offset, int32_t source, NestedState state);
    bool HandleScrollVelocity(float velocity) override;
    bool HandleScrollVelocity(RefPtr<NestableScrollContainer> parent, float velocity);
    void OnScrollStartRecursive(float position, float velocity = 0.f) override;
    void OnScrollStartRecursive(std::vector<float> positions);
    void OnScrollEndRecursive(const std::optional<float>& velocity) override;
    void OnAttachToBuilderNode(NodeStatus nodeStatus) override;
    Axis GetParentAxis();
    RefPtr<NestableScrollContainer> SearchParent() override;
    RefPtr<NestableScrollContainer> SearchParent(Axis scrollAxis);
    /**
     *  End of NestableScrollContainer implementations
     */

    ACE_DEFINE_PROPERTY_GROUP(WebProperty, WebPatternProperty);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, JsEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MediaPlayGestureAccess, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, FileAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OnLineImageAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DomStorageAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ImageAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MixedMode, MixedModeContent);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ZoomAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, GeolocationAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, UserAgent, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, CacheMode, WebCacheMode);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OverviewModeAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, FileFromUrlAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DatabaseAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, TextZoomRatio, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebDebuggingAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, BackgroundColor, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, InitialScale, float);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, PinchSmoothModeEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MultiWindowAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, AllowWindowOpenMethod, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebCursiveFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebFantasyFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebFixedFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebSansSerifFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebSerifFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebStandardFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DefaultFixedFontSize, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DefaultFontSize, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DefaultTextEncodingFormat, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MinFontSize, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MinLogicalFontSize, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, BlockNetwork, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DarkMode, WebDarkMode);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ForceDarkAccess, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, AudioResumeInterval, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, AudioExclusive, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, HorizontalScrollBarAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, VerticalScrollBarAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ScrollBarColor, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OverScrollMode, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, CopyOptionMode, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MetaViewport, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, NativeEmbedModeEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, NativeEmbedRuleTag, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, NativeEmbedRuleType, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, TextAutosizing, bool);
    using NativeVideoPlayerConfigType = std::tuple<bool, bool>;
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, NativeVideoPlayerConfig, NativeVideoPlayerConfigType);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, SmoothDragResizeEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, SelectionMenuOptions, WebMenuOptionsParam);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OverlayScrollbarEnabled, bool);

    bool IsFocus() const
    {
        return isFocus_;
    }

    void RequestFullScreen();
    void ExitFullScreen();
    bool IsFullScreen() const
    {
        return isFullScreen_;
    }
    void UpdateClippedSelectionBounds(int32_t x, int32_t y, int32_t w, int32_t h);
    bool RunQuickMenu(std::shared_ptr<OHOS::NWeb::NWebQuickMenuParams> params,
        std::shared_ptr<OHOS::NWeb::NWebQuickMenuCallback> callback);
    void OnContextMenuShow(const std::shared_ptr<BaseEventInfo>& info);
    void OnContextMenuHide();
    void QuickMenuIsNeedNewAvoid(
        SelectOverlayInfo& selectInfo,
        std::shared_ptr<OHOS::NWeb::NWebQuickMenuParams> params,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> startHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> endHandle);
    RectF ComputeClippedSelectionBounds(
        std::shared_ptr<OHOS::NWeb::NWebQuickMenuParams> params,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> startHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> endHandle);
    void OnQuickMenuDismissed();
    void OnTouchSelectionChanged(std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> insertHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> startSelectionHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> endSelectionHandle);
    bool OnCursorChange(const OHOS::NWeb::CursorType& type, std::shared_ptr<OHOS::NWeb::NWebCursorInfo> info);
    void UpdateLocalCursorStyle(int32_t windowId, const OHOS::NWeb::CursorType& type);
    void UpdateCustomCursor(int32_t windowId, std::shared_ptr<OHOS::NWeb::NWebCursorInfo> info);
    std::shared_ptr<OHOS::Media::PixelMap> CreatePixelMapFromString(const std::string& filePath);
    void OnSelectPopupMenu(std::shared_ptr<OHOS::NWeb::NWebSelectPopupMenuParam> params,
        std::shared_ptr<OHOS::NWeb::NWebSelectPopupMenuCallback> callback);
    void OnDateTimeChooserPopup(
        std::shared_ptr<OHOS::NWeb::NWebDateTimeChooser> chooser,
        const std::vector<std::shared_ptr<OHOS::NWeb::NWebDateTimeSuggestion>>& suggestions,
        std::shared_ptr<NWeb::NWebDateTimeChooserCallback> callback);
    void OnDateTimeChooserClose();
    void OnShowAutofillPopup(const float offsetX, const float offsetY, const std::vector<std::string>& menu_items);
    void OnHideAutofillPopup();
    void UpdateTouchHandleForOverlay();
    bool IsSelectOverlayDragging()
    {
        return selectOverlayDragging_;
    }
    void SetSelectOverlayDragging(bool selectOverlayDragging)
    {
        selectOverlayDragging_ = selectOverlayDragging;
    }
    void UpdateLocale();
    void SetDrawRect(int32_t x, int32_t y, int32_t width, int32_t height, bool isNeedReset);
    void SetSelectPopupMenuShowing(bool showing)
    {
        selectPopupMenuShowing_ = showing;
    }
    void SetCurrentStartHandleDragging(bool isStartHandle)
    {
        isCurrentStartHandleDragging_ = isStartHandle;
    }
    void UpdateSelectHandleInfo();
    bool IsSelectHandleReverse();
    void OnCompleteSwapWithNewSize();
    void OnResizeNotWork();
    void UpdateOnFocusTextField(bool isFocus);
    bool OnBackPressed() override;
    bool OnBackPressedForFullScreen() const;
    void SetFullScreenExitHandler(const std::shared_ptr<FullScreenEnterEvent>& fullScreenExitHandler);
    bool NotifyStartDragTask();
    bool IsImageDrag();
    void UpdateJavaScriptOnDocumentStart();
    void UpdateJavaScriptOnDocumentEnd();
    void JavaScriptOnDocumentStart(const ScriptItems& scriptItems);
    void JavaScriptOnDocumentEnd(const ScriptItems& scriptItems);
    void SetTouchEventInfo(const TouchEvent& touchEvent,
        TouchEventInfo& touchEventInfo, const std::string& embdedId);
    DragRet GetDragAcceptableStatus();
    Offset GetDragOffset() const;
    void OnOverScrollFlingVelocity(float xVelocity, float yVelocity, bool isFling);
    void OnScrollState(bool scrollState);
    void SetLayoutMode(WebLayoutMode mode)
    {
        layoutMode_ = mode;
    }
    WebLayoutMode GetLayoutMode() const
    {
        return layoutMode_;
    }
    void OnRootLayerChanged(int width, int height);
    void ReleaseResizeHold();
    bool GetPendingSizeStatus();
    int GetRootLayerWidth() const
    {
        return rootLayerWidth_;
    }
    int GetRootLayerHeight() const
    {
        return rootLayerHeight_;
    }
    Size GetDrawSize() const
    {
        return drawSize_;
    }
    SizeF GetDragPixelMapSize() const;
    bool IsVirtualKeyBoardShow() const
    {
        return isVirtualKeyBoardShow_ == VkState::VK_SHOW;
    }
    bool FilterScrollEvent(const float x, const float y, const float xVelocity, const float yVelocity);
    RefPtr<WebAccessibilityNode> GetFocusedAccessibilityNode(int64_t accessibilityId, bool isAccessibilityFocus);
    RefPtr<WebAccessibilityNode> GetAccessibilityNodeById(int64_t accessibilityId);
    RefPtr<WebAccessibilityNode> GetAccessibilityNodeByFocusMove(int64_t accessibilityId, int32_t direction);
    void ExecuteAction(int64_t accessibilityId, AceAction action,
        const std::map<std::string, std::string>& actionArguments) const;
    void SetAccessibilityState(bool state);
    void UpdateFocusedAccessibilityId(int64_t accessibilityId = -1);
    void OnTooltip(const std::string& tooltip);
    bool IsDefaultFocusNodeExist();
    bool IsRootNeedExportTexture();
    std::vector<int8_t> GetWordSelection(const std::string& text, int8_t offset);
    bool Backward();
    void OnSelectionMenuOptionsUpdate(const WebMenuOptionsParam& webMenuOption);
    void NotifyForNextTouchEvent() override;
    void CloseKeyboard();
    void CreateOverlay(const RefPtr<OHOS::Ace::PixelMap>& pixelMap, int offsetX, int offsetY, int rectWidth,
        int rectHeight, int pointX, int pointY);
    void OnOverlayStateChanged(int offsetX, int offsetY, int rectWidth, int rectHeight);
    void OnTextSelected();
    void DestroyAnalyzerOverlay();
    WebInfoType GetWebInfoType();
    void RequestFocus();
    void SetCustomKeyboardBuilder(std::function<void()> customKeyboardBuilder)
    {
        customKeyboardBuilder_ = customKeyboardBuilder;
    }
    void AttachCustomKeyboard();
    void CloseCustomKeyboard();
    void KeyboardReDispatch(const std::shared_ptr<OHOS::NWeb::NWebKeyEvent>& event, bool isUsed);
    void OnAttachContext(PipelineContext *context) override;
    void OnDetachContext(PipelineContext *context) override;
    void SetUpdateInstanceIdCallback(std::function<void(int32_t)> &&callabck);
    Rosen::NodeId GetWebSurfaceNodeId() const
    {
        auto rosenRenderContext = AceType::DynamicCast<NG::RosenRenderContext>(renderContextForSurface_);
        CHECK_NULL_RETURN(rosenRenderContext, 0);
        auto rsNode = rosenRenderContext->GetRSNode();
        CHECK_NULL_RETURN(rsNode, 0);
        auto surfaceNodeId = rsNode->GetId();
        TAG_LOGD(AceLogTag::ACE_WEB, "Web surfaceNodeId is %{public}" PRIu64 "", surfaceNodeId);
        return surfaceNodeId;
    }

private:
    friend class WebContextSelectOverlay;
    void ShowContextSelectOverlay(const RectF& firstHandle, const RectF& secondHandle,
        TextResponseType responseType = TextResponseType::RIGHT_CLICK, bool handleReverse = false);
    void CloseContextSelectionMenu();
    RectF ComputeMouseClippedSelectionBounds(int32_t x, int32_t y, int32_t w, int32_t h);
    void RegistVirtualKeyBoardListener(const RefPtr<PipelineContext> &context);
    bool ProcessVirtualKeyBoard(int32_t width, int32_t height, double keyboard);
    void UpdateWebLayoutSize(int32_t width, int32_t height, bool isKeyboard);
    void UpdateLayoutAfterKeyboardShow(int32_t width, int32_t height, double keyboard, double oldWebHeight);
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    void OnRebuildFrame() override;

    void OnAttachToFrameNode() override;
    void OnDetachFromFrameNode(FrameNode* frameNode) override;
    void OnAttachToMainTree() override;
    void OnDetachFromMainTree() override;

    void OnWindowShow() override;
    void OnWindowHide() override;
    void OnWindowSizeChanged(int32_t width, int32_t height, WindowSizeChangeReason type) override;
    void OnInActive() override;
    void OnActive() override;
    void OnVisibleAreaChange(bool isVisible);
    void OnAreaChangedInner() override;
    void OnNotifyMemoryLevel(int32_t level) override;

    void OnWebSrcUpdate();
    void OnWebDataUpdate();
    void OnJsEnabledUpdate(bool value);
    void OnMediaPlayGestureAccessUpdate(bool value);
    void OnFileAccessEnabledUpdate(bool value);
    void OnOnLineImageAccessEnabledUpdate(bool value);
    void OnDomStorageAccessEnabledUpdate(bool value);
    void OnImageAccessEnabledUpdate(bool value);
    void OnMixedModeUpdate(MixedModeContent value);
    void OnZoomAccessEnabledUpdate(bool value);
    void OnGeolocationAccessEnabledUpdate(bool value);
    void OnUserAgentUpdate(const std::string& value);
    void OnCacheModeUpdate(WebCacheMode value);
    void OnOverviewModeAccessEnabledUpdate(bool value);
    void OnFileFromUrlAccessEnabledUpdate(bool value);
    void OnDatabaseAccessEnabledUpdate(bool value);
    void OnTextZoomRatioUpdate(int32_t value);
    void OnWebDebuggingAccessEnabledUpdate(bool value);
    void OnPinchSmoothModeEnabledUpdate(bool value);
    void OnBackgroundColorUpdate(int32_t value);
    void OnInitialScaleUpdate(float value);
    void OnMultiWindowAccessEnabledUpdate(bool value);
    void OnAllowWindowOpenMethodUpdate(bool value);
    void OnWebCursiveFontUpdate(const std::string& value);
    void OnWebFantasyFontUpdate(const std::string& value);
    void OnWebFixedFontUpdate(const std::string& value);
    void OnWebSerifFontUpdate(const std::string& value);
    void OnWebSansSerifFontUpdate(const std::string& value);
    void OnWebStandardFontUpdate(const std::string& value);
    void OnDefaultFixedFontSizeUpdate(int32_t value);
    void OnDefaultFontSizeUpdate(int32_t value);
    void OnDefaultTextEncodingFormatUpdate(const std::string& value);
    void OnMinFontSizeUpdate(int32_t value);
    void OnMinLogicalFontSizeUpdate(int32_t value);
    void OnBlockNetworkUpdate(bool value);
    void OnDarkModeUpdate(WebDarkMode mode);
    void OnForceDarkAccessUpdate(bool access);
    void OnAudioResumeIntervalUpdate(int32_t resumeInterval);
    void OnAudioExclusiveUpdate(bool audioExclusive);
    void OnHorizontalScrollBarAccessEnabledUpdate(bool value);
    void OnVerticalScrollBarAccessEnabledUpdate(bool value);
    void OnScrollBarColorUpdate(const std::string& value);
    void OnOverScrollModeUpdate(const int32_t value);
    void OnCopyOptionModeUpdate(const int32_t value);
    void OnMetaViewportUpdate(bool value);
    void OnNativeEmbedModeEnabledUpdate(bool value);
    void OnNativeEmbedRuleTagUpdate(const std::string& tag);
    void OnNativeEmbedRuleTypeUpdate(const std::string& type);
    void OnTextAutosizingUpdate(bool isTextAutosizing);
    void OnNativeVideoPlayerConfigUpdate(const std::tuple<bool, bool>& config);
    void WindowDrag(int32_t width, int32_t height);
    void OnSmoothDragResizeEnabledUpdate(bool value);
    void OnOverlayScrollbarEnabledUpdate(bool enable);
    int GetWebId();

    void InitEvent();
    void InitConfigChangeCallback(const RefPtr<PipelineContext>& context);
    void InitFeatureParam();
    void InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub);
    void InitMouseEvent(const RefPtr<InputEventHub>& inputHub);
    void InitHoverEvent(const RefPtr<InputEventHub>& inputHub);
    void InitCommonDragDropEvent(const RefPtr<GestureEventHub>& gestureHub);
    void InitWebEventHubDragDropStart(const RefPtr<WebEventHub>& eventHub);
    void InitWebEventHubDragDropEnd(const RefPtr<WebEventHub>& eventHub);
    void InitWebEventHubDragMove(const RefPtr<WebEventHub>& eventHub);
    void InitPanEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleFlingMove(const GestureEvent& event);
    void HandleDragMove(const GestureEvent& event);
    void InitDragEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleDragStart(int32_t x, int32_t y);
    void HandleDragEnd(int32_t x, int32_t y);
    void HandleDragCancel();
    void ClearDragData();
    bool GenerateDragDropInfo(NG::DragDropInfo& dragDropInfo);
    void HandleMouseEvent(MouseInfo& info);
    void WebOnMouseEvent(const MouseInfo& info);
    bool HandleDoubleClickEvent(const MouseInfo& info);
    void SendDoubleClickEvent(const MouseClickInfo& info);
    void InitFocusEvent(const RefPtr<FocusHub>& focusHub);
    void HandleFocusEvent();
    void HandleBlurEvent(const BlurReason& blurReason);
    bool HandleKeyEvent(const KeyEvent& keyEvent);
    bool WebOnKeyEvent(const KeyEvent& keyEvent);
    void WebRequestFocus();
    void ResetDragAction();
    void InitSlideUpdateListener();
    void CalculateHorizontalDrawRect(bool isNeedReset);
    void CalculateVerticalDrawRect(bool isNeedReset);
    void InitPinchEvent(const RefPtr<GestureEventHub>& gestureHub);
    bool CheckZoomStatus(const double& curScale);
    bool ZoomOutAndIn(const double& curScale, double& scale);
    void HandleScaleGestureChange(const GestureEvent& event);

    NG::DragDropInfo HandleOnDragStart(const RefPtr<OHOS::Ace::DragEvent>& info);
    void HandleOnDragEnter(const RefPtr<OHOS::Ace::DragEvent>& info);
    void HandleOnDropMove(const RefPtr<OHOS::Ace::DragEvent>& info);
    void HandleOnDragDrop(const RefPtr<OHOS::Ace::DragEvent>& info);
    void HandleOnDragDropFile(RefPtr<UnifiedData> aceData);
    void HandleOnDragDropLink(RefPtr<UnifiedData> aceData);
    void HandleOnDragLeave(int32_t x, int32_t y);
    void HandleOnDragEnd(int32_t x, int32_t y);
    void InitTouchEventListener();
    void UninitTouchEventListener();
    void DragDropSelectionMenu();
    void OnDragFileNameStart(const RefPtr<UnifiedData>& aceUnifiedData, const std::string& fileName);
    bool needRestoreMenuForDrag_ = false;
    int32_t dropX_ = 0;
    int32_t dropY_ = 0;
    int onDragMoveCnt = 0;
    bool isDragEndMenuShow_ = false;
    std::shared_ptr<OHOS::NWeb::NWebQuickMenuParams> dropParams_ = nullptr;
    std::shared_ptr<OHOS::NWeb::NWebQuickMenuCallback> menuCallback_ = nullptr;
    std::chrono::time_point<std::chrono::system_clock> firstMoveInTime;
    std::chrono::time_point<std::chrono::system_clock> preMoveInTime;
    std::chrono::time_point<std::chrono::system_clock> curMoveInTime;
    CancelableCallback<void()> timer_;
    int32_t duration_ = 100; // 100: 100ms
    void DoRepeat();
    void StartRepeatTimer();

    void HandleTouchDown(const TouchEventInfo& info, bool fromOverlay);

    void HandleTouchUp(const TouchEventInfo& info, bool fromOverlay);

    void HandleTouchMove(const TouchEventInfo& info, bool fromOverlay);

    void HandleTouchCancel(const TouchEventInfo& info);

    bool IsTouchHandleValid(std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> handle);
    bool IsTouchHandleShow(std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> handle);

    void SuggestionSelected(int32_t index);
#ifdef OHOS_STANDARD_SYSTEM
    WebOverlayType GetTouchHandleOverlayType(
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> insertHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> startSelectionHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> endSelectionHandle);
#endif
    void RegisterSelectOverlayCallback(SelectOverlayInfo& selectInfo,
        std::shared_ptr<OHOS::NWeb::NWebQuickMenuParams> params,
        std::shared_ptr<OHOS::NWeb::NWebQuickMenuCallback> callback);
    void RegisterSelectOverlayEvent(SelectOverlayInfo& selectInfo);
    void CloseSelectOverlay();
    RectF ComputeTouchHandleRect(std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> touchHandle);
    void DelTouchOverlayInfoByTouchId(int32_t touchId);
    std::optional<OffsetF> GetCoordinatePoint();
    static void InitSelectPopupMenuViewOption(const std::vector<RefPtr<FrameNode>>& options,
        const std::shared_ptr<OHOS::NWeb::NWebSelectPopupMenuCallback>& callback,
        const std::shared_ptr<OHOS::NWeb::NWebSelectPopupMenuParam>& params,
        const double& dipScale);
    static void InitSelectPopupMenuView(RefPtr<FrameNode>& menuWrapper,
        std::shared_ptr<OHOS::NWeb::NWebSelectPopupMenuCallback> callback,
        std::shared_ptr<OHOS::NWeb::NWebSelectPopupMenuParam> params,
        const double& dipScale);
    OffsetF GetSelectPopupPostion(std::shared_ptr<OHOS::NWeb::NWebSelectMenuBound> bound);
    void SetSelfAsParentOfWebCoreNode(std::shared_ptr<OHOS::NWeb::NWebAccessibilityNodeInfo> info) const;
    bool GetAccessibilityFocusRect(RectT<int32_t>& paintRect, int64_t accessibilityId) const;
    void SetTouchLocationInfo(const TouchEvent& touchEvent, const TouchLocationInfo& changedInfo,
        const TouchEventInfo& tempTouchInfo, TouchEventInfo& touchEventInfo);
    struct TouchInfo {
        float x = -1.0f;
        float y = -1.0f;
        int32_t id = -1;
    };
    static bool ParseTouchInfo(const TouchEventInfo& info, std::list<TouchInfo>& touchInfos);
    void InitEnhanceSurfaceFlag();
    void UpdateBackgroundColorRightNow(int32_t color);
    void UpdateContentOffset(const RefPtr<LayoutWrapper>& dirty);
    DialogProperties GetDialogProperties(const RefPtr<DialogTheme>& theme);
    bool ShowDateTimeDialog(std::shared_ptr<OHOS::NWeb::NWebDateTimeChooser> chooser,
        const std::vector<std::shared_ptr<OHOS::NWeb::NWebDateTimeSuggestion>>& suggestions,
        std::shared_ptr<NWeb::NWebDateTimeChooserCallback> callback);
    bool ShowTimeDialog(std::shared_ptr<OHOS::NWeb::NWebDateTimeChooser> chooser,
        const std::vector<std::shared_ptr<OHOS::NWeb::NWebDateTimeSuggestion>>& suggestions,
        std::shared_ptr<NWeb::NWebDateTimeChooserCallback> callback);
    bool ShowDateTimeSuggestionDialog(std::shared_ptr<OHOS::NWeb::NWebDateTimeChooser> chooser,
        const std::vector<std::shared_ptr<OHOS::NWeb::NWebDateTimeSuggestion>>& suggestions,
        std::shared_ptr<NWeb::NWebDateTimeChooserCallback> callback);
    void PostTaskToUI(const std::function<void()>&& task, const std::string& name) const;
    void OfflineMode();
    void OnOverScrollFlingVelocityHandler(float velocity, bool isFling);
    bool FilterScrollEventHandleOffset(const float offset);
    bool FilterScrollEventHandlevVlocity(const float velocity);
    void UpdateFlingReachEdgeState(const float value, bool status);
    void CalculateTooltipMargin(RefPtr<FrameNode>& textNode, MarginProperty& textMargin);
    void HandleShowTooltip(const std::string& tooltip, int64_t tooltipTimestamp);
    void ShowTooltip(const std::string& tooltip, int64_t tooltipTimestamp);
    void RegisterVisibleAreaChangeCallback(const RefPtr<PipelineContext> &context);
    bool CheckSafeAreaIsExpand();
    bool CheckSafeAreaKeyBoard();
    void SelectCancel() const;
    std::string GetSelectInfo() const;
    void UpdateRunQuickMenuSelectInfo(SelectOverlayInfo& selectInfo,
        std::shared_ptr<OHOS::NWeb::NWebQuickMenuParams> params,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> insertTouchHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> beginTouchHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> endTouchHandle);
    double GetNewScale(double& scale) const;
    void UpdateSlideOffset(bool isNeedReset = false);
    void ClearKeyEventByKeyCode(int32_t keyCode);
    void SetRotation(uint32_t rotation);
    Color GetSystemColor() const;
    void UpdateTransformHintChangedCallbackId(std::optional<int32_t> id)
    {
        transformHintChangedCallbackId_ = id;
    }

    bool HasTransformHintChangedCallbackId()
    {
        return transformHintChangedCallbackId_.has_value();
    }

    std::optional<std::string> webSrc_;
    std::optional<std::string> webData_;
    std::optional<std::string> customScheme_;
    RefPtr<WebController> webController_;
    std::optional<int32_t> transformHintChangedCallbackId_;
    uint32_t rotation_ = 0;
    SetWebIdCallback setWebIdCallback_ = nullptr;
    PermissionClipboardCallback permissionClipboardCallback_ = nullptr;
    OnOpenAppLinkCallback onOpenAppLinkCallback_ = nullptr;
    DefaultFileSelectorShowCallback defaultFileSelectorShowCallback_ = nullptr;
    RenderMode renderMode_;
    bool incognitoMode_ = false;
    SetHapPathCallback setHapPathCallback_ = nullptr;
    JsProxyCallback jsProxyCallback_ = nullptr;
    OnControllerAttachedCallback onControllerAttachedCallback_ = nullptr;
    RefPtr<RenderSurface> renderSurface_ = RenderSurface::Create();
    RefPtr<RenderContext> renderContextForSurface_;
    RefPtr<TouchEventImpl> touchEvent_;
    RefPtr<InputEvent> mouseEvent_;
    RefPtr<InputEvent> hoverEvent_;
    RefPtr<PanEvent> panEvent_ = nullptr;
    RefPtr<SelectOverlayProxy> selectOverlayProxy_ = nullptr;
    RefPtr<WebPaintProperty> webPaintProperty_ = nullptr;
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> insertHandle_ = nullptr;
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> startSelectionHandle_ = nullptr;
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> endSelectionHandle_ = nullptr;
    bool isQuickMenuMouseTrigger_ = false;
    float selectHotZone_ = 10.0f;
    RefPtr<DragEvent> dragEvent_;
    bool isUrlLoaded_ = false;
    std::queue<MouseClickInfo> doubleClickQueue_;
    bool isFullScreen_ = false;
    std::shared_ptr<FullScreenEnterEvent> fullScreenExitHandler_ = nullptr;
    bool needOnFocus_ = false;
    Size drawSize_;
    Size rootLayerChangeSize_;
    Size drawSizeCache_;
    Size areaChangeSize_;
    bool isNeedReDrawRect_ = false;
    bool needUpdateWeb_ = true;
    bool isFocus_ = false;
    VkState isVirtualKeyBoardShow_ { VkState::VK_NONE };
    bool isDragging_ = false;
    bool isReceivedArkDrag_ = false;
    bool isW3cDragEvent_ = false;
    bool isWindowShow_ = true;
    bool isActive_ = true;
    bool isEnhanceSurface_ = false;
    bool isAllowWindowOpenMethod_ = false;
    bool isShowAutofillPopup_ = false;
    OffsetF webOffset_;
    RefPtr<WebContextSelectOverlay> contextSelectOverlay_ = nullptr;
    RefPtr<WebContextMenuParam> contextMenuParam_ = nullptr;
    RefPtr<ContextMenuResult> contextMenuResult_ = nullptr;
    RectF selectArea_;
    std::shared_ptr<OHOS::NWeb::NWebQuickMenuCallback> quickMenuCallback_ = nullptr;
    SelectMenuInfo selectMenuInfo_;
    bool selectOverlayDragging_ = false;
    bool selectPopupMenuShowing_ = false;
    bool isCurrentStartHandleDragging_ = false;
    std::list<TouchInfo> touchOverlayInfo_;
    bool isPopup_ = false;
    int32_t tooltipTextId_ = -1;
    int32_t tooltipId_ = -1;
    int32_t mouseHoveredX_ = -1;
    int32_t mouseHoveredY_ = -1;
    int64_t tooltipTimestamp_ = -1;
    int32_t parentNWebId_ = -1;
    bool isInWindowDrag_ = false;
    bool isWaiting_ = false;
    bool isDisableDrag_ = false;
    bool isMouseEvent_ = false;
    bool isVisible_ = true;
    bool isVisibleActiveEnable_ = true;
    bool isMemoryLevelEnable_ = true;
    OffsetF fitContentOffset_;
    bool isFirstFlingScrollVelocity_ = true;
    bool isNeedUpdateScrollAxis_ = true;
    bool isScrollStarted_ = false;
    WebLayoutMode layoutMode_ = WebLayoutMode::NONE;
    bool scrollState_ = false;
    Axis axis_ = Axis::FREE;
    Axis syncAxis_ = Axis::NONE;
    Axis expectedScrollAxis_ = Axis::FREE;
    int32_t rootLayerWidth_ = 0;
    int32_t rootLayerHeight_ = 0;
    int32_t drawRectWidth_ = 0;
    int32_t drawRectHeight_ = 0;
    std::unordered_map<Axis, WeakPtr<NestableScrollContainer>> parentsMap_;
    RefPtr<WebDelegate> delegate_;
    RefPtr<WebDelegateObserver> observer_;
    std::optional<ScriptItems> onDocumentStartScriptItems_;
    std::optional<ScriptItems> onDocumentEndScriptItems_;
    bool isOfflineMode_ = false;
    bool isAttachedToMainTree_ = false;
    ACE_DISALLOW_COPY_AND_MOVE(WebPattern);
    bool accessibilityState_ = false;
    RefPtr<WebAccessibilityNode> webAccessibilityNode_;
    TouchEventInfo touchEventInfo_{"touchEvent"};
    std::vector<TouchEventInfo> touchEventInfoList_ {};
    bool isParentReachEdge_ = false;
    ReachEdge isFlingReachEdge_ = { false, false };
    RefPtr<PinchGesture> pinchGesture_ = nullptr;
    std::queue<TouchEventInfo> touchEventQueue_;
    std::vector<NG::MenuOptionsParam> menuOptionParam_ {};
    std::list<KeyEvent> webKeyEvent_ {};
    double startPinchScale_ = -1.0;
    double preScale_ = -1.0;
    double pageScale_ = 1.0;
    int32_t pinchIndex_ = 0;
    bool zoomOutSwitch_ = false;
    bool isTouchUpEvent_ = false;
    int32_t zoomStatus_ = 0;
    int32_t zoomErrorCount_ = 0;
    std::shared_ptr<ImageAnalyzerManager> imageAnalyzerManager_ = nullptr;
    bool overlayCreating_ = false;
    RefPtr<OverlayManager> keyboardOverlay_;
    std::function<void()> customKeyboardBuilder_ = nullptr;
    std::function<void(int32_t)> updateInstanceIdCallback_;
    std::shared_ptr<TouchEventListener> touchEventListener_ = nullptr;
    double lastKeyboardHeight_ = 0.0;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_WEB_PATTERN_H
