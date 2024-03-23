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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_FRAME_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_FRAME_NODE_H

#include <functional>
#include <list>
#include <utility>

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/point_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/thread/cancelable_callback.h"
#include "base/thread/task_executor.h"
#include "base/utils/macros.h"
#include "base/utils/utils.h"
#include "core/accessibility/accessibility_utils.h"
#include "core/common/recorder/exposure_processor.h"
#include "core/common/resource/resource_configuration.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_scene_status.h"
#include "core/components_ng/base/geometry_node.h"
#include "core/components_ng/base/modifier.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/event/input_event_hub.h"
#include "core/components_ng/event/target_component.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/property/accessibility_property.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/paint_property.h"
#include "core/components_ng/render/paint_wrapper.h"
#include "core/components_ng/render/render_context.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/components_v2/inspector/inspector_node.h"

namespace OHOS::Accessibility {
class AccessibilityElementInfo;
class AccessibilityEventInfo;
} // namespace OHOS::Accessibility

namespace OHOS::Ace::NG {
class PipelineContext;
class Pattern;
class StateModifyTask;
class UITask;
class FrameProxy;

// FrameNode will display rendering region in the screen.
class ACE_FORCE_EXPORT FrameNode : public UINode, public LayoutWrapper {
    DECLARE_ACE_TYPE(FrameNode, UINode, LayoutWrapper);

public:
    // create a new child element with new element tree.
    static RefPtr<FrameNode> CreateFrameNodeWithTree(
        const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern);

    static RefPtr<FrameNode> GetOrCreateFrameNode(
        const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator);

    // create a new element with new pattern.
    static RefPtr<FrameNode> CreateFrameNode(
        const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern, bool isRoot = false);

    // get element with nodeId from node map.
    static RefPtr<FrameNode> GetFrameNode(const std::string& tag, int32_t nodeId);

    static void ProcessOffscreenNode(const RefPtr<FrameNode>& node);
    // avoid use creator function, use CreateFrameNode
    FrameNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern, bool isRoot = false);

    ~FrameNode() override;

    int32_t FrameCount() const override
    {
        return 1;
    }

    void SetCheckboxFlag(const bool checkboxFlag)
    {
        checkboxFlag_ = checkboxFlag;
    }

    bool GetCheckboxFlag() const
    {
        return checkboxFlag_;
    }
    void OnInspectorIdUpdate(const std::string& id) override;

    struct ZIndexComparator {
        bool operator()(const WeakPtr<FrameNode>& weakLeft, const WeakPtr<FrameNode>& weakRight) const
        {
            auto left = weakLeft.Upgrade();
            auto right = weakRight.Upgrade();
            if (left && right) {
                return left->GetRenderContext()->GetZIndexValue(ZINDEX_DEFAULT_VALUE) <
                       right->GetRenderContext()->GetZIndexValue(ZINDEX_DEFAULT_VALUE);
            }
            return false;
        }
    };

    const std::multiset<WeakPtr<FrameNode>, ZIndexComparator>& GetFrameChildren() const
    {
        return frameChildren_;
    }

    void InitializePatternAndContext();

    virtual void MarkModifyDone();

    void MarkDirtyNode(
        PropertyChangeFlag extraFlag = PROPERTY_UPDATE_NORMAL, bool childExpansiveAndMark = false) override;

    void MarkDirtyNode(bool isMeasureBoundary, bool isRenderBoundary,
        PropertyChangeFlag extraFlag = PROPERTY_UPDATE_NORMAL, bool childExpansiveAndMark = false);

    void ProcessPropertyDiff()
    {
        // TODO: modify done need to optimize.
        MarkModifyDone();
        MarkDirtyNode();
        isPropertyDiffMarked_ = false;
    }

    void FlushUpdateAndMarkDirty() override;

    void MarkNeedFrameFlushDirty(PropertyChangeFlag extraFlag = PROPERTY_UPDATE_NORMAL) override
    {
        MarkDirtyNode(extraFlag);
    }

    void OnMountToParentDone();

    void UpdateLayoutConstraint(const MeasureProperty& calcLayoutConstraint);

    RefPtr<LayoutWrapperNode> CreateLayoutWrapper(bool forceMeasure = false, bool forceLayout = false) override;

    RefPtr<LayoutWrapperNode> UpdateLayoutWrapper(
        RefPtr<LayoutWrapperNode> layoutWrapper, bool forceMeasure = false, bool forceLayout = false);

    void CreateLayoutTask(bool forceUseMainThread = false);

    std::optional<UITask> CreateRenderTask(bool forceUseMainThread = false);

    void SwapDirtyLayoutWrapperOnMainThread(const RefPtr<LayoutWrapper>& dirty);

    // Clear the user callback.
    void ClearUserOnAreaChange();

    void SetOnAreaChangeCallback(OnAreaChangedFunc&& callback);

    void TriggerOnAreaChangeCallback(uint64_t nanoTimestamp);

    void OnConfigurationUpdate(const ConfigurationChange& configurationChange) override;

    void SetVisibleAreaUserCallback(const std::vector<double>& ratios, const VisibleCallbackInfo& callback)
    {
        eventHub_->SetVisibleAreaRatios(ratios, true);
        eventHub_->SetVisibleAreaCallback(callback, true);
    }

    void CleanVisibleAreaUserCallback()
    {
        eventHub_->CleanVisibleAreaCallback(true);
    }

    void SetVisibleAreaInnerCallback(const std::vector<double>& ratios, const VisibleCallbackInfo& callback)
    {
        eventHub_->SetVisibleAreaRatios(ratios, false);
        eventHub_->SetVisibleAreaCallback(callback, false);
    }

    void CleanVisibleAreaInnerCallback()
    {
        eventHub_->CleanVisibleAreaCallback(false);
    }

    void TriggerVisibleAreaChangeCallback(bool forceDisappear = false);

    void SetOnSizeChangeCallback(OnSizeChangedFunc&& callback);

    void TriggerOnSizeChangeCallback();

    void SetGeometryNode(const RefPtr<GeometryNode>& node);

    const RefPtr<RenderContext>& GetRenderContext() const
    {
        return renderContext_;
    }

    const RefPtr<Pattern>& GetPattern() const;

    template<typename T>
    T* GetPatternPtr() const
    {
        return reinterpret_cast<T*>(RawPtr(pattern_));
    }

    template<typename T>
    RefPtr<T> GetPattern() const
    {
        return DynamicCast<T>(pattern_);
    }

    template<typename T>
    RefPtr<T> GetAccessibilityProperty() const
    {
        return DynamicCast<T>(accessibilityProperty_);
    }

    template<typename T>
    T* GetLayoutPropertyPtr() const
    {
        return reinterpret_cast<T*>(RawPtr(layoutProperty_));
    }

    template<typename T>
    RefPtr<T> GetLayoutProperty() const
    {
        return DynamicCast<T>(layoutProperty_);
    }

    template<typename T>
    T* GetPaintPropertyPtr() const
    {
        return reinterpret_cast<T*>(RawPtr(paintProperty_));
    }

    template<typename T>
    RefPtr<T> GetPaintProperty() const
    {
        return DynamicCast<T>(paintProperty_);
    }

    template<typename T>
    RefPtr<T> GetEventHub() const
    {
        return DynamicCast<T>(eventHub_);
    }

    RefPtr<GestureEventHub> GetOrCreateGestureEventHub() const
    {
        return eventHub_->GetOrCreateGestureEventHub();
    }

    RefPtr<InputEventHub> GetOrCreateInputEventHub() const
    {
        return eventHub_->GetOrCreateInputEventHub();
    }

    RefPtr<FocusHub> GetOrCreateFocusHub() const;

    RefPtr<FocusHub> GetFocusHub() const
    {
        return eventHub_->GetFocusHub();
    }

    FocusType GetFocusType() const
    {
        FocusType type = FocusType::DISABLE;
        auto focusHub = GetFocusHub();
        if (focusHub) {
            type = focusHub->GetFocusType();
        }
        return type;
    }

    static void PostTask(std::function<void()>&& task, TaskExecutor::TaskType taskType = TaskExecutor::TaskType::UI);

    void AddJudgeToTargetComponent(RefPtr<TargetComponent>& targetComponent);

    // If return true, will prevent TouchTest Bubbling to parent and brother nodes.
    HitTestResult TouchTest(const PointF& globalPoint, const PointF& parentLocalPoint, const PointF& parentRevertPoint,
        TouchRestrict& touchRestrict, TouchTestResult& result, int32_t touchId, bool isDispatch = false) override;

    HitTestResult MouseTest(const PointF& globalPoint, const PointF& parentLocalPoint, MouseTestResult& onMouseResult,
        MouseTestResult& onHoverResult, RefPtr<FrameNode>& hoverNode) override;

    HitTestResult AxisTest(
        const PointF& globalPoint, const PointF& parentLocalPoint, AxisTestResult& onAxisResult) override;

    void CheckSecurityComponentStatus(std::vector<RectF>& rect);

    bool HaveSecurityComponent();

    bool IsSecurityComponent();

    void AnimateHoverEffect(bool isHovered) const;

    bool IsAtomicNode() const override;

    void MarkNeedSyncRenderTree(bool needRebuild = false) override;

    void RebuildRenderContextTree() override;

    bool IsVisible() const
    {
        return layoutProperty_->GetVisibility().value_or(VisibleType::VISIBLE) == VisibleType::VISIBLE;
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override;

    void FromJson(const std::unique_ptr<JsonValue>& json) override;

    RefPtr<FrameNode> GetAncestorNodeOfFrame(bool checkBoundary = false) const;

    std::string& GetNodeName()
    {
        return nodeName_;
    }

    void SetNodeName(std::string& nodeName)
    {
        nodeName_ = nodeName;
    }

    void OnWindowShow() override;

    void OnWindowHide() override;

    void OnWindowFocused() override;

    void OnWindowUnfocused() override;

    void OnWindowSizeChanged(int32_t width, int32_t height, WindowSizeChangeReason type) override;

    void OnNotifyMemoryLevel(int32_t level) override;

    // call by recycle framework.
    void OnRecycle() override;
    void OnReuse() override;

    OffsetF GetOffsetRelativeToWindow() const;

    OffsetF GetTransformRelativeOffset() const;

    RectF GetTransformRectRelativeToWindow() const;

    OffsetF GetPaintRectOffset(bool excludeSelf = false) const;

    OffsetF GetPaintRectCenter() const;

    std::pair<OffsetF, bool> GetPaintRectGlobalOffsetWithTranslate(bool excludeSelf = false) const;

    OffsetF GetPaintRectOffsetToPage() const;

    RectF GetPaintRectWithTransform() const;

    VectorF GetTransformScale() const;

    void AdjustGridOffset();

    bool IsInternal() const
    {
        return isInternal_;
    }

    void SetInternal()
    {
        isInternal_ = true;
    }

    int32_t GetAllDepthChildrenCount();

    void OnAccessibilityEvent(
        AccessibilityEventType eventType, WindowsContentChangeTypes windowsContentChangeType =
                                              WindowsContentChangeTypes::CONTENT_CHANGE_TYPE_INVALID) const;

    void OnAccessibilityEvent(
        AccessibilityEventType eventType, std::string beforeText, std::string latestContent) const;

    void MarkNeedRenderOnly();

    void OnDetachFromMainTree(bool recursive) override;
    void OnAttachToMainTree(bool recursive) override;
    void OnAttachToBuilderNode(NodeStatus nodeStatus) override;

    void OnVisibleChange(bool isVisible) override;

    void PushDestroyCallback(std::function<void()>&& callback)
    {
        destroyCallbacks_.emplace_back(callback);
    }

    bool MarkRemoving() override;

    void AddHotZoneRect(const DimensionRect& hotZoneRect) const;
    void RemoveLastHotZoneRect() const;

    virtual bool IsOutOfTouchTestRegion(const PointF& parentLocalPoint, int32_t sourceType);
    bool CheckRectIntersect(const RectF& dest, std::vector<RectF>& origin);

    bool IsLayoutDirtyMarked() const
    {
        return isLayoutDirtyMarked_;
    }

    void SetLayoutDirtyMarked(bool marked)
    {
        isLayoutDirtyMarked_ = marked;
    }

    bool HasPositionProp() const
    {
        CHECK_NULL_RETURN(renderContext_, false);
        return renderContext_->HasPosition() || renderContext_->HasOffset() || renderContext_->HasPositionEdges() ||
               renderContext_->HasOffsetEdges() || renderContext_->HasAnchor();
    }

    // The function is only used for fast preview.
    void FastPreviewUpdateChildDone() override
    {
        OnMountToParentDone();
    }

    bool IsExclusiveEventForChild() const
    {
        return exclusiveEventForChild_;
    }

    void SetExclusiveEventForChild(bool exclusiveEventForChild)
    {
        exclusiveEventForChild_ = exclusiveEventForChild;
    }

    void SetDraggable(bool draggable)
    {
        draggable_ = draggable;
        userSet_ = true;
        customerSet_ = false;
    }

    void SetCustomerDraggable(bool draggable)
    {
        draggable_ = draggable;
        userSet_ = true;
        customerSet_ = true;
    }

    void SetDragPreviewOptions(const DragPreviewOption& previewOption)
    {
        previewOption_ = previewOption;
    }

    DragPreviewOption GetDragPreviewOption() const
    {
        return previewOption_;
    }

    void SetBackgroundFunction(std::function<RefPtr<UINode>()>&& buildFunc)
    {
        builderFunc_ = std::move(buildFunc);
        backgroundNode_ = nullptr;
    }

    bool IsDraggable() const
    {
        return draggable_;
    }

    bool IsLayoutComplete() const
    {
        return isLayoutComplete_;
    }

    bool IsUserSet() const
    {
        return userSet_;
    }

    bool IsCustomerSet() const
    {
        return customerSet_;
    }

    void SetAllowDrop(const std::set<std::string>& allowDrop)
    {
        allowDrop_ = allowDrop;
    }

    const std::set<std::string>& GetAllowDrop() const
    {
        return allowDrop_;
    }

    void SetDrawModifier(const RefPtr<NG::DrawModifier>& drawModifier)
    {
        drawModifier_ = drawModifier;
        auto contentModifier = GetContentModifier();
        if (contentModifier) {
            contentModifier->SetDrawModifier(drawModifier);
        }
    }

    bool IsSupportDrawModifier();

    void SetDragPreview(const NG::DragDropInfo& info)
    {
        dragPreviewInfo_ = info;
    }

    const DragDropInfo& GetDragPreview() const
    {
        return dragPreviewInfo_;
    }

    void SetOverlayNode(const RefPtr<FrameNode>& overlayNode)
    {
        overlayNode_ = overlayNode;
    }

    RefPtr<FrameNode> GetOverlayNode() const
    {
        return overlayNode_;
    }

    RefPtr<FrameNode> FindChildByPosition(float x, float y);

    RefPtr<NodeAnimatablePropertyBase> GetAnimatablePropertyFloat(const std::string& propertyName) const;
    static RefPtr<FrameNode> FindChildByName(const RefPtr<FrameNode>& parentNode, const std::string& nodeName);
    void CreateAnimatablePropertyFloat(const std::string& propertyName, float value,
        const std::function<void(float)>& onCallbackEvent, const PropertyUnit& propertyType = PropertyUnit::UNKNOWN);
    void DeleteAnimatablePropertyFloat(const std::string& propertyName);
    void UpdateAnimatablePropertyFloat(const std::string& propertyName, float value);
    void CreateAnimatableArithmeticProperty(const std::string& propertyName, RefPtr<CustomAnimatableArithmetic>& value,
        std::function<void(const RefPtr<CustomAnimatableArithmetic>&)>& onCallbackEvent);
    void UpdateAnimatableArithmeticProperty(const std::string& propertyName, RefPtr<CustomAnimatableArithmetic>& value);

    void SetHitTestMode(HitTestMode mode);
    HitTestMode GetHitTestMode() const override;

    TouchResult GetOnChildTouchTestRet(const std::vector<TouchTestInfo>& touchInfo);
    OnChildTouchTestFunc GetOnTouchTestFunc();
    void CollectTouchInfos(
        const PointF& globalPoint, const PointF& parentRevertPoint, std::vector<TouchTestInfo>& touchInfos);
    RefPtr<FrameNode> GetDispatchFrameNode(const TouchResult& touchRes);

    std::string ProvideRestoreInfo();

    static std::vector<RefPtr<FrameNode>> GetNodesById(const std::unordered_set<int32_t>& set);
    static std::vector<FrameNode*> GetNodesPtrById(const std::unordered_set<int32_t>& set);

    double GetPreviewScaleVal() const;

    bool IsPreviewNeedScale() const;

    void SetViewPort(RectF viewPort)
    {
        viewPort_ = viewPort;
    }

    std::optional<RectF> GetSelfViewPort() const
    {
        return viewPort_;
    }

    std::optional<RectF> GetViewPort() const;

    // Frame Rate Controller(FRC) decides FrameRateRange by scene, speed and scene status
    // speed is measured by millimeter/second
    void AddFRCSceneInfo(const std::string& scene, float speed, SceneStatus status);

    OffsetF GetParentGlobalOffsetDuringLayout() const;
    void OnSetCacheCount(int32_t cacheCount, const std::optional<LayoutConstraintF>& itemConstraint) override {};

    // layoutwrapper function override
    const RefPtr<LayoutAlgorithmWrapper>& GetLayoutAlgorithm(bool needReset = false) override;

    void Measure(const std::optional<LayoutConstraintF>& parentConstraint) override;

    // Called to perform layout children.
    void Layout() override;

    int32_t GetTotalChildCount() const override
    {
        return UINode::TotalChildCount();
    }

    const RefPtr<GeometryNode>& GetGeometryNode() const override
    {
        return geometryNode_;
    }

    void SetLayoutProperty(const RefPtr<LayoutProperty>& layoutProperty)
    {
        layoutProperty_ = layoutProperty;
        layoutProperty_->SetHost(WeakClaim(this));
    }

    const RefPtr<LayoutProperty>& GetLayoutProperty() const override
    {
        return layoutProperty_;
    }

    RefPtr<LayoutWrapper> GetOrCreateChildByIndex(
        uint32_t index, bool addToRenderTree = true, bool isCache = false) override;
    RefPtr<LayoutWrapper> GetChildByIndex(uint32_t index, bool isCache = false) override;
    /**
     * @brief Get the index of Child among all FrameNode children of [this].
     * Handles intermediate SyntaxNodes like LazyForEach.
     *
     * @param child pointer to the Child FrameNode.
     * @return index of Child, or -1 if not found.
     */
    int32_t GetChildTrueIndex(const RefPtr<LayoutWrapper>& child) const;
    uint32_t GetChildTrueTotalCount() const;
    const std::list<RefPtr<LayoutWrapper>>& GetAllChildrenWithBuild(bool addToRenderTree = true) override;
    void RemoveChildInRenderTree(uint32_t index) override;
    void RemoveAllChildInRenderTree() override;
    void DoRemoveChildInRenderTree(uint32_t index, bool isAll) override;
    void SetActiveChildRange(int32_t start, int32_t end) override;
    void DoSetActiveChildRange(int32_t start, int32_t end) override;
    const std::string& GetHostTag() const override
    {
        return GetTag();
    }

    bool SelfOrParentExpansive();
    bool SelfExpansive();
    bool ParentExpansive();
    void SetNeedRestoreSafeArea(bool needRestore)
    {
        needRestoreSafeArea_ = needRestore;
    }
    bool NeedRestoreSafeArea()
    {
        return needRestoreSafeArea_;
    }

    bool IsActive() const override
    {
        return isActive_;
    }

    void SetActive(bool active = true) override;

    bool GetBypass() const
    {
        return bypass_;
    }

    bool IsOutOfLayout() const override
    {
        return renderContext_->HasPosition() || renderContext_->HasPositionEdges();
    }

    bool SkipMeasureContent() const override;
    float GetBaselineDistance() const override;
    void SetCacheCount(
        int32_t cacheCount = 0, const std::optional<LayoutConstraintF>& itemConstraint = std::nullopt) override;

    void SyncGeometryNode(bool needSkipSync = false);
    RefPtr<UINode> GetFrameChildByIndex(uint32_t index, bool needBuild, bool isCache = false) override;
    bool CheckNeedForceMeasureAndLayout() override;

    bool SetParentLayoutConstraint(const SizeF& size) const override;
    void ForceSyncGeometryNode()
    {
        CHECK_NULL_VOID(renderContext_);
        oldGeometryNode_.Reset();
        renderContext_->SyncGeometryProperties(RawPtr(geometryNode_));
    }

    template<typename T>
    RefPtr<T> FindFocusChildNodeOfClass()
    {
        const auto& children = GetChildren();
        for (auto iter = children.rbegin(); iter != children.rend(); ++iter) {
            auto& child = *iter;
            auto target = DynamicCast<FrameNode>(child->FindChildNodeOfClass<T>());
            if (target) {
                auto focusEvent = target->eventHub_->GetFocusHub();
                if (focusEvent && focusEvent->IsCurrentFocus()) {
                    return AceType::DynamicCast<T>(target);
                }
            }
        }

        if (AceType::InstanceOf<T>(this)) {
            auto target = DynamicCast<FrameNode>(this);
            if (target) {
                auto focusEvent = target->eventHub_->GetFocusHub();
                if (focusEvent && focusEvent->IsCurrentFocus()) {
                    return Claim(AceType::DynamicCast<T>(this));
                }
            }
        }
        return nullptr;
    }

    virtual std::vector<RectF> GetResponseRegionList(const RectF& rect, int32_t sourceType);

    bool IsFirstBuilding() const
    {
        return isFirstBuilding_;
    }

    void MarkBuildDone()
    {
        isFirstBuilding_ = false;
    }

    Matrix4 GetLocalMatrix() const
    {
        return localMat_;
    }
    OffsetF GetOffsetInScreen();
    RefPtr<PixelMap> GetPixelMap();
    RefPtr<FrameNode> GetPageNode();
    RefPtr<FrameNode> GetNodeContainer();
    RefPtr<ContentModifier> GetContentModifier();
    void NotifyFillRequestSuccess(RefPtr<PageNodeInfoWrap> nodeWrap, AceAutoFillType autoFillType);
    void NotifyFillRequestFailed(int32_t errCode);

    int32_t GetUiExtensionId();
    int64_t WrapExtensionAbilityId(int64_t extensionOffset, int64_t abilityId);
    void SearchExtensionElementInfoByAccessibilityIdNG(int64_t elementId, int32_t mode,
        int64_t offset, std::list<Accessibility::AccessibilityElementInfo>& output);
    void SearchElementInfosByTextNG(int64_t elementId, const std::string& text,
        int64_t offset, std::list<Accessibility::AccessibilityElementInfo>& output);
    void FindFocusedExtensionElementInfoNG(int64_t elementId, int32_t focusType,
        int64_t offset, Accessibility::AccessibilityElementInfo& output);
    void FocusMoveSearchNG(int64_t elementId, int32_t direction,
        int64_t offset, Accessibility::AccessibilityElementInfo& output);
    bool TransferExecuteAction(int64_t elementId, const std::map<std::string, std::string>& actionArguments,
        int32_t action, int64_t offset);
    std::vector<RectF> GetResponseRegionListForRecognizer(int32_t sourceType);
    bool InResponseRegionList(const PointF& parentLocalPoint, const std::vector<RectF>& responseRegionList) const;

    bool GetMonopolizeEvents() const;

    bool IsWindowBoundary() const
    {
        return isWindowBoundary_;
    }

    void SetWindowBoundary(bool isWindowBoundary = true)
    {
        isWindowBoundary_ = isWindowBoundary;
    }

    void InitLastArea();

    OffsetF CalculateCachedTransformRelativeOffset(uint64_t nanoTimestamp);

    void PaintDebugBoundary(bool flag) override;
    RectF GetRectWithRender();

private:
    void MarkNeedRender(bool isRenderBoundary);
    std::pair<float, float> ContextPositionConvertToPX(
        const RefPtr<RenderContext>& context, const SizeF& percentReference) const;
    bool IsNeedRequestParentMeasure() const;
    void UpdateLayoutPropertyFlag() override;
    void ForceUpdateLayoutPropertyFlag(PropertyChangeFlag propertyChangeFlag) override;
    void AdjustParentLayoutFlag(PropertyChangeFlag& flag) override;
    /**
     * @brief try to mark Parent dirty with flag PROPERTY_UPDATE_BY_CHILD_REQUEST.
     *
     * @return true if Parent is successfully marked dirty.
     */
    virtual bool RequestParentDirty(bool childExpansiveAndMark = false);

    void UpdateChildrenLayoutWrapper(const RefPtr<LayoutWrapperNode>& self, bool forceMeasure, bool forceLayout);
    void AdjustLayoutWrapperTree(const RefPtr<LayoutWrapperNode>& parent, bool forceMeasure, bool forceLayout) override;

    LayoutConstraintF GetLayoutConstraint() const;
    OffsetF GetParentGlobalOffset() const;

    RefPtr<PaintWrapper> CreatePaintWrapper();
    void LayoutOverlay();

    void OnGenerateOneDepthVisibleFrame(std::list<RefPtr<FrameNode>>& visibleList) override;
    void OnGenerateOneDepthVisibleFrameWithTransition(std::list<RefPtr<FrameNode>>& visibleList) override;
    void OnGenerateOneDepthAllFrame(std::list<RefPtr<FrameNode>>& allList) override;

    bool IsMeasureBoundary();
    bool IsRenderBoundary();

    bool OnRemoveFromParent(bool allowTransition) override;
    bool RemoveImmediately() const override;

    // dump self info.
    void DumpInfo() override;
    void DumpOverlayInfo();
    void DumpCommonInfo();
    void DumpSafeAreaInfo();
    void DumpAdvanceInfo() override;
    void DumpViewDataPageNode(RefPtr<ViewDataWrap> viewDataWrap) override;
    void DumpOnSizeChangeInfo();
    bool CheckAutoSave() override;
    void FocusToJsonValue(std::unique_ptr<JsonValue>& json) const;
    void MouseToJsonValue(std::unique_ptr<JsonValue>& json) const;
    void TouchToJsonValue(std::unique_ptr<JsonValue>& json) const;
    void GeometryNodeToJsonValue(std::unique_ptr<JsonValue>& json) const;

    bool GetTouchable() const;

    void ProcessAllVisibleCallback(const std::vector<double>& visibleAreaUserRatios,
        VisibleCallbackInfo& visibleAreaUserCallback, double currentVisibleRatio, double lastVisibleRatio);

    void OnPixelRoundFinish(const SizeF& pixelGridRoundSize);

    double CalculateCurrentVisibleRatio(const RectF& visibleRect, const RectF& renderRect);

    // set costom background layoutConstraint
    void SetBackgroundLayoutConstraint(const RefPtr<FrameNode>& customNode);

    void GetPercentSensitive();
    void UpdatePercentSensitive();

    void AddFrameNodeSnapshot(bool isHit, int32_t parentId, std::vector<RectF> responseRegionList);

    int32_t GetNodeExpectedRate();

    void RecordExposureIfNeed(const std::string& inspectorId);

    OffsetF CalculateOffsetRelativeToWindow(uint64_t nanoTimestamp);

    const std::pair<uint64_t, OffsetF>& GetCachedGlobalOffset() const;

    void SetCachedGlobalOffset(const std::pair<uint64_t, OffsetF>& timestampOffset);

    const std::pair<uint64_t, OffsetF>& GetCachedTransformRelativeOffset() const;

    void SetCachedTransformRelativeOffset(const std::pair<uint64_t, OffsetF>& timestampOffset);

    HitTestMode TriggerOnTouchIntercept(const TouchEvent& touchEvent);

    void AddTouchEventAllFingersInfo(TouchEventInfo& event, const TouchEvent& touchEvent);

    // sort in ZIndex.
    std::multiset<WeakPtr<FrameNode>, ZIndexComparator> frameChildren_;
    RefPtr<GeometryNode> geometryNode_ = MakeRefPtr<GeometryNode>();

    std::list<std::function<void()>> destroyCallbacks_;

    RefPtr<AccessibilityProperty> accessibilityProperty_;
    RefPtr<LayoutProperty> layoutProperty_;
    RefPtr<PaintProperty> paintProperty_;
    RefPtr<RenderContext> renderContext_ = RenderContext::Create();
    RefPtr<EventHub> eventHub_;
    RefPtr<Pattern> pattern_;

    RefPtr<FrameNode> backgroundNode_;
    std::function<RefPtr<UINode>()> builderFunc_;
    std::unique_ptr<RectF> lastFrameRect_;
    std::unique_ptr<OffsetF> lastParentOffsetToWindow_;
    std::unique_ptr<RectF> lastFrameNodeRect_;
    std::set<std::string> allowDrop_;
    std::optional<RectF> viewPort_;
    NG::DragDropInfo dragPreviewInfo_;

    RefPtr<LayoutAlgorithmWrapper> layoutAlgorithm_;
    RefPtr<GeometryNode> oldGeometryNode_;
    std::optional<bool> skipMeasureContent_;
    std::unique_ptr<FrameProxy> frameProxy_;
    WeakPtr<TargetComponent> targetComponent_;

    bool needSyncRenderTree_ = false;

    bool isPropertyDiffMarked_ = false;
    bool isLayoutDirtyMarked_ = false;
    bool isRenderDirtyMarked_ = false;
    bool isMeasureBoundary_ = false;
    bool hasPendingRequest_ = false;

    // for container, this flag controls only the last child in touch area is consuming event.
    bool exclusiveEventForChild_ = false;
    bool isActive_ = false;
    bool isResponseRegion_ = false;
    bool bypass_ = false;
    bool isLayoutComplete_ = false;
    bool isFirstBuilding_ = true;

    double lastVisibleRatio_ = 0.0;
    double lastVisibleCallbackRatio_ = 0.0;

    // internal node such as Text in Button CreateWithLabel
    // should not seen by preview inspector or accessibility
    bool isInternal_ = false;

    std::string nodeName_;

    bool draggable_ = false;
    bool userSet_ = false;
    bool customerSet_ = false;
    bool isWindowBoundary_ = false;

    std::map<std::string, RefPtr<NodeAnimatablePropertyBase>> nodeAnimatablePropertyMap_;
    Matrix4 localMat_ = Matrix4::CreateIdentity();

    bool isRestoreInfoUsed_ = false;
    bool checkboxFlag_ = false;
    bool needRestoreSafeArea_ = true;

    RefPtr<FrameNode> overlayNode_;
    RefPtr<NG::DrawModifier> drawModifier_;

    std::unordered_map<std::string, int32_t> sceneRateMap_;

    DragPreviewOption previewOption_ { DragPreviewMode::AUTO };

    RefPtr<Recorder::ExposureProcessor> exposureProcessor_;

    std::pair<uint64_t, OffsetF> cachedGlobalOffset_ = { 0, OffsetF() };
    std::pair<uint64_t, OffsetF> cachedTransformRelativeOffset_ = { 0, OffsetF() };

    struct onSizeChangeDumpInfo {
        int64_t onSizeChangeTimeStamp;
        RectF lastFrameRect;
        RectF currFrameRect;
    };
    std::vector<onSizeChangeDumpInfo> onSizeChangeDumpInfos;

    friend class RosenRenderContext;
    friend class RenderContext;
    friend class Pattern;

    ACE_DISALLOW_COPY_AND_MOVE(FrameNode);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_FRAME_NODE_H
