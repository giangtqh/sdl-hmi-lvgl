
#ifndef _SDL_MESSAGE_HANDLER_WEB_SOCKET_CONNECTION_
#define _SDL_MESSAGE_HANDLER_WEB_SOCKET_CONNECTION_

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

#include "json/json.h"

namespace sdlcore_message_handler {

class WebsocketConnection {
 public:
    WebsocketConnection();
    virtual ~WebsocketConnection();

    virtual void registerComponent(void) = 0;
    virtual void unregisterComponent(void) = 0;
    virtual void subscribeTo(std::string property) = 0;
    virtual void unsubscribeFrom(std::string property) = 0;

    virtual void onReady(void) = 0;

    virtual void sendJsonMessage(Json::Value& msg) = 0;

    virtual void shutdown(void) = 0;

 protected:
    bool checkMessage(Json::Value& root, Json::Value& error);
    bool isNotification(Json::Value& root);
    bool isResponse(Json::Value& root);
    std::string GetComponentName(std::string& method);
};
}
#endif  // _SDL_MESSAGE_HANDLER_WEB_SOCKET_CONNECTION_
