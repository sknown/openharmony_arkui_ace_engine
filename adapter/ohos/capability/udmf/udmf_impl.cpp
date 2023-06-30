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

#include "udmf_impl.h"

#include <memory>
#include <unordered_map>

#include "html.h"
#include "image.h"
#include "link.h"
#include "summary_napi.h"
#include "system_defined_form.h"
#include "system_defined_pixelmap.h"
#include "text.h"
#include "udmf_client.h"
#include "unified_data.h"
#include "unified_data_napi.h"
#include "unified_types.h"
#include "video.h"
#include "native_engine/native_engine.h"
#include "frameworks/bridge/common/utils/engine_helper.h"
#include "frameworks/bridge/js_frontend/engine/common/js_engine.h"
#include "js_native_api_types.h"

#include "base/utils/utils.h"
#include "core/common/udmf/unified_data.h"
namespace OHOS::Ace {
UdmfClient* UdmfClient::GetInstance()
{
    static UdmfClientImpl instance;
    return &instance;
}

RefPtr<UnifiedData> UdmfClientImpl::CreateUnifiedData()
{
    return AceType::DynamicCast<UnifiedData>(AceType::MakeRefPtr<UnifiedDataImpl>());
}

RefPtr<UnifiedData> UdmfClientImpl::TransformUnifiedData(NativeValue* nativeValue)
{
    auto engine = EngineHelper::GetCurrentEngine();
    CHECK_NULL_RETURN(engine, nullptr);
    NativeEngine* nativeEngine = engine->GetNativeEngine();
    napi_env env = reinterpret_cast<napi_env>(nativeEngine);
    void* native = nullptr;
    napi_unwrap(env, reinterpret_cast<napi_value>(nativeValue), &native);
    auto* unifiedData = reinterpret_cast<UDMF::UnifiedDataNapi*>(native);
    CHECK_NULL_RETURN(unifiedData, nullptr);
    CHECK_NULL_RETURN(unifiedData->value_, nullptr);
    auto udData = AceType::MakeRefPtr<UnifiedDataImpl>();
    udData->SetUnifiedData(unifiedData->value_);
    return udData;
}

NativeValue* UdmfClientImpl::TransformUdmfUnifiedData(RefPtr<UnifiedData>& UnifiedData)
{
    auto engine = EngineHelper::GetCurrentEngine();
    CHECK_NULL_RETURN(engine, nullptr);
    NativeEngine* nativeEngine = engine->GetNativeEngine();
    napi_env env = reinterpret_cast<napi_env>(nativeEngine);
    auto unifiedData = AceType::DynamicCast<UnifiedDataImpl>(UnifiedData)->GetUnifiedData();
    CHECK_NULL_RETURN(unifiedData, nullptr);
    napi_value dataVal = nullptr;
    UDMF::UnifiedDataNapi::NewInstance(env, unifiedData, dataVal);
    CHECK_NULL_RETURN(dataVal, nullptr);
    return reinterpret_cast<NativeValue*>(dataVal);
}

NativeValue* UdmfClientImpl::TransformSummary(std::map<std::string, int64_t>& summary)
{
    auto engine = EngineHelper::GetCurrentEngine();
    CHECK_NULL_RETURN(engine, nullptr);
    NativeEngine* nativeEngine = engine->GetNativeEngine();
    napi_env env = reinterpret_cast<napi_env>(nativeEngine);
    std::shared_ptr<UDMF::Summary> udmfSummary = std::make_shared<UDMF::Summary>();
    CHECK_NULL_RETURN(udmfSummary, nullptr);
    udmfSummary->totalSize = 0;
    for (auto element : summary) {
        udmfSummary->totalSize += element.second;
    }
    udmfSummary->summary = std::move(summary);
    napi_value dataVal = nullptr;
    UDMF::SummaryNapi::NewInstance(env, udmfSummary, dataVal);
    CHECK_NULL_RETURN(dataVal, nullptr);
    return reinterpret_cast<NativeValue*>(dataVal);
}

int32_t UdmfClientImpl::SetData(const RefPtr<UnifiedData>& unifiedData, std::string& key)
{
    auto client = UDMF::UdmfClient::GetInstance();
    UDMF::CustomOption udCustomOption;
    udCustomOption.intention = UDMF::Intention::UD_INTENTION_DRAG;
    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_RETURN(udData, UDMF::E_ERROR);
    int32_t ret = client.SetData(udCustomOption, *udData->GetUnifiedData(), key);
    return ret;
}

int32_t UdmfClientImpl::GetData(const RefPtr<UnifiedData>& unifiedData, const std::string& key)
{
    auto client = UDMF::UdmfClient::GetInstance();
    UDMF::QueryOption queryOption;
    queryOption.key = key;
    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_RETURN(udData, UDMF::E_ERROR);
    int ret = client.GetData(queryOption, *udData->GetUnifiedData());
    return ret;
}

int32_t UdmfClientImpl::GetSummary(std::string& key, std::map<std::string, int64_t>& summaryMap)
{
    auto client = UDMF::UdmfClient::GetInstance();
    UDMF::Summary summary;
    UDMF::QueryOption queryOption;
    queryOption.key = key;
    int32_t ret = client.GetSummary(queryOption, summary);
    summaryMap = summary.summary;
    return ret;
}

int64_t UnifiedDataImpl::GetSize()
{
    CHECK_NULL_RETURN(unifiedData_, 0);
    return unifiedData_->GetRecords().size();
}

std::shared_ptr<UDMF::UnifiedData> UnifiedDataImpl::GetUnifiedData()
{
    if (unifiedData_ == nullptr) {
        unifiedData_ = std::make_shared<UDMF::UnifiedData>();
    }
    return unifiedData_;
}

void UnifiedDataImpl::SetUnifiedData(std::shared_ptr<UDMF::UnifiedData> unifiedData)
{
    unifiedData_ = unifiedData;
}

void UdmfClientImpl::AddFormRecord(
    const RefPtr<UnifiedData>& unifiedData, int32_t formId, const RequestFormInfo& cardInfo)
{
    auto formRecord = std::make_shared<UDMF::SystemDefinedForm>();
    formRecord->SetFormId(formId);
    formRecord->SetFormName(cardInfo.cardName);
    formRecord->SetBundleName(cardInfo.bundleName);
    formRecord->SetAbilityName(cardInfo.abilityName);
    formRecord->SetModule(cardInfo.moduleName);
    formRecord->SetType(UDMF::UDType::SYSTEM_DEFINED_FORM);

    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_VOID(udData);
    udData->GetUnifiedData()->AddRecord(formRecord);
}

void UdmfClientImpl::AddLinkRecord(
    const RefPtr<UnifiedData>& unifiedData, const std::string& url, const std::string& description)
{
    auto record = std::make_shared<UDMF::Link>(url, description);

    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_VOID(udData);
    udData->GetUnifiedData()->AddRecord(record);
}

void UdmfClientImpl::GetLinkRecord(
    const RefPtr<UnifiedData>& unifiedData, std::string& url, std::string& description)
{
    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_VOID(udData);
    auto records = udData->GetUnifiedData()->GetRecords();
    for (auto record : records) {
        UDMF::UDType type = record->GetType();
        if (type == UDMF::UDType::HYPERLINK) {
            UDMF::Link* link = reinterpret_cast<UDMF::Link*>(record.get());
            url = link->GetUrl();
            description = link->GetDescription();
            return;
        }
    }
}

void UdmfClientImpl::AddHtmlRecord(
    const RefPtr<UnifiedData>& unifiedData, const std::string& htmlContent, const std::string& plainContent)
{
    auto htmlRecord = std::make_shared<UDMF::Html>(htmlContent, plainContent);

    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_VOID(udData);
    if (!plainContent.empty() || !htmlContent.empty()) {
        udData->GetUnifiedData()->AddRecord(htmlRecord);
    }
}

void UdmfClientImpl::GetHtmlRecord(
    const RefPtr<UnifiedData>& unifiedData, std::string& htmlContent, std::string& plainContent)
{
    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_VOID(udData);
    auto records = udData->GetUnifiedData()->GetRecords();
    for (auto record : records) {
        UDMF::UDType type = record->GetType();
        if (type == UDMF::UDType::HTML) {
            UDMF::Html* html = reinterpret_cast<UDMF::Html*>(record.get());
            plainContent = html->GetPlainContent();
            htmlContent = html->GetHtmlContent();
            return;
        }
    }
}

void UdmfClientImpl::AddPixelMapRecord(const RefPtr<UnifiedData>& unifiedData, std::vector<uint8_t>& data)
{
    auto record = std::make_shared<UDMF::SystemDefinedPixelMap>(data);

    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_VOID(udData);
    udData->GetUnifiedData()->AddRecord(record);
}

void UdmfClientImpl::AddImageRecord(const RefPtr<UnifiedData>& unifiedData, const std::string& uri)
{
    auto record = std::make_shared<UDMF::Image>(uri);

    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_VOID(udData);
    udData->GetUnifiedData()->AddRecord(record);
}

void UdmfClientImpl::AddTextRecord(const RefPtr<UnifiedData>& unifiedData, const std::string& selectedStr)
{
    UDMF::UDVariant udmfValue(selectedStr);
    UDMF::UDDetails udmfDetails = { { "value", udmfValue } };
    auto record = std::make_shared<UDMF::Text>(udmfDetails);

    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_VOID(udData);
    udData->GetUnifiedData()->AddRecord(record);
}

std::string UdmfClientImpl::GetSingleTextRecord(const RefPtr<UnifiedData>& unifiedData)
{
    std::string str = "";
    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_RETURN(udData, str);
    auto records = udData->GetUnifiedData()->GetRecords();
    if (records.size() >= 1 && records[0]->GetType() == UDMF::UDType::TEXT) {
        UDMF::Text* text = reinterpret_cast<UDMF::Text*>(records[0].get());
        UDMF::UDDetails udmfDetails = text->GetDetails();
        auto value = udmfDetails.find("value");
        if (value != udmfDetails.end()) {
            str = std::get<std::string>(value->second);
        }
    }
    return str;
}

int32_t UdmfClientImpl::GetVideoRecordUri(const RefPtr<UnifiedData>& unifiedData, std::string& uri)
{
    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_RETURN(udData, UDMF::E_ERROR);
    auto records = udData->GetUnifiedData()->GetRecords();
    if (records.size() == 0) {
        return UDMF::E_ERROR;
    }
    auto video = static_cast<UDMF::Video*>(records[0].get());
    uri = video->GetUri();
    return 0;
}
} // namespace OHOS::Ace
