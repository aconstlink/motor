#pragma once

#include "system.h"

namespace motor
{
    namespace device
    {
        class MOTOR_DEVICE_API global
        {
            motor_this_typedefs( global ) ;


        private:

            static motor::device::system_ptr_t _sys ;

        public:

            static motor::device::system_ptr_t system( void_t ) ;
            static void_t deinit( void_t ) noexcept ;

        };
        motor_typedef( global ) ;
    }
}