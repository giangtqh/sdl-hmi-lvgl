
#ifndef _SDL_MESSAGE_HANDLER_UI_
#define _SDL_MESSAGE_HANDLER_UI_

#include "WebsocketConnection.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>

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
    void onShow(const Json::Value&);
    void onAddCommand(const Json::Value&);
    void onGetCapabilities(const Json::Value&);
    /**
    * Sent notification to SDL when HMI closes
    */
    // OnIgnitionCycleOver()
    // Send request if device was unpaired from HMI
    // OnDeviceStateChanged()
    /**
    * This methos is request to get list of registered apps.
    */
    // OnFindApplications()
    // OnSystemError()
    // OnAppDeactivated()
    // Sender: HMI->SDL. When: upon phone-call event started or ended
    // OnPhoneCall()
    // Used by HMI when User chooses to exit application.
    // ExitApplication()
    // Notify if device is choosed
    // OnDeviceChosen()
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
