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

#ifndef OHOS_ACE_FRAMEWORK_CJ_COMMON_FFI_H
#define OHOS_ACE_FRAMEWORK_CJ_COMMON_FFI_H

#include <cstdint>
#include <string>

#include "bridge/cj_frontend/interfaces/cj_ffi/cj_macro.h"
#include "bridge/cj_frontend/interfaces/cj_ffi/utils.h"
#include "core/event/touch_event.h"
#include "core/gestures/gesture_info.h"


extern "C" {
struct NativeFontInfo {
    const char* path = "";
    const char* postScriptName = "";
    const char* fullName = "" ;
    const char* family = "";
    const char* subfamily = "";
    uint32_t weight = 0;
    uint32_t width = 0;
    bool italic = false;
    bool monoSpace = false;
    bool symbolic = false;
};

struct NativeOptionFontInfo {
    bool hasValue;
    NativeFontInfo* info;
};

struct NativeLength {
    double value;
    int32_t unitType;
};

struct NativeOffset {
    NativeLength dx;
    NativeLength dy;
};

struct NativeOptionLength {
    bool hasValue;
    NativeLength value;
};

struct NativeOptionInt32 {
    bool hasValue;
    int32_t value;
};

struct NativeOptionInt64 {
    bool hasValue;
    int64_t value;
};

struct NativeOptionUInt32 {
    bool hasValue;
    uint32_t value;
};

struct NativeOptionFloat32 {
    bool hasValue;
    float value;
};

struct NativeOptionFloat64 {
    bool hasValue;
    double value;
};

struct NativeOptionCallBack {
    bool hasValue;
    void(*value)();
};

struct NativeOptionBool {
    bool hasValue;
    bool value;
};

struct CArrInt32 {
    int32_t* head;
    int64_t size;
};

struct NativeOptionCArrInt32 {
    bool hasValue;
    CArrInt32 value;
};

struct NativeOptionCString {
    bool hasValue;
    const char* value;
};

struct CJOffset {
    double xOffset;
    double yOffset;
};

struct CJTouchInfo {
    uint8_t type;
    int32_t fingerId;
    double globalX;
    double globalY;
    double localX;
    double localY;
};

struct CJTextPickerResult {
    const char* value;
    uint32_t index;
};

struct CJImageComplete {
    double width;
    double height;
    double componentWidth;
    double componentHeight;
    int32_t loadingStatus;
};

struct CJImageError {
    double componentWidth;
    double componentHeight;
};

struct CJPosition {
    double x;
    double y;
};

struct CJArea {
    double width;
    double height;
    CJPosition* position;
    CJPosition* globalPosition;
};

struct CJEventTarget {
    CJArea* area;
};

struct CJTouchEvent {
    uint8_t eventType;
    CJTouchInfo* touches;
    int32_t touchesSize;
    CJTouchInfo* changedTouches;
    int32_t changedTouchesSize;
    int64_t timestamp;
    CJEventTarget* target;
    int32_t sourceType;
    std::string ToString() const;
};

struct CJMouseEvent {
    int64_t timestamp;
    double screenX;
    double screenY;
    double x;
    double y;
    int32_t button;
    int32_t action;
};

struct CJClickInfo {
    double globalX;
    double globalY;
    double localX;
    double localY;
    int64_t timestamp;
    int32_t sourceType;
    CJEventTarget* target;
    std::string ToString() const;
};

struct ClickInfoForSpan {
    double globalX;
    double globalY;
    double localX;
    double localY;
    int64_t timestamp;
    int32_t sourceType;
    std::string ToString() const;
};

struct CJKeyEvent {
    const char* keyText;
    int32_t type;
    int32_t keyCode;
    int32_t keySource;
    int32_t metaKey;
    int64_t deviceId;
    int64_t timestamp;
};

struct CJFingerInfo {
    int32_t id;
    double globalX;
    double globalY;
    double localX;
    double localY;
};

struct CJGestureEvent {
    int64_t timestamp;
    CJEventTarget* target;
    bool repeat;
    CJFingerInfo* fingerList;
    int32_t fingerListSize;
    int32_t source;
    double offsetX;
    double offsetY;
    double scale;
    double pinchCenterX;
    double pinchCenterY;
    double angle;
    double speed;
};

struct CJDragInfo {
    const char* extraParams;
    CJPosition* position;
};

struct CJDragItemInfo {
    int64_t pixelMapId;
    void (*builder)();
    const char* extraInfo;
};


struct AtCPackage;

struct AtCOHOSAceFrameworkCJInstanceLoadEntryParams {
    const char* name;
    bool* result;

    AtCOHOSAceFrameworkCJInstanceLoadEntryParams(const char* name, bool* result) : name(name), result(result) {}
};

struct AtCOHOSAceFrameworkCJInstanceInitializeParams {
    int16_t runtimeId;
    int64_t version;
    AtCPackage* package;
    bool* result;

    AtCOHOSAceFrameworkCJInstanceInitializeParams(int16_t runtimeId, int64_t version, AtCPackage* package, bool* result)
        : runtimeId(runtimeId), version(version), package(package), result(result)
    {}
};

struct AtCPackage {
    bool (*atCOHOSAceFrameworkCJInstanceInitialize)(AtCOHOSAceFrameworkCJInstanceInitializeParams* params) = nullptr;
    bool (*atCOHOSAceFrameworkCJInstanceLoadEntry)(AtCOHOSAceFrameworkCJInstanceLoadEntryParams*) = nullptr;
    void (*atCOHOSAceFrameworkRemoteViewRender)(int64_t self) = nullptr;
    void (*atCOHOSAceFrameworkRemoteViewRerender)(int64_t self) = nullptr;
    void (*atCOHOSAceFrameworkRemoteViewRelease)(int64_t self) = nullptr;
    void (*atCOHOSAceFrameworkRemoteViewOnAppear)(int64_t self) = nullptr;
    void (*atCOHOSAceFrameworkRemoteViewOnShow)(int64_t self) = nullptr;
    void (*atCOHOSAceFrameworkRemoteViewOnHide)(int64_t self) = nullptr;
    bool (*atCOHOSAceFrameworkRemoteViewOnBackPress)(int64_t self) = nullptr;
    void (*atCOHOSAceFrameworkRemoteViewUpdateWithJson)(int64_t self, const char* json) = nullptr;
    void (*atCOHOSAceFrameworkRemoteViewOnTransition)(int64_t self) = nullptr;
    void (*atCOHOSAceFrameworkRemoteViewOnAboutToRender)(int64_t self) = nullptr;
    void (*atCOHOSAceFrameworkRemoteViewOnAfterRender)(int64_t self) = nullptr;
    void (*atCOHOSAceFrameworkRemoteViewOnDisappear)(int64_t self) = nullptr;
    void (*atCOHOSAceFrameworkRemoteViewOnAboutToBeDeleted)(int64_t self) = nullptr;
    ExternalString (*atCOHOSAceFrameworkLazyForEachFuncsGenerateKey)(int64_t self, int64_t idx) = nullptr;
    void (*atCOHOSAceFrameworkLazyForEachFuncsGenerateItem)(int64_t self, int64_t idx) = nullptr;
    int64_t (*atCOHOSAceFrameworkLazyForEachFuncsGetTotalCount)(int64_t self) = nullptr;
    void (*atCOHOSAceFrameworkLazyForEachFuncsMarkLazy)(int64_t self, const char* key) = nullptr;
    void (*atCOHOSAceFrameworkLazyForEachFuncsResetLazy)(int64_t self) = nullptr;
    void (*atCOHOSAceFrameworkLazyForEachFuncsRemoveChildGroup)(int64_t self, const char* composedId) = nullptr;
    void (*atCOHOSAceFrameworkLazyForEachFuncsDataChangeListenerRegister)(int64_t self, int64_t idx) = nullptr;
    void (*atCOHOSAceFrameworkLazyForEachFuncsDataChangeListenerUnregister)(int64_t self, int64_t idx) = nullptr;
};

CJ_EXPORT void FfiOHOSAceFrameworkRegisterCJFuncs(AtCPackage cjFuncs);

CJ_EXPORT int64_t FfiGeneralSizeOfPointer();
}

namespace OHOS::Ace {
void TransformNativeTouchLocationInfo(
    CJTouchInfo* sources, const std::list<OHOS::Ace::TouchLocationInfo>& touchLocationInfoList);

void TransformNativeCJFingerInfo(
    CJFingerInfo* sources, const std::list<OHOS::Ace::FingerInfo>& fingerInfoList);
}
#endif // OHOS_ACE_FRAMEWORK_CJ_COMMON_FFI_H
