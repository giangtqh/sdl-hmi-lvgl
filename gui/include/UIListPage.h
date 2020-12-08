#ifndef UI_BASE_LIST_PAGE_H_
#define UI_BASE_LIST_PAGE_H_

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
class UIListPage: public Page {
 public:
    UIListPage(sdlcore_message_handler::GuiController*, HomePage*);
    virtual ~UIListPage();

    virtual void init(void) override;
    virtual void shutdown(void) override;
    virtual void initDisplay(void) override;

    void setListData(const std::vector<std::shared_ptr<sdlcore_message_handler::ListItem>>& data);

    void setListType(const sdlcore_message_handler::ListType type) {
       type_ = type;
    }

 private:
    lv_obj_t * createList(const sdlcore_message_handler::ListType type, lv_obj_t* parent);
    lv_obj_t * addListItem(lv_obj_t * page, std::shared_ptr<sdlcore_message_handler::ListItem>);
    static void btn_event_cb(lv_obj_t * btn, lv_event_t event);

    sdlcore_message_handler::GuiController* controller_;
    HomePage* home_page_;
    sdlcore_message_handler::ListType   type_;
    std::atomic_bool shutdown_;
    std::vector<std::shared_ptr<sdlcore_message_handler::ListItem>> mListData;
    lv_obj_t *  lv_page_;
    lv_obj_t *  lv_cont_;  // list item
    // TODO: If change from map->vector, we won't need this
   //  std::vector<uint32_t> mCmdIds;
    lv_obj_t* title_ = nullptr;
};

}

#endif  // UI_BASE_LIST_PAGE_H_