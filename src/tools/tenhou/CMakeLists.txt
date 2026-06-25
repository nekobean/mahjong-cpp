include(FetchContent)
find_package(ZLIB QUIET)
if(ZLIB_FOUND)
  set(TENHOU_ZLIB_TARGET ZLIB::ZLIB)
else()
  FetchContent_Declare(
    zlib
    URL https://github.com/madler/zlib/archive/refs/tags/v1.3.1.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(zlib)
  if(TARGET zlibstatic)
    set(TENHOU_ZLIB_TARGET zlibstatic)
  else()
    set(TENHOU_ZLIB_TARGET zlib)
  endif()
endif()

FetchContent_Declare(
  rapidxml
  GIT_REPOSITORY https://github.com/discord/rapidxml.git
  GIT_TAG 2ae4b2888165a393dfb6382168825fddf00c27b9
)
FetchContent_GetProperties(rapidxml)
if(NOT rapidxml_POPULATED)
  FetchContent_Populate(rapidxml)
endif()

set(TENHOU_MJLOG_SRC
    mjlog_event_factory.cpp
    mjlog_parser.cpp
    replay_builder.cpp
    xml_utils.cpp)

add_library(tenhou_mjlog STATIC ${TENHOU_MJLOG_SRC})
target_include_directories(tenhou_mjlog PUBLIC ${CMAKE_SOURCE_DIR}/src
                                                 ${rapidxml_SOURCE_DIR})
target_link_libraries(tenhou_mjlog PUBLIC ${LIB_NAME} ${TENHOU_ZLIB_TARGET})
