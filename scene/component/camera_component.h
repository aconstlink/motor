
#pragma once

#include "icomponent.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API camera_component : public icomponent
        {
            motor_this_typedefs( camera_component ) ;
        };
        motor_typedef( camera_component ) ;
    }
}