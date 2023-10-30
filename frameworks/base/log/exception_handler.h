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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_LOG_EXCEPTION_HANDLER_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_LOG_EXCEPTION_HANDLER_H

#include <string>

#include "base/utils/macros.h"

namespace OHOS::Ace {
    struct JsErrorObject {
    std::string name;
    std::string message;
    std::string stack;
};
class ACE_FORCE_EXPORT ExceptionHandler {
public:
    static void HandleJsException(
        const std::string& exceptionMsg, const JsErrorObject& errorInfo);
};
} // namespace OHOS::Ace

#endif