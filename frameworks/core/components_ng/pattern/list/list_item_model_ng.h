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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_ITEM_MODEL_NG_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_ITEM_MODEL_NG_H

#include <functional>

#include "base/utils/macros.h"
#include "core/components_ng/pattern/list/list_item_event_hub.h"
#include "core/components_ng/pattern/list/list_item_model.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT ListItemModelNG : public ListItemModel {
public:
    void Create(std::function<void(int32_t)>&& deepRenderFunc, V2::ListItemStyle listItemStyle) override;
    void Create() override;
    void SetBorderRadius(const Dimension& borderRadius) override {}
    void SetType(const std::string& type) override {}
    void SetIsLazyCreating(bool isLazy) override {}
    void SetSticky(V2::StickyMode stickyMode) override;
    void SetEditMode(uint32_t editMode) override;
    void SetSelectable(bool selectable) override;
    void SetSelected(bool selected) override;
    void SetSelectChangeEvent(std::function<void(bool)>&& changeEvent) override;
    void SetSwiperAction(std::function<void()>&& startAction, std::function<void()>&& endAction,
        OnOffsetChangeFunc&& onOffsetChangeFunc, V2::SwipeEdgeEffect edgeEffect) override;
    void SetSelectCallback(OnSelectFunc&& selectCallback) override;
    void SetOnDragStart(NG::OnDragStartFunc&& onDragStart) override {}
    void SetDeleteArea(std::function<void()>&& builderAction, OnDeleteEvent&& onDelete,
        OnEnterDeleteAreaEvent&& onEnterDeleteArea, OnExitDeleteAreaEvent&& onExitDeleteArea,
        OnStateChangedEvent&& onStateChangeEvent, const Dimension& length, bool isStartArea) override;
        
    static void SetDeleteArea(FrameNode* frameNode, FrameNode* footerNode, OnDeleteEvent&& onDelete,
        OnEnterDeleteAreaEvent&& onEnterDeleteArea, OnExitDeleteAreaEvent&& onExitDeleteArea,
        OnStateChangedEvent&& onStateChangeEvent, const Dimension& length, bool isStartArea);
    static void SetSwiperAction(FrameNode* frameNode, std::function<void()>&& startAction,
        std::function<void()>&& endAction, OnOffsetChangeFunc&& onOffsetChangeFunc,
        V2::SwipeEdgeEffect edgeEffect);
    static void SetSelected(FrameNode* frameNode, bool selected);
    static void SetSelectable(FrameNode* frameNode, bool selectable);
    static RefPtr<FrameNode> CreateFrameNode(int32_t nodeId);
    static void SetSelectCallback(FrameNode* frameNode, OnSelectFunc&& selectCallback);
private:
    void InstallSwiperCallBack(RefPtr<ListItemEventHub> eventHub,
                               OnDeleteEvent&& onDelete,
                               OnEnterDeleteAreaEvent&& onEnterDeleteArea,
                               OnExitDeleteAreaEvent&& onExitDeleteArea,
                               OnStateChangedEvent&& onStateChangeEvent,
                               bool isStartArea);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_ITEM_MODEL_NG_H
