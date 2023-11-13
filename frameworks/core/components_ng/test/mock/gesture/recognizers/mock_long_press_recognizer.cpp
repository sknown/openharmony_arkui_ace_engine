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

#include "core/components_ng/gestures/recognizers/long_press_recognizer.h"

namespace OHOS::Ace::NG {
LongPressRecognizer::LongPressRecognizer(
    int32_t duration, int32_t fingers, bool repeat, bool isForDrag, bool isDisableMouseLeft) {}

void LongPressRecognizer::OnAccepted() {}

void LongPressRecognizer::OnRejected() {}

void LongPressRecognizer::HandleTouchDownEvent(const TouchEvent& event) {}

void LongPressRecognizer::HandleTouchUpEvent(const TouchEvent& /* event */) {}

void LongPressRecognizer::HandleTouchMoveEvent(const TouchEvent& event) {}

void LongPressRecognizer::HandleTouchCancelEvent(const TouchEvent& event) {}

bool LongPressRecognizer::ReconcileFrom(const RefPtr<NGGestureRecognizer>& /* recognizer */)
{
    return true;
}
void LongPressRecognizer::OnResetStatus() {}

GestureEventFunc LongPressRecognizer::GetLongPressActionFunc()
{
    return [](GestureEvent& info) {};
}

RefPtr<GestureSnapshot> LongPressRecognizer::Dump() const
{
    return nullptr;
}
} // namespace OHOS::Ace::NG
