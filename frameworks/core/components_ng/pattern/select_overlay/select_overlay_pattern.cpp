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

#include "core/components_ng/pattern/select_overlay/select_overlay_pattern.h"

#include <algorithm>

#include "base/geometry/dimension.h"
#include "base/geometry/dimension_rect.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/point_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/offset.h"
#include "base/utils/utils.h"
#include "core/components/menu/menu_component.h"
#include "core/components/text_overlay/text_overlay_theme.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/menu/menu_layout_property.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_node.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/property/safe_area_insets.h"
#include "core/pipeline/base/constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t HIDDEN_HANDLE_TIMER_MS = 4000; // 4000ms
} // namespace

void SelectOverlayPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
    host->GetLayoutProperty()->UpdateAlignment(Alignment::TOP_LEFT);

    UpdateHandleHotZone();
    auto gesture = host->GetOrCreateGestureEventHub();
    gesture->SetHitTestMode(info_->hitTestMode);

    clickEvent_ = MakeRefPtr<ClickEvent>([weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleOnClick(info);
    });
    if (info_->isSingleHandle) {
        gesture->AddClickEvent(clickEvent_);
    }
    auto panStart = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandlePanStart(info);
    };
    auto panUpdate = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandlePanMove(info);
    };
    auto panEnd = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandlePanEnd(info);
    };
    auto panCancel = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandlePanCancel();
    };
    panEvent_ =
        MakeRefPtr<PanEvent>(std::move(panStart), std::move(panUpdate), std::move(panEnd), std::move(panCancel));
    gesture->AddPanEvent(panEvent_, { PanDirection::ALL }, 1, DEFAULT_PAN_DISTANCE);

    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleTouchEvent(info);
        }
    };
    touchEvent_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gesture->AddTouchEvent(touchEvent_);

    if (info_->isSingleHandle && !info_->isHandleLineShow) {
        StartHiddenHandleTask();
    }
}

void SelectOverlayPattern::OnDetachFromFrameNode(FrameNode* /*frameNode*/)
{
    if (info_->onClose) {
        info_->onClose(closedByGlobalTouchEvent_);
        closedByGlobalTouchEvent_ = false;
    }
}

void SelectOverlayPattern::AddMenuResponseRegion(std::vector<DimensionRect>& responseRegion)
{
    auto layoutProps = GetLayoutProperty<LayoutProperty>();
    CHECK_NULL_VOID(layoutProps);
    float safeAreaInsetsLeft = 0.0f;
    float safeAreaInsetsTop = 0.0f;
    auto&& safeAreaInsets = layoutProps->GetSafeAreaInsets();
    if (safeAreaInsets) {
        safeAreaInsetsLeft = static_cast<float>(safeAreaInsets->left_.end);
        safeAreaInsetsTop = static_cast<float>(safeAreaInsets->top_.end);
    }
    const auto& children = GetHost()->GetChildren();
    for (const auto& it : children) {
        auto child = DynamicCast<FrameNode>(it);
        if (child == nullptr) {
            continue;
        }
        auto frameRect = child->GetGeometryNode()->GetFrameRect();
        // rect is relative to window
        auto rect = Rect(frameRect.GetX() + safeAreaInsetsLeft, frameRect.GetY() + safeAreaInsetsTop, frameRect.Width(),
            frameRect.Height());

        DimensionRect region;
        region.SetSize({ Dimension(rect.GetSize().Width()), Dimension(rect.GetSize().Height()) });
        region.SetOffset(DimensionOffset(Offset(rect.GetOffset().GetX(), rect.GetOffset().GetY())));

        responseRegion.emplace_back(region);
    }
}

void SelectOverlayPattern::UpdateHandleHotZone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto firstHandle = info_->firstHandle.paintRect;
    auto secondHandle = info_->secondHandle.paintRect;

    auto theme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_VOID(theme);
    auto hotZone = theme->GetHandleHotZoneRadius().ConvertToPx();
    firstHandleRegion_.SetSize({ hotZone * 2, hotZone * 2 });
    auto firstHandleOffsetX = (firstHandle.Left() + firstHandle.Right()) / 2;
    secondHandleRegion_.SetSize({ hotZone * 2, hotZone * 2 });
    auto secondHandleOffsetX = (secondHandle.Left() + secondHandle.Right()) / 2;
    std::vector<DimensionRect> responseRegion;
    if (info_->isSingleHandle) {
        if (!info_->firstHandle.isShow && info_->secondHandle.isShow) {
            // Use the second handle to make a single handle.
            auto secondHandleOffsetY = secondHandle.Bottom();
            secondHandleRegion_.SetOffset({ secondHandleOffsetX - hotZone, secondHandleOffsetY });
            DimensionRect secondHandleRegion;
            secondHandleRegion.SetSize({ Dimension(secondHandleRegion_.GetSize().Width()),
                Dimension(secondHandleRegion_.GetSize().Height()) });
            secondHandleRegion.SetOffset(DimensionOffset(
                Offset(secondHandleRegion_.GetOffset().GetX(), secondHandleRegion_.GetOffset().GetY())));
            responseRegion.emplace_back(secondHandleRegion);
            host->GetOrCreateGestureEventHub()->SetResponseRegion(responseRegion);
        } else {
            // Use the first handle to make a single handle.
            auto firstHandleOffsetY = firstHandle.Bottom();
            firstHandleRegion_.SetOffset({ firstHandleOffsetX - hotZone, firstHandleOffsetY });
            DimensionRect firstHandleRegion;
            firstHandleRegion.SetSize(
                { Dimension(firstHandleRegion_.GetSize().Width()), Dimension(firstHandleRegion_.GetSize().Height()) });
            firstHandleRegion.SetOffset(
                DimensionOffset(Offset(firstHandleRegion_.GetOffset().GetX(), firstHandleRegion_.GetOffset().GetY())));
            responseRegion.emplace_back(firstHandleRegion);
            host->GetOrCreateGestureEventHub()->SetResponseRegion(responseRegion);
        }
        return;
    }
    if (info_->handleReverse) {
        auto firstHandleOffsetY = firstHandle.Bottom();
        firstHandleRegion_.SetOffset({ firstHandleOffsetX - hotZone, firstHandleOffsetY });
        auto secondHandleOffsetY = secondHandle.Top();
        secondHandleRegion_.SetOffset({ secondHandleOffsetX - hotZone, secondHandleOffsetY - hotZone * 2 });
    } else {
        auto firstHandleOffsetY = firstHandle.Top();
        firstHandleRegion_.SetOffset({ firstHandleOffsetX - hotZone, firstHandleOffsetY - hotZone * 2 });
        auto secondHandleOffsetY = secondHandle.Bottom();
        secondHandleRegion_.SetOffset({ secondHandleOffsetX - hotZone, secondHandleOffsetY });
    }
    DimensionRect firstHandleRegion;
    firstHandleRegion.SetSize(
        { Dimension(firstHandleRegion_.GetSize().Width()), Dimension(firstHandleRegion_.GetSize().Height()) });
    firstHandleRegion.SetOffset(
        DimensionOffset(Offset(firstHandleRegion_.GetOffset().GetX(), firstHandleRegion_.GetOffset().GetY())));
    responseRegion.emplace_back(firstHandleRegion);
    DimensionRect secondHandleRegion;
    secondHandleRegion.SetSize(
        { Dimension(secondHandleRegion_.GetSize().Width()), Dimension(secondHandleRegion_.GetSize().Height()) });
    secondHandleRegion.SetOffset(
        DimensionOffset(Offset(secondHandleRegion_.GetOffset().GetX(), secondHandleRegion_.GetOffset().GetY())));
    responseRegion.emplace_back(secondHandleRegion);
    if (IsCustomMenu()) {
        AddMenuResponseRegion(responseRegion);
    }

    host->GetOrCreateGestureEventHub()->SetResponseRegion(responseRegion);
}

void SelectOverlayPattern::HandleOnClick(GestureEvent& /*info*/)
{
    if (!info_->isSingleHandle) {
        return;
    }
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    if (!info_->menuInfo.menuDisable) {
        if (!info_->isHandleLineShow) {
            info_->menuInfo.menuIsShow = !info_->menuInfo.menuIsShow;
            host->UpdateToolBar(false);

            StopHiddenHandleTask();
            StartHiddenHandleTask();
        } else if (!info_->menuInfo.menuIsShow) {
            info_->menuInfo.menuIsShow = true;
            host->UpdateToolBar(false);
        }
        info_->menuInfo.singleHandleMenuIsShow = info_->menuInfo.menuIsShow;
    }
}

void SelectOverlayPattern::HandleTouchEvent(const TouchEventInfo& info)
{
    const auto& changedPoint = info.GetChangedTouches().front();
    if (changedPoint.GetTouchType() == TouchType::DOWN) {
        HandleTouchDownEvent(info);
    } else if (info_->onTouchDown && changedPoint.GetTouchType() == TouchType::UP) {
        info_->onTouchUp(info);
    } else if (info_->onTouchMove && changedPoint.GetTouchType() == TouchType::MOVE) {
        info_->onTouchMove(info);
    }
    if (IsCustomMenu()) {
        MenuWrapperPattern::OnTouchEvent(info);
    }
}

void SelectOverlayPattern::HandleTouchDownEvent(const TouchEventInfo& info)
{
    if (info_->onTouchDown) {
        info_->onTouchDown(info);
    }
    auto touchOffset = info.GetChangedTouches().front().GetLocalLocation();
    PointF point = { touchOffset.GetX(), touchOffset.GetY() };
    if (firstHandleRegion_.IsInRegion(point)) {
        isFirstHandleTouchDown_ = true;
    } else if (secondHandleRegion_.IsInRegion(point)) {
        isSecondHandleTouchDown_ = true;
    }
}

void SelectOverlayPattern::HandlePanStart(GestureEvent& info)
{
    if (info.GetSourceDevice() == SourceType::MOUSE) {
        return;
    }
    if (!isFirstHandleTouchDown_ && !isSecondHandleTouchDown_) {
        LOGW("no handle is pressed");
        return;
    }
    if (IsFirstHandleMoveStart(info.GetLocalLocation())) {
        firstHandleDrag_ = true;
        secondHandleDrag_ = false;
        if (info_->onHandleMoveStart) {
            info_->onHandleMoveStart(firstHandleDrag_);
        }
    } else {
        firstHandleDrag_ = false;
        secondHandleDrag_ = true;
        if (info_->onHandleMoveStart) {
            info_->onHandleMoveStart(firstHandleDrag_);
        }
    }

    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    orignMenuIsShow_ = info_->menuInfo.menuIsShow;
    if (info_->menuInfo.menuIsShow) {
        info_->menuInfo.menuIsShow = false;
        host->UpdateToolBar(false);
    }
    if (info_->isSingleHandle && !info_->isHandleLineShow) {
        StopHiddenHandleTask();
    }
    isFirstHandleTouchDown_ = false;
    isSecondHandleTouchDown_ = false;
}

void SelectOverlayPattern::HandlePanMove(GestureEvent& info)
{
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    const auto& offset = OffsetF(info.GetDelta().GetX(), info.GetDelta().GetY());
    if (firstHandleDrag_) {
        UpdateOffsetOnMove(firstHandleRegion_, info_->firstHandle, offset, true);
    } else if (secondHandleDrag_) {
        UpdateOffsetOnMove(secondHandleRegion_, info_->secondHandle, offset, false);
    } else {
        LOGW("the move point is not in drag area");
    }
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    if (host->IsLayoutDirtyMarked()) {
        context->AddDirtyLayoutNode(host);
    }
}

void SelectOverlayPattern::UpdateOffsetOnMove(
    RectF& region, SelectHandleInfo& handleInfo, const OffsetF& offset, bool isFirst)
{
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    region += offset;
    handleInfo.paintRect += offset;
    auto paintRect = handleInfo.paintRect;
    handleInfo.paintInfo = handleInfo.paintInfo + offset;
    if (handleInfo.isPaintHandleWithPoints && handleInfo.paintInfoConverter) {
        paintRect = handleInfo.paintInfoConverter(handleInfo.paintInfo);
    }
    CheckHandleReverse();
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    if (info_->onHandleMove) {
        info_->onHandleMove(paintRect, isFirst);
    }
}

void SelectOverlayPattern::HandlePanEnd(GestureEvent& /*info*/)
{
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    if (!info_->menuInfo.menuIsShow) {
        info_->menuInfo.menuIsShow = orignMenuIsShow_;
        host->UpdateToolBar(false);
    }
    if (firstHandleDrag_) {
        if (info_->onHandleMoveDone) {
            auto paintRect = GetHandlePaintRect(info_->firstHandle);
            info_->onHandleMoveDone(paintRect, true);
        }
        firstHandleDrag_ = false;
    } else if (secondHandleDrag_) {
        if (info_->onHandleMoveDone) {
            auto paintRect = GetHandlePaintRect(info_->secondHandle);
            info_->onHandleMoveDone(paintRect, false);
        }
        secondHandleDrag_ = false;
    }
    if (info_->isSingleHandle && !info_->isHandleLineShow) {
        StartHiddenHandleTask();
    }
}

RectF SelectOverlayPattern::GetHandlePaintRect(const SelectHandleInfo& handleInfo)
{
    auto paintRect = handleInfo.paintRect;
    if (handleInfo.isPaintHandleWithPoints && handleInfo.paintInfoConverter) {
        paintRect = handleInfo.paintInfoConverter(handleInfo.paintInfo);
    }
    return paintRect;
}

void SelectOverlayPattern::HandlePanCancel()
{
    GestureEvent info;
    HandlePanEnd(info);
}

void SelectOverlayPattern::CheckHandleReverse()
{
    bool handleReverseChanged = false;
    if (IsHandlesInSameLine()) {
        if (info_->firstHandle.paintRect.Left() > info_->secondHandle.paintRect.Left()) {
            if (!info_->handleReverse) {
                info_->handleReverse = true;
                handleReverseChanged = true;
            }
        } else {
            if (info_->handleReverse) {
                info_->handleReverse = false;
                handleReverseChanged = true;
            }
        }
    } else if (GreatNotEqual(info_->firstHandle.paintRect.Top(), info_->secondHandle.paintRect.Top())) {
        if (!info_->handleReverse) {
            info_->handleReverse = true;
            handleReverseChanged = true;
        }
    } else {
        if (info_->handleReverse) {
            info_->handleReverse = false;
            handleReverseChanged = true;
        }
    }
    if (handleReverseChanged && info_->onHandleReverse) {
        info_->onHandleReverse(info_->handleReverse);
    }
}

bool SelectOverlayPattern::IsHandlesInSameLine()
{
    float lowerHandleTop = 0.0f;
    RectF heigherHandleRect;
    if (GreatNotEqual(info_->firstHandle.paintRect.Top(), info_->secondHandle.paintRect.Top())) {
        lowerHandleTop = info_->firstHandle.paintRect.Top() + 0.5f;
        heigherHandleRect = info_->secondHandle.paintRect;
    } else {
        lowerHandleTop = info_->secondHandle.paintRect.Top() + 0.5f;
        heigherHandleRect = info_->firstHandle.paintRect;
    }
    return GreatNotEqual(lowerHandleTop, heigherHandleRect.Top())
        && LessNotEqual(lowerHandleTop, heigherHandleRect.Bottom());
}

bool SelectOverlayPattern::IsFirstHandleMoveStart(const Offset& touchOffset)
{
    if (isFirstHandleTouchDown_ && isSecondHandleTouchDown_) {
        auto firstHandleCenter = Offset{ firstHandleRegion_.Center().GetX(), firstHandleRegion_.Center().GetX() };
        auto secondHandleCenter = Offset{ secondHandleRegion_.Center().GetX(), secondHandleRegion_.Center().GetX() };
        auto distanceToFirstHandle = (firstHandleCenter - touchOffset).GetDistance();
        auto distanceToSecondHandle = (secondHandleCenter - touchOffset).GetDistance();
        return GreatNotEqual(distanceToSecondHandle, distanceToFirstHandle);
    }
    return isFirstHandleTouchDown_;
}

void SelectOverlayPattern::SetHandleReverse(bool reverse)
{
    info_->handleReverse = reverse;
    UpdateHandleHotZone();
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SelectOverlayPattern::SetSelectRegionVisible(bool isSelectRegionVisible)
{
    if (info_->isSelectRegionVisible != isSelectRegionVisible) {
        info_->isSelectRegionVisible = isSelectRegionVisible;
        auto host = DynamicCast<SelectOverlayNode>(GetHost());
        CHECK_NULL_VOID(host);
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    }
}

void SelectOverlayPattern::UpdateFirstSelectHandleInfo(const SelectHandleInfo& info)
{
    if (info_->firstHandle == info) {
        return;
    }
    info_->firstHandle = info;
    CheckHandleReverse();
    UpdateHandleHotZone();
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    if (info.needLayout) {
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    } else {
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

void SelectOverlayPattern::UpdateSecondSelectHandleInfo(const SelectHandleInfo& info)
{
    if (info_->secondHandle == info) {
        return;
    }
    info_->secondHandle = info;
    CheckHandleReverse();
    UpdateHandleHotZone();
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    if (info.needLayout) {
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    } else {
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

void SelectOverlayPattern::UpdateFirstAndSecondHandleInfo(
    const SelectHandleInfo& firstInfo, const SelectHandleInfo& secondInfo)
{
    if (info_->firstHandle == firstInfo && info_->secondHandle == secondInfo) {
        return;
    }
    if (info_->firstHandle != firstInfo) {
        info_->firstHandle = firstInfo;
    }
    if (info_->secondHandle != secondInfo) {
        info_->secondHandle = secondInfo;
    }
    CheckHandleReverse();
    UpdateHandleHotZone();
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    host->UpdateToolBar(false);
}

void SelectOverlayPattern::UpdateSelectMenuInfo(const SelectMenuInfo& info)
{
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    auto itemChanged = info_->menuInfo.IsIconChanged(info);
    info_->menuInfo = info;
    host->UpdateToolBar(itemChanged);
}

void SelectOverlayPattern::UpdateShowArea(const RectF& area)
{
    if (info_->showArea != area) {
        info_->showArea = area;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
}

void SelectOverlayPattern::UpdateSelectMenuInfo(std::function<void(SelectMenuInfo& menuInfo)> updateAction)
{
    if (updateAction) {
        SelectMenuInfo shadowMenuInfo = info_->menuInfo;
        updateAction(shadowMenuInfo);
        UpdateSelectMenuInfo(shadowMenuInfo);
    }
}

void SelectOverlayPattern::ShowOrHiddenMenu(bool isHidden, bool noAnimation)
{
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    if (info_->menuInfo.menuIsShow && isHidden) {
        info_->menuInfo.menuIsShow = false;
        host->UpdateToolBar(false, noAnimation);
    } else if (!info_->menuInfo.menuIsShow && !isHidden &&
               (info_->firstHandle.isShow || info_->secondHandle.isShow || info_->isSelectRegionVisible ||
               (info_->isNewAvoid && !info_->isSingleHandle))) {
        info_->menuInfo.menuIsShow = true;
        host->UpdateToolBar(false, noAnimation);
    }
}

void SelectOverlayPattern::DisableMenu(bool isDisabled)
{
    info_->menuInfo.menuDisable = isDisabled;
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    host->UpdateToolBar(false);
}

bool SelectOverlayPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    UpdateHandleHotZone();
    if (config.skipMeasure || dirty->SkipMeasureContent()) {
        return false;
    }
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto selectOverlayLayoutAlgorithm =
        DynamicCast<SelectOverlayLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(selectOverlayLayoutAlgorithm, false);
    defaultMenuStartOffset_ = selectOverlayLayoutAlgorithm->GetDefaultMenuStartOffset();
    defaultMenuEndOffset_ = selectOverlayLayoutAlgorithm->GetDefaultMenuEndOffset();
    menuWidth_ = selectOverlayLayoutAlgorithm->GetMenuWidth();
    menuHeight_ = selectOverlayLayoutAlgorithm->GetMenuHeight();
    hasExtensionMenu_ =
        selectOverlayLayoutAlgorithm->GetHasExtensionMenu() && !selectOverlayLayoutAlgorithm->GetHideMoreOrBack();
    if (IsCustomMenu()) {
        MenuWrapperPattern::CheckAndShowAnimation();
    }
    return true;
}

bool SelectOverlayPattern::IsMenuShow()
{
    CHECK_NULL_RETURN(info_, false);
    return info_->menuInfo.menuIsShow;
}

bool SelectOverlayPattern::IsSingleHandleMenuShow()
{
    CHECK_NULL_RETURN(info_, false);
    return info_->menuInfo.singleHandleMenuIsShow;
}

bool SelectOverlayPattern::IsHandleShow()
{
    CHECK_NULL_RETURN(info_, false);
    return info_->firstHandle.isShow || info_->secondHandle.isShow;
}

bool SelectOverlayPattern::IsSingleHandle()
{
    CHECK_NULL_RETURN(info_, false);
    return info_->isSingleHandle;
}

void SelectOverlayPattern::StartHiddenHandleTask(bool isDelay)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
    CHECK_NULL_VOID(context);
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    auto weak = WeakClaim(this);
    hiddenHandleTask_.Reset([weak] {
        auto client = weak.Upgrade();
        CHECK_NULL_VOID(client);
        client->HiddenHandle();
    });
    if (isDelay) {
        taskExecutor->PostDelayedTask(hiddenHandleTask_, TaskExecutor::TaskType::UI, HIDDEN_HANDLE_TIMER_MS,
            "ArkUISelectOverlayHiddenHandle");
    } else {
        taskExecutor->PostTask(hiddenHandleTask_, TaskExecutor::TaskType::UI, "ArkUISelectOverlayHiddenHandle");
    }
}

void SelectOverlayPattern::HiddenHandle()
{
    hiddenHandleTask_.Cancel();
    isHiddenHandle_ = true;
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    host->GetOrCreateGestureEventHub()->RemoveClickEvent(clickEvent_);
    host->GetOrCreateGestureEventHub()->RemovePanEvent(panEvent_);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SelectOverlayPattern::StopHiddenHandleTask()
{
    hiddenHandleTask_.Cancel();
}

void SelectOverlayPattern::UpdateSelectArea(const RectF& selectArea)
{
    info_->selectArea = selectArea;
}

void SelectOverlayPattern::SetIsNewAvoid(bool isNewAvoid)
{
    info_->isNewAvoid = isNewAvoid;
}

void SelectOverlayPattern::SetSelectMenuHeight()
{
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    auto selectMenu = AceType::DynamicCast<FrameNode>(host->GetFirstChild());
    CHECK_NULL_VOID(selectMenu);
    auto geometryNode = selectMenu->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    selectMenuHeight_ = geometryNode->GetFrameSize().Height();
}
} // namespace OHOS::Ace::NG
