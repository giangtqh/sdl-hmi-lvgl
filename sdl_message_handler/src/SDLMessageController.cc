#include "SDLMessageController.h"
#include "SDLMessageControllerImpl.h"

namespace sdlcore_message_handler {

SDLMessageController::SDLMessageController(GuiCallbacks* callback)
    : mShutdown(false) {

    mImpl = std::make_shared<SDLMessageControllerImpl>(*callback);
}

SDLMessageController::~SDLMessageController() {
    shutdown();
}

void SDLMessageController::shutdown(void) {
    if (false == mShutdown.load(std::memory_order_acquire)) {
        mShutdown.store(true, std::memory_order_release);
        mImpl->shutdown();
    }
}

void SDLMessageController::start(void) {
    if (mImpl) {
        mImpl->startSession();
    }
}

void SDLMessageController::activateApplication(uint32_t appId) {
    if (mImpl) {
        mImpl->activateApplication(appId);
    }
}

void SDLMessageController::deactivateApplication(uint32_t appId) {
    if (mImpl) {
        mImpl->deactivateApplication(appId);
    }
}

void SDLMessageController::exitAllApplications(const std::string& reason) {
    if (mImpl) {
        mImpl->exitAllApplications(reason);
    }
}

void SDLMessageController::startDeviceDiscovery(void) {
    if (mImpl) {
        mImpl->startDeviceDiscovery();
    }
}

void SDLMessageController::deviceChosen(const Device& device) {
    if (mImpl) {
        mImpl->deviceChosen(device);
    }
}

void SDLMessageController::findApplications(const Device& device) {
    if (mImpl) {
        mImpl->findApplications(device);
    }
}

void SDLMessageController::updateDeviceList() {
    if (mImpl) {
        mImpl->updateDeviceList();
    }
}

}
