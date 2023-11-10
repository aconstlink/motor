#pragma once

#include <motor/core/std/string>

namespace motor
{
    namespace log
    {
        enum class log_level
        {
            raw,
            status,
            warning,
            error,
            critical,
            timing
        };

        namespace motor_internal
        {
            static motor::core::string_t const __log_level_strings[] = {
                "raw", "status", "warning", "error", "critical", "timing"
            } ;
            static motor::core::string_t const __log_level_short_strings[] = {
                "", "[s]", "[w]", "[e]", "[c]", "[t]"
            } ;
        }

        static motor::core::string_cref_t to_string( log_level const level ) noexcept
        {
            return motor_internal::__log_level_strings[ size_t( level ) ] ;
        }

        static motor::core::string_cref_t const& to_string_short( log_level const level ) noexcept
        {
            return motor_internal::__log_level_short_strings[ size_t( level ) ] ;
        }
    }
}
