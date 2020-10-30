// For test only

#include "GuiController.h"

#include <cstdint>

#include "Log.h"
#include "json/json.h"

#include "ApplicationInfo.h"
#include "SDLMessageController.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "sdl2w.h"
#ifdef __cplusplus
} /* extern "C" */
#endif
namespace sdlcore_message_handler {

GuiController::GuiController()
    : mAppId(0)
    , mController(nullptr) {}

GuiController::~GuiController() {
    shutdown();
}

void GuiController::setMsgController(SDLMessageController* controller) {
    mController = controller;
}

void GuiController::onUpdateDeviceList(const std::map<std::string, std::shared_ptr<Device>>& deviceList) {
    LOGD("GuiController::%s() deviceList size: %lu", __func__, deviceList.size());
}

void GuiController::onUpdateAppList(const std::map<uint32_t, std::shared_ptr<ApplicationInfo>>& appList) {
    LOGD("GuiController::%s() appList size: %lu", __func__, appList.size());
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
    mController->activateApplication(mAppId);
}

void GuiController::onSDLClose(void) {
    LOGD("GuiController::%s()", __func__);
}

void GuiController::onAlert(const Json::Value& message) {
    LOGD("GuiController::%s()", __func__);
}

void GuiController::initDisplay(void) {
    guiThread = std::thread(demo_sdl2w);
}

void GuiController::shutdown(void) {
    waitForExit();
}

void GuiController::waitForExit(void) {
    if (guiThread.joinable()) {
        guiThread.join();
    }
}

}
