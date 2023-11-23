#pragma once

#include "euler_sequence.h"

#include "../typedefs.h"
#include "../vector/vector3.hpp"
#include "../matrix/matrix3.hpp"
#include "../quaternion/quaternion4.hpp"

#include "../utility/angle.hpp"
#include "../utility/3d/transformation.hpp"

namespace motor
{
    namespace math
    {
        template< typename T >
        class euler_angles
        {
            motor_this_typedefs( euler_angles<T> ) ;
            motor_typedefs( T, type ) ;

            motor_typedefs( motor::math::angle< T >, angle ) ;
            motor_typedefs( motor::math::vector3<angle_t>, angles ) ;
            motor_typedefs( motor::math::vector3<T>, vec3 ) ;
            motor_typedefs( motor::math::quaternion4<T>, quat4 ) ;

            motor_typedefs( motor::math::matrix3<T>, mat3 ) ;
            motor_typedefs( motor::math::m3d::transformation<T>, trafo ) ;

        private:

            euler_sequence _sequence ;
            angles_t _angles ;

        public:

            //************************************************************************************
            euler_angles( euler_sequence s ) : 
                _sequence(s), _angles( vec3_t(type_t(0.0)) ){}

            //************************************************************************************
            euler_angles( euler_sequence s, angles_in_t angles ) : 
                _sequence(s), _angles(angles) {}

            //************************************************************************************
            euler_angles( euler_sequence s, vec3_in_t angles ) : 
                _sequence(s), _angles( angle_t(angles.x()), angle_t(angles.y()), angle_t(angles.z()) )
            {}

            //************************************************************************************
            euler_angles( this_cref_t rhv ) : 
                _sequence(rhv._sequence), _angles(rhv._angles) {}

            //************************************************************************************
            euler_angles( this_rref_t rhv ) : 
                _sequence(rhv._sequence), _angles(std::move(rhv._angles)){}

            //************************************************************************************
            ~euler_angles( void_t ) {}

        public:

            inline T pi( void_t ) const 
            {
                return T(3.14159265359) ;
            }

        public:

            //************************************************************************************
            this_ref_t operator = ( this_cref_t rhv )
            {
                _sequence = rhv._sequence ;
                _angles = rhv._angles ;
                return *this ;
            }

            //************************************************************************************
            this_ref_t operator = ( this_rref_t rhv )
            {
                _sequence = rhv._sequence ;
                _angles = std::move(rhv._angles) ;
                return *this ;
            }

        public:

            //************************************************************************************
            euler_sequence get_sequence( void_t ) const 
            { 
                return _sequence ; 
            }

            //************************************************************************************
            angles_cref_t get_angles( void_t ) const 
            { 
                return _angles ; 
            }

        public:

            //************************************************************************************
            mat3_t compute( void_t ) const
            {
                vec3_t const x = vec3_t( motor::math::x_axis() ) ;
                vec3_t const y = vec3_t( motor::math::y_axis() ) ;
                vec3_t const z = vec3_t( motor::math::z_axis() ) ;

                switch( _sequence )
                {
                    case motor::math::euler_sequence::yxz:
                    {
                        auto const mat_a = quat4_t( _angles.x().as_radians(), y,
                            motor::math::axis_normalized() ).to_matrix() ;

                        auto const mat_b = quat4_t( _angles.y().as_radians(), x,
                            motor::math::axis_normalized() ).to_matrix() ;

                        auto const mat_c = quat4_t( _angles.z().as_radians(), z,
                            motor::math::axis_normalized() ).to_matrix() ;

                        return mat_a * mat_b * mat_c ;
                    }
                default :
                break  ;
                }
            

                return mat3_t() ;
            }

            //************************************************************************************
            this_t convert_to( euler_sequence target ) const 
            {
                if( _sequence == target )
                    return *this ;

                mat3_t const trafo = this_t::compute() ;

                angles_t angles ;

                switch( target )
                {
                case euler_sequence::xyz: 
                    angles = this_t::convert_to_xyz( trafo ) ;
                    break ;
                case euler_sequence::yzx: 
                    angles = this_t::convert_to_yzx( trafo ) ;
                    break ;
                case euler_sequence::zxy: 
                    angles = this_t::convert_to_zxy( trafo ) ;
                    break ;
                case euler_sequence::xzy: 
                    angles = this_t::convert_to_xzy( trafo ) ;
                    break ;
                case euler_sequence::zyx: 
                    angles = this_t::convert_to_zyx( trafo ) ;
                    break ;
                case euler_sequence::yxz: 
                    angles = this_t::convert_to_yxz( trafo ) ;
                    break ;
                }

                return this_t( target, angles ) ;
            }

            //************************************************************************************
            angles_t convert_to_xyz( mat3_in_t mat_in ) const 
            {
                motor::math::vec2f_t alpha, beta, gamma ;

                // beta
                {
                    type_t const beta_1 = asin( mat_in.get_element(0,2) ) ;
                    type_t const beta_2 = this_t::pi() - beta_1 ;

                    beta = motor::math::vec2f_t( beta_1, beta_2 ) ;
                }

                // gamma
                {
                    type_t const gamma_1 = -std::atan2( 
                        mat_in.get_element(0,1) / std::cos(beta.x()), 
                        mat_in.get_element(0,0) / std::cos(beta.x()) ) ;

                    type_t const gamma_2 = -std::atan2(
                        mat_in.get_element( 0, 1 ) / std::cos( beta.y() ),
                        mat_in.get_element( 0, 0 ) / std::cos( beta.y() ) ) ;

                    gamma = motor::math::vec2f_t( gamma_1, gamma_2 ) ;
                }

                // alpha
                {
                    type_t const alpha_1 = -std::atan2(
                        mat_in.get_element( 1,2 ) / std::cos( beta.x() ),
                        mat_in.get_element( 2,2 ) / std::cos( beta.x() ) ) ;

                    type_t const alpha_2 = -std::atan2(
                        mat_in.get_element( 1,2 ) / std::cos( beta.y() ),
                        mat_in.get_element( 2,2 ) / std::cos( beta.y() ) ) ;

                    alpha = motor::math::vec2f_t( alpha_1, alpha_2 ) ;
                }

                return angles_t( alpha.x(), beta.x(), gamma.x() ) ;
            }

            //************************************************************************************
            angles_t convert_to_yzx( mat3_in_t mat_in  ) const
            {
                motor::math::vec2f_t alpha, beta, gamma ;

                // beta
                {
                    type_t const beta_1 = -asin( mat_in.get_element( 0, 1 ) ) ;
                    type_t const beta_2 = this_t::pi() - beta_1 ;

                    beta = motor::math::vec2f_t( beta_1, beta_2 ) ;
                }

                // gamma
                {
                    type_t const gamma_1 = std::atan2(
                        mat_in.get_element( 1, 2 ) / std::cos( beta.x() ),
                        mat_in.get_element( 1, 1 ) / std::cos( beta.x() ) ) ;

                    type_t const gamma_2 = std::atan2(
                        mat_in.get_element( 1, 2 ) / std::cos( beta.y() ),
                        mat_in.get_element( 1, 1 ) / std::cos( beta.y() ) ) ;

                    gamma = motor::math::vec2f_t( gamma_1, gamma_2 ) ;
                }

                // alpha
                {
                    type_t const alpha_1 = std::atan2(
                        mat_in.get_element( 1, 2 ) / std::cos( beta.x() ),
                        mat_in.get_element( 2, 2 ) / std::cos( beta.x() ) ) ;

                    type_t const alpha_2 = std::atan2(
                        mat_in.get_element( 1, 2 ) / std::cos( beta.y() ),
                        mat_in.get_element( 2, 2 ) / std::cos( beta.y() ) ) ;

                    alpha = motor::math::vec2f_t( alpha_1, alpha_2 ) ;
                }

                return angles_t( alpha.x(), beta.x(), gamma.x() ) ;
            }

            //************************************************************************************
            angles_t convert_to_zxy( mat3_in_t mat_in  ) const
            {
                motor::math::vec2f_t alpha, beta, gamma ;

                // beta
                {
                    type_t const beta_1 = asin( mat_in.get_element( 2,1 ) ) ;
                    type_t const beta_2 = this_t::pi() - beta_1 ;

                    beta = motor::math::vec2f_t( beta_1, beta_2 ) ;
                }

                // gamma
                {
                    type_t const gamma_1 = -std::atan2(
                        mat_in.get_element( 2,0 ) / std::cos( beta.x() ),
                        mat_in.get_element( 2,2 ) / std::cos( beta.x() ) ) ;

                    type_t const gamma_2 = -std::atan2(
                        mat_in.get_element( 2,0 ) / std::cos( beta.y() ),
                        mat_in.get_element( 2,2) / std::cos( beta.y() ) ) ;

                    gamma = motor::math::vec2f_t( gamma_1, gamma_2 ) ;
                }

                // alpha
                {
                    type_t const alpha_1 = -std::atan2(
                        mat_in.get_element( 0,1 ) / std::cos( beta.x() ),
                        mat_in.get_element( 1,1 ) / std::cos( beta.x() ) ) ;

                    type_t const alpha_2 = -std::atan2(
                        mat_in.get_element( 0,1 ) / std::cos( beta.y() ),
                        mat_in.get_element( 1,1 ) / std::cos( beta.y() ) ) ;

                    alpha = motor::math::vec2f_t( alpha_1, alpha_2 ) ;
                }

                return angles_t( alpha.x(), beta.x(), gamma.x() ) ;
            }

            //************************************************************************************
            angles_t convert_to_xzy( mat3_in_t mat_in ) const
            {
                motor::math::vec2f_t alpha, beta, gamma ;

                // beta
                {
                    type_t const beta_1 = -asin( mat_in.get_element( 0,1 ) ) ;
                    type_t const beta_2 = this_t::pi() - beta_1 ;

                    beta = motor::math::vec2f_t( beta_1, beta_2 ) ;
                }

                // gamma
                {
                    type_t const gamma_1 = std::atan2(
                        mat_in.get_element( 0,2 ) / std::cos( beta.x() ),
                        mat_in.get_element( 0,0 ) / std::cos( beta.x() ) ) ;

                    type_t const gamma_2 = std::atan2(
                        mat_in.get_element( 0,2 ) / std::cos( beta.y() ),
                        mat_in.get_element( 0,0 ) / std::cos( beta.y() ) ) ;

                    gamma = motor::math::vec2f_t( gamma_1, gamma_2 ) ;
                }

                // alpha
                {
                    type_t const alpha_1 = std::atan2(
                        mat_in.get_element( 2,1 ) / std::cos( beta.x() ),
                        mat_in.get_element( 1, 1 ) / std::cos( beta.x() ) ) ;

                    type_t const alpha_2 = std::atan2(
                        mat_in.get_element( 2,1 ) / std::cos( beta.y() ),
                        mat_in.get_element( 1,1 ) / std::cos( beta.y() ) ) ;

                    alpha = motor::math::vec2f_t( alpha_1, alpha_2 ) ;
                }

                return angles_t( alpha.x(), beta.x(), gamma.x() ) ;
            }

            //************************************************************************************
            angles_t convert_to_zyx( mat3_in_t mat_in ) const
            {
                motor::math::vec2f_t alpha, beta, gamma ;
                // beta
                {
                    type_t const beta_1 = -asin( mat_in.get_element( 2, 0 ) ) ;
                    type_t const beta_2 = this_t::pi() - beta_1 ;

                    beta = motor::math::vec2f_t( beta_1, beta_2 ) ;
                }

                // gamma
                {
                    type_t const gamma_1 = std::atan2(
                        mat_in.get_element( 2, 1 ) / std::cos( beta.x() ),
                        mat_in.get_element( 2, 2 ) / std::cos( beta.x() ) ) ;

                    type_t const gamma_2 = std::atan2(
                        mat_in.get_element( 2, 1 ) / std::cos( beta.y() ),
                        mat_in.get_element( 2, 2 ) / std::cos( beta.y() ) ) ;

                    gamma = motor::math::vec2f_t( gamma_1, gamma_2 ) ;
                }

                // alpha
                {
                    type_t const alpha_1 = std::atan2(
                        mat_in.get_element( 1, 0 ) / std::cos( beta.x() ),
                        mat_in.get_element( 0, 0 ) / std::cos( beta.x() ) ) ;

                    type_t const alpha_2 = std::atan2(
                        mat_in.get_element( 1, 0 ) / std::cos( beta.y() ),
                        mat_in.get_element( 0, 0 ) / std::cos( beta.y() ) ) ;

                    alpha = motor::math::vec2f_t( alpha_1, alpha_2 ) ;
                }
                return angles_t( alpha.x(), beta.x(), gamma.x() ) ;
            }

            //************************************************************************************
            angles_t convert_to_yxz( mat3_in_t mat_in ) const
            {
                motor::math::vec2f_t alpha, beta, gamma ;

                // beta
                {
                    type_t const beta_1 = -asin( mat_in.get_element( 1, 2 ) ) ;
                    type_t const beta_2 = this_t::pi() - beta_1 ;

                    beta = motor::math::vec2f_t( beta_1, beta_2 ) ;
                }

                // gamma
                {
                    type_t const gamma_1 = std::atan2(
                        mat_in.get_element( 1, 0 ) / std::cos( beta.x() ),
                        mat_in.get_element( 1, 1 ) / std::cos( beta.x() ) ) ;

                    type_t const gamma_2 = std::atan2(
                        mat_in.get_element( 1, 0 ) / std::cos( beta.y() ),
                        mat_in.get_element( 1, 1 ) / std::cos( beta.y() ) ) ;

                    gamma = motor::math::vec2f_t( gamma_1, gamma_2 ) ;
                }

                // alpha
                {
                    type_t const alpha_1 = std::atan2(
                        mat_in.get_element( 0, 2 ) / std::cos( beta.x() ),
                        mat_in.get_element( 2, 2 ) / std::cos( beta.x() ) ) ;

                    type_t const alpha_2 = std::atan2(
                        mat_in.get_element( 0, 2 ) / std::cos( beta.y() ),
                        mat_in.get_element( 2, 2 ) / std::cos( beta.y() ) ) ;

                    alpha = motor::math::vec2f_t( alpha_1, alpha_2 ) ;
                }

                return angles_t( alpha.x(), beta.x(), gamma.x() ) ;
            }
        };

        motor_typedefs( euler_angles<float_t>, euler_anglesf ) ;
        motor_typedefs( euler_angles<double_t>, euler_anglesd ) ;
    }
}