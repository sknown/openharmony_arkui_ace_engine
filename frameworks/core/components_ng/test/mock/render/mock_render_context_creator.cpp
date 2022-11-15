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

#include "core/components_ng/test/mock/render/mock_render_context.h"

namespace OHOS::Ace::NG {
RefPtr<RenderContext> RenderContext::Create()
{
    auto mockRenderContext = MakeRefPtr<MockRenderContext>();
    EXPECT_CALL(*AceType::RawPtr(mockRenderContext), SetClipToBounds(true)).Times(1);
    return mockRenderContext;
}
} // namespace OHOS::Ace::NG
