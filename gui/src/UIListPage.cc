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

LV_IMG_DECLARE(img_list_border);
// TODO: The icon background, change to circle icon
LV_IMG_DECLARE(btn_bg_green);

#ifdef __cplusplus
} /* extern "C" */
#endif

#include <algorithm>

#include "UIListPage.h"
#include "HomePage.h"
#include "Log.h"

#include "demo_theme.h"

namespace gui {

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * list;
static lv_font_t * list_font_small;
static lv_font_t * list_font_medium;
static lv_style_t list_style_btn;
static lv_style_t list_style_scrollbar;
static lv_style_t list_style_title;
static lv_style_t list_style_body;
static lv_style_t list_style_time;

// TODO: calculate screen, split into parts
#define LV_TITLE_OVERLAP  60

UIListPage::UIListPage(sdlcore_message_handler::GuiController* controller, HomePage* home)
    : controller_(controller)
    , home_page_(home)
    , type_(sdlcore_message_handler::ListType::SMS)
    , shutdown_(false)
    , lv_page_(nullptr) {
}

UIListPage::~UIListPage() {
    shutdown();
}

void UIListPage::init(void) {
    // TODO: Think what we should do here?
}

void UIListPage::shutdown(void) {

}

void UIListPage::setListData(const std::vector<std::shared_ptr<sdlcore_message_handler::ListItem>>& data) {
    mListData = data;
    // mCmdIds.clear();
    // TODO: add cmdId as attribute of the object, change map -> vector as it's very convinient when access by index
    //std::transform(data.begin(), data.end(), std::back_inserter(mCmdIds), [&] (auto& it) { return it.first; });
    initDisplay();
}

void UIListPage::initDisplay(void) {
    LOGD("UIListPage::%s()", __func__);
    home_page_->demo_anim_out_all(lv_scr_act(), 0);
    createList(type_, lv_scr_act());
    home_page_->demo_anim_in(list, 200);

    lv_obj_t * back = home_page_->add_back(home_page_->back_to_home_event_cb);
    home_page_->demo_anim_in(back, 200);
}

lv_obj_t * UIListPage::createList(const sdlcore_message_handler::ListType type, lv_obj_t* parent) {
    list_font_small = &lv_font_montserrat_12;
    list_font_medium = &lv_font_montserrat_14;

    lv_style_init(&list_style_scrollbar);
    lv_style_set_size(&list_style_scrollbar, LV_STATE_DEFAULT, 4);
    lv_style_set_bg_opa(&list_style_scrollbar, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&list_style_scrollbar, LV_STATE_DEFAULT, lv_color_hex3(0xeee));
    lv_style_set_radius(&list_style_scrollbar, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_pad_right(&list_style_scrollbar, LV_STATE_DEFAULT, 4);

    lv_style_init(&list_style_btn);
    lv_style_set_bg_opa(&list_style_btn, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_bg_opa(&list_style_btn, LV_STATE_PRESSED, LV_OPA_COVER);
    lv_style_set_bg_opa(&list_style_btn, LV_STATE_CHECKED, LV_OPA_COVER);
    lv_style_set_bg_color(&list_style_btn, LV_STATE_PRESSED, lv_color_hex(0x4c4965));
    lv_style_set_bg_color(&list_style_btn, LV_STATE_CHECKED, lv_color_hex(0x4c4965));
    lv_style_set_text_opa(&list_style_btn, LV_STATE_DISABLED, LV_OPA_40);
    lv_style_set_image_opa(&list_style_btn, LV_STATE_DISABLED, LV_OPA_40);

    lv_style_init(&list_style_title);
    lv_style_set_text_font(&list_style_title, LV_STATE_DEFAULT, &lv_font_montserrat_14);
    lv_style_set_text_color(&list_style_title, LV_STATE_DEFAULT, lv_color_hex(0xffffff));

    lv_style_init(&list_style_body);
    lv_style_set_text_font(&list_style_body, LV_STATE_DEFAULT, &lv_font_montserrat_12);
    lv_style_set_text_color(&list_style_body, LV_STATE_DEFAULT, lv_color_hex(0xb1b0be));

    // This style for call log duration
    lv_style_init(&list_style_time);
    lv_style_set_text_font(&list_style_time, LV_STATE_DEFAULT, &lv_font_montserrat_12);
    lv_style_set_text_color(&list_style_time, LV_STATE_DEFAULT, lv_color_hex(0xffffff));

    switch (type_) {
        case sdlcore_message_handler::ListType::SMS: {
            title_ = home_page_->add_title("SMS Messages");
        }
        break;
        case sdlcore_message_handler::ListType::CONTACT: {
            title_ = home_page_->add_title("Contacts");
        }
        break;
        case sdlcore_message_handler::ListType::CALL_LOG: {
            title_ = home_page_->add_title("Call Log");
        }
        break;
        default:
        break;
    }
    home_page_->demo_anim_in(title_, 200);

    /*Create an empty white main container*/
    list = lv_page_create(parent, NULL);
    //lv_obj_set_size(list, LV_HOR_RES, LV_VER_RES - LV_TITLE_OVERLAP);
    lv_obj_set_size(list, 720, LV_VER_RES - 2*LV_TITLE_OVERLAP);
    lv_obj_align(list, title_, LV_ALIGN_OUT_BOTTOM_MID, 0, LV_TITLE_OVERLAP);

    // lv_obj_set_y(list, LV_TITLE_OVERLAP);
    lv_obj_clean_style_list(list, LV_PAGE_PART_BG);
    lv_obj_clean_style_list(list, LV_PAGE_PART_SCROLLABLE);
    lv_obj_clean_style_list(list, LV_PAGE_PART_SCROLLBAR);
    lv_obj_add_style(list, LV_PAGE_PART_SCROLLBAR, &list_style_scrollbar);
    lv_page_set_scrl_layout(list, LV_LAYOUT_COLUMN_MID);

    for (auto item : mListData) {
        addListItem(list, item);
    }

    return list;
}

lv_obj_t * UIListPage::addListItem(lv_obj_t * page, std::shared_ptr<sdlcore_message_handler::ListItem> item) {
    const void *img_icon = &btn_bg_green;
    std::string title = item->name_;
    std::string body = item->number_;
    switch (type_) {
        case sdlcore_message_handler::ListType::SMS: {
            auto sms = std::dynamic_pointer_cast<sdlcore_message_handler::SMSMessage>(item);
            if (sms) {
                img_icon = &btn_bg_green;
                title.assign(sms->address_);
                body.assign(sms->body_);
                // LOGD("UIListPage::%s() title: %s, body: %s", __func__, title.c_str(), body.c_str());
            } else {
                LOGD("UIListPage::%s() dynamic_pointer_cast<SMSMessage> return NULL", __func__);
            }
        }
        break;
        case sdlcore_message_handler::ListType::CONTACT: {
            auto contact = std::dynamic_pointer_cast<sdlcore_message_handler::ContactItem>(item);
            if (contact) {
                title.assign(contact->name_);
                body.assign(contact->number_);
            } else {
                LOGD("UIListPage::%s() dynamic_pointer_cast<ContactItem> return NULL", __func__);
            }
        }
        break;
        case sdlcore_message_handler::ListType::CALL_LOG: {
            auto callLog = std::dynamic_pointer_cast<sdlcore_message_handler::CallLogItem>(item);
            if (callLog) {
                title.assign(callLog->name_);
                body.assign(callLog->number_);
                switch (callLog->type_) {
                    case sdlcore_message_handler::CallLogType::DIAL:
                        // img_icon = &out_going_call;
                        break;
                    case sdlcore_message_handler::CallLogType::MISSED:
                        // img_icon = &missed_call;
                        break;
                    case sdlcore_message_handler::CallLogType::IN_CALL:
                        // img_icon = &in_call;
                        break;
                    default:
                        // img_icon = &in_call;
                        break;
                }
            } else {
                LOGD("UIListPage::%s() dynamic_pointer_cast<CallLogItem> return NULL", __func__);
            }
        }
        break;
        default:
        break;
    }

    lv_obj_t * btn = lv_obj_create(page, NULL);
    lv_obj_set_size(btn, lv_page_get_width_fit(page), 60);
    lv_obj_clean_style_list(btn, LV_BTN_PART_MAIN);
    lv_obj_add_style(btn, LV_BTN_PART_MAIN, &list_style_btn);
    lv_obj_add_protect(btn, LV_PROTECT_PRESS_LOST);
    lv_obj_set_event_cb(btn, btn_event_cb);
    lv_obj_set_user_data(btn, this);
    lv_page_glue_obj(btn, true);

    // icon for the first letter
    lv_obj_t * icon = lv_img_create(btn, NULL);
    lv_obj_set_click(icon, true);
    lv_theme_apply(icon, (lv_theme_style_t)LV_DEMO_PRINTER_THEME_ICON);
    lv_img_set_src(icon, img_icon);
    lv_obj_set_size(icon, 58, 60);
    lv_img_set_antialias(icon, false);
    // if (type_ != sdlcore_message_handler::ListType::CALL_LOG) {
        lv_obj_t * label = lv_label_create(icon, NULL);
        lv_label_set_text(label, title.substr(0, 1).c_str());
        lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
    // }

    // The first line: (contact name or phone number)
    lv_obj_t * title_obj = lv_label_create(btn, NULL);
    lv_label_set_text(title_obj, title.c_str());
    lv_obj_align(title_obj, icon, LV_ALIGN_OUT_RIGHT_TOP, 5, 13);
    lv_obj_add_style(title_obj, LV_LABEL_PART_MAIN, &list_style_title);

    // The second line (SMS body)
    lv_obj_t * body_obj = lv_label_create(btn, NULL);
    lv_label_set_text(body_obj, body.c_str());
    lv_label_set_long_mode(body_obj, LV_LABEL_LONG_DOT);
    lv_obj_align(body_obj, title_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_obj_add_style(body_obj, LV_LABEL_PART_MAIN, &list_style_body);

    lv_obj_t * border = lv_img_create(btn, NULL);
    lv_img_set_src(border, &img_list_border);
    lv_obj_set_width(border, lv_obj_get_width(btn));
    lv_obj_align(border, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    return btn;
}

void UIListPage::btn_event_cb(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;

    lv_obj_t * child = lv_obj_get_child_back(lv_page_get_scrl(list), NULL);
    uint32_t index = 0;
    while (child) {
        if (btn == child) break;
        index++;
        child = lv_obj_get_child_back(lv_page_get_scrl(list), child);
    }
    // TODO: handle user data
    LOGD("UIListPage::%s() index: %d", __func__, index);
    if (btn->user_data) {
        auto me = static_cast<UIListPage*>(btn->user_data);
        if (index < (uint32_t)me->mListData.size()) {
            me->controller_->onListItemSelected(me->mListData[index]->command_id_);
        }
    }
}

}
