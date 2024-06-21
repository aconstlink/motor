
#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "../protos.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API icomponent
        {
            motor_this_typedefs( icomponent ) ;

        public:

            virtual void_t execute( motor::scene::node_mtr_t ) noexcept = 0 ;
        };
    }
}