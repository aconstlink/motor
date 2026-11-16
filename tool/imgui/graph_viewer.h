// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "canvas_2d.h"

namespace motor::tool::imgui
{
class MOTOR_TOOL_API graph_viewer
{
    motor_this_typedefs( graph_viewer );

  public:

    using canvas2d_t = motor::tool::imgui::canvas_2d_t;

  private:

    struct rect_cache_entry
    {
        motor::math::vec2f_t bottom_left;
        motor::math::vec2f_t top_right;
    };
    motor::vector< rect_cache_entry > _rect_cache;
    size_t _cur_rect = size_t(-1) ;

    canvas2d_t * _can = nullptr;

  public: // imgui like

    bool_t _is_in = false;

  public:

    bool_t begin( canvas2d_t * ) noexcept;
    void_t end( void_t ) noexcept;

  public: // nodes

    struct node_info
    {
        char const * title;
        // world pos
        motor::math::vec2f_t pos;
        // x: width; y: height
        motor::math::vec2f_t size;
        ImColor header_color = ImColor( .40f, .30f, .70f, 1.0f );
        ImColor body_color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
        ImColor text_color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
        ImColor border_color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
    };
    size_t draw_node( node_info const & info ) noexcept;

    struct link_info
    {
        size_t node_id_from ;
        size_t node_id_to ;

        ImColor color = ImColor( .40f, .30f, .70f, 1.0f );
    };
    void_t draw_link( link_info const & ) noexcept;

  private:

    enum class exec_pin
    {
        in,
        out
    };
    struct exec_pin_info
    {
        size_t node_id ;
        this_t::exec_pin pin;
    };
    motor::math::vec2f_t exec_pin_position( this_t::exec_pin_info const & ) noexcept;
};
motor_typedef( graph_viewer );
} // namespace motor::tool::imgui