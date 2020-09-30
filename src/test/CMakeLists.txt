Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.1)
FetchContent_MakeAvailable(Catch2)

file(GLOB_RECURSE SRC_FILES ../core/*.cpp)
add_executable(test_syanten ${SRC_FILES} test_syanten.cpp)
target_link_libraries(test_syanten Catch2::Catch2 ${Boost_LIBRARIES} spdlog)

file(GLOB_RECURSE SRC_FILES ../core/*.cpp)
add_executable(test_score ${SRC_FILES} test_score.cpp)
target_link_libraries(test_score Catch2::Catch2 ${Boost_LIBRARIES} spdlog)

# 必要なファイルをビルドディレクトリにコピーする。
configure_file(${CMAKE_SOURCE_DIR}/data/shuupai_table.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)
configure_file(${CMAKE_SOURCE_DIR}/data/zihai_table.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)
configure_file(${CMAKE_SOURCE_DIR}/data/test_syanten.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)
configure_file(${CMAKE_SOURCE_DIR}/data/test_ryuiso.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)
configure_file(${CMAKE_SOURCE_DIR}/data/test_tyurenpoto.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)
configure_file(${CMAKE_SOURCE_DIR}/data/test_tyurenpoto9.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)
configure_file(${CMAKE_SOURCE_DIR}/data/test_daisangen.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)
configure_file(${CMAKE_SOURCE_DIR}/data/test_syosusi.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)
configure_file(${CMAKE_SOURCE_DIR}/data/test_daisusi.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)
configure_file(${CMAKE_SOURCE_DIR}/data/test_tuiso.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)
configure_file(${CMAKE_SOURCE_DIR}/data/test_suanko.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)
configure_file(${CMAKE_SOURCE_DIR}/data/test_tinroto.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)
configure_file(${CMAKE_SOURCE_DIR}/data/test_suanko_tanki.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)
configure_file(${CMAKE_SOURCE_DIR}/data/test_kokusi13.txt ${CMAKE_CURRENT_BINARY_DIR}/Release)