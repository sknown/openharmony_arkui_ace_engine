/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <string>

#include "base/log/ace_trace.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "jsi_bindings.h"

namespace OHOS::Ace::Framework {

template<typename C>
thread_local std::unordered_map<std::string, panda::Global<panda::FunctionRef>> JsiClass<C>::staticFunctions_;

template<typename C>
thread_local std::unordered_map<std::string, panda::Global<panda::FunctionRef>> JsiClass<C>::customFunctions_;

template<typename C>
thread_local std::unordered_map<std::string, panda::Global<panda::FunctionRef>> JsiClass<C>::customGetFunctions_;

template<typename C>
thread_local std::unordered_map<std::string, panda::Global<panda::FunctionRef>> JsiClass<C>::customSetFunctions_;

template<typename C>
thread_local FunctionCallback JsiClass<C>::constructor_ = nullptr;

template<typename C>
thread_local JSFunctionCallback JsiClass<C>::jsConstructor_ = nullptr;

template<typename C>
thread_local JSDestructorCallback<C> JsiClass<C>::jsDestructor_ = nullptr;

template<typename C>
thread_local JSGCMarkCallback<C> JsiClass<C>::jsGcMark_ = nullptr;

template<typename C>
thread_local std::string JsiClass<C>::className_;

template<typename C>
thread_local panda::Global<panda::FunctionRef> JsiClass<C>::classFunction_;

template<typename C>
thread_local std::vector<shared_ptr<int32_t>> JsiClass<C>::functionIds_;

template<typename C>
void JsiClass<C>::Declare(const char* name)
{
    className_ = name;
    staticFunctions_.clear();
    customFunctions_.clear();
    customGetFunctions_.clear();
    customSetFunctions_.clear();
    classFunction_.Empty();
}

template<typename C>
template<typename Base, typename R, typename... Args>
void JsiClass<C>::Method(const char* name, R (Base::*func)(Args...), int id)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    auto funcId = std::make_shared<int32_t>(id);
    functionIds_.emplace_back(funcId);
    customFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(vm,
        panda::FunctionRef::New(vm, MethodCallback<Base, R, Args...>, nullptr, funcId.get())));
}

template<typename C>
template<typename T>
void JsiClass<C>::CustomMethod(const char* name, MemberFunctionCallback<T> callback, int id)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    auto funcId = std::make_shared<int32_t>(id);
    functionIds_.emplace_back(funcId);
    customFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm,
        InternalMemberFunctionCallback<T, panda::JsiRuntimeCallInfo*>, nullptr, funcId.get())));
}

template<typename C>
void JsiClass<C>::CustomMethod(const char* name, FunctionCallback callback)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    customFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm, callback)));
}

template<typename C>
template<typename T>
void JsiClass<C>::CustomMethod(const char* name, JSMemberFunctionCallback<T> callback, int id)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    auto funcId = std::make_shared<int32_t>(id);
    functionIds_.emplace_back(funcId);
    customFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm,
        InternalJSMemberFunctionCallback<T>, nullptr, funcId.get())));
}

template<typename C>
template<typename T>
void JsiClass<C>::CustomProperty(const char* name, MemberFunctionGetCallback<T> callback, int getterId, int setterId)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    auto funcGetId = std::make_shared<int32_t>(getterId);
    functionIds_.emplace_back(funcGetId);

    customGetFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm,
        InternalMemberFunctionCallback<T, panda::JsiRuntimeCallInfo*>, nullptr, funcGetId.get())));

    auto funcSetId = std::make_shared<int32_t>(setterId);
    functionIds_.emplace_back(funcSetId);
    customSetFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm,
        InternalMemberFunctionCallback<T, panda::JsiRuntimeCallInfo*>, nullptr, funcSetId.get())));
}

template<typename C>
void JsiClass<C>::CustomProperty(const char* name, FunctionGetCallback getter, FunctionSetCallback setter)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    customGetFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm, getter)));
    customSetFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm, setter)));
}

template<typename C>
template<typename T>
void JsiClass<C>::CustomProperty(const char* name, JSMemberFunctionCallback<T> callback, int getterId, int setterId)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    auto funcGetId = std::make_shared<int32_t>(getterId);
    functionIds_.emplace_back(funcGetId);
    customGetFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm,
        InternalJSMemberFunctionCallback<T>, nullptr, funcGetId.get())));
    auto funcSetId = std::make_shared<int32_t>(setterId);
    functionIds_.emplace_back(funcSetId);
    customSetFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm,
        InternalJSMemberFunctionCallback<T>, nullptr, funcSetId.get())));
}

template<typename C>
template<typename R, typename... Args>
void JsiClass<C>::StaticMethod(const char* name, StaticFunctionBinding<R, Args...>* staticFunctionBinding)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    staticFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm,
        StaticMethodCallback<R, Args...>, nullptr, (void*)staticFunctionBinding)));
}

template<typename C>
void JsiClass<C>::StaticMethod(
    const char* name, StaticFunctionBinding<void, const JSCallbackInfo&>* staticFunctionBinding)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    staticFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm,
        JSStaticMethodCallback, nullptr, (void*)staticFunctionBinding)));
}

template<typename C>
void JsiClass<C>::CustomStaticMethod(const char* name, FunctionCallback callback)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    staticFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm, callback)));
}

template<typename C>
template<typename T>
void JsiClass<C>::StaticConstant(const char* name, T val)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    panda::Local<panda::JSValueRef> key = panda::StringRef::NewFromUtf8(vm, name);
    classFunction_->Set(vm, key, JsiValueConvertor::toJsiValueWithVM<std::string>(vm, val));
}

template<typename C>
void JsiClass<C>::Bind(BindingTarget t, FunctionCallback ctor)
{
    constructor_ = ctor;
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    LocalScope scope(vm);
    classFunction_ = panda::Global<panda::FunctionRef>(
        vm, panda::FunctionRef::NewClassFunction(vm, ConstructorInterceptor, nullptr, nullptr));
    classFunction_->SetName(vm, StringRef::NewFromUtf8(vm, className_.c_str()));
    auto prototype = Local<ObjectRef>(classFunction_->GetFunctionPrototype(vm));
    prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, "constructor"),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));
    for (const auto& [name, val] : staticFunctions_) {
        classFunction_->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }
    for (const auto& [name, val] : customFunctions_) {
        prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }

    for (const auto& [nameGet, valGet] : customGetFunctions_) {
        for (const auto& [nameSet, valSet] : customSetFunctions_) {
            if (nameGet == nameSet) {
                prototype->SetAccessorProperty(vm, panda::StringRef::NewFromUtf8(vm, nameGet.c_str()),
                    valGet.ToLocal(), valSet.ToLocal());
            }
        }
    }

    t->Set(vm, panda::StringRef::NewFromUtf8(vm, ThisJSClass::JSName()),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));
}

template<typename C>
void JsiClass<C>::Bind(
    BindingTarget t, JSFunctionCallback ctor, JSDestructorCallback<C> dtor, JSGCMarkCallback<C> gcMark)
{
    jsConstructor_ = ctor;
    jsDestructor_ = dtor;
    jsGcMark_ = gcMark;
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    LocalScope scope(vm);
    classFunction_ = panda::Global<panda::FunctionRef>(
        vm, panda::FunctionRef::NewClassFunction(vm, JSConstructorInterceptor, nullptr, nullptr));
    classFunction_->SetName(vm, StringRef::NewFromUtf8(vm, className_.c_str()));
    auto prototype = panda::Local<panda::ObjectRef>(classFunction_->GetFunctionPrototype(vm));
    prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, "constructor"),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));
    for (const auto& [name, val] : staticFunctions_) {
        classFunction_->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }
    for (const auto& [name, val] : customFunctions_) {
        prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }

    for (const auto& [nameGet, valGet] : customGetFunctions_) {
        for (const auto& [nameSet, valSet] : customSetFunctions_) {
            if (nameGet == nameSet) {
                prototype->SetAccessorProperty(vm, panda::StringRef::NewFromUtf8(vm, nameGet.c_str()),
                    valGet.ToLocal(), valSet.ToLocal());
            }
        }
    }

    t->Set(vm, panda::Local<panda::JSValueRef>(panda::StringRef::NewFromUtf8(vm, ThisJSClass::JSName())),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));
}

template<typename C>
template<typename... Args>
void JsiClass<C>::Bind(BindingTarget t, JSDestructorCallback<C> dtor, JSGCMarkCallback<C> gcMark)
{
    jsDestructor_ = dtor;
    jsGcMark_ = gcMark;
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    LocalScope scope(vm);
    classFunction_ = panda::Global<panda::FunctionRef>(
        vm, panda::FunctionRef::NewClassFunction(vm, InternalConstructor<Args...>, nullptr, nullptr));
    classFunction_->SetName(vm, StringRef::NewFromUtf8(vm, className_.c_str()));
    auto prototype = panda::Local<panda::ObjectRef>(classFunction_->GetFunctionPrototype(vm));
    prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, "constructor"),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));
    for (const auto& [name, val] : staticFunctions_) {
        classFunction_->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }
    for (const auto& [name, val] : customFunctions_) {
        prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }

    for (const auto& [nameGet, valGet] : customGetFunctions_) {
        for (const auto& [nameSet, valSet] : customSetFunctions_) {
            if (nameGet == nameSet) {
                prototype->SetAccessorProperty(vm, panda::StringRef::NewFromUtf8(vm, nameGet.c_str()),
                    valGet.ToLocal(), valSet.ToLocal());
            }
        }
    }

    t->Set(vm, panda::Local<panda::JSValueRef>(panda::StringRef::NewFromUtf8(vm, ThisJSClass::JSName())),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));
}

template<typename C>
template<typename Base>
void JsiClass<C>::Inherit()
{
    auto& staticFunctions = JsiClass<Base>::GetStaticFunctions();
    for (auto& [name, function] : staticFunctions) {
        if (staticFunctions_.find(name) != staticFunctions_.end()) {
            continue;
        }
        staticFunctions_.emplace(name, function);
    }
    auto& customFunctions = JsiClass<Base>::GetCustomFunctions();
    for (auto& [name, function] : customFunctions) {
        if (customFunctions_.find(name) != customFunctions_.end()) {
            continue;
        }
        customFunctions_.emplace(name, function);
    }
}

template<typename C>
template<typename Base>
void JsiClass<C>::InheritAndBind(
    BindingTarget t, JSFunctionCallback ctor, JSDestructorCallback<C> dtor, JSGCMarkCallback<C> gcMark)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    LocalScope scope(vm);
    panda::Local<panda::JSValueRef> hasExistRef = t->Get(vm,
        panda::Local<panda::JSValueRef>(panda::StringRef::NewFromUtf8(vm, ThisJSClass::JSName())));
    if (hasExistRef.IsEmpty()) {
        return;
    }

    jsConstructor_ = ctor;
    jsDestructor_ = dtor;
    jsGcMark_ = gcMark;
    classFunction_ = panda::Global<panda::FunctionRef>(
        vm, panda::FunctionRef::NewClassFunction(vm, JSConstructorInterceptor, nullptr, nullptr));
    classFunction_->SetName(vm, StringRef::NewFromUtf8(vm, className_.c_str()));

    panda::Local<panda::JSValueRef> getResult = t->Get(vm,
        panda::Local<panda::JSValueRef>(panda::StringRef::NewFromUtf8(vm, JSClassImpl<Base, JsiClass>::JSName())));
    if (getResult.IsEmpty()) {
        return;
    }

    panda::Local<panda::FunctionRef> baseClassFunction(getResult);
    classFunction_->SetPrototype(vm, baseClassFunction);
    auto prototype = panda::Local<panda::ObjectRef>(classFunction_->GetFunctionPrototype(vm));
    auto baseClassPrototype = panda::Local<panda::ObjectRef>(baseClassFunction->GetFunctionPrototype(vm));
    prototype->SetPrototype(vm, baseClassPrototype);
    prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, "constructor"),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));

    for (const auto& [name, val] : staticFunctions_) {
        classFunction_->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }

    for (const auto& [name, val] : customFunctions_) {
        prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }

    for (const auto& [nameGet, valGet] : customGetFunctions_) {
        for (const auto& [nameSet, valSet] : customSetFunctions_) {
            if (nameGet == nameSet) {
                prototype->SetAccessorProperty(vm, panda::StringRef::NewFromUtf8(vm, nameGet.c_str()),
                    valGet.ToLocal(), valSet.ToLocal());
            }
        }
    }

    t->Set(vm, panda::Local<panda::JSValueRef>(panda::StringRef::NewFromUtf8(vm, ThisJSClass::JSName())),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));
}

template<typename C>
std::unordered_map<std::string, panda::Global<panda::FunctionRef>>& JsiClass<C>::GetStaticFunctions()
{
    return staticFunctions_;
}

template<typename C>
std::unordered_map<std::string, panda::Global<panda::FunctionRef>>& JsiClass<C>::GetCustomFunctions()
{
    return customFunctions_;
}

template<typename C>
template<typename T, typename... Args>
panda::Local<panda::JSValueRef> JsiClass<C>::InternalMemberFunctionCallback(panda::JsiRuntimeCallInfo *runtimeCallInfo)
{
    panda::Local<panda::JSValueRef> thisObj = runtimeCallInfo->GetThisRef();
    C* ptr = static_cast<C*>(panda::Local<panda::ObjectRef>(thisObj)->GetNativePointerField(0));
    T* instance = static_cast<T*>(ptr);
    EcmaVM* vm = runtimeCallInfo->GetVM();
    int index = *(static_cast<int*>(runtimeCallInfo->GetData()));
    auto binding = ThisJSClass::GetFunctionBinding(index);
    if (binding == nullptr) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    ACE_BUILD_SCOPED_TRACE("[%s][%s]", ThisJSClass::JSName(), binding->Name());
    auto fnPtr = static_cast<FunctionBinding<T, panda::Local<panda::JSValueRef>, Args...>*>(binding)->Get();
    (instance->*fnPtr)(runtimeCallInfo);
}

template<typename C>
template<typename T>
panda::Local<panda::JSValueRef> JsiClass<C>::InternalJSMemberFunctionCallback(
    panda::JsiRuntimeCallInfo *runtimeCallInfo)
{
    panda::Local<panda::JSValueRef> thisObj = runtimeCallInfo->GetThisRef();
    EcmaVM* vm = runtimeCallInfo->GetVM();
    C* ptr = static_cast<C*>(panda::Local<panda::ObjectRef>(thisObj)->GetNativePointerField(0));
    if (thisObj->IsProxy()) {
        panda::Local<panda::ProxyRef> thisProxiedObj = static_cast<panda::Local<panda::ProxyRef>>(thisObj);
        ptr = static_cast<C*>(panda::Local<panda::ObjectRef>(thisProxiedObj->GetTarget(vm))->GetNativePointerField(0));
    } else {
        ptr = static_cast<C*>(panda::Local<panda::ObjectRef>(thisObj)->GetNativePointerField(0));
    }

    T* instance = static_cast<T*>(ptr);

    int index = *(static_cast<int*>(runtimeCallInfo->GetData()));
    auto binding = ThisJSClass::GetFunctionBinding(index);
    if (binding == nullptr) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    ACE_BUILD_SCOPED_TRACE("[%s][%s]", ThisJSClass::JSName(), binding->Name());
    auto fnPtr = static_cast<FunctionBinding<T, void, const JSCallbackInfo&>*>(binding)->Get();
    JsiCallbackInfo info(runtimeCallInfo);
    (instance->*fnPtr)(info);

    std::variant<void*, panda::CopyableGlobal<panda::JSValueRef>> retVal = info.GetReturnValue();
    auto jsVal = std::get_if<panda::CopyableGlobal<panda::JSValueRef>>(&retVal);
    if (jsVal) {
        return jsVal->ToLocal();
    }
    return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
}

template<typename C>
template<typename Class, typename R, typename... Args>
panda::Local<panda::JSValueRef> JsiClass<C>::MethodCallback(panda::JsiRuntimeCallInfo *runtimeCallInfo)
{
    panda::Local<panda::JSValueRef> thisObj = runtimeCallInfo->GetThisRef();
    C* ptr = static_cast<C*>(panda::Local<panda::ObjectRef>(thisObj)->GetNativePointerField(0));
    Class* instance = static_cast<Class*>(ptr);
    EcmaVM* vm = runtimeCallInfo->GetVM();
    int index = *(static_cast<int*>(runtimeCallInfo->GetData()));
    auto binding = ThisJSClass::GetFunctionBinding(index);
    if (binding == nullptr) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    ACE_BUILD_SCOPED_TRACE("[%s][%s]", ThisJSClass::JSName(), binding->Name());
    auto fnPtr = static_cast<FunctionBinding<Class, R, Args...>*>(binding)->Get();
    auto tuple = __detail__::ToTuple<std::decay_t<Args>...>(runtimeCallInfo);
    bool returnSelf = binding->Options() & MethodOptions::RETURN_SELF;
    constexpr bool isVoid = std::is_void_v<R>;
    constexpr bool hasArguments = sizeof...(Args) != 0;

    if constexpr (isVoid && hasArguments) {
        // C::MemberFunction(Args...)
        FunctionUtils::CallMemberFunction(instance, fnPtr, tuple);
        return returnSelf ? thisObj : panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    } else if constexpr (isVoid && !hasArguments) {
        // C::MemberFunction()
        (instance->*fnPtr)();
        return returnSelf ? thisObj : panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    } else if constexpr (!isVoid && hasArguments) {
        // R C::MemberFunction(Args...)
        auto result = FunctionUtils::CallMemberFunction(instance, fnPtr, tuple);
        return JsiValueConvertor::toJsiValueWithVM<R>(vm, result);
    } else if constexpr (!isVoid && !hasArguments) {
        // R C::MemberFunction()
        auto res = (instance->*fnPtr)();
        return JsiValueConvertor::toJsiValueWithVM<R>(vm, res);
    }
}

template<typename C>
template<typename Class, typename R, typename... Args>
panda::Local<panda::JSValueRef> JsiClass<C>::JSMethodCallback(panda::JsiRuntimeCallInfo *runtimeCallInfo)
{
    panda::Local<panda::JSValueRef> thisObj = runtimeCallInfo->GetThisRef();
    C* ptr = static_cast<C*>(panda::Local<panda::ObjectRef>(thisObj)->GetNativePointerField(0));
    Class* instance = static_cast<Class*>(ptr);
    EcmaVM* vm = runtimeCallInfo->GetVM();
    int index = *(static_cast<int*>(runtimeCallInfo->GetData()));
    auto binding = ThisJSClass::GetFunctionBinding(index);
    if (binding == nullptr) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    ACE_BUILD_SCOPED_TRACE("[%s][%s]", ThisJSClass::JSName(), binding->Name());
    JsiCallbackInfo info(runtimeCallInfo);
    auto fnPtr = static_cast<FunctionBinding<Class, R, Args...>*>(binding)->Get();
    (instance->*fnPtr)(info);
}

template<typename C>
template<typename R, typename... Args>
panda::Local<panda::JSValueRef> JsiClass<C>::StaticMethodCallback(panda::JsiRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    auto binding = static_cast<StaticFunctionBinding<R, Args...>*>(runtimeCallInfo->GetData());
    if (binding == nullptr) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    ACE_BUILD_SCOPED_TRACE("[%s][%s]", ThisJSClass::JSName(), binding->Name());
    auto fnPtr = binding->Get();
    auto tuple = __detail__::ToTuple<std::decay_t<Args>...>(runtimeCallInfo);
    bool returnSelf = binding->Options() & MethodOptions::RETURN_SELF;
    constexpr bool isVoid = std::is_void_v<R>;
    constexpr bool hasArguments = sizeof...(Args) != 0;

    panda::Local<panda::JSValueRef> thisObj = runtimeCallInfo->GetThisRef();
    if constexpr (isVoid && hasArguments) {
        // void C::MemberFunction(Args...)
        FunctionUtils::CallStaticMemberFunction(fnPtr, tuple);
        return returnSelf ? thisObj : panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    } else if constexpr (isVoid && !hasArguments) {
        // void C::MemberFunction()
        fnPtr();
        return panda::JSValueRef::Undefined(vm);
    } else if constexpr (!isVoid && hasArguments) {
        // R C::MemberFunction(Args...)
        auto result = FunctionUtils::CallStaticMemberFunction(fnPtr, tuple);
        return JsiValueConvertor::toJsiValueWithVM(vm, result);
    } else if constexpr (!isVoid && !hasArguments) {
        // R C::MemberFunction()
        auto res = fnPtr();
        return JsiValueConvertor::toJsiValueWithVM(vm, res);
    }
}

template<typename C>
panda::Local<panda::JSValueRef> JsiClass<C>::JSStaticMethodCallback(panda::JsiRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    auto binding = static_cast<StaticFunctionBinding<void, const JSCallbackInfo&>*>(runtimeCallInfo->GetData());
    if (binding == nullptr) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    ACE_BUILD_SCOPED_TRACE("[%s][%s]", ThisJSClass::JSName(), binding->Name());
    auto fnPtr = binding->Get();
    JsiCallbackInfo info(runtimeCallInfo);
    fnPtr(info);
    std::variant<void*, panda::CopyableGlobal<panda::JSValueRef>> retVal = info.GetReturnValue();
    auto jsVal = std::get_if<panda::CopyableGlobal<panda::JSValueRef>>(&retVal);
    if (jsVal) {
        return jsVal->ToLocal();
    }
    return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
}

template<typename C>
template<typename... Args>
panda::Local<panda::JSValueRef> JsiClass<C>::InternalConstructor(panda::JsiRuntimeCallInfo *runtimeCallInfo)
{
    panda::Local<panda::JSValueRef> newTarget = runtimeCallInfo->GetNewTargetRef();
    if (!newTarget->IsFunction()) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(runtimeCallInfo->GetVM()));
    }
    EcmaVM* vm = runtimeCallInfo->GetVM();
    panda::Local<panda::JSValueRef> thisObj = runtimeCallInfo->GetThisRef();
    auto tuple = __detail__::ToTuple<std::decay_t<Args>...>(runtimeCallInfo);
    C* instance = FunctionUtils::ConstructFromTuple<C>(tuple);
    Local<ObjectRef>(thisObj)->SetNativePointerFieldCount(vm, 1);
    panda::Local<panda::ObjectRef>(thisObj)->SetNativePointerField(vm, 0, static_cast<void*>(instance));
    return thisObj;
}

template<typename C>
bool JsiClass<C>::CheckIfConstructCall(panda::JsiRuntimeCallInfo *runtimeCallInfo)
{
    return true;
}

template<typename C>
panda::Local<panda::JSValueRef> JsiClass<C>::ConstructorInterceptor(panda::JsiRuntimeCallInfo *runtimeCallInfo)
{
    panda::Local<panda::JSValueRef> newTarget = runtimeCallInfo->GetNewTargetRef();
    EcmaVM* vm = runtimeCallInfo->GetVM();
    if (!newTarget->IsFunction()) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    return constructor_(runtimeCallInfo);
}

template<typename C>
panda::Local<panda::JSValueRef> JsiClass<C>::JSConstructorInterceptor(panda::JsiRuntimeCallInfo *runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    panda::Local<panda::JSValueRef> newTarget = runtimeCallInfo->GetNewTargetRef();
    if (newTarget->IsFunction() && jsConstructor_) {
        JsiCallbackInfo info(runtimeCallInfo);
        jsConstructor_(info);
        auto retVal = info.GetReturnValue();
        if (retVal.valueless_by_exception()) {
            return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
        }
        auto instance = std::get_if<void*>(&retVal);
        if (instance) {
            panda::Local<panda::JSValueRef> thisObj = runtimeCallInfo->GetThisRef();
            Local<ObjectRef>(thisObj)->SetNativePointerFieldCount(vm, 1);
            size_t nativeSize = info.GetSize();
            Local<ObjectRef>(thisObj)->SetNativePointerField(
                vm, 0, *instance, &JsiClass<C>::DestructorInterceptor, nullptr, nativeSize);
            return thisObj;
        }
    }
    return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
}

template<typename C>
void JsiClass<C>::DestructorInterceptor(void* nativePtr, void* data)
{
    if (jsDestructor_) {
        jsDestructor_(reinterpret_cast<C*>(nativePtr));
    }
}

template<typename C>
panda::Local<panda::JSValueRef> JsiClass<C>::NewInstance()
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    return classFunction_->Constructor(vm, nullptr, 0);
}
} // namespace OHOS::Ace::Framework
