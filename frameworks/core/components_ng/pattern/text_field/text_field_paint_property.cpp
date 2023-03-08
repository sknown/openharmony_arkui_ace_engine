/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components/text_field/textfield_theme.h"
#include "core/components_ng/pattern/text_field/text_field_paint_property.h"

namespace OHOS::Ace::NG {
void TextFieldPaintProperty::ToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    PaintProperty::ToJsonValue(json);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto textFieldTheme = pipeline->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(textFieldTheme);

    json->Put("placeholderColor", propCursorColor_.value_or(Color()).ColorToString().c_str());
    auto jsonValue = JsonUtil::Create(true);
    jsonValue->Put("caretWidth", propCursorWidth_.value_or(textFieldTheme->GetCursorWidth()).ToString().c_str());
    json->Put("caretStyle", jsonValue->ToString().c_str());
    json->Put("selectedBackgroundColor",
        propSelectedBackgroundColor_.value_or(textFieldTheme->GetSelectedColor()).ColorToString().c_str());
}
} // namespace OHOS::Ace::NG
