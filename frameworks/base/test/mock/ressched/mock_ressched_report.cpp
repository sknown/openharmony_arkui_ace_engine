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

#include "base/ressched/ressched_report.h"

namespace OHOS::Ace {
ResSchedReport& ResSchedReport::GetInstance()
{
    static ResSchedReport instance;
    return instance;
}

void ResSchedReport::ResSchedDataReport(
    const char* /* name */, const std::unordered_map<std::string, std::string>& /* param */)
{
    reportDataFunc_ = nullptr;
}

void ResSchedReport::ResSchedDataReport(
    uint32_t resType, int32_t value, const std::unordered_map<std::string, std::string>& payload)
{}

void ResSchedReport::OnTouchEvent(const TouchType& touchType) {}

ResSchedReportScope::ResSchedReportScope(
    const std::string& name, const std::unordered_map<std::string, std::string>& param)
    : name_(name), payload_(param)
{}

ResSchedReportScope::~ResSchedReportScope() = default;
} // namespace OHOS::Ace
