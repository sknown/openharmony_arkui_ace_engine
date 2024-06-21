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

const __relative_container__ = requireInternal('arkui.relativeContainer');

class RelativeContainer extends JSContainerBase {
    static create(value) {
        __relative_container__.create(value);
    }

    static onTouch(value) {
        __Common__.onTouch(value);
    }

    static onKeyEvent(value) {
        __Common__.onKeyEvent(value);
    }

    static onDeleteEvent(value) {
        __Common__.onDeleteEvent(value);
    }

    static onClick(value) {
        __Common__.onClick(value);
    }

    static onHover(value) {
        __Common__.onHover(value);
    }

    static onAppear(value) {
        __Common__.onAppear(value);
    }

    static onDisAppear(value) {
        __Common__.onDisAppear(value);
    }

    static remoteMessage(value) {
        __Common__.remoteMessage(value);
    }
}

export default RelativeContainer;