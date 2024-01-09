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
class ArkRichTextComponent extends ArkComponent implements CommonMethod<RichTextAttribute> {
  constructor(nativePtr: KNode) {
    super(nativePtr);
  }
  onStart(callback: () => void): RichTextAttribute {
    throw new Error('Method not implemented.');
  }
  onComplete(callback: () => void): RichTextAttribute {
    throw new Error('Method not implemented.');
  }
  monopolizeEvents(monopolize: boolean): RichTextAttribute {
    throw new Error('Method not implemented.');
  }
}
// @ts-ignore
globalThis.RichText.attributeModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = getUINativeModule().getFrameNodeById(elmtId);

  let component = this.createOrGetNode(elmtId, () => {
    return new ArkRichTextComponent(nativeNode);
  });
  applyUIAttributes(modifier, nativeNode, component);
  component.applyModifierPatch();
};