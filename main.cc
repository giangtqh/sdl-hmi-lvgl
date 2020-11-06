
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>

#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" \
                            issue*/
#include "GuiController.h"
#include "SDLMessageController.h"

#include "json/json.h"
#include <memory>

/*********************
 *      DEFINES
 *********************/
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
using namespace sdlcore_message_handler;
int main(int argc, char **argv)
{
    (void)argc; /*Unused*/
    (void)argv; /*Unused*/

    GuiController* gui = new GuiController();
    SDLMessageController* sdlMsgController = new SDLMessageController(gui);
    gui->setMsgController(sdlMsgController);
    sdlMsgController->start();
    gui->initDisplay();
    gui->waitForExit();

    if (sdlMsgController) {
        sdlMsgController->shutdown();
        delete sdlMsgController;
    }
    if (gui) {
        gui->shutdown();
        delete gui;
    }
    // TODO: Add signal handler for proper shutdown

    return 0;
}
