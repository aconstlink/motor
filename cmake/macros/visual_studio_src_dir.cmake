
#
# @var_sources pass all files to be processed. the visual studio 
#	will then contain all the file in their respective folder.
#
macro( motor_vs_src_dir var_sources  )
if( MSVC_IDE )
    foreach( F ${${var_sources}} )
        # DEBUG
        #message( STATUS "string = ${F}" )
        string( FIND ${F} "/" POS_BEG REVERSE )
        if( ${POS_BEG} GREATER 0 )
            # DEBUG
            #message( STATUS "beg = " ${POS_BEG} )

            string( SUBSTRING ${F} 0 ${POS_BEG} F_PROC )
            string( REPLACE "/" "\\\\" LST ${F_PROC} )

            # DEBUG
            #message(STATUS "proc = ${F_PROC}")
            
            source_group( ${LST} FILES ${F} )
        endif()
    endforeach()
endif()
endmacro()

