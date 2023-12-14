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

/// <reference path='./import.ts' />
class ArkPathComponent extends ArkCommonShapeComponent implements PathAttribute {
  constructor(nativePtr: KNode) {
    super(nativePtr);
  }
  commands(value: string): this {
    modifierWithKey(this._modifiersWithKeys, CommandsModifier.identity, CommandsModifier, value);
    return this;
  }
}

class CommandsModifier extends ModifierWithKey<string> {
  constructor(value: string) {
    super(value);
  }
  static identity: Symbol = Symbol('commands');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().path.resetPathCommands(node);
    } else {
      GetUINativeModule().path.setPathCommands(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    if (isString(this.stageValue) && isString(this.value)) {
      return this.stageValue !== this.value;
    } else {
      return true;
    }
  }
}

// @ts-ignore
globalThis.Path.attributeModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = GetUINativeModule().getFrameNodeById(elmtId);
  let component = this.createOrGetNode(elmtId, () => {
    return new ArkPathComponent(nativeNode);
  });
  modifier.applyNormalAttribute(component);
  component.applyModifierPatch();
}
