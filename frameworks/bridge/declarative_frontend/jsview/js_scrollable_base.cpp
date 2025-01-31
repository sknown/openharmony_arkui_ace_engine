/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "bridge/declarative_frontend/jsview/js_scrollable_base.h"

#include "bridge/declarative_frontend/jsview/js_view_common_def.h"
#include "core/components_ng/pattern/scrollable/scrollable_model_ng.h"

namespace OHOS::Ace::Framework {
void JSScrollableBase::JSFlingSpeedLimit(const JSCallbackInfo& info)
{
    double max = -1.0;
    if (!JSViewAbstract::ParseJsDouble(info[0], max)) {
        return;
    }
    NG::ScrollableModelNG::SetMaxFlingSpeed(max);
}

void JSScrollableBase::JsOnWillScroll(const JSCallbackInfo& args)
{
    if (args.Length() <= 0) {
        return;
    }
    if (args[0]->IsFunction()) {
        auto onScroll = [execCtx = args.GetExecutionContext(), func = JSRef<JSFunc>::Cast(args[0])](
                            const CalcDimension& scrollOffset, const ScrollState& scrollState,
                            ScrollSource scrollSource) {
            auto params = ConvertToJSValues(scrollOffset, scrollState, scrollSource);
            ScrollFrameResult scrollRes { .offset = scrollOffset };
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx, scrollRes);
            auto result = func->Call(JSRef<JSObject>(), params.size(), params.data());
            if (result.IsEmpty()) {
                return scrollRes;
            }

            if (!result->IsObject()) {
                return scrollRes;
            }

            auto resObj = JSRef<JSObject>::Cast(result);
            auto dxRemainValue = resObj->GetProperty("offsetRemain");
            if (dxRemainValue->IsNumber()) {
                scrollRes.offset = Dimension(dxRemainValue->ToNumber<float>(), DimensionUnit::VP);
            }
            return scrollRes;
        };
        NG::ScrollableModelNG::SetOnWillScroll(std::move(onScroll));
    } else {
        NG::ScrollableModelNG::SetOnWillScroll(nullptr);
    }
}

void JSScrollableBase::JsOnDidScroll(const JSCallbackInfo& args)
{
    if (args.Length() > 0 && args[0]->IsFunction()) {
        auto onScroll = [execCtx = args.GetExecutionContext(), func = JSRef<JSFunc>::Cast(args[0])](
                            const CalcDimension& scrollOffset, const ScrollState& scrollState) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            auto params = ConvertToJSValues(scrollOffset, scrollState);
            func->Call(JSRef<JSObject>(), params.size(), params.data());
        };
        NG::ScrollableModelNG::SetOnDidScroll(std::move(onScroll));
    }
}

void JSScrollableBase::JSBind(BindingTarget globalObj)
{
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSScrollableBase>::Declare("JSContainerBase");
    JSClass<JSScrollableBase>::StaticMethod("flingSpeedLimit", &JSScrollableBase::JSFlingSpeedLimit, opt);
    JSClass<JSScrollableBase>::StaticMethod("onWillScroll", &JSScrollableBase::JsOnWillScroll);
    JSClass<JSScrollableBase>::StaticMethod("onDidScroll", &JSScrollableBase::JsOnDidScroll);
    JSClass<JSScrollableBase>::InheritAndBind<JSContainerBase>(globalObj);
}
} // namespace OHOS::Ace::Framework
