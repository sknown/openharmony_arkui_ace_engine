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

#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "base/geometry/dimension.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_algorithm.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_property.h"
#include "core/components_ng/pattern/navigation/title_bar_layout_property.h"
#include "core/components_ng/pattern/navigation/title_bar_node.h"
#include "core/components_ng/pattern/navigation/title_bar_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
void BuildTitleAndSubtitle(
    const RefPtr<NavDestinationGroupNode>& navDestinationNode, const RefPtr<TitleBarNode>& titleBarNode)
{
    if (!navDestinationNode->GetTitle() && !navDestinationNode->GetSubtitle()) {
        return;
    }
    if (navDestinationNode->GetTitle() && navDestinationNode->HasTitleNodeOperation()) {
        if (navDestinationNode->GetTitleNodeOperationValue() != ChildNodeOperation::NONE) {
            if (navDestinationNode->GetTitleNodeOperationValue() == ChildNodeOperation::REPLACE) {
                navDestinationNode->RemoveChild(titleBarNode->GetTitle());
            }
            titleBarNode->SetTitle(navDestinationNode->GetTitle());
            titleBarNode->AddChild(titleBarNode->GetTitle());
        }
    }
    if (navDestinationNode->GetSubtitle() && navDestinationNode->HasSubtitleNodeOperation()) {
        if (navDestinationNode->GetSubtitleNodeOperationValue() == ChildNodeOperation::NONE) {
            return;
        }
        if (navDestinationNode->GetSubtitleNodeOperationValue() == ChildNodeOperation::REPLACE) {
            navDestinationNode->RemoveChild(titleBarNode->GetSubtitle());
        }
        titleBarNode->SetSubtitle(navDestinationNode->GetSubtitle());
        titleBarNode->AddChild(titleBarNode->GetSubtitle());
    }
    titleBarNode->MarkModifyDone();
}

void BuildTitleBar(const RefPtr<NavDestinationGroupNode>& navDestinationNode, const RefPtr<TitleBarNode>& titleBarNode,
    RefPtr<NavDestinationLayoutProperty>& navDestinationLayoutProperty)
{
    if (!navDestinationNode->GetTitleBarNode()) {
        return;
    }
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    BuildTitleAndSubtitle(navDestinationNode, titleBarNode);
    if (navDestinationLayoutProperty->GetHideTitleBar().value_or(false)) {
        titleBarLayoutProperty->UpdateVisibility(VisibleType::GONE);
        titleBarNode->MarkModifyDone();
    } else {
        titleBarLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
        titleBarNode->MarkModifyDone();
    }
}

void MountTitleBar(const RefPtr<NavDestinationGroupNode>& hostNode)
{
    if (!hostNode->GetTitle() && !hostNode->GetSubtitle()) {
        return;
    }
    auto navDestinationLayoutProperty = hostNode->GetLayoutProperty<NavDestinationLayoutProperty>();
    CHECK_NULL_VOID(navDestinationLayoutProperty);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    BuildTitleBar(hostNode, titleBarNode, navDestinationLayoutProperty);
}

} // namespace

void NavDestinationPattern::OnModifyDone()
{
    auto hostNode = AceType::DynamicCast<NavDestinationGroupNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    MountTitleBar(hostNode);
}

} // namespace OHOS::Ace::NG