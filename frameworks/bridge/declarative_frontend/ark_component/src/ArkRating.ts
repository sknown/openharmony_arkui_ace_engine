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

class RatingStarsModifier extends ModifierWithKey<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('ratingStars');
  applyPeer(node: KNode, reset: boolean) {
    if (reset) {
      getUINativeModule().rating.resetStars(node);
    } else {
      getUINativeModule().rating.setStars(node, this.value);
    }
  }
}

class RatingStepSizeModifier extends ModifierWithKey<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('ratingStepSize');
  applyPeer(node: KNode, reset: boolean) {
    if (reset) {
      getUINativeModule().rating.resetStepSize(node);
    } else {
      getUINativeModule().rating.setStepSize(node, this.value);
    }
  }
}

class RatingStarStyleModifier extends ModifierWithKey<ArkStarStyle> {
  constructor(value: ArkStarStyle) {
    super(value);
  }
  static identity: Symbol = Symbol('ratingStarStyle');
  applyPeer(node: KNode, reset: boolean) {
    if (reset) {
      getUINativeModule().rating.resetStarStyle(node);
    } else {
      getUINativeModule().rating.setStarStyle(node,
        this.value?.backgroundUri, this.value?.foregroundUri, this.value?.secondaryUri);
    }
  }

  checkObjectDiff(): boolean {
    return this.stageValue?.backgroundUri !== this.value?.backgroundUri ||
      this.stageValue?.foregroundUri !== this.value?.foregroundUri || this.stageValue?.secondaryUri !== this.value?.secondaryUri;
  }
}

class ArkRatingComponent extends ArkComponent implements RatingAttribute {
  builder: WrappedBuilder<Object[]> | null = null;
  ratingNode: BuilderNode<[RatingConfiguration]> | null = null;
  modifier: ContentModifier<RatingConfiguration>;
  constructor(nativePtr: KNode, classType?: ModifierType) {
    super(nativePtr, classType);
  }
  stars(value: number): this {
    modifierWithKey(this._modifiersWithKeys, RatingStarsModifier.identity, RatingStarsModifier, value);
    return this;
  }
  stepSize(value: number): this {
    modifierWithKey(this._modifiersWithKeys, RatingStepSizeModifier.identity, RatingStepSizeModifier, value);
    return this;
  }
  starStyle(value: { backgroundUri: string; foregroundUri: string; secondaryUri?: string | undefined; }): this {
    let starStyle = new ArkStarStyle();
    if (!isUndefined(value)) {
      starStyle.backgroundUri = value.backgroundUri;
      starStyle.foregroundUri = value.foregroundUri;
      starStyle.secondaryUri = value.secondaryUri;

      modifierWithKey(this._modifiersWithKeys, RatingStarStyleModifier.identity, RatingStarStyleModifier, value);
    } else {
      modifierWithKey(this._modifiersWithKeys, RatingStarStyleModifier.identity, RatingStarStyleModifier, undefined);
    }
    return this;
  }
  onChange(callback: (value: number) => void): this {
    throw new Error('Method not implemented.');
  }
  setContentModifier(modifier: ContentModifier<RatingConfiguration>): this {
    this.builder = modifier.applyContent();
    this.modifier = modifier;
    getUINativeModule().rating.setContentModifierBuilder(this.nativePtr, this);
  }
  makeContentModifierNode(context: UIContext, ratingConfiguration: RatingConfiguration): FrameNode | null {
    ratingConfiguration.contentModifier = this.modifier;
    if (isUndefined(this.ratingNode)) {
      const xNode = globalThis.requireNapi('arkui.node');
      this.ratingNode = new xNode.BuilderNode(context);
      this.ratingNode.build(this.builder, ratingConfiguration);
    } else {
      this.ratingNode.update(ratingConfiguration);
    }
    return this.ratingNode.getFrameNode();
  }
}
// @ts-ignore
globalThis.Rating.attributeModifier = function (modifier: ArkComponent): void {
  attributeModifierFunc.call(this, modifier, (nativePtr: KNode) => {
    return new ArkRatingComponent(nativePtr);
  }, (nativePtr: KNode, classType: ModifierType, modifierJS: ModifierJS) => {
    return new modifierJS.RatingModifier(nativePtr, classType);
  });
};

// @ts-ignore
globalThis.Rating.contentModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = getUINativeModule().getFrameNodeById(elmtId);
  let component = this.createOrGetNode(elmtId, () => {
    return new ArkRatingComponent(nativeNode);
  });
  component.setContentModifier(modifier);
};
