macro(CopyResources target_name)
    if(WIN32)
        add_custom_command(
            TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:${target_name}>/resources)
    endif()
endmacro(CopyResources)

macro(CopyResForWASM target_name)
    add_custom_command(
        TARGET ${target_name} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources)
endmacro()

macro(CopyResForWASM target_name)
    add_custom_command(
        TARGET ${target_name} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources)
endmacro()

macro(CopyDefScript target_name)
    add_custom_command(
        TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/luabind/defs.lua $<TARGET_FILE_DIR:${target_name}>
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/luabind/defs.lua ${CMAKE_SOURCE_DIR}/resources/script)
endmacro()