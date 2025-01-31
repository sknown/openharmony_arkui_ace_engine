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

#include "core/components_ng/pattern/window_scene/helper/window_scene_helper.h"

#include "input_manager.h"
#include "key_event.h"
#include "pointer_event.h"

#include "adapter/ohos/entrance/ace_view_ohos.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components_ng/pattern/window_scene/scene/system_window_scene.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/pattern/search/search_pattern.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "session/host/include/session.h"

#if not defined(ACE_UNITTEST)
#if defined(ENABLE_STANDARD_INPUT)
#include "input_method_controller.h"
#endif
#endif

namespace OHOS::Ace::NG {
RefPtr<UINode> WindowSceneHelper::FindWindowScene(const RefPtr<FrameNode>& targetNode)
{
    CHECK_NULL_RETURN(targetNode, nullptr);

    auto container = Container::Current();
    if (!container || !container->IsScenceBoardWindow() || !container->IsSceneBoardEnabled()) {
        TAG_LOGD(AceLogTag::ACE_KEYBOARD, "Container nullptr Or not ScenceBoardWindow.");
        return nullptr;
    }

    TAG_LOGD(AceLogTag::ACE_KEYBOARD, "FindWindowScene start.");
    auto parent = targetNode->GetParent();
    while (parent && parent->GetTag() != V2::WINDOW_SCENE_ETS_TAG) {
        parent = parent->GetParent();
    }
    CHECK_NULL_RETURN(parent, nullptr);
    TAG_LOGD(AceLogTag::ACE_KEYBOARD, "FindWindowScene successfully.");

    return parent;
}

sptr<Rosen::Session> GetCurSession(const RefPtr<FrameNode>& focusedFrameNode)
{
    auto sceneBoardWindowUINode = WindowSceneHelper::FindWindowScene(focusedFrameNode);
    if (sceneBoardWindowUINode == nullptr) {
        TAG_LOGD(AceLogTag::ACE_KEYBOARD, "FindWindowScene failed.");
        return nullptr;
    }

    auto windowSceneFrameNode = AceType::DynamicCast<FrameNode>(sceneBoardWindowUINode);
    if (windowSceneFrameNode == nullptr) {
        TAG_LOGD(AceLogTag::ACE_KEYBOARD, "WindowFrameNode to FrameNode failed.");
        return nullptr;
    }

    auto windowScenePattern = windowSceneFrameNode->GetPattern<SystemWindowScene>();
    if (windowScenePattern == nullptr) {
        TAG_LOGD(AceLogTag::ACE_KEYBOARD, "windowScenePattern is nullptr.");
        return nullptr;
    }

    return windowScenePattern->GetSession();
}

bool WindowSceneHelper::IsWindowScene(const RefPtr<FrameNode>& focusedFrameNode)
{
    auto window2patternSession = GetCurSession(focusedFrameNode);
    if (window2patternSession == nullptr) {
        TAG_LOGD(AceLogTag::ACE_KEYBOARD, "The session between window and pattern is nullptr.");
        return false;
    }

    return window2patternSession->GetSessionInfo().isSystem_;
}

int32_t WindowSceneHelper::GetFocusSystemWindowId(const RefPtr<FrameNode>& focusedFrameNode)
{
    int32_t focusSystemWindowId = 0;
    bool isWindowScene = IsWindowScene(focusedFrameNode);
    sptr<Rosen::Session> window2patternSession = GetCurSession(focusedFrameNode);
    if (window2patternSession == nullptr) {
        TAG_LOGD(AceLogTag::ACE_KEYBOARD, "The session between window and pattern is nullptr.");
        return focusSystemWindowId;
    }
    if (isWindowScene) {
        focusSystemWindowId = static_cast<int32_t>(window2patternSession->GetPersistentId());
        LOGI("Get systemWindowScene id( %{public}d ) successfully.", focusSystemWindowId);
    }

    return focusSystemWindowId;
}

int32_t WindowSceneHelper::GetWindowIdForWindowScene(const RefPtr<FrameNode>& windowSceneNode)
{
    int32_t windowId = 0;
    CHECK_NULL_RETURN(windowSceneNode, windowId);
    if (windowSceneNode->GetTag() != V2::WINDOW_SCENE_ETS_TAG) {
        return windowId;
    }
    auto windowScenePattern = windowSceneNode->GetPattern<SystemWindowScene>();
    CHECK_NULL_RETURN(windowScenePattern, windowId);

    auto window2patternSession = windowScenePattern->GetSession();
    CHECK_NULL_RETURN(window2patternSession, windowId);

    windowId = static_cast<int32_t>(window2patternSession->GetPersistentId());
    return windowId;
}

bool WindowSceneHelper::IsFocusWindowSceneCloseKeyboard(const RefPtr<FrameNode>& focusedFrameNode)
{
    sptr<Rosen::Session> window2patternSession = GetCurSession(focusedFrameNode);
    if (window2patternSession == nullptr) {
        TAG_LOGD(AceLogTag::ACE_KEYBOARD, "The session between window and pattern is nullptr.");
        return false;
    }

    return window2patternSession->GetSCBKeepKeyboardFlag();
}

void WindowSceneHelper::IsWindowSceneCloseKeyboard(const RefPtr<FrameNode>& frameNode)
{
#if defined (ENABLE_STANDARD_INPUT)
    // If focus pattern does not need softkeyboard, close it, in windowScene.
    auto curPattern = frameNode->GetPattern<NG::Pattern>();
    CHECK_NULL_VOID(curPattern);
    bool isNeedKeyBoard = curPattern->NeedSoftKeyboard();
    auto saveKeyboard = IsFocusWindowSceneCloseKeyboard(frameNode);
    TAG_LOGD(AceLogTag::ACE_KEYBOARD, "Keep/Need(%{public}d/%{public}d)", saveKeyboard, isNeedKeyBoard);
    if (!saveKeyboard && !isNeedKeyBoard) {
        TAG_LOGI(AceLogTag::ACE_KEYBOARD, "scbFrameNode(%{public}s/%{public}d) notNeedSoftKeyboard.",
            frameNode->GetTag().c_str(), frameNode->GetId());
        auto inputMethod = MiscServices::InputMethodController::GetInstance();
        if (inputMethod) {
            inputMethod->RequestHideInput();
            inputMethod->Close();
            TAG_LOGI(AceLogTag::ACE_KEYBOARD, "scbSoftKeyboard Closes Successfully.");
        }
    }
#endif
}

void WindowSceneHelper::IsCloseKeyboard(const RefPtr<FrameNode>& frameNode)
{
#if defined (ENABLE_STANDARD_INPUT)
    // If focus pattern does not need softkeyboard, close it, not in windowScene.
    auto curPattern = frameNode->GetPattern<NG::Pattern>();
    CHECK_NULL_VOID(curPattern);
    bool isNeedKeyBoard = curPattern->NeedSoftKeyboard();
    auto saveKeyboard = IsFocusWindowSceneCloseKeyboard(frameNode);
    TAG_LOGD(AceLogTag::ACE_KEYBOARD, "Keep/Need(%{public}d/%{public}d)", !saveKeyboard, !isNeedKeyBoard);
    if (!saveKeyboard && !isNeedKeyBoard) {
        TAG_LOGI(AceLogTag::ACE_KEYBOARD, "FrameNode(%{public}s/%{public}d) notNeedSoftKeyboard.",
            frameNode->GetTag().c_str(), frameNode->GetId());
        auto inputMethod = MiscServices::InputMethodController::GetInstance();
        if (inputMethod) {
            inputMethod->Close();
            TAG_LOGI(AceLogTag::ACE_KEYBOARD, "SoftKeyboard Closes Successfully.");
        }
    }
#endif
}

void CaculatePoint(const RefPtr<FrameNode>& node, const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent)
{
    CHECK_NULL_VOID(node);
    CHECK_NULL_VOID(pointerEvent);

    auto pointerId = pointerEvent->GetPointerId();
    auto renderContext = node->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto rect = renderContext->GetPaintRectWithoutTransform();
    MMI::PointerEvent::PointerItem item;
    if (pointerEvent->GetPointerItem(pointerId, item)) {
        PointF tmp(item.GetWindowX() + rect.GetX(), item.GetWindowY() + rect.GetY());
        renderContext->GetPointTransform(tmp);
        item.SetWindowX(static_cast<int32_t>(std::round(tmp.GetX())));
        item.SetWindowY(static_cast<int32_t>(std::round(tmp.GetY())));
        if (pointerEvent->GetSourceType() == OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN &&
            item.GetToolType() == OHOS::MMI::PointerEvent::TOOL_TYPE_PEN) {
            // CaculatePoint for double XY Position.
            PointF tmpPos(item.GetWindowXPos() + rect.GetX(), item.GetWindowYPos() + rect.GetY());
            renderContext->GetPointTransform(tmpPos);
            item.SetWindowXPos(tmpPos.GetX());
            item.SetWindowYPos(tmpPos.GetY());
        }
        pointerEvent->UpdatePointerItem(pointerId, item);
    }
}

void WindowSceneHelper::InjectPointerEvent(
    const std::string& targetNodeName, const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent)
{
    if (!pointerEvent) {
        TAG_LOGE(AceLogTag::ACE_INPUTTRACKING, "InjectPointerEvent pointerEvent is null return.");
        return;
    }
    if (targetNodeName.empty()) {
        MMI::InputManager::GetInstance()->MarkProcessed(
            pointerEvent->GetId(), pointerEvent->GetActionTime(), pointerEvent->IsMarkEnabled());
        TAG_LOGE(AceLogTag::ACE_INPUTTRACKING, "InjectPointerEvent eventId:%{public}d targetNodeName is null return.",
            pointerEvent->GetId());
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    if (!pipelineContext) {
        MMI::InputManager::GetInstance()->MarkProcessed(
            pointerEvent->GetId(), pointerEvent->GetActionTime(), pointerEvent->IsMarkEnabled());
        TAG_LOGE(AceLogTag::ACE_INPUTTRACKING, "InjectPointerEvent eventId:%{public}d pipelineContext is null return.",
            pointerEvent->GetId());
        return;
    }

    auto rootNode = pipelineContext->GetRootElement();
    if (!rootNode) {
        MMI::InputManager::GetInstance()->MarkProcessed(
            pointerEvent->GetId(), pointerEvent->GetActionTime(), pointerEvent->IsMarkEnabled());
        TAG_LOGE(AceLogTag::ACE_INPUTTRACKING, "InjectPointerEvent eventId:%{public}d rootNode is null return.",
            pointerEvent->GetId());
        return;
    }
    auto targetNode = FrameNode::FindChildByName(rootNode, targetNodeName);
    if (!targetNode && pointerEvent->GetPointerAction() != MMI::PointerEvent::POINTER_ACTION_MOVE) {
        TAG_LOGW(AceLogTag::ACE_INPUTTRACKING,
            "PointerEvent Process to inject, targetNode is null. targetNodeName:%{public}s", targetNodeName.c_str());
    }
    InjectPointerEvent(targetNode, pointerEvent);
}

void WindowSceneHelper::InjectPointerEvent(
    const RefPtr<FrameNode>& node, const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent)
{
    if (!pointerEvent) {
        TAG_LOGE(AceLogTag::ACE_INPUTTRACKING, "InjectPointerEvent pointerEvent is null return.");
        return;
    }
    if (!node) {
        MMI::InputManager::GetInstance()->MarkProcessed(
            pointerEvent->GetId(), pointerEvent->GetActionTime(), pointerEvent->IsMarkEnabled());
        TAG_LOGE(AceLogTag::ACE_INPUTTRACKING, "InjectPointerEvent eventId:%{public}d node is null return.",
            pointerEvent->GetId());
        return;
    }

    auto container = Container::Current();
    if (!container) {
        MMI::InputManager::GetInstance()->MarkProcessed(
            pointerEvent->GetId(), pointerEvent->GetActionTime(), pointerEvent->IsMarkEnabled());
        TAG_LOGE(AceLogTag::ACE_INPUTTRACKING, "InjectPointerEvent eventId:%{public}d container is null return.",
            pointerEvent->GetId());
        return;
    }
    CaculatePoint(node, pointerEvent);
    auto aceView = AceType::DynamicCast<OHOS::Ace::Platform::AceViewOhos>(container->GetAceView());
    if (!aceView) {
        MMI::InputManager::GetInstance()->MarkProcessed(
            pointerEvent->GetId(), pointerEvent->GetActionTime(), pointerEvent->IsMarkEnabled());
        TAG_LOGE(AceLogTag::ACE_INPUTTRACKING, "InjectPointerEvent eventId:%{public}d aceView is null return.",
            pointerEvent->GetId());
        return;
    }
    OHOS::Ace::Platform::AceViewOhos::DispatchTouchEvent(aceView, pointerEvent, node, nullptr, true);
}

bool WindowSceneHelper::InjectKeyEvent(const std::shared_ptr<OHOS::MMI::KeyEvent>& keyEvent, bool isPreIme)
{
    CHECK_NULL_RETURN(keyEvent, false);
    TAG_LOGI(AceLogTag::ACE_INPUTTRACKING,
        "KeyEvent Process to inject, eventInfo: id:%{public}d, "
        "keyEvent info: keyCode is %{public}d, "
        "keyAction is %{public}d, keyActionTime is %{public}" PRId64,
        keyEvent->GetId(), keyEvent->GetKeyCode(), keyEvent->GetKeyAction(), keyEvent->GetActionTime());

    auto container = Container::Current();
    CHECK_NULL_RETURN(container, false);
    auto aceView = AceType::DynamicCast<OHOS::Ace::Platform::AceViewOhos>(container->GetAceView());
    CHECK_NULL_RETURN(aceView, false);
    return OHOS::Ace::Platform::AceViewOhos::DispatchKeyEvent(aceView, keyEvent, isPreIme);
}
} // namespace OHOS::Ace::NG
