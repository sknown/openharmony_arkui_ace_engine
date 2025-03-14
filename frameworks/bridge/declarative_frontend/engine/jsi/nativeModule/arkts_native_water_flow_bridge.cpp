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
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_water_flow_bridge.h"

#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"

namespace OHOS::Ace::NG {
namespace {
constexpr double FRICTION_DEFAULT = -1.0;
constexpr double DIMENSION_DEFAULT = 0.0;
constexpr int32_t DISPLAY_MODE_SIZE = 3;
constexpr int32_t NUM_0 = 0;
constexpr int32_t NUM_1 = 1;
constexpr int32_t NUM_2 = 2;
constexpr int32_t NUM_3 = 3;
constexpr int32_t NUM_4 = 4;
const std::vector<FlexDirection> LAYOUT_DIRECTION = { FlexDirection::ROW, FlexDirection::COLUMN,
    FlexDirection::ROW_REVERSE, FlexDirection::COLUMN_REVERSE };

void SetItemConstraintSizeSendParams(CalcDimension& doubleValue, std::string& calcStrValue)
{
    if (doubleValue.Unit() == DimensionUnit::CALC) {
        calcStrValue = doubleValue.CalcValue();
        doubleValue.SetValue(DIMENSION_DEFAULT);
    }
}
} // namespace

ArkUINativeModuleValue WaterFlowBridge::ResetColumnsTemplate(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetColumnsTemplate(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetColumnsTemplate(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> columnsTemplateArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    std::string columnsTemplateValue = columnsTemplateArg->ToString(vm)->ToString();
    GetArkUINodeModifiers()->getWaterFlowModifier()->setColumnsTemplate(nativeNode, columnsTemplateValue.c_str());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetRowsTemplate(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetRowsTemplate(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetRowsTemplate(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> rowsTemplateArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    std::string rowsTemplateValue = rowsTemplateArg->ToString(vm)->ToString();
    GetArkUINodeModifiers()->getWaterFlowModifier()->setRowsTemplate(nativeNode, rowsTemplateValue.c_str());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetEnableScrollInteraction(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> enableScrollInteractionArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    if (enableScrollInteractionArg->IsUndefined() || !enableScrollInteractionArg->IsBoolean()) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->setWaterFlowEnableScrollInteraction(nativeNode, true);
        return panda::JSValueRef::Undefined(vm);
    }
    bool flag = enableScrollInteractionArg->ToBoolean(vm)->Value();
    GetArkUINodeModifiers()->getWaterFlowModifier()->setWaterFlowEnableScrollInteraction(nativeNode, flag);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetEnableScrollInteraction(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetWaterFlowEnableScrollInteraction(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetColumnsGap(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetColumnsGap(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetColumnsGap(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> columnsGapArg = runtimeCallInfo->GetCallArgRef(NUM_1);

    CalcDimension columnsGap;
    std::string calcStr;
    if (columnsGapArg->IsUndefined() || !ArkTSUtils::ParseJsDimensionVpNG(vm, columnsGapArg, columnsGap)) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->resetColumnsGap(nativeNode);
    } else {
        if (LessNotEqual(columnsGap.Value(), DIMENSION_DEFAULT)) {
            columnsGap.SetValue(DIMENSION_DEFAULT);
        }

        if (columnsGap.Unit() == DimensionUnit::CALC) {
            GetArkUINodeModifiers()->getWaterFlowModifier()->setColumnsGap(
                nativeNode, NUM_0, static_cast<int32_t>(columnsGap.Unit()), columnsGap.CalcValue().c_str());
        } else {
            GetArkUINodeModifiers()->getWaterFlowModifier()->setColumnsGap(
                nativeNode, columnsGap.Value(), static_cast<int32_t>(columnsGap.Unit()), calcStr.c_str());
        }
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetRowsGap(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetRowsGap(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetRowsGap(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> rowsGapArg = runtimeCallInfo->GetCallArgRef(NUM_1);

    CalcDimension rowGap;
    std::string calcStr;
    if (rowsGapArg->IsUndefined() || !ArkTSUtils::ParseJsDimensionVpNG(vm, rowsGapArg, rowGap)) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->resetRowsGap(nativeNode);
    } else {
        if (LessNotEqual(rowGap.Value(), DIMENSION_DEFAULT)) {
            rowGap.SetValue(DIMENSION_DEFAULT);
        }
        if (rowGap.Unit() == DimensionUnit::CALC) {
            GetArkUINodeModifiers()->getWaterFlowModifier()->setRowsGap(
                nativeNode, 0, static_cast<int32_t>(rowGap.Unit()), rowGap.CalcValue().c_str());
        } else {
            GetArkUINodeModifiers()->getWaterFlowModifier()->setRowsGap(
                nativeNode, rowGap.Value(), static_cast<int32_t>(rowGap.Unit()), calcStr.c_str());
        }
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetItemConstraintSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> minWidthValue = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> maxWidthValue = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> minHeightValue = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> maxHeightValue = runtimeCallInfo->GetCallArgRef(NUM_4);
    CalcDimension minWidth;
    CalcDimension maxWidth;
    CalcDimension minHeight;
    CalcDimension maxHeight;
    std::string calcMinWidthStr;
    std::string calcMaxWidthStr;
    std::string calcMinHeightStr;
    std::string calcMaxHeightStr;
    if (minWidthValue->IsUndefined() || !ArkTSUtils::ParseJsDimensionVp(vm, minWidthValue, minWidth, false)) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->resetItemMinWidth(nativeNode);
    } else {
        SetItemConstraintSizeSendParams(minWidth, calcMinWidthStr);
        GetArkUINodeModifiers()->getWaterFlowModifier()->setItemMinWidth(
            nativeNode, minWidth.Value(), static_cast<int32_t>(minWidth.Unit()), calcMinWidthStr.c_str());
    }
    if (maxWidthValue->IsUndefined() || !ArkTSUtils::ParseJsDimensionVp(vm, maxWidthValue, maxWidth, false)) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->resetItemMaxWidth(nativeNode);
    } else {
        SetItemConstraintSizeSendParams(maxWidth, calcMaxWidthStr);
        GetArkUINodeModifiers()->getWaterFlowModifier()->setItemMaxWidth(
            nativeNode, maxWidth.Value(), static_cast<int32_t>(maxWidth.Unit()), calcMaxWidthStr.c_str());
    }
    if (minHeightValue->IsUndefined() || !ArkTSUtils::ParseJsDimensionVp(vm, minHeightValue, minHeight, false)) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->resetItemMinHeight(nativeNode);
    } else {
        SetItemConstraintSizeSendParams(minHeight, calcMinHeightStr);
        GetArkUINodeModifiers()->getWaterFlowModifier()->setItemMinHeight(
            nativeNode, minHeight.Value(), static_cast<int32_t>(minHeight.Unit()), calcMinHeightStr.c_str());
    }
    if (maxHeightValue->IsUndefined() || !ArkTSUtils::ParseJsDimensionVp(vm, maxHeightValue, maxHeight, false)) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->resetItemMaxHeight(nativeNode);
    } else {
        SetItemConstraintSizeSendParams(maxHeight, calcMaxHeightStr);
        GetArkUINodeModifiers()->getWaterFlowModifier()->setItemMaxHeight(
            nativeNode, maxHeight.Value(), static_cast<int32_t>(maxHeight.Unit()), calcMaxHeightStr.c_str());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetItemConstraintSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetItemMaxHeight(nativeNode);
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetItemMaxWidth(nativeNode);
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetItemMinHeight(nativeNode);
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetItemMinWidth(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetLayoutDirection(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> layoutDirectionArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    auto value = static_cast<int32_t>(FlexDirection::COLUMN);
    if (!layoutDirectionArg->IsUndefined()) {
        ArkTSUtils::ParseJsInteger(vm, layoutDirectionArg, value);
    }
    if (value >= NUM_0 && value < static_cast<int32_t>(LAYOUT_DIRECTION.size())) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->setLayoutDirection(nativeNode, value);
    } else {
        GetArkUINodeModifiers()->getWaterFlowModifier()->resetLayoutDirection(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetLayoutDirection(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetLayoutDirection(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetNestedScroll(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> scrollForwardValue = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> scrollBackwardValue = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    int32_t froward = NUM_0;
    int32_t backward = NUM_0;
    ArkTSUtils::ParseJsInteger(vm, scrollForwardValue, froward);
    if (froward < static_cast<int32_t>(NestedScrollMode::SELF_ONLY) ||
        froward > static_cast<int32_t>(NestedScrollMode::PARALLEL)) {
        froward = NUM_0;
    }
    ArkTSUtils::ParseJsInteger(vm, scrollBackwardValue, backward);
    if (backward < static_cast<int32_t>(NestedScrollMode::SELF_ONLY) ||
        backward > static_cast<int32_t>(NestedScrollMode::PARALLEL)) {
        backward = NUM_0;
    }
    GetArkUINodeModifiers()->getWaterFlowModifier()->setWaterFlowNestedScroll(nativeNode, froward, backward);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetNestedScroll(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetWaterFlowNestedScroll(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetFriction(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> frictionArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    double friction = FRICTION_DEFAULT;
    if (!ArkTSUtils::ParseJsDouble(vm, frictionArg, friction)) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->resetWaterFlowFriction(nativeNode);
    } else {
        GetArkUINodeModifiers()->getWaterFlowModifier()->setWaterFlowFriction(nativeNode,
            static_cast<ArkUI_Float32>(friction));
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetFriction(ArkUIRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM *vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetWaterFlowFriction(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetEdgeEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> argNode = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> argEdgeEffect = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> argEdgeEffectOptions = runtimeCallInfo->GetCallArgRef(NUM_2);

    auto nativeNode = nodePtr(argNode->ToNativePointer(vm)->Value());
    int32_t effect = static_cast<int32_t>(EdgeEffect::NONE);
    if (!argEdgeEffect->IsUndefined() && !argEdgeEffect->IsNull()) {
        effect = argEdgeEffect->Int32Value(vm);
    }

    if (effect != static_cast<int32_t>(EdgeEffect::SPRING) && effect != static_cast<int32_t>(EdgeEffect::NONE) &&
        effect != static_cast<int32_t>(EdgeEffect::FADE)) {
        effect = static_cast<int32_t>(EdgeEffect::NONE);
    }

    if (argEdgeEffectOptions->IsNull() || argEdgeEffectOptions->IsUndefined()) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->setEdgeEffect(nativeNode, effect, false);
    } else {
        GetArkUINodeModifiers()->getWaterFlowModifier()->setEdgeEffect(
            nativeNode, effect, argEdgeEffectOptions->ToBoolean(vm)->Value());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetEdgeEffect(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> node = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(node->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetEdgeEffect(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetScrollBar(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    int32_t barState = 0;
    if (!ArkTSUtils::ParseJsInteger(vm, secondArg, barState) || barState < 0 || barState >= DISPLAY_MODE_SIZE) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->resetWaterFlowScrollBar(nativeNode);
    } else {
        GetArkUINodeModifiers()->getWaterFlowModifier()->setWaterFlowScrollBar(nativeNode, barState);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetScrollBar(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> argNode = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(argNode->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetWaterFlowScrollBar(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetScrollBarWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> scrollBarArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());

    CalcDimension scrollBarWidth;
    if (!ArkTSUtils::ParseJsDimension(vm, scrollBarArg, scrollBarWidth, DimensionUnit::VP) || scrollBarArg->IsNull() ||
        scrollBarArg->IsUndefined() || (scrollBarArg->IsString(vm) && scrollBarWidth.ToString().empty()) ||
        LessNotEqual(scrollBarWidth.Value(), 0.0) || scrollBarWidth.Unit() == DimensionUnit::PERCENT) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->resetWaterFlowScrollBarWidth(nativeNode);
    } else {
        GetArkUINodeModifiers()->getWaterFlowModifier()->setWaterFlowScrollBarWidth(
            nativeNode, scrollBarWidth.ToString().c_str());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetScrollBarWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> argNode = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(argNode->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetWaterFlowScrollBarWidth(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetScrollBarColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> argNode = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> argColor = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(argNode->ToNativePointer(vm)->Value());
    std::string color = "";
    if (!ArkTSUtils::ParseJsString(vm, argColor, color) || argColor->IsUndefined() || color.empty()) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->resetWaterFlowScrollBarColor(nativeNode);
    } else {
        GetArkUINodeModifiers()->getWaterFlowModifier()->setWaterFlowScrollBarColor(nativeNode, color.c_str());
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetScrollBarColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> argNode = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(argNode->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetWaterFlowScrollBarColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetCachedCount(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> argNode = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> argCachedCount = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(argNode->ToNativePointer(vm)->Value());
    int32_t cachedCount = 0;
    if (!ArkTSUtils::ParseJsInteger(vm, argCachedCount, cachedCount) || cachedCount < 0) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->resetCachedCount(nativeNode);
    } else {
        GetArkUINodeModifiers()->getWaterFlowModifier()->setCachedCount(nativeNode, cachedCount);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetCachedCount(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> argNode = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(argNode->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetCachedCount(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::SetFlingSpeedLimit(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> argNode = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> argSpeed = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(argNode->ToNativePointer(vm)->Value());
    double limitSpeed = -1.0;
    if (!ArkTSUtils::ParseJsDouble(vm, argSpeed, limitSpeed)) {
        GetArkUINodeModifiers()->getWaterFlowModifier()->resetWaterFlowFlingSpeedLimit(nativeNode);
    } else {
        GetArkUINodeModifiers()->getWaterFlowModifier()->setWaterFlowFlingSpeedLimit(
            nativeNode, static_cast<ArkUI_Float32>(limitSpeed));
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue WaterFlowBridge::ResetFlingSpeedLimit(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> argNode = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(argNode->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getWaterFlowModifier()->resetWaterFlowFlingSpeedLimit(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
} // namespace OHOS::Ace::NG