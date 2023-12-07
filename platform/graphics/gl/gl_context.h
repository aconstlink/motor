
#pragma once

#include "../../api.h"
#include "../../typedefs.h"

#include <motor/graphics/backend/icontext.h>
#include <motor/std/string>

namespace motor
{
    namespace platform
    {
        class MOTOR_PLATFORM_API gl_context : public motor::graphics::icontext
        {
        public:

            virtual ~gl_context( void_t ) noexcept {}

        public:

            virtual bool_t is_extension_supported( motor::string_cref_t ) const noexcept = 0 ;
        };
        motor_typedef( gl_context ) ;
    }
}