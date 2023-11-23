#pragma once

#include "fn.hpp"
#include "constants.hpp"
#include "../typedefs.h"

namespace motor
{
    namespace math
    {
        /// constrains a radian value to [0,2pi]
        template< typename T >
        class angle
        {
            motor_this_typedefs( angle<T> ) ;
            motor_typedefs( T, type ) ;

        private:

            /// the angle in radians
            type_t _angle ;

        public:

            /// returns a constrained to [0, 2*pi] value 
            /// based on the passed radians value
            static type_t constrain_angle( type_t const radian )
            {
                return motor::math::fn<type_t>::mod( radian, type_t( 2 ) * motor::math::constants<type_t>::pi() ) ;
            }

            static type_t radian_to_degree( type_t const radian )
            {
                return radian * type_t( 180 ) / motor::math::constants<type_t>::pi() ;
            }

            static type_t degree_to_radian( type_t const degree )
            {
                return degree * motor::math::constants<type_t>::pi() / type_t( 180 ) ;
            }

        public:

            struct degrees {} ;
            struct radians {} ;

        public:

            angle( void_t )
            {
                _angle = type_t( 0 ) ;
            }

            angle( type_t const rad )
            {
                this_t::from_radians( rad ) ;
            }

            /// allows to pass degrees as argument
            angle( type_t const deg, degrees )
            {
                this_t::from_degrees( deg ) ;
            }

        public:

            type_t as_radians( void_t ) const
            {
                return _angle ;
            }

            type_t as_degrees( void_t ) const
            {
                return this_t::radian_to_degree( _angle ) ;
            }

        public:

            this_ref_t from_radians( type_t const rad )
            {
                _angle = rad ; return *this ;
            }

            this_ref_t from_degrees( type_t const deg )
            {
                _angle = this_t::degree_to_radian( deg ) ;  return *this ;
            }

            type_t value( void_t ) const { return _angle ; }

            type_t constrained_value( void_t ) const
            {
                return this_t::constrain_angle( _angle ) ;
            }

        public:

            this_t operator + ( type_t const rad ) const
            {
                return this_t( this_t::as_radians() + rad ) ;
            }

            this_ref_t operator += ( type_t const rad )
            {
                return this_t::from_radians( _angle + rad ) ;
            }

            this_t operator - ( type_t const rad ) const
            {
                return this_t( this_t::as_radians() - rad ) ;
            }

            this_ref_t operator -= ( type_t const rad )
            {
                return this_t::from_radians( _angle - rad ) ;
            }

            this_t operator + ( this_cref_t other ) const
            {
                return this_t( this_t::as_radians() + other.as_radians() ) ;
            }

            this_ref_t operator += ( this_cref_t other )
            {
                return this_t::from_radians( _angle + other.as_radians() ) ;
            }

            this_t operator - ( this_cref_t other ) const
            {
                return this_t( this_t::as_radians() - other.as_radians() ) ;
            }

            this_ref_t operator -= ( this_cref_t other )
            {
                return this_t::from_radians( _angle - other.as_radians() ) ;
            }
        };
        motor_typedefs( angle< float_t >, anglef ) ;
    }
}