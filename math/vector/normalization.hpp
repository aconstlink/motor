

#pragma once

#include "../typedefs.h"

#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

namespace motor
{
    namespace math
    {
        // can be used to state that a vector is passed "not normalized"
        // or can be used to differentiate a overloaded function
        // based on the state of the vector
        template< typename T >
        class not_normalized
        {

            motor_this_typedefs( not_normalized<T> ) ;

            motor_typedefs( T, vec ) ;

        private:

            vec_t _vec ;

        public:

            explicit not_normalized( vec_in_t v ) noexcept : _vec ( v ){}
            not_normalized( this_cref_t rhv ) noexcept : _vec( rhv._vec ) {}
            not_normalized( this_rref_t rhv ) noexcept : _vec( std::move( rhv._vec ) ){}
            ~not_normalized( void_t ) noexcept {}

        public:

            // returns the vector normalized.
            vec_t normalized( void_t ) const noexcept
            {
                return _vec.normalized() ;
            }

            // returns the vector NOT normalized
            // remember to normalize the portion 
            // you need normalized
            vec_cref_t get( void_t ) const noexcept
            {
                return _vec ;
            }
        };

        template< typename T >
        static not_normalized< T > vector_is_not_normalized( T const &  v ) noexcept 
        {
            return not_normalized< T >( v ) ;
        }

        // can be used to state that a vector is passed "normalized"
        // or can be used to differentiate a overloaded function
        // based on the state of the vector
        template< typename T >
        class is_normalized
        {

            motor_this_typedefs( is_normalized<T> ) ;

            motor_typedefs( T, vec ) ;
            motor_typedefs( not_normalized< T >, not_normalized ) ;

        private:

            vec_t _vec ;

        public:

            explicit is_normalized( vec_in_t v ) noexcept : _vec ( v ) {}
            is_normalized( not_normalized_cref_t nn ) noexcept : _vec ( nn.normalized() ) {}
            is_normalized( this_cref_t rhv ) noexcept : _vec( rhv._vec ) {}
            is_normalized( this_rref_t rhv ) noexcept : _vec( std::move( rhv._vec ) ) {}
            ~is_normalized( void_t ) noexcept {}

        public:

            vec_cref_t get( void_t ) const noexcept
            {
                return _vec ;
            }
        };

        template< typename T >
        static is_normalized< T > vector_is_normalized( T const & v ) noexcept
        {
            return is_normalized< T >( v ) ;
        }
    }
}