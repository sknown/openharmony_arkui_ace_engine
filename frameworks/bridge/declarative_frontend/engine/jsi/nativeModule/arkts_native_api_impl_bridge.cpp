/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_api_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_button_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_blank_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_common_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_counter_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_image_span_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_text_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_toggle_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_radio_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_textpicker_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_timepicker_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_rating_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_slider_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_checkbox_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_select_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_divider_bridge.h"

namespace OHOS::Ace::NG {
ArkUINativeModuleValue ArkUINativeModule::GetFrameNodeById(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    int nodeId = firstArg->ToNumber(vm)->Value();
    auto nodePtr = GetArkUIInternalNodeAPI()->GetFrameNodeById(nodeId);
    return panda::NativePointerRef::New(vm, nodePtr);
}

ArkUINativeModuleValue ArkUINativeModule::GetArkUINativeModule(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    auto object = panda::ObjectRef::New(vm);
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "getFrameNodeById"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), GetFrameNodeById));

    auto common = panda::ObjectRef::New(vm);
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBackgroundColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetBackgroundColor));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBackgroundColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetBackgroundColor));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setWidth"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetWidth));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetWidth"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetWidth));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setHeight"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetHeight));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetHeight"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetHeight));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBorderRadius"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetBorderRadius));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBorderRadius"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetBorderRadius));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBorderWidth"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetBorderWidth));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBorderWidth"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetBorderWidth));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setTransform"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetTransform));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetTransform"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetTransform));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBorderColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetBorderColor));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBorderColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetBorderColor));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setPosition"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetPosition));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetPosition"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetPosition));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBorderStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetBorderStyle));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBorderStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetBorderStyle));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setShadow"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetShadow));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetShadow"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetShadow));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setHitTestBehavior"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetHitTestBehavior));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetHitTestBehavior"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetHitTestBehavior));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setZIndex"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetZIndex));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetZIndex"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetZIndex));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setOpacity"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetOpacity));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "ResetOpacity"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetOpacity));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setAlign"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetAlign));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetAlign"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetAlign));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBackdropBlur"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetBackdropBlur));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBackdropBlur"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetBackdropBlur));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setHueRotate"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetHueRotate));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetHueRotate"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetHueRotate));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setInvert"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetInvert));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetInvert"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetInvert));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSepia"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetSepia));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSepia"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetSepia));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSaturate"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetSaturate));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSaturate"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetSaturate));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setColorBlend"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetColorBlend));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetColorBlend"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetColorBlend));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setGrayscale"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetGrayscale));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetGrayscale"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetGrayscale));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setContrast"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetContrast));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetContrast"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetContrast));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBrightness"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetBrightness));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBrightness"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetBrightness));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBlur"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetBlur));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBlur"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetBlur));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setLinearGradient"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetLinearGradient));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetLinearGradient"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetLinearGradient));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSweepGradient"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetSweepGradient));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSweepGradient"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetSweepGradient));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setRadialGradient"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetRadialGradient));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetRadialGradient"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetRadialGradient));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setForegroundBlurStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetForegroundBlurStyle));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetForegroundBlurStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetForegroundBlurStyle));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setLinearGradientBlur"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetLinearGradientBlur));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetLinearGradientBlur"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetLinearGradientBlur));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBackgroundBlurStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetBackgroundBlurStyle));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBackgroundBlurStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetBackgroundBlurStyle));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBorder"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetBorder));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBorder"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetBorder));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBackgroundImagePosition"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetBackgroundImagePosition));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBackgroundImagePosition"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetBackgroundImagePosition));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBackgroundImageSize"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetBackgroundImageSize));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBackgroundImageSize"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetBackgroundImageSize));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBackgroundImage"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetBackgroundImage));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBackgroundImage"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetBackgroundImage));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setTranslate"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetTranslate));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetTranslate"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetTranslate));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setScale"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetScale));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetScale"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetScale));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setRotate"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetRotate));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetRotate"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetRotate));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setGeometryTransition"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetGeometryTransition));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetGeometryTransition"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetGeometryTransition));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setClip"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetClip));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetClip"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetClip));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setPixelStretchEffect"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetPixelStretchEffect));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetPixelStretchEffect"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetPixelStretchEffect));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setLightUpEffect"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetLightUpEffect));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetLightUpEffect"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetLightUpEffect));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSphericalEffect"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetSphericalEffect));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSphericalEffect"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetSphericalEffect));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setRenderGroup"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetRenderGroup));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetRenderGroup"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetRenderGroup));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setRenderFit"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetRenderFit));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetRenderFit"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetRenderFit));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setUseEffect"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetUseEffect));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetUseEffect"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetUseEffect));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "setForegroundColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::SetForegroundColor));
    common->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetForegroundColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CommonBridge::ResetForegroundColor));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "common"), common);

    auto counter = panda::ObjectRef::New(vm);
    counter->Set(vm, panda::StringRef::NewFromUtf8(vm, "setEnableInc"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CounterBridge::SetEnableInc));
    counter->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetEnableInc"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CounterBridge::ResetEnableInc));
    counter->Set(vm, panda::StringRef::NewFromUtf8(vm, "setEnableDec"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CounterBridge::SetEnableDec));
    counter->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetEnableDec"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CounterBridge::ResetEnableDec));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "counter"), counter);

    auto text = panda::ObjectRef::New(vm);
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "setFontColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::SetFontColor));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetFontColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::ResetFontColor));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "setFontSize"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::SetFontSize));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetFontSize"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::ResetFontSize));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "setFontStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::SetFontStyle));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetFontStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::ResetFontStyle));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "setTextAlign"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::SetTextAlign));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetTextAlign"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::ResetTextAlign));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "setFontWeight"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::SetFontWeight));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetFontWeight"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::ResetFontWeight));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "setLineHeight"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::SetLineHeight));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetLineHeight"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::ResetLineHeight));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "setTextOverflow"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::SetTextOverflow));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetTextOverflow"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::ResetTextOverflow));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "setDecoration"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::SetDecoration));
    text->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetDecoration"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextBridge::ResetDecoration));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "text"), text);

    auto select = panda::ObjectRef::New(vm);
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSpace"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::SetSpace));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "setValue"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::SetValue));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSelected"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::SetSelected));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "setFontColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::SetFontColor));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSelectedOptionBgColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::SetSelectedOptionBgColor));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "setOptionBgColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::SetOptionBgColor));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "setOptionFontColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::SetOptionFontColor));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSelectedOptionFontColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::SetSelectedOptionFontColor));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "setArrowPosition"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::SetArrowPosition));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "setMenuAlign"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::SetMenuAlign));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "setFont"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::SetFont));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "setOptionFont"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::SetOptionFont));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSelectedOptionFont"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::SetSelectedOptionFont));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetArrowPosition"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::ResetArrowPosition));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetMenuAlign"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::ResetMenuAlign));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetFont"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::ResetFont));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetOptionFont"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::ResetOptionFont));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSelectedOptionFont"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::ResetSelectedOptionFont));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSpace"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::ResetSpace));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetValue"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::ResetValue));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSelected"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::ResetSelected));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetFontColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::ResetFontColor));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSelectedOptionBgColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::ResetSelectedOptionBgColor));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetOptionBgColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::ResetOptionBgColor));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetOptionFontColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::ResetOptionFontColor));
    select->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSelectedOptionFontColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SelectBridge::ResetSelectedOptionFontColor));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "select"), select);

    auto radio = panda::ObjectRef::New(vm);
    radio->Set(vm, panda::StringRef::NewFromUtf8(vm, "setRadioChecked"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), RadioBridge::SetRadioChecked));
    radio->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetRadioChecked"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), RadioBridge::ResetRadioChecked));
    radio->Set(vm, panda::StringRef::NewFromUtf8(vm, "setRadioStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), RadioBridge::SetRadioStyle));
    radio->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetRadioStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), RadioBridge::ResetRadioStyle));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "radio"), radio);
    
    auto checkbox = panda::ObjectRef::New(vm);
    checkbox->Set(vm, panda::StringRef::NewFromUtf8(vm, "setMark"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CheckboxBridge::SetMark));
    checkbox->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetMark"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CheckboxBridge::ResetMark));
    checkbox->Set(vm, panda::StringRef::NewFromUtf8(vm, "setUnSelectedColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CheckboxBridge::SetUnSelectedColor));
    checkbox->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetUnSelectedColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CheckboxBridge::ResetUnSelectedColor));
    checkbox->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSelect"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CheckboxBridge::SetSelect));
    checkbox->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSelect"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CheckboxBridge::ResetSelect));
    checkbox->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSelectedColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CheckboxBridge::SetSelectedColor));
    checkbox->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSelectedColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CheckboxBridge::ResetSelectedColor));
    checkbox->Set(vm, panda::StringRef::NewFromUtf8(vm, "setWidth"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CheckboxBridge::SetCheckboxWidth));
    checkbox->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetWidth"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CheckboxBridge::ResetCheckboxWidth));
    checkbox->Set(vm, panda::StringRef::NewFromUtf8(vm, "setHeight"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CheckboxBridge::SetCheckboxHeight));
    checkbox->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetHeight"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), CheckboxBridge::ResetCheckboxHeight));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "checkbox"), checkbox);
    
    auto textpicker = panda::ObjectRef::New(vm);
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "setCanLoop"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::SetCanLoop));
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSelectedIndex"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::SetSelectedIndex));
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "setTextStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::SetTextStyle));
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSelectedTextStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::SetSelectedTextStyle));
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "setDisappearTextStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::SetDisappearTextStyle));
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "setDefaultPickerItemHeight"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::SetDefaultPickerItemHeight));
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBackgroundColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::SetBackgroundColor));
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetCanLoop"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::ResetCanLoop));
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSelectedIndex"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::ResetSelectedIndex));
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetTextStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::ResetTextStyle));
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSelectedTextStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::ResetSelectedTextStyle));
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetDisappearTextStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::ResetDisappearTextStyle));
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetDefaultPickerItemHeight"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::ResetDefaultPickerItemHeight));
    textpicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBackgroundColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TextpickerBridge::ResetBackgroundColor));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "textpicker"), textpicker);

    auto timepicker = panda::ObjectRef::New(vm);
    timepicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "setTextStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TimepickerBridge::SetTextStyle));
    timepicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSelectedTextStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TimepickerBridge::SetSelectedTextStyle));
    timepicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "setDisappearTextStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TimepickerBridge::SetDisappearTextStyle));
    timepicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetTextStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TimepickerBridge::ResetTextStyle));
    timepicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSelectedTextStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TimepickerBridge::ResetSelectedTextStyle));
    timepicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetDisappearTextStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TimepickerBridge::ResetDisappearTextStyle));
    timepicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBackgroundColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TimepickerBridge::SetTimepickerBackgroundColor));
    timepicker->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBackgroundColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), TimepickerBridge::ResetTimepickerBackgroundColor));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "timepicker"), timepicker);
    
    auto rating = panda::ObjectRef::New(vm);
    rating->Set(vm, panda::StringRef::NewFromUtf8(vm, "setStars"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), RatingBridge::SetStars));
    rating->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetStars"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), RatingBridge::ResetStars));
    rating->Set(vm, panda::StringRef::NewFromUtf8(vm, "setStepSize"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), RatingBridge::SetRatingStepSize));
    rating->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetStepSize"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), RatingBridge::ResetRatingStepSize));
    rating->Set(vm, panda::StringRef::NewFromUtf8(vm, "setStarStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), RatingBridge::SetStarStyle));
    rating->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetStarStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), RatingBridge::ResetStarStyle));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "rating"), rating);
    
    auto slider = panda::ObjectRef::New(vm);
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setShowTips"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::SetShowTips));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetShowTips"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::ResetShowTips));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setStepSize"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::SetSliderStepSize));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetStepSize"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::ResetSliderStepSize));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBlockSize"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::SetBlockSize));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBlockSize"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::ResetBlockSize));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setTrackBorderRadius"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::SetTrackBorderRadius));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetTrackBorderRadius"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::ResetTrackBorderRadius));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setStepColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::SetStepColor));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetStepColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::ResetStepColor));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBlockBorderColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::SetBlockBorderColor));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBlockBorderColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::ResetBlockBorderColor));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBlockBorderWidth"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::SetBlockBorderWidth));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBlockBorderWidth"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::ResetBlockBorderWidth));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setBlockColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::SetBlockColor));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetBlockColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::ResetBlockColor));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setTrackBackgroundColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::SetTrackBackgroundColor));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetTrackBackgroundColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::ResetTrackBackgroundColor));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSelectColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::SetSelectColor));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSelectColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::ResetSelectColor));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setShowSteps"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::SetShowSteps));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetShowSteps"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::ResetShowSteps));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setThickness"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::SetThickness));
    slider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetThickness"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), SliderBridge::ResetThickness));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "slider"), slider);

    auto imageSpan = panda::ObjectRef::New(vm);
    imageSpan->Set(vm, panda::StringRef::NewFromUtf8(vm, "setVerticalAlign"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ImageSpanBridge::SetVerticalAlign));
    imageSpan->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetVerticalAlign"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ImageSpanBridge::ResetVerticalAlign));
    imageSpan->Set(vm, panda::StringRef::NewFromUtf8(vm, "setObjectFit"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ImageSpanBridge::SetObjectFit));
    imageSpan->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetObjectFit"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ImageSpanBridge::ResetObjectFit));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "imageSpan"), imageSpan);

    auto blank = panda::ObjectRef::New(vm);
    blank->Set(vm, panda::StringRef::NewFromUtf8(vm, "setColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), BlankBridge::SetColor));
    blank->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), BlankBridge::ResetColor));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "blank"), blank);
    
    RegisterButtonAttributes(object, vm);
    RegisterToggleAttributes(object, vm);
    RegisterDividerAttributes(object, vm);

    return object;
}

void ArkUINativeModule::RegisterButtonAttributes(Local<panda::ObjectRef> object, EcmaVM* vm)
{
    auto button = panda::ObjectRef::New(vm);
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "setType"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::SetType));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetType"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::ResetType));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "setStateEffect"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::SetStateEffect));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetStateEffect"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::ResetStateEffect));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "setFontColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::SetFontColor));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetFontColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::ResetFontColor));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "setFontSize"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::SetFontSize));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetFontSize"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::ResetFontSize));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "setFontWeight"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::SetFontWeight));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetFontWeight"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::ResetFontWeight));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "setFontStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::SetFontStyle));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetFontStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::ResetFontStyle));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "setFontFamily"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::SetFontFamily));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetFontFamily"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::ResetFontFamily));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "setLabelStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::SetLabelStyle));
    button->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetLabelStyle"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ButtonBridge::ResetLabelStyle));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "button"), button);
}

void ArkUINativeModule::RegisterToggleAttributes(Local<panda::ObjectRef> object, EcmaVM* vm)
{
    auto toggle = panda::ObjectRef::New(vm);
    toggle->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSelectedColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ToggleBridge::SetSelectedColor));
    toggle->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSelectedColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ToggleBridge::ResetSelectedColor));
    toggle->Set(vm, panda::StringRef::NewFromUtf8(vm, "setSwitchPointColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ToggleBridge::SetSwitchPointColor));
    toggle->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetSwitchPointColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ToggleBridge::ResetSwitchPointColor));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "toggle"), toggle);
}

void ArkUINativeModule::RegisterDividerAttributes(Local<panda::ObjectRef> object, EcmaVM* vm)
{
    auto divider = panda::ObjectRef::New(vm);
    divider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setStrokeWidth"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), DividerBridge::SetStrokeWidth));
    divider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetStrokeWidth"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), DividerBridge::ResetStrokeWidth));
    divider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setLineCap"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), DividerBridge::SetLineCap));
    divider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetLineCap"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), DividerBridge::ResetLineCap));
    divider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), DividerBridge::SetColor));
    divider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetColor"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), DividerBridge::ResetColor));
    divider->Set(vm, panda::StringRef::NewFromUtf8(vm, "setVertical"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), DividerBridge::SetVertical));
    divider->Set(vm, panda::StringRef::NewFromUtf8(vm, "resetVertical"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), DividerBridge::ResetVertical));
    object->Set(vm, panda::StringRef::NewFromUtf8(vm, "divider"), divider);
}
} // namespace OHOS::Ace::NG
