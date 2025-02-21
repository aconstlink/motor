#pragma once

#include <motor/std/string>

namespace motor
{
    namespace platform
    {
        enum class result
        {
            ok,
            failed,
            invalid_argument,

            context_not_active,
            terminate,
            no_app,
            failed_wgl,
            failed_glx,
            failed_d3d,
            failed_gfx_context_creation,
            invalid_extension,
            invalid_win32_handle,
            win32_hdc_failed,

            invalid_xlib_handle,
            no_tool,
            num_results
        };

        namespace detail
        {
            /// not implemented yet.
            static char const * const _result_strings[] =
            {
                "motor::application::result"
            } ;
        }

        static bool success( motor::platform::result const res ) noexcept
        {
            // motor_assert( res < num_results ) ;
            return res == motor::platform::result::ok ;
        }

        static bool no_success( motor::platform::result const res ) noexcept
        {
            return !success( res );
        }

        static motor::string_t to_string( motor::platform::result const /*res*/ ) noexcept
        {
            return motor::platform::detail::_result_strings[ 0 ] ;
        }
    }
}
