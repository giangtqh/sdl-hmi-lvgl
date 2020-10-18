
#include "WebsocketSession.h"
#include <unistd.h>
#include <utility>

#include "utils/jsoncpp_reader_wrapper.h"

// #include "utils/jsoncpp_reader_wrapper.h"
#include "utils/Log.h"

namespace sdlcore_message_handler {

const char HOST[] = "127.0.0.1";
const char PORT[] = "8087";

WebsocketSession::WebsocketSession(DataReceiveCallback data_receive_cb, OnErrorCallback error_cb)
    : resolver_(ioc_)
    , ws_(ioc_)
    , io_pool_(1)
    , data_receive_cb_(data_receive_cb)
    , error_cb_(error_cb)
    , shutdown_(false)
    , thread_delegate_(new LoopThreadDelegate(&message_queue_, this)) {
}

WebsocketSession::~WebsocketSession() {
  ioc_.stop();
  io_pool_.join();
}

WebsocketSession::Error WebsocketSession::open(void) {
    boost::system::error_code ec;
    auto const results = resolver_.resolve(HOST, PORT, ec);
    if (ec) {
      // TODO: invokes error_cb_, with respective error code?
      LOGE("WebsocketSession::%s() Could not resolve host/port: %s", __func__, ec.message().c_str());
      return FAIL;
    }

    boost::asio::connect(ws_.next_layer(), results.begin(), results.end(), ec);
    if (ec) {
      LOGE("WebsocketSession::%s() Could not connect to websocket %s:%s with err: %s", __func__, HOST, PORT, ec.message().c_str());
      return FAIL;
    }

    // Perform websocket handshake
    ws_.handshake(HOST, PORT, ec);
    if (ec) {
      LOGE("WebsocketSession::%s() Could not complete handshake with %s:%s, err: %s", __func__, HOST, PORT, ec.message().c_str());
      return FAIL;
    }

    // Set the binary message write option
    ws_.binary(true);
    // start writer thread
    thread_delegate_->startThread();
    // start async read
    ws_.async_read(buffer_, std::bind(&WebsocketSession::onRead, this, std::placeholders::_1, std::placeholders::_2));

  boost::asio::post(io_pool_, [&]() { ioc_.run(); });

  LOGD("WebsocketSession::%s() Successfully started websocket connection @:%s:%s", __func__, HOST, PORT);
  return OK;
}

void WebsocketSession::shutdown() {
  shutdown_ = true;
  if (thread_delegate_) {
    thread_delegate_->SetShutdown();
    thread_delegate_->stopThread();
    delete thread_delegate_;
  }
}

bool WebsocketSession::IsShuttingDown() {
  return shutdown_;
}

void WebsocketSession::AsyncRead(boost::system::error_code ec) {
  if (shutdown_) {
    return;
  }

  if (ec) {
    LOGE("WebsocketSession::%s() %s", __func__, ec.message().c_str());
    shutdown();
    return;
  }

  ws_.async_read(buffer_, std::bind(&WebsocketSession::onRead, this, std::placeholders::_1, std::placeholders::_2));
}

void WebsocketSession::onRead(boost::system::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    if (ec) {
        LOGE("WebsocketSession::%s() ErrorMessage: %s", __func__, ec.message().c_str());
        ws_.lowest_layer().close();
        ioc_.stop();
        shutdown();
        buffer_.consume(buffer_.size());
        error_cb_();
        return;
    }

    std::string data = boost::beast::buffers_to_string(buffer_.data());
    MessagePtr msg = std::make_shared<std::string>(data);

    utils::JsonReader reader;
    Json::Value root;
    if (!reader.parse(*msg, &root)) {
        LOGE("Invalid JSON Message.");
        return;
    }
    Json::Value error;
    if (checkMessage(root, error)) {
        // TODO: should we pass JSON direclty to data_receive_cb_
        data_receive_cb_(msg);
    } else {
        LOGE("%s(), Message contains wrong data!", __func__);
        sendJsonMessage(error);
    }
    

    buffer_.consume(buffer_.size());
    AsyncRead(ec);
}

void WebsocketSession::sendJsonMessage(Json::Value& message) {
    Json::StreamWriterBuilder m_writer;
    const std::string str_msg = Json::writeString(m_writer, message) + '\n';
    std::string msgType = "request";
    if (isNotification(message)) {
      msgType.assign("notification");
    } else if (isResponse(message)) {
      msgType.assign("response");
    }
    LOGD("%s() send %s\n %s", __func__, msgType.c_str(), str_msg.c_str());
    Send(str_msg, message);
}

void WebsocketSession::Send(const std::string& message, Json::Value& json_message) {
  if (shutdown_) {
    return;
  }
  std::shared_ptr<std::string> message_ptr = std::make_shared<std::string>(message);
  message_queue_.push(message_ptr);
}

bool WebsocketSession::checkMessage(Json::Value& root, Json::Value& error) {
    Json::Value err;
    /* check the JSON-RPC version => 2.0 */
    if (!root.isObject() || !root.isMember("jsonrpc") ||
        root["jsonrpc"] != "2.0") {
        error["id"] = Json::Value::null;
        error["jsonrpc"] = "2.0";
        err["code"] = sdlcore_message_handler::INVALID_REQUEST;
        err["message"] = "Invalid MessageBroker request.";
        error["error"] = err;
        return false;
    }

    if (root.isMember("id") && (root["id"].isArray() || root["id"].isObject())) {
        error["id"] = Json::Value::null;
        error["jsonrpc"] = "2.0";
        err["code"] = sdlcore_message_handler::INVALID_REQUEST;
        err["message"] = "Invalid MessageBroker request.";
        error["error"] = err;
        return false;
    }

    if (root.isMember("result") && root.isMember("error")) {
        /* message can't contain simultaneously result and error*/
        return false;
    }

    if (root.isMember("method")) {
        if (!root["method"].isString()) {
            error["id"] = Json::Value::null;
            error["jsonrpc"] = "2.0";
            err["code"] = sdlcore_message_handler::INVALID_REQUEST;
            err["message"] = "Invalid MessageBroker request.";
            error["error"] = err;
            return false;
        }
        /* Check the params is  an object*/
        if (root.isMember("params") && !root["params"].isObject()) {
            error["id"] = Json::Value::null;
            error["jsonrpc"] = "2.0";
            err["code"] = INVALID_REQUEST;
            err["message"] = "Invalid JSONRPC params.";
            error["error"] = err;
            return false;
        }
    } else if (!root.isMember("result") && !root.isMember("error")) {
        return false;
    }
    return true;
}

bool WebsocketSession::isNotification(Json::Value& root) {
    bool ret = false;
    if (false == root.isMember("id")) {
        ret = true;
    }
    return ret;
}

bool WebsocketSession::isResponse(Json::Value& root) {
    bool ret = false;
    if ((true == root.isMember("result")) || (true == root.isMember("error"))) {
        ret = true;
    }
    return ret;
}

WebsocketSession::LoopThreadDelegate::LoopThreadDelegate(
    utils::MessageQueue<MessagePtr>* message_queue, WebsocketSession* handler)
    : message_queue_(*message_queue), handler_(*handler), shutdown_(false) {}

void WebsocketSession::LoopThreadDelegate::run() {
  while (!message_queue_.IsShuttingDown() && !shutdown_) {
    DrainQueue();
    message_queue_.wait();
  }
  LOGD("%s() Exited.", __func__);
  DrainQueue();
}

int WebsocketSession::LoopThreadDelegate::startThread(void) {
  return Thread::startThread();
}

int WebsocketSession::LoopThreadDelegate::stopThread(void) {
  return Thread::stopThread();
}

void WebsocketSession::LoopThreadDelegate::DrainQueue() {
  while (!message_queue_.empty()) {
    MessagePtr message_ptr;
    message_queue_.pop(message_ptr);
    if (!shutdown_) {
      boost::system::error_code ec;
      handler_.ws_.write(boost::asio::buffer(*message_ptr), ec);
      if (ec) {
        LOGE("%s(), Error: %s", __func__, ec.message().c_str());
      }
    }
  }
}

void WebsocketSession::LoopThreadDelegate::SetShutdown() {
  shutdown_ = true;
  // TODO: clear message_queue_
}

}  // namespace sdlcore_message_handler
