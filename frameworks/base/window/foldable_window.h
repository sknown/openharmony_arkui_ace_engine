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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_SHEETWINDOW_SHEET_WINDOW_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_SHEETWINDOW_SHEET_WINDOW_H

#include "base/memory/ace_type.h"

namespace OHOS::Ace {
class ACE_EXPORT FoldableWindow : public AceType {
    DECLARE_ACE_TYPE(FoldableWindow, AceType)

public:
    static RefPtr<FoldableWindow> CreateFoldableWindow(int32_t instanceId);

    virtual bool IsFoldExpand() = 0;

    int32_t GetFoldableWindowId() const
    {
        return foldableWindowId_;
    }

    void SetFoldableWindowId(int32_t id)
    {
        foldableWindowId_ = id;
    }

private:
    int32_t foldableWindowId_ = 0;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_SHEETWINDOW_SHEET_WINDOW_H