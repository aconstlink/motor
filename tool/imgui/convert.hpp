
#pragma once

#include "../typedefs.h"

#include <motor/math/vector/vector2.hpp>

#include <imgui.h>

namespace motor::tool::imgui
{
struct convert
{
    static ImVec2 from( motor::math::vec2f_cref_t v ) noexcept
    {
        return ImVec2( v.x(), v.y() );
    }

    static motor::math::vec2f_t from( ImVec2 const & v ) noexcept
    {
        return motor::math::vec2f_t( v.x, v.y );
    }
};

} // namespace motor::tool::imgui