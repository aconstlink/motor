

#pragma once

#include "../api.h"
#include "../typedefs.h"

#include "../render_engine.h"

namespace motor
{
    namespace graphics
    {
        class ifrontend
        {
        public:
            
            virtual ~ifrontend( void_t ) noexcept {}

            virtual bool_t can_enter_frame( void_t ) const noexcept = 0 ;
            virtual motor::graphics::render_engine_ptr_t borrow_render_engine( void_t ) noexcept = 0 ;
        };
        motor_typedef( ifrontend ) ;
    }
}