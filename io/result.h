#pragma once

#include <motor/std/string>

namespace motor
{
    namespace io
    {
        enum class result
        {
            ok,
            failed,
            processing,
            state_change_failed,
            file_does_not_exist,
            invalid,
            invalid_argument,
            unknown,
            invalid_handle,
            cached_data,
            num_results
        };

        static bool success( result r ) { return r == motor::io::result::ok ; }
        static bool no_success( result r ) { return !success( r ) ; }

        namespace motor_internal
        {
            static char const * const __result_strings[] = {
                "ok", "failed", "processing", "state_change_failed",
                "file_does_not_exist", "invalid", "invalid_argument", "unknown", "invalid_handle"
            } ;
        }

        static motor::string_t to_string( motor::io::result const r ) noexcept
        {
            return size_t( r ) < size_t( motor::io::result::num_results ) ?
                motor_internal::__result_strings[ size_t( r ) ] :
                motor_internal::__result_strings[ size_t( motor::io::result::num_results ) - 1 ] ;
        }
    }
}