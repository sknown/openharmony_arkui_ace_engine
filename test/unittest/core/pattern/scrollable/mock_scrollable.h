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

#ifndef FOUNDATION_ACE_TEST_CORE_PATTERN_SCROLLABLE_MOCK_SCROLLABLE_H
#define FOUNDATION_ACE_TEST_CORE_PATTERN_SCROLLABLE_MOCK_SCROLLABLE_H

#include "gmock/gmock.h"

#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"

namespace OHOS::Ace::NG {
// implement interfaces with mock
class PartiallyMockedScrollable : public ScrollablePattern {
    DECLARE_ACE_TYPE(PartiallyMockedScrollable, ScrollablePattern);
private:
    MOCK_METHOD(bool, UpdateCurrentOffset, (float delta, int32_t source), (override));
    MOCK_METHOD(bool, IsAtTop, (), (const, override));
    MOCK_METHOD(bool, IsAtBottom, (), (const, override));
    MOCK_METHOD(void, UpdateScrollBarOffset, (), (override));
    MOCK_METHOD(bool, IsScrollable, (), (const, override));
    MOCK_METHOD(OverScrollOffset, GetOverScrollOffset, (double delta), (const, override));
};

class FullyMockedScrollable : public PartiallyMockedScrollable {
    DECLARE_ACE_TYPE(FullyMockedScrollable, PartiallyMockedScrollable);
private:
    MOCK_METHOD(bool, HandleScrollVelocity, (float), (override));
    MOCK_METHOD(void, OnScrollEndCallback, (), (override));
    MOCK_METHOD(bool, OutBoundaryCallback, (), (override));
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_TEST_CORE_PATTERN_SCROLLABLE_MOCK_SCROLLABLE_H
