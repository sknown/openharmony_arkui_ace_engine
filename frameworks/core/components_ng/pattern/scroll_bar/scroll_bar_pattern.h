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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_BAR_SCROLL_BAR_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_BAR_SCROLL_BAR_PATTERN_H

#include "base/geometry/axis.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/scroll/inner/scroll_bar.h"
#include "core/components_ng/pattern/scroll/scroll_event_hub.h"
#include "core/components_ng/pattern/scroll_bar/proxy/scroll_bar_proxy.h"
#include "core/components_ng/pattern/scroll_bar/scroll_bar_accessibility_property.h"
#include "core/components_ng/pattern/scroll_bar/scroll_bar_layout_algorithm.h"
#include "core/components_ng/pattern/scroll_bar/scroll_bar_layout_property.h"
#include "core/components_ng/pattern/scroll_bar/scroll_bar_paint_method.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/render/animation_utils.h"

namespace OHOS::Ace::NG {

class ScrollBarPattern : public Pattern {
    DECLARE_ACE_TYPE(ScrollBarPattern, Pattern);

public:
    ScrollBarPattern() = default;
    ~ScrollBarPattern() override
    {
        if (scrollBarProxy_) {
            scrollBarProxy_->UnRegisterScrollBar(AceType::WeakClaim(this));
        }
        scrollBarProxy_ = nullptr;
        scrollableEvent_ = nullptr;
        disappearAnimation_ = nullptr;
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<ScrollBarLayoutProperty>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<ScrollBarAccessibilityProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        auto layoutAlgorithm = MakeRefPtr<ScrollBarLayoutAlgorithm>(currentOffset_);
        return layoutAlgorithm;
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<ScrollEventHub>();
    }

    float GetCurrentPosition() const
    {
        return currentOffset_;
    }

    void SetCurrentPosition(float currentOffset)
    {
        currentOffset_ = currentOffset;
    }

    Axis GetAxis() const
    {
        return axis_;
    }

    float GetScrollableDistance() const
    {
        return scrollableDistance_;
    }

    void SetScrollBarProxy(const RefPtr<ScrollBarProxy>& scrollBarProxy)
    {
        scrollBarProxy_ = scrollBarProxy;
    }

    const DisplayMode& GetDisplayMode() const
    {
        return displayMode_;
    }

    float GetControlDistance() const
    {
        return controlDistance_;
    }

    void SetControlDistance(float controlDistance)
    {
        controlDistanceChanged_ = Positive(controlDistance_) ? !Positive(controlDistance) : Positive(controlDistance);
        controlDistance_ = controlDistance;
    }

    float GetScrollableNodeOffset() const
    {
        return scrollableNodeOffset_;
    }

    void SetScrollableNodeOffset(float scrollableNodeOffset)
    {
        scrollableNodeOffset_ = scrollableNodeOffset;
    }

    bool IsAtTop() const;
    bool IsAtBottom() const;
    bool UpdateCurrentOffset(float offset, int32_t source);

    /**
     * @brief Stops the motion animator of the scroll bar.
     */
    inline void StopMotion()
    {
        if (frictionController_ && frictionController_->IsRunning()) {
            frictionController_->Stop();
        }
    }
    // disappear Animator
    void StartDisappearAnimator();
    void StopDisappearAnimator();
    void SetOpacity(uint8_t value);
    void SendAccessibilityEvent(AccessibilityEventType eventType);

    void SetChildOffset(float childOffset)
    {
        childOffset_ = childOffset;
    };

    float GetChildOffset() const
    {
        return childOffset_;
    };

    void SetChildRect(const RectF& rect)
    {
        childRect_ = rect;
    }

    void SetDisappearAnimation(const std::shared_ptr<AnimationUtils::Animation>& disappearAnimation)
    {
        disappearAnimation_ = disappearAnimation;
    }

    void OnCollectTouchTarget(const OffsetF& coordinateOffset, const GetEventTargetImpl& getEventTargetImpl,
        TouchTestResult& result, const RefPtr<FrameNode>& frameNode, const RefPtr<TargetComponent>& targetComponent,
        TouchTestResult& responseLinkResult);

    float GetMainOffset(const Offset& offset) const
    {
        return axis_ == Axis::HORIZONTAL ? offset.GetX() : offset.GetY();
    }

    void SetDragStartPosition(float position)
    {
        dragStartPosition_ = position;
    }

    void SetDragEndPosition(float position)
    {
        dragEndPosition_ = position;
    }

    float GetDragOffset()
    {
        return dragEndPosition_ - dragStartPosition_;
    }

    void InitClickEvent();
    void HandleClickEvent(GestureEvent& info);
    void InitLongPressEvent();
    void HandleLongPress(bool smooth);
    void InitMouseEvent();
    bool IsInScrollBar();
    void ScheduleCaretLongPress();
    void StartLongPressEventTimer();
    void OnCollectLongPressTarget(const OffsetF& coordinateOffset, const GetEventTargetImpl& getEventTargetImpl,
        TouchTestResult& result, const RefPtr<FrameNode>& frameNode, const RefPtr<TargetComponent>& targetComponent,
        TouchTestResult& responseLinkResult);
    void SetScrollBar(DisplayMode displayMode);
    void UpdateScrollBarOffset();
    void HandleScrollBarOutBoundary(float scrollBarOutBoundaryExtent);
    void UpdateScrollBarRegion(float offset, float estimatedHeight, Size viewPort, Offset viewOffset);
    void RegisterScrollBarEventTask();
    bool UpdateScrollBarDisplay();
    bool IsReverse() const;
    void SetReverse(bool reverse);

    RefPtr<GestureEventHub> GetGestureHub()
    {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, nullptr);
        auto hub = host->GetEventHub<EventHub>();
        CHECK_NULL_RETURN(hub, nullptr);
        return hub->GetOrCreateGestureEventHub();
    }

    RefPtr<InputEventHub> GetInputHub()
    {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, nullptr);
        auto hub = host->GetEventHub<EventHub>();
        CHECK_NULL_RETURN(hub, nullptr);
        return hub->GetOrCreateInputEventHub();
    }

    void CreateScrollBarOverlayModifier()
    {
        CHECK_NULL_VOID(scrollBar_ && scrollBar_->NeedPaint());
        CHECK_NULL_VOID(!scrollBarOverlayModifier_);
        scrollBarOverlayModifier_ = AceType::MakeRefPtr<ScrollBarOverlayModifier>();
        scrollBarOverlayModifier_->SetRect(scrollBar_->GetActiveRect());
        scrollBarOverlayModifier_->SetPositionMode(scrollBar_->GetPositionMode());
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        if (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
            auto paint = MakeRefPtr<ScrollBarPaintMethod>(hasChild_);
            paint->SetScrollBar(scrollBar_);
            if (!HasChild()) {
                auto activeRect = scrollBar_->GetActiveRect();
                auto offset = activeRect.GetOffset();
                auto offsetF = OffsetF(offset.GetX(), offset.GetY());
                auto size = activeRect.GetSize();
                auto sizeF = SizeF(size.Width(), size.Height());
                childRect_ = RectF(offsetF, sizeF);
            }
            CreateScrollBarOverlayModifier();
            paint->SetScrollBarOverlayModifier(scrollBarOverlayModifier_);
            return paint;
        } else {
            return Pattern::CreateNodePaintMethod();
        }
    }

    void SetChild(bool hasChild)
    {
        hasChild_ = hasChild;
    }

    void SetPreFrameChildState(bool preFrameChildState)
    {
        preFrameChildState_ = preFrameChildState;
    }

    bool GetPreFrameChildState()
    {
        return preFrameChildState_;
    }

    bool HasChild()
    {
        return hasChild_;
    }

    bool CheckChildState()
    {
        auto currentChildState = HasChild();
        auto preChildState = GetPreFrameChildState();
        if (preChildState != currentChildState) {
            SetPreFrameChildState(currentChildState);
            return true;
        }
        return false;
    }

    void AddScrollBarLayoutInfo();

    void GetAxisDumpInfo();

    void GetDisplayModeDumpInfo();

    void GetPanDirectionDumpInfo();

    void DumpAdvanceInfo() override;

    void SetScrollEnabled(bool enabled)
    {
        CHECK_NULL_VOID(scrollableEvent_);
        scrollableEvent_->SetEnabled(enabled);
        if (!enabled) {
            scrollableEvent_->SetAxis(Axis::NONE);
        } else {
            scrollableEvent_->SetAxis(axis_);
        }
    }

private:
    void OnModifyDone() override;
    void OnAttachToFrameNode() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    void ValidateOffset();
    void SetAccessibilityAction();
    void InitPanRecognizer();
    void HandleDragStart(const GestureEvent& info);
    void HandleDragUpdate(const GestureEvent& info);
    void HandleDragEnd(const GestureEvent& info);
    void ProcessFrictionMotion(double value);
    void ProcessFrictionMotionStop();

    RefPtr<ScrollBarProxy> scrollBarProxy_;
    RefPtr<ScrollableEvent> scrollableEvent_;
    Axis axis_ = Axis::VERTICAL;
    DisplayMode displayMode_ { DisplayMode::AUTO };
    float currentOffset_ = 0.0f;
    float lastOffset_ = 0.0f;
    float scrollableDistance_ = 0.0f;
    float controlDistance_ = 0.0f;
    bool  controlDistanceChanged_ = false;
    bool hasChild_ = false;
    bool preFrameChildState_ = false;
    float scrollableNodeOffset_  = 0.0f;
    float friction_ = BAR_FRICTION;
    float frictionPosition_ = 0.0;
    float dragStartPosition_ = 0.0f;
    float dragEndPosition_ = 0.0f;

    RefPtr<ScrollBarOverlayModifier> scrollBarOverlayModifier_;
    RefPtr<ScrollBar> scrollBar_;

    float childOffset_ = 0.0f;  // main size of child
    RefPtr<PanRecognizer> panRecognizer_;
    RefPtr<FrictionMotion> frictionMotion_;
    RefPtr<Animator> frictionController_;
    ScrollPositionCallback scrollPositionCallback_;
    ScrollEndCallback scrollEndCallback_;
    RectF childRect_;
    uint8_t opacity_ = UINT8_MAX;
    CancelableCallback<void()> disapplearDelayTask_;
    std::shared_ptr<AnimationUtils::Animation> disappearAnimation_;
    bool isMousePressed_ = false;
    RefPtr<ClickEvent> clickListener_;
    RefPtr<LongPressRecognizer> longPressRecognizer_;
    RefPtr<InputEvent> mouseEvent_;
    Offset locationInfo_;
    //Determine whether the current scroll direction is scrolling upwards or downwards
    bool scrollingUp_ = false;
    bool scrollingDown_ = false;
    bool isReverse_ = false;

    // dump info
    std::list<OuterScrollBarLayoutInfo> outerScrollBarLayoutInfos_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_BAR_SCROLL_BAR_PATTERN_H
