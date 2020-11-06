#ifndef TELEPHONY_H_
#define TELEPHONY_H_

#include "GuiController.h"
#include "lvgl/lvgl.h"

#define NUMBER_LEN_MAX 20

namespace gui {

enum call_type {INCOMINGCALL, DURINGCALL, DIALING};

class Telephony {
 public:
    Telephony(sdlcore_message_handler::GuiController* controller);
    virtual ~Telephony();

    void incomingCall(const char* number);
    void dialing(const char* number);
    void duringCall();
    void endCall();
    void hangupCall();
    void callActivities(lv_obj_t* parent, const Json::Value logs);

 private:
    void drawCommonCallScr(enum call_type type, lv_obj_t* parent, const char* header, const char* title, const char* number);
    void drawDuringCallStatic(lv_obj_t* parent, const char* number);
    void drawIncomingCall(lv_obj_t* parent, const char* number);
    void drawDuringCall(lv_obj_t* parent, const char* number);
    void drawDialing(lv_obj_t* parent, const char* number);
    void drawCallActivities(lv_obj_t* parent, const Json::Value logs);
    void drawTabLog(lv_obj_t* parent, const Json::Value logs);
    void drawTabKeypad(lv_obj_t* parent);

    static void call_btn_event(lv_obj_t* obj, lv_event_t event);
    static void end_call_btn_event(lv_obj_t* obj, lv_event_t event);
    static void hang_up_call_btn_event(lv_obj_t* obj, lv_event_t event);
    static void tabview_event_cb(lv_obj_t* obj, lv_event_t event);
    static void list_call_event(lv_obj_t* obj, lv_event_t event);
    static void btnm_keypad_event(lv_obj_t* obj, lv_event_t event);

    sdlcore_message_handler::GuiController* _controller;
    char active_number[NUMBER_LEN_MAX];
    static lv_obj_t* cont_call;
    static lv_obj_t* cont_tab;
    static lv_task_t* task_during_call;
    static uint32_t duration_sec;
    static lv_obj_t* label_duration;
    static lv_obj_t* ta_input_phone_num;
};

}

#endif // TELEPHONY_H_
