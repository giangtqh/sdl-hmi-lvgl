
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
    virtual void sendJsonMessage(const Json::Value& msg) = 0;
    virtual bool start(void) = 0;
    virtual void shutdown(void) = 0;

 protected:
    bool checkMessage(const Json::Value& root, Json::Value& error);
    bool isNotification(const Json::Value& root);
    bool isResponse(const Json::Value& root);
    std::string getComponentName(const std::string& method);
};
}
#endif  // _SDL_MESSAGE_HANDLER_WEB_SOCKET_CONNECTION_
