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

#include "core/components_ng/pattern/ui_extension/isolated_pattern.h"

#include "adapter/ohos/osal/want_wrap_ohos.h"
#include "core/event/key_event.h"
#include "core/event/pointer_event.h"
#include "display_manager.h"
#include "session/host/include/session.h"

namespace OHOS::Ace::NG {
namespace {
constexpr char RESOURCE_PATH[] = "resourcePath";
constexpr char ABC_PATH[] = "abcPath";
constexpr char ENTRY_POINT[] = "entryPoint";
}
IsolatedPattern::IsolatedPattern() : UIExtensionPattern()
{
    UIEXT_LOGI("The IsolatedPattern is created.");
}

IsolatedPattern::~IsolatedPattern()
{
    UIEXT_LOGI("The IsolatedPattern is destroyed.");
}

void IsolatedPattern::InitializeDynamicComponent(
    const std::string& hapPath, const std::string& abcPath, const std::string& entryPoint, void* runtime)
{
    componentType_ = ComponentType::DYNAMIC;
    if (hapPath.empty() || abcPath.empty() || entryPoint.empty() || runtime == nullptr) {
        UIEXT_LOGE("The param empty.");
        return;
    }

    curIsolatedInfo_.abcPath = abcPath;
    curIsolatedInfo_.reourcePath = hapPath;
    curIsolatedInfo_.entryPoint = entryPoint;
    InitializeRender(runtime);
}

void IsolatedPattern::InitializeIsolatedComponent(const RefPtr<OHOS::Ace::WantWrap>& wantWrap, void* runtime)
{
    componentType_ = ComponentType::DYNAMIC;
    auto want = AceType::DynamicCast<WantWrapOhos>(wantWrap)->GetWant();
    auto resourcePath = want.GetStringParam(RESOURCE_PATH);
    auto abcPath = want.GetStringParam(ABC_PATH);
    auto entryPoint = want.GetStringParam(ENTRY_POINT);
    if (resourcePath.empty() || abcPath.empty() || entryPoint.empty() || runtime == nullptr) {
        UIEXT_LOGE("The param empty.");
        return;
    }

    curIsolatedInfo_.abcPath = abcPath;
    curIsolatedInfo_.reourcePath = resourcePath;
    curIsolatedInfo_.entryPoint = entryPoint;
    InitializeRender(runtime);
}

void IsolatedPattern::InitializeRender(void* runtime)
{
    if (!dynamicComponentRenderer_) {
        ContainerScope scope(instanceId_);
        dynamicComponentRenderer_ = DynamicComponentRenderer::Create(GetHost(),
            curIsolatedInfo_.reourcePath, curIsolatedInfo_.abcPath, curIsolatedInfo_.entryPoint, runtime);
        CHECK_NULL_VOID(dynamicComponentRenderer_);
        dynamicComponentRenderer_->CreateContent();
    }
}

void IsolatedPattern::FireOnErrorCallback(int32_t code, const std::string& name, const std::string& message)
{
    UIEXT_LOGI("The OnError code: %{public}d, name: %{public}s, message: %{public}s.",
        code, name.c_str(), message.c_str());
    if (onErrorCallback_) {
        ContainerScope scope(instanceId_);
        onErrorCallback_(code, name, message);
        return;
    }
}

void IsolatedPattern::DispatchPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    CHECK_NULL_VOID(pointerEvent);
    CHECK_NULL_VOID(dynamicComponentRenderer_);
    dynamicComponentRenderer_->TransferPointerEvent(pointerEvent);
}

bool IsolatedPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    CHECK_NULL_RETURN(dynamicComponentRenderer_, false);
    CHECK_NULL_RETURN(dirty, false);
    auto host = dirty->GetHostNode();
    CHECK_NULL_RETURN(host, false);
    auto offset = host->GetPaintRectGlobalOffsetWithTranslate().first;
    auto size = dirty->GetGeometryNode()->GetFrameSize();
    Ace::ViewportConfig vpConfig;
    vpConfig.SetSize(size.Width(), size.Height());
    vpConfig.SetPosition(offset.GetX(), offset.GetY());
    float density = 1.0f;
    int32_t orientation = 0;
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (defaultDisplay) {
        density = defaultDisplay->GetVirtualPixelRatio();
        orientation = static_cast<int32_t>(defaultDisplay->GetOrientation());
    }
    vpConfig.SetDensity(density);
    vpConfig.SetOrientation(orientation);
    dynamicComponentRenderer_->UpdateViewportConfig(vpConfig, Rosen::WindowSizeChangeReason::UNDEFINED, nullptr);
    return false;
}

void IsolatedPattern::DispatchKeyEvent(const KeyEvent& event)
{
    CHECK_NULL_VOID(event.rawKeyEvent);
    CHECK_NULL_VOID(dynamicComponentRenderer_);
    dynamicComponentRenderer_->TransferKeyEvent(event.rawKeyEvent);
}

void IsolatedPattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
    CHECK_NULL_VOID(dynamicComponentRenderer_);
    dynamicComponentRenderer_->DestroyContent();
    dynamicComponentRenderer_ = nullptr;
}
} // namespace OHOS::Ace::NG
