
#pragma once

#include "icomponent.h"

#include <motor/graphics/object/msl_object.h>

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API msl_component : public icomponent
        {
            motor_this_typedefs( msl_component ) ;

        public:

            msl_component( motor::graphics::msl_object_mtr_safe_t ) noexcept ;
            virtual ~msl_component( void_t ) noexcept ;
        };
        motor_typedef( msl_component ) ;
    }
}