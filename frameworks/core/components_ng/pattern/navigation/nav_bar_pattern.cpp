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

#include "core/components_ng/pattern/navigation/nav_bar_pattern.h"

#include <algorithm>

#include "base/i18n/localization.h"
#include "core/common/container.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/bubble/bubble_pattern.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/grid/grid_pattern.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/navigation/bar_item_event_hub.h"
#include "core/components_ng/pattern/navigation/bar_item_pattern.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "core/components_ng/pattern/navigation/navigation_title_util.h"
#include "core/components_ng/pattern/navigation/title_bar_pattern.h"
#include "core/components_ng/pattern/navigation/tool_bar_node.h"
#include "core/components_ng/pattern/text/text_pattern.h"

namespace OHOS::Ace::NG {
namespace {
// titlebar ZINDEX
constexpr static int32_t DEFAULT_TITLEBAR_ZINDEX = 2;
constexpr float DEFAULT_NAV_BAR_MASK_OPACITY = 0.6f;
void BuildMoreItemNodeAction(const RefPtr<FrameNode>& buttonNode, const RefPtr<BarItemNode>& barItemNode,
    const RefPtr<FrameNode>& barMenuNode, const RefPtr<NavBarNode>& navBarNode)
{
    auto eventHub = barItemNode->GetEventHub<BarItemEventHub>();
    CHECK_NULL_VOID(eventHub);

    auto context = PipelineContext::GetCurrentContext();
    auto clickCallback = [weakContext = WeakPtr<PipelineContext>(context), id = barItemNode->GetId(),
                             weakMenu = WeakPtr<FrameNode>(barMenuNode),
                             weakBarItemNode = WeakPtr<BarItemNode>(barItemNode),
                             weakNavBarNode = WeakPtr<NavBarNode>(navBarNode)]() {
        auto context = weakContext.Upgrade();
        CHECK_NULL_VOID(context);

        auto overlayManager = context->GetOverlayManager();
        CHECK_NULL_VOID(overlayManager);

        auto menu = weakMenu.Upgrade();
        CHECK_NULL_VOID(menu);

        auto barItemNode = weakBarItemNode.Upgrade();
        CHECK_NULL_VOID(barItemNode);

        auto menuNode = AceType::DynamicCast<FrameNode>(menu->GetChildAtIndex(0));
        CHECK_NULL_VOID(menuNode);

        auto menuPattern = menuNode->GetPattern<MenuPattern>();
        CHECK_NULL_VOID(menuPattern);

        auto navBarNode = weakNavBarNode.Upgrade();
        CHECK_NULL_VOID(navBarNode);

        auto navBarPattern = navBarNode->GetPattern<NavBarPattern>();
        CHECK_NULL_VOID(navBarPattern);

        // navigation menu show like select.
        menuPattern->SetIsSelectMenu(true);
        OffsetF offset(0.0f, 0.0f);
        if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            auto symbol = AceType::DynamicCast<FrameNode>(barItemNode->GetChildren().front());
            CHECK_NULL_VOID(symbol);
            auto symbolProperty = symbol->GetLayoutProperty<TextLayoutProperty>();
            CHECK_NULL_VOID(symbolProperty);
            auto symbolEffectOptions = symbolProperty->GetSymbolEffectOptionsValue(SymbolEffectOptions());
            symbolEffectOptions.SetEffectType(SymbolEffectType::BOUNCE);
            symbolEffectOptions.SetIsTxtActive(true);
            symbolEffectOptions.SetIsTxtActiveSource(0);
            symbolProperty->UpdateSymbolEffectOptions(symbolEffectOptions);
            symbol->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        } else {
            offset = navBarPattern->GetShowMenuOffset(barItemNode, menuNode);
        }
        overlayManager->ShowMenu(id, offset, menu);
        navBarNode->SetIsTitleMenuNodeShowing(true);
        auto hidMenuCallback = [weakNavBarNode = WeakPtr<NavBarNode>(navBarNode)]() {
            auto navBarNode = weakNavBarNode.Upgrade();
            CHECK_NULL_VOID(navBarNode);
            navBarNode->SetIsTitleMenuNodeShowing(false);
        };
        auto menuWrapperPattern = menuNode->GetPattern<MenuWrapperPattern>();
        CHECK_NULL_VOID(menuWrapperPattern);
        menuWrapperPattern->RegisterMenuDisappearCallback(hidMenuCallback);
    };
    eventHub->SetItemAction(clickCallback);

    auto gestureEventHub = buttonNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureEventHub);
    auto callback = [action = clickCallback](GestureEvent& info) {
        if (info.GetSourceDevice() == SourceType::KEYBOARD) {
            return;
        }
        action();
    };
    gestureEventHub->AddClickEvent(AceType::MakeRefPtr<ClickEvent>(callback));
}

RefPtr<FrameNode> CreateMenuItems(const int32_t menuNodeId, const std::vector<NG::BarItem>& menuItems,
    RefPtr<NavBarNode> navBarNode, bool isCreateLandscapeMenu)
{
    auto menuNode = FrameNode::GetOrCreateFrameNode(
        V2::NAVIGATION_MENU_ETS_TAG, menuNodeId, []() { return AceType::MakeRefPtr<LinearLayoutPattern>(false); });
    CHECK_NULL_RETURN(menuNode, nullptr);
    menuNode->Clean();
    menuNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    auto rowProperty = menuNode->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(rowProperty, nullptr);
    rowProperty->UpdateMainAxisAlign(FlexAlign::SPACE_BETWEEN);
    auto theme = NavigationGetTheme();
    auto navBarPattern = AceType::DynamicCast<NavBarPattern>(navBarNode->GetPattern());
    auto navBarMaxNum = navBarPattern->GetMaxMenuNum();
    auto mostMenuItemCount =
        navBarMaxNum < 0 ? theme->GetMostMenuItemCountInBar() : static_cast<uint32_t>(navBarMaxNum);
    mostMenuItemCount = SystemProperties::GetDeviceOrientation() == DeviceOrientation::LANDSCAPE ? MAX_MENU_NUM_LARGE
                                                                                                  : mostMenuItemCount;
    navBarPattern->SetMaxMenuNum(mostMenuItemCount);
    bool needMoreButton = menuItems.size() > mostMenuItemCount ? true : false;

    auto frameNode = navBarNode->GetParent();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    CHECK_NULL_RETURN(navigationGroupNode, nullptr);
    auto hub = navigationGroupNode->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(hub, nullptr);
    auto isButtonEnabled = hub->IsEnabled();

    uint32_t count = 0;
    std::vector<OptionParam> params;
    for (const auto& menuItem : menuItems) {
        ++count;
        if (needMoreButton && (count > mostMenuItemCount - 1)) {
            params.push_back({ menuItem.text.value_or(""), menuItem.icon.value_or(""),
                menuItem.isEnabled.value_or(true), menuItem.action, menuItem.iconSymbol.value_or(nullptr) });
        } else {
            auto menuItemNode = NavigationTitleUtil::CreateMenuItemButton(theme);
            int32_t barItemNodeId = ElementRegister::GetInstance()->MakeUniqueId();
            auto barItemNode = AceType::MakeRefPtr<BarItemNode>(V2::BAR_ITEM_ETS_TAG, barItemNodeId);
            barItemNode->InitializePatternAndContext();
            NavigationTitleUtil::UpdateBarItemNodeWithItem(barItemNode, menuItem, isButtonEnabled);
            auto barItemLayoutProperty = barItemNode->GetLayoutProperty();
            CHECK_NULL_RETURN(barItemLayoutProperty, nullptr);
            barItemLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);

            auto iconNode = AceType::DynamicCast<FrameNode>(barItemNode->GetChildren().front());
            NavigationTitleUtil::InitTitleBarButtonEvent(
                menuItemNode, iconNode, false, menuItem, menuItem.isEnabled.value_or(true));
            barItemNode->MountToParent(menuItemNode);
            barItemNode->MarkModifyDone();
            menuItemNode->MarkModifyDone();
            menuNode->AddChild(menuItemNode);
        }
    }

    // build more button
    if (needMoreButton) {
        int32_t barItemNodeId = ElementRegister::GetInstance()->MakeUniqueId();
        auto barItemNode = AceType::MakeRefPtr<BarItemNode>(V2::BAR_ITEM_ETS_TAG, barItemNodeId);
        barItemNode->InitializePatternAndContext();
        auto barItemLayoutProperty = barItemNode->GetLayoutProperty();
        CHECK_NULL_RETURN(barItemLayoutProperty, nullptr);
        barItemLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);
        NavigationTitleUtil::BuildMoreIemNode(barItemNode, isButtonEnabled);
        auto menuItemNode = NavigationTitleUtil::CreateMenuItemButton(theme);
        MenuParam menuParam;
        menuParam.isShowInSubWindow = false;
        auto targetId = barItemNode->GetId();
        auto targetTag = barItemNode->GetTag();
        if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            menuParam.placement = Placement::BOTTOM_RIGHT;
            targetId = menuItemNode->GetId();
            targetTag = menuItemNode->GetTag();
        }
        auto barMenuNode = MenuView::Create(
            std::move(params), targetId, targetTag, MenuType::NAVIGATION_MENU, menuParam);
        BuildMoreItemNodeAction(menuItemNode, barItemNode, barMenuNode, navBarNode);
        auto iconNode = AceType::DynamicCast<FrameNode>(barItemNode->GetChildren().front());
        NavigationTitleUtil::InitTitleBarButtonEvent(menuItemNode, iconNode, true);

        barItemNode->MountToParent(menuItemNode);
        barItemNode->MarkModifyDone();
        menuItemNode->MarkModifyDone();
        menuNode->AddChild(menuItemNode);
        isCreateLandscapeMenu ? navBarNode->SetLandscapeMenuNode(barMenuNode) : navBarNode->SetMenuNode(barMenuNode);
    }

    NavigationTitleUtil::InitDragAndLongPressEvent(menuNode, menuItems);
    return menuNode;
}

void BuildMenu(const RefPtr<NavBarNode>& navBarNode, const RefPtr<TitleBarNode>& titleBarNode)
{
    if (navBarNode->GetMenuNodeOperationValue(ChildNodeOperation::NONE) == ChildNodeOperation::REPLACE) {
        titleBarNode->RemoveChild(titleBarNode->GetMenu());
        titleBarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }
    if (navBarNode->GetPrevMenuIsCustomValue(false)) {
        if (navBarNode->GetMenuNodeOperationValue(ChildNodeOperation::NONE) == ChildNodeOperation::NONE) {
            return;
        }
        titleBarNode->SetMenu(navBarNode->GetMenu());
        titleBarNode->AddChild(titleBarNode->GetMenu());
    } else {
        auto navBarPattern = navBarNode->GetPattern<NavBarPattern>();
        CHECK_NULL_VOID(navBarPattern);
        auto titleBarMenuItems = navBarPattern->GetTitleBarMenuItems();
        auto toolBarMenuItems = navBarPattern->GetToolBarMenuItems();

        if (navBarPattern->HasMenuNodeId()) {
            auto menuNode = CreateMenuItems(navBarPattern->GetMenuNodeId(), titleBarMenuItems, navBarNode, false);
            CHECK_NULL_VOID(menuNode);
            navBarNode->SetMenu(menuNode);
        }

        titleBarMenuItems.insert(titleBarMenuItems.end(), toolBarMenuItems.begin(), toolBarMenuItems.end());
        auto landscapeMenuNode =
            CreateMenuItems(navBarPattern->GetLandscapeMenuNodeId(), titleBarMenuItems, navBarNode, true);
        CHECK_NULL_VOID(landscapeMenuNode);
        navBarNode->SetLandscapeMenu(landscapeMenuNode);
    }
}

void BuildTitleBar(const RefPtr<NavBarNode>& navBarNode, const RefPtr<TitleBarNode>& titleBarNode)
{
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);

    // Update back button visibility
    auto backButtonNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetBackButton());
    if (backButtonNode) {
        auto backButtonLayoutProperty = backButtonNode->GetLayoutProperty();
        CHECK_NULL_VOID(backButtonLayoutProperty);
        backButtonLayoutProperty->UpdateVisibility(
            titleBarLayoutProperty->GetHideBackButtonValue(false) ? VisibleType::GONE : VisibleType::VISIBLE);
    }

    // update main title
    auto mainTitleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle());
    if (mainTitleNode && !navBarNode->GetPrevTitleIsCustomValue(false)) {
        auto textLayoutProperty = mainTitleNode->GetLayoutProperty<TextLayoutProperty>();
        auto theme = NavigationGetTheme();
        CHECK_NULL_VOID(theme);
        if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::MINI) {
            textLayoutProperty->UpdateFontSize(theme->GetTitleFontSize());
        } else {
            textLayoutProperty->UpdateFontSize(theme->GetTitleFontSizeBig());
        }
        mainTitleNode->MarkModifyDone();
    }

    // update menu
    BuildMenu(navBarNode, titleBarNode);
}

void MountTitleBar(const RefPtr<NavBarNode>& hostNode)
{
    auto navBarLayoutProperty = hostNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    auto navBarPattern = AceType::DynamicCast<NavBarPattern>(hostNode->GetPattern());
    auto hasCustomMenu = hostNode->GetPrevMenuIsCustomValue(false) && hostNode->GetMenu();
    // menu is not consume menu, menu item and tool bar menus need all empty
    auto hideMenu = !hostNode->GetPrevMenuIsCustomValue(false) && navBarPattern->GetTitleBarMenuItems().empty() &&
                    navBarPattern->GetToolBarMenuItems().empty();
    if (!titleBarNode->GetTitle() && !titleBarNode->GetSubtitle() && !titleBarNode->GetBackButton() && !hasCustomMenu &&
        hideMenu) {
        titleBarLayoutProperty->UpdateVisibility(VisibleType::GONE);
        return;
    }
    titleBarLayoutProperty->UpdateTitleMode(navBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE));
    titleBarLayoutProperty->UpdateHideBackButton(navBarLayoutProperty->GetHideBackButtonValue(false));
    BuildTitleBar(hostNode, titleBarNode);
    if (navBarLayoutProperty->GetHideTitleBar().value_or(false)) {
        titleBarLayoutProperty->UpdateVisibility(VisibleType::GONE);
        titleBarNode->SetJSViewActive(false);
    } else {
        titleBarLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
        titleBarNode->SetJSViewActive(true);

        auto&& opts = navBarLayoutProperty->GetSafeAreaExpandOpts();
        if (opts) {
            titleBarLayoutProperty->UpdateSafeAreaExpandOpts(*opts);
        }
    }
    titleBarNode->MarkModifyDone();
    titleBarNode->MarkDirtyNode();
}

void MountToolBar(const RefPtr<NavBarNode>& hostNode)
{
    CHECK_NULL_VOID(hostNode->GetToolBarNode());
    auto navBarLayoutProperty = hostNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);
    auto toolBarNode = AceType::DynamicCast<NavToolbarNode>(hostNode->GetToolBarNode());
    CHECK_NULL_VOID(toolBarNode);
    auto toolBarLayoutProperty = toolBarNode->GetLayoutProperty<LayoutProperty>();
    CHECK_NULL_VOID(toolBarLayoutProperty);

    if (hostNode->GetToolBarNodeOperationValue(ChildNodeOperation::NONE) == ChildNodeOperation::REPLACE) {
        hostNode->RemoveChild(hostNode->GetPreToolBarNode());
        hostNode->AddChild(hostNode->GetToolBarNode());
    }

    if (navBarLayoutProperty->GetHideToolBar().value_or(false) || !toolBarNode->HasValidContent()) {
        toolBarLayoutProperty->UpdateVisibility(VisibleType::GONE);
    } else {
        toolBarLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);

        auto&& opts = navBarLayoutProperty->GetSafeAreaExpandOpts();
        if (opts) {
            toolBarLayoutProperty->UpdateSafeAreaExpandOpts(*opts);
        }
    }
    toolBarNode->MarkModifyDone();
    toolBarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
}
} // namespace

OffsetF NavBarPattern::GetShowMenuOffset(const RefPtr<BarItemNode> barItemNode, RefPtr<FrameNode> menuNode)
{
    auto imageNode = barItemNode->GetChildAtIndex(0);
    CHECK_NULL_RETURN(imageNode, OffsetF(0.0f, 0.0f));

    auto imageFrameNode = AceType::DynamicCast<FrameNode>(imageNode);
    CHECK_NULL_RETURN(imageFrameNode, OffsetF(0.0f, 0.0f));
    auto imgOffset = imageFrameNode->GetOffsetRelativeToWindow();
    auto imageSize = imageFrameNode->GetGeometryNode()->GetFrameSize();

    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    CHECK_NULL_RETURN(menuLayoutProperty, OffsetF(0.0f, 0.0f));
    menuLayoutProperty->UpdateTargetSize(imageSize);

    bool isRightToLeft = AceApplicationInfo::GetInstance().IsRightToLeft();
    if (isRightToLeft) {
        imgOffset.SetX(imgOffset.GetX() + imageSize.Width());
    } else {
        imgOffset.SetX(imgOffset.GetX());
    }
    imgOffset.SetY(imgOffset.GetY() + imageSize.Height());
    return imgOffset;
}

void NavBarPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->AddWindowSizeChangeCallback(host->GetId());

    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    if (theme && theme->GetNavBarUnfocusEffectEnable()) {
        pipelineContext->AddWindowFocusChangedCallback(host->GetId());
    }
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        SafeAreaExpandOpts opts = { .type = SAFE_AREA_TYPE_SYSTEM | SAFE_AREA_TYPE_CUTOUT,
            .edges = SAFE_AREA_EDGE_ALL };
        host->GetLayoutProperty()->UpdateSafeAreaExpandOpts(opts);
    }
}

void NavBarPattern::InitPanEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (isHideTitlebar_ || titleMode_ != NavigationTitleMode::FREE) {
        gestureHub->RemovePanEvent(panEvent_);
        return;
    }

    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleOnDragStart(info.GetOffsetY());
    };

    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleOnDragUpdate(info.GetOffsetY());
    };

    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleOnDragEnd();
    };

    auto actionCancelTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleOnDragEnd();
    };

    if (!panEvent_) {
        panEvent_ = MakeRefPtr<PanEvent>(std::move(actionStartTask), std::move(actionUpdateTask),
            std::move(actionEndTask), std::move(actionCancelTask));
    }

    PanDirection panDirection = { .type = PanDirection::VERTICAL };
    gestureHub->SetPanEvent(panEvent_, panDirection, DEFAULT_PAN_FINGER, DEFAULT_PAN_DISTANCE);
}

void NavBarPattern::HandleOnDragStart(float offset)
{
    auto hostNode = AceType::DynamicCast<NavBarNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto titleNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleNode);
    auto titlePattern = titleNode->GetPattern<TitleBarPattern>();
    CHECK_NULL_VOID(titlePattern);
    titlePattern->SetCanOverDrag(false);
    titlePattern->SetTitleScaleChange(true);
    titlePattern->ProcessTitleDragStart(offset);
}

void NavBarPattern::HandleOnDragUpdate(float offset)
{
    auto hostNode = AceType::DynamicCast<NavBarNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto titleNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleNode);
    auto titlePattern = titleNode->GetPattern<TitleBarPattern>();
    CHECK_NULL_VOID(titlePattern);
    titlePattern->ProcessTitleDragUpdate(offset);
}

void NavBarPattern::HandleOnDragEnd()
{
    auto hostNode = AceType::DynamicCast<NavBarNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto titleNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleNode);
    auto titlePattern = titleNode->GetPattern<TitleBarPattern>();
    CHECK_NULL_VOID(titlePattern);
    titlePattern->ProcessTitleDragEnd();
}

void NavBarPattern::OnCoordScrollStart()
{
    if (isHideTitlebar_ || titleMode_ != NavigationTitleMode::FREE) {
        return;
    }
    auto hostNode = AceType::DynamicCast<NavBarNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto titleNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleNode);
    auto titlePattern = titleNode->GetPattern<TitleBarPattern>();
    CHECK_NULL_VOID(titlePattern);
    titlePattern->OnCoordScrollStart();
}

float NavBarPattern::OnCoordScrollUpdate(float offset)
{
    if (isHideTitlebar_ || titleMode_ != NavigationTitleMode::FREE) {
        return 0.0f;
    }
    auto hostNode = AceType::DynamicCast<NavBarNode>(GetHost());
    CHECK_NULL_RETURN(hostNode, 0.0f);
    auto titleNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_RETURN(titleNode, 0.0f);
    auto titlePattern = titleNode->GetPattern<TitleBarPattern>();
    CHECK_NULL_RETURN(titlePattern, 0.0f);
    return titlePattern->OnCoordScrollUpdate(offset);
}

void NavBarPattern::OnCoordScrollEnd()
{
    TAG_LOGI(AceLogTag::ACE_NAVIGATION, "OnCoordScroll end");
    if (titleMode_ != NavigationTitleMode::FREE) {
        TAG_LOGI(AceLogTag::ACE_NAVIGATION, "titleMode_ is not free");
        return;
    }
    auto hostNode = AceType::DynamicCast<NavBarNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto titleNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleNode);
    auto titlePattern = titleNode->GetPattern<TitleBarPattern>();
    titlePattern->OnCoordScrollEnd();
}

void NavBarPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    auto hostNode = AceType::DynamicCast<NavBarNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarRenderContext = titleBarNode->GetRenderContext();
    CHECK_NULL_VOID(titleBarRenderContext);
    // set the titlebar to float on the top
    titleBarRenderContext->UpdateZIndex(DEFAULT_TITLEBAR_ZINDEX);
    MountTitleBar(hostNode);
    MountToolBar(hostNode);
    auto navBarLayoutProperty = hostNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);

    auto&& opts = navBarLayoutProperty->GetSafeAreaExpandOpts();
    auto navBarContentNode = AceType::DynamicCast<FrameNode>(hostNode->GetNavBarContentNode());
    if (opts && navBarContentNode) {
        navBarContentNode->GetLayoutProperty()->UpdateSafeAreaExpandOpts(*opts);
        navBarContentNode->MarkModifyDone();
    }

    isHideToolbar_ = navBarLayoutProperty->GetHideToolBarValue(false);
    isHideTitlebar_ = navBarLayoutProperty->GetHideTitleBarValue(false);
    titleMode_ = navBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE);
}

void NavBarPattern::OnWindowSizeChanged(int32_t width, int32_t height, WindowSizeChangeReason type)
{
    auto navBarNode = AceType::DynamicCast<NavBarNode>(GetHost());
    CHECK_NULL_VOID(navBarNode);
    // change menu num in landscape and orientation
    do {
        if (navBarNode->GetPrevMenuIsCustomValue(false)) {
            break;
        }
        auto targetNum = SystemProperties::GetDeviceOrientation() == DeviceOrientation::LANDSCAPE ? MAX_MENU_NUM_LARGE
                                                                                                  : MAX_MENU_NUM_SMALL;
        if (targetNum == maxMenuNums_) {
            break;
        }
        maxMenuNums_ = targetNum;
        auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
        CHECK_NULL_VOID(titleBarNode);
        BuildMenu(navBarNode, titleBarNode);
        titleBarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
    } while (0);
    if (isTitleMenuNodeShowing_ == navBarNode->IsTitleMenuNodeShowing()) {
        return;
    }
    if (type == WindowSizeChangeReason::ROTATION || type == WindowSizeChangeReason::RESIZE) {
        isTitleMenuNodeShowing_ = navBarNode->IsTitleMenuNodeShowing();
    }
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    if (titleBarNode->GetMenu()) {
        auto buttonNode = titleBarNode->GetMenu()->GetLastChild();
        CHECK_NULL_VOID(buttonNode);
        auto barItemNode = buttonNode->GetFirstChild();
        CHECK_NULL_VOID(barItemNode);
        auto barItemFrameNode = AceType::DynamicCast<BarItemNode>(barItemNode);
        CHECK_NULL_VOID(barItemFrameNode);
        if (barItemFrameNode->IsMoreItemNode() && isTitleMenuNodeShowing_) {
            auto eventHub = barItemFrameNode->GetEventHub<BarItemEventHub>();
            CHECK_NULL_VOID(eventHub);
            eventHub->FireItemAction();
        }
    }
}

void NavBarPattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
    CHECK_NULL_VOID(frameNode);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->RemoveWindowSizeChangeCallback(frameNode->GetId());
}

void NavBarPattern::WindowFocus(bool isFocus)
{
    isWindowFocus_ = isFocus;
    SetNavBarMask(isFocus);
}

void NavBarPattern::OnColorConfigurationUpdate()
{
    SetNavBarMask(isWindowFocus_);
}

void NavBarPattern::SetNavBarMask(bool isWindowFocus)
{
    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    auto navBarNode = GetHost();
    CHECK_NULL_VOID(navBarNode);
    auto parent = navBarNode->GetParent();
    CHECK_NULL_VOID(parent);
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(parent);
    CHECK_NULL_VOID(navigationGroupNode);
    auto pattern = navigationGroupNode->GetPattern();
    CHECK_NULL_VOID(pattern);
    auto navigationPattern = AceType::DynamicCast<NavigationPattern>(pattern);
    if (navigationPattern && navigationPattern->GetNavigationMode() == NavigationMode::SPLIT) {
        auto renderContext = navBarNode->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        Color maskColor = theme->GetNavBarUnfocusColor().BlendOpacity(DEFAULT_NAV_BAR_MASK_OPACITY);
        auto maskProperty = AceType::MakeRefPtr<ProgressMaskProperty>();
        maskProperty->SetColor(isWindowFocus ? Color::TRANSPARENT : maskColor);
        renderContext->UpdateProgressMask(maskProperty);
    }
}

bool NavBarPattern::CanCoordScrollUp(float offset) const
{
    auto hostNode = AceType::DynamicCast<NavBarNode>(GetHost());
    CHECK_NULL_RETURN(hostNode, false);
    auto titleNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_RETURN(titleNode, false);
    auto titlePattern = titleNode->GetPattern<TitleBarPattern>();
    CHECK_NULL_RETURN(titlePattern, false);
    return Negative(offset) && titlePattern->IsCurrentMaxTitle();
}
} // namespace OHOS::Ace::NG
