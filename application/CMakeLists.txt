

set( sources 
    api.h
    result.h
    typedefs.h

    gl_info.h
    d3d_info.h

    window/iwindow.h
    window/window.h
    window/window.cpp
    window/window_info.h
    window/window_message.h
    window/iwindow_message_listener.h
    window/window_message_listener.h
    window/window_message_listener.cpp
    
    #platform/gfx_context.h
    #platform/platform_application.h
    #platform/platform_application.cpp
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} )
target_compile_options( ${cur_lib_name} PUBLIC ${MOTOR_WINDOW_SYSTEM_DEFINES} )

target_link_libraries( ${cur_lib_name}
    PUBLIC motor::concurrent
    PUBLIC motor::log
    PUBLIC motor::memory
) 



###########################################################
# SECTION: Build Tree
###########################################################

#motor_export( ${CUR_LIB_NAME} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )