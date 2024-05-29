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
#include "core/image/image_file_cache.h"

#include <dirent.h>
#include <fstream>
#include <sys/stat.h>

#include "base/image/image_packer.h"
#include "base/image/image_source.h"
#include "base/log/ace_trace.h"
#include "base/log/dump_log.h"
#include "base/log/log_wrapper.h"
#include "base/thread/background_task_executor.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/image/image_loader.h"
#include "core/image/image_source_info.h"

#ifdef USE_ROSEN_DRAWING
#include "core/components_ng/image_provider/adapter/rosen/drawing_image_data.h"
#endif

namespace OHOS::Ace {
ImageFileCache::ImageFileCache() = default;
ImageFileCache::~ImageFileCache() = default;

namespace {
const std::string ASTC_SUFFIX = ".astc";
const std::string CONVERT_ASTC_FORMAT = "image/astc/4*4";
const std::string SLASH = "/";
const std::string BACKSLASH = "\\";
const mode_t CHOWN_RW_UG = 0660;
const std::string SVG_FORMAT = "image/svg+xml";
bool EndsWith(const std::string& str, const std::string& substr)
{
    return str.rfind(substr) == (str.length() - substr.length());
}
bool IsAstcFile(const char fileName[])
{
    auto fileNameStr = std::string(fileName);
    return (fileNameStr.length() >= ASTC_SUFFIX.length()) && EndsWith(fileNameStr, ASTC_SUFFIX);
}
}

void ImageFileCache::SetImageCacheFilePath(const std::string& cacheFilePath)
{
    std::unique_lock<std::shared_mutex> lock(cacheFilePathMutex_);
    if (cacheFilePath_.empty()) {
        cacheFilePath_ = cacheFilePath;
    }
}

std::string ImageFileCache::GetImageCacheFilePath()
{
    std::shared_lock<std::shared_mutex> lock(cacheFilePathMutex_);
    return cacheFilePath_;
}

std::string ImageFileCache::GetImageCacheFilePath(const std::string& url)
{
    return ConstructCacheFilePath(std::to_string(std::hash<std::string> {}(url)));
}

std::string ImageFileCache::ConstructCacheFilePath(const std::string& fileName)
{
    std::shared_lock<std::shared_mutex> lock(cacheFilePathMutex_);
#if !defined(PREVIEW)
    return cacheFilePath_ + SLASH + fileName;
#elif defined(MAC_PLATFORM) || defined(LINUX_PLATFORM)

    return "/tmp/" + fileName;
#elif defined(WINDOWS_PLATFORM)
    char* pathvar = getenv("TEMP");
    if (!pathvar) {
        return std::string("C:\\Windows\\Temp") + BACKSLASH + fileName;
    }
    return std::string(pathvar) + BACKSLASH + fileName;
#endif
}

bool ImageFileCache::WriteFile(const std::string& url, const void* const data, size_t size,
    const std::string& fileCacheKey, const std::string& suffix)
{
    std::string writeFilePath = ConstructCacheFilePath(fileCacheKey + suffix);
    char realPath[PATH_MAX] = { 0x00 };
    if (!RealPath(writeFilePath.c_str(), realPath)) {
        return false;
    }
#ifdef WINDOWS_PLATFORM
    std::ofstream outFile(writeFilePath, std::ios::binary);
#else
    std::ofstream outFile(writeFilePath, std::fstream::out);
#endif
    if (!outFile.is_open()) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, "open cache file failed, cannot write.");
        return false;
    }
    outFile.write(reinterpret_cast<const char*>(data), size);
    TAG_LOGI(
        AceLogTag::ACE_IMAGE, "write image cache: %{public}s %{private}s", url.c_str(), writeFilePath.c_str());
#ifndef WINDOWS_PLATFORM
    if (chmod(writeFilePath.c_str(), CHOWN_RW_UG) != 0) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, "write image cache chmod failed: %{public}s %{private}s",
            url.c_str(), writeFilePath.c_str());
    }
#endif
    return true;
}

std::string ImageFileCache::GetImageCacheKey(const std::string& fileName)
{
    size_t suffixStartAt = fileName.find_last_of(".");
    return suffixStartAt == std::string::npos ? fileName : fileName.substr(0, suffixStartAt);
}

void ImageFileCache::SetCacheFileLimit(size_t cacheFileLimit)
{
    TAG_LOGI(AceLogTag::ACE_IMAGE, "User Set file cache limit size : %{public}d", static_cast<int32_t>(cacheFileLimit));
    fileLimit_ = cacheFileLimit;
}

void ImageFileCache::SetClearCacheFileRatio(float clearRatio)
{
    // clearRatio must in (0, 1].
    if (clearRatio < 0) {
        clearRatio = 0.1f;
    } else if (clearRatio > 1) {
        clearRatio = 1.0f;
    }
    clearCacheFileRatio_ = clearRatio;
}

RefPtr<NG::ImageData> ImageFileCache::GetDataFromCacheFile(const std::string& url, const std::string& suffix)
{
    std::lock_guard<std::mutex> lock(cacheFileInfoMutex_);
    auto filePath = GetCacheFilePathInner(url, suffix);
    if (filePath == "") {
        return nullptr;
    }
    auto cacheFileLoader = AceType::MakeRefPtr<FileImageLoader>();
    auto rsData = cacheFileLoader->LoadImageData(ImageSourceInfo(std::string("file:/").append(filePath)));
#ifndef USE_ROSEN_DRAWING
    return NG::ImageData::MakeFromDataWrapper(&rsData);
#else
    return AceType::MakeRefPtr<NG::DrawingImageData>(rsData);
#endif
}

void ImageFileCache::SaveCacheInner(const std::string& cacheKey, const std::string& suffix, size_t cacheSize,
    std::vector<std::string>& removeVector)
{
    auto cacheFileName = cacheKey + suffix;
    auto iter = fileNameToFileInfoPos_.find(cacheKey);
    auto cacheTime = time(nullptr);
    auto convertAstcThreshold = SystemProperties::GetImageFileCacheConvertAstcThreshold();
    if (iter != fileNameToFileInfoPos_.end()) {
        auto infoIter = iter->second;
        cacheFileInfo_.splice(cacheFileInfo_.begin(), cacheFileInfo_, infoIter);
        cacheFileSize_ = cacheFileSize_ + cacheSize - infoIter->fileSize;
        removeVector.push_back(ConstructCacheFilePath(infoIter->fileName));

        infoIter->fileName = cacheFileName;
        infoIter->fileSize = cacheSize;
        infoIter->accessTime = cacheTime;
        infoIter->accessCount = static_cast<uint32_t>(suffix == ASTC_SUFFIX ? convertAstcThreshold : 1);
    } else {
        cacheFileInfo_.emplace_front(cacheFileName, cacheSize, cacheTime,
            suffix == ASTC_SUFFIX ? convertAstcThreshold : 1);
        fileNameToFileInfoPos_[cacheKey] = cacheFileInfo_.begin();
        cacheFileSize_ += cacheSize;
    }
    // check if cache files too big.
    if (cacheFileSize_ > fileLimit_) {
        auto removeSizeTarget = fileLimit_ * clearCacheFileRatio_;
        size_t removeSize = 0;
        auto iter = cacheFileInfo_.rbegin();
        while (removeSize < removeSizeTarget && iter != cacheFileInfo_.rend()) {
            removeSize += iter->fileSize;
            removeVector.push_back(ConstructCacheFilePath(iter->fileName));
            fileNameToFileInfoPos_.erase(GetImageCacheKey(iter->fileName));
            iter++;
        }
        cacheFileInfo_.erase(iter.base(), cacheFileInfo_.end());
        cacheFileSize_ -= removeSize;
    }
}

void ImageFileCache::EraseCacheFile(const std::string &url)
{
    auto fileCacheKey = std::to_string(std::hash<std::string> {}(url));
    {
        std::scoped_lock<std::mutex> lock(cacheFileInfoMutex_);
        // 1. first check if file has been cached.
        auto iter = fileNameToFileInfoPos_.find(fileCacheKey);
        if (iter != fileNameToFileInfoPos_.end()) {
            auto infoIter = iter->second;
            auto removeFile = ConstructCacheFilePath(infoIter->fileName);
            if (remove(removeFile.c_str()) != 0) {
                TAG_LOGW(AceLogTag::ACE_IMAGE, "remove file %{private}s failed.", removeFile.c_str());
                return;
            }
            cacheFileInfo_.erase(infoIter);
            cacheFileSize_ -= infoIter->fileSize;
            fileNameToFileInfoPos_.erase(fileCacheKey);
        }
    }
}

void ImageFileCache::WriteCacheFile(
    const std::string& url, const void* const data, size_t size, const std::string& suffix)
{
    if (size > fileLimit_) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, "file size is %{public}d, greater than limit %{public}d, cannot cache",
            static_cast<int32_t>(size), static_cast<int32_t>(fileLimit_));
        return;
    }
    auto fileCacheKey = std::to_string(std::hash<std::string> {}(url));
    {
        std::scoped_lock<std::mutex> lock(cacheFileInfoMutex_);
        // 1. first check if file has been cached.
        auto iter = fileNameToFileInfoPos_.find(fileCacheKey);
        if (iter != fileNameToFileInfoPos_.end()) {
            auto infoIter = iter->second;
            // either suffix not specified, or fileName ends with the suffix
            if (suffix == "" || EndsWith(infoIter->fileName, suffix)) {
                TAG_LOGI(AceLogTag::ACE_IMAGE, "file has been wrote %{private}s", infoIter->fileName.c_str());
                return;
            }
        }
    }

#ifndef ACE_UNITTEST
    // 2. if not in dist, write file into disk.
    if (!WriteFile(url, data, size, fileCacheKey, suffix)) {
        return;
    }
#endif

    std::vector<std::string> removeVector;
    {
        std::scoped_lock<std::mutex> lock(cacheFileInfoMutex_);
        SaveCacheInner(fileCacheKey, suffix, size, removeVector);
    }
    // 3. clear files removed from cache list.
    ClearCacheFile(removeVector);
}

void ImageFileCache::ConvertToAstcAndWriteToFile(const std::string& fileCacheKey, const std::string& filePath,
    const std::string& url)
{
    ACE_FUNCTION_TRACE();
    RefPtr<ImageSource> imageSource = ImageSource::Create(filePath);
    if (!imageSource || imageSource->GetFrameCount() != 1) {
        TAG_LOGI(AceLogTag::ACE_IMAGE, "Image frame count is not 1, will not convert to astc. %{public}s",
            fileCacheKey.c_str());
        return;
    }
    if (imageSource->GetEncodedFormat() == SVG_FORMAT) {
        TAG_LOGI(AceLogTag::ACE_IMAGE, "Image is svg, will not convert to astc. %{public}s",
            fileCacheKey.c_str());
        return;
    }

    RefPtr<ImagePacker> imagePacker = ImagePacker::Create();
    PackOption option;
    option.format = CONVERT_ASTC_FORMAT;
    auto pixelMap = imageSource->CreatePixelMap({-1, -1});
    if (pixelMap == nullptr) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, "Get pixel map failed, will not convert to astc. %{public}s",
            fileCacheKey.c_str());
        return;
    }

    auto astcFileName = fileCacheKey + ASTC_SUFFIX;
    auto astcFilePath = ConstructCacheFilePath(astcFileName);
    imagePacker->StartPacking(astcFilePath, option);
    imagePacker->AddImage(*pixelMap);
    int64_t packedSize = 0;
    if (imagePacker->FinalizePacking(packedSize)) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, "convert to astc failed. %{public}s", fileCacheKey.c_str());
        return;
    }
#if !defined(WINDOWS_PLATFORM) && !defined(ACE_UNITTEST)
    if (chmod(astcFilePath.c_str(), CHOWN_RW_UG) != 0) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, "convert to astc chmod failed: %{public}s %{private}s",
            url.c_str(), astcFilePath.c_str());
    }
#endif

    std::vector<std::string> removeVector;
    {
        std::scoped_lock<std::mutex> lock(cacheFileInfoMutex_);
        removeVector.push_back(filePath);

        auto infoIter = fileNameToFileInfoPos_[fileCacheKey];
        cacheFileSize_ = cacheFileSize_ + packedSize - infoIter->fileSize;
        infoIter->fileName = astcFileName;
        infoIter->fileSize = static_cast<uint64_t>(packedSize);
    }
    // remove the old file before convert
    ClearCacheFile(removeVector);
    TAG_LOGI(AceLogTag::ACE_IMAGE, "write image astc cache: %{public}s %{private}s", url.c_str(), astcFilePath.c_str());
}

void ImageFileCache::ClearCacheFile(const std::vector<std::string>& removeFiles)
{
#ifndef ACE_UNITTEST
    for (auto&& iter : removeFiles) {
        if (remove(iter.c_str()) != 0) {
            TAG_LOGW(AceLogTag::ACE_IMAGE, "remove file %{private}s failed.", iter.c_str());
            continue;
        }
    }
#endif
}

std::string ImageFileCache::GetCacheFilePath(const std::string& url)
{
    std::scoped_lock<std::mutex> lock(cacheFileInfoMutex_);
    return GetCacheFilePathInner(url, "");
}

std::string ImageFileCache::GetCacheFilePathInner(const std::string& url, const std::string& suffix)
{
    auto fileCacheKey = std::to_string(std::hash<std::string> {}(url));
    auto iter = fileNameToFileInfoPos_.find(fileCacheKey);
    // either suffix not specified, or fileName ends with the suffix
    if (iter != fileNameToFileInfoPos_.end() && (suffix == "" || EndsWith(iter->second->fileName, suffix))) {
        auto infoIter = iter->second;
        cacheFileInfo_.splice(cacheFileInfo_.begin(), cacheFileInfo_, infoIter);
        infoIter->accessTime = time(nullptr);
        infoIter->accessCount++;
        auto filePath = ConstructCacheFilePath(infoIter->fileName);
        if (SystemProperties::IsImageFileCacheConvertAstcEnabled() &&
            infoIter->accessCount == static_cast<uint32_t>(SystemProperties::GetImageFileCacheConvertAstcThreshold())) {
            BackgroundTaskExecutor::GetInstance().PostTask(
                [this, fileCacheKey, filePath, url] () {
                    ConvertToAstcAndWriteToFile(fileCacheKey, filePath, url);
                },
                BgTaskPriority::LOW);
        }
        return filePath;
    }
    return "";
}

void ImageFileCache::SetCacheFileInfo()
{
    std::lock_guard<std::mutex> lock(cacheFileInfoMutex_);
    // Set cache file information only once.
    if (hasSetCacheFileInfo_) {
        return;
    }
    std::string cacheFilePath = GetImageCacheFilePath();
    std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(cacheFilePath.c_str()), closedir);
    if (dir == nullptr) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, "cache file path wrong! maybe it is not set.");
        return;
    }
    size_t cacheFileSize = 0;
    dirent* filePtr = readdir(dir.get());
    while (filePtr != nullptr) {
        // skip . or ..
        if (filePtr->d_name[0] != '.') {
            std::string filePath = cacheFilePath + SLASH + std::string(filePtr->d_name);
            struct stat fileStatus;
            if (stat(filePath.c_str(), &fileStatus) == -1) {
                filePtr = readdir(dir.get());
                continue;
            }
            cacheFileInfo_.emplace_front(filePtr->d_name, fileStatus.st_size, fileStatus.st_atime,
                IsAstcFile(filePtr->d_name) ? SystemProperties::GetImageFileCacheConvertAstcThreshold() : 1);
            std::string fileCacheKey = GetImageCacheKey(std::string(filePtr->d_name));
            fileNameToFileInfoPos_[fileCacheKey] = cacheFileInfo_.begin();
            cacheFileSize += static_cast<size_t>(fileStatus.st_size);
        }
        filePtr = readdir(dir.get());
    }
    cacheFileInfo_.sort();
    cacheFileSize_ = cacheFileSize;
    hasSetCacheFileInfo_ = true;
}

void ImageFileCache::DumpCacheInfo()
{
    auto cacheFileInfoSize = cacheFileInfo_.size();
    auto fileLimit = static_cast<int32_t>(fileLimit_);
    auto cacheFileSize = static_cast<int32_t>(cacheFileSize_);
    DumpLog::GetInstance().Print("------------ImageCacheInfo------------");
    DumpLog::GetInstance().Print("User set ImageFileCacheSize : " + std::to_string(fileLimit) + "(B)");
    DumpLog::GetInstance().Print("cacheFileSize: " + std::to_string(cacheFileSize) + "(B)");
    if (cacheFileInfoSize == 0) {
        return;
    }
    size_t totalCount = 0;
    for (const auto& item : cacheFileInfo_) {
        auto filePath = ConstructCacheFilePath(item.fileName);
        auto fileSize = item.fileSize;
        totalCount += fileSize;
        DumpLog::GetInstance().Print(
            "fileCache Obj of filePath: " + filePath + ", fileSize: " + std::to_string(fileSize) + "(B)" +
            ", accessCount: " + std::to_string(item.accessCount));
    }
    DumpLog::GetInstance().Print("FileCache total size: " + std::to_string(totalCount) + "(B)");
}
} // namespace OHOS::Ace
