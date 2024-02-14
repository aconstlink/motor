#pragma once

#include <string>
#include "typedefs.h"

namespace motor
{
    namespace geometry
    {
        enum result
        {
            ok,
            failed,
            invalid,
            invalid_argument,
            processing,
            not_ready,
        };

        namespace detail
        {
            static const std::string __graphics_result_strings[] = {
                "geometry::result"
            } ;
        }

        /// not implemented yet.
        static std::string const& to_string( result /*res*/ )
        {
            return detail::__graphics_result_strings[ 0 ] ;
        }

        static bool_t success( motor::geometry::result res )
        {
            return res == ok ;
        }

        static bool_t no_success( result res )
        {
            return !success( res ) ;
        }
    }
}