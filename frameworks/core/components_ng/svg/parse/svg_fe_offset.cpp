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

#include "frameworks/core/components_ng/svg/parse/svg_fe_offset.h"

#ifndef NEW_SKIA
#include "include/effects/SkOffsetImageFilter.h"
#else
#include "include/effects/SkImageFilters.h"
#endif

#include "base/utils/utils.h"
#include "frameworks/core/components/declaration/svg/svg_fe_offset_declaration.h"

namespace OHOS::Ace::NG {

RefPtr<SvgNode> SvgFeOffset::Create()
{
    return AceType::MakeRefPtr<SvgFeOffset>();
}

SvgFeOffset::SvgFeOffset() : SvgFe()
{
    declaration_ = AceType::MakeRefPtr<SvgFeOffsetDeclaration>();
    declaration_->Init();
    declaration_->InitializeStyle();
}

#ifndef USE_ROSEN_DRAWING
void SvgFeOffset::OnAsImageFilter(sk_sp<SkImageFilter>& imageFilter, const ColorInterpolationType& srcColor,
    ColorInterpolationType& currentColor) const
#else
void SvgFeOffset::OnAsImageFilter(std::shared_ptr<RSImageFilter>& imageFilter, const ColorInterpolationType& srcColor,
    ColorInterpolationType& currentColor) const
#endif
{
    auto declaration = AceType::DynamicCast<SvgFeOffsetDeclaration>(declaration_);
    CHECK_NULL_VOID_NOLOG(declaration);
    imageFilter = MakeImageFilter(declaration->GetIn(), imageFilter);
#ifndef USE_ROSEN_DRAWING
#ifndef NEW_SKIA
    imageFilter = SkOffsetImageFilter::Make(declaration->GetDx(), declaration->GetDy(), imageFilter);
#else
    imageFilter = SkImageFilters::Offset(declaration->GetDx(), declaration->GetDy(), imageFilter);
#endif
#else
    imageFilter =
        RSRecordingImageFilter::CreateOffsetImageFilter(declaration->GetDx(), declaration->GetDy(), imageFilter);
#endif
    ConverImageFilterColor(imageFilter, srcColor, currentColor);
}

} // namespace OHOS::Ace::NG
