#pragma once 

#include "decorator.h"

#include <motor/graphics/object/state_object.h>

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API render_settings : public decorator
        {
            motor_this_typedefs( render_settings ) ;
            motor_core_dd_id_fn() ;

        private:

            motor::graphics::state_object_mtr_t _rs = nullptr  ;

        public:
            
            render_settings( this_rref_t ) noexcept ;
            render_settings( this_cref_t ) = delete ;
            render_settings( node_ptr_t ) noexcept ;
            render_settings( motor::graphics::state_object_mtr_safe_t ) noexcept ;
            render_settings( motor::graphics::state_object_mtr_safe_t, node_ptr_t ) noexcept ;

            virtual ~render_settings( void_t ) noexcept ;

        public:

            void_t set_decorated( node_mtr_safe_t nptr, bool_t const reset_parent ) noexcept ;
            void_t set_decorated( node_mtr_safe_t nptr ) noexcept ;

            motor::graphics::state_object_mtr_t borrow_state( void_t ) noexcept { return _rs ; }
        } ;
        motor_typedef( render_settings ) ;

    }
}