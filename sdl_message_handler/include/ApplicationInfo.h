
#ifndef _SDL_MESSAGE_HANDLER_APPLICATION_INFO_
#define _SDL_MESSAGE_HANDLER_APPLICATION_INFO_

#include <algorithm>
#include <cstdint>
#include <string>
#include <map>
#include <memory>
#include <vector>

#include "SDLTypes.h"

namespace sdlcore_message_handler {

struct Device {
    std::string mId;
    std::string mName;
    std::string mTransportType;
    bool        mIsSDLAllowed;

    Device(std::string id, std::string name, std::string transport_type, bool isSDLAllowed)
        : mId(id)
        , mName(name)
        , mTransportType(transport_type)
        , mIsSDLAllowed(isSDLAllowed) {}
};

struct ApplicationInfo {
    uint32_t mAppId;
    std::string mAppName;
    std::string mHmiDisplayLanguageDesired;
    std::string mIcon;
    std::string mNgnMediaScreenAppName;
    std::string mPolicyAppID;
    std::shared_ptr<Device> mDevice;
    // requestType []
    std::string mPriority;
    std::vector<std::string> mVRSynonyms;
    bool isMediaApplication;

    ApplicationInfo(uint32_t appId, std::string appName, std::shared_ptr<Device> device)
        : mAppId(appId)
        , mAppName(appName)
        , mDevice(device) {}
};

typedef enum
{
    BUTTONUP = 0,
    BUTTONDOWN
}ButtonEventMode;

typedef enum
{
    LONG = 0, // A button was released, after it was pressed for a long time. Actual timing is defined by head unit and may vary
    SHORT =	1 // A button was released, after it was pressed for a short time. Actual timing is defined by head unit and may vary
}ButtonPressMode;

struct showStrings {
    std::string mainField1;
    std::string mainField2;
    std::string mainField3;
    std::string mainField4;

    showStrings(std::string f1, std::string f2, std::string f3, std::string f4)
        : mainField1(f1)
        , mainField2(f2)
        , mainField3(f3)
        , mainField4(f4) {}
};

struct SoftButton {
    uint32_t softButtonID;
    bool isHighlighted;
    std::string systemAction;
    std::string text;
    std::string type;

    SoftButton(uint32_t id, std::string text, std::string type)
        : softButtonID(id)
        , isHighlighted(false)
        , systemAction("DEFAULT_ACTION")
        , text(text)
        , type(type) {}
};

struct ListItem {
    uint32_t    command_id_;
    std::string name_;
    std::string number_;
    ListItem(uint32_t id, const std::string& name, const std::string& number)
        : command_id_(id)
        , name_(name)
        , number_(number) {}
    virtual ~ListItem() {}
};

struct SMSMessage : public ListItem {
    std::string address_;
    std::string body_;
    std::string date_;
    uint32_t read_;     // 0: unread, 1: read
    uint32_t type_;     // 1: address is sender (inbox), 2: address is receiver (sent)

    SMSMessage(uint32_t id, const std::string& address, const std::string& body, const std::string& date, uint32_t read, uint32_t type)
        : ListItem(id, address, body)
        , address_(address)
        , body_(body)
        , date_(date)
        , read_(read)
        , type_(type) {}

    // void setCmdId(uint32_t id) {
    //     cmd_id_ = id;
    // }
};

struct ContactItem : public ListItem {
    std::string name_;
    std::string number_;

    ContactItem(uint32_t id, const std::string& name, const std::string& number)
        : ListItem(id, name, number)
        , name_(name)
        , number_(number) {}
};

struct CallLogItem : public ListItem {
    std::string name_;
    std::string number_;
    std::string date_;
    std::string duration_;
    CallLogType type_;  //

    CallLogItem(uint32_t id, const std::string& name, const std::string& number, const std::string& date, const std::string& duration, CallLogType type)
        : ListItem(id, name, number)
        , name_(name)
        , number_(number)
        , date_(date)
        , duration_(duration)
        , type_(type) {}
};

}
#endif  // _SDL_MESSAGE_HANDLER_APPLICATION_INFO_
