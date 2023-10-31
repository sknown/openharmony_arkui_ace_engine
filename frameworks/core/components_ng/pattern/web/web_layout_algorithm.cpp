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

#include "core/components_ng/pattern/web/web_layout_algorithm.h"

#include "core/components_ng/base/frame_node.h"

#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
#include "core/components_ng/pattern/web/web_pattern.h"
#else
#include "core/components_ng/pattern/web/cross_platform/web_pattern.h"
#endif
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

constexpr int32_t MAX_ROOT_LAYER = 8000;

namespace OHOS::Ace::NG {
WebLayoutAlgorithm::WebLayoutAlgorithm() = default;

std::optional<SizeF> WebLayoutAlgorithm::MeasureContent(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_RETURN(layoutWrapper, std::nullopt);
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(host, std::nullopt);
    auto pattern = DynamicCast<WebPattern>(host->GetPattern());
    CHECK_NULL_RETURN(pattern, std::nullopt);
    int rootLayerWidth = pattern->GetRootLayerWidth();
    int rootLayerHeight = pattern->GetRootLayerHeight();
    if (pattern->GetWrapContent() && IsValidRootLayer(rootLayerWidth) && IsValidRootLayer(rootLayerHeight)) {
        TAG_LOGD(AceLogTag::ACE_WEB, "RootLayerWidth = %{public}d, RootLayerHeight = %{public}d", rootLayerWidth,
            rootLayerHeight);
        SizeF rootLayerSize = { rootLayerWidth, rootLayerHeight };
        return rootLayerSize;
    }
    auto layoutSize = contentConstraint.selfIdealSize.IsValid() ? contentConstraint.selfIdealSize.ConvertToSizeT()
                                                                : contentConstraint.maxSize;
    return layoutSize;
}

bool WebLayoutAlgorithm::IsValidRootLayer(int value)
{
    return value > 0 && value <= MAX_ROOT_LAYER;
}
} // namespace OHOS::Ace::NG
