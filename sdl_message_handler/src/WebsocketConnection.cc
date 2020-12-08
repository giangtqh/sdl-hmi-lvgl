#include "WebsocketConnection.h"
#include "SDLTypes.h"

namespace sdlcore_message_handler {

WebsocketConnection::WebsocketConnection() {

}

WebsocketConnection::~WebsocketConnection() {

}

std::string WebsocketConnection::getComponentName(const std::string& method) {
    std::string return_string = "";
    if (method != "") {
        int position = method.find(".");
        if (position != -1) {
        return_string = method.substr(0, position);
        }
    }
    return return_string;
}

bool WebsocketConnection::checkMessage(const Json::Value& root, Json::Value& error) {
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

bool WebsocketConnection::isNotification(const Json::Value& root) {
    bool ret = false;
    if (false == root.isMember("id")) {
        ret = true;
    }
    return ret;
}

bool WebsocketConnection::isResponse(const Json::Value& root) {
    bool ret = false;
    if ((true == root.isMember("result")) || (true == root.isMember("error"))) {
        ret = true;
    }
    return ret;
}

}