#pragma once

#include "../api.h"
#include "../protos.h"
#include "../typedefs.h"

namespace natus
{
    namespace application
    {
        struct toggle_window
        {
            bool_t toggle_vsync = false ;
            bool_t toggle_fullscreen = false ;
            bool_t toggle_show_cursor = false ;
        };
        natus_typedef( toggle_window ) ;
    }
}