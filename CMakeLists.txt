cmake_minimum_required(VERSION 3.10)
project(lvgl)
set(CMAKE_C_STANDARD 11)#C11
set(CMAKE_CXX_STANDARD 17)#C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXCLUDE_ERROR_FLAGS "-Wno-error=unused-variable -Wno-error=unused-but-set-variable -Wno-error=deprecated-declarations")
set(CMAKE_CXX_FLAGS "-fPIC -std=gnu++0x -Wall -fvisibility=hidden -Werror ${EXCLUDE_ERROR_FLAGS} -Wuninitialized -Wvla")

# --- 3rd party libs (static)
add_subdirectory(./3rd_party-static)
add_subdirectory(./utils)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${JSONCPP_INCLUDE_DIRECTORY}
)

file(GLOB_RECURSE INCLUDES "lv_drivers/*.h" "lvgl/*.h"  "./*.h" )
file(GLOB_RECURSE SOURCES  "lv_drivers/*.c" "gui/*.c" "lvgl/*.c" )

file(GLOB WEBSOCKET_CLIENT_SOURCES "sdl_message_handler/*.cc" )
add_library("websocket_client" ${WEBSOCKET_CLIENT_SOURCES})
SET(LIBRARIES
    pthread
    boost_system
    jsoncpp
    utils
)
target_link_libraries(websocket_client ${LIBRARIES})
#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

find_package(SDL2 REQUIRED SDL2)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(main main.cc ${SOURCES} ${INCLUDES})
target_link_libraries(main PRIVATE SDL2 websocket_client)
add_custom_target (run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main)
