
#pragma once

#include "../../api.h"
#include "../../typedefs.h"
#include "../../result.h"

#include <motor/std/string>

namespace motor
{
    namespace platform
    {
        namespace opengl
        {
            class MOTOR_PLATFORM_API rendering_context 
            {
                motor_this_typedefs( rendering_context ) ;

            public:

                virtual ~rendering_context( void_t ) noexcept {}

            public:

                virtual motor::platform::result activate( void_t ) noexcept = 0;
                virtual motor::platform::result deactivate( void_t ) noexcept = 0 ;

                virtual this_mtr_safe_t create_shared( void_t ) noexcept = 0 ;
                virtual bool_t is_ext_supported( motor::string_cref_t ) const noexcept = 0 ;
            };
            motor_typedef( rendering_context ) ;
        }
    }
}