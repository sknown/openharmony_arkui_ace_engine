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

#include "core/components/common/layout/constants.h"
#include "core/components/progress/progress_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/loading_progress/loading_progress_model_ng.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace::NG {
namespace {
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
        auto pipelineContext = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        auto theme = pipelineContext->GetTheme<ProgressTheme>();
        CHECK_NULL_VOID(theme);
        LoadingProgressModelNG::SetColor(frameNode, theme->GetLoadingColor());
    }
}

void SetEnableLoading(ArkUINodeHandle node, bool enableLoadingValue)
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
} // namespace

namespace NodeModifier {
const ArkUILoadingProgressModifier* GetLoadingProgressModifier()
{
    static const ArkUILoadingProgressModifier modifier = { SetLoadingProgressColor, ResetLoadingProgressColor,
        SetEnableLoading, ResetEnableLoading };

    return &modifier;
}
} // namespace NodeModifier
} // namespace OHOS::Ace::NG
