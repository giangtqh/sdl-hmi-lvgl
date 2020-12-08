#include "VR.h"
#include "jsoncpp_reader_wrapper.h"
#include "json_rc_constants.h"
#include "Log.h"
#include "SDLMessageControllerImpl.h"

#include <utility>

namespace sdlcore_message_handler {

VR::VR(SDLMessageControllerImpl* controller)
    : WebsocketConnection()
    , mShutdown(false)
    , mId("200")
    , mName("VR")
    , mRequestId(-1)
    , mStartId(-1)
    , mMsgController(controller) {
    mWs = std::make_shared<WebsocketSession>(mId, mName,
                                             std::bind(&VR::onMessageReceived, this, std::placeholders::_1),
                                             std::bind(&VR::onError, this));
}

VR::~VR() {
    LOGD("VR::%s()", __func__);
    if (false == mShutdown.load(std::memory_order_acquire)) {
        shutdown();
    }
}

bool VR::start(void) {
    if (WebsocketSession::OK != mWs->open()) {
        LOGE("VR::%s() Failed to open Websocket connection", __func__);
        return false;
    }
    registerComponent();
    return true;
}

void VR::shutdown(void) {
    LOGD("VR::%s()", __func__);
    if (false == mShutdown.load(std::memory_order_acquire)) {
        mShutdown.store(true, std::memory_order_release);
        unsubscribeNotifications();
        unregisterComponent();
        mWs->shutdown();
    } else {
        LOGD("VR::%s() Already shutdown", __func__);
    }
}

void VR::onMessageReceived(MessagePtr msg) {
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
        LOGE("VR::%s(), Message contains wrong data!", __func__);
        sendJsonMessage(error);
    }
}

std::string VR::findMethodById(const std::string& id) {
    std::lock_guard<std::mutex> guard(mWaitResponseQueueMutex);
    std::string res = "";
    std::map<std::string, std::string>::iterator it = mWaitResponseQueue.find(id);
    if (it != mWaitResponseQueue.end()) {
        res = (*it).second;
        mWaitResponseQueue.erase(it);
    }
    return res;
}

void VR::sendJsonMessage(const Json::Value& message) {
    if (isResponse(message)) {
        std::string id = message[json_keys::kId].asString();
        std::lock_guard<std::mutex> guard(mRequestListMutex);
        std::map<std::string, std::string>::iterator it = mRequestList.find(id);
        if (it != mRequestList.end()) {
            mRequestList.erase(it);
        }
    } else if (!isNotification(message)) {
        std::string id = message[json_keys::kId].asString();
        std::lock_guard<std::mutex> guard(mWaitResponseQueueMutex);
        mWaitResponseQueue.insert(std::map<std::string, std::string>::value_type(id, message[json_keys::kMethod].asString()));
    }
    mWs->sendJsonMessage(message);
}

void VR::processResponse(const Json::Value& root) {
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, root) + '\n';
    LOGD("VR::%s() <= RECV response\n%s", __func__, str_msg.c_str());
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
        LOGE("VR::%s() Request with id: %s has not been found!", __func__, id.c_str());
    }
}

void VR::processRequest(const Json::Value& root) {
    bool isReplyNow = false;
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, root) + '\n';
    LOGD("VR::%s() <= RECV request\n%s", __func__, str_msg.c_str());
    std::string id = root[json_keys::kId].asString();
    std::string method = root[json_keys::kMethod].asString();
    {
        std::lock_guard<std::mutex> guard(mRequestListMutex);
        mRequestList.insert(std::map<std::string, std::string>::value_type(id, method));
    }
    Json::Value response;
    response[json_keys::kId] = root[json_keys::kId].asInt();
    response[json_keys::kJsonrpc] = "2.0";
    response["result"][json_keys::kMethod] = method;
    response["result"]["code"] = 0;
    if (method == "VR.IsReady") {
        response["result"]["available"] = true;
        isReplyNow = true;
    } else if (method == "VR.GetLanguage") {
        response["result"]["language"] = "EN-US";
        isReplyNow = true;
    } else if (method == "VR.GetSupportedLanguages") {
        response["result"]["languages"][0] = "EN-US";
        response["result"]["languages"][1] = "FR-CA";
        response["result"]["languages"][2] = "RU-RU";
        isReplyNow = true;
    } else if (method == "VR.GetCapabilities") {
        onGetCapabilities(root);
        isReplyNow = true;
    } else if (method == "VR.ChangeRegistration") {
        isReplyNow = true;
    } else if (method == "VR.AddCommand") {
        // onAddCommand(root);
        isReplyNow = true;
    } else {
        isReplyNow = false;
    }
    // TODO: handle more request
    if (isReplyNow) sendJsonMessage(response);
}

void VR::processNotification(const Json::Value& root) {
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, root) + '\n';
    LOGD("VR::%s() <= RECV notification\n%s", __func__, str_msg.c_str());
}

// TODO: error with err code
void VR::onError(void) {
    LOGD("VR::%s()", __func__);
    shutdown();
}

void VR::onAddCommand(const Json::Value& root) {
    LOGD("VR::%s()", __func__);
    Json::Value response;
    response[json_keys::kId] = root[json_keys::kId].asInt();
    response[json_keys::kJsonrpc] = "2.0";
    response["result"][json_keys::kMethod] = root[json_keys::kMethod].asString();
    response["result"]["code"] = 0;
    sendJsonMessage(response);
}

void VR::onGetCapabilities(const Json::Value& root) {
    LOGD("VR::%s()", __func__);
    std::string getCapabilitiesResponse = "{\"jsonrpc\":\"2.0\",\"id\":" + root[json_keys::kId].asString() +
        ",\"result\":{\"vrCapabilities\":[\"TEXT\"],\"code\":0,\"method\":\"VR.GetCapabilities\"}}";

    utils::JsonReader reader;
    Json::Value response;
    if (!reader.parse(getCapabilitiesResponse, &response)) {
        LOGE("Invalid JSON Message.");
        return;
    }
    sendJsonMessage(response);
}

void VR::registerComponent(void) {
    Json::Value msg;
    msg[json_keys::kId] = std::stoi(mId);
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.registerComponent";
    msg[json_keys::kParams]["componentName"] = mName;
    sendJsonMessage(msg);
}
void VR::unregisterComponent(void) {
    Json::Value msg;
    msg[json_keys::kId] = std::stoi(mId);
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.unregisterComponent";
    msg[json_keys::kParams]["componentName"] = mName;
    sendJsonMessage(msg);
}

void VR::subscribeTo(const std::string& property) {
    Json::Value msg;
    msg[json_keys::kId] = generateId();
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.subscribeTo";
    msg[json_keys::kParams]["propertyName"] = property;
    sendJsonMessage(msg);
}

void VR::unsubscribeFrom(const std::string& property) {
    Json::Value msg;
    msg[json_keys::kId] = generateId();
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.unsubscribeFrom";
    msg[json_keys::kParams]["propertyName"] = property;
    sendJsonMessage(msg);
}

void VR::subscribeNotifications(void) {
    // subscribeTo("VR.OnRecordStart");
}

void VR::unsubscribeNotifications(void) {
    // unsubscribeFrom("VR.OnRecordStart");
}

int VR::generateId(void) {
    mRequestId++;
    if (mRequestId >= mStartId + 1000) {
        mRequestId = mStartId;
    }
    return mRequestId;
}

}
