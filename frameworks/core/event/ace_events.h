/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_EVENT_ACE_EVENTS_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_EVENT_ACE_EVENTS_H

#include <chrono>
#include <cstdint>
#include <string>

#include "base/geometry/dimension_offset.h"
#include "base/geometry/dimension_rect.h"
#include "base/memory/type_info_base.h"
#include "base/utils/type_definition.h"

namespace OHOS::Ace {

enum class SourceType : int32_t {
    NONE = 0,
    MOUSE = 1,
    TOUCH = 2,
    TOUCH_PAD = 3,
    KEYBOARD = 4
};

enum class SourceTool : int32_t {
    UNKNOWN = 0,
    FINGER = 1,
    PEN = 2,
    RUBBER = 3,
    BRUSH = 4,
    PENCIL = 5,
    AIRBRUSH = 6,
    MOUSE = 7,
    LENS = 8,
};

struct EventTarget final {
    std::string id;
    std::string type;
    DimensionRect area;
    DimensionOffset origin;
};

class BaseEventInfo : public virtual TypeInfoBase {
    DECLARE_RELATIONSHIP_OF_CLASSES(BaseEventInfo, TypeInfoBase);

public:
    explicit BaseEventInfo(const std::string& type) : type_(type) {}
    ~BaseEventInfo() override = default;

    const std::string& GetType() const
    {
        return type_;
    }
    void SetType(const std::string& type)
    {
        type_ = type;
    }

    const TimeStamp& GetTimeStamp() const
    {
        return timeStamp_;
    }
    BaseEventInfo& SetTimeStamp(const TimeStamp& timeStamp)
    {
        timeStamp_ = timeStamp;
        return *this;
    }

    const EventTarget& GetTarget() const
    {
        return target_;
    }
    EventTarget& GetTargetWithModify()
    {
        return target_;
    }

    BaseEventInfo& SetTarget(const EventTarget& target)
    {
        target_ = target;
        return *this;
    }

    int64_t GetDeviceId() const
    {
        return deviceId_;
    }
    void SetDeviceId(int64_t deviceId)
    {
        deviceId_ = deviceId;
    }

    SourceType GetSourceDevice() const
    {
        return deviceType_;
    }
    void SetSourceDevice(SourceType deviceType)
    {
        deviceType_ = deviceType;
    }

    void SetForce(float force)
    {
        force_ = force;
    }
    float GetForce() const
    {
        return force_;
    }

    void SetTiltX(float tiltX)
    {
        tiltX_ = tiltX;
    }
    std::optional<float> GetTiltX() const
    {
        return tiltX_;
    }

    void SetTiltY(float tiltY)
    {
        tiltY_ = tiltY;
    }
    std::optional<float> GetTiltY() const
    {
        return tiltY_;
    }

    void SetSourceTool(SourceTool tool)
    {
        sourceTool_ = tool;
    }
    SourceTool GetSourceTool() const
    {
        return sourceTool_;
    }

    bool IsStopPropagation() const
    {
        return stopPropagation_;
    }
    void SetStopPropagation(bool stopPropagation)
    {
        stopPropagation_ = stopPropagation;
    }

protected:
    // Event type like onTouchDown, onClick and so on.
    std::string type_;
    // The origin event time stamp.
    TimeStamp timeStamp_;
    EventTarget target_;
    SourceType deviceType_ = SourceType::NONE;
    float force_ = 0.0f;
    std::optional<float> tiltX_;
    std::optional<float> tiltY_;
    SourceTool sourceTool_ = SourceTool::UNKNOWN;
    int64_t deviceId_ = 0;
    bool stopPropagation_ = false;
};

class PropagationEventInfo : public virtual TypeInfoBase {
    DECLARE_RELATIONSHIP_OF_CLASSES(PropagationEventInfo, TypeInfoBase);

public:
    bool IsStopPropagation() const
    {
        return stopPropagation_;
    }
    void SetStopPropagation(bool stopPropagation)
    {
        stopPropagation_ = stopPropagation;
    }

private:
    bool stopPropagation_ = false;
};

class EventToJSONStringAdapter : public virtual TypeInfoBase {
    DECLARE_RELATIONSHIP_OF_CLASSES(EventToJSONStringAdapter, TypeInfoBase);

public:
    EventToJSONStringAdapter() = default;
    ~EventToJSONStringAdapter() = default;

    virtual std::string ToJSONString() const = 0;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_EVENT_ACE_EVENTS_H
