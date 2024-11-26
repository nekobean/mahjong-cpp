set(EXE_NAME nanikiru)
file(GLOB_RECURSE SRC_FILES *.cpp)
add_executable(${EXE_NAME} ${SRC_FILES})
add_dependencies(${EXE_NAME} ${LIB_NAME})

if (MSVC)
    target_link_libraries(${EXE_NAME} ${LIB_NAME} ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} spdlog)
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra -fsplit-stack")
    target_link_libraries(${EXE_NAME} ${LIB_NAME} ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} spdlog::spdlog -static-libgcc -static-libstdc++ -static)
endif()

add_custom_command(TARGET ${EXE_NAME} PRE_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/data/config/ $<TARGET_FILE_DIR:nanikiru>)

install(TARGETS ${EXE_NAME})
