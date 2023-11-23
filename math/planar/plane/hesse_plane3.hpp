#pragma once

#include "../../vector/vector3.hpp"
#include "../../utility/fn.hpp"

namespace motor
{
    namespace math
    {
        #if 0
        template<typename type_t>
        class hesse_plane< type_t, 3 >
        {
            typedef motor::math::vector3< type_t > vec3_t ;
            typedef vec3_t& vec3_ref_t ;
            typedef vec3_t const& vec3_cref_t ;

            typedef motor::math::vector4< type_t > vec4_t ;
            typedef vec4_t const& vec4_cref_t ;

            typedef motor::math::vector4< type_t > hesse_t ;
            typedef hesse_t& hesse_ref_t ;
            typedef hesse_t const& hesse_cref_t ;

        private:

            hesse_t _plane ;

        public:

            hesse_plane( void_t ) {}

            hesse_plane( vec3_cref_t origin, vec3_cref_t normal )
            {
                _plane = hesse_t( normal.normalized(), calculate_hesse( origin, normal ) ) ;
            }

            hesse_plane( vec4_cref_t hesse_plane_normal )
            {
                _plane = hesse_plane_normal ;
            }

            hesse_cref_t get_plane( void_t ) const { return _plane ; }

            type_t distance( vec3_cref_t point ) const
            {
                return _plane.dot( vec4_t( point, type_t( 1 ) ) ) ;
            }

            inline vec3_t get_abs_normal( void_t ) const {
                //return vec3_t(std::abs(_plane.x()),std::abs(_plane.y()),std::abs(_plane.z())) ;
                return vec3_t(
                    motor::math::fn<float_t>::abs( _plane.x() ),
                    motor::math::fn<float_t>::abs( _plane.y() ),
                    motor::math::fn<float_t>::abs( _plane.z() )
                ) ;
            }

        private:

            type_t calculate_hesse( vec3_cref_t origin, vec3_cref_t normal ) const
            {
                return -( normal.dot( origin ) ) ;
            }
        };
        #endif
    }
}