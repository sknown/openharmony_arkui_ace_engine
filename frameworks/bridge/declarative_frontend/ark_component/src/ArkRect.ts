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

/// <reference path="./import.ts" />
/// <reference path="./ArkCommonShape.ts" />
class RectRadiusWidthModifier extends ModifierWithKey<string | number> {
  constructor(value: string | number) {
    super(value);
  }
  static identity: Symbol = Symbol('rectRadiusWidth');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().rect.resetRectRadiusWidth(node);
    } else {
      GetUINativeModule().rect.setRectRadiusWidth(node, this.value);
    }
  }
}
class RectRadiusHeightModifier extends ModifierWithKey<string | number> {
  constructor(value: string | number) {
    super(value);
  }
  static identity: Symbol = Symbol('rectRadiusHeight');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().rect.resetRectRadiusHeight(node);
    } else {
      GetUINativeModule().rect.setRectRadiusHeight(node, this.value);
    }
  }
}
class RectRadiusModifier extends ModifierWithKey<string | number | Array<any>> {
  constructor(value: string | number | Array<any>) {
    super(value);
  }
  static identity: Symbol = Symbol('rectRadius');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().rect.resetRectRadius(node);
  } else {
      GetUINativeModule().rect.setRectRadius(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !(this.stageValue === this.value);
  }
}
class ArkRectComponent extends ArkCommonShapeComponent implements RectAttribute {
  constructor(nativePtr: KNode) {
    super(nativePtr);
  }
  radiusWidth(value: string | number): this {
    modifierWithKey(this._modifiersWithKeys, RectRadiusWidthModifier.identity, RectRadiusWidthModifier, value);
    return this;
  }
  radiusHeight(value: string | number): this {
    modifierWithKey(this._modifiersWithKeys, RectRadiusHeightModifier.identity, RectRadiusHeightModifier, value);
    return this;
  }
  radius(value: string | number | Array<any>): this {
    modifierWithKey(this._modifiersWithKeys, RectRadiusModifier.identity, RectRadiusModifier, value);
    return this;
  }
}

// @ts-ignore
globalThis.Rect.attributeModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = GetUINativeModule().getFrameNodeById(elmtId);
  let component = this.createOrGetNode(elmtId, () => {
    return new ArkRectComponent(nativeNode);
  });
  modifier.applyNormalAttribute(component);
  component.applyModifierPatch();
}
