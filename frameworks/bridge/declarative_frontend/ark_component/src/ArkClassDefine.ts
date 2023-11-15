/// <reference path="./import.ts" />

class ArkBorderStyle implements Equable {
    type: boolean | undefined;
    style: number | undefined;
    top: number | undefined;
    right: number | undefined;
    bottom: number | undefined;
    left: number | undefined;

    constructor() {
        this.type = undefined;
        this.style = undefined;
        this.top = undefined;
        this.right = undefined;
        this.bottom = undefined;
        this.left = undefined;
    }
    isEqual(another: ArkBorderStyle): boolean {
        return (this.type === another.type) && (this.style === another.style) &&
            (this.top === another.top) && (this.right === another.right) &&
            (this.bottom === another.bottom) && (this.left === another.left);
    }
    parseBorderStyle(value: number | object): boolean {
        if (typeof value === "number") {
            this.style = value
            this.type = true
            return true;
        } else if (typeof value === "object") {
            return this.parseEdgeStyles(value);
        }
        return false;
    }
    parseEdgeStyles(options: object): boolean {
        this.top = (options as ArkBorderStyle).top;
        this.right = (options as ArkBorderStyle).right;
        this.bottom = (options as ArkBorderStyle).bottom;
        this.left = (options as ArkBorderStyle).left;
        this.type = true
        return true
    }
}

class ArkShadow {
    style: number | undefined;
    radius: number | undefined;
    type: number | undefined;
    color: number | undefined;
    offsetX: number | undefined;
    offsetY: number | undefined;
    fill: number | undefined;

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
        return (this.style === another.style) && (this.radius === another.radius) &&
            (this.type === another.type) && (this.color === another.color) &&
            (this.offsetX === another.offsetX) && (this.offsetY === another.offsetY) &&
            (this.fill === another.fill);
    }
    parseShadowValue(value: number | object): boolean {
        if (typeof value === "number") {
            this.style = value
            return true;
        } else if (typeof value === "object") {
            return this.parseShadowOptions(value);
        }
        return false;
    }
    parseShadowOptions(options: object): boolean {
        var arkColor = new ArkColor();
        if (!arkColor.parseColorValue((options as ArkShadow).color)) {
            return false
        }
        this.radius = (options as ArkShadow).radius
        this.type = (options as ArkShadow).type
        this.color = arkColor.getColor();
        this.offsetX = (options as ArkShadow).offsetX
        this.offsetY = (options as ArkShadow).offsetY
        this.fill = (options as ArkShadow).fill
        return true
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
        return (this.leftColor === another.leftColor && this.rightColor === another.rightColor && this.topColor === another.topColor && this.bottomColor === another.bottomColor);
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
        return (this.x === another.x) && (this.y === another.y);
    }
}


class ArkBorderWidth implements Equable {
    left: number | string | undefined | Resource;
    right: number | string | undefined| Resource;
    top: number | string | undefined| Resource;
    bottom: number | string | undefined| Resource;

    constructor() {
        this.left = undefined;
        this.right = undefined;
        this.top = undefined;
        this.bottom = undefined;
    }

    isEqual(another: ArkBorderWidth): boolean {
        return (this.left === another.left && this.right === another.right && this.top === another.top && this.bottom === another.bottom);
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
        return (this.topLeft === another.topLeft && this.topRight === another.topRight && this.bottomLeft === another.bottomLeft && this.bottomRight === another.bottomRight);
    }
}

class ArkTransformMatrix implements Equable  {
    matrix: [];
    length: number;

    constructor(matrix: []) {
        this.matrix = matrix;
        this.length = 16;
    }

    compareArrays(arr1: [], arr2: []): boolean {
        return Array.isArray(arr1)
            && Array.isArray(arr2)
            && arr1.length === arr2.length
            && arr1.every((value, index) => value === arr2[index]);
    }

    isEqual(another: ArkTransformMatrix): boolean {
        return this.compareArrays(this.matrix, another.matrix)
    }
}

class ArkLabelStyle {
    overflow: number | undefined;
    maxLines: number | undefined;
    minFontSize: number | string | undefined| Resource;
    maxFontSize: number | string | undefined| Resource;
    heightAdaptivePolicy: number | undefined;
    font: ArkLabelFont;

    constructor() {
        this.overflow = undefined;
        this.maxLines = undefined;
        this.minFontSize = undefined;
        this.maxFontSize = undefined;
        this.heightAdaptivePolicy = undefined;
        this.font = undefined;
    }

    isEqual(another: ArkLabelStyle): boolean {
        return (this.overflow === another.overflow && this.maxLines === another.maxLines &&
            this.minFontSize === another.minFontSize && this.maxFontSize === another.maxFontSize &&
            this.heightAdaptivePolicy === another.heightAdaptivePolicy && this.font.isEqual(another.font));
    }
}

class ArkLabelFont {
    size: number | string | undefined| Resource;
    weight: string | undefined;
    family: string | undefined;
    style:  number | undefined;
    constructor() {
        this.size = undefined;
        this.weight = undefined;
        this.family = undefined;
        this.style = undefined;
    }

    isEqual(another: ArkLabelFont): boolean {
        return (this.size === another.size && this.weight === another.weight && this.family === another.family &&
            this.style === another.style);
    }
}

function ArkDeepCompareArrays(arr1: Array<any>, arr2: Array<any>): boolean {
    return Array.isArray(arr1)
        && Array.isArray(arr2)
        && arr1.length === arr2.length
        && arr1.every((value, index) => {
            if (Array.isArray(value) && Array.isArray(arr2[index])) {
                return ArkDeepCompareArrays(value, arr2[index]);
            } else {
                return (value === arr2[index]);
            }
        });
}

class ArkLinearGradient {
    angle: number | string | undefined;
    direction: number | undefined;
    colors: Array<any>;
    repeating: boolean | undefined;

    constructor(angle: number | string | undefined,
        direction: number | undefined,
        colors: Array<any>,
        repeating: boolean | undefined) {
        this.angle = angle;
        this.direction = direction;
        this.colors = colors;
        this.repeating = repeating;
    }

    isEqual(another: ArkLinearGradient): boolean {
        return ((this.angle === another.angle) && (this.direction === another.direction) &&
            (ArkDeepCompareArrays(this.colors, another.colors)) && (this.repeating === another.repeating));
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
        return ((this.blurStyle === another.blurStyle) &&
            (this.colorMode === another.colorMode) && (this.adaptiveColor === another.adaptiveColor) &&
            (this.scale === another.scale));
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
        return ((this.blurRadius === another.blurRadius) &&
            (ArkDeepCompareArrays(this.fractionStops, another.fractionStops)) &&
            (this.direction === another.direction));
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
        return ((this.blurStyle === another.blurStyle) &&
            (this.colorMode === another.colorMode) && (this.adaptiveColor === another.adaptiveColor) &&
            (this.scale === another.scale));
    }
}


class ArkFont implements Equable{
    size: string | number | Resource;
    weight: string;
    family: string | Resource | undefined;
    style: number | undefined;

    constructor() {
        this.size = "16fp"
        this.weight = "400";
    }

    setFamily(family:string | Resource){
        this.family = family;
    }

    setSize(size: string | number | Resource){
        this.size = size;
    }

    setStyle(style:number){
        this.style = style;
    }

    isEqual(another: ArkFont): boolean {
        return this.size === another.size && this.weight === another.weight && this.family === another.family && this.style === another.style;
    }
    
    parseFontWeight(value: string | number){
        if(typeof value === 'number'){
            if(value === 0){
                this.weight = "Lighter"
            }
            else if(value === 1){
                this.weight = "Normal"
            }
            else if(value === 2){
                this.weight = "Regular"
            }
            else if(value === 3){
                this.weight = "Medium"
            }
            else if(value === 4){
                this.weight = "Bold"
            }
            else if(value === 5){
                this.weight = "Bolder"
            }
            else{
                this.weight = value.toString()            
            }
        }       
        else if(typeof value === 'string'){
            this.weight =  value
        }
        else {
            this.weight = "400";
        }
    }
}

class ArkMenuAlignType implements Equable{
    alignType: number;
    dx: number;
    dy: number;

    constructor(){
        this.alignType = 2;
        this.dx = 0;
        this.dy = 0;
    }

    isEqual(another: ArkMenuAlignType): boolean {
        return (this.alignType === another.alignType) && (this.dx === another.dx) && (this.dy === another.dy);
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
        return this.showTip === another.showTip && this.tipText === another.tipText
    }

}

class ArkTextStyle implements Equable {
    color : number | string | undefined
    font : ArkFont | undefined

    constructor(){
        this.color = undefined;
        this.font = new ArkFont();
    }

    parseTextStyle(value: PickerTextStyle, defaultColor: string, defaultSize: string, defaultWeight: string){
        if (isObject(value)){
            let color = new ArkColor();
            let inputFont = value.font;
            let inputColor = value.color;

            if(!isUndefined(inputColor) && (isNumber(inputColor) || isString(inputColor))){                
                color.parseColorValue(inputColor);
                this.color = color.getColor();              
            }
            
            if (!isUndefined(inputFont) && isObject(inputFont)) {
                
                if (!isUndefined(inputFont.size) ) {
                    this.font.size = inputFont.size;
                }

                if (!isUndefined(inputFont.weight) ) {
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
        return this.color === another.color && this.font.isEqual(another.font)
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
        return (this.checkedBackgroundColor === another.checkedBackgroundColor 
            && this.uncheckedBorderColor === another.uncheckedBorderColor 
            && this.indicatorColor === another.indicatorColor);
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
        return this.backgroundUri === another.backgroundUri
            && this.foregroundUri === another.foregroundUri 
            && this.secondaryUri === another.secondaryUri;
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
        return (this.strokeColor === another.strokeColor) && (this.size === another.size) && (this.strokeWidth === another.strokeWidth);
    }

    parseMarkStyle(options: object): boolean {
        var arkColor = new ArkColor();
        if (!arkColor.parseColorValue((options as ArkMarkStyle).strokeColor)) {
            return false
        }
        this.strokeColor = arkColor.getColor();
        this.size = (options as ArkMarkStyle).size;
        this.strokeWidth = (options as ArkMarkStyle).strokeWidth;
        return true
    }
}

class ArkSelectedIndices implements Equable {
    selectedValues: number []

    constructor(){
        this.selectedValues = [];
    }

    compareArrays(arr1:number [], arr2:number []): boolean {
        return Array.isArray(arr1)
            && Array.isArray(arr2)
            && arr1.length === arr2.length
            && arr1.every((value, index) => value === arr2[index]);
    }

    isEqual(another: ArkSelectedIndices): boolean {
        return this.compareArrays(this.selectedValues, another.selectedValues)
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
        return (this.width === another.width) && (this.height === another.height);
    }
}