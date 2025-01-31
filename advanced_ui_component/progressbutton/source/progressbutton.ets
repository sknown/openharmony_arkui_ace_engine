/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

import { Theme } from '@ohos.arkui.theme';

const EMPTY_STRING: string = '';
const MAX_PROGRESS: number = 100;
const MAX_PERCENTAGE: string = '100%';
const MIN_PERCENTAGE: string = '0%';
const TEXT_OPACITY: number = 0.4;
const BUTTON_NORMARL_WIDTH: number = 44;
const BUTTON_NORMARL_HEIGHT: number = 28;
const BUTTON_BORDER_RADIUS: number = 14;
const TEXT_ENABLE: number = 1.0;


// Set the key value for the basic component of skin changing corresponding to progressButton
const PROGRESS_BUTTON_PROGRESS_KEY = 'progress_button_progress_key';
const PROGRESS_BUTTON_PRIMARY_FONT_KEY = 'progress_button_primary_font_key';
const PROGRESS_BUTTON_CONTAINER_BACKGROUND_COLOR_KEY = 'progress_button_container_background_color_key';
const PROGRESS_BUTTON_EMPHASIZE_SECONDARY_BUTTON_KEY = 'progress_button_emphasize_secondary_button_key';
@Component
export struct ProgressButton {
  @Prop @Watch('getProgressContext') progress: number
  @State textProgress: string = EMPTY_STRING
  @Prop content: string = EMPTY_STRING
  @State @Watch('getLoadingProgress')isLoading: boolean = false
  progressButtonWidth?: Length = BUTTON_NORMARL_WIDTH
  clickCallback: () => void = () => {}
  @Prop enable: boolean = true

  @State progressColor: ResourceColor = '#330A59F7'
  @State containerBorderColor: ResourceColor = '#330A59F7'
  @State containerBackgroundColor: ResourceColor = $r('sys.color.ohos_id_color_foreground_contrary')

  onWillApplyTheme(theme: Theme) {
    this.progressColor = theme.colors.compEmphasizeSecondary;
    this.containerBorderColor = theme.colors.compEmphasizeSecondary;
    this.containerBackgroundColor = theme.colors.iconOnFourth;
  }

  private getButtonProgress(): number {
    if (this.progress < 0) {
      return 0
    } else if (this.progress > MAX_PROGRESS) {
      return MAX_PROGRESS
    }
    return this.progress
  }

  private getProgressContext() {
    if (this.progress < 0) {
      this.isLoading = false
      this.textProgress = MIN_PERCENTAGE
    } else if (this.progress >= MAX_PROGRESS) {
      this.isLoading = false
      this.textProgress = MAX_PERCENTAGE
    } else {
      this.isLoading = true
      this.textProgress = Math.floor(this.progress / MAX_PROGRESS * MAX_PROGRESS).toString() + '%'
    }
  }

  private getLoadingProgress() {
    if (this.isLoading) {
      if(this.progress < 0) {
        this.textProgress = MIN_PERCENTAGE
      } else if (this.progress >= MAX_PROGRESS) {
        this.textProgress = MAX_PERCENTAGE
      } else {
        this.textProgress = Math.floor(this.progress / MAX_PROGRESS * MAX_PROGRESS).toString() + '%'
      }
    }
  }

  build() {
    Button() {
      Stack(){
        Progress({ value: this.getButtonProgress(), total: MAX_PROGRESS,
          style: ProgressStyle.Capsule })
          .height(BUTTON_NORMARL_HEIGHT)
          .borderRadius(BUTTON_BORDER_RADIUS)
          .width('100%')
          .hoverEffect(HoverEffect.None)
          .clip(false)
          .enabled(this.enable)
          .key(PROGRESS_BUTTON_PROGRESS_KEY)
          .color(this.progressColor)
        Row() {
          Text(this.isLoading ? this.textProgress: this.content)
            .fontSize($r('sys.float.ohos_id_text_size_button3'))
            .fontWeight(FontWeight.Medium)
            .key(PROGRESS_BUTTON_PRIMARY_FONT_KEY)
            .maxLines(1)
            .textOverflow({ overflow: TextOverflow.Ellipsis })
            .padding({left: 8, right: 8})
            .opacity(this.enable ? TEXT_ENABLE : TEXT_OPACITY)
        }
        Row()
          .key(PROGRESS_BUTTON_CONTAINER_BACKGROUND_COLOR_KEY)
          .backgroundColor(Color.Transparent)
          .border({ width: 1, color: this.containerBorderColor})
          .height(BUTTON_NORMARL_HEIGHT)
          .borderRadius(BUTTON_BORDER_RADIUS)
          .width('100%')
      }
    }
    .borderRadius(BUTTON_BORDER_RADIUS)
    .clip(false)
    .hoverEffect(HoverEffect.None)
    .key(PROGRESS_BUTTON_EMPHASIZE_SECONDARY_BUTTON_KEY)
    .backgroundColor(this.containerBackgroundColor)
    .constraintSize({minWidth: 44})
    .width((!this.progressButtonWidth || this.progressButtonWidth < BUTTON_NORMARL_WIDTH) ?
      BUTTON_NORMARL_WIDTH : this.progressButtonWidth)
    .stateEffect(this.enable)
    .onClick(() => {
      if (!this.enable) {
        return
      }
      if (this.progress < MAX_PROGRESS) {
        this.isLoading = !this.isLoading
      }
      this.clickCallback && this.clickCallback()
    })
  }
}