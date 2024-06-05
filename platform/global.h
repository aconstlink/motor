

#pragma once

#include "api.h"
#include "typedefs.h"

#include <motor/application/carrier.h>
#include <motor/memory/global.h>

namespace motor
{
    namespace platform
    {
        class MOTOR_PLATFORM_API global
        {
            motor_this_typedefs( global ) ;

        public:

            static motor::application::carrier_mtr_safe_t create_carrier( void_t ) noexcept ;
            static motor::application::carrier_mtr_safe_t create_carrier( motor::application::app_mtr_safe_t ) noexcept ;

            template< typename T >
            static int_t create_and_exec( void_t ) noexcept
            {
                auto ret = this_t::create_and_exec_carrier_( motor::shared( T() ) ) ;
                // must be called here because the T() object is just ~T called
                // when the previous function returns.
                motor::memory::global::dump_to_std() ;
                return ret ;
            }

            template< typename T >
            static int_t create_and_exec( int argc, char ** argv ) noexcept
            {
                auto ret = this_t::create_and_exec_carrier_( motor::shared( T( argc, argv ) ) ) ;
                // must be called here because the T() object is just ~T called
                // when the previous function returns.
                motor::memory::global::dump_to_std() ;
                return ret ;
            }

        private: 

            static int_t create_and_exec_carrier_( motor::application::app_mtr_safe_t ) noexcept ;

        };
        motor_typedef( global ) ;
    }
}