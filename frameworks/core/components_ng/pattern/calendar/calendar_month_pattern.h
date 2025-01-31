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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CALENDAR_CALENDAR_MONTH_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CALENDAR_CALENDAR_MONTH_PATTERN_H

#include <optional>

#include "base/geometry/axis.h"
#include "base/memory/referenced.h"
#include "core/components/calendar/calendar_data_adapter.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/calendar/calendar_event_hub.h"
#include "core/components_ng/pattern/calendar/calendar_layout_algorithm.h"
#include "core/components_ng/pattern/calendar/calendar_paint_method.h"
#include "core/components_ng/pattern/calendar/calendar_paint_property.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/render/paint_property.h"

namespace OHOS::Ace::NG {

class CalendarMonthPattern : public Pattern {
    DECLARE_ACE_TYPE(CalendarMonthPattern, Pattern);

public:
    CalendarMonthPattern() = default;
    ~CalendarMonthPattern() override = default;

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<CalendarPaintProperty>();
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<CalendarEventHub>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<CalendarLayoutAlgorithm>();
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        return MakeRefPtr<CalendarPaintMethod>(obtainedMonth_, calendarDay_, isCalendarDialog_);
    }

    const ObtainedMonth& GetMonthData() const
    {
        return obtainedMonth_;
    }

    void SetMonthData(const ObtainedMonth& obtainedMonth, MonthState monthState)
    {
        obtainedMonth_ = obtainedMonth;
        monthState_ = monthState;
    }

    const CalendarDay& GetCalendarDay() const
    {
        return calendarDay_;
    }

    void SetCalendarDay(const CalendarDay& calendarDay)
    {
        calendarDay_ = calendarDay;
        if (monthState_ == MonthState::CUR_MONTH && !obtainedMonth_.days.empty()) {
            for (auto& day : obtainedMonth_.days) {
                if (day.month.year == calendarDay.month.year && day.month.month == calendarDay.month.month &&
                    day.day == calendarDay.day) {
                    day.focused = true;
                }
            }
        }
    }

    bool IsCalendarDialog() const
    {
        return isCalendarDialog_;
    }

    void SetCalendarDialogFlag(bool isCalendarDialog)
    {
        isCalendarDialog_ = isCalendarDialog;
    }

    void SetHoverState(bool state)
    {
        hoverState_ = state;
    }

    bool GetHoverState() const
    {
        return hoverState_;
    }

private:
    void OnAttachToFrameNode() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    void OnModifyDone() override;
    void OnClick(Offset& localLocation, const ObtainedMonth& obtainedMonth);
    void OnTouchEvent(const Offset& localLocation, bool isPressed);
    void OnHoverEvent(const Offset& localLocation, bool state);
    void InitClickEvent();
    void InitTouchEvent();
    void InitHoverEvent();
    void SetColRowSpace();
    int32_t JudgeArea(const Offset& offset);
    Dimension GetDaySize(const RefPtr<CalendarTheme>& theme);
    bool isCalendarDialog_ = false;
    bool hoverState_ = false;

    CalendarDay calendarDay_;
    ObtainedMonth obtainedMonth_;
    MonthState monthState_ = MonthState::CUR_MONTH;
    RefPtr<ClickEvent> clickListener_;
    RefPtr<TouchEventImpl> touchListener_;
    RefPtr<InputEvent> hoverListener_;

    ACE_DISALLOW_COPY_AND_MOVE(CalendarMonthPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CALENDAR_CALENDAR_MONTH_PATTERN_H