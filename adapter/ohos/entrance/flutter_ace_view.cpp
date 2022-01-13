/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "adapter/ohos/entrance/flutter_ace_view.h"

#include <algorithm>
#include <fstream>

#include "base/log/dump_log.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/utils/macros.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/ace_engine.h"
#include "core/components/theme/app_theme.h"
#include "core/components/theme/theme_manager.h"
#include "core/event/mouse_event.h"
#include "core/event/touch_event.h"
#include "core/image/image_cache.h"
#include "core/pipeline/layers/flutter_scene_builder.h"

namespace OHOS::Ace::Platform {
namespace {

constexpr int32_t ROTATION_DIVISOR = 64;
constexpr double PERMIT_ANGLE_VALUE = 0.5;

template<typename E>
void GetEventDevice(int32_t sourceType, E& event)
{
    switch (sourceType) {
        case OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN:
            event.sourceType = SourceType::TOUCH;
        case OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHPAD:
            event.sourceType = SourceType::TOUCH_PAD;
            break;
        case OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE:
            event.sourceType = SourceType::MOUSE;
        default:
            event.sourceType = SourceType::NONE;
    }
}

TouchPoint ConvertTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    int32_t pointerID = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem item;
    bool ret = pointerEvent->GetPointerItem(pointerID, item);
    if (!ret) {
        LOGE("get pointer item failed.");
        return TouchPoint();
    }
    std::chrono::microseconds micros(pointerEvent->GetActionTime());
    TimeStamp time(micros);

    int32_t pressWidth = item.GetWidth();
    int32_t pressHeight = item.GetHeight();
    double size = std::max(pressWidth, pressHeight) / 2.0; // just get the max of width and height
    TouchPoint point { pointerID, item.GetLocalX(), item.GetLocalY(), TouchType::UNKNOWN, time, size };
    int32_t orgDevice = pointerEvent->GetSourceType();
    GetEventDevice(orgDevice, point);
    int32_t orgAction = pointerEvent->GetPointerAction();
    switch (orgAction) {
        case OHOS::MMI::PointerEvent::POINTER_ACTION_CANCEL:
            point.type = TouchType::CANCEL;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_DOWN:
            point.type = TouchType::DOWN;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_MOVE:
            point.type = TouchType::MOVE;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_UP:
            point.type = TouchType::UP;
            break;
        default:
            LOGW("unknown type");
            break;
    }
    return point;
}

void GetMouseEventAction(int32_t action, MouseEvent& events)
{
    switch (action) {
        case OHOS::MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN:
            events.action = MouseAction::PRESS;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_BUTTON_UP:
            events.action = MouseAction::RELEASE;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_MOVE:
            events.action = MouseAction::MOVE;
            break;
        default:
            events.action = MouseAction::NONE;
            break;
    }
}

void GetMouseEventButton(int32_t button, MouseEvent& events)
{
    switch (button) {
        case OHOS::MMI::PointerEvent::MOUSE_BUTTON_LEFT:
            events.button = MouseButton::LEFT_BUTTON;
            break;
        case OHOS::MMI::PointerEvent::MOUSE_BUTTON_RIGHT:
            events.button = MouseButton::RIGHT_BUTTON;
            break;
        case OHOS::MMI::PointerEvent::MOUSE_BUTTON_MIDDLE:
            events.button = MouseButton::MIDDLE_BUTTON;
            break;
        default:
            events.button = MouseButton::NONE_BUTTON;
            break;
    }
}

void ConvertMouseEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, MouseEvent& events)
{
    int32_t pointerID = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem item;
    bool ret = pointerEvent->GetPointerItem(pointerID, item);
    if (!ret) {
        LOGE("get pointer item failed.");
        return;
    }

    events.x = item.GetLocalX();
    events.y = item.GetLocalY();
    int32_t orgAction = pointerEvent->GetPointerAction();
    GetMouseEventAction(orgAction, events);
    int32_t orgButton = pointerEvent->GetButtonId();
    GetMouseEventButton(orgButton, events);
    int32_t orgDevice = pointerEvent->GetSourceType();
    GetEventDevice(orgDevice, events);

    std::set<int32_t> pressedSet = pointerEvent->GetPressedButtons();
    uint32_t pressedButtons = 0;
    if (pressedSet.find(OHOS::MMI::PointerEvent::MOUSE_BUTTON_LEFT) != pressedSet.end()) {
        pressedButtons &= static_cast<uint32_t>(MouseButton::LEFT_BUTTON);
    }
    if (pressedSet.find(OHOS::MMI::PointerEvent::MOUSE_BUTTON_RIGHT) != pressedSet.end()) {
        pressedButtons &= static_cast<uint32_t>(MouseButton::RIGHT_BUTTON);
    }
    if (pressedSet.find(OHOS::MMI::PointerEvent::MOUSE_BUTTON_MIDDLE) != pressedSet.end()) {
        pressedButtons &= static_cast<uint32_t>(MouseButton::MIDDLE_BUTTON);
    }
    events.pressedButtons = static_cast<int32_t>(pressedButtons);

    std::chrono::microseconds micros(item.GetDownTime());
    TimeStamp time(micros);
    events.time = time;
}

} // namespace

FlutterAceView* FlutterAceView::CreateView(int32_t instanceId, bool useCurrentEventRunner, bool usePlatfromThread)
{
    FlutterAceView* aceSurface = new Platform::FlutterAceView(instanceId);
    flutter::Settings settings;
    settings.instanceId = instanceId;
    settings.platform = flutter::AcePlatform::ACE_PLATFORM_OHOS;
#ifndef GPU_DISABLED
    settings.enable_software_rendering = false;
#else
    settings.enable_software_rendering = true;
#endif
    settings.platform_as_ui_thread = usePlatfromThread;
    settings.use_current_event_runner = useCurrentEventRunner;
    LOGI("software render: %{public}s", settings.enable_software_rendering ? "true" : "false");
    LOGI("use platform as ui thread: %{public}s", settings.platform_as_ui_thread ? "true" : "false");
    settings.idle_notification_callback = [aceSurface](int64_t deadline) {
        if (aceSurface != nullptr) {
            aceSurface->ProcessIdleEvent(deadline);
        }
    };
    auto shell_holder = std::make_unique<flutter::OhosShellHolder>(settings, false);
    if (aceSurface != nullptr) {
        aceSurface->SetShellHolder(std::move(shell_holder));
    }
    return aceSurface;
}

void FlutterAceView::SurfaceCreated(FlutterAceView* view, OHOS::sptr<OHOS::Rosen::Window> window)
{
    LOGI(">>> FlutterAceView::SurfaceCreated, pWnd:%{public}p", &(*window));
    if (window == nullptr) {
        LOGE("FlutterAceView::SurfaceCreated, window is nullptr");
        return;
    }
    if (view == nullptr) {
        LOGE("FlutterAceView::SurfaceCreated, view is nullptr");
        return;
    }

    auto platformView = view->GetShellHolder()->GetPlatformView();
    LOGI("FlutterAceView::SurfaceCreated, GetPlatformView");
    if (platformView && !SystemProperties::GetRosenBackendEnabled()) {
        LOGI("FlutterAceView::SurfaceCreated, call NotifyCreated");
        platformView->NotifyCreated(window);
    }

    LOGI("<<< FlutterAceView::SurfaceCreated, end");
}

void FlutterAceView::SurfaceChanged(FlutterAceView* view, int32_t width, int32_t height, int32_t orientation)
{
    if (view == nullptr) {
        LOGE("FlutterAceView::SurfaceChanged, view is nullptr");
        return;
    }

    view->NotifySurfaceChanged(width, height);
    auto platformView = view->GetShellHolder()->GetPlatformView();
    LOGI("FlutterAceView::SurfaceChanged, GetPlatformView");
    if (platformView) {
        LOGI("FlutterAceView::SurfaceChanged, call NotifyChanged");
        platformView->NotifyChanged(SkISize::Make(width, height));
    }
    LOGI("<<< FlutterAceView::SurfaceChanged, end");
}

void FlutterAceView::SetViewportMetrics(FlutterAceView* view, const flutter::ViewportMetrics& metrics)
{
    if (view) {
        view->NotifyDensityChanged(metrics.device_pixel_ratio);
        view->NotifySystemBarHeightChanged(metrics.physical_padding_top, metrics.physical_view_inset_bottom);
        auto platformView = view->GetShellHolder()->GetPlatformView();
        if (platformView) {
            platformView->SetViewportMetrics(metrics);
        }
    }
}

void FlutterAceView::DispatchTouchEvent(FlutterAceView* view, const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent->GetSourceType() == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        // mouse event
        LOGD("DispatchTouchEvent MouseEvent");
        view->ProcessMouseEvent(pointerEvent);
    } else {
        // touch event
        LOGD("DispatchTouchEvent TouchEvent");
        view->ProcessTouchEvent(pointerEvent);
    }
}

bool FlutterAceView::DispatchKeyEvent(FlutterAceView* view, int32_t keyCode, int32_t action, int32_t repeatTime,
    int64_t timeStamp, int64_t timeStampStart)
{
    if (view != nullptr) {
        return view->ProcessKeyEvent(keyCode, action, repeatTime, timeStamp, timeStampStart);
    }
    LOGE("view is null, return false!");
    return false;
}

bool FlutterAceView::DispatchRotationEvent(FlutterAceView* view, float rotationValue)
{
    if (view) {
        return view->ProcessRotationEvent(rotationValue);
    }
    LOGE("view is null, return false!");
    return false;
}

void FlutterAceView::RegisterTouchEventCallback(TouchEventCallback&& callback)
{
    ACE_DCHECK(callback);
    touchEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterKeyEventCallback(KeyEventCallback&& callback)
{
    ACE_DCHECK(callback);
    keyEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterMouseEventCallback(MouseEventCallback&& callback)
{
    ACE_DCHECK(callback);
    mouseEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterRotationEventCallback(RotationEventCallBack&& callback)
{
    ACE_DCHECK(callback);
    rotationEventCallBack_ = std::move(callback);
}

void FlutterAceView::Launch()
{
    LOGD("Launch shell holder.");
    if (!viewLaunched_) {
        flutter::RunConfiguration config;
        shell_holder_->Launch(std::move(config));
        viewLaunched_ = true;
    }
}

void FlutterAceView::SetShellHolder(std::unique_ptr<flutter::OhosShellHolder> holder)
{
    shell_holder_ = std::move(holder);
}

void FlutterAceView::ProcessTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    TouchPoint touchPoint = ConvertTouchEvent(pointerEvent);
    if (touchPoint.type != TouchType::UNKNOWN) {
        if (touchEventCallback_) {
            touchEventCallback_(touchPoint);
        }
    } else {
        LOGW("Unknown event.");
    }
}

void FlutterAceView::ProcessMouseEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    MouseEvent event;
    ConvertMouseEvent(pointerEvent, event);
    LOGD("ProcessMouseEvent event");

    if (mouseEventCallback_) {
        mouseEventCallback_(event);
    }
}

bool FlutterAceView::ProcessKeyEvent(
    int32_t keyCode, int32_t keyAction, int32_t repeatTime, int64_t timeStamp, int64_t timeStampStart)
{
    if (!keyEventCallback_) {
        return false;
    }

    auto keyEvents = keyEventRecognizer_.GetKeyEvents(keyCode, keyAction, repeatTime, timeStamp, timeStampStart);
    // First distributes special events.
    // Because the platform receives a raw event, the identified special event processing result is ignored
    if (keyEvents.size() > 1) {
        keyEventCallback_(keyEvents.back());
    }
    return keyEventCallback_(keyEvents.front());
}

void FlutterAceView::ProcessIdleEvent(int64_t deadline)
{
    if (idleCallback_) {
        idleCallback_(deadline);
    }
}

const void* FlutterAceView::GetNativeWindowById(uint64_t textureId)
{
    return nullptr;
}

bool FlutterAceView::ProcessRotationEvent(float rotationValue)
{
    if (!rotationEventCallBack_) {
        return false;
    }

    RotationEvent event { .value = rotationValue * ROTATION_DIVISOR };

    return rotationEventCallBack_(event);
}

bool FlutterAceView::Dump(const std::vector<std::string>& params)
{
    if (params.empty() || params[0] != "-drawcmd") {
        LOGE("Unsupported parameters.");
        return false;
    }
#ifdef DUMP_DRAW_CMD
    if (shell_holder_) {
        auto screenShot = shell_holder_->Screenshot(flutter::Rasterizer::ScreenshotType::SkiaPicture, false);
        if (screenShot.data->data() != nullptr) {
            auto byteData = screenShot.data;
            static int32_t count = 0;
            auto path = ImageCache::GetImageCacheFilePath() + "/picture_" + std::to_string(count++) + ".mskp";
            if (DumpLog::GetInstance().GetDumpFile()) {
                DumpLog::GetInstance().AddDesc("Dump draw command to path: " + path);
                DumpLog::GetInstance().Print(0, "Info:", 0);
            }
            std::ofstream outFile(path, std::fstream::out | std::fstream::binary);
            if (!outFile.is_open()) {
                LOGE("Open file %{private}s failed.", path.c_str());
                return false;
            }
            outFile.write(reinterpret_cast<const char*>(byteData->data()), byteData->size());
            outFile.close();
            return true;
        }
    }
#else
    if (DumpLog::GetInstance().GetDumpFile()) {
        DumpLog::GetInstance().AddDesc("Dump draw command not support on this version.");
        DumpLog::GetInstance().Print(0, "Info:", 0);
        return true;
    }
#endif
    return false;
}

void FlutterAceView::InitCacheFilePath(const std::string& path)
{
    if (!path.empty()) {
        ImageCache::SetImageCacheFilePath(path);
        ImageCache::SetCacheFileInfo();
    } else {
        LOGW("image cache path empty");
    }
}

bool FlutterAceView::IsLastPage() const
{
    auto container = AceEngine::Get().GetContainer(instanceId_);
    if (!container) {
        return false;
    }

    auto context = container->GetPipelineContext();
    if (!context) {
        return false;
    }

    return context->IsLastPage();
}

uint32_t FlutterAceView::GetBackgroundColor()
{
    return Color::WHITE.GetValue();
}

// On watch device, it's probable to quit the application unexpectedly when we slide our finger diagonally upward on the
// screen, so we do restrictions here.
bool FlutterAceView::IsNeedForbidToPlatform(TouchPoint point)
{
    if (point.type == TouchType::DOWN) {
        auto result = touchPointInfoMap_.try_emplace(point.id, TouchPointInfo(point.GetOffset()));
        if (!result.second) {
            result.first->second = TouchPointInfo(point.GetOffset());
        }

        return false;
    }

    auto iter = touchPointInfoMap_.find(point.id);
    if (iter == touchPointInfoMap_.end()) {
        return false;
    }
    if (iter->second.eventState_ == EventState::HORIZONTAL_STATE) {
        return false;
    } else if (iter->second.eventState_ == EventState::VERTICAL_STATE) {
        return true;
    }

    Offset offset = point.GetOffset() - iter->second.offset_;
    double deltaX = offset.GetX();
    double deltaY = std::abs(offset.GetY());

    if (point.type == TouchType::MOVE) {
        if (deltaX > 0.0) {
            if (deltaY / deltaX > PERMIT_ANGLE_VALUE) {
                iter->second.eventState_ = EventState::VERTICAL_STATE;
                return true;
            } else {
                iter->second.eventState_ = EventState::HORIZONTAL_STATE;
            }
        }

        return false;
    }

    touchPointInfoMap_.erase(point.id);
    return deltaX > 0.0 && deltaY / deltaX > PERMIT_ANGLE_VALUE;
}

std::unique_ptr<DrawDelegate> FlutterAceView::GetDrawDelegate()
{
    auto darwDelegate = std::make_unique<DrawDelegate>();

    darwDelegate->SetDrawFrameCallback([this](RefPtr<Flutter::Layer>& layer, const Rect& dirty) {
        if (!layer) {
            return;
        }
        RefPtr<Flutter::FlutterSceneBuilder> flutterSceneBuilder = AceType::MakeRefPtr<Flutter::FlutterSceneBuilder>();
        layer->AddToScene(*flutterSceneBuilder, 0.0, 0.0);
        auto scene = flutterSceneBuilder->Build();
        if (!flutter::UIDartState::Current()) {
            LOGE("uiDartState is nullptr");
            return;
        }
        auto window = flutter::UIDartState::Current()->window();
        if (window != nullptr && window->client() != nullptr) {
            window->client()->Render(scene.get());
        }
    });

    return darwDelegate;
}

std::unique_ptr<PlatformWindow> FlutterAceView::GetPlatformWindow()
{
    return nullptr;
}

} // namespace OHOS::Ace::Platform
