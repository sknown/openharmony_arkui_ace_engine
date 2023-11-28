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

declare class __JSBaseNode__ {
    build(builder: any, params: Object): void;
    markDirty(flag: DirtyFlag): void;
}

enum DirtyFlag {
    UPDATE_PROPERTY = 0,
    UPDATE_CONTENT,
}

class BaseNode extends __JSBaseNode__ {
}
