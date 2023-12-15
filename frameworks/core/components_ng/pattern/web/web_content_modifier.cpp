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

#include "core/components_ng/pattern/web/web_content_modifier.h"
#include "core/components_ng/render/drawing.h"

#ifdef ENABLE_ROSEN_BACKEND
#include "pipeline/rs_recording_canvas.h"
#endif

#if defined (OHOS_STANDARD_SYSTEM) && defined (ENABLE_ROSEN_BACKEND)
#include <ui/rs_surface_node.h>
#endif
#include "core/components_ng/render/adapter/rosen_render_surface.h"
namespace OHOS::Ace::NG {
void WebContentModifier::onDraw(DrawingContext& drawingContext)
{
    ACE_SCOPED_TRACE("WebContentModifier::onDraw");
    auto surface = DynamicCast<NG::RosenRenderSurface>(renderSuface_);
    if (surface) {
        surface->DrawBuffer();
        return;
    }
}
} // namespace OHOS::Ace::NG