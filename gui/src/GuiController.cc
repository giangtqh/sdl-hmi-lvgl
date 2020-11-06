// For test only

#include "GuiController.h"

#include <cstdint>

#include "Log.h"
#include "json/json.h"

#include "ApplicationInfo.h"
#include "SDLMessageController.h"

#include "MainPage.h"
#include "Telephony.h"

namespace sdlcore_message_handler {

GuiController::GuiController()
    : mAppId(0)
    , page(nullptr)
    , mTelephony(nullptr)
    , mController(nullptr) {}

GuiController::~GuiController() {
    shutdown();
}

void GuiController::setMsgController(SDLMessageController* controller) {
    mController = controller;
}

void GuiController::onUpdateDeviceList(const std::map<std::string, std::shared_ptr<Device>>& deviceList) {
    LOGD("GuiController::%s() deviceList size: %lu", __func__, deviceList.size());
    if (deviceList.size() > 0) {
        // page->addButton(deviceList.begin()->second->mName);
        // Just take the first device as example
        const auto dev = deviceList.begin()->second;
        page->addButton(dev->mName, dev->mId);
    } else {
        //page->removeButton();
    }
}

void GuiController::onUpdateAppList(const std::map<uint32_t, std::shared_ptr<ApplicationInfo>>& appList) {
    LOGD("GuiController::%s() appList size: %lu", __func__, appList.size());
    mAppList = appList;
}

void GuiController::onAppRegistered(const std::shared_ptr<ApplicationInfo>& app) {
    LOGD("GuiController::%s() appID: %u, appName: %s", __func__, app->mAppId, app->mAppName.c_str());
    mAppId = app->mAppId;
    // activeApp();
}

void GuiController::onAppUnRegistered(uint32_t appId) {
    LOGD("GuiController::%s() appID: %u", __func__, appId);
}

// SDL request HMI to bring a specific app to the front (aka HMI level FULL)
void GuiController::onActivateApp(uint32_t appId) {
    LOGD("GuiController::%s() SDL request to active appID: %u", __func__, appId);
    // TODO: For test only, fake user chosen app from app list
    // mController->activateApplication(mAppId);
}

void GuiController::onSDLClose(void) {
    LOGD("GuiController::%s()", __func__);
}

void GuiController::onAlert(const Json::Value& message) {
    LOGD("GuiController::%s()", __func__);
}

void GuiController::initDisplay(void) {
    // guiThread = std::thread(demo_sdl2w);
    page = new gui::MainPage(this);
    mTelephony = new gui::Telephony(this);
    page->init();
    page->addGauge();
    // page->addButton();
    page->start_lv_task();
    page->waitForExit();
}

void GuiController::onDialNumber(uint32_t appId, const std::string& number) {
    LOGD("GuiController::%s()", __func__);
    if (mTelephony) {
        mTelephony->dialing(number.c_str());
    }
}

void GuiController::onDeviceChosen(const std::string& devId) {
    LOGD("GuiController::%s() User choose device with id=%s", __func__, devId.c_str());
    mController->deviceChosenId(devId);
    // HMI should show App list on device
    auto app = mAppList.begin()->second;
    page->addApp(app->mAppName, app->mAppId);
}

void GuiController::onAppChosen(const uint32_t appId) {
    LOGD("GuiController::%s() User choose app with id=%u", __func__, appId);
    mController->activateApplication(appId);
    // HMI should show App list on device
    auto app = mAppList.begin()->second;
    page->addApp(app->mAppName, app->mAppId);
}

void GuiController::onIncomingCall(uint32_t appId, const std::string& number) {
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

void GuiController::shutdown(void) {
    delete mTelephony;
    delete page;
    waitForExit();
}

void GuiController::waitForExit(void) {
    // if (guiThread.joinable()) {
    //     guiThread.join();
    // }
}

}
