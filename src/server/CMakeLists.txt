file(GLOB_RECURSE SRC_FILES ../mahjong/*.cpp)

add_executable(server ${SRC_FILES} server.cpp)
target_link_libraries(server ${Boost_LIBRARIES} spdlog ${CMAKE_DL_LIBS})

add_custom_command(TARGET server PRE_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/data/config/ $<TARGET_FILE_DIR:server>)
