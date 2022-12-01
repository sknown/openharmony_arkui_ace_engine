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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_INNER_SCROLL_BAR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_INNER_SCROLL_BAR_H

#include <cmath>

#include "base/geometry/dimension.h"
#include "base/geometry/offset.h"
#include "base/geometry/rect.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/edge.h"

namespace OHOS::Ace::NG {

constexpr double FACTOR_HALF = 0.5;
constexpr double DEFAULT_TOPANGLE = 60.0;
constexpr double DEFAULT_BOTTOMANGLE = 120.0;
constexpr double DEFAULT_MINANGLE = 10.0;
constexpr double STRAIGHT_ANGLE = 180.0;
constexpr Color PRESSED_BLEND_COLOR = Color(0x19000000);

enum class ShapeMode {
    /*
     * unspecified, follow theme.
     */
    DEFAULT = 0,
    /*
     * rect scrollbar.
     */
    RECT,
    /*
     * round scrollbar.
     */
    ROUND,
};

enum class DisplayMode {
    /*
     * do not display scrollbar.
     */
    OFF = 0,
    /*
     * display scrollbar on demand.
     */
    AUTO,
    /*
     * always display scrollbar.
     */
    ON,
};

enum class PositionMode {
    /*
     * display scrollbar on right.
     */
    RIGHT = 0,
    /*
     * display scrollbar on left.
     */
    LEFT,
    /*
     * display scrollbar on bottom.
     */
    BOTTOM,
};

class ScrollBar final : public AceType {
    DECLARE_ACE_TYPE(ScrollBar, AceType);

public:
    ScrollBar();
    explicit ScrollBar(
        DisplayMode displayMode,
        ShapeMode shapeMode = ShapeMode::RECT,
        PositionMode positionMode = PositionMode::RIGHT);
    ~ScrollBar() override = default;

    bool InBarRegion(const Point& point) const;
    bool NeedScrollBar() const;
    bool NeedPaint() const;
    void UpdateScrollBarRegion(
        const Offset& offset, const Size& size, const Offset& lastOffset, double estimatedHeight);
    double GetNormalWidthToPx() const;
    double CalcPatternOffset(double scrollBarOffset);

    ShapeMode GetShapeMode() const
    {
        return shapeMode_;
    }

    DisplayMode GetDisplayMode() const
    {
        return displayMode_;
    }

    PositionMode GetPositionMode() const
    {
        return positionMode_;
    }

    void SetPadding(const Edge& padding)
    {
        padding_ = padding;
    }

    const Edge& GetPadding() const
    {
        return padding_;
    }

    void SetBackgroundColor(const Color& backgroundColor)
    {
        backgroundColor_ = backgroundColor;
    }

    const Color& GetBackgroundColor() const
    {
        return backgroundColor_;
    }

    void SetForegroundColor(const Color& foregroundColor)
    {
        foregroundColor_ = foregroundColor;
    }

    Color GetForegroundColor() const
    {
        return IsPressed() ? foregroundColor_.BlendColor(PRESSED_BLEND_COLOR) : foregroundColor_;
    }

    double GetTopAngle() const
    {
        return topAngle_;
    }

    double GetBottomAngle() const
    {
        return bottomAngle_;
    }

    double GetTrickStartAngle() const
    {
        return trickStartAngle_;
    }

    double GetTrickSweepAngle() const
    {
        return trickSweepAngle_;
    }

    void SetMinHeight(const Dimension& minHeight)
    {
        minHeight_ = minHeight;
    }

    const Dimension& GetMinHeight() const
    {
        return minHeight_;
    }

    void SetMinDynamicHeight(const Dimension& minDynamicHeight)
    {
        minDynamicHeight_ = minDynamicHeight;
    }

    const Dimension& GetMinDynamicHeight() const
    {
        return minDynamicHeight_;
    }

    void SetReservedHeight(const Dimension& height)
    {
        reservedHeight_ = height;
    }

    const Dimension& GetReservedHeight() const
    {
        return reservedHeight_;
    }

    void SetInactiveWidth(const Dimension& inactiveWidth)
    {
        inactiveWidth_ = inactiveWidth;
    }

    void SetActiveWidth(const Dimension& activeWidth)
    {
        activeWidth_ = activeWidth;
    }

    const Dimension& GetActiveWidth() const
    {
        return activeWidth_;
    }

    void SetNormalWidth(const Dimension& normalWidth)
    {
        normalWidth_ = normalWidth;
    }

    const Rect& GetActiveRect() const
    {
        return activeRect_;
    }

    void SetTouchWidth(const Dimension& touchWidth)
    {
        touchWidth_ = touchWidth;
    }

    const Dimension& GetTouchWidth() const
    {
        return touchWidth_;
    }

    const Rect& GetBarRect() const
    {
        return barRect_;
    }

    void SetScrollable(bool isScrollable)
    {
        isScrollable_ = isScrollable;
    }

    bool IsScrollable() const
    {
        return isScrollable_;
    }

    void SetPositionMode(PositionMode positionMode)
    {
        positionMode_ = positionMode;
    }

    void SetShapeMode(ShapeMode shapeMode)
    {
        shapeMode_ = shapeMode;
    }

    void SetDisplayMode(DisplayMode displayMode)
    {
        displayMode_ = displayMode;
    }

    void SetOutBoundary(double outBoundary)
    {
        outBoundary_ = outBoundary;
    }

    void SetPosition(const Dimension& position)
    {
        position_ = position;
    }

    const Dimension& GetPosition() const
    {
        return position_;
    }

    void SetPressed(bool press)
    {
        isPressed_ = press;
    }

    bool IsPressed() const
    {
        return isPressed_;
    }

    void SetDriving(bool isDriving)
    {
        isDriving_ = isDriving;
    }

protected:
    void InitTheme();

private:
    void SetBarRegion(const Offset& offset, const Size& size);
    void SetRectTrickRegion(const Offset& offset, const Size& size, const Offset& lastOffset, double mainScrollExtent);
    void SetRoundTrickRegion(const Offset& offset, const Size& size, const Offset& lastOffset, double mainScrollExtent);
    double NormalizeToPx(const Dimension& dimension) const;

    DisplayMode displayMode_ = DisplayMode::OFF;
    ShapeMode shapeMode_ = ShapeMode::RECT;
    PositionMode positionMode_ = PositionMode::RIGHT;
    Edge padding_;
    Color backgroundColor_;
    Color foregroundColor_;
    Rect touchRegion_;
    Rect barRect_;
    Rect activeRect_;
    Dimension minHeight_; // this is min static height
    Dimension minDynamicHeight_; // this is min dynamic height when on the top or bottom
    Dimension reservedHeight_; // this is reservedHeight on the bottom
    Dimension inactiveWidth_;
    Dimension activeWidth_;
    Dimension normalWidth_;
    Dimension touchWidth_;

    Dimension position_;

    double trickStartAngle_ = 0.0;
    double trickSweepAngle_ = 0.0;
    double topAngle_ = DEFAULT_TOPANGLE;
    double bottomAngle_ = DEFAULT_BOTTOMANGLE;
    double minAngle_ = DEFAULT_MINANGLE;
    double outBoundary_ = 0.0;
    double offsetScale_ = 1.0f;

    bool isScrollable_ = false;

    bool isPressed_ = false;
    bool isDriving_ = false; // false: scroll driving; true: bar driving
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_INNER_SCROLL_BAR_H
