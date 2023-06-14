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

#include "base/log/jank_frame_report.h"

#include <chrono>

#include "render_service_client/core/transaction/rs_interfaces.h"

#include "base/log/event_report.h"

namespace OHOS::Ace {
namespace {
constexpr uint32_t JANK_FRAME_6_FREQ = 0;
constexpr uint32_t JANK_FRAME_15_FREQ = 1;
constexpr uint32_t JANK_FRAME_20_FREQ = 2;
constexpr uint32_t JANK_FRAME_36_FREQ = 3;
constexpr uint32_t JANK_FRAME_48_FREQ = 4;
constexpr uint32_t JANK_FRAME_60_FREQ = 5;
constexpr uint32_t JANK_FRAME_120_FREQ = 6;
constexpr uint32_t JANK_FRAME_180_FREQ = 7;
constexpr uint32_t JANK_SIZE = 8;

using JankNano = std::chrono::nanoseconds;
using JankMilli = std::chrono::milliseconds;

template<class T>
int64_t GetTimeStamp()
{
    return std::chrono::duration_cast<T>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
} // namespace

std::vector<uint16_t> JankFrameReport::frameJankRecord_(JANK_SIZE, 0);
JankFrameFlag JankFrameReport::recordStatus_ = JANK_IDLE;
int64_t JankFrameReport::startTime_ = 0;
std::string JankFrameReport::pageUrl_;
bool JankFrameReport::needReport_ = false;

void JankFrameReport::JankFrameRecord(double jank)
{
    if (!recordStatus_) {
        return;
    }
    needReport_ = true;
    if (jank < 6.0f) {
        frameJankRecord_[JANK_FRAME_6_FREQ]++;
        return;
    }
    if (jank < 15.0f) {
        frameJankRecord_[JANK_FRAME_15_FREQ]++;
        return;
    }
    if (jank < 20.0f) {
        frameJankRecord_[JANK_FRAME_20_FREQ]++;
        return;
    }
    if (jank < 36.0f) {
        frameJankRecord_[JANK_FRAME_36_FREQ]++;
        return;
    }
    if (jank < 48.0f) {
        frameJankRecord_[JANK_FRAME_48_FREQ]++;
        return;
    }
    if (jank < 60.0f) {
        frameJankRecord_[JANK_FRAME_60_FREQ]++;
        return;
    }
    if (jank < 120.0f) {
        frameJankRecord_[JANK_FRAME_120_FREQ]++;
        return;
    }
    frameJankRecord_[JANK_FRAME_180_FREQ]++;
}

void JankFrameReport::ClearFrameJankRecord()
{
    std::fill(frameJankRecord_.begin(), frameJankRecord_.end(), 0);
    recordStatus_ = JANK_IDLE;
}

void JankFrameReport::SetFrameJankFlag(JankFrameFlag flag)
{
    recordStatus_ |= flag;
}

void JankFrameReport::ClearFrameJankFlag(JankFrameFlag flag)
{
    recordStatus_ &= (~flag);
}

void JankFrameReport::StartRecord(const std::string& pageUrl)
{
    if (startTime_ == 0) {
        startTime_ = GetTimeStamp<JankMilli>();
    }
    pageUrl_ = pageUrl;
}

int64_t JankFrameReport::GetDuration()
{
    auto now = GetTimeStamp<JankMilli>();
    return now - startTime_;
}

void JankFrameReport::FlushRecord()
{
    if (!needReport_) {
        ClearFrameJankRecord();
        return;
    }
    Rosen::RSInterfaces::GetInstance().ReportJankStats();
    auto now = GetTimeStamp<JankMilli>();
    EventReport::JankFrameReport(startTime_, now - startTime_, frameJankRecord_, pageUrl_);
    ClearFrameJankRecord();
    startTime_ = now;
}
} // namespace OHOS::Ace
