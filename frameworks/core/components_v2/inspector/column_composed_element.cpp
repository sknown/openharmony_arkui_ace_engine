/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "core/components_v2/inspector/column_composed_element.h"

#include "base/log/dump_log.h"
#include "core/components/common/layout/constants.h"
#include "core/components/flex/flex_element.h"
#include "core/components_v2/inspector/utils.h"

namespace OHOS::Ace::V2 {
namespace {

const std::unordered_map<std::string, std::function<std::string(const ColumnComposedElement&)>> CREATE_JSON_MAP {
    { "alignItems", [](const ColumnComposedElement& inspector) { return inspector.GetAlignContent(); } },
    { "space", [](const ColumnComposedElement& inspector) { return inspector.GetSpace(); } },
    { "selfAlign", [](const ColumnComposedElement& inspector) { return inspector.GetHorizontalAlign(); } },
    { "justifyContent", [](const ColumnComposedElement& inspector) { return inspector.GetJustifyContent(); } },
};

}

void ColumnComposedElement::Dump()
{
    InspectorComposedElement::Dump();
    DumpLog::GetInstance().AddDesc(
        std::string("alignContent: ").append(GetAlignContent()));
}

std::unique_ptr<JsonValue> ColumnComposedElement::ToJsonObject() const
{
    auto resultJson = InspectorComposedElement::ToJsonObject();
    for (const auto& value : CREATE_JSON_MAP) {
        resultJson->Put(value.first.c_str(), value.second(*this).c_str());
    }
    return resultJson;
}

std::string ColumnComposedElement::GetAlignContent() const
{
    auto node = GetInspectorNode(ColumnElement::TypeId());
    if (!node) {
        return "HorizontalAlign::Center";
    }

    RefPtr<RenderFlex> renderColumn = AceType::DynamicCast<RenderFlex>(node);
    if (renderColumn) {
        auto alignItems = renderColumn->GetAlignItems();
        if (alignItems == FlexAlign::FLEX_START) {
            return "HorizontalAlign.Start";
        } else if (alignItems == FlexAlign::CENTER) {
            return "HorizontalAlign.Center";
        } else if (alignItems == FlexAlign::FLEX_END) {
            return "HorizontalAlign.End";
        }
    }
    return "HorizontalAlign::Center";
}

std::string ColumnComposedElement::GetSpace() const
{
    auto node = GetInspectorNode(ColumnElement::TypeId());
    if (!node) {
        return Dimension(0.0).ToString();
    }
    auto renderColumn = AceType::DynamicCast<RenderFlex>(node);
    if (renderColumn) {
        auto dimension = renderColumn->GetInspectorSpace();
        return dimension.ToString();
    }
    return Dimension(0.0).ToString();
}

std::string ColumnComposedElement::GetHorizontalAlign() const
{
    auto node = GetInspectorNode(ColumnElement::TypeId());
    if (!node) {
        return "HorizontalAlign::Center";
    }
    auto renderColumn = AceType::DynamicCast<RenderFlex>(node);
    if (renderColumn) {
        auto alignPtr = renderColumn->GetAlignPtr();
        if (alignPtr) {
            auto horizontalAlign = alignPtr->GetHorizontalAlign();
            switch (horizontalAlign) {
                case HorizontalAlign::START:
                    return "HorizontalAlign::Start";
                case HorizontalAlign::CENTER:
                    return "HorizontalAlign::Center";
                case HorizontalAlign::END:
                    return "HorizontalAlign::End";
                default:
                    break;
            }
        }
    }
    return "HorizontalAlign::Center";
}

std::string ColumnComposedElement::GetJustifyContent() const
{
    auto node = GetRenderColumn();
    if (!node) {
        return "FlexAlign.Start";
    }
    auto flexAlign = node->GetJustifyContent();
    return ConvertFlexAlignToString(flexAlign);
}

std::string ColumnComposedElement::ConvertFlexAlignToString(FlexAlign flexAlign) const
{
    std::string result = "";
    switch (flexAlign) {
        case FlexAlign::FLEX_START:
            result = "FlexAlign.Start";
            break;
        case FlexAlign::FLEX_END:
            result = "FlexAlign.End";
            break;
        case FlexAlign::CENTER:
            result = "FlexAlign.Center";
            break;
        case FlexAlign::SPACE_BETWEEN:
            result = "FlexAlign.SpaceBetween";
            break;
        case FlexAlign::SPACE_AROUND:
            result = "FlexAlign.SpaceAround";
            break;
        case FlexAlign::SPACE_EVENLY:
            result = "FlexAlign.SpaceEvenly";
            break;
        default:
            result = "FlexAlign.Start";
            break;
    }
    return result;
}

RefPtr<RenderFlex> ColumnComposedElement::GetRenderColumn() const
{
    auto node = GetInspectorNode(ColumnElement::TypeId());
    if (node) {
        return AceType::DynamicCast<RenderFlex>(node);
    }
    LOGE("get GetInspectorNode failed");
    return nullptr;
}

void ColumnComposedElement::AddChildWithSlot(int32_t slot, const RefPtr<Component>& newComponent)
{
    auto flexElement = GetContentElement<FlexElement>(ColumnElement::TypeId());
    if (!flexElement) {
        LOGE("get GetFlexElement failed");
        return;
    }
    flexElement->UpdateChildWithSlot(nullptr, newComponent, slot, slot);
    flexElement->MarkDirty();
}

void ColumnComposedElement::UpdateChildWithSlot(int32_t slot, const RefPtr<Component>& newComponent)
{
    auto flexElement = GetContentElement<Element>(ColumnElement::TypeId());
    CHECK_NULL_VOID(flexElement);
    auto child = GetElementChildBySlot(flexElement, slot);
    CHECK_NULL_VOID(child);

    // Deals with the case where the component to be updated is a custom component.
    auto rootElement = flexElement;
    auto childComposedElement = AceType::DynamicCast<ComposedElement>(child);
    if (childComposedElement && childComposedElement->GetName() == "view") {
        rootElement = child;
        child = rootElement->GetChildren().empty() ? nullptr : rootElement->GetChildren().front();
    }

    // Replace the component with newComponent.
    auto context = rootElement->GetContext().Upgrade();
    auto needRebuildFocusElement = AceType::DynamicCast<Element>(rootElement->GetFocusScope());
    if (context && needRebuildFocusElement) {
        context->AddNeedRebuildFocusElement(needRebuildFocusElement);
    }
    ElementRegister::GetInstance()->RemoveItemSilently(child->GetElementId());
    rootElement->DeactivateChild(child);
    auto newChild = rootElement->InflateComponent(newComponent, child->GetSlot(), child->GetRenderSlot());
    ElementRegister::GetInstance()->AddElement(newChild);

    flexElement->MarkDirty();
}

void ColumnComposedElement::DeleteChildWithSlot(int32_t slot)
{
    auto flexElement = GetContentElement<FlexElement>(ColumnElement::TypeId());
    if (!flexElement) {
        LOGE("get GetFlexElement failed");
        return;
    }
    auto child = flexElement->GetChildBySlot(slot);
    flexElement->UpdateChildWithSlot(child, nullptr, slot, slot);
    flexElement->MarkDirty();
}

} // namespace OHOS::Ace::V2
