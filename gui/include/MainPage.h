#ifndef GUI_MAIN_PAGE_H_
#define GUI_MAIN_PAGE_H_

#include <thread>
#include <atomic>
#include "GuiController.h"

#include "lvgl/lvgl.h"

namespace gui {

class MainPage {
 public:
    MainPage(sdlcore_message_handler::GuiController*);
    virtual ~MainPage();

    void init(void);

    void addGauge(void);
    void addButton(const std::string& btnName, const std::string& devId);
    void addApp(const std::string& btnName, uint32_t appId);

    void shutdown(void);

    void start_lv_task(void);

    void waitForExit(void);

    void saySomething(const std::string& msg);

 private:
    void hal_init(void);
    static int tick_thread(void *data);
    void lv_task_handle(void);
    // example for button event_handler
    static void btn_event_handler(lv_obj_t * obj, lv_event_t event);
    static void btn_app_event_handler(lv_obj_t * obj, lv_event_t event);

    sdlcore_message_handler::GuiController* controller_;
    std::thread lv_thread_;
    std::atomic_bool shutdown_;
    // For test only
    std::string mDevId;
    uint32_t mAppId;
};

}

#endif  // GUI_MAIN_PAGE_H_