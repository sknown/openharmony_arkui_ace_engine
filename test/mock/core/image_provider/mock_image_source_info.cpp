/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <memory>
#include <regex>
#include <utility>

#include "core/components/common/layout/constants.h"
#include "core/image/image_source_info.h"
#include "core/pipeline/base/constants.h"

namespace OHOS::Ace {
namespace {
constexpr uint32_t FILE_SUFFIX_LEN = 4;
} // namespace

bool ImageSourceInfo::IsSVGSource(const std::string& src, InternalResource::ResourceId resourceId)
{
    // 4 is the length of ".svg".
    return (src.size() > FILE_SUFFIX_LEN && src.substr(src.size() - FILE_SUFFIX_LEN) == ".svg") ||
           (src.empty() && resourceId > InternalResource::ResourceId::SVG_START &&
               resourceId < InternalResource::ResourceId::SVG_END);
}

bool ImageSourceInfo::IsPngSource(const std::string& src, InternalResource::ResourceId resourceId)
{
    return false;
}

bool ImageSourceInfo::IsValidBase64Head(const std::string& uri, const std::string& pattern)
{
    return true;
}

bool ImageSourceInfo::IsUriOfDataAbilityEncoded(const std::string& uri, const std::string& pattern)
{
    std::regex regular(pattern);
    return std::regex_match(uri, regular);
}

SrcType ImageSourceInfo::ResolveURIType(const std::string& uri)
{
    if (uri.empty()) {
        return SrcType::UNSUPPORTED;
    }
    auto iter = uri.find_first_of(':');
    if (iter == std::string::npos) {
        return SrcType::ASSET;
    }
    std::string head = uri.substr(0, iter);
    std::transform(head.begin(), head.end(), head.begin(), [](unsigned char c) { return std::tolower(c); });
    if (head == "http" || head == "https") {
        return SrcType::NETWORK;
    } else if (head == "file") {
        if (IsUriOfDataAbilityEncoded(uri, "^file://media/.*thumbnail.*$")) {
            return SrcType::DATA_ABILITY_DECODED;
        } else if (IsUriOfDataAbilityEncoded(uri, "^file://media/.*")) {
            return SrcType::DATA_ABILITY;
        }
        return SrcType::FILE;
    } else if (head == "internal") {
        return SrcType::INTERNAL;
    } else if (head == "data") {
        static constexpr char BASE64_PATTERN[] =
            "^data:image/(jpeg|JPEG|jpg|JPG|png|PNG|ico|ICO|gif|GIF|bmp|BMP|webp|WEBP);base64$";
        if (IsValidBase64Head(uri, BASE64_PATTERN)) {
            return SrcType::BASE64;
        }
        return SrcType::UNSUPPORTED;
    } else if (head == "memory") {
        return SrcType::MEMORY;
    } else if (head == "resource") {
        return SrcType::RESOURCE;
    } else if (head == "dataability" || head == "datashare") {
        if (IsUriOfDataAbilityEncoded(uri, "^dataability://.*?/media/.*thumbnail.*$") ||
            IsUriOfDataAbilityEncoded(uri, "^datashare://.*?/media/.*thumbnail.*$")) {
            return SrcType::DATA_ABILITY_DECODED;
        }
        return SrcType::DATA_ABILITY;
    } else {
        return SrcType::UNSUPPORTED;
    }
}

ImageSourceInfo::ImageSourceInfo(std::string imageSrc, std::string bundleName, std::string moduleName, Dimension width,
    Dimension height, InternalResource::ResourceId resourceId, const RefPtr<PixelMap>& pixmap)
    : src_(std::move(imageSrc)), bundleName_(std::move(bundleName)), moduleName_(std::move(moduleName)),
      sourceWidth_(width), sourceHeight_(height), resourceId_(resourceId), pixmap_(pixmap),
      isSvg_(IsSVGSource(src_, resourceId_)), isPng_(IsPngSource(src_, resourceId_)), srcType_(ResolveSrcType())
{}

ImageSourceInfo::ImageSourceInfo(const std::shared_ptr<std::string>& imageSrc, std::string bundleName,
    std::string moduleName, Dimension width, Dimension height, InternalResource::ResourceId resourceId,
    const RefPtr<PixelMap>& pixmap)
    : bundleName_(std::move(bundleName)), moduleName_(std::move(moduleName)), sourceWidth_(width),
      sourceHeight_(height), resourceId_(resourceId), pixmap_(pixmap), isSvg_(IsSVGSource(src_, resourceId_)),
      isPng_(IsPngSource(src_, resourceId_)), srcType_(ResolveSrcType())
{}

SrcType ImageSourceInfo::ResolveSrcType() const
{
    if (pixmap_) {
        return SrcType::PIXMAP;
    }
    if (!src_.empty()) {
        return ResolveURIType(src_);
    }
    return SrcType::UNSUPPORTED;
}

void ImageSourceInfo::SetFillColor(const Color& color)
{
    fillColor_.emplace(color.GetValue());
}

bool ImageSourceInfo::operator==(const ImageSourceInfo& info) const
{
    // only svg uses fillColor
    if (isSvg_ && fillColor_ != info.fillColor_) {
        return false;
    }
    if (sourceWidth_ != info.sourceWidth_ || sourceHeight_ != info.sourceHeight_) {
        return false;
    }
    return ((!pixmap_ && !info.pixmap_) || (pixmap_ && info.pixmap_ && pixmap_ == info.pixmap_)) && src_ == info.src_ &&
           resourceId_ == info.resourceId_;
}

bool ImageSourceInfo::operator!=(const ImageSourceInfo& info) const
{
    return !(operator==(info));
}

void ImageSourceInfo::SetSrc(const std::string& src, std::optional<Color> fillColor)
{
    src_ = src;
    srcType_ = ResolveURIType(src_);
    resourceId_ = InternalResource::ResourceId::NO_ID;
    isSvg_ = IsSVGSource(src_, resourceId_);
    fillColor_ = fillColor;
    pixmap_ = nullptr;
}

const std::string& ImageSourceInfo::GetSrc() const
{
    return src_;
}

void ImageSourceInfo::SetResourceId(InternalResource::ResourceId id, std::optional<Color> fillColor)
{
    resourceId_ = id;
    srcType_ = SrcType::RESOURCE_ID;
    src_.clear();
    isSvg_ = IsSVGSource(src_, resourceId_);
    fillColor_ = fillColor;
    pixmap_ = nullptr;
}

InternalResource::ResourceId ImageSourceInfo::GetResourceId() const
{
    return resourceId_;
}

void ImageSourceInfo::SetPixMap(const RefPtr<PixelMap>& pixmap, std::optional<Color> fillColor) {}

bool ImageSourceInfo::IsInternalResource() const
{
    return src_.empty() && resourceId_ != InternalResource::ResourceId::NO_ID && !pixmap_;
}

bool ImageSourceInfo::IsValid() const
{
    return true;
}

bool ImageSourceInfo::IsPng() const
{
    return isPng_;
}

bool ImageSourceInfo::IsSvg() const
{
    return isSvg_;
}

bool ImageSourceInfo::IsPixmap() const
{
    return pixmap_ || srcType_ == SrcType::DATA_ABILITY_DECODED;
}

SrcType ImageSourceInfo::GetSrcType() const
{
    return srcType_;
}

std::string ImageSourceInfo::ToString() const
{
    return std::string("empty source");
}

void ImageSourceInfo::SetDimension(Dimension width, Dimension Height)
{
    sourceWidth_ = width;
    sourceHeight_ = Height;
}

bool ImageSourceInfo::IsSourceDimensionValid() const
{
    return sourceWidth_.IsValid() && sourceHeight_.IsValid();
}

Size ImageSourceInfo::GetSourceSize() const
{
    return Size(sourceWidth_.Value(), sourceHeight_.Value());
}

void ImageSourceInfo::Reset() {}

const std::optional<Color>& ImageSourceInfo::GetFillColor() const
{
    return fillColor_;
}

const RefPtr<PixelMap>& ImageSourceInfo::GetPixmap() const
{
    return pixmap_;
}

std::string ImageSourceInfo::GetKey() const
{
    return std::string("");
}

bool ImageSourceInfo::SupportObjCache() const
{
    return false;
}

const std::string& ImageSourceInfo::GetModuleName() const
{
    return moduleName_;
}

void ImageSourceInfo::SetBundleName(const std::string& bundleName)
{
    bundleName_ = bundleName;
}

void ImageSourceInfo::SetModuleName(const std::string& moduleName)
{
    moduleName_ = moduleName;
}
} // namespace OHOS::Ace
