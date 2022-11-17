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

#include <string>

#include "gtest/gtest.h"
#define private public
#define protected public
#include "base/memory/ace_type.h"
#include "core/components/common/properties/color.h"
#include "core/components/declaration/svg/svg_line_declaration.h"
#include "core/components_ng/svg/parse/svg_line.h"
#include "core/components_ng/svg/parse/svg_svg.h"
#include "core/components_ng/svg/svg_dom.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::string SVG_LABEL =
    "<svg width=\"400\" height=\"400\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"><line x1=\"10\" x2=\"300\" "
    "y1=\"50\" y2=\"50\" stroke-width=\"4\" fill=\"white\" stroke=\"blue\"></line></svg>";
constexpr float X1 = 10.0f;
constexpr float Y1 = 50.0f;
constexpr float X2 = 300.0f;
constexpr float Y2 = 50.0f;
} // namespace
class ParseLineTestNg : public testing::Test {};

/**
 * @tc.name: ParseTest001
 * @tc.desc: parse line label
 * @tc.type: FUNC
 */

HWTEST_F(ParseLineTestNg, ParseTest001, TestSize.Level1)
{
    auto svgStream = SkMemoryStream::MakeCopy(SVG_LABEL.c_str(), SVG_LABEL.length());
    EXPECT_NE(svgStream, nullptr);
    auto svgDom = SvgDom::CreateSvgDom(*svgStream, Color::BLACK);
    auto svg = AceType::DynamicCast<SvgSvg>(svgDom->root_);
    EXPECT_GT(svg->children_.size(), 0);
    auto svgLine = AceType::DynamicCast<SvgLine>(svg->children_.at(0));
    EXPECT_NE(svgLine, nullptr);
    auto lineDeclaration = AceType::DynamicCast<SvgLineDeclaration>(svgLine->declaration_);
    EXPECT_FLOAT_EQ(lineDeclaration->GetX1().ConvertToPx(), X1);
    EXPECT_FLOAT_EQ(lineDeclaration->GetY1().ConvertToPx(), Y1);
    EXPECT_FLOAT_EQ(lineDeclaration->GetX2().ConvertToPx(), X2);
    EXPECT_FLOAT_EQ(lineDeclaration->GetY2().ConvertToPx(), Y2);
}
} // namespace OHOS::Ace::NG