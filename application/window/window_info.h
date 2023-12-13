#pragma once

#include "../typedefs.h"

namespace motor
{    
    namespace application
    {
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
        };
        motor_typedef( window_info ) ;

        struct graphics_window_info
        {
            enum class graphics_api_type
            {
                none,
                gl4,
                d3d11
            };
            graphics_api_type api_type ;
            window_info_t wi ;
        };
        motor_typedef( graphics_window_info ) ;
    }
}