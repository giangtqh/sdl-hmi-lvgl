
#include "SDLMessageController.h"
#include "BasicCommunication.h"

#include <utility>

namespace sdlcore_message_handler {

SDLMessageController::SDLMessageController(GuiController* gui)
    : mShutdown(false)
    , mGuiController(std::move(std::shared_ptr<GuiController>(gui))) {

}

SDLMessageController::~SDLMessageController() {
    shutdown();
}

void SDLMessageController::shutdown(void) {
    if (false == mShutdown.load(std::memory_order_acquire)) {
        mShutdown.store(true, std::memory_order_release);
        std::lock_guard<std::mutex> guard(mConnectionListMutex);
        for (auto it = mConnectionList.begin(); it != mConnectionList.end(); ++it) {
            it->second->shutdown();
        }
        mConnectionList.clear();
    }
}

void SDLMessageController::startSession(void) {
    auto bs = std::make_shared<BasicCommunication>(mGuiController);
    // TODO: define const variable for "BasicCommunication"
    mConnectionList.insert(std::pair<std::string, std::shared_ptr<WebsocketConnection>>("BasicCommunication", bs));
    // TODO: create and insert other components into list
}

void SDLMessageController::sendJsonMessage(Json::Value& msg) {
    // TODO: get method from msg, find respective Connection and invoke connection->sendJsonMessage(msg)
    std::string method = msg["method"].asString();
    std::lock_guard<std::mutex> guard(mConnectionListMutex);
    // std::map<std::string, std::shared_ptr<WebsocketConnection>>::iterator it;
    auto it = mConnectionList.find(method);
    if (it != mConnectionList.end()) {
        it->second->sendJsonMessage(msg);
    }
};

}