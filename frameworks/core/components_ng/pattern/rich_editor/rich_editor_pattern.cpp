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
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"

#include <algorithm>
#include <chrono>
#include <iterator>

#include "base/geometry/ng/offset_t.h"
#include "base/log/dump_log.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "core/common/clipboard/paste_data.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_event_hub.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_overlay_modifier.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_theme.h"
#include "core/components_ng/pattern/rich_editor_drag/rich_editor_drag_pattern.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_ng/pattern/text/text_base.h"
#include "core/components_ng/pattern/text_field/text_field_manager.h"

#if not defined(ACE_UNITTEST)
#if defined(ENABLE_STANDARD_INPUT)
#include "commonlibrary/c_utils/base/include/refbase.h"

#include "core/components_ng/pattern/rich_editor/on_rich_editor_changed_listener_impl.h"
#endif
#endif

#ifdef ENABLE_DRAG_FRAMEWORK
#include "core/common/ace_engine_ext.h"
#include "core/common/udmf/udmf_client.h"
#endif

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t IMAGE_SPAN_LENGTH = 1;
constexpr int32_t RICH_EDITOR_TWINKLING_INTERVAL_MS = 500;
constexpr float DEFAULT_IMAGE_SIZE = 57.0f;
constexpr float DEFAULT_TEXT_SIZE = 16.0f;

const std::wstring lineSeparator = L"\n";
} // namespace
RichEditorPattern::RichEditorPattern() {}

RichEditorPattern::~RichEditorPattern()
{
    if (isCustomKeyboardAttached_) {
        CloseCustomKeyboard();
    }
}

void RichEditorPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<TextLayoutProperty>();
    copyOption_ = layoutProperty->GetCopyOption().value_or(CopyOptions::Distributed);
    auto context = host->GetContext();
    CHECK_NULL_VOID(context);
    context->AddOnAreaChangeNode(host->GetId());
    if (!clipboard_ && context) {
        clipboard_ = ClipboardProxy::GetInstance()->GetClipboard(context->GetTaskExecutor());
    }
    instanceId_ = context->GetInstanceId();
    InitMouseEvent();
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    InitFocusEvent(focusHub);
    auto gestureEventHub = host->GetOrCreateGestureEventHub();
    InitClickEvent(gestureEventHub);
    InitLongPressEvent(gestureEventHub);
    InitTouchEvent();
    HandleEnabled();
#ifdef ENABLE_DRAG_FRAMEWORK
    if (host->IsDraggable()) {
        InitDragDropEvent();
        AddDragFrameNodeToManager(host);
    }
#endif // ENABLE_DRAG_FRAMEWORK
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RichEditorPattern::HandleEnabled()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    if (IsDisabled()) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto richEditorTheme = pipeline->GetTheme<RichEditorTheme>();
        CHECK_NULL_VOID(richEditorTheme);
        auto disabledAlpha = richEditorTheme->GetDisabledAlpha();
        renderContext->OnOpacityUpdate(disabledAlpha);
    } else {
        auto opacity = renderContext->GetOpacity().value_or(1.0);
        renderContext->OnOpacityUpdate(opacity);
    }
}

void RichEditorPattern::BeforeCreateLayoutWrapper()
{
    paragraphs_.Reset();
    TextPattern::BeforeCreateLayoutWrapper();
}

bool RichEditorPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    frameRect_ = dirty->GetGeometryNode()->GetFrameRect();
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto richEditorLayoutAlgorithm =
        DynamicCast<RichEditorLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(richEditorLayoutAlgorithm, false);
    parentGlobalOffset_ = richEditorLayoutAlgorithm->GetParentGlobalOffset();
    UpdateTextFieldManager(Offset(parentGlobalOffset_.GetX(), parentGlobalOffset_.GetY()), frameRect_.Height());
    bool ret = TextPattern::OnDirtyLayoutWrapperSwap(dirty, config);
    if (textSelector_.baseOffset != -1 && textSelector_.destinationOffset != -1) {
        CalculateHandleOffsetAndShowOverlay();
        ShowSelectOverlay(textSelector_.firstHandle, textSelector_.secondHandle);
    }
    if (!isRichEditorInit_) {
        auto eventHub = GetEventHub<RichEditorEventHub>();
        CHECK_NULL_RETURN(eventHub, ret);
        eventHub->FireOnReady();
        isRichEditorInit_ = true;
    }
    MoveCaretAfterTextChange();
    UpdateCaretInfoToController();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, ret);
    auto context = host->GetRenderContext();
    CHECK_NULL_RETURN(context, ret);
    if (context->GetClipEdge().has_value()) {
        auto geometryNode = host->GetGeometryNode();
        auto frameOffset = geometryNode->GetFrameOffset();
        auto frameSize = geometryNode->GetFrameSize();
        auto height = static_cast<float>(paragraphs_.GetHeight() + std::fabs(baselineOffset_));
        if (!context->GetClipEdge().value() && LessNotEqual(frameSize.Height(), height)) {
            RectF boundsRect(frameOffset.GetX(), frameOffset.GetY(), frameSize.Width(), height);
            overlayMod_->SetBoundsRect(boundsRect);
        }
    }
    return ret;
}

int32_t RichEditorPattern::GetInstanceId() const
{
    return instanceId_;
}

std::function<ImageSourceInfo()> RichEditorPattern::CreateImageSourceInfo(const ImageSpanOptions& options)
{
    std::string src;
    RefPtr<PixelMap> pixMap = nullptr;
    std::string bundleName;
    std::string moduleName;
    if (options.image.has_value()) {
        src = options.image.value();
    }
    if (options.imagePixelMap.has_value()) {
        pixMap = options.imagePixelMap.value();
    }
    if (options.bundleName.has_value()) {
        bundleName = options.bundleName.value();
    }
    if (options.moduleName.has_value()) {
        moduleName = options.moduleName.value();
    }
    auto createSourceInfoFunc = [src, noPixMap = !options.imagePixelMap.has_value(), pixMap, bundleName,
                                    moduleName]() -> ImageSourceInfo {
#if defined(PIXEL_MAP_SUPPORTED)
        if (noPixMap) {
            return { src, bundleName, moduleName };
        }
        return ImageSourceInfo(pixMap);
#else
        return { src, bundleName, moduleName };
#endif
    };
    return std::move(createSourceInfoFunc);
}

int32_t RichEditorPattern::AddImageSpan(const ImageSpanOptions& options, bool isPaste, int32_t index)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, -1);

    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto imageNode = FrameNode::GetOrCreateFrameNode(
        V2::IMAGE_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<ImagePattern>(); });
    auto imageLayoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();

    // Disable the image itself event
    imageNode->SetDraggable(false);
    auto gesture = imageNode->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(gesture, -1);
    gesture->SetHitTestMode(HitTestMode::HTMNONE);

    int32_t spanIndex = 0;
    int32_t offset = -1;
    if (options.offset.has_value()) {
        offset = TextSpanSplit(options.offset.value());
        if (offset == -1) {
            spanIndex = host->GetChildren().size();
        } else {
            spanIndex = offset;
        }
        imageNode->MountToParent(host, offset);
    } else if (index != -1) {
        imageNode->MountToParent(host, index);
        spanIndex = index;
    } else {
        spanIndex = host->GetChildren().size();
        imageNode->MountToParent(host);
    }
    std::function<ImageSourceInfo()> createSourceInfoFunc = CreateImageSourceInfo(options);
    imageLayoutProperty->UpdateImageSourceInfo(createSourceInfoFunc());
    if (options.imageAttribute.has_value()) {
        if (options.imageAttribute.value().size.has_value()) {
            imageLayoutProperty->UpdateUserDefinedIdealSize(
                CalcSize(CalcLength(options.imageAttribute.value().size.value().width),
                    CalcLength(options.imageAttribute.value().size.value().height)));
        }
        if (options.imageAttribute.value().verticalAlign.has_value()) {
            imageLayoutProperty->UpdateVerticalAlign(options.imageAttribute.value().verticalAlign.value());
        }
        if (options.imageAttribute.value().objectFit.has_value()) {
            imageLayoutProperty->UpdateImageFit(options.imageAttribute.value().objectFit.value());
        }
    }
    if (isPaste) {
        isTextChange_ = true;
        moveDirection_ = MoveDirection::FORWARD;
        moveLength_ += 1;
    }
    imageNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    imageNode->MarkModifyDone();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
    auto spanItem = MakeRefPtr<ImageSpanItem>();

    // The length of the imageSpan defaults to the length of a character to calculate the position
    spanItem->content = " ";
    AddSpanItem(spanItem, offset);
    if (options.offset.has_value() && options.offset.value() >= GetCaretPosition()) {
        SetCaretPosition(GetCaretPosition() + 1);
    }
    if (!isPaste && textSelector_.IsValid()) {
        CloseSelectOverlay();
        ResetSelection();
    }

    return spanIndex;
}

void RichEditorPattern::AddSpanItem(const RefPtr<SpanItem>& item, int32_t offset)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (offset == -1) {
        offset = host->GetChildren().size();
    }
    offset = std::clamp(offset, 0, static_cast<int32_t>(host->GetChildren().size()) - 1);
    auto it = spans_.begin();
    std::advance(it, offset);
    spans_.insert(it, item);
    int32_t spanTextLength = 0;
    for (auto& span : spans_) {
        span->position = spanTextLength + StringUtils::ToWstring(span->content).length();
        spanTextLength += StringUtils::ToWstring(span->content).length();
    }
}

int32_t RichEditorPattern::AddTextSpan(const TextSpanOptions& options, bool isPaste, int32_t index)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, -1);

    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto spanNode = SpanNode::GetOrCreateSpanNode(nodeId);

    int32_t spanIndex = 0;
    int32_t offset = -1;
    if (options.offset.has_value()) {
        offset = TextSpanSplit(options.offset.value());
        if (offset == -1) {
            spanIndex = host->GetChildren().size();
        } else {
            spanIndex = offset;
        }
        spanNode->MountToParent(host, offset);
    } else if (index != -1) {
        spanNode->MountToParent(host, index);
        spanIndex = index;
    } else {
        spanIndex = host->GetChildren().size();
        spanNode->MountToParent(host);
    }
    spanNode->UpdateContent(options.value);
    spanNode->AddPropertyInfo(PropertyInfo::NONE);
    if (options.style.has_value()) {
        spanNode->UpdateTextColor(options.style.value().GetTextColor());
        spanNode->AddPropertyInfo(PropertyInfo::FONTCOLOR);
        spanNode->UpdateFontSize(options.style.value().GetFontSize());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSIZE);
        spanNode->UpdateItalicFontStyle(options.style.value().GetFontStyle());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSTYLE);
        spanNode->UpdateFontWeight(options.style.value().GetFontWeight());
        spanNode->AddPropertyInfo(PropertyInfo::FONTWEIGHT);
        spanNode->UpdateFontFamily(options.style.value().GetFontFamilies());
        spanNode->AddPropertyInfo(PropertyInfo::FONTFAMILY);
        spanNode->UpdateTextDecoration(options.style.value().GetTextDecoration());
        spanNode->AddPropertyInfo(PropertyInfo::TEXTDECORATION);
        spanNode->UpdateTextDecorationColor(options.style.value().GetTextDecorationColor());
        spanNode->AddPropertyInfo(PropertyInfo::NONE);
    }
    auto spanItem = spanNode->GetSpanItem();
    spanItem->content = options.value;
    spanItem->SetTextStyle(options.style);
    AddSpanItem(spanItem, offset);

    if (options.paraStyle) {
        int32_t start, end;
        spanItem->GetIndex(start, end);
        UpdateParagraphStyle(start, end, *options.paraStyle);
    }

    if (!isPaste && textSelector_.IsValid()) {
        CloseSelectOverlay();
        ResetSelection();
    }
    SpanNodeFission(spanNode);

    return spanIndex;
}

void RichEditorPattern::SpanNodeFission(RefPtr<SpanNode>& spanNode)
{
    auto spanItem = spanNode->GetSpanItem();
    auto content = StringUtils::ToWstring(spanItem->content);
    auto contentLen = content.length();
    auto spanStart = spanItem->position - contentLen;
    for (size_t i = 0; i < content.length(); i++) {
        auto character = content[i];
        if (character == '\n') {
            auto charPosition = spanStart + i;
            TextSpanSplit(static_cast<int32_t>(charPosition + 1));
        }
    }
}

void RichEditorPattern::DeleteSpans(const RangeOptions& options)
{
    int32_t start = 0;
    int32_t end = 0;
    auto length = GetTextContentLength();
    start = (!options.start.has_value()) ? 0 : options.start.value();
    end = (!options.end.has_value()) ? length : options.end.value();
    if (start > end) {
        auto value = start;
        start = end;
        end = value;
    }
    start = std::max(0, start);
    end = std::min(length, end);
    if (start > length || end < 0 || start == end) {
        return;
    }

    auto startInfo = GetSpanPositionInfo(start);
    auto endInfo = GetSpanPositionInfo(end - 1);
    if (startInfo.spanIndex_ == endInfo.spanIndex_) {
        DeleteSpanByRange(start, end, startInfo);
    } else {
        DeleteSpansByRange(start, end, startInfo, endInfo);
    }
    if (textSelector_.IsValid()) {
        SetCaretPosition(textSelector_.GetTextStart());
        CloseSelectOverlay();
        ResetSelection();
    }
    SetCaretOffset(start);
    ResetSelection();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto childrens = host->GetChildren();
    if (childrens.empty() || GetTextContentLength() == 0) {
        SetCaretPosition(0);
    }
}

void RichEditorPattern::DeleteSpanByRange(int32_t start, int32_t end, SpanPositionInfo info)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto childrens = host->GetChildren();
    auto it = childrens.begin();
    std::advance(it, info.spanIndex_);
    if (start == info.spanStart_ && end == info.spanEnd_) {
        ClearContent(*it);
        host->RemoveChild(*it);
    } else {
        auto spanNode = DynamicCast<SpanNode>(*it);
        CHECK_NULL_VOID(spanNode);
        auto spanItem = spanNode->GetSpanItem();
        auto beforStr = StringUtils::ToWstring(spanItem->content).substr(0, start - info.spanStart_);
        auto endStr = StringUtils::ToWstring(spanItem->content).substr(end - info.spanStart_);
        std::wstring result = beforStr + endStr;
        auto str = StringUtils::ToString(result);
        spanNode->UpdateContent(str);
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
}

void RichEditorPattern::DeleteSpansByRange(
    int32_t start, int32_t end, SpanPositionInfo startInfo, SpanPositionInfo endInfo)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto childrens = host->GetChildren();
    auto itStart = childrens.begin();
    std::advance(itStart, startInfo.spanIndex_);
    auto saveStartSpan = (start == startInfo.spanStart_) ? 0 : 1;
    if (saveStartSpan) {
        auto spanNodeStart = DynamicCast<SpanNode>(*itStart);
        CHECK_NULL_VOID(spanNodeStart);
        auto spanItemStart = spanNodeStart->GetSpanItem();
        auto beforStr = StringUtils::ToWstring(spanItemStart->content).substr(0, start - startInfo.spanStart_);
        auto strStart = StringUtils::ToString(beforStr);
        spanNodeStart->UpdateContent(strStart);
    }
    auto itEnd = childrens.begin();
    std::advance(itEnd, endInfo.spanIndex_);
    auto delEndSpan = (end == endInfo.spanEnd_) ? 1 : 0;
    if (!delEndSpan) {
        auto spanNodeEnd = DynamicCast<SpanNode>(*itEnd);
        CHECK_NULL_VOID(spanNodeEnd);
        auto spanItemEnd = spanNodeEnd->GetSpanItem();
        auto endStr =
            StringUtils::ToWstring(spanItemEnd->content).substr(end - endInfo.spanStart_, endInfo.spanEnd_ - end);
        auto strEnd = StringUtils::ToString(endStr);
        spanNodeEnd->UpdateContent(strEnd);
    }
    auto startIter = childrens.begin();
    std::advance(startIter, startInfo.spanIndex_ + saveStartSpan);
    auto endIter = childrens.begin();
    std::advance(endIter, endInfo.spanIndex_);
    for (auto iter = startIter; iter != endIter; ++iter) {
        ClearContent(*iter);
        host->RemoveChild(*iter);
    }
    if (delEndSpan) {
        ClearContent(*endIter);
        host->RemoveChild(*endIter);
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
}

std::u16string RichEditorPattern::GetLeftTextOfCursor(int32_t number)
{
    if (number > caretPosition_) {
        number = caretPosition_;
    }
    auto start = caretPosition_;
    if (IsSelected()) {
        start = std::min(textSelector_.GetStart(), textSelector_.GetEnd());
    }
    auto stringText = GetSelectedText(start - number, start);
    return StringUtils::Str8ToStr16(stringText);
}

std::u16string RichEditorPattern::GetRightTextOfCursor(int32_t number)
{
    auto end = caretPosition_;
    if (IsSelected()) {
        end = std::max(textSelector_.GetStart(), textSelector_.GetEnd());
    }
    auto stringText = GetSelectedText(end, end + number);
    return StringUtils::Str8ToStr16(stringText);
}

int32_t RichEditorPattern::GetTextIndexAtCursor()
{
    return caretPosition_;
}

void RichEditorPattern::ClearContent(const RefPtr<UINode>& child)
{
    CHECK_NULL_VOID(child);
    if (child->GetTag() == V2::SPAN_ETS_TAG) {
        auto spanNode = DynamicCast<SpanNode>(child);
        if (spanNode) {
            spanNode->UpdateContent("");
            spanNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        }
    }
}

SpanPositionInfo RichEditorPattern::GetSpanPositionInfo(int32_t position)
{
    SpanPositionInfo spanPositionInfo(-1, -1, -1, -1);
    if (!spans_.empty()) {
        position = std::clamp(position, 0, GetTextContentLength());
        // find the spanItem where the position is
        auto it = std::find_if(spans_.begin(), spans_.end(), [position](const RefPtr<SpanItem>& spanItem) {
            return (spanItem->position - static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length()) <=
                       position) &&
                   (position < spanItem->position);
        });
        // the position is at the end
        if (it == spans_.end()) {
            return spanPositionInfo;
        }

        spanPositionInfo.spanIndex_ = std::distance(spans_.begin(), it);
        auto contentLen = StringUtils::ToWstring((*it)->content).length();
        spanPositionInfo.spanStart_ = (*it)->position - contentLen;
        spanPositionInfo.spanEnd_ = (*it)->position;
        spanPositionInfo.spanOffset_ = position - spanPositionInfo.spanStart_;
    }
    return spanPositionInfo;
}

void RichEditorPattern::CopyTextSpanStyle(RefPtr<SpanNode>& source, RefPtr<SpanNode>& target)
{
    CHECK_NULL_VOID(source);
    CHECK_NULL_VOID(target);

    if (source->HasFontSize()) {
        target->UpdateFontSize(source->GetFontSizeValue(Dimension()));
        target->AddPropertyInfo(PropertyInfo::FONTSIZE);
    }

    if (source->HasTextColor()) {
        target->UpdateTextColor(source->GetTextColorValue(Color::BLACK));
        target->AddPropertyInfo(PropertyInfo::FONTCOLOR);
    }

    if (source->HasItalicFontStyle()) {
        target->UpdateItalicFontStyle(source->GetItalicFontStyleValue(OHOS::Ace::FontStyle::NORMAL));
        target->AddPropertyInfo(PropertyInfo::FONTSTYLE);
    }

    if (source->HasFontWeight()) {
        target->UpdateFontWeight(source->GetFontWeightValue(FontWeight::NORMAL));
        target->AddPropertyInfo(PropertyInfo::FONTWEIGHT);
    }

    if (source->HasFontFamily()) {
        target->UpdateFontFamily(source->GetFontFamilyValue({ "HarmonyOS Sans" }));
        target->AddPropertyInfo(PropertyInfo::FONTFAMILY);
    }

    if (source->HasTextDecoration()) {
        target->UpdateTextDecoration(source->GetTextDecorationValue(TextDecoration::NONE));
        target->AddPropertyInfo(PropertyInfo::TEXTDECORATION);
    }

    if (source->HasTextDecorationColor()) {
        target->UpdateTextDecorationColor(source->GetTextDecorationColorValue(Color::BLACK));
        target->AddPropertyInfo(PropertyInfo::NONE);
    }

    if (source->HasTextCase()) {
        target->UpdateTextCase(source->GetTextCaseValue(TextCase::NORMAL));
        target->AddPropertyInfo(PropertyInfo::TEXTCASE);
    }

    if (source->HasLetterSpacing()) {
        target->UpdateLetterSpacing(source->GetLetterSpacingValue(Dimension()));
        target->AddPropertyInfo(PropertyInfo::LETTERSPACE);
    }

    if (source->HasLineHeight()) {
        target->UpdateLineHeight(source->GetLineHeightValue(Dimension()));
        target->AddPropertyInfo(PropertyInfo::LINEHEIGHT);
    }

    if (source->HasTextAlign()) {
        target->UpdateTextAlign(source->GetTextAlignValue(TextAlign::LEFT));
        target->AddPropertyInfo(PropertyInfo::TEXT_ALIGN);
    }

    if (source->HasLeadingMargin()) {
        target->UpdateLeadingMargin(source->GetLeadingMarginValue({}));
        target->AddPropertyInfo(PropertyInfo::LEADING_MARGIN);
    }
}

int32_t RichEditorPattern::TextSpanSplit(int32_t position)
{
    int32_t spanIndex = 0;
    int32_t spanStart = 0;
    int32_t spanOffset = 0;

    if (spans_.empty()) {
        return -1;
    }

    auto positionInfo = GetSpanPositionInfo(position);
    spanIndex = positionInfo.spanIndex_;
    spanStart = positionInfo.spanStart_;
    spanOffset = positionInfo.spanOffset_;

    if (spanOffset == 0 || spanOffset == -1) {
        return spanIndex;
    }

    auto host = GetHost();
    CHECK_NULL_RETURN(host, -1);
    auto it = host->GetChildren().begin();
    std::advance(it, spanIndex);

    auto spanNode = DynamicCast<SpanNode>(*it);
    CHECK_NULL_RETURN(spanNode, -1);
    auto spanItem = spanNode->GetSpanItem();
    auto newContent = StringUtils::ToWstring(spanItem->content).substr(spanOffset);
    auto deleteContent = StringUtils::ToWstring(spanItem->content).substr(0, spanOffset);

    auto* stack = ViewStackProcessor::GetInstance();
    CHECK_NULL_RETURN(stack, -1);
    auto nodeId = stack->ClaimNodeId();
    auto newSpanNode = SpanNode::GetOrCreateSpanNode(nodeId);
    CHECK_NULL_RETURN(newSpanNode, -1);

    auto newSpanItem = newSpanNode->GetSpanItem();
    newSpanItem->position = spanStart + spanOffset;
    auto spanIter = spans_.begin();
    std::advance(spanIter, spanIndex);
    spans_.insert(spanIter, newSpanItem);

    spanNode->UpdateContent(StringUtils::ToString(newContent));
    newSpanNode->UpdateContent(StringUtils::ToString(deleteContent));

    CopyTextSpanStyle(spanNode, newSpanNode);
    newSpanNode->MountToParent(host, spanIndex);

    return spanIndex + 1;
}

int32_t RichEditorPattern::GetTextContentLength()
{
    if (!spans_.empty()) {
        auto it = spans_.rbegin();
        return (*it)->position;
    }
    return 0;
}

int32_t RichEditorPattern::GetCaretPosition()
{
    return caretPosition_;
}

bool RichEditorPattern::SetCaretOffset(int32_t caretPosition)
{
    bool success = false;
    success = SetCaretPosition(caretPosition);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    if (focusHub->IsCurrentFocus()) {
        StartTwinkling();
    }
    return success;
}

OffsetF RichEditorPattern::CalcCursorOffsetByPosition(int32_t position, float& selectLineHeight)
{
    selectLineHeight = 0.0f;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, OffsetF(0, 0));
    auto pipeline = host->GetContext();
    CHECK_NULL_RETURN(pipeline, OffsetF(0, 0));
    auto rootOffset = pipeline->GetRootRect().GetOffset();
    auto textPaintOffset = GetTextRect().GetOffset() - OffsetF(0.0f, std::min(baselineOffset_, 0.0f));
    auto startOffset = paragraphs_.ComputeCursorOffset(position, selectLineHeight);
    auto children = host->GetChildren();
    if (NearZero(selectLineHeight)) {
        if (children.empty() || GetTextContentLength() == 0) {
            float caretHeight = DynamicCast<RichEditorOverlayModifier>(overlayMod_)->GetCaretHeight();
            return textPaintOffset - rootOffset - OffsetF(0.0f, caretHeight / 2.0f);
        }
        if (std::all_of(children.begin(), children.end(), [](RefPtr<UINode>& node) {
                CHECK_NULL_RETURN(node, false);
                return (node->GetTag() == V2::IMAGE_ETS_TAG);
            })) {
            bool isTail = false;
            auto it = children.begin();
            if (position >= static_cast<int32_t>(children.size())) {
                std::advance(it, (static_cast<int32_t>(children.size()) - 1));
                isTail = true;
            } else {
                std::advance(it, position);
            }
            if (it == children.end()) {
                return startOffset;
            }
            auto imageNode = DynamicCast<FrameNode>(*it);
            if (imageNode) {
                auto geometryNode = imageNode->GetGeometryNode();
                CHECK_NULL_RETURN(geometryNode, OffsetF(0.0f, 0.0f));
                startOffset = geometryNode->GetMarginFrameOffset();
                selectLineHeight = geometryNode->GetMarginFrameSize().Height();
                startOffset += isTail ? OffsetF(geometryNode->GetMarginFrameSize().Width(), 0.0f) : OffsetF(0.0f, 0.0f);
            }
            return startOffset;
        }
    }
    return startOffset + textPaintOffset - rootOffset;
}

bool RichEditorPattern::SetCaretPosition(int32_t pos)
{
    auto lastCaretPosition = caretPosition_;
    caretPosition_ = std::clamp(pos, 0, GetTextContentLength());
    if (caretPosition_ == pos) {
        return true;
    }
    caretPosition_ = lastCaretPosition;
    return false;
}

bool RichEditorPattern::GetCaretVisible() const
{
    return caretVisible_;
}

void RichEditorPattern::SetUpdateSpanStyle(struct UpdateSpanStyle updateSpanStyle)
{
    updateSpanStyle_ = updateSpanStyle;
}

void RichEditorPattern::SetTypingStyle(struct UpdateSpanStyle typingStyle, TextStyle textStyle)
{
    typingStyle_ = typingStyle;
    typingTextStyle_ = textStyle;
}

void RichEditorPattern::UpdateTextStyle(
    RefPtr<SpanNode>& spanNode, struct UpdateSpanStyle updateSpanStyle, TextStyle textStyle)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (updateSpanStyle.updateTextColor.has_value()) {
        spanNode->UpdateTextColor(textStyle.GetTextColor());
        spanNode->AddPropertyInfo(PropertyInfo::FONTCOLOR);
    }
    if (updateSpanStyle.updateFontSize.has_value()) {
        spanNode->UpdateFontSize(textStyle.GetFontSize());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSIZE);
    }
    if (updateSpanStyle.updateItalicFontStyle.has_value()) {
        spanNode->UpdateItalicFontStyle(textStyle.GetFontStyle());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSTYLE);
    }
    if (updateSpanStyle.updateFontWeight.has_value()) {
        spanNode->UpdateFontWeight(textStyle.GetFontWeight());
        spanNode->AddPropertyInfo(PropertyInfo::FONTWEIGHT);
    }
    if (updateSpanStyle.updateFontFamily.has_value()) {
        spanNode->UpdateFontFamily(textStyle.GetFontFamilies());
        spanNode->AddPropertyInfo(PropertyInfo::FONTFAMILY);
    }
    if (updateSpanStyle.updateTextDecoration.has_value()) {
        spanNode->UpdateTextDecoration(textStyle.GetTextDecoration());
        spanNode->AddPropertyInfo(PropertyInfo::TEXTDECORATION);
    }
    if (updateSpanStyle.updateTextDecorationColor.has_value()) {
        spanNode->UpdateTextDecorationColor(textStyle.GetTextDecorationColor());
        spanNode->AddPropertyInfo(PropertyInfo::NONE);
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
}

bool RichEditorPattern::HasSameTypingStyle(const RefPtr<SpanNode>& spanNode)
{
    auto spanItem = spanNode->GetSpanItem();
    CHECK_NULL_RETURN(spanItem, false);
    auto spanTextStyle = spanItem->GetTextStyle();
    if (spanTextStyle.has_value() && typingTextStyle_.has_value()) {
        return spanTextStyle.value() == typingTextStyle_.value();
    } else {
        return !(spanTextStyle.has_value() || typingTextStyle_.has_value());
    }
}

void RichEditorPattern::UpdateImageStyle(RefPtr<FrameNode>& imageNode, ImageSpanAttribute imageStyle)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto imageLayoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    if (updateSpanStyle_.updateImageWidth.has_value() || updateSpanStyle_.updateImageHeight.has_value()) {
        imageLayoutProperty->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(imageStyle.size.value().width), CalcLength(imageStyle.size.value().height)));
    }
    if (updateSpanStyle_.updateImageFit.has_value()) {
        imageLayoutProperty->UpdateImageFit(imageStyle.objectFit.value());
    }
    if (updateSpanStyle_.updateImageVerticalAlign.has_value()) {
        imageLayoutProperty->UpdateVerticalAlign(imageStyle.verticalAlign.value());
    }
    imageNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    imageNode->MarkModifyDone();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
}

void RichEditorPattern::UpdateSpanStyle(
    int32_t start, int32_t end, const TextStyle& textStyle, const ImageSpanAttribute& imageStyle)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    int32_t spanStart = 0;
    int32_t spanEnd = 0;
    for (auto it = host->GetChildren().begin(); it != host->GetChildren().end(); ++it) {
        auto spanNode = DynamicCast<SpanNode>(*it);
        auto imageNode = DynamicCast<FrameNode>(*it);
        if (!spanNode) {
            if (spanEnd != 0) {
                spanStart = spanEnd;
            }
            spanEnd = spanStart + 1;
        } else {
            auto spanItem = spanNode->GetSpanItem();
            spanItem->GetIndex(spanStart, spanEnd);
        }
        if (spanEnd < start) {
            continue;
        }

        if (spanStart >= start && spanEnd <= end) {
            if (spanNode) {
                UpdateTextStyle(spanNode, updateSpanStyle_, textStyle);
            } else {
                UpdateImageStyle(imageNode, imageStyle);
            }
            if (spanEnd == end) {
                break;
            }
            continue;
        }
        if (spanStart < start && start < spanEnd) {
            TextSpanSplit(start);
            --it;
            continue;
        }
        if (spanStart < end && end < spanEnd) {
            TextSpanSplit(end);
            --(--it);
            continue;
        }
        if (spanStart >= end) {
            break;
        }
    }
}

std::vector<ParagraphInfo> RichEditorPattern::GetParagraphInfo(int32_t start, int32_t end)
{
    std::vector<ParagraphInfo> res;
    auto spanNodes = GetParagraphNodes(start, end);

    auto&& firstSpan = spanNodes.front()->GetSpanItem();
    auto paraStart = firstSpan->position - StringUtils::ToWstring(firstSpan->content).length();

    for (auto it = spanNodes.begin(); it != spanNodes.end(); ++it) {
        if (it == std::prev(spanNodes.end()) || StringUtils::ToWstring((*it)->GetSpanItem()->content).back() == L'\n') {
            ParagraphInfo info;
            auto lm = (*it)->GetLeadingMarginValue({});

            res.emplace_back(ParagraphInfo {
                .leadingMarginPixmap = lm.pixmap,
                .leadingMarginSize = { Dimension(lm.size.Width()).ConvertToVp(),
                    Dimension(lm.size.Height()).ConvertToVp() },
                .textAlign = static_cast<int32_t>((*it)->GetTextAlignValue(TextAlign::START)),
                .range = { paraStart, (*it)->GetSpanItem()->position },
            });
            paraStart = (*it)->GetSpanItem()->position;
        }
    }

    return res;
}

std::vector<RefPtr<SpanNode>> RichEditorPattern::GetParagraphNodes(int32_t start, int32_t end) const
{
    CHECK_NULL_RETURN(start != end, {});
    auto host = GetHost();
    CHECK_NULL_RETURN(host, {});
    CHECK_NULL_RETURN(!host->GetChildren().empty(), {});

    std::wstring content;

    // find paragraph head
    auto headIt = host->GetChildren().begin();
    int32_t spanEnd = -1;
    for (auto it = headIt; it != host->GetChildren().end(); ++it) {
        if (InstanceOf<SpanNode>(*it)) {
            auto spanNode = DynamicCast<SpanNode>(*it);
            auto&& info = spanNode->GetSpanItem();
            spanEnd = info->position;
            content = StringUtils::ToWstring(info->content);
        } else {
            // placeholder node
            ++spanEnd;
            content = L" ";
        }

        if (spanEnd > start) {
            break;
        }
        if (content.back() == L'\n') {
            headIt = std::next(it);
        }
    }

    // find paragraph tail
    auto tailIt = host->GetChildren().end();
    int32_t spanStart = -1;
    for (auto it = std::prev(tailIt);; --it) {
        if (InstanceOf<SpanNode>(*it)) {
            auto spanNode = DynamicCast<SpanNode>(*it);
            auto&& info = spanNode->GetSpanItem();
            content = StringUtils::ToWstring(info->content);
            spanStart = info->position - content.length();
        } else {
            // placeholder node
            --spanStart;
            content = L" ";
        }

        if (content.back() == L'\n') {
            tailIt = std::next(it);
        }
        if (spanStart < end) {
            break;
        }
        if (it == host->GetChildren().begin()) {
            break;
        }
    }

    // filter illegal iterator
    if (headIt == host->GetChildren().end()) {
        return {};
    }

    std::vector<RefPtr<SpanNode>> res;
    // return spans in range [headIt, tailIt)
    // SPECIAL CASE: only 1 span and *headIt == *tailIt, handled by do-while loop
    do {
        auto spanNode = DynamicCast<SpanNode>(*headIt);
        if (spanNode) {
            res.emplace_back(spanNode);
        }
        ++headIt;
    } while (headIt != host->GetChildren().end() && headIt != tailIt);

    return res;
}

void RichEditorPattern::UpdateParagraphStyle(int32_t start, int32_t end, const struct UpdateParagraphStyle& style)
{
    auto spanNodes = GetParagraphNodes(start, end);
    for (const auto& spanNode : spanNodes) {
        if (style.textAlign.has_value()) {
            spanNode->UpdateTextAlign(*style.textAlign);
        }
        if (style.leadingMargin.has_value()) {
            spanNode->UpdateLeadingMargin(*style.leadingMargin);
        }
    }
}

void RichEditorPattern::ScheduleCaretTwinkling()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
    CHECK_NULL_VOID(context);

    if (!context->GetTaskExecutor()) {
        LOGW("context has no task executor.");
        return;
    }

    auto weak = WeakClaim(this);
    caretTwinklingTask_.Reset([weak] {
        auto client = weak.Upgrade();
        CHECK_NULL_VOID(client);
        client->OnCaretTwinkling();
    });
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostDelayedTask(caretTwinklingTask_, TaskExecutor::TaskType::UI, RICH_EDITOR_TWINKLING_INTERVAL_MS);
}

void RichEditorPattern::StartTwinkling()
{
    caretTwinklingTask_.Cancel();
    caretVisible_ = true;
    GetHost()->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    ScheduleCaretTwinkling();
}

void RichEditorPattern::OnCaretTwinkling()
{
    caretTwinklingTask_.Cancel();
    caretVisible_ = !caretVisible_;
    GetHost()->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    ScheduleCaretTwinkling();
}

void RichEditorPattern::StopTwinkling()
{
    caretTwinklingTask_.Cancel();
    if (caretVisible_) {
        caretVisible_ = false;
        GetHost()->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

void RichEditorPattern::HandleClickEvent(GestureEvent& info)
{
    if (textSelector_.IsValid() && !isMouseSelect_) {
        CloseSelectOverlay();
        ResetSelection();
    }
    UseHostToUpdateTextFieldManager();
    auto contentRect = GetTextRect();
    contentRect.SetTop(contentRect.GetY() - std::min(baselineOffset_, 0.0f));
    contentRect.SetHeight(contentRect.Height() - std::max(baselineOffset_, 0.0f));
    Offset textOffset = { info.GetLocalLocation().GetX() - contentRect.GetX(),
        info.GetLocalLocation().GetY() - contentRect.GetY() };
    auto position = paragraphs_.GetIndex(textOffset);
    auto focusHub = GetHost()->GetOrCreateFocusHub();
    if (focusHub) {
        if (focusHub->RequestFocusImmediately()) {
            float caretHeight = 0.0f;
            SetCaretPosition(position);
            OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
            CHECK_NULL_VOID(overlayMod_);
            DynamicCast<RichEditorOverlayModifier>(overlayMod_)->SetCaretOffsetAndHeight(caretOffset, caretHeight);
            StartTwinkling();
            if (overlayMod_) {
                RequestKeyboard(false, true, true);
            }
        } else {
            LOGE("request focus fail");
        }
    }
}

void RichEditorPattern::InitClickEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID(!clickEventInitialized_);
    auto clickCallback = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleClickEvent(info);
    };
    auto clickListener = MakeRefPtr<ClickEvent>(std::move(clickCallback));
    gestureHub->AddClickEvent(clickListener);
    clickEventInitialized_ = true;
}

void RichEditorPattern::InitFocusEvent(const RefPtr<FocusHub>& focusHub)
{
    CHECK_NULL_VOID(!focusEventInitialized_);
    auto focusTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleFocusEvent();
    };
    focusHub->SetOnFocusInternal(focusTask);
    auto blurTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleBlurEvent();
    };
    focusHub->SetOnBlurInternal(blurTask);
    focusEventInitialized_ = true;
    auto keyTask = [weak = WeakClaim(this)](const KeyEvent& keyEvent) -> bool {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->OnKeyEvent(keyEvent);
    };
    focusHub->SetOnKeyEventInternal(std::move(keyTask));
}

void RichEditorPattern::HandleBlurEvent()
{
    StopTwinkling();
    CloseKeyboard(true);
    if (textSelector_.IsValid()) {
        CloseSelectOverlay();
        ResetSelection();
    }
}

void RichEditorPattern::HandleFocusEvent()
{
    UseHostToUpdateTextFieldManager();
    StartTwinkling();
    if (!usingMouseRightButton_) {
        RequestKeyboard(false, true, true);
    }
}

void RichEditorPattern::UseHostToUpdateTextFieldManager()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
    CHECK_NULL_VOID(context);
    auto globalOffset = host->GetPaintRectOffset() - context->GetRootRect().GetOffset();
    UpdateTextFieldManager(Offset(globalOffset.GetX(), globalOffset.GetY()), frameRect_.Height());
}

void RichEditorPattern::OnVisibleChange(bool isVisible)
{
    TextPattern::OnVisibleChange(isVisible);
    StopTwinkling();
    CloseKeyboard(true);
}

bool RichEditorPattern::CloseKeyboard(bool forceClose)
{
    if (forceClose) {
        if (customKeyboardBuilder_ && isCustomKeyboardAttached_) {
            return CloseCustomKeyboard();
        }
#if defined(ENABLE_STANDARD_INPUT)
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
        if (!imeAttached_) {
            return false;
        }
#endif
        auto inputMethod = MiscServices::InputMethodController::GetInstance();
        CHECK_NULL_RETURN(inputMethod, false);
        inputMethod->HideTextInput();
        inputMethod->Close();
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
        imeAttached_ = false;
#endif
#else
        if (HasConnection()) {
            connection_->Close(GetInstanceId());
            connection_ = nullptr;
        }
#endif
        return true;
    }
    return false;
}

void RichEditorPattern::HandleLongPress(GestureEvent& info)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    if (BetweenSelectedPosition(info.GetGlobalLocation())) {
        dragBoxes_ = GetTextBoxes();
        // prevent long press event from being triggered when dragging
#ifdef ENABLE_DRAG_FRAMEWORK
        gestureHub->SetIsTextDraggable(true);
#endif
        return;
    }
#ifdef ENABLE_DRAG_FRAMEWORK
    gestureHub->SetIsTextDraggable(false);
#endif
    if (isMousePressed_) {
        return;
    }
    auto textPaintOffset = contentRect_.GetOffset() - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    Offset textOffset = { info.GetLocalLocation().GetX() - textPaintOffset.GetX(),
        info.GetLocalLocation().GetY() - textPaintOffset.GetY() };
    InitSelection(textOffset);
    auto selectStart = std::min(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto selectEnd = std::max(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto textSelectInfo = GetSpansInfo(selectStart, selectEnd, GetSpansMethod::ONSELECT);
    UpdateSelectionType(textSelectInfo);
    CalculateHandleOffsetAndShowOverlay();
    CloseSelectOverlay();
    ShowSelectOverlay(textSelector_.firstHandle, textSelector_.secondHandle);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    if (!textSelectInfo.GetSelection().resultObjects.empty()) {
        eventHub->FireOnSelect(&textSelectInfo);
    }
    SetCaretPosition(std::min(selectEnd, GetTextContentLength()));
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->RequestFocusImmediately();
    if (overlayMod_) {
        RequestKeyboard(false, true, true);
    }
    StopTwinkling();
}

void RichEditorPattern::HandleOnSelectAll()
{
    auto textSize = static_cast<int32_t>(GetWideText().length()) + imageCount_;
    textSelector_.Update(0, textSize);
    CalculateHandleOffsetAndShowOverlay();
    CloseSelectOverlay();
    ShowSelectOverlay(textSelector_.firstHandle, textSelector_.secondHandle, true);
    selectMenuInfo_.showCopyAll = false;
    selectOverlayProxy_->UpdateSelectMenuInfo(selectMenuInfo_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    FireOnSelect(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
    SetCaretPosition(textSize);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RichEditorPattern::InitLongPressEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID(!longPressEvent_);
    auto longPressCallback = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleLongPress(info);
    };
    longPressEvent_ = MakeRefPtr<LongPressEvent>(std::move(longPressCallback));
    gestureHub->SetLongPressEvent(longPressEvent_);

    auto onTextSelectorChange = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto frameNode = pattern->GetHost();
        CHECK_NULL_VOID(frameNode);
        frameNode->OnAccessibilityEvent(AccessibilityEventType::TEXT_SELECTION_UPDATE);
    };
    textSelector_.SetOnAccessibility(std::move(onTextSelectorChange));
}

#ifdef ENABLE_DRAG_FRAMEWORK
void RichEditorPattern::InitDragDropEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureHub = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->InitDragDropEvent();
    gestureHub->SetTextDraggable(true);
    gestureHub->SetThumbnailCallback(GetThumbnailCallback());
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto onDragStart = [weakPtr = WeakClaim(this)](const RefPtr<OHOS::Ace::DragEvent>& event,
                           const std::string& extraParams) -> NG::DragDropInfo {
        NG::DragDropInfo itemInfo;
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_RETURN(pattern, itemInfo);
        pattern->timestamp_ = std::chrono::system_clock::now().time_since_epoch().count();
        auto eventHub = pattern->GetEventHub<RichEditorEventHub>();
        eventHub->SetTimestamp(pattern->GetTimestamp());
        CHECK_NULL_RETURN(eventHub, itemInfo);
        return pattern->OnDragStart(event);
    };
    eventHub->SetOnDragStart(std::move(onDragStart));
    auto onDragMove = [weakPtr = WeakClaim(this)](
                          const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) {
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnDragMove(event);
    };
    eventHub->SetOnDragMove(std::move(onDragMove));
    auto onDragEnd = [weakPtr = WeakClaim(this), scopeId = Container::CurrentId()](
                         const RefPtr<OHOS::Ace::DragEvent>& event) {
        ContainerScope scope(scopeId);
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnDragEnd();
    };
    eventHub->SetOnDragEnd(std::move(onDragEnd));
}

NG::DragDropInfo RichEditorPattern::OnDragStart(const RefPtr<OHOS::Ace::DragEvent>& event)
{
    NG::DragDropInfo itemInfo;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, itemInfo);
    auto selectStart = textSelector_.GetTextStart();
    auto selectEnd = textSelector_.GetTextEnd();
    auto textSelectInfo = GetSpansInfo(selectStart, selectEnd, GetSpansMethod::ONSELECT);
    dragResultObjects_ = textSelectInfo.GetSelection().resultObjects;
    if (dragResultObjects_.empty()) {
        return itemInfo;
    }
    RefPtr<UnifiedData> unifiedData = UdmfClient::GetInstance()->CreateUnifiedData();
    auto resultProcessor = [unifiedData, weak = WeakClaim(this)](const ResultObject& result) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (result.type == RichEditorSpanType::TYPESPAN) {
            auto data = pattern->GetSelectedSpanText(StringUtils::ToWstring(result.valueString),
                result.offsetInSpan[RichEditorSpanRange::RANGESTART],
                result.offsetInSpan[RichEditorSpanRange::RANGEEND]);
            UdmfClient::GetInstance()->AddPlainTextRecord(unifiedData, data);
            return;
        }
        if (result.type == RichEditorSpanType::TYPEIMAGE) {
            if (result.valuePixelMap) {
                const uint8_t* pixels = result.valuePixelMap->GetPixels();
                CHECK_NULL_VOID(pixels);
                int32_t length = result.valuePixelMap->GetByteCount();
                std::vector<uint8_t> data(pixels, pixels + length);
                UdmfClient::GetInstance()->AddPixelMapRecord(unifiedData, data);
            } else {
                UdmfClient::GetInstance()->AddImageRecord(unifiedData, result.valueString);
            }
        }
    };
    for (const auto& resultObj : dragResultObjects_) {
        resultProcessor(resultObj);
    }
    event->SetData(unifiedData);

    AceEngineExt::GetInstance().DragStartExt();

    StopTwinkling();
    CloseKeyboard(true);
    CloseSelectOverlay();
    ResetSelection();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    return itemInfo;
}

void RichEditorPattern::OnDragEnd()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (dragResultObjects_.empty()) {
        return;
    }
    UpdateSpanItemDragStatus(dragResultObjects_, false);
    dragResultObjects_.clear();
    StartTwinkling();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void RichEditorPattern::OnDragMove(const RefPtr<OHOS::Ace::DragEvent>& event)
{
    auto focusHub = GetHost()->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->RequestFocusImmediately();
    auto touchX = event->GetX();
    auto touchY = event->GetY();
    auto contentRect = GetTextRect();
    contentRect.SetTop(contentRect.GetY() - std::min(baselineOffset_, 0.0f));
    Offset textOffset = { touchX - contentRect.GetX() - GetParentGlobalOffset().GetX(),
        touchY - contentRect.GetY() - GetParentGlobalOffset().GetY() };
    auto position = paragraphs_.GetIndex(textOffset);
    float caretHeight = 0.0f;
    SetCaretPosition(position);
    OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
    CHECK_NULL_VOID(overlayMod_);
    DynamicCast<RichEditorOverlayModifier>(overlayMod_)->SetCaretOffsetAndHeight(caretOffset, caretHeight);
    StartTwinkling();
}

void RichEditorPattern::UpdateSpanItemDragStatus(const std::list<ResultObject>& resultObjects, bool isDragging)
{
    if (resultObjects.empty()) {
        return;
    }
    auto dragStatusUpdateAction = [weakPtr = WeakClaim(this), isDragging](const ResultObject& resultObj) {
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto it = pattern->spans_.begin();
        std::advance(it, resultObj.spanPosition.spanIndex);
        auto spanItem = *it;
        CHECK_NULL_VOID(spanItem);
        if (resultObj.type == RichEditorSpanType::TYPESPAN) {
            if (isDragging) {
                spanItem->StartDrag(resultObj.offsetInSpan[RichEditorSpanRange::RANGESTART],
                    resultObj.offsetInSpan[RichEditorSpanRange::RANGEEND]);
            } else {
                spanItem->EndDrag();
            }
            return;
        }

        if (resultObj.type == RichEditorSpanType::TYPEIMAGE) {
            auto imageNode = DynamicCast<FrameNode>(pattern->GetChildByIndex(resultObj.spanPosition.spanIndex));
            CHECK_NULL_VOID(imageNode);
            auto renderContext = imageNode->GetRenderContext();
            CHECK_NULL_VOID(renderContext);
            renderContext->UpdateOpacity(isDragging ? (double)DRAGGED_TEXT_OPACITY / 255 : 1);
            imageNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        }
    };
    for (const auto& resultObj : resultObjects) {
        dragStatusUpdateAction(resultObj);
    }
}
#endif // ENABLE_DRAG_FRAMEWORK

bool RichEditorPattern::SelectOverlayIsOn()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    CHECK_NULL_RETURN(selectOverlayProxy_, false);
    auto overlayId = selectOverlayProxy_->GetSelectOverlayId();
    return pipeline->GetSelectOverlayManager()->HasSelectOverlay(overlayId);
}

void RichEditorPattern::UpdateEditingValue(const std::shared_ptr<TextEditingValue>& value, bool needFireChangeEvent)
{
    InsertValue(value->text);
}

void RichEditorPattern::PerformAction(TextInputAction action, bool forceCloseKeyboard)
{
    InsertValue("\n");
}

void RichEditorPattern::InitMouseEvent()
{
    CHECK_NULL_VOID(!mouseEventInitialized_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto inputHub = eventHub->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputHub);

    auto mouseTask = [weak = WeakClaim(this)](MouseInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleMouseEvent(info);
    };
    auto mouseEvent = MakeRefPtr<InputEvent>(std::move(mouseTask));
    inputHub->AddOnMouseEvent(mouseEvent);
    auto hoverTask = [weak = WeakClaim(this)](bool isHover) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->OnHover(isHover);
        }
    };
    auto hoverEvent = MakeRefPtr<InputEvent>(std::move(hoverTask));
    inputHub->AddOnHoverEvent(hoverEvent);
    mouseEventInitialized_ = true;
}

void RichEditorPattern::OnHover(bool isHover)
{
    auto frame = GetHost();
    CHECK_NULL_VOID(frame);
    auto frameId = frame->GetId();
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    if (isHover) {
        pipeline->SetMouseStyleHoldNode(frameId);
        pipeline->ChangeMouseStyle(frameId, MouseFormat::TEXT_CURSOR);
    } else {
        pipeline->ChangeMouseStyle(frameId, MouseFormat::DEFAULT);
        pipeline->FreeMouseStyleHoldNode(frameId);
    }
}

bool RichEditorPattern::RequestKeyboard(bool isFocusViewChanged, bool needStartTwinkling, bool needShowSoftKeyboard)
{
    auto host = GetHost();
    auto context = host->GetContext();
    CHECK_NULL_RETURN(context, false);
    CHECK_NULL_RETURN(needShowSoftKeyboard, false);
    if (needShowSoftKeyboard && customKeyboardBuilder_) {
        return RequestCustomKeyboard();
    }
#if defined(ENABLE_STANDARD_INPUT)
    if (!EnableStandardInput(needShowSoftKeyboard)) {
        return false;
    }
#else
    if (!UnableStandardInput(isFocusViewChanged)) {
        return false;
    }
#endif
    return true;
}

#if defined(ENABLE_STANDARD_INPUT)
bool RichEditorPattern::EnableStandardInput(bool needShowSoftKeyboard)
{
    auto host = GetHost();
    auto context = host->GetContext();
    CHECK_NULL_RETURN(context, false);
    MiscServices::Configuration configuration;
    configuration.SetEnterKeyType(static_cast<MiscServices::EnterKeyType>(static_cast<int32_t>(TextInputAction::DONE)));
    configuration.SetTextInputType(
        static_cast<MiscServices::TextInputType>(static_cast<int32_t>(TextInputType::UNSPECIFIED)));
    MiscServices::InputMethodController::GetInstance()->OnConfigurationChange(configuration);
    if (richEditTextChangeListener_ == nullptr) {
        richEditTextChangeListener_ = new OnRichEditorChangedListenerImpl(WeakClaim(this));
    }
    auto inputMethod = MiscServices::InputMethodController::GetInstance();
    CHECK_NULL_RETURN(inputMethod, false);
    auto miscTextConfig = GetMiscTextConfig();
    CHECK_NULL_RETURN(miscTextConfig.has_value(), false);
    inputMethod->Attach(richEditTextChangeListener_, needShowSoftKeyboard, miscTextConfig.value());
    if (context) {
        inputMethod->SetCallingWindow(context->GetWindowId());
    }
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    imeAttached_ = true;
#endif
    return true;
}

std::optional<MiscServices::TextConfig> RichEditorPattern::GetMiscTextConfig()
{
    auto pipeline = GetHost()->GetContext();
    CHECK_NULL_RETURN(pipeline, {});
    auto windowRect = pipeline->GetCurrentWindowRect();
    float caretHeight = 0.0f;
    OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
    if (NearZero(caretHeight)) {
        auto overlayModifier = DynamicCast<RichEditorOverlayModifier>(overlayMod_);
        caretHeight = overlayModifier ? overlayModifier->GetCaretHeight() : DEFAULT_CARET_HEIGHT;
    }
    MiscServices::CursorInfo cursorInfo { .left = caretOffset.GetX() + windowRect.Left() + parentGlobalOffset_.GetX(),
        .top = caretOffset.GetY() + windowRect.Top() + parentGlobalOffset_.GetY(),
        .width = CARET_WIDTH,
        .height = caretHeight };
    MiscServices::InputAttribute inputAttribute = { .inputPattern = (int32_t)TextInputType::UNSPECIFIED,
        .enterKeyType = (int32_t)TextInputAction::DONE };
    MiscServices::TextConfig textConfig = { .inputAttribute = inputAttribute,
        .cursorInfo = cursorInfo,
        .range = { .start = textSelector_.GetStart(), .end = textSelector_.GetEnd() },
        .windowId = pipeline->GetFocusWindowId() };
    return textConfig;
}
#else
bool RichEditorPattern::UnableStandardInput(bool isFocusViewChanged)
{
    auto host = GetHost();
    auto context = host->GetContext();
    CHECK_NULL_RETURN(context, false);
    if (HasConnection()) {
        connection_->Show(isFocusViewChanged, GetInstanceId());
        return true;
    }
    TextInputConfiguration config;
    config.type = TextInputType::UNSPECIFIED;
    config.action = TextInputAction::DONE;
    config.obscureText = false;
    connection_ =
        TextInputProxy::GetInstance().Attach(WeakClaim(this), config, context->GetTaskExecutor(), GetInstanceId());
    if (!HasConnection()) {
        return false;
    }
    TextEditingValue value;
    if (spans_.empty()) {
        value.text = textForDisplay_;
    } else {
        for (auto it = spans_.begin(); it != spans_.end(); it++) {
            if ((*it)->placeHolderIndex < 0) {
                value.text.append((*it)->content);
            } else {
                value.text.append(" ");
            }
        }
    }
    value.selection.Update(caretPosition_, caretPosition_);
    connection_->SetEditingState(value, GetInstanceId());
    connection_->Show(isFocusViewChanged, GetInstanceId());
    return true;
}
#endif

void RichEditorPattern::UpdateCaretInfoToController()
{
    CHECK_NULL_VOID(HasFocus());
    auto selectionResult = GetSpansInfo(0, GetTextContentLength(), GetSpansMethod::ONSELECT);
    auto resultObjects = selectionResult.GetSelection().resultObjects;
    std::string text = "";
    if (!resultObjects.empty()) {
        for (const auto& resultObj : resultObjects) {
            if (resultObj.type == RichEditorSpanType::TYPESPAN) {
                text += resultObj.valueString;
            }
        }
    }
#if defined(ENABLE_STANDARD_INPUT)
    auto miscTextConfig = GetMiscTextConfig();
    CHECK_NULL_VOID(miscTextConfig.has_value());
    MiscServices::CursorInfo cursorInfo = miscTextConfig.value().cursorInfo;
    LOGD("UpdateCaretInfoToController, left %{public}f, top %{public}f, width %{public}f, height %{public}f",
        cursorInfo.left, cursorInfo.top, cursorInfo.width, cursorInfo.height);
    MiscServices::InputMethodController::GetInstance()->OnCursorUpdate(cursorInfo);
    LOGD("Start %{public}d, end %{public}d", textSelector_.GetStart(), textSelector_.GetEnd());
    MiscServices::InputMethodController::GetInstance()->OnSelectionChange(
        StringUtils::Str8ToStr16(text), textSelector_.GetStart(), textSelector_.GetEnd());

#else
    if (HasConnection()) {
        TextEditingValue editingValue;
        editingValue.text = text;
        editingValue.hint = "";
        editingValue.selection.Update(textSelector_.baseOffset, textSelector_.destinationOffset);
        connection_->SetEditingState(editingValue, GetInstanceId());
    }
#endif
}

bool RichEditorPattern::HasConnection() const
{
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    return imeAttached_;
#else
    return connection_ != nullptr;
#endif
}

bool RichEditorPattern::RequestCustomKeyboard()
{
    if (isCustomKeyboardAttached_) {
        return true;
    }
    CHECK_NULL_RETURN(customKeyboardBuilder_, false);
    auto frameNode = GetHost();
    CHECK_NULL_RETURN(frameNode, false);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto overlayManager = pipeline->GetOverlayManager();
    CHECK_NULL_RETURN(overlayManager, false);
    overlayManager->BindKeyboard(customKeyboardBuilder_, frameNode->GetId());
    isCustomKeyboardAttached_ = true;
    return true;
}

bool RichEditorPattern::CloseCustomKeyboard()
{
    auto frameNode = GetHost();
    CHECK_NULL_RETURN(frameNode, false);

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto overlayManager = pipeline->GetOverlayManager();
    CHECK_NULL_RETURN(overlayManager, false);
    overlayManager->CloseKeyboard(frameNode->GetId());
    isCustomKeyboardAttached_ = false;
    return true;
}

void RichEditorPattern::InsertValue(const std::string& insertValue)
{
    bool isSelector = false;
    if (textSelector_.IsValid()) {
        SetCaretPosition(textSelector_.GetTextStart());
        isSelector = true;
    }

    std::string insertValueTemp = insertValue;
    bool isLineSeparator = false;
    if (insertValueTemp == std::string("\n")) {
        isLineSeparator = true;
    }

    if (insertValueTemp == std::string(" ")) {
        CHECK_NULL_VOID(overlayMod_);
        auto caretOffset = DynamicCast<RichEditorOverlayModifier>(overlayMod_)->GetCaretOffset();
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto geometryNode = host->GetGeometryNode();
        CHECK_NULL_VOID(geometryNode);
        const auto& contentSize = geometryNode->GetContent()->GetRect().GetSize();
        if (caretOffset.GetX() >= contentSize.Width()) {
            LOGD("replace space with newline character");
            insertValueTemp = std::string("\n ");
        }
    }

    auto isInsert = BeforeIMEInsertValue(insertValueTemp);
    CHECK_NULL_VOID(isInsert);
    TextInsertValueInfo info;
    CalcInsertValueObj(info);
    if (isSelector) {
        DeleteForward(textSelector_.GetTextEnd() - textSelector_.GetTextStart());
        CloseSelectOverlay();
        ResetSelection();
    }
    if (!caretVisible_) {
        StartTwinkling();
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    RefPtr<SpanNode> spanNode = DynamicCast<SpanNode>(host->GetChildAtIndex(info.GetSpanIndex()));
    if (spanNode == nullptr && info.GetSpanIndex() == 0) {
        CreateTextSpanNode(spanNode, info, insertValueTemp);
        return;
    }
    if (spanNode == nullptr && info.GetSpanIndex() != 0) {
        auto spanNodeBefore = DynamicCast<SpanNode>(host->GetChildAtIndex(info.GetSpanIndex() - 1));
        if (spanNodeBefore == nullptr) {
            CreateTextSpanNode(spanNode, info, insertValueTemp);
            return;
        }
        if (typingStyle_.has_value() && !HasSameTypingStyle(spanNodeBefore)) {
            CreateTextSpanNode(spanNode, info, insertValueTemp);
            return;
        }
        auto spanNodeGet = InsertValueToBeforeSpan(spanNodeBefore, insertValueTemp);
        bool isCreate = spanNodeBefore->GetId() != spanNodeGet->GetId();
        AfterIMEInsertValue(
            spanNodeGet, static_cast<int32_t>(StringUtils::ToWstring(insertValueTemp).length()), isCreate);
        return;
    }
    if (info.GetOffsetInSpan() == 0) {
        auto spanNodeBefore = DynamicCast<SpanNode>(host->GetChildAtIndex(info.GetSpanIndex() - 1));
        if (spanNodeBefore != nullptr && !IsLineSeparatorInLast(spanNodeBefore)) {
            if (typingStyle_.has_value() && !HasSameTypingStyle(spanNodeBefore)) {
                CreateTextSpanNode(spanNode, info, insertValueTemp);
                return;
            }
            auto spanNodeGet = InsertValueToBeforeSpan(spanNodeBefore, insertValueTemp);
            bool isCreate = spanNodeBefore->GetId() != spanNodeGet->GetId();
            AfterIMEInsertValue(
                spanNodeGet, static_cast<int32_t>(StringUtils::ToWstring(insertValueTemp).length()), isCreate);
            return;
        }
    }
    if (typingStyle_.has_value() && !HasSameTypingStyle(spanNode)) {
        TextSpanOptions options;
        options.value = insertValueTemp;
        options.offset = caretPosition_;
        options.style = typingTextStyle_;
        AddTextSpan(options);
        AfterIMEInsertValue(spanNode, static_cast<int32_t>(StringUtils::ToWstring(insertValueTemp).length()), true);
        return;
    }
    if (!isLineSeparator) {
        InsertValueToSpanNode(spanNode, insertValueTemp, info);
    } else {
        SpanNodeFission(spanNode, insertValueTemp, info);
    }
    AfterIMEInsertValue(spanNode, static_cast<int32_t>(StringUtils::ToWstring(insertValueTemp).length()), false);
}

bool RichEditorPattern::IsLineSeparatorInLast(RefPtr<SpanNode>& spanNode)
{
    auto spanItem = spanNode->GetSpanItem();
    auto text = spanItem->content;
    std::wstring textTemp = StringUtils::ToWstring(text);
    auto index = textTemp.find(lineSeparator);
    if (index != std::wstring::npos) {
        auto textBefore = textTemp.substr(0, index + 1);
        auto textAfter = textTemp.substr(index + 1);
        if (textAfter.empty()) {
            return true;
        }
    }
    return false;
}

void RichEditorPattern::InsertValueToSpanNode(
    RefPtr<SpanNode>& spanNode, const std::string& insertValue, const TextInsertValueInfo& info)
{
    auto spanItem = spanNode->GetSpanItem();
    CHECK_NULL_VOID(spanItem);
    auto text = spanItem->content;
    std::wstring textTemp = StringUtils::ToWstring(text);
    std::wstring insertValueTemp = StringUtils::ToWstring(insertValue);
    textTemp.insert(info.GetOffsetInSpan(), insertValueTemp);
    text = StringUtils::ToString(textTemp);
    spanNode->UpdateContent(text);
    spanItem->position += static_cast<int32_t>(StringUtils::ToWstring(insertValue).length());
}

void RichEditorPattern::SpanNodeFission(
    RefPtr<SpanNode>& spanNode, const std::string& insertValue, const TextInsertValueInfo& info)
{
    auto spanItem = spanNode->GetSpanItem();
    CHECK_NULL_VOID(spanItem);
    auto text = spanItem->content;
    std::wstring textTemp = StringUtils::ToWstring(text);
    std::wstring insertValueTemp = StringUtils::ToWstring(insertValue);
    textTemp.insert(info.GetOffsetInSpan(), insertValueTemp);

    auto index = textTemp.find(lineSeparator);
    if (index != std::wstring::npos) {
        auto textBefore = textTemp.substr(0, index + 1);
        auto textAfter = textTemp.substr(index + 1);
        text = StringUtils::ToString(textBefore);
        spanNode->UpdateContent(text);
        spanItem->position += 1 - static_cast<int32_t>(textAfter.length());
        if (!textAfter.empty()) {
            TextInsertValueInfo infoAfter;
            infoAfter.SetSpanIndex(info.GetSpanIndex() + 1);
            infoAfter.SetOffsetInSpan(0);
            auto host = GetHost();
            CHECK_NULL_VOID(host);
            auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
            RefPtr<SpanNode> spanNodeAfter = SpanNode::GetOrCreateSpanNode(nodeId);
            spanNodeAfter->MountToParent(host, infoAfter.GetSpanIndex());
            spanNodeAfter->UpdateContent(StringUtils::ToString(textAfter));
            CopyTextSpanStyle(spanNode, spanNodeAfter);
        }
    } else {
        text = StringUtils::ToString(textTemp);
        spanNode->UpdateContent(text);
        spanItem->position += static_cast<int32_t>(StringUtils::ToWstring(insertValue).length());
    }
}

RefPtr<SpanNode> RichEditorPattern::InsertValueToBeforeSpan(
    RefPtr<SpanNode>& spanNodeBefore, const std::string& insertValue)
{
    auto spanItem = spanNodeBefore->GetSpanItem();
    CHECK_NULL_RETURN(spanItem, spanNodeBefore);
    auto text = spanItem->content;
    std::wstring textTemp = StringUtils::ToWstring(text);
    std::wstring insertValueTemp = StringUtils::ToWstring(insertValue);
    textTemp.append(insertValueTemp);

    auto index = textTemp.find(lineSeparator);
    if (index != std::wstring::npos) {
        auto textBefore = textTemp.substr(0, index + 1);
        auto textAfter = textTemp.substr(index + 1);
        text = StringUtils::ToString(textBefore);
        spanNodeBefore->UpdateContent(text);
        spanItem->position += 1 - static_cast<int32_t>(textAfter.length());
        if (!textAfter.empty()) {
            auto host = GetHost();
            CHECK_NULL_RETURN(spanItem, spanNodeBefore);
            TextInsertValueInfo infoAfter;
            infoAfter.SetSpanIndex(host->GetChildIndex(spanNodeBefore) + 1);
            infoAfter.SetOffsetInSpan(0);
            auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
            RefPtr<SpanNode> spanNodeAfter = SpanNode::GetOrCreateSpanNode(nodeId);
            spanNodeAfter->MountToParent(host, infoAfter.GetSpanIndex());
            spanNodeAfter->UpdateContent(StringUtils::ToString(textAfter));
            CopyTextSpanStyle(spanNodeBefore, spanNodeAfter);
            return spanNodeAfter;
        }
    } else {
        text = StringUtils::ToString(textTemp);
        spanNodeBefore->UpdateContent(text);
        spanItem->position += static_cast<int32_t>(StringUtils::ToWstring(insertValue).length());
    }
    return spanNodeBefore;
}

void RichEditorPattern::CreateTextSpanNode(
    RefPtr<SpanNode>& spanNode, const TextInsertValueInfo& info, const std::string& insertValue, bool isIME)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
    spanNode = SpanNode::GetOrCreateSpanNode(nodeId);
    spanNode->MountToParent(host, info.GetSpanIndex());
    if (typingStyle_.has_value() && typingTextStyle_.has_value()) {
        UpdateTextStyle(spanNode, typingStyle_.value(), typingTextStyle_.value());
        auto spanItem = spanNode->GetSpanItem();
        spanItem->SetTextStyle(typingTextStyle_);
    } else {
        spanNode->UpdateFontSize(Dimension(DEFAULT_TEXT_SIZE, DimensionUnit::FP));
        spanNode->AddPropertyInfo(PropertyInfo::FONTSIZE);
    }
    spanNode->UpdateContent(insertValue);
    if (isIME) {
        AfterIMEInsertValue(spanNode, static_cast<int32_t>(StringUtils::ToWstring(insertValue).length()), true);
    }
}

bool RichEditorPattern::BeforeIMEInsertValue(const std::string& insertValue)
{
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_RETURN(eventHub, true);
    RichEditorInsertValue insertValueInfo;
    insertValueInfo.SetInsertOffset(caretPosition_);
    insertValueInfo.SetInsertValue(insertValue);
    return eventHub->FireAboutToIMEInput(insertValueInfo);
}

void RichEditorPattern::AfterIMEInsertValue(const RefPtr<SpanNode>& spanNode, int32_t insertValueLength, bool isCreate)
{
    RichEditorAbstractSpanResult retInfo;
    isTextChange_ = true;
    moveDirection_ = MoveDirection::FORWARD;
    moveLength_ += insertValueLength;
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    retInfo.SetSpanIndex(host->GetChildIndex(spanNode));
    retInfo.SetEraseLength(insertValueLength);
    retInfo.SetValue(spanNode->GetSpanItem()->content);
    auto contentLength = StringUtils::ToWstring(spanNode->GetSpanItem()->content).length();
    if (isCreate) {
        auto spanEnd = GetCaretPosition() + 1;
        auto spanStart = spanEnd - static_cast<int32_t>(contentLength);
        retInfo.SetSpanRangeStart(spanStart);
        retInfo.SetSpanRangeEnd(spanEnd);
        retInfo.SetOffsetInSpan(0);
    } else {
        auto spanEnd = spanNode->GetSpanItem()->position;
        auto spanStart = spanEnd - static_cast<int32_t>(contentLength);
        retInfo.SetSpanRangeStart(spanStart);
        retInfo.SetSpanRangeEnd(spanEnd);
        retInfo.SetOffsetInSpan(GetCaretPosition() - retInfo.GetSpanRangeStart());
    }
    retInfo.SetFontColor(spanNode->GetTextColorValue(Color::BLACK).ColorToString());
    retInfo.SetFontSize(spanNode->GetFontSizeValue(Dimension(16.0f, DimensionUnit::VP)).ConvertToVp());
    retInfo.SetFontStyle(spanNode->GetItalicFontStyleValue(OHOS::Ace::FontStyle::NORMAL));
    retInfo.SetFontWeight(static_cast<int32_t>(spanNode->GetFontWeightValue(FontWeight::NORMAL)));
    std::string fontFamilyValue;
    auto fontFamily = spanNode->GetFontFamilyValue({ "HarmonyOS Sans" });
    for (const auto& str : fontFamily) {
        fontFamilyValue += str;
    }
    retInfo.SetFontFamily(fontFamilyValue);
    retInfo.SetTextDecoration(spanNode->GetTextDecorationValue(TextDecoration::NONE));
    retInfo.SetColor(spanNode->GetTextDecorationColorValue(Color::BLACK).ColorToString());
    eventHub->FireOnIMEInputComplete(retInfo);
    int32_t spanTextLength = 0;
    for (auto& span : spans_) {
        spanTextLength += StringUtils::ToWstring(span->content).length();
        span->position = spanTextLength;
    }
}

void RichEditorPattern::DeleteBackward(int32_t length)
{
    if (textSelector_.IsValid()) {
        length = textSelector_.GetTextEnd() - textSelector_.GetTextStart();
        SetCaretPosition(textSelector_.GetTextEnd());
        CloseSelectOverlay();
        ResetSelection();
    }
    if (caretPosition_ == 0) {
        return;
    }
    RichEditorDeleteValue info;
    info.SetOffset(caretPosition_ - 1);
    info.SetRichEditorDeleteDirection(RichEditorDeleteDirection::BACKWARD);
    info.SetLength(length);
    int32_t currentPosition = std::clamp((caretPosition_ - length), 0, static_cast<int32_t>(GetTextContentLength()));
    if (!spans_.empty()) {
        CalcDeleteValueObj(currentPosition, length, info);
        auto eventHub = GetEventHub<RichEditorEventHub>();
        CHECK_NULL_VOID(eventHub);
        auto isDelete = eventHub->FireAboutToDelete(info);
        if (isDelete) {
            DeleteByDeleteValueInfo(info);
            eventHub->FireOnDeleteComplete();
        }
    }
    if (!caretVisible_) {
        StartTwinkling();
    }
}

void RichEditorPattern::DeleteForward(int32_t length)
{
    if (textSelector_.IsValid()) {
        length = textSelector_.GetTextEnd() - textSelector_.GetTextStart();
        SetCaretPosition(textSelector_.GetTextStart());
        CloseSelectOverlay();
        ResetSelection();
    }
    if (caretPosition_ == GetTextContentLength()) {
        return;
    }
    RichEditorDeleteValue info;
    info.SetOffset(caretPosition_);
    info.SetRichEditorDeleteDirection(RichEditorDeleteDirection::FORWARD);
    info.SetLength(length);
    int32_t currentPosition = caretPosition_;
    if (!spans_.empty()) {
        CalcDeleteValueObj(currentPosition, length, info);
        auto eventHub = GetEventHub<RichEditorEventHub>();
        CHECK_NULL_VOID(eventHub);
        auto isDelete = eventHub->FireAboutToDelete(info);
        if (isDelete) {
            DeleteByDeleteValueInfo(info);
            eventHub->FireOnDeleteComplete();
        }
    }
    if (!caretVisible_) {
        StartTwinkling();
    }
}

void RichEditorPattern::SetInputMethodStatus(bool keyboardShown)
{
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    imeShown_ = keyboardShown;
#endif
}

bool RichEditorPattern::CursorMoveLeft()
{
    auto caretPosition = std::clamp((caretPosition_ - 1), 0, static_cast<int32_t>(GetTextContentLength()));
    if (caretPosition_ == caretPosition) {
        return false;
    }
    caretPosition_ = caretPosition;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    return true;
}

bool RichEditorPattern::CursorMoveRight()
{
    auto caretPosition = std::clamp((caretPosition_ + 1), 0, static_cast<int32_t>(GetTextContentLength()));
    if (caretPosition_ == caretPosition) {
        return false;
    }
    caretPosition_ = caretPosition;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    return true;
}

bool RichEditorPattern::CursorMoveUp()
{
    if (static_cast<int32_t>(GetTextContentLength()) > 1) {
        float caretHeight = 0.0f;
        OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
        int32_t caretPosition = paragraphs_.GetIndex(Offset(caretOffset.GetX(), caretOffset.GetY() - caretHeight));
        caretPosition = std::clamp(caretPosition, 0, static_cast<int32_t>(GetTextContentLength()));
        if (caretPosition_ == caretPosition) {
            return false;
        }
        caretPosition_ = caretPosition;
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    return true;
}

bool RichEditorPattern::CursorMoveDown()
{
    if (static_cast<int32_t>(GetTextContentLength()) > 1) {
        float caretHeight = 0.0f;
        OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
        int32_t caretPosition = paragraphs_.GetIndex(Offset(caretOffset.GetX(), caretOffset.GetY() + caretHeight));
        caretPosition = std::clamp(caretPosition, 0, static_cast<int32_t>(GetTextContentLength()));
        if (caretPosition_ == caretPosition) {
            return false;
        }
        caretPosition_ = caretPosition;
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    return true;
}

void RichEditorPattern::CalcInsertValueObj(TextInsertValueInfo& info)
{
    if (spans_.empty()) {
        info.SetSpanIndex(0);
        info.SetOffsetInSpan(0);
    } else {
        auto it = std::find_if(spans_.begin(), spans_.end(),
            [caretPosition = caretPosition_ + moveLength_](const RefPtr<SpanItem>& spanItem) {
                return (spanItem->position - static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length()) <=
                           caretPosition) &&
                       (caretPosition < spanItem->position);
            });
        info.SetSpanIndex(std::distance(spans_.begin(), it));
        if (it == spans_.end()) {
            info.SetOffsetInSpan(0);
            return;
        }
        info.SetOffsetInSpan(
            caretPosition_ + moveLength_ - ((*it)->position - StringUtils::ToWstring((*it)->content).length()));
    }
}

void RichEditorPattern::CalcDeleteValueObj(int32_t currentPosition, int32_t length, RichEditorDeleteValue& info)
{
    auto it =
        std::find_if(spans_.begin(), spans_.end(), [caretPosition = currentPosition](const RefPtr<SpanItem>& spanItem) {
            return (spanItem->position - static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length()) <=
                       caretPosition) &&
                   (caretPosition < spanItem->position);
        });
    while (it != spans_.end() && length > 0) {
        if ((*it)->placeHolderIndex >= 0) {
            RichEditorAbstractSpanResult spanResult;
            spanResult.SetSpanIndex(std::distance(spans_.begin(), it));
            auto eraseLength = DeleteValueSetImageSpan(*it, spanResult);
            currentPosition += eraseLength;
            length -= eraseLength;
            info.SetRichEditorDeleteSpans(spanResult);
        } else {
            RichEditorAbstractSpanResult spanResult;
            spanResult.SetSpanIndex(std::distance(spans_.begin(), it));
            auto eraseLength = DeleteValueSetTextSpan(*it, currentPosition, length, spanResult);
            length -= eraseLength;
            currentPosition += eraseLength;
            info.SetRichEditorDeleteSpans(spanResult);
        }
        std::advance(it, 1);
    }
}

int32_t RichEditorPattern::DeleteValueSetImageSpan(
    const RefPtr<SpanItem>& spanItem, RichEditorAbstractSpanResult& spanResult)
{
    spanResult.SetSpanType(SpanResultType::IMAGE);
    spanResult.SetSpanRangeEnd(spanItem->position);
    spanResult.SetSpanRangeStart(spanItem->position - 1);
    spanResult.SetEraseLength(1);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, IMAGE_SPAN_LENGTH);
    auto uiNode = host->GetChildAtIndex(spanResult.GetSpanIndex());
    CHECK_NULL_RETURN(uiNode, IMAGE_SPAN_LENGTH);
    auto imageNode = AceType::DynamicCast<FrameNode>(uiNode);
    CHECK_NULL_RETURN(imageNode, IMAGE_SPAN_LENGTH);
    auto geometryNode = imageNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, IMAGE_SPAN_LENGTH);
    auto imageLayoutProperty = DynamicCast<ImageLayoutProperty>(imageNode->GetLayoutProperty());
    CHECK_NULL_RETURN(imageLayoutProperty, IMAGE_SPAN_LENGTH);
    spanResult.SetSizeWidth(geometryNode->GetMarginFrameSize().Width());
    spanResult.SetSizeHeight(geometryNode->GetMarginFrameSize().Height());
    if (!imageLayoutProperty->GetImageSourceInfo()->GetPixmap()) {
        spanResult.SetValueResourceStr(imageLayoutProperty->GetImageSourceInfo()->GetSrc());
    } else {
        spanResult.SetValuePixelMap(imageLayoutProperty->GetImageSourceInfo()->GetPixmap());
    }
    if (imageLayoutProperty->HasImageFit()) {
        spanResult.SetImageFit(imageLayoutProperty->GetImageFitValue());
    }
    if (imageLayoutProperty->HasVerticalAlign()) {
        spanResult.SetVerticalAlign(imageLayoutProperty->GetVerticalAlignValue());
    }
    return IMAGE_SPAN_LENGTH;
}

int32_t RichEditorPattern::DeleteValueSetTextSpan(
    const RefPtr<SpanItem>& spanItem, int32_t currentPosition, int32_t length, RichEditorAbstractSpanResult& spanResult)
{
    spanResult.SetSpanType(SpanResultType::TEXT);
    auto contentStartPosition = spanItem->position - StringUtils::ToWstring(spanItem->content).length();
    spanResult.SetSpanRangeStart(contentStartPosition);
    int32_t eraseLength = 0;
    if (spanItem->position - currentPosition >= length) {
        eraseLength = length;
    } else {
        eraseLength = spanItem->position - currentPosition;
    }
    spanResult.SetSpanRangeEnd(spanItem->position);
    spanResult.SetValue(spanItem->content);
    spanResult.SetOffsetInSpan(currentPosition - contentStartPosition);
    spanResult.SetEraseLength(eraseLength);
    return eraseLength;
}

void RichEditorPattern::DeleteByDeleteValueInfo(const RichEditorDeleteValue& info)
{
    auto deleteSpans = info.GetRichEditorDeleteSpans();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::list<RefPtr<UINode>> deleteNode;
    std::set<int32_t, std::greater<int32_t>> deleteNodes;
    for (const auto& it : deleteSpans) {
        switch (it.GetType()) {
            case SpanResultType::TEXT: {
                auto ui_node = host->GetChildAtIndex(it.GetSpanIndex());
                CHECK_NULL_VOID(ui_node);
                auto spanNode = DynamicCast<SpanNode>(ui_node);
                CHECK_NULL_VOID(spanNode);
                auto spanItem = spanNode->GetSpanItem();
                CHECK_NULL_VOID(spanItem);
                auto text = spanItem->content;
                std::wstring textTemp = StringUtils::ToWstring(text);
                textTemp.erase(it.OffsetInSpan(), it.GetEraseLength());
                if (textTemp.size() == 0) {
                    deleteNodes.emplace(it.GetSpanIndex());
                }
                text = StringUtils::ToString(textTemp);
                spanNode->UpdateContent(text);
                spanItem->position -= it.GetEraseLength();
                break;
            }
            case SpanResultType::IMAGE:
                deleteNodes.emplace(it.GetSpanIndex());
                break;
            default:
                break;
        }
    }
    for (auto index : deleteNodes) {
        host->RemoveChildAtIndex(index);
    }
    if (info.GetRichEditorDeleteDirection() == RichEditorDeleteDirection::BACKWARD) {
        caretPosition_ = std::clamp(caretPosition_ - info.GetLength(), 0, static_cast<int32_t>(GetTextContentLength()));
    }
    int32_t spanTextLength = 0;
    for (auto& span : spans_) {
        span->position = spanTextLength + StringUtils::ToWstring(span->content).length();
        spanTextLength += StringUtils::ToWstring(span->content).length();
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    OnModifyDone();
}

bool RichEditorPattern::OnKeyEvent(const KeyEvent& keyEvent)
{
    if (keyEvent.action == KeyAction::DOWN) {
        if (keyEvent.code == KeyCode::KEY_TAB) {
            return false;
        }
        if (keyEvent.code == KeyCode::KEY_DEL) {
#if defined(PREVIEW)
            DeleteForward(1);
#else
            DeleteBackward(1);
#endif
            return true;
        }
        if (keyEvent.code == KeyCode::KEY_FORWARD_DEL) {
#if defined(PREVIEW)
            DeleteBackward(1);
#else
            DeleteForward(1);
#endif
            return true;
        }
        if (keyEvent.code == KeyCode::KEY_ENTER || keyEvent.code == KeyCode::KEY_NUMPAD_ENTER ||
            keyEvent.code == KeyCode::KEY_DPAD_CENTER) {
            InsertValue("\n");
            return true;
        }
        if (keyEvent.IsDirectionalKey()) {
            switch (keyEvent.code) {
                case KeyCode::KEY_DPAD_UP:
                    return CursorMoveUp();
                case KeyCode::KEY_DPAD_DOWN:
                    return CursorMoveDown();
                case KeyCode::KEY_DPAD_LEFT:
                    return CursorMoveLeft();
                case KeyCode::KEY_DPAD_RIGHT:
                    return CursorMoveRight();
                default:
                    return false;
            }
        }
        auto visibilityCode = keyEvent.ConvertInputCodeToString();
        if (visibilityCode != std::string("")) {
            if ((keyEvent.pressedCodes[0] == KeyCode::KEY_SHIFT_LEFT ||
                    keyEvent.pressedCodes[0] == KeyCode::KEY_SHIFT_RIGHT) &&
                visibilityCode.length() > 1) {
                InsertValue(visibilityCode.substr(1, 1));
            } else {
                InsertValue(visibilityCode.substr(0, 1).c_str());
            }
            return true;
        }
        return false;
    }
    return true;
}

void RichEditorPattern::MoveCaretAfterTextChange()
{
    CHECK_NULL_VOID(isTextChange_);
    isTextChange_ = false;
    switch (moveDirection_) {
        case MoveDirection::BACKWARD:
            caretPosition_ =
                std::clamp((caretPosition_ - moveLength_), 0, static_cast<int32_t>(GetTextContentLength()));
            break;
        case MoveDirection::FORWARD:
            caretPosition_ =
                std::clamp((caretPosition_ + moveLength_), 0, static_cast<int32_t>(GetTextContentLength()));
            break;
        default:
            break;
    }
    moveLength_ = 0;
}

void RichEditorPattern::InitTouchEvent()
{
    CHECK_NULL_VOID(!touchListener_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    auto gesture = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gesture);
    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleTouchEvent(info);
    };
    touchListener_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gesture->AddTouchEvent(touchListener_);
}

void RichEditorPattern::HandleTouchEvent(const TouchEventInfo& info)
{
    if (SelectOverlayIsOn()) {
        return;
    }
    auto touchType = info.GetTouches().front().GetTouchType();
    if (touchType == TouchType::DOWN) {
    } else if (touchType == TouchType::UP) {
        isMousePressed_ = false;
    }
}

void RichEditorPattern::HandleMouseLeftButton(const MouseInfo& info)
{
    if (info.GetAction() == MouseAction::MOVE) {
        if (blockPress_ || !leftMousePress_) {
            return;
        }
        auto textPaintOffset = contentRect_.GetOffset() - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
        Offset textOffset = { info.GetLocalLocation().GetX() - textPaintOffset.GetX(),
            info.GetLocalLocation().GetY() - textPaintOffset.GetY() };

        mouseStatus_ = MouseStatus::MOVE;
        if (isFirstMouseSelect_) {
            int32_t extend = paragraphs_.GetIndex(textOffset);
            int32_t extendEnd = extend + GetGraphemeClusterLength(extend);
            textSelector_.Update(extend, extendEnd);
            isFirstMouseSelect_ = false;
        } else {
            int32_t extend = paragraphs_.GetIndex(textOffset);
            textSelector_.Update(textSelector_.baseOffset, extend);
            SetCaretPosition(std::max(textSelector_.baseOffset, extend));
        }
        isMouseSelect_ = true;
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    } else if (info.GetAction() == MouseAction::PRESS) {
        isMousePressed_ = true;
        if (BetweenSelectedPosition(info.GetGlobalLocation())) {
            blockPress_ = true;
            return;
        }
        leftMousePress_ = true;
        mouseStatus_ = MouseStatus::PRESSED;
        blockPress_ = false;
    } else if (info.GetAction() == MouseAction::RELEASE) {
        blockPress_ = false;
        leftMousePress_ = false;
        mouseStatus_ = MouseStatus::RELEASED;
        isMouseSelect_ = false;
        isMousePressed_ = false;
        isFirstMouseSelect_ = true;
        auto selectStart = std::min(textSelector_.baseOffset, textSelector_.destinationOffset);
        auto selectEnd = std::max(textSelector_.baseOffset, textSelector_.destinationOffset);
        FireOnSelect(selectStart, selectEnd);
    }
}

void RichEditorPattern::HandleMouseRightButton(const MouseInfo& info)
{
    if (info.GetAction() == MouseAction::PRESS) {
        isMousePressed_ = true;
        usingMouseRightButton_ = true;
        CloseSelectionMenu();
    } else if (info.GetAction() == MouseAction::RELEASE) {
        rightClickOffset_ = OffsetF(
            static_cast<float>(info.GetGlobalLocation().GetX()), static_cast<float>(info.GetGlobalLocation().GetY()));
        if (textSelector_.IsValid() && BetweenSelectedPosition(info.GetGlobalLocation())) {
            ShowSelectOverlay(RectF(), RectF());
            isMousePressed_ = false;
            usingMouseRightButton_ = false;
            return;
        }
        if (textSelector_.IsValid()) {
            CloseSelectOverlay();
            ResetSelection();
        }
        MouseRightFocus(info);
        ShowSelectOverlay(RectF(), RectF());
        isMousePressed_ = false;
        usingMouseRightButton_ = false;
    }
}

void RichEditorPattern::MouseRightFocus(const MouseInfo& info)
{
    auto contentRect = GetTextRect();
    contentRect.SetTop(contentRect.GetY() - std::min(baselineOffset_, 0.0f));
    contentRect.SetHeight(contentRect.Height() - std::max(baselineOffset_, 0.0f));
    Offset textOffset = { info.GetLocalLocation().GetX() - contentRect.GetX(),
        info.GetLocalLocation().GetY() - contentRect.GetY() };
    InitSelection(textOffset);
    auto selectStart = std::min(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto selectEnd = std::max(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->RequestFocusImmediately();
    SetCaretPosition(selectEnd);

    TextInsertValueInfo spanInfo;
    CalcInsertValueObj(spanInfo);
    auto spanNode = DynamicCast<FrameNode>(GetChildByIndex(spanInfo.GetSpanIndex() - 1));
    if (spanNode && spanNode->GetTag() == V2::IMAGE_ETS_TAG && spanInfo.GetOffsetInSpan() == 0 &&
            selectEnd == selectStart + 1) {
        FireOnSelect(selectStart, selectEnd);
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        StopTwinkling();
        return;
    }
    if (textSelector_.IsValid()) {
        ResetSelection();
    }
    auto position = paragraphs_.GetIndex(textOffset);
    float caretHeight = 0.0f;
    OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
    SetCaretPosition(position);
    CHECK_NULL_VOID(overlayMod_);
    DynamicCast<RichEditorOverlayModifier>(overlayMod_)->SetCaretOffsetAndHeight(caretOffset, caretHeight);
    StartTwinkling();
}

void RichEditorPattern::FireOnSelect(int32_t selectStart, int32_t selectEnd)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto textSelectInfo = GetSpansInfo(selectStart, selectEnd, GetSpansMethod::ONSELECT);
    if (!textSelectInfo.GetSelection().resultObjects.empty()) {
        eventHub->FireOnSelect(&textSelectInfo);
    }
    UpdateSelectionType(textSelectInfo);
}

void RichEditorPattern::HandleMouseEvent(const MouseInfo& info)
{
    if (info.GetButton() == MouseButton::LEFT_BUTTON) {
        HandleMouseLeftButton(info);
    } else if (info.GetButton() == MouseButton::RIGHT_BUTTON) {
        HandleMouseRightButton(info);
    }
}

void RichEditorPattern::OnHandleMoveDone(const RectF& handleRect, bool isFirstHandle)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto selectStart = std::min(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto selectEnd = std::max(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto textSelectInfo = GetSpansInfo(selectStart, selectEnd, GetSpansMethod::ONSELECT);
    if (!textSelectInfo.GetSelection().resultObjects.empty()) {
        eventHub->FireOnSelect(&textSelectInfo);
    }
    UpdateSelectionType(textSelectInfo);
    SetCaretPosition(selectEnd);
    CalculateHandleOffsetAndShowOverlay();
    if (selectOverlayProxy_) {
        SelectHandleInfo handleInfo;
        if (isFirstHandle) {
            handleInfo.paintRect = textSelector_.firstHandle;
            selectOverlayProxy_->UpdateFirstSelectHandleInfo(handleInfo);
        } else {
            handleInfo.paintRect = textSelector_.secondHandle;
            selectOverlayProxy_->UpdateSecondSelectHandleInfo(handleInfo);
        }

        if (IsSelectAll() && selectMenuInfo_.showCopyAll == true) {
            selectMenuInfo_.showCopyAll = false;
            selectOverlayProxy_->UpdateSelectMenuInfo(selectMenuInfo_);
        } else if (!IsSelectAll() && selectMenuInfo_.showCopyAll == false) {
            selectMenuInfo_.showCopyAll = true;
            selectOverlayProxy_->UpdateSelectMenuInfo(selectMenuInfo_);
        }
        return;
    }
    ShowSelectOverlay(textSelector_.firstHandle, textSelector_.secondHandle);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

RefPtr<UINode> RichEditorPattern::GetChildByIndex(int32_t index) const
{
    auto host = GetHost();
    const auto& children = host->GetChildren();
    int32_t size = static_cast<int32_t>(children.size());
    if (index < 0 || index >= size) {
        return nullptr;
    }
    auto pos = children.begin();
    std::advance(pos, index);
    return *pos;
}

std::string RichEditorPattern::GetSelectedSpanText(std::wstring value, int32_t start, int32_t end) const
{
    if (start < 0 || end > static_cast<int32_t>(value.length()) || start >= end) {
        LOGI("Get selected boundary is invalid");
        return "";
    }
    auto min = std::min(start, end);
    auto max = std::max(start, end);

    return StringUtils::ToString(value.substr(min, max - min));
}

TextStyleResult RichEditorPattern::GetTextStyleObject(const RefPtr<SpanNode>& node)
{
    TextStyleResult textStyle;
    textStyle.fontColor = node->GetTextColorValue(Color::BLACK).ColorToString();
    textStyle.fontSize = node->GetFontSizeValue(Dimension(16.0f, DimensionUnit::VP)).ConvertToVp();
    textStyle.fontStyle = static_cast<int32_t>(node->GetItalicFontStyleValue(OHOS::Ace::FontStyle::NORMAL));
    textStyle.fontWeight = static_cast<int32_t>(node->GetFontWeightValue(FontWeight::NORMAL));
    std::string fontFamilyValue;
    const std::vector<std::string> defaultFontFamily = { "HarmonyOS Sans" };
    auto fontFamily = node->GetFontFamilyValue(defaultFontFamily);
    for (const auto& str : fontFamily) {
        fontFamilyValue += str;
        fontFamilyValue += ",";
    }
    fontFamilyValue = fontFamilyValue.substr(0, fontFamilyValue.size() - 1);
    textStyle.fontFamily = !fontFamilyValue.empty() ? fontFamilyValue : defaultFontFamily.front();
    textStyle.decorationType = static_cast<int32_t>(node->GetTextDecorationValue(TextDecoration::NONE));
    textStyle.decorationColor = node->GetTextDecorationColorValue(Color::BLACK).ColorToString();
    return textStyle;
}

ResultObject RichEditorPattern::GetTextResultObject(RefPtr<UINode> uinode, int32_t index, int32_t start, int32_t end)
{
    bool selectFlag = false;
    ResultObject resultObject;
    if (!DynamicCast<SpanNode>(uinode)) {
        return resultObject;
    }
    auto spanItem = DynamicCast<SpanNode>(uinode)->GetSpanItem();
    int32_t itemLength = StringUtils::ToWstring(spanItem->content).length();
    int32_t endPosition = std::min(GetTextContentLength(), spanItem->position);
    int32_t startPosition = endPosition - itemLength;

    if (startPosition >= start && endPosition <= end) {
        selectFlag = true;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = 0;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = itemLength;
    } else if (startPosition < start && endPosition <= end && endPosition > start) {
        selectFlag = true;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = start - startPosition;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = itemLength;
    } else if (startPosition >= start && startPosition < end && endPosition >= end) {
        selectFlag = true;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = 0;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = end - startPosition;
    } else if (startPosition <= start && endPosition >= end) {
        selectFlag = true;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = start - startPosition;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = end - startPosition;
    }
    if (selectFlag == true) {
        resultObject.spanPosition.spanIndex = index;
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGESTART] = startPosition;
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGEEND] = endPosition;
        resultObject.type = RichEditorSpanType::TYPESPAN;
        resultObject.valueString = spanItem->content;
        auto spanNode = DynamicCast<SpanNode>(uinode);
        resultObject.textStyle = GetTextStyleObject(spanNode);
    }
    return resultObject;
}

ResultObject RichEditorPattern::GetImageResultObject(RefPtr<UINode> uinode, int32_t index, int32_t start, int32_t end)
{
    int32_t itemLength = 1;
    ResultObject resultObject;
    if (!DynamicCast<FrameNode>(uinode) || !GetSpanItemByIndex(index)) {
        return resultObject;
    }
    int32_t endPosition = std::min(GetTextContentLength(), GetSpanItemByIndex(index)->position);
    int32_t startPosition = endPosition - itemLength;
    if ((start <= startPosition) && (end >= endPosition)) {
        auto imageNode = DynamicCast<FrameNode>(uinode);
        auto imageLayoutProperty = DynamicCast<ImageLayoutProperty>(imageNode->GetLayoutProperty());
        resultObject.spanPosition.spanIndex = index;
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGESTART] = startPosition;
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGEEND] = endPosition;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = 0;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = itemLength;
        resultObject.type = RichEditorSpanType::TYPEIMAGE;
        if (!imageLayoutProperty->GetImageSourceInfo()->GetPixmap()) {
            resultObject.valueString = imageLayoutProperty->GetImageSourceInfo()->GetSrc().c_str();
        } else {
            resultObject.valuePixelMap = imageLayoutProperty->GetImageSourceInfo()->GetPixmap();
        }
        auto geometryNode = imageNode->GetGeometryNode();
        resultObject.imageStyle.size[RichEditorImageSize::SIZEWIDTH] = geometryNode->GetMarginFrameSize().Width();
        resultObject.imageStyle.size[RichEditorImageSize::SIZEHEIGHT] = geometryNode->GetMarginFrameSize().Height();
        if (imageLayoutProperty->HasImageFit()) {
            resultObject.imageStyle.verticalAlign = static_cast<int32_t>(imageLayoutProperty->GetImageFitValue());
        }
        if (imageLayoutProperty->HasVerticalAlign()) {
            resultObject.imageStyle.objectFit = static_cast<int32_t>(imageLayoutProperty->GetVerticalAlignValue());
        }
    }
    return resultObject;
}

RichEditorSelection RichEditorPattern::GetSpansInfo(int32_t start, int32_t end, GetSpansMethod method)
{
    int32_t index = 0;
    std::int32_t realEnd = 0;
    std::int32_t realStart = 0;
    RichEditorSelection selection;
    std::list<ResultObject> resultObjects;
    auto length = GetTextContentLength();
    if (method == GetSpansMethod::GETSPANS) {
        realStart = (start == -1) ? 0 : start;
        realEnd = (end == -1) ? length : end;
        if (realStart > realEnd) {
            std::swap(realStart, realEnd);
        }
        realStart = std::max(0, realStart);
        realEnd = std::min(length, realEnd);
    } else if (method == GetSpansMethod::ONSELECT) {
        realEnd = std::min(length, end);
        realStart = std::min(length, start);
    }
    selection.SetSelectionEnd(realEnd);
    selection.SetSelectionStart(realStart);
    if (realStart > length || realEnd < 0 || spans_.empty()) {
        selection.SetResultObjectList(resultObjects);
        return selection;
    }
    auto host = GetHost();
    const auto& children = host->GetChildren();
    for (const auto& uinode : children) {
        if (uinode->GetTag() == V2::IMAGE_ETS_TAG) {
            ResultObject resultObject = GetImageResultObject(uinode, index, realStart, realEnd);
            if (!resultObject.valueString.empty() || resultObject.valuePixelMap) {
                resultObjects.emplace_back(resultObject);
            }
        } else if (uinode->GetTag() == V2::SPAN_ETS_TAG) {
            ResultObject resultObject = GetTextResultObject(uinode, index, realStart, realEnd);
            if (!resultObject.valueString.empty()) {
                resultObjects.emplace_back(resultObject);
            }
        }
        index++;
    }
    selection.SetResultObjectList(resultObjects);
    return selection;
}

void RichEditorPattern::CopySelectionMenuParams(SelectOverlayInfo& selectInfo)
{
    auto selectType = selectedType_.value_or(RichEditorType::TEXT);
    std::shared_ptr<SelectionMenuParams> menuParams = nullptr;
    if (selectType == RichEditorType::TEXT) {
        menuParams = GetMenuParams(selectInfo.isUsingMouse, RichEditorType::TEXT);
    } else if (selectType == RichEditorType::IMAGE) {
        menuParams = GetMenuParams(selectInfo.isUsingMouse, RichEditorType::IMAGE);
    } else if (selectType == RichEditorType::MIXED) {
        menuParams = GetMenuParams(selectInfo.isUsingMouse, RichEditorType::MIXED);
        if (menuParams == nullptr) {
            menuParams = GetMenuParams(selectInfo.isUsingMouse, RichEditorType::TEXT);
        }
    }

    if (menuParams == nullptr) {
        return;
    }

    selectInfo.menuInfo.menuBuilder = menuParams->buildFunc;
    if (menuParams->onAppear) {
        auto weak = AceType::WeakClaim(this);
        auto callback = [weak, menuParams]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            CHECK_NULL_VOID(menuParams->onAppear);

            auto& textSelector = pattern->textSelector_;
            auto selectStart = std::min(textSelector.baseOffset, textSelector.destinationOffset);
            auto selectEnd = std::max(textSelector.baseOffset, textSelector.destinationOffset);
            menuParams->onAppear(selectStart, selectEnd);
        };
        selectInfo.menuCallback.onAppear = std::move(callback);
    }
    selectInfo.menuCallback.onDisappear = menuParams->onDisappear;
}

void RichEditorPattern::ShowSelectOverlay(const RectF& firstHandle, const RectF& secondHandle, bool isCopyAll)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto hasDataCallback = [weak = WeakClaim(this), pipeline, firstHandle, secondHandle, isCopyAll](bool hasData) {
        auto pattern = weak.Upgrade();
        SelectOverlayInfo selectInfo;
        bool usingMouse = pattern->IsUsingMouse();
        if (!pattern->IsUsingMouse()) {
            selectInfo.firstHandle.paintRect = firstHandle;
            selectInfo.secondHandle.paintRect = secondHandle;
        } else {
            selectInfo.isUsingMouse = true;
            selectInfo.rightClickOffset = pattern->GetRightClickOffset();
            pattern->ResetIsMousePressed();
        }
        selectInfo.onHandleMove = [weak](const RectF& handleRect, bool isFirst) {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->OnHandleMove(handleRect, isFirst);
        };
        selectInfo.onHandleMoveDone = [weak](const RectF& handleRect, bool isFirst) {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->OnHandleMoveDone(handleRect, isFirst);
        };

        auto host = pattern->GetHost();
        CHECK_NULL_VOID(host);

        pattern->UpdateSelectMenuInfo(hasData, selectInfo, isCopyAll);

        selectInfo.menuCallback.onCopy = [weak]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->HandleOnCopy();
            pattern->CloseSelectOverlay();
            pattern->ResetSelection();
        };

        selectInfo.menuCallback.onCut = [weak]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->HandleOnCut();
        };

        selectInfo.menuCallback.onPaste = [weak]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->HandleOnPaste();
        };
        selectInfo.menuCallback.onSelectAll = [weak, usingMouse]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->isMousePressed_ = usingMouse;
            pattern->HandleOnSelectAll();
        };
        selectInfo.callerFrameNode = host;

        pattern->CopySelectionMenuParams(selectInfo);
        pattern->UpdateSelectOverlayOrCreate(selectInfo);
    };
    CHECK_NULL_VOID(clipboard_);
    clipboard_->HasData(hasDataCallback);
}

void RichEditorPattern::HandleOnCopy()
{
    CHECK_NULL_VOID(clipboard_);
    auto selectStart = textSelector_.GetTextStart();
    auto selectEnd = textSelector_.GetTextEnd();
    auto textSelectInfo = GetSpansInfo(selectStart, selectEnd, GetSpansMethod::ONSELECT);
    auto copyResultObjects = textSelectInfo.GetSelection().resultObjects;
    if (copyResultObjects.empty()) {
        return;
    }
    RefPtr<PasteDataMix> pasteData = clipboard_->CreatePasteDataMix();
    auto resultProcessor = [weak = WeakClaim(this), pasteData, selectStart, selectEnd, clipboard = clipboard_](
                               const ResultObject& result) {
        auto pattern = weak.Upgrade();
        if (result.type == RichEditorSpanType::TYPESPAN) {
            auto data = pattern->GetSelectedSpanText(StringUtils::ToWstring(result.valueString),
                result.offsetInSpan[RichEditorSpanRange::RANGESTART],
                result.offsetInSpan[RichEditorSpanRange::RANGEEND]);
            LOGI("HandleOnCopy TYPESPAN, start: %{public}d, end: %{public}d, data: %{public}s",
                result.offsetInSpan[RichEditorSpanRange::RANGESTART],
                result.offsetInSpan[RichEditorSpanRange::RANGEEND], data.c_str());
            clipboard->AddTextRecord(pasteData, data);
            return;
        }
        if (result.type == RichEditorSpanType::TYPEIMAGE) {
            if (result.valuePixelMap) {
                LOGI("HandleOnCopy TYPEIMAGE valuePixelMap");
                clipboard->AddPixelMapRecord(pasteData, result.valuePixelMap);
            } else {
                LOGI("HandleOnCopy TYPEIMAGE, uri: %{public}s", result.valueString.c_str());
                clipboard->AddImageRecord(pasteData, result.valueString);
            }
        }
    };
    for (auto resultObj = copyResultObjects.rbegin(); resultObj != copyResultObjects.rend(); ++resultObj) {
        resultProcessor(*resultObj);
    }
    clipboard_->SetData(pasteData, copyOption_);
    StartTwinkling();
}

void RichEditorPattern::ResetAfterPaste()
{
    SetCaretSpanIndex(-1);
    StartTwinkling();
    CloseSelectOverlay();
    if (textSelector_.IsValid()) {
        SetCaretPosition(textSelector_.GetTextStart());
        auto length = textSelector_.GetTextEnd() - textSelector_.GetTextStart();
        textSelector_.Update(-1, -1);
        DeleteForward(length);
        ResetSelection();
    }
}

void RichEditorPattern::HandleOnPaste()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    TextCommonEvent event;
    eventHub->FireOnPaste(event);
    if (event.IsPreventDefault()) {
        return;
    }

    CHECK_NULL_VOID(clipboard_);
    auto textCallback = [weak = WeakClaim(this), textSelector = textSelector_](
                            const std::string& data, bool isLastRecord) {
        if (data.empty()) {
            LOGE("Paste value is empty");
            return;
        }
        auto richEditor = weak.Upgrade();
        CHECK_NULL_VOID(richEditor);
        LOGI("HandleOnPaste InsertValue: %{public}s", data.c_str());
        richEditor->InsertValueByPaste(data);
        if (isLastRecord) {
            richEditor->ResetAfterPaste();
        }
        LOGI("after insert text record, CaretPosition:%{public}d,  moveLength_: %{public}d",
            richEditor->GetCaretPosition(), richEditor->moveLength_);
    };
    auto pixelMapCallback = [weak = WeakClaim(this), textSelector = textSelector_](
                                const RefPtr<PixelMap>& pixelMap, bool isLastRecord) {
        auto richEditor = weak.Upgrade();
        CHECK_NULL_VOID(richEditor);
        ImageSpanOptions imageOption;
        imageOption.imagePixelMap = pixelMap;
        ImageSpanAttribute imageAttribute;
        ImageSpanSize size;
        size.width = CalcDimension(DEFAULT_IMAGE_SIZE);
        size.height = CalcDimension(DEFAULT_IMAGE_SIZE);
        imageAttribute.size = size;
        imageOption.imageAttribute = imageAttribute;
        int32_t index = 0;
        if (richEditor->GetCaretSpanIndex() == -1) {
            imageOption.offset = richEditor->GetCaretPosition() + richEditor->moveLength_;
            index = richEditor->AddImageSpan(imageOption, true);
        } else {
            index = richEditor->AddImageSpan(imageOption, true, richEditor->GetCaretSpanIndex() + 1);
        }
        if (isLastRecord) {
            richEditor->ResetAfterPaste();
        } else {
            richEditor->SetCaretSpanIndex(index);
        }
        LOGI("after insert pixelMap record, CaretPosition :%{public}d, moveLength_: %{public}d index: %{public}d",
            richEditor->GetCaretPosition(), richEditor->moveLength_, index);
    };
    auto urlCallback = [weak = WeakClaim(this), textSelector = textSelector_](
                           const std::string& uri, bool isLastRecord) {
        auto richEditor = weak.Upgrade();
        CHECK_NULL_VOID(richEditor);
        ImageSpanOptions imageOption;
        imageOption.image = uri;
        ImageSpanAttribute imageAttribute;
        ImageSpanSize size;
        size.width = CalcDimension(DEFAULT_IMAGE_SIZE);
        size.height = CalcDimension(DEFAULT_IMAGE_SIZE);
        imageAttribute.size = size;
        imageOption.imageAttribute = imageAttribute;
        int32_t index = 0;
        if (richEditor->GetCaretSpanIndex() == -1) {
            imageOption.offset = richEditor->GetCaretPosition() + richEditor->moveLength_;
            index = richEditor->AddImageSpan(imageOption, true);
        } else {
            index = richEditor->AddImageSpan(imageOption, true, richEditor->GetCaretSpanIndex() + 1);
        }
        if (isLastRecord) {
            richEditor->ResetAfterPaste();
        } else {
            richEditor->SetCaretSpanIndex(index);
        }
        LOGI("after insert pixelMap url, CaretPosition :%{public}d, moveLength_: %{public}d index: %{public}d",
            richEditor->GetCaretPosition(), richEditor->moveLength_, index);
    };
    clipboard_->GetData(textCallback, pixelMapCallback, urlCallback);
}

void RichEditorPattern::InsertValueByPaste(const std::string& insertValue)
{
    RefPtr<UINode> child;
    TextInsertValueInfo info;
    CalcInsertValueObj(info);
    LOGD("InsertValueByPaste spanIndex: %{public}d,  offset inspan:  %{public}d, caretPosition: %{public}d",
        info.GetSpanIndex(), info.GetOffsetInSpan(), caretPosition_);
    TextSpanOptions options;
    options.value = insertValue;
    if (typingStyle_.has_value() && typingTextStyle_.has_value()) {
        options.style = typingTextStyle_.value();
    }
    auto newSpanOffset = caretPosition_ + moveLength_;
    isTextChange_ = true;
    moveDirection_ = MoveDirection::FORWARD;
    moveLength_ += static_cast<int32_t>(StringUtils::ToWstring(insertValue).length());
    if (caretSpanIndex_ == -1) {
        child = GetChildByIndex(info.GetSpanIndex());
        if (child && child->GetTag() == V2::SPAN_ETS_TAG) {
            auto spanNode = DynamicCast<SpanNode>(child);
            CHECK_NULL_VOID(spanNode);
            if (typingStyle_.has_value() && !HasSameTypingStyle(spanNode)) {
                options.offset = newSpanOffset;
                caretSpanIndex_ = AddTextSpan(options, true);
            } else {
                InsertValueToSpanNode(spanNode, insertValue, info);
            }
            LOGD("insert first record after SpanNode, caretSpanIndex: %{public}d ", caretSpanIndex_);
            return;
        } else if (!child) {
            auto spanNodeBefore = DynamicCast<SpanNode>(GetChildByIndex(info.GetSpanIndex() - 1));
            if (spanNodeBefore == nullptr) {
                caretSpanIndex_ = AddTextSpan(options, true);
                LOGD("insert the first record at the end, caretSpanIndex: %{public}d ", caretSpanIndex_);
                return;
            }
            if (typingStyle_.has_value() && !HasSameTypingStyle(spanNodeBefore)) {
                auto spanNode = DynamicCast<SpanNode>(child);
                CreateTextSpanNode(spanNode, info, insertValue, false);
                caretSpanIndex_ = info.GetSpanIndex();
            } else {
                InsertValueToBeforeSpan(spanNodeBefore, insertValue);
                caretSpanIndex_ = info.GetSpanIndex() - 1;
            }
            LOGD("insert the first record at the before spanNode, caretSpanIndex: "
                 "%{public}d",
                caretSpanIndex_);
            return;
        }
    } else {
        child = GetChildByIndex(caretSpanIndex_);
        if (child && child->GetTag() == V2::SPAN_ETS_TAG) {
            auto spanNode = DynamicCast<SpanNode>(child);
            CHECK_NULL_VOID(spanNode);
            LOGD("insert record after spanNode at caretSpanIndex: %{public}d ", caretSpanIndex_);
            if (typingStyle_.has_value() && !HasSameTypingStyle(spanNode)) {
                options.offset = newSpanOffset;
                caretSpanIndex_ = AddTextSpan(options, true);
            } else {
                InsertValueToBeforeSpan(spanNode, insertValue);
            }
            return;
        }
    }
    if (child && child->GetTag() == V2::IMAGE_ETS_TAG) {
        LOGD("InsertValueByPaste after imageNode");
        auto spanNodeBefore = DynamicCast<SpanNode>(GetChildByIndex(info.GetSpanIndex() - 1));
        if (spanNodeBefore != nullptr && caretSpanIndex_ == -1) {
            if (typingStyle_.has_value() && !HasSameTypingStyle(spanNodeBefore)) {
                options.offset = newSpanOffset;
                caretSpanIndex_ = AddTextSpan(options, true);
            } else {
                InsertValueToBeforeSpan(spanNodeBefore, insertValue);
                caretSpanIndex_ = info.GetSpanIndex() - 1;
            }
            LOGD("child is image and insert the first record at the before spanNode, "
                 "caretSpanIndex: %{public}d",
                caretSpanIndex_);
        } else {
            auto imageNode = DynamicCast<FrameNode>(child);
            if (imageNode && caretSpanIndex_ == -1) {
                caretSpanIndex_ = AddTextSpan(options, true, info.GetSpanIndex());
                LOGD("child is image and insert the first record after imageNode, "
                     "caretSpanIndex: %{public}d",
                    caretSpanIndex_);
            } else {
                caretSpanIndex_ = AddTextSpan(options, true, caretSpanIndex_ + 1);
                LOGD("child is image and insert the record after imageNode, "
                     "caretSpanIndex: %{public}d",
                    caretSpanIndex_);
            }
        }
    } else {
        caretSpanIndex_ = AddTextSpan(options, true);
        LOGD("after insert, caretSpanIndex: %{public}d ", caretSpanIndex_);
    }
}

void RichEditorPattern::SetCaretSpanIndex(int32_t index)
{
    caretSpanIndex_ = index;
}

void RichEditorPattern::HandleOnCut()
{
    HandleOnCopy();
    DeleteBackward();
}

void RichEditorPattern::OnHandleMove(const RectF& handleRect, bool isFirstHandle)
{
    TextPattern::OnHandleMove(handleRect, isFirstHandle);
    if (!isFirstHandle) {
        SetCaretPosition(textSelector_.destinationOffset);
    }
}

#ifdef ENABLE_DRAG_FRAMEWORK
std::function<void(Offset)> RichEditorPattern::GetThumbnailCallback()
{
    return [wk = WeakClaim(this)](const Offset& point) {
        auto pattern = wk.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (pattern->BetweenSelectedPosition(point)) {
            auto host = pattern->GetHost();
            auto children = host->GetChildren();
            std::list<RefPtr<FrameNode>> imageChildren;
            for (const auto& child : children) {
                auto node = DynamicCast<FrameNode>(child);
                if (!node) {
                    continue;
                }
                auto image = node->GetPattern<ImagePattern>();
                if (image) {
                    imageChildren.emplace_back(node);
                }
            }
            pattern->dragNode_ = RichEditorDragPattern::CreateDragNode(host, imageChildren);
            FrameNode::ProcessOffscreenNode(pattern->dragNode_);
        }
    };
}
#endif

void RichEditorPattern::CreateHandles()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    float startSelectHeight = 0.0f;
    float endSelectHeight = 0.0f;
    auto firstHandlePosition = CalcCursorOffsetByPosition(textSelector_.GetStart(), startSelectHeight);
    OffsetF firstHandleOffset(firstHandlePosition.GetX() + parentGlobalOffset_.GetX(),
        firstHandlePosition.GetY() + parentGlobalOffset_.GetY());
    textSelector_.firstHandleOffset_ = firstHandleOffset;
    auto secondHandlePosition = CalcCursorOffsetByPosition(textSelector_.GetEnd(), endSelectHeight);
    OffsetF secondHandleOffset(secondHandlePosition.GetX() + parentGlobalOffset_.GetX(),
        secondHandlePosition.GetY() + parentGlobalOffset_.GetY());
    textSelector_.secondHandleOffset_ = secondHandleOffset;
    SizeF firstHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), startSelectHeight };
    SizeF secondHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), endSelectHeight };
    RectF firstHandle = RectF(firstHandleOffset, firstHandlePaintSize);
    RectF secondHandle = RectF(secondHandleOffset, secondHandlePaintSize);
    ShowSelectOverlay(firstHandle, secondHandle);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void RichEditorPattern::OnAreaChangedInner()
{
    float selectLineHeight = 0.0f;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
    CHECK_NULL_VOID(context);
    auto parentGlobalOffset = host->GetPaintRectOffset() - context->GetRootRect().GetOffset();
    if (parentGlobalOffset != parentGlobalOffset_) {
        parentGlobalOffset_ = parentGlobalOffset;
        UpdateTextFieldManager(Offset(parentGlobalOffset_.GetX(), parentGlobalOffset_.GetY()), frameRect_.Height());
        CHECK_NULL_VOID(SelectOverlayIsOn());
        textSelector_.selectionBaseOffset.SetX(
            CalcCursorOffsetByPosition(textSelector_.GetStart(), selectLineHeight).GetX());
        textSelector_.selectionDestinationOffset.SetX(
            CalcCursorOffsetByPosition(textSelector_.GetEnd(), selectLineHeight).GetX());
        CreateHandles();
    }
}

void RichEditorPattern::CloseSelectionMenu()
{
    CloseSelectOverlay();
}

void RichEditorPattern::CloseSelectOverlay()
{
    TextPattern::CloseSelectOverlay(true);
}

void RichEditorPattern::CalculateHandleOffsetAndShowOverlay(bool isUsingMouse)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = host->GetContext();
    CHECK_NULL_VOID(pipeline);
    auto rootOffset = pipeline->GetRootRect().GetOffset();
    auto offset = host->GetPaintRectOffset();
    auto textPaintOffset = offset - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    float startSelectHeight = 0.0f;
    float endSelectHeight = 0.0f;
    auto startOffset = CalcCursorOffsetByPosition(textSelector_.baseOffset, startSelectHeight);
    auto endOffset =
        CalcCursorOffsetByPosition(std::min(textSelector_.destinationOffset, GetTextContentLength()), endSelectHeight);
    SizeF firstHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), startSelectHeight };
    SizeF secondHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), endSelectHeight };
    OffsetF firstHandleOffset = startOffset + textPaintOffset - rootOffset;
    OffsetF secondHandleOffset = endOffset + textPaintOffset - rootOffset;
    if (GetTextContentLength() == 0) {
        float caretHeight = DynamicCast<RichEditorOverlayModifier>(overlayMod_)->GetCaretHeight();
        firstHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), caretHeight / 2 };
        secondHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), caretHeight / 2 };
        firstHandleOffset = OffsetF(firstHandleOffset.GetX(), firstHandleOffset.GetY() + caretHeight / 2);
        secondHandleOffset = OffsetF(secondHandleOffset.GetX(), secondHandleOffset.GetY() + caretHeight);
    }
    textSelector_.selectionBaseOffset = firstHandleOffset;
    textSelector_.selectionDestinationOffset = secondHandleOffset;
    RectF firstHandle;
    firstHandle.SetOffset(firstHandleOffset);
    firstHandle.SetSize(firstHandlePaintSize);
    textSelector_.firstHandle = firstHandle;
    RectF secondHandle;
    secondHandle.SetOffset(secondHandleOffset);
    secondHandle.SetSize(secondHandlePaintSize);
    textSelector_.secondHandle = secondHandle;
}

void RichEditorPattern::ResetSelection()
{
    if (textSelector_.IsValid()) {
        textSelector_.Update(-1, -1);
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto eventHub = host->GetEventHub<RichEditorEventHub>();
        CHECK_NULL_VOID(eventHub);
        auto textSelectInfo = GetSpansInfo(-1, -1, GetSpansMethod::ONSELECT);
        eventHub->FireOnSelect(&textSelectInfo);
        UpdateSelectionType(textSelectInfo);
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

RefPtr<SpanItem> RichEditorPattern::GetSpanItemByIndex(int32_t index) const
{
    int32_t size = static_cast<int32_t>(spans_.size());
    if (index < 0 || index >= size) {
        return nullptr;
    }
    auto pos = spans_.begin();
    std::advance(pos, index);
    return *pos;
}

bool RichEditorPattern::BetweenSelectedPosition(const Offset& globalOffset)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto offset = host->GetPaintRectOffset();
    auto localOffset = globalOffset - Offset(offset.GetX(), offset.GetY());
    auto eventHub = host->GetEventHub<EventHub>();
    if (GreatNotEqual(textSelector_.GetTextEnd(), textSelector_.GetTextStart())) {
        // Determine if the pan location is in the selected area
        auto selectedRects = paragraphs_.GetRects(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
        auto panOffset = OffsetF(localOffset.GetX(), localOffset.GetY()) - contentRect_.GetOffset() +
                         OffsetF(0.0, std::min(baselineOffset_, 0.0f));
        for (const auto& selectedRect : selectedRects) {
            if (selectedRect.IsInRegion(Point(panOffset.GetX(), panOffset.GetY()))) {
                return true;
            }
        }
    }
    return false;
}

void RichEditorPattern::HandleSurfaceChanged(int32_t newWidth, int32_t newHeight, int32_t prevWidth, int32_t prevHeight)
{
    if (newWidth != prevWidth || newHeight != prevHeight) {
        TextPattern::CloseSelectOverlay();
    }
    UpdateCaretInfoToController();
}

void RichEditorPattern::HandleSurfacePositionChanged(int32_t posX, int32_t posY)
{
    UpdateCaretInfoToController();
}

void RichEditorPattern::DumpInfo()
{
    if (customKeyboardBuilder_) {
        DumpLog::GetInstance().AddDesc(std::string("CustomKeyboard: true")
                                           .append(", Attached: ")
                                           .append(std::to_string(isCustomKeyboardAttached_)));
    }
}

bool RichEditorPattern::HasFocus() const
{
    auto focusHub = GetHost()->GetOrCreateFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    return focusHub->IsCurrentFocus();
}

void RichEditorPattern::UpdateTextFieldManager(const Offset& offset, float height)
{
    if (!HasFocus()) {
        return;
    }
    auto context = GetHost()->GetContext();
    CHECK_NULL_VOID(context);
    auto textFieldManager = DynamicCast<TextFieldManagerNG>(context->GetTextFieldManager());
    CHECK_NULL_VOID(textFieldManager);
    textFieldManager->SetClickPosition(offset);
    textFieldManager->SetHeight(height);
    textFieldManager->SetOnFocusTextField(WeakClaim(this));
}

bool RichEditorPattern::IsDisabled() const
{
    auto eventHub = GetHost()->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_RETURN(eventHub, true);
    return !eventHub->IsEnabled();
}

void RichEditorPattern::InitSelection(const Offset& pos)
{
    int32_t currentPosition = paragraphs_.GetIndex(pos);
    int32_t nextPosition = currentPosition + GetGraphemeClusterLength(currentPosition);
    nextPosition = std::min(nextPosition, GetTextContentLength());
    textSelector_.Update(currentPosition, nextPosition);
    auto selectedRects = paragraphs_.GetRects(currentPosition, nextPosition);
    bool selectedSingle = selectedRects.size() == 1 &&
                            (pos.GetX() < selectedRects[0].Left() || pos.GetY() < selectedRects[0].Top());
    bool selectedLast = selectedRects.size() == 0 && currentPosition == GetTextContentLength();
    if (selectedSingle || selectedLast) {
        if (selectedLast) {
            nextPosition = currentPosition + 1;
        }
        auto selectedNextRects = paragraphs_.GetRects(currentPosition - 1, nextPosition - 1);
        if (selectedNextRects.size() == 1) {
            bool isInRange = pos.GetX() >= selectedNextRects[0].Left() && pos.GetX() <= selectedNextRects[0].Right() &&
                             pos.GetY() >= selectedNextRects[0].Top() && pos.GetY() <= selectedNextRects[0].Bottom();
            if (isInRange) {
                textSelector_.Update(currentPosition - 1, nextPosition - 1);
            }
        }
    }
}

void RichEditorPattern::BindSelectionMenu(ResponseType type, RichEditorType richEditorType,
    std::function<void()>& menuBuilder, std::function<void(int32_t, int32_t)>& onAppear,
    std::function<void()>& onDisappear)
{
    auto key = std::make_pair(richEditorType, type);
    auto it = selectionMenuMap_.find(key);
    if (it != selectionMenuMap_.end()) {
        if (menuBuilder == nullptr) {
            selectionMenuMap_.erase(it);
            return;
        }
        it->second->buildFunc = menuBuilder;
        it->second->onAppear = onAppear;
        it->second->onDisappear = onDisappear;
        return;
    }

    auto selectionMenuParams =
        std::make_shared<SelectionMenuParams>(richEditorType, menuBuilder, onAppear, onDisappear, type);
    selectionMenuMap_[key] = selectionMenuParams;
    LOGD("BindSelectionMenu, editType = %{public}d responseType = %{public}d, map size = %{public}d", richEditorType,
        type, selectionMenuMap_.size());
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

RefPtr<NodePaintMethod> RichEditorPattern::CreateNodePaintMethod()
{
    if (!contentMod_) {
        contentMod_ = MakeRefPtr<RichEditorContentModifier>(textStyle_, &paragraphs_);
    }
    if (!overlayMod_) {
        overlayMod_ = MakeRefPtr<RichEditorOverlayModifier>();
    }
    if (GetIsCustomFont()) {
        contentMod_->SetIsCustomFont(true);
    }
    return MakeRefPtr<RichEditorPaintMethod>(WeakClaim(this), &paragraphs_, baselineOffset_, contentMod_, overlayMod_);
}

int32_t RichEditorPattern::GetHandleIndex(const Offset& offset) const
{
    return paragraphs_.GetIndex(offset);
}

#ifndef USE_GRAPHIC_TEXT_GINE
std::vector<RSTypographyProperties::TextBox> RichEditorPattern::GetTextBoxes()
#else
std::vector<RSTextRect> RichEditorPattern::GetTextBoxes()
#endif
{
    auto selectedRects = paragraphs_.GetRects(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
#ifndef USE_GRAPHIC_TEXT_GINE
    std::vector<RSTypographyProperties::TextBox> res;
#else
    std::vector<RSTextRect> res;
#endif
    res.reserve(selectedRects.size());
    for (auto&& rect : selectedRects) {
        if (NearZero(rect.Width())) {
            continue;
        }
        res.emplace_back(ConvertRect(rect));
    }
    return res;
}

float RichEditorPattern::GetLineHeight() const
{
    auto selectedRects = paragraphs_.GetRects(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
    CHECK_NULL_RETURN(selectedRects.size(), 0.0f);
    return selectedRects.front().Height();
}

void RichEditorPattern::UpdateSelectionType(RichEditorSelection& selection)
{
    selectedType_.reset();
    auto list = selection.GetSelection().resultObjects;
    bool imageSelected = false;
    bool textSelected = false;
    for (const auto& obj : list) {
        if (obj.type == RichEditorSpanType::TYPEIMAGE) {
            imageSelected = true;
        } else if (obj.type == RichEditorSpanType::TYPESPAN) {
            textSelected = true;
        }
        if (imageSelected && textSelected) {
            selectedType_ = RichEditorType::MIXED;
            return;
        }
    }
    if (imageSelected) {
        selectedType_ = RichEditorType::IMAGE;
    } else if (textSelected) {
        selectedType_ = RichEditorType::TEXT;
    }
    LOGD("UpdateSelectionType selectedType_ is %{public}d", selectedType_.value_or(RichEditorType::TEXT));
}

std::shared_ptr<SelectionMenuParams> RichEditorPattern::GetMenuParams(bool usingMouse, RichEditorType type)
{
    auto key = std::make_pair(type, usingMouse ? ResponseType::RIGHT_CLICK : ResponseType::LONG_PRESS);
    auto it = selectionMenuMap_.find(key);
    if (it != selectionMenuMap_.end()) {
        return it->second;
    }
    return nullptr;
}
} // namespace OHOS::Ace::NG
