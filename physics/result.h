#pragma once

#include "typedefs.h"

#include <string>

namespace motor
{
    namespace physics
    {
        enum class result
        {
            ok,
            failed,
            invalid_argument
        };

        namespace detail
        {
            static char const * const __result_strings[] = {
                "need to fill"
            } ;
        }

        static bool_t success( result r ) { return r == motor::physics::result::ok ; }
        static bool_t no_success( result r ) { return !success( r ) ; }
    }
}