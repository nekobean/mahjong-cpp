cmake_minimum_required(VERSION 3.1.1)
project(mahjong-cpp VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  # MSVC の場合、UTF-8 として解釈するように指定する。
  add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

# 外部ライブラリ
add_subdirectory(lib/spdlog)
include_directories(lib/rapidjson/include)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS
  system
  filesystem
)
include_directories(${Boost_INCLUDE_DIRS} src)

# サーバー
option(BUILD_SERVER "build server programs." ON)
if(BUILD_SERVER)
  add_subdirectory(src/server)
endif()

# テスト
option(BUILD_TEST "build test programs." ON)
if(BUILD_TEST)
  add_subdirectory(src/test)
endif()

# サンプル
option(BUILD_SAMPLES "build sample programs." ON)
if(BUILD_SAMPLES)
  add_subdirectory(src/samples)
endif()
