#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "convert.hpp"

#include <imgui.h>

namespace motor::tool::imgui
{
using namespace motor::core::types;

// pannable and
class MOTOR_TOOL_API canvas_2d
{

    motor_this_typedefs( canvas_2d );

  private:

    
    bool_t _in_section = false;


    motor::math::mat4f_t _proj ;

    // what is in view
    // xy: center
    // zw: width/height
    motor::math::vec4f_t _view_region = motor::math::vec4f_t( 0.0f, 0.0f, 100.0f, 100.0f );

    // the maximum region of the world
    // xy: bottom left
    // zw: top right
    motor::math::vec4f_t _world_region;

    motor::math::vec2f_t _world_center ;
    float_t _zoom = 1.0f ;

    motor::math::vec2f_t view_center( void_t ) const noexcept
    {
        return _view_region.xy();
    }

    motor::math::vec2f_t view_dims( void_t ) const noexcept
    {
        return _view_region.zw();
    }

    ImVec2 _start_screen_pos ;
    ImVec2 _available_region ;
    ImVec2 world_to_screen( motor::math::vec2f_cref_t const & ) noexcept ;
    motor::math::vec2f_t  screen_to_world( ImVec2 const & ) noexcept ;

  public:

    canvas_2d( void_t ) noexcept;
    canvas_2d( this_cref_t ) = delete;
    canvas_2d( this_rref_t ) noexcept;
    ~canvas_2d( void_t ) noexcept;

  public:

#if 0
    motor::math::vec4f_cref_t view_region( void_t ) const noexcept
    {
        return _view_region;
    }
#endif

    motor::math::vec4f_cref_t wrld_region( void_t ) const noexcept
    {
        return _world_region;
    }

    bool_t begin( char const * const name, ImVec2 const & ) noexcept;
    void_t end( void_t ) noexcept;

  public:

    struct draw_line_info
    {
        motor::math::vec2f_t start ;
        motor::math::vec2f_t end ;
    };
    void_t draw_line( this_t::draw_line_info const & dl ) noexcept;
};
motor_typedef( canvas_2d );
} // namespace motor::tool::imgui