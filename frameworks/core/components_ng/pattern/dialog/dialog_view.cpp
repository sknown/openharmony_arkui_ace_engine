/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "core/components_ng/pattern/dialog/dialog_view.h"

#include <string>

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/dialog/dialog_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
RefPtr<FrameNode> DialogView::CreateDialogNode(
    const DialogProperties& param, const RefPtr<UINode>& customNode = nullptr)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto dialogTheme = pipeline->GetTheme<DialogTheme>();
    CHECK_NULL_RETURN(dialogTheme, nullptr);

    std::string tag;
    switch (param.type) {
        case DialogType::ALERT_DIALOG: {
            tag = V2::ALERT_DIALOG_ETS_TAG;
            break;
        }
        case DialogType::ACTION_SHEET: {
            tag = V2::ACTION_SHEET_DIALOG_ETS_TAG;
            break;
        }
        default:
            tag = V2::DIALOG_ETS_TAG;
            break;
    }
    auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", tag.c_str(), nodeId);
    RefPtr<FrameNode> dialog = FrameNode::CreateFrameNode(tag, nodeId,
        AceType::MakeRefPtr<DialogPattern>(dialogTheme, customNode));
    if (customNode) {
        customNode->Build(nullptr);
    }
    // update layout and render props
    auto dialogLayoutProp = AceType::DynamicCast<DialogLayoutProperty>(dialog->GetLayoutProperty());
    CHECK_NULL_RETURN(dialogLayoutProp, dialog);
    DialogAlignment align = static_cast<DialogAlignment>(dialogTheme->GetAlignDialog());
    if (param.alignment != DialogAlignment::DEFAULT && align == DialogAlignment::CENTER) {
        dialogLayoutProp->UpdateDialogAlignment(align);
    } else {
        dialogLayoutProp->UpdateDialogAlignment(param.alignment);
    }
    dialogLayoutProp->UpdateDialogOffset(param.offset);
    dialogLayoutProp->UpdateUseCustomStyle(param.customStyle);
    dialogLayoutProp->UpdateAutoCancel(param.autoCancel);
    dialogLayoutProp->UpdateShowInSubWindow(param.isShowInSubWindow);
    dialogLayoutProp->UpdateDialogButtonDirection(param.buttonDirection);
    dialogLayoutProp->UpdateIsModal(param.isModal);
    dialogLayoutProp->UpdateIsScenceBoardDialog(param.isScenceBoardDialog);
    if (param.width.has_value() && NonNegative(param.width.value().Value())) {
        dialogLayoutProp->UpdateWidth(param.width.value());
    } else {
        dialogLayoutProp->UpdateGridCount(param.gridCount);
    }
    if (param.height.has_value() && NonNegative(param.height.value().Value())) {
        dialogLayoutProp->UpdateHeight(param.height.value());
    }
    // create gray background
    auto dialogContext = dialog->GetRenderContext();
    CHECK_NULL_RETURN(dialogContext, dialog);
    auto pattern = dialog->GetPattern<DialogPattern>();
    CHECK_NULL_RETURN(pattern, dialog);
    pattern->SetDialogProperties(param);

    auto isSubWindow = dialogLayoutProp->GetShowInSubWindowValue(false) && !pattern->IsUIExtensionSubWindow();
    if ((isSubWindow && dialogLayoutProp->GetIsModal().value_or(true)) ||
        !dialogLayoutProp->GetIsModal().value_or(true)) {
        dialogContext->UpdateBackgroundColor(Color(0x00000000));
    } else {
        dialogContext->UpdateBackgroundColor(param.maskColor.value_or(dialogTheme->GetMaskColorEnd()));
    }
    if (dialogLayoutProp->GetIsScenceBoardDialog().value_or(false)) {
        dialogContext->UpdateBackgroundColor(param.maskColor.value_or(dialogTheme->GetMaskColorEnd()));
    }
    // set onCancel callback
    auto hub = dialog->GetEventHub<DialogEventHub>();
    CHECK_NULL_RETURN(hub, dialog);
    hub->SetOnCancel(param.onCancel);
    hub->SetOnSuccess(param.onSuccess);

    pattern->BuildChild(param);
    pattern->SetOnWillDismiss(param.onWillDismiss);

    if (param.transitionEffect != nullptr) {
        dialogContext->UpdateChainedTransition(param.transitionEffect);
    } else {
        // set open and close animation
        pattern->SetOpenAnimation(param.openAnimation);
        pattern->SetCloseAnimation(param.closeAnimation);
    }

    dialog->MarkModifyDone();
    return dialog;
}

} // namespace OHOS::Ace::NG
