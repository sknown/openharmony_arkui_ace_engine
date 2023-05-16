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

#include "frameworks/bridge/declarative_frontend/jsview/models/calendar_controller_model_impl.h"

namespace OHOS::Ace::Framework {
RefPtr<AceType> CalendarControllerModelImpl::GetController()
{
    return Referenced::MakeRefPtr<CalendarControllerV2>();
}

void CalendarControllerModelImpl::BackToToday(RefPtr<AceType>& controller)
{
    CHECK_NULL_VOID_NOLOG(controller);
    auto controllerV2 = AceType::DynamicCast<CalendarControllerV2>(controller);
    CHECK_NULL_VOID_NOLOG(controllerV2);
    controllerV2->BackToToday();
}

void CalendarControllerModelImpl::GoTo(const int32_t& year, const int32_t& month, const int32_t& day,
    RefPtr<AceType>& controller)
{
    CHECK_NULL_VOID_NOLOG(controller);
    auto controllerV2 = AceType::DynamicCast<CalendarControllerV2>(controller);
    CHECK_NULL_VOID_NOLOG(controllerV2);
    controllerV2->GoTo(year, month, day);
}
} // namespace OHOS::Ace::Framework
