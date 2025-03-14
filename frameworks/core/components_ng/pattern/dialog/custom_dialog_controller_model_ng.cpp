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
#include "core/components_ng/pattern/dialog/custom_dialog_controller_model_ng.h"

#include "base/memory/ace_type.h"
#include "base/subwindow/subwindow_manager.h"
#include "base/thread/task_executor.h"
#include "core/common/container_scope.h"

namespace OHOS::Ace::NG {
void CustomDialogControllerModelNG::SetOpenDialog(DialogProperties& dialogProperties,
    const WeakPtr<AceType>& controller, std::vector<WeakPtr<AceType>>& dialogs,
    bool& pending, bool& isShown, std::function<void()>&& cancelTask, std::function<void()>&& buildFunc,
    RefPtr<AceType>& dialogComponent, RefPtr<AceType>& customDialog, std::list<DialogOperation>& dialogOperation)
{
    auto container = Container::Current();
    auto currentId = Container::CurrentId();
    CHECK_NULL_VOID(container);
    if (container->IsSubContainer() && !dialogProperties.isShowInSubWindow) {
        currentId = SubwindowManager::GetInstance()->GetParentContainerId(Container::CurrentId());
        container = AceEngine::Get().GetContainer(currentId);
        CHECK_NULL_VOID(container);
    }
    ContainerScope scope(currentId);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    auto context = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);

    dialogProperties.onStatusChanged = [&isShown](bool isShownStatus) {
        if (!isShownStatus) {
            isShown = isShownStatus;
        }
    };

    auto executor = context->GetTaskExecutor();
    CHECK_NULL_VOID(executor);
    auto task = [currentId, controller, &dialogProperties, &dialogs, func = std::move(buildFunc),
                    weakOverlayManager = AceType::WeakClaim(AceType::RawPtr(overlayManager))]() mutable {
        ContainerScope scope(currentId);
        RefPtr<NG::FrameNode> dialog;
        auto overlayManager = weakOverlayManager.Upgrade();
        CHECK_NULL_VOID(overlayManager);
        auto controllerPtr = controller.Upgrade();
        CHECK_NULL_VOID(controllerPtr);
        auto container = Container::Current();
        CHECK_NULL_VOID(container);
        if (dialogProperties.isShowInSubWindow) {
            dialog = SubwindowManager::GetInstance()->ShowDialogNG(dialogProperties, std::move(func));
            if (dialogProperties.isModal && !dialogProperties.isScenceBoardDialog &&
                !container->IsUIExtensionWindow()) {
                auto mask = overlayManager->SetDialogMask(dialogProperties);
                CHECK_NULL_VOID(mask);
                overlayManager->SetMaskNodeId(dialog->GetId(), mask->GetId());
            }
        } else {
            dialog = overlayManager->ShowDialog(dialogProperties, std::move(func), false);
        }
        CHECK_NULL_VOID(dialog);
        dialogs.emplace_back(dialog);
    };
    executor->PostTask(task, TaskExecutor::TaskType::UI, "ArkUIDialogShowCustomDialog");
}

RefPtr<UINode> CustomDialogControllerModelNG::SetOpenDialogWithNode(DialogProperties& dialogProperties,
    const RefPtr<UINode>& customNode)
{
    ContainerScope scope(Container::CurrentIdSafely());
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, nullptr);
    if (container->IsSubContainer() && !dialogProperties.isShowInSubWindow) {
        auto currentId = SubwindowManager::GetInstance()->GetParentContainerId(Container::CurrentId());
        container = AceEngine::Get().GetContainer(currentId);
        CHECK_NULL_RETURN(container, nullptr);
    }
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_RETURN(pipelineContext, nullptr);
    auto context = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
    CHECK_NULL_RETURN(context, nullptr);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_RETURN(overlayManager, nullptr);
    RefPtr<NG::FrameNode> dialog;
    if (dialogProperties.isShowInSubWindow) {
        dialog = SubwindowManager::GetInstance()->ShowDialogNGWithNode(dialogProperties, customNode);
        if (dialogProperties.isModal && !dialogProperties.isScenceBoardDialog && !container->IsUIExtensionWindow()) {
            DialogProperties Maskarg;
            Maskarg.isMask = true;
            Maskarg.autoCancel = dialogProperties.autoCancel;
            Maskarg.maskColor = dialogProperties.maskColor;
            auto mask = overlayManager->ShowDialogWithNode(Maskarg, nullptr, false);
            CHECK_NULL_RETURN(mask, dialog);
            overlayManager->SetMaskNodeId(dialog->GetId(), mask->GetId());
        }
    } else {
        dialog = overlayManager->ShowDialogWithNode(dialogProperties, customNode, false);
    }
    return dialog;
}

void CustomDialogControllerModelNG::SetCloseDialog(DialogProperties& dialogProperties,
    const WeakPtr<AceType>& controller, std::vector<WeakPtr<AceType>>& dialogs,
    bool& pending, bool& isShown, std::function<void()>&& cancelTask, RefPtr<AceType>& dialogComponent,
    RefPtr<AceType>& customDialog, std::list<DialogOperation>& dialogOperation)
{
    auto container = Container::Current();
    auto currentId = Container::CurrentId();
    CHECK_NULL_VOID(container);
    if (container->IsSubContainer() && !dialogProperties.isShowInSubWindow) {
        currentId = SubwindowManager::GetInstance()->GetParentContainerId(Container::CurrentId());
        container = AceEngine::Get().GetContainer(currentId);
        CHECK_NULL_VOID(container);
    }
    ContainerScope scope(currentId);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    auto context = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    auto executor = context->GetTaskExecutor();
    CHECK_NULL_VOID(executor);
    auto task = [controller, &dialogs, &dialogProperties,
                    weakOverlayManager = AceType::WeakClaim(AceType::RawPtr(overlayManager))]() {
        auto overlayManager = weakOverlayManager.Upgrade();
        CHECK_NULL_VOID(overlayManager);
        auto controllerPtr = controller.Upgrade();
        CHECK_NULL_VOID(controllerPtr);
        RefPtr<NG::FrameNode> dialog;
        while (!dialogs.empty()) {
            dialog = AceType::DynamicCast<NG::FrameNode>(dialogs.back().Upgrade());
            if (dialog && !dialog->IsRemoving()) {
                // get the dialog not removed currently
                break;
            }
            dialogs.pop_back();
        }
        if (dialogs.empty()) {
            return;
        }
        CHECK_NULL_VOID(dialog);
        if (dialogProperties.isShowInSubWindow) {
            SubwindowManager::GetInstance()->CloseDialogNG(dialog);
            dialogs.pop_back();
        } else {
            overlayManager->CloseDialog(dialog);
            dialogs.pop_back();
        }
    };
    executor->PostTask(task, TaskExecutor::TaskType::UI, "ArkUIDialogCloseCustomDialog");
}

void CustomDialogControllerModelNG::SetCloseDialogForNDK(FrameNode* dialogNode)
{
    CHECK_NULL_VOID(dialogNode);
    ContainerScope scope(Container::CurrentIdSafely());
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    auto context = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    auto dialogRef = AceType::Claim(dialogNode);
    overlayManager->CloseDialog(dialogRef);
}
} // namespace OHOS::Ace::NG
