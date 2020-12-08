cmake_minimum_required(VERSION 3.10)
project(sdl2w)

option(USE_ASAN "Enable Address Sanitize tool" OFF)

set(CMAKE_C_STANDARD 11)#C11
set(CMAKE_CXX_STANDARD 17)#C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BASE_DIR ${CMAKE_SOURCE_DIR})

set(EXCLUDE_ERROR_FLAGS "-Wno-error=unused-variable -Wno-error=unused-but-set-variable -Wno-error=deprecated-declarations")
set(CMAKE_CXX_FLAGS "-fPIC -std=gnu++0x -Wall -fvisibility=hidden -Werror ${EXCLUDE_ERROR_FLAGS} -Wuninitialized -Wvla")

if(USE_ASAN)
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -static-libasan")
 message(STATUS "Address Sanitize enabled")
endif()

# --- 3rd party libs (static)
add_subdirectory(./3rd_party-static)
add_subdirectory(./utils)
add_subdirectory(./sdl_message_handler)
add_subdirectory(./gui)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${JSONCPP_INCLUDE_DIRECTORY}
    ${GUI_INCLUDE_DIRECTORY}
    ${UTILS_INCLUDE_DIRECTORY}
    ${SDL_MESSAGE_HANLDER_INCLUDE_DIRECTORY}
)

SET(LIBRARIES
    websocket_client
    gui
)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

add_executable(main main.cc)
target_link_libraries(main ${LIBRARIES})
add_custom_target (run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main)
