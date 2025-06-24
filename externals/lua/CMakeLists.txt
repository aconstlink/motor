message( STATUS ">>> [external : Lua] >>>" )

set( LUA_BUILD_TYPE STATIC )

if( MOTOR_TARGET_OS_WIN )
  option( LUA_BUILD_AS_DLL "Build lua as dynamically linked library" OFF )
else()
  set( LUA_BUILD_AS_DLL OFF )
endif()

file( GLOB INCLUDES
  "lua/*.h" )

file( GLOB SOURCES
  "lua/*.c" )

list( REMOVE_ITEM SOURCES "${CMAKE_CURRENT_LIST_DIR}/lua/onelua.c")

set( SOURCES 
    ${SOURCES} 
    ${INCLUDES}
)

if( LUA_BUILD_AS_DLL )    
    set( LUA_BUILD_TYPE SHARED )
endif()

add_library( lua ${LUA_BUILD_TYPE} ${SOURCES} )
set_target_properties( lua PROPERTIES LINKER_LANGUAGE C )
if( MSVC )
  target_compile_definitions( lua PUBLIC "/wd4244 /wd4324 /wd4310" )
endif()

target_include_directories( lua
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
  PUBLIC $<INSTALL_INTERFACE:"include/${PROJECT_NAME}/externals"> ) 

if( LUA_BUILD_AS_DLL )
    target_compile_options( lua PUBLIC -DLUA_BUILD_AS_DLL -DLUA_USE_LONGJMP) 
endif()

if( MOTOR_TARGET_OS_WIN )
  # this is defined in the lua header itself
  #target_compile_options( lua PUBLIC -DLUA_USE_WINDOWS )
elseif( MOTOR_TARGET_OS_LIN )
  target_compile_options( lua PUBLIC -DLUA_USE_LINUX )
  target_link_libraries( lua PUBLIC m dl )
endif()
add_executable( onelua ${INCLUDES} "lua/onelua.c" )
target_link_libraries( onelua PRIVATE lua )
set_target_properties( onelua PROPERTIES LINKER_LANGUAGE C )

unset( LUA_BUILD_AS_DLL CACHE )
set_target_properties( lua PROPERTIES FOLDER "motor/externals/lua" )
set_target_properties( onelua PROPERTIES FOLDER "motor/externals/lua" )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${SOURCES}" "include/lua" )
install_library( lua ${PROJECT_NAME}-targets )

message( STATUS "<<< [external : Lua] <<<" )
