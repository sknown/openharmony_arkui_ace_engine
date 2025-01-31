/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

import { BusinessError } from '@ohos.base';
import hilog from '@ohos.hilog';
import { KeyCode } from '@ohos.multimodalInput.keyCode';
import resourceManager from '@ohos.resourceManager';
import { Theme } from '@ohos.arkui.theme';
import { LengthMetrics } from '@ohos.arkui.node';
import common from '@ohos.app.ability.common';
import window from '@ohos.window';

export enum EditableLeftIconType {
  Back,
  Cancel,
}

export declare interface EditableTitleBarMenuItem {
  value: ResourceStr;
  isEnabled: boolean;
  label?: ResourceStr;
  action?: () => void;
}

export type EditableTitleBarItem = EditableTitleBarMenuItem;

export declare interface EditableTitleBarOptions {
  backgroundColor?: ResourceColor;
  backgroundBlurStyle?: BlurStyle;
  safeAreaTypes?: Array<SafeAreaType>;
  safeAreaEdges?: Array<SafeAreaEdge>;
}

enum ItemType {
  Image,
  Icon,
  LeftIcon,
}

const PUBLIC_CANCEL = $r('sys.media.ohos_ic_public_cancel');

const PUBLIC_OK = $r('sys.media.ohos_ic_public_ok');

const PUBLIC_BACK = $r('sys.media.ohos_ic_compnent_titlebar_back');

const DEFAULT_TITLE_BAR_HEIGHT = 56;
const DEFAULT_TITLE_PADDING = 2;
const MAX_LINE_ONE = 1;
const MAX_LINES_TWO = 2;
const MAX_MAIN_TITLE_PERCENT = 0.65;
const MAX_SUB_TITLE_PERCENT = 0.35;
const MIN_SUBTITLE_SIZE = '10.0vp';
const TEXT_EDITABLE_DIALOG = '18.3fp';
const IMAGE_SIZE = '64vp';
const MAX_DIALOG = '256vp';
const MIN_DIALOG = '216vp';
const TITLE_VP: number = 20;
const SUBTITLE_VP: number = 14;
const TITLE_F: number = getNumberByResource('titlebar_title_tertiary_size');
const SUBTITLE_F: number = getNumberByResource('titlebar_subheader_size');
const TITLE_F_VP: string = (TITLE_F > 0 ? TITLE_F : TITLE_VP) + 'vp';
const SUBTITLE_F_VP: string = (SUBTITLE_F > 0 ? SUBTITLE_F : SUBTITLE_VP) + 'vp';

class EditableTitleBarTheme {
  public iconColor: ResourceColor = $r('sys.color.titlebar_icon_color');
  public iconBackgroundColor: ResourceColor = $r('sys.color.titlebar_icon_background_color');
  public iconBackgroundPressedColor: ResourceColor = $r('sys.color.titlebar_icon_background_pressed_color');
  public iconBackgroundHoverColor: ResourceColor = $r('sys.color.titlebar_icon_background_hover_color');
  public iconBackgroundFocusOutlineColor: ResourceColor = $r('sys.color.titlebar_icon_background_focus_outline_color');
  public titleColor: ResourceColor = $r('sys.color.titlebar_title_tertiary_color');
  public subTitleColor: ResourceColor = $r('sys.color.titlebar_subheader_color');
}

@Component
export struct EditableTitleBar {
  leftIconStyle: EditableLeftIconType = EditableLeftIconType.Back;
  title: ResourceStr = '';
  subtitle?: ResourceStr = '';
  isSaveIconRequired: boolean = true;
  imageItem?: EditableTitleBarItem;
  menuItems: Array<EditableTitleBarMenuItem> | undefined = undefined;
  options: EditableTitleBarOptions = {
    safeAreaTypes: [SafeAreaType.SYSTEM],
    safeAreaEdges: [SafeAreaEdge.TOP],
  };
  onSave?: () => void;
  onCancel?: () => void;
  constraintWidth: number = 0;
  public static readonly maxCountOfExtraItems = 3;
  public static readonly maxOtherCountOfExtraItems = 2;
  public static readonly commonZero = 0;
  public static readonly noneColor = '#00000000';
  public static readonly defaultHeight: number = getNumberByResource('titlebar_default_height');
  public static readonly defaultMarginLevel1: number = getNumberByResource('margin_level1');
  public static readonly defaultMarginLevel2: number = getNumberByResource('margin_level2');
  public static readonly defaultMarginLevel3: number = getNumberByResource('margin_level3');
  public static readonly defaultTitlePadding: number = getNumberByResource('titlebar_icon_background_space_horizontal');
  public static readonly totalHeight: number =
    EditableTitleBar.defaultHeight === EditableTitleBar.commonZero ? DEFAULT_TITLE_BAR_HEIGHT :
    EditableTitleBar.defaultHeight;
  static readonly titlePadding: number =
    EditableTitleBar.defaultTitlePadding === EditableTitleBar.commonZero ?
      DEFAULT_TITLE_PADDING : EditableTitleBar.defaultTitlePadding;
  private static readonly maxMainTitleHeight =
    (EditableTitleBar.totalHeight - EditableTitleBar.titlePadding) * MAX_MAIN_TITLE_PERCENT;
  private static readonly maxSubTitleHeight =
    (EditableTitleBar.totalHeight - EditableTitleBar.titlePadding) * MAX_SUB_TITLE_PERCENT;
  @Provide editableTitleBarTheme: EditableTitleBarTheme = new EditableTitleBarTheme();
  @Prop contentMargin?: LocalizedMargin;
  @State titleBarMargin: LocalizedMargin = {
    start: LengthMetrics.resource($r('sys.float.margin_left')),
    end: LengthMetrics.resource($r('sys.float.margin_right')),
  }
  @State backActive: boolean = false;
  @State fontSize: number = 1;

  onWillApplyTheme(theme: Theme): void {
    this.editableTitleBarTheme.iconColor = theme.colors.iconPrimary;
    this.editableTitleBarTheme.titleColor = theme.colors.fontPrimary;
    this.editableTitleBarTheme.subTitleColor = theme.colors.fontSecondary;
    this.editableTitleBarTheme.iconBackgroundColor = theme.colors.compBackgroundTertiary;
    this.editableTitleBarTheme.iconBackgroundPressedColor = theme.colors.interactivePressed;
    this.editableTitleBarTheme.iconBackgroundHoverColor = theme.colors.interactiveHover;
    this.editableTitleBarTheme.iconBackgroundFocusOutlineColor = theme.colors.interactiveFocus;
  }

  build() {
    Flex({
      justifyContent: FlexAlign.SpaceBetween,
      alignItems: ItemAlign.Stretch,
    }) {
      Row() {
        Row() {
          this.leftIconLayout();
        }
        .flexShrink(0)

        if (this.imageItem) {
          Row() {
            this.imageItemLayout();
          }
          .flexShrink(0)
        }

        Row() {
          this.titleLayout();
        }
        .width('100%')
        .flexShrink(1)

        Row() {
          this.rightMenuItemsLayout();
        }
        .flexShrink(0)
      }
      .width('100%')
      .margin(this.contentMargin ?? this.titleBarMargin)
      .height(EditableTitleBar.totalHeight)
    }
    .backgroundColor(this.options.backgroundColor ?? EditableTitleBar.noneColor)
    .backgroundBlurStyle(
      this.options.backgroundBlurStyle ?? BlurStyle.NONE)
    .expandSafeArea(
      this.options.safeAreaTypes,
      this.options.safeAreaEdges,
    )
  }

  @Builder
  imageItemLayout(): void {
    ImageMenuItem({
      item: this.imageItem,
      attribute: ItemType.Image,
    })
  }

  @Builder
  leftIconLayout(): void {
    if (this.leftIconStyle === EditableLeftIconType.Back) {
      Navigator()
        .active(this.backActive)

      ImageMenuItem({
        item: {
          value: PUBLIC_BACK,
          isEnabled: true,
          action: () => this.onCancel ? this.onCancel() : this.backActive = true,
        },
        fontSize: this.fontSize,
        attribute: ItemType.LeftIcon,
      })
    } else {
      ImageMenuItem({
        item: {
          value: PUBLIC_CANCEL,
          isEnabled: true,
          action: () => this.onCancel && this.onCancel(),
        },
        fontSize: this.fontSize,
        attribute: ItemType.LeftIcon,
      })
    }
  }

  @Builder
  titleLayout(): void {
    Column() {
      Row() {
        Text(this.title)
          .maxFontSize(TITLE_F_VP)
          .minFontSize(SUBTITLE_F_VP)
          .fontColor(this.editableTitleBarTheme.titleColor)
          .maxLines(this.subtitle ? MAX_LINE_ONE : MAX_LINES_TWO)
          .fontWeight(FontWeight.Bold)
          .textAlign(TextAlign.Start)
          .textOverflow({ overflow: TextOverflow.Ellipsis })
          .heightAdaptivePolicy(this.subtitle ?
          TextHeightAdaptivePolicy.MAX_LINES_FIRST : TextHeightAdaptivePolicy.MIN_FONT_SIZE_FIRST)
          .constraintSize({
            maxHeight: this.subtitle ? EditableTitleBar.maxMainTitleHeight : EditableTitleBar.totalHeight,
          })
      }
      .justifyContent(FlexAlign.Start)

      if (this.subtitle) {
        Row() {
          Text(this.subtitle)
            .maxFontSize(SUBTITLE_F_VP)
            .minFontSize(MIN_SUBTITLE_SIZE)
            .fontColor(this.editableTitleBarTheme.subTitleColor)
            .maxLines(MAX_LINE_ONE)
            .fontWeight(FontWeight.Regular)
            .textAlign(TextAlign.Start)
            .textOverflow({ overflow: TextOverflow.Ellipsis })
            .heightAdaptivePolicy(TextHeightAdaptivePolicy.MAX_LINES_FIRST)
            .constraintSize({
              maxHeight: this.title ? EditableTitleBar.maxSubTitleHeight : EditableTitleBar.totalHeight,
            })
        }
        .margin({
          top: $r('sys.float.padding_level1'),
        })
        .justifyContent(FlexAlign.Start)
      }
    }
    .height(EditableTitleBar.totalHeight)
    .justifyContent(FlexAlign.Center)
    .margin({
      start: LengthMetrics.vp(EditableTitleBar.titlePadding),
    })
    .alignItems(HorizontalAlign.Start)
  }

  @Builder
  rightMenuItemsLayout(): void {
    EditableTitleBarMenuSection({
      menuItems: this.menuItems,
      onSave: this.onSave,
      isSaveEnabled: this.isSaveIconRequired,
      fontSize: this.fontSize,
    })
  }

  onPlaceChildren(selfLayoutInfo: GeometryInfo, children: Layoutable[], constraint: ConstraintSizeOptions): void {
    children.forEach((child) => {
      child.layout({ x: 0, y: 0 });
    })
  }

  onMeasureSize(selfLayoutInfo: GeometryInfo, children: Measurable[], constraint: ConstraintSizeOptions): SizeResult {
    let result: SizeResult = { width: selfLayoutInfo.width, height: selfLayoutInfo.height };
    children.forEach((child) => {
      result.height = child.measure(constraint).height;
      result.width = Number(constraint.maxWidth);
    })
    return result;
  }
}

@Component
struct EditableTitleBarMenuSection {
  menuItems: Array<EditableTitleBarMenuItem> | undefined = undefined;
  onSave?: () => void;
  isSaveEnabled: boolean = true;
  @Prop fontSize: number = 1;

  build() {
    Column() {
      Row() {
        if (this.menuItems !== undefined && this.menuItems.length > EditableTitleBar.commonZero) {
          ForEach(this.menuItems.slice(EditableTitleBar.commonZero,
            this.isSaveEnabled ?
            EditableTitleBar.maxOtherCountOfExtraItems : EditableTitleBar.maxCountOfExtraItems),
            (item: EditableTitleBarMenuItem) => {
              ImageMenuItem({
                item: item,
                attribute: ItemType.Icon,
                fontSize: this.fontSize,
              })
            })
        }
        if (this.isSaveEnabled) {
          ImageMenuItem({
            item: {
              value: PUBLIC_OK,
              isEnabled: true,
              action: () => this.onSave && this.onSave(),
            },
            fontSize: this.fontSize,
            attribute: ItemType.Icon,
          })
        }
      }
    }
    .justifyContent(FlexAlign.Center)
  }
}

@Component
struct ImageMenuItem {
  item: EditableTitleBarMenuItem = {
    value: '',
    isEnabled: true,
    label: '',
  };
  attribute: ItemType = ItemType.Image;
  callbackId: number | undefined = undefined;
  minFontSize: number = 1.75;
  maxFontSize: number = 3.2;
  longPressTime: number = 500;
  @Prop fontSize: number = 1;
  @State isOnFocus: boolean = false;
  @State isOnHover: boolean = false;
  @State isOnClick: boolean = false;
  @Consume editableTitleBarTheme: EditableTitleBarTheme;
  dialogController: CustomDialogController | null = new CustomDialogController({
    builder: EditableTitleBarDialog({
      cancel: () => {
      },
      confirm: () => {
      },
      itemEditableDialog: this.item,
      textEditableTitleBarDialog: this.item.label ? this.item.label : this.textDialog(),
      fontSize: this.fontSize,
    }),
    maskColor: Color.Transparent,
    isModal: true,
    customStyle: true,
  })

  private textDialog(): ResourceStr {
    if (this.item.value === PUBLIC_OK) {
      return $r('sys.string.icon_save');
    } else if (this.item.value === PUBLIC_CANCEL) {
      return $r('sys.string.icon_cancel');
    } else if (this.item.value === PUBLIC_BACK) {
      return $r('sys.string.icon_back');
    } else {
      return this.item.label ? this.item.label : '';
    }
  }

  @Styles
  buttonStateStyles() {
    .stateStyles({
      focused: this.focusedStyle,
      normal: this.notInFocusedStyle,
      pressed: this.notInFocusedStyle,
    })
  }

  @Styles
  focusedStyle() {
    .border({
      radius: $r('sys.float.titlebar_icon_background_shape'),
      width: $r('sys.float.titlebar_icon_background_focus_outline_weight'),
      color: this.editableTitleBarTheme.iconBackgroundFocusOutlineColor,
      style: BorderStyle.Solid,
    })
  }

  @Styles
  notInFocusedStyle() {
    .border({
      radius: $r('sys.float.titlebar_icon_background_shape'),
      width: EditableTitleBar.commonZero,
    })
  }

  private touchEventAction(event: TouchEvent): void {
    if (!this.item.isEnabled) {
      return;
    }
    if (event.type === TouchType.Down) {
      this.isOnClick = true;
    }
    if (event.type === TouchType.Up) {
      if (this.fontSize >= this.minFontSize) {
        this.dialogController?.close()
      }
      this.isOnClick = false;
    }
  }

  private keyEventAction(event: KeyEvent): void {
    if (!this.item.isEnabled) {
      return;
    }
    if (event.keyCode !== KeyCode.KEYCODE_ENTER && event.keyCode !== KeyCode.KEYCODE_SPACE) {
      return;
    }
    if (event.type === KeyType.Down) {
      this.isOnClick = true;
    }
    if (event.type === KeyType.Up) {
      this.isOnClick = false;
    }
  }

  @Styles
  buttonEventStyle() {
    .onFocus(() => {
      if (!this.item.isEnabled) {
        return;
      }
      this.isOnFocus = true;
    })
    .onBlur(() => this.isOnFocus = false)
    .onHover((isOn) => {
      if (!this.item.isEnabled) {
        return;
      }
      this.isOnHover = isOn;
    })
    .onKeyEvent((event) => {
      this.keyEventAction(event);
    })
    .onTouch((event) => {
      this.touchEventAction(event);
    })
    .gesture(GestureGroup(GestureMode.Exclusive,
      LongPressGesture({ repeat: false, duration: this.longPressTime })
        .onAction((event: GestureEvent) => {
          let context = this.getUIContext().getHostContext() as common.UIAbilityContext;
          this.fontSize = context.config?.fontSizeScale ?? 1;
          if (event) {
            if (this.fontSize >= this.minFontSize) {
              this.dialogController?.open()
            }
          }
        }),
      TapGesture()
        .onAction((event: GestureEvent) => {
          this.item.isEnabled && this.item.action && this.item.action()
        })
    ))
  }

  @Styles
  backgroundButtonStyle() {
    .width($r('sys.float.titlebar_icon_background_width'))
    .height($r('sys.float.titlebar_icon_background_height'))
    .focusable(this.item.isEnabled)
    .enabled(this.item.isEnabled)
  }

  getBgColor(): ResourceColor {
    if (this.isOnClick) {
      return this.editableTitleBarTheme.iconBackgroundPressedColor;
    } else if (this.isOnHover) {
      return this.editableTitleBarTheme.iconBackgroundHoverColor;
    } else {
      return this.editableTitleBarTheme.iconBackgroundColor;
    }
  }

  getFgColor(): ResourceStr {
    if (this.isOnClick) {
      return $r('sys.color.titlebar_icon_background_pressed_color');
    } else if (this.isOnHover) {
      return $r('sys.color.titlebar_icon_background_hover_color');
    } else {
      return EditableTitleBar.noneColor;
    }
  }

  @Builder
  IconBuilder(): void {
    Button({ type: ButtonType.Normal, stateEffect: this.item.isEnabled }) {
      Image(this.item.value)
        .width($r('sys.float.titlebar_icon_width'))
        .height($r('sys.float.titlebar_icon_height'))
        .fillColor(this.editableTitleBarTheme.iconColor)
        .focusable(this.item.isEnabled)
        .enabled(this.item.isEnabled)
        .matchTextDirection(this.item.value === PUBLIC_BACK ? true : false)
        .draggable(false)
    }
    .backgroundButtonStyle()
    .borderRadius($r('sys.float.titlebar_icon_background_shape'))
    .margin({
      start: this.attribute === ItemType.LeftIcon ? LengthMetrics.vp(EditableTitleBar.commonZero) :
      LengthMetrics.resource($r('sys.float.titlebar_icon_background_space_horizontal')),
    })
    .foregroundColor(this.getFgColor())
    .backgroundColor(this.getBgColor())
    .buttonStateStyles()
    .buttonEventStyle()
  }

  @Builder
  ImageBuilder() {
    Stack({ alignContent: Alignment.Center }) {
      Image(this.item.value)
        .width($r('sys.float.titlebar_icon_background_width'))
        .height($r('sys.float.titlebar_icon_background_height'))
        .borderRadius($r('sys.float.corner_radius_level10'))
        .focusable(false)
        .enabled(this.item.isEnabled)
        .objectFit(ImageFit.Cover)

      Button({ type: ButtonType.Circle })
        .backgroundButtonStyle()
        .foregroundColor(this.getFgColor())
        .backgroundColor(EditableTitleBar.noneColor)
        .buttonStateStyles()
        .buttonEventStyle()
    }
    .margin({
      start: LengthMetrics.resource($r('sys.float.titlebar_icon_background_space_horizontal')),
    })
  }

  build() {
    if (this.attribute === ItemType.Icon || this.attribute === ItemType.LeftIcon) {
      this.IconBuilder();
    } else {
      this.ImageBuilder();
    }
  }
}

/**
 * EditableTitleBarDialog
 *
 * @since 2024-05-28
 */
@CustomDialog
struct EditableTitleBarDialog {
  itemEditableDialog: EditableTitleBarMenuItem = {
    value: '',
    isEnabled: true,
  };
  callbackId: number | undefined = undefined;
  textEditableTitleBarDialog?: ResourceStr = '';
  mainWindowStage: window.Window | undefined = undefined;
  controller?: CustomDialogController
  minFontSize: number = 1.75;
  maxFontSize: number = 3.2;
  screenWidth: number = 640;
  verticalScreenLines: number = 6;
  horizontalsScreenLines: number = 1;
  cancel: () => void = () => {
  }
  confirm: () => void = () => {
  }
  @StorageLink('mainWindow') mainWindow: Promise<window.Window> | undefined = undefined;
  @Prop fontSize: number = 1;
  @State maxLines: number = 1;
  @StorageProp('windowStandardHeight') windowStandardHeight: number = 0;

  build() {
    if (this.textEditableTitleBarDialog) {
      Column() {
        Image(this.itemEditableDialog.value)
          .width(IMAGE_SIZE)
          .height(IMAGE_SIZE)
          .margin({
            top: $r('sys.float.padding_level24'),
            bottom: $r('sys.float.padding_level8'),
          })
          .fillColor($r('sys.color.icon_primary'))
        Column() {
          Text(this.textEditableTitleBarDialog)
            .fontSize(TEXT_EDITABLE_DIALOG)
            .textOverflow({ overflow: TextOverflow.Ellipsis })
            .maxLines(this.maxLines)
            .width('100%')
            .textAlign(TextAlign.Center)
            .fontColor($r('sys.color.font_primary'))
        }
        .width('100%')
        .padding({
          left: $r('sys.float.padding_level4'),
          right: $r('sys.float.padding_level4'),
          bottom: $r('sys.float.padding_level12'),
        })
      }
      .width(this.fontSize === this.maxFontSize ? MAX_DIALOG : MIN_DIALOG)
      .constraintSize({ minHeight: this.fontSize === this.maxFontSize ? MAX_DIALOG : MIN_DIALOG })
      .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
      .shadow(ShadowStyle.OUTER_DEFAULT_LG)
      .borderRadius(($r('sys.float.corner_radius_level10')))
    } else {
      Column() {
        Image(this.itemEditableDialog.value)
          .width(IMAGE_SIZE)
          .height(IMAGE_SIZE)
          .fillColor($r('sys.color.icon_primary'))
      }
      .width(this.fontSize === this.maxFontSize ? MAX_DIALOG : MIN_DIALOG)
      .constraintSize({ minHeight: this.fontSize === this.maxFontSize ? MAX_DIALOG : MIN_DIALOG })
      .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
      .shadow(ShadowStyle.OUTER_DEFAULT_LG)
      .borderRadius(($r('sys.float.corner_radius_level10')))
      .justifyContent(FlexAlign.Center)
    }
  }

  async aboutToAppear(): Promise<void> {
    let context = this.getUIContext().getHostContext() as common.UIAbilityContext;
    this.mainWindowStage = context.windowStage.getMainWindowSync();
    let properties: window.WindowProperties = this.mainWindowStage.getWindowProperties();
    let rect = properties.windowRect;
    if (px2vp(rect.height) > this.screenWidth) {
      this.maxLines = this.verticalScreenLines;
    } else {
      this.maxLines = this.horizontalsScreenLines;
    }
  }
}

/**
 * get resource size
 *
 * @param resourceName resource name
 * @return resource size
 */
function getNumberByResource(resourceName: string): number {
  try {
    return resourceManager.getSystemResourceManager().getNumberByName(resourceName);
  } catch (error) {
    let code: number = (error as BusinessError).code;
    let message: string = (error as BusinessError).message;
    hilog.error(0x3900, 'Ace', `EditableTitleBar getNumberByResource error, code: ${code},message:${message}`);
    return EditableTitleBar.commonZero;
  }
}