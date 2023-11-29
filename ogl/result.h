#pragma once

#include "typedefs.h"
#include <motor/std/string>

namespace motor
{
    namespace ogl
    {
        enum class result
        {
            ok,
            failed,
            failed_load_function,
            num_results
        };

        typedef result* result_ptr_t ;

        namespace detail
        {
            static char const * const __graphics_result_strings[] = {
                "motor::ogl::result"
            } ;
        }

        /// not implemented yet.
        static motor::string_t to_string( motor::ogl::result /*res*/ )
        {
            return motor::ogl::detail::__graphics_result_strings[ 0 ] ;
        }

        static bool_t success( motor::ogl::result res )
        {
            return res == motor::ogl::result::ok ;
        }

        static bool_t no_success( motor::ogl::result res )
        {
            return !success( res ) ;
        }
    }
}