

#pragma once

#include "../api.h"
#include "../typedefs.h"

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API ifrontend
        {
        public:
            
            virtual ~ifrontend( void_t ) noexcept {}
        };
        motor_typedef( ifrontend ) ;
    }
}