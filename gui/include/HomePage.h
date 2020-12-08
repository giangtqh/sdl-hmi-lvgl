#ifndef UI_DEMO_HOME_PAGE_H_
#define UI_DEMO_HOME_PAGE_H_

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

#define LV_USE_DEMO_PRINTER     1

namespace gui {

class SettingsPage;
class UIListPage;
class GaugeCluster;

struct ActivatedAppInfo {
    uint32_t id_;
    std::vector<std::string> showString_;
    std::vector<std::shared_ptr<sdlcore_message_handler::SoftButton>> softButtons_;
    ActivatedAppInfo(uint32_t id, const std::vector<std::string> showStrs, const std::vector<std::shared_ptr<sdlcore_message_handler::SoftButton>> softBtns)
        : id_(id)
        , showString_(showStrs)
        , softButtons_(softBtns) {
    }
};

class HomePage : public Page {
 public:
    HomePage(sdlcore_message_handler::GuiController*);
    virtual ~HomePage();

    virtual void init(void) override;
    virtual void shutdown(void) override;
    virtual void initDisplay(void) override;

    void onAppRegistered(const std::shared_ptr<sdlcore_message_handler::ApplicationInfo>& app);
    void onAppUnRegistered(uint32_t appId);
    void onUpdateAppList(const std::map<uint32_t, std::shared_ptr<sdlcore_message_handler::ApplicationInfo>>& appList);

    void onUpdateDeviceList(const std::map<std::string, std::shared_ptr<sdlcore_message_handler::Device>>& deviceList);

    void setListData(const std::vector<std::shared_ptr<sdlcore_message_handler::ListItem>>& data);

    void setListType(const sdlcore_message_handler::ListType type);

    void start_lv_task(void);

    void waitForExit(void);
    void home_open(uint32_t delay);
    void addApp(const std::string& appName, uint32_t appId);
    void removeApp(void);

    void onShow(uint32_t appId, std::vector<std::string> showString, const std::vector<std::shared_ptr<sdlcore_message_handler::SoftButton>>& softButtons);
    // These 2 functions can be moved to base class
    lv_obj_t * add_title(const char * txt);
    void demo_anim_in(lv_obj_t * obj, uint32_t delay);
    lv_obj_t * add_back(lv_event_cb_t event_cb);
    LV_EVENT_CB_DECLARE(back_to_home_event_cb);
    static void demo_anim_out_all(lv_obj_t * obj, uint32_t delay);
 private:
    void hal_init(void);
    static int tick_thread(void *data);
    void lv_task_handle(void);

    LV_EVENT_CB_DECLARE(on_app_chosen_cb);
    LV_EVENT_CB_DECLARE(on_soft_button_clicked);
    LV_EVENT_CB_DECLARE(open_gauge_cb);
    LV_EVENT_CB_DECLARE(open_setup_cb);
    // LV_EVENT_CB_DECLARE(back_to_home_event_cb);
    LV_EVENT_CB_DECLARE(icon_generic_event_cb);

    static lv_anim_value_t anim_path_triangle(const lv_anim_path_t * path, const lv_anim_t * a);
    static void demo_anim_bg(uint32_t delay, lv_color_t color, int32_t y_new);
    // static void demo_anim_out_all(lv_obj_t * obj, uint32_t delay);

    static void anim_bg_color_cb(lv_anim_t * a, lv_anim_value_t v);

    // lv_obj_t * add_back(lv_event_cb_t event_cb);
    static lv_obj_t * add_icon(lv_obj_t * parent, const void * src_bg, const void * src_icon, const char * txt) ;

    void info_bottom_create(const char * dsc, const char * btn_txt, lv_event_cb_t btn_event_cb, uint32_t delay);
    void open_app(uint32_t appId, uint32_t delay);

    uint32_t getAppIdFromLVObj(const lv_obj_t* obj);
    uint32_t getBtnIdFromLVObj(const lv_obj_t* obj);

    sdlcore_message_handler::GuiController* controller_;
    std::thread lv_thread_;
    std::atomic_bool shutdown_;
    // For test only
    std::map<uint32_t, std::shared_ptr<sdlcore_message_handler::ApplicationInfo>> mAppList;
    lv_obj_t * mAppBox;
    lv_coord_t mAppBox_w;
    std::map<uint32_t, lv_obj_t*> mUIAppMap;
    std::map<uint32_t, lv_obj_t*> mUIAppSoftBtnId;
    std::map<uint32_t, std::shared_ptr<ActivatedAppInfo>> mUIActivatedApp;

    lv_obj_t * mOpenedAppTitle;
    lv_obj_t * mOpenedAppBox;
    lv_obj_t * mOpenedAppTextBox;

    gui::SettingsPage* mSettingsPage;
    gui::UIListPage* mListPage;
    gui::GaugeCluster* mGaugeClusterPage;
};

}

#endif  // UI_DEMO_HOME_PAGE_H_