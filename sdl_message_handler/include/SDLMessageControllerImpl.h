#ifndef _SDL_MESSAGE_CONTROLLER_IMPL_
#define _SDL_MESSAGE_CONTROLLER_IMPL_

#include "json/json.h"
#include "WebsocketConnection.h"

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "GuiCallbacks.h"
#include "ApplicationInfo.h"

namespace sdlcore_message_handler {

class SDLMessageControllerImpl {
 public:
    SDLMessageControllerImpl(GuiCallbacks&);
    virtual ~SDLMessageControllerImpl();

    void shutdown(void);
    void startSession(void);

    // HMI -> SDL
    void activateApplication(uint32_t appId);
    void deactivateApplication(uint32_t appId);
    void exitAllApplications(const std::string& reason);
    void startDeviceDiscovery(void);
    // DEPRECATED: deviceChosen
    void deviceChosen(const Device& device);
    void deviceChosenId(const std::string& deviceId);
    void updateDeviceList(void);

    // SDL -> HMI
    void onAppRegistered(const Json::Value&);
    void onAppUnRegistered(uint32_t appId, bool unexpectedDisconnect);
    void onUpdateDeviceList(const Json::Value&);
    void onActivateApp(uint32_t appId);
    void onShow(const Json::Value&);
    // Close app request by MD
    void onCloseApplication(uint32_t appId);
    void onAlert(const Json::Value&);
    void onAddCommand(const Json::Value&);
    void onDeleteCommand(const Json::Value&);
    void onUpdateAppList(const Json::Value&);
    void setAppIcon(uint32_t appId, const std::string& path);
    void OnButtonSubscription(const std::string& name, bool isSubscribed, uint32_t appId);
    void onSDLClose(void);
    void onButtonPress(const uint32_t customButtonID, const uint32_t appID);
    void onButtonEvent(const std::string& name, const ButtonEventMode mode, const uint32_t customButtonID, const uint32_t appID);
    void onButtonPress(const std::string& name, const ButtonPressMode mode, const uint32_t customButtonID, const uint32_t appID);
    void onListItemSelected(const uint32_t cmdId, const uint32_t appID);

    // Helper methods
    void sendJsonMessage(const Json::Value& msg);
    std::shared_ptr<ApplicationInfo> getAppById(uint32_t id);
    std::shared_ptr<Device> getDeviceById(const std::string& id);

 private:
    std::string getComponentName(const std::string& method);
    void findApplications(const Device& device);
    bool isNotification(const Json::Value& root);
    bool isResponse(const Json::Value& root);
    const ListType stringToListType(const std::string& sType);
    std::string listToTouchType(const ListType& sType);
    const ListType commandIdtoListType(const uint32_t cmdId);

 private:
    std::atomic_bool mShutdown;
    GuiCallbacks& mGuiCallbacks;
    std::map<std::string, std::shared_ptr<WebsocketConnection>> mConnectionList;
    std::mutex mConnectionListMutex;

    std::mutex  mDeviceListMutex;
    std::map<std::string, std::shared_ptr<Device>> mDeviceList;
    std::mutex  mAppListMutex;
    std::map<uint32_t, std::shared_ptr<ApplicationInfo>> mAppList;

    std::vector<std::shared_ptr<SoftButton>> mSoftButtons;
   //  std::map<uint32_t, std::shared_ptr<SMSMessage>> mSMSList;  // map<cmdId, SMSMessage>
   //  std::map<uint32_t, std::shared_ptr<ContactItem>> mContactList;  // map<cmdId, ContactItem>
   //  std::map<uint32_t, std::shared_ptr<CallLogItem>> mCallLogList;  // map<cmdId, ContactItem>

    //std::unordered_map<uint32_t, std::shared_ptr<ListItem>> mListData;  // map<cmdId, ContactItem>
    std::vector<std::shared_ptr<ListItem>> mListData;  // map<cmdId, ContactItem>
    bool mIsFilledListData = false;
};

}

#endif  // _SDL_MESSAGE_CONTROLLER_IMPL_