/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "adapter/ohos/entrance/ace_ability.h"

#include <regex>

#include <ui/rs_surface_node.h>
#include "ability_process.h"
#include "dm/display_manager.h"
#include "display_type.h"
#include "init_data.h"
#include "res_config.h"
#include "resource_manager.h"
#include "string_wrapper.h"

#ifdef ENABLE_ROSEN_BACKEND
#include "render_service_client/core/ui/rs_ui_director.h"
#endif

#include "adapter/ohos/entrance/ace_application_info.h"
#include "adapter/ohos/entrance/ace_container.h"
#include "adapter/ohos/entrance/capability_registry.h"
#include "adapter/ohos/entrance/flutter_ace_view.h"
#include "adapter/ohos/entrance/plugin_utils_impl.h"
#include "adapter/ohos/entrance/utils.h"
#include "base/log/log.h"
#include "base/utils/system_properties.h"
#include "core/common/container_scope.h"
#include "core/common/frontend.h"
#include "core/common/plugin_manager.h"
#include "core/common/plugin_utils.h"

namespace OHOS {
namespace Ace {
namespace {

const std::string ABS_BUNDLE_CODE_PATH = "/data/app/el1/bundle/public/";
const std::string LOCAL_BUNDLE_CODE_PATH = "/data/storage/el1/bundle/";
const std::string FILE_SEPARATOR = "/";

FrontendType GetFrontendType(const std::string& frontendType)
{
    if (frontendType == "normal") {
        return FrontendType::JS;
    } else if (frontendType == "form") {
        return FrontendType::JS_CARD;
    } else if (frontendType == "declarative") {
        return FrontendType::DECLARATIVE_JS;
    } else {
        LOGW("frontend type not supported. return default frontend: JS frontend.");
        return FrontendType::JS;
    }
}

FrontendType GetFrontendTypeFromManifest(const std::string& packagePathStr, const std::string& srcPath)
{
    std::string manifest = std::string("assets/js/default/manifest.json");
    if (!srcPath.empty()) {
        manifest = "assets/js/" + srcPath + "/manifest.json";
    }
    auto manifestPath = packagePathStr + manifest;
    char realPath[PATH_MAX] = { 0x00 };
    if (realpath(manifestPath.c_str(), realPath) == nullptr) {
        LOGE("realpath fail! filePath: %{private}s, fail reason: %{public}s", manifestPath.c_str(), strerror(errno));
        LOGE("return default frontend: JS frontend.");
        return FrontendType::JS;
    }
    std::unique_ptr<FILE, decltype(&fclose)> file(fopen(realPath, "rb"), fclose);
    if (!file) {
        LOGE("open file failed, filePath: %{private}s, fail reason: %{public}s", manifestPath.c_str(), strerror(errno));
        LOGE("return default frontend: JS frontend.");
        return FrontendType::JS;
    }
    if (std::fseek(file.get(), 0, SEEK_END) != 0) {
        LOGE("seek file tail error, return default frontend: JS frontend.");
        return FrontendType::JS;
    }

    long size = std::ftell(file.get());
    if (size == -1L) {
        return FrontendType::JS;
    }
    char* fileData = new (std::nothrow) char[size];
    if (fileData == nullptr) {
        LOGE("new json buff failed, return default frontend: JS frontend.");
        return FrontendType::JS;
    }
    rewind(file.get());
    std::unique_ptr<char[]> jsonStream(fileData);
    size_t result = std::fread(jsonStream.get(), 1, size, file.get());
    if (result != (size_t)size) {
        LOGE("read file failed, return default frontend: JS frontend.");
        return FrontendType::JS;
    }

    std::string jsonString(jsonStream.get(), jsonStream.get() + size);
    auto rootJson = JsonUtil::ParseJsonString(jsonString);
    auto mode = rootJson->GetObject("mode");
    if (mode != nullptr) {
        if (mode->GetString("syntax") == "ets" || mode->GetString("type") == "pageAbility") {
            return FrontendType::DECLARATIVE_JS;
        }
    }
    return GetFrontendType(rootJson->GetString("type"));
}

} // namespace

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

using AcePlatformFinish = std::function<void()>;
class AcePlatformEventCallback final : public Platform::PlatformEventCallback {
public:
    explicit AcePlatformEventCallback(AcePlatformFinish onFinish) : onFinish_(onFinish) {}

    ~AcePlatformEventCallback() = default;

    virtual void OnFinish() const
    {
        LOGI("AcePlatformEventCallback OnFinish");
        if (onFinish_) {
            onFinish_();
        }
    }

    virtual void OnStatusBarBgColorChanged(uint32_t color)
    {
        LOGI("AcePlatformEventCallback OnStatusBarBgColorChanged");
    }

private:
    AcePlatformFinish onFinish_;
};

int32_t AceAbility::instanceId_ = 0;
const std::string AceAbility::START_PARAMS_KEY = "__startParams";
const std::string AceAbility::PAGE_URI = "url";
const std::string AceAbility::CONTINUE_PARAMS_KEY = "__remoteData";

REGISTER_AA(AceAbility)

void AceAbility::OnStart(const Want& want)
{
    Ability::OnStart(want);
    LOGI("AceAbility::OnStart called");
    static std::once_flag onceFlag;
    auto abilityContext = GetAbilityContext();
    std::call_once(onceFlag, [abilityContext]() {
        LOGI("Initialize for current process.");
        SetHwIcuDirectory();
        Container::UpdateCurrent(INSTANCE_ID_PLATFORM);
        CapabilityRegistry::Register();
        AceApplicationInfo::GetInstance().SetPackageName(abilityContext->GetBundleName());
        AceApplicationInfo::GetInstance().SetDataFileDirPath(abilityContext->GetFilesDir());
        ImageCache::SetImageCacheFilePath(abilityContext->GetCacheDir());
        ImageCache::SetCacheFileInfo();
    });
    OHOS::sptr<OHOS::Rosen::Window> window = Ability::GetWindow();
    // register surface change callback
    OHOS::sptr<OHOS::Rosen::IWindowChangeListener> thisAbility(this);
    window->RegisterWindowChangeListener(thisAbility);

    // register drag event callback
    OHOS::sptr<OHOS::Rosen::IWindowDragListener> dragWindowListener(this);
    window->RegisterDragListener(dragWindowListener);

    int32_t width = window->GetRect().width_;
    int32_t height = window->GetRect().height_;
    LOGI("AceAbility: windowConfig: width: %{public}d, height: %{public}d", width, height);
    // get density
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (defaultDisplay) {
        density_ = defaultDisplay->GetVirtualPixelRatio();
        LOGI("AceAbility: Default display density set: %{public}f", density_);
    } else {
        LOGI("AceAbility: Default display is null, set density failed. Use default density: %{public}f", density_);
    }
    SystemProperties::InitDeviceInfo(width, height, height >= width ? 0 : 1, density_, false);
    SystemProperties::SetColorMode(ColorMode::LIGHT);

    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    auto resourceManager = GetResourceManager();
    if (resourceManager != nullptr) {
        resourceManager->GetResConfig(*resConfig);
        auto localeInfo = resConfig->GetLocaleInfo();
        Platform::AceApplicationInfoImpl::GetInstance().SetResourceManager(resourceManager);
        if (localeInfo != nullptr) {
            auto language = localeInfo->getLanguage();
            auto region = localeInfo->getCountry();
            auto script = localeInfo->getScript();
            AceApplicationInfo::GetInstance().SetLocale((language == nullptr) ? "" : language,
                (region == nullptr) ? "" : region, (script == nullptr) ? "" : script, "");
        } else {
           LOGW("localeInfo is null.");
           AceApplicationInfo::GetInstance().SetLocale("", "", "", "");
        }
    } else {
       LOGW("resourceManager is null.");
       AceApplicationInfo::GetInstance().SetLocale("", "", "", "");
    }

    auto packagePathStr = GetBundleCodePath();
    auto moduleInfo = GetHapModuleInfo();
    if (moduleInfo != nullptr) {
        packagePathStr += "/" + moduleInfo->name + "/";
    }
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    std::string srcPath = "";
    if (info != nullptr && !info->srcPath.empty()) {
        srcPath = info->srcPath;
    }
    if (info != nullptr && !info->bundleName.empty()) {
        AceApplicationInfo::GetInstance().SetPackageName(info->bundleName);
    }

    FrontendType frontendType = GetFrontendTypeFromManifest(packagePathStr, srcPath);
    bool isArkApp = GetIsArkFromConfig(packagePathStr);

    std::string moduleName = info->moduleName;
    std::shared_ptr<ApplicationInfo> appInfo = GetApplicationInfo();
    std::vector<ModuleInfo> moduleList = appInfo->moduleInfos;

    std::string resPath;
    for (auto module : moduleList) {
        if (module.moduleName == moduleName) {
            std::regex pattern(ABS_BUNDLE_CODE_PATH + info->bundleName + FILE_SEPARATOR);
            auto moduleSourceDir = std::regex_replace(module.moduleSourceDir, pattern, LOCAL_BUNDLE_CODE_PATH);
            resPath = moduleSourceDir + "/assets/" + module.moduleName + FILE_SEPARATOR;
            break;
        }
    }

    AceApplicationInfo::GetInstance().SetDebug(appInfo->debug, want.GetBoolParam("debugApp", false));

    auto pluginUtils = std::make_shared<PluginUtilsImpl>();
    PluginManager::GetInstance().SetAceAbility(this, pluginUtils);

    // create container
    Platform::AceContainer::CreateContainer(abilityId_, frontendType, isArkApp, srcPath, shared_from_this(),
        std::make_unique<AcePlatformEventCallback>([this]() { TerminateAbility(); }));
    auto container = Platform::AceContainer::GetContainer(abilityId_);
    if (!container) {
        LOGE("container is null, set configuration failed.");
    } else {
        auto aceResCfg = container->GetResourceConfiguration();
        aceResCfg.SetOrientation(SystemProperties::GetDevcieOrientation());
        aceResCfg.SetDensity(SystemProperties::GetResolution());
        aceResCfg.SetDeviceType(SystemProperties::GetDeviceType());
        container->SetResourceConfiguration(aceResCfg);
        container->SetPackagePathStr(resPath);
    }

    // create view.
    auto flutterAceView = Platform::FlutterAceView::CreateView(abilityId_);
    Platform::FlutterAceView::SurfaceCreated(flutterAceView, window);
    flutter::ViewportMetrics metrics;
    metrics.physical_width = width;
    metrics.physical_height = height;
    metrics.device_pixel_ratio = density_;
    Platform::FlutterAceView::SetViewportMetrics(flutterAceView, metrics);

    if (srcPath.empty()) {
        auto assetBasePathStr = { std::string("assets/js/default/"), std::string("assets/js/share/") };
        Platform::AceContainer::AddAssetPath(abilityId_, packagePathStr, assetBasePathStr);
    } else {
        auto assetBasePathStr = { "assets/js/" + srcPath + "/" };
        Platform::AceContainer::AddAssetPath(abilityId_, packagePathStr, assetBasePathStr);
    }

    Ace::Platform::UIEnvCallback callback = nullptr;
#ifdef ENABLE_ROSEN_BACKEND
    callback = [ window, thisAbility, id = abilityId_ ] (
        const OHOS::Ace::RefPtr<OHOS::Ace::PipelineContext>& context) mutable {
        if (SystemProperties::GetRosenBackendEnabled()) {
            auto rsUiDirector = OHOS::Rosen::RSUIDirector::Create();
            if (rsUiDirector != nullptr) {
                rsUiDirector->SetRSSurfaceNode(window->GetSurfaceNode());

                // todo regist on size change()
                window->RegisterWindowChangeListener(thisAbility);

                rsUiDirector->SetUITaskRunner(
                    [taskExecutor = Platform::AceContainer::GetContainer(id)->GetTaskExecutor(), id ]
                        (const std::function<void()>& task) {
                            ContainerScope scope(id);
                            taskExecutor->PostTask(task, TaskExecutor::TaskType::UI);
                        });
                if (context != nullptr) {
                    context->SetRSUIDirector(rsUiDirector);
                }
                rsUiDirector->Init();
                LOGI("Init Rosen Backend");
            }
        } else {
            LOGI("not Init Rosen Backend");
        }
    };
#endif
    // set view
    Platform::AceContainer::SetView(flutterAceView, density_, width, height, window->GetWindowId(), callback);
    Platform::FlutterAceView::SurfaceChanged(flutterAceView, width, height, 0);

    // action event hadnler
    auto&& actionEventHandler = [this](const std::string& action) {
        LOGI("on Action called to event handler");

        auto eventAction = JsonUtil::ParseJsonString(action);
        auto bundleName = eventAction->GetValue("bundleName");
        auto abilityName = eventAction->GetValue("abilityName");
        auto params = eventAction->GetValue("params");
        auto bundle = bundleName->GetString();
        auto ability = abilityName->GetString();
        LOGI("bundle:%{public}s ability:%{public}s, params:%{public}s", bundle.c_str(), ability.c_str(),
            params->GetString().c_str());
        if (bundle.empty() || ability.empty()) {
            LOGE("action ability or bundle is empty");
            return;
        }

        AAFwk::Want want;
        want.SetElementName(bundle, ability);
        // want.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, std::to_string(formJsInfo_.formId));
        this->StartAbility(want);
    };

    // set window id & action event handler
    auto context = Platform::AceContainer::GetContainer(abilityId_)->GetPipelineContext();
    if (context != nullptr) {
        context->SetActionEventHandler(actionEventHandler);
    }

    // get url
    std::string parsedPageUrl;
    if (!remotePageUrl_.empty()) {
        parsedPageUrl = remotePageUrl_;
    } else if (!pageUrl_.empty()) {
        parsedPageUrl = pageUrl_;
    } else if (want.HasParameter(PAGE_URI)) {
        parsedPageUrl = want.GetStringParam(PAGE_URI);
    } else {
        parsedPageUrl = "";
    }

    // run page.
    Platform::AceContainer::RunPage(abilityId_, Platform::AceContainer::GetContainer(abilityId_)->GeneratePageId(),
        parsedPageUrl, want.GetStringParam(START_PARAMS_KEY));

    if (!remoteData_.empty()) {
        Platform::AceContainer::OnRestoreData(abilityId_, remoteData_);
    }
    LOGI("AceAbility::OnStart called End");
}

void AceAbility::OnStop()
{
    LOGI("AceAbility::OnStop called ");
#ifdef ENABLE_ROSEN_BACKEND
    if (auto context = Platform::AceContainer::GetContainer(abilityId_)->GetPipelineContext()) {
        context->SetRSUIDirector(nullptr);
    }
#endif
    Ability::OnStop();
    Platform::AceContainer::DestroyContainer(abilityId_);
    LOGI("AceAbility::OnStop called End");
}

void AceAbility::OnActive()
{
    LOGI("AceAbility::OnActive called ");
    Ability::OnActive();
    Platform::AceContainer::OnActive(abilityId_);
    LOGI("AceAbility::OnActive called End");
}

void AceAbility::OnForeground(const Want& want)
{
    LOGI("AceAbility::OnForeground called ");
    Ability::OnForeground(want);
    Platform::AceContainer::OnShow(abilityId_);
    LOGI("AceAbility::OnForeground called End");
}

void AceAbility::OnBackground()
{
    LOGI("AceAbility::OnBackground called ");
    Ability::OnBackground();
    Platform::AceContainer::OnHide(abilityId_);
    LOGI("AceAbility::OnBackground called End");
}

void AceAbility::OnInactive()
{
    LOGI("AceAbility::OnInactive called ");
    Ability::OnInactive();
    Platform::AceContainer::OnInactive(abilityId_);
    LOGI("AceAbility::OnInactive called End");
}

void AceAbility::OnBackPressed()
{
    LOGI("AceAbility::OnBackPressed called ");
    if (!Platform::AceContainer::OnBackPressed(abilityId_)) {
        LOGI("AceAbility::OnBackPressed: passed to Ability to process");
        Ability::OnBackPressed();
    }
    LOGI("AceAbility::OnBackPressed called End");
}

void AceAbility::OnPointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    auto container = Platform::AceContainer::GetContainer(abilityId_);
    if (!container) {
        LOGE("container may be destroyed.");
        return;
    }
    auto flutterAceView = static_cast<Platform::FlutterAceView*>(container->GetView());
    if (!flutterAceView) {
        LOGE("flutterAceView is null");
        return;
    }
    flutterAceView->DispatchTouchEvent(flutterAceView, pointerEvent);
}

void AceAbility::OnKeyUp(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    auto container = Platform::AceContainer::GetContainer(abilityId_);
    if (!container) {
        LOGE("container may be destroyed.");
        return;
    }
    auto flutterAceView = static_cast<Platform::FlutterAceView*>(container->GetView());
    if (!flutterAceView) {
        LOGI("flutterAceView is null, keyboard event does not take effect");
        return;
    }
    int32_t repeatTime = 0; // TODO:repeatTime need to be rebuild
    auto result = flutterAceView->DispatchKeyEvent(flutterAceView, keyEvent->GetKeyCode(), keyEvent->GetKeyAction(),
        repeatTime, keyEvent->GetActionTime(), keyEvent->GetActionStartTime());
    if (!result) {
        LOGI("AceAbility::OnKeyUp: passed to Ability to process");
        Ability::OnKeyUp(keyEvent);
    }
}

void AceAbility::OnKeyDown(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    auto container = Platform::AceContainer::GetContainer(abilityId_);
    if (!container) {
        LOGE("container may be destroyed.");
        return;
    }
    auto flutterAceView = static_cast<Platform::FlutterAceView*>(container->GetView());
    if (!flutterAceView) {
        LOGI("flutterAceView is null, keyboard event does not take effect");
        return;
    }
    int32_t repeatTime = 0; // TODO:repeatTime need to be rebuild
    auto result = flutterAceView->DispatchKeyEvent(flutterAceView, keyEvent->GetKeyCode(), keyEvent->GetKeyAction(),
        repeatTime, keyEvent->GetActionTime(), keyEvent->GetActionStartTime());
    if (!result) {
        LOGI("AceAbility::OnKeyDown: passed to Ability to process");
        Ability::OnKeyDown(keyEvent);
    }
}

void AceAbility::OnNewWant(const Want& want)
{
    LOGI("AceAbility::OnNewWant called ");
    Ability::OnNewWant(want);
    std::string params = want.GetStringParam(START_PARAMS_KEY);
    Platform::AceContainer::OnNewRequest(abilityId_, params);
    LOGI("AceAbility::OnNewWant called End");
}

void AceAbility::OnRestoreAbilityState(const PacMap& inState)
{
    LOGI("AceAbility::OnRestoreAbilityState called ");
    Ability::OnRestoreAbilityState(inState);
    LOGI("AceAbility::OnRestoreAbilityState called End");
}

void AceAbility::OnSaveAbilityState(PacMap& outState)
{
    LOGI("AceAbility::OnSaveAbilityState called ");
    Ability::OnSaveAbilityState(outState);
    LOGI("AceAbility::OnSaveAbilityState called End");
}

void AceAbility::OnConfigurationUpdated(const Configuration& configuration)
{
    LOGI("AceAbility::OnConfigurationUpdated called ");
    Ability::OnConfigurationUpdated(configuration);
    Platform::AceContainer::OnConfigurationUpdated(abilityId_, configuration.GetName());
    LOGI("AceAbility::OnConfigurationUpdated called End");
}

void AceAbility::OnAbilityResult(int requestCode, int resultCode, const OHOS::AAFwk::Want& resultData)
{
    LOGI("AceAbility::OnAbilityResult called ");
    AbilityProcess::GetInstance()->OnAbilityResult(this, requestCode, resultCode, resultData);
    LOGI("AceAbility::OnAbilityResult called End");
}

void AceAbility::OnRequestPermissionsFromUserResult(
    int requestCode, const std::vector<std::string>& permissions, const std::vector<int>& grantResults)
{
    LOGI("AceAbility::OnRequestPermissionsFromUserResult called ");
    AbilityProcess::GetInstance()->OnRequestPermissionsFromUserResult(this, requestCode, permissions, grantResults);
    LOGI("AceAbility::OnRequestPermissionsFromUserResult called End");
}

bool AceAbility::OnStartContinuation()
{
    LOGI("AceAbility::OnStartContinuation called.");
    bool ret = Platform::AceContainer::OnStartContinuation(abilityId_);
    LOGI("AceAbility::OnStartContinuation finish.");
    return ret;
}

bool AceAbility::OnSaveData(OHOS::AAFwk::WantParams& saveData)
{
    LOGI("AceAbility::OnSaveData called.");
    std::string data = Platform::AceContainer::OnSaveData(abilityId_);
    if (data == "false") {
        return false;
    }
    auto json = JsonUtil::ParseJsonString(data);
    if (!json) {
        return false;
    }
    if (json->Contains(PAGE_URI)) {
        saveData.SetParam(PAGE_URI, OHOS::AAFwk::String::Box(json->GetString(PAGE_URI)));
    }
    if (json->Contains(CONTINUE_PARAMS_KEY)) {
        std::string params = json->GetObject(CONTINUE_PARAMS_KEY)->ToString();
        saveData.SetParam(CONTINUE_PARAMS_KEY, OHOS::AAFwk::String::Box(params));
    }
    LOGI("AceAbility::OnSaveData finish.");
    return true;
}

bool AceAbility::OnRestoreData(OHOS::AAFwk::WantParams& restoreData)
{
    LOGI("AceAbility::OnRestoreData called.");
    if (restoreData.HasParam(PAGE_URI)) {
        auto value = restoreData.GetParam(PAGE_URI);
        OHOS::AAFwk::IString* ao = OHOS::AAFwk::IString::Query(value);
        if (ao != nullptr) {
            remotePageUrl_ = OHOS::AAFwk::String::Unbox(ao);
        }
    }
    if (restoreData.HasParam(CONTINUE_PARAMS_KEY)) {
        auto value = restoreData.GetParam(CONTINUE_PARAMS_KEY);
        OHOS::AAFwk::IString* ao = OHOS::AAFwk::IString::Query(value);
        if (ao != nullptr) {
            remoteData_ = OHOS::AAFwk::String::Unbox(ao);
        }
    }
    LOGI("AceAbility::OnRestoreData finish.");
    return true;
}

void AceAbility::OnCompleteContinuation(int result)
{
    Ability::OnCompleteContinuation(result);
    LOGI("AceAbility::OnCompleteContinuation called.");
    Platform::AceContainer::OnCompleteContinuation(abilityId_, result);
    LOGI("AceAbility::OnCompleteContinuation finish.");
}

void AceAbility::OnRemoteTerminated()
{
    LOGI("AceAbility::OnRemoteTerminated called.");
    Platform::AceContainer::OnRemoteTerminated(abilityId_);
    LOGI("AceAbility::OnRemoteTerminated finish.");
}

void AceAbility::OnSizeChange(OHOS::Rosen::Rect rect, OHOS::Rosen::WindowSizeChangeReason reason)
{
    uint32_t width = rect.width_;
    uint32_t height = rect.height_;
    LOGI("AceAbility::OnSizeChange width: %{public}u, height: %{public}u", width, height);
    SystemProperties::SetDeviceOrientation(height >= width ? 0 : 1);
    auto container = Platform::AceContainer::GetContainer(abilityId_);
    if (!container) {
        LOGE("container may be destroyed.");
        return;
    }
    auto flutterAceView = static_cast<Platform::FlutterAceView*>(container->GetView());

    if (!flutterAceView) {
        LOGE("flutterAceView is null");
        return;
    }

    flutter::ViewportMetrics metrics;
    metrics.physical_width = width;
    metrics.physical_height = height;
    metrics.device_pixel_ratio = density_;
    Platform::FlutterAceView::SetViewportMetrics(flutterAceView, metrics);
    Platform::FlutterAceView::SurfaceChanged(flutterAceView, width, height, 0, Convert2WindowSizeChangeReason(reason));
}

void AceAbility::OnModeChange(OHOS::Rosen::WindowMode mode)
{
    LOGI("AceAbility::OnModeChange");
}

WindowSizeChangeReason AceAbility::Convert2WindowSizeChangeReason(OHOS::Rosen::WindowSizeChangeReason reason)
{
    auto reasonValue = static_cast<uint32_t>(reason);
    constexpr uint32_t MAX_REASON_VALUE = 5;
    if (reasonValue > MAX_REASON_VALUE) {
        LOGE("AceAbility: unsupported WindowSizeChangeReason");
        return WindowSizeChangeReason::UNDEFINED;
    }
    return static_cast<WindowSizeChangeReason>(reasonValue);
}

void AceAbility::Dump(const std::vector<std::string>& params, std::vector<std::string>& info)
{
    auto container = Platform::AceContainer::GetContainer(abilityId_);
    if (!container) {
        LOGE("container may be destroyed.");
        return;
    }
    auto context = container->GetPipelineContext();
    if (context != nullptr) {
        context->DumpInfo(params, info);
    }
}

void AceAbility::OnDrag(int32_t x, int32_t y, OHOS::Rosen::DragEvent event)
{
    LOGI("AceAbility::OnDrag called ");
    auto container = Platform::AceContainer::GetContainer(abilityId_);
    if (!container) {
        LOGE("container may be destroyed.");
        return;
    }
    auto flutterAceView = static_cast<Platform::FlutterAceView*>(container->GetView());
    if (!flutterAceView) {
        LOGE("AceAbility::OnDrag flutterAceView is null");
        return;
    }

    DragEventAction action;
    switch (event) {
        case OHOS::Rosen::DragEvent::DRAG_EVENT_END:
            action = DragEventAction::DRAG_EVENT_END;
            break;
        case OHOS::Rosen::DragEvent::DRAG_EVENT_IN:
        case OHOS::Rosen::DragEvent::DRAG_EVENT_OUT:
        case OHOS::Rosen::DragEvent::DRAG_EVENT_MOVE:
        default:
            action = DragEventAction::DRAG_EVENT_START;
            break;
    }

    flutterAceView->ProcessDragEvent(x, y, action);
}
} // namespace Ace
} // namespace OHOS
