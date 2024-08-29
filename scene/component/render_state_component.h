
#pragma once

#include "icomponent.h"

#include <motor/graphics/object/state_object.h>

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API render_state_component : public icomponent
        {
            motor_this_typedefs( render_state_component ) ;

        private:

            motor::graphics::state_object_t _so ;

        public:

            render_state_component( motor::graphics::render_state_sets_rref_t rss ) noexcept ;
            render_state_component( motor::graphics::state_object_rref_t so ) noexcept ;
            virtual ~render_state_component( void_t ) noexcept ;
        };
        motor_typedef( render_state_component ) ;
    }
}