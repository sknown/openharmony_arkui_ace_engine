/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <optional>

#include "gtest/gtest.h"
#include "mock_navigation_route.h"
#include "mock_navigation_stack.h"

#include "base/memory/ace_type.h"
#include "core/components_ng/animation/geometry_transition.h"

#define protected public
#define private public
#include "base/json/json_util.h"
#include "test/mock/base/mock_task_executor.h"
#include "core/animation/animator.h"
#include "core/components/button/button_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/manager/navigation/navigation_manager.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/button/toggle_button_model_ng.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/navigation/bar_item_node.h"
#include "core/components_ng/pattern/navigation/bar_item_pattern.h"
#include "core/components_ng/pattern/navigation/nav_bar_layout_property.h"
#include "core/components_ng/pattern/navigation/nav_bar_node.h"
#include "core/components_ng/pattern/navigation/nav_bar_pattern.h"
#include "core/components_ng/pattern/navigation/navigation_content_layout_algorithm.h"
#include "core/components_ng/pattern/navigation/navigation_group_node.h"
#include "core/components_ng/pattern/navigation/navigation_layout_property.h"
#include "core/components_ng/pattern/navigation/navigation_model_ng.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "core/components_ng/pattern/navigation/navigation_stack.h"
#include "core/components_ng/pattern/navigation/title_bar_layout_property.h"
#include "core/components_ng/pattern/navigation/title_bar_node.h"
#include "core/components_ng/pattern/navigation/title_bar_pattern.h"
#include "core/components_ng/pattern/navigation/tool_bar_layout_algorithm.h"
#include "core/components_ng/pattern/navigation/tool_bar_node.h"
#include "core/components_ng/pattern/navigation/tool_bar_pattern.h"
#include "core/components_ng/pattern/navigator/navigator_event_hub.h"
#include "core/components_ng/pattern/navigator/navigator_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_algorithm.h"
#include "core/components_ng/pattern/navrouter/navdestination_model.h"
#include "core/components_ng/pattern/navrouter/navdestination_model_ng.h"
#include "core/components_ng/pattern/stage/page_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_ng/pattern/navrouter/navrouter_event_hub.h"
#include "core/components_ng/pattern/navrouter/navrouter_group_node.h"
#include "core/components_ng/pattern/navrouter/navrouter_model.h"
#include "core/components_ng/pattern/navrouter/navrouter_model_ng.h"
#include "core/components_ng/pattern/navrouter/navrouter_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/stack/stack_layout_algorithm.h"
#include "core/components_ng/pattern/stack/stack_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/common/mock_container.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
const InspectorFilter filter;
const std::string TEST_TAG = "test";
constexpr float DEFAULT_ROOT_HEIGHT = 800.f;
constexpr float DEFAULT_ROOT_WIDTH = 480.f;
} // namespace

class NavigationModelTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void MockPipelineContextGetTheme();
    static void RunMeasureAndLayout(RefPtr<LayoutWrapperNode>& layoutWrapper, float width = DEFAULT_ROOT_WIDTH);
};

void NavigationModelTestNg::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
}

void NavigationModelTestNg::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

void NavigationModelTestNg::RunMeasureAndLayout(RefPtr<LayoutWrapperNode>& layoutWrapper, float width)
{
    layoutWrapper->SetActive();
    layoutWrapper->SetRootMeasureNode();
    LayoutConstraintF LayoutConstraint;
    LayoutConstraint.parentIdealSize = { width, DEFAULT_ROOT_HEIGHT };
    LayoutConstraint.percentReference = { width, DEFAULT_ROOT_HEIGHT };
    LayoutConstraint.selfIdealSize = { width, DEFAULT_ROOT_HEIGHT };
    LayoutConstraint.maxSize = { width, DEFAULT_ROOT_HEIGHT };
    layoutWrapper->Measure(LayoutConstraint);
    layoutWrapper->Layout();
    layoutWrapper->MountToHostOnMainThread();
}

void NavigationModelTestNg::MockPipelineContextGetTheme()
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<NavigationBarTheme>()));
}

/**
 * @tc.name: UpdateOldBarItems001
 * @tc.desc: Test UpdateOldBarItems and cover all conditions in for loop.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, UpdateOldBarItems001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    // Create four new BarItems with different attributes
    std::vector<NG::BarItem> toolBarItems;
    // Make newChildrenSize 4
    NG::BarItem newBar1, newBar2, newBar3, newBar4;
    newBar3.text = "text";
    newBar3.icon = "icon";
    newBar4.text = "text";
    newBar4.icon = "icon";
    toolBarItems.push_back(newBar1);
    toolBarItems.push_back(newBar2);
    toolBarItems.push_back(newBar3);
    toolBarItems.push_back(newBar4);
    // Make prevChildrenSize 4
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    EXPECT_FALSE(navBarNode->GetPrevToolBarIsCustom().value_or(false));
    // Create four old BarItemNodes with different attributes
    auto oldBar1 = AceType::MakeRefPtr<BarItemNode>(V2::BAR_ITEM_ETS_TAG, 101);
    auto oldBar2 = AceType::MakeRefPtr<BarItemNode>(V2::BAR_ITEM_ETS_TAG, 102);
    oldBar2->text_ = FrameNode::CreateFrameNode("text", 201, AceType::MakeRefPtr<TextPattern>());
    oldBar2->icon_ = FrameNode::CreateFrameNode("image", 301, AceType::MakeRefPtr<ImagePattern>());
    auto oldBar3 = AceType::MakeRefPtr<BarItemNode>(V2::BAR_ITEM_ETS_TAG, 103);
    // Make frameNode_ not NULL or crash will happen in Pattern::OnModifyDone
    ASSERT_NE(oldBar3->pattern_, nullptr);
    oldBar3->pattern_->frameNode_ = oldBar3;
    auto oldBar4 = AceType::MakeRefPtr<BarItemNode>(V2::BAR_ITEM_ETS_TAG, 104);
    oldBar4->text_ = FrameNode::CreateFrameNode("text", 202, AceType::MakeRefPtr<TextPattern>());
    oldBar4->icon_ = FrameNode::CreateFrameNode("image", 302, AceType::MakeRefPtr<ImagePattern>());
    auto preToolBarNode = navBarNode->GetPreToolBarNode();
    ASSERT_NE(preToolBarNode, nullptr);
    preToolBarNode->children_.emplace_back(oldBar1);
    preToolBarNode->children_.emplace_back(oldBar2);
    preToolBarNode->children_.emplace_back(oldBar3);
    preToolBarNode->children_.emplace_back(oldBar4);
    navigationModel.SetToolBarItems(std::move(toolBarItems));
}

/**
 * @tc.name: UpdateOldBarItems002
 * @tc.desc: Test UpdateOldBarItems and cover all conditions outside for loop.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, UpdateOldBarItems002, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    // Make newChildrenSize 1
    // Mkae newBarItem not NULL
    std::vector<NG::BarItem> toolBarItems;
    NG::BarItem newBar1;
    toolBarItems.push_back(newBar1);
    // Make prevChildrenSize 1
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    EXPECT_FALSE(navBarNode->GetPrevToolBarIsCustom().value_or(false));
    // Create an old BarItemNode with different attributes
    auto oldBar1 = AceType::MakeRefPtr<BarItemNode>(V2::BAR_ITEM_ETS_TAG, 101);
    auto preToolBarNode = navBarNode->GetPreToolBarNode();
    ASSERT_NE(preToolBarNode, nullptr);
    preToolBarNode->children_.emplace_back(oldBar1);
    navigationModel.SetToolBarItems(std::move(toolBarItems));

    // Make newChildrenSize 2 and prevChildrenSize 1
    NG::BarItem newBar2;
    toolBarItems.push_back(newBar2);
    navigationModel.SetToolBarItems(std::move(toolBarItems));

    // Make newChildrenSize 2 and prevChildrenSize 3
    auto oldBar2 = AceType::MakeRefPtr<BarItemNode>(V2::BAR_ITEM_ETS_TAG, 102);
    auto oldBar3 = AceType::MakeRefPtr<BarItemNode>(V2::BAR_ITEM_ETS_TAG, 103);
    preToolBarNode->children_.emplace_back(oldBar2);
    preToolBarNode->children_.emplace_back(oldBar3);
    navigationModel.SetToolBarItems(std::move(toolBarItems));
}

/**
 * @tc.name: SetToolbarConfiguration001
 * @tc.desc: Test SetToolbarConfiguration and cover all conditions of "GetPrevToolBarIsCustom.value_or".
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetToolbarConfiguration001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    // Make GetPrevToolBarIsCustom.value_or return false
    navBarNode->propPrevToolBarIsCustom_ = false;
    EXPECT_FALSE(navBarNode->GetPrevToolBarIsCustom().value_or(false));
    auto toolbarNode = AceType::DynamicCast<NavToolbarNode>(navBarNode->GetPreToolBarNode());
    ASSERT_NE(toolbarNode, nullptr);
    EXPECT_EQ(toolbarNode->GetToolbarContainerNode(), nullptr);
    std::vector<NG::BarItem> toolBarItems;
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));

    // // Make containerNode not NULL
    toolbarNode->toolbarContainerNode_ = FrameNode::CreateFrameNode("text", 101, AceType::MakeRefPtr<TextPattern>());
    EXPECT_NE(toolbarNode->GetToolbarContainerNode(), nullptr);
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));

    navBarNode->propPrevToolBarIsCustom_ = true;
    EXPECT_TRUE(navBarNode->GetPrevToolBarIsCustom().value_or(false));
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));
}

/**
 * @tc.name: SetToolbarConfiguration002
 * @tc.desc: Test SetToolbarConfiguration and cover all conditions of "LessThanAPIVersion".
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetToolbarConfiguration002, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    std::vector<NG::BarItem> toolBarItems;
    auto context = PipelineBase::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    int32_t preVersion = context->GetMinPlatformVersion();
    context->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN) - 1;
    EXPECT_TRUE(Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN));
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));

    context->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN) + 1;
    EXPECT_FALSE(Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN));
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));
    context->minPlatformVersion_ = preVersion;
    EXPECT_EQ(preVersion, context->GetMinPlatformVersion());
}

/**
 * @tc.name: SetToolbarConfiguration003
 * @tc.desc: Test SetToolbarConfiguration and cover all conditions of "LessThanAPIVersion".
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetToolbarConfiguration003, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    std::vector<NG::BarItem> toolBarItems;
    NG::BarItem newBar1;
    toolBarItems.push_back(newBar1);
    EXPECT_TRUE(toolBarItems.size() <= MAXIMUM_TOOLBAR_ITEMS_IN_BAR);
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));

    NG::BarItem newBars[MAXIMUM_TOOLBAR_ITEMS_IN_BAR];
    toolBarItems.insert(toolBarItems.end(), std::begin(newBars), std::end(newBars));
    EXPECT_TRUE(toolBarItems.size() > MAXIMUM_TOOLBAR_ITEMS_IN_BAR);
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));
}

/**
 * @tc.name: RegisterToolbarHotZoneEvent001
 * @tc.desc: Test RegisterToolbarHotZoneEvent and cover all conditions of the event callback.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, RegisterToolbarHotZoneEvent001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    std::vector<NG::BarItem> toolBarItems;
    NG::BarItem newBars[MAXIMUM_TOOLBAR_ITEMS_IN_BAR + 1];
    toolBarItems.insert(toolBarItems.end(), std::begin(newBars), std::end(newBars));
    EXPECT_TRUE(toolBarItems.size() > MAXIMUM_TOOLBAR_ITEMS_IN_BAR);
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));

    // Get the event callback defined in RegisterToolbarHotZoneEvent
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto toolBarNode = AceType::DynamicCast<NavToolbarNode>(navBarNode->GetPreToolBarNode());
    ASSERT_NE(toolBarNode, nullptr);
    ASSERT_FALSE(toolBarNode->children_.empty());
    auto containerNode = toolBarNode->children_.back();
    ASSERT_NE(containerNode, nullptr);
    ASSERT_FALSE(containerNode->children_.empty());
    auto toolBarItemNode = AceType::DynamicCast<FrameNode>(containerNode->children_.back());
    ASSERT_NE(toolBarItemNode, nullptr);
    auto gestureEventHub = toolBarItemNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureEventHub->clickEventActuator_, nullptr);
    auto event = gestureEventHub->clickEventActuator_->GetClickEvent();
    ASSERT_NE(event, nullptr);

    GestureEvent gestureEvent;
    EXPECT_NE(gestureEvent.GetSourceDevice(), SourceType::KEYBOARD);
    event(gestureEvent);
    gestureEvent.deviceType_ = SourceType::KEYBOARD;
    EXPECT_EQ(gestureEvent.GetSourceDevice(), SourceType::KEYBOARD);
    event(gestureEvent);
}

/**
 * @tc.name: CreateToolbarItemInContainer001
 * @tc.desc: Test CreateToolbarItemInContainer and cover all conditions (1st).
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, CreateToolbarItemInContainer001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto eventHub = navigationGroupNode->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    eventHub->enabled_ = true;
    std::vector<NG::BarItem> toolBarItems;
    NG::BarItem newBars[MAXIMUM_TOOLBAR_ITEMS_IN_BAR > 3 ? MAXIMUM_TOOLBAR_ITEMS_IN_BAR : 3];
    newBars[1].text = "";
    newBars[1].icon = "";
    newBars[1].action = []() {};
    newBars[1].status = NG::NavToolbarItemStatus::DISABLED;
    newBars[2].text = "text";
    newBars[2].icon = "icon";
    newBars[2].status = NG::NavToolbarItemStatus::ACTIVE;
    toolBarItems.insert(toolBarItems.end(), std::begin(newBars), std::end(newBars));

    EXPECT_FALSE(toolBarItems.size() > MAXIMUM_TOOLBAR_ITEMS_IN_BAR);
    EXPECT_FALSE(newBars[0].text.has_value());
    EXPECT_FALSE(newBars[0].icon.has_value());
    EXPECT_EQ(newBars[0].action, nullptr);
    EXPECT_NE(newBars[0].status, NG::NavToolbarItemStatus::DISABLED);
    EXPECT_TRUE(newBars[1].text.has_value() && newBars[1].text.value().empty());
    EXPECT_TRUE(newBars[1].icon.has_value() && newBars[1].icon.value().empty());
    EXPECT_NE(newBars[1].action, nullptr);
    EXPECT_EQ(newBars[1].status, NG::NavToolbarItemStatus::DISABLED);
    EXPECT_TRUE(newBars[2].text.has_value() && !newBars[2].text.value().empty());
    EXPECT_TRUE(newBars[2].icon.has_value() && !newBars[2].icon.value().empty());
    EXPECT_EQ(newBars[2].status, NG::NavToolbarItemStatus::ACTIVE);
    EXPECT_FALSE(newBars[2].activeIcon.has_value());
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));
}

/**
 * @tc.name: CreateToolbarItemInContainer002
 * @tc.desc: Test CreateToolbarItemInContainer and cover all conditions (2td).
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, CreateToolbarItemInContainer002, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto eventHub = navigationGroupNode->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    eventHub->enabled_ = false;
    std::vector<NG::BarItem> toolBarItems;
    NG::BarItem newBars[MAXIMUM_TOOLBAR_ITEMS_IN_BAR > 3 ? MAXIMUM_TOOLBAR_ITEMS_IN_BAR : 3];
    newBars[0].activeIcon = "";
    newBars[0].status = NG::NavToolbarItemStatus::ACTIVE;
    newBars[1].activeIcon = "activeIcon";
    newBars[1].status = NG::NavToolbarItemStatus::ACTIVE;
    newBars[2].activeIcon = "activeIcon";
    newBars[2].icon = "";
    newBars[2].status = NG::NavToolbarItemStatus::ACTIVE;
    toolBarItems.insert(toolBarItems.end(), std::begin(newBars), std::end(newBars));

    EXPECT_EQ(newBars[0].status, NG::NavToolbarItemStatus::ACTIVE);
    EXPECT_TRUE(newBars[0].activeIcon.has_value() && newBars[0].activeIcon.value().empty());
    EXPECT_EQ(newBars[1].status, NG::NavToolbarItemStatus::ACTIVE);
    EXPECT_TRUE(newBars[1].activeIcon.has_value() && !newBars[1].activeIcon.value().empty());
    EXPECT_FALSE(newBars[1].icon.has_value());
    EXPECT_EQ(newBars[2].status, NG::NavToolbarItemStatus::ACTIVE);
    EXPECT_TRUE(newBars[2].activeIcon.has_value() && !newBars[2].activeIcon.value().empty());
    EXPECT_TRUE(newBars[2].icon.has_value() && newBars[2].icon.value().empty());
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));
}

/**
 * @tc.name: CreateToolbarItemInContainer003
 * @tc.desc: Test CreateToolbarItemInContainer and cover all conditions (3th).
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, CreateToolbarItemInContainer003, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    std::vector<NG::BarItem> toolBarItems;
    NG::BarItem newBars[MAXIMUM_TOOLBAR_ITEMS_IN_BAR > 1 ? MAXIMUM_TOOLBAR_ITEMS_IN_BAR : 1];
    newBars[0].activeIcon = "activeIcon";
    newBars[0].icon = "icon";
    newBars[0].status = NG::NavToolbarItemStatus::ACTIVE;
    toolBarItems.insert(toolBarItems.end(), std::begin(newBars), std::end(newBars));

    EXPECT_EQ(newBars[0].status, NG::NavToolbarItemStatus::ACTIVE);
    EXPECT_TRUE(newBars[0].activeIcon.has_value() && !newBars[0].activeIcon.value().empty());
    EXPECT_TRUE(newBars[0].icon.has_value() && !newBars[0].icon.value().empty());
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));
}

/**
 * @tc.name: BuildToolbarMoreItemNode001
 * @tc.desc: Test BuildToolbarMoreItemNode and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, BuildToolbarMoreItemNode001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto eventHub = navigationGroupNode->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    eventHub->enabled_ = true;
    std::vector<NG::BarItem> toolBarItems;
    NG::BarItem newBars[MAXIMUM_TOOLBAR_ITEMS_IN_BAR + 1];
    toolBarItems.insert(toolBarItems.end(), std::begin(newBars), std::end(newBars));
    EXPECT_TRUE(eventHub->IsEnabled());
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));

    eventHub->enabled_ = false;
    EXPECT_FALSE(eventHub->IsEnabled());
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));
}

/**
 * @tc.name: ParseCommonTitle001
 * @tc.desc: Test ParseCommonTitle and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, ParseCommonTitle001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    ASSERT_NE(titleBarNode, nullptr);

    bool hasSubTitle = true, hasMainTitle = true, ignoreMainTitle = false;
    navBarNode->propPrevTitleIsCustom_ = false;
    EXPECT_TRUE(hasSubTitle && hasMainTitle);
    EXPECT_FALSE(navBarNode->GetPrevTitleIsCustomValue(false));
    EXPECT_FALSE(ignoreMainTitle);
    EXPECT_EQ(AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle()), nullptr);
    EXPECT_EQ(AceType::DynamicCast<FrameNode>(titleBarNode->GetSubtitle()), nullptr);
    navigationModel.ParseCommonTitle(hasSubTitle, hasMainTitle, "", "", ignoreMainTitle);

    // Make mainTitle true
    titleBarNode->title_ = FrameNode::CreateFrameNode("title", 101, AceType::MakeRefPtr<TextPattern>());
    // Make subTitle true
    titleBarNode->subtitle_ = FrameNode::CreateFrameNode("subTitle", 102, AceType::MakeRefPtr<TextPattern>());
    EXPECT_NE(AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle()), nullptr);
    EXPECT_NE(AceType::DynamicCast<FrameNode>(titleBarNode->GetSubtitle()), nullptr);
    navigationModel.ParseCommonTitle(hasSubTitle, hasMainTitle, "", "", ignoreMainTitle);

    // Make !hasMainTitle true
    hasMainTitle = false;
    EXPECT_TRUE(hasSubTitle && !hasMainTitle);
    navigationModel.ParseCommonTitle(hasSubTitle, hasMainTitle, "", "", ignoreMainTitle);

    hasMainTitle = true;
    ignoreMainTitle = true;
    // Make !hasSubTitle true
    hasSubTitle = false;
    EXPECT_TRUE(!hasSubTitle && hasMainTitle);
    EXPECT_TRUE(ignoreMainTitle);
    navigationModel.ParseCommonTitle(hasSubTitle, hasMainTitle, "", "", ignoreMainTitle);
}

/**
 * @tc.name: ParseCommonTitle002
 * @tc.desc: Test ParseCommonTitle and make the logic as follows:
 *               1. GetPrevTitleIsCustomValue return false
 *               2. GetPrevTitleIsCustomValue return true
 *                  HasTitleHeight reutrn false
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, ParseCommonTitle002, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    ASSERT_NE(titleBarNode, nullptr);

    bool hasSubTitle = true, hasMainTitle = true, ignoreMainTitle = false;
    // Make GetPrevTitleIsCustomValue return false
    navBarNode->propPrevTitleIsCustom_ = false;
    EXPECT_TRUE(hasSubTitle && hasMainTitle);
    EXPECT_FALSE(navBarNode->GetPrevTitleIsCustomValue(false));
    navigationModel.ParseCommonTitle(hasSubTitle, hasMainTitle, "", "", ignoreMainTitle);

    // Make GetPrevTitleIsCustomValue return true
    navBarNode->propPrevTitleIsCustom_ = true;
    EXPECT_TRUE(navBarNode->GetPrevTitleIsCustomValue(false));
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    ASSERT_NE(titleBarLayoutProperty, nullptr);
    EXPECT_FALSE(titleBarLayoutProperty->HasTitleHeight());
    navigationModel.ParseCommonTitle(hasSubTitle, hasMainTitle, "", "", ignoreMainTitle);

    // Make GetPrevTitleIsCustomValue return true
    navBarNode->propPrevTitleIsCustom_ = true;
    EXPECT_TRUE(navBarNode->GetPrevTitleIsCustomValue(false));
    // Make HasTitleHeight return true
    titleBarLayoutProperty->propTitleHeight_ = Dimension();
    EXPECT_TRUE(titleBarLayoutProperty->HasTitleHeight());
    navigationModel.ParseCommonTitle(hasSubTitle, hasMainTitle, "", "", ignoreMainTitle);
}

/**
 * @tc.name: SetCustomTitle001
 * @tc.desc: Test SetCustomTitle and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetCustomTitle001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    ASSERT_NE(titleBarNode, nullptr);

    // Make GetPrevTitleIsCustomValue return false
    navBarNode->propPrevTitleIsCustom_ = false;
    titleBarNode->title_ = nullptr;
    auto customTitle = FrameNode::CreateFrameNode("customTitle", 101, AceType::MakeRefPtr<TextPattern>());
    EXPECT_FALSE(navBarNode->GetPrevTitleIsCustomValue(false));
    navigationModel.SetCustomTitle(customTitle);

    // Make GetPrevTitleIsCustomValue return true
    navBarNode->propPrevTitleIsCustom_ = true;
    titleBarNode->title_ = FrameNode::CreateFrameNode("title", 102, AceType::MakeRefPtr<TextPattern>());
    EXPECT_TRUE(navBarNode->GetPrevTitleIsCustomValue(false));
    navigationModel.SetCustomTitle(customTitle);

    // Make GetPrevTitleIsCustomValue return true
    navBarNode->propPrevTitleIsCustom_ = true;
    // Make GetId return GetId
    titleBarNode->title_ = customTitle;
    EXPECT_TRUE(navBarNode->GetPrevTitleIsCustomValue(false));
    navigationModel.SetCustomTitle(customTitle);
}

/**
 * @tc.name: SetTitleHeight001
 * @tc.desc: Test SetTitleHeight and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetTitleHeight001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    ASSERT_NE(titleBarNode, nullptr);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    EXPECT_NE(titleBarLayoutProperty, nullptr);

    bool isValid = false;
    EXPECT_FALSE(isValid);
    navigationModel.SetTitleHeight(Dimension(), isValid);

    // Make !isValid true
    isValid = true;
    EXPECT_TRUE(isValid);
    navigationModel.SetTitleHeight(Dimension(), isValid);
}

/**
 * @tc.name: SetTitleMode001
 * @tc.desc: Test SetTitleMode and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetTitleMode001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    ASSERT_NE(titleBarNode, nullptr);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    ASSERT_NE(titleBarLayoutProperty, nullptr);
    auto navigationEventHub = navigationGroupNode->GetEventHub<EventHub>();
    ASSERT_NE(navigationEventHub, nullptr);

    // Make !IsEnabled return false
    navigationEventHub->enabled_ = true;
    EXPECT_FALSE(titleBarLayoutProperty->GetTitleHeight().has_value());
    EXPECT_EQ(AceType::DynamicCast<FrameNode>(titleBarNode->GetBackButton()), nullptr);
    EXPECT_TRUE(navigationEventHub->IsEnabled());
    navigationModel.SetTitleMode(NavigationTitleMode::MINI);

    auto backButtonNode = AceType::DynamicCast<FrameNode>(titleBarNode->backButton_);
    ASSERT_NE(backButtonNode, nullptr);
    auto gestureEventHub = backButtonNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureEventHub->clickEventActuator_, nullptr);
    auto event = gestureEventHub->clickEventActuator_->GetClickEvent();
    ASSERT_NE(event, nullptr);
    GestureEvent gestureEvent;
    event(gestureEvent);

    // Make !IsEnabled return true
    navigationEventHub->enabled_ = false;
    titleBarNode->backButton_ = nullptr;
    EXPECT_EQ(AceType::DynamicCast<FrameNode>(titleBarNode->GetBackButton()), nullptr);
    EXPECT_FALSE(navigationEventHub->IsEnabled());
    navigationModel.SetTitleMode(NavigationTitleMode::MINI);
}

/**
 * @tc.name: SetTitleMode002
 * @tc.desc: Test SetTitleMode and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetTitleMode002, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    ASSERT_NE(titleBarNode, nullptr);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    ASSERT_NE(titleBarLayoutProperty, nullptr);

    NavigationTitleMode mode = NavigationTitleMode::MINI;
    // Make backButtonNode not nullptr
    titleBarNode->backButton_ = FrameNode::CreateFrameNode("button", 101, AceType::MakeRefPtr<ButtonPattern>());
    EXPECT_NE(AceType::DynamicCast<FrameNode>(titleBarNode->GetBackButton()), nullptr);
    navigationModel.SetTitleMode(mode);

    // Make mode not MINI
    mode = NavigationTitleMode::FREE;
    EXPECT_NE(mode, NavigationTitleMode::MINI);
    navigationModel.SetTitleMode(mode);

    // Make has_value return true
    titleBarLayoutProperty->propTitleHeight_ = Dimension();
    EXPECT_TRUE(titleBarLayoutProperty->GetTitleHeight().has_value());
    navigationModel.SetTitleMode(mode);
}

/**
 * @tc.name: SetTitleMode003
 * @tc.desc: Test SetTitleMode and cover all conditions outside the clickCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetTitleMode003, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    ASSERT_NE(titleBarNode, nullptr);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    ASSERT_NE(titleBarLayoutProperty, nullptr);
    auto navigationEventHub = navigationGroupNode->GetEventHub<EventHub>();

    NavigationTitleMode mode = NavigationTitleMode::MINI;
    navigationEventHub->enabled_ = true;
    EXPECT_FALSE(titleBarLayoutProperty->GetTitleHeight().has_value());
    EXPECT_EQ(titleBarNode->GetBackButton(), nullptr);
    EXPECT_EQ(mode, NavigationTitleMode::MINI);
    EXPECT_TRUE(navigationEventHub->IsEnabled());
    NavigationModelNG::SetTitleMode(&(*frameNode), mode);

    navigationEventHub->enabled_ = false;
    titleBarNode->backButton_ = nullptr;
    EXPECT_EQ(titleBarNode->GetBackButton(), nullptr);
    EXPECT_FALSE(navigationEventHub->IsEnabled());
    NavigationModelNG::SetTitleMode(&(*frameNode), mode);

    EXPECT_NE(titleBarNode->GetBackButton(), nullptr);
    NavigationModelNG::SetTitleMode(&(*frameNode), mode);

    mode = NavigationTitleMode::FREE;
    EXPECT_NE(mode, NavigationTitleMode::MINI);
    NavigationModelNG::SetTitleMode(&(*frameNode), mode);

    titleBarLayoutProperty->propTitleHeight_ = Dimension();
    EXPECT_TRUE(titleBarLayoutProperty->GetTitleHeight().has_value());
    NavigationModelNG::SetTitleMode(&(*frameNode), mode);
}

/**
 * @tc.name: SetTitleMode004
 * @tc.desc: Test SetTitleMode and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetTitleMode004, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    ASSERT_NE(titleBarNode, nullptr);

    NavigationTitleMode mode = NavigationTitleMode::MINI;
    EXPECT_EQ(titleBarNode->GetBackButton(), nullptr);
    NavigationModelNG::SetTitleMode(&(*frameNode), mode);

    auto backButtonNode = AceType::DynamicCast<FrameNode>(titleBarNode->backButton_);
    ASSERT_NE(backButtonNode, nullptr);
    auto gestureEventHub = backButtonNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureEventHub->clickEventActuator_, nullptr);
    auto event = gestureEventHub->clickEventActuator_->GetClickEvent();
    ASSERT_NE(event, nullptr);
    GestureEvent gestureEvent;
    event(gestureEvent);
}

/**
 * @tc.name: SetHideNavBar001
 * @tc.desc: Test SetHideNavBar and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetHideNavBar001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto pattern = navigationGroupNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto navigationLayoutProperty = navigationGroupNode->GetLayoutProperty<NavigationLayoutProperty>();
    ASSERT_NE(navigationLayoutProperty, nullptr);

    EXPECT_FALSE(navigationLayoutProperty->GetHideNavBar().has_value());
    navigationModel.SetHideNavBar(false);

    navigationLayoutProperty->propHideNavBar_ = false;
    EXPECT_TRUE(navigationLayoutProperty->GetHideNavBar().has_value());
    navigationModel.SetHideNavBar(false);
}

/**
 * @tc.name: SetHideNavBar002
 * @tc.desc: Test SetHideNavBar and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetHideNavBar002, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto pattern = navigationGroupNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto navigationLayoutProperty = navigationGroupNode->GetLayoutProperty<NavigationLayoutProperty>();
    ASSERT_NE(navigationLayoutProperty, nullptr);

    EXPECT_FALSE(navigationLayoutProperty->GetHideNavBar().has_value());
    NavigationModelNG::SetHideNavBar(&(*frameNode), false);

    navigationLayoutProperty->propHideNavBar_ = false;
    EXPECT_TRUE(navigationLayoutProperty->GetHideNavBar().has_value());
    NavigationModelNG::SetHideNavBar(&(*frameNode), false);
}

/**
 * @tc.name: SetCustomMenu001
 * @tc.desc: Test SetCustomMenu and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetCustomMenu001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);

    auto customNode = FrameNode::CreateFrameNode("menu", 101, AceType::MakeRefPtr<LinearLayoutPattern>(true));
    EXPECT_EQ(navBarNode->GetMenu(), nullptr);
    navigationModel.SetCustomMenu(customNode);

    navBarNode->menu_ = FrameNode::CreateFrameNode("menu", 102, AceType::MakeRefPtr<LinearLayoutPattern>(true));
    ASSERT_NE(navBarNode->GetMenu(), nullptr);
    EXPECT_NE(customNode->GetId(), navBarNode->GetMenu()->GetId());
    navigationModel.SetCustomMenu(customNode);

    EXPECT_EQ(customNode->GetId(), navBarNode->GetMenu()->GetId());
    navigationModel.SetCustomMenu(customNode);
}

/**
 * @tc.name: SetNavBarWidth001
 * @tc.desc: Test SetNavBarWidth and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetNavBarWidth001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navigationPattern = navigationGroupNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);

    Dimension value = 250.0_vp;
    navigationPattern->initNavBarWidthValue_ = value;
    // Make GetInitNavBarWidth return value
    EXPECT_EQ(navigationPattern->GetInitNavBarWidth(), value);
    navigationModel.SetNavBarWidth(value);

    // Make GetInitNavBarWidth return not value
    navigationPattern->initNavBarWidthValue_ = 200.0_vp;
    EXPECT_NE(navigationPattern->GetInitNavBarWidth(), value);
    navigationModel.SetNavBarWidth(value);
}

/**
 * @tc.name: SetNavigationStack001
 * @tc.desc: Test SetNavigationStack and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetNavigationStack001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto pattern = navigationGroupNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->navigationStack_ = nullptr;
    EXPECT_EQ(pattern->GetNavigationStack(), nullptr);
    navigationModel.SetNavigationStack();

    EXPECT_NE(pattern->GetNavigationStack(), nullptr);
    navigationModel.SetNavigationStack();

    pattern->navigationStack_ = nullptr;
    EXPECT_EQ(pattern->GetNavigationStack(), nullptr);
    auto navigationStack = AceType::MakeRefPtr<NavigationStack>();
    navigationModel.SetNavigationStack(navigationStack);

    EXPECT_NE(pattern->GetNavigationStack(), nullptr);
    navigationModel.SetNavigationStack(navigationStack);

    auto stackCreator = []() -> RefPtr<MockNavigationStack> {
        return AceType::MakeRefPtr<MockNavigationStack>();
    };
    auto stackUpdater = [](RefPtr<NG::NavigationStack> stack) {
        auto mockStack = AceType::DynamicCast<MockNavigationStack>(stack);
        ASSERT_NE(mockStack, nullptr);
    };
    pattern->navigationStack_ = nullptr;
    EXPECT_EQ(pattern->GetNavigationStack(), nullptr);
    navigationModel.SetNavigationStackWithCreatorAndUpdater(stackCreator, stackUpdater);

    EXPECT_NE(pattern->GetNavigationStack(), nullptr);
    navigationModel.SetNavigationStackWithCreatorAndUpdater(stackCreator, stackUpdater);
}

/**
 * @tc.name: SetSubtitle001
 * @tc.desc: Test SetSubtitle and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, SetSubtitle001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetTitle("navigationModel", false);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationGroupNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationGroupNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    ASSERT_NE(titleBarNode, nullptr);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    ASSERT_NE(titleBarLayoutProperty, nullptr);

    navBarNode->propPrevTitleIsCustom_ = false;
    EXPECT_FALSE(navBarNode->GetPrevTitleIsCustomValue(false));
    EXPECT_EQ(titleBarNode->GetSubtitle(), nullptr);
    NavigationModelNG::SetSubtitle(&(*frameNode), "");

    navBarNode->propPrevTitleIsCustom_ = true;
    EXPECT_TRUE(navBarNode->GetPrevTitleIsCustomValue(false));
    EXPECT_FALSE(titleBarLayoutProperty->HasTitleHeight());
    EXPECT_NE(titleBarNode->GetSubtitle(), nullptr);
    NavigationModelNG::SetSubtitle(&(*frameNode), "");

    navBarNode->propPrevTitleIsCustom_ = true;
    titleBarLayoutProperty->propTitleHeight_ = Dimension();
    EXPECT_TRUE(navBarNode->GetPrevTitleIsCustomValue(false));
    EXPECT_TRUE(titleBarLayoutProperty->HasTitleHeight());
    NavigationModelNG::SetSubtitle(&(*frameNode), "");
}

/**
 * @tc.name: NavigationLoadPage001
 * @tc.desc: Test navigation page load success or not.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, NavigationLoadPage001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. create navigation node and set navigation stack
     */
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationNode, nullptr);
    auto stackCreator = []() -> RefPtr<MockNavigationStack> {
        return AceType::MakeRefPtr<MockNavigationStack>();
    };
    auto stackUpdater = [&navigationModel](RefPtr<NG::NavigationStack> stack) {
        navigationModel.SetNavigationStackProvided(false);
        auto mockStack = AceType::DynamicCast<MockNavigationStack>(stack);
        ASSERT_NE(mockStack, nullptr);
    };
    navigationModel.SetNavigationStackWithCreatorAndUpdater(stackCreator, stackUpdater);
    auto pattern = AceType::DynamicCast<NavigationPattern>(navigationNode->GetPattern());
    auto navigationStack = pattern->GetNavigationStack();

    /**
     * @tc.steps:step2. set navigation route add add pageOne to navigation stack
     */
    auto route = AceType::MakeRefPtr<MockNavigationRoute>("");
    EXPECT_NE(route, nullptr);
    MockContainer::Current()->SetNavigationRoute(route);
    std::string name = "pageOne";
    navigationStack->Push(name, 0);

    /**
     * @tc.steps: step3. load pageOne
     * @tc.expected: step3. navigationRoute names size is one
     */
    EXPECT_NE(pattern, nullptr);
    pattern->UpdateNavPathList();
    EXPECT_EQ(route->GetPageNames().size(), 1);

    /**
     * @tc.steps: step4. push pageOne
     * @tc.expected: step4. navigation route name size is one
     */
    navigationStack->Push("pageOne", 1);
    pattern->UpdateNavPathList();
    EXPECT_EQ(route->GetPageNames().size(), 1);
}

/**
 * @tc.name: NavigationLoadPage002
 * @tc.desc: Test navigation page load success or not.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, NavigationLoadPage002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. create navigation node and set navigation stack
     */
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationNode, nullptr);
    auto stackCreator = []() -> RefPtr<MockNavigationStack> {
        return AceType::MakeRefPtr<MockNavigationStack>();
    };
    auto stackUpdater = [&navigationModel](RefPtr<NG::NavigationStack> stack) {
        navigationModel.SetNavigationStackProvided(false);
        auto mockStack = AceType::DynamicCast<MockNavigationStack>(stack);
        ASSERT_NE(mockStack, nullptr);
    };
    navigationModel.SetNavigationStackWithCreatorAndUpdater(stackCreator, stackUpdater);
    auto pattern = AceType::DynamicCast<NavigationPattern>(navigationNode->GetPattern());
    auto navigationStack = pattern->GetNavigationStack();

    /**
     * @tc.steps:step2. set navigation route add add error page to navigation stack
     */
    auto route = AceType::MakeRefPtr<MockNavigationRoute>("");
    MockContainer::Current()->SetNavigationRoute(route);
    navigationStack->Push("error", 0);

    /**
     * @tc.steps: step3. load pageOne
     * @tc.expected: step3. navigationRoute names size is zero
     */
    EXPECT_NE(pattern, nullptr);
    pattern->UpdateNavPathList();
    EXPECT_EQ(route->GetPageNames().size(), 0);
}

/**
 * @tc.name: NavigationLoadPage003
 * @tc.desc: Test navigation page load success or not.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, NavigationLoadPage003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. create navigation node and set navigation stack
     */
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationNode, nullptr);
    auto stackCreator = []() -> RefPtr<MockNavigationStack> {
        return AceType::MakeRefPtr<MockNavigationStack>();
    };
    auto stackUpdater = [&navigationModel](RefPtr<NG::NavigationStack> stack) {
        navigationModel.SetNavigationStackProvided(false);
        auto mockStack = AceType::DynamicCast<MockNavigationStack>(stack);
        ASSERT_NE(mockStack, nullptr);
    };
    navigationModel.SetNavigationStackWithCreatorAndUpdater(stackCreator, stackUpdater);
    auto pattern = AceType::DynamicCast<NavigationPattern>(navigationNode->GetPattern());
    auto navigationStack = pattern->GetNavigationStack();

    /**
     * @tc.steps:step2. set navigation route add add pageOne to navigation stack
     */
    auto route = AceType::MakeRefPtr<MockNavigationRoute>("");
    MockContainer::Current()->SetNavigationRoute(route);
    navigationStack->Push("error", 0);

    /**
     * @tc.steps: step3. create pageOne destination
     * @tc.expected: step3. navigation name is empty
     */
    EXPECT_NE(pattern, nullptr);
    auto destinationNode = AceType::DynamicCast<FrameNode>(pattern->GenerateUINodeByIndex(0));
    EXPECT_NE(destinationNode, nullptr);
    auto destinationPattern = AceType::DynamicCast<NavDestinationPattern>(destinationNode->GetPattern());
    EXPECT_NE(destinationPattern, nullptr);
    EXPECT_TRUE(destinationPattern->GetName().empty());
}

/**
 * @tc.name: NavigationLoadPage004
 * @tc.desc: Test navigation page load success or not.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, NavigationLoadPage004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. create navigation node and set navigation stack
     */
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationNode, nullptr);
    auto stackCreator = []() -> RefPtr<MockNavigationStack> {
        return AceType::MakeRefPtr<MockNavigationStack>();
    };
    auto stackUpdater = [&navigationModel](RefPtr<NG::NavigationStack> stack) {
        navigationModel.SetNavigationStackProvided(false);
        auto mockStack = AceType::DynamicCast<MockNavigationStack>(stack);
        ASSERT_NE(mockStack, nullptr);
    };
    navigationModel.SetNavigationStackWithCreatorAndUpdater(stackCreator, stackUpdater);
    auto pattern = AceType::DynamicCast<NavigationPattern>(navigationNode->GetPattern());
    auto navigationStack = pattern->GetNavigationStack();

    /**
     * @tc.steps:step2. set navigation route add add pageOne to navigation stack add push pageOne
     */
    auto route = AceType::MakeRefPtr<MockNavigationRoute>("");
    MockContainer::Current()->SetNavigationRoute(route);
    navigationStack->Push("pageOne", 0);

    /**
     * @tc.steps: step3. create pageOne destination
     * @tc.expected: step3. navdestination name is pageOne
     */
    auto destinationNode = AceType::DynamicCast<FrameNode>(pattern->GenerateUINodeByIndex(0));
    EXPECT_NE(destinationNode, nullptr);
    auto destinationPattern = AceType::DynamicCast<NavDestinationPattern>(destinationNode->GetPattern());
    EXPECT_NE(destinationPattern, nullptr);
    EXPECT_EQ(destinationPattern->GetName(), "pageOne");
}

/**
 * @tc.name: NavigationManager001
 * @tc.desc: Test navigation manager get navigationInfo success or not.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, NavigationManager001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. create navigation node and set navigation stack
     */
    NavigationModelNG navigationModel;
    navigationModel.Create();

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto navigationNode = AceType::DynamicCast<NavigationGroupNode>(frameNode);
    ASSERT_NE(navigationNode, nullptr);

    auto stackCreator = []() -> RefPtr<MockNavigationStack> {
        return AceType::MakeRefPtr<MockNavigationStack>();
    };
    auto stackUpdater = [&navigationModel](RefPtr<NG::NavigationStack> stack) {
        navigationModel.SetNavigationStackProvided(false);
        auto mockStack = AceType::DynamicCast<MockNavigationStack>(stack);
        ASSERT_NE(mockStack, nullptr);
    };
    navigationModel.SetNavigationStackWithCreatorAndUpdater(stackCreator, stackUpdater);
    auto pattern = AceType::DynamicCast<NavigationPattern>(navigationNode->GetPattern());
    auto navigationStack = pattern->GetNavigationStack();
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());

    /**
     * @tc.steps:step2. get navigation info from empty node, and check the return value
     */
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);

    auto navigationMgr = context->GetNavigationManager();
    ASSERT_NE(navigationMgr, nullptr);
    auto result = navigationMgr->GetNavigationInfo(nullptr);
    ASSERT_EQ(result, nullptr);

    /**
     * @tc.steps:step3. get navigation info from navbar node, and check the return value
     */
    result = navigationMgr->GetNavigationInfo(navBarNode);
    ASSERT_NE(result, nullptr);
    auto navigationId = result->navigationId;
    ASSERT_EQ(navigationId, "");
    auto stack = result->pathStack;
    ASSERT_EQ(stack, navigationStack);
}

/**
 * @tc.name: UpdateNavDestinationVisibility001
 * @tc.desc: Test UpdateNavDestinationVisibility and make the logic as follows:
 *               index is not destinationSize - 1
 *               index < lastStandardIndex_
 *               GetCustomNode is remainChild
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, UpdateNavDestinationVisibility001, TestSize.Level1)
{
    auto navigationNode = NavigationGroupNode::GetOrCreateGroupNode(
        "navigationNode", 101, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    RefPtr<NavigationStack> navigationStack = AceType::MakeRefPtr<NavigationStack>();
    navigationPattern->SetNavigationStack(std::move(navigationStack));

    auto navDestinationNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 201, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    auto remainChild = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 202, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    navigationNode->lastStandardIndex_ = 0;
    int32_t index = 1;
    size_t destinationSize = 1;
    auto navDestinationPattern = navDestinationNode->GetPattern<NavDestinationPattern>();
    navDestinationPattern->customNode_ = remainChild;

    EXPECT_NE(navDestinationNode->GetEventHub<NavDestinationEventHub>(), nullptr);
    EXPECT_FALSE(index == static_cast<int32_t>(destinationSize) - 1);
    EXPECT_FALSE(index < navigationNode->lastStandardIndex_);
    EXPECT_TRUE(navDestinationPattern->GetCustomNode() == remainChild);
    bool ret = navigationNode->UpdateNavDestinationVisibility(navDestinationNode, remainChild, index, destinationSize);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateNavDestinationVisibility002
 * @tc.desc: Test UpdateNavDestinationVisibility and make the logic as follows:
 *               index is destinationSize - 1
 *               hasChanged is true
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, UpdateNavDestinationVisibility002, TestSize.Level1)
{
    auto navigationNode = NavigationGroupNode::GetOrCreateGroupNode(
        "navigationNode", 101, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    RefPtr<NavigationStack> navigationStack = AceType::MakeRefPtr<NavigationStack>();
    navigationPattern->SetNavigationStack(std::move(navigationStack));

    auto navDestinationNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 201, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    int32_t index = 0;
    size_t destinationSize = 1;

    EXPECT_NE(navDestinationNode->GetEventHub<NavDestinationEventHub>(), nullptr);
    EXPECT_EQ(index, static_cast<int32_t>(destinationSize) - 1);
    EXPECT_TRUE(CheckNeedMeasure(navDestinationNode->GetLayoutProperty()->GetPropertyChangeFlag()));
    bool ret = navigationNode->UpdateNavDestinationVisibility(navDestinationNode, nullptr, index, destinationSize);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: UpdateNavDestinationVisibility003
 * @tc.desc: Test UpdateNavDestinationVisibility and make the logic as follows:
 *               index is destinationSize - 1
 *               hasChanged is false
 *               IsAutoHeight return false
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, UpdateNavDestinationVisibility003, TestSize.Level1)
{
    auto navigationNode = NavigationGroupNode::GetOrCreateGroupNode(
        "navigationNode", 101, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    RefPtr<NavigationStack> navigationStack = AceType::MakeRefPtr<NavigationStack>();
    navigationPattern->SetNavigationStack(std::move(navigationStack));

    auto navDestinationNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 201, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    int32_t index = 0;
    // Make index destinationSize - 1
    size_t destinationSize = 1;
    // Make hasChanged false
    navDestinationNode->GetLayoutProperty()->propertyChangeFlag_ = PROPERTY_UPDATE_NORMAL;

    EXPECT_NE(navDestinationNode->GetEventHub<NavDestinationEventHub>(), nullptr);
    EXPECT_EQ(index, static_cast<int32_t>(destinationSize) - 1);
    EXPECT_FALSE(CheckNeedMeasure(navDestinationNode->GetLayoutProperty()->GetPropertyChangeFlag()));
    auto navigationLayoutProperty = navigationNode->GetLayoutProperty<NavigationLayoutProperty>();
    EXPECT_FALSE(NavigationLayoutAlgorithm::IsAutoHeight(navigationLayoutProperty));
    bool ret = navigationNode->UpdateNavDestinationVisibility(navDestinationNode, nullptr, index, destinationSize);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateNavDestinationVisibility004
 * @tc.desc: Test UpdateNavDestinationVisibility and make the logic as follows:
 *               index is destinationSize - 1
 *               hasChanged is false
 *               IsAutoHeight return true
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, UpdateNavDestinationVisibility004, TestSize.Level1)
{
    auto navigationNode = NavigationGroupNode::GetOrCreateGroupNode(
        "navigationNode", 101, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    RefPtr<NavigationStack> navigationStack = AceType::MakeRefPtr<NavigationStack>();
    navigationPattern->SetNavigationStack(std::move(navigationStack));

    auto navDestinationNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 201, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    int32_t index = 0;
    // Make index destinationSize - 1
    size_t destinationSize = 1;
    // Make hasChanged false
    navDestinationNode->GetLayoutProperty()->propertyChangeFlag_ = PROPERTY_UPDATE_NORMAL;
    // Make IsAutoHeight return true
    auto navigationLayoutProperty = navigationNode->GetLayoutProperty<NavigationLayoutProperty>();
    navigationLayoutProperty->calcLayoutConstraint_ = std::make_unique<MeasureProperty>();
    auto& calcLayoutConstraint = navigationLayoutProperty->GetCalcLayoutConstraint();
    ASSERT_TRUE(calcLayoutConstraint);
    auto calcSize = CalcSize();
    calcSize.height_ = CalcLength("auto");
    calcLayoutConstraint->selfIdealSize = calcSize;

    EXPECT_NE(navDestinationNode->GetEventHub<NavDestinationEventHub>(), nullptr);
    EXPECT_EQ(index, static_cast<int32_t>(destinationSize) - 1);
    EXPECT_FALSE(CheckNeedMeasure(navDestinationNode->GetLayoutProperty()->GetPropertyChangeFlag()));
    EXPECT_TRUE(NavigationLayoutAlgorithm::IsAutoHeight(navigationLayoutProperty));
    bool ret = navigationNode->UpdateNavDestinationVisibility(navDestinationNode, nullptr, index, destinationSize);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: UpdateNavDestinationVisibility005
 * @tc.desc: Test UpdateNavDestinationVisibility and make the logic as follows:
 *               index is not destinationSize - 1
 *               index is less than lastStandardIndex_
 *               IsOnAnimation return true
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, UpdateNavDestinationVisibility005, TestSize.Level1)
{
    auto navigationNode = NavigationGroupNode::GetOrCreateGroupNode(
        "navigationNode", 101, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    RefPtr<NavigationStack> navigationStack = AceType::MakeRefPtr<NavigationStack>();
    navigationPattern->SetNavigationStack(std::move(navigationStack));

    auto navDestinationNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 201, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    int32_t index = 1;
    // Make index not destinationSize - 1
    size_t destinationSize = 1;
    // Make index less than lastStandardIndex_
    navigationNode->lastStandardIndex_ = 2;
    // Make IsOnAnimation return true
    navDestinationNode->isOnAnimation_ = true;

    EXPECT_NE(navDestinationNode->GetEventHub<NavDestinationEventHub>(), nullptr);
    EXPECT_NE(index, static_cast<int32_t>(destinationSize) - 1);
    EXPECT_TRUE(index < navigationNode->lastStandardIndex_);
    EXPECT_TRUE(navDestinationNode->IsOnAnimation());
    bool ret = navigationNode->UpdateNavDestinationVisibility(navDestinationNode, nullptr, index, destinationSize);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateNavDestinationVisibility006
 * @tc.desc: Test UpdateNavDestinationVisibility and make the logic as follows:
 *               index is not destinationSize - 1
 *               index is less than lastStandardIndex_
 *               IsOnAnimation return false
 *               GetIsOnShow return false
 *               GetCustomNode is remainChild
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, UpdateNavDestinationVisibility006, TestSize.Level1)
{
    auto navigationNode = NavigationGroupNode::GetOrCreateGroupNode(
        "navigationNode", 101, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    RefPtr<NavigationStack> navigationStack = AceType::MakeRefPtr<NavigationStack>();
    navigationPattern->SetNavigationStack(std::move(navigationStack));

    auto navDestinationNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 201, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    auto remainChild = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 202, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    int32_t index = 1;
    // Make index not destinationSize - 1
    size_t destinationSize = 1;
    // Make index less than lastStandardIndex_
    navigationNode->lastStandardIndex_ = 2;
    // Make IsOnAnimation return false
    navDestinationNode->isOnAnimation_ = false;
    auto navDestinationPattern = navDestinationNode->GetPattern<NavDestinationPattern>();
    // Make GetIsOnShow return false
    navDestinationPattern->isOnShow_ = false;
    // Make GetCustomNode is remainChild
    navDestinationPattern->customNode_ = remainChild;

    EXPECT_NE(navDestinationNode->GetEventHub<NavDestinationEventHub>(), nullptr);
    EXPECT_NE(index, static_cast<int32_t>(destinationSize) - 1);
    EXPECT_TRUE(index < navigationNode->lastStandardIndex_);
    EXPECT_FALSE(navDestinationNode->IsOnAnimation());
    EXPECT_FALSE(navDestinationPattern->GetIsOnShow());
    EXPECT_TRUE(navDestinationPattern->GetCustomNode() == remainChild);
    bool ret = navigationNode->UpdateNavDestinationVisibility(navDestinationNode, remainChild, index, destinationSize);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateNavDestinationVisibility007
 * @tc.desc: Test UpdateNavDestinationVisibility and make the logic as follows:
 *               index is not destinationSize - 1
 *               index is less than lastStandardIndex_
 *               IsOnAnimation return false
 *               GetIsOnShow return true
 *               GetCustomNode is not remainChild
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, UpdateNavDestinationVisibility007, TestSize.Level1)
{
    auto navigationNode = NavigationGroupNode::GetOrCreateGroupNode(
        "navigationNode", 101, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    RefPtr<NavigationStack> navigationStack = AceType::MakeRefPtr<NavigationStack>();
    navigationPattern->SetNavigationStack(std::move(navigationStack));

    auto navDestinationNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 201, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    auto remainChild = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 202, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    int32_t index = 1;
    // Make index not destinationSize - 1
    size_t destinationSize = 1;
    // Make index less than lastStandardIndex_
    navigationNode->lastStandardIndex_ = 2;
    // Make IsOnAnimation return false
    navDestinationNode->isOnAnimation_ = false;
    auto navDestinationPattern = navDestinationNode->GetPattern<NavDestinationPattern>();
    // Make GetIsOnShow return true
    navDestinationPattern->isOnShow_ = true;
    // Make GetCustomNode is not remainChild
    navDestinationPattern->customNode_ = nullptr;

    EXPECT_NE(navDestinationNode->GetEventHub<NavDestinationEventHub>(), nullptr);
    EXPECT_NE(index, static_cast<int32_t>(destinationSize) - 1);
    EXPECT_TRUE(index < navigationNode->lastStandardIndex_);
    EXPECT_FALSE(navDestinationNode->IsOnAnimation());
    EXPECT_TRUE(navDestinationPattern->GetIsOnShow());
    EXPECT_TRUE(navDestinationPattern->GetCustomNode() != remainChild);
    bool ret = navigationNode->UpdateNavDestinationVisibility(navDestinationNode, remainChild, index, destinationSize);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateNavDestinationVisibility008
 * @tc.desc: Test UpdateNavDestinationVisibility and make the logic as follows:
 *               index is not destinationSize - 1
 *               index is not less than lastStandardIndex_
 *               GetCustomNode is not remainChild
 *               IsOnAnimation return true
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, UpdateNavDestinationVisibility008, TestSize.Level1)
{
    auto navigationNode = NavigationGroupNode::GetOrCreateGroupNode(
        "navigationNode", 101, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    RefPtr<NavigationStack> navigationStack = AceType::MakeRefPtr<NavigationStack>();
    navigationPattern->SetNavigationStack(std::move(navigationStack));

    auto navDestinationNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 201, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    auto remainChild = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 202, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    int32_t index = 1;
    // Make index not destinationSize - 1
    size_t destinationSize = 1;
    // Make index not less than lastStandardIndex_
    navigationNode->lastStandardIndex_ = 1;
    auto navDestinationPattern = navDestinationNode->GetPattern<NavDestinationPattern>();
    // Make GetCustomNode is not remainChild
    navDestinationPattern->customNode_ = nullptr;
    // Make IsOnAnimation return true
    navDestinationNode->isOnAnimation_ = true;

    EXPECT_NE(navDestinationNode->GetEventHub<NavDestinationEventHub>(), nullptr);
    EXPECT_NE(index, static_cast<int32_t>(destinationSize) - 1);
    EXPECT_FALSE(index < navigationNode->lastStandardIndex_);
    EXPECT_TRUE(navDestinationPattern->GetCustomNode() != remainChild);
    EXPECT_TRUE(navDestinationNode->IsOnAnimation());
    bool ret = navigationNode->UpdateNavDestinationVisibility(navDestinationNode, remainChild, index, destinationSize);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateNavDestinationVisibility009
 * @tc.desc: Test UpdateNavDestinationVisibility and make the logic as follows:
 *               index is not destinationSize - 1
 *               index is not less than lastStandardIndex_
 *               GetCustomNode is not remainChild
 *               IsOnAnimation return false
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, UpdateNavDestinationVisibility009, TestSize.Level1)
{
    auto navigationNode = NavigationGroupNode::GetOrCreateGroupNode(
        "navigationNode", 101, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    RefPtr<NavigationStack> navigationStack = AceType::MakeRefPtr<NavigationStack>();
    navigationPattern->SetNavigationStack(std::move(navigationStack));

    auto navDestinationNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 201, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    auto remainChild = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 202, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    int32_t index = 1;
    // Make index not destinationSize - 1
    size_t destinationSize = 1;
    // Make index not less than lastStandardIndex_
    navigationNode->lastStandardIndex_ = 1;
    auto navDestinationPattern = navDestinationNode->GetPattern<NavDestinationPattern>();
    // Make GetCustomNode is not remainChild
    navDestinationPattern->customNode_ = nullptr;
    // Make IsOnAnimation return false
    navDestinationNode->isOnAnimation_ = false;

    EXPECT_NE(navDestinationNode->GetEventHub<NavDestinationEventHub>(), nullptr);
    EXPECT_NE(index, static_cast<int32_t>(destinationSize) - 1);
    EXPECT_FALSE(index < navigationNode->lastStandardIndex_);
    EXPECT_TRUE(navDestinationPattern->GetCustomNode() != remainChild);
    EXPECT_FALSE(navDestinationNode->IsOnAnimation());
    bool ret = navigationNode->UpdateNavDestinationVisibility(navDestinationNode, remainChild, index, destinationSize);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: OnDetachFromMainTree001
 * @tc.desc: Test OnDetachFromMainTree and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, OnDetachFromMainTree001, TestSize.Level1)
{
    auto navigationNode = NavigationGroupNode::GetOrCreateGroupNode(
        "navigationNode", 101, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    RefPtr<NavigationStack> navigationStack = AceType::MakeRefPtr<NavigationStack>();
    navigationPattern->SetNavigationStack(std::move(navigationStack));

    auto prePattern = navigationNode->GetPattern();
    EXPECT_NE(AceType::DynamicCast<NavigationPattern>(prePattern), nullptr);
    navigationNode->OnDetachFromMainTree(false);

    navigationNode->pattern_ = AceType::MakeRefPtr<NavDestinationPattern>();
    EXPECT_EQ(AceType::DynamicCast<NavigationPattern>(navigationNode->GetPattern()), nullptr);
    navigationNode->OnDetachFromMainTree(false);
    // Reset pattern_ or crash will happen in ~NavigationGroupNode()
    navigationNode->pattern_ = prePattern;
    ASSERT_EQ(AceType::DynamicCast<NavigationPattern>(navigationNode->GetPattern()), prePattern);
}

/**
 * @tc.name: OnAttachToMainTree001
 * @tc.desc: Test OnAttachToMainTree and cover all conditions.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationModelTestNg, OnAttachToMainTree001, TestSize.Level1)
{
    auto navigationNode = NavigationGroupNode::GetOrCreateGroupNode(
        "navigationNode", 101, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    RefPtr<NavigationStack> navigationStack = AceType::MakeRefPtr<NavigationStack>();
    navigationPattern->SetNavigationStack(std::move(navigationStack));

    auto prePattern = navigationNode->GetPattern();
    EXPECT_NE(AceType::DynamicCast<NavigationPattern>(prePattern), nullptr);
    navigationNode->OnAttachToMainTree(false);

    navigationNode->pattern_ = AceType::MakeRefPtr<NavDestinationPattern>();
    EXPECT_EQ(AceType::DynamicCast<NavigationPattern>(navigationNode->GetPattern()), nullptr);
    navigationNode->OnAttachToMainTree(false);
    // Reset pattern_ or crash will happen in ~NavigationGroupNode()
    navigationNode->pattern_ = prePattern;
    ASSERT_EQ(AceType::DynamicCast<NavigationPattern>(navigationNode->GetPattern()), prePattern);
}
} // namespace OHOS::Ace::NG

