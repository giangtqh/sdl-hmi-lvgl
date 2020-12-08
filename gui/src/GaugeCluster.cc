#include "GaugeCluster.h"
#include <time.h>
#include "demo_theme.h"

namespace gui {

#ifdef __cplusplus
extern "C" {
#endif
    LV_IMG_DECLARE(fuel_icon);
    LV_IMG_DECLARE(headlight_icon);
    LV_IMG_DECLARE(engine_trouble_icon);
    LV_IMG_DECLARE(oil_lower_icon);
    LV_IMG_DECLARE(coolant_temp_icon);
    LV_IMG_DECLARE(abs_icon);
    LV_IMG_DECLARE(signal_icon);
    LV_IMG_DECLARE(gauge_bg_rpm);
    LV_IMG_DECLARE(gauge_bg_speed);
    LV_IMG_DECLARE(fuel_level_mask);
    LV_IMG_DECLARE(fuel_icon);
#ifdef __cplusplus
} /* extern "C" */
#endif

lv_style_t style_main;
lv_style_t style_cont_body;
lv_style_t style_label_led_neutral;
lv_obj_t* cont_main;
lv_obj_t* cont_header;
lv_obj_t* cont_body;
lv_obj_t* led_signal;
lv_task_t* taskTurnOnSignal;
lv_obj_t* led_fuel_empty;
lv_obj_t* led_headlight;
lv_obj_t* led_engine_trouble;
lv_obj_t* led_oil_lower;
lv_obj_t* led_coolant_temp;
lv_obj_t* led_neutral;
lv_obj_t* led_abs;
lv_obj_t* label_time;
lv_style_t style_label_time;
lv_task_t* taskUpdateTime;
time_t t;
struct tm tm;
lv_obj_t* gauge_rpm;
lv_obj_t* gauge_speed;
lv_obj_t* label_gear_num;
lv_obj_t* label_record_trip;
lv_obj_t* label_record_fuel_consumption;
lv_obj_t* bar_fuel_level;

lv_task_t* taskTest;

GaugeCluster::GaugeCluster(sdlcore_message_handler::GuiController* controller, HomePage* home)
    : controller_(controller),
    home_page_(home)
{
}

GaugeCluster::~GaugeCluster()
{
}

void GaugeCluster::specific_back_to_home_event_cb(lv_obj_t *obj, lv_event_t e)
{
    if (e == LV_EVENT_CLICKED) {
        if (obj->user_data) {
            auto pHomePage = static_cast<HomePage*>(obj->user_data);
            pHomePage->demo_anim_out_all(lv_scr_act(), 0);
            lv_obj_move_background(cont_main);
            if (taskTurnOnSignal) {
                lv_task_del(taskTurnOnSignal);
                taskTurnOnSignal = NULL;
            }
            if (taskTest){
                lv_task_del(taskTest);
                taskTest = NULL;
            }
            if (cont_main) {
                lv_obj_del(cont_main);
                cont_main = NULL;
            }
            pHomePage->initDisplay();
        }
    }
}

void GaugeCluster::initGaugeCluster()
{
    lv_style_init(&style_main);
    lv_style_set_radius(&style_main, LV_STATE_DEFAULT, 0);
    lv_style_set_bg_opa(&style_main, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&style_main, LV_STATE_DEFAULT, lv_color_hex(0x2f3243));
    lv_style_set_border_width(&style_main, LV_STATE_DEFAULT, 0);
    // lv_style_set_border_width(&style_main, LV_STATE_DEFAULT, 1); // for debbuging

    cont_main = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(cont_main, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_add_style(cont_main, LV_OBJ_PART_MAIN, &style_main);
    lv_obj_set_pos(cont_main, 0, 60);

    cont_header = lv_obj_create(cont_main, NULL);
    lv_obj_set_size(cont_header, LV_HOR_RES_MAX, 60);
    lv_obj_add_style(cont_header, LV_OBJ_PART_MAIN, &style_main);
    lv_obj_align(cont_header, NULL, LV_ALIGN_CENTER, 0, -205);

    cont_body = lv_obj_create(cont_main, NULL);
    lv_obj_set_size(cont_body, LV_HOR_RES_MAX, 340);
    lv_style_init(&style_cont_body);
    lv_style_copy(&style_cont_body, &style_main);
    lv_style_set_radius(&style_cont_body, LV_STATE_DEFAULT, 100);
    lv_style_set_border_color(&style_cont_body, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_border_width(&style_cont_body, LV_STATE_DEFAULT, 2);
    lv_style_set_border_opa(&style_cont_body, LV_STATE_DEFAULT, LV_OPA_20);
    lv_style_set_border_side(&style_cont_body, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);
    lv_obj_add_style(cont_body, LV_OBJ_PART_MAIN, &style_cont_body);
    lv_obj_align(cont_body, NULL, LV_ALIGN_CENTER, 0, 0);
}

void GaugeCluster::drawHeader(lv_obj_t* parent)
{
    const int16_t size_w_h_led = 45;

    led_signal  = lv_led_create(parent, NULL);
    lv_obj_set_size(led_signal, size_w_h_led, size_w_h_led);
    lv_obj_align(led_signal, NULL, LV_ALIGN_CENTER, -250, 0);
    lv_led_off(led_signal);
    lv_obj_set_style_local_bg_color(led_signal, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x84f745));
    lv_obj_t* icon_signal = lv_img_create(led_signal, NULL);
    lv_img_set_src(icon_signal, &signal_icon);
    lv_obj_align(icon_signal, NULL, LV_ALIGN_CENTER, 0, 0);

    led_neutral  = lv_led_create(parent, NULL);
    lv_obj_set_size(led_neutral, size_w_h_led, size_w_h_led);
    lv_obj_align(led_neutral, NULL, LV_ALIGN_CENTER, -180, 0);
    lv_led_off(led_neutral);
    lv_obj_set_style_local_bg_color(led_neutral, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x84f745));
    lv_obj_t* label_led_neutral = lv_label_create(led_neutral, NULL);
    lv_label_set_long_mode(label_led_neutral, LV_LABEL_LONG_BREAK);
    lv_label_set_recolor(label_led_neutral, true);
    lv_label_set_align(label_led_neutral, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(label_led_neutral, "#FFFFFF N#");
    lv_obj_set_width(label_led_neutral, 150);
    lv_obj_align(label_led_neutral, NULL, LV_ALIGN_CENTER, 0, -10);
    lv_style_init(&style_label_led_neutral);
    lv_style_set_text_font(&style_label_led_neutral, LV_STATE_DEFAULT, &lv_font_montserrat_32);
    lv_obj_add_style(label_led_neutral, LV_OBJ_PART_MAIN, &style_label_led_neutral);

    led_fuel_empty  = lv_led_create(parent, NULL);
    lv_obj_set_size(led_fuel_empty, size_w_h_led, size_w_h_led);
    lv_obj_align(led_fuel_empty, NULL, LV_ALIGN_CENTER, -110, 0);
    lv_led_off(led_fuel_empty);
    lv_obj_set_style_local_bg_color(led_fuel_empty, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_obj_t* icon_fuel_empty = lv_img_create(led_fuel_empty, NULL);
    lv_img_set_src(icon_fuel_empty, &fuel_icon);
    lv_obj_align(icon_fuel_empty, NULL, LV_ALIGN_CENTER, 0, 0);

    led_headlight  = lv_led_create(parent, NULL);
    lv_obj_set_size(led_headlight, size_w_h_led, size_w_h_led);
    lv_obj_align(led_headlight, NULL, LV_ALIGN_CENTER, -40, 0);
    lv_led_off(led_headlight);
    lv_obj_set_style_local_bg_color(led_headlight, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x84f745));
    lv_obj_t* icon_headlight = lv_img_create(led_headlight, NULL);
    lv_img_set_src(icon_headlight, &headlight_icon);
    lv_obj_align(icon_headlight, NULL, LV_ALIGN_CENTER, 0, 0);

    led_engine_trouble  = lv_led_create(parent, NULL);
    lv_obj_set_size(led_engine_trouble, size_w_h_led, size_w_h_led);
    lv_obj_align(led_engine_trouble, NULL, LV_ALIGN_CENTER, 30, 0);
    lv_led_off(led_engine_trouble);
    lv_obj_set_style_local_bg_color(led_engine_trouble, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_obj_t* icon_engine_trouble = lv_img_create(led_engine_trouble, NULL);
    lv_img_set_src(icon_engine_trouble, &engine_trouble_icon);
    lv_obj_align(icon_engine_trouble, NULL, LV_ALIGN_CENTER, 0, 0);

    led_oil_lower  = lv_led_create(parent, NULL);
    lv_obj_set_size(led_oil_lower, size_w_h_led, size_w_h_led);
    lv_obj_align(led_oil_lower, NULL, LV_ALIGN_CENTER, 100, 0);
    lv_led_off(led_oil_lower);
    lv_obj_set_style_local_bg_color(led_oil_lower, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_obj_t* icon_oil_lower = lv_img_create(led_oil_lower, NULL);
    lv_img_set_src(icon_oil_lower, &oil_lower_icon);
    lv_obj_align(icon_oil_lower, NULL, LV_ALIGN_CENTER, 0, 0);

    led_coolant_temp  = lv_led_create(parent, NULL);
    lv_obj_set_size(led_coolant_temp, size_w_h_led, size_w_h_led);
    lv_obj_align(led_coolant_temp, NULL, LV_ALIGN_CENTER, 170, 0);
    lv_led_off(led_coolant_temp);
    lv_obj_set_style_local_bg_color(led_coolant_temp, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_obj_t* icon_coolant_temp = lv_img_create(led_coolant_temp, NULL);
    lv_img_set_src(icon_coolant_temp, &coolant_temp_icon);
    lv_obj_align(icon_coolant_temp, NULL, LV_ALIGN_CENTER, 0, 0);

    led_abs  = lv_led_create(parent, NULL);
    lv_obj_set_size(led_abs, size_w_h_led, size_w_h_led);
    lv_obj_align(led_abs, NULL, LV_ALIGN_CENTER, 240, 0);
    lv_led_off(led_abs);
    lv_obj_set_style_local_bg_color(led_abs, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_obj_t* icon_abs = lv_img_create(led_abs, NULL);
    lv_img_set_src(icon_abs, &abs_icon);
    lv_obj_align(icon_abs, NULL, LV_ALIGN_CENTER, 0, 0);

    label_time = lv_label_create(parent, NULL);
    lv_label_set_long_mode(label_time, LV_LABEL_LONG_BREAK);
    lv_label_set_recolor(label_time, true);
    lv_label_set_align(label_time, LV_LABEL_ALIGN_CENTER);
    t = time(NULL);
    tm = *localtime(&t);
    lv_label_set_text_fmt(label_time, "#ffffff %02d:%02d#", tm.tm_hour, tm.tm_min);
    lv_obj_set_width(label_time, 150);
    lv_obj_align(label_time, NULL, LV_ALIGN_CENTER, 310, 0);
    lv_style_init(&style_label_time);
    lv_style_set_text_font(&style_label_time, LV_STATE_DEFAULT, &lv_font_montserrat_22);
    lv_obj_add_style(label_time, LV_OBJ_PART_MAIN, &style_label_time);
    taskUpdateTime = lv_task_create([](auto task)
                                                {
                                                    t = time(NULL);
                                                    tm = *localtime(&t);
                                                    lv_label_set_text_fmt(label_time, "#ffffff %02d:%02d#", tm.tm_hour, tm.tm_min);
                                                },
                                                60000, LV_TASK_PRIO_HIGHEST, NULL
                                    );

}

void GaugeCluster::turnOnSignal()
{
    taskTurnOnSignal = lv_task_create([](auto task)
                                                {
                                                    lv_led_toggle(led_signal);
                                                },
                                                300, LV_TASK_PRIO_HIGHEST, NULL
                                    );
}

void GaugeCluster::turnOffSignal()
{
    lv_led_off(led_signal);
    if (taskTurnOnSignal) {
        lv_task_del(taskTurnOnSignal);
        taskTurnOnSignal = NULL;
    }
}

void GaugeCluster::turnOnHighBeam()
{
    lv_led_on(led_headlight);
}

void GaugeCluster::turnOffHighBeam()
{
    lv_led_off(led_headlight);
}

void GaugeCluster::turnOnEngineTrouble()
{
    lv_led_on(led_engine_trouble);
}

void GaugeCluster::turnOffEngineTrouble()
{
    lv_led_off(led_engine_trouble);
}

void GaugeCluster::turnOnOilLower()
{
    lv_led_on(led_oil_lower);
}

void GaugeCluster::turnOffOilLower()
{
    lv_led_off(led_oil_lower);
}

void GaugeCluster::turnOnCoolantTemp()
{
    lv_led_on(led_coolant_temp);
}

void GaugeCluster::turnOffCoolantTemp()
{
    lv_led_off(led_coolant_temp);
}

void GaugeCluster::turnOnNeutral()
{
    lv_led_on(led_neutral);
}

void GaugeCluster::turnOffNeutral()
{
    lv_led_off(led_neutral);
}

void GaugeCluster::turnOnABS()
{
    lv_led_on(led_abs);
}

void GaugeCluster::turnOffABS()
{
    lv_led_off(led_abs);
}

void GaugeCluster::drawGauges(lv_obj_t* parent)
{
    lv_obj_t* cont_gauge = lv_obj_create(parent, NULL);
    lv_obj_add_style(cont_gauge, LV_OBJ_PART_MAIN, &style_main);
    lv_obj_set_size(cont_gauge, LV_HOR_RES_MAX - 170, LV_VER_RES_MAX - 150);
    lv_obj_align(cont_gauge, NULL, LV_ALIGN_CENTER, 83, 5);

    static lv_color_t needle_colors[1];
    needle_colors[0] = LV_COLOR_RED;

    static lv_style_t neddle_common;
    lv_style_init(&neddle_common);
    lv_style_set_line_width(&neddle_common, LV_STATE_DEFAULT, 5);
    lv_style_set_line_rounded(&neddle_common, LV_STATE_DEFAULT, true);
    lv_style_set_size(&neddle_common, LV_STATE_DEFAULT, 25);
    lv_style_set_bg_color(&neddle_common, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    lv_obj_t* gauge_rpm_bg = lv_img_create(cont_gauge, NULL);
    lv_img_set_src(gauge_rpm_bg, &gauge_bg_rpm);
    lv_obj_align(gauge_rpm_bg, NULL, LV_ALIGN_CENTER, -170, -20);

    lv_obj_t* gauge_speed_bg = lv_img_create(cont_gauge, NULL);
    lv_img_set_src(gauge_speed_bg, &gauge_bg_speed);
    lv_obj_align(gauge_speed_bg, NULL, LV_ALIGN_CENTER, 110, -20);


    gauge_rpm = lv_gauge_create(gauge_rpm_bg, NULL);
    lv_obj_clean_style_list(gauge_rpm, LV_GAUGE_PART_MAJOR);
    lv_obj_clean_style_list(gauge_rpm, LV_GAUGE_PART_MAIN);
    lv_obj_add_style(gauge_rpm, LV_GAUGE_PART_NEEDLE, &neddle_common);
    lv_gauge_set_angle_offset(gauge_rpm, -46);
    lv_gauge_set_scale(gauge_rpm, 185, 0, 0);
    lv_gauge_set_range(gauge_rpm, 0, 100);
    lv_gauge_set_needle_count(gauge_rpm, 1, needle_colors);
    lv_obj_set_size(gauge_rpm, 280, 280);
    lv_obj_align(gauge_rpm, NULL, LV_ALIGN_CENTER, 20, 12);

    lv_obj_t* label_rpm_gauge_inf = lv_label_create(gauge_rpm, NULL);
    lv_label_set_long_mode(label_rpm_gauge_inf, LV_LABEL_LONG_BREAK);
    lv_label_set_recolor(label_rpm_gauge_inf, true);
    lv_label_set_align(label_rpm_gauge_inf, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(label_rpm_gauge_inf, "#FFFFFF x1000r/min#");
    lv_obj_set_width(label_rpm_gauge_inf, 150);
    lv_obj_align(label_rpm_gauge_inf, NULL, LV_ALIGN_CENTER, 25, 30);
    static lv_style_t style_label_rpm_gauge_inf;
    lv_style_init(&style_label_rpm_gauge_inf);
    lv_style_set_text_font(&style_label_rpm_gauge_inf, LV_STATE_DEFAULT, &lv_font_montserrat_16);
    lv_obj_add_style(label_rpm_gauge_inf, LV_OBJ_PART_MAIN, &style_label_rpm_gauge_inf);


    gauge_speed = lv_gauge_create(gauge_speed_bg, NULL);
    lv_obj_clean_style_list(gauge_speed, LV_GAUGE_PART_MAJOR);
    lv_obj_clean_style_list(gauge_speed, LV_GAUGE_PART_MAIN);
    lv_obj_add_style(gauge_speed, LV_GAUGE_PART_NEEDLE, &neddle_common);
    lv_gauge_set_angle_offset(gauge_speed, 0);
    lv_gauge_set_scale(gauge_speed, 257, 0, 0);
    lv_gauge_set_range(gauge_speed, 0, 100);
    lv_gauge_set_needle_count(gauge_speed, 1, needle_colors);
    lv_obj_set_size(gauge_speed, 325, 325);
    lv_obj_align(gauge_speed, NULL, LV_ALIGN_CENTER, -5, 12);

    lv_obj_t* label_speed_gauge_inf = lv_label_create(gauge_speed, NULL);
    lv_label_set_long_mode(label_speed_gauge_inf, LV_LABEL_LONG_BREAK);
    lv_label_set_recolor(label_speed_gauge_inf, true);
    lv_label_set_align(label_speed_gauge_inf, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(label_speed_gauge_inf, "#FFFFFF kmph#");
    lv_obj_set_width(label_speed_gauge_inf, 150);
    lv_obj_align(label_speed_gauge_inf, NULL, LV_ALIGN_CENTER, 5, 30);
    static lv_style_t style_label_speed_gauge_inf;
    lv_style_init(&style_label_speed_gauge_inf);
    lv_style_set_text_font(&style_label_speed_gauge_inf, LV_STATE_DEFAULT, &lv_font_montserrat_16);
    lv_obj_add_style(label_speed_gauge_inf, LV_OBJ_PART_MAIN, &style_label_speed_gauge_inf);

    drawGear(cont_gauge, 0);
    drawRecord(cont_gauge, 0, 0);
}

void GaugeCluster::setSpeed(uint16_t num)
{
    uint8_t speedRate;
    speedRate = num * 0.388;
    lv_gauge_set_value(gauge_speed, 0, speedRate);
    if (num <= 10) { // abs warning light goes off after traviling at speed of 10km/h (6mi/h) or higher.
        turnOnABS();
    } else {
        turnOffABS();
    }
}

void GaugeCluster::setRpm(uint16_t num)
{
    uint8_t rpmRate;
    rpmRate = num * 0.011;
    lv_gauge_set_value(gauge_rpm, 0, rpmRate);
}

void GaugeCluster::drawGear(lv_obj_t* parent, uint8_t gearNum)
{
    lv_obj_t* cont_gear = lv_obj_create(parent, NULL);
    static lv_style_t style_cont_gear;
    lv_style_init(&style_cont_gear);
    lv_style_copy(&style_cont_gear, &style_main);
    lv_style_set_radius(&style_cont_gear, LV_STATE_DEFAULT, 10);
    lv_style_set_border_width(&style_cont_gear, LV_STATE_DEFAULT, 2);
    lv_obj_add_style(cont_gear, LV_OBJ_PART_MAIN, &style_cont_gear);
    lv_obj_set_size(cont_gear, 65, 80);
    lv_obj_align(cont_gear, NULL, LV_ALIGN_CENTER, 110, 100);

    lv_obj_t* label_gear_header_bg = lv_obj_create(cont_gear, NULL);
    lv_obj_set_style_local_bg_color(label_gear_header_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(label_gear_header_bg, 65, 20);
    lv_obj_align(label_gear_header_bg, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    lv_obj_t* label_gear_header = lv_label_create(label_gear_header_bg, NULL);
    lv_label_set_recolor(label_gear_header, true);
    lv_label_set_text(label_gear_header, "#2F3243 GEAR#");
    lv_obj_set_width(label_gear_header, 150);
    static lv_style_t style_label_gear_header;
    lv_style_init(&style_label_gear_header);
    lv_style_set_text_font(&style_label_gear_header, LV_STATE_DEFAULT, &lv_font_montserrat_18);
    lv_style_set_text_letter_space(&style_label_gear_header, LV_STATE_DEFAULT, 2);
    lv_obj_add_style(label_gear_header, LV_OBJ_PART_MAIN, &style_label_gear_header);
    lv_obj_align(label_gear_header, NULL, LV_ALIGN_CENTER, 0, 0);

    label_gear_num = lv_label_create(cont_gear, NULL);
    lv_obj_align(label_gear_num, NULL, LV_ALIGN_CENTER, -2, -10);
    lv_label_set_long_mode(label_gear_num, LV_LABEL_LONG_BREAK);
    lv_label_set_recolor(label_gear_num, true);
    lv_label_set_text(label_gear_num, "#05e82e N#");
    lv_obj_set_width(label_gear_num, 150);
    static lv_style_t style_label_gear_num;
    lv_style_init(&style_label_gear_num);
    lv_style_set_text_font(&style_label_gear_num, LV_STATE_DEFAULT, &lv_font_montserrat_48);
    lv_obj_add_style(label_gear_num, LV_OBJ_PART_MAIN, &style_label_gear_num);
}

void GaugeCluster::setGear(uint8_t gearNum)
{
    if (label_gear_num) {
        if (gearNum == 0) {
            lv_label_set_text(label_gear_num, "#05e82e N#");
        } else {
            lv_label_set_text_fmt(label_gear_num, "#FFFFFF %u#", gearNum);
        }
    }
}

void GaugeCluster::drawRecord(lv_obj_t* parent, uint32_t tripNum, uint8_t fuelConsNum)
{
    lv_obj_t* cont_record = lv_obj_create(parent, NULL);
    static lv_style_t style_cont_record;
    lv_style_init(&style_cont_record);
    lv_style_copy(&style_cont_record, &style_main);
    lv_style_set_radius(&style_cont_record, LV_STATE_DEFAULT, 10);
    lv_style_set_border_width(&style_cont_record, LV_STATE_DEFAULT, 2);
    lv_obj_add_style(cont_record, LV_OBJ_PART_MAIN, &style_cont_record);
    lv_obj_set_size(cont_record, 150, 60);
    lv_obj_align(cont_record, NULL, LV_ALIGN_CENTER, -135, 120);

    label_record_trip = lv_label_create(cont_record, NULL);
    lv_obj_align(label_record_trip, NULL, LV_ALIGN_CENTER, -50, -10);
    lv_label_set_long_mode(label_record_trip, LV_LABEL_LONG_BREAK);
    lv_label_set_recolor(label_record_trip, true);
    lv_label_set_text_fmt(label_record_trip, "#FFFFFF Total: %lu km#", tripNum);

    lv_obj_set_width(label_record_trip, 150);
    static lv_style_t style_label_record_trip;
    lv_style_init(&style_label_record_trip);
    lv_style_set_text_font(&style_label_record_trip, LV_STATE_DEFAULT, &lv_font_montserrat_14);
    lv_obj_add_style(label_record_trip, LV_OBJ_PART_MAIN, &style_label_record_trip);

    label_record_fuel_consumption = lv_label_create(cont_record, NULL);
    lv_obj_align(label_record_fuel_consumption, NULL, LV_ALIGN_CENTER, -50, 10);
    lv_label_set_long_mode(label_record_fuel_consumption, LV_LABEL_LONG_BREAK);
    lv_label_set_recolor(label_record_fuel_consumption, true);
    lv_label_set_text_fmt(label_record_fuel_consumption, "#FFFFFF AVG: %u km/L#", fuelConsNum);
    lv_obj_set_width(label_record_fuel_consumption, 150);
    static lv_style_t style_label_record_fuel_consumption;
    lv_style_init(&style_label_record_fuel_consumption);
    lv_style_set_text_font(&style_label_record_fuel_consumption, LV_STATE_DEFAULT, &lv_font_montserrat_14);
    lv_obj_add_style(label_record_fuel_consumption, LV_OBJ_PART_MAIN, &style_label_record_fuel_consumption);
}

void GaugeCluster::setRecord(uint32_t tripNum, uint8_t fuelConsNum)
{
    if (label_record_trip && label_record_fuel_consumption) {
        lv_label_set_text_fmt(label_record_trip, "#FFFFFF Total: %lu km#", tripNum);
        lv_label_set_text_fmt(label_record_fuel_consumption, "#FFFFFF AVG: %u km/L#", fuelConsNum);
    }
}

void GaugeCluster::drawFuelLevel(lv_obj_t* parent)
{
    lv_obj_t* cont_fuel_level = lv_obj_create(parent, NULL);
    lv_obj_set_size(cont_fuel_level, 120, 150);
    static lv_style_t style_fuel_level;
    lv_style_init(&style_fuel_level);
    lv_style_set_radius(&style_fuel_level, LV_STATE_DEFAULT, 10);
    lv_style_copy(&style_fuel_level, &style_main);
    lv_style_set_border_color(&style_fuel_level, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_border_width(&style_fuel_level, LV_STATE_DEFAULT, 1);
    lv_style_set_border_opa(&style_fuel_level, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_border_side(&style_fuel_level, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);
    lv_obj_add_style(cont_fuel_level, LV_OBJ_PART_MAIN, &style_fuel_level);
    lv_obj_align(cont_fuel_level, NULL, LV_ALIGN_CENTER, -320, 10);

    lv_obj_t* icon_fuel_level = lv_img_create(cont_fuel_level, NULL);
    lv_img_set_src(icon_fuel_level, &fuel_icon);
    lv_obj_align(icon_fuel_level, NULL, LV_ALIGN_CENTER, -30, -10);

    lv_obj_t* label_fuel_level_empty = lv_label_create(cont_fuel_level, NULL);
    lv_label_set_long_mode(label_fuel_level_empty, LV_LABEL_LONG_BREAK);
    lv_label_set_recolor(label_fuel_level_empty, true);
    lv_label_set_align(label_fuel_level_empty, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(label_fuel_level_empty, "#FFFFFF E#");
    lv_obj_set_width(label_fuel_level_empty, 150);
    static lv_style_t style_label_fuel_level_empty;
    lv_style_init(&style_label_fuel_level_empty);
    lv_style_set_text_font(&style_label_fuel_level_empty, LV_STATE_DEFAULT, &lv_font_montserrat_16);
    lv_obj_add_style(label_fuel_level_empty, LV_OBJ_PART_MAIN, &style_label_fuel_level_empty);
    lv_obj_align(label_fuel_level_empty, NULL, LV_ALIGN_CENTER, -15, 60);

    lv_obj_t* label_fuel_level_full = lv_label_create(cont_fuel_level, NULL);
    lv_label_set_long_mode(label_fuel_level_full, LV_LABEL_LONG_BREAK);
    lv_label_set_recolor(label_fuel_level_full, true);
    lv_label_set_align(label_fuel_level_full, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(label_fuel_level_full, "#FFFFFF F#");
    lv_obj_set_width(label_fuel_level_full, 150);
    static lv_style_t style_label_fuel_level_full;
    lv_style_init(&style_label_fuel_level_full);
    lv_style_set_text_font(&style_label_fuel_level_full, LV_STATE_DEFAULT, &lv_font_montserrat_16);
    lv_obj_add_style(label_fuel_level_full, LV_OBJ_PART_MAIN, &style_label_fuel_level_full);
    lv_obj_align(label_fuel_level_full, NULL, LV_ALIGN_CENTER, -15, -60);

    bar_fuel_level = lv_bar_create(cont_fuel_level, NULL);
    lv_obj_set_size(bar_fuel_level, 80, 200);
    lv_bar_set_range(bar_fuel_level, 0, 100);
    lv_bar_set_anim_time(bar_fuel_level, 2000);
    lv_bar_set_value(bar_fuel_level, 100, LV_ANIM_ON);
    static lv_style_t style_bar_fuel_level;
    lv_style_init(&style_bar_fuel_level);
    lv_style_copy(&style_bar_fuel_level, &style_main);
    lv_obj_add_style(bar_fuel_level, LV_OBJ_PART_MAIN, &style_bar_fuel_level);
    lv_obj_set_style_local_bg_color(bar_fuel_level, LV_BAR_PART_INDIC , LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_align(bar_fuel_level, NULL, LV_ALIGN_CENTER, 30, 0);

    lv_obj_t* img_fuel_level_mask = lv_img_create(cont_fuel_level, NULL);
    lv_img_set_src(img_fuel_level_mask, &fuel_level_mask);
    lv_obj_align(img_fuel_level_mask, NULL, LV_ALIGN_CENTER, 30, -5);
}

void GaugeCluster::setFuelLevel(uint8_t fuelLevelNum)
{
    if (bar_fuel_level) {
        lv_bar_set_value(bar_fuel_level, fuelLevelNum, LV_ANIM_ON);
    }
    if (fuelLevelNum < 40) {
        lv_led_on(led_fuel_empty);
    } else {
        lv_led_off(led_fuel_empty);
    }
}

void GaugeCluster::demoGaugeCluster()
{
    static int16_t countingRate = 0;
    static uint8_t gearNum = 0;
    static bool foward = true;

    initGaugeCluster();

    lv_obj_t * back = home_page_->add_back(specific_back_to_home_event_cb);
    home_page_->demo_anim_in(back, 200);
    lv_obj_t* title_ = nullptr;
    title_ = home_page_->add_title("GAUGE CLUSTER");
    home_page_->demo_anim_in(title_, 200);

    drawHeader(cont_header);
    drawGauges(cont_body);
    drawFuelLevel(cont_body);

    // setSpeed(120);
    // setRpm(3000);
    turnOnSignal();

    taskTest = lv_task_create([](auto task)
                                        {
                                            auto self = static_cast<GaugeCluster*>(task->user_data);
                                            self->setSpeed(countingRate);
                                            self->setRpm(countingRate*100);
                                            if (countingRate >= 900) {
                                                foward = false;
                                                self->turnOffSignal();
                                            }
                                            if (foward) {
                                                countingRate += 1;
                                                self->turnOnHighBeam();
                                                self->turnOnEngineTrouble();
                                                self->turnOnOilLower();
                                                self->turnOnCoolantTemp();
                                            } else {
                                                self->turnOffHighBeam();
                                                self->turnOffEngineTrouble();
                                                self->turnOffOilLower();
                                                self->turnOffCoolantTemp();
                                                countingRate -= 1;
                                                if (countingRate < 0) {
                                                    foward = true;
                                                }
                                            }
                                            if (countingRate % 20 == 0) {
                                                gearNum ++;
                                                if (gearNum > 6) {
                                                    gearNum = 0;
                                                }
                                                if (gearNum == 0) {
                                                    lv_led_on(led_neutral);
                                                } else {
                                                    lv_led_off(led_neutral);
                                                }
                                            }
                                            self->setGear(gearNum);
                                            self->setRecord((countingRate*1000 + 123), countingRate / 10);
                                            self->setFuelLevel(countingRate/2);

                                        },
                                        10, LV_TASK_PRIO_LOWEST, this
                                    );
}

}
