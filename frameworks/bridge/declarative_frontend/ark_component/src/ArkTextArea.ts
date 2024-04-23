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
class TextAreaFontStyleModifier extends ModifierWithKey<FontStyle> {
  constructor(value: FontStyle) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaFontStyle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetFontStyle(node);
    } else {
      getUINativeModule().textArea.setFontStyle(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaDecorationModifier extends ModifierWithKey<{ type: TextDecorationType; color?: ResourceColor }> {
  constructor(value: { type: TextDecorationType; color?: ResourceColor }) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaDecoration');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetDecoration(node);
    } else {
      getUINativeModule().textArea.setDecoration(node, this.value!.type, this.value!.color);
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

class TextAreaLetterSpacingModifier extends ModifierWithKey<number | string> {
  constructor(value: number | string) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaLetterSpacing');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetLetterSpacing(node);
    } else {
      getUINativeModule().textArea.setLetterSpacing(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaLineSpacingModifier extends ModifierWithKey<LengthMetrics> {
  constructor(value: LengthMetrics) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaLineSpacing');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetLineSpacing(node);
    } else if (!isObject(this.value)) {
      getUINativeModule().textArea.resetLineSpacing(node);
    } else {
      getUINativeModule().textArea.setLineSpacing(node, this.value.value, this.value.unit);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaLineHeightModifier extends ModifierWithKey<number | string | Resource> {
  constructor(value: number | string | Resource) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaLineHeight');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetLineHeight(node);
    } else {
      getUINativeModule().textArea.setLineHeight(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaWordBreakModifier extends ModifierWithKey<WordBreak> {
  constructor(value: WordBreak) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaWordBreak');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetWordBreak(node);
    } else {
      getUINativeModule().textArea.setWordBreak(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaCopyOptionModifier extends ModifierWithKey<CopyOptions> {
  constructor(value: CopyOptions) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaCopyOption');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetCopyOption(node);
    } else {
      getUINativeModule().textArea.setCopyOption(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaMaxLinesModifier extends ModifierWithKey<number | undefined> {
  constructor(value: number | undefined) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaMaxLines');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetMaxLines(node);
    } else {
      getUINativeModule().textArea.setMaxLines(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaMinFontSizeModifier extends ModifierWithKey<number | string | Resource> {
  constructor(value: number | string | Resource) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaMinFontSize');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetMinFontSize(node);
    } else {
      getUINativeModule().textArea.setMinFontSize(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaMaxFontSizeModifier extends ModifierWithKey<number | string | Resource> {
  constructor(value: number | string | Resource) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaMaxFontSize');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetMaxFontSize(node);
    } else {
      getUINativeModule().textArea.setMaxFontSize(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaHeightAdaptivePolicyModifier extends ModifierWithKey<TextHeightAdaptivePolicy> {
  constructor(value: TextHeightAdaptivePolicy) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaHeightAdaptivePolicy');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetHeightAdaptivePolicy(node);
    } else {
      getUINativeModule().textArea.setHeightAdaptivePolicy(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaFontSizeModifier extends ModifierWithKey<string | number> {
  constructor(value: string | number) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaFontSize');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetFontSize(node);
    } else {
      getUINativeModule().textArea.setFontSize(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaPlaceholderColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaPlaceholderColor');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetPlaceholderColor(node);
    } else {
      getUINativeModule().textArea.setPlaceholderColor(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaFontColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaFontColor');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetFontColor(node);
    } else {
      getUINativeModule().textArea.setFontColor(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaFontWeightModifier extends ModifierWithKey<number | FontWeight | string> {
  constructor(value: number | FontWeight | string) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaFontWeight');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetFontWeight(node);
    } else {
      getUINativeModule().textArea.setFontWeight(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaBarStateModifier extends ModifierWithKey<BarState> {
  constructor(value: BarState) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaBarState');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetBarState(node);
    } else {
      getUINativeModule().textArea.setBarState(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaEnableKeyboardOnFocusModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaEnableKeyboardOnFocus');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetEnableKeyboardOnFocus(node);
    } else {
      getUINativeModule().textArea.setEnableKeyboardOnFocus(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaFontFamilyModifier extends ModifierWithKey<ResourceColor | string> {
  constructor(value: ResourceColor | string) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaFontFamily');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetFontFamily(node);
    } else {
      getUINativeModule().textArea.setFontFamily(node, this.value!);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaCaretColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaCaretColor');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetCaretColor(node);
    } else {
      getUINativeModule().textArea.setCaretColor(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaMaxLengthModifier extends ModifierWithKey<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaMaxLength');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetMaxLength(node);
    } else {
      getUINativeModule().textArea.setMaxLength(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaStyleModifier extends ModifierWithKey<TextContentStyle> {
  constructor(value: TextContentStyle) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaStyle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetStyle(node);
    } else {
      getUINativeModule().textArea.setStyle(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaSelectionMenuHiddenModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaSelectionMenuHidden');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetSelectionMenuHidden(node);
    } else {
      getUINativeModule().textArea.setSelectionMenuHidden(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaPlaceholderFontModifier extends ModifierWithKey<Font> {
  constructor(value: Font) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaPlaceholderFont');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetPlaceholderFont(node);
    } else {
      getUINativeModule().textArea.setPlaceholderFont(node, this.value.size,
        this.value.weight, this.value.family, this.value.style);
    }
  }

  checkObjectDiff(): boolean {
    if (!(this.stageValue.weight === this.value.weight &&
      this.stageValue.style === this.value.style)) {
      return true;
    } else {
      return !isBaseOrResourceEqual(this.stageValue.size, this.value.size) ||
        !isBaseOrResourceEqual(this.stageValue.family, this.value.family);
    }
  }
}

class TextAreaTextAlignModifier extends ModifierWithKey<TextAlign> {
  constructor(value: TextAlign) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaTextAlign');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetTextAlign(node);
    } else {
      getUINativeModule().textArea.setTextAlign(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaShowCounterModifier extends ModifierWithKey<ArkTextAreaShowCounter> {
  constructor(value: ArkTextAreaShowCounter) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaShowCounter');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetShowCounter(node);
    } else {
      getUINativeModule().textArea.setShowCounter(node, this.value.value!, this.value.options);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue.value, this.value.value) ||
      !isBaseOrResourceEqual(this.stageValue.options, this.value.options);
  }
}

class TextAreaFontFeatureModifier extends ModifierWithKey<FontFeature> {
  constructor(value: FontFeature) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaFontFeature');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetFontFeature(node);
    } else {
      getUINativeModule().textArea.setFontFeature(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaSelectedBackgroundColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaSelectedBackgroundColor');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetSelectedBackgroundColor(node);
    } else {
      getUINativeModule().textArea.setSelectedBackgroundColor(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaCaretStyleModifier extends ModifierWithKey<CaretStyle> {
  constructor(value: CaretStyle) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaCaretStyle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetCaretStyle(node);
    } else {
      getUINativeModule().textArea.setCaretStyle(node, this.value.width!);
    }
  }
  checkObjectDiff(): boolean {
    return this.stageValue !== this.value;
  }
}

class TextAreaTextOverflowModifier extends ModifierWithKey<TextOverflow> {
  constructor(value: TextOverflow) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaTextOverflow');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetTextOverflow(node);
    } else {
      getUINativeModule().textArea.setTextOverflow(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return this.stageValue !== this.value;
  }
}

class TextAreaTextIndentModifier extends ModifierWithKey<Dimension> {
  constructor(value: Dimension) {
    super(value);
  }
  static identity: Symbol = Symbol('textAreaTextIndent');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textArea.resetTextIndent(node);
    } else {
      getUINativeModule().textArea.setTextIndent(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class ArkTextAreaComponent extends ArkComponent implements CommonMethod<TextAreaAttribute> {
  constructor(nativePtr: KNode, classType?: ModifierType) {
    super(nativePtr, classType);
  }
  type(value: TextAreaType): TextAreaAttribute {
    throw new Error('Method not implemented.');
  }
  placeholderColor(value: ResourceColor): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaPlaceholderColorModifier.identity, TextAreaPlaceholderColorModifier, value);
    return this;
  }
  placeholderFont(value: Font): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaPlaceholderFontModifier.identity, TextAreaPlaceholderFontModifier, value);
    return this;
  }

  textAlign(value: TextAlign): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaTextAlignModifier.identity, TextAreaTextAlignModifier, value);
    return this;
  }
  caretColor(value: ResourceColor): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaCaretColorModifier.identity, TextAreaCaretColorModifier, value);
    return this;
  }
  fontColor(value: ResourceColor): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaFontColorModifier.identity, TextAreaFontColorModifier, value);
    return this;
  }
  fontSize(value: Length): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaFontSizeModifier.identity, TextAreaFontSizeModifier, value);
    return this;
  }
  fontStyle(value: FontStyle): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaFontStyleModifier.identity, TextAreaFontStyleModifier, value);
    return this;
  }
  fontWeight(value: number | FontWeight | string): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaFontWeightModifier.identity, TextAreaFontWeightModifier, value);
    return this;
  }
  fontFamily(value: ResourceStr): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaFontFamilyModifier.identity, TextAreaFontFamilyModifier, value);
    return this;
  }
  inputFilter(value: ResourceStr, error?: (value: string) => void): TextAreaAttribute {
    throw new Error('Method not implemented.');
  }
  onChange(callback: (value: string) => void): TextAreaAttribute {
    throw new Error('Method not implemented.');
  }
  onTextSelectionChange(callback: (selectionStart: number, selectionEnd: number) => void): TextAreaAttribute {
    throw new Error('Method not implemented.');
  }
  onContentScroll(callback: (totalOffsetX: number, totalOffsetY: number) => void): TextAreaAttribute {
    throw new Error('Method not implemented.');
  }
  onEditChange(callback: (isEditing: boolean) => void): TextAreaAttribute {
    throw new Error('Method not implemented.');
  }
  onCopy(callback: (value: string) => void): TextAreaAttribute {
    throw new Error('Method not implemented.');
  }
  onCut(callback: (value: string) => void): TextAreaAttribute {
    throw new Error('Method not implemented.');
  }
  onPaste(callback: (value: string) => void): TextAreaAttribute {
    throw new Error('Method not implemented.');
  }
  copyOption(value: CopyOptions): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaCopyOptionModifier.identity, TextAreaCopyOptionModifier, value);
    return this;
  }

  enableKeyboardOnFocus(value: boolean): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaEnableKeyboardOnFocusModifier.identity, TextAreaEnableKeyboardOnFocusModifier, value);
    return this;
  }

  maxLength(value: number): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaMaxLengthModifier.identity, TextAreaMaxLengthModifier, value);
    return this;
  }
  showCounter(value: boolean, options?: InputCounterOptions): TextAreaAttribute {
    let arkValue: ArkTextAreaShowCounter = new ArkTextAreaShowCounter();
    arkValue.value = value;
    arkValue.options = options;
    modifierWithKey(this._modifiersWithKeys, TextAreaShowCounterModifier.identity, TextAreaShowCounterModifier, arkValue);
    return this;
  }
  style(value: TextContentStyle): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaStyleModifier.identity, TextAreaStyleModifier, value);
    return this;
  }
  barState(value: BarState): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaBarStateModifier.identity, TextAreaBarStateModifier, value);
    return this;
  }
  selectionMenuHidden(value: boolean): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaSelectionMenuHiddenModifier.identity, TextAreaSelectionMenuHiddenModifier, value);
    return this;
  }
  maxLines(value: number): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaMaxLinesModifier.identity, TextAreaMaxLinesModifier, value);
    return this;
  }
  fontFeature(value: FontFeature): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaFontFeatureModifier.identity, TextAreaFontFeatureModifier, value);
    return this;
  }
  customKeyboard(value: CustomBuilder): TextAreaAttribute {
    throw new Error('Method not implemented.');
  }
  decoration(value: { type: TextDecorationType; color?: ResourceColor }): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaDecorationModifier.identity, TextAreaDecorationModifier, value);
    return this;
  }
  letterSpacing(value: number | string): this {
    modifierWithKey(this._modifiersWithKeys, TextAreaLetterSpacingModifier.identity, TextAreaLetterSpacingModifier, value);
    return this;
  }
  lineHeight(value: number | string | Resource): this {
    modifierWithKey(this._modifiersWithKeys, TextAreaLineHeightModifier.identity, TextAreaLineHeightModifier, value);
    return this;
  }
  lineSpacing(value: LengthMetrics): this {
    modifierWithKey(this._modifiersWithKeys, TextAreaLineSpacingModifier.identity, TextAreaLineSpacingModifier, value);
    return this;
  }
  wordBreak(value: WordBreak): this {
    modifierWithKey(this._modifiersWithKeys, TextAreaWordBreakModifier.identity, TextAreaWordBreakModifier, value);
    return this;
  }
  minFontSize(value: number | string | Resource): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaMinFontSizeModifier.identity, TextAreaMinFontSizeModifier, value);
    return this;
  }
  maxFontSize(value: number | string | Resource): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaMaxFontSizeModifier.identity, TextAreaMaxFontSizeModifier, value);
    return this;
  }
  heightAdaptivePolicy(value: TextHeightAdaptivePolicy): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaHeightAdaptivePolicyModifier.identity, TextAreaHeightAdaptivePolicyModifier, value);
    return this;
  }
  SelectedBackgroundColor(value: ResourceColor): this {
    modifierWithKey(this._modifiersWithKeys, TextAreaSelectedBackgroundColorModifier.identity, TextAreaSelectedBackgroundColorModifier, value);
    return this;
  }
  caretStyle(value: CaretStyle): this {
    modifierWithKey(this._modifiersWithKeys, TextAreaCaretStyleModifier.identity, TextAreaCaretStyleModifier, value);
    return this;
  }
  textOverflow(value: TextOverflow): this {
    modifierWithKey(this._modifiersWithKeys, TextAreaTextOverflowModifier.identity, TextAreaTextOverflowModifier, value);
    return this;
  }
  textIndent(value: Dimension): this {
    modifierWithKey(this._modifiersWithKeys, TextAreaTextIndentModifier.identity, TextAreaTextIndentModifier, value);
    return this;
  }
}
// @ts-ignore
globalThis.TextArea.attributeModifier = function (modifier: ArkComponent): void {
  attributeModifierFunc.call(this, modifier, (nativePtr: KNode) => {
    return new ArkTextAreaComponent(nativePtr);
  }, (nativePtr: KNode, classType: ModifierType, modifierJS: ModifierJS) => {
    return new modifierJS.TextAreaModifier(nativePtr, classType);
  });
};
