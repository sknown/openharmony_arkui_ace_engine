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

#include "bridge/cj_frontend/cppview/native_view.h"

#include <cinttypes>
#include <utility>

#include "base/log/ace_trace.h"
#include "bridge/cj_frontend/runtime/cj_runtime_delegate.h"
#include "core/components_ng/pattern/custom/custom_measure_layout_node.h"
#include "core/components_ng/base/view_stack_model.h"
#include "core/components_ng/base/view_partial_update_model.h"

namespace OHOS::Ace::Framework {

std::string GetProcessViewId(int64_t id)
{
    return ViewStackModel::GetInstance()->ProcessViewId(std::to_string(id));
}

NativeView::NativeView(sptr<RemoteView> cjView) : cjView_(std::move(cjView))
{
    LOGD("Native View constructed: %{public}" PRId64 ".", GetID());
    useNewPipeline_ = Container::IsCurrentUseNewPipeline();
    instanceId_ = Container::CurrentId();
}

NativeView::~NativeView()
{
    LOGD("Native View Destroyed: %{public}" PRId64 ".", GetID());
};

void NativeView::GetDeletedElemtIds(std::vector<int64_t>& vec)
{
    LOGD("NativeView, getting elmtIds of all deleted Elements from ElementRegister:");
}

void NativeView::DeletedElmtIdsHaveBeenPurged(std::vector<int64_t>& vec)
{
    LOGD("NativeView, getting elmtIds of all deleted Elements from ElementRegister:");
}

RefPtr<AceType> NativeView::CreateUI()
{
    wptr<NativeView> weakThis = this;
    NodeInfoPU partialUpdateCallbacks {
        .completeReloadFunc = [weakThis]()-> RefPtr<AceType> {
            auto view = weakThis.promote();
            CHECK_NULL_RETURN(view, nullptr);
            ContainerScope scope(view->instanceId_);
            return view->InitialUIRender();
        },
        .appearFunc = [weakThis]() -> void {
            auto self = weakThis.promote();
            CHECK_NULL_VOID(self);
            ContainerScope scope(self->instanceId_);
            self->cjView_->OnAppear();
        },
        .renderFunc = [weakThis]() -> RefPtr<AceType> {
            auto self = weakThis.promote();
            CHECK_NULL_RETURN(self, nullptr);
            ContainerScope scope(self->instanceId_);
            if (!self->isFirstRender_) {
                LOGW("the view has already called initial render");
                return nullptr;
            }
            self->isFirstRender_ = false;
            return self->InitialUIRender();
        },
        .updateFunc = [weakThis]() -> void {
            auto self = weakThis.promote();
            CHECK_NULL_VOID(self);
            ContainerScope scope(self->instanceId_);
            if (!self->needsUpdate_) {
                LOGW("the view does not need to update");
                return;
            }
            self->needsUpdate_ = false;
            self->cjView_->Rerender();
        },
        .removeFunc = [weakThis]() {
            auto self = weakThis.promote();
            CHECK_NULL_VOID(self);
            self->Destroy();
        },
    };
    auto node = ViewPartialUpdateModel::GetInstance()->CreateNode(std::move(partialUpdateCallbacks));

    node_ = node;

    return node;
}

RefPtr<AceType> NativeView::InitialUIRender()
{
    needsUpdate_ = false;
    {
        cjView_->OnAboutToRender();
    }
    {
        cjView_->Render();
    }
    {
        cjView_->OnAfterRender();
        if (onRenderDone_) {
            onRenderDone_();
        }
    }
    return ViewStackModel::GetInstance()->Finish();
}

void NativeView::SyncInstanceId()
{
    restoreInstanceId_ = Container::CurrentId();
    ContainerScope::UpdateCurrent(instanceId_);
}

void NativeView::RestoreInstanceId()
{
    ContainerScope::UpdateCurrent(restoreInstanceId_);
}

/**
 * marks the NativeView's composed component as needing update / rerender
 */
void NativeView::MarkNeedUpdate()
{
    auto node = node_.Upgrade();
    if (!node) {
        LOGE("fail to update due to custom Node is null");
        return;
    }
    if (AceType::InstanceOf<NG::CustomNode>(node)) {
        auto customNode = AceType::DynamicCast<NG::CustomNode>(node);
        needsUpdate_ = true;
        customNode->MarkNeedUpdate();
    } else if (AceType::InstanceOf<NG::CustomMeasureLayoutNode>(node)) {
        auto customNode = AceType::DynamicCast<NG::CustomMeasureLayoutNode>(node);
        needsUpdate_ = true;
        customNode->MarkNeedUpdate();
    }
}

void NativeView::FlushReload()
{
    auto node = node_.Upgrade();
    if (!node) {
        LOGE("fail to update due to custom Node is null");
        return;
    }
    if (AceType::InstanceOf<NG::CustomNode>(node)) {
        auto customNode = AceType::DynamicCast<NG::CustomNode>(node);
        customNode->FlushReload();
    }
}

void NativeView::FinishUpdateFunc(int32_t elmtId)
{
    LOGI("FinishUpdateFunc start");
    NG::ViewStackProcessor::GetInstance()->FlushRerenderTask();
    LOGI("FinishUpdateFunc finish");
    return;
}


void NativeView::Destroy()
{
    if (!cjView_) {
        LOGE("NativeView::Destroy error, nativeId: %{public}" PRId64 " cj view not exist.", GetID());
        return;
    }
    LOGD("NativeView::Destroy start, nativeId: %{public}" PRId64 ", cjId: %{public}" PRId64, GetID(), cjView_->GetID());
    {
        cjView_->OnDisappear();
    }
    {
        cjView_->OnAboutToBeDeleted();
    }
    LOGD("NativeView::Destroy end");
}

void NativeView::Create(const sptr<NativeView>& view)
{
    ViewStackModel::GetInstance()->Push(view->CreateUI(), true);
}

void NativeView::CleanUpAbandonedChild()
{
    LOGD("NativeView::CleanUpAbandonedChild");
}

void NativeView::FireOnShow()
{
    if (!cjView_) {
        LOGE("NativeView::FireOnShow fail, no cj view on %{public}" PRId64 ".", GetID());
        return;
    }
    {
        cjView_->OnShow();
    }
}

void NativeView::FireOnHide()
{
    if (!cjView_) {
        LOGE("NativeView::FireOnHide fail, no cj view on %{public}" PRId64 ".", GetID());
        return;
    }
    {
        cjView_->OnHide();
    }
}

bool NativeView::FireOnBackPress()
{
    if (!cjView_) {
        LOGE("NativeView::FireOnBackPress fail, no cj view on %{public}" PRId64 ".", GetID());
        return false;
    }
    {
        return cjView_->OnBackPress();
    }
}

void NativeView::FireOnTransition()
{
    if (!cjView_) {
        LOGE("NativeView::FireOnTransition fail, no cj view on %{public}" PRId64 ".", GetID());
        return;
    }
    cjView_->OnTransition();
}

void NativeView::ExecuteUpdateWithValueParams(const std::string& jsonData)
{
    if (!cjView_) {
        LOGE("NativeView::ExecuteUpdateWithValueParams fail, no cj view on %{public}" PRId64 ".", GetID());
        return;
    }
    cjView_->UpdateWithJson(jsonData);
}

void RemoteView::VoidCallback(void (*cjFunc)(int64_t), const char* funcName)
{
    if (!cjFunc) {
        LOGE("CJFunc: RemoteView::%{public}s is empty.", funcName);
        return;
    }
    cjFunc(GetID());
}

void RemoteView::Render()
{
    VoidCallback(CJRuntimeDelegate::GetInstance()->GetCJFuncs().atCOHOSAceFrameworkRemoteViewRender, "Render");
}

void RemoteView::Rerender()
{
    VoidCallback(CJRuntimeDelegate::GetInstance()->GetCJFuncs().atCOHOSAceFrameworkRemoteViewRerender, "Rerender");
}

void RemoteView::OnShow()
{
    VoidCallback(CJRuntimeDelegate::GetInstance()->GetCJFuncs().atCOHOSAceFrameworkRemoteViewOnShow, "OnShow");
}

void RemoteView::OnHide()
{
    VoidCallback(CJRuntimeDelegate::GetInstance()->GetCJFuncs().atCOHOSAceFrameworkRemoteViewOnHide, "OnHide");
}

bool RemoteView::OnBackPress()
{
    auto onBackPressFunc = CJRuntimeDelegate::GetInstance()->GetCJFuncs().atCOHOSAceFrameworkRemoteViewOnBackPress;
    if (!onBackPressFunc) {
        LOGE("CJFunc: RemoteView::OnBackPress is empty.");
        return false;
    }
    return onBackPressFunc(GetID());
}

void RemoteView::UpdateWithJson(const std::string& value)
{
    auto updateWithJsonFunc =
        CJRuntimeDelegate::GetInstance()->GetCJFuncs().atCOHOSAceFrameworkRemoteViewUpdateWithJson;
    if (!updateWithJsonFunc) {
        LOGE("CJFunc: RemoteView::UpdateWithJson is empty.");
        return;
    }
    updateWithJsonFunc(GetID(), value.c_str());
}

void RemoteView::OnAppear()
{
    VoidCallback(CJRuntimeDelegate::GetInstance()->GetCJFuncs().atCOHOSAceFrameworkRemoteViewOnAppear, "OnAppear");
}

void RemoteView::OnTransition()
{
    VoidCallback(
        CJRuntimeDelegate::GetInstance()->GetCJFuncs().atCOHOSAceFrameworkRemoteViewOnTransition, "OnTransition");
}

void RemoteView::OnAboutToRender()
{
    VoidCallback(
        CJRuntimeDelegate::GetInstance()->GetCJFuncs().atCOHOSAceFrameworkRemoteViewOnAboutToRender, "OnAboutToRender");
}

void RemoteView::OnAboutToBeDeleted()
{
    VoidCallback(CJRuntimeDelegate::GetInstance()->GetCJFuncs().atCOHOSAceFrameworkRemoteViewOnAboutToBeDeleted,
        "OnAboutToBeDeleted");
}

void RemoteView::OnAfterRender()
{
    VoidCallback(
        CJRuntimeDelegate::GetInstance()->GetCJFuncs().atCOHOSAceFrameworkRemoteViewOnAfterRender, "OnAfterRender");
}

void RemoteView::OnDisappear()
{
    VoidCallback(
        CJRuntimeDelegate::GetInstance()->GetCJFuncs().atCOHOSAceFrameworkRemoteViewOnDisappear, "OnDisappear");
}

} // namespace OHOS::Ace::Framework
