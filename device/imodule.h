#pragma once

#include "api.h"
#include "typedefs.h"
#include "device.hpp"

namespace motor
{
    namespace device
    {
        class MOTOR_DEVICE_API imodule
        {
        public:

            virtual ~imodule( void_t ) noexcept {}

        public:

            // this search funk requires the user to copy a device pointer if the user
            // will hold on to the pointer. Its a managed pointer. note: _mtr_!
            typedef std::function< void_t ( motor::device::idevice_mtr_t ) > search_funk_t ;

            virtual void_t search( search_funk_t ) noexcept = 0 ;
            virtual void_t update( void_t ) noexcept = 0 ;
            virtual void_t release( void_t ) noexcept = 0 ;
        };
        motor_typedef( imodule ) ;
    }
}