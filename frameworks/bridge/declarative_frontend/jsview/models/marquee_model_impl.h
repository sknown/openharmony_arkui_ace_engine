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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_MARQUEE_MODEL_IMPL_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_MARQUEE_MODEL_IMPL_H

#include <optional>
#include "bridge/declarative_frontend/jsview/js_container_base.h"
#include "core/components/marquee/marquee_component.h"
#include "core/components_ng/pattern/marquee/marquee_model.h"

namespace OHOS::Ace::Framework {
class ACE_EXPORT MarqueeModelImpl : public MarqueeModel {
public:
    void Create() override;
    void SetValue(const std::optional<std::string>& value) override;
    void SetPlayerStatus(const std::optional<bool>& playerStatus) override;
    void SetScrollAmount(const std::optional<double>& scrollAmount) override;
    void SetLoop(const std::optional<int32_t>& loop) override;
    void SetDirection(const std::optional<MarqueeDirection>& direction) override;
    void SetTextColor(const std::optional<Color>& textColor) override;
    void SetFontSize(const std::optional<Dimension>& fontSize) override;
    void SetFontWeight(const std::optional<FontWeight>& fontWeight) override;
    void SetFontFamily(const std::optional<std::vector<std::string>>& fontFamilies) override;
    void SetAllowScale(const std::optional<bool>& allowScale) override;
    void SetOnStart(std::function<void()>&& onChange) override;
    void SetOnBounce(std::function<void()>&& onChange) override;
    void SetOnFinish(std::function<void()>&& onChange) override;

private:
    static RefPtr<MarqueeComponent> GetComponent();
};
} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_MARQUEE_MODEL_IMPL_H
