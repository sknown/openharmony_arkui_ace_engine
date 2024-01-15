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

#include "adapter/ohos/osal/long_frame_report_impl.h"

#include "base/longframe/long_frame_report.h"
#include "base/ressched/ressched_report.h"

namespace OHOS::Ace {
namespace {
    constexpr int32_t LONG_FRAME_EVENT_DELAY = 10000;
}
LongFrameReportImpl::LongFrameReportImpl()
{
    ffrtTaskQueue = std::make_unique<ffrt::queue>("long_frame_report_ffrt_queue",
        ffrt::queue_attr().qos(ffrt::qos_user_initiated));
}

void LongFrameReportImpl::SubmitEvent()
{
    ffrtTask = ffrtTaskQueue->submit_h([] {
        ResSchedReport::GetInstance().ResSchedDataReport("long_frame_start");
    }, ffrt::task_attr().delay(LONG_FRAME_EVENT_DELAY));
}

void LongFrameReportImpl::CancelEvent()
{
    if (ffrtTaskQueue->cancel(ffrtTask)) {
        ffrtTaskQueue->wait(ffrtTask);
        ResSchedReport::GetInstance().ResSchedDataReport("long_frame_end");
    }
}

void ILongFrame::ReportStartEvent()
{
    reporter = new LongFrameReportImpl();
    static_cast<LongFrameReportImpl*>(reporter)->SubmitEvent();
}

void ILongFrame::ReportEndEvent()
{
    static_cast<LongFrameReportImpl*>(reporter)->CancelEvent();
    delete static_cast<LongFrameReportImpl*>(reporter);
}
} // namespace OHOS::Ace
