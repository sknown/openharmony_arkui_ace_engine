/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/ui_extension/security_session_wrapper_impl.h"

#include <cmath>
#include <memory>

#include "accessibility_event_info.h"
#include "refbase.h"
#include "session_manager/include/extension_session_manager.h"
#include "transaction/rs_sync_transaction_controller.h"
#include "transaction/rs_transaction.h"
#include "ui/rs_surface_node.h"
#include "want_params.h"
#include "wm/wm_common.h"

#include "adapter/ohos/entrance/ace_container.h"
#include "adapter/ohos/osal/want_wrap_ohos.h"
#include "base/error/error_code.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/components_ng/pattern/ui_extension/session_wrapper.h"
#include "core/components_ng/pattern/window_scene/helper/window_scene_helper.h"
#include "core/components_ng/pattern/window_scene/scene/system_window_scene.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
// Defines all error names and messages.
constexpr char START_FAIL_NAME[] = "start_ability_fail";
constexpr char START_FAIL_MESSAGE[] = "Start ui extension ability failed, please check the want of UIextensionAbility.";
constexpr char BACKGROUND_FAIL_NAME[] = "background_fail";
constexpr char BACKGROUND_FAIL_MESSAGE[] = "background ui extension ability failed, please check AMS log.";
constexpr char TERMINATE_FAIL_NAME[] = "terminate_fail";
constexpr char TERMINATE_FAIL_MESSAGE[] = "terminate ui extension ability failed, please check AMS log.";
constexpr char PULL_FAIL_NAME[] = "extension_pulling_up_fail";
constexpr char PULL_FAIL_MESSAGE[] = "pulling another embedded component failed, not allowed to cascade.";
constexpr char EXIT_ABNORMALLY_NAME[] = "extension_exit_abnormally";
constexpr char EXIT_ABNORMALLY_MESSAGE[] = "the extension ability exited abnormally, please check AMS log.";
constexpr char LIFECYCLE_TIMEOUT_NAME[] = "extension_lifecycle_timeout";
constexpr char LIFECYCLE_TIMEOUT_MESSAGE[] = "the lifecycle of extension ability is timeout, please check AMS log.";
// Defines the want parameter to control the soft-keyboard area change of the provider.
constexpr char OCCUPIED_AREA_CHANGE_KEY[] = "ability.want.params.IsNotifyOccupiedAreaChange";
} // namespace

class SecurityUIExtensionLifecycleListener : public Rosen::ILifecycleListener {
public:
    explicit SecurityUIExtensionLifecycleListener(const WeakPtr<SessionWrapper>& sessionWrapper)
        : sessionWrapper_(sessionWrapper) {}
    virtual ~SecurityUIExtensionLifecycleListener() = default;

    void OnActivation() override {}
    void OnForeground() override {}
    void OnBackground() override {}

    void OnConnect() override
    {
        auto sessionWrapper = sessionWrapper_.Upgrade();
        CHECK_NULL_VOID(sessionWrapper);
        sessionWrapper->OnConnect();
    }

    void OnDisconnect() override
    {
        auto sessionWrapper = sessionWrapper_.Upgrade();
        CHECK_NULL_VOID(sessionWrapper);
        sessionWrapper->OnDisconnect(false);
    }

    void OnExtensionDied() override
    {
        auto sessionWrapper = sessionWrapper_.Upgrade();
        CHECK_NULL_VOID(sessionWrapper);
        sessionWrapper->OnDisconnect(true);
    }

    void OnExtensionTimeout(int32_t errorCode) override
    {
        auto sessionWrapper = sessionWrapper_.Upgrade();
        CHECK_NULL_VOID(sessionWrapper);
        sessionWrapper->OnExtensionTimeout(errorCode);
    }

    void OnAccessibilityEvent(
        const Accessibility::AccessibilityEventInfo& info, int64_t uiExtensionOffset) override
    {
        auto sessionWrapper = sessionWrapper_.Upgrade();
        CHECK_NULL_VOID(sessionWrapper);
        sessionWrapper->OnAccessibilityEvent(info, uiExtensionOffset);
    }

private:
    WeakPtr<SessionWrapper> sessionWrapper_;
};

/********************* Begin: Initialization *************************/
SecuritySessionWrapperImpl::SecuritySessionWrapperImpl(
    const WeakPtr<SecurityUIExtensionPattern>& hostPattern,
    int32_t instanceId, bool isTransferringCaller, SessionType sessionType)
    : hostPattern_(hostPattern), instanceId_(instanceId), isTransferringCaller_(isTransferringCaller),
      sessionType_(sessionType)
{
    auto pattern = hostPattern.Upgrade();
    platformId_ = pattern ? pattern->GetUiExtensionId() : 0;
    taskExecutor_ = Container::CurrentTaskExecutor();
}

SecuritySessionWrapperImpl::~SecuritySessionWrapperImpl() {}

void SecuritySessionWrapperImpl::InitAllCallback()
{
    CHECK_NULL_VOID(session_);
    auto sessionCallbacks = session_->GetExtensionSessionEventCallback();
    int32_t callSessionId = GetSessionId();
    foregroundCallback_ =
        [weak = hostPattern_, taskExecutor = taskExecutor_, callSessionId](OHOS::Rosen::WSError errcode) {
        if (errcode != OHOS::Rosen::WSError::WS_OK) {
            taskExecutor->PostTask(
                [weak, errcode, callSessionId] {
                    auto pattern = weak.Upgrade();
                    CHECK_NULL_VOID(pattern);
                    if (callSessionId != pattern->GetSessionId()) {
                        LOGW("[AceSecurityUiExtension]foregroundCallback: The callSessionId(%{public}d)"
                             " is inconsistent with the curSession(%{public}d)",
                            callSessionId, pattern->GetSessionId());
                        return;
                    }
                    pattern->FireOnErrorCallback(
                        ERROR_CODE_UIEXTENSION_FOREGROUND_FAILED, START_FAIL_NAME, START_FAIL_MESSAGE);
                },
                TaskExecutor::TaskType::UI, "ArkUIUIExtensionForegroundError");
        }
    };
    backgroundCallback_ = [weak = hostPattern_,
        taskExecutor = taskExecutor_, callSessionId](OHOS::Rosen::WSError errcode) {
            if (errcode != OHOS::Rosen::WSError::WS_OK) {
                taskExecutor->PostTask(
                    [weak, errcode, callSessionId] {
                        auto pattern = weak.Upgrade();
                        CHECK_NULL_VOID(pattern);
                        if (callSessionId != pattern->GetSessionId()) {
                            LOGW("[AceSecurityUiExtension]backgroundCallback: The callSessionId(%{public}d)"
                                " is inconsistent with the curSession(%{public}d)",
                                callSessionId, pattern->GetSessionId());
                            return;
                        }
                        pattern->FireOnErrorCallback(ERROR_CODE_UIEXTENSION_BACKGROUND_FAILED,
                            BACKGROUND_FAIL_NAME, BACKGROUND_FAIL_MESSAGE);
                    },
                    TaskExecutor::TaskType::UI, "ArkUIUIExtensionBackgroundError");
            }
        };
    destructionCallback_ = [weak = hostPattern_,
        taskExecutor = taskExecutor_, callSessionId](OHOS::Rosen::WSError errcode) {
            if (errcode != OHOS::Rosen::WSError::WS_OK) {
                taskExecutor->PostTask(
                    [weak, errcode, callSessionId] {
                        auto pattern = weak.Upgrade();
                        CHECK_NULL_VOID(pattern);
                        if (callSessionId != pattern->GetSessionId()) {
                            LOGW("[AceSecurityUiExtension]destructCallback: The callSessionId(%{public}d)"
                                " is inconsistent with the curSession(%{public}d)",
                                callSessionId, pattern->GetSessionId());
                            return;
                        }
                        pattern->FireOnErrorCallback(ERROR_CODE_UIEXTENSION_DESTRUCTION_FAILED,
                            TERMINATE_FAIL_NAME, TERMINATE_FAIL_MESSAGE);
                    },
                    TaskExecutor::TaskType::UI, "ArkUIUIExtensionDestructionError");
            }
        };
    sessionCallbacks->transferAbilityResultFunc_ = [weak = hostPattern_, taskExecutor = taskExecutor_,
        sessionType = sessionType_, callSessionId](int32_t code, const AAFwk::Want& want) {
            taskExecutor->PostTask(
                [weak, code, want, sessionType, callSessionId]() {
                    auto pattern = weak.Upgrade();
                    CHECK_NULL_VOID(pattern);
                    if (callSessionId != pattern->GetSessionId()) {
                        LOGW("[AceSecurityUiExtension]transferAbilityResult: The callSessionId(%{public}d)"
                            " is inconsistent with the curSession(%{public}d)",
                            callSessionId, pattern->GetSessionId());
                        return;
                    }
                    pattern->FireOnTerminatedCallback(code, MakeRefPtr<WantWrapOhos>(want));
                },
                TaskExecutor::TaskType::UI, "ArkUIUIExtensionTransferAbilityResult");
        };
    sessionCallbacks->transferExtensionDataFunc_ = [weak = hostPattern_,
        taskExecutor = taskExecutor_, callSessionId](const AAFwk::WantParams& params) {
            taskExecutor->PostTask(
                [weak, params, callSessionId]() {
                    auto pattern = weak.Upgrade();
                    CHECK_NULL_VOID(pattern);
                    if (callSessionId != pattern->GetSessionId()) {
                        LOGW("[AceSecurityUiExtension]transferExtensionData: The callSessionId(%{public}d)"
                            " is inconsistent with the curSession(%{public}d)",
                            callSessionId, pattern->GetSessionId());
                        return;
                    }
                    pattern->FireOnReceiveCallback(params);
                },
                TaskExecutor::TaskType::UI, "ArkUIUIExtensionReceiveCallback");
        };
    sessionCallbacks->notifyRemoteReadyFunc_ = [weak = hostPattern_,
        taskExecutor = taskExecutor_, callSessionId]() {
            taskExecutor->PostTask(
                [weak, callSessionId]() {
                    auto pattern = weak.Upgrade();
                    CHECK_NULL_VOID(pattern);
                    if (callSessionId != pattern->GetSessionId()) {
                        LOGW("[AceSecurityUiExtension]notifyRemoteReadyFunc: The callSessionId(%{public}d)"
                            " is inconsistent with the curSession(%{public}d)",
                            callSessionId, pattern->GetSessionId());
                        return;
                    }
                    pattern->FireOnRemoteReadyCallback();
                },
                TaskExecutor::TaskType::UI, "ArkUIUIExtensionRemoteReadyCallback");
        };
    sessionCallbacks->notifySyncOnFunc_ = [weak = hostPattern_,
        taskExecutor = taskExecutor_, callSessionId]() {
            taskExecutor->PostTask(
                [weak, callSessionId]() {
                    auto pattern = weak.Upgrade();
                    CHECK_NULL_VOID(pattern);
                    if (callSessionId != pattern->GetSessionId()) {
                        LOGW("[AceSecurityUiExtension]notifySyncOnFunc: The callSessionId(%{public}d)"
                            " is inconsistent with the curSession(%{public}d)",
                            callSessionId, pattern->GetSessionId());
                        return;
                    }
                    pattern->FireSyncCallbacks();
                },
                TaskExecutor::TaskType::UI, "ArkUIUIExtensionSyncCallbacks");
        };
    sessionCallbacks->notifyAsyncOnFunc_ = [weak = hostPattern_,
        taskExecutor = taskExecutor_, callSessionId]() {
            taskExecutor->PostTask(
                [weak, callSessionId]() {
                    auto pattern = weak.Upgrade();
                    CHECK_NULL_VOID(pattern);
                    if (callSessionId != pattern->GetSessionId()) {
                        LOGW("[AceSecurityUiExtension]notifyAsyncOnFunc: The callSessionId(%{public}d)"
                            " is inconsistent with the curSession(%{public}d)",
                            callSessionId, pattern->GetSessionId());
                        return;
                    }
                    pattern->FireAsyncCallbacks();
                },
                TaskExecutor::TaskType::UI, "ArkUIUIExtensionAsyncCallbacks");
        };
    sessionCallbacks->notifyBindModalFunc_ = [weak = hostPattern_,
        taskExecutor = taskExecutor_, callSessionId]() {
            taskExecutor->PostSyncTask(
                [weak, callSessionId]() {
                    auto pattern = weak.Upgrade();
                    CHECK_NULL_VOID(pattern);
                    if (callSessionId != pattern->GetSessionId()) {
                        LOGW("[AceSecurityUiExtension]notifyBindModalFunc: The callSessionId(%{public}d)"
                            " is inconsistent with the curSession(%{public}d)",
                            callSessionId, pattern->GetSessionId());
                        return;
                    }
                    pattern->FireBindModalCallback();
                },
                TaskExecutor::TaskType::UI, "ArkUIUIExtensionBindModalCallback");
        };
    sessionCallbacks->notifyGetAvoidAreaByTypeFunc_ =
        [instanceId = instanceId_](Rosen::AvoidAreaType type) -> Rosen::AvoidArea {
            Rosen::AvoidArea avoidArea;
            auto container = Platform::AceContainer::GetContainer(instanceId);
            CHECK_NULL_RETURN(container, avoidArea);
            avoidArea = container->GetAvoidAreaByType(type);
            return avoidArea;
        };
}
/*********************** End: Initialization *************************************/

/*********************** Begin: About session ************************************/
void SecuritySessionWrapperImpl::CreateSession(
    const AAFwk::Want& want, bool isAsyncModalBinding, bool isCallerSystem)
{
    PLATFORM_LOGI("The session is created with want = %{private}s", want.ToString().c_str());
    auto container = Platform::AceContainer::GetContainer(instanceId_);
    CHECK_NULL_VOID(container);
    auto wantPtr = std::make_shared<Want>(want);
    if (sessionType_ != SessionType::SECURITY_UI_EXTENSION_ABILITY) {
        PLATFORM_LOGE("The UIExtensionComponent does not allow nested pulling of another.");
        auto pattern = hostPattern_.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->FireOnErrorCallback(
            ERROR_CODE_UIEXTENSION_FORBID_CASCADE, PULL_FAIL_NAME, PULL_FAIL_MESSAGE);
        return;
    }

    isNotifyOccupiedAreaChange_ = want.GetBoolParam(OCCUPIED_AREA_CHANGE_KEY, true);
    auto callerToken = container->GetToken();
    auto parentToken = container->GetParentToken();
    Rosen::SessionInfo extensionSessionInfo = {
        .bundleName_ = want.GetElement().GetBundleName(),
        .abilityName_ = want.GetElement().GetAbilityName(),
        .callerToken_ = callerToken,
        .rootToken_ = (isTransferringCaller_ && parentToken) ? parentToken : callerToken,
        .want = wantPtr,
        .isAsyncModalBinding_ = isAsyncModalBinding,
        .isModal_ = !isCallerSystem,
    };
    session_ = Rosen::ExtensionSessionManager::GetInstance().RequestExtensionSession(extensionSessionInfo);
    CHECK_NULL_VOID(session_);
    lifecycleListener_ = std::make_shared<SecurityUIExtensionLifecycleListener>(
        AceType::WeakClaim(this));
    session_->RegisterLifecycleListener(lifecycleListener_);
    InitAllCallback();
}

void SecuritySessionWrapperImpl::DestroySession()
{
    CHECK_NULL_VOID(session_);
    session_->UnregisterLifecycleListener(lifecycleListener_);
    session_ = nullptr;
}

bool SecuritySessionWrapperImpl::IsSessionValid()
{
    return session_ != nullptr;
}

int32_t SecuritySessionWrapperImpl::GetSessionId()
{
    return session_ ? session_->GetPersistentId() : 0;
}

const std::shared_ptr<AAFwk::Want> SecuritySessionWrapperImpl::GetWant()
{
    return session_ ? session_->GetSessionInfo().want : nullptr;
}
/******************************* End: About session ***************************************/

/******************************* Begin: Synchronous interface for event notify ************/
bool SecuritySessionWrapperImpl::NotifyFocusEventSync(bool isFocus)
{
    return false;
}
bool SecuritySessionWrapperImpl::NotifyFocusStateSync(bool focusState)
{
    return false;
}

bool SecuritySessionWrapperImpl::NotifyBackPressedSync()
{
    return false;
}

bool SecuritySessionWrapperImpl::NotifyPointerEventSync(
    const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent)
{
    return false;
}

bool SecuritySessionWrapperImpl::NotifyKeyEventSync(
    const std::shared_ptr<OHOS::MMI::KeyEvent>& keyEvent, bool isPreIme)
{
    return false;
}

bool SecuritySessionWrapperImpl::NotifyKeyEventAsync(
    const std::shared_ptr<OHOS::MMI::KeyEvent>& keyEvent, bool isPreIme)
{
    return false;
}

bool SecuritySessionWrapperImpl::NotifyAxisEventSync(
    const std::shared_ptr<OHOS::MMI::AxisEvent>& axisEvent)
{
    return false;
}
/*************************** End: Synchronous interface for event notify *************************/

/*************************** Begin: Asynchronous interface for event notify **********************/
bool SecuritySessionWrapperImpl::NotifyFocusEventAsync(bool isFocus)
{
    return false;
}

bool SecuritySessionWrapperImpl::NotifyFocusStateAsync(bool focusState)
{
    return false;
}

bool SecuritySessionWrapperImpl::NotifyBackPressedAsync()
{
    return false;
}
bool SecuritySessionWrapperImpl::NotifyPointerEventAsync(
    const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent)
{
    return false;
}

bool SecuritySessionWrapperImpl::NotifyKeyEventAsync(
    const std::shared_ptr<OHOS::MMI::KeyEvent>& keyEvent)
{
    return false;
}

bool SecuritySessionWrapperImpl::NotifyAxisEventAsync(
    const std::shared_ptr<OHOS::MMI::AxisEvent>& axisEvent)
{
    return false;
}

void SecuritySessionWrapperImpl::NotifyCreate() {}

void SecuritySessionWrapperImpl::NotifyForeground()
{
    CHECK_NULL_VOID(session_);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto hostWindowId = pipeline->GetFocusWindowId();
    Rosen::ExtensionSessionManager::GetInstance().RequestExtensionSessionActivation(
        session_, hostWindowId, std::move(foregroundCallback_));
}

void SecuritySessionWrapperImpl::NotifyBackground()
{
    CHECK_NULL_VOID(session_);
    Rosen::ExtensionSessionManager::GetInstance().RequestExtensionSessionBackground(
        session_, std::move(backgroundCallback_));
}
void SecuritySessionWrapperImpl::NotifyDestroy()
{
    CHECK_NULL_VOID(session_);
    Rosen::ExtensionSessionManager::GetInstance().RequestExtensionSessionDestruction(
        session_, std::move(destructionCallback_));
}

void SecuritySessionWrapperImpl::NotifyConfigurationUpdate() {}

void SecuritySessionWrapperImpl::OnConnect()
{
    int32_t callSessionId = GetSessionId();
    taskExecutor_->PostTask(
        [weak = hostPattern_, wrapperWeak = WeakClaim(this), callSessionId]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            if (callSessionId != pattern->GetSessionId()) {
                LOGW("[AceSecurityUiExtension]OnDisconnect: The callSessionId(%{public}d)"
                        " is inconsistent with the curSession(%{public}d)",
                    callSessionId, pattern->GetSessionId());
                return;
            }
            pattern->OnConnect();
            auto wrapper = wrapperWeak.Upgrade();
            CHECK_NULL_VOID(wrapper && wrapper->session_);
            ContainerScope scope(wrapper->instanceId_);
            if (auto hostWindowNode = WindowSceneHelper::FindWindowScene(pattern->GetHost())) {
                auto hostNode = AceType::DynamicCast<FrameNode>(hostWindowNode);
                CHECK_NULL_VOID(hostNode);
                auto hostPattern = hostNode->GetPattern<SystemWindowScene>();
                CHECK_NULL_VOID(hostPattern);
                wrapper->session_->SetParentSession(hostPattern->GetSession());
            }
        },
        TaskExecutor::TaskType::UI, "ArkUIUIExtensionSessionConnect");
}

void SecuritySessionWrapperImpl::OnDisconnect(bool isAbnormal)
{
    int32_t callSessionId = GetSessionId();
    taskExecutor_->PostTask(
        [weak = hostPattern_, sessionType = sessionType_, isAbnormal, callSessionId]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            if (callSessionId != pattern->GetSessionId()) {
                LOGW("[AceSecurityUiExtension]OnDisconnect: The callSessionId(%{public}d)"
                        " is inconsistent with the curSession(%{public}d)",
                    callSessionId, pattern->GetSessionId());
                return;
            }
            pattern->OnDisconnect(isAbnormal);
            if (isAbnormal) {
                pattern->FireOnErrorCallback(ERROR_CODE_UIEXTENSION_EXITED_ABNORMALLY,
                    EXIT_ABNORMALLY_NAME, EXIT_ABNORMALLY_MESSAGE);
            } else {
                pattern->FireOnTerminatedCallback(0, nullptr);
            }
        },
        TaskExecutor::TaskType::UI, "ArkUIUIExtensionSessionDisconnect");
}

void SecuritySessionWrapperImpl::OnExtensionTimeout(int32_t /* errorCode */)
{
    int32_t callSessionId = GetSessionId();
    taskExecutor_->PostTask(
        [weak = hostPattern_, callSessionId]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            if (callSessionId != pattern->GetSessionId()) {
                LOGW("[AceSecurityUiExtension]OnExtensionTimeout: The callSessionId(%{public}d)"
                        " is inconsistent with the curSession(%{public}d)",
                    callSessionId, pattern->GetSessionId());
                return;
            }
            pattern->FireOnErrorCallback(ERROR_CODE_UIEXTENSION_LIFECYCLE_TIMEOUT,
                LIFECYCLE_TIMEOUT_NAME, LIFECYCLE_TIMEOUT_MESSAGE);
        },
        TaskExecutor::TaskType::UI, "ArkUIUIExtensionTimeout");
}

void SecuritySessionWrapperImpl::OnAccessibilityEvent(
    const Accessibility::AccessibilityEventInfo& info, int64_t offset)
{}

bool SecuritySessionWrapperImpl::TransferExecuteAction(int64_t elementId,
    const std::map<std::string, std::string>& actionArguments, int32_t action, int64_t offset)
{
    CHECK_NULL_RETURN(session_, false);
    return OHOS::Rosen::WSError::WS_OK == session_->TransferExecuteAction(
        elementId, actionArguments, action, offset);
}

void SecuritySessionWrapperImpl::SearchExtensionElementInfoByAccessibilityId(int64_t elementId,
    int32_t mode, int64_t baseParent, std::list<Accessibility::AccessibilityElementInfo>& output)
{
    CHECK_NULL_VOID(session_);
    session_->TransferSearchElementInfo(elementId, mode, baseParent, output);
}

void SecuritySessionWrapperImpl::SearchElementInfosByText(int64_t elementId, const std::string& text,
    int64_t baseParent, std::list<Accessibility::AccessibilityElementInfo>& output)
{
    CHECK_NULL_VOID(session_);
    session_->TransferSearchElementInfosByText(elementId, text, baseParent, output);
}

void SecuritySessionWrapperImpl::FindFocusedElementInfo(int64_t elementId,
    int32_t focusType, int64_t baseParent, Accessibility::AccessibilityElementInfo& output)
{
    CHECK_NULL_VOID(session_);
    session_->TransferFindFocusedElementInfo(elementId, focusType, baseParent, output);
}

void SecuritySessionWrapperImpl::FocusMoveSearch(int64_t elementId,
    int32_t direction, int64_t baseParent, Accessibility::AccessibilityElementInfo& output)
{
    CHECK_NULL_VOID(session_);
    session_->TransferFocusMoveSearch(elementId, direction, baseParent, output);
}

void SecuritySessionWrapperImpl::TransferAccessibilityHoverEvent(float pointX,
    float pointY, int32_t sourceType, int32_t eventType, int64_t timeMs)
{
    CHECK_NULL_VOID(session_);
    session_->TransferAccessibilityHoverEvent(pointX, pointY, sourceType, eventType, timeMs);
}
/************************ End: The interface about the accessibility **************************/

/********** Begin: The interface to control the display area and the avoid area ***************/
std::shared_ptr<Rosen::RSSurfaceNode> SecuritySessionWrapperImpl::GetSurfaceNode() const
{
    return session_ ? session_->GetSurfaceNode() : nullptr;
}

void SecuritySessionWrapperImpl::NotifyDisplayArea(const RectF& displayArea)
{
    CHECK_NULL_VOID(session_);
    ContainerScope scope(instanceId_);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto curWindow = pipeline->GetCurrentWindowRect();
    displayArea_ = displayArea + OffsetF(curWindow.Left(), curWindow.Top());
    PLATFORM_LOGD("The display area with '%{public}s' is notified to the provider.",
        displayArea_.ToString().c_str());
    std::shared_ptr<Rosen::RSTransaction> transaction;
    auto parentSession = session_->GetParentSession();
    auto reason = parentSession ? parentSession->GetSizeChangeReason() : session_->GetSizeChangeReason();
    auto persistentId = parentSession ? parentSession->GetPersistentId() : session_->GetPersistentId();
    ACE_SCOPED_TRACE("NotifyDisplayArea id: %d, reason [%d]", persistentId, reason);
    PLATFORM_LOGD("NotifyDisplayArea id: %{public}d, reason = %{public}d", persistentId, reason);
    if (reason == Rosen::SizeChangeReason::ROTATION) {
        if (transaction_.lock()) {
            transaction = transaction_.lock();
            transaction_.reset();
        } else if (auto transactionController = Rosen::RSSyncTransactionController::GetInstance()) {
            transaction = transactionController->GetRSTransaction();
        }
        if (transaction) {
            transaction->SetParentPid(transaction->GetChildPid());
            transaction->SetChildPid(AceApplicationInfo::GetInstance().GetPid());
            if (parentSession) {
                transaction->SetDuration(pipeline->GetSyncAnimationOption().GetDuration());
            }
        }
    }
    session_->UpdateRect({ std::round(displayArea_.Left()), std::round(displayArea_.Top()),
        std::round(displayArea_.Width()), std::round(displayArea_.Height()) }, reason, transaction);
}

void SecuritySessionWrapperImpl::NotifySizeChangeReason(
    WindowSizeChangeReason type, const std::shared_ptr<Rosen::RSTransaction>& rsTransaction)
{
    CHECK_NULL_VOID(session_);
    auto reason = static_cast<Rosen::SizeChangeReason>(type);
    session_->UpdateSizeChangeReason(reason);
    if (rsTransaction && (type == WindowSizeChangeReason::ROTATION)) {
        transaction_ = rsTransaction;
    }
}

void SecuritySessionWrapperImpl::NotifyOriginAvoidArea(
    const Rosen::AvoidArea& avoidArea, uint32_t type) const
{
    CHECK_NULL_VOID(session_);
    PLATFORM_LOGD("The avoid area is notified to the provider.");
    session_->UpdateAvoidArea(
        sptr<Rosen::AvoidArea>::MakeSptr(avoidArea), static_cast<Rosen::AvoidAreaType>(type));
}

bool SecuritySessionWrapperImpl::NotifyOccupiedAreaChangeInfo(
    sptr<Rosen::OccupiedAreaChangeInfo> info) const
{
    CHECK_NULL_RETURN(session_, false);
    CHECK_NULL_RETURN(info, false);
    CHECK_NULL_RETURN(isNotifyOccupiedAreaChange_, false);
    int32_t keyboardHeight = static_cast<int32_t>(info->rect_.height_);
    if (keyboardHeight > 0) {
        ContainerScope scope(instanceId_);
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_RETURN(pipeline, false);
        auto curWindow = pipeline->GetCurrentWindowRect();
        int32_t spaceWindow = std::max(curWindow.Bottom() - displayArea_.Bottom(), .0);
        keyboardHeight = static_cast<int32_t>(std::max(keyboardHeight - spaceWindow, 0));
    }
    info->rect_.height_ = static_cast<uint32_t>(keyboardHeight);
    PLATFORM_LOGD("The occcupied area with 'keyboardHeight = %{public}d' is notified to the provider.",
        keyboardHeight);
    session_->NotifyOccupiedAreaChangeInfo(info);
    return true;
}

void SecuritySessionWrapperImpl::SetDensityDpiImpl(bool isDensityDpi)
{
    CHECK_NULL_VOID(session_);
    if (isDensityDpi) {
        float density = PipelineBase::GetCurrentDensity();
        session_->NotifyDensityFollowHost(isDensityDpi, density);
    }
}

void SecuritySessionWrapperImpl::SendDataAsync(const AAFwk::WantParams& params) const
{
    PLATFORM_LOGD("The data is asynchronously send and the session is %{public}s",
        session_ ? "valid" : "invalid");
    CHECK_NULL_VOID(session_);
    session_->TransferComponentData(params);
}

int32_t SecuritySessionWrapperImpl::SendDataSync(
    const AAFwk::WantParams& wantParams, AAFwk::WantParams& reWantParams) const
{
    PLATFORM_LOGD("The data is synchronously send and the session is %{public}s",
        session_ ? "valid" : "invalid");
    Rosen::WSErrorCode transferCode = Rosen::WSErrorCode::WS_ERROR_TRANSFER_DATA_FAILED;
    if (session_) {
        transferCode = session_->TransferComponentDataSync(wantParams, reWantParams);
    }
    return static_cast<int32_t>(transferCode);
}
} // namespace OHOS::Ace::NG
