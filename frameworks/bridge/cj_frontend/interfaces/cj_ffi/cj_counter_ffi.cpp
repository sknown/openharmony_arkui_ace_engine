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

#include "bridge/cj_frontend/interfaces/cj_ffi/cj_counter_ffi.h"

#include "cj_lambda.h"
#include "bridge/cj_frontend/interfaces/cj_ffi/cj_view_abstract_ffi.h"
#include "core/components_ng/pattern/counter/counter_model.h"
#include "core/components_ng/pattern/counter/counter_model_ng.h"

using namespace OHOS::Ace;
using namespace OHOS::Ace::Framework;

extern "C" {
void FfiOHOSAceFrameworkCounterCreate()
{
    CounterModel::GetInstance()->Create();
}

void FfiOHOSAceFrameworkCounterSetWidth(double value, int32_t unit)
{
    Dimension dValue(value, static_cast<DimensionUnit>(unit));
    if (LessNotEqual(dValue.Value(), 0.0)) {
        dValue.SetValue(0.0);
    }
    CounterModel::GetInstance()->SetWidth(dValue);
}

void FfiOHOSAceFrameworkCounterSetHeight(double value, int32_t unit)
{
    Dimension dValue(value, static_cast<DimensionUnit>(unit));
    if (LessNotEqual(dValue.Value(), 0.0)) {
        dValue.SetValue(0.0);
    }
    CounterModel::GetInstance()->SetHeight(dValue);
}

void FfiOHOSAceFrameworkCounterSetSize(double width, int32_t widthUnit, double height, int32_t heightUnit)
{
    FfiOHOSAceFrameworkCounterSetWidth(width, widthUnit);
    FfiOHOSAceFrameworkCounterSetHeight(height, heightUnit);
}

void FfiOHOSAceFrameworkCounterSetControlWidth(double value, int32_t unit)
{
    Dimension dValue(value, static_cast<DimensionUnit>(unit));
    if (LessNotEqual(dValue.Value(), 0.0)) {
        dValue.SetValue(0.0);
    }
    CounterModel::GetInstance()->SetControlWidth(dValue);
}

void FfiOHOSAceFrameworkCounterSetStateChange(bool state)
{
    CounterModel::GetInstance()->SetStateChange(state);
}

void FfiOHOSAceFrameworkCounterSetBackgroundColor(uint32_t color)
{
    CounterModel::GetInstance()->SetBackgroundColor(Color(color));
}

void FfiOHOSAceFrameworkCounterSetOnInc(void (*callback)())
{
    CounterModel::GetInstance()->SetOnInc(CJLambda::Create(callback));
}

void FfiOHOSAceFrameworkCounterSetOnDec(void (*callback)())
{
    CounterModel::GetInstance()->SetOnDec(CJLambda::Create(callback));
}
}
