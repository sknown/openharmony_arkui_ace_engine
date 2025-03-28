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

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>

#include "interfaces/napi/kits/utils/napi_utils.h"
#include "js_native_api.h"
#include "js_native_api_types.h"
#include "napi/native_common.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "native_value.h"
#include "node_api.h"

#if defined(ENABLE_DRAG_FRAMEWORK) && defined(PIXEL_MAP_SUPPORTED)
#include "jsnapi.h"
#include "pixel_map.h"
#include "pixel_map_napi.h"

#include "adapter/ohos/capability/interaction/start_drag_listener_impl.h"
#include "base/log/log_wrapper.h"
#include "base/memory/referenced.h"
#include "base/msdp/device_status/interfaces/innerkits/interaction/include/interaction_manager.h"
#include "base/utils/utils.h"
#include "bridge/common/utils/utils.h"
#include "bridge/declarative_frontend/engine/functions/js_drag_function.h"
#include "bridge/declarative_frontend/engine/jsi/jsi_declarative_engine.h"
#include "bridge/js_frontend/engine/jsi/ark_js_runtime.h"
#include "core/common/ace_engine.h"
#include "core/common/container_scope.h"
#include "core/common/udmf/udmf_client.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components_ng/manager/drag_drop/drag_drop_func_wrapper.h"
#include "core/event/ace_events.h"
#include "frameworks/bridge/common/utils/engine_helper.h"
#include "frameworks/base/json/json_util.h"
#include "drag_preview.h"
#endif
namespace OHOS::Ace::Napi {
class DragAction;
static constexpr uint32_t DRAG_STARTED = 0;
static constexpr uint32_t DRAG_ENDED = 1;
#if defined(ENABLE_DRAG_FRAMEWORK) && defined(PIXEL_MAP_SUPPORTED)
namespace {
constexpr float PIXELMAP_WIDTH_RATE = -0.5f;
constexpr float PIXELMAP_HEIGHT_RATE = -0.2f;
constexpr size_t STR_BUFFER_SIZE = 1024;
constexpr int32_t PARAMETER_NUM = 2;
constexpr int32_t argCount3 = 3;
constexpr int32_t SOURCE_TYPE_MOUSE = 1;
constexpr int32_t MOUSE_POINTER_ID = 1001;
constexpr int32_t SOURCE_TOOL_PEN = 1;
constexpr int32_t SOURCE_TYPE_TOUCH = 2;
constexpr int32_t PEN_POINTER_ID = 102;

using DragNotifyMsg = Msdp::DeviceStatus::DragNotifyMsg;
using DragRet = OHOS::Ace::DragRet;
using OnDragCallback = std::function<void(const DragNotifyMsg&)>;
using StopDragCallback = std::function<void()>;
using PixelMapNapiEntry = void* (*)(void*, void*);

enum class DragState { PENDING, SENDING, REJECT, SUCCESS };
enum class DragStatus { STARTED, ENDED };
enum class ParameterType { CUSTOMBUILDER, DRAGITEMINFO, DRAGITEMINFO_ARRAY, MIX, ERROR };

// the context of drag controller
struct DragControllerAsyncCtx {
    napi_env env = nullptr;
    size_t argc = 3;
    napi_value argv[3] { nullptr };
    napi_ref callbackRef = nullptr;
    napi_deferred deferred = nullptr;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    std::vector<std::shared_ptr<Media::PixelMap>> pixelMapList;
    bool isArray = false;
    napi_value customBuilder;
    std::vector<napi_ref> customBuilderList;
    int32_t pointerId = -1;
    RefPtr<OHOS::Ace::UnifiedData> unifiedData;
    std::string extraParams;
    int32_t instanceId = -1;
    int32_t errCode = -1;
    std::mutex mutex;
    bool hasHandle = false;
    int32_t globalX = -1;
    int32_t globalY = -1;
    uint64_t displayId = 0;
    int32_t sourceType = 0;
    float windowScale = 1.0f;
    float dipScale = 0.0;
    int parseBuilderCount = 0;
    std::mutex dragStateMutex;
    DragState dragState = DragState::PENDING;
    DimensionOffset touchPoint = DimensionOffset(0.0_vp, 0.0_vp);
    bool hasTouchPoint = false;
    DragAction *dragAction = nullptr;
    NG::DragPreviewOption dragPreviewOption;
    ~DragControllerAsyncCtx();
};
} // namespace

void OnMultipleComplete(DragControllerAsyncCtx* asyncCtx);
void OnComplete(DragControllerAsyncCtx* asyncCtx);
bool GetPixelMapByCustom(DragControllerAsyncCtx* asyncCtx);
bool GetPixelMapArrayByCustom(DragControllerAsyncCtx* asyncCtx, napi_value customBuilder, int arrayLength);
ParameterType getParameterType(DragControllerAsyncCtx* asyncCtx);

class DragAction {
public:
    DragAction(DragControllerAsyncCtx* asyncCtx) : asyncCtx_(asyncCtx) {}
    ~DragAction()
    {
        CHECK_NULL_VOID(env_);
        for (auto& item : cbList_) {
            napi_delete_reference(env_, item);
        }
    }

    void OnNapiCallback(napi_value resultArg)
    {
        std::vector<napi_value> cbList(cbList_.size());
        for (auto& cbRef : cbList_) {
            napi_value cb = nullptr;
            napi_get_reference_value(env_, cbRef, &cb);
            cbList.push_back(cb);
        }
        for (auto& cb : cbList) {
            napi_call_function(env_, nullptr, cb, 1, &resultArg, nullptr);
        }
    }

    void NapiSerializer(napi_env& env, napi_value& result)
    {
        napi_wrap(
            env, result, this,
            [](napi_env env, void* data, void* hint) {
                DragAction* dragAction = static_cast<DragAction*>(data);
                if (dragAction != nullptr) {
                    dragAction->DeleteRef();
                    delete dragAction;
                }
            },
            nullptr, nullptr);

        /* insert callback functions */
        const char* funName = "on";
        napi_value funcValue = nullptr;
        napi_create_function(env, funName, NAPI_AUTO_LENGTH, On, nullptr, &funcValue);
        napi_set_named_property(env, result, funName, funcValue);

        funName = "off";
        napi_create_function(env, funName, NAPI_AUTO_LENGTH, Off, nullptr, &funcValue);
        napi_set_named_property(env, result, funName, funcValue);

        funName = "startDrag";
        napi_create_function(env, funName, NAPI_AUTO_LENGTH, StartDrag, nullptr, &funcValue);
        napi_set_named_property(env, result, funName, funcValue);
    }

    void DeleteRef()
    {
        CHECK_NULL_VOID(asyncCtx_);
        for (auto customBuilderValue : asyncCtx_->customBuilderList) {
            if (customBuilderValue == nullptr) {
                continue;
            }
            napi_delete_reference(asyncCtx_->env, customBuilderValue);
        }
        delete asyncCtx_;
        asyncCtx_ = nullptr;
    }

    static napi_value On(napi_env env, napi_callback_info info)
    {
        TAG_LOGI(AceLogTag::ACE_DRAG, "drag action On function called.");
        auto jsEngine = EngineHelper::GetCurrentEngineSafely();
        CHECK_NULL_RETURN(jsEngine, nullptr);

        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        CHECK_NULL_RETURN(scope, nullptr);
        napi_value thisVar = nullptr;
        napi_value cb = nullptr;
        size_t argc = ParseArgs(env, info, thisVar, cb);
        NAPI_ASSERT(env, (argc == 2 && thisVar != nullptr && cb != nullptr), "Invalid arguments");
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, cb, &valueType);
        if (valueType != napi_function) {
            NapiThrow(env, "Check param failed", ERROR_CODE_PARAM_INVALID);
            napi_close_handle_scope(env, scope);
            return nullptr;
        }
        DragAction* dragAction = ConvertDragAction(env, thisVar);
        if (!dragAction) {
            NapiThrow(env, "convert drag action failed.", ERROR_CODE_PARAM_INVALID);
            napi_close_handle_scope(env, scope);
            return nullptr;
        }
        auto iter = dragAction->FindCbList(cb);
        if (iter != dragAction->cbList_.end()) {
            NapiThrow(env, "get js callback function error.", ERROR_CODE_PARAM_INVALID);
            napi_close_handle_scope(env, scope);
            return nullptr;
        }
        napi_ref ref = nullptr;
        napi_create_reference(env, cb, 1, &ref);
        dragAction->cbList_.emplace_back(ref);
        napi_close_handle_scope(env, scope);
        return nullptr;
    }

    static napi_value Off(napi_env env, napi_callback_info info)
    {
        TAG_LOGI(AceLogTag::ACE_DRAG, "drag action Off function called.");
        napi_value thisVar = nullptr;
        napi_value cb = nullptr;
        size_t argc = ParseArgs(env, info, thisVar, cb);
        DragAction* dragAction = ConvertDragAction(env, thisVar);
        CHECK_NULL_RETURN(dragAction, nullptr);
        if (argc == 1) {
            for (const auto& item : dragAction->cbList_) {
                napi_delete_reference(dragAction->env_, item);
            }
            dragAction->cbList_.clear();
        } else {
            NAPI_ASSERT(env, (argc == 2 && dragAction != nullptr && cb != nullptr), "Invalid arguments");
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, cb, &valueType);
            if (valueType != napi_function) {
                NapiThrow(env, "Check param failed", ERROR_CODE_PARAM_INVALID);
                return nullptr;
            }
            auto iter = dragAction->FindCbList(cb);
            if (iter != dragAction->cbList_.end()) {
                napi_delete_reference(dragAction->env_, *iter);
                dragAction->cbList_.erase(iter);
            }
        }
        return nullptr;
    }

    static napi_value StartDrag(napi_env env, napi_callback_info info)
    {
        TAG_LOGI(AceLogTag::ACE_DRAG, "drag action StartDrag function called.");
        auto jsEngine = EngineHelper::GetCurrentEngineSafely();
        CHECK_NULL_RETURN(jsEngine, nullptr);

        napi_escapable_handle_scope scope = nullptr;
        napi_open_escapable_handle_scope(env, &scope);
        CHECK_NULL_RETURN(scope, nullptr);
        napi_value thisVar = nullptr;
        napi_value cb = nullptr;
        size_t argc = ParseArgs(env, info, thisVar, cb);
        NAPI_ASSERT(env, (argc == 0 && thisVar != nullptr), "Invalid arguments");
        DragAction* dragAction = ConvertDragAction(env, thisVar);
        if (!dragAction) {
            NapiThrow(env, "convert drag action failed.", ERROR_CODE_INTERNAL_ERROR);
            napi_close_escapable_handle_scope(env, scope);
            return nullptr;
        }
        if (dragAction->asyncCtx_ == nullptr) {
            NapiThrow(env, "drag action must be recreated for each dragging", ERROR_CODE_INTERNAL_ERROR);
            napi_close_escapable_handle_scope(env, scope);
            return nullptr;
        }
        napi_value promiseResult = nullptr;
        napi_status status = napi_create_promise(env, &dragAction->asyncCtx_->deferred, &promiseResult);
        if (status != napi_ok) {
            NapiThrow(env, "ace engine delegate is null", ERROR_CODE_INTERNAL_ERROR);
            napi_close_escapable_handle_scope(env, scope);
            return nullptr;
        }

        dragAction->StartDragInternal(dragAction->asyncCtx_);
        napi_escape_handle(env, scope, promiseResult, &promiseResult);
        napi_close_escapable_handle_scope(env, scope);
        return promiseResult;
    }

    std::list<napi_ref>::iterator FindCbList(napi_value cb)
    {
        return std::find_if(cbList_.begin(), cbList_.end(), [env = env_, cb](const napi_ref& item) -> bool {
            bool result = false;
            napi_value refItem;
            napi_get_reference_value(env, item, &refItem);
            napi_strict_equals(env, refItem, cb, &result);
            return result;
        });
    }

private:
    void Initialize(napi_env env, napi_value thisVar)
    {
        env_ = env;
    }

    static size_t ParseArgs(napi_env& env, napi_callback_info& info, napi_value& thisVar, napi_value& cb)
    {
        size_t argc = 2;
        napi_value argv[argCount2] = { 0 };
        void* data = nullptr;
        napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
        if (argc == 0) {
            return argc;
        }
        NAPI_ASSERT_BASE(env, argc > 0, "too few parameter", 0);

        napi_valuetype napiType;
        NAPI_CALL_BASE(env, napi_typeof(env, argv[0], &napiType), 0);
        NAPI_ASSERT_BASE(env, napiType == napi_string, "parameter 1 should be string", 0);
        char type[STR_BUFFER_SIZE] = { 0 };
        size_t len = 0;
        napi_get_value_string_utf8(env, argv[0], type, STR_BUFFER_SIZE, &len);
        NAPI_ASSERT_BASE(env, len < STR_BUFFER_SIZE, "condition string too long", 0);
        NAPI_ASSERT_BASE(env, strcmp("statusChange", type) == 0, "type mismatch('change')", 0);
        if (argc <= 1) {
            return argc;
        }
        NAPI_CALL_BASE(env, napi_typeof(env, argv[1], &napiType), 0);
        NAPI_ASSERT_BASE(env, napiType == napi_function, "type mismatch for parameter 2", 0);
        cb = argv[1];
        return argc;
    }

    static DragAction* ConvertDragAction(napi_env env, napi_value thisVar)
    {
        DragAction* dragAction = nullptr;
        napi_unwrap(env, thisVar, (void**)&dragAction);
        if (dragAction) {
            dragAction->Initialize(env, thisVar);
        }
        return dragAction;
    }

    void StartDragInternal(DragControllerAsyncCtx *dragCtx)
    {
        CHECK_NULL_VOID(dragCtx);
        ParameterType parameterType = getParameterType(dragCtx);
        TAG_LOGI(AceLogTag::ACE_DRAG, "parameter type is %{public}d", static_cast<int32_t>(parameterType));
        if (parameterType == ParameterType::DRAGITEMINFO_ARRAY) {
            OnMultipleComplete(dragCtx);
        } else if (parameterType == ParameterType::MIX) {
            int32_t arrayLenth = static_cast<int32_t>(dragCtx->customBuilderList.size());
            for (auto customBuilderValue: dragCtx->customBuilderList) {
                napi_value cb = nullptr;
                napi_get_reference_value(dragCtx->env, customBuilderValue, &cb);
                GetPixelMapArrayByCustom(dragCtx, cb, arrayLenth);
            }
        } else {
            NapiThrow(dragCtx->env, "parameter parsing failed.", ERROR_CODE_PARAM_INVALID);
        }
    }

    napi_env env_ = nullptr;
    std::list<napi_ref> cbList_;
    DragControllerAsyncCtx* asyncCtx_;
};

DragControllerAsyncCtx::~DragControllerAsyncCtx()
{
    if (!dragAction) {
        dragAction = nullptr;
    }
}

bool IsExecutingWithDragAction(DragControllerAsyncCtx* asyncCtx)
{
    CHECK_NULL_RETURN(asyncCtx, false);
    return (asyncCtx->isArray && asyncCtx->argc == 2);
}

napi_value CreateCallbackErrorValue(napi_env env, int32_t errCode, const std::string& errMsg = "")
{
    napi_value code = nullptr;
    std::string strCode = std::to_string(errCode);
    napi_create_string_utf8(env, strCode.c_str(), strCode.length(), &code);
    napi_value msg = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), errMsg.length(), &msg);
    napi_value error = nullptr;
    napi_create_error(env, code, msg, &error);
    return error;
}

double ConvertToPx(DragControllerAsyncCtx* asyncCtx, const Dimension& dimension, double size)
{
    auto unit = dimension.Unit();
    auto value = dimension.Value();
    if (unit == DimensionUnit::PERCENT) {
        return value * size;
    }
    if (unit == DimensionUnit::NONE || unit == DimensionUnit::PX) {
        return value;
    }
    auto container = AceEngine::Get().GetContainer(asyncCtx->instanceId);
    if (!container) {
        return 0.0;
    }
    auto pipeline = container->GetPipelineContext();
    CHECK_NULL_RETURN(pipeline, 0.0);
    if (unit == DimensionUnit::VP) {
        return value * pipeline->GetDipScale();
    }
    if (unit == DimensionUnit::FP) {
        return value * pipeline->GetDipScale() * pipeline->GetFontScale();
    }
    if (unit == DimensionUnit::LPX) {
        return value * pipeline->GetLogicScale();
    }
    return 0.0;
}

static std::optional<Dimension> HandleDimensionType(napi_value parameterNapi, napi_env env)
{
    size_t ret = 0;
    std::string parameterStr;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, parameterNapi, &valueType);
    Dimension parameter;
    if (valueType == napi_number) {
        double parameterValue;
        napi_get_value_double(env, parameterNapi, &parameterValue);
        parameter.SetValue(parameterValue);
        parameter.SetUnit(DimensionUnit::VP);
    } else if (valueType == napi_string) {
        size_t parameterLen = GetParamLen(env, parameterNapi) + 1;
        std::unique_ptr<char[]> parameterTemp = std::make_unique<char[]>(parameterLen);
        napi_get_value_string_utf8(env, parameterNapi, parameterTemp.get(), parameterLen, &ret);
        parameterStr = parameterTemp.get();
        parameter = StringUtils::StringToDimensionWithUnit(parameterStr, DimensionUnit::VP);
    } else if (valueType == napi_object) {
        ResourceInfo recv;
        if (!ParseResourceParam(env, parameterNapi, recv)) {
            return std::nullopt;
        }
        if (!ParseString(recv, parameterStr)) {
            return std::nullopt;
        }
        parameter = StringUtils::StringToDimensionWithUnit(parameterStr, DimensionUnit::VP);
    } else {
        return std::nullopt;
    }
    return parameter;
}

void CallBackForJs(DragControllerAsyncCtx* asyncCtx, napi_value result)
{
    CHECK_NULL_VOID(asyncCtx);
    CHECK_NULL_VOID(result);

    if (IsExecutingWithDragAction(asyncCtx) && asyncCtx->dragAction) {
        asyncCtx->dragAction->OnNapiCallback(result);
        if (asyncCtx->deferred != nullptr) {
            napi_value promiseResult = nullptr;
            napi_get_undefined(asyncCtx->env, &promiseResult);
            napi_resolve_deferred(asyncCtx->env, asyncCtx->deferred, promiseResult);
        }
    } else {
        napi_value resultVal[PARAMETER_NUM] = { nullptr };
        napi_get_undefined(asyncCtx->env, &resultVal[0]);
        napi_get_undefined(asyncCtx->env, &resultVal[1]);
        resultVal[1] = result;
        if (asyncCtx->callbackRef) {
            napi_value ret = nullptr;
            napi_value napiCallback = nullptr;
            napi_get_reference_value(asyncCtx->env, asyncCtx->callbackRef, &napiCallback);
            napi_call_function(asyncCtx->env, nullptr, napiCallback, PARAMETER_NUM, resultVal, &ret);
            napi_delete_reference(asyncCtx->env, asyncCtx->callbackRef);
        } else {
            napi_resolve_deferred(asyncCtx->env, asyncCtx->deferred, resultVal[1]);
        }
    }
    asyncCtx->deferred = nullptr;
    asyncCtx->hasHandle = false;
}

void GetCallBackDataForJs(DragControllerAsyncCtx* asyncCtx, const DragNotifyMsg& dragNotifyMsg,
    const DragStatus dragStatus, napi_value& result)
{
    CHECK_NULL_VOID(asyncCtx);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(asyncCtx->env, &scope);
    napi_get_undefined(asyncCtx->env, &result);
    auto resultCode = dragNotifyMsg.result;
    napi_create_object(asyncCtx->env, &result);
    napi_value eventNapi = nullptr;
    napi_value globalObj = nullptr;
    napi_value customDragEvent = nullptr;
    napi_create_object(asyncCtx->env, &customDragEvent);
    napi_get_global(asyncCtx->env, &globalObj);
    napi_get_named_property(asyncCtx->env, globalObj, "DragEvent", &customDragEvent);
    napi_status status = napi_new_instance(asyncCtx->env, customDragEvent, 0, nullptr, &eventNapi);
    if (status != napi_ok) {
        TAG_LOGE(AceLogTag::ACE_DRAG,
            "create new instance dragEvent failed, return value is %{public}d", status);
        napi_close_handle_scope(asyncCtx->env, scope);
        return;
    }
    auto localRef = NapiValueToLocalValue(eventNapi);
    if (localRef->IsNull()) {
        TAG_LOGE(AceLogTag::ACE_DRAG, "napi value convert to local value failed.");
        napi_close_handle_scope(asyncCtx->env, scope);
        return;
    }
    auto* jsDragEvent =
        static_cast<Framework::JsDragEvent*>(Local<panda::ObjectRef>(localRef)->GetNativePointerField(0));
    CHECK_NULL_VOID(jsDragEvent);
    auto dragEvent = AceType::MakeRefPtr<DragEvent>();
    CHECK_NULL_VOID(dragEvent);
    dragEvent->SetResult(static_cast<DragRet>(resultCode));
    dragEvent->SetDragBehavior(static_cast<DragBehavior>(dragNotifyMsg.dragBehavior));
    jsDragEvent->SetDragEvent(dragEvent);
    napi_set_named_property(asyncCtx->env, result, "event", eventNapi);

    napi_value extraParamsNapi = nullptr;
    napi_create_string_utf8(
        asyncCtx->env, asyncCtx->extraParams.c_str(), asyncCtx->extraParams.length(), &extraParamsNapi);
    napi_set_named_property(asyncCtx->env, result, "extraParams", extraParamsNapi);

    if (asyncCtx->isArray) {
        napi_value dragStatusValue = nullptr;
        napi_create_int32(asyncCtx->env, static_cast<int32_t>(dragStatus), &dragStatusValue);
        napi_set_named_property(asyncCtx->env, result, "status", dragStatusValue);
    }

    CallBackForJs(asyncCtx, result);
    napi_close_handle_scope(asyncCtx->env, scope);
}

void HandleSuccess(DragControllerAsyncCtx* asyncCtx, const DragNotifyMsg& dragNotifyMsg,
    const DragStatus dragStatus)
{
    TAG_LOGI(AceLogTag::ACE_DRAG, "drag notify message result is %{public}d.", dragNotifyMsg.result);
    CHECK_NULL_VOID(asyncCtx);
    bool hasHandle = false;
    {
        std::lock_guard<std::mutex> lock(asyncCtx->mutex);
        hasHandle = asyncCtx->hasHandle;
        asyncCtx->hasHandle = true;
    }
    if (hasHandle) {
        return;
    }
    auto container = AceEngine::Get().GetContainer(asyncCtx->instanceId);
    CHECK_NULL_VOID(container);
    if (dragStatus == DragStatus::ENDED) {
        auto pipelineContext = container->GetPipelineContext();
        CHECK_NULL_VOID(pipelineContext);
        pipelineContext->ResetDragging();
    }
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostSyncTask(
        [asyncCtx, dragNotifyMsg, dragStatus]() {
            CHECK_NULL_VOID(asyncCtx);
            napi_value dragAndDropInfoValue;
            GetCallBackDataForJs(asyncCtx, dragNotifyMsg, dragStatus, dragAndDropInfoValue);
        },
        TaskExecutor::TaskType::JS, "ArkUIDragHandleSuccess");
}

void HandleFail(DragControllerAsyncCtx* asyncCtx, int32_t errorCode, const std::string& errMsg = "")
{
    CHECK_NULL_VOID(asyncCtx);
    bool hasHandle = false;
    {
        std::lock_guard<std::mutex> lock(asyncCtx->mutex);
        hasHandle = asyncCtx->hasHandle;
        asyncCtx->hasHandle = true;
    }
    if (hasHandle) {
        return;
    }
    napi_value result[2] = { nullptr };
    result[0] = CreateCallbackErrorValue(asyncCtx->env, errorCode, errMsg);
    if (asyncCtx->callbackRef) {
        napi_value ret = nullptr;
        napi_value napiCallback = nullptr;
        napi_get_reference_value(asyncCtx->env, asyncCtx->callbackRef, &napiCallback);
        napi_create_object(asyncCtx->env, &result[1]);
        napi_call_function(asyncCtx->env, nullptr, napiCallback, 2, result, &ret);
        napi_delete_reference(asyncCtx->env, asyncCtx->callbackRef);
    } else {
        napi_reject_deferred(asyncCtx->env, asyncCtx->deferred, result[0]);
    }
}

void HandleOnDragStart(DragControllerAsyncCtx* asyncCtx)
{
    ContainerScope scope(asyncCtx->instanceId);
    auto container = Container::CurrentSafely();
    CHECK_NULL_VOID(container);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [ctx = asyncCtx, context = pipelineContext]() {
            context->OnDragEvent({ ctx->globalX, ctx->globalY }, DragEventAction::DRAG_EVENT_START_FOR_CONTROLLER);
            NG::DragDropFuncWrapper::DecideWhetherToStopDragging(
                { ctx->globalX, ctx->globalY }, ctx->extraParams, ctx->pointerId, ctx->instanceId);
        },
        TaskExecutor::TaskType::UI, "ArkUIDragHandleDragEventStart");
}

void GetShadowInfoArray(DragControllerAsyncCtx* asyncCtx,
    std::vector<Msdp::DeviceStatus::ShadowInfo>& shadowInfos)
{
    std::set<Media::PixelMap*> scaledPixelMaps;
    auto minScaleWidth = GridSystemManager::GetInstance().GetMaxWidthWithColumnType(GridColumnType::DRAG_PANEL);
    for (const auto& pixelMap: asyncCtx->pixelMapList) {
        double scale = 1.0;
        if (!scaledPixelMaps.count(pixelMap.get())) {
            if (pixelMap->GetWidth() > minScaleWidth && asyncCtx->dragPreviewOption.isScaleEnabled) {
                scale = minScaleWidth / pixelMap->GetWidth();
            }
            auto pixelMapScale = asyncCtx->windowScale * scale;
            pixelMap->scale(pixelMapScale, pixelMapScale, Media::AntiAliasingOption::HIGH);
            scaledPixelMaps.insert(pixelMap.get());
        }
        int32_t width = pixelMap->GetWidth();
        int32_t height = pixelMap->GetHeight();
        double x = ConvertToPx(asyncCtx, asyncCtx->touchPoint.GetX(), width);
        double y = ConvertToPx(asyncCtx, asyncCtx->touchPoint.GetY(), height);
        if (!asyncCtx->hasTouchPoint) {
            x = -width * PIXELMAP_WIDTH_RATE;
            y = -height * PIXELMAP_HEIGHT_RATE;
        }
        Msdp::DeviceStatus::ShadowInfo shadowInfo { pixelMap, -x, -y  };
        shadowInfos.push_back(shadowInfo);
    }
}

static void SetIsDragging(const RefPtr<Container>& container, bool isDragging)
{
    CHECK_NULL_VOID(container);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->SetIsDragging(isDragging);
}

bool JudgeCoordinateCanDrag(Msdp::DeviceStatus::ShadowInfo& shadowInfo)
{
    CHECK_NULL_RETURN(shadowInfo.pixelMap, false);
    int32_t x = -shadowInfo.x;
    int32_t y = -shadowInfo.y;
    int32_t width = shadowInfo.pixelMap->GetWidth();
    int32_t height = shadowInfo.pixelMap->GetHeight();
    if (x < 0 || y < 0 || x > width || y > height) {
        return false;
    }
    return true;
}

void EnvelopedDragData(DragControllerAsyncCtx* asyncCtx, std::optional<Msdp::DeviceStatus::DragData>& dragData)
{
    auto container = AceEngine::Get().GetContainer(asyncCtx->instanceId);
    CHECK_NULL_VOID(container);
    auto displayInfo = container->GetDisplayInfo();
    CHECK_NULL_VOID(displayInfo);
    asyncCtx->displayId = displayInfo->GetDisplayId();
    std::vector<Msdp::DeviceStatus::ShadowInfo> shadowInfos;
    GetShadowInfoArray(asyncCtx, shadowInfos);
    if (shadowInfos.empty()) {
        TAG_LOGE(AceLogTag::ACE_DRAG, "shadowInfo array is empty");
        return;
    }
    if (!JudgeCoordinateCanDrag(shadowInfos[0])) {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(asyncCtx->env, &scope);
        HandleFail(asyncCtx, ERROR_CODE_PARAM_INVALID, "touchPoint's coordinate out of range");
        napi_close_handle_scope(asyncCtx->env, scope);
        return;
    }
    auto pointerId = asyncCtx->pointerId;
    std::string udKey;
    std::map<std::string, int64_t> summary;
    int32_t dataSize = 1;
    if (asyncCtx->unifiedData) {
        int32_t ret = UdmfClient::GetInstance()->SetData(asyncCtx->unifiedData, udKey);
        if (ret != 0) {
            TAG_LOGI(AceLogTag::ACE_DRAG, "udmf set data failed, return value is %{public}d", ret);
        } else {
            ret = UdmfClient::GetInstance()->GetSummary(udKey, summary);
            if (ret != 0) {
                TAG_LOGI(AceLogTag::ACE_DRAG, "get summary failed, return value is %{public}d", ret);
            }
        }
        dataSize = static_cast<int32_t>(asyncCtx->unifiedData->GetSize());
    }
    int32_t recordSize = (dataSize != 0 ? dataSize : static_cast<int32_t>(shadowInfos.size()));
    auto badgeNumber = asyncCtx->dragPreviewOption.GetCustomerBadgeNumber();
    if (badgeNumber.has_value()) {
        recordSize = badgeNumber.value();
    }
    auto windowId = container->GetWindowId();
    auto arkExtraInfoJson = JsonUtil::Create(true);
    arkExtraInfoJson->Put("dip_scale", asyncCtx->dipScale);
    NG::DragDropFuncWrapper::UpdateExtraInfo(arkExtraInfoJson, asyncCtx->dragPreviewOption);
    dragData = { shadowInfos, {}, udKey, asyncCtx->extraParams, arkExtraInfoJson->ToString(), asyncCtx->sourceType,
        recordSize, pointerId, asyncCtx->globalX, asyncCtx->globalY,
        asyncCtx->displayId, windowId, true, false, summary };
}

void StartDragService(DragControllerAsyncCtx* asyncCtx)
{
    std::optional<Msdp::DeviceStatus::DragData> dragData;
    EnvelopedDragData(asyncCtx, dragData);
    if (!dragData) {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(asyncCtx->env, &scope);
        HandleFail(asyncCtx, ERROR_CODE_PARAM_INVALID, "did not has any drag data.");
        napi_close_handle_scope(asyncCtx->env, scope);
        return;
    }
    OnDragCallback callback = [asyncCtx](const DragNotifyMsg& dragNotifyMsg) {
        HandleSuccess(asyncCtx, dragNotifyMsg, DragStatus::ENDED);
    };
    NG::DragDropFuncWrapper::SetDraggingPointerAndPressedState(asyncCtx->pointerId, asyncCtx->instanceId);
    int32_t ret = Msdp::DeviceStatus::InteractionManager::GetInstance()->StartDrag(dragData.value(),
        std::make_shared<OHOS::Ace::StartDragListenerImpl>(callback));
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(asyncCtx->env, &scope);
    HandleSuccess(asyncCtx, DragNotifyMsg {}, DragStatus::STARTED);
    napi_close_handle_scope(asyncCtx->env, scope);
    if (ret != 0) {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(asyncCtx->env, &scope);
        HandleFail(asyncCtx, ERROR_CODE_INTERNAL_ERROR, "msdp start drag failed.");
        napi_close_handle_scope(asyncCtx->env, scope);
        return;
    }
    auto container = AceEngine::Get().GetContainer(asyncCtx->instanceId);
    SetIsDragging(container, true);
    TAG_LOGI(AceLogTag::ACE_DRAG, "msdp start drag successfully");
    std::lock_guard<std::mutex> lock(asyncCtx->dragStateMutex);
    if (asyncCtx->dragState == DragState::SENDING) {
        asyncCtx->dragState = DragState::SUCCESS;
        Msdp::DeviceStatus::InteractionManager::GetInstance()->SetDragWindowVisible(true);
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(asyncCtx->env, &scope);
        HandleOnDragStart(asyncCtx);
        napi_close_handle_scope(asyncCtx->env, scope);
    }
}

void OnMultipleComplete(DragControllerAsyncCtx* asyncCtx)
{
    auto container = AceEngine::Get().GetContainer(asyncCtx->instanceId);
    CHECK_NULL_VOID(container);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    auto windowScale = container->GetWindowScale();
    asyncCtx->windowScale = windowScale;
    taskExecutor->PostTask(
        [asyncCtx]() {
            CHECK_NULL_VOID(asyncCtx);
            DragState dragState = DragState::PENDING;
            {
                std::lock_guard<std::mutex> lock(asyncCtx->dragStateMutex);
                if (asyncCtx->dragState == DragState::PENDING) {
                    asyncCtx->dragState = DragState::SENDING;
                }
                dragState = asyncCtx->dragState;
            }
            if (dragState == DragState::REJECT) {
                napi_handle_scope scope = nullptr;
                napi_open_handle_scope(asyncCtx->env, &scope);
                HandleFail(asyncCtx, ERROR_CODE_INTERNAL_ERROR, "drag state is reject.");
                napi_close_handle_scope(asyncCtx->env, scope);
                return;
            }
            StartDragService(asyncCtx);
        },
        TaskExecutor::TaskType::JS, "ArkUIDragMultipleComplete");
}

void OnComplete(DragControllerAsyncCtx* asyncCtx)
{
    auto container = AceEngine::Get().GetContainer(asyncCtx->instanceId);
    CHECK_NULL_VOID(container);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    auto windowScale = container->GetWindowScale();
    asyncCtx->windowScale = windowScale;
    auto displayInfo = container->GetDisplayInfo();
    CHECK_NULL_VOID(displayInfo);
    asyncCtx->displayId = displayInfo->GetDisplayId();
    taskExecutor->PostTask(
        [asyncCtx]() {
            CHECK_NULL_VOID(asyncCtx);
            DragState dragState = DragState::PENDING;
            {
                std::lock_guard<std::mutex> lock(asyncCtx->dragStateMutex);
                if (asyncCtx->dragState == DragState::PENDING) {
                    asyncCtx->dragState = DragState::SENDING;
                }
                dragState = asyncCtx->dragState;
            }
            if (dragState == DragState::REJECT) {
                napi_handle_scope scope = nullptr;
                napi_open_handle_scope(asyncCtx->env, &scope);
                HandleFail(asyncCtx, ERROR_CODE_INTERNAL_ERROR, "drag state is reject.");
                napi_close_handle_scope(asyncCtx->env, scope);
                return;
            }
            CHECK_NULL_VOID(asyncCtx->pixelMap);
            int32_t dataSize = 1;
            auto pointerId = asyncCtx->pointerId;
            std::string udKey;
            std::map<std::string, int64_t> summary;
            if (asyncCtx->unifiedData) {
                int32_t ret = UdmfClient::GetInstance()->SetData(asyncCtx->unifiedData, udKey);
                if (ret != 0) {
                    TAG_LOGI(AceLogTag::ACE_DRAG, "udmf set data failed, return value is %{public}d", ret);
                } else {
                    ret = UdmfClient::GetInstance()->GetSummary(udKey, summary);
                    if (ret != 0) {
                        TAG_LOGI(AceLogTag::ACE_DRAG, "get summary failed, return value is %{public}d", ret);
                    }
                }
                dataSize = static_cast<int32_t>(asyncCtx->unifiedData->GetSize());
            }
            auto badgeNumber = asyncCtx->dragPreviewOption.GetCustomerBadgeNumber();
            if (badgeNumber.has_value()) {
                dataSize = badgeNumber.value();
            }
            double scale = 1.0;
            auto minScaleWidth = GridSystemManager::GetInstance().GetMaxWidthWithColumnType(GridColumnType::DRAG_PANEL);
            if (asyncCtx->pixelMap->GetWidth() > minScaleWidth && asyncCtx->dragPreviewOption.isScaleEnabled) {
                scale = minScaleWidth / asyncCtx->pixelMap->GetWidth();
            }
            auto pixelMapScale = asyncCtx->windowScale * scale;
            asyncCtx->pixelMap->scale(pixelMapScale, pixelMapScale, Media::AntiAliasingOption::HIGH);
            int32_t width = asyncCtx->pixelMap->GetWidth();
            int32_t height = asyncCtx->pixelMap->GetHeight();
            double x = ConvertToPx(asyncCtx, asyncCtx->touchPoint.GetX(), width);
            double y = ConvertToPx(asyncCtx, asyncCtx->touchPoint.GetY(), height);
            if (!asyncCtx->hasTouchPoint) {
                x = -width * PIXELMAP_WIDTH_RATE;
                y = -height * PIXELMAP_HEIGHT_RATE;
            } else if (x < 0 || y < 0 || x > static_cast<double>(width) || y > static_cast<double>(height)) {
                napi_handle_scope scope = nullptr;
                napi_open_handle_scope(asyncCtx->env, &scope);
                HandleFail(asyncCtx, ERROR_CODE_PARAM_INVALID, "touchPoint's coordinate out of range");
                napi_close_handle_scope(asyncCtx->env, scope);
                return;
            }
            auto container = AceEngine::Get().GetContainer(asyncCtx->instanceId);
            auto arkExtraInfoJson = JsonUtil::Create(true);
            arkExtraInfoJson->Put("dip_scale", asyncCtx->dipScale);
            NG::DragDropFuncWrapper::UpdateExtraInfo(arkExtraInfoJson, asyncCtx->dragPreviewOption);
            auto windowId = container->GetWindowId();
            Msdp::DeviceStatus::ShadowInfo shadowInfo { asyncCtx->pixelMap, -x, -y };
            Msdp::DeviceStatus::DragData dragData { { shadowInfo }, {}, udKey, asyncCtx->extraParams,
                arkExtraInfoJson->ToString(), asyncCtx->sourceType, dataSize, pointerId, asyncCtx->globalX,
                asyncCtx->globalY, asyncCtx->displayId, windowId, true, false, summary };

            OnDragCallback callback = [asyncCtx](const DragNotifyMsg& dragNotifyMsg) {
                HandleSuccess(asyncCtx, dragNotifyMsg, DragStatus::ENDED);
            };
            NG::DragDropFuncWrapper::SetDraggingPointerAndPressedState(asyncCtx->pointerId, asyncCtx->instanceId);
            int32_t ret = Msdp::DeviceStatus::InteractionManager::GetInstance()->StartDrag(dragData,
                std::make_shared<OHOS::Ace::StartDragListenerImpl>(callback));
            if (ret != 0) {
                napi_handle_scope scope = nullptr;
                napi_open_handle_scope(asyncCtx->env, &scope);
                HandleFail(asyncCtx, ERROR_CODE_INTERNAL_ERROR, "msdp start drag failed.");
                napi_close_handle_scope(asyncCtx->env, scope);
                return;
            }
            SetIsDragging(container, true);
            TAG_LOGI(AceLogTag::ACE_DRAG, "msdp start drag successfully");
            {
                std::lock_guard<std::mutex> lock(asyncCtx->dragStateMutex);
                if (asyncCtx->dragState == DragState::SENDING) {
                    asyncCtx->dragState = DragState::SUCCESS;
                    Msdp::DeviceStatus::InteractionManager::GetInstance()->SetDragWindowVisible(true);
                    napi_handle_scope scope = nullptr;
                    napi_open_handle_scope(asyncCtx->env, &scope);
                    HandleOnDragStart(asyncCtx);
                    napi_close_handle_scope(asyncCtx->env, scope);
                }
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIDragComplete");
}

bool ParseTouchPoint(DragControllerAsyncCtx* asyncCtx, napi_valuetype& valueType)
{
    napi_value touchPointNapi = nullptr;
    napi_get_named_property(asyncCtx->env, asyncCtx->argv[1], "touchPoint", &touchPointNapi);
    napi_typeof(asyncCtx->env, touchPointNapi, &valueType);
    if (valueType == napi_object) {
        napi_value xNapi = nullptr;
        napi_get_named_property(asyncCtx->env, touchPointNapi, "x", &xNapi);
        std::optional<Dimension> dx = HandleDimensionType(xNapi, asyncCtx->env);
        if (dx == std::nullopt) {
            return false;
        }
        napi_value yNapi = nullptr;
        napi_get_named_property(asyncCtx->env, touchPointNapi, "y", &yNapi);
        std::optional<Dimension> dy = HandleDimensionType(yNapi, asyncCtx->env);
        if (dy == std::nullopt) {
            return false;
        }
        asyncCtx->touchPoint = DimensionOffset(dx.value(), dy.value());
    } else {
        return false;
    }
    return true;
}

bool ParseDragItemInfoParam(DragControllerAsyncCtx* asyncCtx, std::string& errMsg)
{
    CHECK_NULL_RETURN(asyncCtx, false);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(asyncCtx->env, asyncCtx->argv[0], &valueType);
    if (valueType == napi_function) {
        asyncCtx->customBuilder = asyncCtx->argv[0];
        return true;
    }

    if (valueType != napi_object) {
        errMsg = "The type of first parameter is incorrect.";
        return false;
    }
    // Parse the DragItemInfo
    napi_value pixelMapValue;
    napi_get_named_property(asyncCtx->env, asyncCtx->argv[0], "pixelMap", &pixelMapValue);
    PixelMapNapiEntry pixelMapNapiEntry = Framework::JsEngine::GetPixelMapNapiEntry();
    if (pixelMapNapiEntry == nullptr) {
        TAG_LOGW(AceLogTag::ACE_DRAG, "failed to parse pixelMap from the first argument");
    } else {
        void* pixmapPtrAddr = pixelMapNapiEntry(asyncCtx->env, pixelMapValue);
        if (pixmapPtrAddr == nullptr) {
            napi_get_named_property(asyncCtx->env, asyncCtx->argv[0], "builder", &(asyncCtx->customBuilder));
            napi_typeof(asyncCtx->env, asyncCtx->customBuilder, &valueType);
            if (valueType != napi_function) {
                errMsg = "The first parameter is not a pixelMap or customBuilder.";
                return false;
            }
        } else {
            asyncCtx->pixelMap = *(reinterpret_cast<std::shared_ptr<Media::PixelMap>*>(pixmapPtrAddr));
        }
    }

    napi_value extraInfoValue;
    napi_get_named_property(asyncCtx->env, asyncCtx->argv[0], "extraInfo", &extraInfoValue);
    napi_typeof(asyncCtx->env, extraInfoValue, &valueType);
    if (valueType == napi_string) {
        GetNapiString(asyncCtx->env, extraInfoValue, asyncCtx->extraParams, valueType);
    } else if (valueType != napi_undefined) {
        errMsg = "The type of extraInfo of the first parameter is incorrect.";
        return false;
    }
    return true;
}

bool GetPixelMapByCustom(DragControllerAsyncCtx* asyncCtx)
{
    CHECK_NULL_RETURN(asyncCtx, false);
    napi_escapable_handle_scope scope = nullptr;
    napi_open_escapable_handle_scope(asyncCtx->env, &scope);
    auto delegate = EngineHelper::GetCurrentDelegateSafely();
    if (!delegate) {
        NapiThrow(asyncCtx->env, "ace engine delegate is null", ERROR_CODE_INTERNAL_ERROR);
        napi_close_escapable_handle_scope(asyncCtx->env, scope);
        return false;
    }
    auto callback = [asyncCtx](std::shared_ptr<Media::PixelMap> pixelMap, int32_t errCode,
        std::function<void()> finishCallback) {
        if (finishCallback) {
            finishCallback();
        }
        CHECK_NULL_VOID(pixelMap);
        CHECK_NULL_VOID(asyncCtx);
        asyncCtx->errCode = errCode;
        asyncCtx->pixelMap = std::move(pixelMap);
        OnComplete(asyncCtx);
    };
    auto builder = [build = asyncCtx->customBuilder, env = asyncCtx->env] {
        napi_call_function(env, nullptr, build, 0, nullptr, nullptr);
    };
    delegate->CreateSnapshot(builder, callback, true);
    napi_close_escapable_handle_scope(asyncCtx->env, scope);
    return true;
}

bool GetPixelMapArrayByCustom(DragControllerAsyncCtx* asyncCtx, napi_value customBuilder, int arrayLength)
{
    CHECK_NULL_RETURN(asyncCtx, false);
    napi_escapable_handle_scope scope = nullptr;
    napi_open_escapable_handle_scope(asyncCtx->env, &scope);

    auto delegate = EngineHelper::GetCurrentDelegateSafely();
    if (!delegate) {
        NapiThrow(asyncCtx->env, "ace engine delegate is null", ERROR_CODE_INTERNAL_ERROR);
        napi_close_escapable_handle_scope(asyncCtx->env, scope);
        return false;
    }
    auto callback = [asyncCtx, arrayLength](
        std::shared_ptr<Media::PixelMap> pixelMap, int32_t errCode, std::function<void()> finishCallback) {
        if (finishCallback) {
            finishCallback();
        }
        CHECK_NULL_VOID(pixelMap);
        CHECK_NULL_VOID(asyncCtx);
        asyncCtx->errCode = errCode;
        asyncCtx->pixelMapList.push_back(std::move(pixelMap));
        asyncCtx->parseBuilderCount++;
        if (asyncCtx->parseBuilderCount == arrayLength) {
            OnMultipleComplete(asyncCtx);
        }
    };
    auto builder = [build = customBuilder, env = asyncCtx->env] {
        napi_call_function(env, nullptr, build, 0, nullptr, nullptr);
    };
    delegate->CreateSnapshot(builder, callback, true);
    napi_close_escapable_handle_scope(asyncCtx->env, scope);
    return true;
}

bool ParseExtraInfo(DragControllerAsyncCtx* asyncCtx, std::string& errMsg, napi_value element)
{
    CHECK_NULL_RETURN(asyncCtx, false);
    napi_value extraInfoValue;
    napi_get_named_property(asyncCtx->env, element, "extraInfo", &extraInfoValue);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(asyncCtx->env, extraInfoValue, &valueType);
    if (valueType != napi_string) {
        errMsg = "The type of extraInfo of the first parameter is incorrect.";
        return false;
    }
    GetNapiString(asyncCtx->env, extraInfoValue, asyncCtx->extraParams, valueType);
    return true;
}

bool ParsePixelMapAndBuilder(DragControllerAsyncCtx* asyncCtx, std::string& errMsg, napi_value element)
{
    CHECK_NULL_RETURN(asyncCtx, false);
    napi_value pixelMapValue;
    napi_get_named_property(asyncCtx->env, element, "pixelMap", &pixelMapValue);
    PixelMapNapiEntry pixelMapNapiEntry = Framework::JsEngine::GetPixelMapNapiEntry();
    if (pixelMapNapiEntry == nullptr) {
        TAG_LOGW(AceLogTag::ACE_DRAG, "failed to parse pixelMap from the first argument");
        napi_value customBuilderValue;
        napi_get_named_property(asyncCtx->env, element, "builder", &customBuilderValue);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(asyncCtx->env, customBuilderValue, &valueType);
        if (valueType != napi_function) {
            errMsg = "The type of customBuilder of the first parameter is incorrect.";
            return false;
        }
        napi_ref ref = nullptr;
        napi_create_reference(asyncCtx->env, customBuilderValue, 1, &ref);
        asyncCtx->customBuilderList.push_back(ref);
    } else {
        void* pixmapPtrAddr = pixelMapNapiEntry(asyncCtx->env, pixelMapValue);
        if (pixmapPtrAddr == nullptr) {
            TAG_LOGW(AceLogTag::ACE_DRAG, "the pixelMap parsed from the first argument is null");
            return false;
        } else {
            asyncCtx->pixelMapList.push_back(*(reinterpret_cast<std::shared_ptr<Media::PixelMap>*>(pixmapPtrAddr)));
        }
    }
    return true;
}

bool ParseDragItemListInfoParam(DragControllerAsyncCtx* asyncCtx, std::string& errMsg)
{
    CHECK_NULL_RETURN(asyncCtx, false);
    bool isParseSucess;
    uint32_t arrayLength = 0;
    napi_get_array_length(asyncCtx->env, asyncCtx->argv[0], &arrayLength);
    for (size_t i = 0; i < arrayLength; i++) {
        bool hasElement = false;
        napi_has_element(asyncCtx->env, asyncCtx->argv[0], i, &hasElement);
        napi_value element = nullptr;
        napi_get_element(asyncCtx->env, asyncCtx->argv[0], i, &element);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(asyncCtx->env, element, &valueType);
        if (valueType == napi_function) {
            napi_ref ref = nullptr;
            napi_create_reference(asyncCtx->env, element, 1, &ref);
            asyncCtx->customBuilderList.push_back(ref);
            isParseSucess = true;
            continue;
        }
        if (valueType != napi_object) {
            errMsg = "The type of first parameter is incorrect";
            isParseSucess = false;
            break;
        }
        if (!ParseExtraInfo(asyncCtx, errMsg, element)) {
            errMsg = "The type of first parameter is incorrect by extraInfo";
            isParseSucess = false;
            break;
        }
        if (!ParsePixelMapAndBuilder(asyncCtx, errMsg, element)) {
            errMsg = "The type of first parameter is incorrect by pixelMap or builder";
            isParseSucess = false;
            break;
        }
        isParseSucess = true;
        continue;
    }
    return isParseSucess;
}

bool ParseDragParam(DragControllerAsyncCtx* asyncCtx, std::string& errMsg)
{
    CHECK_NULL_RETURN(asyncCtx, false);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(asyncCtx->env, asyncCtx->argv[0], &valueType);
    if (valueType == napi_function) {
        asyncCtx->customBuilder = asyncCtx->argv[0];
        return true;
    }
    if (valueType != napi_object) {
        errMsg = "The type of first parameter is incorrect.";
        return false;
    }
    
    bool isArray = false;
    napi_is_array(asyncCtx->env, asyncCtx->argv[0], &isArray);
    if (isArray) {
        TAG_LOGI(AceLogTag::ACE_DRAG, "drag controller is multi object drag.");
        asyncCtx->isArray = true;
        return ParseDragItemListInfoParam(asyncCtx, errMsg);
    }
    asyncCtx->isArray = false;
    return ParseDragItemInfoParam(asyncCtx, errMsg);
}

bool ApplyPreviewOptionsFromModifier(
    DragControllerAsyncCtx* asyncCtx, napi_value modifierObj, NG::DragPreviewOption& option)
{
    CHECK_NULL_RETURN(asyncCtx, false);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(asyncCtx->env, modifierObj, &valueType);
    if (valueType == napi_undefined) {
        return true;
    }
    if (valueType != napi_object) {
        return false;
    }

    napi_value globalObj = nullptr;
    napi_get_global(asyncCtx->env, &globalObj);
    napi_value globalFunc = nullptr;
    napi_get_named_property(asyncCtx->env, globalObj, "applyImageModifierToNode", &globalFunc);
    napi_typeof(asyncCtx->env, globalFunc, &valueType);
    if (globalFunc == nullptr || valueType != napi_function) {
        return false;
    }

    auto applyOnNodeSync =
        [modifierObj, globalFunc, asyncCtx](WeakPtr<NG::FrameNode> frameNode) {
            // convert nodeptr to js value
            auto nodePtr = frameNode.Upgrade();
            const size_t size = 64; // fake size for gc
            napi_value nodeJsValue = nullptr;
            napi_create_external_with_size(
                asyncCtx->env, static_cast<void*>(AceType::RawPtr(nodePtr)),
                [](napi_env env, void* data, void* hint) {}, static_cast<void*>(AceType::RawPtr(nodePtr)),
                &nodeJsValue, size);
            if (nodeJsValue == nullptr) {
                return;
            }
            // apply modifier
            napi_value ret;
            napi_value params[2];
            params[0] = modifierObj;
            params[1] = nodeJsValue;
            napi_call_function(asyncCtx->env, nullptr, globalFunc, 2, params, &ret);
        };

    NG::DragDropFuncWrapper::UpdateDragPreviewOptionsFromModifier(applyOnNodeSync, option);
    return true;
}

bool GetNamedPropertyModifier(
    DragControllerAsyncCtx* asyncCtx, napi_value previewOptionsNApi, std::string& errMsg)
{
    napi_value modifierObj = nullptr;
    napi_get_named_property(asyncCtx->env, previewOptionsNApi, "modifier", &modifierObj);
    if (!ApplyPreviewOptionsFromModifier(asyncCtx, modifierObj, asyncCtx->dragPreviewOption)) {
        errMsg = "apply modifier failed.";
        return false;
    }
    return true;
}

bool SetDragPreviewOptionMode(DragControllerAsyncCtx* asyncCtx, napi_value& modeNApi,
    std::string& errMsg, bool& isAuto)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(asyncCtx->env, modeNApi, &valueType);
    if (valueType == napi_undefined) {
        return true;
    } else if (valueType != napi_number) {
        errMsg = "mode type is wrong";
        return false;
    } else if (isAuto) {
        return true;
    }

    int32_t dragPreviewMode = 0;
    napi_get_value_int32(asyncCtx->env, modeNApi, &dragPreviewMode);
    auto mode = static_cast<NG::DragPreviewMode>(dragPreviewMode);
    switch (mode) {
        case NG::DragPreviewMode::AUTO:
            asyncCtx->dragPreviewOption.ResetDragPreviewMode();
            isAuto = true;
            break;
        case NG::DragPreviewMode::DISABLE_SCALE:
            asyncCtx->dragPreviewOption.isScaleEnabled = false;
            break;
        case NG::DragPreviewMode::ENABLE_DEFAULT_SHADOW:
            asyncCtx->dragPreviewOption.isDefaultShadowEnabled = true;
            break;
        case NG::DragPreviewMode::ENABLE_DEFAULT_RADIUS:
            asyncCtx->dragPreviewOption.isDefaultRadiusEnabled = true;
            break;
        default:
            break;
    }
    return true;
}

bool ParseDragPreviewMode(DragControllerAsyncCtx* asyncCtx, napi_value& previewOptionsNApi, std::string& errMsg)
{
    napi_value modeNApi = nullptr;
    napi_get_named_property(asyncCtx->env, previewOptionsNApi, "mode", &modeNApi);
    bool isArray = false;
    bool isAuto = false;
    napi_is_array(asyncCtx->env, modeNApi, &isArray);
    if (isArray) {
        uint32_t arrayLength = 0;
        napi_get_array_length(asyncCtx->env, modeNApi, &arrayLength);
        for (size_t i = 0; i < arrayLength; i++) {
            bool hasElement = false;
            napi_has_element(asyncCtx->env, modeNApi, i, &hasElement);
            if (!hasElement) {
                continue;
            }
            napi_value element = nullptr;
            napi_get_element(asyncCtx->env, modeNApi, i, &element);
            if (!SetDragPreviewOptionMode(asyncCtx, element, errMsg, isAuto)) {
                return false;
            }
        }
    } else if (!SetDragPreviewOptionMode(asyncCtx, modeNApi, errMsg, isAuto)) {
        return false;
    }
    NG::DragDropFuncWrapper::UpdatePreviewOptionDefaultAttr(asyncCtx->dragPreviewOption);
    return true;
}

void GetCurrentDipScale(DragControllerAsyncCtx* asyncCtx)
{
    auto container = AceEngine::Get().GetContainer(asyncCtx->instanceId);
    CHECK_NULL_VOID(container);
    auto pipeline = container->GetPipelineContext();
    CHECK_NULL_VOID(pipeline);
    asyncCtx->dipScale = pipeline->GetDipScale();
}

bool ParsePreviewOptions(
    DragControllerAsyncCtx* asyncCtx, napi_valuetype& valueType, std::string& errMsg)
{
    CHECK_NULL_RETURN(asyncCtx, false);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(asyncCtx->env, &scope);
    asyncCtx->dragPreviewOption.isNumber = false;
    asyncCtx->dragPreviewOption.isShowBadge = true;
    napi_value previewOptionsNApi = nullptr;
    napi_get_named_property(asyncCtx->env, asyncCtx->argv[1], "previewOptions", &previewOptionsNApi);
    napi_typeof(asyncCtx->env, previewOptionsNApi, &valueType);
    if (valueType == napi_object) {
        if (!ParseDragPreviewMode(asyncCtx, previewOptionsNApi, errMsg)) {
            napi_close_handle_scope(asyncCtx->env, scope);
            return false;
        }

        napi_value numberBadgeNApi = nullptr;
        napi_get_named_property(asyncCtx->env, previewOptionsNApi, "numberBadge", &numberBadgeNApi);
        napi_typeof(asyncCtx->env, numberBadgeNApi, &valueType);
        if (valueType == napi_number) {
            asyncCtx->dragPreviewOption.isNumber = true;
            napi_get_value_int32(asyncCtx->env, numberBadgeNApi, &asyncCtx->dragPreviewOption.badgeNumber);
        } else if (valueType == napi_boolean) {
            asyncCtx->dragPreviewOption.isNumber = false;
            napi_get_value_bool(asyncCtx->env, numberBadgeNApi, &asyncCtx->dragPreviewOption.isShowBadge);
        } else if (valueType != napi_undefined) {
            errMsg = "numberBadge type is wrong.";
            napi_close_handle_scope(asyncCtx->env, scope);
            return false;
        }

        if (!(GetNamedPropertyModifier(asyncCtx, previewOptionsNApi, errMsg))) {
            napi_close_handle_scope(asyncCtx->env, scope);
            return false;
        }
    } else if (valueType != napi_undefined) {
        errMsg = "previewOptions type is wrong";
        napi_close_handle_scope(asyncCtx->env, scope);
        return false;
    }
    napi_close_handle_scope(asyncCtx->env, scope);
    return true;
}

bool ParseDragInfoParam(DragControllerAsyncCtx* asyncCtx, std::string& errMsg)
{
    CHECK_NULL_RETURN(asyncCtx, false);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(asyncCtx->env, asyncCtx->argv[1], &valueType);
    if (valueType != napi_object) {
        errMsg = "The type of second parameter is incorrect.";
        return false;
    }
    napi_value pointerIdNApi = nullptr;
    napi_status status = napi_ok;
    napi_get_named_property(asyncCtx->env, asyncCtx->argv[1], "pointerId", &pointerIdNApi);
    napi_typeof(asyncCtx->env, pointerIdNApi, &valueType);
    if (valueType != napi_number) {
        errMsg = "pointerId which type is number must be given";
        return false;
    }
    status = napi_get_value_int32(asyncCtx->env, pointerIdNApi, &asyncCtx->pointerId);
    if (status != napi_ok) {
        errMsg = "parse pointerId fail";
        return false;
    }

    napi_value dataNApi = nullptr;
    napi_get_named_property(asyncCtx->env, asyncCtx->argv[1], "data", &dataNApi);
    napi_typeof(asyncCtx->env, dataNApi, &valueType);
    if (valueType == napi_object) {
        asyncCtx->unifiedData = UdmfClient::GetInstance()->TransformUnifiedData(dataNApi);
    } else if (valueType != napi_undefined) {
        errMsg = "data's type is wrong";
        return false;
    }

    napi_value extraParamsNApi = nullptr;
    napi_get_named_property(asyncCtx->env, asyncCtx->argv[1], "extraParams", &extraParamsNApi);
    napi_typeof(asyncCtx->env, extraParamsNApi, &valueType);
    if (valueType == napi_string) {
        GetNapiString(asyncCtx->env, extraParamsNApi, asyncCtx->extraParams, valueType);
    } else if (valueType != napi_undefined) {
        errMsg = "extraParams's type is wrong";
        return false;
    }

    if (!ParsePreviewOptions(asyncCtx, valueType, errMsg)) {
        return false;
    }

    GetCurrentDipScale(asyncCtx);
    asyncCtx->hasTouchPoint = ParseTouchPoint(asyncCtx, valueType);
    return true;
}

bool CheckAndParseParams(DragControllerAsyncCtx* asyncCtx, std::string& errMsg)
{
    // Check the number of the argument
    CHECK_NULL_RETURN(asyncCtx, false);
    if ((asyncCtx->argc != 2) && (asyncCtx->argc != argCount3)) {
        errMsg = "The number of parameters must be 2 or 3.";
        return false;
    }

    // Check and parse the first parameter
    if (!ParseDragParam(asyncCtx, errMsg)) {
        return false;
    }

    // Check and parse the second parameter
    return ParseDragInfoParam(asyncCtx, errMsg);
}

void CreateCallback(DragControllerAsyncCtx* asyncCtx, napi_value* result)
{
    CHECK_NULL_VOID(asyncCtx);
    if (asyncCtx->argc == argCount3) {
        // Create the JsCallback
        napi_create_reference(asyncCtx->env, asyncCtx->argv[2], 1, &asyncCtx->callbackRef);
    }
    if (!asyncCtx->callbackRef) {
        // Create the promise
        napi_create_promise(asyncCtx->env, &asyncCtx->deferred, result);
    } else {
        napi_get_undefined(asyncCtx->env, result);
    }
}

void InitializeDragControllerCtx(napi_env env, napi_callback_info info, DragControllerAsyncCtx* asyncCtx)
{
    CHECK_NULL_VOID(asyncCtx);
    napi_value thisVar = nullptr;
    void* data = nullptr;
    asyncCtx->instanceId = Container::CurrentIdSafely();
    asyncCtx->env = env;
    // get arguments from JS
    napi_get_cb_info(asyncCtx->env, info, &(asyncCtx->argc), asyncCtx->argv, &thisVar, &data);
}

ParameterType getParameterType(DragControllerAsyncCtx* asyncCtx)
{
    CHECK_NULL_RETURN(asyncCtx, ParameterType::ERROR);
    if (asyncCtx->pixelMap != nullptr) {
        return ParameterType::DRAGITEMINFO;
    }
    if (asyncCtx->customBuilder != nullptr) {
        return ParameterType::CUSTOMBUILDER;
    }
    if (!asyncCtx->pixelMapList.empty() && asyncCtx->customBuilderList.empty()) {
        return ParameterType::DRAGITEMINFO_ARRAY;
    }
    if (!asyncCtx->customBuilderList.empty() || !asyncCtx->pixelMapList.empty()) {
        return ParameterType::MIX;
    } else {
        return ParameterType::ERROR;
    }
}

bool ConfirmCurPointerEventInfo(DragControllerAsyncCtx *asyncCtx, const RefPtr<Container>& container)
{
    CHECK_NULL_RETURN(asyncCtx, false);
    CHECK_NULL_RETURN(container, false);
    StopDragCallback stopDragCallback = [asyncCtx, container]() {
        CHECK_NULL_VOID(asyncCtx);
        CHECK_NULL_VOID(container);
        bool needPostStopDrag = false;
        {
            std::lock_guard<std::mutex> lock(asyncCtx->dragStateMutex);
            needPostStopDrag = (asyncCtx->dragState == DragState::SENDING);
            asyncCtx->dragState = DragState::REJECT;
        }
        if (needPostStopDrag) {
            auto taskExecutor = container->GetTaskExecutor();
            CHECK_NULL_VOID(taskExecutor);
            auto windowId = container->GetWindowId();
            taskExecutor->PostTask(
                [asyncCtx, windowId]() {
                    CHECK_NULL_VOID(asyncCtx);
                    napi_handle_scope scope = nullptr;
                    napi_open_handle_scope(asyncCtx->env, &scope);
                    HandleFail(asyncCtx, ERROR_CODE_INTERNAL_ERROR, "drag state error, stop drag.");
                    napi_close_handle_scope(asyncCtx->env, scope);
                    TAG_LOGI(AceLogTag::ACE_DRAG,
                        "drag state is reject, stop drag, windowId is %{public}d.", windowId);
                    Msdp::DeviceStatus::DragDropResult dropResult { Msdp::DeviceStatus::DragResult::DRAG_CANCEL, false,
                        windowId, Msdp::DeviceStatus::DragBehavior::UNKNOWN };
                    Msdp::DeviceStatus::InteractionManager::GetInstance()->StopDrag(dropResult);
                    Msdp::DeviceStatus::InteractionManager::GetInstance()->SetDragWindowVisible(false);
                },
                TaskExecutor::TaskType::JS, "ArkUIDragStop");
        }
    };
    int32_t sourceTool = -1;
    bool getPointSuccess = container->GetCurPointerEventInfo(
        asyncCtx->pointerId, asyncCtx->globalX, asyncCtx->globalY, asyncCtx->sourceType,
        sourceTool, std::move(stopDragCallback));
    if (asyncCtx->sourceType == SOURCE_TYPE_MOUSE) {
        asyncCtx->pointerId = MOUSE_POINTER_ID;
    } else if (asyncCtx->sourceType == SOURCE_TYPE_TOUCH && sourceTool == SOURCE_TOOL_PEN) {
        asyncCtx->pointerId = PEN_POINTER_ID;
    }
    return getPointSuccess;
}

static bool CheckDragging(const RefPtr<Container>& container)
{
    CHECK_NULL_RETURN(container, false);
    auto pipelineContext = container->GetPipelineContext();
    if (!pipelineContext || !pipelineContext->IsDragging()) {
        return false;
    }
    return true;
}

static napi_value JSExecuteDrag(napi_env env, napi_callback_info info)
{
    napi_escapable_handle_scope scope = nullptr;
    napi_open_escapable_handle_scope(env, &scope);

    auto dragAsyncContext = new (std::nothrow) DragControllerAsyncCtx();
    if (dragAsyncContext == nullptr) {
        NapiThrow(env, "create drag controller async context failed.", ERROR_CODE_INTERNAL_ERROR);
        napi_close_escapable_handle_scope(env, scope);
        return nullptr;
    }
    InitializeDragControllerCtx(env, info, dragAsyncContext);

    std::string errMsg;
    if (!CheckAndParseParams(dragAsyncContext, errMsg)) {
        NapiThrow(env, errMsg, ERROR_CODE_PARAM_INVALID);
        napi_close_escapable_handle_scope(env, scope);
        return nullptr;
    }
    napi_value result = nullptr;
    CreateCallback(dragAsyncContext, &result);
    auto container = AceEngine::Get().GetContainer(dragAsyncContext->instanceId);
    if (!container) {
        NapiThrow(env, "get container failed.", ERROR_CODE_INTERNAL_ERROR);
        napi_close_escapable_handle_scope(env, scope);
        return nullptr;
    }
    if (CheckDragging(container)) {
        NapiThrow(env, "only one drag is allowed at the same time", ERROR_CODE_INTERNAL_ERROR);
        napi_close_escapable_handle_scope(env, scope);
        return nullptr;
    }
    auto getPointSuccess = ConfirmCurPointerEventInfo(dragAsyncContext, container);
    if (!getPointSuccess) {
        NapiThrow(env, "confirm current point info failed.", ERROR_CODE_INTERNAL_ERROR);
        napi_escape_handle(env, scope, result, &result);
        napi_close_escapable_handle_scope(env, scope);
        return result;
    }
    ParameterType parameterType = getParameterType(dragAsyncContext);
    if (parameterType == ParameterType::DRAGITEMINFO) {
        OnComplete(dragAsyncContext);
    } else if (parameterType == ParameterType::CUSTOMBUILDER) {
        GetPixelMapByCustom(dragAsyncContext);
    } else {
        NapiThrow(env, "parameter parsing error.", ERROR_CODE_PARAM_INVALID);
    }
    napi_escape_handle(env, scope, result, &result);
    napi_close_escapable_handle_scope(env, scope);
    return result;
}

static napi_value JSCreateDragAction(napi_env env, napi_callback_info info)
{
    napi_escapable_handle_scope scope = nullptr;
    napi_open_escapable_handle_scope(env, &scope);

    auto dragAsyncContext = new (std::nothrow) DragControllerAsyncCtx();
    if (dragAsyncContext == nullptr) {
        NapiThrow(env, "create drag controller async context failed.", ERROR_CODE_INTERNAL_ERROR);
        napi_close_escapable_handle_scope(env, scope);
        return nullptr;
    }
    InitializeDragControllerCtx(env, info, dragAsyncContext);

    std::string errMsg = "";
    if (!CheckAndParseParams(dragAsyncContext, errMsg)) {
        NapiThrow(env, errMsg, ERROR_CODE_PARAM_INVALID);
        napi_close_escapable_handle_scope(env, scope);
        return nullptr;
    }

    auto container = AceEngine::Get().GetContainer(dragAsyncContext->instanceId);
    if (!container) {
        NapiThrow(env, "get container failed.", ERROR_CODE_INTERNAL_ERROR);
        napi_close_escapable_handle_scope(env, scope);
        return nullptr;
    }

    if (CheckDragging(container)) {
        NapiThrow(env, "only one dragAction is allowed at the same time", ERROR_CODE_INTERNAL_ERROR);
        napi_close_escapable_handle_scope(env, scope);
        return nullptr;
    }

    auto getPointSuccess = ConfirmCurPointerEventInfo(dragAsyncContext, container);
    if (!getPointSuccess) {
        NapiThrow(env, "confirm pointer info failed", ERROR_CODE_PARAM_INVALID);
        napi_close_escapable_handle_scope(env, scope);
        return nullptr;
    }

    napi_value result = nullptr;
    napi_create_object(env, &result);
    DragAction* dragAction = new (std::nothrow) DragAction(dragAsyncContext);
    dragAction->NapiSerializer(env, result);
    dragAsyncContext->dragAction = dragAction;
    napi_escape_handle(env, scope, result, &result);
    napi_close_escapable_handle_scope(env, scope);
    return result;
}

static napi_value JSGetDragPreview(napi_env env, napi_callback_info info)
{
    DragPreview* dragPreview = new DragPreview();
    napi_value result = nullptr;
    napi_create_object(env, &result);
    dragPreview->NapiSerializer(env, result);
    return result;
}
#else

static napi_value JSGetDragPreview(napi_env env, napi_callback_info info)
{
    napi_escapable_handle_scope scope = nullptr;
    napi_open_escapable_handle_scope(env, &scope);
    NapiThrow(env, "The current environment does not enable drag framework or does not support drag preview.",
        ERROR_CODE_INTERNAL_ERROR);
    napi_close_escapable_handle_scope(env, scope);
    return nullptr;
}

static napi_value JSExecuteDrag(napi_env env, napi_callback_info info)
{
    napi_escapable_handle_scope scope = nullptr;
    napi_open_escapable_handle_scope(env, &scope);
    NapiThrow(env, "The current environment does not enable drag framework or does not support pixelMap.",
        ERROR_CODE_INTERNAL_ERROR);
    napi_close_escapable_handle_scope(env, scope);
    return nullptr;
}

static napi_value JSCreateDragAction(napi_env env, napi_callback_info info)
{
    napi_escapable_handle_scope scope = nullptr;
    napi_open_escapable_handle_scope(env, &scope);
    NapiThrow(env, "The current environment does not enable drag framework or does not support pixelMap.",
        ERROR_CODE_INTERNAL_ERROR);
    napi_close_escapable_handle_scope(env, scope);
    return nullptr;
}
#endif

// The default empty implementation function setForegroundColor for dragPreview.
static napi_value JsDragPreviewSetForegroundColor(napi_env env, napi_callback_info info)
{
    return nullptr;
}

// The default empty implementation function animate for dragPreview.
static napi_value JsDragPreviewAnimate(napi_env env, napi_callback_info info)
{
    return nullptr;
}

// The default empty constructor for dragPreview.
static napi_value DragPreviewConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    napi_value global = nullptr;
    napi_get_global(env, &global);
    return thisArg;
}

static napi_value DragControllerExport(napi_env env, napi_value exports)
{
    napi_value dragStatus = nullptr;
    napi_create_object(env, &dragStatus);
    napi_value prop = nullptr;
    napi_create_uint32(env, DRAG_STARTED, &prop);
    napi_set_named_property(env, dragStatus, "STARTED", prop);
    napi_create_uint32(env, DRAG_ENDED, &prop);
    napi_set_named_property(env, dragStatus, "ENDED", prop);

    napi_property_descriptor dragPreviewDesc[] = {
        DECLARE_NAPI_FUNCTION("setForegroundColor", JsDragPreviewSetForegroundColor),
        DECLARE_NAPI_FUNCTION("animate", JsDragPreviewAnimate),
    };
    napi_value classDragPreview = nullptr;
    napi_define_class(env, "DragPreview", NAPI_AUTO_LENGTH, DragPreviewConstructor, nullptr,
        sizeof(dragPreviewDesc) / sizeof(*dragPreviewDesc), dragPreviewDesc, &classDragPreview);

    napi_property_descriptor dragControllerDesc[] = {
        DECLARE_NAPI_FUNCTION("executeDrag", JSExecuteDrag),
        DECLARE_NAPI_FUNCTION("getDragPreview", JSGetDragPreview),
        DECLARE_NAPI_FUNCTION("createDragAction", JSCreateDragAction),
        DECLARE_NAPI_PROPERTY("DragStatus", dragStatus),
        DECLARE_NAPI_PROPERTY("DragPreview", classDragPreview),
    };
    NAPI_CALL(env, napi_define_properties(
                       env, exports, sizeof(dragControllerDesc) / sizeof(dragControllerDesc[0]), dragControllerDesc));
    return exports;
}

static napi_module dragControllerModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = DragControllerExport,
    .nm_modname = "arkui.dragController",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void DragControllerRegister()
{
    napi_module_register(&dragControllerModule);
}
} // namespace OHOS::Ace::Napi
