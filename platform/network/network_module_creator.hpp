

#pragma once

#ifdef MOTOR_TARGET_OS_WIN
#include "win32/win32_net_module.h"
#endif

namespace motor { namespace platform 
{
    struct network_module_creator
    {
        static motor::network::imodule_ptr_t create( void_t ) noexcept
        {
            #ifdef MOTOR_TARGET_OS_WIN
            return motor::memory::global_t::alloc( motor::platform::win32::win32_net_module_t(), 
                "[network_module_creator] : win32 network module" ) ;
            #endif
        }

    };

} }