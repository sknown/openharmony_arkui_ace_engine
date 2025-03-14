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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_TEST_PATTERN_TEST_NG_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_TEST_PATTERN_TEST_NG_H

#include <type_traits>

#include "gtest/gtest.h"

#define private public
#define protected public
#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components/button/button_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_stack_processor.h"

namespace OHOS::Ace::NG {
namespace {
using namespace testing;
using namespace testing::ext;
constexpr Dimension FILL_LENGTH = Dimension(1.0, DimensionUnit::PERCENT);
constexpr double DEFAULT_FRICTION = 0.6;
constexpr double NEW_DEFAULT_FRICTION = 0.7;
constexpr int32_t NULL_VALUE = -1;
} // namespace

class TestNG : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    RefPtr<PaintWrapper> FlushLayoutTask(const RefPtr<FrameNode>& frameNode);
    RefPtr<PaintWrapper> CreateDone(const RefPtr<FrameNode>& frameNode);
    uint64_t GetActions(const RefPtr<AccessibilityProperty>& accessibilityProperty);
    TouchEventInfo CreateTouchEventInfo(TouchType touchType, Offset location);
    static RefPtr<ThemeConstants> CreateThemeConstants(const std::string& patternName);

    AssertionResult IsEqual(const SizeF& actual, const SizeF& expected)
    {
        if (NearEqual(actual, expected)) {
            return AssertionSuccess();
        }
        return AssertionFailure() << "Actual: " << actual.ToString() << " Expected: " << expected.ToString();
    }

    AssertionResult IsEqual(const OverScrollOffset& actual, const OverScrollOffset& expected)
    {
        if (NearEqual(actual.start, expected.start) && NearEqual(actual.end, expected.end)) {
            return AssertionSuccess();
        }
        return AssertionFailure() << "Actual: "
                                  << "{ " << actual.start << " , " << actual.end << " }"
                                  << " Expected: "
                                  << "{ " << expected.start << " , " << expected.end << " }";
    }

    AssertionResult IsEqual(const Offset& actual, const Offset& expected)
    {
        if (NearEqual(actual, expected)) {
            return AssertionSuccess();
        }
        return AssertionFailure() << "Actual: " << actual.ToString() << " Expected: " << expected.ToString();
    }

    AssertionResult IsEqual(const OffsetF& actual, const OffsetF& expected)
    {
        if (NearEqual(actual, expected)) {
            return AssertionSuccess();
        }
        return AssertionFailure() << "Actual: " << actual.ToString() << " Expected: " << expected.ToString();
    }

    AssertionResult IsEqual(const Rect& actual, const Rect& expected)
    {
        if (NearEqual(actual, expected)) {
            return AssertionSuccess();
        }
        return AssertionFailure() << "Actual: " << actual.ToString() << " Expected: " << expected.ToString();
    }

    AssertionResult IsEqual(const RectF& actual, const RectF& expected)
    {
        if (NearEqual(actual, expected)) {
            return AssertionSuccess();
        }
        return AssertionFailure() << "Actual: " << actual.ToString() << " Expected: " << expected.ToString();
    }

    AssertionResult IsEqual(const ListItemIndex& actual, const ListItemIndex& expected)
    {
        if (actual.index == expected.index && actual.area == expected.area &&
            actual.indexInGroup == expected.indexInGroup) {
            return AssertionSuccess();
        }
        return AssertionFailure() << "Actual: "
                                  << "{ " << actual.index << " , " << actual.area << " , " << actual.indexInGroup
                                  << " }"
                                  << " Expected: "
                                  << "{ " << expected.index << " , " << expected.area << " , " << expected.indexInGroup
                                  << " }";
    }

    template<typename T>
    AssertionResult IsEqual(const T& actual, const T& expected)
    {
        if (NearEqual(actual, expected)) {
            return AssertionSuccess();
        }
        return AssertionFailure() << "Actual: " << actual << " Expected: " << expected;
    }

    RefPtr<FrameNode> GetChildFrameNode(const RefPtr<FrameNode>& frameNode, int32_t index)
    {
        return AceType::DynamicCast<FrameNode>(frameNode->GetChildAtIndex(index));
    }

    RefPtr<FocusHub> GetChildFocusHub(const RefPtr<FrameNode>& frameNode, int32_t index)
    {
        return GetChildFrameNode(frameNode, index)->GetOrCreateFocusHub();
    }

    template<typename T>
    RefPtr<T> GetChildPattern(const RefPtr<FrameNode>& frameNode, int32_t index)
    {
        RefPtr<Pattern> pattern = GetChildFrameNode(frameNode, index)->GetPattern();
        return AceType::DynamicCast<T>(pattern);
    }

    template<typename T>
    RefPtr<T> GetChildLayoutProperty(const RefPtr<FrameNode>& frameNode, int32_t index)
    {
        RefPtr<LayoutProperty> layoutProperty = GetChildFrameNode(frameNode, index)->GetLayoutProperty();
        return AceType::DynamicCast<T>(layoutProperty);
    }

    template<typename T>
    RefPtr<T> GetChildAccessibilityProperty(const RefPtr<FrameNode>& frameNode, int32_t index)
    {
        return GetChildFrameNode(frameNode, index)->GetAccessibilityProperty<T>();
    }

    template<typename T>
    RefPtr<T> GetChildEventHub(const RefPtr<FrameNode>& frameNode, int32_t index)
    {
        return GetChildFrameNode(frameNode, index)->GetEventHub<T>();
    }

    RectF GetChildRect(const RefPtr<FrameNode>& frameNode, int32_t index)
    {
        return GetChildFrameNode(frameNode, index)->GetGeometryNode()->GetFrameRect();
    }

    SizeF GetChildSize(const RefPtr<FrameNode>& frameNode, int32_t index)
    {
        return GetChildFrameNode(frameNode, index)->GetGeometryNode()->GetFrameSize();
    }

    OffsetF GetChildOffset(const RefPtr<FrameNode>& frameNode, int32_t index)
    {
        return GetChildFrameNode(frameNode, index)->GetGeometryNode()->GetFrameOffset();
    }

    float GetChildX(const RefPtr<FrameNode>& frameNode, int32_t index)
    {
        return GetChildRect(frameNode, index).GetX();
    }

    float GetChildY(const RefPtr<FrameNode>& frameNode, int32_t index)
    {
        return GetChildRect(frameNode, index).GetY();
    }

    float GetChildWidth(const RefPtr<FrameNode>& frameNode, int32_t index)
    {
        return GetChildRect(frameNode, index).Width();
    }

    float GetChildHeight(const RefPtr<FrameNode>& frameNode, int32_t index)
    {
        return GetChildRect(frameNode, index).Height();
    }

    void ClearOldNodes()
    {
        ElementRegister::GetInstance()->Clear();
    }

    ElementIdType GetElmtId()
    {
        elmtId_++;
        return elmtId_;
    }

    void ResetElmtId()
    {
        elmtId_ = ElementRegister::UndefinedElementId;
    }

    ElementIdType elmtId_ = ElementRegister::UndefinedElementId;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_SCROLL_PATTERN_H