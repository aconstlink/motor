message( STATUS ">>> [external : json] >>>" )

add_subdirectory( json ) 

export(
        TARGETS nlohmann_json
        NAMESPACE ${PROJECT_NAME}::
        APPEND FILE ${MOTOR_BINARY_DIR}/${PROJECT_NAME}-external-targets.cmake
    )

install_library( nlohmann_json ${PROJECT_NAME}-targets )

if( NOT JSON_Install )
    if( JSON_MultipleHeaders )
        install( DIRECTORY json/include/nlohmann DESTINATION include )
    else()
        install( DIRECTORY json/single_include/nlohmann DESTINATION include )
    endif()
    if( MSVC )
        install( FILES json/nlohmann_json.natvis DESTINATION . )
    endif()
endif()

message( STATUS "<<< [external : json] <<<" )
