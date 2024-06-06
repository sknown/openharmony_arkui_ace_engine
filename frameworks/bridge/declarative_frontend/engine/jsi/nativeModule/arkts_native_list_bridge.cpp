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

#include "core/interfaces/native/node/node_api.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_list.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "frameworks/core/components/list/list_theme.h"
using namespace OHOS::Ace::Framework;

namespace OHOS::Ace::NG {
constexpr int32_t LIST_ARG_INDEX_0 = 0;
constexpr int32_t LIST_ARG_INDEX_1 = 1;
constexpr int32_t LIST_ARG_INDEX_2 = 2;
constexpr int32_t LIST_ARG_INDEX_3 = 3;
constexpr int32_t LIST_ARG_INDEX_4 = 4;
constexpr int32_t LIST_ARG_INDEX_5 = 5;
constexpr int32_t LIST_ARG_INDEX_6 = 6;
constexpr int32_t LIST_ARG_INDEX_7 = 7;

constexpr int32_t ARG_LENGTH = 3;

ArkUINativeModuleValue ListBridge::SetListLanes(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0); // 0: index of parameter frameNode
    Local<JSValueRef> laneNumArg = runtimeCallInfo->GetCallArgRef(1);   // 1: index of parameter laneNum
    Local<JSValueRef> minLengthArg = runtimeCallInfo->GetCallArgRef(2); // 2: index of parameter minLength
    Local<JSValueRef> maxLengthArg = runtimeCallInfo->GetCallArgRef(3); // 3: index of parameter maxLength
    Local<JSValueRef> gutterArg = runtimeCallInfo->GetCallArgRef(4);    // 4: index of parameter gutter
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    ArkUIDimensionType gutterType;
    ArkUIDimensionType minLengthType;
    ArkUIDimensionType maxLengthType;

    CalcDimension gutter = Dimension(0.0);
    int32_t laneNum = 1;
    CalcDimension minLength = -1.0_vp;
    CalcDimension maxLength = -1.0_vp;
    if (!gutterArg->IsUndefined() && ArkTSUtils::ParseJsDimensionVp(vm, gutterArg, gutter)) {
        if (gutter.IsNegative()) {
            gutter.Reset();
        }
        gutterType.value = gutter.Value();
        gutterType.units = static_cast<int32_t>(gutter.Unit());
    }
    if (!laneNumArg->IsUndefined() && ArkTSUtils::ParseJsInteger(vm, laneNumArg, laneNum)) {
        minLengthType.value = minLength.Value();
        minLengthType.units = static_cast<int32_t>(minLength.Unit());
        maxLengthType.value = maxLength.Value();
        maxLengthType.units = static_cast<int32_t>(maxLength.Unit());
    }
    if (!minLengthArg->IsUndefined() && !maxLengthArg->IsUndefined() &&
        ArkTSUtils::ParseJsDimensionVp(vm, minLengthArg, minLength) &&
        ArkTSUtils::ParseJsDimensionVp(vm, maxLengthArg, maxLength)) {
        laneNum = -1;
        minLengthType.value = minLength.Value();
        minLengthType.units = static_cast<int32_t>(minLength.Unit());
        maxLengthType.value = maxLength.Value();
        maxLengthType.units = static_cast<int32_t>(maxLength.Unit());
    }
    GetArkUINodeModifiers()->getListModifier()->setListLanes(
        nativeNode, laneNum, &minLengthType, &maxLengthType, &gutterType);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetListLanes(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetListLanes(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue ListBridge::SetEditMode(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsUndefined()) {
        GetArkUINodeModifiers()->getListModifier()->resetEditMode(nativeNode);
    } else {
        bool editMode = secondArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getListModifier()->setEditMode(nativeNode, editMode);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetEditMode(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetEditMode(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetMultiSelectable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsUndefined()) {
        GetArkUINodeModifiers()->getListModifier()->resetMultiSelectable(nativeNode);
    } else {
        bool selectable = secondArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getListModifier()->setMultiSelectable(nativeNode, selectable);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetMultiSelectable(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetMultiSelectable(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetChainAnimation(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    bool chainAnimation = secondArg->ToBoolean(vm)->Value();
    GetArkUINodeModifiers()->getListModifier()->setChainAnimation(nativeNode, chainAnimation);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetChainAnimation(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetChainAnimation(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetCachedCount(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    int32_t cachedCount = secondArg->Int32Value(vm);

    GetArkUINodeModifiers()->getListModifier()->setCachedCount(nativeNode, cachedCount);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetCachedCount(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetCachedCount(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetEnableScrollInteraction(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsBoolean()) {
        bool enableScrollInteraction = secondArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getListModifier()->setEnableScrollInteraction(nativeNode, enableScrollInteraction);
    } else {
        GetArkUINodeModifiers()->getListModifier()->resetEnableScrollInteraction(nativeNode);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetEnableScrollInteraction(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetEnableScrollInteraction(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetSticky(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsUndefined()) {
        GetArkUINodeModifiers()->getListModifier()->resetSticky(nativeNode);
    } else {
        int32_t stickyStyle = secondArg->ToNumber(vm)->Value();
        GetArkUINodeModifiers()->getListModifier()->setSticky(nativeNode, stickyStyle);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetSticky(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetSticky(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetListEdgeEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_2);

    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    int32_t effect = static_cast<int32_t>(EdgeEffect::SPRING);

    if (secondArg->IsUndefined() || secondArg->IsNull()) {
        effect = static_cast<int32_t>(EdgeEffect::SPRING);
    } else {
        effect = secondArg->Int32Value(vm);
    }
    if (effect < static_cast<int32_t>(EdgeEffect::SPRING) || effect > static_cast<int32_t>(EdgeEffect::NONE)) {
        effect = static_cast<int32_t>(EdgeEffect::SPRING);
    }
    if (thirdArg->IsUndefined() || thirdArg->IsNull()) {
        GetArkUINodeModifiers()->getListModifier()->setListEdgeEffect(nativeNode, effect, false);
    } else {
        GetArkUINodeModifiers()->getListModifier()->setListEdgeEffect(
            nativeNode, effect, thirdArg->ToBoolean(vm)->Value());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetListEdgeEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetListEdgeEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetListDirection(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsUndefined()) {
        GetArkUINodeModifiers()->getListModifier()->resetListDirection(nativeNode);
    } else {
        int32_t direction = secondArg->ToNumber(vm)->Value();
        GetArkUINodeModifiers()->getListModifier()->setListDirection(nativeNode, direction);
    }

    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue ListBridge::ResetListDirection(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetListDirection(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetListFriction(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);

    double friction = -1.0;
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsUndefined() || secondArg->IsNull() || !ArkTSUtils::ParseJsDouble(vm, secondArg, friction)) {
        friction = -1.0;
    }
    GetArkUINodeModifiers()->getListModifier()->setListFriction(nativeNode, friction);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetListFriction(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetListFriction(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetListNestedScroll(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_2);

    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
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

    GetArkUINodeModifiers()->getListModifier()->setListNestedScroll(nativeNode, forward, backward);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetListNestedScroll(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetListNestedScroll(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetListScrollBar(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsUndefined()) {
        GetArkUINodeModifiers()->getListModifier()->resetListScrollBar(nativeNode);
    } else {
        int32_t barState = secondArg->ToNumber(vm)->Value();
        GetArkUINodeModifiers()->getListModifier()->setListScrollBar(nativeNode, barState);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetListScrollBar(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetListScrollBar(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetAlignListItem(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    int32_t listItemAlign = secondArg->Int32Value(vm);
    GetArkUINodeModifiers()->getListModifier()->setAlignListItem(nativeNode, listItemAlign);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetAlignListItem(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetAlignListItem(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetScrollSnapAlign(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsUndefined()) {
        GetArkUINodeModifiers()->getListModifier()->resetScrollSnapAlign(nativeNode);
    } else {
        int32_t scrollSnapAlign = secondArg->Int32Value(vm);
        GetArkUINodeModifiers()->getListModifier()->setScrollSnapAlign(nativeNode, scrollSnapAlign);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetScrollSnapAlign(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetAlignListItem(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetContentStartOffset(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> startOffsetArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    double startOffset = 0.0;
    ArkTSUtils::ParseJsDouble(vm, startOffsetArg, startOffset);

    GetArkUINodeModifiers()->getListModifier()->setContentStartOffset(nativeNode, startOffset);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetContentStartOffset(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetContentStartOffset(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetContentEndOffset(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> endOffsetArg = runtimeCallInfo->GetCallArgRef(1);
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    double endOffset = 0.0;
    ArkTSUtils::ParseJsDouble(vm, endOffsetArg, endOffset);

    GetArkUINodeModifiers()->getListModifier()->setContentEndOffset(nativeNode, endOffset);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetContentEndOffset(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> frameNodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(frameNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetContentEndOffset(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetDivider(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> dividerStrokeWidthArgs = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    Local<JSValueRef> colorArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_2);
    Local<JSValueRef> dividerStartMarginArgs = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_3);
    Local<JSValueRef> dividerEndMarginArgs = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_4);
    if (dividerStrokeWidthArgs->IsUndefined() && dividerStartMarginArgs->IsUndefined() &&
        dividerEndMarginArgs->IsUndefined() && colorArg->IsUndefined()) {
        GetArkUINodeModifiers()->getListModifier()->listResetDivider(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    CalcDimension dividerStrokeWidth;
    CalcDimension dividerStartMargin;
    CalcDimension dividerEndMargin;
    uint32_t color;
    auto* frameNode = reinterpret_cast<FrameNode*>(nativeNode);
    auto context = frameNode->GetContext();
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_RETURN(themeManager, panda::NativePointerRef::New(vm, nullptr));
    auto listTheme = themeManager->GetTheme<ListTheme>();
    CHECK_NULL_RETURN(listTheme, panda::NativePointerRef::New(vm, nullptr));

    if (!ArkTSUtils::ParseJsDimensionVpNG(vm, dividerStrokeWidthArgs, dividerStrokeWidth) ||
        LessNotEqual(dividerStrokeWidth.Value(), 0.0f) || dividerStrokeWidth.Unit() == DimensionUnit::PERCENT) {
        dividerStrokeWidth.Reset();
    }
    Color colorObj;
    if (!ArkTSUtils::ParseJsColorAlpha(vm, colorArg, colorObj)) {
        color = listTheme->GetDividerColor().GetValue();
    } else {
        color = colorObj.GetValue();
    }
    if (!ArkTSUtils::ParseJsDimensionVp(vm, dividerStartMarginArgs, dividerStartMargin) ||
        LessNotEqual(dividerStartMargin.Value(), 0.0f) || dividerStartMargin.Unit() == DimensionUnit::PERCENT) {
        dividerStartMargin.Reset();
    }
    if (!ArkTSUtils::ParseJsDimensionVp(vm, dividerEndMarginArgs, dividerEndMargin) ||
        LessNotEqual(dividerEndMargin.Value(), 0.0f) || dividerEndMargin.Unit() == DimensionUnit::PERCENT) {
        dividerEndMargin.Reset();
    }
    uint32_t size = ARG_LENGTH;
    ArkUI_Float32 values[size];
    int32_t units[size];
    values[LIST_ARG_INDEX_0] = static_cast<ArkUI_Float32>(dividerStrokeWidth.Value());
    values[LIST_ARG_INDEX_1] = static_cast<ArkUI_Float32>(dividerStartMargin.Value());
    values[LIST_ARG_INDEX_2] = static_cast<ArkUI_Float32>(dividerEndMargin.Value());
    units[LIST_ARG_INDEX_0] = static_cast<int32_t>(dividerStrokeWidth.Unit());
    units[LIST_ARG_INDEX_1] = static_cast<int32_t>(dividerStartMargin.Unit());
    units[LIST_ARG_INDEX_2] = static_cast<int32_t>(dividerEndMargin.Unit());
    GetArkUINodeModifiers()->getListModifier()->listSetDivider(nativeNode, color, values, units, size);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetDivider(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->listResetDivider(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetChainAnimationOptions(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> minSpaceArgs = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    Local<JSValueRef> maxSpaceArgs = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_2);
    Local<JSValueRef> conductivityArgs = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_3);
    Local<JSValueRef> intensityArgs = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_4);
    Local<JSValueRef> edgeEffectArgs = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_5);
    Local<JSValueRef> stiffnessArgs = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_6);
    Local<JSValueRef> dampingArgs = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_7);

    CalcDimension minSpace;
    CalcDimension maxSpace;

    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (minSpaceArgs->IsUndefined() && maxSpaceArgs->IsUndefined() && conductivityArgs->IsUndefined() &&
        intensityArgs->IsUndefined() && edgeEffectArgs->IsUndefined() && stiffnessArgs->IsUndefined() &&
        dampingArgs->IsUndefined()) {
        GetArkUINodeModifiers()->getListModifier()->resetChainAnimationOptions(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    } else {
        RefPtr<ListTheme> listTheme = ArkTSUtils::GetTheme<ListTheme>();
        CHECK_NULL_RETURN(listTheme, panda::NativePointerRef::New(vm, nullptr));

        minSpace = listTheme->GetChainMinSpace();
        maxSpace = listTheme->GetChainMaxSpace();
        ArkUIChainAnimationOptionsType chainAnimationOptions;

        double conductivity;
        double intensity;

        chainAnimationOptions.conductivity = listTheme->GetChainConductivity();
        chainAnimationOptions.intensity = listTheme->GetChainIntensity();
        chainAnimationOptions.edgeEffect = 0;
        chainAnimationOptions.stiffness = listTheme->GetChainStiffness();
        chainAnimationOptions.damping = listTheme->GetChainDamping();

        ArkTSUtils::ParseJsDimension(vm, minSpaceArgs, minSpace, DimensionUnit::VP);
        ArkTSUtils::ParseJsDimension(vm, maxSpaceArgs, maxSpace, DimensionUnit::VP);
        ArkTSUtils::ParseJsDouble(vm, conductivityArgs, conductivity);
        chainAnimationOptions.conductivity = static_cast<ArkUI_Float32>(conductivity);

        ArkTSUtils::ParseJsDouble(vm, intensityArgs, intensity);
        chainAnimationOptions.intensity = static_cast<ArkUI_Float32>(intensity);

        if (edgeEffectArgs->IsNumber()) {
            chainAnimationOptions.edgeEffect = edgeEffectArgs->ToNumber(vm)->Value();
        }
        
        double stiffness;
        double damping;

        ArkTSUtils::ParseJsDouble(vm, stiffnessArgs, stiffness);
        chainAnimationOptions.stiffness = static_cast<ArkUI_Float32>(stiffness);

        ArkTSUtils::ParseJsDouble(vm, dampingArgs, damping);
        chainAnimationOptions.damping = static_cast<ArkUI_Float32>(damping);

        chainAnimationOptions.minSpace = minSpace.Value();
        chainAnimationOptions.minSpaceUnits = static_cast<int32_t>(minSpace.Unit());
        chainAnimationOptions.maxSpace = maxSpace.Value();
        chainAnimationOptions.maxSpaceUnits = static_cast<int32_t>(maxSpace.Unit());

        GetArkUINodeModifiers()->getListModifier()->setChainAnimationOptions(nativeNode, &chainAnimationOptions);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetChainAnimationOptions(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetChainAnimationOptions(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetFadingEdge(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    if (secondArg->IsUndefined()) {
        GetArkUINodeModifiers()->getListModifier()->resetFadingEdge(nativeNode);
    } else {
        bool fadingEdge = secondArg->ToBoolean(vm)->Value();
        GetArkUINodeModifiers()->getListModifier()->setFadingEdge(nativeNode, fadingEdge);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetFadingEdge(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetFadingEdge(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetListChildrenMainSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    // 2: argument count.
    if (info.Length() != 2 || !(info[1]->IsObject())) {
        return panda::JSValueRef::Undefined(vm);
    }
    JSList::SetChildrenMainSize(Framework::JSRef<Framework::JSObject>::Cast(info[1]));

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetListChildrenMainSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(LIST_ARG_INDEX_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetListChildrenMainSize(nativeNode);

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetInitialIndex(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    int32_t index = secondArg->ToNumber(vm)->Value();
    GetArkUINodeModifiers()->getListModifier()->setInitialIndex(nativeNode, index);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetInitialIndex(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetInitialIndex(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::SetSpace(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    float space = secondArg->ToNumber(vm)->Value();
    GetArkUINodeModifiers()->getListModifier()->setListSpace(nativeNode, space);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ListBridge::ResetSpace(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getListModifier()->resetListSpace(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

} // namespace OHOS::Ace::NG