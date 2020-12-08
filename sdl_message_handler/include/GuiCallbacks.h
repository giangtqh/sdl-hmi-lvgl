// For test only

#ifndef _SDL_MESSAGE_HANDLER_GUI_CALLBACKS_
#define _SDL_MESSAGE_HANDLER_GUI_CALLBACKS_

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "json/json.h"
#include "ApplicationInfo.h"

namespace sdlcore_message_handler {
class Device;
struct SoftButton;

class GuiCallbacks {
 public:
    GuiCallbacks() {}
    virtual ~GuiCallbacks() {}

    virtual void onUpdateDeviceList(const std::map<std::string, std::shared_ptr<Device>>& deviceList) = 0;
    virtual void onUpdateAppList(const std::map<uint32_t, std::shared_ptr<ApplicationInfo>>& appList) = 0;

    virtual void onAppRegistered(const std::shared_ptr<ApplicationInfo>& app) = 0;

    virtual void onAppUnRegistered(uint32_t appId) = 0;

    // SDL request HMI to bring a specific app to the front (aka HMI level FULL)
    // DEPRECATED
    virtual void onActivateApp(uint32_t appId) = 0;

    // Close Application request from mobile application
    virtual void onCloseApplication(uint32_t appId) {}

    virtual void onSDLClose(void) {}
    // display an alert message on the HMI
    virtual void onAlert(const Json::Value& message) {}
    // out-going call
    virtual void onDialNumber(uint32_t appId, const std::string number, uint32_t cancelBtnId) {}
    virtual void onIncomingCall(uint32_t appId, const std::string number, uint32_t acceptBtnId, uint32_t denyBtnId) = 0;
    virtual void onStartCall() = 0;
    virtual void onEndCall() = 0;
    virtual void onHangUpCall() = 0;
    virtual void onSMSNotificaton(uint32_t appId, const std::string number, const std::string body) = 0;

    //Notify the HMI that the specified application's interest in receiving notifications for a button has changed.
    virtual void OnButtonSubscription(const std::string& name, bool isSubscribed, uint32_t appId) {}

    // virtual void onUpdateSMSList(const std::map<uint32_t, std::shared_ptr<SMSMessage>>& smsList) {}
    // virtual void onUpdateContactList(const std::map<uint32_t, std::shared_ptr<ContactItem>>& contactList) {}
    // virtual void onUpdateCallLogList(const std::map<uint32_t, std::shared_ptr<CallLogItem>>& callLog) {}

    virtual void onUpdateListData(const ListType type, const std::vector<std::shared_ptr<ListItem>>& data) {}


    /**
     * @brief Main app features
     * @param[in] appId   Application ID
     * @param[in] showString    vector of string (includes mainField1, mainFiled2, mainField3, mainField4)
     * @param[in] softButtons   list of soft buttons
     */
    virtual void onShow(uint32_t appId, std::vector<std::string> showString, const std::vector<std::shared_ptr<SoftButton>>& softButtons) = 0;
};

}

#endif  // _SDL_MESSAGE_HANDLER_GUI_CALLBACKS_
