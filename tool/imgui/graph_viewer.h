

#pragma once

#include "canvas_2d.h"

namespace motor::tool::imgui
{
class MOTOR_TOOL_API graph_viewer
{
    motor_this_typedefs( graph_viewer );

    public:

    using canvas2d_t = motor::tool::imgui::canvas_2d_t ;

  private:

  public:

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
    void_t draw_node( canvas2d_t & can, node_info const & info ) noexcept;

    struct link_info
    {
        motor::math::vec2f_t from;
        motor::math::vec2f_t from_size ;

        motor::math::vec2f_t to;
        motor::math::vec2f_t to_size ;

        ImColor color = ImColor( .40f, .30f, .70f, 1.0f );
    };
    void_t draw_link( canvas2d_t * can,link_info const & ) noexcept;

  private:

    enum class exec_pin
    {
        in,
        out
    };
    struct exec_pin_info
    {
        motor::math::vec2f_t node_pos;
        motor::math::vec2f_t node_size;
        this_t::exec_pin pin;
    };
    motor::math::vec2f_t exec_pin_position( canvas2d_t * can, this_t::exec_pin_info const & ) noexcept;
};
motor_typedef( graph_viewer );
} // namespace motor::tool::imgui