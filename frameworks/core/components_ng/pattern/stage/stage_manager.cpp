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

#include "core/components_ng/pattern/stage/stage_manager.h"

#include "base/geometry/ng/size_t.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/page/page_pattern.h"
#include "core/components_ng/pattern/stage/stage_pattern.h"
#include "core/components_ng/property/property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {
StageManager::StageManager(const RefPtr<FrameNode>& root) : rootNode_(root)
{
    stagePattern_ = DynamicCast<StagePattern>(rootNode_->GetPattern());
}

void StageManager::PushPage(const RefPtr<UINode>& node)
{
    if (!rootNode_) {
        LOGE("the root node is nullptr");
        return;
    }
    if (!node) {
        LOGE("page node is nullptr, maybe js file execute failed");
        return;
    }
    auto pageNode = FrameNode::CreateFrameNode(
        V2::PAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), MakeRefPtr<PagePattern>());
    node->MountToParent(pageNode);
    pageNode->MountToParent(rootNode_);
    // TODO: change page index
    stagePattern_->currentPageIndex_ = 0;
    // flush layout task.
    if (rootNode_->GetGeometryNode()->GetFrameSize() == SizeF(0.0f, 0.0f)) {
        // in first load case, wait for window size.
        LOGI("waiting for window size");
        return;
    }
    rootNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}
} // namespace OHOS::Ace::NG
