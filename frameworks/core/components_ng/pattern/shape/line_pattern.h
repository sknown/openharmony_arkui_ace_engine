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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SHAPE_LINE_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SHAPE_LINE_PATTERN_H

#include "base/memory/referenced.h"
#include "base/utils/noncopyable.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/shape/line_layout_algorithm.h"
#include "core/components_ng/pattern/shape/line_paint_method.h"
#include "core/components_ng/pattern/shape/line_paint_property.h"

namespace OHOS::Ace::NG {
class LinePattern : public Pattern {
    DECLARE_ACE_TYPE(LinePattern, Pattern);

public:
    LinePattern() = default;
    ~LinePattern() override = default;

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        return MakeRefPtr<LinePaintMethod>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<LineLayoutAlgorithm>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<LinePaintProperty>();
    }

private:
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, bool skipMeasure, bool skipLayout) override;
    ACE_DISALLOW_COPY_AND_MOVE(LinePattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SHAPE_LINE_PATTERN_H