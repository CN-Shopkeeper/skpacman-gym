macro(CopyResources target_name)
    if(WIN32)
        add_custom_command(
            TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:${target_name}>/resources)
    endif()
endmacro(CopyResources)