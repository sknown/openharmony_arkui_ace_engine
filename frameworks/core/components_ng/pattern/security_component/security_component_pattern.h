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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SECURITY_COMPONENT_SECURITY_COMPONENT_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SECURITY_COMPONENT_SECURITY_COMPONENT_PATTERN_H

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/security_component/security_component_accessibility_property.h"
#include "core/components_ng/pattern/security_component/security_component_layout_algorithm.h"
#include "core/components_ng/pattern/security_component/security_component_layout_property.h"
#include "core/components_ng/pattern/security_component/security_component_paint_property.h"
#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace::NG {
constexpr int32_t DEFAULT_SECURITY_COMPONENT_CLICK_DISTANCE = 15;
constexpr uint64_t MAX_REGISTER_WAITING_TIME = 3000; // 3000ms
constexpr int32_t MAX_RETRY_TIMES = 3;
constexpr int64_t REGISTER_RETRY_INTERVAL = 30; // 30ms

static inline RefPtr<FrameNode> GetSecCompChildNode(RefPtr<FrameNode>& parent, const std::string& tag)
{
    for (const auto& child : parent->GetChildren()) {
        auto node = AceType::DynamicCast<FrameNode, UINode>(child);
        CHECK_NULL_RETURN(node, nullptr);
        if (node->GetTag() == tag) {
            return node;
        }
    }
    return nullptr;
}

static inline RefPtr<FrameNode> GetCurSecCompChildNode(const std::string& tag)
{
    auto frameNode = AceType::Claim(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    CHECK_NULL_RETURN(frameNode, nullptr);
    return GetSecCompChildNode(frameNode, tag);
}

class SecurityComponentPattern : public Pattern {
    DECLARE_ACE_TYPE(SecurityComponentPattern, Pattern);

public:
    SecurityComponentPattern() {};
    ~SecurityComponentPattern() override;

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<SecurityComponentLayoutProperty>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<SecurityComponentPaintProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<SecurityComponentLayoutAlgorithm>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<SecurityComponentAccessibilityProperty>();
    }

    FocusPattern GetFocusPattern() const override;

    void OnWindowHide() override;
    void OnWindowShow() override;

    SecurityComponentRegisterStatus regStatus_ = SecurityComponentRegisterStatus::UNREGISTERED;
    std::timed_mutex regMutex_;
    int32_t scId_ = -1;
    bool isAppear_ = true;

protected:
    void InitOnTouch(RefPtr<FrameNode>& secCompNode);
    void InitOnKeyEvent(RefPtr<FrameNode>& secCompNode);
    bool OnKeyEvent(const KeyEvent& event);
    void OnTouch(const TouchEventInfo& info);
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    void OnModifyDone() override;
    void SetNodeHitTestMode(RefPtr<FrameNode>& node, HitTestMode mode);
    void InitOnClick(RefPtr<FrameNode>& secCompNode, RefPtr<FrameNode>& icon,
        RefPtr<FrameNode>& text, RefPtr<FrameNode>& button);
    void InitAppearCallback(RefPtr<FrameNode>& frameNode);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override;
    void ToJsonValueRect(std::unique_ptr<JsonValue>& json) const;
    bool IsParentMenu(RefPtr<FrameNode>& secCompNode);
private:
    void HandleClickEventFromTouch(const TouchEventInfo& info);
    void UpdateIconProperty(RefPtr<FrameNode>& scNode, RefPtr<FrameNode>& iconNode);
    void UpdateTextProperty(RefPtr<FrameNode>& scNode, RefPtr<FrameNode>& textNode);
    void UpdateButtonProperty(RefPtr<FrameNode>& scNode, RefPtr<FrameNode>& buttonNode);
#ifdef SECURITY_COMPONENT_ENABLE
    void RegisterSecurityComponentAsync();
    void RegisterSecurityComponent();
    void RegisterSecurityComponentRetry();
    void UnregisterSecurityComponent();
    int32_t ReportSecurityComponentClickEvent(GestureEvent& event);
    int32_t ReportSecurityComponentClickEvent(const KeyEvent& event);
    void DoTriggerOnclick(int32_t result);
    void TriggerOnclick(int32_t instance_, int32_t result);
#endif
    std::unique_ptr<Offset> lastTouchOffset_;
    RefPtr<ClickEvent> clickListener_;
    RefPtr<TouchEventImpl> onTouchListener_;
    bool isSetOnKeyEvent = false;
    bool isAppearCallback_ = false;
    ACE_DISALLOW_COPY_AND_MOVE(SecurityComponentPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SECURITY_COMPONENT_SECURITY_COMPONENT_PATTERN_H
