/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/text/span_node.h"

#include <optional>

#include "base/geometry/dimension.h"
#include "base/utils/utils.h"
#include "core/common/font_manager.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/text_style.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/text/text_styles.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/components_ng/render/paragraph.h"
#include "core/pipeline/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
std::string GetDeclaration(const std::optional<Color>& color, const std::optional<TextDecoration>& textDecoration,
    const std::optional<TextDecorationStyle>& textDecorationStyle)
{
    auto jsonSpanDeclaration = JsonUtil::Create(true);
    jsonSpanDeclaration->Put(
        "type", V2::ConvertWrapTextDecorationToStirng(textDecoration.value_or(TextDecoration::NONE)).c_str());
    jsonSpanDeclaration->Put("color", (color.value_or(Color::BLACK).ColorToString()).c_str());
    jsonSpanDeclaration->Put("style",
        V2::ConvertWrapTextDecorationStyleToString(textDecorationStyle.value_or(TextDecorationStyle::SOLID)).c_str());
    return jsonSpanDeclaration->ToString();
}
inline std::unique_ptr<JsonValue> ConvertShadowToJson(const Shadow& shadow)
{
    auto jsonShadow = JsonUtil::Create(true);
    jsonShadow->Put("radius", std::to_string(shadow.GetBlurRadius()).c_str());
    jsonShadow->Put("color", shadow.GetColor().ColorToString().c_str());
    jsonShadow->Put("offsetX", std::to_string(shadow.GetOffset().GetX()).c_str());
    jsonShadow->Put("offsetY", std::to_string(shadow.GetOffset().GetY()).c_str());
    jsonShadow->Put("type", std::to_string(static_cast<int32_t>(shadow.GetShadowType())).c_str());
    return jsonShadow;
}
std::unique_ptr<JsonValue> ConvertShadowsToJson(const std::vector<Shadow>& shadows)
{
    auto jsonShadows = JsonUtil::CreateArray(true);
    for (const auto& shadow : shadows) {
        jsonShadows->Put(ConvertShadowToJson(shadow));
    }
    return jsonShadows;
}
} // namespace

std::string SpanItem::GetFont() const
{
    auto jsonValue = JsonUtil::Create(true);
    jsonValue->Put("style", GetFontStyleInJson(fontStyle->GetItalicFontStyle()).c_str());
    jsonValue->Put("size", GetFontSizeInJson(fontStyle->GetFontSize()).c_str());
    jsonValue->Put("weight", GetFontWeightInJson(fontStyle->GetFontWeight()).c_str());
    jsonValue->Put("family", GetFontFamilyInJson(fontStyle->GetFontFamily()).c_str());
    return jsonValue->ToString();
}

void SpanItem::ToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    json->Put("content", content.c_str());
    if (fontStyle) {
        json->Put("font", GetFont().c_str());
        json->Put("fontSize", GetFontSizeInJson(fontStyle->GetFontSize()).c_str());
        json->Put("decoration", GetDeclaration(fontStyle->GetTextDecorationColor(), fontStyle->GetTextDecoration(),
            fontStyle->GetTextDecorationStyle()).c_str());
        json->Put("letterSpacing", fontStyle->GetLetterSpacing().value_or(Dimension()).ToString().c_str());
        json->Put(
            "textCase", V2::ConvertWrapTextCaseToStirng(fontStyle->GetTextCase().value_or(TextCase::NORMAL)).c_str());
        json->Put("fontColor", fontStyle->GetForegroundColor()
                                   .value_or(fontStyle->GetTextColor().value_or(Color::BLACK)).ColorToString().c_str());
        json->Put("fontStyle", GetFontStyleInJson(fontStyle->GetItalicFontStyle()).c_str());
        json->Put("fontWeight", GetFontWeightInJson(fontStyle->GetFontWeight()).c_str());
        json->Put("fontFamily", GetFontFamilyInJson(fontStyle->GetFontFamily()).c_str());
        json->Put("renderingStrategy",
            GetSymbolRenderingStrategyInJson(fontStyle->GetSymbolRenderingStrategy()).c_str());
        json->Put("effectStrategy", GetSymbolEffectStrategyInJson(fontStyle->GetSymbolEffectStrategy()).c_str());

        auto shadow = fontStyle->GetTextShadow().value_or(std::vector<Shadow> { Shadow() });
        // Determines if there are multiple textShadows
        auto jsonShadow = (shadow.size() == 1) ? ConvertShadowToJson(shadow.front()) : ConvertShadowsToJson(shadow);
        json->Put("textShadow", jsonShadow);
    }
    if (textLineStyle) {
        json->Put("lineHeight", textLineStyle->GetLineHeight().value_or(Dimension()).ToString().c_str());
    }
    TextBackgroundStyle::ToJsonValue(json, backgroundStyle);
}

RefPtr<SpanNode> SpanNode::GetOrCreateSpanNode(int32_t nodeId)
{
    auto spanNode = ElementRegister::GetInstance()->GetSpecificItemById<SpanNode>(nodeId);
    if (spanNode) {
        return spanNode;
    }
    spanNode = MakeRefPtr<SpanNode>(nodeId);
    ElementRegister::GetInstance()->AddUINode(spanNode);
    return spanNode;
}

RefPtr<SpanNode> SpanNode::CreateSpanNode(int32_t nodeId)
{
    auto spanNode = MakeRefPtr<SpanNode>(nodeId);
    ElementRegister::GetInstance()->AddUINode(spanNode);
    return spanNode;
}

RefPtr<SpanNode> SpanNode::GetOrCreateSpanNode(const std::string& tag, int32_t nodeId)
{
    auto spanNode = ElementRegister::GetInstance()->GetSpecificItemById<SpanNode>(nodeId);
    if (spanNode) {
        return spanNode;
    }
    spanNode = MakeRefPtr<SpanNode>(tag, nodeId);
    ElementRegister::GetInstance()->AddUINode(spanNode);
    return spanNode;
}

void SpanNode::MountToParagraph()
{
    auto parent = GetParent();
    while (parent) {
        auto spanNode = DynamicCast<SpanNode>(parent);
        if (spanNode) {
            spanNode->AddChildSpanItem(Claim(this));
            return;
        }
        auto textNode = DynamicCast<FrameNode>(parent);
        if (textNode) {
            auto textPattern = textNode->GetPattern<TextPattern>();
            if (textPattern) {
                textPattern->AddChildSpanItem(Claim(this));
                return;
            }
        }
        parent = parent->GetParent();
    }
}

void SpanNode::RequestTextFlushDirty()
{
    RequestTextFlushDirty(Claim<UINode>(this));
}

void SpanNode::RequestTextFlushDirty(const RefPtr<UINode>& node)
{
    CHECK_NULL_VOID(node);
    auto parent = node->GetParent();
    while (parent) {
        auto textNode = DynamicCast<FrameNode>(parent);
        if (textNode) {
            textNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
            auto textPattern = textNode->GetPattern<TextPattern>();
            if (textPattern) {
                textPattern->OnModifyDone();
                return;
            }
        }
        parent = parent->GetParent();
    }
}

void SpanNode::SetTextBackgroundStyle(const TextBackgroundStyle& style)
{
    BaseSpan::SetTextBackgroundStyle(style);
    spanItem_->backgroundStyle = GetTextBackgroundStyle();
}

void SpanNode::UpdateTextBackgroundFromParent(const std::optional<TextBackgroundStyle>& style)
{
    BaseSpan::UpdateTextBackgroundFromParent(style);
    spanItem_->backgroundStyle = GetTextBackgroundStyle();
}

int32_t SpanItem::UpdateParagraph(const RefPtr<FrameNode>& frameNode,
    const RefPtr<Paragraph>& builder, double /* width */, double /* height */, VerticalAlign /* verticalAlign */)
{
    CHECK_NULL_RETURN(builder, -1);
    std::optional<TextStyle> textStyle;
    if (fontStyle || textLineStyle) {
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_RETURN(pipelineContext, -1);
        TextStyle themeTextStyle =
            CreateTextStyleUsingTheme(fontStyle, textLineStyle, pipelineContext->GetTheme<TextTheme>());
        if (frameNode) {
            FontRegisterCallback(frameNode, themeTextStyle);
        }
        if (NearZero(themeTextStyle.GetFontSize().Value())) {
            return -1;
        }
        textStyle = themeTextStyle;
        textStyle->SetHalfLeading(pipelineContext->GetHalfLeading());
        builder->PushStyle(themeTextStyle);
    }

    auto spanContent = GetSpanContent(content);
    auto pattern = frameNode->GetPattern<TextPattern>();
    CHECK_NULL_RETURN(pattern, -1);
    if (textStyle.has_value()) {
        textStyle->SetTextBackgroundStyle(backgroundStyle);
    }
    if (pattern->NeedShowAIDetect() && !aiSpanMap.empty()) {
        UpdateTextStyleForAISpan(spanContent, builder, textStyle);
    } else {
        UpdateTextStyle(spanContent, builder, textStyle);
    }
    textStyle_ = textStyle;

    for (const auto& child : children) {
        if (child) {
            if (!aiSpanMap.empty()) {
                child->aiSpanMap = aiSpanMap;
            }
            child->UpdateParagraph(frameNode, builder);
        }
    }
    if (fontStyle || textLineStyle) {
        builder->PopStyle();
    }
    return -1;
}

void SpanItem::UpdateSymbolSpanParagraph(const RefPtr<FrameNode>& frameNode, const RefPtr<Paragraph>& builder)
{
    CHECK_NULL_VOID(builder);
    std::optional<TextStyle> textStyle;
    auto symbolUnicode = GetSymbolUnicode();
    if (fontStyle || textLineStyle) {
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        TextStyle themeTextStyle =
            CreateTextStyleUsingTheme(fontStyle, textLineStyle, pipelineContext->GetTheme<TextTheme>());
        if (frameNode) {
            FontRegisterCallback(frameNode, themeTextStyle);
        }
        if (NearZero(themeTextStyle.GetFontSize().Value())) {
            return;
        }
        textStyle = themeTextStyle;
        textStyle->SetHalfLeading(pipelineContext->GetHalfLeading());
        if (symbolUnicode != 0) {
            UpdateSymbolSpanColor(frameNode, themeTextStyle);
        }
        builder->PushStyle(themeTextStyle);
    }
    textStyle_ = textStyle;

    if (symbolUnicode != 0) {
        textStyle_->isSymbolGlyph_ = true;
        builder->AddSymbol(symbolUnicode);
    }

    if (fontStyle || textLineStyle) {
        builder->PopStyle();
    }
}

void SpanItem::UpdateSymbolSpanColor(const RefPtr<FrameNode>& frameNode, TextStyle& symbolSpanStyle)
{
    symbolSpanStyle.isSymbolGlyph_ = true;
    CHECK_NULL_VOID(frameNode);
    if (GetIsParentText() && symbolSpanStyle.GetSymbolColorList().empty()) {
        RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
        CHECK_NULL_VOID(layoutProperty);
        RefPtr<TextLayoutProperty> textLayoutProperty = DynamicCast<TextLayoutProperty>(layoutProperty);
        CHECK_NULL_VOID(textLayoutProperty);
        if (textLayoutProperty->GetTextColor().has_value()) {
            std::vector<Color> symbolColor;
            symbolColor.emplace_back(textLayoutProperty->GetTextColor().value());
            symbolSpanStyle.SetSymbolColorList(symbolColor);
        }
    }
}

void SpanItem::UpdateTextStyleForAISpan(
    const std::string& spanContent, const RefPtr<Paragraph>& builder, const std::optional<TextStyle>& textStyle)
{
    auto wSpanContent = StringUtils::ToWstring(spanContent);
    int32_t wSpanContentLength = static_cast<int32_t>(wSpanContent.length());
    int32_t spanStart = position - wSpanContentLength;
    if (needRemoveNewLine) {
        spanStart -= 1;
    }
    int32_t preEnd = spanStart;
    std::optional<TextStyle> aiSpanTextStyle = textStyle;
    SetAiSpanTextStyle(aiSpanTextStyle);
    while (!aiSpanMap.empty()) {
        auto aiSpan = aiSpanMap.begin()->second;
        if (aiSpan.start >= position || preEnd >= position) {
            break;
        }
        int32_t aiSpanStartInSpan = std::max(spanStart, aiSpan.start);
        int32_t aiSpanEndInSpan = std::min(position, aiSpan.end);
        if (aiSpan.end <= spanStart || aiSpanStartInSpan < preEnd) {
            TAG_LOGI(AceLogTag::ACE_TEXT, "Error prediction");
            aiSpanMap.erase(aiSpanMap.begin());
            continue;
        }
        if (preEnd < aiSpanStartInSpan) {
            auto beforeContent =
                StringUtils::ToString(wSpanContent.substr(preEnd - spanStart, aiSpanStartInSpan - preEnd));
            UpdateContentTextStyle(beforeContent, builder, textStyle);
        }
        auto pipelineContext = PipelineContext::GetCurrentContext();
        TextStyle normalStyle =
            !pipelineContext ? TextStyle()
                                : CreateTextStyleUsingTheme(nullptr, nullptr, pipelineContext->GetTheme<TextTheme>());
        TextStyle selectedTextStyle = textStyle.value_or(normalStyle);
        Color color = selectedTextStyle.GetTextColor().ChangeAlpha(DRAGGED_TEXT_OPACITY);
        selectedTextStyle.SetTextColor(color);
        auto midTextStyle = !IsDragging() ? aiSpanTextStyle : selectedTextStyle;
        auto displayContent = StringUtils::ToWstring(
            aiSpan.content).substr(aiSpanStartInSpan - aiSpan.start, aiSpanEndInSpan - aiSpanStartInSpan);
        UpdateContentTextStyle(StringUtils::ToString(displayContent), builder, midTextStyle);
        preEnd = aiSpanEndInSpan;
        if (aiSpan.end > position) {
            return;
        } else {
            aiSpanMap.erase(aiSpanMap.begin());
        }
    }
    if (preEnd < position) {
        auto afterContent = StringUtils::ToString(wSpanContent.substr(preEnd - spanStart, position - preEnd));
        UpdateContentTextStyle(afterContent, builder, textStyle);
    }
}

void SpanItem::SetAiSpanTextStyle(std::optional<TextStyle>& aiSpanTextStyle)
{
    if (!aiSpanTextStyle.has_value()) {
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        TextStyle themeTextStyle =
            CreateTextStyleUsingTheme(fontStyle, textLineStyle, pipelineContext->GetTheme<TextTheme>());
        if (NearZero(themeTextStyle.GetFontSize().Value())) {
            return;
        }
        aiSpanTextStyle = themeTextStyle;
    } else {
        aiSpanTextStyle.value().SetTextColor(Color::BLUE);
        aiSpanTextStyle.value().SetTextDecoration(TextDecoration::UNDERLINE);
        aiSpanTextStyle.value().SetTextDecorationColor(Color::BLUE);
    }
}

void SpanItem::FontRegisterCallback(const RefPtr<FrameNode>& frameNode, const TextStyle& textStyle)
{
    auto callback = [weakNode = WeakPtr<FrameNode>(frameNode)] {
        auto frameNode = weakNode.Upgrade();
        CHECK_NULL_VOID(frameNode);
        frameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        auto pattern = frameNode->GetPattern<TextPattern>();
        CHECK_NULL_VOID(pattern);
        auto modifier = DynamicCast<TextContentModifier>(pattern->GetContentModifier());
        CHECK_NULL_VOID(modifier);
        modifier->SetFontReady(true);
    };
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_VOID(pipeline);
    auto fontManager = pipeline->GetFontManager();
    if (fontManager) {
        bool isCustomFont = false;
        for (const auto& familyName : textStyle.GetFontFamilies()) {
            bool customFont = fontManager->RegisterCallbackNG(frameNode, familyName, callback);
            if (customFont) {
                isCustomFont = true;
            }
        }
        if (isCustomFont) {
            auto pattern = frameNode->GetPattern<TextPattern>();
            CHECK_NULL_VOID(pattern);
            pattern->SetIsCustomFont(true);
            auto modifier = DynamicCast<TextContentModifier>(pattern->GetContentModifier());
            CHECK_NULL_VOID(modifier);
            modifier->SetIsCustomFont(true);
        }
    }
}

void SpanItem::UpdateTextStyle(
    const std::string& content, const RefPtr<Paragraph>& builder, const std::optional<TextStyle>& textStyle)
{
    if (!IsDragging()) {
        UpdateContentTextStyle(content, builder, textStyle);
    } else {
        if (content.empty()) {
            return;
        }
        auto displayContent = StringUtils::Str8ToStr16(content);
        auto contentLength = static_cast<int32_t>(displayContent.length());
        auto beforeSelectedText = displayContent.substr(0, selectedStart);
        UpdateContentTextStyle(StringUtils::Str16ToStr8(beforeSelectedText), builder, textStyle);
        if (selectedStart < contentLength) {
            auto pipelineContext = PipelineContext::GetCurrentContext();
            TextStyle normalStyle =
                !pipelineContext ? TextStyle()
                                 : CreateTextStyleUsingTheme(nullptr, nullptr, pipelineContext->GetTheme<TextTheme>());
            TextStyle selectedTextStyle = textStyle.value_or(normalStyle);
            Color color = selectedTextStyle.GetTextColor().ChangeAlpha(DRAGGED_TEXT_OPACITY);
            selectedTextStyle.SetTextColor(color);
            auto selectedText = displayContent.substr(selectedStart, selectedEnd - selectedStart);
            UpdateContentTextStyle(StringUtils::Str16ToStr8(selectedText), builder, selectedTextStyle);
        }

        if (selectedEnd < contentLength) {
            auto afterSelectedText = displayContent.substr(selectedEnd);
            UpdateContentTextStyle(StringUtils::Str16ToStr8(afterSelectedText), builder, textStyle);
        }
    }
}

void SpanItem::UpdateContentTextStyle(
    const std::string& content, const RefPtr<Paragraph>& builder, const std::optional<TextStyle>& textStyle)
{
    if (content.empty()) {
        return;
    }
    auto displayText = content;
    auto textCase = fontStyle ? fontStyle->GetTextCase().value_or(TextCase::NORMAL) : TextCase::NORMAL;
    StringUtils::TransformStrCase(displayText, static_cast<int32_t>(textCase));
    if (textStyle.has_value()) {
        builder->PushStyle(textStyle.value());
    }
    builder->AddText(StringUtils::Str8ToStr16(displayText));
    if (textStyle.has_value()) {
        builder->PopStyle();
    }
}

std::string SpanItem::GetSpanContent(const std::string& rawContent)
{
    std::string data;
    if (needRemoveNewLine) {
        data = rawContent.substr(0, rawContent.length() - 1);
    } else {
        data = rawContent;
    }
    return data;
}

std::string SpanItem::GetSpanContent()
{
    return content;
}

uint32_t SpanItem::GetSymbolUnicode()
{
    return unicode;
}

void SpanItem::StartDrag(int32_t start, int32_t end)
{
    selectedStart = std::max(0, start);
    int contentLen = content.size();
    selectedEnd = std::min(contentLen, end);
}

void SpanItem::EndDrag()
{
    selectedStart = -1;
    selectedEnd = -1;
}

bool SpanItem::IsDragging()
{
    return selectedStart >= 0 && selectedEnd >= 0;
}

#define COPY_TEXT_STYLE(group, name, func)                          \
    do {                                                            \
        if ((group)->Has##name()) {                                 \
            sameSpan->group->func((group)->prop##name.value());     \
        }                                                           \
    } while (false)

RefPtr<SpanItem> SpanItem::GetSameStyleSpanItem() const
{
    auto sameSpan = MakeRefPtr<SpanItem>();
    COPY_TEXT_STYLE(fontStyle, FontSize, UpdateFontSize);
    COPY_TEXT_STYLE(fontStyle, TextColor, UpdateTextColor);
    COPY_TEXT_STYLE(fontStyle, TextShadow, UpdateTextShadow);
    COPY_TEXT_STYLE(fontStyle, ItalicFontStyle, UpdateItalicFontStyle);
    COPY_TEXT_STYLE(fontStyle, FontWeight, UpdateFontWeight);
    COPY_TEXT_STYLE(fontStyle, FontFamily, UpdateFontFamily);
    COPY_TEXT_STYLE(fontStyle, FontFeature, UpdateFontFeature);
    COPY_TEXT_STYLE(fontStyle, TextDecoration, UpdateTextDecoration);
    COPY_TEXT_STYLE(fontStyle, TextDecorationColor, UpdateTextDecorationColor);
    COPY_TEXT_STYLE(fontStyle, TextDecorationStyle, UpdateTextDecorationStyle);
    COPY_TEXT_STYLE(fontStyle, TextCase, UpdateTextCase);
    COPY_TEXT_STYLE(fontStyle, AdaptMinFontSize, UpdateAdaptMinFontSize);
    COPY_TEXT_STYLE(fontStyle, AdaptMaxFontSize, UpdateAdaptMaxFontSize);
    COPY_TEXT_STYLE(fontStyle, LetterSpacing, UpdateLetterSpacing);

    COPY_TEXT_STYLE(textLineStyle, LineHeight, UpdateLineHeight);
    COPY_TEXT_STYLE(textLineStyle, TextBaseline, UpdateTextBaseline);
    COPY_TEXT_STYLE(textLineStyle, BaselineOffset, UpdateBaselineOffset);
    COPY_TEXT_STYLE(textLineStyle, TextOverflow, UpdateTextOverflow);
    COPY_TEXT_STYLE(textLineStyle, TextAlign, UpdateTextAlign);
    COPY_TEXT_STYLE(textLineStyle, MaxLength, UpdateMaxLength);
    COPY_TEXT_STYLE(textLineStyle, MaxLines, UpdateMaxLines);
    COPY_TEXT_STYLE(textLineStyle, HeightAdaptivePolicy, UpdateHeightAdaptivePolicy);
    COPY_TEXT_STYLE(textLineStyle, TextIndent, UpdateTextIndent);
    COPY_TEXT_STYLE(textLineStyle, LeadingMargin, UpdateLeadingMargin);
    COPY_TEXT_STYLE(textLineStyle, WordBreak, UpdateWordBreak);
    COPY_TEXT_STYLE(textLineStyle, EllipsisMode, UpdateEllipsisMode);

    if (backgroundStyle.has_value()) {
        sameSpan->backgroundStyle->backgroundColor = backgroundStyle->backgroundColor;
        sameSpan->backgroundStyle->backgroundRadius = backgroundStyle->backgroundRadius;
        sameSpan->backgroundStyle->groupId = backgroundStyle->groupId;
    }

    sameSpan->onClick = onClick;
    sameSpan->onLongPress = onLongPress;
    return sameSpan;
}

std::optional<std::pair<int32_t, int32_t>> SpanItem::GetIntersectionInterval(std::pair<int32_t, int32_t> interval) const
{
    // Check the intersection
    if (this->interval.second <= interval.first || interval.second <= this->interval.first) {
        return std::nullopt;
    }

    // Calculate the intersection interval
    int start = std::max(this->interval.first, interval.first);
    int end = std::min(this->interval.second, interval.second);
    return std::make_optional<std::pair<int32_t, int32_t>>(std::make_pair(start, end));
}

int32_t ImageSpanItem::UpdateParagraph(const RefPtr<FrameNode>& /* frameNode */, const RefPtr<Paragraph>& builder,
    double width, double height, VerticalAlign verticalAlign)
{
    CHECK_NULL_RETURN(builder, -1);
    PlaceholderRun run;
    textStyle = TextStyle();
    run.width = width;
    run.height = height;
    switch (verticalAlign) {
        case VerticalAlign::TOP:
            run.alignment = PlaceholderAlignment::TOP;
            break;
        case VerticalAlign::CENTER:
            run.alignment = PlaceholderAlignment::MIDDLE;
            break;
        case VerticalAlign::BOTTOM:
        case VerticalAlign::NONE:
            run.alignment = PlaceholderAlignment::BOTTOM;
            break;
        case VerticalAlign::BASELINE:
            run.alignment = PlaceholderAlignment::ABOVEBASELINE;
            break;
        default:
            run.alignment = PlaceholderAlignment::BOTTOM;
    }
    // ImageSpan should ignore decoration styles
    textStyle.SetTextDecoration(TextDecoration::NONE);
    textStyle.SetTextBackgroundStyle(backgroundStyle);
    builder->PushStyle(textStyle);
    int32_t index = builder->AddPlaceholder(run);
    builder->PopStyle();
    return index;
}

void ImageSpanItem::UpdatePlaceholderBackgroundStyle(const RefPtr<FrameNode>& imageNode)
{
    CHECK_NULL_VOID(imageNode);
    auto property = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(property);
    backgroundStyle = property->GetPlaceHolderStyle();
}

void SpanItem::GetIndex(int32_t& start, int32_t& end) const
{
    auto contentLen = StringUtils::ToWstring(content).length();
    start = position - contentLen;
    end = position;
}

int32_t PlaceholderSpanItem::UpdateParagraph(const RefPtr<FrameNode>& /* frameNode */, const RefPtr<Paragraph>& builder,
    double width, double height, VerticalAlign /* verticalAlign */)
{
    CHECK_NULL_RETURN(builder, -1);
    textStyle = TextStyle();
    PlaceholderRun run;
    run.width = width;
    run.height = height;
    textStyle.SetTextDecoration(TextDecoration::NONE);
    builder->PushStyle(textStyle);
    int32_t index = builder->AddPlaceholder(run);
    builder->PopStyle();
    return index;
}

void BaseSpan::SetTextBackgroundStyle(const TextBackgroundStyle& style)
{
    textBackgroundStyle_ = style;
    textBackgroundStyle_->groupId = groupId_;
    SetHasTextBackgroundStyle(style.backgroundColor.has_value() || style.backgroundRadius.has_value());
    MarkTextDirty();
}

void ContainerSpanNode::ToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    TextBackgroundStyle::ToJsonValue(json, GetTextBackgroundStyle());
}
} // namespace OHOS::Ace::NG
