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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MODEL_MODEL_TOUCH_HANDLER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MODEL_MODEL_TOUCH_HANDLER_H

#include <functional>
#include <unordered_map>

#include "base/memory/referenced.h"
#include "foundation/graphic/graphic_3d/3d_widget_adapter/include/data_type/scene_viewer_touch_event.h"

namespace OHOS::Ace::NG {

using ModelEventCallback = std::function<void(const OHOS::Render3D::SceneViewerTouchEvent&)>;

class ModelTouchHandler : public Referenced {
public:
    explicit ModelTouchHandler() {}
    ~ModelTouchHandler() override = default;

    bool HandleTouchEvent(const TouchEventInfo& info);

    void SetCameraEventCallback(const ModelEventCallback& eventCallback)
    {
        cameraEventCallback_ = std::move(eventCallback);
    }

    void SetClickEventCallback(const ModelEventCallback& eventCallback)
    {
        clickEventCallback_ = std::move(eventCallback);
    }

    void SetCoordinateOffset(const Offset& coordinateOffset)
    {
        coordinateOffset_ = coordinateOffset;
    }

    void HandleCameraEvents(bool handle)
    {
        isHandleCameraMove_ = handle;
    }

private:
    OHOS::Render3D::SceneViewerTouchEvent CreateSceneTouchEvent(const TouchEvent& event) const;
    OHOS::Ace::TouchEvent CreateTouchEvent(const TouchEventInfo& info);

private:
    ModelEventCallback cameraEventCallback_;
    ModelEventCallback clickEventCallback_;
    std::unordered_map<int32_t, TouchEvent> touches_;
    Offset coordinateOffset_;

    bool isClicked_ = false;
    int32_t touchCount_ = 0;
    bool isHandleCameraMove_ = true;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MODEL_MODEL_TOUCH_HANDLER_H
