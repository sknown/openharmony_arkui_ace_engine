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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_MENU_MENU_WRAPPER_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_MENU_MENU_WRAPPER_PATTERN_H

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/subwindow/subwindow_manager.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/placement.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/menu_layout_algorithm.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_layout_algorithm.h"
#include "core/components_ng/pattern/overlay/popup_base_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {

enum class MenuStatus {
    INIT,              // Neither exists in the menuMap_ nor on the tree
    ON_SHOW_ANIMATION, // Exists in the menuMap_ also exists on the tree
    SHOW,              // Exists in the menuMap_ also exists on the tree
    ON_HIDE_ANIMATION, // Exists in the menuMap_ also exists on the tree
    HIDE               // Exists in the menuMap_ but not on the tree
};

// has full screen size
// used for detecting clicks outside Menu area
class MenuWrapperPattern : public PopupBasePattern {
    DECLARE_ACE_TYPE(MenuWrapperPattern, Pattern);

public:
    explicit MenuWrapperPattern(int32_t Id) : targetId_(Id) {}
    ~MenuWrapperPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::SCOPE, true };
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<MenuWrapperLayoutAlgorithm>();
    }

    void HandleMouseEvent(const MouseInfo& info, RefPtr<MenuItemPattern>& menuItem);

    int32_t GetTargetId() const override
    {
        return targetId_;
    }

    void HideMenu();

    bool IsHide() const
    {
        return menuStatus_ == MenuStatus::ON_HIDE_ANIMATION || menuStatus_ == MenuStatus::HIDE;
    }

    bool IsContextMenu() const
    {
        auto menu = GetMenu();
        CHECK_NULL_RETURN(menu, false);
        auto menuPattern = menu->GetPattern<MenuPattern>();
        CHECK_NULL_RETURN(menuPattern, false);
        return menuPattern->IsContextMenu();
    }

    MenuPreviewMode GetPreviewMode() const
    {
        auto menu = GetMenu();
        CHECK_NULL_RETURN(menu, MenuPreviewMode::NONE);
        auto menuPattern = menu->GetPattern<MenuPattern>();
        CHECK_NULL_RETURN(menuPattern, MenuPreviewMode::NONE);
        return menuPattern->GetPreviewMode();
    }

    bool IsSelectMenu() const
    {
        auto menu = GetMenu();
        CHECK_NULL_RETURN(menu, false);
        auto menuPattern = menu->GetPattern<MenuPattern>();
        CHECK_NULL_RETURN(menuPattern, false);
        return menuPattern->IsSelectMenu();
    }

    void HideSubMenu();
    RefPtr<FrameNode> MenuFocusViewShow();
    void HideStackExpandMenu(const RefPtr<UINode>& subMenu);

    RefPtr<FrameNode> GetMenu() const
    {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, nullptr);
        auto menu = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(0));
        CHECK_NULL_RETURN(menu, nullptr);
        return menu;
    }

    RefPtr<FrameNode> GetHoverImageColNode() const
    {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, nullptr);
        auto node = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(1));
        CHECK_NULL_RETURN(node, nullptr);
        if (node->GetTag() != V2::COLUMN_ETS_TAG) {
            return nullptr;
        }
        return node;
    }

    RefPtr<FrameNode> GetHoverImageStackNode() const
    {
        auto node = AceType::DynamicCast<FrameNode>(GetHoverImageColNode()->GetChildAtIndex(0));
        CHECK_NULL_RETURN(node, nullptr);
        if (node->GetTag() != V2::STACK_ETS_TAG) {
            return nullptr;
        }
        return node;
    }

    RefPtr<FrameNode> GetHoverImagePreview() const
    {
        auto node = AceType::DynamicCast<FrameNode>(GetHoverImageStackNode()->GetChildAtIndex(0));
        CHECK_NULL_RETURN(node, nullptr);
        if (node->GetTag() != V2::IMAGE_ETS_TAG) {
            return nullptr;
        }
        return node;
    }

    RefPtr<FrameNode> GetHoverImageCustomPreview() const
    {
        auto node = AceType::DynamicCast<FrameNode>(GetHoverImageStackNode()->GetChildAtIndex(1));
        CHECK_NULL_RETURN(node, nullptr);
        if (node->GetTag() != V2::MENU_PREVIEW_ETS_TAG) {
            return nullptr;
        }
        return node;
    }

    RefPtr<FrameNode> GetPreview() const
    {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, nullptr);
        auto preview = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(1));
        CHECK_NULL_RETURN(preview, nullptr);
        if (preview->GetTag() == V2::COLUMN_ETS_TAG) {
            auto hoverImageCustomPreview = GetHoverImageCustomPreview();
            CHECK_NULL_RETURN(hoverImageCustomPreview, preview);
            return hoverImageCustomPreview;
        }
        return preview;
    }

    // used to obtain the Badge node and delete it.
    RefPtr<FrameNode> GetBadgeNode() const
    {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, nullptr);
        auto badgeNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(2));
        CHECK_NULL_RETURN(badgeNode, nullptr);
        if (badgeNode->GetTag() != V2::TEXT_ETS_TAG) {
            return nullptr;
        }
        return badgeNode;
    }

    OffsetT<Dimension> GetAnimationOffset();
    void SetAniamtinOption(const AnimationOption& animationOption);

    void SetMenuPlacementAfterLayout(const Placement& placement)
    {
        menuPlacement_ = placement;
    }

    void SetFirstShow()
    {
        isFirstShow_ = true;
    }

    void SetIsShowHoverImage(bool isShow)
    {
        isShowHoverImage_ = isShow;
    }

    bool GetIsShowHoverImage()
    {
        return isShowHoverImage_;
    }

    void SetIsShowHoverImagePreviewStartDrag(bool isStart)
    {
        isShowHoverImagePreviewStartDrag_ = isStart;
    }

    bool GetIsShowHoverImagePreviewStartDrag()
    {
        return isShowHoverImagePreviewStartDrag_;
    }

    void RegisterMenuCallback(const RefPtr<FrameNode>& menuWrapperNode, const MenuParam& menuParam);

    void RegisterMenuAppearCallback(const std::function<void()>& onAppear)
    {
        onAppearCallback_ = onAppear;
    }

    void RegisterMenuDisappearCallback(const std::function<void()>& onDisappear)
    {
        onDisappearCallback_ = onDisappear;
    }

    void RegisterMenuAboutToAppearCallback(const std::function<void()>& aboutToAppear)
    {
        aboutToAppearCallback_ = aboutToAppear;
    }

    void RegisterMenuAboutToDisappearCallback(const std::function<void()>& aboutToDisappear)
    {
        aboutToDisappearCallback_ = aboutToDisappear;
    }

    void RegisterMenuStateChangeCallback(const std::function<void(const std::string&)>& callback)
    {
        onStateChangeCallback_ = callback;
    }

    void CallMenuAppearCallback()
    {
        if (onAppearCallback_) {
            onAppearCallback_();
        }
    }

    void CallMenuDisappearCallback()
    {
        if (onDisappearCallback_) {
            onDisappearCallback_();
        }
    }

    void CallMenuAboutToAppearCallback()
    {
        if (aboutToAppearCallback_) {
            aboutToAppearCallback_();
        }
    }

    void CallMenuAboutToDisappearCallback()
    {
        if (aboutToDisappearCallback_) {
            aboutToDisappearCallback_();
        }
    }

    void CallMenuStateChangeCallback(const std::string& value)
    {
        if (onStateChangeCallback_) {
            onStateChangeCallback_(value);
        }
    }

    const std::function<void()>& GetMenuDisappearCallback()
    {
        return onDisappearCallback_;
    }

    bool IsShow() const
    {
        return menuStatus_ == MenuStatus::ON_SHOW_ANIMATION || menuStatus_ == MenuStatus::SHOW;
    }

    void SetMenuStatus(MenuStatus value)
    {
        menuStatus_ = value;
    }

    MenuStatus GetMenuStatus() const
    {
        return menuStatus_;
    }

    bool HasTransitionEffect() const
    {
        return hasTransitionEffect_;
    }

    void SetHasTransitionEffect(bool hasTransitionEffect)
    {
        hasTransitionEffect_ = hasTransitionEffect;
    }

    void SetMenuTransitionEffect(const RefPtr<FrameNode>& menuWrapperNode, const MenuParam& menuParam);

    bool HasPreviewTransitionEffect() const
    {
        return hasPreviewTransitionEffect_;
    }

    void SetHasPreviewTransitionEffect(bool hasPreviewTransitionEffect)
    {
        hasPreviewTransitionEffect_ = hasPreviewTransitionEffect;
    }

    void SetFilterColumnNode(const RefPtr<FrameNode>& columnNode)
    {
        filterColumnNode_ = columnNode;
    }

    RefPtr<FrameNode> GetFilterColumnNode()
    {
        return filterColumnNode_;
    }

    void DumpInfo() override;

    MenuDumpInfo GetDumpInfo() const
    {
        return dumpInfo_;
    }

    void SetDumpInfo(const MenuDumpInfo& dumpInfo)
    {
        dumpInfo_.menuPreviewMode = dumpInfo.menuPreviewMode;
        dumpInfo_.menuType = dumpInfo.menuType;
        dumpInfo_.enableArrow = dumpInfo.enableArrow;
        dumpInfo_.targetNode = dumpInfo.targetNode;
        dumpInfo_.targetOffset = dumpInfo.targetOffset;
        dumpInfo_.targetSize = dumpInfo.targetSize;
        dumpInfo_.wrapperRect = dumpInfo.wrapperRect;
        dumpInfo_.previewBeginScale = dumpInfo.previewBeginScale;
        dumpInfo_.previewEndScale = dumpInfo.previewEndScale;
        dumpInfo_.top = dumpInfo.top;
        dumpInfo_.bottom = dumpInfo.bottom;
        dumpInfo_.globalLocation = dumpInfo.globalLocation;
        dumpInfo_.originPlacement = dumpInfo.originPlacement;
        dumpInfo_.finalPosition = dumpInfo.finalPosition;
        dumpInfo_.finalPlacement = dumpInfo.finalPlacement;
    }

    bool GetHasCustomRadius() const
    {
        return hasCustomRadius_;
    }

    void SetHasCustomRadius(bool hasCustomRadius)
    {
        hasCustomRadius_ = hasCustomRadius;
    }

    RefPtr<FrameNode> GetLastTouchItem()
    {
        return lastTouchItem_;
    }

    void SetLastTouchItem(const RefPtr<FrameNode>& lastTouchItem)
    {
        lastTouchItem_ = lastTouchItem;
    }

    int IncreaseEmbeddedSubMenuCount()
    {
        return ++embeddedSubMenuCount_;
    }
    
    int DecreaseEmbeddedSubMenuCount()
    {
        return --embeddedSubMenuCount_;
    }

    RefPtr<FrameNode> GetMenuChild(const RefPtr<UINode>& node);
    RefPtr<FrameNode> GetShowedSubMenu();
    bool IsSelectOverlayCustomMenu(const RefPtr<FrameNode>& menu) const;
    bool HasEmbeddedSubMenu();
    void UpdateMenuAnimation(const RefPtr<FrameNode>& host);
    bool HasStackSubMenu();
    int embeddedSubMenuCount_ = 0;
protected:
    void OnTouchEvent(const TouchEventInfo& info);
    void CheckAndShowAnimation();

private:
    bool AvoidKeyboard() const override
    {
        return false;
    }
    bool AvoidBottom() const override
    {
        return false;
    }
    void OnModifyDone() override;
    void InitFocusEvent();
    void OnAttachToFrameNode() override;
    void RegisterOnTouch();
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    void SetHotAreas(const RefPtr<LayoutWrapper>& layoutWrapper);
    void StartShowAnimation();
    void HandleInteraction(const TouchEventInfo& info);
    RectF GetMenuZone(RefPtr<UINode>& innerMenuNode);
    RefPtr<FrameNode> FindTouchedMenuItem(const RefPtr<UINode>& menuNode, const OffsetF& position);

    void HideMenu(const RefPtr<FrameNode>& menu);
    void HideMenu(const RefPtr<MenuPattern>& menuPattern, const RefPtr<FrameNode>& menu, const OffsetF& position);
    void SetExitAnimation(const RefPtr<FrameNode>& host);
    std::function<void()> onAppearCallback_ = nullptr;
    std::function<void()> onDisappearCallback_ = nullptr;
    std::function<void()> aboutToAppearCallback_ = nullptr;
    std::function<void()> aboutToDisappearCallback_ = nullptr;
    std::function<void(const std::string&)> onStateChangeCallback_ = nullptr;
    RefPtr<TouchEventImpl> onTouch_;
    RefPtr<FrameNode> lastTouchItem_ = nullptr;
    RefPtr<FrameNode> currentTouchItem_ = nullptr;
    // menuId in OverlayManager's map
    int32_t targetId_ = -1;

    AnimationOption animationOption_;
    Placement menuPlacement_ = Placement::NONE;
    bool isFirstShow_ = true;
    bool isShowInSubWindow_ = true;
    bool isShowHoverImage_ = false;
    bool isShowHoverImagePreviewStartDrag_ = false;
    MenuStatus menuStatus_ = MenuStatus::INIT;
    bool hasTransitionEffect_ = false;
    bool hasPreviewTransitionEffect_ = false;
    RefPtr<FrameNode> filterColumnNode_;
    MenuDumpInfo dumpInfo_;
    bool hasCustomRadius_ = false;
    ACE_DISALLOW_COPY_AND_MOVE(MenuWrapperPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_MENU_MENU_WRAPPER_PATTERN_H
