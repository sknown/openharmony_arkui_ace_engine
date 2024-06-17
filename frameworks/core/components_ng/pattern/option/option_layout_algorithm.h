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
#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_OPTION_OPTION_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_OPTION_OPTION_LAYOUT_ALGORITHM_H

#include <optional>

#include "core/components_ng/layout/box_layout_algorithm.h"
#include "core/components_ng/layout/layout_algorithm.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT OptionLayoutAlgorithm : public BoxLayoutAlgorithm {
    DECLARE_ACE_TYPE(OptionLayoutAlgorithm, BoxLayoutAlgorithm);

public:
    OptionLayoutAlgorithm() = default;
    ~OptionLayoutAlgorithm() override = default;

    void Measure(LayoutWrapper* layoutWrapper) override;

    void Layout(LayoutWrapper* layoutWrapper) override;

private:
    void MeasureRow(const RefPtr<LayoutWrapper>& row, const LayoutConstraintF& constraint);
    void UpdateIconMargin(LayoutWrapper* layoutWrapper);
    std::optional<float> GetIdealWidth(LayoutWrapper* layoutWrapper);
    float horInterval_ = 0.0f;

    ACE_DISALLOW_COPY_AND_MOVE(OptionLayoutAlgorithm);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_OPTION_OPTION_LAYOUT_ALGORITHM_H
