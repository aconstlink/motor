#pragma once

#include "../../typedefs.h"

#include "../../vector/vector2.hpp"
#include "../../vector/vector3.hpp"
#include "../../vector/vector4.hpp"
#include "../../matrix/matrix3.hpp"
#include "../../matrix/matrix4.hpp"

#include "../../quaternion/quaternion4.hpp"

namespace motor
{
    namespace math
    {
        namespace m3d
        {
            template< typename T >
            class transformation
            {
            public:

                motor_typedefs( T, type ) ;
                motor_this_typedefs( transformation<type_t> ) ;

                motor_typedefs( motor::math::vector2<type_t>, vec2 ) ;
                motor_typedefs( motor::math::vector3<type_t>, vec3 ) ;
                motor_typedefs( motor::math::vector4<type_t>, vec4 ) ;
                motor_typedefs( motor::math::matrix3<type_t>, mat3 ) ;
                motor_typedefs( motor::math::matrix4<type_t>, mat4 ) ;
                motor_typedefs( motor::math::quaternion4<type_t>, quat4 ) ;

                motor_typedefs( motor::math::vector4<type_t>, axis_angle ) ;
                motor_typedefs( motor::math::vector3<type_t>, euler_angle ) ;

            private:

                // contains scale, rotation and position
                mat4_t _trafo ;

            public:

                transformation( void_t ) : _trafo( motor::math::with_identity() )
                {}

                transformation( motor::math::xzyw ) : _trafo( motor::math::xzyw() )
                {}

                transformation( this_cref_t rhv )
                {
                    *this = rhv ;
                }

                transformation( mat4_cref_t trafo ) : _trafo( trafo )
                {}

                transformation( vec2_cref_t pos ) : _trafo( motor::math::with_identity() )
                {
                    this_t::translate_by( pos ) ;
                }

                transformation( vec3f_cref_t scale, euler_angle_cref_t axis, vec3f_cref_t translation ) :
                    _trafo( motor::math::with_identity() )
                {
                    this_t::scale_fl( scale ) ;
                    this_t::rotate_by_angle_fl( axis ) ;
                    this_t::translate_fl( translation ) ;
                }

                transformation( vec3f_cref_t scale, axis_angle_cref_t axis, vec3f_cref_t translation ) :
                    _trafo( motor::math::with_identity() )
                {
                    this_t::scale_fl( scale ) ;
                    this_t::rotate_by_angle_fl( axis.xyz(), axis.w() ) ;
                    this_t::translate_fl( translation ) ;
                }


            public:

                void_t set_transformation( mat4f_cref_t mat )
                {
                    _trafo = mat ;
                }

                mat4_cref_t get_transformation( void_t ) const { return _trafo ; }

                vec3_t get_translation( void_t ) const
                {
                    return _trafo.get_column( 3 ) ;
                }

            public:

                this_ref_t set_scale( type_t scaling )
                {
                    _trafo.set_main_diagonal( vec4_t( vec3_t( scaling ), type_t( 1 ) ) ) ;
                    return *this ;
                }

                this_ref_t set_scale( vec3_cref_t scaling )
                {
                    _trafo.set_main_diagonal( vec4_t( scaling, type_t( 1 ) ) ) ;
                    return *this ;
                }

                vec3_t get_scale( void_t ) const
                {
                    return _trafo.get_vec3_diagonal() ;
                }

                /// scale in all dimensions from left
                this_ref_t scale_fl( type_t scaling )
                {
                    mat4_t trafo = mat4_t( motor::math::with_identity() ).
                        scale_by( scaling ) ;

                    trafo[ 15 ] = 1.0f ;

                    _trafo = trafo * _trafo ;

                    return *this ;
                }

                /// scale in all dimensions from right
                this_ref_t scale_fr( type_t scaling )
                {
                    mat4_t trafo = mat4_t( motor::math::with_identity() ).
                        scale_by( scaling ) ;

                    trafo[ 15 ] = 1.0f ;

                    _trafo = _trafo * trafo ;

                    return *this ;
                }

                /// scale from left
                this_ref_t scale_fl( vec3_cref_t scaling )
                {
                    mat4_t trafo = mat4_t( motor::math::with_identity() ).
                        scale_by( scaling ) ;

                    trafo[ 15 ] = 1.0f ;

                    _trafo = trafo * _trafo ;

                    return *this ;
                }

                /// scale from right
                this_ref_t scale_fr( vec3_cref_t scaling )
                {
                    mat4_t trafo = mat4_t( motor::math::with_identity() ).
                        scale_by( scaling ) ;

                    trafo[ 15 ] = 1.0f ;

                    _trafo = _trafo * trafo ;

                    return *this ;
                }

                /// rotate by angle(euler angles) from left
                this_ref_t rotate_by_angle_fl( vec3_cref_t per_axis_angle )
                {
                    vec3_t const x_axis = vec3_t( motor::math::x_axis() ) ;
                    vec3_t const y_axis = vec3_t( motor::math::y_axis() ) ;
                    vec3_t const z_axis = vec3_t( motor::math::z_axis() ) ;

                    quat4_t const x( per_axis_angle.x(), x_axis,
                        motor::math::axis_normalized() ) ;

                    quat4_t const y( per_axis_angle.y(), y_axis,
                        motor::math::axis_normalized() ) ;

                    quat4_t const z( per_axis_angle.z(), z_axis,
                        motor::math::axis_normalized() ) ;

                    // rotation matrix
                    mat4_t rot( quat4_t( z * y * x ).to_matrix() ) ;

                    rot[ 15 ] = float_t( 1 ) ;

                    _trafo = rot * _trafo ;

                    return *this ;
                }

                /// rotate by angle(euler angles) from right
                /// => this * rot(euler angles)
                this_ref_t rotate_by_angle_fr( vec3_cref_t per_axis_angle )
                {
                    vec3_t const x_axis = vec3_t( motor::math::x_axis() ) ;
                    vec3_t const y_axis = vec3_t( motor::math::y_axis() ) ;
                    vec3_t const z_axis = vec3_t( motor::math::z_axis() ) ;

                    quat4_t const x( per_axis_angle.x(), x_axis,
                        motor::math::axis_normalized() ) ;

                    quat4_t const y( per_axis_angle.y(), y_axis,
                        motor::math::axis_normalized() ) ;

                    quat4_t const z( per_axis_angle.z(), z_axis,
                        motor::math::axis_normalized() ) ;

                    // rotation matrix
                    mat4_t rot( quat4_t( x * y * z ).to_matrix() ) ;
                    //mat4_t rot = z.to_matrix() * y.to_matrix() * x.to_matrix() ;
                    rot[ 15 ] = float_t( 1 ) ;

                    _trafo = _trafo * rot ;

                    return *this ;
                }

                /// rotation by axis from left
                /// => rot(axis, angle) * this
                this_ref_t rotate_by_axis_fl( vec3_cref_t axis, type_t angle )
                {
                    auto m = mat4_t( motor::math::quat4f_t(
                        angle, axis, motor::math::axis_normalized() ).to_matrix() ) ;
                    m[ 15 ] = type_t( 1 ) ;

                    _trafo = m * _trafo ;
                    return *this ;
                }

                /// rotation by axis from right
                /// => this * rot(axis, angle)
                this_ref_t rotate_by_axis_fr( vec3_cref_t axis, type_t angle )
                {
                    auto m = mat4_t( motor::math::quat4f_t(
                        angle, axis, motor::math::axis_normalized() ).to_matrix() ) ;
                    m[ 15 ] = type_t( 1 ) ;

                    _trafo = _trafo * m ;
                    return *this ;
                }

                /// rotate by a rotation matrix from left
                /// => rot_mat * this
                this_ref_t rotate_by_matrix_fl( mat3_cref_t rot_mat )
                {
                    mat4f_t trafo( rot_mat ) ;
                    trafo[ 15 ] = 1.0f ;

                    _trafo = trafo * _trafo ;

                    return *this ;
                }

                /// rotate by a rotation matrix from right
                /// => this * rot_mat
                this_ref_t rotate_by_matrix_fr( mat3_cref_t rot_mat )
                {
                    mat4f_t trafo( rot_mat ) ;
                    trafo[ 15 ] = 1.0f ;

                    _trafo = _trafo * trafo ;

                    return *this ;
                }

                mat3_t get_rotation_matrix( void_t ) const
                {
                    return mat3_t( _trafo ) ;
                }

                /// sets the translation to the specified position
                /// => cur_pos = to
                this_ref_t set_translation( vec3_cref_t to )
                {
                    _trafo.set_column( 3, vec4_t( to, type_t( 1 ) ) ) ;
                    return *this ;
                }

                /// translate from left(fl)
                /// => T(by) * this
                this_ref_t translate_fl( vec3_cref_t by )
                {
                    auto const trans = mat4_t( motor::math::with_identity() )
                        .set_column( 3, vec4_t( by, type_t( 1 ) ) ) ;

                    _trafo = trans * _trafo ;

                    return *this ;
                }

                /// translate from right(fr)
                /// => this * T(by)
                this_ref_t translate_fr( vec3_cref_t by )
                {
                    auto const trans = mat4_t( motor::math::with_identity() )
                        .set_column( 3, vec4_t( by, type_t( 1 ) ) ) ;

                    _trafo = _trafo * trans ;

                    return *this ;
                }

                /// transform from left(fl)
                /// => by * this
                this_ref_t transform_fl( this_cref_t by )
                {
                    _trafo = by._trafo * _trafo ;
                    return *this ;
                }

                /// transform from right(fr)
                /// => this * by
                this_ref_t transform_fr( this_cref_t by )
                {
                    _trafo = _trafo * by._trafo ;
                    return *this ;
                }

            public:

                static this_t scaling( type_t scale )
                {
                    return this_t( motor::math::with_identity() ).
                        set_scale( scale ) ;
                }

                static this_t scaling( vec3_cref_t scale )
                {
                    return this_t( motor::math::with_identity() ).
                        set_scale( scale ) ;
                }

                static this_t translation( vec3_cref_t t )
                {
                    return this_t( motor::math::with_identity { } ).
                        set_translation( t ) ;
                }

                static this_t rotation_by_euler( vec3_cref_t angles )
                {
                    return this_t( motor::math::with_identity { } ).
                        rotate_by_angle_fr( angles ) ;
                }

                static this_t rotation_by_axis( vec3_cref_t axis, float_t angle )
                {
                    return this_t( motor::math::with_identity { } ).
                        rotate_by_axis_fr( axis, angle ) ;
                }

                static this_t rotation_by_matrix( mat3_cref_t m )
                {
                    return this_t( motor::math::with_identity { } ).
                        rotate_by_matrix_fr( m ) ;
                }

            public:


                this_ref_t operator = ( this_cref_t rhv )
                {
                    _trafo = rhv._trafo ;
                    return *this ;
                }

                this_ref_t operator = ( this_rref_t rhv )
                {
                    _trafo = rhv._trafo ;
                    return *this ;
                }

                /// transform from right!
                /// this * rhv 
                this_t operator * ( this_cref_t rhv ) const
                {
                    return this_t( _trafo * rhv._trafo ) ;
                }

                vec3_t operator * ( vec3_cref_t v ) const
                {
                    return _trafo * v ;
                }

                /// transform from right!
                /// this = this * rhv 
                this_ref_t operator *= ( this_cref_t rhv )
                {
                    _trafo = _trafo * rhv._trafo ;
                    return *this ;
                }
            };
            motor_typedefs( motor::math::m3d::transformation<float_t>, trafof ) ;
        }
    }
}