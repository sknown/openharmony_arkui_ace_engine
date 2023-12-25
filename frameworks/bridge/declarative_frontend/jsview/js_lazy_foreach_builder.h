/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_LAZY_FOREACH_BUILDER_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_LAZY_FOREACH_BUILDER_H

#include <functional>
#include <set>
#include <string>

#include "base/memory/ace_type.h"
#include "bridge/declarative_frontend/jsview/js_lazy_foreach_actuator.h"
#include "bridge/declarative_frontend/jsview/js_view.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/syntax/lazy_for_each_builder.h"

namespace OHOS::Ace::Framework {

class JSLazyForEachBuilder : public NG::LazyForEachBuilder, public JSLazyForEachActuator {
    DECLARE_ACE_TYPE(JSLazyForEachBuilder, NG::LazyForEachBuilder, JSLazyForEachActuator);

public:
    JSLazyForEachBuilder() = default;
    ~JSLazyForEachBuilder()
    {
        changedLazyForEachNodes_.clear();
        dependElementIds_.clear();
    };

    int32_t OnGetTotalCount() override
    {
        return GetTotalIndexCount();
    }

    void OnExpandChildrenOnInitialInNG() override
    {
        auto totalIndex = GetTotalIndexCount();
        auto* stack = NG::ViewStackProcessor::GetInstance();
        JSRef<JSVal> params[3];
        for (auto index = 0; index < totalIndex; index++) {
            params[0] = CallJSFunction(getDataFunc_, dataSourceObj_, index);
            params[1] = JSRef<JSVal>::Make(ToJSValue(index));
            params[2] = JSRef<JSVal>::Make(ToJSValue(true));
            std::string key = keyGenFunc_(params[0], index);
            stack->PushKey(key);
            itemGenFunc_->Call(JSRef<JSObject>(), 3, params);
            stack->PopKey();
        }
    }

    void NotifyDataChanged(size_t index, RefPtr<NG::UINode>& lazyForEachNode, bool isRebuild) override
    {
        if (updateChangedNodeFlag_ && !isRebuild) {
            changedLazyForEachNodes_[index] = lazyForEachNode;
        }
    }

    void NotifyDataDeleted(RefPtr<NG::UINode>& lazyForEachNode) override
    {
        if (updateChangedNodeFlag_) {
            dependElementIds_.erase(lazyForEachNode);
        }
    }

    void UpdateDependElmtIds(RefPtr<NG::UINode>& node, JSRef<JSVal>& jsElmtIds, std::string key) {
        std::string lastKey;
        if (jsElmtIds->IsArray()) {
            JSRef<JSArray> jsElmtIdArray = JSRef<JSArray>::Cast(jsElmtIds);
            for (size_t i = 0; i < jsElmtIdArray->Length(); i++) {
                JSRef<JSVal> jsElmtId = jsElmtIdArray->GetValueAt(i);
                if (jsElmtId->IsNumber()) {
                    dependElementIds_[node].insert(jsElmtId->ToNumber<uint32_t>());
                }
            }
            lastKey = dependElementIds_[node].second;
            dependElementIds_[node].second = key;
        }
        return lastKey;
    }

    JSRef<JSVal> SetToJSVal(std::set<uint32_t> elmtIds) {
        JSRef<JSArray> jsElmtIdArray = JSRef<JSArray>::New();
        int32_t count = 0;
        for (auto& elmtId : elmtIds) {
            jsElmtIdArray->SetValueAt(count, JSRef<JSVal>::Make(ToJSValue(elmtId)));
            count++;
        }
        return JSRef<JSVal>::Cast(jsElmtIdArray);
    }

    std::pair<std::string, RefPtr<NG::UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, NG::LazyForEachCacheChild>& cachedItems) override
    {
        std::pair<std::string, RefPtr<NG::UINode>> info;
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(executionContext_, info);
        if (getDataFunc_.IsEmpty()) {
            return info;
        }

        JSRef<JSVal> params[4];
        params[0] = CallJSFunction(getDataFunc_, dataSourceObj_, index);
        params[1] = JSRef<JSVal>::Make(ToJSValue(index));
        std::string key = keyGenFunc_(params[0], index);
        auto cachedIter = cachedItems.find(key);
        if (cachedIter != cachedItems.end()) {
            info.first = key;
            info.second = cachedIter->second.second;
            cachedItems.erase(cachedIter);
            return info;
        }

        auto nodeIter = changedLazyForEachNodes_.find(index);
        if (updateChangedNodeFlag_ && nodeIter != changedLazyForEachNodes_.end()) {
            RefPtr<NG::UINode> lazyForEachNode = nodeIter->second;
            info.first = key;
            info.second = lazyForEachNode;
            params[2] = JSRef<JSVal>::Make(ToJSValue(false));
            params[3] = SetToJSVal(dependElementIds_[lazyForEachNode].first);

            auto jsElmtIds = itemGenFunc_->Call(JSRef<JSObject>(), 4, params);
            std::string lastKey = UpdateDependElmtIds(info.second, jsElmtIds, key);
            changedLazyForEachNodes_.erase(nodeIter);
            cachedItems.erase(lastKey);
            return info;
        }

        NG::ScopedViewStackProcessor scopedViewStackProcessor;
        auto* viewStack = NG::ViewStackProcessor::GetInstance();
        if (parentView_) {
            parentView_->MarkLazyForEachProcess(key);
        }
        viewStack->PushKey(key);
        params[2] = JSRef<JSVal>::Make(ToJSValue(true));
        JSRef<JSVal> jsElmtIds = itemGenFunc_->Call(JSRef<JSObject>(), 3, params);
        viewStack->PopKey();
        if (parentView_) {
            parentView_->ResetLazyForEachProcess();
        }
        info.first = key;
        info.second = viewStack->Finish();
        if (updateChangedNodeFlag_) {
            UpdateDependElmtIds(info.second, jsElmtIds, key);
        }
        return info;
    }

    void ReleaseChildGroupById(const std::string& id) override
    {
        JSLazyForEachActuator::ReleaseChildGroupByComposedId(id);
    }

    void RegisterDataChangeListener(const RefPtr<V2::DataChangeListener>& listener) override
    {
        JSLazyForEachActuator::RegisterListener(listener);
    }

    void UnregisterDataChangeListener(V2::DataChangeListener* listener) override
    {
        JSLazyForEachActuator::UnregisterListener(listener);
    }

    ACE_DISALLOW_COPY_AND_MOVE(JSLazyForEachBuilder);

private:
    std::map<int32_t, RefPtr<NG::UINode>> changedLazyForEachNodes_;
    std::map<RefPtr<NG::UINode>, std::pair<std::set<uint32_t>, std::string>> dependElementIds;
};

} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_LAZY_FOREACH_BUILDER_H
