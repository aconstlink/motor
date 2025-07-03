message( STATUS ">>> [external : ImNodes] >>>" )

if( MSVC )
#set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4244 /wd4324 /wd4310" )
endif()

set( INCLUDES 
  "imnodes_config.h"
  "imnodes/imnodes.h"
  "imnodes/imnodes_internal.h"
)

set( SOURCES
  ${INCLUDES}
    "imnodes/imnodes.cpp"
)

#add_definitions( -DIMGUI_DEFINE_MATH_OPERATORS )

add_library( imnodes ${MOTOR_LIBRARY_BUILD_TYPE} ${SOURCES} )

# fix the dll-hell. The self provided imconfig.h 
# sets the IMGUI_API properly for dll export/import
target_compile_definitions( imnodes 
  PUBLIC -DIMNODES_USER_CONFIG="../imnodes_config.h"
  #PUBLIC -DIMGUI_DISABLE_INCLUDE_IMCONFIG_H
)

if( MOTOR_LIBRARY_BUILD_SHARED )
  target_compile_options( imnodes
    PRIVATE -DMOTOR_IMNODES_EXPORT 
    INTERFACE -DMOTOR_IMNODES_IMPORT )
endif()

target_link_libraries( imnodes 
    PUBLIC imgui
    PRIVATE ct_motor_build_library_type )

set_target_properties( imnodes PROPERTIES LINKER_LANGUAGE CXX )
set_target_properties( imnodes PROPERTIES FOLDER "motor/externals/imgui" )
target_include_directories( imnodes 
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/imnodes>
  PUBLIC $<INSTALL_INTERFACE:"include/imnodes"> ) 

export( TARGETS imnodes 
  NAMESPACE ${PROJECT_NAME}::
  APPEND FILE ${MOTOR_BINARY_DIR}/${PROJECT_NAME}-external-targets.cmake)

set_target_properties( imnodes PROPERTIES POSITION_INDEPENDENT_CODE True )
if( MOTOR_COMPILER_GNU )
  target_compile_options( imnodes PUBLIC -DfPIC )
endif()

message( STATUS "[ImNodes] : Target available : imnodes" )

###########################################################
# SECTION: Install
###########################################################

#set( FOLDER_NAME "${PROJECT_NAME}-${IMGUI_MAJOR_VERSION}.${IMGUI_MINOR_VERSION}.${IMGUI_PATCH_VERSION}")
#set( BASE_INSTALL_DIR "${IMGUI_INSTALL_HEADER_BASE}/${FOLDER_NAME}" )

install_headers( "${INCLUDES}" "include/imnodes" )
install_library( imnodes ${PROJECT_NAME}-targets )

message( STATUS "<<< [external : ImNodes] <<< " )
