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
     * DEPRECATED
     * @brief Open the connection with the named device
     * @param[in] device Device structure
     */
    void deviceChosen(const Device& device);

    /**
     * @brief Open the connection with the named device
     * @param[in] device Device structure
     */
    void deviceChosenId(const std::string& deviceId);

    // /**
    //  * @brief Notify SDL that an event becomes active or inactive
    //  * @param[in] type EventType
    //  */
    // void onEventChanged(const EventType& type);

    /**
     * @brief Ask for the updated list of connected devices.
     */
    void updateDeviceList(void);

    /**
     * @brief Inform SDL that a touch event has occurred in the button.
     * @param[in] customButtonID 	customButton ID
     * @param[in] appID appID
     */
    // void onTouchEvent(TouchType, TouchEvent);
    void onButtonPress(const uint32_t customButtonID, const uint32_t appID);
    void onListItemSelected(const uint32_t cmdId, const uint32_t appID);

    /** DEPRECATED
     * @brief Inform SDL about the occurrence of a button event.
     * @param[in] name 	Button Name
     * @param[in] mode 	Button Event Mode
     * @param[in] customButtonID 	customButton ID
     * @param[in] appID appID
     */
    void onButtonEvent(const std::string& name, const ButtonEventMode mode, const uint32_t customButtonID, const uint32_t appID);

    /** DEPRECATED
     * @brief Inform SDL about a Button Press.
     * If the HMI reports to SDL via Buttons.GetCapabilities that it supports long and/or short button press modes, SDL expects
     * the HMI to send the Buttons.OnButtonPress notification but buttons that have been subscribed via Buttons.OnButtonSubscription
     * and custom buttons added in other RPCs as Soft Buttons.
     * @param[in] name 	Button Name
     * @param[in] mode 	Button Event Mode
     * @param[in] customButtonID 	customButton ID
     * @param[in] appID appID
     */
    void onButtonPress(const std::string& name, const ButtonPressMode mode, const uint32_t customButtonID, const uint32_t appID);

 private:
    std::atomic_bool mShutdown;
    std::shared_ptr<SDLMessageControllerImpl> mImpl;
};

}

#endif  // _SDL_MESSAGE_CONTROLLER_
