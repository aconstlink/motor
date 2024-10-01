#pragma once 

#include "leaf.h"

#include <motor/graphics/object/msl_object.h>

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API render_node : public leaf
        {
            motor_typedefs( leaf, base ) ;
            motor_this_typedefs( render_node ) ;
            motor_core_dd_id_fn() ;

        private:

            size_t _vs = 0 ;
            motor::graphics::msl_object_mtr_t _msl = nullptr  ;

        public:

            render_node( this_rref_t ) noexcept ;
            render_node( this_cref_t ) = delete ;
            render_node( motor::graphics::msl_object_mtr_safe_t ) noexcept ;
            render_node( motor::graphics::msl_object_mtr_safe_t, size_t const ) noexcept ;

            virtual ~render_node( void_t ) noexcept ;

        public:

            motor::graphics::msl_object_mtr_t borrow_msl( void_t ) noexcept { return _msl ; }
            size_t get_variable_set_idx( void_t ) const noexcept { return _vs ; }

        } ;
        motor_typedef( render_node ) ;
    }
}