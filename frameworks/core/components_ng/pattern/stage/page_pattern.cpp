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

#include "core/components_ng/pattern/stage/page_pattern.h"

#include "base/utils/utils.h"
#include "core/components/common/properties/alignment.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

void PagePattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
    host->GetLayoutProperty()->UpdateAlignment(Alignment::TOP_LEFT);
    host->GetRenderContext()->UpdateBackgroundColor(Color::WHITE);
}

bool PagePattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& /*wrapper*/, const DirtySwapConfig& /*config*/)
{
    if (!isLoaded_) {
        isOnShow_ = true;
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_RETURN(context, false);
        if (onPageShow_) {
            context->PostAsyncEvent([onPageShow = onPageShow_]() { onPageShow(); });
        }
        isLoaded_ = true;
    }
    return false;
}

bool PagePattern::TriggerPageTransition(PageTransitionType type) const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, false);
    return renderContext->TriggerPageTransition(type);
}

void PagePattern::OnShow()
{
    if (isOnShow_ || !isLoaded_) {
        return;
    }
    isOnShow_ = true;
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    if (onPageShow_) {
        context->PostAsyncEvent([onPageShow = onPageShow_]() { onPageShow(); });
    }
}

void PagePattern::OnHide()
{
    if (!isOnShow_) {
        return;
    }
    isOnShow_ = false;
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    if (onPageHide_) {
        context->PostAsyncEvent([onPageHide = onPageHide_]() { onPageHide(); });
    }
}

} // namespace OHOS::Ace::NG