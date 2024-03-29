

#pragma once

#include "api.h"
#include "typedefs.h"

namespace motor
{
    namespace audio
    {
        // tagging interface
        class MOTOR_AUDIO_API ifrontend
        {
        public:
            
            virtual ~ifrontend( void_t ) noexcept {}
        };
        motor_typedef( ifrontend ) ;
    }
}