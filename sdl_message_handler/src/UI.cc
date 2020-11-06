#include "UI.h"
#include "jsoncpp_reader_wrapper.h"
#include "json_rc_constants.h"
#include "Log.h"
// #include "SdlcoreTypes.h"
#include "SDLMessageControllerImpl.h"

#include <utility>

namespace sdlcore_message_handler {

UI::UI(SDLMessageControllerImpl* controller)
    : WebsocketConnection()
    , mShutdown(false)
    , mId("400")
    , mName("UI")
    , mRequestId(-1)
    , mStartId(-1)
    , mMsgController(controller) {
    mWs = std::make_shared<WebsocketSession>(mId, mName,
                                             std::bind(&UI::onMessageReceived, this, std::placeholders::_1),
                                             std::bind(&UI::onError, this));
}

UI::~UI() {
    LOGD("UI::%s()", __func__);
    if (false == mShutdown.load(std::memory_order_acquire)) {
        shutdown();
    }
}

bool UI::start(void) {
    if (WebsocketSession::OK != mWs->open()) {
        LOGE("UI::%s() Failed to open Websocket connection", __func__);
        return false;
    }
    registerComponent();
    return true;
}

void UI::shutdown(void) {
    LOGD("UI::%s()", __func__);
    if (false == mShutdown.load(std::memory_order_acquire)) {
        mShutdown.store(true, std::memory_order_release);
        unsubscribeNotifications();
        unregisterComponent();
        mWs->shutdown();
    } else {
        LOGD("UI::%s() Already shutdown", __func__);
    }
}

void UI::onMessageReceived(MessagePtr msg) {
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
        LOGE("UI::%s(), Message contains wrong data!", __func__);
        sendJsonMessage(error);
    }
}

std::string UI::findMethodById(const std::string& id) {
    std::lock_guard<std::mutex> guard(mWaitResponseQueueMutex);
    std::string res = "";
    std::map<std::string, std::string>::iterator it = mWaitResponseQueue.find(id);
    if (it != mWaitResponseQueue.end()) {
        res = (*it).second;
        mWaitResponseQueue.erase(it);
    }
    return res;
}

void UI::sendJsonMessage(const Json::Value& message) {
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

void UI::processResponse(const Json::Value& root) {
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, root) + '\n';
    LOGD("UI::%s() <= RECV response\n%s", __func__, str_msg.c_str());
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
        LOGE("UI::%s() Request with id: %s has not been found!", __func__, id.c_str());
    }
}

void UI::processRequest(const Json::Value& root) {
    bool isReply = false;
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, root) + '\n';
    LOGD("UI::%s() <= RECV request\n%s", __func__, str_msg.c_str());
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
    if (method == "UI.IsReady") {
        response["result"]["available"] = true;
        isReply = true;
    } else if (method == "UI.GetLanguage") {
        response["result"]["language"] = "EN-US";
        isReply = true;
    } else if (method == "UI.GetSupportedLanguages") {
        response["result"]["languages"][0] = "EN-US";
        response["result"]["languages"][1] = "FR-CA";
        response["result"]["languages"][2] = "RU-RU";
        isReply = true;
    } else if (method == "UI.GetCapabilities") {
        onGetCapabilities(root);
    } else if (method == "UI.ChangeRegistration") {
        isReply = true;
    } else if (method == "UI.Show") {
        onShow(root);
    } else if (method == "UI.AddCommand") {
        // onAddCommand(root);
        isReply = true;
    } else if (method == "UI.Alert") {
        if (mMsgController) {
            mMsgController->onAlert(root);
        }
        isReply = true;
    } else if (method == "UI.ClosePopUp") {
        // TODO: Close the currently displayed popup UI element
        if (mMsgController) {
            // mMsgController->onClosePopUp(root);
        }
        isReply = true;
    } else if (method == "UI.PerformInteraction") {
        // TODO: A popup request User to perform a UI Interaction (choice set), should response with chosen value
        if (mMsgController) {
            // uint32_t choiceId = mMsgController->onPerformInteraction(root);
            // response["result"]["choiceID"] = choiceId;
        }
        // isReply = true;
    } else if (method == "UI.SetAppIcon") {
        if (mMsgController) {
            uint32_t appId = root[json_keys::kParams][app_infos::kAppID].asUInt();
            std::string path = root[json_keys::kParams]["syncFileName"]["value"].asString();
            mMsgController->setAppIcon(appId, path);
        }
        isReply = true;
    } else {
        isReply = false;
    }
    // TODO: handle more request
    if (isReply) sendJsonMessage(response);
}

void UI::processNotification(const Json::Value& root) {
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, root) + '\n';
    LOGD("UI::%s() <= RECV notification\n%s", __func__, str_msg.c_str());
}

// TODO: error with err code
void UI::onError(void) {
    LOGD("UI::%s()", __func__);
    shutdown();
}

void UI::onShow(const Json::Value& root) {
    LOGD("UI::%s()", __func__);
    // TODO: need to handle for specific app
    Json::Value response;
    response[json_keys::kId] = root[json_keys::kId].asInt();
    response[json_keys::kJsonrpc] = "2.0";
    response["result"][json_keys::kMethod] = root[json_keys::kMethod].asString();
    response["result"]["code"] = 0;
    sendJsonMessage(response);
}

void UI::onAddCommand(const Json::Value& root) {
    LOGD("UI::%s()", __func__);
    Json::Value response;
    response[json_keys::kId] = root[json_keys::kId].asInt();
    response[json_keys::kJsonrpc] = "2.0";
    response["result"][json_keys::kMethod] = root[json_keys::kMethod].asString();
    response["result"]["code"] = 0;
    sendJsonMessage(response);
}

void UI::onGetCapabilities(const Json::Value& root) {
    LOGD("UI::%s()", __func__);
    std::string getCapabilitiesResponse = "{\"jsonrpc\":\"2.0\",\"id\":" + root[json_keys::kId].asString() + ",\"result\":{\"displayCapabilities\":{\"displayType\":\"GEN2_8_DMA\",\"textFields\":[{\"name\":\"mainField1\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"mainField2\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"mainField3\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"mainField4\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"statusBar\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"mediaClock\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"mediaTrack\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"alertText1\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"alertText2\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"alertText3\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"scrollableMessageBody\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"initialInteractionText\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"navigationText1\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"navigationText2\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"ETA\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"totalDistance\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"navigationText\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"audioPassThruDisplayText1\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"audioPassThruDisplayText2\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"sliderHeader\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"sliderFooter\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"notificationText\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"menuName\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"secondaryText\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"tertiaryText\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"timeToDestination\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"turnText\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"menuTitle\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"locationName\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"locationDescription\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"addressLines\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1},{\"name\":\"phoneNumber\",\"characterSet\":\"TYPE2SET\",\"width\":500,\"rows\":1}],\"imageFields\":[{\"name\":\"softButtonImage\",\"imageTypeSupported\":[\"GRAPHIC_BMP\",\"GRAPHIC_JPEG\",\"GRAPHIC_PNG\"],\"imageResolution\":{\"resolutionWidth\":64,\"resolutionHeight\":64}},{\"name\":\"choiceImage\",\"imageTypeSupported\":[\"GRAPHIC_BMP\",\"GRAPHIC_JPEG\",\"GRAPHIC_PNG\"],\"imageResolution\":{\"resolutionWidth\":64,\"resolutionHeight\":64}},{\"name\":\"choiceSecondaryImage\",\"imageTypeSupported\":[\"GRAPHIC_BMP\",\"GRAPHIC_JPEG\",\"GRAPHIC_PNG\"],\"imageResolution\":{\"resolutionWidth\":64,\"resolutionHeight\":64}},{\"name\":\"vrHelpItem\",\"imageTypeSupported\":[\"GRAPHIC_BMP\",\"GRAPHIC_JPEG\",\"GRAPHIC_PNG\"],\"imageResolution\":{\"resolutionWidth\":64,\"resolutionHeight\":64}},{\"name\":\"turnIcon\",\"imageTypeSupported\":[\"GRAPHIC_BMP\",\"GRAPHIC_JPEG\",\"GRAPHIC_PNG\"],\"imageResolution\":{\"resolutionWidth\":64,\"resolutionHeight\":64}},{\"name\":\"menuIcon\",\"imageTypeSupported\":[\"GRAPHIC_BMP\",\"GRAPHIC_JPEG\",\"GRAPHIC_PNG\"],\"imageResolution\":{\"resolutionWidth\":64,\"resolutionHeight\":64}},{\"name\":\"cmdIcon\",\"imageTypeSupported\":[\"GRAPHIC_BMP\",\"GRAPHIC_JPEG\",\"GRAPHIC_PNG\"],\"imageResolution\":{\"resolutionWidth\":64,\"resolutionHeight\":64}},{\"name\":\"graphic\",\"imageTypeSupported\":[\"GRAPHIC_BMP\",\"GRAPHIC_JPEG\",\"GRAPHIC_PNG\"],\"imageResolution\":{\"resolutionWidth\":64,\"resolutionHeight\":64}},{\"name\":\"showConstantTBTIcon\",\"imageTypeSupported\":[\"GRAPHIC_BMP\",\"GRAPHIC_JPEG\",\"GRAPHIC_PNG\"],\"imageResolution\":{\"resolutionWidth\":64,\"resolutionHeight\":64}},{\"name\":\"showConstantTBTNextTurnIcon\",\"imageTypeSupported\":[\"GRAPHIC_BMP\",\"GRAPHIC_JPEG\",\"GRAPHIC_PNG\"],\"imageResolution\":{\"resolutionWidth\":64,\"resolutionHeight\":64}},{\"name\":\"showConstantTBTNextTurnIcon\",\"imageTypeSupported\":[\"GRAPHIC_BMP\",\"GRAPHIC_JPEG\",\"GRAPHIC_PNG\"],\"imageResolution\":{\"resolutionWidth\":64,\"resolutionHeight\":64}}],\"mediaClockFormats\":[\"CLOCK1\",\"CLOCK2\",\"CLOCK3\",\"CLOCKTEXT1\",\"CLOCKTEXT2\",\"CLOCKTEXT3\",\"CLOCKTEXT4\"],\"graphicSupported\":true,\"imageCapabilities\":[\"DYNAMIC\",\"STATIC\"],\"templatesAvailable\":[\"TEMPLATE\"],\"screenParams\":{\"resolution\":{\"resolutionWidth\":800,\"resolutionHeight\":480},\"touchEventAvailable\":{\"pressAvailable\":true,\"multiTouchAvailable\":true,\"doublePressAvailable\":false}},\"numCustomPresetsAvailable\":10},\"audioPassThruCapabilities\":{\"samplingRate\":\"44KHZ\",\"bitsPerSample\":\"8_BIT\",\"audioType\":\"PCM\"},\"hmiZoneCapabilities\":\"FRONT\",\"softButtonCapabilities\":[{\"shortPressAvailable\":true,\"longPressAvailable\":true,\"upDownAvailable\":true,\"imageSupported\":true}],\"hmiCapabilities\":{\"navigation\":true,\"phoneCall\":true,\"steeringWheelLocation\":\"LEFT\"},\"code\":0,\"method\":\"UI.GetCapabilities\"}}";

    utils::JsonReader reader;
    Json::Value response;
    if (!reader.parse(getCapabilitiesResponse, &response)) {
        LOGE("Invalid JSON Message.");
        return;
    }
    sendJsonMessage(response);
}

void UI::registerComponent(void) {
    Json::Value msg;
    msg[json_keys::kId] = std::stoi(mId);
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.registerComponent";
    msg[json_keys::kParams]["componentName"] = mName;
    sendJsonMessage(msg);
}
void UI::unregisterComponent(void) {
    Json::Value msg;
    msg[json_keys::kId] = std::stoi(mId);
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.unregisterComponent";
    msg[json_keys::kParams]["componentName"] = mName;
    sendJsonMessage(msg);
}

void UI::subscribeTo(const std::string& property) {
    Json::Value msg;
    msg[json_keys::kId] = generateId();
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.subscribeTo";
    msg[json_keys::kParams]["propertyName"] = property;
    sendJsonMessage(msg);
}

void UI::unsubscribeFrom(const std::string& property) {
    Json::Value msg;
    msg[json_keys::kId] = generateId();
    msg[json_keys::kJsonrpc] = "2.0";
    msg[json_keys::kMethod] = "MB.unsubscribeFrom";
    msg[json_keys::kParams]["propertyName"] = property;
    sendJsonMessage(msg);
}

void UI::subscribeNotifications(void) {
    subscribeTo("UI.OnRecordStart");
}

void UI::unsubscribeNotifications(void) {
    unsubscribeFrom("UI.OnRecordStart");
}

int UI::generateId(void) {
    mRequestId++;
    if (mRequestId >= mStartId + 1000) {
        mRequestId = mStartId;
    }
    return mRequestId;
}

}
