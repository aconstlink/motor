
#pragma once

#include "api.h"

#include "result.h"
#include "log_level.h"

namespace motor
{
    namespace log
    {
        class MOTOR_LOG_API ilogger
        {
        public:
            virtual ~ilogger( void_t ) {}
            virtual motor::log::result log( motor::log::log_level const ll, motor::string_cref_t msg ) noexcept = 0 ;
            virtual motor::log::result log( motor::log::log_level const ll, char const * msg ) noexcept = 0 ;
        };
    }
}
