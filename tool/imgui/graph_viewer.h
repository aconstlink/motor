// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "canvas_2d.h"
#include "../typedefs.h"

namespace motor::tool::imgui
{
class MOTOR_TOOL_API graph_viewer
{
    motor_this_typedefs( graph_viewer );

  public:

    using canvas2d_t = motor::tool::imgui::canvas_2d_t;

    struct node_info
    {
        motor::string_t title;

        // world pos
        motor::math::vec2f_t pos;
        // x: width; y: height
        motor::math::vec2f_t size;
        ImColor header_color = ImColor( .40f, .30f, .70f, 1.0f );
        ImColor body_color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
        ImColor text_color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
        ImColor border_color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
    };

  private:

    struct node_entry
    {
        bool_t used = false;
        bool_t is_hovered = false;
        bool_t is_dragged = false;

        motor::string_t title;

        motor::math::vec2f_t bottom_left;
        motor::math::vec2f_t top_right;

        ImColor header_color = ImColor( .40f, .30f, .70f, 1.0f );
        ImColor body_color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
        ImColor text_color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
        ImColor border_color = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );

        motor::math::vec2f_t title_size;

        motor::math::vec2f_t size( void_t ) const noexcept
        {
            return ( bottom_left - top_right ).abs();
        }
    };
    motor::vector< node_entry > _nodes;

    size_t _dragging_idx = size_t( -1 );
    size_t _hovered_idx = size_t( -1 );
    motor::math::vec2f_t _mouse_pos;

    canvas2d_t * _can = nullptr;

  public: // imgui like

    // begin/end section
    bool_t _is_in = false;

  public: // node management

    size_t add_node( node_info const & info ) noexcept;
    bool_t remove_node( size_t const ) noexcept;
    
    using min_max_t = std::pair< motor::math::vec2f_t, motor::math::vec2f_t > ;
    min_max_t calculate_extend( void_t ) const noexcept;

  public: // draw related

    bool_t begin( canvas2d_t * ) noexcept;
    void_t end( void_t ) noexcept;
    bool_t draw_node( size_t const ) noexcept;
    bool_t draw_all_nodes( void_t ) noexcept;

    struct link_info
    {
        size_t node_id_from;
        size_t node_id_to;

        ImColor color = ImColor( 0.8f, 0.8f, 0.8f, 1.0f );
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
        size_t node_id;
        this_t::exec_pin pin;
    };
    motor::math::vec2f_t exec_pin_position( this_t::exec_pin_info const & ) noexcept;
};
motor_typedef( graph_viewer );
} // namespace motor::tool::imgui