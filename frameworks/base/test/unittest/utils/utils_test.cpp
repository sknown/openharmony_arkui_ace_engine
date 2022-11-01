/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <atomic>
#include <chrono>
#include <ctime>
#include <regex>
#include <sys/time.h>

#include "gtest/gtest.h"

#include "base/utils/base_id.h"
#include "base/utils/date_util.h"
#include "base/log/log.h"
#include "base/utils/utils.h"
#include "base/utils/resource_configuration.h"
#include "base/utils/string_expression.h"
#include "base/utils/string_utils.h"
#include "base/utils/time_util.h"

#ifndef WINDOWS_PLATFORM
#include "securec.h"
#endif

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {
const uint32_t SET_YESR = 2022;
const uint32_t SET_MONTH = 10;
const uint32_t SET_DAY = 28;
const uint32_t DEFAULT_YEAR = 1900;
const uint32_t DEFAULT_MONTH = 0;
const uint32_t DEFAULT_DAY = 1;
const uint32_t DEFAULT_WEEK = 0;
const uint32_t DAY_OF_MONTH = 31;
const uint32_t DAY_OF_WEEK = 4;
const uint32_t ADD_ONE = 1;
const uint32_t TEST_INPUT_UINT32 = 123456;
const uint64_t NUM_OF_ID = 1;
const int32_t TEST_INPUT_INT32 = 123456;
const int32_t TWELVE_HOUR_BASE = 12;
const int32_t TWENTY_FOUR_HOUR_BASE = 24;
const int32_t DAY_TIME_LOWER_LIMIT = 6;
const int32_t DAY_TIME_UPPER_LIMIT = 18;
const int32_t SET_HOURS_FOR_THE_FIRST_TIME = -13;
const int32_t SET_HOURS_FOR_THE_SECOND_TIME = 8;
const int32_t SET_HOURS_FOR_THE_THIRD_TIME = 24;
const int32_t GET_HOURSWEST_FOR_THE_FIRST_TIME = 24 - 13;
const int32_t GET_HOURSWEST_FOR_THE_SECOND_TIME = 8;
const int32_t GET_HOURSWEST_FOR_THE_THIRD_TIME = 0;
const int32_t TEST_INPUT_ARGS_TWO = 2022;
const int64_t TEST_INPUT_INT64 = 123456;
const int64_t MICROSEC_TO_MILLISEC = 1000;
const int64_t NANOSEC_TO_MILLISEC = 1000000;
const int64_t CONTENT_OF_RESOURCEHANDLERS = 255;
const size_t MAX_STRING_SIZE = 256;
const double NORMAL_CALC_RESULT = 6.2;
const double ERROR_CALC_RESULT = 0.0;
const double CONFIGURATION_OF_DENSITY = 1.0;
const double CONFIGURATION_OF_FONT_RATIO = 1.0;
const double TEST_INPUT_DOUBLE = 123456;
const double STRING_TO_DIMENSION_RESULT = 100.0;
const double STRING_TO_DEGREE_RESULT = 360.0;
const float TEST_INPUT_FLOAT = 123456;
const std::string CONFIGURATION_PATH = "/data/app/el2/100/base";
const std::string FORMULA_ONE = "2 * 3 - (2 + 3) / 5 + 6 / 2";
const std::string FORMULA_TWO = "8 / 2 + 1 )";
const std::string FORMULA_THREE = "931 - 3 * + 102 / +";
const std::string TEST_INPUT_FMT_STR = "%s-%d";
const std::string FMT_OUT_STR = "TODAY-2022";
const std::string TEST_INPUT_U8_STRING = "THIS IS A STRING";
const std::string DEFAULT_STRING = "error";
const std::string TEST_INPUT_U8_STRING_NULL = "";
const std::string TEST_INPUT_U8_STRING_NUMBER = "123456";
const std::string STRING_TO_CALC_DIMENSION_RESULT = "100.0calc";
const std::u16string TEST_INPUT_U16_STRING = u"THIS IS A STRING";
const std::u16string DEFAULT_USTRING = u"error";
const std::wstring TEST_INPUT_W_STRING = L"THIS IS A STRING";
const std::wstring DEFAULT_WSTRING = L"error";
const char TEST_INPUT_ARGS_ONE[MAX_STRING_SIZE] = "TODAY";
const std::vector<int64_t> RESOURCEHANDLERS = {255};
} // namespace

class UtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void UtilsTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "UtilsTest SetUpTestCase";
}

void UtilsTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "UtilsTest TearDownTestCase";
}

void UtilsTest::SetUp()
{
    GTEST_LOG_(INFO) << "UtilsTest SetUp";
}

void UtilsTest::TearDown()
{
    GTEST_LOG_(INFO) << "UtilsTest TearDown";
}

/**
 * @tc.name: UtilsTest001
 * @tc.desc: Call BaseId() constructor twice
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest001, TestSize.Level1)
{
    BaseId baseId_1;
    ASSERT_TRUE(baseId_1.GetId() == NUM_OF_ID);
}

/**
 * @tc.name: UtilsTest002
 * @tc.desc: Set year:2022,month:10,day:28
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest002, TestSize.Level1)
{
    Date oneDay;
    ASSERT_EQ(oneDay.year, DEFAULT_YEAR);
    ASSERT_EQ(oneDay.month, DEFAULT_MONTH);
    ASSERT_EQ(oneDay.day, DEFAULT_DAY);
    ASSERT_EQ(oneDay.week, DEFAULT_WEEK);
    auto locaDay = oneDay.Current();
    time_t nowTime;
    struct tm* localTime;
    time(&nowTime);
    localTime = localtime(&nowTime);
    ASSERT_EQ(locaDay.year, localTime->tm_year + DEFAULT_YEAR);
    ASSERT_EQ(locaDay.month, localTime->tm_mon + ADD_ONE);
    ASSERT_EQ(locaDay.day, localTime->tm_mday);
    ASSERT_EQ(locaDay.week, localTime->tm_wday);
    ASSERT_EQ(locaDay.IsLeapYear(SET_YESR), false);
    ASSERT_EQ(locaDay.DayOfMonth(SET_YESR, SET_MONTH), DAY_OF_MONTH);
    ASSERT_EQ(locaDay.CalculateWeekDay(SET_YESR, SET_MONTH, SET_DAY), DAY_OF_WEEK);
}

/**
 * @tc.name: UtilsTest003
 * @tc.desc: Give a normal formula to ConvertDal2Rpn
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest003, TestSize.Level1)
{
    double CalculateExp(const std::string& expression, const std::function<double(const Dimension&)>& calcFunc);
    auto calcResult =
        StringExpression::CalculateExp(FORMULA_ONE, [](const Dimension& dim) -> double { return dim.Value(); });
    ASSERT_EQ(calcResult, NORMAL_CALC_RESULT);
}

/**
 * @tc.name: UtilsTest004
 * @tc.desc: Give a error formula to ConvertDal2Rpn
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest004, TestSize.Level1)
{
    double CalculateExp(const std::string& expression, const std::function<double(const Dimension&)>& calcFunc);
    auto errResultTwo =
        StringExpression::CalculateExp(FORMULA_TWO, [](const Dimension& dim) -> double { return dim.Value(); });
    auto errResultThree =
        StringExpression::CalculateExp(FORMULA_THREE, [](const Dimension& dim) -> double { return dim.Value(); });
    ASSERT_EQ(errResultTwo, ERROR_CALC_RESULT);
    ASSERT_EQ(errResultThree, ERROR_CALC_RESULT);
}

/**
 * @tc.name: UtilsTest005
 * @tc.desc: Check FormatString
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest005, TestSize.Level1)
{
    auto firstTestText = StringUtils::FormatString("");
    auto secondTestText =
        StringUtils::FormatString(TEST_INPUT_FMT_STR.c_str(), TEST_INPUT_ARGS_ONE, TEST_INPUT_ARGS_TWO);
    ASSERT_TRUE(firstTestText == "");
    ASSERT_EQ(secondTestText, FMT_OUT_STR);
}

/**
 * @tc.name: UtilsTest006
 * @tc.desc: Set the time zone within [-14, -12]
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest006, TestSize.Level1)
{
    time_t nowTime;
    struct tm* localTime;
    time(&nowTime);
    localTime = localtime(&nowTime);
    int32_t localTimeHour12, localTimeHour24;
    auto theTimeOfNow = GetTimeOfNow(SET_HOURS_FOR_THE_FIRST_TIME);
    auto theTimeOfZone = GetTimeOfZone(SET_HOURS_FOR_THE_FIRST_TIME);
    localTimeHour24 = localTime->tm_hour - SET_HOURS_FOR_THE_FIRST_TIME;
    if (localTimeHour24 >= TWENTY_FOUR_HOUR_BASE) {
        localTimeHour24 -= TWENTY_FOUR_HOUR_BASE;
    } else if (localTimeHour24 < 0) {
        localTimeHour24 += TWENTY_FOUR_HOUR_BASE;
    }
    if (localTimeHour24 >= TWELVE_HOUR_BASE) {
        localTimeHour12 = localTimeHour24 - TWELVE_HOUR_BASE;
    } else {
        localTimeHour12 = localTimeHour24;
    }
    ASSERT_EQ(theTimeOfNow.hoursWest_, GET_HOURSWEST_FOR_THE_FIRST_TIME);
    ASSERT_EQ(theTimeOfNow.second_, localTime->tm_sec);
    ASSERT_EQ(theTimeOfNow.minute_, localTime->tm_min);
    ASSERT_EQ(theTimeOfNow.hour12_, localTimeHour12);
    ASSERT_EQ(theTimeOfNow.hour24_, localTimeHour24);
    ASSERT_EQ(theTimeOfZone.hoursWest_, GET_HOURSWEST_FOR_THE_FIRST_TIME);
    ASSERT_EQ(theTimeOfZone.second_, localTime->tm_sec);
    ASSERT_EQ(theTimeOfZone.minute_, localTime->tm_min);
    ASSERT_EQ(theTimeOfZone.hour12_, localTimeHour12);
    ASSERT_EQ(theTimeOfZone.hour24_, localTimeHour24);
}

/**
 * @tc.name: UtilsTest007
 * @tc.desc: Set the time zone within [-12, 12]
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest007, TestSize.Level1)
{
    time_t nowTime;
    struct tm* localTime;
    time(&nowTime);
    localTime = localtime(&nowTime);
    int32_t localTimeHour12, localTimeHour24;
    auto theTimeOfNow = GetTimeOfNow(SET_HOURS_FOR_THE_SECOND_TIME);
    auto theTimeOfZone = GetTimeOfZone(SET_HOURS_FOR_THE_SECOND_TIME);
    localTimeHour24 = localTime->tm_hour - SET_HOURS_FOR_THE_SECOND_TIME;
    if (localTimeHour24 >= TWENTY_FOUR_HOUR_BASE) {
        localTimeHour24 -= TWENTY_FOUR_HOUR_BASE;
    } else if (localTimeHour24 < 0) {
        localTimeHour24 += TWENTY_FOUR_HOUR_BASE;
    }
    if (localTimeHour24 >= TWELVE_HOUR_BASE) {
        localTimeHour12 = localTimeHour24 - TWELVE_HOUR_BASE;
    } else {
        localTimeHour12 = localTimeHour24;
    }
    bool isDayTimeOfNow = IsDayTime(theTimeOfNow);
    bool isDayTimeOfZone = IsDayTime(theTimeOfZone);
    ASSERT_EQ(theTimeOfNow.hoursWest_, GET_HOURSWEST_FOR_THE_SECOND_TIME);
    ASSERT_EQ(theTimeOfNow.second_, localTime->tm_sec);
    ASSERT_EQ(theTimeOfNow.minute_, localTime->tm_min);
    ASSERT_EQ(theTimeOfNow.hour12_, localTimeHour12);
    ASSERT_EQ(theTimeOfNow.hour24_, localTimeHour24);
    ASSERT_EQ(theTimeOfZone.hoursWest_, GET_HOURSWEST_FOR_THE_SECOND_TIME);
    ASSERT_EQ(theTimeOfZone.second_, localTime->tm_sec);
    ASSERT_EQ(theTimeOfZone.minute_, localTime->tm_min);
    ASSERT_EQ(theTimeOfZone.hour12_, localTimeHour12);
    ASSERT_EQ(theTimeOfZone.hour24_, localTimeHour24);
    if (localTimeHour24 >= DAY_TIME_LOWER_LIMIT && localTimeHour24 < DAY_TIME_UPPER_LIMIT) {
        ASSERT_EQ(isDayTimeOfNow, true);
        ASSERT_EQ(isDayTimeOfZone, true);
    } else {
        ASSERT_EQ(isDayTimeOfNow, false);
        ASSERT_EQ(isDayTimeOfZone, false);
    }
}

/**
 * @tc.name: UtilsTest008
 * @tc.desc: Set the time zone beyond the range of [-14, 12]
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest008, TestSize.Level1)
{
    time_t nowTime;
    struct tm* localTime;
    time(&nowTime);
    localTime = localtime(&nowTime);
    int32_t localTimeHour12, localTimeHour24;
    auto theTimeOfNow = GetTimeOfNow(SET_HOURS_FOR_THE_THIRD_TIME);
    auto theTimeOfZone = GetTimeOfZone(SET_HOURS_FOR_THE_THIRD_TIME);
    localTimeHour24 = localTime->tm_hour - SET_HOURS_FOR_THE_THIRD_TIME;
    if (localTimeHour24 >= TWENTY_FOUR_HOUR_BASE) {
        localTimeHour24 -= TWENTY_FOUR_HOUR_BASE;
    } else if (localTimeHour24 < 0) {
        localTimeHour24 += TWENTY_FOUR_HOUR_BASE;
    }
    if (localTimeHour24 >= TWELVE_HOUR_BASE) {
        localTimeHour12 = localTimeHour24 - TWELVE_HOUR_BASE;
    } else {
        localTimeHour12 = localTimeHour24;
    }
    ASSERT_EQ(theTimeOfNow.hoursWest_, GET_HOURSWEST_FOR_THE_THIRD_TIME);
    ASSERT_EQ(theTimeOfNow.second_, localTime->tm_sec);
    ASSERT_EQ(theTimeOfNow.minute_, localTime->tm_min);
    ASSERT_EQ(theTimeOfNow.hour12_, localTimeHour12);
    ASSERT_EQ(theTimeOfNow.hour24_, localTimeHour24);
    ASSERT_EQ(theTimeOfZone.hoursWest_, GET_HOURSWEST_FOR_THE_THIRD_TIME);
    ASSERT_EQ(theTimeOfZone.second_, localTime->tm_sec);
    ASSERT_EQ(theTimeOfZone.minute_, localTime->tm_min);
    ASSERT_EQ(theTimeOfZone.hour12_, localTimeHour12);
    ASSERT_EQ(theTimeOfZone.hour24_, localTimeHour24);
}

/**
 * @tc.name: UtilsTest009
 * @tc.desc: Check the time since the device was started
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest009, TestSize.Level1)
{
    auto microTickCount = GetMicroTickCount();
    auto sysTimestamp = GetSysTimestamp();
    int64_t microTickCountMsec, sysTimestampMsec;
    microTickCountMsec = microTickCount / MICROSEC_TO_MILLISEC;
    sysTimestampMsec = sysTimestamp / NANOSEC_TO_MILLISEC;
    ASSERT_EQ(microTickCountMsec, sysTimestampMsec);
}

/**
 * @tc.name: UtilsTest010
 * @tc.desc: Set ResourceConfiguration
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest010, TestSize.Level1)
{
    ResourceConfiguration resConfiguration;
    ASSERT_EQ(resConfiguration.TestFlag(ResourceConfiguration::COLOR_MODE_UPDATED_FLAG,
        ResourceConfiguration::FONT_RATIO_UPDATED_FLAG), false);
    resConfiguration.SetDeviceType(DeviceType::UNKNOWN);
    resConfiguration.SetOrientation(DeviceOrientation::ORIENTATION_UNDEFINED);
    resConfiguration.SetDensity(CONFIGURATION_OF_DENSITY);
    resConfiguration.SetFontRatio(CONFIGURATION_OF_FONT_RATIO);
    resConfiguration.SetColorMode(ColorMode::COLOR_MODE_UNDEFINED);
    resConfiguration.SetDeviceAccess(true);
    ASSERT_EQ(resConfiguration.GetDeviceType(), DeviceType::UNKNOWN);
    ASSERT_EQ(resConfiguration.GetOrientation(), DeviceOrientation::ORIENTATION_UNDEFINED);
    ASSERT_EQ(resConfiguration.GetDensity(), CONFIGURATION_OF_DENSITY);
    ASSERT_EQ(resConfiguration.GetFontRatio(), CONFIGURATION_OF_FONT_RATIO);
    ASSERT_EQ(resConfiguration.GetColorMode(), ColorMode::COLOR_MODE_UNDEFINED);
    ASSERT_EQ(resConfiguration.GetDeviceAccess(), true);
}

/**
 * @tc.name: UtilsTest011
 * @tc.desc: Set ResourceInfo
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest011, TestSize.Level1)
{
    ResourceInfo resourceInfo;
    ResourceConfiguration resConfiguration;
    resourceInfo.SetResourceConfiguration(resConfiguration);
    resourceInfo.SetResourceHandlers(RESOURCEHANDLERS);
    resourceInfo.SetHapPath(CONFIGURATION_PATH);
    resourceInfo.SetPackagePath(CONFIGURATION_PATH);
    resourceInfo.SetThemeId(NUM_OF_ID);
    ASSERT_EQ(resourceInfo.GetResourceConfiguration().GetDeviceType(), DeviceType::PHONE);
    ASSERT_EQ(resourceInfo.GetResourceHandlers()[0], CONTENT_OF_RESOURCEHANDLERS);
    ASSERT_EQ(resourceInfo.GetHapPath(), CONFIGURATION_PATH);
    ASSERT_EQ(resourceInfo.GetPackagePath(), CONFIGURATION_PATH);
    ASSERT_EQ(resourceInfo.GetThemeId(), NUM_OF_ID);
}

/**
 * @tc.name: UtilsTest012
 * @tc.desc: Json does not contain "colorMode"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest012, TestSize.Level1)
{
    const std::string inputTestJsonStr = "{"
                                         "  \"deviceType\": \"PHONE\",          "
                                         "  \"fontScale\": 1.0                  "
                                         "}";
    ResourceConfiguration resConfiguration;
    uint32_t updateFlags = ResourceConfiguration::COLOR_MODE_UPDATED_FLAG;
    bool isUpdateFromJsonString = resConfiguration.UpdateFromJsonString(inputTestJsonStr, updateFlags);
    ASSERT_EQ(isUpdateFromJsonString, true);
}

/**
 * @tc.name: UtilsTest013
 * @tc.desc: Json does not contain "fontScale"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest013, TestSize.Level1)
{
    const std::string inputTestJsonStr = "{"
                                         "  \"deviceType\": \"PHONE\",          "
                                         "  \"colorMode\": 1.0                  "
                                         "}";
    ResourceConfiguration resConfiguration;
    uint32_t updateFlags = ResourceConfiguration::COLOR_MODE_UPDATED_FLAG;
    bool isUpdateFromJsonString = resConfiguration.UpdateFromJsonString(inputTestJsonStr, updateFlags);
    ASSERT_EQ(isUpdateFromJsonString, true);
}

/**
 * @tc.name: UtilsTest014
 * @tc.desc: ColorMode is not "light" or "dark"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest014, TestSize.Level1)
{
    const std::string inputTestJsonStr = "{"
                                         "  \"deviceType\": \"PHONE\",          "
                                         "  \"fontScale\": \"1.0\",             "
                                         "  \"colorMode\": \"undefined\"            "
                                         "}";
    ResourceConfiguration resConfiguration;
    uint32_t updateFlags = ResourceConfiguration::COLOR_MODE_UPDATED_FLAG;
    bool isUpdateFromJsonString = resConfiguration.UpdateFromJsonString(inputTestJsonStr, updateFlags);
    ASSERT_EQ(isUpdateFromJsonString, true);
}

/**
 * @tc.name: UtilsTest015
 * @tc.desc: FontScale is not 1.0
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest015, TestSize.Level1)
{
    const std::string inputTestJsonStr = "{"
                                         "  \"deviceType\": \"PHONE\",          "
                                         "  \"fontScale\": 2.0,                 "
                                         "  \"colorMode\": \"light\"            "
                                         "}";
    ResourceConfiguration resConfiguration;
    uint32_t updateFlags = ResourceConfiguration::COLOR_MODE_UPDATED_FLAG;
    bool isUpdateFromJsonString = resConfiguration.UpdateFromJsonString(inputTestJsonStr, updateFlags);
    ASSERT_EQ(isUpdateFromJsonString, true);
    ASSERT_EQ(updateFlags, ResourceConfiguration::FONT_RATIO_UPDATED_FLAG);
}

/**
 * @tc.name: UtilsTest016
 * @tc.desc: Set colorMode "dark"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest016, TestSize.Level1)
{
    const std::string inputTestJsonStr = "{"
                                         "  \"deviceType\": \"PHONE\",          "
                                         "  \"fontScale\": 1.0,                 "
                                         "  \"colorMode\": \"dark\"             "
                                         "}";
    ResourceConfiguration resConfiguration;
    uint32_t updateFlags = ResourceConfiguration::COLOR_MODE_UPDATED_FLAG;
    bool isUpdateFromJsonString = resConfiguration.UpdateFromJsonString(inputTestJsonStr, updateFlags);
    ASSERT_EQ(isUpdateFromJsonString, true);
    ASSERT_EQ(updateFlags, ResourceConfiguration::COLOR_MODE_UPDATED_FLAG);
}

/**
 * @tc.name: UtilsTest017
 * @tc.desc: Check char in bmp and whether the content of string is a number
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest017, TestSize.Level1)
{
    const char16_t testChar16 = 0xD800;
    const wchar_t testWchar = 0xD800;
    ASSERT_EQ(StringUtils::NotInUtf16Bmp(testChar16), true);
    ASSERT_EQ(StringUtils::NotInBmp(testWchar), true);
    ASSERT_EQ(StringUtils::IsNumber(TEST_INPUT_U8_STRING_NULL), false);
    ASSERT_EQ(StringUtils::IsNumber(TEST_INPUT_U8_STRING_NUMBER), true);
}

/**
 * @tc.name: UtilsTest018
 * @tc.desc: U16string-string-wstring, string-uint32, string-int64 convert
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest018, TestSize.Level1)
{
    ASSERT_EQ(StringUtils::Str8ToStr16(TEST_INPUT_U8_STRING), TEST_INPUT_U16_STRING);
    ASSERT_EQ(StringUtils::Str16ToStr8(TEST_INPUT_U16_STRING), TEST_INPUT_U8_STRING);
    ASSERT_EQ(StringUtils::ToWstring(TEST_INPUT_U8_STRING), TEST_INPUT_W_STRING);
    ASSERT_EQ(StringUtils::ToString(TEST_INPUT_W_STRING), TEST_INPUT_U8_STRING);
    ASSERT_EQ(StringUtils::Str8ToStr16(StringUtils::DEFAULT_STRING), StringUtils::DEFAULT_USTRING);
    ASSERT_EQ(StringUtils::Str16ToStr8(StringUtils::DEFAULT_USTRING), StringUtils::DEFAULT_STRING);
    ASSERT_EQ(StringUtils::ToWstring(StringUtils::DEFAULT_STRING), StringUtils::DEFAULT_WSTRING);
    ASSERT_EQ(StringUtils::ToString(StringUtils::DEFAULT_WSTRING), StringUtils::DEFAULT_STRING);
    ASSERT_EQ(StringUtils::DoubleToString(TEST_INPUT_DOUBLE, 0), TEST_INPUT_U8_STRING_NUMBER);
    ASSERT_EQ(StringUtils::StringToLongInt(TEST_INPUT_U8_STRING_NUMBER), TEST_INPUT_INT64);
    ASSERT_EQ(StringUtils::StringToUint(TEST_INPUT_U8_STRING_NUMBER), TEST_INPUT_UINT32);
}

/**
 * @tc.name: UtilsTest019
 * @tc.desc: Delete all mark : "."
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest019, TestSize.Level1)
{
    std::string markString = "THIS. IS. A. STRING";
    const char mark = '.';
    StringUtils::DeleteAllMark(markString, mark);
    ASSERT_EQ(markString, TEST_INPUT_U8_STRING);
}

/**
 * @tc.name: UtilsTest020
 * @tc.desc: Replace tab and newline with space
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest020, TestSize.Level1)
{
    std::string inputTabAndNewLine = "THIS\rIS\nA\tSTRING";
    StringUtils::ReplaceTabAndNewLine(inputTabAndNewLine);
    ASSERT_EQ(inputTabAndNewLine, TEST_INPUT_U8_STRING);
}

/**
 * @tc.name: UtilsTest021
 * @tc.desc: Input a string contain "auto"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest021, TestSize.Level1)
{
    const std::string stringToDimensionValue = "auto";
    auto dim = StringUtils::StringToDimension(stringToDimensionValue, true);
    auto calcDim = StringUtils::StringToCalcDimension(stringToDimensionValue, true);
    ASSERT_EQ(dim.Value(), 0);
    ASSERT_EQ(dim.Unit(), DimensionUnit::AUTO);
    ASSERT_EQ(calcDim.Value(), 0);
    ASSERT_EQ(calcDim.Unit(), DimensionUnit::AUTO);
}

/**
 * @tc.name: UtilsTest022
 * @tc.desc: Input a string contain "calc"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest022, TestSize.Level1)
{
    const std::string stringToDimensionValue = "100.0calc";
    auto dim = StringUtils::StringToDimension(stringToDimensionValue, true);
    auto calcDim = StringUtils::StringToCalcDimension(stringToDimensionValue, true);
    ASSERT_EQ(dim.Value(), STRING_TO_DIMENSION_RESULT);
    ASSERT_EQ(dim.Unit(), DimensionUnit::VP);
    ASSERT_EQ(calcDim.CalcValue(), STRING_TO_CALC_DIMENSION_RESULT);
    ASSERT_EQ(calcDim.Unit(), DimensionUnit::CALC);
}

/**
 * @tc.name: UtilsTest023
 * @tc.desc: Input a string contain "%"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest023, TestSize.Level1)
{
    const std::string stringToDimensionValue = "100.0%";
    auto dim = StringUtils::StringToDimension(stringToDimensionValue, true);
    ASSERT_EQ(dim.Value(), 1.0);
    ASSERT_EQ(dim.Unit(), DimensionUnit::PERCENT);
}

/**
 * @tc.name: UtilsTest024
 * @tc.desc: Input a string contain "px"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest024, TestSize.Level1)
{
    const std::string stringToDimensionValue = "100.0px";
    auto dim = StringUtils::StringToDimension(stringToDimensionValue, true);
    ASSERT_EQ(dim.Value(), STRING_TO_DIMENSION_RESULT);
    ASSERT_EQ(dim.Unit(), DimensionUnit::PX);
}

/**
 * @tc.name: UtilsTest025
 * @tc.desc: Input a string contain "vp"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest025, TestSize.Level1)
{
    const std::string stringToDimensionValue = "100.0vp";
    auto dim = StringUtils::StringToDimension(stringToDimensionValue, true);
    ASSERT_EQ(dim.Value(), STRING_TO_DIMENSION_RESULT);
    ASSERT_EQ(dim.Unit(), DimensionUnit::VP);
}

/**
 * @tc.name: UtilsTest026
 * @tc.desc: Input a string contain "fp"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest026, TestSize.Level1)
{
    const std::string stringToDimensionValue = "100.0fp";
    auto dim = StringUtils::StringToDimension(stringToDimensionValue, true);
    ASSERT_EQ(dim.Value(), STRING_TO_DIMENSION_RESULT);
    ASSERT_EQ(dim.Unit(), DimensionUnit::FP);
}

/**
 * @tc.name: UtilsTest027
 * @tc.desc: Input a string contain "lpx"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest027, TestSize.Level1)
{
    const std::string stringToDimensionValue = "100.0lpx";
    auto dim = StringUtils::StringToDimension(stringToDimensionValue, true);
    ASSERT_EQ(dim.Value(), STRING_TO_DIMENSION_RESULT);
    ASSERT_EQ(dim.Unit(), DimensionUnit::LPX);
}

/**
 * @tc.name: UtilsTest028
 * @tc.desc: Set an empty string
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest028, TestSize.Level1)
{
    const std::string stringToDimensionValue = "";
    auto dim = StringUtils::StringToDimension(stringToDimensionValue, true);
    ASSERT_EQ(dim.Value(), 0.0);
    ASSERT_EQ(dim.Unit(), DimensionUnit::VP);
}

/**
 * @tc.name: UtilsTest029
 * @tc.desc: Set an empty string, check StringToDegree
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest029, TestSize.Level1)
{
    const std::string stringToDegreeValue = "";
    auto degreeValue = StringUtils::StringToDegree(stringToDegreeValue);
    ASSERT_EQ(degreeValue, 0.0);
}

/**
 * @tc.name: UtilsTest030
 * @tc.desc: Input a string contain "deg"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest030, TestSize.Level1)
{
    const std::string stringToDegreeValue = "360.0deg";
    auto degreeValue = StringUtils::StringToDegree(stringToDegreeValue);
    ASSERT_EQ(degreeValue, STRING_TO_DEGREE_RESULT);
}

/**
 * @tc.name: UtilsTest031
 * @tc.desc: Input a string contain "gard"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest031, TestSize.Level1)
{
    const std::string stringToDegreeValue = "400.0grad";
    auto degreeValue = StringUtils::StringToDegree(stringToDegreeValue);
    ASSERT_EQ(degreeValue, STRING_TO_DEGREE_RESULT);
}

/**
 * @tc.name: UtilsTest032
 * @tc.desc: Input a string contain "rad"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest032, TestSize.Level1)
{
    const std::string stringToDegreeValue = "6.28318530717958647692rad";
    auto degreeValue = StringUtils::StringToDegree(stringToDegreeValue);
    ASSERT_EQ(degreeValue, STRING_TO_DEGREE_RESULT);
}

/**
 * @tc.name: UtilsTest033
 * @tc.desc: Input a string contain "turn"
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest033, TestSize.Level1)
{
    const std::string stringToDegreeValue = "1turn";
    auto degreeValue = StringUtils::StringToDegree(stringToDegreeValue);
    ASSERT_EQ(degreeValue, STRING_TO_DEGREE_RESULT);
}

/**
 * @tc.name: UtilsTest034
 * @tc.desc: Input a string contain "-", split string to string
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest034, TestSize.Level1)
{
    const std::string stringSplitterValue = "123456-123456";
    const char delimiter = '-';
    std::vector<std::string> out;
    StringUtils::StringSplitter(stringSplitterValue, delimiter, out);
    ASSERT_EQ(out[0], TEST_INPUT_U8_STRING_NUMBER);
}

/**
 * @tc.name: UtilsTest035
 * @tc.desc: Input a string contain "-", split string to int32_t
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest035, TestSize.Level1)
{
    const std::string stringSplitterValue = "123456-123456";
    const char delimiter = '-';
    std::vector<int32_t> out;
    StringUtils::StringSplitter(stringSplitterValue, delimiter, out);
    ASSERT_EQ(out[0], TEST_INPUT_INT32);
}

/**
 * @tc.name: UtilsTest036
 * @tc.desc: Input a string contain "-", split string to double
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest036, TestSize.Level1)
{
    const std::string stringSplitterValue = "123456-123456";
    const char delimiter = '-';
    std::vector<double> out;
    StringUtils::StringSplitter(stringSplitterValue, delimiter, out);
    ASSERT_EQ(out[0], TEST_INPUT_DOUBLE);
}

/**
 * @tc.name: UtilsTest037
 * @tc.desc: Input a string contain "-", split string to float
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest037, TestSize.Level1)
{
    const std::string stringSplitterValue = "123456-123456";
    const char delimiter = '-';
    std::vector<float> out;
    StringUtils::StringSplitter(stringSplitterValue, delimiter, out);
    ASSERT_EQ(out[0], TEST_INPUT_FLOAT);
}

/**
 * @tc.name: UtilsTest038
 * @tc.desc: Input a string contain "-", split string to Dimension
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest038, TestSize.Level1)
{
    const std::string stringSplitterValue = "123456-123456";
    const char delimiter = '-';
    std::vector<Dimension> out;
    StringUtils::StringSplitter(stringSplitterValue, delimiter, out);
    ASSERT_EQ(out[0].Value(), TEST_INPUT_DOUBLE);
    ASSERT_EQ(out[0].Unit(), DimensionUnit::PX);
}

/**
 * @tc.name: UtilsTest039
 * @tc.desc: No characters found for splitSepValue in splitStrValue
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest039, TestSize.Level1)
{
    const std::string splitStrValue = "THIS IS A STRING";
    const std::string splitSepValue = "!?";
    std::vector<std::string> stringOut;
    std::vector<Dimension> dimOut;
    StringUtils::SplitStr(splitStrValue, splitSepValue, stringOut, true);
    StringUtils::SplitStr(splitStrValue, splitSepValue, dimOut, true);
    ASSERT_EQ(stringOut[0], TEST_INPUT_U8_STRING);
    ASSERT_EQ(dimOut[0].Value(), 0);
    ASSERT_EQ(dimOut[0].Unit(), DimensionUnit::PX);
}

/**
 * @tc.name: UtilsTest040
 * @tc.desc: There is some characters found for splitSepValue in splitStrValue
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest040, TestSize.Level1)
{
    const std::string splitStrValue = "##THIS IS A STRING***##";
    const std::string splitSepValue = "#*";
    std::vector<std::string> stringOut;
    std::vector<Dimension> dimOut;
    StringUtils::SplitStr(splitStrValue, splitSepValue, stringOut, true);
    StringUtils::SplitStr(splitStrValue, splitSepValue, dimOut, true);
    ASSERT_EQ(stringOut[0], TEST_INPUT_U8_STRING);
    ASSERT_EQ(dimOut[0].Value(), 0);
    ASSERT_EQ(dimOut[0].Unit(), DimensionUnit::PX);
}

/**
 * @tc.name: UtilsTest041
 * @tc.desc: splitString is empty
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest041, TestSize.Level1)
{
    const std::string splitStrValue = "";
    const std::string splitSepValue = "";
    std::vector<std::string> stringOut;
    std::vector<Dimension> dimOut;
    StringUtils::SplitStr(splitStrValue, splitSepValue, stringOut, true);
    StringUtils::SplitStr(splitStrValue, splitSepValue, dimOut, true);
    ASSERT_TRUE(stringOut.empty());
    ASSERT_TRUE(dimOut.empty());
}

/**
 * @tc.name: UtilsTest042
 * @tc.desc: StartWith, EndWith the same string
 * @tc.type: FUNC
 */
HWTEST_F(UtilsTest, UtilsTest042, TestSize.Level1)
{
    const std::string startWithValue = "THIS IS A STRING";
    const std::string prefixString = "THIS IS A STRING";
    const char* prefixChar = "THIS IS A STRING";
    size_t prefixLen = startWithValue.size();
    ASSERT_EQ(StringUtils::StartWith(startWithValue, prefixString), true);
    ASSERT_EQ(StringUtils::StartWith(startWithValue, prefixChar, prefixLen), true);
    ASSERT_EQ(StringUtils::EndWith(startWithValue, prefixString), true);
    ASSERT_EQ(StringUtils::EndWith(startWithValue, prefixString), true);
}
} // namespace OHOS::Ace