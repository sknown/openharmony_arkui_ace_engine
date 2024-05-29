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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_PIPELINE_PIPELINE_BASE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_PIPELINE_PIPELINE_BASE_H

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>

#include "base/geometry/dimension.h"
#include "base/resource/asset_manager.h"
#include "base/resource/data_provider_manager.h"
#include "base/resource/shared_image_manager.h"
#include "base/thread/task_executor.h"
#include "core/accessibility/accessibility_manager.h"
#include "core/animation/schedule_task.h"
#include "core/common/clipboard/clipboard_proxy.h"
#include "core/common/display_info.h"
#include "core/common/draw_delegate.h"
#include "core/common/event_manager.h"
#include "core/common/platform_bridge.h"
#include "core/common/platform_res_register.h"
#include "core/common/resource/resource_configuration.h"
#include "core/common/thread_checker.h"
#include "core/common/window_animation_config.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/animation_option.h"
#include "core/components/theme/theme_manager.h"
#include "core/components_ng/property/safe_area_insets.h"
#include "core/event/axis_event.h"
#include "core/event/key_event.h"
#include "core/event/mouse_event.h"
#include "core/event/rotation_event.h"
#include "core/event/touch_event.h"
#include "core/event/pointer_event.h"
#include "core/gestures/gesture_info.h"
#include "core/image/image_cache.h"
#include "core/pipeline/container_window_manager.h"
#include "core/components_ng/manager/display_sync/ui_display_sync_manager.h"
#include "interfaces/inner_api/ace/serialized_gesture.h"

namespace OHOS::Rosen {
class RSTransaction;
class AvoidArea;
} // namespace OHOS::Rosen

namespace OHOS::Ace {
namespace NG {
class FrameNode;
} // namespace NG

struct KeyboardAnimationConfig {
    std::string curveType_;
    std::vector<float> curveParams_;
    uint32_t durationIn_ = 0;
    uint32_t durationOut_ = 0;
};

struct FontInfo;
struct FontConfigJsonInfo;
class Frontend;
class OffscreenCanvas;
class Window;
class FontManager;
class ManagerInterface;
class NavigationController;
enum class FrontendType;
using SharePanelCallback = std::function<void(const std::string& bundleName, const std::string& abilityName)>;
using AceVsyncCallback = std::function<void(uint64_t, uint32_t)>;
using EtsCardTouchEventCallback = std::function<void(const TouchEvent&,
    SerializedGesture& serializedGesture)>;

class ACE_FORCE_EXPORT PipelineBase : public AceType {
    DECLARE_ACE_TYPE(PipelineBase, AceType);

public:
    PipelineBase() = default;
    PipelineBase(std::shared_ptr<Window> window, RefPtr<TaskExecutor> taskExecutor, RefPtr<AssetManager> assetManager,
        const RefPtr<Frontend>& frontend, int32_t instanceId);
    PipelineBase(std::shared_ptr<Window> window, RefPtr<TaskExecutor> taskExecutor, RefPtr<AssetManager> assetManager,
        const RefPtr<Frontend>& frontend, int32_t instanceId, RefPtr<PlatformResRegister> platformResRegister);
    ~PipelineBase() override;

    static RefPtr<PipelineBase> GetCurrentContext();

    static RefPtr<PipelineBase> GetCurrentContextSafely();

    static RefPtr<PipelineBase> GetMainPipelineContext();

    static RefPtr<ThemeManager> CurrentThemeManager();

    static void SetCallBackNode(const WeakPtr<NG::FrameNode>& node);

    /*
     * Change px to vp with density of current pipeline
     */
    static double Px2VpWithCurrentDensity(double px);

    /*
     * Change vp to px with density of current pipeline
     */
    static double Vp2PxWithCurrentDensity(double vp);

    /*
     * Get density of current pipeline if valid, or return density of default display
     */
    static double GetCurrentDensity();

    virtual void SetupRootElement() = 0;

    virtual uint64_t GetTimeFromExternalTimer();

    virtual bool NeedSoftKeyboard() = 0;

    virtual void SetOnWindowFocused(const std::function<void()>& callback) = 0;

    bool Animate(const AnimationOption& option, const RefPtr<Curve>& curve,
        const std::function<void()>& propertyCallback, const std::function<void()>& finishCallBack = nullptr);

    virtual void AddKeyFrame(
        float fraction, const RefPtr<Curve>& curve, const std::function<void()>& propertyCallback) = 0;

    virtual void AddKeyFrame(float fraction, const std::function<void()>& propertyCallback) = 0;

    void PrepareOpenImplicitAnimation();

    void OpenImplicitAnimation(const AnimationOption& option, const RefPtr<Curve>& curve,
        const std::function<void()>& finishCallback = nullptr);

    void PrepareCloseImplicitAnimation();

    bool CloseImplicitAnimation();

    void ForceLayoutForImplicitAnimation();

    void ForceRenderForImplicitAnimation();

    // add schedule task and return the unique mark id.
    virtual uint32_t AddScheduleTask(const RefPtr<ScheduleTask>& task) = 0;

    // remove schedule task by id.
    virtual void RemoveScheduleTask(uint32_t id) = 0;

    // Called by view when touch event received.
    virtual void OnTouchEvent(const TouchEvent& point, bool isSubPipe = false) = 0;

    // Called by ohos AceContainer when touch event received.
    virtual void OnTouchEvent(const TouchEvent& point, const RefPtr<NG::FrameNode>& node, bool isSubPipe = false) {}

    // Called by container when key event received.
    // if return false, then this event needs platform to handle it.
    virtual bool OnKeyEvent(const KeyEvent& event) = 0;

    // Called by view when mouse event received.
    virtual void OnMouseEvent(const MouseEvent& event) = 0;

    // Called by ohos AceContainer when mouse event received.
    virtual void OnMouseEvent(const MouseEvent& event, const RefPtr<NG::FrameNode>& node) {}

    // Called by view when axis event received.
    virtual void OnAxisEvent(const AxisEvent& event) = 0;

    // Called by ohos AceContainer when axis event received.
    virtual void OnAxisEvent(const AxisEvent& event, const RefPtr<NG::FrameNode>& node) {}

    // Called by container when rotation event received.
    // if return false, then this event needs platform to handle it.
    virtual bool OnRotationEvent(const RotationEvent& event) const = 0;

    // Called by window when received vsync signal.
    virtual void OnVsyncEvent(uint64_t nanoTimestamp, uint32_t frameCount);

    // Called by viewr
    virtual void OnDragEvent(const PointerEvent& pointerEvent, DragEventAction action) = 0;

    // Called by view when idle event.
    virtual void OnIdle(int64_t deadline) = 0;

    virtual void SetBuildAfterCallback(const std::function<void()>& callback) = 0;

    virtual void DispatchDisplaySync(uint64_t nanoTimestamp) = 0;

    virtual void FlushAnimation(uint64_t nanoTimestamp) = 0;

    virtual void SendEventToAccessibility(const AccessibilityEvent& accessibilityEvent);

    virtual void SaveExplicitAnimationOption(const AnimationOption& option) = 0;

    virtual void CreateExplicitAnimator(const std::function<void()>& onFinishEvent) = 0;

    virtual void ClearExplicitAnimationOption() = 0;

    virtual AnimationOption GetExplicitAnimationOption() const = 0;

    virtual void Destroy();

    virtual void OnShow() = 0;

    virtual void OnHide() = 0;

    virtual void WindowFocus(bool isFocus) = 0;

    virtual void ContainerModalUnFocus() = 0;

    virtual void ShowContainerTitle(bool isShow, bool hasDeco = true, bool needUpdate = false) = 0;

    virtual void OnSurfaceChanged(int32_t width, int32_t height,
        WindowSizeChangeReason type = WindowSizeChangeReason::UNDEFINED,
        const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr) = 0;

    virtual void OnSurfacePositionChanged(int32_t posX, int32_t posY) = 0;

    virtual void OnSurfaceDensityChanged(double density)
    {
        for (auto&& [id, callback] : densityChangedCallbacks_) {
            if (callback) {
                callback(density);
            }
        }
    }

    int32_t RegisterDensityChangedCallback(std::function<void(double)>&& callback)
    {
        if (callback) {
            densityChangedCallbacks_.emplace(++densityChangeCallbackId_, std::move(callback));
            return densityChangeCallbackId_;
        }
        return 0;
    }

    void UnregisterDensityChangedCallback(int32_t callbackId)
    {
        densityChangedCallbacks_.erase(callbackId);
    }

    virtual void OnSystemBarHeightChanged(double statusBar, double navigationBar) = 0;

    virtual void OnSurfaceDestroyed() = 0;

    virtual void NotifyOnPreDraw() = 0;

    virtual bool CallRouterBackToPopPage() = 0;

    virtual bool PopPageStackOverlay()
    {
        return false;
    }

    virtual void HideOverlays() {}

    virtual void OnPageShow() {}

    virtual void OnActionEvent(const std::string& action);

    virtual void Finish(bool autoFinish = true) const {}

    virtual void RequestFullWindow(int32_t duration) {}

    virtual bool RequestFocus(const std::string& targetNodeId, bool isSyncRequest = false)
    {
        return false;
    }

    // Called by AceContainer.
    bool Dump(const std::vector<std::string>& params) const;

    virtual bool IsLastPage()
    {
        return false;
    }

    virtual bool GetIsDeclarative() const
    {
        return true;
    }

    virtual void SetAppBgColor(const Color& color)
    {
        appBgColor_ = color;
    }

    virtual void ChangeDarkModeBrightness(bool isFocus) {}

    void SetFormRenderingMode(int8_t renderMode)
    {
        renderingMode_ = renderMode;
    }

    const Color& GetAppBgColor() const
    {
        return appBgColor_;
    }

    int32_t GetAppLabelId() const
    {
        return appLabelId_;
    }

    void SetAppLabelId(int32_t appLabelId)
    {
        appLabelId_ = appLabelId;
    }

    virtual void SetAppTitle(const std::string& title) = 0;

    virtual void SetAppIcon(const RefPtr<PixelMap>& icon) = 0;

    virtual void SetContainerButtonHide(bool hideSplit, bool hideMaximize, bool hideMinimize) {}

    virtual void RefreshRootBgColor() const {}

    virtual void PostponePageTransition() {}
    virtual void LaunchPageTransition() {}

    virtual void GetBoundingRectData(int32_t nodeId, Rect& rect) {}

    virtual void CheckAndUpdateKeyboardInset() {}

    virtual RefPtr<AccessibilityManager> GetAccessibilityManager() const;

    virtual std::shared_ptr<NavigationController> GetNavigationController(const std::string& id)
    {
        return nullptr;
    }

    void SetRootSize(double density, float width, float height);

    void UpdateFontWeightScale();

    double NormalizeToPx(const Dimension& dimension) const;

    double ConvertPxToVp(const Dimension& dimension) const;

    using FinishEventHandler = std::function<void()>;
    void SetFinishEventHandler(FinishEventHandler&& listener)
    {
        finishEventHandler_ = std::move(listener);
    }

    using StartAbilityHandler = std::function<void(const std::string& address)>;
    void SetStartAbilityHandler(StartAbilityHandler&& listener)
    {
        startAbilityHandler_ = std::move(listener);
    }
    void HyperlinkStartAbility(const std::string& address) const;

    using ActionEventHandler = std::function<void(const std::string& action)>;
    void SetActionEventHandler(ActionEventHandler&& listener)
    {
        actionEventHandler_ = std::move(listener);
    }

    using FormLinkInfoUpdateHandler = std::function<void(const std::vector<std::string>&)>;
    void SetFormLinkInfoUpdateHandler(FormLinkInfoUpdateHandler&& listener)
    {
        formLinkInfoUpdateHandler_ = std::move(listener);
    }

    using StatusBarEventHandler = std::function<void(const Color& color)>;
    void SetStatusBarEventHandler(StatusBarEventHandler&& listener)
    {
        statusBarBgColorEventHandler_ = std::move(listener);
    }
    void NotifyStatusBarBgColor(const Color& color) const;

    using PopupEventHandler = std::function<void()>;
    void SetPopupEventHandler(PopupEventHandler&& listener)
    {
        popupEventHandler_ = std::move(listener);
    }
    void NotifyPopupDismiss() const;

    using MenuEventHandler = std::function<void()>;
    void SetMenuEventHandler(MenuEventHandler&& listener)
    {
        menuEventHandler_ = std::move(listener);
    }
    void NotifyMenuDismiss() const;

    using ContextMenuEventHandler = std::function<void()>;
    void SetContextMenuEventHandler(ContextMenuEventHandler&& listener)
    {
        contextMenuEventHandler_ = std::move(listener);
    }
    void NotifyContextMenuDismiss() const;

    using RouterBackEventHandler = std::function<void()>;
    void SetRouterBackEventHandler(RouterBackEventHandler&& listener)
    {
        routerBackEventHandler_ = std::move(listener);
    }
    void NotifyRouterBackDismiss() const;

    using PopPageSuccessEventHandler = std::function<void(const std::string& pageUrl, const int32_t pageId)>;
    void SetPopPageSuccessEventHandler(PopPageSuccessEventHandler&& listener)
    {
        popPageSuccessEventHandler_.push_back(std::move(listener));
    }
    void NotifyPopPageSuccessDismiss(const std::string& pageUrl, int32_t pageId) const;

    using IsPagePathInvalidEventHandler = std::function<void(bool& isPageInvalid)>;
    void SetIsPagePathInvalidEventHandler(IsPagePathInvalidEventHandler&& listener)
    {
        isPagePathInvalidEventHandler_.push_back(std::move(listener));
    }
    void NotifyIsPagePathInvalidDismiss(bool isPageInvalid) const;

    using DestroyEventHandler = std::function<void()>;
    void SetDestroyHandler(DestroyEventHandler&& listener)
    {
        destroyEventHandler_.push_back(std::move(listener));
    }
    void NotifyDestroyEventDismiss() const;

    using DispatchTouchEventHandler = std::function<void(const TouchEvent& event)>;
    void SetDispatchTouchEventHandler(DispatchTouchEventHandler&& listener)
    {
        dispatchTouchEventHandler_.push_back(std::move(listener));
    }
    void NotifyDispatchTouchEventDismiss(const TouchEvent& event) const;

    using GetViewScaleCallback = std::function<bool(float&, float&)>;
    void SetGetViewScaleCallback(GetViewScaleCallback&& callback)
    {
        getViewScaleCallback_ = callback;
    }

    using OnPageShowCallBack = std::function<void()>;
    void SetOnPageShow(OnPageShowCallBack&& onPageShowCallBack)
    {
        onPageShowCallBack_ = std::move(onPageShowCallBack);
    }

    using AnimationCallback = std::function<void()>;
    void SetAnimationCallback(AnimationCallback&& callback)
    {
        animationCallback_ = std::move(callback);
    }

    using ProfilerCallback = std::function<void(const std::string&)>;
    void SetOnVsyncProfiler(const ProfilerCallback& callback)
    {
        onVsyncProfiler_ = callback;
    }

    using OnRouterChangeCallback = bool (*)(const std::string currentRouterPath);
    void AddRouterChangeCallback(const OnRouterChangeCallback& onRouterChangeCallback)
    {
        onRouterChangeCallback_ = onRouterChangeCallback;
    }

    void onRouterChange(const std::string& url);

    void ResetOnVsyncProfiler()
    {
        onVsyncProfiler_ = nullptr;
    }

    bool GetViewScale(float& scaleX, float& scaleY)
    {
        if (getViewScaleCallback_) {
            return getViewScaleCallback_(scaleX, scaleY);
        }
        return false;
    }

    RefPtr<TaskExecutor> GetTaskExecutor() const
    {
        return taskExecutor_;
    }

    RefPtr<Frontend> GetFrontend() const;

    int32_t GetInstanceId() const
    {
        return instanceId_;
    }

    void ClearImageCache();

    void SetImageCache(const RefPtr<ImageCache>& imageChache);

    RefPtr<ImageCache> GetImageCache() const;

    const RefPtr<SharedImageManager>& GetOrCreateSharedImageManager()
    {
        std::scoped_lock<std::shared_mutex> lock(imageMtx_);
        if (!sharedImageManager_) {
            sharedImageManager_ = MakeRefPtr<SharedImageManager>(taskExecutor_);
        }
        return sharedImageManager_;
    }

    const RefPtr<UIDisplaySyncManager>& GetOrCreateUIDisplaySyncManager()
    {
        std::call_once(displaySyncFlag_, [this]() {
            if (!uiDisplaySyncManager_) {
                uiDisplaySyncManager_ = MakeRefPtr<UIDisplaySyncManager>();
            }
        });
        return uiDisplaySyncManager_;
    }

    Window* GetWindow()
    {
        return window_.get();
    }

    RefPtr<AssetManager> GetAssetManager() const
    {
        return assetManager_;
    }

    int32_t GetMinPlatformVersion() const
    {
        // Since API10, the platform version data format has changed.
        // Use the last three digits of data as platform version. For example (4000000010).
        return minPlatformVersion_ % 1000;
    }

    void SetMinPlatformVersion(int32_t minPlatformVersion)
    {
        minPlatformVersion_ = minPlatformVersion;
    }

    void SetInstallationFree(int32_t installationFree)
    {
        installationFree_ = installationFree;
    }

    bool GetInstallationFree() const
    {
        return installationFree_;
    }

    void SetSharePanelCallback(SharePanelCallback&& callback)
    {
        sharePanelCallback_ = std::move(callback);
    }

    void FireSharePanelCallback(const std::string& bundleName, const std::string& abilityName)
    {
        if (sharePanelCallback_) {
            sharePanelCallback_(bundleName, abilityName);
        }
    }

    RefPtr<ThemeManager> GetThemeManager() const
    {
        std::shared_lock<std::shared_mutex> lock(themeMtx_);
        return themeManager_;
    }

    void SetThemeManager(RefPtr<ThemeManager> theme)
    {
        CHECK_RUN_ON(UI);
        std::unique_lock<std::shared_mutex> lock(themeMtx_);
        themeManager_ = std::move(theme);
    }

    template<typename T>
    RefPtr<T> GetTheme() const
    {
        std::shared_lock<std::shared_mutex> lock(themeMtx_);
        if (themeManager_) {
            return themeManager_->GetTheme<T>();
        }
        return {};
    }

    template<typename T>
    bool GetDraggable()
    {
        if (isJsCard_ || isFormRender_) {
            return false;
        }
        auto theme = GetTheme<T>();
        CHECK_NULL_RETURN(theme, false);
        return theme->GetDraggable();
    }

    const RefPtr<ManagerInterface>& GetTextFieldManager()
    {
        return textFieldManager_;
    }
    void SetTextFieldManager(const RefPtr<ManagerInterface>& manager);

    const RefPtr<FontManager>& GetFontManager() const
    {
        return fontManager_;
    }

    const RefPtr<DataProviderManagerInterface>& GetDataProviderManager() const
    {
        return dataProviderManager_;
    }
    void SetDataProviderManager(const RefPtr<DataProviderManagerInterface>& dataProviderManager)
    {
        dataProviderManager_ = dataProviderManager;
    }

    const RefPtr<PlatformBridge>& GetMessageBridge() const
    {
        return messageBridge_;
    }
    void SetMessageBridge(const RefPtr<PlatformBridge>& messageBridge)
    {
        messageBridge_ = messageBridge;
    }

    void SetIsJsCard(bool isJsCard)
    {
        isJsCard_ = isJsCard;
    }

    void SetIsJsPlugin(bool isJsPlugin)
    {
        isJsPlugin_ = isJsPlugin;
    }

    void SetDrawDelegate(std::unique_ptr<DrawDelegate> delegate)
    {
        drawDelegate_ = std::move(delegate);
    }

    bool IsJsCard() const
    {
        return isJsCard_;
    }

    bool IsJsPlugin() const
    {
        return isJsPlugin_;
    }

    void SetIsFormRender(bool isEtsCard)
    {
        isFormRender_ = isEtsCard;
    }

    bool IsFormRender() const
    {
        return isFormRender_;
    }

    // Get the dp scale which used to covert dp to logic px.
    double GetDipScale() const
    {
        return dipScale_;
    }

    // Get the window design scale used to covert lpx to logic px.
    double GetLogicScale() const
    {
        return designWidthScale_;
    }

    float GetFontScale() const
    {
        return fontScale_;
    }
    void SetFontScale(float fontScale);

    float GetFontWeightScale() const
    {
        return fontWeightScale_;
    }
    void SetFontWeightScale(float fontWeightScale);

    uint32_t GetWindowId() const
    {
        return windowId_;
    }

    void SetWindowId(uint32_t windowId)
    {
        windowId_ = windowId;
    }

    void SetFocusWindowId(uint32_t windowId)
    {
        focusWindowId_ = windowId;
    }

    uint32_t GetFocusWindowId() const
    {
        return focusWindowId_.value_or(windowId_);
    }

    bool IsFocusWindowIdSetted() const
    {
        return focusWindowId_.has_value();
    }

    float GetViewScale() const
    {
        return viewScale_;
    }

    double GetRootWidth() const
    {
        return rootWidth_;
    }

    double GetRootHeight() const
    {
        return rootHeight_;
    }

    void SetWindowModal(WindowModal modal)
    {
        windowModal_ = modal;
    }

    WindowModal GetWindowModal() const
    {
        return windowModal_;
    }

    bool IsFullScreenModal() const
    {
        return windowModal_ == WindowModal::NORMAL || windowModal_ == WindowModal::SEMI_MODAL_FULL_SCREEN ||
               windowModal_ == WindowModal::CONTAINER_MODAL || isFullWindow_;
    }

    void SetIsRightToLeft(bool isRightToLeft)
    {
        isRightToLeft_ = isRightToLeft;
    }

    bool IsRightToLeft() const
    {
        return isRightToLeft_;
    }

    void SetEventManager(const RefPtr<EventManager>& eventManager)
    {
        eventManager_ = eventManager;
    }

    RefPtr<EventManager> GetEventManager() const
    {
        return eventManager_;
    }

    const RefPtr<WindowManager>& GetWindowManager() const
    {
        return windowManager_;
    }

    bool HasFloatTitle() const;

    bool IsRebuildFinished() const
    {
        return isRebuildFinished_;
    }

    void RequestFrame();

    void RegisterFont(const std::string& familyName, const std::string& familySrc, const std::string& bundleName = "",
        const std::string& moduleName = "");

    void GetSystemFontList(std::vector<std::string>& fontList);

    bool GetSystemFont(const std::string& fontName, FontInfo& fontInfo);

    void GetUIFontConfig(FontConfigJsonInfo& fontConfigJsonInfo);

    void TryLoadImageInfo(const std::string& src, std::function<void(bool, int32_t, int32_t)>&& loadCallback);

    RefPtr<OffscreenCanvas> CreateOffscreenCanvas(int32_t width, int32_t height);

    void PostAsyncEvent(TaskExecutor::Task&& task, const std::string& name,
        TaskExecutor::TaskType type = TaskExecutor::TaskType::UI);

    void PostAsyncEvent(const TaskExecutor::Task& task, const std::string& name,
        TaskExecutor::TaskType type = TaskExecutor::TaskType::UI);

    void PostSyncEvent(const TaskExecutor::Task& task, const std::string& name,
        TaskExecutor::TaskType type = TaskExecutor::TaskType::UI);

    virtual void FlushReload(const ConfigurationChange& configurationChange) {}
    virtual void FlushBuild() {}

    virtual void FlushReloadTransition() {}
    virtual void RebuildFontNode() {}
    FrontendType GetFrontendType() const
    {
        return frontendType_;
    }

    double GetDensity() const
    {
        return density_;
    }

    RefPtr<PlatformResRegister> GetPlatformResRegister() const
    {
        return platformResRegister_;
    }

    void SetTouchPipeline(const WeakPtr<PipelineBase>& context);
    void RemoveTouchPipeline(const WeakPtr<PipelineBase>& context);

    void OnVirtualKeyboardAreaChange(Rect keyboardArea,
        const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr, const float safeHeight = 0.0f,
        bool supportAvoidance = false);
    void OnVirtualKeyboardAreaChange(Rect keyboardArea, double positionY, double height,
        const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr, bool forceChange = false);

    void OnFoldStatusChanged(FoldStatus foldStatus);

    using foldStatusChangedCallback = std::function<bool(FoldStatus)>;
    void SetFoldStatusChangeCallback(foldStatusChangedCallback&& listener)
    {
        foldStatusChangedCallback_.emplace_back(std::move(listener));
    }

    void OnFoldDisplayModeChanged(FoldDisplayMode foldDisplayMode);

    using virtualKeyBoardCallback = std::function<bool(int32_t, int32_t, double)>;
    void SetVirtualKeyBoardCallback(virtualKeyBoardCallback&& listener)
    {
        virtualKeyBoardCallback_.push_back(std::move(listener));
    }
    bool NotifyVirtualKeyBoard(int32_t width, int32_t height, double keyboard) const
    {
        bool isConsume = false;
        for (const auto& iterVirtualKeyBoardCallback : virtualKeyBoardCallback_) {
            if (iterVirtualKeyBoardCallback && iterVirtualKeyBoardCallback(width, height, keyboard)) {
                isConsume = true;
            }
        }
        return isConsume;
    }

    using configChangedCallback = std::function<void()>;
    void SetConfigChangedCallback(configChangedCallback&& listener)
    {
        configChangedCallback_.push_back(std::move(listener));
    }

    void NotifyConfigurationChange()
    {
        for (const auto& callback : configChangedCallback_) {
            if (callback) {
                callback();
            }
        }
    }

    using PostRTTaskCallback = std::function<void(std::function<void()>&&)>;
    void SetPostRTTaskCallBack(PostRTTaskCallback&& callback)
    {
        postRTTaskCallback_ = std::move(callback);
    }

    void PostTaskToRT(std::function<void()>&& task)
    {
        if (postRTTaskCallback_) {
            postRTTaskCallback_(std::move(task));
        }
    }

    void SetGetWindowRectImpl(std::function<Rect()>&& callback);

    Rect GetCurrentWindowRect() const;

    using SafeAreaInsets = NG::SafeAreaInsets;
    virtual void UpdateSystemSafeArea(const SafeAreaInsets& systemSafeArea) {}

    virtual void UpdateCutoutSafeArea(const SafeAreaInsets& cutoutSafeArea) {}

    virtual void UpdateNavSafeArea(const SafeAreaInsets& navSafeArea) {}

    virtual void UpdateOriginAvoidArea(const Rosen::AvoidArea& avoidArea, uint32_t type) {}

    virtual void SetEnableKeyBoardAvoidMode(bool value) {}

    virtual bool IsEnableKeyBoardAvoidMode() {
        return false;
    }

    void SetPluginOffset(const Offset& offset)
    {
        pluginOffset_ = offset;
    }

    Offset GetPluginOffset() const
    {
        return pluginOffset_;
    }

    void SetPluginEventOffset(const Offset& offset)
    {
        pluginEventOffset_ = offset;
    }

    Offset GetPluginEventOffset() const
    {
        return pluginEventOffset_;
    }
    virtual void NotifyMemoryLevel(int32_t level) {}

    void SetDisplayWindowRectInfo(const Rect& displayWindowRectInfo)
    {
        displayWindowRectInfo_ = displayWindowRectInfo;
    }

    virtual void SetContainerWindow(bool isShow) = 0;

    // This method can get the coordinates and size of the current window,
    // which can be added to the return value of the GetGlobalOffset method to get the window coordinates of the node.
    const Rect& GetDisplayWindowRectInfo() const
    {
        return displayWindowRectInfo_;
    }
    virtual void FlushModifier() {}
    virtual void FlushMessages() = 0;
    void SetGSVsyncCallback(std::function<void(void)>&& callback)
    {
        gsVsyncCallback_ = std::move(callback);
    }

    virtual void FlushUITasks(bool triggeredByImplicitAnimation = false) = 0;

    virtual void FlushAfterLayoutCallbackInImplicitAnimationTask() {}

    virtual void FlushPipelineImmediately() = 0;

    virtual void FlushOnceVsyncTask() = 0;

    // get animateTo closure option
    AnimationOption GetSyncAnimationOption()
    {
        return animationOption_;
    }

    void SetSyncAnimationOption(const AnimationOption& option)
    {
        animationOption_ = option;
    }

    void SetKeyboardAnimationConfig(const KeyboardAnimationConfig& config)
    {
        keyboardAnimationConfig_ = config;
    }

    void SetNextFrameLayoutCallback(std::function<void()>&& callback)
    {
        nextFrameLayoutCallback_ = std::move(callback);
    }

    void SetForegroundCalled(bool isForegroundCalled)
    {
        isForegroundCalled_ = isForegroundCalled;
    }

    void SetIsSubPipeline(bool isSubPipeline)
    {
        isSubPipeline_ = isSubPipeline;
    }

    bool IsSubPipeline() const
    {
        return isSubPipeline_;
    }

    void SetParentPipeline(const WeakPtr<PipelineBase>& pipeline)
    {
        parentPipeline_ = pipeline;
    }

    void AddEtsCardTouchEventCallback(int32_t ponitId, EtsCardTouchEventCallback&& callback);

    void HandleEtsCardTouchEvent(const TouchEvent& point, SerializedGesture &serializedGesture);

    void RemoveEtsCardTouchEventCallback(int32_t ponitId);

    void SetSubWindowVsyncCallback(AceVsyncCallback&& callback, int32_t subWindowId);

    void SetJsFormVsyncCallback(AceVsyncCallback&& callback, int32_t subWindowId);

    void RemoveSubWindowVsyncCallback(int32_t subWindowId);

    void RemoveJsFormVsyncCallback(int32_t subWindowId);

    virtual void SetIsLayoutFullScreen(bool isLayoutFullScreen) {}
    virtual void SetIsNeedAvoidWindow(bool isLayoutFullScreen) {}
    virtual void SetIgnoreViewSafeArea(bool ignoreViewSafeArea) {}
    virtual void OnFoldStatusChange(FoldStatus foldStatus) {}
    virtual void OnFoldDisplayModeChange(FoldDisplayMode foldDisplayMode) {}

    void SetIsAppWindow(bool isAppWindow)
    {
        isAppWindow_ = isAppWindow;
    }

    bool GetIsAppWindow() const
    {
        return isAppWindow_;
    }

    void SetFormAnimationStartTime(int64_t time)
    {
        formAnimationStartTime_ = time;
    }

    int64_t GetFormAnimationStartTime() const
    {
        return formAnimationStartTime_;
    }

    void SetIsFormAnimation(bool isFormAnimation)
    {
        isFormAnimation_ = isFormAnimation;
    }

    bool IsFormAnimation() const
    {
        return isFormAnimation_;
    }

    void SetFormAnimationFinishCallback(bool isFormAnimationFinishCallback)
    {
        isFormAnimationFinishCallback_ = isFormAnimationFinishCallback;
    }

    bool IsFormAnimationFinishCallback() const
    {
        return isFormAnimationFinishCallback_;
    }

    // restore
    virtual void RestoreNodeInfo(std::unique_ptr<JsonValue> nodeInfo) {}

    virtual std::unique_ptr<JsonValue> GetStoredNodeInfo()
    {
        return nullptr;
    }

    uint64_t GetLastTouchTime() const
    {
        return lastTouchTime_;
    }

    void AddFormLinkInfo(int32_t id, const std::string& info)
    {
        LOGI("AddFormLinkInfo is %{public}s, id is %{public}d", info.c_str(), id);
        formLinkInfoMap_[id] = info;
    }

    virtual bool IsLayouting() const
    {
        return false;
    }

    void SetHalfLeading(bool halfLeading)
    {
        halfLeading_ = halfLeading;
    }

    bool GetHalfLeading() const
    {
        return halfLeading_;
    }

    void SetUseCutout(bool useCutout)
    {
        useCutout_ = useCutout;
    }

    bool GetUseCutout() const
    {
        return useCutout_;
    }

    bool GetOnFoucs() const
    {
        return onFocus_;
    }

    void SetSurfaceChangeMsg(int32_t width, int32_t height,
                                WindowSizeChangeReason type,
                                const std::shared_ptr<Rosen::RSTransaction>& rsTransaction)
    {
        width_ = width;
        height_ = height;
        type_ = type;
        rsTransaction_ = rsTransaction;
        delaySurfaceChange_ = true;
    }

    void ResetSurfaceChangeMsg()
    {
        delaySurfaceChange_ = false;
    }

    uint64_t GetVsyncTime() const
    {
        return vsyncTime_;
    }

    void SetVsyncTime(uint64_t time)
    {
        vsyncTime_ = time;
    }

    virtual void UpdateCurrentActiveNode(const WeakPtr<NG::FrameNode>& node) {}

    virtual std::string GetCurrentExtraInfo() { return ""; }
    virtual void UpdateTitleInTargetPos(bool isShow = true, int32_t height = 0) {}

    virtual void SetCursor(int32_t cursorValue) {}

    virtual void RestoreDefault(int32_t windowId = 0) {}

    void SetOnFormRecycleCallback(std::function<std::string()>&& onFormRecycle)
    {
        onFormRecycle_ = std::move(onFormRecycle);
    }

    std::string OnFormRecycle();

    void SetOnFormRecoverCallback(std::function<void(std::string)>&& onFormRecover)
    {
        onFormRecover_ = std::move(onFormRecover);
    }

    void OnFormRecover(const std::string& statusData);

    virtual bool IsDragging() const
    {
        return false;
    }

    virtual void SetIsDragging(bool isDragging) {}

    virtual void ResetDragging() {}

    virtual const SerializedGesture& GetSerializedGesture() const
    {
        return serializedGesture_;
    }

    virtual bool PrintVsyncInfoIfNeed() const
    {
        return false;
    }

    virtual void CheckVirtualKeyboardHeight() {}

    virtual void StartWindowAnimation() {}

    virtual void StopWindowAnimation() {}

    virtual void AddSyncGeometryNodeTask(std::function<void()>&& task) {}

    virtual void FlushSyncGeometryNodeTasks() {}

    virtual void ChangeSensitiveNodes(bool flag) {}

    virtual bool IsContainerModalVisible()
    {
        return false;
    }

    void SetStateProfilerStatus(bool stateProfilerStatus)
    {
        stateProfilerStatus_ = stateProfilerStatus;
        if (jsStateProfilerStatusCallback_) {
            jsStateProfilerStatusCallback_(stateProfilerStatus);
        }
    }

    void SetStateProfilerStatusCallback(std::function<void(bool)>&& callback)
    {
        jsStateProfilerStatusCallback_ = callback;
    }

    bool GetStateProfilerStatus() const
    {
        return stateProfilerStatus_;
    }

    uint32_t GetFrameCount() const
    {
        return frameCount_;
    }

    virtual void CheckAndLogLastReceivedTouchEventInfo(int32_t eventId, TouchType type) {}

    virtual void CheckAndLogLastConsumedTouchEventInfo(int32_t eventId, TouchType type) {}

    virtual void CheckAndLogLastReceivedMouseEventInfo(int32_t eventId, MouseAction action) {}

    virtual void CheckAndLogLastConsumedMouseEventInfo(int32_t eventId, MouseAction action) {}

    virtual void CheckAndLogLastReceivedAxisEventInfo(int32_t eventId, AxisAction action) {}

    virtual void CheckAndLogLastConsumedAxisEventInfo(int32_t eventId, AxisAction action) {}

protected:
    virtual bool MaybeRelease() override;
    void TryCallNextFrameLayoutCallback()
    {
        if (isForegroundCalled_ && nextFrameLayoutCallback_) {
            isForegroundCalled_ = false;
            nextFrameLayoutCallback_();
            LOGI("nextFrameLayoutCallback called");
        }
    }

    virtual bool OnDumpInfo(const std::vector<std::string>& params) const
    {
        return false;
    }
    virtual void FlushVsync(uint64_t nanoTimestamp, uint32_t frameCount) = 0;
    virtual void SetRootRect(double width, double height, double offset = 0.0) = 0;
    virtual void FlushPipelineWithoutAnimation() = 0;

    virtual void OnVirtualKeyboardHeightChange(float keyboardHeight,
        const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr, const float safeHeight = 0.0f,
        const bool supportAvoidance = false)
    {}
    virtual void OnVirtualKeyboardHeightChange(float keyboardHeight, double positionY, double height,
        const std::shared_ptr<Rosen::RSTransaction>& rsTransaction = nullptr, bool forceChange = false)
    {}

    void UpdateRootSizeAndScale(int32_t width, int32_t height);

    void SetIsReloading(bool isReloading)
    {
        isReloading_ = isReloading;
    }

    std::function<void()> GetWrappedAnimationCallback(const std::function<void()>& finishCallback);

    std::list<configChangedCallback> configChangedCallback_;
    std::list<virtualKeyBoardCallback> virtualKeyBoardCallback_;
    std::list<foldStatusChangedCallback> foldStatusChangedCallback_;

    bool isRebuildFinished_ = false;
    bool isJsCard_ = false;
    bool isFormRender_ = false;
    bool isRightToLeft_ = false;
    bool isFullWindow_ = false;
    bool isAppWindow_ = true;
    bool installationFree_ = false;
    bool isSubPipeline_ = false;
    bool isReloading_ = false;

    bool isJsPlugin_ = false;

    std::unordered_map<int32_t, AceVsyncCallback> subWindowVsyncCallbacks_;
    std::unordered_map<int32_t, AceVsyncCallback> jsFormVsyncCallbacks_;
    int32_t minPlatformVersion_ = 0;
    uint32_t windowId_ = 0;
    // UIExtensionAbility need component windowID
    std::optional<uint32_t> focusWindowId_;

    int32_t appLabelId_ = 0;
    float fontScale_ = 1.0f;
    float fontWeightScale_ = 1.0f;
    float designWidthScale_ = 1.0f;
    float viewScale_ = 1.0f;
    double density_ = 1.0;
    double dipScale_ = 1.0;
    double rootHeight_ = 0.0;
    double rootWidth_ = 0.0;
    FrontendType frontendType_;
    WindowModal windowModal_ = WindowModal::NORMAL;

    Offset pluginOffset_ { 0, 0 };
    Offset pluginEventOffset_ { 0, 0 };
    Color appBgColor_ = Color::WHITE;
    int8_t renderingMode_ = 0;

    std::unique_ptr<DrawDelegate> drawDelegate_;
    std::stack<bool> pendingImplicitLayout_;
    std::stack<bool> pendingImplicitRender_;
    std::stack<bool> pendingFrontendAnimation_;
    std::shared_ptr<Window> window_;
    RefPtr<TaskExecutor> taskExecutor_;
    RefPtr<AssetManager> assetManager_;
    WeakPtr<Frontend> weakFrontend_;
    int32_t instanceId_ = 0;
    RefPtr<EventManager> eventManager_;
    RefPtr<ImageCache> imageCache_;
    RefPtr<SharedImageManager> sharedImageManager_;
    mutable std::shared_mutex imageMtx_;
    mutable std::shared_mutex themeMtx_;
    mutable std::mutex destructMutex_;
    RefPtr<ThemeManager> themeManager_;
    RefPtr<DataProviderManagerInterface> dataProviderManager_;
    RefPtr<FontManager> fontManager_;
    RefPtr<ManagerInterface> textFieldManager_;
    RefPtr<PlatformBridge> messageBridge_;
    RefPtr<WindowManager> windowManager_;
    OnPageShowCallBack onPageShowCallBack_;
    AnimationCallback animationCallback_;
    ProfilerCallback onVsyncProfiler_;
    FinishEventHandler finishEventHandler_;
    StartAbilityHandler startAbilityHandler_;
    ActionEventHandler actionEventHandler_;
    FormLinkInfoUpdateHandler formLinkInfoUpdateHandler_;
    RefPtr<PlatformResRegister> platformResRegister_;

    WeakPtr<PipelineBase> parentPipeline_;

    std::vector<WeakPtr<PipelineBase>> touchPluginPipelineContext_;
    std::unordered_map<int32_t, EtsCardTouchEventCallback> etsCardTouchEventCallback_;

    RefPtr<Clipboard> clipboard_;
    std::function<void(const std::string&)> clipboardCallback_ = nullptr;
    Rect displayWindowRectInfo_;
    AnimationOption animationOption_;
    KeyboardAnimationConfig keyboardAnimationConfig_;

    std::function<void()> nextFrameLayoutCallback_ = nullptr;
    SharePanelCallback sharePanelCallback_ = nullptr;
    std::atomic<bool> isForegroundCalled_ = false;
    std::atomic<bool> onFocus_ = true;
    uint64_t lastTouchTime_ = 0;
    std::map<int32_t, std::string> formLinkInfoMap_;
    struct FunctionHash {
        std::size_t operator()(const std::shared_ptr<std::function<void()>>& functionPtr) const
        {
            return std::hash<std::function<void()>*>()(functionPtr.get());
        }
    };
    std::function<std::string()> onFormRecycle_;
    std::function<void(std::string)> onFormRecover_;

    std::once_flag displaySyncFlag_;
    RefPtr<UIDisplaySyncManager> uiDisplaySyncManager_;

    SerializedGesture serializedGesture_;
private:
    void DumpFrontend() const;
    double ModifyKeyboardHeight(double keyboardHeight) const;
    StatusBarEventHandler statusBarBgColorEventHandler_;
    PopupEventHandler popupEventHandler_;
    MenuEventHandler menuEventHandler_;
    ContextMenuEventHandler contextMenuEventHandler_;
    RouterBackEventHandler routerBackEventHandler_;
    std::list<PopPageSuccessEventHandler> popPageSuccessEventHandler_;
    std::list<IsPagePathInvalidEventHandler> isPagePathInvalidEventHandler_;
    std::list<DestroyEventHandler> destroyEventHandler_;
    std::list<DispatchTouchEventHandler> dispatchTouchEventHandler_;
    GetViewScaleCallback getViewScaleCallback_;
    // OnRouterChangeCallback is function point, need to be initialized.
    OnRouterChangeCallback onRouterChangeCallback_ = nullptr;
    PostRTTaskCallback postRTTaskCallback_;
    std::function<void(void)> gsVsyncCallback_;
    std::unordered_set<std::shared_ptr<std::function<void()>>, FunctionHash> finishFunctions_;
    bool isFormAnimationFinishCallback_ = false;
    int64_t formAnimationStartTime_ = 0;
    bool isFormAnimation_ = false;
    bool halfLeading_ = false;
    bool useCutout_ = false;
    uint64_t vsyncTime_ = 0;

    bool delaySurfaceChange_ = false;
    int32_t width_ = -1;
    int32_t height_ = -1;
    WindowSizeChangeReason type_ = WindowSizeChangeReason::UNDEFINED;
    std::shared_ptr<Rosen::RSTransaction> rsTransaction_;
    uint32_t frameCount_ = 0;
    bool stateProfilerStatus_ = false;
    std::function<void(bool)> jsStateProfilerStatusCallback_;

    int32_t densityChangeCallbackId_ = 0;
    std::unordered_map<int32_t, std::function<void(double)>> densityChangedCallbacks_;

    ACE_DISALLOW_COPY_AND_MOVE(PipelineBase);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_PIPELINE_PIPELINE_BASE_H
