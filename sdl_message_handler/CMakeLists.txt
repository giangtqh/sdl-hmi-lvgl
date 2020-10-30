include(${CMAKE_SOURCE_DIR}/tools/cmake/helpers/sources.cmake)

set(SDL_MESSAGE_HANLDER_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SDL_MESSAGE_HANLDER_INCLUDE_DIRECTORY ${SDL_MESSAGE_HANLDER_INCLUDE_DIRECTORY} PARENT_SCOPE)

include_directories(
  ${JSONCPP_INCLUDE_DIRECTORY}
  ${BASE_DIR}/utils/include
  ${BASE_DIR}/gui/include
  ${SDL_MESSAGE_HANLDER_INCLUDE_DIRECTORY}
)

collect_sources(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library("websocket_client" ${SOURCES})
SET(LIBRARIES
    pthread
    boost_system
    jsoncpp
    utils
)
target_link_libraries(websocket_client ${LIBRARIES})
