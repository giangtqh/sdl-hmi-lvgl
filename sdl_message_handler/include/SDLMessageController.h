#ifndef _SDL_MESSAGE_CONTROLLER_
#define _SDL_MESSAGE_CONTROLLER_

#include <atomic>
#include <memory>
#include <string>
#include "json/json.h"

#include "ApplicationInfo.h"

namespace sdlcore_message_handler {

class SDLMessageControllerImpl;
class GuiCallbacks;

class SDLMessageController {
 public:
    SDLMessageController(GuiCallbacks*);
    virtual ~SDLMessageController();

    enum class EventType : uint32_t {
        PHONE_CALL,
        EMERGENCY_EVENT,
        DEACTIVATE_HMI,
        AUDIO_SOURCE,
        EMBEDDED_NAVI
    };

    /**
     * @brief Shutdown
     */
    void shutdown(void);

    /**
     * @brief Start
     */
    void start(void);

    // void sendJsonMessage(const Json::Value& msg);
    /**
     * @brief Inform SDL that the user has chosen to activate an application.
     * @param[in] appId Application Id
     */
    void activateApplication(uint32_t appId);

    /**
     * @brief Inform the application it is no longer active on the HMI
     * @param[in] appId Application Id
     */
    void deactivateApplication(uint32_t appId);

    /**
     * @brief Inform SDL to exit every registered application before reload or shutting down
     * @param[in] reason Reason
     */
    void exitAllApplications(const std::string& reason);

    // /**
    //  * @brief Request to remove the application from the foreground
    //  * @param[in] appId Application Id
    //  */
    // int closeApplication(uint32_t appId);

    /**
     * @brief HMI triggers searching for devices when user switch to ManageDevices screen
     */
    void startDeviceDiscovery(void);

    /**
     * @brief Open the connection with the named device
     * @param[in] appId Application Id
     */
    void deviceChosen(const Device& device);

    // /**
    //  * @brief Notify SDL that an event becomes active or inactive
    //  * @param[in] type EventType
    //  */
    // void onEventChanged(const EventType& type);

    /**
     * @brief Initiate the search for applications on the named device.
     * @param[in] device Device information
     */
    void findApplications(const Device& device);

    /**
     * @brief Ask for the updated list of connected devices.
     */
    void updateDeviceList(void);

    /**
     * @brief Inform SDL that a touch event has occurred in the HMI.
     * @param[in] device Device information
     */
    // void onTouchEvent(TouchType, TouchEvent);

 private:
    std::atomic_bool mShutdown;
    std::shared_ptr<SDLMessageControllerImpl> mImpl;
};

}

#endif  // _SDL_MESSAGE_CONTROLLER_