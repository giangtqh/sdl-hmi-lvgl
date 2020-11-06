include(${CMAKE_SOURCE_DIR}/tools/cmake/helpers/sources.cmake)

set(UTILS_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(UTILS_INCLUDE_DIRECTORY ${UTILS_INCLUDE_DIRECTORY} PARENT_SCOPE)

include_directories(
  ${JSONCPP_INCLUDE_DIRECTORY}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

collect_sources(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(utils ${SOURCES})
