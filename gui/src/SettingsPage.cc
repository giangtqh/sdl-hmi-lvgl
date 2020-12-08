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

#include "SettingsPage.h"
#include "Log.h"

namespace gui {

SettingsPage::SettingsPage(sdlcore_message_handler::GuiController* controller, HomePage* home)
    : controller_(controller)
    , home_page_(home)
    , shutdown_(false)
    {
}

SettingsPage::~SettingsPage() {
    shutdown();
}

void SettingsPage::init(void) {
    // TODO:create page with tab layout
    // controller_->updateDeviceList();     // this doesn't work
    // TODO: For test only, to get list of connected devices
    // controller_->startDeviceDiscovery();     // this works like a charm

}

void SettingsPage::onUpdateDeviceList(const std::map<std::string, std::shared_ptr<sdlcore_message_handler::Device>>& deviceList) {
    LOGD("SettingsPage::%s() deviceList size: %lu", __func__, (uint64_t)deviceList.size());
    mDeviceList = deviceList;
}

void SettingsPage::shutdown(void) {

}

void SettingsPage::initDisplay(void) {

}

}