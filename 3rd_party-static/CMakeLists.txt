# --- jsoncpp
include(${CMAKE_SOURCE_DIR}/tools/cmake/helpers/sources.cmake)
set(JSONCPP_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/include ${CMAKE_SOURCE_DIR}/src/components/utils/include)
set(JSONCPP_INCLUDE_DIRECTORY ${JSONCPP_INCLUDE_DIRECTORY} PARENT_SCOPE)

include_directories(
  ${JSONCPP_INCLUDE_DIRECTORY}
)

collect_sources(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/src/lib_json")
add_library(jsoncpp ${SOURCES})
