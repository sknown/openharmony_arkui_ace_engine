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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_NAVIGATOR_MODEL_IMPL_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_NAVIGATOR_MODEL_IMPL_H

#include "core/components_ng/pattern/navigator/navigator_model.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_interactable_view.h"

namespace OHOS::Ace::Framework {

class NavigatorModelImpl : public OHOS::Ace::NavigatorModel {
public:
    void Create() override;
    void SetType(NavigatorType value) override;
    void SetActive(bool active) override;
    void SetUri(const std::string& uri) override;
    void SetParams(const std::string& params) override;
    void SetIsDefWidth(bool isDefWidth) override;
    void SetIsDefHeight(bool isDefHeight) override;
};

} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_NAVIGATOR_MODEL_IMPL_H
