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
#include "base/subwindow/subwindow_manager.h"

#ifdef ENABLE_ROSEN_BACKEND
#include "render_service_client/core/transaction/rs_transaction.h"
#include "render_service_client/core/ui/rs_ui_director.h"
#endif

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/log/ace_performance_monitor.h"
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
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/pattern/app_bar/app_bar_view.h"
#include "core/components_ng/pattern/container_modal/container_modal_pattern.h"
#include "core/components_ng/pattern/container_modal/container_modal_view.h"
#include "core/components_ng/pattern/container_modal/container_modal_view_factory.h"
#include "core/components_ng/pattern/container_modal/enhance/container_modal_pattern_enhance.h"
#include "core/components_ng/pattern/custom/custom_node_base.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/navigation/navigation_group_node.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "core/components_ng/pattern/navigation/title_bar_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/stage/page_pattern.h"
#include "core/components_ng/pattern/stage/stage_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_manager.h"
#include "core/components_ng/pattern/window_scene/helper/window_scene_helper.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/safe_area_insets.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/ace_events.h"
#include "core/event/touch_event.h"
#include "core/image/image_file_cache.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline/pipeline_context.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace {
constexpr uint64_t ONE_MS_IN_NS = 1 * 1000 * 1000;
constexpr uint64_t INTERPOLATION_THRESHOLD = 100 * 1000 * 1000; // 100ms
constexpr int32_t INDEX_X = 0;
constexpr int32_t INDEX_Y = 1;
constexpr int32_t INDEX_TIME = 2;
constexpr int32_t TIME_THRESHOLD = 2 * 1000000; // 3 millisecond
constexpr int32_t PLATFORM_VERSION_TEN = 10;
constexpr int32_t USED_ID_FIND_FLAG = 3;                 // if args >3 , it means use id to find
constexpr int32_t MILLISECONDS_TO_NANOSECONDS = 1000000; // Milliseconds to nanoseconds
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

RefPtr<PipelineContext> PipelineContext::GetCurrentContextSafely()
{
    auto currentContainer = Container::CurrentSafely();
    CHECK_NULL_RETURN(currentContainer, nullptr);
    return DynamicCast<PipelineContext>(currentContainer->GetPipelineContext());
}

RefPtr<PipelineContext> PipelineContext::GetMainPipelineContext()
{
    auto pipeline = PipelineBase::GetMainPipelineContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    return DynamicCast<PipelineContext>(pipeline);
}

bool PipelineContext::NeedSoftKeyboard()
{
    auto focusNode = GetFocusNode();
    if (!focusNode) {
        return false;
    }
    TAG_LOGI(AceLogTag::ACE_KEYBOARD, "Focus node id is: %{public}d, tag is %{public}s", focusNode->GetId(),
        focusNode->GetTag().c_str());
    auto pattern = focusNode->GetPattern();
    CHECK_NULL_RETURN(pattern, false);
    bool isNeed = pattern->NeedSoftKeyboard();
#ifdef WINDOW_SCENE_SUPPORTED
    if (isNeed) {
        isNeed = WindowSceneHelper::GetNeedKeyboardOnFocusFlag(focusNode);
    }
#endif
    TAG_LOGI(AceLogTag::ACE_KEYBOARD, "need soft keyboard %{public}d", isNeed);
    return isNeed;
}

RefPtr<PipelineContext> PipelineContext::GetContextByContainerId(int32_t containerId)
{
    auto preContainer = Container::GetContainer(containerId);
    CHECK_NULL_RETURN(preContainer, nullptr);
    return DynamicCast<PipelineContext>(preContainer->GetPipelineContext());
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

void PipelineContext::AddDirtyPropertyNode(const RefPtr<FrameNode>& dirtyNode)
{
    dirtyPropertyNodes_.emplace(dirtyNode);
    hasIdleTasks_ = true;
    RequestFrame();
}

void PipelineContext::AddDirtyCustomNode(const RefPtr<UINode>& dirtyNode)
{
    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(dirtyNode);
    auto customNode = DynamicCast<CustomNode>(dirtyNode);
    if (customNode && !dirtyNode->GetInspectorIdValue("").empty()) {
        ACE_LAYOUT_SCOPED_TRACE("AddDirtyCustomNode[%s][self:%d][parent:%d][key:%s]",
            customNode->GetJSViewName().c_str(),
            dirtyNode->GetId(), dirtyNode->GetParent() ? dirtyNode->GetParent()->GetId() : 0,
            dirtyNode->GetInspectorIdValue("").c_str());
    } else if (customNode) {
        ACE_LAYOUT_SCOPED_TRACE("AddDirtyCustomNode[%s][self:%d][parent:%d]",
            customNode->GetJSViewName().c_str(),
            dirtyNode->GetId(), dirtyNode->GetParent() ? dirtyNode->GetParent()->GetId() : 0);
    }
    dirtyNodes_.emplace(dirtyNode);
    hasIdleTasks_ = true;
    RequestFrame();
}

void PipelineContext::AddDirtyLayoutNode(const RefPtr<FrameNode>& dirty)
{
    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(dirty);
    if (!dirty->GetInspectorIdValue("").empty()) {
        ACE_LAYOUT_SCOPED_TRACE("AddDirtyLayoutNode[%s][self:%d][parent:%d][key:%s]",
            dirty->GetTag().c_str(),
            dirty->GetId(), dirty->GetParent() ? dirty->GetParent()->GetId() : 0,
            dirty->GetInspectorIdValue("").c_str());
    } else {
        ACE_LAYOUT_SCOPED_TRACE("AddDirtyLayoutNode[%s][self:%d][parent:%d]", dirty->GetTag().c_str(),
            dirty->GetId(), dirty->GetParent() ? dirty->GetParent()->GetId() : 0);
    }
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
    if (!dirty->GetInspectorIdValue("").empty()) {
        ACE_LAYOUT_SCOPED_TRACE("AddDirtyRenderNode[%s][self:%d][parent:%d][key:%s]", dirty->GetTag().c_str(),
            dirty->GetId(), dirty->GetParent() ? dirty->GetParent()->GetId() : 0,
            dirty->GetInspectorIdValue("").c_str());
    } else {
        ACE_LAYOUT_SCOPED_TRACE("AddDirtyRenderNode[%s][self:%d][parent:%d]", dirty->GetTag().c_str(),
            dirty->GetId(), dirty->GetParent() ? dirty->GetParent()->GetId() : 0);
    }
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

    // node api property diff before ets update.
    decltype(dirtyPropertyNodes_) dirtyPropertyNodes(std::move(dirtyPropertyNodes_));
    dirtyPropertyNodes_.clear();
    for (const auto& node : dirtyPropertyNodes) {
        node->ProcessPropertyDiff();
    }

    // SomeTimes, customNode->Update may add some dirty custom nodes to dirtyNodes_,
    // use maxFlushTimes to avoid dead cycle.
    int maxFlushTimes = 3;
    while (!dirtyNodes_.empty() && maxFlushTimes > 0) {
        ArkUIPerfMonitor::GetInstance().RecordNodeNum(dirtyNodes_.size());
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

std::pair<float, float> PipelineContext::LinearInterpolation(const std::tuple<float, float, uint64_t>& history,
    const std::tuple<float, float, uint64_t>& current, const uint64_t nanoTimeStamp)
{
    if (nanoTimeStamp == std::get<INDEX_TIME>(history) || nanoTimeStamp == std::get<INDEX_TIME>(current)) {
        return std::make_pair(0.0f, 0.0f);
    }
    if (std::get<INDEX_TIME>(current) <= std::get<INDEX_TIME>(history)) {
        return std::make_pair(0.0f, 0.0f);
    }
    if (std::get<INDEX_TIME>(current) - std::get<INDEX_TIME>(history) > INTERPOLATION_THRESHOLD) {
        return std::make_pair(0.0f, 0.0f);
    }
    if (nanoTimeStamp < std::get<INDEX_TIME>(history)) {
        return std::make_pair(0.0f, 0.0f);
    }
    if (nanoTimeStamp < std::get<INDEX_TIME>(current)) {
        float alpha = (float)(nanoTimeStamp - std::get<INDEX_TIME>(history)) /
                      (float)(std::get<INDEX_TIME>(current) - std::get<INDEX_TIME>(history));
        float x = std::get<INDEX_X>(history) + alpha * (std::get<INDEX_X>(current) - std::get<INDEX_X>(history));
        float y = std::get<INDEX_Y>(history) + alpha * (std::get<INDEX_Y>(current) - std::get<INDEX_Y>(history));
        return std::make_pair(x, y);
    } else if (nanoTimeStamp > std::get<INDEX_TIME>(current)) {
        float alpha = (float)(nanoTimeStamp - std::get<INDEX_TIME>(current)) /
                      (float)(std::get<INDEX_TIME>(current) - std::get<INDEX_TIME>(history));
        float x = std::get<INDEX_X>(current) + alpha * (std::get<INDEX_X>(current) - std::get<INDEX_X>(history));
        float y = std::get<INDEX_Y>(current) + alpha * (std::get<INDEX_Y>(current) - std::get<INDEX_Y>(history));
        return std::make_pair(x, y);
    }
    return std::make_pair(0.0f, 0.0f);
}

std::tuple<float, float, uint64_t> PipelineContext::GetAvgPoint(
    const std::vector<TouchEvent>& events, const bool isScreen)
{
    float avgX = 0.0f;
    float avgY = 0.0f;
    uint64_t avgTime = 0;
    int32_t i = 0;
    uint64_t lastTime = 0;
    for (auto iter = events.begin(); iter != events.end(); iter++) {
        if (lastTime == 0 || static_cast<uint64_t>(iter->time.time_since_epoch().count()) != lastTime) {
            if (!isScreen) {
                avgX += iter->x;
                avgY += iter->y;
            } else {
                avgX += iter->screenX;
                avgY += iter->screenY;
            }
            avgTime += static_cast<uint64_t>(iter->time.time_since_epoch().count());
            i++;
            lastTime = static_cast<uint64_t>(iter->time.time_since_epoch().count());
        }
    }
    avgX /= i;
    avgY /= i;
    avgTime /= i;
    return std::make_tuple(avgX, avgY, avgTime);
}

std::pair<float, float> PipelineContext::GetResampleCoord(const std::vector<TouchEvent>& history,
    const std::vector<TouchEvent>& current, const uint64_t nanoTimeStamp, const bool isScreen)
{
    if (history.empty() || current.empty()) {
        return std::make_pair(0.0f, 0.0f);
    }
    auto historyPoint = GetAvgPoint(history, isScreen);
    auto currentPoint = GetAvgPoint(current, isScreen);

    if (SystemProperties::GetDebugEnabled()) {
        LOGI("input time is %{public}" PRIu64 "", nanoTimeStamp);
        for (auto iter : history) {
            LOGI("history point x %{public}f, y %{public}f, time %{public}" PRIu64 "", iter.x, iter.y,
                static_cast<uint64_t>(iter.time.time_since_epoch().count()));
        }
        LOGI("historyAvgPoint is x %{public}f, y %{public}f, time %{public}" PRIu64 "", std::get<INDEX_X>(historyPoint),
            std::get<INDEX_Y>(historyPoint), std::get<INDEX_TIME>(historyPoint));
        for (auto iter : current) {
            LOGI("current point x %{public}f, y %{public}f, time %{public}" PRIu64 "", iter.x, iter.y,
                static_cast<uint64_t>(iter.time.time_since_epoch().count()));
        }
        LOGI("currentAvgPoint is x %{public}f, y %{public}f, time %{public}" PRIu64 "", std::get<INDEX_X>(currentPoint),
            std::get<INDEX_Y>(currentPoint), std::get<INDEX_TIME>(currentPoint));
    }
    return LinearInterpolation(historyPoint, currentPoint, nanoTimeStamp);
}

TouchEvent PipelineContext::GetResampleTouchEvent(
    const std::vector<TouchEvent>& history, const std::vector<TouchEvent>& current, const uint64_t nanoTimeStamp)
{
    auto newXy = GetResampleCoord(history, current, nanoTimeStamp, false);
    auto newScreenXy = GetResampleCoord(history, current, nanoTimeStamp, true);
    TouchEvent newTouchEvent = GetLatestPoint(current, nanoTimeStamp);
    if (newXy.first != 0 && newXy.second != 0) {
        newTouchEvent.x = newXy.first;
        newTouchEvent.y = newXy.second;
        newTouchEvent.screenX = newScreenXy.first;
        newTouchEvent.screenY = newScreenXy.second;
        std::chrono::nanoseconds nanoseconds(nanoTimeStamp);
        newTouchEvent.time = TimeStamp(nanoseconds);
        newTouchEvent.history = current;
        newTouchEvent.isInterpolated = true;
    }
    if (SystemProperties::GetDebugEnabled()) {
        LOGI("Interpolate point is %{public}d, %{public}f, %{public}f, %{public}f, %{public}f, %{public}" PRIu64 "",
            newTouchEvent.id, newTouchEvent.x, newTouchEvent.y, newTouchEvent.screenX, newTouchEvent.screenY,
            static_cast<uint64_t>(newTouchEvent.time.time_since_epoch().count()));
    }
    return newTouchEvent;
}

TouchEvent PipelineContext::GetLatestPoint(const std::vector<TouchEvent>& current, const uint64_t nanoTimeStamp)
{
    TouchEvent result;
    uint64_t gap = UINT64_MAX;
    for (auto iter = current.begin(); iter != current.end(); iter++) {
        uint64_t timeStamp = static_cast<uint64_t>(iter->time.time_since_epoch().count());
        if (timeStamp == nanoTimeStamp) {
            result = *iter;
            return result;
        } else if (timeStamp > nanoTimeStamp) {
            if (timeStamp - nanoTimeStamp < gap) {
                gap = timeStamp - nanoTimeStamp;
                result = *iter;
            }
        } else {
            if (nanoTimeStamp - timeStamp < gap) {
                gap = nanoTimeStamp - timeStamp;
                result = *iter;
            }
        }
    }
    return result;
}

RefPtr<FrameNode> PipelineContext::HandleFocusNode()
{
    auto curRootNode = GetScreenNode();
    if (curRootNode == nullptr) {
        curRootNode = rootNode_;
    }
    CHECK_NULL_RETURN(curRootNode, nullptr);
    auto rootFocusHub = curRootNode->GetFocusHub();
    CHECK_NULL_RETURN(rootFocusHub, nullptr);
    RefPtr<FocusHub> lastFocusNode;
    std::list<RefPtr<FocusHub>> focusNodes = rootFocusHub->GetChildren();
    for (const auto& item : focusNodes) {
        if (item->IsCurrentFocus()) {
            lastFocusNode = item;
        }
    }
    while (lastFocusNode) {
        TAG_LOGD(AceLogTag::ACE_KEYBOARD, "curLastFocusNodeTAG:(%{public}s).", lastFocusNode->GetFrameName().c_str());
        if (!lastFocusNode->IsCurrentFocus() || !lastFocusNode->IsFocusableNode()) {
            TAG_LOGD(AceLogTag::ACE_KEYBOARD, "Is not CurrentFocus Or not FocusableNode.");
            break;
        }
        std::list<RefPtr<FocusHub>> focusNodesInner = lastFocusNode->GetChildren();
        auto openBreak = false;
        for (const auto& item : focusNodesInner) {
            if (item->IsCurrentFocus()) {
                lastFocusNode = item;
                openBreak = true;
            }
        }
        if (!openBreak) {
            TAG_LOGD(AceLogTag::ACE_KEYBOARD, "Is LastFocusNode, break.");
            break;
        }
    }
    if (lastFocusNode == nullptr) {
        TAG_LOGD(AceLogTag::ACE_KEYBOARD, "lastFocusNode is null.");
        return nullptr;
    }

    auto curFrameNode = lastFocusNode->GetFrameNode();
    if (curFrameNode == nullptr) {
        TAG_LOGD(AceLogTag::ACE_KEYBOARD, "lastFocusNode-curFrameNode is null.");
        return nullptr;
    }
    return curFrameNode;
}

#ifdef WINDOW_SCENE_SUPPORTED
void PipelineContext::IsSCBWindowKeyboard(RefPtr<FrameNode> curFrameNode)
{
    // Frame other window to SCB window Or inSCB window changes,hide keyboard.
    if ((windowFocus_.has_value() && windowFocus_.value()) ||
        curFocusNodeId_ != curFrameNode->GetId()) {
        TAG_LOGI(AceLogTag::ACE_KEYBOARD, "SCB Windowfocus first, ready to hide keyboard.");
        windowFocus_.reset();
        curFocusNodeId_ = curFrameNode->GetId();
        WindowSceneHelper::IsWindowSceneCloseKeyboard(curFrameNode);
        return;
    }
    // In windowscene, focus change, need close keyboard.
    if (needSoftKeyboard_.has_value() && !needSoftKeyboard_.value()) {
        TAG_LOGI(AceLogTag::ACE_KEYBOARD, "SCB WindowscenePage ready to close keyboard.");
        WindowSceneHelper::IsCloseKeyboard(curFrameNode);
        needSoftKeyboard_ = std::nullopt;
    }
}

void PipelineContext::IsNotSCBWindowKeyboard(RefPtr<FrameNode> curFrameNode)
{
    if ((windowFocus_.has_value() && windowFocus_.value()) ||
        (windowShow_.has_value() && windowShow_.value())) {
        TAG_LOGI(AceLogTag::ACE_KEYBOARD, "Nomal Window focus first, set focusflag to window.");
        windowFocus_.reset();
        windowShow_.reset();
        focusOnNodeCallback_();
        FocusHub::IsCloseKeyboard(curFrameNode);
        preNodeId_ = curFrameNode->GetId();
        return;
    }

    if (preNodeId_ != -1 && preNodeId_ == curFrameNode->GetId()) {
        TAG_LOGD(AceLogTag::ACE_KEYBOARD, "FocusNode not change.");
        return;
    }
    preNodeId_ = -1;

    if (needSoftKeyboard_.has_value() && !needSoftKeyboard_.value()) {
        TAG_LOGI(AceLogTag::ACE_KEYBOARD, "Nomal WindowPage ready to close keyboard.");
        FocusHub::IsCloseKeyboard(curFrameNode);
        needSoftKeyboard_ = std::nullopt;
    }
}
#endif

void PipelineContext::IsCloseSCBKeyboard()
{
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto manager = DynamicCast<TextFieldManagerNG>(textFieldManager_);
    CHECK_NULL_VOID(manager);
    if (container->IsKeyboard() || !manager->HasKeyboard()) {
        TAG_LOGI(AceLogTag::ACE_KEYBOARD, "focus in keyboard.");
        return;
    }

    RefPtr<FrameNode> curFrameNode = HandleFocusNode();
    if (curFrameNode == nullptr) {
        TAG_LOGD(AceLogTag::ACE_KEYBOARD, "curFrameNode null.");
        return;
    }
    TAG_LOGD(AceLogTag::ACE_KEYBOARD, "LastFocusNode,(%{public}s/%{public}d).",
        curFrameNode->GetTag().c_str(), curFrameNode->GetId());

#ifdef WINDOW_SCENE_SUPPORTED
    auto isSystem = WindowSceneHelper::IsWindowScene(curFrameNode);
    if (isSystem) {
        IsSCBWindowKeyboard(curFrameNode);
    } else {
        IsNotSCBWindowKeyboard(curFrameNode);
    }
#else
    FocusHub::IsCloseKeyboard(curFrameNode);
#endif
}

void PipelineContext::FlushVsync(uint64_t nanoTimestamp, uint32_t frameCount)
{
    CHECK_RUN_ON(UI);
    ACE_FUNCTION_TRACE();
    window_->Lock();
    auto recvTime = GetSysTimestamp();
    static const std::string abilityName = AceApplicationInfo::GetInstance().GetProcessName().empty()
                                               ? AceApplicationInfo::GetInstance().GetPackageName()
                                               : AceApplicationInfo::GetInstance().GetProcessName();
    window_->RecordFrameTime(nanoTimestamp, abilityName);
    resampleTimeStamp_ = nanoTimestamp - window_->GetVSyncPeriod() + ONE_MS_IN_NS;
#ifdef UICAST_COMPONENT_SUPPORTED
    do {
        auto container = Container::Current();
        CHECK_NULL_BREAK(container);
        auto distributedUI = container->GetDistributedUI();
        CHECK_NULL_BREAK(distributedUI);
        distributedUI->ApplyOneUpdate();
    } while (false);
#endif
    ProcessDelayTasks();
    DispatchDisplaySync(nanoTimestamp);
    FlushAnimation(nanoTimestamp);
    SetVsyncTime(nanoTimestamp);
    bool hasRunningAnimation = window_->FlushAnimation(nanoTimestamp);
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
    UIObserverHandler::GetInstance().HandleLayoutDoneCallBack();
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

    if (hasRunningAnimation || window_->HasUIAnimation()) {
        RequestFrame();
    }
    window_->FlushModifier();
    FlushFrameRate();
    if (dragWindowVisibleCallback_) {
        dragWindowVisibleCallback_();
        dragWindowVisibleCallback_ = nullptr;
    }
    FlushMessages();
    UIObserverHandler::GetInstance().HandleDrawCommandSendCallBack();
    InspectDrew();
    if (!isFormRender_ && onShow_ && onFocus_) {
        FlushFocusView();
        FlushFocus();
    }
    // Close input method in the SCB window.
    IsCloseSCBKeyboard();
    HandleOnAreaChangeEvent(nanoTimestamp);
    HandleVisibleAreaChangeEvent();
    if (isNeedFlushMouseEvent_) {
        FlushMouseEvent();
        isNeedFlushMouseEvent_ = false;
    }
    if (isNeedFlushAnimationStartTime_) {
        window_->FlushAnimationStartTime(nanoTimestamp);
        isNeedFlushAnimationStartTime_ = false;
    }
    needRenderNode_.clear();
    taskScheduler_->FlushAfterRenderTask();
    // Keep the call sent at the end of the function
    ResSchedReport::GetInstance().LoadPageEvent(ResDefine::LOAD_PAGE_COMPLETE_EVENT);
    window_->Unlock();
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

void PipelineContext::ProcessDelayTasks()
{
    if (delayedTasks_.empty()) {
        return;
    }
    auto currentTimeStamp = GetSysTimestamp();
    auto delayedTasks = std::move(delayedTasks_);
    auto result = std::remove_if(delayedTasks.begin(), delayedTasks.end(), [this, currentTimeStamp](const auto& task) {
        if (task.timeStamp + static_cast<int64_t>(task.time) * MILLISECONDS_TO_NANOSECONDS > currentTimeStamp) {
            delayedTasks_.emplace_back(task);
            return true;
        }
        return false;
    });
    delayedTasks.erase(result, delayedTasks.end());
    std::for_each(delayedTasks.begin(), delayedTasks.end(), [this](auto& delayedTask) {
        if (delayedTask.task) {
            delayedTask.task();
        }
    });
}

void PipelineContext::DispatchDisplaySync(uint64_t nanoTimestamp)
{
    CHECK_RUN_ON(UI);
    ACE_FUNCTION_TRACE();

    GetOrCreateUIDisplaySyncManager()->SetRefreshRateMode(window_->GetCurrentRefreshRateMode());
    GetOrCreateUIDisplaySyncManager()->SetVsyncPeriod(window_->GetVSyncPeriod());

    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().BeginFlushAnimation();
    }

    scheduleTasks_.clear();
    GetOrCreateUIDisplaySyncManager()->DispatchFunc(nanoTimestamp);

    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().EndFlushAnimation();
    }

    int32_t displaySyncRate = GetOrCreateUIDisplaySyncManager()->GetDisplaySyncRate();
    frameRateManager_->SetDisplaySyncRate(displaySyncRate);
}

void PipelineContext::FlushAnimation(uint64_t nanoTimestamp)
{
    CHECK_RUN_ON(UI);
    ACE_FUNCTION_TRACE();
    if (scheduleTasks_.empty()) {
        return;
    }
}

void PipelineContext::FlushMessages()
{
    ACE_FUNCTION_TRACE();
    window_->FlushTasks();
}

void PipelineContext::FlushUITasks()
{
    window_->Lock();
    taskScheduler_->FlushTask();
    window_->Unlock();
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

    auto requestFocusNode = dirtyRequestFocusNode_.Upgrade();
    if (!requestFocusNode) {
        dirtyRequestFocusNode_.Reset();
    } else {
        auto focusNodeHub = requestFocusNode->GetFocusHub();
        if (focusNodeHub && !focusNodeHub->RequestFocusImmediately()) {
            TAG_LOGI(AceLogTag::ACE_FOCUS, "Request focus by id on node: %{public}s/%{public}d return false",
                requestFocusNode->GetTag().c_str(), requestFocusNode->GetId());
        }
        dirtyFocusNode_.Reset();
        dirtyFocusScope_.Reset();
        dirtyRequestFocusNode_.Reset();
        return;
    }

    auto focusNode = dirtyFocusNode_.Upgrade();
    if (!focusNode || focusNode->GetFocusType() != FocusType::NODE) {
        dirtyFocusNode_.Reset();
    } else {
        auto focusNodeHub = focusNode->GetFocusHub();
        if (focusNodeHub && !focusNodeHub->RequestFocusImmediately()) {
            TAG_LOGI(AceLogTag::ACE_FOCUS, "Request focus on node: %{public}s/%{public}d return false",
                focusNode->GetTag().c_str(), focusNode->GetId());
        }
        dirtyFocusNode_.Reset();
        dirtyFocusScope_.Reset();
        dirtyRequestFocusNode_.Reset();
        return;
    }
    auto focusScope = dirtyFocusScope_.Upgrade();
    if (!focusScope || focusScope->GetFocusType() != FocusType::SCOPE) {
        dirtyFocusScope_.Reset();
    } else {
        auto focusScopeHub = focusScope->GetFocusHub();
        if (focusScopeHub && !focusScopeHub->RequestFocusImmediately()) {
            TAG_LOGI(AceLogTag::ACE_FOCUS, "Request focus on scope: %{public}s/%{public}d return false",
                focusScope->GetTag().c_str(), focusScope->GetId());
        }
        dirtyFocusNode_.Reset();
        dirtyFocusScope_.Reset();
        dirtyRequestFocusNode_.Reset();
        return;
    }
}

void PipelineContext::FlushFocusView()
{
    CHECK_NULL_VOID(focusManager_);
    auto lastFocusView = (focusManager_->GetLastFocusView()).Upgrade();
    CHECK_NULL_VOID(lastFocusView);
    auto lastFocusViewHub = lastFocusView->GetFocusHub();
    CHECK_NULL_VOID(lastFocusViewHub);
    if (lastFocusView && (!lastFocusView->IsRootScopeCurrentFocus() || !lastFocusView->GetIsViewHasFocused()) &&
        lastFocusViewHub->IsFocusableNode()) {
        lastFocusView->RequestDefaultFocus();
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
    window_->Lock();
    FlushBuild();
    FlushTouchEvents();
    taskScheduler_->FlushTask();
    FlushAnimationClosure();
    FlushMessages();
    FlushFocus();
    window_->Unlock();
}

void PipelineContext::FlushFrameRate()
{
    frameRateManager_->SetAnimateRate(window_->GetAnimateExpectedRate());
    if (frameRateManager_->IsRateChanged()) {
        auto rate = frameRateManager_->GetExpectedRate();
        ACE_SCOPED_TRACE("FlushFrameRate Expected frameRate = %d", rate);
        window_->FlushFrameRate(rate);
        frameRateManager_->SetIsRateChanged(false);
    }
}

void PipelineContext::FlushBuild()
{
    if (vsyncListener_ != nullptr) {
        ACE_SCOPED_TRACE("arkoala build");
        vsyncListener_();
    }
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
    window_->Lock();
    taskScheduler_->FlushTask();

    decltype(animationClosuresList_) temp(std::move(animationClosuresList_));
    for (const auto& animation : temp) {
        animation();
    }
    window_->Unlock();
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
    auto event = [](const GestureEvent& info) mutable {};
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
    RefPtr<AppBarView> appBar = AceType::MakeRefPtr<AppBarView>();
    auto atomicService = installationFree_ ? appBar->Create(stageNode) : nullptr;
    auto container = Container::Current();
    if (container) {
        container->SetAppBar(appBar);
    }
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
#ifdef WINDOW_SCENE_SUPPORTED
    uiExtensionManager_ = MakeRefPtr<UIExtensionManager>();
#endif
    accessibilityManagerNG_ = MakeRefPtr<AccessibilityManagerNG>();
    stageManager_ = MakeRefPtr<StageManager>(stageNode);
    overlayManager_ = MakeRefPtr<OverlayManager>(
        DynamicCast<FrameNode>(installationFree_ ? stageNode->GetParent()->GetParent() : stageNode->GetParent()));
    fullScreenManager_ = MakeRefPtr<FullScreenManager>(rootNode_);
    selectOverlayManager_ = MakeRefPtr<SelectOverlayManager>(rootNode_);
    if (!privacySensitiveManager_) {
        privacySensitiveManager_ = MakeRefPtr<PrivacySensitiveManager>();
    }
    postEventManager_ = MakeRefPtr<PostEventManager>();
    dragDropManager_ = MakeRefPtr<DragDropManager>();
    focusManager_ = MakeRefPtr<FocusManager>();
    sharedTransitionManager_ = MakeRefPtr<SharedOverlayManager>(
        DynamicCast<FrameNode>(installationFree_ ? stageNode->GetParent()->GetParent() : stageNode->GetParent()));

    OnAreaChangedFunc onAreaChangedFunc = [weakOverlayManger = AceType::WeakClaim(AceType::RawPtr(overlayManager_))](
                                              const RectF& /* oldRect */, const OffsetF& /* oldOrigin */,
                                              const RectF& /* rect */, const OffsetF& /* origin */) {
        auto overlay = weakOverlayManger.Upgrade();
        CHECK_NULL_VOID(overlay);
        overlay->HideAllMenus();
        SubwindowManager::GetInstance()->HideMenuNG(false);
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
#ifdef WINDOW_SCENE_SUPPORTED
    uiExtensionManager_ = MakeRefPtr<UIExtensionManager>();
#endif
    accessibilityManagerNG_ = MakeRefPtr<AccessibilityManagerNG>();
    // the subwindow for overlay not need stage
    stageManager_ = MakeRefPtr<StageManager>(nullptr);
    overlayManager_ = MakeRefPtr<OverlayManager>(rootNode_);
    fullScreenManager_ = MakeRefPtr<FullScreenManager>(rootNode_);
    selectOverlayManager_ = MakeRefPtr<SelectOverlayManager>(rootNode_);
    dragDropManager_ = MakeRefPtr<DragDropManager>();
    focusManager_ = MakeRefPtr<FocusManager>();
    postEventManager_ = MakeRefPtr<PostEventManager>();
}

RefPtr<AccessibilityManagerNG> PipelineContext::GetAccessibilityManagerNG()
{
    return accessibilityManagerNG_;
}

const RefPtr<StageManager>& PipelineContext::GetStageManager()
{
    return stageManager_;
}

const RefPtr<DragDropManager>& PipelineContext::GetDragDropManager()
{
    return dragDropManager_;
}

const RefPtr<FocusManager>& PipelineContext::GetFocusManager() const
{
    return focusManager_;
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
    ACE_FUNCTION_TRACE();
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

    UpdateSizeChangeReason(type, rsTransaction);

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

void PipelineContext::OnFoldStatusChange(FoldStatus foldStatus)
{
    for (auto&& [id, callback] : foldStatusChangedCallbackMap_) {
        if (callback) {
            callback(foldStatus);
        }
    }
}

void PipelineContext::OnFoldDisplayModeChange(FoldDisplayMode foldDisplayMode)
{
    for (auto&& [id, callback] : foldDisplayModeChangedCallbackMap_) {
        if (callback) {
            callback(foldDisplayMode);
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
        case WindowSizeChangeReason::FULL_TO_SPLIT:
        case WindowSizeChangeReason::FULL_TO_FLOATING: {
            StartFullToMultWindowAnimation(width, height, type, rsTransaction);
            break;
        }
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
    if (maximizeMode == MaximizeMode::MODE_FULL_FILL || maximizeMode == MaximizeMode::MODE_AVOID_SYSTEM_BAR) {
        int32_t preWidth = GetRootRect().Width();
        int32_t preHeight = GetRootRect().Height();
        if (width > preWidth && height > preHeight) {
            duration = 0;
        }
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

void PipelineContext::StartFullToMultWindowAnimation(int32_t width, int32_t height, WindowSizeChangeReason type,
    const std::shared_ptr<Rosen::RSTransaction>& rsTransaction)
{
    LOGI("Root node start multiple window animation, type = %{public}d, width = %{public}d, height = %{public}d", type,
        width, height);
#ifdef ENABLE_ROSEN_BACKEND
    if (rsTransaction) {
        FlushMessages();
        rsTransaction->Begin();
    }
#endif
    float response = 0.5f;
    float dampingFraction = 1.0f;
    AnimationOption option;
    if (type == WindowSizeChangeReason::FULL_TO_FLOATING) {
        response = 0.45f;
        dampingFraction = 0.75f;
    }
    auto springMotion = AceType::MakeRefPtr<ResponsiveSpringMotion>(response, dampingFraction, 0);
    option.SetCurve(springMotion);
    auto weak = WeakClaim(this);
    Animate(option, springMotion, [width, height, weak]() {
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
    if (Container::CurrentId() < MIN_SUBCONTAINER_ID) {
        ScreenSystemManager::GetInstance().SetWindowInfo(rootWidth_, density_, dipScale_);
        ScreenSystemManager::GetInstance().OnSurfaceChanged(width);
    }
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

void PipelineContext::UpdateNavSafeArea(const SafeAreaInsets& navSafeArea)
{
    CHECK_NULL_VOID(minPlatformVersion_ >= PLATFORM_VERSION_TEN);
    if (safeAreaManager_->UpdateNavArea(navSafeArea)) {
        AnimateOnSafeAreaUpdate();
    }
}

void PipelineContext::UpdateOriginAvoidArea(const Rosen::AvoidArea& avoidArea, uint32_t type)
{
#ifdef WINDOW_SCENE_SUPPORTED
    CHECK_NULL_VOID(uiExtensionManager_);
    uiExtensionManager_->TransferOriginAvoidArea(avoidArea, type);
#endif
}

void PipelineContext::UpdateSizeChangeReason(
    WindowSizeChangeReason type, const std::shared_ptr<Rosen::RSTransaction>& rsTransaction)
{
#ifdef WINDOW_SCENE_SUPPORTED
    CHECK_NULL_VOID(uiExtensionManager_);
    uiExtensionManager_->NotifySizeChangeReason(type, rsTransaction);
#endif
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

void PipelineContext::SetIsNeedAvoidWindow(bool value)
{
    if (safeAreaManager_->SetIsNeedAvoidWindow(value)) {
        SyncSafeArea();
    }
}

PipelineBase::SafeAreaInsets PipelineContext::GetSafeArea() const
{
    return safeAreaManager_->GetSafeArea();
}

PipelineBase::SafeAreaInsets PipelineContext::GetSafeAreaWithoutProcess() const
{
    return safeAreaManager_->GetSafeAreaWithoutProcess();
}

void PipelineContext::SyncSafeArea(bool onKeyboard)
{
    CHECK_NULL_VOID(stageManager_);
    auto page = stageManager_->GetLastPage();
    if (page) {
        page->MarkDirtyNode(onKeyboard && !safeAreaManager_->KeyboardSafeAreaEnabled() ? PROPERTY_UPDATE_LAYOUT
                                                                                       : PROPERTY_UPDATE_MEASURE);
    }
    SubwindowManager::GetInstance()->MarkDirtyDialogSafeArea();
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

void PipelineContext::CheckVirtualKeyboardHeight()
{
#ifdef WINDOW_SCENE_SUPPORTED
    if (uiExtensionManager_) {
        return;
    }
#endif
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto keyboardArea = container->GetKeyboardSafeArea();
    auto keyboardHeight = keyboardArea.bottom_.end - keyboardArea.bottom_.start;
    if (keyboardHeight > 0) {
        LOGI("Current View has keyboard.");
    }
    OnVirtualKeyboardHeightChange(keyboardHeight);
}

void PipelineContext::OnVirtualKeyboardHeightChange(
    float keyboardHeight, const std::shared_ptr<Rosen::RSTransaction>& rsTransaction)
{
    CHECK_RUN_ON(UI);
    // prevent repeated trigger with same keyboardHeight
    if (NearEqual(keyboardHeight, safeAreaManager_->GetKeyboardInset().Length())) {
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
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
        if (offsetFix > 0.0f && positionYWithOffset < offsetFix) {
            offsetFix = keyboardHeight - (rootSize.Height() - positionYWithOffset - height);
        }
#endif
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

void PipelineContext::OnVirtualKeyboardHeightChange(
    float keyboardHeight, double positionY, double height, const std::shared_ptr<Rosen::RSTransaction>& rsTransaction)
{
    CHECK_RUN_ON(UI);
    // prevent repeated trigger with same keyboardHeight
    CHECK_NULL_VOID(safeAreaManager_);
    auto manager = DynamicCast<TextFieldManagerNG>(PipelineBase::GetTextFieldManager());
    CHECK_NULL_VOID(manager);
    if (NearEqual(keyboardHeight, safeAreaManager_->GetKeyboardInset().Length()) &&
        prevKeyboardAvoidMode_ == safeAreaManager_->KeyboardSafeAreaEnabled() && manager->PrevHasTextFieldPattern()) {
        TAG_LOGD(
            AceLogTag::ACE_KEYBOARD, "KeyboardHeight as same as last time, don't need to calculate keyboardOffset");
        return;
    }
    manager->UpdatePrevHasTextFieldPattern();
    prevKeyboardAvoidMode_ = safeAreaManager_->KeyboardSafeAreaEnabled();

    ACE_FUNCTION_TRACE();
#ifdef ENABLE_ROSEN_BACKEND
    if (rsTransaction) {
        FlushMessages();
        rsTransaction->Begin();
    }
#endif

    auto weak = WeakClaim(this);
    auto func = [weak, keyboardHeight, positionY, height, manager]() mutable {
        auto context = weak.Upgrade();
        CHECK_NULL_VOID(context);
        context->safeAreaManager_->UpdateKeyboardSafeArea(keyboardHeight);
        if (keyboardHeight > 0) {
            // add height of navigation bar
            keyboardHeight += context->safeAreaManager_->GetSystemSafeArea().bottom_.Length();
        }

        SizeF rootSize { static_cast<float>(context->rootWidth_), static_cast<float>(context->rootHeight_) };
        float keyboardOffset = context->safeAreaManager_->GetKeyboardOffset();
        float positionYWithOffset = positionY - keyboardOffset;
        if (rootSize.Height() - positionY - height < 0) {
            height = rootSize.Height() - positionY;
        }
        float offsetFix = (rootSize.Height() - positionY - height) < keyboardHeight
                              ? keyboardHeight - (rootSize.Height() - positionY - height)
                              : keyboardHeight;
        if (NearZero(keyboardHeight)) {
            context->safeAreaManager_->UpdateKeyboardOffset(0.0f);
        } else if (positionYWithOffset + height > (rootSize.Height() - keyboardHeight) && offsetFix > 0.0f) {
            context->safeAreaManager_->UpdateKeyboardOffset(-offsetFix);
        } else if (LessOrEqual(rootSize.Height() - positionYWithOffset - height, height) &&
                   LessOrEqual(rootSize.Height() - positionYWithOffset, keyboardHeight)) {
            context->safeAreaManager_->UpdateKeyboardOffset(-keyboardHeight);
        } else if ((positionYWithOffset + height > rootSize.Height() - keyboardHeight &&
                       positionYWithOffset < rootSize.Height() - keyboardHeight && height < keyboardHeight / 2.0f) &&
                   NearZero(context->rootNode_->GetGeometryNode()->GetFrameOffset().GetY())) {
            context->safeAreaManager_->UpdateKeyboardOffset(-height - offsetFix / 2.0f);
        } else {
            context->safeAreaManager_->UpdateKeyboardOffset(0.0f);
        }
        TAG_LOGI(AceLogTag::ACE_KEYBOARD,
            "keyboardHeight: %{public}f, positionY: %{public}f, textHeight: %{public}f, final calculate keyboard"
            "offset is %{public}f",
            keyboardHeight, positionY, height, context->safeAreaManager_->GetKeyboardOffset());
        context->SyncSafeArea(true);
        // layout immediately
        context->FlushUITasks();

        manager->ScrollTextFieldToSafeArea();
        context->FlushUITasks();
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
    CHECK_RUN_ON(PLATFORM);
    auto frontend = weakFrontend_.Upgrade();
    if (!frontend) {
        // return back.
        return false;
    }

    // If the tag of the last child of the rootnode is video, exit full screen.
    if (fullScreenManager_->OnBackPressed()) {
        LOGI("fullscreen component：video or web consumed backpressed event");
        return true;
    }

    // if has sharedTransition, back press will stop the sharedTransition
    if (sharedTransitionManager_->OnBackPressed()) {
        LOGI("sharedTransition consumed backpressed event");
        return true;
    }

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
            hasOverlay = selectOverlay->ResetSelectionAndDestroySelectOverlay();
            hasOverlay |= overlay->RemoveOverlay(true);
        },
        TaskExecutor::TaskType::UI);
    if (hasOverlay) {
        LOGI("popup consumed backpressed event");
        return true;
    }

    auto textfieldManager = DynamicCast<TextFieldManagerNG>(PipelineBase::GetTextFieldManager());
    if (textfieldManager && textfieldManager->OnBackPressed()) {
        LOGI("textfield consumed backpressed event");
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
        LOGI("Navigation consumed backpressed event");
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
        LOGI("router consumed backpressed event");
        return true;
    }
    return false;
}

RefPtr<FrameNode> PipelineContext::FindNavigationNodeToHandleBack(const RefPtr<UINode>& node)
{
    const auto& children = node->GetChildren();
    for (auto iter = children.rbegin(); iter != children.rend(); ++iter) {
        auto& child = *iter;
        auto childNode = AceType::DynamicCast<FrameNode>(child);
        if (childNode && childNode->GetLayoutProperty()) {
            auto property = childNode->GetLayoutProperty();
            if (property->GetVisibilityValue(VisibleType::VISIBLE) != VisibleType::VISIBLE ||
                !childNode->IsActive()) {
                continue;
            }
        }
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
    for (auto& pair : isFocusActiveUpdateEvents_) {
        if (pair.second) {
            pair.second(isFocusActive_);
        }
    }
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
    OnTouchEvent(point, rootNode_, isSubPipe);
}

void PipelineContext::OnMouseEvent(const MouseEvent& event)
{
    OnMouseEvent(event, rootNode_);
}

void PipelineContext::OnAxisEvent(const AxisEvent& event)
{
    OnAxisEvent(event, rootNode_);
}

void PipelineContext::OnTouchEvent(const TouchEvent& point, const RefPtr<FrameNode>& node, bool isSubPipe)
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

    SerializedGesture etsSerializedGesture;
    if (point.type != TouchType::DOWN) {
        HandleEtsCardTouchEvent(point, etsSerializedGesture);
    }

    auto oriPoint = point;
    auto scalePoint = point.CreateScalePoint(GetViewScale());
    ResSchedReport::GetInstance().OnTouchEvent(scalePoint.type);
    if (scalePoint.type != TouchType::MOVE && scalePoint.type != TouchType::PULL_MOVE &&
        scalePoint.type != TouchType::HOVER_MOVE) {
        eventManager_->GetEventTreeRecord().AddTouchPoint(scalePoint);
        TAG_LOGI(AceLogTag::ACE_INPUTTRACKING,
            "TouchEvent Process in ace_container: "
            "eventInfo: id:%{public}d, pointX=%{public}f pointY=%{public}f "
            "type=%{public}d",
            scalePoint.id, scalePoint.x, scalePoint.y, (int)scalePoint.type);
    }
    eventManager_->SetInstanceId(GetInstanceId());
    if (scalePoint.type != TouchType::MOVE && historyPointsById_.find(scalePoint.id) != historyPointsById_.end()) {
        historyPointsById_.erase(scalePoint.id);
    }
    if (scalePoint.type == TouchType::DOWN) {
        // Set focus state inactive while touch down event received
        SetIsFocusActive(false);
        TouchRestrict touchRestrict { TouchRestrict::NONE };
        touchRestrict.sourceType = point.sourceType;
        touchRestrict.touchEvent = point;
        eventManager_->TouchTest(scalePoint, node, touchRestrict, GetPluginEventOffset(), viewScale_, isSubPipe);
        if (!touchRestrict.childTouchTestList.empty()) {
            scalePoint.childTouchTestList = touchRestrict.childTouchTestList;
        }
        touchTestResults_ = eventManager_->touchTestResults_;

        HandleEtsCardTouchEvent(oriPoint, etsSerializedGesture);

        if (etsSerializedGesture.data.size() != 0) {
            GestureGroup rebirth(GestureMode::Exclusive);
            rebirth.Deserialize(etsSerializedGesture.data.data());
            auto recognizer = rebirth.CreateRecognizer();
            if (recognizer) {
                recognizer->SetInnerFlag(true);
                recognizer->BeginReferee(scalePoint.id, true);
                std::list<RefPtr<NGGestureRecognizer>> combined;
                combined.emplace_back(recognizer);
                for (auto iter = touchTestResults_[point.id].begin();
                    iter != touchTestResults_[point.id].end(); iter++) {
                    auto outRecognizer = AceType::DynamicCast<NGGestureRecognizer>(*iter);
                    if (outRecognizer) {
                        combined.emplace_back(outRecognizer);
                        touchTestResults_[point.id].erase(iter);
                        break;
                    }
                }
                auto exclusiveRecognizer = AceType::MakeRefPtr<ExclusiveRecognizer>(std::move(combined));
                exclusiveRecognizer->AttachFrameNode(node);
                exclusiveRecognizer->BeginReferee(scalePoint.id);
                touchTestResults_[point.id].emplace_back(exclusiveRecognizer);
                eventManager_->touchTestResults_ = touchTestResults_;
                eventManager_->SetInnerFlag(true);
            }
        }
        if (IsFormRender() && touchTestResults_.find(point.id) != touchTestResults_.end()) {
            for (const auto& touchResult : touchTestResults_[point.id]) {
                auto recognizer = AceType::DynamicCast<NG::RecognizerGroup>(touchResult);
                if (recognizer) {
                    auto gesture = recognizer->CreateGestureFromRecognizer();
                    if (gesture) {
                        serializedGesture_.data = std::vector<char>(gesture->SizeofMe());
                        gesture->Serialize(serializedGesture_.data.data());
                    }
                    break;
                }
            }
        }
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

    // Currently, SetupRootElement is executed later than InitializeCallback in AceContainer.
    // We need to check whether accessibilityManagerNG_ is created.
    if (accessibilityManagerNG_ != nullptr) {
        accessibilityManagerNG_->HandleAccessibilityHoverEvent(node, scalePoint);
    }

    if (scalePoint.type == TouchType::MOVE) {
        if (!eventManager_->GetInnerFlag()) {
            auto mockPoint = point;
            mockPoint.type = TouchType::CANCEL;
            HandleEtsCardTouchEvent(mockPoint, etsSerializedGesture);
            RemoveEtsCardTouchEventCallback(mockPoint.id);
        }
        touchEvents_.emplace_back(point);
        hasIdleTasks_ = true;
        RequestFrame();
        return;
    }

    if (scalePoint.type == TouchType::UP) {
        lastTouchTime_ = GetTimeFromExternalTimer();
        CompensateTouchMoveEvent(scalePoint);
    }

    eventManager_->DispatchTouchEvent(scalePoint);

    if ((scalePoint.type == TouchType::UP) || (scalePoint.type == TouchType::CANCEL)) {
        // need to reset touchPluginPipelineContext_ for next touch down event.
        touchPluginPipelineContext_.clear();
        RemoveEtsCardTouchEventCallback(point.id);
        ResetDraggingStatus(scalePoint);
    }
    if (scalePoint.type != TouchType::MOVE) {
        lastDispatchTime_.erase(scalePoint.id);
        idToTouchPoints_.erase(scalePoint.id);
    }

    hasIdleTasks_ = true;
    RequestFrame();
}

bool PipelineContext::CompensateTouchMoveEventFromUnhandledEvents(const TouchEvent& event)
{
    std::optional<TouchEvent> lastMoveEvent;
    if (!touchEvents_.empty()) {
        for (auto iter = touchEvents_.begin(); iter != touchEvents_.end();) {
            auto movePoint = (*iter).CreateScalePoint(GetViewScale());
            if (event.id == movePoint.id) {
                lastMoveEvent = movePoint;
                iter = touchEvents_.erase(iter);
            } else {
                ++iter;
            }
        }
        if (lastMoveEvent.has_value()) {
            eventManager_->SetLastMoveBeforeUp(event.sourceType == SourceType::MOUSE);
            eventManager_->DispatchTouchEvent(lastMoveEvent.value());
            eventManager_->SetLastMoveBeforeUp(false);
        } else {
            TAG_LOGI(AceLogTag::ACE_INPUTTRACKING,
                "Finger id: %{public}d, not found unhandled move event, compensate failed.", event.id);
        }
        return true;
    }
    return false;
}

void PipelineContext::CompensateTouchMoveEvent(const TouchEvent& event)
{
    if (event.type != TouchType::UP) {
        // If not up, no need to compensate touch move event.
        return;
    }
    if (!CompensateTouchMoveEventFromUnhandledEvents(event)) {
        // Compensate touch move event with all touch move Event before up has been handled.
        auto lastEventIter = idToTouchPoints_.find(event.id);
        if (lastEventIter != idToTouchPoints_.end()) {
            auto iter = lastDispatchTime_.find(lastEventIter->first);
            if (iter != lastDispatchTime_.end()) {
                ACE_SCOPED_TRACE("CompensateTouchMoveEvent last move event time: %s last dispatch time: %s",
                    std::to_string(static_cast<uint64_t>(lastEventIter->second.time.time_since_epoch().count()))
                        .c_str(),
                    std::to_string(iter->second).c_str());
                if (static_cast<uint64_t>(lastEventIter->second.time.time_since_epoch().count()) > iter->second) {
                    eventManager_->SetLastMoveBeforeUp(event.sourceType == SourceType::MOUSE);
                    eventManager_->DispatchTouchEvent(lastEventIter->second);
                    eventManager_->SetLastMoveBeforeUp(false);
                }
            }
        }
    }

    auto lastEventIter = idToTouchPoints_.find(event.id);
    if (lastEventIter != idToTouchPoints_.end()) {
        ACE_SCOPED_TRACE("Finger id: %d process last move event eventId: %d", lastEventIter->first,
            lastEventIter->second.touchEventId);
    }
}

void PipelineContext::ResetDraggingStatus(const TouchEvent& touchPoint)
{
    auto manager = GetDragDropManager();
    CHECK_NULL_VOID(manager);
    if (manager->IsDraggingPressed(touchPoint.id)) {
        manager->SetDraggingPressedState(false);
    }
    if (manager->IsDragging() && manager->IsSameDraggingPointer(touchPoint.id)) {
        manager->OnDragEnd(PointerEvent(touchPoint.x, touchPoint.y), "");
    }
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
    if (isDensityChanged_) {
        UIObserverHandler::GetInstance().NotifyDensityChange(density_);
    }
}

void PipelineContext::RegisterDumpInfoListener(const std::function<void(const std::vector<std::string>&)>& callback)
{
    dumpListeners_.push_back(callback);
}

bool PipelineContext::DumpPageViewData(const RefPtr<FrameNode>& node, RefPtr<ViewDataWrap> viewDataWrap)
{
    CHECK_NULL_RETURN(viewDataWrap, false);
    RefPtr<FrameNode> pageNode = nullptr;
    if (node == nullptr) {
        if (stageManager_) {
            pageNode = stageManager_->GetLastPage();
        }
    } else {
        pageNode = node->GetPageNode();
    }
    CHECK_NULL_RETURN(pageNode, false);
    pageNode->DumpViewDataPageNodes(viewDataWrap);
    auto pagePattern = pageNode->GetPattern<NG::PagePattern>();
    CHECK_NULL_RETURN(pagePattern, false);
    auto pageInfo = pagePattern->GetPageInfo();
    CHECK_NULL_RETURN(pageInfo, false);
    viewDataWrap->SetPageUrl(pageInfo->GetPageUrl());
    return true;
}

bool PipelineContext::CheckNeedAutoSave()
{
    CHECK_NULL_RETURN(stageManager_, false);
    auto pageNode = stageManager_->GetLastPage();
    CHECK_NULL_RETURN(pageNode, false);
    return pageNode->NeedRequestAutoSave();
}

bool PipelineContext::CheckPageFocus()
{
    CHECK_NULL_RETURN(stageManager_, true);
    auto pageNode = stageManager_->GetLastPage();
    CHECK_NULL_RETURN(pageNode, true);
    return pageNode->GetFocusHub() && pageNode->GetFocusHub()->IsCurrentFocus();
}

void PipelineContext::NotifyFillRequestSuccess(AceAutoFillType autoFillType, RefPtr<ViewDataWrap> viewDataWrap)
{
    CHECK_NULL_VOID(viewDataWrap);
    auto pageNodeInfoWraps = viewDataWrap->GetPageNodeInfoWraps();
    for (const auto& item : pageNodeInfoWraps) {
        if (item == nullptr) {
            continue;
        }
        auto frameNode = DynamicCast<FrameNode>(ElementRegister::GetInstance()->GetUINodeById(item->GetId()));
        if (frameNode == nullptr) {
            TAG_LOGW(AceLogTag::ACE_AUTO_FILL, "frameNode is not found, id=%{public}d", item->GetId());
            continue;
        }
        frameNode->NotifyFillRequestSuccess(item, autoFillType);
    }
}

void PipelineContext::NotifyFillRequestFailed(RefPtr<FrameNode> node, int32_t errCode)
{
    CHECK_NULL_VOID(node);
    node->NotifyFillRequestFailed(errCode);
}

bool PipelineContext::OnDumpInfo(const std::vector<std::string>& params) const
{
    ACE_DCHECK(!params.empty());
    if (params[0] == "-element") {
        if (params.size() > 1 && params[1] == "-lastpage") {
            auto lastPage = stageManager_->GetLastPage();
            if (params.size() < USED_ID_FIND_FLAG && lastPage) {
                lastPage->DumpTree(0);
                DumpLog::GetInstance().OutPutBySize();
            }
            if (params.size() == USED_ID_FIND_FLAG && lastPage && !lastPage->DumpTreeById(0, params[2])) {
                DumpLog::GetInstance().Print(
                    "There is no id matching the ID in the parameter,please check whether the id is correct");
            }
        } else {
            rootNode_->DumpTree(0);
            DumpLog::GetInstance().OutPutBySize();
        }
    } else if (params[0] == "-navigation") {
        auto navigationDumpMgr = GetNavigationDumpManager();
        if (navigationDumpMgr) {
            navigationDumpMgr->OnDumpInfo();
        }
    } else if (params[0] == "-focus") {
        if (rootNode_->GetFocusHub()) {
            rootNode_->GetFocusHub()->DumpFocusTree(0);
        }
    } else if (params[0] == "-focuswindowscene") {
        auto windowSceneNode = GetFocusedWindowSceneNode();
        auto windowSceneFocusHub = windowSceneNode ? windowSceneNode->GetFocusHub() : nullptr;
        if (windowSceneFocusHub) {
            windowSceneFocusHub->DumpFocusTree(0);
        }
    } else if (params[0] == "-focusmanager") {
        if (focusManager_) {
            focusManager_->DumpFocusManager();
        }
    } else if (params[0] == "-accessibility" || params[0] == "-inspector") {
        auto accessibilityManager = GetAccessibilityManager();
        if (accessibilityManager) {
            accessibilityManager->OnDumpInfoNG(params, windowId_);
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
    } else if (params[0] == "-event") {
        if (eventManager_) {
            eventManager_->DumpEvent();
        }
    } else if (params[0] == "-imagecache") {
        if (imageCache_) {
            imageCache_->DumpCacheInfo();
        }
    } else if (params[0] == "-imagefilecache") {
        ImageFileCache::GetInstance().DumpCacheInfo();
    } else if (params[0] == "-default") {
        rootNode_->DumpTree(0);
        DumpLog::GetInstance().OutPutDefault();
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
        DumpLog::GetInstance().Print(1, "NowTime: " + std::to_string(GetSysTimestamp()));
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
        bool needInterpolation = true;
        std::unordered_map<int32_t, TouchEvent> newIdTouchPoints;
        for (auto iter = touchEvents.rbegin(); iter != touchEvents.rend(); ++iter) {
            auto scalePoint = (*iter).CreateScalePoint(GetViewScale());
            idToTouchPoints.emplace(scalePoint.id, scalePoint);
            idToTouchPoints[scalePoint.id].history.insert(idToTouchPoints[scalePoint.id].history.begin(), scalePoint);
            needInterpolation = iter->type != TouchType::MOVE ? false : true;
        }
        if (focusWindowId_.has_value()) {
            needInterpolation = false;
        }
        if (needInterpolation) {
            auto targetTimeStamp = resampleTimeStamp_;
            for (const auto& idIter : idToTouchPoints) {
                auto stamp =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(idIter.second.time.time_since_epoch()).count();
                if (targetTimeStamp > static_cast<uint64_t>(stamp)) {
                    LOGI("Skip interpolation when there is no touch event after interpolation time point. "
                         "(last stamp:%{public}" PRIu64 ", target stamp:%{public}" PRIu64 ")",
                        static_cast<uint64_t>(stamp), targetTimeStamp);
                    continue;
                }
                TouchEvent newTouchEvent =
                    GetResampleTouchEvent(historyPointsById_[idIter.first], idIter.second.history, targetTimeStamp);
                if (newTouchEvent.x != 0 && newTouchEvent.y != 0) {
                    newIdTouchPoints[idIter.first] = newTouchEvent;
                }
                historyPointsById_[idIter.first] = idIter.second.history;
            }
        }
        std::list<TouchEvent> touchPoints;
        for (const auto& iter : idToTouchPoints) {
            lastDispatchTime_[iter.first] = GetVsyncTime();
            auto it = newIdTouchPoints.find(iter.first);
            if (it != newIdTouchPoints.end()) {
                touchPoints.emplace_back(it->second);
            } else {
                touchPoints.emplace_back(iter.second);
            }
        }
        auto maxSize = touchPoints.size();
        for (auto iter = touchPoints.rbegin(); iter != touchPoints.rend(); ++iter) {
            maxSize--;
            if (maxSize == 0) {
                eventManager_->FlushTouchEventsEnd(touchPoints);
            }
            eventManager_->DispatchTouchEvent(*iter);
        }
        idToTouchPoints_ = std::move(idToTouchPoints);
    }
}

void PipelineContext::OnMouseEvent(const MouseEvent& event, const RefPtr<FrameNode>& node)
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

    auto manager = GetDragDropManager();
    CHECK_NULL_VOID(manager);

    if (event.button == MouseButton::RIGHT_BUTTON &&
        (event.action == MouseAction::PRESS || event.action == MouseAction::PULL_UP)) {
        manager->SetIsDragCancel(true);
    } else {
        manager->SetIsDragCancel(false);
    }

    auto container = Container::Current();
    if ((event.action == MouseAction::RELEASE || event.action == MouseAction::PRESS ||
            event.action == MouseAction::MOVE) &&
        (event.button == MouseButton::LEFT_BUTTON || event.pressedButtons == MOUSE_PRESS_LEFT)) {
        auto touchPoint = event.CreateTouchPoint();
        if (event.pullAction == MouseAction::PULL_MOVE) {
            touchPoint.pullType = TouchType::PULL_MOVE;
        }
        OnTouchEvent(touchPoint, node);
    } else {
        auto touchPoint = event.CreateTouchPoint();
        auto scalePoint = touchPoint.CreateScalePoint(GetViewScale());
        auto rootOffset = GetRootRect().GetOffset();
        eventManager_->HandleGlobalEventNG(scalePoint, selectOverlayManager_, rootOffset);
    }

    CHECK_NULL_VOID(node);
    auto scaleEvent = event.CreateScaleEvent(viewScale_);
    TouchRestrict touchRestrict { TouchRestrict::NONE };
    touchRestrict.sourceType = event.sourceType;
    touchRestrict.hitTestType = SourceType::MOUSE;
    eventManager_->MouseTest(scaleEvent, node, touchRestrict);
    eventManager_->DispatchMouseEventNG(scaleEvent);
    eventManager_->DispatchMouseHoverEventNG(scaleEvent);
    eventManager_->DispatchMouseHoverAnimationNG(scaleEvent);
    accessibilityManagerNG_->HandleAccessibilityHoverEvent(node, scaleEvent);
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

bool PipelineContext::ChangeMouseStyle(int32_t nodeId, MouseFormat format, int32_t windowId)
{
    auto window = GetWindow();
    if (window && window->IsUserSetCursor()) {
        return false;
    }
    if (mouseStyleNodeId_ != nodeId) {
        return false;
    }
    auto mouseStyle = MouseStyle::CreateMouseStyle();
    CHECK_NULL_RETURN(mouseStyle, false);
    if (windowId) {
        return mouseStyle->ChangePointerStyle(windowId, format);
    }
    return mouseStyle->ChangePointerStyle(GetWindowId(), format);
}

bool PipelineContext::TriggerKeyEventDispatch(const KeyEvent& event)
{
    auto curFocusView = focusManager_ ? focusManager_->GetLastFocusView().Upgrade() : nullptr;
    auto curEntryFocusView = curFocusView ? curFocusView->GetEntryFocusView() : nullptr;
    auto curEntryFocusViewFrame = curEntryFocusView ? curEntryFocusView->GetFrameNode() : nullptr;
    if (event.isPreIme) {
        return eventManager_->DispatchKeyEventNG(event, curEntryFocusViewFrame);
    }

    auto isKeyTabDown = event.action == KeyAction::DOWN && event.IsKey({ KeyCode::KEY_TAB });
    auto isViewRootScopeFocused = curFocusView ? curFocusView->GetIsViewRootScopeFocused() : true;
    isTabJustTriggerOnKeyEvent_ = false;
    if (isKeyTabDown && isViewRootScopeFocused && curFocusView) {
        // Current focused on the view root scope. Tab key used to extend focus.
        // If return true. This tab key will just trigger onKeyEvent process.
        isTabJustTriggerOnKeyEvent_ = curFocusView->TriggerFocusMove();
    }

    // Tab key set focus state from inactive to active.
    // If return true. This tab key will just trigger onKeyEvent process.
    bool isHandleFocusActive = isKeyTabDown && SetIsFocusActive(true);
    isTabJustTriggerOnKeyEvent_ = isTabJustTriggerOnKeyEvent_ || isHandleFocusActive;
    if (eventManager_->DispatchTabIndexEventNG(event, curEntryFocusViewFrame)) {
        return true;
    }
    return eventManager_->DispatchKeyEventNG(event, curEntryFocusViewFrame);
}

bool PipelineContext::OnKeyEvent(const KeyEvent& event)
{
    CHECK_NULL_RETURN(eventManager_, false);
    eventManager_->SetPressedKeyCodes(event.pressedCodes);

    // onKeyPreIme
    if (event.isPreIme) {
        if (TriggerKeyEventDispatch(event)) {
            return true;
        }
        return eventManager_->DispatchKeyboardShortcut(event);
    }

    // process drag cancel
    if (event.code == KeyCode::KEY_ESCAPE) {
        auto manager = GetDragDropManager();
        if (manager && manager->IsMsdpDragging()) {
            manager->SetIsDragCancel(true);
            manager->OnDragEnd(PointerEvent(0, 0), "");
            manager->SetIsDragCancel(false);
            return true;
        }
    }

    // OnKeyEvent
    if (TriggerKeyEventDispatch(event)) {
        return true;
    }

    // process exit overlay
    if (event.code == KeyCode::KEY_ESCAPE && event.action == KeyAction::DOWN) {
        CHECK_NULL_RETURN(overlayManager_, false);
        auto currentContainer = Container::Current();
        if (currentContainer->IsSubContainer() || currentContainer->IsDialogContainer()) {
            return overlayManager_->RemoveOverlayInSubwindow();
        } else {
            return overlayManager_->RemoveOverlay(false);
        }
    }
    return false;
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

void PipelineContext::AddDirtyRequestFocus(const RefPtr<FrameNode>& node)
{
    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(node);
    dirtyRequestFocusNode_ = WeakPtr<FrameNode>(node);
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

void PipelineContext::OnAxisEvent(const AxisEvent& event, const RefPtr<FrameNode>& node)
{
    auto scaleEvent = event.CreateScaleEvent(viewScale_);

    auto dragManager = GetDragDropManager();
    if (dragManager && !dragManager->IsDragged()) {
        if (event.action == AxisAction::BEGIN) {
            isBeforeDragHandleAxis_ = true;
            TouchRestrict touchRestrict { TouchRestrict::NONE };
            touchRestrict.sourceType = event.sourceType;
            touchRestrict.hitTestType = SourceType::TOUCH;
            eventManager_->TouchTest(scaleEvent, node, touchRestrict);
        }
        eventManager_->DispatchTouchEvent(scaleEvent);
    } else if (isBeforeDragHandleAxis_ && event.action == AxisAction::END) {
        eventManager_->DispatchTouchEvent(scaleEvent);
        isBeforeDragHandleAxis_ = false;
    }

    if (event.action == AxisAction::BEGIN || event.action == AxisAction::UPDATE) {
        eventManager_->AxisTest(scaleEvent, node);
        eventManager_->DispatchAxisEventNG(scaleEvent);
    }

    auto mouseEvent = ConvertAxisToMouse(event);
    OnMouseEvent(mouseEvent, node);
}

bool PipelineContext::HasDifferentDirectionGesture() const
{
    CHECK_NULL_RETURN(eventManager_, false);
    return eventManager_->HasDifferentDirectionGesture();
}

void PipelineContext::AddVisibleAreaChangeNode(const RefPtr<FrameNode>& node,
    const std::vector<double>& ratios, const VisibleRatioCallback& callback, bool isUserCallback)
{
    CHECK_NULL_VOID(node);
    VisibleCallbackInfo addInfo;
    addInfo.callback = callback;
    addInfo.isCurrentVisible = false;
    onVisibleAreaChangeNodeIds_.emplace(node->GetId());
    if (isUserCallback) {
        node->SetVisibleAreaUserCallback(ratios, addInfo);
    } else {
        node->SetVisibleAreaInnerCallback(ratios, addInfo);
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

void PipelineContext::AddFormVisibleChangeNode(const RefPtr<FrameNode>& node, const std::function<void(bool)>& callback)
{
    CHECK_NULL_VOID(node);
    onFormVisibleChangeNodeIds_.emplace(node->GetId());
    onFormVisibleChangeEvents_.insert_or_assign(node->GetId(), callback);
}

void PipelineContext::RemoveFormVisibleChangeNode(int32_t nodeId)
{
    onFormVisibleChangeNodeIds_.erase(nodeId);
    auto iter = onFormVisibleChangeEvents_.find(nodeId);
    if (iter != onFormVisibleChangeEvents_.end()) {
        onFormVisibleChangeEvents_.erase(iter);
    }
}

void PipelineContext::HandleFormVisibleChangeEvent(bool isVisible)
{
    for (auto& pair : onFormVisibleChangeEvents_) {
        if (pair.second) {
            pair.second(isVisible);
        }
    }
}

void PipelineContext::AddOnAreaChangeNode(int32_t nodeId)
{
    onAreaChangeNodeIds_.emplace(nodeId);
    isOnAreaChangeNodesCacheVaild_ = false;
}

void PipelineContext::RemoveOnAreaChangeNode(int32_t nodeId)
{
    onAreaChangeNodeIds_.erase(nodeId);
    isOnAreaChangeNodesCacheVaild_ = false;
}

void PipelineContext::HandleOnAreaChangeEvent(uint64_t nanoTimestamp)
{
    ACE_FUNCTION_TRACE();
    if (onAreaChangeNodeIds_.empty()) {
        return;
    }
    if (!isOnAreaChangeNodesCacheVaild_) {
        onAreaChangeNodesCache_ = FrameNode::GetNodesPtrById(onAreaChangeNodeIds_);
        isOnAreaChangeNodesCacheVaild_ = true;
    }
    for (auto && frameNode : onAreaChangeNodesCache_) {
        if (!frameNode) {
            continue;
        }
        frameNode->TriggerOnAreaChangeCallback(nanoTimestamp);
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
    if (focusOnNodeCallback_) {
        windowShow_ = true;
        TAG_LOGI(AceLogTag::ACE_KEYBOARD, "windowShow is OK.");
    }
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
    auto dragDropManager = GetDragDropManager();
    if (dragDropManager && dragDropManager->IsItemDragging()) {
        dragDropManager->CancelItemDrag();
    }
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
        TAG_LOGI(AceLogTag::ACE_FOCUS, "Window id: %{public}d lost focus.", windowId_);
        RestoreDefault();
        RootLostFocus(BlurReason::WINDOW_BLUR);
        NotifyPopupDismiss();
    } else {
        TAG_LOGI(AceLogTag::ACE_FOCUS, "Window id: %{public}d get focus.", windowId_);
        auto curFocusView = focusManager_ ? focusManager_->GetLastFocusView().Upgrade() : nullptr;
        auto curFocusViewHub = curFocusView ? curFocusView->GetFocusHub() : nullptr;
        if (!curFocusViewHub) {
            TAG_LOGW(AceLogTag::ACE_FOCUS, "Current focus view can not found!");
        } else if (curFocusView->GetIsViewHasFocused() && !curFocusViewHub->IsCurrentFocus()) {
            TAG_LOGI(AceLogTag::ACE_FOCUS, "Request focus on current focus view: %{public}s/%{public}d",
                curFocusView->GetFrameName().c_str(), curFocusView->GetFrameId());
            curFocusViewHub->RequestFocusImmediately();
        }
        if (focusWindowId_.has_value()) {
            if (curFocusView) {
                curFocusView->TriggerFocusMove();
            }
        }
        if (focusOnNodeCallback_) {
            windowFocus_ = true;
            TAG_LOGI(AceLogTag::ACE_KEYBOARD, "windowfocus focusOnNodeCallback_.");
        }
        RequestFrame();
    }
    FlushWindowFocusChangedCallback(isFocus);
}

void PipelineContext::ContainerModalUnFocus()
{
    if (windowModal_ != WindowModal::CONTAINER_MODAL) {
        return;
    }
    CHECK_NULL_VOID(rootNode_);
    auto containerNode = AceType::DynamicCast<FrameNode>(rootNode_->GetChildren().front());
    CHECK_NULL_VOID(containerNode);
    auto containerPattern = containerNode->GetPattern<ContainerModalPattern>();
    CHECK_NULL_VOID(containerPattern);
    containerPattern->OnWindowForceUnfocused();
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
    containerPattern->ShowTitle(isShow, hasDeco, needUpdate);
}

void PipelineContext::UpdateTitleInTargetPos(bool isShow, int32_t height)
{
    if (windowModal_ != WindowModal::CONTAINER_MODAL) {
        return;
    }
    CHECK_NULL_VOID(rootNode_);
    auto containerNode = AceType::DynamicCast<FrameNode>(rootNode_->GetChildren().front());
    CHECK_NULL_VOID(containerNode);
    auto containerPattern = containerNode->GetPattern<ContainerModalPatternEnhance>();
    CHECK_NULL_VOID(containerPattern);
    containerPattern->UpdateTitleInTargetPos(isShow, height);
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

void PipelineContext::FlushReload(const ConfigurationChange& configurationChange)
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
    accessibilityManagerNG_.Reset();
    stageManager_.Reset();
    overlayManager_.Reset();
    sharedTransitionManager_.Reset();
    dragDropManager_.Reset();
    focusManager_.Reset();
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
    dirtyRequestFocusNode_.Reset();
    if (textFieldManager_->GetImeShow()) {
        FocusHub::CloseKeyboard();
    }
#ifdef WINDOW_SCENE_SUPPORTED
    uiExtensionManager_.Reset();
#endif
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

void PipelineContext::AddNavigationStateCallback(
    int32_t pageId, int32_t nodeId, const std::function<void()>& callback, bool isOnShow)
{
    CHECK_RUN_ON(UI);
    if (isOnShow) {
        auto it = pageIdOnShowMap_.find(pageId);
        if (it != pageIdOnShowMap_.end()) {
            it->second.push_back({nodeId, callback});
        } else {
            std::list<std::pair<int32_t, std::function<void()>>> callbacks;
            callbacks.push_back({nodeId, callback});
            pageIdOnShowMap_[pageId] = std::move(callbacks);
        }
    } else {
        auto it = pageIdOnHideMap_.find(pageId);
        if (it != pageIdOnHideMap_.end()) {
            it->second.push_back({nodeId, callback});
        } else {
            std::list<std::pair<int32_t, std::function<void()>>> callbacks;
            callbacks.push_back({nodeId, callback});
            pageIdOnHideMap_[pageId] = std::move(callbacks);
        }
    }
}

void PipelineContext::RemoveNavigationStateCallback(int32_t pageId, int32_t nodeId)
{
    CHECK_RUN_ON(UI);
    auto it = pageIdOnShowMap_.find(pageId);
    if (it != pageIdOnShowMap_.end() && !it->second.empty()) {
        for (auto iter = it->second.begin(); iter != it->second.end();) {
            if (iter->first == nodeId) {
                iter = it->second.erase(iter);
            } else {
                iter++;
            }
        }
    }
    it = pageIdOnHideMap_.find(pageId);
    if (it != pageIdOnHideMap_.end() && !it->second.empty()) {
        for (auto iter = it->second.begin(); iter != it->second.end();) {
            if (iter->first == nodeId) {
                iter = it->second.erase(iter);
            } else {
                iter++;
            }
        }
    }
}

void PipelineContext::FirePageChanged(int32_t pageId, bool isOnShow)
{
    CHECK_RUN_ON(UI);
    if (isOnShow) {
        auto it = pageIdOnShowMap_.find(pageId);
        if (it != pageIdOnShowMap_.end() && !it->second.empty()) {
            for (auto [nodeId, callback] : it->second) {
                callback();
            }
        }
    } else {
        auto it = pageIdOnHideMap_.find(pageId);
        if (it != pageIdOnHideMap_.end() && !it->second.empty()) {
            for (auto [nodeId, callback] : it->second) {
                callback();
            }
        }
    }
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

void PipelineContext::OnDragEvent(const PointerEvent& pointerEvent, DragEventAction action)
{
    auto manager = GetDragDropManager();
    CHECK_NULL_VOID(manager);
    std::string extraInfo;
    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        if (!manager->IsDragged() && manager->IsWindowConsumed()) {
            manager->SetIsWindowConsumed(false);
            return;
        }
    }
    if (action == DragEventAction::DRAG_EVENT_START_FOR_CONTROLLER) {
        manager->RequireSummary();
        manager->OnDragStart(pointerEvent.GetPoint());
        return;
    }
    if (action == DragEventAction::DRAG_EVENT_OUT) {
        manager->OnDragMoveOut(pointerEvent);
        manager->ClearSummary();
        manager->ClearExtraInfo();
        manager->SetDragCursorStyleCore(DragCursorStyleCore::DEFAULT);
        return;
    }

    if (action == DragEventAction::DRAG_EVENT_START) {
        manager->RequireSummary();
        manager->SetDragCursorStyleCore(DragCursorStyleCore::DEFAULT);
        TAG_LOGI(AceLogTag::ACE_DRAG, "start drag, current windowId is %{public}d", container->GetWindowId());
    }
    extraInfo = manager->GetExtraInfo();
    if (action == DragEventAction::DRAG_EVENT_END) {
        manager->OnDragEnd(pointerEvent, extraInfo);
        return;
    }
    if (action == DragEventAction::DRAG_EVENT_MOVE) {
        manager->DoDragMoveAnimate(pointerEvent);
    }
    manager->OnDragMove(pointerEvent, extraInfo);
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
    if (deadline == 0 || isWindowAnimation_) {
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
    if (GetSysTimestamp() < deadline) {
        ElementRegister::GetInstance()->CallJSCleanUpIdleTaskFunc();
    }
}

void PipelineContext::Finish(bool /* autoFinish */) const
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

void PipelineContext::AddPersistAfterLayoutTask(std::function<void()>&& task)
{
    taskScheduler_->AddPersistAfterLayoutTask(std::move(task));
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

void PipelineContext::AddIsFocusActiveUpdateEvent(
    const RefPtr<FrameNode>& node, const std::function<void(bool)>& eventCallback)
{
    CHECK_NULL_VOID(node);
    isFocusActiveUpdateEvents_.insert_or_assign(node->GetId(), eventCallback);
}

void PipelineContext::RemoveIsFocusActiveUpdateEvent(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_VOID(node);
    auto iter = isFocusActiveUpdateEvents_.find(node->GetId());
    if (iter != isFocusActiveUpdateEvents_.end()) {
        isFocusActiveUpdateEvents_.erase(iter);
    }
}

std::shared_ptr<NavigationController> PipelineContext::GetNavigationController(const std::string& id)
{
    std::lock_guard lock(navigationMutex_);
    auto iter = navigationNodes_.find(id);
    if (iter == navigationNodes_.end()) {
        return nullptr;
    }

    auto navigationGroupNode = iter->second.Upgrade();
    CHECK_NULL_RETURN(navigationGroupNode, nullptr);

    auto navigationPattern = navigationGroupNode->GetPattern<NavigationPattern>();
    CHECK_NULL_RETURN(navigationPattern, nullptr);
    return navigationPattern->GetNavigationController();
}

void PipelineContext::AddOrReplaceNavigationNode(const std::string& id, const WeakPtr<FrameNode>& node)
{
    std::lock_guard lock(navigationMutex_);
    auto frameNode = node.Upgrade();
    CHECK_NULL_VOID(frameNode);
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_VOID(navigationGroupNode);
    auto oldId = navigationGroupNode->GetCurId();
    if (!oldId.empty() && navigationNodes_.find(oldId) != navigationNodes_.end()) {
        navigationNodes_.erase(oldId);
    }

    if (!id.empty()) {
        navigationNodes_[id] = node;
    }
}

void PipelineContext::DeleteNavigationNode(const std::string& id)
{
    std::lock_guard lock(navigationMutex_);
    if (!id.empty() && navigationNodes_.find(id) != navigationNodes_.end()) {
        navigationNodes_.erase(id);
    }
}

void PipelineContext::SetJSViewActive(bool active, WeakPtr<CustomNode> custom)
{
    taskScheduler_->SetJSViewActive(active, custom);
}

std::string PipelineContext::GetCurrentExtraInfo()
{
    auto node = activeNode_.Upgrade();
    return node ? node->GetCurrentCustomNodeInfo() : std::string();
}

void PipelineContext::SetCursor(int32_t cursorValue)
{
    if (cursorValue >= 0 && cursorValue <= static_cast<int32_t>(MouseFormat::RUNNING)) {
        auto window = GetWindow();
        CHECK_NULL_VOID(window);
        auto mouseStyle = MouseStyle::CreateMouseStyle();
        CHECK_NULL_VOID(mouseStyle);
        auto cursor = static_cast<MouseFormat>(cursorValue);
        window->SetCursor(cursor);
        window->SetUserSetCursor(true);
        mouseStyle->ChangePointerStyle(GetWindowId(), cursor);
    }
}

void PipelineContext::RestoreDefault()
{
    auto window = GetWindow();
    CHECK_NULL_VOID(window);
    auto mouseStyle = MouseStyle::CreateMouseStyle();
    CHECK_NULL_VOID(mouseStyle);
    window->SetCursor(MouseFormat::DEFAULT);
    window->SetUserSetCursor(false);
    mouseStyle->ChangePointerStyle(GetWindowId(), MouseFormat::DEFAULT);
}

void PipelineContext::OpenFrontendAnimation(
    const AnimationOption& option, const RefPtr<Curve>& curve, const std::function<void()>& finishCallback)
{
    // push false to show we already open a animation closure.
    pendingFrontendAnimation_.push(false);

    // flush ui tasks before open animation closure.
    if (!isReloading_ && !IsLayouting()) {
        FlushUITasks();
    }
    auto wrapFinishCallback = GetWrappedAnimationCallback(finishCallback);
    if (IsFormRender()) {
        SetIsFormAnimation(true);
        if (!IsFormAnimationFinishCallback()) {
            SetFormAnimationStartTime(GetMicroTickCount());
        }
    }
    AnimationUtils::OpenImplicitAnimation(option, curve, wrapFinishCallback);
}

void PipelineContext::CloseFrontendAnimation()
{
    if (pendingFrontendAnimation_.empty()) {
        return;
    }

    if (pendingFrontendAnimation_.top()) {
        if (!isReloading_ && !IsLayouting()) {
            FlushUITasks();
        } else if (IsLayouting()) {
            TAG_LOGW(AceLogTag::ACE_ANIMATION,
                "IsLayouting, CloseFrontendAnimation has tasks not flushed, maybe some layout animation not generated");
        }
    }
    if (!pendingFrontendAnimation_.empty()) {
        pendingFrontendAnimation_.pop();
    }
    AnimationUtils::CloseImplicitAnimation();
}

bool PipelineContext::IsDragging() const
{
    if (!dragDropManager_) {
        return false;
    }
    bool isDragging = dragDropManager_->IsDragging();
    isDragging = (isDragging || dragDropManager_->IsMsdpDragging());
    return isDragging;
}

void PipelineContext::SetIsDragging(bool isDragging)
{
    if (!eventManager_) {
        return;
    }
    eventManager_->SetIsDragging(isDragging);
}

void PipelineContext::ResetDragging()
{
    CHECK_NULL_VOID(dragDropManager_);
    dragDropManager_->ResetDragging();
}

void PipelineContext::SetContainerModalTitleVisible(bool customTitleSettedShow, bool floatingTitleSettedShow)
{
    if (windowModal_ != WindowModal::CONTAINER_MODAL) {
        return;
    }
    CHECK_NULL_VOID(rootNode_);
    auto containerNode = AceType::DynamicCast<FrameNode>(rootNode_->GetFirstChild());
    CHECK_NULL_VOID(containerNode);
    auto containerPattern = containerNode->GetPattern<ContainerModalPattern>();
    CHECK_NULL_VOID(containerPattern);
    containerPattern->SetContainerModalTitleVisible(customTitleSettedShow, floatingTitleSettedShow);
}

void PipelineContext::SetContainerModalTitleHeight(int32_t height)
{
    if (windowModal_ != WindowModal::CONTAINER_MODAL) {
        return;
    }
    CHECK_NULL_VOID(rootNode_);
    auto containerNode = AceType::DynamicCast<FrameNode>(rootNode_->GetFirstChild());
    CHECK_NULL_VOID(containerNode);
    auto containerPattern = containerNode->GetPattern<ContainerModalPattern>();
    CHECK_NULL_VOID(containerPattern);
    containerPattern->SetContainerModalTitleHeight(height);
}

int32_t PipelineContext::GetContainerModalTitleHeight()
{
    if (windowModal_ != WindowModal::CONTAINER_MODAL) {
        return -1;
    }
    CHECK_NULL_RETURN(rootNode_, -1);
    auto containerNode = AceType::DynamicCast<FrameNode>(rootNode_->GetFirstChild());
    CHECK_NULL_RETURN(containerNode, -1);
    auto containerPattern = containerNode->GetPattern<ContainerModalPattern>();
    CHECK_NULL_RETURN(containerPattern, -1);
    return containerPattern->GetContainerModalTitleHeight();
}

bool PipelineContext::GetContainerModalButtonsRect(RectF& containerModal, RectF& buttons)
{
    if (windowModal_ != WindowModal::CONTAINER_MODAL) {
        return false;
    }
    CHECK_NULL_RETURN(rootNode_, false);
    auto containerNode = AceType::DynamicCast<FrameNode>(rootNode_->GetFirstChild());
    CHECK_NULL_RETURN(containerNode, false);
    auto containerPattern = containerNode->GetPattern<ContainerModalPattern>();
    CHECK_NULL_RETURN(containerPattern, false);
    return containerPattern->GetContainerModalButtonsRect(containerModal, buttons);
}

void PipelineContext::SubscribeContainerModalButtonsRectChange(
    std::function<void(RectF& containerModal, RectF& buttons)>&& callback)
{
    if (windowModal_ != WindowModal::CONTAINER_MODAL) {
        return;
    }
    CHECK_NULL_VOID(rootNode_);
    auto containerNode = AceType::DynamicCast<FrameNode>(rootNode_->GetFirstChild());
    CHECK_NULL_VOID(containerNode);
    auto containerPattern = containerNode->GetPattern<ContainerModalPattern>();
    CHECK_NULL_VOID(containerPattern);
    containerPattern->SubscribeContainerModalButtonsRectChange(std::move(callback));
}

const RefPtr<PostEventManager>& PipelineContext::GetPostEventManager()
{
    return postEventManager_;
}

const SerializedGesture& PipelineContext::GetSerializedGesture() const
{
    return serializedGesture_;
}

bool PipelineContext::PrintVsyncInfoIfNeed() const
{
    if (dumpFrameInfos_.empty()) {
        return false;
    }
    auto lastFrameInfo = dumpFrameInfos_.back();
    const uint64_t timeout = 1000000000; // unit is ns, 1s
    if (lastFrameInfo.frameRecvTime_ < window_->GetLastRequestVsyncTime() &&
        GetSysTimestamp() - window_->GetLastRequestVsyncTime() >= timeout) {
        LOGW("lastRequestVsyncTime is %{public}" PRIu64 ", now time is %{public}" PRId64
             ", timeout, window foreground:%{public}d, lastReceiveVsync info:%{public}s",
            window_->GetLastRequestVsyncTime(), GetSysTimestamp(), onShow_, lastFrameInfo.GetTimeInfo().c_str());
        return true;
    }
    return false;
}

void PipelineContext::AddSyncGeometryNodeTask(std::function<void()>&& task)
{
    taskScheduler_->AddSyncGeometryNodeTask(std::move(task));
}

void PipelineContext::FlushSyncGeometryNodeTasks()
{
    taskScheduler_->FlushSyncGeometryNodeTasks();
}

void PipelineContext::SetUIExtensionImeShow(bool imeShow)
{
    textFieldManager_->SetUIExtensionImeShow(imeShow);
}
} // namespace OHOS::Ace::NG
