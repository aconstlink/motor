#pragma once

#include "../../typedefs.h"
#include "../../vector/vector4.hpp"

namespace motor
{
    namespace math
    {
        namespace m3d
        {
            template< typename type_t >
            class frustum
            {
            public:

                motor_this_typedefs( frustum< type_t > ) ;

                // just a temporary fix. Dot not know it suffice.
                motor_typedefs( motor::math::vector4< type_t >, plane ) ;

            private:

                plane_t _planes[ 6 ] ;

            public:

                enum frustum_plane
                {
                    p_left,
                    p_right,
                    p_bottom,
                    p_top,
                    p_near,
                    p_far
                } ;

            public:

                frustum( void_t )
                {

                }


                this_ref_t set_plane( frustum_plane fp, plane_cref_t plane )
                {
                    _planes[ fp ] = plane ;
                    return *this ;
                }

                plane_cref_t get_plane( size_t i ) const
                {
                    motor_assert( i < 6 ) ;
                    return _planes[ i ] ;
                }
            };
        }
    }
}