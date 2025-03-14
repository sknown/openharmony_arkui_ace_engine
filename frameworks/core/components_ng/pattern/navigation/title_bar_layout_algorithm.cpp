/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/navigation/title_bar_layout_algorithm.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "base/utils/measure_util.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/common/container.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/app_bar/app_bar_theme.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/navigation/nav_bar_node.h"
#include "core/components_ng/pattern/navigation/nav_bar_pattern.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/pattern/navigation/navigation_layout_property.h"
#include "core/components_ng/pattern/navigation/title_bar_layout_property.h"
#include "core/components_ng/pattern/navigation/title_bar_node.h"
#include "core/components_ng/pattern/navigation/title_bar_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_property.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"
#ifdef ENABLE_ROSEN_BACKEND
#include "core/components/custom_paint/rosen_render_custom_paint.h"
#endif

namespace OHOS::Ace::NG {

namespace {
constexpr int32_t MENU_OFFSET_RATIO = 9;
// maximum radio of the subtitle height to the titlebar height
constexpr double SUBTITLE_MAX_HEIGHT_RADIO = 0.35;
} // namespace

void TitleBarLayoutAlgorithm::BackButtonLayout(const RefPtr<FrameNode>& backButtonNode,
    const RefPtr<LayoutProperty>& buttonLayoutProperty)
{
    auto backButtonImageNode = AceType::DynamicCast<FrameNode>(backButtonNode->GetChildren().front());
    CHECK_NULL_VOID(backButtonImageNode);
    auto menuPadding = MENU_BUTTON_PADDING;
    if (backButtonImageNode->GetTag() == V2::IMAGE_ETS_TAG) {
        auto backButtonImageLayoutProperty = backButtonImageNode->GetLayoutProperty<ImageLayoutProperty>();
        CHECK_NULL_VOID(backButtonImageLayoutProperty);
        backButtonImageLayoutProperty->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(backIconWidth_), CalcLength(backIconHeight_)));
    } else if (backButtonImageNode->GetTag() == V2::SYMBOL_ETS_TAG) {
        auto symbolProperty = backButtonImageNode->GetLayoutProperty<TextLayoutProperty>();
        auto symbolSourceInfo = symbolProperty->GetSymbolSourceInfo();
        auto theme = NavigationGetTheme();
        CHECK_NULL_VOID(theme);
        if (symbolSourceInfo.has_value() && symbolSourceInfo.value().GetUnicode() == theme->GetBackSymbolId()) {
            menuPadding = BACK_BUTTON_SYMBOL_PADDING;
        }
    }

    PaddingProperty padding;
    padding.SetEdges(CalcLength(menuPadding), CalcLength(menuPadding), CalcLength(MENU_BUTTON_PADDING),
        CalcLength(MENU_BUTTON_PADDING));
    buttonLayoutProperty->UpdatePadding(padding);
}

void TitleBarLayoutAlgorithm::MeasureBackButton(LayoutWrapper* layoutWrapper, const RefPtr<TitleBarNode>& titleBarNode,
    const RefPtr<TitleBarLayoutProperty>& titleBarLayoutProperty)
{
    auto backButtonNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetBackButton());
    CHECK_NULL_VOID(backButtonNode);
    auto index = titleBarNode->GetChildIndexById(backButtonNode->GetId());
    auto backButtonWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_VOID(backButtonWrapper);
    auto backButtonLayoutProperty = backButtonNode->GetLayoutProperty();

    auto constraint = titleBarLayoutProperty->CreateChildConstraint();
    // navDestination title bar
    if (titleBarLayoutProperty->GetTitleBarParentTypeValue(TitleBarParentType::NAVBAR) ==
        TitleBarParentType::NAV_DESTINATION) {
        if (!showBackButton_) {
            backButtonLayoutProperty->UpdateVisibility(VisibleType::GONE);
            return;
        }
        backButtonLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
        PaddingProperty padding;
        padding.SetEdges(CalcLength(BUTTON_PADDING));
        backButtonLayoutProperty->UpdatePadding(padding);
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            constraint.selfIdealSize = OptionalSizeF(static_cast<float>(BACK_BUTTON_ICON_SIZE.ConvertToPx()),
                static_cast<float>(BACK_BUTTON_ICON_SIZE.ConvertToPx()));
            backButtonWrapper->Measure(constraint);
            return;
        }
        if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            BackButtonLayout(backButtonNode, backButtonLayoutProperty);
            constraint.selfIdealSize = OptionalSizeF(static_cast<float>(backButtonWidth_.ConvertToPx()),
                static_cast<float>(backButtonWidth_.ConvertToPx()));
            backButtonWrapper->Measure(constraint);
            return;
        }
        constraint.selfIdealSize = OptionalSizeF(static_cast<float>(BACK_BUTTON_SIZE.ConvertToPx()),
            static_cast<float>(BACK_BUTTON_SIZE.ConvertToPx()));
        backButtonWrapper->Measure(constraint);
        return;
    }

    backButtonLayoutProperty->UpdateVisibility(VisibleType::GONE);
    // navBar title bar
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::MINI) {
        return;
    }

    if (titleBarLayoutProperty->GetHideBackButton().value_or(false)) {
        return;
    }

    backButtonLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        BackButtonLayout(backButtonNode, backButtonLayoutProperty);
        constraint.selfIdealSize = OptionalSizeF(static_cast<float>(backButtonWidth_.ConvertToPx()),
            static_cast<float>(backButtonWidth_.ConvertToPx()));
        backButtonWrapper->Measure(constraint);
        return;
    }

    constraint.selfIdealSize = OptionalSizeF(
        static_cast<float>(BACK_BUTTON_SIZE.ConvertToPx()), static_cast<float>(BACK_BUTTON_SIZE.ConvertToPx()));
    backButtonWrapper->Measure(constraint);
}

float TitleBarLayoutAlgorithm::GetTitleWidth(const RefPtr<TitleBarNode>& titleBarNode,
    const RefPtr<TitleBarLayoutProperty>& titleBarLayoutProperty, const SizeF& titleBarSize)
{
    double leftPadding = maxPaddingStart_.ConvertToPx();
    double rightPadding = maxPaddingEnd_.ConvertToPx();
    double horizontalMargin = NAV_HORIZONTAL_MARGIN_L.ConvertToPx();
    auto backButtonWidth = BACK_BUTTON_ICON_SIZE.ConvertToPx();
    auto customBackButtonRightPadding = BUTTON_PADDING.ConvertToPx();
    auto defaultPaddingStart = defaultPaddingStart_.ConvertToPx();
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        leftPadding = marginLeft_.ConvertToPx();
        rightPadding = marginRight_.ConvertToPx();
        horizontalMargin = menuCompPadding_.ConvertToPx();
        backButtonWidth = backButtonWidth_.ConvertToPx();
        customBackButtonRightPadding = 0.0f;
        defaultPaddingStart = rightPadding;
    }
    // navDestination title bar
    if (titleBarLayoutProperty->GetTitleBarParentTypeValue(TitleBarParentType::NAVBAR) ==
        TitleBarParentType::NAV_DESTINATION) {
        // nav destination custom title
        auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(titleBarNode->GetParent());
        CHECK_NULL_RETURN(navDestination, 0.0f);
        auto isCustom = navDestination->GetPrevTitleIsCustomValue(false);
        float occupiedWidth = 0.0f;
        // left padding
        if (showBackButton_) {
            if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
                occupiedWidth += isCustom ? backButtonWidth_.ConvertToPx() + leftPadding :
                    backButtonWidth_.ConvertToPx() + leftPadding + horizontalMargin;
            } else {
                occupiedWidth += isCustom ? (BACK_BUTTON_ICON_SIZE + BUTTON_PADDING).ConvertToPx() + leftPadding :
                    (BACK_BUTTON_ICON_SIZE).ConvertToPx() + leftPadding + horizontalMargin;
            }
        } else {
            occupiedWidth += isCustom ? 0.0f : leftPadding;
        }
        // compute right padding
        if (NearZero(menuWidth_)) {
            occupiedWidth += isCustom ? 0.0f : rightPadding;
        } else {
            occupiedWidth += menuWidth_;
            if (!titleBarNode->GetPrevMenuIsCustomValue(false)) {
                occupiedWidth += leftPadding;
                occupiedWidth += isCustom ? 0.0f : horizontalMargin;
            }
        }
        return titleBarSize.Width() < occupiedWidth ? 0.0f : titleBarSize.Width() - occupiedWidth;
    }
    // navBar title bar
    auto navBarNode = AceType::DynamicCast<NavBarNode>(titleBarNode->GetParent());
    CHECK_NULL_RETURN(navBarNode, 0.0f);
    float occupiedWidth = 0.0f;
    auto isCustom = navBarNode->GetPrevTitleIsCustomValue(false);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::MINI) {
        // mini mode
        if (titleBarLayoutProperty->GetHideBackButtonValue(false)) {
            occupiedWidth += isCustom ? 0.0f : leftPadding;
        } else {
            occupiedWidth += leftPadding + backButtonWidth;
            // custom right padding is the back button hot zone
            occupiedWidth += isCustom ? customBackButtonRightPadding : horizontalMargin;
        }
        // compute right padding
        if (NearZero(menuWidth_)) {
            occupiedWidth += isCustom ? 0.0f : rightPadding;
        } else {
            occupiedWidth += menuWidth_;
            if (!navBarNode->GetPrevMenuIsCustomValue(false)) {
                occupiedWidth += defaultPaddingStart;
                occupiedWidth += isCustom ? 0.0f : horizontalMargin;
            }
        }
        return titleBarSize.Width() < occupiedWidth ? 0.0f : titleBarSize.Width() - occupiedWidth;
    }
    // left padding of full and free mode
    occupiedWidth = isCustom ? 0.0f : leftPadding;
    // right padding of full mode
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::FULL
        || isCustom) {
        occupiedWidth += isCustom ? 0.0f : rightPadding;
        return titleBarSize.Width() < occupiedWidth ? 0.0f : titleBarSize.Width() - occupiedWidth;
    }
    // right padding of free mode
    auto titleBarPattern = titleBarNode->GetPattern<TitleBarPattern>();
    if (titleBarPattern && titleBarPattern->IsFreeTitleUpdated() &&
        titleBarPattern->GetTempTitleOffsetY() < menuHeight_) {
        if (NearZero(menuWidth_)) {
            occupiedWidth += isCustom ? 0.0f : rightPadding;
        } else {
            occupiedWidth += menuWidth_;
            if (!navBarNode->GetPrevMenuIsCustomValue(false)) {
                occupiedWidth += leftPadding;
                occupiedWidth += isCustom ? 0.0f : horizontalMargin;
            }
        }
    } else {
        occupiedWidth += isCustom ? 0.0f : rightPadding;
    }
    return titleBarSize.Width() < occupiedWidth ? 0.0f : titleBarSize.Width() - occupiedWidth;
}

float TitleBarLayoutAlgorithm::WidthAfterAvoidMenubar(const RefPtr<TitleBarNode>& titleBarNode, float width)
{
    float afterAvoidWidth = width;
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, afterAvoidWidth);
    if (!pipeline->GetInstallationFree()) {
        return afterAvoidWidth;
    }

    auto titlebarRect = titleBarNode->GetParentGlobalOffsetDuringLayout();

    auto container = Container::Current();
    CHECK_NULL_RETURN(container, afterAvoidWidth);
    auto appBar = container->GetAppBar();
    CHECK_NULL_RETURN(appBar, afterAvoidWidth);
    auto appBarRect = appBar->GetAppBarRect();
    CHECK_NULL_RETURN(appBarRect, afterAvoidWidth);
    auto appBarOffset = appBarRect->GetOffset();
    auto appBarSize = appBarRect->GetSize();

    auto titleBarGeo = titleBarNode->GetGeometryNode();
    CHECK_NULL_RETURN(titleBarGeo, afterAvoidWidth);

    auto avoidArea = titlebarRect.GetX() + titleBarGeo->GetFrameSize().Width() - appBarOffset.GetX();
    if (AceApplicationInfo::GetInstance().IsRightToLeft()) {
        avoidArea = appBarOffset.GetX() + appBarSize.Width();
    }
    auto buttonTop = appBarOffset.GetY() + appBarSize.Height();
    if (LessOrEqual(titlebarRect.GetY(), buttonTop) && GreatOrEqual(avoidArea, 0.0)) {
        afterAvoidWidth = afterAvoidWidth - avoidArea;
    }

    if (LessOrEqual(afterAvoidWidth, 0.0)) {
        return 0.0f;
    }
    return afterAvoidWidth;
}

void TitleBarLayoutAlgorithm::MeasureSubtitle(LayoutWrapper* layoutWrapper, const RefPtr<TitleBarNode>& titleBarNode,
    const RefPtr<TitleBarLayoutProperty>& titleBarLayoutProperty, const SizeF& titleBarSize, float maxWidth)
{
    auto subtitleNode = titleBarNode->GetSubtitle();
    CHECK_NULL_VOID(subtitleNode);
    auto index = titleBarNode->GetChildIndexById(subtitleNode->GetId());
    auto subtitleWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_VOID(subtitleWrapper);
    auto constraint = titleBarLayoutProperty->CreateChildConstraint();
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        // limit the maxHeight of the subtitle to adapt to the scenarios where the text is too high
        constraint.maxSize.SetHeight(SUBTITLE_MAX_HEIGHT_RADIO * titleBarSize.Height());
    } else {
        constraint.maxSize.SetHeight(titleBarSize.Height());
    }
    constraint.maxSize.SetWidth(maxWidth);
    subtitleWrapper->Measure(constraint);
}

void TitleBarLayoutAlgorithm::MeasureTitle(LayoutWrapper* layoutWrapper, const RefPtr<TitleBarNode>& titleBarNode,
    const RefPtr<TitleBarLayoutProperty>& titleBarLayoutProperty, const SizeF& titleBarSize, float maxWidth)
{
    auto titleNode = titleBarNode->GetTitle();
    CHECK_NULL_VOID(titleNode);
    auto index = titleBarNode->GetChildIndexById(titleNode->GetId());
    auto titleWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_VOID(titleWrapper);
    auto constraint = titleBarLayoutProperty->CreateChildConstraint();
    constraint.maxSize.SetHeight(titleBarSize.Height());

    // navDestination title bar
    if (titleBarLayoutProperty->GetTitleBarParentTypeValue(TitleBarParentType::NAVBAR) ==
        TitleBarParentType::NAV_DESTINATION) {
        auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(titleBarNode->GetParent());
        CHECK_NULL_VOID(navDestination);
        auto isCustomTitle = navDestination->GetPrevTitleIsCustomValue(false);
        if (isCustomTitle) {
            constraint.parentIdealSize.SetWidth(maxWidth);
            constraint.maxSize.SetWidth(maxWidth);
            // custom title must be single line title

            constraint.parentIdealSize.SetHeight(titleBarSize.Height());
            constraint.maxSize.SetHeight(titleBarSize.Height());
            titleWrapper->Measure(constraint);
            return;
        }
        constraint.maxSize.SetWidth(maxWidth);
        if (!titleBarNode->GetSubtitle()) {
            constraint.maxSize.SetHeight(titleBarSize.Height());
            titleWrapper->Measure(constraint);
            return;
        }
        auto subtitle = AceType::DynamicCast<FrameNode>(titleBarNode->GetSubtitle());
        auto subtitleHeight = subtitle->GetGeometryNode()->GetFrameSize().Height();
        constraint.maxSize.SetHeight(titleBarSize.Height() - subtitleHeight);
        titleWrapper->Measure(constraint);
        return;
    }
    // NavigationCustomTitle: Custom title + height
    if (titleBarLayoutProperty->HasTitleHeight()) {
        constraint.parentIdealSize.SetWidth(maxWidth);
        constraint.maxSize.SetWidth(maxWidth);
        constraint.parentIdealSize.SetHeight(titleBarSize.Height());
        constraint.maxSize.SetHeight(titleBarSize.Height());
        titleWrapper->Measure(constraint);
        return;
    }
    // subTitle
    auto titleMode = titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE);
    auto subTitle = titleBarNode->GetSubtitle();
    float titleSpaceVertical = 0.0f;
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        titleSpaceVertical = static_cast<float>(titleSpaceVertical_.ConvertToPx());
    }
    if (subTitle) {
        // common title
        auto index = titleBarNode->GetChildIndexById(subTitle->GetId());
        auto subtitleWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        CHECK_NULL_VOID(subtitleWrapper);
        auto subtitleHeight = subtitleWrapper->GetGeometryNode()->GetFrameSize().Height();
        // mini mode double title height is 56vp, free/full mode is 82vp
        auto maxTitleHeight = titleMode == NavigationTitleMode::MINI ? SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx() :
            DOUBLE_LINE_TITLEBAR_HEIGHT.ConvertToPx();
        constraint.maxSize.SetWidth(maxWidth);
        constraint.maxSize.SetHeight(maxTitleHeight - subtitleHeight - titleSpaceVertical);
        titleWrapper->Measure(constraint);
        return;
    }
    auto navBarNode = AceType::DynamicCast<NavBarNode>(titleBarNode->GetParent());
    CHECK_NULL_VOID(navBarNode);
    auto isCustomTitle = navBarNode->GetPrevTitleIsCustomValue(false);
    // single line title and mini mode
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::MINI) {
        if (isCustomTitle) {
            constraint.parentIdealSize.SetWidth(maxWidth);
            constraint.maxSize.SetWidth(maxWidth);
            constraint.parentIdealSize.SetHeight(titleBarSize.Height());
            constraint.maxSize.SetHeight(titleBarSize.Height());
        } else {
            constraint.maxSize.SetWidth(maxWidth);
            constraint.maxSize.SetHeight(titleBarSize.Height());
        }
        titleWrapper->Measure(constraint);
        return;
    }
    // custom builder
    if (isCustomTitle) {
        constraint.parentIdealSize.SetWidth(maxWidth);
        constraint.maxSize.SetWidth(maxWidth);
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            constraint.parentIdealSize.SetHeight(titleBarSize.Height());
        } else {
            auto isCustomMenu = navBarNode->GetPrevMenuIsCustomValue(false);
            // if has menu and menu is not custom, max height is single line height
            auto maxHeight = NearZero(menuWidth_) ? titleBarSize.Height()
                             : isCustomMenu       ? titleBarSize.Height() - menuHeight_
                                                  : SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx();
            constraint.parentIdealSize.SetHeight(maxHeight);
            constraint.maxSize.SetHeight(maxHeight);
        }
        titleWrapper->Measure(constraint);
        return;
    }
    // resourceStr title
    constraint.maxSize.SetWidth(maxWidth);
    constraint.maxSize.SetHeight(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    titleWrapper->Measure(constraint);
}

void TitleBarLayoutAlgorithm::MeasureMenu(LayoutWrapper* layoutWrapper, const RefPtr<TitleBarNode>& titleBarNode,
    const RefPtr<TitleBarLayoutProperty>& titleBarLayoutProperty)
{
    auto menuNode = titleBarNode->GetMenu();
    CHECK_NULL_VOID(menuNode);
    auto index = titleBarNode->GetChildIndexById(menuNode->GetId());
    auto menuWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_VOID(menuWrapper);
    auto constraint = titleBarLayoutProperty->CreateChildConstraint();

    auto layoutProperty = AceType::DynamicCast<TitleBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    bool isCustomMenu = false;
    int32_t maxMenu = 0;
    if (layoutProperty->GetTitleBarParentTypeValue(TitleBarParentType::NAVBAR) != TitleBarParentType::NAV_DESTINATION) {
        auto navBarNode = AceType::DynamicCast<NavBarNode>(titleBarNode->GetParent());
        CHECK_NULL_VOID(navBarNode);
        isCustomMenu = navBarNode->GetPrevMenuIsCustomValue(false);
        auto navBarPattern = AceType::DynamicCast<NavBarPattern>(navBarNode->GetPattern());
        CHECK_NULL_VOID(navBarPattern);
        maxMenu = navBarPattern->GetMaxMenuNum();
    } else {
        isCustomMenu = titleBarNode->GetPrevMenuIsCustomValue(false);
        auto titleBarPattern = AceType::DynamicCast<TitleBarPattern>(titleBarNode->GetPattern());
        CHECK_NULL_VOID(titleBarPattern);
        maxMenu = titleBarPattern->GetMaxMenuNum();
    }

    if (isCustomMenu) {
        // custom title can't be higher than 56vp
        constraint.parentIdealSize.SetHeight(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
        if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::MINI &&
            !titleBarLayoutProperty->HasTitleHeight()) {
            auto maxWidth = static_cast<float>(MENU_ITEM_SIZE.ConvertToPx()) * maxMenu +
                            defaultPaddingStart_.ConvertToPx();
            constraint.parentIdealSize.SetWidth(maxWidth);
        }
        menuWrapper->Measure(constraint);
        menuWidth_ = menuWrapper->GetGeometryNode()->GetFrameSize().Width();
        menuHeight_ = menuWrapper->GetGeometryNode()->GetFrameSize().Height();
        return;
    }
    auto menuItemNum = static_cast<int32_t>(menuNode->GetChildren().size());
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        if (menuItemNum >= maxMenu) {
            menuWidth_ = static_cast<float>(iconBackgroundWidth_.ConvertToPx()) * maxMenu +
                static_cast<float>(menuCompPadding_.ConvertToPx()) * (maxMenu - 1);
        } else {
            menuWidth_ = static_cast<float>(iconBackgroundWidth_.ConvertToPx()) * menuItemNum +
                static_cast<float>(menuCompPadding_.ConvertToPx()) * (menuItemNum - 1);
        }
    } else {
        if (menuItemNum >= maxMenu) {
            menuWidth_ = static_cast<float>(MENU_ITEM_SIZE.ConvertToPx()) * maxMenu;
        } else {
            menuWidth_ = static_cast<float>(MENU_ITEM_SIZE.ConvertToPx()) * menuItemNum;
        }
    }
    constraint.selfIdealSize = OptionalSizeF(menuWidth_, menuHeight_);
    menuWrapper->Measure(constraint);
}

void TitleBarLayoutAlgorithm::ShowBackButtonLayout(LayoutWrapper* layoutWrapper,
    const RefPtr<TitleBarLayoutProperty>& titleBarLayoutProperty,
    RefPtr<GeometryNode>& geometryNode, const RefPtr<LayoutWrapper>& backButtonWrapper)
{
    auto titleHeight = titleBarLayoutProperty->GetTitleHeightValue(SINGLE_LINE_TITLEBAR_HEIGHT);
    Dimension backButtonHeight = BACK_BUTTON_SIZE;
    auto leftMargin = maxPaddingStart_ - BUTTON_PADDING;
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        backButtonHeight = backButtonHeight_;
        leftMargin = marginLeft_;
    }
    float dividerOffset = 2.0f;
    auto offsetY = (titleHeight - backButtonHeight) / dividerOffset;
    auto offsetX = static_cast<float>(leftMargin.ConvertToPx());
    offsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, offsetX);
    OffsetF backButtonOffset = OffsetF(offsetX, static_cast<float>(offsetY.ConvertToPx()));
    geometryNode->SetMarginFrameOffset(backButtonOffset);
    backButtonWrapper->Layout();
}

void TitleBarLayoutAlgorithm::LayoutBackButton(LayoutWrapper* layoutWrapper, const RefPtr<TitleBarNode>& titleBarNode,
    const RefPtr<TitleBarLayoutProperty>& titleBarLayoutProperty)
{
    auto backButtonNode = titleBarNode->GetBackButton();
    CHECK_NULL_VOID(backButtonNode);
    auto index = titleBarNode->GetChildIndexById(backButtonNode->GetId());
    auto backButtonWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_VOID(backButtonWrapper);
    auto geometryNode = backButtonWrapper->GetGeometryNode();

    // navDestination title bar
    if (titleBarLayoutProperty->GetTitleBarParentTypeValue(TitleBarParentType::NAVBAR) ==
        TitleBarParentType::NAV_DESTINATION) {
        OffsetF backButtonOffset = OffsetF(0.0f, 0.0f);
        if (!showBackButton_) {
            SizeF size = SizeF(0.0f, 0.0f);
            geometryNode->SetFrameSize(size);
            backButtonWrapper->Layout();
            return;
        }
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            auto offsetY = (menuHeight_ - BACK_BUTTON_ICON_SIZE.ConvertToPx()) / 2;
            auto offsetXResult = ChangeOffsetByDirection(layoutWrapper, geometryNode,
                static_cast<float>(maxPaddingStart_.ConvertToPx()));
            backButtonOffset = OffsetF(offsetXResult, offsetY);
            geometryNode->SetMarginFrameOffset(backButtonOffset);
            backButtonWrapper->Layout();
            return;
        }

        ShowBackButtonLayout(layoutWrapper, titleBarLayoutProperty, geometryNode, backButtonWrapper);
        return;
    }

    // navBar title bar
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::MINI) {
        OffsetF backButtonOffset = OffsetF(0.0f, 0.0f);
        geometryNode->SetMarginFrameOffset(backButtonOffset);
        backButtonWrapper->Layout();
        return;
    }

    if (titleBarLayoutProperty->GetHideBackButton().value_or(false)) {
        OffsetF backButtonOffset = OffsetF(0.0f, 0.0f);
        geometryNode->SetMarginFrameOffset(backButtonOffset);
        backButtonWrapper->Layout();
        return;
    }

    ShowBackButtonLayout(layoutWrapper, titleBarLayoutProperty, geometryNode, backButtonWrapper);
}

float TitleBarLayoutAlgorithm::GetFullModeTitleOffsetY(float titleHeight, float subtitleHeight,
    RefPtr<GeometryNode> titleBarGeometryNode)
{
    auto titleBarHeight = titleBarGeometryNode->GetFrameSize().Height();
    // fixed white space menuHeight
    OffsetF titleOffset = OffsetF(0.0f, 0.0f);
    float offsetY = 0.0f;
    auto titleSpace = titleBarHeight - menuHeight_ - static_cast<float>(paddingTopTwolines_.ConvertToPx());
    auto titleRealHeight = titleHeight + subtitleHeight + navTitleSpaceVertical_;
    float dividerOffset = 2.0f;
    if (NearZero(subtitleHeight) && titleHeight < titleBarHeight - menuHeight_) {
        offsetY = (titleBarHeight - menuHeight_ - titleRealHeight) / dividerOffset;
        return offsetY;
    }
    if (titleRealHeight <= titleSpace) {
        offsetY = (titleSpace - titleRealHeight +
            static_cast<float>(paddingTopTwolines_.ConvertToPx())) / dividerOffset;
    } else {
        offsetY = titleSpace - titleRealHeight;
    }

    return offsetY;
}

void TitleBarLayoutAlgorithm::LayoutTitle(LayoutWrapper* layoutWrapper, const RefPtr<TitleBarNode>& titleBarNode,
    const RefPtr<TitleBarLayoutProperty>& titleBarLayoutProperty, float subtitleHeight)
{
    auto titleNode = titleBarNode->GetTitle();
    CHECK_NULL_VOID(titleNode);
    auto index = titleBarNode->GetChildIndexById(titleNode->GetId());
    auto titleWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_VOID(titleWrapper);
    auto geometryNode = titleWrapper->GetGeometryNode();
    auto titleBarGeometryNode = titleBarNode->GetGeometryNode();
    CHECK_NULL_VOID(titleBarGeometryNode);

    auto titleHeight = geometryNode->GetFrameSize().Height();
    float offsetY = 0.0f;
    float dividerOffset = 2.0f;
    if (!NearZero(subtitleHeight)) {
        offsetY = (doubleLineTitleBarHeight_ - titleHeight - subtitleHeight - navTitleSpaceVertical_) / dividerOffset;
    } else {
        navTitleSpaceVertical_ = 0.0f;
        offsetY = (singleLineTitleHeight_ - titleHeight) / dividerOffset;
    }
    // navDestination title bar
    if (titleBarLayoutProperty->GetTitleBarParentTypeValue(TitleBarParentType::NAVBAR) ==
        TitleBarParentType::NAV_DESTINATION) {
        auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(titleBarNode->GetParent());
        CHECK_NULL_VOID(navDestination);
        auto isCustom = navDestination->GetPrevTitleIsCustomValue(false);
        OffsetF titleOffset = OffsetF(0.0f, 0.0f);
        // add sdk 9 compatible
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            if (showBackButton_) {
                auto offsetXResult = ChangeOffsetByDirection(layoutWrapper, geometryNode,
                    static_cast<float>(
                        (maxPaddingStart_ + BACK_BUTTON_ICON_SIZE + NAV_HORIZONTAL_MARGIN_M).ConvertToPx()));
                titleOffset = OffsetF(offsetXResult, offsetY);
                geometryNode->SetMarginFrameOffset(titleOffset);
                titleWrapper->Layout();
                return;
            }
            auto offsetXResult = ChangeOffsetByDirection(layoutWrapper, geometryNode,
                static_cast<float>(maxPaddingStart_.ConvertToPx()));
            titleOffset = OffsetF(offsetXResult, offsetY);
            geometryNode->SetMarginFrameOffset(titleOffset);
            titleWrapper->Layout();
            return;
        }
        if (showBackButton_) {
            auto offsetX = isCustom ? (navLeftMargin_ + navBackIconWidth_ + navButtonPadding_) :
                (navLeftMargin_ + navBackIconWidth_ + navHorizontalMargin_);
            offsetY = isCustom ? 0.0f : offsetY;
            auto offsetXResult = ChangeOffsetByDirection(layoutWrapper, geometryNode, offsetX);
            titleOffset = OffsetF(offsetXResult, offsetY);
            geometryNode->SetMarginFrameOffset(titleOffset);
            titleWrapper->Layout();
            return;
        }
        auto offsetX = isCustom ? 0.0f : navLeftMargin_;
        offsetY = isCustom ? 0.0f : offsetY;
        auto offsetXResult = ChangeOffsetByDirection(layoutWrapper, geometryNode, offsetX);
        titleOffset = OffsetF(offsetXResult, offsetY);
        geometryNode->SetMarginFrameOffset(titleOffset);
        titleWrapper->Layout();
        return;
    }

    // navBar title bar
    auto navBarNode = AceType::DynamicCast<NavBarNode>(titleBarNode->GetParent());
    CHECK_NULL_VOID(navBarNode);
    auto isCustom = navBarNode->GetPrevTitleIsCustomValue(false);
    // full mode
    if (!isCustom) {
        float dividerOffset = 2.0f;
        if (!NearZero(subtitleHeight)) {
            offsetY = (doubleLineTitleBarHeight_ - titleHeight -
                subtitleHeight - navTitleSpaceVertical_) / dividerOffset;
        } else {
            navTitleSpaceVertical_ = 0.0f;
            offsetY = (singleLineTitleHeight_ - titleHeight) / dividerOffset;
        }
    }
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::MINI) {
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            if (titleBarLayoutProperty->GetHideBackButton().value_or(false)) {
                auto offsetX = maxPaddingStart_.ConvertToPx();
                offsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, offsetX);
                geometryNode->SetMarginFrameOffset(OffsetF { offsetX, offsetY });
                titleWrapper->Layout();
                return;
            }
            auto offsetX =  (defaultPaddingStart_ + BACK_BUTTON_ICON_SIZE + NAV_HORIZONTAL_MARGIN_L).ConvertToPx();
            offsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, offsetX);
            geometryNode->SetMarginFrameOffset(OffsetF {offsetX, offsetY});
            titleWrapper->Layout();
            return;
        }
        // NavigationCustomTitle and Custom builder layout margin is (0, 0);
        offsetY = isCustom ? 0 : offsetY;
        if (titleBarLayoutProperty->GetHideBackButton().value_or(false)) {
            auto offsetX = isCustom ? 0.0f : navLeftMargin_;
            offsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, offsetX);
            OffsetF titleOffset = OffsetF(offsetX, offsetY);
            geometryNode->SetMarginFrameOffset(titleOffset);
            titleWrapper->Layout();
            return;
        }

        auto offsetX = isCustom ? (navLeftMargin_ + navBackIconWidth_ + navButtonPadding_) :
                (navLeftMargin_ + navBackIconWidth_ + navHorizontalMargin_);
        offsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, offsetX);
        OffsetF offset = OffsetF(offsetX, offsetY);
        geometryNode->SetMarginFrameOffset(offset);
        titleWrapper->Layout();
        return;
    }

    float offsetX = navLeftMargin_;
    offsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, offsetX);
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        offsetY = GetFullModeTitleOffsetY(titleHeight, subtitleHeight, titleBarGeometryNode);
    }
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::FREE) {
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            offsetX = maxPaddingStart_.ConvertToPx();
            offsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, offsetX);
            geometryNode->SetMarginFrameOffset(OffsetF { offsetX, menuHeight_ + offsetY });
            titleWrapper->Layout();
            return;
        }
        // full mode
        if (isCustom) {
            // custom title margin is (0.0f, menuHeight_)
            auto customOffsetY = NearZero(menuWidth_) ? 0.0f : menuHeight_;
            float customOffsetX = 0.0f;
            customOffsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, customOffsetX);
            geometryNode->SetMarginFrameOffset(OffsetF { customOffsetX, customOffsetY});
            titleWrapper->Layout();
            return;
        }
        // fixed white space menuHeight
        OffsetF titleOffset = OffsetF(0.0f, 0.0f);
        titleOffset = OffsetF(offsetX, menuHeight_ + offsetY);
        geometryNode->SetMarginFrameOffset(titleOffset);
        titleWrapper->Layout();
        return;
    }

    auto titlePattern = titleBarNode->GetPattern<TitleBarPattern>();
    if (isCustom) {
        // customBuilder and NavigationCustomTitle offset is (0.0f, menuHeight_)
        auto customOffsetY = NearZero(menuWidth_) ? 0.0f : menuHeight_;
        auto customOffsetX = 0.0f;
        customOffsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, customOffsetX);
        geometryNode->SetMarginFrameOffset(OffsetF { customOffsetX, customOffsetY});
        titleWrapper->Layout();
        return;
    }
    auto title = AceType::DynamicCast<FrameNode>(titleNode);
    CHECK_NULL_VOID(title);
    if (isInitialTitle_) {
        auto textLayoutProperty = title->GetLayoutProperty<TextLayoutProperty>();
        if (!textLayoutProperty) {
            // current title mode is Navigation common title
            OffsetF titleOffset = OffsetF(offsetX, menuHeight_+ offsetY);
            geometryNode->SetMarginFrameOffset(titleOffset);
            titleWrapper->Layout();
            return;
        }
        MeasureContext context;
        context.textContent = textLayoutProperty->GetContentValue();
        context.fontSize = titleFontSize_;
#ifdef ENABLE_ROSEN_BACKEND
        minTitleHeight_ = static_cast<float>(RosenRenderCustomPaint::MeasureTextSizeInner(context).Height());
#else
        minTitleHeight_ = 0.0;
#endif
        initialTitleOffsetY_ = menuHeight_ + offsetY;
        isInitialTitle_ = false;
        auto titleOffset = OffsetF(offsetX, initialTitleOffsetY_);
        titlePattern->SetCurrentTitleOffsetY(initialTitleOffsetY_);
        geometryNode->SetMarginFrameOffset(titleOffset);
        titleWrapper->Layout();
        return;
    }

    if (NearZero(titlePattern->GetTempTitleOffsetY()) || !titlePattern->GetIsTitleMoving()) {
        initialTitleOffsetY_ = menuHeight_ + offsetY;
        auto titleOffset = OffsetF(offsetX, initialTitleOffsetY_);
        geometryNode->SetMarginFrameOffset(titleOffset);
        titleWrapper->Layout();
        return;
    }
    auto overDragOffset = titlePattern->GetOverDragOffset();
    auto titleOffset = OffsetF(offsetX, titlePattern->GetTempTitleOffsetY() + overDragOffset / 6.0f);
    titlePattern->SetCurrentTitleOffsetY(titleOffset.GetY());
    geometryNode->SetMarginFrameOffset(titleOffset);
    titleWrapper->Layout();
}

void TitleBarLayoutAlgorithm::LayoutSubtitle(LayoutWrapper* layoutWrapper, const RefPtr<TitleBarNode>& titleBarNode,
    const RefPtr<TitleBarLayoutProperty>& titleBarLayoutProperty, float titleHeight)
{
    auto subtitleNode = titleBarNode->GetSubtitle();
    CHECK_NULL_VOID(subtitleNode);
    auto index = titleBarNode->GetChildIndexById(subtitleNode->GetId());
    auto subtitleWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_VOID(subtitleWrapper);
    auto geometryNode = subtitleWrapper->GetGeometryNode();
    auto titleBarGeometryNode = titleBarNode->GetGeometryNode();
    CHECK_NULL_VOID(titleBarGeometryNode);

    auto subtitleHeight = geometryNode->GetFrameSize().Height();
    float offsetY = 0.0f;
    float dividerOffset = 2.0f;
    if (!NearZero(titleHeight)) {
        offsetY = (doubleLineTitleBarHeight_ - titleHeight -
                  subtitleHeight - navTitleSpaceVertical_) / dividerOffset + titleHeight + navTitleSpaceVertical_;
    } else {
        navTitleSpaceVertical_ = 0.0f;
        offsetY = (singleLineTitleHeight_ - subtitleHeight) / dividerOffset;
    }
    // navDestination title bar
    if (titleBarLayoutProperty->GetTitleBarParentTypeValue(TitleBarParentType::NAVBAR) ==
        TitleBarParentType::NAV_DESTINATION) {
        OffsetF subTitleOffset = OffsetF(0.0f, 0.0f);
        // subtitle doesn't support custom title
        if (showBackButton_) {
            if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
                auto offsetXResult = ChangeOffsetByDirection(layoutWrapper, geometryNode,
                    static_cast<float>(
                        (maxPaddingStart_ + BACK_BUTTON_ICON_SIZE + NAV_HORIZONTAL_MARGIN_M).ConvertToPx()));
                subTitleOffset = OffsetF(offsetXResult, offsetY);
                geometryNode->SetMarginFrameOffset(subTitleOffset);
            } else {
                auto offsetXResult = ChangeOffsetByDirection(layoutWrapper, geometryNode,
                    navLeftMargin_ + navBackIconWidth_ + navHorizontalMargin_);
                subTitleOffset = OffsetF(offsetXResult, offsetY);
                geometryNode->SetMarginFrameOffset(subTitleOffset);
            }
            subtitleWrapper->Layout();
            return;
        }

        auto offsetXResult = ChangeOffsetByDirection(layoutWrapper, geometryNode, navLeftMargin_);
        subTitleOffset = OffsetF(offsetXResult, offsetY);
        geometryNode->SetMarginFrameOffset(subTitleOffset);
        subtitleWrapper->Layout();
        return;
    }

    // navBar title bar
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::MINI) {
        float offsetX = 0.0f;
        offsetX = navLeftMargin_;
        if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            auto titleOffsetY = GetFullModeTitleOffsetY(titleHeight, subtitleHeight, titleBarGeometryNode);
            offsetY = titleOffsetY + titleHeight + navTitleSpaceVertical_;
        }
        offsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, offsetX);
        initialSubtitleOffsetY_ = menuHeight_ + offsetY;
        if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::FREE) {
            if (isInitialSubtitle_) {
                isInitialSubtitle_ = false;
                OffsetF titleOffset = OffsetF(offsetX, initialSubtitleOffsetY_);
                geometryNode->SetMarginFrameOffset(titleOffset);
                subtitleWrapper->Layout();
                return;
            }

            auto titlePattern = titleBarNode->GetPattern<TitleBarPattern>();
            CHECK_NULL_VOID(titlePattern);
            if (NearZero(titlePattern->GetTempTitleOffsetY()) || !titlePattern->GetIsTitleMoving()) {
                OffsetF titleOffset = OffsetF(offsetX, initialSubtitleOffsetY_);
                geometryNode->SetMarginFrameOffset(titleOffset);
                subtitleWrapper->Layout();
                return;
            }
            auto overDragOffset = titlePattern->GetOverDragOffset();
            OffsetF titleOffset = OffsetF(offsetX, titlePattern->GetTempSubTitleOffsetY() + overDragOffset / 6.0f);
            geometryNode->SetMarginFrameOffset(titleOffset);
            subtitleWrapper->Layout();
            return;
        }
        // full mode
        OffsetF titleOffset = OffsetF(offsetX, initialSubtitleOffsetY_);
        geometryNode->SetMarginFrameOffset(titleOffset);
        subtitleWrapper->Layout();
        return;
    }
    // mini mode + hideBackButton true
    if (titleBarLayoutProperty->GetHideBackButton().value_or(false)) {
        auto offsetX = navLeftMargin_;
        offsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, offsetX);
        OffsetF titleOffset = OffsetF(offsetX, offsetY);
        geometryNode->SetMarginFrameOffset(titleOffset);
        subtitleWrapper->Layout();
        return;
    }
    float occupiedWidth = 0.0f;
    // mini mode + back button
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
        occupiedWidth = static_cast<float>((maxPaddingStart_ + BACK_BUTTON_ICON_SIZE +
            NAV_HORIZONTAL_MARGIN_M).ConvertToPx());
    } else {
        occupiedWidth = navLeftMargin_ + navBackIconWidth_ + navHorizontalMargin_;
    }
    auto miniOffsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, occupiedWidth);
    OffsetF offset = OffsetF(miniOffsetX, offsetY);
    geometryNode->SetMarginFrameOffset(offset);
    subtitleWrapper->Layout();
}

void TitleBarLayoutAlgorithm::LayoutMenu(LayoutWrapper* layoutWrapper, const RefPtr<TitleBarNode>& titleBarNode,
    const RefPtr<TitleBarLayoutProperty>& titleBarLayoutProperty, float subtitleHeight)
{
    auto menuNode = titleBarNode->GetMenu();
    CHECK_NULL_VOID(menuNode);
    auto index = titleBarNode->GetChildIndexById(menuNode->GetId());
    auto menuWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_VOID(menuWrapper);
    auto geometryNode = menuWrapper->GetGeometryNode();
    auto menuWidth = geometryNode->GetMarginFrameSize().Width();
    auto maxWidth = geometryNode->GetParentLayoutConstraint()->maxSize.Width();
    maxWidth = WidthAfterAvoidMenubar(titleBarNode, maxWidth);

    auto layoutProperty = AceType::DynamicCast<TitleBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto isCustomMenu = false;
    if (layoutProperty->GetTitleBarParentTypeValue(TitleBarParentType::NAVBAR) != TitleBarParentType::NAV_DESTINATION) {
        auto navBarNode = AceType::DynamicCast<NavBarNode>(titleBarNode->GetParent());
        CHECK_NULL_VOID(navBarNode);
        isCustomMenu = navBarNode->GetPrevMenuIsCustomValue(false);
    } else {
        isCustomMenu = titleBarNode->GetPrevMenuIsCustomValue(false);
    }
    auto currentOffsetX = maxWidth - menuWidth - defaultPaddingStart_.ConvertToPx();
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::FREE) {
        auto titlePattern = titleBarNode->GetPattern<TitleBarPattern>();
        auto overDragOffset = titlePattern->GetOverDragOffset();
        auto menuOffsetY = isCustomMenu ? 0 : (SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx() - menuHeight_) / 2;
        // custom menu width has no right padding
        float offsetX = 0.0f;
        if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            offsetX = isCustomMenu ? maxWidth - menuWidth
                                   : (maxWidth - menuWidth - static_cast<float>(marginRight_.ConvertToPx()));
        } else {
            offsetX = isCustomMenu ? maxWidth - menuWidth
                                   : (maxWidth - menuWidth - static_cast<float>(maxPaddingEnd_.ConvertToPx()) +
                                         BUTTON_PADDING.ConvertToPx());
        }
        currentOffsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, currentOffsetX);
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            geometryNode->SetMarginFrameOffset(OffsetF { currentOffsetX,
                menuOffsetY + overDragOffset / MENU_OFFSET_RATIO });
            menuWrapper->Layout();
            return;
        }
        offsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, offsetX);
        OffsetF menuOffset(offsetX, menuOffsetY + overDragOffset / MENU_OFFSET_RATIO);
        geometryNode->SetMarginFrameOffset(menuOffset);
        menuWrapper->Layout();
        return;
    }
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
        auto totalHeight = NearZero(subtitleHeight) ? SINGLE_LINE_TITLEBAR_HEIGHT : DOUBLE_LINE_TITLEBAR_HEIGHT;
        geometryNode->SetMarginFrameOffset(OffsetF { currentOffsetX, (totalHeight.ConvertToPx() - menuHeight_) / 2 });
        menuWrapper->Layout();
        return;
    }
    // custom menu doesn't have top padding. if menu isn't custom, menu items has top padding
    auto menuOffsetY =  isCustomMenu ? 0.0f : (SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx() - menuHeight_) / 2;
    auto menuOffsetX = maxWidth - menuWidth;
    // custom menu doesn't have right padding. if menu isn't custom, menu items has right padding
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        menuOffsetX =
            isCustomMenu ? menuOffsetX : (menuOffsetX - marginRight_.ConvertToPx());
    } else {
        menuOffsetX =
            isCustomMenu ? menuOffsetX : (menuOffsetX - maxPaddingEnd_.ConvertToPx() + BUTTON_PADDING.ConvertToPx());
    }
    menuOffsetX = ChangeOffsetByDirection(layoutWrapper, geometryNode, menuOffsetX);
    OffsetF menuOffset(menuOffsetX, menuOffsetY);
    geometryNode->SetMarginFrameOffset(menuOffset);
    menuWrapper->Layout();
}

// set variables from theme
void TitleBarLayoutAlgorithm::InitializeTheme()
{
    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    maxPaddingStart_ = theme->GetMaxPaddingStart();
    maxPaddingEnd_ = theme->GetMaxPaddingEnd();
    menuHeight_ = theme->GetHeight().ConvertToPx();
    defaultPaddingStart_ = theme->GetDefaultPaddingStart();
    iconSize_ = theme->GetMenuIconSize();
    titleFontSize_ = theme->GetTitleFontSize();
    marginLeft_ = theme->GetMarginLeft();
    marginRight_ = theme->GetMarginRight();
    menuCompPadding_ = theme->GetCompPadding();
    iconBackgroundWidth_ = theme->GetIconBackgroundWidth();
    backButtonWidth_ = theme->GetBackButtonWidth();
    backButtonHeight_ = theme->GetBackButtonHeight();
    paddingTopTwolines_ = theme->GetPaddingTopTwolines();
    titleSpaceVertical_ = theme->GetTitleSpaceVertical();
    backIconWidth_ = theme->GetIconWidth();
    backIconHeight_ = theme->GetIconHeight();
    singleLineTitleHeight_ = static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    doubleLineTitleBarHeight_ = static_cast<float>(DOUBLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    navTitleSpaceVertical_ = 0.0f;
    navLeftMargin_ = maxPaddingStart_.ConvertToPx();
    navBackIconWidth_ = BACK_BUTTON_ICON_SIZE.ConvertToPx();
    navButtonPadding_ = BUTTON_PADDING.ConvertToPx();
    navHorizontalMargin_ = NAV_HORIZONTAL_MARGIN_L.ConvertToPx();
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        doubleLineTitleBarHeight_ = static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
        navTitleSpaceVertical_ = static_cast<float>(titleSpaceVertical_.ConvertToPx());
        navLeftMargin_ = marginLeft_.ConvertToPx();
        navBackIconWidth_ = backIconWidth_.ConvertToPx();
        navButtonPadding_ = (MENU_BUTTON_PADDING + MENU_BUTTON_PADDING).ConvertToPx();
        navHorizontalMargin_ = navButtonPadding_ + menuCompPadding_.ConvertToPx();
    }
}

void TitleBarLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(layoutWrapper->GetHostNode());
    CHECK_NULL_VOID(titleBarNode);
    auto layoutProperty = AceType::DynamicCast<TitleBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    const auto& constraint = layoutProperty->GetLayoutConstraint();
    CHECK_NULL_VOID(constraint);
    auto titlePattern = titleBarNode->GetPattern<TitleBarPattern>();
    auto size = CreateIdealSize(constraint.value(), Axis::HORIZONTAL, MeasureType::MATCH_PARENT, true);
    const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    InitializeTheme();
    do {
        if (layoutProperty->GetTitleBarParentTypeValue(TitleBarParentType::NAVBAR) !=
        TitleBarParentType::NAV_DESTINATION) {
            break;
        }
        auto navDestinationNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetParent());
        CHECK_NULL_BREAK(navDestinationNode);
        auto navDestinationPattern = AceType::DynamicCast<NavDestinationPattern>(navDestinationNode->GetPattern());
        CHECK_NULL_BREAK(navDestinationPattern);
        showBackButton_ = navDestinationPattern->GetBackButtonState();
    } while (false);
    MeasureBackButton(layoutWrapper, titleBarNode, layoutProperty);
    MeasureMenu(layoutWrapper, titleBarNode, layoutProperty);
    auto titleMaxWidth = GetTitleWidth(titleBarNode, layoutProperty, size);
    titleMaxWidth = WidthAfterAvoidMenubar(titleBarNode, titleMaxWidth);
    MeasureSubtitle(layoutWrapper, titleBarNode, layoutProperty, size, titleMaxWidth);
    MeasureTitle(layoutWrapper, titleBarNode, layoutProperty, size, titleMaxWidth);
    titlePattern->SetCurrentTitleBarHeight(size.Height());
    layoutWrapper->GetGeometryNode()->SetFrameSize(size);
}

void TitleBarLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    if (pipeline && pipeline->GetInstallationFree()) {
        //TitleBar run measure again during Layout in atomic service for avoiding menuBar
        Measure(layoutWrapper);
    }
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(layoutWrapper->GetHostNode());
    CHECK_NULL_VOID(titleBarNode);
    auto layoutProperty = AceType::DynamicCast<TitleBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    LayoutBackButton(layoutWrapper, titleBarNode, layoutProperty);

    float subtitleHeight = 0.0f;
    auto subtitleNode = titleBarNode->GetSubtitle();
    if (subtitleNode) {
        auto index = titleBarNode->GetChildIndexById(subtitleNode->GetId());
        auto subtitleWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        CHECK_NULL_VOID(subtitleWrapper);
        auto geometryNode = subtitleWrapper->GetGeometryNode();
        subtitleHeight = geometryNode->GetFrameSize().Height();
    }
    LayoutTitle(layoutWrapper, titleBarNode, layoutProperty, subtitleHeight);

    float titleHeight = 0.0f;
    auto titleNode = titleBarNode->GetTitle();
    if (titleNode) {
        auto index = titleBarNode->GetChildIndexById(titleNode->GetId());
        auto titleWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        CHECK_NULL_VOID(titleWrapper);
        auto geometryNode = titleWrapper->GetGeometryNode();
        titleHeight = geometryNode->GetFrameSize().Height();
    }
    LayoutSubtitle(layoutWrapper, titleBarNode, layoutProperty, titleHeight);

    LayoutMenu(layoutWrapper, titleBarNode, layoutProperty, subtitleHeight);
}

float TitleBarLayoutAlgorithm::ChangeOffsetByDirection(LayoutWrapper* layoutWrapper,
    const RefPtr<NG::GeometryNode>& childGeometryNode, float offsetX) const
{
    CHECK_NULL_RETURN(layoutWrapper, offsetX);
    CHECK_NULL_RETURN(childGeometryNode, offsetX);
    if (AceApplicationInfo::GetInstance().IsRightToLeft()) {
        auto geometryNode = layoutWrapper->GetGeometryNode();
        CHECK_NULL_RETURN(geometryNode, offsetX);
        auto parentWidth = geometryNode->GetFrameSize().Width();
        offsetX = parentWidth - offsetX - childGeometryNode->GetFrameSize().Width();
    }
    return offsetX;
}
} // namespace OHOS::Ace::NG
