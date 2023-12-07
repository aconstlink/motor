
#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "../protos.h"

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API icontext 
        {
        public:

            virtual ~icontext( void_t ) noexcept ;
        };
        motor_typedef( icontext ) ;
    }
}