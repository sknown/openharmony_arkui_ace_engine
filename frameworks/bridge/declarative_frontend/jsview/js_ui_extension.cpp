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

#include "bridge/declarative_frontend/jsview/js_ui_extension.h"

#include <functional>
#include <string>

#include "base/log/ace_scoring_log.h"
#include "bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_model.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_model_ng.h"

namespace OHOS::Ace {
std::unique_ptr<UIExtensionModel> UIExtensionModel::instance_ = nullptr;
std::mutex UIExtensionModel::mutex_;

UIExtensionModel* UIExtensionModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
#ifdef NG_BUILD
            instance_.reset(new NG::UIExtensionModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                instance_.reset(new NG::UIExtensionModelNG());
            } else {
                return nullptr;
            }
#endif
        }
    }
    return instance_.get();
}
} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {
void JSUIExtension::JSBind(BindingTarget globalObj)
{
    JSClass<JSUIExtension>::Declare("UIExtensionComponent");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSUIExtension>::StaticMethod("create", &JSUIExtension::Create, opt);
    JSClass<JSUIExtension>::StaticMethod("onConnected", &JSUIExtension::SetOnConnect, opt);
    JSClass<JSUIExtension>::StaticMethod("onDisconnected", &JSUIExtension::SetOnDisconnect, opt);
    JSClass<JSUIExtension>::StaticMethod("onError", &JSUIExtension::SetOnError, opt);
    JSClass<JSUIExtension>::StaticMethod("onResult", &JSUIExtension::SetOnResult, opt);
    JSClass<JSUIExtension>::InheritAndBind<JSViewAbstract>(globalObj);
}

void JSUIExtension::Create(const JSCallbackInfo& info)
{
    if (!info[0]->IsObject()) {
        return;
    }

    auto obj = JSRef<JSObject>::Cast(info[0]);
    // Parse want
    auto want = JSRef<JSObject>::Cast(obj->GetProperty("want"));
    if (want->GetProperty("bundleName")->IsNull() || want->GetProperty("bundleName")->IsUndefined() ||
        want->GetProperty("abilityName")->IsNull() || want->GetProperty("abilityName")->IsUndefined()) {
        return;
    }
    std::string bundleName = want->GetProperty("bundleName")->ToString();
    std::string abilityName = want->GetProperty("abilityName")->ToString();

    UIExtensionModel::GetInstance()->Create(bundleName, abilityName);
}

void JSUIExtension::SetOnConnect(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }
    auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onConnect = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc)]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("UIExtension.onConnected");
        auto newJSVal = JSRef<JSVal>::Make();
        func->ExecuteJS(1, &newJSVal);
    };
}

void JSUIExtension::SetOnDisconnect(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }
    auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onConnect = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc)]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("UIExtension.Disconnected");
        auto newJSVal = JSRef<JSVal>::Make();
        func->ExecuteJS(1, &newJSVal);
    };
}

void JSUIExtension::SetOnError(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }
    auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
}

void JSUIExtension::SetOnResult(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }
    auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
}
} // namespace OHOS::Ace::Framework
