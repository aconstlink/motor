
#pragma once

#include "icomponent.h"

#include <motor/graphics/object/state_object.h>

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API render_settings_component : public icomponent
        {
            motor_this_typedefs( render_settings_component ) ;

        private:

            motor::graphics::state_object_mtr_t _rs = nullptr  ;

        public:

            render_settings_component( this_rref_t ) noexcept ;
            render_settings_component( this_cref_t ) = delete ;
            render_settings_component( node_ptr_t ) noexcept ;
            render_settings_component( motor::graphics::state_object_mtr_safe_t ) noexcept ;
            render_settings_component( motor::graphics::state_object_mtr_safe_t, node_ptr_t ) noexcept ;

            virtual ~render_settings_component( void_t ) noexcept ;

            motor::graphics::state_object_mtr_t borrow_state( void_t ) noexcept { return _rs ; }
        };
        motor_typedef( render_settings_component ) ;
    }
}