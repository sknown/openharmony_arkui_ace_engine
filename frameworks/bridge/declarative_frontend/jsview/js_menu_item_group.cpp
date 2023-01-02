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

#include "bridge/declarative_frontend/jsview/js_menu_item_group.h"

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/menu/menu_item_group/menu_item_group_view.h"

namespace OHOS::Ace::Framework {
void JSMenuItemGroup::Create(const JSCallbackInfo& info)
{
    LOGI("JSMenuItemGroup::Create");
    if (Container::IsCurrentUseNewPipeline()) {
        NG::MenuItemGroupView::Create();

        if (info.Length() < 1 || !info[0]->IsObject()) {
            return;
        }
        auto obj = JSRef<JSObject>::Cast(info[0]);
        auto headerBuilder = obj->GetProperty("header");
        if (!headerBuilder.IsEmpty() && headerBuilder->IsFunction()) {
            LOGI("JSMenuItemGroup set header");
            RefPtr<NG::UINode> header;
            {
                auto headerBuilderFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSFunc>::Cast(headerBuilder));
                CHECK_NULL_VOID(headerBuilderFunc);
                NG::ScopedViewStackProcessor builderViewStackProcessor;
                headerBuilderFunc->Execute();
                header = NG::ViewStackProcessor::GetInstance()->Finish();
                CHECK_NULL_VOID(header);
            }
            NG::MenuItemGroupView::SetHeader(header);
        }
        auto footerBuilder = obj->GetProperty("footer");
        if (!footerBuilder.IsEmpty() && footerBuilder->IsFunction()) {
            RefPtr<NG::UINode> footer;
            {
                auto footerBuilderFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSFunc>::Cast(footerBuilder));
                CHECK_NULL_VOID(footerBuilderFunc);
                NG::ScopedViewStackProcessor builderViewStackProcessor;
                footerBuilderFunc->Execute();
                footer = NG::ViewStackProcessor::GetInstance()->Finish();
                CHECK_NULL_VOID(footer);
            }
            NG::MenuItemGroupView::SetFooter(footer);
        }
        return;
    }
}

void JSMenuItemGroup::JSBind(BindingTarget globalObj)
{
    JSClass<JSMenuItemGroup>::Declare("MenuItemGroup");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSMenuItemGroup>::StaticMethod("create", &JSMenuItemGroup::Create, opt);
    JSClass<JSMenuItemGroup>::Inherit<JSViewAbstract>();
    JSClass<JSMenuItemGroup>::Bind(globalObj);
}
} // namespace OHOS::Ace::Framework