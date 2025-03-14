/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "bridge/cj_frontend/interfaces/cj_ffi/cj_text_clock_ffi.h"

#include "cj_lambda.h"
#include "base/utils/string_utils.h"
#include "core/common/ace_application_info.h"
#include "bridge/cj_frontend/interfaces/cj_ffi/cj_view_abstract_ffi.h"
#include "bridge/cj_frontend/interfaces/cj_ffi/utils.h"
#include "bridge/common/utils/utils.h"
#include "core/components/common/properties/text_style_parser.h"


using namespace OHOS::Ace;
using namespace OHOS::FFI;
using namespace OHOS::Ace::Framework;

namespace OHOS::Ace::Framework {

namespace {
const std::vector<FontStyle> FONT_STYLES = { FontStyle::NORMAL, FontStyle::ITALIC };
const std::string DEFAULT_FORMAT_API_ELEVEN = "aa hh:mm:ss";
const std::string DEFAULT_FORMAT_API_TEN = "hms";
} // namespace

NativeTextClockController::NativeTextClockController() : FFIData()
{
    LOGI("Native TextAreaController constructed: %{public}" PRId64, GetID());
}

void NativeTextClockController::Start()
{
    if (controller_) {
        controller_->Start();
    }
}

void NativeTextClockController::Stop()
{
    if (controller_) {
        controller_->Stop();
    }
}

namespace {
constexpr int32_t TWENTY_FOUR_HOUR_BASE = 24;
constexpr int32_t HOURS_WEST_LOWER_LIMIT = -14;
constexpr int32_t HOURS_WEST_UPPER_LIMIT = 12;
constexpr int32_t HOURS_WEST_GEOGRAPHICAL_LOWER_LIMIT = -12;

bool HoursWestIsValid_(int32_t hoursWest)
{
    if (hoursWest < HOURS_WEST_LOWER_LIMIT || hoursWest > HOURS_WEST_UPPER_LIMIT) {
        return false;
    }
    if (hoursWest < HOURS_WEST_GEOGRAPHICAL_LOWER_LIMIT) {
        hoursWest += TWENTY_FOUR_HOUR_BASE;
    }
    return true;
}
} // namespace

} // namespace OHOS::Ace::Framework

extern "C" {
VectorNativeTextShadow FFICJCreateVectorNativeTextShadow(int64_t size)
{
    LOGI("Create NativeTextShadow Vector");
    return new std::vector<NativeTextShadow>(size);
}

void FFICJVectorNativeTextShadowSetElement(
    VectorNativeTextShadow vec, int64_t index, NativeTextShadow textShadow)
{
    LOGI("NativeTextShadow Vector Set Element");
    auto actualVec = reinterpret_cast<std::vector<NativeTextShadow>*>(vec);
    (*actualVec)[index] = textShadow;
    LOGI("NativeTextShadow Vector Set Element Success");
}

void FFICJVectorNativeTextShadowDelete(VectorNativeTextShadow vec)
{
    auto actualVec = reinterpret_cast<std::vector<NativeTextShadow>*>(vec);
    delete actualVec;
}

void FfiOHOSAceFrameworkTextClockCreate(int32_t timeZoneOffset, int64_t controllerId)
{
    auto textClock = TextClockModel::GetInstance()->Create();
    if (HoursWestIsValid_(timeZoneOffset)) {
        TextClockModel::GetInstance()->SetHoursWest(timeZoneOffset);
    } else {
        LOGE("timeZoneOffset is invalid");
    }

    auto controller = FFIData::GetData<NativeTextClockController>(controllerId);
    if (controller != nullptr) {
        controller->SetController(textClock);
    } else {
        LOGE("textClockControllerId is invalid ");
    }
}

void FfiOHOSAceFrameworkTextClockFormat(const char* value)
{
    std::string format = static_cast<std::string>(value);
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        if (format.length() == 0) {
            TextClockModel::GetInstance()->SetFormat(DEFAULT_FORMAT_API_ELEVEN);
            return;
        }
    } else {
        std::regex pattern(
            R"(^([Yy]*[_|\W\s]*[M]*[_|\W\s]*[d]*[_|\W\s]*[D]*[_|\W\s]*[Hh]*[_|\W\s]*[m]*[_|\W\s]*[s]*[_|\W\s]*[S]*)$)");
        if (format.length() == 0 || !StringUtils::IsAscii(format) || !std::regex_match(format, pattern)) {
            TextClockModel::GetInstance()->SetFormat(DEFAULT_FORMAT_API_TEN);
            return;
        }
    }

    TextClockModel::GetInstance()->SetFormat(format);
}

void FfiOHOSAceFrameworkTextClockOnChange(void (*callback)(int64_t timeStamp))
{
    auto lambda = [lambda = CJLambda::Create(callback)](
                      const std::string& value) -> void { lambda(std::atol(value.c_str())); };
    TextClockModel::GetInstance()->SetOnDateChange(lambda);
}

void FfiOHOSAceFrameworkTextClockTextColor(uint32_t color)
{
    TextClockModel::GetInstance()->SetTextColor(Color(color));
}

void FfiOHOSAceFrameworkTextClockFontSize(double size, int32_t unit)
{
    CalcDimension fontSize = CalcDimension(size, DimensionUnit(unit));
    TextClockModel::GetInstance()->SetFontSize(fontSize);
}

void FfiOHOSAceFrameworkTextClockFontWeight(const char* fontWeight)
{
    TextClockModel::GetInstance()->SetFontWeight(ConvertStrToFontWeight(fontWeight));
}

void FfiOHOSAceFrameworkTextClockFontStyle(int32_t fontStyle)
{
    if (!Utils::CheckParamsValid(fontStyle, FONT_STYLES.size())) {
        LOGE("invalid value for font style");
        return;
    }
    TextClockModel::GetInstance()->SetItalicFontStyle(FONT_STYLES[fontStyle]);
}

void FfiOHOSAceFrameworkTextClockFontFamily(const char* fontFamily)
{
    std::vector<std::string> fontFamilies;
    fontFamilies = ConvertStrToFontFamilies(fontFamily);
    TextClockModel::GetInstance()->SetFontFamily(fontFamilies);
}

void FfiOHOSAceFrameworkTextClockTextShadow(VectorStringPtr vecContent)
{
    auto nativeTextShadowVec = *reinterpret_cast<std::vector<NativeTextShadow>*>(vecContent);
    
    std::vector<Shadow> shadows(nativeTextShadowVec.size());
    for (size_t i = 0; i < nativeTextShadowVec.size(); i++) {
        Dimension dOffsetX(nativeTextShadowVec[i].offsetX, DimensionUnit::VP);
        Dimension dOffsetY(nativeTextShadowVec[i].offsetY, DimensionUnit::VP);

        shadows[i].SetBlurRadius(nativeTextShadowVec[i].radius);
        shadows[i].SetOffsetX(dOffsetX.Value());
        shadows[i].SetOffsetY(dOffsetY.Value());
        shadows[i].SetColor(Color(nativeTextShadowVec[i].color));
    }
    
    TextClockModel::GetInstance()->SetTextShadow(shadows);
}

void FfiOHOSAceFrameworkTextClockFontFeature(const char* fontFeature)
{
    std::string fontFeatureSettings = fontFeature;
    TextClockModel::GetInstance()->SetFontFeature(ParseFontFeatureSettings(fontFeatureSettings));
}

int64_t FfiOHOSAceFrameworkTextClockControllerCtor()
{
    auto controller = FFIData::Create<NativeTextClockController>();
    return controller->GetID();
}

void FfiOHOSAceFrameworkTextClockControllerStart(int64_t selfID)
{
    auto self = FFIData::GetData<NativeTextClockController>(selfID);
    if (self != nullptr) {
        self->Start();
    } else {
        LOGE("FfiTextArea: invalid textClockController");
    }
}

void FfiOHOSAceFrameworkTextClockControllerStop(int64_t selfID)
{
    auto self = FFIData::GetData<NativeTextClockController>(selfID);
    if (self != nullptr) {
        self->Stop();
    } else {
        LOGE("FfiTextArea: invalid textClockController");
    }
}
}
