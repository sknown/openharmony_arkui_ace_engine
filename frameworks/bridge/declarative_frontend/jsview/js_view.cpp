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

#include "frameworks/bridge/declarative_frontend/jsview/js_view.h"

#include "base/log/ace_trace.h"
#include "core/pipeline/base/composed_element.h"
#include "frameworks/bridge/declarative_frontend/engine/content_storage_set.h"
#include "frameworks/bridge/declarative_frontend/engine/js_execution_scope_defines.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_register.h"
#include "frameworks/bridge/declarative_frontend/view_stack_processor.h"

namespace OHOS::Ace::Framework {

JSView::JSView(const std::string& viewId, JSRef<JSObject> jsObject, JSRef<JSFunc> jsRenderFunction) : viewId_(viewId)
{
    jsViewFunction_ = AceType::MakeRefPtr<ViewFunctions>(jsObject, jsRenderFunction);
    LOGD("JSView constructor");
}

JSView::~JSView()
{
    LOGD("Destroy");
    jsViewFunction_.Reset();
};

RefPtr<OHOS::Ace::Component> JSView::CreateComponent()
{
    ACE_SCOPED_TRACE("JSView::CreateSpecializedComponent");
    // create component, return new something, need to set proper ID

    std::string key = ViewStackProcessor::GetInstance()->ProcessViewId(viewId_);
    auto composedComponent = AceType::MakeRefPtr<ComposedComponent>(key, "view");

    // add callback for element creation to component, and get pointer reference
    // to the element on creation. When state of this view changes, mark the
    // element to dirty.
    auto renderFunction = [weak = AceType::WeakClaim(this)](const RefPtr<Component>& component) -> RefPtr<Component> {
        auto jsView = weak.Upgrade();
        return jsView ? jsView->InternalRender(component) : nullptr;
    };

    auto elementFunction = [weak = AceType::WeakClaim(this), renderFunction](const RefPtr<ComposedElement>& element) {
        auto jsView = weak.Upgrade();
        if (!jsView) {
            LOGE("the js view is nullptr in element function");
            return;
        }
        if (jsView->element_.Invalid()) {
            ACE_SCORING_EVENT("Component[" + jsView->viewId_ + "].Appear");
            jsView->jsViewFunction_->ExecuteAppear();
        }
        jsView->element_ = element;
        // add render function callback to element. when the element rebuilds due
        // to state update it will call this callback to get the new child component.
        if (element) {
            element->SetRenderFunction(std::move(renderFunction));
            if (jsView->jsViewFunction_ && jsView->jsViewFunction_->HasPageTransition()) {
                auto pageTransitionFunction = [weak]() -> RefPtr<Component> {
                    auto jsView = weak.Upgrade();
                    if (!jsView || !jsView->jsViewFunction_) {
                        return nullptr;
                    }
                    {
                        ACE_SCORING_EVENT("Component[" + jsView->viewId_ + "].Transition");
                        jsView->jsViewFunction_->ExecuteTransition();
                    }
                    return jsView->BuildPageTransitionComponent();
                };
                element->SetPageTransitionFunction(std::move(pageTransitionFunction));
            }
        }
    };

    composedComponent->SetElementFunction(std::move(elementFunction));

    if (IsStatic()) {
        LOGD("will mark composedComponent as static");
        composedComponent->SetStatic();
    }
    return composedComponent;
}

RefPtr<OHOS::Ace::PageTransitionComponent> JSView::BuildPageTransitionComponent()
{
    auto pageTransitionComponent = ViewStackProcessor::GetInstance()->GetPageTransitionComponent();
    ViewStackProcessor::GetInstance()->ClearPageTransitionComponent();
    return pageTransitionComponent;
}

RefPtr<OHOS::Ace::Component> JSView::InternalRender(const RefPtr<Component>& parent)
{
    JAVASCRIPT_EXECUTION_SCOPE_STATIC;
    needsUpdate_ = false;
    if (!jsViewFunction_) {
        LOGE("JSView: InternalRender jsViewFunction_ error");
        return nullptr;
    }
    {
        ACE_SCORING_EVENT("Component[" + viewId_ + "].AboutToRender");
        jsViewFunction_->ExecuteAboutToRender();
        }
    {
        ACE_SCORING_EVENT("Component[" + viewId_ + "].Build");
        jsViewFunction_->ExecuteRender();
        }
    {
        ACE_SCORING_EVENT("Component[" + viewId_ + "].OnRenderDone");
        jsViewFunction_->ExecuteOnRenderDone();
        }
    CleanUpAbandonedChild();
    jsViewFunction_->Destroy(this);
    auto buildComponent = ViewStackProcessor::GetInstance()->Finish();
    return buildComponent;
}

/**
 * marks the JSView's composed component as needing update / rerender
 */
void JSView::MarkNeedUpdate()
{
    ACE_DCHECK((!GetElement().Invalid()) && "JSView's ComposedElement must be created before requesting an update");
    ACE_SCOPED_TRACE("JSView::MarkNeedUpdate");

    auto element = GetElement().Upgrade();
    if (element) {
        element->MarkDirty();
    }
    needsUpdate_ = true;
}

void JSView::Destroy(JSView* parentCustomView)
{
    LOGD("JSView::Destroy start");
    DestroyChild(parentCustomView);
    {
        ACE_SCORING_EVENT("Component[" + viewId_ + "].Disappear");
        jsViewFunction_->ExecuteDisappear();
    }
    {
        ACE_SCORING_EVENT("Component[" + viewId_ + "].AboutToBeDeleted");
        jsViewFunction_->ExecuteAboutToBeDeleted();
    }
    LOGD("JSView::Destroy end");
}

void JSView::Create(const JSCallbackInfo& info)
{
    if (info[0]->IsObject()) {
        JSRefPtr<JSView> view = JSRef<JSObject>::Cast(info[0]);
        ViewStackProcessor::GetInstance()->Push(view->CreateComponent(), true);
    } else {
        LOGE("JSView Object is expected.");
    }
}

void JSView::JSBind(BindingTarget object)
{
    JSClass<JSView>::Declare("NativeView");
    JSClass<JSView>::StaticMethod("create", &JSView::Create);
    JSClass<JSView>::Method("markNeedUpdate", &JSView::MarkNeedUpdate);
    JSClass<JSView>::Method("needsUpdate", &JSView::NeedsUpdate);
    JSClass<JSView>::Method("markStatic", &JSView::MarkStatic);
    JSClass<JSView>::CustomMethod("getContext", &JSView::GetContext);
    JSClass<JSView>::CustomMethod("getContentStorage", &JSView::GetContentStorage);
    JSClass<JSView>::CustomMethod("findChildById", &JSView::FindChildById);
    JSClass<JSView>::Inherit<JSViewAbstract>();
    JSClass<JSView>::Bind(object, ConstructorCallback, DestructorCallback);
}

void JSView::GetContext(const JSCallbackInfo& info)
{
    info.SetReturnValue(ContentStorageSet::GetCurrentContext());
}

void JSView::GetContentStorage(const JSCallbackInfo& info)
{
    info.SetReturnValue(ContentStorageSet::GetCurrentStorage());
}

void JSView::FindChildById(const JSCallbackInfo& info)
{
    LOGD("JSView::FindChildById");
    if (info[0]->IsNumber() || info[0]->IsString()) {
        std::string viewId = info[0]->ToString();
        JSRefPtr<JSView> jsView = GetChildById(viewId);
        info.SetReturnValue(jsView.Get());
    } else {
        LOGE("JSView FindChildById with invalid arguments.");
        JSException::Throw("%s", "JSView FindChildById with invalid arguments.");
    }
}

void JSView::ConstructorCallback(const JSCallbackInfo& info)
{
    JSRef<JSObject> thisObj = info.This();
    JSRef<JSVal> renderFunc = thisObj->GetProperty("render");
    if (!renderFunc->IsFunction()) {
        LOGE("View derived classes must provide render(){...} function");
        JSException::Throw("%s", "View derived classes must provide render(){...} function");
        return;
    }

    int argc = info.Length();
    if (argc > 1 && (info[0]->IsNumber() || info[0]->IsString())) {
        std::string viewId = info[0]->ToString();
        auto instance = AceType::MakeRefPtr<JSView>(viewId, info.This(), JSRef<JSFunc>::Cast(renderFunc));
        auto context = info.GetExecutionContext();
        instance->SetContext(context);
        instance->IncRefCount();
        info.SetReturnValue(AceType::RawPtr(instance));
        if (!info[1]->IsUndefined() && info[1]->IsObject()) {
            JSRef<JSObject> parentObj = JSRef<JSObject>::Cast(info[1]);
            JSView* parentView = parentObj->Unwrap<JSView>();
            parentView->AddChildById(viewId, info.This());
        }
    } else {
        LOGE("JSView creation with invalid arguments.");
        JSException::Throw("%s", "JSView creation with invalid arguments.");
    }
}

void JSView::DestructorCallback(JSView* view)
{
    LOGD("JSView(DestructorCallback) start");
    view->DecRefCount();
    LOGD("JSView(DestructorCallback) end");
}

void JSView::DestroyChild(JSView* parentCustomView)
{
    LOGD("JSView::DestroyChild start");
    for (auto child : customViewChildren_) {
        child.second->Destroy(this);
        child.second.Reset();
    }
    LOGD("JSView::DestroyChild end");
}

void JSView::CleanUpAbandonedChild()
{
    auto startIter = customViewChildren_.begin();
    auto endIter = customViewChildren_.end();
    std::vector<std::string> removedViewIds;
    while (startIter != endIter) {
        auto found = lastAccessedViewIds_.find(startIter->first);
        if (found == lastAccessedViewIds_.end()) {
            LOGD(" found abandoned view with id %{public}s", startIter->first.c_str());
            removedViewIds.emplace_back(startIter->first);
            startIter->second->Destroy(this);
            startIter->second.Reset();
        }
        ++startIter;
    }

    for (auto& viewId : removedViewIds) {
        customViewChildren_.erase(viewId);
    }

    lastAccessedViewIds_.clear();
}

JSRefPtr<JSView> JSView::GetChildById(const std::string& viewId)
{
    auto id = ViewStackProcessor::GetInstance()->ProcessViewId(viewId);
    auto found = customViewChildren_.find(id);
    if (found != customViewChildren_.end()) {
        ChildAccessedById(id);
        return found->second;
    }
    return JSRefPtr<JSView>();
}

void JSView::AddChildById(const std::string& viewId, const JSRefPtr<JSView>& obj)
{
    auto id = ViewStackProcessor::GetInstance()->ProcessViewId(viewId);
    customViewChildren_.emplace(id, obj);
    ChildAccessedById(id);
}

void JSView::RemoveChildGroupById(const std::string& viewId)
{
    if (viewId.empty()) {
        auto removeView = customViewChildren_.find(viewId);
        if (removeView != customViewChildren_.end()) {
            removeView->second->Destroy(this);
            removeView->second.Reset();
            customViewChildren_.erase(removeView);
        }
        return;
    }
    auto startIter = customViewChildren_.begin();
    auto endIter = customViewChildren_.end();
    std::vector<std::string> removedViewIds;
    while (startIter != endIter) {
        if (StartWith(startIter->first, viewId)) {
            removedViewIds.emplace_back(startIter->first);
            startIter->second->Destroy(this);
            startIter->second.Reset();
        }
        ++startIter;
    }

    for (auto&& removeId : removedViewIds) {
        customViewChildren_.erase(removeId);
    }
}

void JSView::ChildAccessedById(const std::string& viewId)
{
    lastAccessedViewIds_.emplace(viewId);
}

} // namespace OHOS::Ace::Framework
