
set( SOURCES 
    typedefs.h
    result.h
    api.h
    protos.h

    3d/fractal/fractal.h
    3d/fractal/fractal.cpp
    
    3d/cube.h
    3d/cube.cpp
    3d/plane.h
    3d/plane.cpp
    3d/tetra.h
    3d/tetra.cpp
    3d/coord_frame.h
    3d/coord_frame.cpp
    3d/helper.h
    3d/rect.h
    3d/rect.cpp

    2d/helper.h
    2d/typedefs.h
    2d/circle.hpp
    2d/quad.hpp

    enums/component_formats.h

    mesh/imesh.h
    mesh/polygon_mesh.h
    mesh/polygon_mesh.cpp
    mesh/tri_mesh.h
    mesh/tri_mesh.cpp
    mesh/flat_tri_mesh.h
    mesh/flat_tri_mesh.cpp
    mesh/flat_line_mesh.h
    mesh/flat_line_mesh.cpp
   )

motor_vs_src_dir( SOURCES )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${SOURCES} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} )

target_link_libraries( ${cur_lib_name}
        PUBLIC motor::log
        PUBLIC motor::memory
        PUBLIC motor::io
    )	

###########################################################
# SECTION: Build Tree
###########################################################

#motor_export( ${CUR_LIB_NAME} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${SOURCES}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )
