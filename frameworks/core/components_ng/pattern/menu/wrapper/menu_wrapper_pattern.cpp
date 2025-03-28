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

#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"

#include <cstddef>

#include "base/log/dump_log.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/select/select_theme.h"
#include "core/components_ng/event/click_event.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
void MenuWrapperPattern::HideMenu(const RefPtr<FrameNode>& menu)
{
    if (GetHost()->GetTag() == V2::SELECT_OVERLAY_ETS_TAG) {
        return;
    }

    auto menuPattern = menu->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    menuPattern->HideMenu();
    CallMenuStateChangeCallback("false");
}

void MenuWrapperPattern::OnModifyDone()
{
    InitFocusEvent();
}

void MenuWrapperPattern::InitFocusEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    auto blurTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HideMenu();
    };
    focusHub->SetOnBlurInternal(std::move(blurTask));
}

RefPtr<FrameNode> MenuWrapperPattern::GetShowedSubMenu()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    return DynamicCast<FrameNode>(host->GetLastChild());
}

RectF MenuWrapperPattern::GetMenuZone(RefPtr<UINode>& innerMenuNode)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, RectF());
    auto outterMenuNode = DynamicCast<FrameNode>(host->GetChildAtIndex(0));
    CHECK_NULL_RETURN(outterMenuNode, RectF());
    auto menuZone = outterMenuNode->GetGeometryNode()->GetFrameRect();
    innerMenuNode = GetMenuChild(outterMenuNode);
    CHECK_NULL_RETURN(innerMenuNode, RectF());
    auto subMenuNode = GetShowedSubMenu();
    if (subMenuNode) {
        innerMenuNode = subMenuNode;
        auto scrollNode = DynamicCast<FrameNode>(innerMenuNode->GetChildAtIndex(0));
        CHECK_NULL_RETURN(scrollNode, RectF());
        innerMenuNode = DynamicCast<FrameNode>(scrollNode->GetChildAtIndex(0));
        CHECK_NULL_RETURN(innerMenuNode, RectF());
        auto offset = DynamicCast<FrameNode>(innerMenuNode)->GetOffsetRelativeToWindow();
        menuZone.SetOffset(offset);
    }
    return menuZone;
}

RefPtr<FrameNode> MenuWrapperPattern::FindTouchedMenuItem(const RefPtr<UINode>& menuNode, const OffsetF& position)
{
    CHECK_NULL_RETURN(menuNode, nullptr);
    RefPtr<FrameNode> menuItem = nullptr;
    const auto& children = menuNode->GetChildren();
    for (auto child : children) {
        if (child->GetTag() == V2::MENU_ITEM_ETS_TAG) {
            auto frameNode = AceType::DynamicCast<FrameNode>(child);
            auto pattern = frameNode ? frameNode->GetPattern<MenuItemPattern>() : nullptr;
            menuItem = pattern ? pattern->FindTouchedEmbeddedMenuItem(position) : nullptr;
        } else {
            menuItem = FindTouchedMenuItem(child, position);
        }
        if (menuItem) {
            auto menuItemOffset = menuItem->GetPaintRectOffset();
            auto menuItemSize = menuItem->GetGeometryNode()->GetFrameSize();
            auto menuItemZone =
                RectF(menuItemOffset.GetX(), menuItemOffset.GetY(), menuItemSize.Width(), menuItemSize.Height());
            if (menuItemZone.IsInRegion(PointF(position.GetX(), position.GetY()))) {
                break;
            } else {
                menuItem = nullptr;
            }
        }
    }
    return menuItem;
}

void MenuWrapperPattern::HandleInteraction(const TouchEventInfo& info)
{
    auto touch = info.GetTouches().front();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto position = OffsetF(
        static_cast<float>(touch.GetGlobalLocation().GetX()), static_cast<float>(touch.GetGlobalLocation().GetY()));
    position -= host->GetPaintRectOffset();
    RefPtr<UINode> innerMenuNode = nullptr;
    auto menuZone = GetMenuZone(innerMenuNode);
    CHECK_NULL_VOID(innerMenuNode);
    // get menuNode's touch region
    if (menuZone.IsInRegion(PointF(position.GetX(), position.GetY()))) {
        currentTouchItem_ = FindTouchedMenuItem(innerMenuNode, position);
        auto lastTouchItem = GetLastTouchItem();
        if (currentTouchItem_ && currentTouchItem_ != lastTouchItem_) {
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto theme = pipeline->GetTheme<SelectTheme>();
            CHECK_NULL_VOID(theme);
            auto curMenuItemPattern = currentTouchItem_->GetPattern<MenuItemPattern>();
            CHECK_NULL_VOID(curMenuItemPattern);
            if (!(curMenuItemPattern->IsDisabled())) {
                curMenuItemPattern->SetBgBlendColor(
                    curMenuItemPattern->GetSubBuilder() ? theme->GetHoverColor() : theme->GetClickedColor());
                curMenuItemPattern->PlayBgColorAnimation(false);
            }
            if (lastTouchItem_) {
                auto lastMenuItemPattern = lastTouchItem_->GetPattern<MenuItemPattern>();
                CHECK_NULL_VOID(lastMenuItemPattern);
                lastMenuItemPattern->SetBgBlendColor(Color::TRANSPARENT);
                lastMenuItemPattern->PlayBgColorAnimation(false);
            }
            lastTouchItem_ = currentTouchItem_;
        }
    } else if (lastTouchItem_) {
        auto lastMenuItemPattern = lastTouchItem_->GetPattern<MenuItemPattern>();
        CHECK_NULL_VOID(lastMenuItemPattern);
        lastMenuItemPattern->SetBgBlendColor(Color::TRANSPARENT);
        lastMenuItemPattern->PlayBgColorAnimation(false);
        lastTouchItem_ = nullptr;
    }
    innerMenuNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void MenuWrapperPattern::OnAttachToFrameNode()
{
    RegisterOnTouch();
}

// close subMenu when mouse move outside
void MenuWrapperPattern::HandleMouseEvent(const MouseInfo& info, RefPtr<MenuItemPattern>& menuItemPattern)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto subMenu = host->GetChildren().back();
    if (host->GetChildren().size() <= 1) {
        return;
    }
    auto subMenuNode = DynamicCast<FrameNode>(subMenu);
    CHECK_NULL_VOID(subMenuNode);
    auto subMenuPattern = subMenuNode->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(subMenuPattern);
    auto currentHoverMenuItem = subMenuPattern->GetParentMenuItem();
    CHECK_NULL_VOID(currentHoverMenuItem);

    auto menuItemNode = menuItemPattern->GetHost();
    CHECK_NULL_VOID(menuItemNode);
    if (currentHoverMenuItem->GetId() != menuItemNode->GetId()) {
        return;
    }
    const auto& mousePosition = info.GetGlobalLocation();
    if (!menuItemPattern->IsInHoverRegions(mousePosition.GetX(), mousePosition.GetY()) &&
        menuItemPattern->IsSubMenuShowed()) {
        LOGI("MenuWrapperPattern Hide SubMenu");
        HideSubMenu();
        menuItemPattern->SetIsSubMenuShowed(false);
        menuItemPattern->ClearHoverRegions();
        menuItemPattern->ResetWrapperMouseEvent();
    }
}

void MenuWrapperPattern::HideMenu()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto menuNode = DynamicCast<FrameNode>(host->GetChildAtIndex(0));
    CHECK_NULL_VOID(menuNode);
    HideMenu(menuNode);
}

void MenuWrapperPattern::HideSubMenu()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (host->GetChildren().size() <= 1) {
        // sub menu not show
        return;
    }
    auto menu = GetMenu();
    CHECK_NULL_VOID(menu);
    auto menuPattern = menu->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    menuPattern->SetShowedSubMenu(nullptr);
    auto subMenu = host->GetChildren().back();
    auto subMenuPattern = DynamicCast<FrameNode>(subMenu)->GetPattern<MenuPattern>();
    if (subMenuPattern) {
        subMenuPattern->RemoveParentHoverStyle();
        auto frameNode = FrameNode::GetFrameNode(subMenuPattern->GetTargetTag(), subMenuPattern->GetTargetId());
        CHECK_NULL_VOID(frameNode);
        auto menuItem = frameNode->GetPattern<MenuItemPattern>();
        if (menuItem) {
            menuItem->SetIsSubMenuShowed(false);
        }
    }
    auto focusMenu = MenuFocusViewShow();
    CHECK_NULL_VOID(focusMenu);
    auto innerMenu = GetMenuChild(focusMenu);
    if (!innerMenu) {
        UpdateMenuAnimation(host);
        host->RemoveChild(subMenu);
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
        return;
    }
    CHECK_NULL_VOID(innerMenu);
    auto innerMenuPattern = innerMenu->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(innerMenuPattern);
    auto layoutProps = innerMenuPattern->GetLayoutProperty<MenuLayoutProperty>();
    CHECK_NULL_VOID(layoutProps);
    auto expandingMode = layoutProps->GetExpandingMode().value_or(SubMenuExpandingMode::SIDE);
    auto outterMenuPattern = focusMenu->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(outterMenuPattern);
    bool hasAnimation = outterMenuPattern->GetDisappearAnimation();
    if (expandingMode == SubMenuExpandingMode::STACK && hasAnimation) {
        HideStackExpandMenu(subMenu);
    } else {
        UpdateMenuAnimation(host);
        host->RemoveChild(subMenu);
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
    }
}

bool MenuWrapperPattern::HasStackSubMenu()
{
    auto outterMenu = GetMenu();
    CHECK_NULL_RETURN(outterMenu, false);
    auto innerMenu = GetMenuChild(outterMenu);
    CHECK_NULL_RETURN(innerMenu, false);
    auto innerMenuPattern = innerMenu->GetPattern<MenuPattern>();
    CHECK_NULL_RETURN(innerMenuPattern, false);
    auto layoutProps = innerMenuPattern->GetLayoutProperty<MenuLayoutProperty>();
    CHECK_NULL_RETURN(layoutProps, false);
    auto expandingMode = layoutProps->GetExpandingMode().value_or(SubMenuExpandingMode::SIDE);
    if (expandingMode != SubMenuExpandingMode::STACK) {
        return false;
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    return host->GetChildren().size() > 1;
}

bool MenuWrapperPattern::HasEmbeddedSubMenu()
{
    auto outterMenu = GetMenu();
    CHECK_NULL_RETURN(outterMenu, false);
    auto innerMenu = GetMenuChild(outterMenu);
    CHECK_NULL_RETURN(innerMenu, false);
    auto innerMenuPattern = innerMenu->GetPattern<MenuPattern>();
    CHECK_NULL_RETURN(innerMenuPattern, false);
    auto layoutProps = innerMenuPattern->GetLayoutProperty<MenuLayoutProperty>();
    CHECK_NULL_RETURN(layoutProps, false);
    auto expandingMode = layoutProps->GetExpandingMode().value_or(SubMenuExpandingMode::SIDE);
    return expandingMode == SubMenuExpandingMode::EMBEDDED;
}

RefPtr<FrameNode> MenuWrapperPattern::MenuFocusViewShow()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto iter = host->GetChildren().begin();
    int32_t focusNodeId = 2;
    std::advance(iter, host->GetChildren().size() - focusNodeId);
    auto focusMenu = *iter;
    if (focusMenu) {
        auto menuHub = DynamicCast<FrameNode>(focusMenu);
        CHECK_NULL_RETURN(menuHub, nullptr);
        // SelectOverlay's custom menu does not need to be focused.
        auto isCustomMenu = IsSelectOverlayCustomMenu(menuHub);
        if (!isCustomMenu) {
            auto menuPattern = menuHub->GetPattern<MenuPattern>();
            CHECK_NULL_RETURN(menuPattern, nullptr);
            menuPattern->FocusViewShow();
        }
    }
    return DynamicCast<FrameNode>(focusMenu);
}

void MenuWrapperPattern::HideStackExpandMenu(const RefPtr<UINode>& subMenu)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto menuNode = host->GetFirstChild();
    CHECK_NULL_VOID(menuNode);
    AnimationOption option;
    option.SetOnFinishEvent(
        [weak = WeakClaim(RawPtr(host)), subMenuWk = WeakClaim(RawPtr(subMenu))] {
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto taskExecutor = pipeline->GetTaskExecutor();
            CHECK_NULL_VOID(taskExecutor);
            taskExecutor->PostTask(
                [weak, subMenuWk]() {
                    auto subMenuNode = subMenuWk.Upgrade();
                    CHECK_NULL_VOID(subMenuNode);
                    auto menuWrapper = weak.Upgrade();
                    CHECK_NULL_VOID(menuWrapper);
                    menuWrapper->RemoveChild(subMenuNode);
                    menuWrapper->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
                },
                TaskExecutor::TaskType::UI, "HideStackExpandMenu");
    });
    auto menuNodePattern = DynamicCast<FrameNode>(menuNode)->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuNodePattern);
    menuNodePattern->ShowStackExpandDisappearAnimation(DynamicCast<FrameNode>(menuNode),
        DynamicCast<FrameNode>(subMenu), option);
    menuNodePattern->SetDisappearAnimation(true);
}

void MenuWrapperPattern::RegisterOnTouch()
{
    // if already initialized touch event
    CHECK_NULL_VOID(!onTouch_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gesture = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gesture);
    // hide menu when touched outside the menu region
    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnTouchEvent(info);
    };
    onTouch_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gesture->AddTouchEvent(onTouch_);
}

void MenuWrapperPattern::OnTouchEvent(const TouchEventInfo& info)
{
    CHECK_NULL_VOID(!info.GetTouches().empty());
    auto touch = info.GetTouches().front();
    // filter out other touch types
    if (touch.GetTouchType() != TouchType::DOWN &&
        Container::LessThanAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
        return;
    }
    if (IsHide()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    auto position = OffsetF(
        static_cast<float>(touch.GetGlobalLocation().GetX()), static_cast<float>(touch.GetGlobalLocation().GetY()));
    position -= host->GetPaintRectOffset();
    auto children = host->GetChildren();
    if (touch.GetTouchType() == TouchType::DOWN) {
        for (auto child = children.rbegin(); child != children.rend(); ++child) {
            // get child frame node of menu wrapper
            auto menuWrapperChildNode = DynamicCast<FrameNode>(*child);
            CHECK_NULL_VOID(menuWrapperChildNode);
            // get menuWrapperChildNode's touch region
            auto menuWrapperChildZone = menuWrapperChildNode->GetGeometryNode()->GetFrameRect();
            if (menuWrapperChildZone.IsInRegion(PointF(position.GetX(), position.GetY()))) {
                return;
            }
            // if DOWN-touched outside the menu region, then hide menu
            auto menuPattern = menuWrapperChildNode->GetPattern<MenuPattern>();
            if (!menuPattern) {
                continue;
            }
            HideMenu(menuPattern, menuWrapperChildNode, position);
        }
    } else if (touch.GetTouchType() == TouchType::MOVE) {
        auto menuNode = DynamicCast<FrameNode>(host->GetChildAtIndex(0));
        CHECK_NULL_VOID(menuNode);
        if (GetPreviewMode() != MenuPreviewMode::NONE || IsSelectOverlayCustomMenu(menuNode)) {
            return;
        }
        HandleInteraction(info);
    } else if (touch.GetTouchType() == TouchType::UP && currentTouchItem_) {
        auto currentTouchItemPattern = currentTouchItem_->GetPattern<MenuItemPattern>();
        CHECK_NULL_VOID(currentTouchItemPattern);
        currentTouchItemPattern->SetBgBlendColor(Color::TRANSPARENT);
        currentTouchItemPattern->PlayBgColorAnimation(false);
        currentTouchItem_ = nullptr;
    }
}

void MenuWrapperPattern::HideMenu(const RefPtr<MenuPattern>& menuPattern, const RefPtr<FrameNode>& menu,
    const OffsetF& position)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto mainMenu = DynamicCast<FrameNode>(host->GetFirstChild());
    CHECK_NULL_VOID(mainMenu);
    auto mainMenuZone = mainMenu->GetGeometryNode()->GetFrameRect();
    bool isFindTargetId = false;
    if (mainMenuZone.IsInRegion(PointF(position.GetX(), position.GetY()))) {
        isFindTargetId = true;
    }
    if (menuPattern->IsSubMenu() || menuPattern->IsSelectOverlaySubMenu()) {
        if (HasStackSubMenu() && !isFindTargetId) {
            UpdateMenuAnimation(host);
        }
        HideSubMenu();
    } else {
        if (HasEmbeddedSubMenu() && embeddedSubMenuCount_ > 0 && !isFindTargetId) {
            UpdateMenuAnimation(host);
        }
        HideMenu(menu);
    }
}

void MenuWrapperPattern::UpdateMenuAnimation(const RefPtr<FrameNode>& host)
{
    // update Menu disappear animation direction
    // change to LEFT_BOTTOM -> RIGHT_TOP by calling SetExitAnimation
    // or keep BOTTOM -> TOP by default
    CHECK_NULL_VOID(host);
    auto outterMenu = host->GetFirstChild();
    CHECK_NULL_VOID(outterMenu);
    auto innerMenu = GetMenuChild(outterMenu);
    if (!innerMenu && host->GetChildren().size() > 1) {
        SetExitAnimation(host);
        return;
    }
    CHECK_NULL_VOID(innerMenu);
    auto innerMenuPattern = innerMenu->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(innerMenuPattern);
    auto layoutProps = innerMenuPattern->GetLayoutProperty<MenuLayoutProperty>();
    CHECK_NULL_VOID(layoutProps);
    auto expandingMode = layoutProps->GetExpandingMode().value_or(SubMenuExpandingMode::SIDE);
    if (host->GetChildren().size() > 1) {
        SetExitAnimation(host);
    }
    if (expandingMode == SubMenuExpandingMode::EMBEDDED && embeddedSubMenuCount_ > 0) {
        SetExitAnimation(host);
    }
}

void MenuWrapperPattern::SetExitAnimation(const RefPtr<FrameNode>& host)
{
    CHECK_NULL_VOID(host);
    auto outterMenu = AceType::DynamicCast<FrameNode>(host->GetFirstChild());
    CHECK_NULL_VOID(outterMenu);
    auto outterMenuPattern = outterMenu->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(outterMenuPattern);
    outterMenuPattern->SetDisappearAnimation(false);
}

void MenuWrapperPattern::CheckAndShowAnimation()
{
    if (isFirstShow_) {
        // only start animation when menu wrapper mount on.
        StartShowAnimation();
        isFirstShow_ = false;
    }
}

bool MenuWrapperPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_RETURN(theme, false);
    auto expandDisplay = theme->GetExpandDisplay();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto menuNode = DynamicCast<FrameNode>(host->GetChildAtIndex(0));
    CHECK_NULL_RETURN(menuNode, false);
    auto menuPattern = AceType::DynamicCast<MenuPattern>(menuNode->GetPattern());
    CHECK_NULL_RETURN(menuPattern, false);
    // copy menu pattern properties to rootMenu
    auto layoutProperty = menuPattern->GetLayoutProperty<MenuLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, false);
    isShowInSubWindow_ = layoutProperty->GetShowInSubWindowValue(true);
    if ((IsContextMenu() && !IsHide()) || ((expandDisplay && isShowInSubWindow_) && !IsHide())) {
        SetHotAreas(dirty);
    }
    CheckAndShowAnimation();
    return false;
}

void MenuWrapperPattern::SetHotAreas(const RefPtr<LayoutWrapper>& layoutWrapper)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    auto expandDisplay = theme->GetExpandDisplay();
    if ((layoutWrapper->GetAllChildrenWithBuild().empty() || !IsContextMenu()) &&
        !(expandDisplay && isShowInSubWindow_)) {
        return;
    }
    auto layoutProps = layoutWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProps);
    float safeAreaInsetsLeft = 0.0f;
    float safeAreaInsetsTop = 0.0f;
    auto&& safeAreaInsets = layoutProps->GetSafeAreaInsets();
    if (safeAreaInsets) {
        safeAreaInsetsLeft = static_cast<float>(safeAreaInsets->left_.end);
        safeAreaInsetsTop = static_cast<float>(safeAreaInsets->top_.end);
    }
    std::vector<Rect> rects;
    for (const auto& child : layoutWrapper->GetAllChildrenWithBuild()) {
        auto frameRect = child->GetGeometryNode()->GetFrameRect();
        // rect is relative to window
        auto rect = Rect(frameRect.GetX() + safeAreaInsetsLeft, frameRect.GetY() + safeAreaInsetsTop, frameRect.Width(),
            frameRect.Height());

        rects.emplace_back(rect);
    }
    // If container is UIExtensionWindow, set hotArea size equals to subwindow's filterColumnNode's size
    if (IsContextMenu() && GetPreviewMode() != MenuPreviewMode::NONE) {
        auto filterNode = GetFilterColumnNode();
        if (filterNode) {
            auto frameRect = filterNode->GetGeometryNode()->GetFrameRect();
            auto rect = Rect(frameRect.GetX(), frameRect.GetY(), frameRect.Width(), frameRect.Height());
            rects.emplace_back(rect);
        }
    }
    SubwindowManager::GetInstance()->SetHotAreas(rects, GetHost()->GetId(), GetContainerId());
}

void MenuWrapperPattern::StartShowAnimation()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetRenderContext();
    CHECK_NULL_VOID(context);
    if (GetPreviewMode() == MenuPreviewMode::NONE) {
        context->UpdateOffset(GetAnimationOffset());
        context->UpdateOpacity(0.0);
    }

    AnimationUtils::Animate(
        animationOption_,
        [context, weak = WeakClaim(this)]() {
            if (context) {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                if (pattern->GetPreviewMode() == MenuPreviewMode::NONE) {
                    context->UpdateOffset(OffsetT<Dimension>());
                    context->UpdateOpacity(1.0);
                }
            }
        },
        animationOption_.GetOnFinishEvent());
}

void MenuWrapperPattern::SetAniamtinOption(const AnimationOption& animationOption)
{
    animationOption_.SetCurve(animationOption.GetCurve());
    animationOption_.SetDuration(animationOption.GetDuration());
    animationOption_.SetFillMode(animationOption.GetFillMode());
    animationOption_.SetOnFinishEvent(animationOption.GetOnFinishEvent());
}

OffsetT<Dimension> MenuWrapperPattern::GetAnimationOffset()
{
    OffsetT<Dimension> offset;

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, offset);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_RETURN(theme, offset);
    auto animationOffset = theme->GetMenuAnimationOffset();

    switch (menuPlacement_) {
        case Placement::LEFT:
        case Placement::LEFT_TOP:
        case Placement::LEFT_BOTTOM:
            offset.SetX(animationOffset);
            break;
        case Placement::RIGHT:
        case Placement::RIGHT_TOP:
        case Placement::RIGHT_BOTTOM:
            offset.SetX(-animationOffset);
            break;
        case Placement::TOP:
        case Placement::TOP_LEFT:
        case Placement::TOP_RIGHT:
            offset.SetY(animationOffset);
            break;
        default:
            offset.SetY(-animationOffset);
            break;
    }
    return offset;
}

bool MenuWrapperPattern::IsSelectOverlayCustomMenu(const RefPtr<FrameNode>& menu) const
{
    auto menuPattern = menu->GetPattern<MenuPattern>();
    CHECK_NULL_RETURN(menuPattern, false);
    return menuPattern->IsSelectOverlayCustomMenu();
}

void MenuWrapperPattern::RegisterMenuCallback(const RefPtr<FrameNode>& menuWrapperNode, const MenuParam& menuParam)
{
    TAG_LOGD(AceLogTag::ACE_DIALOG, "register menu enter");
    CHECK_NULL_VOID(menuWrapperNode);
    auto pattern = menuWrapperNode->GetPattern<MenuWrapperPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->RegisterMenuAppearCallback(menuParam.onAppear);
    pattern->RegisterMenuDisappearCallback(menuParam.onDisappear);
    pattern->RegisterMenuAboutToAppearCallback(menuParam.aboutToAppear);
    pattern->RegisterMenuAboutToDisappearCallback(menuParam.aboutToDisappear);
    pattern->RegisterMenuStateChangeCallback(menuParam.onStateChange);
}

void MenuWrapperPattern::SetMenuTransitionEffect(const RefPtr<FrameNode>& menuWrapperNode, const MenuParam& menuParam)
{
    TAG_LOGD(AceLogTag::ACE_DIALOG, "set menu transition effect");
    CHECK_NULL_VOID(menuWrapperNode);
    auto pattern = menuWrapperNode->GetPattern<MenuWrapperPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetHasTransitionEffect(menuParam.hasTransitionEffect);
    if (menuParam.hasTransitionEffect) {
        auto renderContext = menuWrapperNode->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        CHECK_NULL_VOID(menuParam.transition);
        renderContext->UpdateChainedTransition(menuParam.transition);
    }
    pattern->SetHasPreviewTransitionEffect(menuParam.hasPreviewTransitionEffect);
    if (menuParam.hasPreviewTransitionEffect) {
        auto previewChild = pattern->GetPreview();
        CHECK_NULL_VOID(previewChild);
        auto renderContext = previewChild->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        CHECK_NULL_VOID(menuParam.previewTransition);
        renderContext->UpdateChainedTransition(menuParam.previewTransition);
    }
}

RefPtr<FrameNode> MenuWrapperPattern::GetMenuChild(const RefPtr<UINode>& node)
{
    CHECK_NULL_RETURN(node, nullptr);
    RefPtr<FrameNode> menuChild;
    auto child = node->GetChildAtIndex(0);
    while (child) {
        if (child->GetTag() == V2::JS_VIEW_ETS_TAG) {
            auto customNode = DynamicCast<CustomNode>(child);
            customNode->Render();
        } else if (child->GetTag() == V2::MENU_ETS_TAG) {
            menuChild = DynamicCast<FrameNode>(child);
            break;
        }
        child = child->GetChildAtIndex(0);
    }
    return menuChild;
}

void MenuWrapperPattern::DumpInfo()
{
    DumpLog::GetInstance().AddDesc("MenuPreviewMode: " + std::to_string(dumpInfo_.menuPreviewMode));
    DumpLog::GetInstance().AddDesc("MenuType: " + std::to_string(dumpInfo_.menuType));
    DumpLog::GetInstance().AddDesc("EnableArrow: " + std::to_string(dumpInfo_.enableArrow));
    DumpLog::GetInstance().AddDesc("TargetNode: " + dumpInfo_.targetNode);
    DumpLog::GetInstance().AddDesc("TargetOffset: " + dumpInfo_.targetOffset.ToString());
    DumpLog::GetInstance().AddDesc("TargetSize: " + dumpInfo_.targetSize.ToString());
    DumpLog::GetInstance().AddDesc("WrapperRect: " + dumpInfo_.wrapperRect.ToString());
    DumpLog::GetInstance().AddDesc("PreviewBeginScale: " + std::to_string(dumpInfo_.previewBeginScale));
    DumpLog::GetInstance().AddDesc("PreviewEndScale: " + std::to_string(dumpInfo_.previewEndScale));
    DumpLog::GetInstance().AddDesc("Top: " + std::to_string(dumpInfo_.top));
    DumpLog::GetInstance().AddDesc("Bottom: " + std::to_string(dumpInfo_.bottom));
    DumpLog::GetInstance().AddDesc("GlobalLocation: " + dumpInfo_.globalLocation.ToString());
    DumpLog::GetInstance().AddDesc("OriginPlacement: " + dumpInfo_.originPlacement);
    DumpLog::GetInstance().AddDesc("FinalPosition: " + dumpInfo_.finalPosition.ToString());
    DumpLog::GetInstance().AddDesc("FinalPlacement: " + dumpInfo_.finalPlacement);
}
} // namespace OHOS::Ace::NG
