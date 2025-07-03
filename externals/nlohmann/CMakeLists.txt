message( STATUS ">>> [external : json] >>>" )

add_subdirectory( json ) 

export(
        TARGETS nlohmann_json
        NAMESPACE ${PROJECT_NAME}::
        APPEND FILE ${MOTOR_BINARY_DIR}/${PROJECT_NAME}-external-targets.cmake
    )

install_library( nlohmann_json ${PROJECT_NAME}-targets )

message( STATUS "<<< [external : json] <<<" )
