#pragma once

#include "result.h"
#include "log_level.h"

#include <string>

namespace motor
{
    namespace log
    {
        class isystem
        {
        public:

            virtual ~isystem( void_t ) {}
            virtual motor::log::result log( motor::log::log_level const ll, motor::core::string_cref_t msg ) noexcept = 0 ;
            virtual motor::log::result add_logger( motor::log::ilogger_ptr_t lptr ) noexcept = 0 ;
            virtual motor::log::ilogger_ptr_t set_default_logger( motor::log::ilogger_ptr_t lptr ) noexcept = 0 ;
        };
    }
}

