file(GLOB ENTRY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
foreach(ENTRY_FILE ${ENTRY_FILES})
  get_filename_component(BASE_NAME ${ENTRY_FILE} NAME_WE)

  add_executable(${BASE_NAME} ${ENTRY_FILE})
  if (MSVC)
    target_link_libraries(${BASE_NAME} ${LIB_NAME} ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} spdlog)
  else()
    target_link_libraries(${BASE_NAME} ${LIB_NAME} ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} spdlog::spdlog -static-libgcc -static-libstdc++ -static)
  endif()
  add_dependencies(${BASE_NAME} ${LIB_NAME})
  add_custom_command(TARGET ${BASE_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_directory
                     ${CMAKE_SOURCE_DIR}/data/config/ $<TARGET_FILE_DIR:${BASE_NAME}>)
  install(TARGETS ${BASE_NAME})
endforeach(ENTRY_FILE ${ENTRY_FILES})
