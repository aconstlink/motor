message( STATUS ">>> [external : ImGui] >>>" )

if( MSVC )
#set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4244 /wd4324 /wd4310" )
endif()

set( INCLUDES 
  "imconfig.h"
  "imgui/imgui.h"
  "imgui/imgui_internal.h"
  "imgui/imstb_rectpack.h"
  "imgui/imstb_textedit.h"
  "imgui/imstb_truetype.h"
)

set( SOURCES
  ${INCLUDES}
    "imgui/imgui.cpp"
    "imgui/imgui_demo.cpp"
    "imgui/imgui_draw.cpp"
    "imgui/imgui_widgets.cpp"
    "imgui/imgui_tables.cpp"
)

#add_definitions( -DIMGUI_DEFINE_MATH_OPERATORS )

add_library( imgui ${MOTOR_LIBRARY_BUILD_TYPE} ${SOURCES} )

# fix the dll-hell. The self provided imconfig.h 
# sets the IMGUI_API properly for dll export/import
target_compile_definitions( imgui 
  PUBLIC -DIMGUI_USER_CONFIG="../imconfig.h"
  PUBLIC -DIMGUI_DISABLE_INCLUDE_IMCONFIG_H
  PUBLIC -DIMGUI_DISABLE_OBSOLETE_KEYIO
  PUBLIC -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS
)

if( MOTOR_LIBRARY_BUILD_SHARED )
  target_compile_options( imgui
    PRIVATE -DMOTOR_IMGUI_EXPORT 
    INTERFACE -DMOTOR_IMGUI_IMPORT )
endif()

target_link_libraries( imgui 
    PRIVATE ct_motor_build_library_type )

set_target_properties( imgui PROPERTIES LINKER_LANGUAGE CXX )
set_target_properties( imgui PROPERTIES FOLDER "motor/externals/imgui" )
target_include_directories( imgui 
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/imgui>
  PUBLIC $<INSTALL_INTERFACE:"include/imgui"> ) 

export( TARGETS imgui APPEND FILE ${PROJECT_NAME}-targets.cmake)

set_target_properties( imgui PROPERTIES POSITION_INDEPENDENT_CODE True )
if( MOTOR_COMPILER_GNU )
  target_compile_options( imgui PUBLIC -DfPIC )
endif()

message( STATUS "[ImGui] : Target available : imgui" )

###########################################################
# SECTION: Install
###########################################################

#set( FOLDER_NAME "${PROJECT_NAME}-${IMGUI_MAJOR_VERSION}.${IMGUI_MINOR_VERSION}.${IMGUI_PATCH_VERSION}")
#set( BASE_INSTALL_DIR "${IMGUI_INSTALL_HEADER_BASE}/${FOLDER_NAME}" )

install_headers( "${INCLUDES}" "include/imgui" )
install_library( imgui ${PROJECT_NAME}-targets )

message( STATUS "<<< [external : ImGui] <<< " )
