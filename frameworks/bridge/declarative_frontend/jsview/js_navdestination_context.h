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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_NAVDESTINATION_CONTEXT_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_NAVDESTINATION_CONTEXT_H

#include <optional>
#include <string>

#include "base/memory/referenced.h"
#include "bridge/declarative_frontend/engine/bindings.h"
#include "bridge/declarative_frontend/engine/functions/js_function.h"
#include "bridge/declarative_frontend/engine/js_types.h"
#include "bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "core/components_ng/pattern/navrouter/navdestination_context.h"

namespace OHOS::Ace::Framework {
class JSNavPathInfo : public NG::NavPathInfo {
    DECLARE_ACE_TYPE(JSNavPathInfo, NG::NavPathInfo)
public:
    JSNavPathInfo() = default;
    JSNavPathInfo(const std::string& name, JSRef<JSVal> param) : NG::NavPathInfo(name), param_(param) {}
    ~JSNavPathInfo() = default;

    void SetParam(const JSRef<JSVal>& param)
    {
        param_ = param;
    }

    JSRef<JSVal> GetParam() const
    {
        return param_;
    }

private:
    JSRef<JSVal> param_;
};

class JSNavDestinationContext : public NG::NavDestinationContext {
    DECLARE_ACE_TYPE(JSNavDestinationContext, NG::NavDestinationContext)
public:
    JSNavDestinationContext() = default;
    ~JSNavDestinationContext() = default;

    JSRef<JSObject> CreateJSObject();

private:
    JSRef<JSObject> CreateJSNavPathInfo() const;
    JSRef<JSObject> CreateJSNavPathStack() const;
};
} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_NAVDESTINATION_CONTEXT_H
