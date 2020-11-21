cmake_minimum_required(VERSION 3.0.0)
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
include_directories(${Boost_INCLUDE_DIRS} src/core)

file(GLOB_RECURSE SRC_FILES src/core/*.cpp)
#add_executable(${PROJECT_NAME} ${SRC_FILES} src/main.cpp)
#target_link_libraries(${PROJECT_NAME} spdlog ${Boost_LIBRARIES})

# 必要なファイルをビルドディレクトリにコピーする。
#add_custom_command(TARGET sample_calculate_syanten PRE_BUILD
#                   COMMAND ${CMAKE_COMMAND} -E copy_directory
#                   ${CMAKE_SOURCE_DIR}/data/config/ $<TARGET_FILE_DIR:sample_calculate_syanten>)

# テスト
option(BUILD_TEST "build test programs." ON)
if(BUILD_TEST)
  add_subdirectory(src/test)
endif()

option(BUILD_SAMPLES "build sample programs." ON)
if(BUILD_SAMPLES)
  add_subdirectory(src/samples)
endif()
