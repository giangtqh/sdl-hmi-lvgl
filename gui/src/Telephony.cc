#include <stdio.h>
#include "Telephony.h"

namespace gui {

#ifdef __cplusplus
extern "C" {
#endif
LV_IMG_DECLARE(contact_icon);
LV_IMG_DECLARE(call_icon);
LV_IMG_DECLARE(end_call_icon);
LV_IMG_DECLARE(hang_up_call_icon);
LV_IMG_DECLARE(contact_profile_call_icon);
LV_IMG_DECLARE(call_made_icon);
LV_IMG_DECLARE(call_received_icon);
LV_IMG_DECLARE(call_missed_incoming_icon);
LV_IMG_DECLARE(call_missed_outgoing_icon);
#ifdef __cplusplus
} /* extern "C" */
#endif

uint32_t Telephony::duration_sec = 0;
lv_task_t* Telephony::task_during_call;
lv_obj_t* Telephony::label_duration;
lv_obj_t* Telephony::cont_call;
lv_obj_t* Telephony::cont_tab;
lv_obj_t* Telephony::ta_input_phone_num;

static const char * btnm_map[] = {"1", "2", "3", "\n",
                                  "4", "5", "6", "\n",
                                  "7", "8", "9", "\n",
                                  "*", "0", "#", "\n",
                                  LV_SYMBOL_CALL, LV_SYMBOL_DIRECTORY, LV_SYMBOL_BACKSPACE, ""};

Telephony::Telephony(sdlcore_message_handler::GuiController* controller)
    : _controller(controller)
{
}

Telephony::~Telephony()
{
}

void Telephony::drawCommonCallScr(enum call_type type, lv_obj_t* parent, const char* header, const char* title, const char* number)
{
    strcpy(active_number, number);
    cont_call = lv_obj_create(parent, NULL);
    lv_obj_set_size(cont_call, 300, 400);
    static lv_style_t style_cont_call;
    lv_style_init(&style_cont_call);
    lv_style_set_bg_color(&style_cont_call, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_add_style(cont_call, LV_CONT_PART_MAIN, &style_cont_call);

    lv_obj_t* cont_header = lv_cont_create(cont_call, NULL);
    lv_obj_set_size(cont_header, 300, 50);
    lv_obj_set_pos(cont_header, 0, 0);

    static lv_style_t style_cont_header;
    lv_style_init(&style_cont_header);
    lv_style_set_bg_color(&style_cont_header, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_obj_add_style(cont_header, LV_CONT_PART_MAIN, &style_cont_header);

    lv_obj_t* label_header = lv_label_create(cont_header, NULL);
    lv_label_set_recolor(label_header, true);
    lv_label_set_text(label_header, header);
    lv_label_set_align(label_header, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label_header, NULL, LV_ALIGN_CENTER, 0, 0);

    label_duration = lv_label_create(cont_call, NULL);
    lv_label_set_recolor(label_duration, true);
    lv_label_set_align(label_duration, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label_duration, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(label_duration, 300, 50);
    lv_obj_set_pos(label_duration, 130, 100);
    lv_label_set_text(label_duration, title);

    lv_obj_t* cont_icons = lv_cont_create(cont_call, NULL);
    lv_obj_set_size(cont_icons, 300, 150);
    lv_obj_set_pos(cont_icons, 0, 150);
    static lv_style_t style_cont_icons;
    lv_style_init(&style_cont_icons);
    lv_style_set_bg_color(&style_cont_icons, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    lv_obj_add_style(cont_icons, LV_CONT_PART_MAIN, &style_cont_icons);

    lv_obj_t* img_contact = lv_img_create(cont_icons, NULL);
    lv_img_set_src(img_contact, &contact_icon);
    lv_obj_align(img_contact, NULL, LV_ALIGN_IN_TOP_MID, 0, -20);

    lv_obj_t* label_contact = lv_label_create(cont_icons, NULL);
    lv_label_set_recolor(label_contact, true);
    lv_label_set_text(label_contact, active_number);
    lv_label_set_align(label_contact, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label_contact, NULL, LV_ALIGN_CENTER, 0, 0);

    if (type == INCOMINGCALL){
        lv_obj_t* imgbtn_call = lv_imgbtn_create(cont_icons, NULL);
        lv_imgbtn_set_src(imgbtn_call, LV_BTN_STATE_RELEASED, &call_icon);
        lv_imgbtn_set_src(imgbtn_call, LV_BTN_STATE_PRESSED, &call_icon);
        lv_obj_set_user_data(imgbtn_call, this);
        lv_obj_set_event_cb(imgbtn_call, call_btn_event);
        lv_obj_align(imgbtn_call, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10);

        lv_obj_t* imgbtn_end_call = lv_imgbtn_create(cont_icons, NULL);
        lv_imgbtn_set_src(imgbtn_end_call, LV_BTN_STATE_RELEASED, &end_call_icon);
        lv_imgbtn_set_src(imgbtn_end_call, LV_BTN_STATE_PRESSED, &end_call_icon);
        lv_obj_set_user_data(imgbtn_end_call, this);
        lv_obj_set_event_cb(imgbtn_end_call, end_call_btn_event);
        lv_obj_align(imgbtn_end_call, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -10, 10);
    } else if (type == DURINGCALL) {
        lv_obj_t* imgbtn_hang_up_call = lv_imgbtn_create(cont_icons, NULL);
        lv_imgbtn_set_src(imgbtn_hang_up_call, LV_BTN_STATE_RELEASED, &hang_up_call_icon);
        lv_imgbtn_set_src(imgbtn_hang_up_call, LV_BTN_STATE_PRESSED, &end_call_icon);
        lv_obj_set_user_data(imgbtn_hang_up_call, this);
        lv_obj_set_event_cb(imgbtn_hang_up_call, hang_up_call_btn_event);
        lv_obj_align(imgbtn_hang_up_call, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10);
    } else if (type == DIALING) {
        lv_obj_set_pos(label_duration, 100, 100);

        lv_obj_t* imgbtn_end_call = lv_imgbtn_create(cont_icons, NULL);
        lv_imgbtn_set_src(imgbtn_end_call, LV_BTN_STATE_RELEASED, &end_call_icon);
        lv_imgbtn_set_src(imgbtn_end_call, LV_BTN_STATE_PRESSED, &end_call_icon);
        lv_obj_set_user_data(imgbtn_end_call, this);
        lv_obj_set_event_cb(imgbtn_end_call, end_call_btn_event);
        lv_obj_align(imgbtn_end_call, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10);
    }
}

void Telephony::drawIncomingCall(lv_obj_t* parent, const char* number)
{
    drawCommonCallScr(INCOMINGCALL, parent, "#ffffff INCOMING CALL #", "", number);
}

void Telephony::drawDuringCallStatic(lv_obj_t* parent, const char* number)
{
    drawCommonCallScr(DURINGCALL, parent, "#ffffff CALL ACCEPTED#", "", number);
}

void Telephony::drawDialing(lv_obj_t* parent, const char* number)
{
    drawCommonCallScr(DIALING, parent, "#ffffff DIALING #", "Connecting...", number);
}

void Telephony::drawDuringCall(lv_obj_t* parent, const char* number)
{
    drawDuringCallStatic(parent, number);
    task_during_call = lv_task_create([](auto task)
                                                {
                                                    uint8_t h, m, s;
                                                    s = duration_sec;
                                                    h = (s/3600);
                                                    m = (s -(3600*h))/60;
                                                    s = (s -(3600*h)-(m*60));
                                                    lv_label_set_text_fmt(label_duration, "#ffffff %02d:%02d#", m, s);
                                                    duration_sec++;
                                                },
                                                1000, LV_TASK_PRIO_HIGHEST, NULL);
}

void Telephony::incomingCall(const char* number)
{
    drawIncomingCall(lv_scr_act(), number);
}

void Telephony::duringCall()
{
    lv_obj_del(cont_call); // delete previous obj such as incoming/dialing call scr
    drawDuringCall(lv_scr_act(), active_number);
}

void Telephony::dialing(const char* number)
{
    drawDialing(lv_scr_act(), number);
}

void Telephony::endCall()
{
    lv_obj_del(cont_call);
}

void Telephony::hangupCall()
{
    lv_task_del(task_during_call);
    duration_sec = 0;
    lv_obj_del(cont_call);
}

void Telephony::drawCallActivities(lv_obj_t* parent, const Json::Value logs)
{
    cont_tab = lv_obj_create(parent, NULL);
    lv_obj_set_size(cont_tab, 300, 400);

    lv_obj_t* tv = lv_tabview_create(cont_tab, NULL);
    lv_obj_t* tab_log = lv_tabview_add_tab(tv, "LOG");
    lv_obj_t* tab_keypad = lv_tabview_add_tab(tv, "KEY PAD");
    lv_obj_t* tab_exit = lv_tabview_add_tab(tv, LV_SYMBOL_CLOSE);
    lv_tabview_set_tab_act(tv, 0, LV_ANIM_ON);
    lv_tabview_set_anim_time(tv, 100);
    lv_tabview_set_btns_pos(tv, LV_TABVIEW_TAB_POS_TOP);
    lv_obj_set_event_cb(tv, tabview_event_cb);

    drawTabLog(tab_log, logs);
    drawTabKeypad(tab_keypad);
}

void Telephony::drawTabLog(lv_obj_t* parent, const Json::Value logs)
{
    lv_obj_t* list_call = lv_list_create(parent, NULL);
    lv_obj_align(list_call, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t* list_btn;
    for (uint8_t idx = 0; idx < logs["data"].size(); ++idx) {
        switch(logs["data"][idx]["type"].asInt()) {
            case 1:
                list_btn = lv_list_add_btn(list_call, &call_made_icon, logs["data"][idx]["number"].asString().c_str());
                lv_obj_set_user_data(list_btn, this);
                lv_obj_set_event_cb(list_btn, list_call_event);
                break;
            case 2:
                list_btn = lv_list_add_btn(list_call, &call_received_icon, logs["data"][idx]["number"].asString().c_str());
                lv_obj_set_user_data(list_btn, this);
                lv_obj_set_event_cb(list_btn, list_call_event);
                break;
            case 3:
                list_btn = lv_list_add_btn(list_call, &call_missed_incoming_icon, logs["data"][idx]["number"].asString().c_str());
                lv_obj_set_user_data(list_btn, this);
                lv_obj_set_event_cb(list_btn, list_call_event);
                break;
            case 4:
                list_btn = lv_list_add_btn(list_call, &call_missed_outgoing_icon, logs["data"][idx]["number"].asString().c_str());
                lv_obj_set_user_data(list_btn, this);
                lv_obj_set_event_cb(list_btn, list_call_event);
                break;
            default:
                break;
        }
    }
}

void Telephony::drawTabKeypad(lv_obj_t* parent)
{
    ta_input_phone_num = lv_textarea_create(parent, NULL);
    lv_obj_set_pos(ta_input_phone_num, 0, 10);
    lv_cont_set_fit2(ta_input_phone_num, LV_FIT_PARENT, LV_FIT_NONE);
    lv_textarea_set_text(ta_input_phone_num, "");
    lv_textarea_set_placeholder_text(ta_input_phone_num, "Phone Number");
    lv_textarea_set_one_line(ta_input_phone_num, true);
    lv_textarea_set_cursor_hidden(ta_input_phone_num, true);

    lv_obj_t* btnm_kepad = lv_btnmatrix_create(parent, NULL);
    lv_obj_set_user_data(btnm_kepad, this);
    lv_obj_set_size(btnm_kepad, 150, 200);
    lv_obj_set_pos(btnm_kepad, 60, 50);
    lv_btnmatrix_set_map(btnm_kepad, btnm_map);
    lv_btnmatrix_set_btn_ctrl(btnm_kepad, 10, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_btn_ctrl(btnm_kepad, 11, LV_BTNMATRIX_CTRL_CHECK_STATE);
    lv_obj_set_event_cb(btnm_kepad, btnm_keypad_event);

    // lv_obj_t* imgbtn_call = lv_imgbtn_create(parent, NULL);
    // lv_imgbtn_set_src(imgbtn_call, LV_BTN_STATE_RELEASED, &call_icon);
    // lv_imgbtn_set_src(imgbtn_call, LV_BTN_STATE_PRESSED, &call_icon);
    // lv_obj_set_user_data(imgbtn_call, this);
    // lv_obj_set_event_cb(imgbtn_call, call_btn_event);
    // lv_obj_set_pos(imgbtn_call, 90, 260);
    //
    // lv_obj_t* imgbtn_contact_profile_call = lv_imgbtn_create(parent, NULL);
    // lv_imgbtn_set_src(imgbtn_contact_profile_call, LV_BTN_STATE_RELEASED, &contact_profile_call_icon);
    // lv_imgbtn_set_src(imgbtn_contact_profile_call, LV_BTN_STATE_PRESSED, &contact_profile_call_icon);
    // lv_obj_set_user_data(imgbtn_contact_profile_call, this);
    // lv_obj_set_event_cb(imgbtn_contact_profile_call, call_btn_event);
    // lv_obj_set_pos(imgbtn_contact_profile_call, 150, 250);
}

void Telephony::callActivities(lv_obj_t* parent, const Json::Value logs)
{
    drawCallActivities(parent, logs);
}

void Telephony::call_btn_event(lv_obj_t* obj, lv_event_t event)
{
    if (lv_obj_get_user_data(obj)) {
        auto self = static_cast<Telephony*>(lv_obj_get_user_data(obj));
        if (event == LV_EVENT_PRESSED) {
            self->duringCall();
            // TODO: notify SDL
        }
    }
}

void Telephony::end_call_btn_event(lv_obj_t* obj, lv_event_t event)
{
    if (lv_obj_get_user_data(obj)) {
        auto self = static_cast<Telephony*>(lv_obj_get_user_data(obj));
        if (event == LV_EVENT_PRESSED) {
            self->endCall();
            // TODO: notify SDL
        }
    }
}

void Telephony::hang_up_call_btn_event(lv_obj_t* obj, lv_event_t event)
{
    if (lv_obj_get_user_data(obj)) {
        auto self = static_cast<Telephony*>(lv_obj_get_user_data(obj));
        if (event == LV_EVENT_PRESSED) {
            self->hangupCall();
            // TODO: notify SDL
        }
    }
}

void Telephony::tabview_event_cb(lv_obj_t* obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED) {
        if (lv_tabview_get_tab_act(obj) == 2) {
            lv_obj_del(cont_tab);
        }
    }
}

void Telephony::list_call_event(lv_obj_t* obj, lv_event_t event)
{
    if (lv_obj_get_user_data(obj)) {
        auto self = static_cast<Telephony*>(lv_obj_get_user_data(obj));
        if (event == LV_EVENT_PRESSED) {
            const char* number = lv_list_get_btn_text(obj);
            self->dialing(number);
            // TODO: notify SDL
        }
    }
}

void Telephony::btnm_keypad_event(lv_obj_t* obj, lv_event_t event)
{
    if (lv_obj_get_user_data(obj)) {
        auto self = static_cast<Telephony*>(lv_obj_get_user_data(obj));
        if (event == LV_EVENT_VALUE_CHANGED) {
            const char * txt = lv_btnmatrix_get_active_btn_text(obj);
            static char inputNumber[NUMBER_LEN_MAX];
            if (strlen(inputNumber) < NUMBER_LEN_MAX) {
                if (strcmp(LV_SYMBOL_BACKSPACE, txt) == 0) {
                    inputNumber[strlen(inputNumber)-1] = '\0';
                } else if ((strcmp(LV_SYMBOL_CALL, txt) != 0) && (strcmp(LV_SYMBOL_DIRECTORY, txt) != 0)) {
                    strcat(inputNumber, txt);
                }
                lv_textarea_set_text(ta_input_phone_num, inputNumber);
            } else {
                if (strcmp(LV_SYMBOL_BACKSPACE, txt) == 0) {
                    inputNumber[strlen(inputNumber)-1] = '\0';
                    lv_textarea_set_text(ta_input_phone_num, inputNumber);
                }
            }
            if (strlen(inputNumber) != 0) {
                if (strcmp(LV_SYMBOL_CALL, txt) == 0) {
                    self->dialing(inputNumber);
                    inputNumber[0] = '\0';
                    lv_textarea_set_text(ta_input_phone_num, inputNumber);
                    // TODO: notify SDL
                }
            }
            if (strcmp(LV_SYMBOL_DIRECTORY, txt) == 0) {
                // TODO: jump to `contact list` screen
            }
        }
    }
}

}
