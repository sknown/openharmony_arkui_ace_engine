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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_GEOMETRY_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_GEOMETRY_NODE_H

#include <list>
#include <memory>

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/macros.h"
#include "base/utils/utils.h"
#include "core/components_ng/layout/box_layout_algorithm.h"
#include "core/components_ng/property/geometry_property.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/magic_layout_property.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/property/position_property.h"

namespace OHOS::Ace::NG {
class InspectorFilter;

// GeometryNode acts as a physical property of the size and position of the component
class ACE_EXPORT GeometryNode : public AceType {
    DECLARE_ACE_TYPE(GeometryNode, AceType)
public:
    GeometryNode() = default;
    ~GeometryNode() override = default;

    void Reset();

    bool CheckUnchanged(const GeometryNode& geometryNode)
    {
        return (frame_ == geometryNode.frame_) && (margin_ == geometryNode.margin_) &&
               (content_ == geometryNode.content_) && (parentGlobalOffset_ == geometryNode.parentGlobalOffset_) &&
               (parentLayoutConstraint_ == geometryNode.parentLayoutConstraint_);
    }

    RefPtr<GeometryNode> Clone() const;

    SizeF GetMarginFrameSize(bool withSafeArea = false) const
    {
        // TODO: add margin in negative.
        auto size = frame_.rect_.GetSize();
        if (withSafeArea) {
            size += selfAdjust_.GetSize();
        }
        if (margin_) {
            AddPaddingToSize(*margin_, size);
        }
        return size;
    }

    OffsetF GetMarginFrameOffset(bool withSafeArea = false) const
    {
        // TODO: add margin in negative.
        auto offset = frame_.rect_.GetOffset();
        if (withSafeArea) {
            offset += selfAdjust_.GetOffset();
        }
        if (margin_) {
            offset -= OffsetF(margin_->left.value_or(0), margin_->top.value_or(0));
        }
        return offset;
    }

    RectF GetMarginFrameRect(bool withSafeArea = false) const
    {
        auto offset = frame_.rect_.GetOffset();
        auto size = frame_.rect_.GetSize();
        if (withSafeArea) {
            offset += selfAdjust_.GetOffset();
            size += selfAdjust_.GetSize();
        }
        if (margin_) {
            offset -= OffsetF(margin_->left.value_or(0), margin_->top.value_or(0));
            AddPaddingToSize(*margin_, size);
        }
        return RectF(offset, size);
    }

    void SetMarginFrameOffset(const OffsetF& translate)
    {
        OffsetF offset;
        if (margin_) {
            offset += OffsetF(margin_->left.value_or(0), margin_->top.value_or(0));
        }
        frame_.rect_.SetOffset(translate + offset);
    }

    RectF GetFrameRect(bool withSafeArea = false) const
    {
        auto result = frame_.rect_;
        if (withSafeArea) {
            result += selfAdjust_;
        }
        return result;
    }

    SizeF GetFrameSize(bool withSafeArea = false) const
    {
        auto result = frame_.rect_.GetSize();
        if (withSafeArea) {
            result += selfAdjust_.GetSize();
        }
        return result;
    }

    OffsetF GetFrameOffset(bool withSafeArea = false) const
    {
        auto result = frame_.rect_.GetOffset();
        if (withSafeArea) {
            result += selfAdjust_.GetOffset();
        }
        return result;
    }

    void SetFrameOffset(const OffsetF& offset)
    {
        frame_.rect_.SetOffset(offset);
    }

    void SetFrameSize(const SizeF& size)
    {
        frame_.rect_.SetSize(size);
    }

    void SetFrameWidth(int32_t width)
    {
        frame_.rect_.SetWidth(width);
    }

    void SetFrameHeight(int32_t height)
    {
        frame_.rect_.SetHeight(height);
    }

    void SetMarginFrameOffsetX(int32_t offsetX)
    {
        float offset = offsetX;
        if (margin_) {
            offset += margin_->left.value_or(0);
        }
        frame_.rect_.SetLeft(offset);
    }

    void SetMarginFrameOffsetY(int32_t offsetY)
    {
        float offset = offsetY;
        if (margin_) {
            offset += margin_->top.value_or(0);
        }
        frame_.rect_.SetTop(offset);
    }

    SizeF GetPaddingSize(bool withSafeArea = false) const
    {
        auto size = frame_.rect_.GetSize();
        if (withSafeArea) {
            size += selfAdjust_.GetSize();
        }
        if (padding_) {
            MinusPaddingToSize(*padding_, size);
        }
        return size;
    }

    OffsetF GetPaddingOffset(bool withSafeArea = false) const
    {
        auto offset = frame_.rect_.GetOffset();
        if (withSafeArea) {
            offset += selfAdjust_.GetOffset();
        }
        if (padding_) {
            offset += OffsetF(padding_->left.value_or(0), padding_->top.value_or(0));
        }
        return offset;
    }

    RectF GetPaddingRect(bool withSafeArea = false) const
    {
        auto rect = frame_.rect_;
        if (withSafeArea) {
            rect += selfAdjust_;
        }
        if (padding_) {
            auto size = rect.GetSize();
            MinusPaddingToSize(*padding_, size);
            rect.SetSize(size);
            auto offset = rect.GetOffset();
            offset += OffsetF(padding_->left.value_or(0), padding_->top.value_or(0));
            rect.SetOffset(offset);
        }
        return rect;
    }

    void SetContentSize(const SizeF& size)
    {
        if (!content_) {
            content_ = std::make_unique<GeometryProperty>();
        }
        content_->rect_.SetSize(size);
    }

    void SetContentOffset(const OffsetF& translate)
    {
        if (!content_) {
            content_ = std::make_unique<GeometryProperty>();
        }
        content_->rect_.SetOffset(translate);
    }

    RectF GetContentRect() const
    {
        return content_ ? content_->rect_ : RectF();
    }

    SizeF GetContentSize() const
    {
        return content_ ? content_->rect_.GetSize() : SizeF();
    }

    OffsetF GetContentOffset() const
    {
        return content_ ? content_->rect_.GetOffset() : OffsetF();
    }

    SizeF GetPixelRoundContentSize() const
    {
        auto deltaSize = GetPixelGridRoundSize() - GetFrameSize();
        return content_ ? (content_->rect_.GetSize() + deltaSize) : SizeF();
    }

    OffsetF GetPixelRoundContentOffset() const
    {
        return content_ ? content_->rect_.GetOffset() : OffsetF();
    }

    const OffsetF& GetPixelRoundResult() const
    {
        return pixelRoundResult_;
    }

    void SetPixelRoundResult(const OffsetF& pixelRoundResult)
    {
        pixelRoundResult_ = pixelRoundResult;
    }

    const std::unique_ptr<GeometryProperty>& GetContent() const
    {
        return content_;
    }

    const std::unique_ptr<MarginPropertyF>& GetMargin() const
    {
        return margin_;
    }

    const std::unique_ptr<PaddingPropertyF>& GetPadding() const
    {
        return padding_;
    }

    void UpdateMargin(const MarginPropertyF& margin)
    {
        if (!margin_) {
            margin_ = std::make_unique<MarginPropertyF>(margin);
            return;
        }
        if (margin.left) {
            margin_->left = margin.left;
        }
        if (margin.right) {
            margin_->right = margin.right;
        }
        if (margin.top) {
            margin_->top = margin.top;
        }
        if (margin.bottom) {
            margin_->bottom = margin.bottom;
        }
    }

    void UpdatePaddingWithBorder(const PaddingPropertyF& padding)
    {
        if (!padding_) {
            padding_ = std::make_unique<PaddingPropertyF>(padding);
            return;
        }
        if (padding.left) {
            padding_->left = padding.left;
        }
        if (padding.right) {
            padding_->right = padding.right;
        }
        if (padding.top) {
            padding_->top = padding.top;
        }
        if (padding.bottom) {
            padding_->bottom = padding.bottom;
        }
    }

    const OffsetF& GetParentGlobalOffset() const
    {
        return parentGlobalOffset_;
    }

    void SetParentGlobalOffset(const OffsetF& parentGlobalOffset)
    {
        parentGlobalOffset_ = parentGlobalOffset;
    }

    const OffsetF& GetPixelGridRoundOffset() const
    {
        return pixelGridRoundOffset_;
    }

    void SetPixelGridRoundOffset(const OffsetF& pixelGridRoundOffset)
    {
        pixelGridRoundOffset_ = pixelGridRoundOffset;
    }

    const SizeF& GetPixelGridRoundSize() const
    {
        return pixelGridRoundSize_;
    }

    RectF GetPixelGridRoundRect() const
    {
        return RectF(pixelGridRoundOffset_, pixelGridRoundSize_);
    }

    void SetPixelGridRoundSize(const SizeF& pixelGridRoundSize)
    {
        pixelGridRoundSize_ = pixelGridRoundSize;
    }

    const OffsetF& GetParentAbsoluteOffset() const
    {
        return parentAbsoluteOffset_;
    }

    void SetParentAbsoluteOffset(const OffsetF& parentAbsoluteOffset)
    {
        parentAbsoluteOffset_ = parentAbsoluteOffset;
    }

    void ResetParentLayoutConstraint()
    {
        parentLayoutConstraint_ = std::nullopt;
    }

    void SetParentLayoutConstraint(const LayoutConstraintF& layoutConstraint)
    {
        parentLayoutConstraint_ = layoutConstraint;
    }

    const std::optional<LayoutConstraintF>& GetParentLayoutConstraint() const
    {
        return parentLayoutConstraint_;
    }

    void SetBaselineDistance(float baselineDistance)
    {
        baselineDistance_ = baselineDistance;
    }

    float GetBaselineDistance()
    {
        return baselineDistance_.value_or(frame_.rect_.GetY());
    }

    RectF GetParentAdjust() const;
    void SetParentAdjust(RectF parentAdjust);
    RectF GetSelfAdjust() const;
    void SetSelfAdjust(RectF selfAdjust);
    RectF GetFrameRectWithoutSafeArea() const;
    RectF GetFrameRectWithSafeArea() const;

    void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const;

private:
    // the layoutConstraint of prev measure task.
    std::optional<LayoutConstraintF> parentLayoutConstraint_;

    std::optional<float> baselineDistance_;

    // the frame size in parent local coordinate.
    GeometryProperty frame_;
    // the size of margin property.
    std::unique_ptr<MarginPropertyF> margin_;
    // the size of padding property.
    std::unique_ptr<MarginPropertyF> padding_;
    // the size of content rect in current node local coordinate.
    std::unique_ptr<GeometryProperty> content_;

    RectF parentAdjust_;
    RectF selfAdjust_;

    OffsetF parentGlobalOffset_;
    OffsetF parentAbsoluteOffset_;
    OffsetF pixelGridRoundOffset_;
    OffsetF pixelRoundResult_;
    SizeF pixelGridRoundSize_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_GEOMETRY_NODE_H
