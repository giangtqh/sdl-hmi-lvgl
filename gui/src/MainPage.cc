#include "MainPage.h"

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

#include <SDL2/SDL.h>
// #include "lvgl/lvgl.h"
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/mouse.h"

#include <time.h>
#include <stdlib.h>
#include <unistd.h> // sleep

#include "Log.h"
#include <thread>

namespace gui {

// using namespace sdlcore_message_handler;

#define GAUGE_SCALE_ANGLE 280
#define GAUSE_MAX_SPEED 240
#define GAUSE_SCALE_UNIT 10
#define GAUSE_CRITICAL_SPEED 200
#define LV_DEMO_PRINTER_BG_NORMAL (-2 * (LV_VER_RES / 3))
#define LV_DEMO_PRINTER_BG_SMALL (-5 * (LV_VER_RES / 6))
// number of the scale lines
const int GAUGE_SCALE_LINE_NB = (GAUSE_MAX_SPEED / GAUSE_SCALE_UNIT) + 1;
// number of the scale label
const int GAUGE_SCALE_LABEL_NB = (GAUSE_MAX_SPEED / (GAUSE_SCALE_UNIT*2)) + 1;

MainPage::MainPage(sdlcore_message_handler::GuiController* controller)
    : controller_(controller)
    , shutdown_(false)
    , mDevId()
    , mAppId(0) {
}

MainPage::~MainPage() {
    shutdown();
}

void MainPage::init(void) {
    /*Initialize LVGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    hal_init();

    // Use current time as
    // seed for random generator
    srand(time(0));
}

void MainPage::shutdown(void) {
    shutdown_ = true;
    waitForExit();
}

int MainPage::tick_thread(void *data) {
    auto me = static_cast<MainPage*>(data);
    if (me) {
        while (!me->shutdown_) {
            SDL_Delay(5);   /*Sleep for 5 millisecond*/
            lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
        }
    }
    return 0;
}

void MainPage::hal_init(void) {
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

static void gauge_anim(lv_obj_t * gauge, lv_anim_value_t value) {
    lv_gauge_set_value(gauge, 0, value);

    static char buf[64];
    lv_snprintf(buf, sizeof(buf), "%d", value);
    lv_obj_t * label = lv_obj_get_child(gauge, NULL);
    lv_label_set_text(label, buf);
    lv_obj_align(label, gauge, LV_ALIGN_IN_TOP_MID, 0, lv_obj_get_y(label));
}

/**
 * Calculate the current value of an animation applying linear characteristic
 * @param a pointer to an animation
 * @return the current value to set
 */
static lv_anim_value_t anim_path_random(const lv_anim_path_t * path, const lv_anim_t * a) {
    /*Calculate the current step*/
    static int i = 0;
    static int cnt = 100;
    // LOGD("MainPage::%s() NULL", __func__);
    if (i < cnt) {
      ++i;
    } else if (i > cnt) {
      i -= 3;
    } else {
      cnt = (rand() % (GAUSE_CRITICAL_SPEED)) + 30;
    }
    // In this animation callback we can retrieve user_data
    // if (path->user_data) {
    //     auto me = static_cast<MainPage*>(path->user_data);
    //     me->saySomething("called from animation callback\n");
    // } else {
    //     LOGD("MainPage::%s() NULL", __func__);
    // }
    return (lv_anim_value_t)i;
}

void MainPage::addGauge(void) {
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_value_align(&style, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    // lv_style_set_value_ofs_y(&style, LV_STATE_DEFAULT, - LV_DPX(10));
    // lv_style_set_margin_top(&style, LV_STATE_DEFAULT, LV_DPX(30));
    lv_style_set_bg_opa(&style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_pad_inner(&style, LV_STATE_DEFAULT, 13);

    lv_style_set_text_font(&style, LV_STATE_DEFAULT, &lv_font_montserrat_12);  /*Set a smaller font*/

    lv_style_set_scale_end_color(&style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_line_color(&style, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    // lv_style_set_scale_grad_color(&style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_line_width(&style, LV_STATE_DEFAULT, 1);
    lv_style_set_scale_end_line_width(&style, LV_STATE_DEFAULT, 4);
    lv_style_set_scale_end_border_width(&style, LV_STATE_DEFAULT, 4);

    static lv_style_t gauge_major_style;
    lv_style_init(&gauge_major_style);
    lv_style_set_value_align(&gauge_major_style, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    lv_style_set_value_ofs_y(&gauge_major_style, LV_STATE_DEFAULT, - LV_DPX(10));
    lv_style_set_margin_top(&gauge_major_style, LV_STATE_DEFAULT, LV_DPX(30));
    lv_style_set_line_width(&gauge_major_style, LV_STATE_DEFAULT, 4);
    lv_style_set_scale_end_line_width(&gauge_major_style, LV_STATE_DEFAULT, 4);
    lv_style_set_scale_end_border_width(&gauge_major_style, LV_STATE_DEFAULT, 4);
    lv_style_set_line_color(&gauge_major_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    // lv_style_set_scale_grad_color(&gauge_major_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);

    static lv_style_t needle_style;
    lv_style_init(&needle_style);
    lv_style_set_value_align(&needle_style, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    // make the needle smaller/shorter than the outer radius
    lv_style_set_pad_inner(&needle_style, LV_STATE_DEFAULT, 10);
    lv_style_set_line_width(&needle_style, LV_STATE_DEFAULT, 4);
    lv_style_set_line_color(&needle_style, LV_STATE_DEFAULT, LV_COLOR_RED);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_values(&a, 0, GAUSE_MAX_SPEED);

    lv_anim_path_t path;
    lv_anim_path_init(&path);
    lv_anim_path_set_cb(&path, anim_path_random);
    // For anim_path, we have api to set user_data
    lv_anim_path_set_user_data(&path, this);

    lv_anim_set_time(&a, 4000);
    lv_anim_set_playback_time(&a, 1000);
    // lv_anim_set_playback_delay(&a, 500);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
    /*Gauge has a needle but for simplicity its style is not initialized here*/

    /*Create an object with the new style*/
    lv_obj_t * gauge = lv_gauge_create(lv_scr_act(), NULL);
    lv_obj_set_size(gauge, 200, 200);
    lv_obj_set_pos(gauge, 50, 50);
    lv_obj_add_style(gauge, LV_GAUGE_PART_MAIN, &style);
    lv_obj_add_style(gauge,  LV_GAUGE_PART_MAJOR  , &gauge_major_style);
    // lv_obj_align(gauge, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
    lv_gauge_set_scale(gauge, GAUGE_SCALE_ANGLE, GAUGE_SCALE_LINE_NB, GAUGE_SCALE_LABEL_NB);
    lv_gauge_set_range(gauge, 0, GAUSE_MAX_SPEED);
    lv_gauge_set_critical_value(gauge, GAUSE_CRITICAL_SPEED);

    lv_obj_t * label = lv_label_create(gauge, NULL);
    lv_obj_align(label, gauge, LV_ALIGN_CENTER, 0, 60);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_title());

    lv_anim_set_var(&a, gauge);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)gauge_anim);
    lv_anim_set_path(&a, &path);
    lv_anim_start(&a);
}

void MainPage::lv_task_handle(void) {
    /* Periodically call the lv_task handler.
    * It could be done in a timer interrupt or an OS task too.*/
    while (!shutdown_) {
            lv_task_handler();
            usleep(5 * 1000);
    }
}

void MainPage::start_lv_task(void) {
    lv_thread_ = std::thread(&MainPage::lv_task_handle, this);
}

void MainPage::waitForExit(void) {
    if (lv_thread_.joinable()) {
        lv_thread_.join();
    }
}

void MainPage::btn_event_handler(lv_obj_t * obj, lv_event_t event) {
    // access user_data
    if (obj->user_data) {
        auto me = static_cast<MainPage*>(obj->user_data);
        if (event == LV_EVENT_CLICKED) {
            me->controller_->onDeviceChosen(me->mDevId);
        }
    }
}

void MainPage::btn_app_event_handler(lv_obj_t * obj, lv_event_t event) {
    // access user_data
    if (obj->user_data) {
        auto me = static_cast<MainPage*>(obj->user_data);
        if (event == LV_EVENT_CLICKED) {
            me->controller_->onAppChosen(me->mAppId);
        }
    }
}

void MainPage::addButton(const std::string& btnName, const std::string& devId) {
    lv_obj_t * label;
    mDevId.assign(devId);
    lv_obj_t * btn1 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_user_data(btn1, this);
    lv_obj_set_event_cb(btn1, &MainPage::btn_event_handler);
    lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, -40);

    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, btnName.c_str());
}

void MainPage::addApp(const std::string& btnName, uint32_t appId) {
    lv_obj_t * label;
    mAppId = appId;
    lv_obj_t * btn1 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_user_data(btn1, this);
    lv_obj_set_event_cb(btn1, &MainPage::btn_app_event_handler);
    lv_obj_align(btn1, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -40);

    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, btnName.c_str());
}

void MainPage::addList(void) {
    /*Create a list*/
    mListAppSoftButton = lv_list_create(lv_scr_act(), NULL);
    lv_obj_set_size(mListAppSoftButton, 160, 200);
    lv_obj_align(mListAppSoftButton, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
}

void MainPage::addSoftButton(const std::string& name, uint32_t btnId) {
    /*Add buttons to the list*/
    list_btn = lv_list_add_btn(mListAppSoftButton, LV_SYMBOL_FILE, name.c_str());
    lv_obj_set_event_cb(list_btn, soft_btn_event_handler);
    mSoftButtonMap[name] = btnId;
    lv_obj_set_user_data(list_btn, this);
}

void MainPage::soft_btn_event_handler(lv_obj_t * obj, lv_event_t event) {
    if (obj->user_data) {
        if (event == LV_EVENT_CLICKED) {
            printf("Clicked: %s\n", lv_list_get_btn_text(obj));
            std::string btnName(lv_list_get_btn_text(obj));
            auto me = static_cast<MainPage*>(obj->user_data);
            me->controller_->onButtonPress(me->mSoftButtonMap[btnName]);
        }
    }
}

void MainPage::addContactList(void) {
    /*Create a list*/
    mContactListObj = lv_list_create(lv_scr_act(), NULL);
    lv_obj_set_size(mContactListObj, 160, 200);
    lv_obj_align(mContactListObj, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
}

void MainPage::contact_list_event_handler(lv_obj_t * obj, lv_event_t event) {
    if (obj->user_data) {
        if (event == LV_EVENT_CLICKED) {
            auto me = static_cast<MainPage*>(obj->user_data);
            std::string btnName(lv_list_get_btn_text(obj));
            me->controller_->onListItemSelected(me->mContactList[btnName]);
        }
    }
}

void MainPage::addContactItem(const std::string& name, uint32_t btnId) {
    /*Add buttons to the list*/
    lv_obj_t * list_btn = lv_list_add_btn(mContactListObj, LV_SYMBOL_FILE, name.c_str());
    lv_obj_set_event_cb(list_btn, contact_list_event_handler);
    mContactList[name] = btnId;
    lv_obj_set_user_data(list_btn, this);
}


}
