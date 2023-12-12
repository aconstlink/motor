

#pragma once

#include "api.h"
#include "typedefs.h"

#include <motor/application/carrier.h>

namespace motor
{
    namespace platform
    {
        class MOTOR_PLATFORM_API global
        {
            motor_this_typedefs( global ) ;

        public:

            static motor::application::carrier_mtr_unique_t create_carrier( void_t ) noexcept ;
            static motor::application::carrier_mtr_unique_t create_carrier( motor::application::iapp_mtr_shared_t ) noexcept ;
            static motor::application::carrier_mtr_unique_t create_carrier( motor::application::iapp_mtr_unique_t ) noexcept ;
            static int_t create_and_exec_carrier( motor::application::iapp_mtr_shared_t ) noexcept ;
            static int_t create_and_exec_carrier( motor::application::iapp_mtr_unique_t ) noexcept ;
        };
        motor_typedef( global ) ;
    }
}