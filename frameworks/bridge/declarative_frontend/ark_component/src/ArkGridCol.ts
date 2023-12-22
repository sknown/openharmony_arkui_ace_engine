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

/// <reference path="./import.ts" />
class GridColSpanModifier extends ModifierWithKey<ArkGridColColumnOption> {
  constructor(value: ArkGridColColumnOption) {
    super(value);
  }
  static identity: Symbol = Symbol('gridColSpan');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().gridCol.resetSpan(node);
    } else {
      if (isNumber(this.value)) {
        GetUINativeModule().gridCol.setSpan(node, this.value,
          this.value, this.value, this.value, this.value, this.value);
      } else {
        GetUINativeModule().gridCol.setSpan(node, this.value.xs,
          this.value.sm, this.value.md, this.value.lg, this.value.xl, this.value.xxl);
      }
    }
  }
  checkObjectDiff(): boolean {
    if (isNumber(this.stageValue) && isNumber(this.value)) {
      return this.stageValue !== this.value;
    } else if (isObject(this.stageValue) && isObject(this.value)) {
      return this.stageValue.xs !== this.value.xs ||
        this.stageValue.sm !== this.value.sm ||
        this.stageValue.md !== this.value.md ||
        this.stageValue.lg !== this.value.lg ||
        this.stageValue.xl !== this.value.xl ||
        this.stageValue.xxl !== this.value.xxl;
    } else {
      return true;
    }
  }
}
class GridColOffsetModifier extends ModifierWithKey<ArkGridColColumnOption> {
  constructor(value: ArkGridColColumnOption) {
    super(value);
  }
  static identity: Symbol = Symbol('gridColOffset');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().gridCol.resetGridColOffset(node);
    } else {
      if (isNumber(this.value)) {
        GetUINativeModule().gridCol.setGridColOffset(node, this.value,
          this.value, this.value, this.value, this.value, this.value);
      } else {
        GetUINativeModule().gridCol.setGridColOffset(node, this.value.xs,
          this.value.sm, this.value.md, this.value.lg, this.value.xl, this.value.xxl);
      }
    }
  }
  checkObjectDiff(): boolean {
    if (isNumber(this.stageValue) && isNumber(this.value)) {
      return this.stageValue !== this.value;
    } else if (isObject(this.stageValue) && isObject(this.value)) {
      return this.stageValue.xs !== this.value.xs ||
        this.stageValue.sm !== this.value.sm ||
        this.stageValue.md !== this.value.md ||
        this.stageValue.lg !== this.value.lg ||
        this.stageValue.xl !== this.value.xl ||
        this.stageValue.xxl !== this.value.xxl;
    } else {
      return true;
    }
  }
}
class GridColOrderModifier extends ModifierWithKey<ArkGridColColumnOption> {
  constructor(value: ArkGridColColumnOption) {
    super(value);
  }
  static identity: Symbol = Symbol('gridColOrder');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().gridCol.resetOrder(node);
    } else {
      if (isNumber(this.value)) {
        GetUINativeModule().gridCol.setOrder(node, this.value,
          this.value, this.value, this.value, this.value, this.value);
      } else {
        GetUINativeModule().gridCol.setOrder(node, this.value.xs,
          this.value.sm, this.value.md, this.value.lg, this.value.xl, this.value.xxl);
      }
    }
  }
  checkObjectDiff(): boolean {
    if (isNumber(this.stageValue) && isNumber(this.value)) {
      return this.stageValue !== this.value;
    } else if (isObject(this.stageValue) && isObject(this.value)) {
      return this.stageValue.xs !== this.value.xs ||
        this.stageValue.sm !== this.value.sm ||
        this.stageValue.md !== this.value.md ||
        this.stageValue.lg !== this.value.lg ||
        this.stageValue.xl !== this.value.xl ||
        this.stageValue.xxl !== this.value.xxl;
    } else {
      return true;
    }
  }
}

class ArkGridColComponent extends ArkComponent implements GridColAttribute {
  constructor(nativePtr: KNode) {
    super(nativePtr);
  }
  span(value: number | GridColColumnOption): GridColAttribute {
    modifierWithKey(this._modifiersWithKeys, GridColSpanModifier.identity, GridColSpanModifier, value);
    return this;
  }
  gridColOffset(value: number | GridColColumnOption): GridColAttribute {
    modifierWithKey(this._modifiersWithKeys, GridColOffsetModifier.identity, GridColOffsetModifier, value);
    return this;
  }
  order(value: number | GridColColumnOption): GridColAttribute {
    modifierWithKey(this._modifiersWithKeys, GridColOrderModifier.identity, GridColOrderModifier, value);
    return this;
  }
}

// @ts-ignore
globalThis.GridCol.attributeModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = GetUINativeModule().getFrameNodeById(elmtId);

  let component = this.createOrGetNode(elmtId, () => {
    return new ArkGridColComponent(nativeNode);
  });
  applyUIAttributes(modifier, nativeNode, component);
  component.applyModifierPatch();
};
