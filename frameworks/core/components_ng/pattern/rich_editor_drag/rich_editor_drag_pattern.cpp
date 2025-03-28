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

#include "core/components_ng/pattern/rich_editor_drag/rich_editor_drag_pattern.h"

#include "base/utils/utils.h"
#include "core/components_ng/pattern/rich_editor_drag/rich_editor_drag_info.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/text_drag/text_drag_base.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
RefPtr<FrameNode> RichEditorDragPattern::CreateDragNode(const RefPtr<FrameNode>& hostNode,
    const RichEditorDragInfo& info)
{
    CHECK_NULL_RETURN(hostNode, nullptr);
    auto hostPattern = hostNode->GetPattern<TextDragBase>();
    CHECK_NULL_RETURN(hostPattern, nullptr);
    const auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto dragNode = FrameNode::GetOrCreateFrameNode(V2::RICH_EDITOR_DRAG_ETS_TAG, nodeId, [hostPattern, info]() {
        auto dragInfo = std::make_shared<RichEditorDragInfo>(info);
        return MakeRefPtr<RichEditorDragPattern>(DynamicCast<TextPattern>(hostPattern), dragInfo);
    });
    auto dragContext = dragNode->GetRenderContext();
    CHECK_NULL_RETURN(dragContext, nullptr);
    auto hostContext = hostNode->GetRenderContext();
    CHECK_NULL_RETURN(hostContext, nullptr);
    if (hostContext->HasForegroundColor()) {
        dragContext->UpdateForegroundColor(hostContext->GetForegroundColor().value());
    }
    if (hostContext->HasForegroundColorStrategy()) {
        dragContext->UpdateForegroundColorStrategy(hostContext->GetForegroundColorStrategy().value());
    }
    auto dragPattern = dragNode->GetPattern<RichEditorDragPattern>();
    CHECK_NULL_RETURN(dragPattern, nullptr);
    auto data = CalculateTextDragData(hostPattern, dragNode);
    dragPattern->Initialize(data);
    dragPattern->SetLastLineHeight(data.lineHeight_);
    float frameWidth = dragPattern->GetFrameWidth();
    float frameHeight = dragPattern->GetFrameHeight();
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "CreateDragNode width=%{public}f, height=%{public}f", frameWidth, frameHeight);
    CalcSize size(NG::CalcLength(dragPattern->GetFrameWidth()), NG::CalcLength(dragPattern->GetFrameHeight()));
    dragNode->GetLayoutProperty()->UpdateUserDefinedIdealSize(size);
    return dragNode;
}

RefPtr<FrameNode> RichEditorDragPattern::CreateDragNode(
    const RefPtr<FrameNode>& hostNode, std::list<RefPtr<FrameNode>>& imageChildren)
{
    RichEditorDragInfo info;
    return RichEditorDragPattern::CreateDragNode(hostNode, imageChildren, info);
}

RefPtr<FrameNode> RichEditorDragPattern::CreateDragNode(
    const RefPtr<FrameNode>& hostNode, std::list<RefPtr<FrameNode>>& imageChildren, const RichEditorDragInfo& info)
{
    CHECK_NULL_RETURN(hostNode, nullptr);
    auto hostPattern = hostNode->GetPattern<TextDragBase>();
    CHECK_NULL_RETURN(hostPattern, nullptr);
    auto dragNode = CreateDragNode(hostNode, info);
    CHECK_NULL_RETURN(dragNode, nullptr);
    auto dragPattern = dragNode->GetPattern<RichEditorDragPattern>();
    CHECK_NULL_RETURN(dragPattern, nullptr);
    auto richEditor = hostNode->GetPattern<TextPattern>();
    CHECK_NULL_RETURN(richEditor, nullptr);
    auto placeholderIndex = richEditor->GetPlaceHolderIndex();
    auto rectsForPlaceholders = richEditor->GetRectsForPlaceholders();

    size_t index = 0;
    std::vector<RectF> realRectsForPlaceholders;
    std::list<RefPtr<FrameNode>> realImageChildren;
    auto boxes = hostPattern->GetTextBoxes();
    for (const auto& child : imageChildren) {
        auto imageIndex = placeholderIndex[index];
        if (imageIndex >= static_cast<int32_t>(rectsForPlaceholders.size())) {
            break;
        }
        auto rect = rectsForPlaceholders.at(imageIndex);

        for (const auto& box : boxes) {
            if (box.IsInRegion({rect.GetX() + rect.Width() / 2, rect.GetY() + rect.Height() / 2})) {
                auto gestureHub = child->GetOrCreateGestureEventHub();
                if (gestureHub) {
                    gestureHub->SetPixelMap(nullptr);
                }
                realImageChildren.emplace_back(child);
                realRectsForPlaceholders.emplace_back(rect);
            }
        }
        ++index;
    }
    if (!boxes.empty()) {
        dragPattern->SetLastLineHeight(boxes.back().Height());
    }
    dragPattern->InitSpanImageLayout(realImageChildren, realRectsForPlaceholders);
    return dragNode;
}
} // namespace OHOS::Ace::NG
