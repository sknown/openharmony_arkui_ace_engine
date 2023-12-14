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
class ArkTabsComponent extends ArkComponent implements TabsAttribute {
  constructor(nativePtr: KNode) {
    super(nativePtr);
  }
  onAnimationStart(handler: (index: number, targetIndex: number, event: TabsAnimationEvent) => void): TabsAttribute {
    throw new Error('Method not implemented.');
  }
  onAnimationEnd(handler: (index: number, event: TabsAnimationEvent) => void): TabsAttribute {
    throw new Error('Method not implemented.');
  }
  onGestureSwipe(handler: (index: number, event: TabsAnimationEvent) => void): TabsAttribute {
    throw new Error('Method not implemented.');
  }
  vertical(value: boolean): TabsAttribute {
    modifier(this._modifiers, TabsVerticalModifier, value);
    return this;
  }
  barPosition(value: BarPosition): TabsAttribute {
    modifier(this._modifiers, BarPositionModifier, value);
    return this;
  }
  scrollable(value: boolean): TabsAttribute {
    modifier(this._modifiers, ScrollableModifier, value);
    return this;
  }
  barMode(value: BarMode, options?: ScrollableBarModeOptions | undefined): TabsAttribute {
    let arkBarMode = new ArkBarMode();
    arkBarMode.barMode = value;
    arkBarMode.options = options;
    modifierWithKey(this._modifiersWithKeys, TabBarModeModifier.identity, TabBarModeModifier, arkBarMode);
    return this;
  }

  barWidth(value: Length): TabsAttribute {
    modifierWithKey(this._modifiersWithKeys, BarWidthModifier.identity, BarWidthModifier, value);

    return this;
  }
  barHeight(value: Length): TabsAttribute {
    if (isUndefined(value) || isNull(value)) {
      modifierWithKey(this._modifiersWithKeys, BarHeightModifier.identity, BarHeightModifier, undefined);
    } else {
      modifierWithKey(this._modifiersWithKeys, BarHeightModifier.identity, BarHeightModifier, value);
    }

    return this;
  }
  animationDuration(value: number): TabsAttribute {
    modifier(this._modifiers, AnimationDurationModifier, value);
    return this;
  }
  onChange(event: (index: number) => void): TabsAttribute {
    throw new Error('Method not implemented.');
  }
  onTabBarClick(event: (index: number) => void): TabsAttribute {
    throw new Error('Method not implemented.');
  }
  fadingEdge(value: boolean): TabsAttribute {
    modifier(this._modifiers, FadingEdgeModifier, value);
    return this;
  }
  divider(value: DividerStyle | null): TabsAttribute {
    modifierWithKey(this._modifiersWithKeys, DividerModifier.identity, DividerModifier, value);
    return this;
  }
  barOverlap(value: boolean): TabsAttribute {
    modifier(this._modifiers, BarOverlapModifier, value);
    return this;
  }
  barBackgroundColor(value: ResourceColor): TabsAttribute {
    modifierWithKey(this._modifiersWithKeys, BarBackgroundColorModifier.identity, BarBackgroundColorModifier, value);
    return this;
  }
  barGridAlign(value: BarGridColumnOptions): TabsAttribute {
    modifierWithKey(this._modifiersWithKeys, BarGridAlignModifier.identity, BarGridAlignModifier, value);
    return this;
  }
}

class BarGridAlignModifier extends ModifierWithKey<BarGridColumnOptions> {
  constructor(value: BarGridColumnOptions) {
    super(value);
  }
  static identity: Symbol = Symbol('barGridAlign');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetBarGridAlign(node);
    } else {
      GetUINativeModule().tabs.setBarGridAlign(node, this.value.sm,
        this.value.md, this.value.lg, this.value.gutter, this.value.margin);
    }
  }

  checkObjectDiff(): boolean {
    return !(this.stageValue.sm === this.value.sm &&
      this.stageValue.md === this.value.md &&
      this.stageValue.lg === this.value.lg &&
      this.stageValue.gutter === this.value.gutter &&
      this.stageValue.margin === this.value.margin);
  }
}

class DividerModifier extends ModifierWithKey<DividerStyle> {
  constructor(value: DividerStyle) {
    super(value);
  }
  static identity: Symbol = Symbol('Divider');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetDivider(node);
    } else {
      GetUINativeModule().tabs.setDivider(node, this.value.strokeWidth,
        this.value.color, this.value.startMargin, this.value.endMargin);
    }
  }

  checkObjectDiff(): boolean {
    return !(this.stageValue.strokeWidth === this.value.strokeWidth &&
      this.stageValue.color === this.value.color &&
      this.stageValue.startMargin === this.value.startMargin &&
      this.stageValue.endMargin === this.value.endMargin);
  }
}

class BarWidthModifier extends ModifierWithKey<Length> {
  constructor(value: Length) {
    super(value);
  }
  static identity: Symbol = Symbol('barWidth');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetTabBarWidth(node);
    } else {
      GetUINativeModule().tabs.setTabBarWidth(node, this.value);
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

class BarAdaptiveHeightModifier extends Modifier<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('barAdaptiveHeight');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetBarAdaptiveHeight(node);
    } else {
      GetUINativeModule().tabs.setBarAdaptiveHeight(node, this.value);
    }
  }
}

class BarHeightModifier extends ModifierWithKey<Length> {
  constructor(value: Length) {
    super(value);
  }
  static identity: Symbol = Symbol('barHeight');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetTabBarHeight(node);
    } else {
      GetUINativeModule().tabs.setTabBarHeight(node, this.value);
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

class BarOverlapModifier extends Modifier<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('barOverlap');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetBarOverlap(node);
    } else {
      GetUINativeModule().tabs.setBarOverlap(node, this.value);
    }
  }
}

class TabsVerticalModifier extends Modifier<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('vertical');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetIsVertical(node);
    } else {
      GetUINativeModule().tabs.setIsVertical(node, this.value);
    }
  }
}

class AnimationDurationModifier extends Modifier<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('animationduration');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetAnimationDuration(node);
    } else {
      GetUINativeModule().tabs.setAnimationDuration(node, this.value);
    }
  }
}

class ScrollableModifier extends Modifier<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('scrollable');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetScrollable(node);
    } else {
      GetUINativeModule().tabs.setScrollable(node, this.value);
    }
  }
}

class TabBarModeModifier extends ModifierWithKey<ArkBarMode> {
  constructor(value: ArkBarMode) {
    super(value);
  }
  static identity: Symbol = Symbol('tabsbarMode');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetTabBarMode(node);
    } else {
      GetUINativeModule().tabs.setTabBarMode(node, this.value.barMode
        , this.value.options?.margin
        , this.value.options?.nonScrollableLayoutStyle);
    }
  }

  checkObjectDiff(): boolean {
    if (isResource(this.stageValue) && isResource(this.value)) {
      return !isResourceEqual(this.stageValue, this.value);
    } else if (!isResource(this.stageValue) && !isResource(this.value)) {
      return !(this.value.barMode === this.stageValue.barMode &&
        this.value.options?.margin === this.stageValue.options?.margin &&
        this.value.options?.nonScrollableLayoutStyle === this.stageValue.options?.nonScrollableLayoutStyle);
    }
    else {
      return true;
    }
  }
}

class BarPositionModifier extends Modifier<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('barPosition');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetTabBarPosition(node);
    } else {
      GetUINativeModule().tabs.setTabBarPosition(node, this.value);
    }
  }
}

class TabsHideTitleBarModifier extends Modifier<string> {
  constructor(value: string) {
    super(value);
  }
  static identity: Symbol = Symbol('hideTitleBar');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetHideTitleBar(node);
    } else {
      GetUINativeModule().tabs.setHideTitleBar(node, this.value);
    }
  }
}

class BarBackgroundColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity: Symbol = Symbol('barbackgroundcolor');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetBarBackgroundColor(node);
    } else {
      GetUINativeModule().tabs.setBarBackgroundColor(node, this.value);
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

class FadingEdgeModifier extends Modifier<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('fadingedge');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().tabs.resetFadingEdge(node);
    } else {
      GetUINativeModule().tabs.setFadingEdge(node, this.value);
    }
  }
}

// @ts-ignore
globalThis.Tabs.attributeModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = GetUINativeModule().getFrameNodeById(elmtId);
  let component = this.createOrGetNode(elmtId, () => {
    return new ArkTabsComponent(nativeNode);
  });
  modifier.applyNormalAttribute(component);
  component.applyModifierPatch();
};
