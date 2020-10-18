
#ifndef _SDL_MESSAGE_HANDLER_BASIC_COMMUNICATION_
#define _SDL_MESSAGE_HANDLER_BASIC_COMMUNICATION_

#include "WebsocketConnection.h"

#include <cstdint>
#include <string>
#include <memory>
#include <algorithm>
#include <atomic>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>

#include "json/json.h"
#include "WebsocketSession.h"
#include "GuiController.h"

namespace sdlcore_message_handler {
// class WebsocketSession;

class BasicCommunication : public WebsocketConnection {
 public:
    BasicCommunication(std::shared_ptr<GuiController> gui);

    virtual ~BasicCommunication();

    // void startSession(boost::system::error_code ec)
    virtual void registerComponent(void);
    virtual void unregisterComponent(void);
    virtual void subscribeTo(std::string property);
    virtual void unsubscribeFrom(std::string property);
    virtual void onReady(void);

    virtual void sendJsonMessage(Json::Value& root);
    virtual void shutdown(void);

 protected:
    void onMessageReceived(MessagePtr msg);
    // TODO: error with err code
    void onError(void);
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
    void processResponse(Json::Value& root);
    void processRequest(Json::Value& root);
    void processNotification(Json::Value& root);

    std::string findMethodById(std::string id);

    std::atomic_bool mShutdown;
    std::string mId;
    std::string mName;
    std::shared_ptr<WebsocketSession> mWs;
    std::mutex                         mWaitResponseQueueMutex;
    std::map<std::string, std::string> mWaitResponseQueue;
    std::shared_ptr<GuiController>     mGuiController;
};

}

#endif  // _SDL_MESSAGE_HANDLER_BASIC_COMMUNICATION_
