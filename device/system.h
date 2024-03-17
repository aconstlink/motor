#pragma once

#include "api.h"
#include "typedefs.h"
#include "imodule.h"
#include "iobserver.hpp"

#include "modules/vdev_module.h"

#include <motor/std/vector>

namespace motor
{
    namespace device
    {
        class MOTOR_DEVICE_API system
        {
            motor_this_typedefs( system ) ;

        private:

            motor::vector< imodule_mtr_t > _modules ;
            motor::device::vdev_module_mtr_t _vdev ;

        public:

            system( void_t ) noexcept ;
            system( this_cref_t ) = delete ;
            system( this_rref_t ) noexcept ;
            ~system( void_t ) noexcept ;

        public:

            void_t add_module( motor::device::imodule_mtr_safe_t ) noexcept ;

            void_t search( motor::device::imodule::search_funk_t ) noexcept ;
            void_t update( void_t ) noexcept ;
            void_t release( void_t ) noexcept ;

            void_t install( motor::device::iobserver_mtr_t ) noexcept ;
        };
        motor_typedef( system ) ;
    }
}