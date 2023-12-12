#pragma once

#include "../api.h"
#include "../typedefs.h"

namespace motor
{
    namespace application
    {
        struct toggle_window
        {
            bool_t toggle_vsync = false ;
            bool_t toggle_fullscreen = false ;
            bool_t toggle_show_cursor = false ;
        };
        motor_typedef( toggle_window ) ;
    }
}