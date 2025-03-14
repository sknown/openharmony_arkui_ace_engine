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

#include "core/components_ng/manager/post_event/post_event_manager.h"

#include "base/log/log_wrapper.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

bool PostEventManager::PostEvent(const RefPtr<NG::UINode>& uiNode, TouchEvent& touchEvent)
{
    if (!CheckPointValidity(touchEvent)) {
        return false;
    }
    CHECK_NULL_RETURN(uiNode, false);
    touchEvent.postEventNodeId = uiNode->GetId();
    auto result = false;
    switch (touchEvent.type) {
        case TouchType::DOWN:
            result = PostDownEvent(uiNode, touchEvent);
            break;
        case TouchType::MOVE:
            result = PostMoveEvent(uiNode, touchEvent);
            break;
        case TouchType::UP:
        case TouchType::CANCEL:
            result = PostUpEvent(uiNode, touchEvent);
            break;
        default:
            TAG_LOGE(AceLogTag::ACE_GESTURE, "dispatchEvent touchEvent type unkown");
            break;
    }
    return result;
}

bool PostEventManager::PostDownEvent(const RefPtr<NG::UINode>& targetNode, const TouchEvent& touchEvent)
{
    CHECK_NULL_RETURN(targetNode, false);

    for (const auto& iter : postEventAction_) {
        if (iter.targetNode == targetNode && iter.touchEvent.type == TouchType::DOWN &&
            iter.touchEvent.id == touchEvent.id) {
            auto lastEventMap = lastEventMap_;
            auto lastItem = lastEventMap.find(touchEvent.id);
            if (lastItem != lastEventMap.end()) {
                auto event = lastItem->second.touchEvent;
                event.type = TouchType::CANCEL;
                PostUpEvent(lastItem->second.targetNode, event);
                break;
            }
            return false;
        }
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, false);
    auto eventManager = pipelineContext->GetEventManager();
    CHECK_NULL_RETURN(eventManager, false);
    auto scalePoint = touchEvent.CreateScalePoint(pipelineContext->GetViewScale());
    TouchRestrict touchRestrict { TouchRestrict::NONE };
    touchRestrict.sourceType = touchEvent.sourceType;
    touchRestrict.touchEvent = touchEvent;
    touchRestrict.inputEventType = InputEventType::TOUCH_SCREEN;
    auto result = eventManager->PostEventTouchTest(scalePoint, targetNode, touchRestrict);
    if (!result) {
        return false;
    }
    HandlePostEvent(targetNode, touchEvent);
    return true;
}

bool PostEventManager::PostMoveEvent(const RefPtr<NG::UINode>& targetNode, const TouchEvent& touchEvent)
{
    CHECK_NULL_RETURN(targetNode, false);

    if (!HaveReceiveDownEvent(targetNode, touchEvent.id) || HaveReceiveUpOrCancelEvent(targetNode, touchEvent.id)) {
        return false;
    }

    HandlePostEvent(targetNode, touchEvent);
    return true;
}

bool PostEventManager::PostUpEvent(const RefPtr<NG::UINode>& targetNode, const TouchEvent& touchEvent)
{
    CHECK_NULL_RETURN(targetNode, false);

    if (!HaveReceiveDownEvent(targetNode, touchEvent.id) || HaveReceiveUpOrCancelEvent(targetNode, touchEvent.id)) {
        return false;
    }

    HandlePostEvent(targetNode, touchEvent);
    return true;
}

void PostEventManager::HandlePostEvent(const RefPtr<NG::UINode>& targetNode, const TouchEvent& touchEvent)
{
    // push dispatchAction and store
    PostEventAction postEventAction;
    postEventAction.targetNode = targetNode;
    postEventAction.touchEvent = touchEvent;
    lastEventMap_[touchEvent.id] = postEventAction;
    postEventAction_.push_back(postEventAction);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto eventManager = pipelineContext->GetEventManager();
    eventManager->PostEventFlushTouchEventEnd(touchEvent);
    eventManager->PostEventDispatchTouchEvent(touchEvent);
    // when receive UP event, clear DispatchAction which is same targetNode and same id
    CheckAndClearPostEventAction(targetNode, touchEvent.id);
}

void PostEventManager::CheckAndClearPostEventAction(const RefPtr<NG::UINode>& targetNode, int32_t id)
{
    bool receiveDown = false;
    bool receiveUp = false;
    for (const auto& iter : postEventAction_) {
        if (iter.targetNode == targetNode && iter.touchEvent.id == id) {
            if (iter.touchEvent.type == TouchType::DOWN) {
                receiveDown = true;
            } else if ((iter.touchEvent.type == TouchType::UP || iter.touchEvent.type == TouchType::CANCEL) &&
                       receiveDown) {
                receiveUp = true;
            }
        }
    }
    if (receiveUp) {
        for (auto iter = postEventAction_.begin(); iter != postEventAction_.end();) {
            if (iter->targetNode == targetNode && iter->touchEvent.id == id) {
                iter = postEventAction_.erase(iter);
            } else {
                ++iter;
            }
        }
        lastEventMap_.erase(id);
    }
}

bool PostEventManager::HaveReceiveDownEvent(const RefPtr<NG::UINode>& targetNode, int32_t id)
{
    return std::any_of(postEventAction_.begin(), postEventAction_.end(), [targetNode, id](const auto& actionItem) {
        return actionItem.targetNode == targetNode && actionItem.touchEvent.type == TouchType::DOWN &&
               actionItem.touchEvent.id == id;
    });
}

bool PostEventManager::HaveReceiveUpOrCancelEvent(const RefPtr<NG::UINode>& targetNode, int32_t id)
{
    return std::any_of(postEventAction_.begin(), postEventAction_.end(), [targetNode, id](const auto& actionItem) {
        return actionItem.targetNode == targetNode &&
               (actionItem.touchEvent.type == TouchType::UP || actionItem.touchEvent.type == TouchType::CANCEL) &&
               actionItem.touchEvent.id == id;
    });
}

bool PostEventManager::CheckPointValidity(const TouchEvent& touchEvent)
{
    return !std::any_of(postEventAction_.begin(), postEventAction_.end(), [touchEvent](const auto& actionItem) {
        return actionItem.touchEvent.id == touchEvent.id && actionItem.touchEvent.time == touchEvent.time;
    });
}
} // namespace OHOS::Ace::NG
