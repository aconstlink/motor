#pragma once

#include "../typedefs.h"

#include <motor/math/utility/fn.hpp>

#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector3.hpp>

#include <vector>

namespace motor { namespace noise
{
    template< typename noise_method_t >
    class fbm
    {
        motor_typedefs( noise_method_t, noise ) ;

        typedef fbm<noise_t> fbm_t ;
        motor_this_typedefs( fbm_t ) ;

        typedef typename noise_t::type_t type_t ;
        
        motor_typedefs( motor::math::vector2< type_t >, vec2 ) ;
        motor_typedefs( motor::math::vector3< type_t >, vec3 ) ;

    public:

        //***************************************************************
        static type_t noise( type_t v, type_t const h, type_t const lacunarity, 
                type_t const octaves, noise_cref_t noi )
        {
            type_t value = type_t(0) ;
            int_t i ;

            int_t const octaves_i = int_t( motor::math::fn<type_t>::floor(octaves) ) ;
            type_t const remainder = octaves - type_t(octaves_i) ;

            for( i=0; i<(int)octaves_i; ++i )
            {
                value += noi.noise( v ) * 
                    motor::math::fn<type_t>::pow(lacunarity, -h*type_t(i)) ;

                v *= lacunarity ;
            }

            value += remainder * noi.noise( v ) * 
                motor::math::fn<type_t>::pow(lacunarity, -h*type_t(i)) ;

            return value ;
        }

        //***************************************************************
        /// original from book tam
        static type_t noise( vec2_cref_t point, type_t h, type_t lacunarity, 
                type_t octaves, noise_cref_t noi )
        {
            type_t value = type_t(0) ;
            int_t i ;

            int_t const octaves_i = int_t( motor::math::fn<type_t>::floor(octaves) ) ;
            type_t const remainder = octaves - type_t(octaves_i) ;

            vec2_t p = point ;

            for( i=0; i<(int)octaves_i; ++i )
            {
                value += noi.noise(p.x(), p.y() ) * 
                    motor::math::fn<type_t>::pow(lacunarity, -h*type_t(i)) ;

                p *= lacunarity ;
            }

            value += remainder * noi.noise( p.x(), p.y() ) * 
                motor::math::fn<type_t>::pow(lacunarity, -h*type_t(i)) ;

            return value ;
        }

        /// original from book tam
        static type_t noise( vec3_cref_t point, type_t h, type_t lacunarity, 
                type_t octaves, noise_cref_t noi )
        {
            type_t value = type_t(0) ;
            int i ;

            const int octaves_i = motor::math::fn<type_t>::floor(octaves) ;
            const type_t remainder = octaves - type_t(octaves_i) ;

            vec3_t p = point ;

            for( i=0; i<(int)octaves_i; ++i )
            {
                value += noi.noise(p.x(), p.y(), p.z() ) * 
                    motor::math::fn<type_t>::pow(lacunarity, -h*type_t(i)) ;

                p *= lacunarity ;
            }

            value += remainder * noi.noise( p.x(), p.y(), p.z() ) * 
                motor::math::fn<type_t>::pow(lacunarity, -h*type_t(i)) ;

            return value ;
        }

        /// my tweeked version.
        static type_t noise2( vec2_cref_t point, type_t h, type_t lacunarity, 
            type_t lac_shift, type_t value_damp, type_t octaves, noise_cref_t noi )
        {
            type_t value = type_t(0) ;
            int_t i ;

            int_t const octaves_i = motor::math::fn<type_t>::floor(octaves) ;
            type_t const remainder = octaves - type_t(octaves_i) ;

            vec2_t p = point ;

            for( i=0; i<(int)octaves_i; ++i )
            {
                value += noi.noise(p.x(), p.y() ) * 
                    motor::math::fn<type_t>::pow(lacunarity+lac_shift, -h*type_t(i)) ;

                value *= value_damp ;
            
                p = (p + vec2_t(0.72356f, -0.12324f)) * lacunarity ;
            }

            value += remainder * noi.noise(p.x(), p.y() ) * 
                motor::math::fn<type_t>::pow(lacunarity, -h*type_t(i)) ;

            return value ;
        }

        /// my tweeked version.
        static type_t noise2( vec3_cref_t point, type_t h, type_t lacunarity, 
            type_t lac_shift, type_t value_damp, type_t octaves, noise_cref_t noi )
        {
            type_t value = type_t(0) ;
            int_t i ;

            int_t const octaves_i = motor::math::fn<type_t>::floor(octaves) ;
            type_t const remainder = octaves - type_t(octaves_i) ;

            vec3_t p = point ;

            for( i=0; i<(int)octaves_i; ++i )
            {
            
                value += noi.noise(p.x(), p.y(), p.z() ) * 
                    motor::math::fn<type_t>::pow(lacunarity+lac_shift, -h*type_t(i)) ;

                value *= value_damp ;
            
                p = (p + vec3f_t(0.72356f, -0.12324f, 1.29764f)) * lacunarity ;
            }

            value += remainder * noi.noise(p.x(), p.y(), p.z() ) * 
                motor::math::fn<type_t>::pow(lacunarity, -h*type_t(i)) ;

            return value ;
        }
    };
} }

