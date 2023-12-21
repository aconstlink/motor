#pragma once

#include <motor/std/string>

namespace motor
{
    namespace graphics
    {
        namespace gen4
        {
            // @depricated
            enum class backend_type
            {
                unknown,
                gl4,
                es3,
                d3d11
            };
            static size_t const max_backends = 4 ;

            namespace detail
            {
                char const * const __backend_type_names[] = {
                    "unknown", "gl4", "es3", "d3d11" } ;
            }

            static motor::string_t to_string( motor::graphics::gen4::backend_type const bt ) noexcept
            {
                return detail::__backend_type_names[ size_t( bt ) ] ;
            }
        }
    }
}
