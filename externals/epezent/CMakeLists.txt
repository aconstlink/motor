message( STATUS ">>> [external : ImPlot] >>>" )

if( MSVC )
#set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4244 /wd4324 /wd4310" )
endif()

set( INCLUDES 
  #"imconfig.h"
  "implot/implot.h"
  "implot/implot_internal.h"
)

set( SOURCES
  ${INCLUDES}
    "implot/implot.cpp"
    "implot/implot_demo.cpp"
    "implot/implot_items.cpp"
)

#add_definitions( -DIMGUI_DEFINE_MATH_OPERATORS )

add_library( implot ${MOTOR_LIBRARY_BUILD_TYPE} ${SOURCES} )

if( MOTOR_LIBRARY_BUILD_SHARED )
  target_compile_options( implot
    PRIVATE -DMOTOR_IMPLOT_EXPORT 
    INTERFACE -DMOTOR_IMPLOT_IMPORT )
endif()

target_link_libraries( implot 
    PUBLIC imgui
    PRIVATE ct_motor_build_library_type )

set_target_properties( implot PROPERTIES LINKER_LANGUAGE CXX )
set_target_properties( implot PROPERTIES FOLDER "motor/externals/imgui" )
target_include_directories( implot 
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/implot>
  PUBLIC $<INSTALL_INTERFACE:"include/implot"> ) 

export( TARGETS implot APPEND FILE ${PROJECT_NAME}-targets.cmake)

set_target_properties( implot PROPERTIES POSITION_INDEPENDENT_CODE True )
if( MOTOR_COMPILER_GNU )
  target_compile_options( implot PUBLIC -DfPIC )
endif()

message( STATUS "[ImPlot] : Target available : implot" )

###########################################################
# SECTION: Install
###########################################################

#set( FOLDER_NAME "${PROJECT_NAME}-${IMGUI_MAJOR_VERSION}.${IMGUI_MINOR_VERSION}.${IMGUI_PATCH_VERSION}")
#set( BASE_INSTALL_DIR "${IMGUI_INSTALL_HEADER_BASE}/${FOLDER_NAME}" )

install_headers( "${INCLUDES}" "include/implot" )
install_library( implot ${PROJECT_NAME}-targets )

message( STATUS "<<< [external : ImPlot] <<< " )
