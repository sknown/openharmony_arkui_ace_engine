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
#include "bridge/declarative_frontend/engine/jsi/components/arkts_native_common_modifier.h"

#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t DEFAULT_BUTTON_COLOR = 0xFF007DFF;
constexpr double PERCENT_100 = 100.0;
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
constexpr int DEFAULT_LENGTH = 4;
constexpr double ROUND_UNIT = 360.0;


BorderStyle ConvertBorderStyle(int32_t value)
{
    auto style = static_cast<BorderStyle>(value);
    if (style < BorderStyle::SOLID || style > BorderStyle::NONE) {
        style = BorderStyle::SOLID;
    }
    return style;
}
Alignment ParseAlignment(int32_t align)
{
    Alignment alignment = Alignment::CENTER;
    switch (align) {
        case NUM_0:
            alignment = Alignment::TOP_LEFT;
            break;
        case NUM_1:
            alignment = Alignment::TOP_CENTER;
            break;
        case NUM_2:
            alignment = Alignment::TOP_RIGHT;
            break;
        case NUM_3:
            alignment = Alignment::CENTER_LEFT;
            break;
        case NUM_4:
            alignment = Alignment::CENTER;
            break;
        case NUM_5:
            alignment = Alignment::CENTER_RIGHT;
            break;
        case NUM_6:
            alignment = Alignment::BOTTOM_LEFT;
            break;
        case NUM_7:
            alignment = Alignment::BOTTOM_CENTER;
            break;
        case NUM_8:
            alignment = Alignment::BOTTOM_RIGHT;
            break;
        default:
            break;
    }
    return alignment;
}

/**
 * @param colors color value
 * colors[0], colors[1], colors[2] : color[0](color, hasDimension, dimension)
 * colors[3], colors[4], colors[5] : color[1](color, hasDimension, dimension)
 * ...
 * @param colorsLength colors length
 */
void SetGradientColors(NG::Gradient& gradient, const double* colors, int32_t colorsLength)
{
    if ((colors == nullptr) || (colorsLength % NUM_3) != 0) {
        return;
    }
    for (int32_t index = 0; index < colorsLength; index += NUM_3) {
        auto colorValue = colors[index];
        auto colorHasDimension = colors[index + NUM_1];
        auto colorDimension = colors[index + NUM_2];
        auto color = static_cast<uint32_t>(colorValue);
        auto hasDimension = static_cast<bool>(colorHasDimension);
        auto dimension = colorDimension;
        NG::GradientColor gradientColor;
        gradientColor.SetColor(Color(color));
        gradientColor.SetHasValue(hasDimension);
        if (hasDimension) {
            gradientColor.SetDimension(CalcDimension(dimension * PERCENT_100, DimensionUnit::PERCENT));
        }
        gradient.AddColor(gradientColor);
    }
}

void SetLinearGradientDirectionTo(std::shared_ptr<LinearGradient>& linearGradient, const GradientDirection direction)
{
    switch (direction) {
        case GradientDirection::LEFT:
            linearGradient->linearX = NG::GradientDirection::LEFT;
            break;
        case GradientDirection::RIGHT:
            linearGradient->linearX = NG::GradientDirection::RIGHT;
            break;
        case GradientDirection::TOP:
            linearGradient->linearY = NG::GradientDirection::TOP;
            break;
        case GradientDirection::BOTTOM:
            linearGradient->linearY = NG::GradientDirection::BOTTOM;
            break;
        case GradientDirection::LEFT_TOP:
            linearGradient->linearX = NG::GradientDirection::LEFT;
            linearGradient->linearY = NG::GradientDirection::TOP;
            break;
        case GradientDirection::LEFT_BOTTOM:
            linearGradient->linearX = NG::GradientDirection::LEFT;
            linearGradient->linearY = NG::GradientDirection::BOTTOM;
            break;
        case GradientDirection::RIGHT_TOP:
            linearGradient->linearX = NG::GradientDirection::RIGHT;
            linearGradient->linearY = NG::GradientDirection::TOP;
            break;
        case GradientDirection::RIGHT_BOTTOM:
            linearGradient->linearX = NG::GradientDirection::RIGHT;
            linearGradient->linearY = NG::GradientDirection::BOTTOM;
            break;
        case GradientDirection::NONE:
        case GradientDirection::START_TO_END:
        case GradientDirection::END_TO_START:
        default:
            break;
    }
}

/**
 * @param values value value
 * values[0], values[1] : angle: hasValue, angle value
 * values[2] : direction
 * values[3] : repeating
 * @param valuesLength values length
 */
void SetLinearGradientValues(NG::Gradient& gradient, const double* values, int32_t valuesLength)
{
    if ((values == nullptr) || (valuesLength != NUM_4)) {
        return;
    }
    auto angleHasValue = values[NUM_0];
    auto angleValue = values[NUM_1];
    auto directionValue = values[NUM_2];
    auto repeating = values[NUM_3];
    auto linearGradient = gradient.GetLinearGradient();
    if (linearGradient == nullptr) {
        return;
    }
    if (static_cast<bool>(angleHasValue)) {
        linearGradient->angle = CalcDimension(angleValue, DimensionUnit::PX);
    }
    SetLinearGradientDirectionTo(linearGradient, static_cast<GradientDirection>(directionValue));
    gradient.SetRepeat(static_cast<bool>(repeating));
}
} // namespace

void SetBackgroundColor(NodeHandle node, uint32_t color)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetBackgroundColor(frameNode, Color(color));
}

void ResetBackgroundColor(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetBackgroundColor(frameNode, Color(DEFAULT_BUTTON_COLOR));
}

void SetWidth(NodeHandle node, double value, int unit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetWidth(frameNode, CalcLength(value, static_cast<OHOS::Ace::DimensionUnit>(unit)));
}

void ResetWidth(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::ClearWidthOrHeight(frameNode, true);
}
void SetHeight(NodeHandle node, double value, int unit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetHeight(frameNode, CalcLength(value, static_cast<OHOS::Ace::DimensionUnit>(unit)));
}
void ResetHeight(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::ClearWidthOrHeight(frameNode, true);
}
/**
 * @param values radius values
 * value[0] : radius value for TopLeft，value[1] : radius value for TopRight
 * value[2] : radius value for BottomLeft，value[3] : radius value for BottomRight
 * @param units adius units
 * units[0]: radius unit for TopLeft ,units[1] : radius unit for TopRight
 * units[2]: radius unit for BottomLeft, units[3] : radius unit for TopRight
 */
void SetBorderRadius(NodeHandle node, const double* values, const int* units, int32_t length)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if (length != DEFAULT_LENGTH) {
        return;
    }
    NG::BorderRadiusProperty borderRadius;
    borderRadius.radiusTopLeft = Dimension(values[NUM_0], static_cast<OHOS::Ace::DimensionUnit>(units[NUM_0]));
    borderRadius.radiusTopRight = Dimension(values[NUM_1], static_cast<OHOS::Ace::DimensionUnit>(units[NUM_1]));
    borderRadius.radiusBottomLeft = Dimension(values[NUM_2], static_cast<OHOS::Ace::DimensionUnit>(units[NUM_2]));
    borderRadius.radiusBottomRight = Dimension(values[NUM_3], static_cast<OHOS::Ace::DimensionUnit>(units[NUM_3]));
    borderRadius.multiValued = true;
    ViewAbstract::SetBorderRadius(frameNode, borderRadius);
}

void ResetBorderRadius(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    OHOS::Ace::CalcDimension reset;
    ViewAbstract::SetBorderRadius(frameNode, reset);
}

/**
 * @param values radius values
 * value[0] : BorderWidth value for left，value[1] : BorderWidth value for right
 * value[2] : BorderWidth value for top，value[3] : BorderWidth value for bottom
 * @param units adius units
 * units[0]: BorderWidth unit for left ,units[1] : BorderWidth unit for right
 * units[2]: BorderWidth unit for top, units[3] : BorderWidth unit for bottom
 */
void SetBorderWidth(NodeHandle node, const double* values, const int* units, int32_t length)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if (length != DEFAULT_LENGTH) {
        return;
    }
    NG::BorderWidthProperty borderWidth;
    borderWidth.leftDimen = Dimension(values[NUM_0], static_cast<OHOS::Ace::DimensionUnit>(units[NUM_0]));
    borderWidth.rightDimen = Dimension(values[NUM_1], static_cast<OHOS::Ace::DimensionUnit>(units[NUM_1]));
    borderWidth.topDimen = Dimension(values[NUM_2], static_cast<OHOS::Ace::DimensionUnit>(units[NUM_2]));
    borderWidth.bottomDimen = Dimension(values[NUM_3], static_cast<OHOS::Ace::DimensionUnit>(units[NUM_3]));
    borderWidth.multiValued = true;
    ViewAbstract::SetBorderWidth(frameNode, borderWidth);
}

void ResetBorderWidth(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    OHOS::Ace::Dimension borderWidth;
    ViewAbstract::SetBorderWidth(frameNode, borderWidth);
}

void SetTransform(NodeHandle node, const float* matrix, int32_t length)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    const auto matrix4Len = Matrix4::DIMENSION * Matrix4::DIMENSION;
    if (length != matrix4Len) {
        return;
    }
    NG::ViewAbstract::SetTransformMatrix(frameNode,
        Matrix4(matrix[NUM_0], matrix[NUM_4], matrix[NUM_8], matrix[NUM_12], matrix[NUM_1], matrix[NUM_5],
            matrix[NUM_9], matrix[NUM_13], matrix[NUM_2], matrix[NUM_6], matrix[NUM_10], matrix[NUM_14],
            matrix[NUM_3], matrix[NUM_7], matrix[NUM_11], matrix[NUM_15]));
}

void ResetTransform(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    const auto matrix4Len = Matrix4::DIMENSION * Matrix4::DIMENSION;
    std::vector<float> matrix(matrix4Len);
    const int32_t initPosition = 5;
    for (int32_t i = 0; i < matrix4Len; i = i + initPosition) {
        double value = 1.0;
        matrix[i] = static_cast<float>(value);
    }
    NG::ViewAbstract::SetTransformMatrix(frameNode,
        Matrix4(matrix[NUM_0], matrix[NUM_4], matrix[NUM_8], matrix[NUM_12], matrix[NUM_1], matrix[NUM_5],
            matrix[NUM_9], matrix[NUM_13], matrix[NUM_2], matrix[NUM_6], matrix[NUM_10], matrix[NUM_14],
            matrix[NUM_3], matrix[NUM_7], matrix[NUM_11], matrix[NUM_15]));
}
void SetBorderColor(NodeHandle node, const uint32_t& leftColorInt, const uint32_t& rightColorInt,
    const uint32_t& topColorInt, const uint32_t& bottomColorInt)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NG::BorderColorProperty borderColors;
    borderColors.leftColor = Color(leftColorInt);
    borderColors.rightColor = Color(rightColorInt);
    borderColors.topColor = Color(topColorInt);
    borderColors.bottomColor = Color(bottomColorInt);
    borderColors.multiValued = true;

    ViewAbstract::SetBorderColor(frameNode, borderColors);
}

void ResetBorderColor(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetBorderColor(frameNode, Color::BLACK);
}

/**
 * @param xValue position x value
 * @param xUnit position x unit
 * @param yValue position y unit
 * @param yUnit position y unit
 */
void SetPosition(NodeHandle node, double xValue, int xUnit, double yValue, int yUnit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);

    ViewAbstract::SetPosition(frameNode, { Dimension(xValue, static_cast<OHOS::Ace::DimensionUnit>(xUnit)),
                                             Dimension(yValue, static_cast<OHOS::Ace::DimensionUnit>(yUnit)) });
}

void ResetPosition(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetPosition(frameNode, { 0.0_vp, 0.0_vp });
}

/**
 * @param styles styles value
 * styles[0] : styleLeft, styles[1] : styleRight, styles[2] : styleTop, styles[3] : styleBottom
 * @param length styles length
 */
void SetBorderStyle(NodeHandle node, const int32_t* styles, int32_t length)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if (length == NUM_1) {
        ViewAbstract::SetBorderStyle(frameNode, ConvertBorderStyle(styles[NUM_0]));
        return;
    }
    if (length == NUM_4) {
        NG::BorderStyleProperty borderStyles;
        borderStyles.styleLeft = ConvertBorderStyle(styles[NUM_0]);
        borderStyles.styleRight = ConvertBorderStyle(styles[NUM_1]);
        borderStyles.styleTop = ConvertBorderStyle(styles[NUM_2]);
        borderStyles.styleBottom = ConvertBorderStyle(styles[NUM_3]);
        borderStyles.multiValued = true;
        ViewAbstract::SetBorderStyle(frameNode, borderStyles);
    }
}

void ResetBorderStyle(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetBorderStyle(frameNode, BorderStyle::SOLID);
}

/**
 * @param shadows shadow value
 * shadows[0] : BlurRadius, shadows[1] : SpreadRadius
 * shadows[2] : OffsetX, offset[3] : OffsetY
 * shadows[4] : ShadowType, shadows[5] : Color, shadows[6] : IsFilled
 * @param length shadows length
 */
void SetBackShadow(NodeHandle node, const double* shadows, int32_t length)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if (length != NUM_7) {
        return;
    }
    auto blurRadius = shadows[NUM_0]; // BlurRadius
    auto spreadRadius = shadows[NUM_1]; // SpreadRadius
    auto offsetX = shadows[NUM_2]; // OffsetX
    auto offsetY = shadows[NUM_3]; // OffsetY
    auto shadowType = static_cast<uint32_t>(shadows[NUM_4]); // ShadowType
    auto color = static_cast<uint32_t>(shadows[NUM_5]); // Color
    auto isFilled = static_cast<uint32_t>(shadows[NUM_6]); // IsFilled
    Shadow shadow(blurRadius, spreadRadius, Offset(offsetX, offsetY), Color(color));
    shadow.SetShadowType(static_cast<ShadowType>(shadowType));
    shadow.SetIsFilled(static_cast<bool>(isFilled));
    ViewAbstract::SetBackShadow(frameNode, shadow);
}

void ResetBackShadow(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    Shadow shadow;
    ViewAbstract::SetBackShadow(frameNode, shadow);
}

void SetHitTestBehavior(NodeHandle node, uint32_t value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NG::HitTestMode hitTestModeNG = static_cast<NG::HitTestMode>(value);
    ViewAbstract::SetHitTestMode(frameNode, hitTestModeNG);
}

void ResetHitTestBehavior(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetHitTestMode(frameNode, NG::HitTestMode::HTMDEFAULT);
}

void SetZIndex(NodeHandle node, int32_t value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetZIndex(frameNode, value);
}

void ResetZIndex(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetZIndex(frameNode, 0);
}

void SetOpacity(NodeHandle node, double opacity)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetOpacity(frameNode, opacity);
}

void ResetOpacity(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetOpacity(frameNode, 1.0f);
}
void SetAlign(NodeHandle node, int32_t align)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    Alignment alignment = ParseAlignment(align);
    ViewAbstract::SetAlign(frameNode, alignment);
}

void ResetAlign(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetAlign(frameNode, Alignment::CENTER);
}

void SetBackdropBlur(NodeHandle node, double value)
{
    float blur = 0.0f;
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if (value > 0) {
        blur = value;
    }
    CalcDimension dimensionRadius(blur, DimensionUnit::PX);
    ViewAbstract::SetBackdropBlur(frameNode, dimensionRadius);
}

void ResetBackdropBlur(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    double blur = 0.0;
    CalcDimension dimensionRadius(blur, DimensionUnit::PX);
    ViewAbstract::SetBackdropBlur(frameNode, dimensionRadius);
}

void SetHueRotate(NodeHandle node, float deg)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    deg = std::fmod(deg, ROUND_UNIT);
    if (deg < 0.0f) {
        deg += ROUND_UNIT;
    }
    ViewAbstract::SetHueRotate(frameNode, deg);
}

void ResetHueRotate(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    float deg = 0.0f;
    ViewAbstract::SetHueRotate(frameNode, deg);
}

void SetInvert(NodeHandle node, double invert)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    InvertVariant invertVariant = static_cast<float>(invert);
    ViewAbstract::SetInvert(frameNode, invertVariant);
}

void ResetInvert(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    InvertVariant invert = 0.0f;
    ViewAbstract::SetInvert(frameNode, invert);
}

void SetSepia(NodeHandle node, double sepia)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension value = CalcDimension(sepia, DimensionUnit::VP);
    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }
    if (GreatNotEqual(value.Value(), 1.0)) {
        value.SetValue(1.0);
    }
    ViewAbstract::SetSepia(frameNode, value);
}

void ResetSepia(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension value(0.0, DimensionUnit::VP);
    ViewAbstract::SetSepia(frameNode, value);
}

void SetSaturate(NodeHandle node, double saturate)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension value = CalcDimension(saturate, DimensionUnit::VP);
    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }
    ViewAbstract::SetSaturate(frameNode, value);
}

void ResetSaturate(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension value(1.0, DimensionUnit::VP);
    ViewAbstract::SetSaturate(frameNode, value);
}

void SetColorBlend(NodeHandle node, uint32_t color)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ViewAbstract::SetColorBlend(frameNode, Color(color));
}

void ResetColorBlend(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    Color colorBlend = Color::TRANSPARENT;
    ViewAbstract::SetColorBlend(frameNode, colorBlend);
}

void SetGrayscale(NodeHandle node, double grayScale)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension value = CalcDimension(grayScale, DimensionUnit::VP);
    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }
    if (GreatNotEqual(value.Value(), 1.0)) {
        value.SetValue(1.0);
    }
    ViewAbstract::SetGrayScale(frameNode, value);
}

void ResetGrayscale(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension value(0.0, DimensionUnit::VP);
    ViewAbstract::SetGrayScale(frameNode, value);
}

void SetContrast(NodeHandle node, double contrast)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension value = CalcDimension(contrast, DimensionUnit::VP);
    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }
    ViewAbstract::SetContrast(frameNode, value);
}

void ResetContrast(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension value(1.0, DimensionUnit::VP);
    ViewAbstract::SetContrast(frameNode, value);
}

void SetBrightness(NodeHandle node, double brightness)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension value = CalcDimension(brightness, DimensionUnit::VP);
    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }
    ViewAbstract::SetBrightness(frameNode, value);
}

void ResetBrightness(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension value(1.0, DimensionUnit::VP);
    ViewAbstract::SetBrightness(frameNode, value);
}

void SetBlur(NodeHandle node, double value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    float blur = 0.0f;
    if (value > 0) {
        blur = value;
    }
    CalcDimension dimensionBlur(blur, DimensionUnit::PX);
    ViewAbstract::SetFrontBlur(frameNode, dimensionBlur);
}

void ResetBlur(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    double blur = 0.0;
    CalcDimension dimensionBlur(blur, DimensionUnit::PX);
    ViewAbstract::SetFrontBlur(frameNode, dimensionBlur);
}

/**
 * @param values value value
 * values[0], values[1] : angle: hasValue, angle value
 * values[2] : direction
 * values[3] : repeating
 * @param valuesLength values length
 * @param colors color value
 * colors[0], colors[1], colors[2] : color[0](color, hasDimension, dimension)
 * colors[3], colors[4], colors[5] : color[1](color, hasDimension, dimension)
 * ...
 * @param colorsLength colors length
 */
void SetLinearGradient(NodeHandle node, const double* values, int32_t valuesLength,
    const double* colors, int32_t colorsLength)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if ((values == nullptr) || (valuesLength != NUM_4) || (colors == nullptr) || ((colorsLength % NUM_3) != 0)) {
        return;
    }
    NG::Gradient gradient;
    gradient.CreateGradientWithType(NG::GradientType::LINEAR);
    SetLinearGradientValues(gradient, values, valuesLength);
    SetGradientColors(gradient, colors, colorsLength);
    ViewAbstract::SetLinearGradient(frameNode, gradient);
}

void ResetLinearGradient(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NG::Gradient gradient;
    gradient.CreateGradientWithType(NG::GradientType::LINEAR);
    ViewAbstract::SetLinearGradient(frameNode, gradient);
}

ArkUICommonModifierAPI GetCommonModifier()
{
    static const ArkUICommonModifierAPI modifier = { SetBackgroundColor, ResetBackgroundColor, SetWidth, ResetWidth,
        SetHeight, ResetHeight, SetBorderRadius, ResetBorderRadius, SetBorderWidth, ResetBorderWidth, SetTransform,
        ResetTransform, SetBorderColor, ResetBorderColor, SetPosition, ResetPosition, SetBorderStyle, ResetBorderStyle,
        SetBackShadow, ResetBackShadow, SetHitTestBehavior, ResetHitTestBehavior, SetZIndex, ResetZIndex, SetOpacity,
        ResetOpacity, SetAlign, ResetAlign, SetBackdropBlur, ResetBackdropBlur, SetHueRotate, ResetHueRotate, SetInvert,
        ResetInvert, SetSepia, ResetSepia, SetSaturate, ResetSaturate, SetColorBlend, ResetColorBlend, SetGrayscale,
        ResetGrayscale, SetContrast, ResetContrast, SetBrightness, ResetBrightness, SetBlur, ResetBlur,
        SetLinearGradient, ResetLinearGradient,
    };

    return modifier;
}
} // namespace OHOS::Ace::NG