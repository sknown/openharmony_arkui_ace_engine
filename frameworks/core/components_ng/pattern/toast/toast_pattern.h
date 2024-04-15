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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TOAST_TOAST_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TOAST_TOAST_PATTERN_H

#include <cstdint>

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components/toast/toast_theme.h"
#include "core/components_ng/pattern/overlay/popup_base_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/toast/toast_layout_algorithm.h"
#include "core/components_ng/pattern/toast/toast_layout_property.h"

namespace OHOS::Ace::NG {
class ToastPattern : public PopupBasePattern {
    DECLARE_ACE_TYPE(ToastPattern, PopupBasePattern);

public:
    ToastPattern() = default;
    ~ToastPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return AceType::MakeRefPtr<ToastLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return AceType::MakeRefPtr<ToastLayoutAlgorithm>();
    }

    void OnAttachToFrameNode() override;

    void OnDetachFromFrameNode(FrameNode* node) override;

    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& changeConfig) override;

    void OnColorConfigurationUpdate() override;

    void SetTextNode(RefPtr<FrameNode> textNode)
    {
        textNode_ = textNode;
    }

    bool IsDefaultToast() const
    {
        auto layoutProp = GetLayoutProperty<ToastLayoutProperty>();
        CHECK_NULL_RETURN(layoutProp, false);
        auto showMode = layoutProp->GetShowModeValue(ToastShowMode::DEFAULT);
        return showMode == ToastShowMode::DEFAULT;
    }

    bool AvoidKeyboard() const override
    {
        return IsDefaultToast();
    }

    void UpdateFoldDisplayModeChangedCallbackId(std::optional<int32_t> id)
    {
        foldDisplayModeChangedCallbackId_ = id;
    }

    bool HasFoldDisplayModeChangedCallbackId()
    {
        return foldDisplayModeChangedCallbackId_.has_value();
    }
private:
    void BeforeCreateLayoutWrapper() override;
    void UpdateToastSize(const RefPtr<FrameNode>& toast);
    void UpdateTextSizeConstraint(const RefPtr<FrameNode>& text);
    Dimension GetOffsetX(const RefPtr<LayoutWrapper>& layoutWrapper);
    Dimension GetOffsetY(const RefPtr<LayoutWrapper>& layoutWrapper);

    double GetBottomValue(const RefPtr<LayoutWrapper>& layoutWrapper);
    double GetTextMaxHeight();
    double GetTextMaxWidth();
    int32_t GetTextLineHeight(const RefPtr<FrameNode>& textNode);

    RefPtr<FrameNode> textNode_;
    std::optional<int32_t> foldDisplayModeChangedCallbackId_;
    ACE_DISALLOW_COPY_AND_MOVE(ToastPattern);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TOAST_TOAST_PATTERN_H
