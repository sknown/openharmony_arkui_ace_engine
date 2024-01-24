/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

/**
 * @addtogroup ArkUI_NativeModule
 * @{
 *
 * @brief 提供ArkUI在Native侧的UI能力，如UI组件创建销毁、树节点操作，属性设置，事件监听等。
 *
 * @since 12
 */

/**
 * @file native_node.h
 *
 * @brief 提供NativeNode接口的类型定义。
 *
 * @library libace_ndk.z.so
 * @syscap SystemCapability.ArkUI.ArkUI.Full
 * @since 12
 */

#ifndef ARKUI_NATIVE_NODE_H
#define ARKUI_NATIVE_NODE_H

#include "native_event.h"
#include "native_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NODE_SCOPE_NUM 1000

/**
 * @brief 提供ArkUI在Native侧可创建组件类型。
 *
 * @since 12
 */
typedef enum {
    /** 自定义节点。 */
    ARKUI_NODE_CUSTOM = 0,
    /** 文本。 */
    ARKUI_NODE_TEXT = 1,
    /** 文本段落。 */
    ARKUI_NODE_SPAN,
    /** 文本图片段落。 */
    ARKUI_NODE_IMAGE_SPAN,
    /** 图片。 */
    ARKUI_NODE_IMAGE,
    /** 状态开关。 */
    ARKUI_NODE_TOGGLE,
    /** 等待图标。 */
    ARKUI_NODE_LOADING_PROGRESS,
    /** 单行文本输入。 */
    ARKUI_NODE_TEXT_INPUT,
    /** 堆叠容器。 */
    ARKUI_NODE_STACK,
    /** 滚动容器。 */
    ARKUI_NODE_SCROLL,
    /** 列表。 */
    ARKUI_NODE_LIST,
    /** 翻页容器。 */
    ARKUI_NODE_SWIPER,
    /** 多行文本。 */
    ARKUI_NODE_TEXT_AREA,
    /** 按钮。 */
    ARKUI_NODE_BUTTON,
    /** 进度条。 */
    ARKUI_NODE_PROGRESS,
    /** 复选框。 */
    ARKUI_NODE_CHECKBOX,
    /** 垂直布局容器。 */
    ARKUI_NODE_COLUMN,
    /** 水平布局容器。 */
    ARKUI_NODE_ROW,
    /** 弹性布局容器。 */
    ARKUI_NODE_FLEX,
    /** 列表项。 */
    ARKUI_NODE_LIST_ITEM,
    /** 刷新组件。 */
    ARKUI_NODE_REFRESH,
    /** XComponent。 */
    ARKUI_NODE_XCOMPONENT,
    /** 列表item分组。 */
    ARKUI_NODE_LIST_ITEM_GROUP,
    /** 日期选择器组件。 */
    ARKUI_NODE_DATE_PICKER,
    /** 时间选择组件。 */
    ARKUI_NODE_TIME_PICKER,
    /** 滑动选择文本内容的组件。 */
    ARKUI_NODE_TEXT_PICKER,
    /** 滑动条组件 */
    ARKUI_NODE_SLIDER,
} ArkUI_NodeType;

/**
 * @brief 定义{@link setAttribute}函数通用入参结构。
 *
 * @since 12
 */
typedef struct {
    /** 数字类型数组。*/
    const ArkUI_NumberValue* value;
    /** 数字类型数组大小。*/
    int32_t size;
    /** 字符串类型。*/
    const char* string;
    /** 对象类型。*/
    void* object;
} ArkUI_AttributeItem;

/**
 * @brief 定义ArkUI在Native侧可以设置的属性样式集合。
 *
 * @since 12
 */
typedef enum {
    /**
     * @brief 通过{@link setAttribute}方法设置宽度属性。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].f32：宽度数值，单位为vp。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 1.2 };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_WIDTH, &item);
     * @endcode
     *
     */
    NODE_WIDTH = 0,
    /**
     * @brief 通过{@link setAttribute}方法设置高度属性。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].f32：高度数值，单位为vp。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 100 };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_HEIGHT, &item);
     * @endcode
     *
     */
    NODE_HEIGHT,
    /**
     * @brief 通过{@link setAttribute}方法设置背景色属性。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：背景色数值，0xargb格式，形如 0xFFFF0000 表示红色。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32=0xFFFF0000} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_BACKGROUND_COLOR, &item);
     * @endcode
     *
     */
    NODE_BACKGROUND_COLOR,
    /**
     * @brief 通过{@link setAttribute}方法设置背景图片属性。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .string: 图片地址；\n
     * .value[0]?.i32：可选值，repeat参数，参数类型{@link ArkUI_ImageRepeat}，默认值为ARKUI_IMAGEREPEAT_NO_REPEAT。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = ARKUI_IMAGEREPEAT_NO_REPEAT} };
     * ArkUI_AttributeItem item = { value, sizeof(value)/sizeof(ArkUI_NumberValue), "/pages/icon.png" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_BACKGROUND_IMAGE, &item);
     * @endcode
     *
     */
    NODE_BACKGROUND_IMAGE,
    /**
     * @brief 通过{@link setAttribute}方法设置组件内间距。
     *
     * {@link ArkUI_AttributeItem}支持两种入参格式：\n
     * 1：上下左右四个位置的内间距值相等。\n
     * .value[0].f32：内间距数值，单位为vp；\n
     * 2：分别指定上下左右四个位置的内间距值。\n
     * .value[0].f32：上内间距数值，单位为vp；\n
     * .value[1].f32：右内间距数值，单位为vp；\n
     * .value[2].f32：下内间距数值，单位为vp；\n
     * .value[3].f32：左内间距数值，单位为vp。
     *
     * @code {.c}
     * ArkUI_NumberValue value1[] = { 1, 2, 3, 4};
     * ArkUI_AttributeItem item1 = { value1, sizeof(value1)/sizeof(ArkUI_NumberValue) };
     * ArkUI_NumberValue value2[] = { 10 };
     * ArkUI_AttributeItem item2 = { value2, sizeof(value2)/sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_PADDING, &item1);
     * basicNodeApi->setAttribute(nodeHandle, NODE_PADDING, &item2);
     * @endcode
     *
     */
    NODE_PADDING,
    /**
     * @brief 通过{@link setAttribute}方法设置组件ID。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * string: ID的内容。
     *
     * @code {.c}
     * ArkUI_AttributeItem item = { .string = "test" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_ID, &item);
     * @endcode
     *
     */
    NODE_ID,
    /**
     * @brief 通过{@link setAttribute}方法设置组件是否可交互，设置为false可不响应点击等操作。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * value[0].i32：false表示不可交互，true表示可交互。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = false} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_ENABLED, &item);
     * @endcode
     */
    NODE_ENABLED,
    /**
     * @brief 通过{@link setAttribute}方法设置组件外间距。
     *
     * {@link ArkUI_AttributeItem}支持两种入参格式：\n
     * 1：上下左右四个位置的外间距值相等。\n
     * .value[0].f32：外间距数值，单位为vp；\n
     * 2：分别指定上下左右四个位置的外间距值。\n
     * .value[0].f32：上外间距数值，单位为vp；\n
     * .value[1].f32：右外间距数值，单位为vp；\n
     * .value[2].f32：下外间距数值，单位为vp；\n
     * .value[3].f32：左外间距数值，单位为vp。
     *
     * @code {.c}
     * ArkUI_NumberValue value1[] = { 1, 2, 3, 4};
     * ArkUI_AttributeItem item1 = { value1, sizeof(value1)/sizeof(ArkUI_NumberValue) };
     * ArkUI_NumberValue value2[] = { 10 };
     * ArkUI_AttributeItem item2 = { value2, sizeof(value2)/sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_MARGIN, &item1);
     * basicNodeApi->setAttribute(nodeHandle, NODE_MARGIN, &item2);
     * @endcode
     *
     */
    NODE_MARGIN,
    /**
     * @brief 通过{@link setAttribute}方法设置组件平移。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32： x轴移动距离，单位vp，默认值0；\n
     * .value[1].f32： y轴移动距离，单位vp，默认值0；\n
     * .value[2].f32： z轴移动距离，单位vp，默认值0。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 100, 20, 0 };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TRANSLATE, &item);
     * @endcode
     *
     */
    NODE_TRANSLATE,
    /**
     * @brief 通过{@link setAttribute}方法设置组件缩放。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32： x轴的缩放系数，默认值1；\n
     * .value[1].f32： y轴的缩放系数，默认值1。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 1.0, 0.5 };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCALE, &item);
     * @endcode
     *
     */
    NODE_SCALE,
    /**
     * @brief 通过{@link setAttribute}方法设置组件旋转。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32： 旋转轴向量x坐标，默认值0；\n
     * .value[1].f32： 旋转轴向量y坐标，默认值0；\n
     * .value[2].f32： 旋转轴向量z坐标，默认值0；\n
     * .value[3].f32： 旋转角度，默认值0；\n
     * .value[4].f32： 视距，即视点到z=0平面的距离，单位vp，默认值0。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 0, 0, 1, 300, 0 };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_ROTATE, &item);
     * @endcode
     *
     */
    NODE_ROTATE,
    /**
     * @brief 通过{@link setAttribute}方法设置组件高光效果。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32： 亮度值，默认值1.0，推荐取值范围[0,2]。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 1.2 };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_BRIGHTNESS, &item);
     * @endcode
     *
     */
    NODE_BRIGHTNESS,
    /**
     * @brief 通过{@link setAttribute}方法设置组件饱和度效果。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32： 饱和度值，默认值1.0，推荐取值范围[0,FLT_MAX]。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 1.0 };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_SATURATE, &item);
     * @endcode
     *
     */
    NODE_SATURATE,
    /**
     * @brief 通过{@link setAttribute}方法设置组件内容模糊效果。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32： 模糊半径，模糊半径越大越模糊，为0时不模糊。单位vp，默认值0.0。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 1.0 };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_BLUR, "1.0");
     * @endcode
     *
     */
    NODE_BLUR,
    /**
     * @brief 通过{@link setAttribute}方法设置组件颜色渐变效果。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 字符串组合参数，入参4个，以分号分割：\n
     *      入参1： 指定某百分比位置处的渐变色颜色,设置非法颜色直接跳过。颜色和位置（单位vp）数组类型，以逗号分割；\n
     *      入参2： 线性渐变的起始角度。0点方向顺时针旋转为正向角度，默认值：180；\n
     *      入参3： 线性渐变的方向，设置angle后不生效。取值("left","top","right","bottom","left-top","left-bottom","right-top",\n
     *             "right-bottom","none", 默认值 "bottom")；\n
     *      入参4： 为渐变的颜色重复着色，默认值 false；\n
     *      如 "#ffff0000,0.0,#ff0000ff,0.3,#ffffff00,0.5;;left;true" 。
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "#ffff0000,0.0,#ff0000ff,0.3,#ffffff00,0.5;;left;true" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_LINEAR_GRADIENT, &item);
     * @endcode
     *
     */
    NODE_LINEAR_GRADIENT,
    /**
     * @brief 通过{@link setAttribute}方法设置组件内容在元素绘制区域内的对齐方式。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32： 对齐方式，数据类型{@link ArkUI_Alignment}，默认值ARKUI_ALIGNMENT_CENTER。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_ALIGNMENT_CENTER } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_ALIGN, "center");
     * @endcode
     *
     */
    NODE_ALIGN,
    /**
     * @brief 通过{@link setAttribute}方法设置透明度。
     *
     * @note 入参格式为数字类型字符串,取值范围为0到1。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_OPACITY, "1");
     * @endcode
     */
    NODE_OPACITY,

    /**
     * @brief 通过{@link setAttribute}方法设置元素的边框宽度。
     *
     * @note 入参格式为数字类型字符串或4个数字类型字符串。分别对应上、右、下、左，使用空格隔开,
     * 如"1"或"1 1 2 2"，单位为vp。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_BORDER_WIDTH, "1");
     * basicNodeApi->setAttribute(nodeHandle, NODE_BORDER_WIDTH, "1 1 2 2");
     * @endcode
     */
    NODE_BORDER_WIDTH,

    /**
     * @brief 通过{@link setAttribute}方法设置元素的边框圆角。
     *
     * @note 入参格式为数字类型字符或4个数字类型字符串。分别对应左上、右上、左下、右下，使用空格隔开，如"1"或"1 1 2
     * 2"，单位为vp。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_BORDER_RADIUS, "1");
     * basicNodeApi->setAttribute(nodeHandle, NODE_BORDER_RADIUS, "1 1 2 2");
     * @endcode
     */
    NODE_BORDER_RADIUS,

    /**
     * @brief 通过{@link setAttribute}方法设置元素的边框颜色。
     *
     * @note 入参格式为#argb类型字符或4个#argb类型字符串。分别对应上、右、下、左,使用空格隔开，如"#FF1122FF"或"#FF1122FF
     * #FF1122FF #FFFFFFF #FFFFFFF"。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_BORDER_COLOR, "#FF1122FF");
     * basicNodeApi->setAttribute(nodeHandle, NODE_BORDER_COLOR, "#FF1122FF #FF1122FF #FFFFFFF #FFFFFFF");
     * @endcode
     */
    NODE_BORDER_COLOR,

    /**
     * @brief 通过{@link setAttribute}方法设置元素的边框线条样式。
     *
     * @note 入参格式"dotted"、"dashed"或"solid"或4个使用"dotted"、"dashed"或"solid"的字符串。分别对应上、右、下、左,
     * 使用空格隔开,如"dotted"或"dotted dashed dashed solid"。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_BORDER_STYLE, "dotted");
     * basicNodeApi->setAttribute(nodeHandle, NODE_BORDER_STYLE, "dotted dashed dashed solid");
     * @endcode
     */
    NODE_BORDER_STYLE,

    /**
     * @brief 通过{@link setAttribute}方法设置组件的堆叠顺序。
     *
     * @note 入参格式为数字类型字符串。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_ZINDEX, "1");
     * @endcode
     */
    NODE_ZINDEX,

    /**
     * @brief 通过{@link setAttribute}方法设置组件是否可见。
     *
     * @note 入参格式为"visible"、"hidden"与"none"。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_VISIBILITY, "visible");
     * @endcode
     */
    NODE_VISIBILITY,

    /**
     * @brief 通过{@link setAttribute}方法设置组件进行裁剪、遮罩处理。
     *
     * @note 入参格式为"true"、"false"，或为特定格式的字符串
     * 如"rect(10,10,10,10)"括号内分别为width、height、radiusWidth与radiusHeight";"circle(10,10)"括号内分别为width、height;
     * "ellipse(10,10)"括号内分别为width、height; "path(10,10,M0 0 L600 0)"括号内分别为width、height、commands。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_CLIP, "true");
     * basicNodeApi->setAttribute(nodeHandle, NODE_CLIP, "rect(10 10 10)");
     * @endcode
     */
    NODE_CLIP,

    /**
     * @brief 通过{@link setAttribute}方法设置矩阵变换功能，可对图形进行平移、旋转和缩放等。
     *
     * @note 入参格式为16个数字类型字符串,用空格隔开，如""1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1"。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_TRANSFORM, "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1");
     * @endcode
     *
     */
    NODE_TRANSFORM,

    /**
     * @brief 通过{@link setAttribute}方法设置组件的触摸测试类型。
     *
     * @note 入参格式为"default"、"block"、"transparent"与"none"。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_HIT_TEST_BEHAVIOR, "default");
     * @endcode
     *
     */
    NODE_HIT_TEST_BEHAVIOR,

    /**
     * @brief 通过{@link setAttribute}方法设置元素左上角相对于父容器左上角偏移位置。
     *
     * @note 入参格式为数字类型字符串,需传入x与y两个偏移量，用空格隔开，如"50 100",单位为vp。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_POSITION, "50 100");
     * @endcode
     *
     */
    NODE_POSITION,

    /**
     * @brief 通过{@link setAttribute}方法设置组件添加阴影效果。
     *
     * @note
     * 入参格式为"outer-default-xs"、"outer-default-sm"、"outer-default-md"、"outer-default-lg"、"outer-floating-sm"、
     * "outer-floating-md"与"none"字符串,或为6个字符串。分别对应radius、offsetX、offsetY、type、color与IsFilled,
     * type支持color与blur，color支持#argb类型字符与智能取色的枚举字符串invert、average与primary用空格隔开，
     * 如"5 10 10 color #FF1122FF true"。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SHADOW, "outer-default-md");
     * basicNodeApi->setAttribute(nodeHandle, NODE_SHADOW, "5 10 10 Color #FF1122FF true");
     * @endcode
     *
     */
    NODE_SHADOW,
    /**
     * @brief 通过<b>setAttribute</b>方法设置组件背景图片的宽高。
     *
     * @note
     * 入参width:number,height:number （默认单位vp）。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_BACKGROUND_IMAGE_SIZE, "20 40");
     * @endcode
     *
     */
    NODE_BACKGROUND_IMAGE_SIZE,
    /**
     * @brief 通过<b>setAttribute</b>为当前组件提供一种背景和内容之间的模糊能力。
     *
     * @note 入参blurStyle:String("thin","regular","thick")。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_BACKGROUND_BLUR_STYLE, "thin");
     * @endcode
     *
     */
    NODE_BACKGROUND_BLUR_STYLE,
    /**
     * @brief 通过<b>setAttribute</b>为当前组件设置转场缩放或者旋转的中心点。
     *
     * @note 入参: centerX: number，centerY: number，centerZ: number表示中心点。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_TRANSITION_CENTER, "20 20 0");
     * @endcode
     */
    NODE_TRANSITION_CENTER,
    /**
     * @brief 通过<b>setAttribute</b>为当前组件设置转场时的透明度效果。
     *
     * @note 入参: opacity: number，duration: number,
     * curve: String("linear","ease","easein","easeout","easeinout","fastoutslowin","linearoutslowin",
     * "fastoutlinearin"，"extremedeceleration"，"sharp"，"rhythm"，"smooth"，"friction")，delay: number，iterations:
     * number，playmode: String("normal"，"alternate"，"reverse"，"alternate_reverse")，tempo: number
     * opacity表示插入时的透明度起点值和删除时的透明度终点值。
     * duration表示动画播放时长，单位毫秒。curve表示动画曲线。
     * delay表示动画延迟播放时间，单位毫秒。
     * iterations表示动画播放次数，默认1次。
     * playmode表示动画播放模式，默认播放完从头开始播放。
     * tempo表示动画播放速度，值越大播放越快，默认值1，0表示无动画。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_OPACITY_TRANSITION, "0.5 3000 ease");
     * @endcode
     */
    NODE_OPACITY_TRANSITION,
    /**
     * @brief 通过<b>setAttribute</b>为当前组件设置转场时的旋转效果。
     *
     * @note 入参: x: number，y: number，z: number，angle: number，perspective: number，duration: number,
     * curve: String("linear"，"ease"，"easein"，"easeout"，"easeinout"，"fastoutslowin"，"linearoutslowin",
     * "fastoutlinearin"，"extremedeceleration"，"sharp"，"rhythm"，"smooth"，"friction")，delay: number，iterations:
     * number，playmode: String("normal"，"alternate"，"reverse"，"alternate_reverse")，tempo: number
     * 表示插入时的起点值和删除时的终点值。
     * 其中x表示横向旋转分量，y表示纵向的旋转分量，z表示竖向的旋转分量。
     * duration表示动画播放时长，单位毫秒。curve表示动画曲线。
     * delay表示动画延迟播放时间，单位毫秒。
     * iterations表示动画播放次数，默认1次。
     * playmode表示动画播放模式，默认播放完从头开始播放。
     * tempo表示动画播放速度，值越大播放越快，默认值1，0表示无动画。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_ROTATE_TRANSITION, "0 0 1 180 0 3000 ease");
     * @endcode
     */
    NODE_ROTATE_TRANSITION,
    /**
     * @brief 通过<b>setAttribute</b>为当前组件设置转场时的缩放效果。
     *
     * @note 入参: x: number,y: number,z: number，duration: number,
     * curve: String("linear","ease","easein","easeout","easeinout","fastoutslowin","linearoutslowin",
     * "fastoutlinearin"，"extremedeceleration"，"sharp"，"rhythm"，"smooth"，"friction")，delay: number，iterations:
     * number，playmode: String("normal"，"alternate"，"reverse"，"alternate_reverse")，tempo: number
     * 其中x表示横向放大倍数，y表示纵向的放大倍数，z表示竖向的放大倍数。
     * duration表示动画播放时长，单位毫秒。curve表示动画曲线。
     * delay表示动画延迟播放时间，单位毫秒。
     * iterations表示动画播放次数，默认1次。
     * playmode表示动画播放模式，默认播放完从头开始播放。
     * tempo表示动画播放速度，值越大播放越快，默认值1，0表示无动画。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCALE_TRANSITION, "0 0 0 3000 ease");
     * @endcode
     */
    NODE_SCALE_TRANSITION,
    /**
     * @brief 通过<b>setAttribute</b>为当前组件设置转场时的平移效果。
     *
     * @note 入参: x: number，y: number，z: number，duration: number,
     * curve: String("linear"，"ease"，"easein"，"easeout"，"easeinout"，"fastoutslowin"，"linearoutslowin",
     * "fastoutlinearin"，"extremedeceleration"，"sharp"，"rhythm"，"smooth"，"friction")，delay: number，iterations:
     * number，playmode: String("normal"，"alternate"，"reverse"，"alternate_reverse")，tempo: number
     * x表示横向的平移距离，y表示纵向的平移距离，z表示竖向的平移距离。
     * duration表示动画播放时长，单位毫秒。curve表示动画曲线。
     * delay表示动画延迟播放时间，单位毫秒。
     * iterations表示动画播放次数，默认1次。
     * playmode表示动画播放模式，默认播放完从头开始播放。
     * tempo表示动画播放速度，值越大播放越快，默认值1，0表示无动画。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_TRANSLATE_TRANSITION, "12 50 0 3000 ease");
     * @endcode
     */
    NODE_TRANSLATE_TRANSITION,

    /**
     * @brief 通过{@link setAttribute}方法设置当前组件是否可以获焦。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：参数类型为1或者0。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 1 } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_FOCUSABLE, &item);
     * @endcode
     *
     */
    NODE_FOCUSABLE,

    /**
     * @brief 通过{@link setAttribute}方法设置无障碍组。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：为1时表示该组件及其所有子组件为一整个可以选中的组件
     * 无障碍服务将不再关注其子组件内容。参数类型为1或者0。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 1 } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_ACCESSIBILITY_GROUP, &item);
     * @endcode
     *
     */
    NODE_ACCESSIBILITY_GROUP,

    /**
     * @brief 通过{@link setAttribute}方法设置无障碍文本。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .string：无障碍文本。
     * 
     * @code {.c}
     * ArkUI_AttributeItem item = {.string = "test"};
     * basicNodeApi->setAttribute(nodeHandle, NODE_ACCESSIBILITY_TEXT, &item);
     * @endcode
     *
     */
    NODE_ACCESSIBILITY_TEXT,

    /**
     * @brief 通过{@link setAttribute}方法设置无障碍重要性。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：障碍重要性，参数类型{@link ArkUI_AccessibilityLevel}。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_ACCESSIBILITY_LEVEL_AUTO } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_ACCESSIBILITY_LEVEL, &item);
     * @endcode
     *
     */
    NODE_ACCESSIBILITY_LEVEL,

    /**
     * @brief 通过{@link setAttribute}方法设置无障碍说明。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .string：无障碍说明。
     * 
     * @code {.c}
     * ArkUI_AttributeItem item = { .string = "test" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_ACCESSIBILITY_DESCRIPTION, &item);
     * @endcode
     *
     */
    NODE_ACCESSIBILITY_DESCRIPTION,

    /**
     * @brief 通过{@link setAttribute}方法设置当前组件是否为当前页面上的默认焦点。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * value[0].i32：参数类型为1或者0。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 1 } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_DEFAULT_FOCUS, &item);
     * @endcode
     *
     */
    NODE_DEFAULT_FOCUS,

    /**
     * @brief 通过{@link setAttribute}方法设置一个或多个触摸热区。
     * 
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .data[0].f32：触摸点相对于组件左上角的x轴坐标,单位为vp。\n
     * .data[1].f32：触摸点相对于组件左上角的y轴坐标,单位为vp。\n
     * .data[2].f32：触摸热区的宽度 ，单位为%。\n
     * .data[3].f32：触摸热区的高度，单位为%。\n
     * .data[4...].f32:可以设置多个手势响应区域，顺序和上述一致。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { 0, 0, 100, 100 };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_RESPONSE_REGION, &item);
     *
     * ArkUI_NumberValue value[] = { 0, 0, 100, 100, 0, 0, 100, 100 };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_RESPONSE_REGION, &item);
     * @endcode
     *
     */
    NODE_RESPONSE_REGION,

    /**
     * @brief 通过{@link setAttribute}方法设置组件的遮罩文本。
     * 
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .string 遮罩文本；\n
     * .value[0]?.i32：可选值，浮层相对于组件的位置，参数类型{@link ArkUI_Alignment}，
     *  默认值为ARKUI_ALIGNMENT_TOP_START。\n
     * .value[1]?.i32：可选值，浮层基于自身左上角的偏移量X，单位为vp。\n
     * .value[2]?.i32：可选值，浮层基于自身左上角的偏移量Y，单位为vp。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_ALIGNMENT_TOP_START }, 1.2, 0.3 };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue), "test"};
     * basicNodeApi->setAttribute(nodeHandle, NODE_OVERLAY, &item);
     * @endcode
     *
     */
    NODE_OVERLAY,

    /**
     * @brief 通过<b>setAttribute</b>为当前text组件设置内容。
     *
     * @note 入参:value: String
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_CONTENT, "一段文本。");
     * @endcode
     */
    NODE_TEXT_CONTENT = MAX_NODE_SCOPE_NUM * ARKUI_NODE_TEXT,
    /**
     * @brief 通过{@link setAttribute}方法设置组件字体颜色，只针对包含文本元素的组件。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：字体颜色数值，0xargb格式，形如 0xFFFF0000 表示红色。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32=0xFFFF0000} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_FONT_COLOR, &item);
     * @endcode
     *
     */
    NODE_FONT_COLOR,
    /**
     * @brief 通过{@link setAttribute}方法设置字体大小。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].f32：字体大小数值，单位为fp。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 10 };
     * ArkUI_AttributeItem item = { value, sizeof(value)/sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_FONT_SIZE, &item);
     * @endcode
     *
     */
    NODE_FONT_SIZE,
    /**
     * @brief 通过{@link setAttribute}方法设置字体样式。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：字体样式{@link ArkUI_FontStyle}，默认值为ARKUI_FONT_STYLE_NORMAL。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = ARKUI_FONT_STYLE_NORMAL} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_FONT_STYLE, &item);
     * @endcode
     *
     */
    NODE_FONT_STYLE,
    /**
     * @brief 通过{@link setAttribute}方法设置字重。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .i32：字体粗细样式{@link ArkUI_FontWeight}，默认值为ARKUI_FONT_WEIGHT_NORMAL。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = ARKUI_FONT_WEIGHT_NORMAL} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_FONT_WEIGHT, &item);
     * @endcode
     *
     */
    NODE_FONT_WEIGHT,
    /**
     * @brief 通过<b>setAttribute</b>为当前组件提供设置文本行高的能力。
     *
     * @note 入参格式为value：number,默认单位vp。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_LINE_HEIGHT, "20");
     * @endcode
     */
    NODE_TEXT_LINE_HEIGHT,
    /**
     * @brief 通过<b>setAttribute</b>为当前组件提供设置文本装饰线样式及其颜色。
     *
     * @note 入参:type：String("none"，"underline"，"overline"，"line_through",
     * "inherit"),color：String
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_DECORATION, "overline #FF1122FF");
     * @endcode
     */
    NODE_TEXT_DECORATION,
    /**
     * @brief 通过<b>setAttribute</b>为当前组件提供设置文本大小写。
     *
     * @note 入参:value：String("normal"，"lowercase"，"uppercase")
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_CASE, "lowercase");
     * @endcode
     */
    NODE_TEXT_CASE,
    /**
     * @brief 通过<b>setAttribute</b>为当前组件提供设置文本字符间距。
     *
     * @note 入参:value：number，默认单位fp
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_LETTER_SPACING, "5");
     * @endcode
     */
    NODE_TEXT_LETTER_SPACING,
    /**
     * @brief 通过<b>setAttribute</b>为当前组件提供设置文本最大行数。
     *
     * @note 入参:value：number
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_MAX_LINES, "1");
     * @endcode
     */
    NODE_TEXT_MAX_LINES,
    /**
     * @brief 通过{@link setAttribute}方法设置字体水平对齐方式。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：水平对齐方式{@link ArkUI_TextAlign}，默认值为ARKUI_TEXT_ALIGN_START。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = ARKUI_TEXT_ALIGN_START} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_ALIGN, &item);
     * @endcode
     *
     */
    NODE_TEXT_ALIGN,
    /**
     * @brief 通过<b>setAttribute</b>为当前组件提供设置文本超长时的显示方式。
     *
     * @note 入参:value: String("none"，"clip"，"ellipsis"，"marquee")
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_OVER_FLOW, "ellipsis");
     * @endcode
     */
    NODE_TEXT_OVER_FLOW,
    /**
     * @brief 通过{@link setAttribute}方法设置字体列表。
     * 
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .string：字体字符串，多个用,分隔。
     * 
     * @code {.c}
     * ArkUI_AttributeItem item = {.string = "HarmonyOS Sans"};
     * basicNodeApi->setAttribute(nodeHandle, NODE_FONT_FAMILY, &item);
     * @endcode
     *
     */
    NODE_FONT_FAMILY,

     /**
     * @brief 通过{@link setAttribute}方法设置文本是否可复制粘贴。
     * 
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：否可复制粘贴，参数类型{@link ArkUI_TextCopyOptions}。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_TEXT_COPY_OPTIONS_NONE } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_COPY_OPTION, &item);
     * @endcode
     *
     */
    NODE_TEXT_COPY_OPTION,
    /**
     * @brief 通过<b>setAttribute</b>方法设置文本基线的偏移量。
     * @note 入参格式为数字类型字符串，如"20"，单位为fp。
     *
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_BASELINE_OFFSET, "20");
     * @endcode
     */
    NODE_TEXT_BASELINE_OFFSET,
    /**
     * @brief 通过<b>setAttribute</b>方法设置文字阴影效果。
     *
     * @note 入参格式为按照阴影模糊半径、阴影类型、阴影的颜色、阴影的X轴偏移量、阴影的Y轴偏移量顺序设置的字符串，\n
     *  如"10 blur #FFFFFF00 10 10"，输入参数不足时，按照指定顺序填充，最多只识别前五个参数；\n
     *  其中阴影类型可选范围为[color，blur]，如"color"。
     *  多组阴影设置参数之间用","作为分割符。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_TEXT_SHADOW, "5 blur #FF008000 5 5");
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_TEXT_SHADOW, "5 blur #FF008000 5 5，10 color #FFFFFF00 10 10");
     * @endcode
     */
    NODE_TEXT_TEXT_SHADOW,

    /**
     * @brief 通过{@link setAttribute}方法设置文本最小显示字号。
     * 
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32：文本最小显示字号，单位FP。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { 20 };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_MIN_FONT_SIZE, &item);
     * @endcode
     *
     */
    NODE_TEXT_MIN_FONT_SIZE,

    /**
     * @brief 通过{@link setAttribute}方法设置文本最大显示字号。
     * 
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32：文本最大显示字号 单位FP。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { 20 };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_MAX_FONT_SIZE, &item);
     * @endcode
     *
     */
    NODE_TEXT_MAX_FONT_SIZE,

    /**
     * @brief 通过{@link setAttribute}方法设置文本样式。包括字体大小、字体粗细、字体族和字体风格。
     * 
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .string?：可选值 字体列表，使用多个字体，使用','进行分割。\n
     * .value[0].f32：文本尺寸 单位FP。\n
     * .value[1]?.i32：可选值，文本的字体粗细，参数类型{@link ArkUI_FontWeight}。默认值为ARKUI_FONT_WEIGHT_NORMAL。\n
     * .value[2]?.i32：可选值，字体样式，参数类型{@link ArkUI_TextFontStyle}。
     *  默认值为ARKUI_TEXT_FONT_STYLE_NORMAL。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { 16, { .i32 = ARKUI_TEXT_FONT_WEIGHT_NORMAL }, { .i32 = ARKUI_TEXT_FONT_STYLE_NORMAL } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_FONT, &item);
     *
     * ArkUI_NumberValue value[] = { 16 };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue), "Arial, HarmonyOS Sans"};
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_FONT, &item);
     * @endcode
     *
     */
    NODE_TEXT_FONT,

    /**
     * @brief 通过{@link setAttribute}方法设置文本自适应高度的方式。
     * 
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：参数类型{@link ArkUI_TextHeightAdaptivePolicy}。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_TEXT_HEIGHT_ADAPTIVE_POLICY_MAX_LINES_FIRST } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_HEIGHT_ADAPTIVE_POLICY, &item);
     * @endcode
     *
     */
    NODE_TEXT_HEIGHT_ADAPTIVE_POLICY,

    /**
     * @brief 通过<b>setAttribute</b>为当前span组件设置文本内容。
     *
     * @note 入参:value: String。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SPAN_CONTENT, "一段文本。");
     * @endcode
     */
    NODE_SPAN_CONTENT = MAX_NODE_SCOPE_NUM * ARKUI_NODE_SPAN,
    /**
     * @brief 通过<b>setAttribute</b>为当前imageSpan组件设置图片地址。
     *
     * @note 入参:value: String。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_IMAGE_SPAN_SRC, "https://www.example.com/xxx.png");
     * @endcode
     */
    NODE_IMAGE_SPAN_SRC = MAX_NODE_SCOPE_NUM * ARKUI_NODE_IMAGE_SPAN,
    /**
     * @brief 通过<b>setAttribute</b>为当前imageSpan组件设置图片基于文本的对齐方式。
     *
     * @note 入参:value: String("top","center","bottom","baseline","none")。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_IMAGE_SPAN_VERTICAL_ALIGN, "center");
     * @endcode
     */
    NODE_IMAGE_SPAN_VERTICAL_ALIGN,
    /**
     * @brief 通过<b>setAttribute</b>为当前image组件设置图片地址。
     *
     * @note 入参:value: String。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_IMAGE_SRC, "https://www.example.com/xxx.png");
     * @endcode
     */
    NODE_IMAGE_SRC = MAX_NODE_SCOPE_NUM * ARKUI_NODE_IMAGE,
    /**
     * @brief 通过<b>setAttribute</b>为当前image组件设置图片填充效果。
     *
     * @note 入参:value: String("fill"，"contain"，"cover"，"fitwidth"，"fitheight"，"none",
     * "scale_down"，"top_left" )。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_IMAGE_OBJECT_FIT, "fill");
     * @endcode
     */
    NODE_IMAGE_OBJECT_FIT,
    /**
     * @brief 通过<b>setAttribute</b>为当前image组件设置图片插值效果效果,缓解图片缩放时的锯齿问题。
     *
     * @note 入参:value: String("none"，"low"，"medium"，"high")。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_IMAGE_INTERPOLATION, "low");
     * @endcode
     */
    NODE_IMAGE_INTERPOLATION,
    /**
     * @brief 通过<b>setAttribute</b>为当前image组件设置图片重复样式。
     *
     * @note 入参:value: String("no_repeat"，"repeat_x"，"repeat_y"，"repeat" )
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_IMAGE_OBJECT_REPEAT, "repeat");
     * @endcode
     */
    NODE_IMAGE_OBJECT_REPEAT,
    /**
     * @brief 通过<b>setAttribute</b>为当前image组件设置图片滤镜效果。
     *
     * @note 入参:colorFilter: String(4*5的转换矩阵)。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_IMAGE_COLOR_FILTER, "1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 1 0");
     * @endcode
     */
    NODE_IMAGE_COLOR_FILTER,

    /**
     * @brief 通过{@link setAttribute}方法设置组件打开状态的背景颜色。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：背景色数值，0xargb格式，形如 0xFFFF0000 表示红色。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32=0xFFFF0000} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TOGGLE_SELECTED_COLOR, &item);
     * @endcode
     *
     */
    NODE_TOGGLE_SELECTED_COLOR = MAX_NODE_SCOPE_NUM * ARKUI_NODE_TOGGLE,
    /**
     * @brief 通过{@link setAttribute}方法设置Switch类型的圆形滑块颜色。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：圆形滑块颜色数值，0xargb格式，形如 0xFFFF0000 表示红色。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32=0xFFFF0000} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TOGGLE_SWITCH_POINT_COLOR, &item);
     * @endcode
     *
     */
    NODE_TOGGLE_SWITCH_POINT_COLOR,

    /**
     * @brief 通过{@link setAttribute}方法设置加载进度条前景色。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .data[0].i32： 进度条前景色，0xargb类型，默认值0x99666666。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 0x99666666 } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_LOADING_PROGRESS_COLOR, &item);
     * @endcode
     *
     */
    NODE_LOADING_PROGRESS_COLOR = MAX_NODE_SCOPE_NUM * ARKUI_NODE_LOADING_PROGRESS,
    /**
     * @brief 通过{@link setAttribute}方法设置LoadingProgress动画显示或者不显示。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32： LoadingProgress动画显示或者不显示，默认值true。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = true } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_LOADING_PROGRESS_ENABLE_LOADING, &item);
     * @endcode
     */
    NODE_LOADING_PROGRESS_ENABLE_LOADING,

    /**
     * @brief 通过{@link setAttribute}方法设置单行文本输入框的默认提示文本内容。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .string：默认提示文本的内容。
     *
     * @code {.c}
     * ArkUI_AttributeItem item = { .string="input" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_PLACEHOLDER, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_PLACEHOLDER = MAX_NODE_SCOPE_NUM * ARKUI_NODE_TEXT_INPUT,
    /**
     * @brief 通过{@link setAttribute}方法设置单行文本输入框的默认文本内容。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .string：默认文本的内容。
     *
     * @code {.c}
     * ArkUI_AttributeItem item = { .string="input" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_TEXT, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_TEXT,
    /**
     * @brief 通过{@link setAttribute}方法设置光标颜色。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：背景色数值，0xargb格式，形如 0xFFFF0000 表示红色。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32=0xFFFF0000} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_CARET_COLOR, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_CARET_COLOR,
    /**
     * @brief 通过{@link setAttribute}方法设置光标风格。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].f32：光标宽度数值，单位为vp
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 100 };
     * ArkUI_AttributeItem item = { value, sizeof(value)/sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_CARET_STYLE, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_CARET_STYLE,
    /**
     * @brief 通过{@link setAttribute}方法设置组件是否展示下划线，设置为true则显示。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * value[0].i32：false表示不展示下划线，true表示展示下划线。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = true} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_SHOW_UNDERLINE, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_SHOW_UNDERLINE,
    /**
     * @brief 通过{@link setAttribute}方法设置输入框支持的最大文本数。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * value[0].i32：最大文本数的数字，无单位。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 50 } };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_MAX_LENGTH, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_MAX_LENGTH,
    /**
     * @brief 通过{@link setAttribute}方法设置回车键类型。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：回车键类型枚举{@link ArkUI_EnterKeyType}，默认值为ARKUI_ENTER_KEY_TYPE_DONE。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = ARKUI_ENTER_KEY_TYPE_DONE} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_ENTER_KEY_TYPE, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_ENTER_KEY_TYPE,
    /**
     * @brief 通过{@link setAttribute}方法设置无输入时默认提示文本的颜色。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：颜色数值，0xargb格式，形如 0xFFFF0000 表示红色。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32=0xFFFF0000} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_PLACEHOLDER_COLOR, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_PLACEHOLDER_COLOR,
    /**
     * @brief 通过{@link setAttribute}方法设置无输入时默认提示文本的字体配置（包括大小、字重、样式、字体列表）。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0]?.f32：可选字体大小数值，默认值16.0，单位为fp；\n
     * .value[1]?.i32：可选字体样式{@link ArkUI_FontStyle}，默认值为ARKUI_FONT_STYLE_NORMAL；\n
     * .value[2]?.i32：可选字体粗细样式{@link ArkUI_FontWeight}，默认值为ARKUI_FONT_WEIGHT_NORMAL；\n
     * ?.string: 字体族内容，多个字体族之间使用逗号分隔，形如“字重；字体族1，字体族2”。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = \n
     * { 16.0, {.i32=ARKUI_FONT_STYLE_NORMAL}, {.i32=ARKUI_FONT_WEIGHT_NORMAL} };
     * ArkUI_AttributeItem item = { value, sizeof(value)/sizeof(ArkUI_NumberValue), "Arial" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_PLACEHOLDER_FONT, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_PLACEHOLDER_FONT,
    /**
     * @brief 通过{@link setAttribute}方法设置当通过点击以外的方式聚焦时是否绑定输入法。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * value[0].i32：false表示聚焦不拉起输入法，true表示拉起。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = true} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_ENABLE_KEYBOARD_ON_FOCUS, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_ENABLE_KEYBOARD_ON_FOCUS,
    /**
     * @brief 通过{@link setAttribute}方法设置输入框的类型。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：输入框类型枚举{@link ArkUI_TextInputType}，默认值为ARKUI_TEXTINPUT_TYPE_NORMAL。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = ARKUI_TEXTINPUT_TYPE_NORMAL} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_TYPE, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_TYPE,
    /**
     * @brief 通过{@link setAttribute}方法设置文本选中时的背景色。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：颜色数值，0xargb格式，形如 0xFFFF0000 表示红色。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32=0xFFFF0000} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_SELECTED_BACKGROUND_COLOR, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_SELECTED_BACKGROUND_COLOR,
    /**
     * @brief 通过{@link setAttribute}方法设置密码输入模式时是否显示末尾图标。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * value[0].i32：false表示不显示图标，true表示显示图标。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = true} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_SHOW_PASSWORD_ICON, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_SHOW_PASSWORD_ICON,
    /**
     * @brief 通过{@link setAttribute}方法用于控制单行文本输入框编辑态。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * value[0].i32：false表示退出编辑态，true表示维持现状。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = false} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_EDITING, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_EDITING,
    /**
     * @brief 通过{@link setAttribute}方法设置单行文本右侧清除按钮样式。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * value[0].i32：按钮样式{@link ArkUI_CancelButtonStyle}，默认值为ARKUI_CANCELBUTTON_STYLE_INPUT；\n
     * value[1]?.f32：图标大小数值，单位为vp；\n
     * value[2]?.i32：按钮图标颜色数值，0xargb格式，形如 0xFFFF0000 表示红色；\n
     * ?.string：按钮图标地址，入参内容为图片本地地址，例如 /pages/icon.png。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32=ARKUI_CANCELBUTTON_STYLE_INPUT}, 10.0, {.i32=0xFFFF0000} };
     * ArkUI_AttributeItem item = { value, sizeof(value)/sizeof(ArkUI_NumberValue), "/pages/icon.png" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_INPUT_CANCEL_BUTTON, &item);
     * @endcode
     *
     */
    NODE_TEXT_INPUT_CANCEL_BUTTON,

    /**
     * @brief 通过{@link setAttribute}方法设置子组件在容器内的对齐方式。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32： 对齐方式，数据类型{@link ArkUI_Alignment}，默认值ARKUI_ALIGNMENT_CENTER。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_ALIGNMENT_CENTER } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_STACK_ALIGN_CONTENT, &item);
     * @endcode
     */
    NODE_STACK_ALIGN_CONTENT = MAX_NODE_SCOPE_NUM * ARKUI_NODE_STACK,

    /**
     * @brief 通过{@link setAttribute}方法设置滚动条状态。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32： 滚动条状态，数据类型{@link ArkUI_ScrollBarDisplayMode}，默认值ARKUI_SCROLL_BAR_DISPLAY_MODE_AUTO。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_SCROLL_BAR_DISPLAY_MODE_AUTO } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCROLL_BAR_DISPLAY_MODE, &item);
     * @endcode
     *
     */
    NODE_SCROLL_BAR_DISPLAY_MODE = MAX_NODE_SCOPE_NUM * ARKUI_NODE_SCROLL,
    /**
     * @brief 通过{@link setAttribute}方法设置滚动条的宽度。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32： 滚动条宽度，单位vp，默认值4。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 20 };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCROLL_BAR_WIDTH, &item);
     * @endcode
     *
     */
    NODE_SCROLL_BAR_WIDTH,
    /**
     * @brief 通过{@link setAttribute}方法设置滚动条的颜色。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .data[0].i32： 滚动条颜色，0xargb类型。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 0xFFFFFFFF } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCROLL_BAR_COLOR, &item);
     * @endcode
     *
     */
    NODE_SCROLL_BAR_COLOR,
    /**
     * @brief 通过{@link setAttribute}方法设置滚动方向。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：滚动方向，数据类型{@link ArkUI_Axis}，默认值ARKUI_AXIS_VERTICAL。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_AXIS_VERTICAL } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCROLL_SCROLL_DIRECTION, &item);
     * @endcode
     *
     */
    NODE_SCROLL_SCROLL_DIRECTION,
    /**
     * @brief 通过{@link setAttribute}方法设置边缘滑动效果。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32： 边缘滑动效果，参数类型{@link ArkUI_EdgeEffect}，默认值ARKUI_EDGE_EFFECT_NONE；\n
     * .value[1]?.i32： 可选值，组件内容大小小于组件自身时，设置是否开启滑动效果，开启为1，关闭为0，默认值1。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_EDGE_EFFECT_NONE }, { .i32 = 1 } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCROLL_EDGE_EFFECT, &item);
     * @endcode
     *
     */
    NODE_SCROLL_EDGE_EFFECT,
    /**
     * @brief
     * 通过{@link setAttribute}方法设置是否支持滚动手势，当设置为false时，无法通过手指或者鼠标滚动，但不影响控制器的滚动接口。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32： 是否支持滚动手势，默认值true。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = true } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCROLL_ENABLE_SCROLL_INTERACTION, &item);
     * @endcode
     *
     */
    NODE_SCROLL_ENABLE_SCROLL_INTERACTION,
    /**
     * @brief
     * 通过{@link setAttribute}方法设置摩擦系数，手动划动滚动区域时生效，只对惯性滚动过程有影响，对惯性滚动过程中的链式效果有间接影响。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32： 摩擦系数，默认值：非可穿戴设备为0.6，可穿戴设备为0.9。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 0.6 };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCROLL_FRICTION, &item);
     * @endcode
     *
     */
    NODE_SCROLL_FRICTION,
    /**
     * @brief 通过{@link setAttribute}方法设置Scroll组件的限位滚动模式。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32： Scroll组件限位滚动时的对其方式，数据类型{@link ArkUI_ScrollSnapAlign}，默认值ARKUI_SCROLL_SNAP_ALIGN_NONE；\n
     * .value[1].i32： 在Scroll组件限位滚动模式下，该属性设置为false后，运行Scroll在开头和第一个限位点间自由滑动。默认值true，仅在限位点为多个时生效；\n
     * .value[2].i32： 在Scroll组件限位滚动模式下，该属性设置为false后，运行Scroll在最后一个限位点和末尾间自由滑动。默认值true，仅在限位点为多个时生效；\n
     * .value[3...].f32： Scroll组件限位滚动时的限位点，限位点即为Scroll组件能滑动停靠的偏移量。可以1个或多个。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = {
     *     { .i32=ARKUI_SCROLL_SNAP_ALIGN_NONE }, { .i32=true }, { .i32=true },
     *     { .f32=0 }, { .f32=500 }, { .f32=1000 }, { .f32=1500 }
     * };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCROLL_SNAP, &item);
     * @endcode
     *
     */
    NODE_SCROLL_SNAP,

    /**
     * @brief 通过{@link
     * setAttribute}方法设置嵌套滚动选项。设置向前向后两个方向上的嵌套滚动模式，实现与父组件的滚动联动。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0]?.i32：可滚动组件往末尾端滚动时的嵌套滚动，参数类型{@link ArkUI_ScrollNestedMode}。\n
     * .value[1]?.i32：可滚动组件往起始端滚动时的嵌套滚动，参数类型{@link ArkUI_ScrollNestedMode}。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_SCROLL_NESTED_MODE_SELF_ONLY },
     * { .i32 = ARKUI_SCROLL_NESTED_OPTIONS_SELF_ONLY } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCROLL_NESTED_SCROLL, &item);
     * @endcode
     *
     */
    NODE_SCROLL_NESTED_SCROLL,
    /**
     * @brief 通过{@link setAttribute}方法设置滑动到指定位置。
     * 
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32：水平滑动偏移，单位为vp。\n
     * .value[1].f32：垂直滑动偏移，单位为vp。\n
     * .value[2]?.i32：可选值，滚动时长，单位为毫秒。\n
     * .value[3]?.i32：可选值，滚动曲线，参数类型{@link ArkUI_Curve}。默认值为ARKUI_CURVE_EASE。\n
     * .value[4]?.i32：可选值，是否使能默认弹簧动效，默认值为0不使能。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { 10, 100, { .i32 = 1000 }, { .i32 = ARKUI_CURVE_EASE },
     * { .i32 = 1 } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCROLL_OFFSET, &item);
     * @endcode
     *
     */
    NODE_SCROLL_OFFSET,

    /**
     * @brief 通过{@link setAttribute}方法设置滚动到容器边缘,
     * 不区分滚动轴方向，Edge.Top和Edge.Start表现相同，Edge.Bottom和Edge.End表现相同。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：容器边缘，参数类型{@link ArkUI_ScrollEdge}。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_SCROLL_EDGE_TOP } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCROLL_EDGE, &item);
     * @endcode
     *
     */
    NODE_SCROLL_EDGE,

    /**
     * @brief 通过{@link setAttribute}方法设置是否支持滑动翻页。
     * @note 如果同时设置了划动翻页 enablePaging 和限位滚动 scrollSnap，则 scrollSnap 优先生效，enablePaging 不生效。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32： 是否支持划动翻页，默认值false。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = true } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_SCROLL_ENABLE_PAGING, &item);
     * @endcode
     *
     */
    NODE_SCROLL_ENABLE_PAGING,

    /**
     * @brief 通过{@link setAttribute}方法设置List组件排列方向。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：List组件排列方向，数据类型{@link ArkUI_Axis}，默认值ARKUI_AXIS_VERTICAL。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_AXIS_VERTICAL } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_LIST_LIST_DIRECTION, &item);
     * @endcode
     *
     */
    NODE_LIST_LIST_DIRECTION = MAX_NODE_SCOPE_NUM * ARKUI_NODE_LIST,
    /**
     * @brief 通过{@link setAttribute}方法配合ListItemGroup组件使用，设置 ListItemGroup 中 header 和 footer 是否要吸顶或吸底。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：配合ListItemGroup组件使用，设置ListItemGroup中header和footer是否要吸顶或吸底。数据类型{@link ArkUI_StickyStyle}，默认值
     *     ARKUI_STICKYSTYLE_NONE。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_STICKYSTYLE_NONE } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_LIST_STICKY, &item);
     * @endcode
     *
     */
    NODE_LIST_STICKY,
    /**
     * @brief 通过{@link setAttribute}方法设置列表项间距。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32： 子组件主轴方向的间隔。默认值0。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 10 };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_LIST_SPACE, &item);
     * @endcode
     *
     */
    NODE_LIST_SPACE,

    /**
     * @brief 通过{@link setAttribute}方法设置是否开启循环。
     *
     * @note 入参格式为内容为true或false的字符串，不区分大小写。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SWIPER_LOOP, "true");
     * @endcode
     *
     */
    NODE_SWIPER_LOOP = MAX_NODE_SCOPE_NUM * ARKUI_NODE_SWIPER,

    /**
     * @brief 通过{@link setAttribute}方法设置子组件是否自动播放。
     *
     * @note 入参格式为内容为true或false的字符串，不区分大小写。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SWIPER_AUTO_PLAY, "false");
     * @endcode
     *
     */
    NODE_SWIPER_AUTO_PLAY,

    /**
     * @brief 通过{@link setAttribute}方法设置可选导航点指示器样式。
     *
     * @note 入参格式为内容为true或false的字符串，不区分大小写。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SWIPER_INDICATOR, true);
     * @endcode
     *
     */
    NODE_SWIPER_SHOW_INDICATOR,

    /**
     * @brief 通过{@link setAttribute}方法设置自动播放时播放的时间间隔。
     *
     * @note 入参格式为数字类型字符串，单位为毫秒。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SWIPER_INTERVAL, "3000");
     * @endcode
     *
     */
    NODE_SWIPER_INTERVAL,

    /**
     * @brief 通过{@link setAttribute}方法设置是否为纵向滑动。
     *
     * @note 入参格式为内容为true或false的字符串，不区分大小写。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SWIPER_VERTICAL, "false");
     * @endcode
     *
     */
    NODE_SWIPER_VERTICAL,

    /**
     * @brief 通过{@link setAttribute}方法设置子组件切换的动画时长。
     *
     * @note 入参格式为数字类型字符串，单位为毫秒。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SWIPER_DURATION, "400");
     * @endcode
     *
     */
    NODE_SWIPER_DURATION,

    /**
     * @brief 通过{@link setAttribute}方法设置Swiper的动画曲线。
     *
     * @note 入参格式为"linear"、"ease"、"easeIn"、"easeOut"、"easeInOut"、
     * "fastOutSlowIn"、"linearOutSlowIn"、"fastOutLinearIn"、"extremeDeceleration"、"sharp"、"rhythm"、"smooth"与"friction"。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SWIPER_CURVE, "linear");
     * @endcode
     *
     */
    NODE_SWIPER_CURVE,

    /**
     * @brief 通过{@link setAttribute}方法设置子组件与子组件之间间隙。
     *
     * @note 入参格式为数字类型字符串，单位为vp。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SWIPER_ITEM_SPACE, "50");
     * @endcode
     *
     */
    NODE_SWIPER_ITEM_SPACE,

    /**
     * @brief 通过{@link setAttribute}方法设置当前在容器中显示的子组件的索引值。
     *
     * @note 入参格式为数字类型字符串。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SWIPER_INDEX, "2");
     * @endcode
     *
     */
    NODE_SWIPER_INDEX,

    /**
     * @brief 通过{@link setAttribute}方法设置一页内元素显示个数。
     *
     * @note 入参格式为数字类型字符串或为"auto"字符串。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SWIPER_DISPLAY_COUNT, "2");
     * @endcode
     *
     */
    NODE_SWIPER_DISPLAY_COUNT,

    /**
     * @brief 通过{@link setAttribute}方法设置禁用组件滑动切换功能。
     *
     * @note 入参格式为内容为true或false的字符串，不区分大小写。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SWIPER_DISABLE_SWIPE, "false");
     * @endcode
     *
     */
    NODE_SWIPER_DISABLE_SWIPE,

    /**
     * @brief 通过{@link setAttribute}方法设置导航点箭头样式。
     *
     * @note 入参格式为"true"、"false"与"showOnHover"。
     * @code {.c}
     * basicNodeApi->setAttribute(nodeHandle, NODE_SWIPER_SHOW_DISPLAY_ARROW, true");
     * @endcode
     *
     */
    NODE_SWIPER_SHOW_DISPLAY_ARROW,

    /**
     * @brief 通过{@link setAttribute}方法设置多行文本输入框的默认提示文本内容。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .string：默认提示文本的内容。
     *
     * @code {.c}
     * ArkUI_AttributeItem item = { .string="multi-input" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_AREA_PLACEHOLDER, &item);
     * @endcode
     *
     */
    NODE_TEXT_AREA_PLACEHOLDER = MAX_NODE_SCOPE_NUM * ARKUI_NODE_TEXT_AREA,
    /**
     * @brief 通过{@link setAttribute}方法设置多行文本输入框的默认文本内容。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .string：默认文本的内容。
     *
     * @code {.c}
     * ArkUI_AttributeItem item = { .string="input" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_AREA_TEXT, &item);
     * @endcode
     *
     */
    NODE_TEXT_AREA_TEXT,
    /**
     * @brief 通过{@link setAttribute}方法设置多行输入框支持的最大文本数。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * value[0].i32：最大文本数的数字，无单位。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = 50} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_AREA_MAX_LENGTH, &item);
     * @endcode
     *
     */
    NODE_TEXT_AREA_MAX_LENGTH,
    /**
     * @brief 通过{@link setAttribute}方法设置无输入时默认提示文本的颜色。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：颜色数值，0xargb格式，形如 0xFFFF0000 表示红色。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32=0xFFFF0000} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_AREA_PLACEHOLDER_COLOR, &item);
     * @endcode
     *
     */
    NODE_TEXT_AREA_PLACEHOLDER_COLOR,
    /**
     * @brief 通过{@link setAttribute}方法设置无输入时默认提示文本的字体配置（包括大小、字重、样式、字体列表）。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0]?.f32：可选字体大小数值，默认值16.0，单位为fp；\n
     * .value[1]?.i32：可选字体样式{@link ArkUI_FontStyle}，默认值为ARKUI_FONT_STYLE_NORMAL；\n
     * .value[2]?.i32：可选字体粗细样式{@link ArkUI_FontWeight}，默认值为ARKUI_FONT_WEIGHT_NORMAL；\n
     * ?.string: 字体族内容，多个字体族之间使用逗号分隔，形如“字重；字体族1，字体族2”。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = \n
     * { 16.0, {.i32=ARKUI_FONT_STYLE_NORMAL}, {.i32=ARKUI_FONT_WEIGHT_NORMAL} };
     * ArkUI_AttributeItem item = { value, sizeof(value)/sizeof(ArkUI_NumberValue), "Arial" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_AREA_PLACEHOLDER_FONT, &item);
     * @endcode
     *
     */
    NODE_TEXT_AREA_PLACEHOLDER_FONT,
    /**
     * @brief 通过{@link setAttribute}方法设置光标颜色。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：背景色数值，0xargb格式，形如 0xFFFF0000 表示红色。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32=0xFFFF0000} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_AREA_CARET_COLOR, &item);
     * @endcode
     *
     */
    NODE_TEXT_AREA_CARET_COLOR,
    /**
     * @brief 通过{@link setAttribute}方法用于控制多行文本输入框编辑态。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * value[0].i32：false表示退出编辑态，true表示维持现状。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = false} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_AREA_EDITING, &item);
     * @endcode
     *
     */
    NODE_TEXT_AREA_EDITING,

    /**
     * @brief 通过{@link setAttribute}方法设置button按钮的文本内容。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .string：默认文本的内容。
     *
     * @code {.c}
     * ArkUI_AttributeItem item = { .string="click" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_BUTTON_LABEL, &item);
     * @endcode
     *
     */
    NODE_BUTTON_LABEL = MAX_NODE_SCOPE_NUM * ARKUI_NODE_BUTTON,

    /**
     * @brief 通过{@link setAttribute}方法设置进度条的当前进度值。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * value[0].f32：进度条当前值。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 10 };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_PROGRESS_VALUE, &item);
     * @endcode
     *
     */
    NODE_PROGRESS_VALUE = MAX_NODE_SCOPE_NUM * ARKUI_NODE_PROGRESS,
    /**
     * @brief 通过{@link setAttribute}方法设置进度条的总长。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * value[0].f32：进度条总长。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 100 };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_PROGRESS_TOTAL, &item);
     * @endcode
     *
     */
    NODE_PROGRESS_TOTAL,
    /**
     * @brief 通过{@link setAttribute}方法设置进度条显示进度值的颜色。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：颜色数值，0xargb格式，形如 0xFFFF0000 表示红色。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32=0xFFFF0000} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_PROGRESS_COLOR, &item);
     * @endcode
     *
     */
    NODE_PROGRESS_COLOR,
    /**
     * @brief 通过{@link setAttribute}方法设置进度条的类型。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：进度条类型枚举值{@link ArkUI_ProgressType}，默认值为ARKUI_PROGRESS_LINEAR。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32=ARKUI_PROGRESS_LINEAR} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_PROGRESS_TYPE, &item);
     * @endcode
     */
    NODE_PROGRESS_TYPE,

    /**
     * @brief 通过{@link setAttribute}方法设置XComponent组件ID。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * string: ID的内容。
     *
     * @code {.c}
     * ArkUI_AttributeItem item = { .string = "test" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_XCOMPONENT_ID, &item);
     * @endcode
     *
     */
    NODE_XCOMPONENT_ID = MAX_NODE_SCOPE_NUM * ARKUI_NODE_XCOMPONENT,
    /**
     * @brief 通过{@link setAttribute}方法设置XComponent的类型。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：字体样式{@link ArkUI_XComponentType}，默认值为ARKUI_XCOMPONENT_TYPE_SURFACE。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = ARKUI_XCOMPONENT_TYPE_SURFACE} };
     * ArkUI_AttributeItem item = { value, 1 };
     * basicNodeApi->setAttribute(nodeHandle, NODE_XCOMPONENT_TYPE, &item);
     * @endcode
     *
     */
    NODE_XCOMPONENT_TYPE,
    /**
     * @brief 通过{@link setAttribute}方法设置XComponent的宽高。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].f32：宽数值，单位为vp；\n
     * .value[1].f32：高数值，单位为vp。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { 300, 50 };
     * ArkUI_AttributeItem item = { value, sizeof(value)/sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_XCOMPONENT_SURFACE_SIZE, &item);
     * @endcode
     *
     */
    NODE_XCOMPONENT_SURFACE_SIZE,

    /**
     * @brief 通过{@link setAttribute}方法设置 ListItemGroup 头部组件。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .object：使用{@link ArkUI_NodeHandle}对象作为ListItemGroup头部组件。
     *
     * @code {.c}
     * auto header = nodeAPI->createNode(ARKUI_NODE_TEXT);
     * ARKUI_AttributeItem item = { .object = header };
     * basicNodeApi->setAttribute(nodeHandle, NODE_LIST_ITEM_GROUP_SET_HEADER, &item);
     * @endcode
     */
    NODE_LIST_ITEM_GROUP_SET_HEADER = MAX_NODE_SCOPE_NUM * ARKUI_NODE_LIST_ITEM_GROUP,
    /**
     * @brief 通过{@link setAttribute}方法设置 ListItemGroup 尾部组件。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .object：使用{@link ArkUI_NodeHandle}对象作为ListItemGroup尾部组件。
     *
     * @code {.c}
     * auto footer = nodeAPI->createNode(ARKUI_NODE_TEXT);
     * ARKUI_AttributeItem item = { .object = footer };
     * basicNodeApi->setAttribute(nodeHandle, NODE_LIST_ITEM_GROUP_SET_FOOTER, &item);
     * @endcode
     */
    NODE_LIST_ITEM_GROUP_SET_FOOTER,
    /**
     * @brief 通过{@link setAttribute}方法设置ListItem分割线样式，默认无分割线。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32： 颜色，0xargb类型；\n
     * .value[0].f32： 分割线宽，单位vp；\n
     * .value[1].f32： 分割线距离列表侧边起始端的距离，单位vp；\n
     * .value[2].f32： 分割线距离列表侧边结束端的距离，单位vp。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 0xFFFFFFFF }, 1, 0, 0 };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_XCOMPONENT_SURFACE_SIZE, &item);
     * @endcode
     */
    NODE_LIST_ITEM_GROUP_SET_DIVIDER,
    /**
     * @brief 通过{@link setAttribute}方法设置日期选择器组件的日期是否显示农历。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32： 是否显示农历，默认值false。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = true } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_DATE_PICKER_LUNAR, &item);
     * @endcode
     */
    NODE_DATE_PICKER_LUNAR = MAX_NODE_SCOPE_NUM * ARKUI_NODE_DATE_PICKER,
    /**
     * @brief 通过{@link setAttribute}方法设置日期选择器组件选择器的起始日期。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 日期，默认值"1970-1-1"。
     *
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "1970-1-1" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_DATE_PICKER_START, &item);
     * @endcode
     */
    NODE_DATE_PICKER_START,
    /**
     * @brief 通过{@link setAttribute}方法设置日期选择器组件选择器的结束日期。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 日期，默认值"2100-12-31"。
     *
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "2100-12-31" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_DATE_PICKER_END, &item);
     * @endcode
     */
    NODE_DATE_PICKER_END,
    /**
     * @brief 通过{@link setAttribute}方法设置日期选择器组件选中项的日期。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 日期，默认值"2024-01-22"。
     *
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "2024-01-22" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_DATE_PICKER_SELECTED, &item);
     * @endcode
     */
    NODE_DATE_PICKER_SELECTED,
    /**
     * @brief 通过{@link setAttribute}方法设置日期选择器组件的所有选项中最上和最下两个选项的文本颜色、字号、字体粗细。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 入参5个，格式为字符串，以 ';' 分割：\n
     *       入参1： 文本颜色，#argb类型\n
     *       入参2： 文本大小，数字类型，单位fp\n
     *       入参3： 文本粗细，字符串枚举("bold", "normal", "bolder", "lighter", "medium", "regular")\n
     *       入参4： 文本字体列表，使用 ',' 进行分割\n
     *       入参5： 文本样式，字符串枚举("normal", "italic")\n
     *       如 "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" 。
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_DATE_PICKER_DISAPPEAR_TEXT_STYLE, &item);
     * @endcode
     */
    NODE_DATE_PICKER_DISAPPEAR_TEXT_STYLE,
    /**
     * @brief 通过{@link setAttribute}方法设置日期选择器组件的所有选项中除了最上、最下及选中项以外的文本颜色、字号、字体粗细。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 入参5个，格式为字符串，以 ';' 分割：\n
     *       入参1： 文本颜色，#argb类型\n
     *       入参2： 文本大小，数字类型，单位fp\n
     *       入参3： 文本粗细，字符串枚举("bold", "normal", "bolder", "lighter", "medium", "regular")\n
     *       入参4： 文本字体列表，使用 ',' 进行分割\n
     *       入参5： 文本样式，字符串枚举("normal", "italic")\n
     *       如 "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" 。
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_DATE_PICKER_TEXT_STYLE, &item);
     * @endcode
     */
    NODE_DATE_PICKER_TEXT_STYLE,
    /**
     * @brief 通过{@link setAttribute}方法设置日期选择器组件的选中项的文本颜色、字号、字体粗细。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 入参5个，格式为字符串，以 ';' 分割：\n
     *       入参1： 文本颜色，#argb类型\n
     *       入参2： 文本大小，数字类型，单位fp\n
     *       入参3： 文本粗细，字符串枚举("bold", "normal", "bolder", "lighter", "medium", "regular")\n
     *       入参4： 文本字体列表，使用 ',' 进行分割\n
     *       入参5： 文本样式，字符串枚举("normal", "italic")\n
     *       如 "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" 。
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_DATE_PICKER_SELECTED_TEXT_STYLE, &item);
     * @endcode
     */
    NODE_DATE_PICKER_SELECTED_TEXT_STYLE,
    /**
     * @brief 通过{@link setAttribute}方法设置时间选择组件选中项的时间。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 时间，默认值当前系统时间。
     *
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "17-11" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TIME_PICKER_SELECTED, &item);
     * @endcode
     */

    NODE_TIME_PICKER_SELECTED = MAX_NODE_SCOPE_NUM * ARKUI_NODE_TIME_PICKER,
    /**
     * @brief 通过{@link setAttribute}方法设置时间选择组件展示时间是否为24小时制。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32： 是否为24小时制，默认值false。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = true } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TIME_PICKER_USE_MILITARY_TIME, &item);
     * @endcode
     */
    NODE_TIME_PICKER_USE_MILITARY_TIME,
    /**
     * @brief 通过{@link setAttribute}方法设置时间选择组件所有选项中最上和最下两个选项的文本颜色、字号、字体粗细。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 入参5个，格式为字符串，以 ';' 分割：\n
     *       入参1： 文本颜色，#argb类型\n
     *       入参2： 文本大小，数字类型，单位fp\n
     *       入参3： 文本粗细，字符串枚举("bold", "normal", "bolder", "lighter", "medium", "regular")\n
     *       入参4： 文本字体列表，使用 ',' 进行分割\n
     *       入参5： 文本样式，字符串枚举("normal", "italic")\n
     *       如 "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" 。
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_DATE_PICKER_DISAPPEAR_TEXT_STYLE, &item);
     * @endcode
     */
    NODE_TIME_PICKER_DISAPPEAR_TEXT_STYLE,
    /**
     * @brief 通过{@link setAttribute}方法设置时间选择组件所有选项中除了最上、最下及选中项以外的文本颜色、字号、字体粗细。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 入参5个，格式为字符串，以 ';' 分割：\n
     *       入参1： 文本颜色，#argb类型\n
     *       入参2： 文本大小，数字类型，单位fp\n
     *       入参3： 文本粗细，字符串枚举("bold", "normal", "bolder", "lighter", "medium", "regular")\n
     *       入参4： 文本字体列表，使用 ',' 进行分割\n
     *       入参5： 文本样式，字符串枚举("normal", "italic")\n
     *       如 "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" 。
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TIME_PICKER_TEXT_STYLE, &item);
     * @endcode
     */
    NODE_TIME_PICKER_TEXT_STYLE,
    /**
     * @brief 通过{@link setAttribute}方法设置时间选择组件选中项的文本颜色、字号、字体粗细。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 入参5个，格式为字符串，以 ';' 分割：\n
     *       入参1： 文本颜色，#argb类型\n
     *       入参2： 文本大小，数字类型，单位fp\n
     *       入参3： 文本粗细，字符串枚举("bold", "normal", "bolder", "lighter", "medium", "regular")\n
     *       入参4： 文本字体列表，使用 ',' 进行分割\n
     *       入参5： 文本样式，字符串枚举("normal", "italic")\n
     *       如 "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" 。
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TIME_PICKER_SELECTED_TEXT_STYLE, &item);
     * @endcode
     */
    NODE_TIME_PICKER_SELECTED_TEXT_STYLE,

    /**
     * @brief 通过{@link setAttribute}方法设置滑动选择文本选择器的选择列表。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].i32：使用的选择器类型{@link ArkUI_TextPickerRangeType}，默认值为ARKUI_TEXTPICKER_RANGETYPE_SINGLE；\n
     * ?.string：针对不同选择器类型有如下输入范式：\n
     * 1：单列选择器，入参格式为用分号分隔的一组字符串；\n
     * 2：多列选择器，支持多对纯文本字符串对，多对之间使用分号分隔，每对内部使用逗号分隔；\n
     * ?.object：针对不同选择器类型有如下输入范式：\n
     * 1：单列支持图片的选择器，输入结构体为{@link ARKUI_TextPickerRangeContent}；\n
     * 2：多列联动选择器，输入结构体为{@link ARKUI_TextPickerCascadeRangeContent}。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32=ARKUI_TEXTPICKER_RANGETYPE_MULTI} };
     * ArkUI_AttributeItem item = { value, sizeof(value)/sizeof(ArkUI_NumberValue), "1,2,3;A,B,C" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_PICKER_OPTION_RANGE, &item);
     * @endcode
     *
     */
    NODE_TEXT_PICKER_OPTION_RANGE = MAX_NODE_SCOPE_NUM * ARKUI_NODE_TEXT_PICKER,
    /**
     * @brief 通过{@link setAttribute}方法设置滑动选择文本内容的组件默认选中项在数组中的索引值。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .value[0].u32：索引值，如存在多个索引值则逐个添加。
     *
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.u32 = 1}, {.u32 = 2} };
     * ArkUI_AttributeItem item = { value, sizeof(value)/sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_PICKER_OPTION_SELECTED, &item);
     * @endcode
     *
     */
    NODE_TEXT_PICKER_OPTION_SELECTED,
    /**
     * @brief 通过{@link setAttribute}方法设置滑动选择文本内容的组件默认选中项的值，优先级低于 NODE_TEXT_PICKER_OPTION_SELECTED。
     *
     * {@link ArkUI_AttributeItem}入参格式：\n
     * .string：选中项的值，如存在多个值则逐个添加，用分号分隔。
     *
     * @code {.c}
     * ArkUI_AttributeItem item = { .string = "A;B" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_PICKER_OPTION_VALUE, &item);
     * @endcode
     *
     */
    NODE_TEXT_PICKER_OPTION_VALUE,
    /**
     * @brief 通过{@link setAttribute}方法设置滑动选择文本内容的组件所有选项中最上和最下两个选项的文本颜色、字号、字体粗细。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 入参5个，格式为字符串，以 ';' 分割：\n
     *       入参1： 文本颜色，#argb类型\n
     *       入参2： 文本大小，数字类型，单位fp\n
     *       入参3： 文本粗细，字符串枚举("bold", "normal", "bolder", "lighter", "medium", "regular")\n
     *       入参4： 文本字体列表，使用 ',' 进行分割\n
     *       入参5： 文本样式，字符串枚举("normal", "italic")\n
     *       如 "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" 。
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_PICKER_DISAPPEAR_TEXT_STYLE, &item);
     * @endcode
     */
    NODE_TEXT_PICKER_DISAPPEAR_TEXT_STYLE,
    /**
     * @brief 通过{@link setAttribute}方法设置滑动选择文本内容的组件所有选项中除了最上、最下及选中项以外的文本颜色、字号、字体粗细。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 入参5个，格式为字符串，以 ';' 分割：\n
     *       入参1： 文本颜色，#argb类型\n
     *       入参2： 文本大小，数字类型，单位fp\n
     *       入参3： 文本粗细，字符串枚举("bold", "normal", "bolder", "lighter", "medium", "regular")\n
     *       入参4： 文本字体列表，使用 ',' 进行分割\n
     *       入参5： 文本样式，字符串枚举("normal", "italic")\n
     *       如 "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" 。
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_PICKER_TEXT_STYLE, &item);
     * @endcode
     */
    NODE_TEXT_PICKER_TEXT_STYLE,
    /**
     * @brief 通过{@link setAttribute}方法设置滑动选择文本内容的组件选中项的文本颜色、字号、字体粗细。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .string： 入参5个，格式为字符串，以 ';' 分割：\n
     *       入参1： 文本颜色，#argb类型\n
     *       入参2： 文本大小，数字类型，单位fp\n
     *       入参3： 文本粗细，字符串枚举("bold", "normal", "bolder", "lighter", "medium", "regular")\n
     *       入参4： 文本字体列表，使用 ',' 进行分割\n
     *       入参5： 文本样式，字符串枚举("normal", "italic")\n
     *       如 "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" 。
     * @code {.c}
     * ARKUI_AttributeItem item = { .string = "#ff182431;14;normal;Arial,HarmonyOS Sans;normal" };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_PICKER_SELECTED_TEXT_STYLE, &item);
     * @endcode
     */
    NODE_TEXT_PICKER_SELECTED_TEXT_STYLE,
    /**
     * @brief 通过{@link setAttribute}方法设置滑动选择文本内容的组件默认选中项在数组中的索引值，优先级高于 options 中的选中值。
     *
     * {@link ArkUI_AttributeItem}参数类型：\n
     * .value[0...].i32：默认选中项在数组中的索引值数组，优先级高于options中的选中值。
     *
     * @note 入参索引列表，数字类型数组，格式字符串，以空格分割，如 "0 1" 。
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 0 }, { .i32 = 1 } };
     * ARKUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
     * basicNodeApi->setAttribute(nodeHandle, NODE_TEXT_PICKER_SELECTED_INDEX, &item);
     * @endcode
     */
    NODE_TEXT_PICKER_SELECTED_INDEX,

    /**
     * @brief 通过{@link setAttribute}方法设置滑块的颜色。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：滑块的颜色, 类型为0xargb，如0xFF1122FF。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 0xFF1122FF } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_BLOCK_COLOR, &item);
     * @endcode
     *
     */
    NODE_SLIDER_BLOCK_COLOR = MAX_NODE_SCOPE_NUM * ARKUI_NODE_SLIDER,

    /**
     * @brief 通过{@link setAttribute}方法设置滑轨的背景颜色。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：背景颜色, 类型为0xargb，如0xFF1122FF。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 0xFF1122FF } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_TRACK_COLOR, &item);
     * @endcode
     *
     */
    NODE_SLIDER_TRACK_COLOR,

    /**
     * @brief 通过{@link setAttribute}方法设置滑轨的已滑动部分颜色。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：已滑动部分颜色, 类型为0xargb，如0xFF1122FF。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 0xFF1122FF } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_SELECTED_COLOR, &item);
     * @endcode
     *
     */
    NODE_SLIDER_SELECTED_COLOR,

    /**
     * @brief 通过{@link setAttribute}方法设置滑动时是否显示气泡提示。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：是否显示气泡，1表示显示，0表示不显示，默认值为0。
     * .string? 可选值，气泡提示的文本内容，默认显示当前百分比字符串。
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 1 } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue), "test"};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_SHOW_TIPS, &item);
     * @endcode
     *
     */
    NODE_SLIDER_SHOW_TIPS,

    /**
     * @brief 通过{@link setAttribute}方法设置滑块形状参数。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：形状参数。参数类型{@link ArkUI_SliderBlockStyle}。
     * .string? 可选值，根据形状参数而定。
     * ARKUI_SLIDER_BLOCK_STYLE_IMAGE: 滑块图片资源。如/pages/common/icon.png。
     * ARKUI_SLIDER_BLOCK_STYLE_SHAPE: 滑块使用的自定义形状。
     * 如"rect(10,10,10,10)"括号内分别为width、height、radiusWidth与radiusHeight";
     *    "circle(10,10)"括号内分别为width、height;
     *    "ellipse(10,10)"括号内分别为width、height;
     *    "path(10,10,M0 0 L600 0)"括号内分别为width、height、commands。
     *  
     * @code {.c}
     * ArkUI_NumberValue value[] = {{.i32 = ARKUI_SLIDER_BLOCK_STYLE_DEFAULT}};
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_BLOCK_STYLE, &item);
     *
     * ArkUI_NumberValue value[] = {{.i32 = ARKUI_SLIDER_BLOCK_STYLE_IMAGE}};
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue), "/pages/common/icon.png"};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_BLOCK_STYLE, &item);
     *
     * ArkUI_NumberValue value[] = {{.i32 = ARKUI_SLIDER_BLOCK_STYLE_SHAPE}};
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue), "rect(10,10,10,10)"};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_BLOCK_STYLE, &item);
     * @endcode
     *
     */
    NODE_SLIDER_BLOCK_STYLE,

    /**
     * @brief 通过{@link setAttribute}方法设置slider进度值。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32：进度值。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { 0 };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_VALUE, &item);
     * @endcode
     *
     */
    NODE_SLIDER_VALUE,

    /**
     * @brief 通过{@link setAttribute}方法设置slider最小值。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32：进度值的最小值。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { 0 };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_MIN_VALUE, &item);
     * @endcode
     *
     */
    NODE_SLIDER_MIN_VALUE,

    /**
     * @brief 通过{@link setAttribute}方法设置slider最大值。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32：进度值的最大值。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { 100 };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_MAX_VALUE, &item);
     * @endcode
     *
     */
    NODE_SLIDER_MAX_VALUE,

    /**
     * @brief 通过{@link setAttribute}方法设置Slider滑动步长。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].f32：滑动步长，取值范围：[0.01, 100]。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { 100 };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_STEP, &item);
     * @endcode
     *
     */
    NODE_SLIDER_STEP,

    /**
     * @brief 通过{@link setAttribute}方法设置滑动条滑动方向为水平或竖直方向。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：显示样式，参数类型{@link ArkUI_SliderDirection}，默认值为ARKUI_SLIDER_DIRECTION_VERTICAL。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_SLIDER_DIRECTION_VERTICAL } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_DIRECTION, &item);
     * @endcode
     *
     */
    NODE_SLIDER_DIRECTION,

    /**
     * @brief 通过{@link setAttribute}方法设置滑动条取值范围是否反向，横向Slider默认为从左往右滑动，
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：是否反向，1表示反向，0表示不反向。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { {.i32 = 0} };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_REVERSE, &item);
     * @endcode
     *
     */
    NODE_SLIDER_REVERSE,

    /**
     * @brief 通过{@link setAttribute}方法设置Slider的滑块与滑轨显示样式。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：显示样式，参数类型{@link ArkUI_SliderStyle}，默认值为ARKUI_SLIDER_STYLE_OUT_SET。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ARKUI_SLIDER_STYLE_OUT_SET } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_SLIDER_STYLE, &item);
     * @endcode
     *
     */
    NODE_SLIDER_STYLE,

    /**
     * @brief 通过{@link setAttribute}方法设置多选框是否选中。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：1表示选中，0表示不选中。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 0 } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_CHECKBOX_SELECT, &item);
     * @endcode
     *
     */
    NODE_CHECKBOX_SELECT = MAX_NODE_SCOPE_NUM * ARKUI_NODE_CHECKBOX,

    /**
     * @brief 通过{@link setAttribute}方法设置多选框选中状态颜色。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：多选框选中状态颜色, 类型为0xargb，如0xFF1122FF。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 0xFF1122FF } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_CHECKBOX_SELECT_COLOR, &item);
     * @endcode
     *
     */
    NODE_CHECKBOX_SELECT_COLOR,

    /**
     * @brief 通过{@link setAttribute}方法设置多选框非选中状态边框颜色。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：边框颜色, 类型为0xargb，如0xFF1122FF
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 0xFF1122FF } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_CHECKBOX_UNSELECT_COLOR, &item);
     * @endcode
     *
     */
    NODE_CHECKBOX_UNSELECT_COLOR,

    /**
     * @brief 通过{@link setAttribute}方法设置多选框内部图标样式。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：边框颜色, 类型为0xargb，如0xFF1122FF
     * .value[1]?.f32：可选，内部图标大小，单位vp。
     * .value[2]?.f32：可选，内部图标粗细，单位vp，默认值2。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = 0xFF1122FF }, 20.0f, 2.0f };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_CHECKBOX_MARK, &item);
     * @endcode
     *
     */
    NODE_CHECKBOX_MARK,

    /**
     * @brief 通过{@link setAttribute}方法设置CheckBox组件形状, 包括圆形和圆角方形。
     *
     *{@link ArkUI_AttributeItem}参数类型：\n
     * .value[0].i32：组件形状，参数类型{@link ArkUI_CheckboxShape}。
     * 
     * @code {.c}
     * ArkUI_NumberValue value[] = { { .i32 = ArkUI_CHECKBOX_SHAPE_CIRCLE } };
     * ArkUI_AttributeItem item = {value, sizeof(value)/sizeof(ArkUI_NumberValue)};
     * basicNodeApi->setAttribute(nodeHandle, NODE_CHECKBOX_SHAPE, &item);
     * @endcode
     *
     */
    NODE_CHECKBOX_SHAPE,

} ArkUI_NodeAttributeType;

#define MAX_COMPONENT_EVENT_ARG_NUM 12
/**
 * @brief 定义组件回调事件的参数类型。
 *
 * @since 12
 */
typedef struct {
    /** 数据数组对象。*/
    ArkUI_NumberValue data[MAX_COMPONENT_EVENT_ARG_NUM];
} ArkUI_NodeComponentEvent;

/**
 * @brief 定义组件回调事件使用字符串参数的类型。
 *
 * @since 12
 */
typedef struct {
    /** 字符串数据。*/
    const char* pStr;
} ArkUI_StringAsyncEvent;

/**
 * @brief 提供NativeNode组件支持的事件类型定义。
 *
 * @since 12
 */
typedef enum {
    /**
     * @brief 手势事件类型。
     *
     * 事件回调发生时，事件参数{@link ArkUI_NodeEvent}对象中的联合体类型为{@link ArkUI_TouchEvent}。
     */
    NODE_TOUCH_EVENT = 0,

    /**
     * @brief 挂载事件。
     *
     * 触发该事件的条件 ：组件挂载显示时触发此回调。\n
     * 事件回调发生时，事件参数{@link ArkUI_NodeEvent}对象中的联合体类型为{@link ArkUI_NodeComponentEvent}。\n
     * {@link ArkUI_NodeComponentEvent}中不包含参数。
     */
    NODE_EVENT_ON_APPEAR,

    /**
     * @brief 组件区域变化事件
     *
     * 触发该事件的条件 ：组件区域变化时触发该回调。\n
     * 事件回调发生时，事件参数{@link ArkUI_NodeEvent}对象中的联合体类型为{@link ArkUI_NodeComponentEvent}。\n
     * @note <b>::ArkUI_NodeComponentEvent</b>中包含12个参数\n
     * <b>ArkUI_NodeComponent.data[0].f32</b>表示 old Area 目标元素的宽度，类型为number，单位vp。\n
     * <b>ArkUI_NodeComponent.data[1].f32</b>表示 old Area 目标元素的高度，类型为number，单位vp。\n
     * <b>ArkUI_NodeComponent.data[2].f32</b>表示 old Area
     * 目标元素左上角相对父元素左上角的位置的x轴坐标，类型为number，单位vp。\n
     * <b>ArkUI_NodeComponent.data[3].f32</b>表示 old Area
     * 目标元素左上角相对父元素左上角的位置的y轴坐标，类型为number，单位vp。\n
     * <b>ArkUI_NodeComponent.data[4].f32</b>表示 old Area
     * 目标元素目标元素左上角相对页面左上角的位置的x轴坐标，类型为number，单位vp。\n
     * <b>ArkUI_NodeComponent.data[5].f32</b>表示 old Area
     * 目标元素目标元素左上角相对页面左上角的位置的y轴坐标，类型为number，单位vp。\n
     * <b>ArkUI_NodeComponent.data[6].f32</b>表示 new Area 目标元素的宽度，类型为number，单位vp。\n
     * <b>ArkUI_NodeComponent.data[7].f32</b>表示 new Area 目标元素的高度，类型为number，单位vp。\n
     * <b>ArkUI_NodeComponent.data[8].f32</b>表示 new Area 目标元素左上角相对父元素左上角的位置的x轴坐标，类型为number，单位vp。\n
     * <b>ArkUI_NodeComponent.data[9].f32</b>表示 new Area 目标元素左上角相对父元素左上角的位置的y轴坐标，类型为number，单位vp。\n
     * <b>ArkUI_NodeComponent.data[10].f32</b>表示 new Area 目标元素目标元素左上角相对页面左上角的位置的x轴坐标，类型为number，单位vp。\n
     * <b>ArkUI_NodeComponent.data[11].f32</b>表示 new Area 目标元素目标元素左上角相对页面左上角的位置的y轴坐标，类型为number，单位vp。\n
     */
    NODE_EVENT_ON_ON_AREA_CHANGE,
    NODE_ON_FOCUS,
    NODE_ON_BLUR,

    NODE_TOGGLE_ON_CHANGE = MAX_NODE_SCOPE_NUM * ARKUI_NODE_TOGGLE,

    NODE_TEXT_INPUT_ON_CHANGE = MAX_NODE_SCOPE_NUM * ARKUI_NODE_TEXT_INPUT,
    NODE_TEXT_INPUT_ON_SUBMIT,

    /**
     * @brief 定义ARKUI_NODE_SCROLL滚动组件的滚动事件枚举值。
     *
     * 触发该事件的条件 ：\n
     * 1、滚动组件触发滚动时触发，支持键鼠操作等其他触发滚动的输入设置。\n
     * 2、通过滚动控制器API接口调用。\n
     * 3、越界回弹。\n
     * 事件回调发生时，事件参数{@link ArkUI_NodeEvent}对象中的联合体类型为{@link ArkUI_NodeComponentEvent}。\n
     * <b>::ArkUI_NodeComponentEvent</b>中包含2个参数\n
     * <b>ArkUI_NodeComponent.data[0].f32</b>表示距离上一次事件触发的X轴增量。\n
     * <b>ArkUI_NodeComponent.data[1].f32</b>表示距离上一次事件触发的Y轴增量。\n
     */
    NODE_SCROLL_EVENT_ON_SCROLL = MAX_NODE_SCOPE_NUM * ARKUI_NODE_SCROLL,
    /**
     * @brief 定义ARKUI_NODE_SCROLL滚动组件的滚动帧始事件枚举值
     *
     * 触发该事件的条件 ：\n
     * 1、滚动组件触发滚动时触发，包括键鼠操作等其他触发滚动的输入设置。\n
     * 2、调用控制器接口时不触发。\n
     * 3、越界回弹不触发。\n
     * 事件回调发生时，事件参数{@link ArkUI_NodeEvent}对象中的联合体类型为{@link ArkUI_NodeComponentEvent}。\n
     * <b>::ArkUI_NodeComponentEvent</b>中包含2个参数\n
     * <b>ArkUI_NodeComponent.data[0].f32</b>表示即将发生的滚动量。\n
     * <b>ArkUI_NodeComponent.data[1].i32</b>表示当前滚动状态。\n
     * <b>::ArkUI_NodeComponentEvent</b>中包含1个返回值\n
     * <b>ArkUI_NodeComponent.data[0].f32</b>事件处理函数中可根据应用场景计算实际需要的滚动量并存于data[0].f32中，Scroll将按照返回值的实际滚动量进行滚动\n
     */
    NODE_SCROLL_EVENT_ON_SCROLL_FRAME_BEGIN,
    /**
     * @brief 定义ARKUI_NODE_SCROLL滚动组件的滚动开始事件枚举值。
     *
     * 触发该事件的条件 ：\n
     * 1、滚动组件开始滚动时触发，支持键鼠操作等其他触发滚动的输入设置。\n
     * 2、通过滚动控制器API接口调用后开始，带过渡动效。\n
     * 事件回调发生时，事件参数{@link ArkUI_NodeEvent}对象中的联合体类型为{@link ArkUI_NodeComponentEvent}。\n
     * <b>::ArkUI_NodeComponentEvent</b>中不包含参数。\n
     */
    NODE_SCROLL_EVENT_ON_SCROLL_START,
    /**
     * @brief 定义ARKUI_NODE_SCROLL滚动组件的滚动停止事件枚举值。
     *
     * 触发该事件的条件 ：\n
     * 1、滚动组件触发滚动后停止，支持键鼠操作等其他触发滚动的输入设置。\n
     * 2、通过滚动控制器API接口调用后停止，带过渡动效。\n
     * 事件回调发生时，事件参数{@link ArkUI_NodeEvent}对象中的联合体类型为{@link ArkUI_NodeComponentEvent}。\n
     * <b>::ArkUI_NodeComponentEvent</b>中不包含参数。\n
     */
    NODE_SCROLL_EVENT_ON_SCROLL_STOP,
    /**
     * @brief 定义ARKUI_NODE_SCROLL滚动组件的滚动边缘事件枚举值。
     *
     * 触发该事件的条件 ：\n
     * 1、滚动组件滚动到边缘时触发，支持键鼠操作等其他触发滚动的输入设置。\n
     * 2、通过滚动控制器API接口调用。\n
     * 3、越界回弹。\n
     * 事件回调发生时，事件参数{@link ArkUI_NodeEvent}对象中的联合体类型为{@link ArkUI_NodeComponentEvent}。\n
     * <b>::ArkUI_NodeComponentEvent</b>中包含1个参数。\n
     * <b>ArkUI_NodeComponent.data[0].i32</b>表示当前碰到的是上下左右哪个边。\n
     */
    NODE_SCROLL_EVENT_ON_SCROLL_EDGE,

    NODE_TEXT_AREA_ON_CHANGE = MAX_NODE_SCOPE_NUM * ARKUI_NODE_TEXT_AREA,

    NODE_REFRESH_STATE_CHANGE = 1000 * ARKUI_NODE_REFRESH + 1,
    NODE_REFRESH_ON_REFRESH,

    /**
     * @brief 定义ARKUI_NODE_DATE_PICKER 列表组件的滚动触摸事件枚举值。
     *
     * 触发该事件的条件 ：选择日期时触发该事件。\n
     * 事件回调发生时，事件参数{@link ArkUI_NodeEvent}对象中的联合体类型为{@link ArkUI_NodeComponentEvent}。\n
     * <b>::ArkUI_NodeComponentEvent</b>中包含3个参数。\n
     * <b>ArkUI_NodeComponent.data[0].i32</b>表示 选中时间的年。\n
     * <b>ArkUI_NodeComponent.data[1].i32</b>表示 选中时间的月，取值范围：[0-11]。\n
     * <b>ArkUI_NodeComponent.data[2].i32</b>表示 选中时间的天。\n
     */
    NODE_DATE_PICKER_EVENT_ON_DATE_CHANGE = MAX_NODE_SCOPE_NUM * ARKUI_NODE_DATE_PICKER,

    /**
     * @brief 定义ARKUI_NODE_TIME_PICKER 列表组件的滚动触摸事件枚举值。
     *
     * 触发该事件的条件 ：选择时间时触发该事件。\n
     * 事件回调发生时，事件参数{@link ArkUI_NodeEvent}对象中的联合体类型为{@link ArkUI_NodeComponentEvent}。\n
     * <b>::ArkUI_NodeComponentEvent</b>中包含2个参数。\n
     * <b>ArkUI_NodeComponent.data[0].i32</b>表示 选中时间的时，取值范围：[0-23]。\n
     * <b>ArkUI_NodeComponent.data[1].i32</b>表示 选中时间的分，取值范围：[0-59]。\n
     */
    NODE_TIME_PICKER_EVENT_ON_CHANGE = MAX_NODE_SCOPE_NUM * ARKUI_NODE_TIME_PICKER,

    /**
     * @brief 定义ARKUI_NODE_CHECKBOX当选中状态发生变化时，触发该回调。
     *
     * 事件回调发生时，事件参数{@link ArkUI_NodeEvent}对象中的联合体类型为{@link ArkUI_NodeComponentEvent}。\n
     * <b>ArkUI_NodeComponent.data[0].i32</b>1:表示已选中, 0: 表示未选中\n
     */
    NODE_CHECKBOX_EVENT_ON_CHANGE = MAX_NODE_SCOPE_NUM * ARKUI_NODE_CHECKBOX,
    /**
     * @brief 定义ARKUI_NODE_SLIDER拖动或点击时触发事件回调。
     *
     * 事件回调发生时，事件参数{@link ArkUI_NodeEvent}对象中的联合体类型为{@link ArkUI_NodeComponentEvent}。\n
     * <b>ArkUI_NodeComponent.data[0].f32</b>当前滑动进度值。\n
     * <b>ArkUI_NodeComponent.data[1].i32</b>事件触发的相关状态值\n
     */
    NODE_SLIDER_EVENT_ON_CHANGE = MAX_NODE_SCOPE_NUM * ARKUI_NODE_SLIDER,

} ArkUI_NodeEventType;

/**
 * @brief 定义组件事件的通用结构类型。
 *
 * @since 12
 */
typedef struct {
    /**
     * @brief 事件类型。
     *
     * @see ArkUI_NodeEventType
     */
    int32_t kind;

    /**
     * @brief 事件自定义标识ID。
     *
     * 该事件id在调用{@link registerNodeEvent}函数时作为参数传递进来，可应用于同一事件入口函数{@link
     * registerNodeEventReceiver}分发逻辑。
     */
    int32_t eventId;

    /** 触发该组件的组件对象。*/
    ArkUI_NodeHandle node;
    union {
        /** touch事件类型回调参数。*/
        ArkUI_NodeTouchEvent touchEvent;
        /** 通用组件事件使用数字类型回调参数。*/
        ArkUI_NodeComponentEvent componentEvent;
        /** 通用组件事件使用字符串类型回调参数。*/
        ArkUI_StringAsyncEvent stringEvent;
    };
} ArkUI_NodeEvent;

/**
 * @brief 自定义组件调用<b>::markDirty</b>是传递的藏区标识类型。
 *
 * @since 12
 */
typedef enum {
    /**
     * @brief 重新测算大小。
     *
     * 该flag类型触发时，同时也默认会触发重新布局。
     */
    NODE_NEED_MEASURE = 1,

    /** 重新布局位置。*/
    NODE_NEED_LAYOUT,
    /** 重新进行绘制。*/
    NODE_NEED_RENDER,
} ArkUI_NodeDirtyFlag;

/**
 * @brief ArkUI提供的Native侧Node类型接口集合。
 *
 * @version 1
 * @since 12
 */
typedef struct {
    /** 结构体版本。 */
    int32_t version;

    /**
     * @brief 基于{@link ArkUI_NodeType}生成对应的组件并返回组件对象指针。
     *
     * @param type 创建指定类型的UI组件节点。
     * @return 返回创建完成的组件操作指针，如果创建失败返回NULL。
     */
    ArkUI_NodeHandle (*createNode)(ArkUI_NodeType type);

    /**
     * @brief 销毁组件指针指向的组件对象。
     *
     * @param node 组件指针对象。
     */
    void (*disposeNode)(ArkUI_NodeHandle node);

    /**
     * @brief 将组件挂载到某个父节点之下。
     *
     * @param parent 父节点指针。
     * @param child 子节点指针。
     */
    void (*addChild)(ArkUI_NodeHandle parent, ArkUI_NodeHandle child);

    /**
     * @brief 将组件从父节点中移除。
     *
     * @param parent 父节点指针。
     * @param child 子节点指针。
     */
    void (*removeChild)(ArkUI_NodeHandle parent, ArkUI_NodeHandle child);

    /**
     * @brief 将组件挂载到某个父节点之下，挂载位置在<b>sibling</b>节点之后。
     *
     * @param parent 父节点指针。
     * @param child 子节点指针。
     * @param sibling 前一个兄弟节点指针，如果为空则插入位置在最前面。
     */
    void (*insertChildAfter)(ArkUI_NodeHandle parent, ArkUI_NodeHandle child, ArkUI_NodeHandle sibling);

    /**
     * @brief 将组件挂载到某个父节点之下，挂载位置在<b>sibling</b>节点之前。
     *
     * @param parent 父节点指针。
     * @param child 子节点指针。
     * @param sibling 后一个兄弟节点指针，如果为空则插入位置在最后面。
     */
    void (*insertChildBefore)(ArkUI_NodeHandle parent, ArkUI_NodeHandle child, ArkUI_NodeHandle sibling);

    /**
     * @brief 将组件挂载到某个父节点之下，挂载位置由<b>position</b>指定。
     *
     * @param parent 父节点指针。
     * @param child 子节点指针。
     * @param postion 插入位置，如果插入位置为负数或者不存在，则默认插入位置在最后面。
     */
    void (*insertChildAt)(ArkUI_NodeHandle parent, ArkUI_NodeHandle child, int32_t position);

    /**
     * @brief 属性设置函数。
     *
     * @param node 需要设置属性的节点对象。
     * @param attribute 需要设置的属性类型。
     * @param item 需要设置的属性值。
     * @return 0 - 成功。
     *         401 - 函数参数异常。
     *         106101 - 系统中未找到Native接口的动态实现库。
     */
    int32_t (*setAttribute)(
        ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute, const ArkUI_AttributeItem* item);

    /**
     * @brief 属性获取函数。
     *
     * 该接口返回的指针是ArkUI框架内部的缓冲区指针，不需要开发者主动调用delete释放内存，但是需要在该函数下一次被调用前使用，否则可能会被其他值所覆盖。
     *
     * @param node 需要获取属性的节点对象。
     * @param attribute 需要获取的属性类型。
     * @return 当前属性类型的属性值，失败返回空指针。
     */
    const ArkUI_AttributeItem* (*getAttribute)(ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute);

    /**
     * @brief 重置属性函数。
     *
     * @param node 需要重置属性的节点对象。
     * @param attribute 需要重置的属性类型。
     */
    void (*resetAttribute)(ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute);

    /**
     * @brief 注册节点事件函数。
     *
     * @param node 需要注册事件的节点对象。
     * @param eventType 需要注册的事件类型。
     * @param eventId 自定义事件ID，当事件触发时在回调参数<@link ArkUI_NodeEvent>中携带回来。
     * @return 0 - 成功。
     *         401 - 函数参数异常。
     *         106101 - 系统中未找到Native接口的动态实现库。
     */
    int32_t (*registerNodeEvent)(ArkUI_NodeHandle node, ArkUI_NodeEventType eventType, int32_t eventId);

    /**
     * @brief 反注册节点事件函数。
     *
     * @param node 需要反注册事件的节点对象。
     * @param eventType 需要反注册的事件类型。
     */
    void (*unregisterNodeEvent)(ArkUI_NodeHandle node, ArkUI_NodeEventType eventType);

    /**
     * @brief 注册事件回调统一入口函数。
     *
     * ArkUI框架会统一收集过程中产生的组件事件并通过注册的eventReceiver函数回调给开发者。\n
     * 重复调用时会覆盖前一次注册的函数。
     *
     * @param eventReceiver 事件回调统一入口函数。
     */
    void (*registerNodeEventReceiver)(void (*eventReceiver)(ArkUI_NodeEvent* event));

    /**
     * @brief 反注册事件回调统一入口函数。
     *
     */
    void (*unregisterNodeEventReceiver)();

    /**
     * @brief 强制标记当前节点需要重新测算，布局或者绘制。
     *
     * 系统属性设置更新场景下ArkUI框架会自动标记藏区并重新执行测算，布局或者绘制，不需要开发者主动调用该函数。
     *
     * @param node 需要标记藏区的节点对象。
     * @param dirtyFlag 藏区类型。
     */
    void (*markDirty)(ArkUI_NodeHandle node, ArkUI_NodeDirtyFlag dirtyFlag);
} ArkUI_NativeNodeAPI_1;

#ifdef __cplusplus
};
#endif

#endif // ARKUI_NATIVE_NODE_H
/** @}*/
