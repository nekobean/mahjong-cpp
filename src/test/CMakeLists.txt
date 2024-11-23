# Catch2
find_package(Catch2 QUIET)
if(Catch2_FOUND)
  message(STATUS "Use system provided Catch2")
else()
  include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.13.1)
    FetchContent_MakeAvailable(Catch2)
  message(STATUS "Use fetched Catch2")
endif()
message(STATUS "Catch2: ${spdlog_VERSION}")

#file(GLOB_RECURSE SRC_FILES ../mahjong/*.cpp ../compare/*.cpp ../server/json_parser.cpp)
file(GLOB_RECURSE SRC_FILES ../server/json_parser.cpp)
set(CMAKE_TESTCASE_DIR ${CMAKE_SOURCE_DIR}/data/testcase)
add_definitions("-DCMAKE_TESTCASE_DIR=\"${CMAKE_TESTCASE_DIR}\"")

file(GLOB ENTRY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp)
foreach(ENTRY_FILE ${ENTRY_FILES})
  get_filename_component(EXE_NAME ${ENTRY_FILE} NAME_WE)
  add_executable(${EXE_NAME} ${SRC_FILES} ${ENTRY_FILE})
  add_dependencies(${EXE_NAME} ${LIB_NAME})
  if (MSVC)
    target_link_libraries(${EXE_NAME} ${LIB_NAME} ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} spdlog Catch2)
  else()
    target_link_libraries(${EXE_NAME} ${LIB_NAME} ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} spdlog::spdlog Catch2::Catch2 -static-libgcc -static-libstdc++)
  endif()
  add_custom_command(TARGET ${EXE_NAME} PRE_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_directory
                     ${CMAKE_SOURCE_DIR}/data/config/ $<TARGET_FILE_DIR:${EXE_NAME}>)
endforeach(ENTRY_FILE ${ENTRY_FILES})
