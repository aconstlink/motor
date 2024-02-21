#pragma once

#include <motor/std/string>

namespace motor
{
    namespace application
    {
        enum class result
        {
            ok,
            failed,
            invalid_argument,
            close,
            terminate,
            no_app,
            failed_wgl,
            failed_glx,
            failed_d3d,
            failed_gfx_context_creation,
            invalid_extension,
            invalid_win32_handle,
            invalid_xlib_handle,
            no_tool,
            num_results
        };

        namespace detail
        {
            /// not implemented yet.
            static char const * const _app_result_strings[] =
            {
                "motor::application::result"
            } ;
        }

        static bool success( motor::application::result const res ) noexcept
        {
            // motor_assert( res < num_results ) ;
            return res == motor::application::result::ok ;
        }

        static bool no_success( motor::application::result const res ) noexcept
        {
            return !success( res );
        }

        static motor::string_t to_string( motor::application::result const /*res*/ ) noexcept
        {
            return motor::application::detail::_app_result_strings[ 0 ] ;
        }
    }
}
