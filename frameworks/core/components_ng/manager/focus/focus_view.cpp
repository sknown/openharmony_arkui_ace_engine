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

#include "core/components_ng/manager/focus/focus_view.h"

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/event/key_event.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

void FocusView::FocusViewShow(bool isTriggerByStep)
{
    TAG_LOGI(AceLogTag::ACE_FOCUS, "Focus view: %{public}s/%{public}d show", GetFrameName().c_str(), GetFrameId());
    auto viewRootScope = GetViewRootScope();
    if (viewRootScope && GetIsViewRootScopeFocused()) {
        viewRootScope->SetFocusDependence(FocusDependence::SELF);
    }
    isViewHasFocused_ = false;
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto focusManager = pipeline->GetFocusManager();
    CHECK_NULL_VOID(focusManager);
    focusManager->FocusViewShow(AceType::Claim(this), isTriggerByStep);
    pipeline->RequestFrame();
}

void FocusView::FocusViewHide()
{
    TAG_LOGI(AceLogTag::ACE_FOCUS, "Focus view: %{public}s/%{public}d hide", GetFrameName().c_str(), GetFrameId());
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto focusManager = pipeline->GetFocusManager();
    CHECK_NULL_VOID(focusManager);
    focusManager->FocusViewHide(AceType::Claim(this));
    pipeline->RequestFrame();
}

void FocusView::FocusViewClose()
{
    TAG_LOGI(AceLogTag::ACE_FOCUS, "Focus view: %{public}s/%{public}d close", GetFrameName().c_str(), GetFrameId());
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto focusManager = pipeline->GetFocusManager();
    CHECK_NULL_VOID(focusManager);
    focusManager->FocusViewClose(AceType::Claim(this));
    pipeline->RequestFrame();
}

RefPtr<FrameNode> FocusView::GetFrameNode()
{
    auto focusViewPattern = AceType::DynamicCast<Pattern>(AceType::Claim(this));
    CHECK_NULL_RETURN(focusViewPattern, nullptr);
    return focusViewPattern->GetHost();
}

std::string FocusView::GetFrameName()
{
    auto focusViewPattern = AceType::DynamicCast<Pattern>(AceType::Claim(this));
    CHECK_NULL_RETURN(focusViewPattern, "NULL");
    auto focusViewFrame = focusViewPattern->GetHost();
    CHECK_NULL_RETURN(focusViewFrame, "NULL");
    return focusViewFrame->GetTag();
}

int32_t FocusView::GetFrameId()
{
    auto focusViewPattern = AceType::DynamicCast<Pattern>(AceType::Claim(this));
    CHECK_NULL_RETURN(focusViewPattern, -1);
    auto focusViewFrame = focusViewPattern->GetHost();
    CHECK_NULL_RETURN(focusViewFrame, -1);
    return focusViewFrame->GetId();
}

void FocusView::LostViewFocus()
{
    TAG_LOGI(
        AceLogTag::ACE_FOCUS, "Focus view: %{public}s/%{public}d lost focus", GetFrameName().c_str(), GetFrameId());
    auto focusViewHub = GetFocusHub();
    CHECK_NULL_VOID(focusViewHub);
    if (focusViewHub->IsCurrentFocus()) {
        focusViewHub->LostFocus(BlurReason::VIEW_SWITCH);
    }
}

RefPtr<FocusHub> FocusView::GetFocusHub()
{
    auto focusViewPattern = AceType::DynamicCast<Pattern>(AceType::Claim(this));
    CHECK_NULL_RETURN(focusViewPattern, nullptr);
    auto focusViewFrame = focusViewPattern->GetHost();
    CHECK_NULL_RETURN(focusViewFrame, nullptr);
    auto focusViewHub = focusViewFrame->GetFocusHub();
    return focusViewHub;
}

RefPtr<FocusView> FocusView::GetCurrentFocusView()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto focusManager = pipeline->GetFocusManager();
    CHECK_NULL_RETURN(focusManager, nullptr);
    return focusManager->GetLastFocusView().Upgrade();
}

RefPtr<FocusView> FocusView::GetEntryFocusView()
{
    if (IsEntryFocusView()) {
        return AceType::Claim(this);
    }
    auto focusViewHub = GetFocusHub();
    auto parentFocusHub = focusViewHub ? focusViewHub->GetParentFocusHub() : nullptr;
    while (parentFocusHub) {
        auto parentFrame = parentFocusHub->GetFrameNode();
        auto parentFocusView = parentFrame ? parentFrame->GetPattern<FocusView>() : nullptr;
        if (!parentFocusView) {
            parentFocusHub = parentFocusHub->GetParentFocusHub();
            continue;
        }
        if (parentFocusView->IsEntryFocusView()) {
            return parentFocusView;
        }
        parentFocusHub = parentFocusHub->GetParentFocusHub();
    }
    return AceType::Claim(this);
}

RefPtr<FocusHub> FocusView::GetViewRootScope()
{
    auto rootScopeSpecified = rootScopeSpecified_.Upgrade();
    if (rootScopeSpecified) {
        return rootScopeSpecified;
    }
    auto focusViewPattern = AceType::DynamicCast<Pattern>(AceType::Claim(this));
    CHECK_NULL_RETURN(focusViewPattern, nullptr);
    auto focusViewFrame = focusViewPattern->GetHost();
    CHECK_NULL_RETURN(focusViewFrame, nullptr);
    auto focusViewHub = focusViewFrame->GetFocusHub();
    CHECK_NULL_RETURN(focusViewHub, nullptr);
    std::list<int32_t> rootScopeDeepth = GetRouteOfFirstScope();
    RefPtr<FocusHub> rootScope = focusViewHub;
    for (const auto& index : rootScopeDeepth) {
        CHECK_NULL_RETURN(rootScope, focusViewHub);
        auto children = rootScope->GetChildren();
        auto iter = children.begin();
        std::advance(iter, index);
        if (iter == children.end()) {
            TAG_LOGD(AceLogTag::ACE_FOCUS, "Index: %{public}d of %{public}s/%{public}d 's children is invalid.", index,
                rootScope->GetFrameName().c_str(), rootScope->GetFrameId());
            return focusViewHub;
        }
        rootScope = *iter;
    }
    CHECK_NULL_RETURN(rootScope, nullptr);
    auto pipeline = PipelineContext::GetCurrentContext();
    auto screenNode = pipeline ? pipeline->GetScreenNode() : nullptr;
    auto screenFocusHub = screenNode ? screenNode->GetFocusHub() : nullptr;
    if (rootScope->GetFocusType() != FocusType::SCOPE || (screenFocusHub && rootScope == screenFocusHub)) {
        rootScope = rootScope->GetParentFocusHub();
    }
    if (rootScope != focusViewHub) {
        focusViewHub->SetFocusDependence(FocusDependence::AUTO);
    }
    return rootScope;
}

void FocusView::SetIsViewRootScopeFocused(bool isViewRootScopeFocused)
{
    isViewRootScopeFocused_ = isViewRootScopeFocused;
    auto viewRootScope = GetViewRootScope();
    CHECK_NULL_VOID(viewRootScope);
    viewRootScope->SetFocusDependence(isViewRootScopeFocused ? FocusDependence::SELF : FocusDependence::AUTO);
}

bool FocusView::IsRootScopeCurrentFocus()
{
    auto viewRootScope = GetViewRootScope();
    return viewRootScope ? viewRootScope->IsCurrentFocus() : false;
}

bool FocusView::IsChildFocusViewOf(const RefPtr<FocusView>& parent)
{
    auto focusViewHub = GetFocusHub();
    auto parentFocusHub = focusViewHub ? focusViewHub->GetParentFocusHub() : nullptr;
    while (parentFocusHub) {
        auto parentFrame = parentFocusHub->GetFrameNode();
        auto parentFocusView = parentFrame ? parentFrame->GetPattern<FocusView>() : nullptr;
        if (!parentFocusView) {
            parentFocusHub = parentFocusHub->GetParentFocusHub();
            continue;
        }
        if (parentFocusView == parent) {
            return true;
        }
        parentFocusHub = parentFocusHub->GetParentFocusHub();
    }
    return false;
}

bool FocusView::HasParentFocusHub()
{
    auto focusViewHub = GetFocusHub();
    CHECK_NULL_RETURN(focusViewHub, false);
    return focusViewHub->GetParentFocusHub() != nullptr;
}

bool FocusView::RequestDefaultFocus()
{
    TAG_LOGI(AceLogTag::ACE_FOCUS, "Request focus on focus view: %{public}s/%{public}d.", GetFrameName().c_str(),
        GetFrameId());
    auto focusViewHub = GetFocusHub();
    CHECK_NULL_RETURN(focusViewHub, false);
    if (focusViewHub->GetFocusType() != FocusType::SCOPE || !focusViewHub->IsFocusableNode()) {
        return false;
    }
    auto viewRootScope = GetViewRootScope();
    CHECK_NULL_RETURN(viewRootScope, false);
    isViewHasFocused_ = true;
    auto isViewRootScopeHasChildFocused = viewRootScope->HasFocusedChild();
    if (!isDefaultHasBeFocused_ && !isViewRootScopeHasChildFocused) {
        auto defaultFocusNode = focusViewHub->GetChildFocusNodeByType(FocusNodeType::DEFAULT);
        if (!defaultFocusNode) {
            TAG_LOGI(AceLogTag::ACE_FOCUS, "Focus view has no default focus.");
        } else if (!defaultFocusNode->IsFocusableWholePath()) {
            TAG_LOGI(AceLogTag::ACE_FOCUS, "Default focus node: %{public}s/%{public}d is not focusable",
                defaultFocusNode->GetFrameName().c_str(), defaultFocusNode->GetFrameId());
        } else {
            SetIsViewRootScopeFocused(false);
            auto ret = defaultFocusNode->RequestFocusImmediately();
            isDefaultHasBeFocused_ = true;
            TAG_LOGI(AceLogTag::ACE_FOCUS, "Request focus on default focus: %{public}s/%{public}d return: %{public}d.",
                defaultFocusNode->GetFrameName().c_str(), defaultFocusNode->GetFrameId(), ret);
            return ret;
        }
    }
    if (isViewRootScopeFocused_ && viewRootScope) {
        SetIsViewRootScopeFocused(true);
        auto ret = viewRootScope->RequestFocusImmediately();
        TAG_LOGI(AceLogTag::ACE_FOCUS, "Request focus on view root scope: %{public}s/%{public}d return: %{public}d.",
            viewRootScope->GetFrameName().c_str(), viewRootScope->GetFrameId(), ret);
        return ret;
    }
    SetIsViewRootScopeFocused(false);
    bool ret = false;
    if (focusViewHub->IsCurrentFocus()) {
        focusViewHub->InheritFocus();
        ret = true;
    } else {
        ret = focusViewHub->RequestFocusImmediately();
    }
    TAG_LOGI(AceLogTag::ACE_FOCUS, "Request focus on focus view return: %{public}d.", ret);
    return ret;
}

bool FocusView::TriggerFocusMove()
{
    TAG_LOGI(AceLogTag::ACE_FOCUS, "Focus view: %{public}s/%{public}d trigger focus move.", GetFrameName().c_str(),
        GetFrameId());
    auto viewRootScope = GetViewRootScope();
    CHECK_NULL_RETURN(viewRootScope, false);
    if (!viewRootScope->IsCurrentFocus()) {
        TAG_LOGI(AceLogTag::ACE_FOCUS,
            "Current view root: %{public}s/%{public}d is not on focusing. Cannot trigger focus move.",
            viewRootScope->GetFrameName().c_str(), viewRootScope->GetFrameId());
        return false;
    }
    if (viewRootScope->GetFocusDependence() != FocusDependence::SELF) {
        TAG_LOGI(AceLogTag::ACE_FOCUS,
            "Current view root: %{public}s/%{public}d is not focus depend self. Do not trigger focus move.",
            viewRootScope->GetFrameName().c_str(), viewRootScope->GetFrameId());
        return false;
    }

    auto viewFocusHub = GetFocusHub();
    CHECK_NULL_RETURN(viewFocusHub, false);
    TabIndexNodeList tabIndexNodes;
    tabIndexNodes.clear();
    viewFocusHub->CollectTabIndexNodes(tabIndexNodes);
    if (tabIndexNodes.empty()) {
        // No tabIndex node in current main view. Extend focus from viewRootScope to children.
        isDefaultHasBeFocused_ = true;
        SetIsViewRootScopeFocused(false);
        viewRootScope->InheritFocus();
        return true;
    }

    // First tabIndex node need get focus.
    tabIndexNodes.sort([](std::pair<int32_t, WeakPtr<FocusHub>>& a, std::pair<int32_t, WeakPtr<FocusHub>>& b) {
        return a.first < b.first;
    });
    return viewFocusHub->GoToFocusByTabNodeIdx(tabIndexNodes, 0);
}
} // namespace OHOS::Ace::NG
