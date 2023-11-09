message( STATUS ">>> [external : filesystem] >>>" )

set( GHC_FILESYSTEM_WITH_INSTALL ON )


add_subdirectory( "filesystem" )

#install_library( ghc_filesystem ${PROJECT_NAME}-targets )

message( STATUS "<<< [external : filesystem] <<<" )
