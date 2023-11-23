#pragma once

#include "../../typedefs.h"

#include "../../vector/vector2.hpp"
#include "../../vector/vector3.hpp"
#include "../../matrix/matrix3.hpp"
#include "../../matrix/matrix2x3.hpp"

namespace motor
{
    namespace math
    {
        namespace m2d
        {
            // this transformation performs a rotation and then a transformation
            // mathmatically: T * R * v = v*
            template< typename type_t >
            class transformation
            {
            public:

                motor_this_typedefs( transformation< type_t > ) ;
                motor_typedefs( motor::math::vector2< type_t >, vec2 ) ;
                motor_typedefs( motor::math::vector3< type_t >, vec3 ) ;
                motor_typedefs( motor::math::matrix2< type_t >, mat2 ) ;
                motor_typedefs( motor::math::matrix3< type_t >, mat3 ) ;
                motor_typedefs( motor::math::matrix2x3< type_t >, mat2x3 ) ;

            private:

                // contains rotation and offset : R | o
                mat2x3_t _trafo ;

            public:

                transformation( void_t ) noexcept : _trafo( mat2x3_t::identity() ) {}
                transformation( this_cref_t rhv ) noexcept { *this = rhv ; }

                transformation( vec2_cref_t pos ) noexcept : _trafo( mat2x3_t::identity() )
                {
                    this_t::translate_by( pos ) ;
                }

                // allows to directly set the transformation matrix
                transformation( mat2x3_cref_t m ) noexcept : _trafo( m ) {}

                ~transformation( void_t ) noexcept {}

            public:

                mat2x3_cref_t get_transformation( void_t ) const noexcept { return _trafo ; }

                vec2_t get_translation( void_t ) const noexcept { return _trafo.column( 2 ) ; }
                vec2_t get_origin( void_t )  const noexcept { return _trafo.column( 2 ) ; }

            public:

                static this_t rotation( type_t const angle ) noexcept
                {
                    return this_t().rotate_by( angle ) ;
                }

                /// rotate this transformation by an angle
                this_ref_t rotate_by( type_t const angle )
                {
                    auto const m = mat2_t::rotation( angle ) * _trafo.get2x2() ;
                    _trafo = mat2x3_t( m, _trafo.column(2) ) ;
                    return *this ;
                }

                /// translate this transformation
                this_ref_t translate_by( vec2_cref_t by )
                {
                    _trafo.set_column( 2, _trafo.column( 2 ) + by ) ;
                    return *this ;
                }

                /// transform this transformation by another transformation
                /// rhv is applyed from the left
                this_ref_t transform_by( this_cref_t by )
                {
                    _trafo = by._trafo * _trafo ;
                    return *this ;
                }

                this_ref_t invert( void_t ) noexcept
                {
                    auto const t = _trafo.get2x2().transpose() ;
                    _trafo = mat2x3( t, t * -_trafo.column(2) ) ;
                    return *this ;
                }

                this_t inverted( void_t ) const noexcept
                {
                    return this_t( *this ).invert() ;
                }

            public:

                // transform location
                vec2_t operator * ( vec3_cref_t v ) const noexcept
                {
                    return _trafo * v ;
                }

                // transform direction
                vec2_t operator * ( vec2_cref_t v ) const noexcept
                {
                    return _trafo * v ;
                }

            public:

                this_ref_t operator = ( this_cref_t rhv )
                {
                    _trafo = rhv._trafo ;
                    return *this ;
                }
            };
            motor_typedefs( motor::math::m2d::transformation< float_t >, trafof ) ;
        }
        motor_typedefs( motor::math::m2d::transformation< float_t >, trafo2df ) ;
    }
}