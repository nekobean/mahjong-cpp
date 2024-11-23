cmake_minimum_required(VERSION 3.16)
project(mahjong-cpp VERSION 0.9.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions("-DPROJECT_NAME=\"${PROJECT_NAME}\"")
add_definitions("-DPROJECT_VERSION=\"${${PROJECT_NAME}_VERSION}\"")

option(LANG_EN "Enable English language support" OFF)
if(LANG_EN)
  add_definitions(-DLANG_EN)
  message(STATUS "Enable English language support")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")
  add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

# Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIRS})

# RapidJSON
include(FetchContent)
find_package(RapidJSON QUIET)
if(RapidJSON_FOUND)
  message(STATUS "Use system provided RapidJSON")
  message(STATUS "RapidJSON: ${RapidJSON_VERSION}")
else()
  FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG v1.1.0
  )
  FetchContent_Populate(rapidjson)
  include_directories(${rapidjson_SOURCE_DIR}/include)
endif()

# spdlog
find_package(spdlog QUIET)
if(spdlog_FOUND)
  message(STATUS "Use system provided spdlog")
  message(STATUS "spdlog: ${spdlog_VERSION}")
else()
  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.x
  )
  FetchContent_MakeAvailable(spdlog)
  message(STATUS "Use fetched spdlog")
endif()

option(BUILD_SERVER "build server programs." ON)
option(BUILD_TEST "build test programs." OFF)
option(BUILD_SAMPLES "build sample programs." ON)

# Build mahjong-cpp

# Set stack size to 100MB.
if (MSVC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:104857600")
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,104857600")
endif()

include_directories(src)

# Define variables
set(LIB_NAME mahjong-cpp)

file(GLOB_RECURSE SRC_FILES src/mahjong/*.cpp)
add_library(${LIB_NAME} ${SRC_FILES})
if (MSVC)
  target_link_libraries(${LIB_NAME} ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} spdlog)
else()
  target_link_libraries(${LIB_NAME} ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} spdlog::spdlog -static-libgcc -static-libstdc++)
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE)
endif()
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/config/ DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# server program
if(BUILD_SERVER)
  add_subdirectory(src/server)
endif()

# test programs
if(BUILD_TEST)
  add_subdirectory(src/test)
  add_subdirectory(src/misc/shanten_table)
endif()

# sample programs
if(BUILD_SAMPLES)
  add_subdirectory(src/samples)
endif()
