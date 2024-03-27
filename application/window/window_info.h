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
            gen4_es3,
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
        };
        motor_typedef( window_info ) ;
    }
}