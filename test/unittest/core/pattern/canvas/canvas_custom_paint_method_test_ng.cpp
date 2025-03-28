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

#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>

#include "gtest/gtest.h"
#include "mock_canvas_paint.h"

#define protected public
#define private public
#include "core/components_ng/pattern/custom_paint/canvas_event_hub.h"
#include "core/components_ng/pattern/custom_paint/canvas_layout_algorithm.h"
#include "core/components_ng/pattern/custom_paint/canvas_model.h"
#include "core/components_ng/pattern/custom_paint/canvas_model_ng.h"
#include "core/components_ng/pattern/custom_paint/canvas_modifier.h"
#include "core/components_ng/pattern/custom_paint/canvas_paint_mem.h"
#include "core/components_ng/pattern/custom_paint/canvas_paint_method.h"
#include "core/components_ng/pattern/custom_paint/canvas_paint_op.h"
#include "core/components_ng/pattern/custom_paint/canvas_pattern.h"
#include "core/components_ng/pattern/custom_paint/custom_paint_paint_method.h"
#include "core/components_ng/pattern/custom_paint/offscreen_canvas_paint_method.h"
#include "core/components_ng/pattern/custom_paint/offscreen_canvas_pattern.h"
#include "test/mock/core/common/mock_container.h"
#include "test/mock/core/rosen/mock_canvas.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

namespace {
const double MAX_WIDTH = 2.0;
const double MAX_INTRINSIC_WIDTH = 4.0;
const double SCALE = 0.5;
const double END_ANGLE = 8.0;
const double MAX_END_ANGLE = 1160.0;
const double HALF = 0.5;
const double HANGING_PERCENT = 0.8;
}

class CanvasCustomPaintMethodTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void CanvasCustomPaintMethodTestNg::SetUpTestCase()
{
    MockContainer::SetUp();
}

void CanvasCustomPaintMethodTestNg::TearDownTestCase()
{
    MockContainer::TearDown();
}

/**
 * @tc.name: CanvasCustomPaintMethodTest001
 * @tc.desc: Test the function 'CheckFilterProperty' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest001, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: When FilterType is GRAYSCALE, call the function CheckFilterProperty.
     * @tc.expected: return value are as expected.
     */
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::GRAYSCALE, "10"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::GRAYSCALE, "10.5"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::GRAYSCALE, "10%"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::GRAYSCALE, "10.5%"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::GRAYSCALE, " "));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::GRAYSCALE, "10px"));
    /**
     * @tc.steps3: When FilterType is SEPIA, call the function CheckFilterProperty.
     * @tc.expected: return value are as expected.
     */
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::SEPIA, "10"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::SEPIA, "10.5"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::SEPIA, "10%"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::SEPIA, "10.5%"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::SEPIA, " "));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::SEPIA, "10px"));
    /**
     * @tc.steps4: When FilterType is INVERT, call the function CheckFilterProperty.
     * @tc.expected: return value are as expected.
     */
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::INVERT, "10"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::INVERT, "10.5"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::INVERT, "10%"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::INVERT, "10.5%"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::INVERT, " "));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::INVERT, "10px"));
    /**
     * @tc.steps5: When FilterType is OPACITY, call the function CheckFilterProperty.
     * @tc.expected: return value are as expected.
     */
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::OPACITY, "10"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::OPACITY, "10.5"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::OPACITY, "10%"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::OPACITY, "10.5%"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::OPACITY, " "));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::OPACITY, "10px"));
    /**
     * @tc.steps5: When FilterType is BRIGHTNESS, call the function CheckFilterProperty.
     * @tc.expected: return value are as expected.
     */
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::BRIGHTNESS, "10"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::BRIGHTNESS, "10.5"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::BRIGHTNESS, "10%"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::BRIGHTNESS, "10.5%"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::BRIGHTNESS, " "));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::BRIGHTNESS, "10px"));
    /**
     * @tc.steps6: When FilterType is CONTRAST, call the function CheckFilterProperty.
     * @tc.expected: return value are as expected.
     */
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::CONTRAST, "10"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::CONTRAST, "10.5"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::CONTRAST, "10%"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::CONTRAST, "10.5%"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::CONTRAST, " "));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::CONTRAST, "10px"));
}

/**
 * @tc.name: CanvasCustomPaintMethodTest002
 * @tc.desc: Test the function 'CheckFilterProperty' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest002, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: When FilterType is BLUR, call the function CheckFilterProperty.
     * @tc.expected: return value are as expected.
     */
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::BLUR, "10px"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::BLUR, "10.5px"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::BLUR, "10rem"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::BLUR, "10.5rem"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::BLUR, "10%px"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::BLUR, "10%rem"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::BLUR, "10.5%px"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::BLUR, "10.5%rem"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::BLUR, " "));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::BLUR, "10"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::BLUR, "10.5"));
    /**
     * @tc.steps3: When FilterType is HUE_ROTATE, call the function CheckFilterProperty.
     * @tc.expected: return value are as expected.
     */
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::HUE_ROTATE, "10deg"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::HUE_ROTATE, "10.5deg"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::HUE_ROTATE, "10rad"));
    EXPECT_TRUE(paintMethod->CheckFilterProperty(FilterType::HUE_ROTATE, "10.5rad"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::HUE_ROTATE, "10%deg"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::HUE_ROTATE, "10%rad"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::HUE_ROTATE, "10.5%deg"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::HUE_ROTATE, "10.5%rad"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::HUE_ROTATE, " "));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::HUE_ROTATE, "10"));
    EXPECT_FALSE(paintMethod->CheckFilterProperty(FilterType::HUE_ROTATE, "10.5"));
}

/**
 * @tc.name: CanvasCustomPaintMethodTest003
 * @tc.desc: Test the function 'ParseFilter' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest003, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: Call the function ParseFilter.
     * @tc.expected: return value are as expected.
     */
    std::vector<FilterProperty> filters;
    std::string filter = "gray";
    EXPECT_FALSE(paintMethod->ParseFilter(filter, filters));
    filter = "gray(50)";
    EXPECT_FALSE(paintMethod->ParseFilter(filter, filters));
    filter = "blur(10px";
    EXPECT_TRUE(paintMethod->ParseFilter(filter, filters));
    EXPECT_EQ(filters.size(), 1);
    EXPECT_EQ(filters[0].filterType_, FilterType::BLUR);
    EXPECT_EQ(filters[0].filterParam_, "10px");
}

/**
 * @tc.name: CanvasCustomPaintMethodTest004
 * @tc.desc: Test the function 'UpdateLineDash' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest004, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: Call the function UpdateLineDash.
     * @tc.expected: return value are as expected.
     */
    RSPen pen;
    paintMethod->state_.strokeState.SetLineDash({ { 1.0, 0.0 }, 1.0 });
    paintMethod->state_.strokeState.SetLineDashOffset(1.0);
    paintMethod->UpdateLineDash(pen);
    EXPECT_EQ(paintMethod->state_.strokeState.GetLineDash().lineDash.size(), 2);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest005
 * @tc.desc: Test the function 'MakeConicGradient' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest005, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: Call the function MakeConicGradient.
     * @tc.expected: return value are as expected.
     */
    Ace::Gradient gradient;
    gradient.SetType(Ace::GradientType::CONIC);
    EXPECT_EQ(paintMethod->MakeConicGradient(nullptr, gradient), nullptr);
    AnimatableDimension animatableDimension(1.0);
    gradient.GetConicGradient().centerX = animatableDimension;
    EXPECT_EQ(gradient.GetConicGradient().centerX.has_value(), true);
    auto result = paintMethod->MakeConicGradient(nullptr, gradient);
    EXPECT_EQ(result, nullptr);

    gradient.GetConicGradient().centerY = animatableDimension;
    EXPECT_EQ(gradient.GetConicGradient().centerY.has_value(), true);
    result = paintMethod->MakeConicGradient(nullptr, gradient);
    EXPECT_EQ(result, nullptr);

    gradient.GetConicGradient().startAngle = animatableDimension;
    EXPECT_EQ(gradient.GetConicGradient().startAngle.has_value(), true);
    OHOS::Ace::GradientColor gradientColor1(Color::BLACK);
    OHOS::Ace::GradientColor gradientColor2(Color::RED);
    OHOS::Ace::GradientColor gradientColor3(Color::BLUE);
    gradient.AddColor(gradientColor1);
    gradient.AddColor(gradientColor2);
    gradient.AddColor(gradientColor3);
    result = paintMethod->MakeConicGradient(nullptr, gradient);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest006
 * @tc.desc: Test the function 'GetFilterType' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest006, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    std::vector<FilterProperty> filters;
    /**
     * @tc.steps2: Call the function GetFilterType.
     * @tc.expected: return value are as expected.
     */
    paintMethod->SetFilterParam("none");
    EXPECT_TRUE(paintMethod->GetFilterType(filters));
    filters.clear();
    paintMethod->SetFilterParam("gray(10px)");
    EXPECT_FALSE(paintMethod->GetFilterType(filters));
    EXPECT_EQ(filters.size(), 0);
    filters.clear();
    paintMethod->SetFilterParam("blur(10px)");
    paintMethod->GetFilterType(filters);
    EXPECT_EQ(filters[0].filterType_, FilterType::BLUR);
    EXPECT_EQ(filters[0].filterParam_, "10px");
    EXPECT_EQ(filters.size(), 1);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest007
 * @tc.desc: Test the function 'IsPercentStr' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest007, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: Call the function IsPercentStr.
     * @tc.expected: return value are as expected.
     */
    std::string percentStr1 = "50.5%";
    EXPECT_TRUE(paintMethod->IsPercentStr(percentStr1));
    EXPECT_EQ(percentStr1, "50.5");

    std::string percentStr2 = "50px";
    EXPECT_FALSE(paintMethod->IsPercentStr(percentStr2));
    EXPECT_NE(percentStr2, "50");

    std::string percentStr3;
    EXPECT_FALSE(paintMethod->IsPercentStr(percentStr3));
}

/**
 * @tc.name: CanvasCustomPaintMethodTest008
 * @tc.desc: Test the function 'PxStrToDouble' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest008, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: Call the function PxStrToDouble.
     * @tc.expected: return value are as expected.
     */
    std::string str1 = "10";
    EXPECT_EQ(paintMethod->PxStrToDouble(str1), 0);
    str1 = "10px";
    EXPECT_EQ(paintMethod->PxStrToDouble(str1), 10);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest009
 * @tc.desc: Test the function 'BlurStrToDouble' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest009, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: Call the function BlurStrToDouble.
     * @tc.expected: return value are as expected.
     */
    std::string str1 = "10.0";
    EXPECT_EQ(paintMethod->BlurStrToDouble(str1), 0.0);
    str1 = "aaa";
    EXPECT_EQ(paintMethod->BlurStrToDouble(str1), 0.0);
    str1 = "10.0px";
    EXPECT_EQ(paintMethod->BlurStrToDouble(str1), 10.0);
    str1 = "20.0rem";
    EXPECT_EQ(paintMethod->BlurStrToDouble(str1), 300.0);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest010
 * @tc.desc: Test the function 'PercentStrToFloat' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest010, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: Call the function PercentStrToFloat.
     * @tc.expected: return value are as expected.
     */
    std::string str1 = "10";
    EXPECT_EQ(paintMethod->PercentStrToFloat(str1), 10);
    str1 = "10%";
    EXPECT_EQ(paintMethod->PercentStrToFloat(str1), 0.1f);
    str1 = "abc";
    EXPECT_EQ(paintMethod->PercentStrToFloat(str1), 0.0f);
    str1 = "";
    EXPECT_EQ(paintMethod->PercentStrToFloat(str1), 0.0f);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest011
 * @tc.desc: Test the function 'CheckNumberAndPercentage' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest011, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: Call the function CheckNumberAndPercentage.
     * @tc.expected: return value are as expected.
     */
    float result = 0.0f;
    EXPECT_FALSE(paintMethod->CheckNumberAndPercentage("x", true, result));
    EXPECT_FALSE(paintMethod->CheckNumberAndPercentage("", true, result));
    EXPECT_FALSE(paintMethod->CheckNumberAndPercentage("x101", true, result));
    EXPECT_FALSE(paintMethod->CheckNumberAndPercentage("1/01", true, result));
    EXPECT_TRUE(paintMethod->CheckNumberAndPercentage("88", true, result));
    EXPECT_EQ(result, 1.0f);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest012
 * @tc.desc: Test the function 'CalcTextScale' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest012, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: Call the function CalcTextScale.
     * @tc.expected: return value are as expected.
     */
    auto result = paintMethod->CalcTextScale(0.0, 1.0);
    EXPECT_FALSE(result.has_value());

    result = paintMethod->CalcTextScale(1.0, std::nullopt);
    EXPECT_FALSE(result.has_value());

    result =paintMethod->CalcTextScale(0.0, std::nullopt);
    EXPECT_FALSE(result.has_value());

    result = paintMethod->CalcTextScale(1.0, -1.0);
    EXPECT_EQ(result.value(), 0.0);

    result = paintMethod->CalcTextScale(1.0, 0.0);
    EXPECT_EQ(result.value(), 0.0);

    result = paintMethod->CalcTextScale(1.0, 1.0);
    EXPECT_FALSE(result.has_value());

    result = paintMethod->CalcTextScale(-1.0, 1.0);
    EXPECT_FALSE(result.has_value());

    result = paintMethod->CalcTextScale(MAX_INTRINSIC_WIDTH, MAX_WIDTH);
    EXPECT_EQ(result.value(), SCALE);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest013
 * @tc.desc: Test the function 'GetStrokePaint' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest013, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    Testing::MockPen mockPen;
    RSSamplingOptions options;
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: Call the function GetStrokePaint.
     * @tc.expected: return value are as expected.
     */
    paintMethod->state_.strokeState.paintStyle_ = PaintStyle::Color;
    EXPECT_CALL(mockPen, SetColor(_)).Times(AtLeast(1));
    EXPECT_CALL(mockPen, SetJoinStyle(_)).Times(AtLeast(1));
    EXPECT_CALL(mockPen, SetCapStyle(_)).Times(AtLeast(1));
    EXPECT_CALL(mockPen, SetWidth(_)).Times(AtLeast(1));
    EXPECT_CALL(mockPen, SetMiterLimit(_)).Times(AtLeast(1));
    paintMethod->GetStrokePaint(mockPen, options);
    EXPECT_EQ(paintMethod->state_.strokeState.GetPaintStyle(), PaintStyle::Color);
    paintMethod->state_.globalState.SetAlpha(1.0);
    EXPECT_CALL(mockPen, SetAlphaF(_)).Times(AtLeast(1));
    paintMethod->GetStrokePaint(mockPen, options);
    EXPECT_TRUE(paintMethod->state_.globalState.HasGlobalAlpha());
    paintMethod->state_.strokeState.paintStyle_ = PaintStyle::Gradient;
    ASSERT_NE(paintMethod->state_.strokeState.GetPaintStyle(), PaintStyle::Color);
    EXPECT_CALL(mockPen, SetAlphaF(_)).Times(AtLeast(1));
    paintMethod->GetStrokePaint(mockPen, options);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest014
 * @tc.desc: Test the function 'DrawImageInternal' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest014, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    Testing::MockCanvas canvas;
    Ace::CanvasImage canvasImage;
    std::shared_ptr<RSImage> image;
    Testing::MockBrush brush;
    /**
     * @tc.steps2: Call the function DrawImageInternal.
     * @tc.expected: return value are as expected.
     */
    paintMethod->rsCanvas_ = std::make_shared<RSCanvas>();
    paintMethod->state_.globalState.SetType(CompositeOperation::SOURCE_ATOP);
    paintMethod->state_.globalState.SetAlpha(1.0);
    EXPECT_TRUE(paintMethod->state_.globalState.HasGlobalAlpha());
    EXPECT_CALL(canvas, SaveLayer(_)).WillRepeatedly(Return());
    EXPECT_CALL(brush, SetAlphaF(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    canvasImage.flag = CustomPaintPaintMethod::DrawImageType::THREE_PARAMS;
    EXPECT_CALL(canvas, DrawImage(_, _, _, _)).WillRepeatedly(Return());
    paintMethod->DrawImageInternal(canvasImage, image);
    canvasImage.flag = CustomPaintPaintMethod::DrawImageType::FIVE_PARAMS;
    EXPECT_CALL(canvas, DrawImageRect(_, _, _)).WillRepeatedly(Return());
    paintMethod->DrawImageInternal(canvasImage, image);
    canvasImage.flag = CustomPaintPaintMethod::DrawImageType::NINE_PARAMS;
    EXPECT_CALL(canvas, DrawImageRect(_, _, _)).WillRepeatedly(Return());
    paintMethod->DrawImageInternal(canvasImage, image);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest015
 * @tc.desc: Test the function 'FillRect' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest015, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    Testing::MockCanvas canvas;
    Testing::MockBrush brush;
    Testing::MockPath path;
    Rect rect;
    Offset offset(1.0, 1.0);
    /**
     * @tc.steps2: Call the function FillRect.
     * @tc.expected: return value are as expected.
     */
    paintMethod->rsCanvas_ = std::make_shared<RSCanvas>();
    paintMethod->state_.globalState.SetAlpha(1.0);
    EXPECT_TRUE(paintMethod->state_.globalState.HasGlobalAlpha());
    paintMethod->state_.globalState.SetType(CompositeOperation::SOURCE_OVER);
    paintMethod->state_.shadow.SetOffset(offset);
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawRect(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(brush, SetColor(_)).WillRepeatedly(Return());
    EXPECT_CALL(brush, SetAlphaF(_)).WillRepeatedly(Return());
    EXPECT_CALL(path, AddRect(_, _)).WillRepeatedly(Return());
    paintMethod->FillRect(rect);

    paintMethod->state_.globalState.SetType(CompositeOperation::SOURCE_ATOP);
    EXPECT_CALL(path, AddRect(_, _)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, SaveLayer(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawRect(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, Restore()).WillRepeatedly(Return());
    paintMethod->FillRect(rect);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest016
 * @tc.desc: Test the function 'StrokeRect' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest016, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    Testing::MockCanvas canvas;
    Testing::MockPath path;
    Rect rect;
    Offset offset(1.0, 1.0);
    /**
     * @tc.steps2: Call the function StrokeRect.
     * @tc.expected: return value are as expected.
     */
    paintMethod->rsCanvas_ = std::make_shared<RSCanvas>();
    paintMethod->state_.globalState.SetType(CompositeOperation::SOURCE_OVER);
    paintMethod->state_.shadow.SetOffset(offset);
    EXPECT_CALL(path, AddRect(_, _)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawRect(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    paintMethod->StrokeRect(rect);

    paintMethod->state_.globalState.SetType(CompositeOperation::SOURCE_ATOP);
    EXPECT_CALL(path, AddRect(_, _)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, SaveLayer(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawRect(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, Restore()).WillRepeatedly(Return());
    paintMethod->StrokeRect(rect);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest017
 * @tc.desc: Test the function 'SetFillRuleForPath' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest017, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    CanvasFillRule rule = CanvasFillRule::NONZERO;
    Testing::MockPath path;
    /**
     * @tc.steps2: Call the function SetFillRuleForPath.
     * @tc.expected: return value are as expected.
     */
    EXPECT_CALL(path, SetFillStyle(_)).WillRepeatedly(Return());
    paintMethod->SetFillRuleForPath(rule);

    rule = CanvasFillRule::EVENODD;
    EXPECT_CALL(path, SetFillStyle(_)).WillRepeatedly(Return());
    paintMethod->SetFillRuleForPath(rule);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest018
 * @tc.desc: Test the function 'SetFillRuleForPath2D' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest018, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    CanvasFillRule rule = CanvasFillRule::NONZERO;
    Testing::MockPath path;
    paintMethod->rsCanvas_ = std::make_shared<RSCanvas>();
    /**
     * @tc.steps2: Call the function SetFillRuleForPath2D.
     * @tc.expected: return value are as expected.
     */
    EXPECT_CALL(path, SetFillStyle(_)).WillRepeatedly(Return());
    paintMethod->SetFillRuleForPath2D(rule);

    rule = CanvasFillRule::EVENODD;
    EXPECT_CALL(path, SetFillStyle(_)).WillRepeatedly(Return());
    paintMethod->SetFillRuleForPath2D(rule);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest019
 * @tc.desc: Test the function 'Fill' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest019, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    Testing::MockBrush brush;
    Testing::MockCanvas canvas;
    Offset offset(1.0, 1.0);
    /**
     * @tc.steps2: Call the function Fill.
     * @tc.expected: return value are as expected.
     */
    paintMethod->rsCanvas_ = std::make_shared<RSCanvas>();
    paintMethod->state_.fillState.paintStyle_ = OHOS::Ace::PaintStyle::Color;
    paintMethod->state_.globalState.SetType(CompositeOperation::SOURCE_OVER);
    paintMethod->state_.shadow.SetOffset(offset);
    EXPECT_CALL(brush, SetColor(_)).WillRepeatedly(Return());
    paintMethod->state_.globalState.SetAlpha(1.0);
    EXPECT_TRUE(paintMethod->state_.globalState.HasGlobalAlpha());
    EXPECT_CALL(brush, SetAlphaF(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawPath(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    paintMethod->Fill();

    paintMethod->state_.fillState.paintStyle_ = OHOS::Ace::PaintStyle::Gradient;
    paintMethod->state_.globalState.SetType(CompositeOperation::SOURCE_ATOP);
    EXPECT_CALL(brush, SetAlphaF(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, SaveLayer(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawPath(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, Restore()).WillRepeatedly(Return());
    paintMethod->Fill();
}

/**
 * @tc.name: CanvasCustomPaintMethodTest020
 * @tc.desc: Test the function 'Path2DFill' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest020, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    Testing::MockCanvas canvas;
    Testing::MockBrush brush;
    Offset offset(1.0, 1.0);
    /**
     * @tc.steps2: Call the function Path2DFill.
     * @tc.expected: return value are as expected.
     */
    paintMethod->rsCanvas_ = std::make_shared<RSCanvas>();
    paintMethod->state_.fillState.paintStyle_ = OHOS::Ace::PaintStyle::Color;
    paintMethod->state_.globalState.SetAlpha(1.0);
    EXPECT_TRUE(paintMethod->state_.globalState.HasGlobalAlpha());
    paintMethod->state_.globalState.SetType(CompositeOperation::SOURCE_OVER);
    paintMethod->state_.shadow.SetOffset(offset);
    EXPECT_CALL(brush, SetColor(_)).WillRepeatedly(Return());
    EXPECT_CALL(brush, SetAlphaF(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawPath(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    paintMethod->Path2DFill();

    paintMethod->state_.fillState.paintStyle_ = OHOS::Ace::PaintStyle::Gradient;
    paintMethod->state_.globalState.SetType(CompositeOperation::SOURCE_ATOP);
    EXPECT_CALL(brush, SetAlphaF(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, SaveLayer(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawRect(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, Restore()).WillRepeatedly(Return());
    paintMethod->Path2DFill();
}

/**
 * @tc.name: CanvasCustomPaintMethodTest021
 * @tc.desc: Test the function 'Stroke' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest021, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    Testing::MockCanvas canvas;
    Testing::MockBrush brush;
    Offset offset(1.0, 1.0);
    /**
     * @tc.steps2: Call the function Stroke.
     * @tc.expected: return value are as expected.
     */
    paintMethod->rsCanvas_ = std::make_shared<RSCanvas>();
    paintMethod->state_.fillState.paintStyle_ = OHOS::Ace::PaintStyle::Color;
    paintMethod->state_.globalState.SetType(CompositeOperation::SOURCE_OVER);
    paintMethod->state_.shadow.SetOffset(offset);
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawPath(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    paintMethod->Stroke();

    paintMethod->state_.globalState.SetType(CompositeOperation::SOURCE_ATOP);
    EXPECT_CALL(canvas, SaveLayer(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawPath(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, Restore()).WillRepeatedly(Return());
    paintMethod->Stroke();
}

/**
 * @tc.name: CanvasCustomPaintMethodTest022
 * @tc.desc: Test the function 'Path2DStroke' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest022, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    Testing::MockCanvas canvas;
    Testing::MockBrush brush;
    Offset offset(1.0, 1.0);
    /**
     * @tc.steps2: Call the function Path2DStroke.
     * @tc.expected: return value are as expected.
     */
    paintMethod->rsCanvas_ = std::make_shared<RSCanvas>();
    paintMethod->state_.globalState.SetType(CompositeOperation::SOURCE_OVER);
    paintMethod->state_.shadow.SetOffset(offset);
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawPath(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    paintMethod->Path2DStroke();

    paintMethod->state_.globalState.SetType(CompositeOperation::SOURCE_ATOP);
    EXPECT_CALL(canvas, SaveLayer(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawPath(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, Restore()).WillRepeatedly(Return());
    paintMethod->Path2DStroke();
}

/**
 * @tc.name: CanvasCustomPaintMethodTest023
 * @tc.desc: Test the function 'Arc' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest023, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    ArcParam arcParam;
    Testing::MockPath path;
    /**
     * @tc.steps2: Call the function Arc.
     * @tc.expected: return value are as expected.
     */
    arcParam.x = 1.0;
    arcParam.y = 1.0;
    arcParam.radius = 1.0;
    arcParam.startAngle = 0.0;
    arcParam.endAngle = 1.0;
    EXPECT_CALL(path, ArcTo(_, _, _, _)).WillRepeatedly(Return());
    paintMethod->Arc(arcParam);
    arcParam.endAngle = M_PI * MAX_WIDTH;
    EXPECT_CALL(path, ArcTo(_, _, _, _)).WillRepeatedly(Return());
    paintMethod->Arc(arcParam);
    arcParam.endAngle = END_ANGLE;
    EXPECT_CALL(path, ArcTo(_, _, _, _)).WillRepeatedly(Return());
    paintMethod->Arc(arcParam);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest024
 * @tc.desc: Test the function 'Ellipse' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest024, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    EllipseParam param;
    Testing::MockPath path;
    Testing::MockMatrix matrix;
    /**
     * @tc.steps2: Call the function Ellipse.
     * @tc.expected: return value are as expected.
     */
    param.x = 1.0;
    param.y = 1.0;
    param.startAngle = 0.0;
    param.endAngle = 1.0;
    param.rotation = 1.0;
    
    EXPECT_CALL(matrix, Rotate(_, _, _)).WillRepeatedly(Return());
    EXPECT_CALL(path, Transform(_)).WillRepeatedly(Return());
    paintMethod->Ellipse(param);
    param.endAngle = M_PI * MAX_WIDTH;
    EXPECT_CALL(path, ArcTo(_, _, _, _)).WillRepeatedly(Return());
    paintMethod->Ellipse(param);
    param.endAngle = END_ANGLE;
    EXPECT_CALL(path, ArcTo(_, _, _, _)).WillRepeatedly(Return());
    paintMethod->Ellipse(param);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest025
 * @tc.desc: Test the function 'ParsePath2D' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest025, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    PathArgs args;
    Testing::MockPath path;
    RefPtr<CanvasPath2D> path2D = AceType::MakeRefPtr<CanvasPath2D>();
    /**
     * @tc.steps2: Call the function ParsePath2D.
     * @tc.expected: return value are as expected.
     */
    path2D->caches_.emplace_back(PathCmd::CMDS, args);
    EXPECT_CALL(path, AddPath(_)).WillRepeatedly(Return());
    paintMethod->ParsePath2D(path2D);
    path2D->caches_.emplace_back(PathCmd::TRANSFORM, args);
    EXPECT_CALL(path, AddPath(_)).WillRepeatedly(Return());
    paintMethod->ParsePath2D(path2D);
    path2D->caches_.emplace_back(PathCmd::MOVE_TO, args);
    EXPECT_CALL(path, AddPath(_)).WillRepeatedly(Return());
    paintMethod->ParsePath2D(path2D);
    path2D->caches_.emplace_back(PathCmd::LINE_TO, args);
    EXPECT_CALL(path, AddPath(_)).WillRepeatedly(Return());
    paintMethod->ParsePath2D(path2D);
    path2D->caches_.emplace_back(PathCmd::ARC, args);
    EXPECT_CALL(path, AddPath(_)).WillRepeatedly(Return());
    paintMethod->ParsePath2D(path2D);
    path2D->caches_.emplace_back(PathCmd::ARC_TO, args);
    EXPECT_CALL(path, AddPath(_)).WillRepeatedly(Return());
    paintMethod->ParsePath2D(path2D);
    path2D->caches_.emplace_back(PathCmd::QUADRATIC_CURVE_TO, args);
    EXPECT_CALL(path, AddPath(_)).WillRepeatedly(Return());
    paintMethod->ParsePath2D(path2D);
    path2D->caches_.emplace_back(PathCmd::BEZIER_CURVE_TO, args);
    EXPECT_CALL(path, AddPath(_)).WillRepeatedly(Return());
    paintMethod->ParsePath2D(path2D);
    path2D->caches_.emplace_back(PathCmd::ELLIPSE, args);
    EXPECT_CALL(path, AddPath(_)).WillRepeatedly(Return());
    paintMethod->ParsePath2D(path2D);
    path2D->caches_.emplace_back(PathCmd::RECT, args);
    EXPECT_CALL(path, AddPath(_)).WillRepeatedly(Return());
    paintMethod->ParsePath2D(path2D);
    path2D->caches_.emplace_back(PathCmd::CLOSE_PATH, args);
    EXPECT_CALL(path, AddPath(_)).WillRepeatedly(Return());
    paintMethod->ParsePath2D(path2D);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest026
 * @tc.desc: Test the function 'Path2DArc' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest026, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    PathArgs args;
    Testing::MockPath path;
    /**
     * @tc.steps2: Call the function Path2DArc.
     * @tc.expected: return value are as expected.
     */
    args.para4 = 0.0;
    args.para5 = 1.0;
    EXPECT_CALL(path, ArcTo(_, _, _, _)).WillRepeatedly(Return());
    paintMethod->Path2DArc(args);
    args.para5 = M_PI * MAX_WIDTH;
    EXPECT_CALL(path, ArcTo(_, _, _, _)).WillRepeatedly(Return());
    paintMethod->Path2DArc(args);
    args.para5 = END_ANGLE;
    EXPECT_CALL(path, ArcTo(_, _, _, _)).WillRepeatedly(Return());
    paintMethod->Path2DArc(args);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest027
 * @tc.desc: Test the function 'Path2DEllipse' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest027, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    PathArgs args;
    Testing::MockPath path;
    Testing::MockMatrix matrix;
    /**
     * @tc.steps2: Call the function Path2DEllipse.
     * @tc.expected: return value are as expected.
     */
    args.para5 = 1.0;
    args.para6 = 0.0;
    args.para7 = 1.0;
    EXPECT_CALL(matrix, Rotate(_, _, _)).WillRepeatedly(Return());
    EXPECT_CALL(path, Transform(_)).WillRepeatedly(Return());
    EXPECT_CALL(path, ArcTo(_, _, _, _)).WillRepeatedly(Return());
    paintMethod->Path2DEllipse(args);
    args.para8 = 1.0;
    args.para7 = M_PI * MAX_WIDTH;
    EXPECT_CALL(path, ArcTo(_, _, _, _)).WillRepeatedly(Return());
    paintMethod->Path2DEllipse(args);
    args.para8 = 0.0;
    args.para7 = MAX_END_ANGLE;
    EXPECT_CALL(path, ArcTo(_, _, _, _)).WillRepeatedly(Return());
    paintMethod->Path2DEllipse(args);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest028
 * @tc.desc: Test the function 'PaintText' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest028, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    paintMethod->paragraph_ = std::make_unique<RSParagraph>();
    paintMethod->rsCanvas_ = std::make_shared<RSCanvas>();
    Testing::MockTypography typography;
    Testing::MockCanvas canvas;
    /**
     * @tc.steps2: Call the function PaintText.
     * @tc.expected: return value are as expected.
     */
    float width = 2.f;
    std::optional<double> maxWidth = 0.5;
    int32_t settingApiVersion = 10;
    int32_t backupApiVersion = MockContainer::Current()->GetApiTargetVersion();
    MockContainer::Current()->SetApiTargetVersion(settingApiVersion);
    EXPECT_CALL(typography, Layout(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, Save()).WillRepeatedly(Return());
    EXPECT_CALL(canvas, Scale(_, _)).WillRepeatedly(Return());
    EXPECT_CALL(typography, Paint(_, _, _)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, Restore()).WillRepeatedly(Return());
    paintMethod->PaintText(width, 1.0, 1.0, maxWidth, true, true);
    MockContainer::Current()->SetApiTargetVersion(backupApiVersion);

    EXPECT_CALL(typography, Layout(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, Save()).WillRepeatedly(Return());
    EXPECT_CALL(canvas, Scale(_, _)).WillRepeatedly(Return());
    EXPECT_CALL(typography, Paint(_, _, _)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, Restore()).WillRepeatedly(Return());
    paintMethod->PaintText(width, 1.0, 1.0, maxWidth, true, false);

    maxWidth = 10.0;
    EXPECT_CALL(typography, Layout(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, Save()).WillRepeatedly(Return());
    EXPECT_CALL(canvas, Scale(_, _)).WillRepeatedly(Return());
    EXPECT_CALL(typography, Paint(_, _, _)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, Restore()).WillRepeatedly(Return());
    paintMethod->PaintText(width, 1.0, 1.0, maxWidth, true, true);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest029
 * @tc.desc: Test the function 'PaintText' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest029, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: Call the function PaintText.
     * @tc.expected: return value are as expected.
     */
    TextAlign align = TextAlign::LEFT;
    auto result = paintMethod->GetAlignOffset(align, MAX_WIDTH);
    EXPECT_EQ(result, 0.0);
    align = TextAlign::START;
    paintMethod->state_.fillState.SetOffTextDirection(TextDirection::RTL);
    result = paintMethod->GetAlignOffset(align, MAX_WIDTH);
    EXPECT_EQ(result, -MAX_WIDTH);
    align = TextAlign::RIGHT;
    result = paintMethod->GetAlignOffset(align, MAX_WIDTH);
    EXPECT_EQ(result, -MAX_WIDTH);
    align = TextAlign::END;
    paintMethod->state_.fillState.SetOffTextDirection(TextDirection::LTR);
    result = paintMethod->GetAlignOffset(align, MAX_WIDTH);
    EXPECT_EQ(result, -MAX_WIDTH);
    align = TextAlign::CENTER;
    result = paintMethod->GetAlignOffset(align, MAX_WIDTH);
    EXPECT_EQ(result, -1.0);
    align = TextAlign::JUSTIFY;
    result = paintMethod->GetAlignOffset(align, MAX_WIDTH);
    EXPECT_EQ(result, 0.0);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest030
 * @tc.desc: Test the function 'GetBaselineOffset' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest030, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: Call the function GetBaselineOffset.
     * @tc.expected: return value are as expected.
     */
    auto paragraph = std::make_unique<RSParagraph>();
    TextBaseline baseLine = TextBaseline::ALPHABETIC;
    auto result = paintMethod->GetBaselineOffset(baseLine, paragraph);
    EXPECT_EQ(result, 0.0);

    baseLine = TextBaseline::IDEOGRAPHIC;
    paintMethod->state_.fillState.SetOffTextDirection(TextDirection::RTL);
    result = paintMethod->GetBaselineOffset(baseLine, paragraph);
    EXPECT_EQ(result, 0.0);

    baseLine = TextBaseline::BOTTOM;
    result = paintMethod->GetBaselineOffset(baseLine, paragraph);
    EXPECT_EQ(result, -1.0);

    baseLine = TextBaseline::TOP;
    paintMethod->state_.fillState.SetOffTextDirection(TextDirection::LTR);
    result = paintMethod->GetBaselineOffset(baseLine, paragraph);
    EXPECT_EQ(result, 0.0);

    baseLine = TextBaseline::MIDDLE;
    result = paintMethod->GetBaselineOffset(baseLine, paragraph);
    EXPECT_EQ(result, -HALF);

    baseLine = TextBaseline::HANGING;
    result = paintMethod->GetBaselineOffset(baseLine, paragraph);
    EXPECT_EQ(result, -HANGING_PERCENT);
}

/**
 * @tc.name: CanvasCustomPaintMethodTest031
 * @tc.desc: Test the function 'GetEffectiveAlign' of the class 'CustomPaintPaintMethod'.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasCustomPaintMethodTestNg, CanvasCustomPaintMethodTest031, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    auto paintMethod = AceType::MakeRefPtr<OffscreenCanvasPaintMethod>();
    ASSERT_NE(paintMethod, nullptr);
    /**
     * @tc.steps2: Call the function GetEffectiveAlign.
     * @tc.expected: return value are as expected.
     */
    RSTextAlign align = RSTextAlign::START;
    RSTextDirection direction = RSTextDirection::LTR;
    auto result = paintMethod->GetEffectiveAlign(align, direction);
    EXPECT_EQ(result, RSTextAlign::LEFT);

    align = RSTextAlign::END;
    direction = RSTextDirection::LTR;
    result = paintMethod->GetEffectiveAlign(align, direction);
    EXPECT_EQ(result, RSTextAlign::RIGHT);

    align = RSTextAlign::JUSTIFY;
    result = paintMethod->GetEffectiveAlign(align, direction);
    EXPECT_EQ(result, RSTextAlign::JUSTIFY);
}
} // namespace OHOS::Ace::NG
