#include "BasicCommunication.h"
#include "jsoncpp_reader_wrapper.h"
#include "json_rc_constants.h"
#include "Log.h"
#include "SDLMessageControllerImpl.h"

#include <utility>

namespace sdlcore_message_handler {

BasicCommunication::BasicCommunication(SDLMessageControllerImpl* controller)
    : WebsocketConnection()
    , mShutdown(false)
    , mId("600")
    , mName("BasicCommunication")
    , mRequestId(-1)
    , mStartId(-1)
    , mMsgController(controller)
    /*, mProcessMsgFromSDLThread(std::make_shared<LoopThreadDelegate>(&mMessageFromSDLQueue, this)) */ {
    mWs = std::make_shared<WebsocketSession>(mId, mName,
                                             std::bind(&BasicCommunication::onMessageReceived, this, std::placeholders::_1),
                                             std::bind(&BasicCommunication::onError, this));
}

BasicCommunication::~BasicCommunication() {
    LOGD("BasicCommunication::%s()", __func__);
    if (false == mShutdown.load(std::memory_order_acquire)) {
        shutdown();
    }
}

bool BasicCommunication::start(void) {
    if (WebsocketSession::OK != mWs->open()) {
        LOGE("BasicCommunication::%s() Failed to open Websocket connection", __func__);
        return false;
    }
    registerComponent();
    // mProcessMsgFromSDLThread->startThread();
    return true;
}

void BasicCommunication::shutdown(void) {
    LOGD("BasicCommunication::%s()", __func__);
    if (false == mShutdown.load(std::memory_order_acquire)) {
        mShutdown.store(true, std::memory_order_release);
        unsubscribeNotifications();
        unregisterComponent();
        // if (mProcessMsgFromSDLThread) {
        //     mProcessMsgFromSDLThread->SetShutdown();
        //     mProcessMsgFromSDLThread->stopThread();
        // }
        mWs->shutdown();
    } else {
        LOGD("BasicCommunication::%s() Already shutdown.", __func__);
    }
    LOGD("BasicCommunication::%s() Done!", __func__);
}

void BasicCommunication::onReady(void) {
    Json::Value msg;
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "BasicCommunication.OnReady";
    sendJsonMessage(msg);
}

void BasicCommunication::processMessageFromSDL(MessagePtr msg) {
    // mMessageFromSDLQueue.push(msg);
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
        if (isNotification(root)) {
            processNotification(root);
        } else if (isResponse(root)) {
            processResponse(root);
        } else {
            // request
            std::string method = root[json_keys::kMethod].asString();
            std::string component_name = getComponentName(method);
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

std::string BasicCommunication::findMethodById(const std::string& id) {
    std::lock_guard<std::mutex> guard(mWaitResponseQueueMutex);
    std::string res = "";
    std::map<std::string, std::string>::iterator it = mWaitResponseQueue.find(id);
    if (it != mWaitResponseQueue.end()) {
        res = (*it).second;
        mWaitResponseQueue.erase(it);
    }
    return res;
}

void BasicCommunication::sendJsonMessage(const Json::Value& msg) {
    Json::Value message = msg;
    // TODO: cheat code for SDL.ActivateApp
    if ((message.isMember(json_keys::kMethod)) && ("SDL.ActivateApp" == message[json_keys::kMethod].asString())) {
        message[json_keys::kId] = generateId();
    }
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

void BasicCommunication::processResponse(const Json::Value& root) {
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, root) + '\n';
    LOGD("BasicCommunication::%s() <= RECV response\n%s", __func__, str_msg.c_str());
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
                onReady();
                subscribeNotifications();
            }
        } else if (root.isMember("error")) {
            // TODO: handle error
        } else {
            // TODO:
        }
    } else {
        LOGE("BasicCommunication::%s() Request with id: %s has not been found!", __func__, id.c_str());
    }
}

void BasicCommunication::processRequest(const Json::Value& root) {
    bool isReply = false;
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, root) + '\n';
    LOGD("BasicCommunication::%s() <= RECV request\n%s", __func__, str_msg.c_str());
    std::string id = root[json_keys::kId].asString();
    std::string method = root[json_keys::kMethod].asString();
    {
        std::lock_guard<std::mutex> guard(mRequestListMutex);
        mRequestList.insert(std::map<std::string, std::string>::value_type(id, method));
    }
    Json::Value response;
    response[json_keys::kId] = root[json_keys::kId].asInt();
    response[json_keys::kJsonrpc] = "2.0";
    response[json_keys::kResult][json_keys::kMethod] = method;
    response[json_keys::kResult]["code"] = 0;
    if (method == "BasicCommunication.IsReady") {
        response[json_keys::kResult]["available"] = true;
        isReply = true;
    } else if (method == "BasicCommunication.GetSystemInfo") {
        response[json_keys::kResult]["ccpu_version"] = "12.1.3";
        response[json_keys::kResult]["language"] = "EN-US";
        response[json_keys::kResult]["wersCountryCode"] = "WAEGB";
        isReply = true;
    } else if (method == "BasicCommunication.MixingAudioSupported") {
        response[json_keys::kResult]["attenuatedSupported"] = true;
        isReply = true;
    } else if (method == "BasicCommunication.UpdateAppList") {
        if (mMsgController) {
            mMsgController->onUpdateAppList(root);
        }
        isReply = true;
    } else if (method == "BasicCommunication.UpdateDeviceList") {
        if (mMsgController) {
            mMsgController->onUpdateDeviceList(root);
        }
        sendJsonMessage(response);
        Json::Value msg;
        msg[json_keys::kJsonrpc] = "2.0";
        msg[json_keys::kMethod] = "SDL.OnAllowSDLFunctionality";
        msg[json_keys::kParams]["allowed"] = true;
        msg[json_keys::kParams]["source"] = "GUI";
        sendJsonMessage(msg);
    } else if (method == "BasicCommunication.PolicyUpdate") {
        // TODO: Handle this
        isReply = true;
    } else if (method == "BasicCommunication.ActivateApp") {
        // App ready to show on HMI when UI.setAppIcon() is called, so onActivateApp() will be triggerred by setAppIcon()
        // Request HMI to bring a specific app to the front (aka HMI level FULL)
        // uint32_t appId = root[json_keys::kParams][app_infos::kAppID].asUInt();
        // if (mMsgController) {
        //     mMsgController->onActivateApp(appId);
        // }
        // App can be active from MD by this request, do not allow active app by MD
        isReply = false;
    } else if (method == "BasicCommunication.CloseApplication") {
        uint32_t appId = root[json_keys::kParams][app_infos::kAppID].asUInt();
        if (mMsgController) {
            mMsgController->onCloseApplication(appId);
        }
        isReply = true;
    } else if (method == "BasicCommunication.SystemRequest") {
        isReply = true;
    } else if (method == "BasicCommunication.DialNumber") {
        // uint32_t appId = root[json_keys::kParams][app_infos::kAppID].asUInt();
        // std::string number = root[json_keys::kParams]["number"].asString();
        // if (mMsgController) {
            // TODO: This Dial request initial by phone
            // mMsgController->onDialNumber(appId, number);
        // }
        isReply = true;
    } else {
        isReply = false;
    }
    // TODO: handle more request
    if (isReply) sendJsonMessage(response);
}

void BasicCommunication::processNotification(const Json::Value& root) {
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, root) + '\n';
    LOGD("BasicCommunication::%s() <= RECV notification\n%s", __func__, str_msg.c_str());
    std::string method = root[json_keys::kMethod].asString();
    if (method == "BasicCommunication.OnAppRegistered") {
        if (mMsgController) {
            mMsgController->onAppRegistered(root);
        }
    } else if (method == "BasicCommunication.OnAppUnregistered") {
        if (mMsgController) {
            uint32_t appId = root[json_keys::kParams][app_infos::kAppID].asUInt();
            bool unexpectedDisconnect = root[json_keys::kParams]["unexpectedDisconnect"].asBool();
            mMsgController->onAppUnRegistered(appId, unexpectedDisconnect);
        }
    } else if (method == "BasicCommunication.OnSDLClose") {
        if (mMsgController) {
            mMsgController->onSDLClose();
        }
    }  else if (method == "BasicCommunication.OnSDLPersistenceComplete") {
        // TODO
    } else if (method == "SDL.OnStatusUpdate") {
        // Do nothing
    }
}

// TODO: error with err code
void BasicCommunication::onError(void) {
    shutdown();
}

void BasicCommunication::registerComponent(void) {
    Json::Value msg;
    msg[json_keys::kId] = std::stoi(mId);
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.registerComponent";
    msg[json_keys::kParams]["componentName"] = mName;
    sendJsonMessage(msg);
}
void BasicCommunication::unregisterComponent(void) {
    Json::Value msg;
    msg[json_keys::kId] = std::stoi(mId);
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.unregisterComponent";
    msg[json_keys::kParams]["componentName"] = mName;
    sendJsonMessage(msg);
}

void BasicCommunication::subscribeTo(const std::string& property) {
    Json::Value msg;
    msg[json_keys::kId] = generateId();
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.subscribeTo";
    msg[json_keys::kParams]["propertyName"] = property;
    sendJsonMessage(msg);
}

void BasicCommunication::unsubscribeFrom(const std::string& property) {
    Json::Value msg;
    msg[json_keys::kId] = generateId();
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.unsubscribeFrom";
    msg[json_keys::kParams]["propertyName"] = property;
    sendJsonMessage(msg);
}

void BasicCommunication::subscribeNotifications(void) {
    subscribeTo("BasicCommunication.OnAppRegistered");
    subscribeTo("BasicCommunication.OnAppUnregistered");
    subscribeTo("BasicCommunication.OnPutFile");
    subscribeTo("BasicCommunication.OnSDLPersistenceComplete");
    subscribeTo("BasicCommunication.OnSDLClose");
    subscribeTo("BasicCommunication.OnResumeAudioSource");
    subscribeTo("BasicCommunication.OnSystemCapabilityUpdated");

    // subscribe to SDL notification
    subscribeTo("SDL.OnStatusUpdate");
    subscribeTo("SDL.OnAppPermissionChanged");
    subscribeTo("SDL.OnSDLConsentNeeded");
}

void BasicCommunication::unsubscribeNotifications(void) {
    unsubscribeFrom("BasicCommunication.OnAppRegistered");
    unsubscribeFrom("BasicCommunication.OnAppUnregistered");
    unsubscribeFrom("BasicCommunication.OnPutFile");
    unsubscribeFrom("BasicCommunication.OnSDLPersistenceComplete");
    unsubscribeFrom("BasicCommunication.OnSDLClose");
    unsubscribeFrom("BasicCommunication.OnResumeAudioSource");

    // unsubscribe from SDL notification
    unsubscribeFrom("SDL.OnStatusUpdate");
    unsubscribeFrom("SDL.OnAppPermissionChanged");
    unsubscribeFrom("SDL.OnSDLConsentNeeded");
}

int BasicCommunication::generateId(void) {
    mRequestId++;
    if (mRequestId >= mStartId + 1000) {
        mRequestId = mStartId;
    }
    return mRequestId;
}
#if 0
BasicCommunication::LoopThreadDelegate::LoopThreadDelegate(
    utils::MessageQueue<MessagePtr>* message_queue, BasicCommunication* handler)
    : message_queue_(*message_queue), handler_(*handler), shutdown_(false) {}

void BasicCommunication::LoopThreadDelegate::run() {
    while (!message_queue_.IsShuttingDown() && !shutdown_) {
        DrainQueue();
        message_queue_.wait();
    }
    LOGD("%s() Exited.", __func__);
    DrainQueue();
}

int BasicCommunication::LoopThreadDelegate::startThread(void) {
    return Thread::startThread();
}

int BasicCommunication::LoopThreadDelegate::stopThread(void) {
    return Thread::stopThread();
}

void BasicCommunication::LoopThreadDelegate::DrainQueue() {
    while (!message_queue_.empty()) {
        MessagePtr message_ptr;
        message_queue_.pop(message_ptr);
        if (!shutdown_) {
            boost::system::error_code ec;
            handler_.processMessageFromSDL(message_ptr);
            if (ec) {
                LOGE("%s(), Error: %s", __func__, ec.message().c_str());
            }
        }
    }
}

void BasicCommunication::LoopThreadDelegate::SetShutdown() {
    shutdown_ = true;
    // TODO: clear message_queue_
}
#endif
}
