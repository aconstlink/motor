#pragma once

#include "api.h"
#include "typedefs.h"
#include "device.hpp"
#include "iobserver.hpp"

namespace motor
{
    namespace controls
    {
        class MOTOR_CONTROLS_API imodule
        {
        public:

            virtual ~imodule( void_t ) noexcept {}

        public:

            // this search funk requires the user to copy a device pointer if the user
            // will hold on to the pointer. Its a managed pointer. note: _mtr_!
            typedef std::function< void_t ( motor::controls::device_borrow_t::mtr_t ) > search_funk_t ;

            virtual void_t search( search_funk_t ) noexcept = 0 ;
            virtual void_t update( void_t ) noexcept = 0 ;
            virtual void_t release( void_t ) noexcept = 0 ;
            virtual void_t install( motor::controls::iobserver_mtr_t ) noexcept {}
        };
        motor_typedef( imodule ) ;
    }
}