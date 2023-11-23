#pragma once

#include "../../typedefs.h"

#include "line_segment.hpp"
#include "line.hpp"
#include "ray.hpp"

#include "../../vector/vector2.hpp"

namespace motor
{
    namespace math
    {
        namespace m2d
        {
            template< typename type_t >
            class ray_line_intersection
            {
            public:

                typedef motor::math::m2d::ray< type_t > ray_t ;
                typedef ray_t const& ray_cref_t ;

                typedef motor::math::m2d::line< type_t > line_t ;
                typedef line_t const& line_cref_t ;

                typedef motor::math::vector2< type_t > vec2_t ;
                typedef vec2_t const& vec2_cref_t ;

            public:

                static type_t intersect( type_t at_distance, ray_cref_t r, line_cref_t l )
                {
                    const type_t e = std::numeric_limits<type_t>::epsilon() ;
                    const vec2_t o = r.get_origin() ;
                    const vec2_t d = r.get_direction() ;

                    type_t proj_dir = l.onto_normal_project( d ) ;
                    proj_dir += std::abs( proj_dir ) < e ? e : type_t( 0 ) ;

                    return ( at_distance - l.distance_signed( o ) ) / proj_dir  ;
                }
            };
        }
    }
}