

#pragma once

#if defined (MOTOR_TARGET_OS_WIN)
#include "win32/win32_net_module.h"
#elif defined( MOTOR_TARGET_OS_LIN)
#include "unix/unix_net_module.h"
#endif

namespace motor { namespace platform 
{
    struct network_module_creator
    {
        static motor::network::imodule_mtr_safe_t create( void_t ) noexcept
        {
            #if defined (MOTOR_TARGET_OS_WIN)

            return motor::shared( motor::platform::win32::win32_net_module_t(), 
                "[network_module_creator] : win32 network module" ) ;

            
            #elif defined( MOTOR_TARGET_OS_LIN)

            return motor::shared( motor::platform::unx::unix_net_module_t(),
                "[network_module_creator] : unix network module" ) ;

            #endif
        }
    };
} }