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

#include "core/components_ng/pattern/search/search_layout_algorithm.h"

#include <algorithm>

#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_layout_algorithm.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t TEXTFIELD_INDEX = 0;
constexpr int32_t IMAGE_INDEX = 1;
constexpr int32_t CANCEL_IMAGE_INDEX = 2;
constexpr int32_t CANCEL_BUTTON_INDEX = 3;
constexpr int32_t BUTTON_INDEX = 4;
constexpr int32_t MULTIPLE_2 = 2;
constexpr float MAX_SEARCH_BUTTON_RATE = 0.4f;
constexpr float AGING_MIN_SCALE = 1.75f;
constexpr int TWO = 2;
} // namespace

bool SearchLayoutAlgorithm::IsFixedHeightMode(LayoutWrapper* layoutWrapper)
{
    auto layoutProperty = AceType::DynamicCast<SearchLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, false);

    auto constraint = layoutProperty->GetLayoutConstraint();
    return constraint->selfIdealSize.Height().has_value();
}

void SearchLayoutAlgorithm::CancelImageMeasure(LayoutWrapper* layoutWrapper)
{
    auto layoutProperty = AceType::DynamicCast<SearchLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto cancelImageWrapper = layoutWrapper->GetOrCreateChildByIndex(CANCEL_IMAGE_INDEX);
    CHECK_NULL_VOID(cancelImageWrapper);
    auto cancelImageGeometryNode = cancelImageWrapper->GetGeometryNode();
    CHECK_NULL_VOID(cancelImageGeometryNode);
    auto imageLayoutProperty = cancelImageWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(imageLayoutProperty);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto searchTheme = pipeline->GetTheme<SearchTheme>();
    CHECK_NULL_VOID(searchTheme);
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto imageConstraint = imageLayoutProperty->GetLayoutConstraint();
    auto searchHeight = CalcSearchHeight(constraint.value(), layoutWrapper);
    auto defaultImageHeight = static_cast<float>(searchTheme->GetIconSize().ConvertToPx());
    auto iconStretchSize = (NearZero(defaultImageHeight) || !imageConstraint->maxSize.IsPositive()) &&
                           !layoutProperty->HasCancelButtonUDSize();
    auto imageHeight = static_cast<float>(std::min(layoutProperty->HasCancelButtonUDSize() ?
        layoutProperty->GetCancelButtonUDSizeValue().ConvertToPx() : defaultImageHeight,
        searchHeight));
    CalcSize imageCalcSize;
    if (iconStretchSize) {
        imageCalcSize.SetWidth(CalcLength(imageHeight));
    }
    imageCalcSize.SetHeight(CalcLength(imageHeight));
    imageLayoutProperty->UpdateUserDefinedIdealSize(imageCalcSize);
    auto childLayoutConstraint = layoutProperty->CreateChildConstraint();
    cancelImageWrapper->Measure(childLayoutConstraint);
    cancelIconSizeMeasure_ = cancelImageGeometryNode->GetFrameSize();
}

void SearchLayoutAlgorithm::CancelButtonMeasure(LayoutWrapper* layoutWrapper)
{
    auto cancelButtonWrapper = layoutWrapper->GetOrCreateChildByIndex(CANCEL_BUTTON_INDEX);
    CHECK_NULL_VOID(cancelButtonWrapper);
    auto layoutProperty = AceType::DynamicCast<SearchLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto cancelButtonLayoutProperty =
        AceType::DynamicCast<ButtonLayoutProperty>(cancelButtonWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(cancelButtonLayoutProperty);
    auto cancelButtonGeometryNode = cancelButtonWrapper->GetGeometryNode();
    CHECK_NULL_VOID(cancelButtonGeometryNode);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto searchTheme = pipeline->GetTheme<SearchTheme>();
    CHECK_NULL_VOID(searchTheme);

    // calculate theme space from cancel button to cancel image
    auto spaceHeight = searchTheme->GetHeight().ConvertToPx() - 2 * searchTheme->GetSearchButtonSpace().ConvertToPx() -
                       searchTheme->GetIconHeight().ConvertToPx();

    // calculate cancel button height
    auto cancelButtonHeight =
        layoutProperty->GetCancelButtonUDSizeValue(Dimension(cancelIconSizeMeasure_.Height())).ConvertToPx() +
        spaceHeight;
    CalcSize cancelButtonCalcSize((CalcLength(cancelButtonHeight)), CalcLength(cancelButtonHeight));
    cancelButtonLayoutProperty->UpdateUserDefinedIdealSize(cancelButtonCalcSize);

    auto childLayoutConstraint = layoutProperty->CreateChildConstraint();
    cancelButtonWrapper->Measure(childLayoutConstraint);
    cancelBtnSizeMeasure_ = cancelButtonGeometryNode->GetFrameSize();
}

void SearchLayoutAlgorithm::TextFieldMeasure(LayoutWrapper* layoutWrapper)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto searchTheme = pipeline->GetTheme<SearchTheme>();
    auto layoutProperty = AceType::DynamicCast<SearchLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto textFieldWrapper = layoutWrapper->GetOrCreateChildByIndex(TEXTFIELD_INDEX);
    CHECK_NULL_VOID(textFieldWrapper);
    auto cancelButtonWrapper = layoutWrapper->GetOrCreateChildByIndex(CANCEL_BUTTON_INDEX);
    CHECK_NULL_VOID(cancelButtonWrapper);
    auto textFieldGeometryNode = textFieldWrapper->GetGeometryNode();
    CHECK_NULL_VOID(textFieldGeometryNode);

    UpdateFontFeature(layoutWrapper);
    auto buttonWidth = searchButtonSizeMeasure_.Width();
    auto cancelButtonWidth = cancelBtnSizeMeasure_.Width();
    auto iconRenderWidth =
        layoutProperty->GetSearchIconUDSizeValue(Dimension(searchIconSizeMeasure_.Width())).ConvertToPx();
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto searchWidthMax = CalcSearchWidth(constraint.value(), layoutWrapper);

    auto searchWrapper = layoutWrapper->GetOrCreateChildByIndex(BUTTON_INDEX);
    auto searchButtonNode = searchWrapper->GetHostNode();
    auto searchButtonEvent = searchButtonNode->GetEventHub<ButtonEventHub>();
    auto padding = layoutProperty->CreatePaddingAndBorder();
    float leftPadding = padding.left.value_or(0.0f);
    float rightPadding = padding.right.value_or(0.0f);
    auto textFieldWidth = searchWidthMax - searchTheme->GetSearchIconLeftSpace().ConvertToPx() - iconRenderWidth -
                          searchTheme->GetSearchIconRightSpace().ConvertToPx() - leftPadding - rightPadding;
    if (!AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TEN)) {
        textFieldWidth = searchWidthMax - searchTheme->GetSearchIconLeftSpace().ConvertToPx() - iconRenderWidth -
                         searchTheme->GetSearchIconRightSpace().ConvertToPx();
    }
    if (searchButtonEvent->IsEnabled()) {
        textFieldWidth = textFieldWidth - buttonWidth - searchTheme->GetSearchDividerWidth().ConvertToPx() -
                         MULTIPLE_2 * searchTheme->GetDividerSideSpace().ConvertToPx();
    }

    auto style = layoutProperty->GetCancelButtonStyle().value_or(CancelButtonStyle::INPUT);
    if (style != CancelButtonStyle::INVISIBLE) {
        textFieldWidth = textFieldWidth - cancelButtonWidth;
    }
    auto textFieldHeight = CalcSearchHeight(constraint.value(), layoutWrapper);
    auto childLayoutConstraint = layoutProperty->CreateChildConstraint();
    childLayoutConstraint.selfIdealSize.SetWidth(textFieldWidth);
    if (LessNotEqual(pipeline->GetFontScale(), AGING_MIN_SCALE)) {
        SetTextFieldLayoutConstraintHeight(childLayoutConstraint, textFieldHeight, layoutWrapper);
    }
    textFieldWrapper->Measure(childLayoutConstraint);
    textFieldSizeMeasure_ = textFieldGeometryNode->GetFrameSize();
}

void SearchLayoutAlgorithm::UpdateFontFeature(LayoutWrapper* layoutWrapper)
{
    auto layoutProperty = AceType::DynamicCast<SearchLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto textFieldWrapper = layoutWrapper->GetOrCreateChildByIndex(TEXTFIELD_INDEX);
    CHECK_NULL_VOID(textFieldWrapper);

    auto textFieldLayoutProperty = AceType::DynamicCast<TextFieldLayoutProperty>(textFieldWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(textFieldLayoutProperty);
    if (layoutProperty->HasFontFeature()) {
        textFieldLayoutProperty->UpdateFontFeature(layoutProperty->GetFontFeature().value());
    }
}
void SearchLayoutAlgorithm::SetTextFieldLayoutConstraintHeight(LayoutConstraintF& contentConstraint,
    double textFieldHeight, LayoutWrapper* layoutWrapper)
{
    if (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
        auto textFieldWrapper = layoutWrapper->GetOrCreateChildByIndex(TEXTFIELD_INDEX);
        auto textFieldLayoutProperty =
            AceType::DynamicCast<TextFieldLayoutProperty>(textFieldWrapper->GetLayoutProperty());
        if ((textFieldLayoutProperty == nullptr) || (!textFieldLayoutProperty->HasLineHeight())) {
            contentConstraint.selfIdealSize.SetHeight(textFieldHeight);
        }
        return;
    }
    contentConstraint.selfIdealSize.SetHeight(textFieldHeight);
}

void SearchLayoutAlgorithm::ImageMeasure(LayoutWrapper* layoutWrapper)
{
    auto layoutProperty = AceType::DynamicCast<SearchLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto imageWrapper = layoutWrapper->GetOrCreateChildByIndex(IMAGE_INDEX);
    CHECK_NULL_VOID(imageWrapper);
    auto imageGeometryNode = imageWrapper->GetGeometryNode();
    CHECK_NULL_VOID(imageGeometryNode);
    auto imageLayoutProperty = imageWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(imageLayoutProperty);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto searchTheme = pipeline->GetTheme<SearchTheme>();
    CHECK_NULL_VOID(searchTheme);
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto imageConstraint = imageLayoutProperty->GetLayoutConstraint();
    auto searchHeight = CalcSearchHeight(constraint.value(), layoutWrapper);
    auto defaultImageHeight = searchTheme->GetIconSize().ConvertToPx();
    auto iconStretchSize = (NearZero(defaultImageHeight) || !imageConstraint->maxSize.IsPositive()) &&
        !layoutProperty->HasSearchIconUDSize();
    auto imageHeight = static_cast<float>(std::min(layoutProperty->HasSearchIconUDSize() ?
        layoutProperty->GetSearchIconUDSizeValue().ConvertToPx() : defaultImageHeight,
        searchHeight));
    CalcSize imageCalcSize;
    if (iconStretchSize) {
        imageCalcSize.SetWidth(CalcLength(imageHeight));
    }
    imageCalcSize.SetHeight(CalcLength(imageHeight));
    imageLayoutProperty->UpdateUserDefinedIdealSize(imageCalcSize);

    auto childLayoutConstraint = layoutProperty->CreateChildConstraint();
    imageWrapper->Measure(childLayoutConstraint);
    searchIconSizeMeasure_ = imageGeometryNode->GetFrameSize();
}

void SearchLayoutAlgorithm::SearchButtonMeasure(LayoutWrapper* layoutWrapper)
{
    auto buttonWrapper = layoutWrapper->GetOrCreateChildByIndex(BUTTON_INDEX);
    CHECK_NULL_VOID(buttonWrapper);
    auto buttonLayoutProperty = AceType::DynamicCast<ButtonLayoutProperty>(buttonWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(buttonLayoutProperty);
    auto layoutProperty = AceType::DynamicCast<SearchLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto buttonGeometryNode = buttonWrapper->GetGeometryNode();
    CHECK_NULL_VOID(buttonGeometryNode);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto searchTheme = pipeline->GetTheme<SearchTheme>();
    CHECK_NULL_VOID(searchTheme);

    // calculate theme space from search button to font
    auto spaceHeight = searchTheme->GetHeight().ConvertToPx() - 2 * searchTheme->GetSearchButtonSpace().ConvertToPx() -
                       searchTheme->GetFontSize().ConvertToPx();

    // calculate search button height
    auto defaultButtonHeight =
        searchTheme->GetHeight().ConvertToPx() - 2 * searchTheme->GetSearchButtonSpace().ConvertToPx();
    auto searchButtonHeight = std::max(defaultButtonHeight,
        layoutProperty->GetSearchButtonFontSizeValue(searchTheme->GetFontSize()).ConvertToPx() + spaceHeight);
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto searchHeight = CalcSearchHeight(constraint.value(), layoutWrapper);
    searchButtonHeight = std::min(searchButtonHeight, searchHeight - 0.0f);
    CalcSize searchButtonCalcSize;
    searchButtonCalcSize.SetHeight(CalcLength(searchButtonHeight));
    buttonLayoutProperty->UpdateUserDefinedIdealSize(searchButtonCalcSize);

    if (GreatOrEqual(pipeline->GetFontScale(), AGING_MIN_SCALE)) {
        buttonLayoutProperty->ClearUserDefinedIdealSize(false, true);
    }

    // searchButton Measure
    auto buttonLayoutConstraint = layoutProperty->CreateChildConstraint();
    buttonWrapper->Measure(buttonLayoutConstraint);

    // deal with pixel round
    auto pixelRound = static_cast<uint8_t>(PixelRoundPolicy::FORCE_FLOOR_TOP) |
                        static_cast<uint8_t>(PixelRoundPolicy::FORCE_CEIL_BOTTOM);
    buttonLayoutProperty->UpdatePixelRound(pixelRound);

    // compute searchButton width
    auto searchWidthMax = CalcSearchWidth(constraint.value(), layoutWrapper);
    double searchButtonWidth = searchWidthMax * MAX_SEARCH_BUTTON_RATE;
    double curSearchButtonWidth = buttonGeometryNode->GetFrameSize().Width();
    searchButtonWidth = std::min(searchButtonWidth, curSearchButtonWidth);
    buttonLayoutConstraint.selfIdealSize.SetWidth(searchButtonWidth);
    buttonWrapper->Measure(buttonLayoutConstraint);
    searchButtonSizeMeasure_ = buttonGeometryNode->GetFrameSize();
}

double SearchLayoutAlgorithm::CalcSearchAdaptHeight(LayoutWrapper* layoutWrapper)
{
    double searchHeightAdapt = 0;
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, 0);
    auto searchTheme = pipeline->GetTheme<SearchTheme>();
    CHECK_NULL_RETURN(searchTheme, 0);
    auto layoutProperty = AceType::DynamicCast<SearchLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, 0);
    auto searchBtnWrapper = layoutWrapper->GetOrCreateChildByIndex(BUTTON_INDEX);
    CHECK_NULL_RETURN(searchBtnWrapper, 0);
    auto cancelBtnLayoutWrapper = layoutWrapper->GetOrCreateChildByIndex(CANCEL_BUTTON_INDEX);
    CHECK_NULL_RETURN(cancelBtnLayoutWrapper, 0);

    // search button height
    auto buttonNode = searchBtnWrapper->GetHostNode();
    CHECK_NULL_RETURN(buttonNode, true);
    auto searchButtonEvent = buttonNode->GetEventHub<ButtonEventHub>();
    CHECK_NULL_RETURN(searchButtonEvent, true);
    auto searchButtonHeight = searchButtonSizeMeasure_.Height() + 2 * searchTheme->GetSearchButtonSpace().ConvertToPx();
    searchButtonHeight = (!searchButtonEvent->IsEnabled()) ? 0.0f : searchButtonHeight;

    // search icon height
    auto searchIconFrameHight = searchIconSizeMeasure_.Height();
    auto searchIconHeight = layoutProperty->GetSearchIconUDSizeValue(Dimension(searchIconFrameHight)).ConvertToPx();
    searchIconHeight += searchTheme->GetHeight().ConvertToPx() - searchTheme->GetIconHeight().ConvertToPx();

    // cancel button height
    auto cancelButtonNode = cancelBtnLayoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(cancelButtonNode, 0);
    auto cancelButtonEvent = cancelButtonNode->GetEventHub<ButtonEventHub>();
    CHECK_NULL_RETURN(cancelButtonEvent, 0);
    auto cancelBtnHight = cancelBtnSizeMeasure_.Height() + 2 * searchTheme->GetSearchButtonSpace().ConvertToPx();
    cancelBtnHight = (!cancelButtonEvent->IsEnabled()) ? 0.0f : cancelBtnHight;

    // textfield height
    auto padding = layoutProperty->CreatePaddingAndBorder();
    auto verticalPadding = padding.top.value_or(0.0f) + padding.bottom.value_or(0.0f);
    auto textfieldHeight = textFieldSizeMeasure_.Height() + verticalPadding;

    // calculate the highest
    searchHeightAdapt = std::max(searchIconHeight, searchButtonHeight);
    searchHeightAdapt = std::max(searchHeightAdapt, cancelBtnHight);
    searchHeightAdapt = std::max(searchHeightAdapt, static_cast<double>(textfieldHeight));

    return searchHeightAdapt;
}

void SearchLayoutAlgorithm::SelfMeasure(LayoutWrapper* layoutWrapper)
{
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto layoutProperty = AceType::DynamicCast<SearchLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto searchHeight = CalcSearchHeight(constraint.value(), layoutWrapper);
    // update search height
    constraint->selfIdealSize.SetHeight(searchHeight);
    auto searchWidth = CalcSearchWidth(constraint.value(), layoutWrapper);
    SizeF idealSize(searchWidth, searchHeight);
    if (GreaterOrEqualToInfinity(idealSize.Width()) || GreaterOrEqualToInfinity(idealSize.Height())) {
        geometryNode->SetFrameSize(SizeF());
        return;
    }

    // update search height
    geometryNode->SetFrameSize(idealSize);
    geometryNode->SetContentSize(idealSize);
}

double SearchLayoutAlgorithm::CalcSearchWidth(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    auto searchConstraint = contentConstraint;
    auto idealWidth = contentConstraint.selfIdealSize.Width().value_or(contentConstraint.maxSize.Width());
    auto idealHeight = contentConstraint.selfIdealSize.Height().value_or(contentConstraint.maxSize.Height());
    auto maxIdealSize = SizeF { idealWidth, idealHeight };
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
        auto frameIdealSize = maxIdealSize;
        auto finalSize = UpdateOptionSizeByCalcLayoutConstraint(static_cast<OptionalSize<float>>(frameIdealSize),
            layoutWrapper->GetLayoutProperty()->GetCalcLayoutConstraint(),
            layoutWrapper->GetLayoutProperty()->GetLayoutConstraint()->percentReference);
        finalSize.SetWidth(finalSize.Width().value_or(frameIdealSize.Width()));
        finalSize.SetHeight(finalSize.Height().value_or(frameIdealSize.Height()));
        maxIdealSize.UpdateSizeWhenSmaller(finalSize.ConvertToSizeT());
    }
    searchConstraint.maxSize = maxIdealSize;
    auto searchWidth = (searchConstraint.selfIdealSize.Width().has_value())
                ? std::min(searchConstraint.selfIdealSize.Width().value(), searchConstraint.maxSize.Width())
                : std::min(searchConstraint.percentReference.Width(), searchConstraint.maxSize.Width());

    const auto& calcLayoutConstraint = layoutWrapper->GetLayoutProperty()->GetCalcLayoutConstraint();
    CHECK_NULL_RETURN(calcLayoutConstraint, searchWidth);
    auto hasMinSize = calcLayoutConstraint->minSize->Width().has_value();
    auto hasMaxSize = calcLayoutConstraint->maxSize->Width().has_value();
    auto hasWidth = calcLayoutConstraint->selfIdealSize->Width().has_value();
    if (hasMinSize && ((hasMaxSize && searchConstraint.minSize.Width() >= searchConstraint.maxSize.Width())
        || (!hasMaxSize && !hasWidth))) {
        return searchConstraint.minSize.Width();
    }
    if (hasMinSize) {
        searchWidth = std::max(searchConstraint.minSize.Width(), static_cast<float>(searchWidth));
    }
    if (hasMaxSize) {
        searchWidth = std::min(searchConstraint.maxSize.Width(), static_cast<float>(searchWidth));
    }
    return searchWidth;
}

double SearchLayoutAlgorithm::CalcSearchHeight(
    const LayoutConstraintF& constraint, LayoutWrapper* layoutWrapper)
{
    auto layoutProperty = AceType::DynamicCast<SearchLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, 0.0);
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(host, 0.0);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, 0.0);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, 0.0);
    auto searchTheme = pipeline->GetTheme<SearchTheme>();
    CHECK_NULL_RETURN(searchTheme, 0.0);
    auto themeHeight = searchTheme->GetHeight().ConvertToPx();
    auto searchHeight =
        (constraint.selfIdealSize.Height().has_value()) ? constraint.selfIdealSize.Height().value() : themeHeight;
    auto padding = layoutProperty->CreatePaddingAndBorder();
    auto verticalPadding = padding.top.value_or(0.0f) + padding.bottom.value_or(0.0f);
    searchHeight = std::max(verticalPadding, static_cast<float>(searchHeight));
    auto searchHeightAdapt = searchHeight;
    if (!IsFixedHeightMode(layoutWrapper)) {
        searchHeightAdapt = std::max(searchHeightAdapt, CalcSearchAdaptHeight(layoutWrapper));
        renderContext->SetClipToBounds(false);
    } else {
        renderContext->SetClipToBounds(true);
    }

    const auto& calcLayoutConstraint = layoutWrapper->GetLayoutProperty()->GetCalcLayoutConstraint();
    CHECK_NULL_RETURN(calcLayoutConstraint, searchHeightAdapt);
    auto hasMinSize = calcLayoutConstraint->minSize.has_value() &&
        calcLayoutConstraint->minSize->Height().has_value();
    auto hasMaxSize = calcLayoutConstraint->maxSize.has_value() &&
        calcLayoutConstraint->maxSize->Height().has_value();
    auto hasHeight = calcLayoutConstraint->selfIdealSize.has_value() &&
        calcLayoutConstraint->selfIdealSize->Height().has_value();
    if (hasMinSize && ((hasMaxSize && constraint.minSize.Height() >= constraint.maxSize.Height())
        || (!hasMaxSize && !hasHeight))) {
        return constraint.minSize.Height();
    }
    if (hasMinSize) {
        searchHeightAdapt = std::max(constraint.minSize.Height(),
            static_cast<float>(searchHeightAdapt));
    }
    if (hasMaxSize) {
        searchHeightAdapt = std::min(constraint.maxSize.Height(),
            static_cast<float>(searchHeightAdapt));
    }
    return searchHeightAdapt;
}

void SearchLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto children = host->GetChildren();
    if (children.empty()) {
        return;
    }

    SearchButtonMeasure(layoutWrapper);
    ImageMeasure(layoutWrapper);
    CancelImageMeasure(layoutWrapper);
    CancelButtonMeasure(layoutWrapper);
    TextFieldMeasure(layoutWrapper);
    SelfMeasure(layoutWrapper);
}

void SearchLayoutAlgorithm::CalcChildrenHotZone(LayoutWrapper* layoutWrapper)
{
    // search info
    auto searchGeometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(searchGeometryNode);
    auto searchHeight = searchGeometryNode->GetFrameSize().Height();

    // cancel button info
    auto cancelButtonWrapper = layoutWrapper->GetOrCreateChildByIndex(CANCEL_BUTTON_INDEX);
    CHECK_NULL_VOID(cancelButtonWrapper);
    auto cancelButtonFrameNode = cancelButtonWrapper->GetHostNode();
    CHECK_NULL_VOID(cancelButtonFrameNode);
    auto cancelButtonGeometryNode = cancelButtonWrapper->GetGeometryNode();
    CHECK_NULL_VOID(cancelButtonGeometryNode);
    auto cancelButtonFrameSize = cancelButtonGeometryNode->GetFrameSize();
    auto cancelButtonWidth = cancelButtonFrameSize.Width();
    auto cancelButtonHeight = cancelButtonFrameSize.Height();

    // search button info
    auto searchButtonWrapper = layoutWrapper->GetOrCreateChildByIndex(BUTTON_INDEX);
    CHECK_NULL_VOID(searchButtonWrapper);
    auto searchButtonFrameNode = searchButtonWrapper->GetHostNode();
    CHECK_NULL_VOID(searchButtonFrameNode);
    auto searchButtonGeometryNode = searchButtonWrapper->GetGeometryNode();
    CHECK_NULL_VOID(searchButtonGeometryNode);
    auto searchButtonFrameSize = searchButtonGeometryNode->GetFrameSize();
    auto searchButtonWidth = searchButtonFrameSize.Width();
    auto searchButtonHeight = searchButtonFrameSize.Height();

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto searchTheme = pipeline->GetTheme<SearchTheme>();
    auto buttonSpace = searchTheme->GetSearchButtonSpace().ConvertToPx();
    // calculate cancel button hot zone
    cancelButtonFrameNode->RemoveLastHotZoneRect();
    DimensionRect cancelButtonHotZone;
    if (cancelButtonHeight > searchHeight) {
        cancelButtonHotZone.SetSize(DimensionSize(Dimension(cancelButtonWidth), Dimension(searchHeight)));
        double hotZoneOffsetY = (cancelButtonHeight - searchHeight) / 2;
        cancelButtonHotZone.SetOffset(DimensionOffset(Dimension(0), Dimension(hotZoneOffsetY)));
    } else {
        cancelButtonHotZone.SetSize(DimensionSize(
            Dimension(cancelButtonWidth + 2 * buttonSpace), Dimension(cancelButtonHeight + 2 * buttonSpace)));
        cancelButtonHotZone.SetOffset(
            DimensionOffset(Offset(static_cast<float>(-buttonSpace), static_cast<float>(-buttonSpace))));
    }
    cancelButtonFrameNode->AddHotZoneRect(cancelButtonHotZone);

    // calculate search button hot zone
    searchButtonFrameNode->RemoveLastHotZoneRect();
    DimensionRect searchButtonHotZone;
    if (searchButtonHeight > searchHeight) {
        searchButtonHotZone.SetSize(DimensionSize(Dimension(searchButtonWidth), Dimension(searchHeight)));
        double hotZoneOffsetY = (searchButtonHeight - searchHeight) / 2;
        searchButtonHotZone.SetOffset(DimensionOffset(Dimension(0), Dimension(hotZoneOffsetY)));
    } else {
        searchButtonHotZone.SetSize(DimensionSize(
            Dimension(searchButtonWidth + 2 * buttonSpace), Dimension(searchButtonHeight + 2 * buttonSpace)));
        searchButtonHotZone.SetOffset(
            DimensionOffset(Offset(static_cast<float>(-buttonSpace), static_cast<float>(-buttonSpace))));
    }
    searchButtonFrameNode->AddHotZoneRect(searchButtonHotZone);
}

void SearchLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto children = host->GetChildren();
    if (children.empty()) {
        return;
    }

    auto layoutProperty = DynamicCast<SearchLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto isRTL = layoutProperty->GetNonAutoLayoutDirection() == TextDirection::RTL;
    

    auto pipeline = host->GetContext();
    CHECK_NULL_VOID(pipeline);
    auto searchTheme = pipeline->GetTheme<SearchTheme>();

    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto searchSize = geometryNode->GetFrameSize();
    auto searchFrameWidth = searchSize.Width();
    auto searchFrameHeight = searchSize.Height();

    LayoutSearchParams params = {
        .layoutWrapper = layoutWrapper,
        .layoutProperty = layoutProperty,
        .searchTheme = searchTheme,
        .searchFrameWidth = searchFrameWidth,
        .searchFrameHeight = searchFrameHeight,
        .isRTL = isRTL
    };
    LayoutSearchIcon(params);
    LayoutSearchButton(params);
    LayoutCancelButton(params);
    LayoutCancelImage(params);
    LayoutTextField(params);

    CalcChildrenHotZone(layoutWrapper);
}

void SearchLayoutAlgorithm::LayoutSearchIcon(const LayoutSearchParams& params)
{
    auto searchIconLeftSpace = params.searchTheme->GetSearchIconLeftSpace().ConvertToPx();
    auto searchIconRightSpace = params.searchTheme->GetSearchIconRightSpace().ConvertToPx();

    auto imageWrapper = params.layoutWrapper->GetOrCreateChildByIndex(IMAGE_INDEX);
    CHECK_NULL_VOID(imageWrapper);
    auto imageGeometryNode = imageWrapper->GetGeometryNode();
    CHECK_NULL_VOID(imageGeometryNode);
    auto iconFrameWidth = searchIconSizeMeasure_.Width();
    auto iconFrameHeight = searchIconSizeMeasure_.Height();
    auto layoutProperty = params.layoutProperty;
    auto iconRenderWidth = layoutProperty->GetSearchIconUDSizeValue(Dimension(iconFrameWidth)).ConvertToPx();

    auto padding = layoutProperty->CreatePaddingAndBorder();
    float topPadding = padding.top.value_or(0.0f);
    auto bottomPadding = padding.bottom.value_or(0.0f);
    auto leftOffset = padding.left.value_or(0.0f);
    auto rightOffset = padding.right.value_or(0.0f);
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
        leftOffset = 0.0f;
    }
    float iconHorizontalOffset = params.isRTL ?
        params.searchFrameWidth - searchIconRightSpace - iconRenderWidth - rightOffset :
        leftOffset + searchIconLeftSpace + (iconRenderWidth - iconFrameWidth) / 2.0f;

    auto searchIconConstraint = imageWrapper->GetLayoutProperty()->GetLayoutConstraint();
    auto iconUserHeight =
        searchIconConstraint->selfIdealSize.Height().value_or(params.searchTheme->GetIconHeight().ConvertToPx());
    float imageVerticalOffset = topPadding;
    if (NearEqual(iconUserHeight, iconFrameHeight)) {
        float iconInterval = (params.searchFrameHeight - iconUserHeight) / 2;
        if (topPadding <= iconInterval && bottomPadding <= iconInterval) {
            imageVerticalOffset = iconInterval;
        } else if (topPadding <= iconInterval && bottomPadding > iconInterval) {
            imageVerticalOffset = params.searchFrameHeight - (bottomPadding + iconFrameHeight);
        }
    }
    OffsetF imageOffset(iconHorizontalOffset, imageVerticalOffset);
    imageGeometryNode->SetMarginFrameOffset(imageOffset);
    imageWrapper->Layout();
}

void SearchLayoutAlgorithm::LayoutSearchButton(const LayoutSearchParams& params)
{
    auto searchButtonSpace = params.searchTheme->GetSearchButtonSpace().ConvertToPx();

    auto searchButtonWrapper = params.layoutWrapper->GetOrCreateChildByIndex(BUTTON_INDEX);
    CHECK_NULL_VOID(searchButtonWrapper);
    auto searchButtonGeometryNode = searchButtonWrapper->GetGeometryNode();
    CHECK_NULL_VOID(searchButtonGeometryNode);
    auto searchButtonFrameSize = searchButtonGeometryNode->GetFrameSize();
    float searchButtonVerticalOffset = (params.searchFrameHeight - searchButtonFrameSize.Height()) / TWO;

    auto padding = params.layoutProperty->CreatePaddingAndBorder();
    auto leftOffset = padding.left.value_or(0.0f);
    auto rightOffset = padding.right.value_or(0.0f);
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
        rightOffset = 0.0f;
    }
    float searchButtonHorizontalOffset = 0.0f;
    if (params.isRTL) {
        searchButtonHorizontalOffset = leftOffset + searchButtonSpace;
    } else {
        searchButtonHorizontalOffset =
            params.searchFrameWidth - searchButtonFrameSize.Width() - searchButtonSpace - rightOffset;
        searchButtonHorizontalOffset = std::max(searchButtonHorizontalOffset, 0.0f);
    }
    auto searchButtonOffset = OffsetF(searchButtonHorizontalOffset, searchButtonVerticalOffset);
    searchButtonGeometryNode->SetMarginFrameOffset(searchButtonOffset);
    searchButtonWrapper->Layout();
}

void SearchLayoutAlgorithm::LayoutCancelButton(const LayoutSearchParams& params)
{
    auto dividerSideSpace = params.searchTheme->GetDividerSideSpace().ConvertToPx();
    auto dividerWidth = params.searchTheme->GetSearchDividerWidth().ConvertToPx();

    auto cancelButtonWrapper = params.layoutWrapper->GetOrCreateChildByIndex(CANCEL_BUTTON_INDEX);
    CHECK_NULL_VOID(cancelButtonWrapper);
    auto cancelButtonGeometryNode = cancelButtonWrapper->GetGeometryNode();
    CHECK_NULL_VOID(cancelButtonGeometryNode);
    auto cancelButtonFrameSize = cancelButtonGeometryNode->GetFrameSize();
    auto cancelButtonFrameWidth = cancelButtonFrameSize.Width();
    auto cancelButtonFrameHeight = cancelButtonFrameSize.Height();

    auto searchButtonWrapper = params.layoutWrapper->GetOrCreateChildByIndex(BUTTON_INDEX);
    CHECK_NULL_VOID(searchButtonWrapper);
    auto searchButtonGeometryNode = searchButtonWrapper->GetGeometryNode();
    CHECK_NULL_VOID(searchButtonGeometryNode);
    auto searchButtonFrameSize = searchButtonGeometryNode->GetFrameSize();
    auto searchButtonHorizontalOffset = searchButtonGeometryNode->GetMarginFrameOffset().GetX();

    auto cancelButtonHorizontalOffset = 0;
    auto cancelButtonVerticalOffset = (params.searchFrameHeight - cancelButtonFrameHeight) / 2;
    auto searchButtonNode = searchButtonWrapper->GetHostNode();
    auto searchButtonEvent = searchButtonNode->GetEventHub<ButtonEventHub>();
    if (params.isRTL) {
        if (searchButtonEvent->IsEnabled()) {
            cancelButtonHorizontalOffset =
                searchButtonHorizontalOffset + (searchButtonFrameSize.Width() + TWO * dividerSideSpace + dividerWidth);
        } else {
            cancelButtonHorizontalOffset = searchButtonHorizontalOffset;
        }
    } else {
        if (searchButtonEvent->IsEnabled()) {
            auto cancelButtonOffsetToSearchButton = cancelButtonFrameWidth + 2 * dividerSideSpace + dividerWidth;
            cancelButtonHorizontalOffset =
                std::max(searchButtonHorizontalOffset - cancelButtonOffsetToSearchButton, 0.0);
        } else {
            cancelButtonHorizontalOffset = params.searchFrameWidth - cancelButtonFrameWidth;
        }
    }
    auto cancelButtonOffset = OffsetF(cancelButtonHorizontalOffset, cancelButtonVerticalOffset);
    cancelButtonGeometryNode->SetMarginFrameOffset(cancelButtonOffset);
    cancelButtonWrapper->Layout();
}

void SearchLayoutAlgorithm::LayoutCancelImage(const LayoutSearchParams& params)
{
    auto cancelImageWrapper = params.layoutWrapper->GetOrCreateChildByIndex(CANCEL_IMAGE_INDEX);
    CHECK_NULL_VOID(cancelImageWrapper);
    auto cancelImageGeometryNode = cancelImageWrapper->GetGeometryNode();
    CHECK_NULL_VOID(cancelImageGeometryNode);
    auto cancelImageFrameSize = cancelImageGeometryNode->GetFrameSize();
    auto cancelImageFrameWidth = cancelImageFrameSize.Width();
    auto cancelImageFrameHeight = cancelImageFrameSize.Height();

    auto cancelButtonWrapper = params.layoutWrapper->GetOrCreateChildByIndex(CANCEL_BUTTON_INDEX);
    CHECK_NULL_VOID(cancelButtonWrapper);
    auto cancelButtonGeometryNode = cancelButtonWrapper->GetGeometryNode();
    CHECK_NULL_VOID(cancelButtonGeometryNode);
    auto cancelButtonHorizontalOffset = cancelButtonGeometryNode->GetMarginFrameOffset().GetX();
    auto cancelButtonFrameWidth = cancelButtonGeometryNode->GetFrameSize().Width();

    auto cancelImageVerticalOffset = (params.searchFrameHeight - cancelImageFrameHeight) / 2;
    auto cancelButtonImageCenterOffset = (cancelButtonFrameWidth - cancelImageFrameWidth) / 2;
    auto cancelImageHorizontalOffset = cancelButtonHorizontalOffset + cancelButtonImageCenterOffset;
    auto cancelImageOffset = OffsetF(cancelImageHorizontalOffset, cancelImageVerticalOffset);
    cancelImageGeometryNode->SetMarginFrameOffset(cancelImageOffset);
    cancelImageWrapper->Layout();
}

void SearchLayoutAlgorithm::LayoutTextField(const LayoutSearchParams& params)
{
    auto searchIconLeftSpace = params.searchTheme->GetSearchIconLeftSpace().ConvertToPx();
    auto searchIconRightSpace = params.searchTheme->GetSearchIconRightSpace().ConvertToPx();
    auto searchIconWidth = searchIconSizeMeasure_.Width();
    auto layoutProperty = DynamicCast<SearchLayoutProperty>(params.layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto padding = layoutProperty->CreatePaddingAndBorder();
    auto dividerSideSpace = params.searchTheme->GetDividerSideSpace().ConvertToPx();
    auto dividerWidth = params.searchTheme->GetSearchDividerWidth().ConvertToPx();

    auto textFieldWrapper = params.layoutWrapper->GetOrCreateChildByIndex(TEXTFIELD_INDEX);
    CHECK_NULL_VOID(textFieldWrapper);
    auto textFieldGeometryNode = textFieldWrapper->GetGeometryNode();
    CHECK_NULL_VOID(textFieldGeometryNode);
    auto textFieldHorizontalOffset = 0;

    auto searchButtonWrapper = params.layoutWrapper->GetOrCreateChildByIndex(BUTTON_INDEX);
    CHECK_NULL_VOID(searchButtonWrapper);
    auto searchButtonGeometryNode = searchButtonWrapper->GetGeometryNode();
    auto searchButtonFrameSize = searchButtonGeometryNode->GetFrameSize();
    auto searchButtonHorizontalOffset = searchButtonGeometryNode->GetMarginFrameOffset().GetX();

    auto cancelButtonWrapper = params.layoutWrapper->GetOrCreateChildByIndex(CANCEL_BUTTON_INDEX);
    CHECK_NULL_VOID(cancelButtonWrapper);
    auto cancelButtonGeometryNode = cancelButtonWrapper->GetGeometryNode();
    auto cancelButtonFrameWidth = cancelButtonGeometryNode->GetFrameSize().Width();

    auto searchButtonEvent = searchButtonWrapper->GetHostNode()->GetEventHub<ButtonEventHub>();

    auto style = params.layoutProperty->GetCancelButtonStyle().value_or(CancelButtonStyle::INPUT);
    if (params.isRTL) {
        if (searchButtonEvent->IsEnabled()) {
            if (style != CancelButtonStyle::INVISIBLE) {
                textFieldHorizontalOffset = searchButtonHorizontalOffset + searchButtonFrameSize.Width() +
                    TWO * dividerSideSpace + dividerWidth + searchIconLeftSpace + cancelButtonFrameWidth;
            } else {
                textFieldHorizontalOffset =
                    searchButtonHorizontalOffset + searchButtonFrameSize.Width() + searchIconLeftSpace;
            }
        } else {
            if (style != CancelButtonStyle::INVISIBLE) {
                textFieldHorizontalOffset =
                    searchButtonHorizontalOffset + cancelButtonFrameWidth + searchIconRightSpace;
            } else {
                textFieldHorizontalOffset = searchButtonHorizontalOffset + searchIconRightSpace;
            }
        }
    } else {
        textFieldHorizontalOffset = searchIconWidth + searchIconLeftSpace
            + searchIconRightSpace + padding.left.value_or(0.0f);
    }

    auto textFieldVerticalOffset = (params.searchFrameHeight - textFieldGeometryNode->GetFrameSize().Height()) / 2;
    textFieldGeometryNode->SetMarginFrameOffset(OffsetF(textFieldHorizontalOffset, textFieldVerticalOffset));
    textFieldWrapper->Layout();
}
} // namespace OHOS::Ace::NG
