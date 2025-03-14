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

#include <type_traits>
#include "gtest/gtest.h"

#define private public
#define protected public

#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/render/mock_render_context.h"
#include "test/mock/core/rosen/mock_canvas.h"
#include "test/mock/core/rosen/testing_canvas.h"

#include "core/components/common/layout/constants.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/container_modal/container_modal_constants.h"
#include "core/components/select/select_theme.h"
#include "core/components/theme/shadow_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_model_ng.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/menu_item_group/menu_item_group_pattern.h"
#include "core/components_ng/pattern/menu/menu_item_group/menu_item_group_view.h"
#include "core/components_ng/pattern/menu/menu_model_ng.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_theme.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/menu/multi_menu_layout_algorithm.h"
#include "core/components_ng/pattern/menu/preview/menu_preview_layout_algorithm.h"
#include "core/components_ng/pattern/menu/preview/menu_preview_pattern.h"
#include "core/components_ng/pattern/menu/sub_menu_layout_algorithm.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/border_property.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/syntax/lazy_for_each_model.h"
#include "core/components_ng/syntax/lazy_layout_wrapper_builder.h"
#include "core/event/touch_event.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::Framework;

namespace OHOS::Ace::NG {
namespace {
const InspectorFilter filter;
constexpr int32_t TARGET_ID = 3;
constexpr MenuType TYPE = MenuType::MENU;
const std::string EMPTY_TEXT = "";
const std::string TEXT_TAG = "text";
const std::string MENU_TAG = "menu";
const std::string MENU_ITEM_TEXT = "menuItem";
const std::string MENU_ITEM_GROUP_TEXT = "menuItemGroup";
const std::string MENU_TOUCH_EVENT_TYPE = "1";
const DirtySwapConfig configDirtySwap = { false, false, false, false, true, false };
const std::string IMAGE_SRC_URL = "file://data/data/com.example.test/res/example.svg";

constexpr float FULL_SCREEN_WIDTH = 720.0f;
constexpr float FULL_SCREEN_HEIGHT = 1136.0f;
constexpr float TARGET_SIZE_WIDTH = 100.0f;
constexpr float TARGET_SIZE_HEIGHT = 100.0f;
constexpr float MENU_ITEM_SIZE_WIDTH = 100.0f;
constexpr float MENU_ITEM_SIZE_HEIGHT = 50.0f;

const SizeF FULL_SCREEN_SIZE(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
const std::vector<std::string> FONT_FAMILY_VALUE = {"cursive"};
const std::vector<SelectParam> CREATE_VALUE = { { "content1", "icon1" }, { "content2", "" },
    { "", "icon3" }, { "", "" } };
const std::vector<SelectParam> CREATE_VALUE_NEW = { { "content1_new", "" }, { "", "icon4_new" },
    { "", "" }, { "", "icon4_new" } };
} // namespace
class MenuWrapperTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void InitMenuWrapperTestNg();
    void InitMenuItemTestNg();
    PaintWrapper* GetPaintWrapper(RefPtr<MenuPaintProperty> paintProperty);
    RefPtr<FrameNode> GetPreviewMenuWrapper(
        SizeF itemSize = SizeF(0.0f, 0.0f), std::optional<MenuPreviewAnimationOptions> scaleOptions = std::nullopt);
    RefPtr<FrameNode> GetPreviewMenuWrapper2();
    RefPtr<FrameNode> menuFrameNode_;
    RefPtr<MenuAccessibilityProperty> menuAccessibilityProperty_;
    RefPtr<FrameNode> menuItemFrameNode_;
    RefPtr<MenuItemPattern> menuItemPattern_;
    RefPtr<MenuItemAccessibilityProperty> menuItemAccessibilityProperty_;
};

void MenuWrapperTestNg::SetUpTestCase() {}

void MenuWrapperTestNg::TearDownTestCase() {}

void MenuWrapperTestNg::SetUp()
{
    MockPipelineContext::SetUp();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<SelectTheme>()));
}

void MenuWrapperTestNg::TearDown()
{
    MockPipelineContext::TearDown();
    menuFrameNode_ = nullptr;
    menuAccessibilityProperty_ = nullptr;
    menuItemFrameNode_ = nullptr;
    menuItemPattern_ = nullptr;
    menuItemAccessibilityProperty_ = nullptr;
    SystemProperties::SetDeviceType(DeviceType::PHONE);
    ScreenSystemManager::GetInstance().dipScale_ = 1.0;
    SystemProperties::orientation_ = DeviceOrientation::PORTRAIT;
}

void MenuWrapperTestNg::InitMenuWrapperTestNg()
{
    menuFrameNode_ = FrameNode::GetOrCreateFrameNode(V2::MENU_TAG, ViewStackProcessor::GetInstance()->ClaimNodeId(),
        []() { return AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", TYPE); });
    ASSERT_NE(menuFrameNode_, nullptr);

    menuAccessibilityProperty_ = menuFrameNode_->GetAccessibilityProperty<MenuAccessibilityProperty>();
    ASSERT_NE(menuAccessibilityProperty_, nullptr);
}

void MenuWrapperTestNg::InitMenuItemTestNg()
{
    menuItemFrameNode_ = FrameNode::GetOrCreateFrameNode(V2::MENU_ITEM_ETS_TAG,
        ViewStackProcessor::GetInstance()->ClaimNodeId(), []() { return AceType::MakeRefPtr<MenuItemPattern>(); });
    ASSERT_NE(menuItemFrameNode_, nullptr);

    menuItemPattern_ = menuItemFrameNode_->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern_, nullptr);

    menuItemAccessibilityProperty_ = menuItemFrameNode_->GetAccessibilityProperty<MenuItemAccessibilityProperty>();
    ASSERT_NE(menuItemAccessibilityProperty_, nullptr);
}

PaintWrapper* MenuWrapperTestNg::GetPaintWrapper(RefPtr<MenuPaintProperty> paintProperty)
{
    WeakPtr<RenderContext> renderContext;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper* paintWrapper = new PaintWrapper(renderContext, geometryNode, paintProperty);
    return paintWrapper;
}

RefPtr<FrameNode> MenuWrapperTestNg::GetPreviewMenuWrapper(
    SizeF itemSize, std::optional<MenuPreviewAnimationOptions> scaleOptions)
{
    auto rootNode = FrameNode::CreateFrameNode(
        V2::ROOT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<RootPattern>());
    CHECK_NULL_RETURN(rootNode, nullptr);
    auto targetNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(targetNode, nullptr);
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textNode, nullptr);
    if (!(LessOrEqual(itemSize.Width(), 0.0) || LessOrEqual(itemSize.Height(), 0.0))) {
        auto itemGeometryNode = textNode->GetGeometryNode();
        CHECK_NULL_RETURN(itemGeometryNode, nullptr);
        itemGeometryNode->SetFrameSize(itemSize);
    }
    targetNode->MountToParent(rootNode);
    MenuParam menuParam;
    menuParam.type = MenuType::CONTEXT_MENU;
    menuParam.previewMode = MenuPreviewMode::CUSTOM;
    if (scaleOptions != std::nullopt) {
        menuParam.previewAnimationOptions = scaleOptions.value();
    }
    auto customNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(customNode, nullptr);
    auto customGeometryNode = customNode->GetGeometryNode();
    CHECK_NULL_RETURN(customGeometryNode, nullptr);
    customGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    auto menuWrapperNode =
        MenuView::Create(textNode, targetNode->GetId(), V2::TEXT_ETS_TAG, menuParam, true, customNode);
    return menuWrapperNode;
}

RefPtr<FrameNode> MenuWrapperTestNg::GetPreviewMenuWrapper2()
{
    auto rootNode = FrameNode::CreateFrameNode(
        V2::ROOT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<RootPattern>());
    CHECK_NULL_RETURN(rootNode, nullptr);
    auto targetNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(targetNode, nullptr);
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textNode, nullptr);
    targetNode->MountToParent(rootNode);
    targetNode->GetOrCreateGestureEventHub();
    MenuParam menuParam;
    menuParam.type = MenuType::CONTEXT_MENU;
    menuParam.previewMode = MenuPreviewMode::IMAGE;
    auto customNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(customNode, nullptr);
    auto customGeometryNode = customNode->GetGeometryNode();
    CHECK_NULL_RETURN(customGeometryNode, nullptr);
    customGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    auto menuWrapperNode =
        MenuView::Create(textNode, targetNode->GetId(), V2::TEXT_ETS_TAG, menuParam, true, customNode);
    return menuWrapperNode;
}
/**
 * @tc.name: MenuWrapperPatternTestNg001
 * @tc.desc: Verify HideMenu(Menu).
 * @tc.type: FUNC
 */
HWTEST_F(MenuWrapperTestNg, MenuWrapperPatternTestNg001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create wrapper and child menu
     * @tc.expected: wrapper pattern not null
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto subMenuFirst = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, 3, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::SUB_MENU));
    auto subMenuSecond = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, 4, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::SUB_MENU));
    mainMenu->MountToParent(wrapperNode);
    subMenuFirst->MountToParent(wrapperNode);
    subMenuSecond->MountToParent(wrapperNode);
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    /**
     * @tc.steps: step2. excute HideMenu
     * @tc.expected: wrapper child size is 3
     */
    wrapperPattern->HideMenu(mainMenu);
    wrapperPattern->OnModifyDone();
    EXPECT_EQ(wrapperNode->GetChildren().size(), 3);
}

/**
 * @tc.name: MenuWrapperPatternTestNg002
 * @tc.desc: Verify HideMenu().
 * @tc.type: FUNC
 */
HWTEST_F(MenuWrapperTestNg, MenuWrapperPatternTestNg002, TestSize.Level1)
{
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    mainMenu->MountToParent(wrapperNode);
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    wrapperPattern->HideMenu();
    EXPECT_EQ(wrapperNode->GetChildren().size(), 1);
}

/**
 * @tc.name: MenuWrapperPatternTestNg003
 * @tc.desc: Verify HideSubMenu.
 * @tc.type: FUNC
 */
HWTEST_F(MenuWrapperTestNg, MenuWrapperPatternTestNg003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create wrapper
     * @tc.expected: wrapper pattern not null
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    wrapperPattern->HideSubMenu();
    ASSERT_NE(wrapperPattern, nullptr);
    /**
     * @tc.steps: step2. add submenu to wrapper
     * @tc.expected: wrapper child size is 2
     */
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto subMenu = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::SUB_MENU));
    mainMenu->MountToParent(wrapperNode);
    wrapperPattern->HideSubMenu();
    subMenu->MountToParent(wrapperNode);
    wrapperPattern->HideSubMenu();
    EXPECT_EQ(wrapperNode->GetChildren().size(), 2);
}

/**
 * @tc.name: MenuWrapperPatternTestNg004
 * @tc.desc: Verify HandleMouseEvent.
 * @tc.type: FUNC
 */
HWTEST_F(MenuWrapperTestNg, MenuWrapperPatternTestNg004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create menuItem and mouseInfo
     * @tc.expected: menuItem and mouseInfo function result as expected
     */
    MouseInfo mouseInfo;
    mouseInfo.SetAction(MouseAction::PRESS);
    mouseInfo.SetGlobalLocation(Offset(200, 200));
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 100, AceType::MakeRefPtr<MenuItemPattern>());
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    menuItemPattern->SetIsSubMenuShowed(true);
    menuItemPattern->AddHoverRegions(OffsetF(0, 0), OffsetF(100, 100));
    const auto& mousePosition = mouseInfo.GetGlobalLocation();
    EXPECT_TRUE(!menuItemPattern->IsInHoverRegions(mousePosition.GetX(), mousePosition.GetY()));
    EXPECT_TRUE(menuItemPattern->IsSubMenuShowed());
    /**
     * @tc.steps: step2. Create menuWrapper
     * @tc.expected: wrapperPattern is not null
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    wrapperPattern->HandleMouseEvent(mouseInfo, menuItemPattern);
    ASSERT_NE(wrapperPattern, nullptr);
    /**
     * @tc.steps: step3. add submenu to wrapper
     * @tc.expected: wrapper child size is 2
     */
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto subMenu = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::SUB_MENU));
    auto currentMenuItemNode =
        FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 101, AceType::MakeRefPtr<MenuItemPattern>());
    auto currentMenuItemPattern = currentMenuItemNode->GetPattern<MenuItemPattern>();
    auto subMenuPattern = subMenu->GetPattern<MenuPattern>();
    currentMenuItemPattern->SetIsSubMenuShowed(true);
    mainMenu->MountToParent(wrapperNode);
    subMenu->MountToParent(wrapperNode);
    EXPECT_EQ(wrapperNode->GetChildren().size(), 2);
    /**
     * @tc.steps: step4. execute HandleMouseEvent
     * @tc.expected: menuItemPattern IsSubMenuShowed as expected
     */
    subMenuPattern->SetParentMenuItem(currentMenuItemNode);
    wrapperPattern->HandleMouseEvent(mouseInfo, menuItemPattern);
    EXPECT_TRUE(currentMenuItemPattern->IsSubMenuShowed());
    subMenuPattern->SetParentMenuItem(menuItemNode);
    wrapperPattern->HandleMouseEvent(mouseInfo, menuItemPattern);
    EXPECT_FALSE(menuItemPattern->IsSubMenuShowed());
}

/**
 * @tc.name: MenuWrapperPatternTestNg005
 * @tc.desc: Verify OnTouchEvent.
 * @tc.type: FUNC
 */
HWTEST_F(MenuWrapperTestNg, MenuWrapperPatternTestNg005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create touchEventInfo
     * @tc.expected: touchEventInfo size is 1
     */
    TouchEventInfo contextMenuTouchUpEventInfo(MENU_TOUCH_EVENT_TYPE);
    TouchLocationInfo upLocationInfo(TARGET_ID);
    Offset touchUpGlobalLocation(80, 80);
    upLocationInfo.SetTouchType(TouchType::MOVE);
    auto touchUpLocationInfo = upLocationInfo.SetGlobalLocation(touchUpGlobalLocation);
    contextMenuTouchUpEventInfo.touches_.emplace_back(touchUpLocationInfo);
    EXPECT_EQ(contextMenuTouchUpEventInfo.touches_.size(), 1);
    /**
     * @tc.steps: step2. update touchEventInfo, excute OnTouchEvent
     * @tc.expected: touchEventInfo size is 1
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    wrapperPattern->OnTouchEvent(contextMenuTouchUpEventInfo);
    upLocationInfo.SetTouchType(TouchType::DOWN);
    touchUpLocationInfo = upLocationInfo.SetGlobalLocation(touchUpGlobalLocation);
    contextMenuTouchUpEventInfo.touches_.clear();
    contextMenuTouchUpEventInfo.touches_.emplace_back(touchUpLocationInfo);
    wrapperPattern->OnTouchEvent(contextMenuTouchUpEventInfo);
    wrapperPattern->SetMenuStatus(MenuStatus::HIDE);
    wrapperPattern->OnTouchEvent(contextMenuTouchUpEventInfo);
    wrapperPattern->SetMenuStatus(MenuStatus::SHOW);
    wrapperPattern->OnTouchEvent(contextMenuTouchUpEventInfo);
    EXPECT_EQ(contextMenuTouchUpEventInfo.touches_.size(), 1);
    /**
     * @tc.steps: step3. add submenu to wrapper,execute OnTouchEvent
     * @tc.expected: wrapper child size is 2
     */
    auto topMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto bottomMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 3, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto subMenu = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, 4, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::SUB_MENU));
    topMenu->GetGeometryNode()->SetFrameSize(SizeF(100, 100));
    topMenu->MountToParent(wrapperNode);
    bottomMenu->GetGeometryNode()->SetFrameSize(SizeF(70, 70));
    bottomMenu->MountToParent(wrapperNode);
    subMenu->GetGeometryNode()->SetFrameSize(SizeF(70, 70));
    subMenu->MountToParent(wrapperNode);
    wrapperPattern->OnTouchEvent(contextMenuTouchUpEventInfo);
    wrapperPattern->HideMenu();
    EXPECT_EQ(wrapperNode->GetChildren().size(), 3);
}

/**
 * @tc.name: MenuWrapperPatternTestNg006
 * @tc.desc: Verify HideSubMenu.
 * @tc.type: FUNC
 */
HWTEST_F(MenuWrapperTestNg, MenuWrapperPatternTestNg006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create wrapper
     * @tc.expected: wrapper pattern not null
     */
    auto menuItemGroupPattern = AceType::MakeRefPtr<MenuItemGroupPattern>();
    menuItemGroupPattern->hasSelectIcon_ = true;
    auto menuItemGroup = FrameNode::CreateFrameNode(V2::MENU_ITEM_GROUP_ETS_TAG, -1, menuItemGroupPattern);
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    menuItemGroup->MountToParent(wrapperNode);
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    wrapperPattern->OnModifyDone();
    auto algorithm = AceType::MakeRefPtr<MenuItemGroupLayoutAlgorithm>(-1, -1, 0);
    ASSERT_TRUE(algorithm);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto layoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto* layoutWrapperNode = new LayoutWrapperNode(menuItemGroup, geometryNode, layoutProp);
    RefPtr<LayoutWrapper> layoutWrapper = layoutWrapperNode->GetOrCreateChildByIndex(0, false);
    EXPECT_EQ(layoutWrapper, nullptr);
    /**
     * @tc.steps: step2. create firstChildLayoutWrapper and append it to layoutWrapper
     */
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    auto menuItem = AceType::MakeRefPtr<FrameNode>("", -1, itemPattern);
    auto itemGeoNode = AceType::MakeRefPtr<GeometryNode>();
    itemGeoNode->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto firstChildLayoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(menuItem, itemGeoNode, layoutProp);
    layoutWrapperNode->AppendChild(firstChildLayoutWrapper);
    layoutWrapper = layoutWrapperNode->GetOrCreateChildByIndex(0, false);
    EXPECT_EQ(layoutWrapper, firstChildLayoutWrapper);
    /**
     * @tc.steps: step3. add submenu to wrapper
     * @tc.expected: wrapper child size is 1
     */
    wrapperPattern->isFirstShow_ = true;
    EXPECT_FALSE(wrapperPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, configDirtySwap));
    wrapperPattern->SetHotAreas(layoutWrapper);
}

/**
 * @tc.name: MenuWrapperPatternTestNg006
 * @tc.desc: Verify HideSubMenu.
 * @tc.type: FUNC
 */
HWTEST_F(MenuWrapperTestNg, MenuWrapperPatternTestNg007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create wrapper
     * @tc.expected: wrapper pattern not null
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto layoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto menuItemGroupPattern = AceType::MakeRefPtr<MenuItemGroupPattern>();
    auto menuItemGroup = FrameNode::CreateFrameNode(V2::MENU_ITEM_GROUP_ETS_TAG, -1, menuItemGroupPattern);
    auto* layoutWrapperNode = new LayoutWrapperNode(menuItemGroup, geometryNode, layoutProp);
    RefPtr<LayoutWrapper> layoutWrapper = layoutWrapperNode->GetOrCreateChildByIndex(0, false);
    EXPECT_EQ(layoutWrapper, nullptr);
    /**
     * @tc.steps: step2. create firstChildLayoutWrapper and append it to layoutWrapper
     */
    RefPtr<MenuPattern> menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", TYPE);
    menuPattern->isSelectMenu_ = true;
    menuPattern->SetType(MenuType::CONTEXT_MENU);
    auto menuItem = AceType::MakeRefPtr<FrameNode>("", -1, menuPattern);
    menuItem->MountToParent(wrapperNode);
    auto itemGeoNode = AceType::MakeRefPtr<GeometryNode>();
    itemGeoNode->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto firstChildLayoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(menuItem, itemGeoNode, layoutProp);

    layoutWrapperNode->AppendChild(firstChildLayoutWrapper);
    layoutWrapper = layoutWrapperNode->GetOrCreateChildByIndex(0, false);
    EXPECT_EQ(layoutWrapper, firstChildLayoutWrapper);
    /**
     * @tc.steps: step3. add submenu to wrapper
     * @tc.expected: wrapper child size is 1
     */
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    wrapperPattern->isFirstShow_ = true;
    EXPECT_FALSE(wrapperPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, configDirtySwap));
    wrapperPattern->GetMenu()->GetPattern<MenuPattern>()->SetType(MenuType::CONTEXT_MENU);
    wrapperPattern->SetHotAreas(layoutWrapper);
}

/**
 * @tc.name: MenuWrapperPatternTestNg008
 * @tc.desc: CallMenuStateChangeCallback (Menu).
 * @tc.type: FUNC
 */
HWTEST_F(MenuWrapperTestNg, MenuWrapperPatternTestNg008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create wrapper and child menu
     * @tc.expected: wrapper pattern not null
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));

    mainMenu->MountToParent(wrapperNode);
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    /**
     * @tc.steps: step2. execute HideMenu
     * @tc.expected: wrapper child size is 3
     */
    int32_t callNum = 0;
    std::function<void(const std::string&)> callback = [&](const std::string& param) { callNum++; };
    wrapperPattern->RegisterMenuStateChangeCallback(callback);
    wrapperPattern->CallMenuStateChangeCallback("false");
    EXPECT_EQ(callNum, 1);
}

/**
 * @tc.name: MenuWrapperPatternTestNg009
 * @tc.desc: test MenuWrapperPattern::CheckAndShowAnimation function.
 * @tc.type: FUNC
 */
HWTEST_F(MenuWrapperTestNg, MenuWrapperPatternTestNg009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create wrapper and child menu
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));

    mainMenu->MountToParent(wrapperNode);
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    /**
     * @tc.steps: step2. execute CheckAndShowAnimation
     * @tc.expected: property is set as expected
     */
    wrapperPattern->isFirstShow_ = false;
    wrapperPattern->CheckAndShowAnimation();
    wrapperPattern->isFirstShow_ = true;
    EXPECT_TRUE(wrapperPattern->isFirstShow_);
    wrapperPattern->CheckAndShowAnimation();
    EXPECT_FALSE(wrapperPattern->isFirstShow_);
}

/**
 * @tc.name: MenuWrapperPatternTestNg010
 * @tc.desc: test MenuWrapperPattern::GetAnimationOffset function.
 * @tc.type: FUNC
 */
HWTEST_F(MenuWrapperTestNg, MenuWrapperPatternTestNg010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create wrapper and child menu
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));

    auto prop = AceType::MakeRefPtr<MenuLayoutProperty>();
    mainMenu->SetLayoutProperty(prop);
    mainMenu->MountToParent(wrapperNode);
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    /**
     * @tc.steps: step2. execute GetAnimationOffset
     * @tc.expected: property is set as expected
     */
    wrapperPattern->menuPlacement_ = Placement::LEFT;
    wrapperPattern->GetAnimationOffset();
    EXPECT_EQ(wrapperPattern->menuPlacement_, Placement::LEFT);
    wrapperPattern->menuPlacement_ = Placement::RIGHT;
    wrapperPattern->GetAnimationOffset();
    EXPECT_EQ(wrapperPattern->menuPlacement_, Placement::RIGHT);
    wrapperPattern->menuPlacement_ = Placement::TOP;
    wrapperPattern->GetAnimationOffset();
    EXPECT_EQ(wrapperPattern->menuPlacement_, Placement::TOP);
}

/**
 * @tc.name: MenuWrapperPatternTestNg011
 * @tc.desc: Test Verify interaction effect.
 * @tc.type: FUNC
 */
HWTEST_F(MenuWrapperTestNg, MenuWrapperPatternTestNg011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. set API12.
     */
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto wrapPattern = menuWrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapPattern, nullptr);
    TouchEventInfo info(MENU_TOUCH_EVENT_TYPE);
    TouchLocationInfo locationInfo(TARGET_ID);
    Offset location(1, 1);
    locationInfo.SetTouchType(TouchType::MOVE);
    locationInfo.SetLocalLocation(location);
    info.touches_.emplace_back(locationInfo);
    /**
     * @tc.steps: step2. receive event and test hover event.
     */
    wrapPattern->OnTouchEvent(info);
    EXPECT_EQ(wrapPattern->currentTouchItem_, nullptr);

    auto menuframeNode = wrapPattern->GetMenuChild(menuWrapperNode);
    EXPECT_NE(menuframeNode, nullptr);
    /**
     * @tc.steps: step3. create menuitem and test longpress event.
     */
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 100, AceType::MakeRefPtr<MenuItemPattern>());
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    menuItemPattern->InitLongPressEvent();
    ASSERT_NE(menuItemPattern->longPressEvent_, nullptr);
    GestureEvent gestureEvent;
    gestureEvent.offsetY_ = 1.0;
    (*menuItemPattern->longPressEvent_)(gestureEvent);
    ASSERT_FALSE(menuItemPattern->isSubMenuShowed_);
}
/**
 * @tc.name: MenuWrapperPatternTestNg012
 * @tc.desc: Verify HideMenu(Menu).
 * @tc.type: FUNC
 */
HWTEST_F(MenuWrapperTestNg, MenuWrapperPatternTestNg012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create wrapper and child menu
     * @tc.expected: wrapper pattern not null
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::SELECT_OVERLAY_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto mainMenu = FrameNode::CreateFrameNode(
        V2::SELECT_OVERLAY_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    mainMenu->MountToParent(wrapperNode);
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    /**
     * @tc.steps: step2. excute HideMenu
     * @tc.expected: wrapper pattern return
     */
    wrapperPattern->HideMenu(mainMenu);
    EXPECT_TRUE(mainMenu->GetTag() == V2::SELECT_OVERLAY_ETS_TAG);
}
/**
 * @tc.name: MenuViewTestNg001
 * @tc.desc: Test Verify Create.
 * @tc.type: FUNC
 */
HWTEST_F(MenuWrapperTestNg, MenuViewTestNg001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. set API12.
     */
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    auto menuWrapperNode = GetPreviewMenuWrapper2();
}
} // namespace OHOS::Ace::NG
