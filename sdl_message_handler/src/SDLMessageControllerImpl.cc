
#include <utility>

#include "SDLMessageControllerImpl.h"
#include "BasicCommunication.h"
#include "UI.h"
#include "Buttons.h"
#include "VR.h"
#include "Log.h"
#include "json/json.h"
#include "json_rc_constants.h"
#include "SDLTypes.h"
#include "jsoncpp_reader_wrapper.h"

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

    it = mConnectionList.insert(std::pair<std::string, std::shared_ptr<WebsocketConnection>>("Buttons", std::shared_ptr<WebsocketConnection>(new Buttons(this))));
    if (it.second) {
        (void) it.first->second->start();
    }

    it = mConnectionList.insert(std::pair<std::string, std::shared_ptr<WebsocketConnection>>("VR", std::shared_ptr<WebsocketConnection>(new VR(this))));
    if (it.second) {
        (void) it.first->second->start();
    }

    // TODO: create and insert other components into list
}

bool SDLMessageControllerImpl::isNotification(const Json::Value& root) {
    bool ret = false;
    if (false == root.isMember("id")) {
        ret = true;
    }
    return ret;
}

bool SDLMessageControllerImpl::isResponse(const Json::Value& root) {
    bool ret = false;
    if ((true == root.isMember("result")) || (true == root.isMember("error"))) {
        ret = true;
    }
    return ret;
}

void SDLMessageControllerImpl::sendJsonMessage(const Json::Value& message) {
    std::string componentName = "BasicCommunication";
    if (isResponse(message)) {
        componentName = getComponentName(message[json_keys::kResult][json_keys::kMethod].asString());
    } else {
        componentName = getComponentName(message["method"].asString());
    }

    // route SDL requests to BasicCommunication
    if ("SDL" == componentName) {
        componentName = "BasicCommunication";
    }
    std::lock_guard<std::mutex> guard(mConnectionListMutex);
    auto it = mConnectionList.find(componentName);
    if (it != mConnectionList.end()) {
        it->second->sendJsonMessage(message);
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
    mSoftButtons.clear();
    // mSMSList.clear();
    mListData.clear();
    // mContactList.clear();
    // mCallLogList.clear();
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
    // mGuiCallbacks.onActivateApp(appId);
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

void SDLMessageControllerImpl::onActivateApp(uint32_t appId) {
    // not in use
    mGuiCallbacks.onActivateApp(appId);
}

void SDLMessageControllerImpl::onCloseApplication(uint32_t appId) {
    mGuiCallbacks.onCloseApplication(appId);
}

void SDLMessageControllerImpl::onAlert(const Json::Value& message) {
    // Parse specific Alert type into respective callback
    const Json::Value& params = message[json_keys::kParams];
    const uint32_t appId = params[json_keys::kAppId].asUInt();
    if ((params.isMember("alertStrings"))) {
        const Json::Value& alertStrings = params["alertStrings"];
        uint64_t alertSize = (uint64_t) alertStrings.size();
        if (alertSize > 0) {
            std::string alertType = alertStrings[0]["fieldText"].asString();
            if ("ON_CALL" == alertType) {
                if (alertSize >= 2) {
                    std::string number = alertStrings[1]["fieldText"].asString();
                    // TODO: Parse and get button id for "Accept" and "Deny"
                    const uint32_t acceptButtonId = 200;
                    const uint32_t denyButtonId = 201;
                    mGuiCallbacks.onIncomingCall(appId, number, acceptButtonId, denyButtonId);
                } else {
                    LOGD("SDLMessageControllerImpl::%s() Alert do not contain phone number", __func__);
                }
            } else if ("ON_DIAL" == alertType) {
                if (alertSize >= 2) {
                    std::string number = alertStrings[1]["fieldText"].asString();
                    // TODO: consider to update inComingCallback to pass cancelButtonId
                    const uint32_t cancelButtonId = 203;
                    mGuiCallbacks.onDialNumber(appId, number, cancelButtonId);
                } else {
                    LOGD("SDLMessageControllerImpl::%s() Alert do not contain phone number", __func__);
                }
            } else if ("ON_SMS" == alertType) {
                if (alertSize >= 3) {
                    std::string number = alertStrings[1]["fieldText"].asString();
                    std::string body = alertStrings[2]["fieldText"].asString();
                    mGuiCallbacks.onSMSNotificaton(appId, number, body);
                } else {
                    LOGD("SDLMessageControllerImpl::%s() Alert do not contain phone number/message body", __func__);
                }
            } else if ("ON_END_CALL" == alertType) {
                mGuiCallbacks.onEndCall();
            } else if (("SMS_FILLED" == alertType)) {
                mIsFilledListData = true;
                mGuiCallbacks.onUpdateListData(ListType::SMS, mListData);
            } else if ("CONTACT_FILLED" == alertType) {
                mIsFilledListData = true;
                mGuiCallbacks.onUpdateListData(ListType::CONTACT, mListData);
            } else if ("CALL_LOG_FILLED" == alertType) {
                mIsFilledListData = true;
                mGuiCallbacks.onUpdateListData(ListType::CALL_LOG, mListData);
            }
        } else {
            LOGD("SDLMessageControllerImpl::%s() Alert do not contain alert string", __func__);
        }
    }
}

void SDLMessageControllerImpl::onAddCommand(const Json::Value& message) {
    const Json::Value& params = message[json_keys::kParams];
    if (!params.isMember("menuParams")) {
        LOGD("SDLMessageControllerImpl::%s() Json message do not have \"menuParams\" field.", __func__);
        return;
    }
    if (mIsFilledListData) {
        mListData.clear();
        mIsFilledListData = false;
    }
    const Json::Value& menuParams = params["menuParams"];
    std::string menuName = menuParams["menuName"].asString();
    utils::JsonReader reader;
    Json::Value menuNameJson;
    if (!reader.parse(menuName, &menuNameJson)) {
        LOGE("SDLMessageControllerImpl::%s() Invalid JSON Message.", __func__);
        return;
    }
    uint32_t cmdId = params["cmdID"].asUInt();
    ListType listType = commandIdtoListType(cmdId);
    switch (listType) {
        case ListType::CONTACT: {
            std::string name = menuNameJson["name"].asString();
            std::string number = menuNameJson["number"].asString();
            // mContactList.insert(std::pair<uint32_t, std::shared_ptr<ContactItem>>(cmdId, std::shared_ptr<ContactItem>(new ContactItem(name, number))));
            mListData.push_back(std::make_shared<ContactItem>(cmdId, name, number));
            break;
        }
        case ListType::CALL_LOG: {
            std::string name = menuNameJson["name"].asString();
            std::string number = menuNameJson["number"].asString();
            std::string date = menuNameJson["date"].asString();
            std::string duration = menuNameJson["duration"].asString();
            CallLogType type = static_cast<CallLogType>(menuNameJson["type"].asUInt());
            mListData.push_back(std::make_shared<CallLogItem>(cmdId, name, number, date, duration, type));
            break;
        }
        case ListType::SMS: {
            std::string address = menuNameJson["address"].asString();
            std::string body = menuNameJson["body"].asString();
            std::string date = menuNameJson["date"].asString();
            uint32_t read = menuNameJson["read"].asUInt();
            uint32_t type = menuNameJson["type"].asUInt();
            //mSMSList.insert(std::pair<uint32_t, std::shared_ptr<SMSMessage>>(cmdId, std::shared_ptr<SMSMessage>(new SMSMessage(address, body, date, read, type))));
            // mListData[cmdId] = std::make_shared<SMSMessage>(address, body, date, read, type);
            mListData.push_back(std::make_shared<SMSMessage>(cmdId, address, body, date, read, type));
            break;
        }
        default: {
            LOGD("SDLMessageControllerImpl::%s() Invalid list type", __func__);
            break;
        }
    }
}

void SDLMessageControllerImpl::onDeleteCommand(const Json::Value& message) {
    const Json::Value& params = message[json_keys::kParams];
    if (!params.isMember("cmdID")) {
        LOGD("SDLMessageControllerImpl::%s() Json message do not have \"cmdID\" field.", __func__);
        return;
    }
    uint32_t cmdId = params["cmdID"].asUInt();
    ListType listType = commandIdtoListType(cmdId);
    switch (listType) {
        case ListType::CONTACT:
        case ListType::CALL_LOG:
        case ListType::SMS: {
            // mSMSList.erase(cmdId);
            if (!mListData.empty()) {
                mListData.clear();
                mIsFilledListData = false;
                // mListData.erase(std::remove_if(mListData.begin(), mListData.end(), [&] ( const std::shared_ptr<ListItem>& item ) { return (item->command_id_ == cmdId); }), mListData.end());
            }
            break;
        }
        default: {
            LOGD("SDLMessageControllerImpl::%s() Invalid list type", __func__);
            break;
        }
    }
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

void SDLMessageControllerImpl::deviceChosenId(const std::string& deviceId) {
    auto dev = getDeviceById(deviceId);
    if (dev) {
        Json::Value msg;
        msg[json_keys::kJsonrpc] = "2.0";
        msg[json_keys::kMethod] = "BasicCommunication.OnDeviceChosen";
        msg[json_keys::kParams][app_infos::kDeviceInfo]["name"] = dev->mName;
        msg[json_keys::kParams][app_infos::kDeviceInfo]["id"] = dev->mId;
        sendJsonMessage(msg);
        findApplications(*dev.get());
    } else {
        LOGD("SDLMessageControllerImpl::%s() Device with id=%s not found.", __func__, deviceId.c_str());
    }
}

void SDLMessageControllerImpl::findApplications(const Device& device) {
    Json::Value msg;
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "BasicCommunication.OnFindApplications";
    msg[json_keys::kParams]["deviceInfo"]["name"] = device.mName;
    msg[json_keys::kParams]["deviceInfo"]["id"] = device.mId;
    sendJsonMessage(msg);
}

void SDLMessageControllerImpl::updateDeviceList(void) {
    Json::Value msg;
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "BasicCommunication.OnUpdateDeviceList";
    sendJsonMessage(msg);
}

void SDLMessageControllerImpl::onButtonPress(const uint32_t customButtonID, const uint32_t appID) {
    onButtonEvent("CUSTOM_BUTTON", BUTTONDOWN, customButtonID, appID);
    onButtonPress("CUSTOM_BUTTON", SHORT, customButtonID, appID);
    onButtonEvent("CUSTOM_BUTTON", BUTTONUP, customButtonID, appID);
}

void SDLMessageControllerImpl::onListItemSelected(const uint32_t cmdId, const uint32_t appID) {
    Json::Value msg;
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "UI.OnCommand";
    msg[json_keys::kParams]["cmdID"] = cmdId;
    msg[json_keys::kParams][json_keys::kAppId] = appID;
    sendJsonMessage(msg);
}

void SDLMessageControllerImpl::onButtonEvent(const std::string& name, const ButtonEventMode mode, const uint32_t customButtonID, const uint32_t appID) {
    Json::Value msg;
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "Buttons.OnButtonEvent";
    msg[json_keys::kParams]["name"] = name;
    msg[json_keys::kParams]["mode"] = (mode == BUTTONUP) ? "BUTTONUP" : "BUTTONDOWN";
    msg[json_keys::kParams]["customButtonID"] = customButtonID;
    msg[json_keys::kParams]["appID"] = appID;
    sendJsonMessage(msg);
}

void SDLMessageControllerImpl::onButtonPress(const std::string& name, const ButtonPressMode mode, const uint32_t customButtonID, const uint32_t appID) {
    Json::Value msg;
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "Buttons.OnButtonPress";
    msg[json_keys::kParams]["name"] = name;
    msg[json_keys::kParams]["mode"] = (mode == LONG) ? "LONG" : "SHORT";
    msg[json_keys::kParams]["customButtonID"] = customButtonID;
    msg[json_keys::kParams]["appID"] = appID;
    sendJsonMessage(msg);
}

void SDLMessageControllerImpl::OnButtonSubscription(const std::string& name, bool isSubscribed, uint32_t appId) {
    // Notify the HMI that the specified application's interest in receiving
    //notifications for a button has changed.
    mGuiCallbacks.OnButtonSubscription(name, isSubscribed, appId);
}

void SDLMessageControllerImpl::onShow(const Json::Value& root) {
    LOGD("SDLMessageControllerImpl::%s()", __func__);
    uint32_t appId = root[json_keys::kParams][app_infos::kAppID].asUInt();
    const Json::Value& params = root[json_keys::kParams];
    // showString will be replace for each UI.Show
    std::vector<std::string> showStrings;
    if (params.isMember("showStrings")) {
        const Json::Value& showStringArray = params["showStrings"];
        for (auto item : showStringArray) {
            showStrings.push_back(item["fieldText"].asString());
        }
    }
    std::vector<std::shared_ptr<SoftButton>> softButtons;
    if (params.isMember("softButtons")) {
        const Json::Value& softButtonArray = params["softButtons"];
        for (auto item : softButtonArray) {
            uint32_t id = item["softButtonID"].asUInt();
            std::string type = item["type"].asString();
            std::string text = item["text"].asString();
            softButtons.push_back(std::shared_ptr<SoftButton>(new SoftButton(id, text, type)));
        }
        mSoftButtons.swap(softButtons);
    }

    // Currenlty SDL core call this callback twice for fully update showString (4 mainField).
    // The first call only contain mainField1 and 3 buttons
    // the second call for all mainField (4 mainFields)
    // TODO(GTR): Fix Me
    if (!mSoftButtons.empty()) {
        mGuiCallbacks.onShow(appId, showStrings, mSoftButtons);
    }
    Json::Value response;
    response[json_keys::kId] = root[json_keys::kId].asInt();
    response[json_keys::kJsonrpc] = "2.0";
    response["result"][json_keys::kMethod] = root[json_keys::kMethod].asString();
    response["result"]["code"] = 0;
    sendJsonMessage(response);
}

const ListType SDLMessageControllerImpl::stringToListType(const std::string& sType) {
    ListType type = ListType::CONTACT;
    if ("CONTACT_LIST" == sType) {
        type = ListType::CONTACT;
    } else if ("CALL_LOG" == sType) {
        type = ListType::CALL_LOG;
    } else if ("MESSAGE" == sType) {
        type = ListType::SMS;
    }
    return type;
}

// TODO(GTR): will be removed
std::string SDLMessageControllerImpl::listToTouchType(const ListType& sType) {
    std::string touchType = "BEGIN";
    if (ListType::CONTACT == sType) {
        touchType.assign("BEGIN");
    } else if (ListType::CALL_LOG == sType) {
        touchType.assign("MOVE");
    } else if (ListType::SMS == sType) {
        touchType.assign("END");
    }
    return touchType;
}

const ListType SDLMessageControllerImpl::commandIdtoListType(const uint32_t cmdId) {
    ListType type = ListType::CONTACT;
    if (cmdId <= 1000) {
        type = ListType::CONTACT;
    } else if (cmdId <= 2000) {
        type = ListType::CALL_LOG;
    } else if (cmdId <= 3000) {
        type = ListType::SMS;
    }
    return type;
}

}
