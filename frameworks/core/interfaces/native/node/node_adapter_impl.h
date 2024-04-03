/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#pragma once

#include <cstdint>
#include <string>

#include "base/error/error_code.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/syntax/lazy_for_each_builder.h"
#include "core/interfaces/arkoala/arkoala_api.h"

namespace OHOS::Ace::NG {
class NativeLazyForEachBuilder : public LazyForEachBuilder {
    DECLARE_ACE_TYPE(NativeLazyForEachBuilder, LazyForEachBuilder)
public:
    // used in ArkTS side.
    void ReleaseChildGroupById(const std::string& id) override {}

    void RegisterDataChangeListener(const RefPtr<V2::DataChangeListener>& listener) override;

    void UnregisterDataChangeListener(V2::DataChangeListener* listener) override;

    void SetNodeTotalCount(ArkUI_Uint32 nodeCount)
    {
        totalCount_ = nodeCount;
    }

    ArkUI_Uint32 GetNodeTotalCount() const
    {
        return totalCount_;
    }

    void SetUserData(void* userData)
    {
        userData_ = userData;
    }

    void SetReceiver(void (*receiver)(ArkUINodeAdapterEvent* event))
    {
        receiver_ = receiver;
    }

    ArkUI_Int32 NotifyItemReloaded()
    {
        if (listener_) {
            listener_->OnDataReloaded();
            return ERROR_CODE_NO_ERROR;
        }
        return ERROR_CODE_NATIVE_IMPL_NODE_ADAPTER_NO_LISTENER_ERROR;
    }

    ArkUI_Int32 NotifyItemChanged(ArkUI_Uint32 startPosition, ArkUI_Uint32 itemCount)
    {
        if (listener_) {
            listener_->OnDataBulkChanged(startPosition, itemCount);
            return ERROR_CODE_NO_ERROR;
        }
        return ERROR_CODE_NATIVE_IMPL_NODE_ADAPTER_NO_LISTENER_ERROR;
    }

    ArkUI_Int32 NotifyItemRemoved(ArkUI_Uint32 startPosition, ArkUI_Uint32 itemCount)
    {
        if (listener_) {
            listener_->OnDataBulkDeleted(startPosition, itemCount);
            return ERROR_CODE_NO_ERROR;
        }
        return ERROR_CODE_NATIVE_IMPL_NODE_ADAPTER_NO_LISTENER_ERROR;
    }
    ArkUI_Int32 NotifyItemInserted(ArkUI_Uint32 startPosition, ArkUI_Uint32 itemCount)
    {
        if (listener_) {
            listener_->OnDataBulkAdded(startPosition, itemCount);
            return ERROR_CODE_NO_ERROR;
        }
        return ERROR_CODE_NATIVE_IMPL_NODE_ADAPTER_NO_LISTENER_ERROR;
    }
    ArkUI_Int32 NotifyItemMoved(ArkUI_Uint32 from, ArkUI_Uint32 to)
    {
        if (listener_) {
            listener_->OnDataMoveToNewPlace(from, to);
            return ERROR_CODE_NO_ERROR;
        }
        return ERROR_CODE_NATIVE_IMPL_NODE_ADAPTER_NO_LISTENER_ERROR;
    }

    // should manager the array memory.
    ArkUI_Int32 GetAllItem(ArkUINodeHandle** items, ArkUI_Uint32* size);

    void SetHostHandle(ArkUINodeAdapterHandle handle)
    {
        handle_ = handle;
    }

    ArkUINodeAdapterHandle GetHostHandle() const
    {
        return handle_;
    }

protected:
    int32_t OnGetTotalCount() override;

    LazyForEachChild OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>& cachedItems) override;

    void OnItemDeleted(UINode* node, const std::string& key) override;

    // used in ArkTS side for tabs.
    LazyForEachChild OnGetChildByIndexNew(int32_t index, std::map<int32_t, LazyForEachChild>& cachedItems,
        std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        return {};
    }

    // used in ArkTS side for tabs.
    void OnExpandChildrenOnInitialInNG() override {}

    // used in ArkTS side.
    void NotifyDataChanged(size_t index, const RefPtr<UINode>& lazyForEachNode, bool isRebuild) override {}

    // used in ArkTS side.
    void NotifyDataDeleted(const RefPtr<UINode>& lazyForEachNode, size_t index, bool removeIds) override {}

    // used in ArkTS side.
    void NotifyDataAdded(size_t index) override {}

    // used in ArkTS side.
    void KeepRemovedItemInCache(
        NG::LazyForEachChild node, std::unordered_map<std::string, NG::LazyForEachCacheChild>& cachedItems) override
    {}

private:
    V2::DataChangeListener* listener_;
    uint32_t totalCount_ = 0;
    void* userData_ = nullptr;
    void (*receiver_)(ArkUINodeAdapterEvent* event) = nullptr;
    ArkUINodeAdapterHandle handle_ = nullptr;
};
} // namespace OHOS::Ace::NG

namespace OHOS::Ace::NodeAdapter {
const ArkUINodeAdapterAPI* GetNodeAdapterAPI();
} // namespace OHOS::Ace::NodeAdapter
