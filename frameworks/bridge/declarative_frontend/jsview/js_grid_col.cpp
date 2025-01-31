/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "frameworks/bridge/declarative_frontend/jsview/js_grid_col.h"

#include <cstdint>

#include "base/log/ace_trace.h"
#include "bridge/declarative_frontend/jsview/js_view_common_def.h"
#include "bridge/declarative_frontend/jsview/models/grid_col_model_impl.h"
#include "core/components_ng/pattern/grid_col/grid_col_model_ng.h"

namespace OHOS::Ace {

std::unique_ptr<GridColModel> GridColModel::instance_;
std::mutex GridColModel::mutex_;

GridColModel* GridColModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
#ifdef NG_BUILD
            instance_.reset(new NG::GridColModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                instance_.reset(new NG::GridColModelNG());
            } else {
                instance_.reset(new Framework::GridColModelImpl());
            }
#endif
        }
    }
    return instance_.get();
}

} // namespace OHOS::Ace
namespace OHOS::Ace::Framework {
namespace {
constexpr size_t XS = 0;
constexpr size_t SM = 1;
constexpr size_t MD = 2;
constexpr size_t LG = 3;
constexpr size_t XL = 4;
constexpr size_t XXL = 5;
constexpr size_t MAX_NUMBER_BREAKPOINT = 6;

void InheritGridContainerSize(const RefPtr<V2::GridContainerSize>& gridContainerSize,
    std::optional<int32_t> (&containerSizeArray)[MAX_NUMBER_BREAKPOINT], int32_t defaultVal)
{
    if (!containerSizeArray[0].has_value()) {
        containerSizeArray[0] = defaultVal;
    }
    for (size_t i = 1; i < MAX_NUMBER_BREAKPOINT; i++) {
        if (!containerSizeArray[i].has_value()) {
            containerSizeArray[i] = containerSizeArray[i - 1].value();
        }
    }
    gridContainerSize->xs = containerSizeArray[0].value();
    gridContainerSize->sm = containerSizeArray[1].value();
    gridContainerSize->md = containerSizeArray[2].value();
    gridContainerSize->lg = containerSizeArray[3].value();
    gridContainerSize->xl = containerSizeArray[4].value();
    gridContainerSize->xxl = containerSizeArray[5].value();
}

void ParseGridContainerSizeArray(const JSRef<JSVal>& jsValue,
    std::optional<int32_t> (&containerSizeArray)[MAX_NUMBER_BREAKPOINT], bool isOrder)
{
    auto gridParam = JSRef<JSObject>::Cast(jsValue);
    auto xs = gridParam->GetProperty("xs");
    if (xs->IsNumber() && xs->ToNumber<int32_t>() >= 0) {
        containerSizeArray[XS] = xs->ToNumber<int32_t>();
    } else if (isOrder) {
        containerSizeArray[XS] = 0;
    }
    auto sm = gridParam->GetProperty("sm");
    if (sm->IsNumber() && sm->ToNumber<int32_t>() >= 0) {
        containerSizeArray[SM] = sm->ToNumber<int32_t>();
    } else if (isOrder) {
        containerSizeArray[SM] = 0;
    }
    auto md = gridParam->GetProperty("md");
    if (md->IsNumber() && md->ToNumber<int32_t>() >= 0) {
        containerSizeArray[MD] = md->ToNumber<int32_t>();
    } else if (isOrder) {
        containerSizeArray[MD] = 0;
    }
    auto lg = gridParam->GetProperty("lg");
    if (lg->IsNumber() && lg->ToNumber<int32_t>() >= 0) {
        containerSizeArray[LG] = lg->ToNumber<int32_t>();
    } else if (isOrder) {
        containerSizeArray[LG] = 0;
    }
    auto xl = gridParam->GetProperty("xl");
    if (xl->IsNumber() && xl->ToNumber<int32_t>() >= 0) {
        containerSizeArray[XL] = xl->ToNumber<int32_t>();
    } else if (isOrder) {
        containerSizeArray[XL] = 0;
    }
    auto xxl = gridParam->GetProperty("xxl");
    if (xxl->IsNumber() && xxl->ToNumber<int32_t>() >= 0) {
        containerSizeArray[XXL] = xxl->ToNumber<int32_t>();
    } else if (isOrder) {
        containerSizeArray[XXL] = 0;
    }
}

RefPtr<V2::GridContainerSize> ParserGridContainerSize(const JSRef<JSVal>& jsValue, int32_t defaultVal, bool isOrder)
{
    if (jsValue->IsNumber()) {
        double columnNumber = 0.0;
        JSViewAbstract::ParseJsDouble(jsValue, columnNumber);
        auto gridContainerSize = columnNumber >= 0 ? AceType::MakeRefPtr<V2::GridContainerSize>(columnNumber)
                                                   : AceType::MakeRefPtr<V2::GridContainerSize>(defaultVal);
        return gridContainerSize;
    } else if (jsValue->IsObject()) {
        auto gridContainerSize = AceType::MakeRefPtr<V2::GridContainerSize>(defaultVal);
        std::optional<int32_t> containerSizeArray[MAX_NUMBER_BREAKPOINT];
        ParseGridContainerSizeArray(jsValue, containerSizeArray, isOrder);
        InheritGridContainerSize(gridContainerSize, containerSizeArray, defaultVal);
        return gridContainerSize;
    } else {
        return AceType::MakeRefPtr<V2::GridContainerSize>(defaultVal);
    }
}

} // namespace

void JSGridCol::Create(const JSCallbackInfo& info)
{
    if (info.Length() > 0 && info[0]->IsObject()) {
        auto gridParam = JSRef<JSObject>::Cast(info[0]);
        auto spanParam = gridParam->GetProperty("span");
        auto offsetParam = gridParam->GetProperty("offset");
        auto orderParam = gridParam->GetProperty("order");
        auto span = ParserGridContainerSize(spanParam, 1, false);
        auto offset = ParserGridContainerSize(offsetParam, 0, false);
        auto order = ParserGridContainerSize(orderParam, 0, true);

        GridColModel::GetInstance()->Create(span, offset, order);
    } else {
        GridColModel::GetInstance()->Create();
    }
}

void JSGridCol::Span(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    auto span = ParserGridContainerSize(info[0], 1, false);
    GridColModel::GetInstance()->SetSpan(span);
}

void JSGridCol::Offset(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    if (info[0]->IsObject()) {
        auto obj = JSRef<JSObject>::Cast(info[0]);
        auto xVal = obj->GetProperty("x");
        auto yVal = obj->GetProperty("y");
        if (!xVal->IsUndefined() || !yVal->IsUndefined()) {
            JSViewAbstract::JsOffset(info);
            return;
        }
    }

    auto offset = ParserGridContainerSize(info[0], 0, false);
    GridColModel::GetInstance()->SetOffset(offset);
}

void JSGridCol::Order(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    auto order = ParserGridContainerSize(info[0], 0, true);
    GridColModel::GetInstance()->SetOrder(order);
}

void JSGridCol::JSBind(BindingTarget globalObj)
{
    JSClass<JSGridCol>::Declare("GridCol");
    JSClass<JSGridCol>::StaticMethod("create", &JSGridCol::Create, MethodOptions::NONE);
    JSClass<JSGridCol>::StaticMethod("span", &JSGridCol::Span, MethodOptions::NONE);
    JSClass<JSGridCol>::StaticMethod("offset", &JSGridCol::Offset, MethodOptions::NONE);
    JSClass<JSGridCol>::StaticMethod("gridColOffset", &JSGridCol::Offset, MethodOptions::NONE);
    JSClass<JSGridCol>::StaticMethod("order", &JSGridCol::Order, MethodOptions::NONE);
    JSClass<JSGridCol>::StaticMethod("onDetach", &JSInteractableView::JsOnDetach);
    JSClass<JSGridCol>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSGridCol>::StaticMethod("onAttach", &JSInteractableView::JsOnAttach);
    JSClass<JSGridCol>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSGridCol>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSGridCol>::InheritAndBind<JSContainerBase>(globalObj);
}

} // namespace OHOS::Ace::Framework
