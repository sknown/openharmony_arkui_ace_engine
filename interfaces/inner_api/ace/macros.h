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

#ifndef FOUNDATION_ACE_INTERFACE_INNERKITS_ACE_MACROS_H
#define FOUNDATION_ACE_INTERFACE_INNERKITS_ACE_MACROS_H

// The macro "ACE_EXPORT_WITH_PREVIEW" is the extension of the macro "ACE_EXPORT"
#ifndef ACE_EXPORT_WITH_PREVIEW
#ifndef WINDOWS_PLATFORM
#define ACE_EXPORT_WITH_PREVIEW __attribute__((visibility("default")))
#else
#define ACE_EXPORT_WITH_PREVIEW __declspec(dllexport)
#endif
#endif

#endif // FOUNDATION_ACE_INTERFACE_INNERKITS_ACE_MACROS_H
