#pragma once

#include "typedefs.h"

#include <motor/core/std/string>

namespace motor
{
    namespace log
    {
        enum class result
        {
            ok,
            failed,
            invalid_argument
        };

        namespace detail
        {
            static const motor::core::string_t __result_strings[] = {
                "need to fill"
            } ;
        }

        static bool_t success( result r ) { return r == motor::log::result::ok ; }
        static bool_t no_success( result r ) { return !success( r ) ; }
    }
}