/// <reference path="./import.ts" />

class ArkBorderStyle implements Equable {
  type: boolean | undefined;
  style: BorderStyle | undefined;
  top: BorderStyle | undefined;
  right: BorderStyle | undefined;
  bottom: BorderStyle | undefined;
  left: BorderStyle | undefined;

  constructor() {
    this.type = undefined;
    this.style = undefined;
    this.top = undefined;
    this.right = undefined;
    this.bottom = undefined;
    this.left = undefined;
  }
  isEqual(another: ArkBorderStyle): boolean {
    return (
      this.type === another.type &&
      this.style === another.style &&
      this.top === another.top &&
      this.right === another.right &&
      this.bottom === another.bottom &&
      this.left === another.left
    );
  }
  parseBorderStyle(value: BorderStyle | EdgeStyles): boolean {
    if (typeof value === 'number') {
      this.style = value;
      this.type = true;
      return true;
    } else if (typeof value === 'object') {
      return this.parseEdgeStyles(value as EdgeStyles);
    }
    return false;
  }
  parseEdgeStyles(options: EdgeStyles): boolean {
    this.top = options.top;
    this.right = options.right;
    this.bottom = options.bottom;
    this.left = options.left;
    this.type = true;
    return true;
  }
}

class ArkShadow {
  style: number | undefined;
  radius: number | Resource | undefined;
  type: ShadowType | undefined;
  color: number | undefined;
  offsetX: number | Resource | undefined;
  offsetY: number | Resource | undefined;
  fill: boolean | undefined;

  constructor() {
    this.style = undefined;
    this.radius = undefined;
    this.type = undefined;
    this.color = undefined;
    this.offsetX = undefined;
    this.offsetY = undefined;
    this.fill = undefined;
  }

  isEqual(another: ArkShadow): boolean {
    return (
      this.style === another.style &&
      this.radius === another.radius &&
      this.type === another.type &&
      this.color === another.color &&
      this.offsetX === another.offsetX &&
      this.offsetY === another.offsetY &&
      this.fill === another.fill
    );
  }

  parseShadowValue(value: ShadowOptions | ShadowStyle): boolean {
    if (typeof value === 'number') {
      this.style = value;
      return true;
    } else if (typeof value === 'object') {
      return this.parseShadowOptions(value as ShadowOptions);
    }
    return false;
  }

  parseShadowOptions(options: ShadowOptions): boolean {
    if (
      isResource(options.radius) ||
      isResource(options.color) ||
      isResource(options.offsetX) ||
      isResource(options.offsetY)
    ) {
      return false;
    }
    let arkColor = new ArkColor();
    this.radius = options.radius;
    this.type = options.type;
    if (arkColor.parseColorValue(options.color)) {
      this.color = arkColor.getColor();
    }
    this.offsetX = options.offsetX;
    this.offsetY = options.offsetY;
    this.fill = options.fill;
    return true;
  }
}

class ArkBorderColor {
  leftColor: number | undefined | Resource;
  rightColor: number | undefined | Resource;
  topColor: number | undefined | Resource;
  bottomColor: number | undefined | Resource;

  constructor() {
    this.leftColor = undefined;
    this.rightColor = undefined;
    this.topColor = undefined;
    this.bottomColor = undefined;
  }

  isEqual(another: ArkBorderColor): boolean {
    return (
      this.leftColor === another.leftColor &&
      this.rightColor === another.rightColor &&
      this.topColor === another.topColor &&
      this.bottomColor === another.bottomColor
    );
  }
}

class ArkPosition implements Equable {
  x: number | string | undefined | Resource;
  y: number | string | undefined | Resource;

  constructor() {
    this.x = undefined;
    this.y = undefined;
  }

  isEqual(another: ArkPosition): boolean {
    return this.x === another.x && this.y === another.y;
  }
}

class ArkBorderWidth implements Equable {
  left: number | string | undefined | Resource;
  right: number | string | undefined | Resource;
  top: number | string | undefined | Resource;
  bottom: number | string | undefined | Resource;

  constructor() {
    this.left = undefined;
    this.right = undefined;
    this.top = undefined;
    this.bottom = undefined;
  }

  isEqual(another: ArkBorderWidth): boolean {
    return (
      this.left === another.left &&
      this.right === another.right &&
      this.top === another.top &&
      this.bottom === another.bottom
    );
  }
}

class ArkBorderRadius implements Equable {
  topLeft: number | string | undefined | Resource;
  topRight: number | string | undefined | Resource;
  bottomLeft: number | string | undefined | Resource;
  bottomRight: number | string | undefined | Resource;

  constructor() {
    this.topLeft = undefined;
    this.topRight = undefined;
    this.bottomLeft = undefined;
    this.bottomRight = undefined;
  }

  isEqual(another: ArkBorderRadius): boolean {
    return (
      this.topLeft === another.topLeft &&
      this.topRight === another.topRight &&
      this.bottomLeft === another.bottomLeft &&
      this.bottomRight === another.bottomRight
    );
  }
}

class ArkTransformMatrix implements Equable {
  matrix: [];
  length: number;

  constructor(matrix: []) {
    this.matrix = matrix;
    this.length = 16;
  }

  compareArrays(arr1: [], arr2: []): boolean {
    return (
      Array.isArray(arr1) &&
      Array.isArray(arr2) &&
      arr1.length === arr2.length &&
      arr1.every((value, index) => value === arr2[index])
    );
  }

  isEqual(another: ArkTransformMatrix): boolean {
    return this.compareArrays(this.matrix, another.matrix);
  }
}

class ArkLabelStyle {
  overflow: number | undefined;
  maxLines: number | undefined;
  minFontSize: number | string | undefined | Resource;
  maxFontSize: number | string | undefined | Resource;
  heightAdaptivePolicy: number | undefined;
  font: ArkLabelFont;

  constructor() {
    this.overflow = undefined;
    this.maxLines = undefined;
    this.minFontSize = undefined;
    this.maxFontSize = undefined;
    this.heightAdaptivePolicy = undefined;
    this.font = new ArkLabelFont();
  }

  isEqual(another: ArkLabelStyle): boolean {
    return (
      this.overflow === another.overflow &&
      this.maxLines === another.maxLines &&
      this.minFontSize === another.minFontSize &&
      this.maxFontSize === another.maxFontSize &&
      this.heightAdaptivePolicy === another.heightAdaptivePolicy &&
      this.font.isEqual(another.font)
    );
  }
}

class ArkLabelFont {
  size: number | string | undefined | Resource;
  weight: FontWeight | number | string | undefined;
  family: string | undefined | Resource;
  style: FontStyle | number | undefined;
  constructor() {
    this.size = undefined;
    this.weight = undefined;
    this.family = undefined;
    this.style = undefined;
  }

  isEqual(another: ArkLabelFont): boolean {
    return (
      this.size === another.size &&
      this.weight === another.weight &&
      this.family === another.family &&
      this.style === another.style
    );
  }
}

function ArkDeepCompareArrays(arr1: Array<any>, arr2: Array<any>): boolean {
  return (
    Array.isArray(arr1) &&
    Array.isArray(arr2) &&
    arr1.length === arr2.length &&
    arr1.every((value, index) => {
      if (Array.isArray(value) && Array.isArray(arr2[index])) {
        return ArkDeepCompareArrays(value, arr2[index]);
      } else {
        return value === arr2[index];
      }
    })
  );
}

class ArkLinearGradient {
  angle: number | string | undefined;
  direction: number | undefined;
  colors: Array<any>;
  repeating: boolean | undefined;

  constructor(
    angle: number | string | undefined,
    direction: number | undefined,
    colors: Array<any>,
    repeating: boolean | undefined
  ) {
    this.angle = angle;
    this.direction = direction;
    this.colors = colors;
    this.repeating = repeating;
  }

  isEqual(another: ArkLinearGradient): boolean {
    return (
      this.angle === another.angle &&
      this.direction === another.direction &&
      ArkDeepCompareArrays(this.colors, another.colors) &&
      this.repeating === another.repeating
    );
  }
}

class ArkRadialGradient {
  center: Array<any>;
  radius: number | string;
  colors: Array<any>;
  repeating: boolean | undefined;

  constructor(center: Array<any>, radius: number | string, colors: Array<any>, repeating: boolean | undefined) {
    this.center = center;
    this.radius = radius;
    this.colors = colors;
    this.repeating = repeating;
  }

  isEqual(another: ArkRadialGradient): boolean {
    return (
      ArkDeepCompareArrays(this.center, another.center) &&
      this.radius === another.radius &&
      ArkDeepCompareArrays(this.colors, another.colors) &&
      this.repeating === another.repeating
    );
  }
}

class ArkSweepGradient {
  center: Array<any>;
  start: number | string | undefined;
  end: number | string | undefined;
  rotation: number | string | undefined;
  colors: Array<any>;
  repeating: boolean | undefined;

  constructor(
    center: Array<any>,
    start: number | string | undefined,
    end: number | string | undefined,
    rotation: number | string | undefined,
    colors: Array<any>,
    repeating: boolean | undefined
  ) {
    this.center = center;
    this.start = start;
    this.end = end;
    this.rotation = rotation;
    this.colors = colors;
    this.repeating = repeating;
  }

  isEqual(another: ArkSweepGradient): boolean {
    return (
      ArkDeepCompareArrays(this.center, another.center) &&
      this.start === another.start &&
      this.end === another.end &&
      this.rotation === another.rotation &&
      ArkDeepCompareArrays(this.colors, another.colors) &&
      this.repeating === another.repeating
    );
  }
}

class ArkForegroundBlurStyle {
  blurStyle: number | undefined;
  colorMode: number | undefined;
  adaptiveColor: number | undefined;
  scale: number | undefined;

  constructor() {
    this.blurStyle = undefined;
    this.colorMode = undefined;
    this.adaptiveColor = undefined;
    this.scale = undefined;
  }

  isEqual(another: ArkForegroundBlurStyle): boolean {
    return (
      this.blurStyle === another.blurStyle &&
      this.colorMode === another.colorMode &&
      this.adaptiveColor === another.adaptiveColor &&
      this.scale === another.scale
    );
  }
}

class ArkLinearGradientBlur {
  blurRadius: number | undefined;
  fractionStops: FractionStop[] | undefined;
  direction: number | undefined;

  constructor() {
    this.blurRadius = undefined;
    this.fractionStops = undefined;
    this.direction = undefined;
  }

  isEqual(another: ArkLinearGradientBlur): boolean {
    return (
      this.blurRadius === another.blurRadius &&
      ArkDeepCompareArrays(this.fractionStops, another.fractionStops) &&
      this.direction === another.direction
    );
  }
}

class ArkFont implements Equable {
  size: string | number | Resource;
  weight: string | number;
  family: string | Resource | undefined;
  style: number | undefined;

  constructor() {
    this.size = '16fp';
    this.weight = '400';
  }

  setFamily(family: string | Resource) {
    this.family = family;
  }

  setSize(size: string | number | Resource) {
    this.size = size;
  }

  setStyle(style: number) {
    this.style = style;
  }

  isEqual(another: ArkFont): boolean {
    return (
      this.size === another.size &&
      this.weight === another.weight &&
      this.family === another.family &&
      this.style === another.style
    );
  }

  parseFontWeight(value: string | number) {
    const valueWeightMap = {
      [0]: 'Lighter',
      [1]: 'Normal',
      [2]: 'Regular',
      [3]: 'Medium',
      [4]: 'Bold',
      [5]: 'Bolder'
    };
    if (value in valueWeightMap) {
      this.weight = valueWeightMap[value];
    } else {
      this.weight = value.toString();
    }
  }
}

class ArkMenuAlignType implements Equable {
  alignType: number;
  dx: number;
  dy: number;

  constructor() {
    this.alignType = 2;
    this.dx = 0;
    this.dy = 0;
  }

  isEqual(another: ArkMenuAlignType): boolean {
    return this.alignType === another.alignType && this.dx === another.dx && this.dy === another.dy;
  }
}

class ArkSliderTips implements Equable {
  showTip: boolean;
  tipText: string | undefined;

  constructor() {
    this.showTip = false;
    this.tipText = undefined;
  }

  isEqual(another: ArkSliderTips): boolean {
    return this.showTip === another.showTip && this.tipText === another.tipText;
  }
}

class ArkTextStyle implements Equable {
  color: number | string | undefined;
  font: ArkFont | undefined;

  constructor() {
    this.color = undefined;
    this.font = new ArkFont();
  }

  parseTextStyle(value: PickerTextStyle, defaultColor: string, defaultSize: string, defaultWeight: string) {
    if (isObject(value)) {
      let color = new ArkColor();
      let inputFont = value.font;
      let inputColor = value.color;

      if (!isUndefined(inputColor) && (isNumber(inputColor) || isString(inputColor))) {
        color.parseColorValue(inputColor);
        this.color = color.getColor();
      }

      if (!isUndefined(inputFont) && isObject(inputFont)) {
        if (!isUndefined(inputFont.size)) {
          this.font.size = inputFont.size;
        }

        if (!isUndefined(inputFont.weight)) {
          this.font.parseFontWeight(inputFont.weight);
        }
        this.font.family = inputFont.family;
        this.font.style = inputFont.style;
      }
    } else {
      this.color = defaultColor;
      this.font.size = defaultSize;
      this.font.parseFontWeight(defaultWeight);
    }
  }

  isEqual(another: ArkTextStyle): boolean {
    return this.color === another.color && this.font.isEqual(another.font);
  }
}

class ArkRadioStyle {
  checkedBackgroundColor: number | undefined;
  uncheckedBorderColor: number | undefined;
  indicatorColor: number | undefined;

  constructor() {
    this.checkedBackgroundColor = undefined;
    this.uncheckedBorderColor = undefined;
    this.indicatorColor = undefined;
  }

  isEqual(another: ArkRadioStyle): boolean {
    return (
      this.checkedBackgroundColor === another.checkedBackgroundColor &&
      this.uncheckedBorderColor === another.uncheckedBorderColor &&
      this.indicatorColor === another.indicatorColor
    );
  }
}

class ArkStarStyle implements Equable {
  backgroundUri: string | undefined;
  foregroundUri: string | undefined;
  secondaryUri: string | undefined;

  constructor() {
    this.backgroundUri = undefined;
    this.foregroundUri = undefined;
    this.secondaryUri = undefined;
  }

  isEqual(another: ArkStarStyle): boolean {
    return (
      this.backgroundUri === another.backgroundUri &&
      this.foregroundUri === another.foregroundUri &&
      this.secondaryUri === another.secondaryUri
    );
  }
}

class ArkBackgroundBlurStyle {
  blurStyle: number | undefined;
  colorMode: number | undefined;
  adaptiveColor: number | undefined;
  scale: number | undefined;

  constructor() {
    this.blurStyle = undefined;
    this.colorMode = undefined;
    this.adaptiveColor = undefined;
    this.scale = undefined;
  }

  isEqual(another: ArkBackgroundBlurStyle): boolean {
    return (
      this.blurStyle === another.blurStyle &&
      this.colorMode === another.colorMode &&
      this.adaptiveColor === another.adaptiveColor &&
      this.scale === another.scale
    );
  }
}

class ArkMarkStyle {
  strokeColor: number | undefined;
  size: number | string;
  strokeWidth: number | string;

  constructor() {
    this.strokeColor = undefined;
    this.size = undefined;
    this.strokeWidth = undefined;
  }

  isEqual(another: ArkMarkStyle): boolean {
    return (
      this.strokeColor === another.strokeColor && this.size === another.size && this.strokeWidth === another.strokeWidth
    );
  }

  parseMarkStyle(options: object): boolean {
    let arkColor = new ArkColor();
    if (!arkColor.parseColorValue((options as ArkMarkStyle).strokeColor)) {
      return false;
    }
    this.strokeColor = arkColor.getColor();
    this.size = (options as ArkMarkStyle).size;
    this.strokeWidth = (options as ArkMarkStyle).strokeWidth;
    return true;
  }
}

class ArkSelectedIndices implements Equable {
  selectedValues: number[];

  constructor() {
    this.selectedValues = [];
  }

  compareArrays(arr1: number[], arr2: number[]): boolean {
    return (
      Array.isArray(arr1) &&
      Array.isArray(arr2) &&
      arr1.length === arr2.length &&
      arr1.every((value, index) => value === arr2[index])
    );
  }

  isEqual(another: ArkSelectedIndices): boolean {
    return this.compareArrays(this.selectedValues, another.selectedValues);
  }
}

class ArkBlockSize {
  width: number | string | Resource | undefined;
  height: number | string | Resource | undefined;

  constructor() {
    this.width = undefined;
    this.height = undefined;
  }

  isEqual(another: ArkBlockSize): boolean {
    return this.width === another.width && this.height === another.height;
  }
}

class ArkDecoration {
  type: number;
  color?: string | number | undefined;
  constructor() {
    this.type = TextDecorationType.None;
    this.color = undefined;
  }
  isEqual(another: ArkDecoration): boolean {
    return this.type === another.type && this.color === another.color;
  }
}
class ArkBorder implements Equable {
  arkWidth: ArkBorderWidth;
  arkColor: ArkBorderColor;
  arkRadius: ArkBorderRadius;
  arkStyle: ArkBorderStyle;

  constructor() {
    this.arkWidth = new ArkBorderWidth();
    this.arkColor = new ArkBorderColor();
    this.arkRadius = new ArkBorderRadius();
    this.arkStyle = new ArkBorderStyle();
  }
  isEqual(another: ArkBorder): boolean {
    return (
      this.arkWidth.isEqual(another.arkWidth) &&
      this.arkColor.isEqual(another.arkColor) &&
      this.arkRadius.isEqual(another.arkRadius) &&
      this.arkStyle.isEqual(another.arkStyle)
    );
  }
}

class ArkBackgroundImagePosition implements Equable {
  alignment: number | undefined | Position | Alignment;
  x: number | string | undefined | Resource;
  y: number | string | undefined | Resource;
  constructor() {
    this.alignment = undefined;
    this.x = undefined;
    this.y = undefined;
  }
  isEqual(another: ArkBackgroundImagePosition): boolean {
    return this.alignment === another.alignment && this.x === another.x && this.y === another.y;
  }
}

class ArkBackgroundImageSize implements Equable {
  imageSize: ImageSize | undefined | SizeOptions;
  width: number | string | undefined | Resource;
  height: number | string | undefined | Resource;
  constructor() {
    this.imageSize = undefined;
    this.width = undefined;
    this.height = undefined;
  }
  isEqual(another: ArkBackgroundImageSize): boolean {
    return this.imageSize === another.imageSize && this.width === another.width && this.height === another.height;
  }
}

class ArkBackgroundImage implements Equable {
  src: string | undefined | Resource;
  repeat: number | undefined;
  constructor() {
    this.src = undefined;
    this.repeat = undefined;
  }
  isEqual(another: ArkBackgroundImage): boolean {
    return this.src === another.src && this.repeat === another.repeat;
  }
}

class ArkTranslate implements Equable {
  x: number | string | undefined;
  y: number | string | undefined;
  z: number | string | undefined;
  constructor() {
    this.x = undefined;
    this.y = undefined;
    this.z = undefined;
  }
  isEqual(another: ArkTranslate): boolean {
    return this.x === another.x && this.y === another.y && this.z === another.z;
  }
}

class ArkScale implements Equable {
  x: number | undefined;
  y: number | undefined;
  z: number | undefined;
  centerX: number | string | undefined;
  centerY: number | string | undefined;

  constructor() {
    this.x = undefined;
    this.y = undefined;
    this.z = undefined;
    this.centerX = undefined;
    this.centerY = undefined;
  }

  isEqual(another: ArkRotate): boolean {
    return (
      this.x === another.x &&
      this.y === another.y &&
      this.z === another.z &&
      this.centerX === another.centerX &&
      this.centerY === another.centerY
    );
  }
}

class ArkRotate implements Equable {
  x: number | undefined;
  y: number | undefined;
  z: number | undefined;
  angle: number | string | undefined;
  centerX: number | string | undefined;
  centerY: number | string | undefined;
  centerZ: number | string | undefined;
  perspective: number | undefined;

  constructor() {
    this.x = undefined;
    this.y = undefined;
    this.z = undefined;
    this.angle = undefined;
    this.centerX = undefined;
    this.centerY = undefined;
    this.centerZ = undefined;
    this.perspective = undefined;
  }

  isEqual(another: ArkRotate): boolean {
    return (
      this.x === another.x &&
      this.y === another.y &&
      this.z === another.z &&
      this.angle === another.angle &&
      this.centerX === another.centerX &&
      this.centerY === another.centerY &&
      this.centerZ === another.centerZ &&
      this.perspective === another.perspective
    );
  }
}

class ArkPixelStretchEffect {
  top: Length | undefined;
  right: Length | undefined;
  bottom: Length | undefined;
  left: Length | undefined;

  constructor() {
    this.top = undefined;
    this.right = undefined;
    this.bottom = undefined;
    this.left = undefined;
  }

  isEqual(another: ArkPixelStretchEffect): boolean {
    return (
      this.top === another.top &&
      this.right === another.right &&
      this.bottom === another.bottom &&
      this.left === another.left
    );
  }
}

class ArkForegroundColor {
  color: number | string | undefined;
  strategy: string | undefined;

  constructor() {
    this.color = undefined;
    this.strategy = undefined;
  }

  isEqual(another: ArkForegroundColor): boolean {
    return this.color === another.color && this.strategy === another.strategy;
  }
}

class ArkMotionPath implements Equable {
  path: string;
  from: number | undefined;
  to: number | undefined;
  rotatable: boolean | undefined;

  constructor() {
    this.path = undefined;
    this.from = undefined;
    this.to = undefined;
    this.rotatable = undefined;
  }

  isEqual(another: ArkMotionPath): boolean {
    return (
      this.path === another.path &&
      this.from === another.from &&
      this.to === another.to &&
      this.rotatable === another.rotatable
    );
  }
}

class ArkGridColColumnOption implements Equable {
  xs?: number;
  sm?: number;
  md?: number;
  lg?: number;
  xl?: number;
  xxl?: number;
  constructor() {
    this.xs = 0;
    this.sm = 0;
    this.md = 0;
    this.lg = 0;
    this.xl = 0;
    this.xxl = 0;
  }
  isEqual(another: ArkGridColColumnOption): boolean {
    return (
      this.xs === another.xs &&
      this.sm === another.sm &&
      this.md === another.md &&
      this.lg === another.lg &&
      this.xl === another.xl &&
      this.xxl === another.xxl
    );
  }
}
class ArkConstraintSizeOptions {
  minWidth?: string | number | undefined;
  maxWidth?: string | number | undefined;
  minHeight?: string | number | undefined;
  maxHeight?: string | number | undefined;

  constructor() {
    this.minWidth = undefined;
    this.maxWidth = undefined;
    this.minHeight = undefined;
    this.maxHeight = undefined;
  }

  isEqual(another: ArkConstraintSizeOptions): boolean {
    return (
      this.minWidth === another.minWidth &&
      this.maxWidth === another.maxWidth &&
      this.minHeight === another.minHeight &&
      this.maxHeight === another.maxHeight
    );
  }
}

class ArkSize {
  width: string | number | undefined;
  height: string | number | undefined;

  constructor() {
    this.width = undefined;
    this.height = undefined;
  }

  isEqual(another: ArkSize): boolean {
    return this.width === another.width && this.height === another.height;
  }
}

class ArkPadding implements Equable {
  top: string | number | undefined;
  right: string | number | undefined;
  bottom: string | number | undefined;
  left: string | number | undefined;
  constructor() {
    this.top = undefined;
    this.right = undefined;
    this.bottom = undefined;
    this.left = undefined;
  }
  isEqual(another: ArkPadding) {
    return (
      this.top === another.top &&
      this.right === another.right &&
      this.bottom === another.bottom &&
      this.left === another.left
    );
  }
}

class ArkAllowDrop {
  allowDropArray: Array<UniformDataType>;

  constructor() {
    this.allowDropArray = undefined;
  }

  isEqual(another: ArkAllowDrop): boolean {
    return this.allowDropArray === another.allowDropArray;
  }
}
enum TabBarMode {
  FIXED,
  SCROLLABLE,
  FIXED_START
}

class ArkBarMode {
  convertStrToTabBarMode(value: string): number {
    return value.toLowerCase() == 'fixed' ? TabBarMode.FIXED : TabBarMode.FIXED_START;
  }
}

class ArkObscured {
  reasons: number[];

  constructor() {
    this.reasons = undefined;
  }

  parseReasonsArray(reasonObject: object) {
    if (Array.isArray(reasonObject)) {
      for (let i = 0; i < reasonObject.length; i++) {
        this.reasons.push(reasonObject[i]);
      }
      return true;
    }
    return false;
  }

  isEqual(another: ArkObscured): boolean {
    return this.reasons === another.reasons;
  }
}

class ArkResponseRegion {
  responseRegion: string[];

  constructor() {
    this.responseRegion = undefined;
  }

  isObject(region: object) {
    return region !== null && !Array.isArray(region) && typeof region === 'object';
  }

  parseRegionValue(value: object) {
    if (Array.isArray(value)) {
      return this.parseArrayList(value);
    } else if (this.isObject(value)) {
      return this.parseArray(value);
    } else if (value === undefined) {
      this.responseRegion = [];
      return true;
    }
    return false;
  }

  parseArray(regionObject: object) {
    if (this.isObject(regionObject)) {
      let x = regionObject['x'].toString();
      let y = regionObject['y'].toString();
      let width = regionObject['width'].toString();
      let height = regionObject['height'].toString();
      this.responseRegion.push(x);
      this.responseRegion.push(y);
      this.responseRegion.push(width);
      this.responseRegion.push(height);
      return true;
    }
    return false;
  }

  parseArrayList(regionObject: object) {
    if (Array.isArray(regionObject)) {
      for (let i = 0; i < regionObject.length; i++) {
        this.parseArray(regionObject[i]);
      }
      return true;
    }
    return false;
  }

  isEqual(another: ArkResponseRegion): boolean {
    return this.responseRegion === another.responseRegion;
  }
}
class ArkAlignRules implements Equable {
  left: string | undefined;
  middle: string | undefined;
  right: string | undefined;
  top: string | undefined;
  center: string | undefined;
  bottom: string | undefined;
  constructor() {
    this.left = undefined;
    this.middle = undefined;
    this.right = undefined;
    this.top = undefined;
    this.center = undefined;
    this.bottom = undefined;
  }
  isEqual(another: ArkAlignRules) {
    return (
      this.left === another.left &&
      this.middle === another.middle &&
      this.right === another.right &&
      this.top === another.top &&
      this.center === another.center &&
      this.bottom === another.bottom
    );
  }
}

class ArkSafeAreaExpandOpts {
  type: string | number | undefined = undefined;
  edges: string | number | undefined = undefined;
  isEqual(another: ArkColumnSplitDividerStyle): boolean {
    return false;
  }
}

class ArkSideBarDividerStyle {
  strokeWidth: number | string;
  color?: string | number;
  startMargin?: string | number;
  endMargin?: string | number;
  constructor() {
    this.startMargin = 0;
    this.endMargin = 0;
  }
  isEqual(another: ArkSideBarDividerStyle): boolean {
    return (
      this.strokeWidth === another.strokeWidth &&
      this.color === another.color &&
      this.startMargin === another.startMargin &&
      this.endMargin === another.endMargin
    );
  }
}

class ArkColumnSplitDividerStyle implements Equable {
  startMargin?: string;
  endMargin?: string;
  constructor() {
    this.startMargin = '0';
    this.endMargin = '0';
  }
  isEqual(another: ArkColumnSplitDividerStyle): boolean {
    return false;
  }
}

class ArkButtonStyle {
  left?: number;
  top?: number;
  width?: number;
  height?: number;
  icons?: {
    shown?: string;
    hidden?: string;
    switching?: string;
  };
  constructor() {
    this.left = 16;
    this.top = 48;
    this.width = 24;
    this.height = 24;
    this.icons = {
      shown: undefined,
      hidden: undefined,
      switching: undefined
    };
  }
  isEqual(another: ArkButtonStyle): boolean {
    return (
      this.left === another.left &&
      this.top === another.top &&
      this.width === another.width &&
      this.height === another.height &&
      this.icons === another.icons
    );
  }
}

class ArkShadowInfoToArray {
  radius: (number | string)[];
  color: (Color | string | Resource | ColoringStrategy)[];
  offsetX: (number | string)[];
  offsetY: (number | string)[];
  fill: boolean[];
  constructor() {
    this.radius = [];
    this.color = [];
    this.offsetX = [];
    this.offsetX = [];
    this.offsetY = [];
    this.fill = [];
  }
  isEqual(another: ArkShadowInfoToArray): boolean {
    return (
      this.radius === another.radius &&
      this.color === another.color &&
      this.offsetX === another.offsetX &&
      this.offsetY === another.offsetY &&
      this.fill === another.fill
    );
  }
}
class ArkPasswordIcon implements Equable {
  onIconSrc: string | undefined;
  offIconSrc: string | undefined;
  constructor() {
    this.onIconSrc = undefined;
    this.offIconSrc = undefined;
  }
  isEqual(another: ArkPasswordIcon): boolean {
    return this.onIconSrc === another.onIconSrc && this.offIconSrc === another.offIconSrc;
  }
}
class ArkCaretStyle implements Equable {
  width: string | number | undefined;
  color: number | undefined;
  constructor() {
    this.width = undefined;
    this.color = undefined;
  }
  isEqual(another: ArkCaretStyle): boolean {
    return this.width === another.width && this.color === another.color;
  }
}
class ArkIconOptions implements Equable {
  size: string | number | undefined;
  color: number | undefined;
  src: string | undefined;
  constructor() {
    this.size = undefined;
    this.color = undefined;
    this.src = undefined;
  }
  isEqual(another: ArkIconOptions): boolean {
    return this.size === another.color && this.color === another.color && this.src === another.src;
  }
}
class ArkSearchButton implements Equable {
  value: string | undefined;
  fontSize: string | number | undefined;
  fontColor: number | undefined;
  constructor() {
    this.value = undefined;
    this.fontSize = undefined;
    this.fontColor = undefined;
  }
  isEqual(another: ArkSearchButton): boolean {
    return this.value === another.value && this.fontSize === another.fontSize && this.fontColor === another.fontColor;
  }
}
class ArkCancelButton implements Equable {
  style: CancelButtonStyle | undefined;
  color: number | undefined;
  size: string | number | undefined;
  src: string | undefined;
  constructor() {
    this.style = undefined;
    this.color = undefined;
    this.size = undefined;
    this.src = undefined;
  }
  isEqual(another: ArkCancelButton): boolean {
    return (
      this.style === another.style &&
      this.color === another.color &&
      this.size === another.size &&
      this.src === another.src
    );
  }
}
class ArkImageFrameInfoToArray implements Equable {
  arrSrc: Array<string> | undefined;
  arrWidth: Array<number | string> | undefined;
  arrHeight: Array<number | string> | undefined;
  arrTop: Array<number | string> | undefined;
  arrLeft: Array<number | string> | undefined;
  arrDuration: Array<number> | undefined;
  constructor() {
    this.arrSrc = [];
    this.arrWidth = [];
    this.arrHeight = [];
    this.arrTop = [];
    this.arrLeft = [];
    this.arrDuration = [];
  }
  isEqual(another: ArkImageFrameInfoToArray): boolean {
    return (
      this.arrSrc.toString() === another.arrSrc.toString() &&
      this.arrWidth.toString() === another.arrWidth.toString() &&
      this.arrHeight.toString() === another.arrHeight.toString() &&
      this.arrTop.toString() === another.arrTop.toString() &&
      this.arrLeft.toString() === another.arrLeft.toString() &&
      this.arrDuration.toString() === another.arrDuration.toString()
    );
  }
}

class ArkPickerTextStyle implements Equable {
    color: number;
    size: string;
    weight: string;

    constructor() {
        this.color = undefined;
        this.size = "16fp"
        this.weight = "Regular";
    }

    setColor(color: number | string | Resource){
        let arkColor = new ArkColor();
        arkColor.parseColorValue(color);
        this.color = arkColor.color;
    }

    setSize(size: number | string | Resource) {
        this.size = size.toString();
    }

    setWeight(value: string | number) {
        const valueWeightMap = {
            [0]: 'Lighter',
            [1]: 'Normal',
            [2]: 'Regular',
            [3]: 'Medium',
            [4]: 'Bold',
            [5]: 'Bolder'
        };
        if (value in valueWeightMap) {
            this.weight = valueWeightMap[value];
        } else {
            this.weight = value.toString();
        }
    }

    isEqual(another: ArkPickerTextStyle): boolean {
        return this.color ===another.color && this.size === another.size && this.weight === another.weight;
    }
}

class ArkEdgeAlign {
    alignType: number;
    offset?: number | string | undefined | Resource;

    constructor() {
        this.alignType = undefined;
        this.offset = undefined;
    }

    isEqual(another: ArkEdgeAlign): boolean {
        return (this.alignType === another.alignType && this.offset === another.offset);
    }
}

class ArkClickEffect implements Equable {
    level: number | undefined;
    scale: number | undefined;

    constructor() {
        this.level = undefined;
        this.scale = undefined;
    }

    isEqual(another) {
        return (this.level === another.level) && (this.scale === another.scale);
    }
}

class ArkKeyBoardShortCut {
    value: string | number;
    keys: Array<ModifierKey>;
    action: () => void | undefined;

    constructor() {
        this.value = undefined;
        this.keys = undefined;
        this.action = undefined;
    }

    isEqual(another: ArkKeyBoardShortCut): boolean {
        return (this.value === another.value) && (this.keys === another.keys) &&
            (this.action === another.action);
    }
}
