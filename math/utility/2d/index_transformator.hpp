#pragma once

#include "../../typedefs.h"
#include "../../vector/vector2.hpp"

namespace motor
{
	namespace math
	{
        namespace m2d
        {
            template< typename type_t, bool uniform >
            struct index_transformator { };

            template< typename type_t >
            struct index_transformator< type_t, true >
            {
                typedef motor::math::vector2< type_t > vec2_t ;
                typedef vec2_t const& vec2_cref_t ;

                static vec2_t from_1d_to_2d( type_t index, type_t dim )
                {
                    return vec2_t( index % dim, ( index / dim ) % dim ) ;
                }

                static type_t from_2d_to_1d( vec2_cref_t index, type_t dim )
                {
                    return index.y() * dim + index.x() ;
                }

                static type_t from_2d_to_1d( type_t i, type_t j, type_t dim )
                {
                    return j * dim + i ;
                }
            };


            template< typename type_t, bool uniform >
            struct index_transformator_base_2
            {
                typedef motor::math::vector2< type_t > vec2_t ;
                typedef vec2_t const& vec2_cref_t ;
            };

            ///
            /// usable for indices in 2^e space.
            /// so this allows for 1x1, 2x2, 4x4, 8x8, 16x16, ... indices.
            template< typename type_t >
            struct index_transformator_base_2< type_t, true >
            {
                typedef motor::math::vector2< type_t > vec2_t ;
                typedef vec2_t const& vec2_cref_t ;

                /// @param index [in] must be in [0,2^e*2^e]
                static vec2_t from_1d_to_2d( type_t index, type_t const e )
                {
                    type_t const w = type_t( 1 ) << e ;
                    type_t const h = w ;

                    type_t const wm1 = w - type_t( 1 ) ;
                    type_t const hm1 = h - type_t( 1 ) ;

                    return vec2_t( index & wm1, index >> e & hm1 ) ;
                }

                /// @param index [in] must be in [0,2^e] for each component
                static type_t from_2d_to_1d( vec2_cref_t index, type_t const e )
                {
                    type_t const w = type_t( 1 ) << e ;
                    type_t const h = w ;

                    type_t const wm1 = w - type_t( 1 ) ;
                    type_t const hm1 = h - type_t( 1 ) ;

                    type_t const x = index.x() & wm1 ;
                    type_t const y = index.y() & hm1 ;

                    return y << e | x ;
                }
            };
        }
    }
}