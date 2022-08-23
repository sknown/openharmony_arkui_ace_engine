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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTIES_MEASURE_PROPERTIES_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTIES_MEASURE_PROPERTIES_H

#include <array>
#include <cstdint>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

#include "base/utils/utils.h"
#include "core/components_ng/property/calc_length.h"

namespace OHOS::Ace::NG {
enum class MeasureType {
    MATCH_PARENT,
    MATCH_CONTENT,
};

class CalcSize {
public:
    CalcSize() = default;
    ~CalcSize() = default;
    CalcSize(const CalcLength& width, const CalcLength& height) : width_(width), height_(height) {}

    void Reset()
    {
        width_.Reset();
        height_.Reset();
    }

    const CalcLength& Width() const
    {
        return width_;
    }

    const CalcLength& Height() const
    {
        return height_;
    }

    void SetWidth(const CalcLength& width)
    {
        width_ = width;
    }

    void SetHeight(const CalcLength& height)
    {
        height_ = height;
    }

    void SetSizeT(const CalcSize& Size)
    {
        width_ = Size.Width();
        height_ = Size.Height();
    }

    bool operator==(const CalcSize& Size) const
    {
        return (width_ == Size.width_) && (height_ == Size.height_);
    }

    bool operator!=(const CalcSize& Size) const
    {
        return !operator==(Size);
    }

    bool UpdateSizeWithCheck(const CalcSize& Size)
    {
        if ((width_ == Size.width_) && ((height_ == Size.height_))) {
            return false;
        }
        if (Size.width_.IsValid()) {
            width_ = Size.width_;
        }
        if (Size.height_.IsValid()) {
            height_ = Size.height_;
        }
        return true;
    }

    std::string ToString() const
    {
        static const int32_t precision = 2;
        std::stringstream ss;
        ss << "[" << std::fixed << std::setprecision(precision);
        ss << width_.ToString();
        ss << " x ";
        ss << height_.ToString();
        ss << "]";
        std::string output = ss.str();
        return output;
    }

private:
    CalcLength width_ { -1 };
    CalcLength height_ { -1 };
};

struct MeasureProperty {
    std::optional<CalcSize> minSize;
    std::optional<CalcSize> maxSize;
    std::optional<CalcSize> selfIdealSize;

    void Reset()
    {
        minSize.reset();
        maxSize.reset();
        selfIdealSize.reset();
    }

    bool operator==(const MeasureProperty& measureProperty) const
    {
        return (minSize == measureProperty.minSize) && (maxSize == measureProperty.maxSize) &&
               (selfIdealSize == measureProperty.selfIdealSize);
    }

    bool UpdateSelfIdealSizeWithCheck(const CalcSize& size)
    {
        if (selfIdealSize == size) {
            return false;
        }
        if (selfIdealSize.has_value()) {
            return selfIdealSize->UpdateSizeWithCheck(size);
        }
        selfIdealSize = size;
        return true;
    }

    bool UpdateMaxSizeWithCheck(const CalcSize& size)
    {
        if (maxSize == size) {
            return false;
        }
        if (maxSize.has_value()) {
            return maxSize->UpdateSizeWithCheck(size);
        }
        maxSize = size;
        return true;
    }

    bool UpdateMinSizeWithCheck(const CalcSize& size)
    {
        if (minSize == size) {
            return false;
        }
        if (minSize.has_value()) {
            return minSize->UpdateSizeWithCheck(size);
        }
        minSize = size;
        return true;
    }

    std::string ToString() const
    {
        std::string str;
        str.append("minSize: [").append(minSize.has_value() ? minSize->ToString() : "NA").append("]");
        str.append("maxSize: [").append(maxSize.has_value() ? maxSize->ToString() : "NA").append("]");
        str.append("selfIdealSize: [").append(selfIdealSize.has_value() ? selfIdealSize->ToString() : "NA").append("]");
        return str;
    }
};

template<typename T>
struct PaddingPropertyT {
    std::optional<T> left;
    std::optional<T> right;
    std::optional<T> top;
    std::optional<T> bottom;

    void SetEdges(const T& padding)
    {
        left = padding;
        right = padding;
        top = padding;
        bottom = padding;
    }

    bool operator==(const PaddingPropertyT& value) const
    {
        return (left == value.left) && (right == value.right) && (top == value.top) && (bottom == value.bottom);
    }

    bool operator!=(const PaddingPropertyT& value) const
    {
        return !(*this == value);
    }

    bool UpdateWithCheck(const PaddingPropertyT& value)
    {
        if (*this != value) {
            left = value.left;
            right = value.right;
            top = value.top;
            bottom = value.bottom;
            return true;
        }
        return false;
    }

    std::string ToString() const
    {
        std::string str;
        str.append("left: [").append(left.has_value() ? left->ToString() : "NA").append("]");
        str.append("right: [").append(right.has_value() ? right->ToString() : "NA").append("]");
        str.append("top: [").append(top.has_value() ? top->ToString() : "NA").append("]");
        str.append("bottom: [").append(bottom.has_value() ? bottom->ToString() : "NA").append("]");
        return str;
    }
};

template<>
struct PaddingPropertyT<float> {
    std::optional<float> left;
    std::optional<float> right;
    std::optional<float> top;
    std::optional<float> bottom;

    bool operator==(const PaddingPropertyT<float>& value) const
    {
        if (left.has_value() ^ value.left.has_value()) {
            return false;
        }
        if (!NearEqual(left.value_or(0), value.left.value_or(0))) {
            return false;
        }
        if (right.has_value() ^ value.right.has_value()) {
            return false;
        }
        if (!NearEqual(right.value_or(0), value.right.value_or(0))) {
            return false;
        }
        if (top.has_value() ^ value.top.has_value()) {
            return false;
        }
        if (!NearEqual(top.value_or(0), value.top.value_or(0))) {
            return false;
        }
        if (bottom.has_value() ^ value.bottom.has_value()) {
            return false;
        }
        if (!NearEqual(bottom.value_or(0), value.bottom.value_or(0))) {
            return false;
        }
        return true;
    }

    std::string ToString() const
    {
        std::string str;
        str.append("left: [").append(left.has_value() ? std::to_string(left.value()) : "NA").append("]");
        str.append("right: [").append(right.has_value() ? std::to_string(right.value()) : "NA").append("]");
        str.append("top: [").append(top.has_value() ? std::to_string(top.value()) : "NA").append("]");
        str.append("bottom: [").append(bottom.has_value() ? std::to_string(bottom.value()) : "NA").append("]");
        return str;
    }
};

template<typename T>
struct BorderRadiusPropertyT {
    std::optional<T> radiusTopLeft;
    std::optional<T> radiusTopRight;
    std::optional<T> radiusBottomLeft;
    std::optional<T> radiusBottomRight;

    void SetRadius(const T& borderRadius)
    {
        radiusTopLeft = borderRadius;
        radiusTopRight = borderRadius;
        radiusBottomLeft = borderRadius;
        radiusBottomRight = borderRadius;
    }

    bool operator==(const BorderRadiusPropertyT& value) const
    {
        return (radiusTopLeft == value.radiusTopLeft) && (radiusTopRight == value.radiusTopRight) &&
               (radiusBottomLeft == value.radiusBottomLeft) && (radiusBottomRight == value.radiusBottomRight);
    }

    bool UpdateWithCheck(const BorderRadiusPropertyT& value)
    {
        bool isModified = false;
        if (value.radiusTopLeft.has_value() && (radiusTopLeft != value.radiusTopLeft)) {
            radiusTopLeft = value.radiusTopLeft;
            isModified = true;
        }
        if (value.radiusTopRight.has_value() && (radiusTopRight != value.radiusTopRight)) {
            radiusTopRight = value.radiusTopRight;
            isModified = true;
        }
        if (value.radiusBottomLeft.has_value() && (radiusBottomLeft != value.radiusBottomLeft)) {
            radiusBottomLeft = value.radiusBottomLeft;
            isModified = true;
        }
        if (value.radiusBottomRight.has_value() && (radiusBottomRight != value.radiusBottomRight)) {
            radiusBottomRight = value.radiusBottomRight;
            isModified = true;
        }
        return isModified;
    }
};

template<>
struct BorderRadiusPropertyT<float> {
    std::optional<float> radiusTopLeft;
    std::optional<float> radiusTopRight;
    std::optional<float> radiusBottomLeft;
    std::optional<float> radiusBottomRight;

    bool operator==(const BorderRadiusPropertyT<float>& value) const
    {
        if (radiusTopLeft.has_value() ^ value.radiusTopLeft.has_value()) {
            return false;
        }
        if (!NearEqual(radiusTopLeft.value_or(0), value.radiusTopLeft.value_or(0))) {
            return false;
        }
        if (radiusTopRight.has_value() ^ value.radiusTopRight.has_value()) {
            return false;
        }
        if (!NearEqual(radiusTopRight.value_or(0), value.radiusTopRight.value_or(0))) {
            return false;
        }
        if (radiusBottomLeft.has_value() ^ value.radiusBottomLeft.has_value()) {
            return false;
        }
        if (!NearEqual(radiusBottomLeft.value_or(0), value.radiusBottomLeft.value_or(0))) {
            return false;
        }
        if (radiusBottomRight.has_value() ^ value.radiusBottomRight.has_value()) {
            return false;
        }
        if (!NearEqual(radiusBottomRight.value_or(0), value.radiusBottomRight.value_or(0))) {
            return false;
        }
        return true;
    }

    std::string ToString() const
    {
        std::string str;
        str.append("radiusTopLeft: [")
            .append(radiusTopLeft.has_value() ? std::to_string(radiusTopLeft.value()) : "NA")
            .append("]");
        str.append("radiusTopRight: [")
            .append(radiusTopRight.has_value() ? std::to_string(radiusTopRight.value()) : "NA")
            .append("]");
        str.append("radiusBottomLeft: [")
            .append(radiusBottomLeft.has_value() ? std::to_string(radiusBottomLeft.value()) : "NA")
            .append("]");
        str.append("radiusBottomRight: [")
            .append(radiusBottomRight.has_value() ? std::to_string(radiusBottomRight.value()) : "NA")
            .append("]");
        return str;
    }
};

using PaddingProperty = PaddingPropertyT<CalcLength>;
using MarginProperty = PaddingProperty;
using BorderRadiusProperty = BorderRadiusPropertyT<Dimension>;
using PaddingPropertyF = PaddingPropertyT<float>;
using MarginPropertyF = PaddingPropertyT<float>;
using BorderRadiusPropertyF = BorderRadiusPropertyT<float>;
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTIES_MEASURE_PROPERTIES_H