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
const TITLE_MODE_RANGE = 2;
const NAV_BAR_POSITION_RANGE = 1;
const NAVIGATION_MODE_RANGE = 2;
const DEFAULT_NAV_BAR_WIDTH = 240;
const MIN_NAV_BAR_WIDTH_DEFAULT = '240vp';
const MAX_NAV_BAR_WIDTH_DEFAULT = '40%';
const NAVIGATION_TITLE_MODE_DEFAULT = 0
const DEFAULT_UNIT = 'vp';

class ArkNavigationComponent extends ArkComponent implements NavigationAttribute {
  constructor(nativePtr: KNode) {
    super(nativePtr);
  }
  navBarWidth(value: Length): NavigationAttribute {
    modifierWithKey(this._modifiersWithKeys, NavBarWidthModifier.identity, NavBarWidthModifier, value);
    return this;
  }
  navBarPosition(value: NavBarPosition): NavigationAttribute {
    modifier(this._modifiers, NavBarPositionModifier, value);
    return this;
  }
  navBarWidthRange(value: [Dimension, Dimension]): NavigationAttribute {
    modifierWithKey(this._modifiersWithKeys, NavBarWidthRangeModifier.identity, NavBarWidthRangeModifier, value);
    return this;
  }
  minContentWidth(value: Dimension): NavigationAttribute {
    modifierWithKey(this._modifiersWithKeys, MinContentWidthModifier.identity, MinContentWidthModifier, value);

    return this;
  }
  mode(value: NavigationMode): NavigationAttribute {
    modifier(this._modifiers, ModeModifier, value);
    return this;
  }
  backButtonIcon(value: any): NavigationAttribute {
    modifierWithKey(this._modifiersWithKeys, BackButtonIconModifier.identity, BackButtonIconModifier, value);
    return this
  }
  hideNavBar(value: boolean): NavigationAttribute {
    modifier(this._modifiers, HideNavBarModifier, isBoolean(value) ? value : false);
    return this;
  }
  title(value: any): NavigationAttribute {
    throw new Error('Method not implemented.');
  }
  subTitle(value: string): NavigationAttribute {
    modifier(this._modifiers, SubTitleModifier, value);
    return this;
  }
  hideTitleBar(value: boolean): NavigationAttribute {
    modifier(this._modifiers, NavigationHideTitleBarModifier, isBoolean(value) ? value : false);
    return this;
  }
  hideBackButton(value: boolean): NavigationAttribute {
    modifier(this._modifiers, HideBackButtonModifier, isBoolean(value) ? value : false);
    return this;
  }
  titleMode(value: NavigationTitleMode): NavigationAttribute {
    modifier(this._modifiers, TitleModeModifier, value);
    return this;
  }
  menus(value: any): NavigationAttribute {
    throw new Error('Method not implemented.');
  }
  toolBar(value: any): NavigationAttribute {
    throw new Error('Method not implemented.');
  }
  toolbarConfiguration(value: any): NavigationAttribute {
    throw new Error('Method not implemented.');
  }
  hideToolBar(value: boolean): NavigationAttribute {
    modifier(this._modifiers, HideToolBarModifier, isBoolean(value) ? value : false);
    return this;
  }
  onTitleModeChange(callback: (titleMode: NavigationTitleMode) => void): NavigationAttribute {
    throw new Error('Method not implemented.');
  }
  onNavBarStateChange(callback: (isVisible: boolean) => void): NavigationAttribute {
    throw new Error('Method not implemented.');
  }
  onNavigationModeChange(callback: (mode: NavigationMode) => void): NavigationAttribute {
    throw new Error('Method not implemented.');
  }
  navDestination(builder: (name: string, param: unknown) => void): NavigationAttribute {
    throw new Error('Method not implemented.');
  }
}

class BackButtonIconModifier extends ModifierWithKey<boolean | object> {
  constructor(value: boolean | object) {
    super(value);
  }
  static identity: Symbol = Symbol('backButtonIcon');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().navigation.resetBackButtonIcon(node);
    }
    else {
      GetUINativeModule().navigation.setBackButtonIcon(node, this.value);
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

class NavBarWidthRangeModifier extends ModifierWithKey<[Dimension, Dimension]> {
  constructor(value: [Dimension, Dimension]) {
    super(value);
  }
  static identity: Symbol = Symbol('navBarWidthRange');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().navigation.resetNavBarWidthRange(node);
    } else {
      GetUINativeModule().navigation.setNavBarWidthRange(node, this.value);
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

class MinContentWidthModifier extends ModifierWithKey<Dimension> {
  constructor(value: Dimension) {
    super(value);
  }
  static identity: Symbol = Symbol('minContentWidth');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().navigation.resetMinContentWidth(node);
    } else {
      GetUINativeModule().navigation.setMinContentWidth(node, this.value);
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

class NavBarWidthModifier extends ModifierWithKey<Length> {
  constructor(value: Length) {
    super(value);
  }
  static identity: Symbol = Symbol('navBarWidth');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().navigation.resetNavBarWidth(node);
    } else {
      GetUINativeModule().navigation.setNavBarWidth(node, this.value);
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

class NavBarPositionModifier extends Modifier<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('navBarPosition');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().navigation.resetNavBarPosition(node);
    } else {
      GetUINativeModule().navigation.setNavBarPosition(node, this.value);
    }
  }
}

class ModeModifier extends Modifier<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('mode');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().navigation.resetMode(node);
    } else {
      GetUINativeModule().navigation.setMode(node, this.value);
    }
  }
}

class HideToolBarModifier extends Modifier<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('hideToolBar');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().navigation.resetHideToolBar(node);
    } else {
      GetUINativeModule().navigation.setHideToolBar(node, this.value);
    }
  }
}

class TitleModeModifier extends Modifier<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('titleMode');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().navigation.resetTitleMode(node);
    } else {
      GetUINativeModule().navigation.setTitleMode(node, this.value);
    }
  }
}

class HideBackButtonModifier extends Modifier<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('hideBackButton');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().navigation.resetHideBackButton(node);
    } else {
      GetUINativeModule().navigation.setHideBackButton(node, this.value);
    }
  }
}

class SubTitleModifier extends Modifier<string> {
  constructor(value: string) {
    super(value);
  }
  static identity: Symbol = Symbol('subTitle');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().navigation.resetSubTitle(node);
    } else {
      GetUINativeModule().navigation.setSubTitle(node, this.value);
    }
  }
}

class NavigationHideTitleBarModifier extends Modifier<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('hideTitleBar');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().navigation.resetHideTitleBar(node);
    } else {
      GetUINativeModule().navigation.setHideTitleBar(node, this.value);
    }
  }
}

class HideNavBarModifier extends Modifier<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('hideNavBar');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      GetUINativeModule().navigation.resetHideNavBar(node);
    } else {
      GetUINativeModule().navigation.setHideNavBar(node, this.value);
    }
  }
}

// @ts-ignore
globalThis.Navigation.attributeModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = GetUINativeModule().getFrameNodeById(elmtId);
  let component = this.createOrGetNode(elmtId, () => {
    return new ArkNavigationComponent(nativeNode);
  });
  applyUIAttributes(modifier, nativeNode, component);
  component.applyModifierPatch();
}
