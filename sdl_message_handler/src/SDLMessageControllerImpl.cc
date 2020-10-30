
#include "SDLMessageControllerImpl.h"
#include "BasicCommunication.h"
#include "UI.h"
#include "Log.h"
#include "json/json.h"
#include "json_rc_constants.h"
#include <utility>

namespace sdlcore_message_handler {

SDLMessageControllerImpl::SDLMessageControllerImpl(GuiCallbacks& callback)
    : mShutdown(false)
    , mGuiCallbacks(callback) {

}

SDLMessageControllerImpl::~SDLMessageControllerImpl() {
    LOGD("SDLMessageControllerImpl::%s()", __func__);
    shutdown();
}

void SDLMessageControllerImpl::shutdown(void) {
    if (false == mShutdown.load(std::memory_order_acquire)) {
        mShutdown.store(true, std::memory_order_release);
        std::lock_guard<std::mutex> guard(mConnectionListMutex);
        for (auto item : mConnectionList) {
            item.second->shutdown();
        }
    } else {
        LOGD("SDLMessageControllerImpl::%s() already shutdown.", __func__);
    }
}

void SDLMessageControllerImpl::startSession(void) {
    auto it = mConnectionList.insert(std::pair<std::string, std::shared_ptr<WebsocketConnection>>("BasicCommunication", std::shared_ptr<WebsocketConnection>(new BasicCommunication(this))));
    if (it.second) {
        (void) it.first->second->start();
    }

    it = mConnectionList.insert(std::pair<std::string, std::shared_ptr<WebsocketConnection>>("UI", std::shared_ptr<WebsocketConnection>(new UI(this))));
    if (it.second) {
        (void) it.first->second->start();
    }

    // TODO: create and insert other components into list
}

void SDLMessageControllerImpl::sendJsonMessage(const Json::Value& msg) {
    std::string componentName = getComponentName(msg["method"].asString());
    // route SDL requests to BasicCommunication
    if ("SDL" == componentName) {
        componentName = "BasicCommunication";
    }
    std::lock_guard<std::mutex> guard(mConnectionListMutex);
    auto it = mConnectionList.find(componentName);
    if (it != mConnectionList.end()) {
        it->second->sendJsonMessage(msg);
    } else {
        LOGE("%s() Component not found!", __func__);
    }
}

std::string SDLMessageControllerImpl::getComponentName(const std::string& method) {
    std::string return_string = "";
    if (method != "") {
        int position = method.find(".");
        if (position != -1) {
        return_string = method.substr(0, position);
        }
    }
    return return_string;
}

void SDLMessageControllerImpl::onAppRegistered(const Json::Value& message) {
    const Json::Value& application = message[json_keys::kParams][app_infos::kApplication];
    uint32_t appId = application[app_infos::kAppID].asUInt();
    std::string appName = application[app_infos::kAppName].asString();
    // std::string languageDesired = application[app_infos::kHMILanguageDesired].asString();
    // std::string icon = application[app_infos::kIcon].asString();
    // bool isMedia = application[app_infos::kIsMediaApp].asBool();
    // std::string npmMediaScreenAppname = application[app_infos::kNgnMediaScreenAppName].asString();
    // std::string policy = application[app_infos::kPolicyAppId].asString();
    // std::string priority = message[json_keys::kParams][app_infos::kPriority].asString();

    // Check "deviceInfo" with mDeviceList
    std::shared_ptr<Device> device = nullptr;
    const Json::Value& deviceInfo = application[app_infos::kDeviceInfo];
    std::string deviceId = deviceInfo[json_keys::kId].asString();
    {
        std::lock_guard<std::mutex> guard(mDeviceListMutex);
        auto it = mDeviceList.find(deviceId);
        if (it == mDeviceList.end()) {
            std::string id = deviceInfo[json_keys::kId].asString();
            std::string name = deviceInfo["name"].asString();
            std::string transportType = deviceInfo["transportType"].asString();
            bool isSDLAllowed = deviceInfo["isSDLAllowed"].asBool();
            device = std::make_shared<Device>(id, name, transportType, isSDLAllowed);

        } else {
            device = it->second;
        }
    }
    std::shared_ptr<ApplicationInfo> app = std::make_shared<ApplicationInfo>(appId, appName, device);
    const Json::Value& vrSynonyms = message[json_keys::kParams][app_infos::kVRSynonyms];
    std::vector<std::string> vec_vrSynonym;
    for (auto item : vrSynonyms) {
        app->mVRSynonyms.push_back(item.asString());
    }
    {
        std::lock_guard<std::mutex> guard(mAppListMutex);
        auto it = mAppList.find(appId);
        if (it != mAppList.end()) {
            mAppList.insert(std::pair<uint32_t, std::shared_ptr<ApplicationInfo>>(appId, app));
        }
    }

    mGuiCallbacks.onAppRegistered(app);
}

void SDLMessageControllerImpl::onAppUnRegistered(uint32_t appId, bool unexpectedDisconnect) {
    std::lock_guard<std::mutex> guard(mAppListMutex);
    auto it = mAppList.find(appId);
    if (it != mAppList.end()) {
        mAppList.erase(it);
    }
    mGuiCallbacks.onAppUnRegistered(appId);
}

void SDLMessageControllerImpl::onUpdateDeviceList(const Json::Value& message) {
    const Json::Value& deviceList = message[json_keys::kParams][app_infos::kDeviceList];
    std::map<std::string, std::shared_ptr<Device>> newList;
    for (auto item : deviceList) {
        std::string id = item[json_keys::kId].asString();
        std::string name = item["name"].asString();
        std::string transportType = item["transportType"].asString();
        bool isSDLAllowed = item["isSDLAllowed"].asBool();
        newList[id] = std::make_shared<Device>(id, name, transportType, isSDLAllowed);
    }
    {
        std::lock_guard<std::mutex> guard(mDeviceListMutex);
        mDeviceList.swap(newList);
    }
        // TODO: manage life time of mDeviceList, use with care inside GuiController, it may get destroyed before GuiController
    mGuiCallbacks.onUpdateDeviceList(mDeviceList);
}

void SDLMessageControllerImpl::onUpdateAppList(const Json::Value& message) {
    // TODO: Looks no information change, so currently no update to GUI
    const Json::Value& appList = message[json_keys::kParams][app_infos::kApplications];
    std::map<uint32_t, std::shared_ptr<ApplicationInfo>> newAppList;
    for (auto item : appList) {
        uint32_t appId = item[app_infos::kAppID].asUInt();
        std::string appName = item[app_infos::kAppName].asString();
        // std::string languageDesired = item[app_infos::kHMILanguageDesired].asString();
        // bool isMedia = item[app_infos::kIsMediaApp].asBool();
        // std::string npmMediaScreenAppname = item[app_infos::kNgnMediaScreenAppName].asString();
        std::shared_ptr<Device> device = nullptr;
        const Json::Value& deviceInfo = item[app_infos::kDeviceInfo];
        std::string deviceId = deviceInfo[json_keys::kId].asString();
        {
            std::lock_guard<std::mutex> guard(mDeviceListMutex);
            auto it = mDeviceList.find(deviceId);
            if (it == mDeviceList.end()) {
                std::string id = deviceInfo[json_keys::kId].asString();
                std::string name = deviceInfo["name"].asString();
                std::string transportType = deviceInfo["transportType"].asString();
                bool isSDLAllowed = deviceInfo["isSDLAllowed"].asBool();
                device = std::make_shared<Device>(id, name, transportType, isSDLAllowed);
            } else {
                device = it->second;
            }
        }
        auto app = std::make_shared<ApplicationInfo>(appId, appName, device);
        const Json::Value& vrSynonyms = item[app_infos::kVRSynonyms];
        std::vector<std::string> vec_vrSynonym;
        for (auto item : vrSynonyms) {
            app->mVRSynonyms.push_back(item.asString());
        }
        newAppList[appId] = app;
    }
    {
        std::lock_guard<std::mutex> guard(mAppListMutex);
        mAppList.swap(newAppList);
    }
    mGuiCallbacks.onUpdateAppList(mAppList);
}

void SDLMessageControllerImpl::setAppIcon(uint32_t appId, const std::string& path) {
    {
        std::lock_guard<std::mutex> guard(mAppListMutex);
        auto it = mAppList.find(appId);
        if (it != mAppList.end()) {;
            it->second->mIcon.assign(path);
        }
    }
    // notify HMI for triggering SDL.ActivateApp
    mGuiCallbacks.onActivateApp(appId);
}

std::shared_ptr<ApplicationInfo> SDLMessageControllerImpl::getAppById(uint32_t id) {
    std::lock_guard<std::mutex> guard(mDeviceListMutex);
    std::shared_ptr<ApplicationInfo> app = nullptr;
    auto it = mAppList.find(id);
    if (it != mAppList.end()) {
        app = it->second;
    }
    return app;
}

std::shared_ptr<Device> SDLMessageControllerImpl::getDeviceById(const std::string& id) {
    std::lock_guard<std::mutex> guard(mDeviceListMutex);
    std::shared_ptr<Device> device = nullptr;
    auto it = mDeviceList.find(id);
    if (it != mDeviceList.end()) {
        device = it->second;
    }
    return device;
}

void SDLMessageControllerImpl::onSDLClose(void) {
    mGuiCallbacks.onSDLClose();
    shutdown();
}

void SDLMessageControllerImpl::onDialNumber(uint32_t appId, const std::string& number) {
    mGuiCallbacks.onDialNumber(appId, number);
    shutdown();
}

void SDLMessageControllerImpl::onActivateApp(uint32_t appId) {
    // not in use
    mGuiCallbacks.onActivateApp(appId);
}

void SDLMessageControllerImpl::onCloseApplication(uint32_t appId) {
    mGuiCallbacks.onCloseApplication(appId);
}

void SDLMessageControllerImpl::onAlert(const Json::Value& msg) {
    mGuiCallbacks.onAlert(msg);
}

void SDLMessageControllerImpl::activateApplication(uint32_t appId) {
    Json::Value msg;
    msg[json_keys::kJsonrpc] = "2.0";
    // msg[json_keys::kMethod] = "BasicCommunication.OnAppActivated";
    msg[json_keys::kMethod] = "SDL.ActivateApp";
    msg[json_keys::kParams][app_infos::kAppID] = appId;
    sendJsonMessage(msg);
}

void SDLMessageControllerImpl::deactivateApplication(uint32_t appId) {
    Json::Value msg;
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "BasicCommunication.OnAppDeactivated";
    msg[json_keys::kParams][app_infos::kAppID] = appId;
    sendJsonMessage(msg);
}

void SDLMessageControllerImpl::exitAllApplications(const std::string& reason) {
    Json::Value msg;
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "BasicCommunication.OnExitAllApplications";
    msg[json_keys::kParams]["reason"] = reason;
    sendJsonMessage(msg);
}

void SDLMessageControllerImpl::startDeviceDiscovery(void) {
    Json::Value msg;
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "BasicCommunication.OnStartDeviceDiscovery";
    sendJsonMessage(msg);
}

void SDLMessageControllerImpl::deviceChosen(const Device& device) {
    Json::Value msg;
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "BasicCommunication.OnDeviceChosen";
    msg[json_keys::kParams][app_infos::kDeviceInfo]["name"] = device.mName;
    msg[json_keys::kParams][app_infos::kDeviceInfo]["id"] = device.mId;
    sendJsonMessage(msg);
}

void SDLMessageControllerImpl::findApplications(const Device& device) {
    Json::Value msg;
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "BasicCommunication.OnFindApplications";
    msg[json_keys::kParams][app_infos::kDeviceInfo]["name"] = device.mName;
    msg[json_keys::kParams][app_infos::kDeviceInfo]["id"] = device.mId;
    sendJsonMessage(msg);
}

void SDLMessageControllerImpl::updateDeviceList(void) {
    Json::Value msg;
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "BasicCommunication.OnUpdateDeviceList";
    sendJsonMessage(msg);
}

}
