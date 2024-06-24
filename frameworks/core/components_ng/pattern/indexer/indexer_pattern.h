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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_INDEXER_INDEXER_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_INDEXER_INDEXER_PATTERN_H

#include <optional>
#include <stdint.h>

#include "base/memory/referenced.h"
#include "core/animation/animator.h"
#include "core/components/indexer/indexer_theme.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/pattern/indexer/indexer_accessibility_property.h"
#include "core/components_ng/pattern/indexer/indexer_event_hub.h"
#include "core/components_ng/pattern/indexer/indexer_layout_algorithm.h"
#include "core/components_ng/pattern/indexer/indexer_layout_property.h"
#include "core/components_ng/pattern/indexer/indexer_paint_property.h"
#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace::NG {
enum class IndexerCollapsingMode {
    INVALID,
    NONE, // all array should be displayed
    FIVE, // 5 + 1 collapsing mode
    SEVEN // 7 + 1 collapsing mode
};

enum class PopupListGradientStatus {
    NONE,
    TOP,
    BOTTOM,
    BOTH
};

class IndexerPattern : public Pattern {
    DECLARE_ACE_TYPE(IndexerPattern, Pattern);

public:
    IndexerPattern() = default;
    ~IndexerPattern() override = default;

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<IndexerEventHub>();
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<IndexerLayoutProperty>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<IndexerPaintProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        auto indexerLayoutAlgorithm = MakeRefPtr<IndexerLayoutAlgorithm>(static_cast<int32_t>(fullArrayValue_.size()));
        return indexerLayoutAlgorithm;
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<IndexerAccessibilityProperty>();
    }

    void SetIsTouch(bool isTouch)
    {
        isTouch_ = isTouch;
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, true };
    }

    int32_t GetSelected() const
    {
        return selected_;
    }

    bool IsMeasureBoundary() const override;
    void UpdateChildBoundary(RefPtr<FrameNode>& frameNode);
 
private:
    void OnModifyDone() override;
    void InitArrayValue(bool& autoCollapseModeChanged, bool& itemCountChanged);
    void InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub);
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    void DumpInfo() override;

    void BuildArrayValueItems();
    void BuildFullArrayValue();
    void CollapseArrayValue();
    void ApplySevenPlusOneMode(int32_t fullArraySize);
    void ApplyFivePlusOneMode(int32_t fullArraySize);
    int32_t GetAutoCollapseIndex(int32_t propSelect);

    void OnTouchDown(const TouchEventInfo& info);
    void OnTouchUp(const TouchEventInfo& info);
    void MoveIndexByOffset(const Offset& offset);
    bool MoveIndexByStep(int32_t step);
    bool KeyIndexByStep(int32_t step);
    bool MoveIndexBySearch(const std::string& searchStr);
    void ApplyIndexChanged(
        bool isTextNodeInTree, bool refreshBubble = true, bool fromTouchUp = false, bool indexerSizeChanged = false);
    void OnSelect(bool changed = false);
    int32_t GetSkipChildIndex(int32_t step);
    int32_t GetFocusChildIndex(const std::string& searchStr);

    void InitPanEvent(const RefPtr<GestureEventHub>& gestureHub);
    void InitInputEvent();
    void InitCurrentInputEvent();
    void InitChildInputEvent(RefPtr<FrameNode>& itemNode, int32_t childIndex);
    void InitPopupInputEvent();
    void InitPopupPanEvent();
    void InitOnKeyEvent();
    bool OnKeyEvent(const KeyEvent& event);
    void OnHover(bool isHover);
    void OnChildHover(int32_t index, bool isHover);
    void OnPopupHover(bool isHover);
    void ResetStatus();
    void OnKeyEventDisapear();
    void UpdateBubbleListItem(std::vector<std::string>& currentListData, const RefPtr<FrameNode>& parentNode,
        RefPtr<IndexerTheme>& indexerTheme);
    void AddPopupTouchListener(RefPtr<FrameNode> popupNode);
    void OnPopupTouchDown(const TouchEventInfo& info);
    void AddListItemClickListener(RefPtr<FrameNode>& listItemNode, int32_t index);
    void OnListItemClick(int32_t index);
    void ClearClickStatus();
    void ChangeListItemsSelectedStyle(int32_t clickIndex);
    RefPtr<FrameNode> CreatePopupNode();
    void UpdateBubbleView();
    void UpdateBubbleSize();
    void UpdateBubbleLetterView(bool showDivider, std::vector<std::string>& currentListData);
    void CreateBubbleListView(std::vector<std::string>& currentListData);
    void UpdateBubbleListView(std::vector<std::string>& currentListData);
    void UpdatePopupOpacity(float ratio);
    void UpdatePopupVisibility(VisibleType visible);
    bool NeedShowPopupView();
    bool NeedShowBubble();
    void ShowBubble();
    bool IfSelectIndexValid();
    int32_t GetSelectChildIndex(const Offset& offset);
    void StartBubbleAppearAnimation();
    void StartDelayTask(uint32_t duration = INDEXER_BUBBLE_WAIT_DURATION);
    void StartBubbleDisappearAnimation();
    void IndexerHoverInAnimation();
    void IndexerHoverOutAnimation();
    void IndexerPressInAnimation();
    void IndexerPressOutAnimation();
    int32_t GenerateAnimationId();
    void ItemSelectedInAnimation(RefPtr<FrameNode>& itemNode);
    void ItemSelectedOutAnimation(RefPtr<FrameNode>& itemNode);
    void FireOnSelect(int32_t selectIndex, bool fromPress);
    void SetAccessibilityAction();
    void RemoveBubble();
    void UpdateBubbleBackgroundView();
    CalcSize CalcBubbleListSize(int32_t popupSize, int32_t maxItemsSize);
    GradientColor CreatePercentGradientColor(float percent, Color color);
    void UpdateBubbleLetterStackAndLetterTextView();
    void DrawPopupListGradient(PopupListGradientStatus gradientStatus);
    void UpdatePopupListGradientView(int32_t popupSize, int32_t maxItemsSize);
    RefPtr<FrameNode> GetLetterNode();
    RefPtr<FrameNode> GetAutoCollapseLetterNode();
    void UpdateBubbleListSize(std::vector<std::string>& currentListData);
    void UpdateBubbleListItemContext(
        const RefPtr<FrameNode>& listNode, RefPtr<IndexerTheme>& indexerTheme, uint32_t pos);
    void UpdateBubbleListItemMarkModify(RefPtr<FrameNode>& textNode, RefPtr<FrameNode>& listItemNode);
    void StartCollapseDelayTask(RefPtr<FrameNode>& hostNode, uint32_t duration = INDEXER_COLLAPSE_WAIT_DURATION);

    RefPtr<FrameNode> popupNode_;
    RefPtr<TouchEventImpl> touchListener_;
    RefPtr<PanEvent> panEvent_;
    RefPtr<Animator> bubbleAnimator_;
    bool isInputEventRegisted_ = false;
    bool isKeyEventRegisted_ = false;
    bool isTouch_ = false;
    bool isHover_ = false;
    bool isPopup_ = false;
    bool isPopupHover_ = false;

     // the array of displayed items, ths second param in the pair
     // indicates whether the item should be hidden and displayed as dot
    std::vector<std::pair<std::string, bool>> arrayValue_;
    // full array of items, used in auto-collapse mode
    std::vector<std::string> fullArrayValue_;
    // sharp item count is 0 or 1, indicates whether the first item is # in
    // original array, used in auto-collapse mode
    int32_t sharpItemCount_ = 0;
    int32_t itemCount_ = 0;
    int32_t selected_ = 0;
    int32_t animateSelected_ = -1;
    int32_t lastSelected_ = -1;
    bool initialized_ = false;
    int32_t childHoverIndex_ = -1;
    int32_t childFocusIndex_ = -1;
    int32_t childPressIndex_ = -1;
    int32_t animationId_ = 0;
    int32_t lastPopupIndex_ = -1;
    uint32_t lastPopupSize_ = 0;
    int32_t currentPopupIndex_ = -1;
    float itemSizeRender_ = 0.0f;
    uint32_t popupClickedIndex_ = -1;
    int32_t lastFireSelectIndex_ = -1;
    float lastItemSize_ = -1.0f;
    bool lastIndexFromPress_ = false;
    bool selectChanged_ = false;
    bool autoCollapse_ = false;
    bool enableHapticFeedback_ = true;
    float actualIndexerHeight_ = 0.0f;
    bool isNewHeightCalculated_ = false;
    bool selectedChangedForHaptic_ = false;
    IndexerCollapsingMode lastCollapsingMode_ = IndexerCollapsingMode::INVALID;
    CancelableCallback<void()> delayTask_;
    CancelableCallback<void()> delayCollapseTask_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_PATTERN_H
