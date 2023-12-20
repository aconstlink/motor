
#pragma once

#include "../../api.h"
#include "../../typedefs.h"

#include <motor/graphics/backend/icontext.h>
#include <motor/std/string>

namespace motor
{
    namespace platform
    {
        namespace opengl
        {
            class rendering_context 
            {
            public:

                virtual bool_t is_ext_supported( motor::string_cref_t ) const noexcept = 0 ;
            };
            motor_typedef( rendering_context ) ;
        }
    }
}