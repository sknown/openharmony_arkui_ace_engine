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

#include "bridge/declarative_frontend/jsview/js_isolated_component.h"

#include <cstdint>
#include <functional>
#include <string>

#include "commonlibrary/ets_utils/js_concurrent_module/worker/worker.h"
#include "jsnapi.h"
#include "native_engine.h"

#include "base/log/ace_scoring_log.h"
#include "base/log/log_wrapper.h"
#include "base/memory/ace_type.h"
#include "base/thread/task_executor.h"
#include "base/utils/utils.h"
#include "bridge/common/utils/engine_helper.h"
#include "bridge/declarative_frontend/engine/js_converter.h"
#include "bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "bridge/declarative_frontend/engine/js_types.h"
#include "bridge/declarative_frontend/engine/jsi/jsi_ref.h"
#include "bridge/declarative_frontend/engine/jsi/jsi_types.h"
#include "bridge/declarative_frontend/jsview/js_utils.h"
#include "bridge/js_frontend/engine/jsi/js_value.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_model.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_model_ng.h"

using namespace Commonlibrary::Concurrent::WorkerModule;

namespace OHOS::Ace::Framework {

static Worker* ParseWorker(const JSRef<JSVal>& jsWorker)
{
    auto hostEngine = EngineHelper::GetCurrentEngine();
    CHECK_NULL_RETURN(hostEngine, nullptr);
    NativeEngine* hostNativeEngine = hostEngine->GetNativeEngine();
    CHECK_NULL_RETURN(hostNativeEngine, nullptr);
    panda::Local<JsiValue> value = jsWorker.Get().GetLocalHandle();
    JSValueWrapper valueWrapper = value;
    napi_value nativeValue = hostNativeEngine->ValueToNapiValue(valueWrapper);
    Worker* worker = nullptr;
    napi_unwrap(reinterpret_cast<napi_env>(hostNativeEngine),
        nativeValue, reinterpret_cast<void**>(&worker));
    return worker;
}

void JSIsolatedComponent::JSBind(BindingTarget globalObj)
{
    JSClass<JSIsolatedComponent>::Declare("IsolatedComponent");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSIsolatedComponent>::StaticMethod("create", &JSIsolatedComponent::Create, opt);
    JSClass<JSIsolatedComponent>::StaticMethod("onError", &JSIsolatedComponent::JsOnError, opt);
    JSClass<JSIsolatedComponent>::StaticMethod("onSizeChanged",
        &JSIsolatedComponent::SetOnSizeChanged, opt);
    JSClass<JSIsolatedComponent>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSIsolatedComponent>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSIsolatedComponent>::InheritAndBind<JSViewAbstract>(globalObj);
}

void JSIsolatedComponent::Create(const JSCallbackInfo& info)
{
    if (info.Length() < 1 || !info[0]->IsObject()) {
        TAG_LOGW(AceLogTag::ACE_ISOLATED_COMPONENT, "IsolatedComponent argument is invalid");
        return;
    }

    auto obj = JSRef<JSObject>::Cast(info[0]);
    JSRef<JSVal> wantObj = obj->GetProperty("want");
    if (!wantObj->IsObject()) {
        TAG_LOGW(AceLogTag::ACE_ISOLATED_COMPONENT, "IsolatedComponent want is invalid");
        return;
    }

    RefPtr<OHOS::Ace::WantWrap> want = CreateWantWrapFromNapiValue(wantObj);
    CHECK_NULL_VOID(want);
    Worker* worker = ParseWorker(obj->GetProperty("worker"));
    if (worker == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ISOLATED_COMPONENT, "worker is null");
        return;
    }

    TAG_LOGI(AceLogTag::ACE_ISOLATED_COMPONENT, "worker running=%{public}d,  worker name=%{public}s",
        worker->IsRunning(), worker->GetName().c_str());
    UIExtensionModel::GetInstance()->Create();
    auto frameNode = NG::ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto instanceId = Container::CurrentId();
    auto weak = AceType::WeakClaim(frameNode);
    worker->RegisterCallbackForWorkerEnv([instanceId, weak, want](napi_env env) {
        ContainerScope scope(instanceId);
        auto container = Container::Current();
        container->GetTaskExecutor()->PostTask(
            [weak, want, env]() {
                auto frameNode = weak.Upgrade();
                CHECK_NULL_VOID(frameNode);
                UIExtensionModel::GetInstance()->InitializeIsolatedComponent(
                    frameNode, want, env);
            },
            TaskExecutor::TaskType::UI, "ArkUIIsolatedComponentInitialize");
    });
}

void JSIsolatedComponent::JsOnError(const JSCallbackInfo& info)
{
    if (info.Length() < 1 || !info[0]->IsFunction()) {
        TAG_LOGW(AceLogTag::ACE_ISOLATED_COMPONENT, "onError argument is invalid");
        return;
    }

    WeakPtr<NG::FrameNode> frameNode =
        AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto execCtx = info.GetExecutionContext();
    auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto instanceId = Container::CurrentId();
    auto onError = [execCtx, func = std::move(jsFunc), instanceId, node = frameNode]
        (int32_t code, const std::string& name, const std::string& message) {
            ContainerScope scope(instanceId);
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("IsolatedComponent.onError");
            auto pipelineContext = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipelineContext);
            pipelineContext->UpdateCurrentActiveNode(node);
            JSRef<JSObject> obj = JSRef<JSObject>::New();
            obj->SetProperty<int32_t>("code", code);
            obj->SetProperty<std::string>("name", name);
            obj->SetProperty<std::string>("message", message);
            auto returnValue = JSRef<JSVal>::Cast(obj);
            func->ExecuteJS(1, &returnValue);
        };
    UIExtensionModel::GetInstance()->SetPlatformOnError(std::move(onError));
}

void JSIsolatedComponent::SetOnSizeChanged(const JSCallbackInfo& info)
{
    if (info.Length() < 1 || !info[0]->IsFunction()) {
        TAG_LOGW(AceLogTag::ACE_ISOLATED_COMPONENT, "OnSizeChanged argument is invalid");
        return;
    }
    auto execCtx = info.GetExecutionContext();
    auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onCardSizeChanged = [execCtx, func = std::move(jsFunc)](int32_t width, int32_t height) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("IsolatedComponent.onSizeChanged");
        JSRef<JSObject> obj = JSRef<JSObject>::New();
        obj->SetProperty<int32_t>("width", width);
        obj->SetProperty<int32_t>("height", height);
        auto returnValue = JSRef<JSVal>::Cast(obj);
        func->ExecuteJS(1, &returnValue);
    };
    UIExtensionModel::GetInstance()->SetOnSizeChanged(std::move(onCardSizeChanged));
}
} // namespace OHOS::Ace::Framework
