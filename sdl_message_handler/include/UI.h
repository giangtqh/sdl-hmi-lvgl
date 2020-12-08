
#ifndef _SDL_MESSAGE_HANDLER_UI_
#define _SDL_MESSAGE_HANDLER_UI_

#include "WebsocketConnection.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include "Thread.h"

#include "json/json.h"
#include "WebsocketSession.h"

namespace sdlcore_message_handler {
// class WebsocketSession;
class SDLMessageControllerImpl;

class UI : public WebsocketConnection {
 public:
    UI(SDLMessageControllerImpl* controller);

    virtual ~UI();

    virtual void sendJsonMessage(const Json::Value& root);
    virtual bool start(void);
    virtual void shutdown(void);

 protected:
    void onMessageReceived(MessagePtr msg);
    // TODO: error with err code
    void onError(void);
    void onAddCommand(const Json::Value&);
    void onGetCapabilities(const Json::Value&);

 private:
    void registerComponent(void);
    void unregisterComponent(void);
    void subscribeTo(const std::string& property);
    void unsubscribeFrom(const std::string& property);

    void processResponse(const Json::Value& root);
    void processRequest(const Json::Value& root);
    void processNotification(const Json::Value& root);

    std::string findMethodById(const std::string& id);

    int generateId(void);
    void subscribeNotifications(void);
    void unsubscribeNotifications(void);

    std::atomic_bool mShutdown;
    std::string mId;
    std::string mName;
    std::shared_ptr<WebsocketSession>  mWs;
    // requests from SDL
    std::map<std::string, std::string> mRequestList;
    std::mutex                         mRequestListMutex;

    // requests from HMI
    std::map<std::string, std::string> mWaitResponseQueue;
    std::mutex                         mWaitResponseQueueMutex;

    // unique request ids for different components in RPC
    int mRequestId;
    // mStartId is received as a response for registerRPCComponent messages
    int mStartId;
    SDLMessageControllerImpl* mMsgController;
};

}

#endif  // _SDL_MESSAGE_HANDLER_UI_
