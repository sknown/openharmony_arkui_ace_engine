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
class FontColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity: Symbol = Symbol('textFontColor');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetFontColor(node);
    } else {
      GetUINativeModule().text.setFontColor(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    if (isResource(this.stageValue) && isResource(this.value)) {
      return !isResourceEqual(this.stageValue, this.value);
    } else {
      return true;
    }
  }
}

class FontSizeModifier extends ModifierWithKey<number | string | Resource> {
  constructor(value: number | string | Resource) {
    super(value);
  }
  static identity: Symbol = Symbol('textFontSize');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetFontSize(node);
    } else {
      GetUINativeModule().text.setFontSize(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    if (isResource(this.stageValue) && isResource(this.value)) {
      return !isResourceEqual(this.stageValue, this.value);
    } else {
      return true;
    }
  }
}

class FontWeightModifier extends ModifierWithKey<string> {
  constructor(value: string) {
    super(value);
  }
  static identity: Symbol = Symbol('textFontWeight');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetFontWeight(node);
    } else {
      GetUINativeModule().text.setFontWeight(node, this.value);
    }
  }
}

class FontStyleModifier extends ModifierWithKey<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('textFontStyle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetFontStyle(node);
    } else {
      GetUINativeModule().text.setFontStyle(node, this.value);
    }
  }
}

class TextAlignModifier extends ModifierWithKey<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('textAlign');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetTextAlign(node);
    } else {
      GetUINativeModule().text.setTextAlign(node, this.value);
    }
  }
}

class TextHeightAdaptivePolicyModifier extends ModifierWithKey<TextHeightAdaptivePolicy> {
  constructor(value: TextHeightAdaptivePolicy) {
    super(value);
  }
  static identity: Symbol = Symbol('textHeightAdaptivePolicy');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetHeightAdaptivePolicy(node);
    } else {
      GetUINativeModule().text.setHeightAdaptivePolicy(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextDraggableModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('textDraggable');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetDraggable(node);
    } else {
      GetUINativeModule().text.setDraggable(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextMinFontSizeModifier extends ModifierWithKey<number | string | Resource> {
  constructor(value: number | string | Resource) {
    super(value);
  }
  static identity: Symbol = Symbol('textMinFontSize');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetMinFontSize(node);
    } else if (!isNumber(this.value) && !isString(this.value) && !isResource(this.value)) {
      GetUINativeModule().text.resetMinFontSize(node);
    } else {
      GetUINativeModule().text.setMinFontSize(node, this.value!);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextMaxFontSizeModifier extends ModifierWithKey<number | string | Resource> {
  constructor(value: number | string | Resource) {
    super(value);
  }
  static identity: Symbol = Symbol('textMaxFontSize');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetMaxFontSize(node);
    } else if (!isNumber(this.value) && !isString(this.value) && !isResource(this.value)) {
      GetUINativeModule().text.resetMaxFontSize(node);
    } else {
      GetUINativeModule().text.setMaxFontSize(node, this.value!);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextLineHeightModifier extends ModifierWithKey<number | string | Resource> {
  constructor(value: number | string | Resource) {
    super(value);
  }
  static identity: Symbol = Symbol('textLineHeight');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetLineHeight(node);
    } else if (!isNumber(this.value) && !isString(this.value) && !isResource(this.value)) {
      GetUINativeModule().text.resetLineHeight(node);
    } else {
      GetUINativeModule().text.setLineHeight(node, this.value!);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextCopyOptionModifier extends ModifierWithKey<CopyOptions> {
  constructor(value: CopyOptions) {
    super(value);
  }
  static identity: Symbol = Symbol('textCopyOption');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetCopyOption(node);
    } else {
      GetUINativeModule().text.setCopyOption(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextFontFamilyModifier extends ModifierWithKey<string | Resource> {
  constructor(value: string | Resource) {
    super(value);
  }
  static identity: Symbol = Symbol('textFontFamily');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetFontFamily(node);
    } else if (!isString(this.value) && !isResource(this.value)) {
      GetUINativeModule().text.resetFontFamily(node);
    } else {
      GetUINativeModule().text.setFontFamily(node, this.value!);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextMaxLinesModifier extends ModifierWithKey<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('textMaxLines');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetMaxLines(node);
    } else if (!isNumber(this.value)) {
      GetUINativeModule().text.resetMaxLines(node);
    } else {
      GetUINativeModule().text.setMaxLines(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextLetterSpacingModifier extends ModifierWithKey<number | string> {
  constructor(value: number | string) {
    super(value);
  }
  static identity: Symbol = Symbol('textLetterSpacing');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetLetterSpacing(node);
    } else if (!isNumber(this.value) && !isString(this.value)) {
      GetUINativeModule().text.resetLetterSpacing(node);
    } else {
      GetUINativeModule().text.setLetterSpacing(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextTextOverflowModifier extends ModifierWithKey<{ overflow: TextOverflow }> {
  constructor(value: { overflow: TextOverflow }) {
    super(value);
  }
  static identity: Symbol = Symbol('textTextOverflow');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetTextOverflow(node);
    } else {
      GetUINativeModule().text.setTextOverflow(node, this.value.overflow);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue.overflow, this.value.overflow);
  }
}

class TextBaselineOffsetModifier extends ModifierWithKey<number | string> {
  constructor(value: number | string) {
    super(value);
  }
  static identity: symbol = Symbol('textBaselineOffset');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetBaselineOffset(node);
    } else if (!isNumber(this.value) && !isString(this.value)) {
      GetUINativeModule().text.resetBaselineOffset(node);
    } else {
      GetUINativeModule().text.setBaselineOffset(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextTextCaseModifier extends ModifierWithKey<TextCase> {
  constructor(value: TextCase) {
    super(value);
  }
  static identity: symbol = Symbol('textTextCase');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetTextCase(node);
    } else {
      GetUINativeModule().text.setTextCase(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextTextIndentModifier extends ModifierWithKey<Length> {
  constructor(value: Length) {
    super(value);
  }
  static identity: symbol = Symbol('textTextIndent');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetTextIndent(node);
    } else if (!isNumber(this.value) && !isString(this.value) && !isResource(this.value)) {
      GetUINativeModule().text.resetTextIndent(node);
    } else {
      GetUINativeModule().text.setTextIndent(node, this.value!);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextTextShadowModifier extends ModifierWithKey<ShadowOptions | Array<ShadowOptions>> {
  constructor(value: ShadowOptions | Array<ShadowOptions>) {
    super(value);
  }
  static identity: Symbol = Symbol('textTextShadow');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetTextShadow(node);
    } else {
      let shadow: ArkShadowInfoToArray = new ArkShadowInfoToArray();
      if (!shadow.convertShadowOptions(this.value)) {
        GetUINativeModule().text.resetTextShadow(node);
      } else {
        GetUINativeModule().text.setTextShadow(node, shadow.radius,
          shadow.type, shadow.color, shadow.offsetX, shadow.offsetY, shadow.fill, shadow.radius.length);
      }
    }
  }

  checkObjectDiff(): boolean {
    let checkDiff = true;
    let arkShadow = new ArkShadowInfoToArray();
    if (Object.getPrototypeOf(this.stageValue).constructor === Object &&
      Object.getPrototypeOf(this.value).constructor === Object) {
      checkDiff = arkShadow.checkDiff(<ShadowOptions>this.stageValue, <ShadowOptions>this.value);
    } else if (Object.getPrototypeOf(this.stageValue).constructor === Array &&
      Object.getPrototypeOf(this.value).constructor === Array &&
      (<Array<ShadowOptions>>this.stageValue).length === (<Array<ShadowOptions>>this.value).length) {
      let isDiffItem = false;
      for (let i: number = 0; i < (<Array<ShadowOptions>>this.value).length; i++) {
        if (arkShadow.checkDiff(this.stageValue[i], this.value[1])) {
          isDiffItem = true;
          break;
        }
      }
      if (!isDiffItem) {
        checkDiff = false;
      }
    }
    return checkDiff;
  }
}

class TextDecorationModifier extends ModifierWithKey<{ type: TextDecorationType; color?: ResourceColor }> {
  constructor(value: { type: TextDecorationType; color?: ResourceColor }) {
    super(value);
  }
  static identity: Symbol = Symbol('textDecoration');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetDecoration(node);
    } else {
      GetUINativeModule().text.setDecoration(node, this.value!.type, this.value!.color);
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

class TextFontModifier extends ModifierWithKey<Font> {
  constructor(value: Font) {
    super(value);
  }
  static identity: Symbol = Symbol('textFont');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().text.resetFont(node);
    } else {
      GetUINativeModule().text.setFont(node, this.value.size, this.value.weight, this.value.family, this.value.style);
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
class ArkTextComponent extends ArkComponent implements TextAttribute {
  constructor(nativePtr: KNode) {
    super(nativePtr);
  }
  enableDataDetector(enable: boolean): this {
    throw new Error('Method not implemented.');
  }
  dataDetectorConfig(config: any): this {
    throw new Error('Method not implemented.');
  }
  onGestureJudgeBegin(callback: (gestureInfo: GestureInfo, event: BaseGestureEvent) => GestureJudgeResult): this {
    throw new Error('Method not implemented.');
  }
  font(value: Font): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextFontModifier.identity, TextFontModifier, value);
    return this;
  }
  fontColor(value: ResourceColor): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, FontColorModifier.identity, FontColorModifier, value);
    return this;
  }
  fontSize(value: any): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, FontSizeModifier.identity, FontSizeModifier, value);
    return this;
  }
  minFontSize(value: number | string | Resource): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextMinFontSizeModifier.identity, TextMinFontSizeModifier, value);
    return this;
  }
  maxFontSize(value: number | string | Resource): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextMaxFontSizeModifier.identity, TextMaxFontSizeModifier, value);
    return this;
  }
  fontStyle(value: FontStyle): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, FontStyleModifier.identity, FontStyleModifier, value);
    return this;
  }
  fontWeight(value: number | FontWeight | string): TextAttribute {
    let fontWeightStr: string = '400';
    if (isNumber(value)) {
      if (value === 0) {
        fontWeightStr = 'Lighter';
      } else if (value === 1) {
        fontWeightStr = 'Normal';
      } else if (value === 2) {
        fontWeightStr = 'Regular';
      } else if (value === 3) {
        fontWeightStr = 'Medium';
      } else if (value === 4) {
        fontWeightStr = 'Bold';
      } else if (value === 5) {
        fontWeightStr = 'Bolder';
      } else {
        fontWeightStr = value.toString();
      }
    } else if (isString(value)) {
      fontWeightStr = String(value);
    }
    modifierWithKey(this._modifiersWithKeys, FontWeightModifier.identity, FontWeightModifier, fontWeightStr);
    return this;
  }
  textAlign(value: TextAlign): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAlignModifier.identity, TextAlignModifier, value);
    return this;
  }
  lineHeight(value: number | string | Resource): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextLineHeightModifier.identity, TextLineHeightModifier, value);
    return this;
  }
  textOverflow(value: { overflow: TextOverflow }): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextTextOverflowModifier.identity, TextTextOverflowModifier, value);
    return this;
  }
  fontFamily(value: string | Resource): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextFontFamilyModifier.identity, TextFontFamilyModifier, value);
    return this;
  }
  maxLines(value: number): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextMaxLinesModifier.identity, TextMaxLinesModifier, value);
    return this;
  }
  decoration(value: { type: TextDecorationType; color?: ResourceColor }): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextDecorationModifier.identity, TextDecorationModifier, value);
    return this;
  }
  letterSpacing(value: number | string): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextLetterSpacingModifier.identity, TextLetterSpacingModifier, value);
    return this;
  }
  textCase(value: TextCase): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextTextCaseModifier.identity, TextTextCaseModifier, value);
    return this;
  }
  baselineOffset(value: number | string): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextBaselineOffsetModifier.identity, TextBaselineOffsetModifier, value);
    return this;
  }
  copyOption(value: CopyOptions): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextCopyOptionModifier.identity, TextCopyOptionModifier, value);
    return this;
  }
  draggable(value: boolean): this {
    modifierWithKey(this._modifiersWithKeys, TextDraggableModifier.identity, TextDraggableModifier, value);
    return this;
  }
  textShadow(value: ShadowOptions | Array<ShadowOptions>): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextTextShadowModifier.identity, TextTextShadowModifier, value);
    return this;
  }
  heightAdaptivePolicy(value: TextHeightAdaptivePolicy): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextHeightAdaptivePolicyModifier.identity, TextHeightAdaptivePolicyModifier, value)
    return this;
  }
  textIndent(value: Length): TextAttribute {
    modifierWithKey(this._modifiersWithKeys, TextTextIndentModifier.identity, TextTextIndentModifier, value);
    return this;
  }
  wordBreak(value: WordBreak): TextAttribute {
    throw new Error('Method not implemented.');
  }
  onCopy(callback: (value: string) => void): TextAttribute {
    throw new Error('Method not implemented.');
  }
  selection(selectionStart: number, selectionEnd: number): TextAttribute {
    throw new Error('Method not implemented.');
  }
  ellipsisMode(value: EllipsisMode): TextAttribute {
    throw new Error('Method not implemented.');
  }
}
// @ts-ignore
globalThis.Text.attributeModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = GetUINativeModule().getFrameNodeById(elmtId);

  let component = this.createOrGetNode(elmtId, () => {
    return new ArkTextComponent(nativeNode);
  });
  applyUIAttributes(modifier, nativeNode, component);
  component.applyModifierPatch();
};
