/// <reference path="./import.ts" />
class TextAreaFontStyleModifier extends Modifier<number> {
  static identity: Symbol = Symbol('textAreaFontStyle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetFontStyle(node);
    } else {
      GetUINativeModule().textArea.setFontStyle(node, this.value!);
    }
  }
}

class TextAreaCopyOptionModifier extends Modifier<CopyOptions> {
  static identity: Symbol = Symbol('textAreaCopyOption');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetCopyOption(node);
    } else {
      GetUINativeModule().textArea.setCopyOption(node, this.value!);
    }
  }
}

class TextAreaMaxLinesModifier extends Modifier<number | undefined> {
  static identity: Symbol = Symbol('textAreaMaxLines');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetMaxLines(node);
    } else {
      GetUINativeModule().textArea.setMaxLines(node, this.value!);
    }
  }
}

class TextAreaFontSizeModifier extends ModifierWithKey<string | number> {
  static identity: Symbol = Symbol('textAreaFontSize');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetFontSize(node);
    } else if (!isString(this.value) && !isNumber(this.value) && !isResource(this.value)) {
      GetUINativeModule().textArea.resetFontSize(node);
    } else {
      GetUINativeModule().textArea.setFontSize(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaPlaceholderColorModifier extends ModifierWithKey<ResourceColor> {
  static identity: Symbol = Symbol('textAreaPlaceholderColor');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetPlaceholderColor(node);
    } else {
      GetUINativeModule().textArea.setPlaceholderColor(node, this.value!);
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

class TextAreaFontColorModifier extends ModifierWithKey<ResourceColor> {
  static identity: Symbol = Symbol('textAreaFontColor');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetFontColor(node);
    } else {
      GetUINativeModule().textArea.setFontColor(node, this.value!);
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

class TextAreaFontWeightModifier extends Modifier<number> {
  static identity: Symbol = Symbol('textAreaFontWeight');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetFontWeight(node);
    } else {
      GetUINativeModule().textArea.setFontWeight(node, this.value!);
    }
  }
}

class TextAreaBarStateModifier extends Modifier<number> {
  static identity: Symbol = Symbol('textAreaBarState');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetBarState(node);
    } else {
      GetUINativeModule().textArea.setBarState(node, this.value!);
    }
  }
}

class TextAreaEnableKeyboardOnFocusModifier extends Modifier<boolean> {
  static identity: Symbol = Symbol('textAreaEnableKeyboardOnFocus');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetEnableKeyboardOnFocus(node);
    } else {
      GetUINativeModule().textArea.setEnableKeyboardOnFocus(node, this.value!);
    }
  }
}

class TextAreaFontFamilyModifier extends ModifierWithKey<ResourceColor | string> {
  static identity: Symbol = Symbol('textAreaFontFamily');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetFontFamily(node);
    } else {
      GetUINativeModule().textArea.setFontFamily(node, this.value!);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextAreaCaretColorModifier extends ModifierWithKey<ResourceColor> {
  static identity: Symbol = Symbol('textAreaCaretColor');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetCaretColor(node);
    } else {
      GetUINativeModule().textArea.setCaretColor(node, this.value!);
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

class TextAreaMaxLengthModifier extends Modifier<number> {
  static identity: Symbol = Symbol('textAreaMaxLength');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetMaxLength(node);
    } else {
      GetUINativeModule().textArea.setMaxLength(node, this.value!);
    }
  }
}

class TextAreaStyleModifier extends Modifier<number> {
  static identity: Symbol = Symbol('textAreaStyle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetStyle(node);
    } else {
      GetUINativeModule().textArea.setStyle(node, this.value!);
    }
  }
}

class TextAreaSelectionMenuHiddenModifier extends Modifier<boolean> {
  static identity: Symbol = Symbol('textAreaSelectionMenuHidden');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetSelectionMenuHidden(node);
    } else {
      GetUINativeModule().textArea.setSelectionMenuHidden(node, this.value!);
    }
  }
}

class TextAreaPlaceholderFontModifier extends ModifierWithKey<Font> {
  static identity: Symbol = Symbol('textAreaPlaceholderFont');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetPlaceholderFont(node);
    } else {
      if (!(isNumber(this.value.size)) && !(isString(this.value.size)) &&
        !(isResource(this.value.size))) {
        this.value.size = undefined;
      }
      if (!(isString(this.value.family)) && !(isResource(this.value.family))) {
        this.value.family = undefined;
      }
      GetUINativeModule().textArea.setPlaceholderFont(node, this.value.size,
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

class TextAreaTextAlignModifier extends Modifier<number> {
  static identity: Symbol = Symbol('textAreaTextAlign');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetTextAlign(node);
    } else {
      GetUINativeModule().textArea.setTextAlign(node, this.value!);
    }
  }
}

class TextAreaShowCounterModifier extends Modifier<ArkTextAreaShowCounter> {
  static identity: Symbol = Symbol('textAreaShowCounter');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().textArea.resetShowCounter(node);
    } else {
      GetUINativeModule().textArea.setShowCounter(node, this.value.value!, this.value?.options?.thresholdPercentage);
    }
  }
}

class ArkTextAreaComponent extends ArkComponent implements CommonMethod<TextAreaAttribute> {
  type(value: TextAreaType): TextAreaAttribute {
    throw new Error('Method not implemented.');
  }
  placeholderColor(value: ResourceColor): TextAreaAttribute {
    modifierWithKey(this._modifiersWithKeys, TextAreaPlaceholderColorModifier.identity, TextAreaPlaceholderColorModifier, value);
    return this;
  }
  placeholderFont(value: Font): TextAreaAttribute {
    if (!isLengthType(value.weight)) {
      value.weight = undefined;
    }
    if (!(value.style in FontStyle)) {
      value.style = undefined;
    }
    modifierWithKey(this._modifiersWithKeys, TextAreaPlaceholderFontModifier.identity, TextAreaPlaceholderFontModifier, value);
    return this;
  }

  textAlign(value: TextAlign): TextAreaAttribute {
    if (value) {
      modifier(this._modifiers, TextAreaTextAlignModifier, value);
    } else {
      modifier(this._modifiers, TextAreaTextAlignModifier, undefined);
    }
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
    if (value in FontStyle) {
      modifier(this._modifiers, TextAreaFontStyleModifier, value);
    } else {
      modifier(this._modifiers, TextAreaFontStyleModifier, undefined);
    }
    return this;
  }
  fontWeight(value: number | FontWeight | string): TextAreaAttribute {
    if (!isLengthType(value)) {
      modifier(this._modifiers, TextAreaFontWeightModifier, undefined);
    } else {
      modifier(this._modifiers, TextAreaFontWeightModifier, value);
    }
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
    if (isNumber(value)) {
      modifier(this._modifiers, TextAreaCopyOptionModifier, value);
    } else {
      modifier(this._modifiers, TextAreaCopyOptionModifier, undefined);
    }
    return this;
  }

  enableKeyboardOnFocus(value: boolean): TextAreaAttribute {
    if (value) {
      modifier(this._modifiers, TextAreaEnableKeyboardOnFocusModifier, value);
    } else {
      modifier(this._modifiers, TextAreaEnableKeyboardOnFocusModifier, undefined);
    }
    return this;
  }

  maxLength(value: number): TextAreaAttribute {
    if (isNumber(value)) {
      modifier(this._modifiers, TextAreaMaxLengthModifier, value);
    } else {
      modifier(this._modifiers, TextAreaMaxLengthModifier, undefined);
    }
    return this;
  }
  showCounter(value: boolean, options?: InputCounterOptions): TextAreaAttribute {
    let arkValue: ArkTextAreaShowCounter = new ArkTextAreaShowCounter();
    arkValue.value = value;
    arkValue.options = options;
    modifier(this._modifiers, TextAreaShowCounterModifier, arkValue);
    return this;
  }
  style(value: TextContentStyle): TextAreaAttribute {
    if (value) {
      modifier(this._modifiers, TextAreaStyleModifier, value);
    } else {
      modifier(this._modifiers, TextAreaStyleModifier, undefined);
    }
    return this;
  }
  barState(value: BarState): TextAreaAttribute {
    if (isNumber(value)) {
      modifier(this._modifiers, TextAreaBarStateModifier, value);
    } else {
      modifier(this._modifiers, TextAreaBarStateModifier, undefined);
    }
    return this;
  }
  selectionMenuHidden(value: boolean): TextAreaAttribute {
    if (value) {
      modifier(this._modifiers, TextAreaSelectionMenuHiddenModifier, value);
    } else {
      modifier(this._modifiers, TextAreaSelectionMenuHiddenModifier, undefined);
    }
    return this;
  }
  maxLines(value: number): TextAreaAttribute {
    if (!isNumber(value)) {
      modifier(this._modifiers, TextAreaMaxLinesModifier, undefined);
    } else {
      modifier(this._modifiers, TextAreaMaxLinesModifier, value);
    }
    return this;
  }
  customKeyboard(value: CustomBuilder): TextAreaAttribute {
    throw new Error('Method not implemented.');
  }
}
// @ts-ignore
globalThis.TextArea.attributeModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = GetUINativeModule().getFrameNodeById(elmtId);

  let component = this.createOrGetNode(elmtId, () => {
    return new ArkTextAreaComponent(nativeNode);
  });
  modifier.applyNormalAttribute(component);
  component.applyModifierPatch();

}