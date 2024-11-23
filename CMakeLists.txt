cmake_minimum_required(VERSION 3.16)
project(mahjong-cpp VERSION 0.9.1)

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
find_package(RapidJSON)
if(RapidJSON_FOUND)
  message(STATUS "Use system provided RapidJSON")
else()
  FetchContent_Declare(
    RapidJSON
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG v1.1.0
  )
  FetchContent_Populate(RapidJSON)
endif()
message(STATUS "RapidJSON: ${RapidJSON_VERSION}")

# spdlog
find_package(spdlog)
if(spdlog_FOUND)
  message(STATUS "Use system provided spdlog")
else()
  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.x
  )
  FetchContent_Populate(spdlog)
endif()
message(STATUS "spdlog: ${spdlog_VERSION}")

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
  target_link_libraries(${LIB_NAME} ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} spdlog::spdlog)
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
