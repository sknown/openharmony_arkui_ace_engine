/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_SPAN_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_SPAN_NODE_H

#include <list>
#include <memory>
#include <optional>
#include <string>

#include "base/memory/referenced.h"
#include "base/log/dump_log.h"
#include "core/common/ai/data_detector_adapter.h"
#include "core/common/resource/resource_object.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/text_style.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/rich_editor/selection_info.h"
#include "core/components_ng/pattern/text/text_styles.h"
#include "core/components_ng/pattern/text/span/tlv_util.h"
#include "core/components_ng/render/paragraph.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/components_v2/inspector/utils.h"
#include "core/components_ng/pattern/symbol/symbol_effect_options.h"

#define DEFINE_SPAN_FONT_STYLE_ITEM(name, type)                              \
public:                                                                      \
    std::optional<type> Get##name() const                                    \
    {                                                                        \
        if (spanItem_->fontStyle) {                                          \
            return spanItem_->fontStyle->Get##name();                        \
        }                                                                    \
        return std::nullopt;                                                 \
    }                                                                        \
    bool Has##name() const                                                   \
    {                                                                        \
        if (spanItem_->fontStyle) {                                          \
            return spanItem_->fontStyle->Has##name();                        \
        }                                                                    \
        return false;                                                        \
    }                                                                        \
    type Get##name##Value(const type& defaultValue) const                    \
    {                                                                        \
        if (spanItem_->fontStyle) {                                          \
            return spanItem_->fontStyle->Get##name().value_or(defaultValue); \
        }                                                                    \
        return defaultValue;                                                 \
    }                                                                        \
    void Update##name(const type& value)                                     \
    {                                                                        \
        if (!spanItem_->fontStyle) {                                         \
            spanItem_->fontStyle = std::make_unique<FontStyle>();            \
        }                                                                    \
        if (spanItem_->fontStyle->Check##name(value)) {                      \
            return;                                                          \
        }                                                                    \
        spanItem_->fontStyle->Update##name(value);                           \
        RequestTextFlushDirty();                                             \
    }                                                                        \
    void Reset##name()                                                       \
    {                                                                        \
        if (spanItem_->fontStyle) {                                          \
            return spanItem_->fontStyle->Reset##name();                      \
        }                                                                    \
    }                                                                        \
    void Update##name##WithoutFlushDirty(const type& value)                  \
    {                                                                        \
        if (!spanItem_->fontStyle) {                                         \
            spanItem_->fontStyle = std::make_unique<FontStyle>();            \
        }                                                                    \
        if (spanItem_->fontStyle->Check##name(value)) {                      \
            return;                                                          \
        }                                                                    \
        spanItem_->fontStyle->Update##name(value);                           \
    }

#define DEFINE_SPAN_TEXT_LINE_STYLE_ITEM(name, type)                             \
public:                                                                          \
    std::optional<type> Get##name() const                                        \
    {                                                                            \
        if (spanItem_->textLineStyle) {                                          \
            return spanItem_->textLineStyle->Get##name();                        \
        }                                                                        \
        return std::nullopt;                                                     \
    }                                                                            \
    bool Has##name() const                                                       \
    {                                                                            \
        if (spanItem_->textLineStyle) {                                          \
            return spanItem_->textLineStyle->Has##name();                        \
        }                                                                        \
        return false;                                                            \
    }                                                                            \
    type Get##name##Value(const type& defaultValue) const                        \
    {                                                                            \
        if (spanItem_->textLineStyle) {                                          \
            return spanItem_->textLineStyle->Get##name().value_or(defaultValue); \
        }                                                                        \
        return defaultValue;                                                     \
    }                                                                            \
    void Update##name(const type& value)                                         \
    {                                                                            \
        if (!spanItem_->textLineStyle) {                                         \
            spanItem_->textLineStyle = std::make_unique<TextLineStyle>();        \
        }                                                                        \
        if (spanItem_->textLineStyle->Check##name(value)) {                      \
            return;                                                              \
        }                                                                        \
        spanItem_->textLineStyle->Update##name(value);                           \
        RequestTextFlushDirty();                                                 \
    }                                                                            \
    void Reset##name()                                                           \
    {                                                                            \
        if (spanItem_->textLineStyle) {                                          \
            return spanItem_->textLineStyle->Reset##name();                      \
        }                                                                        \
    }                                                                            \
    void Update##name##WithoutFlushDirty(const type& value)                      \
    {                                                                            \
        if (!spanItem_->textLineStyle) {                                         \
            spanItem_->textLineStyle = std::make_unique<TextLineStyle>();        \
        }                                                                        \
        if (spanItem_->textLineStyle->Check##name(value)) {                      \
            return;                                                              \
        }                                                                        \
        spanItem_->textLineStyle->Update##name(value);                           \
    }

namespace OHOS::Ace::NG {
namespace {
constexpr double DEFAULT_FONT_SIZE_VALUE = 16.0;
}
using FONT_FEATURES_LIST = std::list<std::pair<std::string, int32_t>>;
class InspectorFilter;
class Paragraph;

enum class SpanItemType { NORMAL = 0, IMAGE = 1, CustomSpan = 2 };

struct PlaceholderStyle {
    double width = 0.0f;
    double height = 0.0f;
    double baselineOffset = 0.0f;
    VerticalAlign verticalAlign = VerticalAlign::BOTTOM;
    TextBaseline baseline = TextBaseline::ALPHABETIC;
    Dimension paragraphFontSize = Dimension(DEFAULT_FONT_SIZE_VALUE, DimensionUnit::FP);
};

struct CustomSpanPlaceholderInfo {
    int32_t customSpanIndex = -1;
    int32_t paragraphIndex = -1;
    std::function<void(NG::DrawingContext&, CustomSpanOptions)> onDraw;

    std::string ToString()
    {
        std::string result = "CustomPlaceholderInfo: [";
        result += "customSpanIndex: " + std::to_string(customSpanIndex);
        result += ", paragraphIndex: " + std::to_string(paragraphIndex);
        result += ", onDraw: ";
        result += !onDraw ? "nullptr" : "true";
        result += "]";
        return result;
    }
};
struct SpanItem : public AceType {
    DECLARE_ACE_TYPE(SpanItem, AceType);

public:
    SpanItem() = default;
    virtual ~SpanItem()
    {
        children.clear();
    }
    // position of last char + 1
    int32_t rangeStart = -1;
    int32_t position = -1;
    int32_t imageNodeId = -1;
    std::string inspectId;
    std::string description;
    std::string content;
    uint32_t unicode = 0;
    SpanItemType spanItemType = SpanItemType::NORMAL;
    std::pair<int32_t, int32_t> interval;
    std::unique_ptr<FontStyle> fontStyle = std::make_unique<FontStyle>();
    std::unique_ptr<TextLineStyle> textLineStyle = std::make_unique<TextLineStyle>();
    // for text background style
    std::optional<TextBackgroundStyle> backgroundStyle;
    GestureEventFunc onClick;
    GestureEventFunc onLongPress;
    [[deprecated]] std::list<RefPtr<SpanItem>> children;
    std::map<int32_t, AISpan> aiSpanMap;
    int32_t placeholderIndex = -1;
    // when paragraph ends with a \n, it causes the paragraph height to gain an extra line
    // to have normal spacing between paragraphs, remove \n from every paragraph except the last one.
    bool needRemoveNewLine = false;
    bool hasResourceFontColor = false;
    bool hasResourceDecorationColor = false;
    std::optional<LeadingMargin> leadingMargin;
    int32_t selectedStart = -1;
    int32_t selectedEnd = -1;
    void UpdateSymbolSpanParagraph(const RefPtr<FrameNode>& frameNode, const RefPtr<Paragraph>& builder);
    virtual int32_t UpdateParagraph(const RefPtr<FrameNode>& frameNode, const RefPtr<Paragraph>& builder,
        bool isSpanStringMode = false, PlaceholderStyle placeholderStyle = PlaceholderStyle());
    virtual void UpdateSymbolSpanColor(const RefPtr<FrameNode>& frameNode, TextStyle& symbolSpanStyle);
    virtual void UpdateTextStyleForAISpan(
        const std::string& content, const RefPtr<Paragraph>& builder, const TextStyle& textStyle);
    virtual void UpdateTextStyle(const std::string& content, const RefPtr<Paragraph>& builder,
        const TextStyle& textStyle, const int32_t selStart, const int32_t selEnd);
    virtual void UpdateContentTextStyle(
        const std::string& content, const RefPtr<Paragraph>& builder, const TextStyle& textStyle);
    virtual void SetAiSpanTextStyle(std::optional<TextStyle>& textStyle);
    virtual void GetIndex(int32_t& start, int32_t& end) const;
    virtual void FontRegisterCallback(const RefPtr<FrameNode>& frameNode, const TextStyle& textStyle);
    virtual void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const;
    std::string GetFont() const;
    virtual void StartDrag(int32_t start, int32_t end);
    virtual void EndDrag();
    virtual bool IsDragging();
    virtual ResultObject GetSpanResultObject(int32_t start, int32_t end);
    TextStyle InheritParentProperties(const RefPtr<FrameNode>& frameNode, bool isSpanStringMode = false);
    virtual RefPtr<SpanItem> GetSameStyleSpanItem() const;
    std::optional<std::pair<int32_t, int32_t>> GetIntersectionInterval(std::pair<int32_t, int32_t> interval) const;
    bool Contains(int32_t index)
    {
        return rangeStart < index && index < position;
    }
    std::optional<TextStyle> GetTextStyle() const
    {
        return textStyle_;
    }
    void SetTextStyle(const std::optional<TextStyle>& textStyle)
    {
        textStyle_ = textStyle;
    }
    RefPtr<ResourceObject> GetResourceObject()
    {
        return resourceObject_;
    }
    void SetResourceObject(RefPtr<ResourceObject> resourceObject)
    {
        resourceObject_ = resourceObject;
    }
    void SetNeedRemoveNewLine(bool value)
    {
        needRemoveNewLine = value;
    }
    void SetOnClickEvent(GestureEventFunc&& onClick_)
    {
        onClick = std::move(onClick_);
    }
    void SetLongPressEvent(GestureEventFunc&& onLongPress_)
    {
        onLongPress = std::move(onLongPress_);
    }
    void SetIsParentText(bool isText)
    {
        isParentText = isText;
    }
    bool GetIsParentText()
    {
        return isParentText;
    }
    std::string GetSpanContent(const std::string& rawContent);
    std::string GetSpanContent();
    uint32_t GetSymbolUnicode();
    std::string SymbolColorToString();

    virtual bool EncodeTlv(std::vector<uint8_t>& buff);
    static RefPtr<SpanItem> DecodeTlv(std::vector<uint8_t>& buff, int32_t& cursor);

private:
    std::optional<TextStyle> textStyle_;
    bool isParentText = false;
    RefPtr<ResourceObject> resourceObject_;
};

enum class PropertyInfo {
    FONTSIZE = 0,
    FONTCOLOR,
    FONTSTYLE,
    FONTWEIGHT,
    FONTFAMILY,
    TEXTDECORATION,
    TEXTCASE,
    LETTERSPACE,
    LINEHEIGHT,
    TEXT_ALIGN,
    LEADING_MARGIN,
    NONE,
    TEXTSHADOW,
    SYMBOL_COLOR,
    SYMBOL_RENDERING_STRATEGY,
    SYMBOL_EFFECT_STRATEGY,
    WORD_BREAK,
    LINE_BREAK_STRATEGY,
    FONTFEATURE,
    BASELINE_OFFSET,
    LINESPACING,
    SYMBOL_EFFECT_OPTIONS,
};

class ACE_EXPORT BaseSpan : public virtual AceType {
    DECLARE_ACE_TYPE(BaseSpan, AceType);

public:
    explicit BaseSpan(int32_t id) : groupId_(id) {}
    virtual void MarkTextDirty() = 0;
    virtual void SetTextBackgroundStyle(const TextBackgroundStyle& style);
    virtual void UpdateTextBackgroundFromParent(const std::optional<TextBackgroundStyle>& style)
    {
        textBackgroundStyle_ = style;
    }

    const std::optional<TextBackgroundStyle> GetTextBackgroundStyle() const
    {
        return textBackgroundStyle_;
    }

    void SetHasTextBackgroundStyle(bool hasStyle)
    {
        hasTextBackgroundStyle_ = hasStyle;
    }

    bool HasTextBackgroundStyle()
    {
        return hasTextBackgroundStyle_;
    }

private:
    std::optional<TextBackgroundStyle> textBackgroundStyle_;
    int32_t groupId_ = 0;
    bool hasTextBackgroundStyle_ = false;
};

class ACE_EXPORT SpanNode : public UINode, public BaseSpan {
    DECLARE_ACE_TYPE(SpanNode, UINode, BaseSpan);

public:
    static RefPtr<SpanNode> GetOrCreateSpanNode(int32_t nodeId);
    static RefPtr<SpanNode> GetOrCreateSpanNode(const std::string& tag, int32_t nodeId);
    static RefPtr<SpanNode> CreateSpanNode(int32_t nodeId);

    explicit SpanNode(int32_t nodeId) : UINode(V2::SPAN_ETS_TAG, nodeId), BaseSpan(nodeId) {}
    explicit SpanNode(const std::string& tag, int32_t nodeId) : UINode(tag, nodeId), BaseSpan(nodeId) {}
    ~SpanNode() override = default;

    void SetTextBackgroundStyle(const TextBackgroundStyle& style) override;
    void UpdateTextBackgroundFromParent(const std::optional<TextBackgroundStyle>& style) override;

    bool IsAtomicNode() const override
    {
        return true;
    }

    const RefPtr<SpanItem>& GetSpanItem() const
    {
        return spanItem_;
    }

    void UpdateContent(const uint32_t& unicode)
    {
        if (spanItem_->unicode == unicode) {
            return;
        }
        spanItem_->unicode = unicode;
        RequestTextFlushDirty();
    }

    void UpdateContent(const std::string& content)
    {
        if (spanItem_->content == content) {
            return;
        }
        spanItem_->content = content;
        RequestTextFlushDirty();
    }

    void UpdateOnClickEvent(GestureEventFunc&& onClick)
    {
        spanItem_->onClick = std::move(onClick);
    }

    void OnInspectorIdUpdate(const std::string& inspectorId) override
    {
        spanItem_->inspectId = inspectorId;
    }

    void OnAutoEventParamUpdate(const std::string& desc) override
    {
        spanItem_->description = desc;
    }

    DEFINE_SPAN_FONT_STYLE_ITEM(FontSize, Dimension);
    DEFINE_SPAN_FONT_STYLE_ITEM(TextColor, Color);
    DEFINE_SPAN_FONT_STYLE_ITEM(ItalicFontStyle, Ace::FontStyle);
    DEFINE_SPAN_FONT_STYLE_ITEM(FontWeight, FontWeight);
    DEFINE_SPAN_FONT_STYLE_ITEM(FontFamily, std::vector<std::string>);
    DEFINE_SPAN_FONT_STYLE_ITEM(TextDecoration, TextDecoration);
    DEFINE_SPAN_FONT_STYLE_ITEM(TextDecorationStyle, TextDecorationStyle);
    DEFINE_SPAN_FONT_STYLE_ITEM(TextDecorationColor, Color);
    DEFINE_SPAN_FONT_STYLE_ITEM(FontFeature, FONT_FEATURES_LIST);
    DEFINE_SPAN_FONT_STYLE_ITEM(TextCase, TextCase);
    DEFINE_SPAN_FONT_STYLE_ITEM(TextShadow, std::vector<Shadow>);
    DEFINE_SPAN_FONT_STYLE_ITEM(LetterSpacing, Dimension);
    DEFINE_SPAN_FONT_STYLE_ITEM(SymbolColorList, std::vector<Color>);
    DEFINE_SPAN_FONT_STYLE_ITEM(SymbolRenderingStrategy, uint32_t);
    DEFINE_SPAN_FONT_STYLE_ITEM(SymbolEffectStrategy, uint32_t);
    DEFINE_SPAN_FONT_STYLE_ITEM(SymbolEffectOptions, SymbolEffectOptions);
    DEFINE_SPAN_TEXT_LINE_STYLE_ITEM(LineHeight, Dimension);
    DEFINE_SPAN_TEXT_LINE_STYLE_ITEM(BaselineOffset, Dimension);
    DEFINE_SPAN_TEXT_LINE_STYLE_ITEM(TextAlign, TextAlign);
    DEFINE_SPAN_TEXT_LINE_STYLE_ITEM(WordBreak, WordBreak);
    DEFINE_SPAN_TEXT_LINE_STYLE_ITEM(LeadingMargin, LeadingMargin);
    DEFINE_SPAN_TEXT_LINE_STYLE_ITEM(LineBreakStrategy, LineBreakStrategy);
    DEFINE_SPAN_TEXT_LINE_STYLE_ITEM(LineSpacing, Dimension);

    // Mount to the previous Span node or Text node.
    void MountToParagraph();

    void AddChildSpanItem(const RefPtr<SpanNode>& child)
    {
        spanItem_->children.emplace_back(child->GetSpanItem());
    }

    void CleanSpanItemChildren()
    {
        spanItem_->children.clear();
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const override
    {
        spanItem_->ToJsonValue(json, filter);
    }

    void RequestTextFlushDirty();
    static void RequestTextFlushDirty(const RefPtr<UINode>& node);
    // The function is only used for fast preview.
    void FastPreviewUpdateChildDone() override
    {
        RequestTextFlushDirty();
    }

    void AddPropertyInfo(PropertyInfo value)
    {
        propertyInfo_.insert(value);
    }

    void CleanPropertyInfo()
    {
        propertyInfo_.clear();
    }

    void MarkTextDirty() override
    {
        RequestTextFlushDirty();
    }

    std::set<PropertyInfo> CalculateInheritPropertyInfo();

protected:
    void DumpInfo() override;

private:
    std::list<RefPtr<SpanNode>> spanChildren_;
    std::set<PropertyInfo> propertyInfo_;

    RefPtr<SpanItem> spanItem_ = MakeRefPtr<SpanItem>();

    ACE_DISALLOW_COPY_AND_MOVE(SpanNode);
};

struct PlaceholderSpanItem : public SpanItem {
    DECLARE_ACE_TYPE(PlaceholderSpanItem, SpanItem);

public:
    int32_t placeholderSpanNodeId = -1;
    TextStyle textStyle;
    PlaceholderRun run_;
    PlaceholderSpanItem() = default;
    ~PlaceholderSpanItem() override = default;
    void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const override {};
    int32_t UpdateParagraph(const RefPtr<FrameNode>& frameNode, const RefPtr<Paragraph>& builder,
        bool isSpanStringMode = false, PlaceholderStyle placeholderStyle = PlaceholderStyle()) override;

    void DumpInfo() const
    {
        auto& dumpLog = DumpLog::GetInstance();
        dumpLog.AddDesc("--------------- print run info ---------------");
        dumpLog.AddDesc(std::string("Width: ").append(std::to_string(run_.width)));
        dumpLog.AddDesc(std::string("Height: ").append(std::to_string(run_.height)));
        dumpLog.AddDesc(std::string("Alignment: ").append(StringUtils::ToString(run_.alignment)));
        dumpLog.AddDesc(std::string("Baseline: ").append(StringUtils::ToString(run_.baseline)));
        dumpLog.AddDesc(std::string("BaselineOffset: ").append(std::to_string(run_.baseline_offset)));
        dumpLog.AddDesc("--------------- print text style ---------------");
        dumpLog.AddDesc(std::string("FontSize: ").append(textStyle.GetFontSize().ToString()));
        dumpLog.AddDesc(std::string("LineHeight: ").append(textStyle.GetLineHeight().ToString()));
        dumpLog.AddDesc(std::string("LineSpacing: ").append(textStyle.GetLineSpacing().ToString()));
        dumpLog.AddDesc(std::string("VerticalAlign: ").append(StringUtils::ToString(textStyle.GetTextVerticalAlign())));
        dumpLog.AddDesc(std::string("HalfLeading: ").append(std::to_string(textStyle.GetHalfLeading())));
        dumpLog.AddDesc(std::string("TextBaseline: ").append(StringUtils::ToString(textStyle.GetTextBaseline())));
    }
    ACE_DISALLOW_COPY_AND_MOVE(PlaceholderSpanItem);
};

class PlaceholderSpanPattern : public Pattern {
    DECLARE_ACE_TYPE(PlaceholderSpanPattern, Pattern);

public:
    PlaceholderSpanPattern() = default;
    ~PlaceholderSpanPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }
};

class ACE_EXPORT PlaceholderSpanNode : public FrameNode {
    DECLARE_ACE_TYPE(PlaceholderSpanNode, FrameNode);

public:
    static RefPtr<PlaceholderSpanNode> GetOrCreateSpanNode(
        const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator)
    {
        auto frameNode = GetFrameNode(tag, nodeId);
        CHECK_NULL_RETURN(!frameNode, AceType::DynamicCast<PlaceholderSpanNode>(frameNode));
        auto pattern = patternCreator ? patternCreator() : MakeRefPtr<Pattern>();
        auto placeholderSpanNode = AceType::MakeRefPtr<PlaceholderSpanNode>(tag, nodeId, pattern);
        placeholderSpanNode->InitializePatternAndContext();
        ElementRegister::GetInstance()->AddUINode(placeholderSpanNode);
        return placeholderSpanNode;
    }

    PlaceholderSpanNode(const std::string& tag, int32_t nodeId) : FrameNode(tag, nodeId, AceType::MakeRefPtr<Pattern>())
    {}
    PlaceholderSpanNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
        : FrameNode(tag, nodeId, pattern)
    {}
    ~PlaceholderSpanNode() override = default;

    const RefPtr<PlaceholderSpanItem>& GetSpanItem() const
    {
        return placeholderSpanItem_;
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    void DumpInfo() override
    {
        FrameNode::DumpInfo();
        CHECK_NULL_VOID(placeholderSpanItem_);
        placeholderSpanItem_->DumpInfo();
    }

private:
    RefPtr<PlaceholderSpanItem> placeholderSpanItem_ = MakeRefPtr<PlaceholderSpanItem>();

    ACE_DISALLOW_COPY_AND_MOVE(PlaceholderSpanNode);
};

struct CustomSpanItem : public PlaceholderSpanItem {
    DECLARE_ACE_TYPE(CustomSpanItem, PlaceholderSpanItem);

public:
    CustomSpanItem() : PlaceholderSpanItem()
    {
        this->spanItemType = SpanItemType::CustomSpan;
    }
    ~CustomSpanItem() override = default;
    RefPtr<SpanItem> GetSameStyleSpanItem() const override;
    void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const override {};
    ACE_DISALLOW_COPY_AND_MOVE(CustomSpanItem);
    std::optional<std::function<CustomSpanMetrics(CustomSpanMeasureInfo)>> onMeasure;
    std::optional<std::function<void(NG::DrawingContext&, CustomSpanOptions)>> onDraw;
};

struct ImageSpanItem : public PlaceholderSpanItem {
    DECLARE_ACE_TYPE(ImageSpanItem, PlaceholderSpanItem);

public:
    ImageSpanItem() : PlaceholderSpanItem()
    {
        this->spanItemType = SpanItemType::IMAGE;
    }
    ~ImageSpanItem() override = default;
    int32_t UpdateParagraph(const RefPtr<FrameNode>& frameNode, const RefPtr<Paragraph>& builder,
        bool isSpanStringMode = false, PlaceholderStyle placeholderStyle = PlaceholderStyle()) override;
    void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const override {};
    void UpdatePlaceholderBackgroundStyle(const RefPtr<FrameNode>& imageNode);
    void SetImageSpanOptions(const ImageSpanOptions& options);
    void ResetImageSpanOptions();
    ResultObject GetSpanResultObject(int32_t start, int32_t end) override;
    RefPtr<SpanItem> GetSameStyleSpanItem() const override;
    ACE_DISALLOW_COPY_AND_MOVE(ImageSpanItem);

    bool EncodeTlv(std::vector<uint8_t>& buff) override;
    static RefPtr<ImageSpanItem> DecodeTlv(std::vector<uint8_t>& buff, int32_t& cursor);

    ImageSpanOptions options;
};

class ACE_EXPORT ImageSpanNode : public FrameNode {
    DECLARE_ACE_TYPE(ImageSpanNode, FrameNode);

public:
    static RefPtr<ImageSpanNode> GetOrCreateSpanNode(
        const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator)
    {
        auto frameNode = GetFrameNode(tag, nodeId);
        CHECK_NULL_RETURN(!frameNode, AceType::DynamicCast<ImageSpanNode>(frameNode));
        auto pattern = patternCreator ? patternCreator() : MakeRefPtr<Pattern>();
        auto imageSpanNode = AceType::MakeRefPtr<ImageSpanNode>(tag, nodeId, pattern);
        imageSpanNode->InitializePatternAndContext();
        ElementRegister::GetInstance()->AddUINode(imageSpanNode);
        return imageSpanNode;
    }

    ImageSpanNode(const std::string& tag, int32_t nodeId) : FrameNode(tag, nodeId, AceType::MakeRefPtr<ImagePattern>())
    {}
    ImageSpanNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
        : FrameNode(tag, nodeId, pattern)
    {}
    ~ImageSpanNode() override = default;

    const RefPtr<ImageSpanItem>& GetSpanItem() const
    {
        return imageSpanItem_;
    }

    void DumpInfo() override
    {
        FrameNode::DumpInfo();
        CHECK_NULL_VOID(imageSpanItem_);
        imageSpanItem_->DumpInfo();
    }

    void SetImageItem(const RefPtr<ImageSpanItem>& imageSpan)
    {
        imageSpanItem_ = imageSpan;
    }

private:
    RefPtr<ImageSpanItem> imageSpanItem_ = MakeRefPtr<ImageSpanItem>();

    ACE_DISALLOW_COPY_AND_MOVE(ImageSpanNode);
};

class ACE_EXPORT ContainerSpanNode : public UINode, public BaseSpan {
    DECLARE_ACE_TYPE(ContainerSpanNode, UINode, BaseSpan);

public:
    static RefPtr<ContainerSpanNode> GetOrCreateSpanNode(int32_t nodeId)
    {
        auto spanNode = ElementRegister::GetInstance()->GetSpecificItemById<ContainerSpanNode>(nodeId);
        if (spanNode) {
            spanNode->SetHasTextBackgroundStyle(false);
            return spanNode;
        }
        spanNode = MakeRefPtr<ContainerSpanNode>(nodeId);
        ElementRegister::GetInstance()->AddUINode(spanNode);
        return spanNode;
    }

    explicit ContainerSpanNode(int32_t nodeId) : UINode(V2::CONTAINER_SPAN_ETS_TAG, nodeId), BaseSpan(nodeId) {}
    ~ContainerSpanNode() override = default;

    void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const override;

    bool IsAtomicNode() const override
    {
        return false;
    }

    void MarkTextDirty() override
    {
        SpanNode::RequestTextFlushDirty(Claim(this));
    }

private:
    ACE_DISALLOW_COPY_AND_MOVE(ContainerSpanNode);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SYNTAX_FOR_EACH_NODE_H
