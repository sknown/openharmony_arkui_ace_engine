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

#ifndef FOUNDATION_ACE_ADAPTER_OHOS_OSAL_PAGE_URL_CHECKER_OHOS_H
#define FOUNDATION_ACE_ADAPTER_OHOS_OSAL_PAGE_URL_CHECKER_OHOS_H

#include "frameworks/core/common/page_url_checker.h"

#include "bundlemgr/bundle_mgr_interface.h"
#include "interfaces/inner_api/ace/ui_content.h"

namespace OHOS::Ace {
class PageUrlCheckerOhos : public PageUrlChecker {
    DECLARE_ACE_TYPE(PageUrlCheckerOhos, PageUrlChecker)

public:
    explicit PageUrlCheckerOhos(
        const std::shared_ptr<OHOS::AbilityRuntime::Context>& context) : context_(context) {}
    PageUrlCheckerOhos(
        const std::shared_ptr<OHOS::AbilityRuntime::Context>& context,
        const std::shared_ptr<OHOS::AppExecFwk::AbilityInfo>& abilityInfo)
        : context_(context), abilityInfo_(abilityInfo) {}
    ~PageUrlCheckerOhos() = default;
    void LoadPageUrl(const std::string& url, const std::function<void()>& callback,
        const std::function<void(int32_t, const std::string&)>& silentInstallErrorCallBack) override;
    void CheckPreload(const std::string& url) override;

    void NotifyPageShow(const std::string& pageName) override;
    void NotifyPageHide(const std::string& pageName) override;
    void SetModuleNameCallback(std::function<std::string(const std::string&)>&& callback) override;

private:
    sptr<AppExecFwk::IBundleMgr> GetBundleManager();
    std::shared_ptr<OHOS::AbilityRuntime::Context> context_;
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo_;
    std::function<std::string(const std::string&)> moduleNameCallback_;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_OHOS_OSAL_PAGE_URL_CHECKER_OHOS_H