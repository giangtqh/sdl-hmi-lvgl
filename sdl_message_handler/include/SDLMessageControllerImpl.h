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
    void deviceChosen(const Device& device);
    void findApplications(const Device& device);
    void updateDeviceList(void);

    // SDL -> HMI
    void onAppRegistered(const Json::Value&);
    void onAppUnRegistered(uint32_t appId, bool unexpectedDisconnect);
    void onUpdateDeviceList(const Json::Value&);
    void onActivateApp(uint32_t appId);
    // Close app request by MD
    void onCloseApplication(uint32_t appId);
    void onAlert(const Json::Value&);
    void onUpdateAppList(const Json::Value&);
    void setAppIcon(uint32_t appId, const std::string& path);
    void onSDLClose(void);
    void onDialNumber(uint32_t appId, const std::string& number);

    // Helper methods
    void sendJsonMessage(const Json::Value& msg);
    std::shared_ptr<ApplicationInfo> getAppById(uint32_t id);
    std::shared_ptr<Device> getDeviceById(const std::string& id);

 private:
    std::string getComponentName(const std::string& method);

 private:
    std::atomic_bool mShutdown;
    GuiCallbacks& mGuiCallbacks;
    std::map<std::string, std::shared_ptr<WebsocketConnection>> mConnectionList;
    std::mutex mConnectionListMutex;

    std::mutex  mDeviceListMutex;
    std::map<std::string, std::shared_ptr<Device>> mDeviceList;
    std::mutex  mAppListMutex;
    std::map<uint32_t, std::shared_ptr<ApplicationInfo>> mAppList;
};

}

#endif  // _SDL_MESSAGE_CONTROLLER_IMPL_