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

#include "core/components_ng/layout/layout_property.h"

#include <optional>

#include "base/geometry/ng/size_t.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/common/container.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/property/safe_area_insets.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
std::string VisibleTypeToString(VisibleType type)
{
    static const LinearEnumMapNode<VisibleType, std::string> visibilityMap[] = {
        { VisibleType::VISIBLE, "Visibility.Visible" },
        { VisibleType::INVISIBLE, "Visibility.Hidden" },
        { VisibleType::GONE, "Visibility.None" },
    };
    auto idx = BinarySearchFindIndex(visibilityMap, ArraySize(visibilityMap), type);
    if (idx >= 0) {
        return visibilityMap[idx].value;
    }
    return "Visibility.Visible";
}

VisibleType StringToVisibleType(const std::string& str)
{
    static const std::unordered_map<std::string, VisibleType> uMap {
        { "Visibility.Visible", VisibleType::VISIBLE },
        { "Visibility.Hidden", VisibleType::INVISIBLE },
        { "Visibility.None", VisibleType::GONE },
    };

    auto iter = uMap.find(str);
    if (iter != uMap.end()) {
        return iter->second;
    }
    return VisibleType::VISIBLE;
}

std::string TextDirectionToString(TextDirection type)
{
    static const LinearEnumMapNode<TextDirection, std::string> toStringMap[] = {
        { TextDirection::LTR, "Direction.Ltr" },
        { TextDirection::RTL, "Direction.Rtl" },
        { TextDirection::INHERIT, "Direction.Inherit" },
        { TextDirection::AUTO, "Direction.Auto" },
    };
    auto idx = BinarySearchFindIndex(toStringMap, ArraySize(toStringMap), type);
    if (idx >= 0) {
        return toStringMap[idx].value;
    }
    return "Direction.Ltr";
}

TextDirection StringToTextDirection(const std::string& str)
{
    static const std::unordered_map<std::string, TextDirection> uMap {
        { "Direction.Ltr", TextDirection::LTR },
        { "Direction.Rtl", TextDirection::RTL },
        { "Direction.Inherit", TextDirection::INHERIT },
        { "Direction.Auto", TextDirection::AUTO },
    };

    auto iter = uMap.find(str);
    if (iter != uMap.end()) {
        return iter->second;
    }
    return TextDirection::LTR;
}
} // namespace

void LayoutProperty::Reset()
{
    layoutConstraint_.reset();
    calcLayoutConstraint_.reset();
    padding_.reset();
    margin_.reset();
    borderWidth_.reset();
    outerBorderWidth_.reset();
    magicItemProperty_.Reset();
    positionProperty_.reset();
    measureType_.reset();
    layoutDirection_.reset();
    propVisibility_.reset();
    propIsBindOverlay_.reset();
    CleanDirty();
}

void LayoutProperty::ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    ACE_PROPERTY_TO_JSON_VALUE(calcLayoutConstraint_, MeasureProperty);
    ACE_PROPERTY_TO_JSON_VALUE(positionProperty_, PositionProperty);
    magicItemProperty_.ToJsonValue(json, filter);
    ACE_PROPERTY_TO_JSON_VALUE(flexItemProperty_, FlexItemProperty);
    ACE_PROPERTY_TO_JSON_VALUE(gridProperty_, GridProperty);
    /* no fixed attr below, just return */
    if (filter.IsFastFilter()) {
        return;
    }

    PaddingToJsonValue(json, filter);
    MarginToJsonValue(json, filter);

    json->PutExtAttr("visibility",
        VisibleTypeToString(propVisibility_.value_or(VisibleType::VISIBLE)).c_str(), filter);
    json->PutExtAttr("direction", TextDirectionToString(GetLayoutDirection()).c_str(), filter);
    json->PutExtAttr("pixelRound", PixelRoundToJsonValue().c_str(), filter);
}

void LayoutProperty::PaddingToJsonValue(std::unique_ptr<JsonValue>& json,
    const InspectorFilter& filter) const
{
    if (padding_) {
        if (!padding_->top.has_value() || !padding_->right.has_value()
            || !padding_->left.has_value() || !padding_->bottom.has_value()) {
            auto paddingJsonValue = JsonUtil::Create(true);
            paddingJsonValue->Put("top", padding_->top.has_value()
                ? padding_->top.value().ToString().c_str() : "0.00vp");
            paddingJsonValue->Put("right", padding_->right.has_value()
                ? padding_->right.value().ToString().c_str() : "0.00vp");
            paddingJsonValue->Put("bottom", padding_->bottom.has_value()
                ? padding_->bottom.value().ToString().c_str() : "0.00vp");
            paddingJsonValue->Put("left", padding_->left.has_value()
                ? padding_->left.value().ToString().c_str() : "0.00vp");
            json->PutExtAttr("padding", paddingJsonValue->ToString().c_str(), filter);
        } else {
            json->PutExtAttr("padding", padding_->ToJsonString().c_str(), filter);
        }
    } else {
        json->PutExtAttr("padding", "0.00vp", filter);
    }
}

void LayoutProperty::MarginToJsonValue(std::unique_ptr<JsonValue>& json,
    const InspectorFilter& filter) const
{
    if (margin_) {
        if (!margin_->top.has_value() || !margin_->right.has_value()
            || !margin_->left.has_value() || !margin_->bottom.has_value()) {
            auto marginJsonValue = JsonUtil::Create(true);
            marginJsonValue->Put("top", margin_->top.has_value()
                ? margin_->top.value().ToString().c_str() : "0.00vp");
            marginJsonValue->Put("right", margin_->right.has_value()
                ? margin_->right.value().ToString().c_str() : "0.00vp");
            marginJsonValue->Put("bottom", margin_->bottom.has_value()
                ? margin_->bottom.value().ToString().c_str() : "0.00vp");
            marginJsonValue->Put("left", margin_->left.has_value()
                ? margin_->left.value().ToString().c_str() : "0.00vp");
            json->PutExtAttr("margin", marginJsonValue->ToString().c_str(), filter);
        } else {
            json->PutExtAttr("margin", margin_->ToJsonString().c_str(), filter);
        }
    } else {
        json->PutExtAttr("margin", "0.00vp", filter);
    }
}

void LayoutProperty::FromJson(const std::unique_ptr<JsonValue>& json)
{
    UpdateCalcLayoutProperty(MeasureProperty::FromJson(json));
    UpdateLayoutWeight(json->GetDouble("layoutWeight"));
    UpdateAlignment(Alignment::GetAlignment(TextDirection::LTR, json->GetString("align")));
    auto padding = json->GetString("padding");
    if (padding != "0.0") {
        UpdatePadding(PaddingProperty::FromJsonString(padding));
    }
    auto margin = json->GetString("margin");
    if (margin != "0.0") {
        UpdateMargin(MarginProperty::FromJsonString(margin));
    }
    UpdateVisibility(StringToVisibleType(json->GetString("visibility")));
    UpdateLayoutDirection(StringToTextDirection(json->GetString("direction")));
}

const std::string LayoutProperty::PixelRoundToJsonValue() const
{
    auto res = JsonUtil::Create(true);
    if (pixelRoundFlag_ & static_cast<uint8_t>(PixelRoundPolicy::FORCE_CEIL_START)) {
        res->Put("start", "PixelRoundCalcPolicy.FORCE_CEIL");
    } else if (pixelRoundFlag_ & static_cast<uint8_t>(PixelRoundPolicy::FORCE_FLOOR_START)) {
        res->Put("start", "PixelRoundCalcPolicy.FORCE_FLOOR");
    } else {
        res->Put("start", "PixelRoundCalcPolicy.NO_FORCE_ROUND");
    }
    if (pixelRoundFlag_ & static_cast<uint8_t>(PixelRoundPolicy::FORCE_CEIL_TOP)) {
        res->Put("top", "PixelRoundCalcPolicy.FORCE_CEIL");
    } else if (pixelRoundFlag_ & static_cast<uint8_t>(PixelRoundPolicy::FORCE_FLOOR_TOP)) {
        res->Put("top", "PixelRoundCalcPolicy.FORCE_FLOOR");
    } else {
        res->Put("top", "PixelRoundCalcPolicy.NO_FORCE_ROUND");
    }
    if (pixelRoundFlag_ & static_cast<uint8_t>(PixelRoundPolicy::FORCE_CEIL_END)) {
        res->Put("end", "PixelRoundCalcPolicy.FORCE_CEIL");
    } else if (pixelRoundFlag_ & static_cast<uint8_t>(PixelRoundPolicy::FORCE_FLOOR_END)) {
        res->Put("end", "PixelRoundCalcPolicy.FORCE_FLOOR");
    } else {
        res->Put("end", "PixelRoundCalcPolicy.NO_FORCE_ROUND");
    }
    if (pixelRoundFlag_ & static_cast<uint8_t>(PixelRoundPolicy::FORCE_CEIL_BOTTOM)) {
        res->Put("bottom", "PixelRoundCalcPolicy.FORCE_CEIL");
    } else if (pixelRoundFlag_ & static_cast<uint8_t>(PixelRoundPolicy::FORCE_FLOOR_BOTTOM)) {
        res->Put("bottom", "PixelRoundCalcPolicy.FORCE_FLOOR");
    } else {
        res->Put("bottom", "PixelRoundCalcPolicy.NO_FORCE_ROUND");
    }
    return res->ToString();
}

RefPtr<LayoutProperty> LayoutProperty::Clone() const
{
    auto layoutProperty = MakeRefPtr<LayoutProperty>();
    Clone(layoutProperty);
    return layoutProperty;
}

void LayoutProperty::Clone(RefPtr<LayoutProperty> layoutProperty) const
{
    layoutProperty->UpdateLayoutProperty(this);
}

void LayoutProperty::UpdateLayoutProperty(const LayoutProperty* layoutProperty)
{
    layoutConstraint_ = layoutProperty->layoutConstraint_;
    if (layoutProperty->gridProperty_) {
        gridProperty_ = std::make_unique<GridProperty>(*layoutProperty->gridProperty_);
    }
    if (layoutProperty->calcLayoutConstraint_) {
        calcLayoutConstraint_ = std::make_unique<MeasureProperty>(*layoutProperty->calcLayoutConstraint_);
    }
    if (layoutProperty->padding_) {
        padding_ = std::make_unique<PaddingProperty>(*layoutProperty->padding_);
    }
    if (layoutProperty->margin_) {
        margin_ = std::make_unique<PaddingProperty>(*layoutProperty->margin_);
    }
    if (layoutProperty->borderWidth_) {
        borderWidth_ = std::make_unique<BorderWidthProperty>(*layoutProperty->borderWidth_);
    }
    magicItemProperty_ = layoutProperty->magicItemProperty_;
    if (layoutProperty->positionProperty_) {
        positionProperty_ = std::make_unique<PositionProperty>(*layoutProperty->positionProperty_);
    }
    if (layoutProperty->flexItemProperty_) {
        flexItemProperty_ = std::make_unique<FlexItemProperty>(*layoutProperty->flexItemProperty_);
    }
    if (layoutProperty->safeAreaInsets_) {
        safeAreaInsets_ = std::make_unique<SafeAreaInsets>(*layoutProperty->safeAreaInsets_);
    }
    if (layoutProperty->safeAreaExpandOpts_) {
        safeAreaExpandOpts_ = std::make_unique<SafeAreaExpandOpts>(*layoutProperty->safeAreaExpandOpts_);
    }
    geometryTransition_ = layoutProperty->geometryTransition_;
    propVisibility_ = layoutProperty->GetVisibility();
    measureType_ = layoutProperty->measureType_;
    layoutDirection_ = layoutProperty->layoutDirection_;
    propertyChangeFlag_ = layoutProperty->propertyChangeFlag_;
    propIsBindOverlay_ = layoutProperty->propIsBindOverlay_;
    isOverlayNode_ = layoutProperty->isOverlayNode_;
    overlayOffsetX_ = layoutProperty->overlayOffsetX_;
    overlayOffsetY_ = layoutProperty->overlayOffsetY_;
}

void LayoutProperty::UpdateCalcLayoutProperty(const MeasureProperty& constraint)
{
    if (!calcLayoutConstraint_) {
        calcLayoutConstraint_ = std::make_unique<MeasureProperty>(constraint);
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
        return;
    }
    if (*calcLayoutConstraint_ == constraint) {
        return;
    }
    calcLayoutConstraint_->selfIdealSize = constraint.selfIdealSize;
    calcLayoutConstraint_->maxSize = constraint.maxSize;
    calcLayoutConstraint_->minSize = constraint.minSize;
    propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
}

void LayoutProperty::UpdateLayoutConstraint(const LayoutConstraintF& parentConstraint)
{
    layoutConstraint_ = parentConstraint;
    if (margin_) {
        marginResult_.reset();
        auto margin = CreateMargin();
        if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            MinusPaddingToNonNegativeSize(margin, layoutConstraint_->maxSize);
            MinusPaddingToNonNegativeSize(margin, layoutConstraint_->minSize);
            MinusPaddingToNonNegativeSize(margin, layoutConstraint_->percentReference);
        } else {
            MinusPaddingToSize(margin, layoutConstraint_->maxSize);
            MinusPaddingToSize(margin, layoutConstraint_->minSize);
            MinusPaddingToSize(margin, layoutConstraint_->percentReference);
        }
        // already has non negative protection
        MinusPaddingToSize(margin, layoutConstraint_->selfIdealSize);
        MinusPaddingToSize(margin, layoutConstraint_->parentIdealSize);
    }
    auto originMax = layoutConstraint_->maxSize;
    if (calcLayoutConstraint_) {
        if (calcLayoutConstraint_->maxSize.has_value()) {
            layoutConstraint_->UpdateMaxSizeWithCheck(ConvertToSize(calcLayoutConstraint_->maxSize.value(),
                parentConstraint.scaleProperty, parentConstraint.percentReference));
        }
        if (calcLayoutConstraint_->minSize.has_value()) {
            layoutConstraint_->UpdateMinSizeWithCheck(ConvertToSize(calcLayoutConstraint_->minSize.value(),
                parentConstraint.scaleProperty, parentConstraint.percentReference));
        }
        if (calcLayoutConstraint_->selfIdealSize.has_value()) {
            layoutConstraint_->UpdateIllegalSelfIdealSizeWithCheck(
                ConvertToOptionalSize(calcLayoutConstraint_->selfIdealSize.value(), parentConstraint.scaleProperty,
                    parentConstraint.percentReference));
        }
    }

    CheckSelfIdealSize(parentConstraint, originMax);
    CheckBorderAndPadding();
    CheckAspectRatio();
}

void LayoutProperty::UpdateLayoutConstraintWithLayoutRect()
{
    CHECK_NULL_VOID(layoutRect_);
    auto size = layoutRect_.value().GetSize();
    layoutConstraint_ = {
        .scaleProperty = ScaleProperty::CreateScaleProperty(),
        .minSize = size,
        .maxSize = size,
        .percentReference = size,
        .selfIdealSize = OptionalSizeF(size),
    };
}

void LayoutProperty::CheckBorderAndPadding()
{
    auto selfWidth = layoutConstraint_->selfIdealSize.Width();
    auto selfHeight = layoutConstraint_->selfIdealSize.Height();
    if (!selfWidth && !selfHeight) {
        return;
    }
    auto selfWidthFloat = selfWidth.value_or(Infinity<float>());
    auto selfHeightFloat = selfHeight.value_or(Infinity<float>());
    auto paddingWithBorder = CreatePaddingAndBorder();
    auto deflateWidthF = paddingWithBorder.Width();
    auto deflateHeightF = paddingWithBorder.Height();
    if (LessOrEqual(deflateWidthF, selfWidthFloat) && LessOrEqual(deflateHeightF, selfHeightFloat)) {
        return;
    }
    if (GreatNotEqual(deflateWidthF, selfWidthFloat)) {
        layoutConstraint_->selfIdealSize.SetWidth(deflateWidthF);
    }
    if (GreatNotEqual(deflateHeightF, selfHeightFloat)) {
        layoutConstraint_->selfIdealSize.SetHeight(deflateHeightF);
    }
}

void LayoutProperty::CheckAspectRatio()
{
    if (!magicItemProperty_.HasAspectRatio()) {
        return;
    }
    auto aspectRatio = magicItemProperty_.GetAspectRatioValue();
    // Adjust by aspect ratio, firstly pick height based on width. It means that when width, height and aspectRatio are
    // all set, the height is not used.
    auto maxWidth = layoutConstraint_->maxSize.Width();
    auto maxHeight = layoutConstraint_->maxSize.Height();
    if (maxHeight > maxWidth / aspectRatio) {
        maxHeight = maxWidth / aspectRatio;
    }
    layoutConstraint_->maxSize.SetWidth(maxWidth);
    layoutConstraint_->maxSize.SetHeight(maxHeight);
    std::optional<float> selfWidth;
    std::optional<float> selfHeight;
    if (layoutConstraint_->selfIdealSize.Width()) {
        selfWidth = layoutConstraint_->selfIdealSize.Width().value();
        selfHeight = selfWidth.value() / aspectRatio;
        if (selfHeight > maxHeight) {
            selfHeight = maxHeight;
            selfWidth = selfHeight.value() * aspectRatio;
        }
    } else if (layoutConstraint_->selfIdealSize.Height()) {
        selfHeight = layoutConstraint_->selfIdealSize.Height().value();
        selfWidth = selfHeight.value() * aspectRatio;
        if (selfWidth > maxWidth) {
            selfWidth = maxWidth;
            selfHeight = selfWidth.value() / aspectRatio;
        }
    }

    if (selfHeight) {
        layoutConstraint_->selfIdealSize.SetHeight(selfHeight);
    }
    if (selfWidth) {
        layoutConstraint_->selfIdealSize.SetWidth(selfWidth);
    }
}

void LayoutProperty::BuildGridProperty(const RefPtr<FrameNode>& host)
{
    CHECK_NULL_VOID(gridProperty_);
    auto parent = host->GetAncestorNodeOfFrame();
    while (parent) {
        if (parent->GetTag() == V2::GRIDCONTAINER_ETS_TAG) {
            auto containerLayout = parent->GetLayoutProperty();
            gridProperty_->UpdateContainer(containerLayout, host);
            UpdateUserDefinedIdealSize(CalcSize(CalcLength(gridProperty_->GetWidth()), std::nullopt));
            break;
        }
        parent = parent->GetAncestorNodeOfFrame();
    }
}

void LayoutProperty::UpdateGridProperty(std::optional<int32_t> span, std::optional<int32_t> offset, GridSizeType type)
{
    if (!gridProperty_) {
        gridProperty_ = std::make_unique<GridProperty>();
    }

    bool isSpanUpdated = (span.has_value() && gridProperty_->UpdateSpan(span.value(), type));
    bool isOffsetUpdated = (offset.has_value() && gridProperty_->UpdateOffset(offset.value(), type));
    if (isSpanUpdated || isOffsetUpdated) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

bool LayoutProperty::UpdateGridOffset(const RefPtr<FrameNode>& host)
{
    CHECK_NULL_RETURN(gridProperty_, false);
    auto optOffset = gridProperty_->GetOffset();
    if (optOffset == UNDEFINED_DIMENSION) {
        return false;
    }

    RefPtr<FrameNode> parent = host->GetAncestorNodeOfFrame();
    if (!parent) {
        return false;
    }
    auto parentOffset = parent->GetOffsetRelativeToWindow();
    auto globalOffset = gridProperty_->GetContainerPosition();

    OffsetF offset(optOffset.ConvertToPx(), 0);
    offset = offset + globalOffset - parentOffset;
    const auto& geometryNode = host->GetGeometryNode();
    if (offset.GetX() == geometryNode->GetFrameOffset().GetX()) {
        return false;
    }
    offset.SetY(geometryNode->GetFrameOffset().GetY());
    geometryNode->SetFrameOffset(offset);
    return true;
}

void LayoutProperty::CheckSelfIdealSize(const LayoutConstraintF& parentConstraint, const SizeF& originMax)
{
    if (measureType_ == MeasureType::MATCH_PARENT) {
        layoutConstraint_->UpdateIllegalSelfIdealSizeWithCheck(layoutConstraint_->parentIdealSize);
    }
    if (!calcLayoutConstraint_) {
        return;
    }
    SizeF minSize(-1.0f, -1.0f);
    SizeF maxSize(-1.0f, -1.0f);
    if (calcLayoutConstraint_->maxSize.has_value()) {
        maxSize = ConvertToSize(calcLayoutConstraint_->maxSize.value(), layoutConstraint_->scaleProperty,
            layoutConstraint_->percentReference);
    }
    if (calcLayoutConstraint_->minSize.has_value()) {
        minSize = ConvertToSize(calcLayoutConstraint_->minSize.value(), layoutConstraint_->scaleProperty,
            layoutConstraint_->percentReference);
    }
    if (calcLayoutConstraint_->maxSize.has_value()) {
        layoutConstraint_->selfIdealSize.UpdateWidthWhenSmaller(maxSize);
        if (GreatNotEqual(maxSize.Width(), 0.0f) && GreatOrEqual(maxSize.Width(), minSize.Width())) {
            layoutConstraint_->UpdateMaxWidthWithCheck(maxSize);
        } else if (GreatNotEqual(maxSize.Width(), 0.0f) && LessNotEqual(maxSize.Width(), minSize.Width())) {
            layoutConstraint_->maxSize.SetWidth(minSize.Width());
        } else {
            layoutConstraint_->maxSize.SetWidth(originMax.Width());
        }
        layoutConstraint_->selfIdealSize.UpdateHeightWhenSmaller(maxSize);
        if (GreatNotEqual(maxSize.Height(), 0.0f) && GreatOrEqual(maxSize.Height(), minSize.Height())) {
            layoutConstraint_->UpdateMaxHeightWithCheck(maxSize);
        } else if (GreatNotEqual(maxSize.Height(), 0.0f) && LessNotEqual(maxSize.Height(), minSize.Height())) {
            layoutConstraint_->maxSize.SetHeight(minSize.Height());
        } else {
            layoutConstraint_->maxSize.SetHeight(originMax.Height());
        }
    }
    layoutConstraint_->UpdateMinSizeWithCheck(minSize);
    layoutConstraint_->selfIdealSize.UpdateSizeWhenLarger(minSize);
}

LayoutConstraintF LayoutProperty::CreateChildConstraint() const
{
    CHECK_NULL_RETURN(layoutConstraint_, {});
    auto layoutConstraint = contentConstraint_.value();
    layoutConstraint.parentIdealSize = layoutConstraint.selfIdealSize;
    // update max size when ideal size has value.
    if (layoutConstraint.parentIdealSize.Width()) {
        layoutConstraint.maxSize.SetWidth(layoutConstraint.parentIdealSize.Width().value());
        layoutConstraint.percentReference.SetWidth(layoutConstraint.parentIdealSize.Width().value());
    }
    if (layoutConstraint.parentIdealSize.Height()) {
        layoutConstraint.maxSize.SetHeight(layoutConstraint.parentIdealSize.Height().value());
        layoutConstraint.percentReference.SetHeight(layoutConstraint.parentIdealSize.Height().value());
    }
    // for child constraint, reset current selfIdealSize and minSize.
    layoutConstraint.selfIdealSize.Reset();
    layoutConstraint.minSize.Reset();
    return layoutConstraint;
}

void LayoutProperty::UpdateContentConstraint()
{
    CHECK_NULL_VOID(layoutConstraint_);
    contentConstraint_ = layoutConstraint_.value();
    // update percent reference when parent has size.
    if (contentConstraint_->parentIdealSize.Width()) {
        contentConstraint_->percentReference.SetWidth(contentConstraint_->parentIdealSize.Width().value());
    }
    if (contentConstraint_->parentIdealSize.Height()) {
        contentConstraint_->percentReference.SetHeight(contentConstraint_->parentIdealSize.Height().value());
    }
    if (padding_) {
        auto paddingF = ConvertToPaddingPropertyF(
            *padding_, contentConstraint_->scaleProperty, contentConstraint_->percentReference.Width());
        if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            contentConstraint_->MinusPaddingToNonNegativeSize(
                paddingF.left, paddingF.right, paddingF.top, paddingF.bottom);
        } else {
            contentConstraint_->MinusPadding(paddingF.left, paddingF.right, paddingF.top, paddingF.bottom);
        }
    }
    if (borderWidth_) {
        auto borderWidthF = ConvertToBorderWidthPropertyF(
            *borderWidth_, contentConstraint_->scaleProperty, contentConstraint_->percentReference.Width());
        if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            contentConstraint_->MinusPaddingToNonNegativeSize(
                borderWidthF.leftDimen, borderWidthF.rightDimen, borderWidthF.topDimen, borderWidthF.bottomDimen);
        } else {
            contentConstraint_->MinusPadding(
                borderWidthF.leftDimen, borderWidthF.rightDimen, borderWidthF.topDimen, borderWidthF.bottomDimen);
        }
    }
}

PaddingPropertyF LayoutProperty::CreatePaddingAndBorder()
{
    if (layoutConstraint_.has_value()) {
        auto padding = ConvertToPaddingPropertyF(
            padding_, ScaleProperty::CreateScaleProperty(), layoutConstraint_->percentReference.Width());
        auto borderWidth = ConvertToBorderWidthPropertyF(
            borderWidth_, ScaleProperty::CreateScaleProperty(), layoutConstraint_->percentReference.Width());

        return PaddingPropertyF { padding.left.value_or(0) + borderWidth.leftDimen.value_or(0),
            padding.right.value_or(0) + borderWidth.rightDimen.value_or(0),
            padding.top.value_or(0) + borderWidth.topDimen.value_or(0),
            padding.bottom.value_or(0) + borderWidth.bottomDimen.value_or(0) };
    }
    auto padding = ConvertToPaddingPropertyF(
        padding_, ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth());
    auto borderWidth = ConvertToBorderWidthPropertyF(
        borderWidth_, ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth());

    return PaddingPropertyF { padding.left.value_or(0) + borderWidth.leftDimen.value_or(0),
        padding.right.value_or(0) + borderWidth.rightDimen.value_or(0),
        padding.top.value_or(0) + borderWidth.topDimen.value_or(0),
        padding.bottom.value_or(0) + borderWidth.bottomDimen.value_or(0) };
}

PaddingPropertyF LayoutProperty::CreatePaddingAndBorderWithDefault(float paddingHorizontalDefault,
    float paddingVerticalDefault, float borderHorizontalDefault, float borderVerticalDefault)
{
    if (layoutConstraint_.has_value()) {
        auto padding = ConvertToPaddingPropertyF(
            padding_, ScaleProperty::CreateScaleProperty(), layoutConstraint_->percentReference.Width());
        auto borderWidth = ConvertToBorderWidthPropertyF(
            borderWidth_, ScaleProperty::CreateScaleProperty(), layoutConstraint_->percentReference.Width());
        return PaddingPropertyF { padding.left.value_or(paddingHorizontalDefault) +
                                      borderWidth.leftDimen.value_or(borderHorizontalDefault),
            padding.right.value_or(paddingHorizontalDefault) + borderWidth.rightDimen.value_or(borderHorizontalDefault),
            padding.top.value_or(paddingVerticalDefault) + borderWidth.topDimen.value_or(borderVerticalDefault),
            padding.bottom.value_or(paddingVerticalDefault) + borderWidth.bottomDimen.value_or(borderVerticalDefault) };
    }
    auto padding = ConvertToPaddingPropertyF(
        padding_, ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth());
    auto borderWidth = ConvertToBorderWidthPropertyF(
        borderWidth_, ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth());

    return PaddingPropertyF { padding.left.value_or(paddingHorizontalDefault) +
                                  borderWidth.leftDimen.value_or(borderHorizontalDefault),
        padding.right.value_or(paddingHorizontalDefault) + borderWidth.rightDimen.value_or(borderHorizontalDefault),
        padding.top.value_or(paddingVerticalDefault) + borderWidth.topDimen.value_or(borderVerticalDefault),
        padding.bottom.value_or(paddingVerticalDefault) + borderWidth.bottomDimen.value_or(borderVerticalDefault) };
}

PaddingPropertyF LayoutProperty::CreatePaddingWithoutBorder(bool useRootConstraint, bool roundPixel)
{
    if (layoutConstraint_.has_value()) {
        return ConvertToPaddingPropertyF(
            padding_, layoutConstraint_->scaleProperty, layoutConstraint_->percentReference.Width(), roundPixel);
    }

    return ConvertToPaddingPropertyF(padding_, ScaleProperty::CreateScaleProperty(),
        useRootConstraint ? PipelineContext::GetCurrentRootWidth() : 0.0f, roundPixel);
}

BorderWidthPropertyF LayoutProperty::CreateBorder()
{
    // no pixel rounding
    if (layoutConstraint_.has_value()) {
        return ConvertToBorderWidthPropertyF(
            borderWidth_, layoutConstraint_->scaleProperty, layoutConstraint_->percentReference.Width(), false);
    }

    return ConvertToBorderWidthPropertyF(
        borderWidth_, ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth(), false);
}

MarginPropertyF LayoutProperty::CreateMargin()
{
    CHECK_NULL_RETURN(margin_, MarginPropertyF());
    if (!marginResult_.has_value() && margin_) {
        if (layoutConstraint_.has_value()) {
            marginResult_ = ConvertToMarginPropertyF(
                margin_, layoutConstraint_->scaleProperty, layoutConstraint_->percentReference.Width());
        } else {
            // root node
            marginResult_ = ConvertToMarginPropertyF(
                margin_, ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth());
        }
    }
    return marginResult_.value_or(MarginPropertyF());
}

MarginPropertyF LayoutProperty::CreateMarginWithoutCache()
{
    CHECK_NULL_RETURN(margin_, MarginPropertyF());
    auto host = GetHost();
    CHECK_NULL_RETURN(host, MarginPropertyF());
    const auto& parentConstraint = host->GetGeometryNode()->GetParentLayoutConstraint();
    // no pixel rounding
    if (parentConstraint) {
        return ConvertToMarginPropertyF(
            margin_, parentConstraint->scaleProperty, parentConstraint->percentReference.Width(), false);
    }
    // the root width is not considered at present.
    return ConvertToMarginPropertyF(margin_, ScaleProperty::CreateScaleProperty(), 0.0f, false);
}

void LayoutProperty::SetHost(const WeakPtr<FrameNode>& host)
{
    host_ = host;
}

RefPtr<FrameNode> LayoutProperty::GetHost() const
{
    return host_.Upgrade();
}

void LayoutProperty::OnVisibilityUpdate(VisibleType visible, bool allowTransition)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    // store the previous visibility value.
    auto preVisibility = propVisibility_;

    // update visibility value.
    propVisibility_ = visible;
    host->NotifyVisibleChange(visible == VisibleType::VISIBLE);
    if (allowTransition && preVisibility) {
        if (preVisibility.value() == VisibleType::VISIBLE && visible != VisibleType::VISIBLE) {
            host->GetRenderContext()->OnNodeDisappear(false);
        } else if (preVisibility.value() != VisibleType::VISIBLE && visible == VisibleType::VISIBLE) {
            host->GetRenderContext()->OnNodeAppear(false);
        }
    }

    auto parent = host->GetAncestorNodeOfFrame();
    CHECK_NULL_VOID(parent);
    // if visible is not changed to/from VisibleType::Gone, only need to update render tree.
    if (preVisibility.value_or(VisibleType::VISIBLE) != VisibleType::GONE && visible != VisibleType::GONE) {
        parent->MarkNeedSyncRenderTree();
        parent->RebuildRenderContextTree();
        return;
    }
    UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    parent->MarkNeedSyncRenderTree();
    parent->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void LayoutProperty::UpdateSafeAreaExpandOpts(const SafeAreaExpandOpts& opts)
{
    if (!safeAreaExpandOpts_) {
        safeAreaExpandOpts_ = std::make_unique<SafeAreaExpandOpts>();
    }
    if (*safeAreaExpandOpts_ != opts) {
        *safeAreaExpandOpts_ = opts;
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_LAYOUT | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::UpdateSafeAreaInsets(const SafeAreaInsets& safeArea)
{
    if (!safeAreaInsets_) {
        safeAreaInsets_ = std::make_unique<SafeAreaInsets>();
    }
    if (*safeAreaInsets_ != safeArea) {
        *safeAreaInsets_ = safeArea;
    }
}

bool LayoutProperty::HasFixedWidth() const
{
    CHECK_NULL_RETURN(calcLayoutConstraint_, false);
    auto&& idealSize = calcLayoutConstraint_->selfIdealSize;
    return (idealSize && idealSize->WidthFixed());
}

bool LayoutProperty::HasFixedHeight() const
{
    CHECK_NULL_RETURN(calcLayoutConstraint_, false);
    auto&& idealSize = calcLayoutConstraint_->selfIdealSize;
    return (idealSize && idealSize->HeightFixed());
}

bool LayoutProperty::HasAspectRatio() const
{
    return magicItemProperty_.HasAspectRatio();
}

float LayoutProperty::GetAspectRatio() const
{
    if (magicItemProperty_.HasAspectRatio()) {
        return magicItemProperty_.GetAspectRatioValue();
    }
    return 0.0f;
}

void LayoutProperty::UpdateAspectRatio(float ratio)
{
    if (magicItemProperty_.UpdateAspectRatio(ratio)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::ResetAspectRatio()
{
    if (magicItemProperty_.HasAspectRatio()) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
        magicItemProperty_.ResetAspectRatio();
    }
}

void LayoutProperty::UpdateGeometryTransition(const std::string& id, bool followWithoutTransition)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    auto geometryTransitionOld = GetGeometryTransition();
    auto geometryTransitionNew =
        ElementRegister::GetInstance()->GetOrCreateGeometryTransition(id, followWithoutTransition);
    CHECK_NULL_VOID(geometryTransitionOld != geometryTransitionNew);
    if (geometryTransitionOld) {
        if (geometryTransitionOld->Update(host_, host_)) {
            geometryTransitionOld->OnFollowWithoutTransition();
        }
        // unregister node from old geometry transition
        geometryTransitionOld->Update(host_, nullptr);
        // register node into new geometry transition
        if (geometryTransitionNew) {
            geometryTransitionNew->Update(nullptr, host_);
        }
    } else if (geometryTransitionNew) {
        geometryTransitionNew->Build(host_, true);
    }
    geometryTransition_ = geometryTransitionNew;

    TAG_LOGD(AceLogTag::ACE_GEOMETRY_TRANSITION, "node: %{public}d update id, old id: %{public}s, new id: %{public}s",
        host->GetId(), geometryTransitionOld ? geometryTransitionOld->GetId().c_str() : "empty",
        geometryTransitionNew ? id.c_str() : "empty");
    propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_LAYOUT | PROPERTY_UPDATE_MEASURE;
}

void LayoutProperty::ResetGeometryTransition()
{
    if (!GetGeometryTransition()) {
        return;
    }
    UpdateGeometryTransition("");
}

void LayoutProperty::UpdateLayoutDirection(TextDirection value)
{
    if (layoutDirection_ == value) {
        return;
    }
    layoutDirection_ = value;
    propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
}

TextDirection LayoutProperty::GetNonAutoLayoutDirection() const
{
    auto direction = layoutDirection_.value_or(TextDirection::AUTO);
    return direction != TextDirection::AUTO
               ? direction
               : (AceApplicationInfo::GetInstance().IsRightToLeft() ? TextDirection::RTL : TextDirection::LTR);
}

void LayoutProperty::UpdateLayoutWeight(float value)
{
    if (magicItemProperty_.UpdateLayoutWeight(value)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::UpdateBorderWidth(const BorderWidthProperty& value)
{
    if (!borderWidth_) {
        borderWidth_ = std::make_unique<BorderWidthProperty>();
    }
    if (borderWidth_->UpdateWithCheck(value)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_LAYOUT | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::UpdateOuterBorderWidth(const BorderWidthProperty& value)
{
    if (!outerBorderWidth_) {
        outerBorderWidth_ = std::make_unique<BorderWidthProperty>();
    }
    if (outerBorderWidth_->UpdateWithCheck(value)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_LAYOUT | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::UpdateAlignment(Alignment value)
{
    if (!positionProperty_) {
        positionProperty_ = std::make_unique<PositionProperty>();
    }
    if (positionProperty_->UpdateAlignment(value)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_LAYOUT;
    }
}

void LayoutProperty::UpdateMargin(const MarginProperty& value)
{
    if (!margin_) {
        margin_ = std::make_unique<MarginProperty>();
    }
    if (margin_->UpdateWithCheck(value)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_LAYOUT | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::UpdatePadding(const PaddingProperty& value)
{
    if (!padding_) {
        padding_ = std::make_unique<PaddingProperty>();
    }
    if (padding_->UpdateWithCheck(value)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_LAYOUT | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::UpdateUserDefinedIdealSize(const CalcSize& value)
{
    if (!calcLayoutConstraint_) {
        calcLayoutConstraint_ = std::make_unique<MeasureProperty>();
    }
    if (calcLayoutConstraint_->UpdateSelfIdealSizeWithCheck(value)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::ClearUserDefinedIdealSize(bool clearWidth, bool clearHeight)
{
    if (!calcLayoutConstraint_) {
        return;
    }
    if (calcLayoutConstraint_->ClearSelfIdealSize(clearWidth, clearHeight)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::UpdateCalcMinSize(const CalcSize& value)
{
    if (!calcLayoutConstraint_) {
        calcLayoutConstraint_ = std::make_unique<MeasureProperty>();
    }
    if (calcLayoutConstraint_->UpdateMinSizeWithCheck(value)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::UpdateCalcMaxSize(const CalcSize& value)
{
    if (!calcLayoutConstraint_) {
        calcLayoutConstraint_ = std::make_unique<MeasureProperty>();
    }
    if (calcLayoutConstraint_->UpdateMaxSizeWithCheck(value)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::UpdateMarginSelfIdealSize(const SizeF& value)
{
    if (!layoutConstraint_.has_value()) {
        layoutConstraint_ = LayoutConstraintF();
    }
    if (layoutConstraint_->UpdateSelfMarginSizeWithCheck(OptionalSizeF(value))) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::ResetCalcMinSize()
{
    if (!calcLayoutConstraint_) {
        return;
    }
    if (calcLayoutConstraint_->minSize.has_value()) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
    calcLayoutConstraint_->minSize.reset();
}

void LayoutProperty::ResetCalcMaxSize()
{
    if (!calcLayoutConstraint_) {
        return;
    }
    if (calcLayoutConstraint_->maxSize.has_value()) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
    calcLayoutConstraint_->maxSize.reset();
}

void LayoutProperty::ResetCalcMinSize(bool resetWidth)
{
    if (!calcLayoutConstraint_) {
        return;
    }
    CHECK_NULL_VOID(calcLayoutConstraint_->minSize.has_value());
    bool resetSizeHasValue = resetWidth ? calcLayoutConstraint_->minSize.value().Width().has_value()
                                        : calcLayoutConstraint_->minSize.value().Height().has_value();
    CHECK_NULL_VOID(resetSizeHasValue);
    propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    if (resetWidth) {
        calcLayoutConstraint_->minSize.value().SetWidth(std::nullopt);
    } else {
        calcLayoutConstraint_->minSize.value().SetHeight(std::nullopt);
    }
}

void LayoutProperty::ResetCalcMaxSize(bool resetWidth)
{
    if (!calcLayoutConstraint_) {
        return;
    }
    CHECK_NULL_VOID(calcLayoutConstraint_->maxSize.has_value());
    bool resetSizeHasValue = resetWidth ? calcLayoutConstraint_->maxSize.value().Width().has_value()
                                        : calcLayoutConstraint_->maxSize.value().Height().has_value();
    CHECK_NULL_VOID(resetSizeHasValue);
    propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    if (resetWidth) {
        calcLayoutConstraint_->maxSize.value().SetWidth(std::nullopt);
    } else {
        calcLayoutConstraint_->maxSize.value().SetHeight(std::nullopt);
    }
}

void LayoutProperty::UpdateFlexGrow(float flexGrow)
{
    if (!flexItemProperty_) {
        flexItemProperty_ = std::make_unique<FlexItemProperty>();
    }
    if (flexItemProperty_->UpdateFlexGrow(flexGrow)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::ResetFlexGrow()
{
    if (!flexItemProperty_) {
        return;
    }
    if (flexItemProperty_->HasFlexGrow()) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
    flexItemProperty_->ResetFlexGrow();
}

void LayoutProperty::UpdateFlexShrink(float flexShrink)
{
    if (!flexItemProperty_) {
        flexItemProperty_ = std::make_unique<FlexItemProperty>();
    }
    if (flexItemProperty_->UpdateFlexShrink(flexShrink)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::ResetFlexShrink()
{
    if (!flexItemProperty_) {
        return;
    }
    if (flexItemProperty_->HasFlexShrink()) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
    flexItemProperty_->ResetFlexShrink();
}

void LayoutProperty::UpdateFlexBasis(const Dimension& flexBasis)
{
    if (!flexItemProperty_) {
        flexItemProperty_ = std::make_unique<FlexItemProperty>();
    }
    if (flexItemProperty_->UpdateFlexBasis(flexBasis)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::UpdateAlignSelf(const FlexAlign& flexAlign)
{
    if (!flexItemProperty_) {
        flexItemProperty_ = std::make_unique<FlexItemProperty>();
    }
    if (flexItemProperty_->UpdateAlignSelf(flexAlign)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::ResetAlignSelf()
{
    if (!flexItemProperty_) {
        return;
    }
    if (flexItemProperty_->HasAlignSelf()) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
    flexItemProperty_->ResetAlignSelf();
}

void LayoutProperty::UpdateAlignRules(const std::map<AlignDirection, AlignRule>& alignRules)
{
    if (!flexItemProperty_) {
        flexItemProperty_ = std::make_unique<FlexItemProperty>();
    }
    if (flexItemProperty_->UpdateAlignRules(alignRules)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::UpdateChainStyle(const ChainInfo& chainInfo)
{
    if (!flexItemProperty_) {
        flexItemProperty_ = std::make_unique<FlexItemProperty>();
    }
    if (!chainInfo.direction.has_value()) {
        ChainInfo nullChainInfo;
        if (flexItemProperty_->UpdateHorizontalChainStyle(nullChainInfo)) {
            propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
        }
        if (flexItemProperty_->UpdateVerticalChainStyle(nullChainInfo)) {
            propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
        }
    }
    if (chainInfo.direction == LineDirection::HORIZONTAL) {
        if (flexItemProperty_->UpdateHorizontalChainStyle(chainInfo)) {
            propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
        }
    } else {
        if (flexItemProperty_->UpdateVerticalChainStyle(chainInfo)) {
            propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
        }
    }
}

void LayoutProperty::UpdateBias(const BiasPair& biasPair)
{
    if (!flexItemProperty_) {
        flexItemProperty_ = std::make_unique<FlexItemProperty>();
    }
    if (flexItemProperty_->UpdateBias(biasPair)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

void LayoutProperty::UpdateDisplayIndex(int32_t displayIndex)
{
    if (!flexItemProperty_) {
        flexItemProperty_ = std::make_unique<FlexItemProperty>();
    }
    if (flexItemProperty_->UpdateDisplayIndex(displayIndex)) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

LayoutConstraintF LayoutProperty::CreateContentConstraint() const
{
    auto layoutConstraint = contentConstraint_.value_or(LayoutConstraintF());
    layoutConstraint.maxSize.UpdateSizeWhenSmaller(layoutConstraint.selfIdealSize.ConvertToSizeT());
    return layoutConstraint;
}

void LayoutProperty::UpdateLayoutConstraint(const RefPtr<LayoutProperty>& layoutProperty)
{
    layoutConstraint_ = layoutProperty->layoutConstraint_;
    contentConstraint_ = layoutProperty->contentConstraint_;
    gridProperty_ =
        (layoutProperty->gridProperty_) ? std::make_unique<GridProperty>(*layoutProperty->gridProperty_) : nullptr;
}

void LayoutProperty::UpdateVisibility(const VisibleType& value, bool allowTransition)
{
    if (propVisibility_.has_value()) {
        if (NearEqual(propVisibility_.value(), value)) {
            return;
        }
    }
    OnVisibilityUpdate(value, allowTransition);
}

void LayoutProperty::SetOverlayOffset(
    const std::optional<Dimension>& overlayOffsetX, const std::optional<Dimension>& overlayOffsetY)
{
    bool xChanged = true;
    bool yChanged = false;
    if ((!overlayOffsetX.has_value() && overlayOffsetX_.Value() == 0) ||
        (overlayOffsetX.has_value() && overlayOffsetX.value() == overlayOffsetX_)) {
        xChanged = false;
    }

    if ((!overlayOffsetY.has_value() && overlayOffsetY_.Value() == 0) ||
        (overlayOffsetY.has_value() && overlayOffsetY.value() == overlayOffsetY_)) {
        yChanged = false;
    }

    if (!xChanged && !yChanged) {
        return;
    }

    if (overlayOffsetX.has_value()) {
        overlayOffsetX_ = overlayOffsetX.value();
    } else {
        overlayOffsetX_.Reset();
    }

    if (overlayOffsetY.has_value()) {
        overlayOffsetY_ = overlayOffsetY.value();
    } else {
        overlayOffsetY_.Reset();
    }

    propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_LAYOUT | PROPERTY_UPDATE_MEASURE;
}

void LayoutProperty::GetOverlayOffset(Dimension& overlayOffsetX, Dimension& overlayOffsetY)
{
    overlayOffsetX = overlayOffsetX_;
    overlayOffsetY = overlayOffsetY_;
}

void LayoutProperty::UpdateAllGeometryTransition(const RefPtr<UINode>& parent)
{
    std::queue<RefPtr<UINode>> q;
    q.push(parent);
    while (!q.empty()) {
        auto node = q.front();
        q.pop();
        auto frameNode = AceType::DynamicCast<FrameNode>(node);
        if (frameNode) {
            auto layoutProperty = frameNode->GetLayoutProperty();
            if (layoutProperty && layoutProperty->GetGeometryTransition()) {
                auto geometryTransitionId = layoutProperty->GetGeometryTransition()->GetId();
                layoutProperty->UpdateGeometryTransition("");
                layoutProperty->UpdateGeometryTransition(geometryTransitionId);
            }
        }
        const auto& children = node->GetChildren();
        for (const auto& child : children) {
            q.push(child);
        }
    }
}

std::pair<bool, bool> LayoutProperty::GetPercentSensitive()
{
    if (!contentConstraint_.has_value()) {
        return { false, false };
    }
    std::pair<bool, bool> res = { false, false };
    const auto& constraint = contentConstraint_.value();
    if (GreaterOrEqualToInfinity(constraint.maxSize.Height())) {
        if (calcLayoutConstraint_ && calcLayoutConstraint_->PercentHeight()) {
            res.second = true;
        }
    }
    if (GreaterOrEqualToInfinity(constraint.maxSize.Width())) {
        if (calcLayoutConstraint_ && calcLayoutConstraint_->PercentWidth()) {
            res.first = true;
        }
    }
    return res;
}

std::pair<bool, bool> LayoutProperty::UpdatePercentSensitive(bool width, bool height)
{
    if (!contentConstraint_.has_value()) {
        return { false, false };
    }
    const auto& constraint = contentConstraint_.value();
    if (GreaterOrEqualToInfinity(constraint.maxSize.Height())) {
        heightPercentSensitive_ = heightPercentSensitive_ || height;
    }
    if (GreaterOrEqualToInfinity(constraint.maxSize.Width())) {
        widthPercentSensitive_ = heightPercentSensitive_ || width;
    }
    return { widthPercentSensitive_, heightPercentSensitive_ };
}

bool LayoutProperty::ConstraintEqual(const std::optional<LayoutConstraintF>& preLayoutConstraint,
    const std::optional<LayoutConstraintF>& preContentConstraint)
{
    if (!preLayoutConstraint || !layoutConstraint_) {
        return false;
    }
    if (!preContentConstraint || !contentConstraint_) {
        return false;
    }
    bool isNeedPercent = false;
    auto host = GetHost();
    if (host) {
        auto pattern = host->GetPattern();
        isNeedPercent = pattern ? pattern->IsNeedPercent() : false;
    }
    const auto& layout = layoutConstraint_.value();
    const auto& content = contentConstraint_.value();
    if (!isNeedPercent && GreaterOrEqualToInfinity(layout.maxSize.Width()) && !widthPercentSensitive_) {
        return (layout.EqualWithoutPercentWidth(preLayoutConstraint.value()) &&
                content.EqualWithoutPercentWidth(preContentConstraint.value()));
    }
    if (!isNeedPercent && GreaterOrEqualToInfinity(layout.maxSize.Height()) && !heightPercentSensitive_) {
        return (layout.EqualWithoutPercentHeight(preLayoutConstraint.value()) &&
                content.EqualWithoutPercentHeight(preContentConstraint.value()));
    }
    return (preLayoutConstraint == layoutConstraint_ && preContentConstraint == contentConstraint_);
}
} // namespace OHOS::Ace::NG
