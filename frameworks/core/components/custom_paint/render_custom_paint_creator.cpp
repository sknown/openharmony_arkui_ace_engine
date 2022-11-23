/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "flutter_render_custom_paint.h"
#include "rosen_render_custom_paint.h"

namespace OHOS::Ace {
RefPtr<RenderNode> RenderCustomPaint::Create()
{
    if (SystemProperties::GetRosenBackendEnabled()) {
#ifdef ENABLE_ROSEN_BACKEND
        return AceType::MakeRefPtr<RosenRenderCustomPaint>();
#else
        return nullptr;
#endif
    } else {
        return AceType::MakeRefPtr<FlutterRenderCustomPaint>();
    }
}

double RenderCustomPaint::PaintMeasureText(const std::string& text, double fontSize,
    int32_t fontStyle, const std::string& fontWeight, const std::string& fontFamily, double letterSpacing)
{
    if (SystemProperties::GetRosenBackendEnabled()) {
#ifdef ENABLE_ROSEN_BACKEND
        return RosenRenderCustomPaint::MeasureTextInner(text, fontSize, fontStyle,
            fontWeight, fontFamily, letterSpacing);
#else
        return 0.0;
#endif
    } else {
        return 0.0;
    }
}
} // namespace OHOS::Ace
