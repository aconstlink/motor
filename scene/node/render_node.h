#pragma once 

#include "leaf.h"

#include <motor/graphics/object/msl_object.h>
#include <motor/graphics/variable/wire_variable_bridge.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/gfx/camera/generic_camera.h>

#include <motor/wire/slot/sheet.hpp>

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

            motor::graphics::compilation_listener_mtr_t _comp_lst =
                motor::shared( motor::graphics::compilation_listener(), "render_node comp listener" ) ;

            size_t _vs = 0 ;
            motor::graphics::msl_object_mtr_t _msl = nullptr  ;
            motor::graphics::variable_set_mtr_t _var_set = nullptr ;
            
        private: // camera variables

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

        private: // trafo variables

            struct trafo_variables
            {
                motor::wire::output_slot< motor::math::mat4f_t > * world = 
                    motor::shared( motor::wire::output_slot< motor::math::mat4f_t >() );

                trafo_variables( void_t ) noexcept{}
                trafo_variables( trafo_variables && rhv ) noexcept
                {
                    motor::release( motor::move( world ) ) ;
                    world = motor::move( rhv.world ) ;
                }
                ~trafo_variables( void_t ) noexcept 
                {
                    trafo_variables::clear() ;
                }
                void_t clear( void_t ) noexcept
                {
                    motor::release( motor::move( world ) ) ;
                }
            };

            trafo_variables _trafo_vars ;

        private:

            motor::graphics::wire_variable_bridge_t _brigde ;

        public:
            
            render_node( this_rref_t ) noexcept ;
            render_node( this_cref_t ) = delete ;
            render_node( motor::graphics::msl_object_mtr_safe_t ) noexcept ;
            render_node( motor::graphics::msl_object_mtr_safe_t, size_t const ) noexcept ;

            virtual ~render_node( void_t ) noexcept ;

        public:

            motor::graphics::msl_object_mtr_t borrow_msl( void_t ) noexcept { return _msl ; }
            size_t get_variable_set_idx( void_t ) const noexcept { return _vs ; }

        public: // render interface

            void_t render_update( motor::gfx::generic_camera_ptr_t ) noexcept ;

        public: // transformation interface

            void_t set_world( motor::math::m3d::trafof_cref_t ) noexcept ;

        public: // inputs

            motor::wire::inputs_cptr_t borrow_shader_inputs( void_t ) const noexcept ;
            motor::wire::inputs_ptr_t borrow_shader_inputs( void_t ) noexcept ;

        private:

            void_t update_bindings( void_t ) noexcept ;
            void_t update_camera( motor::gfx::generic_camera_ptr_t ) noexcept ;
            void_t prefill_bridge( void_t ) noexcept ;
        } ;
        motor_typedef( render_node ) ;
    }
}