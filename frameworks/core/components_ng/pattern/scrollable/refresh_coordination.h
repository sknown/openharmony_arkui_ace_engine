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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLLABLE_REFRESH_COORDINATION_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLLABLE_REFRESH_COORDINATION_H

#include <utility>

#include "base/memory/ace_type.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/scrollable/scrollable_coordination_event.h"

namespace OHOS::Ace::NG {
class RefreshCoordination : public AceType {
public:
    RefreshCoordination(RefPtr<FrameNode> scrollableNode)
    {
        scrollableNode_ = scrollableNode;
        refreshNode_ = FindRefreshNode();
        coordinationEvent_ = CreateCoordinationEvent();
    };
    ~RefreshCoordination() = default;
    void OnScrollStart() const;
    bool OnScroll(float offset) const;
    void OnScrollEnd(float speed) const;
    bool InCoordination()
    {
        return !!refreshNode_;
    }

private:
    RefPtr<FrameNode> FindRefreshNode() const;
    RefPtr<ScrollableCoordinationEvent> CreateCoordinationEvent();
    RefPtr<FrameNode> refreshNode_;
    RefPtr<FrameNode> scrollableNode_;
    RefPtr<ScrollableCoordinationEvent> coordinationEvent_;
};
} // namespace OHOS::Ace::NG
#endif