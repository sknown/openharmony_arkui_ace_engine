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

#include "core/components/web/resource/web_client_impl.h"
#include "core/components/web/resource/web_delegate.h"

namespace OHOS::Ace {

void DownloadListenerImpl::OnDownloadStart(const std::string& url, const std::string& userAgent,
    const std::string& contentDisposition, const std::string& mimetype, long contentLength)
{
    ContainerScope scope(instanceId_);
    LOGI("OnDownloadStart.");
    auto delegate = webDelegate_.Upgrade();
    if (!delegate) {
        return;
    }
    delegate->OnDownloadStart(url, userAgent, contentDisposition, mimetype, contentLength);
}

void WebClientImpl::OnPageFinished(int httpStatusCode, const std::string& url)
{
    ContainerScope scope(instanceId_);
    auto delegate = webDelegate_.Upgrade();
    if (!delegate) {
        return;
    }
    delegate->OnPageFinished(url);
}

void WebClientImpl::OnRequestFocus()
{
    ContainerScope scope(instanceId_);
    auto delegate = webDelegate_.Upgrade();
    if (!delegate) {
        return;
    }
    delegate->OnRequestFocus();
}

void WebClientImpl::OnPageStarted(const std::string& url)
{
    ContainerScope scope(instanceId_);
    auto delegate = webDelegate_.Upgrade();
    if (!delegate) {
        return;
    }
    delegate->OnPageStarted(url);
}

void WebClientImpl::OnProgressChanged(int newProgress)
{
    ContainerScope scope(instanceId_);
    auto delegate = webDelegate_.Upgrade();
    if (!delegate) {
        return;
    }
    delegate->OnProgressChanged(newProgress);
}

void WebClientImpl::OnReceivedTitle(const std::string &title)
{
    ContainerScope scope(instanceId_);
    auto delegate = webDelegate_.Upgrade();
    if (!delegate) {
        return;
    }
    delegate->OnReceivedTitle(title);
}

void WebClientImpl::OnGeolocationPermissionsHidePrompt()
{
    ContainerScope scope(instanceId_);
    auto delegate = webDelegate_.Upgrade();
    if (!delegate) {
        return;
    }
    delegate->OnGeolocationPermissionsHidePrompt();
}

void WebClientImpl::OnGeolocationPermissionsShowPrompt(const std::string& origin,
    OHOS::WebView::GeolocationCallback* callback)
{
    ContainerScope scope(instanceId_);
    auto delegate = webDelegate_.Upgrade();
    if (!delegate) {
        return;
    }
    delegate->OnGeolocationPermissionsShowPrompt(origin, callback);
}

void WebClientImpl::SetWebView(std::shared_ptr<OHOS::WebView::WebView> webview)
{
    webviewWeak_ = webview;
}

void WebClientImpl::OnProxyDied()
{
    auto delegate = webDelegate_.Upgrade();
    if (!delegate) {
        return;
    }
}

void WebClientImpl::onReceivedError(std::shared_ptr<WebView::WebResourceRequest> request,
    std::shared_ptr<WebView::WebResourceError> error)
{
    ContainerScope scope(instanceId_);
    auto delegate = webDelegate_.Upgrade();
    if (!delegate) {
        return;
    }
    delegate->OnPageErrorOHOS(error->GetErrorCode(), error->GetDescription(), request->GetUrl());
}

void WebClientImpl::OnMessage(const std::string& param)
{
    ContainerScope scope(instanceId_);
    auto delegate = webDelegate_.Upgrade();
    if (!delegate) {
        return;
    }
    delegate->OnMessage(param);
}

void WebClientImpl::OnRouterPush(const std::string& param)
{
    ContainerScope scope(instanceId_);
    auto delegate = webDelegate_.Upgrade();
    if (!delegate) {
        return;
    }
    delegate->OnRouterPush(param);
}
} // namespace OHOS::Ace
