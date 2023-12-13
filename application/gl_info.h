#pragma once

#include "typedefs.h"

namespace motor
{
    namespace application
    {
        struct gl_version
        {
            int_t major = 4 ;
            int_t minor = 0 ;
        };

        struct gl_info 
        {
            gl_version version ;

            /// RGB color bits.
            size_t color_bits = 24 ;

            /// depth bits.
            size_t depth_bits = 24 ;

            /// Specifies if double buffering should
            /// be used.
            bool_t double_buffer = true ;

            /// Allows to dis-/enable the vsync.
            bool_t vsync_enabled = true ;
        };
        motor_typedef( gl_info ) ;
    }
}