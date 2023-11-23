#pragma once

#include "../../typedefs.h"
#include "../../vector/vector2.hpp"

namespace motor
{
    namespace math
    {
        namespace m2d
        {
            /// assumes the origin of source/destination coords to 
            /// have the origin at the borders.
            /// ndc: normalize device coords 
            /// nnc: negative normalized coords in [-1,1]
            /// pnc: positive normalized coords in [0,1]
            template< typename real_t >
            class coord_transformator
            {
                motor_this_typedefs( coord_transformator< real_t > ) ;

                motor_typedefs( motor::math::vector2<real_t>, vec2r ) ;

            private:

                vec2ui_t _res = vec2ui_t( 1, 1 ) ;

            public:

                coord_transformator( void_t ) {}

                coord_transformator( vec2ui_cref_t res )
                {
                    _res = res ;
                }

                coord_transformator( this_cref_t rhv )
                {
                    _res = rhv._res ;
                }

                ~coord_transformator( void_t ) {}

            public:

                this_ref_t operator = ( this_cref_t rhv )
                {
                    _res = rhv._res ;
                    return *this ;
                }

            public: // transformation

                /// transforms from [0,1] to [-1,1]
                vec2r_t pnc_to_nnc( vec2r_cref_t ndc )
                {
                    return ndc * vec2r_t( 2.0 ) - vec2r_t( 1.0 ) ;
                }

                /// transforms from [-1,1] to [0,1]
                vec2r_t nnc_to_pnc( vec2r_cref_t ndc )
                {
                    return ndc * vec2r_t( 0.5 ) + vec2r_t( 0.5 ) ;
                }

            public: // from 

                /// @point in source space coords
                /// @return pndc
                vec2r_t to_pnc( vec2ui_cref_t point )
                {
                    return vec2r_t( point ) / vec2r_t( _res )  ;
                }

                /// @point in source space coords
                vec2r_t to_nnc( vec2ui_cref_t point )
                {
                    return pnc_to_nnc( to_pnc( point ) )  ;
                }
            };

            motor_typedefs( coord_transformator< float_t >, coord_transformatorf ) ;
        }
    }
}