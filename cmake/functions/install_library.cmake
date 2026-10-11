# SPDX-FileCopyrightText: 2023-2026 Alex Link
# SPDX-License-Identifier: MIT


function( install_library LIB_NAME TARGETS_NAME )

    #message( STATUS "install_library : " ${LIB_NAME} )
    #message( STATUS "at : " ${TARGETS_NAME} )
    install( TARGETS ${LIB_NAME} 
        EXPORT ${TARGETS_NAME}
        RUNTIME DESTINATION bin 
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )

endfunction()
