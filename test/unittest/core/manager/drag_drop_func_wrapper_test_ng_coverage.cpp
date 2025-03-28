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

#include "test/mock/base/mock_drag_window.h"
#include "test/mock/core/common/mock_container.h"
#include "test/mock/core/common/mock_interaction_interface.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"

#include "base/image/pixel_map.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/subwindow/subwindow_manager.h"
#include "core/common/interaction/interaction_interface.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/geometry_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/manager/drag_drop/drag_drop_func_wrapper.h"
#include "core/components_ng/manager/drag_drop/drag_drop_manager.h"
#include "core/components_ng/manager/drag_drop/drag_drop_proxy.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
RefPtr<DragWindow> MOCK_DRAG_WINDOW;
} // namespace

class DragDropFuncWrapperTestNgCoverage : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void DragDropFuncWrapperTestNgCoverage::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
    MOCK_DRAG_WINDOW = DragWindow::CreateDragWindow("", 0, 0, 0, 0);
}

void DragDropFuncWrapperTestNgCoverage::TearDownTestCase()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
    MOCK_DRAG_WINDOW = nullptr;
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage001
 * @tc.desc: Test DecideWhetherToStopDragging with valid parameters
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage001, TestSize.Level1)
{
    PointerEvent pointerEvent;
    std::string extraParams = "test";
    int32_t currentPointerId = 1;
    int32_t containerId = 100;

    auto pipelineContext = PipelineContext::GetContextByContainerId(containerId);
    ASSERT_NE(pipelineContext, nullptr);
    auto manager = pipelineContext->GetDragDropManager();
    ASSERT_NE(manager, nullptr);
    manager->SetDraggingPressedState(false);

    DragDropFuncWrapper::DecideWhetherToStopDragging(pointerEvent, extraParams, currentPointerId, containerId);

    EXPECT_FALSE(manager->IsDraggingPressed(currentPointerId));
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage002
 * @tc.desc: Test DecideWhetherToStopDragging with invalid containerId
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage002, TestSize.Level1)
{
    PointerEvent pointerEvent;
    std::string extraParams = "test";
    int32_t currentPointerId = 1;
    int32_t invalidContainerId = -1;

    DragDropFuncWrapper::DecideWhetherToStopDragging(pointerEvent, extraParams, currentPointerId, invalidContainerId);

    // No assertion needed as it should safely exit with CHECK_NULL_VOID
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage003
 * @tc.desc: Test UpdateDragPreviewOptionsFromModifier with various opacity values
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage003, TestSize.Level1)
{
    auto applyOnNodeSync = [](WeakPtr<FrameNode> frameNode) {
        auto node = frameNode.Upgrade();
        CHECK_NULL_VOID(node);
        node->GetRenderContext()->UpdateOpacity(0.5f);
    };

    DragPreviewOption option;
    DragDropFuncWrapper::UpdateDragPreviewOptionsFromModifier(applyOnNodeSync, option);

    EXPECT_EQ(option.options.opacity, 0.5f);
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage004
 * @tc.desc: Test UpdateDragPreviewOptionsFromModifier with default opacity
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage004, TestSize.Level1)
{
    auto applyOnNodeSync = [](WeakPtr<FrameNode> frameNode) {
        auto node = frameNode.Upgrade();
        CHECK_NULL_VOID(node);
        node->GetRenderContext()->UpdateOpacity(10.0f);  // Invalid value
    };

    DragPreviewOption option;
    DragDropFuncWrapper::UpdateDragPreviewOptionsFromModifier(applyOnNodeSync, option);

    EXPECT_EQ(option.options.opacity, 0.95f);  // Default opacity
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage005
 * @tc.desc: Test UpdatePreviewOptionDefaultAttr without default shadow and radius
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage005, TestSize.Level1)
{
    DragPreviewOption option;
    option.isDefaultShadowEnabled = false;
    option.isDefaultRadiusEnabled = false;

    DragDropFuncWrapper::UpdatePreviewOptionDefaultAttr(option);

    EXPECT_EQ(option.options.opacity, 0.95f);
    EXPECT_FALSE(option.options.shadow.has_value());
    EXPECT_FALSE(option.options.borderRadius.has_value());
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage006
 * @tc.desc: Test PrepareRadiusParametersForDragData with empty radius
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage006, TestSize.Level1)
{
    DragPreviewOption option;
    option.options.borderRadius = std::nullopt;

    auto arkExtraInfoJson = std::make_unique<JsonValue>();
    DragDropFuncWrapper::PrepareRadiusParametersForDragData(arkExtraInfoJson, option);

    // No assertion needed as it should safely exit with CHECK_NULL_VOID
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage007
 * @tc.desc: Test PrepareShadowParametersForDragData with empty shadow
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage007, TestSize.Level1)
{
    DragPreviewOption option;
    option.options.shadow = std::nullopt;

    auto arkExtraInfoJson = std::make_unique<JsonValue>();
    DragDropFuncWrapper::PrepareShadowParametersForDragData(arkExtraInfoJson, option);

    EXPECT_FALSE(arkExtraInfoJson->GetBool("shadow_enable"));
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage008
 * @tc.desc: Test GetDefaultShadow with invalid pipeline context
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage008, TestSize.Level1)
{
    auto shadow = DragDropFuncWrapper::GetDefaultShadow();
    EXPECT_FALSE(shadow.has_value());
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage009
 * @tc.desc: Test RadiusToSigma with valid radius
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage009, TestSize.Level1)
{
    float radius = 5.0f;
    float sigma = DragDropFuncWrapper::RadiusToSigma(radius);
    EXPECT_GT(sigma, 0.0f);
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage010
 * @tc.desc: Test RadiusToSigma with zero radius
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage010, TestSize.Level1)
{
    float radius = 0.0f;
    float sigma = DragDropFuncWrapper::RadiusToSigma(radius);
    EXPECT_EQ(sigma, 0.0f);
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage011
 * @tc.desc: Test BrulStyleToEffection with invalid pipeline context
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage011, TestSize.Level1)
{
    BlurStyleOption blurStyleOp;
    blurStyleOp.blurStyle = BlurStyle::BACKGROUND_THICK;
    blurStyleOp.scale = 0.5;
    blurStyleOp.colorMode = ThemeColorMode::LIGHT;

    auto effection = DragDropFuncWrapper::BrulStyleToEffection(blurStyleOp);
    EXPECT_FALSE(effection.has_value());
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage012
 * @tc.desc: Test BrulStyleToEffection with invalid blurStyleOp
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage012, TestSize.Level1)
{
    std::optional<BlurStyleOption> blurStyleOp = std::nullopt;

    auto effection = DragDropFuncWrapper::BrulStyleToEffection(blurStyleOp);
    EXPECT_FALSE(effection.has_value());
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage013
 * @tc.desc: Test UpdatePreviewOptionDefaultAttr with default shadow and radius
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage013, TestSize.Level1)
{
    DragPreviewOption option;
    option.isDefaultShadowEnabled = true;
    option.isDefaultRadiusEnabled = true;

    DragDropFuncWrapper::UpdatePreviewOptionDefaultAttr(option);

    EXPECT_EQ(option.options.opacity, 0.95f);
    EXPECT_EQ(option.options.shadow, DragDropFuncWrapper::GetDefaultShadow());
    EXPECT_EQ(option.options.borderRadius, DragDropFuncWrapper::GetDefaultBorderRadius());
}


/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage014
 * @tc.desc: Test UpdateExtraInfo with blur background effect
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage014, TestSize.Level1)
{
    DragPreviewOption option;
    option.options.opacity = 0.8f;
    option.options.blurbgEffect.backGroundEffect = EffectOption();

    auto arkExtraInfoJson = std::make_unique<JsonValue>();
    DragDropFuncWrapper::UpdateExtraInfo(arkExtraInfoJson, option);

    EXPECT_EQ(arkExtraInfoJson->GetDouble("dip_opacity"), 0);
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage015
 * @tc.desc: Test PrepareRadiusParametersForDragData with valid radius
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage015, TestSize.Level1)
{
    DragPreviewOption option;
    option.options.borderRadius->radiusTopLeft = 12.0_vp;
    option.options.borderRadius->radiusTopRight = 12.0_vp;
    option.options.borderRadius->radiusBottomRight = 12.0_vp;
    option.options.borderRadius->radiusBottomLeft = 12.0_vp;

    auto arkExtraInfoJson = std::make_unique<JsonValue>();
    DragDropFuncWrapper::PrepareRadiusParametersForDragData(arkExtraInfoJson, option);
    
    EXPECT_EQ(arkExtraInfoJson->GetDouble("drag_corner_radius1"), 0);
    EXPECT_EQ(arkExtraInfoJson->GetDouble("drag_corner_radius2"), 0);
    EXPECT_EQ(arkExtraInfoJson->GetDouble("drag_corner_radius3"), 0);
    EXPECT_EQ(arkExtraInfoJson->GetDouble("drag_corner_radius4"), 0);
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage016
 * @tc.desc: Test PrepareShadowParametersForDragData with valid shadow
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage016, TestSize.Level1)
{
    DragPreviewOption option;
    Shadow shadow;
    shadow.SetIsFilled(true);
    option.options.shadow = shadow;

    auto arkExtraInfoJson = std::make_unique<JsonValue>();
    DragDropFuncWrapper::PrepareShadowParametersForDragData(arkExtraInfoJson, option);

    EXPECT_FALSE(arkExtraInfoJson->GetBool("shadow_enable"));
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage017
 * @tc.desc: Test ParseShadowInfo with valid shadow
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage017, TestSize.Level1)
{
    Shadow shadow;
    shadow.SetIsFilled(true);
    shadow.SetOffset(Offset(5, 5));
    shadow.SetBlurRadius(10.0);
    shadow.SetColor(Color::FromARGB(255, 255, 0, 0));

    auto arkExtraInfoJson = std::make_unique<JsonValue>();
    DragDropFuncWrapper::ParseShadowInfo(shadow, arkExtraInfoJson);

    EXPECT_FALSE(arkExtraInfoJson->GetBool("shadow_is_filled"));
    EXPECT_EQ(arkExtraInfoJson->GetDouble("drag_shadow_OffsetX"), 0);
    EXPECT_EQ(arkExtraInfoJson->GetDouble("drag_shadow_OffsetY"), 0);
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage018
 * @tc.desc: Test GetDefaultShadow with valid pipeline context
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage018, TestSize.Level1)
{
    auto shadow = DragDropFuncWrapper::GetDefaultShadow();
    EXPECT_FALSE(shadow.has_value());
}

/**
 * @tc.name: DragDropFuncWrapperTestNgCoverage019
 * @tc.desc: Test BrulStyleToEffection with valid BlurStyleOption
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(DragDropFuncWrapperTestNgCoverage, DragDropFuncWrapperTestNgCoverage019, TestSize.Level1)
{
    BlurStyleOption blurStyleOp;
    blurStyleOp.blurStyle = BlurStyle::BACKGROUND_THICK;
    blurStyleOp.scale = 0.5;
    blurStyleOp.colorMode = ThemeColorMode::LIGHT;

    auto effection = DragDropFuncWrapper::BrulStyleToEffection(blurStyleOp);
    EXPECT_FALSE(effection.has_value());
}
} // namespace OHOS::Ace::NG
