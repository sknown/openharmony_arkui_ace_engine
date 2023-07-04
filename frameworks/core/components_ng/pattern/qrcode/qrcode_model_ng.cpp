/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/qrcode/qrcode_model_ng.h"

#include "core/components/qrcode/qrcode_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/qrcode/qrcode_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
void QRCodeModelNG::Create(const std::string& value)
{
    auto* stack = ViewStackProcessor::GetInstance();
    int32_t nodeId = (stack == nullptr ? 0 : stack->ClaimNodeId());
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::QRCODE_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<QRCodePattern>(); });
    ViewStackProcessor::GetInstance()->Push(frameNode);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    RefPtr<QrcodeTheme> qrCodeTheme = pipeline->GetTheme<QrcodeTheme>();
    CHECK_NULL_VOID(qrCodeTheme);
    ACE_UPDATE_PAINT_PROPERTY(QRCodePaintProperty, Value, value);
    ACE_UPDATE_PAINT_PROPERTY(QRCodePaintProperty, Color, qrCodeTheme->GetQrcodeColor());
    ACE_UPDATE_PAINT_PROPERTY(QRCodePaintProperty, BackgroundColor, qrCodeTheme->GetBackgroundColor());
    ACE_UPDATE_RENDER_CONTEXT(BackgroundColor, qrCodeTheme->GetBackgroundColor());
}

void QRCodeModelNG::SetQRCodeColor(Color color)
{
    ACE_UPDATE_PAINT_PROPERTY(QRCodePaintProperty, Color, color);
    ACE_UPDATE_RENDER_CONTEXT(ForegroundColor, color);
    ACE_RESET_RENDER_CONTEXT(RenderContext, ForegroundColorStrategy);
    ACE_UPDATE_RENDER_CONTEXT(ForegroundColorFlag, true);
}

void QRCodeModelNG::SetQRBackgroundColor(Color color)
{
    ACE_UPDATE_PAINT_PROPERTY(QRCodePaintProperty, BackgroundColor, color);
}

void QRCodeModelNG::SetContentOpacity(double opacity)
{
    ACE_UPDATE_PAINT_PROPERTY(QRCodePaintProperty, Opacity, opacity);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    RefPtr<QrcodeTheme> qrCodeTheme = pipeline->GetTheme<QrcodeTheme>();
    CHECK_NULL_VOID(qrCodeTheme);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto qrCodePaintProperty = frameNode->GetPaintProperty<NG::QRCodePaintProperty>();
    CHECK_NULL_VOID(qrCodePaintProperty);
    auto qrCodeColor = qrCodePaintProperty->GetColor().value_or(qrCodeTheme->GetQrcodeColor());
    ACE_UPDATE_PAINT_PROPERTY(QRCodePaintProperty, Color, qrCodeColor.BlendOpacity(opacity));
    ACE_UPDATE_RENDER_CONTEXT(ForegroundColor, qrCodeColor.BlendOpacity(opacity));
    ACE_RESET_RENDER_CONTEXT(RenderContext, ForegroundColorStrategy);
    ACE_UPDATE_RENDER_CONTEXT(ForegroundColorFlag, true);
}
} // namespace OHOS::Ace::NG
