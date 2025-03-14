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

#include "core/components_ng/pattern/select_overlay/select_overlay_paint_method.h"

#include "base/geometry/ng/offset_t.h"
#include "base/utils/utils.h"
#include "core/components/text_overlay/text_overlay_theme.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_layout_algorithm.h"
#include "core/components_ng/render/drawing.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
constexpr float AGING_MIN_SCALE = 1.75f;
void SelectOverlayPaintMethod::UpdateOverlayModifier(PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto textOverlayTheme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_VOID(textOverlayTheme);

    const auto& padding = textOverlayTheme->GetMenuPadding();
    auto left = padding.Left().ConvertToPx();
    auto right = padding.Right().ConvertToPx();
    auto top = padding.Top().ConvertToPx();
    auto sideWidth = textOverlayTheme->GetMenuToolbarHeight().ConvertToPx() - padding.Top().ConvertToPx() -
                     padding.Bottom().ConvertToPx();
    auto buttonRadius = sideWidth / 2.0;
    auto offset = isReversePaint_ ? defaultMenuStartOffset_ + OffsetF(buttonRadius + left, buttonRadius + top) :
                                    defaultMenuEndOffset_ + OffsetF(-buttonRadius - right, buttonRadius + top);
    if (GreatOrEqual(pipeline->GetFontScale(), AGING_MIN_SCALE)) {
        offset = defaultMenuEndOffset_ + OffsetF(-buttonRadius - right, selectMenuHeight_ / 2.0f);
    }

    CheckCirclesAndBackArrowIsShown();
    CheckHasExtensionMenu();
    selectOverlayModifier_->SetIsReverse(isReversePaint_);
    selectOverlayModifier_->SetMenuOptionOffset(offset);
    selectOverlayModifier_->SetFirstHandleIsShow(info_.firstHandle.isShow);
    selectOverlayModifier_->SetSecondHandleIsShow(info_.secondHandle.isShow);
    selectOverlayModifier_->SetIsNewAvoid(info_.isNewAvoid);
}

void SelectOverlayPaintMethod::UpdateContentModifier(PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto textOverlayTheme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_VOID(textOverlayTheme);

    auto offset = paintWrapper->GetGeometryNode()->GetFrameOffset();
    auto viewPort = paintWrapper->GetGeometryNode()->GetFrameRect() - offset;
    info_.GetCallerNodeAncestorViewPort(viewPort);

    CheckHandleIsShown();

    selectOverlayContentModifier_->SetIsUsingMouse(info_.isUsingMouse);

    if (!AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        selectOverlayContentModifier_->SetHandleColor(textOverlayTheme->GetHandleColor());
    } else {
        selectOverlayContentModifier_->SetHandleColor(info_.handlerColor.value_or(textOverlayTheme->GetHandleColor()));
    }
    selectOverlayContentModifier_->SetInnerHandleColor(textOverlayTheme->GetHandleColorInner());
    selectOverlayContentModifier_->SetHandleRadius(textOverlayTheme->GetHandleDiameter().ConvertToPx() / 2.0f);
    selectOverlayContentModifier_->SetHandleStrokeWidth(textOverlayTheme->GetHandleDiameterStrokeWidth().ConvertToPx());
    selectOverlayContentModifier_->SetInnerHandleRadius(
        textOverlayTheme->GetHandleDiameterInner().ConvertToPx() / 2.0f);

    selectOverlayContentModifier_->SetInShowArea(SelectOverlayLayoutAlgorithm::CheckInShowArea(info_));
    selectOverlayContentModifier_->SetHandleReverse(info_.handleReverse);
    selectOverlayContentModifier_->SetIsSingleHandle(info_.isSingleHandle);
    selectOverlayContentModifier_->SetFirstHandleIsShow(info_.firstHandle.isShow);
    selectOverlayContentModifier_->SetSecondHandleIsShow(info_.secondHandle.isShow);
    selectOverlayContentModifier_->SetIsHandleLineShow(info_.isHandleLineShow);
    selectOverlayContentModifier_->SetIsHiddenHandle(isHiddenHandle_);

    selectOverlayContentModifier_->SetViewPort(viewPort);
    selectOverlayContentModifier_->SetPaintHandleUsePoints(
        info_.firstHandle.isPaintHandleWithPoints || info_.secondHandle.isPaintHandleWithPoints);
    selectOverlayContentModifier_->SetFirstHandle(info_.firstHandle.paintRect - offset);
    selectOverlayContentModifier_->SetFirstHandlePaintInfo(info_.firstHandle.paintInfo - offset);
    selectOverlayContentModifier_->SetSecondHandle(info_.secondHandle.paintRect - offset);
    selectOverlayContentModifier_->SetSecondHandlePaintInfo(info_.secondHandle.paintInfo - offset);
}

void SelectOverlayPaintMethod::CheckCirclesAndBackArrowIsShown()
{
    if (isCreated_) {
        circlesAndBackArrowIsShown_ = true;
    }

    if (!circlesAndBackArrowIsShown_) {
        if (hasExtensionMenu_) {
            if (hasShowAnimation_) {
                AnimationOption option;
                option.SetDuration(MENU_SHOW_ANIMATION_DURATION);
                option.SetCurve(Curves::SHARP);

                AnimationUtils::Animate(option, [weak = WeakClaim(this)]() {
                    auto paintMethod = weak.Upgrade();
                    CHECK_NULL_VOID(paintMethod);
                    paintMethod->selectOverlayModifier_->SetCirclesAndBackArrowOpacity(1.0);
                });
            } else {
                selectOverlayModifier_->SetCirclesAndBackArrowOpacity(1.0);
            }
        }
        circlesAndBackArrowIsShown_ = true;
        selectOverlayModifier_->SetHasExtensionMenu(hasExtensionMenu_);
    }
}

void SelectOverlayPaintMethod::CheckHasExtensionMenu()
{
    if (selectOverlayModifier_->GetHasExtensionMenu() != hasExtensionMenu_) {
        if (hasExtensionMenu_) {
            AnimationOption option;
            option.SetDuration(MENU_SHOW_ANIMATION_DURATION);
            option.SetCurve(Curves::SHARP);

            AnimationUtils::Animate(option, [weak = WeakClaim(this)]() {
                auto paintMethod = weak.Upgrade();
                CHECK_NULL_VOID(paintMethod);
                paintMethod->selectOverlayModifier_->SetCirclesAndBackArrowOpacity(1.0);
            });
        } else {
            AnimationOption option;
            option.SetDuration(MENU_HIDE_ANIMATION_DURATION);
            option.SetCurve(Curves::SHARP);

            AnimationUtils::Animate(option, [weak = WeakClaim(this)]() {
                auto paintMethod = weak.Upgrade();
                CHECK_NULL_VOID(paintMethod);
                paintMethod->selectOverlayModifier_->SetCirclesAndBackArrowOpacity(0.0);
            });
        }
        selectOverlayModifier_->SetHasExtensionMenu(hasExtensionMenu_);
    }
}

void SelectOverlayPaintMethod::CheckHandleIsShown()
{
    if (isCreated_) {
        handleIsShown_ = true;
    }

    if (!handleIsShown_) {
        if (hasShowAnimation_) {
            AnimationOption option;
            option.SetDuration(HANDLE_ANIMATION_DURATION);
            option.SetCurve(Curves::SHARP);

            AnimationUtils::Animate(option, [weak = WeakClaim(this)]() {
                auto paintMethod = weak.Upgrade();
                CHECK_NULL_VOID(paintMethod);
                paintMethod->selectOverlayContentModifier_->SetHandleOpacity(1.0);
            });
        } else {
            selectOverlayContentModifier_->SetHandleOpacity(1.0);
        }
        handleIsShown_ = true;
    }
}

} // namespace OHOS::Ace::NG