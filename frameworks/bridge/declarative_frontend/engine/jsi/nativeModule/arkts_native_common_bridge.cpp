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
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_common_bridge.h"
#include <utility>

#include "ark_native_engine.h"
#include "jsnapi_expo.h"

#include "base/memory/ace_type.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "bridge/declarative_frontend/engine/jsi/js_ui_index.h"
#include "bridge/declarative_frontend/engine/jsi/jsi_types.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_frame_node_bridge.h"
#include "bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "bridge/declarative_frontend/jsview/js_view_context.h"
#include "bridge/js_frontend/engine/jsi/ark_js_runtime.h"
#include "core/components/common/properties/blend_mode.h"
#include "core/components_ng/base/view_abstract_model_ng.h"
#include "core/components_ng/property/safe_area_insets.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/base/geometry/calc_dimension.h"
#include "frameworks/base/geometry/dimension.h"
#include "frameworks/bridge/declarative_frontend/engine/js_types.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/jsi_value_conversions.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_shape_abstract.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_abstract.h"
using namespace OHOS::Ace::Framework;

namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t COLOR_ALPHA_VALUE = 0xFF000000;
constexpr uint32_t ALIGNMENT_TOP_LEFT = 0;
constexpr float DEFAULT_PROGRESS_TOTAL = 100.0f;
constexpr int NUM_0 = 0;
constexpr int NUM_1 = 1;
constexpr int NUM_2 = 2;
constexpr int NUM_3 = 3;
constexpr int NUM_4 = 4;
constexpr int NUM_5 = 5;
constexpr int NUM_6 = 6;
constexpr int NUM_7 = 7;
constexpr int NUM_8 = 8;
constexpr int NUM_9 = 9;
constexpr int NUM_10 = 10;
constexpr int NUM_13 = 13;
constexpr int SIZE_OF_TWO = 2;
constexpr int SIZE_OF_THREE = 3;
constexpr int SIZE_OF_FOUR = 4;
constexpr int SIZE_OF_FIVE = 5;
constexpr int SIZE_OF_EIGHT = 8;
constexpr int32_t ALIGN_RULES_NUM = 6;
constexpr int32_t ALIGN_DIRECTION_DEFAULT = 2;
constexpr double FULL_DIMENSION = 100.0;
constexpr double HALF_DIMENSION = 50.0;
constexpr uint32_t DEFAULT_DURATION = 1000;
constexpr int64_t MICROSEC_TO_MILLISEC = 1000;
constexpr int32_t MAX_ALIGN_VALUE = 8;
constexpr int32_t BACKWARD_COMPAT_MAGIC_NUMBER_OFFSCREEN = 1000;
constexpr SharedTransitionEffectType DEFAULT_SHARED_EFFECT = SharedTransitionEffectType::SHARED_EFFECT_EXCHANGE;
constexpr int32_t DEFAULT_TAP_FINGER = 1;
constexpr int32_t DEFAULT_TAP_COUNT = 1;
constexpr int32_t DEFAULT_LONG_PRESS_FINGER = 1;
constexpr int32_t DEFAULT_LONG_PRESS_DURATION = 500;
constexpr int32_t DEFAULT_PINCH_FINGER = 2;
constexpr int32_t DEFAULT_MAX_PINCH_FINGER = 5;
constexpr double DEFAULT_PINCH_DISTANCE = 5.0;
constexpr int32_t DEFAULT_PAN_FINGER = 1;
constexpr int32_t DEFAULT_MAX_FINGERS = 10;
constexpr OHOS::Ace::Dimension DEFAULT_PAN_DISTANCE = 5.0_vp;
constexpr int32_t DEFAULT_SLIDE_FINGER = DEFAULT_PAN_FINGER;
constexpr double DEFAULT_SLIDE_SPEED = 100.0;
constexpr int32_t DEFAULT_ROTATION_FINGER = 2;
constexpr int32_t DEFAULT_MAX_ROTATION_FINGER = 5;
constexpr double DEFAULT_ROTATION_ANGLE = 1.0;
constexpr double DEFAULT_MAX_ROTATION_ANGLE = 360.0;
const std::string BLOOM_RADIUS_SYS_RES_NAME = "sys.float.ohos_id_point_light_bloom_radius";
const std::string BLOOM_COLOR_SYS_RES_NAME = "sys.color.ohos_id_point_light_bloom_color";
const std::string ILLUMINATED_BORDER_WIDTH_SYS_RES_NAME = "sys.float.ohos_id_point_light_illuminated_border_width";

BorderStyle ConvertBorderStyle(int32_t value)
{
    auto style = static_cast<BorderStyle>(value);
    if (style < BorderStyle::SOLID || style > BorderStyle::NONE) {
        style = BorderStyle::SOLID;
    }
    return style;
}

bool ParseJsDouble(const EcmaVM *vm, const Local<JSValueRef> &value, double &result)
{
    if (value->IsNumber()) {
        result = value->ToNumber(vm)->Value();
        return true;
    }
    if (value->IsString(vm)) {
        return StringUtils::StringToDouble(value->ToString(vm)->ToString(), result);
    }

    return false;
}

bool ParseJsInt32(const EcmaVM *vm, const Local<JSValueRef> &value, int32_t &result)
{
    if (value->IsNumber()) {
        result = value->Int32Value(vm);
        return true;
    }
    if (value->IsString(vm)) {
        result = StringUtils::StringToInt(value->ToString(vm)->ToString());
        return true;
    }

    return false;
}

void ParseJsAngle(const EcmaVM *vm, const Local<JSValueRef> &value, std::optional<float> &angle)
{
    if (value->IsNumber()) {
        angle = static_cast<float>(value->ToNumber(vm)->Value());
        return;
    }
    if (value->IsString(vm)) {
        angle = static_cast<float>(StringUtils::StringToDegree(value->ToString(vm)->ToString()));
        return;
    }
    return;
}

void ParseGradientAngle(const EcmaVM *vm, const Local<JSValueRef> &value, std::vector<ArkUIInt32orFloat32> &values)
{
    std::optional<float> degree;
    ParseJsAngle(vm, value, degree);
    auto angleHasValue = degree.has_value();
    auto angleValue = angleHasValue ? degree.value() : 0.0f;
    degree.reset();
    values.push_back({.i32 = static_cast<ArkUI_Int32>(angleHasValue)});
    values.push_back({.f32 = static_cast<ArkUI_Float32>(angleValue)});
}

void ParseGradientColorStops(const EcmaVM *vm, const Local<JSValueRef> &value, std::vector<ArkUIInt32orFloat32> &colors)
{
    if (!value->IsArray(vm)) {
        return;
    }
    auto array = panda::Local<panda::ArrayRef>(value);
    auto length = array->Length(vm);
    for (uint32_t index = 0; index < length; index++) {
        auto item = panda::ArrayRef::GetValueAt(vm, array, index);
        if (!item->IsArray(vm)) {
            continue;
        }
        auto itemArray = panda::Local<panda::ArrayRef>(item);
        auto itemLength = itemArray->Length(vm);
        if (itemLength < NUM_1) {
            continue;
        }
        Color color;
        auto colorParams = panda::ArrayRef::GetValueAt(vm, itemArray, NUM_0);
        if (!ArkTSUtils::ParseJsColorAlpha(vm, colorParams, color)) {
            continue;
        }
        bool hasDimension = false;
        double dimension = 0.0;
        if (itemLength > NUM_1) {
            auto stopDimension = panda::ArrayRef::GetValueAt(vm, itemArray, NUM_1);
            if (ArkTSUtils::ParseJsDouble(vm, stopDimension, dimension)) {
                hasDimension = true;
            }
        }
        colors.push_back({.u32 = static_cast<ArkUI_Uint32>(color.GetValue())});
        colors.push_back({.i32 = static_cast<ArkUI_Int32>(hasDimension)});
        colors.push_back({.f32 = static_cast<ArkUI_Float32>(dimension)});
    }
}

bool ParseJsShadowColorStrategy(const EcmaVM *vm, const Local<JSValueRef> &value, ShadowColorStrategy& strategy)
{
    if (value->IsString(vm)) {
        std::string colorStr = value->ToString(vm)->ToString();
        if (colorStr.compare("average") == 0) {
            strategy = ShadowColorStrategy::AVERAGE;
            return true;
        } else if (colorStr.compare("primary") == 0) {
            strategy = ShadowColorStrategy::PRIMARY;
            return true;
        }
    }
    return false;
}

bool ParseJsShadowDimension(const EcmaVM *vm, const Local<JSValueRef> &value, CalcDimension& dimension)
{
    if (ArkTSUtils::ParseJsResource(vm, value, dimension)) {
        return true;
    } else {
        if (ArkTSUtils::ParseJsDimensionVp(vm, value, dimension)) {
            return true;
        }
    }
    return false;
}

bool ParseJsShadowColor(const EcmaVM *vm, const Local<JSValueRef> &colorArg,
    int32_t& type, uint32_t& colorValue)
{
    Color color;
    ShadowColorStrategy shadowColorStrategy;
    if (ParseJsShadowColorStrategy(vm, colorArg, shadowColorStrategy)) {
        type = 1; // 1: has shadowColorStrategy
        colorValue = static_cast<uint32_t>(shadowColorStrategy);
        return true;
    } else if (ArkTSUtils::ParseJsColorAlpha(vm, colorArg, color)) {
        type = 2; // 2: has shadowColor
        colorValue = color.GetValue();
        return true;
    }
    return false;
}

bool ParseCalcDimensions(ArkUIRuntimeCallInfo* runtimeCallInfo, uint32_t offset, uint32_t count,
    std::vector<std::optional<CalcDimension>>& results, const CalcDimension& defValue)
{
    auto end = offset + count;
    auto argsNumber = runtimeCallInfo->GetArgsNumber();
    if (end > argsNumber) {
        return false;
    }
    bool hasValue = false;
    EcmaVM* vm = runtimeCallInfo->GetVM();
    for (uint32_t index = offset; index < end; index++) {
        auto arg = runtimeCallInfo->GetCallArgRef(index);
        std::optional<CalcDimension> optCalcDimension;
        CalcDimension dimension(defValue);
        if (ArkTSUtils::ParseJsDimensionVp(vm, arg, dimension, false)) {
            optCalcDimension = dimension;
            hasValue = true;
        }
        results.push_back(optCalcDimension);
    }
    return hasValue;
}

void ResetCalcDimensions(std::vector<std::optional<CalcDimension>>& optDimensions)
{
    for (uint32_t index = 0; index < optDimensions.size(); index++) {
        auto optDimension = optDimensions[index];
        optDimension.reset();
    }
}

void PushDimensionsToVector(std::vector<ArkUIStringAndFloat>& results,
    const std::vector<std::optional<CalcDimension>>& optDimensions)
{
    for (uint32_t index = 0; index < optDimensions.size(); index++) {
        auto optDimension = optDimensions[index];
        auto hasValue = optDimension.has_value();
        DimensionUnit unit = DimensionUnit::PX;
        ArkUIStringAndFloat value = { 0.0, nullptr };
        if (hasValue) {
            unit = optDimension.value().Unit();
            if (unit == DimensionUnit::CALC) {
                value.valueStr = optDimension.value().CalcValue().c_str();
            } else {
                value.value = optDimension.value().Value();
            }
        }
        results.push_back(ArkUIStringAndFloat { static_cast<double>(hasValue), nullptr });
        results.push_back(value);
        results.push_back(ArkUIStringAndFloat { static_cast<double>(unit), nullptr });
    }
}

void ParseBorderImageSlice(ArkUIRuntimeCallInfo* runtimeCallInfo, uint32_t& offset,
    std::vector<std::optional<CalcDimension>>& optDimensions, uint8_t& bitsets)
{
    if (ParseCalcDimensions(runtimeCallInfo, offset, NUM_4, optDimensions, CalcDimension(0.0))) {
        bitsets |= BorderImage::SLICE_BIT;
    }
    offset += NUM_4;
}

void ParseBorderImageWidth(ArkUIRuntimeCallInfo* runtimeCallInfo, uint32_t& offset,
    std::vector<std::optional<CalcDimension>>& optDimensions, uint8_t& bitsets)
{
    if (ParseCalcDimensions(runtimeCallInfo, offset, NUM_4, optDimensions, CalcDimension(0.0))) {
        bitsets |= BorderImage::WIDTH_BIT;
    }
    offset += NUM_4;
}

void ParseBorderImageOutset(ArkUIRuntimeCallInfo* runtimeCallInfo, uint32_t& offset,
    std::vector<std::optional<CalcDimension>>& optDimensions, uint8_t& bitsets)
{
    if (ParseCalcDimensions(runtimeCallInfo, offset, NUM_4, optDimensions, CalcDimension(0.0))) {
        bitsets |= BorderImage::OUTSET_BIT;
    }
    offset += NUM_4;
}

bool ParseBorderImageRepeat(ArkUIRuntimeCallInfo* runtimeCallInfo, uint32_t& offset,
    std::vector<ArkUIStringAndFloat>& options, uint8_t& bitsets)
{
    auto argsNumber = runtimeCallInfo->GetArgsNumber();
    if ((offset + NUM_1) > argsNumber) {
        return false;
    }
    auto vm = runtimeCallInfo->GetVM();
    auto repeatArg = runtimeCallInfo->GetCallArgRef(offset);
    auto repeatHasValue = repeatArg->IsString(vm);
    auto repeatValue = BorderImageRepeat::STRETCH;
    if (repeatHasValue) {
        auto repeatStr = repeatArg->ToString(vm)->ToString();
        if (repeatStr == "Repeat") {
            repeatValue = BorderImageRepeat::REPEAT;
        } else if (repeatStr == "Round") {
            repeatValue = BorderImageRepeat::ROUND;
        } else if (repeatStr == "Space") {
            repeatValue = BorderImageRepeat::SPACE;
        } else {
            repeatValue = BorderImageRepeat::STRETCH;
        }
    }
    options.push_back(ArkUIStringAndFloat { static_cast<double>(repeatHasValue), nullptr });
    options.push_back(ArkUIStringAndFloat { static_cast<double>(repeatValue), nullptr });
    if (repeatHasValue) {
        bitsets |= BorderImage::REPEAT_BIT;
    }
    offset += NUM_1;
    return true;
}

bool ParseBorderImageFill(ArkUIRuntimeCallInfo* runtimeCallInfo, uint32_t& offset,
    std::vector<ArkUIStringAndFloat>& options)
{
    auto argsNumber = runtimeCallInfo->GetArgsNumber();
    if ((offset + NUM_1) > argsNumber) {
        return false;
    }
    auto fillArg = runtimeCallInfo->GetCallArgRef(offset);
    auto hasValue = fillArg->IsBoolean();
    auto fill = (hasValue) ? fillArg->BooleaValue() : false;
    options.push_back(ArkUIStringAndFloat {static_cast<double>(hasValue), nullptr });
    options.push_back(ArkUIStringAndFloat {static_cast<double>(fill), nullptr });
    offset += NUM_1;
    return true;
}

bool IsArgsUndefined(ArkUIRuntimeCallInfo* runtimeCallInfo, uint32_t offset, uint32_t count)
{
    auto argsNumber = runtimeCallInfo->GetArgsNumber();
    auto end = offset + count;
    end = (end > argsNumber) ? argsNumber : end;
    for (uint32_t index = offset; index < end; index++) {
        auto jsArg = runtimeCallInfo->GetCallArgRef(index);
        if (jsArg->IsUndefined()) {
            continue;
        }
        return false;
    }
    return true;
}

void ParseBorderImageLinearGradient(ArkUINodeHandle node,
    ArkUIRuntimeCallInfo* runtimeCallInfo, uint32_t offset,
    uint8_t& bitsets)
{
    auto argsNumber = runtimeCallInfo->GetArgsNumber();
    if ((offset + NUM_4) > argsNumber) {
        return;
    }
    auto angleArg = runtimeCallInfo->GetCallArgRef(offset);
    auto directionArg = runtimeCallInfo->GetCallArgRef(offset + NUM_1);
    auto colorsArg = runtimeCallInfo->GetCallArgRef(offset + NUM_2);
    auto repeatingArg = runtimeCallInfo->GetCallArgRef(offset + NUM_3);
    if (angleArg->IsUndefined() && directionArg->IsUndefined() &&
        colorsArg->IsUndefined() && repeatingArg->IsUndefined()) {
        return;
    }
    auto vm = runtimeCallInfo->GetVM();
    std::vector<ArkUIInt32orFloat32> options;
    ParseGradientAngle(vm, angleArg, options);
    int32_t direction = static_cast<int32_t>(GradientDirection::NONE);
    ParseJsInt32(vm, directionArg, direction);
    options.push_back({.i32 = static_cast<ArkUI_Int32>(direction)});

    std::vector<ArkUIInt32orFloat32> colors;
    ParseGradientColorStops(vm, colorsArg, colors);
    auto repeating = repeatingArg->IsBoolean() ? repeatingArg->BooleaValue() : false;
    options.push_back({.i32 = static_cast<ArkUI_Int32>(repeating)});
    GetArkUINodeModifiers()->getCommonModifier()->setBorderImageGradient(node,
        options.data(), options.size(), colors.data(), colors.size());
}

bool ParseBorderImageSource(ArkUIRuntimeCallInfo* runtimeCallInfo, uint32_t& offset,
    ArkUINodeHandle node, std::string& src, uint8_t& bitsets)
{
    auto argsNumber = runtimeCallInfo->GetArgsNumber();
    if ((offset + NUM_5) > argsNumber) {
        return false;
    }
    if (IsArgsUndefined(runtimeCallInfo, offset, NUM_5)) {
        return false;
    }
    auto sourceArg = runtimeCallInfo->GetCallArgRef(offset); // use 1 args
    offset += NUM_1;
    auto vm = runtimeCallInfo->GetVM();
    if (sourceArg->IsString(vm)) {
        src = sourceArg->ToString(vm)->ToString();
        bitsets |= BorderImage::SOURCE_BIT;
    } else {
        if (ArkTSUtils::ParseJsMedia(vm, sourceArg, src)) {
            bitsets |= BorderImage::SOURCE_BIT;
        } else {
            ParseBorderImageLinearGradient(node, runtimeCallInfo, offset, bitsets);
        }
    }
    offset += NUM_4; // skip 4 args
    return true;
}

RefPtr<NG::ChainedTransitionEffect> ParseChainedMoveTransition(
    const Framework::JSRef<Framework::JSVal>& effectOption, const JSExecutionContext& context)
{
    int32_t edge = 0;
    if (JSViewAbstract::ParseJsInt32(effectOption, edge)) {
        if (edge < static_cast<int32_t>(NG::TransitionEdge::TOP) ||
            edge > static_cast<int32_t>(NG::TransitionEdge::END)) {
            edge = static_cast<int32_t>(NG::TransitionEdge::START);
        }
        return AceType::MakeRefPtr<NG::ChainedMoveEffect>(static_cast<NG::TransitionEdge>(edge));
    }
    return nullptr;
}

RefPtr<NG::ChainedTransitionEffect> ParseChainedOpacityTransition(
    const Framework::JSRef<Framework::JSVal>& effectOption, const JSExecutionContext& context)
{
    double opacity = 1.0;
    if (Framework::JSViewAbstract::ParseJsDouble(effectOption, opacity)) {
        if ((LessNotEqual(opacity, 0.0)) || opacity > 1.0) {
            opacity = 1.0;
        }
        return AceType::MakeRefPtr<NG::ChainedOpacityEffect>(opacity);
    }
    return nullptr;
}

void ParseJsTranslate(const Framework::JSRef<Framework::JSVal>& jsValue, CalcDimension& translateX,
    CalcDimension& translateY, CalcDimension& translateZ)
{
    if (!jsValue->IsObject()) {
        return;
    }
    Framework::JSRef<Framework::JSObject> jsObj = Framework::JSRef<Framework::JSObject>::Cast(jsValue);
    Framework::JSViewAbstract::ParseJsDimensionVp(jsObj->GetProperty("x"), translateX);
    Framework::JSViewAbstract::ParseJsDimensionVp(jsObj->GetProperty("y"), translateY);
    Framework::JSViewAbstract::ParseJsDimensionVp(jsObj->GetProperty("z"), translateZ);
}

uint32_t ParseStrToUint(std::string safeAreaTypeStr)
{
    uint32_t uintType = NG::SAFE_AREA_TYPE_NONE;
    std::string delimiter = "|";
    std::string type;
    size_t pos = 0;
    while ((pos = safeAreaTypeStr.find(delimiter)) != std::string::npos) {
        type = safeAreaTypeStr.substr(0, pos);
        uintType |= (1 << StringUtils::StringToUint(type));
        safeAreaTypeStr.erase(0, pos + delimiter.length());
    }
    uintType |= (1 << StringUtils::StringToUint(safeAreaTypeStr));
    return uintType;
}

RefPtr<NG::ChainedTransitionEffect> ParseChainedTranslateTransition(
    const Framework::JSRef<Framework::JSVal>& effectOption, const JSExecutionContext& context)
{
    if (effectOption->IsObject()) {
        // default: x, y, z (0.0, 0.0, 0.0)
        NG::TranslateOptions translate;
        ParseJsTranslate(effectOption, translate.x, translate.y, translate.z);
        return AceType::MakeRefPtr<NG::ChainedTranslateEffect>(translate);
    }
    return nullptr;
}

void GetDefaultRotateVector(double& dx, double& dy, double& dz)
{
    dx = 0.0;
    dy = 0.0;
    dz = 0.0;
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_NINE)) {
        dz = 1.0;
    }
}

void ParseJsRotate(
    const Framework::JSRef<Framework::JSVal>& jsValue, NG::RotateOptions& rotate, std::optional<float>& angle)
{
    if (!jsValue->IsObject()) {
        return;
    }
    // default: dx, dy, dz (0.0, 0.0, 0.0)
    double dxVal = 0.0;
    double dyVal = 0.0;
    double dzVal = 0.0;
    Framework::JSRef<Framework::JSObject> jsObj = Framework::JSRef<Framework::JSObject>::Cast(jsValue);
    if (!jsObj->HasProperty("x") && !jsObj->HasProperty("y") && !jsObj->HasProperty("z")) {
        GetDefaultRotateVector(dxVal, dyVal, dzVal);
    } else {
        Framework::JSViewAbstract::ParseJsDouble(jsObj->GetProperty("x"), dxVal);
        Framework::JSViewAbstract::ParseJsDouble(jsObj->GetProperty("y"), dyVal);
        Framework::JSViewAbstract::ParseJsDouble(jsObj->GetProperty("z"), dzVal);
    }
    rotate.xDirection = static_cast<float>(dxVal);
    rotate.yDirection = static_cast<float>(dyVal);
    rotate.zDirection = static_cast<float>(dzVal);
    // if specify centerX
    if (!Framework::JSViewAbstract::ParseJsDimensionVp(jsObj->GetProperty("centerX"), rotate.centerX)) {
        rotate.centerX = Dimension(0.5f, DimensionUnit::PERCENT);
    }
    // if specify centerY
    if (!Framework::JSViewAbstract::ParseJsDimensionVp(jsObj->GetProperty("centerY"), rotate.centerY)) {
        rotate.centerY = Dimension(0.5f, DimensionUnit::PERCENT);
    }
    // if specify centerZ
    if (!Framework::JSViewAbstract::ParseJsDimensionVp(jsObj->GetProperty("centerZ"), rotate.centerZ)) {
        rotate.centerZ = Dimension(0.5f, DimensionUnit::PERCENT);
    }
    // if specify angle
    Framework::JSViewAbstract::GetJsAngle(static_cast<int32_t>(ArkUIIndex::ANGLE), jsObj, angle);
    rotate.perspective = 0.0f;
    Framework::JSViewAbstract::GetJsPerspective(static_cast<int32_t>(ArkUIIndex::PERSPECTIVE), jsObj,
        rotate.perspective);
}

RefPtr<NG::ChainedTransitionEffect> ParseChainedRotateTransition(
    const Framework::JSRef<Framework::JSVal>& effectOption, const JSExecutionContext& context)
{
    RefPtr<NG::ChainedTransitionEffect> effect;
    if (effectOption->IsObject()) {
        NG::RotateOptions rotate(0.0f, 0.0f, 0.0f, 0.0f, 0.5_pct, 0.5_pct);
        std::optional<float> angle;
        ParseJsRotate(effectOption, rotate, angle);
        if (angle.has_value()) {
            rotate.angle = angle.value();
            return AceType::MakeRefPtr<NG::ChainedRotateEffect>(rotate);
        }
    }
    return nullptr;
}

void ParseJsScale(const Framework::JSRef<Framework::JSVal>& jsValue, float& scaleX, float& scaleY, float& scaleZ,
    CalcDimension& centerX, CalcDimension& centerY)
{
    double xVal = 1.0;
    double yVal = 1.0;
    double zVal = 1.0;
    if (!jsValue->IsObject()) {
        scaleX = static_cast<float>(xVal);
        scaleY = static_cast<float>(yVal);
        scaleZ = static_cast<float>(zVal);
        CalcDimension length;
        centerX = length;
        centerY = length;
        return;
    }
    Framework::JSRef<Framework::JSObject> jsObj = Framework::JSRef<Framework::JSObject>::Cast(jsValue);
    Framework::JSViewAbstract::ParseJsDouble(jsObj->GetProperty("x"), xVal);
    Framework::JSViewAbstract::ParseJsDouble(jsObj->GetProperty("y"), yVal);
    Framework::JSViewAbstract::ParseJsDouble(jsObj->GetProperty("z"), zVal);
    scaleX = static_cast<float>(xVal);
    scaleY = static_cast<float>(yVal);
    scaleZ = static_cast<float>(zVal);
    // if specify centerX
    Framework::JSViewAbstract::ParseJsDimensionVp(jsObj->GetProperty("centerX"), centerX);
    // if specify centerY
    Framework::JSViewAbstract::ParseJsDimensionVp(jsObj->GetProperty("centerY"), centerY);
}

RefPtr<NG::ChainedTransitionEffect> ParseChainedScaleTransition(
    const Framework::JSRef<Framework::JSVal>& effectOption, const JSExecutionContext& context)
{
    if (effectOption->IsObject()) {
        // default: x, y, z (1.0, 1.0, 1.0), centerX, centerY 50% 50%;
        NG::ScaleOptions scale(1.0f, 1.0f, 1.0f, 0.5_pct, 0.5_pct);
        ParseJsScale(effectOption, scale.xScale, scale.yScale, scale.zScale, scale.centerX, scale.centerY);
        return AceType::MakeRefPtr<NG::ChainedScaleEffect>(scale);
    }
    return nullptr;
}

RefPtr<NG::ChainedTransitionEffect> ParseChainedTransition(
    const JSRef<JSObject>& object, const JSExecutionContext& context);

RefPtr<NG::ChainedTransitionEffect> ParseChainedAsymmetricTransition(
    const Framework::JSRef<Framework::JSVal>& effectOption, const JSExecutionContext& context)
{
    if (effectOption->IsObject()) {
        auto effectObj = Framework::JSRef<Framework::JSObject>::Cast(effectOption);
        auto appearJsVal = effectObj->GetProperty("appear");
        auto disappearJsVal = effectObj->GetProperty("disappear");
        RefPtr<NG::ChainedTransitionEffect> appearEffect;
        RefPtr<NG::ChainedTransitionEffect> disappearEffect;
        if (appearJsVal->IsObject()) {
            auto appearObj = Framework::JSRef<Framework::JSObject>::Cast(appearJsVal);
            appearEffect = ParseChainedTransition(appearObj, context);
        }
        if (disappearJsVal->IsObject()) {
            auto disappearObj = Framework::JSRef<Framework::JSObject>::Cast(disappearJsVal);
            disappearEffect = ParseChainedTransition(disappearObj, context);
        }
        return AceType::MakeRefPtr<NG::ChainedAsymmetricEffect>(appearEffect, disappearEffect);
    }
    return nullptr;
}

int64_t GetFormAnimationTimeInterval(const RefPtr<PipelineBase>& pipelineContext)
{
    CHECK_NULL_RETURN(pipelineContext, 0);
    return (GetMicroTickCount() - pipelineContext->GetFormAnimationStartTime()) / MICROSEC_TO_MILLISEC;
}

using ChainedTransitionEffectCreator = RefPtr<NG::ChainedTransitionEffect> (*)(
    const Framework::JSRef<Framework::JSVal>&, const JSExecutionContext&);

void GetAnimationOptionResult(shared_ptr<AnimationOption>& animationOptionResult,
    const JSRef<JSObject>& propAnimationOption, const RefPtr<PipelineBase>& pipelineContext,
    const JSExecutionContext& context)
{
    // The maximum of the form-animation-playback duration value is 1000 ms.
    if (pipelineContext->IsFormRender() && pipelineContext->IsFormAnimation()) {
        auto formAnimationTimeInterval = GetFormAnimationTimeInterval(pipelineContext);
        // If the duration exceeds 1000ms, init it to 0 ms.
        if (formAnimationTimeInterval > DEFAULT_DURATION) {
            animationOptionResult->SetDuration(0);
        } else if (animationOptionResult->GetDuration() > (DEFAULT_DURATION - formAnimationTimeInterval)) {
            // If remaining time is less than 1000ms, check for update duration.
            animationOptionResult->SetDuration(DEFAULT_DURATION - formAnimationTimeInterval);
            TAG_LOGI(AceLogTag::ACE_FORM, "[Form animation]  Form Transition SetDuration: %{public}lld ms",
                static_cast<long long>(DEFAULT_DURATION - formAnimationTimeInterval));
        }
    }
    auto animationOptionObj = Framework::JSRef<Framework::JSObject>::Cast(propAnimationOption);
    Framework::JSRef<Framework::JSVal> onFinish = animationOptionObj->GetProperty("onFinish");
    if (onFinish->IsFunction()) {
        RefPtr<JsFunction> jsFunc =
            AceType::MakeRefPtr<JsFunction>(Framework::JSRef<Framework::JSObject>(), JSRef<JSFunc>::Cast(onFinish));
        std::function<void()> onFinishEvent = [execCtx = context, func = std::move(jsFunc),
                                                  id = Container::CurrentId()]() {
            ContainerScope scope(id);
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            func->Execute();
        };
        animationOptionResult->SetOnFinishEvent(onFinishEvent);
    }
}

RefPtr<NG::ChainedTransitionEffect> ParseChainedTransition(
    const Framework::JSRef<Framework::JSObject>& object, const JSExecutionContext& context)
{
    auto propType = object->GetProperty("type_");
    if (!propType->IsString()) {
        return nullptr;
    }
    std::string type = propType->ToString();
    auto propEffectOption = object->GetProperty("effect_");
    auto propAnimationOption = object->GetProperty("animation_");
    auto propSuccessor = object->GetProperty("successor_");
    static const LinearMapNode<ChainedTransitionEffectCreator> creatorMap[] = {
        { "asymmetric", ParseChainedAsymmetricTransition },
        { "identity",
            [](const Framework::JSRef<Framework::JSVal>& effectOption, const JSExecutionContext& context)
                -> RefPtr<NG::ChainedTransitionEffect> { return AceType::MakeRefPtr<NG::ChainedIdentityEffect>(); } },
        { "move", ParseChainedMoveTransition },
        { "opacity", ParseChainedOpacityTransition },
        { "rotate", ParseChainedRotateTransition },
        { "scale", ParseChainedScaleTransition },
        { "slideSwitch",
            [](const Framework::JSRef<Framework::JSVal>& effectOption,
                const JSExecutionContext& context) -> RefPtr<NG::ChainedTransitionEffect> {
                return AceType::MakeRefPtr<NG::ChainedSlideSwitchEffect>();
            } },
        { "translate", ParseChainedTranslateTransition },
    };
    int64_t index = BinarySearchFindIndex(creatorMap, ArraySize(creatorMap), type.c_str());
    if (index < 0) {
        return nullptr;
    }
    RefPtr<NG::ChainedTransitionEffect> result = creatorMap[index].value(propEffectOption, context);
    if (!result) {
        return nullptr;
    }
    if (propAnimationOption->IsObject()) {
        auto container = Container::Current();
        CHECK_NULL_RETURN(container, nullptr);
        auto pipelineContext = container->GetPipelineContext();
        CHECK_NULL_RETURN(pipelineContext, nullptr);
        auto animationOptionResult = std::make_shared<AnimationOption>(
            JSViewContext::CreateAnimation(propAnimationOption, pipelineContext->IsFormRender()));
        GetAnimationOptionResult(animationOptionResult, propAnimationOption, pipelineContext, context);
        result->SetAnimationOption(animationOptionResult);
    }
    if (propSuccessor->IsObject()) {
        result->SetNext(ParseChainedTransition(Framework::JSRef<Framework::JSObject>::Cast(propSuccessor), context));
    }
    return result;
}

NG::TransitionOptions ParseJsTransition(const Framework::JSRef<Framework::JSVal>& transitionArgs)
{
    NG::TransitionOptions transitionOption;
    if (!transitionArgs->IsObject()) {
        return transitionOption;
    }
    Framework::JSRef<Framework::JSObject> jsObj = Framework::JSRef<Framework::JSObject>::Cast(transitionArgs);
    bool hasEffect = false;
    transitionOption.Type = ParseTransitionType(jsObj->GetPropertyValue<std::string>("type", "All"));
    if (jsObj->HasProperty("opacity")) {
        double opacity = 1.0;
        Framework::JSViewAbstract::ParseJsDouble(jsObj->GetProperty("opacity"), opacity);
        opacity = std::clamp(opacity, 0.0, 1.0);
        transitionOption.UpdateOpacity(static_cast<float>(opacity));
        hasEffect = true;
    }
    if (jsObj->HasProperty("translate")) {
        // default: x, y, z (0.0, 0.0, 0.0)
        NG::TranslateOptions translate;
        ParseJsTranslate(jsObj->GetProperty("translate"), translate.x, translate.y, translate.z);
        transitionOption.UpdateTranslate(translate);
        hasEffect = true;
    }
    if (jsObj->HasProperty("scale")) {
        // default: x, y, z (1.0, 1.0, 1.0), centerX, centerY 50% 50%;
        NG::ScaleOptions scale(1.0f, 1.0f, 1.0f, 0.5_pct, 0.5_pct);
        ParseJsScale(
            jsObj->GetProperty("scale"), scale.xScale, scale.yScale, scale.zScale, scale.centerX, scale.centerY);
        transitionOption.UpdateScale(scale);
        hasEffect = true;
    }
    if (jsObj->HasProperty("rotate")) {
        // default: dx, dy, dz (0.0, 0.0, 0.0), angle 0, centerX, centerY 50% 50%;
        NG::RotateOptions rotate(0.0f, 0.0f, 0.0f, 0.0f, 0.5_pct, 0.5_pct);
        std::optional<float> angle;
        ParseJsRotate(jsObj->GetProperty("rotate"), rotate, angle);
        if (angle.has_value()) {
            rotate.angle = angle.value();
            transitionOption.UpdateRotate(rotate);
            hasEffect = true;
        }
    }
    if (!hasEffect) {
        // default transition
        transitionOption = NG::TransitionOptions::GetDefaultTransition(transitionOption.Type);
    }
    return transitionOption;
}

bool ParseMotionPath(const Framework::JSRef<Framework::JSVal>& jsValue, MotionPathOption& option)
{
    if (!jsValue->IsObject()) {
        return false;
    }

    Framework::JSRef<Framework::JSObject> jsObj = Framework::JSRef<Framework::JSObject>::Cast(jsValue);
    auto path = jsObj->GetPropertyValue<std::string>("path", "");
    if (path.empty()) {
        return false;
    }
    option.SetPath(path);
    double from = 0.0;
    double to = 1.0;
    Framework::JSViewAbstract::ParseJsDouble(jsObj->GetProperty("from"), from);
    Framework::JSViewAbstract::ParseJsDouble(jsObj->GetProperty("to"), to);
    if (GreatNotEqual(from, 1.0) || LessNotEqual(from, 0.0)) {
        from = 0.0;
    }
    if (GreatNotEqual(to, 1.0) || LessNotEqual(to, 0.0)) {
        to = 1.0;
    } else if (to < from) {
        to = from;
    }
    option.SetBegin(static_cast<float>(from));
    option.SetEnd(static_cast<float>(to));
    option.SetRotate(jsObj->GetPropertyValue<bool>("rotatable", false));
    return true;
}

bool ParseJsDoublePair(const EcmaVM *vm, const Local<JSValueRef> &value, ArkUI_Float32 &first, ArkUI_Float32 &second)
{
    if (!value->IsArray(vm)) {
        return false;
    }
    auto array = panda::Local<panda::ArrayRef>(value);
    if (array->Length(vm) != NUM_2) {
        return false;
    }
    auto firstArg = panda::ArrayRef::GetValueAt(vm, array, NUM_0);
    if (!firstArg->IsNumber()) {
        return false;
    }
    auto secondArg = panda::ArrayRef::GetValueAt(vm, array, NUM_1);
    if (!secondArg->IsNumber()) {
        return false;
    }
    first = static_cast<ArkUI_Float32>(firstArg->ToNumber(vm)->Value());
    second = static_cast<ArkUI_Float32>(secondArg->ToNumber(vm)->Value());
    return true;
}

void ParseGradientCenter(const EcmaVM* vm, const Local<JSValueRef>& value, std::vector<ArkUIInt32orFloat32>& values)
{
    bool hasValueX = false;
    bool hasValueY = false;
    CalcDimension valueX;
    CalcDimension valueY;
    if (value->IsArray(vm)) {
        auto array = panda::Local<panda::ArrayRef>(value);
        auto length = array->Length(vm);
        if (length == NUM_2) {
            hasValueX =
                ArkTSUtils::ParseJsDimensionVp(vm, panda::ArrayRef::GetValueAt(vm, array, NUM_0), valueX, false);
            hasValueY =
                ArkTSUtils::ParseJsDimensionVp(vm, panda::ArrayRef::GetValueAt(vm, array, NUM_1), valueY, false);
        }
    }
    values.push_back({.i32 = static_cast<ArkUI_Int32>(hasValueX)});
    values.push_back({.f32 = static_cast<ArkUI_Float32>(valueX.Value())});
    values.push_back({.i32 = static_cast<ArkUI_Int32>(valueX.Unit())});
    values.push_back({.i32 = static_cast<ArkUI_Int32>(hasValueY)});
    values.push_back({.f32 = static_cast<ArkUI_Float32>(valueY.Value())});
    values.push_back({.i32 = static_cast<ArkUI_Int32>(valueY.Unit())});
}

void PushOuterBorderDimensionVector(const std::optional<CalcDimension>& valueDim, std::vector<ArkUI_Float32> &options)
{
    options.push_back(static_cast<ArkUI_Float32>(valueDim.has_value()));
    if (valueDim.has_value()) {
        options.push_back(static_cast<ArkUI_Float32>(valueDim.value().Value()));
        options.push_back(static_cast<ArkUI_Float32>(valueDim.value().Unit()));
    } else {
        options.push_back(0);
        options.push_back(0);
    }
}

void ParseOuterBorderWidth(ArkUIRuntimeCallInfo *runtimeCallInfo, EcmaVM *vm, std::vector<ArkUI_Float32> &values)
{
    Local<JSValueRef> leftArgs = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> rightArgs = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> topArgs = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> bottomArgs = runtimeCallInfo->GetCallArgRef(NUM_4);
    std::optional<CalcDimension> leftDim;
    std::optional<CalcDimension> rightDim;
    std::optional<CalcDimension> topDim;
    std::optional<CalcDimension> bottomDim;

    ArkTSUtils::ParseOuterBorder(vm, leftArgs, leftDim);
    ArkTSUtils::ParseOuterBorder(vm, rightArgs, rightDim);
    ArkTSUtils::ParseOuterBorder(vm, topArgs, topDim);
    ArkTSUtils::ParseOuterBorder(vm, bottomArgs, bottomDim);

    PushOuterBorderDimensionVector(leftDim, values);
    PushOuterBorderDimensionVector(rightDim, values);
    PushOuterBorderDimensionVector(topDim, values);
    PushOuterBorderDimensionVector(bottomDim, values);
}

void PushOuterBorderColorVector(const std::optional<Color>& valueColor, std::vector<uint32_t> &options)
{
    options.push_back(static_cast<uint32_t>(valueColor.has_value()));
    if (valueColor.has_value()) {
        options.push_back(static_cast<uint32_t>(valueColor.value().GetValue()));
    } else {
        options.push_back(0);
    }
}
void ParseOuterBorderColor(
    ArkUIRuntimeCallInfo* runtimeCallInfo, EcmaVM* vm, std::vector<uint32_t>& values, int32_t argsIndex)
{
    Local<JSValueRef> leftArg = runtimeCallInfo->GetCallArgRef(argsIndex);
    Local<JSValueRef> rightArg = runtimeCallInfo->GetCallArgRef(argsIndex + NUM_1);
    Local<JSValueRef> topArg = runtimeCallInfo->GetCallArgRef(argsIndex + NUM_2);
    Local<JSValueRef> bottomArg = runtimeCallInfo->GetCallArgRef(argsIndex + NUM_3);

    std::optional<Color> leftColor;
    std::optional<Color> rightColor;
    std::optional<Color> topColor;
    std::optional<Color> bottomColor;

    Color left;
    if (!leftArg->IsUndefined() && ArkTSUtils::ParseJsColorAlpha(vm, leftArg, left)) {
        leftColor = left;
    }
    Color right;
    if (!rightArg->IsUndefined() && ArkTSUtils::ParseJsColorAlpha(vm, rightArg, right)) {
        rightColor = right;
    }
    Color top;
    if (!topArg->IsUndefined() && ArkTSUtils::ParseJsColorAlpha(vm, topArg, top)) {
        topColor = top;
    }
    Color bottom;
    if (!bottomArg->IsUndefined() && ArkTSUtils::ParseJsColorAlpha(vm, bottomArg, bottom)) {
        bottomColor = bottom;
    }

    PushOuterBorderColorVector(leftColor, values);
    PushOuterBorderColorVector(rightColor, values);
    PushOuterBorderColorVector(topColor, values);
    PushOuterBorderColorVector(bottomColor, values);
}

void ParseOuterBorderRadius(
    ArkUIRuntimeCallInfo* runtimeCallInfo, EcmaVM* vm, std::vector<ArkUI_Float32>& values, int32_t argsIndex)
{
    Local<JSValueRef> topLeftArgs = runtimeCallInfo->GetCallArgRef(argsIndex);
    Local<JSValueRef> topRightArgs = runtimeCallInfo->GetCallArgRef(argsIndex + NUM_1);
    Local<JSValueRef> bottomLeftArgs = runtimeCallInfo->GetCallArgRef(argsIndex + NUM_2);
    Local<JSValueRef> bottomRightArgs = runtimeCallInfo->GetCallArgRef(argsIndex + NUM_3);

    std::optional<CalcDimension> topLeftOptional;
    std::optional<CalcDimension> topRightOptional;
    std::optional<CalcDimension> bottomLeftOptional;
    std::optional<CalcDimension> bottomRightOptional;

    ArkTSUtils::ParseOuterBorder(vm, topLeftArgs, topLeftOptional);
    ArkTSUtils::ParseOuterBorder(vm, topRightArgs, topRightOptional);
    ArkTSUtils::ParseOuterBorder(vm, bottomLeftArgs, bottomLeftOptional);
    ArkTSUtils::ParseOuterBorder(vm, bottomRightArgs, bottomRightOptional);

    PushOuterBorderDimensionVector(topLeftOptional, values);
    PushOuterBorderDimensionVector(topRightOptional, values);
    PushOuterBorderDimensionVector(bottomLeftOptional, values);
    PushOuterBorderDimensionVector(bottomRightOptional, values);
}

void PushOuterBorderStyleVector(const std::optional<BorderStyle>& value, std::vector<uint32_t> &options)
{
    options.push_back(static_cast<uint32_t>(value.has_value()));
    if (value.has_value()) {
        options.push_back(static_cast<uint32_t>(value.value()));
    } else {
        options.push_back(0);
    }
}

void ParseOuterBorderStyle(
    ArkUIRuntimeCallInfo* runtimeCallInfo, EcmaVM* vm, std::vector<uint32_t>& values, int32_t argsIndex)
{
    std::optional<BorderStyle> styleLeft;
    std::optional<BorderStyle> styleRight;
    std::optional<BorderStyle> styleTop;
    std::optional<BorderStyle> styleBottom;

    auto topArg = runtimeCallInfo->GetCallArgRef(argsIndex);
    auto rightArg = runtimeCallInfo->GetCallArgRef(argsIndex + NUM_1);
    auto bottomArg = runtimeCallInfo->GetCallArgRef(argsIndex + NUM_2);
    auto leftArg = runtimeCallInfo->GetCallArgRef(argsIndex + NUM_3);

    if (!topArg->IsUndefined() && topArg->IsNumber()) {
        styleTop = ConvertBorderStyle(topArg->Int32Value(vm));
    }
    if (!rightArg->IsUndefined() && rightArg->IsNumber()) {
        styleRight = ConvertBorderStyle(rightArg->Int32Value(vm));
    }
    if (!bottomArg->IsUndefined() && bottomArg->IsNumber()) {
        styleBottom = ConvertBorderStyle(bottomArg->Int32Value(vm));
    }
    if (!leftArg->IsUndefined() && leftArg->IsNumber()) {
        styleLeft = ConvertBorderStyle(leftArg->Int32Value(vm));
    }

    PushOuterBorderStyleVector(styleLeft, values);
    PushOuterBorderStyleVector(styleRight, values);
    PushOuterBorderStyleVector(styleTop, values);
    PushOuterBorderStyleVector(styleBottom, values);
}

void ParseOuterBorderDashParam(ArkUIRuntimeCallInfo *runtimeCallInfo, EcmaVM *vm, std::vector<ArkUI_Float32> &values,
    int32_t argsIndex)
{
    Local<JSValueRef> leftArgs = runtimeCallInfo->GetCallArgRef(argsIndex);
    Local<JSValueRef> rightArgs = runtimeCallInfo->GetCallArgRef(argsIndex + NUM_1);
    Local<JSValueRef> topArgs = runtimeCallInfo->GetCallArgRef(argsIndex + NUM_2);
    Local<JSValueRef> bottomArgs = runtimeCallInfo->GetCallArgRef(argsIndex + NUM_3);
    std::optional<CalcDimension> leftDim;
    std::optional<CalcDimension> rightDim;
    std::optional<CalcDimension> topDim;
    std::optional<CalcDimension> bottomDim;

    ArkTSUtils::ParseOuterBorderForDashParams(vm, leftArgs, leftDim);
    ArkTSUtils::ParseOuterBorderForDashParams(vm, rightArgs, rightDim);
    ArkTSUtils::ParseOuterBorderForDashParams(vm, topArgs, topDim);
    ArkTSUtils::ParseOuterBorderForDashParams(vm, bottomArgs, bottomDim);

    PushOuterBorderDimensionVector(leftDim, values);
    PushOuterBorderDimensionVector(rightDim, values);
    PushOuterBorderDimensionVector(topDim, values);
    PushOuterBorderDimensionVector(bottomDim, values);
}

void SetBackgroundImagePositionAlign(double &value, DimensionUnit &type, double valueContent,
    const DimensionUnit &typeContent)
{
    value = valueContent;
    type = typeContent;
}

void ParseBackgroundImagePositionAlign(const int32_t align, double &valueX, double &valueY, DimensionUnit &typeX,
    DimensionUnit &typeY)
{
    switch (align) {
        case NUM_0:
            SetBackgroundImagePositionAlign(valueX, typeX, 0.0, DimensionUnit::PERCENT);
            SetBackgroundImagePositionAlign(valueY, typeY, 0.0, DimensionUnit::PERCENT);
            break;
        case NUM_1:
            SetBackgroundImagePositionAlign(valueX, typeX, HALF_DIMENSION, DimensionUnit::PERCENT);
            SetBackgroundImagePositionAlign(valueY, typeY, 0.0, DimensionUnit::PERCENT);
            break;
        case NUM_2:
            SetBackgroundImagePositionAlign(valueX, typeX, FULL_DIMENSION, DimensionUnit::PERCENT);
            SetBackgroundImagePositionAlign(valueY, typeY, 0.0, DimensionUnit::PERCENT);
            break;
        case NUM_3:
            SetBackgroundImagePositionAlign(valueX, typeX, 0.0, DimensionUnit::PERCENT);
            SetBackgroundImagePositionAlign(valueY, typeY, HALF_DIMENSION, DimensionUnit::PERCENT);
            break;
        case NUM_4:
            SetBackgroundImagePositionAlign(valueX, typeX, HALF_DIMENSION, DimensionUnit::PERCENT);
            SetBackgroundImagePositionAlign(valueY, typeY, HALF_DIMENSION, DimensionUnit::PERCENT);
            break;
        case NUM_5:
            SetBackgroundImagePositionAlign(valueX, typeX, FULL_DIMENSION, DimensionUnit::PERCENT);
            SetBackgroundImagePositionAlign(valueY, typeY, HALF_DIMENSION, DimensionUnit::PERCENT);
            break;
        case NUM_6:
            SetBackgroundImagePositionAlign(valueX, typeX, 0.0, DimensionUnit::PERCENT);
            SetBackgroundImagePositionAlign(valueY, typeY, FULL_DIMENSION, DimensionUnit::PERCENT);
            break;
        case NUM_7:
            SetBackgroundImagePositionAlign(valueX, typeX, HALF_DIMENSION, DimensionUnit::PERCENT);
            SetBackgroundImagePositionAlign(valueY, typeY, FULL_DIMENSION, DimensionUnit::PERCENT);
            break;
        case NUM_8:
            SetBackgroundImagePositionAlign(valueX, typeX, FULL_DIMENSION, DimensionUnit::PERCENT);
            SetBackgroundImagePositionAlign(valueY, typeY, FULL_DIMENSION, DimensionUnit::PERCENT);
            break;
        default:
            break;
    }
}

bool ParseAxisDimensionVp(const EcmaVM *vm, const Local<JSValueRef> &jsValue, CalcDimension &result,
    bool checkIllegal = false)
{
    if (jsValue->IsNumber()) {
        result = Dimension(jsValue->ToNumber(vm)->Value(), DimensionUnit::VP);
        return true;
    }
    if (jsValue->IsString(vm)) {
        if (checkIllegal) {
            return StringUtils::StringToDimensionWithUnitNG(jsValue->ToString(vm)->ToString(), result,
                DimensionUnit::VP);
        }
        result = StringUtils::StringToCalcDimension(jsValue->ToString(vm)->ToString(), false, DimensionUnit::VP);
        return true;
    }
    return false;
}

void ParseDirection(EcmaVM *vm, const Local<JSValueRef> &directionArg, float &value)
{
    if (directionArg->IsNumber()) {
        value = directionArg->ToNumber(vm)->Value();
    }
}

void GetJsAngle(const EcmaVM* vm, const Local<JSValueRef>& angleArg, std::optional<float>& angle)
{
    if (angleArg->IsString(vm)) {
        angle = static_cast<float>(StringUtils::StringToDegree(angleArg->ToString(vm)->ToString()));
    } else if (angleArg->IsNumber()) {
        angle = static_cast<float>(angleArg->ToNumber(vm)->Value());
    } else {
        LOGE("Invalid value type");
    }
}

void ParseCenterDimension(const EcmaVM* vm, const Local<JSValueRef>& centerArg, CalcDimension& centerDimension)
{
    if (!ArkTSUtils::ParseJsDimensionVp(vm, centerArg, centerDimension, false)) {
        centerDimension = Dimension(0.5f, DimensionUnit::PERCENT);
    }
}

bool ParseRotate(ArkUIRuntimeCallInfo *runtimeCallInfo, ArkUI_Float32 values[], int units[],
    int valuesLength, int unitsLength)
{
    if (valuesLength != SIZE_OF_EIGHT || unitsLength != SIZE_OF_THREE) {
        return false;
    }
    EcmaVM *vm = runtimeCallInfo->GetVM();
    Local<JSValueRef> xDirectionArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> yDirectionArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> zDirectionArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> angleArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    Local<JSValueRef> centerXArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    Local<JSValueRef> centerYArg = runtimeCallInfo->GetCallArgRef(NUM_6);
    Local<JSValueRef> centerZArg = runtimeCallInfo->GetCallArgRef(NUM_7);
    Local<JSValueRef> perspectiveArg = runtimeCallInfo->GetCallArgRef(NUM_8);
    float xDirection = 0.0f;
    float yDirection = 0.0f;
    float zDirection = 0.0f;
    float angle = 0.0f;
    std::optional<float> angleOptional;
    CalcDimension centerX = 0.5_pct;
    CalcDimension centerY = 0.5_pct;
    CalcDimension centerZ = CalcDimension(0.0f, DimensionUnit::VP);
    GetJsAngle(vm, angleArg, angleOptional);
    if (!angleOptional) {
        return false;
    }
    angle = angleOptional.value();
    double perspective = 0.0;
    if (!xDirectionArg->IsNumber() && !yDirectionArg->IsNumber() && !zDirectionArg->IsNumber()) {
        zDirection = 1.0f;
    }
    ParseDirection(vm, xDirectionArg, xDirection);
    ParseDirection(vm, yDirectionArg, yDirection);
    ParseDirection(vm, zDirectionArg, zDirection);
    ParseCenterDimension(vm, centerXArg, centerX);
    ParseCenterDimension(vm, centerYArg, centerY);
    ParseCenterDimension(vm, centerZArg, centerZ);
    ArkTSUtils::ParseJsDouble(vm, perspectiveArg, perspective);
    values[NUM_0] = static_cast<ArkUI_Float32>(centerX.Value());
    units[NUM_0] = static_cast<int>(centerX.Unit());
    values[NUM_1] = static_cast<ArkUI_Float32>(centerY.Value());
    units[NUM_1] = static_cast<int>(centerY.Unit());
    values[NUM_2] = static_cast<ArkUI_Float32>(centerZ.Value());
    units[NUM_2] = static_cast<int>(centerZ.Unit());
    values[NUM_3] = static_cast<ArkUI_Float32>(xDirection);
    values[NUM_4] = static_cast<ArkUI_Float32>(yDirection);
    values[NUM_5] = static_cast<ArkUI_Float32>(zDirection);
    values[NUM_6] = static_cast<ArkUI_Float32>(angle);
    values[NUM_7] = static_cast<ArkUI_Float32>(perspective);
    return true;
}

void ParseBlurOption(const EcmaVM* vm, const Local<JSValueRef> blurOptionsArg, BlurOption& blurOption)
{
    if (blurOptionsArg->IsArray(vm)) {
        Local<panda::ArrayRef> params = static_cast<Local<panda::ArrayRef>>(blurOptionsArg);
        auto grey1 = params->GetValueAt(vm, blurOptionsArg, 0)->Uint32Value(vm);
        auto grey2 = params->GetValueAt(vm, blurOptionsArg, 1)->Uint32Value(vm);
        std::vector<float> greyVec(2); // 2 number
        greyVec[0] = grey1;
        greyVec[1] = grey2;
        blurOption.grayscale = greyVec;
    }
}

void ParseDynamicBrightnessOption(ArkUIRuntimeCallInfo* runtimeCallInfo, EcmaVM* vm,
    ArkUI_Float32& rate, ArkUI_Float32& lightUpDegree, ArkUI_Float32& cubicCoeff, ArkUI_Float32& quadCoeff,
    ArkUI_Float32& saturation, std::vector<float>& posRGB, std::vector<float>& negRGB, ArkUI_Float32& fraction) 
{   
    Local<JSValueRef> rateArg = runtimeCallInfo->GetCallArgRef(1);         
    Local<JSValueRef> lightUpDegreeArg = runtimeCallInfo->GetCallArgRef(2); 
    Local<JSValueRef> cubicCoeffArg = runtimeCallInfo->GetCallArgRef(3);
    Local<JSValueRef> quadCoeffArg = runtimeCallInfo->GetCallArgRef(4);
    Local<JSValueRef> saturationArg = runtimeCallInfo->GetCallArgRef(5);
    Local<JSValueRef> posRGBArg = runtimeCallInfo->GetCallArgRef(6);
    Local<JSValueRef> negRGBArg = runtimeCallInfo->GetCallArgRef(7);
    Local<JSValueRef> fractionArg = runtimeCallInfo->GetCallArgRef(8);
    if (rateArg->IsNumber()) {
        rate = rateArg->ToNumber(vm)->Value();
    }
    if (lightUpDegreeArg->IsNumber()) {
        lightUpDegree = lightUpDegreeArg->ToNumber(vm)->Value();
    }
    if (cubicCoeffArg->IsNumber()) {
        cubicCoeff = cubicCoeffArg->ToNumber(vm)->Value();
    }
    if (quadCoeffArg->IsNumber()) {
        quadCoeff = quadCoeffArg->ToNumber(vm)->Value();
    }
    if (saturationArg->IsNumber()) {
        saturation = saturationArg->ToNumber(vm)->Value();
    }
    if (posRGBArg->IsArray(vm)) {
        Local<panda::ArrayRef> params = static_cast<Local<panda::ArrayRef>>(posRGBArg);
        auto r = params->GetValueAt(vm, posRGBArg, 0)->Uint32Value(vm);
        auto g = params->GetValueAt(vm, posRGBArg, 1)->Uint32Value(vm);
        auto b = params->GetValueAt(vm, posRGBArg, 2)->Uint32Value(vm);
        posRGB[0] = r;
        posRGB[1] = g;
        posRGB[2] = b;
    }
    if (negRGBArg->IsArray(vm)) {
        Local<panda::ArrayRef> params = static_cast<Local<panda::ArrayRef>>(negRGBArg);
        auto r = params->GetValueAt(vm, negRGBArg, 0)->Uint32Value(vm);
        auto g = params->GetValueAt(vm, negRGBArg, 1)->Uint32Value(vm);
        auto b = params->GetValueAt(vm, negRGBArg, 2)->Uint32Value(vm);
        negRGB[0] = r;
        negRGB[1] = g;
        negRGB[2] = b;
    }
    if (fractionArg->IsNumber()) {
        fraction = fractionArg->ToNumber(vm)->Value();
    }
}

bool ParseCalcDimension(const EcmaVM* vm,
    ArkUINodeHandle node, const Local<JSValueRef>& value, CalcDimension& result, bool isWidth)
{
    CHECK_NULL_RETURN(vm, false);
    bool undefined = value->IsUndefined();
    if (undefined) {
        GetArkUINodeModifiers()->getCommonModifier()->clearWidthOrHeight(node, isWidth);
        return true;
    }
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
        if (!ArkTSUtils::ParseJsDimensionVpNG(vm, value, result)) {
            GetArkUINodeModifiers()->getCommonModifier()->clearWidthOrHeight(node, isWidth);
            return false;
        }
    } else if (!ArkTSUtils::ParseJsDimensionVp(vm, value, result)) {
        return false;
    }

    if (LessNotEqual(result.Value(), 0.0)) {
        result.SetValue(0.0);
    }
    std::string calc = result.CalcValue();
    if (isWidth) {
        GetArkUINodeModifiers()->getCommonModifier()->setWidth(
            node, result.Value(), static_cast<int32_t>(result.Unit()), calc.c_str());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->setHeight(
            node, result.Value(), static_cast<int32_t>(result.Unit()), calc.c_str());
    }
    return true;
}

void ParseResizableCalcDimensions(ArkUIRuntimeCallInfo* runtimeCallInfo, uint32_t offset, uint32_t count,
    std::vector<std::optional<CalcDimension>>& results, const CalcDimension& defValue)
{
    auto end = offset + count;
    auto argsNumber = runtimeCallInfo->GetArgsNumber();
    if (end > argsNumber) {
        return;
    }
    CalcDimension defaultDimension(defValue);
    EcmaVM* vm = runtimeCallInfo->GetVM();
    for (uint32_t index = offset; index < end; index++) {
        auto arg = runtimeCallInfo->GetCallArgRef(index);
        std::optional<CalcDimension> optCalcDimension;
        CalcDimension dimension(defValue);
        if (ArkTSUtils::ParseJsDimensionVp(vm, arg, dimension, false)) {
            optCalcDimension = dimension;
        } else {
            optCalcDimension = defaultDimension;
        }
        results.push_back(optCalcDimension);
    }
}

bool ParseJsAlignRule(const EcmaVM* vm, const Local<JSValueRef> &arg, std::string& anchor, ArkUI_Int32 &direction)
{
    if (arg->IsString(vm)) {
        std::string directionString = arg->ToString(vm)->ToString();
        if (directionString.empty()) {
            return false;
        }
        size_t pos = directionString.find('|');
        if (pos == std::string::npos) {
            return false;
        }
        char* endPtr = nullptr;
        long alignValue = std::strtol(directionString.substr(0, pos).c_str(), &endPtr, 10);
        direction = static_cast<int8_t>(alignValue);
        anchor = directionString.substr(pos + 1);
        return true;
    }
    return false;
}

bool ParseResponseRegion(const EcmaVM* vm, const Local<JSValueRef>& jsValue, ArkUI_Float32 regionValues[],
    int32_t regionUnits[], int32_t length)
{
    if (jsValue->IsUndefined() || !jsValue->IsArray(vm)) {
        return false;
    }

    Local<panda::ArrayRef> transArray = static_cast<Local<panda::ArrayRef>>(jsValue);
    for (int32_t i = 0; i < length; i = i + NUM_4) {
        Local<JSValueRef> x = transArray->GetValueAt(vm, jsValue, i + NUM_0);
        Local<JSValueRef> y = transArray->GetValueAt(vm, jsValue, i + NUM_1);
        Local<JSValueRef> width = transArray->GetValueAt(vm, jsValue, i + NUM_2);
        Local<JSValueRef> height = transArray->GetValueAt(vm, jsValue, i + NUM_3);
        CalcDimension xDimen = CalcDimension(0.0, DimensionUnit::VP);
        CalcDimension yDimen = CalcDimension(0.0, DimensionUnit::VP);
        CalcDimension widthDimen = CalcDimension(1, DimensionUnit::PERCENT);
        CalcDimension heightDimen = CalcDimension(1, DimensionUnit::PERCENT);
        auto s1 = width->ToString(vm)->ToString();
        auto s2 = height->ToString(vm)->ToString();
        if (s1.find('-') != std::string::npos) {
            width = ToJSValue("100%");
        }
        if (s2.find('-') != std::string::npos) {
            height = ToJSValue("100%");
        }
        if (!ArkTSUtils::ParseJsDimensionNG(vm, x, xDimen, DimensionUnit::VP)) {
            xDimen = CalcDimension(0.0, DimensionUnit::VP);
        }
        if (!ArkTSUtils::ParseJsDimensionNG(vm, y, yDimen, DimensionUnit::VP)) {
            yDimen = CalcDimension(0.0, DimensionUnit::VP);
        }
        if (!ArkTSUtils::ParseJsDimensionNG(vm, width, widthDimen, DimensionUnit::VP)) {
            widthDimen = CalcDimension(1, DimensionUnit::PERCENT);
        }
        if (!ArkTSUtils::ParseJsDimensionNG(vm, height, heightDimen, DimensionUnit::VP)) {
            heightDimen = CalcDimension(1, DimensionUnit::PERCENT);
        }
        regionValues[i + NUM_0] = static_cast<ArkUI_Float32>(xDimen.Value());
        regionUnits[i + NUM_0] = static_cast<int32_t>(xDimen.Unit());
        regionValues[i + NUM_1] = static_cast<ArkUI_Float32>(yDimen.Value());
        regionUnits[i + NUM_1] = static_cast<int32_t>(yDimen.Unit());
        regionValues[i + NUM_2] = static_cast<ArkUI_Float32>(widthDimen.Value());
        regionUnits[i + NUM_2] = static_cast<int32_t>(widthDimen.Unit());
        regionValues[i + NUM_3] = static_cast<ArkUI_Float32>(heightDimen.Value());
        regionUnits[i + NUM_3] = static_cast<int32_t>(heightDimen.Unit());
    }
    return true;
}

std::function<void(bool)> ParseTransitionCallback(
    const JSRef<JSFunc>& jsFunc, const JSExecutionContext& context, FrameNode* node)
{
    auto jsFuncFinish = AceType::MakeRefPtr<JsFunction>(JSRef<JSFunc>::Cast(jsFunc));
    auto targetNode = AceType::WeakClaim(node);
    auto finishCallback = [execCtx = context, jsFuncFinish, targetNode](bool isTransitionIn) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        NG::PipelineContext::SetCallBackNode(targetNode);
        JSRef<JSVal> newJSVal = JSRef<JSVal>::Make(ToJSValue(isTransitionIn));
        jsFuncFinish->ExecuteJS(1, &newJSVal);
    };
    return finishCallback;
}
} // namespace

ArkUINativeModuleValue CommonBridge::SetBackgroundColor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Color color;
    if (!ArkTSUtils::ParseJsColorAlpha(vm, secondArg, color)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetBackgroundColor(nativeNode);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->setBackgroundColor(nativeNode, color.GetValue());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundColor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBackgroundColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

void SetBorderWidthArray(const EcmaVM* vm, const Local<JSValueRef>& args, ArkUI_Float32 values[],
    int units[], int index)
{
    CalcDimension borderDimension;
    if (!args->IsUndefined()) {
        if (ArkTSUtils::ParseAllBorder(vm, args, borderDimension)) {
            values[index] = borderDimension.Value();
            units[index] = static_cast<int>(borderDimension.Unit());
        } else {
            values[index] = 0;
            units[index] = static_cast<int>(DimensionUnit::VP);
        }
    } else {
        values[index] = -1;
        units[index] = static_cast<int>(DimensionUnit::INVALID);
    }
}

ArkUINativeModuleValue CommonBridge::SetBorderWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> leftArgs = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> rightArgs = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> topArgs = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> bottomArgs = runtimeCallInfo->GetCallArgRef(NUM_4);
    if (leftArgs->IsUndefined() && rightArgs->IsUndefined() && topArgs->IsUndefined() && bottomArgs->IsUndefined()) {
        GetArkUINodeModifiers()->getCommonModifier()->resetBorderWidth(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    uint32_t size = SIZE_OF_FOUR;
    ArkUI_Float32 values[size];
    int units[size];

    SetBorderWidthArray(vm, leftArgs, values, units, NUM_0);
    SetBorderWidthArray(vm, rightArgs, values, units, NUM_1);
    SetBorderWidthArray(vm, topArgs, values, units, NUM_2);
    SetBorderWidthArray(vm, bottomArgs, values, units, NUM_3);

    GetArkUINodeModifiers()->getCommonModifier()->setBorderWidth(nativeNode, values, units, size);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBorderWidth(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBorderWidth(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBorderRadius(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> topLeftArgs = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> topRightArgs = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> bottomLeftArgs = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> bottomRightArgs = runtimeCallInfo->GetCallArgRef(NUM_4);
    if (topLeftArgs->IsUndefined() && topRightArgs->IsUndefined() && bottomLeftArgs->IsUndefined() &&
        bottomRightArgs->IsUndefined()) {
        GetArkUINodeModifiers()->getCommonModifier()->resetBorderRadius(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    CalcDimension topLeft;
    CalcDimension topRight;
    CalcDimension bottomLeft;
    CalcDimension bottomRight;

    ArkTSUtils::ParseAllBorder(vm, topLeftArgs, topLeft);
    ArkTSUtils::ParseAllBorder(vm, topRightArgs, topRight);
    ArkTSUtils::ParseAllBorder(vm, bottomLeftArgs, bottomLeft);
    ArkTSUtils::ParseAllBorder(vm, bottomRightArgs, bottomRight);

    uint32_t size = SIZE_OF_FOUR;
    ArkUI_Float32 values[size];
    int units[size];

    values[NUM_0] = topLeft.Value();
    units[NUM_0] = static_cast<int>(topLeft.Unit());
    values[NUM_1] = topRight.Value();
    units[NUM_1] = static_cast<int>(topRight.Unit());
    values[NUM_2] = bottomLeft.Value();
    units[NUM_2] = static_cast<int>(bottomLeft.Unit());
    values[NUM_3] = bottomRight.Value();
    units[NUM_3] = static_cast<int>(bottomRight.Unit());

    GetArkUINodeModifiers()->getCommonModifier()->setBorderRadius(nativeNode, values, units, SIZE_OF_FOUR);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBorderRadius(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBorderRadius(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> jsValue = runtimeCallInfo->GetCallArgRef(NUM_1);

    CalcDimension width;
    std::string calcStr;
    if (!ArkTSUtils::ParseJsDimensionVpNG(vm, jsValue, width)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetWidth(nativeNode);
    } else {
        if (LessNotEqual(width.Value(), 0.0)) {
            if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
                GetArkUINodeModifiers()->getCommonModifier()->resetWidth(nativeNode);
                return panda::JSValueRef::Undefined(vm);
            }
            width.SetValue(0.0);
        }

        if (width.Unit() == DimensionUnit::CALC) {
            GetArkUINodeModifiers()->getCommonModifier()->setWidth(
                nativeNode, 0, static_cast<int32_t>(width.Unit()), width.CalcValue().c_str());
        } else {
            GetArkUINodeModifiers()->getCommonModifier()->setWidth(
                nativeNode, width.Value(), static_cast<int32_t>(width.Unit()), calcStr.c_str());
        }
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetWidth(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetHeight(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> jsValue = runtimeCallInfo->GetCallArgRef(NUM_1);
    CalcDimension height;
    std::string calcStr;
    if (!ArkTSUtils::ParseJsDimensionVpNG(vm, jsValue, height)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetHeight(nativeNode);
    } else {
        if (LessNotEqual(height.Value(), 0.0)) {
            if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
                GetArkUINodeModifiers()->getCommonModifier()->resetHeight(nativeNode);
                return panda::JSValueRef::Undefined(vm);
            }
            height.SetValue(0.0);
        }
        if (height.Unit() == DimensionUnit::CALC) {
            GetArkUINodeModifiers()->getCommonModifier()->setHeight(
                nativeNode, height.Value(), static_cast<int32_t>(height.Unit()), height.CalcValue().c_str());
        } else {
            GetArkUINodeModifiers()->getCommonModifier()->setHeight(
                nativeNode, height.Value(), static_cast<int32_t>(height.Unit()), calcStr.c_str());
        }
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetHeight(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetHeight(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetPosition(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    bool useEdges = runtimeCallInfo->GetCallArgRef(NUM_1)->ToBoolean(vm)->Value();
    std::vector<ArkUIStringAndFloat> options;
    std::vector<std::optional<CalcDimension>> edges;

    if (useEdges) {
        ParseCalcDimensions(runtimeCallInfo, NUM_2, NUM_4, edges, CalcDimension(0.0));
        PushDimensionsToVector(options, edges);
        GetArkUINodeModifiers()->getCommonModifier()->setPositionEdges(nativeNode, useEdges, options.data());
    } else {
        ParseCalcDimensions(runtimeCallInfo, NUM_2, NUM_2, edges, CalcDimension(0.0));
        PushDimensionsToVector(options, edges);
        GetArkUINodeModifiers()->getCommonModifier()->setPositionEdges(nativeNode, useEdges, options.data());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetPosition(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetPositionEdges(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetTransform(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> jsValue = runtimeCallInfo->GetCallArgRef(NUM_1);

    if (!jsValue->IsArray(vm)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetTransform(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    const auto matrix4Len = Matrix4::DIMENSION * Matrix4::DIMENSION;
    float matrix[matrix4Len];
    Local<panda::ArrayRef> transArray = static_cast<Local<panda::ArrayRef>>(jsValue);
    for (size_t i = 0; i < transArray->Length(vm); i++) {
        Local<JSValueRef> value = transArray->GetValueAt(vm, jsValue, i);
        matrix[i] = value->ToNumber(vm)->Value();
    }
    GetArkUINodeModifiers()->getCommonModifier()->setTransform(nativeNode, matrix, matrix4Len);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetTransform(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    GetArkUINodeModifiers()->getCommonModifier()->resetTransform(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBorderColor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> leftArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> rightArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> topArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> bottomArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    Color leftColor;
    Color rightColor;
    Color topColor;
    Color bottomColor;

    if (!ArkTSUtils::ParseJsColorAlpha(vm, leftArg, leftColor)) {
        leftColor.SetValue(COLOR_ALPHA_VALUE);
    }
    if (!ArkTSUtils::ParseJsColorAlpha(vm, rightArg, rightColor)) {
        rightColor.SetValue(COLOR_ALPHA_VALUE);
    }
    if (!ArkTSUtils::ParseJsColorAlpha(vm, topArg, topColor)) {
        topColor.SetValue(COLOR_ALPHA_VALUE);
    }
    if (!ArkTSUtils::ParseJsColorAlpha(vm, bottomArg, bottomColor)) {
        bottomColor.SetValue(COLOR_ALPHA_VALUE);
    }
    GetArkUINodeModifiers()->getCommonModifier()->setBorderColor(nativeNode, leftColor.GetValue(),
        rightColor.GetValue(), topColor.GetValue(), bottomColor.GetValue());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBorderColor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBorderColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOutlineColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    std::vector<uint32_t> colorOptions;
    ParseOuterBorderColor(runtimeCallInfo, vm, colorOptions, NUM_1);
    GetArkUINodeModifiers()->getCommonModifier()->setOutlineColor(
        nativeNode, colorOptions.data(), colorOptions.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOutlineColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetOutlineColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOutlineRadius(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    std::vector<ArkUI_Float32> radiusOptions;
    ParseOuterBorderRadius(runtimeCallInfo, vm, radiusOptions, NUM_1);
    GetArkUINodeModifiers()->getCommonModifier()->setOutlineRadius(
        nativeNode, radiusOptions.data(), radiusOptions.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOutlineRadius(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetOutlineRadius(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOutlineWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    std::vector<ArkUI_Float32> widthOptions;
    ParseOuterBorderWidth(runtimeCallInfo, vm, widthOptions);
    GetArkUINodeModifiers()->getCommonModifier()->setOutlineWidth(
        nativeNode, widthOptions.data(), widthOptions.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOutlineWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetOutlineWidth(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOutlineStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    std::vector<uint32_t> styleOptions;
    ParseOuterBorderStyle(runtimeCallInfo, vm, styleOptions, NUM_1);
    GetArkUINodeModifiers()->getCommonModifier()->setOutlineStyle(
        nativeNode, styleOptions.data(), styleOptions.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOutlineStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetOutlineStyle(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOutline(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    std::vector<ArkUI_Float32> options;
    ParseOuterBorderWidth(runtimeCallInfo, vm, options);         // Outline Width args start index from 1
    ParseOuterBorderRadius(runtimeCallInfo, vm, options, NUM_9); // Outline Radius args start index

    std::vector<uint32_t> colorAndStyleOptions;
    ParseOuterBorderColor(runtimeCallInfo, vm, colorAndStyleOptions, NUM_5);  // Outline Color args start index
    ParseOuterBorderStyle(runtimeCallInfo, vm, colorAndStyleOptions, NUM_13); // Outline Style args start index

    GetArkUINodeModifiers()->getCommonModifier()->setOutline(
        nativeNode, options.data(), options.size(), colorAndStyleOptions.data(), colorAndStyleOptions.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOutline(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetOutline(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBorderStyle(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto typeArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto styleArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto topArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto rightArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    auto bottomArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    auto leftArg = runtimeCallInfo->GetCallArgRef(NUM_6);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if ((!typeArg->IsBoolean()) || (!typeArg->BooleaValue())) {
        int32_t styles[] = { static_cast<int32_t>(BorderStyle::SOLID) };
        GetArkUINodeModifiers()->getCommonModifier()->setBorderStyle(nativeNode, styles,
            (sizeof(styles) / sizeof(styles[NUM_0])));
        return panda::JSValueRef::Undefined(vm);
    }
    if (styleArg->IsInt()) {
        int32_t styles[] = { styleArg->Int32Value(vm) };
        GetArkUINodeModifiers()->getCommonModifier()->setBorderStyle(nativeNode, styles,
            (sizeof(styles) / sizeof(styles[NUM_0])));
        return panda::JSValueRef::Undefined(vm);
    }
    int32_t styles[] = { -1, -1, -1, -1 };
    if (topArg->IsInt()) {
        styles[NUM_0] = topArg->Int32Value(vm);
    }
    if (rightArg->IsInt()) {
        styles[NUM_1] = rightArg->Int32Value(vm);
    }
    if (bottomArg->IsInt()) {
        styles[NUM_2] = bottomArg->Int32Value(vm);
    }
    if (leftArg->IsInt()) {
        styles[NUM_3] = leftArg->Int32Value(vm);
    }
    GetArkUINodeModifiers()->getCommonModifier()->setBorderStyle(nativeNode, styles,
        (sizeof(styles) / sizeof(styles[NUM_0])));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBorderStyle(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBorderStyle(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetShadow(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto styleArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto radiusArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto typeArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto colorArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    auto offsetXArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    auto offsetYArg = runtimeCallInfo->GetCallArgRef(NUM_6);
    auto fillArg = runtimeCallInfo->GetCallArgRef(NUM_7);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    int32_t shadowStyle = 0;
    if (ArkTSUtils::ParseJsInteger(vm, styleArg, shadowStyle)) {
        ArkUIInt32orFloat32 shadows[] = { {.i32 = shadowStyle} };
        GetArkUINodeModifiers()->getCommonModifier()->setBackShadow(nativeNode, shadows,
            (sizeof(shadows) / sizeof(shadows[NUM_0])));
        return panda::JSValueRef::Undefined(vm);
    }
    ArkUIInt32orFloat32 shadows[] = { { 0.0 }, { .i32 = 0  }, { 0.0 }, { 0.0 },
        { .i32 = static_cast<ArkUI_Int32>(ShadowType::COLOR) }, { .u32 = 0 }, { .i32 = 0 } };
    double radius;
    ArkTSUtils::ParseJsDouble(vm, radiusArg, radius);
    shadows[NUM_0].f32 = radius;

    shadows[NUM_0].f32 = (LessNotEqual(shadows[NUM_0].f32, 0.0)) ? 0.0 : shadows[NUM_0].f32;
    CalcDimension offsetX;
    if (ParseJsShadowDimension(vm, offsetXArg, offsetX)) {
        shadows[NUM_2].f32 = offsetX.Value();
    }
    CalcDimension offsetY;
    if (ParseJsShadowDimension(vm, offsetYArg, offsetY)) {
        shadows[NUM_3].f32 = offsetY.Value();
    }
    if (typeArg->IsInt()) {
        uint32_t shadowType = typeArg->Uint32Value(vm);
        shadows[NUM_4].i32 =
            std::clamp(shadowType, static_cast<uint32_t>(ShadowType::COLOR), static_cast<uint32_t>(ShadowType::BLUR));
    }
    int32_t type = 0;
    uint32_t color = 0;
    if (ParseJsShadowColor(vm, colorArg, type, color)) {
        shadows[NUM_1].i32 = type;
        shadows[NUM_5].u32 = color;
    }
    shadows[NUM_6].i32 = fillArg->IsBoolean() ? fillArg->BooleaValue() : false;
    GetArkUINodeModifiers()->getCommonModifier()->setBackShadow(nativeNode, shadows,
        (sizeof(shadows) / sizeof(shadows[NUM_0])));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetShadow(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBackShadow(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetHitTestBehavior(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsNumber()) {
        uint32_t hitTestModeNG = secondArg->Uint32Value(vm);
        GetArkUINodeModifiers()->getCommonModifier()->setHitTestBehavior(nativeNode, hitTestModeNG);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetHitTestBehavior(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetHitTestBehavior(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetHitTestBehavior(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetZIndex(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsNumber()) {
        int32_t value = secondArg->Int32Value(vm);
        GetArkUINodeModifiers()->getCommonModifier()->setZIndex(nativeNode, value);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetZIndex(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetZIndex(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetZIndex(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOpacity(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    double opacity;
    if (!ArkTSUtils::ParseJsDouble(vm, secondArg, opacity)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetOpacity(nativeNode);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->setOpacity(nativeNode, opacity);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOpacity(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetOpacity(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAlign(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsNumber()) {
        GetArkUINodeModifiers()->getCommonModifier()->setAlign(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetAlign(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAlign(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetAlign(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBackdropBlur(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> blurArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> blurOptionArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    double blur = 0.0;
    if (!ArkTSUtils::ParseJsDouble(vm, blurArg, blur)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetBackdropBlur(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    BlurOption blurOption;
    if (blurOptionArg->IsArray(vm)) {
        ParseBlurOption(vm, blurOptionArg, blurOption);
    }
    GetArkUINodeModifiers()->getCommonModifier()->setBackdropBlur(
        nativeNode, blur, blurOption.grayscale.data(), blurOption.grayscale.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackdropBlur(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBackdropBlur(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetHueRotate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    std::optional<float> degree;
    if (secondArg->IsString(vm)) {
        degree = static_cast<float>(StringUtils::StringToDegree(secondArg->ToString(vm)->ToString()));
    } else if (secondArg->IsNumber()) {
        degree = static_cast<float>(secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetHueRotate(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    float deg = 0.0f;
    if (degree) {
        deg = degree.value();
        degree.reset();
    }
    GetArkUINodeModifiers()->getCommonModifier()->setHueRotate(nativeNode, deg);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetHueRotate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetHueRotate(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetInvert(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> invertValueArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> optionLowArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> optionHighArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> optionThresholdArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    Local<JSValueRef> optionThresholdRangeArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    if (!invertValueArg->IsUndefined()) {
        double invertValue = 0.0;
        if (ArkTSUtils::ParseJsDouble(vm, invertValueArg, invertValue)) {
            ArkUI_Float32 invert[] = { invertValue };
            GetArkUINodeModifiers()->getCommonModifier()->setInvert(nativeNode, invert, NUM_1);
        } else {
            GetArkUINodeModifiers()->getCommonModifier()->resetInvert(nativeNode);
        }
    } else {
        ArkUI_Float32 invert[] = { 0.0, 0.0, 0.0, 0.0 };
        double low = 0.0;
        double high = 0.0;
        double threshold = 0.0;
        double thresholdRange = 0.0;
        if (ArkTSUtils::ParseJsDouble(vm, optionLowArg, low)) {
            invert[NUM_0] = std::clamp(low, 0.0, 1.0);
        }
        if (ArkTSUtils::ParseJsDouble(vm, optionHighArg, high)) {
            invert[NUM_1] = std::clamp(high, 0.0, 1.0);
        }
        if (ArkTSUtils::ParseJsDouble(vm, optionThresholdArg, threshold)) {
            invert[NUM_2] = std::clamp(threshold, 0.0, 1.0);
        }
        if (ArkTSUtils::ParseJsDouble(vm, optionThresholdRangeArg, thresholdRange)) {
            invert[NUM_3] = std::clamp(thresholdRange, 0.0, 1.0);
        }
        GetArkUINodeModifiers()->getCommonModifier()->setInvert(nativeNode, invert, NUM_4);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetInvert(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetInvert(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetSepia(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsNumber()) {
        GetArkUINodeModifiers()->getCommonModifier()->setSepia(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetSepia(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetSepia(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetSepia(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetSaturate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsNumber()) {
        GetArkUINodeModifiers()->getCommonModifier()->setSaturate(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetSaturate(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetSaturate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetSaturate(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetColorBlend(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Color color;
    if (!ArkTSUtils::ParseJsColorAlpha(vm, secondArg, color)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetColorBlend(nativeNode);
        } else {
            GetArkUINodeModifiers()->getCommonModifier()->setColorBlend(nativeNode, color.GetValue());
        }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetColorBlend(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetColorBlend(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetGrayscale(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsNumber()) {
        GetArkUINodeModifiers()->getCommonModifier()->setGrayscale(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetGrayscale(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetGrayscale(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetGrayscale(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetContrast(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsNumber()) {
        GetArkUINodeModifiers()->getCommonModifier()->setContrast(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetContrast(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetContrast(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetContrast(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBrightness(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsNumber()) {
        GetArkUINodeModifiers()->getCommonModifier()->setBrightness(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetBrightness(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBrightness(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBrightness(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBlur(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> blurArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> blurOptionArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    double blur = 0.0;
    if (!ArkTSUtils::ParseJsDouble(vm, blurArg, blur)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetBlur(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    BlurOption blurOption;
    if (blurOptionArg->IsArray(vm)) {
        ParseBlurOption(vm, blurOptionArg, blurOption);
    }
    GetArkUINodeModifiers()->getCommonModifier()->setBlur(
        nativeNode, blur, blurOption.grayscale.data(), blurOption.grayscale.size());

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBlur(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBlur(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetLinearGradient(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto angleArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto directionArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto colorsArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto repeatingArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    std::vector<ArkUIInt32orFloat32> values;
    ParseGradientAngle(vm, angleArg, values);
    int32_t direction = static_cast<int32_t>(GradientDirection::NONE);
    ParseJsInt32(vm, directionArg, direction);
    values.push_back({.i32 = static_cast<ArkUI_Float32>(direction)});

    std::vector<ArkUIInt32orFloat32> colors;
    ParseGradientColorStops(vm, colorsArg, colors);
    auto repeating = repeatingArg->IsBoolean() ? repeatingArg->BooleaValue() : false;
    values.push_back({.i32 = static_cast<ArkUI_Float32>(repeating)});
    GetArkUINodeModifiers()->getCommonModifier()->setLinearGradient(nativeNode, values.data(), values.size(),
        colors.data(), colors.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetLinearGradient(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetLinearGradient(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetSweepGradient(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto centerArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto startArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto endArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto rotationArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    auto colorsArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    auto repeatingArg = runtimeCallInfo->GetCallArgRef(NUM_6);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    std::vector<ArkUIInt32orFloat32> values;
    ParseGradientCenter(vm, centerArg, values);
    ParseGradientAngle(vm, startArg, values);
    ParseGradientAngle(vm, endArg, values);
    ParseGradientAngle(vm, rotationArg, values);
    std::vector<ArkUIInt32orFloat32> colors;
    ParseGradientColorStops(vm, colorsArg, colors);
    auto repeating = repeatingArg->IsBoolean() ? repeatingArg->BooleaValue() : false;
    values.push_back({.i32 = static_cast<ArkUI_Int32>(repeating)});
    GetArkUINodeModifiers()->getCommonModifier()->setSweepGradient(nativeNode, values.data(), values.size(),
        colors.data(), colors.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetSweepGradient(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetSweepGradient(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetRadialGradient(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto centerArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto radiusArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto colorsArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto repeatingArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    std::vector<ArkUIInt32orFloat32> values;
    ParseGradientCenter(vm, centerArg, values);
    CalcDimension radius;
    auto hasRadius = ArkTSUtils::ParseJsDimensionVp(vm, radiusArg, radius, false);
    values.push_back({.i32 = static_cast<ArkUI_Int32>(hasRadius)});
    values.push_back({.f32 = static_cast<ArkUI_Float32>(radius.Value())});
    values.push_back({.i32 = static_cast<ArkUI_Int32>(radius.Unit())});
    std::vector<ArkUIInt32orFloat32> colors;
    ParseGradientColorStops(vm, colorsArg, colors);
    auto repeating = repeatingArg->IsBoolean() ? repeatingArg->BooleaValue() : false;
    values.push_back({.i32 = static_cast<ArkUI_Int32>(repeating)});
    GetArkUINodeModifiers()->getCommonModifier()->setRadialGradient(nativeNode, values.data(), values.size(),
        colors.data(), colors.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetRadialGradient(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetRadialGradient(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOverlay(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto valueArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto alignArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto offsetXArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto offsetYArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    auto hasOptionsArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    auto hasOffsetArg = runtimeCallInfo->GetCallArgRef(NUM_6);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    std::optional<std::string> text;
    if (valueArg->IsString(vm)) {
        text = valueArg->ToString(vm)->ToString();
    }
    int32_t align = ALIGNMENT_TOP_LEFT;
    auto hasAlign = ArkTSUtils::ParseJsInteger(vm, alignArg, align);
    std::optional<CalcDimension> offsetX = CalcDimension(0);
    std::optional<CalcDimension> offsetY = CalcDimension(0);
    CalcDimension dimensionX;
    if (ArkTSUtils::ParseJsDimensionVp(vm, offsetXArg, dimensionX, false)) {
        offsetX = dimensionX;
    }
    CalcDimension dimensionY;
    if (ArkTSUtils::ParseJsDimensionVp(vm, offsetYArg, dimensionY, false)) {
        offsetY = dimensionY;
    }
    auto hasOptions = (hasOptionsArg->IsBoolean()) ? hasOptionsArg->ToBoolean(vm)->Value(): false;
    auto hasOffset = (hasOffsetArg->IsBoolean()) ? hasOffsetArg->ToBoolean(vm)->Value(): false;
    std::vector<ArkUI_Float32> options;
    options.push_back(static_cast<ArkUI_Float32>(hasAlign));
    options.push_back(static_cast<ArkUI_Float32>(align));
    options.push_back(static_cast<ArkUI_Float32>(offsetX.has_value()));
    options.push_back(static_cast<ArkUI_Float32>(offsetX.value().Value()));
    options.push_back(static_cast<ArkUI_Float32>(offsetX.value().Unit()));
    options.push_back(static_cast<ArkUI_Float32>(offsetY.has_value()));
    options.push_back(static_cast<ArkUI_Float32>(offsetY.value().Value()));
    options.push_back(static_cast<ArkUI_Float32>(offsetY.value().Unit()));
    options.push_back(static_cast<ArkUI_Float32>(hasOptions));
    options.push_back(static_cast<ArkUI_Float32>(hasOffset));
    auto textPtr = (text.has_value()) ? text.value().c_str() : nullptr;
    GetArkUINodeModifiers()->getCommonModifier()->setOverlay(nativeNode, textPtr, options.data(), options.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOverlay(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetOverlay(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBorderImage(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto argsNumber = runtimeCallInfo->GetArgsNumber();
    if (IsArgsUndefined(runtimeCallInfo, NUM_1, argsNumber - NUM_1)) {
        return panda::JSValueRef::Undefined(vm);
    }
    std::string src;
    std::vector<ArkUIStringAndFloat> options;
    uint8_t bitsets = 0;
    uint32_t offset = NUM_1;
    std::vector<std::optional<CalcDimension>> sliceDimensions;
    ParseBorderImageSlice(runtimeCallInfo, offset, sliceDimensions, bitsets); // use 4 args
    PushDimensionsToVector(options, sliceDimensions);
    ParseBorderImageRepeat(runtimeCallInfo, offset, options, bitsets); // use 1 args
    if (!ParseBorderImageSource(runtimeCallInfo, offset, nativeNode, src, bitsets)) { // use 5 args
        return panda::JSValueRef::Undefined(vm);
    }
    std::vector<std::optional<CalcDimension>> widthDimensions;
    ParseBorderImageWidth(runtimeCallInfo, offset, widthDimensions, bitsets); // use 4 args
    PushDimensionsToVector(options, widthDimensions);
    std::vector<std::optional<CalcDimension>> outsetDimensions;
    ParseBorderImageOutset(runtimeCallInfo, offset, outsetDimensions, bitsets); // use 4 args
    PushDimensionsToVector(options, outsetDimensions);
    ParseBorderImageFill(runtimeCallInfo, offset, options); // use 1 args
    options.push_back(ArkUIStringAndFloat { static_cast<double>(bitsets), nullptr });
    GetArkUINodeModifiers()->getCommonModifier()->setBorderImage(nativeNode,
        src.c_str(), options.data(), options.size());
    ResetCalcDimensions(sliceDimensions);
    ResetCalcDimensions(widthDimensions);
    ResetCalcDimensions(outsetDimensions);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBorderImage(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBorderImage(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetForegroundBlurStyle(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto blurStyleArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto colorModeArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto adaptiveColorArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto scaleArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    auto blurOptionsArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    int32_t blurStyle = -1;
    if (blurStyleArg->IsNumber()) {
        blurStyle = blurStyleArg->Int32Value(vm);
    }
    bool isHasOptions = !(colorModeArg->IsUndefined() && adaptiveColorArg->IsUndefined() && scaleArg->IsUndefined() &&
                          blurOptionsArg->IsUndefined());
    int32_t colorMode = static_cast<int32_t>(ThemeColorMode::SYSTEM);
    int32_t adaptiveColor = static_cast<int32_t>(AdaptiveColor::DEFAULT);
    double scale = 1.0;
    BlurOption blurOption;
    if (isHasOptions) {
        ParseJsInt32(vm, colorModeArg, colorMode);
        ParseJsInt32(vm, adaptiveColorArg, adaptiveColor);
        if (scaleArg->IsNumber()) {
            scale = scaleArg->ToNumber(vm)->Value();
        }
        if (blurOptionsArg->IsArray(vm)) {
            ParseBlurOption(vm, blurOptionsArg, blurOption);
        }
    }
    int32_t intArray[NUM_3];
    intArray[NUM_0] = blurStyle;
    intArray[NUM_1] = colorMode;
    intArray[NUM_2] = adaptiveColor;
    GetArkUINodeModifiers()->getCommonModifier()->setForegroundBlurStyle(
        nativeNode, &intArray, scale, blurOption.grayscale.data(), blurOption.grayscale.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetForegroundBlurStyle(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetForegroundBlurStyle(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetLinearGradientBlur(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto blurRadiusArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto fractionStopsArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto directionArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    double blurRadius = 0.0;
    ParseJsDouble(vm, blurRadiusArg, blurRadius);
    auto direction = static_cast<int32_t>(GradientDirection::BOTTOM);
    if (directionArg->IsInt()) {
        direction = directionArg->Int32Value(vm);
    }
    std::vector<ArkUI_Float32> fractionStops;
    if (fractionStopsArg->IsArray(vm)) {
        auto array = panda::Local<panda::ArrayRef>(fractionStopsArg);
        auto length = array->Length(vm);
        for (uint32_t index = 0; index < length; index++) {
            auto fractionStop = panda::ArrayRef::GetValueAt(vm, array, index);
            ArkUI_Float32 first = 0.0;
            ArkUI_Float32 second = 0.0;
            if (!ParseJsDoublePair(vm, fractionStop, first, second)) {
                continue;
            }
            fractionStops.push_back(first);
            fractionStops.push_back(second);
        }
    }
    GetArkUINodeModifiers()->getCommonModifier()->setLinearGradientBlur(nativeNode, blurRadius, fractionStops.data(),
        fractionStops.size(), direction);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetLinearGradientBlur(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetLinearGradientBlur(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBackgroundBlurStyle(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto blurStyleArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto colorModeArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto adaptiveColorArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto scaleArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    auto blurOptionsArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    int32_t blurStyle = -1;
    if (blurStyleArg->IsNumber()) {
        blurStyle = blurStyleArg->Int32Value(vm);
    }
    bool isHasOptions = !(colorModeArg->IsUndefined() && adaptiveColorArg->IsUndefined() && scaleArg->IsUndefined() &&
                          blurOptionsArg->IsUndefined());
    int32_t colorMode = -1;
    int32_t adaptiveColor = -1;
    double scale = -1.0;
    BlurOption blurOption;
    if (isHasOptions) {
        colorMode = static_cast<int32_t>(ThemeColorMode::SYSTEM);
        ParseJsInt32(vm, colorModeArg, colorMode);
        adaptiveColor = static_cast<int32_t>(AdaptiveColor::DEFAULT);
        ParseJsInt32(vm, adaptiveColorArg, adaptiveColor);
        scale = 1.0;
        if (scaleArg->IsNumber()) {
            scale = scaleArg->ToNumber(vm)->Value();
        }
        if (blurOptionsArg->IsArray(vm)) {
            ParseBlurOption(vm, blurOptionsArg, blurOption);
        }
    }
    int32_t intArray[NUM_3];
    intArray[NUM_0] = blurStyle;
    intArray[NUM_1] = colorMode;
    intArray[NUM_2] = adaptiveColor;
    GetArkUINodeModifiers()->getCommonModifier()->setBackgroundBlurStyle(
        nativeNode, &intArray, scale, blurOption.grayscale.data(), blurOption.grayscale.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundBlurStyle(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBackgroundBlurStyle(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBorder(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    std::vector<ArkUI_Float32> options;
    ParseOuterBorderWidth(runtimeCallInfo, vm, options);         // Border Width args start index from 1
    ParseOuterBorderRadius(runtimeCallInfo, vm, options, NUM_9); // Border Radius args start index

    std::vector<uint32_t> colorAndStyleOptions;
    ParseOuterBorderColor(runtimeCallInfo, vm, colorAndStyleOptions, NUM_5);  // Border Color args start index
    ParseOuterBorderStyle(runtimeCallInfo, vm, colorAndStyleOptions, NUM_13); // Border Style args start index
    
    GetArkUINodeModifiers()->getCommonModifier()->setBorder(
        nativeNode, options.data(), options.size(), colorAndStyleOptions.data(), colorAndStyleOptions.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBorderWithDashParams(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    CommonBridge::SetBorder(runtimeCallInfo);

    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    std::vector<ArkUI_Float32> dashOptions;
    ParseOuterBorderDashParam(runtimeCallInfo, vm, dashOptions, 17); // Border DashGap args start index from 17
    ParseOuterBorderDashParam(runtimeCallInfo, vm, dashOptions, 21); // Border DashWidth args start index from 21

    GetArkUINodeModifiers()->getCommonModifier()->setBorderDashParams(nativeNode, dashOptions.data(),
        dashOptions.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBorder(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBorder(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBackgroundImagePosition(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> xArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> yArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    double valueX = 0.0;
    double valueY = 0.0;
    DimensionUnit typeX = DimensionUnit::PX;
    DimensionUnit typeY = DimensionUnit::PX;
    bool isAlign = false;

    if (secondArg->IsNumber()) {
        int32_t align = secondArg->ToNumber(vm)->Value();
        ParseBackgroundImagePositionAlign(align, valueX, valueY, typeX, typeY);
        isAlign = true;
    } else {
        CalcDimension x(0, DimensionUnit::VP);
        CalcDimension y(0, DimensionUnit::VP);
        ArkTSUtils::ParseJsDimensionVp(vm, xArg, x);
        ArkTSUtils::ParseJsDimensionVp(vm, yArg, y);
        valueX = x.ConvertToPx();
        valueY = y.ConvertToPx();
        if (x.Unit() == DimensionUnit::PERCENT) {
            valueX = x.Value();
            typeX = DimensionUnit::PERCENT;
        }
        if (y.Unit() == DimensionUnit::PERCENT) {
            valueY = y.Value();
            typeY = DimensionUnit::PERCENT;
        }
    }

    ArkUI_Float32 values[SIZE_OF_TWO];
    int32_t types[SIZE_OF_TWO];
    values[NUM_0] = static_cast<ArkUI_Float32>(valueX);
    types[NUM_0] = static_cast<int32_t>(typeX);
    values[NUM_1] = static_cast<ArkUI_Float32>(valueY);
    types[NUM_1] = static_cast<int32_t>(typeY);

    GetArkUINodeModifiers()->getCommonModifier()->setBackgroundImagePosition(nativeNode, values, types, isAlign,
        SIZE_OF_TWO);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundImagePosition(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBackgroundImagePosition(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBackgroundImageResizable(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    std::vector<ArkUIStringAndFloat> options;
    std::vector<std::optional<CalcDimension>> sliceDimensions;
    ParseResizableCalcDimensions(runtimeCallInfo, NUM_1, NUM_4, sliceDimensions, CalcDimension(0.0));
    PushDimensionsToVector(options, sliceDimensions);

    GetArkUINodeModifiers()->getCommonModifier()->setBackgroundImageResizable(nativeNode, options.data());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundImageResizable(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBackgroundImageResizable(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBackgroundImageSize(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> imageSizeArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> widthArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> heightArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    OHOS::Ace::BackgroundImageSizeType typeWidth = OHOS::Ace::BackgroundImageSizeType::AUTO;
    double valueWidth = 0.0;
    OHOS::Ace::BackgroundImageSizeType typeHeight = OHOS::Ace::BackgroundImageSizeType::AUTO;
    double valueHeight = 0.0;

    if (imageSizeArg->IsNumber()) {
        auto sizeType = imageSizeArg->ToNumber(vm)->Value();
        if (sizeType < static_cast<uint32_t>(BackgroundImageSizeType::CONTAIN) ||
            sizeType > static_cast<uint32_t>(BackgroundImageSizeType::FILL)) {
            sizeType = static_cast<uint32_t>(BackgroundImageSizeType::AUTO);
        }
        typeWidth = static_cast<OHOS::Ace::BackgroundImageSizeType>(sizeType);
        typeHeight = static_cast<OHOS::Ace::BackgroundImageSizeType>(sizeType);
    } else {
        CalcDimension width;
        CalcDimension height;
        ArkTSUtils::ParseJsDimensionVp(vm, widthArg, width);
        ArkTSUtils::ParseJsDimensionVp(vm, heightArg, height);

        valueWidth = width.ConvertToPx();
        valueHeight = height.ConvertToPx();
        typeWidth = BackgroundImageSizeType::LENGTH;
        typeHeight = BackgroundImageSizeType::LENGTH;
        if (width.Unit() == DimensionUnit::PERCENT) {
            typeWidth = BackgroundImageSizeType::PERCENT;
            valueWidth = width.Value() * FULL_DIMENSION;
        }
        if (height.Unit() == DimensionUnit::PERCENT) {
            typeHeight = BackgroundImageSizeType::PERCENT;
            valueHeight = height.Value() * FULL_DIMENSION;
        }
    }
    GetArkUINodeModifiers()->getCommonModifier()->setBackgroundImageSize(
        nativeNode, valueWidth, valueHeight, static_cast<int32_t>(typeWidth), static_cast<int32_t>(typeHeight));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundImageSize(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBackgroundImageSize(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBackgroundImage(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> srcArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> repeatArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    std::string src;
    std::string bundle;
    std::string module;
    int32_t repeatIndex = 0;
    RefPtr<PixelMap> pixmap = nullptr;
    if (repeatArg->IsNumber()) {
        repeatIndex = repeatArg->ToNumber(vm)->Value();
    }
    if (srcArg->IsString(vm)) {
        src = srcArg->ToString(vm)->ToString();
        GetArkUINodeModifiers()->getCommonModifier()->setBackgroundImage(
            nativeNode, src.c_str(), bundle.c_str(), module.c_str(), repeatIndex);
    } else if (ArkTSUtils::ParseJsMedia(vm, srcArg, src)) {
        ArkTSUtils::GetJsMediaBundleInfo(vm, srcArg, bundle, module);
        GetArkUINodeModifiers()->getCommonModifier()->setBackgroundImage(
            nativeNode, src.c_str(), bundle.c_str(), module.c_str(), repeatIndex);
    } else {
#if defined(PIXEL_MAP_SUPPORTED)
        if (ArkTSUtils::IsDrawable(vm, srcArg)) {
            pixmap = ArkTSUtils::GetDrawablePixmap(vm, srcArg);
        } else {
            pixmap = ArkTSUtils::CreatePixelMapFromNapiValue(vm, srcArg);
        }
#endif
        if (pixmap) {
            auto pixelMapSharedPtr = pixmap->GetPixelMapSharedPtr();
            GetArkUINodeModifiers()->getCommonModifier()->setBackgroundImagePixelMapByPixelMapPtr(
                nativeNode, &pixelMapSharedPtr, repeatIndex);
        } else {
            GetArkUINodeModifiers()->getCommonModifier()->resetBackgroundImage(nativeNode);
        }
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundImage(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBackgroundImage(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetTranslate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> xArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> yArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> zArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (!xArg->IsNumber() && !xArg->IsString(vm) && !yArg->IsNumber() && !yArg->IsString(vm) && !zArg->IsNumber() &&
        !zArg->IsString(vm)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetTranslate(nativeNode);
    } else {
        auto translateX = CalcDimension(0.0);
        auto translateY = CalcDimension(0.0);
        auto translateZ = CalcDimension(0.0);
        bool hasX = ParseAxisDimensionVp(vm, xArg, translateX, true);
        bool hasY = ParseAxisDimensionVp(vm, yArg, translateY, true);
        bool hasZ = ParseAxisDimensionVp(vm, zArg, translateZ, true);
        if (hasX || hasY || hasZ) {
            uint32_t size = SIZE_OF_THREE;
            ArkUI_Float32 values[size];
            int units[size];

            values[NUM_0] = static_cast<ArkUI_Float32>(translateX.Value());
            units[NUM_0] = static_cast<int>(translateX.Unit());
            values[NUM_1] = static_cast<ArkUI_Float32>(translateY.Value());
            units[NUM_1] = static_cast<int>(translateY.Unit());
            values[NUM_2] = static_cast<ArkUI_Float32>(translateZ.Value());
            units[NUM_2] = static_cast<int>(translateZ.Unit());
            GetArkUINodeModifiers()->getCommonModifier()->setTranslate(nativeNode, values, units, size);
        } else {
            GetArkUINodeModifiers()->getCommonModifier()->resetTranslate(nativeNode);
        }
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetTranslate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetTranslate(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetScale(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> xArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> yArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> zArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> centerXArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    Local<JSValueRef> centerYArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (xArg->IsNumber() || yArg->IsNumber() || zArg->IsNumber()) {
        auto scaleX = 1.0f;
        auto scaleY = 1.0f;
        auto scaleZ = 1.0f;

        CalcDimension centerX = 0.5_pct;
        CalcDimension centerY = 0.5_pct;

        if (xArg->IsNumber()) {
            scaleX = xArg->ToNumber(vm)->Value();
        }
        if (yArg->IsNumber()) {
            scaleY = yArg->ToNumber(vm)->Value();
        }
        if (zArg->IsNumber()) {
            scaleZ = zArg->ToNumber(vm)->Value();
        }
        if (centerXArg->IsNumber() || centerXArg->IsString(vm)) {
            ParseAxisDimensionVp(vm, centerXArg, centerX, true);
        }
        if (centerYArg->IsNumber() || centerYArg->IsString(vm)) {
            ParseAxisDimensionVp(vm, centerYArg, centerY, true);
        }

        ArkUI_Float32 values[SIZE_OF_FIVE];
        int units[SIZE_OF_TWO];

        values[NUM_0] = static_cast<ArkUI_Float32>(scaleX);
        values[NUM_1] = static_cast<ArkUI_Float32>(scaleY);
        values[NUM_2] = static_cast<ArkUI_Float32>(scaleZ);
        values[NUM_3] = static_cast<ArkUI_Float32>(centerX.Value());
        values[NUM_4] = static_cast<ArkUI_Float32>(centerY.Value());
        units[NUM_0] = static_cast<int>(centerX.Unit());
        units[NUM_1] = static_cast<int>(centerY.Unit());
        GetArkUINodeModifiers()->getCommonModifier()->setScale(nativeNode, values, SIZE_OF_FIVE, units, SIZE_OF_TWO);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetScale(nativeNode);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetScale(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetScale(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetRotate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    ArkUI_Float32 values[SIZE_OF_EIGHT];
    int units[SIZE_OF_THREE];

    if (ParseRotate(runtimeCallInfo, values, units, SIZE_OF_EIGHT, SIZE_OF_THREE)) {
        GetArkUINodeModifiers()->getCommonModifier()->setRotate(
            nativeNode, values, SIZE_OF_EIGHT, units, SIZE_OF_THREE);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetRotate(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetRotate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetRotate(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetGeometryTransition(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> idArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> optionsArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (idArg->IsUndefined() || idArg->IsNull() || !idArg->IsString(vm)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetGeometryTransition(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    std::string id = idArg->ToString(vm)->ToString();
    bool options = false;
    if (optionsArg->IsBoolean()) {
        options = optionsArg->ToBoolean(vm)->Value();
    }
    GetArkUINodeModifiers()->getCommonModifier()->setGeometryTransition(
        nativeNode, id.c_str(), static_cast<int32_t>(options));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetGeometryTransition(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetGeometryTransition(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetClip(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto *frameNode = reinterpret_cast<FrameNode *>(nativeNode);
    ViewAbstract::SetClipEdge(frameNode, false);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetClip(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto *frameNode = reinterpret_cast<FrameNode *>(nativeNode);

    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    if (info[NUM_1]->IsUndefined()) {
        ViewAbstract::SetClipEdge(frameNode, false);
        return panda::JSValueRef::Undefined(vm);
    }
    if (info[NUM_1]->IsObject()) {
        Framework::JSShapeAbstract *clipShape =
            Framework::JSRef<Framework::JSObject>::Cast(info[NUM_1])->Unwrap<Framework::JSShapeAbstract>();
        if (clipShape == nullptr) {
            return panda::JSValueRef::Undefined(vm);
        }
        ViewAbstract::SetClipShape(frameNode, clipShape->GetBasicShape());
    } else if (info[NUM_1]->IsBoolean()) {
        ViewAbstract::SetClipEdge(frameNode, info[NUM_1]->ToBoolean());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetClipShape(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto *frameNode = reinterpret_cast<FrameNode *>(nativeNode);

    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    if (info[NUM_1]->IsObject()) {
        Framework::JSShapeAbstract *clipShape =
            Framework::JSRef<Framework::JSObject>::Cast(info[NUM_1])->Unwrap<Framework::JSShapeAbstract>();
        if (clipShape == nullptr) {
            return panda::JSValueRef::Undefined(vm);
        }
        ViewAbstract::SetClipShape(frameNode, clipShape->GetBasicShape());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetClipShape(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto *frameNode = reinterpret_cast<FrameNode *>(nativeNode);
    ViewAbstract::SetClipEdge(frameNode, false);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetPixelStretchEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto topArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto rightArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto bottomArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto leftArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CalcDimension left;
    ArkTSUtils::ParseJsDimensionVp(vm, leftArg, left);
    CalcDimension right;
    ArkTSUtils::ParseJsDimensionVp(vm, rightArg, right);
    CalcDimension top;
    ArkTSUtils::ParseJsDimensionVp(vm, topArg, top);
    CalcDimension bottom;
    ArkTSUtils::ParseJsDimensionVp(vm, bottomArg, bottom);
    ArkUI_Float32 values[] = { left.Value(), top.Value(), right.Value(), bottom.Value() };
    int units[] = { static_cast<int>(left.Unit()), static_cast<int>(top.Unit()), static_cast<int>(right.Unit()),
                    static_cast<int>(bottom.Unit()) };
    GetArkUINodeModifiers()->getCommonModifier()->setPixelStretchEffect(nativeNode, values, units,
        (sizeof(values) / sizeof(values[NUM_0])));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetPixelStretchEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetPixelStretchEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetLightUpEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto radio = 1.0;
    if (secondArg->IsNumber()) {
        radio = secondArg->ToNumber(vm)->Value();
    }
    GetArkUINodeModifiers()->getCommonModifier()->setLightUpEffect(nativeNode, radio);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetLightUpEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetLightUpEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetSphericalEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto radio = 0.0;
    if (secondArg->IsNumber()) {
        radio = secondArg->ToNumber(vm)->Value();
    }
    GetArkUINodeModifiers()->getCommonModifier()->setSphericalEffect(nativeNode, radio);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetSphericalEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetSphericalEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetRenderGroup(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto isRenderGroup = false;
    if (secondArg->IsBoolean()) {
        isRenderGroup = secondArg->ToBoolean(vm)->Value();
    }
    GetArkUINodeModifiers()->getCommonModifier()->setRenderGroup(nativeNode, isRenderGroup);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetRenderGroup(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetRenderGroup(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetRenderFit(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto fitModeArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto renderFit = static_cast<int32_t>(RenderFit::TOP_LEFT);
    if (fitModeArg->IsNumber()) {
        renderFit = fitModeArg->Int32Value(vm);
    }
    GetArkUINodeModifiers()->getCommonModifier()->setRenderFit(nativeNode, renderFit);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetRenderFit(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetRenderFit(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetUseEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto useEffect = false;
    if (secondArg->IsBoolean()) {
        useEffect = secondArg->ToBoolean(vm)->Value();
    }
    GetArkUINodeModifiers()->getCommonModifier()->setUseEffect(nativeNode, useEffect);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetUseEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetUseEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetForegroundColor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto colorArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (colorArg->IsString(vm)) {
        std::string colorStr = colorArg->ToString(vm)->ToString();
        colorStr.erase(std::remove(colorStr.begin(), colorStr.end(), ' '), colorStr.end());
        std::transform(colorStr.begin(), colorStr.end(), colorStr.begin(), ::tolower);
        if (colorStr.compare("invert") == 0) {
            auto strategy = static_cast<uint32_t>(ForegroundColorStrategy::INVERT);
            GetArkUINodeModifiers()->getCommonModifier()->setForegroundColor(nativeNode, false, strategy);
            return panda::JSValueRef::Undefined(vm);
        }
    }
    Color foregroundColor;
    if (!ArkTSUtils::ParseJsColorAlpha(vm, colorArg, foregroundColor)) {
        return panda::JSValueRef::Undefined(vm);
    }
    GetArkUINodeModifiers()->getCommonModifier()->setForegroundColor(nativeNode, true, foregroundColor.GetValue());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetForegroundColor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetForegroundColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetMotionPath(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto pathArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto fromArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto toArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto rotatableArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    std::string pathStringValue;
    if (pathArg->IsString(vm)) {
        pathStringValue = pathArg->ToString(vm)->ToString();
    }
    float fromValue = (fromArg->IsNumber()) ? fromArg->ToNumber(vm)->Value() : 0.0f;
    float toValue = (toArg->IsNumber()) ? toArg->ToNumber(vm)->Value() : 1.0f;
    if (fromValue > 1.0f || fromValue < 0.0f) {
        fromValue = 0.0f;
    }
    if (toValue > 1.0f || toValue < 0.0f) {
        toValue = 1.0f;
    } else if (toValue < fromValue) {
        toValue = fromValue;
    }
    bool rotatableValue = (rotatableArg->IsBoolean()) ? rotatableArg->ToBoolean(vm)->Value() : false;
    GetArkUINodeModifiers()->getCommonModifier()->setMotionPath(nativeNode, pathStringValue.c_str(), fromValue,
        toValue, rotatableValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetMotionPath(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetMotionPath(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}


ArkUINativeModuleValue CommonBridge::SetMotionBlur(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> radiusArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> xArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> yArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    double radius = 0.0;
    if (!ArkTSUtils::ParseJsDouble(vm, radiusArg, radius) || LessNotEqual(radius, 0.0)) {
        radius = 0.0;
    }
    double x = 0.0;
    double y = 0.0;
    if (!ArkTSUtils::ParseJsDouble(vm, xArg, x) || LessNotEqual(x, 0.0)) {
        x = 0.0;
    }
    if (!ArkTSUtils::ParseJsDouble(vm, yArg, y) || LessNotEqual(y, 0.0)) {
        y = 0.0;
    }
    x = std::clamp(x, 0.0, 1.0);
    y = std::clamp(y, 0.0, 1.0);
    GetArkUINodeModifiers()->getCommonModifier()->setMotionBlur(nativeNode, radius, x, y);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetMotionBlur(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetMotionBlur(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetGroupDefaultFocus(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsBoolean()) {
        bool groupDefaultFocus = secondArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setGroupDefaultFocus(nativeNode, groupDefaultFocus);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetGroupDefaultFocus(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetGroupDefaultFocus(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetGroupDefaultFocus(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetFocusOnTouch(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsBoolean()) {
        bool focusOnTouch = secondArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setFocusOnTouch(nativeNode, focusOnTouch);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetFocusOnTouch(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetFocusOnTouch(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetFocusOnTouch(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetFocusable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsBoolean()) {
        bool focusable = secondArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setFocusable(nativeNode, focusable);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetFocusable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetFocusable(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetTouchable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsBoolean()) {
        bool touchable = secondArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setTouchable(nativeNode, touchable);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetTouchable(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetTouchable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetTouchable(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetDefaultFocus(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsBoolean()) {
        bool defaultFocus = secondArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setDefaultFocus(nativeNode, defaultFocus);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetDefaultFocus(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetDefaultFocus(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetDefaultFocus(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetDisplayPriority(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsNumber()) {
        double value = secondArg->ToNumber(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setDisplayPriority(nativeNode, value);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetDisplayPriority(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetDisplayPriority(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetDisplayPriority(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAccessibilityLevel(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsString(vm)) {
        std::string stringValue = secondArg->ToString(vm)->ToString();
        GetArkUINodeModifiers()->getCommonModifier()->setAccessibilityLevel(nativeNode, stringValue.c_str());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetAccessibilityLevel(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAccessibilityLevel(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetAccessibilityLevel(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAccessibilityDescription(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsString(vm)) {
        std::string stringValue = secondArg->ToString(vm)->ToString();
        GetArkUINodeModifiers()->getCommonModifier()->setAccessibilityDescription(nativeNode, stringValue.c_str());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetAccessibilityDescription(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAccessibilityDescription(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetAccessibilityDescription(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOffset(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    bool useEdges = runtimeCallInfo->GetCallArgRef(NUM_1)->ToBoolean(vm)->Value();
    std::vector<ArkUIStringAndFloat> options;
    std::vector<std::optional<CalcDimension>> edges;

    if (useEdges) {
        ParseCalcDimensions(runtimeCallInfo, NUM_2, NUM_4, edges, CalcDimension(0.0));
        PushDimensionsToVector(options, edges);
        GetArkUINodeModifiers()->getCommonModifier()->setOffsetEdges(nativeNode, useEdges, options.data());
    } else {
        ParseCalcDimensions(runtimeCallInfo, NUM_2, NUM_2, edges, CalcDimension(0.0));
        PushDimensionsToVector(options, edges);
        GetArkUINodeModifiers()->getCommonModifier()->setOffsetEdges(nativeNode, useEdges, options.data());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOffset(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    GetArkUINodeModifiers()->getCommonModifier()->resetOffset(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

void ParsePadding(const EcmaVM* vm, const Local<JSValueRef>& value, CalcDimension& dimen, ArkUISizeType& result)
{
    if (ArkTSUtils::ParseJsDimensionVp(vm, value, dimen)) {
        if (LessOrEqual(dimen.Value(), 0.0)) {
            dimen.SetValue(0.0);
            dimen.SetUnit(DimensionUnit::VP);
        }
        result.unit = static_cast<int8_t>(dimen.Unit());
        if (dimen.CalcValue() != "") {
            result.string = dimen.CalcValue().c_str();
        } else {
            result.value = dimen.Value();
        }
    }
}

ArkUINativeModuleValue CommonBridge::SetPadding(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> forthArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> fifthArg = runtimeCallInfo->GetCallArgRef(NUM_4);

    struct ArkUISizeType top = { 0.0, static_cast<int8_t>(DimensionUnit::VP) };
    struct ArkUISizeType right = { 0.0, static_cast<int8_t>(DimensionUnit::VP) };
    struct ArkUISizeType bottom = { 0.0, static_cast<int8_t>(DimensionUnit::VP) };
    struct ArkUISizeType left = { 0.0, static_cast<int8_t>(DimensionUnit::VP) };

    CalcDimension topDimen(0, DimensionUnit::VP);
    CalcDimension rightDimen(0, DimensionUnit::VP);
    CalcDimension bottomDimen(0, DimensionUnit::VP);
    CalcDimension leftDimen(0, DimensionUnit::VP);
    ParsePadding(vm, secondArg, topDimen, top);
    ParsePadding(vm, thirdArg, rightDimen, right);
    ParsePadding(vm, forthArg, bottomDimen, bottom);
    ParsePadding(vm, fifthArg, leftDimen, left);
    GetArkUINodeModifiers()->getCommonModifier()->setPadding(nativeNode, &top, &right, &bottom, &left);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetPadding(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetPadding(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetMargin(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> forthArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> fifthArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    ArkUISizeType top = { 0.0, static_cast<int8_t>(DimensionUnit::VP), nullptr };
    ArkUISizeType right = { 0.0, static_cast<int8_t>(DimensionUnit::VP), nullptr };
    ArkUISizeType bottom = { 0.0, static_cast<int8_t>(DimensionUnit::VP), nullptr };
    ArkUISizeType left = { 0.0, static_cast<int8_t>(DimensionUnit::VP), nullptr };
    CalcDimension topDimen(0, DimensionUnit::VP);
    if (ArkTSUtils::ParseJsDimensionVp(vm, secondArg, topDimen)) {
        top.unit = static_cast<int8_t>(topDimen.Unit());
        if (topDimen.CalcValue() != "") {
            top.string = topDimen.CalcValue().c_str();
        } else {
            top.value = topDimen.Value();
        }
    }
    CalcDimension rightDimen(0, DimensionUnit::VP);
    if (ArkTSUtils::ParseJsDimensionVp(vm, thirdArg, rightDimen)) {
        right.unit = static_cast<int8_t>(rightDimen.Unit());
        if (rightDimen.CalcValue() != "") {
            right.string = rightDimen.CalcValue().c_str();
        } else {
            right.value = rightDimen.Value();
        }
    }
    CalcDimension bottomDimen(0, DimensionUnit::VP);
    if (ArkTSUtils::ParseJsDimensionVp(vm, forthArg, bottomDimen)) {
        bottom.unit = static_cast<int8_t>(bottomDimen.Unit());
        if (bottomDimen.CalcValue() != "") {
            bottom.string = bottomDimen.CalcValue().c_str();
        } else {
            bottom.value = bottomDimen.Value();
        }
    }
    CalcDimension leftDimen(0, DimensionUnit::VP);
    if (ArkTSUtils::ParseJsDimensionVp(vm, fifthArg, leftDimen)) {
        left.unit = static_cast<int8_t>(leftDimen.Unit());
        if (leftDimen.CalcValue() != "") {
            left.string = leftDimen.CalcValue().c_str();
        } else {
            left.value = leftDimen.Value();
        }
    }
    GetArkUINodeModifiers()->getCommonModifier()->setMargin(nativeNode, &top, &right, &bottom, &left);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetMargin(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetMargin(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetMarkAnchor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> xArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> yArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    CalcDimension x(0.0, DimensionUnit::VP);
    CalcDimension y(0.0, DimensionUnit::VP);
    bool hasX = ArkTSUtils::ParseJsDimensionNG(vm, xArg, x, DimensionUnit::VP);
    bool hasY = ArkTSUtils::ParseJsDimensionNG(vm, yArg, y, DimensionUnit::VP);
    if (hasX || hasY) {
        GetArkUINodeModifiers()->getCommonModifier()->setMarkAnchor(nativeNode, x.Value(),
            static_cast<int32_t>(x.Unit()), y.Value(), static_cast<int32_t>(y.Unit()));
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetMarkAnchor(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetMarkAnchor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetMarkAnchor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetVisibility(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    int32_t value = 0;
    if (secondArg->IsNumber()) {
        value = secondArg->Int32Value(vm);
        if (value<NUM_0 || value>NUM_2) {
            value = 0;
        }
    }
    GetArkUINodeModifiers()->getCommonModifier()->setVisibility(nativeNode, value);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetVisibility(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetVisibility(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAccessibilityText(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsString(vm)) {
        std::string stringValue = secondArg->ToString(vm)->ToString();
        GetArkUINodeModifiers()->getCommonModifier()->setAccessibilityText(nativeNode, stringValue.c_str());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetAccessibilityText(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAccessibilityText(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetAccessibilityText(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetConstraintSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> forthArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> fifthArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CalcDimension minWidth;
    CalcDimension maxWidth;
    CalcDimension minHeight;
    CalcDimension maxHeight;
    struct ArkUISizeType minWidthValue = {0.0, 0};
    struct ArkUISizeType maxWidthValue = {0.0, 0};
    struct ArkUISizeType minHeightValue = {0.0, 0};
    struct ArkUISizeType maxHeightValue = {0.0, 0};

    bool version10OrLarger = Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN);
    if (ArkTSUtils::ParseJsDimensionVp(vm, secondArg, minWidth, false)) {
        if (minWidth.CalcValue() != "") {
            minWidthValue.string = minWidth.CalcValue().c_str();
        } else {
            minWidthValue.value = minWidth.Value();
            minWidthValue.unit = static_cast<int8_t>(minWidth.Unit());
        }
        GetArkUINodeModifiers()->getCommonModifier()->setMinWidth(nativeNode, &minWidthValue);
    } else if (version10OrLarger) {
        GetArkUINodeModifiers()->getCommonModifier()->resetMinWidth(nativeNode);
    }

    if (ArkTSUtils::ParseJsDimensionVp(vm, thirdArg, maxWidth, false)) {
        if (maxWidth.CalcValue() != "") {
            maxWidthValue.string = maxWidth.CalcValue().c_str();
        } else {
            maxWidthValue.value = maxWidth.Value();
            maxWidthValue.unit = static_cast<int8_t>(maxWidth.Unit());
        }
        GetArkUINodeModifiers()->getCommonModifier()->setMaxWidth(nativeNode, &maxWidthValue);
    } else if (version10OrLarger) {
        GetArkUINodeModifiers()->getCommonModifier()->resetMaxWidth(nativeNode);
    }

    if (ArkTSUtils::ParseJsDimensionVp(vm, forthArg, minHeight, false)) {
        if (minHeight.CalcValue() != "") {
            minHeightValue.string = minHeight.CalcValue().c_str();
        } else {
            minHeightValue.value = minHeight.Value();
            minHeightValue.unit = static_cast<int8_t>(minHeight.Unit());
        }
        GetArkUINodeModifiers()->getCommonModifier()->setMinHeight(nativeNode, &minHeightValue);
    } else if (version10OrLarger) {
        GetArkUINodeModifiers()->getCommonModifier()->resetMinHeight(nativeNode);
    }

    if (ArkTSUtils::ParseJsDimensionVp(vm, fifthArg, maxHeight, false)) {
        if (maxHeight.CalcValue() != "") {
            maxHeightValue.string = maxHeight.CalcValue().c_str();
        } else {
            maxHeightValue.value = maxHeight.Value();
            maxHeightValue.unit = static_cast<int8_t>(maxHeight.Unit());
        }
        GetArkUINodeModifiers()->getCommonModifier()->setMaxHeight(nativeNode, &maxHeightValue);
    } else if (version10OrLarger) {
        GetArkUINodeModifiers()->getCommonModifier()->resetMaxHeight(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetConstraintSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetMaxHeight(nativeNode);
    GetArkUINodeModifiers()->getCommonModifier()->resetMaxWidth(nativeNode);
    GetArkUINodeModifiers()->getCommonModifier()->resetMinHeight(nativeNode);
    GetArkUINodeModifiers()->getCommonModifier()->resetMinWidth(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetDirection(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    std::string dir;
    int32_t direction = NUM_3;
    if (secondArg->IsString(vm)) {
        dir = secondArg->ToString(vm)->ToString();
        if (dir == "Ltr") {
            direction = NUM_0;
        } else if (dir == "Rtl") {
            direction = NUM_1;
        } else if (dir == "Auto") {
            direction = NUM_3;
        } else if (dir == "undefined" && Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
            direction = NUM_3;
        }
    }
    GetArkUINodeModifiers()->getCommonModifier()->setDirection(nativeNode, direction);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetDirection(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetDirection(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetLayoutWeight(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    float layoutWeight = 0.0f;
    if (secondArg->IsNumber()) {
        if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            layoutWeight = secondArg->ToNumber(vm)->Value();
        } else {
            layoutWeight = secondArg->Int32Value(vm);
        }
    } else if (secondArg->IsString(vm)) {
        if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            layoutWeight = StringUtils::StringToFloat(secondArg->ToString(vm)->ToString());
        } else {
            layoutWeight = StringUtils::StringToInt(secondArg->ToString(vm)->ToString());
        }
    }
    GetArkUINodeModifiers()->getCommonModifier()->setLayoutWeight(nativeNode, layoutWeight);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetLayoutWeight(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetLayoutWeight(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CalcDimension wVal(0.0, DimensionUnit::VP);
    CalcDimension hVal(0.0, DimensionUnit::VP);
    ParseCalcDimension(vm, nativeNode, secondArg, wVal, true);
    ParseCalcDimension(vm, nativeNode, thirdArg, hVal, false);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetWidth(nativeNode);
    GetArkUINodeModifiers()->getCommonModifier()->resetHeight(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAlignSelf(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    if (secondArg->IsNumber() && secondArg->ToNumber(vm)->Value() >= 0 &&
        secondArg->ToNumber(vm)->Value() <= MAX_ALIGN_VALUE) {
        GetArkUINodeModifiers()->getCommonModifier()->setAlignSelf(nativeNode, secondArg->Int32Value(vm));
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetAlignSelf(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAlignSelf(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetAlignSelf(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAspectRatio(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    if (secondArg->IsNumber()) {
        double value = secondArg->ToNumber(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setAspectRatio(nativeNode, value);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetAspectRatio(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAspectRatio(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetAspectRatio(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetFlexGrow(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    if (secondArg->IsNumber()) {
        double value = secondArg->ToNumber(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setFlexGrow(nativeNode, value);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetFlexGrow(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetFlexGrow(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetFlexGrow(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetFlexShrink(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    if (secondArg->IsNumber()) {
        double value = secondArg->ToNumber(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setFlexShrink(nativeNode, value);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetFlexShrink(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetFlexShrink(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetFlexShrink(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetGridOffset(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> offsetArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    int32_t offset = 0;
    if (offsetArg->IsNumber()) {
        offset = offsetArg->Int32Value(vm);
        GetArkUINodeModifiers()->getCommonModifier()->setGridOffset(nativeNode, offset);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetGridOffset(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetGridOffset(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetGridOffset(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetGridSpan(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    int32_t value = 0;
    if (secondArg->IsNumber()) {
        value = secondArg->Int32Value(vm);
        GetArkUINodeModifiers()->getCommonModifier()->setGridSpan(nativeNode, value);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetGridSpan(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetGridSpan(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetGridSpan(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetExpandSafeArea(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    std::string typeCppStr = "";
    std::string edgesCppStr = "";
    uint32_t safeAreaType = NG::SAFE_AREA_TYPE_NONE;
    uint32_t safeAreaEdge = NG::SAFE_AREA_EDGE_NONE;
    if (secondArg->IsString(vm)) {
        typeCppStr = secondArg->ToString(vm)->ToString();
        safeAreaType = ParseStrToUint(typeCppStr);
    } else {
        safeAreaType = NG::SAFE_AREA_TYPE_ALL;
    }

    if (thirdArg->IsString(vm)) {
        edgesCppStr = thirdArg->ToString(vm)->ToString();
        safeAreaEdge = ParseStrToUint(edgesCppStr);
    } else {
        safeAreaEdge = NG::SAFE_AREA_EDGE_ALL;
    }
    GetArkUINodeModifiers()->getCommonModifier()->setExpandSafeArea(nativeNode, safeAreaType, safeAreaEdge);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetExpandSafeArea(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetExpandSafeArea(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAlignRules(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> leftArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> middleArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> rightArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> topArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    Local<JSValueRef> centerArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    Local<JSValueRef> bottomArg = runtimeCallInfo->GetCallArgRef(NUM_6);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    auto anchors = std::make_unique<std::string []>(ALIGN_RULES_NUM);
    auto direction = std::make_unique<ArkUI_Int32 []>(ALIGN_RULES_NUM);
    for (int32_t i = 0; i < ALIGN_RULES_NUM; i++) {
        anchors[i] = "";
        direction[i] = ALIGN_DIRECTION_DEFAULT;
    }
    bool leftParseResult = ParseJsAlignRule(vm, leftArg, anchors[0], direction[0]);
    bool middleParseResult = ParseJsAlignRule(vm, middleArg, anchors[1], direction[1]);
    bool rightParseResult = ParseJsAlignRule(vm, rightArg, anchors[2], direction[2]);
    bool topParseResult = ParseJsAlignRule(vm, topArg, anchors[3], direction[3]);
    bool centerParseResult = ParseJsAlignRule(vm, centerArg, anchors[4], direction[4]);
    bool bottomParseResult = ParseJsAlignRule(vm, bottomArg, anchors[5], direction[5]);
    if (!leftParseResult && !middleParseResult && !rightParseResult && !topParseResult && !centerParseResult &&
        !bottomParseResult) {
        GetArkUINodeModifiers()->getCommonModifier()->resetAlignRules(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    auto realAnchors = std::make_unique<char* []>(ALIGN_RULES_NUM);
    for (int32_t i = 0; i < ALIGN_RULES_NUM; i++) {
        realAnchors[i] = const_cast<char*>(anchors[i].c_str());
    }
    GetArkUINodeModifiers()->getCommonModifier()->setAlignRules(nativeNode, realAnchors.get(), direction.get(),
        ALIGN_RULES_NUM);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAlignRules(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetAlignRules(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetFlexBasis(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    struct ArkUIStringAndFloat flexBasis { 0.0, nullptr};
    std::string tempValueStr = "";
    if (secondArg->IsNumber()) {
        flexBasis.value = secondArg->ToNumber(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setFlexBasis(nativeNode, &flexBasis);
    } else if (secondArg->IsString(vm)) {
        tempValueStr = secondArg->ToString(vm)->ToString();
        flexBasis.valueStr = tempValueStr.c_str();
        GetArkUINodeModifiers()->getCommonModifier()->setFlexBasis(nativeNode, &flexBasis);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetFlexBasis(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetFlexBasis(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetFlexBasis(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAllowDrop(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsUndefined()) {
        GetArkUINodeModifiers()->getCommonModifier()->resetAllowDrop(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    Local<panda::ArrayRef> allowDropArray = static_cast<Local<panda::ArrayRef>>(secondArg);
    auto length = allowDropArray->Length(vm);
    if (length <= 0) {
        GetArkUINodeModifiers()->getCommonModifier()->resetAllowDrop(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    std::vector<std::string> keepStr(length);
    std::vector<const char*> strList;
    for (size_t i = 0; i < length; i++) {
        Local<JSValueRef> objValue = allowDropArray->GetValueAt(vm, secondArg, i);
        keepStr[i] = objValue->ToString(vm)->ToString();
        strList.push_back(keepStr[i].c_str());
    }
    GetArkUINodeModifiers()->getCommonModifier()->setAllowDrop(nativeNode, strList.data(), strList.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAllowDrop(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetAllowDrop(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetId(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsString(vm)) {
        std::string stringValue = secondArg->ToString(vm)->ToString();
        GetArkUINodeModifiers()->getCommonModifier()->setId(nativeNode, stringValue.c_str());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetId(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetId(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetId(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetKey(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsString(vm)) {
        std::string stringValue = secondArg->ToString(vm)->ToString();
        GetArkUINodeModifiers()->getCommonModifier()->setKey(nativeNode, stringValue.c_str());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetKey(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetKey(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetKey(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetRestoreId(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsNumber()) {
        uint32_t value = secondArg->Uint32Value(vm);
        GetArkUINodeModifiers()->getCommonModifier()->setRestoreId(nativeNode, value);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetRestoreId(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetRestoreId(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetRestoreId(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetTabIndex(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsNumber()) {
        int32_t index = secondArg->Int32Value(vm);
        GetArkUINodeModifiers()->getCommonModifier()->setTabIndex(nativeNode, index);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetTabIndex(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetTabIndex(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetTabIndex(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetObscured(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    if (secondArg->IsUndefined() || !secondArg->IsArray(vm)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetObscured(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    Local<panda::ArrayRef> transArray = static_cast<Local<panda::ArrayRef>>(secondArg);
    auto length = transArray->Length(vm);
    int32_t reasonArray[length];

    for (size_t i = 0; i < length; i++) {
        Local<JSValueRef> value = transArray->GetValueAt(vm, secondArg, i);
        reasonArray[i] = value->Int32Value(vm);
    }
    GetArkUINodeModifiers()->getCommonModifier()->setObscured(nativeNode, reasonArray, length);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetObscured(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetObscured(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetForegroundEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> radiusArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    CalcDimension radius;
    if (!ArkTSUtils::ParseJsDimensionVp(vm, radiusArg, radius) || LessNotEqual(radius.Value(), 0.0f)) {
        radius.SetValue(0.0f);
    }
    ArkUI_Float32 radiusArk = static_cast<ArkUI_Int32>(radius.Value());
    GetArkUINodeModifiers()->getCommonModifier()->setForegroundEffect(nativeNode, radiusArk);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetForegroundEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetForegroundEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBackgroundEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);     // 0:index of parameter frameNode
    Local<JSValueRef> radiusArg = runtimeCallInfo->GetCallArgRef(1);        // 1:index of parameter radius
    Local<JSValueRef> saturationArg = runtimeCallInfo->GetCallArgRef(2);    // 2:index of parameter saturation
    Local<JSValueRef> brightnessArg = runtimeCallInfo->GetCallArgRef(3);    // 3:index of parameter brightness
    Local<JSValueRef> colorArg = runtimeCallInfo->GetCallArgRef(4);         // 4:index of parameter color
    Local<JSValueRef> adaptiveColorArg = runtimeCallInfo->GetCallArgRef(5); // 5:index of parameter adaptiveColor
    Local<JSValueRef> blurOptionsArg = runtimeCallInfo->GetCallArgRef(6);   // 6:index of parameter blurOptions
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    CalcDimension radius;
    if (!ArkTSUtils::ParseJsDimensionVp(vm, radiusArg, radius) || LessNotEqual(radius.Value(), 0.0f)) {
        radius.SetValue(0.0f);
    }
    ArkUI_Float32 saturation = 1.0f;
    if (saturationArg->IsNumber()) {
        saturation = saturationArg->ToNumber(vm)->Value();
        saturation = (saturation > 0.0f || NearZero(saturation)) ? saturation : 1.0f;
    }
    ArkUI_Float32 brightness = 1.0f;
    if (brightnessArg->IsNumber()) {
        brightness = brightnessArg->ToNumber(vm)->Value();
        brightness = (brightness > 0.0f || NearZero(brightness)) ? brightness : 1.0f;
    }
    Color color = Color::TRANSPARENT;
    if (!ArkTSUtils::ParseJsColor(vm, colorArg, color)) {
        color.SetValue(Color::TRANSPARENT.GetValue());
    }
    auto adaptiveColorValue = static_cast<int32_t>(AdaptiveColor::DEFAULT);
    auto adaptiveColor = AdaptiveColor::DEFAULT;
    if (adaptiveColorArg->IsNumber()) {
        adaptiveColorValue = adaptiveColorArg->Int32Value(vm);
        if (adaptiveColorValue >= static_cast<int32_t>(AdaptiveColor::DEFAULT) &&
            adaptiveColorValue <= static_cast<int32_t>(AdaptiveColor::AVERAGE)) {
            adaptiveColor = static_cast<AdaptiveColor>(adaptiveColorValue);
        }
    }
    BlurOption blurOption;
    if (blurOptionsArg->IsArray(vm)) {
        ParseBlurOption(vm, blurOptionsArg, blurOption);
    }

    GetArkUINodeModifiers()->getCommonModifier()->setBackgroundEffect(
        nativeNode, static_cast<ArkUI_Float32>(radius.Value()),
        saturation, brightness, color.GetValue(), static_cast<ArkUI_Int32>(adaptiveColor),
        blurOption.grayscale.data(), blurOption.grayscale.size());

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBackgroundEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBackgroundBrightness(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);     // 0:index of parameter frameNode
    Local<JSValueRef> ratedArg = runtimeCallInfo->GetCallArgRef(1);         // 1:index of parameter ratedArg
    Local<JSValueRef> lightUpDegreeArg = runtimeCallInfo->GetCallArgRef(2); // 2:index of parameter lightUpDegreeArg
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    double rate = 0.0;
    double lightUpDegree = 0.0;
    if (!ArkTSUtils::ParseJsDouble(vm, ratedArg, rate)) {
        return panda::JSValueRef::Undefined(vm);
    }
    if (!ArkTSUtils::ParseJsDouble(vm, lightUpDegreeArg, lightUpDegree)) {
        return panda::JSValueRef::Undefined(vm);
    }
    GetArkUINodeModifiers()->getCommonModifier()->setBackgroundBrightness(nativeNode, rate, lightUpDegree);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundBrightness(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBackgroundBrightness(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBackgroundBrightnessInternal(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    ArkUI_Float32 rate = 1.0f;
    ArkUI_Float32 lightUpDegree = 0.0f;
    ArkUI_Float32 cubicCoeff = 0.0f;
    ArkUI_Float32 quadCoeff = 0.0f;
    ArkUI_Float32 saturation = 1.0f;
    std::vector<float> posRGB(3, 0.0);
    std::vector<float> negRGB(3, 0.0);
    ArkUI_Float32 fraction = 1.0f;
    ParseDynamicBrightnessOption(runtimeCallInfo, vm, rate, lightUpDegree, 
        cubicCoeff, quadCoeff, saturation, posRGB, negRGB, fraction);
    GetArkUINodeModifiers()->getCommonModifier()->setBackgroundBrightnessInternal(
        nativeNode, rate, lightUpDegree, cubicCoeff, quadCoeff, saturation,
        posRGB.data(), posRGB.size(), negRGB.data(), negRGB.size(), fraction);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundBrightnessInternal(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBackgroundBrightnessInternal(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetForegroundBrightness(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    ArkUI_Float32 rate = 1.0f;
    ArkUI_Float32 lightUpDegree = 0.0f;
    ArkUI_Float32 cubicCoeff = 0.0f;
    ArkUI_Float32 quadCoeff = 0.0f;
    ArkUI_Float32 saturation = 1.0f;
    std::vector<float> posRGB(3, 0.0);
    std::vector<float> negRGB(3, 0.0);
    ArkUI_Float32 fraction = 1.0f;
    ParseDynamicBrightnessOption(runtimeCallInfo, vm, rate, lightUpDegree, 
        cubicCoeff, quadCoeff, saturation, posRGB, negRGB, fraction);
    GetArkUINodeModifiers()->getCommonModifier()->setForegroundBrightness(
        nativeNode, rate, lightUpDegree, cubicCoeff, quadCoeff, saturation,
        posRGB.data(), posRGB.size(), negRGB.data(), negRGB.size(), fraction);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetForegroundBrightness(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetForegroundBrightness(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetDragPreviewOptions(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> mode = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> numberBadge = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> isMultiSelectionEnabled = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> defaultAnimationBeforeLifting = runtimeCallInfo->GetCallArgRef(NUM_4);
 
    struct ArkUIDragPreViewOptions preViewOptions = { 1, 0, 0, nullptr, false, true, false};
    int32_t* modeIntArray = nullptr;
    if (mode->IsNumber()) {
        preViewOptions.isModeArray = false;
        preViewOptions.mode = mode->Int32Value(vm);
    } else if (mode->IsArray(vm)) {
        Local<panda::ArrayRef> modeArray = static_cast<Local<panda::ArrayRef>>(mode);
        int32_t arrLength = static_cast<int32_t>(modeArray->Length(vm));
        if (arrLength > NUM_4) {
            arrLength = NUM_4;
        }
        modeIntArray = new int32_t[arrLength];
        for (int32_t i = 0; i < arrLength; i++) {
            Local<JSValueRef> objValue = modeArray->GetValueAt(vm, modeArray, i);
            modeIntArray[i] = objValue->Int32Value(vm);
        }
        preViewOptions.isModeArray = true;
        preViewOptions.modeArray = modeIntArray;
        preViewOptions.modeArrayLength = static_cast<ArkUI_Int32>(arrLength);
    }

    if (numberBadge->IsBoolean()) {
        preViewOptions.isBadgeNumber = false;
        preViewOptions.isShowBadge = numberBadge->ToBoolean(vm)->Value();
    } else if (numberBadge->IsNumber()) {
        preViewOptions.isBadgeNumber = true;
        preViewOptions.badgeNumber = numberBadge->Int32Value(vm);
    }

    struct ArkUIDragInteractionOptions interactionOptions = { false, false };
    if (isMultiSelectionEnabled->IsBoolean()) {
        interactionOptions.isMultiSelectionEnabled = isMultiSelectionEnabled->ToBoolean(vm)->Value();
    }
    if (defaultAnimationBeforeLifting->IsBoolean()) {
        interactionOptions.defaultAnimationBeforeLifting = defaultAnimationBeforeLifting->ToBoolean(vm)->Value();
    }
    GetArkUINodeModifiers()->getCommonModifier()->setDragPreviewOptions(
        nativeNode, preViewOptions, interactionOptions);
    delete[] modeIntArray;
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetDragPreviewOptions(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetDragPreviewOptions(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetResponseRegion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    int32_t length = thirdArg->Int32Value(vm);
    ArkUI_Float32 regionArray[length];
    int32_t regionUnits[length];
    if (!ParseResponseRegion(vm, secondArg, regionArray, regionUnits, length)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetResponseRegion(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    GetArkUINodeModifiers()->getCommonModifier()->setResponseRegion(nativeNode, regionArray, regionUnits, length);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetResponseRegion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetResponseRegion(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetTransition(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto* frameNode = reinterpret_cast<FrameNode*>(nativeNode);
    ViewAbstract::CleanTransition(frameNode);
    ViewAbstract::SetChainedTransition(frameNode, nullptr, nullptr);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetTransition(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto* frameNode = reinterpret_cast<FrameNode*>(nativeNode);
    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    if (!info[1]->IsObject()) {
        ViewAbstract::CleanTransition(frameNode);
        ViewAbstract::SetChainedTransition(frameNode, nullptr, nullptr);
        return panda::JSValueRef::Undefined(vm);
    }
    auto obj = Framework::JSRef<Framework::JSObject>::Cast(info[1]);
    if (!obj->GetProperty("successor_")->IsUndefined()) {
        auto chainedEffect = ParseChainedTransition(obj, info.GetExecutionContext());
        std::function<void(bool)> finishCallback;
        if (info.Length() > 2 && info[2]->IsFunction()) {
            finishCallback =
                ParseTransitionCallback(JSRef<JSFunc>::Cast(info[2]), info.GetExecutionContext(), frameNode);
        }
        ViewAbstract::SetChainedTransition(frameNode, chainedEffect, std::move(finishCallback));
        return panda::JSValueRef::Undefined(vm);
    }
    auto options = ParseJsTransition(info[1]);
    ViewAbstract::SetTransition(frameNode, options);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetTransitionPassThrough(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    return CommonBridge::ResetTransition(runtimeCallInfo);
}

ArkUINativeModuleValue CommonBridge::SetTransitionPassThrough(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    return CommonBridge::SetTransition(runtimeCallInfo);
}

ArkUINativeModuleValue CommonBridge::ResetSharedTransition(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto* frameNode = reinterpret_cast<FrameNode*>(nativeNode);
    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    if (!info[1]->IsString() || info[1]->ToString().empty()) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto id = info[1]->ToString();
    std::shared_ptr<SharedTransitionOption> sharedOption;
    sharedOption = std::make_shared<SharedTransitionOption>();
    sharedOption->duration = DEFAULT_DURATION;
    sharedOption->delay = NUM_0;
    sharedOption->curve = Curves::LINEAR;
    sharedOption->zIndex = NUM_0;
    sharedOption->type = DEFAULT_SHARED_EFFECT;

    ViewAbstract::SetSharedTransition(frameNode, id, sharedOption);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetSharedTransition(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto* frameNode = reinterpret_cast<FrameNode*>(nativeNode);
    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    if (!info[NUM_1]->IsString() || info[NUM_1]->ToString().empty()) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto id = info[NUM_1]->ToString();
    std::shared_ptr<SharedTransitionOption> sharedOption;
    if (info[NUM_2]->IsObject()) {
        Framework::JSRef<Framework::JSObject> jsObj = Framework::JSRef<Framework::JSObject>::Cast(info[NUM_2]);
        sharedOption = std::make_shared<SharedTransitionOption>();
        sharedOption->duration = jsObj->GetPropertyValue<int32_t>("duration", DEFAULT_DURATION);
        if (sharedOption->duration < 0) {
            sharedOption->duration = DEFAULT_DURATION;
        }
        sharedOption->delay = jsObj->GetPropertyValue<int32_t>("delay", 0);
        if (sharedOption->delay < 0) {
            sharedOption->delay = 0;
        }
        RefPtr<Curve> curve;
        Framework::JSRef<Framework::JSVal> curveArgs = jsObj->GetProperty("curve");
        if (curveArgs->IsString()) {
            curve = Framework::CreateCurve(jsObj->GetPropertyValue<std::string>("curve", "linear"), false);
        } else if (curveArgs->IsObject()) {
            Framework::JSRef<Framework::JSVal> curveString =
                Framework::JSRef<Framework::JSObject>::Cast(curveArgs)->GetProperty("__curveString");
            if (!curveString->IsString()) {
                return panda::JSValueRef::Undefined(vm);
            }
            curve = Framework::CreateCurve(curveString->ToString(), false);
        }
        if (!curve) {
            curve = Curves::LINEAR;
        }
        sharedOption->curve = curve;
        if (jsObj->HasProperty("motionPath")) {
            MotionPathOption motionPathOption;
            if (ParseMotionPath(jsObj->GetProperty("motionPath"), motionPathOption)) {
                sharedOption->motionPathOption = motionPathOption;
            }
        }
        sharedOption->zIndex = jsObj->GetPropertyValue<int32_t>("zIndex", 0);
        int32_t type = jsObj->GetPropertyValue<int32_t>("type", static_cast<int32_t>(DEFAULT_SHARED_EFFECT));
        sharedOption->type = static_cast<SharedTransitionEffectType>(type);
    }
    ViewAbstract::SetSharedTransition(frameNode, id, sharedOption);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue CommonBridge::SetMask(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto* frameNode = reinterpret_cast<FrameNode*>(nativeNode);

    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    if (!info[NUM_1]->IsObject()) {
        ViewAbstract::SetProgressMask(frameNode, nullptr);
        return panda::JSValueRef::Undefined(vm);
    }
    auto paramObject = Framework::JSRef<Framework::JSObject>::Cast(info[NUM_1]);
    Framework::JSRef<Framework::JSVal> typeParam = paramObject->GetProperty("type");
    if (!typeParam->IsNull() && !typeParam->IsUndefined() && typeParam->IsString() &&
        typeParam->ToString() == "ProgressMask") {
        auto progressMask = AceType::MakeRefPtr<NG::ProgressMaskProperty>();
        Framework::JSRef<Framework::JSVal> jValue = paramObject->GetProperty("value");
        auto value = jValue->IsNumber() ? jValue->ToNumber<float>() : 0.0f;
        if (value < 0.0f) {
            value = 0.0f;
        }
        progressMask->SetValue(value);
        Framework::JSRef<Framework::JSVal> jTotal = paramObject->GetProperty("total");
        auto total = jTotal->IsNumber() ? jTotal->ToNumber<float>() : DEFAULT_PROGRESS_TOTAL;
        if (total < 0.0f) {
            total = DEFAULT_PROGRESS_TOTAL;
        }
        progressMask->SetMaxValue(total);
        Framework::JSRef<Framework::JSVal> jColor = paramObject->GetProperty("color");
        Color colorVal;
        if (Framework::JSViewAbstract::ParseJsColor(jColor, colorVal)) {
            progressMask->SetColor(colorVal);
        } else {
            auto theme = Framework::JSShapeAbstract::GetTheme<ProgressTheme>();
            progressMask->SetColor(theme->GetMaskColor());
        }
        ViewAbstract::SetProgressMask(frameNode, progressMask);
    } else {
        Framework::JSShapeAbstract* maskShape =
            Framework::JSRef<Framework::JSObject>::Cast(info[NUM_1])->Unwrap<Framework::JSShapeAbstract>();
        if (maskShape == nullptr) {
            return panda::JSValueRef::Undefined(vm);
        };
        ViewAbstract::SetMask(frameNode, maskShape->GetBasicShape());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetMask(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto* frameNode = reinterpret_cast<FrameNode*>(nativeNode);
    ViewAbstract::SetProgressMask(frameNode, nullptr);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetMaskShape(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto* frameNode = reinterpret_cast<FrameNode*>(nativeNode);

    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    if (!info[NUM_1]->IsObject()) {
        return panda::JSValueRef::Undefined(vm);
    }

    Framework::JSShapeAbstract* maskShape =
        Framework::JSRef<Framework::JSObject>::Cast(info[NUM_1])->Unwrap<Framework::JSShapeAbstract>();
    if (maskShape == nullptr) {
        return panda::JSValueRef::Undefined(vm);
    };
    ViewAbstract::SetMask(frameNode, maskShape->GetBasicShape());
    
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetMaskShape(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto* frameNode = reinterpret_cast<FrameNode*>(nativeNode);
    ViewAbstract::SetProgressMask(frameNode, nullptr);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetMouseResponseRegion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    int32_t length = thirdArg->Int32Value(vm);
    ArkUI_Float32 regionArray[length];
    int32_t regionUnits[length];
    if (!ParseResponseRegion(vm, secondArg, regionArray, regionUnits, length)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetMouseResponseRegion(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    GetArkUINodeModifiers()->getCommonModifier()->setMouseResponseRegion(nativeNode, regionArray, regionUnits, length);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetMouseResponseRegion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetMouseResponseRegion(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetEnabled(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsBoolean()) {
        bool boolValue = secondArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setEnabled(nativeNode, boolValue);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetEnabled(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetEnabled(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetEnabled(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetUseShadowBatching(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> booleanArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    if (booleanArg->IsBoolean()) {
        bool boolValue = booleanArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setUseShadowBatching(nativeNode, boolValue);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetUseShadowBatching(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetUseShadowBatching(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetUseShadowBatching(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBlendMode(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);      // 0: index of parameter frameNode
    Local<JSValueRef> blendModeArg = runtimeCallInfo->GetCallArgRef(1);      // 1: index of parameter blendMode
    Local<JSValueRef> blendApplyTypeArg = runtimeCallInfo->GetCallArgRef(2); // 2: index of parameter blendApplyType
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    int32_t blendModeValue = static_cast<int32_t>(OHOS::Ace::BlendMode::NONE);
    int32_t blendApplyTypeValue = static_cast<int32_t>(OHOS::Ace::BlendApplyType::FAST);
    if (blendModeArg->IsNumber()) {
        int32_t blendModeNum = blendModeArg->Int32Value(vm);
        if (blendModeNum >= static_cast<int32_t>(OHOS::Ace::BlendMode::NONE) &&
            blendModeNum <= static_cast<int32_t>(OHOS::Ace::BlendMode::LUMINOSITY)) {
            blendModeValue = blendModeNum;
        } else if (blendModeNum == BACKWARD_COMPAT_MAGIC_NUMBER_OFFSCREEN) {
            blendModeValue = static_cast<int32_t>(OHOS::Ace::BlendMode::SRC_OVER);
            blendApplyTypeValue = static_cast<int32_t>(OHOS::Ace::BlendApplyType::OFFSCREEN);
        }
        if (blendApplyTypeArg->IsNumber()) {
            int32_t blendApplyTypeNum = blendApplyTypeArg->Int32Value(vm);
            if (blendApplyTypeNum >= static_cast<int>(OHOS::Ace::BlendApplyType::FAST) &&
                blendApplyTypeNum <= static_cast<int>(OHOS::Ace::BlendApplyType::OFFSCREEN)) {
                blendApplyTypeValue = blendApplyTypeNum;
            }
        }
        GetArkUINodeModifiers()->getCommonModifier()->setBlendMode(nativeNode, blendModeValue, blendApplyTypeValue);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetBlendMode(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBlendMode(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetBlendMode(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetMonopolizeEvents(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> booleanArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    if (booleanArg->IsBoolean()) {
        bool boolValue = booleanArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setMonopolizeEvents(nativeNode, boolValue);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetMonopolizeEvents(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetMonopolizeEvents(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetMonopolizeEvents(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetDraggable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsBoolean()) {
        bool boolValue = secondArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setDraggable(nativeNode, boolValue);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetDraggable(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetDraggable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetDraggable(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAccessibilityGroup(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsBoolean()) {
        bool boolValue = secondArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getCommonModifier()->setAccessibilityGroup(nativeNode, boolValue);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetAccessibilityGroup(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAccessibilityGroup(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetAccessibilityGroup(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetHoverEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);

    if (secondArg->IsUndefined() || !secondArg->IsNumber()) {
        GetArkUINodeModifiers()->getCommonModifier()->resetHoverEffect(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    int32_t intValue = secondArg->Int32Value(vm);
    GetArkUINodeModifiers()->getCommonModifier()->setHoverEffect(nativeNode, intValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetHoverEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetHoverEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetClickEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> levelArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> scaleArg = runtimeCallInfo->GetCallArgRef(NUM_2);

    int32_t clickEffectLevelValue = 0;
    if (levelArg->IsNumber()) {
        clickEffectLevelValue = levelArg->Int32Value(vm);
        if (clickEffectLevelValue < static_cast<int32_t>(OHOS::Ace::ClickEffectLevel::LIGHT) ||
            clickEffectLevelValue > static_cast<int32_t>(OHOS::Ace::ClickEffectLevel::HEAVY)) {
            clickEffectLevelValue = 0;
        }
    }
    float scaleNumberValue = 0.9f;
    if (!scaleArg->IsNumber()) {
        if ((OHOS::Ace::ClickEffectLevel)clickEffectLevelValue == OHOS::Ace::ClickEffectLevel::MIDDLE ||
            (OHOS::Ace::ClickEffectLevel)clickEffectLevelValue == OHOS::Ace::ClickEffectLevel::HEAVY) {
            scaleNumberValue = 0.95f;
        }
    } else {
        scaleNumberValue = scaleArg->ToNumber(vm)->Value();
        if (LessNotEqual(scaleNumberValue, 0.0) || GreatNotEqual(scaleNumberValue, 1.0)) {
            if ((OHOS::Ace::ClickEffectLevel)clickEffectLevelValue == OHOS::Ace::ClickEffectLevel::MIDDLE ||
                (OHOS::Ace::ClickEffectLevel)clickEffectLevelValue == OHOS::Ace::ClickEffectLevel::HEAVY) {
                scaleNumberValue = 0.95f;
            } else {
                scaleNumberValue = 0.9f;
            }
        }
    }
    GetArkUINodeModifiers()->getCommonModifier()->setClickEffect(nativeNode, clickEffectLevelValue,
        scaleNumberValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetClickEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetClickEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetKeyBoardShortCut(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> valueArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> keysArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if ((!valueArg->IsString(vm) && !valueArg->IsNumber()) || !keysArg->IsArray(vm)) {
        GetArkUINodeModifiers()->getCommonModifier()->resetKeyBoardShortCut(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    std::string stringValue;
    if (valueArg->IsNumber()) {
        OHOS::Ace::FunctionKey functionkey = static_cast<OHOS::Ace::FunctionKey>(valueArg->Int32Value(vm));
        stringValue = JSViewAbstract::GetFunctionKeyName(functionkey);
    } else {
        stringValue = valueArg->ToString(vm)->ToString();
    }
    Local<panda::ArrayRef> keysArray = static_cast<Local<panda::ArrayRef>>(keysArg);
    auto arrLength = keysArray->Length(vm);
    if (arrLength > NUM_10) {
        arrLength = NUM_10;
    }
    int32_t* keysIntArray = new int32_t[arrLength];
    for (size_t i = 0; i < arrLength; i++) {
        Local<JSValueRef> objValue = keysArray->GetValueAt(vm, keysArg, i);
        keysIntArray[i] = objValue->Int32Value(vm);
    }
    GetArkUINodeModifiers()->getCommonModifier()->setKeyBoardShortCut(
        nativeNode, stringValue.c_str(), keysIntArray, arrLength);
    delete[] keysIntArray;
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetKeyBoardShortCut(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetKeyBoardShortCut(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

RefPtr<ResourceWrapper> CreateResourceWrapper()
{
    RefPtr<ResourceAdapter> resourceAdapter = nullptr;
    RefPtr<ThemeConstants> themeConstants = nullptr;
    if (SystemProperties::GetResourceDecoupling()) {
        resourceAdapter = ResourceManager::GetInstance().GetResourceAdapter();
        if (!resourceAdapter) {
            return nullptr;
        }
    } else {
        themeConstants = JSViewAbstract::GetThemeConstants();
        if (!themeConstants) {
            return nullptr;
        }
    }
    auto resourceWrapper = AceType::MakeRefPtr<ResourceWrapper>(themeConstants, resourceAdapter);
    return resourceWrapper;
}

bool ParseLightPosition(ArkUIRuntimeCallInfo *runtimeCallInfo, EcmaVM* vm, ArkUISizeType& dimPosX,
    ArkUISizeType& dimPosY, ArkUISizeType& dimPosZ)
{
    Local<JSValueRef> positionXArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> positionYArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> positionZArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    CalcDimension dimPositionX;
    CalcDimension dimPositionY;
    CalcDimension dimPositionZ;
    bool xSuccess = ArkTSUtils::ParseJsDimensionVp(vm, positionXArg, dimPositionX, false);
    bool ySuccess = ArkTSUtils::ParseJsDimensionVp(vm, positionYArg, dimPositionY, false);
    bool zSuccess = ArkTSUtils::ParseJsDimensionVp(vm, positionZArg, dimPositionZ, false);
    if (!(xSuccess && ySuccess && zSuccess)) {
        return false;
    }
    dimPosX.value = dimPositionX.Value();
    dimPosX.unit = static_cast<int8_t>(dimPositionX.Unit());
    dimPosY.value = dimPositionY.Value();
    dimPosY.unit = static_cast<int8_t>(dimPositionY.Unit());
    dimPosZ.value = dimPositionZ.Value();
    dimPosZ.unit = static_cast<int8_t>(dimPositionZ.Unit());
    return true;
}

void ParseLightSource(ArkUIRuntimeCallInfo *runtimeCallInfo, EcmaVM* vm, ArkUINodeHandle& nativeNode)
{
    struct ArkUISizeType dimPosX = { 0.0, 0 };
    struct ArkUISizeType dimPosY = { 0.0, 0 };
    struct ArkUISizeType dimPosZ = { 0.0, 0 };
    bool success = ParseLightPosition(runtimeCallInfo, vm, dimPosX, dimPosY, dimPosZ);
    if (success) {
        GetArkUINodeModifiers()->getCommonModifier()->setPointLightPosition(nativeNode, &dimPosX, &dimPosY, &dimPosZ);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetPointLightPosition(nativeNode);
    }

    Local<JSValueRef> intensityArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    if (intensityArg->IsNumber()) {
        auto intensityValue = static_cast<ArkUI_Float32>((intensityArg->ToNumber(vm)->Value()));
        GetArkUINodeModifiers()->getCommonModifier()->setPointLightIntensity(nativeNode, intensityValue);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetPointLightIntensity(nativeNode);
    }

    Local<JSValueRef> colorArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    Color colorValue;
    if (ArkTSUtils::ParseJsColorAlpha(vm, colorArg, colorValue)) {
        GetArkUINodeModifiers()->getCommonModifier()->setPointLightColor(nativeNode, colorValue.GetValue());
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetPointLightColor(nativeNode);
    }
}

ArkUINativeModuleValue CommonBridge::SetPointLightStyle(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
#ifdef POINT_LIGHT_ENABLE
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    ParseLightSource(runtimeCallInfo, vm, nativeNode);

    auto resourceWrapper = CreateResourceWrapper();
    Local<JSValueRef> illuminatedArg = runtimeCallInfo->GetCallArgRef(NUM_6);
    if (illuminatedArg->IsNumber() || !resourceWrapper) {
        auto illuminatedValue = static_cast<ArkUI_Uint32>(illuminatedArg->ToNumber(vm)->Value());
        Dimension illuminatedBorderWidth = resourceWrapper->GetDimensionByName(ILLUMINATED_BORDER_WIDTH_SYS_RES_NAME);
        struct ArkUISizeType illuminatedBorderWidthValue = { 0.0, 0 };
        illuminatedBorderWidthValue.value = illuminatedBorderWidth.Value();
        illuminatedBorderWidthValue.unit = static_cast<int8_t>(illuminatedBorderWidth.Unit());
        GetArkUINodeModifiers()->getCommonModifier()->setPointLightIlluminated(
            nativeNode, illuminatedValue, &illuminatedBorderWidthValue);
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetPointLightIlluminated(nativeNode);
    }

    Local<JSValueRef> bloomArg = runtimeCallInfo->GetCallArgRef(NUM_7);
    if (bloomArg->IsNumber() || !resourceWrapper) {
        auto bloomValue = static_cast<ArkUI_Float32>(bloomArg->ToNumber(vm)->Value());
        double bloomRadius = resourceWrapper->GetDoubleByName(BLOOM_RADIUS_SYS_RES_NAME);
        Color bloomColor = resourceWrapper->GetColorByName(BLOOM_COLOR_SYS_RES_NAME);
        GetArkUINodeModifiers()->getCommonModifier()->setPointLightBloom(nativeNode, bloomValue,
            static_cast<ArkUI_Float32>(bloomRadius), static_cast<ArkUI_Uint32>(bloomColor.GetValue()));
    } else {
        GetArkUINodeModifiers()->getCommonModifier()->resetPointLightBloom(nativeNode);
    }
#endif
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetPointLightStyle(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
#ifdef POINT_LIGHT_ENABLE
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetPointLightPosition(nativeNode);
    GetArkUINodeModifiers()->getCommonModifier()->resetPointLightIntensity(nativeNode);
    GetArkUINodeModifiers()->getCommonModifier()->resetPointLightColor(nativeNode);
    GetArkUINodeModifiers()->getCommonModifier()->resetPointLightIlluminated(nativeNode);
    GetArkUINodeModifiers()->getCommonModifier()->resetPointLightBloom(nativeNode);
#endif
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetClipWithEdge(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto *frameNode = reinterpret_cast<FrameNode *>(nativeNode);

    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    if (info[NUM_1]->IsUndefined()) {
        ViewAbstract::SetClipEdge(frameNode, true);
        return panda::JSValueRef::Undefined(vm);
    }
    if (info[NUM_1]->IsObject()) {
        Framework::JSShapeAbstract *clipShape =
            Framework::JSRef<Framework::JSObject>::Cast(info[NUM_1])->Unwrap<Framework::JSShapeAbstract>();
        if (clipShape == nullptr) {
            return panda::JSValueRef::Undefined(vm);
        }
        ViewAbstract::SetClipShape(frameNode, clipShape->GetBasicShape());
    } else if (info[NUM_1]->IsBoolean()) {
        ViewAbstract::SetClipEdge(frameNode, info[NUM_1]->ToBoolean());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetClipWithEdge(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto *frameNode = reinterpret_cast<FrameNode *>(nativeNode);
    ViewAbstract::SetClipEdge(frameNode, true);
    return panda::JSValueRef::Undefined(vm);
}

FrameNode* CommonBridge::GetFrameNode(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, nullptr);
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), nullptr);
    auto* nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, nullptr);
    auto* frameNode = reinterpret_cast<FrameNode*>(nativeNode);
    return frameNode;
}

Local<panda::ObjectRef> CommonBridge::CreateGestureInfo(EcmaVM* vm, const RefPtr<NG::GestureInfo>& gestureInfo)
{
    if (gestureInfo->GetTag().has_value()) {
        const char* keys[] = { "tag", "type", "isSystemGesture" };
        Local<JSValueRef> values[] = { panda::StringRef::NewFromUtf8(vm, gestureInfo->GetTag().value().c_str()),
            panda::NumberRef::New(vm, static_cast<int32_t>(gestureInfo->GetType())),
            panda::BooleanRef::New(vm, gestureInfo->IsSystemGesture()) };
        return panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, values);
    }
    const char* keys[] = { "type", "isSystemGesture" };
    Local<JSValueRef> values[] = { panda::NumberRef::New(vm, static_cast<int32_t>(gestureInfo->GetType())),
        panda::BooleanRef::New(vm, gestureInfo->IsSystemGesture()) };
    return panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, values);
}

Local<panda::ObjectRef> CommonBridge::CreateGestureEventInfo(
    EcmaVM* vm, GestureTypeName typeName, const std::shared_ptr<BaseGestureEvent>& info)
{
    auto obj = SetUniqueAttributes(vm, typeName, info);
    obj->Set(vm, panda::StringRef::NewFromUtf8(vm, "timestamp"),
        panda::NumberRef::New(vm, static_cast<double>(info->GetTimeStamp().time_since_epoch().count())));
    obj->Set(vm, panda::StringRef::NewFromUtf8(vm, "source"),
        panda::NumberRef::New(vm, static_cast<int32_t>(info->GetSourceDevice())));
    obj->Set(vm, panda::StringRef::NewFromUtf8(vm, "pressure"), panda::NumberRef::New(vm, info->GetForce()));
    if (info->GetTiltX().has_value()) {
        obj->Set(vm, panda::StringRef::NewFromUtf8(vm, "tiltX"), panda::NumberRef::New(vm, info->GetTiltX().value()));
    }
    if (info->GetTiltY().has_value()) {
        obj->Set(vm, panda::StringRef::NewFromUtf8(vm, "tiltY"), panda::NumberRef::New(vm, info->GetTiltY().value()));
    }
    obj->Set(vm, panda::StringRef::NewFromUtf8(vm, "sourceTool"),
        panda::NumberRef::New(vm, static_cast<int32_t>(info->GetSourceTool())));

    auto fingerArr = panda::ArrayRef::New(vm);
    const std::list<FingerInfo>& fingerList = info->GetFingerList();
    std::list<FingerInfo> notTouchFingerList;
    int32_t maxFingerId = -1;
    for (const FingerInfo& fingerInfo : fingerList) {
        auto element = CreateFingerInfo(vm, fingerInfo);
        if (fingerInfo.sourceType_ == SourceType::TOUCH && fingerInfo.sourceTool_ == SourceTool::FINGER) {
            fingerArr->SetValueAt(vm, fingerArr, fingerInfo.fingerId_, element);
            if (fingerInfo.fingerId_ > maxFingerId) {
                maxFingerId = fingerInfo.fingerId_;
            }
        } else {
            notTouchFingerList.emplace_back(fingerInfo);
        }
    }
    auto idx = maxFingerId + 1;
    for (const FingerInfo& fingerInfo : notTouchFingerList) {
        auto element = CreateFingerInfo(vm, fingerInfo);
        fingerArr->SetValueAt(vm, fingerArr, idx++, element);
    }
    obj->Set(vm, panda::StringRef::NewFromUtf8(vm, "fingerList"), fingerArr);
    obj->Set(vm, panda::StringRef::NewFromUtf8(vm, "target"), CreateEventTargetObject(vm, info));
    return obj;
}

Local<panda::ObjectRef> CommonBridge::SetUniqueAttributes(
    EcmaVM* vm, GestureTypeName typeName, const std::shared_ptr<BaseGestureEvent>& info)
{
    double density = PipelineBase::GetCurrentDensity();
    switch (typeName) {
        case OHOS::Ace::GestureTypeName::LONG_PRESS_GESTURE: {
            auto* longPressGestureEvent = TypeInfoHelper::DynamicCast<LongPressGestureEvent>(info.get());
            if (longPressGestureEvent) {
                const char* keys[] = { "repeat" };
                Local<JSValueRef> values[] = { panda::BooleanRef::New(vm, longPressGestureEvent->GetRepeat()) };
                return panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, values);
            }
            return panda::ObjectRef::New(vm);
        }
        case OHOS::Ace::GestureTypeName::PAN_GESTURE: {
            auto* panGestureEvent = TypeInfoHelper::DynamicCast<PanGestureEvent>(info.get());
            if (panGestureEvent) {
                const char* keys[] = { "offsetX", "offsetY", "velocityX", "velocityY", "velocity" };
                Local<JSValueRef> values[] = { panda::NumberRef::New(vm, panGestureEvent->GetOffsetX() / density),
                    panda::NumberRef::New(vm, panGestureEvent->GetOffsetY() / density),
                    panda::NumberRef::New(vm, panGestureEvent->GetVelocity().GetVelocityX() / density),
                    panda::NumberRef::New(vm, panGestureEvent->GetVelocity().GetVelocityY() / density),
                    panda::NumberRef::New(vm, panGestureEvent->GetVelocity().GetVelocityValue() / density) };
                return panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, values);
            }
            return panda::ObjectRef::New(vm);
        }
        case OHOS::Ace::GestureTypeName::PINCH_GESTURE: {
            auto* pinchGestureEvent = TypeInfoHelper::DynamicCast<PinchGestureEvent>(info.get());
            if (pinchGestureEvent) {
                const char* keys[] = { "scale", "pinchCenterX", "pinchCenterY" };
                Local<JSValueRef> values[] = { panda::NumberRef::New(vm, pinchGestureEvent->GetScale()),
                    panda::NumberRef::New(vm, pinchGestureEvent->GetPinchCenter().GetX() / density),
                    panda::NumberRef::New(vm, pinchGestureEvent->GetPinchCenter().GetY() / density) };
                return panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, values);
            }
            return panda::ObjectRef::New(vm);
        }
        case OHOS::Ace::GestureTypeName::ROTATION_GESTURE: {
            auto* rotationGestureEvent = TypeInfoHelper::DynamicCast<RotationGestureEvent>(info.get());
            if (rotationGestureEvent) {
                const char* keys[] = { "angle" };
                Local<JSValueRef> values[] = { panda::NumberRef::New(vm, rotationGestureEvent->GetAngle()) };
                return panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, values);
            }
            return panda::ObjectRef::New(vm);
        }
        case OHOS::Ace::GestureTypeName::SWIPE_GESTURE: {
            auto* swipeGestureEvent = TypeInfoHelper::DynamicCast<SwipeGestureEvent>(info.get());
            if (swipeGestureEvent) {
                const char* keys[] = { "angle", "speed" };
                Local<JSValueRef> values[] = { panda::NumberRef::New(vm, swipeGestureEvent->GetAngle()),
                    panda::NumberRef::New(vm, swipeGestureEvent->GetSpeed()) };
                return panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, values);
            }
            return panda::ObjectRef::New(vm);
        }
        default:
            return panda::ObjectRef::New(vm);
    }
}

Local<panda::ObjectRef> CommonBridge::CreateFingerInfo(EcmaVM* vm, const FingerInfo& fingerInfo)
{
    const OHOS::Ace::Offset& globalLocation = fingerInfo.globalLocation_;
    const OHOS::Ace::Offset& localLocation = fingerInfo.localLocation_;
    const OHOS::Ace::Offset& screenLocation = fingerInfo.screenLocation_;
    double density = PipelineBase::GetCurrentDensity();
    const char* keys[] = { "id", "globalX", "globalY", "localX", "localY", "displayX", "displayY" };
    Local<JSValueRef> values[] = { panda::NumberRef::New(vm, fingerInfo.fingerId_),
        panda::NumberRef::New(vm, globalLocation.GetX() / density),
        panda::NumberRef::New(vm, globalLocation.GetY() / density),
        panda::NumberRef::New(vm, localLocation.GetX() / density),
        panda::NumberRef::New(vm, localLocation.GetY() / density),
        panda::NumberRef::New(vm, screenLocation.GetX() / density),
        panda::NumberRef::New(vm, screenLocation.GetY() / density) };
        return panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, values);
}

Local<panda::ObjectRef> CommonBridge::CreateEventTargetObject(EcmaVM* vm, const std::shared_ptr<BaseGestureEvent>& info)
{
    const auto& localOffset = info->GetTarget().area.GetOffset();
    const auto& origin = info->GetTarget().origin;
    const char* keysOfOffset[] = { "x", "y" };
    Local<JSValueRef> valuesOfOffset[] = { panda::NumberRef::New(vm, localOffset.GetX().ConvertToVp()),
        panda::NumberRef::New(vm, localOffset.GetY().ConvertToVp()) };
    auto offset = panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keysOfOffset), keysOfOffset, valuesOfOffset);

    const char* keysOfGlobalOffset[] = { "x", "y" };
    Local<JSValueRef> valuesOfGlobalOffset[] = { panda::NumberRef::New(
                                                     vm, (origin.GetX() + localOffset.GetX()).ConvertToVp()),
        panda::NumberRef::New(vm, (origin.GetY() + localOffset.GetY()).ConvertToVp()) };
    auto globalOffset = panda::ObjectRef::NewWithNamedProperties(
        vm, ArraySize(keysOfGlobalOffset), keysOfGlobalOffset, valuesOfGlobalOffset);
    const char* keysOfArea[] = { "position", "globalPosition", "width", "height" };
    Local<JSValueRef> valuesOfArea[] = { offset, globalOffset,
        panda::NumberRef::New(vm, info->GetTarget().area.GetWidth().ConvertToVp()),
        panda::NumberRef::New(vm, info->GetTarget().area.GetHeight().ConvertToVp()) };
    auto area = panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keysOfArea), keysOfArea, valuesOfArea);
    auto target = panda::ObjectRef::New(vm);
    target->Set(vm, panda::StringRef::NewFromUtf8(vm, "area"), area);
    return target;
}

Local<panda::ObjectRef> CommonBridge::CreateAreaObject(EcmaVM* vm, const RectF& rect, const OffsetF& origin)
{
    double density = PipelineBase::GetCurrentDensity();
    auto localOffset = rect.GetOffset();
    const char* keysOfOffset[] = { "x", "y" };
    Local<JSValueRef> valuesOfOffset[] = { panda::NumberRef::New(vm, localOffset.GetX() / density),
        panda::NumberRef::New(vm, localOffset.GetY() / density) };
    auto offset = panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keysOfOffset), keysOfOffset, valuesOfOffset);
    const char* keysOfGlobalOffset[] = { "x", "y" };
    Local<JSValueRef> valuesOfGlobalOffset[] = { panda::NumberRef::New(
                                                     vm, (localOffset.GetX() + origin.GetX()) / density),
        panda::NumberRef::New(vm, (localOffset.GetY() + origin.GetY()) / density) };
    auto globalOffset = panda::ObjectRef::NewWithNamedProperties(
        vm, ArraySize(keysOfGlobalOffset), keysOfGlobalOffset, valuesOfGlobalOffset);

    const char* keysOfArea[] = { "pos", "position", "globalPos", "globalPosition", "width", "height" };
    Local<JSValueRef> valuesOfArea[] = { offset, offset, globalOffset, globalOffset,
        panda::NumberRef::New(vm, rect.Width() / density), panda::NumberRef::New(vm, rect.Height() / density) };
    return panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keysOfArea), keysOfArea, valuesOfArea);
}

void CommonBridge::GetGestureCommonValue(ArkUIRuntimeCallInfo* runtimeCallInfo, int32_t& priority, int32_t& mask)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_VOID(vm);
    Local<JSValueRef> priorityArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    if (!priorityArg.IsNull() && !priorityArg->IsUndefined()) {
        priority = static_cast<int32_t>(priorityArg->ToNumber(vm)->Value());
    }
    Local<JSValueRef> maskArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    if (!maskArg.IsNull() && !maskArg->IsUndefined()) {
        mask = static_cast<int32_t>(maskArg->ToNumber(vm)->Value());
    }
}

void CommonBridge::SetGestureTag(ArkUIRuntimeCallInfo* runtimeCallInfo, uint32_t argNumber, ArkUIGesture* gesture)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_VOID(vm);
    Local<JSValueRef> gestureTagArg = runtimeCallInfo->GetCallArgRef(argNumber);
    if (!gestureTagArg.IsNull() && !gestureTagArg->IsUndefined()) {
        auto gesturePtr = Referenced::Claim(reinterpret_cast<Gesture*>(gesture));
        gesturePtr->SetTag(gestureTagArg->ToString(vm)->ToString());
    }
}

void CommonBridge::GetTapGestureValue(
    ArkUIRuntimeCallInfo* runtimeCallInfo, int32_t& fingers, int32_t& count, uint32_t argNumber)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_VOID(vm);
    Local<JSValueRef> fingersArg = runtimeCallInfo->GetCallArgRef(argNumber);
    if (!fingersArg.IsNull() && !fingersArg->IsUndefined()) {
        auto fingersValue = static_cast<int32_t>(fingersArg->ToNumber(vm)->Value());
        fingers = (fingersValue < DEFAULT_TAP_FINGER || fingersValue > DEFAULT_MAX_FINGERS) ? DEFAULT_TAP_FINGER
                                                                                            : fingersValue;
    }
    Local<JSValueRef> countArg = runtimeCallInfo->GetCallArgRef(argNumber + 1);
    if (!countArg.IsNull() && !countArg->IsUndefined()) {
        auto countValue = static_cast<int32_t>(countArg->ToNumber(vm)->Value());
        count = countValue < DEFAULT_TAP_COUNT ? DEFAULT_TAP_COUNT : countValue;
    }
}

void CommonBridge::GetLongPressGestureValue(
    ArkUIRuntimeCallInfo* runtimeCallInfo, int32_t& fingers, bool& repeat, int32_t& duration, uint32_t argNumber)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_VOID(vm);
    Local<JSValueRef> fingersArg = runtimeCallInfo->GetCallArgRef(argNumber);
    if (!fingersArg.IsNull() && !fingersArg->IsUndefined()) {
        auto fingersValue = static_cast<int32_t>(fingersArg->ToNumber(vm)->Value());
        fingers = (fingersValue < DEFAULT_LONG_PRESS_FINGER || fingersValue > DEFAULT_MAX_FINGERS)
                      ? DEFAULT_LONG_PRESS_FINGER
                      : fingersValue;
    }
    Local<JSValueRef> repeatArg = runtimeCallInfo->GetCallArgRef(argNumber + 1);
    if (!repeatArg.IsNull() && !repeatArg->IsUndefined()) {
        repeat = repeatArg->ToBoolean(vm)->Value();
    }
    Local<JSValueRef> durationArg = runtimeCallInfo->GetCallArgRef(argNumber + 2);
    if (!durationArg.IsNull() && !durationArg->IsUndefined()) {
        auto durationValue = static_cast<int32_t>(durationArg->ToNumber(vm)->Value());
        duration = durationValue <= 0 ? DEFAULT_LONG_PRESS_DURATION : durationValue;
    }
}

void CommonBridge::GetPanGestureValue(
    ArkUIRuntimeCallInfo* runtimeCallInfo, int32_t& fingers, int32_t& direction, double& distance, uint32_t argNumber)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_VOID(vm);
    Local<JSValueRef> fingersArg = runtimeCallInfo->GetCallArgRef(argNumber);
    if (!fingersArg.IsNull() && !fingersArg->IsUndefined()) {
        auto fingersValue = static_cast<int32_t>(fingersArg->ToNumber(vm)->Value());
        fingers = (fingersValue < DEFAULT_PAN_FINGER || fingersValue > DEFAULT_MAX_FINGERS) ? DEFAULT_PAN_FINGER
                                                                                            : fingersValue;
    }
    Local<JSValueRef> directionArg = runtimeCallInfo->GetCallArgRef(argNumber + 1);
    if (!directionArg.IsNull() && !directionArg->IsUndefined()) {
        direction = static_cast<int32_t>(directionArg->ToNumber(vm)->Value());
    }
    Local<JSValueRef> distanceArg = runtimeCallInfo->GetCallArgRef(argNumber + 2);
    if (!distanceArg.IsNull() && !distanceArg->IsUndefined()) {
        auto distanceValue = static_cast<double>(distanceArg->ToNumber(vm)->Value());
        distance = distanceValue < 0.0f ? DEFAULT_PAN_DISTANCE.ConvertToPx() : distanceValue;
    }
}

void CommonBridge::GetSwipeGestureValue(
    ArkUIRuntimeCallInfo* runtimeCallInfo, int32_t& fingers, int32_t& direction, double& speed, uint32_t argNumber)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_VOID(vm);
    Local<JSValueRef> fingersArg = runtimeCallInfo->GetCallArgRef(argNumber);
    if (!fingersArg.IsNull() && !fingersArg->IsUndefined()) {
        auto fingersValue = static_cast<int32_t>(fingersArg->ToNumber(vm)->Value());
        fingers = (fingersValue < DEFAULT_SLIDE_FINGER || fingersValue > DEFAULT_MAX_FINGERS) ? DEFAULT_SLIDE_FINGER
                                                                                              : fingersValue;
    }
    Local<JSValueRef> directionArg = runtimeCallInfo->GetCallArgRef(argNumber + 1);
    if (!directionArg.IsNull() && !directionArg->IsUndefined()) {
        direction = static_cast<int32_t>(directionArg->ToNumber(vm)->Value());
    }
    Local<JSValueRef> speedArg = runtimeCallInfo->GetCallArgRef(argNumber + 2);
    if (!speedArg.IsNull() && !speedArg->IsUndefined()) {
        auto speedValue = static_cast<double>(speedArg->ToNumber(vm)->Value());
        speed = LessOrEqual(speedValue, 0.0) ? DEFAULT_SLIDE_SPEED : speedValue;
    }
}

void CommonBridge::GetPinchGestureValue(
    ArkUIRuntimeCallInfo* runtimeCallInfo, int32_t& fingers, double& distance, uint32_t argNumber)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_VOID(vm);
    Local<JSValueRef> fingersArg = runtimeCallInfo->GetCallArgRef(argNumber);
    if (!fingersArg.IsNull() && !fingersArg->IsUndefined()) {
        auto fingersValue = static_cast<int32_t>(fingersArg->ToNumber(vm)->Value());
        fingers = (fingersValue < DEFAULT_PINCH_FINGER || fingersValue > DEFAULT_MAX_PINCH_FINGER)
                      ? DEFAULT_PINCH_FINGER
                      : fingersValue;
    }
    Local<JSValueRef> distanceArg = runtimeCallInfo->GetCallArgRef(argNumber + 1);
    if (!distanceArg.IsNull() && !distanceArg->IsUndefined()) {
        auto distanceValue = static_cast<double>(distanceArg->ToNumber(vm)->Value());
        distance = distanceValue <= 0.0 ? DEFAULT_PINCH_DISTANCE : distanceValue;
    }
}

void CommonBridge::GetRotationGestureValue(
    ArkUIRuntimeCallInfo* runtimeCallInfo, int32_t& fingers, double& angle, uint32_t argNumber)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_VOID(vm);
    Local<JSValueRef> fingersArg = runtimeCallInfo->GetCallArgRef(argNumber);
    if (!fingersArg.IsNull() && !fingersArg->IsUndefined()) {
        auto fingersValue = static_cast<int32_t>(fingersArg->ToNumber(vm)->Value());
        fingers = (fingersValue < DEFAULT_ROTATION_FINGER || fingersValue > DEFAULT_MAX_ROTATION_FINGER)
                      ? DEFAULT_ROTATION_FINGER
                      : fingersValue;
    }
    Local<JSValueRef> angleArg = runtimeCallInfo->GetCallArgRef(argNumber + 1);
    if (!angleArg.IsNull() && !angleArg->IsUndefined()) {
        auto angleValue = static_cast<double>(angleArg->ToNumber(vm)->Value());
        angle = (angleValue <= 0 || angleValue > DEFAULT_MAX_ROTATION_ANGLE) ? DEFAULT_ROTATION_ANGLE : angleValue;
    }
}

void CommonBridge::GetGestureModeValue(ArkUIRuntimeCallInfo* runtimeCallInfo, int32_t& mode, uint32_t argNumber)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_VOID(vm);
    Local<JSValueRef> modeArg = runtimeCallInfo->GetCallArgRef(argNumber);
    if (!modeArg.IsNull() && !modeArg->IsUndefined()) {
        mode = static_cast<int32_t>(modeArg->ToNumber(vm)->Value());
    }
}

void CommonBridge::SetOnGestureEvent(
    ArkUIRuntimeCallInfo* runtimeCallInfo, const GestureEventAction& action, uint32_t argNumber, ArkUIGesture* gesture)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_VOID(vm);
    Local<JSValueRef> eventArg = runtimeCallInfo->GetCallArgRef(argNumber);
    if (eventArg.IsNull() || eventArg->IsUndefined() || !eventArg->IsFunction(vm)) {
        return;
    }
    auto obj = eventArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;

    if (action == Ace::GestureEventAction::CANCEL) {
        auto onActionCancelFunc = [vm, func = panda::CopyableGlobal(vm, func), containerId]() {
            panda::LocalScope pandaScope(vm);
            panda::TryCatch trycatch(vm);
            ContainerScope scope(containerId);
            func->Call(vm, func.ToLocal(), nullptr, 0);
        };
        auto gesturePtr = Referenced::Claim(reinterpret_cast<Gesture*>(gesture));
        gesturePtr->SetOnActionCancelId(onActionCancelFunc);
        return;
    }
    auto event = [vm, func = panda::CopyableGlobal(vm, func), containerId](GestureEvent& info) {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        auto obj = CreateCommonGestureEventInfo(vm, info);
        panda::Local<panda::JSValueRef> params[1] = { obj };
        func->Call(vm, func.ToLocal(), params, 1);
    };
    auto gesturePtr = Referenced::Claim(reinterpret_cast<Gesture*>(gesture));
    switch (action) {
        case Ace::GestureEventAction::ACTION:
            gesturePtr->SetOnActionId(event);
            break;
        case Ace::GestureEventAction::START:
            gesturePtr->SetOnActionStartId(event);
            break;
        case Ace::GestureEventAction::UPDATE:
            gesturePtr->SetOnActionUpdateId(event);
            break;
        case Ace::GestureEventAction::END:
            gesturePtr->SetOnActionEndId(event);
            break;
        default:
            break;
    }
}

Local<panda::ObjectRef> CommonBridge::CreateCommonGestureEventInfo(EcmaVM* vm, GestureEvent& info)
{
    double density = PipelineBase::GetCurrentDensity();
    const char* keys[] = { "repeat", "offsetX", "offsetY", "scale", "angle", "speed", "timestamp", "pinchCenterX",
        "pinchCenterY", "source", "pressure", "sourceTool", "velocityX", "velocityY", "velocity",
        "getModifierKeyState" };
    Local<JSValueRef> values[] = { panda::BooleanRef::New(vm, info.GetRepeat()),
        panda::NumberRef::New(vm, info.GetOffsetX() / density), panda::NumberRef::New(vm, info.GetOffsetY() / density),
        panda::NumberRef::New(vm, info.GetScale()), panda::NumberRef::New(vm, info.GetAngle()),
        panda::NumberRef::New(vm, info.GetSpeed()),
        panda::NumberRef::New(vm, static_cast<double>(info.GetTimeStamp().time_since_epoch().count())),
        panda::NumberRef::New(vm, info.GetPinchCenter().GetX() / density),
        panda::NumberRef::New(vm, info.GetPinchCenter().GetY() / density),
        panda::NumberRef::New(vm, static_cast<int32_t>(info.GetSourceDevice())),
        panda::NumberRef::New(vm, info.GetForce()),
        panda::NumberRef::New(vm, static_cast<int32_t>(static_cast<int32_t>(info.GetSourceTool()))),
        panda::NumberRef::New(vm, info.GetVelocity().GetVelocityX() / density),
        panda::NumberRef::New(vm, info.GetVelocity().GetVelocityY() / density),
        panda::NumberRef::New(vm, info.GetVelocity().GetVelocityValue() / density),
        panda::FunctionRef::New(vm, ArkTSUtils::JsGetModifierKeyState) };
    auto obj = panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, values);
    if (info.GetTiltX().has_value()) {
        obj->Set(vm, panda::StringRef::NewFromUtf8(vm, "tiltX"),
            panda::NumberRef::New(vm, static_cast<int32_t>(info.GetTiltX().value())));
    }
    if (info.GetTiltY().has_value()) {
        obj->Set(vm, panda::StringRef::NewFromUtf8(vm, "tiltY"),
            panda::NumberRef::New(vm, static_cast<int32_t>(info.GetTiltY().value())));
    }
    auto fingerArr = CreateFingerListArray(vm, info);
    obj->Set(vm, panda::StringRef::NewFromUtf8(vm, "fingerList"), fingerArr);
    obj->Set(vm, panda::StringRef::NewFromUtf8(vm, "target"), FrameNodeBridge::CreateEventTargetObject(vm, info));
    obj->SetNativePointerFieldCount(vm, 1);
    obj->SetNativePointerField(vm, 0, static_cast<void*>(&info));
    return obj;
}

Local<panda::ArrayRef> CommonBridge::CreateFingerListArray(EcmaVM* vm, GestureEvent& info)
{
    auto fingerArr = panda::ArrayRef::New(vm);
    const std::list<FingerInfo>& fingerList = info.GetFingerList();
    std::list<FingerInfo> notTouchFingerList;
    int32_t maxFingerId = -1;
    for (const FingerInfo& fingerInfo : fingerList) {
        auto element = CreateFingerInfo(vm, fingerInfo);
        if (fingerInfo.sourceType_ == SourceType::TOUCH && fingerInfo.sourceTool_ == SourceTool::FINGER) {
            fingerArr->SetValueAt(vm, fingerArr, fingerInfo.fingerId_, element);
            if (fingerInfo.fingerId_ > maxFingerId) {
                maxFingerId = fingerInfo.fingerId_;
            }
        } else {
            notTouchFingerList.emplace_back(fingerInfo);
        }
    }
    auto idx = maxFingerId + 1;
    for (const FingerInfo& fingerInfo : notTouchFingerList) {
        auto element = CreateFingerInfo(vm, fingerInfo);
        fingerArr->SetValueAt(vm, fingerArr, idx++, element);
    }
    return fingerArr;
}

ArkUIGesture* CommonBridge::GetGestureGroup(ArkUIRuntimeCallInfo* runtimeCallInfo, uint32_t argNumber)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, nullptr);
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(argNumber);
    CHECK_NULL_RETURN(!firstArg.IsNull(), nullptr);
    auto* group = reinterpret_cast<ArkUIGesture*>(firstArg->ToNativePointer(vm)->Value());
    return group;
}

ArkUINativeModuleValue CommonBridge::SetOnClick(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onClick = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode), containerId](
                       GestureEvent& info) {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        auto obj = FrameNodeBridge::CreateGestureEventInfo(vm, info);
        panda::Local<panda::JSValueRef> params[1] = { obj };
        func->Call(vm, func.ToLocal(), params, 1);
    };
    NG::ViewAbstract::SetOnClick(frameNode, std::move(onClick));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnClick(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::DisableOnClick(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOnTouch(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onTouch = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode), containerId](
                       TouchEventInfo& info) {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        auto eventObj = FrameNodeBridge::CreateTouchEventInfo(vm, info);
        panda::Local<panda::JSValueRef> params[1] = { eventObj };
        func->Call(vm, func.ToLocal(), params, 1);
    };
    NG::ViewAbstract::SetOnTouch(frameNode, std::move(onTouch));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnTouch(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::DisableOnTouch(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetChainMode(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    int32_t direction = 0;
    int32_t style = 0;
    if (!secondArg.IsNull() && !secondArg->IsUndefined()) {
        direction = static_cast<int32_t>(secondArg->ToNumber(vm)->Value());
    }
    if (!thirdArg.IsNull() && !thirdArg->IsUndefined()) {
        style = static_cast<int32_t>(thirdArg->ToNumber(vm)->Value());
    }
    GetArkUINodeModifiers()->getCommonModifier()->setChainStyle(nativeNode, direction, style);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetChainMode(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetChainStyle(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue CommonBridge::SetOnAppear(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onAppear = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode), containerId]() {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        func->Call(vm, func.ToLocal(), nullptr, 0);
    };
    NG::ViewAbstract::SetOnAppear(frameNode, std::move(onAppear));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnAppear(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::DisableOnAppear(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOnDisappear(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onDisappear = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode),
                           containerId]() {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        func->Call(vm, func.ToLocal(), nullptr, 0);
    };
    NG::ViewAbstract::SetOnDisappear(frameNode, std::move(onDisappear));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnDisappear(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::DisableOnDisappear(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOnAttach(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onAttach = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode), containerId]() {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        func->Call(vm, func.ToLocal(), nullptr, 0);
    };
    NG::ViewAbstract::SetOnAttach(frameNode, std::move(onAttach));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnAttach(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::DisableOnAttach(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOnDetach(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onDetach = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode),
                           containerId]() {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        func->Call(vm, func.ToLocal(), nullptr, 0);
    };
    NG::ViewAbstract::SetOnDetach(frameNode, std::move(onDetach));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnDetach(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::DisableOnDetach(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOnKeyEvent(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onKeyEvent = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode), containerId](
                          KeyEventInfo& info) {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        const char* keys[] = { "type", "keyCode", "keyText", "keySource", "deviceId", "metaKey", "timestamp",
            "stopPropagation", "getModifierKeyState", "intentionCode" };
        Local<JSValueRef> values[] = { panda::NumberRef::New(vm, static_cast<int32_t>(info.GetKeyType())),
            panda::NumberRef::New(vm, static_cast<int32_t>(info.GetKeyCode())),
            panda::StringRef::NewFromUtf8(vm, info.GetKeyText()),
            panda::NumberRef::New(vm, static_cast<int32_t>(info.GetKeySource())),
            panda::NumberRef::New(vm, info.GetDeviceId()), panda::NumberRef::New(vm, info.GetMetaKey()),
            panda::NumberRef::New(vm, static_cast<double>(info.GetTimeStamp().time_since_epoch().count())),
            panda::FunctionRef::New(vm, Framework::JsStopPropagation),
            panda::FunctionRef::New(vm, ArkTSUtils::JsGetModifierKeyState),
            panda::NumberRef::New(vm, static_cast<int32_t>(info.GetKeyIntention())) };
        auto obj = panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, values);
        obj->SetNativePointerFieldCount(vm, 1);
        obj->SetNativePointerField(vm, 0, static_cast<void*>(&info));
        panda::Local<panda::JSValueRef> params[] = { obj };
        func->Call(vm, func.ToLocal(), params, 1);
    };
    NG::ViewAbstract::SetOnKeyEvent(frameNode, std::move(onKeyEvent));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnKeyEvent(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::DisableOnKeyEvent(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOnKeyPreIme(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onPreImeEvent = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode),
                             containerId](KeyEventInfo& info) -> bool {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        const char* keys[] = { "type", "keyCode", "keyText", "keySource", "deviceId", "metaKey", "timestamp",
            "stopPropagation", "getModifierKeyState", "intentionCode" };
        Local<JSValueRef> values[] = { panda::NumberRef::New(vm, static_cast<int32_t>(info.GetKeyType())),
            panda::NumberRef::New(vm, static_cast<int32_t>(info.GetKeyCode())),
            panda::StringRef::NewFromUtf8(vm, info.GetKeyText()),
            panda::NumberRef::New(vm, static_cast<int32_t>(info.GetKeySource())),
            panda::NumberRef::New(vm, info.GetDeviceId()), panda::NumberRef::New(vm, info.GetMetaKey()),
            panda::NumberRef::New(vm, static_cast<double>(info.GetTimeStamp().time_since_epoch().count())),
            panda::FunctionRef::New(vm, Framework::JsStopPropagation),
            panda::FunctionRef::New(vm, ArkTSUtils::JsGetModifierKeyState),
            panda::NumberRef::New(vm, static_cast<int32_t>(info.GetKeyIntention())) };
        auto obj = panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, values);
        obj->SetNativePointerFieldCount(vm, 1);
        obj->SetNativePointerField(vm, 0, static_cast<void*>(&info));
        panda::Local<panda::JSValueRef> params[] = { obj };
        auto ret = func->Call(vm, func.ToLocal(), params, 1);
        if (ret->IsBoolean()) {
            return ret->ToBoolean(vm)->Value();
        }
        return false;
    };
    NG::ViewAbstractModelNG::SetOnKeyPreIme(frameNode, std::move(onPreImeEvent));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnKeyPreIme(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    NG::ViewAbstractModelNG::DisableOnKeyPreIme(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOnFocus(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onFocus = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode), containerId]() {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        func->Call(vm, func.ToLocal(), nullptr, 0);
    };
    NG::ViewAbstract::SetOnFocus(frameNode, std::move(onFocus));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnFocus(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::DisableOnFocus(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOnBlur(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onBlur = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode), containerId]() {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        func->Call(vm, func.ToLocal(), nullptr, 0);
    };
    NG::ViewAbstract::SetOnBlur(frameNode, std::move(onBlur));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnBlur(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::DisableOnBlur(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOnHover(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onHover = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode), containerId](
                       bool isHover, HoverInfo& hoverInfo) {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        auto isHoverParam = panda::BooleanRef::New(vm, isHover);
        const char* keys[] = {
            "stopPropagation",
            "getModifierKeyState",
            "timestamp",
            "source",
            "target",
        };
        Local<JSValueRef> values[] = { panda::FunctionRef::New(vm, Framework::JsStopPropagation),
            panda::FunctionRef::New(vm, ArkTSUtils::JsGetModifierKeyState),
            panda::NumberRef::New(vm, static_cast<double>(hoverInfo.GetTimeStamp().time_since_epoch().count())),
            panda::NumberRef::New(vm, static_cast<int32_t>(hoverInfo.GetSourceDevice())),
            FrameNodeBridge::CreateEventTargetObject(vm, hoverInfo) };
        auto obj = panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, values);
        obj->SetNativePointerFieldCount(vm, 1);
        obj->SetNativePointerField(vm, 0, static_cast<void*>(&hoverInfo));
        panda::Local<panda::JSValueRef> params[] = { isHoverParam, obj };
        func->Call(vm, func.ToLocal(), params, ArraySize(params));
    };
    NG::ViewAbstract::SetOnHover(frameNode, std::move(onHover));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnHover(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::DisableOnHover(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOnMouse(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onMouse = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode), containerId](
                       MouseInfo& info) {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        auto obj = FrameNodeBridge::CreateMouseInfo(vm, info);
        panda::Local<panda::JSValueRef> params[1] = { obj };
        func->Call(vm, func.ToLocal(), params, 1);
    };
    NG::ViewAbstract::SetOnMouse(frameNode, std::move(onMouse));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnMouse(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::DisableOnMouse(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOnSizeChange(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onSizeChange = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode), containerId](
                            const NG::RectF& oldRect, const NG::RectF& rect) {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        double density = PipelineBase::GetCurrentDensity();
        const char* keys[] = { "width", "height" };
        Local<JSValueRef> oldValues[] = { panda::NumberRef::New(vm, oldRect.Width() / density),
            panda::NumberRef::New(vm, oldRect.Height() / density) };
        auto oldSize = panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, oldValues);
        Local<JSValueRef> newValues[] = { panda::NumberRef::New(vm, rect.Width() / density),
            panda::NumberRef::New(vm, rect.Height() / density) };
        auto newSize = panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keys), keys, newValues);
        panda::Local<panda::JSValueRef> params[2] = { oldSize, newSize };
        func->Call(vm, func.ToLocal(), params, 2);
    };
    NG::ViewAbstract::SetOnSizeChanged(frameNode, std::move(onSizeChange));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnSizeChange(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::SetOnSizeChanged(frameNode, nullptr);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOnAreaChange(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onAreaChange = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode), containerId](
                            const RectF& oldRect, const OffsetF& oldOrigin, const RectF& rect, const OffsetF& origin) {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        auto oldArea = CreateAreaObject(vm, oldRect, oldOrigin);
        auto area = CreateAreaObject(vm, rect, origin);
        panda::Local<panda::JSValueRef> params[2] = { oldArea, area };
        func->Call(vm, func.ToLocal(), params, 2);
    };
    NG::ViewAbstract::SetOnAreaChanged(frameNode, std::move(onAreaChange));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnAreaChange(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::DisableOnAreaChange(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOnGestureJudgeBegin(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> secondeArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(secondeArg->IsFunction(vm), panda::JSValueRef::Undefined(vm));
    auto obj = secondeArg->ToObject(vm);
    auto containerId = Container::CurrentId();
    panda::Local<panda::FunctionRef> func = obj;
    auto onGestureJudgeBegin = [vm, func = panda::CopyableGlobal(vm, func), node = AceType::WeakClaim(frameNode),
                                   containerId](const RefPtr<GestureInfo>& gestureInfo,
                                   const std::shared_ptr<BaseGestureEvent>& info) -> GestureJudgeResult {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(containerId);
        PipelineContext::SetCallBackNode(node);
        auto gestureInfoObj = CreateGestureInfo(vm, gestureInfo);
        auto gestureEventObj = CreateGestureEventInfo(vm, gestureInfo->GetType(), info);
        panda::Local<panda::JSValueRef> params[2] = { gestureInfoObj, gestureEventObj };
        auto returnValue = GestureJudgeResult::CONTINUE;
        auto value = func->Call(vm, func.ToLocal(), params, 2);
        if (value->IsNumber()) {
            returnValue = static_cast<GestureJudgeResult>(value->ToNumber(vm)->Value());
        }
        return returnValue;
    };
    NG::ViewAbstract::SetOnGestureJudgeBegin(frameNode, std::move(onGestureJudgeBegin));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOnGestureJudgeBegin(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    auto* frameNode = GetFrameNode(runtimeCallInfo);
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    ViewAbstract::SetOnGestureJudgeBegin(frameNode, nullptr);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::AddTapGesture(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto* nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    int32_t priority = 0;
    int32_t mask = 0;
    GetGestureCommonValue(runtimeCallInfo, priority, mask);
    int32_t fingers = DEFAULT_TAP_FINGER;
    int32_t count = DEFAULT_TAP_COUNT;
    GetTapGestureValue(runtimeCallInfo, fingers, count, NUM_4);
    auto* gesture = GetArkUINodeModifiers()->getGestureModifier()->createTapGesture(count, fingers, nullptr);
    SetGestureTag(runtimeCallInfo, NUM_3, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::ACTION, NUM_6, gesture);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToNode(nativeNode, gesture, priority, mask);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::AddLongPressGesture(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto* nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    int32_t priority = 0;
    int32_t mask = 0;
    GetGestureCommonValue(runtimeCallInfo, priority, mask);
    int32_t fingers = DEFAULT_LONG_PRESS_FINGER;
    bool repeat = false;
    int32_t duration = DEFAULT_LONG_PRESS_DURATION;
    GetLongPressGestureValue(runtimeCallInfo, fingers, repeat, duration, NUM_4);
    auto* gesture = GetArkUINodeModifiers()->getGestureModifier()->createLongPressGesture(
        fingers, repeat, duration, nullptr);
    SetGestureTag(runtimeCallInfo, NUM_3, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::ACTION, NUM_7, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::END, NUM_8, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::CANCEL, NUM_9, gesture);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToNode(nativeNode, gesture, priority, mask);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::AddPanGesture(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto* nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    int32_t priority = 0;
    int32_t mask = 0;
    GetGestureCommonValue(runtimeCallInfo, priority, mask);
    int32_t fingers = DEFAULT_PAN_FINGER;
    int32_t direction = PanDirection::ALL;
    double distance = DEFAULT_PAN_DISTANCE.ConvertToPx();
    GetPanGestureValue(runtimeCallInfo, fingers, direction, distance, NUM_4);
    auto* gesture = GetArkUINodeModifiers()->getGestureModifier()->createPanGesture(
        fingers, direction, distance, nullptr);
    SetGestureTag(runtimeCallInfo, NUM_3, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::START, NUM_7, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::UPDATE, NUM_8, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::END, NUM_9, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::CANCEL, NUM_10, gesture);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToNode(nativeNode, gesture, priority, mask);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::AddSwipeGesture(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto* nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    int32_t priority = 0;
    int32_t mask = 0;
    GetGestureCommonValue(runtimeCallInfo, priority, mask);
    int32_t fingers = DEFAULT_SLIDE_FINGER;
    int32_t direction = SwipeDirection::ALL;
    double speed = DEFAULT_SLIDE_SPEED;
    GetSwipeGestureValue(runtimeCallInfo, fingers, direction, speed, NUM_4);
    auto* gesture =
        GetArkUINodeModifiers()->getGestureModifier()->createSwipeGestureByModifier(fingers, direction, speed);
    SetGestureTag(runtimeCallInfo, NUM_3, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::ACTION, NUM_7, gesture);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToNode(nativeNode, gesture, priority, mask);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::AddPinchGesture(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto* nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    int32_t priority = 0;
    int32_t mask = 0;
    GetGestureCommonValue(runtimeCallInfo, priority, mask);
    int32_t fingers = DEFAULT_PINCH_FINGER;
    double distance = DEFAULT_PINCH_DISTANCE;
    GetPinchGestureValue(runtimeCallInfo, fingers, distance, NUM_4);
    auto* gesture = GetArkUINodeModifiers()->getGestureModifier()->createPinchGesture(fingers, distance, nullptr);
    SetGestureTag(runtimeCallInfo, NUM_3, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::START, NUM_6, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::UPDATE, NUM_7, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::END, NUM_8, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::CANCEL, NUM_9, gesture);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToNode(nativeNode, gesture, priority, mask);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::AddRotationGesture(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto* nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    int32_t priority = 0;
    int32_t mask = 0;
    GetGestureCommonValue(runtimeCallInfo, priority, mask);
    int32_t fingers = DEFAULT_ROTATION_FINGER;
    double angle = DEFAULT_ROTATION_ANGLE;
    GetRotationGestureValue(runtimeCallInfo, fingers, angle, NUM_4);
    auto* gesture = GetArkUINodeModifiers()->getGestureModifier()->createRotationGesture(fingers, angle, nullptr);
    SetGestureTag(runtimeCallInfo, NUM_3, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::START, NUM_6, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::UPDATE, NUM_7, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::END, NUM_8, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::CANCEL, NUM_9, gesture);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToNode(nativeNode, gesture, priority, mask);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::AddGestureGroup(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    int32_t mode = 2;
    GetGestureModeValue(runtimeCallInfo, mode, NUM_2);
    auto* gesture = GetArkUINodeModifiers()->getGestureModifier()->createGestureGroup(mode);
    SetGestureTag(runtimeCallInfo, NUM_0, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::CANCEL, NUM_1, gesture);
    return panda::NativePointerRef::New(vm, gesture);
}

ArkUINativeModuleValue CommonBridge::AddTapGestureToGroup(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    int32_t fingers = DEFAULT_TAP_FINGER;
    int32_t count = DEFAULT_TAP_COUNT;
    GetTapGestureValue(runtimeCallInfo, fingers, count, NUM_1);
    auto* gesture = GetArkUINodeModifiers()->getGestureModifier()->createTapGesture(count, fingers, nullptr);
    SetGestureTag(runtimeCallInfo, NUM_0, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::ACTION, NUM_3, gesture);
    auto* group = GetGestureGroup(runtimeCallInfo, NUM_4);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToGestureGroup(group, gesture);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::AddLongPressGestureToGroup(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    int32_t fingers = DEFAULT_LONG_PRESS_FINGER;
    bool repeat = false;
    int32_t duration = DEFAULT_LONG_PRESS_DURATION;
    GetLongPressGestureValue(runtimeCallInfo, fingers, repeat, duration, NUM_1);
    auto* gesture = GetArkUINodeModifiers()->getGestureModifier()->createLongPressGesture(
        fingers, repeat, duration, nullptr);
    SetGestureTag(runtimeCallInfo, NUM_0, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::ACTION, NUM_4, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::END, NUM_5, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::CANCEL, NUM_6, gesture);
    auto* group = GetGestureGroup(runtimeCallInfo, NUM_7);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToGestureGroup(group, gesture);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::AddPanGestureToGroup(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    int32_t fingers = DEFAULT_PAN_FINGER;
    int32_t direction = PanDirection::ALL;
    double distance = DEFAULT_PAN_DISTANCE.ConvertToPx();
    GetPanGestureValue(runtimeCallInfo, fingers, direction, distance, NUM_1);
    auto* gesture = GetArkUINodeModifiers()->getGestureModifier()->createPanGesture(
        fingers, direction, distance, nullptr);
    SetGestureTag(runtimeCallInfo, NUM_0, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::START, NUM_4, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::UPDATE, NUM_5, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::END, NUM_6, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::CANCEL, NUM_7, gesture);
    auto* group = GetGestureGroup(runtimeCallInfo, NUM_8);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToGestureGroup(group, gesture);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::AddSwipeGestureToGroup(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    int32_t fingers = DEFAULT_SLIDE_FINGER;
    int32_t direction = SwipeDirection::ALL;
    double speed = DEFAULT_SLIDE_SPEED;
    GetSwipeGestureValue(runtimeCallInfo, fingers, direction, speed, NUM_1);
    auto* gesture =
        GetArkUINodeModifiers()->getGestureModifier()->createSwipeGestureByModifier(fingers, direction, speed);
    SetGestureTag(runtimeCallInfo, NUM_0, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::ACTION, NUM_4, gesture);
    auto* group = GetGestureGroup(runtimeCallInfo, NUM_5);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToGestureGroup(group, gesture);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::AddPinchGestureToGroup(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    int32_t fingers = DEFAULT_PINCH_FINGER;
    double distance = DEFAULT_PINCH_DISTANCE;
    GetPinchGestureValue(runtimeCallInfo, fingers, distance, NUM_1);
    auto* gesture = GetArkUINodeModifiers()->getGestureModifier()->createPinchGesture(fingers, distance, nullptr);
    SetGestureTag(runtimeCallInfo, NUM_0, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::START, NUM_3, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::UPDATE, NUM_4, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::END, NUM_5, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::CANCEL, NUM_6, gesture);
    auto* group = GetGestureGroup(runtimeCallInfo, NUM_7);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToGestureGroup(group, gesture);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::AddRotationGestureToGroup(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    int32_t fingers = DEFAULT_ROTATION_FINGER;
    double angle = DEFAULT_ROTATION_ANGLE;
    GetRotationGestureValue(runtimeCallInfo, fingers, angle, NUM_1);
    auto* gesture = GetArkUINodeModifiers()->getGestureModifier()->createRotationGesture(fingers, angle, nullptr);
    SetGestureTag(runtimeCallInfo, NUM_0, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::START, NUM_3, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::UPDATE, NUM_4, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::END, NUM_5, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::CANCEL, NUM_6, gesture);
    auto* group = GetGestureGroup(runtimeCallInfo, NUM_7);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToGestureGroup(group, gesture);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::AddGestureGroupToGroup(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    int32_t mode = 2;
    GetGestureModeValue(runtimeCallInfo, mode, NUM_2);
    auto* gesture = GetArkUINodeModifiers()->getGestureModifier()->createGestureGroup(mode);
    SetGestureTag(runtimeCallInfo, NUM_0, gesture);
    SetOnGestureEvent(runtimeCallInfo, GestureEventAction::CANCEL, NUM_1, gesture);
    auto* group = GetGestureGroup(runtimeCallInfo, NUM_3);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToGestureGroup(group, gesture);
    return panda::NativePointerRef::New(vm, gesture);
}

ArkUINativeModuleValue CommonBridge::AttachGestureGroup(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto* nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    int32_t priority = 0;
    int32_t mask = 0;
    GetGestureCommonValue(runtimeCallInfo, priority, mask);
    auto* group = GetGestureGroup(runtimeCallInfo, NUM_3);
    GetArkUINodeModifiers()->getGestureModifier()->addGestureToNode(nativeNode, group, priority, mask);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::RemoveGestureByTag(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto* nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> gestureTagArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    if (!gestureTagArg.IsNull() && !gestureTagArg->IsUndefined()) {
        auto tag = gestureTagArg->ToString(vm)->ToString();
        GetArkUINodeModifiers()->getGestureModifier()->removeGestureFromNodeByTag(nativeNode, tag.c_str());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ClearGestures(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto* nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    GetArkUINodeModifiers()->getGestureModifier()->clearGestures(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::GetWindowName(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(context, panda::JSValueRef::Undefined(vm));
    std::string windowName = context->GetWindow()->GetWindowName();
    return panda::StringRef::NewFromUtf8(vm, windowName.c_str());
}

ArkUINativeModuleValue CommonBridge::SetSystemBarEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto* nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> enableArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    if (enableArg->IsBoolean()) {
        auto enable = enableArg->ToBoolean(vm);
        GetArkUINodeModifiers()->getCommonModifier()->setSystemBarEffect(nativeNode, enable->Value());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::PostFrameCallback(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));

    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0); // frameCallback
    if (firstArg->IsNull() || firstArg->IsUndefined() || !firstArg->IsObject(vm)) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto frameCallback = firstArg->ToObject(vm);

    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    int64_t delayMillis = 0;
    if (secondArg->IsNumber()) {
        delayMillis = secondArg->IntegerValue(vm);
    }

    FrameCallbackFunc frameCallbackFunc = [vm, frameCallbackObj = panda::CopyableGlobal(vm, frameCallback),
                                              delayMillis](int64_t nanoTimestamp) -> void {
        if (frameCallbackObj->IsNull() || frameCallbackObj->IsUndefined() || !frameCallbackObj->IsObject(vm)) {
            return;
        }
        Local<FunctionRef> onFrameFunc = frameCallbackObj->Get(vm, "onFrame");
        if (!onFrameFunc->IsFunction(vm)) {
            return;
        }

        auto nanoTimestampRef = NumberRef::New(vm, nanoTimestamp);
        Local<JSValueRef> params[] = { nanoTimestampRef };
        onFrameFunc->Call(vm, frameCallbackObj.ToLocal(), params, 1);
    };

    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, panda::JSValueRef::Undefined(vm));
    if (delayMillis <= 0) {
        context->AddFrameCallback(std::move(frameCallbackFunc));
    } else {
        auto taskScheduler = context->GetTaskExecutor();
        CHECK_NULL_RETURN(taskScheduler, panda::JSValueRef::Undefined(vm));
        taskScheduler->PostDelayedTask(
            [context, callbackFunc = std::move(frameCallbackFunc)]() -> void {
                auto callback = const_cast<FrameCallbackFunc&>(callbackFunc);
                context->AddFrameCallback(std::move(callback));
            },
            TaskExecutor::TaskType::UI, delayMillis, "ArkUIPostFrameCallbackFuncDelayed");
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetFocusScopeId(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    GetArkUINodeModifiers()->getCommonModifier()->resetFocusScopeId(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetFocusScopeId(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> idArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> isGroupArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    if (!idArg->IsString(vm)) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto id = idArg->ToString(vm)->ToString();
    bool isGroup = false;
    if (isGroupArg->IsBoolean()) {
        isGroup = isGroupArg->ToBoolean(vm)->Value();
    }
    GetArkUINodeModifiers()->getCommonModifier()->setFocusScopeId(nativeNode, id.c_str(), isGroup);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetFocusScopePriority(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    GetArkUINodeModifiers()->getCommonModifier()->resetFocusScopePriority(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetFocusScopePriority(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(!firstArg.IsNull(), panda::JSValueRef::Undefined(vm));
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> scopeIdArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> priorityArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    if (!scopeIdArg->IsString(vm)) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto scopeId = scopeIdArg->ToString(vm)->ToString();
    int32_t priority = 0;
    if (priorityArg->IsNumber()) {
        priority = priorityArg->Int32Value(vm);
    }
    GetArkUINodeModifiers()->getCommonModifier()->setFocusScopePriority(nativeNode, scopeId.c_str(), priority);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue CommonBridge::SetPixelRound(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    auto startArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto topArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto endArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    auto bottomArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    if (startArg->IsUndefined() && topArg->IsUndefined() && endArg->IsUndefined() && bottomArg->IsUndefined()) {
        GetArkUINodeModifiers()->getCommonModifier()->resetPixelRound(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    int32_t values[] = { -1, -1, -1, -1 };
    if (startArg->IsInt()) {
        values[NUM_0] = startArg->Int32Value(vm);
    }
    if (topArg->IsInt()) {
        values[NUM_1] = topArg->Int32Value(vm);
    }
    if (endArg->IsInt()) {
        values[NUM_2] = endArg->Int32Value(vm);
    }
    if (bottomArg->IsInt()) {
        values[NUM_3] = bottomArg->Int32Value(vm);
    }
    GetArkUINodeModifiers()->getCommonModifier()->setPixelRound(
        nativeNode, values, (sizeof(values) / sizeof(values[NUM_0])));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetPixelRound(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getCommonModifier()->resetPixelRound(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::GreatOrEqualAPITargetVersion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));

    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    if (firstArg->IsNull() || firstArg->IsUndefined() || !firstArg->IsNumber()) {
        return panda::BooleanRef::New(vm, false);
    }
    int32_t apiTargetVersion = firstArg->Int32Value(vm);
    auto platformVersion = static_cast<PlatformVersion>(apiTargetVersion);
    return panda::BooleanRef::New(vm, Container::GreatOrEqualAPITargetVersion(platformVersion));
}

ArkUINativeModuleValue CommonBridge::LessThanAPITargetVersion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));

    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    if (firstArg->IsNull() || firstArg->IsUndefined() || !firstArg->IsNumber()) {
        return panda::BooleanRef::New(vm, false);
    }
    int32_t apiTargetVersion = firstArg->Int32Value(vm);
    auto platformVersion = static_cast<PlatformVersion>(apiTargetVersion);
    return panda::BooleanRef::New(vm, Container::LessThanAPITargetVersion(platformVersion));
}

ArkUINativeModuleValue CommonBridge::GetApiTargetVersion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));

    auto container = Container::CurrentSafely();
    CHECK_NULL_RETURN(container, panda::JSValueRef::Undefined(vm));
    int32_t apiTargetVersion = container->GetApiTargetVersion();
    return panda::NumberRef::New(vm, apiTargetVersion);
}
} // namespace OHOS::Ace::NG
