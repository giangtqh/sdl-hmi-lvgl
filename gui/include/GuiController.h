// For test only

#ifndef _SDL_MESSAGE_HANDLER_GUI_CONTROLLER_
#define _SDL_MESSAGE_HANDLER_GUI_CONTROLLER_

#include "GuiCallbacks.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <thread>

#include "json/json.h"

namespace gui {
    class Telephony;
    class MainPage;
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

    void onDialNumber(uint32_t appId, const std::string& number) override;
    void onIncomingCall(uint32_t appId, const std::string& number) override;
    void onStartCall() override;
    void onEndCall() override;
    void onHangUpCall() override;

    void waitForExit(void);

    void shutdown(void);

    // Handler User event
    void onDeviceChosen(const std::string& devId);
    void onAppChosen(const uint32_t appId);

    // SDLMessageController* getMsgController(void) { return mController; }

 private:
    // TODO: Gui should manage list of registered app/device
    uint32_t mAppId;
    gui::MainPage* page;
    gui::Telephony* mTelephony;
    SDLMessageController* mController;
    std::thread guiThread;
    std::map<uint32_t, std::shared_ptr<ApplicationInfo>> mAppList;
};

}

#endif  // _SDL_MESSAGE_HANDLER_GUI_CONTROLLER_
