#ifndef GAUGE_CLUSTER_H_
#define GAUGE_CLUSTER_H_

#include "lvgl/lvgl.h"
#include "GuiController.h"
#include "HomePage.h"

namespace gui {

class HomePage;
class GaugeCluster {
 public:
    GaugeCluster(sdlcore_message_handler::GuiController* controller, HomePage* home);
    virtual ~GaugeCluster();

    void initGaugeCluster();
    void setSpeed(uint16_t num);
    void setRpm(uint16_t num);
    void setGear(uint8_t gearNum);
    void setRecord(uint32_t tripNum, uint8_t fuelConsNum);
    void setFuelLevel(uint8_t fuelLevelNum);
    void turnOnSignal();
    void turnOffSignal();
    void turnOnHighBeam();
    void turnOffHighBeam();
    void turnOnEngineTrouble();
    void turnOffEngineTrouble();
    void turnOnOilLower();
    void turnOffOilLower();
    void turnOnCoolantTemp();
    void turnOffCoolantTemp();
    void turnOnNeutral();
    void turnOffNeutral();
    void turnOnABS();
    void turnOffABS();

    void demoGaugeCluster();

 private:
    void drawHeader(lv_obj_t* parent);
    void drawGauges(lv_obj_t* parent);
    void drawGear(lv_obj_t* parent, uint8_t gearNum);
    void drawRecord(lv_obj_t* parent, uint32_t tripNum, uint8_t fuelConsNum);
    void drawFuelLevel(lv_obj_t* parent);
    static void specific_back_to_home_event_cb(lv_obj_t *obj, lv_event_t e);

    sdlcore_message_handler::GuiController* controller_;
    HomePage* home_page_;
};

}

#endif // GAUGE_CLUSTER_H_
