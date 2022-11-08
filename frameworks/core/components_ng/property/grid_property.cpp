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

#include "grid_property.h"

#include <cstddef>

#include "core/components/common/layout/grid_container_info.h"
#include "core/components_ng/pattern/grid_container/grid_container_layout_property.h"

namespace OHOS::Ace::NG {

Dimension GridProperty::GetWidth()
{
    // gridInfo_ must exist, because layout algorithm invoke UpdateContainer first
    return Dimension(gridInfo_->GetWidth());
}

Dimension GridProperty::GetOffset()
{
    // gridInfo_ must exist, because layout algorithm invoke UpdateContainer() first
    auto offset = gridInfo_->GetOffset();
    if (offset == UNDEFINED_DIMENSION) {
        return UNDEFINED_DIMENSION;
    }
    auto marginOffset = Dimension(gridInfo_->GetParent()->GetMarginLeft().ConvertToPx());
    return offset + marginOffset;
}

bool GridProperty::UpdateContainer(const RefPtr<Property>& container, const RefPtr<AceType>& host)
{
    auto gridContainer = DynamicCast<GridContainerLayoutProperty>(container);

    GridColumnInfo::Builder builder;
    auto containerInfo = AceType::MakeRefPtr<GridContainerInfo>(gridContainer->GetContainerInfoValue());
    builder.SetParent(containerInfo);
    for (const auto& item : typedPropertySet_) {
        builder.SetSizeColumn(item.type_, item.span_);
        builder.SetOffset(item.offset_, item.type_);
    }
    gridInfo_ = builder.Build();
    container_ = container;

    gridContainer->RegistGridChild(DynamicCast<FrameNode>(host));
    return true;
}

bool GridProperty::UpdateSpan(int32_t span, GridSizeType type)
{
    LOGD("Update grid span. (span=%i, type=%i)", span, type);
    if (span < 0) {
        LOGE("Span value is illegal.");
        return false;
    }
    if (!container_) {
        SetSpan(type, span);
        return true;
    }

    auto container = DynamicCast<GridContainerLayoutProperty>(container_);
    GridSizeType currentType = container->GetContainerInfo()->GetSizeType(); // working type, not UNDEFINED
    auto currentProp = GetTypedProperty(type);                               // working property

    return (currentProp->type_ == type || currentType == type) && SetSpan(type, span);
}

bool GridProperty::UpdateOffset(int32_t offset, GridSizeType type)
{
    LOGD("Update grid span. (offset=%u, type=%i)", offset, type);
    if (!container_) {
        SetOffset(type, offset);
        return true;
    }
    auto container = DynamicCast<GridContainerLayoutProperty>(container_);
    GridSizeType currentType = container->GetContainerInfo()->GetSizeType(); // working type, not UNDEFINED
    auto currentProp = GetTypedProperty(type);                               // working property

    return (currentProp->type_ == type || currentType == type) && SetOffset(type, offset);
}

bool GridProperty::SetSpan(GridSizeType type, int32_t span)
{
    auto item = std::find_if(typedPropertySet_.begin(), typedPropertySet_.end(),
        [type](const GridTypedProperty& p) { return p.type_ == type; });
    if (item == typedPropertySet_.end()) {
        typedPropertySet_.emplace_back(type, span, DEFAULT_GRID_OFFSET);
        return true;
    }
    if (item->span_ == span) {
        return false;
    }
    item->span_ = span;
    return true;
}

bool GridProperty::SetOffset(GridSizeType type, int32_t offset)
{
    auto item = std::find_if(typedPropertySet_.begin(), typedPropertySet_.end(),
        [type](const GridTypedProperty& p) { return p.type_ == type; });
    if (item == typedPropertySet_.end()) {
        typedPropertySet_.emplace_back(type, DEFAULT_GRID_SPAN, offset);
        return true;
    }
    if (item->offset_ == offset) {
        return false;
    }
    item->offset_ = offset;
    return true;
}

void GridProperty::ToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    if (!gridInfo_) {
        return;
    }
    const std::string sizeTypeStrs[] { "undefined", "xs", "sm", "md", "lg", "xl" };

    auto gridOffset = gridInfo_->GetOffset(GridSizeType::UNDEFINED);
    auto gridSpan = gridInfo_->GetColumns(GridSizeType::UNDEFINED);
    json->Put("gridSpan", std::to_string(gridSpan).c_str());
    json->Put("gridOffset", std::to_string(gridOffset).c_str());

    auto useSizeType = JsonUtil::Create(false);
    for (uint32_t i = 1; i < sizeof(sizeTypeStrs) / sizeof(std::string); i++) {
        auto type = static_cast<GridSizeType>(i);
        auto span = gridInfo_->GetColumns(type);
        auto offset = gridInfo_->GetOffset(type);

        auto typeStr = JsonUtil::Create(false);
        typeStr->Put("span", std::to_string(span).c_str());
        typeStr->Put("offset", std::to_string(offset).c_str());
        useSizeType->Put(sizeTypeStrs[i].c_str(), typeStr);
    }
    json->Put("useSizeType", useSizeType);
}

} // namespace OHOS::Ace::NG
