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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_PIPELINE_NG_CONTEXT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_PIPELINE_NG_CONTEXT_H

#include <cstdint>
#include <functional>
#include <list>
#include <unordered_map>
#include <utility>

#include "interfaces/inner_api/ace/arkui_rect.h"

#include "base/geometry/ng/rect_t.h"
#include "base/log/frame_info.h"
#include "base/log/frame_report.h"
#include "base/memory/referenced.h"
#include "base/utils/device_config.h"
#include "base/view_data/view_data_wrap.h"
#include "core/accessibility/accessibility_manager_ng.h"
#include "core/common/frontend.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/gestures/recognizers/gesture_recognizer.h"
#include "core/components_ng/manager/drag_drop/drag_drop_manager.h"
#include "core/components_ng/manager/frame_rate/frame_rate_manager.h"
#include "core/components_ng/manager/full_screen/full_screen_manager.h"
#include "core/components_ng/manager/post_event/post_event_manager.h"
#include "core/components_ng/manager/privacy_sensitive/privacy_sensitive_manager.h"
#include "core/components_ng/manager/safe_area/safe_area_manager.h"
#include "core/components_ng/manager/navigation/navigation_manager.h"
#include "core/components_ng/manager/select_overlay/select_overlay_manager.h"
#include "core/components_ng/manager/shared_overlay/shared_overlay_manager.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#ifdef WINDOW_SCENE_SUPPORTED
#include "core/components_ng/pattern/ui_extension/ui_extension_manager.h"
#endif
#include "core/components_ng/manager/focus/focus_manager.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/stage/stage_manager.h"
#include "core/components_ng/pattern/web/itouch_event_callback.h"
#include "core/components_ng/property/safe_area_insets.h"
#include "core/event/touch_event.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {

using VsyncCallbackFun = std::function<void()>;
using FrameCallbackFunc = std::function<void(uint64_t nanoTimestamp)>;

class ACE_FORCE_EXPORT PipelineContext : public PipelineBase {
    DECLARE_ACE_TYPE(NG::PipelineContext, PipelineBase);

public:
    using SurfaceChangedCallbackMap =
        std::unordered_map<int32_t, std::function<void(int32_t, int32_t, int32_t, int32_t, WindowSizeChangeReason)>>;
    using SurfacePositionChangedCallbackMap = std::unordered_map<int32_t, std::function<void(int32_t, int32_t)>>;
    using FoldStatusChangedCallbackMap = std::unordered_map<int32_t, std::function<void(FoldStatus)>>;
    using FoldDisplayModeChangedCallbackMap = std::unordered_map<int32_t, std::function<void(FoldDisplayMode)>>;
    using TransformHintChangedCallbackMap = std::unordered_map<int32_t, std::function<void(uint32_t)>>;
    using PredictTask = std::function<void(int64_t, bool)>;
    PipelineContext(std::shared_ptr<Window> window, RefPtr<TaskExecutor> taskExecutor,
        RefPtr<AssetManager> assetManager, RefPtr<PlatformResRegister> platformResRegister,
        const RefPtr<Frontend>& frontend, int32_t instanceId);
    PipelineContext(std::shared_ptr<Window> window, RefPtr<TaskExecutor> taskExecutor,
        RefPtr<AssetManager> assetManager, const RefPtr<Frontend>& frontend, int32_t instanceId);
    PipelineContext() = default;

    ~PipelineContext() override = default;

    static RefPtr<PipelineContext> GetCurrentContext();

    static RefPtr<PipelineContext> GetCurrentContextSafely();

    static RefPtr<PipelineContext> GetCurrentContextSafelyWithCheck();

    static PipelineContext* GetCurrentContextPtrSafely();
    
    static PipelineContext* GetCurrentContextPtrSafelyWithCheck();


    static RefPtr<PipelineContext> GetMainPipelineContext();

    static RefPtr<PipelineContext> GetContextByContainerId(int32_t containerId);

    static float GetCurrentRootWidth();

    static float GetCurrentRootHeight();

    void SetupRootElement() override;

    void SetupSubRootElement();

    bool NeedSoftKeyboard() override;

    void SetOnWindowFocused(const std::function<void()>& callback) override
    {
        focusOnNodeCallback_ = callback;
    }

    const std::function<void()>& GetWindowFocusCallback() const
    {
        return focusOnNodeCallback_;
    }

    const RefPtr<FrameNode>& GetRootElement() const
    {
        return rootNode_;
    }

    void AddKeyFrame(float fraction, const RefPtr<Curve>& curve, const std::function<void()>& propertyCallback) override
    {}

    void AddKeyFrame(float fraction, const std::function<void()>& propertyCallback) override {}

    // add schedule task and return the unique mark id.
    uint32_t AddScheduleTask(const RefPtr<ScheduleTask>& task) override;

    // remove schedule task by id.
    void RemoveScheduleTask(uint32_t id) override;

    void OnTouchEvent(const TouchEvent& point, const RefPtr<NG::FrameNode>& node, bool isSubPipe = false) override;

    void OnMouseEvent(const MouseEvent& event, const RefPtr<NG::FrameNode>& node) override;

    void OnAxisEvent(const AxisEvent& event, const RefPtr<NG::FrameNode>& node) override;

    // Called by view when touch event received.
    void OnTouchEvent(const TouchEvent& point, bool isSubPipe = false) override;

    // Called by container when key event received.
    // if return false, then this event needs platform to handle it.
    bool OnKeyEvent(const KeyEvent& event) override;

    // ReDispatch KeyEvent from Web process.
    void ReDispatch(KeyEvent& keyEvent);

    // Called by view when mouse event received.
    void OnMouseEvent(const MouseEvent& event) override;

    // Do mouse event actively.
    void FlushMouseEvent();

    // Called by view when axis event received.
    void OnAxisEvent(const AxisEvent& event) override;

    // Called by container when rotation event received.
    // if return false, then this event needs platform to handle it.
    bool OnRotationEvent(const RotationEvent& event) const override
    {
        return false;
    }

    void OnDragEvent(const PointerEvent& pointerEvent, DragEventAction action) override;

    // Called by view when idle event.
    void OnIdle(int64_t deadline) override;

    void SetBuildAfterCallback(const std::function<void()>& callback) override
    {
        buildFinishCallbacks_.emplace_back(callback);
    }

    void SaveExplicitAnimationOption(const AnimationOption& option) override {}

    void CreateExplicitAnimator(const std::function<void()>& onFinishEvent) override {}

    void ClearExplicitAnimationOption() override {}

    AnimationOption GetExplicitAnimationOption() const override
    {
        return {};
    }

    void AddOnAreaChangeNode(int32_t nodeId);

    void RemoveOnAreaChangeNode(int32_t nodeId);

    void HandleOnAreaChangeEvent(uint64_t nanoTimestamp);

    // Just register notification, no need to update callback.
    void AddVisibleAreaChangeNode(const int32_t nodeId);

    void AddVisibleAreaChangeNode(const RefPtr<FrameNode>& node,
        const std::vector<double>& ratio, const VisibleRatioCallback& callback, bool isUserCallback = true);
    void RemoveVisibleAreaChangeNode(int32_t nodeId);

    void HandleVisibleAreaChangeEvent();

    void HandleSubwindow(bool isShow);

    void Destroy() override;

    void OnShow() override;

    void OnHide() override;

    void WindowFocus(bool isFocus) override;

    void ContainerModalUnFocus() override;

    void ShowContainerTitle(bool isShow, bool hasDeco = true, bool needUpdate = false) override;

    void SetAppBgColor(const Color& color) override;

    void SetAppTitle(const std::string& title) override;

    void SetAppIcon(const RefPtr<PixelMap>& icon) override;

    void OnSurfaceChanged(int32_t width, int32_t height,
        WindowSizeChangeReason type = WindowSizeChangeReason::UNDEFINED,
        const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr) override;

    void OnLayoutCompleted(const std::string& componentId);
    void OnDrawCompleted(const std::string& componentId);

    void OnSurfacePositionChanged(int32_t posX, int32_t posY) override;

    void OnSurfaceDensityChanged(double density) override;

    void OnSystemBarHeightChanged(double statusBar, double navigationBar) override {}

    void OnSurfaceDestroyed() override {}

    void NotifyOnPreDraw() override {}

    bool CallRouterBackToPopPage() override
    {
        return OnBackPressed();
    }

    bool OnBackPressed();

    RefPtr<FrameNode> FindNavigationNodeToHandleBack(const RefPtr<UINode>& node);

    void AddDirtyPropertyNode(const RefPtr<FrameNode>& dirty);

    void AddDirtyCustomNode(const RefPtr<UINode>& dirtyNode);

    void AddDirtyLayoutNode(const RefPtr<FrameNode>& dirty);

    void AddLayoutNode(const RefPtr<FrameNode>& layoutNode);

    void AddDirtyRenderNode(const RefPtr<FrameNode>& dirty);

    void AddPredictTask(PredictTask&& task);

    void AddAfterLayoutTask(std::function<void()>&& task, bool isFlushInImplicitAnimationTask = false);

    void AddPersistAfterLayoutTask(std::function<void()>&& task);

    void AddAfterRenderTask(std::function<void()>&& task);

    void AddDragWindowVisibleTask(std::function<void()>&& task)
    {
        dragWindowVisibleCallback_ = std::move(task);
    }

    void FlushOnceVsyncTask() override;

    void FlushDirtyNodeUpdate();

    void SetRootRect(double width, double height, double offset) override;

    void SetWindowSceneConsumed(bool isConsumed);

    bool IsWindowSceneConsumed();

    bool IsDestroyed();

    void UpdateSystemSafeArea(const SafeAreaInsets& systemSafeArea) override;
    void UpdateCutoutSafeArea(const SafeAreaInsets& cutoutSafeArea) override;
    void UpdateNavSafeArea(const SafeAreaInsets& navSafeArea) override;
    void UpdateOriginAvoidArea(const Rosen::AvoidArea& avoidArea, uint32_t type) override;

    float GetPageAvoidOffset() override;

    void CheckAndUpdateKeyboardInset() override;

    void UpdateSizeChangeReason(
        WindowSizeChangeReason type, const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr);

    void UpdateDisplayAvailableRect(const Rect& displayAvailableRect)
    {
        displayAvailableRect_ = displayAvailableRect;
    }
    Rect GetDisplayAvailableRect() const
    {
        return displayAvailableRect_;
    }
    void SetEnableKeyBoardAvoidMode(bool value) override;
    bool IsEnableKeyBoardAvoidMode() override;
    const RefPtr<SafeAreaManager>& GetSafeAreaManager() const
    {
        return safeAreaManager_;
    }
    virtual SafeAreaInsets GetSafeArea() const;

    virtual SafeAreaInsets GetSafeAreaWithoutProcess() const;

    const RefPtr<FullScreenManager>& GetFullScreenManager();

    RefPtr<AccessibilityManagerNG> GetAccessibilityManagerNG();

    void SendEventToAccessibilityWithNode(
        const AccessibilityEvent& accessibilityEvent, const RefPtr<FrameNode>& node);

    const RefPtr<StageManager>& GetStageManager();

    const RefPtr<OverlayManager>& GetOverlayManager();

    const RefPtr<SelectOverlayManager>& GetSelectOverlayManager();

    const RefPtr<SharedOverlayManager>& GetSharedOverlayManager()
    {
        return sharedTransitionManager_;
    }

#ifdef WINDOW_SCENE_SUPPORTED
    const RefPtr<UIExtensionManager>& GetUIExtensionManager()
    {
        return uiExtensionManager_;
    }
#endif

    const RefPtr<DragDropManager>& GetDragDropManager();

    const RefPtr<FocusManager>& GetFocusManager() const;

    const RefPtr<FocusManager>& GetOrCreateFocusManager();

    const RefPtr<FrameRateManager>& GetFrameRateManager()
    {
        return frameRateManager_;
    }

    void FlushBuild() override;

    void FlushPipelineImmediately() override;
    void RebuildFontNode() override;

    void AddBuildFinishCallBack(std::function<void()>&& callback);

    void AddWindowStateChangedCallback(int32_t nodeId);

    void RemoveWindowStateChangedCallback(int32_t nodeId);

    void AddWindowFocusChangedCallback(int32_t nodeId);

    void RemoveWindowFocusChangedCallback(int32_t nodeId);

    void AddWindowSizeChangeCallback(int32_t nodeId);

    void RemoveWindowSizeChangeCallback(int32_t nodeId);

    void AddNavigationNode(int32_t pageId, WeakPtr<UINode> navigationNode);

    void RemoveNavigationNode(int32_t pageId, int32_t nodeId);

    void FirePageChanged(int32_t pageId, bool isOnShow);

    bool HasDifferentDirectionGesture() const;

    bool IsKeyInPressed(KeyCode tarCode) const
    {
        CHECK_NULL_RETURN(eventManager_, false);
        return eventManager_->IsKeyInPressed(tarCode);
    }

    bool GetIsFocusingByTab() const
    {
        return isFocusingByTab_;
    }

    void SetIsFocusingByTab(bool isFocusingByTab)
    {
        isFocusingByTab_ = isFocusingByTab;
    }

    bool GetIsFocusActive() const
    {
        return isFocusActive_;
    }

    bool SetIsFocusActive(bool isFocusActive);

    void AddIsFocusActiveUpdateEvent(const RefPtr<FrameNode>& node, const std::function<void(bool)>& eventCallback);
    void RemoveIsFocusActiveUpdateEvent(const RefPtr<FrameNode>& node);

    bool IsTabJustTriggerOnKeyEvent() const
    {
        return isTabJustTriggerOnKeyEvent_;
    }

    bool GetOnShow() const override
    {
        return onShow_;
    }

    bool ChangeMouseStyle(int32_t nodeId, MouseFormat format, int32_t windowId = 0, bool isByPass = false);

    bool RequestFocus(const std::string& targetNodeId, bool isSyncRequest = false) override;
    void AddDirtyFocus(const RefPtr<FrameNode>& node);
    void AddDirtyRequestFocus(const RefPtr<FrameNode>& node);
    void RootLostFocus(BlurReason reason = BlurReason::FOCUS_SWITCH) const;

    void SetContainerWindow(bool isShow) override;
    void SetContainerButtonHide(bool hideSplit, bool hideMaximize, bool hideMinimize) override;
    void SetCloseButtonStatus(bool isEnabled);

    void AddNodesToNotifyMemoryLevel(int32_t nodeId);
    void RemoveNodesToNotifyMemoryLevel(int32_t nodeId);
    void NotifyMemoryLevel(int32_t level) override;
    void FlushModifier() override;
    void FlushMessages() override;

    void FlushUITasks(bool triggeredByImplicitAnimation = false) override;

    void FlushAfterLayoutCallbackInImplicitAnimationTask() override;

    bool IsLayouting() const override
    {
        return taskScheduler_->IsLayouting();
    }
    // end pipeline, exit app
    void Finish(bool autoFinish) const override;
    RectF GetRootRect()
    {
        CHECK_NULL_RETURN(rootNode_, RectF());
        auto geometryNode = rootNode_->GetGeometryNode();
        CHECK_NULL_RETURN(geometryNode, RectF());
        return geometryNode->GetFrameRect();
    }

    void FlushReload(const ConfigurationChange& configurationChange, bool fullUpdate = true) override;

    int32_t RegisterSurfaceChangedCallback(
        std::function<void(int32_t, int32_t, int32_t, int32_t, WindowSizeChangeReason)>&& callback)
    {
        if (callback) {
            surfaceChangedCallbackMap_.emplace(++callbackId_, std::move(callback));
            return callbackId_;
        }
        return 0;
    }

    void UnregisterSurfaceChangedCallback(int32_t callbackId)
    {
        surfaceChangedCallbackMap_.erase(callbackId);
    }

    int32_t RegisterFoldStatusChangedCallback(std::function<void(FoldStatus)>&& callback)
    {
        if (callback) {
            foldStatusChangedCallbackMap_.emplace(callbackId_, std::move(callback));
            return callbackId_;
        }
        return 0;
    }

    void UnRegisterFoldStatusChangedCallback(int32_t callbackId)
    {
        foldStatusChangedCallbackMap_.erase(callbackId);
    }

    int32_t RegisterFoldDisplayModeChangedCallback(std::function<void(FoldDisplayMode)>&& callback)
    {
        if (callback) {
            foldDisplayModeChangedCallbackMap_.emplace(++callbackId_, std::move(callback));
            return callbackId_;
        }
        return 0;
    }

    void UnRegisterFoldDisplayModeChangedCallback(int32_t callbackId)
    {
        foldDisplayModeChangedCallbackMap_.erase(callbackId);
    }

    int32_t RegisterSurfacePositionChangedCallback(std::function<void(int32_t, int32_t)>&& callback)
    {
        if (callback) {
            surfacePositionChangedCallbackMap_.emplace(++callbackId_, std::move(callback));
            return callbackId_;
        }
        return 0;
    }

    void UnregisterSurfacePositionChangedCallback(int32_t callbackId)
    {
        surfacePositionChangedCallbackMap_.erase(callbackId);
    }

    int32_t RegisterTransformHintChangeCallback(std::function<void(uint32_t)>&& callback)
    {
        if (callback) {
            transformHintChangedCallbackMap_.emplace(++callbackId_, std::move(callback));
            return callbackId_;
        }
        return 0;
    }

    void UnregisterTransformHintChangedCallback(int32_t callbackId)
    {
        transformHintChangedCallbackMap_.erase(callbackId);
    }

    void SetMouseStyleHoldNode(int32_t id)
    {
        if (mouseStyleNodeId_ == -1) {
            mouseStyleNodeId_ = id;
        }
    }
    void FreeMouseStyleHoldNode(int32_t id)
    {
        if (mouseStyleNodeId_ == id) {
            mouseStyleNodeId_ = -1;
        }
    }

    void MarkNeedFlushMouseEvent()
    {
        isNeedFlushMouseEvent_ = true;
    }

    void MarkNeedFlushAnimationStartTime()
    {
        isNeedFlushAnimationStartTime_ = true;
    }

    // font
    void AddFontNodeNG(const WeakPtr<UINode>& node);
    void RemoveFontNodeNG(const WeakPtr<UINode>& node);

    // restore
    void RestoreNodeInfo(std::unique_ptr<JsonValue> nodeInfo) override;
    std::unique_ptr<JsonValue> GetStoredNodeInfo() override;
    void StoreNode(int32_t restoreId, const WeakPtr<FrameNode>& node);
    bool GetRestoreInfo(int32_t restoreId, std::string& restoreInfo);
    void RemoveStoredNode(int32_t restoreId)
    {
        storeNode_.erase(restoreId);
    }
    void SetNeedRenderNode(const RefPtr<FrameNode>& node);

    void SetIgnoreViewSafeArea(bool value) override;
    void SetIsLayoutFullScreen(bool value) override;
    void SetIsNeedAvoidWindow(bool value) override;

    void AddAnimationClosure(std::function<void()>&& animation);
    void FlushAnimationClosure();
    void RegisterDumpInfoListener(const std::function<void(const std::vector<std::string>&)>& callback);
    void DumpJsInfo(const std::vector<std::string>& params) const;

    bool DumpPageViewData(const RefPtr<FrameNode>& node, RefPtr<ViewDataWrap> viewDataWrap,
        bool skipSubAutoFillContainer = false);
    bool CheckNeedAutoSave();
    bool CheckPageFocus();
    bool CheckOverlayFocus();
    void NotifyFillRequestSuccess(AceAutoFillType autoFillType, RefPtr<ViewDataWrap> viewDataWrap);
    void NotifyFillRequestFailed(RefPtr<FrameNode> node, int32_t errCode,
        const std::string& fillContent = "", bool isPopup = false);

    std::shared_ptr<NavigationController> GetNavigationController(const std::string& id) override;
    void AddOrReplaceNavigationNode(const std::string& id, const WeakPtr<FrameNode>& node);
    void DeleteNavigationNode(const std::string& id);

    void AddGestureTask(const DelayedTask& task)
    {
        delayedTasks_.emplace_back(task);
    }

    void RemoveGestureTask(const DelayedTask& task)
    {
        for (auto iter = delayedTasks_.begin(); iter != delayedTasks_.end();) {
            if (iter->recognizer == task.recognizer) {
                delayedTasks_.erase(iter++);
            } else {
                ++iter;
            }
        }
    }

    void SetScreenNode(const RefPtr<FrameNode>& node)
    {
        CHECK_NULL_VOID(node);
        screenNode_ = AceType::WeakClaim(AceType::RawPtr(node));
    }
    RefPtr<FrameNode> GetScreenNode() const
    {
        return screenNode_.Upgrade();
    }

    void SetFocusedWindowSceneNode(const RefPtr<FrameNode>& node)
    {
        CHECK_NULL_VOID(node);
        windowSceneNode_ = AceType::WeakClaim(AceType::RawPtr(node));
    }
    RefPtr<FrameNode> GetFocusedWindowSceneNode() const
    {
        return windowSceneNode_.Upgrade();
    }

    void SetJSViewActive(bool active, WeakPtr<CustomNode> custom);

    void UpdateCurrentActiveNode(const WeakPtr<FrameNode>& node) override
    {
        activeNode_ = std::move(node);
    }

    const WeakPtr<FrameNode>& GetCurrentActiveNode() const
    {
        return activeNode_;
    }

    std::string GetCurrentExtraInfo() override;
    void UpdateTitleInTargetPos(bool isShow, int32_t height) override;

    void SetCursor(int32_t cursorValue) override;

    void RestoreDefault(int32_t windowId = 0) override;

    void OnFoldStatusChange(FoldStatus foldStatus) override;
    void OnFoldDisplayModeChange(FoldDisplayMode foldDisplayMode) override;

    void OnTransformHintChanged(uint32_t transform) override;

    uint32_t GetTransformHint() const
    {
        return transform_;
    }

    // for frontend animation interface.
    void OpenFrontendAnimation(
        const AnimationOption& option, const RefPtr<Curve>& curve, const std::function<void()>& finishCallback);
    void CloseFrontendAnimation();

    bool IsDragging() const override;
    void SetIsDragging(bool isDragging) override;

    void ResetDragging() override;
    const RefPtr<PostEventManager>& GetPostEventManager();

    void SetContainerModalTitleVisible(bool customTitleSettedShow, bool floatingTitleSettedShow);
    void SetContainerModalTitleHeight(int32_t height);
    int32_t GetContainerModalTitleHeight();
    bool GetContainerModalButtonsRect(RectF& containerModal, RectF& buttons);
    void SubscribeContainerModalButtonsRectChange(
        std::function<void(RectF& containerModal, RectF& buttons)>&& callback);

    const SerializedGesture& GetSerializedGesture() const override;
    // return value means whether it has printed info
    bool PrintVsyncInfoIfNeed() const override;
    void SetUIExtensionImeShow(bool imeShow);

    void CheckVirtualKeyboardHeight() override;

    void StartWindowAnimation() override
    {
        isWindowAnimation_ = true;
    }

    void StopWindowAnimation() override
    {
        isWindowAnimation_ = false;
    }

    void AddSyncGeometryNodeTask(std::function<void()>&& task) override;
    void FlushSyncGeometryNodeTasks() override;
    void SetVsyncListener(VsyncCallbackFun vsync)
    {
        vsyncListener_ = std::move(vsync);
    }

    void SetOnceVsyncListener(VsyncCallbackFun vsync)
    {
        onceVsyncListener_ = std::move(vsync);
    }

    bool HasOnceVsyncListener() {
        return onceVsyncListener_ != nullptr;
    }

    const RefPtr<NavigationManager>& GetNavigationManager() const
    {
        return navigationMgr_;
    }

    RefPtr<PrivacySensitiveManager> GetPrivacySensitiveManager() const
    {
        return privacySensitiveManager_;
    }

    void ChangeSensitiveNodes(bool flag) override
    {
        privacySensitiveManager_->TriggerFrameNodesSensitive(flag);
    }

    void FlushRequestFocus();

    Dimension GetCustomTitleHeight();

    void SetOverlayNodePositions(std::vector<Ace::RectF> rects);

    static void SetCallBackNode(const WeakPtr<NG::FrameNode>& node);

    std::vector<Ace::RectF> GetOverlayNodePositions();

    void RegisterOverlayNodePositionsUpdateCallback(
        const std::function<void(std::vector<Ace::RectF>)>&& callback);

    void TriggerOverlayNodePositionsUpdateCallback(std::vector<Ace::RectF> rects);

    void DetachNode(RefPtr<UINode> uiNode);

    void CheckNeedUpdateBackgroundColor(Color& color);

    bool CheckNeedDisableUpdateBackgroundImage();

    void ChangeDarkModeBrightness() override;
    void SetLocalColorMode(ColorMode colorMode)
    {
        auto localColorModeValue = static_cast<int32_t>(colorMode);
        localColorMode_ = localColorModeValue;
    }

    ColorMode GetLocalColorMode() const
    {
        ColorMode colorMode = static_cast<ColorMode>(localColorMode_.load());
        return colorMode;
    }

    void SetIsFreezeFlushMessage(bool isFreezeFlushMessage)
    {
        isFreezeFlushMessage_ = isFreezeFlushMessage;
    }

    bool IsFreezeFlushMessage() const
    {
        return isFreezeFlushMessage_;
    }
    bool IsContainerModalVisible() override;
    void SetDoKeyboardAvoidAnimate(bool isDoKeyboardAvoidAnimate)
    {
        isDoKeyboardAvoidAnimate_ = isDoKeyboardAvoidAnimate;
    }

    void CheckAndLogLastReceivedTouchEventInfo(int32_t eventId, TouchType type) override;

    void CheckAndLogLastConsumedTouchEventInfo(int32_t eventId, TouchType type) override;

    void CheckAndLogLastReceivedMouseEventInfo(int32_t eventId, MouseAction action) override;

    void CheckAndLogLastConsumedMouseEventInfo(int32_t eventId, MouseAction action) override;

    void CheckAndLogLastReceivedAxisEventInfo(int32_t eventId, AxisAction action) override;

    void CheckAndLogLastConsumedAxisEventInfo(int32_t eventId, AxisAction action) override;

    void AddFrameCallback(FrameCallbackFunc&& frameCallbackFunc)
    {
        frameCallbackFuncs_.emplace_back(std::move(frameCallbackFunc));
        RequestFrame();
    }

    void FlushFrameCallback(uint64_t nanoTimestamp);

    void RegisterTouchEventListener(const std::shared_ptr<ITouchEventCallback>& listener);
    void UnregisterTouchEventListener(const WeakPtr<NG::Pattern>& pattern);

    void SetPredictNode(const RefPtr<FrameNode>& node)
    {
        predictNode_ = node;
    }

    void ResetPredictNode()
    {
        predictNode_.Reset();
    }

    void PreLayout(uint64_t nanoTimestamp, uint32_t frameCount);

    bool IsDensityChanged() const override
    {
        return isDensityChanged_;
    }


    void UpdateLastVsyncEndTimestamp(uint64_t lastVsyncEndTimestamp) override
    {
        lastVsyncEndTimestamp_ = lastVsyncEndTimestamp;
    }
protected:
    void StartWindowSizeChangeAnimate(int32_t width, int32_t height, WindowSizeChangeReason type,
        const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr);
    void StartWindowMaximizeAnimation(
        int32_t width, int32_t height, const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr);
    void StartFullToMultWindowAnimation(int32_t width, int32_t height, WindowSizeChangeReason type,
        const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr);

    void FlushVsync(uint64_t nanoTimestamp, uint32_t frameCount) override;
    void FlushPipelineWithoutAnimation() override;
    void FlushFocus();
    void FlushFocusWithNode(RefPtr<FrameNode> focusNode, bool isScope);
    void DispatchDisplaySync(uint64_t nanoTimestamp) override;
    void FlushAnimation(uint64_t nanoTimestamp) override;
    bool OnDumpInfo(const std::vector<std::string>& params) const override;

    void OnVirtualKeyboardHeightChange(float keyboardHeight,
        const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr, const float safeHeight = 0.0f,
        const bool supportAvoidance = false) override;
    void OnVirtualKeyboardHeightChange(float keyboardHeight, double positionY, double height,
        const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr, bool forceChange = false) override;

    void SetIsLayouting(bool layouting)
    {
        taskScheduler_->SetIsLayouting(layouting);
    }

    void AvoidanceLogic(float keyboardHeight, const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr,
        const float safeHeight = 0.0f, const bool supportAvoidance = false);
    void OriginalAvoidanceLogic(
        float keyboardHeight, const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr);
    RefPtr<FrameNode> GetContainerModalNode();
    void DoKeyboardAvoidAnimate(const KeyboardAnimationConfig& keyboardAnimationConfig, float keyboardHeight,
        const std::function<void()>& func);

private:
    void ExecuteSurfaceChangedCallbacks(int32_t newWidth, int32_t newHeight, WindowSizeChangeReason type);

    void FlushWindowStateChangedCallback(bool isShow);

    void FlushWindowFocusChangedCallback(bool isFocus);

    void FlushWindowSizeChangeCallback(int32_t width, int32_t height, WindowSizeChangeReason type);

    void FlushTouchEvents();

    void FlushFocusView();
    void FlushFocusScroll();

    void ProcessDelayTasks();

    void InspectDrew();

    bool TriggerKeyEventDispatch(const KeyEvent& event);

    bool DispatchTabKey(const KeyEvent& event, const RefPtr<FocusView>& curFocusView);

    bool IsSkipShortcutAndFocusMove();

    void FlushBuildFinishCallbacks();

    void DumpPipelineInfo() const;

    void RegisterRootEvent();

    void ResetDraggingStatus(const TouchEvent& touchPoint);

    void CompensateTouchMoveEvent(const TouchEvent& event);

    bool CompensateTouchMoveEventFromUnhandledEvents(const TouchEvent& event);

    FrameInfo* GetCurrentFrameInfo(uint64_t recvTime, uint64_t timeStamp);

    void AnimateOnSafeAreaUpdate();
    void SyncSafeArea(SafeAreaSyncType syncType = SafeAreaSyncType::SYNC_TYPE_NONE);

    // only used for static form.
    void UpdateFormLinkInfos();

    void FlushFrameRate();

    void RegisterFocusCallback();

    template<typename T>
    struct NodeCompare {
        bool operator()(const T& nodeLeft, const T& nodeRight) const
        {
            if (!nodeLeft || !nodeRight) {
                return false;
            }
            if (nodeLeft->GetDepth() < nodeRight->GetDepth()) {
                return true;
            }
            if (nodeLeft->GetDepth() == nodeRight->GetDepth()) {
                return nodeLeft < nodeRight;
            }
            return false;
        }
    };

    std::pair<float, float> LinearInterpolation(const std::tuple<float, float, uint64_t>& history,
        const std::tuple<float, float, uint64_t>& current, const uint64_t nanoTimeStamp);

    std::pair<float, float> GetResampleCoord(const std::vector<TouchEvent>& history,
        const std::vector<TouchEvent>& current, const uint64_t nanoTimeStamp, const bool isScreen);

    std::tuple<float, float, uint64_t> GetAvgPoint(const std::vector<TouchEvent>& events, const bool isScreen);

    TouchEvent GetResampleTouchEvent(
        const std::vector<TouchEvent>& history, const std::vector<TouchEvent>& current, const uint64_t nanoTimeStamp);

    TouchEvent GetLatestPoint(const std::vector<TouchEvent>& current, const uint64_t nanoTimeStamp);

    std::unique_ptr<UITaskScheduler> taskScheduler_ = std::make_unique<UITaskScheduler>();

    std::unordered_map<uint32_t, WeakPtr<ScheduleTask>> scheduleTasks_;

    std::set<RefPtr<FrameNode>, NodeCompare<RefPtr<FrameNode>>> dirtyPropertyNodes_; // used in node api.
    std::set<RefPtr<UINode>, NodeCompare<RefPtr<UINode>>> dirtyNodes_;
    std::list<std::function<void()>> buildFinishCallbacks_;

    // window on show or on hide
    std::set<int32_t> onWindowStateChangedCallbacks_;
    // window on focused or on unfocused
    std::list<int32_t> onWindowFocusChangedCallbacks_;
    // window on drag
    std::list<int32_t> onWindowSizeChangeCallbacks_;

    std::list<int32_t> nodesToNotifyMemoryLevel_;

    std::list<TouchEvent> touchEvents_;

    std::vector<std::function<void(const std::vector<std::string>&)>> dumpListeners_;

    RefPtr<FrameNode> rootNode_;

    int32_t curFocusNodeId_ = -1;

    std::set<RefPtr<FrameNode>> needRenderNode_;

    int32_t callbackId_ = 0;
    SurfaceChangedCallbackMap surfaceChangedCallbackMap_;
    SurfacePositionChangedCallbackMap surfacePositionChangedCallbackMap_;
    FoldStatusChangedCallbackMap foldStatusChangedCallbackMap_;
    FoldDisplayModeChangedCallbackMap foldDisplayModeChangedCallbackMap_;
    TransformHintChangedCallbackMap transformHintChangedCallbackMap_;

    bool isOnAreaChangeNodesCacheVaild_ = false;
    std::vector<FrameNode*> onAreaChangeNodesCache_;
    std::unordered_set<int32_t> onAreaChangeNodeIds_;
    std::unordered_set<int32_t> onVisibleAreaChangeNodeIds_;

    RefPtr<AccessibilityManagerNG> accessibilityManagerNG_;
    RefPtr<StageManager> stageManager_;
    RefPtr<OverlayManager> overlayManager_;
    RefPtr<FullScreenManager> fullScreenManager_;
    RefPtr<SelectOverlayManager> selectOverlayManager_;
    RefPtr<DragDropManager> dragDropManager_;
    RefPtr<FocusManager> focusManager_;
    RefPtr<SharedOverlayManager> sharedTransitionManager_;
#ifdef WINDOW_SCENE_SUPPORTED
    RefPtr<UIExtensionManager> uiExtensionManager_;
#endif
    RefPtr<SafeAreaManager> safeAreaManager_ = MakeRefPtr<SafeAreaManager>();
    RefPtr<FrameRateManager> frameRateManager_ = MakeRefPtr<FrameRateManager>();
    RefPtr<PrivacySensitiveManager> privacySensitiveManager_ = MakeRefPtr<PrivacySensitiveManager>();
    Rect displayAvailableRect_;
    std::unordered_map<size_t, TouchTestResult> touchTestResults_;
    WeakPtr<FrameNode> dirtyFocusNode_;
    WeakPtr<FrameNode> dirtyFocusScope_;
    WeakPtr<FrameNode> dirtyRequestFocusNode_;
    WeakPtr<FrameNode> screenNode_;
    WeakPtr<FrameNode> windowSceneNode_;
    uint32_t nextScheduleTaskId_ = 0;
    int32_t mouseStyleNodeId_ = -1;
    uint64_t resampleTimeStamp_ = 0;
    uint64_t lastVsyncEndTimestamp_ = 0;
    bool hasIdleTasks_ = false;
    bool isFocusingByTab_ = false;
    bool isFocusActive_ = false;
    bool isTabJustTriggerOnKeyEvent_ = false;
    bool isWindowHasFocused_ = false;
    bool onShow_ = false;
    bool isNeedFlushMouseEvent_ = false;
    bool isNeedFlushAnimationStartTime_ = false;
    bool canUseLongPredictTask_ = false;
    bool isWindowSceneConsumed_ = false;
    bool isDensityChanged_ = false;
    bool isBeforeDragHandleAxis_ = false;
    WeakPtr<FrameNode> activeNode_;
    bool isWindowAnimation_ = false;
    bool prevKeyboardAvoidMode_ = false;
    bool isFreezeFlushMessage_ = false;
    bool destroyed_ = false;

    RefPtr<FrameNode> focusNode_;
    std::function<void()> focusOnNodeCallback_;
    std::function<void()> dragWindowVisibleCallback_;

    std::optional<bool> needSoftKeyboard_;
    std::optional<bool> windowFocus_;
    std::optional<bool> windowShow_;

    std::unique_ptr<MouseEvent> lastMouseEvent_;

    std::unordered_map<int32_t, WeakPtr<FrameNode>> storeNode_;
    std::unordered_map<int32_t, std::string> restoreNodeInfo_;
    std::unordered_map<int32_t, std::vector<WeakPtr<UINode>>> pageToNavigationNodes_;
    std::unordered_map<int32_t, std::vector<TouchEvent>> historyPointsById_;

    std::list<FrameInfo> dumpFrameInfos_;
    std::list<std::function<void()>> animationClosuresList_;

    std::map<int32_t, std::function<void(bool)>> isFocusActiveUpdateEvents_;
    mutable std::mutex navigationMutex_;
    std::map<std::string, WeakPtr<FrameNode>> navigationNodes_;
    std::list<DelayedTask> delayedTasks_;
    RefPtr<PostEventManager> postEventManager_;

    std::unordered_map<int32_t, TouchEvent> idToTouchPoints_;
    std::unordered_map<int32_t, uint64_t> lastDispatchTime_;
    std::vector<Ace::RectF> overlayNodePositions_;
    std::function<void(std::vector<Ace::RectF>)> overlayNodePositionUpdateCallback_;

    RefPtr<FrameNode> predictNode_;

    VsyncCallbackFun vsyncListener_;
    VsyncCallbackFun onceVsyncListener_;
    ACE_DISALLOW_COPY_AND_MOVE(PipelineContext);

    int32_t preNodeId_ = -1;

    RefPtr<NavigationManager> navigationMgr_ = MakeRefPtr<NavigationManager>();
    std::atomic<int32_t> localColorMode_ = static_cast<int32_t>(ColorMode::COLOR_MODE_UNDEFINED);
    std::vector<std::shared_ptr<ITouchEventCallback>> listenerVector_;
    bool customTitleSettedShow_ = true;
    bool isShowTitle_ = false;
    bool lastAnimationStatus_ = true;
    bool isDoKeyboardAvoidAnimate_ = true;

    std::list<FrameCallbackFunc> frameCallbackFuncs_;
    uint32_t transform_ = 0;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_PIPELINE_NG_CONTEXT_H
