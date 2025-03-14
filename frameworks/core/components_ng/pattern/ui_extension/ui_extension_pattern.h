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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_PATTERN_H

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <refbase.h>
#include <vector>

#include "base/memory/referenced.h"
#include "base/want/want_wrap.h"
#include "core/common/container.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/ui_extension/session_wrapper.h"
#include "core/components_ng/pattern/ui_extension/accessibility_session_adapter_ui_extension.h"
#include "core/event/mouse_event.h"
#include "core/event/touch_event.h"

#define UIEXT_LOGD(fmt, ...)                                                                                      \
    TAG_LOGD(AceLogTag::ACE_UIEXTENSIONCOMPONENT, "[@%{public}d][ID: %{public}d] " fmt, __LINE__, uiExtensionId_, \
        ##__VA_ARGS__)
#define UIEXT_LOGI(fmt, ...)                                                                                      \
    TAG_LOGI(AceLogTag::ACE_UIEXTENSIONCOMPONENT, "[@%{public}d][ID: %{public}d] " fmt, __LINE__, uiExtensionId_, \
        ##__VA_ARGS__)
#define UIEXT_LOGW(fmt, ...)                                                                                      \
    TAG_LOGW(AceLogTag::ACE_UIEXTENSIONCOMPONENT, "[@%{public}d][ID: %{public}d] " fmt, __LINE__, uiExtensionId_, \
        ##__VA_ARGS__)
#define UIEXT_LOGE(fmt, ...)                                                                                      \
    TAG_LOGE(AceLogTag::ACE_UIEXTENSIONCOMPONENT, "[@%{public}d][ID: %{public}d] " fmt, __LINE__, uiExtensionId_, \
        ##__VA_ARGS__)
#define UIEXT_LOGF(fmt, ...)                                                                                      \
    TAG_LOGF(AceLogTag::ACE_UIEXTENSIONCOMPONENT, "[@%{public}d][ID: %{public}d] " fmt, __LINE__, uiExtensionId_, \
        ##__VA_ARGS__)

namespace OHOS::Accessibility {
class AccessibilityElementInfo;
class AccessibilityEventInfo;
} // namespace OHOS::Accessibility

namespace OHOS::MMI {
class KeyEvent;
class PointerEvent;
} // namespace OHOS::MMI

namespace OHOS::Ace {
class ModalUIExtensionProxy;
} // namespace OHOS::Ace

namespace OHOS::Rosen {
class AvoidArea;
class RSTransaction;
} // namespace OHOS::Rosen

namespace OHOS::Ace::NG {
class UIExtensionProxy;
class UIExtensionPattern : public Pattern {
    DECLARE_ACE_TYPE(UIExtensionPattern, Pattern);

public:
    explicit UIExtensionPattern(bool isTransferringCaller = false, bool isModal = false,
        bool isAsyncModalBinding = false, SessionType sessionType = SessionType::UI_EXTENSION_ABILITY);
    ~UIExtensionPattern() override;

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override;
    FocusPattern GetFocusPattern() const override;
    RefPtr<AccessibilitySessionAdapter> GetAccessibilitySessionAdapter() override;

    void SetPlaceholderNode(const RefPtr<FrameNode>& placeholderNode)
    {
        placeholderNode_ = placeholderNode;
    }
    void UpdateWant(const RefPtr<OHOS::Ace::WantWrap>& wantWrap);
    void UpdateWant(const AAFwk::Want& want);

    void OnWindowShow() override;
    void OnWindowHide() override;
    void OnVisibleChange(bool visible) override;
    void OnMountToParentDone() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    void OnConnect();
    void OnDisconnect(bool isAbnormal);
    void HandleDragEvent(const PointerEvent& info) override;

    void SetModalOnDestroy(const std::function<void()>&& callback);
    void FireModalOnDestroy();
    void SetModalOnRemoteReadyCallback(
        const std::function<void(const std::shared_ptr<ModalUIExtensionProxy>&)>&& callback);
    void SetOnRemoteReadyCallback(const std::function<void(const RefPtr<UIExtensionProxy>&)>&& callback);
    void FireOnRemoteReadyCallback();
    void SetOnReleaseCallback(const std::function<void(int32_t)>&& callback);
    void FireOnReleaseCallback(int32_t releaseCode);
    void SetOnResultCallback(const std::function<void(int32_t, const AAFwk::Want&)>&& callback);
    void FireOnResultCallback(int32_t code, const AAFwk::Want& want);
    void SetOnTerminatedCallback(const std::function<void(int32_t, const RefPtr<WantWrap>&)>&& callback);
    void FireOnTerminatedCallback(int32_t code, const RefPtr<WantWrap>& wantWrap);
    void SetOnReceiveCallback(const std::function<void(const AAFwk::WantParams&)>&& callback);
    void FireOnReceiveCallback(const AAFwk::WantParams& params);
    void SetOnErrorCallback(
        const std::function<void(int32_t code, const std::string& name, const std::string& message)>&& callback);
    virtual void FireOnErrorCallback(int32_t code, const std::string& name, const std::string& message);
    void SetSyncCallbacks(const std::list<std::function<void(const RefPtr<UIExtensionProxy>&)>>&& callbackList);
    void FireSyncCallbacks();
    void SetAsyncCallbacks(const std::list<std::function<void(const RefPtr<UIExtensionProxy>&)>>&& callbackList);
    void FireAsyncCallbacks();
    void SetBindModalCallback(const std::function<void()>&& callback);
    void FireBindModalCallback();
    void DispatchFollowHostDensity(bool densityDpi);
    void OnDpiConfigurationUpdate() override;
    void SetDensityDpi(bool densityDpi);
    bool GetDensityDpi();
    bool IsCompatibleOldVersion();

    void NotifySizeChangeReason(
        WindowSizeChangeReason type, const std::shared_ptr<Rosen::RSTransaction>& rsTransaction);
    void NotifyForeground();
    void NotifyBackground();
    void NotifyDestroy();
    int32_t GetInstanceId();
    int32_t GetSessionId();
    int32_t GetNodeId();
    int32_t GetUiExtensionId() override;
    int64_t WrapExtensionAbilityId(int64_t extensionOffset, int64_t abilityId) override;
    void DispatchOriginAvoidArea(const Rosen::AvoidArea& avoidArea, uint32_t type);
    void SetWantWrap(const RefPtr<OHOS::Ace::WantWrap>& wantWrap);
    RefPtr<OHOS::Ace::WantWrap> GetWantWrap();
    bool IsShowPlaceholder()
    {
        return isShowPlaceholder_;
    }

    virtual void SearchExtensionElementInfoByAccessibilityId(int64_t elementId, int32_t mode, int64_t baseParent,
        std::list<Accessibility::AccessibilityElementInfo>& output) override;
    virtual void SearchElementInfosByText(int64_t elementId, const std::string& text, int64_t baseParent,
        std::list<Accessibility::AccessibilityElementInfo>& output) override;
    virtual void FindFocusedElementInfo(int64_t elementId, int32_t focusType, int64_t baseParent,
        Accessibility::AccessibilityElementInfo& output) override;
    virtual void FocusMoveSearch(int64_t elementId, int32_t direction, int64_t baseParent,
        Accessibility::AccessibilityElementInfo& output) override;
    virtual bool TransferExecuteAction(int64_t elementId, const std::map<std::string, std::string>& actionArguments,
        int32_t action, int64_t offset) override;
    void OnAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info, int64_t uiExtensionOffset);

protected:
    virtual void DispatchPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    virtual void DispatchKeyEvent(const KeyEvent& event);

    int32_t uiExtensionId_ = 0;
    int32_t instanceId_ = Container::CurrentId();
    std::function<void(int32_t code, const std::string& name, const std::string& message)> onErrorCallback_;

private:
    enum class AbilityState {
        NONE = 0,
        FOREGROUND,
        BACKGROUND,
        DESTRUCTION,
    };

    struct ErrorMsg {
        int32_t code = 0;
        std::string name;
        std::string message;
    };

    const char* ToString(AbilityState state);
    void OnAttachToFrameNode() override;
    void OnDetachFromFrameNode(FrameNode* frameNode) override;
    void OnLanguageConfigurationUpdate() override;
    void OnColorConfigurationUpdate() override;
    void OnModifyDone() override;

    void InitKeyEvent(const RefPtr<FocusHub>& focusHub);
    void InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub);
    void InitMouseEvent(const RefPtr<InputEventHub>& inputHub);
    void InitHoverEvent(const RefPtr<InputEventHub>& inputHub);
    bool HandleKeyEvent(const KeyEvent& event);
    void HandleFocusEvent();
    void HandleBlurEvent();
    void HandleTouchEvent(const TouchEventInfo& info);
    void HandleMouseEvent(const MouseInfo& info);
    void HandleHoverEvent(bool isHover);
    bool DispatchKeyEventSync(const KeyEvent& event);
    void DispatchFocusActiveEvent(bool isFocusActive);
    void DispatchFocusState(bool focusState);
    void DispatchDisplayArea(bool isForce = false);

    void RegisterVisibleAreaChange();
    void MountPlaceholderNode();
    void RemovePlaceholderNode();
    bool ShouldCallSystem(const AAFwk::Want& want);

    RefPtr<TouchEventImpl> touchEvent_;
    RefPtr<InputEvent> mouseEvent_;
    RefPtr<InputEvent> hoverEvent_;
    std::shared_ptr<MMI::PointerEvent> lastPointerEvent_ = nullptr;

    std::function<void()> onModalDestroy_;
    std::function<void(const std::shared_ptr<ModalUIExtensionProxy>&)> onModalRemoteReadyCallback_;
    std::function<void(const RefPtr<UIExtensionProxy>&)> onRemoteReadyCallback_;
    std::function<void(int32_t)> onReleaseCallback_;
    std::function<void(int32_t, const AAFwk::Want&)> onResultCallback_;
    std::function<void(int32_t, const RefPtr<WantWrap>&)> onTerminatedCallback_;
    std::function<void(const AAFwk::WantParams&)> onReceiveCallback_;
    std::list<std::function<void(const RefPtr<UIExtensionProxy>&)>> onSyncOnCallbackList_;
    std::list<std::function<void(const RefPtr<UIExtensionProxy>&)>> onAsyncOnCallbackList_;
    std::function<void()> bindModalCallback_;
    RefPtr<FrameNode> placeholderNode_ = nullptr;

    RefPtr<OHOS::Ace::WantWrap> curWant_;
    RefPtr<FrameNode> contentNode_;
    RefPtr<SessionWrapper> sessionWrapper_;
    RefPtr<AccessibilitySessionAdapterUIExtension> accessibilitySessionAdapter_;
    ErrorMsg lastError_;
    AbilityState state_ = AbilityState::NONE;
    bool isTransferringCaller_ = false;
    bool isVisible_ = true;
    bool isModal_ = false;
    bool isAsyncModalBinding_ = false;
    bool isShowPlaceholder_ = false;
    bool densityDpi_ = false;
    int32_t callbackId_ = 0;
    RectF displayArea_;
    bool isKeyAsync_ = false;
    SessionType sessionType_ = SessionType::UI_EXTENSION_ABILITY;

    ACE_DISALLOW_COPY_AND_MOVE(UIExtensionPattern);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_PATTERN_H
