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

#include <cstdint>
#include <string>
#include "cj_transitioneffect.h"

namespace OHOS::Ace::Framework {
    NativeTransitionEffect::NativeTransitionEffect(RefPtr<NG::ChainedTransitionEffect> effect)
    {
        this->effect = effect;
    }

    void NativeTransitionEffect::Combine(sptr<OHOS::Ace::Framework::NativeTransitionEffect> tagEffect)
    {
        this->effect->SetNext(tagEffect->effect);
    }

    void NativeTransitionEffect::Animation(const std::shared_ptr<AnimationOption> option)
    {
        this->effect->SetAnimationOption(option);
    }
} // namespace OHOS::Ace::Framework