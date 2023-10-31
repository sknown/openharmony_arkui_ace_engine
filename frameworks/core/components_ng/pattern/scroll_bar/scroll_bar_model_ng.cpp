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
#include "core/components_ng/pattern/scroll_bar/scroll_bar_model_ng.h"

#include "base/geometry/axis.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
const std::vector<Axis> AXIS = { Axis::VERTICAL, Axis::HORIZONTAL, Axis::NONE };
const std::vector<DisplayMode> DISPLAY_MODE = { DisplayMode::OFF, DisplayMode::AUTO, DisplayMode::ON };
}
RefPtr<ScrollProxy> ScrollBarModelNG::GetScrollBarProxy(const RefPtr<ScrollProxy>& scrollProxy)
{
    auto proxy = AceType::DynamicCast<NG::ScrollBarProxy>(scrollProxy);
    if (!proxy) {
        proxy = AceType::MakeRefPtr<NG::ScrollBarProxy>();
    }

    return proxy;
}

void ScrollBarModelNG::Create(const RefPtr<ScrollProxy>& proxy, bool infoflag, bool proxyFlag,
    int directionValue, int stateValue)
{
    CHECK_NULL_VOID(proxy);
    auto* stack = ViewStackProcessor::GetInstance();
    CHECK_NULL_VOID(stack);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::SCROLL_BAR_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<ScrollBarPattern>(); });
    CHECK_NULL_VOID(frameNode);
    stack->Push(frameNode);
    auto scrollbarpattern = frameNode->GetPattern();
    CHECK_NULL_VOID(scrollbarpattern);
    auto pattern = AceType::DynamicCast<NG::ScrollBarPattern>(scrollbarpattern);
    CHECK_NULL_VOID(pattern);
    if (infoflag) {
        if (proxyFlag) {
            auto scrollBarProxy = AceType::DynamicCast<NG::ScrollBarProxy>(proxy);
            CHECK_NULL_VOID(scrollBarProxy);
            scrollBarProxy->RegisterScrollBar(pattern);
            pattern->SetScrollBarProxy(scrollBarProxy);
        }

        if (directionValue < 0 || directionValue >= static_cast<int32_t>(AXIS.size())) {
            directionValue = static_cast<int32_t>(Axis::VERTICAL);
        }

        if (stateValue < 0 || stateValue >= static_cast<int32_t>(DISPLAY_MODE.size())) {
            stateValue = static_cast<int32_t>(DisplayMode::AUTO);
        }

        ACE_UPDATE_LAYOUT_PROPERTY(ScrollBarLayoutProperty, Axis, AXIS[directionValue]);
        ACE_UPDATE_LAYOUT_PROPERTY(ScrollBarLayoutProperty, DisplayMode, DISPLAY_MODE[stateValue]);
        auto visible = (DISPLAY_MODE[stateValue] == DisplayMode::OFF) ? VisibleType::INVISIBLE : VisibleType::VISIBLE;
        // VisibleType controls whether the scroll bar is visible. When DisplayMode is AUTO or ON,
        // to prevent the scrollbar from appearing when it is not scrollable, set the initial opacity to 0.
        pattern->SetOpacity(0);
        ACE_UPDATE_LAYOUT_PROPERTY(ScrollBarLayoutProperty, Visibility, visible);
    }
}
} // namespace OHOS::Ace::NG
