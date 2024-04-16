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

#ifndef FOUNDATION_ACE_FRAMEWORK_JAVASCRIPT_BRIDGE_JS_VIEW_JS_DRAWING_RENDERING_CONTEXT_H
#define FOUNDATION_ACE_FRAMEWORK_JAVASCRIPT_BRIDGE_JS_VIEW_JS_DRAWING_RENDERING_CONTEXT_H

#include "base/memory/referenced.h"
#include "bridge/declarative_frontend/engine/bindings_defines.h"
#include "bridge/declarative_frontend/engine/js_converter.h"
#include "core/components_ng/base/modifier.h"
#include "core/pipeline/base/rosen_render_context.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_rendering_context_settings.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "frameworks/core/components_ng/pattern/custom_paint/custom_paint_pattern.h"

namespace OHOS::Ace::Framework {

class JSDrawingRenderingContext : public Referenced {
public:
    JSDrawingRenderingContext();
    ~JSDrawingRenderingContext() override = default;

    static void JSBind(BindingTarget globalObj);
    static void Constructor(const JSCallbackInfo& args);
    static void Destructor(JSDrawingRenderingContext* controller);

    void JsGetCanvas(const JSCallbackInfo& info);
    void JsGetSize(const JSCallbackInfo& info);
    void JsSetCanvas(const JSCallbackInfo& info);
    void JsSetSize(const JSCallbackInfo& info);
    void SetInvalidate(const JSCallbackInfo& info);
    void SetRSCanvasCallback(RefPtr<AceType>& canvasPattern);

    ACE_DISALLOW_COPY_AND_MOVE(JSDrawingRenderingContext);
    void SetCanvasPattern(const RefPtr<AceType>& canvas)
    {
        canvasPattern_ = canvas;
    }
    void SetInstanceId(int32_t id)
    {
        instanceId_ = id;
    }

protected:
    RefPtr<AceType> canvasPattern_;
    int32_t instanceId_ = INSTANCE_ID_UNDEFINED;

private:
    JSRef<JSVal> jsCanvasVal_;
    NG::OptionalSizeF size_;
};
} // namespace OHOS::Ace::Framework

#endif // FOUNDATION_ACE_FRAMEWORK_JAVASCRIPT_BRIDGE_JS_VIEW_JS_DRAWING_RENDERING_CONTEXT_H
