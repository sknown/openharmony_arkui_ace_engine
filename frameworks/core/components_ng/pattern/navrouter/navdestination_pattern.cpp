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

#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/common/container.h"
#include "base/log/dump_log.h"
#include "core/components/theme/app_theme.h"
#include "core/components_ng/pattern/navigation/title_bar_layout_property.h"
#include "core/components_ng/pattern/navigation/title_bar_node.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"

namespace OHOS::Ace::NG {
namespace {
void BuildTitle(const RefPtr<NavDestinationGroupNode>& navDestinationNode, const RefPtr<TitleBarNode>& titleBarNode)
{
    CHECK_NULL_VOID(navDestinationNode->GetTitle());
    if (navDestinationNode->GetTitleNodeOperationValue(ChildNodeOperation::NONE) == ChildNodeOperation::NONE) {
        return;
    }
    if (navDestinationNode->GetTitleNodeOperationValue(ChildNodeOperation::NONE) == ChildNodeOperation::REPLACE) {
        titleBarNode->RemoveChild(titleBarNode->GetTitle());
        auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
        CHECK_NULL_VOID(titleBarLayoutProperty);
        titleBarLayoutProperty->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    }
    titleBarNode->SetTitle(navDestinationNode->GetTitle());
    titleBarNode->AddChild(titleBarNode->GetTitle());
}

void BuildSubtitle(const RefPtr<NavDestinationGroupNode>& navDestinationNode, const RefPtr<TitleBarNode>& titleBarNode)
{
    if (!navDestinationNode->GetSubtitle() && titleBarNode->GetSubtitle()) {
        auto subtitleNode = titleBarNode->GetSubtitle();
        titleBarNode->SetSubtitle(nullptr);
        titleBarNode->RemoveChild(subtitleNode);
        titleBarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        return;
    }
    CHECK_NULL_VOID(navDestinationNode->GetSubtitle());
    if (navDestinationNode->GetSubtitleNodeOperationValue(ChildNodeOperation::NONE) == ChildNodeOperation::NONE) {
        return;
    }
    if (navDestinationNode->GetSubtitleNodeOperationValue(ChildNodeOperation::NONE) == ChildNodeOperation::REPLACE) {
        titleBarNode->RemoveChild(titleBarNode->GetSubtitle());
    }
    titleBarNode->SetSubtitle(navDestinationNode->GetSubtitle());
    titleBarNode->AddChild(titleBarNode->GetSubtitle());
}

void BuildTitleBar(const RefPtr<NavDestinationGroupNode>& navDestinationNode, const RefPtr<TitleBarNode>& titleBarNode,
    RefPtr<NavDestinationLayoutProperty>& navDestinationLayoutProperty)
{
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);

    // back button icon
    if (navDestinationLayoutProperty->HasNoPixMap()) {
        if (navDestinationLayoutProperty->HasImageSource()) {
            titleBarLayoutProperty->UpdateImageSource(navDestinationLayoutProperty->GetImageSourceValue());
        }
        if (navDestinationLayoutProperty->HasPixelMap()) {
            titleBarLayoutProperty->UpdatePixelMap(navDestinationLayoutProperty->GetPixelMapValue());
        }
        titleBarLayoutProperty->UpdateNoPixMap(navDestinationLayoutProperty->GetNoPixMapValue());
    }

    BuildTitle(navDestinationNode, titleBarNode);
    BuildSubtitle(navDestinationNode, titleBarNode);
}

void MountTitleBar(const RefPtr<NavDestinationGroupNode>& hostNode)
{
    auto navDestinationLayoutProperty = hostNode->GetLayoutProperty<NavDestinationLayoutProperty>();
    CHECK_NULL_VOID(navDestinationLayoutProperty);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    BuildTitleBar(hostNode, titleBarNode, navDestinationLayoutProperty);
}

} // namespace

void NavDestinationPattern::OnActive()
{
    Pattern::OnActive();
    auto hostNode = AceType::DynamicCast<NavDestinationGroupNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    auto navDestinationContext = hostNode->GetRenderContext();
    CHECK_NULL_VOID(navDestinationContext);
    auto navDestinationLayoutProperty = hostNode->GetLayoutProperty<NavDestinationLayoutProperty>();
    CHECK_NULL_VOID(navDestinationLayoutProperty);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (navDestinationLayoutProperty->GetHideTitleBar().value_or(false)) {
        titleBarLayoutProperty->UpdateVisibility(VisibleType::GONE);
    } else {
        titleBarLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
    }
    titleBarNode->MarkModifyDone();
}

void NavDestinationPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    auto hostNode = AceType::DynamicCast<NavDestinationGroupNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    UpdateNameIfNeeded(hostNode);
    UpdateBackgroundColorIfNeeded(hostNode);
    UpdateTitlebarVisibility(hostNode);
}

void NavDestinationPattern::UpdateNameIfNeeded(RefPtr<NavDestinationGroupNode>& hostNode)
{
    if (!name_.empty()) {
        return;
    }

    if (hostNode->GetInspectorId().has_value()) {
        name_ = hostNode->GetInspectorIdValue();
    } else {
        name_ = std::to_string(GetHost()->GetId());
    }
    auto pathInfo = GetNavPathInfo();
    if (pathInfo) {
        pathInfo->SetName(name_);
    }
}

void NavDestinationPattern::UpdateBackgroundColorIfNeeded(RefPtr<NavDestinationGroupNode>& hostNode)
{
    auto renderContext = hostNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    if (renderContext->GetBackgroundColor().has_value()) {
        TAG_LOGI(AceLogTag::ACE_NAVIGATION, "Background already has color: %{public}s",
            renderContext->GetBackgroundColor()->ColorToString().c_str());
        return;
    }
    if (hostNode->GetNavDestinationMode() == NavDestinationMode::DIALOG) {
        renderContext->UpdateBackgroundColor(Color::TRANSPARENT);
        TAG_LOGI(AceLogTag::ACE_NAVIGATION, "Set dialog background color: %{public}s",
            renderContext->GetBackgroundColor()->ColorToString().c_str());
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    if (!pipelineContext) {
        return;
    }
    auto theme = pipelineContext->GetTheme<AppTheme>();
    if (!theme) {
        return;
    }
    renderContext->UpdateBackgroundColor(theme->GetBackgroundColor());
    TAG_LOGI(AceLogTag::ACE_NAVIGATION, "Set default background color: %{public}s",
        renderContext->GetBackgroundColor()->ColorToString().c_str());
}

void NavDestinationPattern::UpdateTitlebarVisibility(RefPtr<NavDestinationGroupNode>& hostNode)
{
    auto navDestinationLayoutProperty = hostNode->GetLayoutProperty<NavDestinationLayoutProperty>();
    CHECK_NULL_VOID(navDestinationLayoutProperty);
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);

    auto&& opts = hostNode->GetLayoutProperty()->GetSafeAreaExpandOpts();
    auto navDestinationContentNode = AceType::DynamicCast<FrameNode>(hostNode->GetContentNode());
    if (opts && opts->Expansive() && navDestinationContentNode) {
        navDestinationContentNode->GetLayoutProperty()->UpdateSafeAreaExpandOpts(*opts);
        navDestinationContentNode->MarkModifyDone();
    }

    if (navDestinationLayoutProperty->GetHideTitleBar().value_or(false)) {
        titleBarLayoutProperty->UpdateVisibility(VisibleType::GONE);
        titleBarNode->SetJSViewActive(false);
    } else {
        titleBarLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
        titleBarNode->SetJSViewActive(true);
        MountTitleBar(hostNode);
        if (opts && opts->Expansive()) {
            titleBarLayoutProperty->UpdateSafeAreaExpandOpts(*opts);
        }
        titleBarNode->MarkModifyDone();
    }

    auto navDesIndex = hostNode->GetIndex();
    if (navDesIndex == 0) {
        navDestinationLayoutProperty->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
        titleBarLayoutProperty->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    }
}

bool NavDestinationPattern::GetBackButtonState()
{
    auto hostNode = AceType::DynamicCast<NavDestinationGroupNode>(GetHost());
    CHECK_NULL_RETURN(hostNode, false);
    auto navDestinationLayoutProperty = hostNode->GetLayoutProperty<NavDestinationLayoutProperty>();
    CHECK_NULL_RETURN(navDestinationLayoutProperty, false);
    if (navDestinationLayoutProperty->GetHideTitleBarValue(false)) {
        return false;
    }
    // get navigation node
    auto parent = AceType::DynamicCast<FrameNode>(hostNode->GetParent());
    RefPtr<NavigationGroupNode> navigationNode;
    while (parent && !parent->IsRootNode()) {
        navigationNode = AceType::DynamicCast<NavigationGroupNode>(parent);
        if (navigationNode) {
            break;
        }
        parent = AceType::DynamicCast<FrameNode>(parent->GetParent());
    }
    if (!navigationNode) {
        TAG_LOGW(AceLogTag::ACE_NAVIGATION, "can't find navigation node");
        return false;
    }
    auto navigationLayoutProperty = navigationNode->GetLayoutProperty<NavigationLayoutProperty>();
    CHECK_NULL_RETURN(navigationLayoutProperty, false);
    auto pattern = navigationNode->GetPattern<NavigationPattern>();
    auto stack = pattern->GetNavigationStack();
    auto index = stack->FindIndex(name_, navDestinationNode_.Upgrade(), true);
    bool showBackButton = true;
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(hostNode->GetTitleBarNode());
    auto layoutWrapper = AceType::DynamicCast<LayoutAlgorithmWrapper>(navigationNode->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutWrapper, showBackButton);
    auto layout = AceType::DynamicCast<NavigationLayoutAlgorithm>(layoutWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layout, false);
    if (index == 0 && (layout->GetNavigationMode() == NavigationMode::SPLIT ||
        navigationLayoutProperty->GetHideNavBarValue(false))) {
        showBackButton = false;
    }
    auto isCustomTitle = hostNode->GetPrevTitleIsCustomValue(false);
    if (isCustomTitle) {
        return showBackButton;
    }
    auto titleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle());
    CHECK_NULL_RETURN(titleNode, showBackButton);
    auto theme = NavigationGetTheme();
    CHECK_NULL_RETURN(theme, showBackButton);
    auto textLayoutProperty = titleNode->GetLayoutProperty<TextLayoutProperty>();
    auto currentFontSize = textLayoutProperty->GetAdaptMaxFontSizeValue(Dimension(0.0, DimensionUnit::FP));
    auto targetFontSize = showBackButton ? theme->GetTitleFontSizeMin() : theme->GetTitleFontSize();
    if (targetFontSize == currentFontSize) {
        return showBackButton;
    }
    textLayoutProperty->UpdateAdaptMaxFontSize(targetFontSize);
    textLayoutProperty->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    return showBackButton;
}

void NavDestinationPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        SafeAreaExpandOpts opts = {.type = SAFE_AREA_TYPE_SYSTEM, .edges = SAFE_AREA_EDGE_ALL};
        host->GetLayoutProperty()->UpdateSafeAreaExpandOpts(opts);
    }
}

void NavDestinationPattern::OnAttachToMainTree()
{
    RefPtr<UINode> node = DynamicCast<UINode>(GetHost());
    while (node) {
        if (node->GetTag() == V2::NAVIGATION_VIEW_ETS_TAG) {
            break;
        }
        node = node->GetParent();
    }
    CHECK_NULL_VOID(node);
    navigationNode_ = AceType::WeakClaim(RawPtr(node));
}

void NavDestinationPattern::DumpInfo()
{
    DumpLog::GetInstance().AddDesc(std::string("name: ").append(name_));
}
} // namespace OHOS::Ace::NG
