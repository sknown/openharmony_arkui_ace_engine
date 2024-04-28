/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "core/interfaces/native/node/node_scroll_modifier.h"

#include "base/utils/utils.h"
#include "core/components/common/layout/constants.h"
#include "core/components/scroll/scroll_bar_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/scroll/scroll_model_ng.h"
#include "core/components_ng/pattern/scrollable/scrollable_properties.h"
#include "core/interfaces/native/node/node_api.h"
#include "frameworks/bridge/common/utils/utils.h"
#include "core/components/scroll/scroll_position_controller.h"
#include "core/animation/curves.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t SNAP_START_INDEX = 0;
constexpr int32_t DEFAULT_SNAP_ALIGN_VALUE = 0;
constexpr int32_t DEFAULT_SCROLLABLE_AXIS_VALUE = 0;
constexpr double FRICTION_DEFAULT = 0.6;
constexpr double DEFAULT_DIMENSION_VALUE = 0.0;
constexpr double DEFAULT_SCROLLBARWIDTH_VALUE = 4.0;
constexpr int32_t PARAM_SIZE = 4;
constexpr float DEFAULT_OFFSET_VALUE = 0.0;
constexpr int32_t ERROR_INT_CODE = -1;
constexpr int32_t SCROLL_TO_INDEX_0 = 0;
constexpr int32_t SCROLL_TO_INDEX_1 = 1;
constexpr int32_t SCROLL_TO_INDEX_2 = 2;
constexpr int32_t SCROLL_TO_INDEX_CURVE = 5;

constexpr int32_t EDGE_NONE = -1;

const std::vector<RefPtr<Curve>> CurvesVector = { Curves::LINEAR, Curves::EASE, Curves::EASE_IN,
    Curves::EASE_OUT, Curves::EASE_IN_OUT, Curves::FAST_OUT_SLOW_IN, Curves::LINEAR_OUT_SLOW_IN,
    Curves::FAST_OUT_LINEAR_IN, Curves::EXTREME_DECELERATION, Curves::SHARP, Curves::RHYTHM,
    Curves::SMOOTH, Curves::FRICTION };

bool CheckSnapPagination(const std::vector<Dimension>& snapPagination)
{
    CHECK_NULL_RETURN(!snapPagination.empty(), false);
    float preValue = (*snapPagination.begin()).Value();
    CHECK_NULL_RETURN(!Negative(preValue), false);
    auto unit = (*snapPagination.begin()).Unit();
    for (auto iter = snapPagination.begin() + 1; iter < snapPagination.end(); ++iter) {
        if (Negative((*iter).Value()) || (*iter).Unit() != unit || LessOrEqual((*iter).Value(), preValue)) {
            return false;
        }
        preValue = (*iter).Value();
    }
    return true;
}
void SetScrollNestedScroll(ArkUINodeHandle node, ArkUI_Int32 scrollForward, ArkUI_Int32 scrollBackward)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NestedScrollOptions nestedOpt = {
        .forward = NestedScrollMode::SELF_ONLY,
        .backward = NestedScrollMode::SELF_ONLY,
    };
    nestedOpt.forward = static_cast<NestedScrollMode>(scrollForward);
    nestedOpt.backward = static_cast<NestedScrollMode>(scrollBackward);
    ScrollModelNG::SetNestedScroll(frameNode, nestedOpt);
}

void ResetScrollNestedScroll(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NestedScrollOptions nestedOpt = {
        .forward = NestedScrollMode::SELF_ONLY,
        .backward = NestedScrollMode::SELF_ONLY,
    };
    ScrollModelNG::SetNestedScroll(frameNode, nestedOpt);
}

ArkUI_Bool GetScrollEnableScroll(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return ScrollModelNG::GetScrollEnabled(frameNode);
}

void SetScrollEnableScroll(ArkUINodeHandle node, ArkUI_Bool isEnabled)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ScrollModelNG::SetScrollEnabled(frameNode, isEnabled);
}

void ResetScrollEnableScroll(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ScrollModelNG::SetScrollEnabled(frameNode, true);
}

ArkUI_Float32 GetScrollFriction(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return ScrollModelNG::GetFriction(frameNode);
}

void SetScrollFriction(ArkUINodeHandle node, ArkUI_Float32 friction)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ScrollModelNG::SetFriction(frameNode, friction);
}

void ResetScrollFriction(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ScrollModelNG::SetFriction(frameNode, FRICTION_DEFAULT);
}

ArkUI_Int32 GetScrollScrollSnap(ArkUINodeHandle node, ArkUI_Int32* values)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, 0);
    ScrollSnapOptions options = ScrollModelNG::GetScrollSnap(frameNode);
    values[0] = static_cast<ArkUI_Int32>(options.snapAlign);
    values[1] = static_cast<ArkUI_Int32>(options.enableSnapToStart);
    values[2] = static_cast<ArkUI_Int32>(options.enableSnapToEnd);
    auto index = 3;
    for (auto item : options.paginationParams) {
        values[index] = item.ConvertToVp();
        index++;
    }
    return index;
}

void SetScrollScrollSnap(ArkUINodeHandle node, const ArkUI_Float32* paginationValue, ArkUI_Int32 paginationSize,
    const int32_t* paginationParam, ArkUI_Int32 paramSize)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);

    int32_t snapAlign = 0;
    auto snapToStart = false;
    auto snapToEnd = false;
    auto isArray = false;
    auto snapAlignIndex = paginationSize;
    auto snapToStartIndex = paginationSize + 1;
    auto snapToEndIndex = paginationSize + 2;
    auto isArrayIndex = paginationSize + 3;
    if (paramSize - paginationSize == PARAM_SIZE) {
        snapAlign = paginationParam[snapAlignIndex];
        snapToStart = static_cast<bool>(paginationParam[snapToStartIndex]);
        snapToEnd = static_cast<bool>(paginationParam[snapToEndIndex]);
        isArray = static_cast<bool>(paginationParam[isArrayIndex]);
    }
    std::pair<bool, bool> enableSnapToSide = { snapToStart, snapToEnd };

    std::vector<Dimension> snapPagination;
    CalcDimension intervalSize;
    if (isArray) {
        for (auto i = 0; i < paginationSize; i++) {
            auto pValue = paginationValue[i];
            auto pUnit = static_cast<DimensionUnit>(paginationParam[i]);
            CalcDimension dms = Dimension(pValue, pUnit);
            snapPagination.push_back(dms);
        }
        intervalSize = CalcDimension(DEFAULT_DIMENSION_VALUE);
    } else {
        auto pValue = paginationValue[SNAP_START_INDEX];
        auto pUnit = static_cast<DimensionUnit>(paginationParam[SNAP_START_INDEX]);
        intervalSize = Dimension(pValue, pUnit);
    }

    if (!CheckSnapPagination(snapPagination)) {
        std::vector<Dimension>().swap(snapPagination);
    }

    ScrollModelNG::SetScrollSnap(
        frameNode, static_cast<ScrollSnapAlign>(snapAlign), intervalSize, snapPagination, enableSnapToSide);
}

void ResetScrollScrollSnap(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);

    int32_t snapAlign = DEFAULT_SNAP_ALIGN_VALUE;
    CalcDimension intervalSize = CalcDimension(DEFAULT_DIMENSION_VALUE);
    std::vector<Dimension> snapPaginations;
    std::pair<bool, bool> enableSnapToSide = { true, true };
    ScrollModelNG::SetScrollSnap(
        frameNode, static_cast<ScrollSnapAlign>(snapAlign), intervalSize, snapPaginations, enableSnapToSide);
}

ArkUI_Int32 GetScrollScrollBar(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return ScrollModelNG::GetScrollBar(frameNode);
}

void SetScrollScrollBar(ArkUINodeHandle node, ArkUI_Int32 direction)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ScrollModelNG::SetScrollBar(frameNode, static_cast<DisplayMode>(direction));
}

void ResetScrollScrollBar(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ScrollModelNG::SetScrollBar(frameNode, DisplayMode::AUTO);
}

ArkUI_Int32 GetScrollScrollable(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return ScrollModelNG::GetAxis(frameNode);
}

void SetScrollScrollable(ArkUINodeHandle node, ArkUI_Int32 scrollDirection)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ScrollModelNG::SetAxis(frameNode, static_cast<Axis>(scrollDirection));
}

void ResetScrollScrollable(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ScrollModelNG::SetAxis(frameNode, static_cast<Axis>(DEFAULT_SCROLLABLE_AXIS_VALUE));
}

ArkUI_Uint32 GetScrollScrollBarColor(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return ScrollModelNG::GetScrollBarColor(frameNode);
}

void SetScrollScrollBarColor(ArkUINodeHandle node, uint32_t scrollBarColor)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ScrollModelNG::SetScrollBarColor(frameNode, Color(scrollBarColor));
}

void ResetScrollScrollBarColor(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto pipeline = PipelineBase::GetCurrentContextSafely();
    CHECK_NULL_VOID(pipeline);
    auto scrollBarTheme = pipeline->GetTheme<ScrollBarTheme>();
    CHECK_NULL_VOID(scrollBarTheme);
    Color foregroundColor = scrollBarTheme->GetForegroundColor();
    ScrollModelNG::SetScrollBarColor(frameNode, foregroundColor);
}

ArkUI_Float32 GetScrollScrollBarWidth(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return ScrollModelNG::GetScrollBarWidth(frameNode);
}

void SetScrollScrollBarWidth(ArkUINodeHandle node, ArkUI_Float32 value, ArkUI_Int32 unit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension width = Dimension(value, static_cast<OHOS::Ace::DimensionUnit>(unit));
    ScrollModelNG::SetScrollBarWidth(frameNode, width);
}

void ResetScrollScrollBarWidth(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension width = CalcDimension(DEFAULT_SCROLLBARWIDTH_VALUE, DimensionUnit::VP);
    ScrollModelNG::SetScrollBarWidth(frameNode, width);
}

ArkUI_Int32 GetScrollEdgeEffect(ArkUINodeHandle node, ArkUI_Int32* values)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    values[SCROLL_TO_INDEX_0] = static_cast<ArkUI_Int32>(ScrollModelNG::GetEdgeEffect(frameNode));
    values[SCROLL_TO_INDEX_1] = static_cast<ArkUI_Int32>(ScrollModelNG::GetEdgeEffectAlways(frameNode));
    return SCROLL_TO_INDEX_2;
}

void SetScrollEdgeEffect(ArkUINodeHandle node, ArkUI_Int32 edgeEffect, ArkUI_Bool alwaysEnabled)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ScrollModelNG::SetEdgeEffect(frameNode, static_cast<EdgeEffect>(edgeEffect), alwaysEnabled);
}

void ResetScrollEdgeEffect(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ScrollModelNG::SetEdgeEffect(frameNode, EdgeEffect::NONE, true);
}

ArkUI_Bool GetEnableScrollInteraction(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return ScrollModelNG::GetScrollEnabled(frameNode);
}

void SetEnableScrollInteraction(ArkUINodeHandle node, ArkUI_Bool enableScrollInteraction)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ScrollModelNG::SetScrollEnabled(frameNode, enableScrollInteraction);
}

void ResetEnableScrollInteraction(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ScrollModelNG::SetScrollEnabled(frameNode, true);
}

void SetScrollTo(ArkUINodeHandle node, const ArkUI_Float32* values)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    RefPtr<ScrollControllerBase> scrollControllerBase = ScrollModelNG::GetOrCreateController(frameNode);

    Dimension xOffset(values[0], static_cast<OHOS::Ace::DimensionUnit>(values[1]));
    Dimension yOffset(values[2], static_cast<OHOS::Ace::DimensionUnit>(values[3]));
    float duration = values[4];
    RefPtr<Curve> curve = Curves::EASE;
    if (static_cast<int>(values[SCROLL_TO_INDEX_CURVE]) < CurvesVector.size()) {
        curve = CurvesVector[static_cast<int>(values[SCROLL_TO_INDEX_CURVE])];
    }
    auto smooth = static_cast<bool>(values[6]);
    auto direction = scrollControllerBase->GetScrollDirection();
    auto position = direction == Axis::VERTICAL ? yOffset : xOffset;
    scrollControllerBase->AnimateTo(position, duration, curve, smooth);
}

void SetScrollEdge(ArkUINodeHandle node, ArkUI_Int32 value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    RefPtr<ScrollControllerBase> scrollControllerBase =  ScrollModelNG::GetOrCreateController(frameNode);

    scrollControllerBase->ScrollToEdge(static_cast<ScrollEdgeType>(value), true);
}

void ResetScrollTo(ArkUINodeHandle node)
{
    std::vector<float> values = { DEFAULT_OFFSET_VALUE, DEFAULT_OFFSET_VALUE };
    SetScrollTo(node, values.data());
}

void ResetScrollEdge(ArkUINodeHandle node)
{
    SetScrollEdge(node, DEFAULT_SNAP_ALIGN_VALUE);
}

ArkUI_Int32 GetScrollEnablePaging(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_INT_CODE);
    return ScrollModelNG::GetEnablePaging(frameNode);
}

void SetScrollEnablePaging(ArkUINodeHandle node, int32_t value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);

    ScrollModelNG::SetEnablePaging(frameNode, value);
}

void ResetScrollEnablePaging(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);

    ScrollModelNG::SetEnablePaging(frameNode, false);
}

void GetScrollNestedScroll(ArkUINodeHandle node, ArkUI_Int32* values)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NestedScrollOptions options = ScrollModelNG::GetNestedScroll(frameNode);
    values[0] = static_cast<ArkUI_Int32>(options.forward);
    values[1] = static_cast<ArkUI_Int32>(options.backward);
}

void GetScrollOffset(ArkUINodeHandle node, ArkUI_Float32* values)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RefPtr<ScrollControllerBase> scrollControllerBase = ScrollModelNG::GetOrCreateController(frameNode);
    Offset offset = scrollControllerBase->GetCurrentOffset();
    values[0] = offset.GetX();
    values[1] = offset.GetY();
}

ArkUI_Int32 GetScrollEdge(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, EDGE_NONE);
    ScrollEdgeType type = ScrollModelNG::GetOnScrollEdge(frameNode);
    if (type == ScrollEdgeType::SCROLL_NONE) {
        return EDGE_NONE;
    }
    return static_cast<ArkUI_Int32>(type);
}

void SetScrollPage(ArkUINodeHandle node, ArkUI_Int32 next, ArkUI_Int32 animation)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto* pattern = frameNode->GetPattern<ScrollablePattern>();
    CHECK_NULL_VOID(pattern);
    pattern->ScrollPage(next, animation);
}

void SetScrollBy(ArkUINodeHandle node, ArkUI_Float32* values)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RefPtr<ScrollControllerBase> scrollControllerBase = ScrollModelNG::GetOrCreateController(frameNode);
    CHECK_NULL_VOID(scrollControllerBase);
    scrollControllerBase->ScrollBy(values[0], values[1], false);
}

} // namespace

namespace NodeModifier {
const ArkUIScrollModifier* GetScrollModifier()
{
    /* clang-format off */
    static const ArkUIScrollModifier modifier = {
        SetScrollNestedScroll,
        ResetScrollNestedScroll,
        GetScrollEnableScroll,
        SetScrollEnableScroll,
        ResetScrollEnableScroll,
        GetScrollFriction,
        SetScrollFriction,
        ResetScrollFriction,
        GetScrollScrollSnap,
        SetScrollScrollSnap,
        ResetScrollScrollSnap,
        GetScrollScrollBar,
        SetScrollScrollBar,
        ResetScrollScrollBar,
        GetScrollScrollable,
        SetScrollScrollable,
        ResetScrollScrollable,
        GetScrollScrollBarColor,
        SetScrollScrollBarColor,
        ResetScrollScrollBarColor,
        GetScrollScrollBarWidth,
        SetScrollScrollBarWidth,
        ResetScrollScrollBarWidth,
        GetScrollEdgeEffect,
        SetScrollEdgeEffect,
        ResetScrollEdgeEffect,
        GetEnableScrollInteraction,
        SetEnableScrollInteraction,
        ResetEnableScrollInteraction,
        SetScrollTo,
        SetScrollEdge,
        ResetScrollTo,
        ResetScrollEdge,
        GetScrollEnablePaging,
        SetScrollEnablePaging,
        ResetScrollEnablePaging,
        GetScrollNestedScroll,
        GetScrollOffset,
        GetScrollEdge,
        SetScrollPage,
        SetScrollBy,
    };
    /* clang-format on */
    return &modifier;
}

void SetOnScroll(ArkUINodeHandle node, void* extraParam)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onScroll = [node, extraParam](Dimension scrollX, Dimension scrollY) {
        ArkUINodeEvent event;
        event.kind = COMPONENT_ASYNC_EVENT;
        event.extraParam = reinterpret_cast<intptr_t>(extraParam);
        event.componentAsyncEvent.subKind = ON_SCROLL;
        event.componentAsyncEvent.data[0].f32 = static_cast<float>(scrollX.Value());
        event.componentAsyncEvent.data[1].f32 = static_cast<float>(scrollY.Value());
        SendArkUIAsyncEvent(&event);
    };
    ScrollModelNG::SetOnScroll(frameNode, std::move(onScroll));
}

void SetOnScrollFrameBegin(ArkUINodeHandle node, void* extraParam)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    int32_t nodeId = frameNode->GetId();
    auto onScrollFrameBegin = [nodeId, node, extraParam](const Dimension& offset, const ScrollState& state) ->
            ScrollFrameResult {
        ScrollFrameResult scrollRes { .offset = offset };
        ArkUINodeEvent event;
        event.kind = COMPONENT_ASYNC_EVENT;
        event.extraParam = reinterpret_cast<intptr_t>(extraParam);
        event.componentAsyncEvent.subKind = ON_SCROLL_FRAME_BEGIN;
        event.componentAsyncEvent.data[0].f32 = static_cast<float>(offset.Value());
        event.componentAsyncEvent.data[1].i32 = static_cast<int>(state);
        SendArkUIAsyncEvent(&event);
        scrollRes.offset = Dimension(event.componentAsyncEvent.data[0].f32, DimensionUnit::VP);
        return scrollRes;
    };
    ScrollModelNG::SetOnScrollFrameBegin(frameNode, std::move(onScrollFrameBegin));
}

void SetOnWillScroll(ArkUINodeHandle node, void* extraParam)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    int32_t nodeId = frameNode->GetId();
    auto onWillScroll = [nodeId, node, extraParam](const Dimension& xOffset, const Dimension& yOffset,
        const ScrollState& state) -> void {
        ArkUINodeEvent event;
        event.kind = COMPONENT_ASYNC_EVENT;
        event.extraParam = reinterpret_cast<intptr_t>(extraParam);
        event.componentAsyncEvent.subKind = ON_WILL_SCROLL;
        event.componentAsyncEvent.data[0].f32 = static_cast<float>(xOffset.Value());
        event.componentAsyncEvent.data[1].f32 = static_cast<float>(yOffset.Value());
        event.componentAsyncEvent.data[2].i32 = static_cast<int>(state);
        SendArkUIAsyncEvent(&event);
    };
    ScrollModelNG::SetOnWillScroll(frameNode, std::move(onWillScroll));
}

void SetOnDidScroll(ArkUINodeHandle node, void* extraParam)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    int32_t nodeId = frameNode->GetId();
    auto onDidScroll = [nodeId, node, extraParam](const Dimension& xOffset, const Dimension& yOffset,
        const ScrollState& state) -> void {
        ArkUINodeEvent event;
        event.kind = COMPONENT_ASYNC_EVENT;
        event.extraParam = reinterpret_cast<intptr_t>(extraParam);
        event.componentAsyncEvent.subKind = ON_DID_SCROLL;
        event.componentAsyncEvent.data[0].f32 = static_cast<float>(xOffset.Value());
        event.componentAsyncEvent.data[1].f32 = static_cast<float>(yOffset.Value());
        event.componentAsyncEvent.data[2].i32 = static_cast<int>(state);
        SendArkUIAsyncEvent(&event);
    };
    ScrollModelNG::SetOnDidScroll(std::move(onDidScroll));
}

void SetOnScrollStart(ArkUINodeHandle node, void* extraParam)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    int32_t nodeId = frameNode->GetId();
    auto onScrollStart = [nodeId, node, extraParam]() -> void {
        ArkUINodeEvent event;
        event.kind = COMPONENT_ASYNC_EVENT;
        event.extraParam = reinterpret_cast<intptr_t>(extraParam);
        event.componentAsyncEvent.subKind = ON_SCROLL_START;
        SendArkUIAsyncEvent(&event);
    };
    ScrollModelNG::SetOnScrollStart(frameNode, std::move(onScrollStart));
}

void SetOnScrollStop(ArkUINodeHandle node, void* extraParam)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    int32_t nodeId = frameNode->GetId();
    auto onScrollStop = [nodeId, node, extraParam]() -> void {
        ArkUINodeEvent event;
        event.kind = COMPONENT_ASYNC_EVENT;
        event.extraParam = reinterpret_cast<intptr_t>(extraParam);
        event.componentAsyncEvent.subKind = ON_SCROLL_STOP;
        SendArkUIAsyncEvent(&event);
    };
    ScrollModelNG::SetOnScrollStop(frameNode, std::move(onScrollStop));
}

void SetOnScrollEdge(ArkUINodeHandle node, void* extraParam)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onScroll = [node, extraParam](ScrollEdge edge) {
        ArkUINodeEvent event;
        event.kind = COMPONENT_ASYNC_EVENT;
        event.extraParam = reinterpret_cast<intptr_t>(extraParam);
        event.componentAsyncEvent.data[0].i32 = static_cast<int>(edge);
        event.componentAsyncEvent.subKind = ON_SCROLL_EDGE;
        SendArkUIAsyncEvent(&event);
    };
    ScrollModelNG::SetOnScrollEdge(frameNode, std::move(onScroll));
}
} // namespace NodeModifier
} // namespace OHOS::Ace::NG
