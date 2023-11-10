
#pragma once

#include <motor/core/api.h>

#include "result.h"
#include "log_level.h"

namespace motor
{
    namespace log
    {
        class MOTOR_CORE_API ilogger
        {
        public:
            virtual ~ilogger( void_t ) {}
            virtual motor::log::result log( motor::log::log_level const ll, motor::core::string_cref_t msg ) noexcept = 0 ;
        };
    }
}
