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

#include "bridge/cj_frontend/interfaces/cj_ffi/cj_customdialog_controller_ffi.h"

#include "cj_lambda.h"

#include "bridge/cj_frontend/interfaces/cj_ffi/utils.h"
#include "bridge/common/utils/utils.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/base/memory/referenced.h"
#include "frameworks/bridge/common/utils/engine_helper.h"

using namespace OHOS::Ace;
using namespace OHOS::FFI;
using namespace OHOS::Ace::Framework;

namespace {
const std::vector<DialogAlignment> DIALOG_ALIGNMENT = { DialogAlignment::TOP, DialogAlignment::CENTER,
    DialogAlignment::BOTTOM, DialogAlignment::DEFAULT, DialogAlignment::TOP_START, DialogAlignment::TOP_END,
    DialogAlignment::CENTER_START, DialogAlignment::CENTER_END, DialogAlignment::BOTTOM_START,
    DialogAlignment::BOTTOM_END };
}

namespace OHOS::Ace::Framework {
NativeCustomDialogController::NativeCustomDialogController(NativeCustomDialogControllerOptions options) : FFIData()
{
    WeakPtr<NG::FrameNode> frameNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    cancelFunction_ = CJLambda::Create(options.cancel);
    auto onCancel = [cjCallback = cancelFunction_, node = frameNode]() {
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        pipelineContext->UpdateCurrentActiveNode(node);
        cjCallback();
    };
    dialogProperties_.onCancel = onCancel;
    dialogProperties_.autoCancel = options.autoCancel;
    dialogProperties_.customStyle = options.customStyle;
    dialogProperties_.alignment = DIALOG_ALIGNMENT[options.alignment];

    CalcDimension dx(options.offset.dx.value, static_cast<DimensionUnit>(options.offset.dx.unitType));
    CalcDimension dy(options.offset.dy.value, static_cast<DimensionUnit>(options.offset.dy.unitType));
    dx.ResetInvalidValue();
    dy.ResetInvalidValue();
    dialogProperties_.offset = DimensionOffset(dx, dy);
    if (options.gridCount.hasValue) {
        dialogProperties_.gridCount = options.gridCount.value;
    }
    dialogProperties_.maskColor = Color(options.maskColor);

    Dimension rectX(options.maskRect.x, static_cast<DimensionUnit>(options.maskRect.xUnit));
    Dimension rectY(options.maskRect.y, static_cast<DimensionUnit>(options.maskRect.yUnit));
    Dimension rectWidth(options.maskRect.width, static_cast<DimensionUnit>(options.maskRect.widthUnit));
    Dimension rectHeight(options.maskRect.height, static_cast<DimensionUnit>(options.maskRect.heightUnit));
    DimensionOffset rectOffset(rectX, rectY);
    DimensionRect dimenRect(rectWidth, rectHeight, rectOffset);
    dialogProperties_.maskRect = dimenRect;
    if (options.backgroundColor.hasValue) {
        dialogProperties_.backgroundColor = Color(options.backgroundColor.value);
    }

    NG::BorderRadiusProperty radius;
    CalcDimension radiusCalc(options.cornerRadius.value, static_cast<DimensionUnit>(options.cornerRadius.unitType));
    radius.radiusTopLeft = radiusCalc;
    radius.radiusTopRight = radiusCalc;
    radius.radiusBottomLeft = radiusCalc;
    radius.radiusBottomRight = radiusCalc;
    radius.multiValued = true;
    dialogProperties_.borderRadius = radius;
    if (options.openAnimation.hasValue) {
        AnimationOption openAnimation;
        ParseCjAnimation(options.openAnimation.value, openAnimation);
        dialogProperties_.openAnimation = openAnimation;
    }
    if (options.closeAnimation.hasValue) {
        AnimationOption closeAnimation;
        ParseCjAnimation(options.closeAnimation.value, closeAnimation);
        dialogProperties_.closeAnimation = closeAnimation;
    }
#if defined(PREVIEW)
    LOGW("[Engine Log] Unable to use the SubWindow in the Previewer. Perform this operation on the "
         "emulator or a real device instead.");
#else
    dialogProperties_.isShowInSubWindow = options.showInSubWindow;
#endif
    refself_ = this;
}

void NativeCustomDialogController::OpenDialog()
{
    if (!builderFunction_) {
        return;
    }

    if (!ownerView_) {
        return;
    }
    auto containerId = ownerView_->GetInstanceId();
    ContainerScope containerScope(containerId);
    WeakPtr<NG::FrameNode> frameNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto buildFunc = [builder = builderFunction_, node = frameNode, context = pipelineContext]() {
        {
            context->UpdateCurrentActiveNode(node);
            builder();
        }
    };

    auto cancelTask = ([cancelCallback = cancelFunction_, node = frameNode, context = pipelineContext]() {
        if (cancelCallback) {
            context->UpdateCurrentActiveNode(node);
            cancelCallback();
        }
    });

    auto currentObj = Container::Current();
    if (currentObj && currentObj->IsScenceBoardWindow() && !dialogProperties_.windowScene.Upgrade()) {
        dialogProperties_.isScenceBoardDialog = true;
        auto viewNode = ownerView_->GetViewNode();
        CHECK_NULL_VOID(viewNode);
        auto parentCustom = AceType::DynamicCast<NG::CustomNode>(viewNode);
        CHECK_NULL_VOID(parentCustom);
        auto parent = parentCustom->GetParent();
        while (parent && parent->GetTag() != "WindowScene") {
            parent = parent->GetParent();
        }
        if (parent) {
            dialogProperties_.windowScene = parent;
        }
    }
    dialogProperties_.isSysBlurStyle = false;
    CustomDialogControllerModel::GetInstance()->SetOpenDialog(dialogProperties_, AccessibilityManager::WeakClaim(this),
        dialogs_, pending_, isShown_, std::move(cancelTask), std::move(buildFunc), dialogComponent_, customDialog_,
        dialogOperation_);
    return;
}

void NativeCustomDialogController::CloseDialog()
{
    if (ownerView_ == nullptr) {
        return;
    }
    auto containerId = ownerView_->GetInstanceId();
    ContainerScope containerScope(containerId);

    WeakPtr<NG::FrameNode> frameNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto cancelTask = ([cancelCallback = cancelFunction_, node = frameNode]() {
        if (cancelCallback) {
            auto pipelineContext = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipelineContext);
            pipelineContext->UpdateCurrentActiveNode(node);
            cancelCallback();
        }
    });

    CustomDialogControllerModel::GetInstance()->SetCloseDialog(dialogProperties_, AccessibilityManager::WeakClaim(this),
        dialogs_, pending_, isShown_, std::move(cancelTask), dialogComponent_, customDialog_, dialogOperation_);
}
} // namespace OHOS::Ace::Framework

extern "C" {
int64_t FfiOHOSAceFrameworkCustomDialogControllerCtor(NativeCustomDialogControllerOptions options)
{
    auto controller = FFIData::Create<NativeCustomDialogController>(options);
    return controller->GetID();
}

void FfiOHOSAceFrameworkCustomDialogControllerSetBuilder(int64_t controllerId, void (*builder)())
{
    auto nativeController = FFIData::GetData<NativeCustomDialogController>(controllerId);
    if (nativeController != nullptr) {
        auto builderFunction = CJLambda::Create(builder);
        nativeController->SetBuilder(builderFunction);
    } else {
        LOGE("CustomDialog: invalid CustomDialogController id");
    }
}

void FfiOHOSAceFrameworkCustomDialogControllerBindView(int64_t controllerId, int64_t nativeViewId)
{
    auto nativeView = FFIData::GetData<NativeView>(nativeViewId);
    if (!nativeView) {
        LOGE("FfiOHOSAceFrameworkCustomDialogControllerBindView: invalid nativeView id");
        return;
    }

    auto nativeController = FFIData::GetData<NativeCustomDialogController>(controllerId);
    if (nativeController != nullptr) {
        nativeController->SetView(nativeView);
    } else {
        LOGE("FfiOHOSAceFrameworkCustomDialogControllerBindView: invalid CustomDialogController id");
    }
}

void FfiOHOSAceFrameworkCustomDialogControllerOpen(int64_t id)
{
    auto nativeController = FFIData::GetData<NativeCustomDialogController>(id);
    if (nativeController != nullptr) {
        nativeController->OpenDialog();
    } else {
        LOGE("CustomDialog: invalid CustomDialogController id");
    }
}

void FfiOHOSAceFrameworkCustomDialogControllerClose(int64_t id)
{
    auto nativeController = FFIData::GetData<NativeCustomDialogController>(id);
    if (nativeController != nullptr) {
        nativeController->CloseDialog();
    } else {
        LOGE("CustomDialog: invalid CustomDialogController id");
    }
}
}
