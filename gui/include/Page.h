#ifndef UI_BASE_PAGE_H_
#define UI_BASE_PAGE_H_

#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <map>

#include "lvgl/lvgl.h"

#include "ApplicationInfo.h"

namespace gui {

class Page {
 public:
    Page() {}
    virtual ~Page() {}

    virtual void init(void) = 0;

    virtual void shutdown(void) = 0;

    virtual void initDisplay(void) {}
};

}

#endif  // UI_BASE_PAGE_H_