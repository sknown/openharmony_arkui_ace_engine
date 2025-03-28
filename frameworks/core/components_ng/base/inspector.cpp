/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/base/inspector.h"

#include <unistd.h>
#include <unordered_set>

#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/common/container.h"
#include "core/common/recorder/event_recorder.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/stage/page_info.h"
#include "core/components_ng/pattern/stage/page_pattern.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/base/memory/type_info_base.h"
#include "foundation/arkui/ace_engine/frameworks/base/utils/utf.h"

namespace OHOS::Ace::NG {
namespace {
const char INSPECTOR_TYPE[] = "$type";
const char INSPECTOR_ID[] = "$ID";
const char INSPECTOR_RECT[] = "$rect";
const char INSPECTOR_ATTRS[] = "$attrs";
const char INSPECTOR_ROOT[] = "root";
const char INSPECTOR_PAGE_URL[] = "pageUrl";
const char INSPECTOR_NAV_DST_NAME[] = "navDstName";
const char INSPECTOR_WIDTH[] = "width";
const char INSPECTOR_HEIGHT[] = "height";
const char INSPECTOR_RESOLUTION[] = "$resolution";
const char INSPECTOR_CHILDREN[] = "$children";
const char INSPECTOR_DEBUGLINE[] = "$debugLine";
#ifdef PREVIEW
const char INSPECTOR_VIEW_ID[] = "$viewID";
#else
const char INSPECTOR_CUSTOM_VIEW_TAG[] = "viewTag";
const char INSPECTOR_COMPONENT_TYPE[] = "type";
const char INSPECTOR_STATE_VAR[] = "state";
#endif
const char INSPECTOR_ATTR_ID[] = "id";
const char INSPECTOR_LABEL[] = "label";
const char INSPECTOR_CONTENT[] = "content";
const char INSPECTOR_ENABLED[] = "enabled";
const char INSPECTOR_OPACITY[] = "opacity";
const char INSPECTOR_ZINDEX[] = "zindex";
const char INSPECTOR_VISIBILITY[] = "visibility";


const uint32_t LONG_PRESS_DELAY = 1000;
RectF deviceRect;

RefPtr<UINode> GetInspectorByKey(const RefPtr<FrameNode>& root, const std::string& key)
{
    std::queue<RefPtr<UINode>> elements;
    elements.push(root);
    RefPtr<UINode> inspectorElement;
    while (!elements.empty()) {
        auto current = elements.front();
        elements.pop();
        if (key == current->GetInspectorId().value_or("")) {
            return current;
        }

        const auto& children = current->GetChildren();
        for (const auto& child : children) {
            elements.push(child);
        }
    }
    return nullptr;
}

void DumpElementTree(
    int32_t depth, const RefPtr<UINode>& element, std::map<int32_t, std::list<RefPtr<UINode>>>& depthElementMap)
{
    if (element->GetChildren().empty()) {
        return;
    }
    const auto& children = element->GetChildren();
    depthElementMap[depth].insert(depthElementMap[depth].end(), children.begin(), children.end());
    for (const auto& depthElement : children) {
        DumpElementTree(depth + 1, depthElement, depthElementMap);
    }
}

TouchEvent GetUpPoint(const TouchEvent& downPoint)
{
    return TouchEvent {}
        .SetX(downPoint.x)
        .SetY(downPoint.y)
        .SetType(TouchType::UP)
        .SetTime(std::chrono::high_resolution_clock::now())
        .SetSourceType(SourceType::TOUCH);
}
#ifdef PREVIEW
void GetFrameNodeChildren(const RefPtr<NG::UINode>& uiNode, std::vector<RefPtr<NG::UINode>>& children, int32_t pageId,
    bool isLayoutInspector = false)
{
    // Set ViewId for the fast preview.
    auto parent = uiNode->GetParent();
    if (parent && parent->GetTag() == "JsView") {
        uiNode->SetViewId(std::to_string(parent->GetId()));
    } else {
        uiNode->SetViewId(parent->GetViewId());
    }
    if (uiNode->GetTag() == "stage") {
    } else if (uiNode->GetTag() == "page") {
        if (uiNode->GetPageId() != pageId) {
            return;
        }
    } else {
        if (!uiNode->GetDebugLine().empty()) {
            children.emplace_back(uiNode);
            return;
        }
    }

    for (const auto& frameChild : uiNode->GetChildren()) {
        GetFrameNodeChildren(frameChild, children, pageId);
    }
}

void GetSpanInspector(
    const RefPtr<NG::UINode>& parent, std::unique_ptr<OHOS::Ace::JsonValue>& jsonNodeArray, int pageId)
{
    // span rect follows parent text size
    auto spanParentNode = parent->GetParent();
    while (spanParentNode != nullptr) {
        if (AceType::InstanceOf<NG::FrameNode>(spanParentNode)) {
            break;
        }
        spanParentNode = spanParentNode->GetParent();
    }
    CHECK_NULL_VOID(spanParentNode);
    auto node = AceType::DynamicCast<FrameNode>(spanParentNode);
    auto jsonNode = JsonUtil::Create(true);
    auto jsonObject = JsonUtil::Create(true);

    InspectorFilter filter;
    parent->ToJsonValue(jsonObject, filter);
    jsonNode->PutRef(INSPECTOR_ATTRS, std::move(jsonObject));
    jsonNode->Put(INSPECTOR_TYPE, parent->GetTag().c_str());
    jsonNode->Put(INSPECTOR_ID, parent->GetId());
    RectF rect = node->GetTransformRectRelativeToWindow();
    rect = rect.Constrain(deviceRect);
    if (rect.IsEmpty()) {
        rect.SetRect(0, 0, 0, 0);
    }
    auto strRec = std::to_string(rect.Left())
                      .append(",")
                      .append(std::to_string(rect.Top()))
                      .append(",")
                      .append(std::to_string(rect.Width()))
                      .append(",")
                      .append(std::to_string(rect.Height()));
    jsonNode->Put(INSPECTOR_RECT, strRec.c_str());
    jsonNode->Put(INSPECTOR_DEBUGLINE, parent->GetDebugLine().c_str());
    jsonNode->Put(INSPECTOR_VIEW_ID, parent->GetViewId().c_str());
    jsonNodeArray->PutRef(std::move(jsonNode));
}

void GetInspectorChildren(const RefPtr<NG::UINode>& parent, std::unique_ptr<OHOS::Ace::JsonValue>& jsonNodeArray,
    int pageId, bool isActive, const InspectorFilter& filter = InspectorFilter(), uint32_t depth = UINT32_MAX,
    bool isLayoutInspector = false)
{
    // Span is a special case in Inspector since span inherits from UINode
    if (AceType::InstanceOf<SpanNode>(parent)) {
        GetSpanInspector(parent, jsonNodeArray, pageId);
        return;
    }
    auto jsonNode = JsonUtil::Create(true);
    jsonNode->Put(INSPECTOR_TYPE, parent->GetTag().c_str());
    jsonNode->Put(INSPECTOR_ID, parent->GetId());
    auto node = AceType::DynamicCast<FrameNode>(parent);
    if (node) {
        RectF rect;
        isActive = isActive && node->IsActive();
        if (isActive) {
            rect = node->GetTransformRectRelativeToWindow();
        }
        rect = rect.Constrain(deviceRect);
        if (rect.IsEmpty()) {
            rect.SetRect(0, 0, 0, 0);
        }
        auto strRec = std::to_string(rect.Left()).append(",")
                          .append(std::to_string(rect.Top())).append(",")
                          .append(std::to_string(rect.Width())).append(",")
                          .append(std::to_string(rect.Height()));
        jsonNode->Put(INSPECTOR_RECT, strRec.c_str());
        jsonNode->Put(INSPECTOR_DEBUGLINE, node->GetDebugLine().c_str());
        jsonNode->Put(INSPECTOR_VIEW_ID, node->GetViewId().c_str());
        auto jsonObject = JsonUtil::Create(true);

        InspectorFilter filter;
        parent->ToJsonValue(jsonObject, filter);
        jsonNode->PutRef(INSPECTOR_ATTRS, std::move(jsonObject));
    }

    std::vector<RefPtr<NG::UINode>> children;
    for (const auto& item : parent->GetChildren()) {
        GetFrameNodeChildren(item, children, pageId);
    }
    if (node != nullptr) {
        auto overlayNode = node->GetOverlayNode();
        if (overlayNode != nullptr) {
            GetFrameNodeChildren(overlayNode, children, pageId);
        }
    }
    if (depth) {
        auto jsonChildrenArray = JsonUtil::CreateArray(true);
        for (auto uiNode : children) {
            GetInspectorChildren(uiNode, jsonChildrenArray, pageId, isActive, filter, depth - 1);
        }
        if (jsonChildrenArray->GetArraySize()) {
            jsonNode->PutRef(INSPECTOR_CHILDREN, std::move(jsonChildrenArray));
        }
    }
    jsonNodeArray->PutRef(std::move(jsonNode));
}

#else
void GetFrameNodeChildren(const RefPtr<NG::UINode>& uiNode, std::vector<RefPtr<NG::UINode>>& children,
    int32_t pageId, bool isLayoutInspector = false)
{
    if (AceType::InstanceOf<NG::FrameNode>(uiNode) || AceType::InstanceOf<SpanNode>(uiNode) ||
        AceType::InstanceOf<CustomNode>(uiNode)) {
        if (uiNode->GetTag() == "stage") {
        } else if (uiNode->GetTag() == "page") {
            if (uiNode->GetPageId() != pageId) {
                return;
            }
        } else {
            auto custom = AceType::DynamicCast<NG::CustomNode>(uiNode);
            auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
            auto spanNode = AceType::DynamicCast<NG::SpanNode>(uiNode);
            if ((frameNode && !frameNode->IsInternal()) || spanNode || (custom && isLayoutInspector)) {
                children.emplace_back(uiNode);
                return;
            }
        }
    }
    for (const auto& frameChild : uiNode->GetChildren()) {
        GetFrameNodeChildren(frameChild, children, pageId, isLayoutInspector);
    }
}

void GetSpanInspector(
    const RefPtr<NG::UINode>& parent, std::unique_ptr<OHOS::Ace::JsonValue>& jsonNodeArray, int pageId)
{
    // span rect follows parent text size
    auto spanParentNode = parent->GetParent();
    while (spanParentNode != nullptr) {
        if (AceType::InstanceOf<NG::FrameNode>(spanParentNode)) {
            break;
        }
        spanParentNode = spanParentNode->GetParent();
    }
    CHECK_NULL_VOID(spanParentNode);
    auto node = AceType::DynamicCast<FrameNode>(spanParentNode);
    auto jsonNode = JsonUtil::Create(true);
    auto jsonObject = JsonUtil::Create(true);

    InspectorFilter filter;
    parent->ToJsonValue(jsonObject, filter);
    jsonNode->PutRef(INSPECTOR_ATTRS, std::move(jsonObject));
    jsonNode->Put(INSPECTOR_TYPE, parent->GetTag().c_str());
    jsonNode->Put(INSPECTOR_ID, parent->GetId());
    jsonNode->Put(INSPECTOR_DEBUGLINE, parent->GetDebugLine().c_str());
    RectF rect = node->GetTransformRectRelativeToWindow();
    jsonNode->Put(INSPECTOR_RECT, rect.ToBounds().c_str());
    jsonNodeArray->PutRef(std::move(jsonNode));
}

void GetCustomNodeInfo(const RefPtr<NG::UINode> &customNode, std::unique_ptr<OHOS::Ace::JsonValue> &jsonNode)
{
    // custom node rect follows parent size
    auto hostNode = customNode->GetParent();
    while (hostNode != nullptr) {
        if (AceType::InstanceOf<NG::FrameNode>(hostNode)) {
            break;
        }
        hostNode = hostNode->GetParent();
    }
    CHECK_NULL_VOID(hostNode);
    jsonNode->Put(INSPECTOR_COMPONENT_TYPE, "custom");
    auto node = AceType::DynamicCast<CustomNode>(customNode);
    CHECK_NULL_VOID(node);
    auto parentNode = AceType::DynamicCast<FrameNode>(hostNode);
    jsonNode->Put(INSPECTOR_STATE_VAR, node->GetStateInspectorInfo());
    RectF rect = parentNode->GetTransformRectRelativeToWindow();
    jsonNode->Put(INSPECTOR_RECT, rect.ToBounds().c_str());
    jsonNode->Put(INSPECTOR_DEBUGLINE, customNode->GetDebugLine().c_str());
    jsonNode->Put(INSPECTOR_CUSTOM_VIEW_TAG, node->GetCustomTag().c_str());
}

void GetInspectorChildren(const RefPtr<NG::UINode>& parent, std::unique_ptr<OHOS::Ace::JsonValue>& jsonNodeArray,
    int pageId, bool isActive, const InspectorFilter& filter = InspectorFilter(), uint32_t depth = UINT32_MAX,
    bool isLayoutInspector = false)
{
    // Span is a special case in Inspector since span inherits from UINode
    if (AceType::InstanceOf<SpanNode>(parent)) {
        GetSpanInspector(parent, jsonNodeArray, pageId);
        return;
    }
    if (AceType::InstanceOf<CustomNode>(parent) && !isLayoutInspector) {
        return;
    }
    auto jsonNode = JsonUtil::Create(true);
    jsonNode->Put(INSPECTOR_TYPE, parent->GetTag().c_str());
    jsonNode->Put(INSPECTOR_ID, parent->GetId());
    if (parent->GetTag() == V2::JS_VIEW_ETS_TAG) {
        GetCustomNodeInfo(parent, jsonNode);
    } else {
        jsonNode->Put(INSPECTOR_COMPONENT_TYPE, "build-in");
    }
    auto node = AceType::DynamicCast<FrameNode>(parent);
    if (node) {
        RectF rect;
        isActive = isActive && node->IsActive();
        if (isActive) {
            rect = node->GetTransformRectRelativeToWindow();
        }
        jsonNode->Put(INSPECTOR_RECT, rect.ToBounds().c_str());
        jsonNode->Put(INSPECTOR_DEBUGLINE, node->GetDebugLine().c_str());
    }
    auto jsonObject = JsonUtil::Create(true);
    parent->ToJsonValue(jsonObject, filter);
    jsonNode->PutRef(INSPECTOR_ATTRS, std::move(jsonObject));
    std::string jsonNodeStr = jsonNode->ToString();
    ConvertIllegalStr(jsonNodeStr);
    auto jsonNodeNew = JsonUtil::ParseJsonString(jsonNodeStr);
    std::vector<RefPtr<NG::UINode>> children;
    for (const auto& item : parent->GetChildren()) {
        GetFrameNodeChildren(item, children, pageId, isLayoutInspector);
    }
    if (node) {
        auto overlayNode = node->GetOverlayNode();
        if (overlayNode != nullptr) {
            GetFrameNodeChildren(overlayNode, children, pageId, isLayoutInspector);
        }
    }
    if (depth) {
        auto jsonChildrenArray = JsonUtil::CreateArray(true);
        for (auto uiNode : children) {
            GetInspectorChildren(uiNode, jsonChildrenArray, pageId, isActive, filter, depth - 1, isLayoutInspector);
        }
        if (jsonChildrenArray->GetArraySize()) {
            jsonNodeNew->PutRef(INSPECTOR_CHILDREN, std::move(jsonChildrenArray));
        }
    }
    jsonNodeArray->PutRef(std::move(jsonNodeNew));
}
#endif

RefPtr<NG::UINode> GetOverlayNode(const RefPtr<NG::UINode>& pageNode)
{
    CHECK_NULL_RETURN(pageNode, nullptr);
    auto stageNode = pageNode->GetParent();
    CHECK_NULL_RETURN(stageNode, nullptr);
    auto stageParent = stageNode->GetParent();
    CHECK_NULL_RETURN(stageParent, nullptr);
    auto overlayNode = stageParent->GetChildren().back();
    if (overlayNode->GetTag() == "stage") {
        return nullptr;
    }
    return overlayNode;
}

void GetContextInfo(const RefPtr<PipelineContext>& context, std::unique_ptr<JsonValue>& jsonRoot)
{
    auto scale = context->GetViewScale();
    auto rootHeight = context->GetRootHeight();
    auto rootWidth = context->GetRootWidth();
    deviceRect.SetRect(0, 0, rootWidth * scale, rootHeight * scale);
    jsonRoot->Put(INSPECTOR_WIDTH, std::to_string(rootWidth * scale).c_str());
    jsonRoot->Put(INSPECTOR_HEIGHT, std::to_string(rootHeight * scale).c_str());
    jsonRoot->Put(INSPECTOR_RESOLUTION, std::to_string(PipelineBase::GetCurrentDensity()).c_str());
}

std::string GetInspectorInfo(std::vector<RefPtr<NG::UINode>> children, int32_t pageId,
    std::unique_ptr<JsonValue> jsonRoot, bool isLayoutInspector, const InspectorFilter& filter = InspectorFilter())
{
    auto jsonNodeArray = JsonUtil::CreateArray(true);
    auto depth = filter.GetFilterDepth();
    for (auto& uiNode : children) {
        GetInspectorChildren(uiNode, jsonNodeArray, pageId, true, filter, depth - 1, isLayoutInspector);
    }
    if (jsonNodeArray->GetArraySize()) {
        jsonRoot->PutRef(INSPECTOR_CHILDREN, std::move(jsonNodeArray));
    }

    if (isLayoutInspector) {
        auto jsonTree = JsonUtil::Create(true);
        jsonTree->Put("type", "root");
        jsonTree->PutRef("content", std::move(jsonRoot));
        auto pipeline = PipelineContext::GetCurrentContextSafely();
        if (pipeline) {
            jsonTree->Put("VsyncID", (int32_t)pipeline->GetFrameCount());
            jsonTree->Put("ProcessID", getpid());
            jsonTree->Put("WindowID", (int32_t)pipeline->GetWindowId());
        }
        return jsonTree->ToString();
    }

    return jsonRoot->ToString();
}
} // namespace

std::set<RefPtr<FrameNode>> Inspector::offscreenNodes;

RefPtr<FrameNode> Inspector::GetFrameNodeByKey(const std::string& key)
{
    if (!offscreenNodes.empty()) {
        for (auto node : offscreenNodes) {
            auto frameNode = AceType::DynamicCast<FrameNode>(GetInspectorByKey(node, key));
            if (frameNode) {
                return frameNode;
            }
        }
    }
    auto context = NG::PipelineContext::GetCurrentContext();
    if (!context) {
        LOGW("Internal error! The PipelineContext returned by the system is null. param: %{public}s", key.c_str());
        return nullptr;
    }
    auto rootNode = context->GetRootElement();
    if (!rootNode) {
        LOGW("Internal error! The rootNode returned by the system is null. param: %{public}s", key.c_str());
        return nullptr;
    }

    return AceType::DynamicCast<FrameNode>(GetInspectorByKey(rootNode, key));
}

std::string Inspector::GetInspectorNodeByKey(const std::string& key, const InspectorFilter& filter)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, "");
    auto rootNode = context->GetRootElement();
    CHECK_NULL_RETURN(rootNode, "");

    auto inspectorElement = GetInspectorByKey(rootNode, key);
    CHECK_NULL_RETURN(inspectorElement, "");

    auto jsonNode = JsonUtil::Create(true);
    jsonNode->Put(INSPECTOR_TYPE, inspectorElement->GetTag().c_str());
    jsonNode->Put(INSPECTOR_ID, inspectorElement->GetId());
    auto frameNode = AceType::DynamicCast<FrameNode>(inspectorElement);
    if (frameNode) {
        auto rect = frameNode->GetTransformRectRelativeToWindow();
        jsonNode->Put(INSPECTOR_RECT, rect.ToBounds().c_str());
    }
    auto jsonAttrs = JsonUtil::Create(true);
    std::string debugLine = inspectorElement->GetDebugLine();
    jsonNode->Put(INSPECTOR_DEBUGLINE, debugLine.c_str());

    inspectorElement->ToJsonValue(jsonAttrs, filter);
    jsonNode->PutRef(INSPECTOR_ATTRS, std::move(jsonAttrs));
    return jsonNode->ToString();
}

void Inspector::GetRectangleById(const std::string& key, Rectangle& rectangle)
{
    auto frameNode = Inspector::GetFrameNodeByKey(key);
    if (!frameNode) {
        LOGW("Can't find a component that id or key are %{public}s, Please check your parameters are correct",
            key.c_str());
        return;
    }
    rectangle.size = frameNode->GetGeometryNode()->GetFrameSize();
    auto context = frameNode->GetRenderContext();
    if (!context) {
        LOGW("Internal error! The RenderContext returned by the component(id=%{public}s) is null",
            key.c_str());
        return;
    }
    rectangle.localOffset = context->GetPaintRectWithTransform().GetOffset();
    rectangle.windowOffset = frameNode->GetOffsetRelativeToWindow();
    auto pipeline = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    rectangle.screenRect = pipeline->GetCurrentWindowRect();
    LOGD("GetRectangleById Id = %{public}s localOffset = %{public}s windowOffset = %{public}s screenRect = %{public}s",
        key.c_str(), rectangle.localOffset.ToString().c_str(), rectangle.windowOffset.ToString().c_str(),
        rectangle.screenRect.ToString().c_str());
    auto renderContext = frameNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    Matrix4 defMatrix4 = Matrix4::CreateIdentity();
    Matrix4 matrix4 = renderContext->GetTransformMatrixValue(defMatrix4);
    rectangle.matrix4 = matrix4;
    auto rect = renderContext->GetPaintRectWithoutTransform();
    const double halfDimension = 50.0;
    auto center = renderContext->GetTransformCenter().value_or(DimensionOffset(
        Dimension(halfDimension, DimensionUnit::PERCENT), Dimension(halfDimension, DimensionUnit::PERCENT)));
    double centerX = 0.0;
    double centerY = 0.0;
    if (center.GetX().Unit() == DimensionUnit::PERCENT || center.GetY().Unit() == DimensionUnit::PERCENT) {
        if (rect.IsValid()) {
            centerX = Dimension(center.GetX().ConvertToPxWithSize(rect.Width()), DimensionUnit::PX).ConvertToVp();
            centerY = Dimension(center.GetY().ConvertToPxWithSize(rect.Height()), DimensionUnit::PX).ConvertToVp();
        }
    } else {
        centerX = center.GetX().ConvertToVp();
        centerY = center.GetY().ConvertToVp();
    }
    VectorF defScale = VectorF(1.0, 1.0);
    VectorF scale = renderContext->GetTransformScaleValue(defScale);
    rectangle.scale.x = scale.x;
    rectangle.scale.y = scale.y;
    rectangle.scale.z = 1.0;
    rectangle.scale.centerX = centerX;
    rectangle.scale.centerY = centerY;
    Vector5F defRotate = Vector5F(0.0, 0.0, 0.0, 0.0, 0.0);
    Vector5F rotate = renderContext->GetTransformRotateValue(defRotate);
    rectangle.rotate.x = rotate.x;
    rectangle.rotate.y = rotate.y;
    rectangle.rotate.z = rotate.z;
    rectangle.rotate.angle = rotate.w;
    rectangle.rotate.centerX = centerX;
    rectangle.rotate.centerY = centerY;
    TranslateOptions defTranslate = TranslateOptions(0.0, 0.0, 0.0);
    TranslateOptions translate = renderContext->GetTransformTranslateValue(defTranslate);
    if ((translate.x.Unit() == DimensionUnit::PERCENT) && rect.IsValid()) {
        rectangle.translate.x =
            Dimension(translate.x.ConvertToPxWithSize(rect.Width()), DimensionUnit::PX).ConvertToVp();
    } else {
        rectangle.translate.x = translate.x.ConvertToVp();
    }
    if ((translate.y.Unit() == DimensionUnit::PERCENT) && rect.IsValid()) {
        rectangle.translate.y =
            Dimension(translate.y.ConvertToPxWithSize(rect.Height()), DimensionUnit::PX).ConvertToVp();
    } else {
        rectangle.translate.y = translate.y.ConvertToVp();
    }
    rectangle.translate.z = translate.z.ConvertToVp();
}

std::string Inspector::GetInspector(bool isLayoutInspector)
{
    InspectorFilter filter;
    bool needThrow = false;
    return GetInspector(isLayoutInspector, filter, needThrow);
}

std::string Inspector::GetInspector(bool isLayoutInspector, const InspectorFilter& filter, bool& needThrow)
{
    auto jsonRoot = JsonUtil::Create(true);
    jsonRoot->Put(INSPECTOR_TYPE, INSPECTOR_ROOT);
    needThrow = false;
    auto context = NG::PipelineContext::GetCurrentContext();
    if (context == nullptr) {
        needThrow = true;
        return jsonRoot->ToString();
    }
    GetContextInfo(context, jsonRoot);

    RefPtr<UINode> pageRootNode;
    const std::string key = filter.GetFilterID();
    if (key.empty()) {
        pageRootNode = context->GetStageManager()->GetLastPage();
    } else {
        auto rootNode = context->GetStageManager()->GetLastPage();
        if (rootNode == nullptr) {
            needThrow = true;
            return jsonRoot->ToString();
        }
        pageRootNode = GetInspectorByKey(rootNode, key);
    }
    if (pageRootNode == nullptr) {
        needThrow = true;
        return jsonRoot->ToString();
    }
    auto pageId = context->GetStageManager()->GetLastPage()->GetPageId();
    std::vector<RefPtr<NG::UINode>> children;
    if (key.empty()) {
        for (const auto& item : pageRootNode->GetChildren()) {
            GetFrameNodeChildren(item, children, pageId, isLayoutInspector);
        }
        auto overlayNode = GetOverlayNode(pageRootNode);
        if (overlayNode) {
            GetFrameNodeChildren(overlayNode, children, pageId, isLayoutInspector);
        }
    } else {
        children.emplace_back(pageRootNode);
    }
    return GetInspectorInfo(children, pageId, std::move(jsonRoot), isLayoutInspector, filter);
}

std::string Inspector::GetInspectorOfNode(RefPtr<NG::UINode> node)
{
    auto jsonRoot = JsonUtil::Create(true);

    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, jsonRoot->ToString());
    GetContextInfo(context, jsonRoot);
    CHECK_NULL_RETURN(node, jsonRoot->ToString());
    auto pageId = context->GetStageManager()->GetLastPage()->GetPageId();
    auto jsonNodeArray = JsonUtil::CreateArray(true);
    GetInspectorChildren(node, jsonNodeArray, pageId, true);
    if (jsonNodeArray->GetArraySize()) {
        jsonRoot = jsonNodeArray->GetArrayItem(0);
        jsonRoot->Delete(INSPECTOR_CHILDREN);
        GetContextInfo(context, jsonRoot);
    }

    return jsonRoot->ToString();
}

std::string Inspector::GetSubWindowInspector(bool isLayoutInspector)
{
    auto jsonRoot = JsonUtil::Create(true);
    jsonRoot->Put(INSPECTOR_TYPE, INSPECTOR_ROOT);

    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, jsonRoot->ToString());
    GetContextInfo(context, jsonRoot);
    auto overlayNode = context->GetOverlayManager()->GetRootNode().Upgrade();
    CHECK_NULL_RETURN(overlayNode, jsonRoot->ToString());
    auto pageId = 0;
    std::vector<RefPtr<NG::UINode>> children;
    GetFrameNodeChildren(overlayNode, children, pageId, isLayoutInspector);

    return GetInspectorInfo(children, 0, std::move(jsonRoot), isLayoutInspector);
}

void FillSimplifiedInspectorAttrs(const RefPtr<NG::UINode>& parent, std::unique_ptr<OHOS::Ace::JsonValue>& jsonNode)
{
    auto tmpJson = JsonUtil::Create(true);

    InspectorFilter filter;
    parent->ToJsonValue(tmpJson, filter);
    jsonNode->Put(INSPECTOR_ATTR_ID, tmpJson->GetString(INSPECTOR_ATTR_ID).c_str());

    auto jsonObject = JsonUtil::Create(true);
    if (tmpJson->Contains(INSPECTOR_LABEL)) {
        jsonObject->Put(INSPECTOR_LABEL, tmpJson->GetString(INSPECTOR_LABEL).c_str());
    }
    if (tmpJson->Contains(INSPECTOR_CONTENT)) {
        jsonObject->Put(INSPECTOR_CONTENT, tmpJson->GetString(INSPECTOR_CONTENT).c_str());
    }
    jsonObject->Put(INSPECTOR_ENABLED, tmpJson->GetBool(INSPECTOR_ENABLED));
    jsonObject->Put(INSPECTOR_OPACITY, tmpJson->GetDouble(INSPECTOR_OPACITY));
    jsonObject->Put(INSPECTOR_ZINDEX, tmpJson->GetInt(INSPECTOR_ZINDEX));
    jsonObject->Put(INSPECTOR_VISIBILITY, tmpJson->GetString(INSPECTOR_VISIBILITY).c_str());
    jsonNode->PutRef(INSPECTOR_ATTRS, std::move(jsonObject));
}

void GetSimplifiedSpanInspector(
    const RefPtr<NG::UINode>& parent, std::unique_ptr<OHOS::Ace::JsonValue>& jsonNodeArray, int pageId)
{
    // span rect follows parent text size
    auto spanParentNode = parent->GetParent();
    CHECK_NULL_VOID(spanParentNode);
    auto node = AceType::DynamicCast<FrameNode>(spanParentNode);
    CHECK_NULL_VOID(node);
    auto jsonNode = JsonUtil::Create(true);

    FillSimplifiedInspectorAttrs(parent, jsonNode);

    jsonNode->Put(INSPECTOR_TYPE, parent->GetTag().c_str());
    RectF rect = node->GetTransformRectRelativeToWindow();
    jsonNode->Put(INSPECTOR_RECT, rect.ToBounds().c_str());
    jsonNodeArray->PutRef(std::move(jsonNode));
}

void GetSimplifiedInspectorChildren(
    const RefPtr<NG::UINode>& parent, std::unique_ptr<OHOS::Ace::JsonValue>& jsonNodeArray, int pageId, bool isActive)
{
    // Span is a special case in Inspector since span inherits from UINode
    if (AceType::InstanceOf<SpanNode>(parent)) {
        GetSimplifiedSpanInspector(parent, jsonNodeArray, pageId);
        return;
    }
    auto jsonNode = JsonUtil::Create(true);
    jsonNode->Put(INSPECTOR_TYPE, parent->GetTag().c_str());
    auto node = AceType::DynamicCast<FrameNode>(parent);

    RectF rect;
    isActive = isActive && node->IsActive();
    if (isActive) {
        rect = node->GetTransformRectRelativeToWindow();
    }

    jsonNode->Put(INSPECTOR_RECT, rect.ToBounds().c_str());

    FillSimplifiedInspectorAttrs(parent, jsonNode);

    std::vector<RefPtr<NG::UINode>> children;
    for (const auto& item : parent->GetChildren()) {
        GetFrameNodeChildren(item, children, pageId);
    }
    auto jsonChildrenArray = JsonUtil::CreateArray(true);
    for (auto uiNode : children) {
        GetSimplifiedInspectorChildren(uiNode, jsonChildrenArray, pageId, isActive);
    }
    if (jsonChildrenArray->GetArraySize()) {
        jsonNode->PutRef(INSPECTOR_CHILDREN, std::move(jsonChildrenArray));
    }
    jsonNodeArray->PutRef(std::move(jsonNode));
}

std::string Inspector::GetSimplifiedInspector(int32_t containerId)
{
    TAG_LOGI(AceLogTag::ACE_UIEVENT, "GetSimplifiedInspector start: container %{public}d", containerId);
    auto jsonRoot = JsonUtil::Create(true);
    jsonRoot->Put(INSPECTOR_TYPE, INSPECTOR_ROOT);

    auto context = NG::PipelineContext::GetContextByContainerId(containerId);
    CHECK_NULL_RETURN(context, jsonRoot->ToString());
    auto scale = context->GetViewScale();
    auto rootHeight = context->GetRootHeight();
    auto rootWidth = context->GetRootWidth();
    deviceRect.SetRect(0, 0, rootWidth * scale, rootHeight * scale);
    jsonRoot->Put(INSPECTOR_WIDTH, std::to_string(rootWidth * scale).c_str());
    jsonRoot->Put(INSPECTOR_HEIGHT, std::to_string(rootHeight * scale).c_str());
    jsonRoot->Put(INSPECTOR_RESOLUTION, std::to_string(SystemProperties::GetResolution()).c_str());

    auto pageRootNode = context->GetStageManager()->GetLastPage();
    CHECK_NULL_RETURN(pageRootNode, jsonRoot->ToString());

    auto pagePattern = pageRootNode->GetPattern<PagePattern>();
    CHECK_NULL_RETURN(pagePattern, jsonRoot->ToString());
    auto pageInfo = pagePattern->GetPageInfo();
    CHECK_NULL_RETURN(pageInfo, jsonRoot->ToString());
    jsonRoot->Put(INSPECTOR_PAGE_URL, pageInfo->GetPageUrl().c_str());
    jsonRoot->Put(INSPECTOR_NAV_DST_NAME, Recorder::EventRecorder::Get().GetNavDstName().c_str());

    auto pageId = context->GetStageManager()->GetLastPage()->GetPageId();
    std::vector<RefPtr<NG::UINode>> children;
    for (const auto& item : pageRootNode->GetChildren()) {
        GetFrameNodeChildren(item, children, pageId);
    }
    auto overlayNode = GetOverlayNode(pageRootNode);
    if (overlayNode) {
        GetFrameNodeChildren(overlayNode, children, pageId);
    }
    auto jsonNodeArray = JsonUtil::CreateArray(true);
    for (auto& uiNode : children) {
        GetSimplifiedInspectorChildren(uiNode, jsonNodeArray, pageId, true);
    }
    if (jsonNodeArray->GetArraySize()) {
        jsonRoot->PutRef(INSPECTOR_CHILDREN, std::move(jsonNodeArray));
    }

    return jsonRoot->ToString();
}

bool Inspector::SendEventByKey(const std::string& key, int action, const std::string& params)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    auto rootNode = context->GetRootElement();
    CHECK_NULL_RETURN(rootNode, false);

    auto inspectorElement = AceType::DynamicCast<FrameNode>(GetInspectorByKey(rootNode, key));
    CHECK_NULL_RETURN(inspectorElement, false);

    auto size = inspectorElement->GetGeometryNode()->GetFrameSize();
    auto offset = inspectorElement->GetTransformRelativeOffset();
    Rect rect { offset.GetX(), offset.GetY(), size.Width(), size.Height() };
    context->GetTaskExecutor()->PostTask(
        [weak = AceType::WeakClaim(AceType::RawPtr(context)), rect, action, params]() {
            auto context = weak.Upgrade();
            if (!context) {
                return;
            }
            TouchEvent point;
            point.SetX(static_cast<float>(rect.Left() + rect.Width() / 2))
                .SetY(static_cast<float>(rect.Top() + rect.Height() / 2))
                .SetType(TouchType::DOWN)
                .SetTime(std::chrono::high_resolution_clock::now())
                .SetSourceType(SourceType::TOUCH);
            context->OnTouchEvent(point.UpdatePointers());

            switch (action) {
                case static_cast<int>(AceAction::ACTION_CLICK): {
                    context->OnTouchEvent(GetUpPoint(point).UpdatePointers());
                    break;
                }
                case static_cast<int>(AceAction::ACTION_LONG_CLICK): {
                    CancelableCallback<void()> inspectorTimer;
                    auto&& callback = [weak, point]() {
                        auto refPtr = weak.Upgrade();
                        if (refPtr) {
                            refPtr->OnTouchEvent(GetUpPoint(point).UpdatePointers());
                        }
                    };
                    inspectorTimer.Reset(callback);
                    auto taskExecutor =
                        SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
                    taskExecutor.PostDelayedTask(inspectorTimer, LONG_PRESS_DELAY, "ArkUIInspectorLongPressTouchEvent");
                    break;
                }
                default:
                    break;
            }
        },
        TaskExecutor::TaskType::UI, "ArkUIInspectorSendEventByKey");

    return true;
}

void Inspector::HideAllMenus()
{
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    overlayManager->HideAllMenus();
}

void Inspector::AddOffscreenNode(RefPtr<FrameNode> node)
{
    CHECK_NULL_VOID(node);
    offscreenNodes.insert(node);
}

void Inspector::RemoveOffscreenNode(RefPtr<FrameNode> node)
{
    CHECK_NULL_VOID(node);
    offscreenNodes.erase(node);
}

} // namespace OHOS::Ace::NG
