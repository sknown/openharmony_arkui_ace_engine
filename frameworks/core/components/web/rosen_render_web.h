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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_ROSEN_RENDER_WEB_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_ROSEN_RENDER_WEB_H

#include "core/components/web/render_web.h"

namespace OHOS::Ace {

class RosenRenderWeb final : public RenderWeb {
    DECLARE_ACE_TYPE(RosenRenderWeb, RenderWeb);

public:
    void DumpTree(int32_t depth) override;

#ifdef OHOS_STANDARD_SYSTEM
    void Update(const RefPtr<Component>& component) override;
    void OnAttachContext() override;
    void Paint(RenderContext& context, const Offset& offset) override;
    void SyncGeometryProperties() override;
    std::shared_ptr<RSNode> CreateRSNode() const override;
#endif

private:
    void PerformLayout() override;
#ifdef OHOS_STANDARD_SYSTEM
    OHOS::sptr<OHOS::Surface> GetSurface();
#endif
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_ROSEN_RENDER_WEB_H
