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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_DRAG_DROP_DRAG_DROP_FUNC_WRAPPER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_DRAG_DROP_DRAG_DROP_FUNC_WRAPPER_H

#include "core/pipeline_ng/pipeline_context.h"

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/gestures/gesture_info.h"

namespace OHOS::Ace::NG {
/* DragDropFuncWrapper as a utility class, all function calls must use containerId. */
class FrameNode;
class ACE_FORCE_EXPORT DragDropFuncWrapper {
public:
    static void SetDraggingPointerAndPressedState(int32_t currentPointerId, int32_t containerId);
    static void DecideWhetherToStopDragging(const PointerEvent& pointerEvent,
        const std::string& extraParams, int32_t currentPointerId, int32_t containerId);
    static void UpdateDragPreviewOptionsFromModifier(
        std::function<void(WeakPtr<FrameNode>)> applyOnNodeSync, DragPreviewOption& options);
    static void UpdatePreviewOptionDefaultAttr(DragPreviewOption& option);
    static void UpdateExtraInfo(std::unique_ptr<JsonValue>& arkExtraInfoJson, DragPreviewOption& option);
    static void PrepareRadiusParametersForDragData(std::unique_ptr<JsonValue>& arkExtraInfoJson,
        DragPreviewOption& option);
    static void PrepareShadowParametersForDragData(std::unique_ptr<JsonValue>& arkExtraInfoJson,
        DragPreviewOption& option);
    static void ParseShadowInfo(Shadow& shadow, std::unique_ptr<JsonValue>& arkExtraInfoJson);
    static std::optional<Shadow> GetDefaultShadow();
    static std::optional<BorderRadiusProperty> GetDefaultBorderRadius();
    static float RadiusToSigma(float radius);
    static std::optional<EffectOption> BrulStyleToEffection(const std::optional<BlurStyleOption>& blurStyleOp);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_DRAG_DROP_DRAG_DROP_FUNC_WRAPPER_H