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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_ACCESSIBILITY_ACCESSIBILITY_MANAGER_NG_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_ACCESSIBILITY_ACCESSIBILITY_MANAGER_NG_H

#include <vector>

#include "base/memory/ace_type.h"

#include "base/geometry/ng/point_t.h"
#include "core/event/ace_events.h"

namespace OHOS::Ace {
struct MouseEvent;
struct TouchEvent;

namespace NG {
class FrameNode;

enum class AccessibilityHoverEventType {
    ENTER,
    MOVE,
    EXIT
};

struct AccessibilityHoverState {
    SourceType source = SourceType::NONE;
    std::vector<WeakPtr<FrameNode>> nodesHovering;
    TimeStamp time;
    bool idle = true;
};

class AccessibilityManagerNG final: public AceType {
    DECLARE_ACE_TYPE(AccessibilityManagerNG, AceType);

public:
    void HandleAccessibilityHoverEvent(const RefPtr<FrameNode>& root, const MouseEvent& event);
    void HandleAccessibilityHoverEvent(const RefPtr<FrameNode>& root, const TouchEvent& event);
    void HoverTestDebug(const RefPtr<FrameNode>& root, const PointF& point,
        std::string& summary, std::string& detail) const;

private:
    /*
    * Compute components which are hovered in accessibility mode.
    * And send hover enter/exit events to accessibility framework;
    * param: {root} should be not-null.
    */
    void HandleAccessibilityHoverEventInner(
        const RefPtr<FrameNode>& root,
        const PointF& point,
        SourceType sourceType,
        AccessibilityHoverEventType eventType,
        TimeStamp time);

    void ResetHoverState();

    AccessibilityHoverState hoverState_;
};
} // namespace NG
} // namespace OHOS::Ace

#endif
