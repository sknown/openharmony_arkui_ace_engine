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

#include "adapter/ohos/capability/clipboard/clipboard_impl.h"

#include "adapter/ohos/osal/pixel_map_ohos.h"
#include "adapter/ohos/capability/html/html_to_span.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/text/span/span_string.h"

namespace OHOS::Ace {
#ifndef SYSTEM_CLIPBOARD_SUPPORTED
namespace {
std::string g_clipboard;
RefPtr<PixelMap> g_pixmap;
} // namespace
#endif

#ifdef SYSTEM_CLIPBOARD_SUPPORTED
MiscServices::ShareOption TransitionCopyOption(CopyOptions copyOption)
{
    auto shareOption = MiscServices::ShareOption::InApp;
    switch (copyOption) {
        case CopyOptions::InApp:
            shareOption = MiscServices::ShareOption::InApp;
            break;
        case CopyOptions::Local:
            shareOption = MiscServices::ShareOption::LocalDevice;
            break;
        case CopyOptions::Distributed:
            shareOption = MiscServices::ShareOption::CrossDevice;
            break;
        default:
            break;
    }
    return shareOption;
}

const std::string SPAN_STRING_TAG = "openharmony.styled-string";
#endif

void ClipboardImpl::HasData(const std::function<void(bool hasData)>& callback)
{
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    bool hasData = false;
    CHECK_NULL_VOID(taskExecutor_);
    taskExecutor_->PostSyncTask(
        [&hasData]() { hasData = OHOS::MiscServices::PasteboardClient::GetInstance()->HasPasteData(); },
        TaskExecutor::TaskType::PLATFORM, "ArkUIClipboardHasData");
    callback(hasData);
#endif
}

void ClipboardImpl::SetData(const std::string& data, CopyOptions copyOption, bool isDragData)
{
    CHECK_NULL_VOID(taskExecutor_);
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    auto shareOption = TransitionCopyOption(copyOption);
    taskExecutor_->PostTask(
        [data, shareOption, isDragData]() {
            auto pasteData = OHOS::MiscServices::PasteboardClient::GetInstance()->CreatePlainTextData(data);
            CHECK_NULL_VOID(pasteData);
            pasteData->SetShareOption(shareOption);
            pasteData->SetDraggedDataFlag(isDragData);
            OHOS::MiscServices::PasteboardClient::GetInstance()->SetPasteData(*pasteData);
        },
        TaskExecutor::TaskType::PLATFORM, "ArkUIClipboardSetDataWithCopyOption");
#else
    taskExecutor_->PostTask(
        [data]() { g_clipboard = data; }, TaskExecutor::TaskType::UI, "ArkUIClipboardSetTextPasteData");
#endif
}

void ClipboardImpl::SetPixelMapData(const RefPtr<PixelMap>& pixmap, CopyOptions copyOption)
{
    CHECK_NULL_VOID(taskExecutor_);
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    auto shareOption = TransitionCopyOption(copyOption);
    taskExecutor_->PostTask(
        [pixmap, shareOption]() {
            CHECK_NULL_VOID(pixmap);
            auto pixmapOhos = AceType::DynamicCast<PixelMapOhos>(pixmap);
            CHECK_NULL_VOID(pixmapOhos);
            auto pasteData = OHOS::MiscServices::PasteboardClient::GetInstance()->CreatePixelMapData(
                pixmapOhos->GetPixelMapSharedPtr());
            CHECK_NULL_VOID(pasteData);
            pasteData->SetShareOption(shareOption);
            LOGI("Set pixmap to system clipboard");
            OHOS::MiscServices::PasteboardClient::GetInstance()->SetPasteData(*pasteData);
        },
        TaskExecutor::TaskType::PLATFORM, "ArkUIClipboardSetPixelMapWithCopyOption");
#else
    taskExecutor_->PostTask(
        [pixmap]() { g_pixmap = pixmap; }, TaskExecutor::TaskType::UI, "ArkUIClipboardSetImagePasteData");
#endif
}

void ClipboardImpl::GetData(const std::function<void(const std::string&)>& callback, bool syncMode)
{
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    if (!taskExecutor_ || !callback) {
        return;
    }

    if (syncMode) {
        GetDataSync(callback);
    } else {
        GetDataAsync(callback);
    }
#else
    if (syncMode) {
        callback(g_clipboard);
        return;
    }
    CHECK_NULL_VOID(taskExecutor_);
    taskExecutor_->PostTask(
        [callback, taskExecutor = WeakClaim(RawPtr(taskExecutor_)), textData = g_clipboard]() {
            callback(textData);
        },
        TaskExecutor::TaskType::UI, "ArkUIClipboardTextDataCallback");
#endif
}

void ClipboardImpl::GetPixelMapData(const std::function<void(const RefPtr<PixelMap>&)>& callback, bool syncMode)
{
    if (!taskExecutor_ || !callback) {
        return;
    }
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    if (syncMode) {
        GetPixelMapDataSync(callback);
    } else {
        GetPixelMapDataAsync(callback);
    }
#else
    if (syncMode) {
        callback(g_pixmap);
    } else {
        taskExecutor_->PostTask([callback, taskExecutor = WeakClaim(RawPtr(taskExecutor_)),
                                    imageData = g_pixmap]() { callback(imageData); },
            TaskExecutor::TaskType::UI, "ArkUIClipboardImageDataCallback");
    }
#endif
}

RefPtr<PasteDataMix> ClipboardImpl::CreatePasteDataMix()
{
    return AceType::MakeRefPtr<PasteDataImpl>();
}

void ClipboardImpl::AddPixelMapRecord(const RefPtr<PasteDataMix>& pasteData, const RefPtr<PixelMap>& pixmap)
{
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    CHECK_NULL_VOID(taskExecutor_);
    auto peData = AceType::DynamicCast<PasteDataImpl>(pasteData);
    CHECK_NULL_VOID(peData);
    auto pixmapOhos = AceType::DynamicCast<PixelMapOhos>(pixmap);
    CHECK_NULL_VOID(pixmapOhos);
    LOGI("add pixelMap record to pasteData");
    peData->GetPasteDataData()->AddPixelMapRecord(pixmapOhos->GetPixelMapSharedPtr());
#endif
}

void ClipboardImpl::AddImageRecord(const RefPtr<PasteDataMix>& pasteData, const std::string& uri)
{
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    CHECK_NULL_VOID(taskExecutor_);
    auto peData = AceType::DynamicCast<PasteDataImpl>(pasteData);
    CHECK_NULL_VOID(peData);
    LOGI("add url record to pasteData, url:  %{public}s", uri.c_str());
    peData->GetPasteDataData()->AddUriRecord(OHOS::Uri(uri));
#endif
}

void ClipboardImpl::AddTextRecord(const RefPtr<PasteDataMix>& pasteData, const std::string& selectedStr)
{
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    CHECK_NULL_VOID(taskExecutor_);
    auto peData = AceType::DynamicCast<PasteDataImpl>(pasteData);
    CHECK_NULL_VOID(peData);
    LOGI("add text record to pasteData, length: %{public}d",
        static_cast<int32_t>(StringUtils::ToWstring(selectedStr).length()));
    peData->GetPasteDataData()->AddTextRecord(selectedStr);
#endif
}

void ClipboardImpl::AddSpanStringRecord(const RefPtr<PasteDataMix>& pasteData, std::vector<uint8_t>& data)
{
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    CHECK_NULL_VOID(taskExecutor_);
    auto peData = AceType::DynamicCast<PasteDataImpl>(pasteData);
    CHECK_NULL_VOID(peData);
    LOGI("add spanstring record to pasteData, length: %{public}d", static_cast<int32_t>(data.size()));
    peData->GetPasteDataData()->AddKvRecord(SPAN_STRING_TAG, data);
#endif
}

void ClipboardImpl::SetData(const RefPtr<PasteDataMix>& pasteData, CopyOptions copyOption)
{
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    auto shareOption = TransitionCopyOption(copyOption);
    auto peData = AceType::DynamicCast<PasteDataImpl>(pasteData);
    CHECK_NULL_VOID(peData);
    taskExecutor_->PostTask(
        [peData, shareOption]() {
            auto pasteData = peData->GetPasteDataData();
            pasteData->SetShareOption(shareOption);
            LOGI("add pasteData to clipboard, shareOption:  %{public}d", shareOption);
            OHOS::MiscServices::PasteboardClient::GetInstance()->SetPasteData(*pasteData);
        },
        TaskExecutor::TaskType::PLATFORM, "ArkUIClipboardSetMixDataWithCopyOption");
#endif
}

void ClipboardImpl::GetData(const std::function<void(const std::string&, bool isLastRecord)>& textCallback,
    const std::function<void(const RefPtr<PixelMap>&, bool isLastRecord)>& pixelMapCallback,
    const std::function<void(const std::string&, bool isLastRecord)>& urlCallback, bool syncMode)
{
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    if (!taskExecutor_ || !textCallback || !pixelMapCallback || !urlCallback) {
        return;
    }

    if (syncMode) {
        GetDataSync(textCallback, pixelMapCallback, urlCallback);
    } else {
        GetDataAsync(textCallback, pixelMapCallback, urlCallback);
    }
#endif
}

#ifdef SYSTEM_CLIPBOARD_SUPPORTED
std::shared_ptr<MiscServices::PasteData> PasteDataImpl::GetPasteDataData()
{
    if (pasteData_ == nullptr) {
        pasteData_ = std::make_shared<MiscServices::PasteData>();
    }
    return pasteData_;
}
void PasteDataImpl::SetUnifiedData(std::shared_ptr<MiscServices::PasteData> pasteData)
{
    pasteData_ = pasteData;
}

void ClipboardImpl::GetDataSync(const std::function<void(const std::string&)>& callback)
{
    std::string result;
    taskExecutor_->PostSyncTask(
        [&result]() {
            auto has = OHOS::MiscServices::PasteboardClient::GetInstance()->HasPasteData();
            CHECK_NULL_VOID(has);
            OHOS::MiscServices::PasteData pasteData;
            auto ok = OHOS::MiscServices::PasteboardClient::GetInstance()->GetPasteData(pasteData);
            CHECK_NULL_VOID(ok);
            for (const auto& pasteDataRecord : pasteData.AllRecords()) {
                if (pasteDataRecord == nullptr) {
                    continue;
                }
                if (pasteDataRecord->GetCustomData() == nullptr) {
                    auto customData = pasteDataRecord->GetCustomData();
                    auto itemData = customData->GetItemData();
                    if (itemData.find(SPAN_STRING_TAG) == itemData.end()) {
                        continue;
                    }
                    auto spanStr = SpanString::DecodeTlv(itemData[SPAN_STRING_TAG]);
                    if (spanStr) {
                        result = spanStr->GetString();
                        break;
                    }
                }
                if (pasteDataRecord->GetHtmlText() != nullptr) {
                    auto htmlText = pasteDataRecord->GetHtmlText();
                    HtmlToSpan toSpan;
                    auto spanStr = toSpan.ToSpanString(*htmlText);
                    if (spanStr) {
                        result = spanStr->GetString();
                        break;
                    }
                }
            }
            if (result.empty()) {
                auto textData = pasteData.GetPrimaryText();
                CHECK_NULL_VOID(textData);
                result = *textData;
            }
        },
        TaskExecutor::TaskType::PLATFORM, "ArkUIClipboardGetTextDataSync");
    callback(result);
}

void ClipboardImpl::GetDataAsync(const std::function<void(const std::string&)>& callback)
{
    taskExecutor_->PostTask(
        [callback, weakExecutor = WeakClaim(RawPtr(taskExecutor_)), weak = WeakClaim(this)]() {
            auto clip = weak.Upgrade();
            auto taskExecutor = weakExecutor.Upgrade();
            CHECK_NULL_VOID(taskExecutor);
            if (!OHOS::MiscServices::PasteboardClient::GetInstance()->HasPasteData()) {
                LOGW("GetDataAsync: SystemKeyboardData is not exist from MiscServices");
                taskExecutor->PostTask(
                    [callback]() { callback(""); }, TaskExecutor::TaskType::UI, "ArkUIClipboardHasDataFailed");
                return;
            }
            OHOS::MiscServices::PasteData pasteData;
            if (!OHOS::MiscServices::PasteboardClient::GetInstance()->GetPasteData(pasteData)) {
                LOGW("GetDataAsync: Get SystemKeyboardData fail from MiscServices");
                taskExecutor->PostTask(
                    [callback]() { callback(""); }, TaskExecutor::TaskType::UI, "ArkUIClipboardGetDataFailed");
                return;
            }
            std::string resText;
            for (const auto& pasteDataRecord : pasteData.AllRecords()) {
                clip->ProcessPasteDataRecord(pasteDataRecord, resText);
            }
            if (resText.empty()) {
                LOGW("GetDataAsync: Get SystemKeyboardTextData fail from MiscServices");
                taskExecutor->PostTask(
                    [callback]() { callback(""); }, TaskExecutor::TaskType::UI, "ArkUIClipboardGetTextDataFailed");
                return;
            }
            auto result = resText;
            taskExecutor->PostTask(
                [callback, result]() { callback(result); },
                TaskExecutor::TaskType::UI, "ArkUIClipboardGetTextDataCallback");
        },
        TaskExecutor::TaskType::PLATFORM, "ArkUIClipboardGetTextDataAsync");
}

void ClipboardImpl::ProcessPasteDataRecord(const std::shared_ptr<MiscServices::PasteDataRecord>& pasteDataRecord,
    std::string& resText)
{
    if (pasteDataRecord == nullptr) {
        return;
    }
    if (pasteDataRecord->GetHtmlText() != nullptr) {
        auto htmlText = pasteDataRecord->GetHtmlText();
        HtmlToSpan toSpan;
        auto spanStr = toSpan.ToSpanString(*htmlText);
        if (spanStr) {
            resText = spanStr->GetString();
            return;
        }
    }
    if (pasteDataRecord->GetCustomData() != nullptr) {
        auto itemData = pasteDataRecord->GetCustomData()->GetItemData();
        if (itemData.find(SPAN_STRING_TAG) != itemData.end()) {
            auto spanStr = SpanString::DecodeTlv(itemData[SPAN_STRING_TAG]);
            if (spanStr) {
                resText = spanStr->GetString();
                return;
            }
        }
    }
    if (pasteDataRecord->GetPlainText() != nullptr) {
        auto textData = pasteDataRecord->GetPlainText();
        resText.append(*textData);
    }
}

void ClipboardImpl::GetDataSync(const std::function<void(const std::string&, bool isLastRecord)>& textCallback,
    const std::function<void(const RefPtr<PixelMap>&, bool isLastRecord)>& pixelMapCallback,
    const std::function<void(const std::string&, bool isLastRecord)>& urlCallback)
{
    LOGI("get data from clipboard, sync");
    OHOS::MiscServices::PasteData pasteData;
    taskExecutor_->PostSyncTask(
        [&pasteData]() {
            auto has = OHOS::MiscServices::PasteboardClient::GetInstance()->HasPasteData();
            CHECK_NULL_VOID(has);
            auto ok = OHOS::MiscServices::PasteboardClient::GetInstance()->GetPasteData(pasteData);
            CHECK_NULL_VOID(ok);
        },
        TaskExecutor::TaskType::PLATFORM, "ArkUIClipboardGetPasteDataSync");

    auto count = pasteData.GetRecordCount();
    size_t index = 0;
    for (const auto& pasteDataRecord : pasteData.AllRecords()) {
        index++;
        if (pasteDataRecord == nullptr) {
            continue;
        }
        bool isLastRecord = index == count;
        if (pasteDataRecord->GetPlainText() != nullptr) {
            auto textData = pasteDataRecord->GetPlainText();
            auto result = *textData;
            textCallback(result, isLastRecord);
        } else if (pasteDataRecord->GetPixelMap() != nullptr) {
            auto imageData = pasteDataRecord->GetPixelMap();
            auto result = AceType::MakeRefPtr<PixelMapOhos>(imageData);
            pixelMapCallback(result, isLastRecord);
        } else if (pasteDataRecord->GetUri() != nullptr) {
            auto textData = pasteDataRecord->GetUri();
            auto result = (*textData).ToString();
            urlCallback(result, isLastRecord);
        }
    }
}

void ClipboardImpl::GetDataAsync(const std::function<void(const std::string&, bool isLastRecord)>& textCallback,
    const std::function<void(const RefPtr<PixelMap>&, bool isLastRecord)>& pixelMapCallback,
    const std::function<void(const std::string&, bool isLastRecord)>& urlCallback)
{
    LOGI("get data from clipboard, async");
    taskExecutor_->PostTask(
        [textCallback, pixelMapCallback, urlCallback, weakExecutor = WeakClaim(RawPtr(taskExecutor_))]() {
            auto taskExecutor = weakExecutor.Upgrade();
            auto has = OHOS::MiscServices::PasteboardClient::GetInstance()->HasPasteData();
            CHECK_NULL_VOID(has);
            OHOS::MiscServices::PasteData pasteData;
            auto ok = OHOS::MiscServices::PasteboardClient::GetInstance()->GetPasteData(pasteData);
            CHECK_NULL_VOID(ok);
            auto count = pasteData.GetRecordCount();
            size_t index = 0;
            for (const auto& pasteDataRecord : pasteData.AllRecords()) {
                index++;
                if (pasteDataRecord == nullptr) {
                    continue;
                }
                bool isLastRecord = index == count;
                if (pasteDataRecord->GetPlainText() != nullptr) {
                    auto textData = pasteDataRecord->GetPlainText();
                    auto result = *textData;
                    taskExecutor->PostTask(
                        [textCallback, result, isLastRecord]() { textCallback(result, isLastRecord); },
                        TaskExecutor::TaskType::UI, "ArkUIClipboardGetTextCallback");
                } else if (pasteDataRecord->GetPixelMap() != nullptr) {
                    auto imageData = pasteDataRecord->GetPixelMap();
                    auto result = AceType::MakeRefPtr<PixelMapOhos>(imageData);
                    taskExecutor->PostTask(
                        [pixelMapCallback, result, isLastRecord]() { pixelMapCallback(result, isLastRecord); },
                        TaskExecutor::TaskType::UI, "ArkUIClipboardGetImageCallback");
                } else if (pasteDataRecord->GetUri() != nullptr) {
                    auto textData = pasteDataRecord->GetUri();
                    auto result = (*textData).ToString();
                    taskExecutor->PostTask([urlCallback, result, isLastRecord]() { urlCallback(result, isLastRecord); },
                        TaskExecutor::TaskType::UI, "ArkUIClipboardGetUrlCallback");
                }
            }
        },
        TaskExecutor::TaskType::PLATFORM, "ArkUIClipboardGetDataAsync");
}

void ClipboardImpl::GetSpanStringData(
    const std::function<void(std::vector<uint8_t>&, const std::string&)>& callback, bool syncMode)
{
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    if (!taskExecutor_ || !callback) {
        return;
    }

    GetSpanStringDataHelper(callback, syncMode);
#endif
}

void ClipboardImpl::GetSpanStringDataHelper(
    const std::function<void(std::vector<uint8_t>&, const std::string&)>& callback, bool syncMode)
{
    auto task = [callback, weak = WeakClaim(this)]() {
        auto clip = weak.Upgrade();
        CHECK_NULL_VOID(clip);
        auto hasData = OHOS::MiscServices::PasteboardClient::GetInstance()->HasPasteData();
        CHECK_NULL_VOID(hasData);
        OHOS::MiscServices::PasteData pasteData;
        auto getDataRes = OHOS::MiscServices::PasteboardClient::GetInstance()->GetPasteData(pasteData);
        CHECK_NULL_VOID(getDataRes);
        std::vector<uint8_t> arr;
        clip->ProcessSpanStringData(arr, pasteData);
        std::string text;
        auto textData = pasteData.GetPrimaryText();
        if (textData) {
            text = *textData;
        }
        callback(arr, text);
    };
    if (syncMode) {
        taskExecutor_->PostSyncTask(task, TaskExecutor::TaskType::PLATFORM, "ArkUIClipboardGetSpanStringDataSync");
    } else {
        taskExecutor_->PostTask(task, TaskExecutor::TaskType::PLATFORM, "ArkUIClipboardGetSpanStringDataAsync");
    }
}

void ClipboardImpl::ProcessSpanStringData(std::vector<uint8_t>& arr, const OHOS::MiscServices::PasteData& pasteData)
{
    for (const auto& pasteDataRecord : pasteData.AllRecords()) {
        if (pasteDataRecord == nullptr) {
            continue;
        }
        if (pasteDataRecord->GetCustomData() != nullptr) {
            auto itemData = pasteDataRecord->GetCustomData()->GetItemData();
            if (itemData.find(SPAN_STRING_TAG) != itemData.end()) {
                arr = itemData[SPAN_STRING_TAG];
                return;
            }
        }
        if (pasteDataRecord->GetHtmlText() != nullptr) {
            auto htmlText = pasteDataRecord->GetHtmlText();
            HtmlToSpan toSpan;
            auto spanStr = toSpan.ToSpanString(*htmlText);
            if (spanStr) {
                spanStr->EncodeTlv(arr);
                return;
            }
        }
    }
}

void ClipboardImpl::GetPixelMapDataSync(const std::function<void(const RefPtr<PixelMap>&)>& callback)
{
    RefPtr<PixelMap> pixmap;
    taskExecutor_->PostSyncTask(
        [&pixmap]() {
            auto has = OHOS::MiscServices::PasteboardClient::GetInstance()->HasPasteData();
            CHECK_NULL_VOID(has);
            OHOS::MiscServices::PasteData pasteData;
            auto ok = OHOS::MiscServices::PasteboardClient::GetInstance()->GetPasteData(pasteData);
            CHECK_NULL_VOID(ok);
            auto imageData = pasteData.GetPrimaryPixelMap();
            CHECK_NULL_VOID(imageData);
            pixmap = AceType::MakeRefPtr<PixelMapOhos>(imageData);
        },
        TaskExecutor::TaskType::PLATFORM, "ArkUIClipboardGetImageDataSync");
    callback(pixmap);
}

void ClipboardImpl::GetPixelMapDataAsync(const std::function<void(const RefPtr<PixelMap>&)>& callback)
{
    taskExecutor_->PostTask(
        [callback, weakExecutor = WeakClaim(RawPtr(taskExecutor_))]() {
            auto taskExecutor = weakExecutor.Upgrade();
            CHECK_NULL_VOID(taskExecutor);
            auto has = OHOS::MiscServices::PasteboardClient::GetInstance()->HasPasteData();
            if (!has) {
                LOGW("SystemKeyboardData is not exist from MiscServices");
                taskExecutor->PostTask(
                    [callback]() { callback(nullptr); }, TaskExecutor::TaskType::UI, "ArkUIClipboardHasDataFailed");
                return;
            }
            OHOS::MiscServices::PasteData pasteData;
            auto ok = OHOS::MiscServices::PasteboardClient::GetInstance()->GetPasteData(pasteData);
            if (!ok) {
                LOGW("Get SystemKeyboardData fail from MiscServices");
                taskExecutor->PostTask(
                    [callback]() { callback(nullptr); }, TaskExecutor::TaskType::UI, "ArkUIClipboardGetDataFailed");
                return;
            }
            auto imageData = pasteData.GetPrimaryPixelMap();
            if (!imageData) {
                LOGW("Get SystemKeyboardImageData fail from MiscServices");
                taskExecutor->PostTask(
                    [callback]() { callback(nullptr); },
                    TaskExecutor::TaskType::UI, "ArkUIClipboardGetImageDataFailed");
                return;
            }
            auto result = AceType::MakeRefPtr<PixelMapOhos>(imageData);
            taskExecutor->PostTask(
                [callback, result]() { callback(result); },
                TaskExecutor::TaskType::UI, "ArkUIClipboardGetImageDataCallback");
        },
        TaskExecutor::TaskType::PLATFORM, "ArkUIClipboardGetImageDataAsync");
}
#endif

void ClipboardImpl::Clear() {}

} // namespace OHOS::Ace
