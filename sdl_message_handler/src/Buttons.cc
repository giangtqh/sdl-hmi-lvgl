#include "Buttons.h"
#include "jsoncpp_reader_wrapper.h"
#include "json_rc_constants.h"
#include "Log.h"
#include "SDLMessageControllerImpl.h"

#include <utility>

namespace sdlcore_message_handler {

Buttons::Buttons(SDLMessageControllerImpl* controller)
    : WebsocketConnection()
    , mShutdown(false)
    , mId("500")
    , mName("Buttons")
    , mRequestId(-1)
    , mStartId(-1)
    , mMsgController(controller) {
    mWs = std::make_shared<WebsocketSession>(mId, mName,
                                             std::bind(&Buttons::onMessageReceived, this, std::placeholders::_1),
                                             std::bind(&Buttons::onError, this));
}

Buttons::~Buttons() {
    if (false == mShutdown.load(std::memory_order_acquire)) {
        shutdown();
    }
}

bool Buttons::start(void) {
    if (WebsocketSession::OK != mWs->open()) {
        LOGE("Buttons::%s() Failed to open Websocket connection", __func__);
        return false;
    }
    registerComponent();
    LOGD("Buttons::%s sucessfully()", __func__);
    return true;
}

void Buttons::shutdown(void) {
    LOGD("Buttons::%s()", __func__);
    if (false == mShutdown.load(std::memory_order_acquire)) {
        mShutdown.store(true, std::memory_order_release);
        unsubscribeNotifications();
        unregisterComponent();
        mWs->shutdown();
    } else {
        LOGD("UI::%s() Already shutdown", __func__);
    }
}

void Buttons::onMessageReceived(MessagePtr msg) {
    utils::JsonReader reader;
    Json::Value root;
    if (!reader.parse(*msg, &root)) {
        LOGE("Invalid JSON Message.");
        return;
    }
    Json::Value error;
    if (checkMessage(root, error)) {
        if (isNotification(root)) {
            processNotification(root);
        } else if (isResponse(root)) {
            processResponse(root);
        } else {
            processRequest(root);
        }
    } else {
        LOGE("Buttons::%s(), Message contains wrong data!", __func__);
        sendJsonMessage(error);
    }
}

void Buttons::sendJsonMessage(const Json::Value& message) {
    if (isResponse(message)) {
        std::string id = message["id"].asString();
        std::lock_guard<std::mutex> guard(mRequestListMutex);
        std::map<std::string, std::string>::iterator it = mRequestList.find(id);
        if (it != mRequestList.end()) {
            mRequestList.erase(it);
        }
    } else if (!isNotification(message)) {
        std::string id = message["id"].asString();
        std::lock_guard<std::mutex> guard(mWaitResponseQueueMutex);
        mWaitResponseQueue.insert(std::map<std::string, std::string>::value_type(id, message["method"].asString()));
    }
    mWs->sendJsonMessage(message);
}

void Buttons::processResponse(const Json::Value& root) {
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, root) + '\n';
    LOGD("Buttons::%s() <= RECV response\n%s", __func__, str_msg.c_str());
    // response for registerRPCComponent message
    std::string id = root[json_keys::kId].asString();
    std::string method = findMethodById(id);
    if ("" != method) {
        if (root.isMember(json_keys::kResult) && root[json_keys::kResult].isInt()) {
            int result = root[json_keys::kResult].asInt();
            // startId returned by registerComponent
            mRequestId = mStartId = result;
            // result Value of id multiplied by 10. HMI can treat this as a successful registration
            if (0 == (result % 10)) {
                subscribeNotifications();
            }
        } else if (root.isMember("error")) {
            // TODO: handle error
        } else {
            // TODO:
        }
    } else {
        LOGE("Buttons::%s() Request with id: %s has not been found!", __func__, id.c_str());
    }
}

std::string Buttons::findMethodById(const std::string& id) {
    std::lock_guard<std::mutex> guard(mWaitResponseQueueMutex);
    std::string res = "";
    std::map<std::string, std::string>::iterator it = mWaitResponseQueue.find(id);
    if (it != mWaitResponseQueue.end()) {
        res = (*it).second;
        mWaitResponseQueue.erase(it);
    }
    return res;
}

void Buttons::processRequest(const Json::Value& root) {
    bool isReply = false;
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, root) + '\n';
    LOGD("Buttons::%s() <= RECV request\n%s", __func__, str_msg.c_str());
    std::string id = root["id"].asString();
    std::string method = root["method"].asString();
    {
        std::lock_guard<std::mutex> guard(mRequestListMutex);
        mRequestList.insert(std::map<std::string, std::string>::value_type(id, method));
    }
    Json::Value response;
    response["id"] = root["id"].asInt();
    response["jsonrpc"] = "2.0";
    response["result"]["method"] = method;
    response["result"]["code"] = 0;
    if (method == "Buttons.GetCapabilities") {
        //TODO: GUI can provide following capabilities?
        response["result"]["capabilities"]["shortPressAvailable"] = true;
        response["result"]["capabilities"]["longPressAvailable"] = true;
        response["result"]["capabilities"]["upDownAvailable"] = true;
        response["result"]["capabilities"]["imageSupported"] = true;
        response["result"]["capabilities"]["textSupported"] = true;
        isReply = true;
    } else if (method == "Buttons.ButtonPress") {
        //not applicable currently
        //Emulate button press event on HMI for the common climate or radio control buttons in vehicle
        isReply = false;
    } else {
        isReply = false;
    }
    if (isReply) sendJsonMessage(response);
}

void Buttons::processNotification(const Json::Value& root) {
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, root) + '\n';
    LOGD("Buttons::%s() <= RECV notification\n%s", __func__, str_msg.c_str());
    std::string method = root["method"].asString();
    if("Buttons.OnButtonSubscription" == method) {
        if (mMsgController) {
            std::string name = root["name"].asString();
            bool isSubscribed = root["isSubscribed"].asBool();
            uint32_t appID = root["appID"].asUInt();
            LOGD("Buttons::%s() Buttons.OnButtonSubscription isSubscribed = %d, appID = %d", __func__, isSubscribed, appID);
            mMsgController->OnButtonSubscription(name, isSubscribed, appID);
        }
    }
}

// TODO: error with err code
void Buttons::onError(void) {
    LOGD("Buttons::%s()", __func__);
    // No error for buttons component

}

void Buttons::registerComponent(void) {
    Json::Value msg;
    msg["id"] = std::stoi(mId);
    msg["jsonrpc"] = "2.0";
    msg["method"] = "MB.registerComponent";
    msg["params"]["componentName"] = mName;
    sendJsonMessage(msg);
}

void Buttons::unregisterComponent(void) {
    Json::Value msg;
    msg["id"] = std::stoi(mId);
    msg["jsonrpc"] = "2.0";
    msg["method"] = "MB.unregisterComponent";
    msg["params"]["componentName"] = mName;
    sendJsonMessage(msg);
}

int Buttons::generateId(void) {
    mRequestId++;
    if (mRequestId >= mStartId + 1000) {
        mRequestId = mStartId;
    }
    return mRequestId;
}

void Buttons::subscribeNotifications(void) {
    subscribeTo("Buttons.OnButtonSubscription");
}
void Buttons::unsubscribeNotifications(void) {
    unsubscribeFrom("Buttons.OnButtonSubscription");
}

void Buttons::subscribeTo(const std::string& property) {
    Json::Value msg;
    msg[json_keys::kId] = generateId();
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.subscribeTo";
    msg[json_keys::kParams]["propertyName"] = property;
    sendJsonMessage(msg);
}

void Buttons::unsubscribeFrom(const std::string& property) {
    Json::Value msg;
    msg[json_keys::kId] = generateId();
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.unsubscribeFrom";
    msg[json_keys::kParams]["propertyName"] = property;
    sendJsonMessage(msg);
}

}
