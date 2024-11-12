cmake_minimum_required(VERSION 3.5)
project(mahjong-cpp VERSION 0.9.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions("-DPROJECT_NAME=\"${PROJECT_NAME}\"")
add_definitions("-DPROJECT_VERSION=\"${${PROJECT_NAME}_VERSION}\"")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")
  # MSVC の場合、UTF-8 として解釈するように指定する。
  add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

# Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})

option(USE_SYSTEM_LIBS "use all available system-provided libraries." OFF)
option(USE_SYSTEM_RAPIDJSON "use system-provided RapidJSON." OFF)
option(USE_SYSTEM_SPDLOG "use system-provided spdlog." OFF)
option(USE_SYSTEM_CATCH2 "use system-provided Catch2." OFF)
if (USE_SYSTEM_LIBS)
  set(USE_SYSTEM_RAPIDJSON ON)
  set(USE_SYSTEM_SPDLOG ON)
  set(USE_SYSTEM_CATCH2 ON)
else()
  include(FetchContent)
endif()

if (NOT USE_SYSTEM_RAPIDJSON)
  # Fetch rapidjson
  FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG v1.1.0)
  FetchContent_GetProperties(rapidjson)
  if(NOT rapidjson_POPULATED)
    FetchContent_Populate(rapidjson)
  endif()
  include_directories(${rapidjson_SOURCE_DIR}/include)
else()
  find_package(RapidJSON REQUIRED)
endif()

if (NOT USE_SYSTEM_SPDLOG)
  # Fetch spdlog
  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.x)
  FetchContent_GetProperties(spdlog)
  if(NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
  endif()
else()
  find_package(spdlog REQUIRED)
  link_libraries(spdlog::spdlog)
endif()

# mahjong
include_directories(src)

# サーバー
option(BUILD_SERVER "build server programs." ON)
if(BUILD_SERVER)
  add_subdirectory(src/server)
endif()

# テスト
option(BUILD_TEST "build test programs." ON)
if(BUILD_TEST)
  add_subdirectory(src/test)
  add_subdirectory(src/misc/shanten_table)
endif()

# サンプル
option(BUILD_SAMPLES "build sample programs." ON)
if(BUILD_SAMPLES)
  add_subdirectory(src/samples)
endif()
