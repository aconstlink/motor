// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT


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
            virtual motor::log::result test( motor::string_cref_t msg, bool_t const passed ) noexcept
            {
                return this->log( passed ? log_level::status : log_level::error,
                    motor::string_t( passed ? "[OK] " : "[FAIL] " ) + msg ) ;
            }
        };
    }
}
