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

#include "frameworks/bridge/declarative_frontend/jsview/js_navdestination.h"

#include "base/log/ace_scoring_log.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "base/system_bar/system_bar_style.h"
#include "bridge/declarative_frontend/engine/functions/js_function.h"
#include "bridge/declarative_frontend/engine/js_converter.h"
#include "bridge/declarative_frontend/engine/js_execution_scope_defines.h"
#include "bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "bridge/declarative_frontend/engine/js_types.h"
#include "bridge/declarative_frontend/jsview/js_navdestination_context.h"
#include "bridge/declarative_frontend/jsview/js_navigation.h"
#include "bridge/declarative_frontend/jsview/js_utils.h"
#include "core/components_ng/base/view_stack_model.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/navigation/navigation_options.h"
#include "core/components_ng/pattern/navrouter/navdestination_model_ng.h"

namespace OHOS::Ace {
std::unique_ptr<NavDestinationModel> NavDestinationModel::instance_ = nullptr;
std::mutex NavDestinationModel::mutex_;

NavDestinationModel* NavDestinationModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        instance_.reset(new NG::NavDestinationModelNG());
    }
    return instance_.get();
}

} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {

namespace {
constexpr uint32_t SAFE_AREA_TYPE_LIMIT = 3;
constexpr uint32_t SAFE_AREA_EDGE_LIMIT = 4;
constexpr uint32_t SAFE_AREA_EDGE_SYSTEM = 0;
constexpr uint32_t SAFE_AREA_EDGE_TOP = 0;
constexpr uint32_t SAFE_AREA_EDGE_BOTTOM = 1;
constexpr uint32_t PARAMATER_LENGTH_ONE = 1;
constexpr uint32_t PARAMATER_LENGTH_TWO = 2;
constexpr uint32_t FIRST_INDEX = 0;
constexpr uint32_t SECOND_INDEX = 1;

bool ParseCommonTitle(const JSRef<JSObject>& jsObj)
{
    JSRef<JSVal> subtitle = jsObj->GetProperty("sub");
    JSRef<JSVal> title = jsObj->GetProperty("main");
    bool hasSub = subtitle->IsString();
    bool hasMain = title->IsString();
    if (hasSub || hasMain) {
        return NG::NavDestinationModelNG::GetInstance()->ParseCommonTitle(
            hasSub, hasMain, subtitle->ToString(), title->ToString());
    }
    return false;
}

} // namespace

void JSNavDestination::Create()
{
    NavDestinationModel::GetInstance()->Create();
}

void JSNavDestination::Create(const JSCallbackInfo& info)
{
    if (info.Length() <= 0) {
        NavDestinationModel::GetInstance()->Create();
        return;
    }

    std::string moduleName;
    std::string pagePath;
    if (info.Length() == 1) {
        if (info[0]->IsFunction()) {
            auto builderFunctionJS = info[0];
            auto builderFunc = [context = info.GetExecutionContext(), builder = std::move(builderFunctionJS)]() {
                JAVASCRIPT_EXECUTION_SCOPE(context)
                JSRef<JSFunc>::Cast(builder)->Call(JSRef<JSObject>());
            };
            auto ctx = AceType::MakeRefPtr<NG::NavDestinationContext>();
            auto navPathInfo = AceType::MakeRefPtr<JSNavPathInfo>();
            ctx->SetNavPathInfo(navPathInfo);
            NavDestinationModel::GetInstance()->Create(std::move(builderFunc), std::move(ctx));
            return;
        } else if (info[0]->IsObject()) {
            auto infoObj = JSRef<JSObject>::Cast(info[0]);
            if (!infoObj->GetProperty(NG::NAVIGATION_MODULE_NAME)->IsString() ||
                !infoObj->GetProperty(NG::NAVIGATION_PAGE_PATH)->IsString()) {
                TAG_LOGE(AceLogTag::ACE_NAVIGATION, "navDestination current pageInfo is invalid");
                return;
            }
            moduleName = infoObj->GetProperty(NG::NAVIGATION_MODULE_NAME)->ToString();
            pagePath = infoObj->GetProperty(NG::NAVIGATION_PAGE_PATH)->ToString();
            NavDestinationModel::GetInstance()->Create();
            NavDestinationModel::GetInstance()->SetNavDestinationPathInfo(moduleName, pagePath);
            return;
        }
        TAG_LOGE(AceLogTag::ACE_NAVIGATION,
            "current input info is neither buildFunction or navDestination usefulInfo");
        return;
    } else if (info.Length() == 2) {
        if (!info[0]->IsFunction() || !info[1]->IsObject()) {
            TAG_LOGE(AceLogTag::ACE_NAVIGATION, "buider or pageInfo is invalid");
            return;
        }
        auto builderFunctionJS = info[0];
        auto builderFunc = [context = info.GetExecutionContext(), builder = std::move(builderFunctionJS)]() {
            JAVASCRIPT_EXECUTION_SCOPE(context)
            JSRef<JSFunc>::Cast(builder)->Call(JSRef<JSObject>());
        };
        auto ctx = AceType::MakeRefPtr<NG::NavDestinationContext>();
        auto navPathInfo = AceType::MakeRefPtr<JSNavPathInfo>();
        ctx->SetNavPathInfo(navPathInfo);
        
        auto infoObj = JSRef<JSObject>::Cast(info[1]);
        if (!infoObj->GetProperty(NG::NAVIGATION_MODULE_NAME)->IsString() ||
            !infoObj->GetProperty(NG::NAVIGATION_PAGE_PATH)->IsString()) {
            TAG_LOGE(AceLogTag::ACE_NAVIGATION, "navDestination current pageInfo is invalid");
            return;
        }
        moduleName = infoObj->GetProperty(NG::NAVIGATION_MODULE_NAME)->ToString();
        pagePath = infoObj->GetProperty(NG::NAVIGATION_PAGE_PATH)->ToString();
        NavDestinationModel::GetInstance()->Create(std::move(builderFunc), std::move(ctx));
        NavDestinationModel::GetInstance()->SetNavDestinationPathInfo(moduleName, pagePath);
    }
    return;
}

void JSNavDestination::SetHideTitleBar(bool hide)
{
    NavDestinationModel::GetInstance()->SetHideTitleBar(hide);
}

void JSNavDestination::SetTitle(const JSCallbackInfo& info)
{
    // Resource and string type.
    std::string title;
    if (ParseJsString(info[0], title)) {
        NavDestinationModel::GetInstance()->ParseCommonTitle(false, true, "", title);
    } else if (info[0]->IsObject()) {
        JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(info[0]);
        do {
            // NavigationCommonTitle
            if (ParseCommonTitle(jsObj)) {
                break;
            }
            // CustomBuilder | NavigationCustomTitle
            CalcDimension titleHeight;
            if (!jsObj->HasProperty("height")) {
                NavDestinationModel::GetInstance()->SetTitleHeight(titleHeight, false);
                break;
            }
            JSRef<JSVal> height = jsObj->GetProperty("height");
            bool isValid = JSContainerBase::ParseJsDimensionVpNG(height, titleHeight);
            if (height->IsString()) {
                std::string heightValue;
                ParseJsString(height, heightValue);
                if (heightValue == NG::TITLE_MAIN_WITH_SUB) {
                    NavDestinationModel::GetInstance()->SetTitleHeight(NG::DOUBLE_LINE_TITLEBAR_HEIGHT);
                    break;
                }
                if (heightValue == NG::TITLE_MAIN) {
                    NavDestinationModel::GetInstance()->SetTitleHeight(NG::SINGLE_LINE_TITLEBAR_HEIGHT);
                    break;
                }
            }
            if (!isValid || titleHeight.Value() < 0) {
                NavDestinationModel::GetInstance()->SetTitleHeight(Dimension(), true);
                break;
            }
            NavDestinationModel::GetInstance()->SetTitleHeight(titleHeight);
        } while (0);
        JSRef<JSVal> builderObject = jsObj->GetProperty("builder");
        if (builderObject->IsFunction()) {
            ViewStackModel::GetInstance()->NewScope();
            JsFunction jsBuilderFunc(info.This(), JSRef<JSFunc>::Cast(builderObject));
            ACE_SCORING_EVENT("Navdestination.title.builder");
            jsBuilderFunc.Execute();
            auto customNode = ViewStackModel::GetInstance()->Finish();
            NavDestinationModel::GetInstance()->SetCustomTitle(customNode);
        }
    } else {
        NavDestinationModel::GetInstance()->ParseCommonTitle(false, false, "", "");
    }

    NG::NavigationTitlebarOptions options;
    if (info.Length() > 1) {
        ParseBackgroundOptions(info[1], options.bgOptions);
    }
    NavDestinationModel::GetInstance()->SetTitlebarOptions(std::move(options));
}

void JSNavDestination::SetBackButtonIcon(const JSCallbackInfo& info)
{
    // srcType、pixmap、string
    if (info.Length() < 1) {
        return;
    }
    std::string src;
    auto noPixMap = ParseJsMedia(info[0], src);

    RefPtr<PixelMap> pixMap = nullptr;
    auto isValidImage = false;
#if defined(PIXEL_MAP_SUPPORTED)
    if (!noPixMap) {
        pixMap = CreatePixelMapFromNapiValue(info[0]);
    }
#endif
    if (noPixMap || pixMap != nullptr) {
        isValidImage = true;
    }
    std::vector<std::string> nameList;
    NG::ImageOption imageOption;
    std::string bundleName;
    std::string moduleName;
    GetJsMediaBundleInfo(info[0], bundleName, moduleName);
    nameList.emplace_back(bundleName);
    nameList.emplace_back(moduleName);
    imageOption.noPixMap = noPixMap;
    imageOption.isValidImage = isValidImage;
    std::function<void(WeakPtr<NG::FrameNode>)> iconSymbol = nullptr;
    auto isSymbol = info[0]->IsObject() && src.empty() && pixMap == nullptr;
    if (isSymbol) {
        SetSymbolOptionApply(info, iconSymbol, info[0]);
    }

    NavDestinationModel::GetInstance()->SetBackButtonIcon(iconSymbol, src, imageOption, pixMap, nameList);
}

void JSNavDestination::SetOnShown(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }

    auto onShownCallback = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    WeakPtr<NG::FrameNode> targetNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto onShown = [execCtx = info.GetExecutionContext(), func = std::move(onShownCallback), node = targetNode]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("NavDestination.onShown");
        PipelineContext::SetCallBackNode(node);
        JSRef<JSVal> params[1];
        params[0] = JSRef<JSVal>::Make(ToJSValue("undefined"));
        func->ExecuteJS(1, params);
    };
    NavDestinationModel::GetInstance()->SetOnShown(std::move(onShown));
    info.ReturnSelf();
}

void JSNavDestination::SetOnHidden(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }
    auto onHiddenCallback = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    WeakPtr<NG::FrameNode> targetNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto onHidden = [execCtx = info.GetExecutionContext(), func = std::move(onHiddenCallback), node = targetNode]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("NavDestination.onHidden");
        PipelineContext::SetCallBackNode(node);
        func->ExecuteJS();
    };
    NavDestinationModel::GetInstance()->SetOnHidden(std::move(onHidden));
    info.ReturnSelf();
}

void JSNavDestination::SetOnBackPressed(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }
    auto onBackPressedCallback = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onBackPressed = [execCtx = info.GetExecutionContext(), func = std::move(onBackPressedCallback)]() -> bool {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx, false);
        ACE_SCORING_EVENT("NavDestination.onBackPressed");
        return (func->ExecuteJS())->ToBoolean();
    };
    NavDestinationModel::GetInstance()->SetOnBackPressed(std::move(onBackPressed));
    info.ReturnSelf();
}

void JSNavDestination::SetOnReady(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }
    auto onReadyCallback = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onReady = [execCtx = info.GetExecutionContext(), func = std::move(onReadyCallback)](
                       RefPtr<NG::NavDestinationContext> context) {
        auto jsContext = JSClass<JSNavDestinationContext>::NewInstance();
        auto jsNavDestinationContext = Referenced::Claim(jsContext->Unwrap<JSNavDestinationContext>());
        CHECK_NULL_VOID(jsNavDestinationContext);
        jsNavDestinationContext->SetNavDestinationContext(context);
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("NavDestination.onReady");
        JSRef<JSVal> params[1];
        params[0] = jsContext;
        func->ExecuteJS(1, params);
    };
    NavDestinationModel::GetInstance()->SetOnReady(std::move(onReady));
    info.ReturnSelf();
}

void JSNavDestination::SetMode(const JSCallbackInfo& info)
{
    if (!info[0]->IsNumber()) {
        // set default back ground color
        NavDestinationModel::GetInstance()->SetNavDestinationMode(NG::NavDestinationMode::STANDARD);
        return;
    }
    auto mode = info[0]->ToNumber<int32_t>();
    NavDestinationModel::GetInstance()->SetNavDestinationMode(static_cast<NG::NavDestinationMode>(mode));
}

void JSNavDestination::SetMenus(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    if (info[0]->IsUndefined() || info[0]->IsArray()) {
        std::vector<NG::BarItem> menuItems;
        if (info[0]->IsUndefined()) {
            menuItems = {};
        } else {
            JSNavigation::ParseBarItems(info, JSRef<JSArray>::Cast(info[0]), menuItems);
        }
        NavDestinationModel::GetInstance()->SetMenuItems(std::move(menuItems));
        return;
    } else if (info[0]->IsObject()) {
        auto builderObject = JSRef<JSObject>::Cast(info[0])->GetProperty("builder");
        if (builderObject->IsFunction()) {
            ViewStackModel::GetInstance()->NewScope();
            JsFunction jsBuilderFunc(info.This(), JSRef<JSObject>::Cast(builderObject));
            ACE_SCORING_EVENT("NavDestiNation.menu.builder");
            jsBuilderFunc.Execute();
            auto customNode = ViewStackModel::GetInstance()->Finish();
            NavDestinationModel::GetInstance()->SetCustomMenu(customNode);
        }
    }
}

void JSNavDestination::SetBackgroundColor(const JSCallbackInfo& info)
{
    Color backgroundColor;
    bool isValid = ParseJsColor(info[0], backgroundColor);

    NavDestinationModel::GetInstance()->SetBackgroundColor(backgroundColor, isValid);
}

void JSNavDestination::SetWillAppear(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }

    auto willAppear = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onWillAppear = [execCtx = info.GetExecutionContext(), func = std::move(willAppear)]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("NavDestination.WillAppear");
        func->ExecuteJS();
    };
    NavDestinationModel::GetInstance()->SetOnWillAppear(std::move(onWillAppear));
    info.ReturnSelf();
}

void JSNavDestination::SetWillHide(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }

    auto willHideCallback = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onWillHide = [execCtx = info.GetExecutionContext(), func = std::move(willHideCallback)]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("NavDestination.WillHide");
        func->ExecuteJS();
    };
    NavDestinationModel::GetInstance()->SetOnWillHide(std::move(onWillHide));
    info.ReturnSelf();
}

void JSNavDestination::SetWillShow(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }

    auto willShowCallback = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onWillShow = [execCtx = info.GetExecutionContext(), func = std::move(willShowCallback)]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("NavDestination.WillShow");
        func->ExecuteJS();
    };
    NavDestinationModel::GetInstance()->SetOnWillShow(std::move(onWillShow));
    info.ReturnSelf();
}

void JSNavDestination::SetWillDisAppear(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }

    auto WillDisAppear = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onWillDisAppear = [execCtx = info.GetExecutionContext(), func = std::move(WillDisAppear)]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("NavDestination.WillDisAppear");
        func->ExecuteJS();
    };
    NavDestinationModel::GetInstance()->SetOnWillDisAppear(std::move(onWillDisAppear));
    info.ReturnSelf();
}

void JSNavDestination::SetIgnoreLayoutSafeArea(const JSCallbackInfo& info)
{
    NG::SafeAreaExpandOpts opts { .type = NG::SAFE_AREA_TYPE_SYSTEM, .edges = NG::SAFE_AREA_EDGE_ALL};
    if (info.Length() >= PARAMATER_LENGTH_ONE && info[FIRST_INDEX]->IsArray()) {
        auto paramArray = JSRef<JSArray>::Cast(info[0]);
        uint32_t safeAreaType = NG::SAFE_AREA_TYPE_NONE;
        for (size_t i = 0; i < paramArray->Length(); ++i) {
            auto value = paramArray->GetValueAt(i);
            if (!value->IsNumber() ||
                value->ToNumber<uint32_t>() >= SAFE_AREA_TYPE_LIMIT ||
                value->ToNumber<uint32_t>() == SAFE_AREA_EDGE_SYSTEM) {
                safeAreaType = NG::SAFE_AREA_TYPE_SYSTEM;
                break;
            }
        }
        opts.type = safeAreaType;
    }

    if (info.Length() >= PARAMATER_LENGTH_TWO && info[SECOND_INDEX]->IsArray()) {
        auto paramArray = JSRef<JSArray>::Cast(info[1]);
        uint32_t safeAreaEdge = NG::SAFE_AREA_EDGE_NONE;
        for (size_t i = 0; i < paramArray->Length(); ++i) {
            auto value = paramArray->GetValueAt(i);
            if (!value->IsNumber() ||
                value->ToNumber<uint32_t>() >= SAFE_AREA_EDGE_LIMIT) {
                safeAreaEdge = NG::SAFE_AREA_EDGE_ALL;
                break;
            }
            if (value->ToNumber<uint32_t>() == SAFE_AREA_EDGE_TOP ||
                value->ToNumber<uint32_t>() == SAFE_AREA_EDGE_BOTTOM) {
                    safeAreaEdge |= (1 << value->ToNumber<uint32_t>());
                }
        }
        opts.edges = safeAreaEdge;
    }
    NavDestinationModel::GetInstance()->SetIgnoreLayoutSafeArea(opts);
}

void JSNavDestination::JSBind(BindingTarget globalObj)
{
    JSNavDestinationContext::JSBind(globalObj);
    JSClass<JSNavDestination>::Declare("NavDestination");
    JSClass<JSNavDestination>::StaticMethod("create", &JSNavDestination::Create);
    JSClass<JSNavDestination>::StaticMethod("title", &JSNavDestination::SetTitle);
    JSClass<JSNavDestination>::StaticMethod("hideTitleBar", &JSNavDestination::SetHideTitleBar);
    JSClass<JSNavDestination>::StaticMethod("backButtonIcon", &JSNavDestination::SetBackButtonIcon);
    JSClass<JSNavDestination>::StaticMethod("backgroundColor", &JSNavDestination::SetBackgroundColor);
    JSClass<JSNavDestination>::StaticMethod("onShown", &JSNavDestination::SetOnShown);
    JSClass<JSNavDestination>::StaticMethod("onHidden", &JSNavDestination::SetOnHidden);
    JSClass<JSNavDestination>::StaticMethod("onBackPressed", &JSNavDestination::SetOnBackPressed);
    JSClass<JSNavDestination>::StaticMethod("onReady", &JSNavDestination::SetOnReady);
    JSClass<JSNavDestination>::StaticMethod("onAttach", &JSInteractableView::JsOnAttach);
    JSClass<JSNavDestination>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSNavDestination>::StaticMethod("onDetach", &JSInteractableView::JsOnDetach);
    JSClass<JSNavDestination>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSNavDestination>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSNavDestination>::StaticMethod("id", &JSViewAbstract::JsId);
    JSClass<JSNavDestination>::StaticMethod("mode", &JSNavDestination::SetMode);
    JSClass<JSNavDestination>::StaticMethod("menus", &JSNavDestination::SetMenus);
    JSClass<JSNavDestination>::StaticMethod("onWillAppear", &JSNavDestination::SetWillAppear);
    JSClass<JSNavDestination>::StaticMethod("onWillShow", &JSNavDestination::SetWillShow);
    JSClass<JSNavDestination>::StaticMethod("onWillHide", &JSNavDestination::SetWillHide);
    JSClass<JSNavDestination>::StaticMethod("onWillDisappear", &JSNavDestination::SetWillDisAppear);
    JSClass<JSNavDestination>::StaticMethod("ignoreLayoutSafeArea", &JSNavDestination::SetIgnoreLayoutSafeArea);
    JSClass<JSNavDestination>::StaticMethod("systemBarStyle", &JSNavDestination::SetSystemBarStyle);
    JSClass<JSNavDestination>::InheritAndBind<JSContainerBase>(globalObj);
}

void JSNavDestination::SetSystemBarStyle(const JSCallbackInfo& info)
{
    RefPtr<SystemBarStyle> style = nullptr;
    if (info.Length() == 1 && info[0]->IsObject()) {
        auto styleObj = JsConverter::ConvertJsValToNapiValue(info[0]);
        auto env = GetCurrentEnv();
        if (env) {
            style = SystemBarStyle::CreateStyleFromJsObj(env, styleObj);
        }
    }
    NavDestinationModel::GetInstance()->SetSystemBarStyle(style);
}
} // namespace OHOS::Ace::Framework
