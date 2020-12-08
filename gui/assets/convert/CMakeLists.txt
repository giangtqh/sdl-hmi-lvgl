include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lvgl
)

file(GLOB ASSETS_SRCS  "*.c" )
add_library("assets" ${ASSETS_SRCS} )
