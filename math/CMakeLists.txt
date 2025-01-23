
set( cpp_sources 
)

set( avx_sources )

# not arch specific
set( non_arch_sources

    animation/evaluation_result.h
    animation/keyframe.hpp
    animation/keyframe_sequence.hpp
    animation/time_remap_funk.h

    spline/linear_bezier_spline.hpp
    spline/quadratic_bezier_spline.hpp
    spline/cubic_bezier_spline.hpp
    spline/cubic_hermit_spline.hpp
    spline/continuity.h
    curve/catmull_rom_curve.hpp
    curve/cubic_bezier_curve.hpp

    color/protos.hpp
    color/color3.hpp
    color/color4.hpp

    vector/protos.hpp
    vector/vector2.hpp
    vector/vector3.hpp
    vector/vector4.hpp
    vector/vector2b.hpp
    vector/vector3b.hpp
    vector/vector4b.hpp
    vector/normalization.hpp

    interpolation/interpolate.hpp

    matrix/protos.hpp
    matrix/matrix2.hpp
    matrix/matrix3.hpp
    matrix/matrix4.hpp
    matrix/matrix2x3.hpp
    quaternion/protos.hpp
    quaternion/quaternion3.hpp
    quaternion/quaternion4.hpp

    euler/euler_angles.hpp
    euler/euler_sequence.h

    planar/plane/hesse_plane3.hpp

    primitive/2d/line.hpp
    primitive/2d/ray.hpp
    primitive/2d/line_segment.hpp
    primitive/2d/ray_line_intersect.hpp
    primitive/2d/plane.hpp

    primitive/3d/ray.hpp
    primitive/3d/frustum.hpp

    utility/constants.hpp
    utility/radian.hpp
    utility/degree.hpp
    utility/angle.hpp
    utility/fn.hpp
    utility/factorial.hpp
    utility/taylor.hpp
    utility/index_1d_to_2d.hpp
    utility/index_2d_to_1d.hpp
    utility/index_3d_to_1d.hpp
    utility/time.hpp
    
    utility/3d/ortho_basis.hpp
    utility/3d/transformation.hpp


    utility/2d/viewport.hpp
    utility/2d/coord_transformator.hpp
    utility/2d/index_transformator.hpp
    utility/2d/transformation.hpp

    camera/3d/perspective_camera.hpp
    camera/3d/orthographic_camera.hpp
    camera/3d/camera_util.hpp
    camera/3d/perspective_fov.hpp
    camera/3d/orthographic_projection.hpp
    
    dsp/fft.hpp
)

# compound include files
set( sources 
    typedefs.h

    ${cpp_sources}
    ${non_arch_sources}	
)

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} INTERFACE ${sources} )
target_link_libraries( ${cur_lib_name}
    INTERFACE motor::std
    INTERFACE motor::base
)

target_compile_options( ${cur_lib_name} INTERFACE -DNOMINMAX)

###########################################################
# SECTION: Build Tree
###########################################################

#so_export( ${CUR_LIB_NAME} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )

