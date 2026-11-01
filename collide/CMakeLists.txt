# SPDX-FileCopyrightText: 2023-2026 Alex Link
# SPDX-License-Identifier: MIT

set( sources
    typedefs.h

    hit_test_types.h

    3d/hit_test/hit_test_aabb_aabb.hpp
    3d/hit_test/hit_test_aabb_frustum.hpp
    3d/hit_test/hit_test_aabb_plane.hpp
    3d/hit_test/hit_test_aabb_sphere.hpp
    3d/hit_test/hit_test_aabb_point.hpp
    3d/hit_test/hit_test_aabb_ray.hpp
    3d/hit_test/hit_test_plane_ray.hpp

    3d/aabb.hpp
    3d/sphere.hpp
    3d/view_frustum.hpp
    3d/obb.hpp
    3d/transform.hpp

    2d/hit_test/hit_test_aabb_aabb.hpp
    2d/hit_test/hit_test_aabb_frustum.hpp
    2d/hit_test/hit_test_aabb_circle.hpp
    2d/hit_test/hit_test_aabb_ray.hpp
    2d/hit_test/hit_test_aabb_line_segment.hpp

    2d/collision/circle_line.hpp

    2d/bounds/aabb.hpp
    2d/bounds/circle.hpp
    2d/bounds/obb.hpp

    dummy.cpp
    )

motor_vs_src_dir( sources )


#set( pch_files )
#add_library( collide_pch INTERFACE )
#target_precompile_headers( collide_pch INTERFACE ${pch_files})

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name}
    INTERFACE motor::std
    INTERFACE motor::base
    INTERFACE motor::math
)

###########################################################
# SECTION: Build Tree
###########################################################



###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )
