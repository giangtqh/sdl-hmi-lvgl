#ifndef _SDL_MESSAGE_CONTROLLER_
#define _SDL_MESSAGE_CONTROLLER_

#include "json/json.h"
#include "WebsocketConnection.h"

#include <atomic>
#include <map>
#include <memory>
#include <mutex> 
#include <string>

#include "GuiController.h"

namespace sdlcore_message_handler {

class SDLMessageController {
 public:
    SDLMessageController(GuiController*);
    virtual ~SDLMessageController();

    void shutdown(void);
    void startSession(void);
    void sendJsonMessage(Json::Value& msg);

 private:
    std::atomic_bool mShutdown;
    std::shared_ptr<GuiController> mGuiController;
    std::map<std::string, std::shared_ptr<WebsocketConnection>> mConnectionList;
    std::mutex mConnectionListMutex;
};

}

#endif  // _SDL_MESSAGE_CONTROLLER_