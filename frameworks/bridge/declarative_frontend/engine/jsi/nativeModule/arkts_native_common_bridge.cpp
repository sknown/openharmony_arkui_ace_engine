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
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_common_bridge.h"

#include "bridge/declarative_frontend/engine/jsi/jsi_types.h"
#include "bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "bridge/declarative_frontend/engine/jsi/components/arkts_native_api.h"
#include "frameworks/base/geometry/calc_dimension.h"
#include "frameworks/base/geometry/dimension.h"
#include "frameworks/bridge/declarative_frontend/engine/js_types.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/jsi_value_conversions.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_shape_abstract.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"
using namespace OHOS::Ace::Framework;

namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t COLOR_ALPHA_OFFSET = 24;
constexpr uint32_t COLOR_ALPHA_VALUE = 0xFF000000;
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
constexpr int NUM_11 = 11;
constexpr int NUM_12 = 12;
constexpr int NUM_13 = 13;
constexpr int NUM_14 = 14;
constexpr int NUM_15 = 15;
constexpr int NUM_16 = 16;
constexpr int SIZE_OF_TWO = 2;
constexpr int SIZE_OF_THREE = 3;
constexpr int SIZE_OF_FOUR = 4;
constexpr int SIZE_OF_FIVE = 5;
constexpr int SIZE_OF_EIGHT = 8;
constexpr int32_t ALIGN_RULES_NUM = 6;
constexpr int SIZE_ARRAY_NUM = 2;
constexpr int32_t ALIGN_DIRECTION_DEFAULT = 2;
constexpr double FULL_DIMENSION = 100.0;
constexpr double HALF_DIMENSION = 50.0;

uint32_t ColorAlphaAdapt(uint32_t origin)
{
    uint32_t result = origin;
    if ((origin >> COLOR_ALPHA_OFFSET) == 0) {
        result = origin | COLOR_ALPHA_VALUE;
    }
    return result;
}

bool ParseJsDimensionVp(const EcmaVM *vm, const Local<JSValueRef> &value, CalcDimension &result)
{
    if (value->IsNumber()) {
        result = CalcDimension(value->ToNumber(vm)->Value(), DimensionUnit::VP);
        return true;
    }
    if (value->IsString()) {
        result = StringUtils::StringToCalcDimension(value->ToString(vm)->ToString(), false, DimensionUnit::VP);
        return true;
    }

    return false;
}

bool ParseJsColor(const EcmaVM *vm, const Local<JSValueRef> &value, Color &result)
{
    if (value->IsNumber()) {
        result = Color(ColorAlphaAdapt(value->Uint32Value(vm)));
        return true;
    }
    if (value->IsString()) {
        return Color::ParseColorString(value->ToString(vm)->ToString(), result);
    }

    return false;
}

bool ParseJsInteger(const EcmaVM *vm, const Local<JSValueRef> &value, int32_t &result)
{
    if (value->IsNumber()) {
        result = value->Int32Value(vm);
        return true;
    }

    return false;
}

bool ParseJsDouble(const EcmaVM *vm, const Local<JSValueRef> &value, double &result)
{
    if (value->IsNumber()) {
        result = value->ToNumber(vm)->Value();
        return true;
    }
    if (value->IsString()) {
        return StringUtils::StringToDouble(value->ToString(vm)->ToString(), result);
    }

    return false;
}

void ParseAllBorder(const EcmaVM *vm, const Local<JSValueRef> &args, CalcDimension &result)
{
    if (ParseJsDimensionVp(vm, args, result) && result.IsNonNegative()) {
        if (result.Unit() == DimensionUnit::PERCENT) {
            result.Reset();
        }
    }
}

bool ParseJsDimensionNG(const EcmaVM *vm, const Local<JSValueRef> &jsValue, CalcDimension &result,
    DimensionUnit defaultUnit, bool isSupportPercent = true)
{
    if (jsValue->IsNumber()) {
        result = CalcDimension(jsValue->ToNumber(vm)->Value(), defaultUnit);
        return true;
    }
    if (jsValue->IsString()) {
        auto value = jsValue->ToString(vm)->ToString();
        if (value.back() == '%' && !isSupportPercent) {
            return false;
        }
        return StringUtils::StringToCalcDimensionNG(jsValue->ToString(vm)->ToString(), result, false, defaultUnit);
    }

    return false;
}

bool ParseJsDimensionVpNG(const EcmaVM *vm, const Local<JSValueRef> &jsValue, CalcDimension &result,
    bool isSupportPercent = true)
{
    return ParseJsDimensionNG(vm, jsValue, result, DimensionUnit::VP, isSupportPercent);
}

bool ParseJsInt32(const EcmaVM *vm, const Local<JSValueRef> &value, int32_t &result)
{
    if (value->IsNumber()) {
        result = value->Int32Value(vm);
        return true;
    }
    if (value->IsString()) {
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
    if (value->IsString()) {
        angle = static_cast<float>(StringUtils::StringToDegree(value->ToString(vm)->ToString()));
        return;
    }
    return;
}

void ParseGradientAngle(const EcmaVM *vm, const Local<JSValueRef> &value, std::vector<double> &values)
{
    std::optional<float> degree;
    ParseJsAngle(vm, value, degree);
    auto angleHasValue = degree.has_value();
    auto angleValue = angleHasValue ? degree.value() : 0.0f;
    degree.reset();
    values.push_back(static_cast<double>(angleHasValue));
    values.push_back(static_cast<double>(angleValue));
}

void ParseGradientColorStops(const EcmaVM *vm, const Local<JSValueRef> &value, std::vector<double> &colors)
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
        if (!ParseJsColor(vm, colorParams, color)) {
            continue;
        }
        bool hasDimension = false;
        double dimension = 0.0;
        if (itemLength > NUM_1) {
            auto stopDimension = panda::ArrayRef::GetValueAt(vm, itemArray, NUM_1);
            if (ParseJsDouble(vm, stopDimension, dimension)) {
                hasDimension = true;
            }
        }
        colors.push_back(static_cast<double>(color.GetValue()));
        colors.push_back(static_cast<double>(hasDimension));
        colors.push_back(dimension);
    }
}

bool ParseJsDoublePair(const EcmaVM *vm, const Local<JSValueRef> &value, double &first, double &second)
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
    first = firstArg->ToNumber(vm)->Value();
    second = secondArg->ToNumber(vm)->Value();
    return true;
}

void ParseGradientCenter(const EcmaVM *vm, const Local<JSValueRef> &value, std::vector<double> &values)
{
    bool hasValueX = false;
    bool hasValueY = false;
    CalcDimension valueX;
    CalcDimension valueY;
    if (value->IsArray(vm)) {
        auto array = panda::Local<panda::ArrayRef>(value);
        auto length = array->Length(vm);
        if (length == NUM_2) {
            hasValueX = ParseJsDimensionVp(vm, panda::ArrayRef::GetValueAt(vm, array, NUM_0), valueX);
            hasValueY = ParseJsDimensionVp(vm, panda::ArrayRef::GetValueAt(vm, array, NUM_1), valueY);
        }
    }
    values.push_back(static_cast<double>(hasValueX));
    values.push_back(static_cast<double>(valueX.Value()));
    values.push_back(static_cast<double>(valueX.Unit()));
    values.push_back(static_cast<double>(hasValueY));
    values.push_back(static_cast<double>(valueY.Value()));
    values.push_back(static_cast<double>(valueY.Unit()));
}

void ParseBorderWidth(ArkUIRuntimeCallInfo *runtimeCallInfo, EcmaVM *vm, double values[], int units[], int size)
{
    if (size != NUM_8) {
        return;
    }
    Local<JSValueRef> leftArgs = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> rightArgs = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> topArgs = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> bottomArgs = runtimeCallInfo->GetCallArgRef(NUM_4);

    CalcDimension left;
    CalcDimension right;
    CalcDimension top;
    CalcDimension bottom;

    ParseAllBorder(vm, leftArgs, left);
    ParseAllBorder(vm, rightArgs, right);
    ParseAllBorder(vm, topArgs, top);
    ParseAllBorder(vm, bottomArgs, bottom);

    values[NUM_0] = left.Value();
    units[NUM_0] = static_cast<int>(left.Unit());
    values[NUM_1] = right.Value();
    units[NUM_1] = static_cast<int>(right.Unit());
    values[NUM_2] = top.Value();
    units[NUM_2] = static_cast<int>(top.Unit());
    values[NUM_3] = bottom.Value();
    units[NUM_3] = static_cast<int>(bottom.Unit());
}

void ParseBorderColor(ArkUIRuntimeCallInfo *runtimeCallInfo, EcmaVM *vm, uint32_t arr[], int size)
{
    if (size != NUM_8) {
        return;
    }
    Local<JSValueRef> leftArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    Local<JSValueRef> rifghtArg = runtimeCallInfo->GetCallArgRef(NUM_6);
    Local<JSValueRef> topArg = runtimeCallInfo->GetCallArgRef(NUM_7);
    Local<JSValueRef> bottomArg = runtimeCallInfo->GetCallArgRef(NUM_8);

    uint32_t leftColor = leftArg->Uint32Value(vm);
    uint32_t rightColor = rifghtArg->Uint32Value(vm);
    uint32_t topColor = topArg->Uint32Value(vm);
    uint32_t bottomColor = bottomArg->Uint32Value(vm);
    arr[NUM_0] = leftColor;
    arr[NUM_1] = rightColor;
    arr[NUM_2] = topColor;
    arr[NUM_3] = bottomColor;
}

void ParseBorderRadius(ArkUIRuntimeCallInfo *runtimeCallInfo, EcmaVM *vm, double values[], int units[], int size)
{
    if (size != NUM_8) {
        return;
    }
    Local<JSValueRef> topLeftArgs = runtimeCallInfo->GetCallArgRef(NUM_9);
    Local<JSValueRef> topRightArgs = runtimeCallInfo->GetCallArgRef(NUM_10);
    Local<JSValueRef> bottomLeftArgs = runtimeCallInfo->GetCallArgRef(NUM_11);
    Local<JSValueRef> bottomRightArgs = runtimeCallInfo->GetCallArgRef(NUM_12);

    CalcDimension topLeft;
    CalcDimension topRight;
    CalcDimension bottomLeft;
    CalcDimension bottomRight;

    ParseAllBorder(vm, topLeftArgs, topLeft);
    ParseAllBorder(vm, topRightArgs, topRight);
    ParseAllBorder(vm, bottomLeftArgs, bottomLeft);
    ParseAllBorder(vm, bottomRightArgs, bottomRight);

    values[NUM_4] = topLeft.Value();
    units[NUM_4] = static_cast<int>(topLeft.Unit());
    values[NUM_5] = topRight.Value();
    units[NUM_5] = static_cast<int>(topRight.Unit());
    values[NUM_6] = bottomLeft.Value();
    units[NUM_6] = static_cast<int>(bottomLeft.Unit());
    values[NUM_7] = bottomRight.Value();
    units[NUM_7] = static_cast<int>(bottomRight.Unit());
}

void ParseBorderStyle(ArkUIRuntimeCallInfo *runtimeCallInfo, EcmaVM *vm, uint32_t styles[], int size)
{
    if (size != NUM_8) {
        return;
    }
    auto topArg = runtimeCallInfo->GetCallArgRef(NUM_13);
    auto rightArg = runtimeCallInfo->GetCallArgRef(NUM_14);
    auto bottomArg = runtimeCallInfo->GetCallArgRef(NUM_15);
    auto leftArg = runtimeCallInfo->GetCallArgRef(NUM_16);

    styles[NUM_4] = topArg->Int32Value(vm);
    styles[NUM_5] = rightArg->Int32Value(vm);
    styles[NUM_6] = bottomArg->Int32Value(vm);
    styles[NUM_7] = leftArg->Int32Value(vm);
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
    if (jsValue->IsString()) {
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

void ParseRotate(ArkUIRuntimeCallInfo *runtimeCallInfo, double values[], int units[])
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    Local<JSValueRef> xDirectionArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> yDirectionArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> zDirectionArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> angleArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    Local<JSValueRef> centerXArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    Local<JSValueRef> centerYArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> centerZArg = runtimeCallInfo->GetCallArgRef(NUM_6);
    Local<JSValueRef> perspectiveArg = runtimeCallInfo->GetCallArgRef(NUM_7);
    float xDirection = 0.0f;
    float yDirection = 0.0f;
    float zDirection = 0.0f;
    float angle = 0.0f;
    CalcDimension centerX = 0.5_pct;
    CalcDimension centerY = 0.5_pct;
    CalcDimension centerZ = CalcDimension(0.0f, DimensionUnit::VP);
    float perspective = 0.0f;
    if (!xDirectionArg->IsNumber() && !yDirectionArg->IsNumber() && !zDirectionArg->IsNumber()) {
        xDirection = 0.0f;
        yDirection = 0.0f;
        zDirection = 1.0f;
    }
    ParseDirection(vm, xDirectionArg, xDirection);
    ParseDirection(vm, yDirectionArg, yDirection);
    ParseDirection(vm, zDirectionArg, zDirection);
    if (angleArg->IsString()) {
        angle = static_cast<float>(StringUtils::StringToDegree(angleArg->ToString(vm)->ToString()));
    } else if (angleArg->IsNumber()) {
        angle = static_cast<float>(angleArg->ToNumber(vm)->Value());
    }
    ParseAxisDimensionVp(vm, centerXArg, centerX, true);
    ParseAxisDimensionVp(vm, centerYArg, centerY, true);
    ParseAxisDimensionVp(vm, centerZArg, centerZ, true);
    if (perspectiveArg->IsNumber()) {
        perspective = static_cast<float>(perspectiveArg->ToNumber(vm)->Value());
    }
    values[NUM_0] = centerX.Value();
    units[NUM_0] = static_cast<int>(centerX.Unit());
    values[NUM_1] = centerY.Value();
    units[NUM_1] = static_cast<int>(centerY.Unit());
    values[NUM_2] = centerZ.Value();
    units[NUM_2] = static_cast<int>(centerZ.Unit());
    values[NUM_3] = xDirection;
    values[NUM_4] = yDirection;
    values[NUM_5] = zDirection;
    values[NUM_6] = angle;
    values[NUM_7] = perspective;
}

bool ParseCalcDimension(const EcmaVM* vm,
    NodeHandle node, const Local<JSValueRef>& value, CalcDimension& result, bool isWidth)
{
    CHECK_NULL_RETURN(vm, false);
    bool undefined = value->IsUndefined();
    if (undefined) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ClearWidthOrHeight(node, isWidth);
        return false;
    }
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
        if (!ParseJsDimensionVpNG(vm, value, result)) {
            GetArkUIInternalNodeAPI()->GetCommonModifier().ClearWidthOrHeight(node, isWidth);
            return false;
        }
    } else if (!ParseJsDimensionVp(vm, value, result)) {
        return false;
    }

    if (LessNotEqual(result.Value(), 0.0)) {
        result.SetValue(0.0);
    }
    return true;
}

bool ParseJsAlignRule(const EcmaVM* vm, const Local<JSValueRef> &arg, std::string& anchor, int8_t &direction)
{
    if (arg->IsString()) {
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
} // namespace

ArkUINativeModuleValue CommonBridge::SetBackgroundColor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    Color color;
    if (!ArkTSUtils::ParseJsColorAlpha(vm, secondArg, color)) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBackgroundColor(nativeNode);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetBackgroundColor(nativeNode, color.GetValue());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundColor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBackgroundColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBorderWidth(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    Local<JSValueRef> leftArgs = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> rightArgs = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> topArgs = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> bottomArgs = runtimeCallInfo->GetCallArgRef(NUM_4);
    if (leftArgs->IsUndefined() && rightArgs->IsUndefined() && topArgs->IsUndefined() && bottomArgs->IsUndefined()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBorderWidth(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    CalcDimension left;
    CalcDimension right;
    CalcDimension top;
    CalcDimension bottom;

    ParseAllBorder(vm, leftArgs, left);
    ParseAllBorder(vm, rightArgs, right);
    ParseAllBorder(vm, topArgs, top);
    ParseAllBorder(vm, bottomArgs, bottom);

    uint32_t size = SIZE_OF_FOUR;
    double values[size];
    int units[size];

    values[NUM_0] = left.Value();
    units[NUM_0] = static_cast<int>(left.Unit());
    values[NUM_1] = right.Value();
    units[NUM_1] = static_cast<int>(right.Unit());
    values[NUM_2] = top.Value();
    units[NUM_2] = static_cast<int>(top.Unit());
    values[NUM_3] = bottom.Value();
    units[NUM_3] = static_cast<int>(bottom.Unit());

    GetArkUIInternalNodeAPI()->GetCommonModifier().SetBorderWidth(nativeNode, values, units, size);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBorderWidth(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBorderWidth(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBorderRadius(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    Local<JSValueRef> topLeftArgs = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> topRightArgs = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> bottomLeftArgs = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> bottomRightArgs = runtimeCallInfo->GetCallArgRef(NUM_4);
    if (!topLeftArgs->IsString() && !topLeftArgs->IsNumber() && !topRightArgs->IsString() &&
        !topRightArgs->IsNumber() && !bottomLeftArgs->IsString() && !bottomLeftArgs->IsNumber() &&
        !bottomRightArgs->IsString() && !bottomRightArgs->IsNumber()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBorderRadius(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    CalcDimension topLeft;
    CalcDimension topRight;
    CalcDimension bottomLeft;
    CalcDimension bottomRight;

    ParseAllBorder(vm, topLeftArgs, topLeft);
    ParseAllBorder(vm, topRightArgs, topRight);
    ParseAllBorder(vm, bottomLeftArgs, bottomLeft);
    ParseAllBorder(vm, bottomRightArgs, bottomRight);

    uint32_t size = SIZE_OF_FOUR;
    double values[size];
    int units[size];

    values[NUM_0] = topLeft.Value();
    units[NUM_0] = static_cast<int>(topLeft.Unit());
    values[NUM_1] = topRight.Value();
    units[NUM_1] = static_cast<int>(topRight.Unit());
    values[NUM_2] = bottomLeft.Value();
    units[NUM_2] = static_cast<int>(bottomLeft.Unit());
    values[NUM_3] = bottomRight.Value();
    units[NUM_3] = static_cast<int>(bottomRight.Unit());

    GetArkUIInternalNodeAPI()->GetCommonModifier().SetBorderRadius(nativeNode, values, units, SIZE_OF_FOUR);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBorderRadius(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBorderRadius(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    Local<JSValueRef> jsValue = runtimeCallInfo->GetCallArgRef(NUM_1);

    CalcDimension width;
    std::string calcStr;
    if (jsValue->IsUndefined() || !ParseJsDimensionVpNG(vm, jsValue, width)) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetWidth(nativeNode);
    } else {
        if (LessNotEqual(width.Value(), 0.0)) {
            width.SetValue(0.0);
        }

        if (width.Unit() == DimensionUnit::CALC) {
            GetArkUIInternalNodeAPI()->GetCommonModifier().SetWidth(
                nativeNode, 0, static_cast<int>(width.Unit()), width.CalcValue().c_str());
        } else {
            GetArkUIInternalNodeAPI()->GetCommonModifier().SetWidth(
                nativeNode, width.Value(), static_cast<int>(width.Unit()), calcStr.c_str());
        }
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetWidth(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetHeight(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    Local<JSValueRef> jsValue = runtimeCallInfo->GetCallArgRef(NUM_1);
    CalcDimension height;
    std::string calcStr;
    if (jsValue->IsUndefined() || !ParseJsDimensionVpNG(vm, jsValue, height)) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetHeight(nativeNode);
    } else {
        if (LessNotEqual(height.Value(), 0.0)) {
            height.SetValue(0.0);
        }
        if (height.Unit() == DimensionUnit::CALC) {
            GetArkUIInternalNodeAPI()->GetCommonModifier().SetHeight(
                nativeNode, height.Value(), static_cast<int>(height.Unit()), height.CalcValue().c_str());
        } else {
            GetArkUIInternalNodeAPI()->GetCommonModifier().SetHeight(
                nativeNode, height.Value(), static_cast<int>(height.Unit()), calcStr.c_str());
        }
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetHeight(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetHeight(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetPosition(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    Local<JSValueRef> sizeX = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> sizeY = runtimeCallInfo->GetCallArgRef(NUM_2);

    CalcDimension x;
    CalcDimension y;
    bool hasX = ParseJsDimensionVp(vm, sizeX, x);
    bool hasY = ParseJsDimensionVp(vm, sizeY, y);
    if (!hasX && !hasY) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetPosition(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetPosition(nativeNode, x.Value(), static_cast<int>(x.Unit()),
        y.Value(), static_cast<int>(y.Unit()));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetPosition(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetPosition(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetTransform(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    Local<JSValueRef> jsValue = runtimeCallInfo->GetCallArgRef(NUM_1);

    if (!jsValue->IsArray(vm)) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetTransform(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    const auto matrix4Len = Matrix4::DIMENSION * Matrix4::DIMENSION;
    float matrix[matrix4Len];
    Local<panda::ArrayRef> transArray = static_cast<Local<panda::ArrayRef>>(jsValue);
    for (size_t i = 0; i < transArray->Length(vm); i++) {
        Local<JSValueRef> value = transArray->GetValueAt(vm, jsValue, i);
        matrix[i] = value->ToNumber(vm)->Value();
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetTransform(nativeNode, matrix, matrix4Len);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetTransform(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();

    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetTransform(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBorderColor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> leftArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> rifghtArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> topArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> bottomArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();

    Color leftColor;
    Color rightColor;
    Color topColor;
    Color bottomColor;

    if (!ArkTSUtils::ParseJsColorAlpha(vm, leftArg, leftColor)) {
        leftColor.SetValue(COLOR_ALPHA_VALUE);
    }
    if (!ArkTSUtils::ParseJsColorAlpha(vm, rifghtArg, rightColor)) {
        rightColor.SetValue(COLOR_ALPHA_VALUE);
    }
    if (!ArkTSUtils::ParseJsColorAlpha(vm, topArg, topColor)) {
        topColor.SetValue(COLOR_ALPHA_VALUE);
    }
    if (!ArkTSUtils::ParseJsColorAlpha(vm, bottomArg, bottomColor)) {
        bottomColor.SetValue(COLOR_ALPHA_VALUE);
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetBorderColor(nativeNode, leftColor.GetValue(),
        rightColor.GetValue(), topColor.GetValue(), bottomColor.GetValue());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBorderColor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBorderColor(nativeNode);
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
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    if ((!typeArg->IsBoolean()) || (!typeArg->BooleaValue())) {
        int32_t styles[] = { static_cast<int32_t>(BorderStyle::SOLID) };
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetBorderStyle(nativeNode, styles,
            (sizeof(styles) / sizeof(styles[NUM_0])));
        return panda::JSValueRef::Undefined(vm);
    }
    if (styleArg->IsInt()) {
        int32_t styles[] = { styleArg->Int32Value(vm) };
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetBorderStyle(nativeNode, styles,
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
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetBorderStyle(nativeNode, styles,
        (sizeof(styles) / sizeof(styles[NUM_0])));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBorderStyle(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBorderStyle(nativeNode);
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
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t shadowStyle = 0;
    if (ParseJsInteger(vm, styleArg, shadowStyle)) {
        double shadows[] = { shadowStyle };
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetBackShadow(nativeNode, shadows,
            (sizeof(shadows) / sizeof(shadows[NUM_0])));
        return panda::JSValueRef::Undefined(vm);
    }

    double shadows[] = { 0.0, 0.0, 0.0, 0.0, static_cast<double>(ShadowType::COLOR), 0.0, 0.0 };
    ParseJsDouble(vm, radiusArg, shadows[NUM_0]);
    shadows[NUM_0] = (LessNotEqual(shadows[NUM_0], 0.0)) ? 0.0 : shadows[NUM_0];
    CalcDimension offsetX;
    if (ParseJsDimensionVp(vm, offsetXArg, offsetX)) {
        shadows[NUM_2] = offsetX.Value();
    }
    CalcDimension offsetY;
    if (ParseJsDimensionVp(vm, offsetYArg, offsetY)) {
        shadows[NUM_3] = offsetY.Value();
    }
    if (typeArg->IsInt()) {
        uint32_t shadowType = typeArg->Uint32Value(vm);
        shadows[NUM_4] = static_cast<double>(
            std::clamp(shadowType, static_cast<uint32_t>(ShadowType::COLOR), static_cast<uint32_t>(ShadowType::BLUR)));
    }
    Color color;
    if (ParseJsColor(vm, colorArg, color)) {
        shadows[NUM_5] = color.GetValue();
    }
    shadows[NUM_6] = static_cast<uint32_t>((fillArg->IsBoolean()) ? fillArg->BooleaValue() : false);
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetBackShadow(nativeNode, shadows,
        (sizeof(shadows) / sizeof(shadows[NUM_0])));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetShadow(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBackShadow(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetHitTestBehavior(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    uint32_t hitTestModeNG = secondArg->Uint32Value(vm);
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetHitTestBehavior(nativeNode, hitTestModeNG);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetHitTestBehavior(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetHitTestBehavior(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetZIndex(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t value = secondArg->Int32Value(vm);
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetZIndex(nativeNode, value);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetZIndex(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetZIndex(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOpacity(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    double opacity = secondArg->ToNumber(vm)->Value();
    if ((LessNotEqual(opacity, 0.0)) || opacity > 1) {
        opacity = 1.0;
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetOpacity(nativeNode, opacity);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOpacity(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetOpacity(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAlign(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsNumber()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetAlign(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetAlign(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAlign(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetAlign(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBackdropBlur(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsNumber()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetBackdropBlur(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBackdropBlur(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackdropBlur(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBackdropBlur(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetHueRotate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    std::optional<float> degree;
    if (secondArg->IsString()) {
        degree = static_cast<float>(StringUtils::StringToDegree(secondArg->ToString(vm)->ToString()));
    } else if (secondArg->IsNumber()) {
        degree = static_cast<float>(secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetHueRotate(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    float deg = 0.0f;
    if (degree) {
        deg = degree.value();
        degree.reset();
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetHueRotate(nativeNode, deg);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetHueRotate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetHueRotate(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetInvert(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsNumber()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetInvert(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetInvert(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetInvert(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetInvert(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetSepia(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsNumber()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetSepia(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetSepia(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetSepia(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetSepia(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetSaturate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsNumber()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetSaturate(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetSaturate(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetSaturate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetSaturate(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetColorBlend(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    uint32_t color = secondArg->Uint32Value(vm);
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetColorBlend(nativeNode, color);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetColorBlend(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetColorBlend(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetGrayscale(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsNumber()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetGrayscale(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetGrayscale(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetGrayscale(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetGrayscale(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetContrast(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsNumber()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetContrast(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetContrast(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetContrast(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetContrast(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBrightness(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsNumber()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetBrightness(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBrightness(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBrightness(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBrightness(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBlur(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsNumber()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetBlur(nativeNode, secondArg->ToNumber(vm)->Value());
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBlur(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBlur(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBlur(nativeNode);
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
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    std::vector<double> values;
    ParseGradientAngle(vm, angleArg, values);
    int32_t direction = static_cast<int32_t>(GradientDirection::NONE);
    ParseJsInt32(vm, directionArg, direction);
    values.push_back(static_cast<double>(direction));
    std::vector<double> colors;
    ParseGradientColorStops(vm, colorsArg, colors);
    auto repeating = repeatingArg->IsBoolean() ? repeatingArg->BooleaValue() : false;
    values.push_back(static_cast<double>(repeating));
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetLinearGradient(nativeNode, values.data(), values.size(),
        colors.data(), colors.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetLinearGradient(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetLinearGradient(nativeNode);
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
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    std::vector<double> values;
    ParseGradientCenter(vm, centerArg, values);
    ParseGradientAngle(vm, startArg, values);
    ParseGradientAngle(vm, endArg, values);
    ParseGradientAngle(vm, rotationArg, values);
    std::vector<double> colors;
    ParseGradientColorStops(vm, colorsArg, colors);
    auto repeating = repeatingArg->IsBoolean() ? repeatingArg->BooleaValue() : false;
    values.push_back(static_cast<double>(repeating));
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetSweepGradient(nativeNode, values.data(), values.size(),
        colors.data(), colors.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetSweepGradient(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetSweepGradient(nativeNode);
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
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    std::vector<double> values;
    ParseGradientCenter(vm, centerArg, values);
    CalcDimension radius;
    auto hasRadius = ParseJsDimensionVp(vm, radiusArg, radius);
    values.push_back(static_cast<double>(hasRadius));
    values.push_back(static_cast<double>(radius.Value()));
    values.push_back(static_cast<double>(radius.Unit()));
    std::vector<double> colors;
    ParseGradientColorStops(vm, colorsArg, colors);
    auto repeating = repeatingArg->IsBoolean() ? repeatingArg->BooleaValue() : false;
    values.push_back(static_cast<double>(repeating));
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetRadialGradient(nativeNode, values.data(), values.size(),
        colors.data(), colors.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetRadialGradient(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetRadialGradient(nativeNode);
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
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t blurStyle = -1;
    if (blurStyleArg->IsNumber()) {
        blurStyle = blurStyleArg->Int32Value(vm);
    }
    bool isHasOptions = !(colorModeArg->IsUndefined() && adaptiveColorArg->IsUndefined() && scaleArg->IsUndefined());
    int32_t colorMode = -1;
    int32_t adaptiveColor = -1;
    double scale = -1.0;
    if (isHasOptions) {
        colorMode = static_cast<int32_t>(ThemeColorMode::SYSTEM);
        ParseJsInt32(vm, colorModeArg, colorMode);
        adaptiveColor = static_cast<int32_t>(AdaptiveColor::DEFAULT);
        ParseJsInt32(vm, adaptiveColorArg, adaptiveColor);
        scale = 1.0;
        if (scaleArg->IsNumber()) {
            scale = scaleArg->ToNumber(vm)->Value();
        }
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetForegroundBlurStyle(nativeNode, blurStyle, colorMode,
        adaptiveColor, scale);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetForegroundBlurStyle(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetForegroundBlurStyle(nativeNode);
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
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    double blurRadius = 0.0;
    ParseJsDouble(vm, blurRadiusArg, blurRadius);
    auto direction = static_cast<int32_t>(GradientDirection::BOTTOM);
    if (directionArg->IsInt()) {
        direction = directionArg->Int32Value(vm);
    }
    std::vector<double> fractionStops;
    if (fractionStopsArg->IsArray(vm)) {
        auto array = panda::Local<panda::ArrayRef>(fractionStopsArg);
        auto length = array->Length(vm);
        for (uint32_t index = 0; index < length; index++) {
            auto fractionStop = panda::ArrayRef::GetValueAt(vm, array, index);
            double first = 0.0;
            double second = 0.0;
            if (!ParseJsDoublePair(vm, fractionStop, first, second)) {
                continue;
            }
            fractionStops.push_back(first);
            fractionStops.push_back(second);
        }
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetLinearGradientBlur(nativeNode, blurRadius, fractionStops.data(),
        fractionStops.size(), direction);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetLinearGradientBlur(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetLinearGradientBlur(nativeNode);
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
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t blurStyle = -1;
    if (blurStyleArg->IsNumber()) {
        blurStyle = blurStyleArg->Int32Value(vm);
    }
    bool isHasOptions = !(colorModeArg->IsUndefined() && adaptiveColorArg->IsUndefined() && scaleArg->IsUndefined());
    int32_t colorMode = -1;
    int32_t adaptiveColor = -1;
    double scale = -1.0;
    if (isHasOptions) {
        colorMode = static_cast<int32_t>(ThemeColorMode::SYSTEM);
        ParseJsInt32(vm, colorModeArg, colorMode);
        adaptiveColor = static_cast<int32_t>(AdaptiveColor::DEFAULT);
        ParseJsInt32(vm, adaptiveColorArg, adaptiveColor);
        scale = 1.0;
        if (scaleArg->IsNumber()) {
            scale = scaleArg->ToNumber(vm)->Value();
        }
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetBackgroundBlurStyle(nativeNode, blurStyle, colorMode,
        adaptiveColor, scale);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundBlurStyle(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBackgroundBlurStyle(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBorder(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();

    uint32_t size = SIZE_OF_EIGHT;
    double values[size];
    int units[size];

    ParseBorderWidth(runtimeCallInfo, vm, values, units, size);
    ParseBorderRadius(runtimeCallInfo, vm, values, units, size);

    uint32_t colorAndStyle[size];
    ParseBorderColor(runtimeCallInfo, vm, colorAndStyle, size);
    ParseBorderStyle(runtimeCallInfo, vm, colorAndStyle, size);

    GetArkUIInternalNodeAPI()->GetCommonModifier().SetBorder(nativeNode, values, units, colorAndStyle, size);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBorder(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBorder(nativeNode);
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
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    double valueX = 0.0;
    double valueY = 0.0;
    DimensionUnit typeX = DimensionUnit::PX;
    DimensionUnit typeY = DimensionUnit::PX;
    bool isAlign = false;

    if (secondArg->IsNumber()) {
        int32_t align = secondArg->ToNumber(vm)->Value();
        ParseBackgroundImagePositionAlign(align, valueX, valueY, typeX, typeY);
        isAlign = true;
    } else if (xArg->IsNumber() || xArg->IsString() || yArg->IsNumber() || yArg->IsString()) {
        CalcDimension x;
        CalcDimension y;

        bool hasX = ParseJsDimensionVp(vm, xArg, x);
        bool hasY = ParseJsDimensionVp(vm, yArg, y);
        if (hasX || hasY) {
            valueX = x.Value();
            valueY = y.Value();
            typeX = DimensionUnit::PX;
            typeY = DimensionUnit::PX;
        }

        if (x.Unit() == DimensionUnit::PERCENT) {
            valueX = x.Value();
            typeX = DimensionUnit::PERCENT;
        }
        if (y.Unit() == DimensionUnit::PERCENT) {
            valueY = y.Value();
            typeY = DimensionUnit::PERCENT;
        }
    }

    double values[SIZE_OF_TWO];
    int32_t types[SIZE_OF_TWO];
    values[NUM_0] = valueX;
    types[NUM_0] = static_cast<int32_t>(typeX);
    values[NUM_1] = valueY;
    types[NUM_1] = static_cast<int32_t>(typeY);

    GetArkUIInternalNodeAPI()->GetCommonModifier().SetBackgroundImagePosition(nativeNode, values, types, isAlign,
        SIZE_OF_TWO);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundImagePosition(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBackgroundImagePosition(nativeNode);
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
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();

    OHOS::Ace::BackgroundImageSizeType typeWidth = OHOS::Ace::BackgroundImageSizeType::AUTO;
    double valueWidth = 0.0;
    OHOS::Ace::BackgroundImageSizeType typeHeight = OHOS::Ace::BackgroundImageSizeType::AUTO;
    double valueHeight = 0.0;

    if (imageSizeArg->IsNumber()) {
        auto sizeType = imageSizeArg->ToNumber(vm)->Value();
        typeWidth = static_cast<OHOS::Ace::BackgroundImageSizeType>(sizeType);
        typeHeight = static_cast<OHOS::Ace::BackgroundImageSizeType>(sizeType);
    } else if (widthArg->IsNumber() || widthArg->IsString() || heightArg->IsNumber() || heightArg->IsString()) {
        CalcDimension width;
        CalcDimension height;

        bool hasWidth = ParseJsDimensionVp(vm, widthArg, width);
        bool hasHeight = ParseJsDimensionVp(vm, heightArg, height);
        if (hasWidth || hasHeight) {
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
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetBackgroundImageSize(nativeNode, valueWidth, valueHeight,
        static_cast<int32_t>(typeWidth), static_cast<int32_t>(typeHeight));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundImageSize(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBackgroundImageSize(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetBackgroundImage(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> srcArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> repeatArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    std::string src;
    int32_t repeatIndex = 0;
    if (!srcArg->IsString()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBackgroundImage(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    src = srcArg->ToString(vm)->ToString();
    if (repeatArg->IsNumber()) {
        repeatIndex = repeatArg->ToNumber(vm)->Value();
    }

    GetArkUIInternalNodeAPI()->GetCommonModifier().SetBackgroundImage(nativeNode, src.c_str(), repeatIndex);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetBackgroundImage(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetBackgroundImage(nativeNode);
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
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (!xArg->IsNumber() && !xArg->IsString() && !yArg->IsNumber() && !yArg->IsString() && !zArg->IsNumber() &&
        !zArg->IsString()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetTranslate(nativeNode);
    } else {
        auto translateX = CalcDimension(0.0);
        auto translateY = CalcDimension(0.0);
        auto translateZ = CalcDimension(0.0);
        bool hasX = ParseAxisDimensionVp(vm, xArg, translateX, true);
        bool hasY = ParseAxisDimensionVp(vm, yArg, translateY, true);
        bool hasZ = ParseAxisDimensionVp(vm, zArg, translateZ, true);
        if (hasX || hasY || hasZ) {
            uint32_t size = SIZE_OF_THREE;
            double values[size];
            int units[size];

            values[NUM_0] = translateX.Value();
            units[NUM_0] = static_cast<int>(translateX.Unit());
            values[NUM_1] = translateY.Value();
            units[NUM_1] = static_cast<int>(translateY.Unit());
            values[NUM_2] = translateZ.Value();
            units[NUM_2] = static_cast<int>(translateZ.Unit());
            GetArkUIInternalNodeAPI()->GetCommonModifier().SetTranslate(nativeNode, values, units, size);
        } else {
            GetArkUIInternalNodeAPI()->GetCommonModifier().ResetTranslate(nativeNode);
        }
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetTranslate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetTranslate(nativeNode);
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
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
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
        if (centerXArg->IsNumber() || centerXArg->IsString()) {
            ParseAxisDimensionVp(vm, centerXArg, centerX, true);
        }
        if (centerYArg->IsNumber() || centerYArg->IsString()) {
            ParseAxisDimensionVp(vm, centerYArg, centerY, true);
        }

        double values[SIZE_OF_FIVE];
        int units[SIZE_OF_TWO];

        values[NUM_0] = centerX.Value();
        units[NUM_0] = static_cast<int>(centerX.Unit());
        values[NUM_1] = centerY.Value();
        units[NUM_1] = static_cast<int>(centerY.Unit());
        values[NUM_2] = scaleX;
        values[NUM_3] = scaleY;
        values[NUM_4] = scaleZ;
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetScale(nativeNode, values, SIZE_OF_FIVE, units, SIZE_OF_TWO);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetScale(nativeNode);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetScale(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetScale(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetRotate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();

    double values[SIZE_OF_EIGHT];
    int units[SIZE_OF_THREE];

    ParseRotate(runtimeCallInfo, values, units);
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetRotate(nativeNode, values, SIZE_OF_EIGHT, units, SIZE_OF_THREE);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetRotate(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetRotate(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetGeometryTransition(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> idArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (!idArg->IsString()) {
        return panda::JSValueRef::Undefined(vm);
    }

    std::string id = idArg->ToString(vm)->ToString();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetGeometryTransition(nativeNode, id.c_str());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetGeometryTransition(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetGeometryTransition(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetClip(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    auto *frameNode = reinterpret_cast<FrameNode *>(nativeNode);
    ViewAbstract::SetClipEdge(frameNode, false);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetClip(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    auto *frameNode = reinterpret_cast<FrameNode *>(nativeNode);

    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    if (info[1]->IsUndefined()) {
        ViewAbstract::SetClipEdge(frameNode, false);
        return panda::JSValueRef::Undefined(vm);
    }
    if (info[1]->IsObject()) {
        Framework::JSShapeAbstract *clipShape =
            Framework::JSRef<Framework::JSObject>::Cast(info[1])->Unwrap<Framework::JSShapeAbstract>();
        if (clipShape == nullptr) {
            LOGD("clipShape is null");
            return panda::JSValueRef::Undefined(vm);
        }
        ViewAbstract::SetClipShape(frameNode, clipShape->GetBasicShape());
    } else if (info[1]->IsBoolean()) {
        ViewAbstract::SetClipEdge(frameNode, info[1]->ToBoolean());
    }
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
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    CalcDimension left;
    ParseJsDimensionVp(vm, leftArg, left);
    CalcDimension right;
    ParseJsDimensionVp(vm, rightArg, right);
    CalcDimension top;
    ParseJsDimensionVp(vm, topArg, top);
    CalcDimension bottom;
    ParseJsDimensionVp(vm, bottomArg, bottom);
    double values[] = { left.Value(), top.Value(), right.Value(), bottom.Value() };
    int units[] = { static_cast<int>(left.Unit()), static_cast<int>(top.Unit()), static_cast<int>(right.Unit()),
                    static_cast<int>(bottom.Unit()) };
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetPixelStretchEffect(nativeNode, values, units,
        (sizeof(values) / sizeof(values[NUM_0])));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetPixelStretchEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetPixelStretchEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetLightUpEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    auto radio = 1.0;
    if (secondArg->IsNumber()) {
        radio = secondArg->ToNumber(vm)->Value();
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetLightUpEffect(nativeNode, radio);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetLightUpEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetLightUpEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetSphericalEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    auto radio = 0.0;
    if (secondArg->IsNumber()) {
        radio = secondArg->ToNumber(vm)->Value();
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetSphericalEffect(nativeNode, radio);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetSphericalEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetSphericalEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetRenderGroup(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    auto isRenderGroup = false;
    if (secondArg->IsBoolean()) {
        isRenderGroup = secondArg->ToBoolean(vm)->Value();
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetRenderGroup(nativeNode, isRenderGroup);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetRenderGroup(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetRenderGroup(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetRenderFit(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto fitModeArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    auto renderFit = static_cast<int32_t>(RenderFit::TOP_LEFT);
    if (fitModeArg->IsNumber()) {
        renderFit = fitModeArg->Int32Value(vm);
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetRenderFit(nativeNode, renderFit);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetRenderFit(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetRenderFit(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetUseEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    auto useEffect = false;
    if (secondArg->IsBoolean()) {
        useEffect = secondArg->ToBoolean(vm)->Value();
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetUseEffect(nativeNode, useEffect);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetUseEffect(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetUseEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetForegroundColor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto colorArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto strategyArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (strategyArg->IsString()) {
        std::string colorStr = strategyArg->ToString(vm)->ToString();
        colorStr.erase(std::remove(colorStr.begin(), colorStr.end(), ' '), colorStr.end());
        std::transform(colorStr.begin(), colorStr.end(), colorStr.begin(), ::tolower);
        if (colorStr.compare("invert") == 0) {
            auto strategy = static_cast<uint32_t>(ForegroundColorStrategy::INVERT);
            GetArkUIInternalNodeAPI()->GetCommonModifier().SetForegroundColor(nativeNode, false, strategy);
            return panda::JSValueRef::Undefined(vm);
        }
    }
    Color foregroundColor;
    if (!ParseJsColor(vm, colorArg, foregroundColor)) {
        return panda::JSValueRef::Undefined(vm);
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetForegroundColor(nativeNode, true, foregroundColor.GetValue());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetForegroundColor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetForegroundColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetMotionPath(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    std::string pathStringValue = runtimeCallInfo->GetCallArgRef(NUM_1)->ToString(vm)->ToString();
    float fromValue = runtimeCallInfo->GetCallArgRef(NUM_2)->ToNumber(vm)->Value();
    float toValue = runtimeCallInfo->GetCallArgRef(NUM_3)->ToNumber(vm)->Value();
    if (fromValue > 1.0f || fromValue < 0.0f) {
        fromValue = 0.0f;
    }
    if (toValue > 1.0f || toValue < 0.0f) {
        toValue = 1.0f;
    } else if (toValue < fromValue) {
        toValue = fromValue;
    }
    bool rotatableValue = runtimeCallInfo->GetCallArgRef(NUM_4)->ToBoolean(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetMotionPath(nativeNode, pathStringValue.c_str(), fromValue,
        toValue, rotatableValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetMotionPath(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMotionPath(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetGroupDefaultFocus(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    bool groupDefaultFocus = secondArg->ToBoolean(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetGroupDefaultFocus(nativeNode, groupDefaultFocus);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetGroupDefaultFocus(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetGroupDefaultFocus(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetFocusOnTouch(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    bool focusOnTouch = secondArg->ToBoolean(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetFocusOnTouch(nativeNode, focusOnTouch);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetFocusOnTouch(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetFocusOnTouch(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetFocusable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    bool focusable = secondArg->ToBoolean(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetFocusable(nativeNode, focusable);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetFocusable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetFocusable(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetTouchable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    bool touchable = secondArg->ToBoolean(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetTouchable(nativeNode, touchable);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetTouchable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetTouchable(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetDefaultFocus(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    bool defaultFocus = secondArg->ToBoolean(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetDefaultFocus(nativeNode, defaultFocus);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetDefaultFocus(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetDefaultFocus(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetDisplayPriority(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    double value = secondArg->ToNumber(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetDisplayPriority(nativeNode, value);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetDisplayPriority(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetDisplayPriority(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAccessibilityLevel(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    std::string stringValue = secondArg->ToString(vm)->ToString();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetAccessibilityLevel(nativeNode, stringValue.c_str());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAccessibilityLevel(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetAccessibilityLevel(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAccessibilityDescription(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    std::string stringValue = secondArg->ToString(vm)->ToString();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetAccessibilityDescription(nativeNode, stringValue.c_str());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAccessibilityDescription(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetAccessibilityDescription(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetOffset(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    CalcDimension xVal(0, DimensionUnit::VP);
    CalcDimension yVal(0, DimensionUnit::VP);
    ParseJsDimensionVp(vm, secondArg, xVal);
    ParseJsDimensionVp(vm, thirdArg, yVal);

    double number[2] = {xVal.Value(), yVal.Value()};
    int8_t unit[2] = {static_cast<int8_t>(xVal.Unit()), static_cast<int8_t>(yVal.Unit())};
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetOffset(nativeNode, number, unit);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetOffset(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();

    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetOffset(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetPadding(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> forthArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> fifthArg = runtimeCallInfo->GetCallArgRef(NUM_4);

    struct StringAndDouble top = { 0.0, nullptr };
    struct StringAndDouble right = { 0.0, nullptr };
    struct StringAndDouble bottom = { 0.0, nullptr };
    struct StringAndDouble left = { 0.0, nullptr };

    if (!secondArg->IsNumber() && !secondArg->IsString() && !thirdArg->IsNumber() && !thirdArg->IsString() &&
        !forthArg->IsNumber() && !forthArg->IsString() && !fifthArg->IsNumber() && !fifthArg->IsString()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetPadding(nativeNode);
    } else {
        if (secondArg->IsNumber()) {
            top.value = secondArg->ToNumber(vm)->Value();
        } else if (secondArg->IsString()) {
            top.valueStr = secondArg->ToString(vm)->ToString().c_str();
        }

        if (thirdArg->IsNumber()) {
            right.value = thirdArg->ToNumber(vm)->Value();
        } else if (thirdArg->IsString()) {
            right.valueStr = thirdArg->ToString(vm)->ToString().c_str();
        }

        if (forthArg->IsNumber()) {
            bottom.value = forthArg->ToNumber(vm)->Value();
        } else if (forthArg->IsString()) {
            bottom.valueStr = forthArg->ToString(vm)->ToString().c_str();
        }

        if (fifthArg->IsNumber()) {
            left.value = fifthArg->ToNumber(vm)->Value();
        } else if (fifthArg->IsString()) {
            left.valueStr = fifthArg->ToString(vm)->ToString().c_str();
        }

        GetArkUIInternalNodeAPI()->GetCommonModifier().SetPadding(nativeNode, &top, &right, &bottom, &left);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetPadding(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetPadding(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetMargin(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> forthArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> fifthArg = runtimeCallInfo->GetCallArgRef(NUM_4);

    struct StringAndDouble top = { 0.0, nullptr };
    struct StringAndDouble right = { 0.0, nullptr };
    struct StringAndDouble bottom = { 0.0, nullptr };
    struct StringAndDouble left = { 0.0, nullptr };

    if (!secondArg->IsNumber() && !secondArg->IsString() && !thirdArg->IsNumber() && !thirdArg->IsString() &&
        !forthArg->IsNumber() && !forthArg->IsString() && !fifthArg->IsNumber() && !fifthArg->IsString()) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMargin(nativeNode);
    } else {
        if (secondArg->IsNumber()) {
            top.value = secondArg->ToNumber(vm)->Value();
        } else if (secondArg->IsString()) {
            top.valueStr = secondArg->ToString(vm)->ToString().c_str();
        }

        if (thirdArg->IsNumber()) {
            right.value = thirdArg->ToNumber(vm)->Value();
        } else if (thirdArg->IsString()) {
            right.valueStr = thirdArg->ToString(vm)->ToString().c_str();
        }

        if (forthArg->IsNumber()) {
            bottom.value = forthArg->ToNumber(vm)->Value();
        } else if (forthArg->IsString()) {
            bottom.valueStr = forthArg->ToString(vm)->ToString().c_str();
        }

        if (fifthArg->IsNumber()) {
            left.value = fifthArg->ToNumber(vm)->Value();
        } else if (fifthArg->IsString()) {
            left.valueStr = fifthArg->ToString(vm)->ToString().c_str();
        }

        GetArkUIInternalNodeAPI()->GetCommonModifier().SetMargin(nativeNode, &top, &right, &bottom, &left);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetMargin(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMargin(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetMarkAnchor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> xArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> yArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    void *nativeNode = nativeNodeArg->ToNativePointer(vm)->Value();
    CalcDimension x(0.0, DimensionUnit::VP);
    CalcDimension y(0.0, DimensionUnit::VP);
    bool hasX = ParseJsDimensionNG(vm, xArg, x, DimensionUnit::VP);
    bool hasY = ParseJsDimensionNG(vm, yArg, y, DimensionUnit::VP);
    if (hasX || hasY) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetMarkAnchor(nativeNode, x.Value(),
            static_cast<int32_t>(x.Unit()), y.Value(), static_cast<int32_t>(y.Unit()));
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMarkAnchor(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetMarkAnchor(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = nativeNodeArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMarkAnchor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetVisibility(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t value = secondArg->Int32Value(vm);
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetVisibility(nativeNode, value);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetVisibility(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetVisibility(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAccessibilityText(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    std::string stringValue = secondArg->ToString(vm)->ToString();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetAccessibilityText(nativeNode, stringValue.c_str());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAccessibilityText(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetAccessibilityText(nativeNode);
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
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    struct StringAndDouble minWidth {
        0.0, nullptr
    };
    struct StringAndDouble maxWidth {
        0.0, nullptr
    };
    struct StringAndDouble minHeight {
        0.0, nullptr
    };
    struct StringAndDouble maxHeight {
        0.0, nullptr
    };
    std::string secondStr = "";
    std::string thirdStr = "";
    std::string forthStr = "";
    std::string fifthStr = "";
    if (secondArg->IsNumber()) {
        minWidth.value = secondArg->ToNumber(vm)->Value();
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetMinWidth(nativeNode, &minWidth);
    } else if (secondArg->IsString()) {
        secondStr = secondArg->ToString(vm)->ToString();
        minWidth.valueStr = secondStr.c_str();
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetMinWidth(nativeNode, &minWidth);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMinWidth(nativeNode);
    }

    if (thirdArg->IsNumber()) {
        maxWidth.value = thirdArg->ToNumber(vm)->Value();
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetMaxWidth(nativeNode, &maxWidth);
    } else if (thirdArg->IsString()) {
        thirdStr = thirdArg->ToString(vm)->ToString();
        maxWidth.valueStr = thirdStr.c_str();
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetMaxWidth(nativeNode, &maxWidth);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMaxWidth(nativeNode);
    }

    if (forthArg->IsNumber()) {
        minHeight.value = forthArg->ToNumber(vm)->Value();
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetMinHeight(nativeNode, &minHeight);
    } else if (forthArg->IsString()) {
        forthStr = forthArg->ToString(vm)->ToString();
        minHeight.valueStr = forthStr.c_str();
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetMinHeight(nativeNode, &minHeight);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMinHeight(nativeNode);
    }

    if (fifthArg->IsNumber()) {
        maxHeight.value = fifthArg->ToNumber(vm)->Value();
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetMaxHeight(nativeNode, &maxHeight);
    } else if (fifthArg->IsString()) {
        fifthStr = fifthArg->ToString(vm)->ToString();
        maxHeight.valueStr = fifthStr.c_str();
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetMaxHeight(nativeNode, &maxHeight);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMaxHeight(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetConstraintSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMaxHeight(nativeNode);
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMaxWidth(nativeNode);
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMinHeight(nativeNode);
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMinWidth(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetDirection(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    std::string dir;
    if (secondArg->IsString()) {
        dir = secondArg->ToString(vm)->ToString();
        int32_t direction = NUM_3;
        if (dir == "Ltr") {
            direction = NUM_0;
        } else if (dir == "Rtl") {
            direction = NUM_1;
        } else if (dir == "Auto") {
            direction = NUM_3;
        } else if (dir == "undefined" && Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
            direction = NUM_3;
        }
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetDirection(nativeNode, direction);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetDirection(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetDirection(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetDirection(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetLayoutWeight(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t layoutWeight = 0;
    if (secondArg->IsNumber()) {
        layoutWeight = secondArg->Int32Value(vm);
    } else {
        layoutWeight = StringUtils::StringToInt(secondArg->ToString(vm)->ToString());
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetLayoutWeight(nativeNode, layoutWeight);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetLayoutWeight(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetLayoutWeight(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    CalcDimension wVal(0.0, DimensionUnit::VP);
    CalcDimension hVal(0.0, DimensionUnit::VP);
    if (ParseCalcDimension(vm, nativeNode, secondArg, wVal, true) &&
        ParseCalcDimension(vm, nativeNode, thirdArg, hVal, false)) {
        const char* calc[SIZE_ARRAY_NUM] = {wVal.CalcValue().c_str(), hVal.CalcValue().c_str()};
        double number[SIZE_ARRAY_NUM] = {wVal.Value(), hVal.Value()};
        int8_t unit[SIZE_ARRAY_NUM] = {static_cast<int8_t>(wVal.Unit()), static_cast<int8_t>(hVal.Unit())};
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetSize(nativeNode, number, unit, calc);
    } else {
        const char* calc[SIZE_ARRAY_NUM] = {"", ""};
        double number[SIZE_ARRAY_NUM] = {0.0, 0.0};
        int8_t unit[SIZE_ARRAY_NUM] = {static_cast<int8_t>(DimensionUnit::VP), static_cast<int8_t>(DimensionUnit::VP)};
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetSize(nativeNode, number, unit, calc);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    const char* calc[SIZE_ARRAY_NUM] = {"", ""};
    double number[SIZE_ARRAY_NUM] = {0.0, 0.0};
    int8_t unit[SIZE_ARRAY_NUM] = {static_cast<int8_t>(DimensionUnit::VP), static_cast<int8_t>(DimensionUnit::VP)};
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetSize(nativeNode, number, unit, calc);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAlignSelf(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    if (secondArg->IsNumber()) {
        uint32_t value = secondArg->Int32Value(vm);
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetAlignSelf(nativeNode, value);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetAlignSelf(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAlignSelf(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetAlignSelf(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAspectRatio(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    if (secondArg->IsNumber()) {
        double value = secondArg->ToNumber(vm)->Value();
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetAspectRatio(nativeNode, value);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetAspectRatio(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAspectRatio(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetAspectRatio(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetFlexGrow(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    if (secondArg->IsNumber()) {
        double value = secondArg->ToNumber(vm)->Value();
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetFlexGrow(nativeNode, value);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetFlexGrow(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetFlexGrow(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetFlexGrow(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetFlexShrink(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    if (secondArg->IsNumber()) {
        double value = secondArg->ToNumber(vm)->Value();
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetFlexShrink(nativeNode, value);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetFlexShrink(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetFlexShrink(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetFlexShrink(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetGridOffset(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> offsetArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = nativeNodeArg->ToNativePointer(vm)->Value();
    int32_t offset = 0;
    if (offsetArg->IsNumber()) {
        offset = offsetArg->Int32Value(vm);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetGridOffset(nativeNode);
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetGridOffset(nativeNode, offset);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetGridOffset(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = nativeNodeArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetGridOffset(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetGridSpan(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t value = 0;
    if (secondArg->IsNumber()) {
        value = secondArg->Int32Value(vm);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetGridSpan(nativeNode);
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetGridSpan(nativeNode, value);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetGridSpan(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetGridSpan(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetExpandSafeArea(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    std::string typeCppStr = "";
    std::string edgesCppStr = "";
    if (secondArg->IsString()) {
        typeCppStr = secondArg->ToString(vm)->ToString();
    } else {
        typeCppStr = "1|2|4";
    }

    if (secondArg->IsString()) {
        edgesCppStr = thirdArg->ToString(vm)->ToString();
    } else {
        edgesCppStr = "1|2|4|8";
    }
    const char* typeStr = typeCppStr.c_str();
    const char* edgesStr = edgesCppStr.c_str();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetExpandSafeArea(nativeNode, typeStr, edgesStr);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetExpandSafeArea(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetExpandSafeArea(nativeNode);
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
    void *nativeNode = firstArg->ToNativePointer(vm)->Value();
    
    auto anchors = std::make_unique<std::string []>(ALIGN_RULES_NUM);
    auto direction = std::make_unique<int8_t []>(ALIGN_RULES_NUM);
    for (int i = 0; i < ALIGN_RULES_NUM; i++) {
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
        return panda::JSValueRef::Undefined(vm);
    }
    auto realAnchors = std::make_unique<char* []>(ALIGN_RULES_NUM);
    for (int i = 0; i < ALIGN_RULES_NUM; i++) {
        realAnchors[i] = const_cast<char*>(anchors[i].c_str());
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetAlignRules(nativeNode, realAnchors.get(), direction.get(),
        ALIGN_RULES_NUM);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAlignRules(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetAlignRules(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetFlexBasis(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    struct StringAndDouble flexBasis { 0.0, nullptr};
    std::string tempValueStr = "";
    if (secondArg->IsNumber()) {
        flexBasis.value = secondArg->ToNumber(vm)->Value();
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetFlexBasis(nativeNode, &flexBasis);
    } else if (secondArg->IsString()) {
        tempValueStr = secondArg->ToString(vm)->ToString();
        flexBasis.valueStr = tempValueStr.c_str();
        GetArkUIInternalNodeAPI()->GetCommonModifier().SetFlexBasis(nativeNode, &flexBasis);
    } else {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetFlexBasis(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetFlexBasis(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetFlexBasis(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetAllowDrop(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    Local<panda::ArrayRef> allowDropArray = static_cast<Local<panda::ArrayRef>>(secondArg);
    std::vector<std::string> keepStr;
    std::vector<const char*> strList;
    for (size_t i = 0; i < allowDropArray->Length(vm); i++) {
        Local<JSValueRef> objValue = allowDropArray->GetValueAt(vm, secondArg, i);
        keepStr.push_back(objValue->ToString(vm)->ToString());
        strList.push_back(keepStr[i].c_str());
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetAllowDrop(nativeNode, strList.data(), strList.size());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetAllowDrop(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetAllowDrop(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetId(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    std::string stringValue = secondArg->ToString(vm)->ToString();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetId(nativeNode, stringValue.c_str());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetId(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetId(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetKey(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    std::string stringValue = secondArg->ToString(vm)->ToString();
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetKey(nativeNode, stringValue.c_str());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetKey(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetKey(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetRestoreId(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    uint32_t value = secondArg->Uint32Value(vm);
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetRestoreId(nativeNode, value);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetRestoreId(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetRestoreId(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetTabIndex(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t index = secondArg->Int32Value(vm);
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetTabIndex(nativeNode, index);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetTabIndex(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetTabIndex(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetObscured(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    if (secondArg->IsUndefined() || !secondArg->IsArray(vm)) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetObscured(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    Local<panda::ArrayRef> transArray = static_cast<Local<panda::ArrayRef>>(secondArg);
    int32_t length = transArray->Length(vm);
    int32_t reasonArray[length];

    for (int32_t i = 0; i < length; i++) {
        Local<JSValueRef> value = transArray->GetValueAt(vm, secondArg, i);
        reasonArray[i] = value->Int32Value(vm);
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetObscured(nativeNode, reasonArray, length);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetObscured(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetObscured(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetResponseRegion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    if (secondArg->IsUndefined() || !secondArg->IsArray(vm)) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetResponseRegion(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    Local<panda::ArrayRef> transArray = static_cast<Local<panda::ArrayRef>>(secondArg);
    int32_t length = transArray->Length(vm);
    double regionArray[length];

    for (int32_t i = 0; i < length; i++) {
        Local<JSValueRef> value = transArray->GetValueAt(vm, secondArg, i);
        CalcDimension result =
            StringUtils::StringToCalcDimension(value->ToString(vm)->ToString(), false, DimensionUnit::VP);
        regionArray[i] = result.Value();
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetResponseRegion(nativeNode, regionArray, length);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetResponseRegion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetResponseRegion(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::SetMouseResponseRegion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    if (secondArg->IsUndefined() || !secondArg->IsArray(vm)) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMouseResponseRegion(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    Local<panda::ArrayRef> transArray = static_cast<Local<panda::ArrayRef>>(secondArg);
    int32_t length = transArray->Length(vm);
    double regionArray[length];

    for (int32_t i = 0; i < length; i++) {
        Local<JSValueRef> value = transArray->GetValueAt(vm, secondArg, i);
        CalcDimension result =
            StringUtils::StringToCalcDimension(value->ToString(vm)->ToString(), false, DimensionUnit::VP);
        regionArray[i] = result.Value();
    }
    GetArkUIInternalNodeAPI()->GetCommonModifier().SetMouseResponseRegion(nativeNode, regionArray, length);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue CommonBridge::ResetMouseResponseRegion(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetCommonModifier().ResetMouseResponseRegion(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
} // namespace OHOS::Ace::NG
