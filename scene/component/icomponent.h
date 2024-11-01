
#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "../protos.h"

#include "../ivisitable.hpp"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API icomponent
        {
            motor_this_typedefs( icomponent ) ;

        public:

            virtual ~icomponent( void_t ) noexcept {}
        };
    }
}