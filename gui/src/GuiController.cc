// For test only

#include "GuiController.h"

#include <cstdint>

#include "Log.h"
#include "json/json.h"

#include "ApplicationInfo.h"
#include "SDLMessageController.h"

#include "MainPage.h"
#include "Telephony.h"
#include "SettingsPage.h"
#include "HomePage.h"
#include "UIListPage.h"

namespace sdlcore_message_handler {

GuiController::GuiController()
    : mAppId(0)
    , page(nullptr)
    , mTelephony(nullptr)
    , mController(nullptr)
    , mHomePage(nullptr) {
}

GuiController::~GuiController() {
    shutdown();
}

void GuiController::setMsgController(SDLMessageController* controller) {
    mController = controller;
}

void GuiController::updateDeviceList(void) {
    mController->updateDeviceList();
}

void GuiController::startDeviceDiscovery(void) {
    mController->startDeviceDiscovery();
}

void GuiController::onUpdateDeviceList(const std::map<std::string, std::shared_ptr<Device>>& deviceList) {
    LOGD("GuiController::%s() deviceList size: %lu", __func__, (uint64_t)deviceList.size());
    // settingsPage->onUpdateDeviceList(deviceList);
    mHomePage->onUpdateDeviceList(deviceList);
}

void GuiController::onUpdateAppList(const std::map<uint32_t, std::shared_ptr<ApplicationInfo>>& appList) {
    LOGD("GuiController::%s() appList size: %lu", __func__, (uint64_t)appList.size());
    mAppList = appList;
    mHomePage->onUpdateAppList(appList);
}

void GuiController::onAppRegistered(const std::shared_ptr<ApplicationInfo>& app) {
    LOGD("GuiController::%s() appID: %u, appName: %s", __func__, app->mAppId, app->mAppName.c_str());
    mAppId = app->mAppId;
    mHomePage->onAppRegistered(app);
}

void GuiController::onAppUnRegistered(uint32_t appId) {
    LOGD("GuiController::%s() appID: %u", __func__, appId);
    mHomePage->onAppUnRegistered(appId);
}

// SDL request HMI to bring a specific app to the front (aka HMI level FULL)
void GuiController::onActivateApp(uint32_t appId) {
    LOGD("GuiController::%s() SDL request to active appID: %u", __func__, appId);
}

void GuiController::onSDLClose(void) {
    LOGD("GuiController::%s()", __func__);
}

void GuiController::onAlert(const Json::Value& message) {
    LOGD("GuiController::%s()", __func__);
}

void GuiController::initDisplay(void) {
    //page = new gui::MainPage(this);
    mTelephony = new gui::Telephony(this);
    #if 0
    page->init();
    page->addGauge();
    // page->addButton();
    page->start_lv_task();
    page->waitForExit();
    #else
    mHomePage = new gui::HomePage(this);
    mHomePage->init();
    mHomePage->start_lv_task();
    mHomePage->waitForExit();
    #endif
}

void GuiController::onDialNumber(uint32_t appId, const std::string number, uint32_t cancelBtnId) {
    LOGD("GuiController::%s()", __func__);
    if (mTelephony) {
        mTelephony->dialing(number.c_str());
    }
}

void GuiController::onDeviceChosen(const std::string& devId) {
    LOGD("GuiController::%s() User choose device with id=%s", __func__, devId.c_str());
    mController->deviceChosenId(devId);
}

void GuiController::onAppChosen(const uint32_t appId) {
    LOGD("GuiController::%s() User choose app with id=%u", __func__, appId);
    mController->activateApplication(appId);
}

void GuiController::onIncomingCall(uint32_t appId, const std::string number, uint32_t acceptBtnId, uint32_t denyBtnId) {
    LOGD("GuiController::%s()", __func__);
    if (mTelephony) {
        mTelephony->incomingCall(number.c_str());
    }
}

void GuiController::onStartCall() {
    LOGD("GuiController::%s()", __func__);
    if (mTelephony) {
        mTelephony->duringCall();
    }
}

void GuiController::onEndCall() {
    LOGD("GuiController::%s()", __func__);
    if (mTelephony) {
        mTelephony->endCall();
    }
}

void GuiController::onHangUpCall() {
    LOGD("GuiController::%s()", __func__);
    if (mTelephony) {
        mTelephony->hangupCall();
    }
}

void GuiController::onSMSNotificaton(uint32_t appId, const std::string number, const std::string body) {
    LOGD("GuiController::%s() number: %s, body: %s", __func__, number.c_str(), body.c_str());
}

void GuiController::shutdown(void) {
    delete mTelephony;
    delete page;
    if (mHomePage) {
        mHomePage->shutdown();
        delete mHomePage;
    }
    waitForExit();
}

void GuiController::waitForExit(void) {
    // if (guiThread.joinable()) {
    //     guiThread.join();
    // }
}

void GuiController::onShow(uint32_t appId, std::vector<std::string> showString, const std::vector<std::shared_ptr<SoftButton>>& softButtons) {
    for (auto item : showString) {
        LOGD("GuiController::%s() show string: %s", __func__, item.c_str());
    }

    if (softButtons.size() > 0) {
        LOGD("GuiController::%s() size of button: %lu", __func__, (uint64_t)softButtons.size());
    }
    mHomePage->onShow(appId, showString, softButtons);
}
#if 0
void GuiController::onUpdateSMSList(const std::map<uint32_t, std::shared_ptr<SMSMessage>>& smsList) {
    // page->addContactList();
    // for (auto item : smsList) {
    //     auto message = item.second;
    //     LOGD("GuiController::%s() SMSMessage(%s, %s, %d), cmdId: %d", __func__, message->address_.c_str(), message->body_.c_str(), message->read_, item.first);
    //     page->addContactItem(message->body_, item.first);
    // }
}
#endif
void GuiController::onUpdateListData(const ListType type, const std::vector<std::shared_ptr<ListItem>>& data) {
    if (mHomePage) {
        mHomePage->setListType(type);
        mHomePage->setListData(data);
    }
}

void GuiController::testList(void) {
    // std::unordered_map<uint32_t, std::shared_ptr<ListItem>> dummy;
    dummy.push_back(std::make_shared<SMSMessage>(12, "Tran Quang Hoang Giang", "09671281982", "", 1, 1));
    dummy.push_back(std::make_shared<SMSMessage>(14, "Nguyen Van A", "09671281982", "", 1, 1));
    dummy.push_back(std::make_shared<SMSMessage>(15, "Nguyen Nguyen Anh", "09671281982", "", 1, 1));
    dummy.push_back(std::make_shared<SMSMessage>(2, "Tran Quang Hoang XXXX", "09671281982", "", 1, 1));
    dummy.push_back(std::make_shared<SMSMessage>(4, "Tran Quang Hoang VVVV", "09671281982", "", 1, 1));
    dummy.push_back(std::make_shared<SMSMessage>(5, "Tran Quang Hoang JJJJ", "09671281982", "", 1, 1));
    dummy.push_back(std::make_shared<SMSMessage>(6, "Tran Quang Hoang KKKK", "09671281982", "", 1, 1));
    dummy.push_back(std::make_shared<SMSMessage>(7, "Tran Quang Hoang LLLLLLLLLLLLLLLLLL", "09671281982", "", 1, 1));
    dummy.push_back(std::make_shared<SMSMessage>(9, "Tran Quang Hoang MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM", "09671281982", "", 1, 1));
    dummy.push_back(std::make_shared<SMSMessage>(10, "Tran Quang Hoang NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNasdfffffffffffffffffffasdfasdfasdfasdfasdfasdf", "09671281982NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNasdfffffffffffffffffffasdfasdfasdfasdfasdfasdf", "", 1, 1));

    if (mHomePage) {
        mHomePage->setListType(ListType::SMS);
        mHomePage->setListData(dummy);
    }
}
#if 0
void GuiController::onUpdateContactList(const std::map<uint32_t, std::shared_ptr<ContactItem>>& contactList) {
    // page->addContactList();
    // for (auto item : contactList) {
    //     LOGD("GuiController::%s() Contact(%s, %s)", __func__, item.second->name_.c_str(), item.second->number_.c_str());
    //     page->addContactItem(item.second->name_, item.first);
    // }
}

void GuiController::onUpdateCallLogList(const std::map<uint32_t, std::shared_ptr<CallLogItem>>& callLog) {

}
#endif
void GuiController::onButtonPress(uint32_t btnId) {
    mController->onButtonPress(btnId, mAppId);
}

void GuiController::onListItemSelected(uint32_t cmdId) {
    mController->onListItemSelected(cmdId, mAppId);
}

}
