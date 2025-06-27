message( STATUS ">>> [external : json] >>>" )

add_subdirectory( json ) 
install_library( nlohmann_json ${PROJECT_NAME}-targets )

message( STATUS "<<< [external : json] <<<" )
