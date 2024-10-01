#pragma once

#include "../../api.h"
#include "../../typedefs.h"
#include "imgui_node_component.h"

#include <motor/scene/visitor/ivisitor.h>
#include <motor/std/string>

namespace motor
{
    namespace tool
    {
        class MOTOR_TOOL_API imgui_node_visitor : public motor::scene::ivisitor
        {
            motor_this_typedefs( imgui_node_visitor ) ;
            motor_core_dd_id_fn() ;

        private:

            int_t _id = 0;
            size_t _depth = 0 ;

            motor::scene::node_mtr_t _selected_node = nullptr ;

        public:

            imgui_node_visitor( void_t ) noexcept ;
            imgui_node_visitor( motor::scene::node_mtr_safe_t selected ) noexcept ;
            virtual ~imgui_node_visitor( void_t ) noexcept ;

            virtual motor::scene::result visit( motor::scene::ivisitable_ptr_t ) noexcept ;
            virtual motor::scene::result post_visit( motor::scene::ivisitable_ptr_t, motor::scene::result const ) noexcept ;
            
            virtual motor::scene::result visit( motor::scene::group_ptr_t ) noexcept  ;
            virtual motor::scene::result post_visit( motor::scene::group_ptr_t, motor::scene::result const  ) noexcept  ;
            virtual motor::scene::result visit( motor::scene::leaf_ptr_t ) noexcept ;
            virtual motor::scene::result visit( motor::scene::decorator_ptr_t ) noexcept ;
            virtual motor::scene::result post_visit( motor::scene::decorator_ptr_t, motor::scene::result const ) noexcept ;

            virtual motor::scene::result visit( motor::scene::render_settings_ptr_t ) noexcept ;
            virtual motor::scene::result post_visit( motor::scene::render_settings_ptr_t, motor::scene::result const ) noexcept ;

            virtual motor::scene::result visit( motor::scene::render_node_ptr_t ) noexcept ;

            virtual void_t on_finish( void_t ) noexcept ;

        public:

            static void_t init_function_callbacks( void_t ) noexcept ;

            motor::scene::node_mtr_safe_t get_selected( void_t ) noexcept ; 

        private:

            motor::string_t check_for_name( motor::string_rref_t, motor::scene::node_ptr_t ) const noexcept ;
            void_t list_components( motor::scene::node_ptr_t ) noexcept ;

            motor::tool::imgui_node_component_mtr_t check_and_borrow_imgui_component( motor::scene::node_ptr_t  ) noexcept ;
            void_t check_selected_item( motor::scene::node_ptr_t ) noexcept ;
            bool_t is_selected_item( motor::scene::node_ptr_t ) noexcept ;
        };
        motor_typedef( imgui_node_visitor ) ;
    }
}