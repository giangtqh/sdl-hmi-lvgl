// For test only

#ifndef _SDL_MESSAGE_HANDLER_GUI_CALLBACKS_
#define _SDL_MESSAGE_HANDLER_GUI_CALLBACKS_

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include "json/json.h"

namespace sdlcore_message_handler {
class ApplicationInfo;
class Device;

class GuiCallbacks {
 public:
    GuiCallbacks() {}
    virtual ~GuiCallbacks() {}

    virtual void onUpdateDeviceList(const std::map<std::string, std::shared_ptr<Device>>& deviceList) = 0;
    virtual void onUpdateAppList(const std::map<uint32_t, std::shared_ptr<ApplicationInfo>>& appList) = 0;

    virtual void onAppRegistered(const std::shared_ptr<ApplicationInfo>& app) = 0;

    virtual void onAppUnRegistered(uint32_t appId) = 0;

    // SDL request HMI to bring a specific app to the front (aka HMI level FULL)
    virtual void onActivateApp(uint32_t appId) = 0;

    // Close Application request from mobile application
    virtual void onCloseApplication(uint32_t appId) {}

    virtual void onSDLClose(void) {}
    // display an alert message on the HMI
    virtual void onAlert(const Json::Value& message) {}
    // out-going call
    virtual void onDialNumber(uint32_t appId, const std::string& number) {}
};

}

#endif  // _SDL_MESSAGE_HANDLER_GUI_CALLBACKS_
