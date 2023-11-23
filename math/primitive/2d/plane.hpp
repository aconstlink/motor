#pragma once

#include "../../vector/vector3.hpp"
#include "../../utility/fn.hpp"

namespace motor
{
    namespace math
    {
        namespace m2d
        {
            // stores the plane in hesse form
            // that is, a plane normal + the ortho distance negative
            template<typename type_t>
            class plane
            {
                motor_this_typedefs( plane< type_t > ) ;
                motor_typedefs( motor::math::vector2< type_t >, vec2 ) ;
                motor_typedefs( motor::math::vector3< type_t >, vec3 ) ;

            private:

                // xy : normal
                // z: distance to origin
                vec3_t _n ;

            public:

                plane( void_t ) noexcept {}

                plane( this_cref_t rhv ) noexcept : _n( rhv._n ) {}

                // construct at some point
                // @param normal must be normalized
                plane( vec2_cref_t origin, vec2_cref_t normal )
                {
                    _n = vec3_t( normal, -normal.dot(origin) ) ;
                }

                vec3_cref_t get_plane( void_t ) const { return _n ; }

                type_t distance( vec2_cref_t p ) const
                {
                    return n.dot( vec3_t( p, type_t(1) ) ;
                }
            };
        }
    }
}