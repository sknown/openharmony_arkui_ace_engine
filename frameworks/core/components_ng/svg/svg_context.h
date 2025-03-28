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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SVG_PARSE_SVG_CONTEXT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SVG_PARSE_SVG_CONTEXT_H

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>

#include "base/geometry/dimension.h"
#include "base/geometry/rect.h"
#include "base/memory/ace_type.h"
#include "base/utils/noncopyable.h"
#include "core/animation/animator.h"
#include "core/components_ng/render/canvas_image.h"

namespace OHOS::Ace::NG {
using AttrMap = std::unordered_map<std::string, std::string>;
using ClassStyleMap = std::unordered_map<std::string, AttrMap>;
using FuncNormalizeToPx = std::function<double(const Dimension&)>;
using FuncAnimateFlush = std::function<void()>;

class SvgNode;

class SvgContext : public AceType {
    DECLARE_ACE_TYPE(SvgContext, AceType);

public:
    SvgContext() = default;
    ~SvgContext() override
    {
        idMapper_.clear();
    }

    void Push(const std::string& value, const RefPtr<SvgNode>& svgNode)
    {
        idMapper_.emplace(value, svgNode);
    }

    RefPtr<SvgNode> GetSvgNodeById(const std::string& id) const;

    void PushStyle(const std::string& styleName, const std::pair<std::string, std::string>& attrPair);

    const AttrMap& GetAttrMap(const std::string& key) const;

    void AddAnimator(int32_t key, const RefPtr<Animator>& animator);

    void RemoveAnimator(int32_t key);

    void ControlAnimators(bool play);

    size_t GetAnimatorCount();

    void SetFuncNormalizeToPx(const FuncNormalizeToPx& funcNormalizeToPx);

    double NormalizeToPx(const Dimension& value);

    void SetFuncAnimateFlush(FuncAnimateFlush&& funcAnimateFlush, const WeakPtr<CanvasImage>& imagePtr);

    void AnimateFlush();

    void SetRootViewBox(const Rect& viewBox)
    {
        rootViewBox_ = viewBox;
    }

    const Rect& GetRootViewBox() const
    {
        return rootViewBox_;
    }

    void SetViewPort(const Size& viewPort)
    {
        viewPort_ = viewPort;
    }

    const Size& GetViewPort() const
    {
        return viewPort_;
    }

    uint32_t ReleaseAndGetAnimatorNeedFinishCnt()
    {
        return --animatorNeedFinishCnt_;
    }

    void InitAnimatorNeedFinishCnt()
    {
        animatorNeedFinishCnt_ = animatorSumCnt_;
    }

private:
    std::unordered_map<std::string, WeakPtr<SvgNode>> idMapper_;
    uint32_t animatorNeedFinishCnt_ = 0;
    uint32_t animatorSumCnt_ = 0;
    // weak references to animators in svgDom
    std::unordered_map<int32_t, WeakPtr<Animator>> animators_;
    ClassStyleMap styleMap_;
    FuncNormalizeToPx funcNormalizeToPx_ = nullptr;
    // svg dom shared by multiple images
    std::map<WeakPtr<CanvasImage>, FuncAnimateFlush> animateCallbacks_;
    Rect rootViewBox_;
    Size viewPort_;

    ACE_DISALLOW_COPY_AND_MOVE(SvgContext);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SVG_PARSE_SVG_CONTEXT_H