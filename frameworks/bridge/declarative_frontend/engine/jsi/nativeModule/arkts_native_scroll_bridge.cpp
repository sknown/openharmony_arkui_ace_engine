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

#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_scroll_bridge.h"

#include "base/log/log.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "bridge/declarative_frontend/jsview/js_scroller.h"
#include "core/components_ng/pattern/scroll/scroll_model.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"
#include "frameworks/core/components/scroll_bar/scroll_proxy.h"

namespace OHOS::Ace::NG {
constexpr int NUM_0 = 0;
constexpr int NUM_1 = 1;
constexpr int NUM_2 = 2;
constexpr double FRICTION_DEFAULT = 0.6;
constexpr int32_t FROWARD_INITIAL_VALUE = 0;
constexpr int32_t BACKWARD_INITIAL_VALUE = 0;

bool ParsePagination(const EcmaVM* vm, const Local<JSValueRef>& paginationValue,
    std::vector<ArkUI_Float32>& vPaginationValue,
    std::vector<int32_t>& vPaginationUnit)
{
    uint32_t pLength = 0;
    if (paginationValue->IsArray(vm)) {
        auto paginationArray = panda::Local<panda::ArrayRef>(paginationValue);
        pLength = paginationArray->Length(vm);
        if (pLength <= 0) {
            return false;
        }
        for (uint32_t i = 0; i < pLength; i++) {
            CalcDimension dims;
            Local<JSValueRef> xValue = panda::ArrayRef::GetValueAt(vm, paginationArray, i);
            if (!ArkTSUtils::ParseJsDimensionVpNG(vm, xValue, dims, true)) {
                return false;
            }
            vPaginationValue.push_back(static_cast<ArkUI_Float32>(dims.Value()));
            vPaginationUnit.push_back(static_cast<int32_t>(dims.Unit()));
        }
    } else {
        CalcDimension intervalSize;
        if (!ArkTSUtils::ParseJsDimensionVp(vm, paginationValue, intervalSize) || intervalSize.IsNegative()) {
            intervalSize = CalcDimension(0.0);
        }
        vPaginationValue.push_back(static_cast<ArkUI_Float32>(intervalSize.Value()));
        vPaginationUnit.push_back(static_cast<int32_t>(intervalSize.Unit()));
    }

    return true;
}

ArkUINativeModuleValue ScrollBridge::SetNestedScroll(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> scrollForwardValue = runtimeCallInfo->GetCallArgRef(1);  // 1: index of scroll forward value
    Local<JSValueRef> scrollBackwardValue = runtimeCallInfo->GetCallArgRef(2); // 2: index of scroll backward value
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    auto froward = 0;
    auto backward = 0;
    ArkTSUtils::ParseJsInteger(vm, scrollForwardValue, froward);
    if (froward < static_cast<int32_t>(NestedScrollMode::SELF_ONLY) ||
        froward > static_cast<int32_t>(NestedScrollMode::PARALLEL)) {
        froward = FROWARD_INITIAL_VALUE;
    }
    ArkTSUtils::ParseJsInteger(vm, scrollBackwardValue, backward);
    if (backward < static_cast<int32_t>(NestedScrollMode::SELF_ONLY) ||
        backward > static_cast<int32_t>(NestedScrollMode::PARALLEL)) {
        backward = BACKWARD_INITIAL_VALUE;
    }
    GetArkUINodeModifiers()->getScrollModifier()->setScrollNestedScroll(nativeNode, froward, backward);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::ResetNestedScroll(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getScrollModifier()->resetScrollNestedScroll(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::SetEnableScroll(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> isEnabledArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    bool isEnabled = isEnabledArg->IsBoolean() ? isEnabledArg->ToBoolean(vm)->Value() : true;
    GetArkUINodeModifiers()->getScrollModifier()->setScrollEnableScroll(nativeNode, isEnabled);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::ResetEnableScroll(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getScrollModifier()->resetScrollEnableScroll(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::SetFriction(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> scrollFrictionArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    double friction = FRICTION_DEFAULT;
    if (!ArkTSUtils::ParseJsDouble(vm, scrollFrictionArg, friction)) {
        GetArkUINodeModifiers()->getScrollModifier()->resetScrollFriction(nativeNode);
    } else {
        GetArkUINodeModifiers()->getScrollModifier()->setScrollFriction(nativeNode,
            static_cast<ArkUI_Float32>(friction));
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::ResetFriction(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getScrollModifier()->resetScrollFriction(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::SetScrollSnap(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> snapAlignValue = runtimeCallInfo->GetCallArgRef(1);         // 1: index of snap align value
    Local<JSValueRef> paginationValue = runtimeCallInfo->GetCallArgRef(2);        // 2: index of pagination value
    Local<JSValueRef> enableSnapToStartValue = runtimeCallInfo->GetCallArgRef(3); // 3: index of enableSnapToStart value
    Local<JSValueRef> enableSnapToEndValue = runtimeCallInfo->GetCallArgRef(4);   // 4: index of enableSnapToEnd value
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    auto snapAlign = static_cast<int32_t>(ScrollSnapAlign::NONE);
    if (snapAlignValue->IsNull() || snapAlignValue->IsUndefined() ||
        !ArkTSUtils::ParseJsInteger(vm, snapAlignValue, snapAlign) ||
        snapAlign < static_cast<int32_t>(ScrollSnapAlign::NONE) ||
        snapAlign > static_cast<int32_t>(ScrollSnapAlign::END)) {
        snapAlign = static_cast<int32_t>(ScrollSnapAlign::NONE);
    }
    std::vector<ArkUI_Float32> vPaginationValue;
    std::vector<int32_t> vPaginationUnit;
    if (!ParsePagination(vm, paginationValue, vPaginationValue, vPaginationUnit)) {
        GetArkUINodeModifiers()->getScrollModifier()->resetScrollScrollSnap(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    bool isArray = true;
    if (!paginationValue->IsArray(vm)) {
        isArray = false;
    }
    auto pLength = vPaginationValue.size();
    vPaginationUnit.push_back(snapAlign);
    vPaginationUnit.push_back(static_cast<int32_t>(enableSnapToStartValue->ToBoolean(vm)->Value()));
    vPaginationUnit.push_back(static_cast<int32_t>(enableSnapToEndValue->ToBoolean(vm)->Value()));
    vPaginationUnit.push_back(static_cast<int32_t>(isArray));
    auto uLength = pLength + 4;
    GetArkUINodeModifiers()->getScrollModifier()->setScrollScrollSnap(
        nativeNode, vPaginationValue.data(), pLength, vPaginationUnit.data(), uLength);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::ResetScrollSnap(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getScrollModifier()->resetScrollScrollSnap(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::SetScrollBar(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> jsValue = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    auto value = static_cast<int32_t>(DisplayMode::AUTO);
    if (!jsValue->IsUndefined()) {
        ArkTSUtils::ParseJsInteger(vm, jsValue, value);
    }
    GetArkUINodeModifiers()->getScrollModifier()->setScrollScrollBar(nativeNode, value);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::ResetScrollBar(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getScrollModifier()->resetScrollScrollBar(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::SetScrollable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> scrollDirectionArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    if (scrollDirectionArg->IsUndefined() || scrollDirectionArg->IsNull()) {
        GetArkUINodeModifiers()->getScrollModifier()->resetScrollScrollable(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    int32_t scrollDirection = scrollDirectionArg->Int32Value(vm);
    if (scrollDirection != static_cast<int32_t>(Axis::VERTICAL) &&
        scrollDirection != static_cast<int32_t>(Axis::HORIZONTAL) &&
        scrollDirection != static_cast<int32_t>(Axis::FREE) && scrollDirection != static_cast<int32_t>(Axis::NONE)) {
        GetArkUINodeModifiers()->getScrollModifier()->resetScrollScrollable(nativeNode);
    } else {
        GetArkUINodeModifiers()->getScrollModifier()->setScrollScrollable(nativeNode, scrollDirection);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::ResetScrollable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getScrollModifier()->resetScrollScrollable(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::SetScrollBarColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> barcolorArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    Color color;
    if (!ArkTSUtils::ParseJsColorAlpha(vm, barcolorArg, color)) {
        GetArkUINodeModifiers()->getScrollModifier()->resetScrollScrollBarColor(nativeNode);
    } else {
        GetArkUINodeModifiers()->getScrollModifier()->setScrollScrollBarColor(nativeNode, color.GetValue());
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::ResetScrollBarColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getScrollModifier()->resetScrollScrollBarColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::SetScrollBarWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> scrollBarArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    CalcDimension scrollBarWidth;
    if (!ArkTSUtils::ParseJsDimensionVpNG(vm, scrollBarArg, scrollBarWidth, false)) {
        GetArkUINodeModifiers()->getScrollModifier()->resetScrollScrollBarWidth(nativeNode);
    } else {
        if (LessNotEqual(scrollBarWidth.Value(), 0.0)) {
            GetArkUINodeModifiers()->getScrollModifier()->resetScrollScrollBarWidth(nativeNode);
        } else {
            GetArkUINodeModifiers()->getScrollModifier()->setScrollScrollBarWidth(
                nativeNode, scrollBarWidth.Value(), static_cast<int32_t>(scrollBarWidth.Unit()));
        }
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::ResetScrollBarWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getScrollModifier()->resetScrollScrollBarWidth(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::SetEdgeEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> effectArg = runtimeCallInfo->GetCallArgRef(1);    // 1: index of effect value
    Local<JSValueRef> isEffectArg = runtimeCallInfo->GetCallArgRef(2);  // 2: index of isEffect value
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    int32_t effect = static_cast<int32_t>(EdgeEffect::NONE);
    if (!effectArg->IsUndefined() && !effectArg->IsNull()) {
        effect = effectArg->Int32Value(vm);
    }

    if (effect != static_cast<int32_t>(EdgeEffect::SPRING) && effect != static_cast<int32_t>(EdgeEffect::NONE) &&
        effect != static_cast<int32_t>(EdgeEffect::FADE)) {
        effect = static_cast<int32_t>(EdgeEffect::NONE);
    }

    if (isEffectArg->IsUndefined() || isEffectArg->IsNull()) {
        GetArkUINodeModifiers()->getScrollModifier()->setScrollEdgeEffect(nativeNode, effect, true);
    } else {
        GetArkUINodeModifiers()->getScrollModifier()->setScrollEdgeEffect(
            nativeNode, effect, isEffectArg->ToBoolean(vm)->Value());
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::ResetEdgeEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getScrollModifier()->resetScrollEdgeEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::SetEnablePaging(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> enablePagingArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    bool enablePaging;
    if (enablePagingArg->IsBoolean()) {
        enablePaging = enablePagingArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getScrollModifier()->setScrollEnablePaging(nativeNode, enablePaging);
    } else {
        GetArkUINodeModifiers()->getScrollModifier()->resetScrollEnablePaging(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::ResetEnablePaging(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getScrollModifier()->resetScrollEnablePaging(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::SetInitialOffset(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> xOffsetArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> yOffsetArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());

    CalcDimension x;
    ArkTSUtils::ParseJsDimensionVpNG(vm, xOffsetArg, x, false);
    CalcDimension y;
    ArkTSUtils::ParseJsDimensionVpNG(vm, yOffsetArg, y, false);

    GetArkUINodeModifiers()->getScrollModifier()->setScrollInitialOffset(nativeNode, x.Value(),
        static_cast<int32_t>(x.Unit()), y.Value(), static_cast<int32_t>(y.Unit()));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::ResetInitialOffset(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getScrollModifier()->resetScrollInitialOffset(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::SetFlingSpeedLimit(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> flingSpeedLimitArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    double max = -1.0;
    if (!ArkTSUtils::ParseJsDouble(vm, flingSpeedLimitArg, max)) {
        GetArkUINodeModifiers()->getScrollModifier()->resetScrollFlingSpeedLimit(nativeNode);
    } else {
        GetArkUINodeModifiers()->getScrollModifier()->setScrollFlingSpeedLimit(nativeNode,
            static_cast<ArkUI_Float32>(max));
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::ResetFlingSpeedLimit(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getScrollModifier()->resetScrollFlingSpeedLimit(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::SetScrollInitialize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());

    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    if (!info[1]->IsNull() && info[1]->IsObject()) {
        Framework::JSScroller* jsScroller =
            Framework::JSRef<Framework::JSObject>::Cast(info[1])->Unwrap<Framework::JSScroller>();
        if (jsScroller) {
            jsScroller->SetInstanceId(Container::CurrentId());
            auto positionController = GetArkUINodeModifiers()->getScrollModifier()->getScroll(nativeNode);
            auto nodePositionController =
                AceType::Claim(reinterpret_cast<OHOS::Ace::ScrollControllerBase*>(positionController));
            jsScroller->SetController(nodePositionController);
            // Init scroll bar proxy.
            auto proxy = jsScroller->GetScrollBarProxy();
            if (!proxy) {
                proxy = ScrollModel::GetInstance()->CreateScrollBarProxy();
                jsScroller->SetScrollBarProxy(proxy);
            }
            auto proxyPtr = reinterpret_cast<ArkUINodeHandle>(OHOS::Ace::AceType::RawPtr(proxy));
            GetArkUINodeModifiers()->getScrollModifier()->setScrollBarProxy(nativeNode, proxyPtr);
        }
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollBridge::ResetScrollInitialize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    return panda::JSValueRef::Undefined(vm);
}
} // namespace OHOS::Ace::NG
