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

#include "core/components_ng/base/view_abstract_model_ng.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t LONG_PRESS_DURATION = 280;

void CreateCustomMenu(std::function<void()>& buildFunc, const RefPtr<NG::FrameNode>& targetNode, bool isContextMenu,
    const NG::OffsetF& offset, const MenuParam& menuParam = MenuParam())
{
    NG::ScopedViewStackProcessor builderViewStackProcessor;
    buildFunc();
    auto customNode = NG::ViewStackProcessor::GetInstance()->Finish();
    NG::ViewAbstract::BindMenuWithCustomNode(customNode, targetNode, isContextMenu, offset, menuParam);
}
} // namespace

void ViewAbstractModelNG::BindMenu(
    std::vector<NG::OptionParam>&& params, std::function<void()>&& buildFunc, const MenuParam& menuParam)
{
    auto targetNode = NG::ViewStackProcessor::GetInstance()->GetMainFrameNode();
#ifdef ENABLE_DRAG_FRAMEWORK
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, IsBindOverlay, true);
#endif // ENABLE_DRAG_FRAMEWORK
    GestureEventFunc showMenu;
    auto weakTarget = AceType::WeakClaim(AceType::RawPtr(targetNode));
    if (!params.empty()) {
        showMenu = [params, weakTarget, menuParam](GestureEvent& info) mutable {
            auto targetNode = weakTarget.Upgrade();
            CHECK_NULL_VOID(targetNode);
            NG::OffsetF menuPosition { info.GetGlobalLocation().GetX(), info.GetGlobalLocation().GetY() };
            // menu already created
            if (params.empty()) {
                NG::ViewAbstract::ShowMenu(targetNode->GetId(), menuPosition);
                return;
            }
            NG::ViewAbstract::BindMenuWithItems(std::move(params), targetNode, menuPosition, menuParam);
            params.clear();
        };
    } else if (buildFunc) {
        showMenu = [builderFunc = std::move(buildFunc), weakTarget, menuParam](const GestureEvent& info) mutable {
            auto targetNode = weakTarget.Upgrade();
            CHECK_NULL_VOID(targetNode);
            NG::OffsetF menuPosition { info.GetGlobalLocation().GetX(), info.GetGlobalLocation().GetY() };
            CreateCustomMenu(builderFunc, targetNode, false, menuPosition, menuParam);
        };
    } else {
        LOGE("empty param or null builder");
        return;
    }
    auto gestureHub = targetNode->GetOrCreateGestureEventHub();
    gestureHub->BindMenu(std::move(showMenu));

    // delete menu when target node is removed from render tree
    auto eventHub = targetNode->GetEventHub<NG::EventHub>();
    auto destructor = [id = targetNode->GetId()]() {
        auto pipeline = NG::PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto overlayManager = pipeline->GetOverlayManager();
        CHECK_NULL_VOID(overlayManager);
        overlayManager->DeleteMenu(id);
    };
    eventHub->SetOnDisappear(destructor);
}

void ViewAbstractModelNG::BindContextMenu(ResponseType type, std::function<void()>&& buildFunc)
{
    auto targetNode = NG::ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(targetNode);
#ifdef ENABLE_DRAG_FRAMEWORK
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, IsBindOverlay, true);
#endif // ENABLE_DRAG_FRAMEWORK
    auto hub = targetNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(hub);
    auto weakTarget = AceType::WeakClaim(AceType::RawPtr(targetNode));
    if (type == ResponseType::RIGHT_CLICK) {
        OnMouseEventFunc event = [builder = std::move(buildFunc), weakTarget](MouseInfo& info) mutable {
            auto targetNode = weakTarget.Upgrade();
            CHECK_NULL_VOID(targetNode);
            NG::OffsetF menuPosition { info.GetGlobalLocation().GetX(), info.GetGlobalLocation().GetY() };
            auto pipelineContext = NG::PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipelineContext);
            auto windowRect = pipelineContext->GetDisplayWindowRectInfo();
            menuPosition += NG::OffsetF { windowRect.Left(), windowRect.Top() };
            if (info.GetButton() == MouseButton::RIGHT_BUTTON && info.GetAction() == MouseAction::RELEASE) {
                CreateCustomMenu(builder, targetNode, true, menuPosition);
                info.SetStopPropagation(true);
            }
        };
        auto inputHub = targetNode->GetOrCreateInputEventHub();
        CHECK_NULL_VOID(inputHub);
        inputHub->BindContextMenu(std::move(event));
    } else if (type == ResponseType::LONG_PRESS) {
        // create or show menu on long press
        auto event = [builder = std::move(buildFunc), weakTarget](const GestureEvent& info) mutable {
            auto targetNode = weakTarget.Upgrade();
            CHECK_NULL_VOID(targetNode);
            NG::OffsetF menuPosition { info.GetGlobalLocation().GetX(), info.GetGlobalLocation().GetY() };
            auto pipelineContext = NG::PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipelineContext);
            auto windowRect = pipelineContext->GetDisplayWindowRectInfo();
            menuPosition += NG::OffsetF { windowRect.Left(), windowRect.Top() };
            CreateCustomMenu(builder, targetNode, true, menuPosition);
        };
        auto longPress = AceType::MakeRefPtr<NG::LongPressEvent>(std::move(event));

        hub->SetLongPressEvent(longPress, false, true, LONG_PRESS_DURATION);
    } else {
        LOGE("The arg responseType is invalid.");
        return;
    }
}

void ViewAbstractModelNG::SetPivot(const Dimension& x, const Dimension& y, const Dimension& z)
{
    DimensionOffset center(x, y);
    if (!NearZero(z.Value())) {
        center.SetZ(z);
    }
    ViewAbstract::SetPivot(center);
}

void ViewAbstractModelNG::SetScale(float x, float y, float z)
{
    if (x < 0) {
        x = 1;
    }
    if (y < 0) {
        y = 1;
    }
    VectorF scale(x, y);
    ViewAbstract::SetScale(scale);
}

void ViewAbstractModelNG::BindContentCover(
    bool isShow, std::function<void(const std::string&)>&& callback, std::function<void()>&& buildFunc, int32_t type)
{
    auto targetNode = NG::ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(targetNode);
    auto buildNodeFunc = [buildFunc]() -> RefPtr<UINode> {
        NG::ScopedViewStackProcessor builderViewStackProcessor;
        buildFunc();
        auto customNode = NG::ViewStackProcessor::GetInstance()->Finish();
        return customNode;
    };
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);

    overlayManager->BindContentCover(isShow, std::move(callback), std::move(buildNodeFunc), type, targetNode->GetId());
}
} // namespace OHOS::Ace::NG
