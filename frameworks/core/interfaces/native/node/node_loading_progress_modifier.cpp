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
#include "node_loading_progress_modifier.h"

#include "base/error/error_code.h"
#include "core/components/common/layout/constants.h"
#include "core/components/progress/progress_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/loading_progress/loading_progress_model_ng.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace::NG {
namespace {
ArkUI_Uint32 GetLoadingProgressColor(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, 0x0);
    return LoadingProgressModelNG::GetColor(frameNode);
}

void SetLoadingProgressColor(ArkUINodeHandle node, uint32_t colorValue)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    LoadingProgressModelNG::SetColor(frameNode, Color(colorValue));
}

void ResetLoadingProgressColor(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
        auto pipelineContext = frameNode->GetContext();
        CHECK_NULL_VOID(pipelineContext);
        auto theme = pipelineContext->GetTheme<ProgressTheme>();
        CHECK_NULL_VOID(theme);
        LoadingProgressModelNG::SetColor(frameNode, theme->GetLoadingColor());
    }
}

ArkUI_Bool GetEnableLoading(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_CODE_PARAM_INVALID);
    return LoadingProgressModelNG::GetEnableLoading(frameNode);
}

void SetEnableLoading(ArkUINodeHandle node, ArkUI_Bool enableLoadingValue)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    LoadingProgressModelNG::SetEnableLoading(frameNode, enableLoadingValue);
}

void ResetEnableLoading(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    LoadingProgressModelNG::SetEnableLoading(frameNode, true);
}

void SetLoadingProgressForegroundColor(ArkUINodeHandle node, ArkUI_Uint32 colorValue)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    LoadingProgressModelNG::SetForegroundColor(frameNode, Color(colorValue));
}

void ResetLoadingProgressForegroundColor(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
}
} // namespace

namespace NodeModifier {
const ArkUILoadingProgressModifier* GetLoadingProgressModifier()
{
    static const ArkUILoadingProgressModifier modifier = {
        GetLoadingProgressColor, SetLoadingProgressColor, ResetLoadingProgressColor,
        GetEnableLoading, SetEnableLoading, ResetEnableLoading,
        SetLoadingProgressForegroundColor, ResetLoadingProgressForegroundColor };

    return &modifier;
}
} // namespace NodeModifier
} // namespace OHOS::Ace::NG
