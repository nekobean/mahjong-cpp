

file(GLOB_RECURSE SRC_FILES ../../src/mahjong/*.cpp)

FetchContent_Declare(
    cppitertools
GIT_REPOSITORY https://github.com/ryanhaining/cppitertools)
FetchContent_GetProperties(cppitertools)
if(NOT cppitertools_POPULATED)
FetchContent_Populate(cppitertools)
add_subdirectory(${cppitertools_SOURCE_DIR} ${cppitertools_BINARY_DIR})
INCLUDE_DIRECTORIES(${cppitertools_SOURCE_DIR})
endif()

set(CMAKE_CONFIG_DIR ${CMAKE_SOURCE_DIR}/data/config)
add_definitions("-DCMAKE_CONFIG_DIR=\"${CMAKE_CONFIG_DIR}\"")

# create_distance_table
add_executable(create_distance_table ${SRC_FILES} create_distance_table.cpp)
target_link_libraries(create_distance_table ${Boost_LIBRARIES} spdlog ${CMAKE_DL_LIBS})

add_custom_command(TARGET create_distance_table PRE_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/data/config/ $<TARGET_FILE_DIR:create_distance_table>)

# create_uradora_table
add_executable(create_uradora_table ${SRC_FILES} create_uradora_table.cpp)
target_link_libraries(create_uradora_table ${Boost_LIBRARIES} spdlog ${CMAKE_DL_LIBS})
add_custom_command(TARGET create_uradora_table PRE_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/data/config/ $<TARGET_FILE_DIR:create_uradora_table>)
