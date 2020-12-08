#ifndef UI_SETTINGS_H_
#define UI_SETTINGS_H_

#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "GuiController.h"

#include "lvgl/lvgl.h"

#include "ApplicationInfo.h"
#include "Page.h"

namespace gui {
class HomePage;

class SettingsPage : public Page {
 public:
    SettingsPage(sdlcore_message_handler::GuiController*, HomePage* home);
    virtual ~SettingsPage();

    virtual void init(void) override;
    virtual void shutdown(void) override;
    virtual void initDisplay(void) override;

    void onUpdateDeviceList(const std::map<std::string, std::shared_ptr<sdlcore_message_handler::Device>>& deviceList);

 private:
    sdlcore_message_handler::GuiController* controller_;
    HomePage*  home_page_;
    std::atomic_bool shutdown_;
    std::map<std::string, std::shared_ptr<sdlcore_message_handler::Device>> mDeviceList;
};

}

#endif  // UI_SETTINGS_H_