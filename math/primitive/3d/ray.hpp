#pragma once

#include "../../vector/vector3.hpp"

#include <motor/core/types.hpp>

namespace motor
{
    namespace math
    {
        namespace m3d
        {
            template< typename type_t >
            class ray
            {
                typedef motor::math::vector3< type_t > vec3_t ;
                typedef vec3_t const& vec3_cref_t ;

            private:

                vec3_t _origin ;
                vec3_t _dir ;

            public:

                ray( void_t ) {}

                ray( vec3_cref_t orig, vec3_cref_t dir )
                {
                    _origin = orig ;
                    _dir = dir ;
                }

                vec3_cref_t get_origin( void_t ) const { return _origin ; }
                vec3_cref_t get_direction( void_t ) const { return _dir ; }

                vec3_t point_at( type_t dist ) const { return _origin + _dir * dist ; }
            };
            typedef ray< float_t > ray3f_t ;
        }
    }
}