
#pragma once

#include "typedefs.h"

#include "midi_message.hpp"
#include <motor/log/global.h>

namespace motor
{
    namespace controls
    {
        class iobserver
        { 
        public:
            virtual ~iobserver( void_t ) noexcept{}
        };
        motor_typedef( iobserver ) ;

    }

}
