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

#include "core/components_ng/pattern/navigation/nav_bar_layout_algorithm.h"

#include "core/components/common/layout/grid_system_manager.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/navigation/nav_bar_layout_property.h"
#include "core/components_ng/pattern/navigation/nav_bar_node.h"
#include "core/components_ng/pattern/navigation/nav_bar_pattern.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/pattern/navigation/title_bar_pattern.h"
#include "core/components_ng/pattern/navigation/tool_bar_node.h"

namespace OHOS::Ace::NG {
namespace {
float MeasureTitleBar(LayoutWrapper* layoutWrapper, const RefPtr<NavBarNode>& hostNode,
    const RefPtr<NavBarLayoutProperty>& navBarLayoutProperty, const SizeF& navigationSize)
{
    CHECK_NULL_RETURN(hostNode, 0.0f);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_RETURN(titleBarNode, 0.0f);
    auto index = hostNode->GetChildIndexById(titleBarNode->GetId());
    auto titleBarWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_RETURN(titleBarWrapper, 0.0f);
    auto constraint = navBarLayoutProperty->CreateChildConstraint();
    if (navBarLayoutProperty->GetHideTitleBar().value_or(false)) {
        constraint.selfIdealSize = OptionalSizeF(0.0f, 0.0f);
        titleBarWrapper->Measure(constraint);
        return 0.0f;
    }
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_RETURN(titleBarLayoutProperty, 0.0f);
    if (titleBarLayoutProperty->HasTitleHeight()) {
        auto titleHeight =
            titleBarLayoutProperty->GetTitleHeightValue().ConvertToPxWithSize(constraint.percentReference.Height());
        constraint.selfIdealSize.SetHeight(static_cast<float>(titleHeight));
        titleBarWrapper->Measure(constraint);
        return static_cast<float>(titleHeight);
    }

    auto singleLineTitleHeight = static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    auto doubleLineTitleBarHeight = static_cast<float>(DOUBLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        doubleLineTitleBarHeight = static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    }
    // MINI mode
    if (navBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::MINI) {
        if (titleBarNode->GetSubtitle()) {
            constraint.selfIdealSize =
                OptionalSizeF(navigationSize.Width(), doubleLineTitleBarHeight);
            titleBarWrapper->Measure(constraint);
            return doubleLineTitleBarHeight;
        }

        constraint.selfIdealSize =
            OptionalSizeF(navigationSize.Width(), singleLineTitleHeight);
        titleBarWrapper->Measure(constraint);
        return singleLineTitleHeight;
    }

    float titleBarHeight = 0.0f;
    if (navBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::FREE) {
        auto titlePattern = titleBarNode->GetPattern<TitleBarPattern>();
        CHECK_NULL_RETURN(titlePattern, 0.0f);
        titleBarHeight = titlePattern->GetTempTitleBarHeight();
    }

    // FREE 和 FULL 模式，有subtitle
    if (titleBarNode->GetSubtitle()) {
        if (NearZero(titleBarHeight)) {
            titleBarHeight = static_cast<float>(FULL_DOUBLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
        }
        constraint.selfIdealSize = OptionalSizeF(navigationSize.Width(), titleBarHeight);
        titleBarWrapper->Measure(constraint);
        return titleBarHeight;
    }

    // no subtitle
    if (NearZero(titleBarHeight)) {
        titleBarHeight = static_cast<float>(FULL_SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    }
    constraint.selfIdealSize = OptionalSizeF(navigationSize.Width(), titleBarHeight);
    titleBarWrapper->Measure(constraint);
    return titleBarHeight;
}

MarginPropertyF GetNavigationMargin(const RefPtr<NavBarNode>& hostNode)
{
    CHECK_NULL_RETURN(hostNode, MarginPropertyF());
    auto parent = AceType::DynamicCast<NavigationGroupNode>(hostNode->GetParent());
    CHECK_NULL_RETURN(parent, MarginPropertyF());
    auto layoutProperty = parent->GetLayoutProperty();
    CHECK_NULL_RETURN(layoutProperty,  MarginPropertyF());
    const auto& margin = layoutProperty->CreateMargin();
    return margin;
}

bool CheckTopEdgeOverlap(const RefPtr<NavBarLayoutProperty>& navBarLayoutProperty,
    const RefPtr<NavBarNode>& hostNode, SafeAreaExpandOpts opts)
{
    if (!navBarLayoutProperty || !hostNode) {
        return false;
    }
    const auto& margin = GetNavigationMargin(hostNode);
    float topMargin = margin.top.value_or(0.0f);
    const auto& padding = navBarLayoutProperty->CreatePaddingAndBorder();
    float topPadding = padding.top.value_or(0.0f);
    if (!NearEqual(topPadding, 0.0f) || !NearEqual(topMargin, 0.0f)) {
        return false;
    }

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto safeAreaManager = pipeline->GetSafeAreaManager();

    auto parentGlobalOffset = hostNode->GetParentGlobalOffsetDuringLayout();
    auto NavBarGeometryNode = hostNode->GetGeometryNode();
    CHECK_NULL_RETURN(NavBarGeometryNode, false);
    auto frame = NavBarGeometryNode->GetFrameRect() + parentGlobalOffset;

    if ((opts.edges & SAFE_AREA_EDGE_TOP) && (opts.type & SAFE_AREA_TYPE_SYSTEM)) {
        SafeAreaExpandOpts opts = {.type = SAFE_AREA_TYPE_SYSTEM, .edges = SAFE_AREA_EDGE_TOP};
        auto safeAreaPos = safeAreaManager->GetCombinedSafeArea(opts);

        auto titleBarNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
        CHECK_NULL_RETURN(titleBarNode, false);
        auto titlePattern = titleBarNode->GetPattern<TitleBarPattern>();
        CHECK_NULL_RETURN(titlePattern, false);
        auto options = titlePattern->GetTitleBarOptions();
        auto barStyle = options.bgOptions.barStyle.value_or(BarStyle::STANDARD);
        if ((navBarLayoutProperty->GetHideTitleBar().value_or(false) || barStyle == BarStyle::STACK) &&
            safeAreaPos.top_.IsOverlapped(frame.Top())) {
            return true;
        }
    }
    return false;
}

bool CheckBottomEdgeOverlap(const RefPtr<NavBarLayoutProperty>& navBarLayoutProperty,
    const RefPtr<NavBarNode>& hostNode, SafeAreaExpandOpts opts)
{
    if (!navBarLayoutProperty || !hostNode) {
        return false;
    }
    const auto& margin = GetNavigationMargin(hostNode);
    float bottomMargin = margin.bottom.value_or(0.0f);
    const auto& padding = navBarLayoutProperty->CreatePaddingAndBorder();
    float bottomPadding = padding.bottom.value_or(0.0f);
    if (!NearEqual(bottomPadding, 0.0f) || !NearEqual(bottomMargin, 0.0f)) {
        return false;
    }

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto safeAreaManager = pipeline->GetSafeAreaManager();

    auto parentGlobalOffset = hostNode->GetParentGlobalOffsetDuringLayout();
    auto NavBarGeometryNode = hostNode->GetGeometryNode();
    CHECK_NULL_RETURN(NavBarGeometryNode, false);
    auto frame = NavBarGeometryNode->GetFrameRect() + parentGlobalOffset;

    if ((opts.edges & SAFE_AREA_EDGE_BOTTOM) && (opts.type & SAFE_AREA_TYPE_SYSTEM)) {
        SafeAreaExpandOpts opts = {.type = SAFE_AREA_TYPE_SYSTEM, .edges = SAFE_AREA_EDGE_BOTTOM};
        auto safeAreaPos = safeAreaManager->GetCombinedSafeArea(opts);
        if (safeAreaPos.bottom_.IsOverlapped(frame.Bottom())
            && navBarLayoutProperty->GetHideToolBar().value_or(false)) {
            return true;
        }
    }
    return false;
}

NavSafeArea CheckIgnoreLayoutSafeArea(LayoutWrapper* layoutWrapper, const RefPtr<NavBarNode>& hostNode,
    const RefPtr<NavBarLayoutProperty>& navBarLayoutProperty)
{
    NavSafeArea safeArea;
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, safeArea);
    auto inset = pipeline->GetSafeArea();
    auto safeAreaManager = pipeline->GetSafeAreaManager();

    auto opts = navBarLayoutProperty->GetIgnoreLayoutSafeAreaValue({.type = SAFE_AREA_TYPE_NONE,
        .edges = SAFE_AREA_TYPE_NONE});
    bool edgeTopOverLayCondition = CheckTopEdgeOverlap(navBarLayoutProperty, hostNode, opts);
    bool edgeBottomOverLayCondition = CheckBottomEdgeOverlap(navBarLayoutProperty, hostNode, opts);

    if (edgeTopOverLayCondition) {
        auto navBarContentNode = AceType::DynamicCast<FrameNode>(hostNode->GetNavBarContentNode());
        SafeAreaExpandOpts topParam = {.type = SAFE_AREA_TYPE_SYSTEM, .edges = SAFE_AREA_EDGE_NONE};
        navBarContentNode->GetLayoutProperty()->UpdateSafeAreaExpandOpts(topParam);

        if (navBarLayoutProperty->GetHideToolBar().value_or(false)) {
            auto navBarContentRenderContext = navBarContentNode->GetRenderContext();
            navBarContentRenderContext->UpdateClipEdge(false);
        }
        safeArea.top = static_cast<float>(inset.top_.Length());
    }

    if (edgeBottomOverLayCondition) {
        safeArea.bottom = static_cast<float>(inset.bottom_.Length());
    }
    return safeArea;
}

bool CheckWhetherNeedToHideToolbar(const RefPtr<NavBarNode>& hostNode, const SizeF& navigationSize)
{
    if (hostNode->GetPrevMenuIsCustomValue(false)) {
        return false;
    }

    auto toolbarNode = AceType::DynamicCast<NavToolbarNode>(hostNode->GetToolBarNode());
    CHECK_NULL_RETURN(toolbarNode, false);
    if (!toolbarNode->HasValidContent()) {
        return true;
    }

    auto theme = NavigationGetTheme();
    CHECK_NULL_RETURN(theme, false);
    auto rotationLimitCount = theme->GetToolbarRotationLimitGridCount();

    RefPtr<GridColumnInfo> columnInfo;
    columnInfo = GridSystemManager::GetInstance().GetInfoByType(GridColumnType::NAVIGATION_TOOLBAR);
    columnInfo->GetParent()->BuildColumnWidth();

    auto currentColumns = columnInfo->GetParent()->GetColumns();
    float gridWidth = static_cast<float>(columnInfo->GetWidth(rotationLimitCount));
    float gutterWidth = columnInfo->GetParent()->GetGutterWidth().ConvertToPx();
    float hideLimitWidth = gridWidth + gutterWidth * 2;
    if (SystemProperties::GetDeviceType() == DeviceType::PHONE) {
        if (currentColumns >= static_cast<int32_t>(rotationLimitCount) &&
            GreatOrEqual(navigationSize.Width(), gridWidth)) {
            return true;
        }
    } else if (SystemProperties::GetDeviceType() == DeviceType::TABLET) {
        if (currentColumns > static_cast<int32_t>(rotationLimitCount) &&
            GreatNotEqual(navigationSize.Width(), hideLimitWidth)) {
            return true;
        }
    }
    return false;
}

float MeasureToolBar(LayoutWrapper* layoutWrapper, const RefPtr<NavBarNode>& hostNode,
    const RefPtr<NavBarLayoutProperty>& navBarLayoutProperty, const SizeF& navigationSize)
{
    auto toolBarNode = hostNode->GetToolBarNode();
    CHECK_NULL_RETURN(toolBarNode, 0.0f);
    auto index = hostNode->GetChildIndexById(toolBarNode->GetId());
    auto toolBarWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_RETURN(toolBarWrapper, 0.0f);
    auto constraint = navBarLayoutProperty->CreateChildConstraint();

    if (navBarLayoutProperty->GetHideToolBar().value_or(false) || toolBarNode->GetChildren().empty() ||
        CheckWhetherNeedToHideToolbar(hostNode, navigationSize)) {
        constraint.selfIdealSize = OptionalSizeF(0.0f, 0.0f);
        toolBarWrapper->Measure(constraint);
        return 0.0f;
    }

    auto theme = NavigationGetTheme();
    CHECK_NULL_RETURN(theme, 0.0f);
    auto toolbarHeight = theme->GetHeight();
    constraint.selfIdealSize = OptionalSizeF(navigationSize.Width(), static_cast<float>(toolbarHeight.ConvertToPx()));
    toolBarWrapper->Measure(constraint);
    auto toolbarHeightAfterMeasure = toolBarWrapper->GetGeometryNode()->GetFrameSize().Height();
    return static_cast<float>(toolbarHeightAfterMeasure);
}

float MeasureToolBarDivider(LayoutWrapper* layoutWrapper, const RefPtr<NavBarNode>& hostNode,
    const RefPtr<NavBarLayoutProperty>& navBarLayoutProperty, const SizeF& navigationSize, float toolBarHeight)
{
    if (hostNode->GetPrevToolBarIsCustom().value_or(false) || !hostNode->IsNavbarUseToolbarConfiguration()) {
        return 0.0f;
    }

    auto toolBarDividerNode = hostNode->GetToolBarDividerNode();
    CHECK_NULL_RETURN(toolBarDividerNode, 0.0f);
    auto dividerIndex = hostNode->GetChildIndexById(toolBarDividerNode->GetId());
    auto dividerWrapper = layoutWrapper->GetOrCreateChildByIndex(dividerIndex);
    CHECK_NULL_RETURN(dividerWrapper, 0.0f);
    auto constraint = navBarLayoutProperty->CreateChildConstraint();

    if (navBarLayoutProperty->GetHideToolBar().value_or(false) || NearEqual(toolBarHeight, 0.0f)) {
        constraint.selfIdealSize = OptionalSizeF(0.0f, 0.0f);
        dividerWrapper->Measure(constraint);
        return 0.0f;
    }
    auto theme = NavigationGetTheme();
    CHECK_NULL_RETURN(theme, 0.0f);
    constraint.selfIdealSize =
        OptionalSizeF(navigationSize.Width(), static_cast<float>(theme->GetToolBarDividerWidth().ConvertToPx()));
    dividerWrapper->Measure(constraint);
    return static_cast<float>(theme->GetToolBarDividerWidth().ConvertToPx());
}

float MeasureContentChild(LayoutWrapper* layoutWrapper, const RefPtr<NavBarNode>& hostNode,
    const RefPtr<NavBarLayoutProperty>& navBarLayoutProperty, const SizeF& navigationSize, float titleBarHeight,
    float toolBarHeight, float toolBarDividerHeight)
{
    auto contentNode = hostNode->GetNavBarContentNode();
    CHECK_NULL_RETURN(contentNode, 0.0f);
    auto index = hostNode->GetChildIndexById(contentNode->GetId());
    auto contentWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_RETURN(contentWrapper, 0.0f);
    auto constraint = navBarLayoutProperty->CreateChildConstraint();
    float contentHeight = navigationSize.Height() - titleBarHeight - toolBarHeight - toolBarDividerHeight;
    if (NavigationLayoutAlgorithm::IsAutoHeight(navBarLayoutProperty)) {
        constraint.selfIdealSize.SetWidth(navigationSize.Width());
        contentWrapper->Measure(constraint);
        return static_cast<float>(contentWrapper->GetGeometryNode()->GetFrameSize().Height());
    } else {
        constraint.selfIdealSize = OptionalSizeF(navigationSize.Width(), contentHeight);
    }
    auto safeArea = CheckIgnoreLayoutSafeArea(layoutWrapper, hostNode, navBarLayoutProperty);
    auto currentHeight = static_cast<float>(constraint.selfIdealSize.Height().value());
    constraint.selfIdealSize.SetHeight(currentHeight + safeArea.top + safeArea.bottom);
    contentWrapper->Measure(constraint);
    return currentHeight;
}

float LayoutTitleBar(LayoutWrapper* layoutWrapper, const RefPtr<NavBarNode>& hostNode,
    const RefPtr<NavBarLayoutProperty>& navBarLayoutProperty)
{
    if (navBarLayoutProperty->GetHideTitleBar().value_or(false)) {
        return 0.0f;
    }
    auto titleBarNode = hostNode->GetTitleBarNode();
    CHECK_NULL_RETURN(titleBarNode, 0.0f);
    auto index = hostNode->GetChildIndexById(titleBarNode->GetId());
    auto titleBarWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_RETURN(titleBarWrapper, 0.0f);
    auto geometryNode = titleBarWrapper->GetGeometryNode();
    auto titleBarOffset = OffsetF(0.0f, 0.0f);
    geometryNode->SetMarginFrameOffset(titleBarOffset);
    titleBarWrapper->Layout();
    return geometryNode->GetFrameSize().Height();
}

void LayoutContent(LayoutWrapper* layoutWrapper, const RefPtr<NavBarNode>& hostNode,
    const RefPtr<NavBarLayoutProperty>& navBarLayoutProperty, float titlebarHeight)
{
    auto contentNode = hostNode->GetNavBarContentNode();
    CHECK_NULL_VOID(contentNode);
    auto index = hostNode->GetChildIndexById(hostNode->GetNavBarContentNode()->GetId());
    auto contentWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_VOID(contentWrapper);
    auto geometryNode = contentWrapper->GetGeometryNode();
    auto pattern = hostNode->GetPattern<NavBarPattern>();
    float avoidKeyboardOffset = pattern ? pattern->GetAvoidKeyboardOffset() : 0.0f;
    auto contentOffset = OffsetF(0.0f, avoidKeyboardOffset);
    if (!navBarLayoutProperty->GetHideTitleBar().value_or(false)) {
        contentOffset += OffsetF(geometryNode->GetFrameOffset().GetX(), titlebarHeight);
    }
    auto safeArea = CheckIgnoreLayoutSafeArea(layoutWrapper, hostNode, navBarLayoutProperty);
    auto offsetY = contentOffset.GetY();
    auto opts = navBarLayoutProperty->GetIgnoreLayoutSafeAreaValue({.type = SAFE_AREA_TYPE_NONE,
        .edges = SAFE_AREA_TYPE_NONE});
    if ((opts.edges & SAFE_AREA_EDGE_TOP) && NearEqual(offsetY, 0.0f)) {
        offsetY -= safeArea.top;
        contentOffset.SetY(offsetY);
    }
    geometryNode->SetMarginFrameOffset(contentOffset);
    contentWrapper->Layout();
}

float LayoutToolBar(LayoutWrapper* layoutWrapper, const RefPtr<NavBarNode>& hostNode,
    const RefPtr<NavBarLayoutProperty>& navBarLayoutProperty)
{
    if (navBarLayoutProperty->GetHideToolBar().value_or(false)) {
        return 0.0f;
    }
    auto toolBarNode = hostNode->GetToolBarNode();
    CHECK_NULL_RETURN(toolBarNode, 0.0f);
    auto index = hostNode->GetChildIndexById(toolBarNode->GetId());
    auto toolBarWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_RETURN(toolBarWrapper, 0.0f);
    auto geometryNode = toolBarWrapper->GetGeometryNode();
    float toolbarHeight = geometryNode->GetFrameSize().Height();
    if (NearZero(toolbarHeight)) {
        return 0.0f;
    }

    auto toolBarOffsetY = layoutWrapper->GetGeometryNode()->GetFrameSize().Height() - toolbarHeight;
    auto toolBarOffset = OffsetF(geometryNode->GetFrameOffset().GetX(), static_cast<float>(toolBarOffsetY));
    geometryNode->SetMarginFrameOffset(toolBarOffset);
    toolBarWrapper->Layout();
    return toolbarHeight;
}

void LayoutToolBarDivider(LayoutWrapper* layoutWrapper, const RefPtr<NavBarNode>& hostNode,
    const RefPtr<NavBarLayoutProperty>& navBarLayoutProperty, float toolbarHeight)
{
    if (navBarLayoutProperty->GetHideToolBar().value_or(false) || hostNode->GetPrevToolBarIsCustom().value_or(false) ||
        !hostNode->IsNavbarUseToolbarConfiguration() || NearZero(toolbarHeight)) {
        return;
    }
    auto dividerNode = hostNode->GetToolBarDividerNode();
    CHECK_NULL_VOID(dividerNode);
    auto dividerIndex = hostNode->GetChildIndexById(dividerNode->GetId());
    auto dividerWrapper = layoutWrapper->GetOrCreateChildByIndex(dividerIndex);
    CHECK_NULL_VOID(dividerWrapper);
    auto dividerGeometryNode = dividerWrapper->GetGeometryNode();

    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    auto dividerOffsetY = layoutWrapper->GetGeometryNode()->GetFrameSize().Height() - toolbarHeight -
                          theme->GetToolBarDividerWidth().ConvertToPx();
    auto toolBarDividerOffset =
        OffsetF(dividerGeometryNode->GetFrameOffset().GetX(), static_cast<float>(dividerOffsetY));
    dividerGeometryNode->SetFrameOffset(toolBarDividerOffset);
    dividerWrapper->Layout();
}

void UpdateTitleBarMenuNode(const RefPtr<NavBarNode>& navBarNode, const SizeF& navigationSize)
{
    if (navBarNode->GetPrevMenuIsCustomValue(false)) {
        return;
    }

    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto toolBarNode = AceType::DynamicCast<FrameNode>(navBarNode->GetToolBarNode());
    CHECK_NULL_VOID(toolBarNode);
    auto toolBarLayoutProperty = toolBarNode->GetLayoutProperty<LayoutProperty>();
    CHECK_NULL_VOID(toolBarLayoutProperty);
    auto navBarPattern = AceType::DynamicCast<NavBarPattern>(navBarNode->GetPattern());
    auto isHideToolbar = navBarPattern->GetToolbarHideStatus();

    if (titleBarNode->GetMenu()) {
        titleBarNode->RemoveChild(titleBarNode->GetMenu());
    }

    if (CheckWhetherNeedToHideToolbar(navBarNode, navigationSize) && !isHideToolbar) {
        toolBarLayoutProperty->UpdateVisibility(VisibleType::GONE);
        titleBarNode->SetMenu(navBarNode->GetLandscapeMenu());
    } else {
        if (!isHideToolbar) {
            toolBarLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
        }
        titleBarNode->SetMenu(navBarNode->GetMenu());
    }
    titleBarNode->AddChild(titleBarNode->GetMenu());
}

float TransferTitleBarHeight(const RefPtr<NavBarNode>& hostNode, float titleBarHeight)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_RETURN(titleBarNode, 0.0f);
    auto titlePattern = titleBarNode->GetPattern<TitleBarPattern>();
    CHECK_NULL_RETURN(titlePattern, 0.0f);
    auto resetTitleBarHeight = 0.0f;
    auto options = titlePattern->GetTitleBarOptions();
    auto barStyle = options.bgOptions.barStyle.value_or(BarStyle::STANDARD);
    if (barStyle == BarStyle::STACK) {
        resetTitleBarHeight = 0.0f;
    } else {
        resetTitleBarHeight = titleBarHeight;
    }
    return resetTitleBarHeight;
}
} // namespace

void NavBarLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto hostNode = AceType::DynamicCast<NavBarNode>(layoutWrapper->GetHostNode());
    CHECK_NULL_VOID(hostNode);
    auto navBarLayoutProperty = AceType::DynamicCast<NavBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(navBarLayoutProperty);
    const auto& constraint = navBarLayoutProperty->GetLayoutConstraint();
    CHECK_NULL_VOID(constraint);
    auto geometryNode = layoutWrapper->GetGeometryNode();
    auto size = CreateIdealSize(constraint.value(), Axis::HORIZONTAL, MeasureType::MATCH_PARENT, true);
    const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    UpdateTitleBarMenuNode(hostNode, size);
    float titleBarHeight = MeasureTitleBar(layoutWrapper, hostNode, navBarLayoutProperty, size);
    auto resetTitleBarHeight = TransferTitleBarHeight(hostNode, titleBarHeight);
    float toolBarHeight = MeasureToolBar(layoutWrapper, hostNode, navBarLayoutProperty, size);
    float toolBarDividerHeight =
        MeasureToolBarDivider(layoutWrapper, hostNode, navBarLayoutProperty, size, toolBarHeight);
    float contentChildHeight = MeasureContentChild(
        layoutWrapper, hostNode, navBarLayoutProperty, size, resetTitleBarHeight, toolBarHeight, toolBarDividerHeight);
    size.SetHeight(resetTitleBarHeight + toolBarHeight + toolBarDividerHeight + contentChildHeight);
    layoutWrapper->GetGeometryNode()->SetFrameSize(size);
}

void NavBarLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    auto hostNode = AceType::DynamicCast<NavBarNode>(layoutWrapper->GetHostNode());
    CHECK_NULL_VOID(hostNode);
    auto navBarLayoutProperty = AceType::DynamicCast<NavBarLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(navBarLayoutProperty);

    auto opts = navBarLayoutProperty->GetIgnoreLayoutSafeAreaValue({.type = SAFE_AREA_TYPE_NONE,
        .edges = SAFE_AREA_TYPE_NONE});
    bool edgeTopOverLayCondition = CheckTopEdgeOverlap(navBarLayoutProperty, hostNode, opts);
    bool edgeBottomOverLayCondition = CheckBottomEdgeOverlap(navBarLayoutProperty, hostNode, opts);
    if (edgeTopOverLayCondition || edgeBottomOverLayCondition) {
        Measure(layoutWrapper);
    }

    float titlebarHeight = LayoutTitleBar(layoutWrapper, hostNode, navBarLayoutProperty);
    auto resetTitleBarHeight = TransferTitleBarHeight(hostNode, titlebarHeight);
    LayoutContent(layoutWrapper, hostNode, navBarLayoutProperty, resetTitleBarHeight);
    float toolbarHeight = LayoutToolBar(layoutWrapper, hostNode, navBarLayoutProperty);
    LayoutToolBarDivider(layoutWrapper, hostNode, navBarLayoutProperty, toolbarHeight);
}
} // namespace OHOS::Ace::NG
