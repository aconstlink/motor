

#pragma once

#include "matrix2.hpp"

#include "../typedefs.h"
#include "../vector/vector2.hpp"
#include "../vector/vector3.hpp"

#include <array>

namespace motor
{
    namespace math
    {
        template< typename T >
        class matrix2x3
        {
            motor_this_typedefs( matrix2x3<T> ) ;

            motor_typedefs( T, type ) ;
            motor_typedefs( motor::math::vector2< type_t >, vec2 ) ;
            motor_typedefs( motor::math::vector3< type_t >, vec3 ) ;
            motor_typedefs( motor::math::matrix2< type_t >, mat2 ) ;

        private:

            // store column major
            std::array< type_t, 6 > _m ;

        public:

            matrix2x3( void_t ) noexcept
            {
                for( size_t i=0; i<6; ++i ) _m[i] = type_t(0) ;
            }

            matrix2x3( vec2_cref_t c0, vec2_cref_t c1, vec2_cref_t c2 ) noexcept
            {
                this_t::set_column( 0, c0 ) ;
                this_t::set_column( 1, c1 ) ;
                this_t::set_column( 2, c2 ) ;
            }

            matrix2x3( vec3_cref_t r0, vec3_cref_t r1 ) noexcept
            {
                this_t::set_row( 0, r0 ) ;
                this_t::set_row( 1, r1 ) ;
            }

            // sets the left 2x2 matrix along with the third column
            matrix2x3( mat2_cref_t m, vec2_cref_t c2 ) noexcept
            {
                this_t::set_2x2( m ) ;
                this_t::set_column( 2, c2 ) ;
            }

            matrix2x3( this_cref_t rhv ) noexcept : _m( rhv._m ) {}
            ~matrix2x3( void_t ) noexcept{}

        public:

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _m = rhv._m ;
                return *this ;
            }

            // transform location vector
            vec2_t operator * ( vec3_cref_t p ) const noexcept
            {
                return vec2_t( 
                    this_t::row(0).dot( p ), 
                    this_t::row(1).dot( p ) ) ;
            }

            // transform direction vector
            vec2_t operator * ( vec2_cref_t p ) const noexcept
            {
                return vec2_t( 
                    this_t::row(0).xy().dot( p ), 
                    this_t::row(1).xy().dot( p ) ) ;
            }

        public:

            vec3_t row( size_t const i ) const noexcept
            { 
                size_t const idx = (i % 2) ;
                return vec3_t( _m[idx], _m[idx+2], _m[idx+4] ) ; 
            }

            vec2_t column( size_t const i ) const noexcept
            { 
                size_t const idx = (i % 3) * 2 ;
                return vec2_t( _m[idx], _m[idx+1] ) ; 
            }

            this_ref_t set_row( size_t const i, vec3_cref_t v ) noexcept
            {
                size_t const idx = (i % 2) ;
                _m[idx+0] = v.x() ;
                _m[idx+2] = v.y() ;
                _m[idx+4] = v.z() ;
                return *this ;
            }

            this_ref_t set_column( size_t const i, vec2_cref_t v ) noexcept
            {
                size_t const idx = (i % 3) * 2 ;
                _m[idx+0] = v.x() ;
                _m[idx+1] = v.y() ;
                return *this ;
            }

            // returns columsn 0 and 1 as a 2x2 matrix
            mat2_t get2x2( void_t ) const noexcept
            {
                return mat2_t::from_columns( this_t::column(0), this_t::column(1) ) ;
            }

            // sets the left 2x2 matrix 
            this_ref_t set_2x2( mat2_cref_t m ) noexcept
            {
                this_t::set_column( 0, m.get_column(0) ) ;
                this_t::set_column( 1, m.get_column(1) ) ;

                return *this ;
            }

            static this_t identity( void_t ) noexcept 
            {
                return this_t( vec3_t(type_t(1), type_t(0),type_t(0)), 
                    vec3_t(type_t(0), type_t(1),type_t(0)) ) ;
            }
        };
        template< typename T >
        using matrix23 = matrix2x3<T> ;

        motor_typedefs( matrix2x3<float_t>, mat2x3f ) ;
        motor_typedefs( matrix2x3<float_t>, mat23f ) ;
    }
}