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

#include "frameworks/bridge/card_frontend/card_frontend_declarative.h"

#include <memory>
#include <vector>

#include "base/log/event_report.h"
#include "core/common/thread_checker.h"
#include "frameworks/bridge/common/utils/utils.h"
#include "frameworks/core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace {
namespace {

const char FILE_TYPE_BIN[] = ".abc";

} // namespace

CardFrontendDeclarative::~CardFrontendDeclarative()
{
    LOG_DESTROY();
}

bool CardFrontendDeclarative::Initialize(FrontendType type, const RefPtr<TaskExecutor>& taskExecutor)
{
    type_ = type;
    taskExecutor_ = taskExecutor;
    InitializeDelegate(taskExecutor);
    manifestParser_ = AceType::MakeRefPtr<Framework::ManifestParser>();
    return true;
}

void CardFrontendDeclarative::InitializeDelegate(const RefPtr<TaskExecutor>& taskExecutor)
{
    auto pageRouterManager = AceType::MakeRefPtr<NG::PageRouterManager>();
    delegate_ = AceType::MakeRefPtr<Framework::CardFrontendDelegateDeclarative>(taskExecutor);
    delegate_->SetPageRouterManager(pageRouterManager);
}

RefPtr<NG::PageRouterManager> CardFrontendDeclarative::GetPageRouterManager() const
{
    CHECK_NULL_RETURN(delegate_, nullptr);
    return delegate_->GetPageRouterManager();
}

void CardFrontendDeclarative::Destroy()
{
    CHECK_RUN_ON(JS);
    delegate_.Reset();
    eventHandler_.Reset();
}

void CardFrontendDeclarative::AttachPipelineContext(const RefPtr<PipelineBase>& context)
{
    auto pipelineContext = DynamicCast<NG::PipelineContext>(context);
    if (!delegate_ || !pipelineContext) {
        return;
    }
    eventHandler_ = AceType::MakeRefPtr<CardEventHandlerDeclarative>(delegate_);

    holder_.Attach(context);
    delegate_->AttachPipelineContext(context);
}

void CardFrontendDeclarative::SetAssetManager(const RefPtr<AssetManager>& assetManager)
{
    assetManager_ = assetManager;
    if (delegate_) {
        delegate_->SetAssetManager(assetManager);
    }
}

void CardFrontendDeclarative::RunPage(int32_t pageId, const std::string& url, const std::string& params)
{
    std::string urlPath;
    if (GetFormSrc().empty()) {
        ParseManifest();
        if (!url.empty()) {
            urlPath = manifestParser_->GetRouter()->GetPagePath(url, FILE_TYPE_BIN);
        }
        if (urlPath.empty()) {
            urlPath = manifestParser_->GetRouter()->GetEntry(FILE_TYPE_BIN);
        }
    } else {
        urlPath = GetFormSrcPath(GetFormSrc(), FILE_TYPE_BIN);
    }
    if (urlPath.empty()) {
        LOGE("fail to eTS Card run page due to path url is empty");
        EventReport::SendFormException(FormExcepType::RUN_PAGE_ERR);
        return;
    }

    if (delegate_) {
        auto container = Container::Current();
        if (!container) {
            LOGE("RunPage host container null");
            EventReport::SendFormException(FormExcepType::RUN_PAGE_ERR);
            return;
        }
        container->SetCardFrontend(AceType::WeakClaim(this), cardId_);
        delegate_->RunCard(urlPath, params, "", cardId_);
    }
}

void CardFrontendDeclarative::OnPageLoaded(const RefPtr<Framework::JsAcePage>& page)
{
    CHECK_RUN_ON(JS);
    // Pop all JS command and execute them in UI thread.
    auto jsCommands = std::make_shared<std::vector<RefPtr<Framework::JsCommand>>>();
    page->PopAllCommands(*jsCommands);
    page->SetPipelineContext(holder_.Get());
    taskExecutor_->PostTask(
        [weak = AceType::WeakClaim(this), page, jsCommands] {
            auto frontend = weak.Upgrade();
            if (!frontend) {
                return;
            }
            // Flush all JS commands.
            for (const auto& command : *jsCommands) {
                command->Execute(page);
            }

            auto pipelineContext = AceType::DynamicCast<PipelineContext>(frontend->holder_.Get());
            CHECK_NULL_VOID(pipelineContext);
            auto minSdk = frontend->manifestParser_->GetMinPlatformVersion();
            pipelineContext->SetMinPlatformVersion(minSdk);

            auto document = page->GetDomDocument();
            if (frontend->pageLoaded_) {
                page->ClearShowCommand();
                std::vector<NodeId> dirtyNodes;
                page->PopAllDirtyNodes(dirtyNodes);
                if (dirtyNodes.empty()) {
                    return;
                }
                auto rootNodeId = dirtyNodes.front();
                if (rootNodeId == DOM_ROOT_NODE_ID_BASE) {
                    auto patchComponent = page->BuildPagePatch(rootNodeId);
                    if (patchComponent) {
                        pipelineContext->ScheduleUpdate(patchComponent);
                    }
                }
                if (document) {
                    // When a component is configured with "position: fixed", there is a proxy node in root tree
                    // instead of the real composed node. So here updates the real composed node.
                    for (int32_t nodeId : document->GetProxyRelatedNodes()) {
                        auto patchComponent = page->BuildPagePatch(nodeId);
                        if (patchComponent) {
                            pipelineContext->ScheduleUpdate(patchComponent);
                        }
                    }
                }
                return;
            }

            // Just clear all dirty nodes.
            page->ClearAllDirtyNodes();
            if (document) {
                document->HandleComponentPostBinding();
            }
            if (pipelineContext->GetAccessibilityManager()) {
                pipelineContext->GetAccessibilityManager()->HandleComponentPostBinding();
            }
            if (pipelineContext->CanPushPage()) {
                pipelineContext->PushPage(page->BuildPage(page->GetUrl()));
                frontend->pageLoaded_ = true;
            }
        },
        TaskExecutor::TaskType::UI);
}

void CardFrontendDeclarative::UpdateData(const std::string& dataList)
{
    taskExecutor_->PostTask(
        [weak = AceType::WeakClaim(this), dataList] {
            auto frontend = weak.Upgrade();
            if (frontend) {
                frontend->UpdatePageData(dataList);
            }
        },
        TaskExecutor::TaskType::JS);
}

void CardFrontendDeclarative::UpdatePageData(const std::string& dataList)
{
    CHECK_RUN_ON(JS);
    if (!delegate_) {
        LOGE("the delegate is null");
        EventReport::SendFormException(FormExcepType::UPDATE_PAGE_ERR);
        return;
    }
}

void CardFrontendDeclarative::SetColorMode(ColorMode colorMode)
{
    taskExecutor_->PostTask(
        [weak = AceType::WeakClaim(this), colorMode]() {
            auto frontend = weak.Upgrade();
            if (frontend) {
                frontend->colorMode_ = colorMode;
                if (!frontend->delegate_) {
                    LOGE("the delegate is null");
                    return;
                }
                frontend->OnMediaFeatureUpdate();
            } else {
                LOGE("eTS Card frontend is nullptr");
            }
        },
        TaskExecutor::TaskType::JS);
}

void CardFrontendDeclarative::RebuildAllPages()
{
}

void CardFrontendDeclarative::OnSurfaceChanged(int32_t width, int32_t height)
{
    taskExecutor_->PostTask(
        [weak = AceType::WeakClaim(this), width, height] {
            auto frontend = weak.Upgrade();
            if (frontend) {
                frontend->HandleSurfaceChanged(width, height);
            }
        },
        TaskExecutor::TaskType::JS);
}

void CardFrontendDeclarative::HandleSurfaceChanged(int32_t width, int32_t height)
{
    CHECK_RUN_ON(JS);
    OnMediaFeatureUpdate();
}

void CardFrontendDeclarative::OnMediaFeatureUpdate()
{
    CHECK_RUN_ON(JS);
}

} // namespace OHOS::Ace
