#include "BasicCommunication.h"

#include "utils/jsoncpp_reader_wrapper.h"
#include "utils/json_rc_constants.h"
#include "utils/Log.h"
// #include "SdlcoreTypes.h"

#include <utility>

namespace sdlcore_message_handler {

BasicCommunication::BasicCommunication(std::shared_ptr<GuiController> gui)
    : WebsocketConnection()
    , mShutdown(false)
    , mId("600")
    , mName("BasicCommunication")
    , mGuiController(gui)
    , mRequestId(-1)
    , mStartId(-1) {
    mWs = std::make_shared<WebsocketSession>(mId, mName,
                                             std::bind(&BasicCommunication::onMessageReceived, this, std::placeholders::_1),
                                             std::bind(&BasicCommunication::onError, this));
    WebsocketSession::Error ret = mWs->open();
    if (WebsocketSession::OK != ret) {
        LOGE("%s() Failed to open Websocket connection", __func__);
    }
    // if (mGuiController) LOGE("%s() mGuiController use_count: %ld", __func__, mGuiController.use_count());
}

BasicCommunication::~BasicCommunication() {
    if (false == mShutdown.load(std::memory_order_acquire)) {
        shutdown();
    }
}

void BasicCommunication::shutdown(void) {
    if (false == mShutdown.load(std::memory_order_acquire)) {
        mShutdown.store(true, std::memory_order_release);
        mWs->shutdown();
    }
}

void BasicCommunication::onReady(void) {
    Json::Value msg;
    msg["jsonrpc"] = "2.0";
    msg["method"] = "BasicCommunication.OnReady";
    sendJsonMessage(msg);
}

void BasicCommunication::onMessageReceived(MessagePtr msg) {
    utils::JsonReader reader;
    Json::Value root;
    if (!reader.parse(*msg, &root)) {
        LOGE("Invalid JSON Message.");
        return;
    }
    Json::Value error;
    if (checkMessage(root, error)) {
        Json::StreamWriterBuilder builder;
        const std::string str_msg = Json::writeString(builder, root) + '\n';
        if (isNotification(root)) {
            processNotification(root);
        } else if (isResponse(root)) {
            std::string id = root["id"].asString();
            std::string method = findMethodById(id);
            if ("" != method) {
                processResponse(root);
            } else {
                LOGE("Request with id: %s has not been found!", id.c_str());
            }
        } else {
            // request
            std::string method = root["method"].asString();
            std::string component_name = GetComponentName(method);
            LOGD("BasicCommunication::%s() method: %s", __func__, str_msg.c_str());

            if (component_name == "MB") {
                // processInternalRequest(message, this);
                LOGD("BasicCommunication::%s() -----------------> component name start with MB.", __func__);
            } else {
                processRequest(root);
            }
        }
    } else {
        LOGE("BasicCommunication::%s(), Message contains wrong data!", __func__);
        sendJsonMessage(error);
    }
}

// TODO: handle SDL response for respective HMI request, but how to handle when no response
std::string BasicCommunication::findMethodById(std::string id) {
  std::lock_guard<std::mutex> guard(mWaitResponseQueueMutex);
  std::string res = "";
  std::map<std::string, std::string>::iterator it;
  it = mWaitResponseQueue.find(id);
  if (it != mWaitResponseQueue.end()) {
    res = (*it).second;
    mWaitResponseQueue.erase(it);
  }
  return res;
}

// TODO: race condition between send and recv message on mWaitResponseQueue, check with add queue for incomming message
void BasicCommunication::sendJsonMessage(Json::Value& message) {
    std::lock_guard<std::mutex> guard(mWaitResponseQueueMutex);
    if (!isNotification(message) && !isResponse(message)) {
        mWaitResponseQueue.insert(std::map<std::string, std::string>::value_type(
            message["id"].asString(), message["method"].asString()));
    }
    mWs->sendJsonMessage(message);
}

void BasicCommunication::processResponse(Json::Value& root) {
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, root) + '\n';
    // response for registerRPCComponent message
    if (root.isMember(json_keys::kResult) && root[json_keys::kResult].isInt()) {
        int result = root[json_keys::kResult].asInt();
        // startId returned by registerComponent
        mRequestId = mStartId = result;
        // result Value of id multiplied by 10. HMI can treat this as a successful registration
        if (0 == (result % 10)) {
            onReady();
            subscribeNotifications();
        }
    } else if (root.isMember("error")) {
        // Handle respone of a request

        // std::string id = root["id"].asString();
        // std::string method = findMethodById(id);
        LOGD("BasicCommunication::%s(), err: \n%s", __func__, str_msg.c_str());
        // TODO: handle more response
    } else {
        LOGD("BasicCommunication::%s(), response:\n%s", __func__, str_msg.c_str());
    }
}

void BasicCommunication::processRequest(Json::Value& root) {
    bool isReply = true;
    std::string method = root["method"].asString();
    Json::Value response;
    response["id"] = root["id"].asInt();
    response["jsonrpc"] = "2.0";
    response["result"]["method"] = method;
    response["result"]["code"] = 0;
    if (method == "BasicCommunication.IsReady") {
        response["result"]["available"] = true;
    } else if (method == "BasicCommunication.GetSystemInfo") {
        response["result"]["ccpu_version"] = "ccpu_version";
        response["result"]["language"] = "EN-US";
        response["result"]["wersCountryCode"] = "US";
    } else if (method == "BasicCommunication.MixingAudioSupported") {
        response["result"]["attenuatedSupported"] = true;
    } else {
        isReply = false;
    }
    // TODO: handle more request
    if (isReply) sendJsonMessage(response);
}

void BasicCommunication::processNotification(Json::Value& root) {
}

// TODO: error with err code
void BasicCommunication::onError(void) {

}

// void startSession(boost::system::error_code ec)
void BasicCommunication::registerComponent(void) {
    Json::Value msg;
    msg["id"] = std::stoi(mId);
    msg["jsonrpc"] = "2.0";
    msg["method"] = "MB.registerComponent";
    msg["params"]["componentName"] = mName;
    sendJsonMessage(msg);
}
void BasicCommunication::unregisterComponent(void) {
    Json::Value msg;
    msg["id"] = std::stoi(mId);
    msg["jsonrpc"] = "2.0";
    msg["method"] = "MB.unregisterComponent";
    msg["params"]["componentName"] = mName;
    sendJsonMessage(msg);
}

void BasicCommunication::subscribeTo(std::string property) {
    Json::Value msg;
    msg["id"] = generateId();
    msg["jsonrpc"] = "2.0";
    msg["method"] = "MB.subscribeTo";
    msg["params"]["propertyName"] = property;
    sendJsonMessage(msg);
}

void BasicCommunication::unsubscribeFrom(std::string property) {
    // TODO: unscribe notifications
}

void BasicCommunication::subscribeNotifications(void) {
    subscribeTo("BasicCommunication.OnReady");
    subscribeTo("BasicCommunication.OnExitAllApplications");
    subscribeTo("BasicCommunication.OnAppDeactivated");
    subscribeTo("BasicCommunication.OnStartDeviceDiscovery");
    subscribeTo("BasicCommunication.OnUpdateDeviceList");
    subscribeTo("BasicCommunication.OnFindApplications");
    subscribeTo("BasicCommunication.OnAppActivated");
    subscribeTo("BasicCommunication.OnAwakeSDL");
    subscribeTo("BasicCommunication.OnExitApplication");
    subscribeTo("BasicCommunication.OnDeviceChosen");
    subscribeTo("BasicCommunication.OnSystemRequest");
    subscribeTo("BasicCommunication.OnIgnitionCycleOver");
    subscribeTo("BasicCommunication.OnSystemInfoChanged");
    subscribeTo("BasicCommunication.OnEventChanged");
    subscribeTo("BasicCommunication.OnSystemCapabilityUpdated");
}

int BasicCommunication::generateId(void) {
    mRequestId++;
    if (mRequestId >= mStartId + 1000) {
        mRequestId = mStartId;
    }
    return mRequestId;
}

}
