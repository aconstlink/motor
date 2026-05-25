#pragma once

#include "../../vector/vector3.hpp"
#include "../../vector/vector4.hpp"
#include "../../matrix/matrix3.hpp"
#include "../../matrix/matrix4.hpp"

#include "../../utility/3d/ortho_basis.hpp"

namespace motor
{
namespace math
{
namespace m3d
{

//
// this utility class helps to create the
// camera and the view matrix.
// utility for :
// - create camera matrix
// - create view matrix
// - extract camera matrix properties
// - transform camera matrix
//
// @note
// motor uses a camera model where it is looking in the
// positive z direction.
//
template < typename T >
class camera_util
{
    using this_t = camera_util< T >;
    using real_t = T;
    using vec3_t = motor::math::vector3< T >;
    using vec4_t = motor::math::vector4< T >;
    using mat3_t = motor::math::matrix3< T >;
    using mat4_t = motor::math::matrix4< T >;

  public: // look at

    // allows to specify an up vector.
    // @param pos_ the cameras' position
    // @param up_ a specific up vector
    // @param at_ at which point the camera is looking at.
    static void create_lookat( vec3_t const & pos_, vec3_t const & up_, vec3_t const & at_, mat4_t & inout ) noexcept
    {
        motor::math::m3d::orthonormal_basis< real_t >::create_affine( pos_, up_, ( pos_ - at_ ).normalized(), inout );
        inout.set_column( 2, inout.column3( 2 ).negated() );
    }

    // creates the camera matrix via the looking position.
    // this engine uses a camera model where it is looking in the
    // positive z direction.
    // no up vector can be set in this function. a default up vector is assumed.
    // usually the default up is (0,1,0)
    // @param pos_ the cameras' position
    // @param at_ at which point the camera is looking at.
    static void create_lookat( vec3_t const & pos_, vec3_t const & at_, mat4_t & inout ) noexcept
    {
        this_t::create_lookat_from_dir( pos_, ( pos_ - at_ ).normalized(), inout );        
    }

    static void create_lookat_from_dir( vec3_t const & pos_, vec3_t const & dir_, mat4_t & inout ) noexcept
    {
        motor::math::m3d::orthonormal_basis< real_t >::create_affine( pos_, dir_, inout );
        inout.set_column( 2, inout.column3( 2 ).negated() );
    }

    static void create_lookat_dir( vec3_t const & pos, vec3_t const & dir, mat4_t & inout ) noexcept
    {
        this_t::create_lookat_from_dir( pos, dir, inout );
    }

  public: // view matrix

    /**
        This function inverts the camera matrix.
        The camera matrix is given by T*R.
        Invert this gives: R^t * T^-1
        This transforms all points in world space, or relative space
        to the camera, into the camera space or view space.


        @param cm [in] The camera matrix.
        @param out [out] The viewing matrix.

        @note
        In OpenGL, this matrix must be loaded transposed.
        This is because OpenGL stores the matrices in row order.
        @endnote
    */
    static void create_view_matrix( mat4_t const & cm, mat4_t & out ) noexcept
    {
        vec3_t t = vec3_t( cm.column( 3 ) );
        mat3_t cm3 = mat3_t( cm ).transpose();
        out = mat4_t( cm3 );

        out.set_column( 3, vec3_t( -cm3.row_u( 0 ).dot( t ), -cm3.row_u( 1 ).dot( t ), -cm3.row_u( 2 ).dot( t ) ) );

        out[ 15 ] = 1.0f;
    }

    static void translate_view_matrix_to( vec3_t const & pos, mat4_t & inout )
    {
        inout.set_column(
            3, vec3_t( -inout.row( 0 ).dot( pos ), -inout.row( 1 ).dot( pos ), -inout.row( 2 ).dot( pos ) ) );

        inout[ 15 ] = 1.0f;
    }

  public: // general transformation helpers

    static void translate_camera_matrix_to( vec3_t const & pos, mat4_t & inout ) noexcept
    {
        inout.set_column( 3, vec4_t( pos ) );
        inout[ 15 ] = 1.0f;
    }

    static void create_lookat_from_vec( vec3_t const & pos, vec3_t const & dir, vec3_t const & up, vec3_t const & right,
                                        mat4_t & inout ) noexcept
    {
        // recreate the object matrix
        inout.set_column( 0, vec4_t( right ) );
        inout.set_column( 1, vec4_t( up ) );
        inout.set_column( 2, vec4_t( dir ) );
        inout.set_column( 3, vec4_t( pos ) );

        inout[ 15 ] = 1.0f;
    }

    static void get_frame_from_camera_matrix( vec3_t & pos, vec3_t & dir, vec3_t & up, vec3_t & right,
                                              mat4_t const & mat_in ) noexcept
    {
        mat_in.get_column( 0, right );
        mat_in.get_column( 1, up );
        mat_in.get_column( 2, dir );
        mat_in.get_column( 3, pos );
    }

    static vec3_t get_dir_from_camera_matrix( mat4_t const & mat_in ) noexcept
    {
        return mat_in.get_column( 2 ).xyz();
    }

    static vec3_t get_up_from_camera_matrix( mat4_t const & mat_in ) noexcept
    {
        return mat_in.get_column( 1 ).xyz();
    }

    static vec3_t get_right_from_camera_matrix( mat4_t const & mat_in ) noexcept
    {
        return mat_in.get_column( 0 ).xyz();
    }

    static vec3_t get_pos_from_camera_matrix( mat4_t const & mat_in ) noexcept
    {
        return mat_in.get_column( 3 ).xyz();
    }

    static void set_pos_for_camera_matrix( vec3_t const & pos, mat4_t & mat_inout ) noexcept
    {
        mat_inout.set_column( 3, pos );
    }

    static void get_frame_from_view_matrix( vec3_t & dir, vec3_t & up, vec3_t & right, mat4_t const & mat_in ) noexcept
    {
        mat_in.get_row( 0, right );
        mat_in.get_row( 1, up );
        mat_in.get_row( 2, dir );
    }

    static void get_frame_from_view_matrix( mat3_t & mat_out, mat4_t const & mat_in ) noexcept
    {
        vec3_t vec;
        mat_in.get_row( 0, vec ); // right
        mat_out.set_column( 0, vec );
        mat_in.get_row( 1, vec ); // up
        mat_out.set_column( 1, vec );
        mat_in.get_row( 2, vec ); // dir
        mat_out.set_column( 2, vec );
    }

    static void get_dir_from_view_matrix( vec3_t & dir, mat4_t const & mat_in ) noexcept
    {
        mat_in.get_row( 2, dir );
    }
};
} // namespace m3d
} // namespace math
} // namespace motor
