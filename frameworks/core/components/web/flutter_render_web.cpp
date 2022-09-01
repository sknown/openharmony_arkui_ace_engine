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

#include "core/components/web/flutter_render_web.h"
namespace OHOS::Ace {

using namespace Flutter;

void FlutterRenderWeb::PerformLayout()
{
    RenderWeb::PerformLayout();
}

void FlutterRenderWeb::DumpTree(int32_t depth) {}

#if !defined(PREVIEW) and defined(OHOS_STANDARD_SYSTEM)
void FlutterRenderWeb::OnPaintFinish()
{
    if (!delegate_) {
        return;
    }
    if (!isCreateWebView_) {
        isCreateWebView_ = true;
        delegate_->InitWebViewWithWindow();
    }
}
#endif

} // namespace OHOS::Ace
