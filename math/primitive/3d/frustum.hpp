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

                enum class frustum_plane
                {
                    left_plane,
                    right_plane,
                    bottom_plane,
                    top_plane,
                    near_plane,
                    far_plane,
                    num_planes
                } ;

            public:

                frustum( void_t )
                {

                }


                this_ref_t set_plane( this_t::frustum_plane const fp, plane_cref_t plane )
                {
                    _planes[ size_t( fp ) % size_t(frustum_plane::num_planes) ] = plane ;
                    return *this ;
                }

                plane_cref_t get_plane( this_t::frustum_plane const fp ) const
                {
                    return _planes[ size_t(fp) % size_t(frustum_plane::num_planes) ] ;
                }

                plane_cref_t get_plane( size_t i ) const
                {
                    return _planes[ i % size_t(frustum_plane::num_planes) ] ;
                }
            };
        }
    }
}