
function( install_headers FILE_LIST BASE_PATH )
    #message( "Will install to : " ${BASE_PATH} )
    #message( "Will install : " ${FILE_LIST} )

    foreach( FILE_NAME ${FILE_LIST} )

        file( REAL_PATH ${FILE_NAME} P )
        file( RELATIVE_PATH P ${CMAKE_CURRENT_LIST_DIR} ${P} )

        get_filename_component( FNAME ${P} NAME )
        get_filename_component( FPATH ${P} DIRECTORY )

        #message( "testing ${FNAME}" )

        # all .h, .hxx, .hpp and "no ext" files
        # \\ escape sequence
        if( ${FNAME} MATCHES "(\\.h$)|(\\.hpp$)|(^[A-Za-z0-9_]*$)" )
            #message( "${FNAME} will install to : " "${BASE_PATH}/${FPATH}" )
            install( FILES ${FILE_NAME} DESTINATION "${BASE_PATH}/${FPATH}" )
        endif()
    endforeach() 

endfunction()
