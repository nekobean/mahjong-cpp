# Catch2
find_package(Catch2 QUIET)
if(Catch2_FOUND)
  message(STATUS "Use system provided Catch2")
  message(STATUS "Catch2: ${Catch2_VERSION}")
else()
  include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.13.1)
    FetchContent_MakeAvailable(Catch2)
  message(STATUS "Use fetched Catch2")
endif()

file(GLOB_RECURSE SRC_FILES ../mahjong/*.cpp ../compare/*.cpp ../server/json_parser.cpp ../server/request_processor.cpp ../server/server.cpp)
set(CMAKE_TESTCASE_DIR ${CMAKE_SOURCE_DIR}/data/testcase)
add_definitions("-DCMAKE_TESTCASE_DIR=\"${CMAKE_TESTCASE_DIR}\"")

add_custom_target(copy_test_config
                  COMMAND ${CMAKE_COMMAND} -E copy_directory
                  ${CMAKE_SOURCE_DIR}/data/config/
                  ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)

file(GLOB_RECURSE ENTRY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp)
foreach(ENTRY_FILE ${ENTRY_FILES})
  get_filename_component(EXE_NAME ${ENTRY_FILE} NAME_WE)
  add_executable(${EXE_NAME} ${SRC_FILES} ${ENTRY_FILE})
  add_dependencies(${EXE_NAME} ${LIB_NAME} copy_test_config)
  target_compile_definitions(${EXE_NAME} PRIVATE MAHJONG_CPP_DISABLE_SERVER_MAIN)
  if (MSVC)
    target_link_libraries(${EXE_NAME} ${LIB_NAME} ${CMAKE_DL_LIBS}
                          Boost::filesystem Boost::system spdlog Catch2)
  else()
    target_link_libraries(${EXE_NAME} ${LIB_NAME} ${CMAKE_DL_LIBS}
                          Boost::filesystem Boost::system spdlog::spdlog
                          Catch2::Catch2 -static-libgcc -static-libstdc++ -static)
  endif()
endforeach(ENTRY_FILE ${ENTRY_FILES})
