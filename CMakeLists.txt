cmake_minimum_required(VERSION 3.0.0)
project(mahjong-cpp VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC の場合、UTF-8 として解釈するように指定する。
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 外部ライブラリ
add_subdirectory(lib/spdlog)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS
  system
  filesystem
)
include_directories(${Boost_INCLUDE_DIRS} src/core)

file(GLOB_RECURSE SRC_FILES src/core/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES} src/main.cpp)
target_link_libraries(${PROJECT_NAME}
  spdlog
  ${Boost_LIBRARIES}
)

# 必要なファイルをビルドディレクトリにコピーする。
configure_file(data/shuupai_table.txt ${CMAKE_BINARY_DIR}/Release)
configure_file(data/zihai_table.txt ${CMAKE_BINARY_DIR}/Release)
configure_file(data/test_syanten.txt ${CMAKE_BINARY_DIR}/Release)

# テスト
option(BUILD_TEST "build test program." ON)
if(BUILD_TEST)
  add_executable(test_syanten ${SRC_FILES} src/test/test_syanten.cpp)
  target_link_libraries(test_syanten
    spdlog
    ${Boost_LIBRARIES}
  )
endif()
