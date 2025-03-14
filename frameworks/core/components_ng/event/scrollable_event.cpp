/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/event/scrollable_event.h"

#include "base/geometry/offset.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/components_ng/pattern/list/list_pattern.h"

namespace OHOS::Ace::NG {

ScrollableActuator::ScrollableActuator(const WeakPtr<GestureEventHub>& gestureEventHub)
    : gestureEventHub_(gestureEventHub)
{}

void ScrollableActuator::AddScrollEdgeEffect(const Axis& axis, RefPtr<ScrollEdgeEffect>& effect)
{
    CHECK_NULL_VOID(effect);
    auto scrollable = scrollableEvents_[axis];
    CHECK_NULL_VOID(scrollable);
    effect->SetScrollable(scrollable->GetScrollable());
    effect->InitialEdgeEffect();
    scrollEffects_[axis] = effect;
}

bool ScrollableActuator::RemoveScrollEdgeEffect(const RefPtr<ScrollEdgeEffect>& effect)
{
    CHECK_NULL_RETURN(effect, false);
    for (auto iter = scrollEffects_.begin(); iter != scrollEffects_.end(); ++iter) {
        if (effect == iter->second) {
            scrollEffects_.erase(iter);
            return true;
        }
    }
    return false;
}

void ScrollableActuator::CollectTouchTarget(const OffsetF& coordinateOffset, const TouchRestrict& touchRestrict,
    const GetEventTargetImpl& getEventTargetImpl, TouchTestResult& result, const PointF& localPoint,
    const RefPtr<FrameNode>& frameNode, const RefPtr<TargetComponent>& targetComponent,
    TouchTestResult& responseLinkResult)
{
    for (const auto& [axis, event] : scrollableEvents_) {
        if (!event) {
            continue;
        }
        if (event->GetEnabled()) {
            if (event->InBarRegion(localPoint, touchRestrict.sourceType)) {
                event->BarCollectTouchTarget(
                    coordinateOffset, getEventTargetImpl, result, frameNode, targetComponent, responseLinkResult);
            } else if (event->InBarRectRegion(localPoint, touchRestrict.sourceType)) {
                event->BarCollectLongPressTarget(
                    coordinateOffset, getEventTargetImpl, result, frameNode, targetComponent, responseLinkResult);
            } else if (event->GetScrollable()) {
                const auto& scrollable = event->GetScrollable();
                scrollable->SetGetEventTargetImpl(getEventTargetImpl);
                scrollable->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
                scrollable->OnCollectTouchTarget(result, frameNode, targetComponent, responseLinkResult);
            }
        }
        bool clickJudge = event->ClickJudge(localPoint);
        if (event->GetEnabled() || clickJudge) {
            if (!clickRecognizer_) {
                clickRecognizer_ = MakeRefPtr<ClickRecognizer>();
            }
            bool isHitTestBlock = event->IsHitTestBlock();
            clickRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
            clickRecognizer_->SetGetEventTargetImpl(getEventTargetImpl);
            clickRecognizer_->SetNodeId(frameNode->GetId());
            clickRecognizer_->AttachFrameNode(frameNode);
            clickRecognizer_->SetTargetComponent(targetComponent);
            clickRecognizer_->SetIsSystemGesture(true);
            clickRecognizer_->SetRecognizerType(GestureTypeName::TAP_GESTURE);
            clickRecognizer_->SetSysGestureJudge([isHitTestBlock, clickJudge](const RefPtr<GestureInfo>& gestureInfo,
                                                     const std::shared_ptr<BaseGestureEvent>&) -> GestureJudgeResult {
                TAG_LOGI(AceLogTag::ACE_SCROLLABLE,
                    "Scrollable GestureJudge:%{public}d, %{public}d", isHitTestBlock, clickJudge);
                return isHitTestBlock || clickJudge ? GestureJudgeResult::CONTINUE : GestureJudgeResult::REJECT;
            });
            clickRecognizer_->SetOnClick([weak = WeakClaim(RawPtr(frameNode))](const ClickInfo&) {
                auto frameNode = weak.Upgrade();
                CHECK_NULL_VOID(frameNode);
                auto pattern = frameNode->GetPattern<ListPattern>();
                CHECK_NULL_VOID(pattern);
                auto item = pattern->GetSwiperItem().Upgrade();
                CHECK_NULL_VOID(item);
                item->SwiperReset(true);
            });
            result.emplace_front(clickRecognizer_);
            responseLinkResult.emplace_back(clickRecognizer_);
        }
        break;
    }
}
} // namespace OHOS::Ace::NG
