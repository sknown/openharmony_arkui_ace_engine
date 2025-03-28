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

#include "core/components_ng/pattern/text_clock/text_clock_pattern.h"

#include <ctime>
#include <string>
#include <sys/time.h>

#include "base/i18n/localization.h"
#include "base/log/dump_log.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text_clock/text_clock_layout_property.h"
#include "core/components_ng/property/property.h"
#include "core/event/time/time_event_proxy.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t TOTAL_SECONDS_OF_HOUR = 60 * 60;
constexpr int32_t BASE_YEAR = 1900;
constexpr int32_t INTERVAL_OF_U_SECOND = 1000000;
constexpr int32_t MICROSECONDS_OF_MILLISECOND = 1000;
constexpr int32_t MILLISECONDS_OF_SECOND = 1000;
constexpr int32_t TOTAL_SECONDS_OF_MINUTE = 60;
constexpr int32_t STR_SIZE_ONE = 1;
constexpr int32_t STR_SIZE_TWO = 2;
constexpr bool ON_TIME_CHANGE = true;
const char CHAR_0 = '0';
const char CHAR_9 = '9';
const std::string STR_0 = "0";
const std::string DEFAULT_FORMAT = "aa hh:mm:ss";
const std::string DEFAULT_FORMAT_24HOUR = "HH:mm:ss";
const std::string FORM_FORMAT = "hh:mm";
const std::string FORM_FORMAT_24HOUR = "HH:mm";
constexpr char TEXTCLOCK_WEEK[] = "textclock.week";
constexpr char TEXTCLOCK_YEAR[] = "textclock.year";
constexpr char TEXTCLOCK_MONTH[] = "textclock.month";
constexpr char TEXTCLOCK_DAY[] = "textclock.day";

enum class TextClockElementIndex {
    CUR_YEAR_INDEX = 0,
    CUR_MONTH_INDEX = 1,
    CUR_DAY_INDEX = 2,
    CUR_HOUR_INDEX = 3,
    CUR_MINUTE_INDEX = 4,
    CUR_SECOND_INDEX = 5,
    CUR_MILLISECOND_INDEX = 6,
    CUR_AMPM_INDEX = 7,
    CUR_WEEK_INDEX = 8,
    CUR_SHORT_YEAR_INDEX = 9,
    CUR_SHORT_MONTH_INDEX = 10,
    CUR_SHORT_DAY_INDEX = 11,
    CUR_CENTISECOND_INDEX = 12,
    CUR_SHORT_WEEK_INDEX = 13,
    CUR_MAX_INDEX
};
enum class TextClockElementLen {
    ONLY_ONE_DATE_ELEMENT = 1,
    SHORT_YEAR_IN_YEAR_INDEX = 2,
    YEAR_FORMAT_MIN_LEN = 2,
    MON_DAY_FORMAT_MAX_LEN = 2,
    MON_DAY_FORMAT_MIN_LEN = 1,
    CENTISECOND_FORMAT_LEN = 2,
    MILLISECOND_FORMAT_LEN = 3,
    YEAR_WEEK_FORMAT_MAX_LEN = 4,
};
enum class TextClockWeekType {
    WEEK_FULL_TYPE = 1,
    WEEK_SHORT_TYPE = 2,
};
} // namespace

TextClockPattern::TextClockPattern()
{
    textClockController_ = MakeRefPtr<TextClockController>();
}

void TextClockPattern::OnAttachToFrameNode()
{
    InitTextClockController();
    InitUpdateTimeTextCallBack();
    auto* eventProxy = TimeEventProxy::GetInstance();
    if (eventProxy) {
        eventProxy->Register(WeakClaim(this));
    }
}

void TextClockPattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->RemoveVisibleAreaChangeNode(frameNode->GetId());
}

void TextClockPattern::UpdateTextLayoutProperty(
    RefPtr<TextClockLayoutProperty>& layoutProperty, RefPtr<TextLayoutProperty>& textLayoutProperty)
{
    if (layoutProperty->GetFontSize().has_value()) {
        textLayoutProperty->UpdateFontSize(layoutProperty->GetFontSize().value());
    }
    if (layoutProperty->GetFontWeight().has_value()) {
        textLayoutProperty->UpdateFontWeight(layoutProperty->GetFontWeight().value());
    }
    if (layoutProperty->GetTextColor().has_value()) {
        textLayoutProperty->UpdateTextColor(layoutProperty->GetTextColor().value());
    }
    if (layoutProperty->GetFontFamily().has_value() && !layoutProperty->GetFontFamily().value().empty()) {
        textLayoutProperty->UpdateFontFamily(layoutProperty->GetFontFamily().value());
    }
    if (layoutProperty->GetItalicFontStyle().has_value()) {
        textLayoutProperty->UpdateItalicFontStyle(layoutProperty->GetItalicFontStyle().value());
    }
    if (layoutProperty->GetTextShadow().has_value()) {
        textLayoutProperty->UpdateTextShadow(layoutProperty->GetTextShadow().value());
    }
    if (layoutProperty->GetFontFeature().has_value()) {
        textLayoutProperty->UpdateFontFeature(layoutProperty->GetFontFeature().value());
    }
}

void TextClockPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto textNode = GetTextNode();
    CHECK_NULL_VOID(textNode);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    auto textClockProperty = host->GetLayoutProperty<TextClockLayoutProperty>();
    CHECK_NULL_VOID(textClockProperty);
    textLayoutProperty->UpdateTextOverflow(TextOverflow::NONE);
    UpdateTextLayoutProperty(textClockProperty, textLayoutProperty);
    hourWest_ = GetHoursWest();
    delayTask_.Cancel();
    UpdateTimeText();
    FireBuilder();
}

void TextClockPattern::InitTextClockController()
{
    CHECK_NULL_VOID(textClockController_);
    if (textClockController_->HasInitialized()) {
        return;
    }
    textClockController_->OnStart([wp = WeakClaim(this)]() {
        auto textClock = wp.Upgrade();
        if (textClock) {
            textClock->isStart_ = true;
            textClock->UpdateTimeText();
        }
    });
    textClockController_->OnStop([wp = WeakClaim(this)]() {
        auto textClock = wp.Upgrade();
        if (textClock) {
            textClock->isStart_ = false;
            textClock->FireBuilder();
            textClock->delayTask_.Cancel();
        }
    });
}

void TextClockPattern::OnVisibleChange(bool isVisible)
{
    TAG_LOGI(AceLogTag::ACE_TEXT_CLOCK,
        "Clock is %{public}s and clock %{public}s running",
        isVisible ? "visible" : "invisible", isVisible ? "starts" : "stops");
    if (isVisible && !isSetVisible_) {
        isSetVisible_ = isVisible;
        UpdateTimeText();
    } else if (!isVisible) {
        isSetVisible_ = isVisible;
        delayTask_.Cancel();
    }
}

void TextClockPattern::OnVisibleAreaChange(bool visible)
{
    TAG_LOGI(AceLogTag::ACE_TEXT_CLOCK,
        "Clock is %{public}s the visible area and clock %{public}s running",
        visible ? "in" : "out of", visible ? "starts" : "stops");
    if (visible && !isInVisibleArea_) {
        isInVisibleArea_ = visible;
        UpdateTimeText();
    } else if (!visible) {
        isInVisibleArea_ = visible;
        delayTask_.Cancel();
    }
}

void TextClockPattern::RegistVisibleAreaChangeCallback()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);

    auto areaCallback = [weak = WeakClaim(this)](bool visible, double ratio) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnVisibleAreaChange(visible);
    };
    pipeline->RemoveVisibleAreaChangeNode(host->GetId());
    std::vector<double> ratioList = {0.0};
    pipeline->AddVisibleAreaChangeNode(host, ratioList, areaCallback, false);
}

void TextClockPattern::InitUpdateTimeTextCallBack()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
    if (context) {
        isForm_ = context->IsFormRender();
    }
    RegistVisibleAreaChangeCallback();
}

void TextClockPattern::UpdateTimeText(bool isTimeChange)
{
    if (!isStart_ || (!isTimeChange && (!isSetVisible_ || !isInVisibleArea_))) {
        return;
    }
    FireBuilder();
    if (!isForm_) {
        RequestUpdateForNextSecond();
    }
    std::string currentTime = GetCurrentFormatDateTime();
    if (currentTime.empty()) {
        return;
    }
    if (currentTime != prevTime_ || isTimeChange) {
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto textNode = GetTextNode();
        CHECK_NULL_VOID(textNode);
        auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_VOID(textLayoutProperty);
        textLayoutProperty->UpdateContent(currentTime); // update time text.
        auto textContext = textNode->GetRenderContext();
        CHECK_NULL_VOID(textContext);
        textContext->SetClipToFrame(false);
        textContext->UpdateClipEdge(false);
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);
        textNode->MarkModifyDone();
        prevTime_ = currentTime;
        FireChangeEvent();
    }
}

void TextClockPattern::RequestUpdateForNextSecond()
{
    struct timeval currentTime {};
    gettimeofday(&currentTime, nullptr);
    /**
     * 1 second = 1000 millisecond = 1000000 microsecond.
     * Millisecond is abbreviated as msec. Microsecond is abbreviated as usec.
     * unit of [delayTime] is msec, unit of [tv_usec] is usec
     * when [tv_usec] is 000100, (INTERVAL_OF_U_SECOND - timeUsec) / MICROSECONDS_OF_MILLISECOND = 999 msec
     * which will cause the delay task still arriving in current second, because 999000 + 000100 = 999100 < 1 second
     * so add an additional millisecond to modify the loss of precision during division
     */
    int32_t delayTime =
        (INTERVAL_OF_U_SECOND - static_cast<int32_t>(currentTime.tv_usec)) / MICROSECONDS_OF_MILLISECOND +
        1; // millisecond
    if (isForm_) {
        time_t current = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto* timeZoneTime = std::localtime(&current);
        // delay to next minute
        int32_t tempTime = (TOTAL_SECONDS_OF_MINUTE - timeZoneTime->tm_sec) * MILLISECONDS_OF_SECOND;
        delayTime += (tempTime - MILLISECONDS_OF_SECOND);
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(context->GetTaskExecutor());
    delayTask_.Reset([weak = WeakClaim(this)] {
        auto textClock = weak.Upgrade();
        CHECK_NULL_VOID(textClock);
        if (!textClock->isStart_) {
            return;
        }
        textClock->UpdateTimeText();
    });
    context->GetTaskExecutor()->PostDelayedTask(
        delayTask_, TaskExecutor::TaskType::UI, delayTime, "ArkUITextClockUpdateTimeText");
}

std::string TextClockPattern::GetCurrentFormatDateTime()
{
    time_t current = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto* timeZoneTime = std::localtime(&current);
    if (!std::isnan(hourWest_)) {
        current = current - int32_t(hourWest_ * TOTAL_SECONDS_OF_HOUR);
        timeZoneTime = std::gmtime(&current); // Convert to UTC time.
    }
    CHECK_NULL_RETURN(timeZoneTime, "");
    DateTime dateTime; // This is for i18n date time.
    dateTime.year = timeZoneTime->tm_year + BASE_YEAR;
    dateTime.month = timeZoneTime->tm_mon;
    dateTime.day = timeZoneTime->tm_mday;
    dateTime.hour = timeZoneTime->tm_hour;
    dateTime.minute = timeZoneTime->tm_min;
    dateTime.second = timeZoneTime->tm_sec;
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN) && !isForm_) {
        return Localization::GetInstance()->FormatDateTime(dateTime, GetFormat());
    }
    dateTime.week = timeZoneTime->tm_wday; // 0-6

    // parse input format
    formatElementMap_.clear();
    ParseInputFormat();

    // get date time from third party
    std::string dateTimeFormat = DEFAULT_FORMAT; // the format to get datetime value from the thirdlib
    dateTimeFormat = "yyyyMMdd";
    dateTimeFormat += is24H_ ? "HH" : "hh";
    dateTimeFormat += "mmss";
    dateTimeFormat += "SSS";
    std::string dateTimeValue = Localization::GetInstance()->FormatDateTime(dateTime, dateTimeFormat);
    std::string outputDateTime = ParseDateTime(dateTimeValue, timeZoneTime->tm_wday, timeZoneTime->tm_mon);
    return outputDateTime;
}

std::string TextClockPattern::ParseDateTime(const std::string& dateTimeValue, int32_t week, int32_t month)
{
    std::string language = Localization::GetInstance()->GetLanguage();
    // parse data time
    std::string tempdateTimeValue = dateTimeValue;
    std::string strAmPm = GetAmPm(tempdateTimeValue);
    auto strAmPmPos = tempdateTimeValue.find(strAmPm);
    if (!strAmPm.empty() && strAmPmPos != std::string::npos) {
        tempdateTimeValue.replace(strAmPmPos, strAmPm.length(), "");
    }
    std::vector<std::string> curDateTime = ParseDateTimeValue(tempdateTimeValue);
    curDateTime[(int32_t)(TextClockElementIndex::CUR_AMPM_INDEX)] = strAmPm;

    // parse week
    curDateTime[(int32_t)(TextClockElementIndex::CUR_WEEK_INDEX)] = GetWeek(false, week);
    curDateTime[(int32_t)(TextClockElementIndex::CUR_SHORT_WEEK_INDEX)] = GetWeek(true, week);
    // parse month
    if (strcmp(language.c_str(), "zh") != 0) {
        auto strMonth = GetMonth(month);
        curDateTime[(int32_t)(TextClockElementIndex::CUR_MONTH_INDEX)] = strMonth;
        curDateTime[(int32_t)(TextClockElementIndex::CUR_SHORT_MONTH_INDEX)] = strMonth;
    }
    // splice date time
    std::string outputDateTime = SpliceDateTime(curDateTime);
    if ((curDateTime[(int32_t)(TextClockElementIndex::CUR_YEAR_INDEX)] == "1900") || (outputDateTime == "")) {
        if (isForm_) {
            TextClockFormatElement tempFormatElement;
            std::vector<std::string> formSplitter = { "h", ":", "m" };
            formatElementMap_.clear();
            tempFormatElement.formatElement = formSplitter[0];
            tempFormatElement.elementKey = 'h';
            tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_HOUR_INDEX);
            formatElementMap_[0] = tempFormatElement;
            tempFormatElement.formatElement = formSplitter[1];
            tempFormatElement.elementKey = ':';
            tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_MAX_INDEX);
            formatElementMap_[1] = tempFormatElement;
            tempFormatElement.formatElement = formSplitter[2];
            tempFormatElement.elementKey = 'm';
            tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_MINUTE_INDEX);
            formatElementMap_[2] = tempFormatElement;
            outputDateTime = SpliceDateTime(curDateTime);
        } else {
            outputDateTime = dateTimeValue;
        }
    }
    return outputDateTime;
}

void TextClockPattern::ParseInputFormat()
{
    std::string inputFormat = GetFormat();
    if (inputFormat.find('H') != std::string::npos) {
        is24H_ = true;
    } else if (inputFormat.find('h') != std::string::npos || inputFormat.find('a') != std::string::npos) {
        is24H_ = false;
    }
    std::vector<std::string> formatSplitter;
    auto i = 0;
    auto j = 0;
    auto len = static_cast<int32_t>(inputFormat.length());
    std::string tempFormat = "";
    TextClockFormatElement tempFormatElement;
    tempFormatElement.formatElement = "";
    tempFormatElement.formatElementNum = 0;
    for (tempFormat = inputFormat[i]; i < len; i++) {
        if ((i + 1) < len) {
            if (inputFormat[i] == inputFormat[i + 1]) {
                tempFormat += inputFormat[i + 1]; // the same element format
                tempFormatElement.formatElementNum++;
            } else {
                tempFormatElement.formatElement = tempFormat;
                tempFormatElement.formatElementNum++;
                GetDateTimeIndex(inputFormat[i], tempFormatElement);
                tempFormatElement.elementKey = inputFormat[i];
                formatElementMap_[j] = tempFormatElement;
                j++;
                // clear current element
                tempFormat = "";
                tempFormat += inputFormat[i + 1]; // the next element format
                tempFormatElement.formatElement = "";
                tempFormatElement.formatElementNum = 0;
            }
        } else { // the last element
            tempFormatElement.formatElement = tempFormat;
            tempFormatElement.formatElementNum++;
            GetDateTimeIndex(inputFormat[i], tempFormatElement);
            tempFormatElement.elementKey = inputFormat[i];
            formatElementMap_[j] = tempFormatElement;
        }
    }
}

void TextClockPattern::GetDateTimeIndex(const char& element, TextClockFormatElement& tempFormatElement)
{
    switch (element) {
        case 'y':
            if (tempFormatElement.formatElementNum >= (int32_t)(TextClockElementLen::YEAR_WEEK_FORMAT_MAX_LEN)) {
                tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_YEAR_INDEX);
            } else {
                tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_SHORT_YEAR_INDEX);
            }
            break;
        case 'M':
            if (tempFormatElement.formatElementNum >= (int32_t)(TextClockElementLen::MON_DAY_FORMAT_MAX_LEN)) {
                tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_MONTH_INDEX);
            } else {
                tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_SHORT_MONTH_INDEX);
            }
            break;
        case 'd':
            if (tempFormatElement.formatElementNum >= (int32_t)(TextClockElementLen::MON_DAY_FORMAT_MAX_LEN)) {
                tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_DAY_INDEX);
            } else {
                tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_SHORT_DAY_INDEX);
            }
            break;
        case 'E':
            if (tempFormatElement.formatElementNum >= (int32_t)(TextClockElementLen::YEAR_WEEK_FORMAT_MAX_LEN)) {
                tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_WEEK_INDEX);
            } else {
                tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_SHORT_WEEK_INDEX);
            }
            break;
        case 'a':
            tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_AMPM_INDEX);
            break;
        case 'H':
        case 'h':
            tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_HOUR_INDEX);
            break;
        case 'm':
            tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_MINUTE_INDEX);
            break;
        case 's':
            tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_SECOND_INDEX);
            break;
        case 'S':
            if (tempFormatElement.formatElementNum >= (int32_t)(TextClockElementLen::MILLISECOND_FORMAT_LEN)) {
                tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_MILLISECOND_INDEX);
            } else {
                tempFormatElement.formatElementNum = (int32_t)(TextClockElementIndex::CUR_CENTISECOND_INDEX);
            }
            break;
        default:
            break;
    }
}

std::string TextClockPattern::GetAmPm(const std::string& dateTimeValue)
{
    std::string language = Localization::GetInstance()->GetLanguage();
    std::string curAmPm = "";
    if (dateTimeValue != "") {
        std::vector<std::string> amPmStrings = Localization::GetInstance()->GetAmPmStrings();
        for (std::string amPmString : amPmStrings) {
            if (dateTimeValue.find(amPmString) != std::string::npos) {
                curAmPm = amPmString;
                break;
            }
        }
    }
    return curAmPm;
}

std::string TextClockPattern::AddZeroPrefix(const std::string& strTimeValue)
{
    if (strTimeValue.size() == STR_SIZE_ONE && CHAR_0 <= strTimeValue[0] && strTimeValue[0] <= CHAR_9) {
        return std::string(STR_0) + strTimeValue;
    }
    return strTimeValue;
}

std::string TextClockPattern::RemoveZeroPrefix(const std::string& strTimeValue)
{
    if (strTimeValue.size() == STR_SIZE_TWO && strTimeValue[0] == CHAR_0) {
        return strTimeValue.substr(1, 1);
    }
    return strTimeValue;
}

std::vector<std::string> TextClockPattern::ParseDateTimeValue(const std::string& strDateTimeValue)
{
    std::string dateTimeValue = strDateTimeValue;
    std::vector<std::string> curDateTime = { "1900", "0", "1", "0", "0", "0", "0", "", "0", "", "", "", "", "" };
    auto dateFirstSplit = dateTimeValue.find('/');
    auto dateSecondSplit = dateTimeValue.find('/', dateFirstSplit + 1);
    auto dateThirdSplit =
        (dateTimeValue.find(',') != std::string::npos) ? dateTimeValue.find(',') : dateTimeValue.find(' ');
    if (curDateTime.size() != (int32_t)(TextClockElementIndex::CUR_MAX_INDEX)) {
        return curDateTime;
    }
    if ((dateFirstSplit != std::string::npos) && (dateSecondSplit != std::string::npos) &&
        (dateThirdSplit != std::string::npos) && (dateSecondSplit > dateFirstSplit) &&
        (dateThirdSplit > dateSecondSplit)) {
        std::string dateFirst = dateTimeValue.substr(0, dateFirstSplit);
        std::string dateSecond = dateTimeValue.substr(dateFirstSplit + 1, dateSecondSplit - dateFirstSplit - 1);
        std::string dateThird = dateTimeValue.substr(dateSecondSplit + 1, dateThirdSplit - dateSecondSplit - 1);
        if (dateFirstSplit > (int32_t)(TextClockElementLen::MON_DAY_FORMAT_MAX_LEN)) {
            curDateTime[(int32_t)(TextClockElementIndex::CUR_YEAR_INDEX)] = dateFirst;
            curDateTime[(int32_t)(TextClockElementIndex::CUR_MONTH_INDEX)] = dateSecond;
            curDateTime[(int32_t)(TextClockElementIndex::CUR_DAY_INDEX)] = dateThird;
        } else {
            curDateTime[(int32_t)(TextClockElementIndex::CUR_YEAR_INDEX)] = dateThird;
            curDateTime[(int32_t)(TextClockElementIndex::CUR_MONTH_INDEX)] = dateFirst;
            curDateTime[(int32_t)(TextClockElementIndex::CUR_DAY_INDEX)] = dateSecond;
        }

        // get short date
        curDateTime[(int32_t)(TextClockElementIndex::CUR_SHORT_YEAR_INDEX)] =
            (curDateTime[(int32_t)(TextClockElementIndex::CUR_YEAR_INDEX)].length() >
                (int32_t)(TextClockElementLen::YEAR_FORMAT_MIN_LEN))
                ? curDateTime[(int32_t)(TextClockElementIndex::CUR_YEAR_INDEX)].substr(
                    (int32_t)(TextClockElementLen::SHORT_YEAR_IN_YEAR_INDEX),
                    (int32_t)(TextClockElementLen::YEAR_FORMAT_MIN_LEN))
                : curDateTime[(int32_t)(TextClockElementIndex::CUR_YEAR_INDEX)];
        if (curDateTime[(int32_t)(TextClockElementIndex::CUR_MONTH_INDEX)].length() >
            (int32_t)(TextClockElementLen::MON_DAY_FORMAT_MIN_LEN)) {
            curDateTime[(int32_t)(TextClockElementIndex::CUR_SHORT_MONTH_INDEX)] =
                (curDateTime[(int32_t)(TextClockElementIndex::CUR_MONTH_INDEX)].substr(0, 1) == "0")
                    ? curDateTime[(int32_t)(TextClockElementIndex::CUR_MONTH_INDEX)].substr(1, 1)
                    : curDateTime[(int32_t)(TextClockElementIndex::CUR_MONTH_INDEX)];
        }
        if (curDateTime[(int32_t)(TextClockElementIndex::CUR_DAY_INDEX)].length() >
            (int32_t)(TextClockElementLen::MON_DAY_FORMAT_MIN_LEN)) {
            curDateTime[(int32_t)(TextClockElementIndex::CUR_SHORT_DAY_INDEX)] =
                (curDateTime[(int32_t)(TextClockElementIndex::CUR_DAY_INDEX)].substr(0, 1) == "0")
                    ? curDateTime[(int32_t)(TextClockElementIndex::CUR_DAY_INDEX)].substr(1, 1)
                    : curDateTime[(int32_t)(TextClockElementIndex::CUR_DAY_INDEX)];
        }

        dateTimeValue.erase(0, dateThirdSplit);
        if (dateTimeValue.find(" ") != std::string::npos) {
            dateTimeValue.replace(dateTimeValue.find(" "), 1, "");
        }
        if (dateTimeValue.find(",") != std::string::npos) {
            dateTimeValue.replace(dateTimeValue.find(","), 1, "");
        }
        auto timeFirstSplit = dateTimeValue.find(':');
        auto timeSecondSplit = dateTimeValue.find(':', timeFirstSplit + 1);
        auto timeThirdSplit =
            (dateTimeValue.find('.') != std::string::npos) ? dateTimeValue.find('.') : (dateTimeValue.length() - 1);
        if ((timeFirstSplit != std::string::npos) && (timeSecondSplit != std::string::npos) &&
            (timeThirdSplit != std::string::npos) && (timeSecondSplit > timeFirstSplit) &&
            (timeThirdSplit > timeSecondSplit)) {
            if (GetPrefixHour() == ZeroPrefixType::SHOW && !is24H_) {
                curDateTime[(int32_t)(TextClockElementIndex::CUR_HOUR_INDEX)] =
                    AddZeroPrefix((dateTimeValue.substr(0, timeFirstSplit)));
            } else if (GetPrefixHour() == ZeroPrefixType::HIDE && is24H_) {
                curDateTime[(int32_t)(TextClockElementIndex::CUR_HOUR_INDEX)] =
                    RemoveZeroPrefix((dateTimeValue.substr(0, timeFirstSplit)));
            } else {
                curDateTime[(int32_t)(TextClockElementIndex::CUR_HOUR_INDEX)] = dateTimeValue.substr(0, timeFirstSplit);
            }
            curDateTime[(int32_t)(TextClockElementIndex::CUR_MINUTE_INDEX)] =
                dateTimeValue.substr(timeFirstSplit + 1, timeSecondSplit - timeFirstSplit - 1);
            curDateTime[(int32_t)(TextClockElementIndex::CUR_SECOND_INDEX)] =
                dateTimeValue.substr(timeSecondSplit + 1, timeThirdSplit - timeSecondSplit - 1);
            curDateTime[(int32_t)(TextClockElementIndex::CUR_MILLISECOND_INDEX)] =
                (timeThirdSplit < (dateTimeValue.length() - 1))
                    ? dateTimeValue.substr(timeThirdSplit + 1, (dateTimeValue.length() - 1 - timeThirdSplit))
                    : "";
        }
        curDateTime[(int32_t)(TextClockElementIndex::CUR_CENTISECOND_INDEX)] =
            (curDateTime[(int32_t)(TextClockElementIndex::CUR_MILLISECOND_INDEX)].length() >
                (int32_t)(TextClockElementLen::CENTISECOND_FORMAT_LEN))
                ? curDateTime[(int32_t)(TextClockElementIndex::CUR_MILLISECOND_INDEX)].substr(
                    0, (int32_t)(TextClockElementLen::CENTISECOND_FORMAT_LEN))
                : curDateTime[(int32_t)(TextClockElementIndex::CUR_MILLISECOND_INDEX)];
    }
    return curDateTime;
}

// abstractItem: true:get letter  false:get non letter
std::string TextClockPattern::Abstract(const std::string& strSource, const bool& abstractItem)
{
    std::string strTemp = "";
    for (char str : strSource) {
        if (abstractItem ? (std::isalpha(str)) : !(std::isalpha(str))) {
            strTemp += str;
        }
    }
    return strTemp;
}

int32_t TextClockPattern::GetDigitNumber(const std::string& strSource)
{
    auto letterNum = 0;
    for (char str : strSource) {
        if (std::isdigit(str)) {
            letterNum++;
        }
    }
    return letterNum;
}

// isShortType: false:full type week, true:short type week
std::string TextClockPattern::GetWeek(const bool& isShortType, const int32_t& week)
{
    std::string language = Localization::GetInstance()->GetLanguage();
    std::vector<std::string> weeks = Localization::GetInstance()->GetWeekdays(isShortType);
    std::string curWeek = "";

    if (week < (int32_t)weeks.size()) {
        if ((strcmp(language.c_str(), "zh") == 0) && isShortType) {
            // chinese E/EE/EEE
            curWeek = Localization::GetInstance()->GetEntryLetters(TEXTCLOCK_WEEK) + weeks[week];
        } else {
            curWeek = weeks[week];
        }
    }
    return curWeek;
}

std::string TextClockPattern::GetMonth(int32_t month)
{
    std::vector<std::string> months = Localization::GetInstance()->GetMonths(true);
    std::string curMonth = "";
    if (month < (int32_t)months.size()) {
        curMonth = months[month];
    }
    return curMonth;
}

std::string TextClockPattern::SpliceDateTime(const std::vector<std::string>& curDateTime)
{
    std::string format = "";
    std::string tempFormat = "";
    bool oneElement = false;
    if (((int32_t)(formatElementMap_.size()) == (int32_t)TextClockElementLen::ONLY_ONE_DATE_ELEMENT) &&
        ((strcmp(Localization::GetInstance()->GetLanguage().c_str(), "zh") == 0))) {
        oneElement = true; // year,month or day need Chinese suffix when Chinese system
    }
    auto it = formatElementMap_.begin();
    while (it != formatElementMap_.end()) {
        tempFormat = CheckDateTimeElement(curDateTime, it->second.elementKey, it->second.formatElementNum, oneElement);
        if (tempFormat.empty()) {
            tempFormat = Abstract(it->second.formatElement, false); // get non letter splitter
        }
        format += tempFormat;
        it++;
    }
    return format;
}

std::string TextClockPattern::CheckDateTimeElement(const std::vector<std::string>& curDateTime, const char& element,
    const int32_t& elementIndex, const bool& oneElement)
{
    std::string format = "";
    std::string curDateTimeElement = "yMdHhmsSaE";
    if (curDateTimeElement.find(element) != std::string::npos) {
        format = curDateTime[elementIndex];
        if ((oneElement)) {
            switch (element) {
                case 'y':
                    format += Localization::GetInstance()->GetEntryLetters(TEXTCLOCK_YEAR);
                    break;
                case 'M':
                    format += Localization::GetInstance()->GetEntryLetters(TEXTCLOCK_MONTH);
                    break;
                case 'd':
                    format += Localization::GetInstance()->GetEntryLetters(TEXTCLOCK_DAY);
                    break;
                default:
                    break;
            }
        }
    }
    return format;
}

void TextClockPattern::FireChangeEvent() const
{
    auto textClockEventHub = GetEventHub<TextClockEventHub>();
    CHECK_NULL_VOID(textClockEventHub);
    textClockEventHub->FireChangeEvent(std::to_string(GetMilliseconds() / MICROSECONDS_OF_MILLISECOND));
}

std::string TextClockPattern::GetFormat() const
{
    auto textClockLayoutProperty = GetLayoutProperty<TextClockLayoutProperty>();
    if (isForm_) {
        std::string defaultFormFormat = SystemProperties::Is24HourClock() ? FORM_FORMAT_24HOUR : FORM_FORMAT;
        CHECK_NULL_RETURN(textClockLayoutProperty, defaultFormFormat);
        std::string result = textClockLayoutProperty->GetFormat().value_or(defaultFormFormat);
        if (result.find("s") != std::string::npos || result.find("S") != std::string::npos) {
            return defaultFormFormat;
        }
        return result;
    }
    std::string defaultFormat = SystemProperties::Is24HourClock() ? DEFAULT_FORMAT_24HOUR : DEFAULT_FORMAT;
    CHECK_NULL_RETURN(textClockLayoutProperty, defaultFormat);
    return textClockLayoutProperty->GetFormat().value_or(defaultFormat);
}

float TextClockPattern::GetHoursWest() const
{
    auto textClockLayoutProperty = GetLayoutProperty<TextClockLayoutProperty>();
    CHECK_NULL_RETURN(textClockLayoutProperty, NAN);
    if (textClockLayoutProperty->GetHoursWest().has_value()) {
        return textClockLayoutProperty->GetHoursWest().value();
    }
    return NAN;
}

RefPtr<FrameNode> TextClockPattern::GetTextNode()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto textNode = AceType::DynamicCast<FrameNode>(host->GetLastChild());
    CHECK_NULL_RETURN(textNode, nullptr);
    if (textNode->GetTag() != V2::TEXT_ETS_TAG) {
        return nullptr;
    }
    return textNode;
}

void TextClockPattern::OnTimeChange()
{
    TAG_LOGI(AceLogTag::ACE_TEXT_CLOCK, "Time is changed and clock updates");
    is24H_ = SystemProperties::Is24HourClock();
    UpdateTimeText(ON_TIME_CHANGE);
}

void TextClockPattern::FireBuilder()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (!makeFunc_.has_value()) {
        auto children = host->GetChildren();
        for (const auto& child : children) {
            if (child->GetId() == nodeId_) {
                host->RemoveChildAndReturnIndex(child);
            }
        }
        host->MarkNeedFrameFlushDirty(PROPERTY_UPDATE_MEASURE);
        return;
    }
    auto node = BuildContentModifierNode();
    if (contentModifierNode_ == node) {
        return;
    }
    host->RemoveChildAndReturnIndex(contentModifierNode_);
    contentModifierNode_ = node;
    CHECK_NULL_VOID(contentModifierNode_);
    nodeId_ = contentModifierNode_->GetId();
    host->AddChild(contentModifierNode_, 0);
    host->MarkNeedFrameFlushDirty(PROPERTY_UPDATE_MEASURE);
}

RefPtr<FrameNode> TextClockPattern::BuildContentModifierNode()
{
    if (!makeFunc_.has_value()) {
        return nullptr;
    }
    auto timeZoneOffset = GetHoursWest();
    auto started = isStart_;
    auto timeValue = static_cast<int64_t>(GetMilliseconds() / MICROSECONDS_OF_MILLISECOND);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto eventHub = host->GetEventHub<TextClockEventHub>();
    CHECK_NULL_RETURN(eventHub, nullptr);
    auto enabled = eventHub->IsEnabled();
    TextClockConfiguration textClockConfiguration(timeZoneOffset, started, timeValue, enabled);
    return (makeFunc_.value())(textClockConfiguration);
}

void TextClockPattern::OnLanguageConfigurationUpdate()
{
    TAG_LOGI(AceLogTag::ACE_TEXT_CLOCK, "Language is changed and clock updates");
    UpdateTimeText(true);
}

void TextClockPattern::DumpInfo()
{
    DumpLog::GetInstance().AddDesc("format: ", GetFormat());
    DumpLog::GetInstance().AddDesc("hourWest: ", hourWest_);
    DumpLog::GetInstance().AddDesc("is24H: ", is24H_ ? "true" : "false");
    DumpLog::GetInstance().AddDesc("isInVisibleArea: ", isInVisibleArea_ ? "true" : "false");
    DumpLog::GetInstance().AddDesc("isStart: ", isStart_ ? "true" : "false");
}
} // namespace OHOS::Ace::NG
