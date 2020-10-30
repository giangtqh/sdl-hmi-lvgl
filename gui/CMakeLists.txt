include(${CMAKE_SOURCE_DIR}/tools/cmake/helpers/sources.cmake)

set(GUI_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(GUI_INCLUDE_DIRECTORY ${GUI_INCLUDE_DIRECTORY} PARENT_SCOPE)

include_directories(
  ${JSONCPP_INCLUDE_DIRECTORY}
  ${GUI_INCLUDE_DIRECTORY}
  ${BASE_DIR}/utils/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${SDL_MESSAGE_HANLDER_INCLUDE_DIRECTORY}
)

file(GLOB_RECURSE INCLUDES "src/lv_drivers/*.h" "src/lvgl/*.h"  "src/*.h" )
file(GLOB_RECURSE SOURCES  "src/lv_drivers/*.c" "src/*.c" "src/*.cc" "src/lvgl/*.c" )

find_package(SDL2 REQUIRED SDL2)
include_directories(${SDL2_INCLUDE_DIRS})

add_library("gui" ${SOURCES} ${INCLUDES})
target_link_libraries(gui PRIVATE SDL2)
