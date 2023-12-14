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
class SpanFontSizeModifier extends ModifierWithKey<Length> {
  constructor(value: Length) {
    super(value);
  }
  static identity: Symbol = Symbol('spanFontSize');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().span.resetFontSize(node);
    } else {
      GetUINativeModule().span.setFontSize(node, this.value!);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class SpanFontFamilyModifier extends ModifierWithKey<string | Resource> {
  constructor(value: string | Resource) {
    super(value);
  }
  static identity: Symbol = Symbol('spanFontFamily');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().span.resetFontFamily(node);
    } else {
      GetUINativeModule().span.setFontFamily(node, this.value!);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class SpanLineHeightModifier extends ModifierWithKey<Length> {
  constructor(value: Length) {
    super(value);
  }
  static identity: Symbol = Symbol('spanLineHeight');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().span.resetLineHeight(node);
    } else {
      GetUINativeModule().span.setLineHeight(node, this.value!);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class SpanFontStyleModifier extends ModifierWithKey<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('spanFontStyle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().span.resetFontStyle(node);
    } else {
      GetUINativeModule().span.setFontStyle(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class SpanTextCaseModifier extends ModifierWithKey<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('spanTextCase');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().span.resetTextCase(node);
    } else {
      GetUINativeModule().span.setTextCase(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class SpanFontColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity = Symbol('spanFontColor');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().span.resetFontColor(node);
    } else {
      GetUINativeModule().span.setFontColor(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class SpanLetterSpacingModifier extends ModifierWithKey<string> {
  constructor(value: string) {
    super(value);
  }
  static identity = Symbol('spanLetterSpacing');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().span.resetLetterSpacing(node);
    } else {
      GetUINativeModule().span.setLetterSpacing(node, this.value!);
    }
  }
}
class SpanFontModifier extends ModifierWithKey<Font> {
  constructor(value: Font) {
    super(value);
  }
  static identity = Symbol('spanFont');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().span.resetFont(node);
    } else {
      GetUINativeModule().span.setFont(node, this.value.size, this.value.weight, this.value.family, this.value.style);
    }
  }

  checkObjectDiff(): boolean {
    if (this.stageValue.weight !== this.value.weight || this.stageValue.style !== this.value.style) {
      return true;
    }
    if (((isResource(this.stageValue.size) && isResource(this.value.size) &&
      isResourceEqual(this.stageValue.size, this.value.size)) ||
      (!isResource(this.stageValue.size) && !isResource(this.value.size) &&
        this.stageValue.size === this.value.size)) &&
      ((isResource(this.stageValue.family) && isResource(this.value.family) &&
        isResourceEqual(this.stageValue.family, this.value.family)) ||
        (!isResource(this.stageValue.family) && !isResource(this.value.family) &&
          this.stageValue.family === this.value.family))) {
      return false;
    } else {
      return true;
    }
  }
}
class SpanDecorationModifier extends ModifierWithKey<{ type: TextDecorationType, color?: ResourceColor }> {
  constructor(value: { type: TextDecorationType, color?: ResourceColor }) {
    super(value);
  }
  static identity = Symbol('spanDecoration');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().span.resetDecoration(node);
    } else {
      GetUINativeModule().span.setDecoration(node, this.value.type, this.value.color);
    }
  }

  checkObjectDiff(): boolean {
    if (this.stageValue.type !== this.value.type) {
      return true;
    }
    if (isResource(this.stageValue.color) && isResource(this.value.color)) {
      return !isResourceEqual(this.stageValue.color, this.value.color);
    } else if (!isResource(this.stageValue.color) && !isResource(this.value.color)) {
      return !(this.stageValue.color === this.value.color);
    } else {
      return true;
    }
  }
}
class SpanFontWeightModifier extends ModifierWithKey<string> {
  constructor(value: string) {
    super(value);
  }
  static identity = Symbol('spanfontweight');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().span.resetFontWeight(node);
    } else {
      GetUINativeModule().span.setFontWeight(node, this.value!);
    }
  }
}

class ArkSpanComponent extends ArkComponent implements CommonMethod<SpanAttribute> {
  constructor(nativePtr: KNode) {
    super(nativePtr);
  }
  decoration(value: { type: TextDecorationType, color?: ResourceColor }): SpanAttribute {
    modifierWithKey(this._modifiersWithKeys, SpanDecorationModifier.identity, SpanDecorationModifier, value);
    return this;
  }
  font(value: Font): SpanAttribute {
    modifierWithKey(this._modifiersWithKeys, SpanFontModifier.identity, SpanFontModifier, value);
    return this;
  }
  lineHeight(value: Length): SpanAttribute {
    modifierWithKey(this._modifiersWithKeys, SpanLineHeightModifier.identity, SpanLineHeightModifier, value);
    return this;
  }
  fontSize(value: Length): SpanAttribute {
    modifierWithKey(this._modifiersWithKeys, SpanFontSizeModifier.identity, SpanFontSizeModifier, value);
    return this;
  }
  fontColor(value: ResourceColor): SpanAttribute {
    modifierWithKey(this._modifiersWithKeys, SpanFontColorModifier.identity, SpanFontColorModifier, value);
    return this;
  }
  fontStyle(value: FontStyle): SpanAttribute {
    modifierWithKey(this._modifiersWithKeys, SpanFontStyleModifier.identity, SpanFontStyleModifier, value);
    return this;
  }
  fontWeight(value: number | FontWeight | string): SpanAttribute {
    modifierWithKey(this._modifiersWithKeys, SpanFontWeightModifier.identity, SpanFontWeightModifier, value);
    return this;
  }
  fontFamily(value: string | Resource): SpanAttribute {
    modifierWithKey(this._modifiersWithKeys, SpanFontFamilyModifier.identity, SpanFontFamilyModifier, value);
    return this;
  }
  letterSpacing(value: number | string): SpanAttribute {
    modifierWithKey(this._modifiersWithKeys, SpanLetterSpacingModifier.identity, SpanLetterSpacingModifier, value);
    return this;
  }
  textCase(value: TextCase): SpanAttribute {
    modifierWithKey(this._modifiersWithKeys, SpanTextCaseModifier.identity, SpanTextCaseModifier, value);
    return this;
  }
}
// @ts-ignore
globalThis.Span.attributeModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = GetUINativeModule().getFrameNodeById(elmtId);

  let component = this.createOrGetNode(elmtId, () => {
    return new ArkSpanComponent(nativeNode);
  });
  applyUIAttributes(modifier, nativeNode, component);
  component.applyModifierPatch();

}
