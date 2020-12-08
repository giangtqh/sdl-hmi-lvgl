#include "HomePage.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#if defined(LV_EX_CONF_PATH)
#define __LV_TO_STR_AUX(x) #x
#define __LV_TO_STR(x) __LV_TO_STR_AUX(x)
#include __LV_TO_STR(LV_EX_CONF_PATH)
#undef __LV_TO_STR_AUX
#undef __LV_TO_STR
#elif defined(LV_EX_CONF_INCLUDE_SIMPLE)
#include "lv_ex_conf.h"
#else
#include "lv_ex_conf.h"
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#include <time.h>
#include <stdlib.h>
#include <unistd.h> // sleep

#include "Log.h"
#include <thread>
#include "demo_theme.h"

#include <SDL2/SDL.h>
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/mouse.h"

#include "UIListPage.h"
#include "SettingsPage.h"
#include "GaugeCluster.h"

#ifdef __cplusplus
extern "C" {
#endif
LV_IMG_DECLARE(icon_wifi);
LV_IMG_DECLARE(btn_bg_green);
LV_IMG_DECLARE(btn_bg_red);
LV_IMG_DECLARE(img_setup);
LV_IMG_DECLARE(img_gauge);
LV_IMG_DECLARE(call_icon);
LV_IMG_DECLARE(contact_icon);
LV_IMG_DECLARE(hang_up_call_icon);
#ifdef __cplusplus
} /* extern "C" */
#endif

namespace gui {

    /*********************
 *      DEFINES
 *********************/
/*Bg positions*/
#define LV_DEMO_PRINTER_BG_NONE (-LV_VER_RES)
#define LV_DEMO_PRINTER_BG_FULL 0
#define LV_DEMO_PRINTER_BG_NORMAL (-2 * (LV_VER_RES / 3))
#define LV_DEMO_PRINTER_BG_SMALL (-5 * (LV_VER_RES / 6))

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * bg_top;
static lv_obj_t * bg_bottom;
// static lv_obj_t * scan_img;
// static lv_obj_t * print_cnt_label;
static lv_color_t bg_color_prev;
static lv_color_t bg_color_act;
// static uint16_t print_cnt;
// static uint16_t hue_act;
// static int16_t lightness_act;
// static const char * scan_btn_txt;

HomePage::HomePage(sdlcore_message_handler::GuiController* controller)
    : controller_(controller)
    , shutdown_(false)
    , mAppBox(nullptr)
    , mAppBox_w(LV_HOR_RES_MAX - 100)
    // , mDevId()
    // , mAppId(0)
    {
}

HomePage::~HomePage() {
    shutdown();
}

void HomePage::init(void) {
    // TODO: these stuff should be init by mainPage
    {
        /*Initialize LVGL*/
        lv_init();

        /*Initialize the HAL (display, input devices, tick) for LVGL*/
        hal_init();

        // Use current time as
        // seed for random generator
        srand(time(0));
    }

    bg_color_prev = LV_DEMO_PRINTER_BLUE;
    bg_color_act = LV_DEMO_PRINTER_BLUE;

    // lv_theme_t * th = lv_demo_printer_theme_init(LV_COLOR_BLACK, LV_COLOR_BLACK,
    //         0, &lv_font_montserrat_14, &lv_font_montserrat_22,
    //         &lv_font_montserrat_28, &lv_font_montserrat_32);
    // lv_theme_set_act(th);

    lv_obj_t * scr = lv_obj_create(NULL, NULL);
    lv_scr_load(scr);

    bg_top = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_clean_style_list(bg_top, LV_OBJ_PART_MAIN);
    lv_obj_set_style_local_bg_opa(bg_top, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(bg_top, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_DEMO_PRINTER_BLUE);
    lv_obj_set_size(bg_top, LV_HOR_RES, LV_VER_RES);
    //lv_obj_set_y(bg_top, LV_DEMO_PRINTER_BG_NORMAL);
    initDisplay();

    mSettingsPage = new SettingsPage(controller_, this);
    mSettingsPage->init();
    mListPage = new UIListPage(controller_, this);
    mListPage->init();
    mGaugeClusterPage = new GaugeCluster(controller_, this);
}

void HomePage::initDisplay(void) {
    home_open(0);
    mUIAppMap.clear();
    for (auto item : mAppList) {
        addApp(item.second->mAppName, item.second->mAppId);
    }
    // TODO: // ask SDL to send list of applications and handle updateAppList()
    // send onFindApplications
}

void HomePage::onAppRegistered(const std::shared_ptr<sdlcore_message_handler::ApplicationInfo>& app) {
    mAppList.insert(std::pair<uint32_t, std::shared_ptr<sdlcore_message_handler::ApplicationInfo>>(app->mAppId, app));
    addApp(app->mAppName, app->mAppId);
}

void HomePage::onAppUnRegistered(uint32_t appId) {
    mAppList.erase(appId);
    auto it = mUIAppMap.find(appId);
    if (it != mUIAppMap.end()) {
        // lv_obj_del(it->second); // this works only if we stand at homepage
        mUIAppMap.erase(appId);
        mUIActivatedApp.erase(appId);
        // reload homepage when we stand on other pages than homepage
        demo_anim_out_all(lv_scr_act(), 0);
        home_open(100);
    }
}

void HomePage::onUpdateAppList(const std::map<uint32_t, std::shared_ptr<sdlcore_message_handler::ApplicationInfo>>& appList) {
    // TODO: Show apps
}

void HomePage::onUpdateDeviceList(const std::map<std::string, std::shared_ptr<sdlcore_message_handler::Device>>& deviceList) {
    mSettingsPage->onUpdateDeviceList(deviceList);
}

void HomePage::setListData(const std::vector<std::shared_ptr<sdlcore_message_handler::ListItem>>& data) {
    mListPage->setListData(data);
}

void HomePage::setListType(const sdlcore_message_handler::ListType type) {
    mListPage->setListType(type);
}

void HomePage::shutdown(void) {
    shutdown_ = true;
    if (mSettingsPage) {
        mSettingsPage->shutdown();
        delete mSettingsPage;
    }

    if (mListPage) {
        mListPage->shutdown();
        delete mListPage;
    }

    waitForExit();
}

int HomePage::tick_thread(void *data) {
    auto me = static_cast<HomePage*>(data);
    if (me) {
        while (!me->shutdown_) {
            SDL_Delay(5);   /*Sleep for 5 millisecond*/
            lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
        }
    }
    return 0;
}

void HomePage::hal_init(void) {
    /* Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    monitor_init();

    /*Create a display buffer*/
    static lv_disp_buf_t disp_buf1;
    static lv_color_t buf1_1[LV_HOR_RES_MAX * 120];
    lv_disp_buf_init(&disp_buf1, buf1_1, NULL, LV_HOR_RES_MAX * 120);

    /*Create a display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv); /*Basic initialization*/
    disp_drv.buffer = &disp_buf1;
    disp_drv.flush_cb = monitor_flush;
    lv_disp_drv_register(&disp_drv);

    /* Add the mouse as input device
    * Use the 'mouse' driver which reads the PC's mouse*/
    mouse_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv); /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv.read_cb = mouse_read;
    lv_indev_drv_register(&indev_drv);

    /* Tick init.
    * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about
    * how much time were elapsed Create an SDL thread to do this*/
    SDL_CreateThread(tick_thread, "tick", this);
}

void HomePage::demo_anim_in(lv_obj_t * obj, uint32_t delay) {
    if (obj != bg_top && obj != bg_bottom && obj != lv_scr_act()) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, obj);
        lv_anim_set_time(&a, LV_DEMO_PRINTER_ANIM_TIME);
        lv_anim_set_delay(&a, delay);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_y);
        lv_anim_set_values(&a, lv_obj_get_y(obj) - LV_DEMO_PRINTER_ANIM_Y,
                lv_obj_get_y(obj));
        lv_anim_start(&a);

        lv_obj_fade_in(obj, LV_DEMO_PRINTER_ANIM_TIME - 50, delay);
    }
}

lv_obj_t * HomePage::add_back(lv_event_cb_t event_cb) {
    lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);
    // lv_theme_apply(btn, (lv_theme_style_t)LV_DEMO_PRINTER_THEME_BTN_BACK);
    static lv_style_t style_back;
    lv_style_init(&style_back);
    lv_style_set_value_color(&style_back, LV_STATE_DEFAULT, LV_DEMO_PRINTER_WHITE);
    lv_style_set_value_color(&style_back, LV_STATE_PRESSED, LV_DEMO_PRINTER_LIGHT_GRAY);
    lv_style_set_value_str(&style_back, LV_STATE_DEFAULT, LV_SYMBOL_LEFT);
    lv_style_set_bg_color(&style_back, LV_STATE_DEFAULT, LV_DEMO_PRINTER_BLUE);
    lv_style_set_value_font(&style_back, LV_STATE_DEFAULT, &lv_font_montserrat_20);
    lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_back);
    lv_obj_set_size(btn, 40, 40);
    lv_obj_set_pos(btn, 30, 10);

    lv_obj_set_event_cb(btn, event_cb);
    lv_obj_set_user_data(btn, this);

    return btn;
}

lv_obj_t * HomePage::add_title(const char * txt) {
    lv_obj_t * title = lv_label_create(lv_scr_act(), NULL);
    // lv_theme_apply(title, (lv_theme_style_t)LV_DEMO_PRINTER_THEME_TITLE);
    static lv_style_t style_title;
    lv_style_init(&style_title);
    lv_style_set_text_color(&style_title, LV_STATE_DEFAULT, LV_DEMO_PRINTER_WHITE);
    lv_style_set_text_font(&style_title, LV_STATE_DEFAULT, &lv_font_montserrat_20);

    lv_obj_clean_style_list(title, LV_LABEL_PART_MAIN);
    lv_style_list_t* list = lv_obj_get_style_list(title, LV_LABEL_PART_MAIN);
    _lv_style_list_add_style(list, &style_title);
    lv_obj_add_style(title, LV_BTN_PART_MAIN, &style_title);

    lv_label_set_text(title, txt);
    lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, LV_DEMO_PRINTER_TITLE_PAD);
    return title;
}

lv_obj_t * HomePage::add_icon(lv_obj_t * parent, const void * src_bg, const void * src_icon, const char * txt) {
    lv_obj_t * bg = lv_img_create(parent, NULL);
    lv_obj_set_click(bg, true);
    lv_theme_apply(bg, (lv_theme_style_t)LV_DEMO_PRINTER_THEME_ICON);
    lv_img_set_src(bg, src_bg);
    lv_img_set_antialias(bg, false);

    lv_obj_t * icon = lv_img_create(bg, NULL);
    lv_img_set_src(icon, src_icon);
    lv_obj_set_style_local_image_recolor_opa(icon, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_align(icon, NULL, LV_ALIGN_IN_TOP_RIGHT, -30, 30);

    lv_obj_t * label = lv_label_create(bg, NULL);
    lv_label_set_text(label, txt);
    lv_obj_align(label, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 30, -30);


    return bg;
}
// static, do we need static?
void HomePage::home_open(uint32_t delay) {
    lv_obj_t * icon;
#if 0   // remove toolbar
    lv_obj_t * icon;
    lv_obj_t * cont_toolbar = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(cont_toolbar, LV_HOR_RES_MAX - 100, 80);
    lv_obj_clean_style_list(cont_toolbar, LV_CONT_PART_MAIN);
    lv_obj_set_style_local_bg_color(cont_toolbar, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    // lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_LEFT, 60, 0);
    lv_obj_align(cont_toolbar, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 60, 0);

    icon = lv_img_create(cont_toolbar, NULL);
    lv_img_set_src(icon, &icon_wifi);
    lv_obj_align_origo(icon, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 50);
    demo_anim_in(icon, delay);

    delay += LV_DEMO_PRINTER_ANIM_DELAY;
    lv_obj_t * title = add_title("22 April 2020 15:36");
    // lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, -60, LV_DEMO_PRINTER_TITLE_PAD);
    lv_obj_align(title, NULL, LV_ALIGN_IN_BOTTOM_MID, -60, -10);
    demo_anim_in(title, delay);

    delay += LV_DEMO_PRINTER_ANIM_DELAY;
    icon = lv_img_create(cont_toolbar, NULL);
    lv_img_set_src(icon, &icon_wifi);
    lv_obj_align_origo(icon, NULL, LV_ALIGN_IN_TOP_RIGHT, -20, 50);
    // lv_obj_align(icon, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 50);
    demo_anim_in(icon, delay);
#endif
    // if (mAppBox) lv_obj_del(mAppBox);
    mAppBox = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(mAppBox, mAppBox_w, LV_VER_RES_MAX/2);
    lv_obj_align(mAppBox, NULL, LV_ALIGN_IN_TOP_MID, 0, 100);
    delay += LV_DEMO_PRINTER_ANIM_DELAY;
    demo_anim_in(mAppBox, delay);

    icon = add_icon(mAppBox, &btn_bg_green, &img_gauge, "Gauge");
    lv_obj_align_origo(icon, NULL, LV_ALIGN_IN_LEFT_MID, 1 * (mAppBox_w - 20) / 8 + 10, 0);
    lv_obj_fade_in(icon, LV_DEMO_PRINTER_ANIM_TIME * 2, delay + LV_DEMO_PRINTER_ANIM_TIME + 50);
    lv_obj_set_event_cb(icon, open_gauge_cb);
    lv_obj_set_user_data(icon, this);
    demo_anim_in(mAppBox, delay);

    icon = add_icon(mAppBox, &btn_bg_red, &img_setup, "Settings");
    lv_obj_align_origo(icon, NULL, LV_ALIGN_IN_LEFT_MID, 3 * (mAppBox_w - 20) / 8 + 10, 0);
    lv_obj_fade_in(icon, LV_DEMO_PRINTER_ANIM_TIME * 2, delay + LV_DEMO_PRINTER_ANIM_TIME + 50);
    lv_obj_set_event_cb(icon, open_setup_cb);
    lv_obj_set_user_data(icon, this);
    demo_anim_in(mAppBox, delay);

    // static bool first_run = true;
    // if (first_run)
    //     first_run = false;
    // else {
    //     demo_anim_bg(0, LV_DEMO_PRINTER_BLUE, LV_DEMO_PRINTER_BG_NORMAL);
    // }
}

void HomePage::addApp(const std::string& appName, uint32_t appId) {
    uint32_t delay = 50;
    lv_obj_t * icon = add_icon(mAppBox, &btn_bg_red, &img_setup, appName.substr(0, 13).c_str());
    mUIAppMap.insert(std::pair<uint32_t, lv_obj_t*>(appId, icon));
    // TODO: Change to grid layout
    lv_obj_align_origo(icon, NULL, LV_ALIGN_IN_LEFT_MID, 5 * (mAppBox_w - 20) / 8 + 10, 0);
    lv_obj_fade_in(icon, LV_DEMO_PRINTER_ANIM_TIME * 2, delay + LV_DEMO_PRINTER_ANIM_TIME + 50);
    lv_obj_set_event_cb(icon, on_app_chosen_cb);
    lv_obj_set_user_data(icon, this);
    demo_anim_in(mAppBox, delay);
}

uint32_t HomePage::getAppIdFromLVObj(const lv_obj_t* obj) {
    for (auto item : mUIAppMap) {
        if (item.second == obj) {
            return item.first;
        }
    }
    return 0;
}

uint32_t HomePage::getBtnIdFromLVObj(const lv_obj_t* obj) {
    for (auto item : mUIAppSoftBtnId) {
        if (item.second == obj) {
            return item.first;
        }
    }
    return 0;
}

void HomePage::demo_anim_out_all(lv_obj_t * obj, uint32_t delay) {
    lv_obj_t * child = lv_obj_get_child_back(obj, NULL);
    while (child) {
        if(child != bg_top && child != bg_bottom && child != lv_scr_act()) {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, child);
            lv_anim_set_time(&a, LV_DEMO_PRINTER_ANIM_TIME);
            lv_anim_set_delay(&a, delay);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_y);
            lv_coord_t y_cord = lv_obj_get_y(child);
            // printf("-----> y_cord: %d\n", y_cord);
            if (y_cord < 80) {
                lv_anim_set_values(&a, y_cord, y_cord - LV_DEMO_PRINTER_ANIM_Y);
            } else {
                lv_anim_set_values(&a, y_cord, y_cord + LV_DEMO_PRINTER_ANIM_Y);

                delay += LV_DEMO_PRINTER_ANIM_DELAY;
            }
            lv_anim_set_ready_cb(&a, lv_obj_del_anim_ready_cb);
            lv_anim_start(&a);
        }
        child = lv_obj_get_child_back(obj, child);
    }
}

/**
 * Calculate the current value of an animation applying linear characteristic
 * @param a pointer to an animation
 * @return the current value to set
 */
lv_anim_value_t HomePage::anim_path_triangle(const lv_anim_path_t * path, const lv_anim_t * a) {
    /*Calculate the current step*/
    uint32_t step;
    lv_anim_value_t ret = 0;
    if(a->time == a->act_time) {
        ret = (lv_anim_value_t)a->end;
    }
    else {
        if(a->act_time < a->time / 2) {
            step = ((int32_t)a->act_time * 1024) / (a->time / 2);
            int32_t new_value;
            new_value = (int32_t)step * (LV_DEMO_PRINTER_BG_SMALL - a->start);
            new_value = new_value >> 10;
            new_value += a->start;

            ret = (lv_anim_value_t)new_value;
        } else {
            uint32_t t = a->act_time - a->time / 2;
            step = ((int32_t)t * 1024) / (a->time / 2);
            int32_t new_value;
            new_value = (int32_t)step * (a->end - LV_DEMO_PRINTER_BG_SMALL);
            new_value = new_value >> 10;
            new_value += LV_DEMO_PRINTER_BG_SMALL;

            ret = (lv_anim_value_t)new_value;
        }
    }

    return ret;
}

void HomePage::anim_bg_color_cb(lv_anim_t * a, lv_anim_value_t v) {
    lv_color_t c = lv_color_mix(bg_color_act, bg_color_prev, v);
    lv_obj_set_style_local_bg_color(bg_top, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, c);
}

void HomePage::demo_anim_bg(uint32_t delay, lv_color_t color, int32_t y_new) {
    lv_coord_t y_act = lv_obj_get_y(bg_top);
    lv_color_t act_color = lv_obj_get_style_bg_color(bg_top, LV_OBJ_PART_MAIN);
    if(y_new != LV_DEMO_PRINTER_BG_NORMAL && y_new == y_act && act_color.full == color.full) return;


    lv_anim_t a;
    if((y_new == LV_DEMO_PRINTER_BG_NORMAL && y_new == y_act) ||
            (y_new == LV_DEMO_PRINTER_BG_NORMAL && y_act == LV_DEMO_PRINTER_BG_FULL)) {
        lv_anim_path_t path;
        lv_anim_path_init(&path);
        lv_anim_path_set_cb(&path, anim_path_triangle);

        lv_anim_init(&a);
        lv_anim_set_var(&a, bg_top);
        lv_anim_set_time(&a, LV_DEMO_PRINTER_ANIM_TIME_BG + 200);
        lv_anim_set_delay(&a, delay);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_y);
        lv_anim_set_values(&a, y_act, y_new);
        lv_anim_set_path(&a, &path);
        lv_anim_start(&a);
    } else {
        lv_anim_init(&a);
        lv_anim_set_var(&a, bg_top);
        lv_anim_set_time(&a, LV_DEMO_PRINTER_ANIM_TIME_BG);
        lv_anim_set_delay(&a, delay);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_y);
        lv_anim_set_values(&a, lv_obj_get_y(bg_top), y_new);
        lv_anim_start(&a);
    }

    bg_color_prev = bg_color_act;
    bg_color_act = color;

    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) anim_bg_color_cb);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_time(&a, LV_DEMO_PRINTER_ANIM_TIME_BG);
    lv_anim_set_path(&a, &lv_anim_path_def);
    lv_anim_start(&a);

}

void HomePage::open_app(uint32_t appId, uint32_t delay) {
    LOGD("HomePage::%s() appId %d, delay: %d", __func__, appId, delay);
    lv_obj_t * back = add_back(back_to_home_event_cb);
    demo_anim_in(back, delay);
    auto it = mUIActivatedApp.find(appId);
    if (it != mUIActivatedApp.end()) {
        if (it->second->showString_.size() > 0) {
            std::string textField1 = it->second->showString_[0];
            // lv_obj_t * title = add_title(textField1.c_str());
            //if (mOpenedAppTitle) lv_obj_del(mOpenedAppTitle);
            mOpenedAppTitle = add_title(textField1.c_str());
            demo_anim_in(mOpenedAppTitle, delay);
        } else {
            LOGD("HomePage::%s() showString_ size 0", __func__);
        }
        //if (mOpenedAppBox) lv_obj_del(mOpenedAppBox);
        // TODO: issue delete item when onAppUnregistered, destroy open_app screen
        if (it->second->softButtons_.size() > 0) {
            // lv_obj_t * box = lv_obj_create(lv_scr_act(), NULL);
            mOpenedAppBox = lv_obj_create(lv_scr_act(), NULL);
            lv_obj_set_size(mOpenedAppBox, APP_BOX_WIDTH, APP_BOX_HEIGHT);
            lv_obj_align(mOpenedAppBox, NULL, LV_ALIGN_IN_TOP_MID, 0, 100);

            delay += LV_DEMO_PRINTER_ANIM_DELAY;
            demo_anim_in(mOpenedAppBox, delay);

            lv_obj_t * icon;
            mUIAppSoftBtnId.clear();
            for (uint64_t i = 0; i < (uint64_t)it->second->softButtons_.size(); ++i) {
                icon = add_icon(mOpenedAppBox, &btn_bg_green, &call_icon, it->second->softButtons_[i]->text.c_str());
                mUIAppSoftBtnId.insert(std::pair<uint32_t, lv_obj_t*>(it->second->softButtons_[i]->softButtonID, icon));
                lv_obj_align_origo(icon, NULL, LV_ALIGN_IN_LEFT_MID, (2*i + 1) * APP_BOX_WIDTH / 6, -15);
                lv_obj_set_event_cb(icon, on_soft_button_clicked);
                lv_obj_set_user_data(icon, this);
                lv_obj_fade_in(icon, LV_DEMO_PRINTER_ANIM_TIME * 2, delay + LV_DEMO_PRINTER_ANIM_TIME + 50);
            }

            //if (mOpenedAppTextBox) lv_obj_del(mOpenedAppTextBox);
            mOpenedAppTextBox = lv_obj_create(lv_scr_act(), NULL);
            lv_obj_set_size(mOpenedAppTextBox, APP_BOX_WIDTH, 80);
            lv_obj_align(mOpenedAppTextBox, NULL, LV_ALIGN_IN_BOTTOM_LEFT, LV_HOR_RES / 20,
                    - LV_HOR_RES / 40);
            lv_obj_set_style_local_value_str(mOpenedAppTextBox, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,
                    "Welcome to SDL2W");

            delay += LV_DEMO_PRINTER_ANIM_DELAY;
            demo_anim_in(mOpenedAppTextBox, delay);

            // demo_anim_bg(0, LV_DEMO_PRINTER_BLUE, LV_DEMO_PRINTER_BG_NORMAL);
        } else {
            LOGD("HomePage::%s() softButtons_ size 0", __func__);
        }
    } else {
        LOGD("HomePage::%s() APP Id: %d not found", __func__, appId);
    }
}

void HomePage::back_to_home_event_cb(lv_obj_t *obj, lv_event_t e) {
    if (e == LV_EVENT_CLICKED) {
        demo_anim_out_all(lv_scr_act(), 0);
        // home_open(200);
        if (obj->user_data) {
            auto me = static_cast<HomePage*>(obj->user_data);
            me->initDisplay();
        }
    }
}

void HomePage::on_app_chosen_cb(lv_obj_t *obj, lv_event_t e) {
    if (e == LV_EVENT_CLICKED) {
        demo_anim_out_all(lv_scr_act(), 0);
        if (obj->user_data) {
            auto me = static_cast<HomePage*>(obj->user_data);
            uint32_t appId = me->getAppIdFromLVObj(obj);
            // If app already activated, just show it
            auto it = me->mUIActivatedApp.find(appId);
            if (it != me->mUIActivatedApp.end()) {
                me->open_app(appId, 200);
            } else {
                me->controller_->onAppChosen(appId);
            }
        }
    }
    icon_generic_event_cb(obj, e);
}

void HomePage::on_soft_button_clicked(lv_obj_t *obj, lv_event_t e) {
    if (e == LV_EVENT_CLICKED) {
        // demo_anim_out_all(lv_scr_act(), 0);
        if (obj->user_data) {
            auto me = static_cast<HomePage*>(obj->user_data);
            uint32_t btnId = me->getBtnIdFromLVObj(obj);
            me->controller_->onButtonPress(btnId);
        }
    }
}

void HomePage::icon_generic_event_cb(lv_obj_t *obj, lv_event_t e) {
    if (e == LV_EVENT_PRESSED) {
        lv_obj_t * img = lv_obj_get_child_back(obj, NULL);
        lv_obj_t * txt = lv_obj_get_child(obj, NULL);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_time(&a, 100);

        lv_anim_set_var(&a, img);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_set_values(&a, lv_obj_get_x(img), lv_obj_get_width(obj) - lv_obj_get_width(img) - 35);
        lv_anim_start(&a);

        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_values(&a, lv_obj_get_y(img), 35);
        lv_anim_start(&a);

        lv_anim_set_var(&a, txt);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_set_values(&a, lv_obj_get_x(txt), 35);
        lv_anim_start(&a);

        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_values(&a, lv_obj_get_y(txt), lv_obj_get_height(obj) - lv_obj_get_height(txt) -35);
        lv_anim_start(&a);
    } else if (e == LV_EVENT_PRESS_LOST || e == LV_EVENT_RELEASED) {
        lv_obj_t * img = lv_obj_get_child_back(obj, NULL);
        lv_obj_t * txt = lv_obj_get_child(obj, NULL);
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_time(&a, 100);
        lv_anim_set_var(&a, img);

        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_set_values(&a, lv_obj_get_x(img), lv_obj_get_width(obj) - lv_obj_get_width(img) - 30);
        lv_anim_start(&a);

        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_values(&a, lv_obj_get_y(img), 30);
        lv_anim_start(&a);

        lv_anim_set_var(&a, txt);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_set_values(&a, lv_obj_get_x(txt), 30);
        lv_anim_start(&a);

        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_values(&a, lv_obj_get_y(txt), lv_obj_get_height(obj) - lv_obj_get_height(txt) - 30);
        lv_anim_start(&a);
    }
}

void HomePage::open_gauge_cb(lv_obj_t *obj, lv_event_t e) {
    if (e == LV_EVENT_CLICKED) {
        demo_anim_out_all(lv_scr_act(), 0);
        // demo_anim_bg(0, LV_DEMO_PRINTER_GREEN, LV_DEMO_PRINTER_BG_FULL);
        uint32_t delay = 200;

        if (obj->user_data) {
            auto me = static_cast<HomePage*>(obj->user_data);
            me->mGaugeClusterPage->demoGaugeCluster();
        }
    }

    icon_generic_event_cb(obj, e);
}

void HomePage::open_setup_cb(lv_obj_t *obj, lv_event_t e) {
    if (e == LV_EVENT_CLICKED) {
        demo_anim_out_all(lv_scr_act(), 0);
        // demo_anim_bg(0, LV_DEMO_PRINTER_RED, LV_DEMO_PRINTER_BG_FULL);
        // uint32_t delay = 200;
        if (obj->user_data) {
            auto me = static_cast<HomePage*>(obj->user_data);
            me->controller_->updateDeviceList();
#if 0
            me->info_bottom_create("You have no permission to change the settings.", "BACK", back_to_home_event_cb, delay);
#else            // For test only
            lv_obj_t * back = me->add_back(back_to_home_event_cb);
            me->demo_anim_in(back, 200);
            me->controller_->testList();
#endif
        }
    }

    icon_generic_event_cb(obj, e);
}

void HomePage::info_bottom_create(const char * dsc, const char * btn_txt, lv_event_cb_t btn_event_cb, uint32_t delay) {

    lv_obj_t * txt = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(txt, dsc);
    lv_theme_apply(txt, (lv_theme_style_t)LV_DEMO_PRINTER_THEME_LABEL_WHITE);
    lv_obj_align(txt, NULL, LV_ALIGN_CENTER, 0, 100);

    lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);
    lv_theme_apply(btn, (lv_theme_style_t)LV_DEMO_PRINTER_THEME_BTN_BORDER);
    lv_obj_set_size(btn, LV_DEMO_PRINTER_BTN_W, LV_DEMO_PRINTER_BTN_H);
    lv_obj_align(btn, txt, LV_ALIGN_OUT_BOTTOM_MID, 0, 60);
    lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, btn_txt);
    lv_obj_set_event_cb(btn, btn_event_cb);
    lv_obj_set_user_data(btn, this);

    demo_anim_in(txt, delay);
    delay += LV_DEMO_PRINTER_ANIM_DELAY;

    demo_anim_in(btn, delay);
    delay += LV_DEMO_PRINTER_ANIM_DELAY;

    demo_anim_in(btn, delay);
}

void HomePage::onShow(uint32_t appId, std::vector<std::string> showString, const std::vector<std::shared_ptr<sdlcore_message_handler::SoftButton>>& softButtons) {
    mUIActivatedApp[appId] = std::make_shared<ActivatedAppInfo>(appId, showString, softButtons);
    demo_anim_out_all(lv_scr_act(), 0);
    open_app(appId, 200);
}

void HomePage::lv_task_handle(void) {
    /* Periodically call the lv_task handler.
    * It could be done in a timer interrupt or an OS task too.*/
    while (!shutdown_) {
            lv_task_handler();
            usleep(1 * 1000);
    }
}

void HomePage::start_lv_task(void) {
    lv_thread_ = std::thread(&HomePage::lv_task_handle, this);
}

void HomePage::waitForExit(void) {
    if (lv_thread_.joinable()) {
        lv_thread_.join();
    }
}

}
