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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RELATIVE_CONTAINER_RELATIVE_CONTAINER_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RELATIVE_CONTAINER_RELATIVE_CONTAINER_PATTERN_H

#include "base/log/dump_log.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/relative_container/relative_container_layout_algorithm.h"
#include "core/components_ng/pattern/relative_container/relative_container_layout_property.h"

namespace OHOS::Ace::NG {

class RelativeContainerPattern : public Pattern {
    DECLARE_ACE_TYPE(RelativeContainerPattern, Pattern);

public:
    RelativeContainerPattern() = default;
    ~RelativeContainerPattern() override = default;

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<RelativeContainerLayoutAlgorithm>();
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<RelativeContainerLayoutProperty>();
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    void SetTopologicalResult(std::string result)
    {
        topologicalResult_ = result;
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::SCOPE, true };
    }

    ScopeFocusAlgorithm GetScopeFocusAlgorithm() override
    {
        return { true, true, ScopeType::PROJECT_AREA };
    }

    void DumpInfo() override
    {
        DumpLog::GetInstance().AddDesc(std::string("topologicalResult:").append(topologicalResult_));
    }

private:
    std::string topologicalResult_;

};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RELATIVE_CONTAINER_RELATIVE_CONTAINER_PATTERN_H