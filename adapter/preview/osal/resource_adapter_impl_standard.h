/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_PREVIEW_OSAL_RESOURCE_ADAPTER_IMPL_H
#define FOUNDATION_ACE_ADAPTER_PREVIEW_OSAL_RESOURCE_ADAPTER_IMPL_H

#include "resource_manager.h"

#include "core/components/theme/resource_adapter.h"

namespace OHOS::Ace {

class ResourceAdapterImpl : public ResourceAdapter {
    DECLARE_ACE_TYPE(ResourceAdapterImpl, ResourceAdapter);

public:
    ResourceAdapterImpl() = default;
    ~ResourceAdapterImpl() override = default;

    void Init(const ResourceInfo& resourceInfo) override;
    void UpdateConfig(const ResourceConfiguration& config, bool themeFlag = false) override;

    RefPtr<ThemeStyle> GetTheme(int32_t themeId) override;

    Color GetColor(uint32_t resId) override;
    Color GetColorByName(const std::string& resName) override;
    Dimension GetDimension(uint32_t resId) override;
    Dimension GetDimensionByName(const std::string& resName) override;
    std::string GetString(uint32_t resId) override;
    std::string GetStringByName(const std::string& resName) override;
    std::string GetPluralString(uint32_t resId, int quantity) override;
    std::string GetPluralStringByName(const std::string& resName, int quantity) override;
    std::vector<std::string> GetStringArray(uint32_t resId) const override;
    std::vector<std::string> GetStringArrayByName(const std::string& resName) const override;
    double GetDouble(uint32_t resId) override;
    double GetDoubleByName(const std::string& resName) override;
    int32_t GetInt(uint32_t resId) override;
    int32_t GetIntByName(const std::string& resName) override;
    std::vector<uint32_t> GetIntArray(uint32_t resId) const override;
    std::vector<uint32_t> GetIntArrayByName(const std::string& resName) const override;
    bool GetBoolean(uint32_t resId) const override;
    bool GetBooleanByName(const std::string& resName) const override;
    std::string GetMediaPath(uint32_t resId) override;
    std::string GetMediaPathByName(const std::string& resName) override;
    std::string GetRawfile(const std::string& fileName) override;
    bool GetRawFileData(const std::string& rawFile, size_t& len, std::unique_ptr<uint8_t[]> &dest) override;
    bool GetMediaData(uint32_t resId, size_t& len, std::unique_ptr<uint8_t[]> &dest) override;
    bool GetMediaData(const std::string& resName, size_t& len, std::unique_ptr<uint8_t[]> &dest) override;
    void UpdateResourceManager(const std::string& bundleName, const std::string& moduleName) override;
    uint32_t GetSymbolById(uint32_t resId) const override;
    uint32_t GetSymbolByName(const char* resName) const override;
private:
    static std::string GetActualResourceName(const std::string& resName);

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager_;
    std::string packagePathStr_;
    ACE_DISALLOW_COPY_AND_MOVE(ResourceAdapterImpl);
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_PREVIEW_OSAL_RESOURCE_ADAPTER_IMPL_H
