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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEST_UNITTEST_RELATIVE_CONTAINER_RELATIVE_CONTAINER_TEST_UTILS_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEST_UNITTEST_RELATIVE_CONTAINER_RELATIVE_CONTAINER_TEST_UTILS_H

#include "core/components/root/render_root.h"
#include "core/components/relative_container/render_relative_container.h"
#include "core/components/text/render_text.h"


namespace OHOS::Ace {
    constexpr double LENGTH = 356.0;

class MockRenderRoot final : public RenderRoot {
    DECLARE_ACE_TYPE(MockRenderRoot, RenderRoot);
};

class MockRenderRelativeContainer final : public RenderRelativeContainer {
    DECLARE_ACE_TYPE(MockRenderRelativeContainer, RenderRelativeContainer);
};

class MockRenderText final : public RenderText {
    DECLARE_ACE_TYPE(MockRenderText, RenderText);

public:
    uint32_t GetTextLines() override
    {
        return 0;
    }

    Size Measure() override
    {
        return GetLayoutSize();
    }

    double GetTextWidth() override
    {
        return 0;
    }

    int32_t GetTouchPosition(const Offset& offset) override
    {
        return 0;
    }

    void ShowTextOverlay(const Offset& showOffset) override
    {
        return;
    }

    void RegisterCallbacksToOverlay() override
    {
        return;
    }

    Offset GetHandleOffset(int32_t extend) override
    {
        return Offset();
    }
};

class RelativeContainerTestUtils {
public:
    static RefPtr<RenderRoot> CreateRenderRoot();
    static RefPtr<RenderText> CreateRenderText(const RefPtr<PipelineContext>& context);
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEST_UNITTEST_RELATIVE_CONTAINER_RELATIVE_CONTAINER_TEST_UTILS_H