/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WATERFLOW_WATER_FLOW_SEGMENTED_LAYOUT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WATERFLOW_WATER_FLOW_SEGMENTED_LAYOUT_H

#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/pattern/waterflow/water_flow_layout_algorithm.h"
#include "core/components_ng/pattern/waterflow/water_flow_layout_info.h"
#include "core/components_ng/property/measure_property.h"

namespace OHOS::Ace::NG {
class WaterFlowSegmentedLayout : public WaterFlowLayoutBase {
    DECLARE_ACE_TYPE(WaterFlowSegmentedLayout, WaterFlowLayoutBase);

public:
    explicit WaterFlowSegmentedLayout(WaterFlowLayoutInfo layoutInfo) : info_(std::move(layoutInfo)) {}
    ~WaterFlowSegmentedLayout() override = default;

    void Measure(LayoutWrapper* layoutWrapper) override;

    void Layout(LayoutWrapper* layoutWrapper) override;

    WaterFlowLayoutInfo GetLayoutInfo() override
    {
        return std::move(info_);
    }

    void SetCanOverScroll(bool value) override
    {
        overScroll_ = value;
    }

private:
    /**
     * @brief Initialize member variables from LayoutProperty.
     *
     * @param frameSize of WaterFlow component.
     */
    void Init(const SizeF& frameSize);

    /**
     * @brief init regular WaterFlow with a single segment.
     *
     * @param frameSize
     */
    void RegularInit(const SizeF& frameSize);
    void InitFooter(float crossSize);

    /**
     * @brief Measure self before measuring children.
     *
     * @return [idealSize given by parent, whether measure is successful (need to adapt to children size if not)].
     */
    std::pair<SizeF, bool> PreMeasureSelf();

    /**
     * @brief Measure self after measuring children. Only when pre-measure failed.
     *
     * @param size ideal content size from parent.
     */
    void PostMeasureSelf(SizeF size);

    void MeasureOnOffset();

    /**
     * @brief Fills the viewport with new items when scrolling downwards.
     *
     * WaterFlow's item map only supports orderly forward layout,
     * because the position of a new item always depends on previous items.
     */
    void Fill();

    /**
     * @brief Helper to measure FlowItems.
     * Assumes the Item map is already filled and only call Measure on children in range [start, end).
     *
     * @param startIdx FlowItem index.
     * @param endIdx (exclusive)
     */
    void MeasureItems(int32_t startIdx, int32_t endIdx);

    /**
     * @brief Layout a FlowItem at [idx].
     *
     * @param idx FlowItem index.
     * @param padding top-left padding of WaterFlow component.
     * @param isReverse need to layout in reverse.
     */
    void LayoutItem(int32_t idx, const OffsetF& padding, bool isReverse);

    LayoutWrapper* wrapper_ {};

    std::vector<std::vector<float>> itemsCrossSize_;
    std::vector<std::vector<float>> itemsCrossPosition_;
    std::vector<PaddingPropertyF> margins_; // margin of each segment
    Axis axis_ = Axis::VERTICAL;

    std::vector<float> mainGaps_;
    std::vector<float> crossGaps_;
    float mainSize_ = 0.0f;
    WaterFlowLayoutInfo info_;

    // true if WaterFlow can be overScrolled
    bool overScroll_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(WaterFlowSegmentedLayout);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WATERFLOW_WATER_FLOW_SEGMENTED_LAYOUT_H
