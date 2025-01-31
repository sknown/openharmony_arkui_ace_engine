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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_SWIPER_EVENT_HUB_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_SWIPER_EVENT_HUB_H

#include <algorithm>
#include <memory>

#include "base/memory/ace_type.h"
#include "core/common/recorder/event_recorder.h"
#include "core/common/recorder/node_data_cache.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/swiper/swiper_model.h"

namespace OHOS::Ace::NG {

enum class Direction {
    PRE = 0,
    NEXT,
};
using ChangeIndicatorEvent = std::function<void()>;
using ChangeEvent = std::function<void(int32_t index)>;
using ChangeEventPtr = std::shared_ptr<ChangeEvent>;
using ChangeDoneEvent = std::function<void()>;

class SwiperEventHub : public EventHub {
    DECLARE_ACE_TYPE(SwiperEventHub, EventHub)

public:
    SwiperEventHub() = default;
    ~SwiperEventHub() override = default;

    /* Using shared_ptr to enable event modification without adding again */
    void AddOnChangeEvent(const ChangeEventPtr& changeEvent)
    {
        changeEvents_.emplace_back(changeEvent);
    }

    void SetIndicatorOnChange(ChangeIndicatorEvent&& changeEvent)
    {
        changeIndicatorEvent_ = std::move(changeEvent);
    }

    void SetChangeDoneEvent(ChangeDoneEvent&& changeDoneEvent)
    {
        changeDoneEvent_ = std::move(changeDoneEvent);
    }

    void AddAnimationStartEvent(const AnimationStartEventPtr& animationStartEvent)
    {
        animationStartEvents_.emplace_back(animationStartEvent);
    }

    void AddAnimationEndEvent(const AnimationEndEventPtr& animationEndEvent)
    {
        animationEndEvents_.emplace_back(animationEndEvent);
    }

    void SetGestureSwipeEvent(GestureSwipeEvent&& gestureSwipeEvent)
    {
        gestureSwipeEvent_ = std::move(gestureSwipeEvent);
    }

    void FireChangeDoneEvent(bool direction)
    {
        if (changeDoneEvent_) {
            if (direction) {
                direction_ = Direction::NEXT;
            } else {
                direction_ = Direction::PRE;
            }
            changeDoneEvent_();
        }
    }

    void FireChangeEvent(int32_t index) const
    {
        ACE_SCOPED_TRACE("Swiper FireChangeEvent, index: %d eventSize: %zu", index, changeEvents_.size());
        if (!changeEvents_.empty()) {
            std::for_each(
                changeEvents_.begin(), changeEvents_.end(), [index](const ChangeEventPtr& changeEvent) {
                    auto event = *changeEvent;
                    event(index);
                });
        }

        if (Recorder::EventRecorder::Get().IsComponentRecordEnable()) {
            Recorder::EventParamsBuilder builder;
            auto host = GetFrameNode();
            if (host) {
                auto id = host->GetInspectorIdValue("");
                builder.SetId(id).SetType(host->GetHostTag()).SetDescription(host->GetAutoEventParamValue(""));
                if (!id.empty()) {
                    Recorder::NodeDataCache::Get().PutInt(host, id, index);
                }
            }
            builder.SetIndex(index);
            Recorder::EventRecorder::Get().OnChange(std::move(builder));
        }
    }

    void FireIndicatorChangeEvent(int32_t index) const
    {
        if (changeIndicatorEvent_) {
            changeIndicatorEvent_();
        }
    }

    Direction GetDirection()
    {
        return direction_;
    }

    void FireAnimationStartEvent(int32_t index, int32_t targetIndex, const AnimationCallbackInfo& info) const
    {
        if (!animationStartEvents_.empty()) {
            std::for_each(animationStartEvents_.begin(), animationStartEvents_.end(),
                [index, targetIndex, info](const AnimationStartEventPtr& animationStartEvent) {
                    auto event = *animationStartEvent;
                    event(index, targetIndex, info);
                });
        }
    }

    void FireAnimationEndEvent(int32_t index, const AnimationCallbackInfo& info) const
    {
        if (!animationEndEvents_.empty()) {
            std::for_each(animationEndEvents_.begin(), animationEndEvents_.end(),
                [index, info](const AnimationEndEventPtr& animationEndEvent) {
                    auto event = *animationEndEvent;
                    event(index, info);
                });
        }
    }

    void FireAnimationEndOnForceEvent(int32_t index, const AnimationCallbackInfo& info) const
    {
        if (!animationEndEvents_.empty()) {
            auto context = GetFrameNode()->GetContext();
            CHECK_NULL_VOID(context);
            context->AddBuildFinishCallBack([this, index, info]() {
                std::for_each(animationEndEvents_.begin(), animationEndEvents_.end(),
                    [index, info](const AnimationEndEventPtr& animationEndEvent) {
                        auto event = *animationEndEvent;
                        event(index, info);
                    });
            });
        }
    }

    void FireGestureSwipeEvent(int32_t index, const AnimationCallbackInfo& info) const
    {
        if (gestureSwipeEvent_) {
            // gestureSwipeEvent_ may be overwrite in its invoke, so copy it first
            auto event = gestureSwipeEvent_;
            event(index, info);
        }
    }

private:
    Direction direction_;
    std::list<ChangeEventPtr> changeEvents_;
    ChangeDoneEvent changeDoneEvent_;
    ChangeIndicatorEvent changeIndicatorEvent_;
    std::list<AnimationStartEventPtr> animationStartEvents_;
    std::list<AnimationEndEventPtr> animationEndEvents_;
    GestureSwipeEvent gestureSwipeEvent_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_SWIPER_EVENT_HUB_H