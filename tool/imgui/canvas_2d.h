
// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

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

  private: // view management stuff

    bool_t _in_section = false;

    motor::math::mat4f_t _proj;

    // what is in view
    // xy: center
    // zw: width/height
    motor::math::vec4f_t _view_region = motor::math::vec4f_t( 0.0f, 0.0f, 100.0f, 100.0f );

    // the maximum region of the world
    // xy: bottom left
    // zw: top right
    motor::math::vec4f_t _world_region;

    motor::math::vec2f_t _world_center;
    float_t _zoom = 1.0f;

    // caching mouse position if middle mouse buttom
    // for panning for example.
    motor::math::vec2f_t _middle_mouse_pos;

    motor::math::vec2f_t view_center( void_t ) const noexcept
    {
        return _view_region.xy();
    }

    motor::math::vec2f_t view_dims( void_t ) const noexcept
    {
        return _view_region.zw();
    }

    ImVec2 _start_screen_pos;
    ImVec2 _available_region;

    float_t _header_height = 24.0f;

  private: // cache

    motor::vector< ImVec2 > _point_cache;

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

    struct draw_options
    {
        bool_t draw_grid = true;
        bool_t draw_outline = true;
        bool_t draw_grid_coords = true;
        uint_t draw_grid_coords_every = 2;
        ImColor grid_color = ImColor( 0.5f, 0.5f, 0.5f );
    };
    bool_t begin( char const * const name, ImVec2 const & ) noexcept;
    bool_t begin( char const * const name, ImVec2 const &, draw_options const & ) noexcept;
    void_t end( void_t ) noexcept;

    // only valid within begin/end
    ImVec2 world_to_screen( motor::math::vec2f_cref_t const & ) noexcept;

    // only valid within begin/end
    motor::math::vec2f_t screen_to_world( ImVec2 const & ) noexcept;
    motor::math::vec2f_t measure_text_size( char const * text, float_t const height) noexcept;

    float_t zoom( void_t ) const noexcept
    {
        return _zoom;
    }

  public:

    struct draw_line_info
    {
        motor::math::vec2f_t start;
        motor::math::vec2f_t end;
        ImColor color = ImColor( 1.0f, 1.0f, 1.0f );
    };
    void_t draw_line( this_t::draw_line_info const & dl ) noexcept;

    struct draw_point_info
    {
        motor::math::vec2f_t point;
        ImColor color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
    };
    void_t draw_point( draw_point_info const & ) noexcept;

    struct draw_circle_info
    {
        motor::math::vec2f_t point;
        float_t radius = 1.0f;
        ImColor color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
        bool_t filled = true;
    };
    void_t draw_circle( draw_circle_info const & info ) noexcept;

    struct poly_line_info
    {
        float_t thickness;
        motor::vector< motor::math::vec2f_t > points;
        ImColor color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
    };
    void_t draw_poly_line( poly_line_info const & ) noexcept;

    struct polygon_info
    {
        // convex, cw
        motor::vector< motor::math::vec2f_t > points;
        ImColor fill_color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
        ImColor outline_color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
        float_t outline_width = 1.0f;
    };
    void_t draw_polygon( polygon_info const & info ) noexcept;

    struct rect_filled_info
    {
        // world pos
        motor::math::vec2f_t pos;
        // x: width; y: height
        motor::math::vec2f_t size;

        ImColor color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );

        float_t radius = 1.0f;
        ImDrawFlags flags = 0;
    };
    void_t draw_rect_filled( rect_filled_info const & ) noexcept;

    struct rect_info
    {
        // world pos
        motor::math::vec2f_t pos;
        // x: width; y: height
        motor::math::vec2f_t size;

        ImColor color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );

        float_t thickness = 1.0f ;

        float_t radius = 0.0f;
        ImDrawFlags flags = 0;
    };
    void_t draw_rect( rect_info const & ) noexcept ;

    struct text_info
    {
        char const * title = nullptr;
        motor::math::vec2f_t pos;
        int_t font_height = 10;
        ImColor color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
    };
    void_t draw_text( text_info const & ) noexcept;


    struct clip_rect_info
    {
        motor::math::vec2f_t pos ;
        motor::math::vec2f_t size ;
    };
    void_t push_clip_rect( clip_rect_info const & ) noexcept ;
    void_t pop_clip_rect( void_t ) noexcept ;
};
motor_typedef( canvas_2d );
} // namespace motor::tool::imgui