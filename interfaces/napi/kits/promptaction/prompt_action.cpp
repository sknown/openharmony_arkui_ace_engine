/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "prompt_action.h"

#include <cstddef>
#include <memory>
#include <string>

#include "interfaces/napi/kits/utils/napi_utils.h"
#include "base/i18n/localization.h"
#include "base/log/log_wrapper.h"
#include "base/subwindow/subwindow_manager.h"
#include "base/utils/system_properties.h"
#include "bridge/common/utils/engine_helper.h"
#include "core/common/ace_engine.h"
#include "core/components/common/properties/shadow.h"
#include "core/components/theme/shadow_theme.h"
#include "core/components_ng/pattern/toast/toast_layout_property.h"

namespace OHOS::Ace::Napi {
namespace {
const int32_t SHOW_DIALOG_BUTTON_NUM_MAX = -1;
const int32_t SHOW_ACTION_MENU_BUTTON_NUM_MAX = 6;
const int32_t CUSTOM_DIALOG_PARAM_NUM = 2;
const int32_t BG_BLUR_STYLE_MAX_INDEX = 12;
const int32_t PROMPTACTION_VALID_PRIMARY_BUTTON_NUM = 1;
constexpr char DEFAULT_FONT_COLOR_STRING_VALUE[] = "#ff007dff";
const std::vector<DialogAlignment> DIALOG_ALIGNMENT = { DialogAlignment::TOP, DialogAlignment::CENTER,
    DialogAlignment::BOTTOM, DialogAlignment::DEFAULT, DialogAlignment::TOP_START, DialogAlignment::TOP_END,
    DialogAlignment::CENTER_START, DialogAlignment::CENTER_END, DialogAlignment::BOTTOM_START,
    DialogAlignment::BOTTOM_END };

#ifdef OHOS_STANDARD_SYSTEM
bool ContainerIsService()
{
    auto containerId = Container::CurrentIdSafely();
    // Get active container when current instanceid is less than 0
    if (containerId < 0) {
        auto container = Container::GetActive();
        if (container) {
            containerId = container->GetInstanceId();
        }
    }
    // for pa service
    return containerId >= MIN_PA_SERVICE_ID || containerId < 0;
}

bool ContainerIsScenceBoard()
{
    auto container = Container::CurrentSafely();
    if (!container) {
        container = Container::GetActive();
    }

    return container && container->IsScenceBoardWindow();
}
#endif
} // namespace

bool HasProperty(napi_env env, napi_value value, const std::string& targetStr)
{
    bool hasProperty = false;
    napi_has_named_property(env, value, targetStr.c_str(), &hasProperty);
    return hasProperty;
}

bool ParseNapiDimension(napi_env env, CalcDimension& result, napi_value napiValue, DimensionUnit defaultUnit)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiValue, &valueType);
    if (valueType == napi_number) {
        double value = 0;
        napi_get_value_double(env, napiValue, &value);
        result.SetUnit(defaultUnit);
        result.SetValue(value);
        return true;
    } else if (valueType == napi_string) {
        std::string valueString;
        if (!GetNapiString(env, napiValue, valueString, valueType)) {
            return false;
        }
        result = StringUtils::StringToCalcDimension(valueString, false, defaultUnit);
        return true;
    } else if (valueType == napi_object) {
        ResourceInfo recv;
        std::string parameterStr;
        if (!ParseResourceParam(env, napiValue, recv)) {
            return false;
        }
        if (!ParseString(recv, parameterStr)) {
            return false;
        }
        result = StringUtils::StringToDimensionWithUnit(parameterStr, defaultUnit);
        return true;
    }
    return false;
}

napi_value GetReturnObject(napi_env env, std::string callbackString)
{
    napi_value result = nullptr;
    napi_value returnObj = nullptr;
    napi_create_object(env, &returnObj);
    napi_create_string_utf8(env, callbackString.c_str(), NAPI_AUTO_LENGTH, &result);
    napi_set_named_property(env, returnObj, "errMsg", result);
    return returnObj;
}

napi_value JSPromptShowToast(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AceLogTag::ACE_DIALOG, "show toast enter");
    size_t requireArgc = 1;
    size_t argc = 1;
    napi_value argv = nullptr;
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, &argv, &thisVar, &data);
    if (argc != requireArgc) {
        NapiThrow(env, "The number of parameters must be equal to 1.", ERROR_CODE_PARAM_INVALID);
        return nullptr;
    }
    napi_value messageNApi = nullptr;
    napi_value durationNApi = nullptr;
    napi_value bottomNApi = nullptr;
    napi_value showModeNApi = nullptr;
    napi_value alignmentApi = nullptr;
    napi_value offsetApi = nullptr;
    std::string messageString;
    std::string bottomString;
    NG::ToastShowMode showMode = NG::ToastShowMode::DEFAULT;
    int32_t alignment = -1;
    std::optional<DimensionOffset> offset;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv, &valueType);
    if (valueType == napi_object) {
        // message can not be null
        if (!HasProperty(env, argv, "message")) {
            NapiThrow(env, "Required input parameters are missing.", ERROR_CODE_PARAM_INVALID);
            return nullptr;
        }
        napi_get_named_property(env, argv, "message", &messageNApi);
        napi_get_named_property(env, argv, "duration", &durationNApi);
        napi_get_named_property(env, argv, "bottom", &bottomNApi);
        napi_get_named_property(env, argv, "showMode", &showModeNApi);
        napi_get_named_property(env, argv, "alignment", &alignmentApi);
        napi_get_named_property(env, argv, "offset", &offsetApi);
    } else {
        NapiThrow(env, "The type of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
        return nullptr;
    }

    size_t ret = 0;
    ResourceInfo recv;
    napi_typeof(env, messageNApi, &valueType);
    if (valueType == napi_string) {
        size_t messageLen = GetParamLen(env, messageNApi) + 1;
        std::unique_ptr<char[]> message = std::make_unique<char[]>(messageLen);
        napi_get_value_string_utf8(env, messageNApi, message.get(), messageLen, &ret);
        messageString = message.get();
    } else if (valueType == napi_object) {
        if (!ParseResourceParam(env, messageNApi, recv)) {
            NapiThrow(env, "Can not parse resource info from input params.", ERROR_CODE_INTERNAL_ERROR);
            return nullptr;
        }
        if (!ParseString(recv, messageString)) {
            NapiThrow(env, "Can not get message from resource manager.", ERROR_CODE_INTERNAL_ERROR);
            return nullptr;
        }
        TAG_LOGD(AceLogTag::ACE_DIALOG, "Toast message: %{public}s", messageString.c_str());
    } else {
        NapiThrow(env, "The type of message is incorrect.", ERROR_CODE_PARAM_INVALID);
        return nullptr;
    }

    int32_t duration = -1;
    std::string durationStr;
    napi_typeof(env, durationNApi, &valueType);
    if (valueType == napi_number) {
        napi_get_value_int32(env, durationNApi, &duration);
    } else if (valueType == napi_object) {
        recv = {};
        if (!ParseResourceParam(env, durationNApi, recv)) {
            NapiThrow(env, "Can not parse resource info from input params.", ERROR_CODE_INTERNAL_ERROR);
            return nullptr;
        }
        if (!ParseString(recv, durationStr)) {
            NapiThrow(env, "Can not get message from resource manager.", ERROR_CODE_INTERNAL_ERROR);
            return nullptr;
        }
        duration = StringUtils::StringToInt(durationStr);
    }

    napi_typeof(env, bottomNApi, &valueType);
    if (valueType == napi_string) {
        size_t bottomLen = GetParamLen(env, bottomNApi) + 1;
        std::unique_ptr<char[]> bottom = std::make_unique<char[]>(bottomLen);
        napi_get_value_string_utf8(env, bottomNApi, bottom.get(), bottomLen, &ret);
        bottomString = bottom.get();
    } else if (valueType == napi_number) {
        double bottom = 0.0;
        napi_get_value_double(env, bottomNApi, &bottom);
        bottomString = std::to_string(bottom);
    } else if (valueType == napi_object) {
        recv = {};
        if (!ParseResourceParam(env, bottomNApi, recv)) {
            NapiThrow(env, "Can not parse resource info from input params.", ERROR_CODE_INTERNAL_ERROR);
            return nullptr;
        }
        if (!ParseString(recv, bottomString)) {
            NapiThrow(env, "Can not get message from resource manager.", ERROR_CODE_INTERNAL_ERROR);
            return nullptr;
        }
    }

    napi_typeof(env, showModeNApi, &valueType);
    if (valueType == napi_number) {
        int32_t num = -1;
        napi_get_value_int32(env, showModeNApi, &num);
        if (num >= 0 && num <= static_cast<int32_t>(NG::ToastShowMode::SYSTEM_TOP_MOST)) {
            showMode = static_cast<NG::ToastShowMode>(num);
        }
    }

    // parse alignment
    napi_typeof(env, alignmentApi, &valueType);
    if (valueType == napi_number) {
        napi_get_value_int32(env, alignmentApi, &alignment);
    }

    // parse offset
    napi_typeof(env, offsetApi, &valueType);
    if (valueType == napi_object) {
        napi_value dxApi = nullptr;
        napi_value dyApi = nullptr;
        napi_get_named_property(env, offsetApi, "dx", &dxApi);
        napi_get_named_property(env, offsetApi, "dy", &dyApi);
        CalcDimension dx;
        CalcDimension dy;
        ParseNapiDimension(env, dx, dxApi, DimensionUnit::VP);
        ParseNapiDimension(env, dy, dyApi, DimensionUnit::VP);
        offset = DimensionOffset { dx, dy };
    }
#ifdef OHOS_STANDARD_SYSTEM
    if ((SystemProperties::GetExtSurfaceEnabled() || !ContainerIsService()) && !ContainerIsScenceBoard() &&
        showMode == NG::ToastShowMode::DEFAULT) {
        auto delegate = EngineHelper::GetCurrentDelegateSafely();
        if (!delegate) {
            NapiThrow(env, "Can not get delegate.", ERROR_CODE_INTERNAL_ERROR);
            return nullptr;
        }
        TAG_LOGD(AceLogTag::ACE_DIALOG, "before delegate show toast");
        delegate->ShowToast(messageString, duration, bottomString, showMode, alignment, offset);
    } else if (SubwindowManager::GetInstance() != nullptr) {
        TAG_LOGD(AceLogTag::ACE_DIALOG, "before subwindow manager show toast");
        SubwindowManager::GetInstance()->ShowToast(messageString, duration, bottomString, showMode, alignment, offset);
    }
#else
    auto delegate = EngineHelper::GetCurrentDelegateSafely();
    if (!delegate) {
        NapiThrow(env, "UI execution context not found.", ERROR_CODE_INTERNAL_ERROR);
        return nullptr;
    }
    if (showMode == NG::ToastShowMode::DEFAULT) {
        TAG_LOGD(AceLogTag::ACE_DIALOG, "before delegate show toast");
        delegate->ShowToast(messageString, duration, bottomString, showMode, alignment, offset);
    } else if (SubwindowManager::GetInstance() != nullptr) {
        TAG_LOGD(AceLogTag::ACE_DIALOG, "before subwindow manager show toast");
        SubwindowManager::GetInstance()->ShowToast(messageString, duration, bottomString, showMode, alignment, offset);
    }
#endif
    return nullptr;
}

struct PromptAsyncContext {
    napi_env env = nullptr;
    napi_value titleNApi = nullptr;
    napi_value messageNApi = nullptr;
    napi_value buttonsNApi = nullptr;
    napi_value autoCancel = nullptr;
    napi_value showInSubWindow = nullptr;
    napi_value isModal = nullptr;
    napi_value alignmentApi = nullptr;
    napi_value offsetApi = nullptr;
    napi_value maskRectApi = nullptr;
    napi_value builder = nullptr;
    napi_value onWillDismiss = nullptr;
    napi_value backgroundColorApi = nullptr;
    napi_value backgroundBlurStyleApi = nullptr;
    napi_value borderWidthApi = nullptr;
    napi_value borderColorApi = nullptr;
    napi_value borderStyleApi = nullptr;
    napi_value borderRadiusApi = nullptr;
    napi_value shadowApi = nullptr;
    napi_value widthApi = nullptr;
    napi_value heightApi = nullptr;
    napi_value frameNodePtr = nullptr;
    napi_value maskColorApi = nullptr;
    napi_value onDidAppear = nullptr;
    napi_value onDidDisappear = nullptr;
    napi_value onWillAppear = nullptr;
    napi_value onWillDisappear = nullptr;
    napi_value transitionApi = nullptr;
    napi_ref callbackSuccess = nullptr;
    napi_ref callbackCancel = nullptr;
    napi_ref callbackComplete = nullptr;
    std::string titleString;
    std::string messageString;
    std::vector<ButtonInfo> buttons;
    bool autoCancelBool = true;
    bool showInSubWindowBool = false;
    bool isModalBool = true;
    std::set<std::string> callbacks;
    std::string callbackSuccessString;
    std::string callbackCancelString;
    std::string callbackCompleteString;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    napi_ref builderRef = nullptr;
    napi_ref onWillDismissRef = nullptr;
    int32_t callbackType = -1;
    int32_t successType = -1;
    bool valid = true;
    int32_t instanceId = -1;
    void* nativePtr = nullptr;
    napi_ref onDidAppearRef = nullptr;
    napi_ref onDidDisappearRef = nullptr;
    napi_ref onWillAppearRef = nullptr;
    napi_ref onWillDisappearRef = nullptr;
};

void DeleteContextAndThrowError(
    napi_env env, std::shared_ptr<PromptAsyncContext>& context, const std::string& errorMessage)
{
    if (!context) {
        // context is null, no need to delete
        return;
    }
    NapiThrow(env, errorMessage, ERROR_CODE_PARAM_INVALID);
}

int32_t GetButtonArraryLen(napi_env env, std::shared_ptr<PromptAsyncContext>& context,
    int32_t maxButtonNum)
{
    uint32_t buttonsLen = 0;
    napi_get_array_length(env, context->buttonsNApi, &buttonsLen);
    int32_t buttonsLenInt = static_cast<int32_t>(buttonsLen);
    if (buttonsLenInt > maxButtonNum && maxButtonNum != -1) {
        buttonsLenInt = maxButtonNum;
    }
    return buttonsLenInt;
}

void GetPrimaryButtonNum(napi_env env, std::shared_ptr<PromptAsyncContext>& context,
    int32_t buttonsLenInt, int32_t& primaryButtonNum)
{
    napi_value buttonArray = nullptr;
    napi_value primaryButtonNApi = nullptr;
    napi_valuetype valueType = napi_undefined;
    for (int32_t index = 0; index < buttonsLenInt; index++) {
        napi_get_element(env, context->buttonsNApi, index, &buttonArray);
        bool isPrimaryButtonSet = false;
        napi_get_named_property(env, buttonArray, "primary", &primaryButtonNApi);
        napi_typeof(env, primaryButtonNApi, &valueType);
        if (valueType == napi_boolean) {
            napi_get_value_bool(env, primaryButtonNApi, &isPrimaryButtonSet);
        }
        if (isPrimaryButtonSet) {
            primaryButtonNum++;
        }
    }
}

bool ParseButtons(napi_env env, std::shared_ptr<PromptAsyncContext>& context,
    int32_t maxButtonNum, int32_t& primaryButtonNum)
{
    napi_value buttonArray = nullptr;
    napi_value textNApi = nullptr;
    napi_value colorNApi = nullptr;
    napi_value primaryButtonNApi = nullptr;
    napi_valuetype valueType = napi_undefined;
    int32_t buttonsLenInt = GetButtonArraryLen(env, context, maxButtonNum);
    GetPrimaryButtonNum(env, context, buttonsLenInt, primaryButtonNum);
    for (int32_t index = 0; index < buttonsLenInt; index++) {
        napi_get_element(env, context->buttonsNApi, index, &buttonArray);
        if (!HasProperty(env, buttonArray, "text")) {
            DeleteContextAndThrowError(env, context, "Required input parameters are missing.");
            return false;
        }
        std::string textString;
        napi_get_named_property(env, buttonArray, "text", &textNApi);
        if (!GetNapiString(env, textNApi, textString, valueType)) {
            DeleteContextAndThrowError(env, context, "The type of parameters is incorrect.");
            return false;
        }
        if (!HasProperty(env, buttonArray, "color")) {
            DeleteContextAndThrowError(env, context, "Required input parameters are missing.");
            return false;
        }
        std::string colorString;
        napi_get_named_property(env, buttonArray, "color", &colorNApi);
        if (!GetNapiString(env, colorNApi, colorString, valueType)) {
            if (valueType == napi_undefined) {
                colorString = DEFAULT_FONT_COLOR_STRING_VALUE;
            } else {
                DeleteContextAndThrowError(env, context, "The type of parameters is incorrect.");
                return false;
            }
        }
        ButtonInfo buttonInfo = { .text = textString, .textColor = colorString };
        if (primaryButtonNum <= PROMPTACTION_VALID_PRIMARY_BUTTON_NUM) {
            napi_get_named_property(env, buttonArray, "primary", &primaryButtonNApi);
            napi_typeof(env, primaryButtonNApi, &valueType);
            if (valueType == napi_boolean) {
                napi_get_value_bool(env, primaryButtonNApi, &buttonInfo.isPrimary);
            }
        }
        context->buttons.emplace_back(buttonInfo);
    }
    return true;
}

bool ParseButtonsPara(napi_env env, std::shared_ptr<PromptAsyncContext>& context,
    int32_t maxButtonNum, bool isShowActionMenu)
{
    bool isBool = false;
    napi_valuetype valueType = napi_undefined;
    int32_t primaryButtonNum = 0;
    napi_is_array(env, context->buttonsNApi, &isBool);
    napi_typeof(env, context->buttonsNApi, &valueType);
    if (valueType == napi_object && isBool) {
        if (!ParseButtons(env, context, SHOW_DIALOG_BUTTON_NUM_MAX, primaryButtonNum)) {
            return false;
        }
    } else if (isShowActionMenu) {
        DeleteContextAndThrowError(env, context, "The type of the button parameters is incorrect.");
        return false;
    }
    if (isShowActionMenu) {
        ButtonInfo buttonInfo = { .text = Localization::GetInstance()->GetEntryLetters("common.cancel"),
            .textColor = "", .isPrimary = primaryButtonNum == 0 ? true : false};
        context->buttons.emplace_back(buttonInfo);
    }
    return true;
}

void GetNapiDialogProps(napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext,
    std::optional<DialogAlignment>& alignment, std::optional<DimensionOffset>& offset,
    std::optional<DimensionRect>& maskRect)
{
    TAG_LOGD(AceLogTag::ACE_DIALOG, "get napi dialog props enter");
    napi_valuetype valueType = napi_undefined;
    // parse alignment
    napi_typeof(env, asyncContext->alignmentApi, &valueType);
    if (valueType == napi_number) {
        int32_t num;
        napi_get_value_int32(env, asyncContext->alignmentApi, &num);
        if (num >= 0 && num < static_cast<int32_t>(DIALOG_ALIGNMENT.size())) {
            alignment = DIALOG_ALIGNMENT[num];
        }
    }

    // parse offset
    napi_typeof(env, asyncContext->offsetApi, &valueType);
    if (valueType == napi_object) {
        napi_value dxApi = nullptr;
        napi_value dyApi = nullptr;
        napi_get_named_property(env, asyncContext->offsetApi, "dx", &dxApi);
        napi_get_named_property(env, asyncContext->offsetApi, "dy", &dyApi);
        CalcDimension dx;
        CalcDimension dy;
        ParseNapiDimension(env, dx, dxApi, DimensionUnit::VP);
        ParseNapiDimension(env, dy, dyApi, DimensionUnit::VP);
        offset = DimensionOffset { dx, dy };
    }

    // parse maskRect
    napi_typeof(env, asyncContext->maskRectApi, &valueType);
    if (valueType == napi_object) {
        napi_value xApi = nullptr;
        napi_value yApi = nullptr;
        napi_value widthApi = nullptr;
        napi_value heightApi = nullptr;
        napi_get_named_property(env, asyncContext->maskRectApi, "x", &xApi);
        napi_get_named_property(env, asyncContext->maskRectApi, "y", &yApi);
        napi_get_named_property(env, asyncContext->maskRectApi, "width", &widthApi);
        napi_get_named_property(env, asyncContext->maskRectApi, "height", &heightApi);
        CalcDimension x;
        CalcDimension y;
        CalcDimension width;
        CalcDimension height;
        ParseNapiDimension(env, x, xApi, DimensionUnit::VP);
        ParseNapiDimension(env, y, yApi, DimensionUnit::VP);
        ParseNapiDimension(env, width, widthApi, DimensionUnit::VP);
        ParseNapiDimension(env, height, heightApi, DimensionUnit::VP);
        DimensionOffset dimensionOffset = { x, y };
        maskRect = DimensionRect { width, height, dimensionOffset };
    }
}

void GetNapiDialogbackgroundBlurStyleProps(napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext,
    std::optional<int32_t>& backgroundBlurStyle)
{
    TAG_LOGD(AceLogTag::ACE_DIALOG, "get napi dialog backgroundBlurStyle props enter");
    napi_valuetype valueType = napi_undefined;

    napi_typeof(env, asyncContext->backgroundBlurStyleApi, &valueType);
    if (valueType == napi_number) {
        int32_t num;
        napi_get_value_int32(env, asyncContext->backgroundBlurStyleApi, &num);
        if (num >= 0 && num < BG_BLUR_STYLE_MAX_INDEX) {
            backgroundBlurStyle = num;
        }
    }
}

bool ParseNapiDimensionNG(
    napi_env env, CalcDimension& result, napi_value napiValue, DimensionUnit defaultUnit, bool isSupportPercent)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiValue, &valueType);
    if (valueType == napi_number) {
        double value = 0;
        napi_get_value_double(env, napiValue, &value);

        result.SetUnit(defaultUnit);
        result.SetValue(value);
        return true;
    } else if (valueType == napi_string) {
        std::string valueString;
        if (!GetNapiString(env, napiValue, valueString, valueType)) {
            return false;
        }
        if (valueString.back() == '%' && !isSupportPercent) {
            return false;
        }
        return StringUtils::StringToCalcDimensionNG(valueString, result, false, defaultUnit);
    } else if (valueType == napi_object) {
        ResourceInfo recv;
        std::string parameterStr;
        if (!ParseResourceParam(env, napiValue, recv)) {
            return false;
        }
        if (!ParseString(recv, parameterStr)) {
            return false;
        }
        if (!ParseIntegerToString(recv, parameterStr)) {
            return false;
        }
        result = StringUtils::StringToDimensionWithUnit(parameterStr, defaultUnit);
        return true;
    }
    return false;
}

void CheckNapiDimension(CalcDimension value)
{
    if (value.IsNegative()) {
        value.Reset();
    }
}

bool ParseNapiColor(napi_env env, napi_value value, Color& result)
{
    napi_valuetype valueType = GetValueType(env, value);
    if (valueType != napi_number && valueType != napi_string && valueType != napi_object) {
        return false;
    }
    if (valueType == napi_number) {
        int32_t colorId = 0;
        napi_get_value_int32(env, value, &colorId);
        constexpr uint32_t colorAlphaOffset = 24;
        constexpr uint32_t colorAlphaDefaultValue = 0xFF000000;
        auto origin = static_cast<uint32_t>(colorId);
        uint32_t alphaResult = origin;
        if ((origin >> colorAlphaOffset) == 0) {
            alphaResult = origin | colorAlphaDefaultValue;
        }
        result = Color(alphaResult);
        return true;
    }
    if (valueType == napi_string) {
        std::optional<std::string> colorString = GetStringFromValueUtf8(env, value);
        if (!colorString.has_value()) {
            LOGE("Parse color from string failed");
            return false;
        }
        return Color::ParseColorString(colorString.value(), result);
    }

    return ParseColorFromResourceObject(env, value, result);
}

std::optional<NG::BorderColorProperty> GetBorderColorProps(
    napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext)
{
    napi_valuetype valueType = napi_undefined;
    NG::BorderColorProperty colorProperty;
    napi_typeof(env, asyncContext->borderColorApi, &valueType);
    if (valueType != napi_number && valueType != napi_string && valueType != napi_object) {
        return std::nullopt;
    }
    Color borderColor;
    if (ParseNapiColor(env, asyncContext->borderColorApi, borderColor)) {
        colorProperty.SetColor(borderColor);
        return colorProperty;
    } else if (valueType == napi_object) {
        napi_value leftApi = nullptr;
        napi_value rightApi = nullptr;
        napi_value topApi = nullptr;
        napi_value bottomApi = nullptr;
        napi_get_named_property(env, asyncContext->borderColorApi, "left", &leftApi);
        napi_get_named_property(env, asyncContext->borderColorApi, "right", &rightApi);
        napi_get_named_property(env, asyncContext->borderColorApi, "top", &topApi);
        napi_get_named_property(env, asyncContext->borderColorApi, "bottom", &bottomApi);
        Color leftColor;
        Color rightColor;
        Color topColor;
        Color bottomColor;
        if (ParseNapiColor(env, leftApi, leftColor)) {
            colorProperty.leftColor = leftColor;
        }
        if (ParseNapiColor(env, rightApi, rightColor)) {
            colorProperty.rightColor = rightColor;
        }
        if (ParseNapiColor(env, topApi, topColor)) {
            colorProperty.topColor = topColor;
        }
        if (ParseNapiColor(env, bottomApi, bottomColor)) {
            colorProperty.bottomColor = bottomColor;
        }
        colorProperty.multiValued = true;
        return colorProperty;
    }
    return std::nullopt;
}

std::optional<NG::BorderWidthProperty> GetBorderWidthProps(
    napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, asyncContext->borderWidthApi, &valueType);
    if (valueType != napi_number && valueType != napi_string && valueType != napi_object) {
        return std::nullopt;
    }
    NG::BorderWidthProperty borderWidthProps;
    CalcDimension borderWidth;
    if (ParseNapiDimensionNG(env, borderWidth, asyncContext->borderWidthApi, DimensionUnit::VP, true)) {
        CheckNapiDimension(borderWidth);
        borderWidthProps = NG::BorderWidthProperty({ borderWidth, borderWidth, borderWidth, borderWidth });
        return borderWidthProps;
    } else if (valueType == napi_object) {
        napi_value leftApi = nullptr;
        napi_value rightApi = nullptr;
        napi_value topApi = nullptr;
        napi_value bottomApi = nullptr;
        napi_get_named_property(env, asyncContext->borderWidthApi, "left", &leftApi);
        napi_get_named_property(env, asyncContext->borderWidthApi, "right", &rightApi);
        napi_get_named_property(env, asyncContext->borderWidthApi, "top", &topApi);
        napi_get_named_property(env, asyncContext->borderWidthApi, "bottom", &bottomApi);
        CalcDimension leftDimen;
        CalcDimension rightDimen;
        CalcDimension topDimen;
        CalcDimension bottomDimen;
        if (ParseNapiDimensionNG(env, leftDimen, leftApi, DimensionUnit::VP, true)) {
            CheckNapiDimension(leftDimen);
            borderWidthProps.leftDimen = leftDimen;
        }
        if (ParseNapiDimensionNG(env, rightDimen, rightApi, DimensionUnit::VP, true)) {
            CheckNapiDimension(rightDimen);
            borderWidthProps.rightDimen = rightDimen;
        }
        if (ParseNapiDimensionNG(env, topDimen, topApi, DimensionUnit::VP, true)) {
            CheckNapiDimension(topDimen);
            borderWidthProps.topDimen = topDimen;
        }
        if (ParseNapiDimensionNG(env, bottomDimen, bottomApi, DimensionUnit::VP, true)) {
            CheckNapiDimension(bottomDimen);
            borderWidthProps.bottomDimen = bottomDimen;
        }
        borderWidthProps.multiValued = true;
        return borderWidthProps;
    }
    return std::nullopt;
}

std::optional<NG::BorderRadiusProperty> GetBorderRadiusProps(
    napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, asyncContext->borderRadiusApi, &valueType);
    if (valueType != napi_number && valueType != napi_object && valueType != napi_string) {
        return std::nullopt;
    }
    CalcDimension borderRadius;
    if (ParseNapiDimensionNG(env, borderRadius, asyncContext->borderRadiusApi, DimensionUnit::VP, true)) {
        CheckNapiDimension(borderRadius);
        return NG::BorderRadiusProperty(borderRadius);
    } else if (valueType == napi_object) {
        NG::BorderRadiusProperty radiusProps;
        napi_value topLeft = nullptr;
        napi_value topRight = nullptr;
        napi_value bottomLeft = nullptr;
        napi_value bottomRight = nullptr;
        napi_get_named_property(env, asyncContext->borderRadiusApi, "topLeft", &topLeft);
        napi_get_named_property(env, asyncContext->borderRadiusApi, "topRight", &topRight);
        napi_get_named_property(env, asyncContext->borderRadiusApi, "bottomLeft", &bottomLeft);
        napi_get_named_property(env, asyncContext->borderRadiusApi, "bottomRight", &bottomRight);
        CalcDimension radiusTopLeft;
        CalcDimension radiusTopRight;
        CalcDimension radiusBottomLeft;
        CalcDimension radiusBottomRight;
        if (ParseNapiDimensionNG(env, radiusTopLeft, topLeft, DimensionUnit::VP, true)) {
            CheckNapiDimension(radiusTopLeft);
            radiusProps.radiusTopLeft = radiusTopLeft;
        }
        if (ParseNapiDimensionNG(env, radiusTopRight, topRight, DimensionUnit::VP, true)) {
            CheckNapiDimension(radiusTopRight);
            radiusProps.radiusTopRight = radiusTopRight;
        }
        if (ParseNapiDimensionNG(env, radiusBottomLeft, bottomLeft, DimensionUnit::VP, true)) {
            CheckNapiDimension(radiusBottomLeft);
            radiusProps.radiusBottomLeft = radiusBottomLeft;
        }
        if (ParseNapiDimensionNG(env, radiusBottomRight, bottomRight, DimensionUnit::VP, true)) {
            CheckNapiDimension(radiusBottomRight);
            radiusProps.radiusBottomRight = radiusBottomRight;
        }
        radiusProps.multiValued = true;
        return radiusProps;
    }
    return std::nullopt;
}

std::optional<Color> GetColorProps(napi_env env, napi_value value)
{
    Color color;
    if (ParseNapiColor(env, value, color)) {
        return color;
    }
    return std::nullopt;
}

bool ParseStyle(napi_env env, napi_value value, std::optional<BorderStyle>& style)
{
    napi_valuetype valueType = GetValueType(env, value);
    if (valueType != napi_number) {
        return false;
    }
    int32_t num;
    napi_get_value_int32(env, value, &num);
    style = static_cast<BorderStyle>(num);
    if (style < BorderStyle::SOLID || style > BorderStyle::NONE) {
        return false;
    }
    return true;
}

std::optional<NG::BorderStyleProperty> GetBorderStyleProps(
    napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext)
{
    NG::BorderStyleProperty styleProps;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, asyncContext->borderStyleApi, &valueType);
    if (valueType != napi_number && valueType != napi_object) {
        return std::nullopt;
    } else if (valueType == napi_object) {
        napi_value leftApi = nullptr;
        napi_value rightApi = nullptr;
        napi_value topApi = nullptr;
        napi_value bottomApi = nullptr;
        napi_get_named_property(env, asyncContext->borderStyleApi, "left", &leftApi);
        napi_get_named_property(env, asyncContext->borderStyleApi, "right", &rightApi);
        napi_get_named_property(env, asyncContext->borderStyleApi, "top", &topApi);
        napi_get_named_property(env, asyncContext->borderStyleApi, "bottom", &bottomApi);
        std::optional<BorderStyle> styleLeft;
        std::optional<BorderStyle> styleRight;
        std::optional<BorderStyle> styleTop;
        std::optional<BorderStyle> styleBottom;
        if (ParseStyle(env, leftApi, styleLeft)) {
            styleProps.styleLeft = styleLeft;
        }
        if (ParseStyle(env, rightApi, styleRight)) {
            styleProps.styleRight = styleRight;
        }
        if (ParseStyle(env, topApi, styleTop)) {
            styleProps.styleTop = styleTop;
        }
        if (ParseStyle(env, bottomApi, styleBottom)) {
            styleProps.styleBottom = styleBottom;
        }
        styleProps.multiValued = true;
        return styleProps;
    }
    std::optional<BorderStyle> borderStyle;
    if (ParseStyle(env, asyncContext->borderStyleApi, borderStyle)) {
        styleProps = NG::BorderStyleProperty({ borderStyle, borderStyle, borderStyle, borderStyle });
        return styleProps;
    }
    return std::nullopt;
}

bool ParseShadowColorStrategy(napi_env env, napi_value value, ShadowColorStrategy& strategy)
{
    napi_valuetype valueType = GetValueType(env, value);
    if (valueType == napi_string) {
        std::optional<std::string> colorStr = GetStringFromValueUtf8(env, value);
        if (colorStr.has_value()) {
            if (colorStr->compare("average") == 0) {
                strategy = ShadowColorStrategy::AVERAGE;
                return true;
            } else if (colorStr->compare("primary") == 0) {
                strategy = ShadowColorStrategy::PRIMARY;
                return true;
            }
        }
    }
    return false;
}

bool GetShadowFromTheme(ShadowStyle shadowStyle, Shadow& shadow)
{
    auto colorMode = SystemProperties::GetColorMode();
    if (shadowStyle == ShadowStyle::None) {
        return true;
    }
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, false);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_RETURN(pipelineContext, false);
    auto shadowTheme = pipelineContext->GetTheme<ShadowTheme>();
    if (!shadowTheme) {
        return false;
    }
    shadow = shadowTheme->GetShadow(shadowStyle, colorMode);
    return true;
}

void GetNapiObjectShadow(napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext, Shadow& shadow)
{
    napi_value radiusApi = nullptr;
    napi_value colorApi = nullptr;
    napi_value typeApi = nullptr;
    napi_value fillApi = nullptr;
    napi_get_named_property(env, asyncContext->shadowApi, "radius", &radiusApi);
    napi_get_named_property(env, asyncContext->shadowApi, "color", &colorApi);
    napi_get_named_property(env, asyncContext->shadowApi, "type", &typeApi);
    napi_get_named_property(env, asyncContext->shadowApi, "fill", &fillApi);
    double radius = 0.0;
    napi_get_value_double(env, radiusApi, &radius);
    if (LessNotEqual(radius, 0.0)) {
        radius = 0.0;
    }
    shadow.SetBlurRadius(radius);
    Color color;
    ShadowColorStrategy shadowColorStrategy;
    if (ParseShadowColorStrategy(env, colorApi, shadowColorStrategy)) {
        shadow.SetShadowColorStrategy(shadowColorStrategy);
    } else if (ParseNapiColor(env, colorApi, color)) {
        shadow.SetColor(color);
    }
    napi_valuetype valueType = GetValueType(env, typeApi);
    int32_t shadowType = static_cast<int32_t>(ShadowType::COLOR);
    if (valueType == napi_number) {
        napi_get_value_int32(env, typeApi, &shadowType);
    }
    if (shadowType != static_cast<int32_t>(ShadowType::BLUR)) {
        shadowType = static_cast<int32_t>(ShadowType::COLOR);
    }
    shadowType =
        std::clamp(shadowType, static_cast<int32_t>(ShadowType::COLOR), static_cast<int32_t>(ShadowType::BLUR));
    shadow.SetShadowType(static_cast<ShadowType>(shadowType));
    valueType = GetValueType(env, fillApi);
    bool isFilled = false;
    if (valueType == napi_boolean) {
        napi_get_value_bool(env, fillApi, &isFilled);
    }
    shadow.SetIsFilled(isFilled);
}

std::optional<Shadow> GetShadowProps(napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext)
{
    Shadow shadow;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, asyncContext->shadowApi, &valueType);
    if (valueType != napi_object && valueType != napi_number) {
        return std::nullopt;
    }
    if (valueType == napi_number) {
        int32_t num = 0;
        if (napi_get_value_int32(env, asyncContext->shadowApi, &num) == napi_ok) {
            auto style = static_cast<ShadowStyle>(num);
            GetShadowFromTheme(style, shadow);
            return shadow;
        }
    } else if (valueType == napi_object) {
        napi_value offsetXApi = nullptr;
        napi_value offsetYApi = nullptr;
        napi_get_named_property(env, asyncContext->shadowApi, "offsetX", &offsetXApi);
        napi_get_named_property(env, asyncContext->shadowApi, "offsetY", &offsetYApi);
        ResourceInfo recv;
        bool isRtl = AceApplicationInfo::GetInstance().IsRightToLeft();
        if (ParseResourceParam(env, offsetXApi, recv)) {
            auto resourceWrapper = CreateResourceWrapper(recv);
            auto offsetX = resourceWrapper->GetDimension(recv.resId);
            double xValue = isRtl ? offsetX.Value() * (-1) : offsetX.Value();
            shadow.SetOffsetX(xValue);
        } else {
            CalcDimension offsetX;
            if (ParseNapiDimension(env, offsetX, offsetXApi, DimensionUnit::VP)) {
                double xValue = isRtl ? offsetX.Value() * (-1) : offsetX.Value();
                shadow.SetOffsetX(xValue);
            }
        }
        if (ParseResourceParam(env, offsetYApi, recv)) {
            auto resourceWrapper = CreateResourceWrapper(recv);
            auto offsetY = resourceWrapper->GetDimension(recv.resId);
            shadow.SetOffsetY(offsetY.Value());
        } else {
            CalcDimension offsetY;
            if (ParseNapiDimension(env, offsetY, offsetYApi, DimensionUnit::VP)) {
                shadow.SetOffsetY(offsetY.Value());
            }
        }
        GetNapiObjectShadow(env, asyncContext, shadow);
        return shadow;
    }
    return std::nullopt;
}

std::optional<CalcDimension> GetNapiDialogWidthProps(
    napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext)
{
    std::optional<CalcDimension> widthProperty;
    CalcDimension width;
    if (ParseNapiDimensionNG(env, width, asyncContext->widthApi, DimensionUnit::VP, true)) {
        widthProperty = width;
    }
    return widthProperty;
}

std::optional<CalcDimension> GetNapiDialogHeightProps(
    napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext)
{
    std::optional<CalcDimension> heightProperty;
    CalcDimension height;
    if (ParseNapiDimensionNG(env, height, asyncContext->heightApi, DimensionUnit::VP, true)) {
        heightProperty = height;
    }
    return heightProperty;
}

void GetNapiNamedProperties(napi_env env, napi_value* argv, size_t index,
    std::shared_ptr<PromptAsyncContext>& asyncContext)
{
    napi_valuetype valueType = napi_undefined;

    if (index == 0) {
        napi_get_named_property(env, argv[index], "builder", &asyncContext->builder);
        napi_get_named_property(env, argv[index], "backgroundColor", &asyncContext->backgroundColorApi);
        napi_get_named_property(env, argv[index], "backgroundBlurStyle", &asyncContext->backgroundBlurStyleApi);
        napi_get_named_property(env, argv[index], "cornerRadius", &asyncContext->borderRadiusApi);
        napi_get_named_property(env, argv[index], "borderWidth", &asyncContext->borderWidthApi);
        napi_get_named_property(env, argv[index], "borderColor", &asyncContext->borderColorApi);
        napi_get_named_property(env, argv[index], "borderStyle", &asyncContext->borderStyleApi);
        napi_get_named_property(env, argv[index], "shadow", &asyncContext->shadowApi);
        napi_get_named_property(env, argv[index], "width", &asyncContext->widthApi);
        napi_get_named_property(env, argv[index], "height", &asyncContext->heightApi);

        napi_typeof(env, asyncContext->builder, &valueType);
        if (valueType == napi_function) {
            napi_create_reference(env, asyncContext->builder, 1, &asyncContext->builderRef);
        }
    }
    napi_get_named_property(env, argv[index], "showInSubWindow", &asyncContext->showInSubWindow);
    napi_get_named_property(env, argv[index], "isModal", &asyncContext->isModal);
    napi_get_named_property(env, argv[index], "alignment", &asyncContext->alignmentApi);
    napi_get_named_property(env, argv[index], "offset", &asyncContext->offsetApi);
    napi_get_named_property(env, argv[index], "maskRect", &asyncContext->maskRectApi);
    napi_get_named_property(env, argv[index], "autoCancel", &asyncContext->autoCancel);
    napi_get_named_property(env, argv[index], "maskColor", &asyncContext->maskColorApi);
    napi_get_named_property(env, argv[index], "transition", &asyncContext->transitionApi);
    napi_get_named_property(env, argv[index], "onWillDismiss", &asyncContext->onWillDismiss);
    napi_get_named_property(env, argv[index], "onDidAppear", &asyncContext->onDidAppear);
    napi_get_named_property(env, argv[index], "onDidDisappear", &asyncContext->onDidDisappear);
    napi_get_named_property(env, argv[index], "onWillAppear", &asyncContext->onWillAppear);
    napi_get_named_property(env, argv[index], "onWillDisappear", &asyncContext->onWillDisappear);

    napi_typeof(env, asyncContext->autoCancel, &valueType);
    if (valueType == napi_boolean) {
        napi_get_value_bool(env, asyncContext->autoCancel, &asyncContext->autoCancelBool);
    }
    napi_typeof(env, asyncContext->showInSubWindow, &valueType);
    if (valueType == napi_boolean) {
        napi_get_value_bool(env, asyncContext->showInSubWindow, &asyncContext->showInSubWindowBool);
    }
    napi_typeof(env, asyncContext->isModal, &valueType);
    if (valueType == napi_boolean) {
        napi_get_value_bool(env, asyncContext->isModal, &asyncContext->isModalBool);
    }
}

bool JSPromptParseParam(napi_env env, size_t argc, napi_value* argv, std::shared_ptr<PromptAsyncContext>& asyncContext)
{
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0 || i == 1) {
            if (valueType != napi_object) {
                DeleteContextAndThrowError(env, asyncContext, "The type of parameters is incorrect.");
                return false;
            }
            GetNapiNamedProperties(env, argv, i, asyncContext);
            auto result = napi_get_named_property(env, argv[0], "nodePtr_", &asyncContext->frameNodePtr);
            if (result == napi_ok) {
                napi_get_value_external(env, asyncContext->frameNodePtr, &asyncContext->nativePtr);
            }

            napi_typeof(env, asyncContext->onWillDismiss, &valueType);
            if (valueType == napi_function) {
                napi_create_reference(env, asyncContext->onWillDismiss, 1, &asyncContext->onWillDismissRef);
            }
            napi_typeof(env, asyncContext->onDidAppear, &valueType);
            if (valueType == napi_function) {
                napi_create_reference(env, asyncContext->onDidAppear, 1, &asyncContext->onDidAppearRef);
            }
            napi_typeof(env, asyncContext->onDidDisappear, &valueType);
            if (valueType == napi_function) {
                napi_create_reference(env, asyncContext->onDidDisappear, 1, &asyncContext->onDidDisappearRef);
            }
            napi_typeof(env, asyncContext->onWillAppear, &valueType);
            if (valueType == napi_function) {
                napi_create_reference(env, asyncContext->onWillAppear, 1, &asyncContext->onWillAppearRef);
            }
            napi_typeof(env, asyncContext->onWillDisappear, &valueType);
            if (valueType == napi_function) {
                napi_create_reference(env, asyncContext->onWillDisappear, 1, &asyncContext->onWillDisappearRef);
            }
        } else {
            DeleteContextAndThrowError(env, asyncContext, "The type of parameters is incorrect.");
            return false;
        }
    }
    return true;
}

void JSPromptThrowInterError(napi_env env, std::shared_ptr<PromptAsyncContext>& asyncContext, std::string& strMsg)
{
    napi_value code = nullptr;
    std::string strCode = std::to_string(ERROR_CODE_INTERNAL_ERROR);
    napi_create_string_utf8(env, strCode.c_str(), strCode.length(), &code);
    napi_value msg = nullptr;
    napi_create_string_utf8(env, strMsg.c_str(), strMsg.length(), &msg);
    napi_value error = nullptr;
    napi_create_error(env, code, msg, &error);

    if (asyncContext->deferred) {
        napi_reject_deferred(env, asyncContext->deferred, error);
    }
}

void UpdatePromptAlignment(DialogAlignment& alignment)
{
    bool isRtl = AceApplicationInfo::GetInstance().IsRightToLeft();
    if (alignment == DialogAlignment::TOP_START) {
        if (isRtl) {
            alignment = DialogAlignment::TOP_END;
        }
    } else if (alignment == DialogAlignment::TOP_END) {
        if (isRtl) {
            alignment = DialogAlignment::TOP_START;
        }
    } else if (alignment == DialogAlignment::CENTER_START) {
        if (isRtl) {
            alignment = DialogAlignment::CENTER_END;
        }
    } else if (alignment == DialogAlignment::CENTER_END) {
        if (isRtl) {
            alignment = DialogAlignment::CENTER_START;
        }
    } else if (alignment == DialogAlignment::BOTTOM_START) {
        if (isRtl) {
            alignment = DialogAlignment::BOTTOM_END;
        }
    } else if (alignment == DialogAlignment::BOTTOM_END) {
        if (isRtl) {
            alignment = DialogAlignment::BOTTOM_START;
        }
    }
}

napi_value JSPromptShowDialog(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AceLogTag::ACE_DIALOG, "js prompt show dialog enter");
    size_t requireArgc = 1;
    size_t argc = 2;
    napi_value argv[3] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc < requireArgc) {
        NapiThrow(
            env, "The number of parameters must be greater than or equal to 1.", ERROR_CODE_PARAM_INVALID);
        return nullptr;
    }
    if (thisVar == nullptr) {
        return nullptr;
    }
    napi_valuetype valueTypeOfThis = napi_undefined;
    napi_typeof(env, thisVar, &valueTypeOfThis);
    if (valueTypeOfThis == napi_undefined) {
        return nullptr;
    }

    auto asyncContext = std::make_shared<PromptAsyncContext>();
    asyncContext->env = env;
    asyncContext->instanceId = Container::CurrentIdSafely();

    std::optional<DialogAlignment> alignment;
    std::optional<DimensionOffset> offset;
    std::optional<DimensionRect> maskRect;
    std::optional<Shadow> shadowProps;
    std::optional<Color> backgroundColor;
    std::optional<int32_t> backgroundBlurStyle;

    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0) {
            if (valueType != napi_object) {
                DeleteContextAndThrowError(env, asyncContext, "The type of parameters is incorrect.");
                return nullptr;
            }
            napi_get_named_property(env, argv[0], "title", &asyncContext->titleNApi);
            napi_get_named_property(env, argv[0], "message", &asyncContext->messageNApi);
            napi_get_named_property(env, argv[0], "buttons", &asyncContext->buttonsNApi);
            napi_get_named_property(env, argv[0], "autoCancel", &asyncContext->autoCancel);
            napi_get_named_property(env, argv[0], "showInSubWindow", &asyncContext->showInSubWindow);
            napi_get_named_property(env, argv[0], "isModal", &asyncContext->isModal);
            napi_get_named_property(env, argv[0], "alignment", &asyncContext->alignmentApi);
            napi_get_named_property(env, argv[0], "offset", &asyncContext->offsetApi);
            napi_get_named_property(env, argv[0], "maskRect", &asyncContext->maskRectApi);
            napi_get_named_property(env, argv[0], "shadow", &asyncContext->shadowApi);
            napi_get_named_property(env, argv[0], "backgroundColor", &asyncContext->backgroundColorApi);
            napi_get_named_property(env, argv[0], "backgroundBlurStyle", &asyncContext->backgroundBlurStyleApi);
            GetNapiString(env, asyncContext->titleNApi, asyncContext->titleString, valueType);
            GetNapiString(env, asyncContext->messageNApi, asyncContext->messageString, valueType);
            GetNapiDialogProps(env, asyncContext, alignment, offset, maskRect);
            GetNapiDialogbackgroundBlurStyleProps(env, asyncContext, backgroundBlurStyle);
            backgroundColor = GetColorProps(env, asyncContext->backgroundColorApi);
            shadowProps = GetShadowProps(env, asyncContext);
            if (!ParseButtonsPara(env, asyncContext, SHOW_DIALOG_BUTTON_NUM_MAX, false)) {
                return nullptr;
            }
            napi_typeof(env, asyncContext->autoCancel, &valueType);
            if (valueType == napi_boolean) {
                napi_get_value_bool(env, asyncContext->autoCancel, &asyncContext->autoCancelBool);
            }
            napi_typeof(env, asyncContext->showInSubWindow, &valueType);
            if (valueType == napi_boolean) {
                napi_get_value_bool(env, asyncContext->showInSubWindow, &asyncContext->showInSubWindowBool);
            }
            napi_typeof(env, asyncContext->isModal, &valueType);
            if (valueType == napi_boolean) {
                napi_get_value_bool(env, asyncContext->isModal, &asyncContext->isModalBool);
            }
        } else if (valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &asyncContext->callbackRef);
        } else {
            DeleteContextAndThrowError(env, asyncContext, "The type of parameters is incorrect.");
            return nullptr;
        }
    }
    auto onLanguageChange = [shadowProps, alignment, offset,
        updateAlignment = UpdatePromptAlignment](DialogProperties& dialogProps) mutable {
        bool isRtl = AceApplicationInfo::GetInstance().IsRightToLeft();
        if (shadowProps.has_value()) {
            std::optional<Shadow> shadow = shadowProps.value();
            double offsetX = isRtl ? shadow->GetOffset().GetX() * (-1) : shadow->GetOffset().GetX();
            shadow->SetOffsetX(offsetX);
            dialogProps.shadow = shadow.value();
        }
        if (alignment.has_value()) {
            std::optional<DialogAlignment> pmAlign = alignment.value();
            updateAlignment(pmAlign.value());
            dialogProps.alignment = pmAlign.value();
        }
        if (offset.has_value()) {
            std::optional<DimensionOffset> pmOffset = offset.value();
            double xValue = isRtl ? pmOffset->GetX().Value() * (-1) : pmOffset->GetX().Value();
            Dimension offsetX = Dimension(xValue);
            pmOffset->SetX(offsetX);
            dialogProps.offset = pmOffset.value();
        }
    };
    napi_value result = nullptr;
    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }
    asyncContext->callbacks.emplace("success");
    asyncContext->callbacks.emplace("cancel");

    auto callBack = [asyncContext](int32_t callbackType, int32_t successType) mutable {
        if (asyncContext == nullptr) {
            return;
        }

        asyncContext->callbackType = callbackType;
        asyncContext->successType = successType;
        auto container = AceEngine::Get().GetContainer(asyncContext->instanceId);
        if (!container) {
            return;
        }

        auto taskExecutor = container->GetTaskExecutor();
        if (!taskExecutor) {
            return;
        }
        taskExecutor->PostTask(
            [asyncContext]() {
                if (asyncContext == nullptr) {
                    return;
                }

                if (!asyncContext->valid) {
                    return;
                }

                napi_handle_scope scope = nullptr;
                napi_open_handle_scope(asyncContext->env, &scope);
                if (scope == nullptr) {
                    return;
                }

                napi_value ret;
                napi_value successIndex = nullptr;
                napi_create_int32(asyncContext->env, asyncContext->successType, &successIndex);
                napi_value indexObj = nullptr;
                napi_create_object(asyncContext->env, &indexObj);
                napi_set_named_property(asyncContext->env, indexObj, "index", successIndex);
                napi_value result[2] = { 0 };
                napi_create_object(asyncContext->env, &result[1]);
                napi_set_named_property(asyncContext->env, result[1], "index", successIndex);
                bool dialogResult = true;
                switch (asyncContext->callbackType) {
                    case 0:
                        napi_get_undefined(asyncContext->env, &result[0]);
                        dialogResult = true;
                        break;
                    case 1:
                        napi_value message = nullptr;
                        napi_create_string_utf8(asyncContext->env, "cancel", strlen("cancel"), &message);
                        napi_create_error(asyncContext->env, nullptr, message, &result[0]);
                        dialogResult = false;
                        break;
                }
                if (asyncContext->deferred) {
                    if (dialogResult) {
                        napi_resolve_deferred(asyncContext->env, asyncContext->deferred, result[1]);
                    } else {
                        napi_reject_deferred(asyncContext->env, asyncContext->deferred, result[0]);
                    }
                } else {
                    napi_value callback = nullptr;
                    napi_get_reference_value(asyncContext->env, asyncContext->callbackRef, &callback);
                    napi_call_function(
                        asyncContext->env, nullptr, callback, sizeof(result) / sizeof(result[0]), result, &ret);
                    napi_delete_reference(asyncContext->env, asyncContext->callbackRef);
                }
                napi_close_handle_scope(asyncContext->env, scope);
            },
            TaskExecutor::TaskType::JS, "ArkUIDialogParseDialogCallback");
        asyncContext = nullptr;
    };

    PromptDialogAttr promptDialogAttr = {
        .title = asyncContext->titleString,
        .message = asyncContext->messageString,
        .autoCancel = asyncContext->autoCancelBool,
        .showInSubWindow = asyncContext->showInSubWindowBool,
        .isModal = asyncContext->isModalBool,
        .alignment = alignment,
        .offset = offset,
        .maskRect = maskRect,
        .backgroundColor = backgroundColor,
        .backgroundBlurStyle = backgroundBlurStyle,
        .shadow = shadowProps,
        .onLanguageChange = onLanguageChange,
    };

#ifdef OHOS_STANDARD_SYSTEM
    // NG
    if (SystemProperties::GetExtSurfaceEnabled() || !ContainerIsService()) {
        auto delegate = EngineHelper::GetCurrentDelegateSafely();
        if (delegate) {
            delegate->ShowDialog(promptDialogAttr, asyncContext->buttons, std::move(callBack), asyncContext->callbacks);
        } else {
            // throw internal error
            napi_value code = nullptr;
            std::string strCode = std::to_string(ERROR_CODE_INTERNAL_ERROR);
            napi_create_string_utf8(env, strCode.c_str(), strCode.length(), &code);
            napi_value msg = nullptr;
            std::string strMsg = ErrorToMessage(ERROR_CODE_INTERNAL_ERROR) + "Can not get delegate.";
            napi_create_string_utf8(env, strMsg.c_str(), strMsg.length(), &msg);
            napi_value error = nullptr;
            napi_create_error(env, code, msg, &error);

            if (asyncContext->deferred) {
                napi_reject_deferred(env, asyncContext->deferred, error);
            } else {
                napi_value ret1;
                napi_value callback = nullptr;
                napi_get_reference_value(env, asyncContext->callbackRef, &callback);
                napi_call_function(env, nullptr, callback, 1, &error, &ret1);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
        }
    } else if (SubwindowManager::GetInstance() != nullptr) {
        SubwindowManager::GetInstance()->ShowDialog(
            promptDialogAttr, asyncContext->buttons, std::move(callBack), asyncContext->callbacks);
    }
#else
    auto delegate = EngineHelper::GetCurrentDelegateSafely();
    if (delegate) {
        delegate->ShowDialog(promptDialogAttr, asyncContext->buttons, std::move(callBack), asyncContext->callbacks);
    } else {
        // throw internal error
        napi_value code = nullptr;
        std::string strCode = std::to_string(ERROR_CODE_INTERNAL_ERROR);
        napi_create_string_utf8(env, strCode.c_str(), strCode.length(), &code);
        napi_value msg = nullptr;
        std::string strMsg = ErrorToMessage(ERROR_CODE_INTERNAL_ERROR) + "UI execution context not found.";
        napi_create_string_utf8(env, strMsg.c_str(), strMsg.length(), &msg);
        napi_value error = nullptr;
        napi_create_error(env, code, msg, &error);

        if (asyncContext->deferred) {
            napi_reject_deferred(env, asyncContext->deferred, error);
        } else {
            napi_value ret1;
            napi_value callback = nullptr;
            napi_get_reference_value(env, asyncContext->callbackRef, &callback);
            napi_call_function(env, nullptr, callback, 1, &error, &ret1);
            napi_delete_reference(env, asyncContext->callbackRef);
        }
    }
#endif
    return result;
}

napi_value JSPromptShowActionMenu(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AceLogTag::ACE_DIALOG, "js prompt show action menu enter");
    size_t requireArgc = 1;
    size_t argc = 2;
    napi_value argv[3] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc < requireArgc) {
        NapiThrow(
            env, "The number of parameters must be greater than or equal to 1.", ERROR_CODE_PARAM_INVALID);
        return nullptr;
    }
    if (thisVar == nullptr) {
        return nullptr;
    }
    napi_valuetype valueTypeOfThis = napi_undefined;
    napi_typeof(env, thisVar, &valueTypeOfThis);
    if (valueTypeOfThis == napi_undefined) {
        return nullptr;
    }

    auto asyncContext = std::make_shared<PromptAsyncContext>();
    asyncContext->env = env;
    asyncContext->instanceId = Container::CurrentIdSafely();
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0) {
            if (valueType != napi_object) {
                DeleteContextAndThrowError(env, asyncContext, "The type of parameters is incorrect.");
                return nullptr;
            }
            napi_get_named_property(env, argv[0], "title", &asyncContext->titleNApi);
            napi_get_named_property(env, argv[0], "showInSubWindow", &asyncContext->showInSubWindow);
            napi_get_named_property(env, argv[0], "isModal", &asyncContext->isModal);
            GetNapiString(env, asyncContext->titleNApi, asyncContext->titleString, valueType);
            if (!HasProperty(env, argv[0], "buttons")) {
                DeleteContextAndThrowError(env, asyncContext, "Required input parameters are missing.");
                return nullptr;
            }
            napi_get_named_property(env, argv[0], "buttons", &asyncContext->buttonsNApi);
            if (!ParseButtonsPara(env, asyncContext, SHOW_ACTION_MENU_BUTTON_NUM_MAX, true)) {
                return nullptr;
            }
            napi_typeof(env, asyncContext->showInSubWindow, &valueType);
            if (valueType == napi_boolean) {
                napi_get_value_bool(env, asyncContext->showInSubWindow, &asyncContext->showInSubWindowBool);
            }
            napi_typeof(env, asyncContext->isModal, &valueType);
            if (valueType == napi_boolean) {
                napi_get_value_bool(env, asyncContext->isModal, &asyncContext->isModalBool);
            }
        } else if (valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &asyncContext->callbackRef);
        } else {
            DeleteContextAndThrowError(env, asyncContext, "The type of parameters is incorrect.");
            return nullptr;
        }
    }
    napi_value result = nullptr;
    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    auto callBack = [asyncContext](int32_t callbackType, int32_t successType) mutable {
        if (asyncContext == nullptr) {
            return;
        }

        asyncContext->callbackType = callbackType;
        asyncContext->successType = successType;
        auto container = AceEngine::Get().GetContainer(asyncContext->instanceId);
        if (!container) {
            return;
        }

        auto taskExecutor = container->GetTaskExecutor();
        if (!taskExecutor) {
            return;
        }
        taskExecutor->PostTask(
            [asyncContext]() {
                if (asyncContext == nullptr) {
                    return;
                }

                if (!asyncContext->valid) {
                    return;
                }

                napi_handle_scope scope = nullptr;
                napi_open_handle_scope(asyncContext->env, &scope);
                if (scope == nullptr) {
                    return;
                }

                napi_value ret;
                napi_value successIndex = nullptr;
                napi_create_int32(asyncContext->env, asyncContext->successType, &successIndex);
                asyncContext->callbackSuccessString = "showActionMenu:ok";
                napi_value indexObj = GetReturnObject(asyncContext->env, asyncContext->callbackSuccessString);
                napi_set_named_property(asyncContext->env, indexObj, "index", successIndex);
                napi_value result[2] = { 0 };
                napi_create_object(asyncContext->env, &result[1]);
                napi_set_named_property(asyncContext->env, result[1], "index", successIndex);
                bool dialogResult = true;
                switch (asyncContext->callbackType) {
                    case 0:
                        napi_get_undefined(asyncContext->env, &result[0]);
                        dialogResult = true;
                        break;
                    case 1:
                        napi_value message = nullptr;
                        napi_create_string_utf8(asyncContext->env, "cancel", strlen("cancel"), &message);
                        napi_create_error(asyncContext->env, nullptr, message, &result[0]);
                        dialogResult = false;
                        break;
                }
                if (asyncContext->deferred) {
                    if (dialogResult) {
                        napi_resolve_deferred(asyncContext->env, asyncContext->deferred, result[1]);
                    } else {
                        napi_reject_deferred(asyncContext->env, asyncContext->deferred, result[0]);
                    }
                } else {
                    napi_value callback = nullptr;
                    napi_get_reference_value(asyncContext->env, asyncContext->callbackRef, &callback);
                    napi_call_function(
                        asyncContext->env, nullptr, callback, sizeof(result) / sizeof(result[0]), result, &ret);
                    napi_delete_reference(asyncContext->env, asyncContext->callbackRef);
                }
                napi_close_handle_scope(asyncContext->env, scope);
            },
            TaskExecutor::TaskType::JS, "ArkUIDialogParseActionMenuCallback");
        asyncContext = nullptr;
    };

    PromptDialogAttr promptDialogAttr = {
        .title = asyncContext->titleString,
        .showInSubWindow = asyncContext->showInSubWindowBool,
        .isModal = asyncContext->isModalBool,
    };
#ifdef OHOS_STANDARD_SYSTEM
    if (SystemProperties::GetExtSurfaceEnabled() || !ContainerIsService()) {
        auto delegate = EngineHelper::GetCurrentDelegateSafely();
        if (delegate) {
            delegate->ShowActionMenu(promptDialogAttr, asyncContext->buttons, std::move(callBack));
        } else {
            napi_value code = nullptr;
            std::string strCode = std::to_string(ERROR_CODE_INTERNAL_ERROR);
            napi_create_string_utf8(env, strCode.c_str(), strCode.length(), &code);
            napi_value msg = nullptr;
            std::string strMsg = ErrorToMessage(ERROR_CODE_INTERNAL_ERROR) + "Can not get delegate.";
            napi_create_string_utf8(env, strMsg.c_str(), strMsg.length(), &msg);
            napi_value error = nullptr;
            napi_create_error(env, code, msg, &error);

            if (asyncContext->deferred) {
                napi_reject_deferred(env, asyncContext->deferred, error);
            } else {
                napi_value ret1;
                napi_value callback = nullptr;
                napi_get_reference_value(env, asyncContext->callbackRef, &callback);
                napi_call_function(env, nullptr, callback, 1, &error, &ret1);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
        }
    } else if (SubwindowManager::GetInstance() != nullptr) {
        SubwindowManager::GetInstance()->ShowActionMenu(
            asyncContext->titleString, asyncContext->buttons, std::move(callBack));
    }
#else
    auto delegate = EngineHelper::GetCurrentDelegateSafely();
    if (delegate) {
        delegate->ShowActionMenu(promptDialogAttr, asyncContext->buttons, std::move(callBack));
    } else {
        napi_value code = nullptr;
        std::string strCode = std::to_string(ERROR_CODE_INTERNAL_ERROR);
        napi_create_string_utf8(env, strCode.c_str(), strCode.length(), &code);
        napi_value msg = nullptr;
        std::string strMsg = ErrorToMessage(ERROR_CODE_INTERNAL_ERROR) + "UI execution context not found.";
        napi_create_string_utf8(env, strMsg.c_str(), strMsg.length(), &msg);
        napi_value error = nullptr;
        napi_create_error(env, code, msg, &error);

        if (asyncContext->deferred) {
            napi_reject_deferred(env, asyncContext->deferred, error);
        } else {
            napi_value ret1;
            napi_value callback = nullptr;
            napi_get_reference_value(env, asyncContext->callbackRef, &callback);
            napi_call_function(env, nullptr, callback, 1, &error, &ret1);
            napi_delete_reference(env, asyncContext->callbackRef);
        }
    }
#endif
    return result;
}

napi_value JSRemoveCustomDialog(napi_env env, napi_callback_info info)
{
    auto delegate = EngineHelper::GetCurrentDelegateSafely();
    if (delegate) {
        delegate->RemoveCustomDialog();
    }
    return nullptr;
}

void ParseDialogCallback(std::shared_ptr<PromptAsyncContext>& asyncContext,
    std::function<void(const int32_t& info)>& onWillDismiss)
{
    onWillDismiss = [env = asyncContext->env, onWillDismissRef = asyncContext->onWillDismissRef]
        (const int32_t& info) {
        if (onWillDismissRef) {
            napi_value onWillDismissFunc = nullptr;
            napi_value value = nullptr;
            napi_value funcValue = nullptr;
            napi_value paramObj = nullptr;
            napi_create_object(env, &paramObj);

            napi_create_function(env, "dismiss", strlen("dismiss"), JSRemoveCustomDialog, nullptr, &funcValue);
            napi_set_named_property(env, paramObj, "dismiss", funcValue);

            napi_create_int32(env, info, &value);
            napi_set_named_property(env, paramObj, "reason", value);
            napi_get_reference_value(env, onWillDismissRef, &onWillDismissFunc);
            napi_call_function(env, nullptr, onWillDismissFunc, 1, &paramObj, nullptr);
        }
    };
}

PromptDialogAttr GetDialogLifeCycleCallback(napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext)
{
    auto onDidAppear = [env = asyncContext->env, onDidAppearRef = asyncContext->onDidAppearRef]() {
        if (onDidAppearRef) {
            napi_value onDidAppearFunc = nullptr;
            napi_get_reference_value(env, onDidAppearRef, &onDidAppearFunc);
            napi_call_function(env, nullptr, onDidAppearFunc, 0, nullptr, nullptr);
            napi_delete_reference(env, onDidAppearRef);
        }
    };
    auto onDidDisappear = [env = asyncContext->env, onDidDisappearRef = asyncContext->onDidDisappearRef]() {
        if (onDidDisappearRef) {
            napi_value onDidDisappearFunc = nullptr;
            napi_get_reference_value(env, onDidDisappearRef, &onDidDisappearFunc);
            napi_call_function(env, nullptr, onDidDisappearFunc, 0, nullptr, nullptr);
            napi_delete_reference(env, onDidDisappearRef);
        }
    };
    auto onWillAppear = [env = asyncContext->env, onWillAppearRef = asyncContext->onWillAppearRef]() {
        if (onWillAppearRef) {
            napi_value onWillAppearFunc = nullptr;
            napi_get_reference_value(env, onWillAppearRef, &onWillAppearFunc);
            napi_call_function(env, nullptr, onWillAppearFunc, 0, nullptr, nullptr);
            napi_delete_reference(env, onWillAppearRef);
        }
    };
    auto onWillDisappear = [env = asyncContext->env, onWillDisappearRef = asyncContext->onWillDisappearRef]() {
        if (onWillDisappearRef) {
            napi_value onWillDisappearFunc = nullptr;
            napi_get_reference_value(env, onWillDisappearRef, &onWillDisappearFunc);
            napi_call_function(env, nullptr, onWillDisappearFunc, 0, nullptr, nullptr);
            napi_delete_reference(env, onWillDisappearRef);
        }
    };
    PromptDialogAttr promptDialogAttr = {
        .onDidAppear =  std::move(onDidAppear),
        .onDidDisappear = std::move(onDidDisappear),
        .onWillAppear = std::move(onWillAppear),
        .onWillDisappear = std::move(onWillDisappear) };
    return promptDialogAttr;
}

void ParseBorderColorAndStyle(napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext,
    std::optional<NG::BorderWidthProperty>& borderWidthProps, std::optional<NG::BorderColorProperty>& borderColorProps,
    std::optional<NG::BorderStyleProperty>& borderStyleProps)
{
    if (borderWidthProps.has_value()) {
        borderColorProps = GetBorderColorProps(env, asyncContext);
        if (!borderColorProps.has_value()) {
            NG::BorderColorProperty borderColor;
            borderColor.SetColor(Color::BLACK);
            borderColorProps = borderColor;
        }
        borderStyleProps = GetBorderStyleProps(env, asyncContext);
        if (!borderStyleProps.has_value()) {
            borderStyleProps = NG::BorderStyleProperty(
                { BorderStyle::SOLID, BorderStyle::SOLID, BorderStyle::SOLID, BorderStyle::SOLID });
        }
    }
}

RefPtr<NG::ChainedTransitionEffect> GetTransitionProps(
    napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext)
{
    RefPtr<NG::ChainedTransitionEffect> transitionEffect = nullptr;
    auto delegate = EngineHelper::GetCurrentDelegateSafely();
    if (delegate) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, asyncContext->transitionApi, &valueType);
        if (valueType == napi_object) {
            transitionEffect = delegate->GetTransitionEffect(asyncContext->transitionApi);
        }
    }
    return transitionEffect;
}

std::function<void()> GetCustomBuilder(napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext)
{
    auto builder = [env = asyncContext->env, builderRef = asyncContext->builderRef]() {
        if (builderRef) {
            napi_value builderFunc = nullptr;
            napi_get_reference_value(env, builderRef, &builderFunc);
            napi_call_function(env, nullptr, builderFunc, 0, nullptr, nullptr);
            napi_delete_reference(env, builderRef);
        }
    };
    return builder;
}

PromptDialogAttr GetPromptActionDialog(napi_env env, const std::shared_ptr<PromptAsyncContext>& asyncContext,
    std::function<void(const int32_t& info)> onWillDismiss)
{
    std::optional<DialogAlignment> alignment;
    std::optional<DimensionOffset> offset;
    std::optional<DimensionRect> maskRect;
    std::optional<int32_t> backgroundBlurStyle;
    GetNapiDialogProps(env, asyncContext, alignment, offset, maskRect);
    auto borderWidthProps = GetBorderWidthProps(env, asyncContext);
    std::optional<NG::BorderColorProperty> borderColorProps;
    std::optional<NG::BorderStyleProperty> borderStyleProps;
    GetNapiDialogbackgroundBlurStyleProps(env, asyncContext, backgroundBlurStyle);
    ParseBorderColorAndStyle(env, asyncContext, borderWidthProps, borderColorProps, borderStyleProps);
    auto borderRadiusProps = GetBorderRadiusProps(env, asyncContext);
    auto backgroundColorProps = GetColorProps(env, asyncContext->backgroundColorApi);
    auto widthProps = GetNapiDialogWidthProps(env, asyncContext);
    auto heightProps = GetNapiDialogHeightProps(env, asyncContext);
    auto shadowProps = GetShadowProps(env, asyncContext);
    auto builder = GetCustomBuilder(env, asyncContext);
    auto* nodePtr = reinterpret_cast<OHOS::Ace::NG::UINode*>(asyncContext->nativePtr);
    auto frameNodeWeak = AceType::WeakClaim(nodePtr);
    auto maskColorProps = GetColorProps(env, asyncContext->maskColorApi);
    auto transitionEffectProps = GetTransitionProps(env, asyncContext);
    PromptDialogAttr lifeCycleAttr = GetDialogLifeCycleCallback(env, asyncContext);
    PromptDialogAttr promptDialogAttr = { .autoCancel = asyncContext->autoCancelBool,
        .showInSubWindow = asyncContext->showInSubWindowBool,
        .isModal = asyncContext->isModalBool,
        .customBuilder = std::move(builder),
        .customOnWillDismiss = std::move(onWillDismiss),
        .alignment = alignment,
        .offset = offset,
        .maskRect = maskRect,
        .backgroundColor = backgroundColorProps,
        .backgroundBlurStyle = backgroundBlurStyle,
        .borderWidth = borderWidthProps,
        .borderColor = borderColorProps,
        .borderStyle = borderStyleProps,
        .borderRadius = borderRadiusProps,
        .shadow = shadowProps,
        .width = widthProps,
        .height = heightProps,
        .contentNode = frameNodeWeak,
        .maskColor = maskColorProps,
        .transitionEffect = transitionEffectProps,
        .onDidAppear = lifeCycleAttr.onDidAppear,
        .onDidDisappear = lifeCycleAttr.onDidDisappear,
        .onWillAppear = lifeCycleAttr.onWillAppear,
        .onWillDisappear = lifeCycleAttr.onWillDisappear };
    return promptDialogAttr;
}

std::string GetErrorMsg(int32_t errorCode)
{
    std::string strMsg;
    if (errorCode == ERROR_CODE_DIALOG_CONTENT_ERROR) {
        strMsg = ErrorToMessage(ERROR_CODE_DIALOG_CONTENT_ERROR) + "The ComponentContent is incorrect.";
    } else if (errorCode == ERROR_CODE_DIALOG_CONTENT_ALREADY_EXIST) {
        strMsg = ErrorToMessage(ERROR_CODE_DIALOG_CONTENT_ALREADY_EXIST) +
            "The ComponentContent has already been opened.";
    } else if (errorCode == ERROR_CODE_DIALOG_CONTENT_NOT_FOUND) {
        strMsg = ErrorToMessage(ERROR_CODE_DIALOG_CONTENT_NOT_FOUND) + "The ComponentContent cannot be found.";
    } else {
        strMsg = ErrorToMessage(ERROR_CODE_INTERNAL_ERROR) + "Build custom dialog failed.";
    }
    return strMsg;
}

std::string GetErrorCode(int32_t errorCode)
{
    std::string strCode;
    if (errorCode == ERROR_CODE_DIALOG_CONTENT_ERROR) {
        strCode = std::to_string(ERROR_CODE_DIALOG_CONTENT_ERROR);
    } else if (errorCode == ERROR_CODE_DIALOG_CONTENT_ALREADY_EXIST) {
        strCode = std::to_string(ERROR_CODE_DIALOG_CONTENT_ALREADY_EXIST);
    } else if (errorCode == ERROR_CODE_DIALOG_CONTENT_NOT_FOUND) {
        strCode = std::to_string(ERROR_CODE_DIALOG_CONTENT_NOT_FOUND);
    } else {
        strCode = std::to_string(ERROR_CODE_INTERNAL_ERROR);
    }
    return strCode;
}

void ParseCustomDialogContentCallback(std::shared_ptr<PromptAsyncContext>& asyncContext,
    std::function<void(int32_t)>& callBack)
{
    callBack = [asyncContext](int32_t errorCode) mutable {
        if (!asyncContext) {
            return;
        }
        auto container = AceEngine::Get().GetContainer(asyncContext->instanceId);
        if (!container) {
            return;
        }
        auto taskExecutor = container->GetTaskExecutor();
        if (!taskExecutor) {
            return;
        }
        taskExecutor->PostTask(
            [asyncContext, errorCode]() {
                if (asyncContext == nullptr || !asyncContext->valid) {
                    return;
                }
                napi_handle_scope scope = nullptr;
                napi_open_handle_scope(asyncContext->env, &scope);
                if (scope == nullptr) {
                    return;
                }
                if (!asyncContext->deferred) {
                    return;
                }
                if (errorCode == ERROR_CODE_NO_ERROR) {
                    napi_value result = nullptr;
                    napi_get_undefined(asyncContext->env, &result);
                    napi_resolve_deferred(asyncContext->env, asyncContext->deferred, result);
                } else {
                    std::string strMsg = GetErrorMsg(errorCode);
                    std::string strCode = GetErrorCode(errorCode);
                    napi_value code = nullptr;
                    napi_create_string_utf8(asyncContext->env, strCode.c_str(), strCode.length(), &code);
                    napi_value msg = nullptr;
                    napi_create_string_utf8(asyncContext->env, strMsg.c_str(), strMsg.length(), &msg);
                    napi_value error = nullptr;
                    napi_create_error(asyncContext->env, code, msg, &error);
                    napi_reject_deferred(asyncContext->env, asyncContext->deferred, error);
                }
                napi_close_handle_scope(asyncContext->env, scope);
            },
            TaskExecutor::TaskType::JS, "ArkUIDialogParseCustomDialogContentCallback");
        asyncContext = nullptr;
    };
}

void ParseCustomDialogIdCallback(std::shared_ptr<PromptAsyncContext>& asyncContext,
    std::function<void(int32_t)>& callBack)
{
    callBack = [asyncContext](int32_t dialogId) mutable {
        if (!asyncContext) {
            return;
        }
        auto container = AceEngine::Get().GetContainer(asyncContext->instanceId);
        if (!container) {
            return;
        }
        auto taskExecutor = container->GetTaskExecutor();
        if (!taskExecutor) {
            return;
        }
        taskExecutor->PostTask(
            [asyncContext, dialogId]() {
                if (asyncContext == nullptr || !asyncContext->valid) {
                    return;
                }

                napi_handle_scope scope = nullptr;
                napi_open_handle_scope(asyncContext->env, &scope);
                if (scope == nullptr) {
                    return;
                }

                napi_value ret = nullptr;
                if (!asyncContext->deferred) {
                    return;
                }
                if (dialogId > 0) {
                    napi_create_int32(asyncContext->env, dialogId, &ret);
                    napi_resolve_deferred(asyncContext->env, asyncContext->deferred, ret);
                } else {
                    std::string strMsg = GetErrorMsg(dialogId);
                    std::string strCode = GetErrorCode(dialogId);
                    napi_value code = nullptr;
                    napi_create_string_utf8(asyncContext->env, strCode.c_str(), strCode.length(), &code);
                    napi_value msg = nullptr;
                    napi_create_string_utf8(asyncContext->env, strMsg.c_str(), strMsg.length(), &msg);
                    napi_value error = nullptr;
                    napi_create_error(asyncContext->env, code, msg, &error);
                    napi_reject_deferred(asyncContext->env, asyncContext->deferred, error);
                }
                napi_close_handle_scope(asyncContext->env, scope);
            },
            TaskExecutor::TaskType::JS, "ArkUIDialogParseCustomDialogIdCallback");
        asyncContext = nullptr;
    };
}

void OpenCustomDialog(napi_env env, std::shared_ptr<PromptAsyncContext>& asyncContext,
    PromptDialogAttr& promptDialogAttr, std::function<void(int32_t)>& openCallback)
{
#ifdef OHOS_STANDARD_SYSTEM
    // NG
    if (SystemProperties::GetExtSurfaceEnabled() || !ContainerIsService()) {
        auto delegate = EngineHelper::GetCurrentDelegateSafely();
        if (delegate) {
            delegate->OpenCustomDialog(promptDialogAttr, std::move(openCallback));
        } else {
            // throw internal error
            std::string strMsg = ErrorToMessage(ERROR_CODE_INTERNAL_ERROR) + "Can not get delegate.";
            JSPromptThrowInterError(env, asyncContext, strMsg);
        }
    } else if (SubwindowManager::GetInstance() != nullptr) {
        SubwindowManager::GetInstance()->OpenCustomDialog(promptDialogAttr, std::move(openCallback));
    }
#else
    auto delegate = EngineHelper::GetCurrentDelegateSafely();
    if (delegate) {
        delegate->OpenCustomDialog(promptDialogAttr, std::move(openCallback));
    } else {
        // throw internal error
        std::string strMsg = ErrorToMessage(ERROR_CODE_INTERNAL_ERROR) + "UI execution context not found.";
        JSPromptThrowInterError(env, asyncContext, strMsg);
    }
#endif
}

napi_value JSPromptOpenCustomDialog(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        NapiThrow(
            env, "The number of parameters must be greater than or equal to 1.", ERROR_CODE_PARAM_INVALID);
        return nullptr;
    }

    auto asyncContext = std::make_shared<PromptAsyncContext>();
    asyncContext->env = env;
    asyncContext->instanceId = Container::CurrentIdSafely();
    bool parseOK = JSPromptParseParam(env, argc, argv, asyncContext);
    if (!parseOK) {
        return nullptr;
    }
    napi_value result = nullptr;
    napi_create_promise(env, &asyncContext->deferred, &result);

    std::function<void(const int32_t& info)> onWillDismiss = nullptr;
    if (asyncContext->onWillDismissRef) {
        ParseDialogCallback(asyncContext, onWillDismiss);
    }
    std::function<void(int32_t)> openCallback = nullptr;
    PromptDialogAttr promptDialogAttr = GetPromptActionDialog(env, asyncContext, onWillDismiss);
    if (!asyncContext->builderRef) {
        ParseCustomDialogContentCallback(asyncContext, openCallback);
        promptDialogAttr.customStyle = true;
        promptDialogAttr.customBuilder = nullptr;
    } else {
        ParseCustomDialogIdCallback(asyncContext, openCallback);
    }

    OpenCustomDialog(env, asyncContext, promptDialogAttr, openCallback);

    return result;
}

void CloseCustomDialog(napi_env env, std::shared_ptr<PromptAsyncContext>& asyncContext, bool useDialogId,
    int32_t dialogId, const WeakPtr<NG::UINode>& nodeWk, std::function<void(int32_t)>& contentCallback)
{
#ifdef OHOS_STANDARD_SYSTEM
    // NG
    if (SystemProperties::GetExtSurfaceEnabled() || !ContainerIsService()) {
        auto delegate = EngineHelper::GetCurrentDelegateSafely();
        if (delegate) {
            if (useDialogId) {
                delegate->CloseCustomDialog(dialogId);
            } else {
                delegate->CloseCustomDialog(nodeWk, std::move(contentCallback));
            }
        } else {
            // throw internal error
            napi_create_promise(env, &asyncContext->deferred, nullptr);
            std::string strMsg = ErrorToMessage(ERROR_CODE_INTERNAL_ERROR) + "Can not get delegate.";
            JSPromptThrowInterError(env, asyncContext, strMsg);
        }
    } else if (SubwindowManager::GetInstance() != nullptr) {
        if (useDialogId) {
            SubwindowManager::GetInstance()->CloseCustomDialogNG(dialogId);
        } else {
            SubwindowManager::GetInstance()->CloseCustomDialogNG(nodeWk, std::move(contentCallback));
        }
    }
#else
    auto delegate = EngineHelper::GetCurrentDelegateSafely();
    if (delegate) {
        if (useDialogId) {
            delegate->CloseCustomDialog(dialogId);
        } else {
            delegate->CloseCustomDialog(nodeWk, std::move(contentCallback));
        }
    } else {
        // throw internal error
        napi_create_promise(env, &asyncContext->deferred, nullptr);
        std::string strMsg = ErrorToMessage(ERROR_CODE_INTERNAL_ERROR) + "UI execution context not found.";
        JSPromptThrowInterError(env, asyncContext, strMsg);
    }
#endif
}

napi_value JSPromptCloseCustomDialog(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    int32_t dialogId = -1;
    WeakPtr<NG::UINode> nodeWk;
    bool useDialogId = true;
    std::function<void(int32_t)> contentCallback = nullptr;
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    auto asyncContext = std::make_shared<PromptAsyncContext>();
    asyncContext->env = env;
    asyncContext->instanceId = Container::CurrentIdSafely();
    napi_value ret = nullptr;
    if (argc > 1) {
        NapiThrow(env, "The number of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
        return nullptr;
    } else if (argc == 0) {
        dialogId = -1;
    } else {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType == napi_number) {
            napi_get_value_int32(env, argv[0], &dialogId);
        } else if (valueType == napi_object) {
            napi_value frameNodePtr = nullptr;
            auto result = napi_get_named_property(env, argv[0], "nodePtr_", &frameNodePtr);
            if (result != napi_ok) {
                NapiThrow(env, "The type of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
                return nullptr;
            }
            void* nativePtr = nullptr;
            result = napi_get_value_external(env, frameNodePtr, &nativePtr);
            if (result != napi_ok) {
                NapiThrow(env, "The type of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
                return nullptr;
            }
            auto* uiNodePtr = reinterpret_cast<OHOS::Ace::NG::UINode*>(nativePtr);
            nodeWk = AceType::WeakClaim(uiNodePtr);
            useDialogId = false;
            napi_create_promise(env, &asyncContext->deferred, &ret);
            ParseCustomDialogContentCallback(asyncContext, contentCallback);
        } else {
            NapiThrow(env, "The type of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
            return nullptr;
        }
    }

    CloseCustomDialog(env, asyncContext, useDialogId, dialogId, nodeWk, contentCallback);

    return ret;
}

void UpdateCustomDialog(napi_env env, std::shared_ptr<PromptAsyncContext>& asyncContext,
    PromptDialogAttr& promptDialogAttr, const WeakPtr<NG::UINode>& nodeWk,
    std::function<void(int32_t)>& contentCallback)
{
#ifdef OHOS_STANDARD_SYSTEM
    // NG
    if (SystemProperties::GetExtSurfaceEnabled() || !ContainerIsService()) {
        auto delegate = EngineHelper::GetCurrentDelegateSafely();
        if (delegate) {
            delegate->UpdateCustomDialog(nodeWk, promptDialogAttr, std::move(contentCallback));
        } else {
            // throw internal error
            napi_create_promise(env, &asyncContext->deferred, nullptr);
            std::string strMsg = ErrorToMessage(ERROR_CODE_INTERNAL_ERROR) + "Can not get delegate.";
            JSPromptThrowInterError(env, asyncContext, strMsg);
        }
    } else if (SubwindowManager::GetInstance() != nullptr) {
        SubwindowManager::GetInstance()->UpdateCustomDialogNG(nodeWk, promptDialogAttr, std::move(contentCallback));
    }
#else
    auto delegate = EngineHelper::GetCurrentDelegateSafely();
    if (delegate) {
        delegate->UpdateCustomDialog(nodeWk, promptDialogAttr, std::move(contentCallback));
    } else {
        // throw internal error
        napi_create_promise(env, &asyncContext->deferred, nullptr);
        std::string strMsg = ErrorToMessage(ERROR_CODE_INTERNAL_ERROR) + "UI execution context not found.";
        JSPromptThrowInterError(env, asyncContext, strMsg);
    }
#endif
}

napi_value JSPromptUpdateCustomDialog(napi_env env, napi_callback_info info)
{
    size_t argc = CUSTOM_DIALOG_PARAM_NUM;
    napi_value argv[CUSTOM_DIALOG_PARAM_NUM] = { nullptr };
    WeakPtr<NG::UINode> nodeWk;
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != CUSTOM_DIALOG_PARAM_NUM) {
        NapiThrow(env, "The number of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
        return nullptr;
    }
    auto asyncContext = std::make_shared<PromptAsyncContext>();
    asyncContext->env = env;
    asyncContext->instanceId = Container::CurrentIdSafely();
    napi_value ret = nullptr;

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType == napi_object) {
        napi_value frameNodePtr = nullptr;
        auto result = napi_get_named_property(env, argv[0], "nodePtr_", &frameNodePtr);
        if (result != napi_ok) {
            NapiThrow(env, "The type of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
            return nullptr;
        }
        void* nativePtr = nullptr;
        result = napi_get_value_external(env, frameNodePtr, &nativePtr);
        if (result != napi_ok) {
            NapiThrow(env, "The type of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
            return nullptr;
        }
        auto* uiNodePtr = reinterpret_cast<OHOS::Ace::NG::UINode*>(nativePtr);
        nodeWk = AceType::WeakClaim(uiNodePtr);
    } else {
        NapiThrow(env, "The type of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
        return nullptr;
    }

    napi_typeof(env, argv[1], &valueType);
    if (valueType != napi_object) {
        NapiThrow(env, "The type of parameters is incorrect.", ERROR_CODE_PARAM_INVALID);
        return nullptr;
    }
    GetNapiNamedProperties(env, argv, 1, asyncContext);

    napi_create_promise(env, &asyncContext->deferred, &ret);
    std::function<void(int32_t)> contentCallback = nullptr;
    ParseCustomDialogContentCallback(asyncContext, contentCallback);
    PromptDialogAttr promptDialogAttr = GetPromptActionDialog(env, asyncContext, nullptr);

    UpdateCustomDialog(env, asyncContext, promptDialogAttr, nodeWk, contentCallback);

    return ret;
}

} // namespace OHOS::Ace::Napi
