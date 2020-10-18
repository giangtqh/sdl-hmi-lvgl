
#ifndef WEBSOCKET_SESSION_H
#define WEBSOCKET_SESSION_H

#include <algorithm>
#include <atomic>
#include <boost/asio/connect.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/thread_pool.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include "json/json.h"
#include <mutex>
#include <condition_variable>
#include <queue>

#include "utils/Thread.h"
#include "utils/MessageQueue.h"
#include "SdlcoreTypes.h"

namespace sdlcore_message_handler {

typedef std::queue<std::shared_ptr<std::string> > AsyncQueue;
typedef std::shared_ptr<std::string> MessagePtr;
using DataReceiveCallback = std::function<void(MessagePtr)>;
// TODO: error callback with code
using OnErrorCallback = std::function<void()>;

class WebsocketSession: public std::enable_shared_from_this<WebsocketSession> {
    boost::asio::io_context ioc_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
    boost::beast::multi_buffer buffer_;
    boost::asio::thread_pool io_pool_;

  public:
    enum Error {
        UNKNOWN = -1,
        OK,
        FAIL,
        NOT_SUPPORTED,
        ALREADY_EXISTS,
        BAD_STATE,
        BAD_PARAM
    };
    WebsocketSession(DataReceiveCallback data_receive,
                    OnErrorCallback on_error);
    ~WebsocketSession();
    WebsocketSession::Error open(void);
    void sendJsonMessage(Json::Value& message);
    void shutdown();

 private:
    bool IsShuttingDown();
    void AsyncRead(boost::system::error_code ec);

    void Send(const std::string& message, Json::Value& json_message);
    void onRead(boost::system::error_code ec, std::size_t bytes_transferred);

    bool checkMessage(Json::Value& root, Json::Value& error);
    bool isNotification(Json::Value& root);
    bool isResponse(Json::Value& root);
    std::string GetComponentName(std::string& method);

 protected:
    DataReceiveCallback data_receive_cb_;
    OnErrorCallback error_cb_;

 private:
    std::atomic_bool shutdown_;

    class LoopThreadDelegate : public utils::Thread {
    public:
        LoopThreadDelegate(utils::MessageQueue<MessagePtr>* message_queue,
                          WebsocketSession* handler);
        virtual void run();
        void OnWrite();
        void SetShutdown();
        int startThread(void);
        int stopThread(void);
        void notify(void) {}
    private:
        void DrainQueue();
        utils::MessageQueue<MessagePtr>& message_queue_;
        WebsocketSession& handler_;
        std::mutex        mMutex;
        std::condition_variable   mConVar;
        std::atomic_bool write_pending_;
        std::atomic_bool shutdown_;
    };

    utils::MessageQueue<MessagePtr> message_queue_;
    LoopThreadDelegate* thread_delegate_;
};

}  // namespace sdlcore_message_handler

#endif /* WEBSOCKET_SESSION_H */