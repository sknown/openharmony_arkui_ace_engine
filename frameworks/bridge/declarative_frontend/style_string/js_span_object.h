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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_STYLE_STRING_JS_SPAN_OBJECT_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_STYLE_STRING_JS_SPAN_OBJECT_H

#include "base/geometry/dimension.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "bridge/declarative_frontend/engine/bindings_defines.h"
#include "bridge/declarative_frontend/engine/js_types.h"
#include "bridge/declarative_frontend/jsview/js_container_base.h"
#include "core/components_ng/pattern/text/span/span_objects.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::Framework {

class JSFontSpan : public virtual AceType {
    DECLARE_ACE_TYPE(JSFontSpan, AceType)

public:
    JSFontSpan() = default;
    ~JSFontSpan() override = default;
    static void Constructor(const JSCallbackInfo& args);
    static void Destructor(JSFontSpan* fontSpan);
    static void JSBind(BindingTarget globalObj);
    static RefPtr<FontSpan> ParseJsFontSpan(JSRef<JSObject> obj);
    void GetColor(const JSCallbackInfo& info);
    void GetSize(const JSCallbackInfo& info);
    void GetWeight(const JSCallbackInfo& info);
    void GetFamily(const JSCallbackInfo& info);
    void GetStyle(const JSCallbackInfo& info);

    RefPtr<FontSpan>& GetFontSpan();
    void SetFontSpan(const RefPtr<FontSpan>& fontSpan);

private:
    ACE_DISALLOW_COPY_AND_MOVE(JSFontSpan);
    RefPtr<FontSpan> fontSpan_;
};
} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_STYLE_STRING_JS_SPAN_OBJECT_H