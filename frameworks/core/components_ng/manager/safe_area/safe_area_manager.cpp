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
#include "safe_area_manager.h"

#include "base/utils/utils.h"
#include "core/components/container_modal/container_modal_constants.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_ng/pattern/stage/page_pattern.h"
#include "core/components_ng/property/safe_area_insets.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
bool SafeAreaManager::UpdateCutoutSafeArea(const SafeAreaInsets& safeArea)
{
    // cutout regions adjacent to edges.
    auto cutoutArea = safeArea;

    if (cutoutArea.top_.IsValid()) {
        cutoutArea.top_.start = 0;
    }
    if (safeArea.bottom_.IsValid()) {
        cutoutArea.bottom_.end = PipelineContext::GetCurrentRootHeight();
    }
    if (cutoutArea.left_.IsValid()) {
        cutoutArea.left_.start = 0;
    }
    if (cutoutArea.right_.IsValid()) {
        cutoutArea.right_.end = PipelineContext::GetCurrentRootWidth();
    }

    if (cutoutSafeArea_ == cutoutArea) {
        return false;
    }
    cutoutSafeArea_ = cutoutArea;
    return true;
}

bool SafeAreaManager::UpdateSystemSafeArea(const SafeAreaInsets& safeArea)
{
    if (systemSafeArea_ == safeArea) {
        return false;
    }
    systemSafeArea_ = safeArea;
    return true;
}

bool SafeAreaManager::UpdateNavArea(const SafeAreaInsets& safeArea)
{
    if (navSafeArea_ == safeArea) {
        return false;
    }
    navSafeArea_ = safeArea;
    return true;
}

bool SafeAreaManager::UpdateKeyboardSafeArea(float keyboardHeight)
{
    uint32_t bottom;
    if (systemSafeArea_.bottom_.IsValid()) {
        bottom = systemSafeArea_.bottom_.start;
    } else {
        bottom = PipelineContext::GetCurrentRootHeight();
    }
    SafeAreaInsets::Inset inset = { .start = bottom - keyboardHeight, .end = bottom };
    if (inset == keyboardInset_) {
        return false;
    }
    keyboardInset_ = inset;
    return true;
}

SafeAreaInsets SafeAreaManager::GetCombinedSafeArea(const SafeAreaExpandOpts& opts) const
{
    SafeAreaInsets res;
#ifdef PREVIEW
    if (ignoreSafeArea_) {
        return res;
    }
#else
    if (ignoreSafeArea_ || (!isFullScreen_ && !isNeedAvoidWindow_)) {
        return res;
    }
#endif
    if (opts.type & SAFE_AREA_TYPE_CUTOUT) {
        res = res.Combine(cutoutSafeArea_);
    }
    if (opts.type & SAFE_AREA_TYPE_SYSTEM) {
        res = res.Combine(systemSafeArea_).Combine(navSafeArea_);
    }
    if (keyboardSafeAreaEnabled_ && (opts.type & SAFE_AREA_TYPE_KEYBOARD)) {
        res.bottom_ = res.bottom_.Combine(keyboardInset_);
    }
    return res;
}

bool SafeAreaManager::IsSafeAreaValid() const
{
#ifdef PREVIEW
    return !ignoreSafeArea_;
#else
    return !(ignoreSafeArea_ || (!isFullScreen_ && !isNeedAvoidWindow_));
#endif
}

bool SafeAreaManager::SetIsFullScreen(bool value)
{
    if (isFullScreen_ == value) {
        return false;
    }
    isFullScreen_ = value;
    return true;
}

bool SafeAreaManager::SetIsNeedAvoidWindow(bool value)
{
    if (isNeedAvoidWindow_ == value) {
        return false;
    }
    isNeedAvoidWindow_ = value;
    return true;
}

bool SafeAreaManager::SetIgnoreSafeArea(bool value)
{
    if (ignoreSafeArea_ == value) {
        return false;
    }
    ignoreSafeArea_ = value;
    return true;
}

bool SafeAreaManager::SetKeyBoardAvoidMode(bool value)
{
    if (keyboardSafeAreaEnabled_ == value) {
        return false;
    }
    keyboardSafeAreaEnabled_ = value;
    return true;
}

bool SafeAreaManager::SetIsAtomicService(bool value)
{
    if (isAtomicService_ == value) {
        return false;
    }
    isAtomicService_ = value;
    return true;
}

bool SafeAreaManager::IsAtomicService() const
{
    return isAtomicService_;
}

SafeAreaInsets SafeAreaManager::GetSystemSafeArea() const
{
    return systemSafeArea_;
}

SafeAreaInsets SafeAreaManager::GetCutoutSafeArea() const
{
    if (ignoreSafeArea_ || !isFullScreen_) {
        return {};
    }
    return cutoutSafeArea_;
}

SafeAreaInsets SafeAreaManager::GetSafeArea() const
{
#ifdef PREVIEW
    if (ignoreSafeArea_) {
        return {};
    }
#else
    if (ignoreSafeArea_ || (!isFullScreen_ && !isNeedAvoidWindow_)) {
        return {};
    }
#endif
    return systemSafeArea_.Combine(cutoutSafeArea_).Combine(navSafeArea_);
}

SafeAreaInsets SafeAreaManager::GetSafeAreaWithoutProcess() const
{
    return systemSafeArea_.Combine(cutoutSafeArea_).Combine(navSafeArea_);
}

float SafeAreaManager::GetKeyboardOffset() const
{
    if (keyboardSafeAreaEnabled_) {
        return 0.0f;
    }
    return keyboardOffset_;
}

OffsetF SafeAreaManager::GetWindowWrapperOffset()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, OffsetF());
    auto windowManager = pipelineContext->GetWindowManager();
    auto isContainerModal = pipelineContext->GetWindowModal() == WindowModal::CONTAINER_MODAL && windowManager &&
                            windowManager->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING;
    if (isContainerModal) {
        auto wrapperOffset = OffsetF(static_cast<float>((CONTAINER_BORDER_WIDTH + CONTENT_PADDING).ConvertToPx()),
            static_cast<float>((pipelineContext->GetCustomTitleHeight() + CONTAINER_BORDER_WIDTH).ConvertToPx()));
        return wrapperOffset;
    }
    return OffsetF();
}

void SafeAreaManager::ExpandSafeArea()
{
    ACE_LAYOUT_SCOPED_TRACE("ExpandSafeArea node count %zu", needExpandNodes_.size());
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto manager = pipeline->GetSafeAreaManager();
    bool isFocusOnPage = pipeline->CheckPageFocus();
    auto iter = needExpandNodes_.begin();
    while (iter != needExpandNodes_.end()) {
        auto frameNode = (*iter).Upgrade();
        if (frameNode) {
            manager->AddGeoRestoreNode(frameNode);
            frameNode->ExpandSafeArea(isFocusOnPage);
        }
        ++iter;
    }
    ClearNeedExpandNode();
}

bool SafeAreaManager::AddNodeToExpandListIfNeeded(const WeakPtr<FrameNode>& node)
{
    if (needExpandNodes_.find(node) == needExpandNodes_.end()) {
        AddNeedExpandNode(node);
        return true;
    }
    return false;
}

bool SafeAreaManager::CheckPageNeedAvoidKeyboard(const RefPtr<FrameNode>& frameNode)
{
    if (frameNode->GetTag() != V2::PAGE_ETS_TAG) {
        return false;
    }
    // page will not avoid keyboard when lastChild is sheet
    RefPtr<OverlayManager> overlay;
    if (frameNode->RootNodeIsPage()) {
        auto pattern = frameNode->GetPattern<PagePattern>();
        CHECK_NULL_RETURN(pattern, true);
        overlay = pattern->GetOverlayManager();
    } else {
        auto navNode = FrameNode::GetFrameNode(V2::NAVDESTINATION_VIEW_ETS_TAG, frameNode->GetRootNodeId());
        CHECK_NULL_RETURN(navNode, true);
        auto pattern = navNode->GetPattern<NavDestinationPattern>();
        CHECK_NULL_RETURN(pattern, true);
        overlay = pattern->GetOverlayManager();
    }
    CHECK_NULL_RETURN(overlay, true);
    return overlay->CheckPageNeedAvoidKeyboard();
}
} // namespace OHOS::Ace::NG
