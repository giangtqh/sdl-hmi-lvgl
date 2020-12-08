// For test only

#ifndef _SDL_MESSAGE_HANDLER_GUI_CONTROLLER_
#define _SDL_MESSAGE_HANDLER_GUI_CONTROLLER_

#include "GuiCallbacks.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "json/json.h"

namespace gui {
    class Telephony;
    class MainPage;
    class HomePage;
}

namespace sdlcore_message_handler {

class ApplicationInfo;
class SDLMessageController;

class GuiController: public GuiCallbacks {
 public:
    GuiController();
    virtual ~GuiController();

    void setMsgController(SDLMessageController* controller);

    void onUpdateDeviceList(const std::map<std::string, std::shared_ptr<Device>>& deviceList) override;
    void onUpdateAppList(const std::map<uint32_t, std::shared_ptr<ApplicationInfo>>& appList) override;
    void onAppRegistered(const std::shared_ptr<ApplicationInfo>& app) override;

    void onAppUnRegistered(uint32_t appId) override;

    // SDL request HMI to bring a specific app to the front (aka HMI level FULL)
    void onActivateApp(uint32_t appId) override;

    void onSDLClose(void) override;

    void onAlert(const Json::Value& message) override;

    void initDisplay(void);

    void onDialNumber(uint32_t appId, const std::string number, uint32_t cancelBtnId) override;
    void onIncomingCall(uint32_t appId, const std::string number, uint32_t acceptBtnId, uint32_t denyBtnId) override;
    void onStartCall() override;
    void onEndCall() override;
    void onHangUpCall() override;
    void onSMSNotificaton(uint32_t appId, const std::string number, const std::string body) override;

    void onShow(uint32_t appId, std::vector<std::string> showString, const std::vector<std::shared_ptr<SoftButton>>& softButtons) override;
    // void onUpdateSMSList(const std::map<uint32_t, std::shared_ptr<SMSMessage>>& smsList) override;
    // void onUpdateContactList(const std::map<uint32_t, std::shared_ptr<ContactItem>>& contactList) override;
    // void onUpdateCallLogList(const std::map<uint32_t, std::shared_ptr<CallLogItem>>& callLog) override;

    void onUpdateListData(const ListType type, const std::vector<std::shared_ptr<ListItem>>& data) override;

    void testList(void);

    void waitForExit(void);

    void shutdown(void);

    // Handler User event
    void onDeviceChosen(const std::string& devId);
    void onAppChosen(const uint32_t appId);

    void onButtonPress(uint32_t btnId);
    void onListItemSelected(uint32_t cmdId);

    // request SDL to send device list
    void updateDeviceList(void);
    void startDeviceDiscovery(void);

 private:
    // TODO: Gui should manage list of registered app/device
    uint32_t mAppId;
    gui::MainPage* page;
    gui::Telephony* mTelephony;
    SDLMessageController* mController;
    std::thread guiThread;
    std::map<uint32_t, std::shared_ptr<ApplicationInfo>> mAppList;

    gui::HomePage* mHomePage;
    std::vector<std::shared_ptr<ListItem>> dummy;
};

}

#endif  // _SDL_MESSAGE_HANDLER_GUI_CONTROLLER_
