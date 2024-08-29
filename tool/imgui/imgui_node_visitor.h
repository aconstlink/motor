#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/scene/visitor/ivisitor.h>
#include <motor/std/string>

namespace motor
{
    namespace tool
    {
        class MOTOR_TOOL_API imgui_node_visitor : public motor::scene::ivisitor
        {
            motor_this_typedefs( imgui_node_visitor ) ;

        private:

            int_t _id = 0;
            size_t _depth = 0 ;

        public:

            imgui_node_visitor( void_t ) noexcept ;
            virtual ~imgui_node_visitor( void_t ) noexcept ;

            virtual motor::scene::result visit( motor::scene::node_ptr_t ) noexcept ;
            virtual motor::scene::result visit( motor::scene::group_ptr_t ) noexcept  ;
            virtual motor::scene::result post_visit( motor::scene::group_ptr_t, motor::scene::result const  ) noexcept  ;
            virtual motor::scene::result visit( motor::scene::leaf_ptr_t ) noexcept ;

            virtual void_t on_finish( void_t ) noexcept ;

        private:

            motor::string_t check_for_name( motor::string_rref_t, motor::scene::node_ptr_t ) const noexcept ;
        };
        motor_typedef( imgui_node_visitor ) ;
    }
}