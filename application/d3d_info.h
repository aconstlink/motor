#pragma once

#include "../typedefs.h"

namespace motor
{
    namespace application
    {
        struct d3d_version
        {
            int_t major = 11 ;
            int_t minor = 0 ;
        };

        struct d3d_info
        {
            d3d_version version ;

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
        motor_typedef( d3d_info ) ;
    }
}