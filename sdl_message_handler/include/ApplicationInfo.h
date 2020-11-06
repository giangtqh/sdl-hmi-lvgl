
#ifndef _SDL_MESSAGE_HANDLER_APPLICATION_INFO_
#define _SDL_MESSAGE_HANDLER_APPLICATION_INFO_

#include <algorithm>
#include <cstdint>
#include <string>
#include <map>
#include <memory>
#include <vector>

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

}
#endif  // _SDL_MESSAGE_HANDLER_APPLICATION_INFO_