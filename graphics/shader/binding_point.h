
#pragma once

#include <motor/std/string>

namespace motor
{
namespace graphics
{
enum class binding_point
{
    undefined,
    object_matrix,
    world_matrix,
    projection_matrix, // mat4()
    view_matrix,       // mat4()
    camera_matrix,     // mat3()
    camera_position,   // vec3( x, y, z )
    camera_clip,       // vec4( n, f, 1/n, 1/f )
    light_direction,   // vec3( x, y, z )
    light_projection,  // mat4
    light_view,        // mat4
    light_shadow_map,  // tex2d -> try shadow texture(depth texture)
    viewport_dimension // vec4( x, y, w, h )
};

namespace detail
{
static char const * const __binding_point_names[] = { "undefined", "projection_matrix",
    "view_matrix", "camera_matrix", "camera_position", "camera_clip", "light_direction",
    "light_projection", "light_view", "light_shadow_map", "viewport_dimension" };
}

static motor::string_t to_string( binding_point const bp ) noexcept
{
    return detail::__binding_point_names[ size_t( bp ) ];
}
} // namespace graphics
} // namespace motor