#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/wire/node/node.h>

#include <imgui.h>
#include <imnodes.h>

namespace motor
{
    namespace tool
    {
        class MOTOR_TOOL_API imnodes_wire
        {
            motor_this_typedefs( imnodes_wire ) ;

        private:

        public:

            imnodes_wire(  void_t ) noexcept ;
            ~imnodes_wire( void_t ) noexcept ;

        public:

            void_t build( motor::wire::inode_mtr_t start ) noexcept ;


        };
        motor_typedef( imnodes_wire ) ;
    }
}