#pragma once

#include "../typedefs.h"

namespace motor
{    
    namespace application
    {
        enum class graphics_generation
        {
            none,
            gen4_auto,
            gen4_gl4,
            gen4_d3d11
        };

        struct window_info
        {
            int_t x = 0 ;
            int_t y = 0 ;
            int_t w = 100 ;
            int_t h = 100 ;

            bool_t borderless = false ;
            bool_t fullscreen = false ;
            bool_t show_cursor = true ;
            motor::string_t window_name = motor::string_t( "window name" );

            bool_t show = true ;

            bool_t dpi_aware = false ;

            motor::application::graphics_generation gen = 
                motor::application::graphics_generation::gen4_auto ;

            static window_info create( int_t const x, int_t const y, int_t const w, int_t const h, motor::application::graphics_generation const gen ) noexcept
            {
                window_info wi ;
                wi.x = x ;
                wi.y = y ;
                wi.w = w ;
                wi.h = h ;
                wi.gen = gen ;
                return wi ;
            }
        };
        motor_typedef( window_info ) ;
    }
}