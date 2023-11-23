#pragma once

#include "switch.h"
#include "../typedefs.h"
#include "../vector/protos.hpp"

namespace motor
{
    namespace math
    {
        /// stores color values in [0,1].
        template< typename type_t >
        class color4
        {
            typedef color4< type_t > this_t ;
            typedef this_t& this_ref_t ;
            typedef this_t const& this_cref_t ;

            typedef motor::math::vector3< type_t > vec3_t ;
            typedef motor::math::vector4< type_t > vec4_t ;
            typedef vec4_t const& vec4_cref_t ;

        private:

            vec4_t _color ;

        public:

            color4( void_t ) {}
            color4( vec4_cref_t color ) : _color( color ) {}
            color4( this_cref_t color ) : _color( color._color ) {}

            color4( motor::math::color_red ) : _color( vec4_t( type_t( 1 ), type_t( 0 ), type_t( 0 ), type_t( 1 ) ) ) {}
            color4( motor::math::color_green ) : _color( vec4_t( type_t( 0 ), type_t( 1 ), type_t( 0 ), type_t( 1 ) ) ) {}
            color4( motor::math::color_blue ) : _color( vec4_t( type_t( 0 ), type_t( 0 ), type_t( 1 ), type_t( 1 ) ) ) {}

        public:

            type_t r( void_t ) const { return _color.x() ; }
            type_t g( void_t ) const { return _color.y() ; }
            type_t b( void_t ) const { return _color.z() ; }
            type_t a( void_t ) const { return _color.w() ; }

            this_ref_t r( type_t v ) { _color.x( v ) ; return *this ; }
            this_ref_t g( type_t v ) { _color.y( v ) ; return *this ; }
            this_ref_t b( type_t v ) { _color.z( v ) ; return *this ; }
            this_ref_t a( type_t v ) { _color.w( v ) ; return *this ; }

        public:

            vec4_cref_t to_vector( void_t ) const { return _color ; }
        };

        typedef color4<float_t> color4f_t ;
        typedef color4<double_t> color4d_t ;
    }
}