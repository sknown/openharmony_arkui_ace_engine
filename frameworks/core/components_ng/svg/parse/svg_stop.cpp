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

#include "frameworks/core/components_ng/svg/parse/svg_stop.h"

#include "frameworks/core/components_ng/svg/parse/svg_constants.h"

namespace OHOS::Ace::NG {

namespace {
const char DOM_SVG_SRC_STOP_COLOR[] = "stop-color";
const char DOM_SVG_SRC_STOP_OPACITY[] = "stop-opacity";
const char VALUE_NONE[] = "none";
}

SvgStop::SvgStop() : SvgNode() {}

RefPtr<SvgNode> SvgStop::Create()
{
    return AceType::MakeRefPtr<SvgStop>();
}

const GradientColor& SvgStop::GetGradientColor() const
{
    return stopAttr_.gradientColor;
}

bool SvgStop::ParseAndSetSpecializedAttr(const std::string& name, const std::string& value)
{
    static const LinearMapNode<void (*)(const std::string&, SvgStopAttribute&)> attrs[] = {
        { SVG_OFFSET,
            [](const std::string& val, SvgStopAttribute& attribute) {
                attribute.gradientColor.SetDimension(SvgAttributesParser::ParseDimension(val));
            } },
        { DOM_SVG_SRC_STOP_COLOR,
            [](const std::string& val, SvgStopAttribute& attribute) {
                Color color = (val == VALUE_NONE ? Color::TRANSPARENT : SvgAttributesParser::GetColor(val));
                attribute.gradientColor.SetColor(color);
            } },
        { DOM_SVG_SRC_STOP_OPACITY,
            [](const std::string& val, SvgStopAttribute& attribute) {
                attribute.gradientColor.SetOpacity(SvgAttributesParser::ParseDouble(val));
            } },
        { SVG_STOP_COLOR,
            [](const std::string& val, SvgStopAttribute& attribute) {
                Color color = (val == VALUE_NONE ? Color::TRANSPARENT : SvgAttributesParser::GetColor(val));
                attribute.gradientColor.SetColor(color);
            } },
        { SVG_STOP_OPACITY,
            [](const std::string& val, SvgStopAttribute& attribute) {
                attribute.gradientColor.SetOpacity(SvgAttributesParser::ParseDouble(val));
            } },
    };
    auto attrIter = BinarySearchFindIndex(attrs, ArraySize(attrs), name.c_str());
    if (attrIter != -1) {
        attrs[attrIter].value(value, stopAttr_);
        return true;
    }
    return false;
}

} // namespace OHOS::Ace::NG
