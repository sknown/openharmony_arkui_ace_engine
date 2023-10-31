/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/pipeline_ng/pipeline_context.h"

#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <memory>
#include <string>

#include "base/log/log_wrapper.h"

#ifdef ENABLE_ROSEN_BACKEND
#include "render_service_client/core/transaction/rs_transaction.h"
#include "render_service_client/core/ui/rs_ui_director.h"
#endif

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/log/ace_trace.h"
#include "base/log/ace_tracker.h"
#include "base/log/dump_log.h"
#include "base/log/event_report.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/ressched/ressched_report.h"
#include "base/thread/task_executor.h"
#include "base/utils/time_util.h"
#include "base/utils/utils.h"
#include "core/animation/scheduler.h"
#include "core/common/ace_application_info.h"
#include "core/common/container.h"
#include "core/common/font_manager.h"
#include "core/common/layout_inspector.h"
#include "core/common/text_field_manager.h"
#include "core/common/thread_checker.h"
#include "core/common/window.h"
#include "core/components/common/layout/screen_system_manager.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/app_bar/app_bar_view.h"
#include "core/components_ng/pattern/container_modal/container_modal_pattern.h"
#include "core/components_ng/pattern/container_modal/container_modal_view.h"
#include "core/components_ng/pattern/container_modal/container_modal_view_factory.h"
#include "core/components_ng/pattern/custom/custom_node_base.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/navigation/navigation_group_node.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "core/components_ng/pattern/navigation/title_bar_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/stage/stage_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_manager.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_pattern.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/safe_area_insets.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/ace_events.h"
#include "core/event/touch_event.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline/pipeline_context.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace {
constexpr int32_t TIME_THRESHOLD = 2 * 1000000; // 3 millisecond
constexpr int32_t PLATFORM_VERSION_TEN = 10;
constexpr int32_t USED_ID_FIND_FLAG = 3; // if args >3 , it means use id to find
} // namespace

namespace OHOS::Ace::NG {

PipelineContext::PipelineContext(std::shared_ptr<Window> window, RefPtr<TaskExecutor> taskExecutor,
    RefPtr<AssetManager> assetManager, RefPtr<PlatformResRegister> platformResRegister,
    const RefPtr<Frontend>& frontend, int32_t instanceId)
    : PipelineBase(window, std::move(taskExecutor), std::move(assetManager), frontend, instanceId, platformResRegister)
{
    window_->OnHide();
}

PipelineContext::PipelineContext(std::shared_ptr<Window> window, RefPtr<TaskExecutor> taskExecutor,
    RefPtr<AssetManager> assetManager, const RefPtr<Frontend>& frontend, int32_t instanceId)
    : PipelineBase(window, std::move(taskExecutor), std::move(assetManager), frontend, instanceId)
{
    window_->OnHide();
}

RefPtr<PipelineContext> PipelineContext::GetCurrentContext()
{
    auto currentContainer = Container::Current();
    CHECK_NULL_RETURN(currentContainer, nullptr);
    return DynamicCast<PipelineContext>(currentContainer->GetPipelineContext());
}

float PipelineContext::GetCurrentRootWidth()
{
    auto context = GetCurrentContext();
    CHECK_NULL_RETURN(context, 0.0f);
    return static_cast<float>(context->rootWidth_);
}

float PipelineContext::GetCurrentRootHeight()
{
    auto context = GetCurrentContext();
    CHECK_NULL_RETURN(context, 0.0f);
    return static_cast<float>(context->rootHeight_);
}

void PipelineContext::AddDirtyCustomNode(const RefPtr<UINode>& dirtyNode)
{
    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(dirtyNode);
    dirtyNodes_.emplace(dirtyNode);
    hasIdleTasks_ = true;
    RequestFrame();
}

void PipelineContext::AddDirtyLayoutNode(const RefPtr<FrameNode>& dirty)
{
    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(dirty);
    taskScheduler_->AddDirtyLayoutNode(dirty);
    ForceLayoutForImplicitAnimation();
#ifdef UICAST_COMPONENT_SUPPORTED
    do {
        auto container = Container::Current();
        CHECK_NULL_BREAK(container);
        auto distributedUI = container->GetDistributedUI();
        CHECK_NULL_BREAK(distributedUI);
        distributedUI->AddDirtyLayoutNode(dirty->GetId());
    } while (false);
#endif
    hasIdleTasks_ = true;
    RequestFrame();
}

void PipelineContext::AddDirtyRenderNode(const RefPtr<FrameNode>& dirty)
{
    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(dirty);
    taskScheduler_->AddDirtyRenderNode(dirty);
    ForceRenderForImplicitAnimation();
#ifdef UICAST_COMPONENT_SUPPORTED
    do {
        auto container = Container::Current();
        CHECK_NULL_BREAK(container);
        auto distributedUI = container->GetDistributedUI();
        CHECK_NULL_BREAK(distributedUI);
        distributedUI->AddDirtyRenderNode(dirty->GetId());
    } while (false);
#endif
    hasIdleTasks_ = true;
    RequestFrame();
}

void PipelineContext::FlushDirtyNodeUpdate()
{
    CHECK_RUN_ON(UI);
    ACE_FUNCTION_TRACE();
    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().BeginFlushBuild();
    }

    // SomeTimes, customNode->Update may add some dirty custom nodes to dirtyNodes_,
    // use maxFlushTimes to avoid dead cycle.
    int maxFlushTimes = 3;
    while (!dirtyNodes_.empty() && maxFlushTimes > 0) {
        decltype(dirtyNodes_) dirtyNodes(std::move(dirtyNodes_));
        for (const auto& node : dirtyNodes) {
            if (AceType::InstanceOf<NG::CustomNodeBase>(node)) {
                auto customNode = AceType::DynamicCast<NG::CustomNodeBase>(node);
                ACE_SCOPED_TRACE("CustomNodeUpdate %s", customNode->GetJSViewName().c_str());
                customNode->Update();
            }
        }
        --maxFlushTimes;
    }
    if (!dirtyNodes_.empty()) {
        LOGD("FlushDirtyNodeUpdate 3 times, still has dirty nodes");
    }

    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().EndFlushBuild();
    }
}

uint32_t PipelineContext::AddScheduleTask(const RefPtr<ScheduleTask>& task)
{
    CHECK_RUN_ON(UI);
    scheduleTasks_.try_emplace(++nextScheduleTaskId_, task);
    RequestFrame();
    return nextScheduleTaskId_;
}

void PipelineContext::RemoveScheduleTask(uint32_t id)
{
    CHECK_RUN_ON(UI);
    scheduleTasks_.erase(id);
}

void PipelineContext::FlushVsync(uint64_t nanoTimestamp, uint32_t frameCount)
{
    CHECK_RUN_ON(UI);
    ACE_FUNCTION_TRACE();
    auto recvTime = GetSysTimestamp();
    static const std::string abilityName = AceApplicationInfo::GetInstance().GetProcessName().empty()
                                               ? AceApplicationInfo::GetInstance().GetPackageName()
                                               : AceApplicationInfo::GetInstance().GetProcessName();
    window_->RecordFrameTime(nanoTimestamp, abilityName);
    FlushFrameTrace();
#ifdef UICAST_COMPONENT_SUPPORTED
    do {
        auto container = Container::Current();
        CHECK_NULL_BREAK(container);
        auto distributedUI = container->GetDistributedUI();
        CHECK_NULL_BREAK(distributedUI);
        distributedUI->ApplyOneUpdate();
    } while (false);
#endif
    FlushAnimation(GetTimeFromExternalTimer());
    FlushTouchEvents();
    FlushBuild();
    if (isFormRender_ && drawDelegate_ && rootNode_) {
        auto renderContext = AceType::DynamicCast<NG::RenderContext>(rootNode_->GetRenderContext());
        drawDelegate_->DrawRSFrame(renderContext);
        drawDelegate_ = nullptr;
    }
    if (!taskScheduler_->isEmpty()) {
#if !defined(PREVIEW)
        LayoutInspector::SupportInspector();
#endif
    }

    taskScheduler_->StartRecordFrameInfo(GetCurrentFrameInfo(recvTime, nanoTimestamp));
    taskScheduler_->FlushTask();
    taskScheduler_->FinishRecordFrameInfo();
    FlushAnimationClosure();
    TryCallNextFrameLayoutCallback();

#ifdef UICAST_COMPONENT_SUPPORTED
    do {
        auto container = Container::Current();
        CHECK_NULL_BREAK(container);
        auto distributedUI = container->GetDistributedUI();
        CHECK_NULL_BREAK(distributedUI);
        distributedUI->OnTreeUpdate();
    } while (false);
#endif

    bool hasAnimation = window_->FlushCustomAnimation(nanoTimestamp);
    if (hasAnimation) {
        RequestFrame();
    }
    FlushMessages();
    if (dragCleanTask_) {
        dragCleanTask_();
        dragCleanTask_ = nullptr;
    }
    InspectDrew();
    if (!isFormRender_ && onShow_ && onFocus_) {
        FlushFocus();
    }
    HandleOnAreaChangeEvent();
    HandleVisibleAreaChangeEvent();
    if (isNeedFlushMouseEvent_) {
        FlushMouseEvent();
        isNeedFlushMouseEvent_ = false;
    }
    needRenderNode_.clear();
    taskScheduler_->FlushAfterRenderTask();
    // Keep the call sent at the end of the function
    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().FlushEnd();
    }
    ResSchedReport::GetInstance().LoadPageEvent(ResDefine::LOAD_PAGE_COMPLETE_EVENT);
}

void PipelineContext::InspectDrew()
{
    CHECK_RUN_ON(UI);
    if (!needRenderNode_.empty()) {
        auto needRenderNode = std::move(needRenderNode_);
        for (auto&& node : needRenderNode) {
            if (node) {
                OnDrawCompleted(node->GetInspectorId()->c_str());
            }
        }
    }
}

void PipelineContext::FlushFrameTrace()
{
    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().FlushBegin();
    }
}

void PipelineContext::FlushAnimation(uint64_t nanoTimestamp)
{
    CHECK_RUN_ON(UI);
    ACE_FUNCTION_TRACE();
    if (scheduleTasks_.empty()) {
        return;
    }

    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().BeginFlushAnimation();
    }

    decltype(scheduleTasks_) temp(std::move(scheduleTasks_));
    for (const auto& [id, weakTask] : temp) {
        auto task = weakTask.Upgrade();
        if (task) {
            task->OnFrame(nanoTimestamp);
        }
    }

    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().EndFlushAnimation();
    }
}

void PipelineContext::FlushMessages()
{
    ACE_FUNCTION_TRACE();
    window_->FlushTasks();
}

void PipelineContext::SetNeedRenderNode(const RefPtr<FrameNode>& node)
{
    CHECK_RUN_ON(UI);
    needRenderNode_.insert(node);
}

void PipelineContext::FlushFocus()
{
    CHECK_RUN_ON(UI);
    ACE_FUNCTION_TRACK();
    ACE_FUNCTION_TRACE();

    auto defaultFocusNode = dirtyDefaultFocusNode_.Upgrade();
    if (!defaultFocusNode) {
        dirtyDefaultFocusNode_.Reset();
    } else {
        auto focusNodeHub = defaultFocusNode->GetFocusHub();
        if (focusNodeHub) {
            RequestDefaultFocus(focusNodeHub);
        }
        dirtyFocusNode_.Reset();
        dirtyFocusScope_.Reset();
        dirtyDefaultFocusNode_.Reset();
        return;
    }

    auto focusNode = dirtyFocusNode_.Upgrade();
    if (!focusNode || focusNode->GetFocusType() != FocusType::NODE) {
        dirtyFocusNode_.Reset();
    } else {
        auto focusNodeHub = focusNode->GetFocusHub();
        if (focusNodeHub) {
            focusNodeHub->RequestFocusImmediately();
        }
        dirtyFocusNode_.Reset();
        dirtyFocusScope_.Reset();
        dirtyDefaultFocusNode_.Reset();
        return;
    }
    auto focusScope = dirtyFocusScope_.Upgrade();
    if (!focusScope || focusScope->GetFocusType() != FocusType::SCOPE) {
        dirtyFocusScope_.Reset();
    } else {
        auto focusScopeHub = focusScope->GetFocusHub();
        if (focusScopeHub) {
            focusScopeHub->RequestFocusImmediately();
        }
        dirtyFocusNode_.Reset();
        dirtyFocusScope_.Reset();
        dirtyDefaultFocusNode_.Reset();
        return;
    }
}

void PipelineContext::FlushPipelineImmediately()
{
    CHECK_RUN_ON(UI);
    ACE_FUNCTION_TRACE();
    FlushPipelineWithoutAnimation();
}

void PipelineContext::FlushPipelineWithoutAnimation()
{
    ACE_FUNCTION_TRACE();
    FlushBuild();
    FlushTouchEvents();
    taskScheduler_->FlushTask();
    FlushAnimationClosure();
    FlushMessages();
    FlushFocus();
}

void PipelineContext::FlushBuild()
{
    isRebuildFinished_ = false;
    FlushDirtyNodeUpdate();
    isRebuildFinished_ = true;
    FlushBuildFinishCallbacks();
}

void PipelineContext::AddAnimationClosure(std::function<void()>&& animation)
{
    animationClosuresList_.emplace_back(std::move(animation));
}

void PipelineContext::FlushAnimationClosure()
{
    if (animationClosuresList_.empty()) {
        return;
    }
    taskScheduler_->FlushTask();

    decltype(animationClosuresList_) temp(std::move(animationClosuresList_));
    auto scheduler = std::move(taskScheduler_);
    taskScheduler_ = std::make_unique<UITaskScheduler>();
    for (const auto& animation : temp) {
        animation();
        taskScheduler_->CleanUp();
    }
    taskScheduler_ = std::move(scheduler);
}

void PipelineContext::FlushBuildFinishCallbacks()
{
    decltype(buildFinishCallbacks_) buildFinishCallbacks(std::move(buildFinishCallbacks_));
    for (const auto& func : buildFinishCallbacks) {
        if (func) {
            func();
        }
    }
}

void PipelineContext::RegisterRootEvent()
{
    if (!IsFormRender()) {
        return;
    }

    // To avoid conflicts between longPress and click events on the card,
    // use an empty longPress event placeholder in the EtsCard scenario
    auto hub = rootNode_->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(hub);
    auto event = [](const GestureEvent& info) mutable { LOGD("Not Support LongPress"); };
    auto longPress = AceType::MakeRefPtr<NG::LongPressEvent>(std::move(event));
    hub->SetLongPressEvent(longPress, false, true);
}

void PipelineContext::SetupRootElement()
{
    CHECK_RUN_ON(UI);
    rootNode_ = FrameNode::CreateFrameNodeWithTree(
        V2::ROOT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), MakeRefPtr<RootPattern>());
    rootNode_->SetHostRootId(GetInstanceId());
    rootNode_->SetHostPageId(-1);
    rootNode_->SetActive(true);
    RegisterRootEvent();
    CalcSize idealSize { CalcLength(rootWidth_), CalcLength(rootHeight_) };
    MeasureProperty layoutConstraint;
    layoutConstraint.selfIdealSize = idealSize;
    layoutConstraint.maxSize = idealSize;
    rootNode_->UpdateLayoutConstraint(layoutConstraint);
    auto rootFocusHub = rootNode_->GetOrCreateFocusHub();
    rootFocusHub->SetFocusType(FocusType::SCOPE);
    rootFocusHub->SetFocusable(true);
    window_->SetRootFrameNode(rootNode_);
    rootNode_->AttachToMainTree();

    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), MakeRefPtr<StagePattern>());
    auto atomicService = installationFree_ ? AppBarView::Create(stageNode) : nullptr;
    if (windowModal_ == WindowModal::CONTAINER_MODAL) {
        MaximizeMode maximizeMode = GetWindowManager()->GetWindowMaximizeMode();
        rootNode_->AddChild(
            ContainerModalViewFactory::GetView(atomicService ? atomicService : stageNode, maximizeMode));
    } else {
        rootNode_->AddChild(atomicService ? atomicService : stageNode);
    }
#ifdef ENABLE_ROSEN_BACKEND
    if (!IsJsCard() && !isFormRender_) {
        auto window = GetWindow();
        if (window) {
            auto rsUIDirector = window->GetRSUIDirector();
            if (rsUIDirector) {
                rsUIDirector->SetAbilityBGAlpha(appBgColor_.GetAlpha());
            }
        }
    }
#endif
    stageManager_ = MakeRefPtr<StageManager>(stageNode);
    overlayManager_ = MakeRefPtr<OverlayManager>(
        DynamicCast<FrameNode>(installationFree_ ? stageNode->GetParent()->GetParent() : stageNode->GetParent()));
    fullScreenManager_ = MakeRefPtr<FullScreenManager>(rootNode_);
    selectOverlayManager_ = MakeRefPtr<SelectOverlayManager>(rootNode_);
    dragDropManager_ = MakeRefPtr<DragDropManager>();
    sharedTransitionManager_ = MakeRefPtr<SharedOverlayManager>(
        DynamicCast<FrameNode>(installationFree_ ? stageNode->GetParent()->GetParent() : stageNode->GetParent()));

    OnAreaChangedFunc onAreaChangedFunc = [weakOverlayManger = AceType::WeakClaim(AceType::RawPtr(overlayManager_))](
                                              const RectF& /* oldRect */, const OffsetF& /* oldOrigin */,
                                              const RectF& /* rect */, const OffsetF& /* origin */) {
        auto overlay = weakOverlayManger.Upgrade();
        CHECK_NULL_VOID(overlay);
        overlay->HideAllMenus();
        overlay->HideCustomPopups();
    };
    rootNode_->SetOnAreaChangeCallback(std::move(onAreaChangedFunc));
    AddOnAreaChangeNode(rootNode_->GetId());
}

void PipelineContext::SetupSubRootElement()
{
    CHECK_RUN_ON(UI);
    appBgColor_ = Color::TRANSPARENT;
    rootNode_ = FrameNode::CreateFrameNodeWithTree(
        V2::ROOT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), MakeRefPtr<RootPattern>());
    rootNode_->SetHostRootId(GetInstanceId());
    rootNode_->SetHostPageId(-1);
    rootNode_->SetActive(true);
    CalcSize idealSize { CalcLength(rootWidth_), CalcLength(rootHeight_) };
    MeasureProperty layoutConstraint;
    layoutConstraint.selfIdealSize = idealSize;
    layoutConstraint.maxSize = idealSize;
    rootNode_->UpdateLayoutConstraint(layoutConstraint);
    auto rootFocusHub = rootNode_->GetOrCreateFocusHub();
    rootFocusHub->SetFocusType(FocusType::SCOPE);
    rootFocusHub->SetFocusable(true);
    window_->SetRootFrameNode(rootNode_);
    rootNode_->AttachToMainTree();

#ifdef ENABLE_ROSEN_BACKEND
    if (!IsJsCard()) {
        auto window = GetWindow();
        if (window) {
            auto rsUIDirector = window->GetRSUIDirector();
            if (rsUIDirector) {
                rsUIDirector->SetAbilityBGAlpha(appBgColor_.GetAlpha());
            }
        }
    }
#endif
    // the subwindow for overlay not need stage
    stageManager_ = MakeRefPtr<StageManager>(nullptr);
    overlayManager_ = MakeRefPtr<OverlayManager>(rootNode_);
    fullScreenManager_ = MakeRefPtr<FullScreenManager>(rootNode_);
    selectOverlayManager_ = MakeRefPtr<SelectOverlayManager>(rootNode_);
    dragDropManager_ = MakeRefPtr<DragDropManager>();
}

const RefPtr<StageManager>& PipelineContext::GetStageManager()
{
    return stageManager_;
}

const RefPtr<DragDropManager>& PipelineContext::GetDragDropManager()
{
    return dragDropManager_;
}

const RefPtr<SelectOverlayManager>& PipelineContext::GetSelectOverlayManager()
{
    return selectOverlayManager_;
}

const RefPtr<OverlayManager>& PipelineContext::GetOverlayManager()
{
    return overlayManager_;
}

const RefPtr<FullScreenManager>& PipelineContext::GetFullScreenManager()
{
    return fullScreenManager_;
}

void PipelineContext::OnSurfaceChanged(int32_t width, int32_t height, WindowSizeChangeReason type,
    const std::shared_ptr<Rosen::RSTransaction>& rsTransaction)
{
    CHECK_RUN_ON(UI);
    if (NearEqual(rootWidth_, width) && NearEqual(rootHeight_, height) &&
        type == WindowSizeChangeReason::CUSTOM_ANIMATION && !isDensityChanged_) {
        TryCallNextFrameLayoutCallback();
        return;
    }
    ExecuteSurfaceChangedCallbacks(width, height, type);
    // TODO: add adjust for textFieldManager when ime is show.
    auto callback = [weakFrontend = weakFrontend_, width, height]() {
        auto frontend = weakFrontend.Upgrade();
        if (frontend) {
            frontend->OnSurfaceChanged(width, height);
        }
    };
    auto container = Container::Current();
    if (!container) {
        return;
    }
    if (container->IsUseStageModel()) {
        callback();
        FlushBuild();
    } else {
        taskExecutor_->PostTask(callback, TaskExecutor::TaskType::JS);
    }

    FlushWindowSizeChangeCallback(width, height, type);

#ifdef ENABLE_ROSEN_BACKEND
    StartWindowSizeChangeAnimate(width, height, type, rsTransaction);
#else
    SetRootRect(width, height, 0.0);
#endif
}

void PipelineContext::OnLayoutCompleted(const std::string& componentId)
{
    CHECK_RUN_ON(UI);
    auto frontend = weakFrontend_.Upgrade();
    if (frontend) {
        frontend->OnLayoutCompleted(componentId);
    }
}

void PipelineContext::OnDrawCompleted(const std::string& componentId)
{
    CHECK_RUN_ON(UI);
    auto frontend = weakFrontend_.Upgrade();
    if (frontend) {
        frontend->OnDrawCompleted(componentId);
    }
}

void PipelineContext::ExecuteSurfaceChangedCallbacks(int32_t newWidth, int32_t newHeight, WindowSizeChangeReason type)
{
    for (auto&& [id, callback] : surfaceChangedCallbackMap_) {
        if (callback) {
            callback(newWidth, newHeight, rootWidth_, rootHeight_, type);
        }
    }
}

void PipelineContext::OnSurfacePositionChanged(int32_t posX, int32_t posY)
{
    for (auto&& [id, callback] : surfacePositionChangedCallbackMap_) {
        if (callback) {
            callback(posX, posY);
        }
    }
}

void PipelineContext::StartWindowSizeChangeAnimate(int32_t width, int32_t height, WindowSizeChangeReason type,
    const std::shared_ptr<Rosen::RSTransaction>& rsTransaction)
{
    static const bool IsWindowSizeAnimationEnabled = SystemProperties::IsWindowSizeAnimationEnabled();
    if (!IsWindowSizeAnimationEnabled) {
        SetRootRect(width, height, 0.0);
        return;
    }
    switch (type) {
        case WindowSizeChangeReason::RECOVER:
        case WindowSizeChangeReason::MAXIMIZE: {
            StartWindowMaximizeAnimation(width, height, rsTransaction);
            break;
        }
        case WindowSizeChangeReason::ROTATION: {
            safeAreaManager_->UpdateKeyboardOffset(0.0);
            SetRootRect(width, height, 0.0);
            FlushUITasks();
            if (textFieldManager_) {
                DynamicCast<TextFieldManagerNG>(textFieldManager_)->ScrollTextFieldToSafeArea();
            }
            FlushUITasks();
            break;
        }
        case WindowSizeChangeReason::DRAG_START:
        case WindowSizeChangeReason::DRAG:
        case WindowSizeChangeReason::DRAG_END:
        case WindowSizeChangeReason::RESIZE:
        case WindowSizeChangeReason::UNDEFINED:
        default: {
            SetRootRect(width, height, 0.0f);
        }
    }
}

void PipelineContext::StartWindowMaximizeAnimation(
    int32_t width, int32_t height, const std::shared_ptr<Rosen::RSTransaction>& rsTransaction)
{
    LOGI("Root node start RECOVER/MAXIMIZE animation, width = %{public}d, height = %{public}d", width, height);
#ifdef ENABLE_ROSEN_BACKEND
    if (rsTransaction) {
        FlushMessages();
        rsTransaction->Begin();
    }
#endif
    AnimationOption option;
    int32_t duration = 400;
    MaximizeMode maximizeMode = GetWindowManager()->GetWindowMaximizeMode();
    if (maximizeMode == MaximizeMode::MODE_FULL_FILL
        || maximizeMode == MaximizeMode::MODE_AVOID_SYSTEM_BAR) {
        duration = 0;
    }
    option.SetDuration(duration);
    auto curve = Curves::EASE_OUT;
    option.SetCurve(curve);
    auto weak = WeakClaim(this);
    Animate(option, curve, [width, height, weak]() {
        auto pipeline = weak.Upgrade();
        CHECK_NULL_VOID(pipeline);
        pipeline->SetRootRect(width, height, 0.0);
        pipeline->FlushUITasks();
    });
#ifdef ENABLE_ROSEN_BACKEND
    if (rsTransaction) {
        rsTransaction->Commit();
    }
#endif
}

void PipelineContext::SetRootRect(double width, double height, double offset)
{
    CHECK_RUN_ON(UI);
    UpdateRootSizeAndScale(width, height);
    CHECK_NULL_VOID(rootNode_);
    ScreenSystemManager::GetInstance().SetWindowInfo(rootWidth_, density_, dipScale_);
    ScreenSystemManager::GetInstance().OnSurfaceChanged(width);
    SizeF sizeF { static_cast<float>(width), static_cast<float>(height) };
    if (rootNode_->GetGeometryNode()->GetFrameSize() != sizeF || rootNode_->IsLayoutDirtyMarked()) {
        CalcSize idealSize { CalcLength(width), CalcLength(height) };
        MeasureProperty layoutConstraint;
        layoutConstraint.selfIdealSize = idealSize;
        layoutConstraint.maxSize = idealSize;
        rootNode_->UpdateLayoutConstraint(layoutConstraint);
        // reset parentLayoutConstraint to update itself when next measure task
        rootNode_->GetGeometryNode()->ResetParentLayoutConstraint();
        rootNode_->MarkDirtyNode();
    }
    if (rootNode_->GetGeometryNode()->GetFrameOffset().GetY() != offset) {
        OffsetF newOffset = rootNode_->GetGeometryNode()->GetFrameOffset();
        newOffset.SetY(static_cast<float>(offset));
        rootNode_->GetGeometryNode()->SetMarginFrameOffset(newOffset);
        auto rootContext = rootNode_->GetRenderContext();
        rootContext->SyncGeometryProperties(RawPtr(rootNode_->GetGeometryNode()));
        RequestFrame();
    }
    if (isDensityChanged_) {
        rootNode_->GetGeometryNode()->ResetParentLayoutConstraint();
        rootNode_->MarkForceMeasure();
        isDensityChanged_ = false;
    }
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
    // For cross-platform build, flush tasks when first resize, speed up for fisrt frame.
    if (window_ && rootNode_->GetRenderContext() && !NearZero(width) && !NearZero(height)) {
        rootNode_->GetRenderContext()->SetBounds(0.0, 0.0, width, height);
        window_->FlushTasks();
        FlushVsync(GetTimeFromExternalTimer(), 0);
    }
#endif
}

void PipelineContext::UpdateSystemSafeArea(const SafeAreaInsets& systemSafeArea)
{
    CHECK_NULL_VOID(minPlatformVersion_ >= PLATFORM_VERSION_TEN);
    if (safeAreaManager_->UpdateSystemSafeArea(systemSafeArea)) {
        AnimateOnSafeAreaUpdate();
    }
}

void PipelineContext::UpdateCutoutSafeArea(const SafeAreaInsets& cutoutSafeArea)
{
    CHECK_NULL_VOID(minPlatformVersion_ >= PLATFORM_VERSION_TEN);
    if (safeAreaManager_->UpdateCutoutSafeArea(cutoutSafeArea)) {
        AnimateOnSafeAreaUpdate();
    }
}

void PipelineContext::SetEnableKeyBoardAvoidMode(bool value)
{
    safeAreaManager_->SetKeyBoardAvoidMode(value);
}

bool PipelineContext::IsEnableKeyBoardAvoidMode()
{
    return safeAreaManager_->KeyboardSafeAreaEnabled();
}

void PipelineContext::SetIgnoreViewSafeArea(bool value)
{
    if (safeAreaManager_->SetIgnoreSafeArea(value)) {
        SyncSafeArea();
    }
}

void PipelineContext::SetIsLayoutFullScreen(bool value)
{
    if (safeAreaManager_->SetIsFullScreen(value)) {
        SyncSafeArea();
    }
}

PipelineBase::SafeAreaInsets PipelineContext::GetSafeArea() const
{
    return safeAreaManager_->GetSafeArea();
}

void PipelineContext::SyncSafeArea(bool onKeyboard)
{
    CHECK_NULL_VOID(stageManager_);
    auto page = stageManager_->GetLastPage();
    if (page) {
        page->MarkDirtyNode(onKeyboard && !safeAreaManager_->KeyboardSafeAreaEnabled() ? PROPERTY_UPDATE_LAYOUT
                                                                                       : PROPERTY_UPDATE_MEASURE);
    }
    if (overlayManager_) {
        overlayManager_->MarkDirty(PROPERTY_UPDATE_MEASURE);
    }
    if (selectOverlayManager_) {
        selectOverlayManager_->MarkDirty(PROPERTY_UPDATE_MEASURE);
    }
    auto&& restoreNodes = safeAreaManager_->GetGeoRestoreNodes();
    for (auto&& wk : restoreNodes) {
        auto node = wk.Upgrade();
        if (node) {
            node->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
        }
    }
}

void PipelineContext::OnVirtualKeyboardHeightChange(
    float keyboardHeight, const std::shared_ptr<Rosen::RSTransaction>& rsTransaction)
{
    CHECK_RUN_ON(UI);
    // prevent repeated trigger with same keyboardHeight
    if (keyboardHeight == safeAreaManager_->GetKeyboardInset().Length()) {
        return;
    }

    ACE_FUNCTION_TRACE();
#ifdef ENABLE_ROSEN_BACKEND
    if (rsTransaction) {
        FlushMessages();
        rsTransaction->Begin();
    }
#endif

    auto func = [this, keyboardHeight]() mutable {
        safeAreaManager_->UpdateKeyboardSafeArea(keyboardHeight);
        if (keyboardHeight > 0) {
            // add height of navigation bar
            keyboardHeight += safeAreaManager_->GetSystemSafeArea().bottom_.Length();
        }
        float positionY = 0.0f;
        auto manager = DynamicCast<TextFieldManagerNG>(PipelineBase::GetTextFieldManager());
        float height = 0.0f;
        if (manager) {
            height = manager->GetHeight();
            positionY = static_cast<float>(manager->GetClickPosition().GetY());
        }
        SizeF rootSize { static_cast<float>(rootWidth_), static_cast<float>(rootHeight_) };
        float keyboardOffset = safeAreaManager_->GetKeyboardOffset();
        float positionYWithOffset = positionY - keyboardOffset;
        float offsetFix = (rootSize.Height() - positionYWithOffset) > 100.0f
                              ? keyboardHeight - (rootSize.Height() - positionYWithOffset) / 2.0f
                              : keyboardHeight;
        if (NearZero(keyboardHeight)) {
            safeAreaManager_->UpdateKeyboardOffset(0.0f);
        } else if (LessOrEqual(rootSize.Height() - positionYWithOffset - height, height) &&
                   LessOrEqual(rootSize.Height() - positionYWithOffset, keyboardHeight)) {
            safeAreaManager_->UpdateKeyboardOffset(-keyboardHeight);
        } else if (positionYWithOffset + height > (rootSize.Height() - keyboardHeight) && offsetFix > 0.0f) {
            safeAreaManager_->UpdateKeyboardOffset(-offsetFix);
        } else if ((positionYWithOffset + height > rootSize.Height() - keyboardHeight &&
                       positionYWithOffset < rootSize.Height() - keyboardHeight && height < keyboardHeight / 2.0f) &&
                   NearZero(rootNode_->GetGeometryNode()->GetFrameOffset().GetY())) {
            safeAreaManager_->UpdateKeyboardOffset(-height - offsetFix / 2.0f);
        } else {
            safeAreaManager_->UpdateKeyboardOffset(0.0f);
        }
        SyncSafeArea(true);
        // layout immediately
        FlushUITasks();

        CHECK_NULL_VOID(manager);
        manager->ScrollTextFieldToSafeArea();
        FlushUITasks();
    };

    AnimationOption option = AnimationUtil::CreateKeyboardAnimationOption(keyboardAnimationConfig_, keyboardHeight);
    Animate(option, option.GetCurve(), func);

#ifdef ENABLE_ROSEN_BACKEND
    if (rsTransaction) {
        rsTransaction->Commit();
    }
#endif
}

bool PipelineContext::OnBackPressed()
{
    LOGD("OnBackPressed");
    CHECK_RUN_ON(PLATFORM);
    auto frontend = weakFrontend_.Upgrade();
    if (!frontend) {
        // return back.
        return false;
    }

    // If the tag of the last child of the rootnode is video, exit full screen.
    if (fullScreenManager_->OnBackPressed()) {
        return true;
    }

    // if has sharedTransition, back press will stop the sharedTransition
    if (sharedTransitionManager_->OnBackPressed()) {
        return true;
    }

    auto textfieldManager = DynamicCast<TextFieldManagerNG>(PipelineBase::GetTextFieldManager());
    if (textfieldManager && textfieldManager->OnBackPressed()) {
        return true;
    }

#ifdef WINDOW_SCENE_SUPPORTED
    if (uiExtensionManager_->OnBackPressed()) {
        return true;
    }
#endif

    // if has popup, back press would hide popup and not trigger page back
    auto hasOverlay = false;
    taskExecutor_->PostSyncTask(
        [weakOverlay = AceType::WeakClaim(AceType::RawPtr(overlayManager_)),
            weakSelectOverlay = AceType::WeakClaim(AceType::RawPtr(selectOverlayManager_)), &hasOverlay]() {
            // Destroy behaviour of Select Overlay shouble be adjusted.
            auto overlay = weakOverlay.Upgrade();
            CHECK_NULL_VOID(overlay);
            auto selectOverlay = weakSelectOverlay.Upgrade();
            CHECK_NULL_VOID(selectOverlay);
            selectOverlay->DestroySelectOverlay();
            hasOverlay = overlay->RemoveOverlay(true);
        },
        TaskExecutor::TaskType::UI);
    if (hasOverlay) {
        return true;
    }

    auto result = false;
    taskExecutor_->PostSyncTask(
        [weakFrontend = weakFrontend_, weakPipelineContext = WeakClaim(this), stageManager = stageManager_, &result]() {
            auto frontend = weakFrontend.Upgrade();
            if (!frontend) {
                result = false;
                return;
            }
            auto context = weakPipelineContext.Upgrade();
            if (!context) {
                result = false;
                return;
            }
            CHECK_NULL_VOID(stageManager);
            auto lastPage = stageManager->GetLastPage();
            CHECK_NULL_VOID(lastPage);
            auto navigationGroupNode =
                AceType::DynamicCast<NavigationGroupNode>(context->FindNavigationNodeToHandleBack(lastPage));
            if (navigationGroupNode) {
                result = true;
            }
        },
        TaskExecutor::TaskType::UI);

    if (result) {
        // user accept
        return true;
    }

    taskExecutor_->PostSyncTask(
        [weakFrontend = weakFrontend_, weakPipelineContext = WeakClaim(this), &result]() {
            auto frontend = weakFrontend.Upgrade();
            if (!frontend) {
                result = false;
                return;
            }
            result = frontend->OnBackPressed();
        },
        TaskExecutor::TaskType::JS);

    if (result) {
        // user accept
        return true;
    }
    return false;
}

RefPtr<FrameNode> PipelineContext::FindNavigationNodeToHandleBack(const RefPtr<UINode>& node)
{
    const auto& children = node->GetChildren();
    for (auto iter = children.rbegin(); iter != children.rend(); ++iter) {
        auto& child = *iter;

        auto target = FindNavigationNodeToHandleBack(child);
        if (target) {
            return target;
        }
    }
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(node);
    if (navigationGroupNode && navigationGroupNode->CheckCanHandleBack()) {
        return navigationGroupNode;
    }
    return nullptr;
}

bool PipelineContext::SetIsFocusActive(bool isFocusActive)
{
    if (isFocusActive_ == isFocusActive) {
        return false;
    }
    isFocusActive_ = isFocusActive;
    CHECK_NULL_RETURN(rootNode_, false);
    auto rootFocusHub = rootNode_->GetFocusHub();
    CHECK_NULL_RETURN(rootFocusHub, false);
    if (isFocusActive_) {
        return rootFocusHub->PaintAllFocusState();
    }
    rootFocusHub->ClearAllFocusState();
    return true;
}

void PipelineContext::OnTouchEvent(const TouchEvent& point, bool isSubPipe)
{
    CHECK_RUN_ON(UI);

#ifdef UICAST_COMPONENT_SUPPORTED
    do {
        auto container = Container::Current();
        CHECK_NULL_BREAK(container);
        auto distributedUI = container->GetDistributedUI();
        CHECK_NULL_BREAK(distributedUI);
        if (distributedUI->IsSinkMode()) {
            distributedUI->BypassEvent(point, isSubPipe);
            return;
        }
    } while (false);
#endif

    HandleEtsCardTouchEvent(point);

    auto scalePoint = point.CreateScalePoint(GetViewScale());
    LOGD("AceTouchEvent: x = %{public}f, y = %{public}f, type = %{public}zu", scalePoint.x, scalePoint.y,
        scalePoint.type);
    eventManager_->SetInstanceId(GetInstanceId());
    if (scalePoint.type == TouchType::DOWN) {
        // Set focus state inactive while touch down event received
        SetIsFocusActive(false);
        TouchRestrict touchRestrict { TouchRestrict::NONE };
        touchRestrict.sourceType = point.sourceType;
        touchRestrict.touchEvent = point;
        eventManager_->TouchTest(scalePoint, rootNode_, touchRestrict, GetPluginEventOffset(), viewScale_, isSubPipe);
        for (const auto& weakContext : touchPluginPipelineContext_) {
            auto pipelineContext = DynamicCast<OHOS::Ace::PipelineBase>(weakContext.Upgrade());
            if (!pipelineContext) {
                continue;
            }
            auto pluginPoint =
                point.UpdateScalePoint(viewScale_, static_cast<float>(pipelineContext->GetPluginEventOffset().GetX()),
                    static_cast<float>(pipelineContext->GetPluginEventOffset().GetY()), point.id);
            // eventManager_ instance Id may changed.
            pipelineContext->OnTouchEvent(pluginPoint, true);
        }

        // restore instance Id.
        eventManager_->SetInstanceId(GetInstanceId());
    }

    auto rootOffset = GetRootRect().GetOffset();
    eventManager_->HandleGlobalEventNG(scalePoint, selectOverlayManager_, rootOffset);

    if (isSubPipe) {
        return;
    }

    if (scalePoint.type == TouchType::MOVE) {
        touchEvents_.emplace_back(point);
        auto container = Container::Current();
        if (container && container->IsScenceBoardWindow() && IsWindowSceneConsumed()) {
            FlushTouchEvents();
            return;
        } else {
            hasIdleTasks_ = true;
            RequestFrame();
            return;
        }
    }

    if (scalePoint.type == TouchType::UP) {
        lastTouchTime_ = GetTimeFromExternalTimer();
    }

    std::optional<TouchEvent> lastMoveEvent;
    if (scalePoint.type == TouchType::UP && !touchEvents_.empty()) {
        for (auto iter = touchEvents_.begin(); iter != touchEvents_.end(); ++iter) {
            auto movePoint = (*iter).CreateScalePoint(GetViewScale());
            if (scalePoint.id == movePoint.id) {
                lastMoveEvent = movePoint;
                touchEvents_.erase(iter++);
            }
        }
        if (lastMoveEvent.has_value()) {
            eventManager_->SetLastMoveBeforeUp(scalePoint.sourceType == SourceType::MOUSE);
            eventManager_->DispatchTouchEvent(lastMoveEvent.value());
            eventManager_->SetLastMoveBeforeUp(false);
        }
    }

    eventManager_->DispatchTouchEvent(scalePoint);

    if ((scalePoint.type == TouchType::UP) || (scalePoint.type == TouchType::CANCEL)) {
        // need to reset touchPluginPipelineContext_ for next touch down event.
        touchPluginPipelineContext_.clear();
        RemoveEtsCardTouchEventCallback(point.id);
    }

    hasIdleTasks_ = true;
    RequestFrame();
}

void PipelineContext::OnSurfaceDensityChanged(double density)
{
    CHECK_RUN_ON(UI);
    if (!NearEqual(density, density_)) {
        isDensityChanged_ = true;
    }
    density_ = density;
    if (!NearZero(viewScale_)) {
        dipScale_ = density_ / viewScale_;
    }
}

void PipelineContext::RegisterDumpInfoListener(const std::function<void(const std::vector<std::string>&)>& callback)
{
    dumpListeners_.push_back(callback);
}

bool PipelineContext::OnDumpInfo(const std::vector<std::string>& params) const
{
    ACE_DCHECK(!params.empty());
    if (params[0] == "-element") {
        if (params.size() > 1 && params[1] == "-lastpage") {
            auto lastPage = stageManager_->GetLastPage();
            if (params.size() < USED_ID_FIND_FLAG && lastPage) {
                lastPage->DumpTree(0);
            }
            if (params.size() == USED_ID_FIND_FLAG && lastPage && !lastPage->DumpTreeById(0, params[2])) {
                DumpLog::GetInstance().Print(
                    "There is no id matching the ID in the parameter,please check whether the id is correct");
            }
        } else {
            rootNode_->DumpTree(0);
        }
    } else if (params[0] == "-render") {
    } else if (params[0] == "-focus") {
        if (rootNode_->GetFocusHub()) {
            rootNode_->GetFocusHub()->DumpFocusTree(0);
        }
    } else if (params[0] == "-layer") {
    } else if (params[0] == "-frontend") {
#ifndef WEARABLE_PRODUCT
    } else if (params[0] == "-multimodal") {
#endif
    } else if (params[0] == "-accessibility" || params[0] == "-inspector") {
        auto accessibilityManager = GetAccessibilityManager();
        if (accessibilityManager) {
            accessibilityManager->OnDumpInfo(params);
        }
    } else if (params[0] == "-rotation" && params.size() >= 2) {
    } else if (params[0] == "-animationscale" && params.size() >= 2) {
    } else if (params[0] == "-velocityscale" && params.size() >= 2) {
    } else if (params[0] == "-scrollfriction" && params.size() >= 2) {
    } else if (params[0] == "-threadstuck" && params.size() >= 3) {
    } else if (params[0] == "-pipeline") {
        DumpPipelineInfo();
    } else if (params[0] == "-jsdump") {
        std::vector<std::string> jsParams;
        if (params.begin() != params.end()) {
            jsParams = std::vector<std::string>(params.begin() + 1, params.end());
        }

        for (const auto& func : dumpListeners_) {
            func(jsParams);
        } 
    }

    return true;
}

FrameInfo* PipelineContext::GetCurrentFrameInfo(uint64_t recvTime, uint64_t timeStamp)
{
    if (SystemProperties::GetDumpFrameCount() == 0) {
        return nullptr;
    }
    if (dumpFrameInfos_.size() >= SystemProperties::GetDumpFrameCount()) {
        dumpFrameInfos_.pop_front();
    }

    dumpFrameInfos_.push_back({ .frameRecvTime_ = recvTime, .frameTimeStamp_ = timeStamp });
    return &dumpFrameInfos_.back();
}

void PipelineContext::DumpPipelineInfo() const
{
    DumpLog::GetInstance().Print("PipelineInfo:");
    if (window_) {
        DumpLog::GetInstance().Print(1, "DisplayRefreshRate: " + std::to_string(window_->GetRefreshRate()));
        DumpLog::GetInstance().Print(1, "LastRequestVsyncTime: " + std::to_string(window_->GetLastRequestVsyncTime()));
    }
    if (!dumpFrameInfos_.empty()) {
        DumpLog::GetInstance().Print("==================================FrameTask==================================");
        for (const auto& info : dumpFrameInfos_) {
            DumpLog::GetInstance().Print("Task: " + info.GetTimeInfo());
            DumpLog::GetInstance().Print(1, "LayoutTask:");
            for (const auto& layout : info.layoutInfos_) {
                DumpLog::GetInstance().Print(2, layout.ToString());
            }
            DumpLog::GetInstance().Print(1, "RenderTask:");
            for (const auto& layout : info.renderInfos_) {
                DumpLog::GetInstance().Print(2, layout.ToString());
            }
            DumpLog::GetInstance().Print(
                "==================================FrameTask==================================");
        }
    }
}

void PipelineContext::FlushTouchEvents()
{
    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(rootNode_);
    {
        std::unordered_set<int32_t> moveEventIds;
        decltype(touchEvents_) touchEvents(std::move(touchEvents_));
        if (touchEvents.empty()) {
            canUseLongPredictTask_ = true;
            return;
        }
        canUseLongPredictTask_ = false;
        eventManager_->FlushTouchEventsBegin(touchEvents_);
        std::unordered_map<int, TouchEvent> idToTouchPoints;
        for (auto iter = touchEvents.rbegin(); iter != touchEvents.rend(); ++iter) {
            auto scalePoint = (*iter).CreateScalePoint(GetViewScale());
            idToTouchPoints.emplace(scalePoint.id, scalePoint);
            idToTouchPoints[scalePoint.id].history.insert(idToTouchPoints[scalePoint.id].history.begin(), scalePoint);
        }
        std::list<TouchEvent> touchPoints;
        for (auto& [_, item] : idToTouchPoints) {
            touchPoints.emplace_back(std::move(item));
        }
        auto maxSize = touchPoints.size();
        for (auto iter = touchPoints.rbegin(); iter != touchPoints.rend(); ++iter) {
            maxSize--;
            if (maxSize == 0) {
                eventManager_->FlushTouchEventsEnd(touchPoints);
            }
            eventManager_->DispatchTouchEvent(*iter);
        }
    }
}

void PipelineContext::OnMouseEvent(const MouseEvent& event)
{
    CHECK_RUN_ON(UI);
    if (!lastMouseEvent_) {
        lastMouseEvent_ = std::make_unique<MouseEvent>();
    }
    lastMouseEvent_->x = event.x;
    lastMouseEvent_->y = event.y;
    lastMouseEvent_->button = event.button;
    lastMouseEvent_->action = event.action;
    lastMouseEvent_->sourceType = event.sourceType;
    lastMouseEvent_->time = event.time;

    if (event.button == MouseButton::RIGHT_BUTTON && event.action == MouseAction::PRESS) {
        // Mouse right button press event set focus inactive here.
        // Mouse left button press event will set focus inactive in touch process.
        SetIsFocusActive(false);
    }
    auto container = Container::Current();
    if (((event.action == MouseAction::RELEASE || event.action == MouseAction::PRESS ||
             event.action == MouseAction::MOVE) &&
            (event.button == MouseButton::LEFT_BUTTON || event.pressedButtons == MOUSE_PRESS_LEFT)) ||
        (container && container->IsScenceBoardWindow() &&
            (event.pullAction == MouseAction::PULL_MOVE || event.pullAction == MouseAction::PULL_UP))) {
        auto touchPoint = event.CreateTouchPoint();
        OnTouchEvent(touchPoint);
    }

    CHECK_NULL_VOID(rootNode_);
    auto scaleEvent = event.CreateScaleEvent(viewScale_);
    LOGD(
        "MouseEvent (x,y): (%{public}f,%{public}f), button: %{public}d, action: %{public}d, pressedButtons: %{public}d",
        scaleEvent.x, scaleEvent.y, scaleEvent.button, scaleEvent.action, scaleEvent.pressedButtons);
    TouchRestrict touchRestrict { TouchRestrict::NONE };
    touchRestrict.sourceType = event.sourceType;
    touchRestrict.hitTestType = SourceType::MOUSE;
    eventManager_->MouseTest(scaleEvent, rootNode_, touchRestrict);
    eventManager_->DispatchMouseEventNG(scaleEvent);
    eventManager_->DispatchMouseHoverEventNG(scaleEvent);
    eventManager_->DispatchMouseHoverAnimationNG(scaleEvent);
    RequestFrame();
}

void PipelineContext::FlushMouseEvent()
{
    if (!lastMouseEvent_ || lastMouseEvent_->action == MouseAction::WINDOW_LEAVE) {
        return;
    }
    MouseEvent event;
    event.x = lastMouseEvent_->x;
    event.y = lastMouseEvent_->y;
    event.time = lastMouseEvent_->time;
    event.action = MouseAction::MOVE;
    event.button = MouseButton::NONE_BUTTON;
    event.sourceType = SourceType::MOUSE;

    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(rootNode_);
    auto scaleEvent = event.CreateScaleEvent(viewScale_);
    TouchRestrict touchRestrict { TouchRestrict::NONE };
    touchRestrict.sourceType = event.sourceType;
    touchRestrict.hitTestType = SourceType::MOUSE;
    eventManager_->MouseTest(scaleEvent, rootNode_, touchRestrict);
    eventManager_->DispatchMouseEventNG(scaleEvent);
    eventManager_->DispatchMouseHoverEventNG(scaleEvent);
    eventManager_->DispatchMouseHoverAnimationNG(scaleEvent);
}

bool PipelineContext::ChangeMouseStyle(int32_t nodeId, MouseFormat format)
{
    if (!onFocus_) {
        return false;
    }
    if (mouseStyleNodeId_ != nodeId) {
        return false;
    }
    auto mouseStyle = MouseStyle::CreateMouseStyle();
    CHECK_NULL_RETURN(mouseStyle, false);
    return mouseStyle->ChangePointerStyle(GetWindowId(), format);
}

bool PipelineContext::OnKeyEvent(const KeyEvent& event)
{
    eventManager_->SetPressedKeyCodes(event.pressedCodes);
    CHECK_NULL_RETURN(eventManager_, false);
    if (event.action == KeyAction::DOWN) {
        eventManager_->DispatchKeyboardShortcut(event);
    }
    if (event.code == KeyCode::KEY_ESCAPE) {
        auto manager = GetDragDropManager();
        if (manager) {
            manager->SetIsDragCancel(true);
            manager->OnDragEnd({ 0.0f, 0.0f }, "");
        }
    }
    auto isKeyTabDown = event.action == KeyAction::DOWN && event.IsKey({ KeyCode::KEY_TAB });
    auto curMainView = FocusHub::GetCurrentMainView();
    auto isViewRootScopeFocused = curMainView ? curMainView->GetIsViewRootScopeFocused() : true;
    isTabJustTriggerOnKeyEvent_ = false;
    if (isKeyTabDown && isViewRootScopeFocused) {
        if (curMainView) {
            auto viewRootScope = curMainView->GetMainViewRootScope();
            if (viewRootScope && viewRootScope->GetFocusDependence() == FocusDependence::SELF &&
                viewRootScope->IsCurrentFocus()) {
                curMainView->SetIsDefaultHasFocused(true);
                curMainView->SetIsViewRootScopeFocused(viewRootScope, false);
                viewRootScope->InheritFocus();
                isTabJustTriggerOnKeyEvent_ = true;
            }
        }
    }
    // TAB key set focus state from inactive to active.
    // If return success. This tab key will just trigger onKeyEvent process.
    bool isHandleFocusActive = isKeyTabDown && SetIsFocusActive(true);
    isTabJustTriggerOnKeyEvent_ = isTabJustTriggerOnKeyEvent_ || isHandleFocusActive;
    auto lastPage = stageManager_ ? stageManager_->GetLastPage() : nullptr;
    auto mainNode = lastPage ? lastPage : rootNode_;
    CHECK_NULL_RETURN(mainNode, false);
    if (!eventManager_->DispatchTabIndexEventNG(event, rootNode_, mainNode)) {
        auto result = eventManager_->DispatchKeyEventNG(event, rootNode_);
        if (!result && event.code == KeyCode::KEY_ESCAPE && event.action == KeyAction::DOWN) {
            CHECK_NULL_RETURN(overlayManager_, false);
            auto currentContainer = Container::Current();
            if (currentContainer->IsSubContainer() || currentContainer->IsDialogContainer()) {
                return overlayManager_->RemoveOverlayInSubwindow();
            } else {
                return overlayManager_->RemoveOverlay(false);
            }
        } else {
            return result;
        }
    }
    return true;
}

bool PipelineContext::RequestDefaultFocus(const RefPtr<FocusHub>& mainView)
{
    if (mainView->GetFocusType() != FocusType::SCOPE) {
        return false;
    }
    auto viewRootScope = mainView->GetMainViewRootScope();
    auto defaultFocusNode = mainView->GetChildFocusNodeByType(FocusNodeType::DEFAULT);
    if (!mainView->IsDefaultHasFocused() && defaultFocusNode && defaultFocusNode->IsFocusableWholePath()) {
        mainView->SetIsViewRootScopeFocused(viewRootScope, false);
        auto ret = defaultFocusNode->RequestFocusImmediately();
        mainView->SetIsDefaultHasFocused(true);
        LOGI("Target view's default focus is %{public}s/%{public}d. Request default focus return: %{public}d.",
            defaultFocusNode->GetFrameName().c_str(), defaultFocusNode->GetFrameId(), ret);
        return ret;
    }
    if (mainView->GetIsViewRootScopeFocused() && viewRootScope) {
        mainView->SetIsViewRootScopeFocused(viewRootScope, true);
        auto ret = viewRootScope->RequestFocusImmediately();
        LOGI("Target view's default focus is %{public}s/%{public}d. Request view root focus return: %{public}d.",
            viewRootScope->GetFrameName().c_str(), viewRootScope->GetFrameId(), ret);
        return ret;
    }
    mainView->SetIsViewRootScopeFocused(viewRootScope, false);
    auto ret = mainView->RequestFocusImmediately();
    LOGI("Target view's default focus has been focused. Request view focus return: %{public}d.", ret);
    return ret;
}

bool PipelineContext::RequestFocus(const std::string& targetNodeId)
{
    CHECK_NULL_RETURN(rootNode_, false);
    auto focusHub = rootNode_->GetFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    auto currentFocusChecked = focusHub->RequestFocusImmediatelyById(targetNodeId);
    if (!isSubPipeline_ || currentFocusChecked) {
        return currentFocusChecked;
    }
    auto parentPipelineBase = parentPipeline_.Upgrade();
    CHECK_NULL_RETURN(parentPipelineBase, false);
    auto parentPipelineContext = AceType::DynamicCast<NG::PipelineContext>(parentPipelineBase);
    CHECK_NULL_RETURN(parentPipelineContext, false);
    return parentPipelineContext->RequestFocus(targetNodeId);
}

void PipelineContext::AddDirtyFocus(const RefPtr<FrameNode>& node)
{
    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(node);
    if (node->GetFocusType() == FocusType::NODE) {
        dirtyFocusNode_ = WeakClaim(RawPtr(node));
    } else {
        dirtyFocusScope_ = WeakClaim(RawPtr(node));
    }
    RequestFrame();
}

void PipelineContext::AddDirtyDefaultFocus(const RefPtr<FrameNode>& node)
{
    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(node);
    dirtyDefaultFocusNode_ = WeakPtr<FrameNode>(node);
    RequestFrame();
}

void PipelineContext::RootLostFocus(BlurReason reason) const
{
    CHECK_NULL_VOID(rootNode_);
    auto focusHub = rootNode_->GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->LostFocus(reason);
    CHECK_NULL_VOID(overlayManager_);
    overlayManager_->HideAllMenus();
    overlayManager_->HideCustomPopups();
}

MouseEvent ConvertAxisToMouse(const AxisEvent& event)
{
    MouseEvent result;
    result.x = event.x;
    result.y = event.y;
    result.action = MouseAction::MOVE;
    result.button = MouseButton::NONE_BUTTON;
    result.time = event.time;
    result.deviceId = event.deviceId;
    result.sourceType = event.sourceType;
    result.pointerEvent = event.pointerEvent;
    return result;
}

void PipelineContext::OnAxisEvent(const AxisEvent& event)
{
    auto scaleEvent = event.CreateScaleEvent(viewScale_);
    LOGD("AxisEvent (x,y): (%{public}f,%{public}f), action: %{public}d, horizontalAxis: %{public}f, verticalAxis: "
         "%{public}f, pinchAxisScale: %{public}f",
        scaleEvent.x, scaleEvent.y, scaleEvent.action, scaleEvent.horizontalAxis, scaleEvent.verticalAxis,
        scaleEvent.pinchAxisScale);

    auto dragManager = GetDragDropManager();
    if (dragManager && !dragManager->IsDragged()) {
        if (event.action == AxisAction::BEGIN) {
            TouchRestrict touchRestrict { TouchRestrict::NONE };
            touchRestrict.sourceType = event.sourceType;
            touchRestrict.hitTestType = SourceType::TOUCH;
            eventManager_->TouchTest(scaleEvent, rootNode_, touchRestrict);
        }
        eventManager_->DispatchTouchEvent(scaleEvent);
    }

    if (event.action == AxisAction::BEGIN || event.action == AxisAction::UPDATE) {
        eventManager_->AxisTest(scaleEvent, rootNode_);
        eventManager_->DispatchAxisEventNG(scaleEvent);
    }

    auto mouseEvent = ConvertAxisToMouse(event);
    OnMouseEvent(mouseEvent);
}

void PipelineContext::AddVisibleAreaChangeNode(
    const RefPtr<FrameNode>& node, double ratio, const VisibleRatioCallback& callback, bool isUserCallback)
{
    CHECK_NULL_VOID(node);
    VisibleCallbackInfo addInfo;
    addInfo.callback = callback;
    addInfo.isCurrentVisible = false;
    onVisibleAreaChangeNodeIds_.emplace(node->GetId());
    if (isUserCallback) {
        node->AddVisibleAreaUserCallback(ratio, addInfo);
    } else {
        node->AddVisibleAreaInnerCallback(ratio, addInfo);
    }
}

void PipelineContext::RemoveVisibleAreaChangeNode(int32_t nodeId)
{
    onVisibleAreaChangeNodeIds_.erase(nodeId);
}

void PipelineContext::HandleVisibleAreaChangeEvent()
{
    ACE_FUNCTION_TRACE();
    if (onVisibleAreaChangeNodeIds_.empty()) {
        return;
    }
    auto nodes = FrameNode::GetNodesById(onVisibleAreaChangeNodeIds_);
    for (auto&& frameNode : nodes) {
        frameNode->TriggerVisibleAreaChangeCallback();
    }
}

void PipelineContext::AddOnAreaChangeNode(int32_t nodeId)
{
    onAreaChangeNodeIds_.emplace(nodeId);
}

void PipelineContext::RemoveOnAreaChangeNode(int32_t nodeId)
{
    onAreaChangeNodeIds_.erase(nodeId);
}

void PipelineContext::HandleOnAreaChangeEvent()
{
    ACE_FUNCTION_TRACE();
    if (onAreaChangeNodeIds_.empty()) {
        return;
    }
    auto nodes = FrameNode::GetNodesById(onAreaChangeNodeIds_);
    for (auto&& frameNode : nodes) {
        frameNode->TriggerOnAreaChangeCallback();
    }
    UpdateFormLinkInfos();
}

void PipelineContext::UpdateFormLinkInfos()
{
    if (formLinkInfoUpdateHandler_ && !formLinkInfoMap_.empty()) {
        std::vector<std::string> formLinkInfos;
        for (auto iter = formLinkInfoMap_.rbegin(); iter != formLinkInfoMap_.rend(); ++iter) {
            auto info = iter->second;
            formLinkInfos.push_back(info);
        }
        formLinkInfoUpdateHandler_(formLinkInfos);
    }
}

void PipelineContext::OnShow()
{
    CHECK_RUN_ON(UI);
    onShow_ = true;
    window_->OnShow();
    RequestFrame();
    FlushWindowStateChangedCallback(true);
    AccessibilityEvent event;
    event.windowChangeTypes = WindowUpdateType::WINDOW_UPDATE_ACTIVE;
    event.type = AccessibilityEventType::PAGE_CHANGE;
    SendEventToAccessibility(event);
}

void PipelineContext::OnHide()
{
    CHECK_RUN_ON(UI);
    onShow_ = false;
    window_->OnHide();
    RequestFrame();
    OnVirtualKeyboardAreaChange(Rect());
    FlushWindowStateChangedCallback(false);
    AccessibilityEvent event;
    event.type = AccessibilityEventType::PAGE_CHANGE;
    SendEventToAccessibility(event);
}

void PipelineContext::WindowFocus(bool isFocus)
{
    CHECK_RUN_ON(UI);
    onFocus_ = isFocus;
    if (!isFocus) {
        LOGD("WindowFocus: window - %{public}d on blur.", windowId_);
        auto mouseStyle = MouseStyle::CreateMouseStyle();
        if (mouseStyle) {
            mouseStyle->ChangePointerStyle(static_cast<int32_t>(GetWindowId()), MouseFormat::DEFAULT);
        }
        RootLostFocus(BlurReason::WINDOW_BLUR);
        NotifyPopupDismiss();
        OnVirtualKeyboardAreaChange(Rect());
    } else {
        LOGD("WindowFocus: window - %{public}d on focus.", windowId_);
        auto rootFocusHub = rootNode_ ? rootNode_->GetFocusHub() : nullptr;
        if (rootFocusHub && !rootFocusHub->IsCurrentFocus()) {
            rootFocusHub->RequestFocusImmediately();
        }
    }
    FlushWindowFocusChangedCallback(isFocus);
}

void PipelineContext::ShowContainerTitle(bool isShow, bool hasDeco, bool needUpdate)
{
    if (windowModal_ != WindowModal::CONTAINER_MODAL) {
        return;
    }
    CHECK_NULL_VOID(rootNode_);
    auto containerNode = AceType::DynamicCast<FrameNode>(rootNode_->GetChildren().front());
    CHECK_NULL_VOID(containerNode);
    auto containerPattern = containerNode->GetPattern<ContainerModalPattern>();
    CHECK_NULL_VOID(containerPattern);
    auto callback = [weakPattern = WeakClaim(RawPtr(containerPattern)), isShow, hasDeco, needUpdate]() {
        auto pattern = weakPattern.Upgrade();
        if (pattern != nullptr) {
            pattern->ShowTitle(isShow, hasDeco, needUpdate);
        }
    };
    MaximizeMode maximizeMode = GetWindowManager()->GetWindowMaximizeMode();
    if (maximizeMode == MaximizeMode::MODE_FULL_FILL
        || maximizeMode == MaximizeMode::MODE_AVOID_SYSTEM_BAR) {
        constexpr int32_t delayedTime = 50;
        taskExecutor_->PostDelayedTask(callback, TaskExecutor::TaskType::UI, delayedTime);
    } else {
        callback();
    }
}

void PipelineContext::SetContainerWindow(bool isShow)
{
#ifdef ENABLE_ROSEN_BACKEND
    if (!IsJsCard()) {
        auto window = GetWindow();
        if (window) {
            auto rsUIDirector = window->GetRSUIDirector();
            if (rsUIDirector) {
                // set container window show state to render service
                rsUIDirector->SetContainerWindow(isShow, density_);
            }
        }
    }
#endif
}

void PipelineContext::SetAppBgColor(const Color& color)
{
    appBgColor_ = color;
#ifdef ENABLE_ROSEN_BACKEND
    if (!IsJsCard()) {
        auto window = GetWindow();
        if (window) {
            auto rsUIDirector = window->GetRSUIDirector();
            if (rsUIDirector) {
                rsUIDirector->SetAbilityBGAlpha(appBgColor_.GetAlpha());
            }
        }
    }
#endif
    CHECK_NULL_VOID(rootNode_);
    auto rootPattern = rootNode_->GetPattern<RootPattern>();
    CHECK_NULL_VOID(rootPattern);
    rootPattern->SetAppBgColor(appBgColor_, windowModal_ == WindowModal::CONTAINER_MODAL);
}

void PipelineContext::SetAppTitle(const std::string& title)
{
    if (windowModal_ != WindowModal::CONTAINER_MODAL) {
        return;
    }
    CHECK_NULL_VOID(rootNode_);
    auto containerNode = AceType::DynamicCast<FrameNode>(rootNode_->GetChildren().front());
    CHECK_NULL_VOID(containerNode);
    auto containerPattern = containerNode->GetPattern<ContainerModalPattern>();
    CHECK_NULL_VOID(containerPattern);
    containerPattern->SetAppTitle(title);
}

void PipelineContext::SetAppIcon(const RefPtr<PixelMap>& icon)
{
    if (windowModal_ != WindowModal::CONTAINER_MODAL) {
        return;
    }
    CHECK_NULL_VOID(rootNode_);
    auto containerNode = AceType::DynamicCast<FrameNode>(rootNode_->GetChildren().front());
    CHECK_NULL_VOID(containerNode);
    auto containerPattern = containerNode->GetPattern<ContainerModalPattern>();
    CHECK_NULL_VOID(containerPattern);
    containerPattern->SetAppIcon(icon);
}

void PipelineContext::FlushReload(const OnConfigurationChange& configurationChange)
{
    AnimationOption option;
    const int32_t duration = 400;
    option.SetDuration(duration);
    option.SetCurve(Curves::FRICTION);
    AnimationUtils::Animate(option, [weak = WeakClaim(this), configurationChange]() {
        auto pipeline = weak.Upgrade();
        CHECK_NULL_VOID(pipeline);
        if (configurationChange.IsNeedUpdate()) {
            auto rootNode = pipeline->GetRootElement();
            rootNode->UpdateConfigurationUpdate(configurationChange);
        }
        CHECK_NULL_VOID(pipeline->stageManager_);
        pipeline->SetIsReloading(true);
        pipeline->stageManager_->ReloadStage();
        pipeline->SetIsReloading(false);
        pipeline->FlushUITasks();
    });
}

void PipelineContext::Destroy()
{
    CHECK_RUN_ON(UI);
    taskScheduler_->CleanUp();
    scheduleTasks_.clear();
    dirtyNodes_.clear();
    rootNode_.Reset();
    stageManager_.Reset();
    overlayManager_.Reset();
    sharedTransitionManager_.Reset();
    dragDropManager_.Reset();
    selectOverlayManager_.Reset();
    fullScreenManager_.Reset();
    touchEvents_.clear();
    buildFinishCallbacks_.clear();
    onWindowStateChangedCallbacks_.clear();
    onWindowFocusChangedCallbacks_.clear();
    nodesToNotifyMemoryLevel_.clear();
    dirtyFocusNode_.Reset();
    dirtyFocusScope_.Reset();
    needRenderNode_.clear();
    dirtyDefaultFocusNode_.Reset();
    PipelineBase::Destroy();
}

void PipelineContext::AddBuildFinishCallBack(std::function<void()>&& callback)
{
    buildFinishCallbacks_.emplace_back(std::move(callback));
}

void PipelineContext::AddWindowStateChangedCallback(int32_t nodeId)
{
    onWindowStateChangedCallbacks_.emplace(nodeId);
}

void PipelineContext::RemoveWindowStateChangedCallback(int32_t nodeId)
{
    onWindowStateChangedCallbacks_.erase(nodeId);
}

void PipelineContext::FlushWindowStateChangedCallback(bool isShow)
{
    auto iter = onWindowStateChangedCallbacks_.begin();
    while (iter != onWindowStateChangedCallbacks_.end()) {
        auto node = ElementRegister::GetInstance()->GetUINodeById(*iter);
        if (!node) {
            iter = onWindowStateChangedCallbacks_.erase(iter);
        } else {
            if (isShow) {
                node->OnWindowShow();
            } else {
                node->OnWindowHide();
            }
            ++iter;
        }
    }
    HandleVisibleAreaChangeEvent();
    HandleSubwindow(isShow);
}

void PipelineContext::AddWindowFocusChangedCallback(int32_t nodeId)
{
    onWindowFocusChangedCallbacks_.emplace_back(nodeId);
}

void PipelineContext::RemoveWindowFocusChangedCallback(int32_t nodeId)
{
    onWindowFocusChangedCallbacks_.remove(nodeId);
}

void PipelineContext::FlushWindowFocusChangedCallback(bool isFocus)
{
    auto iter = onWindowFocusChangedCallbacks_.begin();
    while (iter != onWindowFocusChangedCallbacks_.end()) {
        auto node = ElementRegister::GetInstance()->GetUINodeById(*iter);
        if (!node) {
            iter = onWindowFocusChangedCallbacks_.erase(iter);
        } else {
            if (isFocus) {
                node->OnWindowFocused();
            } else {
                node->OnWindowUnfocused();
            }
            ++iter;
        }
    }
}

void PipelineContext::AddWindowSizeChangeCallback(int32_t nodeId)
{
    onWindowSizeChangeCallbacks_.emplace_back(nodeId);
}

void PipelineContext::RemoveWindowSizeChangeCallback(int32_t nodeId)
{
    onWindowSizeChangeCallbacks_.remove(nodeId);
}

void PipelineContext::FlushWindowSizeChangeCallback(int32_t width, int32_t height, WindowSizeChangeReason type)
{
    auto iter = onWindowSizeChangeCallbacks_.begin();
    while (iter != onWindowSizeChangeCallbacks_.end()) {
        auto node = ElementRegister::GetInstance()->GetUINodeById(*iter);
        if (!node) {
            iter = onWindowSizeChangeCallbacks_.erase(iter);
        } else {
            node->OnWindowSizeChanged(width, height, type);
            ++iter;
        }
    }
}

void PipelineContext::OnDragEvent(int32_t x, int32_t y, DragEventAction action)
{
    auto manager = GetDragDropManager();
    CHECK_NULL_VOID(manager);
#ifdef ENABLE_DRAG_FRAMEWORK
    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        if (!manager->IsDragged() && manager->IsWindowConsumed()) {
            manager->SetIsWindowConsumed(false);
            return;
        }
    }
    if (action == DragEventAction::DRAG_EVENT_OUT) {
        manager->ClearSummary();
        manager->ClearExtraInfo();
    }
#endif // ENABLE_DRAG_FRAMEWORK

    std::string extraInfo;

#ifdef ENABLE_DRAG_FRAMEWORK
    if (action == DragEventAction::DRAG_EVENT_START) {
        manager->RequireSummary();
        manager->GetExtraInfoFromClipboard(extraInfo);
        manager->SetExtraInfo(extraInfo);
    }
#else
    manager->GetExtraInfoFromClipboard(extraInfo);
#endif // ENABLE_DRAG_FRAMEWORK
    if (action == DragEventAction::DRAG_EVENT_END) {
#ifdef ENABLE_DRAG_FRAMEWORK
        manager->GetExtraInfoFromClipboard(extraInfo);
        manager->SetExtraInfo(extraInfo);
#endif // ENABLE_DRAG_FRAMEWORK
        manager->OnDragEnd(Point(x, y, x, y), extraInfo);
        manager->RestoreClipboardData();
        return;
    }
    manager->OnDragMove(Point(x, y, x, y), extraInfo);
}

void PipelineContext::AddNodesToNotifyMemoryLevel(int32_t nodeId)
{
    nodesToNotifyMemoryLevel_.emplace_back(nodeId);
}

void PipelineContext::RemoveNodesToNotifyMemoryLevel(int32_t nodeId)
{
    nodesToNotifyMemoryLevel_.remove(nodeId);
}

void PipelineContext::NotifyMemoryLevel(int32_t level)
{
    auto iter = nodesToNotifyMemoryLevel_.begin();
    while (iter != nodesToNotifyMemoryLevel_.end()) {
        auto node = ElementRegister::GetInstance()->GetUINodeById(*iter);
        if (!node) {
            iter = nodesToNotifyMemoryLevel_.erase(iter);
        } else {
            node->OnNotifyMemoryLevel(level);
            ++iter;
        }
    }
}
void PipelineContext::AddPredictTask(PredictTask&& task)
{
    taskScheduler_->AddPredictTask(std::move(task));
    RequestFrame();
}

void PipelineContext::OnIdle(int64_t deadline)
{
    if (deadline == 0) {
        canUseLongPredictTask_ = false;
        return;
    }
    if (canUseLongPredictTask_) {
        // check new incoming event after vsync.
        if (!touchEvents_.empty()) {
            canUseLongPredictTask_ = false;
        }
    }
    CHECK_RUN_ON(UI);
    ACE_SCOPED_TRACE("OnIdle, targettime:%" PRId64 "", deadline);
    taskScheduler_->FlushPredictTask(deadline - TIME_THRESHOLD, canUseLongPredictTask_);
    canUseLongPredictTask_ = false;
}

void PipelineContext::Finish(bool /*autoFinish*/) const
{
    CHECK_RUN_ON(UI);
    if (finishEventHandler_) {
        finishEventHandler_();
    }
}

void PipelineContext::AddAfterLayoutTask(std::function<void()>&& task)
{
    taskScheduler_->AddAfterLayoutTask(std::move(task));
}

void PipelineContext::AddAfterRenderTask(std::function<void()>&& task)
{
    taskScheduler_->AddAfterRenderTask(std::move(task));
}

void PipelineContext::RestoreNodeInfo(std::unique_ptr<JsonValue> nodeInfo)
{
    auto child = nodeInfo->GetChild();
    while (child->IsValid()) {
        auto key = child->GetKey();
        auto value = child->GetString();
        restoreNodeInfo_.try_emplace(StringUtils::StringToInt(key), value);
        child = child->GetNext();
    }
}

std::unique_ptr<JsonValue> PipelineContext::GetStoredNodeInfo()
{
    auto jsonNodeInfo = JsonUtil::Create(true);
    auto iter = storeNode_.begin();
    while (iter != storeNode_.end()) {
        auto node = (iter->second).Upgrade();
        if (node) {
            std::string info = node->ProvideRestoreInfo();
            if (!info.empty()) {
                jsonNodeInfo->Put(std::to_string(iter->first).c_str(), info.c_str());
            }
        }
        ++iter;
    }
    return jsonNodeInfo;
}

void PipelineContext::StoreNode(int32_t restoreId, const WeakPtr<FrameNode>& node)
{
    auto ret = storeNode_.try_emplace(restoreId, node);
    if (!ret.second) {
        storeNode_[restoreId] = node;
    }
}

bool PipelineContext::GetRestoreInfo(int32_t restoreId, std::string& restoreInfo)
{
    auto iter = restoreNodeInfo_.find(restoreId);
    if (iter != restoreNodeInfo_.end()) {
        restoreInfo = iter->second;
        restoreNodeInfo_.erase(iter);
        return true;
    }
    return false;
}

void PipelineContext::SetContainerButtonHide(bool hideSplit, bool hideMaximize, bool hideMinimize)
{
    if (windowModal_ != WindowModal::CONTAINER_MODAL) {
        LOGW("Set app icon failed, Window modal is not container.");
        return;
    }
    CHECK_NULL_VOID(rootNode_);
    auto containerNode = AceType::DynamicCast<FrameNode>(rootNode_->GetChildren().front());
    CHECK_NULL_VOID(containerNode);
    auto containerPattern = containerNode->GetPattern<ContainerModalPattern>();
    CHECK_NULL_VOID(containerPattern);
    containerPattern->SetContainerButtonHide(hideSplit, hideMaximize, hideMinimize);
}

void PipelineContext::AddFontNodeNG(const WeakPtr<UINode>& node)
{
    if (fontManager_) {
        fontManager_->AddFontNodeNG(node);
    }
}

void PipelineContext::RemoveFontNodeNG(const WeakPtr<UINode>& node)
{
    if (fontManager_) {
        fontManager_->RemoveFontNodeNG(node);
    }
}

void PipelineContext::SetWindowSceneConsumed(bool isConsumed)
{
    isWindowSceneConsumed_ = isConsumed;
}

bool PipelineContext::IsWindowSceneConsumed()
{
    return isWindowSceneConsumed_;
}

void PipelineContext::SetCloseButtonStatus(bool isEnabled)
{
    if (windowModal_ != WindowModal::CONTAINER_MODAL) {
        return;
    }
    CHECK_NULL_VOID(rootNode_);
    auto containerNode = AceType::DynamicCast<FrameNode>(rootNode_->GetChildren().front());
    CHECK_NULL_VOID(containerNode);
    auto containerPattern = containerNode->GetPattern<ContainerModalPattern>();
    CHECK_NULL_VOID(containerPattern);
    containerPattern->SetCloseButtonStatus(isEnabled);
}

void PipelineContext::AnimateOnSafeAreaUpdate()
{
    // complete other layout tasks before animation
    FlushUITasks();
    AnimationOption option;
    option.SetCurve(safeAreaManager_->GetSafeAreaCurve());
    AnimationUtils::Animate(option, [weak = WeakClaim(this)]() {
        auto self = weak.Upgrade();
        CHECK_NULL_VOID(self);
        self->SyncSafeArea();
        self->FlushUITasks();
    });
}

void PipelineContext::HandleSubwindow(bool isShow)
{
    // When the main window is applied to the background,
    // there are sub windows that do not immediately hide, such as Toast floating window
    if (!isShow) {
        overlayManager_->ClearToastInSubwindow();
    }
}
} // namespace OHOS::Ace::NG
