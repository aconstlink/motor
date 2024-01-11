#pragma once

#include "typedefs.h"

namespace motor
{
    namespace tool
    {
        struct time_info
        {
            size_t max_milli = 0 ;
            size_t cur_milli = 0 ;
        };
        motor_typedef( time_info ) ;
    }
}