#pragma once

#include "../../typedefs.h"
#include "../../vector/vector2.hpp"
#include "../../vector/vector4.hpp"

namespace motor
{
    namespace math
    {
        namespace m2d
        {
            class viewport
            {
            private:

                size_t _x ;
                size_t _y ;
                size_t _width ;
                size_t _height ;

            public:

                viewport( void )
                {
                    set( 0, 0, 0, 0 ) ;
                }

                viewport( size_t n )
                {
                    set( n, n, n, n ) ;
                }

                viewport( size_t start_x, size_t start_y, size_t width, size_t height )
                {
                    set( start_x, start_y, width, height ) ;
                }

                inline void set( size_t start_x, size_t start_y, size_t width, size_t height )
                {
                    _x = start_x ; _y = start_y ; _width = width ; _height = height ;
                }

            public:

                size_t get_x( void_t ) const { return _x ; }
                size_t get_y( void_t ) const { return _y ; }
                size_t get_width( void_t ) const { return _width ; }
                size_t get_height( void_t ) const { return _height ; }

                template< typename type_t >
                type_t get_width( void_t ) const { return static_cast< type_t >( _width ) ; }

                template< typename type_t >
                type_t get_height( void_t ) const { return static_cast< type_t >( _height ) ; }

                template< typename type_t >
                motor::math::vector2< type_t > get_width_height( void_t ) const
                {
                    return motor::math::vector2< type_t >( type_t( _width ), type_t( _height ) ) ;
                }

                template< typename type_t >
                motor::math::vector2< type_t > get_xy( void_t ) const
                {
                    return motor::math::vector2< type_t >( type_t( _x ), type_t( _y ) ) ;
                }

                template< typename type_t >
                motor::math::vector4< type_t > get_as_vec4( void_t ) const
                {
                    return motor::math::vector4< type_t >( type_t( _x ), type_t( _y ), type_t( _width ), type_t( _height ) ) ;
                }
            } ;
        }
    }
    motor_typedefs( motor::math::m2d::viewport, viewport_2d ) ;
}