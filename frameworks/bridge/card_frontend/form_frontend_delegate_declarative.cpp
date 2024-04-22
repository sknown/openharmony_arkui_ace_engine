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

#include "frameworks/bridge/card_frontend/form_frontend_delegate_declarative.h"

#include "base/log/event_report.h"
#include "base/utils/utils.h"
#include "base/utils/measure_util.h"
#include "core/common/thread_checker.h"

namespace OHOS::Ace::Framework {
FormFrontendDelegateDeclarative::~FormFrontendDelegateDeclarative()
{
    CHECK_RUN_ON(JS);
    LOG_DESTROY();
}

UIContentErrorCode FormFrontendDelegateDeclarative::RunCard(const std::string& url, const std::string& params,
    const std::string& profile, int64_t cardId, const std::string& entryPoint)
{
    ACE_SCOPED_TRACE("FormFrontendDelegateDeclarative::RunCard");
    auto pageRouterManager = GetPageRouterManager();
    CHECK_NULL_RETURN(pageRouterManager, UIContentErrorCode::NULL_PAGE_ROUTER);
    pageRouterManager->SetManifestParser(GetManifestParser());
    pageRouterManager->SetIsCard();
    auto cardPipeline = GetPipelineContext();
    auto taskExecutor = GetTaskExecutor();
    CHECK_NULL_RETURN(taskExecutor, UIContentErrorCode::NULL_POINTER);
    cardData_ = params;
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, UIContentErrorCode::NULL_POINTER);
    auto weakCardPipeline = WeakPtr<PipelineBase>(cardPipeline);
    container->SetCardPipeline(weakCardPipeline, cardId);
#ifndef PREVIEW
    return pageRouterManager->RunCard(url, params, cardId, entryPoint);
#else
    taskExecutor->PostTask(
        [weak = WeakClaim<NG::PageRouterManager>(RawPtr(pageRouterManager)), url, params, cardId, entryPoint]() {
            auto pageRouterManager = weak.Upgrade();
            CHECK_NULL_VOID(pageRouterManager);
            pageRouterManager->RunCard(url, params, cardId, entryPoint);
        },
        TaskExecutor::TaskType::JS, "ArkUIFormFrontendRunCard");
    return UIContentErrorCode::NO_ERRORS;
#endif
}

void FormFrontendDelegateDeclarative::FireCardEvent(const EventMarker& eventMarker, const std::string& params) {}

void FormFrontendDelegateDeclarative::FireCardAction(const std::string& action)
{
    auto context = GetPipelineContext();
    CHECK_NULL_VOID(context);
    auto taskExecutor = GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [weakCardPipeline = WeakPtr<PipelineBase>(context), action]() {
            auto context = weakCardPipeline.Upgrade();
            if (context) {
                context->OnActionEvent(action);
            }
        },
        TaskExecutor::TaskType::UI, "ArkUIFormFrontendFireAction"); // eTSCard UI == Main JS/UI/PLATFORM
}
} // namespace OHOS::Ace::Framework
