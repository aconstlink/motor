#pragma once 

#include "leaf.h"

#include <motor/graphics/object/msl_object.h>
#include <motor/gfx/camera/generic_camera.h>

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
            motor::graphics::variable_set_mtr_safe_t _var_set ;
            
            struct camera_variables
            {
                motor::graphics::data_variable< motor::math::mat4f_t > * proj ;
                motor::graphics::data_variable< motor::math::mat4f_t > * cam ;
                motor::graphics::data_variable< motor::math::mat4f_t > * view ;
                motor::graphics::data_variable< motor::math::vec3f_t > * cam_pos ;

                void_t clear( void_t ) noexcept 
                {
                    proj = nullptr ;
                    cam = nullptr ;
                    view = nullptr ;
                    cam_pos = nullptr ;
                }
            };

            camera_variables _cam_vars ;

        public:
            
            render_node( this_rref_t ) noexcept ;
            render_node( this_cref_t ) = delete ;
            render_node( motor::graphics::msl_object_mtr_safe_t ) noexcept ;
            render_node( motor::graphics::msl_object_mtr_safe_t, size_t const ) noexcept ;

            virtual ~render_node( void_t ) noexcept ;

        public:

            motor::graphics::msl_object_mtr_t borrow_msl( void_t ) noexcept { return _msl ; }
            size_t get_variable_set_idx( void_t ) const noexcept { return _vs ; }

            void_t update_bindings( void_t ) noexcept ;
            void_t update_camera( motor::gfx::generic_camera_ptr_t ) noexcept ;

        } ;
        motor_typedef( render_node ) ;
    }
}