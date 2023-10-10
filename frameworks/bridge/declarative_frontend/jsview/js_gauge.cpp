/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "bridge/declarative_frontend/jsview/js_gauge.h"

#include <string>

#include "base/log/ace_scoring_log.h"
#include "base/memory/ace_type.h"
#include "bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "bridge/declarative_frontend/jsview/js_linear_gradient.h"
#include "bridge/declarative_frontend/jsview/js_utils.h"
#include "bridge/declarative_frontend/jsview/models/gauge_model_impl.h"
#include "core/components_ng/base/view_stack_model.h"
#include "core/components_ng/pattern/gauge/gauge_model_ng.h"
#include "core/components_ng/pattern/gauge/gauge_theme.h"

namespace OHOS::Ace {

std::unique_ptr<GaugeModel> GaugeModel::instance_ = nullptr;
std::mutex GaugeModel::mutex_;

GaugeModel* GaugeModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
#ifdef NG_BUILD
            instance_.reset(new NG::GaugeModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                instance_.reset(new NG::GaugeModelNG());
            } else {
                instance_.reset(new Framework::GaugeModelImpl());
            }
#endif
        }
    }
    return instance_.get();
}

} // namespace OHOS::Ace
namespace OHOS::Ace::Framework {

void JSGauge::JSBind(BindingTarget globalObj)
{
    JSClass<JSGauge>::Declare("Gauge");
    JSClass<JSGauge>::StaticMethod("create", &JSGauge::Create);

    JSClass<JSGauge>::StaticMethod("value", &JSGauge::SetValue);
    JSClass<JSGauge>::StaticMethod("startAngle", &JSGauge::SetStartAngle);
    JSClass<JSGauge>::StaticMethod("endAngle", &JSGauge::SetEndAngle);
    JSClass<JSGauge>::StaticMethod("colors", &JSGauge::SetColors);
    JSClass<JSGauge>::StaticMethod("strokeWidth", &JSGauge::SetStrokeWidth);
    JSClass<JSGauge>::StaticMethod("labelConfig", &JSGauge::SetLabelConfig);
    JSClass<JSGauge>::StaticMethod("trackShadow", &JSGauge::SetShadowOptions);
    JSClass<JSGauge>::StaticMethod("indicator", &JSGauge::SetIndicator);
    JSClass<JSGauge>::StaticMethod("description", &JSGauge::SetDescription);
    JSClass<JSGauge>::StaticMethod("onClick", &JSInteractableView::JsOnClick);
    JSClass<JSGauge>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSGauge>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSGauge>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSGauge>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSGauge>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);

    JSClass<JSGauge>::InheritAndBind<JSViewAbstract>(globalObj);
}

void JSGauge::Create(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsObject()) {
        LOGE("gauge create error, info is non-valid");
        return;
    }

    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    auto value = paramObject->GetProperty("value");
    auto min = paramObject->GetProperty("min");
    auto max = paramObject->GetProperty("max");

    double gaugeMin = min->IsNumber() ? min->ToNumber<double>() : 0;
    double gaugeMax = max->IsNumber() ? max->ToNumber<double>() : 100;
    double gaugeValue = value->IsNumber() ? value->ToNumber<double>() : 0;
    GaugeModel::GetInstance()->Create(gaugeValue, gaugeMin, gaugeMax);
    if (min->IsNumber() || max->IsNumber()) {
        GaugeModel::GetInstance()->SetIsShowLimitValue(true);
    } else {
        GaugeModel::GetInstance()->SetIsShowLimitValue(false);
    }
}

void JSGauge::SetValue(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsNumber()) {
        LOGE("JSGauge::SetValue::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    GaugeModel::GetInstance()->SetValue(info[0]->ToNumber<float>());
}

void JSGauge::SetStartAngle(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsNumber()) {
        LOGE("JSGauge::SetStartAngle::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    GaugeModel::GetInstance()->SetStartAngle(info[0]->ToNumber<float>());
}

void JSGauge::SetEndAngle(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsNumber()) {
        LOGE("JSGauge::SetEndAngle::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    GaugeModel::GetInstance()->SetEndAngle(info[0]->ToNumber<float>());
}

void JSGauge::SetColors(const JSCallbackInfo& info)
{
    if (!Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        SetGradientColors(info);
        return;
    }

    if (info.Length() < 1 || !info[0]->IsArray()) {
        LOGE("The number of argument is less than 1, or the argument is not array.");
        return;
    }
    std::vector<Color> colors;
    std::vector<double> values;
    std::vector<float> weights;
    auto theme = GetTheme<ProgressTheme>();
    auto jsColor = JSRef<JSArray>::Cast(info[0]);
    for (size_t i = 0; i < jsColor->Length(); ++i) {
        JSRef<JSVal> jsValue = jsColor->GetValueAt(i);
        if (!jsValue->IsArray()) {
            return;
        }
        JSRef<JSArray> tempColors = jsColor->GetValueAt(i);
        double value = tempColors->GetValueAt(1)->ToNumber<double>();
        float weight = tempColors->GetValueAt(1)->ToNumber<float>();
        Color selectedColor;
        if (!ParseJsColor(tempColors->GetValueAt(0), selectedColor)) {
            selectedColor = Color::BLACK;
        }
        colors.push_back(selectedColor);
        values.push_back(value);
        if (weight > 0) {
            weights.push_back(weight);
        } else {
            weights.push_back(0.0f);
        }
    }
    GaugeModel::GetInstance()->SetColors(colors, weights);
}

void JSGauge::SetGradientColors(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGW("The number of argument is less than 1, or the argument is not array.");
        return;
    }

    if (info[0]->IsNull() || info[0]->IsUndefined()) {
        return;
    }

    NG::GaugeType type = NG::GaugeType::TYPE_CIRCULAR_MULTI_SEGMENT_GRADIENT;
    std::vector<NG::ColorStopArray> colors;
    std::vector<float> weights;
    if (info[0]->IsArray()) {
        auto jsColorsArray = JSRef<JSArray>::Cast(info[0]);
        for (size_t i = 0; i < jsColorsArray->Length(); ++i) {
            if (static_cast<int32_t>(i) >= NG::COLORS_MAX_COUNT) {
                break;
            }
            JSRef<JSVal> jsValue = jsColorsArray->GetValueAt(i);
            if (!jsValue->IsArray()) {
                continue;
            }
            auto tempColors = JSRef<JSArray>::Cast(jsValue);
            // Get weight
            float weight = tempColors->GetValueAt(1)->ToNumber<float>();
            weight = Negative(weight) ? 0.0f : weight;
            weights.push_back(weight);
            // Get color
            JSRef<JSVal> jsColorValue = tempColors->GetValueAt(0);
            ConvertGradientColor(jsColorValue, colors, type);
        }
        type = NG::GaugeType::TYPE_CIRCULAR_MULTI_SEGMENT_GRADIENT;
        GaugeModel::GetInstance()->SetGradientColors(colors, weights, type);
        return;
    }
    ConvertGradientColor(info[0], colors, type);
    GaugeModel::GetInstance()->SetGradientColors(colors, weights, type);
}

void JSGauge::ConvertGradientColor(
    const JsiRef<JsiValue>& itemParam, std::vector<NG::ColorStopArray>& colors, NG::GaugeType& type)
{
    if (!itemParam->IsObject()) {
        type = NG::GaugeType::TYPE_CIRCULAR_MONOCHROME;
        return ConvertResourceColor(itemParam, colors);
    }

    JSLinearGradient* jsLinearGradient = JSRef<JSObject>::Cast(itemParam)->Unwrap<JSLinearGradient>();
    if (!jsLinearGradient) {
        type = NG::GaugeType::TYPE_CIRCULAR_MONOCHROME;
        return ConvertResourceColor(itemParam, colors);
    }

    type = NG::GaugeType::TYPE_CIRCULAR_SINGLE_SEGMENT_GRADIENT;
    colors.emplace_back(jsLinearGradient->GetGradient());
}

void JSGauge::ConvertResourceColor(const JsiRef<JsiValue>& itemParam, std::vector<NG::ColorStopArray>& colors)
{
    Color color;
    if (!ParseJsColor(itemParam, color)) {
        color = Color::BLACK;
    }
    NG::ColorStopArray colorStopArray;
    colorStopArray.emplace_back(std::make_pair(color, Dimension(0.0)));
    colors.emplace_back(colorStopArray);
}

void JSGauge::SetStrokeWidth(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE(" JSGauge::SetStrokeWidth::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    CalcDimension strokeWidth;
    if (!ParseJsDimensionVpNG(info[0], strokeWidth) || strokeWidth.Unit() == DimensionUnit::PERCENT) {
        strokeWidth = CalcDimension(0);
    }
    GaugeModel::GetInstance()->SetStrokeWidth(strokeWidth);
}

void JSGauge::SetLabelConfig(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsObject()) {
        LOGE("JSGauge::SetLabelTextConfig::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    auto labelText = paramObject->GetProperty("text");
    auto labelColor = paramObject->GetProperty("color");
    Color currentColor;
    ParseJsColor(labelColor, currentColor);
    if (labelText->IsString()) {
        GaugeModel::GetInstance()->SetLabelMarkedText(labelText->ToString());
    }
    if (ParseJsColor(labelColor, currentColor)) {
        GaugeModel::GetInstance()->SetMarkedTextColor(currentColor);
    }
}

void JSGauge::SetShadowOptions(const JSCallbackInfo& info)
{
    NG::GaugeShadowOptions shadowOptions;
    if (info[0]->IsNull()) {
        shadowOptions.isShadowVisible = false;
        GaugeModel::GetInstance()->SetShadowOptions(shadowOptions);
        return;
    }
    if (!info[0]->IsObject()) {
        return;
    }
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    JSRef<JSVal> jsRadius = paramObject->GetProperty("radius");
    JSRef<JSVal> jsOffsetX = paramObject->GetProperty("offsetX");
    JSRef<JSVal> jsOffsetY = paramObject->GetProperty("offsetY");

    double radius = 0.0;
    if (!ParseJsDouble(jsRadius, radius)) {
        radius = NG::DEFAULT_GAUGE_SHADOW_RADIUS;
    }

    if (NonPositive(radius)) {
        radius = NG::DEFAULT_GAUGE_SHADOW_RADIUS;
    }

    double offsetX = 0.0;
    if (!ParseJsDouble(jsOffsetX, offsetX)) {
        offsetX = NG::DEFAULT_GAUGE_SHADOW_OFFSETX;
    }

    double offsetY = 0.0;
    if (!ParseJsDouble(jsOffsetY, offsetY)) {
        offsetY = NG::DEFAULT_GAUGE_SHADOW_OFFSETY;
    }

    shadowOptions.radius = radius;
    shadowOptions.offsetX = offsetX;
    shadowOptions.offsetY = offsetY;

    GaugeModel::GetInstance()->SetShadowOptions(shadowOptions);
}

void JSGauge::SetDescription(const JSCallbackInfo& info)
{
    if (info[0]->IsNull()) {
        GaugeModel::GetInstance()->SetIsShowLimitValue(false);
        GaugeModel::GetInstance()->SetIsShowDescription(false);
        return;
    }
    if (info[0]->IsUndefined()) {
        GaugeModel::GetInstance()->SetIsShowLimitValue(true);
        GaugeModel::GetInstance()->SetIsShowDescription(false);
        return;
    }

    auto builderObject = JSRef<JSObject>::Cast(info[0])->GetProperty("builder");
    if (builderObject->IsFunction()) {
        GaugeModel::GetInstance()->SetIsShowLimitValue(false);
        GaugeModel::GetInstance()->SetIsShowDescription(true);
        ViewStackModel::GetInstance()->NewScope();
        JsFunction jsBuilderFunc(info.This(), JSRef<JSObject>::Cast(builderObject));
        ACE_SCORING_EVENT("Gauge.description.builder");
        jsBuilderFunc.Execute();
        auto customNode = ViewStackModel::GetInstance()->Finish();
        GaugeModel::GetInstance()->SetDescription(customNode);
    } else {
        GaugeModel::GetInstance()->SetIsShowLimitValue(true);
        GaugeModel::GetInstance()->SetIsShowDescription(false);
    }
}

void JSGauge::SetIndicator(const JSCallbackInfo& info)
{
    if (info[0]->IsNull()) {
        GaugeModel::GetInstance()->SetIsShowIndicator(false);
        return;
    }

    if (info[0]->IsUndefined()) {
        GaugeModel::GetInstance()->SetIsShowIndicator(true);
        return;
    }

    GaugeModel::GetInstance()->SetIsShowIndicator(true);
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    JSRef<JSVal> jsIcon = paramObject->GetProperty("icon");
    JSRef<JSVal> jsSpace = paramObject->GetProperty("space");

    std::string iconPath;
    if (ParseJsMedia(jsIcon, iconPath)) {
        std::string bundleName;
        std::string moduleName;
        GetJsMediaBundleInfo(jsIcon, bundleName, moduleName);
        GaugeModel::GetInstance()->SetIndicatorIconPath(iconPath, bundleName, moduleName);
    }

    CalcDimension space;
    if (ParseJsDimensionVpNG(jsSpace, space, false)) {
        if (space.IsNegative()) {
            space = NG::INDICATOR_DISTANCE_TO_TOP;
        }
        GaugeModel::GetInstance()->SetIndicatorSpace(space);
    }
}
} // namespace OHOS::Ace::Framework
