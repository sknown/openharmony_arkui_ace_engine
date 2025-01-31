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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_FONT_FONT_PLATFORM_PROXY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_FONT_FONT_PLATFORM_PROXY_H

#include <memory>

#include "base/utils/singleton.h"
#include "core/common/font/font_platform_interface.h"

namespace OHOS::Ace {

class FontPlatformProxy : public Singleton<FontPlatformProxy>, public FontPlatformInterface {
    DECLARE_SINGLETON(FontPlatformProxy);

public:
    void SetDelegate(std::unique_ptr<FontPlatformInterface>&& delegate);
    RefPtr<FontPlatform> GetFontPlatform() const override;

private:
    std::unique_ptr<FontPlatformInterface> delegate_;
};

} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_FONT_FONT_PLATFORM_PROXY_H
