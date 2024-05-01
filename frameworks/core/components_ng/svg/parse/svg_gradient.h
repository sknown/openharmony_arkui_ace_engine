/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SVG_PARSE_SVG_GRADIENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SVG_PARSE_SVG_GRADIENT_H

#include "frameworks/core/components_ng/svg/parse/svg_attributes_parser.h"
#include "frameworks/core/components_ng/svg/parse/svg_node.h"

namespace OHOS::Ace::NG {

class SvgGradient : public SvgNode {
    DECLARE_ACE_TYPE(SvgGradient, SvgNode);

public:
    explicit SvgGradient(GradientType gradientType);
    ~SvgGradient() override = default;

    static RefPtr<SvgNode> CreateLinearGradient();
    static RefPtr<SvgNode> CreateRadialGradient();
    void SetAttr(const std::string& name, const std::string& value) override;

    void OnAppendChild(const RefPtr<SvgNode>& child) override;

    const Gradient& GetGradient() const;

private:
    SvgGradientAttribute gradientAttr_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SVG_PARSE_SVG_GRADIENT_H