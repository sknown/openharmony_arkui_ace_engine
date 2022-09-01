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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_RESOURCE_WEBVIEW_CLIENT_IMPL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_RESOURCE_WEBVIEW_CLIENT_IMPL_H

#include "foundation/arkui/ace_engine/frameworks/base/memory/referenced.h"
#include "nweb_handler.h"

#include "base/log/log.h"
#include "core/common/container_scope.h"

namespace OHOS::Ace {
class WebDelegate;

class DownloadListenerImpl : public OHOS::NWeb::NWebDownloadCallback {
public:
    DownloadListenerImpl() = default;
    explicit DownloadListenerImpl(int32_t instanceId) : instanceId_(instanceId) {}
    ~DownloadListenerImpl() = default;

    void OnDownloadStart(const std::string& url, const std::string& userAgent, const std::string& contentDisposition,
        const std::string& mimetype, long contentLength) override;

    void SetWebDelegate(const WeakPtr<WebDelegate>& delegate)
    {
        webDelegate_ = delegate;
    }

private:
    WeakPtr<WebDelegate> webDelegate_;
    int32_t instanceId_ = -1;
};

class FindListenerImpl : public OHOS::NWeb::NWebFindCallback {
public:
    FindListenerImpl() = default;
    explicit FindListenerImpl(int32_t instanceId) : instanceId_(instanceId) {}
    ~FindListenerImpl() = default;

    void OnFindResultReceived(
        const int activeMatchOrdinal, const int numberOfMatches, const bool isDoneCounting) override;

    void SetWebDelegate(const WeakPtr<WebDelegate>& delegate)
    {
        webDelegate_ = delegate;
    }

private:
    WeakPtr<WebDelegate> webDelegate_;
    int32_t instanceId_ = -1;
};

class WebClientImpl : public OHOS::NWeb::NWebHandler {
public:
    WebClientImpl() = default;
    explicit WebClientImpl(int32_t instanceId) : instanceId_(instanceId) {}
    ~WebClientImpl() = default;

    void SetNWeb(std::shared_ptr<OHOS::NWeb::NWeb> nweb) override;
    void OnProxyDied() override;
    void OnRouterPush(const std::string& param) override;
    bool OnConsoleLog(const OHOS::NWeb::NWebConsoleLog& message) override;
    void OnMessage(const std::string& param) override;
    void OnPageLoadBegin(const std::string& url) override;
    void OnPageLoadEnd(int httpStatusCode, const std::string& url) override;
    void OnLoadingProgress(int newProgress) override;
    void OnPageTitle(const std::string &title) override;
    void OnGeolocationHide() override;
    void OnGeolocationShow(const std::string& origin,
        OHOS::NWeb::NWebGeolocationCallbackInterface* callback) override;

    bool OnAlertDialogByJS(const std::string &url,
                           const std::string &message,
                           std::shared_ptr<NWeb::NWebJSDialogResult> result) override;
    bool OnBeforeUnloadByJS(const std::string &url,
                            const std::string &message,
                            std::shared_ptr<NWeb::NWebJSDialogResult> result) override;
    bool OnConfirmDialogByJS(const std::string &url,
                             const std::string &message,
                             std::shared_ptr<NWeb::NWebJSDialogResult> result) override;
    bool OnPromptDialogByJS(const std::string &url,
                             const std::string &message,
                             const std::string &defaultValue,
                             std::shared_ptr<NWeb::NWebJSDialogResult> result) override;
    bool OnFileSelectorShow(std::shared_ptr<NWeb::FileSelectorCallback> callback,
                            std::shared_ptr<NWeb::NWebFileSelectorParams> params) override;

    void OnFocus() override;
    void OnResourceLoadError(std::shared_ptr<OHOS::NWeb::NWebUrlResourceRequest> request,
        std::shared_ptr<OHOS::NWeb::NWebUrlResourceError> error) override;
    void OnHttpError(std::shared_ptr<OHOS::NWeb::NWebUrlResourceRequest> request,
        std::shared_ptr<OHOS::NWeb::NWebUrlResourceResponse> response) override;
    void OnRenderExited(OHOS::NWeb::RenderExitReason reason) override;
    void OnRefreshAccessedHistory(const std::string& url, bool isReload) override;
    std::shared_ptr<OHOS::NWeb::NWebUrlResourceResponse> OnHandleInterceptRequest(
        std::shared_ptr<OHOS::NWeb::NWebUrlResourceRequest> request) override;
    bool OnHandleInterceptUrlLoading(const std::string& url) override;
    void OnResource(const std::string& url) override;
    void OnScaleChanged(float oldScaleFactor, float newScaleFactor) override;
    void OnScroll(double xOffset, double yOffset) override;
    bool OnHttpAuthRequestByJS(std::shared_ptr<NWeb::NWebJSHttpAuthResult> result, const std::string &host,
        const std::string &realm) override;
    void OnPermissionRequest(std::shared_ptr<NWeb::NWebAccessRequest> request) override;
    bool RunContextMenu(std::shared_ptr<NWeb::NWebContextMenuParams> params,
        std::shared_ptr<NWeb::NWebContextMenuCallback> callback) override;
    bool RunQuickMenu(std::shared_ptr<NWeb::NWebQuickMenuParams> params,
                      std::shared_ptr<NWeb::NWebQuickMenuCallback> callback) override;
    void OnQuickMenuDismissed() override;
    void OnTouchSelectionChanged(
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> insertHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> startSelectionHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> endSelectionHandle) override;

    void SetWebDelegate(const WeakPtr<WebDelegate>& delegate)
    {
        webDelegate_ = delegate;
    }

    const RefPtr<WebDelegate> GetWebDelegate() const
    {
        return webDelegate_.Upgrade();
    }

private:
    std::weak_ptr<OHOS::NWeb::NWeb> webviewWeak_;
    WeakPtr<WebDelegate> webDelegate_;
    int32_t instanceId_ = -1;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_RESOURCE_WEBVIEW_CLIENT_IMPL_H
