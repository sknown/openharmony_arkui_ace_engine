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

#include "core/components_ng/event/drag_event.h"

#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/common/interaction/interaction_data.h"
#include "core/common/interaction/interaction_interface.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/event/gesture_info.h"
#include "core/components_ng/gestures/recognizers/long_press_recognizer.h"
#include "core/components_ng/gestures/recognizers/pan_recognizer.h"
#include "core/components_ng/gestures/recognizers/sequenced_recognizer.h"
#include "core/pipeline_ng/pipeline_context.h"

#include "base/subwindow/subwindow_manager.h"
#include "core/animation/animation_pub.h"
#include "core/components/container_modal/container_modal_constants.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/text/text_base.h"
#include "core/components_ng/pattern/text_drag/text_drag_base.h"
#include "core/components_ng/pattern/text_drag/text_drag_pattern.h"
#include "core/components_ng/render/adapter/component_snapshot.h"
#include "core/components_ng/render/render_context.h"
#include "core/components_v2/inspector/inspector_constants.h"

#ifdef WEB_SUPPORTED
#include "core/components_ng/pattern/web/web_pattern.h"
#endif // WEB_SUPPORTED

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t PAN_FINGER = 1;
constexpr double PAN_DISTANCE = 5.0;
constexpr int32_t LONG_PRESS_DURATION = 500;
constexpr int32_t PREVIEW_LONG_PRESS_RECONGNIZER = 800;
constexpr Dimension FILTER_VALUE(0.0f);
constexpr float PIXELMAP_DRAG_SCALE_MULTIPLE = 1.05f;
constexpr int32_t PIXELMAP_ANIMATION_TIME = 800;
constexpr float SCALE_NUMBER = 0.95f;
constexpr int32_t FILTER_TIMES = 250;
constexpr float PIXELMAP_ANIMATION_SCALE = 1.1f;
constexpr int32_t PIXELMAP_ANIMATION_DURATION = 300;
constexpr float SPRING_RESPONSE = 0.416f;
constexpr float SPRING_DAMPING_FRACTION = 0.73f;
constexpr Dimension PIXELMAP_BORDER_RADIUS = 16.0_vp;
#if defined(PIXEL_MAP_SUPPORTED)
constexpr int32_t CREATE_PIXELMAP_TIME = 80;
#endif
} // namespace

DragEventActuator::DragEventActuator(
    const WeakPtr<GestureEventHub>& gestureEventHub, PanDirection direction, int32_t fingers, float distance)
    : gestureEventHub_(gestureEventHub), direction_(direction), fingers_(fingers), distance_(distance)
{
    if (fingers_ < PAN_FINGER) {
        fingers_ = PAN_FINGER;
    }

    if (LessOrEqual(distance_, PAN_DISTANCE)) {
        distance_ = PAN_DISTANCE;
    }

    panRecognizer_ = MakeRefPtr<PanRecognizer>(fingers_, direction_, distance_);
    panRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    longPressRecognizer_ = AceType::MakeRefPtr<LongPressRecognizer>(LONG_PRESS_DURATION, fingers_, false, true);
    longPressRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    previewLongPressRecognizer_ =
        AceType::MakeRefPtr<LongPressRecognizer>(PREVIEW_LONG_PRESS_RECONGNIZER, fingers_, false, true);
    previewLongPressRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    isNotInPreviewState_ = false;
}

void DragEventActuator::StartDragTaskForWeb(const GestureEvent& info)
{
    auto gestureInfo = const_cast<GestureEvent&>(info);
    if (actionStart_) {
        actionStart_(gestureInfo);
    }
}

void DragEventActuator::StartLongPressActionForWeb()
{
    if (!isReceivedLongPress_) {
        TAG_LOGW(AceLogTag::ACE_DRAG, "not received long press action, don't start long press action for web");
        return;
    }
    if (longPressUpdate_) {
        longPressUpdate_(longPressInfo_);
    }
    isReceivedLongPress_ = false;
}

void DragEventActuator::CancelDragForWeb()
{
    if (actionCancel_) {
        actionCancel_();
    }
}

void DragEventActuator::OnCollectTouchTarget(const OffsetF& coordinateOffset, const TouchRestrict& touchRestrict,
    const GetEventTargetImpl& getEventTargetImpl, TouchTestResult& result)
{
    CHECK_NULL_VOID(userCallback_);
    isDragUserReject_ = false;
    auto gestureHub = gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureHub);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto dragDropManager = pipeline->GetDragDropManager();
    CHECK_NULL_VOID(dragDropManager);
    if (dragDropManager->IsDragging() || dragDropManager->IsMsdpDragging() ||
        (!frameNode->IsDraggable() && frameNode->IsCustomerSet())) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "No need to collect drag gestures result, dragging is %{public}d,"
            "MSDP dragging is %{public}d, frameNode draggable is %{public}d, custom set is %{public}d",
            dragDropManager->IsDragging(), dragDropManager->IsMsdpDragging(),
            frameNode->IsDraggable(), frameNode->IsCustomerSet());
        return;
    }
    auto actionStart = [weak = WeakClaim(this), this](GestureEvent& info) {
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto dragDropManager = pipeline->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        dragDropManager->ResetDragging(DragDropMgrState::ABOUT_TO_PREVIEW);
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        auto frameNode = gestureHub->GetFrameNode();
        CHECK_NULL_VOID(frameNode);
        auto renderContext = frameNode->GetRenderContext();
        if (info.GetSourceDevice() != SourceType::MOUSE) {
            if (gestureHub->GetTextDraggable()) {
                auto pattern = frameNode->GetPattern<TextBase>();
                CHECK_NULL_VOID(pattern);
                if (!pattern->IsSelected()) {
                    dragDropManager->ResetDragging();
                    gestureHub->SetIsTextDraggable(false);
                    return;
                }
                if (gestureHub->GetIsTextDraggable()) {
                    SetTextPixelMap(gestureHub);
                } else {
                    gestureHub->SetPixelMap(nullptr);
                }
            } else if (!isNotInPreviewState_) {
                if (gestureHub->GetTextDraggable()) {
                    HideTextAnimation(true, info.GetGlobalLocation().GetX(), info.GetGlobalLocation().GetY());
                } else {
                    HideEventColumn();
                    HidePixelMap(true, info.GetGlobalLocation().GetX(), info.GetGlobalLocation().GetY());
                    HideFilter();
                    SubwindowManager::GetInstance()->HideMenuNG(false, true);
                }
            }
        }

        if (info.GetSourceDevice() == SourceType::MOUSE) {
            frameNode->MarkModifyDone();
            auto pattern = frameNode->GetPattern<TextBase>();
            if (gestureHub->GetTextDraggable() && pattern) {
                if (!pattern->IsSelected() || pattern->GetMouseStatus() == MouseStatus::MOVE) {
                    dragDropManager->ResetDragging();
                    gestureHub->SetIsTextDraggable(false);
                    return;
                }
                if (pattern->BetweenSelectedPosition(info.GetGlobalLocation())) {
                    gestureHub->SetIsTextDraggable(true);
                    if (textDragCallback_) {
                        textDragCallback_(info.GetGlobalLocation());
                    }
                }
            }
        }

       // Trigger drag start event set by user.
        CHECK_NULL_VOID(actuator->userCallback_);
        auto userActionStart = actuator->userCallback_->GetActionStartEventFunc();
        if (userActionStart) {
            TAG_LOGI(AceLogTag::ACE_DRAG, "Trigger drag start event set by user.");
            userActionStart(info);
        }
        // Trigger custom drag start event
        CHECK_NULL_VOID(actuator->customCallback_);
        auto customActionStart = actuator->customCallback_->GetActionStartEventFunc();
        if (customActionStart) {
            customActionStart(info);
        }
    };
    actionStart_ = actionStart;
    panRecognizer_->SetOnActionStart(actionStart);

    auto actionUpdate = [weak = WeakClaim(this)](GestureEvent& info) {
        TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent panRecognizer onActionUpdate.");
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        CHECK_NULL_VOID(actuator->userCallback_);
        auto userActionUpdate = actuator->userCallback_->GetActionUpdateEventFunc();
        if (userActionUpdate) {
            userActionUpdate(info);
        }
        CHECK_NULL_VOID(actuator->customCallback_);
        auto customActionUpdate = actuator->customCallback_->GetActionUpdateEventFunc();
        if (customActionUpdate) {
            customActionUpdate(info);
        }
    };
    panRecognizer_->SetOnActionUpdate(actionUpdate);

    auto actionEnd = [weak = WeakClaim(this)](GestureEvent& info) {
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        CHECK_NULL_VOID(actuator->userCallback_);
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        if (gestureHub->GetTextDraggable()) {
            actuator->HideTextAnimation();
        }
        auto userActionEnd = actuator->userCallback_->GetActionEndEventFunc();
        if (userActionEnd) {
            TAG_LOGI(AceLogTag::ACE_DRAG, "Trigger drag end event set by user.");
            userActionEnd(info);
        }
        CHECK_NULL_VOID(actuator->customCallback_);
        auto customActionEnd = actuator->customCallback_->GetActionEndEventFunc();
        if (customActionEnd) {
            customActionEnd(info);
        }
        actuator->SetIsNotInPreviewState(false);
    };
    panRecognizer_->SetOnActionEnd(actionEnd);
    auto actionCancel = [weak = WeakClaim(this), this]() {
        TAG_LOGD(AceLogTag::ACE_DRAG, "Drag event has been canceled.");
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        if (!GetIsBindOverlayValue(actuator)) {
            if (gestureHub->GetTextDraggable()) {
                if (gestureHub->GetIsTextDraggable()) {
                    HideTextAnimation();
                }
            } else {
                auto frameNode = gestureHub->GetFrameNode();
                CHECK_NULL_VOID(frameNode);
                auto renderContext = frameNode->GetRenderContext();
                BorderRadiusProperty borderRadius;
                if (renderContext->GetBorderRadius().has_value()) {
                    borderRadius.UpdateWithCheck(renderContext->GetBorderRadius().value());
                }
                borderRadius.multiValued = false;
                AnimationOption option;
                option.SetDuration(PIXELMAP_ANIMATION_DURATION);
                option.SetCurve(Curves::FRICTION);
                AnimationUtils::Animate(
                    option,
                    [renderContext_ = renderContext, borderRadius_ = borderRadius]() {
                        renderContext_->UpdateBorderRadius(borderRadius_);
                    },
                    option.GetOnFinishEvent());
                HidePixelMap();
                HideFilter();
            }
        } else {
            if (actuator->panRecognizer_->getDeviceType() == SourceType::MOUSE) {
                if (!gestureHub->GetTextDraggable()) {
                    HideEventColumn();
                    HidePixelMap();
                    HideFilter();
                }
            }
        }
        actuator->SetIsNotInPreviewState(false);
        CHECK_NULL_VOID(actuator->userCallback_);
        auto userActionCancel = actuator->userCallback_->GetActionCancelEventFunc();
        if (userActionCancel) {
            userActionCancel();
        }
        CHECK_NULL_VOID(actuator->customCallback_);
        auto customActionCancel = actuator->customCallback_->GetActionCancelEventFunc();
        if (customActionCancel) {
            customActionCancel();
        }
    };
    panRecognizer_->SetIsForDrag(true);
    panRecognizer_->SetMouseDistance(DRAG_PAN_DISTANCE_MOUSE.ConvertToPx());
    actionCancel_ = actionCancel;
    panRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
    panRecognizer_->SetOnActionCancel(actionCancel);
    if (touchRestrict.sourceType == SourceType::MOUSE) {
        std::vector<RefPtr<NGGestureRecognizer>> recognizers { panRecognizer_ };
        SequencedRecognizer_ = AceType::MakeRefPtr<SequencedRecognizer>(recognizers);
        SequencedRecognizer_->RemainChildOnResetStatus();
        SequencedRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
        SequencedRecognizer_->SetGetEventTargetImpl(getEventTargetImpl);
        result.emplace_back(SequencedRecognizer_);
        return;
    }
    auto longPressUpdateValue = [weak = WeakClaim(this)](GestureEvent& info) {
        TAG_LOGD(AceLogTag::ACE_DRAG, "Trigger long press for 500ms.");
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        actuator->SetIsNotInPreviewState(true);
    };
    longPressRecognizer_->SetOnActionUpdate(longPressUpdateValue);
    auto longPressUpdate = [weak = WeakClaim(this)](GestureEvent& info) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "Trigger long press for 800ms.");
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);

        auto overlayManager = pipeline->GetOverlayManager();
        CHECK_NULL_VOID(overlayManager);
        if (overlayManager->GetHasPixelMap()) {
            return;
        }
        
        auto dragDropManager = pipeline->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        if (dragDropManager->IsAboutToPreview() || dragDropManager->IsDragging()) {
            return;
        }
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        if (gestureHub->GetTextDraggable()) {
            actuator->SetIsNotInPreviewState(false);
            if (gestureHub->GetIsTextDraggable()) {
                actuator->SetTextAnimation(gestureHub, info.GetGlobalLocation());
            }
            return;
        }

        bool isAllowedDrag = actuator->IsAllowedDrag();
        if (!isAllowedDrag) {
            actuator->longPressInfo_ = info;
            actuator->isReceivedLongPress_ = true;
            return;
        }

        actuator->SetFilter(actuator);
        auto manager = pipeline->GetOverlayManager();
        CHECK_NULL_VOID(manager);
        actuator->SetIsNotInPreviewState(false);
        actuator->SetPixelMap(actuator);
        auto motion = AceType::MakeRefPtr<ResponsiveSpringMotion>(SPRING_RESPONSE, SPRING_DAMPING_FRACTION, 0);
        auto column = manager->GetPixelMapNode();
        CHECK_NULL_VOID(column);

        auto imageNode = AceType::DynamicCast<FrameNode>(column->GetFirstChild());
        CHECK_NULL_VOID(imageNode);
        auto imageContext = imageNode->GetRenderContext();
        CHECK_NULL_VOID(imageContext);
        if (gestureHub->GetPreviewMode() == MenuPreviewMode::NONE) {
            AnimationOption option;
            option.SetDuration(PIXELMAP_ANIMATION_TIME);
            option.SetCurve(motion);
            AnimationUtils::Animate(
                option,
                [imageContext]() {
                    imageContext->UpdateTransformScale({ PIXELMAP_DRAG_SCALE_MULTIPLE, PIXELMAP_DRAG_SCALE_MULTIPLE });
                },
                option.GetOnFinishEvent());
        } else {
            imageContext->UpdateOpacity(0.0);
        }
        actuator->SetEventColumn(actuator);
    };
    auto longPressCancel = [weak = WeakClaim(this)] {
        // remove drag overlay info by Cancel event.
        TAG_LOGD(AceLogTag::ACE_DRAG, "Long press event has been canceled.");
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        actuator->HideEventColumn();
        actuator->HidePixelMap(true, 0, 0, false);
        actuator->HideFilter();
        actuator->SetIsNotInPreviewState(false);
    };
    longPressUpdate_ = longPressUpdate;
    previewLongPressRecognizer_->SetOnAction(longPressUpdate);
    previewLongPressRecognizer_->SetOnActionCancel(longPressCancel);
    previewLongPressRecognizer_->SetGestureHub(gestureEventHub_);
    auto eventHub = frameNode->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    bool isAllowedDrag = gestureHub->IsAllowedDrag(eventHub);
    if (!longPressRecognizer_->HasThumbnailCallback() && isAllowedDrag) {
        auto callback = [weakPtr = gestureEventHub_](Offset point) {
            auto gestureHub = weakPtr.Upgrade();
            CHECK_NULL_VOID(gestureHub);
            auto frameNode = gestureHub->GetFrameNode();
            CHECK_NULL_VOID(frameNode);
            auto pipeline = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto dragPreviewInfo = frameNode->GetDragPreview();
            if (dragPreviewInfo.inspectorId != "") {
                auto previewPixelMap = GetPreviewPixelMap(dragPreviewInfo.inspectorId, frameNode);
                gestureHub->SetPixelMap(previewPixelMap);
                gestureHub->SetDragPreviewPixelMap(previewPixelMap);
            } else if (dragPreviewInfo.pixelMap != nullptr) {
                gestureHub->SetPixelMap(dragPreviewInfo.pixelMap);
                gestureHub->SetDragPreviewPixelMap(dragPreviewInfo.pixelMap);
            } else if (dragPreviewInfo.customNode != nullptr) {
#if defined(PIXEL_MAP_SUPPORTED)
                auto callback = [id = Container::CurrentId(), pipeline, gestureHub]
                    (std::shared_ptr<Media::PixelMap> pixelMap, int32_t arg, std::function<void()>) {
                    ContainerScope scope(id);
                    if (pixelMap != nullptr) {
                        auto customPixelMap = PixelMap::CreatePixelMap(reinterpret_cast<void*>(&pixelMap));
                        auto taskScheduler = pipeline->GetTaskExecutor();
                        CHECK_NULL_VOID(taskScheduler);
                        taskScheduler->PostTask([gestureHub, customPixelMap]() {
                            CHECK_NULL_VOID(gestureHub);
                            gestureHub->SetPixelMap(customPixelMap);
                            gestureHub->SetDragPreviewPixelMap(customPixelMap);
                            }, TaskExecutor::TaskType::UI);
                    }
                };

                OHOS::Ace::NG::ComponentSnapshot::Create(
                    dragPreviewInfo.customNode, std::move(callback), false, CREATE_PIXELMAP_TIME);
#endif
            } else {
                auto context = frameNode->GetRenderContext();
                CHECK_NULL_VOID(context);
                auto pixelMap = context->GetThumbnailPixelMap(true);
                gestureHub->SetPixelMap(pixelMap);
            }
        };

        longPressRecognizer_->SetThumbnailCallback(std::move(callback));
    }
    std::vector<RefPtr<NGGestureRecognizer>> recognizers { longPressRecognizer_, panRecognizer_ };
    SequencedRecognizer_ = AceType::MakeRefPtr<SequencedRecognizer>(recognizers);
    SequencedRecognizer_->RemainChildOnResetStatus();
    previewLongPressRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
    longPressRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
    SequencedRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
    SequencedRecognizer_->SetGetEventTargetImpl(getEventTargetImpl);
    result.emplace_back(SequencedRecognizer_);
    result.emplace_back(previewLongPressRecognizer_);
}

void DragEventActuator::SetFilter(const RefPtr<DragEventActuator>& actuator)
{
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent start setFilter.");
    auto gestureHub = actuator->gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureHub);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto parent = frameNode->GetParent();
    CHECK_NULL_VOID(parent);
    while (parent && parent->GetDepth() != 1) {
        parent = parent->GetParent();
    }
    if (!parent) {
        TAG_LOGD(AceLogTag::ACE_DRAG, "DragFrameNode is %{public}s, depth %{public}d, can not find filter root",
            frameNode->GetTag().c_str(), frameNode->GetDepth());
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    if (!manager->GetHasFilter() && !manager->GetIsOnAnimation()) {
        if (frameNode->GetTag() == V2::WEB_ETS_TAG) {
#ifdef WEB_SUPPORTED
            auto webPattern = frameNode->GetPattern<WebPattern>();
            CHECK_NULL_VOID(webPattern);
            bool isWebmageDrag = webPattern->IsImageDrag();
            CHECK_NULL_VOID(isWebmageDrag && SystemProperties::GetDeviceType() == DeviceType::PHONE);
#endif
        } else {
            bool isBindOverlayValue = frameNode->GetLayoutProperty()->GetIsBindOverlayValue(false);
            CHECK_NULL_VOID(isBindOverlayValue && SystemProperties::GetDeviceType() == DeviceType::PHONE);
        }
        // insert columnNode to rootNode
        auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(true));
        columnNode->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
        // set filter
        TAG_LOGI(AceLogTag::ACE_DRAG, "User Device use default Filter");
        auto container = Container::Current();
        if (container && container->IsScenceBoardWindow()) {
            auto windowScene = manager->FindWindowScene(frameNode);
            manager->MountFilterToWindowScene(columnNode, windowScene);
        } else {
            columnNode->MountToParent(parent);
            columnNode->OnMountToParentDone();
            manager->SetHasFilter(true);
            manager->SetFilterColumnNode(columnNode);
            parent->MarkDirtyNode(NG::PROPERTY_UPDATE_BY_CHILD_REQUEST);
        }
        AnimationOption option;
        BlurStyleOption styleOption;
        styleOption.blurStyle = static_cast<BlurStyle>(BlurStyle::BACKGROUND_THIN);
        styleOption.colorMode = static_cast<ThemeColorMode>(static_cast<int32_t>(ThemeColorMode::SYSTEM));
        option.SetDuration(FILTER_TIMES);
        option.SetCurve(Curves::SHARP);
        columnNode->GetRenderContext()->UpdateBackBlurRadius(FILTER_VALUE);
        AnimationUtils::Animate(
            option, [columnNode, styleOption]() { columnNode->GetRenderContext()->UpdateBackBlurStyle(styleOption); },
            option.GetOnFinishEvent());
    }
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent set filter success.");
}

OffsetF DragEventActuator::GetFloatImageOffset(const RefPtr<FrameNode>& frameNode, const RefPtr<PixelMap>& pixelMap)
{
    CHECK_NULL_RETURN(frameNode, OffsetF());
    auto centerPosition = frameNode->GetPaintRectCenter();
    float width = 0.0f;
    float height = 0.0f;
    if (pixelMap) {
        width = pixelMap->GetWidth();
        height = pixelMap->GetHeight();
    }
    auto offsetX = centerPosition.GetX() - width / 2.0f;
    auto offsetY = centerPosition.GetY() - height / 2.0f;
#ifdef WEB_SUPPORTED
    if (frameNode->GetTag() == V2::WEB_ETS_TAG) {
        auto webPattern = frameNode->GetPattern<WebPattern>();
        if (webPattern) {
            auto offsetToWindow = frameNode->GetPaintRectOffset();
            offsetX = offsetToWindow.GetX() + webPattern->GetDragOffset().GetX();
            offsetY = offsetToWindow.GetY() + webPattern->GetDragOffset().GetY();
        }
    }
#endif
    // Check web tag.
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, OffsetF());
    if (pipelineContext->HasFloatTitle()) {
        offsetX -= static_cast<float>((CONTAINER_BORDER_WIDTH + CONTENT_PADDING).ConvertToPx());
        offsetY -= static_cast<float>((CONTAINER_TITLE_HEIGHT + CONTAINER_BORDER_WIDTH).ConvertToPx());
    }
    return OffsetF(offsetX, offsetY);
}

void DragEventActuator::UpdatePreviewPositionAndScale(const RefPtr<FrameNode>& imageNode, const OffsetF& frameOffset)
{
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    imageContext->UpdatePosition(OffsetT<Dimension>(Dimension(frameOffset.GetX()), Dimension(frameOffset.GetY())));
    ClickEffectInfo clickEffectInfo;
    clickEffectInfo.level = ClickEffectLevel::LIGHT;
    clickEffectInfo.scaleNumber = SCALE_NUMBER;
    imageContext->UpdateClickEffectLevel(clickEffectInfo);
}

void DragEventActuator::CreatePreviewNode(const RefPtr<FrameNode>& frameNode, OHOS::Ace::RefPtr<FrameNode>& imageNode)
{
    CHECK_NULL_VOID(frameNode);
    auto pixelMap = frameNode->GetPixelMap();
    CHECK_NULL_VOID(pixelMap);
    auto frameOffset = frameNode->GetOffsetInScreen();
    imageNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() {return AceType::MakeRefPtr<ImagePattern>(); });
    CHECK_NULL_VOID(imageNode);
    imageNode->SetDragPreviewOptions(frameNode->GetDragPreviewOption());

    auto renderProps = imageNode->GetPaintProperty<ImageRenderProperty>();
    renderProps->UpdateImageInterpolation(ImageInterpolation::HIGH);
    auto props = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    props->UpdateAutoResize(false);
    props->UpdateImageSourceInfo(ImageSourceInfo(pixelMap));
    auto targetSize = CalcSize(NG::CalcLength(pixelMap->GetWidth()), NG::CalcLength(pixelMap->GetHeight()));
    props->UpdateUserDefinedIdealSize(targetSize);

    UpdatePreviewPositionAndScale(imageNode, frameOffset);
    imageNode->MarkDirtyNode(NG::PROPERTY_UPDATE_MEASURE);
    imageNode->MarkModifyDone();
    imageNode->SetLayoutDirtyMarked(true);
    imageNode->SetActive(true);
    imageNode->CreateLayoutTask();
    FlushSyncGeometryNodeTasks();
}

void DragEventActuator::SetPreviewDefaultAnimateProperty(const RefPtr<FrameNode>& imageNode)
{
    if (imageNode->IsPreviewNeedScale()) {
        auto imageContext = imageNode->GetRenderContext();
        CHECK_NULL_VOID(imageContext);
        imageContext->UpdateTransformScale({ 1.0f, 1.0f });
        imageContext->UpdateTransformTranslate({ 0.0f, 0.0f, 0.0f });
    }
}

void DragEventActuator::MountPixelMap(const RefPtr<OverlayManager>& manager, const RefPtr<GestureEventHub>& gestureHub,
    const RefPtr<FrameNode>& imageNode)
{
    CHECK_NULL_VOID(manager);
    CHECK_NULL_VOID(imageNode);
    CHECK_NULL_VOID(gestureHub);
    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    columnNode->AddChild(imageNode);
    auto hub = columnNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(hub);
    hub->SetPixelMap(gestureHub->GetPixelMap());
    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        auto frameNode = gestureHub->GetFrameNode();
        CHECK_NULL_VOID(frameNode);
        auto windowScene = manager->FindWindowScene(frameNode);
        manager->MountPixelMapToWindowScene(columnNode, windowScene);
    } else {
        manager->MountPixelMapToRootNode(columnNode);
    }
    SetPreviewDefaultAnimateProperty(imageNode);
    columnNode->MarkDirtyNode(NG::PROPERTY_UPDATE_MEASURE);
    columnNode->MarkModifyDone();
    columnNode->SetActive(true);
    columnNode->CreateLayoutTask();
    FlushSyncGeometryNodeTasks();
}

/* Retrieves a preview PixelMap for a given drag event action.
 * This function attempts to obtain a screenshot of a frameNode associated with an inspector ID.
 * If the frameNode with the given ID does not exist or hasn't been rendered,
 * it falls back to taking a screenshot of the provided frame node.
 *
 * @param inspectorId A string representing the unique identifier for the frameNode's ID.
 * @param selfFrameNode A RefPtr to the frame node associated with the drag event.
 * @return A RefPtr to a PixelMap containing the preview image, or nullptr if not found.
 */
RefPtr<PixelMap> DragEventActuator::GetPreviewPixelMap(
    const std::string& inspectorId, const RefPtr<FrameNode>& selfFrameNode)
{
    // Attempt to retrieve the PixelMap using the inspector ID.
    auto previewPixelMap = GetPreviewPixelMapByInspectorId(inspectorId);

    // If a preview PixelMap was found, return it.
    if (previewPixelMap != nullptr) {
        return previewPixelMap;
    }

    // If not found by inspector ID, attempt to get a screenshot of the frame node.
    return GetScreenShotPixelMap(selfFrameNode);
}

/* Retrieves a preview PixelMap based on an inspector ID.
 * This function attempts to find a frame node associated with the given inspector ID and then takes a screenshot of it.
 *
 * @param inspectorId The unique identifier for a frameNode.
 * @return A RefPtr to a PixelMap containing the preview image, or nullptr if not found or the ID is empty.
 */
RefPtr<PixelMap> DragEventActuator::GetPreviewPixelMapByInspectorId(const std::string& inspectorId)
{
    // Check for an empty inspector ID and return nullptr if it is empty.
    if (inspectorId == "") {
        return nullptr;
    }

    // Retrieve the frame node using the inspector's ID.
    auto dragPreviewFrameNode = Inspector::GetFrameNodeByKey(inspectorId);

    // Take a screenshot of the frame node and return it as a PixelMap.
    return GetScreenShotPixelMap(dragPreviewFrameNode);
}


/* Captures a screenshot of the specified frame node and encapsulates it in a PixelMap.
 *
 * @param frameNode A RefPtr reference to the frame node from which to capture the screenshot.
 * @return A RefPtr to a PixelMap containing the screenshot.
 */
RefPtr<PixelMap> DragEventActuator::GetScreenShotPixelMap(const RefPtr<FrameNode>& frameNode)
{
    // Ensure the frame node is not nulls before proceeding.
    CHECK_NULL_RETURN(frameNode, nullptr);

    // Obtain the rendering context from the frame node.
    auto context = frameNode->GetRenderContext();

    // If the rendering context is not available, return nullptr.
    CHECK_NULL_RETURN(context, nullptr);

    // Capture and return the thumbnail PixelMap from the rendering context.
    return context->GetThumbnailPixelMap(true);
}

void DragEventActuator::SetPixelMap(const RefPtr<DragEventActuator>& actuator)
{
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent start set pixelMap");
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    if (manager->GetHasPixelMap()) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "Dragging animation is currently executing, unable to float other pixelMap.");
        return;
    }
    auto gestureHub = actuator->gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureHub);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    RefPtr<PixelMap> pixelMap = gestureHub->GetPixelMap();
    CHECK_NULL_VOID(pixelMap);
    auto width = pixelMap->GetWidth();
    auto height = pixelMap->GetHeight();
    auto offset = GetFloatImageOffset(frameNode, pixelMap);
    // create imageNode
    auto imageNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<ImagePattern>(); });
    imageNode->SetDragPreviewOptions(frameNode->GetDragPreviewOption());
    auto renderProps = imageNode->GetPaintProperty<ImageRenderProperty>();
    renderProps->UpdateImageInterpolation(ImageInterpolation::HIGH);
    auto props = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    props->UpdateAutoResize(false);
    props->UpdateImageSourceInfo(ImageSourceInfo(pixelMap));
    auto targetSize = CalcSize(NG::CalcLength(width), NG::CalcLength(height));
    props->UpdateUserDefinedIdealSize(targetSize);
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    imageContext->UpdatePosition(OffsetT<Dimension>(Dimension(offset.GetX()), Dimension(offset.GetY())));
    ClickEffectInfo clickEffectInfo;
    clickEffectInfo.level = ClickEffectLevel::LIGHT;
    clickEffectInfo.scaleNumber = SCALE_NUMBER;
    imageContext->UpdateClickEffectLevel(clickEffectInfo);
    // create columnNode
    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    columnNode->AddChild(imageNode);
    auto hub = columnNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(hub);
    hub->SetPixelMap(gestureHub->GetPixelMap());
    // mount to rootNode
    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        auto windowScene = manager->FindWindowScene(frameNode);
        manager->MountPixelMapToWindowScene(columnNode, windowScene);
    } else {
        manager->MountPixelMapToRootNode(columnNode);
    }
    imageNode->MarkModifyDone();
    imageNode->SetLayoutDirtyMarked(true);
    imageNode->SetActive(true);
    imageNode->CreateLayoutTask();
    FlushSyncGeometryNodeTasks();
    auto focusHub = frameNode->GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    bool hasContextMenu = focusHub->FindContextMenuOnKeyEvent(OnKeyEventType::CONTEXT_MENU);
    ShowPixelMapAnimation(imageNode, hasContextMenu);
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent set pixelMap success.");
    SetPreviewDefaultAnimateProperty(imageNode);
}

void DragEventActuator::SetEventColumn(const RefPtr<DragEventActuator>& actuator)
{
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent start set eventColumn.");
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    if (manager->GetHasEvent()) {
        return;
    }
    auto rootNode = pipelineContext->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    auto geometryNode = rootNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto width = geometryNode->GetFrameSize().Width();
    auto height = geometryNode->GetFrameSize().Height();
    // create columnNode
    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto props = columnNode->GetLayoutProperty<LinearLayoutProperty>();
    auto targetSize = CalcSize(NG::CalcLength(width), NG::CalcLength(height));
    props->UpdateUserDefinedIdealSize(targetSize);
    BindClickEvent(columnNode);
    columnNode->MarkModifyDone();
    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        auto frameNode = gestureHub->GetFrameNode();
        CHECK_NULL_VOID(frameNode);
        auto windowScene = manager->FindWindowScene(frameNode);
        manager->MountEventToWindowScene(columnNode, windowScene);
    } else {
        manager->MountEventToRootNode(columnNode);
    }
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent set eventColumn success.");
}

void DragEventActuator::HideFilter()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    manager->RemoveFilterAnimation();
}

void DragEventActuator::HidePixelMap(bool startDrag, double x, double y, bool showAnimation)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    if (showAnimation) {
        manager->RemovePixelMapAnimation(startDrag, x, y);
    } else {
        manager->RemovePixelMap();
    }
}

void DragEventActuator::HideEventColumn()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    manager->RemoveEventColumn();
}

void DragEventActuator::BindClickEvent(const RefPtr<FrameNode>& columnNode)
{
    auto callback = [this, weak = WeakClaim(this)](GestureEvent& /* info */) {
        HideEventColumn();
        HidePixelMap();
        HideFilter();
    };
    auto columnGestureHub = columnNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(columnGestureHub);
    auto clickListener = MakeRefPtr<ClickEvent>(std::move(callback));
    columnGestureHub->AddClickEvent(clickListener);
    if (!GetIsBindOverlayValue(Claim(this))) {
        columnGestureHub->SetHitTestMode(HitTestMode::HTMBLOCK);
    }
}

void DragEventActuator::ShowPixelMapAnimation(const RefPtr<FrameNode>& imageNode, bool hasContextMenu)
{
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    // pixel map animation
    AnimationOption option;
    option.SetDuration(PIXELMAP_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    imageContext->UpdateTransformScale({ 1, 1 });
    auto shadow = imageContext->GetBackShadow();
    if (!shadow.has_value()) {
        shadow = Shadow::CreateShadow(ShadowStyle::None);
    }
    imageContext->UpdateBackShadow(shadow.value());

    AnimationUtils::Animate(
        option,
        [imageContext, shadow, hasContextMenu]() mutable {
            auto color = shadow->GetColor();
            auto newColor = Color::FromARGB(100, color.GetRed(), color.GetGreen(), color.GetBlue());
            shadow->SetColor(newColor);
            imageContext->UpdateBackShadow(shadow.value());
            imageContext->UpdateTransformScale({ PIXELMAP_ANIMATION_SCALE, PIXELMAP_ANIMATION_SCALE });
            if (hasContextMenu) {
                BorderRadiusProperty borderRadius;
                borderRadius.SetRadius(PIXELMAP_BORDER_RADIUS);
                imageContext->UpdateBorderRadius(borderRadius);
            }
        },
        option.GetOnFinishEvent());
}

void DragEventActuator::SetThumbnailCallback(std::function<void(Offset)>&& callback)
{
    textDragCallback_ = callback;
    longPressRecognizer_->SetThumbnailCallback(std::move(callback));
}

void DragEventActuator::SetTextPixelMap(const RefPtr<GestureEventHub>& gestureHub)
{
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextDragBase>();
    CHECK_NULL_VOID(pattern);

    auto dragNode = pattern->MoveDragNode();
    pattern->CloseSelectOverlay();
    CHECK_NULL_VOID(dragNode);
    auto pixelMap = dragNode->GetRenderContext()->GetThumbnailPixelMap();
    if (pixelMap) {
        gestureHub->SetPixelMap(pixelMap);
    } else {
        gestureHub->SetPixelMap(nullptr);
    }
}

void DragEventActuator::SetTextAnimation(const RefPtr<GestureEventHub>& gestureHub, const Offset& globalLocation)
{
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent start set textAnimation.");
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    manager->SetHasFilter(false);
    CHECK_NULL_VOID(gestureHub);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextDragBase>();
    auto textBase = frameNode->GetPattern<TextBase>();
    CHECK_NULL_VOID(pattern);
    CHECK_NULL_VOID(textBase);
    if (!textBase->BetweenSelectedPosition(globalLocation)) {
        TAG_LOGD(AceLogTag::ACE_DRAG, "Position is between selected position, stop set text animation.");
        return;
    }
    pattern->CloseHandleAndSelect();
    auto dragNode = pattern->MoveDragNode();
    CHECK_NULL_VOID(dragNode);
    dragNode->SetDragPreviewOptions(frameNode->GetDragPreviewOption());
    // create columnNode
    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    columnNode->AddChild(dragNode);
    // mount to rootNode
    manager->MountPixelMapToRootNode(columnNode);
    auto modifier = dragNode->GetPattern<TextDragPattern>()->GetOverlayModifier();
    modifier->StartAnimate();
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent set text animation success.");
}

void DragEventActuator::HideTextAnimation(bool startDrag, double globalX, double globalY)
{
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent start hide text animation.");
    auto gestureHub = gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureHub);
    bool isAllowedDrag = IsAllowedDrag();
    if (!gestureHub->GetTextDraggable() || !isAllowedDrag) {
        TAG_LOGD(AceLogTag::ACE_DRAG, "Text is not draggable, stop set hide text animation.");
        return;
    }
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextDragBase>();
    CHECK_NULL_VOID(pattern);
    auto removeColumnNode = [id = Container::CurrentId(), startDrag, weakPattern = WeakPtr<TextDragBase>(pattern),
                                weakEvent = gestureEventHub_] {
        ContainerScope scope(id);
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto manager = pipeline->GetOverlayManager();
        CHECK_NULL_VOID(manager);
        manager->RemovePixelMap();
        if (!startDrag) {
            auto pattern = weakPattern.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->CreateHandles();
        }
        TAG_LOGD(AceLogTag::ACE_DRAG, "In removeColumnNode callback, set DragWindowVisible true.");
        auto gestureHub = weakEvent.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        if (!gestureHub->IsPixelMapNeedScale()) {
            InteractionInterface::GetInstance()->SetDragWindowVisible(true);
        }
        gestureHub->SetPixelMap(nullptr);
    };
    AnimationOption option;
    option.SetDuration(PIXELMAP_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    option.SetOnFinishEvent(removeColumnNode);

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto manager = pipeline->GetOverlayManager();
    auto dragNode = manager->GetPixelMapNode();
    CHECK_NULL_VOID(dragNode);
    auto dragFrame = dragNode->GetGeometryNode()->GetFrameRect();
    auto frameWidth = dragFrame.Width();
    auto frameHeight = dragFrame.Height();
    auto pixelMap = gestureHub->GetPixelMap();
    float scale = 1.0f;
    if (pixelMap) {
        scale = gestureHub->GetPixelMapScale(pixelMap->GetHeight(), pixelMap->GetWidth());
    }
    auto context = dragNode->GetRenderContext();
    CHECK_NULL_VOID(context);
    context->UpdateTransformScale(VectorF(1.0f, 1.0f));
    AnimationUtils::Animate(
        option,
        [context, startDrag, globalX, globalY, frameWidth, frameHeight, scale]() {
            if (startDrag) {
                context->UpdatePosition(OffsetT<Dimension>(Dimension(globalX + frameWidth * PIXELMAP_WIDTH_RATE),
                    Dimension(globalY + frameHeight * PIXELMAP_HEIGHT_RATE)));
                context->UpdateTransformScale(VectorF(scale, scale));
                context->OnModifyDone();
            }
        },
        option.GetOnFinishEvent());
    TAG_LOGD(AceLogTag::ACE_DRAG, "DragEvent set hide text animation success.");
}
bool DragEventActuator::GetIsBindOverlayValue(const RefPtr<DragEventActuator>& actuator)
{
    auto gestureHub = actuator->gestureEventHub_.Upgrade();
    CHECK_NULL_RETURN(gestureHub, true);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_RETURN(frameNode, true);
    bool isBindOverlayValue = frameNode->GetLayoutProperty()->GetIsBindOverlayValue(false);
    return isBindOverlayValue;
}

bool DragEventActuator::IsAllowedDrag()
{
    auto gestureHub = gestureEventHub_.Upgrade();
    CHECK_NULL_RETURN(gestureHub, false);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_RETURN(frameNode, false);
    auto eventHub = frameNode->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    bool isAllowedDrag = gestureHub->IsAllowedDrag(eventHub);
    return isAllowedDrag;
}

void DragEventActuator::CopyDragEvent(const RefPtr<DragEventActuator>& dragEventActuator)
{
    userCallback_ = dragEventActuator->userCallback_;
    customCallback_ = dragEventActuator->customCallback_;
    panRecognizer_ = MakeRefPtr<PanRecognizer>(fingers_, direction_, distance_);
    panRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    longPressRecognizer_ = AceType::MakeRefPtr<LongPressRecognizer>(LONG_PRESS_DURATION, fingers_, false, false);
    longPressRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    previewLongPressRecognizer_ =
        AceType::MakeRefPtr<LongPressRecognizer>(PREVIEW_LONG_PRESS_RECONGNIZER, fingers_, false, false);
    previewLongPressRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    isNotInPreviewState_ = false;
    actionStart_ = dragEventActuator->actionStart_;
    longPressUpdate_ = dragEventActuator->longPressUpdate_;
    actionCancel_ = dragEventActuator->actionCancel_;
    textDragCallback_ = dragEventActuator->textDragCallback_;
    longPressInfo_ = dragEventActuator->longPressInfo_;
}

void DragEventActuator::FlushSyncGeometryNodeTasks()
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->FlushSyncGeometryNodeTasks();
}
} // namespace OHOS::Ace::NG
