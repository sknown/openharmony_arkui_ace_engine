/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/dialog/dialog_accessibility_property.h"

#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/dialog/dialog_pattern.h"

namespace OHOS::Ace::NG {
std::string DialogAccessibilityProperty::GetText() const
{
    if (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
        return "";
    }

    auto frameNode = host_.Upgrade();
    CHECK_NULL_RETURN(frameNode, "");
    auto dialogPattern = frameNode->GetPattern<DialogPattern>();
    CHECK_NULL_RETURN(dialogPattern, "");
    std::string title = dialogPattern->GetTitle();
    std::string message = dialogPattern->GetMessage();
    return title.append(message);
}
} // namespace OHOS::Ace::NG