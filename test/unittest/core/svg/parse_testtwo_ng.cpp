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

#include <string>

#include "gtest/gtest.h"

#define private public
#define protected public

#include "include/core/SkStream.h"
#include "test/mock/core/rosen/mock_canvas.h"

#include "base/memory/ace_type.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"
#include "core/components/declaration/svg/svg_animate_declaration.h"
#include "core/components/declaration/svg/svg_circle_declaration.h"
#include "core/components/declaration/svg/svg_declaration.h"
#include "core/components/declaration/svg/svg_ellipse_declaration.h"
#include "core/components/declaration/svg/svg_fe_blend_declaration.h"
#include "core/components/declaration/svg/svg_fe_colormatrix_declaration.h"
#include "core/components/declaration/svg/svg_fe_composite_declaration.h"
#include "core/components/declaration/svg/svg_fe_declaration.h"
#include "core/components/declaration/svg/svg_fe_flood_declaration.h"
#include "core/components/declaration/svg/svg_fe_gaussianblur_declaration.h"
#include "core/components/declaration/svg/svg_filter_declaration.h"
#include "core/components/declaration/svg/svg_gradient_declaration.h"
#include "core/components/declaration/svg/svg_image_declaration.h"
#include "core/components/declaration/svg/svg_line_declaration.h"
#include "core/components/declaration/svg/svg_mask_declaration.h"
#include "core/components/declaration/svg/svg_path_declaration.h"
#include "core/components/declaration/svg/svg_pattern_declaration.h"
#include "core/components/declaration/svg/svg_polygon_declaration.h"
#include "core/components/declaration/svg/svg_rect_declaration.h"
#include "core/components/declaration/svg/svg_stop_declaration.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/svg/parse/svg_animation.h"
#include "core/components_ng/svg/parse/svg_circle.h"
#include "core/components_ng/svg/parse/svg_clip_path.h"
#include "core/components_ng/svg/parse/svg_defs.h"
#include "core/components_ng/svg/parse/svg_ellipse.h"
#include "core/components_ng/svg/parse/svg_fe_blend.h"
#include "core/components_ng/svg/parse/svg_fe_color_matrix.h"
#include "core/components_ng/svg/parse/svg_fe_composite.h"
#include "core/components_ng/svg/parse/svg_fe_flood.h"
#include "core/components_ng/svg/parse/svg_fe_gaussian_blur.h"
#include "core/components_ng/svg/parse/svg_filter.h"
#include "core/components_ng/svg/parse/svg_g.h"
#include "core/components_ng/svg/parse/svg_gradient.h"
#include "core/components_ng/svg/parse/svg_image.h"
#include "core/components_ng/svg/parse/svg_line.h"
#include "core/components_ng/svg/parse/svg_mask.h"
#include "core/components_ng/svg/parse/svg_path.h"
#include "core/components_ng/svg/parse/svg_pattern.h"
#include "core/components_ng/svg/parse/svg_polygon.h"
#include "core/components_ng/svg/parse/svg_rect.h"
#include "core/components_ng/svg/parse/svg_stop.h"
#include "core/components_ng/svg/parse/svg_style.h"
#include "core/components_ng/svg/parse/svg_svg.h"
#include "core/components_ng/svg/parse/svg_use.h"
#include "core/components_ng/svg/svg_dom.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
const std::string CIRCLE_SVG_LABEL =
    "<svg width=\"400px\" height=\"400px\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"><circle cx=\"60px\" "
    "cy=\"200px\" r = \"50px\" fill=\"red\" opacity=\"0.5\" stroke=\"blue\" stroke-width=\"16px\" "
    "stroke-opacity=\"0.3\" id=\"circleId\"/></svg>";
const std::string CLIP_SVG_LABEL =
    "<svg width=\"120\" height=\"120\" viewBox=\"0 0 120 120\" version=\"1.1\"><defs><clipPath id=\"myClip\"><circle "
    "cx=\"30\" cy=\"30\" r=\"20\"/><circle cx=\"70\" cy=\"70\" r=\"30\"/></clipPath></defs><rect x=\"10\" y=\"10\" "
    "width=\"100\" height=\"100\" clip-path=\"url(#myClip)\" fill=\"red\" /></svg>";
const std::string ID = "myClip";
const std::string SVG_LABEL = "<svg width=\"400\" height=\"500\" viewBox=\"-4 -10 300 300\"></svg>";
const std::string USE_SVG_LABEL =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"24\" height=\"24\" "
    "version=\"1.1\" viewBox=\"0 0 24 24\"><defs><path id=\"uxs-a\" d=\"M11.5,13.5 C12.7426407,13.5 13.75,14.5073593 "
    "13.75,15.75 C13.75,16.9926407 12.7426407,18 11.5,18 C10.2573593,18 9.25,16.9926407 9.25,15.75 C9.25,14.5073593 "
    "10.2573593,13.5 11.5,13.5 Z M11.5006868,9 C13.6153489,9 15.5906493,9.84916677 16.9758057,11.3106505 "
    "C17.3557222,11.7115019 17.3387512,12.3444394 16.9378998,12.724356 C16.5370484,13.1042725 15.9041109,13.0873015 "
    "15.5241943,12.6864501 C14.5167672,11.62351 13.0663814,11 11.5006868,11 C9.93437756,11 8.48347933,11.6240033 "
    "7.47603048,12.6876625 C7.09624495,13.0886381 6.46331303,13.105816 6.06233747,12.7260305 C5.66136192,12.3462449 "
    "5.644184,11.713313 6.02396952,11.3123375 C7.40917586,9.84984392 9.38518621,9 11.5006868,9 Z M11.5002692,4.5 "
    "C14.7685386,4.5 17.818619,5.90678629 19.9943022,8.33155689 C20.3631417,8.74262367 20.3289097,9.37486259 "
    "19.9178429,9.74370206 C19.5067762,10.1125415 18.8745372,10.0783095 18.5056978,9.66724276 C16.703513,7.6587313 "
    "14.1912454,6.5 11.5002692,6.5 C8.80904291,6.5 6.29656204,7.6589485 4.49435171,9.66778779 C4.1255427,10.0788819 "
    "3.49330631,10.1131607 3.08221221,9.74435171 C2.67111811,9.3755427 2.63683928,8.74330631 3.00564829,8.33221221 "
    "C5.1813597,5.90704879 8.23169642,4.5 11.5002692,4.5 Z M11.4995363,-5.68434189e-14 C15.8001105,-5.68434189e-14 "
    "19.8214916,1.76017363 22.7244081,4.81062864 C23.1051374,5.21070819 23.0894509,5.84367883 22.6893714,6.22440812 "
    "C22.2892918,6.60513741 21.6563212,6.58945092 21.2755919,6.18937136 C18.7465254,3.53176711 15.2469734,2 "
    "11.4995363,2 C7.75253773,2 4.25335915,3.53140612 1.72434435,6.1884639 C1.34357805,6.58850824 "
    "0.71060597,6.60413618 0.310561632,6.22336988 C-0.0894827058,5.84260359 -0.105110646,5.2096315 "
    "0.27565565,4.80958716 C3.1785132,1.75975912 7.19946582,-5.68434189e-14 11.4995363,-5.68434189e-14 "
    "Z\"/></defs><use fill=\"red\" fill-rule=\"nonzero\" stroke=\"blue\" stroke-width=\"1\" "
    "transform=\"translate(.5 2.75)\" xlink:href=\"#uxs-a\"/></svg>";
const std::string HREF = "uxs-a";
const std::string FILL_RULE = "nonzero";
const std::string TRANSFORM = "translate(.5 2.75)";
constexpr int32_t INDEX_ONE = 1;
const std::string STYLE_SVG_LABEL = "<svg viewBox=\"0 0 10 10\"><style>circle{fill:gold;stroke:maroon;stroke-width : "
                                    "2px;}</style><circle cx =\"5\" cy=\"5\" r=\"4\" /></svg>";
const std::string STOP_SVG_LABEL =
    "<svg height=\"150\" width=\"400\"><defs><linearGradient id=\"grad1\" x1=\"0%\" y1=\"0%\" x2=\"100%\" "
    "y2=\"0%\"><stop offset=\"0%\" style=\"stop-color:rgb(255,255,0);stop-opacity:1\" /><stop offset=\"20px\" "
    "style=\"stop-color:rgb(255,0,0);stop-opacity:1\" /></linearGradient></defs><ellipse cx=\"200\" cy=\"70\" "
    "rx=\"85\" ry=\"55\" fill=\"url(#grad1)\" /></svg>";
constexpr int32_t CHILD_NUMBER = 2;
const std::string RECT_SVG_LABEL = "<svg width=\"400\" height=\"400\" version=\"1.1\" fill=\"red\" "
                                   "xmlns=\"http://www.w3.org/2000/svg\"><rect width=\"100\" height=\"100\" x=\"150\" "
                                   "y=\"20\" stroke-width=\"4\" stroke=\"#000000\" rx=\"10\" ry=\"10\"></rect></svg>";
const std::string RECT_SVG_LABEL2 = "<svg version=\"1.1\" fill=\"red\" "
                                    "xmlns=\"http://www.w3.org/2000/svg\"><rect width=\"100\" height=\"100\" x=\"150\" "
                                    "y=\"20\" stroke-width=\"4\" stroke=\"#000000\" rx=\"10\" ry=\"10\"></rect></svg>";
const std::string RECT_SVG_LABEL3 = "<svg version=\"1.1\" fill=\"red\" "
                                    "xmlns=\"http://www.w3.org/2000/svg\"><rect width=\"100\" height=\"100\" x=\"150\" "
                                    "y=\"20\" stroke-width=\"4\" stroke=\"#000000\" rx=\"1\" ry=\"-1\"></rect></svg>";
constexpr float X = 150.0f;
constexpr float Y = 20.0f;
constexpr float RX = 10.0f;
constexpr float RY = 10.0f;
constexpr float RECT_WIDTH = 100.0f;
constexpr float RECT_HEIGHT = 100.0f;
const std::string POLYGON_SVG_LABEL1 =
    "<svg fill=\"white\" stroke=\"blue\" width=\"800\" height=\"400\" version=\"1.1\" "
    "xmlns=\"http://www.w3.org/2000/svg\"><polygon points=\"10,110 60,35 60,85 110,10\" "
    "fill=\"red\"></polygon> <polyline points=\"10,200 60,125 60,175 110,100\" "
    "stroke-dasharray=\"10 5\" stroke-dashoffset=\"3\"></polyline></svg>";
const std::string POLYGON_SVG_LABEL2 =
    "<svg fill=\"white\" stroke=\"blue\" width=\"300\" height=\"400\" version=\"1.1\" "
    "xmlns=\"http://www.w3.org/2000/svg\"><polygon points=\"10,110 60,35 60,85 110,10\" "
    "fill=\"red\"></polygon> <polyline points=\"10,200 60,125 60,175 110,100\" "
    "stroke-dasharray=\"10 5\" stroke-dashoffset=\"3\"></polyline></svg>";
const std::string POLYGON_POINT = "10,110 60,35 60,85 110,10";
const std::string POLYLINE_POINT = "10,200 60,125 60,175 110,100";
const std::string PATTERN_SVG_LABEL =
    "<svg viewBox=\"0 0 230 100\"><defs><pattern id=\"star\" viewBox=\"0 0 10 10\" width=\"10\" "
    "height=\"10\"><polygon points=\"0,0 2,5 0,10 5,8 10,10 8,5 10,0 5,2\" /></pattern></defs><circle cx=\"50\" "
    "cy=\"50\" r=\"50\" fill=\"url(#star)\" /><circle cx=\"180\" cy=\"50\" r=\"40\"  fill=\"none\" stroke-width=\"20\" "
    "stroke=\"url(#star)\"/> </svg>";
const std::string PATH_SVG_LABEL1 =
    "<svg width=\"400\" height=\"800\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M 10,30 A 20,20 "
    "0,0,1 50,30 A 20,20 0,0,1 90,30 Q 90,60 50,90 Q 10,60 10,30 z\" stroke=\"blue\" stroke-width=\"3\" "
    "fill=\"red\"></path></svg>";
const std::string PATH_SVG_LABEL2 =
    "<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M 10,30 A 20,20 "
    "0,0,1 50,30 A 20,20 0,0,1 90,30 Q 90,60 50,90 Q 10,60 10,30 z\" stroke=\"blue\" stroke-width=\"3\" "
    "fill=\"red\"></path></svg>";
const std::string PATH_SVG_LABEL3 =
    "<svg width=\"-400\" height=\"-400\" viewBox=\"-4 -10 300 300\" version=\"1.1\" "
    "xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M 10,30 A 20,20 "
    "0,0,1 50,30 A 20,20 0,0,1 90,30 Q 90,60 50,90 Q 10,60 10,30 z\" stroke=\"blue\" stroke-width=\"3\" "
    "fill=\"red\"></path></svg>";
const std::string PATH_SVG_LABEL4 =
    "<svg width=\"300\" height=\"400\" viewBox=\"-4 -10 300 300\" version=\"1.1\" "
    "xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M 10,30 A 20,20 "
    "0,0,1 50,30 A 20,20 0,0,1 90,30 Q 90,60 50,90 Q 10,60 10,30 z\" stroke=\"blue\" stroke-width=\"3\" "
    "fill=\"red\"></path></svg>";
const std::string PATH_SVG_LABEL5 =
    "<svg width=\"400\" height=\"400\" viewBox=\"-4 -10 -300 -300\" version=\"1.1\" "
    "xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M 10,30 A 20,20 "
    "0,0,1 50,30 A 20,20 0,0,1 90,30 Q 90,60 50,90 Q 10,60 10,30 z\" stroke=\"blue\" stroke-width=\"3\" "
    "fill=\"red\"></path></svg>";
const std::string PATH_CMD = "M 10,30 A 20,20 0,0,1 50,30 A 20,20 0,0,1 90,30 Q 90,60 50,90 Q 10,60 10,30 z";
const std::string MASK_SVG_LABEL =
    "<svg width=\"50px\" height=\"50px\" viewBox=\"0 0 24 24\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" "
    "xmlns:xlink=\"http://www.w3.org/1999/xlink\"><defs><path d=\"M4.3,14l-2,1.8c-0.2,0.2 -0.5,0.2 -0.7,0c-0.1,-0.1 "
    "-0.1,-0.2 -0.1,-0.3V8.6c0,-0.3 0.2,-0.5 0.5,-0.5c0.1,0 0.2,0 0.3,0.1l2,1.8l0,0H10L7,3.5C6.7,2.9 6.9,2.3 "
    "7.5,2c0.6,-0.3 1.3,-0.1 1.7,0.4l6,7.6l0,0H21c1.1,0 2,0.9 2,2s-0.9,2 -2,2h-6l0,0l-5.8,7.6c-0.4,0.5 -1.1,0.7 "
    "-1.7,0.4c-0.6,-0.3 -0.8,-0.9 -0.5,-1.5l3,-6.5l0,0H4.3z\" id=\"path-1\"></path></defs><g stroke=\"none\" "
    "stroke-width=\"1\" fill=\"none\" fill-rule=\"evenodd\"><g><mask id=\"mask-2\" fill=\"#FFFFFF\"><use "
    "xlink:href=\"#path-1\"></use></mask><use id=\"myId\" fill=\"#FFFFFF\" fill-rule=\"nonzero\" "
    "xlink:href=\"#path-1\"></use></g></g></svg>";
const std::string MASK_ID = "mask-2";
const std::string LINE_SVG_LABEL =
    "<svg width=\"400\" height=\"400\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"><line x1=\"10\" x2=\"300\" "
    "y1=\"50\" y2=\"50\" stroke-width=\"4\" fill=\"white\" stroke=\"blue\"></line></svg>";
const std::string GRADIENT_SVG_LINEAR =
    "<svg height=\"150\" width=\"400\"><defs><linearGradient id=\"grad1\" x1=\"0%\" y1=\"0%\" x2=\"100%\" "
    "y2=\"0%\"><stop offset=\"0%\" style=\"stop-color:rgb(255,255,0);stop-opacity:1\" /><stop offset=\"100%\" "
    "style=\"stop-color:rgb(255,0,0);stop-opacity:1\" /></linearGradient></defs><ellipse cx=\"200\" cy=\"70\" "
    "rx=\"85\" ry=\"55\" fill=\"url(#grad1)\" /></svg>";
const std::string GRADIENT_SVG_RADIAL =
    "<svg height=\"150\" width=\"500\"><defs><radialGradient id=\"grad1\" cx=\"50%\" cy=\"50%\" r=\"50%\" fx=\"50%\" "
    "fy=\"50%\"><stop offset=\"0%\" style=\"stop-color:rgb(255,255,255);      stop-opacity:0\" /><stop offset=\"100%\" "
    "style=\"stop-color:rgb(0,0,255);stop-opacity:1\" /></radialGradient></defs><ellipse cx=\"200\" cy=\"70\" "
    "rx=\"85\" ry=\"55\" fill=\"url(#grad1)\" /></svg>";
const std::string G_SVG_LABEL = "<svg width=\"400\" height=\"500\"> <g id=\"myId\"> </g></svg>";
const std::string G_ID = "myId";
const std::string FILTER_SVG_LABEL =
    "<svg height=\"900\" width=\"900\"><filter id=\"composite\" y=\"0\" x=\"0\" width=\"900\" "
    "height=\"900\"><feTurbulence baseFrequency=\".05\" numOctaves=\"3\" result=\"B\"/><feComposite in2=\"B\" "
    "in=\"SourceGraphic\" operator=\"in\" /></filter><ellipse cx=\"100\" cy=\"87\" rx=\"75\" ry=\"87\" fill=\"red\" "
    "filter=\"url(#composite)\"/></svg>";
const std::string FILTER_ID = "composite";
const std::string FEGAUSS_SVG_LABEL =
    "<svg width=\"230\" height=\"120\"><filter id=\"blurMe\"><feGaussianBlur in=\"Graphic\" stdDeviation=\"5\" "
    "/></filter><circle cx=\"170\" cy=\"60\" r=\"50\" fill=\"green\" filter=\"url(#blurMe)\" /></svg>";
const std::string FEGAUSS_SVG_LABEL2 =
    "<svg width=\"-230\" height=\"-120\"><filter id=\"blurMe\"><feGaussianBlur in=\"Graphic\" stdDeviation=\"5\" "
    "/></filter><circle cx=\"170\" cy=\"60\" r=\"50\" fill=\"green\" filter=\"url(#blurMe)\" /></svg>";
const std::string COMPOSITE_SVG_LABEL =
    "<svg height=\"900\" width=\"900\"><filter id=\"composite\" y=\"0\" x=\"0\" width=\"100%\" "
    "height=\"100%\"><feComposite in2=\"B\" "
    "in=\"SourceGraphic\" operator=\"in\" /></filter><ellipse cx=\"100\" cy=\"87\" rx=\"75\" ry=\"87\" fill=\"red\" "
    "filter=\"url(#composite)\"/></svg>";
const std::string COLOR_MATRIX_SVG_LABEL =
    "<svg height=\"900\" width=\"900\"><filter id=\"linear\"><feColorMatrix type=\"matrix\" "
    "values=\"R 0 0 0 0 0 G 0 0 0 0 0 B 0 0 0 0 0 A 0\"></feColorMatrix ></filter><ellipse cx=\"100\" cy=\"87\" "
    "rx=\"75\" ry=\"87\" fill=\"red\" filter=\"url(#linear)\"></ellipse></svg>";
const std::string TYPE = "matrix";
const std::string VALUE = "R 0 0 0 0 0 G 0 0 0 0 0 B 0 0 0 0 0 A 0";
const std::string ELLIPSE_SVG_LABEL1 =
    "<svg fill=\"white\" width=\"400\" height=\"400\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"><ellipse "
    "cx=\"60\" cy=\"200\" rx=\"50\" ry=\"100\" stroke-width=\"4\" fill=\"red\" stroke=\"blue\"></ellipse></svg>";
const std::string ELLIPSE_SVG_LABEL2 =
    "<svg fill=\"white\" width=\"10\" height=\"10\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"><ellipse "
    "cx=\"60\" cy=\"200\" rx=\"50\" ry=\"100\" stroke-width=\"4\" fill=\"red\" stroke=\"blue\"></ellipse></svg>";
const std::string ELLIPSE_SVG_LABEL3 =
    "<svg fill=\"white\" width=\"10\" height=\"10\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"><ellipse "
    "cx=\"0.0\" cy=\"0.0\" rx=\"-1\" ry=\"-1\" stroke-width=\"4\" fill=\"red\" stroke=\"blue\"></ellipse></svg>";
const std::string ELLIPSE_SVG_LABEL4 =
    "<svg fill=\"white\" width=\"10\" height=\"10\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"><ellipse "
    "cx=\"0.0\" cy=\"0.0\" rx=\"1\" ry=\"-1\" stroke-width=\"4\" fill=\"red\" stroke=\"blue\"></ellipse></svg>";
constexpr float ELLIPSE_CX = 60.0f;
constexpr float ELLIPSE_CY = 200.0f;
constexpr float ELLIPSE_RX = 50.0f;
constexpr float ELLIPSE_RY = 100.0f;
const std::string SVG_ANIMATE_TRANSFORM(
    "<svg width=\"200px\" height=\"200px\" viewBox=\"0 0 100 100\" xmlns=\"http://www.w3.org/2000/svg\">"
    "<path d =\"M50 50L20 50A30 30 0 0 0 80 50Z\">"
    "<animateTransform attributeName =\"transform\" type=\"rotate\" repeatCount=\"3\" dur=\"1s\""
    " values=\"0 50 50;45 50 50;0 50 50\" keyTimes=\"0;0.5;1\"></animateTransform></path></svg>");

const std::string NONE_STR = "";
const std::string SATURATE_VALUE = "10";
const std::string HUE_ROTATE = "80";
const std::string FE_COLOR_MATRIX =
    "<svg width=\"900\" height=\"900\" viewBox=\"0 0 150 120\" xmlns=\"http://www.w3.org/2000/svg\">"
    "<filter id=\"colorMatrix\">"
        "<feColorMatrix in=\"SourceGraphic\" type=\"matrix\" values=\"R 0 0 0 0 0 G 0 0 0 0 0 B 0 0 0 0 0 A 0\" />"
        "<feColorMatrix type=\"saturate\" values=\"10\"/>"
        "<feColorMatrix type=\"hueRotate\" values=\"80\"/>"
        "<feColorMatrix type=\"luminanceToAlpha\" values=\"80\"/>"
    "</filter>"
    "<g>"
        "<circle cx=\"30\" cy=\"30\" r=\"20\" fill=\"red\" fill-opacity=\"0.5\" />"
    "</g>"
    "<g filter=\"url(#colorMatrix)\">"
        "<circle cx=\"80\" cy=\"30\" r=\"20\" fill=\"red\" fill-opacity=\"0.5\" />"
    "</g>"
"</svg>";

const std::string FE_GAUSSIAN_BLUR =
    "<svg width=\"900\" height=\"900\" viewBox=\"0 0 150 120\" xmlns=\"http://www.w3.org/2000/svg\">"
    "<filter id=\"colorMatrix\">"
        "<feGaussianBlur stdDeviation=\"10 50\"/>"
        "<feGaussianBlur stdDeviation=\"10\"/>"
        "<feGaussianBlur stdDeviation=\"abc abc\"/>"
    "</filter>"
    "<g>"
        "<rect width=\"90\" height=\"90\" fill=\"#0099cc\" filter=\"url(#blurFilter)\" />"
    "</g>"
"</svg>";

const std::string FE_FLOOD_AND_COMPOSITE =
    "<svg width=\"900\" height=\"900\" viewBox=\"0 0 150 120\" >"
    "<filter id=\"colorMatrix\">"
    "<feFlood flood-color=\"red\" flood-opacity=\"0\" result=\"flood\" /><feFlood flood-color=\"green\" "
    "flood-opacity=\"1\" result=\"flood1\" />"
    "<feComposite in=\"SourceAlpha\" in2=\"SourceGraphic\" operator=\"xor\" result=\"composite\" k1=\"1\" "
    "k2=\"0\"/></filter>"
    "<g><rect width=\"90\" height=\"90\" fill=\"#0099cc\" filter=\"url(#blurFilter)\" /></g></svg>";

const std::string FE_BLEND =
    "<svg width=\"900\" height=\"900\" viewBox=\"0 0 150 120\" xmlns=\"http://www.w3.org/2000/svg\">"
    "<filter id=\"colorMatrix\">"
        "<feBlend in=\"SourceGraphic\" in2=\"SourceAlpha\" mode=\"lighten\" />"
    "</filter>"
    "<g>"
        "<rect width=\"90\" height=\"90\" fill=\"#0099cc\" filter=\"url(#blurFilter)\" />"
    "</g>"
"</svg>";

const std::string IMAGE_HREF = "test.png";
const std::string IMAGE_LABEL =
    "<svg width=\"900\" height=\"900\" viewBox=\"0 0 150 120\" xmlns=\"http://www.w3.org/2000/svg\">"
    "<image id=\"image001\" x=\"150\" y=\"20\" width=\"100\" height=\"100\" href=\"test.png\" />"
"</svg>";

constexpr float IMAGE_COMPONENT_WIDTH = 100.0f;
constexpr float IMAGE_COMPONENT_HEIGHT = 100.0f;

std::unordered_map<std::string, std::shared_ptr<RSImageFilter>> resultHash;
} // namespace
class ParseTestTwoNg : public testing::Test {
public:
    static RefPtr<SvgDom> ParseRect(const std::string& svgLabel);
    RefPtr<SvgDom> parsePolygon(const std::string& svgLable);
    static RefPtr<SvgDom> ParsePath(const std::string& svgLabel);
    RefPtr<SvgDom> ParseFeGaussianblur(const std::string& svgLabel);
    static RefPtr<SvgDom> ParseEllipse(const std::string& svgLabel);
    void CallBack(Testing::MockCanvas& rSCanvas);
};

RefPtr<SvgDom> ParseTestTwoNg::ParseRect(const std::string& svgLabel)
{
    auto svgStream = SkMemoryStream::MakeCopy(RECT_SVG_LABEL.c_str(), RECT_SVG_LABEL.length());
    EXPECT_NE(svgStream, nullptr);
    ImageSourceInfo src;
    src.SetFillColor(Color::BLACK);
    auto svgDom = SvgDom::CreateSvgDom(*svgStream, src);
    auto svg = AceType::DynamicCast<SvgSvg>(svgDom->root_);
    EXPECT_NE(svg, nullptr);
    EXPECT_GT(static_cast<int32_t>(svg->children_.size()), 0);
    auto svgRect = AceType::DynamicCast<SvgRect>(svg->children_.at(0));
    EXPECT_NE(svgRect, nullptr);
    auto rectDeclaration = svgRect->rectAttr_;
    EXPECT_FLOAT_EQ(rectDeclaration.x.ConvertToPx(), X);
    EXPECT_FLOAT_EQ(rectDeclaration.y.ConvertToPx(), Y);
    EXPECT_FLOAT_EQ(rectDeclaration.rx.ConvertToPx(), RX);
    EXPECT_FLOAT_EQ(rectDeclaration.ry.ConvertToPx(), RY);
    EXPECT_FLOAT_EQ(rectDeclaration.width.ConvertToPx(), RECT_WIDTH);
    EXPECT_FLOAT_EQ(rectDeclaration.height.ConvertToPx(), RECT_HEIGHT);
    return svgDom;
}

RefPtr<SvgDom> ParseTestTwoNg::parsePolygon(const std::string& svgLable)
{
    auto svgStream = SkMemoryStream::MakeCopy(svgLable.c_str(), svgLable.length());
    EXPECT_NE(svgStream, nullptr);
    ImageSourceInfo src;
    src.SetFillColor(Color::BLACK);
    auto svgDom = SvgDom::CreateSvgDom(*svgStream, src);
    EXPECT_NE(svgDom, nullptr);
    auto svg = AceType::DynamicCast<SvgSvg>(svgDom->root_);
    EXPECT_NE(svg, nullptr);
    EXPECT_EQ(static_cast<int32_t>(svg->children_.size()), CHILD_NUMBER);
    auto svgPolygon = AceType::DynamicCast<SvgPolygon>(svg->children_.at(0));
    EXPECT_NE(svgPolygon, nullptr);
    auto svgPolyline = AceType::DynamicCast<SvgPolygon>(svg->children_.at(1));
    EXPECT_NE(svgPolyline, nullptr);
    auto polygonDeclaration = svgPolygon->polyAttr_;
    EXPECT_STREQ(polygonDeclaration.points.c_str(), POLYGON_POINT.c_str());
    auto polylineDeclaration = svgPolyline->polyAttr_;
    EXPECT_STREQ(polylineDeclaration.points.c_str(), POLYLINE_POINT.c_str());
    return svgDom;
}

RefPtr<SvgDom> ParseTestTwoNg::ParsePath(const std::string& svgLabel)
{
    auto svgStream = SkMemoryStream::MakeCopy(svgLabel.c_str(), svgLabel.length());
    EXPECT_NE(svgStream, nullptr);
    ImageSourceInfo src;
    src.SetFillColor(Color::BLACK);
    auto svgDom = SvgDom::CreateSvgDom(*svgStream, src);
    auto svg = AceType::DynamicCast<SvgSvg>(svgDom->root_);
    EXPECT_GT(static_cast<int32_t>(svg->children_.size()), 0);
    auto svgPath = AceType::DynamicCast<SvgPath>(svg->children_.at(0));
    EXPECT_NE(svgPath, nullptr);
    auto pathDeclaration = svgPath->d_;
    EXPECT_STREQ(pathDeclaration.c_str(), PATH_CMD.c_str());
    return svgDom;
}

RefPtr<SvgDom> ParseTestTwoNg::ParseFeGaussianblur(const std::string& svgLabel)
{
    auto svgStream = SkMemoryStream::MakeCopy(svgLabel.c_str(), svgLabel.length());
    EXPECT_NE(svgStream, nullptr);
    ImageSourceInfo src;
    src.SetFillColor(Color::BLACK);
    auto svgDom = SvgDom::CreateSvgDom(*svgStream, src);
    auto svg = AceType::DynamicCast<SvgSvg>(svgDom->root_);
    EXPECT_GT(svg->children_.size(), 0);
    auto svgFilter = AceType::DynamicCast<SvgFilter>(svg->children_.at(0));
    EXPECT_NE(svgFilter, nullptr);
    auto svgFeGaussiaBlur = AceType::DynamicCast<SvgFeGaussianBlur>(svgFilter->children_.at(0));
    EXPECT_NE(svgFeGaussiaBlur, nullptr);
    auto feDeclaration = svgFeGaussiaBlur->gaussianBlurAttr_;
    EXPECT_EQ(feDeclaration.edgeMode, SvgFeEdgeMode::EDGE_DUPLICATE);
    return svgDom;
}

RefPtr<SvgDom> ParseTestTwoNg::ParseEllipse(const std::string& svgLabel)
{
    auto svgStream = SkMemoryStream::MakeCopy(svgLabel.c_str(), svgLabel.length());
    EXPECT_NE(svgStream, nullptr);
    ImageSourceInfo src;
    src.SetFillColor(Color::BLACK);
    auto svgDom = SvgDom::CreateSvgDom(*svgStream, src);
    auto svg = AceType::DynamicCast<SvgSvg>(svgDom->root_);
    EXPECT_GT(svg->children_.size(), 0);
    auto svgEllipse = AceType::DynamicCast<SvgEllipse>(svg->children_.at(0));
    EXPECT_NE(svgEllipse, nullptr);
    auto ellipseDeclaration = svgEllipse->ellipseAttr_;
    EXPECT_FLOAT_EQ(ellipseDeclaration.cx.ConvertToPx(), ELLIPSE_CX);
    EXPECT_FLOAT_EQ(ellipseDeclaration.cy.ConvertToPx(), ELLIPSE_CY);
    EXPECT_FLOAT_EQ(ellipseDeclaration.rx.ConvertToPx(), ELLIPSE_RX);
    EXPECT_FLOAT_EQ(ellipseDeclaration.ry.ConvertToPx(), ELLIPSE_RY);
    return svgDom;
}

void ParseTestTwoNg::CallBack(Testing::MockCanvas& rSCanvas)
{
    EXPECT_CALL(rSCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rSCanvas));
    EXPECT_CALL(rSCanvas, DetachBrush()).WillRepeatedly(ReturnRef(rSCanvas));
    EXPECT_CALL(rSCanvas, AttachPen(_)).WillRepeatedly(ReturnRef(rSCanvas));
    EXPECT_CALL(rSCanvas, DetachPen()).WillRepeatedly(ReturnRef(rSCanvas));
    EXPECT_CALL(rSCanvas, DrawPath(_)).Times(AtLeast(1));
}

/**
 * @tc.name: ParseLineTest002
 * @tc.desc: Create an SvgLine and set path
 * @tc.type: FUNC
 */
HWTEST_F(ParseTestTwoNg, ParseLineTest002, TestSize.Level1)
{
    /* *
     * @tc.steps: step1. call AsPath
     * @tc.expected: Execute function return value not is nullptr
     */
    auto svgLine = AccessibilityManager::MakeRefPtr<SvgLine>();
    svgLine->AsPath(Size(IMAGE_COMPONENT_WIDTH, IMAGE_COMPONENT_HEIGHT));
    EXPECT_NE(svgLine, nullptr);
}

/**
 * @tc.name: ParseEllipseTest005
 * @tc.desc: Create an SvgEllipse and set path
 * @tc.type: FUNC
 */
HWTEST_F(ParseTestTwoNg, ParseEllipseTest005, TestSize.Level1)
{
    auto svgStream = SkMemoryStream::MakeCopy(ELLIPSE_SVG_LABEL3.c_str(), ELLIPSE_SVG_LABEL3.length());
    EXPECT_NE(svgStream, nullptr);

    /* *
     * @tc.steps: step1. call CreateSvgDom
     * @tc.expected: Execute function return value size not is 0
     */
    ImageSourceInfo src;
    src.SetFillColor(Color::BLACK);
    auto svgDom = SvgDom::CreateSvgDom(*svgStream, src);
    auto svg = AceType::DynamicCast<SvgSvg>(svgDom->root_);
    EXPECT_GT(svg->children_.size(), 0);

    /* *
     * @tc.steps: step2. call AsPath
     * @tc.expected: Execute function return value not is nullptr
     */
    auto svgEllipse = AceType::DynamicCast<SvgEllipse>(svg->children_.at(0));
    svgEllipse->AsPath(Size(IMAGE_COMPONENT_WIDTH, IMAGE_COMPONENT_HEIGHT));
    EXPECT_NE(svgEllipse, nullptr);
}

/**
 * @tc.name: ParseEllipseTest006
 * @tc.desc: Create an SvgEllipse and set patha
 * @tc.type: FUNC
 */
HWTEST_F(ParseTestTwoNg, ParseEllipseTest006, TestSize.Level1)
{
    auto svgStream = SkMemoryStream::MakeCopy(ELLIPSE_SVG_LABEL4.c_str(), ELLIPSE_SVG_LABEL4.length());
    EXPECT_NE(svgStream, nullptr);

    /* *
     * @tc.steps: step1. call CreateSvgDom
     * @tc.expected: Execute function return value size not is 0
     */
    ImageSourceInfo src;
    src.SetFillColor(Color::BLACK);
    auto svgDom = SvgDom::CreateSvgDom(*svgStream, src);
    auto svg = AceType::DynamicCast<SvgSvg>(svgDom->root_);
    EXPECT_GT(svg->children_.size(), 0);

    /* *
     * @tc.steps: step2. call AsPath
     * @tc.expected: Execute function return value not is nullptr
     */
    auto svgEllipse = AceType::DynamicCast<SvgEllipse>(svg->children_.at(0));
    svgEllipse->AsPath(Size(IMAGE_COMPONENT_WIDTH, IMAGE_COMPONENT_HEIGHT));
    EXPECT_NE(svgEllipse, nullptr);
}

/**
 * @tc.name: ParsePolygonTest003
 * @tc.desc: parse polygon and polyline label
 * @tc.type: FUNC
 */
HWTEST_F(ParseTestTwoNg, ParsePolygonTest003, TestSize.Level1)
{
    /* *
     * @tc.steps: step1. call CreateSvgDom
     * @tc.expected: Execute svgDom root node is 2
     */
    auto svgStream = SkMemoryStream::MakeCopy(POLYGON_SVG_LABEL1.c_str(), POLYGON_SVG_LABEL1.length());
    ImageSourceInfo src;
    src.SetFillColor(Color::BLACK);
    auto svgDom = SvgDom::CreateSvgDom(*svgStream, src);
    auto svg = AceType::DynamicCast<SvgSvg>(svgDom->root_);
    EXPECT_EQ(static_cast<int32_t>(svg->children_.size()), CHILD_NUMBER);

    /* *
     * @tc.steps: step2. call AsPath
     * @tc.expected: Execute SvgPolygon Points is empty
     */
    auto svgPolygon = AceType::DynamicCast<SvgPolygon>(svg->children_.at(0));
    auto declaration = svgPolygon->polyAttr_;
    declaration.points = "";
    svgPolygon->AsPath(Size(IMAGE_COMPONENT_WIDTH, IMAGE_COMPONENT_HEIGHT));
    EXPECT_TRUE(declaration.points.empty());

    /* *
     * @tc.steps: step3. call AsPath
     * @tc.expected: Execute SvgPolygon Points parse error
     */
    declaration.points = "ccc";
    svgPolygon->AsPath(Size(IMAGE_COMPONENT_WIDTH, IMAGE_COMPONENT_HEIGHT));
    EXPECT_FALSE(declaration.points.empty());
}

/**
 * @tc.name: ParseStyleTest002
 * @tc.desc: parse use label
 * @tc.type: FUNC
 */
HWTEST_F(ParseTestTwoNg, ParseStyleTest002, TestSize.Level1)
{
    /* *
     * @tc.steps: step1. call ParseCssStyle
     * @tc.expected: Execute function return value false
     */
    SvgStyle::ParseCssStyle("", nullptr);
    std::string str;
    PushAttr callback = [&str](const std::string& key, const std::pair<std::string, std::string>& value) { str = key; };
    SvgStyle::ParseCssStyle("body {font-style: oblique;}.normal {font-style: normal;}", callback);
    EXPECT_FALSE(str.empty());

    SvgStyle::ParseCssStyle("body font-style: oblique;}. {font-style: normal;}", callback);
    EXPECT_FALSE(str.empty());
}

/**
 * @tc.name: ParseRectTest004
 * @tc.desc: parse rect label
 * @tc.type: FUNC
 */
HWTEST_F(ParseTestTwoNg, ParseRectTest004, TestSize.Level1)
{
    auto svgStream = SkMemoryStream::MakeCopy(RECT_SVG_LABEL3.c_str(), RECT_SVG_LABEL3.length());
    ImageSourceInfo src;
    src.SetFillColor(Color::BLACK);
    auto svgDom = SvgDom::CreateSvgDom(*svgStream, src);
    auto svg = AceType::DynamicCast<SvgSvg>(svgDom->root_);
    EXPECT_GT(static_cast<int32_t>(svg->children_.size()), 0);

    /* *
     * @tc.steps: step1. call AsPath
     * @tc.expected: Execute function return value not is 0
     */
    auto svgRect = AceType::DynamicCast<SvgRect>(svg->children_.at(0));
    svgRect->AsPath(Size(IMAGE_COMPONENT_WIDTH, IMAGE_COMPONENT_HEIGHT));
    auto rectDeclaration = svgRect->rectAttr_;
    EXPECT_NE(rectDeclaration.rx.Value(), 0);
}

/**
 * @tc.name: ParseUseTest002
 * @tc.desc: parse use label
 * @tc.type: FUNC
 */
HWTEST_F(ParseTestTwoNg, ParseUseTest002, TestSize.Level1)
{
    auto svgStream = SkMemoryStream::MakeCopy(USE_SVG_LABEL.c_str(), USE_SVG_LABEL.length());
    ImageSourceInfo src;
    src.SetFillColor(Color::GREEN);
    auto svgDom = SvgDom::CreateSvgDom(*svgStream, src);
    auto svg = AceType::DynamicCast<SvgSvg>(svgDom->root_);
    EXPECT_GT(static_cast<int32_t>(svg->children_.size()), 0);

    /* *
     * @tc.steps: step1. call AsPath
     * @tc.expected: Execute function return value is true
     */
    auto svgUse = AceType::DynamicCast<SvgUse>(svg->children_.at(INDEX_ONE));
    svgUse->attributes_.href = "";
    svgUse->AsPath(Size(IMAGE_COMPONENT_WIDTH, IMAGE_COMPONENT_HEIGHT));
    EXPECT_TRUE(svgUse->attributes_.href.empty());
}

/**
 * @tc.name: ParseImageTest001
 * @tc.desc: parse image label
 * @tc.type: FUNC
 */
HWTEST_F(ParseTestTwoNg, ParseImageTest001, TestSize.Level1)
{
    auto svgStream = SkMemoryStream::MakeCopy(IMAGE_LABEL.c_str(), IMAGE_LABEL.length());
    ImageSourceInfo src;
    src.SetFillColor(Color::BLACK);
    auto svgDom = SvgDom::CreateSvgDom(*svgStream, src);
    auto svg = AceType::DynamicCast<SvgSvg>(svgDom->root_);
    EXPECT_GT(static_cast<int32_t>(svg->children_.size()), 0);

    /* *
     * @tc.steps: step1. call AsPath
     * @tc.expected: Execute function return value is true
     */
    auto svgImage = AceType::DynamicCast<SvgImage>(svg->children_.at(0));
    auto imageDeclaration = svgImage->imageAttr_;
    EXPECT_FLOAT_EQ(imageDeclaration.x.ConvertToPx(), X);
    EXPECT_FLOAT_EQ(imageDeclaration.y.ConvertToPx(), Y);
    EXPECT_FLOAT_EQ(imageDeclaration.width.ConvertToPx(), RECT_WIDTH);
    EXPECT_FLOAT_EQ(imageDeclaration.height.ConvertToPx(), RECT_HEIGHT);
    EXPECT_STREQ(imageDeclaration.href.c_str(), IMAGE_HREF.c_str());
}
} // namespace OHOS::Ace::NG
