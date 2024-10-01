#pragma once

#include "../../api.h"
#include "../../typedefs.h"

#include <motor/scene/component/icomponent.h>

namespace motor
{
    namespace tool
    {
        class imgui_node_component : public motor::scene::icomponent
        {
            motor_this_typedefs( imgui_node_component ) ;

        private:


        public:

            imgui_node_component( void_t ) noexcept ;
            imgui_node_component( this_cref_t ) = delete ;
            imgui_node_component( this_rref_t ) noexcept ;
            virtual ~imgui_node_component( void_t ) noexcept ;

        public:


        };
        motor_typedef( imgui_node_component ) ;
    }
}