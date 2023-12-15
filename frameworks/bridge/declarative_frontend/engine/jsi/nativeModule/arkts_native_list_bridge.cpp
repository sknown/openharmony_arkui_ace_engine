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
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_list_bridge.h"

#include "bridge/declarative_frontend/engine/jsi/components/arkts_native_api.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "frameworks/core/components/list/list_theme.h"

namespace OHOS::Ace::NG {
constexpr int32_t NUM_0 = 0;
constexpr int32_t NUM_1 = 1;
constexpr int32_t NUM_2 = 2;
constexpr int32_t NUM_3 = 3;
constexpr int32_t NUM_4 = 4;
constexpr int32_t NUM_5 = 5;
constexpr int32_t NUM_6 = 6;
constexpr int32_t NUM_7 = 7;

constexpr int32_t ARG_LENGTH = 3;

ArkUINativeModuleValue ListBridge::SetListLanes(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> fourthArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> fifthArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    ArkUIDimensionType gutterType;
    ArkUIDimensionType minLengthType;
    ArkUIDimensionType maxLengthType;

    CalcDimension gutter;
    if (fifthArg->IsUndefined() || !ArkTSUtils::ParseJsDimension(vm, fifthArg, gutter, DimensionUnit::VP)) {
        gutter = Dimension(0);
        gutterType.value = gutter.Value();
        gutterType.units = static_cast<int32_t>(gutter.Unit());
    }

    if (!secondArg->IsUndefined()) {
        CalcDimension minLength = 0.0_vp;
        CalcDimension maxLength = 0.0_vp;
        minLengthType.value = minLength.Value();
        minLengthType.units = static_cast<int32_t>(minLength.Unit());
        maxLengthType.value = maxLength.Value();
        maxLengthType.units = static_cast<int32_t>(maxLength.Unit());
        GetArkUIInternalNodeAPI()->GetListModifier().SetListLanes(
            nativeNode, secondArg->ToNumber(vm)->Value(), &minLengthType, &maxLengthType, &gutterType);
    } else {
        CalcDimension minLength;
        CalcDimension maxLength;
        ArkTSUtils::ParseJsDimension(vm, thirdArg, minLength, DimensionUnit::VP);
        ArkTSUtils::ParseJsDimension(vm, fourthArg, maxLength, DimensionUnit::VP);
        minLengthType.value = minLength.Value();
        minLengthType.units = static_cast<int32_t>(minLength.Unit());
        maxLengthType.value = maxLength.Value();
        maxLengthType.units = static_cast<int32_t>(maxLength.Unit());
        GetArkUIInternalNodeAPI()->GetListModifier().SetListLanes(
            nativeNode, -1, &minLengthType, &maxLengthType, &gutterType); // invild value .
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetListLanes(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetListLanes(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue ListBridge::SetEditMode(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsUndefined()) {
        GetArkUIInternalNodeAPI()->GetListModifier().ResetEditMode(nativeNode);
    } else {
        bool editMode = secondArg->ToBoolean(vm)->Value();
        GetArkUIInternalNodeAPI()->GetListModifier().SetEditMode(nativeNode, editMode);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetEditMode(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetEditMode(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetMultiSelectable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsUndefined()) {
        GetArkUIInternalNodeAPI()->GetListModifier().ResetMultiSelectable(nativeNode);
    } else {
        bool selectable = secondArg->ToBoolean(vm)->Value();
        GetArkUIInternalNodeAPI()->GetListModifier().SetMultiSelectable(nativeNode, selectable);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetMultiSelectable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetMultiSelectable(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetChainAnimation(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    bool chainAnimation = secondArg->ToBoolean(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().SetChainAnimation(nativeNode, chainAnimation);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetChainAnimation(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetChainAnimation(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetCachedCount(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t cachedCount = secondArg->Int32Value(vm);

    GetArkUIInternalNodeAPI()->GetListModifier().SetCachedCount(nativeNode, cachedCount);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetCachedCount(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetCachedCount(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetEnableScrollInteraction(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsBoolean()) {
        bool enableScrollInteraction = secondArg->ToBoolean(vm)->Value();
        GetArkUIInternalNodeAPI()->GetListModifier().SetEnableScrollInteraction(nativeNode, enableScrollInteraction);
    } else {
        GetArkUIInternalNodeAPI()->GetListModifier().ResetEnableScrollInteraction(nativeNode);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetEnableScrollInteraction(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetEnableScrollInteraction(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetSticky(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsUndefined()) {
        GetArkUIInternalNodeAPI()->GetListModifier().ResetSticky(nativeNode);
    } else {
        int32_t stickyStyle = secondArg->ToNumber(vm)->Value();
        GetArkUIInternalNodeAPI()->GetListModifier().SetSticky(nativeNode, stickyStyle);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetSticky(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetSticky(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetListEdgeEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);

    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t effect = static_cast<int32_t>(EdgeEffect::SPRING);

    if (secondArg->IsUndefined() || secondArg->IsNull()) {
        effect = static_cast<int32_t>(EdgeEffect::SPRING);
    } else {
        effect = secondArg->Uint32Value(vm);
    }
    if (effect < static_cast<int32_t>(EdgeEffect::SPRING) || effect > static_cast<int32_t>(EdgeEffect::NONE)) {
        effect = static_cast<int32_t>(EdgeEffect::SPRING);
    }
    if (thirdArg->IsUndefined() || thirdArg->IsNull()) {
        GetArkUIInternalNodeAPI()->GetListModifier().SetListEdgeEffect(nativeNode, effect, false);
    } else {
        GetArkUIInternalNodeAPI()->GetListModifier().SetListEdgeEffect(
            nativeNode, effect, thirdArg->ToBoolean(vm)->Value());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetListEdgeEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetListEdgeEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetListDirection(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsUndefined()) {
        GetArkUIInternalNodeAPI()->GetListModifier().ResetListDirection(nativeNode);
    } else {
        int32_t direction = secondArg->ToNumber(vm)->Value();
        GetArkUIInternalNodeAPI()->GetListModifier().SetListDirection(nativeNode, direction);
    }

    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue ListBridge::ResetListDirection(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetListDirection(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetListFriction(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);

    double friction = -1.0;
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsUndefined() || secondArg->IsNull() || !ArkTSUtils::ParseJsDouble(vm, secondArg, friction)) {
        friction = -1.0;
    }
    GetArkUIInternalNodeAPI()->GetListModifier().SetListFriction(nativeNode, friction);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetListFriction(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetListFriction(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetListNestedScroll(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);

    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t forward = 0;
    int32_t backward = 0;
    if (!secondArg->IsUndefined()) {
        forward = secondArg->Int32Value(vm);
    }
    if (!thirdArg->IsUndefined()) {
        backward = thirdArg->Int32Value(vm);
    }

    if (forward < static_cast<int32_t>(NestedScrollMode::SELF_ONLY) ||
        forward > static_cast<int32_t>(NestedScrollMode::PARALLEL)) {
        forward = static_cast<int32_t>(NestedScrollMode::SELF_ONLY);
    }

    if (backward < static_cast<int32_t>(NestedScrollMode::SELF_ONLY) ||
        backward > static_cast<int32_t>(NestedScrollMode::PARALLEL)) {
        backward = static_cast<int32_t>(NestedScrollMode::SELF_ONLY);
    }

    GetArkUIInternalNodeAPI()->GetListModifier().SetListNestedScroll(nativeNode, forward, backward);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetListNestedScroll(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetListNestedScroll(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetListScrollBar(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsUndefined()) {
        GetArkUIInternalNodeAPI()->GetListModifier().ResetListScrollBar(nativeNode);
    } else {
        int32_t barState = secondArg->ToNumber(vm)->Value();
        GetArkUIInternalNodeAPI()->GetListModifier().SetListScrollBar(nativeNode, barState);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetListScrollBar(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetListScrollBar(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetAlignListItem(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    int32_t listItemAlign = secondArg->Int32Value(vm);
    GetArkUIInternalNodeAPI()->GetListModifier().SetAlignListItem(nativeNode, listItemAlign);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetAlignListItem(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetAlignListItem(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetScrollSnapAlign(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsUndefined()) {
        GetArkUIInternalNodeAPI()->GetListModifier().ResetScrollSnapAlign(nativeNode);
    } else {
        int32_t scrollSnapAlign = secondArg->Int32Value(vm);
        GetArkUIInternalNodeAPI()->GetListModifier().SetScrollSnapAlign(nativeNode, scrollSnapAlign);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetScrollSnapAlign(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetAlignListItem(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetDivider(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> fourthArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> fifthArg = runtimeCallInfo->GetCallArgRef(NUM_4);

    CalcDimension strokeWidth;
    Color color;
    CalcDimension startMargin;
    CalcDimension endMargin;
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsNull() || secondArg->IsUndefined() ||
        !ArkTSUtils::ParseJsDimension(vm, secondArg, strokeWidth, DimensionUnit::VP)) {
        strokeWidth = 0.0_vp;
    };
    if (!ArkTSUtils::ParseJsColorAlpha(vm, thirdArg, color)) {
        RefPtr<ListTheme> listTheme = Framework::JSViewAbstract::GetTheme<ListTheme>();
        if (listTheme) {
            color = listTheme->GetDividerColor();
        }
    };
    if (fourthArg->IsNull() || fourthArg->IsUndefined() ||
        !ArkTSUtils::ParseJsDimension(vm, fourthArg, startMargin, DimensionUnit::VP)) {
        startMargin = 0.0_vp;
    };
    if (fifthArg->IsNull() || fifthArg->IsUndefined() ||
        !ArkTSUtils::ParseJsDimension(vm, fifthArg, endMargin, DimensionUnit::VP)) {
        endMargin = 0.0_vp;
    };
    double values[ARG_LENGTH];
    int32_t units[ARG_LENGTH];
    values[NUM_0] = strokeWidth.Value();
    values[NUM_1] = startMargin.Value();
    values[NUM_2] = endMargin.Value();
    units[NUM_0] = static_cast<int32_t>(strokeWidth.Unit());
    units[NUM_1] = static_cast<int32_t>(startMargin.Unit());
    units[NUM_2] = static_cast<int32_t>(endMargin.Unit());
    GetArkUIInternalNodeAPI()->GetListModifier().ListSetDivider(nativeNode, color.GetValue(), values, units);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetDivider(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ListResetDivider(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetChainAnimationOptions(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> fourthArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> fifthArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    Local<JSValueRef> sixthArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    Local<JSValueRef> seventhArg = runtimeCallInfo->GetCallArgRef(NUM_6);
    Local<JSValueRef> eighthArg = runtimeCallInfo->GetCallArgRef(NUM_7);

    CalcDimension minSpace;
    CalcDimension maxSpace;

    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    if (secondArg->IsUndefined() || thirdArg->IsUndefined()) {
        RefPtr<ListTheme> listTheme = Framework::JSViewAbstract::GetTheme<ListTheme>();
        CHECK_NULL_RETURN(listTheme, panda::NativePointerRef::New(vm, nullptr));

        minSpace = listTheme->GetChainMinSpace();
        maxSpace = listTheme->GetChainMaxSpace();
        ArkUIChainAnimationOptionsType chainAnimationOptions;
        chainAnimationOptions.conductivity = listTheme->GetChainConductivity();
        chainAnimationOptions.intensity = listTheme->GetChainIntensity();
        chainAnimationOptions.edgeEffect = 0;
        chainAnimationOptions.stiffness = listTheme->GetChainStiffness();
        chainAnimationOptions.damping = listTheme->GetChainDamping();

        ArkTSUtils::ParseJsDimension(vm, secondArg, minSpace, DimensionUnit::VP);
        ArkTSUtils::ParseJsDimension(vm, thirdArg, maxSpace, DimensionUnit::VP);
        ArkTSUtils::ParseJsDouble(vm, fourthArg, chainAnimationOptions.conductivity);
        ArkTSUtils::ParseJsDouble(vm, fifthArg, chainAnimationOptions.intensity);
        chainAnimationOptions.edgeEffect = sixthArg->ToNumber(vm)->Value();
        ArkTSUtils::ParseJsDouble(vm, seventhArg, chainAnimationOptions.stiffness);
        ArkTSUtils::ParseJsDouble(vm, eighthArg, chainAnimationOptions.damping);
        chainAnimationOptions.minSpace = minSpace.Value();
        chainAnimationOptions.minSpaceUnits = static_cast<int32_t>(minSpace.Unit());
        chainAnimationOptions.maxSpace = maxSpace.Value();
        chainAnimationOptions.maxSpaceUnits = static_cast<int32_t>(maxSpace.Unit());

        GetArkUIInternalNodeAPI()->GetListModifier().SetChainAnimationOptions(nativeNode, &chainAnimationOptions);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetChainAnimationOptions(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetListModifier().ResetChainAnimationOptions(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

} // namespace OHOS::Ace::NG